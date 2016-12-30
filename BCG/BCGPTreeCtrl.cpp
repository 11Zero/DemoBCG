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
// BCGPTreeCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "BCGPTreeCtrl.h"
#include "BCGPDlgImpl.h"
#include "BCGPVisualManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBCGPTreeCtrl

IMPLEMENT_DYNAMIC(CBCGPTreeCtrl, CTreeCtrl)

CBCGPTreeCtrl::CBCGPTreeCtrl()
{
	m_bVisualManagerStyle = FALSE;
}

CBCGPTreeCtrl::~CBCGPTreeCtrl()
{
}


BEGIN_MESSAGE_MAP(CBCGPTreeCtrl, CTreeCtrl)
	//{{AFX_MSG_MAP(CBCGPTreeCtrl)
	ON_WM_ERASEBKGND()
	ON_WM_CREATE()
	ON_WM_NCPAINT()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_PRINT, OnPrint)
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomDraw)
	ON_REGISTERED_MESSAGE(BCGM_ONSETCONTROLVMMODE, OnBCGSetControlVMMode)
	ON_REGISTERED_MESSAGE(BCGM_CHANGEVISUALMANAGER, OnChangeVisualManager)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGPTreeCtrl message handlers

void CBCGPTreeCtrl::OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMTVCUSTOMDRAW pNMTVCD = (LPNMTVCUSTOMDRAW) pNMHDR;
	if (pNMTVCD == NULL)
	{
		*pResult = -1;
		return;
	}

	*pResult = CDRF_DODEFAULT;

	if (!m_bVisualManagerStyle)
	{
		return;
	}

	if (pNMTVCD->nmcd.dwDrawStage == CDDS_PREPAINT)
	{
		*pResult = CDRF_NOTIFYPOSTPAINT | CDRF_NOTIFYITEMDRAW;
	}
	else if (pNMTVCD->nmcd.dwDrawStage == CDDS_ITEMPREPAINT)
	{
		BOOL bIsSelected = FALSE;
		BOOL bIsFocused = FALSE;
		BOOL bIsDisabled = FALSE;

		if (!IsWindowEnabled ())
		{
			bIsDisabled = TRUE;
		}
		else
		{
			UINT nState = GetItemState ((HTREEITEM) pNMTVCD->nmcd.dwItemSpec, TVIF_STATE);
			if ((nState & (TVIS_SELECTED | TVIS_DROPHILITED)) != 0)
			{
				if (GetFocus () != this && (nState & TVIS_DROPHILITED) == 0)
				{
					bIsSelected = (GetStyle () & TVS_SHOWSELALWAYS) == TVS_SHOWSELALWAYS;
				}
				else
				{
					bIsSelected = TRUE;
					bIsFocused = TRUE;
				}
			}
		}

		COLORREF clrBk = CBCGPVisualManager::GetInstance ()->GetTreeControlFillColor(this, bIsSelected, bIsFocused, bIsDisabled);
		COLORREF clrText = CBCGPVisualManager::GetInstance ()->GetTreeControlTextColor(this, bIsSelected, bIsFocused, bIsDisabled);

		if (clrBk != (COLORREF)-1)
		{
			pNMTVCD->clrTextBk = clrBk;
		}

		if (clrText != (COLORREF)-1)
		{
			pNMTVCD->clrText = clrText;
		}
	}
}
//**************************************************************************
LRESULT CBCGPTreeCtrl::OnBCGSetControlVMMode (WPARAM wp, LPARAM)
{
	m_bVisualManagerStyle = (BOOL) wp;
	OnChangeVisualManager (0, 0);

	return 0L;
}
//**************************************************************************
BOOL CBCGPTreeCtrl::OnEraseBkgnd(CDC* pDC) 
{
#ifndef _BCGSUITE_
	if (m_bVisualManagerStyle && (globalData.bIsWindows9x || globalData.bIsWindowsNT4))
	{
		COLORREF clr = CBCGPVisualManager::GetInstance ()->GetTreeControlFillColor(this);
		if (clr != (COLORREF)-1)
		{
			CRect rectClient;
			GetClientRect (&rectClient);

			CBrush br(clr);
			pDC->FillRect(rectClient, &br);

			return TRUE;
		}
	}
#endif
	return CTreeCtrl::OnEraseBkgnd(pDC);
}
//**************************************************************************
LRESULT CBCGPTreeCtrl::OnChangeVisualManager (WPARAM, LPARAM)
{
	HWND hWnd = GetSafeHwnd ();
	if (hWnd == NULL)
	{
		return 0L;
	}

	COLORREF clrBack = (COLORREF)-1;
	COLORREF clrText = (COLORREF)-1;
	COLORREF clrLine = CLR_DEFAULT;

	if (m_bVisualManagerStyle)
	{
		clrBack = CBCGPVisualManager::GetInstance ()->GetTreeControlFillColor(this);
		clrText = CBCGPVisualManager::GetInstance ()->GetTreeControlTextColor(this);
		clrLine = CBCGPVisualManager::GetInstance ()->GetTreeControlLineColor(this);
	}

	TreeView_SetBkColor(hWnd, clrBack);
	TreeView_SetTextColor(hWnd, clrText);
	TreeView_SetLineColor(hWnd, clrLine);

	RedrawWindow(NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW | RDW_FRAME);

	return 0L;
}
//**************************************************************************
int CBCGPTreeCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CTreeCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (m_bVisualManagerStyle)
	{
		OnChangeVisualManager (0, 0);
	}
	
	return 0;
}
//**************************************************************************
void CBCGPTreeCtrl::PreSubclassWindow() 
{
	CTreeCtrl::PreSubclassWindow();

	if (m_bVisualManagerStyle)
	{
		OnChangeVisualManager (0, 0);
	}
}
//**************************************************************************
void CBCGPTreeCtrl::OnNcPaint()
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
LRESULT CBCGPTreeCtrl::OnPrint(WPARAM wp, LPARAM lp)
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
