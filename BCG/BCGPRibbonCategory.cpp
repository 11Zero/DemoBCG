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
// BCGPRibbonCategoryWnd.cpp : implementation file
//

#include "stdafx.h"
#include "multimon.h"
#include "BCGPRibbonPanel.h"
#include "BCGPRibbonCategory.h"
#include "BCGPRibbonBar.h"
#include "BCGPVisualManager.h"
#include "BCGPRibbonPanelMenu.h"
#include "BCGPRibbonMainPanel.h"

#ifndef BCGP_EXCLUDE_RIBBON

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static const int nPanelMarginLeft = 2;
static const int nPanelMarginRight = 2;
static const int nPanelMarginBottom = 4;

/////////////////////////////////////////////////////////////////////////////
// CBCGPRibbonTab

IMPLEMENT_DYNAMIC(CBCGPRibbonTab, CBCGPBaseRibbonElement)

CBCGPRibbonTab::CBCGPRibbonTab ()
{
	m_bIsTrancated = FALSE;
	m_Color = BCGPCategoryColor_None;
	m_nFullWidth = 0;
}

CString CBCGPRibbonTab::GetToolTipText () const
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pParent);

	if (!m_bIsTrancated)
	{
		return _T("");
	}

	CString strToolTipText = m_pParent->GetDisplayName();
	strToolTipText.Remove (_T('&'));

	return strToolTipText;
}

void CBCGPRibbonTab::CopyFrom (const CBCGPBaseRibbonElement& s)
{
	CBCGPBaseRibbonElement::CopyFrom (s);

	CBCGPRibbonTab& src = (CBCGPRibbonTab&) s;
	m_Color = src.m_Color;
	m_nFullWidth = src.m_nFullWidth;
}

void CBCGPRibbonTab::OnDraw (CDC* pDC)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);
	ASSERT_VALID (m_pParent);
	ASSERT_VALID (m_pParent->GetParentRibbonBar ());

	if (m_rect.IsRectEmpty ())
	{
		return;
	}

	COLORREF clrText = CBCGPVisualManager::GetInstance ()->OnDrawRibbonCategoryTab (
		pDC, this, (m_pParent->IsActive () || GetDroppedDown () != NULL) && !m_pParent->GetParentRibbonBar ()->IsBackstageViewActive());

	CString strName = m_pParent->GetDisplayName(TRUE);

	if (!strName.IsEmpty ())
	{
		m_nTextGlassGlowSize = 0;

		CRect rectTab = m_rect;
		CRect rectTabText = m_rect;

		pDC->DrawText (strName, rectTabText, DT_CALCRECT | DT_SINGLELINE | DT_VCENTER);

		const int cxTabText = rectTabText.Width ();
		const int cxTabTextMargin = max (4, (rectTab.Width () - cxTabText) / 2);

		rectTab.DeflateRect (cxTabTextMargin, 0);
		rectTab.top += CBCGPVisualManager::GetInstance()->GetRibbonPanelMarginTop();

		DoDrawText(pDC, strName, rectTab, DT_SINGLELINE | DT_VCENTER, clrText);
	}	
}

CRect CBCGPRibbonTab::GetKeyTipRect (CDC* pDC, BOOL /*bIsMenu*/)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);
	ASSERT_VALID (m_pParent);
	ASSERT_VALID (m_pParent->m_pParenrRibbonBar);

	CSize sizeKeyTip = GetKeyTipSize (pDC);

	if (sizeKeyTip == CSize (0, 0) || m_rect.IsRectEmpty ())
	{
		return CRect (0, 0, 0, 0);
	}

	CRect rectKeyTip (0, 0, 0, 0);

	CRect rectTab = m_rect;
	CRect rectTabText = m_rect;

	CString strName = m_pParent->GetDisplayName(TRUE);

	if (!strName.IsEmpty ())
	{
		pDC->DrawText (strName, rectTabText, DT_CALCRECT | DT_SINGLELINE | DT_VCENTER);
	}

	const int cxTabText = rectTabText.Width ();
	const int cxTabTextMargin = max (4, (rectTab.Width () - cxTabText) / 2);

	rectTab.DeflateRect (cxTabTextMargin, 0);
	rectTab.top += CBCGPVisualManager::GetInstance()->GetRibbonPanelMarginTop();

	rectKeyTip.left = rectTab.CenterPoint ().x - sizeKeyTip.cx / 2;
	rectKeyTip.right = rectKeyTip.left + sizeKeyTip.cx;

	rectKeyTip.top = rectTabText.bottom - 2;
	rectKeyTip.bottom = rectKeyTip.top + sizeKeyTip.cy;

	return rectKeyTip;
}

BOOL CBCGPRibbonTab::OnKey (BOOL /*bIsMenuKey*/)
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pParent);

	CBCGPRibbonBar* pBar = m_pParent->GetParentRibbonBar ();
	ASSERT_VALID (pBar);

	if (IsDisabled ())
	{
		return FALSE;
	}

	if (m_pParent->GetParentMenuBar () != NULL)
	{
		// Already dropped-down
		return TRUE;
	}

	pBar->SetActiveCategory (m_pParent);

	if ((pBar->GetHideFlags () & BCGPRIBBONBAR_HIDE_ELEMENTS) == 0 && !pBar->AreAllTabsAreInvisible())
	{
		pBar->SetKeyboardNavigationLevel (m_pParent);
	}

	return FALSE;
}

void CBCGPRibbonTab::Redraw ()
{
	ASSERT_VALID (this);

	if (m_rect.IsRectEmpty ())
	{
		return;
	}

	ASSERT_VALID (m_pParent);

	CBCGPRibbonBar* pBar = m_pParent->GetParentRibbonBar ();
	ASSERT_VALID (pBar);

	CRect rect = m_rect;

	rect.InflateRect (10, 10);
	pBar->RedrawWindow (rect);
}

CSize CBCGPRibbonTab::GetRegularSize (CDC* /*pDC*/)
{
	return CSize (0, 0);
}

void CBCGPRibbonTab::OnLButtonDown (CPoint /*point*/)
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pParent);

	m_pParent->GetParentRibbonBar ()->SetActiveCategory (m_pParent);
}

void CBCGPRibbonTab::OnAccDefaultAction()
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pParent);

	if (m_pParent == NULL || m_pParent->GetParentRibbonBar()->GetSafeHwnd() == NULL)
	{
		return;
	}

	CBCGPRibbonBar* pBar = m_pParent->GetParentRibbonBar();
	ASSERT_VALID(pBar);

	const BOOL bIsRibbonMinimized = (pBar->GetHideFlags() & BCGPRIBBONBAR_HIDE_ELEMENTS) != 0;

	if (pBar->IsSingleLevelAccessibilityMode() || !bIsRibbonMinimized)
	{
	   pBar->SetActiveCategory(m_pParent);
	}
	else
	{
		pBar->OnLButtonDown(0, m_rect.TopLeft());
		pBar->OnLButtonUp(0, m_rect.TopLeft());
	}
}

void CBCGPRibbonTab::OnLButtonDblClk (CPoint /*point*/)
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pParent);

	if (m_pParent->IsActive ())
	{
		if (m_pParent->m_ActiveTime != (clock_t)-1 && 
			clock () - m_pParent->m_ActiveTime < (int) GetDoubleClickTime ())
		{
			return;
		}

		CBCGPRibbonBar* pBar = m_pParent->GetParentRibbonBar ();
		ASSERT_VALID (pBar);

		if ((pBar->GetHideFlags () & BCGPRIBBONBAR_HIDE_ELEMENTS) != 0)
		{
			// Ribbon is minimized, restore it now:
			if (IsDroppedDown ())
			{
				ClosePopupMenu ();
			}
			
			m_pParent->ShowElements ();
		}
		else
		{
			// Minimize ribbon:
			m_pParent->ShowElements (FALSE);
		}

		pBar->GetParentFrame ()->RecalcLayout ();
		pBar->RedrawWindow ();
	}
}

BOOL CBCGPRibbonTab::IsSelected () const
{
	ASSERT_VALID (this);
	return m_bIsFocused;
}

BOOL CBCGPRibbonTab::SetACCData (CWnd* pParent, CBCGPAccessibilityData& data)
{
	ASSERT_VALID(this);

	if (m_pParent == NULL)
	{
		return FALSE;
	}

	ASSERT_VALID(m_pParent);

	CBCGPRibbonBar* pBar = m_pParent->GetParentRibbonBar();
	if (pBar->GetSafeHwnd() == NULL)
	{
		return FALSE;
	}

	ASSERT_VALID(pBar);

	const BOOL bIsRibbonMinimized = (pBar->GetHideFlags () & BCGPRIBBONBAR_HIDE_ELEMENTS) != 0;

	if (!CBCGPBaseRibbonElement::SetACCData(pParent, data))
	{
		return FALSE;
	}

	data.m_bAccState = STATE_SYSTEM_FOCUSABLE | STATE_SYSTEM_SELECTABLE;

	if (bIsRibbonMinimized)
	{
		data.m_bAccState |= STATE_SYSTEM_HASPOPUP;

		if (IsDroppedDown())
		{
			data.m_bAccState |= (STATE_SYSTEM_SELECTED | STATE_SYSTEM_PRESSED);
			data.m_strAccDefAction = _T("Close");
		}
		else
		{
			data.m_strAccDefAction = _T("Open");
		}
	}
	else 
	{
		if (m_pParent->IsActive())
		{
			 data.m_bAccState |= STATE_SYSTEM_SELECTED;
		}

		data.m_strAccDefAction = _T("Switch");
	}

	if (IsFocused ())
	{
		data.m_bAccState |= STATE_SYSTEM_FOCUSED;
	}

	data.m_strAccName = m_pParent->GetDisplayName();
	data.m_nAccRole = ROLE_SYSTEM_PAGETAB;
	data.m_strAccKeys = _T("Alt, ") + m_strKeys;

	return TRUE;
}
//********************************************************************************
HRESULT CBCGPRibbonTab::accHitTest(long /*xLeft*/, long /*yTop*/, VARIANT* pvarChild)
{
	if (!pvarChild)
    {
        return E_INVALIDARG;
    }

	pvarChild->vt = VT_I4;
	pvarChild->lVal = CHILDID_SELF;

	return S_OK;
}
//********************************************************************************
HRESULT CBCGPRibbonTab::get_accParent(IDispatch **ppdispParent)
{
	HRESULT hr = E_INVALIDARG;

	if (m_pParent == NULL || m_pParent->GetParentRibbonBar()->GetSafeHwnd() == NULL)
	{
		return hr;
	}

    if (ppdispParent && m_pParent->m_pParenrRibbonBar->GetTabs() != NULL)
    {
		*ppdispParent = m_pParent->m_pParenrRibbonBar->GetTabs()->GetIDispatch(TRUE);
		hr  = (*ppdispParent) ? S_OK : S_FALSE;
    }          

    return hr;
}
//********************************************************************************
HRESULT CBCGPRibbonTab::accLocation(long *pxLeft, long *pyTop, long *pcxWidth, long *pcyHeight, VARIANT varChild)
{
    if (!pxLeft || !pyTop || !pcxWidth || !pcyHeight)
    {
        return E_INVALIDARG;
    }

	if (m_pParent == NULL || m_pParent->m_pParenrRibbonBar->GetSafeHwnd() == NULL)
	{
		return S_FALSE;
	}

	if (varChild.vt == VT_I4 && varChild.lVal == CHILDID_SELF)
	{		
		CRect rectAccLocation = m_rect;
	    m_pParent->m_pParenrRibbonBar->ClientToScreen(&rectAccLocation);

		*pxLeft = rectAccLocation.left;
		*pyTop = rectAccLocation.top;
		*pcxWidth = rectAccLocation.Width();
		*pcyHeight = rectAccLocation.Height();
	}

	return S_OK;
}
//********************************************************************************
HRESULT CBCGPRibbonTab::get_accDefaultAction(VARIANT varChild, BSTR *pszDefaultAction)
{
	if (varChild.vt == VT_I4 && varChild.lVal == CHILDID_SELF)
	{
		CString str = _T("Switch");
		*pszDefaultAction = str.AllocSysString();

		return S_OK;
	}

	return S_FALSE;
}
//********************************************************************************
HRESULT CBCGPRibbonTab::accNavigate(long navDir, VARIANT varStart, VARIANT *pvarEndUpAt)
{
	pvarEndUpAt->vt = VT_EMPTY;

    if (varStart.vt != VT_I4)
    {
        return E_INVALIDARG;
    }

	if (m_pParent == NULL)
	{
		return S_FALSE;
	}

	ASSERT_VALID(m_pParent);
	
	CBCGPRibbonBar* pBar = m_pParent->GetParentRibbonBar();
	if (pBar->GetSafeHwnd() == NULL)
	{
		return S_FALSE;
	}
	
	ASSERT_VALID(pBar);

	CBCGPRibbonTabsGroup* pTabs = pBar->GetTabs();
	if (pTabs == NULL)
	{
		return S_FALSE;
	}

	ASSERT_VALID(pTabs);

	switch (navDir)
	{
	case NAVDIR_NEXT:   
	case NAVDIR_RIGHT:
		if (varStart.lVal == CHILDID_SELF)
		{
			//next tab
			int nIndex =  pTabs->GetButtonIndex(this) + 1;
			if (nIndex < pTabs->GetCount ())
			{
				CBCGPBaseRibbonElement* pButton = pTabs->GetButton(nIndex);
				if (pButton != NULL)
				{
					ASSERT_VALID(pButton);
					pvarEndUpAt->vt = VT_DISPATCH;
					pvarEndUpAt->pdispVal = pButton->GetIDispatch(TRUE);
					return S_OK;
				}
			}
		}
		break;

	case NAVDIR_PREVIOUS: 
	case NAVDIR_LEFT:
		if (varStart.lVal == CHILDID_SELF)
		{
			//prev tab
			int nIndex =  pTabs->GetButtonIndex (this) - 1;
			if (nIndex >= 0)
			{
				CBCGPBaseRibbonElement* pButton = pTabs->GetButton(nIndex);
				if (pButton != NULL)
				{
					ASSERT_VALID(pButton);
					pvarEndUpAt->vt = VT_DISPATCH;
					pvarEndUpAt->pdispVal = pButton->GetIDispatch(TRUE);
					return S_OK;
				}
			}
		}
		break;
	}

	return S_FALSE;
}
/////////////////////////////////////////////////////////////////////////////
// CBCGPRibbonCategoryScroll

IMPLEMENT_DYNAMIC(CBCGPRibbonCategoryScroll, CBCGPRibbonButton)

CBCGPRibbonCategoryScroll::CBCGPRibbonCategoryScroll ()
{
	m_bIsLeft = FALSE;
}

void CBCGPRibbonCategoryScroll::CopyFrom (const CBCGPBaseRibbonElement& s)
{
	CBCGPRibbonButton::CopyFrom (s);

	CBCGPRibbonCategoryScroll& src = (CBCGPRibbonCategoryScroll&) s;
	m_bIsLeft = src.m_bIsLeft;
}

void CBCGPRibbonCategoryScroll::OnDraw (CDC* pDC)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);

	if (m_rect.IsRectEmpty ())
	{
		return;
	}

	CBCGPVisualManager::GetInstance ()->OnDrawRibbonCategoryScroll (
		pDC, this);
}

BOOL CBCGPRibbonCategoryScroll::OnAutoRepeat ()
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pParent);

	if (m_rect.IsRectEmpty ())
	{
		return FALSE;
	}

	return m_pParent->OnScrollHorz (m_bIsLeft);
}

void CBCGPRibbonCategoryScroll::OnMouseMove(CPoint point) 
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pParent);

	if (m_rect.IsRectEmpty ())
	{
		m_bIsHighlighted = FALSE;
		return;
	}

	BOOL bWasHighlighted = m_bIsHighlighted;
	m_bIsHighlighted = m_rect.PtInRect (point);

	if (bWasHighlighted != m_bIsHighlighted)
	{
		if (m_pParent->GetParentMenuBar () != NULL)
		{
			m_pParent->GetParentMenuBar ()->PopTooltip ();
		}
		else if (m_pParent->GetParentRibbonBar () != NULL)
		{
			m_pParent->GetParentRibbonBar ()->PopTooltip ();
		}

		Redraw ();
	}
}
/////////////////////////////////////////////////////////////////////////////
// CBCGPRibbonCategory

IMPLEMENT_DYNCREATE(CBCGPRibbonCategory, CBCGPBaseAccessibleObject)

CBCGPRibbonCategory::CBCGPRibbonCategory ()
{
	CommonInit ();
}
//******************************************************************************************
CBCGPRibbonCategory::CBCGPRibbonCategory (
		CBCGPRibbonBar*	pParenrRibbonBar,
		LPCTSTR			lpszName,
		UINT			uiSmallImagesResID,
		UINT			uiLargeImagesResID,
		CSize			sizeSmallImage,
		CSize			sizeLargeImage)
{
	ASSERT_VALID (pParenrRibbonBar);
	ASSERT (lpszName != NULL);

	CommonInit (pParenrRibbonBar, lpszName, uiSmallImagesResID, uiLargeImagesResID,
		sizeSmallImage, sizeLargeImage);
}
//******************************************************************************************
CBCGPRibbonCategory::CBCGPRibbonCategory(CBCGPRibbonBar*	pParenrRibbonBar)
{
	ASSERT_VALID (pParenrRibbonBar);

	CommonInit (pParenrRibbonBar);
	m_bIsCustom = TRUE;
}
//******************************************************************************************
void CBCGPRibbonCategory::CommonInit (
		CBCGPRibbonBar*	pParenrRibbonBar,
		LPCTSTR			lpszName,
		UINT			uiSmallImagesResID,
		UINT			uiLargeImagesResID,
		CSize			sizeSmallImage,
		CSize			sizeLargeImage)
{
	m_nKey = -1;

	m_bIsNew = FALSE;
	m_bToBeDeleted = FALSE;
	m_bIsPrintPreview = FALSE;

	m_pParentMenuBar = NULL;
	m_bMouseIsPressed = FALSE;
	m_bIsActive = FALSE;
	m_bIsVisible = TRUE;
	m_bIsQATOnly = FALSE;
	m_bIsCustom = FALSE;
	m_dwData = 0;
	m_uiContextID = 0;
	m_nLastCategoryWidth = -1;
	m_nLastCategoryOffsetY = 0;
	m_nMinWidth = -1;
	m_bOnDialogBar = FALSE;

	m_rect.SetRectEmpty ();

	m_pParenrRibbonBar = pParenrRibbonBar;
	SetName (lpszName);

	//-------------
	// Load images:
	//-------------
	if (sizeSmallImage != CSize (0, 0))
	{
		m_SmallImages.SetImageSize (sizeSmallImage);
	}

	if (sizeLargeImage != CSize (0, 0))
	{
		m_LargeImages.SetImageSize (sizeLargeImage);
	}

	if (uiSmallImagesResID > 0)
	{
		if (!m_SmallImages.Load (uiSmallImagesResID))
		{
			ASSERT (FALSE);
		}
	}

	if (uiLargeImagesResID > 0)
	{
		if (!m_LargeImages.Load (uiLargeImagesResID))
		{
			ASSERT (FALSE);
		}
	}

	const double dblScale = globalData.GetRibbonImageScale ();
	if (dblScale != 1.0)
	{
		if (sizeSmallImage == CSize (16, 16))
		{
			m_SmallImages.SetTransparentColor (globalData.clrBtnFace);
			m_SmallImages.SmoothResize (dblScale);
		}

		if (sizeLargeImage == CSize (32, 32))
		{
			m_LargeImages.SetTransparentColor (globalData.clrBtnFace);
			m_LargeImages.SmoothResize (dblScale);
		}
	}

	m_Tab.m_pParent = this;

	int nIndex = m_strName.Find (_T('\n'));
	if (nIndex >= 0)
	{
		m_Tab.SetKeys (m_strName.Mid (nIndex + 1));
		m_strName = m_strName.Left (nIndex);
	}

	m_ScrollLeft.m_pParent = this;
	m_ScrollRight.m_pParent = this;
	m_ScrollLeft.m_bIsLeft = TRUE;
	m_ScrollRight.m_bIsLeft = FALSE;

	m_nScrollOffset = 0;
	m_ActiveTime = (clock_t)-1;
}
//******************************************************************************************
CBCGPRibbonCategory::~CBCGPRibbonCategory()
{
	int i = 0;

	for (i = 0; i < m_arPanels.GetSize (); i++)
	{
		delete m_arPanels [i];
	}

	for (i = 0; i < m_arElements.GetSize (); i++)
	{
		delete m_arElements [i];
	}
}
//******************************************************************************************
void CBCGPRibbonCategory::OnDraw (CDC* pDC) 
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);

	if (m_rect.IsRectEmpty ())
	{
		return;
	}

	CBCGPVisualManager::GetInstance ()->OnDrawRibbonCategory (
		pDC, this, m_rect);

	BOOL bClip = FALSE;

	CRgn rgnClip;

	if (!m_ScrollLeft.GetRect ().IsRectEmpty () ||
		!m_ScrollRight.GetRect ().IsRectEmpty ())
	{
		CRect rectClient = m_rect;
		rectClient.DeflateRect (nPanelMarginLeft, CBCGPVisualManager::GetInstance()->GetRibbonPanelMarginTop(), 
			nPanelMarginRight, nPanelMarginBottom);

		rgnClip.CreateRectRgnIndirect (rectClient);
		pDC->SelectClipRgn (&rgnClip);

		bClip = TRUE;
	}

	for (int i = 0; i < m_arPanels.GetSize (); i++)
	{
		CBCGPRibbonPanel* pPanel = m_arPanels [i];
		ASSERT_VALID (pPanel);

		pPanel->DoPaint (pDC);
	}

	if (bClip)
	{
		pDC->SelectClipRgn (NULL);
	}

	m_ScrollLeft.OnDraw (pDC);
	m_ScrollRight.OnDraw (pDC);
}
//******************************************************************************************
CBCGPRibbonPanel* CBCGPRibbonCategory::AddPanel (LPCTSTR lpszPanelName,
												 HICON hIcon,
												 CRuntimeClass* pRTI,
												 BOOL bAutoDestroyIcon,
												 int nInsertAt/* = -1*/)
{
	ASSERT_VALID (this);
	ASSERT (lpszPanelName != NULL);

	CBCGPRibbonPanel* pPanel = NULL;

	if (pRTI != NULL)
	{
		pPanel = DYNAMIC_DOWNCAST (CBCGPRibbonPanel, pRTI->CreateObject ());

		if (pPanel == NULL)
		{
			ASSERT (FALSE);
			return NULL;
		}

		pPanel->CommonInit (lpszPanelName, hIcon, bAutoDestroyIcon);
	}
	else
	{
		pPanel = new CBCGPRibbonPanel (lpszPanelName, hIcon, bAutoDestroyIcon);
	}

	AddPanelInternal(pPanel, nInsertAt);

	return pPanel;
}
//******************************************************************************************
CBCGPRibbonPanel* CBCGPRibbonCategory::AddPanel (LPCTSTR lpszPanelName,
												 const CBCGPToolBarImages& icons, 
												 int nIconIndex,
												 CRuntimeClass* pRTI,
												 int nInsertAt/* = -1*/)
{
	ASSERT_VALID (this);
	ASSERT (lpszPanelName != NULL);

	CBCGPRibbonPanel* pPanel = NULL;

	if (pRTI != NULL)
	{
		pPanel = DYNAMIC_DOWNCAST (CBCGPRibbonPanel, pRTI->CreateObject ());

		if (pPanel == NULL)
		{
			ASSERT (FALSE);
			return NULL;
		}

		pPanel->CommonInit (lpszPanelName, NULL, FALSE, &icons, nIconIndex);
	}
	else
	{
		pPanel = new CBCGPRibbonPanel (lpszPanelName, icons, nIconIndex);
	}

	AddPanelInternal(pPanel, nInsertAt);

	return pPanel;
}
//******************************************************************************************
void CBCGPRibbonCategory::AddPanelInternal(CBCGPRibbonPanel* pPanel, int nInsertAt)
{
	ASSERT_VALID (this);
	ASSERT_VALID(pPanel);

	if (nInsertAt == -1)
	{
		m_arPanels.Add (pPanel);
	}
	else
	{
		m_arPanels.InsertAt (nInsertAt, pPanel);
	}

	if (GetParentRibbonBar() != NULL)
	{
		pPanel->m_nKey = GetParentRibbonBar()->m_nNextPanelKey++;
	}

	pPanel->m_pParent = this;
	pPanel->m_btnLaunch.m_pParent = this;
	pPanel->m_btnDefault.m_pParent = this;

	if (m_bIsCustom)
	{
		pPanel->SetCustom();
	}

	m_nLastCategoryWidth = -1;
	m_nMinWidth = -1;
}
//******************************************************************************************
void CBCGPRibbonCategory::AddCustomPanel(CBCGPRibbonPanel* pPanel, int nIndex)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pPanel);

	if (nIndex == -1)
	{
		m_arPanels.Add (pPanel);
	}
	else
	{
		m_arPanels.InsertAt (nIndex, pPanel);
	}

	if (pPanel->m_pOriginal == NULL && GetParentRibbonBar() != NULL)
	{
		pPanel->m_nKey = GetParentRibbonBar()->m_nNextPanelKey++;
	}

	pPanel->m_pParent = this;
	pPanel->m_btnLaunch.m_pParent = this;
	pPanel->m_btnDefault.m_pParent = this;

	m_nLastCategoryWidth = -1;
	m_nMinWidth = -1;
}
//******************************************************************************************
int CBCGPRibbonCategory::GetPanelCount () const
{
	ASSERT_VALID (this);
	return (int) m_arPanels.GetSize ();
}
//******************************************************************************************
BOOL CBCGPRibbonCategory::RemovePanel (int nIndex, BOOL bDelete/* = TRUE*/)
{
	ASSERT_VALID (this);

	if (nIndex < 0 || nIndex >= m_arPanels.GetSize ())
	{
		ASSERT (FALSE);
		return FALSE;
	}

	CBCGPRibbonPanel* pPanel = m_arPanels [nIndex];
	ASSERT_VALID (pPanel);

	m_arPanels.RemoveAt (nIndex);

	if (bDelete)
	{
		delete pPanel;
	}

	return TRUE;
}
//******************************************************************************************
CBCGPRibbonPanel* CBCGPRibbonCategory::GetPanel (int nIndex)
{
	ASSERT_VALID (this);
	return m_arPanels [nIndex];
}
//******************************************************************************************
int CBCGPRibbonCategory::GetPanelIndex (const CBCGPRibbonPanel* pPanel) const
{
	ASSERT_VALID (this);
	ASSERT_VALID (pPanel);

	for (int i = 0; i < m_arPanels.GetSize (); i++)
	{
		if (m_arPanels [i] == pPanel)
		{
			return i;
		}
	}

	return -1;
}
//******************************************************************************************
int CBCGPRibbonCategory::GetMaxHeight (CDC* pDC)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);

	int nMaxHeight = 0;

	for (int i = 0; i < m_arPanels.GetSize (); i++)
	{
		CBCGPRibbonPanel* pPanel = m_arPanels [i];
		ASSERT_VALID (pPanel);

		nMaxHeight = max (nMaxHeight, pPanel->GetHeight (pDC));
	}

	return nMaxHeight + pDC->GetTextExtent (GetDisplayName(TRUE)).cy + CBCGPVisualManager::GetInstance()->GetRibbonPanelMarginTop() + nPanelMarginBottom;
}
//******************************************************************************************
void CBCGPRibbonCategory::RecalcLayout (CDC* pDC)
{
	if (m_rect.IsRectEmpty ())
	{
		return;
	}

	if (m_pParentMenuBar != NULL)
	{
		CleanUpSizes ();
	}

	RecalcPanelWidths (pDC);

	if (m_arPanels.GetSize () == 0)
	{
		return;
	}

	if (m_bOnDialogBar)
	{
		ReposPanelsVert (pDC);

		if (GetParentRibbonBar ()->GetSafeHwnd () != NULL)
		{
			GetParentRibbonBar ()->RedrawWindow (m_rect);
		}

		return;
	}

	int i = 0;
	BOOL bRedrawScroll = FALSE;

	const DWORD dwRibbonHideFlags = GetParentRibbonBar ()->m_dwHideFlags;
	const BOOL bHideAll = (dwRibbonHideFlags & BCGPRIBBONBAR_HIDE_ALL) ||
		(dwRibbonHideFlags & BCGPRIBBONBAR_HIDE_ELEMENTS);

	if (m_nMinWidth < 0)
	{
		m_nMinWidth = GetMinWidth (pDC);
	}

	if (bHideAll && m_pParentMenuBar == NULL)
	{
		for (i = 0; i < m_arPanels.GetSize (); i++)
		{
			CBCGPRibbonPanel* pPanel = m_arPanels [i];
			ASSERT_VALID (pPanel);

			pPanel->Repos (pDC, CRect (0, 0, 0, 0));
			pPanel->OnShow (FALSE);
		}

		m_nLastCategoryWidth = -1;
		m_nMinWidth = -1;
	}
	else if (m_nLastCategoryWidth != m_rect.Width () || m_nLastCategoryOffsetY != m_rect.top)
	{
		m_nLastCategoryWidth = m_rect.Width ();
		m_nLastCategoryOffsetY = m_rect.top;

		CRect rectClient = m_rect;
		rectClient.DeflateRect (nPanelMarginLeft * 2, CBCGPVisualManager::GetInstance()->GetRibbonPanelMarginTop(), 
								nPanelMarginRight * 2, nPanelMarginBottom);

		ResetPanelsLayout ();

		if (rectClient.Width () <= m_nMinWidth)
		{
			//-------------------------
			// Just collapse all panes:
			//-------------------------
			for (i = 0; i < m_arPanels.GetSize (); i++)
			{
				CBCGPRibbonPanel* pPanel = m_arPanels [i];
				ASSERT_VALID (pPanel);

				pPanel->ForceCollapse();
			}
		}
		else
		{
			BOOL bAutoResize = TRUE;

			if (m_arCollapseOrder.GetSize () > 0)
			{
				bAutoResize = FALSE;

				BOOL bOK = TRUE;

				for (int iNextPane = 0; iNextPane <= m_arCollapseOrder.GetSize (); iNextPane++)
				{
					bOK = SetPanelsLayout (rectClient.Width ());
					if (bOK || iNextPane == m_arCollapseOrder.GetSize ())
					{
						break;
					}

					//------------------------------------------------------------
					// Find next pane for collapsing - from the user-defined list:
					//------------------------------------------------------------
					int nPaneIndex = m_arCollapseOrder [iNextPane];
					if (nPaneIndex < 0 || nPaneIndex >= m_arPanels.GetSize ())
					{
						ASSERT (FALSE);
						bOK = FALSE;
						break;
					}

					CBCGPRibbonPanel* pPanel = m_arPanels [nPaneIndex];
					ASSERT_VALID (pPanel);

					if (iNextPane < m_arCollapseOrder.GetSize () - 1 &&
						m_arCollapseOrder [iNextPane + 1] == -1)
					{
						pPanel->ForceCollapse();
						iNextPane++;
					}
					else
					{
						if (pPanel->m_nCurrWidthIndex < pPanel->m_arWidths.GetSize () - 1)
						{
							pPanel->m_nCurrWidthIndex++;
						}
					}
				}

				if (!bOK)
				{
					bAutoResize = TRUE;
					ResetPanelsLayout ();
				}
			}
			
			if (bAutoResize)
			{
				while (TRUE)
				{
					if (SetPanelsLayout (rectClient.Width ()))
					{
						break;
					}

					//----------------------------------------------
					// Find next pane for collapsing - next matched:
					//----------------------------------------------
					int nMaxWeightIndex = -1;
					int nMaxWeight = 1;

					for (i = 0; i < m_arPanels.GetSize (); i++)
					{
						CBCGPRibbonPanel* pPanel = m_arPanels [i];
						ASSERT_VALID (pPanel);

						int nWeight = (int) pPanel->m_arWidths.GetSize () - pPanel->m_nCurrWidthIndex - 1;
						if (nWeight >= nMaxWeight)
						{
							nMaxWeightIndex = i;
							nMaxWeight = nWeight;
						}
					}

					if (nMaxWeightIndex < 0)
					{
						break;
					}

					CBCGPRibbonPanel* pPanel = m_arPanels [nMaxWeightIndex];
					ASSERT_VALID (pPanel);

					pPanel->m_nCurrWidthIndex++;
				}
			}
		}

		ReposPanels (pDC);
		bRedrawScroll = TRUE;
	}

	UpdateScrollButtons ();

	if (bRedrawScroll && GetParentRibbonBar ()->GetSafeHwnd () != NULL)
	{
		if (!m_ScrollLeft.GetRect ().IsRectEmpty () ||
			!m_ScrollRight.GetRect ().IsRectEmpty ())
		{
			GetParentRibbonBar ()->RedrawWindow (m_rect);
		}
	}
}
//******************************************************************************************
void CBCGPRibbonCategory::UpdateScrollButtons ()
{
	m_ScrollLeft.m_pParentMenu = m_pParentMenuBar;
	m_ScrollRight.m_pParentMenu = m_pParentMenuBar;

	CRect rectScrollRightOld = m_ScrollRight.GetRect ();

	const int cxScrollWidth = (int) (globalData.GetRibbonImageScale () * 13);

	CRect rectScrollLeft (0, 0, 0, 0);
	CRect rectScrollRight (0, 0, 0, 0);

	if (m_nScrollOffset > 0)
	{
		rectScrollLeft = m_rect;
		rectScrollLeft.right = rectScrollLeft.left + cxScrollWidth;
	}

	if (m_rect.Width () - 2 * cxScrollWidth + m_nScrollOffset < m_nMinWidth)
	{
		rectScrollRight = m_rect;
		rectScrollRight.left = rectScrollRight.right - cxScrollWidth;
	}

	m_ScrollLeft.SetRect (rectScrollLeft);
	m_ScrollRight.SetRect (rectScrollRight);

	if (rectScrollRight.IsRectEmpty () && !rectScrollRightOld.IsRectEmpty ())
	{
		GetParentRibbonBar ()->RedrawWindow (rectScrollRightOld);
	}
}
//******************************************************************************************
void CBCGPRibbonCategory::ReposPanels (CDC* pDC)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);

	CBCGPRibbonBar* pRibbonBar = GetParentRibbonBar ();
	ASSERT_VALID (pRibbonBar);

	CRect rectClient = m_rect;
	rectClient.DeflateRect (nPanelMarginLeft * 2, CBCGPVisualManager::GetInstance()->GetRibbonPanelMarginTop(), 
							nPanelMarginRight * 2, nPanelMarginBottom);

	const BOOL bForceCollpapse = (rectClient.Width () <= m_nMinWidth);

	int x = rectClient.left - m_nScrollOffset;

	CArray<CBCGPRibbonPanel*,CBCGPRibbonPanel*> arPanelsOrdered;
	
	if (m_bIsPrintPreview)
	{
		arPanelsOrdered.Append(m_arPanels);
	}
	else
	{
		pRibbonBar->m_CustomizationData.PreparePanelsArray(m_arPanels, arPanelsOrdered);
	}

	for (int i = 0; i < arPanelsOrdered.GetSize (); i++)
	{
		CBCGPRibbonPanel* pPanel = arPanelsOrdered [i];
		ASSERT_VALID (pPanel);

		if (pRibbonBar->m_CustomizationData.IsPanelHidden(pPanel))
		{
			pPanel->Repos (pDC, CRect(0, 0, 0, 0));
			pPanel->OnAfterChangeRect (pDC);
			continue;
		}

		if (bForceCollpapse)
		{
			pPanel->ForceCollapse();
		}

		const int nCurrPanelWidth = 
			pPanel->m_arWidths [pPanel->m_nCurrWidthIndex] + 
			2 * pPanel->m_nXMargin;

		CRect rectPanel = CRect (x, rectClient.top, 
								x + nCurrPanelWidth, rectClient.bottom);

		pPanel->Repos (pDC, rectPanel);

		x = pPanel->m_rect.right + CBCGPVisualManager::GetInstance()->GetRibbonPanelMargin(this);

		if (rectPanel.right <= rectClient.left + 2 * nPanelMarginLeft ||
			rectPanel.left >= rectClient.right - 2 * nPanelMarginRight)
		{
			rectPanel.SetRectEmpty ();
			pPanel->Repos (pDC, rectPanel);
		}

		if (bForceCollpapse && !pPanel->IsNonCollapsible())
		{
			pPanel->m_bForceCollpapse = TRUE;
		}

		pPanel->m_btnDefault.SetText(pPanel->GetDisplayName());
		pPanel->OnAfterChangeRect (pDC);
	}
}
//******************************************************************************************
void CBCGPRibbonCategory::ReposPanelsVert (CDC* pDC)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);

	ResetPanelsLayout ();

	CRect rectClient = m_rect;
	rectClient.DeflateRect (nPanelMarginLeft * 2, CBCGPVisualManager::GetInstance()->GetRibbonPanelMarginTop() * 2, 
							nPanelMarginRight * 2, nPanelMarginBottom * 2);

	int x = rectClient.left;
	int y = rectClient.top;

	for (int i = 0; i < m_arPanels.GetSize (); i++)
	{
		CBCGPRibbonPanel* pPanel = m_arPanels [i];
		ASSERT_VALID (pPanel);

		do
		{
			const int nCurrPanelWidth = 
				pPanel->m_arWidths [pPanel->m_nCurrWidthIndex] + 
				2 * pPanel->m_nXMargin;

			if (nCurrPanelWidth <= rectClient.Width ())
			{
				break;
			}

			pPanel->m_nCurrWidthIndex++;
		}
		while (pPanel->m_nCurrWidthIndex < pPanel->m_arWidths.GetSize () - 1);

		if (pPanel->m_nCurrWidthIndex == (int) pPanel->m_arWidths.GetSize () - 1)
		{
			pPanel->m_nCurrWidthIndex = 0;
		}

		int nVertMargin = 16;
		const double dblImageScale = globalData.GetRibbonImageScale ();
		if (dblImageScale != 1.)
		{
			nVertMargin = (int)(.5 + nVertMargin * dblImageScale);
		}

		CRect rectPanel = CRect (x, y, x + rectClient.Width (), y + pPanel->GetHeight (pDC) + nVertMargin);

		pPanel->Repos (pDC, rectPanel);

		pPanel->m_rect.right = x + rectClient.Width ();

		//-----------------------------
		// Set launch button rectangle:
		//-----------------------------
		CRect rectLaunch = pPanel->m_btnLaunch.GetRect ();

		if (!rectLaunch.IsRectEmpty ())
		{
			int cx = rectLaunch.Width ();

			rectLaunch.right = pPanel->m_rect.right - 1;
			rectLaunch.left = rectLaunch.right - cx;

			pPanel->m_btnLaunch.SetRect (rectLaunch);
		}

		//-----------------------
		// Set caption rectangle:
		//-----------------------
		if (pPanel->m_bShowCaption)
		{
			pPanel->m_rectCaption.right = pPanel->m_rect.right;
		}

		y = pPanel->m_rect.bottom + nPanelMarginBottom * 2;

		if (rectPanel.bottom <= rectClient.top + 2 * CBCGPVisualManager::GetInstance()->GetRibbonPanelMarginTop() ||
			rectPanel.top >= rectClient.bottom - 2 * nPanelMarginBottom)
		{
			rectPanel.SetRectEmpty ();
			pPanel->Repos (pDC, rectPanel);
		}

		pPanel->OnAfterChangeRect (pDC);
	}
}
//******************************************************************************************
void CBCGPRibbonCategory::RecalcPanelWidths (CDC* pDC)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);

	int nHeight = -1;

	for (int i = 0; i < m_arPanels.GetSize (); i++)
	{
		CBCGPRibbonPanel* pPanel = m_arPanels [i];
		ASSERT_VALID (pPanel);

		if (pPanel->m_arWidths.GetSize () == 0)
		{
			if (nHeight == -1)
			{
				nHeight = GetMaxHeight (pDC);
			}

			pPanel->RecalcWidths (pDC, nHeight);
			m_nLastCategoryWidth = -1;
		}
	}

	m_nMinWidth = -1;
}
//******************************************************************************************
void CBCGPRibbonCategory::CleanUpSizes ()
{
	ASSERT_VALID (this);

	for (int i = 0; i < m_arPanels.GetSize (); i++)
	{
		CBCGPRibbonPanel* pPanel = m_arPanels [i];
		ASSERT_VALID (pPanel);

		pPanel->CleanUpSizes ();

		pPanel->m_bForceCollpapse = FALSE;
		pPanel->m_arWidths.RemoveAll ();
	}

	m_nLastCategoryWidth = -1;
	m_nMinWidth = -1;
}
//******************************************************************************************
int CBCGPRibbonCategory::GetMinWidth (CDC* pDC)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);

	int nMinWidth = nPanelMarginLeft;

	for (int i = 0; i < m_arPanels.GetSize (); i++)
	{
		CBCGPRibbonPanel* pPanel = m_arPanels [i];
		ASSERT_VALID (pPanel);

		nMinWidth += pPanel->GetMinWidth (pDC) + CBCGPVisualManager::GetInstance()->GetRibbonPanelMargin(this);
	}

	return nMinWidth;
}
//******************************************************************************************
void CBCGPRibbonCategory::OnMouseMove(CPoint point) 
{
	ASSERT_VALID (this);

	m_ScrollLeft.OnMouseMove (point);
	m_ScrollRight.OnMouseMove (point);

	if (m_ScrollLeft.IsHighlighted () || m_ScrollRight.IsHighlighted ())
	{
		return;
	}

	HighlightPanel (GetPanelFromPoint (point), point);
}
//******************************************************************************************
CBCGPBaseRibbonElement* CBCGPRibbonCategory::HitTest (CPoint point, BOOL bCheckPanelCaption) const
{
	ASSERT_VALID (this);

	CBCGPBaseRibbonElement* pBtnScroll = HitTestScrollButtons (point);
	if (pBtnScroll != NULL)
	{
		return pBtnScroll;
	}

	CBCGPRibbonPanel* pPanel = GetPanelFromPoint (point);
	if (pPanel != NULL)
	{
		ASSERT_VALID (pPanel);
		return pPanel->HitTest (point, bCheckPanelCaption);
	}

	return NULL;
}
//******************************************************************************************
CBCGPBaseRibbonElement* CBCGPRibbonCategory::HitTestScrollButtons (CPoint point) const
{
	ASSERT_VALID (this);

	if (m_ScrollLeft.GetRect ().PtInRect (point))
	{
		return (CBCGPBaseRibbonElement*)&m_ScrollLeft;
	}

	if (m_ScrollRight.GetRect ().PtInRect (point))
	{
		return (CBCGPBaseRibbonElement*)&m_ScrollRight;
	}

	return NULL;
}
//******************************************************************************************
int CBCGPRibbonCategory::HitTestEx (CPoint point) const
{
	ASSERT_VALID (this);

	CBCGPRibbonPanel* pPanel = GetPanelFromPoint (point);
	if (pPanel != NULL)
	{
		ASSERT_VALID (pPanel);
		return pPanel->HitTestEx (point);
	}

	return -1;
}
//******************************************************************************************
CBCGPBaseRibbonElement* CBCGPRibbonCategory::GetFirstVisibleElement () const
{
	ASSERT_VALID (this);

	if (m_arPanels.GetSize () == 0)
	{
		return NULL;
	}

	//------------------------------------------------------
	// If the category is scrolled right, scroll left first:
	//------------------------------------------------------
	if (m_nScrollOffset > 0)
	{
		((CBCGPRibbonCategory*) this)->OnScrollHorz (TRUE, m_nScrollOffset);
	}

	CBCGPRibbonPanel* pPanel = m_arPanels [0];
	ASSERT_VALID (pPanel);

	if (pPanel->IsCollapsed ())
	{
		return &pPanel->GetDefaultButton ();
	}

	return pPanel->GetElement (0);
}
//******************************************************************************************
CBCGPBaseRibbonElement* CBCGPRibbonCategory::GetLastVisibleElement () const
{
	ASSERT_VALID (this);

	if (m_arPanels.GetSize () == 0)
	{
		return NULL;
	}

	//--------------------
	// Scroll right first:
	//--------------------
	while (!m_ScrollRight.GetRect ().IsRectEmpty ())
	{
		((CBCGPRibbonCategory*) this)->OnScrollHorz (FALSE);
	}

	CBCGPRibbonPanel* pPanel = m_arPanels [m_arPanels.GetSize () - 1];
	ASSERT_VALID (pPanel);

	if (pPanel->IsCollapsed ())
	{
		return &pPanel->GetDefaultButton ();
	}

	if (pPanel->GetCount () == 0)
	{
		return NULL;
	}

	return pPanel->GetElement (pPanel->GetCount () - 1);
}
//******************************************************************************************
CBCGPRibbonPanel* CBCGPRibbonCategory::GetPanelFromPoint (CPoint point) const
{
	ASSERT_VALID (this);

	for (int i = 0; i < m_arPanels.GetSize (); i++)
	{
		CBCGPRibbonPanel* pPanel = m_arPanels [i];
		ASSERT_VALID (pPanel);

		if (pPanel->m_rect.PtInRect (point))
		{
			return pPanel;
		}
	}

	return NULL;
}
//******************************************************************************************
CBCGPRibbonPanel* CBCGPRibbonCategory::HighlightPanel (CBCGPRibbonPanel* pHLPanel, CPoint point)
{
	ASSERT_VALID (this);

	CBCGPRibbonPanel* pPrevHLPanel = NULL;

	for (int i = 0; i < m_arPanels.GetSize (); i++)
	{
		CBCGPRibbonPanel* pPanel = m_arPanels [i];
		ASSERT_VALID (pPanel);

		if (pPanel->IsHighlighted ())
		{
			if (pHLPanel != pPanel)
			{
				pPanel->Highlight (FALSE, point);
			}

			pPrevHLPanel = pPanel;
		}
		
		if (pHLPanel == pPanel)
		{
			pPanel->Highlight (TRUE, point);
		}
	}

	if (m_pParentMenuBar != NULL)
	{
		ASSERT_VALID (m_pParentMenuBar);
		m_pParentMenuBar->UpdateWindow ();
	}
	else
	{
		ASSERT_VALID (m_pParenrRibbonBar);
		m_pParenrRibbonBar->UpdateWindow ();
	}

	return pPrevHLPanel;
}
//******************************************************************************************
void CBCGPRibbonCategory::OnCancelMode() 
{
	m_bMouseIsPressed = FALSE;

	for (int i = 0; i < m_arPanels.GetSize (); i++)
	{
		CBCGPRibbonPanel* pPanel = m_arPanels [i];
		ASSERT_VALID (pPanel);

		pPanel->CancelMode ();
	}
}
//******************************************************************************************
CBCGPBaseRibbonElement* CBCGPRibbonCategory::OnLButtonDown (CPoint point)
{
	CBCGPBaseRibbonElement* pBtnScroll = HitTestScrollButtons (point);
	if (pBtnScroll != NULL)
	{
		ASSERT_VALID(pBtnScroll);
		pBtnScroll->OnAutoRepeat ();

		if (HitTestScrollButtons (point) == pBtnScroll)
		{
			return pBtnScroll;
		}
		else
		{
			return NULL;
		}
	}

	CBCGPRibbonPanel* pPanel = GetPanelFromPoint (point);
	if (pPanel == NULL)
	{
		return NULL;
	}

	m_bMouseIsPressed = TRUE;

	ASSERT_VALID (pPanel);
	return pPanel->MouseButtonDown (point);
}
//******************************************************************************************
void CBCGPRibbonCategory::OnLButtonUp (CPoint point) 
{
	m_ScrollLeft.m_bIsHighlighted = FALSE;
	m_ScrollRight.m_bIsHighlighted = FALSE;

	CBCGPRibbonPanel* pPanel = GetPanelFromPoint (point);
	if (pPanel == NULL)
	{
		return;
	}

	m_bMouseIsPressed = FALSE;

	ASSERT_VALID (pPanel);
	pPanel->MouseButtonUp (point);
}
//********************************************************************************
void CBCGPRibbonCategory::OnUpdateCmdUI (CBCGPRibbonCmdUI* pCmdUI, 
										 CFrameWnd* pTarget, 
										 BOOL bDisableIfNoHndler)
{
	ASSERT_VALID (this);

	for (int i = 0; i < m_arPanels.GetSize (); i++)
	{
		CBCGPRibbonPanel* pPanel = m_arPanels [i];
		ASSERT_VALID (pPanel);

		pPanel->OnUpdateCmdUI (pCmdUI, pTarget, bDisableIfNoHndler);
	}
}
//********************************************************************************
BOOL CBCGPRibbonCategory::NotifyControlCommand (
	BOOL bAccelerator, int nNotifyCode, WPARAM wParam, LPARAM lParam)
{
	ASSERT_VALID (this);

	for (int i = 0; i < m_arPanels.GetSize (); i++)
	{
		CBCGPRibbonPanel* pPanel = m_arPanels [i];
		ASSERT_VALID (pPanel);

		if (pPanel->NotifyControlCommand (bAccelerator, nNotifyCode, wParam, lParam))
		{
			return TRUE;
		}
	}

	return FALSE;
}
//********************************************************************************
void CBCGPRibbonCategory::SetActive (BOOL bIsActive)
{
	ASSERT_VALID (this);

	if (m_bIsActive == bIsActive)
	{
		return;
	}

	if ((m_pParenrRibbonBar->m_dwHideFlags & BCGPRIBBONBAR_HIDE_ELEMENTS) != 0)
	{
		m_bIsActive = bIsActive;
		return;
	}

	if (!m_pParenrRibbonBar->m_bIsPrintPreview || !CBCGPVisualManager::GetInstance()->IsDWMCaptionSupported ())
	{
		ShowElements ();
	}

	m_bIsActive = bIsActive;

	for (int i = 0; i < m_arPanels.GetSize (); i++)
	{
		CBCGPRibbonPanel* pPanel = m_arPanels [i];
		ASSERT_VALID (pPanel);

		pPanel->OnShow (bIsActive);
	}

	m_ActiveTime = bIsActive ? clock () : (clock_t)-1;
}
//********************************************************************************
void CBCGPRibbonCategory::ShowElements  (BOOL bShow)
{
	ASSERT_VALID (this);

	CBCGPRibbonBar* pRibbonBar = GetParentRibbonBar ();
	ASSERT_VALID (pRibbonBar);

	pRibbonBar->m_dwHideFlags = bShow ? 0 : BCGPRIBBONBAR_HIDE_ELEMENTS;

	for (int i = 0; i < m_arPanels.GetSize (); i++)
	{
		CBCGPRibbonPanel* pPanel = m_arPanels [i];
		ASSERT_VALID (pPanel);

		pPanel->OnShow (bShow);
	}

	pRibbonBar->GetParentFrame ()->RecalcLayout ();
}
//*************************************************************************************
CBCGPBaseRibbonElement* CBCGPRibbonCategory::FindByID (UINT uiCmdID,
													   BOOL bVisibleOnly,
													   BOOL bNonCustomOnly) const
{
	ASSERT_VALID (this);

	int i = 0;

	if (!bVisibleOnly)
	{
		for (i = 0; i < m_arElements.GetSize (); i++)
		{
			CBCGPBaseRibbonElement* pElem = m_arElements [i];
			ASSERT_VALID (pElem);

			if (pElem->GetID () == uiCmdID)
			{
				return pElem;
			}
		}
	}

	for (i = 0; i < m_arPanels.GetSize (); i++)
	{
		CBCGPRibbonPanel* pPanel = m_arPanels [i];
		ASSERT_VALID (pPanel);

		CBCGPBaseRibbonElement* pElem = pPanel->FindByID (uiCmdID, bNonCustomOnly);
		if (pElem != NULL)
		{
			ASSERT_VALID (pElem);
			return pElem;
		}
	}

	return NULL;
}
//*************************************************************************************
CBCGPBaseRibbonElement* CBCGPRibbonCategory::FindByData (DWORD_PTR dwData,
													     BOOL bVisibleOnly) const
{
	ASSERT_VALID (this);

	int i = 0;

	for (i = 0; i < m_arPanels.GetSize (); i++)
	{
		CBCGPRibbonPanel* pPanel = m_arPanels [i];
		ASSERT_VALID (pPanel);

		CBCGPBaseRibbonElement* pElem = pPanel->FindByData (dwData);
		if (pElem != NULL)
		{
			ASSERT_VALID (pElem);
			return pElem;
		}
	}

	if (!bVisibleOnly)
	{
		for (i = 0; i < m_arElements.GetSize (); i++)
		{
			CBCGPBaseRibbonElement* pElem = m_arElements [i];
			ASSERT_VALID (pElem);

			if (pElem->GetData () == dwData)
			{
				return pElem;
			}
		}
	}

	return NULL;
}
//*************************************************************************************
void CBCGPRibbonCategory::GetElementsByID (UINT uiCmdID, 
	CArray<CBCGPBaseRibbonElement*, CBCGPBaseRibbonElement*>& arButtons)
{
	ASSERT_VALID (this);

	int i = 0;

	for (i = 0; i < m_arPanels.GetSize (); i++)
	{
		CBCGPRibbonPanel* pPanel = m_arPanels [i];
		ASSERT_VALID (pPanel);

		pPanel->GetElementsByID (uiCmdID, arButtons);
	}

	for (i = 0; i < m_arElements.GetSize (); i++)
	{
		CBCGPBaseRibbonElement* pElem = m_arElements [i];
		ASSERT_VALID (pElem);

		if (pElem->GetID () == uiCmdID)
		{
			arButtons.Add (pElem);
		}
	}
}
//*************************************************************************************
void CBCGPRibbonCategory::GetElementsByName (LPCTSTR lpszName, 
	CArray<CBCGPBaseRibbonElement*, CBCGPBaseRibbonElement*>& arButtons, DWORD dwFlags)
{
	ASSERT_VALID (this);

	int i = 0;

	for (i = 0; i < m_arPanels.GetSize (); i++)
	{
		CBCGPRibbonPanel* pPanel = m_arPanels [i];
		ASSERT_VALID (pPanel);

		pPanel->GetElementsByName (lpszName, arButtons, dwFlags);
	}

	for (i = 0; i < m_arElements.GetSize (); i++)
	{
		CBCGPBaseRibbonElement* pElem = m_arElements [i];
		ASSERT_VALID (pElem);

		pElem->GetElementsByName (lpszName, arButtons, dwFlags);
	}
}
//*************************************************************************************
void CBCGPRibbonCategory::GetVisibleElements (
	CArray<CBCGPBaseRibbonElement*, CBCGPBaseRibbonElement*>& arButtons)
{
	ASSERT_VALID (this);

	for (int i = 0; i < m_arPanels.GetSize (); i++)
	{
		CBCGPRibbonPanel* pPanel = m_arPanels [i];
		ASSERT_VALID (pPanel);

		pPanel->GetVisibleElements (arButtons);
	}
}
//*************************************************************************************
void CBCGPRibbonCategory::GetItemIDsList (CList<UINT,UINT>& lstItems, BOOL bHiddenOnly) const
{
	ASSERT_VALID (this);

	int i = 0;

	if (!bHiddenOnly)
	{
		for (i = 0; i < m_arPanels.GetSize (); i++)
		{
			CBCGPRibbonPanel* pPanel = m_arPanels [i];
			ASSERT_VALID (pPanel);

			pPanel->GetItemIDsList (lstItems);
		}
	}

	for (i = 0; i < m_arElements.GetSize (); i++)
	{
		CBCGPBaseRibbonElement* pElem = m_arElements [i];
		ASSERT_VALID (pElem);

		pElem->GetItemIDsList (lstItems);
	}
}
//*************************************************************************************
CBCGPRibbonPanel* CBCGPRibbonCategory::FindPanelWithElem (const CBCGPBaseRibbonElement* pElement)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pElement);

	for (int i = 0; i < m_arPanels.GetSize (); i++)
	{
		CBCGPRibbonPanel* pPanel = m_arPanels [i];
		ASSERT_VALID (pPanel);

		if (pPanel->HasElement (pElement))
		{
			return pPanel;
		}
	}

	return NULL;
}
//*************************************************************************************
BOOL CBCGPRibbonCategory::HasPanel(CBCGPRibbonPanel* pPanel)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pPanel);

	for (int i = 0; i < m_arPanels.GetSize (); i++)
	{
		if (m_arPanels [i] == pPanel)
		{
			return TRUE;
		}
	}

	return FALSE;
}
//*************************************************************************************
CBCGPRibbonPanel* CBCGPRibbonCategory::FindPanelByOriginal(CBCGPRibbonPanel* pOriginal)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pOriginal);

	for (int i = 0; i < m_arPanels.GetSize (); i++)
	{
		CBCGPRibbonPanel* pPanel = m_arPanels [i];
		ASSERT_VALID (pPanel);

		if (pPanel->m_pOriginal == pOriginal && !pPanel->m_bToBeDeleted)
		{
			return pPanel;
		}
	}

	return NULL;
}
//*************************************************************************************
void CBCGPRibbonCategory::AddHidden (CBCGPBaseRibbonElement* pElement)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pElement);

	pElement->SetParentCategory (this);
	m_arElements.Add (pElement);
}
//*************************************************************************************
BOOL CBCGPRibbonCategory::OnDrawImage (CDC* pDC, CRect rect, 
		CBCGPBaseRibbonElement* pElement, BOOL bIsLargeImage,
		BOOL nImageIndex, BOOL bCenter)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pElement);

	CBCGPToolBarImages& image = bIsLargeImage ? m_LargeImages : m_SmallImages;

	if (nImageIndex >= image.GetCount ())
	{
		return FALSE;
	}

	CBCGPDrawState ds;

	CPoint ptImage = rect.TopLeft ();
	const CSize sizeImage = GetImageSize (bIsLargeImage);

	if (bCenter)
	{
		ptImage.Offset (
			max (0, (rect.Width () - sizeImage.cx) / 2), 
			max (0, (rect.Height () - sizeImage.cy) / 2));
	}

	image.SetTransparentColor (globalData.clrBtnFace);
	image.PrepareDrawImage (ds);

	image.Draw (pDC, ptImage.x, ptImage.y, nImageIndex, FALSE, 
		pElement->IsDisabled ());

	image.EndDrawImage (ds);
	return TRUE;
}
//*************************************************************************************
CSize CBCGPRibbonCategory::GetImageSize (BOOL bIsLargeImage) const
{
	ASSERT_VALID (this);

	const CBCGPToolBarImages& image = bIsLargeImage ? m_LargeImages : m_SmallImages;
	const CSize sizeImage = image.GetImageSize ();

	return sizeImage;
}
//*************************************************************************************
void CBCGPRibbonCategory::GetElements (
		CArray <CBCGPBaseRibbonElement*, CBCGPBaseRibbonElement*>& arElements)
{
	ASSERT_VALID (this);

	arElements.RemoveAll ();

	int i = 0;

	for (i = 0; i < m_arPanels.GetSize (); i++)
	{
		CBCGPRibbonPanel* pPanel = m_arPanels [i];
		ASSERT_VALID (pPanel);

		pPanel->GetElements (arElements);
	}

	//---------------------
	// Add hidden elements:
	//---------------------
	for (i = 0; i < m_arElements.GetSize (); i++)
	{
		CBCGPBaseRibbonElement* pElem = m_arElements [i];
		ASSERT_VALID (pElem);

		arElements.Add (pElem);
	}
}
//*******************************************************************************
CBCGPBaseRibbonElement* CBCGPRibbonCategory::GetDroppedDown ()
{
	ASSERT_VALID (this);

	for (int i = 0; i < m_arPanels.GetSize (); i++)
	{
		CBCGPRibbonPanel* pPanel = m_arPanels [i];
		ASSERT_VALID (pPanel);

		CBCGPBaseRibbonElement* pElem = pPanel->GetDroppedDown ();
		if (pElem != NULL)
		{
			ASSERT_VALID (pElem);
			return pElem;
		}
	}

	return NULL;
}
//*******************************************************************************
CBCGPBaseRibbonElement* CBCGPRibbonCategory::GetFocused ()
{
	ASSERT_VALID (this);

	for (int i = 0; i < m_arPanels.GetSize (); i++)
	{
		CBCGPRibbonPanel* pPanel = m_arPanels [i];
		ASSERT_VALID (pPanel);

		CBCGPBaseRibbonElement* pElem = pPanel->GetFocused ();
		if (pElem != NULL)
		{
			ASSERT_VALID (pElem);
			return pElem;
		}
	}

	return NULL;
}
//*******************************************************************************
CBCGPBaseRibbonElement* CBCGPRibbonCategory::GetHighlighted ()
{
	ASSERT_VALID (this);

	for (int i = 0; i < m_arPanels.GetSize (); i++)
	{
		CBCGPRibbonPanel* pPanel = m_arPanels [i];
		ASSERT_VALID (pPanel);

		CBCGPBaseRibbonElement* pElem = pPanel->GetHighlighted ();
		if (pElem != NULL)
		{
			ASSERT_VALID (pElem);
			return pElem;
		}
	}

	return NULL;
}
//*******************************************************************************
void CBCGPRibbonCategory::NormalizeFloatingRect (CBCGPRibbonBar* pRibbonBar, CRect& rectCategory)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pRibbonBar);

	CRect rectRibbon;
	pRibbonBar->GetWindowRect (rectRibbon);

	CRect rectScreen;

	MONITORINFO mi;
	mi.cbSize = sizeof (MONITORINFO);
	if (GetMonitorInfo (MonitorFromPoint (rectRibbon.TopLeft (), MONITOR_DEFAULTTONEAREST),
		&mi))
	{
		rectScreen = mi.rcWork;
	}
	else
	{
		::SystemParametersInfo (SPI_GETWORKAREA, 0, &rectScreen, 0);
	}

	rectCategory.right = min (rectCategory.right, rectScreen.right);
	rectCategory.left = max (rectCategory.left, rectScreen.left);
}
//*******************************************************************************
void CBCGPRibbonCategory::ShowFloating (CRect rectFloating)
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pParenrRibbonBar);

	if ((m_pParenrRibbonBar->m_dwHideFlags & BCGPRIBBONBAR_HIDE_ELEMENTS) == 0)
	{
		ASSERT (FALSE);
		return;
	}

	const BOOL bIsRTL = (m_pParenrRibbonBar->GetExStyle () & WS_EX_LAYOUTRTL);

	CBCGPRibbonPanelMenu* pMenu = new CBCGPRibbonPanelMenu (this, rectFloating.Size ());

	m_Tab.SetDroppedDown (pMenu);

	pMenu->Create (m_pParenrRibbonBar, 
		bIsRTL ? rectFloating.right : rectFloating.left, 
		rectFloating.top, (HMENU) NULL);
}
//*******************************************************************************
CBCGPRibbonCategory* CBCGPRibbonCategory::CreateCustomCopy(CBCGPRibbonBar* pParenrRibbonBar)
{
	ASSERT_VALID (this);

	CBCGPRibbonCategory* pCustomCategory = (CBCGPRibbonCategory*)GetRuntimeClass ()->CreateObject();
	ASSERT_VALID(pCustomCategory);

	int i = 0;

	pCustomCategory->m_strName = m_strName;
	pCustomCategory->m_bIsCustom = TRUE;

	for (i = 0; i < m_arPanels.GetSize (); i++)
	{
		CBCGPRibbonPanel* pPanelSrc = m_arPanels [i];
		ASSERT_VALID (pPanelSrc);

		CBCGPRibbonPanel* pPanel = pPanelSrc->CreateCustomCopy(pCustomCategory);
		ASSERT_VALID (pPanel);

		pCustomCategory->m_arPanels.Add (pPanel);

		if (GetParentRibbonBar() != NULL)
		{
			pPanel->m_nKey = GetParentRibbonBar()->m_nNextPanelKey++;
		}
	}

	pCustomCategory->m_pParenrRibbonBar = pParenrRibbonBar;
	pCustomCategory->m_Tab.CopyFrom (m_Tab);
	pCustomCategory->m_Tab.m_pParent = pCustomCategory;

	pCustomCategory->m_dwData = m_dwData;
	pCustomCategory->m_uiContextID = m_uiContextID;
	pCustomCategory->m_bIsVisible = (m_uiContextID == 0);

	pCustomCategory->m_ScrollLeft.CopyFrom (m_ScrollLeft);
	pCustomCategory->m_ScrollLeft.m_pParent = pCustomCategory;
	pCustomCategory->m_ScrollRight.CopyFrom (m_ScrollRight);
	pCustomCategory->m_ScrollRight.m_pParent = pCustomCategory;

	return pCustomCategory;
}
//*******************************************************************************
void CBCGPRibbonCategory::CopyFrom (CBCGPRibbonCategory& src)
{
	ASSERT_VALID (this);

	int i = 0;

	m_strName = src.m_strName;
	m_bIsActive = src.m_bIsActive;
	m_bIsVisible = src.m_bIsVisible;
	m_bIsQATOnly = src.m_bIsQATOnly;
	m_nKey = src.m_nKey;
	m_bIsPrintPreview = src.m_bIsPrintPreview;
	m_bIsCustom = src.m_bIsCustom;

	for (i = 0; i < src.m_arPanels.GetSize (); i++)
	{
		CBCGPRibbonPanel* pPanelSrc = src.m_arPanels [i];
		ASSERT_VALID (pPanelSrc);

		CBCGPRibbonPanel* pPanel = 
			(CBCGPRibbonPanel*) pPanelSrc->GetRuntimeClass ()->CreateObject ();
		ASSERT_VALID (pPanel);

		pPanel->CopyFrom (*pPanelSrc);

		pPanel->m_btnDefault.CopyFrom (pPanelSrc->m_btnDefault);
		pPanel->m_btnDefault.SetOriginal (&pPanelSrc->m_btnDefault);

		m_arPanels.Add (pPanel);
	}

	m_pParenrRibbonBar = src.m_pParenrRibbonBar;
	m_rect = src.m_rect;
	m_Tab.CopyFrom (src.m_Tab);
	m_Tab.m_pParent = this;

	m_dwData = src.m_dwData;
	m_uiContextID = src.m_uiContextID;
	m_pParentMenuBar = src.m_pParentMenuBar;

	src.m_SmallImages.CopyTo (m_SmallImages);
	src.m_LargeImages.CopyTo (m_LargeImages);

	m_arCollapseOrder.RemoveAll ();
	m_arCollapseOrder.Copy (src.m_arCollapseOrder);

	m_ScrollLeft.CopyFrom (src.m_ScrollLeft);
	m_ScrollLeft.m_pParent = this;
	m_ScrollRight.CopyFrom (src.m_ScrollRight);
	m_ScrollRight.m_pParent = this;
}
//***************************************************************************
CBCGPBaseRibbonElement* CBCGPRibbonCategory::GetParentButton () const
{
	ASSERT_VALID (this);

	if (m_pParentMenuBar == NULL)
	{
		return NULL;
	}

	return ((CBCGPPopupMenu*)m_pParentMenuBar->GetParent ())->GetParentRibbonElement ();
}
//***************************************************************************
void CBCGPRibbonCategory::SetKeys (LPCTSTR lpszKeys)
{
	ASSERT_VALID (this);
	ASSERT (lpszKeys != NULL);

	m_Tab.SetKeys (lpszKeys);
}
//***************************************************************************
void CBCGPRibbonCategory::SetName (LPCTSTR lpszName)
{
	ASSERT_VALID (this);

	m_strName = lpszName == NULL ? _T("") : lpszName;

	// Remove '&' characters and build key string:
	CString strKeys;

	for (int i = 0; i < m_strName.GetLength (); i++)
	{
		if (m_strName [i] == _T('&'))
		{
			m_strName.Delete (i);
			
			if (i < m_strName.GetLength ())
			{
				strKeys += m_strName [i];
			}
		}
	}

	m_Tab.SetKeys (strKeys);
}
//***************************************************************************
void CBCGPRibbonCategory::OnRTLChanged (BOOL bIsRTL)
{
	ASSERT_VALID (this);

	int i = 0;

	for (i = 0; i < m_arPanels.GetSize (); i++)
	{
		CBCGPRibbonPanel* pPanel = m_arPanels [i];
		ASSERT_VALID (pPanel);

		pPanel->OnRTLChanged (bIsRTL);
	}

	for (i = 0; i < m_arElements.GetSize (); i++)
	{
		CBCGPBaseRibbonElement* pElem = m_arElements [i];
		ASSERT_VALID (pElem);

		pElem->OnRTLChanged (bIsRTL);
	}

	m_nLastCategoryWidth = -1;
}
//***************************************************************************
int CBCGPRibbonCategory::GetTextTopLine () const
{
	ASSERT_VALID (this);

	const CSize sizeImageLarge = GetImageSize (TRUE);
	if (sizeImageLarge == CSize (0, 0))
	{
		return -1;
	}

	return sizeImageLarge.cy + 5;
}
//***************************************************************************
void CBCGPRibbonCategory::SetCollapseOrder (const CArray<int, int>& arCollapseOrder)
{
	ASSERT_VALID (this);

	m_arCollapseOrder.RemoveAll ();
	m_arCollapseOrder.Copy (arCollapseOrder);
}
//***************************************************************************
BOOL CBCGPRibbonCategory::SetPanelsLayout (int nWidth)
{
	CBCGPRibbonBar* pRibbonBar = GetParentRibbonBar ();
	ASSERT_VALID (pRibbonBar);

	int nTotalWidth = 0;

	for (int i = 0; i < m_arPanels.GetSize (); i++)
	{
		CBCGPRibbonPanel* pPanel = m_arPanels [i];
		ASSERT_VALID (pPanel);

		if (pRibbonBar->m_CustomizationData.IsPanelHidden(pPanel))
		{
			continue;
		}

		pPanel->m_bForceCollpapse = FALSE;

		if (pPanel->m_nCurrWidthIndex == pPanel->m_arWidths.GetSize () - 1 &&
			pPanel->m_arWidths.GetSize () > 1 && !pPanel->IsNonCollapsible())
		{
			pPanel->m_bForceCollpapse = TRUE;
		}

		const int nCurrPanelWidth = 
			pPanel->m_arWidths [pPanel->m_nCurrWidthIndex] + 
			2 * pPanel->m_nXMargin;

		nTotalWidth += nCurrPanelWidth + CBCGPVisualManager::GetInstance()->GetRibbonPanelMargin(this);

		if (nTotalWidth > nWidth)
		{
			return FALSE;
		}
	}

	return TRUE;
}
//***************************************************************************
void CBCGPRibbonCategory::ResetPanelsLayout ()
{
	//--------------------------
	// all panels in max. width:
	//--------------------------
	for (int i = 0; i < m_arPanels.GetSize (); i++)
	{
		CBCGPRibbonPanel* pPanel = m_arPanels [i];
		ASSERT_VALID (pPanel);

		pPanel->m_nCurrWidthIndex = 0;
		pPanel->m_bTrancateCaption = FALSE;
	}

	m_nScrollOffset = 0;
}
//***************************************************************************
BOOL CBCGPRibbonCategory::OnScrollHorz (BOOL bScrollLeft, int nScrollOffset/* = 0*/)
{
	ASSERT_VALID(this);

	const int nPrevScrollOffset = m_nScrollOffset;

	if (nScrollOffset == 0)
	{
		nScrollOffset = 50;
	}

	if (bScrollLeft)
	{
		m_nScrollOffset -= nScrollOffset;
	}
	else
	{
		m_nScrollOffset += nScrollOffset;
	}

	m_nScrollOffset = max(0, min (m_nMinWidth - GetClientRect().Width (), m_nScrollOffset));

	CBCGPRibbonBar* pRibbonBar = GetParentRibbonBar ();
	ASSERT_VALID (pRibbonBar);

	CClientDC dc (pRibbonBar);

	CFont* pOldFont = dc.SelectObject (pRibbonBar->GetFont ());
	ASSERT (pOldFont != NULL);

	ReposPanels (&dc);

	dc.SelectObject (pOldFont);

	UpdateScrollButtons ();

	if (m_pParentMenuBar != NULL)
	{
		ASSERT_VALID (m_pParentMenuBar);
		m_pParentMenuBar->RedrawWindow ();
	}
	else
	{
		pRibbonBar->RedrawWindow (m_rect);
	}

	return nPrevScrollOffset != m_nScrollOffset;
}
//***************************************************************************
void CBCGPRibbonCategory::EnsureVisible (CBCGPRibbonButton* pButton)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pButton);

	CRect rectClient = m_rect;
	rectClient.DeflateRect (nPanelMarginLeft * 2, CBCGPVisualManager::GetInstance()->GetRibbonPanelMarginTop(), 
		nPanelMarginRight * 2, nPanelMarginBottom);

	if (m_rect.IsRectEmpty ())
	{
		return;
	}

	CRect rectButton = pButton->GetRect ();
	if (rectButton.IsRectEmpty ())
	{
		CBCGPRibbonPanel* pParentPanel = pButton->GetParentPanel ();
		if (pParentPanel == NULL)
		{
			return;
		}

		ASSERT_VALID(pParentPanel);
		ASSERT(pParentPanel->GetRect ().IsRectEmpty ());

		int nPanelIndex = -1;
		int nFirstVisiblePanel = -1;
		int nLastVisiblePanel = -1;
		int i = 0;

		for (i = 0; i < m_arPanels.GetSize (); i++)
		{
			CBCGPRibbonPanel* pPanel = m_arPanels [i];
			ASSERT_VALID (pPanel);

			if (pPanel == pParentPanel)
			{
				nPanelIndex = i;
			}

			if (!pPanel->GetRect ().IsRectEmpty ())
			{
				if (nFirstVisiblePanel < 0)
				{
					nFirstVisiblePanel = i;
				}

				nLastVisiblePanel = i;
			}
		}

		if (nPanelIndex == -1 || nFirstVisiblePanel == -1 || nLastVisiblePanel == -1)
		{
			return;
		}

		if (nPanelIndex < nFirstVisiblePanel)
		{
			while (OnScrollHorz (TRUE))
			{
				if (!pParentPanel->GetRect ().IsRectEmpty () &&
					pParentPanel->GetRect ().left >= rectClient.left)
				{
					break;
				}
			}
		}
		else if (nPanelIndex > nLastVisiblePanel)
		{
			while (OnScrollHorz (FALSE))
			{
				if (!pParentPanel->GetRect ().IsRectEmpty () &&
					pParentPanel->GetRect ().right <= rectClient.right)
				{
					break;
				}
			}
		}

		return;
	}

	if (rectButton.left < m_rect.left - nPanelMarginRight)
	{
		OnScrollHorz (TRUE, rectClient.left - rectButton.left);
	}
	else if (rectButton.right > m_rect.right + nPanelMarginRight)
	{
		OnScrollHorz (FALSE, rectButton.right - rectClient.right);
	}
}
//***************************************************************************
BOOL CBCGPRibbonCategory::OnKey (UINT nChar)
{
	ASSERT_VALID(this);

	CBCGPBaseRibbonElement* pFocused = NULL;
	CBCGPBaseRibbonElement* pFocusedNew = NULL;

	switch (nChar)
	{
	case VK_LEFT:
	case VK_RIGHT:
	case VK_DOWN:
	case VK_UP:
	case VK_TAB:
		{
			CArray<CBCGPBaseRibbonElement*, CBCGPBaseRibbonElement*> arElems;
			GetVisibleElements (arElems);

			if (arElems.GetSize () == 0)
			{
				return FALSE;
			}

			if ((pFocused = GetFocused ()) == NULL)
			{
				for (int i = 0; i < arElems.GetSize (); i++)
				{
					CBCGPBaseRibbonElement* pElem = arElems [i];
					ASSERT_VALID (pElem);

					if (pElem->IsTabStop () && !pElem->GetRect ().IsRectEmpty ())
					{
						pFocusedNew = pElem;
						break;
					}
				}
			}
			else
			{
				ASSERT_VALID (pFocused);

				int nScroll = 0;

				BOOL bIsScrollLeftAvailable = !m_ScrollLeft.GetRect ().IsRectEmpty ();
				BOOL bIsScrollRightAvailable = !m_ScrollRight.GetRect ().IsRectEmpty ();

				pFocusedNew = CBCGPRibbonBar::FindNextFocusedElement (
					nChar, arElems, m_rect, pFocused,
					bIsScrollLeftAvailable, bIsScrollRightAvailable, nScroll);

				if (nScroll != 0)
				{
					switch (nScroll)
					{
					case -2:
						pFocusedNew = GetFirstVisibleElement ();
						break;

					case 2:
						pFocusedNew = GetLastVisibleElement ();
						break;

					case -1:
					case 1:
						OnScrollHorz (nScroll < 0);
					}
				}
			}
		}
		break;

	case VK_RETURN:
	case VK_SPACE:
		if ((pFocused = GetFocused ()) != NULL)
		{
			ASSERT_VALID (pFocused);
			pFocused->OnKey (FALSE);

			return TRUE;
		}
		return FALSE;;

	default:
		return FALSE;
	}

	if (pFocusedNew == pFocused)
	{
		return TRUE;
	}

	if (pFocusedNew == NULL)
	{
		return FALSE;
	}

	if (m_pParenrRibbonBar != NULL)
	{
		ASSERT_VALID (m_pParenrRibbonBar);
		m_pParenrRibbonBar->DeactivateKeyboardFocus (FALSE);
	}

	if (pFocused != NULL)
	{
		pFocused->m_bIsHighlighted = pFocused->m_bIsFocused = FALSE;
		pFocused->OnSetFocus (FALSE);
		pFocused->Redraw ();
	}

	ASSERT_VALID (pFocusedNew);

	pFocusedNew->m_bIsFocused = TRUE;
	pFocusedNew->OnSetFocus (TRUE);
	pFocusedNew->Redraw ();

	if (globalData.IsAccessibilitySupport () && m_pParentMenuBar != NULL)
	{

		CArray<CBCGPBaseRibbonElement*, CBCGPBaseRibbonElement*> arButtons;
		GetVisibleElements (arButtons);

		for (int i = 0; i < arButtons.GetSize (); i++)
		{
			CBCGPBaseRibbonElement* pElem = arButtons [i];
			ASSERT_VALID (pElem);

			if (pElem == pFocusedNew)
			{
				globalData.NotifyWinEvent (EVENT_OBJECT_FOCUS, m_pParentMenuBar->GetSafeHwnd (), OBJID_CLIENT , i + 1);
				break;
			}
		}
	}

	return TRUE;
}

//***************************************************************************
BOOL CBCGPRibbonCategory::IsScenicLook () const
{
	CBCGPRibbonBar* pBar = GetParentRibbonBar ();
	if (pBar == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	return pBar->IsScenicLook ();
}
//*************************************************************************************
void CBCGPRibbonCategory::OnChangeVisualManager()
{
	ASSERT_VALID (this);

	int i = 0;

	for (i = 0; i < m_arPanels.GetSize (); i++)
	{
		CBCGPRibbonPanel* pPanel = m_arPanels [i];
		ASSERT_VALID (pPanel);

		pPanel->OnChangeVisualManager();
	}

	for (i = 0; i < m_arElements.GetSize (); i++)
	{
		CBCGPBaseRibbonElement* pElem = m_arElements [i];
		ASSERT_VALID (pElem);

		pElem->OnChangeVisualManager();
	}
}
//*************************************************************************************
CBCGPRibbonMainPanel* CBCGPRibbonCategory::GetMainPanel()
{
	ASSERT_VALID (this);

	if (GetPanelCount () == 1)
	{
		return DYNAMIC_DOWNCAST (CBCGPRibbonMainPanel, GetPanel (0));
	}

	return NULL;
}
//*************************************************************************************
CString CBCGPRibbonCategory::GetDisplayName (BOOL bCheckForStyle) const
{
	CString strName = m_strName;

	CBCGPRibbonBar* pRibbonBar = GetParentRibbonBar();
	if (pRibbonBar != NULL)
	{
		CString strCustomName;
		if (pRibbonBar->m_CustomizationData.GetTabName(this, strCustomName))
		{
			strName = strCustomName;
		}
	}

	if (strName.IsEmpty())
	{
		strName = _T(" ");
	}

	if (bCheckForStyle && CBCGPVisualManager::GetInstance()->IsTopLevelMenuItemUpperCase())
	{
		strName.MakeUpper();
	}

	return strName;
}
//*************************************************************************************
void CBCGPRibbonCategory::OnCloseCustomizePage(BOOL bIsOK)
{
	if (!bIsOK)
	{
		m_bToBeDeleted = FALSE;
	}

	for (int i = 0; i < m_arPanels.GetSize (); i++)
	{
		CBCGPRibbonPanel* pPanel = m_arPanels [i];
		ASSERT_VALID (pPanel);

		if (pPanel->m_bIsNew && !bIsOK)
		{
			delete pPanel;
			m_arPanels.RemoveAt(i);

			i--;
		}
		else
		{
			pPanel->m_bIsNew = FALSE;
			pPanel->OnCloseCustomizePage(bIsOK);
		}
	}
}
//*************************************************************************************
void CBCGPRibbonCategory::SetCustom()
{
	m_bIsCustom = TRUE;

	for (int i = 0; i < m_arPanels.GetSize (); i++)
	{
		CBCGPRibbonPanel* pPanel = m_arPanels [i];
		ASSERT_VALID (pPanel);

		pPanel->SetCustom();
	}
}
//*************************************************************************************
void CBCGPRibbonCategory::ApplyCustomizationData(CBCGPRibbonCustomCategory& customCategory)
{
	CBCGPRibbonBar* pRibbonBar = GetParentRibbonBar ();
	ASSERT_VALID (pRibbonBar);

	if (m_bIsCustom)
	{
		m_uiContextID = customCategory.m_uiContextID;
		m_bIsVisible = (m_uiContextID == 0);

		SetTabColor(pRibbonBar->GetContextColor(m_uiContextID));
	}

	for (int i = 0; i < m_arPanels.GetSize (); i++)
	{
		CBCGPRibbonPanel* pPanel = m_arPanels [i];
		ASSERT_VALID (pPanel);

		if (pPanel->IsCustom() || pPanel->m_bToBeDeleted)
		{
			CBCGPRibbonCustomPanel* pCustomPanel = customCategory.FindCustomPanel(pPanel->GetKey());
			if (pCustomPanel == NULL || pPanel->m_bToBeDeleted)
			{
				RemovePanel(i);
				i--;
			}
			else
			{
				pPanel->ApplyCustomizationData(pRibbonBar, *pCustomPanel);
			}
		}
	}
}
//**************************************************************************************
BOOL CBCGPRibbonCategory::OnSetAccData (long lVal)
{
	ASSERT_VALID(this);

	m_AccData.Clear();

	int nIndex = (int)lVal - 1;

	if (nIndex < 0 || nIndex >= m_arPanels.GetSize())
	{
		return FALSE;
	}

	ASSERT_VALID(m_arPanels[nIndex]);
	return m_arPanels[nIndex]->SetACCData(m_pParenrRibbonBar, m_AccData);
}
//**************************************************************************************
HRESULT CBCGPRibbonCategory::get_accParent(IDispatch **ppdispParent)
{
    if (!ppdispParent)
    {
        return E_INVALIDARG;
    }

    *ppdispParent = NULL;

    CBCGPRibbonBar* pRibbonBar = GetParentRibbonBar();
    if (pRibbonBar->GetSafeHwnd() == NULL)
    {
        return S_FALSE;
    }

	ASSERT_VALID (pRibbonBar);

    LPDISPATCH lpDispatch = pRibbonBar->GetAccessibleDispatch();
	if (lpDispatch != NULL)
	{
		*ppdispParent =  lpDispatch;
		return S_OK;
	}

    return S_OK;
}
//*******************************************************************************
HRESULT CBCGPRibbonCategory::get_accChildCount(long *pcountChildren)
{
	if (!pcountChildren)
    {
        return E_INVALIDARG;
    }

	*pcountChildren = (long)m_arPanels.GetSize(); 
	return S_OK;
}
//*******************************************************************************
HRESULT CBCGPRibbonCategory::get_accChild(VARIANT varChild, IDispatch **ppdispChild)
{
	if (!ppdispChild)
    {
        return E_INVALIDARG;
    }

	if (varChild.vt == VT_I4)
	{
		int nIndex = varChild.lVal -1;
		CBCGPRibbonPanel* pPanel = m_arPanels[nIndex];
		if (pPanel == NULL)
		{
			return S_FALSE;
		}
		
		ASSERT_VALID(pPanel);

		*ppdispChild = pPanel->GetIDispatch(TRUE);
		if (*ppdispChild)
		{
			return S_OK;
		}
	}

	return S_FALSE;
}
//*******************************************************************************
HRESULT CBCGPRibbonCategory::accLocation(long *pxLeft, long *pyTop, long *pcxWidth, long *pcyHeight, VARIANT varChild)
{
    if (!pxLeft || !pyTop || !pcxWidth || !pcyHeight)
    {
        return E_INVALIDARG;
    }

	if (varChild.vt == VT_I4 && varChild.lVal == CHILDID_SELF && m_pParenrRibbonBar->GetSafeHwnd() != NULL)
	{	
		CRect panelRect = m_rect;
		m_pParenrRibbonBar->ClientToScreen(&panelRect);

		*pxLeft = panelRect.left;
		*pyTop = panelRect.top;
		*pcxWidth = panelRect.Width();
		*pcyHeight = panelRect.Height();

		return S_OK;
	}

	if (varChild.vt == VT_I4 && varChild.lVal > 0)
	{
		OnSetAccData(varChild.lVal);

		*pxLeft = m_AccData.m_rectAccLocation.left;
		*pyTop = m_AccData.m_rectAccLocation.top;
		*pcxWidth = m_AccData.m_rectAccLocation.Width();
		*pcyHeight = m_AccData.m_rectAccLocation.Height();
	}

	return S_OK;
}
//*******************************************************************************
HRESULT CBCGPRibbonCategory::accNavigate(long navDir, VARIANT varStart, VARIANT* pvarEndUpAt)
{
	pvarEndUpAt->vt = VT_EMPTY;

    if (varStart.vt != VT_I4)
    {
        return E_INVALIDARG;
    }

	switch (navDir)
	{
	case NAVDIR_FIRSTCHILD:
		if (varStart.lVal == CHILDID_SELF)
		{
			pvarEndUpAt->vt = VT_I4;
			pvarEndUpAt->lVal = 1;
			return S_OK;	
		}
		break;

	case NAVDIR_LASTCHILD:
		if (varStart.lVal == CHILDID_SELF)
		{
			pvarEndUpAt->vt = VT_I4;
			pvarEndUpAt->lVal = (int)m_arPanels.GetSize();
			return S_OK;
		}
		break;

	case NAVDIR_NEXT:   
	case NAVDIR_RIGHT:
		if (varStart.lVal != CHILDID_SELF)
		{
			pvarEndUpAt->vt = VT_I4;
			pvarEndUpAt->lVal = varStart.lVal + 1;

			if (pvarEndUpAt->lVal > (int)m_arPanels.GetSize())
			{
				pvarEndUpAt->vt = VT_EMPTY;
				return S_FALSE;
			}

			return S_OK;
		}
		break;

	case NAVDIR_PREVIOUS: 
	case NAVDIR_LEFT:
		if (varStart.lVal != CHILDID_SELF)
		{
			pvarEndUpAt->vt = VT_I4;
			pvarEndUpAt->lVal = varStart.lVal - 1;
		
			if (pvarEndUpAt->lVal <= 0)
			{
				pvarEndUpAt->vt = VT_EMPTY;
				return S_FALSE;
			}

			return S_OK;
		}
		else if (m_pParenrRibbonBar->GetSafeHwnd() != NULL)
		{
			int nCount = m_pParenrRibbonBar->m_TabElements.GetCount();
			if (nCount > 0)
			{
				CBCGPBaseRibbonElement* pTabElement = m_pParenrRibbonBar->m_TabElements.GetButton(nCount-1);
				if (pTabElement != NULL)
				{
					ASSERT_VALID(pTabElement);

					pvarEndUpAt->vt = VT_DISPATCH;
					pvarEndUpAt->pdispVal = pTabElement->GetIDispatch(TRUE);

					return S_OK;
				}
			}
			else if (m_pParenrRibbonBar->GetTabs() != NULL)
			{
				CBCGPRibbonTabsGroup* pTabs = m_pParenrRibbonBar->GetTabs();
				if (pTabs != NULL)
				{
					ASSERT_VALID(pTabs);

					pvarEndUpAt->vt = VT_DISPATCH;
					pvarEndUpAt->pdispVal = pTabs->GetIDispatch(TRUE);

					return S_OK;
				}
			}
		}
		break;
	}

	return S_FALSE;
}
//*******************************************************************************
HRESULT CBCGPRibbonCategory::accHitTest(long xLeft, long yTop, VARIANT *pvarChild)
{
	if (!pvarChild)
    {
        return E_INVALIDARG;
    }

	if (m_pParenrRibbonBar->GetSafeHwnd() == NULL)
	{
		return S_FALSE;
	}

	pvarChild->vt = VT_I4;
	pvarChild->lVal = CHILDID_SELF;

	CPoint pt(xLeft, yTop);
	m_pParenrRibbonBar->ScreenToClient(&pt);

	for (int i = 0; i < m_arPanels.GetSize(); i++)
	{
		CBCGPRibbonPanel* pPanel = m_arPanels[i];
		if (pPanel == NULL)
		{
			continue;
		}

		ASSERT_VALID(pPanel);

		if (pPanel->GetRect().PtInRect(pt))
		{
			IDispatch* pDispatch = pPanel->GetIDispatch(TRUE);
			if (pDispatch)
			{
				pPanel->SetACCData(m_pParenrRibbonBar, m_AccData);

				pvarChild->vt = VT_DISPATCH;
			    pvarChild->pdispVal = pDispatch;

			    return S_OK;
			}
		}
	}

	return S_FALSE;
}
//*******************************************************************************
BOOL CBCGPRibbonCategory::SetACCData(CWnd* /*pParent*/, CBCGPAccessibilityData& data)
{
	ASSERT_VALID(this);

	data.m_strAccName = m_strName;
	data.m_nAccRole = ROLE_SYSTEM_TOOLBAR;
	data.m_strAccValue = _T("Group");
	data.m_rectAccLocation = GetRect();

	if (m_pParenrRibbonBar->GetSafeHwnd() != NULL)
	{
		m_pParenrRibbonBar->ClientToScreen(&data.m_rectAccLocation);
	}

	data.m_bAccState = 0;
	return TRUE;
}
//*******************************************************************************
void CBCGPRibbonCategory::SetPadding(const CSize& size)
{
	ASSERT_VALID(this);

	m_Tab.SetPadding(size);

	int i = 0;

	for (i = 0; i < m_arPanels.GetSize(); i++)
	{
		CBCGPRibbonPanel* pPanel = m_arPanels[i];
		if (pPanel == NULL)
		{
			continue;
		}
		
		ASSERT_VALID(pPanel);
		pPanel->SetPadding(size);
	}

	for (i = 0; i < m_arElements.GetSize(); i++)
	{
		CBCGPBaseRibbonElement* pElem = m_arElements [i];
		ASSERT_VALID (pElem);

		pElem->SetPadding(size);
	}
}
//*******************************************************************************
CRect CBCGPRibbonCategory::GetClientRect() const
{
	CRect rect = m_rect;

	rect.left += m_ScrollLeft.GetRect().Width();
	rect.right -= m_ScrollRight.GetRect().Width();

	return rect;
}

#endif // BCGP_EXCLUDE_RIBBON
