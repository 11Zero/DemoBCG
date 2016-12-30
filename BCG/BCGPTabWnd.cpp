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

// bcgptabwnd.cpp : implementation file
//

#include "stdafx.h"

#include "BCGGlobals.h"
#include "BCGPTabWnd.h"

// detachable bars support
#include "BCGPDockingControlBar.h"
#include "BCGPVisualManager.h"
#include "BCGPTabbedControlBar.h"
#include "BCGPTabbedToolbar.h"
#include "BCGPToolbarButton.h"
#include "BCGPMiniFrameWnd.h"

#include "BCGPLocalResource.h"
#include "BCGProRes.h"
#include "BCGCBProVer.h"

#include "BCGPMDIFrameWnd.h"
#include "BCGPContextMenuManager.h"

#include "BCGPMainClientAreaWnd.h"
#include "BCGPTooltipManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

UINT BCGM_ON_HSCROLL			= ::RegisterWindowMessage (_T("BCGM_ON_HSCROLL"));
UINT BCGM_GETDRAGBOUNDS			= ::RegisterWindowMessage (_T("BCGM_GETDRAGBOUNDS"));
UINT BCGM_ON_DRAGCOMPLETE		= ::RegisterWindowMessage (_T("BCGM_ON_DRAGCOMPLETE"));
UINT BCGM_ON_TABGROUPMOUSEMOVE  = ::RegisterWindowMessage (_T("BCGM_ON_TABGROUPMOUSEMOVE"));
UINT BCGM_ON_CANCELTABMOVE		= ::RegisterWindowMessage (_T("BCGM_ON_CANCELTABMOVE"));
UINT BCGM_ON_MOVETABCOMPLETE	= ::RegisterWindowMessage (_T("BCGM_ON_MOVETABCOMPLETE"));

BOOL CBCGPTabWnd::m_bEnableActivate = TRUE;
CMap<UINT,UINT,HICON,HICON>	CBCGPTabWnd::m_mapDocIcons;

//------------------
// Timer event IDs:
//------------------
static const UINT idTabAnimation = 1;

/////////////////////////////////////////////////////////////////////////////
// CBCGPTabWnd

IMPLEMENT_DYNCREATE(CBCGPTabWnd, CBCGPBaseTabWnd)

#define MIN_SROLL_WIDTH			(::GetSystemMetrics (SM_CXHSCROLL) * 2)
#define SPLITTER_WIDTH			5
#define RESIZEBAR_SIZE			6
#define TABS_FONT				_T("Arial")

CBCGPTabWnd::CBCGPTabWnd()
{
	m_iTabsNum = 0;
	m_iActiveTab = -1;
	
	m_bFlat = FALSE;
	m_bIsOneNoteStyle = FALSE;
	m_bIsVS2005Style = FALSE;
	m_bIsPointerStyle = FALSE;
	m_bLeftRightRounded = FALSE;
	m_bScroll = FALSE;
	m_bScrollButtonFullSize = FALSE;
	m_bCloseBtn = FALSE;
	m_bSharedScroll = FALSE;
	m_rectTabsArea.SetRectEmpty ();
	m_rectWndArea.SetRectEmpty ();
	m_nTabsHorzOffset = 0;
	m_nFirstVisibleTab = 0;
	m_nTabsHorzOffsetMax = 0;
	m_nTabsTotalWidth = 0;
	m_nHorzScrollWidth = 0;
	m_nScrollBarRight = 0;
	m_rectTabSplitter.SetRectEmpty ();
	m_bTrackSplitter = FALSE;

	m_bFlatFrame = TRUE;
	
	m_bHideInactiveWnd = TRUE;
	m_bAutoSizeWindow = TRUE;
	m_bAutoDestoyWindow = TRUE;

	m_bTransparent = FALSE;
	m_bTopEdge = FALSE;
	m_bDrawFrame = TRUE;

	m_bHideNoTabs = FALSE;

	m_bIsActiveTabBold = FALSE;
	m_bTabDocumentsMenu = FALSE;
	m_TabCloseButtonMode = TAB_CLOSE_BUTTON_NONE;
	m_bHiddenDocuments = FALSE;
	m_nTabMaxWidth = 0;
	
	m_ResizeMode = RESIZE_NO;
	m_rectResize.SetRectEmpty ();
	m_rectResizeDrag.SetRectEmpty ();
	m_rectResizeBounds.SetRectEmpty ();
	m_bResize = FALSE;
	m_bIsActiveInMDITabGroup = FALSE;
	m_bIsMDIFocused = FALSE;

	m_bButtonsVisible = TRUE;
	m_bNewTab = FALSE;

	m_nTabAnimationOffset = 0;
	m_nTabAnimationStep = 0;
	m_bTabAnimationSupport = TRUE;
}
//***************************************************************************************
CBCGPTabWnd::~CBCGPTabWnd()
{
}

BEGIN_MESSAGE_MAP(CBCGPTabWnd, CBCGPBaseTabWnd)
	//{{AFX_MSG_MAP(CBCGPTabWnd)
	ON_WM_DESTROY()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_LBUTTONDOWN()
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_HSCROLL()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_CANCELMODE()
	ON_WM_SYSCOLORCHANGE()
	ON_WM_SETTINGCHANGE()
	ON_WM_SETFOCUS()
	ON_WM_WINDOWPOSCHANGED()
	ON_WM_WINDOWPOSCHANGING()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
	ON_REGISTERED_MESSAGE(BCGM_UPDATETOOLTIPS, OnBCGUpdateToolTips)
	ON_MESSAGE(WM_PRINTCLIENT, OnPrintClient)
END_MESSAGE_MAP()

BOOL CBCGPTabWnd::Create (Style style, const RECT& rect, CWnd* pParentWnd, 
						 UINT nID, Location location /* = LOCATION_BOTTOM*/,
						 BOOL bCloseBtn /* = FALSE */)
{
	m_bFlat = (style == STYLE_FLAT) || (style == STYLE_FLAT_SHARED_HORZ_SCROLL);
	m_bSharedScroll = style == STYLE_FLAT_SHARED_HORZ_SCROLL;
	m_bIsOneNoteStyle = (style == STYLE_3D_ONENOTE);
	m_bIsVS2005Style = (style == STYLE_3D_VS2005);
	m_bIsPointerStyle = (style == STYLE_POINTER);
	m_bLeftRightRounded = (style == STYLE_3D_ROUNDED || 
		style == STYLE_3D_ROUNDED_SCROLL);
	m_bHighLightTabs = m_bIsOneNoteStyle;
	m_location = location;
	m_bScroll = (m_bFlat || style == STYLE_3D_SCROLLED || 
				 style == STYLE_3D_ONENOTE || style == STYLE_3D_VS2005 ||
				 style == STYLE_3D_ROUNDED_SCROLL || style == STYLE_POINTER);
	m_bCloseBtn = bCloseBtn;

	if (!m_bFlat && m_bSharedScroll)
	{
		//--------------------------------------
		// Only flat tab has a shared scrollbar!
		//--------------------------------------
		ASSERT (FALSE);
		m_bSharedScroll = FALSE;
	}

	return CBCGPBaseTabWnd::Create (
		globalData.RegisterWindowClass (_T("BCGPTabWnd")), _T(""),
		WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, rect,
		pParentWnd, nID);
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPTabWnd message handlers

void CBCGPTabWnd::OnDestroy() 
{
	if (m_brActiveTab.GetSafeHandle () != NULL)
	{
		m_brActiveTab.DeleteObject ();
	}

	m_lstButtons.RemoveAll ();

	CBCGPTooltipManager::DeleteToolTip (m_pToolTip);
	CBCGPTooltipManager::DeleteToolTip (m_pToolTipClose);

	CBCGPBaseTabWnd::OnDestroy();
}
//***************************************************************************************
void CBCGPTabWnd::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	CBCGPMemDC memDC (dc, this);
	CDC* pDC = &memDC.GetDC ();

	dc.GetClipBox (&m_rectCurrClip);

	OnDraw(pDC);

	if (memDC.IsMemDC ())
	{
		dc.ExcludeClipRect (m_rectWndArea);
	}
}
//***************************************************************************************
void CBCGPTabWnd::OnDraw(CDC* pDC)
{
	ASSERT_VALID(pDC);

	COLORREF	clrDark;
	COLORREF	clrBlack;
	COLORREF	clrHighlight;
	COLORREF	clrFace;
	COLORREF	clrDarkShadow;
	COLORREF	clrLight;
	CBrush*		pbrFace = NULL;
	CBrush*		pbrBlack = NULL;
				   
	CBCGPVisualManager::GetInstance ()->GetTabFrameColors (
		this, clrDark, clrBlack, clrHighlight, clrFace, clrDarkShadow, clrLight,
		pbrFace, pbrBlack);

	ASSERT_VALID (pbrFace);
	ASSERT_VALID (pbrBlack);

	CRect rectClient;
	GetClientRect (&rectClient);

	switch (m_ResizeMode)
	{
	case RESIZE_VERT:
		rectClient.right -= m_rectResize.Width ();
		break;

	case RESIZE_HORIZ:
		rectClient.bottom -= m_rectResize.Height ();
		break;
	}

	CBrush* pOldBrush = pDC->SelectObject (pbrFace);
	ASSERT (pOldBrush != NULL);

	CPen penDark (PS_SOLID, 1, clrDark);
	CPen penBlack (PS_SOLID, 1, clrBlack);
	CPen penHiLight (PS_SOLID, 1, clrHighlight);

	CPen* pOldPen = (CPen*) pDC->SelectObject (&penDark);
	ASSERT(pOldPen != NULL);

	const int nTabBorderSize = GetTabBorderSize ();

	CRect rectTabs = rectClient;

	if (m_location == LOCATION_BOTTOM)
	{
		rectTabs.top = m_rectTabsArea.top - GetPointerAreaHeight();
	}
	else
	{
		rectTabs.bottom = m_rectTabsArea.bottom + GetPointerAreaHeight();
	}
	
	pDC->ExcludeClipRect (m_rectWndArea);

	BOOL bBackgroundIsReady =
		CBCGPVisualManager::GetInstance ()->OnEraseTabsFrame (pDC, rectClient, this);

	if ((!m_bDrawFrame || m_bIsPointerStyle) && !bBackgroundIsReady)
	{
		pDC->FillRect (rectClient, pbrFace);
	}

	CBCGPVisualManager::GetInstance ()->OnEraseTabsArea (pDC, rectTabs, this);

	if (!m_bIsPointerStyle)
	{
		CRect rectFrame = rectClient;

		if (nTabBorderSize == 0)
		{
			if (m_location == LOCATION_BOTTOM)
			{
				rectFrame.bottom = m_rectTabsArea.top + 1;
			}
			else
			{
				rectFrame.top = m_rectTabsArea.bottom - 1;
			}

			if (m_bFlat)
			{
				pDC->FrameRect(&rectFrame, pbrBlack);
			}
			else
			{
				pDC->FrameRect(&rectFrame, pbrFace);
			}
		}
		else
		{
			int yLine = m_location == LOCATION_BOTTOM ? 
				m_rectTabsArea.top : m_rectTabsArea.bottom;

			if (!m_bFlat)
			{
				if (m_location == LOCATION_BOTTOM)
				{
					rectFrame.bottom = m_rectTabsArea.top;
				}
				else
				{
					rectFrame.top = m_rectTabsArea.bottom;
				}
			}

			//-----------------------------------------------------
			// Draw wide 3-dimensional frame around the Tabs area:
			//-----------------------------------------------------
			if (m_bFlatFrame)
			{
				CRect rectBorder (rectFrame);

				if (m_bFlat)
				{
					if (m_location == LOCATION_BOTTOM)
					{
						rectBorder.bottom = m_rectTabsArea.top + 1;
					}
					else
					{
						rectBorder.top = m_rectTabsArea.bottom - 1;
					}
				}

				rectFrame.DeflateRect (1, 1);

				if (m_bDrawFrame && !bBackgroundIsReady && rectFrame.Width () > 0 && rectFrame.Height () > 0)
				{
					pDC->PatBlt (rectFrame.left, rectFrame.top, nTabBorderSize, rectFrame.Height (), PATCOPY);
					pDC->PatBlt (rectFrame.left, rectFrame.top, rectFrame.Width (), nTabBorderSize, PATCOPY);
					pDC->PatBlt (rectFrame.right - nTabBorderSize - 1, rectFrame.top, nTabBorderSize + 1, rectFrame.Height (), PATCOPY);
					pDC->PatBlt (rectFrame.left, rectFrame.bottom - nTabBorderSize, rectFrame.Width (), nTabBorderSize, PATCOPY);

					if (m_location == LOCATION_BOTTOM)
					{
						pDC->PatBlt (rectFrame.left, m_rectWndArea.bottom, rectFrame.Width (), 
							rectFrame.bottom - m_rectWndArea.bottom, PATCOPY);
					}
					else
					{
						pDC->PatBlt (rectFrame.left, rectFrame.top, rectFrame.Width (), 
							m_rectWndArea.top - rectFrame.top, PATCOPY);
					}
				}

				if (m_bFlat)
				{
					//---------------------------
					// Draw line below the tabs:
					//---------------------------
					pDC->SelectObject (&penBlack);
					pDC->MoveTo (rectFrame.left + nTabBorderSize, yLine);
					pDC->LineTo (rectFrame.right - nTabBorderSize, yLine);
				}

				if (GetTabsHeight () == 0)
				{
					pDC->Draw3dRect (&rectBorder, clrFace, clrFace);
				}
				else
				{
					CBCGPVisualManager::GetInstance ()->OnDrawTabBorder(pDC, this, rectBorder,
						m_bDrawFrame ? clrDark : clrFace,
						m_location == LOCATION_BOTTOM ? penBlack : penHiLight);
				}
			}
			else
			{
				if (m_bDrawFrame)
				{
					pDC->Draw3dRect (&rectFrame, clrHighlight, clrDarkShadow);
					
					rectFrame.DeflateRect (1, 1);
					pDC->Draw3dRect (&rectFrame, clrLight, clrDark);
					
					rectFrame.DeflateRect (1, 1);
					
					if (!bBackgroundIsReady &&
						rectFrame.Width () > 0 && rectFrame.Height () > 0)
					{
						pDC->PatBlt (rectFrame.left, rectFrame.top, nTabBorderSize, rectFrame.Height (), PATCOPY);
						pDC->PatBlt (rectFrame.left, rectFrame.top, rectFrame.Width (), nTabBorderSize, PATCOPY);
						pDC->PatBlt (rectFrame.right - nTabBorderSize, rectFrame.top, nTabBorderSize, rectFrame.Height (), PATCOPY);
						pDC->PatBlt (rectFrame.left, rectFrame.bottom - nTabBorderSize, rectFrame.Width (), nTabBorderSize, PATCOPY);
						
						if (m_location == LOCATION_BOTTOM)
						{
							pDC->PatBlt (rectFrame.left, m_rectWndArea.bottom, rectFrame.Width (), 
								rectFrame.bottom - m_rectWndArea.bottom, PATCOPY);
						}
						else
						{
							pDC->PatBlt (rectFrame.left, rectFrame.top, rectFrame.Width (), 
								m_rectWndArea.top - rectFrame.top, PATCOPY);
						}

						if (m_bFlat)
						{
							//---------------------------
							// Draw line below the tabs:
							//---------------------------
							pDC->SelectObject (&penBlack);

							pDC->MoveTo (rectFrame.left + nTabBorderSize, yLine);
							pDC->LineTo (rectFrame.right - nTabBorderSize, yLine);
						}

						if (nTabBorderSize > 2)
						{
							rectFrame.DeflateRect (nTabBorderSize - 2, nTabBorderSize - 2);
						}
						
						if (rectFrame.Width () > 0 && rectFrame.Height () > 0)
						{
							pDC->Draw3dRect (&rectFrame, clrDarkShadow, clrHighlight);
						}
					}
					else
					{
						rectFrame.DeflateRect (2, 2);
					}
				}
			}
		}

		const BOOL bTopEdge = m_bTopEdge && m_location == LOCATION_TOP &&
						(!m_bIsMDITab || CBCGPVisualManager::GetInstance ()->IsMDITabsTopEdge ());
		if (bTopEdge)
		{
			pDC->SelectObject (&penDark);

			pDC->MoveTo (rectClient.left, m_rectTabsArea.bottom);
			pDC->LineTo (rectClient.left, rectClient.top);
			pDC->LineTo (rectClient.right - 1, rectClient.top);
			pDC->LineTo (rectClient.right - 1, m_rectTabsArea.bottom);
		}
	}
	else
	{
		CRect rectActiveTab(0, 0, 0, 0);
		if (m_iActiveTab >= 0)
		{
			CBCGPTabInfo* pTabActive = (CBCGPTabInfo*) m_arTabs [m_iActiveTab];
			ASSERT_VALID (pTabActive);

			rectActiveTab = pTabActive->GetRect();

			if (m_bTabAnimationSupport && m_nTabAnimationOffset != 0)
			{
				rectActiveTab.OffsetRect(m_nTabAnimationOffset, 0);
			}
		}

		CBCGPVisualManager::GetInstance ()->OnDrawTabsPointer(pDC, this, rectTabs, GetPointerAreaHeight(), rectActiveTab);
	}

	CFont* pOldFont = pDC->SelectObject(GetTabFont());
	ASSERT(pOldFont != NULL);

	pDC->SetBkMode (TRANSPARENT);
	pDC->SetTextColor (globalData.clrBtnText);

	if (m_rectTabsArea.Width () > 5 && m_rectTabsArea.Height () > 5)
	{
		//-----------
		// Draw tabs:
		//-----------
		CRect rectClip = m_rectTabsArea;
		rectClip.InflateRect (1, nTabBorderSize);

		CRgn rgn;
		rgn.CreateRectRgnIndirect (rectClip);

		for (int nIndex = m_iTabsNum - 1; nIndex >= 0; nIndex--)
		{
			int i = nIndex;

			if (m_arTabIndexs.GetSize() == m_iTabsNum)
			{
				i = m_arTabIndexs[nIndex];
			}

			CBCGPTabInfo* pTab = (CBCGPTabInfo*) m_arTabs [i];
			ASSERT_VALID (pTab);

			if (!pTab->m_bVisible)
				continue;

			m_iCurTab = i;

			if (i != m_iActiveTab)	// Draw active tab last
			{
				pDC->SelectClipRgn (&rgn);

				if (m_bFlat)
				{
					pDC->SelectObject (&penBlack);
					DrawFlatTab (pDC, pTab, FALSE);
				}
				else
				{
					Draw3DTab (pDC, pTab, FALSE);
				}
			}
		}

		if (m_iActiveTab >= 0)
		{
			//-----------------
			// Draw active tab:
			//-----------------
			pDC->SetTextColor (globalData.clrWindowText);

			CBCGPTabInfo* pTabActive = (CBCGPTabInfo*) m_arTabs [m_iActiveTab];
			ASSERT_VALID (pTabActive);

			m_iCurTab = m_iActiveTab;

			pDC->SelectClipRgn (&rgn);

			if (m_bFlat)
			{
				if (m_brActiveTab.GetSafeHandle() == NULL)
				{
					m_brActiveTab.CreateSolidBrush (GetActiveTabColor ());
				}

				pDC->SelectObject (&m_brActiveTab);

				if (m_hFontCustom == NULL)
				{
					pDC->SelectObject (&m_fntTabsBold);
				}

				pDC->SetTextColor (GetActiveTabTextColor ());
				pDC->SelectObject (&penBlack);

				DrawFlatTab (pDC, pTabActive, TRUE);

				//---------------------------------
				// Draw line bellow the active tab:
				//---------------------------------
				const int xLeft = max (	m_rectTabsArea.left + 1,
										pTabActive->m_rect.left + 1);

				if (pTabActive->m_rect.right > m_rectTabsArea.left + 1)
				{
					CPen penLight (PS_SOLID, 1, GetActiveTabColor ());
					pDC->SelectObject (&penLight);

					if (m_location == LOCATION_BOTTOM)
					{
						pDC->MoveTo (xLeft, pTabActive->m_rect.top);
						pDC->LineTo (pTabActive->m_rect.right, pTabActive->m_rect.top);
					}
					else
					{
						pDC->MoveTo (xLeft, pTabActive->m_rect.bottom);
						pDC->LineTo (pTabActive->m_rect.right, pTabActive->m_rect.bottom);
					}

					pDC->SelectObject (pOldPen);
				}
			}
			else
			{
				if (m_bIsActiveTabBold && m_hFontCustom == NULL && !IsCaptionFont())
				{
					if (!IsMDITabGroup () || m_bIsActiveInMDITabGroup)
					{
						pDC->SelectObject (&globalData.fontBold);
					}
				}

				Draw3DTab (pDC, pTabActive, TRUE);
			}
		}

		pDC->SelectClipRgn (NULL);
	}

	if (!m_rectTabSplitter.IsRectEmpty ())
	{
		pDC->FillRect (m_rectTabSplitter, pbrFace);

		CRect rectTabSplitter = m_rectTabSplitter;

		pDC->Draw3dRect (rectTabSplitter, clrDarkShadow, clrDark);
		rectTabSplitter.DeflateRect (1, 1);
		pDC->Draw3dRect (rectTabSplitter, clrHighlight, clrDark);
	}
	
	if (m_bFlat && m_nTabsHorzOffset > 0)
	{
		pDC->SelectObject (&penDark);

		const int xDivider = m_rectTabsArea.left - 1;

		if (m_location == LOCATION_BOTTOM)
		{
			pDC->MoveTo (xDivider, m_rectTabsArea.top + 1);
			pDC->LineTo (xDivider, m_rectTabsArea.bottom - 2);
		}
		else
		{
			pDC->MoveTo (xDivider, m_rectTabsArea.bottom);
			pDC->LineTo (xDivider, m_rectTabsArea.top + 2);
		}
	}

	if (!m_rectResize.IsRectEmpty ())
	{
		CBCGPVisualManager::GetInstance ()->OnDrawTabResizeBar (pDC, this, 
			m_ResizeMode == RESIZE_VERT, m_rectResize,
			pbrFace, &penDark);
	}

	pDC->SelectObject (pOldFont);
	pDC->SelectObject (pOldBrush);
	pDC->SelectObject (pOldPen);
}
//***************************************************************************************
void CBCGPTabWnd::OnSize(UINT nType, int cx, int cy) 
{
	CBCGPBaseTabWnd::OnSize(nType, cx, cy);

	int nTabsAreaWidth = cx - 4 * ::GetSystemMetrics (SM_CXVSCROLL) 
							- 2 * GetTabBorderSize ();

	if (nTabsAreaWidth <= MIN_SROLL_WIDTH)
	{
		m_nHorzScrollWidth = 0;
	}
	else if (nTabsAreaWidth / 2 > MIN_SROLL_WIDTH)
	{
		m_nHorzScrollWidth = nTabsAreaWidth / 2;
	}
	else
	{
		m_nHorzScrollWidth = nTabsAreaWidth; 
	}

	if (m_bIsOneNoteStyle || m_bIsVS2005Style || m_bLeftRightRounded)
	{
		m_nTabsHorzOffset = 0;
		m_nFirstVisibleTab = 0;

		SetRedraw (FALSE);

		RecalcLayout ();

		if (m_iActiveTab >= 0)
		{
			EnsureVisible (m_iActiveTab);
		}

		SetRedraw (TRUE);
		RedrawWindow (NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_FRAME | RDW_ALLCHILDREN);
	}
	else
	{
		RecalcLayout ();

		if (m_iActiveTab >= 0)
		{
			EnsureVisible (m_iActiveTab);
		}
	}

	SynchronizeScrollBar ();
}
//***************************************************************************************
BOOL CBCGPTabWnd::SetActiveTab (int iTab)
{
	if (iTab < 0 || iTab >= m_iTabsNum)
	{
		TRACE(_T("CBCGPTabWnd::SetActiveTab: illegal tab number %d\n"), iTab);
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
		if (IsMDITabGroup ())
		{
			ActivateMDITab (m_iActiveTab);
		}

		return TRUE;
	}

	m_nTabAnimationStep = m_nTabAnimationOffset = 0;
	KillTimer(idTabAnimation);

	if (IsNewTabEnabled())
	{
		CBCGPTabInfo* pTab = (CBCGPTabInfo*) m_arTabs [m_iTabsNum - 1];
		ASSERT_VALID (pTab);

		if (pTab->m_bIsNewTab)
		{
			if (m_iTabsNum <= 1)
			{
				return FALSE;
			}

			if (iTab == m_iTabsNum - 1)
			{
				iTab--;
			}
		}
	}

	BOOL bResChangingActiveTab = FireChangingActiveTab (iTab);

	if (bResChangingActiveTab)
	{
		return FALSE;
	}
	
	CBCGPMDIFrameWnd* pParentFrame = DYNAMIC_DOWNCAST (CBCGPMDIFrameWnd, GetParentFrame ());
	BOOL bEnableSetRedraw = FALSE;

	if (pParentFrame != NULL && m_bIsMDITab)
	{
		bEnableSetRedraw = !pParentFrame->m_bClosing && !CBCGPMDIFrameWnd::m_bDisableSetRedraw;
	}

	CWnd* pWndParent = GetParent ();
	ASSERT_VALID (pWndParent);

	if (m_iTabsNum > 1 && bEnableSetRedraw)
	{
		pWndParent->SetRedraw (FALSE);
	}

	if (m_iActiveTab != -1 && m_bHideInactiveWnd)
	{
		//--------------------
		// Hide active window:
		//--------------------
		CWnd* pWndActive = GetActiveWnd ();
		if (pWndActive != NULL)
		{
			pWndActive->ShowWindow (SW_HIDE);
		}
	}

	int nPrevActiveTab = m_iActiveTab;
	m_iActiveTab = iTab;
	
	//------------------------
	// Show new active window:
	//------------------------
	HideActiveWindowHorzScrollBar ();

	CWnd* pWndActive = GetActiveWnd ();
	if (pWndActive == NULL)
	{
		ASSERT (FALSE);
		pWndParent->SetRedraw (TRUE);
		return FALSE;
	}

	ASSERT_VALID (pWndActive);

	if (pWndActive->GetSafeHwnd() == NULL)
	{
		TRACE(_T("CBCGPTabWnd::SetActiveTab: tab window %d wasn't properly created\n"), iTab);
	}
	
	pWndActive->ShowWindow (SW_SHOW);
	if (!m_bHideInactiveWnd)
	{
		pWndActive->BringWindowToTop ();
	}

	if (m_bAutoSizeWindow)
	{
		//----------------------------------------------------------------------
		// Small trick: to adjust active window scroll sizes, I should change an
		// active window size twice (+1 pixel and -1 pixel):
		//----------------------------------------------------------------------
		pWndActive->SetWindowPos (NULL,
				-1, -1,
				m_rectWndArea.Width () + 1, m_rectWndArea.Height (),
				SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE);
		pWndActive->SetWindowPos (NULL,
				-1, -1,
				m_rectWndArea.Width (), m_rectWndArea.Height (),
				SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE);
	}

	EnsureVisible (m_iActiveTab);

	if (m_bFlat)
	{
		SynchronizeScrollBar ();
	}

	//--------------------------------------------------
	// Set text to the parent frame/docking control bar:
	//--------------------------------------------------
	CBCGPTabbedControlBar* pTabControlBar = 
		DYNAMIC_DOWNCAST (CBCGPTabbedControlBar, GetParent ());
	if (pTabControlBar != NULL && pTabControlBar->CanSetCaptionTextToTabName ()) // tabbed dock bar - redraw caption only in this case
	{
		CString strCaption;
		GetTabLabel (m_iActiveTab, strCaption);

		// miniframe will take the text from the tab control bar
		pTabControlBar->SetWindowText (strCaption);

		CWnd* pWndToUpdate = pTabControlBar;
		if (!pTabControlBar->IsDocked ())
		{
			pWndToUpdate = pTabControlBar->GetParent ();
		}

		if (pWndToUpdate != NULL)
		{
			pWndToUpdate->RedrawWindow (NULL, NULL, RDW_FRAME | RDW_INVALIDATE);
		}
	}
	
	if (m_bIsActiveTabBold || m_TabCloseButtonMode != TAB_CLOSE_BUTTON_NONE || m_TabCloseButtonMode == TAB_CLOSE_BUTTON_ALL)
	{
		RecalcLayout ();
	}

	//-------------
	// Redraw tabs:
	//-------------
	if (m_bTabAnimationSupport && IsPointerStyle() && !m_bIsMDITab)
	{
		CRect rectActivePrev(0,  0, 0, 0);
		GetTabRect(nPrevActiveTab, rectActivePrev);

		CRect rectActive(0,  0, 0, 0);
		GetTabRect(m_iActiveTab, rectActive);

		if (!rectActivePrev.IsRectEmpty() && !rectActive.IsRectEmpty())
		{
			int nDeltaX = rectActive.left - rectActivePrev.left;
			m_nTabAnimationStep = nDeltaX / 3;

			if (m_nTabAnimationStep != 0)
			{
				m_nTabAnimationOffset = -nDeltaX;
				SetTimer(idTabAnimation, 50, NULL);
			}
		}
	}

	Invalidate ();
	UpdateWindow ();

	if (!bIsFirstTime)
	{
		CView* pActiveView = DYNAMIC_DOWNCAST (CView, pWndActive);
		if (pActiveView != NULL && !m_bDontActivateView)
		{
			CFrameWnd* pFrame = BCGPGetParentFrame (pActiveView);
			ASSERT_VALID (pFrame);

			pFrame->SetActiveView (pActiveView);
		}
		else if (m_bEnableActivate)
		{
			pWndActive->SetFocus ();
		}
	}

	if (m_btnClose.GetSafeHwnd () != NULL)
	{
		//----------------------------------------------------
		// Enable/disable "Close" button according to ability 
		// to close an active window:
		//----------------------------------------------------
		BOOL bEnableClose = TRUE;

		HMENU hSysMenu = pWndActive->GetSystemMenu (FALSE)->GetSafeHmenu ();
		if (hSysMenu != NULL)
		{
			MENUITEMINFO menuInfo;
			ZeroMemory(&menuInfo,sizeof(MENUITEMINFO));
			menuInfo.cbSize = sizeof(MENUITEMINFO);
			menuInfo.fMask = MIIM_STATE;

			if (!::GetMenuItemInfo (hSysMenu, SC_CLOSE, FALSE, &menuInfo) ||
				(menuInfo.fState & MFS_GRAYED) || 
				(menuInfo.fState & MFS_DISABLED))
			{
				bEnableClose = FALSE;
			}
		}

		m_btnClose.EnableWindow (bEnableClose);
	}
	
	FireChangeActiveTab (m_iActiveTab);

	if (m_iTabsNum > 1 && bEnableSetRedraw)
	{
		pWndParent->SetRedraw (TRUE);

		const UINT uiRedrawFlags =	RDW_INVALIDATE | RDW_UPDATENOW | 
									RDW_ERASE | RDW_ALLCHILDREN;

		if (m_bSetActiveTabByMouseClick)
		{
			CRect rectWindow;
			GetWindowRect (rectWindow);
			GetParent ()->ScreenToClient (rectWindow);

			pWndParent->RedrawWindow (rectWindow, NULL, uiRedrawFlags);
		}
		else
		{
			pWndParent->RedrawWindow (NULL, NULL, uiRedrawFlags);
		}
	}


	if (m_iActiveTab != -1 && pTabControlBar != NULL)
	{
		CBCGPBaseControlBar* pBar = 
			DYNAMIC_DOWNCAST(CBCGPBaseControlBar, GetTabWnd (m_iActiveTab));
		if (pBar != NULL)
		{
			CBCGPMiniFrameWnd* pParentMiniFrame = pBar->GetParentMiniFrame ();
			
			if (pBar->GetBCGStyle () & CBRS_BCGP_AUTO_ROLLUP)
			{
				pTabControlBar->m_dwBCGStyle |= CBRS_BCGP_AUTO_ROLLUP;
				if (pParentMiniFrame != NULL)
				{
					pParentMiniFrame->OnSetRollUpTimer ();
				}
			}
			else
			{
				pTabControlBar->m_dwBCGStyle &= ~CBRS_BCGP_AUTO_ROLLUP;
				if (pParentMiniFrame != NULL)
				{
					pParentMiniFrame->OnKillRollUpTimer ();
				}

			}
		}
	}

	return TRUE;
}
//***************************************************************************************
void CBCGPTabWnd::AdjustTabs ()
{
	m_bHiddenDocuments = FALSE;

	int	nVisibleTabsNum	= GetVisibleTabsNum ();
	if (nVisibleTabsNum == 0 || GetTabsHeight () == 0)
	{
		return;
	}

	if (m_bHideSingleTab && nVisibleTabsNum <= 1)
	{
		for (int i = 0; i < m_iTabsNum; i ++)
		{
			CBCGPTabInfo* pTab = (CBCGPTabInfo*) m_arTabs [i];
			ASSERT_VALID (pTab);
			
			pTab->m_rect.SetRectEmpty ();
		}

		return;
	}

	CRect rectActiveTabTT (0, 0, 0, 0);

	//-------------------------
	// Define tab's full width:
	//-------------------------
	CClientDC dc (this);

	CFont* pOldFont = dc.SelectObject (m_bFlat && m_hFontCustom == NULL ? &m_fntTabsBold : GetTabFont());
	ASSERT(pOldFont != NULL);

	m_nTabsTotalWidth = 0;

	//----------------------------------------------
	// First, try set all tabs in its original size:
	//----------------------------------------------
	int x = m_rectTabsArea.left - m_nTabsHorzOffset;
	int i = 0;
	int nNewTabWidth = IsNewTabEnabled() ? m_rectTabsArea.Height () - 2 * TAB_IMAGE_MARGIN : 0;

	TabCloseButtonMode tabCloseButtonMode = m_TabCloseButtonMode;
	if (m_bFlat)
	{
		if (tabCloseButtonMode == TAB_CLOSE_BUTTON_HIGHLIGHTED_COMPACT || tabCloseButtonMode == TAB_CLOSE_BUTTON_HIGHLIGHTED)
		{
			tabCloseButtonMode = TAB_CLOSE_BUTTON_ACTIVE;
		}
	}

	for (int nIndex = 0; nIndex < m_iTabsNum; nIndex++)
	{
		if (m_arTabIndexs.GetSize() == m_iTabsNum)
		{
			i = m_arTabIndexs[nIndex];
		}
		else
		{
			i = nIndex;
		}

		CBCGPTabInfo* pTab = (CBCGPTabInfo*) m_arTabs [i];
		ASSERT_VALID (pTab);

		pTab->m_rectClose.SetRectEmpty();
		
		if (m_pToolTipClose->GetSafeHwnd() != NULL)
		{
			m_pToolTipClose->SetToolRect(this, pTab->m_iTabID, pTab->m_rectClose);
		}

		CSize sizeImage (0, 0);
		if (pTab->m_hIcon != NULL || pTab->m_uiIcon != (UINT)-1)
		{
			sizeImage = m_sizeImage;
		}

		if (m_bIsActiveTabBold && (m_bIsOneNoteStyle || m_bIsVS2005Style || i == m_iActiveTab) && m_hFontCustom == NULL && !IsCaptionFont())
		{
			dc.SelectObject (&globalData.fontBold);
		}

		int nExtraWidth = 0;

		if (pTab->m_bVisible)
		{
			if (pTab->m_bIsNewTab)
			{
				pTab->m_nFullWidth = nNewTabWidth;
			}
			else
			{
				CString strText = pTab->m_strText;
				if (IsDrawNameInUpperCase())
				{
					strText.MakeUpper();
				}
				
				pTab->m_nFullWidth = sizeImage.cx + TAB_IMAGE_MARGIN +
					(pTab->m_bIconOnly ? 
						0 : dc.GetTextExtent(strText).cx) + 2 * TAB_TEXT_MARGIN;

				if (m_bLeftRightRounded)
				{
					pTab->m_nFullWidth += m_nTabsHeight;
					nExtraWidth = m_nTabsHeight - 2 * TAB_IMAGE_MARGIN - 1;
				}
				else if (m_bIsOneNoteStyle)
				{
					pTab->m_nFullWidth += m_rectTabsArea.Height () + 2 * TAB_IMAGE_MARGIN;
					nExtraWidth = m_rectTabsArea.Height () - TAB_IMAGE_MARGIN - 1;
				}
				else if (m_bIsVS2005Style)
				{
					pTab->m_nFullWidth += m_rectTabsArea.Height ();
					nExtraWidth = m_rectTabsArea.Height () - TAB_IMAGE_MARGIN - 1;
				}
			}

			if (!m_bNewTab || i < m_iTabsNum - 1)
			{
				BOOL bCloseButtonSpace = FALSE;
				
				switch (tabCloseButtonMode)
				{
				case TAB_CLOSE_BUTTON_ACTIVE:
				case TAB_CLOSE_BUTTON_HIGHLIGHTED_COMPACT:
					if (i == m_iActiveTab)
					{
						bCloseButtonSpace = TRUE;
					}
					break;

				case TAB_CLOSE_BUTTON_HIGHLIGHTED:
				case TAB_CLOSE_BUTTON_ALL:
					bCloseButtonSpace = TRUE;
					break;
				}

				if (bCloseButtonSpace)
				{
					pTab->m_nFullWidth += m_rectTabsArea.Height () - 2;
				}
			}
		}
		else
		{
			pTab->m_nFullWidth = 0;
		}

		if (m_bIsActiveTabBold && i == m_iActiveTab && m_hFontCustom == NULL && !IsCaptionFont())
		{
			dc.SelectObject(GetTabFont());	// Bold tab is availible for 3d tabs only
		}

		int nTabWidth = pTab->m_nFullWidth;

		if (m_bScroll && m_nTabMaxWidth > 0)
		{
			nTabWidth = min (nTabWidth, m_nTabMaxWidth);
		}

		pTab->m_rect = CRect (CPoint (x, m_rectTabsArea.top),
						CSize (nTabWidth, m_rectTabsArea.Height () - 2));
		
		if (!pTab->m_bVisible)
		{
			if (m_pToolTip->GetSafeHwnd () != NULL)
			{
				m_pToolTip->SetToolRect (this, pTab->m_iTabID, CRect (0, 0, 0, 0));
			}
			continue;
		}
		
		if (m_location == LOCATION_TOP)
		{
			pTab->m_rect.OffsetRect (0, 2);
		}

		if (m_bTabDocumentsMenu && pTab->m_rect.right > m_rectTabsArea.right - nNewTabWidth && !pTab->m_bIsNewTab)
		{
			BOOL bHideTab = TRUE;

			if (i == m_iActiveTab && nIndex == 0)
			{
				int nWidth = m_rectTabsArea.right - pTab->m_rect.left;

				if (nWidth >= nExtraWidth + 2 * TAB_TEXT_MARGIN)
				{
					pTab->m_rect.right = m_rectTabsArea.right;
					bHideTab = FALSE;
				}
			}

			if (bHideTab)
			{
				pTab->m_nFullWidth = 0;
				pTab->m_rect.SetRectEmpty ();
				m_bHiddenDocuments = TRUE;
				continue;
			}
		}
			
		if (m_pToolTip->GetSafeHwnd () != NULL)
		{
			BOOL bShowTooltip = pTab->m_bAlwaysShowToolTip || m_bCustomToolTips;

			if (pTab->m_rect.left < m_rectTabsArea.left ||
				pTab->m_rect.right > m_rectTabsArea.right)
			{
				bShowTooltip = TRUE;
			}

			if (m_bScroll && m_nTabMaxWidth > 0 &&
				pTab->m_rect.Width () < pTab->m_nFullWidth)
			{
				bShowTooltip = TRUE;
			}

			AdjustTooltipRect(pTab, bShowTooltip);

			if (bShowTooltip && i == m_iActiveTab)
			{
				rectActiveTabTT = pTab->m_rect;
			}
		}

		x += pTab->m_rect.Width () + 1 - nExtraWidth;
		m_nTabsTotalWidth += pTab->m_rect.Width () + 1;

		if (!IsFirstTab(i))
		{
			m_nTabsTotalWidth -= nExtraWidth;
		}

		if (m_bFlat)
		{
			//--------------------------------------------
			// In the flat mode tab is overlapped by next:
			//--------------------------------------------
			pTab->m_rect.right += m_nTabsHeight / 2;
		}
	}

	if (m_bScroll || x < m_rectTabsArea.right)
	{
		m_nTabsTotalWidth += m_nTabsHeight / 2;
	}
	else
	{
		//-----------------------------------------
		// Not enouth space to show the whole text.
		//-----------------------------------------
		int nTabsWidth = m_rectTabsArea.Width () - nNewTabWidth;
		int nTabWidth = nTabsWidth / nVisibleTabsNum - 1;

		if (m_bLeftRightRounded)
		{
			nTabWidth = max (
				m_sizeImage.cx + m_nTabsHeight / 2,
				(nTabsWidth - m_nTabsHeight / 3) / nVisibleTabsNum);
		}

		//------------------------------------
		// May be it's too wide for some tabs?
		//------------------------------------
		int nRest = 0;
		int nCutTabsNum = nVisibleTabsNum;

		for (int nIndex = 0; nIndex < m_iTabsNum; nIndex++)
		{
			if (m_arTabIndexs.GetSize() == m_iTabsNum)
			{
				i = m_arTabIndexs[nIndex];
			}
			else
			{
				i = nIndex;
			}
			
			CBCGPTabInfo* pTab = (CBCGPTabInfo*) m_arTabs [i];
			ASSERT_VALID (pTab);
			
			if (!pTab->m_bVisible)
			{
				continue;
			}
			
			if (pTab->m_nFullWidth < nTabWidth)
			{
				nRest += nTabWidth - pTab->m_nFullWidth;
				nCutTabsNum --;
			}
		}

		if (nCutTabsNum > 0)
		{
			nTabWidth += nRest / nCutTabsNum;

			//----------------------------------
			// Last pass: set actual rectangles:
			//----------------------------------
			x = m_rectTabsArea.left;

			for (int nIndex = 0; nIndex < m_iTabsNum; nIndex++)
			{
				if (m_arTabIndexs.GetSize() == m_iTabsNum)
				{
					i = m_arTabIndexs[nIndex];
				}
				else
				{
					i = nIndex;
				}
				
				CBCGPTabInfo* pTab = (CBCGPTabInfo*) m_arTabs [i];
				ASSERT_VALID (pTab);
				
				if (!pTab->m_bVisible)
				{
					if (m_pToolTip->GetSafeHwnd () != NULL)
					{
						m_pToolTip->SetToolRect (this, pTab->m_iTabID, CRect (0, 0, 0, 0));
					}

					continue;
				}
				
				CSize sizeImage (0, 0);
				if (pTab->m_hIcon != NULL || pTab->m_uiIcon != (UINT)-1)
				{
					sizeImage = m_sizeImage;
				}

				BOOL bIsTrucncated = pTab->m_nFullWidth > nTabWidth;
				int nCurrTabWidth = (bIsTrucncated) ? nTabWidth : pTab->m_nFullWidth;

				if (pTab->m_bIsNewTab)
				{
					nCurrTabWidth = pTab->m_nFullWidth;
				}
				else
				{
					if (nTabWidth < sizeImage.cx + TAB_IMAGE_MARGIN)
					{
						// Too narrow!
						nCurrTabWidth = (m_rectTabsArea.Width () + m_nTabBorderSize * 2) / nVisibleTabsNum;
					}
					else
					{
						if (pTab->m_strText.IsEmpty () || pTab->m_bIconOnly)
						{
							nCurrTabWidth = sizeImage.cx + 2 * CBCGPBaseTabWnd::TAB_TEXT_MARGIN;
						}
					}
				}

				if (m_bLeftRightRounded)
				{
					nCurrTabWidth += m_nTabsHeight / 2 - 1;
				}

				pTab->m_rect = CRect (CPoint (x, m_rectTabsArea.top),
								CSize (nCurrTabWidth, m_rectTabsArea.Height () - 2));

				if (!m_bFlat)
				{
					if (m_location == LOCATION_TOP)
					{
						pTab->m_rect.OffsetRect (0, 2);
					}

					if (m_pToolTip->GetSafeHwnd () != NULL)
					{
						BOOL bShowTooltip = bIsTrucncated || pTab->m_bAlwaysShowToolTip || m_bCustomToolTips;

						AdjustTooltipRect(pTab, bShowTooltip);

						if (bShowTooltip && i == m_iActiveTab)
						{
							rectActiveTabTT = pTab->m_rect;
						}
					}
				}

				x += nCurrTabWidth;
				if (m_bLeftRightRounded)
				{
					x -= m_nTabsHeight / 2;
				}

				if (nRest > 0)
				{
					x ++;
				}
			}
		}
	}

	dc.SelectObject(pOldFont);

	if (IsTabCloseButton())
	{
		for (i = 0; i < m_iTabsNum; i ++)
		{
			CBCGPTabInfo* pTab = (CBCGPTabInfo*) m_arTabs [i];
			ASSERT_VALID (pTab);
			
			if (!pTab->m_bVisible)
			{
				continue;
			}

			BOOL bSetCloseButton = FALSE;
			
			switch (tabCloseButtonMode)
			{
			case TAB_CLOSE_BUTTON_ACTIVE:
			case TAB_CLOSE_BUTTON_HIGHLIGHTED_COMPACT:
			case TAB_CLOSE_BUTTON_HIGHLIGHTED:
				if (i == m_iActiveTab)
				{
					bSetCloseButton = TRUE;
				}
				break;

			case TAB_CLOSE_BUTTON_ALL:
				bSetCloseButton = TRUE;
				break;
			}

			if (bSetCloseButton)
			{
				SetupTabCloseButton(pTab, i);
			}
		}
	}
}
//***************************************************************************************
void CBCGPTabWnd::Draw3DTab (CDC* pDC, CBCGPTabInfo* pTab, BOOL bActive)
{
	ASSERT_VALID (pTab);
	ASSERT_VALID (pDC);

	if ((m_bIsOneNoteStyle || m_bIsVS2005Style || m_bLeftRightRounded)
		&& pTab->m_rect.left < m_rectTabsArea.left)
	{
		return;
	}

	if (pTab->m_bVisible)
	{
		CRect rectInter;
		if (m_rectCurrClip.IsRectEmpty () ||
			rectInter.IntersectRect (pTab->m_rect, m_rectCurrClip))
		{
			CBCGPVisualManager::GetInstance ()->OnDrawTab (
				pDC, pTab->m_rect, m_iCurTab, bActive, this);
		}
	}
}
//***************************************************************************************
void CBCGPTabWnd::DrawFlatTab (CDC* pDC, CBCGPTabInfo* pTab, BOOL bActive)
{
	ASSERT_VALID (pTab);
	ASSERT_VALID (pDC);

	if (pTab->m_bVisible)
	{
		CBCGPVisualManager::GetInstance ()->OnDrawTab (
			pDC, pTab->m_rect, m_iCurTab, bActive, this);
	}
}
//***************************************************************************************
void CBCGPTabWnd::OnLButtonDown(UINT nFlags, CPoint point) 
{
	if (m_rectTabSplitter.PtInRect (point))
	{
		m_bTrackSplitter = TRUE;
		SetCapture ();
		return;
	}

	if (m_ResizeMode != RESIZE_NO && 
		m_rectResize.PtInRect (point))
	{
		RECT rectBounds;
		LRESULT lResult = GetParent ()->SendMessage (BCGM_GETDRAGBOUNDS, 
				(WPARAM) (LPVOID)this, (LPARAM) (LPVOID) &rectBounds);
		m_rectResizeBounds = rectBounds;

		if (lResult != 0 && !m_rectResizeBounds.IsRectEmpty ())
		{
			m_bResize = TRUE;
			SetCapture ();
			m_rectResizeDrag = m_rectResize;
			ClientToScreen (m_rectResizeDrag);

			CRect rectEmpty;
			rectEmpty.SetRectEmpty ();

			DrawResizeDragRect (m_rectResizeDrag, rectEmpty);
			return;
		}
	}

	if (IsMDITabGroup ())
	{
		int nTab = GetTabFromPoint (point);
		if (nTab == m_iActiveTab)
		{
			ActivateMDITab (nTab);
		}
	}
	
	CBCGPBaseTabWnd::OnLButtonDown(nFlags, point);

	if (!m_bReadyToDetach)
	{
		CWnd* pWndTarget = FindTargetWnd (point);
		if (pWndTarget != NULL)
		{
			ASSERT_VALID (pWndTarget);

			MapWindowPoints (pWndTarget, &point, 1);
			pWndTarget->SendMessage (WM_LBUTTONDOWN, nFlags, 
									MAKELPARAM (point.x, point.y));
		}
	}
}
//***************************************************************************************
void CBCGPTabWnd::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	for (POSITION pos = m_lstButtons.GetHeadPosition (); pos != NULL;)
	{
		CWnd* pWndBtn = CWnd::FromHandle (m_lstButtons.GetNext (pos));
		ASSERT_VALID (pWndBtn);

		CRect rectBtn;
		pWndBtn->GetClientRect (rectBtn);

		pWndBtn->MapWindowPoints (this, rectBtn);

		if (rectBtn.PtInRect (point))
		{
			return;
		}
	}

	CWnd* pWndTarget = FindTargetWnd (point);
	if (pWndTarget != NULL)
	{
		ASSERT_VALID (pWndTarget);

		MapWindowPoints (pWndTarget, &point, 1);
		pWndTarget->SendMessage (WM_LBUTTONDBLCLK, nFlags, 
								MAKELPARAM (point.x, point.y));
	}
	else
	{
		CBCGPBaseTabWnd::OnLButtonDblClk(nFlags, point);
	}
}
//****************************************************************************************
int CBCGPTabWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CBCGPBaseTabWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	CRect rectDummy (0, 0, 0, 0);

	if (m_bScroll)
	{
		//-----------------------
		// Create scroll buttons:
		//-----------------------
		if (m_bFlat)
		{
			m_btnScrollFirst.Create (_T(""), WS_CHILD | WS_VISIBLE, rectDummy, this, (UINT) -1);
			m_btnScrollFirst.SetStdImage (CBCGPMenuImages::IdArowFirst);
			m_btnScrollFirst.m_bDrawFocus = FALSE;
			m_btnScrollFirst.m_nFlatStyle = CBCGPButton::BUTTONSTYLE_FLAT;
			m_lstButtons.AddTail (m_btnScrollFirst.GetSafeHwnd ());
		}

		m_btnScrollLeft.Create (_T(""), WS_CHILD | WS_VISIBLE, rectDummy, this, (UINT) -1);
		m_btnScrollLeft.SetStdImage (
			m_bFlat ? CBCGPMenuImages::IdArowLeftLarge : CBCGPMenuImages::IdArowLeftTab3d,
			m_bIsOneNoteStyle || m_bIsVS2005Style || m_bFlat ? 
					CBCGPMenuImages::ImageBlack : CBCGPMenuImages::ImageDkGray,
			m_bFlat ? (CBCGPMenuImages::IMAGES_IDS) 0 : CBCGPMenuImages::IdArowLeftDsbldTab3d);
		m_btnScrollLeft.m_bDrawFocus = FALSE;
		m_btnScrollLeft.m_nFlatStyle = CBCGPButton::BUTTONSTYLE_FLAT;

		if (!m_bIsOneNoteStyle && !m_bIsVS2005Style)
		{
			m_btnScrollLeft.SetAutorepeatMode (50);
		}

		m_lstButtons.AddTail (m_btnScrollLeft.GetSafeHwnd ());

		m_btnScrollRight.Create (_T(""), WS_CHILD | WS_VISIBLE, rectDummy, this, (UINT) -1);
		m_btnScrollRight.SetStdImage (
			m_bFlat ? CBCGPMenuImages::IdArowRightLarge : CBCGPMenuImages::IdArowRightTab3d,
			m_bIsOneNoteStyle || m_bIsVS2005Style || m_bFlat ? 
					CBCGPMenuImages::ImageBlack : CBCGPMenuImages::ImageDkGray,
			m_bFlat ? (CBCGPMenuImages::IMAGES_IDS) 0 : CBCGPMenuImages::IdArowRightDsbldTab3d);
		m_btnScrollRight.m_bDrawFocus = FALSE;
		m_btnScrollRight.m_nFlatStyle = CBCGPButton::BUTTONSTYLE_FLAT;

		if (!m_bIsOneNoteStyle && !m_bIsVS2005Style)
		{
			m_btnScrollRight.SetAutorepeatMode (50);
		}

		m_lstButtons.AddTail (m_btnScrollRight.GetSafeHwnd ());

		if (m_bFlat)
		{
			m_btnScrollLast.Create (_T(""), WS_CHILD | WS_VISIBLE, rectDummy, this, (UINT) -1);
			m_btnScrollLast.SetStdImage (CBCGPMenuImages::IdArowLast);
			m_btnScrollLast.m_bDrawFocus = FALSE;
			m_btnScrollLast.m_nFlatStyle = CBCGPButton::BUTTONSTYLE_FLAT;
			m_lstButtons.AddTail (m_btnScrollLast.GetSafeHwnd ());
		}

		m_btnClose.Create (_T(""), WS_CHILD | WS_VISIBLE, rectDummy, this, (UINT) -1);
		m_btnClose.SetStdImage (CBCGPMenuImages::IdClose,
			m_bIsOneNoteStyle || m_bIsVS2005Style || m_bFlat ? 
				CBCGPMenuImages::ImageBlack : CBCGPMenuImages::ImageDkGray);
		m_btnClose.m_bDrawFocus = FALSE;
		m_btnClose.m_nFlatStyle = CBCGPButton::BUTTONSTYLE_FLAT;
		m_lstButtons.AddTail (m_btnClose.GetSafeHwnd ());

		if (!m_bFlat && m_bScroll)
		{
			CBCGPLocalResource locaRes;
			CString str;

			str.LoadString (IDS_BCGBARRES_CLOSEBAR);
			m_btnClose.SetTooltip (str);

			str.LoadString (IDP_BCGBARRES_SCROLL_LEFT);
			m_btnScrollLeft.SetTooltip (str);

			str.LoadString (IDP_BCGBARRES_SCROLL_RIGHT);
			m_btnScrollRight.SetTooltip (str);
		}
	}

	if (m_bSharedScroll)
	{
		m_wndScrollWnd.Create (WS_CHILD | WS_VISIBLE | SBS_HORZ, rectDummy,
			this, (UINT) -1);
	}

	if (m_bFlat)
	{
		//---------------------
		// Create active brush:
		//---------------------
		m_brActiveTab.CreateSolidBrush (GetActiveTabColor ());
	}
	else
	{
		//---------------------------------------
		// Text may be truncated. Create tooltip.
		//---------------------------------------
		if (CBCGPTooltipManager::CreateToolTip (m_pToolTip, this,
			BCGP_TOOLTIP_TYPE_TAB))
		{
			m_pToolTip->SetWindowPos (&wndTop, -1, -1, -1, -1,    
									SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOSIZE);
		}
	}

	CBCGPTooltipManager::CreateToolTip (m_pToolTipClose, this, BCGP_TOOLTIP_TYPE_TAB);

	if (globalData.m_hcurStretch == NULL)
	{
		globalData.m_hcurStretch = AfxGetApp ()->LoadCursor (AFX_IDC_HSPLITBAR);
	}

	if (globalData.m_hcurStretchVert == NULL)
	{
		globalData.m_hcurStretchVert = AfxGetApp ()->LoadCursor (AFX_IDC_VSPLITBAR);
	}

	SetTabsHeight ();
	return 0;
}
//***************************************************************************************
BOOL CBCGPTabWnd::SetImageList (UINT uiID, int cx, COLORREF clrTransp)
{
	if (m_bFlat)
	{
		ASSERT (FALSE);
		return FALSE;
	}
	return CBCGPBaseTabWnd::SetImageList (uiID, cx, clrTransp);
}
//***************************************************************************************
BOOL CBCGPTabWnd::SetImageList (HIMAGELIST hImageList)
{
	if (m_bFlat)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	return CBCGPBaseTabWnd::SetImageList (hImageList);
}
//***************************************************************************************
BOOL CBCGPTabWnd::OnEraseBkgnd(CDC* pDC)
{
	if (!m_bTransparent && GetVisibleTabsNum () == 0)
	{
		CRect rectClient;
		GetClientRect (rectClient);
		pDC->FillRect (rectClient, &globalData.brBtnFace);
	}

	return TRUE;
}
//****************************************************************************************
BOOL CBCGPTabWnd::PreTranslateMessage(MSG* pMsg) 
{
   	switch (pMsg->message)
	{
	case WM_KEYDOWN:
		if (m_iActiveTab != -1 &&
			::GetAsyncKeyState (VK_CONTROL) & 0x8000)	// Ctrl is pressed
		{
			switch (pMsg->wParam)
			{
			case VK_NEXT:
				{
					for (int i = m_iActiveTab + 1; i < m_iActiveTab + m_iTabsNum; ++i)
					{
						int iTabIndex = i % m_iTabsNum;
						if (IsTabVisible (iTabIndex))
						{
 							m_bUserSelectedTab = TRUE;
							SetActiveTab (iTabIndex);
							GetActiveWnd ()->SetFocus ();
							FireChangeActiveTab (m_iActiveTab);
							m_bUserSelectedTab = FALSE;
							break;
						}
					}
					return TRUE;
				}
			case VK_PRIOR:
				{
					for (int i = m_iActiveTab - 1 + m_iTabsNum; i > m_iActiveTab; --i)
					{
						int iTabIndex = i % m_iTabsNum;
						if (IsTabVisible (iTabIndex))
						{
						    m_bUserSelectedTab = TRUE;
							SetActiveTab (iTabIndex);
							GetActiveWnd ()->SetFocus ();
							FireChangeActiveTab (m_iActiveTab);
							m_bUserSelectedTab = FALSE;
							break;
						}
					}
					return TRUE;
				}
			}
		}
			
		// Continue....

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
		if (m_pToolTipClose->GetSafeHwnd () != NULL)
		{
			m_pToolTipClose->RelayEvent(pMsg);
		}

		if (m_pToolTip->GetSafeHwnd () != NULL)
		{
			m_pToolTip->RelayEvent(pMsg);
		}
		break;
	}

	if (pMsg->message == WM_LBUTTONDBLCLK && pMsg->hwnd == m_btnClose.GetSafeHwnd ())
	{
		return TRUE;
	}

	return CBCGPBaseTabWnd::PreTranslateMessage(pMsg);
}
//******************************************************************************************
void CBCGPTabWnd::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	if (!m_bFlat)
	{
		CBCGPBaseTabWnd::OnHScroll (nSBCode, nPos, pScrollBar);
		return;
	}

	if (pScrollBar->GetSafeHwnd () == m_wndScrollWnd.GetSafeHwnd ())
	{
		static BOOL bInsideScroll = FALSE;

		if (m_iActiveTab != -1 && !bInsideScroll)
		{
			CWnd* pWndActive = GetActiveWnd ();
			ASSERT_VALID (pWndActive);

			CBCGPTabInfo* pTab = (CBCGPTabInfo*) m_arTabs [m_iActiveTab];
			ASSERT_VALID (pTab);

			WPARAM wParam = MAKEWPARAM (nSBCode, nPos);

			//----------------------------------
			// Pass scroll to the active window:
			//----------------------------------
			bInsideScroll = TRUE;

			if (pTab->m_bIsListView &&
				(LOBYTE (nSBCode) == SB_THUMBPOSITION ||
				LOBYTE (nSBCode) == SB_THUMBTRACK))
			{
				int dx = nPos - pWndActive->GetScrollPos (SB_HORZ);
				pWndActive->SendMessage (LVM_SCROLL, dx, 0);
			}

			pWndActive->SendMessage (WM_HSCROLL, wParam, 0);

			bInsideScroll = FALSE;

			m_wndScrollWnd.SetScrollPos (pWndActive->GetScrollPos (SB_HORZ));

			HideActiveWindowHorzScrollBar ();
			GetParent ()->SendMessage (BCGM_ON_HSCROLL, wParam);
		}

		return;
	}

	CBCGPBaseTabWnd::OnHScroll (nSBCode, nPos, pScrollBar);
}
//******************************************************************************************
CWnd* CBCGPTabWnd::FindTargetWnd (const CPoint& point)
{
	if (point.y < m_nTabsHeight)
	{
		return NULL;
	}

	for (int i = 0; i < m_iTabsNum; i ++)
	{
		CBCGPTabInfo* pTab = (CBCGPTabInfo*) m_arTabs [i];
		ASSERT_VALID (pTab);
		
		if (!pTab->m_bVisible)
			continue;
		
		if (pTab->m_rect.PtInRect (point))
		{
			return NULL;
		}
	}

	CWnd* pWndParent = GetParent ();
	ASSERT_VALID (pWndParent);

	return pWndParent;
}
//************************************************************************************
void CBCGPTabWnd::AdjustTabsScroll ()
{
	ASSERT_VALID (this);

	if (!m_bScroll)
	{
		m_nTabsHorzOffset = 0;
		m_nFirstVisibleTab = 0;
		return;
	}

	if (m_iTabsNum == 0)
	{
		m_nTabsHorzOffsetMax = 0;
		m_nTabsHorzOffset = 0;
		m_nFirstVisibleTab = 0;
		return;
	}

	int nPrevHorzOffset = m_nTabsHorzOffset;

	m_nTabsHorzOffsetMax = max (0, m_nTabsTotalWidth - m_rectTabsArea.Width ());

	if (m_bIsOneNoteStyle || m_bIsVS2005Style || m_bLeftRightRounded)
	{
		m_nTabsHorzOffset = max (0, m_nTabsHorzOffset);
	}
	else
	{
		m_nTabsHorzOffset = min (max (0, m_nTabsHorzOffset), m_nTabsHorzOffsetMax);
	}

	if (nPrevHorzOffset != m_nTabsHorzOffset)
	{
		AdjustTabs ();
		InvalidateRect (m_rectTabsArea);
		UpdateWindow ();
	}

	UpdateScrollButtonsState ();
}
//*************************************************************************************
void CBCGPTabWnd::RecalcLayout ()
{
	if (GetSafeHwnd () == NULL)
	{
		return;
	}

	ASSERT_VALID (this);

	const int nPointerHeight = GetPointerAreaHeight();
	int nTabsHeight = GetTabsHeight ();
	const int nTabBorderSize = GetTabBorderSize ();

	int nVisiableTabs = GetVisibleTabsNum ();

	BOOL bHideTabs =	(m_bHideSingleTab && nVisiableTabs <= 1) ||
						(m_bHideNoTabs && nVisiableTabs == 0);

	CRect rectClient;
	GetClientRect (rectClient);

	switch (m_ResizeMode)
	{
	case RESIZE_VERT:
		m_rectResize = rectClient;
		rectClient.right -= RESIZEBAR_SIZE;
		m_rectResize.left = rectClient.right + 1;
		break;

	case RESIZE_HORIZ:
		m_rectResize = rectClient;
		rectClient.bottom -= RESIZEBAR_SIZE;
		m_rectResize.top = rectClient.bottom + 1;
		break;

	default:
		m_rectResize.SetRectEmpty ();
	}

	m_rectTabsArea = rectClient;
	m_rectTabsArea.DeflateRect (CBCGPVisualManager::GetInstance ()->GetTabsMargin(this), 0);

	int nScrollBtnWidth = 0;
	int nButtons = 0;
	int nButtonsWidth = 0;
	int nButtonsHeight = 0;
	int nButtonMargin = 0;

	if (m_bScroll)
	{
		if (m_bScrollButtonFullSize)
		{
			nScrollBtnWidth = 3 * nTabsHeight / 2;
		}
		else
		{
			nScrollBtnWidth = CBCGPMenuImages::Size ().cx + 4 +
				CBCGPVisualManager::GetInstance ()->GetButtonExtraBorder ().cx;

			if (!m_bFlat)
			{
				nScrollBtnWidth = min (nTabsHeight - 4 , nScrollBtnWidth + 2);
			}
		}
		
		nButtons = (int) m_lstButtons.GetCount ();
		if (!m_bCloseBtn || IsTabCloseButton())
		{
			nButtons--;
		}

		if (m_bTabDocumentsMenu)
		{
			nButtons--;
		}

		nButtonMargin = 3;
		nButtonsWidth = bHideTabs || !m_bButtonsVisible ? 0 : (nScrollBtnWidth + nButtonMargin) * nButtons;
	}

	if (m_bFlat)
	{
		if (m_location == LOCATION_BOTTOM)
		{
			if (nTabBorderSize > 1)
			{
				m_rectTabsArea.bottom -= nTabBorderSize - 1;
			}

			m_rectTabsArea.top = m_rectTabsArea.bottom - nTabsHeight;
		}
		else
		{
			if (nTabBorderSize > 1)
			{
				m_rectTabsArea.top += nTabBorderSize - 1;
			}

			m_rectTabsArea.bottom = m_rectTabsArea.top + nTabsHeight;
		}

		m_rectTabsArea.left += nButtonsWidth + 1;
		m_rectTabsArea.right--;

		if (m_rectTabsArea.right < m_rectTabsArea.left)
		{
			if (nTabBorderSize > 0)
			{
				m_rectTabsArea.left = rectClient.left + nTabBorderSize + 1;
				m_rectTabsArea.right = rectClient.right - nTabBorderSize - 1;
			}
			else
			{
				m_rectTabsArea.left = rectClient.left;
				m_rectTabsArea.right = rectClient.right;
			}
		}

		nButtonsHeight = m_rectTabsArea.Height ();

		if (m_rectTabsArea.Height () + nTabBorderSize > rectClient.Height ())
		{
			nButtonsHeight = 0;
			m_rectTabsArea.left = 0;
			m_rectTabsArea.right = 0;
		}

		int y = m_rectTabsArea.top;

		if (nButtonsHeight != 0 && !m_bScrollButtonFullSize)
		{
			y += max (0, (nButtonsHeight - nScrollBtnWidth) / 2);
			nButtonsHeight = nScrollBtnWidth;
		}

		// Reposition scroll butons:
		ReposButtons (	CPoint (rectClient.left + nTabBorderSize + 1, y), 
						CSize (nScrollBtnWidth, nButtonsHeight),
						bHideTabs, nButtonMargin);
	}
	else
	{
		if (m_location == LOCATION_BOTTOM)
		{
			m_rectTabsArea.top = m_rectTabsArea.bottom - nTabsHeight;
		}
		else
		{
			m_rectTabsArea.bottom = m_rectTabsArea.top + nTabsHeight;
		}

		if (m_bScroll)
		{
			m_rectTabsArea.right -= nButtonsWidth;

			if ((m_bIsOneNoteStyle || m_bIsVS2005Style || m_bLeftRightRounded) && !m_bTabDocumentsMenu)
			{
				m_rectTabsArea.OffsetRect (nScrollBtnWidth, 0);
			}

			int nButtonsHeight = m_bScrollButtonFullSize ? nTabsHeight : nScrollBtnWidth;

			// Reposition scroll butons:
			ReposButtons (	
				CPoint (m_rectTabsArea.right + 1, m_rectTabsArea.CenterPoint ().y - nButtonsHeight / 2),
				CSize (nScrollBtnWidth, nButtonsHeight), bHideTabs, nButtonMargin);
		}
	}

	m_rectWndArea = rectClient;
	m_nScrollBarRight = m_rectTabsArea.right - ::GetSystemMetrics (SM_CXVSCROLL);

	if (nTabBorderSize > 0)
	{
		m_rectWndArea.DeflateRect (nTabBorderSize + 1, nTabBorderSize + 1);
	}
	
	if (m_bFlat)
	{
		if (m_location == LOCATION_BOTTOM)
		{
			m_rectWndArea.bottom = m_rectTabsArea.top;

			if (nTabBorderSize == 0)
			{
				m_rectWndArea.top++;
				m_rectWndArea.left++;
			}
		}
		else
		{
			m_rectWndArea.top = m_rectTabsArea.bottom + nTabBorderSize;

			if (nTabBorderSize == 0)
			{
				m_rectWndArea.bottom--;
				m_rectWndArea.left++;
			}
		}
	}
	else
	{
		if (m_location == LOCATION_BOTTOM)
		{
			m_rectWndArea.bottom = m_rectTabsArea.top - nTabBorderSize - nPointerHeight;
		}
		else
		{
			m_rectWndArea.top = m_rectTabsArea.bottom + nTabBorderSize + nPointerHeight;
		}
	}

	if (m_bAutoSizeWindow)
	{
		for (int i = 0; i < m_iTabsNum; i ++)
		{
			CBCGPTabInfo* pTab = (CBCGPTabInfo*) m_arTabs [i];
			ASSERT_VALID (pTab);
			
			if (pTab->m_bVisible && pTab->m_pWnd->GetSafeHwnd () != NULL && ::IsWindow(pTab->m_pWnd->GetSafeHwnd()))
			{
				pTab->m_pWnd->SetWindowPos (NULL,
					m_rectWndArea.left, m_rectWndArea.top,
					m_rectWndArea.Width (), m_rectWndArea.Height (),
					SWP_NOACTIVATE | SWP_NOZORDER);
			}
		}
	}

	AdjustWndScroll ();
	AdjustTabs ();

	if (!m_bHiddenDocuments && m_arTabIndexs.GetSize() == m_arTabs.GetSize())
	{
		m_arTabIndexs.RemoveAll();
		AdjustTabs ();
	}

	AdjustTabsScroll ();

	CRect rectFrame = rectClient;
	if (nTabBorderSize == 0)
	{
		if (m_location == LOCATION_BOTTOM)
		{
			rectFrame.bottom = m_rectTabsArea.top + 1;
		}
		else
		{
			rectFrame.top = m_rectTabsArea.bottom - 1;
		}
		InvalidateRect (rectFrame);
	}
	else
	{
		if (!m_bFlat)
		{
			if (m_location == LOCATION_BOTTOM)
			{
				rectFrame.bottom = m_rectTabsArea.top;
			}
			else
			{
				rectFrame.top = m_rectTabsArea.bottom;
			}
		}

		if (m_bFlatFrame)
		{
			CRect rectBorder (rectFrame);

			if (m_bFlat)
			{
				if (m_location == LOCATION_BOTTOM)
				{
					rectBorder.bottom = m_rectTabsArea.top + 1;
				}
				else
				{
					rectBorder.top = m_rectTabsArea.bottom - 1;
				}
			}
			InvalidateRect (rectBorder);
		}
		else
		{
			rectFrame.DeflateRect (1, 1);
			InvalidateRect (rectFrame);
		}
	}

	CRect rcUpdateArea;
	GetClientRect (&rcUpdateArea);

	if (m_location != LOCATION_BOTTOM)
	{
		rcUpdateArea.bottom = m_rectWndArea.bottom;
	}
	else
	{
		rcUpdateArea.top = m_rectWndArea.top;
	}

	InvalidateRect (rcUpdateArea);
	UpdateWindow ();
}
//*************************************************************************************
void CBCGPTabWnd::AdjustWndScroll ()
{
	if (!m_bSharedScroll)
	{
		return;
	}

	ASSERT_VALID (this);

	CRect rectScroll = m_rectTabsArea;

	int nVisiableTabs = GetVisibleTabsNum ();

	BOOL bHideTabs =	(m_bHideSingleTab && nVisiableTabs <= 1) ||
						(m_bHideNoTabs && nVisiableTabs == 0);

	if (!bHideTabs)
	{
		if (m_nHorzScrollWidth >= MIN_SROLL_WIDTH)
		{
			rectScroll.top++;
			rectScroll.right = m_nScrollBarRight;
			rectScroll.left = rectScroll.right - m_nHorzScrollWidth;
			rectScroll.bottom -= 2;

			m_rectTabSplitter = rectScroll;
			m_rectTabSplitter.top ++;
			m_rectTabSplitter.right = rectScroll.left;
			m_rectTabSplitter.left = m_rectTabSplitter.right - SPLITTER_WIDTH;

			m_rectTabsArea.right = m_rectTabSplitter.left;
			
			ASSERT (!m_rectTabSplitter.IsRectEmpty ());
		}
		else
		{
			rectScroll.SetRectEmpty ();
			m_rectTabSplitter.SetRectEmpty ();
		}
	}
	else
	{
		rectScroll.bottom -= 2;
		m_rectTabSplitter.SetRectEmpty ();
	}

	m_wndScrollWnd.SetWindowPos (NULL,
		rectScroll.left, rectScroll.top,
		rectScroll.Width (), rectScroll.Height (),
		SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOCOPYBITS);
}
//***************************************************************************************
BOOL CBCGPTabWnd::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	if (m_bFlat && !m_rectTabSplitter.IsRectEmpty ())
	{
		CPoint ptCursor;
		::GetCursorPos (&ptCursor);
		ScreenToClient (&ptCursor);

		if (m_rectTabSplitter.PtInRect (ptCursor))
		{
			::SetCursor (globalData.m_hcurStretch);
			return TRUE;
		}
	}

	if (!m_rectResize.IsRectEmpty ())
	{
		CPoint ptCursor;
		::GetCursorPos (&ptCursor);
		ScreenToClient (&ptCursor);

		if (m_rectResize.PtInRect (ptCursor))
		{
			::SetCursor (m_ResizeMode == RESIZE_VERT ?
				globalData.m_hcurStretch : globalData.m_hcurStretchVert);
			return TRUE;
		}
	}
	
	return CBCGPBaseTabWnd::OnSetCursor(pWnd, nHitTest, message);
}
//***************************************************************************************
void CBCGPTabWnd::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if (m_bTrackSplitter || m_bResize)
	{
		StopResize (FALSE);
		m_bTrackSplitter = FALSE;
		m_bResize = FALSE;
		ReleaseCapture ();
	}

	if (IsMDITabGroup ())
	{
		CPoint pointDelta;
		GetCursorPos (&pointDelta);
		pointDelta = m_ptHot - pointDelta;
		int nDrag = GetSystemMetrics (SM_CXDRAG);
		if (GetCapture () == this && m_bReadyToDetach && 
			(abs (pointDelta.x) > nDrag || abs (pointDelta.y) > nDrag))
		{
			ReleaseCapture ();
			if (!IsPtInTabArea (point))
			{
				GetParent ()->SendMessage (BCGM_ON_MOVETABCOMPLETE, (WPARAM) this, 
								(LPARAM) MAKELPARAM (point.x, point.y));
			}
		}
		else
		{
			ActivateMDITab ();
		}
	}
	
	CBCGPBaseTabWnd::OnLButtonUp(nFlags, point);
}
//***************************************************************************************
void CBCGPTabWnd::StopResize (BOOL bCancel)
{
	if (m_bResize)
	{
		CRect rectEmpty;
		rectEmpty.SetRectEmpty ();	

		DrawResizeDragRect (rectEmpty, m_rectResizeDrag);

		m_bResize = FALSE;
		ReleaseCapture ();

		if (!bCancel)
		{
			CRect rectWnd;
			GetWindowRect (rectWnd);

			if (m_ResizeMode == RESIZE_VERT)
			{
				rectWnd.right = m_rectResizeDrag.right;
			}
			else if (m_ResizeMode == RESIZE_HORIZ)
			{
				rectWnd.bottom = m_rectResizeDrag.bottom;
			}

			RECT rect = rectWnd;
			GetParent ()->SendMessage (BCGM_ON_DRAGCOMPLETE, (WPARAM) this, (LPARAM) &rect);
		}

		m_rectResizeDrag.SetRectEmpty ();
		m_rectResizeBounds.SetRectEmpty ();
	}
}
//***************************************************************************************
void CBCGPTabWnd::OnMouseMove(UINT nFlags, CPoint point) 
{
	if (m_bResize)
	{
		CRect rectNew = m_rectResizeDrag;
		ClientToScreen (&point);
		CSize size;
		if (m_ResizeMode == RESIZE_VERT) 
		{
			int nWidth = rectNew.Width ();
			size.cx = size.cy = nWidth;

			rectNew.left = point.x - nWidth / 2;
			rectNew.right = rectNew.left + nWidth;

			if (rectNew.left < m_rectResizeBounds.left)
			{
				rectNew.left = m_rectResizeBounds.left;
				rectNew.right = rectNew.left + nWidth;
			}
			else if (rectNew.right > m_rectResizeBounds.right)
			{
				rectNew.right = m_rectResizeBounds.right;
				rectNew.left = rectNew.right - nWidth;
			}
		}
		else if (m_ResizeMode == RESIZE_HORIZ)
		{
			int nHeight = rectNew.Height ();
			size.cx = size.cy = nHeight;

			rectNew.top = point.y - nHeight / 2;
			rectNew.bottom = rectNew.top + nHeight;

			if (rectNew.top < m_rectResizeBounds.top)
			{
				rectNew.top = m_rectResizeBounds.top;
				rectNew.bottom = rectNew.top + nHeight;
			}
			else if (rectNew.bottom > m_rectResizeBounds.bottom)
			{
				rectNew.bottom = m_rectResizeBounds.bottom;
				rectNew.top = rectNew.bottom - nHeight;
			}
		}
	
		DrawResizeDragRect (rectNew, m_rectResizeDrag);
		m_rectResizeDrag = rectNew;
		return;
	}
	if (m_bTrackSplitter)
	{
		int nSplitterLeftPrev = m_rectTabSplitter.left;

		m_nHorzScrollWidth = min (
			m_nScrollBarRight - m_rectTabsArea.left - SPLITTER_WIDTH, 
			m_nScrollBarRight - point.x);

		m_nHorzScrollWidth = max (MIN_SROLL_WIDTH, m_nHorzScrollWidth);
		AdjustWndScroll ();

		if (m_rectTabSplitter.left > nSplitterLeftPrev)
		{
			CRect rect = m_rectTabSplitter;
			rect.left = nSplitterLeftPrev - 20;
			rect.right = m_rectTabSplitter.left;
			rect.InflateRect (0, GetTabBorderSize () + 1);

			InvalidateRect (rect);
		}

		CRect rectTabSplitter = m_rectTabSplitter;
		rectTabSplitter.InflateRect (0, GetTabBorderSize ());

		InvalidateRect (rectTabSplitter);
		UpdateWindow ();
		AdjustTabsScroll ();
	}
	else if (GetCapture () == this && IsMDITabGroup () && m_bReadyToDetach)
	{
		CPoint pointDelta;
		GetCursorPos (&pointDelta);
		pointDelta = m_ptHot - pointDelta;
		int nDrag = GetSystemMetrics (SM_CXDRAG);
		if (GetCapture () == this && m_bReadyToDetach && 
			(abs (pointDelta.x) < nDrag && abs (pointDelta.y) < nDrag))
		{
			return;
		}

		if (GetParent ()->SendMessage (BCGM_ON_TABGROUPMOUSEMOVE, nFlags, MAKELPARAM (point.x, point.y)))
		{
			return;
		}
	}

	if (!m_bFlat)
	{
		if (CBCGPVisualManager::GetInstance ()->AlwaysHighlight3DTabs ())
		{
			m_bHighLightTabs = TRUE;
		}
		else if (m_bIsOneNoteStyle)
		{
			m_bHighLightTabs = CBCGPVisualManager::GetInstance ()->IsHighlightOneNoteTabs ();
		}
	}

	CBCGPBaseTabWnd::OnMouseMove(nFlags, point);
}
//***********************************************************************************
void CBCGPTabWnd::OnCancelMode() 
{
	BOOL bWasCaptured = (GetCapture () == this);

	if (IsMDITabGroup () && bWasCaptured)
	{
		GetParent ()->SendMessage (BCGM_ON_CANCELTABMOVE);
	}

	CBCGPBaseTabWnd::OnCancelMode();
	StopResize (TRUE);

	if (m_bTrackSplitter)
	{
		m_bResize = FALSE;
		m_bTrackSplitter = FALSE;
		ReleaseCapture ();
	}
}
//**********************************************************************************
void CBCGPTabWnd::OnSysColorChange() 
{
	CBCGPBaseTabWnd::OnSysColorChange();

	if (m_bFlat && m_clrActiveTabFg == (COLORREF) -1)
	{
		if (m_brActiveTab.GetSafeHandle () != NULL)
		{
			m_brActiveTab.DeleteObject ();
		}

		m_brActiveTab.CreateSolidBrush (GetActiveTabColor ());

		Invalidate ();
		UpdateWindow ();
	}
}
//***********************************************************************************
BOOL CBCGPTabWnd::SynchronizeScrollBar (SCROLLINFO* pScrollInfo/* = NULL*/)
{
	if (!m_bSharedScroll)
	{
		return FALSE;
	}

	ASSERT_VALID (this);

	SCROLLINFO scrollInfo;
	memset (&scrollInfo, 0, sizeof (SCROLLINFO));

	scrollInfo.cbSize = sizeof (SCROLLINFO);
	scrollInfo.fMask = SIF_ALL;

	CWnd* pWndActive = GetActiveWnd ();

	if (pScrollInfo != NULL)
	{
		scrollInfo = *pScrollInfo;
	}
	else if (pWndActive != NULL)
	{
		if (!pWndActive->GetScrollInfo (SB_HORZ, &scrollInfo) ||
			scrollInfo.nMin + (int) scrollInfo.nPage >= scrollInfo.nMax)
		{
			m_wndScrollWnd.EnableScrollBar (ESB_DISABLE_BOTH);
			return TRUE;
		}
	}

	m_wndScrollWnd.EnableScrollBar (ESB_ENABLE_BOTH);
	m_wndScrollWnd.SetScrollInfo (&scrollInfo);

	HideActiveWindowHorzScrollBar ();
	return TRUE;
}
//*************************************************************************************
void CBCGPTabWnd::HideActiveWindowHorzScrollBar ()
{
	CWnd* pWnd = GetActiveWnd ();
	if (pWnd == NULL || !m_bSharedScroll)
	{
		return;
	}

	ASSERT_VALID (pWnd);

	pWnd->ShowScrollBar (SB_HORZ, FALSE);
	pWnd->ModifyStyle (WS_HSCROLL, 0, SWP_DRAWFRAME);
}
//************************************************************************************
void CBCGPTabWnd::SetTabsHeight ()
{
	int nExtraHeight = CBCGPVisualManager::GetInstance ()->GetTabExtraHeight(this);

	if (m_bFlat && m_hFontCustom == NULL)
	{
		m_nTabsHeight = ::GetSystemMetrics (SM_CYHSCROLL) + CBCGPBaseTabWnd::TAB_TEXT_MARGIN / 2 + nExtraHeight;

		LOGFONT lfDefault;
		globalData.fontRegular.GetLogFont (&lfDefault);

		LOGFONT lf;
		memset (&lf, 0, sizeof (LOGFONT));

		lf.lfCharSet = lfDefault.lfCharSet;
		lf.lfHeight = lfDefault.lfHeight;
		lf.lfQuality = 5 /*CLEARTYPE_QUALITY*/;
		lstrcpy (lf.lfFaceName, TABS_FONT);

		CClientDC dc (this);

		TEXTMETRIC tm;

		do
		{
			m_fntTabs.DeleteObject ();
			m_fntTabs.CreateFontIndirect (&lf);

			CFont* pFont = dc.SelectObject (&m_fntTabs);
			ASSERT (pFont != NULL);

			dc.GetTextMetrics (&tm);
			dc.SelectObject (pFont);

			if (tm.tmHeight + CBCGPBaseTabWnd::TAB_TEXT_MARGIN / 2 <= m_nTabsHeight)
			{
				break;
			}

			//------------------
			// Try smaller font:
			//------------------
			if (lf.lfHeight < 0)
			{
				lf.lfHeight ++;
			}
			else
			{
				lf.lfHeight --;
			}
		}
		while (lf.lfHeight != 0);

		//------------------
		// Create bold font:
		//------------------
		lf.lfWeight = FW_BOLD;
		m_fntTabsBold.DeleteObject ();
		m_fntTabsBold.CreateFontIndirect (&lf);
	}
	else if (m_bIsVS2005Style || m_bIsPointerStyle)
	{
		const int nImageHeight = m_sizeImage.cy <= 0 ? 0 : m_sizeImage.cy + 7;
		int nTextMargin = 0;
		int nTextHeight = GetTextHeight(nTextMargin);

		m_nTabsHeight = (max (nImageHeight, nTextHeight - nTextMargin + nExtraHeight + 6));
	}
	else
	{
		CBCGPBaseTabWnd::SetTabsHeight ();
		m_nTabsHeight += nExtraHeight;
	}
}
//*************************************************************************************
void CBCGPTabWnd::OnSettingChange(UINT uFlags, LPCTSTR lpszSection) 
{
	CBCGPBaseTabWnd::OnSettingChange(uFlags, lpszSection);
	
	//-----------------------------------------------------------------
	// In the flat modetabs height should be same as scroll bar height
	//-----------------------------------------------------------------
	if (m_bFlat)
	{
		SetTabsHeight ();
		RecalcLayout ();
		SynchronizeScrollBar ();
	}
}
//*************************************************************************************
BOOL CBCGPTabWnd::EnsureVisible (int iTab)
{
	if (iTab < 0 || iTab >= m_iTabsNum)
	{
		TRACE(_T("CBCGPTabWnd::EnsureVisible: illegal tab number %d\n"), iTab);
		return FALSE;
	}

	CBCGPTabInfo* pTabInfo = (CBCGPTabInfo*)m_arTabs [iTab];
	CRect rectTab = pTabInfo->m_rect;

	if (m_bTabDocumentsMenu)
	{
		if (rectTab.left + pTabInfo->m_nFullWidth > m_rectTabsArea.right || rectTab.IsRectEmpty ())
		{
			CBCGPBaseTabWnd::MoveTab (iTab, 0);
		}

		return TRUE;
	}

	if (!m_bScroll || m_rectTabsArea.Width () <= 0)
	{
		return TRUE;
	}

	//---------------------------------------------------------
	// Be sure, that active tab is visible (not out of scroll):
	//---------------------------------------------------------
	BOOL bAdjustTabs = FALSE;

	if (m_bIsOneNoteStyle || m_bIsVS2005Style || m_bLeftRightRounded)
	{
		if (rectTab.left < m_rectTabsArea.left || 
			rectTab.right > m_rectTabsArea.right)
		{
			// Calculate total width of tabs located left from active + active:
			int nWidthLeft = 0;
			const int nExtraWidth = m_rectTabsArea.Height () - TAB_IMAGE_MARGIN - 1;

			for (int i = 0; i <= iTab; i++)
			{
				nWidthLeft += ((CBCGPTabInfo*) m_arTabs [i])->m_rect.Width () - nExtraWidth;
			}

			m_nTabsHorzOffset = 0;
			m_nFirstVisibleTab = 0;

			while (	m_nFirstVisibleTab < iTab &&
					nWidthLeft > m_rectTabsArea.Width ())
			{
				const int nCurrTabWidth = 
					((CBCGPTabInfo*) m_arTabs [m_nFirstVisibleTab])->m_rect.Width () - nExtraWidth;

				m_nTabsHorzOffset += nCurrTabWidth;
				nWidthLeft -= nCurrTabWidth;

				m_nFirstVisibleTab++;
			}

			bAdjustTabs = TRUE;
		}
	}
	else
	{
		if (rectTab.left < m_rectTabsArea.left)
		{
			m_nTabsHorzOffset -= (m_rectTabsArea.left - rectTab.left);
			bAdjustTabs = TRUE;
		}
		else if (rectTab.right > m_rectTabsArea.right &&
			rectTab.Width () <= m_rectTabsArea.Width ())
		{
			m_nTabsHorzOffset += (rectTab.right - m_rectTabsArea.right);
			bAdjustTabs = TRUE;
		}
	}

	if (bAdjustTabs)
	{
		AdjustTabs ();
		AdjustTabsScroll ();

		RedrawWindow ();
	}

	return TRUE;
}
//**********************************************************************************
BOOL CBCGPTabWnd::OnNotify (WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	BOOL bRes = CBCGPBaseTabWnd::OnNotify (wParam, lParam, pResult);

	NMHDR* pNMHDR = (NMHDR*)lParam;
	ASSERT (pNMHDR != NULL);

	if (pNMHDR->code == TTN_SHOW)
	{
		if (m_pToolTip->GetSafeHwnd () != NULL)
		{
			m_pToolTip->SetWindowPos (&wndTop, -1, -1, -1, -1,
				SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOSIZE);
		}

		if (m_pToolTipClose->GetSafeHwnd () != NULL && pNMHDR->hwndFrom == m_pToolTipClose->GetSafeHwnd ())
		{
			m_pToolTipClose->SetWindowPos (&wndTop, -1, -1, -1, -1,
				SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOSIZE);
		}
	}

	if (pNMHDR->code == HDN_ITEMCHANGED)
	{
		SynchronizeScrollBar ();
	}

	return bRes;
}
//*********************************************************************************
void CBCGPTabWnd::HideNoTabs (BOOL bHide)
{
	if (m_bHideNoTabs == bHide)
	{
		return;
	}

	m_bHideNoTabs = bHide;

	if (GetSafeHwnd () != NULL)
	{
		RecalcLayout ();
		SynchronizeScrollBar ();
	}
}
//*************************************************************************************
BOOL CBCGPTabWnd::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	const int nScrollOffset = 20;

	BOOL bScrollTabs = FALSE;
	const int nPrevOffset = m_nTabsHorzOffset;

	if ((HWND)lParam == m_btnScrollLeft.GetSafeHwnd ())
	{
		bScrollTabs = TRUE;

		if (m_bIsOneNoteStyle || m_bIsVS2005Style || m_bLeftRightRounded)
		{
			if (m_nFirstVisibleTab > 0)
			{
				m_nTabsHorzOffset -= ((CBCGPTabInfo*) m_arTabs [m_nFirstVisibleTab - 1])->m_rect.Width () - 
					(m_rectTabsArea.Height () - TAB_IMAGE_MARGIN - 2);
				m_nFirstVisibleTab --;
			}
		}
		else
		{
			m_nTabsHorzOffset -= nScrollOffset;
		}
	}
	else if ((HWND)lParam == m_btnScrollRight.GetSafeHwnd ())
	{
		if (m_bTabDocumentsMenu)
		{
			CRect rectButton;
			m_btnScrollRight.GetWindowRect (&rectButton);

			m_btnScrollRight.SetPressed (TRUE);

			CPoint ptMenu (rectButton.left, rectButton.bottom + 2);

			if (GetExStyle () & WS_EX_LAYOUTRTL)
			{
				ptMenu.x += rectButton.Width ();
			}

			m_btnScrollRight.SendMessage (WM_CANCELMODE);

			HWND hwndThis = GetSafeHwnd ();

			m_btnScrollRight.SetPressed (TRUE);

			OnShowTabDocumentsMenu (ptMenu);

			if (!::IsWindow (hwndThis))
			{
				return TRUE;
			}

			m_btnScrollRight.SetPressed (FALSE);
		}
		else
		{
			bScrollTabs = TRUE;

			if (m_bIsOneNoteStyle || m_bIsVS2005Style || m_bLeftRightRounded)
			{
				if (m_nFirstVisibleTab < m_iTabsNum)
				{
					m_nTabsHorzOffset += ((CBCGPTabInfo*) m_arTabs [m_nFirstVisibleTab])->m_rect.Width () - 
						(m_rectTabsArea.Height () - TAB_IMAGE_MARGIN - 1);
					m_nFirstVisibleTab ++;
				}
			}
			else
			{
				m_nTabsHorzOffset += nScrollOffset;
			}
		}
	}
	else if ((HWND)lParam == m_btnScrollFirst.GetSafeHwnd ())
	{
		bScrollTabs = TRUE;
		m_nTabsHorzOffset = 0;
	}
	else if ((HWND)lParam == m_btnScrollLast.GetSafeHwnd ())
	{
		bScrollTabs = TRUE;
		m_nTabsHorzOffset = m_nTabsHorzOffsetMax;
	}
	else if ((HWND)lParam == m_btnClose.GetSafeHwnd ())
	{
		CWnd* pWndActive = GetActiveWnd ();
		if (pWndActive != NULL)
		{
			OnClickCloseButton (pWndActive);
		}

		return TRUE;
	}

	if (bScrollTabs)
	{
		if (m_bIsOneNoteStyle || m_bIsVS2005Style || m_bLeftRightRounded)
		{
			m_nTabsHorzOffset = max (0, m_nTabsHorzOffset);
		}
		else
		{
			m_nTabsHorzOffset = min (max (0, m_nTabsHorzOffset), m_nTabsHorzOffsetMax);
		}

		if (nPrevOffset != m_nTabsHorzOffset)
		{
			AdjustTabs ();
			UpdateScrollButtonsState ();
			Invalidate ();
			UpdateWindow ();
		}

		return TRUE;
	}

	return CBCGPBaseTabWnd::OnCommand(wParam, lParam);
}
//*************************************************************************************
void CBCGTabButton::OnFillBackground (CDC* pDC, const CRect& rectClient)
{
	CBCGPTabWnd* pTabWnd = DYNAMIC_DOWNCAST (CBCGPTabWnd, GetParent ());

	CBCGPVisualManager::GetInstance ()->OnEraseTabsButton (pDC, rectClient, this, pTabWnd);

	if (pTabWnd != NULL)
	{
		m_StdImageState = (CBCGPMenuImages::IMAGE_STATE) pTabWnd->GetButtonState (this);
	}
}
//*************************************************************************************
void CBCGTabButton::OnDrawBorder (CDC* pDC, CRect& rectClient, UINT uiState)
{
	CBCGPVisualManager::GetInstance ()->OnDrawTabsButtonBorder (pDC, rectClient,
		this, uiState, DYNAMIC_DOWNCAST (CBCGPTabWnd, GetParent ()));
}
//***********************************************************************************
void CBCGPTabWnd::OnSetFocus(CWnd* pOldWnd) 
{
	CBCGPBaseTabWnd::OnSetFocus(pOldWnd);
	
	CWnd* pWndActive = GetActiveWnd ();
	if (pWndActive != NULL)
	{
		pWndActive->SetFocus ();
	}
}
//************************************************************************************
void CBCGPTabWnd::ReposButtons (CPoint pt, CSize sizeButton, BOOL bHide, int nButtonMargin)
{
	BOOL bIsFirst = TRUE;
	for (POSITION pos = m_lstButtons.GetHeadPosition (); pos != NULL;)
	{
		HWND hWndButton = m_lstButtons.GetNext (pos);
		ASSERT (hWndButton != NULL);

		BOOL bCloseBtn = m_bCloseBtn && !IsTabCloseButton();

		if (bHide || (!bCloseBtn && hWndButton == m_btnClose.GetSafeHwnd ()) ||
			(m_bTabDocumentsMenu && bIsFirst) || !m_bButtonsVisible)
		{
			::SetWindowPos (hWndButton, NULL,
							0, 0, 0, 0,
							SWP_NOACTIVATE | SWP_NOZORDER);
		}
		else
		{
			int x = pt.x;

			if ((m_bIsOneNoteStyle || m_bIsVS2005Style || m_bLeftRightRounded) && bIsFirst)	// Scroll left is on left
			{
				x = m_rectTabsArea.left - sizeButton.cx - 1;
			}

			::SetWindowPos (hWndButton, NULL,
				x, pt.y,
				sizeButton.cx, sizeButton.cy,
				SWP_NOACTIVATE | SWP_NOZORDER);

			if ((!m_bIsOneNoteStyle && !m_bIsVS2005Style && !m_bLeftRightRounded) || !bIsFirst)
			{
				pt.x += sizeButton.cx + nButtonMargin;
			}
		}

		::InvalidateRect (hWndButton, NULL, TRUE);
		::UpdateWindow (hWndButton);

		bIsFirst = FALSE;
	}
}
//************************************************************************************
BOOL CBCGPTabWnd::IsPtInTabArea (CPoint point) const
{
	return m_rectTabsArea.PtInRect (point);
}
//************************************************************************************
void CBCGPTabWnd::EnableInPlaceEdit (BOOL bEnable)
{
	ASSERT_VALID (this);

	if (!m_bFlat)
	{
		// In-place editing is available for the flat tabs only!
		ASSERT (FALSE);
		return;
	}

	m_bIsInPlaceEdit = bEnable;
}
//************************************************************************************
void CBCGPTabWnd::SetActiveTabBoldFont (BOOL bIsBold)
{
	ASSERT_VALID (this);

	if (m_bFlat && bIsBold)
	{
		ASSERT (FALSE);
		bIsBold = FALSE;
	}

	m_bIsActiveTabBold = bIsBold;

	if (GetSafeHwnd () != NULL)
	{
		RecalcLayout ();
		SynchronizeScrollBar ();
	}
}
//*************************************************************************************
void CBCGPTabWnd::HideSingleTab (BOOL bHide)
{
	if (m_bHideSingleTab == bHide)
	{
		return;
	}

	m_bHideSingleTab = bHide;

	if (GetSafeHwnd () != NULL)
	{
		RecalcLayout ();
		SynchronizeScrollBar ();
	}
}
//*************************************************************************************
void CBCGPTabWnd::UpdateScrollButtonsState ()
{
	ASSERT_VALID (this);

	if (GetSafeHwnd () == NULL || !m_bScroll || m_bFlat)
	{
		return;
	}

	if (m_bIsOneNoteStyle || m_bIsVS2005Style || m_bLeftRightRounded)
	{
		if (m_arTabs.GetSize () == 0)
		{
			m_btnScrollLeft.EnableWindow (FALSE);
			m_btnScrollRight.EnableWindow (FALSE);
		}
		else
		{
			m_btnScrollLeft.EnableWindow (m_nFirstVisibleTab > 0);

			CBCGPTabInfo* pLastTab = (CBCGPTabInfo*) m_arTabs [m_arTabs.GetSize () - 1];
			ASSERT_VALID (pLastTab);

			m_btnScrollRight.EnableWindow (m_bTabDocumentsMenu ||
				(pLastTab->m_rect.right > m_rectTabsArea.right &&
				m_nFirstVisibleTab < m_arTabs.GetSize () - 1));
		}
	}
	else
	{
		m_btnScrollLeft.EnableWindow (m_nTabsHorzOffset > 0);
		m_btnScrollRight.EnableWindow (m_bTabDocumentsMenu || m_nTabsHorzOffset < m_nTabsHorzOffsetMax);
	}

	if (m_bTabDocumentsMenu)
	{
		m_btnScrollRight.SetStdImage (
			m_bHiddenDocuments ? CBCGPMenuImages::IdCustomizeArowDownBold : CBCGPMenuImages::IdArowDownLarge,
			m_bIsOneNoteStyle || m_bIsVS2005Style || m_bLeftRightRounded ? CBCGPMenuImages::ImageBlack : CBCGPMenuImages::ImageDkGray,
			CBCGPMenuImages::IdArowDownLarge);
	}

	for (POSITION pos = m_lstButtons.GetHeadPosition (); pos != NULL;)
	{
		HWND hWndButton = m_lstButtons.GetNext (pos);
		ASSERT (hWndButton != NULL);

		if (!::IsWindowEnabled (hWndButton))
		{
			::SendMessage (hWndButton, WM_CANCELMODE, 0, 0);
		}
	}
}
//*************************************************************************************
BOOL CBCGPTabWnd::ModifyTabStyle (Style style)
{
	ASSERT_VALID (this);

	m_bFlat = (style == STYLE_FLAT);
	m_bIsOneNoteStyle = (style == STYLE_3D_ONENOTE);
	m_bIsVS2005Style = (style == STYLE_3D_VS2005);
	m_bHighLightTabs = m_bIsOneNoteStyle;
	m_bLeftRightRounded = (style == STYLE_3D_ROUNDED || style == STYLE_3D_ROUNDED_SCROLL);
	m_bIsPointerStyle = (style == STYLE_POINTER);
	
	SetScrollButtons ();
	SetTabsHeight ();
	
	return TRUE;
}
//***********************************************************************************
void CBCGPTabWnd::SetScrollButtons ()
{
	const int nAutoRepeat = m_bIsOneNoteStyle || m_bTabDocumentsMenu ? 0 : 50;
	const BOOL bRTL = GetExStyle() & WS_EX_LAYOUTRTL;

	m_btnScrollLeft.SetAutorepeatMode (nAutoRepeat);
	m_btnScrollRight.SetAutorepeatMode (nAutoRepeat);

	m_btnScrollLeft.SetStdImage (
		bRTL ? CBCGPMenuImages::IdArowRightTab3d : CBCGPMenuImages::IdArowLeftTab3d,
		m_bIsOneNoteStyle || m_bIsVS2005Style || m_bLeftRightRounded ? CBCGPMenuImages::ImageBlack : CBCGPMenuImages::ImageDkGray,
		bRTL ? CBCGPMenuImages::IdArowRightDsbldTab3d : CBCGPMenuImages::IdArowLeftDsbldTab3d);

	if (m_bTabDocumentsMenu)
	{
		m_btnScrollRight.SetStdImage (
			CBCGPMenuImages::IdArowDownLarge,
			m_bIsOneNoteStyle || m_bIsVS2005Style || m_bLeftRightRounded ? CBCGPMenuImages::ImageBlack : CBCGPMenuImages::ImageDkGray,
			CBCGPMenuImages::IdArowDownLarge);
	}
	else
	{
		m_btnScrollRight.SetStdImage (
			bRTL ? CBCGPMenuImages::IdArowLeftTab3d : CBCGPMenuImages::IdArowRightTab3d,
			m_bIsOneNoteStyle || m_bIsVS2005Style || m_bLeftRightRounded ? CBCGPMenuImages::ImageBlack : CBCGPMenuImages::ImageDkGray,
			bRTL ? CBCGPMenuImages::IdArowLeftDsbldTab3d : CBCGPMenuImages::IdArowRightDsbldTab3d);
	}

	m_btnClose.SetStdImage (CBCGPMenuImages::IdClose,
		m_bIsOneNoteStyle || m_bIsVS2005Style || m_bLeftRightRounded ? CBCGPMenuImages::ImageBlack : CBCGPMenuImages::ImageDkGray);

	if (m_bFlat)
	{
		m_btnScrollFirst.SetStdImage (bRTL ? CBCGPMenuImages::IdArowLast : CBCGPMenuImages::IdArowFirst);
		m_btnScrollLast.SetStdImage (bRTL ? CBCGPMenuImages::IdArowFirst : CBCGPMenuImages::IdArowLast);
	}
}
//***********************************************************************************
void CBCGPTabWnd::SetDrawFrame (BOOL bDraw)
{
	m_bDrawFrame = bDraw;
}
//**************************************************************************************
DROPEFFECT CBCGPTabWnd::OnDragEnter(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point) 
{
	return OnDragOver(pDataObject, dwKeyState, point);
}
//****************************************************************************************
DROPEFFECT CBCGPTabWnd::OnDragOver(COleDataObject* pDataObject, DWORD dwKeyState, 
										CPoint point) 
{
	if (!GetParent ()->IsKindOf (RUNTIME_CLASS (CBCGPTabbedToolbar)))
	{
		return DROPEFFECT_NONE;
	}

	CBCGPToolbarButton* pButton = CBCGPToolbarButton::CreateFromOleData (pDataObject);
	if (pButton == NULL)
	{
		return DROPEFFECT_NONE;
	}

	if (!pButton->IsKindOf (RUNTIME_CLASS (CBCGPToolbarButton)))
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

	m_bUserSelectedTab = TRUE;
	SetActiveTab (nTab);
	BOOL bCopy = (dwKeyState & MK_CONTROL);
	m_bUserSelectedTab = FALSE;

	return (bCopy) ? DROPEFFECT_COPY : DROPEFFECT_MOVE;
}
//***************************************************************************************
int CBCGPTabWnd::GetTabFromPoint (CPoint& pt) const
{
	ASSERT_VALID (this);

	if (!m_rectTabsArea.PtInRect (pt))
	{
		return -1;
	}

	if (!m_bIsOneNoteStyle && !m_bIsVS2005Style && !m_bLeftRightRounded)
	{
		return CBCGPBaseTabWnd::GetTabFromPoint (pt);
	}

	//------------------------
	// Check active tab first:
	//------------------------
	if (m_iActiveTab >= 0)
	{
		CBCGPTabInfo* pActiveTab = (CBCGPTabInfo*) m_arTabs [m_iActiveTab];
		ASSERT_VALID (pActiveTab);

		CRect rectTab = pActiveTab->m_rect;

		if (rectTab.PtInRect (pt))
		{
			if (!IsFirstTab(m_iActiveTab) && pt.x < rectTab.left + rectTab.Height ())
			{
				const int x = pt.x - rectTab.left;
				const int y = pt.y - rectTab.top;

				if (x * x + y * y < rectTab.Height () * rectTab.Height () / 2)
				{
					for (int i = m_iActiveTab - 1; i >= 0; i--)
					{
						CBCGPTabInfo* pTab = (CBCGPTabInfo*) m_arTabs [i];
						ASSERT_VALID (pTab);

						if (pTab->m_bVisible)
						{
							return i;
						}
					}
				}
			}

			return m_iActiveTab;
		}
	}

	for (int nIndex = 0; nIndex < m_iTabsNum; nIndex++)
	{
		int i = nIndex;
		
		if (m_arTabIndexs.GetSize() == m_iTabsNum)
		{
			i = m_arTabIndexs[nIndex];
		}
		
		CBCGPTabInfo* pTab = (CBCGPTabInfo*) m_arTabs [i];
		ASSERT_VALID (pTab);

		if (pTab->m_bVisible && pTab->m_rect.PtInRect (pt))
		{
			return i;
		}
	}

	return -1;
}
//***************************************************************************************
void CBCGPTabWnd::SwapTabs (int nFisrtTabID, int nSecondTabID)
{
	ASSERT_VALID (this);

	CBCGPBaseTabWnd::SwapTabs (nFisrtTabID, nSecondTabID);

	if (m_bIsOneNoteStyle || m_bIsVS2005Style || m_bLeftRightRounded)
	{
		m_nTabsHorzOffset = 0;
		m_nFirstVisibleTab = 0;
	}
}
//***************************************************************************************
void CBCGPTabWnd::MoveTab(int nSource, int nDest)
{
	ASSERT_VALID (this);

	CBCGPBaseTabWnd::MoveTab (nSource, nDest);

	if (m_bIsOneNoteStyle || m_bIsVS2005Style || m_bLeftRightRounded)
	{
		m_nTabsHorzOffset = 0;
		m_nFirstVisibleTab = 0;

		EnsureVisible (m_iActiveTab);
	}
}
//***************************************************************************************
void CBCGPTabWnd::EnableTabDocumentsMenu (BOOL bEnable/* = TRUE*/)
{
	ASSERT_VALID (this);

	if (m_bFlat && !m_bScroll)
	{
		ASSERT (FALSE);
		return;
	}

	CBCGPLocalResource locaRes;

	m_bTabDocumentsMenu = bEnable;

	if (m_btnScrollRight.GetSafeHwnd() != NULL)
	{
		CString str;
		str.LoadString (m_bTabDocumentsMenu ?
			IDS_BCGBARRES_OPENED_DOCS : IDP_BCGBARRES_SCROLL_RIGHT);
		m_btnScrollRight.SetTooltip (str);
	}

	SetScrollButtons ();

	RecalcLayout ();

	m_nTabsHorzOffset = 0;
	m_nFirstVisibleTab = 0;

	if (m_iActiveTab >= 0)
	{
		EnsureVisible (m_iActiveTab);
	}
}
//***************************************************************************************
void CBCGPTabWnd::EnableActiveTabCloseButton (BOOL bEnable/* = TRUE*/)
{
	ASSERT_VALID (this);

	SetTabCloseButtonMode(bEnable ? TAB_CLOSE_BUTTON_ACTIVE : TAB_CLOSE_BUTTON_NONE);
}
//***************************************************************************************
void CBCGPTabWnd::SetTabCloseButtonMode(TabCloseButtonMode mode)
{
	ASSERT_VALID (this);

	m_TabCloseButtonMode = mode;
	RecalcLayout ();

	if (m_iActiveTab >= 0)
	{
		EnsureVisible (m_iActiveTab);
	}
}
//****************************************************************************
void CBCGPTabWnd::OnShowTabDocumentsMenu (CPoint point)
{
	const UINT idStart = (UINT) -100;

	CMenu menu;
	menu.CreatePopupMenu ();

	for (int i = 0; i < m_iTabsNum; i ++)
	{
		CBCGPTabInfo* pTab = (CBCGPTabInfo*) m_arTabs [i];
		ASSERT_VALID (pTab);

		if (!pTab->m_bVisible || pTab->m_bIsNewTab)
		{
			continue;
		}

		const UINT uiID = idStart - i;
		CString strTabName = pTab->m_strText;

		//--------------------------------
		// Replace all single '&' by '&&':
		//--------------------------------
		const CString strDummyAmpSeq = _T("\001\001");

		strTabName.Replace (_T("&&"), strDummyAmpSeq);
		strTabName.Replace (_T("&"), _T("&&"));
		strTabName.Replace (strDummyAmpSeq, _T("&&"));

		// Insert sorted:
		BOOL bInserted = FALSE;

		for (int iMenu = 0; iMenu < (int)menu.GetMenuItemCount (); iMenu++)
		{
			CString strMenuItem;
			menu.GetMenuString (iMenu, strMenuItem, MF_BYPOSITION);

			if (strTabName.CompareNoCase (strMenuItem) < 0)
			{
				menu.InsertMenu (iMenu, MF_BYPOSITION, uiID, strTabName);
				bInserted = TRUE;
				break;
			}
		}

		if (!bInserted)
		{
			menu.AppendMenu (MF_STRING, uiID, strTabName);
		}

		if (pTab->m_pWnd->GetSafeHwnd () != NULL)
		{
			HICON hIcon = pTab->m_pWnd->GetIcon (FALSE);
			if (hIcon == NULL)
			{
				hIcon = (HICON)(LONG_PTR) GetClassLongPtr (pTab->m_pWnd->GetSafeHwnd (), GCLP_HICONSM);
			}

			m_mapDocIcons.SetAt (uiID, hIcon);
		}
	}

	HWND hwndThis = GetSafeHwnd ();

	int nMenuResult = 0;
	
	if (g_pContextMenuManager != NULL)
	{
		nMenuResult = g_pContextMenuManager->TrackPopupMenu(menu, point.x, point.y, this);
	}
	else
	{
		nMenuResult = ::TrackPopupMenuEx(menu, TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD, point.x, point.y, GetSafeHwnd (), NULL);
	}

	if (!::IsWindow (hwndThis))
	{
		return;
	}

	int iTab = idStart - nMenuResult;
	if (iTab >= 0 && iTab < m_iTabsNum)
	{
		m_bUserSelectedTab = TRUE;
		SetActiveTab (iTab);
		m_bUserSelectedTab = FALSE;
	}

	m_mapDocIcons.RemoveAll ();
}
//****************************************************************************
HICON CBCGPTabWnd::GetDocumentIcon (UINT nCmdID)
{
	HICON hIcon = NULL;
	m_mapDocIcons.Lookup (nCmdID, hIcon);

	return hIcon;
}
//****************************************************************************
void CBCGPTabWnd::SetTabMaxWidth (int nTabMaxWidth)
{
	m_nTabMaxWidth = nTabMaxWidth;
	RecalcLayout ();
}
//****************************************************************************
void CBCGPTabWnd::SetResizeMode (ResizeMode resizeMode)
{
	m_ResizeMode = resizeMode;
	RecalcLayout ();
}
//****************************************************************************
void CBCGPTabWnd::OnWindowPosChanged(WINDOWPOS FAR* lpwndpos) 
{
	CBCGPBaseTabWnd::OnWindowPosChanged(lpwndpos);
	if (IsMDITabGroup ())
	{	
		lpwndpos->hwndInsertAfter = HWND_BOTTOM;
	}
}
//****************************************************************************
void CBCGPTabWnd::OnWindowPosChanging(WINDOWPOS FAR* lpwndpos) 
{
	CBCGPBaseTabWnd::OnWindowPosChanging(lpwndpos);
	if (IsMDITabGroup ())
	{	
		lpwndpos->hwndInsertAfter = HWND_BOTTOM;
	}
}
//****************************************************************************
void CBCGPTabWnd::DrawResizeDragRect (CRect& rectNew, CRect& rectOld)
{
	CWindowDC dc (GetDesktopWindow ());
	CSize size;

	if (m_ResizeMode == RESIZE_VERT)
	{
		size.cx = size.cy = m_rectResizeDrag.Width () / 2 + 1;
	}
	else
	{
		size.cx = size.cy = m_rectResizeDrag.Height () / 2 + 1;
	}

	dc.DrawDragRect (rectNew, size, rectOld, size);
}
//****************************************************************************
BOOL CBCGPTabWnd::IsMDITabGroup () const
{
	CWnd* pParent = GetParent ();
	if (pParent != NULL)
	{	
		ASSERT_VALID (pParent);
		return pParent->IsKindOf (RUNTIME_CLASS (CBCGPMainClientAreaWnd));
	}
	return FALSE;
}
//****************************************************************************
void CBCGPTabWnd::ActivateMDITab (int nTab)
{
	ASSERT (IsMDITabGroup ());

	if (nTab == -1)
	{
		nTab = m_iActiveTab;
	}

	if (nTab == -1)
	{
		return; 
	}

	CWnd* pActiveWnd = GetTabWnd (nTab);
	if (pActiveWnd != NULL)
	{
		ASSERT_VALID (pActiveWnd);

		if (nTab != m_iActiveTab)
		{
			if (!SetActiveTab (nTab))
			{
				return;
			}
		}

		GetParent ()->SendMessage (WM_MDIACTIVATE, (WPARAM) pActiveWnd->GetSafeHwnd ());
		pActiveWnd->SetFocus ();
	}
}
//**************************************************************************
LRESULT CBCGPTabWnd::OnBCGUpdateToolTips (WPARAM wp, LPARAM)
{
	UINT nTypes = (UINT) wp;

	if ((nTypes & BCGP_TOOLTIP_TYPE_TAB) == 0)
	{
		return 0;
	}

	CBCGPTooltipManager::CreateToolTip (m_pToolTip, this,
		BCGP_TOOLTIP_TYPE_TAB);

	if (m_pToolTip->GetSafeHwnd () == NULL)
	{
		return 0;
	}

	CRect rectDummy (0, 0, 0, 0);

	CBCGPTooltipManager::CreateToolTip (m_pToolTipClose, this,
		BCGP_TOOLTIP_TYPE_TAB);

	for (int i = 0; i < m_iTabsNum; i++)
	{
		CBCGPTabInfo* pTab = (CBCGPTabInfo*) m_arTabs [i];
		ASSERT_VALID (pTab);

		m_pToolTip->AddTool (this,
			m_bCustomToolTips ? LPSTR_TEXTCALLBACK : (LPCTSTR)(pTab->m_strText), 
			&rectDummy, pTab->m_iTabID);

		if (m_pToolTipClose->GetSafeHwnd () != NULL)
		{
			m_pToolTipClose->AddTool (this, LPSTR_TEXTCALLBACK, &rectDummy, pTab->m_iTabID);
		}
	}

	RecalcLayout ();
	return 0;
}
//**************************************************************************
void CBCGPTabWnd::SetButtonsVisible (BOOL bVisisble/* = TRUE*/)
{
	m_bButtonsVisible = bVisisble;
	RecalcLayout ();
}
//**************************************************************************
int CBCGPTabWnd::GetButtonState (CBCGTabButton* pButton)
{
	int nState = CBCGPVisualManager::GetInstance ()->GetTabButtonState (this, pButton);
	if (nState != -1)
	{
		return nState;
	}

	return m_bIsOneNoteStyle || m_bIsVS2005Style || m_bLeftRightRounded || m_bIsPointerStyle ? CBCGPMenuImages::ImageBlack : CBCGPMenuImages::ImageDkGray;
}
//**************************************************************************
void CBCGPTabWnd::EnableNewTab(BOOL bEnable/* = TRUE*/)
{
	m_bNewTab = bEnable;

	if (m_bNewTab)
	{
		CBCGPTabInfo* pTabInfo = new CBCGPTabInfo (_T(""), (UINT)-1, NULL, m_nNextTabID, FALSE);
		pTabInfo->m_bIsNewTab = TRUE;
		
		m_arTabs.InsertAt (m_iTabsNum, pTabInfo);

		m_iTabsNum++;

		if (m_pToolTip->GetSafeHwnd () != NULL)
		{
			CRect rectEmpty (0, 0, 0, 0);
			m_pToolTip->AddTool (this, 
				m_bCustomToolTips ? LPSTR_TEXTCALLBACK : _T(""),
				&rectEmpty, m_nNextTabID);
		}

		m_nNextTabID ++;
	}
	else
	{
		// Remove 'new' tab:
		if (m_iTabsNum > 0)
		{
			CBCGPTabInfo* pTab = (CBCGPTabInfo*) m_arTabs [m_iTabsNum - 1];
			ASSERT_VALID (pTab);

			if (pTab->m_bIsNewTab)
			{
				RemoveTab(m_iTabsNum - 1, FALSE);
			}
		}
	}

	RecalcLayout ();
}
//**************************************************************************
void CBCGPTabWnd::OnChangeHighlightedTab(int iPrevHighlighted)
{
	CBCGPBaseTabWnd::OnChangeHighlightedTab(iPrevHighlighted);

	if (!IsTabCloseButton())
	{
		return;
	}

	TabCloseButtonMode tabCloseButtonMode = m_TabCloseButtonMode;
	if (m_bFlat)
	{
		if (tabCloseButtonMode == TAB_CLOSE_BUTTON_HIGHLIGHTED_COMPACT || tabCloseButtonMode == TAB_CLOSE_BUTTON_HIGHLIGHTED)
		{
			tabCloseButtonMode = TAB_CLOSE_BUTTON_ACTIVE;
		}
	}

	if (iPrevHighlighted >= 0 && iPrevHighlighted < m_arTabs.GetSize())
	{
		CBCGPTabInfo* pTab = (CBCGPTabInfo*) m_arTabs[iPrevHighlighted];
		ASSERT_VALID (pTab);

		pTab->m_bIsCloseHighlighted = pTab->m_bIsClosePressed = FALSE;

		switch (tabCloseButtonMode)
		{
		case TAB_CLOSE_BUTTON_ACTIVE:
		case TAB_CLOSE_BUTTON_HIGHLIGHTED_COMPACT:
		case TAB_CLOSE_BUTTON_HIGHLIGHTED:
			if (iPrevHighlighted != m_iActiveTab)
			{
				pTab->m_rectClose.SetRectEmpty();
				RedrawWindow(pTab->m_rect);
			}
			break;
		}
	}

	if (m_bNewTab && m_iHighlighted == m_iTabsNum - 1)
	{
		return;
	}

	CRect rectTT;
	rectTT.SetRectEmpty();

	if (m_iHighlighted >= 0 && (tabCloseButtonMode == TAB_CLOSE_BUTTON_HIGHLIGHTED_COMPACT || tabCloseButtonMode == TAB_CLOSE_BUTTON_HIGHLIGHTED))
	{
		CBCGPTabInfo* pTab = (CBCGPTabInfo*) m_arTabs[m_iHighlighted];
		ASSERT_VALID (pTab);

		SetupTabCloseButton(pTab, m_iHighlighted);
		RedrawWindow(pTab->m_rect);
	}
}
//**************************************************************************
void CBCGPTabWnd::SetupTabCloseButton(CBCGPTabInfo* pTab, int nIndex)
{
	ASSERT_VALID(pTab);

	pTab->m_rectClose = pTab->m_rect;

	if (!pTab->m_rectClose.IsRectEmpty())
	{
		if (pTab->m_rectClose.Width () < pTab->m_rectClose.Height() * 3 / 2)
		{
			pTab->m_rectClose.SetRectEmpty();
		}
		else
		{
			if (m_location == LOCATION_BOTTOM && m_bFlat)
			{
				pTab->m_rectClose.top--;
			}

			pTab->m_rectClose.left = pTab->m_rectClose.right - pTab->m_rectClose.Height ();

			const int nMargin = 3;
			const int nBoxSize = CBCGPMenuImages::Size().cy + 2 * nMargin;
			int nCloseTotalHeight = pTab->m_rect.Height() - 2;

			if (CBCGPVisualManager::GetInstance()->IsTabColorBar(this, nIndex))
			{
				int nColorBoxHeight = max(3, pTab->m_rectClose.Height() / 6);

				if (m_location == LOCATION_BOTTOM)
				{
					pTab->m_rectClose.bottom -= nColorBoxHeight;
				}
				else
				{
					pTab->m_rectClose.top += nColorBoxHeight;
				}

				nCloseTotalHeight = nBoxSize;
			}

			pTab->m_rectClose.DeflateRect(0, 1);
			pTab->m_rectClose.bottom = pTab->m_rectClose.top + nBoxSize;

			int nVertOffset = max(0, (nCloseTotalHeight - nBoxSize + 2) / 2);

			pTab->m_rectClose.OffsetRect (
				-CBCGPVisualManager::GetInstance ()->GetTabHorzMargin (this), nVertOffset);

			if (m_bFlat || m_bLeftRightRounded)
			{
				pTab->m_rectClose.OffsetRect (-m_nTabsHeight / 2, 0);
			}

			pTab->m_rectClose.left = pTab->m_rectClose.CenterPoint().x - nBoxSize / 2;
			pTab->m_rectClose.right = pTab->m_rectClose.left + nBoxSize;
		}

		if (pTab->m_rectClose.right > m_rectTabsArea.right)
		{
			pTab->m_rectClose.SetRectEmpty();
		}

		if (m_bNewTab && nIndex == m_iTabsNum - 1)
		{
			pTab->m_rectClose.SetRectEmpty();
		}
	}

	BOOL bShowToolTip = FALSE;

	if (m_pToolTip->GetSafeHwnd() != NULL)
	{
		CToolInfo info;
		if (m_pToolTip->GetToolInfo(info, this, pTab->m_iTabID))
		{
			CRect rectTT = info.rect;
			bShowToolTip = !rectTT.IsRectEmpty();
		}
	}

	AdjustTooltipRect(pTab, bShowToolTip);
}
//**************************************************************************
void CBCGPTabWnd::AdjustTooltipRect(CBCGPTabInfo* pTab, BOOL bShowTooltip)
{
	ASSERT_VALID(pTab);

	if (m_pToolTip->GetSafeHwnd() != NULL)
	{
		CRect rectTabTT = bShowTooltip ? pTab->m_rect : CRect(0, 0, 0, 0);
		if (bShowTooltip && !pTab->m_rectClose.IsRectEmpty())
		{
			rectTabTT.right = pTab->m_rectClose.left - 1;
		}

		if ((m_bIsOneNoteStyle || m_bIsVS2005Style || m_bLeftRightRounded) &&
			(m_TabCloseButtonMode == TAB_CLOSE_BUTTON_HIGHLIGHTED_COMPACT || m_TabCloseButtonMode == TAB_CLOSE_BUTTON_HIGHLIGHTED || m_TabCloseButtonMode == TAB_CLOSE_BUTTON_ALL))
		{
			rectTabTT.left += rectTabTT.Height();
		}

		m_pToolTip->SetToolRect (this, pTab->m_iTabID, rectTabTT);
	}

	if (m_pToolTipClose->GetSafeHwnd () != NULL)
	{
		m_pToolTipClose->SetToolRect (this, pTab->m_iTabID, pTab->m_rectClose);
	}
}
//**************************************************************************
void CBCGPTabWnd::SetMDIFocused(BOOL bIsFocused)
{
	if (m_bIsMDIFocused == bIsFocused)
	{
		return;
	}

	m_bIsMDIFocused = bIsFocused;

	if (GetSafeHwnd() != NULL)
	{
		RedrawWindow();
	}
}
//**************************************************************************
void CBCGPTabWnd::OnRTLChanged(BOOL bIsRTL)
{
	CBCGPBaseTabWnd::OnRTLChanged(bIsRTL);
	SetScrollButtons();
}
//**************************************************************************
int CBCGPTabWnd::GetPointerAreaHeight() const
{
	int nTextMargin = 0;
	return m_bIsPointerStyle ? ((CBCGPTabWnd*)this)->GetTextHeight(nTextMargin) : 0;
}
//*****************************************************************************
CFont* CBCGPTabWnd::GetTabFont()
{
	ASSERT_VALID(this);

	if (m_hFontCustom != NULL && ::GetObjectType(m_hFontCustom) != OBJ_FONT)
	{
		m_hFontCustom = NULL;
	}

	return m_hFontCustom == NULL ? (m_bFlat ?	&m_fntTabs : (IsCaptionFont() ? &globalData.fontCaption : &globalData.fontRegular)) : CFont::FromHandle(m_hFontCustom);
}
//**************************************************************************************
void CBCGPTabWnd::OnTimer(UINT_PTR nIDEvent) 
{
	switch (nIDEvent)
	{
	case idTabAnimation:
		m_nTabAnimationOffset += m_nTabAnimationStep;

		if ((m_nTabAnimationStep > 0 && m_nTabAnimationOffset >= 0) || (m_nTabAnimationStep < 0 && m_nTabAnimationOffset <= 0))
		{
			KillTimer(nIDEvent);
			m_nTabAnimationOffset = m_nTabAnimationStep = 0;
		}
		else
		{
			if (m_nTabAnimationStep > 1)
			{
				m_nTabAnimationStep--;
			}
			else if (m_nTabAnimationStep < -1)
			{
				m_nTabAnimationStep++;
			}
		}

		CRect rectClient;
		GetClientRect(rectClient);

		CRect rectTabs = rectClient;
		
		if (m_location == LOCATION_BOTTOM)
		{
			rectTabs.top = m_rectTabsArea.top - GetPointerAreaHeight();
		}
		else
		{
			rectTabs.bottom = m_rectTabsArea.bottom + GetPointerAreaHeight();
		}

		RedrawWindow(rectTabs);
		return;
	}

	CBCGPBaseTabWnd::OnTimer(nIDEvent);
}
//**************************************************************************************
void CBCGPTabWnd::EnableTabAnimation(BOOL bEnable)
{
	m_bTabAnimationSupport = bEnable;
}
//**************************************************************************************
LRESULT CBCGPTabWnd::OnPrintClient(WPARAM wp, LPARAM lp)
{
	if ((lp & PRF_CLIENT) == PRF_CLIENT)
	{
		CDC* pDC = CDC::FromHandle((HDC) wp);
		ASSERT_VALID(pDC);

		CRect rectClient;
		GetClientRect(rectClient);

		if (rectClient.Width() > 0 && rectClient.Height() > 0)
		{
			CBCGPMemDC memDC(*pDC, rectClient);

			OnDraw(&memDC.GetDC());
			pDC->ExcludeClipRect (m_rectWndArea);
		}
	}

	return 0;
}
