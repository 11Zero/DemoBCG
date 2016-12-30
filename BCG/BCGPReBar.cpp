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
// BCGPReBar.cpp: implementation of the CBCGPReBar class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BCGPReBar.h"

HWND _bcgpWindowFromPoint(HWND hWnd, POINT pt);

#pragma warning (disable : 4355)

/////////////////////////////////////////////////////////////////////////////
// CReBar

BEGIN_MESSAGE_MAP(CBCGPReBar, CBCGPControlBar)
	//{{AFX_MSG_MAP(CBCGPReBar)
	ON_WM_NCCREATE()
	ON_WM_PAINT()
	ON_WM_NCCALCSIZE()
	ON_WM_ERASEBKGND()
	ON_WM_NCPAINT()
	ON_NOTIFY_REFLECT(RBN_HEIGHTCHANGE, OnHeightChange)
	ON_NOTIFY_REFLECT(RBN_ENDDRAG, OnHeightChange)
	ON_MESSAGE(RB_SHOWBAND, OnShowBand)
	ON_MESSAGE_VOID(WM_RECALCPARENT, OnRecalcParent)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CBCGPReBar::CBCGPReBar() : m_Impl (this)
{
	m_bLocked = FALSE;
	SetBorders();
}

void CBCGPReBar::OnRecalcParent()
{
	CFrameWnd* pFrameWnd = BCGPGetParentFrame(this);
	ASSERT(pFrameWnd != NULL);
	pFrameWnd->RecalcLayout();
}

void CBCGPReBar::OnHeightChange(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	AdjustDockingLayout ();
	*pResult = 0;
}

LRESULT CBCGPReBar::OnShowBand(WPARAM wParam, LPARAM)
{
	LRESULT lResult = Default();
	if (lResult)
	{
		// keep window visible state in sync with band visible state
		REBARBANDINFO rbBand;
		rbBand.cbSize = globalData.GetRebarBandInfoSize ();
		rbBand.fMask = RBBIM_CHILD|RBBIM_STYLE;
		VERIFY(DefWindowProc(RB_GETBANDINFO, wParam, (LPARAM)&rbBand));
		CBCGPControlBar* pBar = DYNAMIC_DOWNCAST(CBCGPControlBar, CWnd::FromHandlePermanent(rbBand.hwndChild));
		BOOL bWindowVisible;
		if (pBar != NULL)
			bWindowVisible = pBar->IsVisible();
		else
			bWindowVisible =  (::GetWindowLong(rbBand.hwndChild, GWL_STYLE) & WS_VISIBLE) != 0;
		BOOL bBandVisible = (rbBand.fStyle & RBBS_HIDDEN) == 0;
		if (bWindowVisible != bBandVisible)
			VERIFY(::ShowWindow(rbBand.hwndChild, bBandVisible ? SW_SHOW : SW_HIDE));
	}
	return lResult;
}

BOOL CBCGPReBar::_AddBar(CWnd* pBar, REBARBANDINFO* pRBBI)
{
	ASSERT_VALID(this);
	ASSERT(::IsWindow(m_hWnd));
	ASSERT(pBar != NULL);
	ASSERT(::IsWindow(pBar->m_hWnd));

	pRBBI->cbSize = globalData.GetRebarBandInfoSize ();
	pRBBI->fMask |= RBBIM_CHILD | RBBIM_CHILDSIZE;
	pRBBI->hwndChild = pBar->m_hWnd;

	CSize size;
	CBCGPControlBar* pTemp = DYNAMIC_DOWNCAST(CBCGPControlBar, pBar);
	if (pTemp != NULL)
	{
		size = pTemp->CalcFixedLayout(FALSE, m_dwStyle & CBRS_ORIENT_HORZ);
		pTemp->SetBCGStyle(pTemp->GetBCGStyle () & ~(CBRS_BCGP_AUTOHIDE | CBRS_BCGP_FLOAT));
		pTemp->m_bIsRebarPane = TRUE;
	}
	else
	{
		CRect rect;
		pBar->GetWindowRect(&rect);
		size = rect.Size();
	}
	//WINBUG: COMCTL32.DLL is off by 4 pixels in its sizing logic.  Whatever
	//  is specified as the minimum size, the system rebar will allow that band
	//  to be 4 actual pixels smaller!  That's why we add 4 to the size here.

	pRBBI->cxMinChild = size.cx;
	pRBBI->cyMinChild = size.cy;
	BOOL bResult = (BOOL)DefWindowProc(RB_INSERTBAND, (WPARAM)-1, (LPARAM)pRBBI);

	CFrameWnd* pFrameWnd = BCGPGetParentFrame(this);
	if (pFrameWnd != NULL)
		pFrameWnd->RecalcLayout();

	return bResult;
}

BOOL CBCGPReBar::AddBar(CWnd* pBar, LPCTSTR pszText, CBitmap* pbmp, DWORD dwStyle)
{
	REBARBANDINFO rbBand;
	rbBand.fMask = RBBIM_STYLE;
	rbBand.fStyle = dwStyle;
	if (pszText != NULL)
	{
		rbBand.fMask |= RBBIM_TEXT;
		rbBand.lpText = const_cast<LPTSTR>(pszText);
	}
	if (pbmp != NULL)
	{
		rbBand.fMask |= RBBIM_BACKGROUND;
		rbBand.hbmBack = (HBITMAP)*pbmp;
	}

	if (m_bLocked)
	{
		rbBand.fStyle |= RBBS_NOGRIPPER;
	}

	return _AddBar(pBar, &rbBand);
}

BOOL CBCGPReBar::AddBar(CWnd* pBar, COLORREF clrFore, COLORREF clrBack, LPCTSTR pszText, DWORD dwStyle)
{
	REBARBANDINFO rbBand;
	rbBand.fMask = RBBIM_STYLE | RBBIM_COLORS;
	rbBand.fStyle = dwStyle;
	rbBand.clrFore = clrFore;
	rbBand.clrBack = clrBack;
	if (pszText != NULL)
	{
		rbBand.fMask |= RBBIM_TEXT;
		rbBand.lpText = const_cast<LPTSTR>(pszText);
	}

	if (m_bLocked)
	{
		rbBand.fStyle |= RBBS_NOGRIPPER;
	}

	return _AddBar(pBar, &rbBand);
}

CSize CBCGPReBar::CalcFixedLayout(BOOL bStretch, BOOL bHorz)
{

	ASSERT_VALID(this);
	ASSERT(::IsWindow(m_hWnd));

	// the union of the band rectangles is the total bounding rect
	int nCount = (int) DefWindowProc(RB_GETBANDCOUNT, 0, 0);
	REBARBANDINFO rbBand;
	rbBand.cbSize = globalData.GetRebarBandInfoSize ();
	int nTemp;

	// sync up hidden state of the bands
	for (nTemp = nCount; nTemp--; )
	{
		rbBand.fMask = RBBIM_CHILD|RBBIM_STYLE;
		VERIFY(DefWindowProc(RB_GETBANDINFO, nTemp, (LPARAM)&rbBand));
		CBCGPControlBar* pBar = DYNAMIC_DOWNCAST(CBCGPControlBar, CWnd::FromHandlePermanent(rbBand.hwndChild));
		BOOL bWindowVisible;
		if (pBar != NULL)
			bWindowVisible = pBar->IsVisible();
		else
			bWindowVisible =  (::GetWindowLong(rbBand.hwndChild, GWL_STYLE) & WS_VISIBLE) != 0;
		BOOL bBandVisible = (rbBand.fStyle & RBBS_HIDDEN) == 0;
		if (bWindowVisible != bBandVisible)
			VERIFY(DefWindowProc(RB_SHOWBAND, nTemp, bWindowVisible));
	}

	// determine bounding rect of all visible bands
	CRect rectBound; rectBound.SetRectEmpty();
	for (nTemp = nCount; nTemp--; )
	{
		rbBand.fMask = RBBIM_STYLE;
		VERIFY(DefWindowProc(RB_GETBANDINFO, nTemp, (LPARAM)&rbBand));
		if ((rbBand.fStyle & RBBS_HIDDEN) == 0)
		{
			CRect rect;
			VERIFY(DefWindowProc(RB_GETRECT, nTemp, (LPARAM)&rect));
			rectBound |= rect;
		}
	}

	// add borders as part of bounding rect
	if (!rectBound.IsRectEmpty())
	{
		CRect rect; rect.SetRectEmpty();
		CalcInsideRect(rect, bHorz);
		rectBound.right -= rect.Width();
		rectBound.bottom -= rect.Height();
	}
	bStretch = 1;
	return CSize((bHorz && bStretch) ? 32767 : rectBound.Width(),
				 (!bHorz && bStretch) ? 32767 : rectBound.Height());
}
//****************************************************************************************
BOOL CBCGPReBar::Create(CWnd* pParentWnd, DWORD dwCtrlStyle, DWORD dwStyle, UINT nID)
{
	ASSERT_VALID(pParentWnd);   // must have a parent
	ASSERT (!((dwStyle & CBRS_SIZE_FIXED) && (dwStyle & CBRS_SIZE_DYNAMIC)));

	// save the style
	m_dwStyle = (dwStyle & CBRS_ALL);
	if (nID == AFX_IDW_REBAR)
		m_dwStyle |= CBRS_HIDE_INPLACE;

	dwStyle &= ~CBRS_ALL;
	dwStyle |= CCS_NOPARENTALIGN|CCS_NOMOVEY|CCS_NODIVIDER|CCS_NORESIZE|RBS_VARHEIGHT;
	dwStyle |= dwCtrlStyle | WS_CLIPCHILDREN;

	m_pDockSite = pParentWnd;

	// initialize common controls
	VERIFY(AfxDeferRegisterClass(AFX_WNDCOMMCTL_COOL_REG));

	// create the HWND
	CRect rect; rect.SetRectEmpty();
	if (!CWnd::Create(REBARCLASSNAME, NULL, dwStyle, rect, pParentWnd, nID))
		return FALSE;

	if (m_bLocked)
	{
		ModifyStyle (0, RBS_FIXEDORDER);
	}
	
	return TRUE;
}

void CBCGPReBar::OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHandler)
{
	UpdateDialogControls(pTarget, bDisableIfNoHandler);
}

BOOL CBCGPReBar::OnNcCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (!CBCGPControlBar::OnNcCreate(lpCreateStruct))
		return FALSE;

	// if the owner was set before the rebar was created, set it now
	if (m_hWndOwner != NULL)
		DefWindowProc(RB_SETPARENT, (WPARAM)m_hWndOwner, 0);

	return TRUE;
}

BOOL CBCGPReBar::OnEraseBkgnd(CDC*)
{
	return (BOOL)Default();
}

void CBCGPReBar::OnNcCalcSize(BOOL /*bCalcValidRects*/, NCCALCSIZE_PARAMS* lpncsp)
{
	// calculate border space (will add to top/bottom, subtract from right/bottom)
	CRect rect; rect.SetRectEmpty();
	BOOL bHorz = (m_dwStyle & CBRS_ORIENT_HORZ) != 0;
	CBCGPControlBar::CalcInsideRect(rect, bHorz);

	// adjust non-client area for border space
	lpncsp->rgrc[0].left += rect.left;
	lpncsp->rgrc[0].top += rect.top;
	lpncsp->rgrc[0].right += rect.right;
	lpncsp->rgrc[0].bottom += rect.bottom;
}

void CBCGPReBar::OnNcPaint()
{
	m_Impl.DrawNcArea ();
}

void CBCGPReBar::OnPaint()
{
	Default();
}

INT_PTR CBCGPReBar::OnToolHitTest(CPoint point, TOOLINFO* pTI) const
{
	ASSERT_VALID(this);
	ASSERT(::IsWindow(m_hWnd));

	HWND hWndChild = _bcgpWindowFromPoint(m_hWnd, point);
	CWnd* pWnd = CWnd::FromHandlePermanent(hWndChild);
	if (pWnd == NULL)
		return (INT_PTR) CBCGPControlBar::OnToolHitTest(point, pTI);

	ASSERT(pWnd->m_hWnd == hWndChild);
	return (INT_PTR) pWnd->OnToolHitTest(point, pTI);
}

LRESULT CBCGPReBar::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	// special handling for certain messages (forwarding to owner/parent)
	switch (message)
	{
	case WM_POPMESSAGESTRING:
	case WM_SETMESSAGESTRING:
		return GetOwner()->SendMessage(message, wParam, lParam);
	}
	return CBCGPControlBar::WindowProc(message, wParam, lParam);
}

#ifdef AFX_INIT_SEG
#pragma code_seg(AFX_INIT_SEG)
#endif

#ifdef _DEBUG

// rebars do not support docking
void CBCGPReBar::EnableDocking(DWORD dwAlignment)
{ 
	CBCGPControlBar::EnableDocking (dwAlignment);
}

#endif

CReBarCtrl& CBCGPReBar::GetReBarCtrl() const
{
	return *(CReBarCtrl*)this;
}

IMPLEMENT_DYNAMIC(CBCGPReBar, CBCGPControlBar)

/////////////////////////////////////////////////////////////////////////////
HWND _bcgpWindowFromPoint(HWND hWnd, POINT pt)
{
	ASSERT(hWnd != NULL);

	// check child windows
	::ClientToScreen(hWnd, &pt);
	HWND hWndChild = ::GetWindow(hWnd, GW_CHILD);
	for (; hWndChild != NULL; hWndChild = ::GetWindow(hWndChild, GW_HWNDNEXT))
	{
		if (_AfxGetDlgCtrlID(hWndChild) != (WORD)-1 &&
			(::GetWindowLong(hWndChild, GWL_STYLE) & WS_VISIBLE))
		{
			// see if point hits the child window
			CRect rect;
			::GetWindowRect(hWndChild, rect);
			if (rect.PtInRect(pt))
				return hWndChild;
		}
	}

	return NULL;    // not found
}

void CBCGPReBar::OnLButtonDown(UINT nFlags, CPoint point)
{
	CWnd::OnLButtonDown (nFlags, point);
}

void CBCGPReBar::OnLButtonUp(UINT nFlags, CPoint point)
{
	CWnd::OnLButtonUp (nFlags, point);
}

void CBCGPReBar::OnMouseMove(UINT nFlags, CPoint point)
{
	CWnd::OnMouseMove (nFlags, point);
}

void CBCGPReBar::SetBarAlignment (DWORD dwAlignment) 
{
	CReBarCtrl& wndReBar = GetReBarCtrl ();
	UINT uiReBarsCount = wndReBar.GetBandCount ();

	REBARBANDINFO bandInfo;
	bandInfo.cbSize = globalData.GetRebarBandInfoSize ();
	bandInfo.fMask = (RBBIM_CHILDSIZE | RBBIM_CHILD | RBBIM_IDEALSIZE);

	for (UINT uiBand = 0; uiBand < uiReBarsCount; uiBand ++)
	{
		wndReBar.GetBandInfo (uiBand, &bandInfo);
		if (bandInfo.hwndChild != NULL)
		{
			CBCGPBaseControlBar* pBar = 
				DYNAMIC_DOWNCAST (CBCGPBaseControlBar, 
				CWnd::FromHandlePermanent (bandInfo.hwndChild));

			if (pBar != NULL)
			{
				pBar->SetBarAlignment (dwAlignment);
			}
		}
	}
}

void CBCGPReBar::LockBars (BOOL bLock)
{ 
	ASSERT_VALID (this);

	if (bLock == m_bLocked)
	{
		return;
	}

	m_bLocked = bLock;

	if (GetSafeHwnd () == NULL)
	{
		return;
	}

	if (m_bLocked)
	{
		ModifyStyle (0, RBS_FIXEDORDER);
	}
	else
	{
		ModifyStyle (RBS_FIXEDORDER, 0);
	}

	CReBarCtrl& wndReBar = GetReBarCtrl ();
	UINT uiReBarsCount = wndReBar.GetBandCount ();

	REBARBANDINFO bandInfo;
	bandInfo.cbSize = globalData.GetRebarBandInfoSize ();
	bandInfo.fMask = RBBIM_STYLE;

	for (UINT uiBand = 0; uiBand < uiReBarsCount; uiBand ++)
	{
		wndReBar.GetBandInfo (uiBand, &bandInfo);

		if (m_bLocked)
		{
			bandInfo.fStyle |= RBBS_NOGRIPPER;
		}
		else 
		{
			bandInfo.fStyle &= ~RBBS_NOGRIPPER;
		}
		
		wndReBar.SetBandInfo (uiBand, &bandInfo);
	}
} 

void CBCGPReBar::GetControlBarList (CObList& lst, CRuntimeClass* pRTCFilter)
{
	CReBarCtrl& wndReBar = GetReBarCtrl ();
	UINT uiReBarsCount = wndReBar.GetBandCount ();

	REBARBANDINFO bandInfo;
	bandInfo.cbSize = globalData.GetRebarBandInfoSize ();
	bandInfo.fMask = (RBBIM_CHILDSIZE | RBBIM_CHILD | RBBIM_IDEALSIZE);

	for (UINT uiBand = 0; uiBand < uiReBarsCount; uiBand ++)
	{
		wndReBar.GetBandInfo (uiBand, &bandInfo);
		if (bandInfo.hwndChild != NULL)
		{
			CBCGPBaseControlBar* pBar = 
				DYNAMIC_DOWNCAST (CBCGPBaseControlBar, 
				CWnd::FromHandlePermanent (bandInfo.hwndChild));

			if (pBar != NULL)
			{
				ASSERT_VALID (pBar);
				if (pRTCFilter == NULL || pBar->GetRuntimeClass () == pRTCFilter)
				{
					lst.AddTail (pBar);
				}
			}
		}
	}
}
