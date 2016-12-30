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
// BCGPBreadcrumbControlImpl.cpp: implementation of the CBCGPBreadcrumbImpl class.
//
/////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "BCGPBreadcrumbControlImpl.h"
#include "BCGPBreadcrumb.h"
#include "BCGPVisualManager.h"
#include "BCGPDrawManager.h"

#ifndef _BCGSUITE_
#include "BCGPPopupMenu.h"
#include "BCGPContextMenuManager.h"
#endif

#ifndef NM_LDOWN
#define NM_LDOWN                (NM_FIRST-20)
#endif

#ifndef NM_RDOWN
#define NM_RDOWN                (NM_FIRST-21)
#endif

#ifndef NM_FONTCHANGED
#define NM_FONTCHANGED          (NM_FIRST-23)
#endif


class CMousePosTracker
{
protected:
	mutable POINT m_ptCursorPos;
	HWND m_hWnd;
	RECT m_rectTracked;
public:
	CMousePosTracker ()
		: m_hWnd (NULL)
	{
		::SetRectEmpty (&m_rectTracked);
		::GetCursorPos (&m_ptCursorPos);
	}

	POINT GetCursorPosClient () const
	{
		POINT ptCursorPos = m_ptCursorPos;
		::ScreenToClient (m_hWnd, &ptCursorPos);
		return ptCursorPos;
	}

	bool IsMousePosChanged () const
	{
		POINT ptOldCursorPos = m_ptCursorPos;
		::GetCursorPos (&m_ptCursorPos);
		return (ptOldCursorPos.x != m_ptCursorPos.x) || (ptOldCursorPos.y != m_ptCursorPos.y);
	}

	void TrackRect (HWND hwndHost, const RECT& rect)
	{
		m_hWnd = hwndHost;
		::CopyRect (&m_rectTracked, &rect);
	}

	bool IsMouseOut () const
	{
		if (!IsMousePosChanged ())
		{
			return false;
		}

		POINT ptCursorPos = m_ptCursorPos;
		::ScreenToClient (m_hWnd, &ptCursorPos);
		return !(::PtInRect (&m_rectTracked, ptCursorPos));
	}
};



struct _BREADCRUMBITEM
{
	static void* operator new (size_t nBytes)
	{
		return ::HeapAlloc (::GetProcessHeap (), 0, nBytes);
	}

	static void operator delete (void* ptr)
	{
		::HeapFree (::GetProcessHeap (), 0, ptr);
	}

	_BREADCRUMBITEM ()
		: iImage (-1), lParam (0), bDeleted (false), bDynamic (false), bHasData (false)
	{
	}

public:
	CString             strText;
	CString             strTooltip;
	_BREADCRUMBITEM*    pParent;
	int                 iImage;
	LPARAM              lParam;

	BOOL                bDeleted;
	BOOL                bDynamic;
	BOOL                bHasData; // valid only for dynamic items
};


class CBCGPBreadcrumbImpl
{
public:

	static void* operator new (size_t nBytes)
	{
		return ::HeapAlloc (::GetProcessHeap (), 0, nBytes);
	}

	static void operator delete (void* ptr)
	{
		::HeapFree (::GetProcessHeap (), 0, ptr);
	}

public:
	CBCGPBreadcrumbImpl (HWND hWnd)
		: m_hImageList (NULL)
		, m_pWnd (DYNAMIC_DOWNCAST (CBCGPBreadcrumb, CWnd::FromHandlePermanent (hWnd)))
		, m_hFont (NULL)
		, m_hwndTooltips (NULL)
		, m_clrText (CLR_INVALID)
		, m_clrBackground (CLR_INVALID)
		, m_clrHighlight (CLR_INVALID)
		, m_clrHighlightText (CLR_INVALID)
		, m_cxRightMargin (48)
		, m_bNeedRecalcLayout (false)
		, m_bCalculatingLayout (false)
		, m_bGettingDynamicSubItems (false)
		, m_bProcessingSelectionChange (false)
		, m_pSelectedItem (NULL)
		, m_bIsMenuDropped (false)
		, m_bNeedShowPopupMenuAgain (false)
		, m_pPressedItem (NULL)
		, m_iHilitedIndex (-2)
		, m_uHilitedHitTest (BCCHT_EMPTY)
		, m_bIsMenuClosedByMouseButton (false)
	{
		ASSERT_VALID (m_pWnd);

		m_pWnd->SetTimer (1, 50, NULL);
		ResetLayoutData ();
		ValidateSelection ();
	}

	~CBCGPBreadcrumbImpl ()
	{
		m_pWnd->KillTimer (1);
		ResetTooltips ();

		if (m_hFont != NULL)
		{
			::DeleteObject (m_hFont);
		}
	}

protected:
	struct LAYOUT_ITEM
	{
		_BREADCRUMBITEM*    pItem;
		CRect               rectItem;
		CRect               rectArrow;
		CString             strTooltip;

		LAYOUT_ITEM ()
		{
			pItem = NULL;
			rectItem.SetRectEmpty ();
			rectArrow.SetRectEmpty ();
		}
	};

	void ResetLayoutData ()
	{
		m_rectIconLayout.SetRectEmpty ();
		m_rectLeftButtonLayout.SetRectEmpty ();
		m_arrItemsLayout.RemoveAll ();
		m_uLeftMenuState = CDIS_DEFAULT;
		m_bRootItemVisible = false;
	}

	void CalculateLayout ()
	{
		m_bNeedRecalcLayout = true;
		Invalidate (); // actual layout recalculating occurs on next WM_PAINT message
	}

	void PerformCalculateLayout ()
	{
		// Prevent recursive calls
		if (m_bCalculatingLayout)
		{
			return;
		}

		CancelMenu ();

		ResetTooltips ();

		m_bCalculatingLayout = true;
		m_bNeedRecalcLayout = false;
		m_iHilitedIndex = -2;
		m_uHilitedHitTest = BCCHT_EMPTY;

		ResetLayoutData (); // Cleanup previous layout

		RECT rectClient;
		m_pWnd->GetClientRect (&rectClient);

		rectClient.left --;

		if (!IsWindowStylePresent (BCCS_EXTERNALBORDER))
		{
			rectClient.left ++;
			rectClient.top ++;
			rectClient.bottom --;
			rectClient.right --;
		}

		int cxControlWidth = rectClient.right - rectClient.left;
		int cyControlHeight = rectClient.bottom - rectClient.top;
		if (cxControlWidth <= 1 || cyControlHeight <= 1)
		{
			m_bCalculatingLayout = false;
			return;
		}

		if (m_hImageList != NULL)
		{
			int cx, cy;
			ImageList_GetIconSize (m_hImageList, &cx, &cy);
			rectClient.left += 2; // Gap between left border and icon
			m_rectIconLayout.left = rectClient.left;
			m_rectIconLayout.right = rectClient.left + cx;

			// Center icon vertically if there's too much space
			m_rectIconLayout.top = rectClient.top;
			if (cyControlHeight > cy)
			{
				m_rectIconLayout.top += (cyControlHeight - cy) / 2;
			}
			m_rectIconLayout.bottom = rectClient.top + cy;

			rectClient.left += cx;
			rectClient.left += 2; // Gap between icon and the following items
		}

		int nSpaceLeft = rectClient.right - rectClient.left;
		nSpaceLeft -= GetRightMarginLayoutValue ();

		if (ValidateSelection ())
		{
			if (!m_bProcessingSelectionChange)
			{
				m_bProcessingSelectionChange = true;
				NotifyParent (BCCN_SELECTIONCHANGED);
				m_bProcessingSelectionChange = false;
			}
		}

		const _BREADCRUMBITEM& rootItem = GetRootItem ();
		_BREADCRUMBITEM* pItem = m_pSelectedItem;

		m_bRootItemVisible = (m_pSelectedItem == &rootItem || IsWindowStylePresent (BCCS_SHOWROOTALWAYS));
		if (rootItem.strText.IsEmpty ())
		{
			m_bRootItemVisible = false;
		}
		
		CDC* pdcClient = m_pWnd->GetDC ();
		HFONT hFontOld = SelectCurrentFont (pdcClient->GetSafeHdc ());

		CArray<LAYOUT_ITEM, const LAYOUT_ITEM&> arrTemp;

		while (pItem != NULL && nSpaceLeft > 0)
		{
			LAYOUT_ITEM layout;
			layout.pItem = pItem;
			if (pItem != &rootItem || m_bRootItemVisible)
			{
				// Add item text
				SIZE szItemText;
				if (MeasuseItem (*pItem, pdcClient->GetSafeHdc (), szItemText))
				{
					layout.rectItem.right = szItemText.cx;
					layout.rectItem.bottom = szItemText.cy;
				}
			}

			if (GetSubitemsCount (*pItem) > 0)
			{
				// Add item arrow
				SIZE szArrow;
				if (MeasuseArrow (*pItem, pdcClient->GetSafeHdc (), szArrow))
				{
					layout.rectArrow.right = szArrow.cx;
					layout.rectArrow.bottom = szArrow.cy;
				}
			}

			if (!layout.rectItem.IsRectEmpty () || !layout.rectArrow.IsRectEmpty ())
			{
				nSpaceLeft -= (layout.rectItem.right - layout.rectItem.left);
				nSpaceLeft -= (layout.rectArrow.right - layout.rectArrow.left);
				if (nSpaceLeft >= 0 || arrTemp.GetSize () == 0)
				{
					arrTemp.Add (layout);
				}
			}

			pItem = pItem->pParent;
		}

		int nVisible = (int)arrTemp.GetSize ();
		if (nVisible > 0)
		{
			int xPos = rectClient.left;
			bool bIsRootItemVisible = arrTemp[nVisible - 1].pItem == &rootItem;

			if (!bIsRootItemVisible)
			{
				SIZE szArrow;
				if (MeasureLeftMenuButton (pdcClient->GetSafeHdc (), szArrow))
				{
					m_rectLeftButtonLayout.top = rectClient.top;
					m_rectLeftButtonLayout.left = xPos;
					m_rectLeftButtonLayout.bottom = rectClient.bottom;
					m_rectLeftButtonLayout.right = xPos + szArrow.cx;
					xPos += szArrow.cx;
				}
			}

			for (int i = nVisible - 1; i >= 0; --i)
			{
				int itemWidth = arrTemp[i].rectItem.right; //.left == 0
				int arrowWidth = arrTemp[i].rectArrow.right; //.left == 0
				if (xPos >= rectClient.right)
				{
					break;
				}

				if (xPos + arrowWidth + itemWidth > rectClient.right)
				{
					itemWidth = (rectClient.right - xPos - arrowWidth);
					itemWidth = max (itemWidth, 12); // 12 pixels is the minimal width for an item text
				}

				if (itemWidth > 0)
				{
					arrTemp[i].rectItem.left = xPos;
					arrTemp[i].rectItem.right = xPos + itemWidth;
					arrTemp[i].rectItem.top = rectClient.top;
					arrTemp[i].rectItem.bottom = rectClient.bottom;
					xPos += itemWidth;
				}

				if (arrowWidth > 0)
				{
					arrTemp[i].rectArrow.left = xPos;
					arrTemp[i].rectArrow.right = xPos + arrowWidth;
					arrTemp[i].rectArrow.top = rectClient.top;
					arrTemp[i].rectArrow.bottom = rectClient.bottom;
					xPos += arrowWidth;
				}

				m_arrItemsLayout.Add (arrTemp[i]);
			}
		}

		pdcClient->SelectObject (hFontOld);
		m_pWnd->ReleaseDC (pdcClient);

		// Setting tooltips
		if (::IsWindow (m_hwndTooltips))
		{
			for (int i = 0; i < (int)m_arrItemsLayout.GetSize (); ++i)
			{
				CRect rectTool = m_arrItemsLayout[i].rectItem;
				if (!m_arrItemsLayout[i].rectArrow.IsRectEmpty ())
				{
					int r = m_arrItemsLayout[i].rectArrow.right;
					if (r > rectTool.right) rectTool.right = r;
				}

				LPCTSTR pszToolText = NULL;
				if (m_arrItemsLayout[i].pItem != NULL)
				{
					m_arrItemsLayout[i].strTooltip = m_arrItemsLayout[i].pItem->strTooltip;
					pszToolText = m_arrItemsLayout[i].strTooltip;
				}

				if (!::IsRectEmpty (&rectTool) && lstrlen (pszToolText) > 0)
				{
					LAYOUT_ITEM* pLayoutItem = &m_arrItemsLayout[i];
					TOOLINFO ti;
					ZeroMemory (&ti, sizeof (ti));
					ti.cbSize = sizeof (ti);
					ti.uFlags = TTF_TRANSPARENT | TTF_SUBCLASS;
					ti.hwnd = m_pWnd->GetSafeHwnd ();
					ti.uId = (UINT_PTR)(void*)pLayoutItem;
					ti.lpszText = const_cast<LPTSTR>(pszToolText);
					ti.rect = rectTool;
					::SendMessage (m_hwndTooltips, TTM_ADDTOOL, 0, (LPARAM)&ti);
				}
			}
		}

		m_bCalculatingLayout = false;
	}

	void ResetTooltips ()
	{
		if (!::IsWindow (m_hwndTooltips)) return;

		for (int i = 0; i < (int)m_arrItemsLayout.GetSize (); ++i)
		{
			TOOLINFO ti;
			ti.cbSize = sizeof (ti);
			ti.hwnd = m_pWnd->GetSafeHwnd ();
			ti.uId = (UINT_PTR)(void*)&m_arrItemsLayout[i];
			::SendMessage (m_hwndTooltips, TTM_DELTOOL, 0, (LPARAM)&ti);
		}
	}

	int LayoutIndexFromItem (_BREADCRUMBITEM* pItem)
	{
		for (int i = 0; i < (int)m_arrItemsLayout.GetSize (); ++i)
		{
			if (m_arrItemsLayout[i].pItem == pItem)
			{
				return i;
			}
		}
		return -2;
	}

	bool IsItemVisible (const _BREADCRUMBITEM* pItem) const
	{
		int n = (int)m_arrItemsLayout.GetSize ();
		for (int i = 0; i < n; ++i)
		{
			if (m_arrItemsLayout[i].pItem == pItem)
			{
				return !m_arrItemsLayout[i].rectItem.IsRectEmpty ();
			}
		}
		return false;
	}

	// Returns size in pixels of content (icon + items) that must be visible regardless of control width.
	UINT GetMinimalContentWidth () const
	{
		UINT min = 50;
		if (m_hImageList != NULL)
		{
			int cx, cy;
			ImageList_GetIconSize (m_hImageList, &cx, &cy);
			min += cx + 2; // gap size for icon
		}
		return min;
	}

	UINT GetRightMarginLayoutValue () const
	{
		RECT rectClient;
		m_pWnd->GetClientRect (&rectClient);
		int cxRemainder = rectClient.right - rectClient.left - GetMinimalContentWidth ();
		if (cxRemainder < 0)
		{
			return 0;
		}
		return min (cxRemainder, (int)m_cxRightMargin);
	}

	CRect GetInplaceEditorArea () const
	{
		CRect rect;
		m_pWnd->GetClientRect (&rect);
		if (m_rectIconLayout.IsRectEmpty ())
		{
			return rect;
		}
		rect.left = m_rectIconLayout.right + 2;
		return rect;
	}


	bool MeasuseItem (const _BREADCRUMBITEM& item, HDC hdcClient, SIZE& size)
	{
		UNUSED_ALWAYS (hdcClient);
		
		::GetTextExtentPoint32 (hdcClient, item.strText, item.strText.GetLength (), &size);
		size.cx += 10; // Margins: 5 pixels left an right

		return true;
	}

	bool MeasuseArrow (const _BREADCRUMBITEM& item, HDC hdcClient, SIZE& size)
	{
		UNUSED_ALWAYS (hdcClient);
		UNUSED_ALWAYS (item);

		RECT rectClient;
		m_pWnd->GetClientRect (&rectClient);

		size.cx = 13;
		size.cy = rectClient.bottom - rectClient.top;

		return true;
	}

	bool MeasureLeftMenuButton (HDC hdcClient, SIZE& size)
	{
		UNUSED_ALWAYS (hdcClient);
		
		RECT rectClient;
		m_pWnd->GetClientRect (&rectClient);

		size.cx = 15;
		size.cy = rectClient.bottom - rectClient.top;

		return true;
	}

	_BREADCRUMBITEM* HitTest (POINT ptClient, UINT& uResult) const
	{
		CRect rectClient;
		m_pWnd->GetClientRect (&rectClient);
		
		if (!rectClient.PtInRect (ptClient))
		{
			uResult = BCCHT_OUT;
			return NULL;
		}

		// Check if inplace editor is active
		CWnd* pEditor = m_pWnd->GetWindow (GW_CHILD);
		if (pEditor != NULL && pEditor->IsWindowVisible ())
		{
			uResult = BCCHT_EMPTY;
			return NULL;
		}

		if (m_rectIconLayout.PtInRect (ptClient))
		{
			uResult = BCCHT_ICON;
			return NULL;
		}

		if (m_rectLeftButtonLayout.PtInRect (ptClient))
		{
			uResult = BCCHT_MENUBUTTON;
			return NULL;
		}

		for (int i = 0; i < (int)m_arrItemsLayout.GetSize (); ++i)
		{
			if (m_arrItemsLayout[i].rectItem.PtInRect (ptClient))
			{
				uResult = BCCHT_ITEM;
				return m_arrItemsLayout[i].pItem;
			}
			if (m_arrItemsLayout[i].rectArrow.PtInRect (ptClient))
			{
				uResult = BCCHT_ITEMBUTTON;
				return m_arrItemsLayout[i].pItem;
			}
		}

		if (IsWindowStylePresent (BCCS_INPLACEEDIT))
		{
			CRect rectEditorArea = GetInplaceEditorArea ();
			if (rectEditorArea.PtInRect (ptClient))
			{
				uResult = BCCHT_EDIT;
				return NULL;
			}
		}

		uResult = BCCHT_EMPTY;
		return NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	//                      Window Procedure
	//////////////////////////////////////////////////////////////////////////
public:
	virtual LRESULT WindowProc (UINT code, WPARAM wParam, LPARAM lParam, BOOL* bHandled)
	{
		LRESULT lResult = 0;
		if (bHandled != NULL)
		{
			*bHandled = TRUE;
		}

		if (MsgUpdate (code, wParam, lParam, lResult))
		{
			return lResult;
		}
		if (MsgPaint (code, wParam, lParam, lResult))
		{
			return lResult;
		}
		if (MsgColors (code, wParam, lParam, lResult))
		{
			return lResult;
		}
		if (MsgFont (code, wParam, lParam, lResult))
		{
			return lResult;
		}
		if (MsgTooltips (code, wParam, lParam, lResult))
		{
			return lResult;
		}
		if (MsgStyleChange (code, wParam, lParam, lResult))
		{
			return lResult;
		}
		if (MsgImageList (code, wParam, lParam, lResult))
		{
			return lResult;
		}
		if (MsgRightMargin (code, wParam, lParam, lResult))
		{
			return lResult;
		}
		if (MsgMenuCommand (code, wParam, lParam, lResult))
		{
			return lResult;
		}
		if (MsgFocus (code, wParam, lParam, lResult))
		{
			return lResult;
		}
		if (MsgDlgCode (code, wParam, lParam, lResult))
		{
			return lResult;
		}
		if (MsgText (code, wParam, lParam, lResult))
		{
			return lResult;
		}
		if (MsgKeyboard (code, wParam, lParam, lResult))
		{
			return lResult;
		}
		if (MsgClicks (code, wParam, lParam, lResult))
		{
			return lResult;
		}
		if (MsgHitTest (code, wParam, lParam, lResult))
		{
			return lResult;
		}
		if (MsgTimer (code, wParam, lParam, lResult))
		{
			return lResult;
		}
		if (MsgItems (code, wParam, lParam, lResult))
		{
			return lResult;
		}
		if (MsgDestroy (code, wParam, lParam, lResult))
		{
			return lResult;
		}

		if (bHandled != NULL)
		{
			*bHandled = FALSE;
		}

		return 0;
	}


	//////////////////////////////////////////////////////////////////////////
	//                      Message Handlers
	//////////////////////////////////////////////////////////////////////////

protected:

	bool MsgUpdate (UINT code, WPARAM wParam, LPARAM lParam, LRESULT& rResult)
	{
		UNUSED_ALWAYS (wParam);
		UNUSED_ALWAYS (lParam);
		UNUSED_ALWAYS (rResult);
		if (code == WM_SIZE)
		{
			CalculateLayout ();
			return true;
		}

		return false;
	}

	bool MsgPaint (UINT code, WPARAM wParam, LPARAM lParam, LRESULT& rResult)
	{
		if (code == WM_ERASEBKGND)
		{
			rResult = 0;
			return true;
		}
		if (code == WM_PAINT)
		{
			if (m_bNeedRecalcLayout)
			{
				PerformCalculateLayout ();
			}

			PAINTSTRUCT ps;
			PaintControl (::BeginPaint (m_pWnd->GetSafeHwnd (), &ps));
			::EndPaint (m_pWnd->GetSafeHwnd (), &ps);   
			return true;
		}

		if (code == WM_NCPAINT)
		{
			CWindowDC dc (m_pWnd);

			CRect rectWindow;
			m_pWnd->GetWindowRect (&rectWindow);

			CRect rectClient;
			m_pWnd->GetClientRect (&rectClient);
			CPoint ptClientTopLeft = rectClient.TopLeft ();
			m_pWnd->ClientToScreen (&ptClientTopLeft);
			rectClient.OffsetRect (ptClientTopLeft.x - rectWindow.left, ptClientTopLeft.y - rectWindow.top);
			dc.ExcludeClipRect (rectClient);

			rectWindow.OffsetRect (- rectWindow.left, - rectWindow.top);

			if (!rectWindow.IsRectEmpty ())
			{
				CBCGPVisualManager::GetInstance ()->OnDrawControlBorder (&dc, rectWindow, m_pWnd, TRUE);
			}
			return true;
		}

		if (code == WM_PRINTCLIENT && (lParam & PRF_CLIENT) != 0)
		{
			PaintControl ((HDC)wParam);
			return true;
		}

		if (code == WM_PRINT && (lParam & PRF_NONCLIENT) == PRF_NONCLIENT)
		{
			CDC* pDC = CDC::FromHandle((HDC) wParam);
			ASSERT_VALID(pDC);
			
			CRect rect;
			m_pWnd->GetWindowRect(rect);
			
			rect.bottom -= rect.top;
			rect.right -= rect.left;
			rect.left = rect.top = 0;
			
			CBCGPVisualManager::GetInstance ()->OnDrawControlBorder(pDC, rect, m_pWnd, FALSE);
			return false;
		}

		return false;

	}

	bool MsgFont (UINT code, WPARAM wParam, LPARAM lParam, LRESULT& rResult)
	{
		if (code == WM_SETFONT)
		{
			HFONT hfNew = (HFONT)wParam;
			if (hfNew != m_hFont)
			{
				::DeleteObject (m_hFont);
				m_hFont = hfNew;
				CalculateLayout ();
				NotifyParent (NM_FONTCHANGED);
			}
			if (LOWORD (lParam) != 0)
			{
				m_pWnd->RedrawWindow (NULL, NULL, RDW_UPDATENOW | RDW_ALLCHILDREN);
			}
			return true;
		}

		if (code == WM_GETFONT)
		{
			rResult = (LRESULT)m_hFont;
			return true;
		}

		return false;
	}

	bool MsgColors (UINT code, WPARAM wParam, LPARAM lParam, LRESULT& rResult)
	{
		UNUSED_ALWAYS (wParam);

		if (code == BCCM_SETTEXTCOLOR)
		{
			rResult = (LRESULT)m_clrText;
			m_clrText = (COLORREF)lParam; 
			Invalidate ();
			return true;
		}
		if (code == BCCM_GETTEXTCOLOR)
		{
			rResult = (LRESULT)m_clrText;
			return true;
		}

		if (code == BCCM_SETBKCOLOR)
		{
			rResult = (LRESULT)m_clrBackground;
			m_clrBackground = (COLORREF)lParam; 
			Invalidate ();
			return true;
		}
		if (code == BCCM_GETBKCOLOR)
		{
			rResult = (LRESULT)m_clrBackground;
			return true;
		}

		if (code == BCCM_SETHILITECOLOR)
		{
			rResult = (LRESULT)m_clrHighlight;
			m_clrHighlight = (COLORREF)lParam; 
			Invalidate ();
			return true;
		}
		if (code == BCCM_GETHILITECOLOR)
		{
			rResult = (LRESULT)m_clrHighlight;
			return true;
		}

		if (code == BCCM_SETHILITETEXTCOLOR)
		{
			rResult = (LRESULT)m_clrHighlightText;
			m_clrHighlightText = (COLORREF)lParam; 
			Invalidate ();
			return true;
		}
		if (code == BCCM_GETHILITETEXTCOLOR)
		{
			rResult = (LRESULT)m_clrHighlightText;
			return true;
		}

		return false;
	}

	bool MsgImageList (UINT code, WPARAM wParam, LPARAM lParam, LRESULT& rResult)
	{
		UNUSED_ALWAYS (wParam);

		if (code == BCCM_SETIMAGELIST)
		{
			rResult = (LRESULT)m_hImageList;
			m_hImageList = (HIMAGELIST)lParam; 
			CalculateLayout ();
			return true;
		}
		if (code == BCCM_GETIMAGELIST)
		{
			rResult = (LRESULT)m_hImageList;
			return true;
		}
		return false;
	}

	bool MsgTooltips (UINT code, WPARAM wParam, LPARAM lParam, LRESULT& rResult)
	{
		UNUSED_ALWAYS (lParam);

		if (code == BCCM_SETTOOLTIPS)
		{
			rResult = (LRESULT)m_hwndTooltips;
			if (::IsWindow (m_hwndTooltips))
			{
				::SendMessage (m_hwndTooltips, TTM_POP, 0, 0);
				ResetTooltips ();
				m_hwndTooltips = NULL; 
			}
			else
			{
				m_hwndTooltips = (HWND)wParam; 
				CalculateLayout ();
			}

			return true;
		}
		if (code == BCCM_GETTOOLTIPS)
		{
			rResult = (LRESULT)m_hwndTooltips;
			return true;
		}
		return false;
	}

	bool MsgRightMargin (UINT code, WPARAM wParam, LPARAM lParam, LRESULT& rResult)
	{
		UNUSED_ALWAYS (lParam);

		if (code == BCCM_SETRIGHTMARGIN)
		{
			rResult = (LRESULT)m_cxRightMargin;
			m_cxRightMargin = (UINT)wParam;
			CalculateLayout ();
			return true;
		}
		if (code == BCCM_GETRIGHTMARGIN)
		{
			rResult = (LRESULT)((wParam == FALSE) ? m_cxRightMargin : GetRightMarginLayoutValue ());
			return true;
		}
		return false;
	}
	
	bool MsgMenuCommand (UINT code, WPARAM wParam, LPARAM lParam, LRESULT& rResult)
	{
		UNUSED_ALWAYS (lParam);

		if (code == WM_COMMAND && HIWORD (wParam) == 0) // menu command
		{
			int itemID = LOWORD (wParam) - 1;
			if (itemID < m_arrCurrentMenuItems.GetSize ())
			{
				_BREADCRUMBITEM* pItem = m_arrCurrentMenuItems[itemID];
				m_arrCurrentMenuItems.RemoveAll ();
				SetSelection (pItem);
				rResult = 0;
				return true;
			}
		}

		return false;
	}

	bool MsgStyleChange (UINT code, WPARAM wParam, LPARAM lParam, LRESULT& rResult)
	{
		if (code == WM_STYLECHANGED)
		{
			const STYLESTRUCT* pStyles = (const STYLESTRUCT*)lParam;
			if (wParam == GWL_STYLE && pStyles != NULL)
			{
				DWORD dwChanged = pStyles->styleNew ^ pStyles->styleOld;

				if (dwChanged & WS_DISABLED)
				{
					Invalidate ();
				}
				if (dwChanged & BCCS_SHOWROOTALWAYS)
				{
					CalculateLayout ();
				}

				rResult = 0;
				return true;
			}
		}
		return false;
	}

	bool MsgDlgCode (UINT code, WPARAM wParam, LPARAM lParam, LRESULT& rResult)
	{
		UNUSED_ALWAYS (lParam);

		if (code == WM_GETDLGCODE)
		{
			rResult = (IsWindowStylePresent (BCCS_WANTRETURN)) ? 
							DLGC_WANTALLKEYS :
							DLGC_WANTCHARS | DLGC_WANTARROWS;

			if (wParam == VK_TAB)
			{
				rResult = DLGC_WANTARROWS;
			}
			return true;
		}

		return false;
	}

	bool MsgText (UINT code, WPARAM wParam, LPARAM lParam, LRESULT& rResult)
	{
		if (code == WM_GETTEXTLENGTH || code == WM_GETTEXT)
		{
			CString strPath = m_pWnd->GetSelectedPath ();
			if (code == WM_GETTEXTLENGTH)
			{
				rResult = strPath.GetLength ();
				return true;
			}
			else if ((int)wParam > 0)
			{
				LPTSTR pszBuffer = (LPTSTR)lParam;
				if (::lstrcpyn (pszBuffer, strPath, (int)wParam) == NULL)
				{
					pszBuffer[(int)wParam - 1] = 0;
				}
				return true;
			}
		}
		if (code == WM_SETTEXT)
		{
			CString strPath = CString ((LPCTSTR)lParam);
			rResult = m_pWnd->SelectPath (strPath);
			return true;
		}

		return false;
	}

	bool MsgKeyboard (UINT code, WPARAM wParam, LPARAM lParam, LRESULT& rResult)
	{
		if (code == WM_KEYDOWN)
		{
			rResult = 0;

			NMKEY nm;
			nm.nVKey = (UINT)wParam;
			nm.uFlags = (UINT)lParam;
			if (NotifyParent (nm.hdr, NM_KEYDOWN) != 0)
			{
				return true;    // Parent has already processed message
			}

			if (wParam == VK_RETURN)
			{
				NotifyParent (NM_RETURN);
			}

			// keyboard navigation
			switch (wParam)
			{
			case VK_RETURN:
				SetSelection (GetCurrentHilitedItem ());
				return true;
			case VK_LEFT:
				MoveHiliteLeft ();
				return true;
			case VK_RIGHT:
				MoveHiliteRight ();
				return true;
			case VK_DOWN:
			case VK_UP:
				if (m_uHilitedHitTest == BCCHT_MENUBUTTON)
				{
					ShowLeftButtonMenu ();
				}
				else
				{
					ShowItemMenu (GetCurrentHilitedItem ());
				}
				return true;
			case VK_SPACE:
			case VK_F2:
				if (IsWindowStylePresent (BCCS_INPLACEEDIT) && !m_bIsMenuDropped)
				{
					BeginInplaceEdit ();
					return true;
				}
				break;
			}
		}

		return false;
	}

	bool MsgClicks (UINT code, WPARAM wParam, LPARAM lParam, LRESULT& rResult)
	{
		UNUSED_ALWAYS (wParam); // key states

		if (code == WM_LBUTTONDOWN || code == WM_LBUTTONUP || code == WM_RBUTTONDOWN || code == WM_RBUTTONUP)
		{
			CPoint pt(BCG_GET_X_LPARAM(lParam), BCG_GET_Y_LPARAM(lParam));
			UINT uHT = 0;
			_BREADCRUMBITEM* pItem = HitTest (pt, uHT);

			rResult = 0;    // Message has been processed

			NMCLICK nmClick;
			nmClick.pt = pt;
			m_pWnd->ClientToScreen (&nmClick.pt);
			nmClick.dwHitInfo = uHT;
			nmClick.dwItemSpec = (DWORD_PTR)(HBREADCRUMBITEM)pItem;
			nmClick.dwItemData = (pItem == NULL) ? 0 : pItem->lParam;

			// Processing right button

			if (code == WM_RBUTTONDOWN)
			{
				NotifyParent (nmClick.hdr, NM_RDOWN);
				return true;
			}
			if (code == WM_RBUTTONUP)
			{
				if (NotifyParent (nmClick.hdr, NM_RCLICK) != 0)
				{
					return true;
				}

				// Default action on right button
				return true;
			}

			// Processing left button
			if (code == WM_LBUTTONDOWN)
			{
				m_bIsMenuClosedByMouseButton = m_bIsMenuDropped; // Stores the m_bIsMenuDropped for WM_LBUTTONUP.

				NotifyParent (nmClick.hdr, NM_LDOWN);

				switch (uHT)
				{
				case BCCHT_ITEM:
					if (pItem != NULL)
					{
						m_pWnd->SetCapture ();
						m_pPressedItem = pItem;
						Invalidate ();
						return true;
					}
					break;
				case BCCHT_MENUBUTTON:
					if (!m_bIsMenuClosedByMouseButton)
					{
						m_uLeftMenuState = CDIS_SELECTED;
						Invalidate  ();
					}
					return true;
				case BCCHT_ITEMBUTTON:
					if (pItem != NULL && !m_bIsMenuClosedByMouseButton)
					{
						m_pPressedItem = pItem;
						Invalidate ();
						return true;
					}
					break;
				}
			}
			
			if (code == WM_LBUTTONUP)
			{
				if (m_pPressedItem != NULL)
				{
					::ReleaseCapture ();
					Invalidate ();
				}

				if (!m_bIsMenuDropped)
				{
					if (NotifyParent (nmClick.hdr, NM_CLICK) != 0)
					{
						// Skip default processing
						return true;
					}

					if (uHT == BCCHT_ITEM && m_pPressedItem == pItem)
					{
						SetSelection (m_pPressedItem);
					}

					if (uHT == BCCHT_MENUBUTTON && !m_bIsMenuClosedByMouseButton)
					{
						ShowLeftButtonMenu ();
						return true;
					}

					if (uHT == BCCHT_ITEMBUTTON && pItem != NULL && !m_bIsMenuClosedByMouseButton)
					{
						ShowItemMenu (pItem);
						return true;
					}

					m_uLeftMenuState = CDIS_DEFAULT;
					m_pPressedItem = NULL;
					Invalidate ();
				}

				if (uHT == BCCHT_EDIT)
				{
					if (m_pPressedItem == NULL && !m_bIsMenuDropped)
					{
						BeginInplaceEdit ();
					}
					else
					{
						m_pWnd->SetFocus ();
					}
				}

				// Reset the flag after WM_LBUTTONUP processing.
				m_bIsMenuClosedByMouseButton = false;
			}

			UpdateHilite ();
		}

		return false;
	}

	bool MsgHitTest (UINT code, WPARAM wParam, LPARAM lParam, LRESULT& rResult)
	{
		UNUSED_ALWAYS (wParam);

		if (code == BCCM_HITTEST && lParam != NULL)
		{
			NMCLICK* pHitTest = (NMCLICK*)lParam;
			pHitTest->hdr.code = NM_CLICK;
			pHitTest->hdr.hwndFrom = m_pWnd->GetSafeHwnd ();
			pHitTest->hdr.idFrom = m_pWnd->GetDlgCtrlID ();

			UINT uHT = 0;
			_BREADCRUMBITEM* pItem = HitTest (pHitTest->pt, uHT);

			pHitTest->dwHitInfo = uHT;
			pHitTest->dwItemSpec = (DWORD_PTR)(HBREADCRUMBITEM)pItem;
			pHitTest->dwItemData = (pItem == NULL) ? 0 : pItem->lParam;
			rResult = TRUE; // Message has been processed
			return true;
		}

		if (code == BCCM_GETINPLACEEDITRECT && lParam != NULL)
		{
			RECT* pRect = (RECT*)lParam;
			*pRect = GetInplaceEditorArea ();
			rResult = TRUE;
			return true;
		}

		return false;
	}

	bool MsgTimer (UINT code, WPARAM wParam, LPARAM lParam, LRESULT& rResult)
	{
		UNUSED_ALWAYS (wParam);
		UNUSED_ALWAYS (lParam);
		UNUSED_ALWAYS (rResult);

		if (code == WM_TIMER && m_pWnd->IsWindowEnabled())
		{
			if (m_bNeedShowPopupMenuAgain)
			{
				m_bNeedShowPopupMenuAgain = false;

				_BREADCRUMBITEM* pItem = NULL;
				if (m_iHilitedIndex >= 0 && m_iHilitedIndex < (int)m_arrItemsLayout.GetSize ())
				{
					pItem = m_arrItemsLayout[m_iHilitedIndex].pItem;
				}

				switch (m_uHilitedHitTest)
				{
				case BCCHT_ICON:
				case BCCHT_MENUBUTTON:
					ShowLeftButtonMenu ();
					break;
				case BCCHT_ITEM:
				case BCCHT_ITEMBUTTON:
					if (pItem != NULL)
					{
						ShowItemMenu (pItem);
					}
					break;
				}

				return false;
			}

			if (m_MouseTracker.IsMousePosChanged ())
			{
				UpdateHilite ();
				return false;
			}
		}

		return false;
	}

	bool MsgFocus (UINT code, WPARAM wParam, LPARAM lParam, LRESULT& rResult)
	{
		UNUSED_ALWAYS (wParam);
		UNUSED_ALWAYS (lParam);
		UNUSED_ALWAYS (rResult);

		if (code == WM_SETFOCUS || code == WM_KILLFOCUS)
		{
			NotifyParent ((code == WM_SETFOCUS) ? NM_SETFOCUS : NM_KILLFOCUS);

			// Setting focus to leftmost item
			if (code == WM_SETFOCUS)
			{
				if (GetCurrentHilitedItem () == NULL)
				{
					MoveHiliteHome ();
				}
			}
			else
			{
				RemoveHilite ();
			}
			return true;
		}

		return false;
	}

	bool MsgItems (UINT code, WPARAM wParam, LPARAM lParam, LRESULT& rResult)
	{
		switch (code)
		{
		case BCCM_INSERTITEM:
			{
				rResult = NULL;

				_BREADCRUMBITEM item;
				if (SetItemInfo (item, (const BREADCRUMBITEMINFO*)lParam, false)) // ignore mask when inserting item
				{
					POSITION pos = m_Items.AddTail (item);
					if (pos != NULL)
					{
						rResult = (LRESULT)&m_Items.GetAt (pos);
						CalculateLayout ();
					}
				}
			}
			return true;

		case BCCM_DELETEITEM:
			{
				rResult = FALSE;
				_BREADCRUMBITEM* pItem = ItemFromHandle ((HBREADCRUMBITEM)lParam);
				if (pItem == NULL)
				{
					::SetLastError (ERROR_INVALID_PARAMETER);
					return true;
				}

				bool bModifyTree = IsItemInSelectionPath (pItem);
				if (RemoveItem (*pItem) > 0)
				{
					CleanupRemovedItems ();
					if (ValidateSelection () || bModifyTree)
					{
						if (!m_bProcessingSelectionChange)
						{
							m_bProcessingSelectionChange = true;
							NotifyParent (BCCN_SELECTIONCHANGED);
							m_bProcessingSelectionChange = false;
						}
						CalculateLayout ();
					}
					rResult = TRUE;
				}
			}
			return true;

		case BCCM_SETITEM:
			{
				rResult = FALSE;
				_BREADCRUMBITEM* pItem = ItemFromHandle ((HBREADCRUMBITEM)wParam);
				if (pItem == NULL)
				{
					::SetLastError (ERROR_INVALID_PARAMETER);
					return true;
				}

				if (SetItemInfo (*pItem, (const BREADCRUMBITEMINFO*)lParam))
				{
					rResult = TRUE;
				}
			}
			return true;

		case BCCM_GETITEM:
			{
				rResult = FALSE;
				_BREADCRUMBITEM* pItem = ItemFromHandle ((HBREADCRUMBITEM)wParam);
				if (pItem == NULL)
				{
					::SetLastError (ERROR_INVALID_PARAMETER);
					return true;
				}

				rResult = GetItemInfo ((BREADCRUMBITEMINFO*)lParam, *pItem);
			}
			return true;

		case BCCM_GETROOTITEM:
			rResult = (LRESULT)(HBREADCRUMBITEM)&(GetRootItem ());
			return true;

		case BCCM_GETSELECTEDITEM:
			ValidateSelection ();
			rResult = (LRESULT)(HBREADCRUMBITEM)m_pSelectedItem;
			return true;

		case BCCM_SELECTITEM:
			ValidateSelection ();
			rResult = (LRESULT)(HBREADCRUMBITEM)m_pSelectedItem; // previous active item
			SetSelection (ItemFromHandle ((HBREADCRUMBITEM)lParam));
			return true;

		case BCCM_GETSUBITEMCOUNT:
			{
				rResult = FALSE;
				_BREADCRUMBITEM* pItem = ItemFromHandle ((HBREADCRUMBITEM)lParam);
				if (pItem == NULL)
				{
					::SetLastError (ERROR_INVALID_PARAMETER);
					return true;
				}

				rResult = (LRESULT)GetSubitemsCount (*pItem);
			}
			return true;

		case BCCM_GETSUBITEM:
			{
				rResult = FALSE;
				_BREADCRUMBITEM* pItem = ItemFromHandle ((HBREADCRUMBITEM)lParam);
				if (pItem == NULL)
				{
					::SetLastError (ERROR_INVALID_PARAMETER);
					return true;
				}

				rResult = (LRESULT)GetSubitem (*pItem, (int)wParam);
				if (rResult == NULL)
				{
					::SetLastError (ERROR_INVALID_INDEX);
					return true;
				}
			}

			return true;

		case BCCM_RESETCONTENT:
			RemoveAllItems ();
			rResult = TRUE;
			return true;
		}

		return false;
	}


	bool MsgDestroy (UINT code, WPARAM wParam, LPARAM lParam, LRESULT& rResult)
	{
		UNUSED_ALWAYS (wParam);
		UNUSED_ALWAYS (lParam);
		UNUSED_ALWAYS (rResult);

		if (code == WM_DESTROY)
		{
			RemoveAllItems ();
			return true;
		}
	
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	//                          Notifications
	//////////////////////////////////////////////////////////////////////////

	void NotifyDeleteItem (const _BREADCRUMBITEM& item)
	{
		if (item.lParam != NULL)
		{
			DELETEITEMSTRUCT del;
			del.CtlType = 0; // undefined control type
			del.CtlID = m_pWnd->GetDlgCtrlID ();
			del.hwndItem = m_pWnd->GetSafeHwnd ();
			del.itemID = (UINT)-1; // no index provided
			del.itemData = item.lParam;
			m_pWnd->GetParent ()->SendMessage (WM_DELETEITEM, (WPARAM)del.CtlID, (LPARAM)&del);
		}
	}


	//////////////////////////////////////////////////////////////////////////
	//                  Working with Breadcrumb Items
	//////////////////////////////////////////////////////////////////////////

	void GetDynamicSubItems (_BREADCRUMBITEM& item)
	{
		m_bGettingDynamicSubItems = true;
		if (item.bDynamic && !item.bHasData)
		{
			BREADCRUMB_ITEM_NOTIFICATION itemInfo;
			itemInfo.hItem = (HBREADCRUMBITEM)&item;
			BOOL bSubItemsAdded = NotifyParent (itemInfo.nmhdr, BCCN_DYNAMIC_GETSUBITEMS) != FALSE;

			item.bHasData = true;
			if (bSubItemsAdded && IsItemInSelectionPath (&item))
			{
				CalculateLayout ();
			}
		}
		m_bGettingDynamicSubItems = false;
	}

	// Tries to free all dynamic subitems recursively.
	// Can be applied to non-dynamic items also.
	void CleanupDynamicSubItems (_BREADCRUMBITEM* pItem)
	{
		_BREADCRUMBITEM* pParentItem = (pItem == NULL) ? &(GetRootItem ()) : pItem->pParent;
		if (pParentItem == NULL) return;

		m_bGettingDynamicSubItems = true; // prevent adding dynamic sub-items
		int i = GetSubitemsCount (*pParentItem);
		m_bGettingDynamicSubItems = false;

		while (--i >= 0)
		{
			_BREADCRUMBITEM* pSubItem = GetSubitem (*pParentItem, i);
			if (pSubItem != NULL && !pSubItem->bDeleted)
			{
				if (!IsItemInSelectionPath (pSubItem))
				{
					FreeDynamicSubItemsRecursive (*pSubItem);
				}
			}
		}

		CleanupRemovedItems (); // Finally delete all marked items. This may cause sending WM_DELETEITEM multiple times.
	}

	bool FreeDynamicSubItemsRecursive (_BREADCRUMBITEM& item)
	{
		m_bGettingDynamicSubItems = true; // prevent adding dynamic sub-items
		int i = GetSubitemsCount (item);
		m_bGettingDynamicSubItems = false;

		bool bReleasedAll = true;
		while (--i >= 0)
		{
			_BREADCRUMBITEM* pSubItem = GetSubitem (item, i);
			if (pSubItem != NULL && !pSubItem->bDeleted)
			{
				if (!FreeDynamicSubItemsRecursive (*pSubItem))
				{
					bReleasedAll = false;
				}
			}
		}

		if (bReleasedAll && item.bDynamic && item.bHasData)
		{
			item.bHasData = false;
			RemoveSubitems (item);
		}

		return bReleasedAll;
	}

	bool SetItemInfo (_BREADCRUMBITEM& item, const BREADCRUMBITEMINFO* info, bool bUseMask = true)
	{
		if (info == NULL)
		{
			return false;
		}

		UINT mask = bUseMask ? info->mask : 0xFFFFFFFF;

		bool bRet = false;
		bool bRecalcLayout = false;
		bool bItemInActivePath = IsItemInSelectionPath (&item);

		if (mask & BCCIF_TEXT)
		{
			item.strText = info->pszText;
			bRecalcLayout = true;
			bRet = true;
		}
		if (mask & BCCIF_IMAGE)
		{
			item.iImage = info->iImage;
			Invalidate ();
			bRet = true;
		}
		if (mask & BCCIF_DYNAMIC)
		{
			if (item.bDynamic && !info->bDynamic) //removing bDynamic flag
			{
				FreeDynamicSubItemsRecursive (item);
			}
			else if (!item.bDynamic && info->bDynamic) //setting bDynamic flag
			{
				item.bDynamic = true;
				item.bHasData = false;
			}

			if (bItemInActivePath)
			{
				SetSelection (&item);
			}
			Invalidate ();
			bRet = true;
		}

		if (mask & BCCIF_PARAM)
		{
			item.lParam = info->lParam;
			Invalidate ();
			bRet = true;
		}

		if (!bUseMask) // for new items only
		{
			_BREADCRUMBITEM* pNewParent = (info->hParentItem == NULL) ? &(GetRootItem ()) : ItemFromHandle (info->hParentItem);
			if (pNewParent != NULL)
			{
				if (item.pParent != pNewParent)
				{
					item.pParent = pNewParent;
					bRecalcLayout = true;
					bRet = true;
				}
			}
		}

		if (mask & BCCIF_TOOLTIP)
		{
			item.strTooltip = info->pszTooltipText;
			bRet = true;
		}

		if (bRecalcLayout)
		{
			CalculateLayout ();
		}

		return bRet;
	}

	static bool GetItemInfo (BREADCRUMBITEMINFO* info, const _BREADCRUMBITEM& item, bool bUseMask = true)
	{
		if (info == NULL)
		{
			return false;
		}

		if (!bUseMask || (info->mask & BCCIF_TEXT))
		{
			if (info->pszText == NULL) // query text length in cchTextMax
			{
				info->cchTextMax = item.strText.GetLength () + 1;
			}
			else if (info->cchTextMax > 0)
			{
				if (::lstrcpyn (info->pszText, item.strText, info->cchTextMax) == NULL)
				{
					info->pszText[info->cchTextMax - 1] = 0;
				}
			}
		}
		if (!bUseMask || (info->mask & BCCIF_IMAGE))
		{
			info->iImage = item.iImage;
		}
		if (!bUseMask || (info->mask & BCCIF_DYNAMIC))
		{
			info->bDynamic = item.bDynamic;
		}
		if (!bUseMask || (info->mask & BCCIF_PARAM))
		{
			info->lParam = item.lParam;
		}
		if (!bUseMask || (info->mask & BCCIF_PARENT))
		{
			info->hParentItem = (HBREADCRUMBITEM)item.pParent;
		}
		if (!bUseMask || (info->mask & BCCIF_TOOLTIP))
		{
			if (info->pszTooltipText == NULL) // query text length in cchTooltipTextMax
			{
				info->cchTooltipTextMax = item.strTooltip.GetLength () + 1;
			}
			else if (info->cchTooltipTextMax > 0)
			{
				if (::lstrcpyn (info->pszTooltipText, item.strTooltip, info->cchTooltipTextMax) == NULL)
				{
					info->pszTooltipText[info->cchTooltipTextMax - 1] = 0;
				}
			}
		}
		return true;
	}

	
	static void GetFullItemInfo (BREADCRUMBITEMINFO* pItemInfo, const _BREADCRUMBITEM& item, CString& strText, CString& strTooltip)
	{
		if (pItemInfo == NULL) return;
		pItemInfo->mask = BCCIF_PARENT | BCCIF_TEXT | BCCIF_TOOLTIP | BCCIF_IMAGE | BCCIF_DYNAMIC | BCCIF_PARAM;
		strText = item.strText;
		strTooltip = item.strTooltip;
		pItemInfo->cchTextMax = strText.GetLength ();
		pItemInfo->pszText = strText.GetBuffer (pItemInfo->cchTextMax);
		pItemInfo->cchTooltipTextMax = strTooltip.GetLength ();
		pItemInfo->pszTooltipText = strTooltip.GetBuffer (pItemInfo->cchTooltipTextMax);

		pItemInfo->iImage = item.iImage;
		pItemInfo->bDynamic = item.bDynamic;
		pItemInfo->lParam = item.lParam;
		pItemInfo->hParentItem = (HBREADCRUMBITEM)item.pParent;
	}

	_BREADCRUMBITEM& GetRootItem ()
	{
		if (m_Items.IsEmpty ())
		{
			// Create root item if it does not exist yet
			_BREADCRUMBITEM root;
			root.strText = _T(".");
			m_Items.AddHead (root);
		}

		_BREADCRUMBITEM& item = m_Items.GetHead ();
		item.bDeleted = false;  // root item cannot be deleted
		item.pParent = NULL;    // root item can have no parent
		return item;
	}

	// Translates incoming HBREADCRUMBITEM handle into _BREADCRUMBITEM*.
	// Ensures that the item is in the list and is not marked as deleted.
	_BREADCRUMBITEM* ItemFromHandle (HBREADCRUMBITEM handle)
	{
		if (handle == NULL)
		{
			return NULL;
		}

		POSITION pos = m_Items.GetHeadPosition ();
		while (pos != NULL)
		{
			_BREADCRUMBITEM* pItem = &(m_Items.GetNext (pos));
			if (pItem == (_BREADCRUMBITEM*)handle && !pItem->bDeleted)
			{
				return pItem;
			}
		}

		return NULL;
	}

	int GetSubitemsCount (_BREADCRUMBITEM& item)
	{
		if (item.bDynamic && !item.bHasData && !m_bGettingDynamicSubItems)
		{
			GetDynamicSubItems (item);
		}

		int nCount = 0;
		POSITION pos = m_Items.GetHeadPosition ();
		while (pos != NULL)
		{
			_BREADCRUMBITEM* pItem = &(m_Items.GetNext (pos));
			if (!pItem->bDeleted && pItem->pParent == &item)
			{
				nCount ++;
			}
		}
		return nCount;
	}

	_BREADCRUMBITEM* GetSubitem (const _BREADCRUMBITEM& item, int index)
	{
		int nCurrentIndex = 0;
		POSITION pos = m_Items.GetHeadPosition ();
		while (pos != NULL)
		{
			_BREADCRUMBITEM* pItem = &(m_Items.GetNext (pos));
			if (!pItem->bDeleted && pItem->pParent == &item)
			{
				if (nCurrentIndex == index)
				{
					return pItem;
				}
				nCurrentIndex ++;
			}
		}
		return NULL;
	}

	void RemoveAllItems ()
	{
		RemoveItem (GetRootItem ());
		m_pSelectedItem = NULL;
		CleanupRemovedItems ();
		CalculateLayout ();
	}

	int RemoveItem (_BREADCRUMBITEM& item)
	{
		ASSERT (!item.bDeleted);
		if (item.bDeleted)
		{
			return 0; //already deleted
		}

		item.bDeleted = true;
		return 1 + RemoveSubitems (item);
	}

	int RemoveSubitems (const _BREADCRUMBITEM& parentItem)
	{
		int nRemoved = 0;
		POSITION pos = m_Items.GetHeadPosition ();
		while (pos != NULL)
		{
			_BREADCRUMBITEM& item = m_Items.GetNext (pos);
			if (!item.bDeleted && item.pParent == &parentItem)
			{
				ASSERT (&item != &parentItem); // Item cannot be a parent of itself
				if (&item != &parentItem)
				{
					nRemoved += RemoveItem (item);
				}
			}
		}
		return nRemoved;
	}

	void SetSelection (_BREADCRUMBITEM* pNewSelectedItem)
	{
		if (pNewSelectedItem != m_pSelectedItem && m_pSelectedItem != NULL)
		{
			m_pSelectedItem = pNewSelectedItem;
			CleanupDynamicSubItems (m_pSelectedItem);
			ValidateSelection ();
			CalculateLayout ();

			if (!m_bProcessingSelectionChange)
			{
				m_bProcessingSelectionChange = true;
				NotifyParent (BCCN_SELECTIONCHANGED);
				m_bProcessingSelectionChange = false;
			}
		}
	}

	// Returns true if an active item has changed.
	bool ValidateSelection ()
	{
		int nLevels = 0;
		int nItemCount = (int)m_Items.GetCount ();
		if (nItemCount == 0)
		{
			m_pSelectedItem = &(GetRootItem ());
			return true;
		}

		while (m_pSelectedItem != NULL && m_pSelectedItem->bDeleted)
		{
			m_pSelectedItem = m_pSelectedItem->pParent; 
			nLevels++;

			ASSERT (nLevels <= nItemCount); // Is there cyclic references in an items list?!

			if (nLevels > nItemCount)
			{
				// clean up broken contents
				RemoveAllItems ();
				m_pSelectedItem = &(GetRootItem ());
				return true;
			}
		}

		if (m_pSelectedItem == NULL)
		{
			m_pSelectedItem = &(GetRootItem ());
			return true;
		}

		return false;
	}

	bool IsItemInSelectionPath (const _BREADCRUMBITEM* pItem)
	{
		_BREADCRUMBITEM* pActive = m_pSelectedItem;
		_BREADCRUMBITEM* pRoot = &(GetRootItem ());
		while (pActive != NULL)
		{
			if (pActive == pItem) return true;
			pActive = pActive->pParent;
			if (pActive == pRoot) break;
		}
		return false;
	}

	// Removes all the items marked to delete
	void CleanupRemovedItems ()
	{
		POSITION pos = m_Items.GetHeadPosition ();
		while (pos != NULL)
		{
			POSITION posCurrent = pos;
			const _BREADCRUMBITEM& item = m_Items.GetNext (pos);
			if (item.bDeleted)
			{
				NotifyDeleteItem (item); // Send WM_DELETEITEM to parent if necessary
				m_Items.RemoveAt (posCurrent);
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//  Menu

	// returns false if root item was not inserted.
	bool MenuInsertRootItem (HMENU hMenu)
	{
		_BREADCRUMBITEM* pRoot = &(GetRootItem ());

		// Do not insert the root item if it has no text and BCCS_SHOWROOTALWAYS style not set.
		if (!IsWindowStylePresent (BCCS_SHOWROOTALWAYS) && pRoot->strText.IsEmpty ())
		{
			return false;
		}

		int index = (int)m_arrCurrentMenuItems.Add (pRoot);
		::AppendMenu (hMenu, MF_STRING | MF_ENABLED, (index + 1), pRoot->strText);
		return true;
	}

	void MenuInsertSubItems (HMENU hMenu, _BREADCRUMBITEM* pItem)
	{
		if (pItem == NULL)
		{
			return;
		}

		int nSubItems = GetSubitemsCount (*pItem);
		bool bNeedSeparator = ::GetMenuItemCount (hMenu) > 0;
		int iDefault = -1;
		for (int i = 0; i < nSubItems; ++i)
		{
			if (bNeedSeparator)
			{
				::AppendMenu (hMenu, MF_SEPARATOR, 0, NULL);
				bNeedSeparator = false;
			}

			_BREADCRUMBITEM* pSubItem = GetSubitem (*pItem, i);
			if (pSubItem != NULL)
			{
				int index = (int)m_arrCurrentMenuItems.Add (pSubItem) + 1;
				::AppendMenu (hMenu, MF_STRING | MF_ENABLED, index, pSubItem->strText);
				if (IsItemInSelectionPath (pSubItem))
				{
					iDefault = index;
				}
			}
		}
		if (iDefault >= 0)
		{
			::SetMenuDefaultItem (hMenu, iDefault, FALSE);
		}
	}

	void ShowLeftButtonMenu ()
	{
		HMENU hMenu = ::CreatePopupMenu ();
		int nInserted = 0;
		m_arrCurrentMenuItems.RemoveAll ();

		// Fill in parent items (max 19 levels)
		int nLevel = 19;
		_BREADCRUMBITEM* pRoot = &(GetRootItem ());
		_BREADCRUMBITEM* pItem = m_pSelectedItem;
		while (pItem != NULL && nLevel > 0)
		{
			if (!IsItemVisible (pItem))
			{
				if (pItem == pRoot)
				{
					MenuInsertRootItem (hMenu);
				}
				else
				{
					int index = (int)m_arrCurrentMenuItems.Add (pItem);
					::AppendMenu (hMenu, MF_STRING, (index + 1), pItem->strText);       
					nInserted ++;
				}
			}

			if (pItem == pRoot) break;

			pItem = pItem->pParent;
			nLevel --;
		}

		MenuInsertSubItems (hMenu, pRoot);

		m_bIsMenuDropped = true;

		m_uLeftMenuState = CDIS_SELECTED;
		Invalidate  ();

		RECT rectWindow;
		m_pWnd->GetWindowRect (&rectWindow);
		POINT ptMenuOrigin;
		ptMenuOrigin.y = rectWindow.bottom + 1;
		ptMenuOrigin.x = rectWindow.left + m_rectLeftButtonLayout.left;

		BREADCRUMB_MENU menuInfo;
		menuInfo.hItem = NULL;
		menuInfo.hMenu = hMenu;
		menuInfo.ptOrigin = ptMenuOrigin;
		if (NotifyParent (menuInfo.nmhdr, BCCN_MENUSHOW) == 0)
		{
			if (::GetMenuItemCount (hMenu) > 0)
			{
				TPMPARAMS tpp;
				tpp.cbSize = sizeof (tpp);
				tpp.rcExclude.left = 0;
				tpp.rcExclude.right = ::GetSystemMetrics (SM_CXSCREEN);
				tpp.rcExclude.top = rectWindow.top - 1;
				tpp.rcExclude.bottom = rectWindow.bottom + 1;

#ifndef _BCGSUITE_
				CBCGPContextMenuManager* pMenuManager = g_pContextMenuManager;
#else
				CWinAppEx* pApp = DYNAMIC_DOWNCAST(CWinAppEx, AfxGetApp());
				CContextMenuManager* pMenuManager = (pApp != NULL) ? pApp->GetContextMenuManager () : NULL;
#endif

				if (pMenuManager != NULL)
				{
					UINT iCmd = pMenuManager->TrackPopupMenu (hMenu, menuInfo.ptOrigin.x, menuInfo.ptOrigin.y, m_pWnd);
					if (iCmd != 0)
					{
						m_pWnd->SendMessage (WM_COMMAND, MAKEWPARAM (iCmd, 0), 0);
					}
				}
				else
				{
					::TrackPopupMenuEx (hMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON, menuInfo.ptOrigin.x, menuInfo.ptOrigin.y, m_pWnd->GetSafeHwnd (), &tpp);
				}

				NotifyParent (menuInfo.nmhdr, BCCN_MENUCLOSE);
			}
		}

		m_bIsMenuDropped = false;
		m_uLeftMenuState = CDIS_DEFAULT;
		Invalidate ();
	}

	void ShowItemMenu (_BREADCRUMBITEM* pItem)
	{
		if (pItem == NULL)
		{
			return;
		}

		HMENU hMenu = ::CreatePopupMenu ();
		m_arrCurrentMenuItems.RemoveAll ();

		if (pItem == &(GetRootItem ()))
		{
			if (!IsItemVisible (pItem))
			{
				MenuInsertRootItem (hMenu);
			}
		}

		MenuInsertSubItems (hMenu, pItem);

		m_bIsMenuDropped = true;
		m_pPressedItem = pItem;
		Invalidate ();


		RECT rectWindow;
		m_pWnd->GetWindowRect (&rectWindow);
		POINT ptMenuOrigin;
		ptMenuOrigin.x = rectWindow.left;
		ptMenuOrigin.y = rectWindow.bottom + 1;

		int index = LayoutIndexFromItem (pItem);
		if (index >= 0 && index < (int)m_arrItemsLayout.GetSize ())
		{
			ptMenuOrigin.x = rectWindow.left + m_arrItemsLayout[index].rectArrow.left - 18; // See 5.14 in requirements
		}

		BREADCRUMB_MENU menuInfo;
		menuInfo.hItem = NULL;
		menuInfo.hMenu = hMenu;
		menuInfo.ptOrigin = ptMenuOrigin;
		if (NotifyParent (menuInfo.nmhdr, BCCN_MENUSHOW) == 0)
		{
			if (::GetMenuItemCount (hMenu) > 0)
			{
				TPMPARAMS tpp;
				tpp.cbSize = sizeof (tpp);
				tpp.rcExclude.left = 0;
				tpp.rcExclude.right = ::GetSystemMetrics (SM_CXSCREEN);
				tpp.rcExclude.top = rectWindow.top - 1;
				tpp.rcExclude.bottom = rectWindow.bottom + 1;

#ifndef _BCGSUITE_
				CBCGPContextMenuManager* pMenuManager = g_pContextMenuManager;
#else
				CWinAppEx* pApp = DYNAMIC_DOWNCAST(CWinAppEx, AfxGetApp());
				CContextMenuManager* pMenuManager = (pApp != NULL) ? pApp->GetContextMenuManager () : NULL;
#endif

				if (pMenuManager != NULL)
				{
					UINT iCmd = pMenuManager->TrackPopupMenu (hMenu, menuInfo.ptOrigin.x, menuInfo.ptOrigin.y, m_pWnd);
					if (iCmd != 0)
					{
						m_pWnd->SendMessage (WM_COMMAND, MAKEWPARAM (iCmd, 0), 0);
					}
				}
				else
				{
					::TrackPopupMenuEx (hMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON, menuInfo.ptOrigin.x, menuInfo.ptOrigin.y, m_pWnd->GetSafeHwnd (), &tpp);
				}

				NotifyParent (menuInfo.nmhdr, BCCN_MENUCLOSE);
			}
		}

		m_bIsMenuDropped = false;
		m_pPressedItem = NULL;
		Invalidate ();
	}

	//////////////////////////////////////////////////////////////////////////
	//                      Rendering control
	//////////////////////////////////////////////////////////////////////////

	void PaintControl (HDC hDC)
	{
		CRect rectControl;
		m_pWnd->GetClientRect (&rectControl);

		// Double-buffering and Glass rendering
		CDC tempDC;
		tempDC.Attach (hDC);

		if (rectControl.IsRectEmpty ())
		{
			return;
		}

		CBCGPAlphaDC alphaDC (tempDC, rectControl, 1.0);
		alphaDC.IgnoreSourceAlpha ();

		CDC& dc = alphaDC;

		tempDC.Detach ();

		m_clrText = FillControlBackground (dc, rectControl);

		bool bSuppressSelection = false;
		if (!m_bIsMenuDropped)
		{
			// Suppress the CDIS_SELECTED style when cursor is out of pressed item area.
			POINT pt;
			::GetCursorPos (&pt);
			m_pWnd->ScreenToClient (&pt);

			UINT uHitTest = 0;
			if (m_pPressedItem != HitTest (pt, uHitTest))
			{
				bSuppressSelection = true;
			}
		}

		if (!m_rectIconLayout.IsRectEmpty () && m_hImageList != NULL && m_pSelectedItem != NULL)
		{
			int iIndex = (m_pSelectedItem != NULL) ? m_pSelectedItem->iImage : -1;
			UINT uState = (m_uHilitedHitTest == BCCHT_ICON) ? CDIS_HOT : CDIS_DEFAULT;
			if (m_pPressedItem == &(GetRootItem ()))
			{
				uState = bSuppressSelection ? CDIS_DEFAULT : CDIS_SELECTED;
			}

			DrawIcon (dc, m_hImageList, m_rectIconLayout, iIndex, uState);
		}

		// Drawing left menu button (if present)

		UINT uState = m_uLeftMenuState;
		if (uState != CDIS_SELECTED && m_uHilitedHitTest == BCCHT_MENUBUTTON)
		{
			uState = CDIS_HOT;
		}
		if (bSuppressSelection && uState == CDIS_SELECTED)
		{
			uState = CDIS_DEFAULT;
		}

		DrawLeftArrowButtonBackground (dc, m_rectLeftButtonLayout, uState);
		DrawLeftArrowButton (dc, m_rectLeftButtonLayout, uState);

		HFONT hFontOld = SelectCurrentFont (dc);
		::SetBkMode (dc, TRANSPARENT);

		int i;
		for (i = 0; i < (int)m_arrItemsLayout.GetSize (); ++i)
		{
			bool bHilited = (i == m_iHilitedIndex);
			bool bItemHilited = bHilited && (m_uHilitedHitTest == BCCHT_ITEM);
			bool bArrowHilited = bHilited && (m_uHilitedHitTest == BCCHT_ITEMBUTTON);

			UINT uItemState = (m_arrItemsLayout[i].pItem == m_pPressedItem) ? CDIS_SELECTED : CDIS_DEFAULT;
			UINT uArrowState = uItemState;
			if (bHilited && uItemState != CDIS_SELECTED)
			{
				uItemState = bArrowHilited ? CDIS_OTHERSIDEHOT : CDIS_HOT;
				uArrowState = bItemHilited ? CDIS_OTHERSIDEHOT : CDIS_HOT; // The arrow can also be "grayed" depending on visual theme.
			}

			if (bSuppressSelection && uItemState == CDIS_SELECTED)
			{
				uItemState = CDIS_DEFAULT;
			}
			if (bSuppressSelection && uArrowState == CDIS_SELECTED)
			{
				uArrowState = CDIS_DEFAULT;
			}

			CRect rectItem = m_arrItemsLayout[i].rectItem;
			CRect rectArrow = m_arrItemsLayout[i].rectArrow;
			BOOL bItemRectEmpty = rectItem.IsRectEmpty ();
			BOOL bArrowRectEmpty = rectArrow.IsRectEmpty ();

			// for root item when it is not visible
			if (bItemRectEmpty && uItemState == CDIS_HOT)
			{
				uArrowState = CDIS_HOT;
			}

			BREADCRUMBITEMINFO itemInfoParam;
			CString strText, strTooltip; // temporary storage for string and tooltip
			GetFullItemInfo (&itemInfoParam, *m_arrItemsLayout[i].pItem, strText, strTooltip);

			if (!bItemRectEmpty)
			{
				DrawItemBackground (dc, &itemInfoParam, rectItem, uItemState);
				DrawItem (dc, &itemInfoParam, rectItem, uItemState);
			}

			if (!bArrowRectEmpty)
			{
				if (!bItemRectEmpty && rectArrow.left > 1)
				{
					rectArrow.left --;
				}
				DrawArrowBackground (dc, &itemInfoParam, rectArrow, uArrowState);
				DrawArrow (dc, &itemInfoParam, rectArrow, uArrowState);
			}
		}

		::SelectObject (dc, hFontOld);
	}

	HFONT SelectCurrentFont (HDC hdc)
	{
		HFONT hFont = m_hFont;
		if (m_hFont == NULL)
		{
			hFont = (HFONT)::GetStockObject (DEFAULT_GUI_FONT);
		}
		return (HFONT)::SelectObject (hdc, hFont);
	}

	COLORREF TranslateColor (COLORREF clr, int sysDefaultColor)
	{
		if (clr == CLR_DEFAULT || clr == CLR_INVALID)
		{
			clr = ::GetSysColor (sysDefaultColor);
		}
		return clr;
	}

	// Control rendering

	COLORREF FillControlBackground (CDC& dc, RECT rect)
	{
		return CBCGPVisualManager::GetInstance ()->BreadcrumbFillBackground (dc, m_pWnd, rect);
	}

	void DrawItemBackground (CDC& dc, BREADCRUMBITEMINFO* pItemInfo, RECT rect, UINT uState)
	{
		if (::IsRectEmpty (&rect)) return;
		CBCGPVisualManager::GetInstance ()->BreadcrumbDrawItemBackground (dc, m_pWnd, pItemInfo, rect, uState, TranslateColor (m_clrHighlight, COLOR_HIGHLIGHT));
	}

	void DrawItem (CDC& dc, BREADCRUMBITEMINFO* pItemInfo, RECT rect, UINT uState)
	{
		if (pItemInfo != NULL && !::IsRectEmpty (&rect))
		{
			COLORREF clrText;
			if (uState == CDIS_HOT || uState == CDIS_SELECTED || uState == CDIS_OTHERSIDEHOT)
			{
				clrText = TranslateColor (m_clrHighlightText, COLOR_HIGHLIGHTTEXT);
			}
			else
			{
				clrText = TranslateColor (m_clrText, COLOR_WINDOWTEXT);
			}

			CBCGPVisualManager::GetInstance ()->BreadcrumbDrawItem (dc, m_pWnd, pItemInfo, rect, uState, clrText);
		}
	}

	void DrawArrowBackground (CDC& dc, BREADCRUMBITEMINFO* pItemInfo, RECT rect, UINT uState)
	{
		if (::IsRectEmpty (&rect)) return;
		CBCGPVisualManager::GetInstance ()->BreadcrumbDrawArrowBackground (dc, m_pWnd, pItemInfo, rect, uState, TranslateColor (m_clrHighlight, COLOR_HIGHLIGHT));
	}

	void DrawArrow (CDC& dc, BREADCRUMBITEMINFO* pItemInfo, RECT rect, UINT uState)
	{
		if (pItemInfo != NULL && !::IsRectEmpty (&rect))
		{
			COLORREF clrArrow;

			if (!m_pWnd->IsWindowEnabled())
			{
#ifdef _BCGSUITE_
				clrArrow = afxGlobalData.clrGrayedText;
#else
				clrArrow = globalData.clrGrayedText;
#endif
			}
			else if (uState == CDIS_HOT || uState == CDIS_SELECTED || uState == CDIS_OTHERSIDEHOT)
			{
				clrArrow = TranslateColor (m_clrHighlightText, COLOR_HIGHLIGHTTEXT);
			}
			else
			{
				clrArrow = TranslateColor (m_clrText, COLOR_WINDOWTEXT);
			}

			CBCGPVisualManager::GetInstance ()->BreadcrumbDrawArrow (dc, m_pWnd, pItemInfo, rect, uState, clrArrow);
		}
	}

	void DrawLeftArrowButtonBackground (CDC& dc, RECT rect, UINT uState)
	{
		if (::IsRectEmpty (&rect)) return;
		CBCGPVisualManager::GetInstance ()->BreadcrumbDrawLeftArrowBackground (dc, m_pWnd, rect, uState, TranslateColor (m_clrHighlight, COLOR_HIGHLIGHT));
	}

	void DrawLeftArrowButton (CDC& dc, const RECT& rect, UINT uState)
	{
		if (::IsRectEmpty (&rect)) return;
		COLORREF clrArrows;
		if (uState == CDIS_HOT || uState == CDIS_SELECTED || uState == CDIS_OTHERSIDEHOT)
		{
			clrArrows = TranslateColor (m_clrHighlightText, COLOR_HIGHLIGHTTEXT);
		}
		else
		{
			clrArrows = TranslateColor (m_clrText, COLOR_WINDOWTEXT);
		}

		CBCGPVisualManager::GetInstance ()->BreadcrumbDrawLeftArrow (dc, m_pWnd, rect, uState, clrArrows);
	}

	void DrawIcon (CDC& dc, HIMAGELIST hImageList, RECT rectIcon, int iImageIndex, UINT uState)
	{
		if (!::IsRectEmpty (&m_rectIconLayout) && m_hImageList != NULL && m_pSelectedItem != NULL)
		{
			// Drawing an icon
			if (uState == CDIS_SELECTED)
			{
				rectIcon.left ++;
				rectIcon.top ++;
			}
			ImageList_Draw (hImageList, iImageIndex, dc.GetSafeHdc (), rectIcon.left, rectIcon.top, ILD_TRANSPARENT);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// Breadcrumb state 

	// Updates the highlight of control regarding of current mouse cursor position.
	void UpdateHilite ()
	{
		POINT pt;
		::GetCursorPos (&pt);

		// Prevent updating highlight if the mouse cursor is over the menu.
		if (m_bIsMenuDropped && ::WindowFromPoint (pt) != m_pWnd->GetSafeHwnd ())
		{
			return;
		}

		m_pWnd->ScreenToClient (&pt);
		UINT uHitTest = 0;
		_BREADCRUMBITEM* pItem = HitTest (pt, uHitTest);
		int iHilitedIndex = (uHitTest == BCCHT_MENUBUTTON) ? -1 : LayoutIndexFromItem (pItem);

		bool bUpdate = true;
		
		// Do not highlight items until left mouse button pressed.
		if (m_pPressedItem != NULL && !m_bIsMenuDropped)
		{
			if (pItem != m_pPressedItem)
			{
				RemoveHilite ();
				return;
			}
		}

		if (CWnd::GetFocus () == m_pWnd)
		{
			// The highlight will update only if mouse hovers the items or leaves the control.
			bUpdate = (uHitTest != BCCHT_EMPTY) && (uHitTest != BCCHT_ICON) && (uHitTest != BCCHT_EDIT);
		}

		if (bUpdate)
		{
			SetHilite (iHilitedIndex, uHitTest);

			if (m_pPressedItem != NULL)
			{
				Invalidate ();
			}
		}
	}

	void CancelMenu ()
	{
		::DefWindowProc (m_pWnd->GetSafeHwnd (), WM_CANCELMODE, 0, 0); // end menu

#ifndef _BCGSUITE_
		if (g_pContextMenuManager != NULL)
		{
			CBCGPPopupMenu* pActiveMenu = CBCGPPopupMenu::GetActiveMenu ();
			if (pActiveMenu != NULL)
			{
				pActiveMenu->CloseMenu ();
			}
		}
#endif
	}

	// Replaces the current highlight with the new one.
	// Invalidates corresponding areas.
	void SetHilite (int iHilitedItemIndex, UINT uHitTest)
	{
		if (m_bIsMenuDropped && (iHilitedItemIndex != m_iHilitedIndex))
		{
			// If the menu is active, cancel it.
			if (iHilitedItemIndex >= -1 && iHilitedItemIndex < (int)m_arrItemsLayout.GetSize ())
			{
				CancelMenu ();
				m_bNeedShowPopupMenuAgain = true;
			}
			else return;
		}

		CRect rectHilite = GetCurrentHilitedItemRect ();
		if (!rectHilite.IsRectEmpty ())
		{
			m_pWnd->InvalidateRect (&rectHilite, TRUE);
		}
		
		m_iHilitedIndex = iHilitedItemIndex;
		m_uHilitedHitTest = uHitTest;
		if (m_iHilitedIndex < 0 || m_iHilitedIndex >= (int)m_arrItemsLayout.GetSize ())
		{
			// Validate m_uHilitedHitTest
			if (m_uHilitedHitTest == BCCHT_ITEM || m_uHilitedHitTest == BCCHT_ITEMBUTTON)
			{
				m_uHilitedHitTest = BCCHT_EMPTY;
			}
		}

		rectHilite = GetCurrentHilitedItemRect ();
		if (!rectHilite.IsRectEmpty ())
		{
			m_pWnd->InvalidateRect (&rectHilite, TRUE);
		}
		else
		{
			// Hide tool tips
			if (::IsWindow (m_hwndTooltips))
			{
				::SendMessage (m_hwndTooltips, TTM_POP, 0, 0);
			}

			m_pWnd->GetClientRect (&rectHilite);
		}

		m_MouseTracker.TrackRect (m_pWnd->GetSafeHwnd (), rectHilite);
	}

	void HiliteItem (_BREADCRUMBITEM* pItem, bool bArrow = false)
	{
		int iHiliteIndex = LayoutIndexFromItem (pItem);
		SetHilite (iHiliteIndex, (bArrow) ? BCCHT_ITEMBUTTON : BCCHT_ITEM);
	}

	void HiliteLeftMenuButton ()
	{
		SetHilite (-1, BCCHT_MENUBUTTON);
	}

	void RemoveHilite ()
	{
		SetHilite (-2, BCCHT_EMPTY);
		Invalidate ();
	}

	static RECT UnionRects (const RECT& rect1, const RECT& rect2)
	{
		RECT rect = {0, 0, 0, 0};
		if (::IsRectEmpty (&rect1)) return rect2;
		if (::IsRectEmpty (&rect2)) return rect1;
		::UnionRect (&rect, &rect1, &rect2);
		return rect;
	}

	RECT GetCurrentHilitedItemRect () const
	{
		RECT rect = {0, 0, 0, 0};
		switch (m_uHilitedHitTest)
		{
		case BCCHT_ICON:
		case BCCHT_MENUBUTTON:
			rect = UnionRects (m_rectIconLayout, m_rectLeftButtonLayout);
			break;
		case BCCHT_ITEM:
		case BCCHT_ITEMBUTTON:
			if (m_iHilitedIndex >= 0 || m_iHilitedIndex < (int)m_arrItemsLayout.GetSize ())
			{
				rect = UnionRects (m_arrItemsLayout[m_iHilitedIndex].rectItem, m_arrItemsLayout[m_iHilitedIndex].rectArrow);
			}
			break;
		}

		// Include one pixel for border.
		rect.left --;
		rect.top --;
		rect.right ++;
		rect.bottom ++;

		return rect;
	}

	_BREADCRUMBITEM* GetCurrentHilitedItem () const
	{
		if (m_uHilitedHitTest == BCCHT_ITEMBUTTON || m_uHilitedHitTest == BCCHT_ITEM)
		{
			if (m_iHilitedIndex >= 0 || m_iHilitedIndex < (int)m_arrItemsLayout.GetSize ())
			{
				return m_arrItemsLayout[m_iHilitedIndex].pItem;
			}
		}

		return NULL;
	}

	bool MoveHiliteHome (bool bSkipLeftMenuButton = false)
	{
		if (!m_rectLeftButtonLayout.IsRectEmpty () && !bSkipLeftMenuButton)
		{
			HiliteLeftMenuButton ();
			return true;
		}

		int n = (int)m_arrItemsLayout.GetSize ();
		for (int i = 0; i < n; ++i)
		{
			if (m_arrItemsLayout[i].pItem != NULL)
			{
				if (!m_arrItemsLayout[i].rectItem.IsRectEmpty ())
				{
					HiliteItem (m_arrItemsLayout[i].pItem, false);
					return true;
				}
				if (!m_arrItemsLayout[i].rectArrow.IsRectEmpty ())
				{
					HiliteItem (m_arrItemsLayout[i].pItem, true);
					return true;
				}
			}
		}

		return false;
	}

	bool MoveHiliteLeft ()
	{
		bool bLeftButtonMenuVisible = !m_rectLeftButtonLayout.IsRectEmpty ();

		int newIndex = m_iHilitedIndex - 1;
		if (newIndex == -1 && bLeftButtonMenuVisible)
		{
			HiliteLeftMenuButton ();
			return true;
		}

		if (newIndex < 0)
		{
			HiliteItem (m_pSelectedItem); // go to rightmost item
			return true;
		}

		if (newIndex < (int)m_arrItemsLayout.GetSize ())
		{
			HiliteItem (m_arrItemsLayout[newIndex].pItem);
			return true;
		}

		return false;
	}

	bool MoveHiliteRight ()
	{
		if (m_iHilitedIndex < -1)
		{
			return MoveHiliteHome (false);
		}

		int newIndex = m_iHilitedIndex + 1;
		if (newIndex >= (int)m_arrItemsLayout.GetSize ())
		{
			return MoveHiliteHome (false);
		}

		HiliteItem (m_arrItemsLayout[newIndex].pItem);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// Inplace editing

	void BeginInplaceEdit ()
	{
		RemoveHilite ();
		NotifyParent (BCCN_BEGIN_INPLACE_EDITING);
	}

	//////////////////////////////////////////////////////////////////////////
	// Helper methods

	BOOL IsWindowStylePresent (DWORD dwStyle) const
	{
		return (dwStyle & (DWORD)::GetWindowLong (m_pWnd->GetSafeHwnd (), GWL_STYLE)) != 0;
	}

	void Invalidate ()
	{
		m_pWnd->InvalidateRect (NULL, TRUE);
	}

	LRESULT NotifyParent (UINT code)
	{
		NMHDR nmhdrSimple;
		return NotifyParent (nmhdrSimple, code);
	}

	LRESULT NotifyParent (NMHDR& nmhdr, UINT code)
	{
		nmhdr.hwndFrom = m_pWnd->GetSafeHwnd ();
		nmhdr.idFrom = m_pWnd->GetDlgCtrlID ();
		nmhdr.code = code;
		return m_pWnd->GetParent ()->SendMessage (WM_NOTIFY, (WPARAM)nmhdr.idFrom, (LPARAM)&nmhdr);
	}

private:
	CBCGPBreadcrumb*            m_pWnd;

	HIMAGELIST                  m_hImageList;
	HFONT                       m_hFont;
	HWND                        m_hwndTooltips;

	// Appearance
	UINT                        m_cxRightMargin;
	COLORREF                    m_clrText;
	COLORREF                    m_clrBackground;
	COLORREF                    m_clrHighlight;
	COLORREF                    m_clrHighlightText;

	// Items
	CList<_BREADCRUMBITEM, const _BREADCRUMBITEM&>  m_Items;
	_BREADCRUMBITEM*            m_pSelectedItem;

	// Layout
	CRect                       m_rectIconLayout;
	CArray<LAYOUT_ITEM, const LAYOUT_ITEM&>  m_arrItemsLayout;
	CRect                       m_rectLeftButtonLayout;
	bool                        m_bRootItemVisible;
	UINT                        m_uLeftMenuState;

	// State
	bool                        m_bNeedRecalcLayout;
	bool                        m_bCalculatingLayout;
	bool                        m_bGettingDynamicSubItems;
	bool                        m_bIsMenuDropped;
	bool                        m_bIsMenuClosedByMouseButton; // Prevents the menu from being reopened by clicking the arrow button
	bool                        m_bNeedShowPopupMenuAgain;
	bool                        m_bProcessingSelectionChange;
	CArray<_BREADCRUMBITEM*, _BREADCRUMBITEM*>  m_arrCurrentMenuItems;
	int                         m_iHilitedIndex; // -1 means that left menu button highlighted; -2 if no items highlited.
	UINT                        m_uHilitedHitTest;
	_BREADCRUMBITEM*            m_pPressedItem; // If not NULL, specifies the item on which the user has clicked (but not yet released the button). If the m_bIsMenuDropped not set to true, specifies that the mouse has been captured.
	CMousePosTracker            m_MouseTracker;
};

CBCGPBreadcrumbImpl* AttachBreadcrumbImplementation (CWnd* pAttachTo)
{
	if (pAttachTo != NULL)
	{
		return new CBCGPBreadcrumbImpl (pAttachTo->GetSafeHwnd ());
	}
	return NULL;
}

LRESULT BreadcrumbWindowProc (CBCGPBreadcrumbImpl* pImplementation, UINT message, WPARAM wParam, LPARAM lParam, BOOL* bHandled)
{
	ASSERT (pImplementation != NULL);
	if (pImplementation == NULL) return 0;

	LRESULT lResult = pImplementation->WindowProc (message, wParam, lParam, bHandled);

	if (message == WM_NCDESTROY)
	{
		delete pImplementation;
	}

	return lResult;
}
