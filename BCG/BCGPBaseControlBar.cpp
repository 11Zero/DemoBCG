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
// BCGPBaseControlBar.cpp : implementation file
//

#include "stdafx.h"

#include "BCGPFrameWnd.h"
#include "BCGPMiniFrameWnd.h"
#include "BCGPMultiMiniFrameWnd.h"
#include "BCGPAppBarWnd.h"
#include "BCGPSlider.h"

#include "BCGPFrameWnd.h"
#include "BCGPMDIFrameWnd.h"
#include "BCGPOleIPFrameWnd.h"
#include "BCGPOleDocIPFrameWnd.h"
#include "BCGPMDIChildWnd.h"
#include "BCGPOleCntrFrameWnd.h"

#include "BCGPDockBar.h"
#include "BCGPDockBarRow.h"

#include "BCGPBaseTabWnd.h"

#include "BCGPBaseControlBar.h"
#include "BCGPBaseTabbedBar.h"
#include "BCGPDockingCBWrapper.h"

#include "RegPath.h"
#include "BCGPRegistry.h"

#include "BCGPGlobalUtils.h"

#include "BCGPDockManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static const CString strBaseControlBarProfile = _T ("BCGPBaseControlBars");
BOOL CBCGPBaseControlBar::m_bSetTooltipTopmost = TRUE;

#define REG_SECTION_FMT					_T("%sBCGPBaseControlBar-%d")
#define REG_SECTION_FMT_EX				_T("%sBCGPBaseControlBar-%d%x")

BOOL CBCGPBaseControlBar::m_bMultiThreaded = FALSE;
CCriticalSection CBCGPBaseControlBar::g_cs;

IMPLEMENT_DYNAMIC (CBCGPBaseControlBar, CBCGPWnd)

/////////////////////////////////////////////////////////////////////////////
// CBCGPBaseControlBar

CBCGPBaseControlBar::CBCGPBaseControlBar()
{
	m_dwEnabledAlignment	= 0;
	m_dwStyle				= 0;
	m_pParentDockBar		= NULL; 
	m_pDockBarRow			= NULL;
	m_pDockSite				= NULL;

	m_bRecentVisibleState	= FALSE;
	m_bIsRestoredFromRegistry = FALSE;

	m_dwBCGStyle			= 0;

	m_bVisible				= FALSE;
	m_dockMode				= BCGP_DT_UNDEFINED; 
	m_bEnableIDChecking		= TRUE;

	m_lpszBarTemplateName	= NULL;
	m_sizeDialog			= CSize (0, 0);

	m_rectBar.SetRectEmpty ();

	m_bIsDlgControl			= FALSE;
	m_bIsMDITabbed			= FALSE;

	m_bIsRebarPane			= FALSE;

	m_hiconGraySmall		= NULL;
	m_hiconGrayLarge		= NULL;
}

CBCGPBaseControlBar::~CBCGPBaseControlBar()
{
	if (m_hiconGraySmall != NULL)
	{
		::DestroyIcon(m_hiconGraySmall);
	}

	if (m_hiconGrayLarge != NULL)
	{
		::DestroyIcon(m_hiconGrayLarge);
	}
}


BEGIN_MESSAGE_MAP(CBCGPBaseControlBar, CBCGPWnd)
	//{{AFX_MSG_MAP(CBCGPBaseControlBar)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_SETTINGCHANGE()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_IDLEUPDATECMDUI, OnIdleUpdateCmdUI)
	ON_MESSAGE(WM_HELPHITTEST, OnHelpHitTest)
	ON_MESSAGE(WM_INITDIALOG, HandleInitDialog)
	ON_MESSAGE(WM_SETICON, OnSetIcon)
	ON_MESSAGE(WM_PRINT, OnPrint)
	ON_MESSAGE(WM_PRINTCLIENT, OnPrintClient)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CBCGPBaseControlBar message handlers
//*******************************************************************************
BOOL CBCGPBaseControlBar::CreateEx(DWORD dwStyleEx, LPCTSTR lpszClassName, 
								  LPCTSTR lpszWindowName,
								  DWORD dwStyle, const RECT& rect, 
								  CWnd* pParentWnd, UINT nID, 
								  DWORD dwBCGStyle,
								  CCreateContext* pContext)
{
	ASSERT_VALID (pParentWnd);

	m_bIsDlgControl = pParentWnd->IsKindOf (RUNTIME_CLASS (CDialog));

	if (m_bEnableIDChecking) 
	{
		CBCGPDockManager* pDockManager = globalUtils.GetDockManager (pParentWnd);
		if (pDockManager == NULL)
		{
			pDockManager = globalUtils.GetDockManager (BCGPGetParentFrame (pParentWnd));
			if (pDockManager != NULL)
			{
				if (pDockManager->FindBarByID (nID, TRUE) != NULL)
				{
					TRACE0("Control bar must be created with unique ID!\n");
				}
			}
		}
	}

	m_bVisible = m_bVisible & WS_VISIBLE;

	SetBarStyle (dwStyle | GetBarStyle ());
	m_dwBCGStyle = dwBCGStyle;

	BOOL bResult = FALSE;
	
	if (m_lpszBarTemplateName != NULL)
	{
		CREATESTRUCT cs;
		memset(&cs, 0, sizeof(cs));
		cs.lpszClass = lpszClassName;//AFX_WNDCONTROLBAR;
		cs.lpszName = lpszWindowName;
		cs.style = dwStyle | WS_CHILD;
		cs.hMenu = (HMENU)(UINT_PTR) nID;
		cs.hInstance = AfxGetInstanceHandle();
		cs.hwndParent = pParentWnd->GetSafeHwnd();

		if (!PreCreateWindow(cs))
		{
			return FALSE;
		}

		//----------------------------
		// initialize common controls
		//----------------------------
		//VERIFY(AfxDeferRegisterClass(AFX_WNDCOMMCTLS_REG));
		//AfxDeferRegisterClass(AFX_WNDCOMMCTLSNEW_REG);

		//--------------------------
		// create a modeless dialog
		//--------------------------
		if (!CreateDlg (m_lpszBarTemplateName, pParentWnd))
		{
			if (IS_INTRESOURCE(m_lpszBarTemplateName))
			{
				TRACE(_T("Can't create dialog: %d\n"), (INT_PTR)m_lpszBarTemplateName);
			}
			else
			{
				TRACE(_T("Can't create dialog: %s\n"), m_lpszBarTemplateName);
			}
			return FALSE;
		}

#pragma warning (disable : 4311)
		SetClassLongPtr (m_hWnd, GCLP_HBRBACKGROUND, (LONG_PTR) ::GetSysColorBrush(COLOR_BTNFACE));
#pragma warning (default : 4311)

		SetDlgCtrlID(nID);

		CRect rect;
		GetWindowRect(&rect);

		m_sizeDialog = rect.Size ();
		bResult = TRUE;
	}
	else
	{
		bResult = CBCGPWnd::CreateEx (dwStyleEx, lpszClassName, lpszWindowName, 
										dwStyle, rect, pParentWnd, nID, pContext);
	}

	if (bResult)
	{
		if (pParentWnd->IsKindOf (RUNTIME_CLASS (CFrameWnd)))
		{
			m_pDockSite = DYNAMIC_DOWNCAST (CFrameWnd, pParentWnd);
		}
		else 
		{
			// case of miniframe or smth. else
			m_pDockSite = DYNAMIC_DOWNCAST (CFrameWnd, BCGPGetParentFrame (pParentWnd));
		}

		m_bIsDlgControl = pParentWnd->IsKindOf (RUNTIME_CLASS (CDialog));
	}
	
	return bResult;
}
//*******************************************************************************
BOOL CBCGPBaseControlBar::OnEraseBkgnd(CDC* /*pDC*/) 
{
	return TRUE;
}
//*******************************************************************************
void CBCGPBaseControlBar::DoPaint(CDC* pDC)
{
	CRect rectClip;
	pDC->GetClipBox (rectClip);

	CRect rect;
	GetClientRect(rect);

	CBCGPVisualManager::GetInstance ()->OnFillBarBackground (pDC, this,
			rect, rectClip);
}	
//*******************************************************************************
BOOL CBCGPBaseControlBar::IsDocked () const
{
	// return TRUE if its parent is not miniframe or the bar is floating 
	// in the miniframe with another control bar
	CBCGPMiniFrameWnd* pParentMiniFrame = GetParentMiniFrame ();
	if (pParentMiniFrame != NULL)
	{
		ASSERT_VALID (pParentMiniFrame);
		if (pParentMiniFrame->GetControlBarCount () == 1)
		{
			return FALSE;
		}
	}

	return TRUE;
}
//*******************************************************************************
BOOL CBCGPBaseControlBar::IsTabbed () const
{
	ASSERT_VALID (this);
	CWnd* pParent = GetParent ();
	ASSERT_VALID (pParent);
	return pParent->IsKindOf (RUNTIME_CLASS (CBCGPBaseTabWnd));
}
//*******************************************************************************
BOOL CBCGPBaseControlBar::IsMDITabbed () const
{
	return m_bIsMDITabbed;
}
//*******************************************************************************
BOOL CBCGPBaseControlBar::IsVisible () const 
{
	ASSERT_VALID (this);

	if (!IsTabbed ())
	{
		if (CBCGPDockManager::m_bRestoringDockState)
		{
			return GetRecentVisibleState ();
		}
		
		return ((GetStyle () & WS_VISIBLE) != 0);
	}

	HWND hWndTab = NULL; 
	CBCGPBaseTabWnd* pParent = GetParentTabWnd (hWndTab);

	ASSERT_VALID (pParent);

	if (!pParent->IsWindowVisible ())
	{
		return FALSE;
	}

	int iTabNum = pParent->GetTabFromHwnd (hWndTab);
	
	if (iTabNum >= 0 && iTabNum < pParent->GetTabsNum ())
	{
		return pParent->IsTabVisible (iTabNum);
	}

	return FALSE;
}
//***********************************************************************************//
CBCGPMiniFrameWnd* CBCGPBaseControlBar::GetParentMiniFrame (BOOL bNoAssert) const
{
	ASSERT_VALID (this);
	CBCGPMiniFrameWnd* pMiniFrame = NULL;
	CWnd* pParent = GetParent ();

	while (pParent != NULL)
	{
		if (!bNoAssert)
		{
			ASSERT_VALID (pParent);
		}

		if (pParent != NULL && pParent->IsKindOf (RUNTIME_CLASS (CBCGPMiniFrameWnd)))
		{
			pMiniFrame = DYNAMIC_DOWNCAST (CBCGPMiniFrameWnd, pParent);
			break;
		}
		pParent = pParent->GetParent ();
	}

	return pMiniFrame;
}
//***********************************************************************************
void CBCGPBaseControlBar::OnPaint()
{
	if (m_bMultiThreaded)
	{
		g_cs.Lock ();
	}

 	CPaintDC dc(this);

	// erase background now
	if (GetStyle() & WS_VISIBLE)
		DoPaint(&dc);      

	if (m_bMultiThreaded)
	{
		g_cs.Unlock ();
	}
}
//*************************************************************************************
HDWP CBCGPBaseControlBar::MoveWindow (CRect& rect, BOOL bRepaint, HDWP hdwp)
{

	CRect rectOld;
	GetWindowRect (rectOld);

	if (IsFloating ())
	{
		CBCGPMiniFrameWnd* pMiniFrame = GetParentMiniFrame ();
		ASSERT_VALID (pMiniFrame);
		pMiniFrame->ScreenToClient (rectOld);
	}
	else if (m_pDockSite != NULL)
	{
		m_pDockSite->ScreenToClient (rectOld);
	}

	if (rectOld == rect)
	{
		return hdwp;
	}


	if (hdwp != NULL)
	{
		UINT uFlags = SWP_NOZORDER | SWP_NOACTIVATE;
		return DeferWindowPos (hdwp, GetSafeHwnd (), NULL, rect.left, rect.top, rect.Width (), 
						rect.Height (), uFlags);
		
	}
	CBCGPWnd::MoveWindow (&rect, bRepaint);
	return NULL;
}
//****************************************************************************************
HDWP CBCGPBaseControlBar::SetWindowPos (const CWnd* pWndInsertAfter, int x, int y, 
										int cx, int cy, UINT nFlags, HDWP hdwp)
{
	if (hdwp == NULL)
	{
		CBCGPWnd::SetWindowPos (pWndInsertAfter, x, y, cx, cy, nFlags);
		return NULL;
	}
	HDWP hdwpNew = DeferWindowPos (hdwp, GetSafeHwnd (), NULL, x, y, cx, cy, nFlags);
	if (hdwpNew == NULL)
	{
		TRACE1 ("DeferWindowPos failded, error code %d\n", GetLastError());
		SetWindowPos (NULL, x, y, cx, cy, nFlags);
		return hdwp;
	}

	return hdwpNew;
}

//*******************************************************************************
// frame mapping functions
//*******************************************************************************
void CBCGPBaseControlBar::AddControlBar (CBCGPBaseControlBar* pBar)
{
	CWnd* pParentFrame = GetDockSite ();
	if (pParentFrame == NULL || globalUtils.m_bDialogApp)
	{
		return;
	}
	ASSERT_VALID (pParentFrame);

	if (pParentFrame->IsKindOf (RUNTIME_CLASS (CBCGPFrameWnd)))
	{
		((CBCGPFrameWnd*) pParentFrame)->AddControlBar (pBar);
	}
	else if (pParentFrame->IsKindOf (RUNTIME_CLASS (CBCGPMDIFrameWnd)))
	{
		((CBCGPMDIFrameWnd*) pParentFrame)->AddControlBar (pBar);
	}
	else if (pParentFrame->IsKindOf (RUNTIME_CLASS (CBCGPOleIPFrameWnd)))
	{
		((CBCGPOleIPFrameWnd*) pParentFrame)->AddControlBar (pBar);
	}
	else if (pParentFrame->IsKindOf (RUNTIME_CLASS (CBCGPOleDocIPFrameWnd)))
	{
		((CBCGPOleDocIPFrameWnd*) pParentFrame)->AddControlBar (pBar);
	}
	else if (pParentFrame->IsKindOf (RUNTIME_CLASS (CBCGPMDIChildWnd)))
	{
		((CBCGPMDIChildWnd*) pParentFrame)->AddControlBar (pBar);
	}
	else if (pParentFrame->IsKindOf (RUNTIME_CLASS (CBCGPOleCntrFrameWnd)))
	{
		((CBCGPOleCntrFrameWnd*) pParentFrame)->AddControlBar (pBar);
	}
	else 
	{
		ASSERT (FALSE);
	}
}
//*******************************************************************************
void CBCGPBaseControlBar::RemoveControlBarFromDockManager (CBCGPBaseControlBar* pBar, 
											   BOOL bDestroy, BOOL bAdjustLayout,
											   BOOL bAutoHide, CBCGPBaseControlBar* pBarReplacement)
{
	CWnd* pParentFrame = GetDockSite ();
	if (pParentFrame == NULL || globalUtils.m_bDialogApp)
	{
		return;
	}

	ASSERT_VALID (pParentFrame);

	if (pParentFrame->IsKindOf (RUNTIME_CLASS (CBCGPFrameWnd)))
	{
		((CBCGPFrameWnd*) pParentFrame)->RemoveControlBarFromDockManager (pBar, bDestroy, bAdjustLayout, bAutoHide, pBarReplacement);
	}
	else if (pParentFrame->IsKindOf (RUNTIME_CLASS (CBCGPMDIFrameWnd)))
	{
		((CBCGPMDIFrameWnd*) pParentFrame)->RemoveControlBarFromDockManager (pBar, bDestroy, bAdjustLayout, bAutoHide, pBarReplacement);
	}
	else if (pParentFrame->IsKindOf (RUNTIME_CLASS (CBCGPOleIPFrameWnd)))
	{
		((CBCGPOleIPFrameWnd*) pParentFrame)->RemoveControlBarFromDockManager (pBar, bDestroy, bAdjustLayout, bAutoHide, pBarReplacement);
	}
	else if (pParentFrame->IsKindOf (RUNTIME_CLASS (CBCGPOleDocIPFrameWnd)))
	{
		((CBCGPOleDocIPFrameWnd*) pParentFrame)->RemoveControlBarFromDockManager (pBar, bDestroy, bAdjustLayout, bAutoHide, pBarReplacement);
	}
	else if (pParentFrame->IsKindOf (RUNTIME_CLASS (CBCGPMDIChildWnd)))
	{
		((CBCGPMDIChildWnd*) pParentFrame)->RemoveControlBarFromDockManager (pBar, bDestroy, bAdjustLayout, bAutoHide, pBarReplacement);
	}
	else if (pParentFrame->IsKindOf (RUNTIME_CLASS (CBCGPOleCntrFrameWnd)))
	{
		((CBCGPOleCntrFrameWnd*) pParentFrame)->RemoveControlBarFromDockManager (pBar, bDestroy, bAdjustLayout, bAutoHide, pBarReplacement);
	}
	else 
	{
		ASSERT (FALSE);
	}
}
//*******************************************************************************
BOOL CBCGPBaseControlBar::IsPointNearDockBar (CPoint point, DWORD& dwBarAlignment, 
												 BOOL& bOuterEdge) const
{
	CWnd* pParentFrame = GetDockSite ();

	if (pParentFrame == NULL || globalUtils.m_bDialogApp)
	{
		return TRUE;
	}

	ASSERT_VALID (pParentFrame);

	if (pParentFrame->IsKindOf (RUNTIME_CLASS (CBCGPFrameWnd)))
	{
		return ((CBCGPFrameWnd*) pParentFrame)->
			IsPointNearDockBar (point, dwBarAlignment, bOuterEdge);
	}
	else if (pParentFrame->IsKindOf (RUNTIME_CLASS (CBCGPMDIFrameWnd)))
	{
		return ((CBCGPMDIFrameWnd*) pParentFrame)->
			IsPointNearDockBar (point, dwBarAlignment, bOuterEdge);
	}
	else if (pParentFrame->IsKindOf (RUNTIME_CLASS (CBCGPOleIPFrameWnd)))
	{
		return ((CBCGPOleIPFrameWnd*) pParentFrame)->
			IsPointNearDockBar (point, dwBarAlignment, bOuterEdge);
	}
	else if (pParentFrame->IsKindOf (RUNTIME_CLASS (CBCGPOleDocIPFrameWnd)))
	{
		return ((CBCGPOleDocIPFrameWnd*) pParentFrame)->
			IsPointNearDockBar (point, dwBarAlignment, bOuterEdge);
	}
	else if (pParentFrame->IsKindOf (RUNTIME_CLASS (CBCGPMDIChildWnd)))
	{
		return ((CBCGPMDIChildWnd*) pParentFrame)->
			IsPointNearDockBar (point, dwBarAlignment, bOuterEdge);
	}
	else if (pParentFrame->IsKindOf (RUNTIME_CLASS (CBCGPOleCntrFrameWnd)))
	{
		return ((CBCGPOleCntrFrameWnd*) pParentFrame)->
			IsPointNearDockBar (point, dwBarAlignment, bOuterEdge);
	}
	else 
	{
		ASSERT (FALSE);
	}
	return FALSE;
}
//*******************************************************************************
CBCGPBaseControlBar* CBCGPBaseControlBar::ControlBarFromPoint (CPoint point, 
				int nSensitivity, bool bExactBar, CRuntimeClass* pRTCBarType) const
{
	CWnd* pParentFrame = GetDockSite ();

	if (pParentFrame == NULL || globalUtils.m_bDialogApp)
	{
		return NULL;
	}

	ASSERT_VALID (pParentFrame);

	if (pParentFrame->IsKindOf (RUNTIME_CLASS (CBCGPFrameWnd)))
	{
		return ((CBCGPFrameWnd*) pParentFrame)->
			ControlBarFromPoint (point, nSensitivity, bExactBar, pRTCBarType);
	}
	else if (pParentFrame->IsKindOf (RUNTIME_CLASS (CBCGPMDIFrameWnd)))
	{
		return ((CBCGPMDIFrameWnd*) pParentFrame)->
			ControlBarFromPoint (point, nSensitivity, bExactBar, pRTCBarType);
	}
	else if (pParentFrame->IsKindOf (RUNTIME_CLASS (CBCGPOleIPFrameWnd)))
	{
		return ((CBCGPOleIPFrameWnd*) pParentFrame)->
			ControlBarFromPoint (point, nSensitivity, bExactBar, pRTCBarType);
	}
	else if (pParentFrame->IsKindOf (RUNTIME_CLASS (CBCGPOleDocIPFrameWnd)))
	{
		return ((CBCGPOleDocIPFrameWnd*) pParentFrame)->
			ControlBarFromPoint (point, nSensitivity, bExactBar, pRTCBarType);
	}
	else if (pParentFrame->IsKindOf (RUNTIME_CLASS (CBCGPMDIChildWnd)))
	{
		return ((CBCGPMDIChildWnd*) pParentFrame)->
			ControlBarFromPoint (point, nSensitivity, bExactBar, pRTCBarType);
	}
	else if (pParentFrame->IsKindOf (RUNTIME_CLASS (CBCGPOleCntrFrameWnd)))
	{
		return ((CBCGPOleCntrFrameWnd*) pParentFrame)->
			ControlBarFromPoint (point, nSensitivity, bExactBar, pRTCBarType);
	}
	else 
	{
		ASSERT (FALSE);
	}
	return FALSE;
}
//*******************************************************************************
BOOL CBCGPBaseControlBar::InsertControlBar (CBCGPBaseControlBar* pControlBar, 
								CBCGPBaseControlBar* pTarget, BOOL bAfter)
{
	CBCGPMultiMiniFrameWnd* pParentMiniFrame = 
		DYNAMIC_DOWNCAST (CBCGPMultiMiniFrameWnd, GetParentMiniFrame ());
	if (pParentMiniFrame != NULL)
	{
		return pParentMiniFrame->InsertControlBar (pControlBar, pTarget, bAfter);
	}

	CWnd* pParentFrame = GetDockSite ();

	if (pParentFrame == NULL || globalUtils.m_bDialogApp)
	{
		return TRUE;
	}

	ASSERT_VALID (pParentFrame);

	if (pParentFrame->IsKindOf (RUNTIME_CLASS (CBCGPFrameWnd)))
	{
		return ((CBCGPFrameWnd*) pParentFrame)->
			InsertControlBar (pControlBar, pTarget, bAfter);
	}
	else if (pParentFrame->IsKindOf (RUNTIME_CLASS (CBCGPMDIFrameWnd)))
	{
		return ((CBCGPMDIFrameWnd*) pParentFrame)->
			InsertControlBar (pControlBar, pTarget, bAfter);
	}
	else if (pParentFrame->IsKindOf (RUNTIME_CLASS (CBCGPOleIPFrameWnd)))
	{
		return ((CBCGPOleIPFrameWnd*) pParentFrame)->
			InsertControlBar (pControlBar, pTarget, bAfter);
	}
	else if (pParentFrame->IsKindOf (RUNTIME_CLASS (CBCGPOleDocIPFrameWnd)))
	{
		return ((CBCGPOleDocIPFrameWnd*) pParentFrame)->
			InsertControlBar (pControlBar, pTarget, bAfter);
	}
	else if (pParentFrame->IsKindOf (RUNTIME_CLASS (CBCGPMDIChildWnd)))
	{
		return ((CBCGPMDIChildWnd*) pParentFrame)->
			InsertControlBar (pControlBar, pTarget, bAfter);
	}
	else if (pParentFrame->IsKindOf (RUNTIME_CLASS (CBCGPOleCntrFrameWnd)))
	{
		return ((CBCGPOleCntrFrameWnd*) pParentFrame)->
			InsertControlBar (pControlBar, pTarget, bAfter);
	}
	else 
	{
		ASSERT (FALSE);
	}
	return FALSE;
}
//*******************************************************************************
void CBCGPBaseControlBar::AdjustDockingLayout (HDWP hdwp)
{
	CBCGPMiniFrameWnd* pParentMiniFrame = GetParentMiniFrame ();

	if (pParentMiniFrame != NULL)
	{
		pParentMiniFrame->OnBarRecalcLayout ();
		return;
	}

	CWnd* pParentFrame = GetDockSite ();

	if (globalUtils.m_bDialogApp && pParentFrame == NULL)
	{
		return;
	}

	ASSERT_VALID (pParentFrame);

	if (pParentFrame->IsKindOf (RUNTIME_CLASS (CBCGPFrameWnd)))
	{
		((CBCGPFrameWnd*) pParentFrame)->AdjustDockingLayout (hdwp);
	}
	else if (pParentFrame->IsKindOf (RUNTIME_CLASS (CBCGPMDIFrameWnd)))
	{
		((CBCGPMDIFrameWnd*) pParentFrame)->AdjustDockingLayout (hdwp);
	}
	else if (pParentFrame->IsKindOf (RUNTIME_CLASS (CBCGPOleIPFrameWnd)))
	{
		((CBCGPOleIPFrameWnd*) pParentFrame)->AdjustDockingLayout (hdwp);
	}
	else if (pParentFrame->IsKindOf (RUNTIME_CLASS (CBCGPOleDocIPFrameWnd)))
	{
		((CBCGPOleDocIPFrameWnd*) pParentFrame)->AdjustDockingLayout (hdwp);
	}
	else if (pParentFrame->IsKindOf (RUNTIME_CLASS (CBCGPMDIChildWnd)))
	{
		((CBCGPMDIChildWnd*) pParentFrame)->AdjustDockingLayout (hdwp);
	}
	else if (pParentFrame->IsKindOf (RUNTIME_CLASS (CBCGPOleCntrFrameWnd)))
	{
		((CBCGPOleCntrFrameWnd*) pParentFrame)->AdjustDockingLayout (hdwp);
	}
	else 
	{
		ASSERT (FALSE);
	}
}
//*******************************************************************************
void CBCGPBaseControlBar::DockControlBarMap (BOOL bUseDocSite)
{
	CWnd* pParentFrame = bUseDocSite ? m_pDockSite : (CWnd*) BCGPGetParentFrame (this);

	if (pParentFrame == NULL || globalUtils.m_bDialogApp)
	{
		return;
	}

	ASSERT_VALID (pParentFrame);

	if (pParentFrame->IsKindOf (RUNTIME_CLASS (CBCGPFrameWnd)))
	{
		((CBCGPFrameWnd*) pParentFrame)->DockControlBar (this);
	}
	else if (pParentFrame->IsKindOf (RUNTIME_CLASS (CBCGPMDIFrameWnd)))
	{
		((CBCGPMDIFrameWnd*) pParentFrame)->DockControlBar (this);
	}
	else if (pParentFrame->IsKindOf (RUNTIME_CLASS (CBCGPOleIPFrameWnd)))
	{
		((CBCGPOleIPFrameWnd*) pParentFrame)->DockControlBar (this);
	}
	else if (pParentFrame->IsKindOf (RUNTIME_CLASS (CBCGPOleDocIPFrameWnd)))
	{
		((CBCGPOleDocIPFrameWnd*) pParentFrame)->DockControlBar (this);
	}
	else if (pParentFrame->IsKindOf (RUNTIME_CLASS (CBCGPMDIChildWnd)))
	{
		((CBCGPMDIChildWnd*) pParentFrame)->DockControlBar (this);
	}
	else if (pParentFrame->IsKindOf (RUNTIME_CLASS (CBCGPOleCntrFrameWnd)))
	{
		((CBCGPOleCntrFrameWnd*) pParentFrame)->DockControlBar (this);
	}
	else 
	{
		ASSERT (FALSE);
	}
}
//*******************************************************************************
void CBCGPBaseControlBar::ShowControlBar (BOOL bShow, BOOL bDelay, BOOL bActivate)
{
	int nShowCmd = bShow ? SW_SHOWNOACTIVATE : SW_HIDE;

	if (IsFloating () && !IsTabbed ())
	{
		ShowWindow (nShowCmd);

		CWnd* pParent = GetParent ();
		ASSERT_VALID (pParent);

		pParent->ShowWindow (nShowCmd);
		pParent->PostMessage (BCGPM_CHECKEMPTYMINIFRAME);
	}
	else if (m_pParentDockBar != NULL)
	{
		m_pParentDockBar->ShowControlBar (this, bShow, bDelay, bActivate);
	}
	else if (IsTabbed ())
	{
		HWND hWndTab = NULL;
		CBCGPBaseTabWnd* pTabParent = GetParentTabWnd (hWndTab);
		ASSERT_VALID (pTabParent);

		CBCGPBaseTabbedBar* pTabbedControlBar = 
			DYNAMIC_DOWNCAST (CBCGPBaseTabbedBar, pTabParent->GetParent ());

		if (pTabbedControlBar != NULL && !pTabbedControlBar->IsBarVisible () && 
			pTabbedControlBar->GetTabsNum () > 1 && bShow)
		{
			pTabbedControlBar->ShowTab (this, TRUE, bDelay, bActivate);
			return;
		}

		if (pTabbedControlBar != NULL)
		{
			ASSERT_VALID (pTabbedControlBar);
			pTabbedControlBar->ShowTab (this, bShow, bDelay, bActivate);
		
			if (pTabParent->GetVisibleTabsNum () == 0)
			{
				pTabbedControlBar->ShowControlBar (bShow, bDelay, bActivate);
			}
		}
		else
		{
			int iTab = pTabParent->GetTabFromHwnd (GetSafeHwnd ());
			pTabParent->ShowTab (iTab, bShow, !bDelay);
		}
	}
	else
	{
		ShowWindow (nShowCmd);
		if (!bDelay)
		{
			AdjustDockingLayout ();
		}
	}

	if (GetDockRow () != NULL)
	{
		GetDockRow ()->FixupVirtualRects (false);
	}
}
//************************************************************************************
LRESULT CBCGPBaseControlBar::OnIdleUpdateCmdUI(WPARAM wParam, LPARAM)
{
	// the style must be visible and if it is docked
	// the dockbar style must also be visible
	if ((GetStyle() & WS_VISIBLE) &&
		(m_pParentDockBar == NULL || (m_pParentDockBar->GetStyle() & WS_VISIBLE)))
	{
		CFrameWnd* pTarget = (CFrameWnd*)GetOwner();
		if (pTarget == NULL || !pTarget->IsFrameWnd())
			pTarget = BCGPGetParentFrame(this);
		if (pTarget != NULL)
			OnUpdateCmdUI(pTarget, (BOOL)wParam);
	}

	return 0L;
}
//*************************************************************************************
void CBCGPBaseControlBar::OnSize(UINT nType, int cx, int cy) 
{
	CBCGPWnd::OnSize(nType, cx, cy);

	if (m_pDockBarRow != NULL)	
	{
		m_pDockBarRow->OnResizeControlBar (this);
	}
}
//*************************************************************************************
void CBCGPBaseControlBar::Serialize (CArchive& ar)
{
	CBCGPWnd::Serialize (ar);

	if (ar.IsLoading ())
	{
		DWORD dwAlign = 0;
		ar >> dwAlign;
		m_dwStyle |= dwAlign;

		ar >> m_bRecentVisibleState;
	}
	else
	{
		ar << (m_dwStyle & CBRS_ALIGN_ANY);
		ar << IsVisible ();
	}
}
//*************************************************************************************
BOOL CBCGPBaseControlBar::LoadState (LPCTSTR lpszProfileName, int nIndex, UINT uiID)
{
	CString strProfileName = ::BCGPGetRegPath (strBaseControlBarProfile, lpszProfileName);

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

	CBCGPRegistrySP regSP;
	CBCGPRegistry& reg = regSP.Create (FALSE, TRUE);

	if (!reg.Open (strSection))
	{
		return FALSE;
	}

	reg.Read (_T ("IsVisible"), m_bRecentVisibleState);
	m_bIsRestoredFromRegistry = TRUE;

	if (m_hiconGraySmall != NULL)
	{
		::DestroyIcon(m_hiconGraySmall);
		m_hiconGraySmall = NULL;
	}

	if (m_hiconGrayLarge != NULL)
	{
		::DestroyIcon(m_hiconGrayLarge);
		m_hiconGrayLarge = NULL;
	}

	return TRUE;
}
//*************************************************************************************
BOOL CBCGPBaseControlBar::SaveState (LPCTSTR lpszProfileName, int nIndex, UINT uiID)
{

	CString strProfileName = ::BCGPGetRegPath (strBaseControlBarProfile, lpszProfileName);

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

	CBCGPRegistrySP regSP;
	CBCGPRegistry& reg = regSP.Create (FALSE, FALSE);

	if (reg.CreateKey (strSection))
	{
		BOOL bIsVisible = IsVisible ();
		reg.Write (_T ("IsVisible"), bIsVisible);
	}

	return TRUE;
}
//*************************************************************************************
CWnd* CBCGPBaseControlBar::GetDockSite () const
{
	if (m_pDockSite == NULL)
	{
		CWnd* pParentWnd = GetParent ();
		if (pParentWnd != NULL)
		{
			if (pParentWnd->IsKindOf (RUNTIME_CLASS (CDialog)) ||
				pParentWnd->IsKindOf (RUNTIME_CLASS (CBCGPAppBarWnd)))
			{
				if (pParentWnd->GetSafeHwnd() == AfxGetMainWnd()->GetSafeHwnd())
				{
					globalUtils.m_bDialogApp = TRUE;
				}
			}
		}
	}

	return m_pDockSite;
}
//*************************************************************************************
BCGP_DOCK_TYPE CBCGPBaseControlBar::GetDockMode () const 
{
	if (m_dockMode != BCGP_DT_UNDEFINED)
	{
		return m_dockMode;
	}

	return CBCGPDockManager::GetDockMode ();
}
//*************************************************************************************
HICON CBCGPBaseControlBar::GetBarIcon (BOOL bBigIcon) 
{
	HICON hiconWnd = CWnd::GetIcon (bBigIcon);

	if (!CBCGPVisualManager::GetInstance()->IsAutoGrayscaleImages())
	{
		return hiconWnd;
	}

	HICON& hIconGray = bBigIcon ? m_hiconGrayLarge : m_hiconGraySmall;

	if (hIconGray == NULL)
	{
		hIconGray = globalUtils.GrayIcon(hiconWnd);
	}

	return hIconGray == NULL ? hiconWnd : hIconGray;
}
//*************************************************************************************
BOOL CBCGPBaseControlBar::CanFloat () const 
{ 
	if (!IsTabbed ())
	{
		return m_dwBCGStyle & CBRS_BCGP_FLOAT; 
	}

	HWND hWndTab = NULL;

	CBCGPBaseTabWnd* pParentTabWnd = GetParentTabWnd (hWndTab);
	
	if (pParentTabWnd == NULL)
	{
		return m_dwBCGStyle & CBRS_BCGP_FLOAT; 
	}
	
	int nTabNum = pParentTabWnd->GetTabFromHwnd (hWndTab);
	if (nTabNum == -1)
	{
		return m_dwBCGStyle & CBRS_BCGP_FLOAT; 
	}

	return pParentTabWnd->IsTabDetachable (nTabNum);
}
//***************************************************************************************
CBCGPBaseTabWnd* CBCGPBaseControlBar::GetParentTabWnd (HWND& hWndTab) const
{
	ASSERT_VALID (this);

	const CWnd* pWndToCheck = this;

	CBCGPDockingCBWrapper* pWrapper = DYNAMIC_DOWNCAST (CBCGPDockingCBWrapper, GetParent ()); 
	if (pWrapper != NULL)
	{
		pWndToCheck = pWrapper;
		hWndTab = pWrapper->GetSafeHwnd ();
	}
	else
	{
		hWndTab = GetSafeHwnd ();
	}

	CBCGPBaseTabWnd* pParentTabWnd = DYNAMIC_DOWNCAST (CBCGPBaseTabWnd, pWndToCheck->GetParent ());
	if (pParentTabWnd == NULL)
	{
		CBCGPBaseTabbedBar* pParentTabBar = DYNAMIC_DOWNCAST (CBCGPBaseTabbedBar, pWndToCheck->GetParent ());
		if (pParentTabBar != NULL)
		{
			return pParentTabBar->GetUnderlinedWindow ();
		}
	}

	return pParentTabWnd;
}
//***************************************************************************************
CBCGPBaseTabbedBar* CBCGPBaseControlBar::GetParentTabbedBar () const
{
	HWND hWndTab = NULL;

	if (!IsTabbed ())
	{
		return NULL;
	}

	CBCGPBaseTabWnd* pTabWnd = GetParentTabWnd (hWndTab);
	if (hWndTab == NULL || pTabWnd == NULL)
	{
		return NULL;
	}

	ASSERT_VALID (pTabWnd);

	return DYNAMIC_DOWNCAST (CBCGPBaseTabbedBar, pTabWnd->GetParent ());
}
//***************************************************************************************
LRESULT CBCGPBaseControlBar::OnHelpHitTest(WPARAM, LPARAM lParam)
{
	ASSERT_VALID(this);

	INT_PTR nID = OnToolHitTest((DWORD_PTR)lParam, NULL);
	if (nID != -1)
		return HID_BASE_COMMAND+nID;

	nID = _AfxGetDlgCtrlID(m_hWnd);
	return nID != 0 ? HID_BASE_CONTROL+nID : 0;
}
//***************************************************************************************
LRESULT CBCGPBaseControlBar::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	ASSERT_VALID(this);

	LRESULT lResult;
	switch (message)
	{
	case WM_NOTIFY:
	case WM_COMMAND:
	case WM_DRAWITEM:
	case WM_MEASUREITEM:
	case WM_DELETEITEM:
	case WM_COMPAREITEM:
	case WM_VKEYTOITEM:
	case WM_CHARTOITEM:
		// send these messages to the owner if not handled
		if (OnWndMsg(message, wParam, lParam, &lResult))
			return lResult;
		else if (GetOwner()->GetSafeHwnd() != NULL)
		{
			// try owner next
			lResult = GetOwner()->SendMessage(message, wParam, lParam);

			// special case for TTN_NEEDTEXTA and TTN_NEEDTEXTW
			if(message == WM_NOTIFY)
			{
				NMHDR* pNMHDR = (NMHDR*)lParam;
				if (pNMHDR->code == TTN_NEEDTEXTA || pNMHDR->code == TTN_NEEDTEXTW)
				{
					TOOLTIPTEXTA* pTTTA = (TOOLTIPTEXTA*)pNMHDR;
					TOOLTIPTEXTW* pTTTW = (TOOLTIPTEXTW*)pNMHDR;

					if (pNMHDR->code == TTN_NEEDTEXTA)
					{
						if (pTTTA->hinst == 0 && (!pTTTA->lpszText || !*pTTTA->lpszText))
						{
							// not handled by owner, so let bar itself handle it
							lResult = CBCGPWnd::WindowProc(message, wParam, lParam);
						}
					} else if (pNMHDR->code == TTN_NEEDTEXTW)
					{
						if (pTTTW->hinst == 0 && (!pTTTW->lpszText || !*pTTTW->lpszText))
						{
							// not handled by owner, so let bar itself handle it
							lResult = CBCGPWnd::WindowProc(message, wParam, lParam);
						}
					}
				}
			}
			return lResult;
		}
	}

	// otherwise, just handle in default way
	lResult = CBCGPWnd::WindowProc(message, wParam, lParam);
	return lResult;
}
//***************************************************************************************
BOOL CBCGPBaseControlBar::PreTranslateMessage(MSG* pMsg) 
{
	ASSERT_VALID(this);
	ASSERT(m_hWnd != NULL);
	
	if (CBCGPWnd::PreTranslateMessage(pMsg))
		return TRUE;

	CWnd* pOwner = GetOwner();

	// don't translate dialog messages when in Shift+F1 help mode
	CFrameWnd* pFrameWnd = GetTopLevelFrame();
	if (pFrameWnd != NULL && pFrameWnd->m_bHelpMode)
		return FALSE;

	// since 'IsDialogMessage' will eat frame window accelerators,
	//   we call all frame windows' PreTranslateMessage first
	while (pOwner != NULL)
	{
		// allow owner & frames to translate before IsDialogMessage does
		if (pOwner->PreTranslateMessage(pMsg))
			return TRUE;

		// try parent frames until there are no parent frames
		if (IsWindow (pOwner->GetSafeHwnd ()))
		{
			pOwner = pOwner->GetParentFrame();
		}
		else
		{
			break;
		}
	}

	// filter both messages to dialog and from children
	return PreTranslateInput(pMsg);
}
//***************************************************************************************
LRESULT CBCGPBaseControlBar::HandleInitDialog(WPARAM, LPARAM)
{
	if (m_lpszBarTemplateName != NULL)
	{
		if (!ExecuteDlgInit(m_lpszBarTemplateName))
		{
			return FALSE;
		}
	}

	if (!UpdateData(FALSE))
	{
		return FALSE;
	}

	return TRUE;
}
//*******************************************************************************
LRESULT CBCGPBaseControlBar::OnSetIcon (WPARAM,LPARAM)
{
	LRESULT lres = Default ();

	if (m_hiconGraySmall != NULL)
	{
		::DestroyIcon(m_hiconGraySmall);
		m_hiconGraySmall = NULL;
	}

	if (m_hiconGrayLarge != NULL)
	{
		::DestroyIcon(m_hiconGrayLarge);
		m_hiconGrayLarge = NULL;
	}

	if (IsTabbed ())
	{
		HWND hWndTab = NULL; 
		CBCGPBaseTabWnd* pParentTab = GetParentTabWnd (hWndTab);

		ASSERT_VALID (pParentTab);

		int iTabNum = pParentTab->GetTabFromHwnd (hWndTab);
		
		if (iTabNum >= 0 && iTabNum < pParentTab->GetTabsNum ())
		{
			pParentTab->SetTabHicon (iTabNum, GetIcon (FALSE));
		}
	}

	return lres;
}
//*******************************************************************************
DWORD CBCGPBaseControlBar::GetCurrentAlignment () const
{
	return m_dwStyle & CBRS_ALIGN_ANY;
}
//*******************************************************************************
void CBCGPBaseControlBar::CopyState (CBCGPBaseControlBar* pOrgBar)
{
	ASSERT_VALID (pOrgBar);

	m_dwEnabledAlignment		= pOrgBar->GetEnabledAlignment ();
	m_bRecentVisibleState		= pOrgBar->GetRecentVisibleState ();
	m_bIsRestoredFromRegistry	= pOrgBar->IsRestoredFromRegistry ();
	m_pDockSite					= pOrgBar->GetDockSite ();
	m_rectBar					= pOrgBar->GetBarRect ();
	m_bIsDlgControl				= pOrgBar->IsDialogControl ();
	m_dwStyle					= pOrgBar->GetBarStyle ();
	m_dwBCGStyle				= pOrgBar->GetBCGStyle ();
}
//********************************************************************************
void CBCGPBaseControlBar::OnSettingChange(UINT uFlags, LPCTSTR lpszSection) 
{
	CBCGPWnd::OnSettingChange(uFlags, lpszSection);
	globalData.OnSettingChange ();
}
//*****************************************************************************
void CBCGPBaseControlBar::OnControlBarContextMenu (CWnd* pParentFrame, CPoint point)
{
	ASSERT_VALID (pParentFrame);

	if (pParentFrame->SendMessage (BCGM_TOOLBARMENU,
		(WPARAM) GetSafeHwnd (),
		MAKELPARAM(point.x, point.y)) == 0)
	{
		return;
	}

    CBCGPDockManager* pDockManager = globalUtils.GetDockManager (GetParentFrame ());
	if (pDockManager == NULL)
	{
		return;
	}

	ASSERT_VALID (pDockManager);
	pDockManager->OnControlBarContextMenu (point);
}
//****************************************************************************
LRESULT CBCGPBaseControlBar::OnPrint(WPARAM wp, LPARAM lp)
{
	DWORD dwFlags = (DWORD)lp;

	if ((dwFlags & PRF_NONCLIENT) == PRF_NONCLIENT)
	{
		CDC* pDC = CDC::FromHandle((HDC) wp);
		ASSERT_VALID(pDC);
		
		DoNcPaint(pDC);
	}

	return Default();
}
//****************************************************************************
LRESULT CBCGPBaseControlBar::OnPrintClient(WPARAM wp, LPARAM lp)
{
	DWORD dwFlags = (DWORD)lp;

	if ((dwFlags & PRF_ERASEBKGND) == PRF_ERASEBKGND)
	{
		SendMessage(WM_ERASEBKGND, wp);
	}
	
	if ((dwFlags & PRF_CLIENT) == PRF_CLIENT)
	{
		CDC* pDC = CDC::FromHandle((HDC) wp);
		ASSERT_VALID(pDC);
		
		DoPaint(pDC);
	}

	return Default();
}
