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
// BCGPTasksPane.cpp : implementation file
//

#include "stdafx.h"
#include "bcgcbpro.h"

#ifndef BCGP_EXCLUDE_TASK_PANE

#include "BCGPLocalResource.h"
#include "bcgprores.h"
#include "BCGPVisualManager.h"
#include "BCGPContextMenuManager.h"
#include "BCGPWorkspace.h"
#include "BCGPTasksPane.h"
#include "BCGPTaskPaneMiniFrameWnd.h"
#include "BCGPMultiMiniFrameWnd.h"
#include "RegPath.h"
#include "BCGPRegistry.h"
#include "BCGPToolbarMenuButton.h"
#include "BCGPPopupMenu.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CBCGPWorkspace* g_pWorkspace;

static const CString strTasksPaneProfile	= _T("BCGPTasksPanes");
#define REG_SECTION_FMT						_T("%sBCGPTasksPane-%d")
#define REG_SECTION_FMT_EX					_T("%sBCGPTasksPane-%d%x")
#define REG_ENTRY_SETTINGS					_T("Settings")

#define ID_SCROLL_VERT	1

static const int iBorderSize = 1;
static const int iNavToolbarId = 1;
static const int iAnimTimerId =		ID_CHECK_AUTO_HIDE_CONDITION + 1;
static const int iScrollTimerId =	ID_CHECK_AUTO_HIDE_CONDITION + 2;

static inline BOOL IsSystemCommand (UINT uiCmd)
{
	return (uiCmd >= 0xF000 && uiCmd < 0xF1F0);
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPTasksPane additional classes: CBCGPTasksPaneNavigateButton

class CBCGPTasksPaneNavigateButton : public CBCGPToolbarButton
{
	friend class CBCGPTasksPane;
	DECLARE_SERIAL(CBCGPTasksPaneNavigateButton)

protected:
    CBCGPTasksPaneNavigateButton(int iImage = -1)
		: CBCGPToolbarButton ()
	{
		m_iImage = iImage;
		m_bLocked = TRUE;
	}
	CBCGPTasksPaneNavigateButton (UINT uiID, int iImage, LPCTSTR lpszText = NULL)
		: CBCGPToolbarButton (uiID, iImage, lpszText, FALSE, TRUE)
	{
	}
};

IMPLEMENT_SERIAL(CBCGPTasksPaneNavigateButton, CBCGPToolbarButton, 1)

/////////////////////////////////////////////////////////////////////////////
// CBCGPTasksPaneHistoryButton

class CBCGPTasksPaneHistoryButton : public CBCGPToolbarMenuButton
{
	friend class CBCGPTasksPane;
	DECLARE_SERIAL(CBCGPTasksPaneHistoryButton)

public:
    CBCGPTasksPaneHistoryButton(int iImage = -1)
		: CBCGPToolbarMenuButton ()
	{
		m_iImage = iImage;
		m_bLocked = TRUE;

		m_pParentBar = NULL;
	}
	//***********************************************************************
	CBCGPTasksPaneHistoryButton (UINT uiID, int iImage, 
							LPCTSTR lpszText = NULL, BOOL bUserButton = FALSE)
		: CBCGPToolbarMenuButton ()
	{
		m_nID = uiID;
		m_bUserButton = bUserButton;

		SetImage (iImage);
		m_strText = (lpszText == NULL) ? _T("") : lpszText;

		CMenu menu;
		menu.CreatePopupMenu ();
		CreateFromMenu (menu.GetSafeHmenu ());

		m_pParentBar = NULL;
	}
	//***********************************************************************
	virtual void OnChangeParentWnd (CWnd* pWndParent)
	{
		CBCGPToolbarMenuButton::OnChangeParentWnd (pWndParent);
		m_pParentBar = DYNAMIC_DOWNCAST (CBCGPTasksPane, pWndParent);
	}
	//***********************************************************************
	void UpdateMenu ()
	{
		if (m_pParentBar == NULL)
		{
			return;
		}

		if (m_nID == ID_BCGBARRES_TASKPANE_BACK)
		{
			m_pParentBar->GetPreviousPages (m_lstPages);
		}
		else if (m_nID == ID_BCGBARRES_TASKPANE_FORWARD)
		{
			m_pParentBar->GetNextPages (m_lstPages);
		}

		CMenu menu;
		menu.CreatePopupMenu ();

		for (POSITION pos = m_lstPages.GetHeadPosition (); pos != NULL; )
		{
			CString& strPageName = m_lstPages.GetNext (pos);
			menu.AppendMenu (MF_STRING, m_nID, strPageName);
		}

		CreateFromMenu (menu.GetSafeHmenu ());
	}

// data:
	CBCGPTasksPane*		m_pParentBar;
	CStringList			m_lstPages;	// pages history
};

IMPLEMENT_SERIAL(CBCGPTasksPaneHistoryButton, CBCGPToolbarMenuButton, 1)

/////////////////////////////////////////////////////////////////////////////
// CBCGPTasksPaneMenuButton

class CBCGPTasksPaneMenuButton : public CBCGPToolbarMenuButton
{
	friend class CBCGPTasksPane;
	DECLARE_SERIAL(CBCGPTasksPaneMenuButton)

public:
	CBCGPTasksPaneMenuButton(HMENU hMenu = NULL) :
		CBCGPToolbarMenuButton ((UINT)-1, hMenu, -1)
	{
		m_pParentBar = NULL;
	}
	//***********************************************************************
	virtual HMENU CreateMenu () const
	{
		if (m_pParentBar == NULL)
		{
			return NULL;
		}

		ASSERT_VALID (m_pParentBar);

		return m_pParentBar->CreateMenu ();
	}
	//***********************************************************************
	virtual CBCGPPopupMenu* CreatePopupMenu ()
	{
		CBCGPPopupMenu* pMenu = CBCGPToolbarMenuButton::CreatePopupMenu ();
		if (pMenu == NULL)
		{
			ASSERT (FALSE);
			return NULL;
		}

		pMenu->SetRightAlign (TRUE);
		return pMenu;
	}
	//***********************************************************************
	virtual void OnChangeParentWnd (CWnd* pWndParent)
	{
		CBCGPToolbarMenuButton::OnChangeParentWnd (pWndParent);
		m_pParentBar = DYNAMIC_DOWNCAST (CBCGPTasksPane, pWndParent);
	}
	
// data:
	CBCGPTasksPane*	m_pParentBar;
};

IMPLEMENT_SERIAL(CBCGPTasksPaneMenuButton, CBCGPToolbarMenuButton, 1)


/////////////////////////////////////////////////////////////////////////////
// CBCGPTasksPaneToolBar

IMPLEMENT_SERIAL(CBCGPTasksPaneToolBar, CBCGPToolBar, 1)

BEGIN_MESSAGE_MAP(CBCGPTasksPaneToolBar, CBCGPToolBar)
	//{{AFX_MSG_MAP(CBCGPTasksPaneToolBar)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_IDLEUPDATECMDUI, OnIdleUpdateCmdUI)
END_MESSAGE_MAP()

LRESULT CBCGPTasksPaneToolBar::OnIdleUpdateCmdUI(WPARAM wParam, LPARAM)
{
	// the style must be visible and if it is docked
	// the dockbar style must also be visible
	if (GetStyle() & WS_VISIBLE)
	{
		OnUpdateCmdUI((CFrameWnd*) GetOwner (), (BOOL)wParam);
	}

	return 0L;
}
//******************************************************************************************
void CBCGPTasksPaneToolBar::AdjustLocations ()
{
	ASSERT_VALID(this);

	if (GetSafeHwnd () == NULL || !::IsWindow (m_hWnd))
	{
		return;
	}

	CBCGPToolBar::AdjustLocations ();

	//----------------------------------
	// Get menu button and close button:
	//----------------------------------
	CBCGPTasksPaneNavigateButton* pCloseBtn = NULL;
	CBCGPTasksPaneMenuButton* pMenuBtn = NULL;

	for (POSITION pos = m_Buttons.GetHeadPosition (); pos != NULL; )
	{
		CBCGPToolbarButton* pButton = (CBCGPToolbarButton*) m_Buttons.GetNext (pos);
		ASSERT_VALID (pButton);
		
		if (pButton->m_nStyle & TBBS_SEPARATOR)
		{
		}
		else
		{
			if (pButton->IsKindOf (RUNTIME_CLASS (CBCGPTasksPaneNavigateButton)))
			{
				if (pButton->GetImage () == 3)
				{
					pCloseBtn = DYNAMIC_DOWNCAST (CBCGPTasksPaneNavigateButton, pButton);
					ASSERT_VALID (pCloseBtn);
				}

			}
			else if (pButton->IsKindOf (RUNTIME_CLASS (CBCGPTasksPaneMenuButton)))
			{
				pMenuBtn = DYNAMIC_DOWNCAST (CBCGPTasksPaneMenuButton, pButton);
				ASSERT_VALID (pMenuBtn);
			}
		}
	}	

	CRect rectClient;
	GetClientRect (&rectClient);
	
	BOOL bShowCloseButton = FALSE;
	BOOL bStrechMenuButton = TRUE;

	if (pMenuBtn != NULL)
	{
		CRect rectMenuBtn = pMenuBtn->Rect ();
		int nMin = rectMenuBtn.left + rectMenuBtn.Height () * 3;
		int nMax = rectClient.right - 1;
		if (pCloseBtn != NULL && bShowCloseButton)
		{
			nMax = rectClient.right - 1 - rectMenuBtn.Height ();
		}

		// -------------------
		// Adjust menu button:
		// -------------------
		if (bStrechMenuButton)
		{
			rectMenuBtn.right = max (nMin, nMax);
			pMenuBtn->SetRect (rectMenuBtn);
		}

		// --------------------
		// Adjust close button:
		// --------------------
		if (pCloseBtn != NULL && bShowCloseButton)
		{
			CRect rectCloseBtn = pMenuBtn->Rect ();
			rectCloseBtn.left = rectMenuBtn.right;
			rectCloseBtn.right = rectMenuBtn.right + rectCloseBtn.Height ();

			if (rectCloseBtn.right < rectClient.right - 1)
			{
				rectCloseBtn.OffsetRect (rectClient.right - 1 - rectCloseBtn.right, 0);
			}

			pCloseBtn->SetRect (rectCloseBtn);
			pCloseBtn->Show (TRUE);
		}
		else if (pCloseBtn != NULL)
		{
			pCloseBtn->Show (FALSE);
		}
	}
	UpdateTooltips ();
}
//******************************************************************************************
void CBCGPTasksPaneToolBar::UpdateMenuButtonText (const CString& str)
{
	for (POSITION pos = m_Buttons.GetHeadPosition (); pos != NULL; )
	{
		CBCGPToolbarButton* pButton = (CBCGPToolbarButton*) m_Buttons.GetNext (pos);
		ASSERT_VALID (pButton);

		CBCGPTasksPaneMenuButton* pMenuBtn = DYNAMIC_DOWNCAST (CBCGPTasksPaneMenuButton, pButton);
		if (pMenuBtn != NULL)
		{
			ASSERT_VALID (pMenuBtn);

			pMenuBtn->m_strText = str;
		}
	}
}
//******************************************************************************************
void CBCGPTasksPaneToolBar::UpdateButtons ()
{
	for (POSITION pos = m_Buttons.GetHeadPosition (); pos != NULL; )
	{
		CBCGPToolbarButton* pButton = (CBCGPToolbarButton*) m_Buttons.GetNext (pos);
		ASSERT_VALID (pButton);

		CBCGPTasksPaneHistoryButton* pHistoryBtn = DYNAMIC_DOWNCAST (CBCGPTasksPaneHistoryButton, pButton);
		if (pHistoryBtn != NULL)
		{
			pHistoryBtn->UpdateMenu ();
		}
	}
}
//******************************************************************************************
BOOL CBCGPTasksPaneToolBar::OnUserToolTip (CBCGPToolbarButton* pButton, CString& strTTText) const
{
	ASSERT_VALID (pButton);

	if (pButton->IsKindOf (RUNTIME_CLASS (CBCGPTasksPaneMenuButton)))
	{
		CBCGPLocalResource locaRes;
		strTTText.LoadString (ID_BCGBARRES_TASKPANE_OTHER);
		return TRUE;
	}

	CBCGPTasksPaneNavigateButton* pNavButton = DYNAMIC_DOWNCAST (CBCGPTasksPaneNavigateButton, pButton);
	if (pNavButton != NULL)
	{
		ASSERT_VALID (pNavButton);
		strTTText = pNavButton->m_strText;
		return TRUE;
	}

	CBCGPTasksPaneHistoryButton* pHisButton = DYNAMIC_DOWNCAST (CBCGPTasksPaneHistoryButton, pButton);
	if (pHisButton != NULL)
	{
		ASSERT_VALID (pHisButton);
		strTTText = pHisButton->m_strText;
		return TRUE;
	}
		
	return CBCGPToolBar::OnUserToolTip (pButton, strTTText);
}
//********************************************************************************
void CBCGPTasksPaneToolBar::AdjustLayout ()
{
	CBCGPToolBar::AdjustLayout ();

	CBCGPTasksPane* pTaskPane = DYNAMIC_DOWNCAST (CBCGPTasksPane, GetParent ());
	if (pTaskPane != NULL)
	{
		pTaskPane->RecalcLayout (TRUE);
	}
}


/////////////////////////////////////////////////////////////////////////////
// CBCGPTasksPane

clock_t CBCGPTasksPane::m_nLastAnimTime = 0;
const int CBCGPTasksPane::m_iAnimTimerDuration = 30;
const int CBCGPTasksPane::m_iScrollTimerDuration = 80;
const LONG CBCGPTasksPane::m_lAccUseCursorPosValue = MAKELONG(0, 1);
const LONG CBCGPTasksPane::m_lAccUseFocus = MAKELONG(0, 2);

IMPLEMENT_SERIAL(CBCGPTasksPane, CBCGPDockingControlBar, VERSIONABLE_SCHEMA | 1)

CBCGPTasksPane::CBCGPTasksPane(): CBCGPDockingControlBar (), m_nMaxHistory (10)
{
	m_hFont = NULL;
	m_sizeIcon = CSize (0, 0);

	m_arrHistoryStack.Add (0);
	m_iActivePage = 0;
	m_pHotTask = NULL;
	m_pClickedTask = NULL;
	m_pFocusedTask = NULL;
	m_pHotGroupCaption = NULL;
	m_pClickedGroupCaption = NULL;
	m_pFocusedGroupCaption = NULL;
	m_bCanCollapse = TRUE;
	m_nVertScrollOffset = 0;
	m_nVertScrollTotal = 0;
	m_nVertScrollPage = 0;
	m_nRowHeight = 0;

	m_nVertMargin = -1;	// default, use Visual Manager's settings
	m_nHorzMargin = -1;
	m_nGroupVertOffset = -1;
	m_nGroupCaptionHeight = -1;
	m_nGroupCaptionHorzOffset = -1;
	m_nGroupCaptionVertOffset = -1;
	m_nTaskMinHeight = -1;
	m_nTasksVertMargin = -1;
	m_nTasksHorzOffset = -1;
	m_nTasksIconHorzOffset = -1;
	m_nTasksIconVertOffset = -1;
	
	m_bOffsetCustomControls = TRUE;

	m_rectTasks.SetRectEmpty ();

	m_bUseNavigationToolbar = FALSE;
	m_bHistoryMenuButtons	= FALSE;
	m_uiToolbarBmpRes = 0;
	m_sizeToolbarImage = CSize (0, 0);
	m_sizeToolbarButton = CSize (0, 0);
	m_rectToolbar.SetRectEmpty ();

	m_bUseScrollButtons = TRUE;
	m_rectScrollUp.SetRectEmpty ();
	m_rectScrollDn.SetRectEmpty ();
	m_iScrollBtnHeight = CBCGPMenuImages::Size ().cy + 2 * iBorderSize;
	m_iScrollMode = 0;

	SetScrollBarsStyle(CBCGPScrollBar::BCGP_SBSTYLE_VISUAL_MANAGER);
	
	m_bAnimationEnabled = !globalData.bIsRemoteSession;

	m_pAnimatedGroup = NULL;
	m_sizeAnim = CSize (0, 0);

	m_bMenuBtnPressed = FALSE;

	m_bWrapTasks = FALSE;
	m_bWrapLabels = FALSE;

	m_bPrecessAccFocusEvent = FALSE;
}

CBCGPTasksPane::~CBCGPTasksPane()
{
	while (!m_lstTasksPanes.IsEmpty ())
	{
		delete m_lstTasksPanes.RemoveHead ();
	}
}

BEGIN_MESSAGE_MAP(CBCGPTasksPane, CBCGPDockingControlBar)
	//{{AFX_MSG_MAP(CBCGPTasksPane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_SETTINGCHANGE()
	ON_WM_SETCURSOR()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_CANCELMODE()
	ON_WM_LBUTTONDOWN()
	ON_WM_PAINT()
	ON_WM_VSCROLL()
	ON_WM_ERASEBKGND()
	ON_WM_DESTROY()
	ON_COMMAND(IDOK, OnOK)
	ON_COMMAND(IDCANCEL, OnCancel)
	ON_WM_MOUSEWHEEL()
	ON_WM_TIMER()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_SETFONT, OnSetFont)
	ON_MESSAGE(WM_GETFONT, OnGetFont)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXT, 0, 0xFFFF, OnNeedTipText)
	ON_COMMAND(ID_BCGBARRES_TASKPANE_BACK, OnBack)
	ON_COMMAND(ID_BCGBARRES_TASKPANE_FORWARD, OnForward)
	ON_COMMAND(ID_BCGBARRES_TASKPANE_HOME, OnHome)
	ON_COMMAND(ID_BCGBARRES_TASKPANE_CLOSE, OnClose)
	ON_COMMAND(ID_BCGBARRES_TASKPANE_OTHER, OnOther)
	ON_UPDATE_COMMAND_UI(ID_BCGBARRES_TASKPANE_BACK, OnUpdateBack)
	ON_UPDATE_COMMAND_UI(ID_BCGBARRES_TASKPANE_FORWARD, OnUpdateForward)
	ON_UPDATE_COMMAND_UI(ID_BCGBARRES_TASKPANE_CLOSE, OnUpdateClose)
	ON_MESSAGE(WM_SETTEXT, OnSetText)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGPTasksPane message handlers

int CBCGPTasksPane::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CBCGPDockingControlBar::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	//-----------------------------
	// Load Task Pane text strings:
	//-----------------------------
	CString strOther;
	CString strForward;
	CString strBack;

	{
		CBCGPLocalResource locaRes;
		strOther.LoadString (ID_BCGBARRES_TASKPANE_OTHER);
		strForward.LoadString (ID_BCGBARRES_TASKPANE_FORWARD);
		strBack.LoadString (ID_BCGBARRES_TASKPANE_BACK);

		GetWindowText (m_strCaption);

		if (m_strCaption.IsEmpty ())
		{
			m_strCaption.LoadString (IDS_BCGBARRES_TASKPANE);
		}
	}

	// --------------------------------------------
	// Register tools for caption buttons tooltips:
	// --------------------------------------------
	if (m_pToolTip->GetSafeHwnd () != NULL)
	{
		for (int i = 1; i <= 3; i ++)
		{
			CRect rectDummy;
			rectDummy.SetRectEmpty ();

			m_pToolTip->AddTool (this, LPSTR_TEXTCALLBACK, &rectDummy, CBCGSIZINGCONTROLBAR_BUTTONS_NUM + i);
		}
	}

	// ------------------------------------
	// Add default page to m_lstTasksPanes:
	// ------------------------------------
	AddPage (m_strCaption);
	SetCaption (m_strCaption);

	// ---------------
	// Create toolbar:
	// ---------------
	if (!CreateNavigationToolbar ())
	{
		TRACE(_T("Can't create taskspane toolbar bar\n"));
		return FALSE;
	}

	CreateFonts ();

	CRect rectDummy;
	rectDummy.SetRectEmpty ();
	m_wndScrollVert.Create (WS_CHILD | WS_VISIBLE | SBS_VERT, rectDummy, this, ID_SCROLL_VERT);

	if (m_bUseScrollButtons)
	{
		CBCGPGestureConfig gestureConfig;
		gestureConfig.EnablePan(TRUE, BCGP_GC_PAN_WITH_SINGLE_FINGER_VERTICALLY | BCGP_GC_PAN_WITH_GUTTER | BCGP_GC_PAN_WITH_INERTIA);
		
		bcgpGestureManager.SetGestureConfig(GetSafeHwnd(), gestureConfig);
	}

	return 0;
}
//******************************************************************************************
void CBCGPTasksPane::OnSize(UINT nType, int cx, int cy) 
{
	CBCGPDockingControlBar::OnSize(nType, cx, cy);

	AdjustScroll ();
	ReposTasks ();

	RedrawWindow (NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
}
//******************************************************************************************
int CBCGPTasksPane::ReposTasks (BOOL bCalcHeightOnly/* = FALSE*/)
{
	if (globalData.bIsRemoteSession)
	{
		m_bAnimationEnabled = FALSE;
	}

	if (GetSafeHwnd () == NULL || m_lstTaskGroups.IsEmpty ())
	{
		return 0;
	}

	if ((m_rectTasks.top < 0) || (m_rectTasks.bottom <= m_rectTasks.top) ||
		(m_rectTasks.left < 0) || (m_rectTasks.right <= m_rectTasks.left))
	{
		return 0; // m_rectTasks is not set yet
	}

	CRect rectTasks = m_rectTasks;
	rectTasks.DeflateRect (
		(GetHorzMargin() != -1 ? GetHorzMargin() :
		CBCGPVisualManager::GetInstance ()->GetTasksPaneHorzMargin()), 
		(GetVertMargin() != -1 ? GetVertMargin() :
		CBCGPVisualManager::GetInstance ()->GetTasksPaneVertMargin()));

	CClientDC dc (this);
	CFont* pFontOld = dc.SelectObject (&m_fontBold);

	TEXTMETRIC tm;
	dc.GetTextMetrics (&tm);

	m_nRowHeight = max (tm.tmHeight, m_sizeIcon.cy);
	m_nAnimGroupExtraHeight = 0;

	int y = rectTasks.top - m_nVertScrollOffset * m_nRowHeight;

	// ---------------
	// Get active page
	// ---------------
	CBCGPTasksPanePage* pActivePage = NULL;
	POSITION posPage = m_lstTasksPanes.FindIndex (m_arrHistoryStack[m_iActivePage]);
	ASSERT(posPage != NULL);

	pActivePage = (CBCGPTasksPanePage*) m_lstTasksPanes.GetAt (posPage);
	ASSERT_VALID(pActivePage);

	POSITION pos = NULL;

	// -------------
	// Recalc groups
	// -------------
	for (pos = m_lstTaskGroups.GetHeadPosition (); pos != NULL;)
	{
		CBCGPTasksGroup* pGroup = (CBCGPTasksGroup*) m_lstTaskGroups.GetNext (pos);
		ASSERT_VALID (pGroup);

		if (pGroup->m_pPage == pActivePage)
		{
		dc.SelectObject (&m_fontBold);

		// -----------------
		// Calc caption size
		// -----------------
		int nCaptionHeight = 0;
		if (!pGroup->m_strName.IsEmpty())
		{
			CFont* pFontOld = dc.SelectObject (&globalData.fontBold);
			CSize sizeText = dc.GetTextExtent (pGroup->m_strName);
			dc.SelectObject (pFontOld);
			int nVOffset = (GetGroupCaptionVertOffset() != -1 ? GetGroupCaptionVertOffset() :
				CBCGPVisualManager::GetInstance ()->GetTasksPaneGroupCaptionVertOffset());
			int nHeight = (GetGroupCaptionHeight() != -1 ? GetGroupCaptionHeight() : 
				CBCGPVisualManager::GetInstance ()->GetTasksPaneGroupCaptionHeight());

			if (IsToolBox ())
			{
				nVOffset = max (5, nVOffset);
				if (GetGroupCaptionHeight() == -1)
				{
					nHeight = 18;
				}
			}

				nCaptionHeight = max( sizeText.cy + nVOffset, nHeight );
		}
		else
		{
			nCaptionHeight = 0;
		}

		if (pGroup->m_hIcon != NULL && 
			(pGroup->m_sizeIcon.cx < rectTasks.Width () - nCaptionHeight))
		{
			if (nCaptionHeight < pGroup->m_sizeIcon.cy)
			{
				y += pGroup->m_sizeIcon.cy - nCaptionHeight;
			}
		}

		if (!bCalcHeightOnly)
		{
				pGroup->m_rect = CRect (rectTasks.left, y, 
					rectTasks.right, y + nCaptionHeight);
		}

		y += nCaptionHeight;
		int yGroup = y;

		SetFont (&dc);

		if (m_bCanCollapse && pGroup->m_bIsCollapsed && !pGroup->m_strName.IsEmpty() &&
			!(m_bAnimationEnabled && pGroup == m_pAnimatedGroup && m_sizeAnim.cy > 0 && !bCalcHeightOnly))
		{
			if (!bCalcHeightOnly)
			{
				// ---------------------
				// Recalc tasks in group
				// ---------------------
				pGroup->m_rectGroup = CRect (rectTasks.left, yGroup - 1, 
					rectTasks.right, yGroup - 1);

				for (POSITION posTask = pGroup->m_lstTasks.GetHeadPosition (); posTask != NULL;)
				{
					CBCGPTask* pTask = (CBCGPTask*) pGroup->m_lstTasks.GetNext (posTask);
					ASSERT_VALID (pTask);

					if (pTask->m_hwndTask == NULL)
					{
						pTask->m_rect.SetRectEmpty ();
					}
					else if (IsToolBox ())
					{
						CSize sizeGroupBorders = GetTasksGroupBorders ();

						CRect rectChildWnd = rectTasks;
						rectChildWnd.bottom = y + (pTask->m_bVisible ? pTask->m_nWindowHeight : 0);
						rectChildWnd.top = max (m_rectTasks.top + 1, y);
						rectChildWnd.bottom = min (m_rectTasks.bottom, rectChildWnd.bottom);

						if (m_bOffsetCustomControls)
						{
							rectChildWnd.DeflateRect ((GetTasksHorzOffset() != -1 ? GetTasksHorzOffset(): 
								CBCGPVisualManager::GetInstance ()->GetTasksPaneTaskHorzOffset()), 0);
						}
						else
						{
							rectChildWnd.DeflateRect (sizeGroupBorders.cx, 0);
						}

						pTask->m_rect = rectChildWnd;
					}
				}
			}
		}
		else	// not collapsed
		{
			// ---------------------
			// Recalc tasks in group
			// ---------------------
			BOOL bNeedHeaderOffset = TRUE;
			BOOL bNeedFooterOffset = TRUE;
			CSize sizeGroupBorders = GetTasksGroupBorders ();

			for (POSITION posTask = pGroup->m_lstTasks.GetHeadPosition (); posTask != NULL;)
			{
				CBCGPTask* pTask = (CBCGPTask*) pGroup->m_lstTasks.GetNext (posTask);
				ASSERT_VALID (pTask);

				if (pTask->m_hwndTask == NULL)
				{
						if (pTask->m_bVisible)
						{
							if (bNeedHeaderOffset)
							{
								y += (GetTasksVertMargin() != -1 ? GetTasksVertMargin () :
									CBCGPVisualManager::GetInstance ()->GetTasksPaneTasksVertMargin());

								y += (GetTasksIconVertOffset() != -1 ? GetTasksIconVertOffset() :
									CBCGPVisualManager::GetInstance ()->GetTasksPaneIconVertOffset());
							}

							int nTaskHOffset = (GetTasksHorzOffset() != -1 ? GetTasksHorzOffset() :
								CBCGPVisualManager::GetInstance ()->GetTasksPaneTaskHorzOffset());
							int nIconHOffset = (GetTasksIconHorzOffset() != -1 ? GetTasksIconHorzOffset() :
								CBCGPVisualManager::GetInstance ()->GetTasksPaneIconHorzOffset());
							int nTaskMinHeight = (GetTaskMinHeight() != -1 ? GetTaskMinHeight() : 
								CBCGPVisualManager::GetInstance ()->GetTasksPaneTaskMinHeight());

							// -----------------
							// if multiline text
							// -----------------
							if ((pTask->m_uiCommandID == 0) ? m_bWrapLabels : m_bWrapTasks)
							{
								CRect rectTask = rectTasks;
								rectTask.DeflateRect (nTaskHOffset, 0);
								rectTask.top = y;
								rectTask.bottom = y + m_sizeIcon.cy;

								// Determines the width of the text rectangle
								CRect rectText = rectTask;
								rectText.left += m_sizeIcon.cx + nIconHOffset;

								// Determines the height of the text rectangle
								CFont* pFontOld = dc.SelectObject (&globalData.fontUnderline);
								int cy = dc.DrawText (pTask->m_strName, rectText, DT_CALCRECT | DT_WORDBREAK);
								dc.SelectObject (pFontOld);
								
								if (pTask->m_bIsSeparator)
								{
									cy = max (cy, 10);
								}
								cy = max (cy, m_sizeIcon.cy);
								cy = max (cy, nTaskMinHeight);
								rectTask.bottom = rectTask.top + cy;

								if (!bCalcHeightOnly)
								{
									pTask->m_rect = rectTask;
								}
								
								y += cy + (GetTasksIconVertOffset() != -1 ? GetTasksIconVertOffset() :
									CBCGPVisualManager::GetInstance ()->GetTasksPaneIconVertOffset());
								bNeedHeaderOffset = FALSE;
								bNeedFooterOffset = FALSE;
							}
							// ----------------
							// single-line text
							// ----------------
							else
							{
								CFont* pFontOld = dc.SelectObject (&globalData.fontUnderline);
								CSize sizeText = dc.GetTextExtent (pTask->m_strName);
								dc.SelectObject (pFontOld);
								int cy = max (sizeText.cy, m_sizeIcon.cy);
								cy = max (cy, nTaskMinHeight);

								if (pTask->m_bIsSeparator)
								{
									cy = max (cy, 10);
								}
							
								if (!bCalcHeightOnly)
								{
									pTask->m_rect = CRect (
										rectTasks.left + nTaskHOffset, 
										y, 
										rectTasks.left + sizeText.cx + m_sizeIcon.cx + nTaskHOffset + nIconHOffset, 
										y + cy);
									pTask->m_rect.right = max(pTask->m_rect.left, rectTasks.right - nTaskHOffset);
								}

								y += cy + (GetTasksIconVertOffset() != -1 ? GetTasksIconVertOffset() :
									CBCGPVisualManager::GetInstance ()->GetTasksPaneIconVertOffset());
								bNeedHeaderOffset = FALSE;
								bNeedFooterOffset = FALSE;
							}
						}
						else
						{
							if (!bCalcHeightOnly)
							{
								pTask->m_rect.SetRectEmpty();
							}
						}
				}

				else // Use child window
				{
					if (bNeedHeaderOffset && pTask->m_bVisible)
					{
						if (m_bOffsetCustomControls)
						{
							y += (GetTasksIconVertOffset() != -1 ? GetTasksIconVertOffset() :
								CBCGPVisualManager::GetInstance ()->GetTasksPaneIconVertOffset());
						}
						else
						{
							y += sizeGroupBorders.cy;
						}
					}

					if (!bCalcHeightOnly)
					{
						CRect rectChildWnd = rectTasks;
						rectChildWnd.bottom = y + (pTask->m_bVisible ? pTask->m_nWindowHeight : 0);
						rectChildWnd.top = max (m_rectTasks.top + 1, y);
						int nChildScrollValue = pTask->m_nWindowHeight - rectChildWnd.Height ();
						rectChildWnd.bottom = min (m_rectTasks.bottom, rectChildWnd.bottom);

						if (m_bOffsetCustomControls)
						{
							rectChildWnd.DeflateRect ((GetTasksHorzOffset() != -1 ? GetTasksHorzOffset(): 
								CBCGPVisualManager::GetInstance ()->GetTasksPaneTaskHorzOffset()), 0);
						}
						else
						{
							rectChildWnd.DeflateRect (sizeGroupBorders.cx, 0);
						}

						if (pTask->m_WindowAlign != CBCGPTask::BCGP_TASKPANE_WND_STRETCH && 
							pTask->m_nWindowWidth > 0)
						{
							int nWidth = min (rectChildWnd.Width (), pTask->m_nWindowWidth);
							switch (pTask->m_WindowAlign)
							{
							case CBCGPTask::BCGP_TASKPANE_WND_LEFT:
								rectChildWnd.right = rectChildWnd.left + nWidth;
								break;

							case CBCGPTask::BCGP_TASKPANE_WND_CENTER:
								rectChildWnd.left += (rectChildWnd.Width () - nWidth) / 2;
								rectChildWnd.right = rectChildWnd.left + nWidth;
								break;

							case CBCGPTask::BCGP_TASKPANE_WND_RIGHT:
								rectChildWnd.left = rectChildWnd.right - nWidth;
								break;
							}
						}

						pTask->m_rect = rectChildWnd;

						// Scroll child windows:
						if (IsToolBox () || pTask->m_hwndTask != NULL)
						{
							if (pTask->m_rect.IsRectEmpty ())
							{
								ScrollChild (pTask->m_hwndTask, 0);
							}
							else
							{
								ScrollChild (pTask->m_hwndTask, nChildScrollValue);
							}
						}
					}

					if (pTask->m_bVisible)
					{
						y += pTask->m_nWindowHeight;
						bNeedHeaderOffset = TRUE;
						bNeedFooterOffset = TRUE;
					}
				}

				if (bNeedFooterOffset && pTask->m_bVisible)
				{
					if (m_bOffsetCustomControls)
					{
						y += (GetTasksIconVertOffset() != -1 ? GetTasksIconVertOffset() :
							CBCGPVisualManager::GetInstance ()->GetTasksPaneIconVertOffset());
					}
					else
					{
						y += sizeGroupBorders.cy;
					}
				}

				y += (GetTasksVertMargin() != -1 ? GetTasksVertMargin () :
					CBCGPVisualManager::GetInstance ()->GetTasksPaneTasksVertMargin());

				// constrain task's height during the animation:
				if (!bCalcHeightOnly)
				{
					if (m_bAnimationEnabled && pGroup == m_pAnimatedGroup)
					{
						if (y > yGroup + m_sizeAnim.cy)
						{
							int nSave = y;
							y = yGroup + max (0, m_sizeAnim.cy);
							pTask->m_rect.bottom = max (pTask->m_rect.top, min (pTask->m_rect.bottom, y - 1));
							m_nAnimGroupExtraHeight += nSave - y;
						}
					}
				}
			}

			if (!bCalcHeightOnly)
			{
				pGroup->m_rectGroup = CRect (rectTasks.left, yGroup, rectTasks.right, y);
			}
		}

		y += (GetGroupVertOffset() != -1 ? GetGroupVertOffset() :
			CBCGPVisualManager::GetInstance ()->GetTasksPaneGroupVertOffset());
		}
	}

	if (!bCalcHeightOnly)
	{
		// ---------------------------------------------
		// Find the last task group for the active page:
		// ---------------------------------------------
		CBCGPTasksGroup* pLastGroup = NULL;
		for (POSITION posGroup = m_lstTaskGroups.GetTailPosition (); posGroup != NULL; )
		{
			CBCGPTasksGroup* pGroup = (CBCGPTasksGroup*) m_lstTaskGroups.GetPrev (posGroup);
			ASSERT_VALID (pGroup);

			if (pGroup->m_pPage == pActivePage)
			{
				pLastGroup = pGroup;
				break;
			}
		}

		if (pLastGroup != NULL)
		{
			// ---------------------------------------------
			// Offset the last group if it's bottom aligned:
			// ---------------------------------------------
			if (pLastGroup->m_bIsBottom && !pLastGroup->m_lstTasks.IsEmpty () &&
				m_nVertScrollTotal == 0)
			{
				int nOffset = 0;
				for (POSITION posTask = pLastGroup->m_lstTasks.GetTailPosition (); posTask != NULL;)
				{
					CBCGPTask* pTask = (CBCGPTask*) pLastGroup->m_lstTasks.GetPrev (posTask);
					ASSERT_VALID (pTask);

					if (pTask->m_bVisible)
					{
						nOffset = rectTasks.bottom - pLastGroup->m_rectGroup.bottom;
						break;
					}
				}

				if (nOffset > 0)
				{
					for (POSITION posTask = pLastGroup->m_lstTasks.GetHeadPosition (); posTask != NULL;)
					{
						CBCGPTask* pTask = (CBCGPTask*) pLastGroup->m_lstTasks.GetNext (posTask);
						ASSERT_VALID (pTask);

						if (pTask->m_bVisible)
						{
							pTask->m_rect.OffsetRect (0, nOffset);
						}
					}

					pLastGroup->m_rect.OffsetRect (0, nOffset);
					pLastGroup->m_rectGroup.OffsetRect (0, nOffset);
				}
			}
		}

		// --------------------------------------------
		// Repos or hide child windows for active page:
		// --------------------------------------------
		for (pos = m_lstTaskGroups.GetHeadPosition (); pos != NULL;)
		{
			CBCGPTasksGroup* pGroup = (CBCGPTasksGroup*) m_lstTaskGroups.GetNext (pos);
			ASSERT_VALID (pGroup);

			if (pGroup->m_pPage == pActivePage)
			{
				BOOL bCollapsed = m_bCanCollapse && pGroup->m_bIsCollapsed && !pGroup->m_strName.IsEmpty();
				BOOL bAnimating = m_bAnimationEnabled && pGroup == m_pAnimatedGroup && m_sizeAnim.cy > 0;
				for (POSITION posTask = pGroup->m_lstTasks.GetHeadPosition (); posTask != NULL;)
				{
					CBCGPTask* pTask = (CBCGPTask*) pGroup->m_lstTasks.GetNext (posTask);
					ASSERT_VALID (pTask);

					if (pTask->m_hwndTask != NULL) // Use child window
					{
						CWnd* pChildWnd = CWnd::FromHandle (pTask->m_hwndTask);
						ASSERT_VALID(pChildWnd);
							
						if (bCollapsed && !bAnimating || !pTask->m_bVisible || pTask->m_rect.IsRectEmpty ())
						{
							if (IsToolBox ())
							{
								pChildWnd->SetWindowPos (NULL,
									pTask->m_rect.left, pTask->m_rect.top,
									pTask->m_rect.Width (), pTask->m_rect.Height (),
									SWP_NOZORDER | SWP_NOACTIVATE);
							}
							pChildWnd->ShowWindow (SW_HIDE);
						}
						else
						{
							pChildWnd->SetWindowPos (NULL,
								pTask->m_rect.left, pTask->m_rect.top,
								pTask->m_rect.Width (), pTask->m_rect.Height (),
								SWP_NOZORDER | SWP_NOACTIVATE);
							pChildWnd->ShowWindow (SW_SHOWNOACTIVATE);
						}
					}

				}
			}
		}
	}

	dc.SelectObject (pFontOld);
	return y - (GetGroupVertOffset() != -1 ? GetGroupVertOffset() :
		CBCGPVisualManager::GetInstance ()->GetTasksPaneGroupVertOffset()) +
		m_nVertScrollOffset * m_nRowHeight +
		(GetVertMargin() != -1 ? GetVertMargin() :
		CBCGPVisualManager::GetInstance ()->GetTasksPaneVertMargin());
}
//******************************************************************************************
void CBCGPTasksPane::OnDrawTasks (CDC* pDC, CRect /*rectWorkArea*/)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);

	CRect rectFill = m_rectTasks;
	rectFill.InflateRect (0, m_nVertScrollOffset * m_nRowHeight, 0, 0);

	OnFillBackground (pDC, rectFill);

	// ---------------
	// Get active page
	// ---------------
	CBCGPTasksPanePage* pActivePage = NULL;
	POSITION posPage = m_lstTasksPanes.FindIndex (m_arrHistoryStack[m_iActivePage]);
	ASSERT(posPage != NULL);

	pActivePage = (CBCGPTasksPanePage*) m_lstTasksPanes.GetAt (posPage);
	ASSERT_VALID(pActivePage);

	// ---------------------
	// Draw all tasks groups
	// ---------------------
	CRgn rgnClipTask;
	rgnClipTask.CreateRectRgnIndirect (CRect (0, 0, 0, 0));
	for (POSITION pos = m_lstTaskGroups.GetHeadPosition (); pos != NULL; )
	{
		CBCGPTasksGroup* pGroup = (CBCGPTasksGroup*) m_lstTaskGroups.GetNext (pos);
		ASSERT_VALID (pGroup);

		if (pGroup->m_pPage == pActivePage)
		{
		if (!pGroup->m_bIsCollapsed || pGroup->m_strName.IsEmpty() ||
			(m_bAnimationEnabled && pGroup == m_pAnimatedGroup && m_sizeAnim.cy > 0))
		{
			if (!pGroup->m_rectGroup.IsRectEmpty ())
			{
				OnFillGroup(pDC, pGroup);
			}

			if (!pGroup->m_rect.IsRectEmpty ())
			{
				CBCGPVisualManager::GetInstance ()->OnDrawTasksGroupCaption (
					pDC, pGroup, m_pHotGroupCaption == pGroup, FALSE, m_bCanCollapse);
			}
			if (!pGroup->m_rectGroup.IsRectEmpty ())
			{
				CSize sizeGroupBorders = GetTasksGroupBorders ();
				if (sizeGroupBorders.cx > 0 || sizeGroupBorders.cy > 0)
				{
					CBCGPVisualManager::GetInstance ()->OnDrawTasksGroupAreaBorder (
						pDC, pGroup->m_rectGroup, pGroup->m_bIsSpecial, pGroup->m_strName.IsEmpty());
				}

				// --------------
				// Draw all tasks
				// --------------
				for (POSITION posTask = pGroup->m_lstTasks.GetHeadPosition (); posTask != NULL;)
				{
					CBCGPTask* pTask = (CBCGPTask*) pGroup->m_lstTasks.GetNext (posTask);
					ASSERT_VALID (pTask);

					if (pTask->m_bVisible && pTask->m_hwndTask == NULL) // the task is not child window
					{
						rgnClipTask.SetRectRgn (&pTask->m_rect);
						pDC->SelectClipRgn (&rgnClipTask);

						CBCGPVisualManager::GetInstance ()->OnDrawTask(
							pDC, pTask, &m_lstIcons, (pTask == m_pHotTask), m_bActive && (pTask == m_pFocusedTask));

						if (m_bActive && m_pFocusedTask != NULL && pTask == m_pFocusedTask)
						{
							OnDrawTaskFocusRect (pDC, pTask);
						}

						pDC->SelectClipRgn (NULL);
					}
				}
			}
		}
		else	// Group is collapsed
		{
			if (!pGroup->m_rect.IsRectEmpty ())
			{
				CBCGPVisualManager::GetInstance ()->OnDrawTasksGroupCaption(pDC, pGroup, 
					m_pHotGroupCaption == pGroup, FALSE, m_bCanCollapse);
			}
		}
		if (m_bActive && m_pFocusedGroupCaption != NULL && pGroup == m_pFocusedGroupCaption)
		{
			OnDrawGroupFocusRect (pDC, pGroup);
		}

		}
	}
	rgnClipTask.DeleteObject ();

	// ------------------------
	// Draw navigation toolbar: 
	// ------------------------
	CRect rectToolbarOld = m_rectToolbar;
	if (m_bUseNavigationToolbar)
	{
		m_wndToolBar.Invalidate ();
		m_wndToolBar.UpdateWindow ();
	}

	// --------------------
	// Draw scroll buttons:
	// --------------------
	if (m_bUseScrollButtons)
	{
		if (IsScrollUpAvailable ())
		{	
			CBCGPVisualManager::GetInstance ()->OnDrawScrollButtons(pDC, m_rectScrollUp,
				iBorderSize, CBCGPMenuImages::IdArowUp, m_iScrollMode < 0);
		}

		if (IsScrollDnAvailable ())
		{
			CBCGPVisualManager::GetInstance ()->OnDrawScrollButtons(pDC, m_rectScrollDn,
				iBorderSize, CBCGPMenuImages::IdArowDown, m_iScrollMode > 0);
		}
	}
}
//******************************************************************************************
void CBCGPTasksPane::OnFillBackground (CDC* pDC, CRect rectFill)
{
	CBCGPVisualManager::GetInstance ()->OnFillTasksPaneBackground (pDC, rectFill);
}
//******************************************************************************************
void CBCGPTasksPane::OnDrawTaskFocusRect (CDC* pDC, CBCGPTask* pTask)
{
	CBCGPVisualManager::GetInstance ()->OnDrawFocusRect (pDC, pTask->m_rect);
}
//******************************************************************************************
void CBCGPTasksPane::OnDrawGroupFocusRect (CDC* pDC, CBCGPTasksGroup* pGroup)
{
	CBCGPVisualManager::GetInstance ()->OnDrawFocusRect (pDC, pGroup->m_rect);
}
//******************************************************************************************
void CBCGPTasksPane::OnFillGroup(CDC* pDC, CBCGPTasksGroup* pGroup)
{
	CBCGPVisualManager::GetInstance ()->OnFillTasksGroupInterior (pDC, pGroup->m_rectGroup);
}
//******************************************************************************************
void CBCGPTasksPane::SetIconsList (HIMAGELIST hIcons)
{
	ASSERT_VALID(this);

	if (m_lstIcons.GetSafeHandle () != NULL)
	{
		m_lstIcons.DeleteImageList ();
	}

	if (hIcons == NULL)
	{
		m_sizeIcon = CSize (0, 0);
	}
	else
	{
		m_lstIcons.Create (CImageList::FromHandle (hIcons));
		::ImageList_GetIconSize (hIcons, (int*) &m_sizeIcon.cx, (int*) &m_sizeIcon.cy);
	}

	AdjustScroll ();
	ReposTasks ();
	RedrawWindow (NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
}
//******************************************************************************************
BOOL CBCGPTasksPane::SetIconsList (UINT uiImageListResID, int cx,
							  COLORREF clrTransparent)
{
	ASSERT_VALID(this);

	CBitmap bmp;
	if (!bmp.LoadBitmap (uiImageListResID))
	{
		TRACE(_T("Can't load bitmap: %x\n"), uiImageListResID);
		return FALSE;
	}

	CImageList icons;

	BITMAP bmpObj;
	bmp.GetBitmap (&bmpObj);

	UINT nFlags = (clrTransparent == (COLORREF) -1) ? 0 : ILC_MASK;

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
		if (clrTransparent == (COLORREF)-1)
		{
			nFlags |= ILC_COLOR32 | ILC_MASK;
		}
		else
		{
			nFlags |= ILC_COLOR32;
		}
		break;
	}

	icons.Create (cx, bmpObj.bmHeight, nFlags, 0, 0);

	if (bmpObj.bmBitsPixel == 32 && clrTransparent == (COLORREF)-1)
	{
		icons.Add (&bmp, (CBitmap*) NULL);
	}
	else
	{
		icons.Add (&bmp, clrTransparent);
	}

	SetIconsList (icons);
	return TRUE;
}
//******************************************************************************************
int CBCGPTasksPane::AddPage (LPCTSTR lpszPageLabel)
{
	ASSERT(lpszPageLabel != NULL);

	CBCGPTasksPanePage* pPage = new CBCGPTasksPanePage(lpszPageLabel, this);
	ASSERT_VALID (pPage);

	m_lstTasksPanes.AddTail (pPage);

	RebuildMenu ();
	return (int) m_lstTasksPanes.GetCount() - 1;
}
//******************************************************************************************
void CBCGPTasksPane::RemovePage (int nPageIdx)
{
	ASSERT(nPageIdx <= m_lstTasksPanes.GetCount ()-1);
	if (nPageIdx <= 0)
	{
		ASSERT (FALSE);
		return;
	}

	POSITION posPage = m_lstTasksPanes.FindIndex (nPageIdx);
	ASSERT(posPage != NULL);
	CBCGPTasksPanePage* pPage = (CBCGPTasksPanePage*) m_lstTasksPanes.GetAt (posPage);
	ASSERT_VALID(pPage);

	//------------------------
	// Change the active page:
	//------------------------
	ASSERT(m_iActivePage >= 0);
	ASSERT(m_iActivePage <= m_arrHistoryStack.GetUpperBound ());
	int nOldPageIdx = m_arrHistoryStack[m_iActivePage];

	if (m_arrHistoryStack.GetSize () == 1)
	{
		// history is empty - select the default page
		SaveHistory (0);
		int nOldActivePage = m_iActivePage;
		m_iActivePage = (int) m_arrHistoryStack.GetUpperBound ();

		ChangeActivePage (m_iActivePage, nOldActivePage);
	}
	else if (nOldPageIdx == nPageIdx)
	{
		int nOldActivePage = m_iActivePage;
		if (m_iActivePage < m_arrHistoryStack.GetUpperBound ())
		{
			m_iActivePage = (int) m_arrHistoryStack.GetUpperBound ();
		}
		else
		{
			m_iActivePage--;
		}
		
		ChangeActivePage (m_iActivePage, nOldActivePage);
	}
	else if (GetSafeHwnd () != NULL)
	{
		RebuildMenu ();

		AdjustScroll ();
		ReposTasks ();
		RedrawWindow (NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
	}

	POSITION pos = NULL;

	//-----------------------------------------------
	// First, remove all tasks groups from this page:
	//-----------------------------------------------
	for (pos = m_lstTaskGroups.GetHeadPosition (); pos != NULL;)
	{
		POSITION posSave = pos;
		
		CBCGPTasksGroup* pGroup = (CBCGPTasksGroup*) m_lstTaskGroups.GetNext (pos);
		ASSERT_VALID(pGroup);

		if (pGroup->m_pPage == pPage)
		{
			m_lstTaskGroups.RemoveAt (posSave);
			delete pGroup;
		}
	}

	//-------------
	// Remove page:
	//-------------
	pos = m_lstTasksPanes.FindIndex (nPageIdx);
	ASSERT(pos != NULL);

	m_lstTasksPanes.RemoveAt (pos);
	delete pPage;

	// --------------------------------------------------------
	// Refresh history - remove references to the deleted page:
	// --------------------------------------------------------
	CArray <int, int> arrCopy;
	arrCopy.Copy (m_arrHistoryStack);
	m_arrHistoryStack.RemoveAll ();

	int nPrevIdx = nPageIdx;
	int iResult = -1;
	int iResultActivePage = 0;
	for (int i=0; i < arrCopy.GetSize (); i++)
	{
		if (arrCopy[i] != nPrevIdx)
		{
			if (arrCopy[i] < nPageIdx)
			{
				m_arrHistoryStack.Add (arrCopy[i]);
				nPrevIdx = arrCopy[i];
				iResult++;
			}
			else if (arrCopy[i] > nPageIdx)
			{
				m_arrHistoryStack.Add (arrCopy[i]-1);
				nPrevIdx = arrCopy[i];
				iResult++;
			}
		}
		if (i == m_iActivePage)
		{
			iResultActivePage = iResult;
		}
	}
	m_iActivePage = iResultActivePage;
	ASSERT (m_arrHistoryStack.GetSize () > 0);
	ASSERT(m_iActivePage >= 0);
	ASSERT(m_iActivePage <= m_arrHistoryStack.GetUpperBound ());

	RebuildMenu ();
}
//******************************************************************************************
void CBCGPTasksPane::RemoveAllPages ()
{
	//----------------------
	// Reset an active page:
	//----------------------
	int nOldActivePage = m_iActivePage;
	m_iActivePage = 0;
	ChangeActivePage (0, nOldActivePage);	// Default page
	m_arrHistoryStack.RemoveAll ();
	m_arrHistoryStack.Add (0);

	//--------------------------------------------------------
	// First, remove all tasks group except from default page:
	//--------------------------------------------------------
	for (POSITION pos = m_lstTaskGroups.GetHeadPosition (); pos != NULL;)
	{
		POSITION posSave = pos;
		
		CBCGPTasksGroup* pGroup = (CBCGPTasksGroup*) m_lstTaskGroups.GetNext (pos);
		ASSERT_VALID(pGroup);

		if (pGroup->m_pPage != NULL && 
			pGroup->m_pPage != m_lstTasksPanes.GetHead ()) // except default page
		{
			m_lstTaskGroups.RemoveAt (posSave);
			delete pGroup;
		}
	}

	//----------------------------------
	// Remove pages except default page:
	//----------------------------------
	while (m_lstTasksPanes.GetCount () > 1)
	{
		delete m_lstTasksPanes.RemoveTail ();
	}

	RebuildMenu ();
}
//******************************************************************************************
int CBCGPTasksPane::AddGroup (int nPageIdx, LPCTSTR lpszGroupName, BOOL bBottomLocation/* = FALSE*/, 
							 BOOL bSpecial/* = FALSE*/, HICON hIcon/* = NULL*/)
{
	ASSERT (nPageIdx >= 0);
	ASSERT (nPageIdx <= m_lstTasksPanes.GetCount ()-1);

	// ---------------
	// Get active page
	// ---------------
	CBCGPTasksPanePage* pActivePage = NULL;
	POSITION posPage = m_lstTasksPanes.FindIndex (nPageIdx);
	ASSERT(posPage != NULL);

	pActivePage = (CBCGPTasksPanePage*) m_lstTasksPanes.GetAt (posPage);
	ASSERT_VALID(pActivePage);

	// -------------
	// Add new group
	// -------------
	for (POSITION pos = m_lstTaskGroups.GetHeadPosition (); pos != NULL;)
	{
		CBCGPTasksGroup* pGroup = (CBCGPTasksGroup*) m_lstTaskGroups.GetNext (pos);
		ASSERT_VALID (pGroup);

		if (pGroup->m_pPage == pActivePage)
		{
			if (pGroup->m_bIsBottom)
			{
				pGroup->m_bIsBottom = FALSE;
			}
		}
	}

	m_lstTaskGroups.AddTail (new CBCGPTasksGroup (lpszGroupName, bBottomLocation, 
		bSpecial, FALSE, pActivePage, hIcon));

	AdjustScroll ();
	ReposTasks ();

	return (int) m_lstTaskGroups.GetCount () - 1;
}
//******************************************************************************************
void CBCGPTasksPane::RemoveGroup (int nGroup)
{
	ASSERT (nGroup >= 0);
	ASSERT (nGroup < m_lstTaskGroups.GetCount ());

	POSITION pos = m_lstTaskGroups.FindIndex (nGroup);
	if (pos == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	CBCGPTasksGroup* pGroup = (CBCGPTasksGroup*) m_lstTaskGroups.GetAt (pos);
	ASSERT_VALID (pGroup);

	m_lstTaskGroups.RemoveAt (pos);
	delete pGroup;

	AdjustScroll ();
	ReposTasks ();
	RedrawWindow (NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
}
//******************************************************************************************
void CBCGPTasksPane::RemoveAllGroups (int nPageIdx/* = 0*/)
{
	ASSERT (nPageIdx >= 0);
	ASSERT (nPageIdx < m_lstTasksPanes.GetCount ());
	
	POSITION posPage = m_lstTasksPanes.FindIndex (nPageIdx);
	if (posPage == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	CBCGPTasksPanePage* pPage = (CBCGPTasksPanePage*) m_lstTasksPanes.GetAt (posPage);
	ASSERT_VALID (pPage);

	//----------------------------------------
	// Remove all tasks groups from this page:
	//----------------------------------------
	for (POSITION pos = m_lstTaskGroups.GetHeadPosition (); pos != NULL;)
	{
		POSITION posSave = pos;
		
		CBCGPTasksGroup* pGroup = (CBCGPTasksGroup*) m_lstTaskGroups.GetNext (pos);
		ASSERT_VALID(pGroup);

		if (pGroup->m_pPage == pPage)
		{
			m_lstTaskGroups.RemoveAt (posSave);
			delete pGroup;
		}
	}

	AdjustScroll ();
	ReposTasks ();
	RedrawWindow (NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
}
//******************************************************************************************
BOOL CBCGPTasksPane::SetGroupName (int nGroup, LPCTSTR lpszGroupName)
{
	POSITION pos = m_lstTaskGroups.FindIndex (nGroup);
	if (pos == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	CBCGPTasksGroup* pGroup = (CBCGPTasksGroup*) m_lstTaskGroups.GetAt (pos);
	ASSERT_VALID (pGroup);

	BOOL	bCaptionWasEmpty	= pGroup->m_strName.IsEmpty();

	pGroup->m_strName	= lpszGroupName;

	if ((!bCaptionWasEmpty && pGroup->m_strName.IsEmpty()) || 
		(bCaptionWasEmpty && !pGroup->m_strName.IsEmpty()))
	{
		AdjustScroll ();
		ReposTasks ();
		RedrawWindow (NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
	}
	else
	{
		InvalidateRect (&pGroup->m_rect);
		UpdateWindow ();
	}

	return TRUE;
}
//******************************************************************************************
BOOL CBCGPTasksPane::SetGroupTextColor (int nGroup, COLORREF color, COLORREF colorHot)
{
	POSITION pos = m_lstTaskGroups.FindIndex (nGroup);
	if (pos == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	CBCGPTasksGroup* pGroup = (CBCGPTasksGroup*) m_lstTaskGroups.GetAt (pos);
	ASSERT_VALID (pGroup);

	pGroup->m_clrText = (COLORREF) color;
	pGroup->m_clrTextHot = (COLORREF) colorHot;

	InvalidateRect (&pGroup->m_rect);
	UpdateWindow ();

	return TRUE;
}
//******************************************************************************************
BOOL CBCGPTasksPane::CollapseGroup (CBCGPTasksGroup* pGroup, BOOL bCollapse)
{
	ASSERT_VALID (pGroup);

	if ((!bCollapse && pGroup->m_bIsCollapsed) ||
		(bCollapse && !pGroup->m_bIsCollapsed))
	{
		pGroup->m_bIsCollapsed	= bCollapse;

		AdjustScroll ();
		ReposTasks ();
		RedrawWindow (NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
	}

	return TRUE;
}
//******************************************************************************************
void CBCGPTasksPane::CollapseAllGroups (BOOL bCollapse)
{
	// -------------------
	// Collapse all groups
	// -------------------
	for (POSITION pos = m_lstTaskGroups.GetHeadPosition (); pos != NULL;)
	{
		CBCGPTasksGroup* pGroup = (CBCGPTasksGroup*) m_lstTaskGroups.GetNext (pos);
		ASSERT_VALID(pGroup);

		if ((!bCollapse && pGroup->m_bIsCollapsed) ||
			(bCollapse && !pGroup->m_bIsCollapsed))
		{
			pGroup->m_bIsCollapsed	= bCollapse;
		}
	}

	AdjustScroll ();
	ReposTasks ();
	RedrawWindow (NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
}
//******************************************************************************************
void CBCGPTasksPane::CollapseAllGroups (int nPageIdx, BOOL bCollapse)
{
	ASSERT (nPageIdx >= 0);
	ASSERT (nPageIdx < m_lstTasksPanes.GetCount ());
	
	POSITION posPage = m_lstTasksPanes.FindIndex (nPageIdx);
	if (posPage == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	CBCGPTasksPanePage* pPage = (CBCGPTasksPanePage*) m_lstTasksPanes.GetAt (posPage);
	ASSERT_VALID (pPage);

	// -----------------------------------------
	// Collapse all groups at the specified page
	// -----------------------------------------
	for (POSITION pos = m_lstTaskGroups.GetHeadPosition (); pos != NULL;)
	{
		CBCGPTasksGroup* pGroup = (CBCGPTasksGroup*) m_lstTaskGroups.GetNext (pos);
		ASSERT_VALID(pGroup);

		if (pGroup->m_pPage == pPage)
		{
			if ((!bCollapse && pGroup->m_bIsCollapsed) ||
				(bCollapse && !pGroup->m_bIsCollapsed))
			{
				pGroup->m_bIsCollapsed	= bCollapse;
			}
		}
	}

	AdjustScroll ();
	ReposTasks ();
	RedrawWindow (NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
}
//******************************************************************************************
CBCGPTasksGroup* CBCGPTasksPane::GetTaskGroup (int nGroup) const
{
	ASSERT(nGroup >= 0);
	ASSERT(nGroup < m_lstTaskGroups.GetCount ());

	POSITION pos = m_lstTaskGroups.FindIndex (nGroup);
	if (pos == NULL)
	{
		return NULL;
	}

	CBCGPTasksGroup* pGroup = (CBCGPTasksGroup*) m_lstTaskGroups.GetAt (pos);
	ASSERT_VALID (pGroup);

	return pGroup;
}
//******************************************************************************************
BOOL CBCGPTasksPane::GetGroupLocation (CBCGPTasksGroup* pGroup, int &nGroup) const
{
	ASSERT_VALID (pGroup);

	int nGroupCount = 0;
	for (POSITION pos = m_lstTaskGroups.GetHeadPosition (); pos != NULL; nGroupCount++)
	{
		CBCGPTasksGroup* pTaskGroup = (CBCGPTasksGroup*) m_lstTaskGroups.GetNext (pos);
		ASSERT_VALID (pTaskGroup);

		if (pTaskGroup == pGroup)
		{
			nGroup = nGroupCount;
			return TRUE;
		}
	}

	return FALSE; // not found
}
//******************************************************************************************
int CBCGPTasksPane::AddTask (int nGroup, LPCTSTR lpszTaskName, int nTaskIcon/* = -1*/,
						UINT uiCommandID/* = 0*/, DWORD_PTR dwUserData/* = 0*/)
{
	POSITION pos = m_lstTaskGroups.FindIndex (nGroup);
	if (pos == NULL)
	{
		ASSERT (FALSE);
		return -1;
	}

	CBCGPTasksGroup* pGroup = (CBCGPTasksGroup*) m_lstTaskGroups.GetAt (pos);
	ASSERT_VALID (pGroup);

	pGroup->m_lstTasks.AddTail (new CBCGPTask (
		pGroup, lpszTaskName, nTaskIcon, uiCommandID, dwUserData));

	AdjustScroll ();
	ReposTasks ();

	return (int) pGroup->m_lstTasks.GetCount () - 1;
}
//*********************************************************************************
BOOL CBCGPTasksPane::SetTaskName (int nGroup, int nTask, LPCTSTR lpszTaskName)
{
	POSITION pos = m_lstTaskGroups.FindIndex (nGroup);
	if (pos == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	CBCGPTasksGroup* pGroup = (CBCGPTasksGroup*) m_lstTaskGroups.GetAt (pos);
	ASSERT_VALID (pGroup);

	pos	= pGroup->m_lstTasks.FindIndex (nTask);
	if (pos == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	CBCGPTask*	pTask	= (CBCGPTask*) pGroup->m_lstTasks.GetAt (pos);
	pTask->m_strName	= lpszTaskName;

	if (pTask->m_bVisible)
		InvalidateRect (pTask->m_rect);

	return TRUE;
}
//*********************************************************************************
BOOL CBCGPTasksPane::SetTaskTextColor (int nGroup, int nTask, COLORREF color,
									   COLORREF colorHot)
{
	POSITION pos = m_lstTaskGroups.FindIndex (nGroup);
	if (pos == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	CBCGPTasksGroup* pGroup = (CBCGPTasksGroup*) m_lstTaskGroups.GetAt (pos);
	ASSERT_VALID (pGroup);

	pos	= pGroup->m_lstTasks.FindIndex (nTask);
	if (pos == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	CBCGPTask*	pTask	= (CBCGPTask*) pGroup->m_lstTasks.GetAt (pos);
	pTask->m_clrText	= color;
	pTask->m_clrTextHot	= colorHot;

	if (pTask->m_bVisible)
		InvalidateRect (pTask->m_rect);

	return TRUE;
}
//******************************************************************************************
BOOL CBCGPTasksPane::ShowTask (int nGroup, int nTask, BOOL bShow, BOOL bRedraw)
{
	POSITION pos = m_lstTaskGroups.FindIndex (nGroup);
	if (pos == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	CBCGPTasksGroup* pGroup = (CBCGPTasksGroup*) m_lstTaskGroups.GetAt (pos);
	ASSERT_VALID (pGroup);

	pos	= pGroup->m_lstTasks.FindIndex (nTask);
	if (pos == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	CBCGPTask*	pTask	= (CBCGPTask*) pGroup->m_lstTasks.GetAt (pos);
	if ((!bShow && pTask->m_bVisible) ||
		(bShow && !pTask->m_bVisible))
	{
		pTask->m_bVisible	= bShow;

		AdjustScroll ();
		ReposTasks ();

		if (bRedraw)
		{
			RedrawWindow (NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
		}
	}

	return TRUE;
}
//******************************************************************************************
BOOL CBCGPTasksPane::ShowTaskByCmdId (UINT uiCommandID, BOOL bShow, BOOL bRedraw)
{
	int nGroup, nTask;

	if (!GetTaskLocation (uiCommandID, nGroup, nTask))
		return FALSE;

	return ShowTask (nGroup, nTask, bShow, bRedraw);
}
//******************************************************************************************
BOOL CBCGPTasksPane::RemoveTask (int nGroup, int nTask, BOOL bRedraw)
{
	ASSERT(nGroup >= 0);
	ASSERT(nGroup < m_lstTaskGroups.GetCount ());
	
	POSITION pos = m_lstTaskGroups.FindIndex (nGroup);
	if (pos == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	CBCGPTasksGroup* pGroup = (CBCGPTasksGroup*) m_lstTaskGroups.GetAt (pos);
	ASSERT_VALID (pGroup);

	pos	= pGroup->m_lstTasks.FindIndex (nTask);
	if (pos == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	delete pGroup->m_lstTasks.GetAt (pos);
	pGroup->m_lstTasks.RemoveAt (pos);

	AdjustScroll ();
	ReposTasks ();

	if (bRedraw)
	{
		RedrawWindow (NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
	}

	return TRUE;
}
//******************************************************************************************
void CBCGPTasksPane::RemoveAllTasks (int nGroup)
{
	ASSERT(nGroup >= 0);
	ASSERT(nGroup < m_lstTaskGroups.GetCount ());

	CBCGPTasksGroup* pGroup = GetTaskGroup (nGroup);
	ASSERT_VALID (pGroup);

	while (!pGroup->m_lstTasks.IsEmpty ())
	{
		delete pGroup->m_lstTasks.RemoveHead ();
	}

	AdjustScroll ();
	ReposTasks ();
	RedrawWindow (NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
}
//******************************************************************************************
BOOL CBCGPTasksPane::GetTaskLocation (UINT uiCommandID, int& nGroup, int& nTask) const
{
	nGroup	= 0;
	for (POSITION pos = m_lstTaskGroups.GetHeadPosition (); pos != NULL; ++nGroup)
	{
		CBCGPTasksGroup* pGroup = (CBCGPTasksGroup*) m_lstTaskGroups.GetNext (pos);
		ASSERT_VALID (pGroup);

		nTask	= 0;
		for (POSITION posTask = pGroup->m_lstTasks.GetHeadPosition (); posTask != NULL; ++nTask)
		{
			CBCGPTask* pTask = (CBCGPTask*) pGroup->m_lstTasks.GetNext (posTask);
			ASSERT_VALID (pTask);

			if (pTask->m_uiCommandID == uiCommandID)
			{
				return TRUE;
			}
		}
	}

	nGroup	= -1;
	nTask	= -1;

	return FALSE;
}
//******************************************************************************************
BOOL CBCGPTasksPane::GetTaskLocation (HWND hwndTask, int& nGroup, int& nTask) const
{
	nGroup	= 0;
	for (POSITION pos = m_lstTaskGroups.GetHeadPosition (); pos != NULL; ++nGroup)
	{
		CBCGPTasksGroup* pGroup = (CBCGPTasksGroup*) m_lstTaskGroups.GetNext (pos);
		ASSERT_VALID (pGroup);

		nTask	= 0;
		for (POSITION posTask = pGroup->m_lstTasks.GetHeadPosition (); posTask != NULL; ++nTask)
		{
			CBCGPTask* pTask = (CBCGPTask*) pGroup->m_lstTasks.GetNext (posTask);
			ASSERT_VALID (pTask);

			if (pTask->m_hwndTask == hwndTask)
			{
				return TRUE;
			}
		}
	}

	nGroup	= -1;
	nTask	= -1;

	return FALSE;
}
//******************************************************************************************
CBCGPTask* CBCGPTasksPane::GetTask (int nGroup, int nTask) const
{
	ASSERT(nGroup >= 0);
	ASSERT(nGroup < m_lstTaskGroups.GetCount ());

	CBCGPTasksGroup* pGroup = GetTaskGroup (nGroup);
	ASSERT_VALID (pGroup);

	POSITION pos = pGroup->m_lstTasks.FindIndex (nTask);
	if (pos == NULL)
	{
		return NULL;
	}

	CBCGPTask* pTask = (CBCGPTask*) pGroup->m_lstTasks.GetAt (pos);
	ASSERT_VALID (pTask);

	return pTask;
}
//******************************************************************************************
BOOL CBCGPTasksPane::GetTaskLocation (CBCGPTask* pTask, int& nGroup, int& nTask) const
{
	ASSERT_VALID (pTask);
	ASSERT_VALID (pTask->m_pGroup);

	nGroup	= -1;
	nTask	= -1;

	CBCGPTasksGroup* pGroupToFind = pTask->m_pGroup;

	int nGroupCount	= 0;
	for (POSITION pos = m_lstTaskGroups.GetHeadPosition (); pos != NULL; nGroupCount++)
	{
		CBCGPTasksGroup* pGroup = (CBCGPTasksGroup*) m_lstTaskGroups.GetNext (pos);
		ASSERT_VALID (pGroup);

		if (pGroup == pGroupToFind)
		{
			int nTaskCount	= 0;
			for (POSITION posTask = pGroup->m_lstTasks.GetHeadPosition (); posTask != NULL; nTaskCount++)
			{
				CBCGPTask* pCurTask = (CBCGPTask*) pGroup->m_lstTasks.GetNext (posTask);
				ASSERT_VALID (pCurTask);

				if (pCurTask == pTask)
				{
					nGroup	= nGroupCount;
					nTask	= nTaskCount;
					return TRUE;
				}
			}

			return FALSE;
		}
	}

	return FALSE;
}
//******************************************************************************************
int CBCGPTasksPane::AddWindow (int nGroup, HWND hwndTask, int nWndHeight, 
							  BOOL bAutoDestroyWindow/* = FALSE*/,
							  DWORD_PTR dwUserData/* = 0*/,
							  CBCGPTask::WindowAlign align/* = BCGP_TASKPANE_WND_STRETCH*/,
							  int nWndWidth/* = 0*/)
{
	POSITION pos = m_lstTaskGroups.FindIndex (nGroup);
	if (pos == NULL)
	{
		ASSERT (FALSE);
		return -1;
	}

	ASSERT (::IsWindow (hwndTask));

	if (nWndWidth == 0)
	{
		align = CBCGPTask::BCGP_TASKPANE_WND_STRETCH;
	}

	CBCGPTasksGroup* pGroup = (CBCGPTasksGroup*) m_lstTaskGroups.GetAt (pos);
	ASSERT_VALID (pGroup);

	pGroup->m_lstTasks.AddTail (new CBCGPTask (
		pGroup, _T(""), -1, 0, dwUserData, hwndTask, bAutoDestroyWindow, nWndHeight, align, nWndWidth));

	AdjustScroll ();
	ReposTasks ();

	return (int) pGroup->m_lstTasks.GetCount () - 1;
}
//******************************************************************************************
BOOL CBCGPTasksPane::SetWindowHeight (int nGroup, HWND hwndTask, int nWndHeight)
{
	POSITION pos = m_lstTaskGroups.FindIndex (nGroup);
	if (pos == NULL)
	{
		ASSERT (FALSE);
		return -1;
	}

	ASSERT (::IsWindow (hwndTask));

	CBCGPTasksGroup* pGroup = (CBCGPTasksGroup*) m_lstTaskGroups.GetAt (pos);
	ASSERT_VALID (pGroup);

	POSITION pos2 = pGroup->m_lstTasks.GetHeadPosition();
	while (pos2 != NULL)
	{
		CBCGPTask*	pTask	= (CBCGPTask*) pGroup->m_lstTasks.GetNext(pos2);

		if (pTask->m_hwndTask == hwndTask)
		{
			pTask->m_nWindowHeight	= nWndHeight;

			if (!pGroup->m_bIsCollapsed)
			{
				AdjustScroll ();
				ReposTasks ();
				RedrawWindow (NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
			}

			return TRUE;
		}
	}

	return FALSE;
}
//******************************************************************************************
BOOL CBCGPTasksPane::SetWindowHeight (HWND hwndTask, int nWndHeight)
{
	ASSERT (::IsWindow (hwndTask));

	int	nGroup, nTask;
	if (GetTaskLocation (hwndTask, nGroup, nTask))
	{
		return SetWindowHeight (nGroup, hwndTask, nWndHeight);
	}

	return FALSE;
}
//******************************************************************************************
LRESULT CBCGPTasksPane::OnSetFont (WPARAM wParam, LPARAM /*lParam*/)
{
	m_hFont = (HFONT) wParam;

	CreateFonts ();

	AdjustScroll ();
	ReposTasks ();
	return 0;
}
//******************************************************************************************
LRESULT CBCGPTasksPane::OnGetFont (WPARAM, LPARAM)
{
	return (LRESULT) (m_hFont != NULL ? m_hFont : ::GetStockObject (DEFAULT_GUI_FONT));
}
//******************************************************************************************
void CBCGPTasksPane::CreateFonts ()
{
	if (m_fontBold.GetSafeHandle () != NULL)
	{
		m_fontBold.DeleteObject ();
	}
	if (m_fontBoldUnderline.GetSafeHandle() != NULL)
	{
		m_fontBoldUnderline.DeleteObject ();
	}
	if (m_fontUnderline.GetSafeHandle () != NULL)
	{
		m_fontUnderline.DeleteObject ();
	}

	CFont* pFont = CFont::FromHandle (
		m_hFont != NULL ? m_hFont : (HFONT) ::GetStockObject (DEFAULT_GUI_FONT));
	ASSERT_VALID (pFont);

	LOGFONT lf;
	memset (&lf, 0, sizeof (LOGFONT));

	pFont->GetLogFont (&lf);

	lf.lfWeight = FW_BOLD;
	m_fontBold.CreateFontIndirect (&lf);

	lf.lfUnderline = TRUE;
	m_fontBoldUnderline.CreateFontIndirect (&lf);

	lf.lfWeight = FW_NORMAL;
	lf.lfUnderline = TRUE;
	m_fontUnderline.CreateFontIndirect (&lf);
}
//******************************************************************************************
void CBCGPTasksPane::OnSettingChange(UINT uFlags, LPCTSTR lpszSection) 
{
	CWnd::OnSettingChange(uFlags, lpszSection);	

	AdjustScroll ();
	ReposTasks ();
}
//******************************************************************************************
HFONT CBCGPTasksPane::SetFont (CDC* pDC)
{
	ASSERT_VALID (pDC);
	
	return (HFONT) ::SelectObject (pDC->GetSafeHdc (), 
		m_hFont != NULL ? m_hFont : ::GetStockObject (DEFAULT_GUI_FONT));
}
//****************************************************************************************
CBCGPTask* CBCGPTasksPane::TaskHitTest (CPoint pt) const
{
	if (!m_rectTasks.PtInRect (pt))
	{
		return NULL;
	}

	// ---------------
	// Get active page
	// ---------------
	CBCGPTasksPanePage* pActivePage = NULL;
	POSITION posPage = m_lstTasksPanes.FindIndex (m_arrHistoryStack[m_iActivePage]);
	ASSERT(posPage != NULL);

	pActivePage = (CBCGPTasksPanePage*) m_lstTasksPanes.GetAt (posPage);
	ASSERT_VALID(pActivePage);

	// -----------------------------
	// Test all tasks in active page
	// -----------------------------
	for (POSITION pos = m_lstTaskGroups.GetHeadPosition (); pos != NULL;)
	{
		CBCGPTasksGroup* pGroup = (CBCGPTasksGroup*) m_lstTaskGroups.GetNext (pos);
		ASSERT_VALID (pGroup);

		if (pGroup->m_pPage == pActivePage)
		{
		for (POSITION posTask = pGroup->m_lstTasks.GetHeadPosition (); posTask != NULL;)
		{
			CBCGPTask* pTask = (CBCGPTask*) pGroup->m_lstTasks.GetNext (posTask);
			ASSERT_VALID (pTask);

			if (pTask->m_bVisible && pTask->m_rect.PtInRect (pt))
			{
				if (pTask->m_uiCommandID != 0) // ignore labels
				{
					return pTask;
				}
			}
		}
		}
	}

	return NULL;
}
//***************************************************************************************
CBCGPTasksGroup* CBCGPTasksPane::GroupCaptionHitTest (CPoint pt) const
{
	if (!m_bCanCollapse)
	{
		return NULL;
	}

	if (!m_rectTasks.PtInRect (pt))
	{
		return NULL;
	}

	// ---------------
	// Get active page
	// ---------------
	CBCGPTasksPanePage* pActivePage = NULL;
	POSITION posPage = m_lstTasksPanes.FindIndex (m_arrHistoryStack[m_iActivePage]);
	ASSERT(posPage != NULL);

	pActivePage = (CBCGPTasksPanePage*) m_lstTasksPanes.GetAt (posPage);
	ASSERT_VALID(pActivePage);
	
	// ------------------------------
	// Test all groups in active page
	// ------------------------------
	for (POSITION pos = m_lstTaskGroups.GetHeadPosition (); pos != NULL;)
	{
		CBCGPTasksGroup* pGroup = (CBCGPTasksGroup*) m_lstTaskGroups.GetNext (pos);
		ASSERT_VALID (pGroup);

		if (pGroup->m_pPage == pActivePage)
		{
		if (pGroup->m_rect.PtInRect (pt))
		{
			return pGroup;
		}
		}
	}

	return NULL;
}
//***************************************************************************************
BOOL CBCGPTasksPane::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	CPoint ptCursor;
	::GetCursorPos (&ptCursor);
	ScreenToClient (&ptCursor);

	CBCGPTask* pTaskHit = TaskHitTest (ptCursor);
	if (m_pClickedTask != NULL && m_pClickedTask->m_bEnabled || 
		pTaskHit != NULL && pTaskHit->m_bEnabled)
	{
		::SetCursor (globalData.GetHandCursor ());
		return TRUE;
	}

	if (m_bCanCollapse && (m_pClickedGroupCaption != NULL || 
		GroupCaptionHitTest (ptCursor) != NULL))
	{
		::SetCursor (globalData.GetHandCursor ());
		return TRUE;
	}
	
	return CBCGPDockingControlBar::OnSetCursor(pWnd, nHitTest, message);
}
//******************************************************************************************
void CBCGPTasksPane::OnMouseMove(UINT nFlags, CPoint point) 
{
	CBCGPDockingControlBar::OnMouseMove(nFlags, point);

	BOOL bUpdate = FALSE;

	if (m_bUseScrollButtons)
	{
		if (m_rectScrollUp.PtInRect (point) && IsScrollUpAvailable ())
		{
			m_iScrollMode = -1;
			InvalidateRect (m_rectScrollUp);
		}
		else if (m_rectScrollDn.PtInRect (point) && IsScrollDnAvailable ())
		{
			m_iScrollMode = 1;
			InvalidateRect (m_rectScrollDn);
		}
		else
		{
			m_iScrollMode = 0;
		}

		if (m_iScrollMode != 0)
		{
			SetTimer (iScrollTimerId, m_iScrollTimerDuration, NULL);
			return;
		}
	}

	CBCGPTasksGroup* pHotGroup = GroupCaptionHitTest (point);
	CBCGPTask* pHotTask = TaskHitTest (point);

	// ----------
	// No changes
	// ----------
	if (m_pHotTask == pHotTask && m_pHotGroupCaption == pHotGroup)
	{
		return;
	}


	// ----------------
	// No new hot areas
	// ----------------
	if (pHotTask == NULL && pHotGroup == NULL)
	{
		if (m_pHotGroupCaption != NULL)
		{
			// remove old group caption hotlight
			CRect rectUpdate = m_pHotGroupCaption->m_rect;
			m_pHotGroupCaption = NULL;
			if (m_pClickedGroupCaption == NULL)
			{
				ReleaseCapture ();
			}

			RedrawWindow (rectUpdate, NULL, RDW_INVALIDATE | RDW_ERASE);
			bUpdate = TRUE;
		}

		if (m_pHotTask != NULL)
		{
			// remove old task hotlight
			CRect rectUpdate = m_pHotTask->m_rect;
			m_pHotTask = NULL;
			if (m_pClickedTask == NULL)
			{
				ReleaseCapture ();
			}

			RedrawWindow (rectUpdate, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
			bUpdate = TRUE;
		}
		
		GetOwner()->SendMessage (WM_SETMESSAGESTRING, AFX_IDS_IDLEMESSAGE);

		if (bUpdate)
		{
			UpdateWindow ();
		}

		return;
	}

	// ---------------------
	// New hot group caption
	// ---------------------
	if (pHotGroup != NULL)
	{
		if (m_pHotGroupCaption == NULL)
		{
			if (GetCapture () != NULL)
			{
				return;
			}
			SetCapture ();
		}
		else
		{	
			// remove old group caption hotlight
			CRect rectTask = m_pHotGroupCaption->m_rect;
			m_pHotGroupCaption = NULL;
			RedrawWindow (rectTask, NULL, RDW_INVALIDATE | RDW_ERASE);

			bUpdate = TRUE;
		}
		
		// remove old task hotlight
		if (m_pHotTask != NULL)
		{
			CRect rectUpdate = m_pHotTask->m_rect;
			m_pHotTask = NULL;
			RedrawWindow (rectUpdate, NULL, RDW_INVALIDATE | RDW_ERASE);

			bUpdate = TRUE;
		}

		// add new group caption hotlight
		m_pHotGroupCaption = pHotGroup;
		RedrawWindow (pHotGroup->m_rect, NULL, RDW_INVALIDATE | RDW_ERASE);

		bUpdate = TRUE;

		OnHotGroupCaption (pHotGroup);
	}
	
	// ------------
	// New hot task
	// ------------
	else if (pHotTask != NULL)
	{
		if (!pHotTask->m_bEnabled)
		{
			GetOwner()->SendMessage (WM_SETMESSAGESTRING, AFX_IDS_IDLEMESSAGE);
			return;
		}

		if (m_pHotTask == NULL)
		{
			if (GetCapture () != NULL)
			{
				return;
			}
			SetCapture ();
		}
		else
		{
			// remove old task hotlight
			CRect rectTask = m_pHotTask->m_rect;
			m_pHotTask = NULL;
			RedrawWindow (rectTask, NULL, RDW_INVALIDATE | RDW_ERASE);

			bUpdate = TRUE;
		}
		
		// remove old group caption hotlight
		if (m_pHotGroupCaption != NULL)
		{
			CRect rectUpdate = m_pHotGroupCaption->m_rect;
			m_pHotGroupCaption = NULL;
			RedrawWindow (rectUpdate, NULL, RDW_INVALIDATE | RDW_ERASE);

			bUpdate = TRUE;
		}

		// add new task hotlight
		m_pHotTask = pHotTask;
		RedrawWindow (pHotTask->m_rect, NULL, RDW_INVALIDATE | RDW_ERASE);

		bUpdate = TRUE;

		if (pHotTask->m_uiCommandID != 0)
		{
			ShowCommandMessageString (pHotTask->m_uiCommandID);
		}
		OnHotTask (pHotTask);
	}

	if (bUpdate)
	{
		UpdateWindow ();
	}
}
//******************************************************************************************
void CBCGPTasksPane::OnLButtonUp(UINT nFlags, CPoint point) 
{
	CBCGPDockingControlBar::OnLButtonUp(nFlags, point);

	if (m_pHotTask == NULL && m_pClickedTask == NULL && 
		m_pHotGroupCaption == NULL && m_pClickedGroupCaption == NULL)
	{
		return;
	}

	ReleaseCapture ();

	// --------------------------
	// Handle group caption click
	// --------------------------
	CBCGPTasksGroup* pHotGroupCaption = m_pHotGroupCaption;
	BOOL bIsGroupCaptionClick = (m_pHotGroupCaption != NULL && 
		m_pHotGroupCaption == GroupCaptionHitTest (point) && 
		m_pClickedGroupCaption == m_pHotGroupCaption);

	m_pClickedGroupCaption = NULL;

	if (bIsGroupCaptionClick)
	{
		m_pHotGroupCaption = NULL;

		DoGroupCaptionClick(pHotGroupCaption, FALSE);

		// Trigger mouse move event (to change selection notification):
		SendMessage (WM_MOUSEMOVE, nFlags, MAKELPARAM (point.x, point.y));
		return;
	}
	else
	{
		CRect rectGroupCaption = (m_pHotGroupCaption != NULL) ? 
			m_pHotGroupCaption->m_rect : CRect (0, 0, 0, 0);
		RedrawWindow (rectGroupCaption, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
	}

	// -----------------
	// Handle task click
	// -----------------
	CBCGPTask* pHotTask = m_pHotTask;
	BOOL bIsTaskClick = (m_pHotTask != NULL && m_pHotTask == TaskHitTest (point) &&
		m_pClickedTask == m_pHotTask);

	CRect rectTask = (m_pHotTask != NULL) ? m_pHotTask->m_rect : CRect (0, 0, 0, 0);

	m_pHotTask = NULL;
	m_pClickedTask = NULL;

	RedrawWindow (rectTask, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);

	if (bIsTaskClick)
	{
		DoTaskClick(pHotTask);
	}
}
//******************************************************************************************
void CBCGPTasksPane::OnCancelMode() 
{
	CBCGPDockingControlBar::OnCancelMode();
	
	if (m_pHotTask != NULL || m_pClickedTask != NULL)
	{
		CRect rectTask = m_pHotTask != NULL ? m_pHotTask->m_rect : CRect (0, 0, 0, 0);
		m_pHotTask = NULL;
		m_pClickedTask = NULL;
		ReleaseCapture ();
		RedrawWindow (rectTask, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
	}

	if (m_pHotGroupCaption != NULL || m_pClickedGroupCaption != NULL)
	{
		CRect rectTask = m_pHotGroupCaption != NULL ? 
			m_pHotGroupCaption->m_rect : CRect (0, 0, 0, 0);
		m_pHotGroupCaption = NULL;
		m_pClickedGroupCaption = NULL;
		ReleaseCapture ();
		RedrawWindow (rectTask, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
	}

	m_pClickedTask = NULL;
	m_pClickedGroupCaption = NULL;
}
//******************************************************************************************
void CBCGPTasksPane::OnClickTask (int /*nGroupNumber*/, int /*nTaskNumber*/, 
							 UINT uiCommandID, DWORD_PTR /*dwUserData*/)
{
	if (uiCommandID != 0)
	{
		GetOwner ()->PostMessage (WM_COMMAND, uiCommandID);
	}
}
//****************************************************************************************
void CBCGPTasksPane::OnLButtonDown(UINT nFlags, CPoint point) 
{
	m_pClickedGroupCaption = GroupCaptionHitTest (point);
	m_pClickedTask = TaskHitTest (point);

	if (m_pClickedTask != NULL && 
		    globalData.IsAccessibilitySupport ())
	{
		LPARAM lParam = (LPARAM)m_lAccUseCursorPosValue;
		globalData.NotifyWinEvent (EVENT_OBJECT_FOCUS, GetSafeHwnd (), 
					OBJID_CLIENT , (LONG)lParam);
	}

	CRect rectClient;
	GetClientRect (&rectClient);

	if (!rectClient.PtInRect(point) || m_rectToolbar.PtInRect(point))
	{
		CBCGPDockingControlBar::OnLButtonDown(nFlags, point);
	}
	else
	{
		SetFocus ();

		if (!IsToolBox ())
		{
			if (m_pClickedTask != NULL || m_pClickedGroupCaption != NULL)
			{
				OnChangeFocus (m_pClickedTask, m_pClickedGroupCaption);
			}
			else
			{
				OnFirst ();
			}
		}
	}
}
//****************************************************************************************

//-----------------------------------------------------
// My "classic " trick - how I can access to protected
// member m_pRecentFileList?
//-----------------------------------------------------
class CBCGApp : public CWinApp
{
	friend class CBCGPTasksPane;
};

int CBCGPTasksPane::AddMRUFilesList(int nGroup, int nMaxFiles /* = 4 */)
{
	POSITION pos = m_lstTaskGroups.FindIndex (nGroup);
	if (pos == NULL)
	{
		ASSERT (FALSE);
		return -1;
	}

	CBCGPTasksGroup* pGroup = (CBCGPTasksGroup*) m_lstTaskGroups.GetAt (pos);
	ASSERT_VALID (pGroup);

	POSITION posFirstMRUFile = NULL;

	// Clean up old MRU files from the group:
	for (pos = pGroup->m_lstTasks.GetHeadPosition (); pos != NULL;)
	{
		POSITION posSave = pos;

		CBCGPTask* pTask = (CBCGPTask*) pGroup->m_lstTasks.GetNext (pos);
		ASSERT_VALID (pTask);

		if (pTask->m_uiCommandID >= ID_FILE_MRU_FILE1 &&
			pTask->m_uiCommandID <= ID_FILE_MRU_FILE16)
		{
			posFirstMRUFile = posSave;
			pGroup->m_lstTasks.GetNext (posFirstMRUFile);

			delete pGroup->m_lstTasks.GetAt (posSave);
			pGroup->m_lstTasks.RemoveAt (posSave);
		}
	}

	CRecentFileList* pRecentFileList = 
		((CBCGApp*) AfxGetApp ())->m_pRecentFileList;

	if (pRecentFileList == NULL)
	{
		return (int) pGroup->m_lstTasks.GetCount () - 1;
	}

	int nNum = min (pRecentFileList->GetSize(), nMaxFiles);

	// Add new MRU files to the group:
	for (int i = 0; i < nNum; i++)
	{
		CString strName;
		if (GetMRUFileName (pRecentFileList, i, strName))
		{
			CBCGPTask* pTask = new CBCGPTask (pGroup, strName, -1, ID_FILE_MRU_FILE1 + i);
			ASSERT_VALID (pTask);

			if (posFirstMRUFile == NULL)
			{
				pGroup->m_lstTasks.AddTail (pTask);
			}
			else
			{
				pGroup->m_lstTasks.InsertBefore (posFirstMRUFile, pTask);
			}
		}
	}

	AdjustScroll ();
	ReposTasks ();
	return (int) pGroup->m_lstTasks.GetCount () - 1;
}
//*******************************************************************************************
BOOL CBCGPTasksPane::GetMRUFileName (CRecentFileList* pRecentFileList, int nIndex, CString &strName)
{
	ASSERT (pRecentFileList != NULL);

	if ((*pRecentFileList)[nIndex].GetLength() != 0)
	{
		const int MAX_NAME_LEN = 512;

		TCHAR lpcszBuffer [MAX_NAME_LEN + 1];
		memset(lpcszBuffer, 0, (MAX_NAME_LEN + 1) * sizeof(TCHAR));

		if (GetFileTitle((*pRecentFileList)[nIndex], lpcszBuffer, MAX_NAME_LEN) == 0)
		{
			strName = lpcszBuffer;
			return TRUE;
		}

		ASSERT(FALSE);
	}

	return FALSE;
}
//*******************************************************************************************
CScrollBar* CBCGPTasksPane::GetScrollBarCtrl(int nBar) const
{
	if (nBar == SB_HORZ || m_wndScrollVert.GetSafeHwnd () == NULL)
	{
		return NULL;
	}

	return (CScrollBar* ) &m_wndScrollVert;
}
//******************************************************************************************
void CBCGPTasksPane::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* /*pScrollBar*/) 
{
	int nPrevOffset = m_nVertScrollOffset;

	switch (nSBCode)
	{
	case SB_LINEUP:
		m_nVertScrollOffset--;
		break;

	case SB_LINEDOWN:
		m_nVertScrollOffset++;
		break;

	case SB_TOP:
		m_nVertScrollOffset = 0;
		break;

	case SB_BOTTOM:
		m_nVertScrollOffset = m_nVertScrollTotal;
		break;

	case SB_PAGEUP:
		m_nVertScrollOffset -= m_nVertScrollPage;
		break;

	case SB_PAGEDOWN:
		m_nVertScrollOffset += m_nVertScrollPage;
		break;

	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		m_nVertScrollOffset = nPos;
		break;

	default:
		return;
	}

	m_nVertScrollOffset = min (max (0, m_nVertScrollOffset), 
		m_nVertScrollTotal - m_nVertScrollPage + 1);

	if (m_nVertScrollOffset == nPrevOffset)
	{
		return;
	}

	SetScrollPos (SB_VERT, m_nVertScrollOffset);

	AdjustScroll ();
	ReposTasks ();
	RedrawWindow (NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
}
//*****************************************************************************************
void CBCGPTasksPane::SetScrollSizes ()
{
	ASSERT_VALID (this);

	if (m_wndScrollVert.GetSafeHwnd () == NULL)
	{
		return;
	}

	if (m_nRowHeight == 0)
	{
		m_nVertScrollPage = 0;
		m_nVertScrollTotal = 0;
		m_nVertScrollOffset = 0;
	}
	else
	{
		int nPageHeight = m_rectTasks.Height ();
		if (m_bUseScrollButtons)
		{
			nPageHeight -= m_iScrollBtnHeight + iBorderSize;
		}
		BOOL bMultiPage = (m_lstTasksPanes.GetCount () > 1);
		if ((m_bUseNavigationToolbar || ForceShowNavToolbar ()) && bMultiPage)
		{
			nPageHeight += m_rectToolbar.Height ();
		}

		m_nVertScrollPage = nPageHeight / m_nRowHeight - 1;

		int nTotalHeight = ReposTasks (TRUE);
		if (nTotalHeight == 0 || nTotalHeight <= nPageHeight)
		{
			m_nVertScrollPage = 0;
			m_nVertScrollTotal = 0;
			m_nVertScrollOffset = 0;
		}
		else
		{
			m_nVertScrollTotal = nTotalHeight / m_nRowHeight - 1;
		}

		m_nVertScrollOffset = min (max (0, m_nVertScrollOffset), 
			m_nVertScrollTotal - m_nVertScrollPage + 1);
	}

	if (!m_bUseScrollButtons)
	{
		SCROLLINFO si;

		ZeroMemory (&si, sizeof (SCROLLINFO));
		si.cbSize = sizeof (SCROLLINFO);

		si.fMask = SIF_RANGE | SIF_POS | SIF_PAGE;
		si.nMin = 0;
		si.nMax = m_nVertScrollTotal;
		si.nPage = m_nVertScrollPage;
		si.nPos = m_nVertScrollOffset;

		SetScrollInfo (SB_VERT, &si, TRUE);
	}
	m_wndScrollVert.EnableScrollBar (!m_bUseScrollButtons && m_nVertScrollTotal > 0 ? ESB_ENABLE_BOTH : ESB_DISABLE_BOTH);
}
//****************************************************************************************
void CBCGPTasksPane::AdjustScroll ()
{
	ASSERT_VALID (this);
	
	if (GetSafeHwnd () == NULL)
	{
		return;
	}

	CRect rectClient;
	GetClientRect (rectClient);

	if (IsToolBox ())
	{
		rectClient.DeflateRect (1, 1);
	}

	// --------------------------
	// Adjust navigation toolbar: 
	// --------------------------
	CRect rectToolbarOld = m_rectToolbar;
	BOOL bMultiPage = (m_lstTasksPanes.GetCount () > 1);
	if ((m_bUseNavigationToolbar || ForceShowNavToolbar ()) && bMultiPage)
	{
		int nToolbarHeight = m_wndToolBar.CalcFixedLayout (FALSE, TRUE).cy;

		m_rectToolbar = rectClient;
		m_rectToolbar.bottom = m_rectToolbar.top + nToolbarHeight;

		rectClient.top += m_rectToolbar.Height ();

		m_wndToolBar.SetWindowPos (NULL, m_rectToolbar.left, m_rectToolbar.top, 
									m_rectToolbar.Width (), nToolbarHeight,
									SWP_NOACTIVATE | SWP_NOZORDER);
		m_wndToolBar.ShowWindow (TRUE);
	}
	else
	{
		m_rectToolbar.SetRectEmpty ();
		m_wndToolBar.ShowWindow (FALSE);
	}

	// --------------------
	// Calculate work area:
	// --------------------
	m_rectTasks = rectClient;

	// ------------------
	// Adjust scroll bar:
	// ------------------
	SetScrollSizes ();

	m_wndScrollVert.EnableWindow (!m_bUseScrollButtons);
	if (!m_bUseScrollButtons && m_nVertScrollTotal > 0)
	{
		int cxScroll = ::GetSystemMetrics (SM_CXHSCROLL);

		m_rectTasks.right -= cxScroll;

		m_wndScrollVert.SetWindowPos (NULL, rectClient.right - cxScroll, rectClient.top,
			cxScroll, rectClient.Height (), SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOACTIVATE);
		rectClient.right -= cxScroll;
	}
	else
	{
		m_wndScrollVert.SetWindowPos (NULL, 0, 0,
			0, 0, SWP_HIDEWINDOW);
	}

	// ----------------------
	// Adjust scroll buttons:
	// ----------------------
	CRect rectScrollUpOld = m_rectScrollUp;
	CRect rectScrollDnOld = m_rectScrollDn;

	m_rectScrollUp.SetRectEmpty ();
	m_rectScrollDn.SetRectEmpty ();

	if (m_bUseScrollButtons)
	{
		if (IsScrollUpAvailable ())
		{
			m_rectScrollUp = rectClient;
			m_rectScrollUp.top += iBorderSize;
			m_rectScrollUp.bottom = m_rectScrollUp.top + m_iScrollBtnHeight;

			rectClient.top += m_iScrollBtnHeight + iBorderSize;
		}

		if (IsScrollDnAvailable ())
		{
			m_rectScrollDn = rectClient;
			m_rectScrollDn.top = m_rectScrollDn.bottom - m_iScrollBtnHeight;

			rectClient.bottom -= m_iScrollBtnHeight + iBorderSize;
		}

		m_rectTasks = rectClient;
	}
	else if (m_pAnimatedGroup != NULL/* animation is in progress */)
	{
		KillTimer (iScrollTimerId);
		m_iScrollMode = 0;
	}

	// ------------------------------
	// Invalidate navigation toolbar:
	// ------------------------------
	if (rectToolbarOld != m_rectToolbar)
	{
		InvalidateRect (m_rectToolbar);
		InvalidateRect (rectToolbarOld);
		UpdateWindow ();
	}

	// --------------------------
	// Invalidate scroll buttons:
	// --------------------------
	BOOL bScrollButtonsChanged = FALSE;

	if (rectScrollUpOld != m_rectScrollUp)
	{
		InvalidateRect (rectScrollUpOld);
		InvalidateRect (m_rectScrollUp);

		bScrollButtonsChanged = TRUE;
	}

	if (rectScrollDnOld != m_rectScrollDn)
	{
		InvalidateRect (rectScrollDnOld);
		InvalidateRect (m_rectScrollDn);

		bScrollButtonsChanged = TRUE;
	}

	if (bScrollButtonsChanged)
	{
		UpdateWindow ();
	}
}
//****************************************************************************************
void CBCGPTasksPane::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	DoPaint(&dc);
}
//****************************************************************************************
void CBCGPTasksPane::DoPaint(CDC* pDCPaint)
{
	CBCGPMemDC memDC (*pDCPaint, this);
	CDC* pDC = &memDC.GetDC ();

	CRect rect;
	GetClientRect (rect);

	CRect rectFrame = rect;
	rectFrame.SetRectEmpty ();

	if (IsToolBox ())
	{
		rectFrame = rect;
		rect.DeflateRect (1, 1);
	}

	OnDrawTasks (pDC, rect);

	if (!rectFrame.IsRectEmpty ())
	{
		CBCGPVisualManager::GetInstance ()->OnDrawToolBoxFrame (pDC, rectFrame);
	}
}
//***************************************************************************************
BOOL CBCGPTasksPane::OnEraseBkgnd(CDC* /*pDC*/) 
{
	return TRUE;
}
//****************************************************************************************
void CBCGPTasksPane::OnDestroy() 
{
	while (!m_lstTaskGroups.IsEmpty ())
	{
		delete m_lstTaskGroups.RemoveHead ();
	}

	CBCGPDockingControlBar::OnDestroy();
}
//****************************************************************************************
void CBCGPTasksPane::RebuildMenu ()
{
	ASSERT(m_iActivePage >= 0);
	ASSERT(m_iActivePage <= m_arrHistoryStack.GetUpperBound ());
	ASSERT(m_arrHistoryStack[m_iActivePage] >= 0);
	ASSERT(m_arrHistoryStack[m_iActivePage] <= m_lstTasksPanes.GetCount ()-1);

	if (m_menuOther.m_hMenu != NULL)
	{
		m_menuOther.DestroyMenu ();
	}

	HMENU hMenu = CreateMenu ();
	m_menuOther.Attach (hMenu);

	m_wndToolBar.UpdateButtons ();

	EnableButton (HTLEFTBUTTON_BCG, m_iActivePage > 0);
	EnableButton (HTRIGHTBUTTON_BCG, m_iActivePage < m_arrHistoryStack.GetUpperBound ());
}
//****************************************************************************************
void CBCGPTasksPane::SaveHistory (int nPageIdx)
{
	ASSERT(nPageIdx >= 0);
	ASSERT(nPageIdx <= m_lstTasksPanes.GetCount ()-1);

	ASSERT(m_iActivePage >= 0);
	ASSERT(m_iActivePage <= m_arrHistoryStack.GetUpperBound ());
	ASSERT(m_arrHistoryStack[m_iActivePage] >= 0);
	ASSERT(m_arrHistoryStack[m_iActivePage] <= m_lstTasksPanes.GetCount ()-1);
	
	if (nPageIdx == m_arrHistoryStack[m_iActivePage])
	{
		return;
	}

	if (m_iActivePage < m_arrHistoryStack.GetUpperBound ())
	{
		int nStackTailCount = (int) m_arrHistoryStack.GetUpperBound () - m_iActivePage;
		m_arrHistoryStack.RemoveAt (m_iActivePage+1, nStackTailCount);
	}
	if (m_arrHistoryStack.GetSize () == m_nMaxHistory)
	{
		m_arrHistoryStack.RemoveAt (0);
		if (m_iActivePage > 0)
		{
			m_iActivePage--;
		}
	}
	m_arrHistoryStack.Add (nPageIdx);
}
//****************************************************************************************
void CBCGPTasksPane::ChangeActivePage (int nNewPageHistoryIdx, int nOldPageHistoryIdx)
{
	ASSERT(nNewPageHistoryIdx >= 0);
	ASSERT(nNewPageHistoryIdx <= m_arrHistoryStack.GetUpperBound ());
	ASSERT(nOldPageHistoryIdx >= 0);
	ASSERT(nOldPageHistoryIdx <= m_arrHistoryStack.GetUpperBound ());

	int nNewPageIdx = m_arrHistoryStack[nNewPageHistoryIdx];
	int nOldPageIdx = m_arrHistoryStack[nOldPageHistoryIdx];

	ASSERT(nNewPageIdx >= 0);
	ASSERT(nNewPageIdx <= m_lstTasksPanes.GetCount ()-1);
	ASSERT(nOldPageIdx >= 0);
	ASSERT(nOldPageIdx <= m_lstTasksPanes.GetCount ()-1);

	if (nNewPageIdx == nOldPageIdx)
	{
		// Already active, do nothing
		return;
	}

	if (GetSafeHwnd () == NULL)
	{
		OnActivateTasksPanePage ();
		RebuildMenu ();	
		return;
	}

	// ------------------------------------------
	// Hide all windows for previous active page:
	// ------------------------------------------
	CBCGPTasksPanePage* pOldPage = NULL;
	POSITION posPage = m_lstTasksPanes.FindIndex (nOldPageIdx);
	ASSERT(posPage != NULL);
	
	pOldPage = (CBCGPTasksPanePage*) m_lstTasksPanes.GetAt (posPage);
	ASSERT_VALID(pOldPage);

	for (POSITION pos = m_lstTaskGroups.GetHeadPosition (); pos != NULL; )
	{
		CBCGPTasksGroup* pGroup = (CBCGPTasksGroup*) m_lstTaskGroups.GetNext (pos);
		ASSERT_VALID (pGroup);

		if (pGroup->m_pPage == pOldPage)
		{
			for (POSITION posTask = pGroup->m_lstTasks.GetHeadPosition (); posTask != NULL;)
			{
				CBCGPTask* pTask = (CBCGPTask*) pGroup->m_lstTasks.GetNext (posTask);
				ASSERT_VALID (pTask);

				if (pTask->m_hwndTask != NULL)
				{
					CWnd* pChildWnd = CWnd::FromHandle (pTask->m_hwndTask);
					ASSERT_VALID(pChildWnd);

					pChildWnd->ShowWindow (SW_HIDE);
				}
			}
		}
	}

	// ------------------
	// Update page title:
	// ------------------
	UpdateCaption ();

	// ------------------
	// Change active page
	// ------------------
	OnActivateTasksPanePage ();

	RebuildMenu ();
	
	m_nVertScrollOffset = 0;
	if (m_pFocusedTask != NULL || m_pFocusedGroupCaption != NULL)
	{
		OnFirst();
	}
	AdjustScroll ();
	ReposTasks ();
	
	Invalidate ();
	UpdateWindow ();
}
//****************************************************************************************
void CBCGPTasksPane::SetActivePage (int nPageIdx)
{
	ASSERT(nPageIdx >= 0);
	ASSERT(nPageIdx < m_lstTasksPanes.GetCount ());

	// ------------------------------------------
	// Activate the page specified by index
	// saving the current one in the history list
	// ------------------------------------------
	if (GetActivePage () != nPageIdx)
	{
		SaveHistory (nPageIdx);
		int nOldActivePage = m_iActivePage;
		m_iActivePage = (int) m_arrHistoryStack.GetUpperBound ();
		ChangeActivePage (m_iActivePage, nOldActivePage);
	}
}
//****************************************************************************************
void CBCGPTasksPane::SetCaptionButtons ()
{
	CBCGPDockingControlBar::SetCaptionButtons ();

	m_arrButtons.Add (new CBCGPCaptionButton (HTLEFTBUTTON_BCG, TRUE));
	m_arrButtons.Add (new CBCGPCaptionButton (HTRIGHTBUTTON_BCG, TRUE));
	
	CBCGPCaptionMenuButton *pBtn = new CBCGPCaptionMenuButton (HTMENU_BCG);
	pBtn->m_bOSMenu = FALSE;
	m_arrButtons.Add (pBtn);
}
//****************************************************************************************
BOOL CBCGPTasksPane::GetPageByGroup (int nGroup, int &nPage) const
{
	ASSERT (nGroup >= 0);
	ASSERT (nGroup < m_lstTaskGroups.GetCount ());

	CBCGPTasksGroup* pGroup = GetTaskGroup (nGroup);
	ASSERT_VALID (pGroup);

	int nPageCount = 0;
	for (POSITION posPage = m_lstTasksPanes.GetHeadPosition (); posPage != NULL; nPageCount++)
	{
		CBCGPTasksPanePage* pPage = (CBCGPTasksPanePage*) m_lstTasksPanes.GetNext (posPage);
		ASSERT_VALID (pPage);

		if (pPage == pGroup->m_pPage)
		{
			nPage = nPageCount;
			return TRUE;
		}
	}

	ASSERT (FALSE);
	return FALSE;
}
//****************************************************************************************
void CBCGPTasksPane::OnPressButtons (UINT nHit)
{
	CBCGPDockingControlBar::OnPressButtons (nHit);

	switch (nHit)
	{
	case HTLEFTBUTTON_BCG:
		OnPressBackButton ();
		break;

	case HTRIGHTBUTTON_BCG:
		OnPressForwardButton ();
		break;

	case HTMENU_BCG:
		{
			CBCGPCaptionMenuButton* pbtn = (CBCGPCaptionMenuButton*)FindButtonByHit (HTMENU_BCG);
			if (pbtn != NULL)
			{
				m_bMenuBtnPressed = TRUE;
				OnPressOtherButton (pbtn, this);
				m_bMenuBtnPressed = FALSE;
			}
		}
		break;
	}
}
//****************************************************************************************
void CBCGPTasksPane::OnPressBackButton ()
{
	// --------------------------
	// Handle Back caption button
	// --------------------------
	if (m_iActivePage > 0)
	{
		ASSERT (m_iActivePage >= 0);
		ASSERT (m_iActivePage <= m_arrHistoryStack.GetUpperBound ());

		int nOldActivePage = m_iActivePage;
		m_iActivePage--;
		ChangeActivePage (m_iActivePage, nOldActivePage);
	}
}
//****************************************************************************************
void CBCGPTasksPane::OnPressForwardButton ()
{
	// -----------------------------
	// Handle Forward caption button
	// -----------------------------
	if (m_iActivePage < m_arrHistoryStack.GetUpperBound ())
	{
		ASSERT (m_iActivePage >= 0);
		ASSERT (m_iActivePage <= m_arrHistoryStack.GetUpperBound ());

		int nOldActivePage = m_iActivePage;
		m_iActivePage++;
		ChangeActivePage (m_iActivePage, nOldActivePage);
	}
}
//****************************************************************************************
void CBCGPTasksPane::OnPressHomeButton ()
{
	ASSERT (m_iActivePage >= 0);
	ASSERT (m_iActivePage <= m_arrHistoryStack.GetUpperBound ());

	if (GetActivePage () != 0)
	{
		SetActivePage (0);
	}
}
//****************************************************************************************
void CBCGPTasksPane::OnPressOtherButton (CBCGPCaptionMenuButton* pbtn, CWnd* pWndOwner)
{
	ASSERT_VALID (pWndOwner);
	if (pbtn != NULL)
	{
		// ---------------------------
		// Handle Other caption button
		// ---------------------------
		ASSERT_VALID (pbtn);
		pbtn->ShowMenu (m_menuOther, pWndOwner);

		if (pbtn->m_nMenuResult != 0)
		{
			int nMenuIndex = CBCGPPopupMenuBar::GetLastCommandIndex ();
			if (nMenuIndex >= 0)
			{
				SetActivePage (nMenuIndex);
			}
		}
	}
}
//****************************************************************************************
void CBCGPTasksPane::DrawCaption (CDC* pDC, CRect rectCaption)
{
	ASSERT_VALID(pDC);

	// Enable or disable Taskpane specific caption buttons:
	BOOL bMultiPage = (m_lstTasksPanes.GetCount () > 1);
	for (int i = 0; i < m_arrButtons.GetSize (); i ++)
	{
		CBCGPCaptionButton* pbtn = m_arrButtons [i];
		ASSERT_VALID (pbtn);

		switch (pbtn->GetHit ())
		{
		case HTLEFTBUTTON_BCG:
		case HTRIGHTBUTTON_BCG:
		case HTMENU_BCG:
			pbtn->m_bHidden = pbtn->m_bHidden || !bMultiPage || m_bUseNavigationToolbar;
			break;
		}
		if (pbtn->GetHit () == HTLEFTBUTTON_BCG)
		{
			pbtn->m_bEnabled = IsBackButtonEnabled ();
		}
		if (pbtn->GetHit () == HTRIGHTBUTTON_BCG)
		{
			pbtn->m_bEnabled = IsForwardButtonEnabled ();
		}

	}
	
	UpdateTooltips ();

	CBCGPDockingControlBar::DrawCaption (pDC, rectCaption);
}
//****************************************************************************************
void CBCGPTasksPane::Serialize (CArchive& ar)
{
	CBCGPDockingControlBar::Serialize (ar);

	if (ar.IsLoading ())
	{
		if (g_pWorkspace != NULL && g_pWorkspace->GetDataVersion () >= 0x60210 && g_pWorkspace->GetDataVersion () != 0x70000)
		{
			// Load margin settings:
			ar >> m_nVertMargin;
			ar >> m_nHorzMargin;
			ar >> m_nGroupVertOffset;
			ar >> m_nGroupCaptionHeight;
			ar >> m_nGroupCaptionHorzOffset;
			ar >> m_nGroupCaptionVertOffset;
			ar >> m_nTasksHorzOffset;
			ar >> m_nTasksIconHorzOffset;
			ar >> m_nTasksIconVertOffset;

			// Load active page index:
			int nActivePage = 0;
			ar >> nActivePage;
			if (nActivePage < 0 || nActivePage >= m_lstTasksPanes.GetCount ())
			{
				nActivePage = 0;
			}
				
			// Load the titles of pages:
			CStringArray arrPagesNames;
			arrPagesNames.Serialize (ar);
			if (arrPagesNames.GetSize () == m_lstTasksPanes.GetCount ())
			{
				int i = 0;
				POSITION pos = m_lstTasksPanes.GetHeadPosition ();
				while (pos != NULL && i < arrPagesNames.GetSize ())
				{
					CBCGPTasksPanePage* pPage = (CBCGPTasksPanePage*) m_lstTasksPanes.GetNext (pos);
					ASSERT_VALID (pPage);

					pPage->m_strName = arrPagesNames[i++];
				}
			}

			// Change active page:
			SetActivePage (nActivePage);

			m_nVertScrollOffset = 0;
			AdjustScroll ();
		}

		if (g_pWorkspace != NULL && g_pWorkspace->GetDataVersion () >= 0x60500 && g_pWorkspace->GetDataVersion () != 0x70000)
		{
			// Load taskpane's caption:
			ar >> m_strCaption;
			UpdateCaption ();
		}
	}
	else
	{
		// Save margin settings:
		ar << m_nVertMargin;
		ar << m_nHorzMargin;
		ar << m_nGroupVertOffset;
		ar << m_nGroupCaptionHeight;
		ar << m_nGroupCaptionHorzOffset;
		ar << m_nGroupCaptionVertOffset;
		ar << m_nTasksHorzOffset;
		ar << m_nTasksIconHorzOffset;
		ar << m_nTasksIconVertOffset;

		// Save active page index:
		ar << GetActivePage ();

		// Save the titles of pages:
		CStringArray arrPagesNames;
		for (POSITION pos = m_lstTasksPanes.GetHeadPosition (); pos != NULL;)
		{
			CBCGPTasksPanePage* pPage = (CBCGPTasksPanePage*) m_lstTasksPanes.GetNext (pos);
			ASSERT_VALID (pPage);

			arrPagesNames.Add (pPage->m_strName);
		}

		arrPagesNames.Serialize (ar);

		// Save taskpane's caption:
		ar << m_strCaption;
	}
}
//****************************************************************************************
void CBCGPTasksPane::SetCaption (LPCTSTR lpszName)
{
	ASSERT (lpszName != NULL);

	m_strCaption = lpszName;
	SetWindowText (lpszName);

	UpdateCaption ();
}
//****************************************************************************************
void CBCGPTasksPane::SetPageCaption (int nPageIdx, LPCTSTR lpszName)
{
	ASSERT (nPageIdx >= 0);
	ASSERT (nPageIdx < m_lstTasksPanes.GetCount ());
	ASSERT (lpszName != NULL);

	POSITION pos = m_lstTasksPanes.FindIndex (nPageIdx);
	ASSERT (pos != NULL);
	CBCGPTasksPanePage* pPage = (CBCGPTasksPanePage*)m_lstTasksPanes.GetAt (pos);
	ASSERT_VALID (pPage);

	pPage->m_strName = lpszName;

	UpdateCaption ();
}
//****************************************************************************************
BOOL CBCGPTasksPane::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message == WM_LBUTTONDBLCLK)
	{
		int nGroup, nTask;
		if (GetTaskLocation ((HWND)pMsg->hwnd, nGroup, nTask))
		{
			return CBCGPDockingControlBar::PreTranslateMessage(pMsg);
		}
		
		CPoint point(BCG_GET_X_LPARAM(pMsg->lParam), BCG_GET_Y_LPARAM(pMsg->lParam));

		CRect rectClient;
		GetClientRect (&rectClient);

		if (IsToolBox ())
		{
			rectClient.DeflateRect (1, 1);
		}

		if (rectClient.PtInRect(point))
		{
			return TRUE;
		}
	}

	if (pMsg->message == WM_KEYDOWN)
	{
		if ((UINT)pMsg->wParam != VK_TAB)
		{
			int nGroup, nTask;
			if (GetTaskLocation ((HWND)pMsg->hwnd, nGroup, nTask))
			{
				return CBCGPDockingControlBar::PreTranslateMessage(pMsg);
			}
		}

		// If popup menu is active, pass keyboard control to menu:
		if (CBCGPPopupMenu::GetActiveMenu () != NULL &&
			::IsWindow (CBCGPPopupMenu::GetActiveMenu ()->m_hWnd))
		{
			if (CBCGPDockingControlBar::PreTranslateMessage(pMsg))
			{
				return TRUE;
			}
		}

		if (ProcessKeyboard((UINT)pMsg->wParam))
		{
			return TRUE;
		}
	}

	return CBCGPDockingControlBar::PreTranslateMessage(pMsg);
}
//****************************************************************************************
CBCGPMiniFrameWnd* CBCGPTasksPane::CreateDefaultMiniframe (CRect rectInitial)
{
	ASSERT_VALID (this);

	if (GetStyle () & CBRS_FLOAT_MULTI)
	{
		m_pMiniFrameRTC = RUNTIME_CLASS (CBCGPMultiMiniFrameWnd);
	}
	else
	{
		m_pMiniFrameRTC = RUNTIME_CLASS (CBCGPTaskPaneMiniFrameWnd);
	}

	return CBCGPControlBar::CreateDefaultMiniframe (rectInitial);
}
//****************************************************************************************
void CBCGPTasksPane::OnTrackCaptionButtons (CPoint point)
{
	if (!m_bMenuBtnPressed)
	{
		CBCGPDockingControlBar::OnTrackCaptionButtons (point);
	}
}
//****************************************************************************************
void CBCGPTasksPane::StopCaptionButtonsTracking ()
{
	if (!m_bMenuBtnPressed)
	{
		CBCGPDockingControlBar::StopCaptionButtonsTracking ();
	}
}
//****************************************************************************************
BOOL CBCGPTasksPane::OnNeedTipText(UINT id, NMHDR* pNMH, LRESULT* pResult)
{
	static CString strTipText;

	if (m_pToolTip->GetSafeHwnd () == NULL || 
		pNMH->hwndFrom != m_pToolTip->GetSafeHwnd ())
	{
		return FALSE;
	}

	LPNMTTDISPINFO	pTTDispInfo	= (LPNMTTDISPINFO) pNMH;
	ASSERT((pTTDispInfo->uFlags & TTF_IDISHWND) == 0);

	if (pNMH->idFrom == CBCGSIZINGCONTROLBAR_BUTTONS_NUM + 1)
	{
		CBCGPLocalResource locaRes;
		strTipText.LoadString (ID_BCGBARRES_TASKPANE_BACK);

		pTTDispInfo->lpszText = const_cast<LPTSTR> ((LPCTSTR) strTipText);
		return TRUE;
	}
	else if (pNMH->idFrom == CBCGSIZINGCONTROLBAR_BUTTONS_NUM + 2)
	{
		CBCGPLocalResource locaRes;
		strTipText.LoadString (ID_BCGBARRES_TASKPANE_FORWARD);

		pTTDispInfo->lpszText = const_cast<LPTSTR> ((LPCTSTR) strTipText);
		return TRUE;
	}
	else if (pNMH->idFrom == CBCGSIZINGCONTROLBAR_BUTTONS_NUM + 3)
	{
		CBCGPLocalResource locaRes;
		strTipText.LoadString (ID_BCGBARRES_TASKPANE_OTHER);

		pTTDispInfo->lpszText = const_cast<LPTSTR> ((LPCTSTR) strTipText);
		return TRUE;
	}

	return CBCGPDockingControlBar::OnNeedTipText (id, pNMH, pResult);
}
//****************************************************************************************
void CBCGPTasksPane::OnOK()
{
	if (IsToolBox ())
	{
		return;
	}

	CWnd* pFocusWnd = CWnd::GetFocus();

	// Send the IDOK command to the focused task window
	if (pFocusWnd != NULL && IsChild(pFocusWnd))
	{
		while (pFocusWnd != this)
		{
			CWnd* pParentWnd = pFocusWnd->GetParent();

			if (pParentWnd == this)
			{
				const MSG* pMsg = GetCurrentMessage();
				pFocusWnd->SendMessage(WM_COMMAND, pMsg->wParam, pMsg->lParam);
				break;
			}

			pFocusWnd = pFocusWnd->GetParent();
		}
	}
}
//****************************************************************************************
void CBCGPTasksPane::OnCancel()
{
	if (IsToolBox ())
	{
		return;
	}

	CWnd* pFocusWnd = CWnd::GetFocus();

	// Send the IDCANCEL command to the focused task window
	if (pFocusWnd != NULL && IsChild(pFocusWnd))
	{
		while (pFocusWnd != this)
		{
			CWnd* pParentWnd = pFocusWnd->GetParent();

			if (pParentWnd == this)
			{
				const MSG* pMsg = GetCurrentMessage();
				pFocusWnd->SendMessage(WM_COMMAND, pMsg->wParam, pMsg->lParam);
				break;
			}

			pFocusWnd = pFocusWnd->GetParent();
		}
	}

	// Set the focus to the application window
	CFrameWnd* pTopFrame = ::BCGCBProGetTopLevelFrame (this);

	if (::IsWindow (pTopFrame->GetSafeHwnd ()))
	{
		pTopFrame->SetFocus();
	}
}
//****************************************************************************************
BOOL CBCGPTasksPane::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
	if (CBCGPPopupMenu::GetActiveMenu () != NULL)
	{
		return TRUE;
	}

	if (m_nVertScrollTotal > 0)
	{
		int	iRemainingDelta	= abs(zDelta);

		while (iRemainingDelta != 0)
		{
			if (zDelta > 0)
				OnVScroll(SB_LINEUP, 0, &m_wndScrollVert);
			else
				OnVScroll(SB_LINEDOWN, 0, &m_wndScrollVert);

			iRemainingDelta	-= min(WHEEL_DELTA, iRemainingDelta);

			VERIFY(RedrawWindow(NULL, NULL, RDW_ALLCHILDREN | RDW_UPDATENOW));
		}

		if (g_pWorkspace != NULL && g_pWorkspace->IsMouseWheelInInactiveWindowEnabled())
		{
			return TRUE;
		}
	}
	
	return CBCGPDockingControlBar::OnMouseWheel(nFlags, zDelta, pt);
}
//****************************************************************************************
BOOL CBCGPTasksPane::SaveState (LPCTSTR lpszProfileName, int nIndex, UINT uiID)
{
	CString strProfileName = ::BCGPGetRegPath (strTasksPaneProfile, lpszProfileName);

	BOOL bResult = FALSE;

	if (nIndex == -1)
	{
		nIndex = GetDlgCtrlID ();
	}

	CString strSection;
	if (uiID == (UINT) -1)
	{
		strSection.Format (REG_SECTION_FMT, strProfileName, nIndex);
	}
	else
	{
		strSection.Format (REG_SECTION_FMT_EX, strProfileName, nIndex, uiID);
	}

	try
	{
		CMemFile file;

		{
			CArchive ar (&file, CArchive::store);

			Serialize (ar);
			ar.Flush ();
		}

		UINT uiDataSize = (UINT) file.GetLength ();
		LPBYTE lpbData = file.Detach ();

		if (lpbData != NULL)
		{
			CBCGPRegistrySP regSP;
			CBCGPRegistry& reg = regSP.Create (FALSE, FALSE);

			if (reg.CreateKey (strSection))
			{
				bResult = reg.Write (REG_ENTRY_SETTINGS, lpbData, uiDataSize);
			}

			free (lpbData);
		}
	}
	catch (CMemoryException* pEx)
	{
		pEx->Delete ();
		TRACE(_T("Memory exception in CBCGPToolBar::SaveState ()!\n"));
	}

	bResult = CBCGPDockingControlBar::SaveState (lpszProfileName, nIndex, uiID);

	return bResult;
}
//****************************************************************************************
BOOL CBCGPTasksPane::LoadState (LPCTSTR lpszProfileName, int nIndex, UINT uiID)
{
	CString strProfileName = ::BCGPGetRegPath (strTasksPaneProfile, lpszProfileName);

	BOOL bResult = FALSE;

	if (nIndex == -1)
	{
		nIndex = GetDlgCtrlID ();
	}

	CString strSection;
	if (uiID == (UINT) -1)
	{
		strSection.Format (REG_SECTION_FMT, strProfileName, nIndex);
	}
	else
	{
		strSection.Format (REG_SECTION_FMT_EX, strProfileName, nIndex, uiID);
	}

	LPBYTE	lpbData = NULL;
	UINT	uiDataSize;

	CBCGPRegistrySP regSP;
	CBCGPRegistry& reg = regSP.Create (FALSE, TRUE);

	if (!reg.Open (strSection))
	{
		return FALSE;
	}

	if (!reg.Read (REG_ENTRY_SETTINGS, &lpbData, &uiDataSize))
	{
		return FALSE;
	}

	try
	{
		CMemFile file (lpbData, uiDataSize);
		CArchive ar (&file, CArchive::load);

		Serialize (ar);
		bResult = TRUE;
	}
	catch (CMemoryException* pEx)
	{
		pEx->Delete ();
		TRACE(_T("Memory exception in CBCGPTasksPane::LoadState ()!\n"));
	}
	catch (CArchiveException* pEx)
	{
		pEx->Delete ();
		TRACE(_T("CArchiveException exception in CBCGPTasksPane::LoadState ()!\n"));
	}

	if (lpbData != NULL)
	{
		delete [] lpbData;
	}

	bResult = CBCGPDockingControlBar::LoadState (lpszProfileName, nIndex, uiID);

	return bResult;
}
//****************************************************************************************
void CBCGPTasksPane::OnTimer(UINT_PTR nIDEvent) 
{
	switch (nIDEvent)
	{
	case iAnimTimerId:
		if (m_pAnimatedGroup != NULL && m_nRowHeight != 0)
		{
			ASSERT_VALID (m_pAnimatedGroup);

			clock_t nCurrAnimTime = clock ();

			int nDuration = nCurrAnimTime - m_nLastAnimTime;
			int nSteps = (int) (.5 + (float) nDuration / m_iAnimTimerDuration);

			// speed up animation
			const int MAX_ANIMATIONSTEPS_NUM = 9;
			int nAnimatedGroupHeight = m_sizeAnim.cy + m_nAnimGroupExtraHeight;
			int nStepsTotal = (int) (.5 + (float) nAnimatedGroupHeight / m_nRowHeight);
			if (nStepsTotal > MAX_ANIMATIONSTEPS_NUM)
			{
				nSteps = 1 + nSteps * nStepsTotal / MAX_ANIMATIONSTEPS_NUM;
			}			

			if (m_pAnimatedGroup->m_bIsCollapsed) // collapsing
			{
				m_sizeAnim.cy -= nSteps * m_nRowHeight;
			}
			else // expanding
			{
				m_sizeAnim.cy += nSteps * m_nRowHeight;
			}

			CRect rectUpdate = m_rectTasks;
			rectUpdate.top = m_pAnimatedGroup->m_rect.top - 1;
			int nSaveTop = rectUpdate.top;
			InvalidateRect (rectUpdate);

			RedrawWindow (NULL, NULL, RDW_ERASE | RDW_ALLCHILDREN | RDW_UPDATENOW);

			ReposTasks ();

			rectUpdate = m_rectTasks;
			rectUpdate.top = min (m_pAnimatedGroup->m_rect.top - 1, nSaveTop);
			InvalidateRect (rectUpdate);

			RedrawWindow (NULL, NULL, RDW_ERASE);

			// stop rule:
			if (m_pAnimatedGroup->m_bIsCollapsed && m_sizeAnim.cy < 0 ||
				!m_pAnimatedGroup->m_bIsCollapsed && m_sizeAnim.cy > m_pAnimatedGroup->m_rectGroup.Height ())
			{
				m_pAnimatedGroup = NULL;
				m_sizeAnim = CSize (0, 0);
			}

			m_nLastAnimTime = nCurrAnimTime;
		}
		else
		{
			CBCGPTasksGroup* pSaveGroup = m_pAnimatedGroup;

			KillTimer (iAnimTimerId);
			m_pAnimatedGroup = NULL;

			OnEndAnimation (pSaveGroup);
		}
		break;

	case iScrollTimerId:
		{
			CPoint point;
			::GetCursorPos (&point);
			ScreenToClient (&point);

			if (m_rectScrollUp.PtInRect (point) && m_iScrollMode < 0)	// Scroll Up
			{
				m_nVertScrollOffset--;

				AdjustScroll ();
				ReposTasks ();
				RedrawWindow (NULL, NULL, RDW_INVALIDATE | RDW_ERASE);
			}
			else if (m_rectScrollDn.PtInRect (point) && m_iScrollMode > 0)	// Scroll Down
			{
				m_nVertScrollOffset++;

				AdjustScroll ();
				ReposTasks ();
				RedrawWindow (NULL, NULL, RDW_INVALIDATE | RDW_ERASE);
			}
			else
			{
				KillTimer (iScrollTimerId);
				m_iScrollMode = 0;
				InvalidateRect (m_rectScrollDn);
				InvalidateRect (m_rectScrollUp);
				UpdateWindow ();
			}
		}
		break;
	}
	
	CBCGPDockingControlBar::OnTimer(nIDEvent);
}
//*************************************************************************************
void CBCGPTasksPane::OnSetFocus(CWnd* pOldWnd) 
{
	CBCGPDockingControlBar::OnSetFocus(pOldWnd);

	UpdateFocusedItems ();
}
//*************************************************************************************
void CBCGPTasksPane::OnKillFocus(CWnd* pNewWnd) 
{
	CBCGPDockingControlBar::OnKillFocus(pNewWnd);

	UpdateFocusedItems ();
}
//*************************************************************************************
void CBCGPTasksPane::RecalcLayout (BOOL bRedraw/* = TRUE*/)
{
	if (GetSafeHwnd () == NULL)
	{
		return;
	}

	AdjustScroll ();
	ReposTasks ();

	if (bRedraw)
	{
		RedrawWindow (NULL, NULL, RDW_ALLCHILDREN | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPTasksPane idle update through CBCGPTaskCmdUI class

class CBCGPTaskCmdUI : public CCmdUI        // class private to this file !
{
	// m_nIndex - taskgroup index
	// m_pOther - taskspane pointer

public: // re-implementations only
	virtual void Enable(BOOL bOn);
	virtual void SetCheck(int /*nCheck*/) {}		// ignore
	virtual void SetRadio(BOOL /*bOn*/ = TRUE) {}	// ignore
	virtual void SetText(LPCTSTR /*lpszText*/);
};

void CBCGPTaskCmdUI::Enable(BOOL bOn)
{
	m_bEnableChanged = TRUE;
	CBCGPTasksPane* pTasksPane = (CBCGPTasksPane*)m_pOther;
	ASSERT(pTasksPane != NULL);
	ASSERT_KINDOF(CBCGPTasksPane, pTasksPane);
	ASSERT(m_nIndex < m_nIndexMax);

	// Enable all tasks with uiCommandID in the taskgroup:
	CBCGPTasksGroup* pGroup = pTasksPane->GetTaskGroup (m_nIndex);
	if (pGroup == NULL)
	{
		return;
	}

	for (POSITION posTask = pGroup->m_lstTasks.GetHeadPosition (); posTask != NULL;)
	{
		CBCGPTask* pTask = (CBCGPTask*) pGroup->m_lstTasks.GetNext (posTask);
		ASSERT_VALID (pTask);

		if (pTask->m_uiCommandID == m_nID)
		{
			if (pTask->m_bEnabled != bOn)
			{
				pTask->m_bEnabled = bOn;
				pTasksPane->InvalidateRect (pTask->m_rect);

				if (pTask->m_hwndTask != NULL)
				{
					CWnd* pChildWnd = CWnd::FromHandle (pTask->m_hwndTask);
					ASSERT_VALID(pChildWnd);

					pChildWnd->EnableWindow (bOn);
				}
			}
		}
	}
}
//*************************************************************************************
void CBCGPTaskCmdUI::SetText (LPCTSTR lpszText)
{
	ASSERT (lpszText != NULL);

	CBCGPTasksPane* pTasksPane = (CBCGPTasksPane*)m_pOther;
	ASSERT(pTasksPane != NULL);
	ASSERT_KINDOF(CBCGPTasksPane, pTasksPane);
	ASSERT(m_nIndex < m_nIndexMax);

	//Remove any amperstands and trailing label (ex.:"\tCtrl+S")
	CString strNewText(lpszText);

	int iOffset = strNewText.Find (_T('\t'));
	if (iOffset != -1)
	{
		strNewText = strNewText.Left (iOffset);
	}

	// Set name for all tasks with uiCommandID in the taskgroup:
	CBCGPTasksGroup* pGroup = pTasksPane->GetTaskGroup (m_nIndex);
	if (pGroup == NULL)
	{
		return;
	}

	for (POSITION posTask = pGroup->m_lstTasks.GetHeadPosition (); posTask != NULL;)
	{
		CBCGPTask* pTask = (CBCGPTask*) pGroup->m_lstTasks.GetNext (posTask);
		ASSERT_VALID (pTask);

		if (pTask->m_uiCommandID == m_nID)
		{
			if (pTask->m_strName != strNewText)		
			{
				pTask->m_strName = strNewText;

				pTasksPane->InvalidateRect (pTask->m_rect);
			}
		}
	}
}
//*************************************************************************************
void CBCGPTasksPane::OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler)
{
	CBCGPTaskCmdUI state;
	state.m_pOther = this;

	// update all tasks:
	state.m_nIndexMax = (UINT)GetGroupCount ();
	state.m_nIndex = 0;
	for (POSITION posGroup = m_lstTaskGroups.GetHeadPosition (); posGroup != NULL; state.m_nIndex++)
	{
		CBCGPTasksGroup* pGroup = (CBCGPTasksGroup*) m_lstTaskGroups.GetNext (posGroup);
		ASSERT_VALID (pGroup);

		for (POSITION posTask = pGroup->m_lstTasks.GetHeadPosition (); posTask != NULL;)
		{
			CBCGPTask* pTask = (CBCGPTask*) pGroup->m_lstTasks.GetNext (posTask);
			ASSERT_VALID (pTask);

			if (g_pUserToolsManager != NULL &&
				g_pUserToolsManager->IsUserToolCmd (pTask->m_uiCommandID))
			{
				bDisableIfNoHndler = FALSE;
			}

			//state.m_nIndex == taskgroup index
			state.m_nID = pTask->m_uiCommandID;

			// ignore separators and system commands
			if (pTask->m_uiCommandID != 0 &&
				!IsSystemCommand (pTask->m_uiCommandID) &&
				pTask->m_uiCommandID < AFX_IDM_FIRST_MDICHILD)
			{
				// check for handlers in the target (owner)
				state.DoUpdate(pTarget, bDisableIfNoHndler);
			}
		}

	}

	BOOL bActiveOld = m_bActive;

	CBCGPDockingControlBar::OnUpdateCmdUI (pTarget, bDisableIfNoHndler);

	if (m_bActive != bActiveOld)
	{
		UpdateFocusedItems ();
	}
}
//*************************************************************************************
void CBCGPTasksPane::ShowCommandMessageString (UINT uiCmdId)
{
	GetOwner()->SendMessage (WM_SETMESSAGESTRING,
		uiCmdId == (UINT) -1 ? AFX_IDS_IDLEMESSAGE : (WPARAM) uiCmdId);
}
//*************************************************************************************
BOOL CBCGPTasksPane::CreateNavigationToolbar ()
{
	if (GetSafeHwnd () == NULL)
	{
		return FALSE;
	}

	// ---------------
	// Create toolbar:
	// ---------------
	if (!m_wndToolBar.Create (this, dwDefaultToolbarStyle | CBRS_TOOLTIPS | CBRS_FLYBY, iNavToolbarId))
	{
		return FALSE;
	}
	m_wndToolBar.SetBarStyle (m_wndToolBar.GetBarStyle () & ~CBRS_GRIPPER);
	
	m_wndToolBar.SetOwner (this);

	// All commands will be routed via this bar, not via the parent frame:
	m_wndToolBar.SetRouteCommandsViaFrame (FALSE);

	CSize sizeNavImage = globalData.Is32BitIcons () ? CSize (16, 16) : CSize (12, 12);
	const int nImageMargin = 4;

	CSize sizeNavButton = sizeNavImage + CSize (nImageMargin, nImageMargin);

	const double dblImageScale = globalData.GetRibbonImageScale ();

	// -----------------------
	// Load navigation images:
	// -----------------------
	if (m_uiToolbarBmpRes == 0)
	{
		//----------------------
		// Use default resource:
		//----------------------

		if (dblImageScale != 1.)
		{
			sizeNavButton = CSize ((int)(.5 + dblImageScale * sizeNavButton.cx), (int)(.5 + dblImageScale * sizeNavButton.cy));
		}

		m_wndToolBar.SetLockedSizes (sizeNavButton, sizeNavImage);

		CBCGPLocalResource	lr;
		if (!m_wndToolBar.LoadBitmap (
			globalData.Is32BitIcons () ? IDB_BCGBARRES_TASKPANE32 : IDB_BCGBARRES_TASKPANE, 
			0, 0, TRUE))
		{
			ASSERT(FALSE);
		}
	}
	else
	{
		if (m_sizeToolbarImage != CSize (0, 0))
		{
			sizeNavImage = m_sizeToolbarImage;

			if (m_sizeToolbarButton != CSize (0, 0))
			{
				sizeNavButton = m_sizeToolbarButton;
			}
			else
			{
				sizeNavButton = sizeNavImage + CSize (nImageMargin, nImageMargin);
			}
		}

		if (dblImageScale != 1.)
		{
			sizeNavButton = CSize ((int)(.5 + dblImageScale * sizeNavButton.cx), (int)(.5 + dblImageScale * sizeNavButton.cy));
		}

		m_wndToolBar.SetLockedSizes (sizeNavButton, sizeNavImage);

		if (!m_wndToolBar.LoadBitmap (m_uiToolbarBmpRes, 0, 0, TRUE))
		{
			ASSERT(FALSE);
		}
	}

	//-----------------------------
	// Load Task Pane text strings:
	//-----------------------------
	CString strBack;
	CString strForward;
	CString strHome;
	CString strClose;

	{
		CBCGPLocalResource locaRes;
		strBack.LoadString (ID_BCGBARRES_TASKPANE_BACK);
		strForward.LoadString (ID_BCGBARRES_TASKPANE_FORWARD);
		strHome.LoadString (ID_BCGBARRES_TASKPANE_HOME);
		strClose.LoadString (ID_BCGBARRES_TASKPANE_CLOSE);
	}

	// --------------------
	// Add toolbar buttons:
	// --------------------
	m_wndToolBar.RemoveAllButtons ();

	if (m_bHistoryMenuButtons)
	{
		// Create drop-down menubutton for the "Back" button:
		CBCGPTasksPaneHistoryButton* pBtnBack = new CBCGPTasksPaneHistoryButton (
												ID_BCGBARRES_TASKPANE_BACK, 0, strBack);
		m_wndToolBar.m_pBtnBack = pBtnBack;

		if (pBtnBack != NULL)
		{
			m_wndToolBar.InsertButton (pBtnBack);
			pBtnBack->SetMessageWnd (this);
			pBtnBack->OnChangeParentWnd (this);
			pBtnBack->m_bDrawDownArrow = TRUE;
		}

		// Create drop-down menubutton for the "Forward" button:
		CBCGPTasksPaneHistoryButton* pBtnForward = new CBCGPTasksPaneHistoryButton (
												ID_BCGBARRES_TASKPANE_FORWARD, 1, strForward);
		m_wndToolBar.m_pBtnForward = pBtnForward;

		if (pBtnForward != NULL)
		{
			m_wndToolBar.InsertButton (pBtnForward);
			pBtnForward->SetMessageWnd (this);
			pBtnForward->OnChangeParentWnd (this);
			pBtnForward->m_bDrawDownArrow = TRUE;
		}
	}
	else
	{
		m_wndToolBar.InsertButton (new CBCGPTasksPaneNavigateButton (ID_BCGBARRES_TASKPANE_BACK, 0, strBack));
		m_wndToolBar.InsertButton (new CBCGPTasksPaneNavigateButton (ID_BCGBARRES_TASKPANE_FORWARD, 1, strForward));
	}

	m_wndToolBar.InsertButton (new CBCGPTasksPaneNavigateButton (ID_BCGBARRES_TASKPANE_HOME, 2, strHome));

	m_wndToolBar.InsertSeparator ();

	CBCGPTasksPaneMenuButton* pButton = new CBCGPTasksPaneMenuButton (m_menuOther.GetSafeHmenu ());

	if (pButton != NULL)
	{
		m_wndToolBar.InsertButton (pButton);
		pButton->m_bText = TRUE;
		pButton->m_bImage = FALSE;
		pButton->m_bLocked = TRUE;
		pButton->m_strText = _T("Tasks Pane");;
		pButton->SetMessageWnd (this);
		pButton->OnChangeParentWnd (this);
	}
	
	m_wndToolBar.InsertButton (new CBCGPTasksPaneNavigateButton (ID_BCGBARRES_TASKPANE_CLOSE, 3, strClose));

	return TRUE;
}
//*************************************************************************************
void CBCGPTasksPane::OnBack ()
{
	if (m_bHistoryMenuButtons)
	{
		// Get index of the clicked history page
		int iPage = -1;

		if (m_wndToolBar.m_pBtnBack != NULL)
		{
			ASSERT_VALID (m_wndToolBar.m_pBtnBack);
			if (m_wndToolBar.m_pBtnBack->IsClickedOnMenu ())
			{
				iPage = CBCGPPopupMenuBar::GetLastCommandIndex ();
			}
		}

		// Go back
		int nPrevPagesCount = m_iActivePage;
		if (iPage >= 0 && iPage < nPrevPagesCount)
		{
			int nOldActivePage = m_iActivePage;
			m_iActivePage -= iPage + 1;
			ChangeActivePage (m_iActivePage, nOldActivePage);
			
			return;
		}
	}

	OnPressBackButton ();
}
//*************************************************************************************
void CBCGPTasksPane::OnForward ()
{
	if (m_bHistoryMenuButtons)
	{
		// Get index of the clicked history page
		int iPage = -1;

		if (m_wndToolBar.m_pBtnForward != NULL)
		{
			ASSERT_VALID (m_wndToolBar.m_pBtnForward);
			if (m_wndToolBar.m_pBtnForward->IsClickedOnMenu ())
			{
				iPage = CBCGPPopupMenuBar::GetLastCommandIndex ();
			}
		}

		// Go forward
		int nNextPagesCount = (int) m_arrHistoryStack.GetUpperBound () - m_iActivePage;
		if (iPage >= 0 && iPage < nNextPagesCount)
		{
			int nOldActivePage = m_iActivePage;
			m_iActivePage += iPage + 1;
			ChangeActivePage (m_iActivePage, nOldActivePage);

			return;
		}
	}

	OnPressForwardButton ();
}
//*************************************************************************************
void CBCGPTasksPane::OnHome ()
{
	OnPressHomeButton ();
}
//*************************************************************************************
void CBCGPTasksPane::OnClose ()
{
	OnPressCloseButton ();
}
//*************************************************************************************
void CBCGPTasksPane::OnOther()
{
	// ------------------------------------
	// Handle "Other Task Pane" menubutton:
	// ------------------------------------
	int iPage = CBCGPPopupMenuBar::GetLastCommandIndex ();

	ASSERT (iPage >= 0);
	ASSERT (iPage < GetPagesCount());

	SetActivePage (iPage);
}
//*************************************************************************************
void CBCGPTasksPane::OnUpdateBack (CCmdUI* pCmdUI)
{
	pCmdUI->Enable (IsBackButtonEnabled ());
}
//*************************************************************************************
void CBCGPTasksPane::OnUpdateForward (CCmdUI* pCmdUI)
{
	pCmdUI->Enable (IsForwardButtonEnabled ());
}
//*************************************************************************************
void CBCGPTasksPane::OnUpdateClose (CCmdUI* pCmdUI)
{
	CBCGPTasksPane*	pParentBar = DYNAMIC_DOWNCAST (CBCGPTasksPane, FromHandle (m_hWndOwner));
	if (pParentBar != NULL)
	{
		pCmdUI->Enable (pParentBar->CanBeClosed ());	
	}
}
//*************************************************************************************
HMENU CBCGPTasksPane::CreateMenu () const
{
	// ------------------------------------------------
	// Create popup menu with a list of taskpane pages:
	// ------------------------------------------------
	CMenu menu;
	menu.CreatePopupMenu ();

	for (POSITION pos = m_lstTasksPanes.GetHeadPosition (); pos != NULL;)
	{
		CBCGPTasksPanePage* pPage = (CBCGPTasksPanePage*) m_lstTasksPanes.GetNext (pos);
		ASSERT_VALID (pPage);
		
		menu.AppendMenu (MF_STRING, ID_BCGBARRES_TASKPANE_OTHER, pPage->m_strName);
	}

	HMENU hMenu = menu.Detach ();

	// ------------------------------------
	// Check menu item for the active page:
	// ------------------------------------
	if (hMenu != NULL)
	{
		int iPage = GetActivePage ();
		::CheckMenuItem (hMenu, iPage, MF_BYPOSITION | MF_CHECKED);
	}

	return hMenu;
}
//********************************************************************************
void CBCGPTasksPane::EnableScrollButtons (BOOL bEnable)
{
	m_bUseScrollButtons = bEnable;

	if (!m_bUseScrollButtons && GetSafeHwnd() != NULL)
	{
		CBCGPGestureConfig gestureConfig;
		gestureConfig.EnablePan(FALSE);
		
		bcgpGestureManager.SetGestureConfig(GetSafeHwnd(), gestureConfig);
	}
}
//********************************************************************************
void CBCGPTasksPane::EnableNavigationToolbar (BOOL bEnable,
	UINT uiToolbarBmpRes, CSize sizeToolbarImage, CSize sizeToolbarButton)
{
	BOOL bReloadImages = m_wndToolBar.GetSafeHwnd () != NULL &&
		(m_uiToolbarBmpRes != uiToolbarBmpRes);

	m_bUseNavigationToolbar = bEnable;
	m_uiToolbarBmpRes = uiToolbarBmpRes;
	m_sizeToolbarImage = sizeToolbarImage;
	m_sizeToolbarButton = sizeToolbarButton;

	m_wndToolBar.m_bLargeIconsAreEnbaled = FALSE;

	if (bReloadImages)
	{
		const double dblImageScale = globalData.GetRibbonImageScale ();

		CSize sizeNavImage = globalData.Is32BitIcons () ? CSize (16, 16) : CSize (12, 12);
		const int nImageMargin = 4;

		CSize sizeNavButton = sizeNavImage + CSize (nImageMargin, nImageMargin);

		m_wndToolBar.m_ImagesLocked.Clear ();

		if (m_uiToolbarBmpRes == 0)
		{
			//----------------------
			// Use default resource:
			//----------------------
			if (dblImageScale != 1.)
			{
				sizeNavButton = CSize ((int)(.5 + dblImageScale * sizeNavButton.cx), (int)(.5 + dblImageScale * sizeNavButton.cy));
			}

			m_wndToolBar.SetLockedSizes (sizeNavButton, sizeNavImage);

			CBCGPLocalResource	lr;
			if (!m_wndToolBar.LoadBitmap (
				globalData.Is32BitIcons () ? IDB_BCGBARRES_TASKPANE32 : IDB_BCGBARRES_TASKPANE,
				0, 0, TRUE))
			{
				ASSERT(FALSE);
			}
		}
		else
		{
			if (m_sizeToolbarImage != CSize (0, 0))
			{
				sizeNavImage = m_sizeToolbarImage;

				if (m_sizeToolbarButton != CSize (0, 0))
				{
					sizeNavButton = m_sizeToolbarButton;
				}
				else
				{
					sizeNavButton = sizeNavImage + CSize (nImageMargin, nImageMargin);
				}
			}

			if (dblImageScale != 1.)
			{
				sizeNavButton = CSize ((int)(.5 + dblImageScale * sizeNavButton.cx), (int)(.5 + dblImageScale * sizeNavButton.cy));
			}

			m_wndToolBar.SetLockedSizes (sizeNavButton, sizeNavImage);

			if (!m_wndToolBar.LoadBitmap (m_uiToolbarBmpRes, 0, 0, TRUE))
			{
				ASSERT(FALSE);
			}
		}
	}

	UpdateCaption ();
}
//********************************************************************************
void CBCGPTasksPane::UpdateCaption ()
{
	if (IsToolBox())
	{
		return;
	}

	POSITION pos = m_lstTasksPanes.FindIndex (GetActivePage ());
	ASSERT(pos != NULL);
	
	CBCGPTasksPanePage* pPage = (CBCGPTasksPanePage*) m_lstTasksPanes.GetAt (pos);
	ASSERT_VALID(pPage);

	BOOL bMultiPage = (m_lstTasksPanes.GetCount () > 1);
	if (m_bUseNavigationToolbar || ForceShowNavToolbar () || bMultiPage)
	{
		SetWindowText (m_strCaption);
	}
	else
	{
		SetWindowText(pPage->m_strName);
	}

	m_wndToolBar.UpdateMenuButtonText (pPage->m_strName);

	// Update caption in non-client area:
	UINT uiSWPFlags =	SWP_FRAMECHANGED |
						SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE;

	CBCGPMiniFrameWnd* pParentMiniFrame = GetParentMiniFrame (TRUE);
	if (pParentMiniFrame != NULL)
	{
		ASSERT_VALID (pParentMiniFrame);

		pParentMiniFrame->SetWindowPos (NULL, -1, -1, -1, -1, uiSWPFlags);
	}
	else
	{
		SetWindowPos (NULL, -1, -1, -1, -1, uiSWPFlags);
	}
}
//********************************************************************************
void CBCGPTasksPane::Update ()
{
	UpdateCaption ();

	AdjustScroll ();
	ReposTasks ();
	RedrawWindow (NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
}
//********************************************************************************
void CBCGPTasksPane::UpdateFocusedItems ()
{
	if (GetSafeHwnd () == NULL)
	{
		return;
	}

	BOOL bUpdate = FALSE;

	if (m_pFocusedGroupCaption != NULL)
	{
		CRect rectUpdate = m_pFocusedGroupCaption->m_rect;
		RedrawWindow (rectUpdate, NULL, RDW_INVALIDATE | RDW_ERASE);
		bUpdate = TRUE;
	}
	if (m_pFocusedTask != NULL)
	{
		CRect rectUpdate = m_pFocusedTask->m_rect;
		RedrawWindow (rectUpdate, NULL, RDW_INVALIDATE | RDW_ERASE);
		bUpdate = TRUE;
	}

	if (bUpdate)
	{
		UpdateWindow ();
	}
}
//********************************************************************************
void CBCGPTasksPane::GetPreviousPages (CStringList& lstPrevPages) const
{
	ASSERT (m_iActivePage >= 0);
	ASSERT (m_iActivePage <= m_arrHistoryStack.GetUpperBound ());

	// -----------------------------------------
	// Collect names list of the previous pages:
	// -----------------------------------------
	lstPrevPages.RemoveAll ();
	const int nCount = m_iActivePage;
	for (int i = 0; i < nCount; i++)
	{
		int nPageIdx = m_arrHistoryStack [m_iActivePage - 1 - i];

		POSITION posPage = m_lstTasksPanes.FindIndex (nPageIdx);
		ASSERT(posPage != NULL);

		CBCGPTasksPanePage* pPage = (CBCGPTasksPanePage*) m_lstTasksPanes.GetAt (posPage);
		ASSERT_VALID(pPage);

		lstPrevPages.AddTail (pPage->m_strName);
	}
}
//********************************************************************************
void CBCGPTasksPane::GetNextPages (CStringList& lstNextPages) const
{
	ASSERT (m_iActivePage >= 0);
	ASSERT (m_iActivePage <= m_arrHistoryStack.GetUpperBound ());

	// -------------------------------------
	// Collect names list of the next pages:
	// -------------------------------------
	lstNextPages.RemoveAll ();
	const int nCount = (int) m_arrHistoryStack.GetUpperBound () - m_iActivePage;
	for (int i = 0; i < nCount; i++)
	{
		int nPageIdx = m_arrHistoryStack [m_iActivePage + 1 + i];

		POSITION posPage = m_lstTasksPanes.FindIndex (nPageIdx);
		ASSERT(posPage != NULL);

		CBCGPTasksPanePage* pPage = (CBCGPTasksPanePage*) m_lstTasksPanes.GetAt (posPage);
		ASSERT_VALID(pPage);

		lstNextPages.AddTail (pPage->m_strName);
	}
}
//********************************************************************************
void CBCGPTasksPane::EnableHistoryMenuButtons (BOOL bEnable)
{
	if (m_bHistoryMenuButtons == bEnable)
	{
		return;
	}

	BOOL bRecreateToolBar = FALSE;

	if (m_wndToolBar.GetSafeHwnd () != NULL)
	{
		bRecreateToolBar = TRUE;
		m_wndToolBar.DestroyWindow ();
	}

	m_bHistoryMenuButtons = bEnable;

	if (bRecreateToolBar)
	{
		CreateNavigationToolbar ();
		m_wndToolBar.UpdateButtons ();
	}
}
//********************************************************************************
CSize CBCGPTasksPane::GetTasksGroupBorders () const
{
	if (IsToolBox ())
	{
		return CSize (0, 0);
	}

	return CSize (1, 1);
}
//*****************************************************************************************
LRESULT CBCGPTasksPane::OnSetText(WPARAM, LPARAM lParam) 
{
	LRESULT	lRes = Default();

	if (lParam != NULL)
	{
		m_strCaption = (LPCTSTR)lParam;
	}

	return lRes;
}
//********************************************************************************
void CBCGPTasksPane::NotifyAccessibility (int nGroupNumber, int nTaskNumber)
{
	if (!globalData.IsAccessibilitySupport ())
	{
		return;
	}

	if (nGroupNumber == -1)
	{
		return;
	}

	CBCGPTask* pTask = GetTask (nGroupNumber, nTaskNumber);
	if (pTask == NULL)
	{
		return;
	}
	
	globalData.NotifyWinEvent (EVENT_OBJECT_FOCUS, GetSafeHwnd (), 
		OBJID_CLIENT , CHILDID_SELF);
}
//*****************************************************************************************
void CBCGPTasksPane::NotifyAccessibilityFocusEvent (BOOL bUseCursor)
{
	if (globalData.IsAccessibilitySupport ())
	{
		LPARAM lParam;
		if (bUseCursor)
		{
			lParam = (LPARAM)m_lAccUseCursorPosValue;
			if (m_bPrecessAccFocusEvent)
			{
				return;
			}
		}
		else
		{
			lParam = (LPARAM)m_lAccUseFocus;
			m_bPrecessAccFocusEvent = TRUE;
		}

		globalData.NotifyWinEvent (EVENT_OBJECT_FOCUS, GetSafeHwnd (), 
			OBJID_CLIENT , (LONG)lParam);
	}
}
//*****************************************************************************************
BOOL CBCGPTasksPane::OnSetAccData (long lVal)
{
	ASSERT_VALID (this);

	CBCGPTasksGroup* pGroup = NULL;
	CBCGPTask* pTask = NULL;

	if (lVal == m_lAccUseCursorPosValue)
	{
		if (m_bPrecessAccFocusEvent)
		{
			return TRUE;
		}

		CPoint pt;
		::GetCursorPos (&pt);
		ScreenToClient (&pt);

		pGroup = GroupCaptionHitTest (pt);
		pTask = TaskHitTest (pt);
	} 
	else if (lVal == m_lAccUseFocus)
	{
		if (m_pFocusedTask != NULL)
		{
			pGroup = NULL;
			pTask = m_pFocusedTask;
		}
		else if (m_pFocusedGroupCaption != NULL)
		{
			pGroup = m_pFocusedGroupCaption;
			pTask = NULL;
		}
	}
	else
	{
		return TRUE;
	}

	if (pGroup == NULL && pTask == NULL)
	{
		POSITION pos = m_lstTasksPanes.FindIndex (GetActivePage ());
		ASSERT(pos != NULL);
	
		CBCGPTasksPanePage* pPage = (CBCGPTasksPanePage*) m_lstTasksPanes.GetAt (pos);
		if (pPage != NULL)
		{
			ASSERT_VALID (pPage);
			pPage->SetACCData (this, m_AccData);
			m_AccData.m_rectAccLocation = m_rectTasks;
			ClientToScreen (&m_AccData.m_rectAccLocation);
		}
	}

	m_AccData.Clear ();

	if (pGroup != NULL)
	{
		ASSERT_VALID (pGroup);
		pGroup->SetACCData (this, m_AccData);
		if (lVal == m_lAccUseCursorPosValue)
		{
			m_AccData.m_bAccState |= STATE_SYSTEM_FOCUSED;
			m_AccData.m_bAccState |= STATE_SYSTEM_HOTTRACKED;
		}
		else if (lVal == m_lAccUseFocus)
		{
			m_AccData.m_bAccState |= STATE_SYSTEM_FOCUSED;
		}
	}

	if (pTask != NULL)
	{
		ASSERT_VALID (pTask);
		pTask->SetACCData (this, m_AccData);
		if (lVal == m_lAccUseCursorPosValue)
		{
			m_AccData.m_bAccState |= STATE_SYSTEM_FOCUSED;
			m_AccData.m_bAccState |= STATE_SYSTEM_HOTTRACKED;
		}
		else if (lVal == m_lAccUseFocus)
		{
			m_AccData.m_bAccState |= STATE_SYSTEM_FOCUSED;
		}
	}
	return TRUE;
}
//*****************************************************************************************
BOOL CBCGPTasksGroup::SetACCData (CWnd* pParent, CBCGPAccessibilityData& data)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pParent);

	CBCGPTasksPane* pTaskpane = DYNAMIC_DOWNCAST(CBCGPTasksPane, pParent);

	data.Clear ();
	data.m_strAccName = m_strName;
	data.m_nAccRole = ROLE_SYSTEM_GROUPING;
	data.m_bAccState = STATE_SYSTEM_NORMAL;
	data.m_nAccHit = 1;

	if (pTaskpane != NULL && pTaskpane->IsGroupCollapseEnabled())
	{
		data.m_bAccState = STATE_SYSTEM_FOCUSABLE;
	}

	data.m_rectAccLocation = m_rect;
	pParent->ClientToScreen (&data.m_rectAccLocation);

	return TRUE;
}
//*****************************************************************************************
BOOL CBCGPTask::SetACCData (CWnd* pParent, CBCGPAccessibilityData& data)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pParent);

	data.Clear ();
	data.m_strAccName = m_strName;
	data.m_strAccValue = m_strName;
	data.m_nAccRole = ROLE_SYSTEM_LINK;
	data.m_bAccState = STATE_SYSTEM_FOCUSABLE;

	if (!m_bEnabled)
	{
		data.m_bAccState |= STATE_SYSTEM_UNAVAILABLE;
	}

	data.m_nAccHit = 1;
	data.m_strAccDefAction = L"Press";
	data.m_rectAccLocation = m_rect;
	pParent->ClientToScreen (&data.m_rectAccLocation);

	return TRUE;
}
//*****************************************************************************************
BOOL CBCGPTasksPanePage::SetACCData (CWnd* /*pParent*/, CBCGPAccessibilityData& data)
{
	ASSERT_VALID (this);

	data.Clear ();
	data.m_strAccName = m_strName;
	data.m_nAccRole = ROLE_SYSTEM_PAGETAB ;
    data.m_bAccState = STATE_SYSTEM_NORMAL;
	data.m_nAccHit = 1;
	data.m_bAccState = STATE_SYSTEM_DEFAULT;

	return TRUE;
}
//*****************************************************************************************
BOOL CBCGPTasksPane::ProcessKeyboard (UINT nKey)
{
	ASSERT_VALID (this);
	const BOOL bShift = ::GetAsyncKeyState (VK_SHIFT) & 0x8000;

	switch (nKey)
	{
	case VK_UP:
		OnPrev ();
		return TRUE;

	case VK_DOWN:
		OnNext ();
		return TRUE;

	case VK_TAB:
		bShift ? OnPrev () : OnNext ();
		return TRUE;

	case VK_RETURN:
		return OnEnter ();

	case VK_LEFT:
	case VK_SUBTRACT:
		if (m_pFocusedGroupCaption != NULL && !m_pFocusedGroupCaption->m_bIsCollapsed)
		{
			DoGroupCaptionClick (m_pFocusedGroupCaption, TRUE);
			return TRUE;
		}
		break;

	case VK_RIGHT:
	case VK_ADD:
		if (m_pFocusedGroupCaption != NULL && m_pFocusedGroupCaption->m_bIsCollapsed)
		{
			DoGroupCaptionClick (m_pFocusedGroupCaption, TRUE);
			return TRUE;
		}
		break;
	}

	return FALSE;
}
//*************************************************************************************
void CBCGPTasksPane::OnChangeFocus (CBCGPTask* pTask, CBCGPTasksGroup* pGroup)
{
	if (IsToolBox ())
	{
		return;
	}

	if (pTask == m_pFocusedTask && pGroup == m_pFocusedGroupCaption)
	{
		return;
	}

	if (pTask != NULL)
	{
		ASSERT_VALID (pTask);
		m_pFocusedTask = pTask;
		m_pFocusedGroupCaption = NULL;
	}
	else if (pGroup != NULL)
	{
		ASSERT_VALID (pGroup);
		m_pFocusedTask = NULL;
		m_pFocusedGroupCaption = pGroup;
	}

	EnsureVisible ();

	CWnd* pFocusWnd = CWnd::GetFocus();

	// If focus is within taskpane
	if (pFocusWnd != NULL && IsChild(pFocusWnd) || pFocusWnd == this)
	{
		CWnd* pNewFocusWnd = NULL;

		// Set focus to new task window
		if (m_pFocusedTask != NULL && m_pFocusedTask->m_hwndTask != NULL)
		{
			CWnd* pTaskWnd = CWnd::FromHandle (m_pFocusedTask->m_hwndTask);
			if (pTaskWnd != NULL && IsChild(pTaskWnd))
			{
				pNewFocusWnd = pTaskWnd->SetFocus ();
			}
		}

		// Return focus back to the taskpane from the previously focused task window
		if (pNewFocusWnd == NULL)
		{
			SetFocus ();
		}
	}

	// update
	Invalidate ();

	// notify accessibility
	if (m_pFocusedTask != NULL || m_pFocusedGroupCaption != NULL)
	{
		NotifyAccessibilityFocusEvent (FALSE);
	}
}
//*************************************************************************************
void CBCGPTasksPane::OnHotTask (CBCGPTask* pTask)
{
	if (pTask != NULL && 
		globalData.IsAccessibilitySupport ())
	{
		m_bPrecessAccFocusEvent = FALSE;
		NotifyAccessibilityFocusEvent (TRUE);
	}
}
//*************************************************************************************
void CBCGPTasksPane::OnHotGroupCaption (CBCGPTasksGroup* pGroup)
{
	if (pGroup != NULL && 
		globalData.IsAccessibilitySupport ())
	{
		m_bPrecessAccFocusEvent = FALSE;
		NotifyAccessibilityFocusEvent (TRUE);
	}
}
//*************************************************************************************
void CBCGPTasksPane::EnsureVisible ()
{
	CRect rect;
	if (m_pFocusedTask != NULL)
	{
		rect = m_pFocusedTask->m_rect;

		if (m_pFocusedTask->m_hwndTask != NULL)
		{
			if (rect.top > m_rectTasks.top + 1) // scroll down
			{
				rect.bottom = rect.top + m_pFocusedTask->m_nWindowHeight;
			}
			else if (rect.bottom < m_rectTasks.bottom) // scroll up
			{
				rect.top = rect.bottom - m_pFocusedTask->m_nWindowHeight;
			}
		}
	}
	else if (m_pFocusedGroupCaption != NULL)
	{
		rect = m_pFocusedGroupCaption->m_rect;
	}
	else
	{
		return;
	}

	if (rect.bottom > m_rectTasks.bottom &&
		rect.top > m_rectTasks.top)
	{
		// scroll down
		int nDelta = (rect.bottom - m_rectTasks.bottom + m_nRowHeight - 1) / m_nRowHeight;
		if (rect.top - nDelta * m_nRowHeight < m_rectTasks.top)
		{
			int nError = rect.top % m_nRowHeight;
			nDelta = (rect.top - m_rectTasks.top + m_nRowHeight - 1 - nError) / m_nRowHeight;
		}
		m_nVertScrollOffset += nDelta;
	}
	else if (rect.top < m_rectTasks.top &&
		rect.bottom < m_rectTasks.bottom)
	{
		// scroll up
		int nDelta = (m_rectTasks.top - rect.top + m_nRowHeight - 1) / m_nRowHeight;
		if (rect.bottom + nDelta * m_nRowHeight > m_rectTasks.bottom)
		{
			int nError = rect.top % m_nRowHeight;
			nDelta = (m_rectTasks.top - rect.top + m_nRowHeight - 1 + nError) / m_nRowHeight;
		}
		m_nVertScrollOffset = max (0, m_nVertScrollOffset - nDelta);
	}

	AdjustScroll ();
	ReposTasks ();

	Invalidate ();
	UpdateWindow ();
}
//*************************************************************************************
void CBCGPTasksPane::DoTaskClick (CBCGPTask* pClickTask)
{
	if (pClickTask == NULL || pClickTask->m_pGroup == NULL)
	{
		return;
	}

	// Find task number and group number:
	ASSERT_VALID (pClickTask);
	ASSERT_VALID (pClickTask->m_pGroup);
	
	int nTaskNumber = -1;
	int i = 0;
	
	for (POSITION posTask = pClickTask->m_pGroup->m_lstTasks.GetHeadPosition (); 
	posTask != NULL; i++)
	{
		CBCGPTask* pTask = (CBCGPTask*) pClickTask->m_pGroup->m_lstTasks.GetNext (posTask);
		ASSERT_VALID (pTask);
		
		if (pTask == pClickTask)
		{
			nTaskNumber = i;
			break;
		}
	}
	
	int nGroupNumber = -1;
	i = 0;
	
	for (POSITION pos = m_lstTaskGroups.GetHeadPosition (); pos != NULL; i++)
	{
		CBCGPTasksGroup* pGroup = (CBCGPTasksGroup*) m_lstTaskGroups.GetNext (pos);
		ASSERT_VALID (pGroup);
		
		if (pClickTask->m_pGroup == pGroup)
		{
			nGroupNumber = i;
			break;
		}
	}
	
	OnClickTask (nGroupNumber, nTaskNumber, pClickTask->m_uiCommandID, pClickTask->m_dwUserData);

}
//*************************************************************************************
void CBCGPTasksPane::DoGroupCaptionClick(CBCGPTasksGroup* pClickGroupCaption, BOOL bDisableAnimation)
{
	if (pClickGroupCaption == NULL)
	{
		return;
	}

	ASSERT_VALID (pClickGroupCaption);

	pClickGroupCaption->m_bIsCollapsed = !pClickGroupCaption->m_bIsCollapsed;

	if (m_bAnimationEnabled && !bDisableAnimation)
	{
		m_pAnimatedGroup = pClickGroupCaption;
		m_sizeAnim = m_pAnimatedGroup->m_rectGroup.Size ();

		SetTimer (iAnimTimerId, m_iAnimTimerDuration, NULL);
		m_nLastAnimTime = clock ();
	}

	AdjustScroll ();
	ReposTasks ();
	RedrawWindow (NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
}
//*****************************************************************************************
void CBCGPTasksPane::OnFirst()
{
	m_pFocusedTask = NULL;
	m_pFocusedGroupCaption = NULL;

	OnNext ();
}
//*****************************************************************************************
void CBCGPTasksPane::OnNext()
{
	CBCGPTask* pOldFocusedTask = m_pFocusedTask;
	CBCGPTasksGroup* pOldFocusedGroupCaption = m_pFocusedGroupCaption;

	CBCGPTask* pTask = NULL;
	CBCGPTasksGroup* pGroup = NULL;

	if (pOldFocusedTask != NULL)
	{
		pTask = GetNextTask(pOldFocusedTask->m_pGroup, pOldFocusedTask, TRUE);
		pGroup = pOldFocusedTask->m_pGroup;
	}
	else if (pOldFocusedGroupCaption != NULL)
	{
		pTask = GetNextTask(pOldFocusedGroupCaption, NULL, TRUE);
		pGroup = pOldFocusedGroupCaption;
	}

	// If task is not found - search first focusable group or task
	do
	{
		if (pTask == NULL)
		{
			pGroup = GetNextGroup(pGroup, TRUE);
		}
		
		if (pTask == NULL && pGroup != NULL)
		{
			// If group caption is focusable
			if (!pGroup->m_strName.IsEmpty ())
			{
				// set focus to this group
				break;
			}
			else if (!pGroup->m_bIsCollapsed)
			{
				// search tasks inside this group
				pTask = GetNextTask(pGroup, NULL, TRUE);
				// if task is not found - search once more in the next group
			}
		}
	}
	while (pTask == NULL && pGroup != NULL);

	OnChangeFocus (pTask, pGroup);
}
//*****************************************************************************************
void CBCGPTasksPane::OnPrev()
{
	CBCGPTask* pOldFocusedTask = m_pFocusedTask;
	CBCGPTasksGroup* pOldFocusedGroupCaption = m_pFocusedGroupCaption;
	
	CBCGPTask* pTask = NULL;
	CBCGPTasksGroup* pGroup = NULL;
	
	if (pOldFocusedTask != NULL)
	{
		pTask = pOldFocusedTask;
		pGroup = pOldFocusedTask->m_pGroup;
	}
	else if (pOldFocusedGroupCaption != NULL)
	{
		pGroup = pOldFocusedGroupCaption;
	}
	
	// If task is not found - search first focusable group or task
	do
	{
		if (pTask == NULL)
		{
			pGroup = GetNextGroup(pGroup, FALSE);
		}
		if (pGroup != NULL)
		{
			if (!pGroup->m_bIsCollapsed)
			{
				// First - search tasks inside this group
				pTask = GetNextTask(pGroup, pTask, FALSE);
				// if task is not found - search once more in the next group
			}
			else
			{
				pTask = NULL;
			}
			// If group caption is focusable
			if (!pGroup->m_strName.IsEmpty ())
			{
				// set focus to this group
				break;
			}
		}
	}
	while (pTask == NULL && pGroup != NULL);
	
	OnChangeFocus (pTask, pGroup);
}
//*****************************************************************************************
BOOL CBCGPTasksPane::OnEnter()
{
	if (m_pFocusedTask != NULL)
	{
		DoTaskClick (m_pFocusedTask);
		return TRUE;
	}
	else if (m_pFocusedGroupCaption != NULL)
	{
		DoGroupCaptionClick (m_pFocusedGroupCaption, TRUE);
		return TRUE;
	}
	return FALSE;
}
//*****************************************************************************************
CBCGPTask* CBCGPTasksPane::GetNextTask (CBCGPTasksGroup* pGroup, CBCGPTask* pStartTask, BOOL bForward) const
{
	ASSERT_VALID (this);

	if (pGroup == NULL)
	{
		return NULL;
	}

	ASSERT_VALID (pGroup);

	if (pGroup->m_bIsCollapsed)
	{
		return NULL;
	}

	//---------------
	// Find next task
	//---------------
	CBCGPTask* pTask = NULL;
	for (POSITION posTask = (bForward ? pGroup->m_lstTasks.GetHeadPosition () : pGroup->m_lstTasks.GetTailPosition ()); posTask != NULL;)
	{
		CBCGPTask* pNextTask = (CBCGPTask*) (bForward ? pGroup->m_lstTasks.GetNext (posTask) : pGroup->m_lstTasks.GetPrev (posTask));
		ASSERT_VALID (pNextTask);

		if (pNextTask->m_bEnabled && !pNextTask->m_bIsSeparator)
		{
			if (pStartTask == pTask)
			{
				// return next after current
				return pNextTask;
			}
			if (pStartTask == NULL)
			{
				// return first task
				return pNextTask;
			}
			
			pTask = pNextTask;
		}
	}

	// no more tasks
	return NULL;
}
//*****************************************************************************************
CBCGPTasksGroup* CBCGPTasksPane::GetNextGroup (CBCGPTasksGroup* pStartGroup, BOOL bForward) const
{
	ASSERT_VALID (this);

	// ---------------
	// Get active page
	// ---------------
	CBCGPTasksPanePage* pActivePage = NULL;
	POSITION posPage = m_lstTasksPanes.FindIndex (m_arrHistoryStack[m_iActivePage]);
	ASSERT(posPage != NULL);
	
	pActivePage = (CBCGPTasksPanePage*) m_lstTasksPanes.GetAt (posPage);
	ASSERT_VALID(pActivePage);

	//----------------
	// Find next group
	//----------------
	CBCGPTasksGroup* pGroup = NULL;
	for (POSITION pos = (bForward ? m_lstTaskGroups.GetHeadPosition () : m_lstTaskGroups.GetTailPosition ()); pos != NULL;)
	{
		CBCGPTasksGroup* pNextGroup = (CBCGPTasksGroup*) (bForward ? m_lstTaskGroups.GetNext (pos) : m_lstTaskGroups.GetPrev (pos));
		ASSERT_VALID (pNextGroup);

		if (pNextGroup->m_pPage == pActivePage)
		{
			if (pStartGroup == pGroup)
			{
				// return next after current
				return pNextGroup;
			}
			if (pStartGroup == NULL)
			{
				// return first group
				return pNextGroup;
			}

			pGroup = pNextGroup;
		}
	}

	// no more groups
	return NULL;
}
//*****************************************************************************************
BOOL CBCGPTasksPane::OnGestureEventPan(const CPoint& ptFrom, const CPoint& ptTo, CSize& sizeOverPan)
{
	const int nDelta = ptTo.y - ptFrom.y;
	if (nDelta == 0 || m_nRowHeight == 0)
	{
		return FALSE;
	}

	const int nSteps = (int)(0.5 + abs(nDelta) / m_nRowHeight);
	if (nSteps == 0)
	{
		return FALSE;
	}

	int nOffset = m_nVertScrollOffset;
	int nPrevOffset = m_nVertScrollOffset;

	if (nDelta < 0)
	{
		nOffset += nSteps;
	}
	else
	{
		nOffset -= nSteps;
	}

	OnVScroll(SB_THUMBPOSITION, nOffset, NULL);
	
	if (m_nVertScrollOffset == nPrevOffset)
	{
		sizeOverPan.cy = nDelta;
	}
	
	return TRUE;
}

#endif // BCGP_EXCLUDE_TASK_PANE
