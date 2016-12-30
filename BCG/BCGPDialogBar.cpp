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

// BCGPDialogBar.cpp : implementation file
//

#include "stdafx.h"
#if _MSC_VER >= 1300
	#include <afxocc.h>
#else
	#include <../src/occimpl.h>
#endif

#include "BCGPDialogBar.h"
#include "BCGPGlobalUtils.h"
#include "BCGPVisualManager.h"
#include "BCGPGestureManager.h"

#ifndef _BCGSUITE_
#include "BCGPDockManager.h"
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBCGPDialogBar

IMPLEMENT_SERIAL(CBCGPDialogBar, CBCGPDockingControlBar, VERSIONABLE_SCHEMA | 1)

#pragma warning (disable : 4355)

CBCGPDialogBar::CBCGPDialogBar() :
	m_Impl (*this)
{
#ifndef _AFX_NO_OCC_SUPPORT
	m_pOccDialogInfo = NULL;
#endif

	m_bUpdateScroll = FALSE;
	m_scrollPos     = CPoint(0, 0);
	m_scrollRange   = CSize(0, 0);
	m_scrollSize    = CSize(0, 0);
	m_scrollLine    = CSize(5, 5);
	m_bIsScrollingEnabled = TRUE;
}

#pragma warning (default : 4355)

CBCGPDialogBar::~CBCGPDialogBar()
{

}

/////////////////////////////////////////////////////////////////////////////
// CBCGPDialogBar message handlers

BOOL CBCGPDialogBar::Create(LPCTSTR lpszWindowName, CWnd* pParentWnd, BOOL bHasGripper, 
						   UINT nIDTemplate, UINT nStyle, UINT nID)
{ 
	return Create(lpszWindowName, pParentWnd, bHasGripper, MAKEINTRESOURCE(nIDTemplate), nStyle, nID); 
}
//****************************************************************************************
BOOL CBCGPDialogBar::Create(LPCTSTR lpszWindowName, CWnd* pParentWnd, BOOL bHasGripper, 
						   LPCTSTR lpszTemplateName, UINT nStyle, UINT nID, 
						   DWORD dwTabbedStyle, DWORD dwBCGStyle)
{
	m_lpszBarTemplateName = (LPTSTR) lpszTemplateName;

	if (!CBCGPDockingControlBar::Create(lpszWindowName, pParentWnd, CSize (0, 0), bHasGripper, 
						   nID, nStyle, dwTabbedStyle, dwBCGStyle))
	{
		return FALSE;
	}

	if ((GetStyle() & WS_CHILD) == 0)
	{
		TRACE0("CBCGPDialogBar::Create failed: make sure that your dialog has \"Child\" style in resources.\n");
		ASSERT(FALSE);
		return FALSE;
	}

	m_lpszBarTemplateName = NULL;
	SetOwner (BCGCBProGetTopLevelFrame (this));
	
	if (m_sizeDialog != CSize (0, 0))
	{
		SetWindowPos (NULL, -1, -1, m_sizeDialog.cx, m_sizeDialog.cy, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
	}

	return TRUE;
}
//****************************************************************************************
void CBCGPDialogBar::OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler)
{
	CBCGPDockingControlBar::OnUpdateCmdUI (pTarget, bDisableIfNoHndler);
}
//****************************************************************************************
void CBCGPDialogBar::EnableScrolling(BOOL bEnable)
{
	m_bIsScrollingEnabled = bEnable;

	if (GetSafeHwnd() != NULL)
	{
		CBCGPGestureConfig gestureConfig;

		if (m_bIsScrollingEnabled)
		{
			CRect rectClient;
			GetClientRect (rectClient);

			m_scrollSize = rectClient.Size ();

			gestureConfig.EnablePan(TRUE, BCGP_GC_PAN_WITH_SINGLE_FINGER_HORIZONTALLY | BCGP_GC_PAN_WITH_SINGLE_FINGER_VERTICALLY | BCGP_GC_PAN_WITH_GUTTER | BCGP_GC_PAN_WITH_INERTIA);
		}
		else
		{
			m_scrollSize = CSize(0, 0);
		}
		
		bcgpGestureManager.SetGestureConfig(GetSafeHwnd(), gestureConfig);
	}
}

BEGIN_MESSAGE_MAP(CBCGPDialogBar, CBCGPDockingControlBar)
	//{{AFX_MSG_MAP(CBCGPDialogBar)
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
	ON_WM_WINDOWPOSCHANGING()
	ON_WM_CTLCOLOR()
	ON_WM_DESTROY()
	ON_WM_CREATE()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_INITDIALOG, HandleInitDialog)
	ON_REGISTERED_MESSAGE(BCGM_CHANGEVISUALMANAGER, OnChangeVisualManager)	
END_MESSAGE_MAP()

//*****************************************************************************************
LRESULT CBCGPDialogBar::HandleInitDialog(WPARAM wParam, LPARAM lParam)
{
	CBCGPBaseControlBar::HandleInitDialog(wParam, lParam);

#ifndef _AFX_NO_OCC_SUPPORT
	Default();  // allow default to initialize first (common dialogs/etc)

	// create OLE controls
	COccManager* pOccManager = afxOccManager;
	if ((pOccManager != NULL) && (m_pOccDialogInfo != NULL))
	{
		if (!pOccManager->CreateDlgControls(this, m_lpszBarTemplateName,
			m_pOccDialogInfo))
		{
			TRACE (_T("Warning: CreateDlgControls failed during dialog bar init.\n"));
			return FALSE;
		}
	}
#endif //!_AFX_NO_OCC_SUPPORT

	if (IsVisualManagerStyle ())
	{
		m_Impl.EnableVisualManagerStyle (TRUE);
		m_Impl.m_bTransparentStaticCtrls = FALSE;
	}

	if (m_bIsScrollingEnabled)
	{
		CRect rectClient;
		GetClientRect (rectClient);

		m_scrollSize = rectClient.Size ();

		CBCGPGestureConfig gestureConfig;
		gestureConfig.EnablePan(TRUE, BCGP_GC_PAN_WITH_SINGLE_FINGER_HORIZONTALLY | BCGP_GC_PAN_WITH_SINGLE_FINGER_VERTICALLY | BCGP_GC_PAN_WITH_GUTTER | BCGP_GC_PAN_WITH_INERTIA);
		
		bcgpGestureManager.SetGestureConfig(GetSafeHwnd(), gestureConfig);
	}

	return TRUE;
}

#ifndef _AFX_NO_OCC_SUPPORT
//*****************************************************************************************
BOOL CBCGPDialogBar::SetOccDialogInfo(_AFX_OCC_DIALOG_INFO* pOccDialogInfo)
{
	m_pOccDialogInfo = pOccDialogInfo;
	return TRUE;
}
#endif //!_AFX_NO_OCC_SUPPORT
//*****************************************************************************************
BOOL CBCGPDialogBar::OnEraseBkgnd(CDC* pDC) 
{
	CRect rectClient;
	GetClientRect (rectClient);

	if (IsVisualManagerStyle ())
	{
#ifndef _BCGSUITE_
		if (IsRebarPane())
		{
			CBCGPVisualManager::GetInstance ()->FillRebarPane(pDC, this, rectClient);
			return TRUE;
		}
#endif
		if (CBCGPVisualManager::GetInstance ()->OnFillDialog (pDC, this, rectClient))
		{
			return TRUE;
		}
	}

	pDC->FillRect (rectClient, &globalData.brBtnFace);
	return TRUE;
}
//*****************************************************************************************
void CBCGPDialogBar::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
#ifndef _BCGSUITE_
	CPoint ptScr = point;
	ClientToScreen (&ptScr);

	int nHitTest = HitTest (ptScr, TRUE);

	switch (nHitTest)
	{
	case HTSCROLLUPBUTTON_BCG:
	case HTSCROLLDOWNBUTTON_BCG:
	case HTSCROLLLEFTBUTTON_BCG:
	case HTSCROLLRIGHTBUTTON_BCG:
	case HTCAPTION:
		CBCGPDockingControlBar::OnLButtonDblClk(nFlags, point);
		break;

	default:
		CWnd::OnLButtonDblClk(nFlags, point);
	}
#else
	CBCGPDockingControlBar::OnLButtonDblClk(nFlags, point);
#endif
}
//*****************************************************************************************
void CBCGPDialogBar::OnLButtonDown(UINT nFlags, CPoint point) 
{
#ifndef _BCGSUITE_
	CPoint ptScr = point;
	ClientToScreen (&ptScr);

	int nHitTest = HitTest (ptScr, TRUE);

	if (nHitTest == HTCAPTION || nHitTest == HTCLOSE_BCG || 
		nHitTest == HTMAXBUTTON || nHitTest == HTMINBUTTON ||
		nHitTest == HTSCROLLUPBUTTON_BCG || nHitTest == HTSCROLLDOWNBUTTON_BCG ||
		nHitTest == HTSCROLLLEFTBUTTON_BCG || nHitTest == HTSCROLLRIGHTBUTTON_BCG)
	{
		CBCGPDockingControlBar::OnLButtonDown(nFlags, point);
	}
	else
	{
		CWnd::OnLButtonDown(nFlags, point);
	}
#else
	CBCGPDockingControlBar::OnLButtonDown(nFlags, point);
#endif
}
//*****************************************************************************************
void CBCGPDialogBar::OnWindowPosChanging(WINDOWPOS FAR* lpwndpos) 
{
	CBCGPDockingControlBar::OnWindowPosChanging(lpwndpos);
	
	if (!CanBeResized ())
	{
		CSize sizeMin; 
		GetMinSize (sizeMin);
		
		if (IsHorizontal () && lpwndpos->cy < sizeMin.cy)
		{
			lpwndpos->cy = sizeMin.cy;
			lpwndpos->hwndInsertAfter = HWND_BOTTOM;
		}
		else if (!IsHorizontal () && lpwndpos->cx < sizeMin.cx)
		{
			lpwndpos->cx = sizeMin.cx;
			lpwndpos->hwndInsertAfter = HWND_BOTTOM;
		}
	}
}
//*****************************************************************************************
void CBCGPDialogBar::EnableVisualManagerStyle (BOOL bEnable, const CList<UINT,UINT>* plstNonSubclassedItems)
{
	ASSERT_VALID (this);

	m_Impl.EnableVisualManagerStyle (bEnable, FALSE, plstNonSubclassedItems);
}
//***************************************************************************************
HBRUSH CBCGPDialogBar::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	if (IsVisualManagerStyle ())
	{
		HBRUSH hbr = m_Impl.OnCtlColor (pDC, pWnd, nCtlColor);
		if (hbr != NULL)
		{
			return hbr;
		}
	}	

	return CBCGPDockingControlBar::OnCtlColor(pDC, pWnd, nCtlColor);
}
//***************************************************************************************
void CBCGPDialogBar::OnDestroy() 
{
	m_Impl.OnDestroy ();
	CBCGPDockingControlBar::OnDestroy();
}
//***************************************************************************************
void CBCGPDialogBar::OnEraseNCBackground (CDC* pDC, CRect rectBar)
{
#ifndef _BCGSUITE_
	if (IsVisualManagerStyle () &&
		CBCGPVisualManager::GetInstance ()->OnFillDialog (pDC, this, rectBar))
	{
		return;
	}

	CBCGPDockingControlBar::OnEraseNCBackground (pDC, rectBar);
#else
	CMFCVisualManager::GetInstance ()->OnFillBarBackground (pDC, this, rectBar, rectBar, 
		TRUE /* NC area */);
#endif
}
//****************************************************************************
void CBCGPDialogBar::EnableLayout(BOOL bEnable, CRuntimeClass* pRTC)
{
	m_Impl.EnableLayout(bEnable, pRTC, FALSE);
}
//****************************************************************************
int CBCGPDialogBar::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CBCGPDockingControlBar::OnCreate(lpCreateStruct) == -1)
		return -1;
	
    return m_Impl.OnCreate();
}
//****************************************************************************
void CBCGPDialogBar::OnSize(UINT nType, int cx, int cy) 
{
	CBCGPDockingControlBar::OnSize(nType, cx, cy);
	AdjustControlsLayout();
}
//****************************************************************************
BOOL CBCGPDialogBar::ScrollHorzAvailable(BOOL bLeft)
{
	BOOL bIsRTL = GetExStyle () & WS_EX_LAYOUTRTL;

	if (m_scrollSize.cx == 0 || bIsRTL)
	{
		return FALSE;
	}

	return (m_scrollRange.cx > 0) && (bLeft ? m_scrollPos.x > 0 : m_scrollPos.x < m_scrollRange.cx);
}
//****************************************************************************
BOOL CBCGPDialogBar::ScrollVertAvailable(BOOL bTop)
{
	if (m_scrollSize.cy == 0)
	{
		return FALSE;
	}

	return (m_scrollRange.cy > 0) && (bTop ? m_scrollPos.y > 0 : m_scrollPos.y < m_scrollRange.cy);
}
//****************************************************************************
void CBCGPDialogBar::OnScrollClient(UINT uiScrollCode)
{
#ifndef _BCGSUITE_
	if (m_scrollSize == CSize(0, 0))
	{
		return;
	}

	CRect rectClient;
	GetClientRect (rectClient);

	CSize sizeScroll (0, 0);

	switch (LOBYTE(uiScrollCode))
	{
	case SB_LEFT:
		sizeScroll.cx = -m_scrollPos.x;
		break;
	case SB_RIGHT:
		sizeScroll.cx = m_scrollSize.cx;
		break;
	case SB_LINELEFT:
		sizeScroll.cx -= m_scrollLine.cx;
		break;
	case SB_LINERIGHT:
		sizeScroll.cx += m_scrollLine.cx;
		break;
	case SB_PAGELEFT:
		sizeScroll.cx -= rectClient.Width();
		break;
	case SB_PAGERIGHT:
		sizeScroll.cx += rectClient.Width();
		break;
	}

	const int nScrollButtonSize = GetScrollButtonSize();

	if (m_scrollPos.x == 0 && sizeScroll.cx > 0)
	{
		sizeScroll.cx += nScrollButtonSize;
	}
	if ((m_scrollPos.x + sizeScroll.cx) <= nScrollButtonSize && sizeScroll.cx < 0)
	{
		sizeScroll.cx -= nScrollButtonSize;
	}

	switch (HIBYTE(uiScrollCode))
	{
	case SB_TOP:
		sizeScroll.cy = -m_scrollPos.y;
		break;
	case SB_BOTTOM:
		sizeScroll.cy = m_scrollSize.cy;
		break;
	case SB_LINEUP:
		sizeScroll.cy -= m_scrollLine.cy;
		break;
	case SB_LINEDOWN:
		sizeScroll.cy += m_scrollLine.cy;
		break;
	case SB_PAGEUP:
		sizeScroll.cy -= rectClient.Height();
		break;
	case SB_PAGEDOWN:
		sizeScroll.cy += rectClient.Height();
		break;
	}

	if (m_scrollPos.y == 0 && sizeScroll.cy > 0)
	{
		sizeScroll.cy += nScrollButtonSize;
	}
	if ((m_scrollPos.y + sizeScroll.cy) <= nScrollButtonSize && sizeScroll.cy < 0)
	{
		sizeScroll.cy -= nScrollButtonSize;
	}

	ScrollClient(sizeScroll);
#else
	UNREFERENCED_PARAMETER(uiScrollCode);
#endif
}
//****************************************************************************
void CBCGPDialogBar::ScrollClient(const CSize& sizeScroll)
{
	if (sizeScroll == CSize(0, 0) || m_scrollSize == CSize(0, 0))
	{
		return;
	}

	CPoint ptScroll(m_scrollPos);

	ptScroll.x = min (max (ptScroll.x + sizeScroll.cx, 0), m_scrollRange.cx);
	ptScroll.y = min (max (ptScroll.y + sizeScroll.cy, 0), m_scrollRange.cy);

	if (ptScroll == m_scrollPos)
	{
		return;
	}

	LockWindowUpdate ();

	CWnd* pWndChild = GetWindow (GW_CHILD);
	while (pWndChild != NULL)
	{
		CRect rectChild;
		pWndChild->GetWindowRect (rectChild);
		ScreenToClient (rectChild);

		rectChild.OffsetRect (m_scrollPos.x - ptScroll.x, m_scrollPos.y - ptScroll.y);

		pWndChild->SetWindowPos (NULL, rectChild.left, rectChild.top,
			-1, -1, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

		pWndChild = pWndChild->GetNextWindow ();
	}

	m_scrollPos = ptScroll;

	UnlockWindowUpdate ();

	RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
}
//****************************************************************************
void CBCGPDialogBar::UpdateScrollBars()
{
#ifndef _BCGSUITE_
    if(GetSafeHwnd() == NULL || m_scrollSize == CSize(0, 0))
    {
        return;
    }

	if (m_bUpdateScroll)
	{
		return;
	}

	m_bUpdateScroll = TRUE;

	CRect rectClient;
	GetClientRect(rectClient);
	CSize sizeClient(rectClient.Size());

	CSize sizeRange (m_scrollSize - sizeClient);

	CPoint ptScroll (m_scrollPos);
	ASSERT(ptScroll.x >= 0 && ptScroll.y >= 0);

	int nScrollButtonSize = GetScrollButtonSize();

	BOOL bNeedH = sizeRange.cx > 0;
	if (!bNeedH)
	{
		ptScroll.x = 0;
		sizeRange.cx = 0;
	}
	else
	{
		if (ScrollHorzAvailable (TRUE) && ScrollHorzAvailable (FALSE))
		{
			sizeRange.cx -= nScrollButtonSize;
		}
	}

	BOOL bNeedV = sizeRange.cy > 0;
	if (!bNeedV)
	{
		ptScroll.y = 0;
		sizeRange.cy = 0;
	}
	else
	{
		if (ScrollVertAvailable (TRUE) && ScrollVertAvailable (FALSE))
		{
			sizeRange.cy -= nScrollButtonSize;
		}
	}

	if (sizeRange.cx > 0 && ptScroll.x >= sizeRange.cx)
	{
		ptScroll.x = sizeRange.cx;
	}
	if (sizeRange.cy > 0 && ptScroll.y >= sizeRange.cy)
	{
		ptScroll.y = sizeRange.cy;
	}

	if (sizeRange.cx <= nScrollButtonSize)
	{
		sizeRange.cx = 0;
	}

	if (sizeRange.cy <= nScrollButtonSize)
	{
		sizeRange.cy = 0;
	}

	m_scrollRange = sizeRange;

	ScrollClient(CSize(m_scrollPos.x - ptScroll.x, m_scrollPos.y - ptScroll.y));

	CalcScrollButtons ();

	m_bUpdateScroll = FALSE;
#endif
}
//****************************************************************************
LRESULT CBCGPDialogBar::OnChangeVisualManager (WPARAM wp, LPARAM lp)
{
	if (IsVisualManagerStyle ())
	{
		m_Impl.OnChangeVisualManager ();
	}

#ifndef _BCGSUITE_
	return CBCGPDockingControlBar::OnChangeVisualManager(wp, lp);
#else
	UNREFERENCED_PARAMETER(wp);
	UNREFERENCED_PARAMETER(lp);
	return 0;
#endif
}
//*********************************************************************************
BOOL CBCGPDialogBar::OnGestureEventPan(const CPoint& ptFrom, const CPoint& ptTo, CSize& sizeOverPan)
{
	if (m_scrollSize == CSize(0, 0))
	{
		return FALSE;
	}

	if (ptTo.x != ptFrom.x)
	{
		int nDelta = ptTo.x - ptFrom.x;
		BOOL bRes = FALSE;
		
		BOOL bLeft  = ScrollHorzAvailable(TRUE);
		BOOL bRight = ScrollHorzAvailable(FALSE);
		
		if (bLeft || bRight)
		{
			if (nDelta > 0 && ScrollHorzAvailable(TRUE))
			{
				OnScrollClient(MAKEWORD(SB_LINELEFT, -1));
				bRes = TRUE;
			}
			else if (nDelta < 0 && ScrollHorzAvailable(FALSE))
			{
				OnScrollClient(MAKEWORD(SB_LINERIGHT, -1));
				bRes = TRUE;
			}
			
			if (bRes)
			{
				if (bLeft != ScrollHorzAvailable(TRUE) || bRight != ScrollHorzAvailable(FALSE))
				{
					SetWindowPos (NULL, 0, 0, 0, 0, 
						SWP_NOZORDER | SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE | SWP_FRAMECHANGED);
				}
				
				return TRUE;
			}
			
			sizeOverPan.cx = m_scrollLine.cx;
			return TRUE;
		}
	}
	else if (ptTo.y != ptFrom.y)
	{
		int nDelta = ptTo.y - ptFrom.y;
		BOOL bRes = FALSE;
		
		BOOL bUp   = ScrollVertAvailable(TRUE);
		BOOL bDown = ScrollVertAvailable(FALSE);
		
		if (bUp || bDown)
		{
			if (nDelta > 0 && ScrollVertAvailable(TRUE))
			{
				OnScrollClient(MAKEWORD(-1, SB_LINEUP));
				bRes = TRUE;
			}
			else if (nDelta < 0 && ScrollVertAvailable(FALSE))
			{
				OnScrollClient(MAKEWORD(-1, SB_LINEDOWN));
				bRes = TRUE;
			}
			
			if (bRes)
			{
				if (bUp != ScrollVertAvailable(TRUE) || bDown != ScrollVertAvailable(FALSE))
				{
					SetWindowPos (NULL, 0, 0, 0, 0, 
						SWP_NOZORDER | SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE | SWP_FRAMECHANGED);
				}
				
				return TRUE;
			}

			sizeOverPan.cy = m_scrollLine.cy;
			return TRUE;
		}
	}
		
	return FALSE;
}
