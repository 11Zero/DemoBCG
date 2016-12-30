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
// BCGPTabView.cpp : implementation file
//

#include "stdafx.h"
#include "BCGPTabView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBCGPTabView

IMPLEMENT_DYNCREATE(CBCGPTabView, CView)

CBCGPTabView::CBCGPTabView()
{
	m_bIsReady = FALSE;
	m_nFirstActiveTab = -1;
}

CBCGPTabView::~CBCGPTabView()
{
}

BEGIN_MESSAGE_MAP(CBCGPTabView, CView)
	//{{AFX_MSG_MAP(CBCGPTabView)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_MOUSEACTIVATE()
	//}}AFX_MSG_MAP
	ON_REGISTERED_MESSAGE(BCGM_CHANGE_ACTIVE_TAB, OnChangeActiveTab)
	ON_MESSAGE(WM_PRINTCLIENT, OnPrintClient)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGPTabView drawing

void CBCGPTabView::OnDraw(CDC* /*pDC*/)
{
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPTabView diagnostics

#ifdef _DEBUG
void CBCGPTabView::AssertValid() const
{
	CView::AssertValid();
}

void CBCGPTabView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CBCGPTabView message handlers

int CBCGPTabView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	CRect rectDummy;
	rectDummy.SetRectEmpty ();

	// Create tabs window:
	if (!m_wndTabs.Create (
		IsScrollBar () ? 
			CBCGPTabWnd::STYLE_FLAT_SHARED_HORZ_SCROLL : CBCGPTabWnd::STYLE_FLAT, 
			rectDummy, this, 1))
	{
		TRACE0("Failed to create tab window\n");
		return -1;      // fail to create
	}

	m_wndTabs.SetFlatFrame ();
	m_wndTabs.SetTabBorderSize (0);
	m_wndTabs.AutoDestroyWindow (FALSE);
	return 0;
}
//************************************************************************************
void CBCGPTabView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);
	
	// Tab control should cover a whole client area:
	m_wndTabs.SetWindowPos (NULL, -1, -1, cx + 1, cy + 3,
		SWP_NOACTIVATE | SWP_NOZORDER);
}
//************************************************************************************
int CBCGPTabView::AddView (CRuntimeClass* pViewClass, const CString& strViewLabel,
						  int iIndex /*= -1*/, CCreateContext* pContext/* = NULL*/)
{
	ASSERT_VALID (this);
	ASSERT (pViewClass != NULL);
	ASSERT (pViewClass->IsDerivedFrom (RUNTIME_CLASS (CView)));

	CView* pView = DYNAMIC_DOWNCAST (CView, pViewClass->CreateObject ());
	ASSERT_VALID (pView);

	if (!pView->Create (NULL, _T(""), WS_CHILD | WS_VISIBLE,
					CRect (0, 0, 0, 0), &m_wndTabs, (UINT) -1, pContext))
	{
		TRACE1("CBCGPTabView:Failed to create view '%s'\n", pViewClass->m_lpszClassName);
		return -1;
	}

	CDocument* pDoc = GetDocument ();
	if (pDoc != NULL)
	{
		ASSERT_VALID (pDoc);

		BOOL bFound = FALSE;
		for (POSITION pos = pDoc->GetFirstViewPosition (); !bFound && pos != NULL;)
		{
			if (pDoc->GetNextView (pos) == pView)
			{
				bFound = TRUE;
			}
		}

		if (!bFound)
		{
			pDoc->AddView (pView);
		}
	}

	m_wndTabs.InsertTab (pView, strViewLabel, iIndex);

	int nTabs = m_wndTabs.GetTabsNum ();
	return nTabs - 1;
}
//************************************************************************************
LRESULT CBCGPTabView::OnChangeActiveTab (WPARAM wp, LPARAM)
{
	if (!m_bIsReady)
	{
		m_nFirstActiveTab = (int) wp;
		return 0;
	}

	CFrameWnd* pFrame = BCGPGetParentFrame (this);
	ASSERT_VALID (pFrame);

	int iTabNum = (int) wp;
	if (iTabNum >= 0)
	{
		CView* pView = DYNAMIC_DOWNCAST (CView, m_wndTabs.GetTabWnd (iTabNum));
		ASSERT_VALID (pView);

		pFrame->SetActiveView (pView);

		OnActivateView (pView);
	}
	else
	{
		pFrame->SetActiveView (NULL);

		OnActivateView (NULL);
	}

	return 0;
}
//************************************************************************************
int CBCGPTabView::FindTab (HWND hWndView) const
{
	ASSERT_VALID (this);

	for (int i = 0; i < m_wndTabs.GetTabsNum (); i++)
	{
		if (m_wndTabs.GetTabWnd (i)->GetSafeHwnd () == hWndView)
		{
			return i;
		}
	}

	return -1;
}
//************************************************************************************
BOOL CBCGPTabView::RemoveView (int iTabNum)
{
	ASSERT_VALID (this);
	return m_wndTabs.RemoveTab (iTabNum);
}
//************************************************************************************
BOOL CBCGPTabView::SetActiveView (int iTabNum)
{
	ASSERT_VALID (this);
	return m_wndTabs.SetActiveTab (iTabNum);
}
//************************************************************************************
CView* CBCGPTabView::GetActiveView () const
{
	ASSERT_VALID (this);

	int iActiveTab = m_wndTabs.GetActiveTab ();
	if (iActiveTab < 0)
	{
		return NULL;
	}

	return DYNAMIC_DOWNCAST (CView, m_wndTabs.GetTabWnd (iActiveTab));
}
//****************************************************************************************
LRESULT CBCGPTabView::OnPrintClient(WPARAM wp, LPARAM lp)
{
	if (lp & PRF_CLIENT)
	{
		CDC* pDC = CDC::FromHandle((HDC) wp);
		ASSERT_VALID(pDC);

		CRect rect;
		GetClientRect(rect);

		m_wndTabs.OnDraw(pDC);
	}

	return 0;
}

class CInternalTabView : public CView
{
	friend class CBCGPTabView;
};

int CBCGPTabView::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message) 
{
	CView* pCurrView = GetActiveView ();
	if (pCurrView == NULL)
	{
		return CView::OnMouseActivate (pDesktopWnd, nHitTest, message);
	}

	int nResult = CWnd::OnMouseActivate(pDesktopWnd, nHitTest, message);
	if (nResult == MA_NOACTIVATE || nResult == MA_NOACTIVATEANDEAT)
		return nResult;   // frame does not want to activate

	CFrameWnd* pParentFrame = BCGPGetParentFrame(this);
	if (pParentFrame != NULL)
	{
		// eat it if this will cause activation
		ASSERT(pParentFrame == pDesktopWnd || pDesktopWnd->IsChild(pParentFrame));

		// either re-activate the current view, or set this view to be active
		CView* pView = pParentFrame->GetActiveView();
		HWND hWndFocus = ::GetFocus();
		if (pView == pCurrView &&
			pCurrView->m_hWnd != hWndFocus && !::IsChild(pCurrView->m_hWnd, hWndFocus))
		{
			// re-activate this view
			((CInternalTabView*)pCurrView)->OnActivateView(TRUE, pCurrView, pCurrView);
		}
		else
		{
			// activate this view
			pParentFrame->SetActiveView(pCurrView);
		}
	}

	return nResult;
}
//****************************************************************************************
void CBCGPTabView::OnInitialUpdate() 
{
	CView::OnInitialUpdate();
	
	m_bIsReady = TRUE;
	OnChangeActiveTab (m_nFirstActiveTab, 0);
}
