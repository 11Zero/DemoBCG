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

// BCGPLayout.cpp: implementation of the CBCGPLayout class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BCGPLayout.h"
#include "BCGPDialog.h"
#include "BCGPDialogBar.h"
#include "BCGPPropertySheet.h"
#include "BCGPPropertyPage.h"
#include "BCGPFormView.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CBCGPControlsLayout, CObject)

CBCGPControlsLayout::CBCGPControlsLayout()
	: m_pHostWnd   (NULL)
	, m_pHostLayout(NULL)
	, m_MinSize    (0, 0)
{
}

CBCGPControlsLayout::~CBCGPControlsLayout()
{
}

void CBCGPControlsLayout::GetHostWndRect (CRect& rect) const
{
	if (m_pHostWnd->GetSafeHwnd () != NULL)
	{
		m_pHostWnd->GetClientRect (rect);

		if (DYNAMIC_DOWNCAST(CBCGPPropertyPage, m_pHostWnd) != NULL)
		{
			CBCGPPropertySheet* pParent = DYNAMIC_DOWNCAST(CBCGPPropertySheet, m_pHostWnd->GetParent ());
			if (pParent != NULL)
			{
				int nNavigatorWidth = pParent->GetNavBarWidth();
				int nHeaderHeight = pParent->GetHeaderHeight();

				if (nHeaderHeight > 0 && !pParent->m_bDrawHeaderOnAeroCaption)
				{
					rect.top += nHeaderHeight;

					if (pParent->GetLook() != CBCGPPropertySheet::PropSheetLook_Tabs)
					{
						rect.bottom -= nHeaderHeight;
					}
				}

				rect.left += nNavigatorWidth;
			}

			rect.OffsetRect (-rect.TopLeft ());
		}
		else if (DYNAMIC_DOWNCAST(CBCGPDialogBar, m_pHostWnd) != NULL)
		{
			CPoint ptScroll(((CBCGPDialogBar*)m_pHostWnd)->GetScrollPos());
			rect.InflateRect (0 ,0, ptScroll.x, ptScroll.y);
			rect.OffsetRect (-ptScroll.x, -ptScroll.y);
		}
		else if (DYNAMIC_DOWNCAST(CBCGPFormView, m_pHostWnd) != NULL)
		{
			CPoint ptScroll(((CBCGPFormView*)m_pHostWnd)->GetScrollPos(SB_HORZ),
				((CBCGPFormView*)m_pHostWnd)->GetScrollPos(SB_VERT));
			rect.InflateRect (0 ,0, ptScroll.x, ptScroll.y);
			rect.OffsetRect (-ptScroll.x, -ptScroll.y);
		}

		rect.right  = rect.left + max(m_MinSize.cx, rect.Width ());
		rect.bottom = rect.top + max(m_MinSize.cy, rect.Height ());
	}
	else
	{
		rect = CRect(0, 0, 0, 0);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// CBCGPStaticLayout

BOOL CBCGPStaticLayout::m_bDontCheckFormViewInitDlg = FALSE;

IMPLEMENT_DYNCREATE(CBCGPStaticLayout, CBCGPControlsLayout)

CBCGPStaticLayout::CBCGPStaticLayout()
{
}

CBCGPStaticLayout::~CBCGPStaticLayout()
{
}

BOOL CBCGPStaticLayout::Create(CWnd* pHostWnd)
{
	if (pHostWnd->GetSafeHwnd () == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}
	
	m_pHostWnd = pHostWnd;

	return TRUE;
}

void CBCGPStaticLayout::AdjustLayout ()
{
	const int count = (int)m_listWnd.GetCount ();
	if (count == 0)
	{
		return;
	}

	HDWP hDWP = ::BeginDeferWindowPos (count);

	POSITION pos = m_listWnd.GetHeadPosition ();
	while (pos != NULL)
	{
		XWndItem& item = m_listWnd.GetNext (pos);
		HWND hwnd = (HWND)item.m_Handle;

		if (!::IsWindow (hwnd))
		{
			continue;
		}

		CRect rectItem;
		UINT uiFlags = CalculateItem (item, rectItem);

		if ((uiFlags & (SWP_NOMOVE | SWP_NOSIZE)) != (SWP_NOMOVE | SWP_NOSIZE))
		{
			::DeferWindowPos (hDWP, hwnd, HWND_TOP, rectItem.left, rectItem.top, 
				rectItem.Width (), rectItem.Height (), 
				uiFlags | SWP_NOZORDER | SWP_NOREPOSITION | SWP_NOACTIVATE | SWP_NOCOPYBITS);
		}
	}

	::EndDeferWindowPos (hDWP);
}

BOOL CBCGPStaticLayout::AddAnchor (UINT nID, XMoveType typeMove, XSizeType typeSize, 
	const CPoint& percMove, const CPoint& percSize)
{
	if (m_pHostWnd->GetSafeHwnd () == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	return AddAnchor (m_pHostWnd->GetDlgItem (nID)->GetSafeHwnd (), 
		typeMove, typeSize, percMove, percSize);
}

BOOL CBCGPStaticLayout::AddAnchor (HWND hWnd, XMoveType typeMove, XSizeType typeSize, 
	const CPoint& percMove, const CPoint& percSize)
{
	if (hWnd == NULL || !::IsWindow (hWnd) || !::IsChild (m_pHostWnd->GetSafeHwnd (), hWnd))
	{
		ASSERT(FALSE);
		return FALSE;
	}

	return AddAnchor ((LPVOID)hWnd, typeMove, typeSize, percMove, percSize);
}

BOOL CBCGPStaticLayout::AddAnchor (LPVOID lpHandle, XMoveType typeMove, XSizeType typeSize, 
	const CPoint& percMove, const CPoint& percSize)
{
	XWndItem item;
	if (FindItem (lpHandle, item))
	{
		ASSERT(FALSE);
		return FALSE;
	}

	if (!m_bDontCheckFormViewInitDlg)
	{
		CBCGPFormView* pFormView = DYNAMIC_DOWNCAST(CBCGPFormView, m_pHostWnd);
		if (pFormView != NULL)
		{
			ASSERT_VALID(pFormView);

			if (pFormView->IsInitDlgCompleted())
			{
				TRACE0("CBCGPStaticLayout::AddAnchor failed! Please call this method inside WM_INITDIALOG message handler in your CBCGPFormView-derived class.\n");
				ASSERT(FALSE);
			}
		}
	}

	item.m_Handle   = lpHandle;
	item.m_typeMove = typeMove;
	item.m_percMove = percMove;
	item.m_typeSize = typeSize;
	item.m_percSize = percSize;

	CorrectItem (item);

	if (item.m_typeMove == e_MoveTypeHorz)
	{
		item.m_percMove.y = 0;
	}
	if (item.m_typeMove == e_MoveTypeVert)
	{
		item.m_percMove.x = 0;
	}

	if (item.m_typeSize == e_SizeTypeHorz)
	{
		item.m_percSize.y = 0;
	}
	if (item.m_typeSize == e_SizeTypeVert)
	{
		item.m_percSize.x = 0;
	}

	if (item.m_percMove.x == 0)
	{
		item.m_typeMove = (item.m_typeMove == e_MoveTypeBoth || item.m_typeMove == e_MoveTypeVert)
									? e_MoveTypeVert
									: e_MoveTypeNone;
	}
	if (item.m_percMove.y == 0)
	{
		item.m_typeMove = (item.m_typeMove == e_MoveTypeBoth || item.m_typeMove == e_MoveTypeHorz)
									? e_MoveTypeHorz
									: e_MoveTypeNone;
	}

	if (item.m_percSize.x == 0)
	{
		item.m_typeSize = (item.m_typeSize == e_SizeTypeBoth || item.m_typeSize == e_SizeTypeVert)
									? e_SizeTypeVert
									: e_SizeTypeNone;
	}
	if (item.m_percSize.y == 0)
	{
		item.m_typeSize = (item.m_typeSize == e_SizeTypeBoth || item.m_typeSize == e_SizeTypeHorz)
									? e_SizeTypeHorz
									: e_SizeTypeNone;
	}

	if (CollectItem (item))
	{
		m_listWnd.AddTail (item);
	}

	return TRUE;
}

BOOL CBCGPStaticLayout::FindItem (LPVOID handle, XWndItem& item)
{
	BOOL bRes = FALSE;

	POSITION pos = m_listWnd.GetHeadPosition ();
	while (pos != NULL)
	{
		XWndItem& it = m_listWnd.GetNext (pos);
		if (it.m_Handle == handle)
		{
			item = it;
			bRes = TRUE;
			break;
		}
	}

	return bRes;
}

void CBCGPStaticLayout::CorrectItem (XWndItem& item) const
{
	HWND hwnd = (HWND)item.m_Handle;

    CString strName;
    ::GetClassName (hwnd, strName.GetBufferSetLength (1024), 1024);
    strName.ReleaseBuffer ();

	DWORD dwStyle = ::GetWindowLong (hwnd, GWL_STYLE);

	if (strName.CompareNoCase (_T("COMBOBOX")) == 0 || 
		strName.CompareNoCase (WC_COMBOBOXEX) == 0)
	{
		if (item.m_typeSize == e_SizeTypeVert || 
			item.m_typeSize == e_SizeTypeBoth)
		{
			if ((dwStyle & CBS_SIMPLE) == 0)
			{
				item.m_typeSize = item.m_typeSize == e_SizeTypeBoth
									? e_SizeTypeHorz
									: e_SizeTypeNone;
			}
		}
	}
}

CRect CBCGPStaticLayout::GetItemRect(XWndItem& item) const
{
	CRect rectChild (0, 0, 0, 0);

	if (m_pHostWnd == NULL)
	{
		ASSERT(FALSE);
		return NULL;
	}

	::GetWindowRect ((HWND)item.m_Handle, rectChild);
	m_pHostWnd->ScreenToClient (rectChild);

	return rectChild;
}

BOOL CBCGPStaticLayout::CollectItem (XWndItem& item) const
{
	CRect rectHost;
	GetHostWndRect (rectHost);

	if (rectHost.IsRectNull ())
	{
		ASSERT(FALSE);
		return FALSE;
	}

	CRect rectChild  = GetItemRect(item);

	const double deltaX = (double)rectHost.Width () / 100.0;
	const double deltaY = (double)rectHost.Height () / 100.0;

	item.m_initPoint.x = (double)rectChild.left;
	item.m_initPoint.y = (double)rectChild.top;

	if (item.m_typeMove == e_MoveTypeHorz || item.m_typeMove == e_MoveTypeBoth)
	{
		item.m_initPoint.x -= deltaX * item.m_percMove.x;
	}
	if (item.m_typeMove == e_MoveTypeVert || item.m_typeMove == e_MoveTypeBoth)
	{
		item.m_initPoint.y -= deltaY * item.m_percMove.y;
	}

	item.m_initSize.x  = (double)rectChild.Width ();
	item.m_initSize.y  = (double)rectChild.Height ();
	if (item.m_typeSize == e_SizeTypeHorz || item.m_typeSize == e_SizeTypeBoth)
	{
		item.m_initSize.x -= deltaX * item.m_percSize.x;
	}
	if (item.m_typeSize == e_SizeTypeVert || item.m_typeSize == e_SizeTypeBoth)
	{
		item.m_initSize.y -= deltaY * item.m_percSize.y;
	}

	return TRUE;
}

UINT CBCGPStaticLayout::CalculateItem (XWndItem& item, CRect& rectItem) const
{
	rectItem.SetRect(0, 0, 0, 0);

	CRect rectHost;
	GetHostWndRect (rectHost);

	if (rectHost.IsRectNull ())
	{
		return 0;
	}

	UINT uiFlags = 0;
	const double deltaX = (double)rectHost.Width () / 100.0;
	const double deltaY = (double)rectHost.Height () / 100.0;

	XWndItem::XPoint point(item.m_initPoint);
	XWndItem::XPoint size (item.m_initSize);

	if (item.m_typeMove == e_MoveTypeHorz || item.m_typeMove == e_MoveTypeBoth)
	{
		point.x += deltaX * item.m_percMove.x;
	}
	if (item.m_typeMove == e_MoveTypeVert || item.m_typeMove == e_MoveTypeBoth)
	{
		point.y += deltaY * item.m_percMove.y;
	}

	if (item.m_typeSize == e_SizeTypeHorz || item.m_typeSize == e_SizeTypeBoth)
	{
		size.x += deltaX * item.m_percSize.x;
	}
	if (item.m_typeSize == e_SizeTypeVert || item.m_typeSize == e_SizeTypeBoth)
	{
		size.y += deltaY * item.m_percSize.y;
	}

	rectItem.left   = (long)point.x + rectHost.left;
	rectItem.top    = (long)point.y + rectHost.top;
	rectItem.right  = rectItem.left + (long)size.x;
	rectItem.bottom = rectItem.top + (long)size.y;

	if (rectItem.left == (item.m_initPoint.x + rectHost.left) &&
		rectItem.top == (item.m_initPoint.y + rectHost.top))
	{
		uiFlags |= SWP_NOMOVE;
	}
	if (rectItem.Width() == item.m_initSize.x &&
		rectItem.Height() == item.m_initSize.y)
	{
		uiFlags |= SWP_NOSIZE;
	}

	return uiFlags;
}

