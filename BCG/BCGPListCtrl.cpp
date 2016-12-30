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
// BCGListCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "BCGPListCtrl.h"
#include "BCGPDrawManager.h"
#include "BCGPDlgImpl.h"
#include "BCGPVisualManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBCGPListCtrl

IMPLEMENT_DYNAMIC(CBCGPListCtrl, CListCtrl)

CBCGPListCtrl::CBCGPListCtrl()
{
	m_bVisualManagerStyle = FALSE;
	m_iSortedColumn = -1;
	m_bAscending = TRUE;
	m_bMarkSortedColumn = FALSE;
	m_clrSortedColumn = (COLORREF)-1;
	m_hOldFont = NULL;
}
//*********************************************************************************
CBCGPListCtrl::~CBCGPListCtrl()
{
}

BEGIN_MESSAGE_MAP(CBCGPListCtrl, CListCtrl)
	//{{AFX_MSG_MAP(CBCGPListCtrl)
	ON_WM_CREATE()
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnColumnClick)
	ON_WM_ERASEBKGND()
	ON_WM_SYSCOLORCHANGE()
	ON_WM_SIZE()
	ON_WM_NCPAINT()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_PRINT, OnPrint)
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomDraw)
	ON_MESSAGE(WM_STYLECHANGED, OnStyleChanged)
	ON_REGISTERED_MESSAGE(BCGM_ONSETCONTROLVMMODE, OnBCGSetControlVMMode)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGPListCtrl message handlers

LRESULT CBCGPListCtrl::OnBCGSetControlVMMode (WPARAM wp, LPARAM)
{
	m_bVisualManagerStyle = (BOOL) wp;

	CBCGPHeaderCtrl& wndHeader = GetHeaderCtrl();
	if (wndHeader.GetSafeHwnd() != NULL)
	{
		wndHeader.SendMessage(BCGM_ONSETCONTROLVMMODE, wp);
	}

	return 0;
}
//**************************************************************************
BOOL CBCGPListCtrl::InitList ()
{
	InitHeader ();
	InitColors ();

	CBCGPHeaderCtrl& wndHeader = GetHeaderCtrl();
	if (wndHeader.GetSafeHwnd() != NULL)
	{
		wndHeader.SendMessage(BCGM_ONSETCONTROLVMMODE, (WPARAM)m_bVisualManagerStyle);
	}

	return TRUE;
}
//*********************************************************************************
void CBCGPListCtrl::InitHeader ()
{
	//---------------------------
	// Initialize header control:
	//---------------------------
	GetHeaderCtrl().SubclassDlgItem (0, this);
}
//*********************************************************************************
void CBCGPListCtrl::PreSubclassWindow() 
{
	CListCtrl::PreSubclassWindow();

	_AFX_THREAD_STATE* pThreadState = AfxGetThreadState ();
	if (pThreadState->m_pWndInit == NULL)
	{
		if (!InitList ())
		{
			ASSERT(FALSE);
		}
	}
}
//*********************************************************************************
int CBCGPListCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CListCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!InitList ())
	{
		return -1;
	}

	return 0;
}
//*********************************************************************************
void CBCGPListCtrl::OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	ASSERT (pNMListView != NULL);

	int iColumn = pNMListView->iSubItem;
	BOOL bShiftIsPressed = (::GetAsyncKeyState (VK_SHIFT) & 0x8000);
	int nColumnState = GetHeaderCtrl ().GetColumnState (iColumn);
	BOOL bAscending = TRUE;
	
	if (nColumnState != 0)
	{
		bAscending = nColumnState <= 0;
	}

	Sort (iColumn, bAscending, bShiftIsPressed && IsMultipleSort ());
	*pResult = 0;
}
//*********************************************************************************
void CBCGPListCtrl::Sort (int iColumn, BOOL bAscending, BOOL bAdd)
{
	CWaitCursor wait;

	GetHeaderCtrl ().SetSortColumn (iColumn, bAscending, bAdd);

	m_iSortedColumn = iColumn;
	m_bAscending = bAscending;

	SortItems (CompareProc, (LPARAM) this);
}
//*********************************************************************************
void CBCGPListCtrl::SetSortColumn (int iColumn, BOOL bAscending, BOOL bAdd)
{
	GetHeaderCtrl ().SetSortColumn (iColumn, bAscending, bAdd);
}
//*********************************************************************************
void CBCGPListCtrl::RemoveSortColumn (int iColumn)
{
	GetHeaderCtrl ().RemoveSortColumn (iColumn);
}
//*********************************************************************************
void CBCGPListCtrl::EnableMultipleSort (BOOL bEnable)
{
	GetHeaderCtrl ().EnableMultipleSort (bEnable);
}
//*********************************************************************************
BOOL CBCGPListCtrl::IsMultipleSort () const
{
	return ((CBCGPListCtrl*) this)->GetHeaderCtrl ().IsMultipleSort ();
}
//*********************************************************************************
int CBCGPListCtrl::OnCompareItems (LPARAM /*lParam1*/, 
								  LPARAM /*lParam2*/, 
								  int /*iColumn*/)
{
	return 0;
}
//***************************************************************************************
int CALLBACK CBCGPListCtrl::CompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	CBCGPListCtrl* pList = (CBCGPListCtrl*) lParamSort;
	ASSERT_VALID (pList);

	int nRes = pList->OnCompareItems (lParam1, lParam2, pList->m_iSortedColumn);
	if (!pList->m_bAscending)
	{
		nRes = -nRes;
	}

	return nRes;
}
//****************************************************************************************
void CBCGPListCtrl::OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLVCUSTOMDRAW lplvcd = (LPNMLVCUSTOMDRAW)pNMHDR;

	switch(lplvcd->nmcd.dwDrawStage)
	{
	case CDDS_PREPAINT:
		*pResult = CDRF_NOTIFYITEMDRAW;
		break;

	case CDDS_ITEMPREPAINT:
	case CDDS_ITEMPREPAINT | CDDS_SUBITEM:
		{
			if (lplvcd->nmcd.dwDrawStage == CDDS_ITEMPREPAINT && 
				(GetStyle() & LVS_TYPEMASK) == LVS_REPORT)
			{
				*pResult = CDRF_NOTIFYSUBITEMDRAW;
				break;
			}

			int iColumn = lplvcd->iSubItem;
			int iRow = (int) lplvcd->nmcd.dwItemSpec;

			lplvcd->clrTextBk = OnGetCellBkColor (iRow, iColumn);
			lplvcd->clrText = OnGetCellTextColor (iRow, iColumn);

			if (iColumn == m_iSortedColumn && m_bMarkSortedColumn &&
				lplvcd->clrTextBk == GetDefaultBkColor())
			{
				lplvcd->clrTextBk = GetMarkedColor();
			}

			HFONT hFont = OnGetCellFont (	iRow, iColumn, 
											(DWORD) lplvcd->nmcd.lItemlParam);
				
			if (hFont != NULL)
			{
				m_hOldFont = (HFONT) SelectObject (lplvcd->nmcd.hdc, hFont);
				ASSERT (m_hOldFont != NULL);

				*pResult = CDRF_NEWFONT | CDRF_NOTIFYPOSTPAINT;
			}
			else
			{
				*pResult = CDRF_DODEFAULT;
			}
		}
		break;

	case CDDS_ITEMPOSTPAINT | CDDS_SUBITEM:
		if (m_hOldFont != NULL)
		{
			SelectObject (lplvcd->nmcd.hdc, m_hOldFont);
			m_hOldFont = NULL;
		}

		*pResult = CDRF_DODEFAULT;
		break;
	}
}
//****************************************************************************************
void CBCGPListCtrl::EnableMarkSortedColumn (BOOL bMark/* = TRUE*/,
										   BOOL bRedraw/* = TRUE */)
{
	m_bMarkSortedColumn = bMark;

	if (GetSafeHwnd () != NULL && bRedraw)
	{
		RedrawWindow ();
	}
}
//****************************************************************************************
BOOL CBCGPListCtrl::OnEraseBkgnd(CDC* pDC) 
{
	CRect rectClient;
	GetClientRect (&rectClient);

	BOOL bRes = TRUE;
	
	if (m_bVisualManagerStyle)
	{
		::FillRect(pDC->GetSafeHdc(), rectClient, CBCGPVisualManager::GetInstance ()->GetListControlFillBrush(this));
	}
	else
	{
		bRes = CListCtrl::OnEraseBkgnd(pDC);
	}

	if (m_iSortedColumn >= 0 && m_bMarkSortedColumn)
	{
		CRect rectHeader;
		GetHeaderCtrl ().GetItemRect (m_iSortedColumn, &rectHeader);
		GetHeaderCtrl ().MapWindowPoints (this, rectHeader);

		CRect rectColumn = rectClient;
		rectColumn.left = rectHeader.left;
		rectColumn.right = rectHeader.right;

		CBrush br(GetMarkedColor());
		pDC->FillRect (rectColumn, &br);
	}

	return bRes;
}
//*****************************************************************************************
void CBCGPListCtrl::OnSysColorChange() 
{
	CListCtrl::OnSysColorChange();
	
	InitColors ();
	RedrawWindow ();
}
//****************************************************************************************
COLORREF CBCGPListCtrl::GetMarkedColor()
{
	return m_bVisualManagerStyle ? CBCGPVisualManager::GetInstance ()->GetListControlMarkedColor(this) : m_clrSortedColumn;
}
//****************************************************************************************
void CBCGPListCtrl::InitColors ()
{
	m_clrSortedColumn = CBCGPDrawManager::PixelAlpha(GetBkColor(), .97, .97, .97);
}
//***************************************************************************************
LRESULT CBCGPListCtrl::OnStyleChanged(WPARAM wp, LPARAM lp)
{
	int nStyleType = (int) wp;
	LPSTYLESTRUCT lpStyleStruct = (LPSTYLESTRUCT) lp;

	CListCtrl::OnStyleChanged (nStyleType, lpStyleStruct);

	if ((lpStyleStruct->styleNew & LVS_REPORT) &&
		(lpStyleStruct->styleOld & LVS_REPORT) == 0)
	{
		if (GetHeaderCtrl ().GetSafeHwnd () == NULL)
		{
			InitHeader ();
		}
	}

	return 0;
}
//****************************************************************************************
void CBCGPListCtrl::OnSize(UINT nType, int cx, int cy) 
{
	CListCtrl::OnSize(nType, cx, cy);
	
	if (GetHeaderCtrl().GetSafeHwnd() != NULL)
	{
		GetHeaderCtrl().RedrawWindow();
	}
}
//****************************************************************************************
COLORREF CBCGPListCtrl::OnGetCellTextColor (int /*nRow*/, int /*nColum*/)
{
	return m_bVisualManagerStyle ? CBCGPVisualManager::GetInstance ()->GetListControlTextColor(this) : GetTextColor ();
}
//****************************************************************************************
COLORREF CBCGPListCtrl::OnGetCellBkColor (int /*nRow*/, int /*nColum*/)
{
	return GetDefaultBkColor();
}
//****************************************************************************************
COLORREF CBCGPListCtrl::GetDefaultBkColor()
{
	if (!m_bVisualManagerStyle)
	{
		return GetBkColor ();
	}
	
	CBrush* pBr = CBrush::FromHandle(CBCGPVisualManager::GetInstance ()->GetListControlFillBrush(this));
	
	LOGBRUSH lbr;
	pBr->GetLogBrush(&lbr);
	
	return lbr.lbColor;
}
//**************************************************************************
void CBCGPListCtrl::OnNcPaint()
{
	Default();

	if (!m_bVisualManagerStyle)
	{
		return;
	}

	if ((GetStyle () & WS_BORDER) != 0 || (GetExStyle () & WS_EX_CLIENTEDGE) != 0)
	{
		CBCGPVisualManager::GetInstance ()->OnDrawControlBorder(this);
	}
}
//****************************************************************************
LRESULT CBCGPListCtrl::OnPrint(WPARAM wp, LPARAM lp)
{
	LRESULT lRes = Default();

	if (!m_bVisualManagerStyle)
	{
		return lRes;
	}

	DWORD dwFlags = (DWORD)lp;

	if ((dwFlags & PRF_NONCLIENT) == PRF_NONCLIENT)
	{
		if ((GetStyle () & WS_BORDER) != 0 || (GetExStyle () & WS_EX_CLIENTEDGE) != 0)
		{
			CDC* pDC = CDC::FromHandle((HDC) wp);
			ASSERT_VALID(pDC);

			CRect rect;
			GetWindowRect(rect);
			
			rect.bottom -= rect.top;
			rect.right -= rect.left;
			rect.left = rect.top = 0;

			CBCGPVisualManager::GetInstance ()->OnDrawControlBorder(pDC, rect, this, FALSE);
			return 0;
		}
	}

	return lRes;
}


