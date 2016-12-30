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
//

#include "stdafx.h"
#include "BCGPPrintPreviewView.h"
#include "BCGPStatusBar.h"
#include "BCGPRibbonBar.h"
#include "BCGPRibbonCategory.h"
#include "bcgprores.h"
#include "BCGPLocalResource.h"
#include "BCGPGlobalUtils.h"
#include "BCGPDockManager.h"
#include "BCGPMDIChildWnd.h"
#include "BCGPRibbonBackstagePagePrint.h"

IMPLEMENT_DYNCREATE(CBCGPPrintPreviewView, CPreviewView)

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static const int iSimplePaneIndex = 255;

/////////////////////////////////////////////////////////////////////////////
// CBCGPPrintPreviewToolBar

IMPLEMENT_DYNAMIC(CBCGPPrintPreviewToolBar, CBCGPToolBar)

BEGIN_MESSAGE_MAP(CBCGPPrintPreviewToolBar, CBCGPToolBar)
	//{{AFX_MSG_MAP(CBCGPPrintPreviewToolBar)
	ON_WM_CONTEXTMENU()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CBCGPPrintPreviewToolBar::OnContextMenu(CWnd* /*pWnd*/, CPoint /*pos*/)
{
	// Prevent print preview toolbar context menu appearing
}

INT_PTR CBCGPPrintPreviewToolBar::OnToolHitTest(CPoint point, TOOLINFO* pTI) const
{
	CBCGPLocalResource locaRes;
	return CBCGPToolBar::OnToolHitTest (point, pTI);
}

void CBCGPPrintPreviewToolBar::OnDestroy() 
{
	CFrameWnd* pParentFrame = BCGPGetParentFrame (this);
	ASSERT_VALID (pParentFrame);

	CBCGPDockManager* pDockManager = globalUtils.GetDockManager (pParentFrame);
	if (pDockManager != NULL)
	{
		pDockManager->RemoveControlBarFromDockManager (this, FALSE, FALSE, FALSE, NULL);
	}

	CBCGPToolBar::OnDestroy();
}
/////////////////////////////////////////////////////////////////////////////
// CBCGPPrintPreviewView

BOOL CBCGPPrintPreviewView::m_bScaleLargeImages = TRUE;

static CBCGPPrintPreviewView* g_pActivePrintPreview = NULL;
static CBCGPLocalResource* g_pPrintPreviewlocaRes = NULL;

CBCGPPrintPreviewView::CBCGPPrintPreviewView()
{
	m_iPagesBtnIndex = -1;
	m_iOnePageImageIndex = -1;
	m_iTwoPageImageIndex = -1;
	m_pWndStatusBar = NULL;
	m_pWndRibbonBar = NULL;
	m_pNumPageButton = NULL;
	m_bIsStatusBarSimple = FALSE;
	m_nSimpleType = 0;
	m_nCurrentPage = 1;
	m_recentToolbarSize.cx = m_recentToolbarSize.cy = -1;
}
//*********************************************************************************
CBCGPPrintPreviewView::~CBCGPPrintPreviewView()
{
	if (m_pWndStatusBar != NULL)
	{
		//----------------------------------
		// Restore previous StatusBar state:
		//----------------------------------
		m_pWndStatusBar->SetPaneText (iSimplePaneIndex, NULL);
	}

#ifndef BCGP_EXCLUDE_RIBBON
	if (m_pWndRibbonBar != NULL && m_pWndRibbonBar->IsVisible())
	{
		m_pWndRibbonBar->SetPrintPreviewMode (FALSE);
	}
#endif
	g_pActivePrintPreview = NULL;
}


BEGIN_MESSAGE_MAP(CBCGPPrintPreviewView, CPreviewView)
	//{{AFX_MSG_MAP(CBCGPPrintPreviewView)
	ON_WM_CREATE()
	ON_UPDATE_COMMAND_UI(AFX_ID_PREVIEW_NUMPAGE, OnUpdatePreviewNumPage)
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CBCGPPrintPreviewView message handlers

int CBCGPPrintPreviewView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CPreviewView::OnCreate(lpCreateStruct) == -1)
	{
		if (g_pPrintPreviewlocaRes != NULL)
		{
			delete g_pPrintPreviewlocaRes;
			g_pPrintPreviewlocaRes = NULL;
		}

		return -1;
	}

	g_pActivePrintPreview = this;

	ASSERT_VALID (m_pToolBar);

	CFrameWnd* pParentFrame = BCGPGetParentFrame (this);
	ASSERT_VALID (pParentFrame);

	CFrameWnd* pToplevelFrame = pParentFrame;

	if (pToplevelFrame->IsKindOf (RUNTIME_CLASS (CBCGPMDIChildWnd)))
	{
		pToplevelFrame = pToplevelFrame->GetTopLevelFrame ();
	}

#ifndef BCGP_EXCLUDE_RIBBON

	m_pWndRibbonBar = DYNAMIC_DOWNCAST (CBCGPRibbonBar,
		pToplevelFrame->GetDlgItem (AFX_IDW_RIBBON_BAR));

	if (m_pWndRibbonBar != NULL && m_pWndRibbonBar->IsVisible())
	{
		m_pWndRibbonBar->SetPrintPreviewMode ();
	}
	else
#endif
	{
		const UINT uiToolbarHotID = globalData.Is32BitIcons () ? IDR_BCGRES_PRINT_PREVIEW32 : 0;

		if (!m_wndToolBar.Create (m_pToolBar) ||
			!m_wndToolBar.LoadToolBar(	IDR_BCGRES_PRINT_PREVIEW, 0, 0, TRUE /* Locked */, 
										0, 0, uiToolbarHotID))
		{
			TRACE0("Failed to create print preview toolbar\n");
			return FALSE;      // fail to create
		}

		m_wndToolBar.SetOwner (this);

		//-------------------------------------------
		// Remember One Page/Two pages image indexes:
		//-------------------------------------------
		m_iPagesBtnIndex = m_wndToolBar.CommandToIndex (AFX_ID_PREVIEW_NUMPAGE);
		ASSERT (m_iPagesBtnIndex >= 0);
		
		CBCGPToolbarButton* pButton= m_wndToolBar.GetButton (m_iPagesBtnIndex);
		ASSERT_VALID (pButton);

		m_iOnePageImageIndex = pButton->GetImage ();

		int iIndex = m_wndToolBar.CommandToIndex (ID_BCGRES_TWO_PAGES_DUMMY);
		ASSERT (iIndex >= 0);
		
		pButton= m_wndToolBar.GetButton (iIndex);
		ASSERT_VALID (pButton);

		m_iTwoPageImageIndex = pButton->GetImage ();

		//---------------------------------
		// Remove dummy "Two pages" button:
		//---------------------------------
		m_wndToolBar.RemoveButton (iIndex);

		//------------------------------------
		// Set "Print" button to image + text:
		//------------------------------------
		m_wndToolBar.SetToolBarBtnText (m_wndToolBar.CommandToIndex (AFX_ID_PREVIEW_PRINT));

		//---------------------------------
		// Set "Close" button to text only:
		//---------------------------------
		m_wndToolBar.SetToolBarBtnText (m_wndToolBar.CommandToIndex (AFX_ID_PREVIEW_CLOSE),
			NULL, TRUE, FALSE);

		CBCGPDockManager* pDockManager = globalUtils.GetDockManager (pParentFrame);
		ASSERT_VALID (pDockManager);			
		pDockManager->AddControlBar (&m_wndToolBar, FALSE);

		//-------------------------
		// Change the Toolbar size:
		//-------------------------
		if (!m_bScaleLargeImages && m_wndToolBar.m_bLargeIcons)
		{
			m_wndToolBar.m_sizeCurButtonLocked = m_wndToolBar.m_sizeButtonLocked;
			m_wndToolBar.m_sizeCurImageLocked = m_wndToolBar.m_sizeImageLocked;
		}

		SetToolbarSize ();
	}

	//-------------------------------------------
	// Set Application Status Bar to Simple Text:
	//-------------------------------------------
	m_pWndStatusBar = DYNAMIC_DOWNCAST (CBCGPStatusBar,
		pToplevelFrame->GetDlgItem (AFX_IDW_STATUS_BAR));

	if (m_pWndStatusBar != NULL)
	{
		//-------------------------------------
		// Set Simple Pane Style to No Borders:
		//-------------------------------------
		m_pWndStatusBar->SetPaneText (iSimplePaneIndex, NULL);
	}

	if (g_pPrintPreviewlocaRes != NULL)
	{
		delete g_pPrintPreviewlocaRes;
		g_pPrintPreviewlocaRes = NULL;
	}

	return 0;
}
//*********************************************************************************
void CBCGPPrintPreviewView::OnUpdatePreviewNumPage(CCmdUI *pCmdUI) 
{
	CPreviewView::OnUpdateNumPageChange(pCmdUI);

	//--------------------------------------------------
	// Change the Icon of AFX_ID_PREVIEW_NUMPAGE button:
	//--------------------------------------------------
	UINT nPages = m_nZoomState == ZOOM_OUT ? m_nPages : m_nZoomOutPages;

#ifndef BCGP_EXCLUDE_RIBBON
	if (m_pWndRibbonBar != NULL && m_pWndRibbonBar->IsVisible())
	{
		ASSERT_VALID (m_pWndRibbonBar);

		if (m_pNumPageButton == NULL)
		{
			m_pNumPageButton = DYNAMIC_DOWNCAST (
				CBCGPRibbonButton,
				m_pWndRibbonBar->GetActiveCategory ()->FindByID (AFX_ID_PREVIEW_NUMPAGE));
		}

		if (m_pNumPageButton != NULL)
		{
			ASSERT_VALID (m_pNumPageButton);

			int nImageIndex = nPages == 1 ? 5 : 4;

			if (m_pNumPageButton->GetImageIndex (TRUE) != nImageIndex)
			{
				m_pNumPageButton->SetImageIndex (nImageIndex, TRUE);
				m_pNumPageButton->SetKeys (nPages == 1 ? _T("2") : _T("1"));
				m_pNumPageButton->Redraw ();
			}
		}
	}
	else
#endif
	if (m_wndToolBar.GetSafeHwnd () != NULL)
	{
		CBCGPToolbarButton* pButton = m_wndToolBar.GetButton (m_iPagesBtnIndex);
		ASSERT_VALID (pButton);

		pButton->SetImage (nPages == 1 ? m_iTwoPageImageIndex : m_iOnePageImageIndex);

		m_wndToolBar.InvalidateRect (pButton->Rect ());
	}
}
//*********************************************************************************
void CBCGPPrintPreviewView::OnDisplayPageNumber (UINT nPage, UINT nPagesDisplayed)
{
	ASSERT (m_pPreviewInfo != NULL);

	CFrameWnd* pParentFrame = BCGPGetParentFrame (this);
	ASSERT_VALID (pParentFrame);

	int nSubString = (nPagesDisplayed == 1) ? 0 : 1;

	CString s;
	if (AfxExtractSubString (s, m_pPreviewInfo->m_strPageDesc, nSubString))
	{
		CString strPage;

		if (nSubString == 0)
		{
			strPage.Format (s, nPage);
		}
		else
		{
			UINT nEndPage = nPage + nPagesDisplayed - 1;
			strPage.Format (s, nPage, nEndPage);
		}

		if (m_pWndStatusBar != NULL)
		{
			m_pWndStatusBar->SetPaneText (iSimplePaneIndex, strPage);
		}
		else
		{
			pParentFrame->SendMessage (WM_SETMESSAGESTRING, 0, 
										(LPARAM)(LPCTSTR) strPage);
		}
	}
	else
	{
		TRACE1("Malformed Page Description string. Could not get string %d.\n",
			nSubString);
	}
}
//*********************************************************************************
BCGCBPRODLLEXPORT void BCGPPrintPreview (CView* pView)
{
	ASSERT_VALID (pView);

#ifndef BCGP_EXCLUDE_RIBBON
	CFrameWnd* pParentFrame = BCGPGetParentFrame (pView);
	ASSERT_VALID (pParentFrame);

	CFrameWnd* pToplevelFrame = pParentFrame;

	if (pToplevelFrame->IsKindOf (RUNTIME_CLASS (CBCGPMDIChildWnd)))
	{
		pToplevelFrame = pToplevelFrame->GetTopLevelFrame ();
	}

	CBCGPRibbonBar* pWndRibbonBar = DYNAMIC_DOWNCAST (CBCGPRibbonBar,
		pToplevelFrame->GetDlgItem (AFX_IDW_RIBBON_BAR));

	if (pWndRibbonBar != NULL && pWndRibbonBar->ShowBackstagePrintView())
	{
		return;
	}
#endif

	if (g_pActivePrintPreview != NULL &&
		CWnd::FromHandlePermanent (g_pActivePrintPreview->GetSafeHwnd ()) != NULL)
	{
		return;
	}

	CPrintPreviewState *pState= new CPrintPreviewState;

	ASSERT (g_pPrintPreviewlocaRes == NULL);

	g_pPrintPreviewlocaRes = new CBCGPLocalResource;

	if (!pView->DoPrintPreview (IDD_BCGBAR_RES_PRINT_PREVIEW, pView, 
		RUNTIME_CLASS (CBCGPPrintPreviewView), pState))
	{
		TRACE0("Error: OnFilePrintPreview failed.\n");
		AfxMessageBox (AFX_IDP_COMMAND_FAILURE);
		delete pState;      // preview failed to initialize, delete State now
	}

	ASSERT (g_pPrintPreviewlocaRes == NULL);
}
//*******************************************************************************
void CBCGPPrintPreviewView::OnSize(UINT nType, int cx, int cy) 
{
	CPreviewView::OnSize(nType, cx, cy);
	
	//-------------------------
	// Change the Toolbar size:
	//-------------------------
	SetToolbarSize ();
}
//******************************************************************************
void CBCGPPrintPreviewView::SetToolbarSize ()
{
	if (m_wndToolBar.GetSafeHwnd () == NULL)
	{
		return;
	}

	ASSERT_VALID (m_pToolBar);

	CSize szSize = m_wndToolBar.CalcFixedLayout (TRUE, TRUE);

	//----------------------------------------------------------------------
	// Print toolbar should occupy the whole width of the mainframe (Win9x):
	//----------------------------------------------------------------------
	CFrameWnd* pParent = BCGPGetParentFrame (this);
	ASSERT_VALID (pParent);

	CRect rectParent;
	pParent->GetClientRect (rectParent);
	szSize.cx = rectParent.Width ();

	CRect rectToolBar;
	m_wndToolBar.GetWindowRect (rectToolBar);
	pParent->ScreenToClient (rectToolBar);

	m_pToolBar->SetWindowPos (NULL, rectToolBar.left, rectToolBar.top, szSize.cx, szSize.cy, 
				SWP_NOACTIVATE|SWP_SHOWWINDOW|SWP_NOZORDER);

	m_wndToolBar.SetWindowPos (NULL, 0, 0, szSize.cx, szSize.cy, 
				SWP_NOACTIVATE|SWP_SHOWWINDOW|SWP_NOZORDER);

	//----------------------------------------------------
	// Adjust parent toolbar (actually - dialog bar) size:
	//----------------------------------------------------
	m_pToolBar->m_sizeDefault.cy = szSize.cy;

	if (m_recentToolbarSize == szSize)
	{
		return;
	}

	m_recentToolbarSize = szSize;
	
	pParent->RecalcLayout();            // position and size everything
	pParent->UpdateWindow();
}
//******************************************************************************
BOOL CBCGPPrintPreviewView::OnEraseBkgnd(CDC* pDC) 
{
	ASSERT_VALID (pDC);

	CRect rectClient;
	GetClientRect (rectClient);

	if (CBCGPVisualManager::GetInstance ()->OnEraseMDIClientArea (pDC, rectClient))
	{
		return TRUE;
	}

	return CPreviewView::OnEraseBkgnd(pDC);
}
