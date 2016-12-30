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
// BCGPRibbonStatusBar.cpp : implementation file
//

#include "stdafx.h"
#include "bcgcbpro.h"
#include "BCGPContextMenuManager.h"
#include "BCGPRibbonStatusBar.h"
#include "BCGPFrameWnd.h"
#include "BCGPMDIFrameWnd.h"
#include "BCGPRibbonStatusBarPane.h"
#include "BCGPRibbonPanelMenu.h"
#include "BCGPRibbonLabel.h"
#include "BCGPToolbarMenuButton.h"
#include "RegPath.h"
#include "BCGPRegistry.h"
#include "BCGPLocalResource.h"
#include "bcgprores.h"
#include "BCGPRibbonInfoLoader.h"
#include "BCGPRibbonConstructor.h"

#ifndef BCGP_EXCLUDE_RIBBON

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static const CString strRibbonProfile	= _T("BCGPRibbons");

#define UM_UPDATE_SHADOWS				(WM_USER + 101)

#define REG_SECTION_FMT					_T("%sBCGRibbonBar-%d")
#define REG_SECTION_FMT_EX				_T("%sBCGRibbonBar-%d%x")
#define REG_ENTRY_STATUSBAR_PANES		_T("StatusBarPanes")

static const int nMaxValueLen = 50;

/////////////////////////////////////////////////////////////////////////////
// CBCGPRibbonStatusBarCustomizeButton

class CBCGPRibbonStatusBarCustomizeButton : public CBCGPRibbonButton
{
	DECLARE_DYNCREATE(CBCGPRibbonStatusBarCustomizeButton)

public:
	CBCGPRibbonStatusBarCustomizeButton()	{}

	CBCGPRibbonStatusBarCustomizeButton (LPCTSTR lpszLabel) :
		CBCGPRibbonButton (0, lpszLabel)
	{
	}

	virtual CSize GetIntermediateSize (CDC* pDC)
	{
		ASSERT_VALID (pDC);

		CBCGPBaseRibbonElement* pElement = (CBCGPBaseRibbonElement*) m_dwData;
		ASSERT_VALID (pElement);

		CSize size = CBCGPRibbonButton::GetIntermediateSize (pDC);

		size.cx += size.cy * 2;	// Reserve space for checkbox

		CString strValue = pElement->GetText ();

		if (strValue.GetLength () > nMaxValueLen)
		{
			strValue = strValue.Left (nMaxValueLen - 1);
		}

		if (!strValue.IsEmpty ())
		{
			size.cx += pDC->GetTextExtent (strValue).cx + 4 * m_szMargin.cx;
		}

		return size;
	}

	virtual void OnDraw (CDC* pDC)
	{
		ASSERT_VALID (pDC);

		CBCGPBaseRibbonElement* pElement = (CBCGPBaseRibbonElement*) m_dwData;
		ASSERT_VALID (pElement);

		CBCGPToolbarMenuButton dummy;

		dummy.m_strText = m_strText;

		CString strValue = pElement->GetText ();

		if (strValue.GetLength () > nMaxValueLen)
		{
			strValue = strValue.Left (nMaxValueLen - 1);
		}

		if (!strValue.IsEmpty ())
		{
			dummy.m_strText += _T('\t');
			dummy.m_strText += strValue;
		}

		dummy.m_bMenuMode = TRUE;
		dummy.m_pWndParent = GetParentWnd ();

		if (pElement->IsVisible ())
		{
			dummy.m_nStyle |= TBBS_CHECKED;
		}

		dummy.OnDraw (pDC, m_rect, NULL, TRUE, FALSE, m_bIsHighlighted);
	}

	virtual void OnClick (CPoint /*point*/)
	{
		CBCGPBaseRibbonElement* pElement = (CBCGPBaseRibbonElement*) m_dwData;
		ASSERT_VALID (pElement);

		pElement->SetVisible (!pElement->IsVisible ());
		Redraw ();

		CBCGPRibbonBar* pRibbonStatusBar = pElement->GetParentRibbonBar ();
		ASSERT_VALID (pRibbonStatusBar);

		pRibbonStatusBar->RecalcLayout ();
		pRibbonStatusBar->RedrawWindow ();

		CFrameWnd* pParentFrame = pRibbonStatusBar->GetParentFrame ();
		ASSERT_VALID (pParentFrame);

		pParentFrame->RedrawWindow (NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW);

		CRect rectScreen;
		pRibbonStatusBar->GetWindowRect (&rectScreen);

		CBCGPPopupMenu::UpdateAllShadows (rectScreen);
	}
};

IMPLEMENT_DYNCREATE(CBCGPRibbonStatusBarCustomizeButton, CBCGPRibbonButton)

const int xExtAreaMargin = 5;

/////////////////////////////////////////////////////////////////////////////
// CBCGPRibbonStatusBar

IMPLEMENT_DYNAMIC(CBCGPRibbonStatusBar, CBCGPRibbonBar)

CBCGPRibbonStatusBar::CBCGPRibbonStatusBar()
{
	m_cxSizeBox = 0;
	m_cxFree = -1;
	m_rectSizeBox.SetRectEmpty ();
	m_rectResizeBottom.SetRectEmpty ();
	m_bBottomFrame = FALSE;
	m_rectInfo.SetRectEmpty ();
	m_bShowEmptyExtArea = FALSE;
	m_bTemporaryHidden = FALSE;
}

CBCGPRibbonStatusBar::~CBCGPRibbonStatusBar()
{
	RemoveAll ();
}


BEGIN_MESSAGE_MAP(CBCGPRibbonStatusBar, CBCGPRibbonBar)
	//{{AFX_MSG_MAP(CBCGPRibbonStatusBar)
	ON_WM_SIZE()
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP
	ON_WM_NCHITTEST()
	ON_MESSAGE(UM_UPDATE_SHADOWS, OnUpdateShadows)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CBCGPRibbonStatusBar message handlers

BOOL CBCGPRibbonStatusBar::PreCreateWindow(CREATESTRUCT& cs) 
{
	if ((m_dwStyle & (CBRS_ALIGN_ANY|CBRS_BORDER_ANY)) == CBRS_BOTTOM)
	{
		m_dwStyle &= ~(CBRS_BORDER_ANY|CBRS_BORDER_3D);
	}

	return CBCGPRibbonBar::PreCreateWindow(cs);
}
//********************************************************************************
BOOL CBCGPRibbonStatusBar::Create(CWnd* pParentWnd, DWORD dwStyle, UINT nID)
{
	return CreateEx(pParentWnd, 0, dwStyle, nID);
}
//********************************************************************************
BOOL CBCGPRibbonStatusBar::CreateEx (CWnd* pParentWnd, DWORD /*dwCtrlStyle*/, DWORD dwStyle, 
							 UINT nID)
{
	ASSERT_VALID(pParentWnd);   // must have a parent

	// save the style
	SetBarAlignment (dwStyle & CBRS_ALL);

	// create the HWND
	CRect rect;
	rect.SetRectEmpty();

	m_dwBCGStyle = 0; // can't float, resize, close, slide

	if (pParentWnd->GetStyle() & WS_THICKFRAME)
	{
		dwStyle |= SBARS_SIZEGRIP;
	}

	if (!CWnd::Create (globalData.RegisterWindowClass (_T("BCGPRibbonStatusBar")),
		NULL, dwStyle | WS_CLIPSIBLINGS, rect, pParentWnd, nID))
	{
		return FALSE;
	}

	if (pParentWnd->IsKindOf (RUNTIME_CLASS (CBCGPFrameWnd)))
	{
		((CBCGPFrameWnd*) pParentWnd)->AddControlBar (this);
	}
	else if (pParentWnd->IsKindOf (RUNTIME_CLASS (CBCGPMDIFrameWnd)))
	{
		((CBCGPMDIFrameWnd*) pParentWnd)->AddControlBar (this);
	}
	else
	{
		ASSERT (FALSE);
		return FALSE;
	}
	return TRUE;
}
//******************************************************************************************
BOOL CBCGPRibbonStatusBar::LoadFromXML (LPCTSTR lpszXMLResID)
{
	ASSERT_VALID (this);

	CBCGPRibbonInfo info;
	CBCGPRibbonInfoLoader loader (info, CBCGPRibbonInfo::e_UseStatus);

	if (!loader.Load (lpszXMLResID))
	{
		TRACE0("Cannot load status from buffer\n");
		return FALSE;
	}

	CBCGPRibbonConstructor constr (info);
	constr.ConstructStatusBar (*this);

	return TRUE;
}
//******************************************************************************************
BOOL CBCGPRibbonStatusBar::LoadFromBuffer (LPCTSTR lpszXMLBuffer)
{
	ASSERT_VALID (this);
	ASSERT (lpszXMLBuffer != NULL);

	CBCGPRibbonInfo info;
	CBCGPRibbonInfoLoader loader (info, CBCGPRibbonInfo::e_UseStatus);

	if (!loader.LoadFromBuffer (lpszXMLBuffer))
	{
		TRACE0("Cannot load ribbon from buffer\n");
		return FALSE;
	}

	CBCGPRibbonConstructor constr (info);
	constr.ConstructStatusBar (*this);

	return TRUE;
}
//********************************************************************************
CSize CBCGPRibbonStatusBar::CalcFixedLayout(BOOL, BOOL /*bHorz*/)
{
	ASSERT_VALID(this);

	CClientDC dc (this);

	CFont* pOldFont = dc.SelectObject (GetFont ());
	ASSERT (pOldFont != NULL);

	TEXTMETRIC tm;
	dc.GetTextMetrics (&tm);

	int i = 0;
	int cyMax = tm.tmHeight;

	for (i = 0; i < m_arExElements.GetSize (); i++)
	{
		CBCGPBaseRibbonElement* pElem = m_arExElements [i];
		ASSERT_VALID (pElem);

		pElem->OnCalcTextSize (&dc);
		pElem->SetInitialMode ();

		CSize sizeElem = pElem->GetSize (&dc);
		cyMax = max (cyMax, sizeElem.cy + 1);
	}

	for (i = 0; i < m_arElements.GetSize (); i++)
	{
		CBCGPBaseRibbonElement* pElem = m_arElements [i];
		ASSERT_VALID (pElem);

		pElem->OnCalcTextSize (&dc);
		pElem->SetInitialMode ();

		CSize sizeElem = pElem->GetSize (&dc);
		cyMax = max (cyMax, sizeElem.cy + 1);
	}

	dc.SelectObject (pOldFont);

	int nMinHeight = 24;

	if (globalData.GetRibbonImageScale () != 1.)
	{
		nMinHeight = (int) (.5 + globalData.GetRibbonImageScale () * nMinHeight);
	}

	return CSize (32767, max (nMinHeight, cyMax));
}
//********************************************************************************
void CBCGPRibbonStatusBar::AddElement (CBCGPBaseRibbonElement* pElement, 
									   LPCTSTR lpszLabel, BOOL bIsVisible)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pElement);
	ASSERT (lpszLabel != NULL);

	pElement->SetParentRibbonBar (this);
	pElement->m_bIsVisible = bIsVisible;

	m_arElements.Add (pElement);
	m_arElementLabels.Add (lpszLabel);

	CleanUpCustomizeItems ();
}
//********************************************************************************
void CBCGPRibbonStatusBar::AddExtendedElement (CBCGPBaseRibbonElement* pElement,
											   LPCTSTR lpszLabel, BOOL bIsVisible)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pElement);
	ASSERT (lpszLabel != NULL);

	pElement->SetParentRibbonBar (this);
	pElement->m_bIsVisible = bIsVisible;

	CBCGPRibbonStatusBarPane* pPane = DYNAMIC_DOWNCAST (
		CBCGPRibbonStatusBarPane, pElement);

	if (pPane != NULL)
	{
		ASSERT_VALID (pPane);
		pPane->m_bIsExtended = TRUE;
	}

	m_arExElements.Add (pElement);
	m_arExElementLabels.Add (lpszLabel);

	CleanUpCustomizeItems ();
}
//********************************************************************************
void CBCGPRibbonStatusBar::AddSeparator ()
{
	ASSERT_VALID (this);

	CBCGPRibbonSeparator* pSeparator = new CBCGPRibbonSeparator;
	pSeparator->SetParentRibbonBar (this);

	m_arElements.Add (pSeparator);
	m_arElementLabels.Add (_T(""));

	CleanUpCustomizeItems ();
}
//********************************************************************************
void CBCGPRibbonStatusBar::AddDynamicElement (CBCGPBaseRibbonElement* pElement)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pElement);

	pElement->SetParentRibbonBar (this);
	pElement->m_bIsVisible = TRUE;

	m_arElements.Add (pElement);
	m_arElementLabels.Add (_T(""));

	m_lstDynElements.AddTail (pElement);
}
//********************************************************************************
BOOL CBCGPRibbonStatusBar::ReplaceElementByID(UINT uiCmdID, CBCGPBaseRibbonElement& newElement, LPCTSTR lpszLabel, BOOL bCopyContent)
{
	ASSERT_VALID (this);

	if (uiCmdID == 0 || uiCmdID == (UINT)-1)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	CBCGPBaseRibbonElement* pNewElement = DYNAMIC_DOWNCAST(CBCGPBaseRibbonElement, newElement.GetRuntimeClass()->CreateObject());
	if (pNewElement == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	ASSERT_VALID(pNewElement);

	pNewElement->CopyFrom(newElement);
	pNewElement->SetParentRibbonBar (this);

	CBCGPBaseRibbonElement* pOldElement = NULL;

	int i = 0;

	for (i = 0; i < m_arElements.GetSize () && pOldElement == NULL; i++)
	{
		CBCGPBaseRibbonElement* pElem = m_arElements [i];
		ASSERT_VALID (pElem);

		if (pElem->GetID () == uiCmdID)
		{
			pOldElement = pElem;

			POSITION pos = m_lstDynElements.Find (pElem);
			if (pos != NULL)
			{
				m_lstDynElements.InsertAfter(pos, pNewElement);
				m_lstDynElements.RemoveAt (pos);
			}

			m_arElements[i] = pNewElement;

			if (lpszLabel != NULL)
			{
				m_arElementLabels[i] = lpszLabel;
			}
		}
	}

	if (pOldElement == NULL)
	{
		for (i = 0; i < m_arExElements.GetSize () && pOldElement == NULL; i++)
		{
			CBCGPBaseRibbonElement* pElem = m_arExElements [i];
			ASSERT_VALID (pElem);

			if (pElem->GetID () == uiCmdID)
			{
				pOldElement = pElem;

				m_arExElements[i] = pNewElement;

				if (lpszLabel != NULL)
				{
					m_arExElementLabels[i] = lpszLabel;
				}
			}
		}
	}

	if (pOldElement != NULL)
	{
		if (pOldElement == m_pHighlighted)
		{
			m_pHighlighted = NULL;
		}

		if (pOldElement == m_pPressed)
		{
			m_pPressed = NULL;
		}

		if (bCopyContent)
		{
			pNewElement->CopyFrom(*pOldElement);
		}
		else
		{
			pNewElement->CopyBaseFrom(*pOldElement);
		}

		delete pOldElement;

		CleanUpCustomizeItems();
		return TRUE;
	}

	delete pNewElement;
	return FALSE;
}
//********************************************************************************
BOOL CBCGPRibbonStatusBar::RemoveElement (UINT uiID)
{
	ASSERT_VALID (this);

	int i = 0;

	for (i = 0; i < m_arElements.GetSize (); i++)
	{
		CBCGPBaseRibbonElement* pElem = m_arElements [i];
		ASSERT_VALID (pElem);

		if (pElem->GetID () == uiID)
		{
			POSITION pos = m_lstDynElements.Find (pElem);
			if (pos != NULL)
			{
				// Element is dynamic: remove it from dynamic elements list
				m_lstDynElements.RemoveAt (pos);
			}

			if (pElem == m_pHighlighted)
			{
				m_pHighlighted = NULL;
			}

			if (pElem == m_pPressed)
			{
				m_pPressed = NULL;
			}

			delete pElem;
			m_arElements.RemoveAt (i);
			m_arElementLabels.RemoveAt (i);

			return TRUE;
		}
	}

	for (i = 0; i < m_arExElements.GetSize (); i++)
	{
		CBCGPBaseRibbonElement* pElem = m_arExElements [i];
		ASSERT_VALID (pElem);

		if (pElem->GetID () == uiID)
		{
			if (pElem == m_pHighlighted)
			{
				m_pHighlighted = NULL;
			}

			if (pElem == m_pPressed)
			{
				m_pPressed = NULL;
			}

			delete pElem;
			m_arExElements.RemoveAt (i);
			m_arExElementLabels.RemoveAt (i);

			return TRUE;
		}
	}

	return FALSE;
}
//********************************************************************************
void CBCGPRibbonStatusBar::RemoveAll ()
{
	ASSERT_VALID (this);

	int i = 0;

	for (i = 0; i < m_arElements.GetSize (); i++)
	{
		delete m_arElements [i];
	}

	m_arElements.RemoveAll ();

	for (i = 0; i < m_arExElements.GetSize (); i++)
	{
		delete m_arExElements [i];
	}

	m_arExElements.RemoveAll ();
	
	m_arElementLabels.RemoveAll ();
	m_arExElementLabels.RemoveAll ();

	CleanUpCustomizeItems ();
}
//********************************************************************************
int CBCGPRibbonStatusBar::GetCount () const
{
	ASSERT_VALID (this);
	return (int) m_arElements.GetSize ();
}
//********************************************************************************
int CBCGPRibbonStatusBar::GetExCount () const
{
	ASSERT_VALID (this);
	return (int) m_arExElements.GetSize ();
}
//********************************************************************************
CBCGPBaseRibbonElement* CBCGPRibbonStatusBar::GetElement (int nIndex)
{
	ASSERT_VALID (this);

	if (nIndex < 0 || nIndex >= (int) m_arElements.GetSize ())
	{
		ASSERT (FALSE);
		return NULL;
	}

	return m_arElements [nIndex];
}
//********************************************************************************
CBCGPBaseRibbonElement* CBCGPRibbonStatusBar::GetExElement (int nIndex)
{
	ASSERT_VALID (this);

	if (nIndex < 0 || nIndex >= (int) m_arExElements.GetSize ())
	{
		ASSERT (FALSE);
		return NULL;
	}

	return m_arExElements [nIndex];
}
//********************************************************************************
CBCGPBaseRibbonElement* CBCGPRibbonStatusBar::FindElement (UINT uiID)
{
	ASSERT_VALID (this);

	int i = 0;

	for (i = 0; i < m_arElements.GetSize (); i++)
	{
		ASSERT_VALID (m_arElements [i]);

		if (m_arElements [i]->GetID () == uiID)
		{
			return m_arElements [i];
		}
	}

	for (i = 0; i < m_arExElements.GetSize (); i++)
	{
		ASSERT_VALID (m_arExElements [i]);

		if (m_arExElements [i]->GetID () == uiID)
		{
			return m_arExElements [i];
		}
	}

	return NULL;
}
//********************************************************************************
BOOL CBCGPRibbonStatusBar::GetExtendedArea (CRect& rect) const
{
	ASSERT_VALID (this);

	CRect rectClient;
	GetClientRect (rectClient);

	for (int i = 0; i < m_arExElements.GetSize (); i++)
	{
		CBCGPBaseRibbonElement* pElem = m_arExElements [i];
		ASSERT_VALID (pElem);

		CRect rectElem = pElem->GetRect ();
		
		if (!rectElem.IsRectEmpty ())
		{
			rect = rectClient;
			rect.left = rectElem.left - xExtAreaMargin;

			return TRUE;
		}
	}

	if (m_bShowEmptyExtArea && (m_rectSizeBox.Width () > 0))
	{
		rect = rectClient;
		rect.left = rect.right - m_rectSizeBox.Width () - xExtAreaMargin;

		return TRUE;
	}

	return FALSE;
}
//********************************************************************************
void CBCGPRibbonStatusBar::OnSize(UINT nType, int cx, int cy) 
{
	CBCGPRibbonBar::OnSize(nType, cx, cy);

	RecalcLayout ();
	RedrawWindow ();
}
//********************************************************************************
BCGNcHitTestType CBCGPRibbonStatusBar::OnNcHitTest(CPoint point)
{
	BOOL bRTL = GetExStyle() & WS_EX_LAYOUTRTL;

	// hit test the size box - convert to HTCAPTION if so
	if (m_pPressed == NULL && !m_rectSizeBox.IsRectEmpty ())
	{
		CRect rect = m_rectSizeBox;
		ClientToScreen(&rect);

		if (rect.PtInRect (point))
		{
			OnCancelMode ();
			return bRTL ? HTBOTTOMLEFT : HTBOTTOMRIGHT;
		}

		rect = m_rectResizeBottom;
		ClientToScreen (&rect);

		if (rect.PtInRect (point))
		{
			OnCancelMode ();
			return HTBOTTOM;
		}
	}

	return CBCGPRibbonBar::OnNcHitTest(point);
}
//********************************************************************************
void CBCGPRibbonStatusBar::OnSysCommand(UINT nID, LPARAM lParam) 
{
	if (m_cxSizeBox != 0 && (nID & 0xFFF0) == SC_SIZE)
	{
		CFrameWnd* pFrameWnd = BCGPGetParentFrame(this);
		if (pFrameWnd != NULL)
		{
			pFrameWnd->SendMessage(WM_SYSCOMMAND, (WPARAM)nID, lParam);
			return;
		}
	}

	CBCGPControlBar::OnSysCommand(nID, lParam);
}
//**********************************************************************************
void CBCGPRibbonStatusBar::RecalcLayout ()
{
	ASSERT_VALID (this);
	ASSERT (GetSafeHwnd () != NULL);

	// get the drawing area for the status bar
	CRect rect;
	GetClientRect(rect);

	// the size box is based off the size of a scrollbar
	m_cxSizeBox = min(GetSystemMetrics(SM_CXVSCROLL)+1, rect.Height());
	
	CFrameWnd* pFrameWnd = BCGPGetParentFrame(this);
	if (pFrameWnd->GetSafeHwnd() != NULL)
	{
		if (pFrameWnd->IsZoomed())
		{
			m_cxSizeBox = 0;
		}
		
		if ((pFrameWnd->GetStyle() & WS_THICKFRAME) == 0)
		{
			m_cxSizeBox = 0;
		}
	}

	if ((GetStyle() & SBARS_SIZEGRIP) == 0)
	{
		m_cxSizeBox = 0;
	}

	CClientDC dc (this);

	CFont* pOldFont = dc.SelectObject (GetFont ());
	ASSERT (pOldFont != NULL);

	int xMax = (rect.right -= m_cxSizeBox);

	m_rectResizeBottom.SetRectEmpty ();

	if (m_cxSizeBox != 0)
	{
		m_rectSizeBox = rect;
		m_rectSizeBox.left = rect.right;
		m_rectSizeBox.right = m_rectSizeBox.left + min(m_cxSizeBox, rect.Height() + m_cyTopBorder);

		if (m_bBottomFrame)
		{
			m_rectSizeBox.OffsetRect (0, -2);

			m_rectResizeBottom = rect;
			m_rectResizeBottom.top = m_rectResizeBottom.bottom - GetSystemMetrics (SM_CYSIZEFRAME);
		}
	}
	else
	{
		m_rectSizeBox.SetRectEmpty ();
	}

	int i = 0;

	rect.DeflateRect (0, 2);

	//---------------------------------
	// Repos extended (right) elements:
	//---------------------------------
	for (i = (int) m_arExElements.GetSize () - 1; i >= 0; i--)
	{
		CBCGPBaseRibbonElement* pElem = m_arExElements [i];
		ASSERT_VALID (pElem);

		pElem->OnCalcTextSize (&dc);

		CSize sizeElem = pElem->GetSize (&dc);

		if (xMax - sizeElem.cx < rect.left || !pElem->m_bIsVisible)
		{
			pElem->SetRect (CRect (0, 0, 0, 0));
		}
		else
		{
			if (pElem->CanBeStretched ())
			{
				pElem->SetRect (CRect (xMax - sizeElem.cx, rect.top, xMax, rect.bottom));
			}
			else
			{
				int yOffset = max (0, (rect.Height () - sizeElem.cy) / 2);

				pElem->SetRect (CRect (CPoint (xMax - sizeElem.cx, rect.top + yOffset), sizeElem));
			}

			xMax = pElem->GetRect ().left;
		}

		pElem->OnAfterChangeRect (&dc);
	}

	xMax -= 2 * xExtAreaMargin;

	//----------------------------
	// Repos main (left) elements:
	//----------------------------
	int x = rect.left;

	if (IsInformationMode ())
	{
		m_rectInfo = rect;
		m_rectInfo.right = xMax;

		for (i = (int) m_arElements.GetSize () - 1; i >= 0; i--)
		{
			CBCGPBaseRibbonElement* pElem = m_arElements [i];
			ASSERT_VALID (pElem);

			pElem->SetRect (CRect (0, 0, 0, 0));
		}
	}
	else
	{
		m_rectInfo.SetRectEmpty ();

		m_cxFree = xMax - rect.left;

		BOOL bIsPrevSeparator = TRUE;
		CBCGPBaseRibbonElement* pLastVisibleElem = NULL;

		for (i = 0; i < (int) m_arElements.GetSize (); i++)
		{
			CBCGPBaseRibbonElement* pElem = m_arElements [i];
			ASSERT_VALID (pElem);

			BOOL bIsSeparator = pElem->IsKindOf (RUNTIME_CLASS (CBCGPRibbonSeparator));

			if (bIsSeparator && bIsPrevSeparator)
			{
				pElem->SetRect (CRect (0, 0, 0, 0));
				continue;
			}

			pElem->OnCalcTextSize (&dc);

			CSize sizeElem = pElem->GetSize (&dc);

			if (x + sizeElem.cx > xMax || !pElem->m_bIsVisible)
			{
				pElem->SetRect (CRect (0, 0, 0, 0));
			}
			else
			{
				if (pElem->CanBeStretched ())
				{
					pElem->SetRect (CRect (x, rect.top, x + sizeElem.cx, rect.bottom));
				}
				else
				{
					sizeElem.cy	= min (sizeElem.cy, rect.Height ());
					int yOffset = max (0, (rect.Height () - sizeElem.cy) / 2);

					pElem->SetRect (CRect (CPoint (x, rect.top + yOffset), sizeElem));
				}

				x += sizeElem.cx;

				m_cxFree = xMax - x;
				bIsPrevSeparator = bIsSeparator;

				pLastVisibleElem = pElem;
			}

			pElem->OnAfterChangeRect (&dc);
		}

		if (pLastVisibleElem != NULL)
		{
			ASSERT_VALID (pLastVisibleElem);

			if (pLastVisibleElem->IsKindOf (RUNTIME_CLASS (CBCGPRibbonSeparator)))
			{
				// Last visible element is separator - hide it:
				pLastVisibleElem->SetRect (CRect (0, 0, 0, 0));
				pLastVisibleElem->OnAfterChangeRect (&dc);
			}
		}
	}

	dc.SelectObject (pOldFont);
}
//*****************************************************************************************************
void CBCGPRibbonStatusBar::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	CBCGPMemDC memDC (dc, this);
	CDC* pDC = &memDC.GetDC ();

	CRect rectClip;
	dc.GetClipBox (rectClip);

	CRgn rgnClip;

	if (!rectClip.IsRectEmpty ())
	{
		rgnClip.CreateRectRgnIndirect (rectClip);
		pDC->SelectClipRgn (&rgnClip);
	}

	OnDraw(pDC);

	pDC->SelectClipRgn (NULL);
}
//*****************************************************************************************************
void CBCGPRibbonStatusBar::OnDraw(CDC* pDC)
{
	pDC->SetBkMode (TRANSPARENT);
	
	CRect rectClient;
	GetClientRect (rectClient);

	OnFillBackground (pDC, rectClient);

	// draw the size box in the bottom right corner
	if (!m_rectSizeBox.IsRectEmpty ())
	{
		CRect rectSizeBox = m_rectSizeBox;

		if (m_bBottomFrame)
		{
			rectSizeBox.OffsetRect (-2, -2);
		}

		CBCGPVisualManager::GetInstance ()->OnDrawStatusBarSizeBox (pDC, NULL, rectSizeBox);
	}

	CFont* pOldFont = pDC->SelectObject (GetFont ());
	ASSERT (pOldFont != NULL);

	int i = 0;

	if (IsInformationMode ())
	{
		OnDrawInformation (pDC, m_strInfo, m_rectInfo);
	}
	else
	{
		for (i = 0; i < (int) m_arElements.GetSize (); i++)
		{
			ASSERT_VALID (m_arElements [i]);
			m_arElements [i]->OnDraw (pDC);
		}
	}

	for (i = 0; i < (int) m_arExElements.GetSize (); i++)
	{
		ASSERT_VALID (m_arExElements [i]);
		m_arExElements [i]->OnDraw (pDC);
	}

	pDC->SelectObject (pOldFont);
}
//******************************************************************************
CBCGPBaseRibbonElement* CBCGPRibbonStatusBar::HitTest (CPoint point, 
												 BOOL /*bCheckActiveCategory*/,
												 BOOL /*bCheckPanelCaption*/)
{
	ASSERT_VALID (this);

	int i = 0;

	for (i = 0; i < (int) m_arElements.GetSize (); i++)
	{
		ASSERT_VALID (m_arElements [i]);

		if (m_arElements [i]->GetRect ().PtInRect (point))
		{
			return m_arElements [i]->HitTest (point);
		}
	}

	for (i = 0; i < (int) m_arExElements.GetSize (); i++)
	{
		ASSERT_VALID (m_arExElements [i]);

		if (m_arExElements [i]->GetRect ().PtInRect (point))
		{
			return m_arExElements [i]->HitTest (point);
		}
	}

	return NULL;
}
//*******************************************************************************
void CBCGPRibbonStatusBar::OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler)
{
	ASSERT_VALID (this);

	CBCGPRibbonCmdUI state;
	state.m_pOther = this;

	int i = 0;

	for (i = 0; i < (int) m_arElements.GetSize (); i++)
	{
		ASSERT_VALID (m_arElements [i]);
		m_arElements [i]->OnUpdateCmdUI (&state, pTarget, bDisableIfNoHndler);
	}

	for (i = 0; i < (int) m_arExElements.GetSize (); i++)
	{
		ASSERT_VALID (m_arExElements [i]);
		m_arExElements [i]->OnUpdateCmdUI (&state, pTarget, bDisableIfNoHndler);
	}

	// update the dialog controls added to the ribbon
	UpdateDialogControls(pTarget, bDisableIfNoHndler);
}
//*******************************************************************************
CBCGPBaseRibbonElement* CBCGPRibbonStatusBar::GetDroppedDown ()
{
	ASSERT_VALID (this);

	int i = 0;

	for (i = 0; i < (int) m_arElements.GetSize (); i++)
	{
		ASSERT_VALID (m_arElements [i]);

		if (m_arElements [i]->GetDroppedDown () != NULL)
		{
			return m_arElements [i];
		}
	}

	for (i = 0; i < (int) m_arExElements.GetSize (); i++)
	{
		ASSERT_VALID (m_arExElements [i]);

		if (m_arExElements [i]->GetDroppedDown () != NULL)
		{
			return m_arExElements [i];
		}
	}

	return NULL;
}
//*****************************************************************************
void CBCGPRibbonStatusBar::OnControlBarContextMenu (CWnd* /*pParentFrame*/, CPoint point)
{
	if ((GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0)	// Left mouse button is pressed
	{
		return;
	}

	if (m_arCustomizeItems.GetSize () == 0)
	{
		CString strCaption;

		{
			CBCGPLocalResource locaRes;
			strCaption.LoadString (IDS_BCGBARRES_STATBAR_CUSTOMIZE);
		}

		m_arCustomizeItems.Add (new CBCGPRibbonLabel (strCaption));

		int i = 0;

		for (i = 0; i < (int) m_arElements.GetSize (); i++)
		{
			CBCGPBaseRibbonElement* pElem = m_arElements [i];
			ASSERT_VALID (pElem);

			if (m_lstDynElements.Find (pElem) != NULL)
			{
				// Dynamic element, don't add it to customization menu
				continue;
			}

			if (pElem->IsKindOf (RUNTIME_CLASS (CBCGPRibbonSeparator)))
			{
				CBCGPRibbonSeparator* pSeparator = new CBCGPRibbonSeparator (TRUE);
				pSeparator->SetDefaultMenuLook ();

				m_arCustomizeItems.Add (pSeparator);
			}
			else
			{
				CBCGPRibbonStatusBarCustomizeButton* pItem = new 
					CBCGPRibbonStatusBarCustomizeButton (
						m_arElementLabels [i]);

				pItem->SetData ((DWORD_PTR) pElem);
				pItem->SetDefaultMenuLook ();

				m_arCustomizeItems.Add (pItem);
			}
		}

		if ((int) m_arCustomizeItems.GetSize () > 1 && m_arExElements.GetSize () > 0)
		{
			CBCGPRibbonSeparator* pSeparator = new CBCGPRibbonSeparator (TRUE);
			pSeparator->SetDefaultMenuLook ();

			m_arCustomizeItems.Add (pSeparator);
		}

		for (i = 0; i < (int) m_arExElements.GetSize (); i++)
		{
			CBCGPBaseRibbonElement* pElem = m_arExElements [i];
			ASSERT_VALID (pElem);

			if (pElem->IsKindOf (RUNTIME_CLASS (CBCGPRibbonSeparator)))
			{
				CBCGPRibbonSeparator* pSeparator = new CBCGPRibbonSeparator (TRUE);
				pSeparator->SetDefaultMenuLook ();

				m_arCustomizeItems.Add (pSeparator);
			}
			else
			{
				CBCGPRibbonStatusBarCustomizeButton* pItem = new 
					CBCGPRibbonStatusBarCustomizeButton (
						m_arExElementLabels [i]);

				pItem->SetData ((DWORD_PTR) pElem);
				m_arCustomizeItems.Add (pItem);
			}
		}
	}
	
	CBCGPRibbonPanelMenu* pMenu = new CBCGPRibbonPanelMenu (this, m_arCustomizeItems);
	pMenu->SetMenuMode ();
	pMenu->SetDefaultMenuLook ();
	pMenu->EnableCustomizeMenu (FALSE);
	
	pMenu->Create (this, point.x, point.y, (HMENU) NULL);
}
//*****************************************************************************
void CBCGPRibbonStatusBar::CleanUpCustomizeItems ()
{
	for (int i = 0; i < (int) m_arCustomizeItems.GetSize (); i++)
	{
		ASSERT_VALID (m_arCustomizeItems [i]);
		delete m_arCustomizeItems [i];
	}

	m_arCustomizeItems.RemoveAll ();
}
//*******************************************************************************************
BOOL CBCGPRibbonStatusBar::SaveState (LPCTSTR lpszProfileName, int nIndex, UINT uiID)
{
	CString strProfileName = ::BCGPGetRegPath (strRibbonProfile, lpszProfileName);

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

	CBCGPRegistrySP regSP;
	CBCGPRegistry& reg = regSP.Create (FALSE, FALSE);

	if (reg.CreateKey (strSection))
	{
		CList<UINT,UINT> lstInvisiblePanes;

		int i = 0;

		for (i = 0; i < (int) m_arElements.GetSize (); i++)
		{
			CBCGPBaseRibbonElement* pElem = m_arElements [i];
			ASSERT_VALID (pElem);

			if (!pElem->m_bIsVisible && pElem->GetID () != 0)
			{
				lstInvisiblePanes.AddTail (pElem->GetID ());
			}
		}

		for (i = 0; i < (int) m_arExElements.GetSize (); i++)
		{
			CBCGPBaseRibbonElement* pElem = m_arExElements [i];
			ASSERT_VALID (pElem);

			if (!pElem->m_bIsVisible && pElem->GetID () != 0)
			{
				lstInvisiblePanes.AddTail (pElem->GetID ());
			}
		}

		reg.Write (REG_ENTRY_STATUSBAR_PANES, lstInvisiblePanes);
	}

	bResult = CBCGPControlBar::SaveState (lpszProfileName, nIndex, uiID);

	return bResult;
}
//*********************************************************************
BOOL CBCGPRibbonStatusBar::LoadState (LPCTSTR lpszProfileName, int nIndex, UINT uiID)
{
	CString strProfileName = ::BCGPGetRegPath (strRibbonProfile, lpszProfileName);

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

	CList<UINT,UINT> lstInvisiblePanes;
	reg.Read (REG_ENTRY_STATUSBAR_PANES, lstInvisiblePanes);

	int i = 0;

	for (i = 0; i < (int) m_arElements.GetSize (); i++)
	{
		CBCGPBaseRibbonElement* pElem = m_arElements [i];
		ASSERT_VALID (pElem);

		if (lstInvisiblePanes.Find (pElem->GetID ()) != NULL)
		{
			pElem->SetVisible (FALSE);
		}
	}

	for (i = 0; i < (int) m_arExElements.GetSize (); i++)
	{
		CBCGPBaseRibbonElement* pElem = m_arExElements [i];
		ASSERT_VALID (pElem);

		if (lstInvisiblePanes.Find (pElem->GetID ()) != NULL)
		{
			pElem->SetVisible (FALSE);
		}
	}

	RecalcLayout ();

	return CBCGPControlBar::LoadState (lpszProfileName, nIndex, uiID);
}
//*********************************************************************
void CBCGPRibbonStatusBar::OnRTLChanged (BOOL bIsRTL)
{
	CBCGPControlBar::OnRTLChanged (bIsRTL);

	int i = 0;

	for (i = 0; i < (int) m_arElements.GetSize (); i++)
	{
		CBCGPBaseRibbonElement* pElem = m_arElements [i];
		ASSERT_VALID (pElem);

		pElem->OnRTLChanged (bIsRTL);
	}

	for (i = 0; i < (int) m_arExElements.GetSize (); i++)
	{
		CBCGPBaseRibbonElement* pElem = m_arExElements [i];
		ASSERT_VALID (pElem);

		pElem->OnRTLChanged (bIsRTL);
	}
}
//*********************************************************************
BOOL CBCGPRibbonStatusBar::IsExtendedElement (CBCGPBaseRibbonElement* pElement) const
{
	ASSERT_VALID (this);

	for (int i = 0; i < (int) m_arExElements.GetSize (); i++)
	{
		if (pElement == m_arExElements [i])
		{
			return TRUE;
		}
	}

	return FALSE;
}
//*********************************************************************
void CBCGPRibbonStatusBar::SetInformation (LPCTSTR lpszInfo)
{
	ASSERT_VALID (this);

	CString strInfoOld = m_strInfo;

	m_strInfo = lpszInfo == NULL ? _T("") : lpszInfo;

	if (strInfoOld == m_strInfo)
	{
		return;
	}

	BOOL bRecalcLayout = m_strInfo.IsEmpty () != strInfoOld.IsEmpty ();

	if (bRecalcLayout)
	{
		RecalcLayout ();
		RedrawWindow ();
	}
	else
	{
		RedrawWindow (m_rectInfo);
	}

	PostMessage (UM_UPDATE_SHADOWS);
}
//*********************************************************************
void CBCGPRibbonStatusBar::OnDrawInformation (CDC* pDC, CString& strInfo, CRect rectInfo)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);

	UINT uiDTFlags = DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX;

	rectInfo.DeflateRect (2, 0);

	COLORREF clrTextOld = pDC->SetTextColor (
		CBCGPVisualManager::GetInstance ()->GetRibbonStatusBarTextColor (this));

	pDC->DrawText (strInfo, rectInfo, uiDTFlags);
	pDC->SetTextColor (clrTextOld);
}
//*********************************************************************
LRESULT CBCGPRibbonStatusBar::OnUpdateShadows(WPARAM,LPARAM)
{
	CRect rectWindow;
	GetWindowRect (rectWindow);

	CBCGPPopupMenu::UpdateAllShadows (rectWindow);
	return 0;
}
//*********************************************************************
void CBCGPRibbonStatusBar::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CBCGPRibbonBar::OnShowWindow(bShow, nStatus);
	
	if (!m_bTemporaryHidden && CBCGPVisualManager::GetInstance()->IsStatusBarCoversFrame() && GetParentFrame () != NULL)
	{
		GetParentFrame ()->PostMessage (BCGM_CHANGEVISUALMANAGER);
	}
}
//*********************************************************************
void CBCGPRibbonStatusBar::SetShowEmptyExtendedArea (BOOL bShowEmptyExtArea)
{
	if (m_bShowEmptyExtArea != bShowEmptyExtArea)
	{
		m_bShowEmptyExtArea = bShowEmptyExtArea;
	}
}
//*********************************************************************
CString CBCGPRibbonStatusBar::GetLabel (const CBCGPBaseRibbonElement* pElement) const
{
	CStringArray* pSA = const_cast<CStringArray*>(&m_arExElementLabels);

	int nIndex = -1;
	int i = 0;
	for (i = 0; i < m_arExElements.GetSize (); i++)
	{
		CBCGPBaseRibbonElement* pElem = m_arExElements [i];
		ASSERT_VALID (pElem);

		if (pElement == pElem)
		{
			nIndex = i;
			break;
		}
	}

	if (nIndex == -1)
	{
		pSA = const_cast<CStringArray*>(&m_arElementLabels);

		for (i = 0; i < m_arElements.GetSize (); i++)
		{
			CBCGPBaseRibbonElement* pElem = m_arElements [i];
			ASSERT_VALID (pElem);

			if (pElement == pElem)
			{
				nIndex = i;
				break;
			}
		}
	}

	CString str;
	if (nIndex != -1)
	{
		str = pSA->GetAt (nIndex);
	}

	return str;
}
//****************************************************************************
void CBCGPRibbonStatusBar::SetInputMode(BCGP_INPUT_MODE mode)
{
	ASSERT_VALID(this);

	CBCGPRibbonBar::SetInputMode(mode);

	int i = 0;
	
	for (i = 0; i < (int) m_arElements.GetSize (); i++)
	{
		CBCGPBaseRibbonElement* pElem = m_arElements [i];
		ASSERT_VALID (pElem);
		
		pElem->SetPadding(m_sizePadding);
	}
	
	for (i = 0; i < (int) m_arExElements.GetSize (); i++)
	{
		CBCGPBaseRibbonElement* pElem = m_arExElements [i];
		ASSERT_VALID (pElem);
		
		pElem->SetPadding(m_sizePadding);
	}

	ForceRecalcLayout();
}
//*********************************************************************
void CBCGPRibbonStatusBar::GetVisibleElements (CArray<CBCGPBaseRibbonElement*, CBCGPBaseRibbonElement*>& arButtons)
{
	ASSERT_VALID (this);

	arButtons.RemoveAll ();

	for (int i = 0; i < GetCount(); i++)
	{
		CBCGPBaseRibbonElement* pElement = GetElement(i);

	  if (pElement != NULL && !pElement->IsSeparator () && pElement->IsVisible())
		{
			pElement->GetVisibleElements(arButtons);
		}
	}

	for (int j = 0; j < GetExCount(); j++)
	{
		CBCGPBaseRibbonElement* pElement = GetExElement(j);
		if (pElement != NULL && !pElement->IsSeparator () && pElement->IsVisible())
		{
			pElement->GetVisibleElements(arButtons);
		}
	}
}

//*********************************************************************
CBCGPBaseAccessibleObject* CBCGPRibbonStatusBar::AccessibleObjectFromPoint(CPoint point)
{
	return HitTest(point);
}
//**********************************************************************************
CBCGPBaseAccessibleObject* CBCGPRibbonStatusBar::AccessibleObjectByIndex(long lVal)
{
	CArray<CBCGPBaseRibbonElement*, CBCGPBaseRibbonElement*> arButtons;
	GetVisibleElements(arButtons);
	if (lVal > 0 && lVal <= (int)arButtons.GetSize())
	{
		return arButtons[lVal - 1];
	}
	return NULL;
}
#endif // BCGP_EXCLUDE_RIBBON

