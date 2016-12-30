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
// BCGPSlider.cpp : implementation file
//

#include "stdafx.h"

#include "BCGGlobals.h"
#include "BCGPSlider.h"
#include "BCGPDockingControlBar.h"
#include "BCGPBarContainerManager.h"
#include "BCGPBarContainer.h"
#include "BCGPGlobalUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CBCGPSlider,CBCGPBaseControlBar)

int CBCGPSlider::m_nDefaultWidth = 4;
CRuntimeClass* CBCGPSlider::m_pContainerManagerRTC = RUNTIME_CLASS (CBCGPBarContainerManager);
CRuntimeClass* CBCGPSlider::m_pSliderRTC = RUNTIME_CLASS (CBCGPSlider);

class CBCGPTrackingWnd : public CWnd
{
public:
	CBCGPTrackingWnd()
	{
	}

	BOOL Create(const CRect& rect, CWnd* pWndOwner)
	{
		CString strClassName = globalData.RegisterWindowClass (_T("BCGPTrackingWnd"));
		
		if (!CWnd::CreateEx(WS_EX_TOOLWINDOW | WS_EX_TOPMOST, 
			strClassName, 
			_T (""),
			WS_POPUP | MFS_SYNCACTIVE, 
			rect,
			pWndOwner == NULL ? AfxGetMainWnd() : pWndOwner,
			0))
		{
			return FALSE;
		}
		
		if (pWndOwner != NULL)
		{
			SetOwner(pWndOwner);
		}
		
		ShowWindow(SW_SHOWNOACTIVATE);
		return TRUE;
	}

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPTrackingWnd)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

BEGIN_MESSAGE_MAP(CBCGPTrackingWnd, CWnd)
//{{AFX_MSG_MAP(CBCGPTrackingWnd)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CBCGPTrackingWnd::OnEraseBkgnd(CDC* /*pDC*/) 
{
	return TRUE;
}

void CBCGPTrackingWnd::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	CRect rectClient;
	GetClientRect(&rectClient);

	::FillRect(dc.GetSafeHdc(), &rectClient, globalData.hbrBtnShadow);
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPSlider

CBCGPSlider::CBCGPSlider ()
{
	Init ();
}
//--------------------------------------------------------------------------//
CBCGPSlider::CBCGPSlider (BOOL bDefaultSlider, CWnd* pParentWnd) 
{
	Init (bDefaultSlider, pParentWnd);
}
//--------------------------------------------------------------------------//
void CBCGPSlider::Init (BOOL bDefaultSlider, CWnd* pParentWnd)
{
	m_nID = (UINT) -1;
	m_dwSliderStyle = 0;
	m_nWidth = 0;
	m_bCaptured = false; 
	m_pContainerManager = NULL;
	m_bDefaultSlider = bDefaultSlider;

	m_rectLastDragRect.SetRectEmpty ();	
	m_rectDragBounds.SetRectEmpty ();
	m_pWndTrack = NULL;

	m_nMinOffset = 0;  
	m_nMaxOffset = 0; 
	m_nStep		 = -1;
	m_bAutoHideMode = FALSE;
	m_pParentWndForSerialize = pParentWnd;
}
//--------------------------------------------------------------------------//
CBCGPSlider::~CBCGPSlider()
{
	
}
//--------------------------------------------------------------------------//
BEGIN_MESSAGE_MAP(CBCGPSlider, CBCGPBaseControlBar)
	//{{AFX_MSG_MAP(CBCGPSlider)
	ON_WM_SIZE()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_NCDESTROY()
	ON_WM_DESTROY()
	ON_WM_CREATE()
	ON_WM_CANCELMODE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGPSlider message handlers
//--------------------------------------------------------------------------//
BOOL CBCGPSlider::Create (DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, 
						 CCreateContext* pContext) 
{
	ASSERT_VALID (this);

	
	return CBCGPSlider::CreateEx(0L, dwStyle, rect, pParentWnd, nID, pContext);
}
//--------------------------------------------------------------------------//
BOOL CBCGPSlider::CreateEx(DWORD dwStyleEx, DWORD dwStyle, const RECT& rect, 
						  CWnd* pParentWnd, UINT nID, CCreateContext* pContext)
{
	ASSERT_VALID (this);

	m_nID = nID;
	m_dwSliderStyle = dwStyle;

	if (m_dwSliderStyle & SS_VERT)
	{
		m_nWidth = rect.right - rect.left;
	}
	else if (m_dwSliderStyle & SS_HORZ)
	{
		m_nWidth = rect.bottom - rect.top;
	}

	DWORD dwSliderStyle = m_dwSliderStyle | WS_CHILDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

	if (m_bDefaultSlider)
	{
		ASSERT (m_pContainerManagerRTC != NULL);
		m_pContainerManager = DYNAMIC_DOWNCAST (CBCGPBarContainerManager, m_pContainerManagerRTC->CreateObject ());

		ASSERT (m_pContainerManager != NULL);
		m_pContainerManager->Create (pParentWnd, this);
	}

	m_pDockSite = DYNAMIC_DOWNCAST (CFrameWnd, pParentWnd);

	if (m_pDockSite == NULL)
	{
		m_pDockSite = BCGPGetParentFrame (pParentWnd);
	}

	if (m_pDockSite == NULL)
	{
		ASSERT (FALSE);
	}

	return CWnd::CreateEx (dwStyleEx, 
		globalData.RegisterWindowClass (_T("BCGPSlider")), NULL, dwSliderStyle, rect, pParentWnd, nID, pContext);
}
//--------------------------------------------------------------------------//
void CBCGPSlider::Serialize (CArchive& ar)
{
	ASSERT_VALID (this);
	CBCGPBaseControlBar::Serialize (ar);

	CRect rect;
	
	if (ar.IsStoring ())
	{
		GetWindowRect (rect);
		GetParent ()->ScreenToClient (rect);

		ar << m_nID;
		ar << m_nStep;
		ar << rect;
		ar << IsWindowVisible ();	
		ar << m_dwSliderStyle;
		ar << m_nWidth;
		ar << m_bDefaultSlider;
		ar << m_nMinOffset;
		ar << m_nMaxOffset;
	}
	else
	{
		BOOL bVisible = FALSE;

		ar >> m_nID;
		ar >> m_nStep;
		ar >> rect;
		ar >> bVisible;
		ar >> m_dwSliderStyle;
		ar >> m_nWidth;
		ar >> m_bDefaultSlider;
		ar >> m_nMinOffset;
		ar >> m_nMaxOffset;

		if (bVisible)
		{
			m_dwSliderStyle |= WS_VISIBLE;
		}
		else
		{
			m_dwSliderStyle &= ~WS_VISIBLE;
		}

		if (!CreateEx (0, m_dwSliderStyle, rect, m_pParentWndForSerialize, m_nID, NULL))
		{
			TRACE0 ("Unable to create slider from archive");
		}
	}

	if (m_pContainerManager != NULL && m_bDefaultSlider)
	{
		m_pContainerManager->Serialize (ar);
	}

}
//--------------------------------------------------------------------------//
CBCGPDockingControlBar* CBCGPSlider::FindTabbedBar (UINT nID)
{
	ASSERT_VALID (this);
	if (m_pContainerManager != NULL)
	{
		return m_pContainerManager->FindTabbedBar (nID);
	}
	return NULL;
}
//--------------------------------------------------------------------------//
void CBCGPSlider::OnSize(UINT nType, int cx, int cy) 
{
	ASSERT_VALID (this);
	CWnd::OnSize(nType, cx, cy);
}
//--------------------------------------------------------------------------//
BOOL CBCGPSlider::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	ASSERT_VALID (this);

	switch (nHitTest)
	{
	case HTCLIENT:
		if (m_dwSliderStyle & SS_HORZ)
		{
			SetCursor (globalData.m_hcurStretchVert);
		}
		else if (m_dwSliderStyle & SS_VERT) 
		{
			SetCursor (globalData.m_hcurStretch);
		}
		return TRUE;
	}

	return CWnd::OnSetCursor(pWnd, nHitTest, message);
}
//--------------------------------------------------------------------------//
void CBCGPSlider::OnLButtonDown(UINT nFlags, CPoint point) 
{
	ASSERT_VALID (this);
	if (!m_bCaptured)
	{
		m_bCaptured = true;
		SetCapture ();

		CRect rectSlider;
		GetWindowRect (rectSlider);
		
		ASSERT(m_pWndTrack == NULL);
		m_pWndTrack = new CBCGPTrackingWnd();
		if (!m_pWndTrack->Create(rectSlider, this))
		{
			ASSERT(FALSE);
		}

		m_rectLastDragRect = rectSlider;
		m_rectDragBounds = rectSlider;

		if (m_pContainerManager != NULL)
		{
			m_pContainerManager->GetMinMaxOffset (this, m_nMinOffset, m_nMaxOffset, m_nStep);
			
			if (IsHorizontal ())
			{
				m_rectDragBounds.top = rectSlider.top + m_nMinOffset;
				m_rectDragBounds.bottom = rectSlider.bottom + m_nMaxOffset;
			}
			else
			{
				m_rectDragBounds.left = rectSlider.left + m_nMinOffset;
				m_rectDragBounds.right = rectSlider.right + m_nMaxOffset;
			}

			m_pContainerManager->SetResizeMode (TRUE);
		}
	}
	
	CWnd::OnLButtonDown(nFlags, point);
}
//--------------------------------------------------------------------------//
void CBCGPSlider::OnLButtonUp(UINT nFlags, CPoint point) 
{
	ASSERT_VALID (this);
	
	StopTracking (TRUE);
	Sleep(20);
	CWnd::OnLButtonUp(nFlags, point);

	Invalidate();
	UpdateWindow();
}
//--------------------------------------------------------------------------//
void CBCGPSlider::OnMouseMove(UINT nFlags, CPoint point) 
{
	ASSERT_VALID (this);
	if (m_bCaptured)
	{
		CRect rectNew = m_rectLastDragRect;

		CPoint ptNew;
		GetCursorPos (&ptNew);

		if (m_dwSliderStyle & SS_VERT)
		{
			rectNew.left = ptNew.x - m_nWidth / 2;
			rectNew.right = rectNew.left + m_nWidth;
			if (rectNew.left < m_rectDragBounds.left)
			{
				rectNew.left = m_rectDragBounds.left;
				rectNew.right = rectNew.left + m_rectLastDragRect.Width ();
			}

			if (rectNew.right > m_rectDragBounds.right)
			{
				rectNew.right = m_rectDragBounds.right;
				rectNew.left = rectNew.right - m_rectLastDragRect.Width ();
			}
		}
		else
		{
			rectNew.top = ptNew.y - m_nWidth / 2;
			rectNew.bottom = rectNew.top + m_nWidth;
			if (rectNew.top < m_rectDragBounds.top)
			{
				rectNew.top = m_rectDragBounds.top;
				rectNew.bottom = rectNew.top + m_nWidth;
			}

			if (rectNew.bottom > m_rectDragBounds.bottom)
			{
				rectNew.bottom = m_rectDragBounds.bottom;
				rectNew.top = rectNew.bottom - m_nWidth;
			}
		}
		
		if (m_pWndTrack->GetSafeHwnd() != NULL)
		{
			m_pWndTrack->SetWindowPos(&CWnd::wndTopMost, rectNew.left, rectNew.top, -1, -1, SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOOWNERZORDER);
		}

		m_rectLastDragRect = rectNew;
	}
	
	CWnd::OnMouseMove(nFlags, point);
}
//--------------------------------------------------------------------------//
void CBCGPSlider::OnCancelMode ()
{
	StopTracking (FALSE);
	CBCGPBaseControlBar::OnCancelMode ();
	Invalidate();
	UpdateWindow();
}
//--------------------------------------------------------------------------//
void CBCGPSlider::StopTracking (BOOL bMoveSlider)
{
	if (m_bCaptured)
	{
		CRect rectSlider;
		GetWindowRect (rectSlider);

		CPoint ptOffset = m_rectLastDragRect.TopLeft () - rectSlider.TopLeft ();

		if (m_pWndTrack->GetSafeHwnd() != NULL)
		{
			m_pWndTrack->DestroyWindow();
			delete m_pWndTrack;
			m_pWndTrack = NULL;
		}
		
		if (bMoveSlider)
		{
			MoveSlider (ptOffset);	
		}
		
		m_rectLastDragRect.SetRectEmpty ();

		ReleaseCapture ();
		m_bCaptured = false;

		if (m_pContainerManager != NULL)
		{
			m_pContainerManager->SetResizeMode (FALSE);
		}
	}
}
//--------------------------------------------------------------------------//
void CBCGPSlider::OnPaint() 
{
	ASSERT_VALID (this);
	CPaintDC dc(this); // device context for painting
	CBCGPMemDC memDC (dc, this);

	CRect rectClient;
	GetClientRect (rectClient);
	CBCGPVisualManager::GetInstance ()->OnDrawSlider (&memDC.GetDC (), this, rectClient, m_bAutoHideMode);
}
//--------------------------------------------------------------------------//
void CBCGPSlider::MoveSlider (CPoint& ptOffset, BOOL /*bAdjustLayout*/)
{
	ASSERT_VALID (this);

	BOOL bIsRTL = GetParent ()->GetExStyle () & WS_EX_LAYOUTRTL;
	
	CRect rectSlider;
	CRect rectSliderWnd;
	GetWindowRect (rectSlider);
	GetParent ()->ScreenToClient (rectSlider);
	rectSliderWnd = rectSlider;

	int nOffset = 0;
	if (m_dwSliderStyle & SS_VERT)
	{
		nOffset = bIsRTL ? -ptOffset.x : ptOffset.x;
		//nOffset = ptOffset.x;
		rectSlider.OffsetRect (nOffset, 0);
	}
	else if (m_dwSliderStyle & SS_HORZ)
	{
		nOffset = ptOffset.y;
		rectSlider.OffsetRect (0, nOffset);
	}
	else
	{
		return;
	}

	HDWP hdwp = BeginDeferWindowPos (50);
	if (m_pContainerManager != NULL)
	{
		m_pContainerManager->OnSliderMove (this, 0, nOffset, hdwp);
	}
	EndDeferWindowPos (hdwp);
	
	// it moves the slider
	AdjustDockingLayout ();
	

//	CBCGPGlobalUtils::ScreenToClientUnmapped (GetParent (), rectSlider);
/*
	int nLeftBound = rectSlider.left;
	if (bIsRTL && m_dwSliderStyle & SS_VERT)
	{
		GetParent ()->ScreenToClient (rectSliderWnd);
		nLeftBound = rectSliderWnd.left - ptOffset.x;
	}
*/	
	// move the slider by ourself
	SetWindowPos (NULL, rectSlider.left, rectSlider.top, 
					rectSlider.Width (), rectSlider.Height (), SWP_NOZORDER | SWP_NOACTIVATE);
}
//--------------------------------------------------------------------------//
void CBCGPSlider::AddControlBar  (CBCGPDockingControlBar* pBar)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pBar);
	ASSERT_KINDOF (CBCGPDockingControlBar, pBar);

	m_pContainerManager->AddControlBar (pBar);
	CheckVisibility ();
}
//--------------------------------------------------------------------------//
CBCGPDockingControlBar*  CBCGPSlider::AddRecentControlBar (CBCGPDockingControlBar* pBar)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pBar);
	ASSERT_KINDOF (CBCGPDockingControlBar, pBar);

	CBCGPDockingControlBar* pAddedBar = NULL;
	CBCGPBarContainer* pRecentContainer = pBar->m_recentDockInfo.GetRecentContainer (TRUE);
	CBCGPBarContainer* pRecentTabContainer = pBar->m_recentDockInfo.GetRecentTabContainer (TRUE);
	if (pRecentContainer != NULL)
	{
		pAddedBar = m_pContainerManager->AddControlBarToRecentContainer (pBar, pRecentContainer);
		CheckVisibility ();
	}
	else if (pRecentTabContainer != NULL)
	{
		pAddedBar = m_pContainerManager->AddControlBarToRecentContainer (pBar, pRecentTabContainer);
		CheckVisibility ();
	}
	else
	{
		ASSERT (FALSE);
	}
	return pAddedBar;
}
//--------------------------------------------------------------------------//
BOOL CBCGPSlider::InsertControlBar	(CBCGPDockingControlBar* pBarToInsert, 
									 CBCGPDockingControlBar* pTargetBar, 
									 DWORD dwAlignment, 
									 LPCRECT lpRect)
{
	ASSERT_VALID (this);
	ASSERT_KINDOF (CBCGPDockingControlBar, pBarToInsert);
	ASSERT_KINDOF (CBCGPDockingControlBar, pTargetBar);

	BOOL bResult = FALSE;
	if (m_pContainerManager != NULL)
	{
		bResult = m_pContainerManager->InsertControlBar (pBarToInsert, pTargetBar, 
														dwAlignment, lpRect);
		CheckVisibility ();
	}
	return bResult;
}
//--------------------------------------------------------------------------//
const CBCGPBaseControlBar* CBCGPSlider::GetFirstBar () const
{
	ASSERT_VALID (this);
	if (m_pContainerManager != NULL)
	{
		return m_pContainerManager->GetFirstBar ();
	}
	return NULL;
}
//--------------------------------------------------------------------------//
void CBCGPSlider::RemoveControlBar (CBCGPDockingControlBar* pBar)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pBar);
	ASSERT_KINDOF (CBCGPDockingControlBar, pBar);

	if (m_pContainerManager != NULL)
	{
		// remove the bar from container
		m_pContainerManager->RemoveControlBarFromContainer (pBar);
		// remove the bar from docksite (do not destroy!!!)
		CBCGPBaseControlBar::RemoveControlBarFromDockManager (pBar, FALSE, FALSE, m_bAutoHideMode);
		if (m_pContainerManager->IsEmpty () && 
			m_pContainerManager->GetTotalRefCount () == 0 &&
			pBar->m_recentDockInfo.GetRecentDefaultSlider () != this)
		{
			// it was the last control bar in the container - 
			// remove and DESTROY  the slider as well
			CBCGPBaseControlBar::RemoveControlBarFromDockManager (this, TRUE, FALSE, m_bAutoHideMode);
		}
		else
		{
			if (!CheckVisibility ())
			{
				ShowWindow (SW_HIDE);
			}
		}
	}
}
//--------------------------------------------------------------------------//
BOOL CBCGPSlider::ReplaceControlBar	(CBCGPDockingControlBar* pBarToReplace, 
									 CBCGPDockingControlBar* pBarToReplaceWith)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pBarToReplace);
	ASSERT_VALID (pBarToReplaceWith);
	ASSERT_KINDOF (CBCGPDockingControlBar, pBarToReplaceWith);

	if (m_pContainerManager == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	pBarToReplaceWith->SetDefaultSlider (m_hWnd);
	
	BOOL bResult = m_pContainerManager->ReplaceControlBar (pBarToReplace, pBarToReplaceWith);
	CheckVisibility ();
	return bResult;
}
//--------------------------------------------------------------------------//
BOOL CBCGPSlider::OnEraseBkgnd(CDC* /*pDC*/) 
{
	return TRUE;
}
//*******************************************************************************
CSize CBCGPSlider::CalcFixedLayout(BOOL /*bStretch*/, BOOL /*bHorz*/)
{
	ASSERT_VALID (this);
	CRect rectWnd;
	GetWindowRect (&rectWnd);
	CSize size = rectWnd.Size ();

	CRect rectContainer;
	if (m_pContainerManager != NULL)
	{
		m_pContainerManager->GetWindowRect (rectContainer);
		size += rectContainer.Size ();
	}
	return size;
}
//*******************************************************************************
void CBCGPSlider::RepositionBars	(CRect& rectNew, HDWP& hdwp)
{
	CRect rectWnd;
	GetWindowRect (&rectWnd);

	CRect rectContainer = rectNew; 
	CRect rectNewSlider = rectNew;
	
	DWORD dwAlignment = GetCurrentAlignment ();

	switch (dwAlignment)
	{
	case CBRS_ALIGN_LEFT:
		rectNewSlider.left = rectNew.right - m_nWidth;
		rectContainer.right = rectNewSlider.left;
		rectContainer.top = rectNewSlider.top;
		rectContainer.bottom = rectNewSlider.bottom;
		break;
	case CBRS_ALIGN_RIGHT:
		rectNewSlider.right = rectNew.left + m_nWidth;
		rectContainer.left = rectNewSlider.right;
		rectContainer.top = rectNewSlider.top;
		rectContainer.bottom = rectNewSlider.bottom;
		break;
	case CBRS_ALIGN_TOP:
		rectNewSlider.top = rectNew.bottom - m_nWidth;
		rectContainer.bottom = rectNewSlider.top;
		rectContainer.left = rectNewSlider.left;
		rectContainer.right = rectNewSlider.right;
		break;
	case CBRS_ALIGN_BOTTOM:
		rectNewSlider.bottom = rectNew.top + m_nWidth;
		rectContainer.top = rectNewSlider.bottom;
		rectContainer.left = rectNewSlider.left;
		rectContainer.right = rectNewSlider.right;
		break;
	}

	CWnd* pParentWnd = GetParent ();
	ASSERT_VALID (pParentWnd);

	pParentWnd->ScreenToClient (rectNew);
	pParentWnd->ScreenToClient (rectNewSlider);
	pParentWnd->ScreenToClient (rectContainer);
	hdwp = MoveWindow (rectNewSlider, TRUE, hdwp);

	if (m_pContainerManager != NULL)
	{
		m_pContainerManager->ResizeBarContainers (rectContainer, hdwp);
		CSize sizeMin;
		m_pContainerManager->GetMinSize (sizeMin);

		if (CBCGPControlBar::m_bHandleMinSize)
		{
			CObList lstBars;
			CObList lstSliders;
			m_pContainerManager->AddControlBarsToList(&lstBars, &lstSliders);

			if (rectContainer.Width () < sizeMin.cx ||
				rectContainer.Height () < sizeMin.cy)
			{
				
				POSITION pos = NULL;
				for (pos = lstBars.GetHeadPosition (); pos != NULL;)
				{
					CBCGPDockingControlBar* pBar = DYNAMIC_DOWNCAST (CBCGPDockingControlBar, lstBars.GetNext (pos));
					ASSERT_VALID (pBar);

					CRect rectWnd;
					pBar->GetWindowRect (rectWnd);
					pParentWnd->ScreenToClient (rectWnd);
					if (rectWnd.right > rectContainer.right)
					{
						rectWnd.right = rectContainer.right;
					}
					if (rectWnd.bottom > rectContainer.bottom)
					{
						rectWnd.bottom = rectContainer.bottom;
					}
					rectWnd.OffsetRect (-rectWnd.left, -rectWnd.top);
					CRgn rgn;
					rgn.CreateRectRgn (rectWnd.left, rectWnd.top, rectWnd.right, rectWnd.bottom);
					pBar->SetWindowRgn (rgn, TRUE);
				}

				for (pos = lstSliders.GetHeadPosition (); pos != NULL;)
				{
					CBCGPSlider* pSlider = DYNAMIC_DOWNCAST (CBCGPSlider, lstBars.GetNext (pos));
					ASSERT_VALID (pSlider);

					pSlider->SetWindowPos (&CWnd::wndBottom, -1, -1, -1, -1, 
						SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
				}
			}
			else
			{
				for (POSITION pos = lstBars.GetHeadPosition (); pos != NULL;)
				{
					CBCGPDockingControlBar* pBar = (CBCGPDockingControlBar*) lstBars.GetNext (pos);
					pBar->SetWindowRgn (NULL, TRUE);
				}
			}
		}
	}
}
//*******************************************************************************
void CBCGPSlider::OnDestroy() 
{
	if (m_pWndTrack->GetSafeHwnd() != NULL)
	{
		m_pWndTrack->DestroyWindow();
		delete m_pWndTrack;
		m_pWndTrack = NULL;
	}

	CBCGPBaseControlBar::OnDestroy();
}
//*******************************************************************************
void CBCGPSlider::OnNcDestroy() 
{
	if (m_pContainerManager != NULL)
	{
		m_pContainerManager->RemoveSlider (this);
	}

	if (m_pContainerManager != NULL && m_bDefaultSlider)
	{
		delete m_pContainerManager;
		m_pContainerManager = NULL;
	}

	CBCGPBaseControlBar::OnNcDestroy();
	delete this;		
}
//*******************************************************************************
void CBCGPSlider::ShowWindow (int nCmdShow)
{
	CWnd::ShowWindow (nCmdShow);
}
//*******************************************************************************
int CBCGPSlider::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CBCGPBaseControlBar::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (globalData.m_hcurStretch == NULL)
	{
		globalData.m_hcurStretch = AfxGetApp ()->LoadCursor (AFX_IDC_HSPLITBAR);
	}

	if (globalData.m_hcurStretchVert == NULL)
	{
		globalData.m_hcurStretchVert = AfxGetApp ()->LoadCursor (AFX_IDC_VSPLITBAR);
	}
	
	return 0;
}
//*******************************************************************************
void CBCGPSlider::StoreRecentDockInfo (CBCGPDockingControlBar* pBar)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pBar);

	if (m_pContainerManager != NULL)
	{
		m_pContainerManager->StoreRecentDockInfo (pBar);
	}
}
//*******************************************************************************
void CBCGPSlider::StoreRecentTabRelatedInfo (CBCGPDockingControlBar* pDockingBar, 
												CBCGPDockingControlBar* pTabbedBar)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDockingBar);
	ASSERT_VALID (pTabbedBar);

	if (m_pContainerManager != NULL)
	{
		BOOL bLeftBar = FALSE;
		CBCGPBarContainer* pTabbedContainer = 
			m_pContainerManager->FindContainer (pTabbedBar, bLeftBar);
		if (pTabbedContainer != NULL)
		{
			pDockingBar->
				m_recentDockInfo.StoreDockInfo (pTabbedContainer, pTabbedBar);
		}
		else
		{
			ASSERT (FALSE);
		}
	}
}
//*******************************************************************************
CBCGPBarContainer* CBCGPSlider::FindContainer (CBCGPDockingControlBar* pBar, 
											   BOOL& bLeftBar)
{
	ASSERT_VALID (this);

	return m_pContainerManager->FindContainer (pBar, bLeftBar);
}
//*******************************************************************************
BOOL CBCGPSlider::AddContainer (CBCGPBarContainerManager& barContainerManager, 
								BOOL bOuterEdge)	
{
	BOOL bResult = FALSE;
	if (m_pContainerManager != NULL)
	{
		bResult = m_pContainerManager->AddContainerManager (barContainerManager, bOuterEdge);
		CheckVisibility ();
	}
	return bResult;
}
//*******************************************************************************
BOOL CBCGPSlider::AddContainer	(CBCGPDockingControlBar* pTargetBar, 
								 CBCGPBarContainerManager& barContainerManager, 
								 DWORD dwAlignment)
{
	BOOL bResult = FALSE;
	if (m_pContainerManager != NULL)
	{
		bResult = m_pContainerManager->AddContainerManager (pTargetBar, dwAlignment, 
																 barContainerManager,
																 TRUE);
		CheckVisibility ();
	}
	return bResult;
}
//*******************************************************************************
void CBCGPSlider::OnShowControlBar (CBCGPDockingControlBar* pBar, BOOL bShow)
{
	if (m_pContainerManager != NULL && !IsAutoHideMode	())
	{
		BOOL bNewVisibleState = m_pContainerManager->OnShowControlBar (pBar, bShow);
		if (!bShow)
		{
			// actual only for hide, because when bShow is TRUE slider must be always
			// visible
			ShowWindow (bNewVisibleState ? SW_SHOW : SW_HIDE);
			BOOL bLeftBar = FALSE;
			CBCGPBarContainer* pContainer = m_pContainerManager->FindContainer (pBar, bLeftBar);
			if (pContainer != NULL)
			{
				pContainer->OnShowControlBar (pBar, bShow);
			}
		}
		else
		{
			ShowWindow (SW_SHOW);
		}
	}
}
//*******************************************************************************
BOOL CBCGPSlider::CheckVisibility ()
{
	if (m_bDefaultSlider && !IsAutoHideMode	() && m_pContainerManager != NULL)
	{
		BOOL bIsRootContainerVisible = m_pContainerManager->IsRootContainerVisible ();
		ShowWindow (bIsRootContainerVisible ? SW_SHOW : SW_HIDE);
		return bIsRootContainerVisible;
	}

	return FALSE;
}
//*******************************************************************************
BOOL CBCGPSlider::DoesContainFloatingBar ()
{
	if (m_pContainerManager != NULL)
	{
		return m_pContainerManager->DoesContainFloatingBar ();
	}
	return FALSE;
}
//*******************************************************************************
BOOL CBCGPSlider::DoesAllowDynInsertBefore () const
{
	if (m_pContainerManager != NULL)
	{
		return m_pContainerManager->DoesAllowDynInsertBefore ();
	}
	return TRUE;
}
//*******************************************************************************
void CBCGPSlider::CalcExpectedDockedRect (CWnd* pWndToDock, CPoint ptMouse, 
										  CRect& rectResult, BOOL& bDrawTab, 
										  CBCGPDockingControlBar** ppTargetBar)
{
	CBCGPGlobalUtils globalUtils;
	if (m_pContainerManager != NULL)
	{
		globalUtils.CalcExpectedDockedRect (*m_pContainerManager, pWndToDock, 
											ptMouse, rectResult, bDrawTab, ppTargetBar);
	}
}
//*******************************************************************************
void CBCGPSlider::NotifyAboutRelease ()
{
	if (m_pContainerManager->IsEmpty () && 
		m_pContainerManager->GetTotalRefCount () == 0)
	{
		// it was the last control bar in the container - 
		// remove and DESTROY  the slider as well
		CBCGPBaseControlBar::RemoveControlBarFromDockManager (this, TRUE, FALSE, m_bAutoHideMode);
	}	
}
//*******************************************************************************
void CBCGPSlider::GetControlBars (CObList& lstBars) 
{
	if (m_pContainerManager != NULL)
	{
		m_pContainerManager->AddControlBarsToList(&lstBars, NULL);
	}
}
//*******************************************************************************
void CBCGPSlider::GetSliders (CObList& lstSliders)
{
	if (m_pContainerManager != NULL)
	{
		m_pContainerManager->AddControlBarsToList(NULL, &lstSliders);
	}
}
//*******************************************************************************
void CBCGPSlider::ReleaseEmptyContainers ()
{
	if (m_pContainerManager != NULL)
	{
		m_pContainerManager->ReleaseEmptyContainers ();
	}
}
//*******************************************************************************
CRect CBCGPSlider::GetRootContainerRect ()
{
	CRect rect; rect.SetRectEmpty ();
	if (m_pContainerManager != NULL)
	{
		m_pContainerManager->GetWindowRect (rect);
	}
	return rect;
}