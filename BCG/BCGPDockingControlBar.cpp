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
// BCGPDockingControlBar.cpp : implementation file
//

#include "stdafx.h"

#include "BCGGlobals.h"
#include "BCGPGlobalUtils.h"
#include "BCGPFrameWnd.h"
#include "BCGPMDIFrameWnd.h"
#include "BCGPOleIPFrameWnd.h"
#include "BCGPOleDocIPFrameWnd.h"
#include "BCGPMiniFrameWnd.h"
#include "BCGPMultiMiniFrameWnd.h"
#include "BCGPBaseControlBar.h"
#include "BCGPDockBarRow.h"
#include "BCGPTabbedControlBar.h"
#include "BCGPDrawManager.h"
#include "BCGPAutoHideButton.h"
#include "BCGPAutoHideToolBar.h"
#include "BCGPAutoHideDockBar.h"
#include "BCGPSlider.h"
#include "BCGPLocalResource.h"
#include "BCGProRes.h"
#include "BCGPDockingControlBar.h"
#include "BCGPBarContainerManager.h"
#include "BCGPOutlookBar.h"
#include "BCGPMultiMiniFrameWnd.h"
#include "BCGPInplaceToolTipCtrl.h"
#include "BCGPTooltipManager.h"
#include "BCGPMDIChildWnd.h"
#include "BCGPWorkspace.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CBCGPWorkspace* g_pWorkspace;

int  CBCGPDockingControlBar::m_nTimeOutBeforeAutoHide	= 700;
int  CBCGPDockingControlBar::m_nSlideDefaultTimeOut		= 1;
BOOL CBCGPDockingControlBar::m_bHideInAutoHideMode		= FALSE;
int  CBCGPDockingControlBar::m_nSlideSteps				= 12;
int  CBCGPDockingControlBar::m_nScrollTimeOut			= 50;

static int g_nCloseButtonMargin = 1;
static int g_nCaptionVertMargin = 2;
static int g_nCaptionHorzMargin = 2;

CSize CBCGPDockingControlBar::m_sizeDragSencitivity = CSize (GetSystemMetrics (SM_CXDRAG), 
															 GetSystemMetrics (SM_CYDRAG));

BOOL CBCGPDockingControlBar::m_bCaptionText = FALSE;
BOOL CBCGPDockingControlBar::m_bHideDisabledButtons = TRUE;
BOOL CBCGPDockingControlBar::m_bDisableAnimation = FALSE;

IMPLEMENT_SERIAL(CBCGPDockingControlBar, CBCGPControlBar, VERSIONABLE_SCHEMA | 2)

UINT BCGM_ON_PRESS_CLOSE_BUTTON	= ::RegisterWindowMessage (_T("BCGM_ON_PRESS_CLOSE_BUTTON"));

#define UM_REDRAWFRAME	(WM_USER + 1001)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGPDockingControlBar::CBCGPDockingControlBar()
{
	m_bPrepareToFloat = false;	
	m_bReadyToFloat = false;

	m_pTabbedControlBarRTC = RUNTIME_CLASS (CBCGPTabbedControlBar);

	m_hDefaultSlider = NULL;
	m_cyGripper = 0;
	m_bHasGripper = FALSE;
	m_nBorderSize = 0;
	m_dwSCBStyle = 0;
	m_bActive = FALSE;

	m_bEnableAutoHideAll = TRUE;

	m_bPinState = FALSE;
	m_nAutoHideConditionTimerID = 0;
	m_nSlideTimer = 0;
	m_nSlideStep = 0;
	m_nSlideDelta = 0;
	m_pAutoHideButton = NULL;
	m_pAutoHideBar = NULL;

	m_ahSlideMode = CBCGPDockManager::m_ahSlideModeGlobal;

	m_bIsSliding = FALSE;
	m_bIsHiding = FALSE;
	m_bIsResizing = FALSE;

	m_nLastPercent = 100;

	m_rectRedraw.SetRectEmpty ();
	m_rectRestored.SetRectEmpty ();

	m_nHot = HTNOWHERE;
	m_nHit = HTNOWHERE;
	m_bCaptionButtonsCaptured = FALSE;

	m_hRestoredDefaultSlider = NULL;

	m_pToolTip = NULL;

	m_arScrollButtons.Add(new CBCGPDockingBarScrollButton(HTSCROLLUPBUTTON_BCG));
	m_arScrollButtons.Add(new CBCGPDockingBarScrollButton(HTSCROLLDOWNBUTTON_BCG));
	m_arScrollButtons.Add(new CBCGPDockingBarScrollButton(HTSCROLLLEFTBUTTON_BCG));
	m_arScrollButtons.Add(new CBCGPDockingBarScrollButton(HTSCROLLRIGHTBUTTON_BCG));
}

CBCGPDockingControlBar::~CBCGPDockingControlBar()
{
	for (int i = 0; i < m_arScrollButtons.GetSize(); i++)
	{
		delete m_arScrollButtons[i];
	}

	m_arScrollButtons.RemoveAll();
}

BEGIN_MESSAGE_MAP(CBCGPDockingControlBar, CBCGPControlBar)
	//{{AFX_MSG_MAP(CBCGPDockingControlBar)
	ON_WM_NCCALCSIZE()
	ON_WM_NCPAINT()
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_NCLBUTTONDOWN()
	ON_WM_CLOSE()
	ON_WM_CREATE()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_DESTROY()
	ON_WM_NCMOUSEMOVE()
	ON_WM_CANCELMODE()
	ON_WM_TIMER()
	ON_WM_RBUTTONDOWN()
	ON_WM_SETTINGCHANGE()
	ON_WM_CONTEXTMENU()
	ON_WM_SETFOCUS()
	ON_WM_SIZE()
	ON_WM_NCHITTEST()
	ON_WM_MOUSEWHEEL()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_SETTEXT, OnSetText)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXT, 0, 0xFFFF, OnNeedTipText)
	ON_REGISTERED_MESSAGE(BCGM_UPDATETOOLTIPS, OnBCGUpdateToolTips)
	ON_MESSAGE(UM_REDRAWFRAME, OnRedrawFrame)
	ON_REGISTERED_MESSAGE(BCGM_CHANGEVISUALMANAGER, OnChangeVisualManager)
END_MESSAGE_MAP()

//***********************************************************************
BOOL CBCGPDockingControlBar::Create(LPCTSTR lpszCaption, 
									CWnd* pParentWnd, 
									const RECT& rect, 
									BOOL bHasGripper, 
									UINT nID, 
									DWORD dwStyle, 
									DWORD dwTabbedStyle,
									DWORD dwBCGStyle,
									CCreateContext* pContext)
{
	ASSERT_VALID (this);
	return CBCGPDockingControlBar::CreateEx (0, lpszCaption, pParentWnd, rect, 
											 bHasGripper, nID, dwStyle, dwTabbedStyle, 
											 dwBCGStyle, pContext);
}
//*****************************************************************************************
BOOL CBCGPDockingControlBar::Create(LPCTSTR lpszWindowName, 
									CWnd* pParentWnd,
									CSize sizeDefault, 
									BOOL bHasGripper,
									UINT nID, 
									DWORD dwStyle, 
									DWORD dwTabbedStyle, 
									DWORD dwBCGStyle)
{
	ASSERT_VALID (this);
	CRect rect (0, 0, sizeDefault.cx, sizeDefault.cy);
	return CBCGPDockingControlBar::CreateEx (0, lpszWindowName, pParentWnd, rect, 
											 bHasGripper, nID, dwStyle, dwTabbedStyle, 
											 dwBCGStyle, NULL);
}
//***********************************************************************
BOOL CBCGPDockingControlBar::CreateEx (DWORD dwStyleEx, 
									   LPCTSTR lpszCaption, 
									   CWnd* pParentWnd, 
									   const RECT& rect, 
									   BOOL bHasGripper, 
									   UINT nID, 
									   DWORD dwStyle, 
									   DWORD dwTabbedStyle, 
									   DWORD dwBCGStyle,
									   CCreateContext* pContext)
{
	ASSERT_VALID (this);

	if (dwStyle & CBRS_FLOAT_MULTI)
	{
		m_pMiniFrameRTC = RUNTIME_CLASS (CBCGPMultiMiniFrameWnd);
	}

	if (dwTabbedStyle & CBRS_BCGP_OUTLOOK_TABS)
	{
		m_pTabbedControlBarRTC = RUNTIME_CLASS (CBCGPOutlookBar);
	}
	else if (dwTabbedStyle & CBRS_BCGP_REGULAR_TABS)
	{
		m_pTabbedControlBarRTC = RUNTIME_CLASS (CBCGPTabbedControlBar);
	}

	if (dwStyle & WS_CAPTION || bHasGripper)
	{
		m_bHasGripper = bHasGripper = TRUE;
		dwStyle &= ~WS_CAPTION;
	}


	if (!CBCGPControlBar::CreateEx (dwStyleEx, NULL, dwStyle, rect, pParentWnd, nID, 
											 dwBCGStyle, pContext))
	{
		return FALSE;
	}
	
	m_rectRestored = rect;

	SetBarAlignment (dwStyle & CBRS_ALIGN_ANY);
	EnableGripper (bHasGripper);

	if (m_sizeDialog != CSize (0, 0))
	{
		m_sizeDialog.cy += GetCaptionHeight();
		m_rectRestored.right = m_rectRestored.left + m_sizeDialog.cx;
		m_rectRestored.bottom = m_rectRestored.top + m_sizeDialog.cy;
	}
	
	if (lpszCaption != NULL)
	{
		SetWindowText (lpszCaption);
	}

	return TRUE;
}
//***********************************************************************
void CBCGPDockingControlBar::EnableGripper (BOOL bEnable)
{
	if (bEnable && m_bHasGripper)
	{
		if (CBCGPVisualManager::GetInstance()->UseLargeCaptionFontInDockingCaptions())
		{
			m_cyGripper = globalData.GetCaptionTextHeight();
		}
		else
		{
			m_cyGripper = globalData.GetTextHeight ();
		}

		m_cyGripper = m_cyGripper - globalData.GetTextMargins () + g_nCaptionVertMargin * 2 + 3;
	}
	else
	{
		m_cyGripper = 0;
	}

	BOOL bClosing = FALSE;
	CWnd* pFrame = AfxGetMainWnd ();

	if (pFrame != NULL)
	{
		ASSERT_VALID (pFrame);

		if (pFrame->IsKindOf (RUNTIME_CLASS (CBCGPMDIFrameWnd)))
		{
			CBCGPMDIFrameWnd* pMDIFrame = DYNAMIC_DOWNCAST (CBCGPMDIFrameWnd, pFrame);
			if (pMDIFrame != NULL)
			{
				bClosing = pMDIFrame->IsClosing ();
			}
		}
		else if (pFrame->IsKindOf (RUNTIME_CLASS (CBCGPFrameWnd)))
		{
			CBCGPFrameWnd* pSDIFrame = DYNAMIC_DOWNCAST (CBCGPFrameWnd, pFrame);
			if (pSDIFrame != NULL)
			{
				bClosing = pSDIFrame->IsClosing ();
			}	
		}
	}

	if (!bClosing)
	{
		SetWindowPos (NULL, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | 
										SWP_NOACTIVATE | SWP_FRAMECHANGED);
	}
}
//***********************************************************************
int CBCGPDockingControlBar::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CBCGPControlBar::OnCreate(lpCreateStruct) == -1)
		return -1;

	SetCaptionButtons ();

	if (CBCGPTooltipManager::CreateToolTip (m_pToolTip, this,
		BCGP_TOOLTIP_TYPE_DOCKBAR))
	{
		for (int i = 0; i < (int) m_arrButtons.GetSize (); i ++)
		{
			CBCGPLocalResource locaRes;

			CRect rectDummy;
			rectDummy.SetRectEmpty ();

			m_pToolTip->AddTool (this, LPSTR_TEXTCALLBACK, &rectDummy, i + 1);
		}
	}

	return 0;
}
//***********************************************************************
BOOL CBCGPDockingControlBar::IsDocked () const
{
	ASSERT_VALID (this);
	CBCGPMiniFrameWnd* pParent = GetParentMiniFrame ();
	
	if (pParent != NULL && pParent->GetControlBarCount () == 1)
	{
		return FALSE;
	}

	return TRUE;
}
//***********************************************************************
void CBCGPDockingControlBar::OnAfterDock  (CBCGPBaseControlBar* /*pBar*/, LPCRECT /*lpRect*/, BCGP_DOCK_METHOD /*dockMethod*/) 
{
	if (!CBCGPDockManager::m_bRestoringDockState)
	{
		SetFocus ();
	}

	if (GetDockMode () == BCGP_DT_IMMEDIATE)
	{
		GetCursorPos (&m_ptClientHotSpot);
		ScreenToClient (&m_ptClientHotSpot);
	}

	if (GetDlgCtrlID () != -1 && GetParentMiniFrame () == NULL)
	{
		CBCGPMiniFrameWnd::AddRemoveBarFromGlobalList (this, FALSE /* remove*/);
	}


	UpdateScrollBars();
	SetWindowPos (NULL, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | 
									SWP_NOACTIVATE | SWP_FRAMECHANGED);
}
//***********************************************************************
void CBCGPDockingControlBar::OnBeforeChangeParent (CWnd* pWndNewParent, BOOL bDelay)
{
	ASSERT_VALID (this);

	if (pWndNewParent != NULL)
	{
		BOOL bIsMDIChild = pWndNewParent->IsKindOf (RUNTIME_CLASS (CBCGPMDIChildWnd));

		if (bIsMDIChild)
		{
			StoreRecentDockInfo ();
		}

		// is being floated or tabbed
		if (pWndNewParent->IsKindOf (RUNTIME_CLASS (CBCGPMiniFrameWnd)) ||
			pWndNewParent->IsKindOf (RUNTIME_CLASS (CBCGPTabWnd)) ||
			bIsMDIChild)
		{
			UnDockControlBar (bDelay);
		}

		CBCGPControlBar::OnBeforeChangeParent (pWndNewParent);
	}
}
//***********************************************************************
void CBCGPDockingControlBar::RemoveFromDefaultSlider ()
{
	ASSERT_VALID (this);

	if (m_hDefaultSlider != NULL)
	{
		// slider will be deleted here (by delete this) if it was a last
		// control bar registered with the slider
		SetDefaultSlider (NULL);
	}
}
//***********************************************************************
void CBCGPDockingControlBar::OnAfterChangeParent(CWnd* pWndOldParent)
{
	ASSERT_VALID (this);

	CBCGPControlBar::OnAfterChangeParent (pWndOldParent);

	CBCGPMultiMiniFrameWnd* pMiniFrameParentOld = DYNAMIC_DOWNCAST(CBCGPMultiMiniFrameWnd, pWndOldParent);
	if (pMiniFrameParentOld->GetSafeHwnd() != NULL && pMiniFrameParentOld->GetControlBarCount() == 1)
	{
		pMiniFrameParentOld->AdjustBarFrames();
	}

	CBCGPMiniFrameWnd* pMiniFrameParent = GetParentMiniFrame ();
	if (pMiniFrameParent != NULL)
	{
		pMiniFrameParent->AddRemoveBarFromGlobalList (this, TRUE);
	}
}
//***********************************************************************
void CBCGPDockingControlBar::UpdateTooltips ()
{
	if (m_pToolTip->GetSafeHwnd () == NULL)
	{
		return;
	}

    CRect rcBar;
    GetWindowRect(rcBar);
	ScreenToClient (rcBar);

	for (int i = 0; i < m_arrButtons.GetSize () && i < m_pToolTip->GetToolCount( ); i ++)
	{
		CBCGPCaptionButton* pbtn = m_arrButtons [i];
		ASSERT_VALID (pbtn);

		CRect rectTT = pbtn->GetRect ();
		rectTT.OffsetRect (rcBar.TopLeft());
		m_pToolTip->SetToolRect (this, i + 1, rectTT);
	}
}
//***********************************************************************
void CBCGPDockingControlBar::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp) 
{
	ASSERT_VALID (this);
	CBCGPControlBar::OnNcCalcSize(bCalcValidRects, lpncsp);

	if (IsFloating ())
	{
		for (int i = 0; i < m_arrButtons.GetSize (); i ++)
		{
			CBCGPCaptionButton* pbtn = m_arrButtons [i];
			ASSERT_VALID (pbtn);

			pbtn->m_bHidden = TRUE;
		}

		CalcScrollButtons();

		lpncsp->rgrc[0].top += m_arScrollButtons[0]->GetRect().Height();
		lpncsp->rgrc[0].bottom -= m_arScrollButtons[1]->GetRect().Height();
		lpncsp->rgrc[0].left += m_arScrollButtons[2]->GetRect().Width();
		lpncsp->rgrc[0].right -= m_arScrollButtons[3]->GetRect().Width();

		return;
	}

	int cyGripper = GetCaptionHeight ();

	CRect rcClient = lpncsp->rgrc[0];
	rcClient.DeflateRect (0, cyGripper, 0, 0);

	// "hide" and "expand" buttons positioning:
	CSize sizeButton = CBCGPCaptionButton::GetSize ();
    CPoint ptOrgBtnRight = CPoint (rcClient.right - sizeButton.cx - g_nCaptionHorzMargin,
		rcClient.top - cyGripper - m_nBorderSize + (cyGripper - sizeButton.cy) / 2);
    CPoint ptOrgBtnLeft = CPoint (rcClient.left + g_nCaptionHorzMargin, ptOrgBtnRight.y);

	CRect rcBar;
    GetWindowRect(rcBar);
	ScreenToClient (rcBar);

	BOOL bHidePinBtn = !CanAutoHide ();

	if (cyGripper > 0)
	{
		int i = 0;

		for (i = 0; i < m_arrButtons.GetSize (); i ++)
		{
			CBCGPCaptionButton* pbtn = m_arrButtons [i];
			ASSERT_VALID (pbtn);

			UINT unHit = pbtn->GetHit ();

			BOOL bHide = FALSE;
			if (m_bHideDisabledButtons)
			{
				bHide = bHidePinBtn && unHit == HTMAXBUTTON || 
						!CanBeClosed () && unHit == HTCLOSE_BCG;
			}

			if (!CBCGPDockManager::IsDockBarMenu () &&
				unHit == HTMINBUTTON)
			{
				bHide = TRUE;
			}

			pbtn->m_bFocused = pbtn->m_bPushed = FALSE;

			if (pbtn->m_bLeftAlign)
			{
				pbtn->Move (ptOrgBtnLeft - CRect (lpncsp->rgrc[0]).TopLeft (), bHide);

				if (!bHide)
				{
					ptOrgBtnLeft.Offset (sizeButton.cx + 2, 0);
				}
			}
			else
			{
				pbtn->Move (ptOrgBtnRight - CRect (lpncsp->rgrc[0]).TopLeft (), bHide);

				if (!bHide)
				{
					ptOrgBtnRight.Offset (- sizeButton.cx - 2, 0);
				}
			}
		}

		// Hide left aligned buttons if there is no room for them:
		for (i = 0; i < m_arrButtons.GetSize (); i ++)
		{
			CBCGPCaptionButton* pbtn = m_arrButtons [i];
			ASSERT_VALID (pbtn);

			if (pbtn->m_bLeftAlign)
			{
				pbtn->m_bHidden = CRect (lpncsp->rgrc[0]).left + pbtn->GetRect ().left >= ptOrgBtnRight.x;
			}
		}
	}
	else
	{
		for (int i = 0; i < m_arrButtons.GetSize (); i ++)
		{
			CBCGPCaptionButton* pbtn = m_arrButtons [i];
			ASSERT_VALID (pbtn);

			pbtn->m_bHidden = TRUE;
		}
	}

	rcClient.right = max(rcClient.right, rcClient.left);
	rcClient.bottom = max(rcClient.bottom, rcClient.top);

	CalcScrollButtons();

	rcClient.top += m_arScrollButtons[0]->GetRect().Height();
	rcClient.bottom -= m_arScrollButtons[1]->GetRect().Height();
	rcClient.left += m_arScrollButtons[2]->GetRect().Width();
	rcClient.right -= m_arScrollButtons[3]->GetRect().Width();

	lpncsp->rgrc[0] = rcClient;

	UpdateTooltips ();
}
//***********************************************************************
void CBCGPDockingControlBar::DoNcPaint(CDC* pDCPaiint) 
{
	ASSERT_VALID(pDCPaiint);

	CRect rectUpd;
	GetUpdateRect (rectUpd);

    CRect rcClient, rcBar;
    GetClientRect(rcClient);
    ClientToScreen(rcClient);
    GetWindowRect(rcBar);

    rcClient.OffsetRect(-rcBar.TopLeft());
    rcBar.OffsetRect(-rcBar.TopLeft());

	CDC*		pDC = pDCPaiint;
	BOOL		m_bMemDC = FALSE;
	CDC			dcMem;
	CBitmap		bmp;
	CBitmap*	pOldBmp = NULL;

	if (dcMem.CreateCompatibleDC (pDCPaiint) &&
		bmp.CreateCompatibleBitmap (pDCPaiint, rcBar.Width (), rcBar.Height ()))
	{
		//-------------------------------------------------------------
		// Off-screen DC successfully created. Better paint to it then!
		//-------------------------------------------------------------
		m_bMemDC = TRUE;
		pOldBmp = dcMem.SelectObject (&bmp);
		pDC = &dcMem;
	}

    // client area is not our bussiness :)
    pDCPaiint->ExcludeClipRect(rcClient);

	CRgn rgn;
	if (!m_rectRedraw.IsRectEmpty ())
	{
		rgn.CreateRectRgnIndirect (m_rectRedraw);
		pDCPaiint->SelectClipRgn (&rgn);
	}

    // erase parts not drawn
    pDCPaiint->IntersectClipRect(rcBar);

    // erase NC background the hard way
	OnEraseNCBackground (pDC, rcBar);

	int cyGripper = GetCaptionHeight ();

    if (cyGripper > 0)
	{
		// Paint caption and buttons:
		CRect rectCaption;
		
		GetWindowRect (&rectCaption);
		ScreenToClient (&rectCaption);

		rectCaption.OffsetRect (-rectCaption.left, -rectCaption.top);
		rectCaption.DeflateRect (0, 1);

		rectCaption.left = rcClient.left - m_arScrollButtons[2]->GetRect().Width();
		rectCaption.top --;
		rectCaption.bottom = rectCaption.top + cyGripper - 2;

		DrawCaption (pDC, rectCaption);

		for (int i = 0; i < m_arrButtons.GetSize (); i ++)
		{
			CBCGPCaptionButton* pbtn = m_arrButtons [i];
			ASSERT_VALID (pbtn);

			BOOL bIsMax = FALSE;

			switch (pbtn->GetHit ())
			{
			case HTMAXBUTTON:
				bIsMax = m_bPinState;
				break;

			case HTMINBUTTON:
				bIsMax = TRUE;
				break;
			}

			pbtn->OnDraw (pDC, m_bActive, IsHorizontal (), bIsMax);
			pbtn->m_clrForeground = (COLORREF)-1;
		}
	}

	for (int nStep = 0; nStep < 2; nStep++)
	{
		for (int i = 0; i < m_arScrollButtons.GetSize(); i++)
		{
			BOOL bIsActive = m_arScrollButtons[i]->m_bFocused;

			if ((bIsActive && nStep == 1) || (!bIsActive && nStep == 0))
			{
				m_arScrollButtons[i]->OnDraw(pDC, FALSE, FALSE, FALSE);
			}
		}
	}

	if (m_bMemDC)
	{
		//--------------------------------------
		// Copy the results to the on-screen DC:
		//-------------------------------------- 
		pDCPaiint->BitBlt (rcBar.left, rcBar.top, rcBar.Width(), rcBar.Height(),
					   &dcMem, rcBar.left, rcBar.top, SRCCOPY);

		dcMem.SelectObject(pOldBmp);
	}

	pDCPaiint->SelectClipRgn (NULL);
}
//***********************************************************************
void CBCGPDockingControlBar::OnNcPaint() 
{
	if (m_bMultiThreaded)
	{
		g_cs.Lock ();
	}

	ASSERT_VALID (this);
	
	// get window DC that is clipped to the non-client area
    CWindowDC dcPaint (this);

	DoNcPaint(&dcPaint);

	if (m_bMultiThreaded)
	{
		g_cs.Unlock ();
	}
}
//***********************************************************************
void CBCGPDockingControlBar::OnEraseNCBackground (CDC* pDC, CRect rcBar)
{
	ASSERT_VALID (this);

	CBCGPVisualManager::GetInstance ()->OnFillBarBackground (pDC, this, rcBar, rcBar, 
		TRUE /* NC area */);
}
//***********************************************************************
void CBCGPDockingControlBar::OnDrawDragRect (LPCRECT lprectNew, LPCRECT lprectOld)
{
	ASSERT_VALID (this);
	CWindowDC dcWnd (GetDesktopWindow ());
	dcWnd.DrawDragRect (lprectNew, CSize (1, 1), lprectOld, CSize (1, 1));
}
//***********************************************************************
BCGNcHitTestType CBCGPDockingControlBar::OnNcHitTest(CPoint point) 
{
	ASSERT_VALID (this);
	UINT nHitTest = HitTest (point);
	if (nHitTest != HTERROR)
	{
		return nHitTest;
	}
	return CBCGPControlBar::OnNcHitTest(point);
}
//***********************************************************************
int CBCGPDockingControlBar::HitTest (CPoint point, BOOL bDetectCaption)
{
	ASSERT_VALID (this);
	CRect rectWnd;
	GetWindowRect (&rectWnd);

	if (!rectWnd.PtInRect (point))
	{
		return HTNOWHERE;
	}

	CBCGPDockManager* pDockManager = globalUtils.GetDockManager (GetDockSite ());
	ASSERT (pDockManager != NULL || globalUtils.m_bDialogApp);	

	// should return hite test of client or caption only in the lock update mode
	if (pDockManager != NULL && !pDockManager->m_bLockUpdate)
	{
		int i = 0;

		for (i = 0; i < m_arrButtons.GetSize (); i ++)
		{
			CBCGPCaptionButton* pbtn = m_arrButtons [i];
			ASSERT_VALID (pbtn);

			CRect rc = pbtn->GetRect();
			rc.OffsetRect(rectWnd.TopLeft());
			if (rc.PtInRect(point))
			{
				return pbtn->GetHit ();
			}
		}

		for (i = 0; i < m_arScrollButtons.GetSize(); i++)
		{
			CRect rectScroll = m_arScrollButtons[i]->GetRect();
			rectScroll.OffsetRect (rectWnd.TopLeft());

			if (rectScroll.PtInRect(point))
			{
				return m_arScrollButtons[i]->GetHit();
			}
		}
	}

	CRect rectClient;
	GetClientRect (&rectClient);
	ClientToScreen (&rectClient);

	if (rectClient.PtInRect (point))
	{
		return HTCLIENT;
	}

	if (IsDocked ())
	{
		CRect rect;
		int nBorderWidth  = 0;
		int nBorderHeight = 1;
		// caption
		rect.SetRect (rectWnd.left + nBorderWidth, rectWnd.top + nBorderHeight, 
							 rectWnd.right - nBorderWidth, 
							 rectWnd.top + nBorderHeight + GetCaptionHeight ());
		rect.bottom += m_arScrollButtons[0]->GetRect().Height();

		if (rect.PtInRect (point))
		{
			return bDetectCaption ? HTCAPTION : HTCLIENT;
		}
	}

	return HTERROR;
}
//***********************************************************************
CSize CBCGPDockingControlBar::CalcFixedLayout(BOOL /*bStretch*/, BOOL /*bHorz*/)
{
	ASSERT_VALID (this);
	CRect rectWnd;
	GetWindowRect (&rectWnd);
	CSize size = rectWnd.Size ();
	return size;
}
//***********************************************************************
void CBCGPDockingControlBar::OnPaint() 
{
	ASSERT_VALID (this);
	CPaintDC dc(this); // device context for painting
}
//***********************************************************************
BCGP_CS_STATUS CBCGPDockingControlBar::IsChangeState (int nOffset, 
													CBCGPBaseControlBar** ppTargetBar) const
{
	ASSERT_VALID (this);
	ASSERT (ppTargetBar != NULL);

	CPoint ptMouse;
	GetCursorPos (&ptMouse);

	CWnd* pParentWnd = GetParent ();

	if (pParentWnd->IsKindOf (RUNTIME_CLASS (CBCGPMiniFrameWnd)))
	{
		CBCGPMiniFrameWnd* pMiniFrame = DYNAMIC_DOWNCAST (CBCGPMiniFrameWnd, pParentWnd);
		pParentWnd = pMiniFrame->GetParent ();
	}

	CBCGPDockManager* pDockManager = globalUtils.GetDockManager (pParentWnd);
	
	if (pDockManager == NULL)
	{
		return BCGP_CS_NOTHING;
	}

	return pDockManager->DetermineControlBarAndStatus (ptMouse, nOffset, 
				GetEnabledAlignment (), ppTargetBar, this, this);
}
//***********************************************************************
void CBCGPDockingControlBar::OnLButtonDown(UINT nFlags, CPoint point) 
{
	ASSERT_VALID (this);

	if (m_nHot != HTNOWHERE)
	{
		CBCGPCaptionButton* pBtn = FindButtonByHit (m_nHot);
		if (pBtn != NULL)
		{
			SetFocus ();

			m_nHit = m_nHot;
			pBtn->m_bPushed = TRUE;
			RedrawButton (pBtn);

			switch (m_nHit)
			{
			case HTSCROLLUPBUTTON_BCG:
			case HTSCROLLDOWNBUTTON_BCG:
			case HTSCROLLLEFTBUTTON_BCG:
			case HTSCROLLRIGHTBUTTON_BCG:
				SetTimer(BCGP_SCROLL_TIMER_ID, m_nScrollTimeOut, NULL);
			}

			return;
		}
	}
	else
	{
		CWnd* pWndChild = GetWindow (GW_CHILD);
		CWnd* pWndFirstChild = NULL;
		int nCount = 0;

		while (pWndChild != NULL)
		{
			pWndFirstChild = pWndChild;
			pWndChild = pWndChild->GetNextWindow ();
			nCount++;
		}

		if (nCount == 1)
		{
			pWndFirstChild->SetFocus ();
		}
	}

	if (!IsAutoHideMode () && !IsTabbed ())
	{
		if (CanFloat ())
		{
			m_bPrepareToFloat = true;
		}

		CBCGPControlBar::OnLButtonDown(nFlags, point);
	}
	
	SetFocus ();
}
//***********************************************************************
void CBCGPDockingControlBar::StoreRecentDockInfo ()
{
	CBCGPMiniFrameWnd* pParentMiniFrame = GetParentMiniFrame ();

	CBCGPDockingControlBar* pBarToSave = this;

	if (IsTabbed ())
	{
		CBCGPBaseTabWnd* pTabWnd = 
			DYNAMIC_DOWNCAST (CBCGPBaseTabWnd, GetParent ());
		if (pTabWnd != NULL)
		{
			pBarToSave = 
				DYNAMIC_DOWNCAST (CBCGPDockingControlBar, pTabWnd->GetParent ());	
		}
	}			

	CBCGPSlider* pDefaultSlider = pBarToSave->GetDefaultSlider ();

	if (pParentMiniFrame != NULL)
	{
		pParentMiniFrame->StoreRecentDockInfo (pBarToSave);
	}
	else if (pDefaultSlider != NULL)
	{
		pDefaultSlider->StoreRecentDockInfo (pBarToSave);
	}
}
//***********************************************************************
void CBCGPDockingControlBar::StoreRecentTabRelatedInfo ()
{
	if (!IsTabbed ())
	{
		return;
	}

	CBCGPDockingControlBar* pParentTabbedBar = NULL;

	CBCGPBaseTabWnd* pTabWnd = 
			DYNAMIC_DOWNCAST (CBCGPBaseTabWnd, GetParent ());
	if (pTabWnd != NULL)
	{
		pParentTabbedBar = 
			DYNAMIC_DOWNCAST (CBCGPDockingControlBar, pTabWnd->GetParent ());	
	}

	if (pParentTabbedBar == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	CBCGPMiniFrameWnd* pParentMiniFrame = GetParentMiniFrame ();
	CBCGPSlider* pDefaultSlider = pParentTabbedBar->GetDefaultSlider ();

	if (pParentMiniFrame != NULL)
	{
		pParentMiniFrame->StoreRecentTabRelatedInfo (this, pParentTabbedBar);
	}
	else if (pDefaultSlider != NULL)
	{
		pDefaultSlider->StoreRecentTabRelatedInfo (this, pParentTabbedBar); 
	}
}
//***********************************************************************
void CBCGPDockingControlBar::OnRButtonDown(UINT nFlags, CPoint point) 
{
	SetFocus ();

	CWnd* pMenu = CBCGPPopupMenu::GetActiveMenu ();

	if (pMenu != NULL && CWnd::FromHandlePermanent (pMenu->GetSafeHwnd ()) != NULL)
	{	
		CBCGPPopupMenu::UpdateAllShadows ();
	}
	
	CBCGPControlBar::OnRButtonDown(nFlags, point);
}
//***********************************************************************
void CBCGPDockingControlBar::OnMouseMove(UINT nFlags, CPoint point) 
{
	ASSERT_VALID (this);
	CPoint ptMouse;
	GetCursorPos (&ptMouse);

	if ((GetDockMode () & BCGP_DT_IMMEDIATE) != 0)
	{
		if ((!m_bCaptured && GetCapture () == this ||
			m_bCaptured && GetCapture () != this ||
			(GetAsyncKeyState(::GetSystemMetrics(SM_SWAPBUTTON) ? VK_RBUTTON : VK_LBUTTON) & 0x8000) == 0)
			&& !m_bCaptionButtonsCaptured)
		{
			ReleaseCapture ();
			m_bCaptured = false;
			m_bPrepareToFloat = false;
			
			KillTimer(BCGP_SCROLL_TIMER_ID);
		}

		if (m_bPrepareToFloat)
		{
			CRect rectBar;
			GetWindowRect (rectBar);

			if (!m_bReadyToFloat)
			{
				m_bReadyToFloat = rectBar.PtInRect (ptMouse) == TRUE;
			}

			CRect rectLast = m_rectDragImmediate;

			CPoint ptOffset = ptMouse - m_dragFrameImpl.m_ptHot;
			m_dragFrameImpl.m_ptHot = ptMouse;

			CPoint ptClientHot = m_ptClientHotSpot;
			ClientToScreen (&ptClientHot);
			CPoint ptDragOffset = ptMouse - ptClientHot;

			UpdateVirtualRect (ptOffset);
		

			if ((abs (ptDragOffset.x) > m_sizeDragSencitivity.cx || 
				 abs (ptDragOffset.y) > m_sizeDragSencitivity.cy) && m_bReadyToFloat)
			{
				if (IsTabbed ())
				{
					CBCGPBaseTabWnd* pParentTab = 
						DYNAMIC_DOWNCAST (CBCGPBaseTabWnd, GetParent ());
					if (pParentTab != NULL)
					{
						pParentTab->DetachTab (BCGP_DM_MOUSE);
					}
				}
				else
				{
					FloatControlBar (m_recentDockInfo.m_rectRecentFloatingRect, BCGP_DM_MOUSE);			
					CBCGPDockManager* pDockManager = globalUtils.GetDockManager (GetDockSite ());
					globalUtils.ForceAdjustLayout (pDockManager);
				}
				m_bPrepareToFloat = false;
				m_bReadyToFloat = false;
			}
			return;
		}
	}
	else if ((GetDockMode () & BCGP_DT_STANDARD) != 0 && m_bPrepareToFloat)
	{
		CBCGPControlBar::OnMouseMove(nFlags, point);
		return;
	}

	CPoint ptScreen = point;
	ClientToScreen (&ptScreen);

	OnTrackCaptionButtons (ptScreen);
}
//***********************************************************************
void CBCGPDockingControlBar::OnLButtonUp(UINT nFlags, CPoint point) 
{
	ASSERT_VALID (this);

	HWND hwndThis = GetSafeHwnd();

	if (m_bPrepareToFloat)
	{
		m_bPrepareToFloat = false;
	}

	if (m_nHit != HTNOWHERE)
	{
		CBCGPDockManager* pDockManager = globalUtils.GetDockManager (GetDockSite ());
		ASSERT (pDockManager != NULL || globalUtils.m_bDialogApp);		

		UINT nHot = m_nHot;
		UINT nHit = m_nHit;

		StopCaptionButtonsTracking ();

		CBCGPSlider* pDefaultSlider = GetDefaultSlider ();

		if (nHot == nHit)
		{
			switch (nHit)
			{
			case HTCLOSE_BCG:
				{
					BOOL bCanClose = TRUE;
					CFrameWnd* pWndMain = BCGCBProGetTopLevelFrame (this);
					if (pWndMain != NULL)
					{
						CBCGPMDIFrameWnd* pMainFrame = DYNAMIC_DOWNCAST (CBCGPMDIFrameWnd, pWndMain);
						if (pMainFrame != NULL)
						{
							bCanClose = pMainFrame->OnCloseDockingBar (this);
						}
						else	// Maybe, SDI frame...
						{
							CBCGPFrameWnd* pFrame = DYNAMIC_DOWNCAST (CBCGPFrameWnd, pWndMain);
							if (pFrame != NULL)
							{
								bCanClose = pFrame->OnCloseDockingBar (this);
							}
							else	// Maybe, OLE frame...
							{
								CBCGPOleIPFrameWnd* pOleFrame = 
									DYNAMIC_DOWNCAST (CBCGPOleIPFrameWnd, pWndMain);
								if (pOleFrame != NULL)
								{
									bCanClose = pOleFrame->OnCloseDockingBar (this);
								}
								else
								{
									CBCGPOleDocIPFrameWnd* pOleDocFrame = 
										DYNAMIC_DOWNCAST (CBCGPOleDocIPFrameWnd, pWndMain);
									if (pOleDocFrame != NULL)
									{
										bCanClose = pOleDocFrame->OnCloseDockingBar (this);
									}
								}
							}
						}
					}

					if (bCanClose)
					{
						OnPressCloseButton ();
					}
					break;
				}

			case HTMAXBUTTON:
				if (GetAsyncKeyState (VK_CONTROL) && IsAutohideAllEnabled ())
				{
					m_pDockSite->SetRedraw (FALSE);
					if (!m_bPinState)
					{
						CObList lstBars;
						pDefaultSlider->GetControlBars (lstBars);
						
						for (POSITION pos = lstBars.GetHeadPosition (); pos != NULL;)
						{
							CBCGPDockingControlBar* pBar = 
								DYNAMIC_DOWNCAST (CBCGPDockingControlBar, lstBars.GetNext (pos));
							if (pBar->IsAutohideAllEnabled ())
							{
								pBar->SetAutoHideMode (TRUE, 
									pDefaultSlider->GetCurrentAlignment (), NULL, FALSE);	
							}
						}
					}
					else
					{
						CBCGPAutoHideDockBar* pParentDockBar = 
							DYNAMIC_DOWNCAST (CBCGPAutoHideDockBar, m_pAutoHideBar->GetParentDockBar ());

						if (pParentDockBar != NULL)
						{
							pParentDockBar->UnSetAutoHideMode (NULL);
						}
					}

					m_pDockSite->SetRedraw (TRUE);

					CFrameWnd* pFrame = DYNAMIC_DOWNCAST (CFrameWnd, m_pDockSite);
					if (pFrame != NULL)
					{
						pFrame->RecalcLayout ();
					}

					m_pDockSite->RedrawWindow (NULL, NULL, 
							RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
				}
				else
				{
					if (pDockManager != NULL && pDefaultSlider != NULL && 
						(pDefaultSlider->GetCurrentAlignment () & 
						pDockManager->GetEnabledAutoHideAlignment ()))
					{
						SetAutoHideMode (!m_bPinState, 
							pDefaultSlider->GetCurrentAlignment ());
					}
				}
				return;

			case HTMINBUTTON:
				if (CBCGPDockManager::IsDockBarMenu ())
				{
					CBCGPCaptionButton* pMenuButton = FindButtonByHit (HTMINBUTTON);
					if (pMenuButton == NULL)
					{
						ASSERT (FALSE);
						return;
					}

					CRect rectButton = pMenuButton->GetRect ();
					
					CRect rcBar;
					GetWindowRect(rcBar);
					ScreenToClient (rcBar);

					rectButton.OffsetRect (rcBar.TopLeft());

					ClientToScreen (&rectButton);

					pMenuButton->m_bDroppedDown = TRUE;
					RedrawButton (pMenuButton);

					CPoint ptMenu (rectButton.left, rectButton.bottom + 1);

					if (GetExStyle () & WS_EX_LAYOUTRTL)
					{
						ptMenu.x += rectButton.Width ();
					}

					HWND hwndThis = GetSafeHwnd ();

					OnShowControlBarMenu (ptMenu);

					if (::IsWindow (hwndThis))
					{
						pMenuButton->m_bDroppedDown = FALSE;
						RedrawButton (pMenuButton);
					}
				}
				return;

			default:
				OnPressButtons (nHit);
			}
		}

		if (::IsWindow(hwndThis))
		{
			CWnd::OnLButtonUp(nFlags, point);
		}
		return;
	}

	if (::IsWindow(hwndThis))
	{
		CBCGPControlBar::OnLButtonUp(nFlags, point);
	}
}
//***********************************************************************
void CBCGPDockingControlBar::OnPressCloseButton ()
{
	CFrameWnd* pParentFrame = DYNAMIC_DOWNCAST (CFrameWnd, BCGPGetParentFrame (this));
	ASSERT_VALID (pParentFrame);

	if (pParentFrame != NULL)
	{
		if (pParentFrame->SendMessage (BCGM_ON_PRESS_CLOSE_BUTTON, NULL, (LPARAM) (LPVOID) this))
		{
			return;
		}
	}

	if (IsAutoHideMode ())
	{
		SetAutoHideMode (FALSE, GetCurrentAlignment ());
	}
	ShowControlBar (FALSE, FALSE, FALSE);
	AdjustDockingLayout ();
}
//***********************************************************************
void CBCGPDockingControlBar::EnterDragMode (BOOL bChangeHotPoint)
{
	m_bPrepareToFloat = true;
	CBCGPControlBar::EnterDragMode (bChangeHotPoint);
}
//***********************************************************************
CBCGPAutoHideToolBar* CBCGPDockingControlBar::SetAutoHideMode (BOOL bMode, DWORD dwAlignment, 
																CBCGPAutoHideToolBar* pCurrAutoHideBar, 
																BOOL bUseTimer)
{
	ASSERT_VALID (this);
	ASSERT (dwAlignment & CBRS_ALIGN_ANY);

	if (bMode == IsAutoHideMode ())
	{
		return pCurrAutoHideBar;
	}

	CBCGPDockManager* pDockManager = globalUtils.GetDockManager (GetDockSite ());
	ASSERT_VALID (pDockManager);

	if (bMode)
	{
		m_bPinState = TRUE;

		CRect rectBeforeUndock;
		GetWindowRect (rectBeforeUndock);
		GetDockSite ()->ScreenToClient (rectBeforeUndock);

		StoreRecentDockInfo ();

		// set autohide mode
		UnDockControlBar (FALSE);

		CBCGPSlider* pDefaultSlider = GetDefaultSlider ();
		ASSERT (pDefaultSlider == NULL);
		pDefaultSlider = CreateDefaultSlider (dwAlignment, GetDockSite ());

		if (pDefaultSlider == NULL)
		{
			TRACE0 ("Failed to create default slider\n");
			DockControlBar (this, NULL, BCGP_DM_DBL_CLICK);
			return NULL;
		}

		m_hDefaultSlider = pDefaultSlider->m_hWnd;

		pDefaultSlider->SetAutoHideMode (TRUE);
		pDefaultSlider->AddControlBar (this);

		SetBarAlignment (dwAlignment);
		pDefaultSlider->SetBarAlignment (dwAlignment);
		
		pCurrAutoHideBar = pDockManager->AutoHideBar (this, pCurrAutoHideBar);

		if (IsBarVisible ())
		{
			pDefaultSlider->RedrawWindow (NULL, NULL,
					RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ALLCHILDREN);
			RedrawWindow (NULL, NULL,
					RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ALLCHILDREN);

			GetDockSite ()->RedrawWindow (rectBeforeUndock,  NULL,
							RDW_INVALIDATE | RDW_UPDATENOW |  RDW_ALLCHILDREN);
		}
		else
		{
			ShowWindow (SW_SHOW);
		}

		if (bUseTimer)
		{
			m_nAutoHideConditionTimerID = SetTimer (ID_CHECK_AUTO_HIDE_CONDITION, 
													m_nTimeOutBeforeAutoHide, NULL);	
			Slide (FALSE, TRUE);
			GetDockSite ()->SetFocus ();
		}
		else
		{
			Slide (FALSE, FALSE);
		}

		SetWindowPos (NULL, -1, -1, -1, -1, 
			SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_NOREDRAW | SWP_FRAMECHANGED);
	}
	else if (m_pAutoHideBar != NULL)
	{
		CBCGPAutoHideDockBar* pParentDockBar = 
			DYNAMIC_DOWNCAST (CBCGPAutoHideDockBar, m_pAutoHideBar->GetParentDockBar ());

		if (pParentDockBar != NULL)
		{
			pParentDockBar->UnSetAutoHideMode (m_pAutoHideBar);
		}
	}

	return pCurrAutoHideBar;
}
//***********************************************************************
void CBCGPDockingControlBar::UnSetAutoHideMode (CBCGPDockingControlBar* pFirstBarInGroup)
{
	m_bPinState = FALSE;

	if (m_nAutoHideConditionTimerID != 0)
	{
		KillTimer (m_nAutoHideConditionTimerID);
	}

	if (m_nSlideTimer != 0)
	{
		KillTimer (m_nSlideTimer);
	}

	BOOL bWasActive = m_pAutoHideBar->m_bActiveInGroup;

	m_pAutoHideBar->RemoveAutoHideWindow (this);

	RemoveFromDefaultSlider ();		
	// unset autohide mode - make it docked back
	if (pFirstBarInGroup == NULL)
	{
		if (!DockControlBar (this, NULL, BCGP_DM_DBL_CLICK))
		{
			return;
		}
	}
	else
	{
		AttachToTabWnd (pFirstBarInGroup, BCGP_DM_SHOW, bWasActive);
	}
	ShowControlBar (TRUE, FALSE, bWasActive);
	AdjustDockingLayout ();
}
//***********************************************************************
void CBCGPDockingControlBar::OnTimer(UINT_PTR nIDEvent) 
{
	BOOL bSlideDirection = FALSE;

	switch (nIDEvent)
	{
	case ID_CHECK_AUTO_HIDE_CONDITION:
		if (CheckAutoHideCondition ())
		{
			KillTimer (m_nAutoHideConditionTimerID);
			m_nAutoHideConditionTimerID = 0;
		}
		return;

	case BCGP_AUTO_HIDE_SLIDE_OUT_EVENT:
		bSlideDirection = TRUE;
		m_bIsHiding = FALSE;
		break;

	case BCGP_AUTO_HIDE_SLIDE_IN_EVENT:
		bSlideDirection = FALSE;
		m_bIsHiding = TRUE;
		break;

	case BCGP_SCROLL_TIMER_ID:
		if (m_nHit == m_nHot)
		{
			CBCGPCaptionButton* pButton = FindButtonByHit (m_nHit);

			if (pButton != NULL && !pButton->m_bHidden)
			{
				switch (m_nHit)
				{
				case HTSCROLLUPBUTTON_BCG:
				case HTSCROLLDOWNBUTTON_BCG:
					{
						BOOL bUp   = ScrollVertAvailable(TRUE);
						BOOL bDown = ScrollVertAvailable(FALSE);

						OnScrollClient(MAKEWORD(-1, m_nHit == HTSCROLLUPBUTTON_BCG ? SB_LINEUP : SB_LINEDOWN));

						if (bUp != ScrollVertAvailable(TRUE) || bDown != ScrollVertAvailable(FALSE))
						{
							SetWindowPos (NULL, 0, 0, 0, 0, 
									SWP_NOZORDER | SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE | SWP_FRAMECHANGED);
							pButton->m_bFocused = pButton->m_bPushed = TRUE;
							RedrawButton(pButton);
						}
					}
					return;

				case HTSCROLLLEFTBUTTON_BCG:
				case HTSCROLLRIGHTBUTTON_BCG:
					{
						BOOL bLeft = ScrollHorzAvailable(TRUE);
						BOOL bRight= ScrollHorzAvailable(FALSE);

						OnScrollClient(MAKEWORD(m_nHit == HTSCROLLLEFTBUTTON_BCG ? SB_LINELEFT : SB_LINERIGHT, -1));

						if (bLeft != ScrollHorzAvailable(TRUE) || bRight != ScrollHorzAvailable(FALSE))
						{
							SetWindowPos (NULL, 0, 0, 0, 0, 
									SWP_NOZORDER | SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE | SWP_FRAMECHANGED);
							pButton->m_bFocused = pButton->m_bPushed = TRUE;
							RedrawButton(pButton);
						}
					}
					return;
				}
			}
		}
		return;

	default:
		CBCGPControlBar::OnTimer(nIDEvent);
		return;
	}

	OnSlide (bSlideDirection);

	if (CheckStopSlideCondition (bSlideDirection))
	{
		KillTimer (m_nSlideTimer);

		m_bIsSliding = FALSE;
		m_nSlideTimer = 0;
		m_nSlideStep = 0;

		if (bSlideDirection) // slide out - show
		{
		
			RedrawWindow (NULL, NULL,
				RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | 
				RDW_ALLCHILDREN);

			::RedrawWindow (m_hDefaultSlider, NULL, NULL, RDW_INVALIDATE);
			// one second time out to give the user ability to move the mouse over
			// miniframe
			if (m_nAutoHideConditionTimerID != 0)
			{
				KillTimer (m_nAutoHideConditionTimerID);
			}

			m_nAutoHideConditionTimerID = SetTimer (ID_CHECK_AUTO_HIDE_CONDITION, 
													m_nTimeOutBeforeAutoHide, NULL);
		}
		else
		{
			ShowWindow (SW_HIDE);

			CBCGPSlider* pDefaultSlider = GetDefaultSlider ();
			if (pDefaultSlider != NULL)
			{
				ASSERT_VALID (pDefaultSlider);
				pDefaultSlider->ShowWindow (SW_HIDE);
			}
		}
	}

	CBCGPControlBar::OnTimer(nIDEvent);
}
//***********************************************************************
// Returns TRUE when the dock bar should be hidden (strats slide in)
//***********************************************************************
BOOL CBCGPDockingControlBar::CheckAutoHideCondition ()
{
	if (m_bActive || m_bIsResizing || !IsAutoHideMode () || 
		CBCGPPopupMenu::GetActiveMenu () != NULL)
	{
		return FALSE;
	}

	if (m_pToolTip->GetSafeHwnd () != NULL &&
		m_pToolTip->IsWindowVisible())
	{
		return FALSE;
	}

	ASSERT_VALID (m_pAutoHideButton);
	ASSERT_VALID (m_pAutoHideBar);

	CRect rectAutoHideBtn = m_pAutoHideButton->GetRect ();
	m_pAutoHideBar->ClientToScreen (&rectAutoHideBtn);

	CPoint ptCursor;
	GetCursorPos (&ptCursor);

	CWnd* pWndFromPoint = WindowFromPoint (ptCursor);
	BOOL bCursorOverThisWindow = FALSE; // and this is topmost window
	while (pWndFromPoint != NULL)
	{
		if (pWndFromPoint == this ||
			pWndFromPoint->m_hWnd == m_hDefaultSlider ||
			pWndFromPoint->IsKindOf (RUNTIME_CLASS (CBCGPInplaceToolTipCtrl)))
		{
			bCursorOverThisWindow = TRUE;
			break;
		}
		pWndFromPoint = pWndFromPoint->GetParent ();
	}

	CRect rectWnd;
	GetWindowRect (rectWnd);
	CRect rectSlider;
	::GetWindowRect (m_hDefaultSlider, &rectSlider);

	rectWnd.UnionRect (rectWnd, rectSlider);

	if (rectWnd.PtInRect (ptCursor) ||  bCursorOverThisWindow ||
		rectAutoHideBtn.PtInRect (ptCursor))
	{
		return FALSE;
	}

	Slide (FALSE);

	return TRUE;
}
//***********************************************************************
BOOL CBCGPDockingControlBar::CheckStopSlideCondition (BOOL bDirection)
{
	if (!IsAutoHideMode ())
	{
		return TRUE;
	}

	CRect rectWnd;
	GetWindowRect (rectWnd);

	GetDockSite ()->ScreenToClient (rectWnd);
	BOOL bIsRTL = GetDockSite ()->GetExStyle () & WS_EX_LAYOUTRTL;
 
	CRect rectAutoHideDockBar;
	m_pAutoHideBar->GetParentDockBar ()->GetWindowRect (rectAutoHideDockBar);
	GetDockSite ()->ScreenToClient (rectAutoHideDockBar);

	BOOL bStop = FALSE;
	switch (GetCurrentAlignment ())
	{

	case CBRS_ALIGN_RIGHT:
		if (m_ahSlideMode == BCGP_AHSM_MOVE)
		{
			if (bIsRTL)
			{
				bStop = bDirection ? rectWnd.left >= rectAutoHideDockBar.right :
					 rectWnd.right <= rectAutoHideDockBar.right;

			}
			else
			{
				bStop = bDirection ? rectWnd.right <= rectAutoHideDockBar.left : 
									 rectWnd.left >= rectAutoHideDockBar.left;
			}
		}
		else
		{
			bStop = bDirection ? rectWnd.Width () >= m_rectRestored.Width () : 
							 rectWnd.Width () <= 0;
		}
		break;
	case CBRS_ALIGN_LEFT:
		if (m_ahSlideMode == BCGP_AHSM_MOVE)
		{
			if (bIsRTL)
			{
				bStop = bDirection ? rectWnd.right <= rectAutoHideDockBar.left : 
					 rectWnd.left >= rectAutoHideDockBar.left;

			}
			else
			{
				bStop = bDirection ? rectWnd.left >= rectAutoHideDockBar.right :
									 rectWnd.right <= rectAutoHideDockBar.right;
			}
		}
		else
		{
			bStop = bDirection ? rectWnd.Width () >= m_rectRestored.Width () : 
							 rectWnd.Width () <= 0;
		}
		break;
	case CBRS_ALIGN_TOP:	
		if (m_ahSlideMode == BCGP_AHSM_MOVE)
		{
			bStop = bDirection ? rectWnd.top >= rectAutoHideDockBar.bottom :
								 rectWnd.bottom <= rectAutoHideDockBar.bottom;
		}
		else
		{

		}
		break;
	case CBRS_ALIGN_BOTTOM:	
		if (m_ahSlideMode == BCGP_AHSM_MOVE)
		{
			bStop = bDirection ? rectWnd.bottom <= rectAutoHideDockBar.top :
								 rectWnd.top >= rectAutoHideDockBar.top;
		}
		else
		{
			bStop = bDirection ? rectWnd.Height () >= m_rectRestored.Height () : 
							 rectWnd.Height () <= 0;
		}
		break;
	}

	return bStop;
}
//***********************************************************************
void CBCGPDockingControlBar::OnSlide (BOOL bSlideDirection)
{
	if (!IsAutoHideMode () && !IsWindow (m_hDefaultSlider))
	{
		return;
	}

	BOOL bIsRTL = GetDockSite ()->GetExStyle () & WS_EX_LAYOUTRTL;

	m_nSlideStep++;

	CRect rect;
	GetWindowRect (&rect);
	GetDockSite ()->ScreenToClient (&rect); 

	CRect rectSlider;
	::GetWindowRect (m_hDefaultSlider, &rectSlider);
	GetDockSite ()->ScreenToClient (&rectSlider); 

	if (m_ahSlideMode == BCGP_AHSM_MOVE)
	{
		OffsetRectForSliding (rect, bSlideDirection, bIsRTL);
		OffsetRectForSliding (rectSlider, bSlideDirection, bIsRTL);
		if (bSlideDirection)
		{
			CPoint pt = CalcCorrectOffset (rect, bIsRTL);
			rect.OffsetRect (pt);
			rectSlider.OffsetRect (pt);
		}
	}
	else
	{
		CalcRectForSliding (rect, rectSlider, bSlideDirection);
	}


	SetWindowPos (NULL, rect.left, rect.top,
					rect.Width (), rect.Height (),
					SWP_NOZORDER | SWP_NOACTIVATE);

	::SetWindowPos (m_hDefaultSlider, NULL, rectSlider.left, rectSlider.top,
					rectSlider.Width (), rectSlider.Height (), SWP_NOZORDER | SWP_NOACTIVATE);

}
//***********************************************************************
void CBCGPDockingControlBar::OffsetRectForSliding (CRect& rect, BOOL bSlideDirection, BOOL bIsRTL)
{
	if (!IsAutoHideMode ())
	{
		return;
	}

	switch (GetCurrentAlignment ())
	{
	case CBRS_ALIGN_LEFT:
		if (bIsRTL)
		{
			bSlideDirection ? rect.OffsetRect (-m_nSlideDelta, 0) : 
							  rect.OffsetRect (m_nSlideDelta, 0);	
		}
		else
		{
			bSlideDirection ? rect.OffsetRect (m_nSlideDelta, 0) : 
							  rect.OffsetRect (-m_nSlideDelta, 0);	
		}
		break;

	case CBRS_ALIGN_RIGHT:
		if (bIsRTL)
		{
			bSlideDirection  ? rect.OffsetRect (m_nSlideDelta, 0) : 
							   rect.OffsetRect (-m_nSlideDelta, 0);
		}
		else
		{
			bSlideDirection  ? rect.OffsetRect (-m_nSlideDelta, 0) : 
							   rect.OffsetRect (m_nSlideDelta, 0);
		}
		break;

	case CBRS_ALIGN_TOP:
		bSlideDirection ? rect.OffsetRect (0, m_nSlideDelta) : 
						  rect.OffsetRect (0, -m_nSlideDelta);
		break;

	case CBRS_ALIGN_BOTTOM:
		bSlideDirection ? rect.OffsetRect (0, -m_nSlideDelta) : 
						  rect.OffsetRect (0, m_nSlideDelta);
		break;

	}
}
//***********************************************************************
void CBCGPDockingControlBar::CalcRectForSliding (CRect& rect, CRect& rectSlider, BOOL bSlideDirection)
{
	if (!IsAutoHideMode ())
	{
		return;
	}
	
	switch (GetCurrentAlignment ())
	{
	case CBRS_ALIGN_LEFT:
		if (bSlideDirection)   
		{
			rect.right += m_nSlideDelta;
			if (rect.Width () > m_rectRestored.Width ())
			{
				rect.right = rect.left + m_rectRestored.Width ();
			}
		}
		else
		{
			rect.right -= m_nSlideDelta;	
			if (rect.right < rect.left)
			{
				rect.right = rect.left;
			}
		}
		{
			int nSliderWidth = rectSlider.Width ();
			rectSlider.left = rect.right;
			rectSlider.right = rectSlider.left + nSliderWidth;
		}
		break;

	case CBRS_ALIGN_RIGHT:
		if (bSlideDirection)   
		{
			rect.left -= m_nSlideDelta;
			if (rect.Width () > m_rectRestored.Width ())
			{
				rect.left = rect.right - m_rectRestored.Width ();
			}
		}
		else
		{
			rect.left += m_nSlideDelta;	
			if (rect.left > rect.right)
			{
				rect.left = rect.right;
			}
		}
		{
			int nSliderWidth = rectSlider.Width ();
			rectSlider.right = rect.left;
			rectSlider.left = rectSlider.right - nSliderWidth;
		}
		break;

	case CBRS_ALIGN_TOP:
		if (bSlideDirection)   
		{
			rect.bottom += m_nSlideDelta;
			if (rect.Height () > m_rectRestored.Height ())
			{
				rect.bottom = rect.top + m_rectRestored.Height ();
			}
		}
		else
		{
			rect.bottom -= m_nSlideDelta;	
			if (rect.bottom < rect.top)
			{
				rect.bottom = rect.top;
			}
		}
		{
			int nSliderHeight = rectSlider.Height ();
			rectSlider.top = rect.bottom;
			rectSlider.bottom = rectSlider.top + nSliderHeight;
		}
		break;

	case CBRS_ALIGN_BOTTOM:
		if (bSlideDirection)   
		{
			rect.top -= m_nSlideDelta;
			if (rect.Height () > m_rectRestored.Height ())
			{
				rect.top = rect.bottom - m_rectRestored.Height ();
			}
		}
		else
		{
			rect.top += m_nSlideDelta;	
			if (rect.top > rect.bottom)
			{
				rect.top = rect.bottom;
			}
		}
		{
			int nSliderHeight = rectSlider.Height ();
			rectSlider.bottom = rect.top;
			rectSlider.top = rectSlider.bottom - nSliderHeight;
		}
		break;

	}
}
//***********************************************************************
CPoint CBCGPDockingControlBar::CalcCorrectOffset (CRect rect, BOOL bIsRTL)
{
	CRect rectAutoHideDockBar;
	m_pAutoHideBar->GetParentDockBar ()->GetWindowRect (rectAutoHideDockBar);
	GetDockSite ()->ScreenToClient (rectAutoHideDockBar);
	
	switch (GetCurrentAlignment ())
	{
	case CBRS_ALIGN_LEFT:
		if (bIsRTL)
		{
			if (rect.right < rectAutoHideDockBar.left)
			{
				return CPoint (rectAutoHideDockBar.left - rect.right, 0);
			}
		}
		else
		{
			if (rect.left > rectAutoHideDockBar.right)
			{
				return CPoint (rectAutoHideDockBar.right - rect.left, 0);
			}
		}
		break;
	case CBRS_ALIGN_RIGHT:
		if (bIsRTL)
		{
			if (rect.left > rectAutoHideDockBar.right)
			{
				return CPoint (rectAutoHideDockBar.right - rect.left, 0);
			}
		}
		else
		{
			if (rect.right < rectAutoHideDockBar.left)
			{
				return CPoint (rectAutoHideDockBar.left - rect.right, 0);
			}
		}
		break;
	case CBRS_ALIGN_TOP:
		if (rect.top > rectAutoHideDockBar.bottom)
		{
			return CPoint (0, rectAutoHideDockBar.bottom - rect.top);
		}
		break;
	case CBRS_ALIGN_BOTTOM:
		if (rect.bottom < rectAutoHideDockBar.top)
		{
			return CPoint (0, rectAutoHideDockBar.top - rect.bottom);
		}
		break;
	}
	return CPoint (0, 0);
}
//***********************************************************************
void CBCGPDockingControlBar::Slide (BOOL bSlideOut, BOOL bUseTimer)
{
	ASSERT_VALID (this);

	if (!IsAutoHideMode ())
	{
		return;
	}

	if (m_nSlideTimer != 0)
	{
		KillTimer (m_nSlideTimer);
	}

	if (m_nAutoHideConditionTimerID != 0)
	{
		KillTimer (m_nAutoHideConditionTimerID);
		m_nAutoHideConditionTimerID = 0;
	}

	CRect rectWnd;
	GetWindowRect (rectWnd);

	if (!bUseTimer || m_bDisableAnimation || globalData.bIsRemoteSession)
	{
		m_nSlideDelta = IsHorizontal () ? rectWnd.Height () : rectWnd.Width ();
	}

	if (!bUseTimer)
	{
		m_rectRestored = rectWnd;
		// just move out from the screen
		
		OnSlide (FALSE);
		ShowWindow (SW_HIDE);
		::ShowWindow (m_hDefaultSlider, SW_HIDE);
		return;
	}

	CBCGPDockManager* pDockManager = globalUtils.GetDockManager (GetDockSite ());
	ASSERT_VALID (pDockManager);

	if (bSlideOut)		
	{
		pDockManager->HideAutoHideBars (this);
		pDockManager->AlignAutoHideBar (GetDefaultSlider (), FALSE);
		ShowWindow (SW_SHOW);
		::ShowWindow (m_hDefaultSlider, SW_SHOW);
	}

	
	BringWindowToTop ();
	::BringWindowToTop (m_hDefaultSlider);

	if (m_ahSlideMode == BCGP_AHSM_MOVE)
	{
		pDockManager->BringBarsToTop ();
	}

	m_nSlideTimer = SetTimer (bSlideOut ? BCGP_AUTO_HIDE_SLIDE_OUT_EVENT : 
										  BCGP_AUTO_HIDE_SLIDE_IN_EVENT, 
										  m_nSlideDefaultTimeOut, NULL);

	
	if (!m_bDisableAnimation && !globalData.bIsRemoteSession)
	{
		if (m_ahSlideMode == BCGP_AHSM_MOVE)
		{
			GetDockSite ()->ScreenToClient (rectWnd);
			m_nSlideDelta = max (1, ((GetCurrentAlignment () & CBRS_ORIENT_HORZ) ?
					rectWnd.Height () : rectWnd.Width ()) / m_nSlideSteps);

		}
		else if (m_ahSlideMode == BCGP_AHSM_STRETCH)
		{
			if (!bSlideOut && !m_bIsSliding)
			{
				m_rectRestored = rectWnd;
				GetDockSite ()->ScreenToClient (m_rectRestored);
			}
			m_nSlideDelta = max (1, ((GetCurrentAlignment () & CBRS_ORIENT_HORZ) ?
					m_rectRestored.Height () : m_rectRestored.Width ()) / m_nSlideSteps);
		}	
	}

	m_nSlideStep = 0;
	m_bIsSliding = TRUE;
}
//***********************************************************************
void CBCGPDockingControlBar::SetAutoHideParents (CBCGPAutoHideToolBar* pToolBar, 
												 CBCGPAutoHideButton* pBtn)
{
	ASSERT_VALID (pToolBar);
	ASSERT_VALID (pBtn);

	m_pAutoHideBar		= pToolBar;
	m_pAutoHideButton	= pBtn;
}
//***********************************************************************
void CBCGPDockingControlBar::SetResizeMode (BOOL bResize) 
{ 
	m_bIsResizing = bResize;
}
//***********************************************************************
CBCGPSlider* CBCGPDockingControlBar::CreateDefaultSlider (DWORD dwAlignment, CWnd* pParent, 
															CRuntimeClass* pSliderRTC)
{
	CRect rectSlider (0, 0, CBCGPSlider::GetDefaultWidth (), CBCGPSlider::GetDefaultWidth ());
	WORD dwSliderStyle = CBCGPSlider::SS_HORZ;

	if (dwAlignment & CBRS_ALIGN_LEFT || dwAlignment & CBRS_ALIGN_RIGHT)
	{
		dwSliderStyle = CBCGPSlider::SS_VERT;
	}

	// create a slider with a control bar container
	CBCGPSlider* pSlider = NULL; 
	if (pSliderRTC != NULL)
	{
		pSlider = DYNAMIC_DOWNCAST (CBCGPSlider, pSliderRTC->CreateObject ());
		ASSERT_VALID (pSlider);
		
		pSlider->SetDefaultMode (TRUE);
	}
	else
	{
		pSlider = DYNAMIC_DOWNCAST (CBCGPSlider, CBCGPSlider::m_pSliderRTC->CreateObject ());
		ASSERT_VALID (pSlider);

		pSlider->Init (TRUE);
	}

	

	if (!pSlider->CreateEx (0, dwSliderStyle | WS_VISIBLE, 
							rectSlider, pParent, (UINT) -1, NULL))
	{
		TRACE0 ("Can't create default slider while docking\n");
		delete pSlider;
		return NULL;
	}

	pSlider->SetBarAlignment (dwAlignment);

	return pSlider;
}
//***********************************************************************
void CBCGPDockingControlBar::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	CPoint ptScreen = point;
	ClientToScreen (&ptScreen);

	CBCGPCaptionButton* pBtn = FindButton (ptScreen);
	if (pBtn != NULL)
	{
		if (!pBtn->m_bHidden)
		{
			switch (pBtn->GetHit())
			{
			case HTSCROLLUPBUTTON_BCG:
			case HTSCROLLDOWNBUTTON_BCG:
				OnScrollClient(MAKEWORD(-1, pBtn->GetHit() == HTSCROLLUPBUTTON_BCG ? SB_TOP : SB_BOTTOM));
				SetWindowPos (NULL, 0, 0, 0, 0, 
						SWP_NOZORDER | SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE | SWP_FRAMECHANGED);
				return;

			case HTSCROLLLEFTBUTTON_BCG:
			case HTSCROLLRIGHTBUTTON_BCG:
				OnScrollClient(MAKEWORD(pBtn->GetHit() == HTSCROLLLEFTBUTTON_BCG ? SB_LEFT : SB_RIGHT, -1));
				SetWindowPos (NULL, 0, 0, 0, 0, 
						SWP_NOZORDER | SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE | SWP_FRAMECHANGED);
				return;
			}
		}

		CWnd::OnLButtonDblClk(nFlags, point);
		return;
	}

	if (!IsAutoHideMode ())
	{
		CBCGPDockingControlBar* pBarToDock = this;
		if (IsTabbed ())
		{
			CBCGPBaseTabWnd* pTabWnd = 
				DYNAMIC_DOWNCAST (CBCGPBaseTabWnd, GetParent ());
			if (pTabWnd != NULL)
			{
				pBarToDock = 
					DYNAMIC_DOWNCAST (CBCGPDockingControlBar, pTabWnd->GetParent ());	
			}
		}

		CBCGPMultiMiniFrameWnd* pParentMiniFrame = 
				DYNAMIC_DOWNCAST (CBCGPMultiMiniFrameWnd, GetParentMiniFrame ());

		if (pParentMiniFrame != NULL)
		{
			OnProcessDblClk ();
			pParentMiniFrame->DockRecentControlBarToMainFrame (pBarToDock);
		}
		else if (IsWindow (m_hDefaultSlider))
		{
			// currently docked at main frame
			CBCGPMultiMiniFrameWnd* pRecentMiniFrame = 
				DYNAMIC_DOWNCAST (CBCGPMultiMiniFrameWnd,
					CWnd::FromHandlePermanent (m_recentDockInfo.m_hRecentMiniFrame));
			
			if (pRecentMiniFrame != NULL && 
				(m_recentDockInfo.GetRecentContainer (FALSE) != NULL ||
				 m_recentDockInfo.GetRecentTabContainer (FALSE) != NULL))
			{
				OnBeforeFloat (m_recentDockInfo.m_rectRecentFloatingRect, BCGP_DM_DBL_CLICK);

				OnProcessDblClk ();
				UnDockControlBar ();

				HWND hwndThis = GetSafeHwnd ();
				BOOL bCanFocus = CanFocus ();

				pRecentMiniFrame->AddRecentControlBar (pBarToDock);

				if (IsWindow (hwndThis))
				{
					OnAfterFloat ();	
				}
				
				if (bCanFocus)
				{
					pRecentMiniFrame->SetFocus ();
				}
			}
			else
			{
				CBCGPControlBar::OnLButtonDblClk(nFlags, point);
			}
		}
		else
		{
			OnProcessDblClk ();
		}
	}
	else
	{
		CWnd::OnLButtonDblClk(nFlags, point);
	}
}
//***********************************************************************
BOOL CBCGPDockingControlBar::OnBeforeFloat (CRect& rectFloat, BCGP_DOCK_METHOD dockMethod)
{
	ASSERT_VALID (this);
	BOOL bResult = CBCGPControlBar::OnBeforeFloat (rectFloat, dockMethod);
	
	if (dockMethod == BCGP_DM_MOUSE)
	{
		// prevent drawing of the drag rectangle on mouse up
		m_bPrepareToFloat = false;
	}

	return bResult;
}
//***********************************************************************
void CBCGPDockingControlBar::OnNcLButtonDown(UINT nHitTest, CPoint point) 
{
	ASSERT_VALID (this);

	if (!IsDocked ())
	{
		CBCGPControlBar::OnNcLButtonDown(nHitTest, point);
	}
}
//***********************************************************************
void CBCGPDockingControlBar::OnClose() 
{
	ASSERT_VALID (this);
	DestroyWindow ();
}
//***********************************************************************
CBCGPDockingControlBar* CBCGPDockingControlBar::AttachToTabWnd (CBCGPDockingControlBar* pTabControlBarAttachTo, 
																BCGP_DOCK_METHOD dockMethod,
																BOOL bSetActive, 
																CBCGPDockingControlBar** ppTabbedControlBar)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pTabControlBarAttachTo);

	if (ppTabbedControlBar != NULL)
	{
		*ppTabbedControlBar = NULL;
	}

	if (pTabControlBarAttachTo == NULL)
	{
		return NULL;
	}

	if (!pTabControlBarAttachTo->CanBeAttached () || !CanBeAttached () || 
		!pTabControlBarAttachTo->CanAcceptBar(this))
	{
		return NULL; // invalid attempt to attach non-attachable control bar
	}

	// check whether pTabBar is derived from CBCGPTabbedControlBar. If so, we 
	// can attach this bar to it immediately. Otherwise, we need to create a 
	// new tabbed control bar and replace pTabControlBarAttachTo with it.
	CBCGPBaseTabbedBar* pTabbedBarAttachTo = 
		DYNAMIC_DOWNCAST (CBCGPBaseTabbedBar, pTabControlBarAttachTo);

	BOOL bBarAttachToIsFloating = (pTabControlBarAttachTo->GetParentMiniFrame () != NULL);

	CWnd* pOldParent = GetParent ();
	CRect rectWndTab; rectWndTab.SetRectEmpty ();
	if (pTabbedBarAttachTo == NULL)
	{
		CWnd* pTabParent = pTabControlBarAttachTo->GetParent ();
		if (DYNAMIC_DOWNCAST (CBCGPBaseTabWnd, pTabParent) != NULL)
		{
			pTabParent = pTabParent->GetParent ();
		}

		pTabbedBarAttachTo = DYNAMIC_DOWNCAST (CBCGPBaseTabbedBar, pTabParent);

		if (pTabbedBarAttachTo == NULL)
		{
			pTabControlBarAttachTo->StoreRecentDockInfo ();

			pTabControlBarAttachTo->GetWindowRect (rectWndTab);
			pTabControlBarAttachTo->GetParent ()->ScreenToClient (&rectWndTab);

			pTabbedBarAttachTo = pTabControlBarAttachTo->CreateTabbedControlBar (); 
			ASSERT_VALID (pTabbedBarAttachTo);

			pTabControlBarAttachTo->InsertControlBar (pTabbedBarAttachTo, pTabControlBarAttachTo);

			if (!pTabControlBarAttachTo->ReplaceControlBar (pTabbedBarAttachTo, dockMethod))
			{
				if (!bBarAttachToIsFloating)
				{
					RemoveControlBarFromDockManager (pTabbedBarAttachTo);
				}
				ASSERT (FALSE);
				TRACE0 ("Failed to replace resizable control bar by tabbed control bar. \n");
				delete pTabbedBarAttachTo;
				return NULL;
			}

			pTabbedBarAttachTo->
				EnableDocking (pTabControlBarAttachTo->GetEnabledAlignment ());
			pTabbedBarAttachTo->
				SetBarAlignment (pTabControlBarAttachTo->GetCurrentAlignment ());
			
			pTabControlBarAttachTo->UnDockControlBar (TRUE);
			pTabbedBarAttachTo->AddTab (pTabControlBarAttachTo, TRUE, bSetActive);
			pTabControlBarAttachTo->EnableGripper (FALSE);
		}
	}

	if (ppTabbedControlBar != NULL)
	{
		*ppTabbedControlBar = pTabbedBarAttachTo;
	}

	EnableGripper (FALSE);
	
	// send before dock notification without guarantee that the bar will 
	// be attached to another dock bar
	OnBeforeDock ((CBCGPBaseControlBar**)&pTabbedBarAttachTo, NULL, dockMethod);
	// reassign the parentship to the tab bar
	OnBeforeChangeParent (pTabbedBarAttachTo, TRUE);

	// remove from miniframe
	RemoveFromMiniframe (pTabbedBarAttachTo, dockMethod);

	// AddTab returns TRUE only if this pointer is not tabbed control bar
	// (tabbed control bar is destroyed by AddTab and its tab windows are copied
	// to pTabbedBarAttachTo tabbed window)
	BOOL bResult = pTabbedBarAttachTo->AddTab (this, TRUE, bSetActive);
	if (bResult)
	{
		OnAfterChangeParent (pOldParent);
		OnAfterDock (pTabbedBarAttachTo, NULL, dockMethod);
	}

	if (!rectWndTab.IsRectEmpty ())
	{
		pTabbedBarAttachTo->SetWindowPos (NULL, rectWndTab.left, rectWndTab.top, 
			rectWndTab.Width (), rectWndTab.Height (), 
			SWP_NOZORDER | SWP_NOACTIVATE);
		
		if (bResult)
		{
			AdjustDockingLayout ();
		}
	}

	pTabbedBarAttachTo->RecalcLayout ();

	return bResult ? this : pTabbedBarAttachTo;
}
//***********************************************************************
BOOL CBCGPDockingControlBar::ReplaceControlBar (CBCGPDockingControlBar* pBarToReplaceWith, 
												BCGP_DOCK_METHOD /*dockMethod*/,
												BOOL bRegisterWithFrame)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pBarToReplaceWith);
	
	CBCGPMiniFrameWnd* pParentMiniFrame = GetParentMiniFrame ();
	
	if (pParentMiniFrame != NULL)
	{
		// this is tabbed control bar that should be replaced by docking control bar
		// within miniframe
		
		ASSERT_VALID (pParentMiniFrame);
		pParentMiniFrame->ReplaceControlBar (this, pBarToReplaceWith);
		return TRUE;
	}
	else if (m_hDefaultSlider != NULL)
	{
		CBCGPSlider* pDefaultSlider = GetDefaultSlider ();

		if (pDefaultSlider != NULL && 
			pDefaultSlider->ReplaceControlBar (this, pBarToReplaceWith))
		{
			// unregister from parent frame/dock manager the bar that is being replaced (this)
			if (bRegisterWithFrame)
			{
				RemoveControlBarFromDockManager (this, FALSE, FALSE, FALSE, pBarToReplaceWith);
			}
			else
			{
				RemoveControlBarFromDockManager (this, FALSE);
			}
			
			return TRUE;
		}
	}
	return FALSE;
}
//***********************************************************************
CBCGPTabbedControlBar* CBCGPDockingControlBar::CreateTabbedControlBar ()
{
	ASSERT_VALID (this);
	CRect rectTabBar;
	GetWindowRect (&rectTabBar);
	ASSERT_VALID (GetParent ());
	GetParent ()->ScreenToClient (&rectTabBar);

	CBCGPTabbedControlBar* pTabbedBar = 
			(CBCGPTabbedControlBar*) m_pTabbedControlBarRTC->CreateObject ();
	ASSERT_VALID (pTabbedBar);

	pTabbedBar->SetAutoDestroy (TRUE);

	if (!pTabbedBar->Create (_T (""), 
							GetParent (), 
							rectTabBar, 
							TRUE, 
							(UINT) -1, 
							GetStyle () | CBRS_FLOAT_MULTI))
	{
		TRACE0 ("Failed to create tabbed control bar\n");
		return NULL;
	}

	// override recent floating/docking info

	pTabbedBar->m_recentDockInfo.m_recentMiniFrameInfo.m_rectDockedRect = 
		m_recentDockInfo.m_recentMiniFrameInfo.m_rectDockedRect;
	pTabbedBar->m_recentDockInfo.m_recentSliderInfo.m_rectDockedRect = 
		m_recentDockInfo.m_recentSliderInfo.m_rectDockedRect;
	pTabbedBar->m_recentDockInfo.m_rectRecentFloatingRect = 
		m_recentDockInfo.m_rectRecentFloatingRect;

	return pTabbedBar;
}
//***********************************************************************
BOOL CBCGPDockingControlBar::Dock (CBCGPBaseControlBar* pTargetBar, LPCRECT lpRect, 
								  BCGP_DOCK_METHOD dockMethod)
{
	if (pTargetBar != NULL && !pTargetBar->CanAcceptBar (this) && 
		pTargetBar != this)
	{
		return FALSE;
	}

	if (dockMethod == BCGP_DM_RECT && lpRect == NULL)
	{
		TRACE0 ("Docking control bar must be docked by rect or by mouse!");
		ASSERT (FALSE);
		return FALSE;
	}

	m_bPrepareToFloat = false;

	if (dockMethod == BCGP_DM_DBL_CLICK || dockMethod == BCGP_DM_SHOW)
	{
		CBCGPBarContainer* pRecentTabContainer = 
			m_recentDockInfo.GetRecentTabContainer (TRUE);

		ShowWindow (SW_HIDE);

		RemoveFromMiniframe (BCGPGetParentFrame (this), dockMethod);
		SetBarAlignment (m_recentDockInfo.m_dwRecentAlignmentToFrame);

		CBCGPSlider* pRecentDefaultSlider = m_recentDockInfo.GetRecentDefaultSlider ();
		if (pRecentDefaultSlider != NULL)
		{
			SetDefaultSlider (pRecentDefaultSlider->m_hWnd);
		}

		if (pRecentTabContainer != NULL)
		{
			BOOL bRecentLeftBar = m_recentDockInfo.IsRecentLeftBar (TRUE);
			CBCGPDockingControlBar* pTabbedBar = (CBCGPDockingControlBar*) (bRecentLeftBar ? 
				pRecentTabContainer->GetLeftBar () : pRecentTabContainer->GetRightBar ());
			if (pTabbedBar != NULL)
			{
				BOOL bResult = (AttachToTabWnd (pTabbedBar, BCGP_DM_DBL_CLICK) != NULL);
				if (bResult)
				{
					ShowControlBar (TRUE, FALSE, TRUE);
				}
				AdjustDockingLayout ();
				return bResult;
			}
		}

		if (pRecentDefaultSlider != NULL)
		{
			EnableGripper (TRUE);
			/*
			SetWindowPos (NULL, 
				m_recentDockInfo.m_recentSliderInfo.m_rectDockedRect.left, 
				m_recentDockInfo.m_recentSliderInfo.m_rectDockedRect.top, 
				m_recentDockInfo.m_recentSliderInfo.m_rectDockedRect.Width (),
				m_recentDockInfo.m_recentSliderInfo.m_rectDockedRect.Height (),
				SWP_NOZORDER | SWP_NOREDRAW);*/

			AdjustBarWindowToContainer (pRecentDefaultSlider);

			InsertControlBar (this, pRecentDefaultSlider, FALSE);

			ShowWindow (SW_SHOW);

			CBCGPDockingControlBar* pAddedControlBar = 
				pRecentDefaultSlider->AddRecentControlBar (this);
			if (pAddedControlBar == this)
			{
				AdjustDockingLayout ();
				return TRUE;
			}
			else if (pAddedControlBar != NULL)
			{
				pAddedControlBar->AdjustDockingLayout ();
				return FALSE;
			}
		}
		else
		{
			ShowWindow (SW_SHOW);
			return DockToFrameWindow (m_recentDockInfo.m_dwRecentAlignmentToFrame, 
				(lpRect == NULL) ? 
						&m_recentDockInfo.m_recentSliderInfo.m_rectDockedRect : lpRect);
		}
		return FALSE;
	}

	CPoint ptMouse (0, 0);
	if (dockMethod == BCGP_DM_MOUSE || dockMethod == BCGP_DM_STANDARD)
	{
		GetCursorPos (&ptMouse);
	}


	DWORD dwAlignment = 0;
	if (pTargetBar == NULL)
	{
		// insert the resizable bar as first resizable if it crosses the outer edge
		// IsPointNearDockBar will return this information
		BOOL bOuterEdge = FALSE;
	
		if (dockMethod == BCGP_DM_MOUSE  || dockMethod == BCGP_DM_STANDARD)
		{
			CPoint ptMouse;
			GetCursorPos (&ptMouse);
			if (!IsPointNearDockBar (ptMouse, dwAlignment, bOuterEdge))
			{
				return FALSE;
			}
			return DockToFrameWindow (dwAlignment, NULL, BCGP_DT_DOCK_LAST, 
										NULL, -1, bOuterEdge);
		}
		else if (lpRect != NULL)
		{

		}
		
	}
	else
	{
		ASSERT_VALID (pTargetBar);

		if (dockMethod == BCGP_DM_MOUSE || dockMethod == BCGP_DM_STANDARD)
		{
			if (!globalUtils.CheckAlignment (ptMouse, pTargetBar, 
											CBCGPDockManager::m_nDockSencitivity, NULL,
											FALSE, dwAlignment))
			{
				return FALSE;
			}

			return DockToWindow ((CBCGPDockingControlBar*) pTargetBar, dwAlignment, NULL);
		}
		else if (lpRect != NULL)
		{
			return DockToWindow ((CBCGPDockingControlBar*) pTargetBar, 0, lpRect);
		}
	}

	return FALSE;
}
//***********************************************************************
BOOL CBCGPDockingControlBar::DockToFrameWindow (DWORD dwAlignment, 
												LPCRECT lpRect, 
												DWORD /*dwDockFlags*/, 
												CBCGPBaseControlBar* /*pRelativeBar*/, 
												int /*nRelativeIndex*/, 
												BOOL bOuterEdge)
{
	ASSERT_VALID (this);
	ASSERT (dwAlignment & CBRS_ALIGN_ANY);

	BOOL	bLocked	= LockWindowUpdate ();

	RemoveFromMiniframe (BCGPGetParentFrame (this), BCGP_DM_UNKNOWN);
	
	if (m_hDefaultSlider != NULL && IsWindow (m_hDefaultSlider))
	{
		UnDockControlBar (FALSE);
	}

	CBCGPSlider* pDefaultSlider = NULL;
	// create a slider with a control bar container
	if ((pDefaultSlider = CreateDefaultSlider (dwAlignment, GetDockSite ())) == NULL)
	{
		TRACE0 ("Failde to create default slider");
		ShowWindow (SW_SHOW);
		return FALSE;
	}

	m_hDefaultSlider = pDefaultSlider->m_hWnd;

	CRect rectBar;
	GetWindowRect (rectBar);
	
	CBCGPDockManager* pDockManager = globalUtils.GetDockManager (GetDockSite ());
	ASSERT_VALID (pDockManager);

	CSize minSize;
	GetMinSize (minSize);
	BOOL bSetMinSize = FALSE;
	if (rectBar.Width () < minSize.cx)
	{
		rectBar.right = rectBar.left + minSize.cx;
		bSetMinSize = TRUE;
	}
	if (rectBar.Height () < minSize.cy)
	{
		rectBar.bottom = rectBar.top + minSize.cy;
		bSetMinSize = TRUE;
	}

	if (pDockManager->AdjustRectToClientArea (rectBar, dwAlignment) ||
		bSetMinSize)
	{
		SetWindowPos (NULL, 0, 0, rectBar.Width (), rectBar.Height (),  
							SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
	}
	
	pDefaultSlider->AddControlBar (this);

	// register this docking bar and slider with the frame's window dock manager
	if (!bOuterEdge)
	{
		AddControlBar (this);
		AddControlBar (pDefaultSlider);
	}
	else
	{
		pDockManager->AddControlBar (pDefaultSlider, !bOuterEdge, FALSE, bOuterEdge);
		pDockManager->AddControlBar (this, !bOuterEdge, FALSE, bOuterEdge);
	}

	SetBarAlignment (dwAlignment);
	pDefaultSlider->SetBarAlignment (GetCurrentAlignment ());
	m_recentDockInfo.m_dwRecentAlignmentToFrame = GetCurrentAlignment ();

	EnableGripper (TRUE);

	if (lpRect != NULL)
	{
		CRect rect (lpRect);
		SetWindowPos (NULL, 0, 0, rect.Width (), rect.Height (),  
					SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE | SWP_FRAMECHANGED);
	}

	if (bLocked)
	{
		UnlockWindowUpdate ();
	}
	AdjustDockingLayout ();
	OnAfterDock (this, NULL, BCGP_DM_UNKNOWN);
	return TRUE;
}
//***********************************************************************
BOOL CBCGPDockingControlBar::DockToWindow (CBCGPDockingControlBar* pTargetWindow, 
										  DWORD dwAlignment, 
										  LPCRECT lpRect)
{

	ASSERT_VALID (this);
	ASSERT_VALID (pTargetWindow);
	ASSERT (dwAlignment & CBRS_ALIGN_ANY || lpRect != NULL);
	ASSERT_KINDOF (CBCGPDockingControlBar, pTargetWindow);

	CBCGPSlider* pSlider = pTargetWindow->GetDefaultSlider ();

	if (pSlider == NULL)
	{
		ShowWindow (SW_SHOW);
		return FALSE;
	}


	if (m_hDefaultSlider != NULL && IsWindow (m_hDefaultSlider))
	{
		UnDockControlBar (FALSE);
	}

	RemoveFromMiniframe (BCGPGetParentFrame (this), BCGP_DM_UNKNOWN);
	
	if (pSlider->InsertControlBar (this, pTargetWindow, dwAlignment, lpRect))
	{
		// the bar was successfully inserted into slider's container. Now, we need 
		// to register it with the frame
		InsertControlBar (this, pTargetWindow, TRUE);
		m_hDefaultSlider = pSlider->m_hWnd;

		EnableGripper (TRUE);
		// force NcCalcSize to recalculate and draw the caption (gripper)
		SetWindowPos (NULL, 0, 0, 0, 0, 
						SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | 
						SWP_NOREDRAW | SWP_FRAMECHANGED | SWP_SHOWWINDOW);

		AdjustDockingLayout ();
		OnAfterDock (this, NULL, BCGP_DM_UNKNOWN);
		return TRUE;
	}
	
	return FALSE;
}
//*****************************************************************************************
BOOL CBCGPDockingControlBar::DockContainer (CBCGPBarContainerManager& barContainerManager, 
											DWORD dwAlignment, BCGP_DOCK_METHOD /*dockMethod*/)
{
	if (m_hDefaultSlider != NULL && IsWindow (m_hDefaultSlider))
	{
		CObList lstControlBars; 
		barContainerManager.AddControlBarsToList (&lstControlBars, NULL);

		for (POSITION pos = lstControlBars.GetHeadPosition (); pos != NULL;)
		{
			CBCGPDockingControlBar* pBar = 
				DYNAMIC_DOWNCAST (CBCGPDockingControlBar, lstControlBars.GetNext (pos));

			InsertControlBar (pBar, this, TRUE);
			pBar->SetDefaultSlider (m_hDefaultSlider); 
			pBar->SetBarAlignment (GetCurrentAlignment ());
		}

		CBCGPSlider* pDefaultSlider = GetDefaultSlider ();
		if (pDefaultSlider != NULL)
		{
			return pDefaultSlider->AddContainer (this, barContainerManager, dwAlignment);
		}
	}

	return FALSE;
}
//*****************************************************************************************
void CBCGPDockingControlBar::DrawCaption (CDC* pDC, CRect rectCaption)
{
	ASSERT_VALID (pDC);

	CRect rcbtnRight = CRect (rectCaption.BottomRight (), CSize (0, 0));
	int i = 0;

	for (i = (int) m_arrButtons.GetUpperBound (); i >= 0; i --)
	{
		if (!m_arrButtons [i]->m_bLeftAlign && !m_arrButtons [i]->m_bHidden)
		{
			rcbtnRight = m_arrButtons [i]->GetRect();
			break;
		}
	}

	CRect rcbtnLeft = CRect (rectCaption.TopLeft (), CSize (0, 0));
	for (i = (int) m_arrButtons.GetUpperBound (); i >= 0; i --)
	{
		if (m_arrButtons [i]->m_bLeftAlign && !m_arrButtons [i]->m_bHidden)
		{
			rcbtnLeft = m_arrButtons [i]->GetRect();
			break;
		}
	}

	COLORREF clrCptnText = CBCGPVisualManager::GetInstance ()->OnDrawControlBarCaption (
		pDC, this, m_bActive, rectCaption, rcbtnRight);

	for (i = 0; i < m_arrButtons.GetSize (); i ++)
	{
		CBCGPCaptionButton* pbtn = m_arrButtons [i];
		ASSERT_VALID (pbtn);

		pbtn->m_clrForeground = clrCptnText;
	}

    int nOldBkMode = pDC->SetBkMode(TRANSPARENT);
    COLORREF clrOldText = pDC->SetTextColor (clrCptnText);

    CFont* pOldFont = pDC->SelectObject (CBCGPVisualManager::GetInstance()->UseLargeCaptionFontInDockingCaptions() ?
		&globalData.fontCaption : &globalData.fontRegular);
	ASSERT (pOldFont != NULL);

    CString strTitle;
    GetWindowText (strTitle);

	rectCaption.right = rcbtnRight.left;
	rectCaption.left = rcbtnLeft.right;
	rectCaption.top++;
	rectCaption.DeflateRect (g_nCaptionHorzMargin * 2, 0);

	if (CBCGPVisualManager::GetInstance ()->IsDockingTabUpperCase())
	{
		strTitle.MakeUpper();
	}

	CBCGPVisualManager::GetInstance ()->OnDrawControlBarCaptionText (
		pDC, this, m_bActive, strTitle, rectCaption);

    pDC->SelectObject(pOldFont);
    pDC->SetBkMode(nOldBkMode);
    pDC->SetTextColor(clrOldText);
}
//*****************************************************************************************
void CBCGPDockingControlBar::RedrawButton (const CBCGPCaptionButton* pButton)
{
	if (pButton == NULL /*|| GetParentMiniFrame (TRUE) != NULL*/)
	{
		return;
	}

	if (!pButton->m_bEnabled)
	{
		return;
	}

	m_rectRedraw = pButton->GetRect ();
	SendMessage (WM_NCPAINT);
	m_rectRedraw.SetRectEmpty ();

	UpdateWindow ();
}
//*****************************************************************************************
void CBCGPDockingControlBar::SetCaptionStyle (BOOL bDrawText, BOOL /*bForceGradient*/,
											BOOL bHideDisabledButtons)
{
	m_bCaptionText = bDrawText;
	m_bHideDisabledButtons = bHideDisabledButtons;
}
//*****************************************************************************************
void CBCGPDockingControlBar::AdjustBarWindowToContainer (CBCGPSlider* pSlider)
{
	CRect rectContainer = pSlider->GetRootContainerRect ();
	if (!rectContainer.IsRectEmpty ())
	{
		CFrameWnd* pFrame = GetParentFrame ();
		if (pFrame != NULL)
		{
			ASSERT_VALID (pFrame);
			pFrame->ScreenToClient (rectContainer);
			CRect rectWnd;
			GetWindowRect (rectWnd);
			pFrame->ScreenToClient (rectWnd);

			CRect rectUnion;
			rectUnion.UnionRect (rectWnd, rectContainer);

			if (rectUnion != rectContainer)
			{
				rectWnd.OffsetRect (rectContainer.left - rectWnd.left, 
									rectContainer.top - rectWnd.top); 
			if (rectWnd.Width () > rectContainer.Width ())
			{
				rectWnd.right = rectWnd.left + rectContainer.Width ();
			}
			if (rectWnd.Height () > rectContainer.Height ())
			{
				rectWnd.bottom = rectWnd.top + rectContainer.Height ();
			}

			SetWindowPos (NULL, rectWnd.left, rectWnd.top, 
				 rectContainer.Width (), rectContainer.Height (),  
				 SWP_NOZORDER | SWP_NOACTIVATE);
			}
		}
	}
}
//*****************************************************************************************
void CBCGPDockingControlBar::ShowControlBar (BOOL bShow, BOOL bDelay, BOOL bActivate)
{
	if (IsAutoHideMode ())
	{
		if (IsHideInAutoHideMode ())
		{
			if (IsBarVisible () && !bShow)
			{
				m_pAutoHideButton->ShowAttachedWindow (FALSE);
			}
			m_pAutoHideBar->ShowAutoHideWindow(this, bShow, bDelay);
		}
		else
		{
			m_pAutoHideButton->ShowAttachedWindow (TRUE);
			if (bShow && bActivate)
			{
				SetFocus ();

				if (!m_bActive)
				{
					m_bActive = TRUE;
					OnChangeActiveState();
				}
			}
		}
	}
	else if (IsFloating () || IsTabbed ())
	{
		// standard procedure - show/hide bar and its miniframe
		CBCGPControlBar::ShowControlBar (bShow, bDelay, bActivate);
		CBCGPMiniFrameWnd* pMiniFrame = GetParentMiniFrame ();
		if (pMiniFrame != NULL)
		{
			pMiniFrame->OnShowControlBar (this, bShow);
		}
		if (IsTabbed () && bDelay)
		{
			GetParentTabbedBar ()->RecalcLayout ();
		}
	}
	else if (IsMDITabbed ())
	{
		CWnd* pParent = GetParent ();
		if (bShow)
		{
			ConvertToTabbedDocument ();			
			ShowWindow (SW_SHOW);
		}
		else 
		{
			pParent->SendMessage (WM_CLOSE);
		}
	}
	else
	{
		CBCGPSlider* pDefaultSlider = GetDefaultSlider ();
		ShowWindow (bShow ? SW_SHOW : SW_HIDE);
		if (bShow && pDefaultSlider != NULL)
		{
			// adjust rect to fit the container, otherwise it will break the size 
			// of container;
			AdjustBarWindowToContainer (pDefaultSlider);
		}
		
		CBCGPMiniFrameWnd* pMiniFrame = GetParentMiniFrame ();

		if (pMiniFrame != NULL)
		{
			pMiniFrame->OnShowControlBar (this, bShow);
		}
		else if (pDefaultSlider != NULL)
		{
			if (bShow)
			{
				int nLastPercent = GetLastPercentInContainer ();
				if (nLastPercent >= 50)
				{
					SetLastPercentInContainer (50);
				}
				else
				{
					SetLastPercentInContainer (nLastPercent + 1);
				}
			}
			
			// docked at main frame - notify to adjust container
			pDefaultSlider->OnShowControlBar (this, bShow);
			if (!bDelay)
			{
				AdjustDockingLayout ();
			}
		}
		else 
		{
			// floating with other bars on miniframe  - notify to adjust container
			
		}
		
	}

	if (IsTabbed () && bShow && bActivate)
	{
		CBCGPBaseTabWnd* pParentTab = DYNAMIC_DOWNCAST (CBCGPBaseTabWnd, GetParent ());
		if (pParentTab == NULL)
		{
			ASSERT (FALSE);
			return;
		}

		ASSERT_VALID (pParentTab);
		pParentTab->SetActiveTab (pParentTab->GetTabFromHwnd (GetSafeHwnd ()));
	}
}
//*****************************************************************************************
void CBCGPDockingControlBar::UnDockControlBar (BOOL bDelay)
{
	CBCGPMiniFrameWnd* pMiniFrame = 
		DYNAMIC_DOWNCAST (CBCGPMiniFrameWnd, GetParentMiniFrame ());

	if (pMiniFrame == NULL)
	{
		RemoveFromDefaultSlider ();
		// remove from dock site		
		RemoveControlBarFromDockManager (this, FALSE, !bDelay);

		if (!bDelay && !IsFloating ())
		{
			AdjustDockingLayout ();
		}
	}
	else
	{
		pMiniFrame->RemoveControlBar (this);
	}
}
//*****************************************************************************************
void CBCGPDockingControlBar::OnDestroy() 
{
	RemoveCaptionButtons ();

	if (GetParentMiniFrame () != NULL)
	{
		RemoveFromMiniframe (NULL, BCGP_DM_UNKNOWN);
	}
	else
	{
		UnDockControlBar (TRUE);
	}	

	if (IsMDITabbed ())
	{
		CBCGPDockManager* pDockManager = globalUtils.GetDockManager (GetDockSite ());
		pDockManager->RemoveHiddenMDITabbedBar (this);


		CBCGPMDIChildWnd* pWnd = DYNAMIC_DOWNCAST (CBCGPMDIChildWnd, GetParent ());
		if (pWnd != NULL)
		{
			pWnd->PostMessage (WM_CLOSE);
		}
	}

	CBCGPTooltipManager::DeleteToolTip (m_pToolTip);

	CBCGPControlBar::OnDestroy();
}
//*************************************************************************************
void CBCGPDockingControlBar::OnTrackCaptionButtons (CPoint point)
{
	if (CBCGPPopupMenu::GetActiveMenu () != NULL)
	{
		return;
	}

	UINT nHot = m_nHot;

	CBCGPCaptionButton* pBtn = FindButton (point);
	if (pBtn != NULL)
	{
		m_nHot = pBtn->GetHit ();

		if (m_nHit == HTNOWHERE || m_nHit == m_nHot)
		{
			pBtn->m_bFocused = TRUE;
		}
	}
	else
	{
		m_nHot = HTNOWHERE;
	}

	if (m_nHot != nHot)
	{
		RedrawButton (pBtn);

		CBCGPCaptionButton* pBtnOld = FindButtonByHit (nHot);
		if (pBtnOld != NULL)
		{
			pBtnOld->m_bFocused = FALSE;
			RedrawButton (pBtnOld);
		}
	}

	if (m_nHit == HTNOWHERE)
	{
		if (nHot != HTNOWHERE && m_nHot == HTNOWHERE)
		{
			::ReleaseCapture();
			m_bCaptionButtonsCaptured = FALSE;
		}
		else if (nHot == HTNOWHERE && m_nHot != HTNOWHERE)
		{
			SetCapture ();
			m_bCaptionButtonsCaptured = TRUE;
		}
	}
}
//*************************************************************************************
void CBCGPDockingControlBar::OnNcMouseMove(UINT nHitTest, CPoint point) 
{
	if (!m_bPrepareToFloat)
	{
		OnTrackCaptionButtons (point);
	}

	CBCGPControlBar::OnNcMouseMove(nHitTest, point);
}
//************************************************************************************
void CBCGPDockingControlBar::StopCaptionButtonsTracking ()
{
	if (m_nHit != HTNOWHERE)
	{
		CBCGPCaptionButton* pBtn = FindButtonByHit (m_nHit);
		m_nHit = HTNOWHERE;

		ReleaseCapture ();
		if (pBtn != NULL)
		{
			pBtn->m_bPushed = FALSE;
			RedrawButton (pBtn);
		}

		KillTimer(BCGP_SCROLL_TIMER_ID);
	}

	if (m_nHot != HTNOWHERE)
	{
		CBCGPCaptionButton* pBtn = FindButtonByHit (m_nHot);
		m_nHot = HTNOWHERE;

		ReleaseCapture ();
		if (pBtn != NULL)
		{
			pBtn->m_bFocused = FALSE;
			RedrawButton (pBtn);
		}
	}

	m_bCaptionButtonsCaptured = FALSE;
}
//*************************************************************************************
void CBCGPDockingControlBar::OnCancelMode() 
{
	StopCaptionButtonsTracking ();
	if (m_bPrepareToFloat)
	{
		m_bPrepareToFloat = false;
	}
	CBCGPControlBar::OnCancelMode();
}
//*************************************************************************************
CBCGPCaptionButton* CBCGPDockingControlBar::FindButton (CPoint point) const
{
	ASSERT_VALID (this);

	CRect rcBar;
    GetWindowRect(rcBar);
	ScreenToClient (rcBar);

	int i = 0;

	for (i = 0; i < m_arrButtons.GetSize (); i ++)
	{
		CBCGPCaptionButton* pbtn = m_arrButtons [i];
		ASSERT_VALID (pbtn);

		CRect rectBtn = pbtn->GetRect ();
		rectBtn.OffsetRect (rcBar.TopLeft());

		ClientToScreen (rectBtn);

		if (rectBtn.PtInRect (point))
		{
			return pbtn;
		}
	}

	for (i = 0; i < m_arScrollButtons.GetSize(); i++)
	{
		CBCGPDockingBarScrollButton* pbtn = m_arScrollButtons[i];
		ASSERT_VALID (pbtn);

		CRect rectBtn = pbtn->GetRect ();
		rectBtn.OffsetRect (rcBar.TopLeft());
		rectBtn.OffsetRect(0, -m_nBorderSize);

		ClientToScreen (rectBtn);

		if (rectBtn.PtInRect (point))
		{
			return pbtn;
		}
	}

	return NULL;
}
//*****************************************************************************************
CBCGPCaptionButton* CBCGPDockingControlBar::FindButtonByHit (UINT nHit) const
{
	ASSERT_VALID (this);

	int i = 0;

	for (i = 0; i < m_arrButtons.GetSize (); i ++)
	{
		CBCGPCaptionButton* pbtn = m_arrButtons [i];
		ASSERT_VALID (pbtn);

		if (pbtn->GetHit () == nHit)
		{
			return pbtn;
		}
	}

	for (i = 0; i < m_arScrollButtons.GetSize(); i++)
	{
		CBCGPDockingBarScrollButton* pbtn = m_arScrollButtons[i];
		ASSERT_VALID (pbtn);

		if (pbtn->GetHit () == nHit)
		{
			return pbtn;
		}
	}

	return NULL;
}
//*****************************************************************************************
void CBCGPDockingControlBar::EnableButton (UINT nHit, BOOL bEnable/* = TRUE*/)
{
	ASSERT_VALID (this);

	CBCGPCaptionButton* pButton = FindButtonByHit (nHit);
	if (pButton == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	pButton->m_bEnabled = bEnable;
}
//******************************************************************************
BOOL CBCGPDockingControlBar::IsButtonEnabled (UINT nHit) const
{
	ASSERT_VALID (this);

	CBCGPCaptionButton* pButton = FindButtonByHit (nHit);
	if (pButton == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	return pButton->m_bEnabled;
}
//******************************************************************************
void CBCGPDockingControlBar::OnUpdateCmdUI(class CFrameWnd *pTarget, int bDisableIfNoHndler)
{
    UpdateDialogControls(pTarget, bDisableIfNoHndler);

    CWnd* pFocus = GetFocus();
    BOOL bActiveOld = m_bActive;

    m_bActive = (pFocus->GetSafeHwnd () != NULL && 
		(IsChild (pFocus) || pFocus->GetSafeHwnd () == GetSafeHwnd ()));

    if (m_bActive != bActiveOld)
	{
		OnChangeActiveState();
        SendMessage (WM_NCPAINT);
	}
}
//*****************************************************************************************
BOOL CBCGPDockingControlBar::IsVisible () const
{
	if (IsAutoHideMode ())
	{
		if (!IsHideInAutoHideMode ())
		{
			return FALSE;
		}
		return m_pAutoHideBar->IsVisible ();
	}
	return CBCGPControlBar::IsVisible ();
}
//*****************************************************************************************
BOOL CBCGPDockingControlBar::PreTranslateMessage(MSG* pMsg) 
{
   	switch (pMsg->message)
	{
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
	case WM_NCLBUTTONDOWN:
	case WM_NCRBUTTONDOWN:
	case WM_NCMBUTTONDOWN:
	case WM_NCLBUTTONUP:
	case WM_NCRBUTTONUP:
	case WM_NCMBUTTONUP:
	case WM_MOUSEMOVE:
		if (m_pToolTip->GetSafeHwnd () != NULL)
		{
			m_pToolTip->RelayEvent(pMsg);
		}
		break;
	}

	if (pMsg->message == WM_KEYDOWN && (GetDockMode () & BCGP_DT_STANDARD) != 0 && m_bPrepareToFloat &&
		pMsg->wParam == VK_ESCAPE)
	{
		if (m_bPrepareToFloat)
		{
			PostMessage (WM_CANCELMODE);
			return TRUE;
		}
		else if (IsFloating ())
		{
			CBCGPMiniFrameWnd* pParentWnd = DYNAMIC_DOWNCAST (CBCGPMiniFrameWnd, GetParent ());
			if (pParentWnd != NULL && GetCapture () == pParentWnd)
			{
				pParentWnd->PostMessage (WM_CANCELMODE);
				return TRUE;
			}
		}
	}

	if (pMsg->message == WM_KEYDOWN && IsTabbed () && pMsg->wParam == VK_ESCAPE)
	{
		CBCGPBaseTabbedBar* pParentBar = GetParentTabbedBar ();
		CBCGPMiniFrameWnd* pParentMiniFrame = (pParentBar == NULL) ? NULL : pParentBar->GetParentMiniFrame ();

		if (pParentBar != NULL && 
			(pParentBar->IsTracked () || pParentMiniFrame != NULL && pParentMiniFrame->IsCaptured ()))
		{
			if (pParentMiniFrame != NULL)
			{
				pParentMiniFrame->PostMessage (WM_CANCELMODE);
			}
			else
			{
				pParentBar->PostMessage (WM_CANCELMODE);
			}

			return TRUE;
		}
	}

	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE && 
		(GetDockMode () & BCGP_DT_SMART) != 0)
	{
		CBCGPDockManager* pDockManager = globalUtils.GetDockManager (GetParent ());
		if (pDockManager != NULL)
		{
			CBCGPSmartDockingManager* pSDManager = pDockManager->GetSDManagerPermanent ();
			if (pSDManager != NULL && pSDManager->IsStarted ())
			{
				CBCGPMiniFrameWnd* pParentWnd = DYNAMIC_DOWNCAST (CBCGPMiniFrameWnd, GetParent ());
				if (pParentWnd != NULL && GetCapture () == pParentWnd)
				{
					pParentWnd->PostMessage (WM_CANCELMODE);
					return TRUE;
				}
			}
		}
	}

	return CBCGPControlBar::PreTranslateMessage(pMsg);
}
//*****************************************************************************************
void CBCGPDockingControlBar::SetDefaultSlider (HWND hSliderWnd)  
{
	if (m_hDefaultSlider != hSliderWnd)
	{
		CBCGPSlider* pDefaultSlider = GetDefaultSlider ();
		if (pDefaultSlider != NULL)
		{
			pDefaultSlider->RemoveControlBar (this);
		}
	}
	m_hDefaultSlider = hSliderWnd;
}
//*****************************************************************************************
BOOL CBCGPDockingControlBar::LoadState (LPCTSTR lpszProfileName, int nIndex, UINT uiID)
{
	return CBCGPControlBar::LoadState (lpszProfileName, nIndex, uiID);
}
//*****************************************************************************************
BOOL CBCGPDockingControlBar::SaveState (LPCTSTR lpszProfileName, int nIndex, UINT uiID)
{
	return CBCGPControlBar::SaveState (lpszProfileName, nIndex, uiID);
}
//*****************************************************************************************
void CBCGPDockingControlBar::Serialize (CArchive& ar)
{
	CBCGPControlBar::Serialize (ar);
	if (ar.IsLoading ())
	{
		ar >> m_recentDockInfo.m_rectRecentFloatingRect;
		ar >> m_rectSavedDockedRect;
		m_recentDockInfo.m_recentSliderInfo.m_rectDockedRect = m_rectSavedDockedRect;
		ar >> m_bRecentFloatingState;
	}
	else
	{
		BOOL bFloating = IsFloating ();

		if (bFloating)
		{
			CBCGPMiniFrameWnd* pMiniFrame = GetParentMiniFrame ();
			if (pMiniFrame != NULL)
			{
				pMiniFrame->GetWindowRect (m_recentDockInfo.m_rectRecentFloatingRect);
			}
		}
		else
		{
			CalcRecentDockedRect ();
		}

		ar << m_recentDockInfo.m_rectRecentFloatingRect;
		ar << m_recentDockInfo.m_recentSliderInfo.m_rectDockedRect;
		ar << m_bRecentFloatingState;
	}
}
//*****************************************************************************************
void CBCGPDockingControlBar::GetRecentSiblingBarInfo (CList<UINT, UINT&>& /*lstBarIDs*/)
{
}
//*****************************************************************************************
LRESULT CBCGPDockingControlBar::OnSetText(WPARAM, LPARAM lParam) 
{
	LRESULT	lRes = Default();

	if (!lRes)
	{
		return lRes;
	}
	
	CBCGPMiniFrameWnd* pParentMiniFrame = NULL;

	if (IsTabbed())
	{
		// If we are docked on a tabbed control bar, we have to update the tab label
		CBCGPBaseTabWnd* pParentTabWnd	= 
			DYNAMIC_DOWNCAST(CBCGPBaseTabWnd, GetParent());

		ASSERT_VALID (pParentTabWnd);	

		CWnd* pWndTabbedControlBar = 
			DYNAMIC_DOWNCAST(CBCGPBaseTabbedBar, pParentTabWnd->GetParent());

		if (pWndTabbedControlBar != NULL)
		{
			LPCTSTR	lpcszTitle	= reinterpret_cast<LPCTSTR> (lParam);
			int		iTab	= pParentTabWnd->GetTabFromHwnd(GetSafeHwnd());
			CString	strLabel;
			if (iTab >= 0 && iTab < pParentTabWnd->GetTabsNum ())
			{
				VERIFY(pParentTabWnd->GetTabLabel(iTab, strLabel));
				if (strLabel != lpcszTitle)
				{
					VERIFY(pParentTabWnd->SetTabLabel(iTab, lpcszTitle));
				}
			}
		}
	}
	else if ((pParentMiniFrame = GetParentMiniFrame ()) != NULL)
	{
		pParentMiniFrame->SetWindowPos (NULL, 0, 0, 0, 0, 
				SWP_NOZORDER | SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE | 
				SWP_FRAMECHANGED);
	}
	else if (IsAutoHideMode ())
	{
		ASSERT_VALID (m_pAutoHideBar);
		m_pAutoHideBar->RedrawWindow ();
		SetWindowPos (NULL, 0, 0, 0, 0, 
				SWP_NOZORDER | SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE | 
				SWP_FRAMECHANGED);
		AdjustDockingLayout ();
	}
	else 
	{
		SetWindowPos (NULL, 0, 0, 0, 0, 
				SWP_NOZORDER | SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE | 
				SWP_FRAMECHANGED);
	}


	return lRes;
}
//*****************************************************************************************
CBCGPControlBar* CBCGPDockingControlBar::DockControlBarStandard (BOOL& bWasDocked)
{
	CBCGPBaseControlBar* pTargetBar = NULL;
    int nSensitivity = ((GetDockMode () & BCGP_DT_SMART) != 0)
        ? -1 : CBCGPDockManager::m_nDockSencitivity;

	BCGP_CS_STATUS status = IsChangeState (nSensitivity, &pTargetBar);

	CBCGPDockingControlBar* pTargetDockingBar = 
		DYNAMIC_DOWNCAST (CBCGPDockingControlBar, pTargetBar);

	if (pTargetDockingBar == this || GetAsyncKeyState (VK_CONTROL) < 0)
	{
		return NULL;
	}

	CBCGPMultiMiniFrameWnd* pTargetMiniFrame = pTargetDockingBar != NULL ? 
		DYNAMIC_DOWNCAST (CBCGPMultiMiniFrameWnd, pTargetDockingBar->GetParentMiniFrame ()) : NULL;

	if (status == BCGP_CS_DELAY_DOCK) // status returned by resizable control bar
	{
	
		if (pTargetMiniFrame != NULL)
		{
			if ((GetBarStyle () & CBRS_FLOAT_MULTI) == 0)
			{
				return NULL;
			}
			else if (pTargetBar != NULL)
			{
				bWasDocked = !pTargetMiniFrame->DockBar (this);
				return this;
			}
		}

		bWasDocked = DockControlBar (pTargetDockingBar, NULL, BCGP_DM_STANDARD);
	}
	else if (status == BCGP_CS_DELAY_DOCK_TO_TAB && pTargetDockingBar != NULL && 
			 pTargetDockingBar->CanAcceptBar (this) && CanBeAttached ())
	{
		UnDockControlBar (FALSE);
		CBCGPDockingControlBar* pBar = AttachToTabWnd (pTargetDockingBar, BCGP_DM_STANDARD);
		bWasDocked = (pBar != NULL);
		return pBar;
	}

	return NULL;
}
//*****************************************************************************************
CBCGPSlider* CBCGPDockingControlBar::GetDefaultSlider () const 
{
	return DYNAMIC_DOWNCAST (CBCGPSlider, CWnd::FromHandlePermanent (m_hDefaultSlider));
}
//*****************************************************************************************
BCGP_CS_STATUS CBCGPDockingControlBar::GetDockStatus (CPoint pt, int nSencitivity) 
{
	ASSERT_VALID (this);


	BCGP_DOCK_TYPE docktype = GetDockMode ();

	CBCGPDockManager* pDockManager = globalUtils.GetDockManager (GetDockSite ());
	CBCGPSmartDockingMarker::SDMarkerPlace nHilitedSideNo = CBCGPSmartDockingMarker::sdNONE;

	if ((docktype & BCGP_DT_SMART) != 0 && pDockManager != NULL)
	{
		CBCGPSmartDockingManager* pSDManager = pDockManager->GetSDManager ();
		if (pSDManager != NULL && pSDManager->IsStarted())
		{
			nHilitedSideNo = pSDManager->GetHilitedMarkerNo ();
		}
	}

	// detect caption
	UINT nHitTest = HitTest (pt, TRUE);

	CRect rectTabAreaTop;
	CRect rectTabAreaBottom;
	GetTabArea (rectTabAreaTop, rectTabAreaBottom);

	if (nHitTest == HTCAPTION || rectTabAreaTop.PtInRect (pt) ||
		rectTabAreaBottom.PtInRect (pt) || nHilitedSideNo == CBCGPSmartDockingMarker::sdCMIDDLE)
	{
		// need to display "ready to create detachable tab" status
		return BCGP_CS_DELAY_DOCK_TO_TAB;
	}
	else
	{
		CRect rectBar;
		GetWindowRect (&rectBar);
		
		rectBar.top += GetCaptionHeight ();
		rectBar.top += rectTabAreaTop.Height (); 
		rectBar.bottom -= rectTabAreaBottom.Height (); 

        if (nSencitivity == -1)
        {
            // is it demanded?
		    if (rectBar.PtInRect (pt))
		    {
			    // mouse over an edge
			    return BCGP_CS_DELAY_DOCK;
		    }
        }
        else
        {
            rectBar.DeflateRect (nSencitivity, nSencitivity);
		    if (!rectBar.PtInRect (pt))
		    {
			    // mouse over an edge
			    return BCGP_CS_DELAY_DOCK;
		    }
        }
	}

	return BCGP_CS_NOTHING;
}
//*****************************************************************************************
BOOL CBCGPDockingControlBar::CanAcceptMiniFrame (CBCGPMiniFrameWnd* pMiniFrame) const
{
	return pMiniFrame->CanBeDockedToBar (this);
}
//*****************************************************************************************
BOOL CBCGPDockingControlBar::IsFloatingMulti () const
{
	CBCGPMiniFrameWnd* pMiniFrame = GetParentMiniFrame ();
	if (pMiniFrame != NULL)
	{	
		return pMiniFrame->IsKindOf (RUNTIME_CLASS (CBCGPMultiMiniFrameWnd));
	}
	return FALSE;
}
//*****************************************************************************************
void CBCGPDockingControlBar::SetCaptionButtons ()
{
	RemoveCaptionButtons ();

	m_arrButtons.Add (new CBCGPCaptionButton (HTCLOSE_BCG));
	m_arrButtons.Add (new CBCGPCaptionButton (HTMAXBUTTON));
	m_arrButtons.Add (new CBCGPCaptionButton (HTMINBUTTON));
}
//*****************************************************************************************
void CBCGPDockingControlBar::RemoveCaptionButtons ()
{
	for (int i = 0; i < m_arrButtons.GetSize (); i++)
	{
		delete m_arrButtons[i];
	}
	m_arrButtons.RemoveAll ();
}
//*****************************************************************************************
void CBCGPDockingControlBar::OnSettingChange(UINT uFlags, LPCTSTR lpszSection) 
{
	CBCGPControlBar::OnSettingChange(uFlags, lpszSection);
	
	if (m_cyGripper > 0)
	{
		m_cyGripper = 0;
		EnableGripper (TRUE);
	}
}
//*****************************************************************************************
BOOL CBCGPDockingControlBar::OnNeedTipText(UINT /*id*/, NMHDR* pNMH, LRESULT* /*pResult*/)
{
	static CString strTipText;

	if (m_pToolTip->GetSafeHwnd () == NULL || 
		pNMH->hwndFrom != m_pToolTip->GetSafeHwnd ())
	{
		return FALSE;
	}

	LPNMTTDISPINFO	pTTDispInfo	= (LPNMTTDISPINFO) pNMH;
	ASSERT((pTTDispInfo->uFlags & TTF_IDISHWND) == 0);

	int nIndex = (int)pNMH->idFrom - 1;
	if (nIndex >= 0 && nIndex < (int) m_arrButtons.GetSize ())
	{
		CBCGPCaptionButton* pbtn = m_arrButtons [nIndex];
		ASSERT_VALID (pbtn);

		if (pbtn->GetCustomToolTip (strTipText))
		{
			pTTDispInfo->lpszText = const_cast<LPTSTR> ((LPCTSTR) strTipText);
			return TRUE;
		}
	}

	UINT nTooltipResID = 0;

	switch (pNMH->idFrom)
	{
	case 1:
		nTooltipResID = IDS_BCGBARRES_CLOSEBAR;
		break;

	case 2:
		{
			SHORT state = GetAsyncKeyState (VK_CONTROL);
			nTooltipResID = IDS_BCGBARRES_AUTOHIDEBAR;
			
			if ((state & 0x8000) && IsAutohideAllEnabled ())
			{
				nTooltipResID = IDS_BCGBARRES_AUTOHIDE_ALL;
			}
		}
		break;

	case 3:
		nTooltipResID = IDS_BCGBARRES_MENU;
		break;
	}

	if (nTooltipResID == 0)
	{
		return FALSE;
	}

	{
		CBCGPLocalResource locaRes;
		strTipText.LoadString (nTooltipResID);
	}

	pTTDispInfo->lpszText = const_cast<LPTSTR> ((LPCTSTR) strTipText);
	return TRUE;
}
//*****************************************************************************************
void CBCGPDockingControlBar::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	if (IsTracked ())
	{
		return;
	}

	if (m_bCaptionButtonsCaptured)
	{
		StopCaptionButtonsTracking ();
	}

	CBCGPControlBar::OnContextMenu (pWnd, point);
}
//*****************************************************************************************
void CBCGPDockingControlBar::OnSetFocus(CWnd* pOldWnd) 
{
	CBCGPControlBar::OnSetFocus(pOldWnd);

	CBCGPMultiMiniFrameWnd* pParentMiniFrame = 
		DYNAMIC_DOWNCAST (CBCGPMultiMiniFrameWnd, GetParentMiniFrame ());
	if (pParentMiniFrame != NULL)
	{
		pParentMiniFrame->SetLastFocusedBar (GetSafeHwnd ());
	}
}
//*****************************************************************************************
void CBCGPDockingControlBar::ToggleAutoHide ()
{
	ASSERT_VALID (this);

	CBCGPSlider* pDefaultSlider = GetDefaultSlider ();

	if (CanAutoHide () && pDefaultSlider != NULL)
	{
		SetAutoHideMode (!m_bPinState, 
			pDefaultSlider->GetCurrentAlignment ());
	}
}
//*****************************************************************************************
BOOL CBCGPDockingControlBar::CanAutoHide () const
{
	ASSERT_VALID (this);

	if (!CBCGPControlBar::CanAutoHide ())
	{
		return FALSE;
	}

	CWnd* pParentWnd = GetParent ();
	if (pParentWnd == NULL)
	{
		return FALSE;
	}

	if (pParentWnd->IsKindOf (RUNTIME_CLASS (CBCGPMiniFrameWnd)))
	{
		pParentWnd = pParentWnd->GetParent ();
	}

	if (pParentWnd == NULL)
	{
		return FALSE;
	}

	CBCGPSlider* pDefaultSlider = GetDefaultSlider ();
	CBCGPDockManager* pDockManager = globalUtils.GetDockManager (pParentWnd);

	return pDockManager != NULL &&
		pDefaultSlider != NULL && 
		(pDefaultSlider->GetCurrentAlignment () & 
		 pDockManager->GetEnabledAutoHideAlignment ());
}
//*****************************************************************************************
void CBCGPDockingControlBar::CopyState (CBCGPDockingControlBar* pOrgBar)
{
	ASSERT_VALID (pOrgBar);
	CBCGPControlBar::CopyState (pOrgBar);

	m_rectRestored			= pOrgBar->GetAHRestoredRect ();
	m_ahSlideMode			= pOrgBar->GetAHSlideMode ();
	m_nLastPercent			= pOrgBar->GetLastPercentInContainer ();
	m_bEnableAutoHideAll	= pOrgBar->IsAutohideAllEnabled ();
}
//**************************************************************************
LRESULT CBCGPDockingControlBar::OnBCGUpdateToolTips (WPARAM wp, LPARAM)
{
	UINT nTypes = (UINT) wp;

	if (nTypes & BCGP_TOOLTIP_TYPE_DOCKBAR)
	{
		CBCGPTooltipManager::CreateToolTip (m_pToolTip, this,
			BCGP_TOOLTIP_TYPE_DOCKBAR);

		for (int i = 0; i < (int) m_arrButtons.GetSize (); i ++)
		{
			CBCGPLocalResource locaRes;

			CRect rectDummy;
			rectDummy.SetRectEmpty ();

			m_pToolTip->AddTool (this, LPSTR_TEXTCALLBACK, &rectDummy, i + 1);
		}

		UpdateTooltips ();
	}

	return 0;
}
//**************************************************************************
int CBCGPDockingControlBar::GetCaptionHeight () const
{
	if (IsFloating () || IsMDITabbed () || m_cyGripper == 0)
	{
		return 0;
	}

	return m_cyGripper + CBCGPVisualManager::GetInstance ()->GetDockingBarCaptionExtraHeight() + CBCGPVisualManager::GetInstance ()->GetCaptionButtonExtraBorder ().cy;
}
//**************************************************************************
void CBCGPDockingControlBar::ConvertToTabbedDocument (BOOL bActiveTabOnly)
{
	ASSERT_VALID (this);

	if (IsAutoHideMode ())
	{
		return;
	}

	CBCGPMDIFrameWnd* pMDIFrame = DYNAMIC_DOWNCAST (CBCGPMDIFrameWnd, GetDockSite ());
	if (pMDIFrame == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	ASSERT_VALID (pMDIFrame);
	if (IsTabbed ())
	{
		CBCGPTabbedControlBar* pBar = DYNAMIC_DOWNCAST (CBCGPTabbedControlBar, GetParentTabbedBar ());
		if (pBar != NULL)
		{
			pBar->ConvertToTabbedDocument (bActiveTabOnly);
		}
	}
	else
	{
		pMDIFrame->ControlBarToTabbedDocument (this);
	}
}
//**************************************************************************
void CBCGPDockingControlBar::OnSize(UINT nType, int cx, int cy) 
{
	CBCGPControlBar::OnSize(nType, cx, cy);

	CArray<BOOL, BOOL> arScrollButtonStates;
	int i = 0;

	for (i = 0; i < m_arScrollButtons.GetSize(); i++)
	{
		CBCGPDockingBarScrollButton* pbtn = m_arScrollButtons[i];
		ASSERT_VALID (pbtn);

		arScrollButtonStates.Add(pbtn->m_bHidden);
	}

	CalcScrollButtons();
	UpdateScrollBars();

	BOOL bRedrawFrame = FALSE;

	for (i = 0; i < m_arScrollButtons.GetSize(); i++)
	{
		CBCGPDockingBarScrollButton* pbtn = m_arScrollButtons[i];
		ASSERT_VALID (pbtn);

		if (!pbtn->GetRect().IsRectEmpty())
		{
			bRedrawFrame = TRUE;
		}

		if (pbtn->m_bHidden != arScrollButtonStates[i])
		{
			SetWindowPos (NULL, 0, 0, 0, 0, 
					SWP_NOZORDER | SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE | SWP_FRAMECHANGED);
			bRedrawFrame = TRUE;
			break;
		}
	}

	if (bRedrawFrame)
	{
		PostMessage(UM_REDRAWFRAME);
	}
}
//**************************************************************************
LRESULT CBCGPDockingControlBar::OnRedrawFrame(WPARAM, LPARAM)
{
	RedrawWindow (NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW);
	return 0;
}
//**************************************************************************
void CBCGPDockingControlBar::CalcScrollButtons()
{
	const int nScrollButtonSize = GetScrollButtonSize();

	for (int i = 0; i < m_arScrollButtons.GetSize(); i++)
	{
		CBCGPDockingBarScrollButton* pbtn = m_arScrollButtons[i];
		ASSERT_VALID (pbtn);

		pbtn->SetRect(CRect(0, 0, 0, 0));
		pbtn->m_bHidden = TRUE;
		pbtn->m_bFocused = pbtn->m_bPushed = FALSE;
	}

	CRect rectScroll;
	GetWindowRect (&rectScroll);
	ScreenToClient (&rectScroll);

	if (rectScroll.Width() < nScrollButtonSize * 3 || rectScroll.Height() < nScrollButtonSize * 3)
	{
		return;
	}

	int y1 = -1;
	int y2 = -1;
	BOOL bIsScrollLeft = ScrollHorzAvailable(TRUE);

	if (ScrollVertAvailable(TRUE))
	{
		CRect rectScrollUp (rectScroll);

		rectScrollUp.top += GetCaptionHeight () - rectScrollUp.top;
		rectScrollUp.bottom = rectScrollUp.top + nScrollButtonSize;

		y1 = rectScrollUp.bottom - 1;

		if (bIsScrollLeft)
		{
			rectScrollUp.OffsetRect(nScrollButtonSize, 0);
		}

		m_arScrollButtons[0]->SetRect(rectScrollUp);
		m_arScrollButtons[0]->m_bHidden = FALSE;
	}
	else
	{
		y1 = rectScroll.top + GetCaptionHeight () - rectScroll.top;
	}

	if (ScrollVertAvailable(FALSE))
	{
		CRect rectScrollDown (rectScroll);

		rectScrollDown.bottom -= rectScrollDown.top;
		rectScrollDown.top = rectScrollDown.bottom - nScrollButtonSize;

		y2 = rectScrollDown.top + 1;

		if (bIsScrollLeft)
		{
			rectScrollDown.OffsetRect(nScrollButtonSize, 0);
		}

		m_arScrollButtons[1]->SetRect(rectScrollDown);
		m_arScrollButtons[1]->m_bHidden = FALSE;
	}
	else
	{
		y2 = rectScroll.bottom - rectScroll.top;
	}

	if (bIsScrollLeft)
	{
		CRect rectScrollLeft (rectScroll);

		rectScrollLeft.top = y1;
		rectScrollLeft.bottom = y2;
		rectScrollLeft.left -= rectScrollLeft.left;
		rectScrollLeft.right = rectScrollLeft.left + nScrollButtonSize;

		m_arScrollButtons[2]->SetRect(rectScrollLeft);
		m_arScrollButtons[2]->m_bHidden = FALSE;
	}

	if (ScrollHorzAvailable(FALSE))
	{
		CRect rectScrollRight (rectScroll);

		rectScrollRight.top = y1;
		rectScrollRight.bottom = y2;
		rectScrollRight.right -= rectScrollRight.left;
		rectScrollRight.left = rectScrollRight.right - nScrollButtonSize;

		m_arScrollButtons[3]->SetRect(rectScrollRight);
		m_arScrollButtons[3]->m_bHidden = FALSE;
	}
}
//**************************************************************************
int CBCGPDockingControlBar::GetScrollButtonSize() const
{
	return max(CBCGPMenuImages::Size().cx, CBCGPMenuImages::Size().cy) + 2;
}
//**************************************************************************
BOOL CBCGPDockingControlBar::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
	if (CBCGPPopupMenu::GetActiveMenu () != NULL)
	{
		return TRUE;
	}

	if (nFlags & (MK_SHIFT | MK_CONTROL))
	{
		return FALSE;
	}

	BOOL bRes = FALSE;

	BOOL bUp   = ScrollVertAvailable(TRUE);
	BOOL bDown = ScrollVertAvailable(FALSE);

	if (bUp || bDown)
	{
		if (zDelta > 0 && ScrollVertAvailable(TRUE))
		{
			OnScrollClient(MAKEWORD(-1, SB_LINEUP));
			bRes = TRUE;
		}
		else if (zDelta < 0 && ScrollVertAvailable(FALSE))
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
		
		if (g_pWorkspace != NULL && g_pWorkspace->IsMouseWheelInInactiveWindowEnabled())
		{
			return TRUE;
		}
	}

	return CBCGPControlBar::OnMouseWheel(nFlags, zDelta, pt);
}
//**************************************************************************
LRESULT CBCGPDockingControlBar::OnChangeVisualManager (WPARAM, LPARAM)
{
	if (m_cyGripper > 0)
	{
		m_cyGripper = 0;
		EnableGripper (TRUE);
	}

	return 0;
}
