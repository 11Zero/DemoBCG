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
// BCGPBaseRibbonElement.cpp: implementation of the CBCGPBaseRibbonElement class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BCGPBaseRibbonElement.h"

#ifndef BCGP_EXCLUDE_RIBBON

#include "BCGPRibbonCategory.h"
#include "BCGPRibbonBar.h"
#include "BCGPRibbonStatusBar.h"
#include "BCGPPopupMenu.h"
#include "BCGPRibbonPanelMenu.h"
#include "BCGPKeyboardManager.h"
#include "BCGPVisualManager.h"
#include "BCGPRibbonCommandsListBox.h"
#include "BCGPRibbonQuickAccessToolbar.h"
#include "BCGPLocalResource.h"
#include "BCGPRibbonKeyTip.h"
#include "bcgprores.h"
#include "MenuImages.h"
#include "BCGPMDIFrameWnd.h"
#include "BCGPGridCtrl.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

static const CString strDummyAmpSeq = _T("\001\001");

static inline BOOL IsSystemCommand (UINT uiCmd)
{
	return (uiCmd >= 0xF000 && uiCmd < 0xF1F0);
}

//////////////////////////////////////////////////////////////////////
// CBCGPRibbonSeparator

IMPLEMENT_DYNCREATE (CBCGPRibbonSeparator, CBCGPBaseRibbonElement)

CBCGPRibbonSeparator::CBCGPRibbonSeparator (BOOL bIsHoriz)
{
	m_bIsHoriz = bIsHoriz;
}
//******************************************************************************
CSize CBCGPRibbonSeparator::GetRegularSize (CDC* /*pDC*/)
{
	ASSERT_VALID (this);
	return CSize (4, 4);
}
//******************************************************************************
void CBCGPRibbonSeparator::CopyFrom (const CBCGPBaseRibbonElement& s)
{
	ASSERT_VALID (this);

	CBCGPBaseRibbonElement::CopyFrom (s);

	const CBCGPRibbonSeparator& src = (const CBCGPRibbonSeparator&) s;
	m_bIsHoriz = src.m_bIsHoriz;
}
//******************************************************************************
void CBCGPRibbonSeparator::OnDraw (CDC* pDC)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);

	if (m_rect.IsRectEmpty ())
	{
		return;
	}

	CRect rect = m_rect;

	if (m_bQuickAccessMode)
	{
		rect.left = rect.CenterPoint ().x - 1;
		rect.right = rect.left + 2;

		rect.DeflateRect (0, 3);

		CBCGPVisualManager::GetInstance ()->OnDrawRibbonQATSeparator (pDC, this, rect);
		return;
	}

	if (m_bIsHoriz)
	{
		rect.top = rect.CenterPoint ().y;
		rect.bottom = rect.top + 1;
	}
	else
	{
		rect.left = rect.CenterPoint ().x;
		rect.right = rect.left + 1;

		rect.DeflateRect (0, 5);
	}

	CBCGPBaseControlBar* pParentBar = NULL;

	if (m_pParentMenu != NULL)
	{
		pParentBar = m_pParentMenu;
	}
	else
	{
		pParentBar = GetTopLevelRibbonBar ();
	}

	if (pParentBar != NULL)
	{
		BOOL bDisableSideBarInXPMode = FALSE;

		if (m_bIsDefaultMenuLook && m_pParentMenu != NULL)
		{
			bDisableSideBarInXPMode = m_pParentMenu->m_bDisableSideBarInXPMode;

			rect.left += 2 * CBCGPVisualManager::GetInstance ()->GetMenuImageMargin () + 2;

			m_pParentMenu->m_bDisableSideBarInXPMode = FALSE;
		}

		CBCGPVisualManager::GetInstance ()->OnDrawSeparator (pDC, 
			pParentBar, rect, !m_bIsHoriz);

		if (m_pParentMenu != NULL)
		{
			m_pParentMenu->m_bDisableSideBarInXPMode = bDisableSideBarInXPMode;
		}
	}
}
//*************************************************************************************
int CBCGPRibbonSeparator::AddToListBox (CBCGPRibbonCommandsListBox* pWndListBox, BOOL /*bDeep*/)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pWndListBox);
	ASSERT (pWndListBox->GetSafeHwnd () != NULL);

	CString strText;

	const CBCGPRibbonBar* pRibbonBar = pWndListBox->GetRibbonBar();
	if (pRibbonBar == NULL || !pRibbonBar->GetSeparatorCustomLabel(strText))
	{
		CBCGPLocalResource locaRes;
		strText.LoadString (IDS_BCGBARRES_QAT_SEPARATOR);
	}

	int nIndex = pWndListBox->AddString (_T(" ") + strText);	// Should be always first!
	pWndListBox->SetItemData (nIndex, (DWORD_PTR) this);

	return nIndex;
}
//*************************************************************************************
void CBCGPRibbonSeparator::OnDrawOnList (CDC* pDC, CString strText, int nTextOffset, CRect rect, BOOL /*bIsSelected*/, BOOL /*bHighlighted*/)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);

	BOOL bIsDisabled = m_bIsDisabled;
	m_bIsDisabled = FALSE;

	const int xMargin = 3;

	rect.DeflateRect (xMargin, 0);
	rect.left += nTextOffset;

	pDC->DrawText (strText, rect, DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

	m_bIsDisabled = bIsDisabled;
}

//////////////////////////////////////////////////////////////////////
// CBCGPBaseRibbonElement

IMPLEMENT_DYNAMIC(CBCGPBaseRibbonElement, CBCGPBaseAccessibleObject)

CBCGPBaseRibbonElement::CBCGPBaseRibbonElement ()
{
	m_Location = RibbonElementNotInGroup;
	m_sizePadding = CSize(0, 0);
	m_nID = 0;
	m_dwData = 0;
	m_bIsCustom = FALSE;
	m_nCustomImageIndex = -1;
	m_bIsNew = FALSE;
	m_rect.SetRectEmpty ();
	m_pParent = NULL;
	m_pParentGroup = NULL;
	m_pParentMenu = NULL;
	m_bCompactMode = FALSE;
	m_bIntermediateMode = FALSE;
	m_bFloatyMode = FALSE;
	m_bQuickAccessMode = FALSE;
	m_bStatusBarMode = FALSE;
	m_bSearchResultMode = FALSE;
	m_bIsHighlighted = FALSE;
	m_bIsFocused = FALSE;
	m_bIsPressed = FALSE;
	m_bIsDisabled = FALSE;
	m_bIsChecked = FALSE;
	m_bIsRadio = FALSE;
	m_bIsDroppedDown = FALSE;
	m_pOriginal = NULL;
	m_pRibbonBar = NULL;
	m_nRow = -1;
	m_bDontNotify = FALSE;
	m_bTextAlwaysOnRight = FALSE;
	m_pPopupMenu = NULL;
	m_nImageOffset = 0;
	m_bShowGroupBorder = FALSE;
	m_bIsVisible = TRUE;
	m_bIsDefaultMenuLook = FALSE;
	m_bIsAlwaysLarge = FALSE;
	m_bDrawDefaultIcon = TRUE;
	m_bIsOnPaletteTop = FALSE;
	m_bOnBeforeShowItemMenuIsSent = FALSE;
	m_bIsTabElement = FALSE;
	m_bEnableUpdateTooltipInfo   = TRUE;
	m_bEnableTooltipInfoShortcut = TRUE;
	m_bOnDialogBar = FALSE;
	m_bCanBeStretchedOnDialogBar = FALSE;
	m_nTextGlassGlowSize = 10;
	m_bIsBackstageViewMode = FALSE;
	m_nExtraMaginX = 0;
	m_pWndTargetCmd = NULL;
	m_bForceDrawDisabledOnList = FALSE;
}
//******************************************************************************
CBCGPBaseRibbonElement::~CBCGPBaseRibbonElement ()
{
	if (m_pPopupMenu != NULL)
	{
		ASSERT_VALID (m_pPopupMenu);
		ASSERT (m_pPopupMenu->m_pParentRibbonElement == this);

		m_pPopupMenu->m_pParentRibbonElement = NULL;

		ClosePopupMenu ();
	}
}
//******************************************************************************
void CBCGPBaseRibbonElement::SetID (UINT nID)
{
	ASSERT_VALID (this);
	m_nID = nID;
}
//******************************************************************************
void CBCGPBaseRibbonElement::SetText (LPCTSTR lpszText)
{
	ASSERT_VALID (this);
	m_strText = lpszText == NULL ? _T("") : lpszText;

	int nIndex = m_strText.Find (_T('\n'));
	if (nIndex >= 0)
	{
		m_strKeys = m_strText.Mid (nIndex + 1);
		m_strText = m_strText.Left (nIndex);
	}

	m_strText.TrimLeft ();
	m_strText.TrimRight ();
}
//******************************************************************************
void CBCGPBaseRibbonElement::SetKeys (LPCTSTR lpszKeys, LPCTSTR lpszMenuKeys)
{
	ASSERT_VALID (this);
	
	m_strKeys = lpszKeys == NULL ? _T("") : lpszKeys;
	m_strMenuKeys = lpszMenuKeys == NULL ? _T("") : lpszMenuKeys;
}
//******************************************************************************
void CBCGPBaseRibbonElement::SetTextAlwaysOnRight (BOOL bSet)
{
	ASSERT_VALID (this);
	m_bTextAlwaysOnRight = bSet;
}
//******************************************************************************
void CBCGPBaseRibbonElement::OnLButtonDown (CPoint /*point*/)
{
	CBCGPRibbonPanel* pPanel = GetParentPanel ();
	if (pPanel != NULL)
	{
		ASSERT_VALID (pPanel);

		CBCGPBaseRibbonElement* pDroppedDown = pPanel->GetDroppedDown ();
		if (pDroppedDown != NULL)
		{
			ASSERT_VALID (pDroppedDown);
			pDroppedDown->ClosePopupMenu ();
		}
	}

	if (m_pParentMenu != NULL)
	{
		return;
	}

	if (m_pRibbonBar != NULL)
	{
		ASSERT_VALID (m_pRibbonBar);

		CBCGPBaseRibbonElement* pDroppedDown = m_pRibbonBar->GetDroppedDown ();
		if (pDroppedDown != NULL)
		{
			ASSERT_VALID (pDroppedDown);
			pDroppedDown->ClosePopupMenu ();
		}
	}

	if (m_pParent != NULL)
	{
		ASSERT_VALID (m_pParent);

		CBCGPBaseRibbonElement* pDroppedDown = m_pParent->GetDroppedDown ();
		if (pDroppedDown != NULL)
		{
			ASSERT_VALID (pDroppedDown);
			pDroppedDown->ClosePopupMenu ();
		}
	}
}
//******************************************************************************
void CBCGPBaseRibbonElement::OnUpdateCmdUI (CBCGPRibbonCmdUI* pCmdUI,
											CFrameWnd* pTarget,
											BOOL bDisableIfNoHndler)
{
	ASSERT_VALID (this);
	ASSERT (pCmdUI != NULL);

	if (m_nID == 0 || IsSystemCommand (m_nID) ||
		m_nID >= AFX_IDM_FIRST_MDICHILD)
	{
		return;
	}

	if (g_pUserToolsManager != NULL &&
		g_pUserToolsManager->IsUserToolCmd(m_nID))
	{
		bDisableIfNoHndler = FALSE;
	}

	pCmdUI->m_pUpdated = this;

	pCmdUI->m_nID = m_nID;
	pCmdUI->DoUpdate (m_pWndTargetCmd != NULL ? m_pWndTargetCmd : pTarget, bDisableIfNoHndler);

	pCmdUI->m_pUpdated = NULL;
}
//******************************************************************************
BOOL CBCGPBaseRibbonElement::NotifyControlCommand (
	BOOL bAccelerator, int nNotifyCode, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(bAccelerator);
	UNREFERENCED_PARAMETER(nNotifyCode);
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);

	return FALSE;
}
//******************************************************************************
CBCGPBaseRibbonElement* CBCGPBaseRibbonElement::HitTest (CPoint /*point*/)
{
	ASSERT_VALID (this);
	return this;
}
//******************************************************************************
CBCGPRibbonBar* CBCGPBaseRibbonElement::GetTopLevelRibbonBar () const
{
	ASSERT_VALID (this);

	if (m_pRibbonBar != NULL)
	{
		ASSERT_VALID (m_pRibbonBar);
		return m_pRibbonBar;
	}

	if (m_pParent != NULL)
	{
		ASSERT_VALID (m_pParent);

		if (m_pParent->GetParentRibbonBar () != NULL)
		{
			ASSERT_VALID (m_pParent->GetParentRibbonBar ());
			return m_pParent->GetParentRibbonBar ();
		}
	}

	if (m_pParentMenu != NULL)
	{
		ASSERT_VALID (m_pParentMenu);
		return m_pParentMenu->GetTopLevelRibbonBar ();
	}

	return NULL;
}
//******************************************************************************
void CBCGPBaseRibbonElement::SetDroppedDown (CBCGPPopupMenu* pPopupMenu)
{
	ASSERT_VALID (this);

	m_pPopupMenu = pPopupMenu;

	if (pPopupMenu != NULL)
	{
		ASSERT_VALID (pPopupMenu);
		pPopupMenu->SetParentRibbonElement (this);
	}
	else
	{
		NotifyHighlightListItem (-1);
	}

	BOOL bWasDroppedDown = m_bIsDroppedDown;
	m_bIsDroppedDown = pPopupMenu != NULL;

	if (!m_bIsDroppedDown)
	{
		m_bIsHighlighted = m_bIsPressed = FALSE;
	}

	if (bWasDroppedDown != m_bIsDroppedDown)
	{
		Redraw ();
	}

	if (m_pParentMenu->GetSafeHwnd () != NULL && pPopupMenu == NULL)
	{
		ASSERT_VALID (m_pParentMenu);
		ASSERT_VALID (m_pParentMenu->GetParent ());

		CBCGPPopupMenu::m_pActivePopupMenu = 
			(CBCGPPopupMenu*) m_pParentMenu->GetParent ();
	}

	m_bOnBeforeShowItemMenuIsSent = FALSE;
}
//******************************************************************************
CBCGPBaseRibbonElement* CBCGPBaseRibbonElement::Find (const CBCGPBaseRibbonElement* pElement)
{
	ASSERT_VALID (this);
	return (pElement == this) ? this : NULL;
}
//******************************************************************************
CBCGPBaseRibbonElement* CBCGPBaseRibbonElement::FindByID (UINT uiCmdID)
{
	ASSERT_VALID (this);

	return (m_nID == uiCmdID) ? this : NULL;
}
//******************************************************************************
CBCGPBaseRibbonElement* CBCGPBaseRibbonElement::FindByIDNonCustom (UINT uiCmdID)
{
	ASSERT_VALID (this);

	return (m_nID == uiCmdID && !m_bIsCustom) ? this : NULL;
}
//******************************************************************************
CBCGPBaseRibbonElement* CBCGPBaseRibbonElement::FindByData (DWORD_PTR dwData)
{
	ASSERT_VALID (this);

	return (m_dwData == dwData) ? this : NULL;
}
//******************************************************************************
CBCGPBaseRibbonElement* CBCGPBaseRibbonElement::FindByOriginal (CBCGPBaseRibbonElement* pOriginal)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pOriginal);

	return (m_pOriginal == pOriginal) ? this : NULL;
}
//******************************************************************************
CBCGPBaseRibbonElement* CBCGPBaseRibbonElement::GetPressed ()
{
	ASSERT_VALID (this);

	return IsPressed () ? this : NULL;
}
//******************************************************************************
CBCGPBaseRibbonElement* CBCGPBaseRibbonElement::GetDroppedDown ()
{
	ASSERT_VALID (this);

	return IsDroppedDown () ? this : NULL;
}
//******************************************************************************
CBCGPBaseRibbonElement* CBCGPBaseRibbonElement::GetFocused ()
{
	ASSERT_VALID (this);

	return IsFocused () ? this : NULL;
}
//******************************************************************************
CBCGPBaseRibbonElement* CBCGPBaseRibbonElement::GetHighlighted ()
{
	ASSERT_VALID (this);

	return IsHighlighted () ? this : NULL;
}
//******************************************************************************
BOOL CBCGPBaseRibbonElement::ReplaceByID (UINT uiCmdID, CBCGPBaseRibbonElement* pElem)
{
	ASSERT_VALID (this);
	UNREFERENCED_PARAMETER (uiCmdID);
	UNREFERENCED_PARAMETER (pElem);

	return FALSE;
}
//******************************************************************************
BOOL CBCGPBaseRibbonElement::ReplaceSubitemByID (UINT uiCmdID, CBCGPBaseRibbonElement* pElem, BOOL bCopyContent)
{
	ASSERT_VALID(this);
	UNREFERENCED_PARAMETER(uiCmdID);
	UNREFERENCED_PARAMETER(pElem);
	UNREFERENCED_PARAMETER(bCopyContent);

	return FALSE;
}
//******************************************************************************
void CBCGPBaseRibbonElement::SetParentCategory (CBCGPRibbonCategory* pParent)
{
	ASSERT_VALID (this);
	m_pParent = pParent;
}
//******************************************************************************
void CBCGPBaseRibbonElement::CopyFrom (const CBCGPBaseRibbonElement& src)
{
	ASSERT_VALID (this);

	m_nID = src.m_nID;
	m_dwData = src.m_dwData;
	m_sizePadding = src.m_sizePadding;
	m_bIsCustom = src.m_bIsCustom;
	m_nCustomImageIndex = src.m_nCustomImageIndex;
	m_bIsNew = src.m_bIsNew;
	m_bTextAlwaysOnRight = src.m_bTextAlwaysOnRight;
	m_strText = src.m_strText;
	m_strKeys = src.m_strKeys;
	m_strMenuKeys = src.m_strMenuKeys;
	m_pParent = src.m_pParent;
	m_pParentGroup = src.m_pParentGroup;
	m_strToolTip = src.m_strToolTip;
	m_strDescription = src.m_strDescription;
	m_bQuickAccessMode = src.m_bQuickAccessMode;
	m_bIsVisible = src.m_bIsVisible;
	m_bIsDefaultMenuLook = src.m_bIsDefaultMenuLook;
	m_bIsRadio = src.m_bIsRadio;
	m_bIsAlwaysLarge = src.m_bIsAlwaysLarge;
	m_bIsOnPaletteTop = src.m_bIsOnPaletteTop;
	m_bEnableUpdateTooltipInfo = src.m_bEnableUpdateTooltipInfo;
	m_bEnableTooltipInfoShortcut = src.m_bEnableTooltipInfoShortcut;
	m_bIsBackstageViewMode = src.m_bIsBackstageViewMode;
	m_pWndTargetCmd = src.m_pWndTargetCmd;

	if (m_nID >= AFX_IDM_FIRST_MDICHILD)
	{
		m_bIsChecked = src.m_bIsChecked;
	}
}
//******************************************************************************
void CBCGPBaseRibbonElement::CopyBaseFrom (const CBCGPBaseRibbonElement& src)
{
	ASSERT_VALID (this);

	if (m_nID == 0)
	{
		m_nID = src.m_nID;
	}

	if (m_dwData == 0)
	{
		m_dwData = src.m_dwData;
	}

	if (m_strText.IsEmpty())
	{
		m_strText = src.m_strText;
	}

	if (m_strKeys.IsEmpty())
	{
		m_strKeys = src.m_strKeys;
	}

	m_pParent = src.m_pParent;
	m_pParentGroup = src.m_pParentGroup;

	if (m_strToolTip.IsEmpty())
	{
		m_strToolTip = src.m_strToolTip;
	}

	if (m_strDescription.IsEmpty())
	{
		m_strDescription = src.m_strDescription;
	}
}
//******************************************************************************
void CBCGPBaseRibbonElement::SetParentMenu (CBCGPRibbonPanelMenuBar* pMenuBar)
{
	ASSERT_VALID (this);

	m_pParentMenu = pMenuBar;
}
//*****************************************************************************
void CBCGPBaseRibbonElement::SetOriginal (CBCGPBaseRibbonElement* pOriginal)
{
	ASSERT_VALID (this);

	if (pOriginal != NULL)
	{
		ASSERT_VALID (pOriginal);

		while (pOriginal->m_pOriginal != NULL)
		{
			pOriginal = pOriginal->m_pOriginal;
		}
	}

	m_pOriginal = pOriginal;
}
//******************************************************************************
CWnd* CBCGPBaseRibbonElement::GetParentWnd () const
{
	ASSERT_VALID (this);

	if (m_pRibbonBar != NULL)
	{
		ASSERT_VALID (m_pRibbonBar);
		return m_pRibbonBar;
	}
	else if (m_pParentMenu != NULL)
	{
		ASSERT_VALID (m_pParentMenu);
		return m_pParentMenu;
	}
	else if (m_pParent != NULL)
	{
		ASSERT_VALID (m_pParent);
		ASSERT_VALID (m_pParent->GetParentRibbonBar ());

		return m_pParent->GetParentRibbonBar ();
	}

	return NULL;
}
//******************************************************************************
CBCGPRibbonPanel* CBCGPBaseRibbonElement::GetParentPanel () const
{
	ASSERT_VALID (this);

	if (m_pParentMenu != NULL)
	{
		ASSERT_VALID (m_pParentMenu);

		if (m_pParentMenu->m_pCategory != NULL)
		{
			ASSERT_VALID (m_pParentMenu->m_pCategory);
			return m_pParentMenu->m_pCategory->FindPanelWithElem (this);
		}

		return m_pParentMenu->m_pPanel;
	}
	else if (m_pParent != NULL)
	{
		ASSERT_VALID (m_pParent);
		return m_pParent->FindPanelWithElem (this);
	}

	return NULL;
}
//******************************************************************************
BOOL CBCGPBaseRibbonElement::IsMenuMode () const
{
	ASSERT_VALID (this);

	if (m_pParentMenu != NULL)
	{
		ASSERT_VALID (m_pParentMenu);

		if (m_pParentMenu->m_pPanel == NULL)
		{
			return FALSE;
		}

		ASSERT_VALID (m_pParentMenu->m_pPanel);

		return m_pParentMenu->m_pPanel->IsMenuMode ();
	}

	return FALSE;
}
//******************************************************************************
void CBCGPBaseRibbonElement::Redraw ()
{
	ASSERT_VALID (this);

	if (m_rect.IsRectEmpty ())
	{
		return;
	}

	if (m_pParentMenu->GetSafeHwnd () != NULL)
	{
		m_pParentMenu->RedrawWindow (m_rect);
		return;
	}

	CWnd* pWndParent = GetParentWnd ();

	if (pWndParent->GetSafeHwnd () != NULL)
	{
		pWndParent->RedrawWindow (m_rect);
	}
}
//******************************************************************************
CString CBCGPBaseRibbonElement::GetToolTipText () const
{
	ASSERT_VALID (this);

	if (IsDroppedDown ())
	{
		return _T("");
	}

	CString strTipText = m_strToolTip;

	if (m_bQuickAccessMode && strTipText.IsEmpty ())
	{
		strTipText = m_strText;

		strTipText.Replace (_T("&&"), strDummyAmpSeq);
		strTipText.Remove (_T('&'));
		strTipText.Replace (strDummyAmpSeq, _T("&"));
	}

	//--------------------
	// Add shortcut label:
	//--------------------
	CWnd* pWndParent = NULL;

	if (m_pRibbonBar != NULL)
	{
		pWndParent = m_pRibbonBar;
	}
	else if (m_pParentMenu != NULL)
	{
		pWndParent = m_pParentMenu;
	}
	else if (m_pParent != NULL)
	{
		ASSERT_VALID (m_pParent);
		pWndParent = m_pParent->GetParentRibbonBar ();
	}

	if (m_bEnableTooltipInfoShortcut)
	{
		CString strLabel;
		CFrameWnd* pParentFrame = BCGPGetParentFrame (pWndParent) == NULL ?
			NULL : BCGCBProGetTopLevelFrame (BCGPGetParentFrame (pWndParent));

		if (pParentFrame != NULL &&
			(CBCGPKeyboardManager::FindDefaultAccelerator (
				m_nID, strLabel, pParentFrame, TRUE) ||
			CBCGPKeyboardManager::FindDefaultAccelerator (
				m_nID, strLabel, pParentFrame->GetActiveFrame (), FALSE)))
		{
			strTipText += _T(" (");
			strTipText += strLabel;
			strTipText += _T(')');
		}
	}

	return strTipText;
}
//******************************************************************************
CString CBCGPBaseRibbonElement::GetDescription () const
{
	ASSERT_VALID (this);
	return m_strDescription;
}
//******************************************************************************
void CBCGPBaseRibbonElement::SetToolTipText (LPCTSTR lpszText)
{
	ASSERT_VALID (this);
	m_strToolTip = lpszText == NULL ? _T("") : lpszText;
}
//******************************************************************************
void CBCGPBaseRibbonElement::SetDescription (LPCTSTR lpszText)
{
	ASSERT_VALID (this);
	m_strDescription = lpszText == NULL ? _T("") : lpszText;
}
//******************************************************************************
void CBCGPBaseRibbonElement::EnableUpdateTooltipInfo (BOOL bEnable)
{
	if (m_bEnableUpdateTooltipInfo != bEnable)
	{
		m_bEnableUpdateTooltipInfo = bEnable;

		if (m_bEnableUpdateTooltipInfo)
		{
			UpdateTooltipInfo ();
		}
	}
}
//******************************************************************************
void CBCGPBaseRibbonElement::EnableTooltipInfoShortcut (BOOL bEnable)
{
	m_bEnableTooltipInfoShortcut = bEnable;
}
//******************************************************************************
void CBCGPBaseRibbonElement::UpdateTooltipInfo ()
{
	ASSERT_VALID (this);

	if (!m_bEnableUpdateTooltipInfo || m_nID == 0 || m_nID == (UINT)-1)
	{
		return;
	}

	CString strText;
	if (!strText.LoadString (m_nID))
	{
		return;
	}

	m_strToolTip.Empty ();
	m_strDescription.Empty ();

	if (strText.IsEmpty ())
	{
		return;
	}

	AfxExtractSubString (m_strDescription, strText, 0);
	AfxExtractSubString (m_strToolTip, strText, 1, '\n');

	const CString strDummyAmpSeq = _T("\001\001");

	m_strToolTip.Replace (_T("&&"), strDummyAmpSeq);
	m_strToolTip.Remove (_T('&'));
	m_strToolTip.Replace (strDummyAmpSeq, _T("&"));
}
//******************************************************************************
void CBCGPBaseRibbonElement::OnAfterChangeRect (CDC* /*pDC*/)
{
	ASSERT_VALID (this);

	if (m_strToolTip.IsEmpty ())
	{
		UpdateTooltipInfo ();
	}
}
//******************************************************************************
BOOL CBCGPBaseRibbonElement::NotifyCommand (BOOL bWithDelay)
{
	ASSERT_VALID (this);

	if (m_pOriginal != NULL)
	{
		if (m_bQuickAccessMode && (m_bIsHighlighted || m_bIsPressed || m_bIsFocused))
		{
			m_bIsHighlighted = m_bIsPressed = m_bIsFocused = FALSE;
			Redraw ();
		}

		ASSERT_VALID (m_pOriginal);
		return m_pOriginal->NotifyCommand (bWithDelay);
	}

	UINT uiID = GetNotifyID ();

	if (uiID == 0 || uiID == (UINT)-1)
	{
		return FALSE;
	}

	if (GetBackstageAttachedView() != NULL)
	{
		return FALSE;
	}

	CWnd* pWndParent = GetTargetCmdWnd();
	if (pWndParent == NULL)
	{
		return FALSE;
	}

	m_bIsHighlighted = m_bIsPressed = m_bIsFocused = FALSE;

	Redraw ();

	ASSERT_VALID (pWndParent);

	if (uiID == AFX_IDM_FIRST_MDICHILD)
	{
		HWND hwndMDIChild = (HWND) m_dwData;

		if (::IsWindow (hwndMDIChild))
		{
			CBCGPRibbonBar* pTopLevelRibbon = GetTopLevelRibbonBar ();
			if (pTopLevelRibbon != NULL)
			{
				CBCGPMDIFrameWnd* pMDIFrameWnd = DYNAMIC_DOWNCAST (
					CBCGPMDIFrameWnd, pTopLevelRibbon->GetTopLevelFrame ());

				if (pMDIFrameWnd != NULL)
				{
					WINDOWPLACEMENT	wndpl;
					wndpl.length = sizeof(WINDOWPLACEMENT);
					::GetWindowPlacement (hwndMDIChild,&wndpl);

					if (wndpl.showCmd == SW_SHOWMINIMIZED)
					{
						::ShowWindow (hwndMDIChild, SW_RESTORE);
					}

					if (bWithDelay)
					{
						::PostMessage (pMDIFrameWnd->m_hWndMDIClient, WM_MDIACTIVATE, (WPARAM) hwndMDIChild, 0);
					}
					else
					{
						::SendMessage (pMDIFrameWnd->m_hWndMDIClient, WM_MDIACTIVATE, (WPARAM) hwndMDIChild, 0);
					}

					return TRUE;
				}
			}
		}
	}

	if (g_pUserToolsManager != NULL && g_pUserToolsManager->InvokeTool (uiID))
	{
		return TRUE;
	}

	if (bWithDelay)
	{
		pWndParent->PostMessage (WM_COMMAND, uiID);
	}
	else
	{
		pWndParent->SendMessage (WM_COMMAND, uiID);
	}

	return TRUE;
}
//*************************************************************************************
CWnd* CBCGPBaseRibbonElement::GetTargetCmdWnd()
{
	ASSERT_VALID (this);

	if (m_pWndTargetCmd != NULL)
	{
		ASSERT_VALID(m_pWndTargetCmd);
		return m_pWndTargetCmd;
	}

	CBCGPRibbonBar* pRibbonBar = GetTopLevelRibbonBar ();
	if (pRibbonBar == NULL)
	{
		return NULL;
	}

	ASSERT_VALID (pRibbonBar);

	return pRibbonBar->GetParent ();
}
//*************************************************************************************
void CBCGPBaseRibbonElement::PostMenuCommand (UINT /*uiCmdId*/)
{
	ASSERT_VALID (this);

	m_bIsDroppedDown = FALSE;
	Redraw ();

	if (m_pParentMenu != NULL)
	{
		ASSERT_VALID (m_pParentMenu);
		m_pParentMenu->GetParent ()->SendMessage (WM_CLOSE);
	}
}
//*************************************************************************************
void CBCGPBaseRibbonElement::GetElementsByID (UINT uiCmdID,
		CArray <CBCGPBaseRibbonElement*, CBCGPBaseRibbonElement*>& arElements)
{
	ASSERT_VALID (this);

	if (uiCmdID == m_nID)
	{
		arElements.Add (this);
	}
}
//*************************************************************************************
void CBCGPBaseRibbonElement::GetElementsByName (LPCTSTR lpszName, 
		CArray<CBCGPBaseRibbonElement*, CBCGPBaseRibbonElement*>& arButtons, DWORD /*dwFlags*/)
{
	ASSERT_VALID (this);
	ASSERT (lpszName != NULL);

	if (m_nID == 0 || m_nID == (UINT)-1)
	{
		return;
	}

	CString strText = m_strText;
	strText.MakeUpper ();

	CString strName = lpszName;
	strName.MakeUpper ();

	if (strText.Find (strName) >= 0)
	{
		arButtons.Add (this);
		return;
	}

	strText = m_strToolTip;
	strText.MakeUpper ();

	if (strText.Find (strName) >= 0)
	{
		arButtons.Add (this);
	}
}
//*************************************************************************************
void CBCGPBaseRibbonElement::GetVisibleElements (
		CArray <CBCGPBaseRibbonElement*, CBCGPBaseRibbonElement*>& arElements)
{
	ASSERT_VALID (this);

	if (!m_rect.IsRectEmpty ())
	{
		arElements.Add (this);
	}
}
//*************************************************************************************
void CBCGPBaseRibbonElement::GetItemIDsList (CList<UINT,UINT>& lstItems) const
{
	ASSERT_VALID (this);

	if (m_nID != 0 && m_nID != (UINT)-1 && lstItems.Find (m_nID) == NULL)
	{
		lstItems.AddTail (m_nID);
	}
}
//*************************************************************************************
int CBCGPBaseRibbonElement::AddToListBox (CBCGPRibbonCommandsListBox* pWndListBox, BOOL /*bDeep*/)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pWndListBox);
	ASSERT (pWndListBox->GetSafeHwnd () != NULL);

	if (m_nID == 0 || m_nID == (UINT)-1)
	{
		return -1;
	}

	if (m_nID >= ID_FILE_MRU_FILE1 && m_nID <= ID_FILE_MRU_FILE16)
	{
		return -1;
	}

	if (!CBCGPToolBar::IsCommandPermitted (m_nID))
	{
		return -1;
	}

	if (g_pUserToolsManager != NULL && (g_pUserToolsManager->IsUserToolCmd(m_nID) || g_pUserToolsManager->GetToolsEntryCmd() == m_nID))
	{
		return -1;
	}

	for (int i = 0; i < pWndListBox->GetCount (); i++)
	{
		CBCGPBaseRibbonElement* pItem = (CBCGPBaseRibbonElement*) pWndListBox->GetItemData (i);
		if (pItem == NULL)
		{
			continue;
		}

		ASSERT_VALID (pItem);

		if (pItem->m_nID == m_nID && (!pItem->HasMenu () || pWndListBox->CommandsOnly()))
		{
			// Already exist, don't add it
			return -1;
		}
	}

	UpdateTooltipInfo ();

	CString strText = m_strToolTip;
	if (strText.IsEmpty ())
	{
		strText = GetText ();
	}

	strText.Replace (_T("&&"), strDummyAmpSeq);
	strText.Remove (_T('&'));
	strText.Replace (strDummyAmpSeq, _T("&"));

	int nIndex = pWndListBox->AddString (strText);
	pWndListBox->SetItemData (nIndex, (DWORD_PTR) this);

	return nIndex;
}
//********************************************************************************
CBCGPGridRow* CBCGPBaseRibbonElement::AddToTree (CBCGPGridCtrl* pGrid, CBCGPGridRow* pParent)
{
#ifndef BCGP_EXCLUDE_GRID_CTRL
	ASSERT_VALID (this);
	ASSERT_VALID (pGrid);
	ASSERT (pGrid->GetSafeHwnd () != NULL);

	if (m_nID == 0 || m_nID == (UINT)-1 || IsSeparator())
	{
		return NULL;
	}

	if (m_nID >= ID_FILE_MRU_FILE1 && m_nID <= ID_FILE_MRU_FILE16)
	{
		return NULL;
	}

	if (!CBCGPToolBar::IsCommandPermitted (m_nID))
	{
		return NULL;
	}

	if (g_pUserToolsManager != NULL && g_pUserToolsManager->IsUserToolCmd(m_nID))
	{
		return NULL;
	}

	if (pParent != NULL)
	{
		ASSERT_VALID(pParent);

		for (int i = 0; i < pParent->GetSubItemsCount(); i++)
		{
			CBCGPGridRow* pRow = pParent->GetSubItem (i);
			ASSERT_VALID(pRow);

			CBCGPBaseRibbonElement* pItem = DYNAMIC_DOWNCAST(CBCGPBaseRibbonElement, (CObject*) pRow->GetData());
			if (pItem == NULL)
			{
				continue;
			}

			ASSERT_VALID (pItem);

			if (pItem->m_nID == m_nID)
			{
				// Already exist, don't add it
				return NULL;
			}
		}
	}
	else
	{
		for (int i = 0; i < pGrid->GetRowCount(); i++)
		{
			CBCGPGridRow* pRow = pGrid->GetRow (i);
			if (pRow == NULL)
			{
				continue;
			}

			ASSERT_VALID(pRow);

			CBCGPBaseRibbonElement* pItem = DYNAMIC_DOWNCAST(CBCGPBaseRibbonElement, (CObject*) pRow->GetData());
			if (pItem == NULL)
			{
				continue;
			}

			ASSERT_VALID (pItem);

			if (pItem->m_nID == m_nID)
			{
				// Already exist, don't add it
				return NULL;
			}
		}
	}

	UpdateTooltipInfo ();

	CString strText = m_strToolTip;
	if (strText.IsEmpty ())
	{
		strText = GetText ();
	}

	strText.Replace (_T("&&"), strDummyAmpSeq);
	strText.Remove (_T('&'));
	strText.Replace (strDummyAmpSeq, _T("&"));

	const int nColumns = pGrid->GetColumnCount ();

	CBCGPGridRow* pElemItem = pGrid->CreateRow(nColumns);
	ASSERT_VALID(pElemItem);

	pElemItem->SetData((DWORD_PTR)this);
	pElemItem->GetItem (0)->SetValue((LPCTSTR)strText);

	if (pParent != NULL)
	{
		pParent->AddSubItem (pElemItem, FALSE);
	}
	else
	{
		pGrid->AddRow(pElemItem, FALSE);
	}

	return pElemItem;
#else
	return NULL;
#endif
}
//********************************************************************************
void CBCGPBaseRibbonElement::OnDrawOnList (CDC* pDC, CString strText,
									  int nTextOffset, CRect rect,
									  BOOL /*bIsSelected*/,
									  BOOL /*bHighlighted*/)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);

	BOOL bIsDisabled = m_bIsDisabled;
	m_bIsDisabled = FALSE;

	if (m_bDrawDefaultIcon)
	{
		CRect rectImage = rect;
		rectImage.right = rect.left + nTextOffset;

		CBCGPVisualManager::GetInstance ()->OnDrawDefaultRibbonImage (
			pDC, rectImage);
	}

	CRect rectText = rect;

	rectText.left += nTextOffset;

	const int xMargin = 3;
	rectText.DeflateRect (xMargin, 0);

	pDC->DrawText (strText, rectText, DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
	m_bIsDisabled = bIsDisabled;
}
//*************************************************************************************
void CBCGPBaseRibbonElement::ClosePopupMenu ()
{
	ASSERT_VALID (this);

	if (m_pPopupMenu != NULL && ::IsWindow (m_pPopupMenu->m_hWnd))
	{
		if (m_pPopupMenu->InCommand ())
		{
			return;
		}

		m_pPopupMenu->m_bAutoDestroyParent = FALSE;
		m_pPopupMenu->CloseMenu ();
	}

	m_pPopupMenu = NULL;
	m_bOnBeforeShowItemMenuIsSent = FALSE;
}
//*************************************************************************************
BOOL CBCGPBaseRibbonElement::CanBeAddedToQAT () const
{
	ASSERT_VALID (this);

	if (g_pUserToolsManager != NULL && (g_pUserToolsManager->IsUserToolCmd(m_nID) || g_pUserToolsManager->GetToolsEntryCmd() == m_nID))
	{
		return FALSE;
	}

	return m_nID != 0 && m_nID != (UINT)-1 && !IsBCGPStandardCommand (m_nID);
}
//*************************************************************************************
BOOL CBCGPBaseRibbonElement::OnAddToQAToolbar (CBCGPRibbonQuickAccessToolbar& qat)
{
	qat.Add (this);
	return TRUE;
}
//*************************************************************************************
BOOL CBCGPBaseRibbonElement::StretchToWholeRow (CDC* /*pDC*/, int nHeight)
{
	ASSERT_VALID (this);

	if (!CanBeStretched () || m_bCompactMode || m_bIntermediateMode)
	{
		return FALSE;
	}

	m_rect.bottom = m_rect.top + nHeight;
	return TRUE;
}
//*************************************************************************************
void CBCGPBaseRibbonElement::OnDrawKeyTip (CDC* pDC, const CRect& rect, BOOL bIsMenu)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);

	if (m_strKeys.IsEmpty ())
	{
		return;
	}

	CBCGPVisualManager::GetInstance ()->OnDrawRibbonKeyTip (pDC, 
			this, rect, bIsMenu ? m_strMenuKeys : m_strKeys);
}
//*************************************************************************************
CSize CBCGPBaseRibbonElement::GetKeyTipSize (CDC* pDC)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);

	if (!m_bQuickAccessMode && m_pParentMenu != NULL && m_strKeys.GetLength () < 2)
	{
		// Try to find key from label:
		int nIndexAmp = m_strText.Find (_T('&'));

		if (nIndexAmp >= 0 && nIndexAmp < m_strText.GetLength () - 1 &&
			m_strText [nIndexAmp + 1] != _T('&'))
		{
			m_strKeys = m_strText.Mid (nIndexAmp + 1, 1);
		}
	}

	if (m_strKeys.IsEmpty ())
	{
		return CSize (0, 0);
	}

	const CString strMin = _T("O");

	CSize sizeMin = pDC->GetTextExtent (strMin);
	CSize sizeText = pDC->GetTextExtent (m_strKeys);

	sizeText.cx = max (sizeText.cx, sizeMin.cx);
	sizeText.cy = max (sizeText.cy, sizeMin.cy);

	return CSize (sizeText.cx + 10, sizeText.cy + 2);
}
//*************************************************************************************
BOOL CBCGPBaseRibbonElement::OnKey (BOOL bIsMenuKey)
{
	ASSERT_VALID (this);

	if (m_bIsDisabled)
	{
		return FALSE;
	}

	if (m_rect.IsRectEmpty ())
	{
		CBCGPRibbonPanel* pParentPanel = GetParentPanel ();
		if (pParentPanel != NULL)
		{
			ASSERT_VALID (pParentPanel);

			if (pParentPanel->IsCollapsed ())
			{
				if (!HasMenu ())
				{
					if (!NotifyCommand (TRUE))
					{
						return FALSE;
					}

					if (m_pParentMenu != NULL)
					{
						CBCGPRibbonBar* pTopLevelRibbon = GetTopLevelRibbonBar ();

						// Close menu:
						ASSERT_VALID (m_pParentMenu);
						
						CFrameWnd* pParentFrame = BCGPGetParentFrame (m_pParentMenu);
						ASSERT_VALID (pParentFrame);

						pParentFrame->DestroyWindow ();
						
						if (pTopLevelRibbon != NULL && pTopLevelRibbon->GetTopLevelFrame () != NULL)
						{
							pTopLevelRibbon->GetTopLevelFrame ()->SetFocus ();
						}
					}

					return TRUE;
				}
				else
				{
					CBCGPRibbonBar* pTopLevelRibbon = GetTopLevelRibbonBar ();
					if (pTopLevelRibbon != NULL)
					{
						pTopLevelRibbon->HideKeyTips ();
					}

					CBCGPRibbonPanelMenu* pPopup = pParentPanel->ShowPopup ();
					if (pPopup != NULL)
					{
						ASSERT_VALID (pPopup);

						CBCGPBaseRibbonElement* pPopupElem = pPopup->FindByOrigin (this);
						if (pPopupElem != NULL)
						{
							ASSERT_VALID (pPopupElem);
							return pPopupElem->OnKey (bIsMenuKey);
						}
					}
				}
			}
		}

		return FALSE;
	}

	return NotifyCommand (TRUE);
}
//*************************************************************************************
void CBCGPBaseRibbonElement::AddToKeyList (CArray<CBCGPRibbonKeyTip*,CBCGPRibbonKeyTip*>& arElems)
{
	ASSERT_VALID (this);

	arElems.Add (new CBCGPRibbonKeyTip (this));

	if (!m_strMenuKeys.IsEmpty () && HasMenu ())
	{
		arElems.Add (new CBCGPRibbonKeyTip (this, TRUE));
	}
}
//********************************************************************************
int CBCGPBaseRibbonElement::GetDropDownImageWidth () const
{
	ASSERT_VALID (this);

	return CBCGPMenuImages::Size ().cx;
}
//********************************************************************************
void CBCGPBaseRibbonElement::NotifyHighlightListItem (int nIndex)
{
	ASSERT_VALID (this);

	CBCGPRibbonBar* pRibbonBar = GetTopLevelRibbonBar ();
	if (pRibbonBar == NULL)
	{
		return;
	}

	ASSERT_VALID (pRibbonBar);

	CWnd* pWndParent = pRibbonBar->GetParent ();
	if (pWndParent == NULL)
	{
		return;
	}

	pWndParent->SendMessage (BCGM_ON_HIGHLIGHT_RIBBON_LIST_ITEM, (WPARAM) nIndex, (LPARAM) this);
}
//********************************************************************************
void CBCGPBaseRibbonElement::OnShowPopupMenu ()
{
	ASSERT_VALID (this);

	CBCGPRibbonBar* pRibbonBar = GetTopLevelRibbonBar ();
	if (pRibbonBar == NULL)
	{
		return;
	}

	ASSERT_VALID (pRibbonBar);

	CWnd* pWndParent = pRibbonBar->GetParent ();
	if (pWndParent == NULL)
	{
		return;
	}

	if (!m_bOnBeforeShowItemMenuIsSent)
	{
		m_bOnBeforeShowItemMenuIsSent = TRUE;
		pWndParent->SendMessage (BCGM_ON_BEFORE_SHOW_RIBBON_ITEM_MENU, (WPARAM) 0, (LPARAM) this);
	}
}

//********************************************************************************
BOOL CBCGPBaseRibbonElement::SetACCData (CWnd* pParent, CBCGPAccessibilityData& data)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pParent);

	data.Clear ();

	data.m_strAccName = m_strText.IsEmpty () ? m_strToolTip : m_strText;
	data.m_strAccName.Remove (_T('&'));
	data.m_strAccName.TrimLeft ();
	data.m_strAccName.TrimRight ();

	data.m_nAccRole = IsMenuMode () ? ROLE_SYSTEM_MENUITEM : ROLE_SYSTEM_PUSHBUTTON;

	data.m_strDescription = m_strDescription;
	data.m_nAccHit = 1;
	data.m_strAccDefAction = IsMenuMode () ? _T("Execute") : _T("Press");

	data.m_bAccState = STATE_SYSTEM_FOCUSABLE;
	if (IsChecked ())
	{
		data.m_bAccState |= STATE_SYSTEM_CHECKED; 
	}

	if (IsDisabled ())
	{
		data.m_bAccState |= STATE_SYSTEM_UNAVAILABLE;
	}

	if (IsPressed () || IsMenuMode () && IsHighlighted ())
	{
		 data.m_bAccState |= STATE_SYSTEM_FOCUSED;
	}

	if (IsHighlighted () || IsFocused ())
	{
		data.m_bAccState |= STATE_SYSTEM_FOCUSED;
	}
	
	data.m_rectAccLocation = m_rect;
	pParent->ClientToScreen (&data.m_rectAccLocation);

	if (IsSeparator())
	{
		data.m_strAccName =  m_strText.IsEmpty() ?  _T ("Separator") : m_strText;
		data.m_nAccRole = ROLE_SYSTEM_SEPARATOR;
		data.m_bAccState = STATE_SYSTEM_NORMAL;
		data.m_strAccDefAction.Empty();

		return TRUE;
	}

	CString strKeys = m_strKeys;

	if (!m_bQuickAccessMode && m_pParentMenu != NULL && strKeys.GetLength () < 2)
	{
		// Try to find key from label:
		int nIndexAmp = m_strText.Find (_T('&'));

		if (nIndexAmp >= 0 && nIndexAmp < m_strText.GetLength () - 1 &&
			m_strText [nIndexAmp + 1] != _T('&'))
		{
			strKeys = m_strText.Mid (nIndexAmp + 1, 1);
		}
	}

	if (!strKeys.IsEmpty ())
	{
		data.m_strAccKeys = _T("Alt, ");

		if (m_pParent != NULL)
		{
			ASSERT_VALID(m_pParent);
			data.m_strAccKeys +=  m_pParent->m_Tab.m_strKeys + _T(", ");
		}

		data.m_strAccKeys += strKeys;
	}

	return TRUE;
}
//********************************************************************************
BOOL CBCGPBaseRibbonElement::IsScrolledOut () const
{
	ASSERT_VALID (this);

	CBCGPRibbonPanel* pPanel = GetParentPanel ();
	if (pPanel == NULL)
	{
		return FALSE;
	}

	ASSERT_VALID (pPanel);

	CBCGPRibbonPanelMenuBar* pMenuBar = pPanel->GetParentMenuBar ();
	if (pMenuBar == NULL)
	{
		return FALSE;
	}

	ASSERT_VALID (pMenuBar);

	CRect rectMenu;
	pMenuBar->GetClientRect (rectMenu);

	return (m_rect.bottom > rectMenu.bottom || m_rect.top < rectMenu.top);
}
//******************************************************************************
int CBCGPBaseRibbonElement::DoDrawText (CDC* pDC, const CString& strText, CRect rectText, UINT uiDTFlags,
								 COLORREF clrText)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);

	if (CBCGPToolBarImages::m_bIsDrawOnGlass)
	{
		const BOOL bIsZoomed = GetParentRibbonBar ()->GetSafeHwnd () != NULL &&
			GetParentRibbonBar ()->GetParent ()->IsZoomed ();

		CBCGPVisualManager::GetInstance ()->DrawTextOnGlass (pDC, strText, rectText, uiDTFlags, m_nTextGlassGlowSize, 
			bIsZoomed && !globalData.bIsWindows7 ? RGB (255, 255, 255) : clrText);

		return pDC->GetTextExtent (strText).cy;
	}

	COLORREF clrTextOld = (COLORREF)-1;
	if (clrText != (COLORREF)-1)
	{
		clrTextOld = pDC->SetTextColor (clrText);
	}

	int nRes = 0;

	if (m_bSearchResultMode)
	{
		CString strWithoutAmp = strText;
		strWithoutAmp.Replace (_T("&&"), strDummyAmpSeq);
		strWithoutAmp.Remove (_T('&'));
		strWithoutAmp.Replace (strDummyAmpSeq, _T("&"));

		nRes = pDC->DrawText (strWithoutAmp, rectText, uiDTFlags);
	}
	else if (m_bIsTabElement && CBCGPVisualManager::GetInstance()->IsTopLevelMenuItemUpperCase())
	{
		CString strTextUpper = strText;
		strTextUpper.MakeUpper();

		nRes = pDC->DrawText (strTextUpper, rectText, uiDTFlags);
	}
	else
	{
		nRes = pDC->DrawText (strText, rectText, uiDTFlags);
	}

	if (clrTextOld != (COLORREF)-1)
	{
		pDC->SetTextColor (clrTextOld);
	}

	return nRes;
}
//******************************************************************************
CBCGPBaseRibbonElement* CBCGPBaseRibbonElement::CreateQATCopy()
{
	ASSERT_VALID (this);

	CBCGPBaseRibbonElement* pButton = (CBCGPBaseRibbonElement*)GetRuntimeClass ()->CreateObject();
	ASSERT_VALID (pButton);

	pButton->CopyFrom(*this);

	if (m_pOriginal != NULL)
	{
		pButton->SetOriginal(m_pOriginal);
	}
	else
	{
		pButton->SetOriginal(this);
	}

	pButton->m_bQuickAccessMode = TRUE;

	return pButton;
}
//******************************************************************************
CBCGPBaseRibbonElement* CBCGPBaseRibbonElement::CreateCustomCopy()
{
	ASSERT_VALID (this);

	CBCGPBaseRibbonElement* pButton = (CBCGPBaseRibbonElement*)GetRuntimeClass ()->CreateObject();
	ASSERT_VALID (pButton);

	pButton->CopyFrom(*this);

	pButton->m_pParentGroup = NULL;
	pButton->m_bIsDefaultMenuLook = FALSE;

	CString strName = pButton->GetText() == NULL ? _T("") : pButton->GetText();
	if (strName.IsEmpty())
	{
		pButton->SetText(pButton->m_strToolTip);
	}

	if (pButton->m_strToolTip.IsEmpty())
	{
		pButton->m_strToolTip = m_strText;
	}

	return pButton;
}
//******************************************************************************
void CBCGPBaseRibbonElement::ApplyCustomizationData(CBCGPRibbonCustomElement& customElement)
{
	ASSERT_VALID (this);

	SetText(customElement.m_strName);
	m_nCustomImageIndex = customElement.m_nCustomIcon;
}
//******************************************************************************
void CBCGPBaseRibbonElement::SetParentRibbonBar (CBCGPRibbonBar* pRibbonBar)
{
	ASSERT_VALID (this);

	m_pRibbonBar = pRibbonBar;
	m_bStatusBarMode = DYNAMIC_DOWNCAST(CBCGPRibbonStatusBar, pRibbonBar) != NULL;
}
//**************************************************************************************
HRESULT CBCGPBaseRibbonElement::get_accParent(IDispatch **ppdispParent)
{
    if (!ppdispParent)
    {
        return E_INVALIDARG;
    }

    *ppdispParent = NULL;

    if ((!IsTabElement() && !IsStatusBarMode ()) || m_pRibbonBar->GetSafeHwnd() == NULL)
    {
        return S_FALSE;
    }

    LPDISPATCH lpDispatch = m_pRibbonBar->GetAccessibleDispatch();
	if (lpDispatch != NULL)
	{
		*ppdispParent = lpDispatch;
	}

    return S_OK;
}
//*******************************************************************************
HRESULT CBCGPBaseRibbonElement::accLocation(long* pxLeft, long* pyTop, long* pcxWidth, long* pcyHeight, VARIANT varChild)
{
    if (!pxLeft || !pyTop || !pcxWidth || !pcyHeight)
    {
        return E_INVALIDARG;
    }

	if (varChild.vt == VT_I4 && varChild.lVal == CHILDID_SELF)
	{		
		CRect rectLocation = m_rect;
		
		CWnd* pParentWnd = m_pRibbonBar;
		if (pParentWnd->GetSafeHwnd() == NULL)
		{
			pParentWnd = GetParentWnd();
			if (pParentWnd->GetSafeHwnd() == NULL)
			{
				return S_FALSE;
			}
		}

		pParentWnd->ClientToScreen(&rectLocation);

		*pxLeft = rectLocation.left;
		*pyTop = rectLocation.top;
		*pcxWidth = rectLocation.Width();
		*pcyHeight = rectLocation.Height();

		return S_OK;
	}

	if (varChild.vt == VT_I4 && varChild.lVal > 0)
	{
		OnSetAccData(varChild.lVal);

		*pxLeft = m_AccData.m_rectAccLocation.left;
		*pyTop = m_AccData.m_rectAccLocation.top;
		*pcxWidth = m_AccData.m_rectAccLocation.Width();
		*pcyHeight = m_AccData.m_rectAccLocation.Height();

		return S_OK;
	}

	return S_FALSE;
}
//*******************************************************************************
HRESULT CBCGPBaseRibbonElement::accNavigate(long navDir, VARIANT varStart, VARIANT* pvarEndUpAt)
{
	if (!IsTabElement())
	{
		return S_FALSE;
	}

	pvarEndUpAt->vt = VT_EMPTY;

    if (varStart.vt != VT_I4)
    {
        return E_INVALIDARG;
    }

	CBCGPRibbonButtonsGroup* pGroup = GetParentGroup();
	if (pGroup == NULL)
	{
		return S_FALSE;
	}

	ASSERT_VALID(pGroup);

	switch (navDir)
	{
	case NAVDIR_NEXT:   
	case NAVDIR_RIGHT:
		if (varStart.lVal == CHILDID_SELF)
		{
			int nIndex = pGroup->GetButtonIndex(this) + 1;
			if (nIndex < pGroup->GetCount())
			{	
				CBCGPBaseRibbonElement* pElement = pGroup->GetButton(nIndex);
				if (pElement != NULL)
				{
					ASSERT_VALID(pElement);

					pvarEndUpAt->vt = VT_DISPATCH;
					pvarEndUpAt->pdispVal = pElement->GetIDispatch(TRUE);

					return S_OK;
				}
			}
			else
			{
				CBCGPRibbonCategory* pCategory = m_pRibbonBar->GetActiveCategory();
				if (pCategory != NULL)
				{
					ASSERT_VALID(pCategory);

					pvarEndUpAt->vt = VT_DISPATCH;
					pvarEndUpAt->pdispVal = pCategory->GetIDispatch(TRUE);

					return S_OK;
				}
			}
		}
		break;

	case NAVDIR_PREVIOUS: 
	case NAVDIR_LEFT:
		if (varStart.lVal == CHILDID_SELF)
		{
			int nIndex = pGroup->GetButtonIndex(this) - 1;
			if (nIndex >= 0)
			{	
				CBCGPBaseRibbonElement* pElement = pGroup->GetButton(nIndex);
				if (pElement != NULL)
				{
					ASSERT_VALID (pElement);

					pvarEndUpAt->vt = VT_DISPATCH;
					pvarEndUpAt->pdispVal = pElement->GetIDispatch(TRUE);

					return S_OK;
				}
			}
			else
			{
				CBCGPRibbonTabsGroup* pTabs = m_pRibbonBar->GetTabs();
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
HRESULT CBCGPBaseRibbonElement::accHitTest(long xLeft, long yTop, VARIANT *pvarChild)
{
	if (!pvarChild)
    {
        return E_INVALIDARG;
    }

	if (m_pRibbonBar->GetSafeHwnd() != NULL)
	{
		pvarChild->vt = VT_I4;
		pvarChild->lVal = CHILDID_SELF;

		CPoint pt(xLeft, yTop);
		m_pRibbonBar->ScreenToClient(&pt);
	}

	return S_OK;
}
//*******************************************************************************
HRESULT CBCGPBaseRibbonElement::accDoDefaultAction(VARIANT /*varChild*/)
{
	OnAccDefaultAction();
	return S_OK;
}

#endif // BCGP_EXCLUDE_RIBBON
