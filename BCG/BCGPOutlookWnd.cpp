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
//
// BCGPOutlookWnd.cpp : implementation file
//

#include "stdafx.h"
#include "bcgcbpro.h"
#include "BCGPOutlookWnd.h"
#include "BCGPOutlookBarPane.h"
#include "BCGPOutlookBar.h"
#include "BCGPDockingCBWrapper.h"
#include "BCGGlobals.h"
#include "BCGPLocalResource.h"
#include "BCGPVisualManager.h"
#include "BCGPOutlookButton.h"
#include "BCGPDockManager.h"
#include "bcgprores.h"
#include "CustomizeButton.h"
#include "BCGPMultiMiniFrameWnd.h"
#include "BCGPDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static const int nToolbarMarginHeight = 4;
static const UINT idShowMoreButtons = 0xf200;
static const UINT idShowFewerButtons = 0xf201;
static const UINT idNavigationPaneOptions = 0xf202;
static const UINT idToolbarCommandID = 0xf203;
static const int nTopEdgeHeight = 4;

/////////////////////////////////////////////////////////////////////////////
// CBCGPOutlookSrcrollButton

void CBCGPOutlookSrcrollButton::OnFillBackground (CDC* pDC, const CRect& rectClient)
{
	COLORREF clrText = globalData.clrBarText;
	CBCGPVisualManager::GetInstance ()->OnFillOutlookPageButton (pDC,
		rectClient, m_bHighlighted, m_bPushed, clrText);
}

void CBCGPOutlookSrcrollButton::OnDrawBorder (CDC* pDC, CRect& rectClient, UINT /*uiState*/)
{
	CBCGPVisualManager::GetInstance ()->OnDrawOutlookPageButtonBorder (
		pDC, rectClient, m_bHighlighted, m_bPushed);
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPOutlookCustomizeButton

class CBCGPOutlookCustomizeButton : public CCustomizeButton
{
	DECLARE_DYNCREATE(CBCGPOutlookCustomizeButton)

	friend class CBCGPOutlookWnd;

	virtual void OnDraw (CDC* pDC, const CRect& rect, CBCGPToolBarImages* pImages,
						BOOL bHorz = TRUE, BOOL bCustomizeMode = FALSE,
						BOOL bHighlight = FALSE,
						BOOL bDrawBorder = TRUE,
						BOOL bGrayDisabledButtons = TRUE);
	virtual CBCGPPopupMenu* CreatePopupMenu ();
};

IMPLEMENT_DYNCREATE(CBCGPOutlookCustomizeButton, CCustomizeButton)

CBCGPPopupMenu* CBCGPOutlookCustomizeButton::CreatePopupMenu ()
{
	CBCGPPopupMenu* pMenu = CCustomizeButton::CreatePopupMenu ();
	if (pMenu == NULL)
	{
		return NULL;
	}

	pMenu->RemoveItem (pMenu->GetMenuBar ()->CommandToIndex (m_uiCustomizeCmdId));

	if (pMenu->GetMenuItemCount () > 0)
	{
		pMenu->InsertSeparator ();
	}

	CBCGPLocalResource locaRes;
	CString strItem;

	strItem.LoadString (IDS_BCGBARRES_SHOW_MORE_BUTTONS);
	pMenu->InsertItem (CBCGPToolbarMenuButton (idShowMoreButtons, NULL, -1, strItem));

	strItem.LoadString (IDS_BCGBARRES_SHOW_FEWER_BUTTONS);
	pMenu->InsertItem (CBCGPToolbarMenuButton (idShowFewerButtons, NULL, -1, strItem));

	strItem.LoadString (IDS_BCGBARRES_NAV_PANE_OPTIONS);
	pMenu->InsertItem (CBCGPToolbarMenuButton (idNavigationPaneOptions, NULL, -1, strItem));

	return pMenu;
}

void CBCGPOutlookCustomizeButton::OnDraw (CDC* pDC, const CRect& rect, CBCGPToolBarImages* /*pImages*/,
			BOOL /*bHorz*/, BOOL /*bCustomizeMode*/, BOOL bHighlight,
			BOOL /*bDrawBorder*/, BOOL /*bGrayDisabledButtons*/)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (this);

	m_bDefaultDraw = TRUE;

	FillInterior (pDC, rect, bHighlight || IsDroppedDown ());

	CSize sizeImage = CBCGPMenuImages::Size ();

	int x = rect.left + max (0, (rect.Width () - sizeImage.cx) / 2);
	int y = rect.top + max (0, (rect.Height () - 2 * sizeImage.cy) / 2);

	CBCGPVisualManager::BCGBUTTON_STATE state = IsDroppedDown() ? CBCGPVisualManager::ButtonsIsPressed : 
		bHighlight ? CBCGPVisualManager::ButtonsIsHighlighted : CBCGPVisualManager::ButtonsIsRegular;

	COLORREF clrText = CBCGPVisualManager::GetInstance()->GetToolbarButtonTextColor(this, state);

	CBCGPMenuImages::DrawByColor(pDC, CBCGPMenuImages::IdMoreButtons, CPoint (x, y), clrText, FALSE);
	y += sizeImage.cy;
	CBCGPMenuImages::DrawByColor(pDC, CBCGPMenuImages::IdArowDown, CPoint (x, y), clrText, FALSE);
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPOutlookBarToolBar

IMPLEMENT_DYNAMIC(CBCGPOutlookBarToolBar, CBCGPToolBar)

CBCGPOutlookBarToolBar::CBCGPOutlookBarToolBar (CBCGPOutlookWnd* pParentBar) :
	m_pParentBar (pParentBar)
{
	m_bLocked = TRUE;
}

BEGIN_MESSAGE_MAP(CBCGPOutlookBarToolBar, CBCGPToolBar)
	//{{AFX_MSG_MAP(CBCGPOutlookBarToolBar)
	//}}AFX_MSG_MAP
	ON_WM_SETCURSOR()
	ON_WM_NCCALCSIZE()
	ON_WM_NCPAINT()
END_MESSAGE_MAP()

BOOL CBCGPOutlookBarToolBar::OnSendCommand (const CBCGPToolbarButton* pButton)
{
	int nIndex = ButtonToIndex (pButton);
	if (nIndex >= 0)
	{
		int iTab = -1;

		if (m_TabButtons.Lookup (nIndex, iTab))
		{
			if (m_pParentBar->SetActiveTab (iTab) &&
				m_pParentBar->GetParentFrame () != NULL)
			{
				m_pParentBar->GetParentFrame()->SendMessage (
					BCGM_CHANGE_ACTIVE_TAB, iTab, 0);
			}

			return TRUE;
		}
	}

	return FALSE;
}

void CBCGPOutlookBarToolBar::OnChangeVisualManager()
{
	CBCGPToolBar::OnChangeVisualManager();

	CBCGPOutlookWnd* pParent = DYNAMIC_DOWNCAST(CBCGPOutlookWnd, GetParent());
	if (pParent->GetSafeHwnd() != NULL)
	{
		pParent->RebuildToolBar();
	}
}

void CBCGPOutlookBarToolBar::OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL /*bDisableIfNoHndler*/)
{
	for (int i = 0; i < m_Buttons.GetCount (); i++)
	{
		UINT nNewStyle = GetButtonStyle(i) &
					~(TBBS_CHECKED | TBBS_INDETERMINATE);
		
		int iTab = -1;
		if (m_TabButtons.Lookup (i, iTab))
		{
			if (m_pParentBar->GetActiveTab () == iTab)
			{
				nNewStyle |= TBBS_CHECKED;
			}

			SetButtonStyle (i, nNewStyle | TBBS_CHECKBOX);
		}
	}
}

BOOL CBCGPOutlookBarToolBar::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	CPoint ptCursor;
	::GetCursorPos (&ptCursor);

	ScreenToClient (&ptCursor);

	if (HitTest (ptCursor) >= 0)
	{
		::SetCursor (globalData.GetHandCursor ());
		return TRUE;
	}

	return CBCGPToolBar::OnSetCursor(pWnd, nHitTest, message);
}

BOOL CBCGPOutlookBarToolBar::OnUserToolTip (CBCGPToolbarButton* pButton, CString& strTTText) const
{
	strTTText = pButton->m_strText;
	return TRUE;
}

void CBCGPOutlookBarToolBar::AdjustLocations ()
{
	const double dblImageScale = globalData.GetRibbonImageScale ();

	CSize sizeImage = GetImageSize ();
	if (sizeImage == CSize (0, 0))
	{
		sizeImage = CSize (16, 16);
	}

	CSize sizeButton = sizeImage + CSize (10, 6 + 2 * nToolbarMarginHeight);
	if (dblImageScale != 1.)
	{
		sizeButton = CSize ((int)(.5 + sizeButton.cx * dblImageScale), (int)(.5 + sizeButton.cy * dblImageScale));
	}

	CSize sizeCustomizeButton (0, 0);
	if (m_pCustomizeBtn != NULL)
	{
		sizeCustomizeButton = sizeButton;
		sizeCustomizeButton.cx = 
			max (sizeCustomizeButton.cx, CBCGPMenuImages::Size ().cx + 10);
	}

	CRect rectToolbar;
	GetClientRect (rectToolbar);

	int nCount = sizeCustomizeButton == CSize (0, 0) ? 
		(int) m_Buttons.GetCount () :
		(int) m_Buttons.GetCount () - 1;

	int x = rectToolbar.right -  sizeCustomizeButton.cx + 2;

	int nCountToHide = nCount - (rectToolbar.Width () - sizeCustomizeButton.cx + 2) / 
		(sizeButton.cx - 2);

	for (POSITION pos = m_Buttons.GetTailPosition ();  pos != NULL;)
	{
		CBCGPToolbarButton* pButton = (CBCGPToolbarButton*) m_Buttons.GetPrev (pos); 
		ASSERT_VALID (pButton);
		CCustomizeButton* pCustomizeBtn = DYNAMIC_DOWNCAST (CCustomizeButton, pButton);

		if (nCountToHide >0 && pCustomizeBtn == NULL)
		{
			CObList& list = const_cast<CObList&> (m_pCustomizeBtn->GetInvisibleButtons ());
            list.AddHead (pButton);
			pButton->SetRect (CRect (0, 0, 0, 0));
			nCountToHide--;
		}
		else
		{
			CSize sizeCurrButton = sizeButton;

			if (pButton == m_pCustomizeBtn)
			{
				sizeCurrButton = sizeCustomizeButton;
			}

			sizeCurrButton.cy++;
			pButton->SetRect (CRect (CPoint (x, -1), sizeCurrButton));

			x -= sizeButton.cx - 2;
		}
	}

	UpdateTooltips ();
}

void CBCGPOutlookBarToolBar::OnNcCalcSize(BOOL /*bCalcValidRects*/, NCCALCSIZE_PARAMS FAR* /*lpncsp*/)
{
}

void CBCGPOutlookBarToolBar::OnNcPaint()
{
}

void CBCGPOutlookBarToolBar::OnCustomizeMode (BOOL bSet)
{
	CBCGPToolBar::OnCustomizeMode (bSet);
	EnableWindow (!bSet);
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPOutlookWnd

IMPLEMENT_DYNCREATE(CBCGPOutlookWnd, CBCGPBaseTabWnd)

BOOL CBCGPOutlookWnd::m_bEnableAnimation = FALSE;

#pragma warning (disable : 4355)

CBCGPOutlookWnd::CBCGPOutlookWnd() :
	m_wndToolBar (this)
{
	m_rectWndArea.SetRectEmpty ();
	m_rectCaption.SetRectEmpty ();
	m_bHasCaption = TRUE;
	m_bDrawFrame = TRUE;
	m_bDrawBottomLine = TRUE;
	m_nBorderSize = 0;
	m_bActivateOnBtnUp = TRUE;
	m_bEnableTabSwap = FALSE;
	m_bScrollButtons = FALSE;

	m_btnUp.m_nFlatStyle = CBCGPButton::BUTTONSTYLE_SEMIFLAT;
	m_btnUp.m_bDrawFocus = FALSE;

	m_btnDown.m_nFlatStyle = CBCGPButton::BUTTONSTYLE_SEMIFLAT;
	m_btnDown.m_bDrawFocus = FALSE;

	m_nPageButtonTextAlign = TA_CENTER;

	m_bIsTracking = FALSE;
	m_rectSplitter.SetRectEmpty ();

	m_nVisiblePageButtons = -1;
	m_nMaxVisiblePageButtons = 0;
	m_bDontAdjustLayout = FALSE;

	m_sizeToolbarImage = CSize (0, 0);

	m_bAlphaBlendIcons = FALSE;
	m_bIsPrintingClient = FALSE;
}

#pragma warning (default : 4355)

CBCGPOutlookWnd::~CBCGPOutlookWnd()
{
}

BEGIN_MESSAGE_MAP(CBCGPOutlookWnd, CBCGPBaseTabWnd)
	//{{AFX_MSG_MAP(CBCGPOutlookWnd)
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_SETCURSOR()
	ON_WM_ERASEBKGND()
	ON_WM_CREATE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_CANCELMODE()
	//}}AFX_MSG_MAP
	ON_COMMAND_RANGE(idShowMoreButtons, idShowMoreButtons + 10, OnToolbarCommand)
	ON_UPDATE_COMMAND_UI_RANGE(idShowMoreButtons, idShowMoreButtons + 10, OnUpdateToolbarCommand)
	ON_MESSAGE(WM_PRINTCLIENT, OnPrintClient)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGPOutlookWnd message handlers

BOOL CBCGPOutlookWnd::Create (const CRect& rect, CWnd* pParentWnd, UINT nID)
{
	if (!CWnd::Create (NULL, _T(""), 
							WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 
							rect, pParentWnd, nID))
	{
		return FALSE;
	}

	SetTabsHeight ();
	m_bHighLightTabs = TRUE;

	return TRUE;
}
//*********************************************************************************
BOOL CBCGPOutlookWnd::IsPtInTabArea (CPoint point) const
{
	CRect rectTop; rectTop.SetRectEmpty ();
	CRect rectBottom; rectBottom.SetRectEmpty ();
	GetTabArea (rectTop, rectBottom);
	
	ScreenToClient (rectTop);
	ScreenToClient (rectBottom);

	return rectTop.PtInRect (point) || rectBottom.PtInRect (point);
}
//*********************************************************************************
void CBCGPOutlookWnd::AddControl (CWnd* pWndCtrl, LPCTSTR lpszName, int nImageID,
	BOOL bDetachable, DWORD dwBCGStyle)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pWndCtrl);

	AddTab (pWndCtrl, lpszName, nImageID, bDetachable);
	
	if (bDetachable && !IsMode2003 ())
	{
		int nInsertedTab = GetTabFromHwnd (pWndCtrl->GetSafeHwnd ());

		CBCGPDockingCBWrapper* pWrapper = DYNAMIC_DOWNCAST (CBCGPDockingCBWrapper, 
								GetTabWnd (nInsertedTab));
		if (pWrapper != NULL)
		{
			ASSERT_VALID (pWrapper);

			pWrapper->SetTabbedControlBarRTC (RUNTIME_CLASS (CBCGPOutlookBar));
			pWrapper->SetMiniFrameRTC (RUNTIME_CLASS (CBCGPMultiMiniFrameWnd));

			// we need this flag for the runtime checking
			pWrapper->SetBarStyle(pWrapper->GetBarStyle() | CBRS_FLOAT_MULTI);
			pWrapper->SetBCGStyle (dwBCGStyle);
		}
	}
}
//********************************************************************************
void CBCGPOutlookWnd::RecalcLayout ()
{
	ASSERT_VALID (this);

	if (GetSafeHwnd () == NULL || m_nTabsHeight == 0)
	{
		return;
	}

	const BOOL bIsMode2003 = IsMode2003 ();
	int nToolBarHeight = 0;

	if (bIsMode2003)
	{
		CSize sizeImage (0, 0);
			
		if (m_imagesToolbar.GetSafeHandle () != NULL)
		{
			sizeImage = m_sizeToolbarImage;
		}
		else
		{
			sizeImage = GetImageSize ();
		}

		if (sizeImage.cy == 0)
		{
			sizeImage.cy = 16;
		}

		const double dblImageScale = globalData.GetRibbonImageScale ();

		int nVertMargin = 6 + 2 * nToolbarMarginHeight - 2;
		if (dblImageScale != 1.)
		{
			nVertMargin = (int)(.5 + nVertMargin * dblImageScale);
		}

		nToolBarHeight = sizeImage.cy + nVertMargin;
	}

	m_btnUp.SendMessage (WM_CANCELMODE);
	m_btnDown.SendMessage (WM_CANCELMODE);

	CRect rectClient;
	GetClientRect (rectClient);

	rectClient.DeflateRect(m_nBorderSize + 1, m_nBorderSize + 1);

	m_rectWndArea = rectClient;

	int nVisibleTabsNum = GetVisibleTabsNum ();

	if (bIsMode2003)
	{
		if (m_nVisiblePageButtons == -1)
		{
			m_nVisiblePageButtons = nVisibleTabsNum;
		}

		if (m_nVisiblePageButtons > nVisibleTabsNum)
		{
			// Maybe, pages were removed?
			m_nVisiblePageButtons = nVisibleTabsNum;
		}

		m_nMaxVisiblePageButtons = min (nVisibleTabsNum, (rectClient.Height () - m_nTabsHeight - nToolBarHeight) / 
									(2 * m_nTabsHeight));
		int nVisiblePageButtons = min (m_nMaxVisiblePageButtons, m_nVisiblePageButtons);

		if (m_bHasCaption)
		{
			m_rectCaption = rectClient;

			int nTextHeight = CBCGPVisualManager::GetInstance()->UseLargeCaptionFontInDockingCaptions() ?
				globalData.GetCaptionTextHeight() : globalData.GetTextHeight();

			m_rectCaption.bottom = m_rectCaption.top + nTextHeight + 2 * CBCGPBaseTabWnd::TAB_TEXT_MARGIN;
			m_rectCaption.top += nTopEdgeHeight - 1;
		}
		else
		{
			m_rectCaption.SetRectEmpty();
		}

		m_rectSplitter = rectClient;
		m_rectSplitter.bottom -= nToolBarHeight + m_nTabsHeight * nVisiblePageButtons;
		m_rectSplitter.top = m_rectSplitter.bottom - CBCGPVisualManager::GetInstance()->GetOutlookBarSplitterHeight();

		m_rectWndArea.top = m_bHasCaption ? m_rectCaption.bottom : rectClient.top + nTopEdgeHeight - 1;
		m_rectWndArea.bottom = m_rectSplitter.top;
	}
	else
	{
		m_rectCaption.SetRectEmpty ();
		m_rectSplitter.SetRectEmpty ();

		if (nVisibleTabsNum > 1 || !IsHideSingleTab ())
		{
			m_rectWndArea.DeflateRect (0, 1);
		}
	}

	int y = bIsMode2003 ? m_rectSplitter.bottom : rectClient.top;

	if (nVisibleTabsNum > 1 || !IsHideSingleTab ())
	{
		for (int i = 0; i < m_iTabsNum; i ++)
		{
			CBCGPTabInfo* pTab = (CBCGPTabInfo*) m_arTabs [i];
			ASSERT_VALID (pTab);
			
			pTab->m_rect = rectClient;
			pTab->m_rect.top = y;
			pTab->m_rect.right++;
			pTab->m_rect.bottom = y + m_nTabsHeight;

			if (pTab->m_rect.top >= rectClient.bottom - nToolBarHeight && 
				bIsMode2003)
			{
				pTab->m_rect.SetRectEmpty ();
			}

			if (!pTab->m_bVisible)
			{
				pTab->m_rect.SetRectEmpty ();
				continue;
			}

			if (m_bScrollButtons && !bIsMode2003 &&
				(i == m_iActiveTab || i == m_iActiveTab + 1))
			{
				CRect rectScroll = pTab->m_rect;
				pTab->m_rect.right -= m_nTabsHeight;
				rectScroll.left = pTab->m_rect.right;

				if (i == m_iActiveTab)
				{
					m_btnUp.SetWindowPos (NULL, rectScroll.left, rectScroll.top,
						rectScroll.Width (), rectScroll.Height (),
						SWP_NOACTIVATE | SWP_NOZORDER);
				}
				else
				{
					m_btnDown.SetWindowPos (NULL, rectScroll.left, rectScroll.top,
						rectScroll.Width (), rectScroll.Height (),
						SWP_NOACTIVATE | SWP_NOZORDER);
				}
			}

			if (i == m_iActiveTab && !bIsMode2003)
			{
				m_rectWndArea.top = y + m_nTabsHeight;
				
				int nVisibleAfter = 0;
				for (int j = i + 1; j < m_iTabsNum; j ++)
				{
					CBCGPTabInfo* pTab = (CBCGPTabInfo*) m_arTabs [j];
					if (pTab->m_bVisible)
					{
						nVisibleAfter++;
					}
				}
				
				y = rectClient.bottom - m_nTabsHeight * nVisibleAfter + 1;
				m_rectWndArea.bottom = y - 1;
			}
			else
			{
				y += m_nTabsHeight;
			}
		}
	}

	
	if (m_bScrollButtons && !bIsMode2003 && m_iActiveTab == nVisibleTabsNum - 1)
	{
		m_rectWndArea.bottom -= m_nTabsHeight;

		m_btnDown.SetWindowPos (NULL, rectClient.right - m_nTabsHeight + 1,
			rectClient.bottom - m_nTabsHeight + 1,
			m_nTabsHeight, m_nTabsHeight,
			SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);
	}

	for (int i = 0; i < m_iTabsNum; i ++)
	{
		CBCGPTabInfo* pTab = (CBCGPTabInfo*) m_arTabs [i];
		ASSERT_VALID (pTab);

		CBCGPOutlookBarPane* pOutlookPane = NULL;
		
		if (pTab->m_bVisible)
		{
			CBCGPDockingCBWrapper* pWrapper = DYNAMIC_DOWNCAST (CBCGPDockingCBWrapper, pTab->m_pWnd);
			if (pWrapper != NULL)
			{
				pOutlookPane = DYNAMIC_DOWNCAST (CBCGPOutlookBarPane, pWrapper->GetWrappedWnd ());
				if (pOutlookPane != NULL)
				{
					pOutlookPane->m_nSize = m_rectWndArea.Width ();
					pOutlookPane->m_nMaxLen = m_rectWndArea.Height ();

					if (m_bDontAdjustLayout)
					{
						pOutlookPane->m_bDontAdjustLayout = TRUE;
					}
				}
			}

			pTab->m_pWnd->SetWindowPos (NULL,
				m_rectWndArea.left, m_rectWndArea.top,
				m_rectWndArea.Width (), m_rectWndArea.Height (),
				SWP_NOACTIVATE | SWP_NOZORDER);

			if (pOutlookPane != NULL)
			{
				pOutlookPane->m_bDontAdjustLayout = FALSE;
			}
		}
	}

	if (nVisibleTabsNum != 0 || bIsMode2003) 
	{
		RedrawWindow (NULL, NULL, 
			RDW_FRAME | RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW | RDW_ALLCHILDREN);
	}
	else
	{
		ShowWindow (SW_HIDE);
	}

	if (bIsMode2003)
	{
		m_wndToolBar.ShowWindow (SW_SHOWNOACTIVATE);
		m_wndToolBar.SetWindowPos (NULL, 
			rectClient.left, rectClient.bottom - nToolBarHeight,
			rectClient.Width (), nToolBarHeight,
			SWP_NOZORDER | SWP_NOACTIVATE);
		RebuildToolBar ();
	}
	else
	{
		m_wndToolBar.ShowWindow (SW_HIDE);

		m_btnUp.RedrawWindow ();
		m_btnDown.RedrawWindow ();

		GetParent ()->RedrawWindow (NULL, NULL);
	}
}
//**********************************************************************************
BOOL CBCGPOutlookWnd::SetActiveTab (int iTab)
{
	ASSERT_VALID (this);

	if (iTab < 0 || iTab >= m_iTabsNum)
	{
		TRACE(_T("SetActiveTab: illegal tab number %d\n"), iTab);
		return FALSE;
	}

	if (iTab >= m_arTabs.GetSize ())
	{
		ASSERT (FALSE);
		return FALSE;
	}

	BOOL bIsFirstTime = (m_iActiveTab == -1);

	if (m_iActiveTab == iTab)	// Already active, do nothing
	{
		return TRUE;
	}

	if (FireChangingActiveTab (iTab))
	{
		return FALSE;
	}

	const BOOL bIsMode2003 = IsMode2003 ();

	//-------------------------------------------------------------------
	// Show active tab with animation only if tab was activated by mouse:
	//-------------------------------------------------------------------
	BOOL bAnimate = (m_iHighlighted == m_iPressed) && (m_iHighlighted != -1) && 
		m_bEnableAnimation && !bIsMode2003;

	CBCGPOutlookBar* pOutlookBar = 
		DYNAMIC_DOWNCAST (CBCGPOutlookBar, GetParent ());
	if (pOutlookBar != NULL && !pOutlookBar->OnBeforeAnimation (iTab))
	{
		bAnimate = FALSE;
	}

	if (globalData.bIsRemoteSession)
	{
		// Disable animation in Terminal Services Environment
		bAnimate = FALSE;
	}
	
	int iOldActiveTab = m_iActiveTab;
	CWnd* pWndOld = GetActiveWnd ();
	
	m_iActiveTab = iTab;
	CWnd* pWndActive = GetActiveWnd ();
	if (pWndActive == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	ASSERT_VALID (pWndActive);

	if (bAnimate)
	{
		BOOL bMoveDown = (m_iActiveTab < iOldActiveTab);

#ifdef _DEBUG
		CBCGPTabInfo* pTabInfoNew = (CBCGPTabInfo*) m_arTabs [m_iActiveTab];
		CBCGPTabInfo* pTabInfoOld = (CBCGPTabInfo*) m_arTabs [iOldActiveTab];

		ASSERT_VALID (pTabInfoNew);
		ASSERT_VALID (pTabInfoOld);
#endif

		CRect rectClient;
		GetClientRect (rectClient);

		CRect rectOldWnd;

		pWndOld->GetWindowRect (rectOldWnd);	
		ScreenToClient (rectOldWnd);

		const int dy = bMoveDown ? 30 : -30;
		const int nSteps = abs (rectOldWnd.Height () / dy);

		//---------------------
		// Hide scroll buttons:
		//---------------------
		BOOL bScrollButtons = m_bScrollButtons && !bIsMode2003;
		BOOL bIsUp = m_btnUp.IsWindowEnabled ();
		BOOL bIsDown = m_btnDown.IsWindowEnabled ();

		if (bScrollButtons)
		{
			m_btnUp.ShowWindow (SW_HIDE);
			m_btnDown.ShowWindow (SW_HIDE);

			for (int i = 0; i < m_iTabsNum; i++)
			{
				CBCGPTabInfo* pTab = (CBCGPTabInfo*) m_arTabs [i];
				ASSERT_VALID (pTab);
				
				if (i == m_iActiveTab || i == m_iActiveTab + 1)
				{
					pTab->m_rect.right += m_nTabsHeight;
				}
			}
		}

		CRect rectOld;

		if (bMoveDown)
		{
			CBCGPTabInfo* pTabInfo = (CBCGPTabInfo*) m_arTabs [m_iActiveTab + 1];
			rectOld = pTabInfo->m_rect;
			rectOld.bottom = rectOld.top + dy;
		}
		else
		{
			CBCGPTabInfo* pTabInfo = (CBCGPTabInfo*) m_arTabs [m_iActiveTab];
			rectOld = pTabInfo->m_rect;
			rectOld.top = rectOld.bottom + dy;
		}

		ModifyStyle (WS_CLIPCHILDREN, 0, SWP_NOREDRAW);

		CClientDC dc (this);

		CFont* pOldFont = (CFont*) dc.SelectObject (&globalData.fontRegular);
		dc.SetBkMode (TRANSPARENT);

		int nStartBtnIdx = bMoveDown ? m_iActiveTab + 1 : iOldActiveTab + 1;
		int nEndBtnIdx = bMoveDown ? iOldActiveTab : m_iActiveTab;

		CRect rectRedraw = rectOldWnd;

		// we need to move all tabs between old active and new active tabs
		BOOL	bPrevDisableRecalcLayout	= CBCGPDockManager::m_bDisableRecalcLayout;
		CBCGPDockManager::m_bDisableRecalcLayout = TRUE;

		for (int i = 0; i < nSteps; i++)
		{
			bMoveDown ? rectOldWnd.top += dy : rectOldWnd.bottom += dy;
			
			pWndOld->SetWindowPos (NULL,
					rectOldWnd.left, rectOldWnd.top, 
					rectOldWnd.Width (), rectOldWnd.Height (), 
					SWP_NOZORDER  | SWP_NOACTIVATE);

			for (int i = nStartBtnIdx; i <= nEndBtnIdx; i++)
			{
				CBCGPTabInfo* pTabInfo = (CBCGPTabInfo*) m_arTabs [i];
				
				pTabInfo->m_rect.OffsetRect (0, dy);
				DrawTabButton (dc, i, FALSE);
			}

			dc.FillRect (rectOld, &globalData.brBarFace);
			rectOld.OffsetRect (0, dy);

			Sleep (10);
		}

		if (bScrollButtons)
		{
			//------------------------
			// Restore scroll buttons:
			//------------------------
			EnableScrollButtons (TRUE, bIsUp, bIsDown);
		}

		CBCGPDockManager::m_bDisableRecalcLayout = bPrevDisableRecalcLayout;
		dc.SelectObject (pOldFont);

		ModifyStyle (0, WS_CLIPCHILDREN, SWP_NOREDRAW);
		pWndOld->ShowWindow (SW_HIDE);

		RecalcLayout ();

		if (pOutlookBar != NULL)
		{
			pOutlookBar->OnAfterAnimation (iTab);
		}

		pWndActive->SetWindowPos (NULL, 0, 0, 0, 0, 
			SWP_NOSIZE | SWP_NOMOVE | SWP_SHOWWINDOW | SWP_NOREDRAW | SWP_NOZORDER | SWP_NOACTIVATE);

		pWndActive->BringWindowToTop ();
		pWndActive->RedrawWindow (NULL, NULL, 
			RDW_FRAME | RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW | RDW_ALLCHILDREN);
	}
	else
	{
		//--------------------
		// Hide active window:
		//--------------------
		if (pWndOld != NULL)
		{
			pWndOld->ShowWindow (SW_HIDE);
		}

		RecalcLayout ();
		
		//------------------------
		// Show new active window:
		//------------------------
		pWndActive->ShowWindow (SW_SHOW);
		pWndActive->BringWindowToTop ();

		//----------------------------------------------------------------------
		// Small trick: to adjust active window scroll sizes, I should change an
		// active window size twice (+1 pixel and -1 pixel):
		//----------------------------------------------------------------------
		BOOL	bPrevDisableRecalcLayout	= CBCGPDockManager::m_bDisableRecalcLayout;
		CBCGPDockManager::m_bDisableRecalcLayout = TRUE;

		pWndActive->SetWindowPos (NULL,
				-1, -1,
				m_rectWndArea.Width () + 1, m_rectWndArea.Height (),
				SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE);
		pWndActive->SetWindowPos (NULL,
				-1, -1,
				m_rectWndArea.Width (), m_rectWndArea.Height (),
				SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE);

		CBCGPDockManager::m_bDisableRecalcLayout = bPrevDisableRecalcLayout;

	}

	//--------------------------------------------------
	// Set text to the parent frame/docking control bar:
	//--------------------------------------------------
	if (pOutlookBar != NULL && 
		pOutlookBar->CanSetCaptionTextToTabName ()) // tabbed dock bar - redraw caption only in this case
	{
		CString strCaption;
		GetTabLabel (m_iActiveTab, strCaption);

		//-------------------------------------------------------
		// Miniframe will take the text from the tab control bar:
		//-------------------------------------------------------
		if (pOutlookBar->CanSetCaptionTextToTabName ())
		{
			pOutlookBar->SetWindowText (strCaption);
		}

		CWnd* pWndToUpdate = pOutlookBar;
		if (!pOutlookBar->IsDocked ())
		{
			pWndToUpdate = pOutlookBar->GetParent ();
		}

		if (pWndToUpdate != NULL)
		{
			pWndToUpdate->RedrawWindow (NULL, NULL, RDW_FRAME | RDW_INVALIDATE);
		}
	}

	//-------------
	// Redraw tabs:
	//-------------
	RedrawWindow ();

	if (!bIsFirstTime)
	{
		CView* pActiveView = DYNAMIC_DOWNCAST (CView, pWndActive);
		if (pActiveView != NULL && !m_bDontActivateView)
		{
			CFrameWnd* pFrame = BCGPGetParentFrame (pActiveView);
			ASSERT_VALID (pFrame);

			pFrame->SetActiveView (pActiveView);
		}
		else
		{
			pWndActive->SetFocus ();
		}
	}

	return TRUE;
}
//**********************************************************************************
CWnd* CBCGPOutlookWnd::FindTargetWnd (const CPoint& pt)
{
	for (int i = 0; i < m_iTabsNum; i ++)
	{
		CBCGPTabInfo* pTab = (CBCGPTabInfo*) m_arTabs [i];
		ASSERT_VALID (pTab);
		
		if (!pTab->m_bVisible)
			continue;
		
		if (pTab->m_rect.PtInRect (pt))
		{
			return NULL;
		}
	}

	CWnd* pWndParent = GetParent ();
	ASSERT_VALID (pWndParent);

	return pWndParent;
}
//**********************************************************************************
void CBCGPOutlookWnd::OnSize(UINT nType, int cx, int cy) 
{
	CWnd::OnSize(nType, cx, cy);
	RecalcLayout ();
}
//***********************************************************************************
void CBCGPOutlookWnd::OnPaint() 
{
	CPaintDC dcPaint (this); // device context for painting
	CBCGPMemDC memDC (dcPaint, this);

	OnDraw(&memDC.GetDC());
}
//***********************************************************************************
void CBCGPOutlookWnd::OnDraw(CDC* pDC)
{
	ASSERT_VALID(pDC);

	int nVisibleTabsNum = GetVisibleTabsNum ();

	CRect rectClient;
	GetClientRect (rectClient);

	CBCGPOutlookBar* pOutlookBar = DYNAMIC_DOWNCAST(CBCGPOutlookBar, GetParent());

	if (!m_bIsPrintingClient || GetActiveTab() < 0)
	{
		if (pOutlookBar->GetSafeHwnd() != NULL)
		{
			CRect rectScreen = globalData.m_rectVirtual;
			ScreenToClient(&rectScreen);
			
			CRect rectFill = rectClient;
			rectFill.left = min (rectFill.left, rectScreen.left);
			
			CBCGPVisualManager::GetInstance()->OnFillBarBackground(pDC, pOutlookBar, rectFill, rectClient);
		}
		else
		{
			pDC->FillRect (rectClient, &globalData.brBarFace);
		}
	}

	//-------------
	// Draw border:
	//-------------
	if (m_nBorderSize > 0)
	{
		CBrush* pOldBrush = pDC->SelectObject (&globalData.brBarFace);
		ASSERT (pOldBrush != NULL);

		pDC->PatBlt (rectClient.left, rectClient.top, m_nBorderSize, rectClient.Height (), PATCOPY);
		pDC->PatBlt (rectClient.left, rectClient.top, rectClient.Width (), m_nBorderSize, PATCOPY);
		pDC->PatBlt (rectClient.right - m_nBorderSize - 1, rectClient.top, m_nBorderSize + 1, rectClient.Height (), PATCOPY);
		pDC->PatBlt (rectClient.left, rectClient.bottom - m_nBorderSize, rectClient.Width (), m_nBorderSize, PATCOPY);

		pDC->SelectObject (pOldBrush);

		rectClient.DeflateRect (m_nBorderSize, m_nBorderSize);
	}

	if (m_rectSplitter.IsRectEmpty() && m_bDrawBottomLine)
	{
		CPen penDrak (PS_SOLID, 1, globalData.clrBarShadow);
		CPen* pOldPen = (CPen*) pDC->SelectObject (&penDrak);
		ASSERT(pOldPen != NULL);

		pDC->MoveTo (m_rectWndArea.left - 1, m_rectWndArea.bottom);
		pDC->LineTo (m_rectWndArea.right + 1, m_rectWndArea.bottom);

		pDC->SelectObject (pOldPen);
	}

	CFont* pOldFont = (CFont*) pDC->SelectObject (
		pOutlookBar != NULL && pOutlookBar->GetButtonsFont () != NULL ?
			pOutlookBar->GetButtonsFont () : &globalData.fontRegular);
	pDC->SetBkMode (TRANSPARENT);

	if (nVisibleTabsNum > 1 || !IsHideSingleTab ())
	{
		for (int i = 0; i < m_iTabsNum; i ++)
		{
			DrawTabButton(*pDC, i);
		}
	}

	CRect rectFrame = rectClient;

	if (!m_rectCaption.IsRectEmpty ())
	{
		// Draw caption:
		CRect rectTop = m_rectCaption;
		rectTop.right++;

		rectTop.top -= nTopEdgeHeight + 1;
		rectTop.bottom = rectTop.top + nTopEdgeHeight + 1;

		pDC->FillRect (rectTop, &globalData.brBarFace);
		
		COLORREF clrText = globalData.clrBarText;
		CBCGPVisualManager::GetInstance ()->OnFillOutlookBarCaption (pDC, m_rectCaption, clrText);

		CString strActivePage;
		GetTabLabel (m_iActiveTab, strActivePage);

		CRect rcText = m_rectCaption;
		rcText.DeflateRect (CBCGPBaseTabWnd::TAB_TEXT_MARGIN, 0);

		UINT uiDTFlags = DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS;

		pDC->SelectObject(CBCGPVisualManager::GetInstance()->UseLargeCaptionFontInDockingCaptions() ? 
			&globalData.fontCaption : &globalData.fontRegular);

		pDC->SetTextColor (clrText);
		pDC->DrawText (strActivePage, rcText, uiDTFlags);

		rectFrame.top = m_rectCaption.bottom;
	}

	if (m_bDrawFrame)
	{
		CBCGPVisualManager::GetInstance ()->OnDrawOutlookBarFrame(pDC, rectFrame);
	}

	if (!m_rectSplitter.IsRectEmpty ())
	{
		// Draw splitter:
		CBCGPVisualManager::GetInstance ()->OnDrawOutlookBarSplitter(pDC, m_rectSplitter);
	}

	// Draw scroll buttons:
	if (m_bScrollButtons && !IsMode2003 ())
	{
		if (m_iActiveTab == m_iTabsNum - 1)
		{
			CRect rectFill = rectClient;
			rectFill.top = rectFill.bottom - m_nTabsHeight;

			pDC->FillRect (rectFill, &globalData.brBarFace);
		}
	}

	pDC->SelectObject (pOldFont);
}
//*************************************************************************************
void CBCGPOutlookWnd::DrawTabButton (CDC& dc, int iButtonIdx, BOOL bDrawPressedButton)
{
	CBCGPTabInfo* pTab = (CBCGPTabInfo*) m_arTabs [iButtonIdx];
	ASSERT_VALID (pTab);

	CRect rectBtn = pTab->m_rect;

	if (rectBtn.IsRectEmpty ())
	{
		return;
	}

	BOOL bIsHighlighted = (iButtonIdx == m_iHighlighted);
	BOOL bIsPressed = (iButtonIdx == m_iPressed) && bDrawPressedButton;
	BOOL bIsActive = (iButtonIdx == m_iActiveTab);

	if (IsMode2003 () && bIsActive)
	{
		bIsPressed = TRUE;
	}

	BOOL bIsSingleStaticTab = GetTabsNum() == 1 && IsSingleTabStatic();

	COLORREF clrBtnText = globalData.clrBarText;

	if (!bIsSingleStaticTab)
	{
		CBCGPVisualManager::GetInstance ()->OnFillOutlookPageButton (&dc,
			rectBtn, bIsHighlighted, bIsPressed, clrBtnText);

		CBCGPVisualManager::GetInstance ()->OnDrawOutlookPageButtonBorder (
			&dc, rectBtn, bIsHighlighted, bIsPressed);
	}

	//---------------
	// Draw tab icon:
	//---------------
	CSize sizeImage = GetImageSize ();
	UINT uiIcon = GetTabIcon (iButtonIdx);
	HICON hIcon = GetTabHicon (iButtonIdx);

	if (uiIcon == (UINT)-1 && hIcon == NULL)
	{
		sizeImage.cx = 0;
	}

	if (sizeImage.cx + CBCGPBaseTabWnd::TAB_IMAGE_MARGIN <= rectBtn.Width ())
	{
		CRect rectImage = rectBtn;

		rectImage.top += (rectBtn.Height () - sizeImage.cy) / 2;
		rectImage.bottom = rectImage.top + sizeImage.cy;

		rectImage.left += IMAGE_MARGIN;
		rectImage.right = rectImage.left + sizeImage.cx;

		if (hIcon != NULL)
		{
			//---------------------
			// Draw the tab's icon:
			//---------------------
			dc.DrawState (rectImage.TopLeft (), rectImage.Size (), 
				hIcon, DSS_NORMAL, (HBRUSH) NULL);
		}
		else
		{
			const CImageList* pImageList = GetImageList ();
			if (pImageList != NULL && uiIcon != (UINT)-1)
			{
				ASSERT_VALID (pImageList);

				//----------------------
				// Draw the tab's image:
				//----------------------
				((CImageList*)pImageList)->Draw (&dc, uiIcon, rectImage.TopLeft (), ILD_TRANSPARENT);
			}
		}
	}

	#define TEXT_MARGIN		4
	#define GRIPPER_MARGIN	4

	//---------------
	// Draw tab text:
	//---------------
	dc.SetTextColor (clrBtnText);

	CRect rcText = pTab->m_rect;

	if (!IsMode2003 ())
	{
		if (pTab->m_bIsDetachable)
		{
			rcText.right -= CX_GRIPPER + GRIPPER_MARGIN * 2;
		}
		else
		{
			rcText.right -= 2 * TEXT_MARGIN;
		}
	}

	rcText.left += sizeImage.cx + 2 * TEXT_MARGIN;

	UINT uiDTFlags = DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS;
	
	if (IsMode2003 ())
	{
		uiDTFlags |= DT_LEFT;
	}
	else
	{
		switch (m_nPageButtonTextAlign)
		{
		case TA_LEFT:
			uiDTFlags |= DT_LEFT;
			break;

		case TA_CENTER:
		default:
			uiDTFlags |= DT_CENTER;
			break;

		case TA_RIGHT:
			uiDTFlags |= DT_RIGHT;
			break;
		}
	}

	dc.DrawText (pTab->m_strText, rcText, uiDTFlags);

	if (pTab->m_bIsDetachable && !IsMode2003 ())
	{
		//--------------
		// Draw gripper:
		//--------------
		CRect rectGripper = pTab->m_rect;
		rectGripper.left = rcText.right;
		rectGripper.DeflateRect (GRIPPER_MARGIN, 2);

		CBCGPBaseControlBar bar;
		CBCGPVisualManager::GetInstance ()->OnDrawBarGripper (&dc,
			rectGripper, TRUE, &bar);
	}
}
//**************************************************************************************
void CBCGPOutlookWnd::GetTabArea (CRect& rectTabAreaTop, CRect& rectTabAreaBottom) const
{
	rectTabAreaTop.SetRectEmpty ();
	rectTabAreaBottom.SetRectEmpty ();

	for (int i = 0; i < m_iTabsNum; i ++)
	{
		CBCGPTabInfo* pTab = (CBCGPTabInfo*) m_arTabs [i];
		ASSERT_VALID (pTab);

		CRect rectBtn = pTab->m_rect;

		//--------------------------------
		// The first tab is always at top:
		//--------------------------------
		if (i == 0)
		{
			rectTabAreaTop = pTab->m_rect;
			continue;
		}

		if (rectTabAreaTop.bottom == pTab->m_rect.top)
		{
			rectTabAreaTop.bottom += pTab->m_rect.Height ();
		}
		else if (rectTabAreaBottom.IsRectEmpty ())
		{
			rectTabAreaBottom = pTab->m_rect;
		}
		else
		{
			rectTabAreaBottom.bottom += pTab->m_rect.Height ();
		}
	}
	ClientToScreen (rectTabAreaTop);
	ClientToScreen (rectTabAreaBottom);
}
//*************************************************************************************
void CBCGPOutlookWnd::SetBorderSize (int nBorderSize)
{
	ASSERT_VALID (this);
	
	m_nBorderSize = nBorderSize;
	RecalcLayout ();
}
//*************************************************************************************
BOOL CBCGPOutlookWnd::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	CPoint ptCursor;
	::GetCursorPos (&ptCursor);
	ScreenToClient (&ptCursor);

	if (m_rectSplitter.PtInRect (ptCursor))
	{
		SetCursor (AfxGetApp ()->LoadStandardCursor (IDC_SIZENS));
		return TRUE;
	}

	if (GetTabFromPoint (ptCursor) >= 0)
	{
		if (GetTabsNum() > 1 || !IsSingleTabStatic())
		{
			::SetCursor (globalData.GetHandCursor ());
			return TRUE;
		}
	}
	
	return CBCGPBaseTabWnd::OnSetCursor(pWnd, nHitTest, message);
}
//***************************************************************************************
int CBCGPOutlookWnd::GetTabNumberToDetach (int nTabNum) const
{
	return (nTabNum == -1 ? m_iPressed : nTabNum);
}
//**************************************************************************************
BOOL CBCGPOutlookWnd::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}
//**************************************************************************************
DROPEFFECT CBCGPOutlookWnd::OnDragEnter(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point) 
{
	return OnDragOver(pDataObject, dwKeyState, point);
}
//****************************************************************************************
DROPEFFECT CBCGPOutlookWnd::OnDragOver(COleDataObject* pDataObject, DWORD dwKeyState, 
								   CPoint point) 
{
	CBCGPToolbarButton* pButton = CBCGPToolbarButton::CreateFromOleData (pDataObject);
	if (pButton == NULL)
	{
		return DROPEFFECT_NONE;
	}
	
	if (!pButton->IsKindOf (RUNTIME_CLASS (CBCGPOutlookButton)))
	{
		delete pButton;
		return DROPEFFECT_NONE;
	}

	delete pButton;

	int nTab = GetTabFromPoint (point);
	if (nTab < 0)
	{
		return DROPEFFECT_NONE;
	}

	SetActiveTab (nTab);
	BOOL bCopy = (dwKeyState & MK_CONTROL);

	return (bCopy) ? DROPEFFECT_COPY : DROPEFFECT_MOVE;
}
//****************************************************************************************
void CBCGPOutlookWnd::EnableAnimation (BOOL bEnable)
{
	m_bEnableAnimation = bEnable;
}
//***************************************************************************************
void CBCGPOutlookWnd::EnableScrollButtons (BOOL bEnable/* = TRUE*/,
										   BOOL bIsUp/* = TRUE*/, BOOL bIsDown/* = TRUE*/)
{
	ASSERT_VALID (this);

	if (IsMode2003 ())
	{
		bEnable = FALSE;
	}

	BOOL bRecalcLayout = m_bScrollButtons != bEnable;

	m_bScrollButtons = bEnable;

	if (m_bScrollButtons)
	{
		m_btnUp.ShowWindow (SW_SHOWNOACTIVATE);
		m_btnUp.EnableWindow (bIsUp);
		m_btnUp.SetStdImage (CBCGPMenuImages::IdArowUpLarge,
			bIsUp ? CBCGPMenuImages::ImageBlack : CBCGPMenuImages::ImageGray);

		m_btnDown.ShowWindow (SW_SHOWNOACTIVATE);
		m_btnDown.EnableWindow (bIsDown);
		m_btnDown.SetStdImage (CBCGPMenuImages::IdArowDownLarge,
			bIsDown ? CBCGPMenuImages::ImageBlack : CBCGPMenuImages::ImageGray);
	}
	else
	{
		m_btnUp.ShowWindow (SW_HIDE);
		m_btnDown.ShowWindow (SW_HIDE);
	}

	m_btnUp.RedrawWindow ();
	m_btnDown.RedrawWindow ();

	if (bRecalcLayout)
	{
		RecalcLayout ();
	}
}
//****************************************************************************************
int CBCGPOutlookWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CBCGPBaseTabWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	//-----------------------
	// Create scroll buttons:
	//-----------------------
	CRect rectDummy (0, 0, 0, 0);

	m_btnUp.Create (_T(""), WS_CHILD | BS_PUSHBUTTON, rectDummy,
					this, (UINT)-1);
	m_btnUp.SetStdImage (CBCGPMenuImages::IdArowUpLarge);
	m_btnUp.SetAutorepeatMode (100);

	m_btnDown.Create (_T(""), WS_CHILD | BS_PUSHBUTTON, rectDummy,
					this, (UINT)-1);
	m_btnDown.SetStdImage (CBCGPMenuImages::IdArowDownLarge);
	m_btnDown.SetAutorepeatMode (100);

	m_wndToolBar.m_bLargeIconsAreEnbaled = FALSE;

	m_wndToolBar.CreateEx (this, TBSTYLE_FLAT,
		dwDefaultToolbarStyle, CRect(0, 0, 0, 0));
	m_wndToolBar.SetOwner (this);
	m_wndToolBar.SetRouteCommandsViaFrame (FALSE);

	m_wndToolBar.SetBarStyle (
		m_wndToolBar.GetBarStyle () & 
			~(CBRS_GRIPPER | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));

	m_wndToolBar.SetHotBorder (FALSE);

	return 0;
}
//***************************************************************************************
BOOL CBCGPOutlookWnd::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	HWND hwnd = (HWND) lParam;

	CBCGPOutlookBar* pOutlookBar = 
		DYNAMIC_DOWNCAST (CBCGPOutlookBar, GetParent ());

	if (pOutlookBar != NULL)
	{
		if (m_btnUp.GetSafeHwnd () == hwnd)
		{
			pOutlookBar->OnScroll (FALSE);

			if (!m_btnUp.IsWindowEnabled ())
			{
				SetFocus ();
			}

			return TRUE;
		}
		
		if (m_btnDown.GetSafeHwnd () == hwnd)
		{
			pOutlookBar->OnScroll (TRUE);

			if (!m_btnDown.IsWindowEnabled ())
			{
				SetFocus ();
			}

			return TRUE;
		}
	}
	
	return CBCGPBaseTabWnd::OnCommand(wParam, lParam);
}
//*********************************************************************************
void CBCGPOutlookWnd::SetPageButtonTextAlign (UINT uiAlign, BOOL bRedraw/* = TRUE*/)
{
	m_nPageButtonTextAlign = uiAlign;

	if (bRedraw && GetSafeHwnd () != NULL)
	{
		RedrawWindow ();
	}
}
//********************************************************************************
BOOL CBCGPOutlookWnd::IsMode2003 () const
{
	ASSERT_VALID (this);

	CBCGPOutlookBar* pOutlookBar = 
		DYNAMIC_DOWNCAST (CBCGPOutlookBar, GetParent ());

	return pOutlookBar != NULL && pOutlookBar->IsMode2003 ();
}
//*******************************************************************************
BOOL CBCGPOutlookWnd::IsTabDetachable (int iTab) const
{
	ASSERT_VALID (this);

	if (IsMode2003 ())
	{
		return FALSE;
	}

	return CBCGPBaseTabWnd::IsTabDetachable (iTab);
}
//********************************************************************************
void CBCGPOutlookWnd::OnLButtonDown(UINT nFlags, CPoint point) 
{
	if (m_rectSplitter.PtInRect (point))
	{
		m_bIsTracking = TRUE;
		SetCapture ();
		return;
	}
	
	CBCGPBaseTabWnd::OnLButtonDown(nFlags, point);
}
//*******************************************************************************
void CBCGPOutlookWnd::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if (m_bIsTracking)
	{
		ReleaseCapture ();
		m_bIsTracking = FALSE;
	}
	
	CBCGPBaseTabWnd::OnLButtonUp(nFlags, point);
}
//*********************************************************************************
void CBCGPOutlookWnd::OnMouseMove(UINT nFlags, CPoint point) 
{
	if (!m_bIsTracking)
	{
		CBCGPBaseTabWnd::OnMouseMove(nFlags, point);
		return;
	}

	if (m_nTabsHeight == 0 || m_nVisiblePageButtons == -1)
	{
		return;
	}

	int nDelta = (m_rectSplitter.top - point.y) / m_nTabsHeight;
	if (nDelta == 0)
	{
		return;
	}

	int nVisiblePageButtonsPrev = m_nVisiblePageButtons;

	m_nVisiblePageButtons += nDelta;

	m_nVisiblePageButtons = min (GetVisibleTabsNum (), 
		max (0, m_nVisiblePageButtons));

	if (nVisiblePageButtonsPrev != m_nVisiblePageButtons)
	{
		m_bDontAdjustLayout = TRUE;
		RecalcLayout ();
		m_bDontAdjustLayout = FALSE;

		point.y = m_rectSplitter.CenterPoint ().y;
		ClientToScreen (&point);

		::SetCursorPos (point.x, point.y);
	}
}
//********************************************************************************
void CBCGPOutlookWnd::OnCancelMode() 
{
	CBCGPBaseTabWnd::OnCancelMode();
	
	if (m_bIsTracking)
	{
		ReleaseCapture ();
		m_bIsTracking = FALSE;
	}
}
//*******************************************************************************
void CBCGPOutlookWnd::UseAlphaBlendIcons (BOOL bAlphaBlend/* = TRUE*/, BOOL bRebuildIcons/* = FALSE*/)
{
	if (m_bAlphaBlendIcons != bAlphaBlend)
	{
		m_bAlphaBlendIcons = bAlphaBlend;

		if (bRebuildIcons)
		{
			RebuildToolBar();
		}
	}
}
//*******************************************************************************
void CBCGPOutlookWnd::RebuildToolBar ()
{
	ASSERT_VALID (this);

	if (!IsMode2003 ())
	{
		return;
	}

	m_wndToolBar.RemoveAllButtons ();
	m_wndToolBar.m_TabButtons.RemoveAll ();

	m_wndToolBar.EnableCustomizeButton (TRUE, 0, _T(""), FALSE);

	CSize sizeImage (0, 0);
		
	if (m_imagesToolbar.GetSafeHandle () != NULL)
	{
		sizeImage = m_sizeToolbarImage;
	}
	else
	{
		sizeImage = GetImageSize ();
	}

	if (sizeImage == CSize (0, 0))
	{
		sizeImage = CSize (16, 16);
	}

	CSize sizeButton = sizeImage + CSize (6, 6 + 2 * nToolbarMarginHeight);
	m_wndToolBar.SetLockedSizes (sizeButton, sizeImage);
	m_wndToolBar.m_ImagesLocked.Clear ();
	m_wndToolBar.m_ImagesLocked.SetImageSize (sizeImage);

	if (m_wndToolBar.m_pCustomizeBtn != NULL)
	{
		CBCGPOutlookCustomizeButton customizeButton;
		customizeButton.CopyFrom (*m_wndToolBar.m_pCustomizeBtn);

		customizeButton.SetPipeStyle (FALSE);
		customizeButton.SetMenuRightAlign (FALSE);
		customizeButton.m_bShowAtRightSide = TRUE;
		customizeButton.SetMessageWnd (this);

		m_wndToolBar.m_Buttons.RemoveHead ();
		delete m_wndToolBar.m_pCustomizeBtn;
		m_wndToolBar.m_pCustomizeBtn = NULL;

		m_wndToolBar.InsertButton (customizeButton);

		m_wndToolBar.m_pCustomizeBtn = (CCustomizeButton*) m_wndToolBar.m_Buttons.GetHead ();
	}

	int nButtonNum = 0;
	for (int i = 0; i < m_iTabsNum; i ++)
	{
		CBCGPTabInfo* pTab = (CBCGPTabInfo*) m_arTabs [i];
		ASSERT_VALID (pTab);

		if (pTab->m_bVisible && pTab->m_rect.IsRectEmpty ())
		{
			CBCGPToolbarButton button (idToolbarCommandID + nButtonNum, nButtonNum, pTab->m_strText);
			m_wndToolBar.InsertButton (button);

			m_wndToolBar.m_TabButtons.SetAt (nButtonNum, i);

			HICON hIcon = NULL;
			UINT uiIcon = GetTabIcon (i);

			BOOL bDestroyIcon = FALSE;
			if (m_imagesToolbar.GetSafeHandle () != NULL)
			{
				hIcon = m_imagesToolbar.ExtractIcon (uiIcon);
				bDestroyIcon = hIcon != NULL;
			}
			else
			{
				hIcon = GetTabHicon (i);

				if (hIcon == NULL)
				{
					CImageList* pImageList = (CImageList*) GetImageList ();

					if (pImageList != NULL && uiIcon != (UINT)-1)
					{
						hIcon = pImageList->ExtractIcon (uiIcon);
						bDestroyIcon = hIcon != NULL;
					}
				}
			}

			m_wndToolBar.m_ImagesLocked.AddIcon (hIcon, m_bAlphaBlendIcons);
			if (bDestroyIcon && hIcon != NULL)
			{
				::DestroyIcon (hIcon);
			}

			nButtonNum++;
		}
	}

	m_wndToolBar.AdjustLocations ();
	m_wndToolBar.RedrawWindow ();
}
//*******************************************************************************
void CBCGPOutlookWnd::OnShowMorePageButtons ()
{
	m_nVisiblePageButtons++;

	m_bDontAdjustLayout = TRUE;
	RecalcLayout ();
	m_bDontAdjustLayout = FALSE;
}
//*******************************************************************************
void CBCGPOutlookWnd::OnShowFewerPageButtons ()
{
	m_nVisiblePageButtons--;

	m_bDontAdjustLayout = TRUE;
	RecalcLayout ();
	m_bDontAdjustLayout = FALSE;
}
//*******************************************************************************
BOOL CBCGPOutlookWnd::CanShowMorePageButtons () const
{
	return m_nVisiblePageButtons < m_nMaxVisiblePageButtons;
}
//*******************************************************************************
BOOL CBCGPOutlookWnd::CanShowFewerPageButtons () const
{
	return m_nVisiblePageButtons > 0;
}
//*******************************************************************************
void CBCGPOutlookWnd::OnChangeTabs ()
{
	// Will be recalculated in the next RecalcLayout ()
	m_nVisiblePageButtons = -1;
}
//***************************************************************************************
BOOL CBCGPOutlookWnd::SetToolbarImageList (UINT uiID, int cx, COLORREF clrTransp)
{
	if (!IsMode2003 ())
	{
		ASSERT (FALSE);
		return FALSE;
	}

	CBitmap bmp;
	if (!bmp.LoadBitmap (uiID))
	{
		TRACE(_T("CBCGPOutlookWnd::SetToolbarImageList Can't load bitmap: %x\n"), uiID);
		return FALSE;
	}

	if (m_imagesToolbar.GetSafeHandle () != NULL)
	{
		m_imagesToolbar.DeleteImageList ();
	}

	BITMAP bmpObj;
	bmp.GetBitmap (&bmpObj);

	UINT nFlags = (clrTransp == (COLORREF) -1) ? 0 : ILC_MASK;

	switch (bmpObj.bmBitsPixel)
	{
	case 4:
	default:
		nFlags |= ILC_COLOR4;
		break;

	case 8:
		nFlags |= ILC_COLOR8;
		break;

	case 16:
		nFlags |= ILC_COLOR16;
		break;

	case 24:
		nFlags |= ILC_COLOR24;
		break;

	case 32:
		nFlags |= ILC_COLOR32;
		break;
	}

	m_imagesToolbar.Create (cx, bmpObj.bmHeight, nFlags, 0, 0);
	m_imagesToolbar.Add (&bmp, clrTransp);

	m_sizeToolbarImage = CSize (cx, bmpObj.bmHeight);

	RecalcLayout ();
	return TRUE;
}
//*******************************************************************************
void CBCGPOutlookWnd::OnToolbarCommand (UINT id)
{
	switch (id)
	{
	case idShowMoreButtons:
		OnShowMorePageButtons ();
		break;

	case idShowFewerButtons:
		OnShowFewerPageButtons ();
		break;

	case idNavigationPaneOptions:
		OnShowOptions ();
		break;
	}
}
//********************************************************************************
void CBCGPOutlookWnd::OnUpdateToolbarCommand (CCmdUI* pCmdUI)
{
	switch (pCmdUI->m_nID)
	{
	case idShowMoreButtons:
		pCmdUI->Enable (CanShowMorePageButtons ());
		break;

	case idShowFewerButtons:
		pCmdUI->Enable (CanShowFewerPageButtons ());
		break;
	}
}
//**************************************************************************************
LRESULT CBCGPOutlookWnd::OnPrintClient(WPARAM wp, LPARAM lp)
{
	if ((lp & PRF_CLIENT) == PRF_CLIENT)
	{
		CDC* pDC = CDC::FromHandle((HDC) wp);
		ASSERT_VALID(pDC);

		m_bIsPrintingClient = TRUE;
		OnDraw(pDC);
		m_bIsPrintingClient = FALSE;
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPOutlookOptionsDlg dialog

class CBCGPOutlookOptionsDlg : public CBCGPDialog
{
// Construction
public:
	CBCGPOutlookOptionsDlg(CBCGPOutlookWnd& parentBar);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CBCGPOutlookOptionsDlg)
	enum { IDD = IDD_BCGBARRES_OUTLOOKBAR_OPTIONS };
	CBCGPExCheckList	m_wndList;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPOutlookOptionsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CBCGPOutlookOptionsDlg)
	afx_msg void OnSelchange();
	afx_msg void OnDblclkList();
	afx_msg void OnMoveDown();
	afx_msg void OnMoveUp();
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnReset();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CBCGPOutlookWnd& m_parentBar;

	void MoveItem (BOOL bMoveUp);
};

CBCGPOutlookOptionsDlg::CBCGPOutlookOptionsDlg(CBCGPOutlookWnd& parentBar)
	: CBCGPDialog(CBCGPOutlookOptionsDlg::IDD, &parentBar),
	m_parentBar (parentBar)
{
	m_bIsLocal = TRUE;

	//{{AFX_DATA_INIT(CBCGPOutlookOptionsDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	EnableVisualManagerStyle (globalData.m_bUseVisualManagerInBuiltInDialogs, TRUE);
}
//*******************************************************************************
void CBCGPOutlookOptionsDlg::DoDataExchange(CDataExchange* pDX)
{
	CBCGPDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBCGPOutlookOptionsDlg)
	DDX_Control(pDX, IDC_BCGBARRES_LIST, m_wndList);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CBCGPOutlookOptionsDlg, CBCGPDialog)
	//{{AFX_MSG_MAP(CBCGPOutlookOptionsDlg)
	ON_LBN_SELCHANGE(IDC_BCGBARRES_LIST, OnSelchange)
	ON_LBN_DBLCLK(IDC_BCGBARRES_LIST, OnDblclkList)
	ON_BN_CLICKED(IDC_BCGBARRES_MOVEDOWN, OnMoveDown)
	ON_BN_CLICKED(IDC_BCGBARRES_MOVEUP, OnMoveUp)
	ON_BN_CLICKED(IDC_BCGBARRES_RESET, OnReset)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGPOutlookOptionsDlg message handlers

void CBCGPOutlookOptionsDlg::OnSelchange() 
{
	CWnd* pButtonUp = GetDlgItem (IDC_BCGBARRES_MOVEUP);
	CWnd* pButtonDn = GetDlgItem (IDC_BCGBARRES_MOVEDOWN);

	if (pButtonUp->GetSafeHwnd () != NULL)
	{
		pButtonUp->EnableWindow (m_wndList.GetCurSel () > 0);
	}

	if (pButtonDn->GetSafeHwnd () != NULL)
	{
		pButtonDn->EnableWindow (m_wndList.GetCurSel () < m_wndList.GetCount () - 1);
	}
}
//**************************************************************************************
void CBCGPOutlookOptionsDlg::OnDblclkList() 
{
	int nSel = m_wndList.GetCurSel ();
	if (nSel >= 0)
	{
		m_wndList.SetCheck (nSel, !m_wndList.GetCheck (nSel));
	}
}
//**************************************************************************************
void CBCGPOutlookOptionsDlg::OnMoveDown() 
{
	MoveItem (FALSE);
}
//**************************************************************************************
void CBCGPOutlookOptionsDlg::OnMoveUp() 
{
	MoveItem (TRUE);
}
//**************************************************************************************
BOOL CBCGPOutlookOptionsDlg::OnInitDialog() 
{
	CBCGPDialog::OnInitDialog();
	
	if (AfxGetMainWnd () != NULL && 
		(AfxGetMainWnd ()->GetExStyle () & WS_EX_LAYOUTRTL))
	{
		ModifyStyleEx (0, WS_EX_LAYOUTRTL);
	}

	for (int i = 0; i < m_parentBar.m_iTabsNum; i ++)
	{
		CString str;
		m_parentBar.GetTabLabel (i, str);

		int nIndex = m_wndList.AddString (str);

		m_wndList.SetItemData (nIndex, (DWORD_PTR) i);
		m_wndList.SetCheck (nIndex, m_parentBar.IsTabVisible (i));
	}

	m_wndList.SetCurSel (0);
	OnSelchange();
	
	CBCGPOutlookBar* pOutlookBar = 
		DYNAMIC_DOWNCAST (CBCGPOutlookBar, m_parentBar.GetParent ());
	if (pOutlookBar == NULL)
	{
		CWnd* pButtonReset = GetDlgItem (IDC_BCGBARRES_RESET);
		if (pButtonReset->GetSafeHwnd () != NULL)
		{
			pButtonReset->EnableWindow (FALSE);
			pButtonReset->ShowWindow (SW_HIDE);
		}
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
//**************************************************************************************
void CBCGPOutlookOptionsDlg::OnOK() 
{
	CArray<int, int> arTabsOrder;

	for (int nIndex = 0; nIndex < m_wndList.GetCount (); nIndex++)
	{
		int i = (int) m_wndList.GetItemData (nIndex);

		BOOL bVisible = m_wndList.GetCheck (nIndex);

		if (bVisible != m_parentBar.IsTabVisible (i))
		{
			m_parentBar.ShowTab (i, bVisible, FALSE);
		}

		arTabsOrder.Add (i);
	}

	m_parentBar.SetTabsOrder (arTabsOrder);

	CBCGPDialog::OnOK();
}
//**************************************************************************************
void CBCGPOutlookOptionsDlg::OnReset() 
{
	CBCGPOutlookBar* pOutlookBar = 
		DYNAMIC_DOWNCAST (CBCGPOutlookBar, m_parentBar.GetParent ());
	if (pOutlookBar == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	CArray<int, int> arTabsOrder;
	int i = 0;

	for (i = 0; i < pOutlookBar->GetDefaultTabsOrder ().GetSize (); i++)
	{
		int iTabID = pOutlookBar->GetDefaultTabsOrder () [i];
		int iTab = m_parentBar.GetTabByID (iTabID);
		
		if (iTab < 0)
		{
			ASSERT (FALSE);
			return;
		}

		arTabsOrder.Add (iTab);
	}

	m_wndList.ResetContent ();

	for (i = 0; i < arTabsOrder.GetSize (); i ++)
	{
		int iTabNum = arTabsOrder [i];

		CString str;
		m_parentBar.GetTabLabel (iTabNum, str);

		int nIndex = m_wndList.AddString (str);

		m_wndList.SetItemData (nIndex, (DWORD_PTR) iTabNum);
		m_wndList.SetCheck (nIndex, TRUE);
	}

	m_wndList.SetCurSel (0);
	OnSelchange();
}
//**************************************************************************************
void CBCGPOutlookOptionsDlg::MoveItem (BOOL bMoveUp)
{
	int nSel = m_wndList.GetCurSel ();

	CString str;
	m_wndList.GetText (nSel, str);
	DWORD_PTR dwData = m_wndList.GetItemData (nSel);
	BOOL bCheck = m_wndList.GetCheck (nSel);

	m_wndList.DeleteString (nSel);

	int nNewIndex = bMoveUp ? nSel - 1 : nSel + 1;

	int nIndex = m_wndList.InsertString (nNewIndex, str);

	m_wndList.SetItemData (nIndex, dwData);
	m_wndList.SetCheck (nIndex, bCheck);

	m_wndList.SetCurSel (nIndex);
	OnSelchange();
}
//**************************************************************************************
void CBCGPOutlookWnd::OnShowOptions ()
{
	CBCGPOutlookOptionsDlg dlg (*this);
	if (dlg.DoModal () == IDOK)
	{
		m_bDontAdjustLayout = TRUE;
		RecalcLayout ();
		m_bDontAdjustLayout = FALSE;
	}
}
