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
//********************************************************************

#include "stdafx.h"
#include "BCGPOutlookBar.h"
#include "BCGPOutlookWnd.h"
#include "BCGPOutlookBarDockingPane.h"
#include "BCGPOutlookBarPane.h"
#include "BCGPDockManager.h"
#include "BCGPGlobalUtils.h"
#include "RegPath.h"
#include "BCGPRegistry.h"
#include "BCGPMiniFrameWnd.h"
#include "BCGPWorkspace.h"

extern CBCGPWorkspace* g_pWorkspace;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define REG_SECTION_FMT					_T("%sBCGPOutlookBar-%d")
#define REG_SECTION_FMT_EX				_T("%ssBCGPOutlookBar-%d%x")

static const CString strOutlookBarProfile	= _T ("BCGPOutlookBars");
static const CString strRegCustomPages		= _T ("BCGPOutlookCustomPages");

UINT g_nBCGPMinReservedPageID = 0xF000;

#define TOTAL_RESERVED_PAGES  100

static bool g_arTakenIDs [TOTAL_RESERVED_PAGES];

/////////////////////////////////////////////////////////////////////////////
// CBCGPOutlookBar

IMPLEMENT_SERIAL(CBCGPOutlookBar, CBCGPBaseTabbedBar, 1)

CBCGPOutlookBar::CBCGPOutlookBar()
{
	m_bMode2003 = FALSE;
	m_pFontButtons = NULL;
}
//*************************************************************************************
CBCGPOutlookBar::~CBCGPOutlookBar()
{
	while (!m_lstCustomPages.IsEmpty ()) 
	{
		delete m_lstCustomPages.RemoveHead ();
	}
}

BEGIN_MESSAGE_MAP(CBCGPOutlookBar, CBCGPBaseTabbedBar)
	//{{AFX_MSG_MAP(CBCGPOutlookBar)
	ON_WM_CREATE()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGPOutlookBar message handlers

void CBCGPOutlookBar::GetTabArea (CRect& rectTabAreaTop, CRect& rectTabAreaBottom) const
{
	rectTabAreaTop.SetRectEmpty ();
	rectTabAreaBottom.SetRectEmpty ();

	if (CanFloat ())
	{
		if (m_pTabWnd != NULL)
		{
			m_pTabWnd->GetTabArea (rectTabAreaTop, rectTabAreaBottom);
		}
	}
	else
	{
		GetClientRect (rectTabAreaTop);
		ClientToScreen (rectTabAreaTop);
	}
}
//*********************************************************************************
int CBCGPOutlookBar::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CBCGPBaseTabbedBar::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	CRect rectClient (0, 0, lpCreateStruct->cx, lpCreateStruct->cy);

	ASSERT (m_pTabWnd == NULL);
	m_pTabWnd = new CBCGPOutlookWnd;

	CBCGPOutlookWnd* pTabWnd = (CBCGPOutlookWnd*) m_pTabWnd;

	// enable this before create,a s it may change inside for dialog applications
	pTabWnd->m_bEnableWrapping = TRUE;
	
	// Create tabs window:
	if (!pTabWnd->Create (rectClient, this, 101))
	{
		TRACE0("Failed to create tab window\n");
		delete m_pTabWnd;
		m_pTabWnd = NULL;
		return -1;      // fail to create
	}
	
	pTabWnd->SetDockingBarWrapperRTC (RUNTIME_CLASS (CBCGPOutlookBarDockingPane));

	if (CanFloat ())
	{
		pTabWnd->HideSingleTab ();
	}

	return 0;
}
//*********************************************************************************
BOOL CBCGPOutlookBar::Create(LPCTSTR lpszCaption, CWnd* pParentWnd, 
							 const RECT& rect, UINT nID, 
							 DWORD dwStyle, DWORD dwBCGStyle, CCreateContext* pContext)
{
	BOOL bResult = CBCGPBaseTabbedBar::Create (lpszCaption, pParentWnd, rect, 
												FALSE, nID, dwStyle, 
												CBRS_BCGP_OUTLOOK_TABS, dwBCGStyle, pContext);
	if (!bResult)
	{
		TRACE0("Failed to create CBCGPOutlookBar\n");
		return FALSE;
	}

	if (dwBCGStyle & CBRS_BCGP_RESIZE)
	{
		EnableDocking (CBRS_ALIGN_ANY);
		DockControlBarMap (FALSE);
	}
	else
	{
		CBCGPDockManager* pManager = globalUtils.GetDockManager (pParentWnd);
		if (pManager != NULL)
		{
			pManager->AddControlBar (this);
		}
	}

	if (lpszCaption != NULL)
	{
		m_strBarName = lpszCaption;
	}

	return TRUE;
}
//*********************************************************************************
BOOL CBCGPOutlookBar::CanAcceptBar (const CBCGPBaseControlBar* pBar) const
{
	ASSERT_VALID (this);

	if (pBar == NULL || m_bMode2003)
	{
		return FALSE;
	}

	if (CanFloat ())
	{
		return CBCGPBaseTabbedBar::CanAcceptBar (pBar);
	}

	return (pBar->IsKindOf (RUNTIME_CLASS (CBCGPOutlookBarDockingPane)) ||
			pBar->IsKindOf (RUNTIME_CLASS (CBCGPOutlookBarPane)) ||
			pBar->IsKindOf (RUNTIME_CLASS (CBCGPOutlookBar)));
}
//*********************************************************************************
BCGP_CS_STATUS CBCGPOutlookBar::GetDockStatus (CPoint pt, int nSencitivity) 
{
	ASSERT_VALID (this);

	if (m_pTabWnd == NULL)
	{
		return BCGP_CS_NOTHING;
	}

	if (m_pTabWnd->GetTabsNum () == 0 || 
		m_pTabWnd->GetVisibleTabsNum () == 0)
	{
		return BCGP_CS_DELAY_DOCK_TO_TAB;
	}

	// detect caption
	UINT nHitTest = HitTest (pt, TRUE);

	CRect rectTabAreaTop;
	CRect rectTabAreaBottom;
	GetTabArea (rectTabAreaTop, rectTabAreaBottom);

	if (!rectTabAreaTop.IsRectEmpty ())
	{
		rectTabAreaTop.bottom += nSencitivity;
	}
	
	if (!rectTabAreaBottom.IsRectEmpty ())
	{
		rectTabAreaBottom.top -= nSencitivity;
	}

	if (nHitTest == HTCAPTION || rectTabAreaTop.PtInRect (pt) ||
		rectTabAreaBottom.PtInRect (pt))
	{
		// need to display "ready to create detachable tab" status
		return BCGP_CS_DELAY_DOCK_TO_TAB;
	}
	
	BCGP_CS_STATUS status = CBCGPDockingControlBar::GetDockStatus (pt, nSencitivity);

	//if the bar can't float it's a static bar and it can't accept 
	// anything but dock to tabs

	if (!CanFloat () && status == BCGP_CS_DELAY_DOCK)
	{
		return BCGP_CS_NOTHING;
	}

	return status;
}
//*********************************************************************************
void CBCGPOutlookBar::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	CBCGPMemDC memDC (dc, this);
	
	CRect rectClient;
	GetClientRect (rectClient);

	CBCGPVisualManager::GetInstance ()->OnFillBarBackground (&memDC.GetDC (), this,
		rectClient, rectClient);
}
//*********************************************************************************
CBCGPOutlookBarPane* CBCGPOutlookBar::CreateCustomPage (LPCTSTR lpszPageName,
														BOOL bActivatePage,
														DWORD dwEnabledDocking, 
														BOOL bEnableTextLabels)
{
	ASSERT (lpszPageName != NULL);

	if (m_bMode2003)
	{
		ASSERT (FALSE);
		return NULL;
	}

	CBCGPOutlookWnd* pOutlookWnd = (CBCGPOutlookWnd*) GetUnderlinedWindow ();

	ASSERT_VALID (pOutlookWnd);

	UINT uiPageID = FindAvailablePageID ();

	if (uiPageID == 0xFFFF)
	{
		TRACE0("There is no page ID available!\n");
		return NULL;
	}

	CBCGPOutlookBarPane* pNewPage = new CBCGPOutlookBarPane;
	pNewPage->Create (this, dwDefaultToolbarStyle | CBRS_FLOAT_MULTI, uiPageID);
	pNewPage->SetOwner (GetOwner ());
	pNewPage->EnableDocking (dwEnabledDocking);
	pNewPage->EnableTextLabels (bEnableTextLabels);
	pOutlookWnd->AddTab (pNewPage, lpszPageName);

	m_lstCustomPages.AddTail (pNewPage);
	g_arTakenIDs [uiPageID - g_nBCGPMinReservedPageID] = true;

	if (bActivatePage)
	{
		pOutlookWnd->SetActiveTab (pOutlookWnd->m_iTabsNum - 1);
	}

	return pNewPage;
}
//*********************************************************************************
BOOL CBCGPOutlookBar::RemoveCustomPage (UINT uiPage, 
										CBCGPOutlookWnd* pTargetWnd)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pTargetWnd);

	if (m_bMode2003)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	CBCGPOutlookWnd* pOutlookBar = 
		DYNAMIC_DOWNCAST (CBCGPOutlookWnd, GetUnderlinedWindow ());
	if (pOutlookBar == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	CBCGPOutlookBarPane* pPage = 
			DYNAMIC_DOWNCAST (CBCGPOutlookBarPane, 
								pTargetWnd->GetTabWndNoWrapper (uiPage));
	if (pPage == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	POSITION pos = m_lstCustomPages.Find (pPage);
	if (pos != NULL)
	{
		UINT uID = pPage->GetDlgCtrlID ();

		if (uID >= g_nBCGPMinReservedPageID && 
			uID < g_nBCGPMinReservedPageID + TOTAL_RESERVED_PAGES)
		{
			g_arTakenIDs [uID - g_nBCGPMinReservedPageID] = false;
			m_lstCustomPages.RemoveAt (pos);

			BOOL bSave = pTargetWnd->m_bAutoDestoyWindow;

			pTargetWnd->m_bAutoDestoyWindow = TRUE;
			pTargetWnd->RemoveTab (uiPage);
			pTargetWnd->m_bAutoDestoyWindow = bSave;

			delete pPage;
			return TRUE;
		}
	}

	return FALSE;
}
//*********************************************************************************
BOOL CBCGPOutlookBar::LoadState (LPCTSTR lpszProfileName, int nIndex, UINT uiID)
{
	CBCGPBaseTabbedBar::LoadState (lpszProfileName, nIndex, uiID);

	CString strProfileName = ::BCGPGetRegPath (strOutlookBarProfile, lpszProfileName);

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

	if (!reg.Read (strRegCustomPages, &lpbData, &uiDataSize))
	{
		return FALSE;
	}

	CBCGPOutlookWnd* pOutlookBar = (CBCGPOutlookWnd*) GetUnderlinedWindow ();

	try
	{
		CMemFile file (lpbData, uiDataSize);
		CArchive ar (&file, CArchive::load);

		int nCount = 0;
		ar >> nCount;

		for (int i = 0; i < nCount; i++)
		{
			int nID = 0;
			CString strName;
			ar >> nID;
			ar >> strName;

			CBCGPOutlookBarPane* pPage = new CBCGPOutlookBarPane ();
			pPage->Create (this, dwDefaultToolbarStyle, nID);
			pPage->SetOwner (GetOwner ());

			pPage->LoadState (lpszProfileName, nID,  nID);

			m_lstCustomPages.AddTail (pPage);
			pOutlookBar->AddTab (pPage, strName);

			g_arTakenIDs [nID - g_nBCGPMinReservedPageID] = true;
		}

		if (g_pWorkspace != NULL && g_pWorkspace->GetDataVersion () >= 0x60710 && g_pWorkspace->GetDataVersion () != 0x70000)
		{
			int nVisiblePages = 0;
			ar >> nVisiblePages;

			pOutlookBar->SetVisiblePageButtons (nVisiblePages);
		}
	}
	catch (CMemoryException* pEx)
	{
		pEx->Delete ();
		TRACE(_T("Memory exception in CBCGPOutlookBar::SaveState ()!\n"));
	}
	catch (CArchiveException* pEx)
	{
		pEx->Delete ();
		TRACE(_T("Archive exception in CBCGPOutlookBar::LoadState ()!\n"));
	}

	free (lpbData);
	return TRUE;
}
//*********************************************************************************
BOOL CBCGPOutlookBar::SaveState (LPCTSTR lpszProfileName, int nIndex, UINT uiID)
{
	CBCGPBaseTabbedBar::SaveState (lpszProfileName, nIndex, uiID);

	for (POSITION pos = m_lstCustomPages.GetHeadPosition (); pos != NULL;)
	{
		CBCGPOutlookBarPane* pPage = (CBCGPOutlookBarPane*)m_lstCustomPages.GetNext (pos);
		ASSERT_VALID (pPage);
		int nID = pPage->GetDlgCtrlID ();
		pPage->SaveState (lpszProfileName, nID, nID);
	}

	CString strProfileName = ::BCGPGetRegPath (strOutlookBarProfile, lpszProfileName);

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

			ar << (int) m_lstCustomPages.GetCount ();
			for (POSITION pos = m_lstCustomPages.GetHeadPosition (); pos != NULL;)
			{
				CBCGPOutlookBarPane* pPage = (CBCGPOutlookBarPane*)m_lstCustomPages.GetNext (pos);
				ASSERT_VALID (pPage);

				ar << pPage->GetDlgCtrlID ();
				
				CString strName;
				if (pPage->IsTabbed ())
				{
					pPage->GetWindowText (strName);
				}
				else
				{
					pPage->GetParent ()->GetWindowText (strName);
				}

				ar << strName;
			}
			
			CBCGPOutlookWnd* pOutlookBar = (CBCGPOutlookWnd*) GetUnderlinedWindow ();
			if (pOutlookBar != NULL)
			{
				ar << pOutlookBar->GetVisiblePageButtons ();
			}
			else
			{
				ar << -1;
			}

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
				reg.Write (strRegCustomPages, lpbData, uiDataSize);
			}

			free (lpbData);
		}
	}
	catch (CMemoryException* pEx)
	{
		pEx->Delete ();
		TRACE(_T("Memory exception in CBCGPOutlookBar::SaveState ()!\n"));
	}
	catch (CArchiveException* pEx)
	{
		pEx->Delete ();
		TRACE(_T("Archive exception in CBCGPOutlookBar::SaveState ()!\n"));
	}

	return TRUE;
}
//*********************************************************************************
UINT CBCGPOutlookBar::FindAvailablePageID ()
{
	for (UINT ui = 0; ui < TOTAL_RESERVED_PAGES; ui++)
	{
		if (!g_arTakenIDs [ui])
		{
			return ui + g_nBCGPMinReservedPageID;
		}
	}

	return 0xFFFF;
}
//********************************************************************************
void CBCGPOutlookBar::SetMode2003 (BOOL bMode2003/* = TRUE*/)
{
	ASSERT_VALID (this);

	if (GetSafeHwnd () != NULL)
	{
		ASSERT (FALSE);
		return;
	}

	m_bMode2003 = bMode2003;
}
//*******************************************************************************
void CBCGPOutlookBar::SetButtonsFont (CFont* pFont, BOOL bRedraw/* = TRUE*/)
{
	ASSERT_VALID (this);
	m_pFontButtons = pFont;

	CBCGPBaseTabWnd* pTabs = GetUnderlinedWindow();
	if (pTabs->GetSafeHwnd() != NULL)
	{
		pTabs->SetFont(m_pFontButtons, bRedraw);

		if (!bRedraw)
		{
			pTabs->SetTabsHeight();
		}
	}

	if (bRedraw && GetSafeHwnd () != NULL)
	{
		RedrawWindow (NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_FRAME | RDW_ALLCHILDREN);
	}
}
//*******************************************************************************
BOOL CBCGPOutlookBar::FloatTab (CWnd* pBar, int nTabID, 
									  BCGP_DOCK_METHOD dockMethod, 
									  BOOL bHide)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pBar);

	CBCGPOutlookWnd* pOutlookWnd = DYNAMIC_DOWNCAST (CBCGPOutlookWnd, GetUnderlinedWindow ());

	ASSERT_VALID (pOutlookWnd);

	if (pOutlookWnd->GetTabsNum () > 1)
	{
		return CBCGPBaseTabbedBar::FloatTab (pBar, nTabID, dockMethod, bHide);
	}

	return FALSE;
}
