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
// BCGPRibbonPanel.cpp: implementation of the CBCGPRibbonPanel class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "BCGPRibbonPanel.h"
#include "BCGPRibbonCategory.h"
#include "BCGPDrawManager.h"
#include "bcgglobals.h"
#include "BCGPLocalResource.h"
#include "bcgprores.h"
#include "BCGPVisualManager.h"
#include "BCGPToolBar.h"
#include "BCGPRibbonButtonsGroup.h"
#include "BCGPRibbonBar.h"
#include "BCGPRibbonPanelMenu.h"
#include "BCGPRibbonButton.h"
#include "BCGPRibbonPaletteButton.h"
#include "BCGPRibbonLabel.h"
#include "BCGPRibbonUndoButton.h"
#include "BCGPKeyboardManager.h"

#ifndef BCGP_EXCLUDE_RIBBON

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// CBCGPRibbonLaunchButton

IMPLEMENT_DYNCREATE(CBCGPRibbonLaunchButton, CBCGPRibbonButton)


CBCGPRibbonLaunchButton::CBCGPRibbonLaunchButton ()
{
	m_pParentPanel = NULL;
}
//****************************************************************************
void CBCGPRibbonLaunchButton::OnDraw (CDC* pDC)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);

	if (m_rect.IsRectEmpty ())
	{
		return;
	}

	if (m_pParentPanel != NULL)
	{
		ASSERT_VALID (m_pParentPanel);

		CBCGPVisualManager::GetInstance ()->OnDrawRibbonLaunchButton (
			pDC, this, m_pParentPanel);
	}
	else
	{
		CBCGPRibbonButton::OnDraw (pDC);
	}
}
//****************************************************************************
CSize CBCGPRibbonLaunchButton::GetRegularSize (CDC* pDC)
{
	ASSERT_VALID (this);

	if (m_pParentPanel != NULL)
	{
		return CSize (0, 0);
	}

	return CBCGPRibbonButton::GetRegularSize (pDC);
}
//****************************************************************************
void CBCGPRibbonLaunchButton::OnClick (CPoint point)
{
	ASSERT_VALID (this);

	if (m_pParentMenu != NULL)
	{
		ASSERT_VALID (m_pParentMenu);
		m_pParentMenu->OnClickButton (this, point);
		return;
	}

	NotifyCommand ();
}
//****************************************************************************
CRect CBCGPRibbonLaunchButton::GetKeyTipRect (CDC* pDC, BOOL bIsMenu)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);

	if (m_pParentPanel == NULL)
	{
		return CBCGPRibbonButton::GetKeyTipRect (pDC, bIsMenu);
	}

	ASSERT_VALID (m_pParentPanel);

	CSize sizeKeyTip = GetKeyTipSize (pDC);
	CRect rectKeyTip (0, 0, 0, 0);

	if (sizeKeyTip == CSize (0, 0) || m_rect.IsRectEmpty ())
	{
		return rectKeyTip;
	}

	rectKeyTip.top = m_rect.bottom;
	rectKeyTip.right = m_pParentPanel->GetRect ().right;
	rectKeyTip.left = rectKeyTip.right - sizeKeyTip.cx;
	rectKeyTip.bottom = rectKeyTip.top + sizeKeyTip.cy;

	return rectKeyTip;
}
//**********************************************************************************
BOOL CBCGPRibbonLaunchButton::SetACCData (CWnd* pParent, CBCGPAccessibilityData& data)
{
	if (!CBCGPRibbonButton::SetACCData (pParent, data))
	{
		return FALSE;
	}

	data.m_bAccState |= STATE_SYSTEM_HASPOPUP;
	return TRUE;
}
//**********************************************************************************
CBCGPBaseRibbonElement* CBCGPRibbonLaunchButton::CreateCustomCopy()
{
	CBCGPRibbonButton* pButton = new CBCGPRibbonButton();
	pButton->CopyFrom(*this);

	CString strName = pButton->GetText() == NULL ? _T("") : pButton->GetText();
	if (strName.IsEmpty())
	{
		pButton->SetText(pButton->m_strToolTip);
	}

	return pButton;
}

//////////////////////////////////////////////////////////////////////
// CBCGPRibbonDefaultPanelButton

IMPLEMENT_DYNCREATE(CBCGPRibbonDefaultPanelButton, CBCGPRibbonButton)

CBCGPRibbonDefaultPanelButton::CBCGPRibbonDefaultPanelButton (CBCGPRibbonPanel* pPanel)
{
	m_hIcon = NULL;
	m_pPanel = pPanel;
	m_pIconsImageList = NULL;
}
//****************************************************************************
CBCGPRibbonDefaultPanelButton::~CBCGPRibbonDefaultPanelButton ()
{
	if (m_bAutoDestroyIcon && m_hIcon != NULL)
	{
		::DestroyIcon (m_hIcon);
	}
}
//****************************************************************************
void CBCGPRibbonDefaultPanelButton::OnDraw (CDC* pDC)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);

	CBCGPVisualManager::GetInstance ()->OnDrawRibbonDefaultPaneButton 
		(pDC, this);
}
//****************************************************************************
int CBCGPRibbonDefaultPanelButton::AddToListBox (CBCGPRibbonCommandsListBox* pWndListBox, BOOL bDeep)
{
	ASSERT_VALID (this);

	if (m_pPanel != NULL)
	{
		ASSERT_VALID (m_pPanel);
		if (m_pPanel->IsMainPanel ())
		{
			return -1;
		}
	}

	return CBCGPRibbonButton::AddToListBox (pWndListBox, bDeep);
}
//****************************************************************************
CBCGPGridRow* CBCGPRibbonDefaultPanelButton::AddToTree (CBCGPGridCtrl* /*pGrid*/, CBCGPGridRow* /*pParent*/)
{
	return NULL;
}
//****************************************************************************
void CBCGPRibbonDefaultPanelButton::CopyFrom (const CBCGPBaseRibbonElement& s)
{
	ASSERT_VALID (this);

	CBCGPRibbonButton::CopyFrom (s);

	CBCGPRibbonDefaultPanelButton& src = (CBCGPRibbonDefaultPanelButton&) s;

	m_pPanel = src.m_pPanel;
	m_pParent = src.m_pParent;
	m_pIconsImageList = src.m_pIconsImageList;

	if (m_pPanel != NULL)
	{
		ASSERT_VALID (m_pPanel);
		m_strToolTip = m_pPanel->GetDisplayName ();
	}
}
//****************************************************************************
void CBCGPRibbonDefaultPanelButton::OnLButtonDown (CPoint point)
{
	ASSERT_VALID (this);
	
	CBCGPBaseRibbonElement::OnLButtonDown (point);
	OnShowPopupMenu ();
}
//****************************************************************************
void CBCGPRibbonDefaultPanelButton::OnShowPopupMenu ()
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pPanel);

	if (!m_pPanel->IsBackstageView())
	{
		m_pPanel->ShowPopup (this);
	}
}
//********************************************************************************
void CBCGPRibbonDefaultPanelButton::OnDrawOnList (CDC* pDC, CString strText,
									  int nTextOffset, CRect rect,
									  BOOL bIsSelected,
									  BOOL bHighlighted)
{
	BOOL bIsDisabled = m_bIsDisabled;
	m_bIsDisabled = FALSE;

	CRect rectText = rect;

	rectText.left += nTextOffset;
	const int xMargin = 3;
	rectText.DeflateRect (xMargin, 0);

	pDC->DrawText (strText, rectText, DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
	m_bIsDisabled = bIsDisabled;

	CBCGPVisualManager::GetInstance ()->OnDrawRibbonDefaultPaneButtonIndicator
		(pDC, this, rect, bIsSelected, bHighlighted);
}
//*****************************************************************************
void CBCGPRibbonDefaultPanelButton::DrawImage (CDC* pDC, 
	RibbonImageType type, CRect rectImage)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);

	CBCGPRibbonDefaultPanelButton* pOrigButton = DYNAMIC_DOWNCAST (
		CBCGPRibbonDefaultPanelButton, m_pOriginal);

	if (pOrigButton != NULL)
	{
		ASSERT_VALID (pOrigButton);

		pOrigButton->DrawImage (pDC, type, rectImage);
		return;
	}

	if (m_pIconsImageList != NULL && m_nSmallImageIndex >= 0)
	{
		ASSERT_VALID(m_pIconsImageList);

		((CBCGPToolBarImages*)m_pIconsImageList)->DrawEx(pDC, rectImage, m_nSmallImageIndex,
			CBCGPToolBarImages::ImageAlignHorzCenter, CBCGPToolBarImages::ImageAlignVertCenter);
		return;
	}

	if (m_hIcon == NULL)
	{
		CBCGPVisualManager::GetInstance ()->OnDrawDefaultRibbonImage (
			pDC, rectImage);
		return;
	}

	CSize sizeIcon (16, 16);

	if (globalData.GetRibbonImageScale () != 1.)
	{
		sizeIcon.cx = (int) (.5 + globalData.GetRibbonImageScale () * sizeIcon.cx);
		sizeIcon.cy = (int) (.5 + globalData.GetRibbonImageScale () * sizeIcon.cy);
	}

	BOOL bIsRTL = FALSE;

	CBCGPRibbonBar* pTopLevelRibbon = GetTopLevelRibbonBar ();
	if (pTopLevelRibbon != NULL && (pTopLevelRibbon->GetExStyle () & WS_EX_LAYOUTRTL))
	{
		bIsRTL = TRUE;
	}

	if (globalData.GetRibbonImageScale () != 1. || bIsRTL)
	{
		UINT diFlags = DI_NORMAL;

		if (bIsRTL)
		{
			diFlags |= 0x0010 /*DI_NOMIRROR*/;
		}

		::DrawIconEx (pDC->GetSafeHdc (), 
			rectImage.CenterPoint ().x - sizeIcon.cx / 2, 
			rectImage.CenterPoint ().y - sizeIcon.cy / 2, 
			m_hIcon, sizeIcon.cx, sizeIcon.cy, 0, NULL,
			diFlags);
	}
	else
	{
		pDC->DrawState (
			CPoint (
				rectImage.CenterPoint ().x - sizeIcon.cx / 2,
				rectImage.CenterPoint ().y - sizeIcon.cy / 2),
			sizeIcon, m_hIcon, DSS_NORMAL, (HBRUSH) NULL);
	}
}
//*****************************************************************************
BOOL CBCGPRibbonDefaultPanelButton::OnKey (BOOL /*bIsMenuKey*/)
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pPanel);

	if (IsDisabled ())
	{
		return FALSE;
	}

	if (!m_pPanel->m_rect.IsRectEmpty () && !m_pPanel->IsCollapsed () && !IsQATMode ())
	{
		return FALSE;
	}

	OnShowPopupMenu ();

	if (m_pPopupMenu != NULL)
	{
		ASSERT_VALID (m_pPopupMenu);
		m_pPopupMenu->SendMessage (WM_KEYDOWN, VK_DOWN);
	}

	return FALSE;
}
//*****************************************************************************
BOOL CBCGPRibbonDefaultPanelButton::SetACCData (CWnd* pParent, CBCGPAccessibilityData& data)
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pPanel);
	ASSERT_VALID (pParent);

	if (!CBCGPRibbonButton::SetACCData (pParent, data))
	{
		return FALSE;
	}

	if (m_rect.Width () == 0 && m_rect.Height () == 0)
	{
		data.m_nAccRole = ROLE_SYSTEM_TOOLBAR;
		data.m_strAccValue = _T("group");
		data.m_rectAccLocation = m_pPanel->GetRect ();
		pParent->ClientToScreen (&data.m_rectAccLocation);
		data.m_bAccState = 0;
		data.m_strAccDefAction = _T("");
		return TRUE;
	}

	data.m_nAccRole = ROLE_SYSTEM_BUTTONDROPDOWNGRID;
	data.m_bAccState |= STATE_SYSTEM_HASPOPUP;
	data.m_strAccDefAction = _T("Open");

	if (IsDroppedDown ())
	{
		data.m_bAccState |= STATE_SYSTEM_PRESSED;
		data.m_strAccDefAction = _T("Close");
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CBCGPRibbonPanel

UINT CBCGPRibbonPanel::m_nNextPanelID = (UINT)-10;

IMPLEMENT_DYNCREATE(CBCGPRibbonPanel, CBCGPBaseAccessibleObject)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#pragma warning (disable : 4355)

CBCGPRibbonPanel::CBCGPRibbonPanel(LPCTSTR lpszName, HICON hIcon, BOOL bAutoDestroyIcon) :
	m_btnDefault (this)
{
	CommonInit (lpszName, hIcon, bAutoDestroyIcon);
}
//****************************************************************************
CBCGPRibbonPanel::CBCGPRibbonPanel(LPCTSTR lpszName, const CBCGPToolBarImages& icons, int nIconIndex) :
	m_btnDefault (this)
{
	CommonInit (lpszName, NULL, FALSE, &icons, nIconIndex);
}
//****************************************************************************
CBCGPRibbonPanel::CBCGPRibbonPanel (CBCGPRibbonPaletteButton* pPaletteButton) :
	m_btnDefault (this)
{
	CommonInit ();

	ASSERT_VALID (pPaletteButton);
	m_pPaletteButton = pPaletteButton;
}
//****************************************************************************
CBCGPRibbonPanel* CBCGPRibbonPanel::CreateCustomCopy(CBCGPRibbonCategory* pParent)
{
	ASSERT_VALID(this);

	CBCGPRibbonPanel* pCustomPanel = (CBCGPRibbonPanel*)GetRuntimeClass()->CreateObject();
	ASSERT_VALID(pCustomPanel);

	pCustomPanel->CopyFrom(*this);

	pCustomPanel->m_bToBeDeleted = FALSE;	
	pCustomPanel->m_pOriginal = this;
	pCustomPanel->m_pParent = pParent;
	pCustomPanel->m_bIsCustom = FALSE;

	for (int i = 0; i < pCustomPanel->GetCount(); i++)
	{
		CBCGPBaseRibbonElement* pElem = pCustomPanel->GetElement(i);
		ASSERT_VALID(pElem);

		if (g_pUserToolsManager != NULL && g_pUserToolsManager->IsUserToolCmd(pElem->GetID()))
		{
			// Don't copy user-defined tools:
			pCustomPanel->Remove(i, TRUE);
			i--;
		}
		else
		{
			pElem->SetParentCategory(pParent);
		}
	}

	m_btnLaunch.SetParentCategory(pParent);
	
	return pCustomPanel;
}
//****************************************************************************
void CBCGPRibbonPanel::CopyFrom (CBCGPRibbonPanel& src)
{
	m_strName = src.m_strName;
	m_sizePadding = src.m_sizePadding;
	m_dwData = src.m_dwData;
	m_nKey = src.m_nKey;
	m_bIsCustom = src.m_bIsCustom;
	m_bIsNew = src.m_bIsNew;
	m_bToBeDeleted = src.m_bToBeDeleted;
	m_pParent = src.m_pParent;
	m_nXMargin = src.m_nXMargin;
	m_nYMargin = src.m_nYMargin;
	m_bShowCaption = src.m_bShowCaption;
	m_bAlignByColumn = src.m_bAlignByColumn;
	m_bCenterColumnVert = src.m_bCenterColumnVert;
	m_bJustifyColumns = src.m_bJustifyColumns;
	m_nMaxRows = src.m_nMaxRows;
	m_bPreserveElementOrder = src.m_bPreserveElementOrder;
	m_bNonCollapsible = src.m_bNonCollapsible;

	int i = 0;

	for (i = 0; i < src.m_arWidths.GetSize (); i++)
	{
		m_arWidths.Add (src.m_arWidths [i]);
	}

	for (i = 0; i < src.m_arElements.GetSize (); i++)
	{
		CBCGPBaseRibbonElement* pSrcElem = src.m_arElements [i];
		ASSERT_VALID (pSrcElem);

		CBCGPBaseRibbonElement* pElem =
			(CBCGPBaseRibbonElement*) pSrcElem->GetRuntimeClass ()->CreateObject ();
		ASSERT_VALID (pElem);

		pElem->CopyFrom (*pSrcElem);
		pElem->SetOriginal (pSrcElem);

		m_arElements.Add (pElem);
	}

	m_btnLaunch.CopyFrom (src.m_btnLaunch);
	m_btnLaunch.SetOriginal (&src.m_btnLaunch);
}
//****************************************************************************
void CBCGPRibbonPanel::CommonInit (LPCTSTR lpszName, HICON hIcon, BOOL bAutoDestroyIcon, const CBCGPToolBarImages* pIconsImageList, int nIconIndex)
{
	m_strName = lpszName != NULL ? lpszName : _T("");

	int nIndex = m_strName.Find (_T('\n'));
	if (nIndex >= 0)
	{
		m_btnDefault.SetKeys (m_strName.Mid (nIndex + 1));
		m_strName = m_strName.Left (nIndex);
	}

	m_dwData = 0;
	m_sizePadding = CSize(0, 0);
	m_nKey = -1;
	m_pOriginal = NULL;
	m_bIsCustom = FALSE;
	m_bIsNew = FALSE;
	m_bToBeDeleted = FALSE;

	m_btnDefault.m_bAutoDestroyIcon = bAutoDestroyIcon;
	m_btnDefault.m_hIcon = hIcon;
	m_btnDefault.m_nSmallImageIndex = nIconIndex;
	m_btnDefault.m_pIconsImageList = pIconsImageList;
	m_btnDefault.SetText (m_strName);

	// jump across a ribbon (system) id
	if ((UINT)-108 <= m_nNextPanelID && m_nNextPanelID <= (UINT)-102)
	{
		m_nNextPanelID = (UINT)-109;
	}

	m_btnDefault.SetID (m_nNextPanelID--);

	m_rect.SetRectEmpty ();
	m_pParent = NULL;
	m_pParentMenuBar = NULL;
	m_nCurrWidthIndex = 0;
	m_nFullWidth = 0;
	m_nRows = 0;
	m_nXMargin = 4;
	m_nYMargin = 2;
	m_bShowCaption = FALSE;
	m_bForceCollpapse = FALSE;
	m_bIsHighlighted = FALSE;
	m_bIsCalcWidth = FALSE;
	m_pHighlighted = NULL;
	m_bAlignByColumn = TRUE;
	m_bCenterColumnVert = FALSE;
	m_bFloatyMode = FALSE;
	m_bIsQATPopup = FALSE;
	m_bMenuMode = FALSE;
	m_bIsDefaultMenuLook = FALSE;
	m_rectCaption.SetRectEmpty ();
	m_pPaletteButton = NULL;
	m_rectMenuAreaTop.SetRectEmpty ();
	m_rectMenuAreaBottom.SetRectEmpty ();
	m_pScrollBar = NULL;
	m_pScrollBarHorz = NULL;
	m_nScrollOffset = 0;
	m_nScrollOffsetHorz = 0;
	m_bSizeIsLocked = FALSE;
	m_bJustifyColumns = FALSE;
	m_nMaxRows = 3;
	m_bOnDialogBar = FALSE;
	m_bScrollDnAvailable = FALSE;
	m_bTrancateCaption = FALSE;
	m_bPreserveElementOrder = FALSE;
	m_bNavigateSearchResultsOnly = FALSE;
	m_bMouseIsDown = FALSE;
	m_bNonCollapsible = FALSE;
}

#pragma warning (default : 4355)

CBCGPRibbonPanel::~CBCGPRibbonPanel()
{
	CBCGPBaseRibbonElement* pDroppedDown = GetDroppedDown ();
	if (pDroppedDown != NULL)
	{
		ASSERT_VALID (pDroppedDown);
		pDroppedDown->ClosePopupMenu ();
	}

	RemoveAll ();
}
//********************************************************************************
void CBCGPRibbonPanel::EnableLaunchButton (UINT uiCmdID,
										   int nIconIndex,
										   LPCTSTR lpszKeys)
{
	ASSERT_VALID (this);

	m_btnLaunch.SetID (uiCmdID);
	m_btnLaunch.m_nSmallImageIndex = nIconIndex;
	m_btnLaunch.SetKeys (lpszKeys);
}
//********************************************************************************
void CBCGPRibbonPanel::Add (CBCGPBaseRibbonElement* pElem)
{
	Insert (pElem, (int) m_arElements.GetSize ());
}
//********************************************************************************
void CBCGPRibbonPanel::AddSeparator ()
{
	InsertSeparator ((int) m_arElements.GetSize ());
}
//********************************************************************************
BOOL CBCGPRibbonPanel::Insert (CBCGPBaseRibbonElement* pElem, int nIndex)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pElem);

	if (nIndex == -1)
	{
		nIndex = (int) m_arElements.GetSize ();
	}

	if (nIndex < 0 || nIndex > m_arElements.GetSize ())
	{
		return FALSE;
	}

	pElem->SetParentCategory (m_pParent);

	if (!pElem->IsAlignByColumn () && m_bAlignByColumn)
	{
		//---------------------------------------------------
		// If 2 or more elements are aligned by row, set this
		// flag for whole panel:
		//---------------------------------------------------
		for (int i = 0; i < m_arElements.GetSize (); i++)
		{
			CBCGPBaseRibbonElement* pListElem = m_arElements [i];
			ASSERT_VALID (pListElem);

			if (!pListElem->IsAlignByColumn ())
			{
				m_bAlignByColumn = FALSE;
				break;
			}
		}
	}

	if (nIndex == m_arElements.GetSize ())
	{
		m_arElements.Add (pElem);
	}
	else
	{
		m_arElements.InsertAt (nIndex, pElem);
	}

	pElem->m_bOnDialogBar = m_bOnDialogBar;

	if (m_bIsCustom)
	{
		pElem->SetCustom();
	}

	return TRUE;
}
//********************************************************************************
BOOL CBCGPRibbonPanel::InsertSeparator (int nIndex)
{
	ASSERT_VALID (this);

	if (nIndex < 0 || nIndex > m_arElements.GetSize ())
	{
		return FALSE;
	}

	CBCGPRibbonSeparator* pSeparator = new CBCGPRibbonSeparator;
	ASSERT_VALID (pSeparator);

	pSeparator->m_pParent = m_pParent;

	if (nIndex == m_arElements.GetSize ())
	{
		m_arElements.Add (pSeparator);
	}
	else
	{
		m_arElements.InsertAt (nIndex, pSeparator);
	}

	return TRUE;
}
//********************************************************************************
CBCGPRibbonButtonsGroup* CBCGPRibbonPanel::AddToolBar (	
									UINT uiToolbarResID, UINT uiColdResID,
									UINT uiHotResID, UINT uiDisabledResID)
{
	ASSERT_VALID (this);

	//-------------------------------------------
	// Create temporary toolbar and load bitmaps:
	//-------------------------------------------
	CBCGPToolBar wndToolbar;
	if (!wndToolbar.LoadToolBar (uiToolbarResID, uiColdResID, 0, 
								TRUE, uiDisabledResID, 0, uiHotResID))
	{
		return NULL;
	}

	CBCGPToolBarImages* pImages = wndToolbar.GetLockedImages ();
	CBCGPToolBarImages* pColdImages = wndToolbar.GetLockedColdImages ();
	CBCGPToolBarImages* pDisabledImages = wndToolbar.GetLockedDisabledImages ();
	CBCGPToolBarImages* pHotImages = NULL;

	if (pColdImages != NULL && pColdImages->GetCount () > 0)
	{
		pHotImages = uiHotResID != 0 ? pImages : NULL;
		pImages = pColdImages;
	}

	CList<CBCGPBaseRibbonElement*, CBCGPBaseRibbonElement*>	lstButtons;

	for (int i = 0; i < wndToolbar.GetCount (); i++)
	{
		CBCGPToolbarButton* pToolbarButton = wndToolbar.GetButton (i);
		ASSERT_VALID (pToolbarButton);

		if (pToolbarButton->m_nStyle & TBBS_SEPARATOR)
		{
			if (!lstButtons.IsEmpty ())
			{
				CBCGPRibbonButtonsGroup* pGroup = new CBCGPRibbonButtonsGroup;

				pGroup->AddButtons (lstButtons);
				pGroup->SetImages (pImages, pHotImages, pDisabledImages);

				Add (pGroup);

				lstButtons.RemoveAll ();
			}
		}
		else
		{
			CBCGPRibbonButton* pButton = new CBCGPRibbonButton;

			pButton->SetID (pToolbarButton->m_nID);
			pButton->SetText (pToolbarButton->m_strText);
			pButton->m_nSmallImageIndex = pToolbarButton->GetImage ();

			lstButtons.AddTail (pButton);
		}
	}

	if (!lstButtons.IsEmpty ())
	{
		CBCGPRibbonButtonsGroup* pGroup = new CBCGPRibbonButtonsGroup;

		pGroup->AddButtons (lstButtons);
		pGroup->SetImages (pImages, pHotImages, pDisabledImages);

		Add (pGroup);
		return pGroup;
	}

	return NULL;
}
//********************************************************************************
CBCGPBaseRibbonElement* CBCGPRibbonPanel::GetElement (int nIndex) const
{
	ASSERT_VALID (this);

	if (nIndex < 0 || nIndex >= m_arElements.GetSize ())
	{
		return NULL;
	}

	return m_arElements [nIndex];
}
//********************************************************************************
int CBCGPRibbonPanel::GetCount () const
{
	ASSERT_VALID (this);
	return (int) m_arElements.GetSize ();
}
//********************************************************************************
BOOL CBCGPRibbonPanel::Remove (int nIndex, BOOL bDelete)
{
	ASSERT_VALID (this);

	if (nIndex < 0 || nIndex >= m_arElements.GetSize ())
	{
		ASSERT (FALSE);
		return FALSE;
	}

	CBCGPBaseRibbonElement* pElem = m_arElements [nIndex];
	ASSERT_VALID (pElem);

	if (pElem == m_pHighlighted)
	{
		m_pHighlighted = NULL;
	}

	m_arElements.RemoveAt (nIndex);

	if (bDelete)
	{
		delete pElem;
	}

	if (!m_bAlignByColumn)
	{
		int nCount = 0;

		for (int i = 0; i < m_arElements.GetSize () && nCount < 2; i++)
		{
			CBCGPBaseRibbonElement* pListElem = m_arElements [i];
			ASSERT_VALID (pListElem);

			if (!pListElem->IsAlignByColumn ())
			{
				nCount++;
			}
		}

		if (nCount < 2)
		{
			m_bAlignByColumn = TRUE;
		}
	}

	return TRUE;
}
//********************************************************************************
void CBCGPRibbonPanel::RemoveAll ()
{
	ASSERT_VALID (this);

	for (int i = 0; i < m_arElements.GetSize (); i++)
	{
		delete m_arElements [i];
	}

	m_arElements.RemoveAll ();
	m_bAlignByColumn = TRUE;
	m_pHighlighted = NULL;
}
//********************************************************************************
void CBCGPRibbonPanel::DoPaint (CDC* pDC)
{
	ASSERT_VALID (pDC);

	if (m_rect.IsRectEmpty ())
	{
		return;
	}

	CRect rectClip;
	pDC->GetClipBox (rectClip);

	CRect rectInter;

	if (!rectInter.IntersectRect (m_rect, rectClip))
	{
		return;
	}

	COLORREF clrTextOld = pDC->GetTextColor ();

	//-----------------------
	// Fill panel background:
	//-----------------------
	COLORREF clrText = m_pParent == NULL || m_pPaletteButton != NULL ? 
		globalData.clrBarText :
		CBCGPVisualManager::GetInstance ()->OnDrawRibbonPanel (
		pDC, this, m_rect, m_rectCaption);

	//--------------------
	// Draw panel caption:
	//--------------------
	if (!m_rectCaption.IsRectEmpty () &&
		rectInter.IntersectRect (m_rectCaption, rectClip))
	{
		CBCGPVisualManager::GetInstance ()->OnDrawRibbonPanelCaption (
			pDC, this, m_rectCaption);
	}

	//--------------------
	// Draw launch button:
	//--------------------
	if (rectInter.IntersectRect (m_btnLaunch.GetRect (), rectClip))
	{
		m_btnLaunch.OnDraw (pDC);
	}

	pDC->SetTextColor (clrText);

	if (!m_btnDefault.GetRect ().IsRectEmpty ())
	{
		//----------------------------------------------
		// Panel is collapsed, draw default button only:
		//----------------------------------------------
		if (rectInter.IntersectRect (m_btnDefault.GetRect (), rectClip))
		{
			m_btnDefault.OnDraw (pDC);
		}
	}
	else if (m_pPaletteButton != NULL)
	{
		OnDrawPaletteMenu (pDC);
	}
	else
	{
		if (m_bIsDefaultMenuLook && m_pParentMenuBar != NULL)
		{
			ASSERT_VALID (m_pParentMenuBar);

			BOOL bDisableSideBarInXPMode = m_pParentMenuBar->m_bDisableSideBarInXPMode;
			m_pParentMenuBar->m_bDisableSideBarInXPMode = FALSE;

			CBCGPVisualManager::GetInstance ()->OnFillBarBackground (
				pDC, m_pParentMenuBar, m_rect, m_rect);

			m_pParentMenuBar->m_bDisableSideBarInXPMode = bDisableSideBarInXPMode;
		}

		//---------------------
		// Draw panel elements:
		//---------------------
		CRect rectSeparator(0, 0, 0, 0);
		int nPrevRow = -1;

		BOOL bDrawGroupSeparator = CBCGPVisualManager::GetInstance ()->IsDrawRibbonGroupSeparator();

		for (int i = 0; i < m_arElements.GetSize (); i++)
		{
			CBCGPBaseRibbonElement* pElem = m_arElements [i];
			ASSERT_VALID (pElem);

			if (nPrevRow == pElem->m_nRow && !rectSeparator.IsRectEmpty())
			{
				CBCGPVisualManager::GetInstance ()->OnDrawRibbonGroupSeparator (pDC, rectSeparator);
			}

			if (rectInter.IntersectRect (pElem->GetRect (), rectClip))
			{
				pDC->SetTextColor (clrText);

				BOOL bIsHighlighted = pElem->m_bIsHighlighted;

				if (IsMenuMode () && pElem->IsDroppedDown () && m_pHighlighted == NULL)
				{
					pElem->m_bIsHighlighted = TRUE;
				}

				pElem->OnDraw (pDC);

				if (bDrawGroupSeparator && pElem->IsKindOf(RUNTIME_CLASS(CBCGPRibbonButtonsGroup)) && pElem->CanBeSeparated ())
				{
					rectSeparator = pElem->GetRect ();
					rectSeparator.left = rectSeparator.right;
					rectSeparator.right = rectSeparator.left + CBCGPVisualManager::GetInstance ()->GetRibbonButtonsGroupHorzMargin();

					nPrevRow = pElem->m_nRow;
				}
				else
				{
					rectSeparator.SetRectEmpty();
					nPrevRow = -1;
				}

				pElem->m_bIsHighlighted = bIsHighlighted;
			}
		}
	}

	pDC->SetTextColor (clrTextOld);
}
//********************************************************************************
void CBCGPRibbonPanel::OnDrawPaletteMenu (CDC* pDC)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);
	ASSERT_VALID (m_pPaletteButton);

	const BOOL bNoSideBar = m_pPaletteButton->IsKindOf (RUNTIME_CLASS (CBCGPRibbonUndoButton));
	const BOOL bIsXPSideBar = !bNoSideBar && m_pPaletteButton->IsMenuSideBar ();

	int i = 0;

	CRect rectIcons = m_rect;

	CRect rectSeparatorBottom;
	rectSeparatorBottom.SetRectEmpty ();

	CRect rectSeparatorTop;
	rectSeparatorTop.SetRectEmpty ();

	if (!m_rectMenuAreaBottom.IsRectEmpty ())
	{
		if (m_pParentMenuBar != NULL && !bIsXPSideBar && !bNoSideBar)
		{
			BOOL bDisableSideBarInXPMode = m_pParentMenuBar->m_bDisableSideBarInXPMode;
			m_pParentMenuBar->m_bDisableSideBarInXPMode = FALSE;

			CBCGPVisualManager::GetInstance ()->OnFillBarBackground (
				pDC, m_pParentMenuBar, m_rectMenuAreaBottom, m_rectMenuAreaBottom);

			m_pParentMenuBar->m_bDisableSideBarInXPMode = bDisableSideBarInXPMode;
		}

		rectSeparatorBottom = m_rectMenuAreaBottom;
		rectSeparatorBottom.top--;
		rectSeparatorBottom.bottom = rectSeparatorBottom.top + 1;

		rectIcons.bottom = m_rectMenuAreaBottom.top - 1;
	}

	if (!m_rectMenuAreaTop.IsRectEmpty ())
	{
		if (m_pParentMenuBar != NULL && !bIsXPSideBar)
		{
			BOOL bDisableSideBarInXPMode = m_pParentMenuBar->m_bDisableSideBarInXPMode;
			m_pParentMenuBar->m_bDisableSideBarInXPMode = FALSE;

			CBCGPVisualManager::GetInstance ()->OnFillBarBackground (
				pDC, m_pParentMenuBar, m_rectMenuAreaTop, m_rectMenuAreaTop);

			m_pParentMenuBar->m_bDisableSideBarInXPMode = bDisableSideBarInXPMode;
		}

		rectSeparatorTop = m_rectMenuAreaTop;
		rectSeparatorTop.bottom++;
		rectSeparatorTop.top = rectSeparatorTop.bottom - 1;

		rectIcons.top = m_rectMenuAreaTop.bottom + 1;
	}

	if (m_pParentMenuBar != NULL && bIsXPSideBar)
	{
		BOOL bDisableSideBarInXPMode = m_pParentMenuBar->m_bDisableSideBarInXPMode;
		m_pParentMenuBar->m_bDisableSideBarInXPMode = FALSE;

		CBCGPVisualManager::GetInstance ()->OnFillBarBackground (
			pDC, m_pParentMenuBar, m_rect, m_rect);

		m_pParentMenuBar->m_bDisableSideBarInXPMode = bDisableSideBarInXPMode;
	}

	CRgn rgnClip;

	rgnClip.CreateRectRgnIndirect (rectIcons);
	pDC->SelectClipRgn (&rgnClip);

	CBCGPDrawState ds;

	if (m_pPaletteButton->m_imagesPalette.GetCount () > 0)
	{
		m_pPaletteButton->m_imagesPalette.SetTransparentColor (globalData.clrBtnFace);
		m_pPaletteButton->m_imagesPalette.PrepareDrawImage (ds, m_pPaletteButton->GetIconSize ());
	}

	// First, draw icons + labels:
	for (i = 0; i < m_arElements.GetSize (); i++)
	{
		CBCGPBaseRibbonElement* pElem = m_arElements [i];
		ASSERT_VALID (pElem);

		BOOL bIsLabel = pElem->IsKindOf (RUNTIME_CLASS (CBCGPRibbonLabel));
		BOOL bIsIcon = pElem->IsKindOf (RUNTIME_CLASS (CBCGPRibbonPaletteIcon));

		if (bIsLabel || bIsIcon)
		{
			pElem->OnDraw (pDC);
		}
	}

	pDC->SelectClipRgn (NULL);

	// Draw rest of elements:
	for (i = 0; i < m_arElements.GetSize (); i++)
	{
		CBCGPBaseRibbonElement* pElem = m_arElements [i];
		ASSERT_VALID (pElem);

		BOOL bIsLabel = pElem->IsKindOf (RUNTIME_CLASS (CBCGPRibbonLabel));
		BOOL bIsIcon = pElem->IsKindOf (RUNTIME_CLASS (CBCGPRibbonPaletteIcon));

		if (!bIsLabel && !bIsIcon)
		{
			pElem->m_bIsDefaultMenuLook = TRUE;
			pElem->OnDraw (pDC);
		}
	}

	if (!rectSeparatorTop.IsRectEmpty ())
	{
		CBCGPVisualManager::GetInstance ()->OnDrawSeparator (pDC, 
			m_pParentMenuBar, rectSeparatorTop, FALSE);
	}

	if (!rectSeparatorBottom.IsRectEmpty ())
	{
		CBCGPVisualManager::GetInstance ()->OnDrawSeparator (pDC, 
			m_pParentMenuBar, rectSeparatorBottom, FALSE);
	}

	if (m_pPaletteButton->m_imagesPalette.GetCount () > 0)
	{
		m_pPaletteButton->m_imagesPalette.EndDrawImage (ds);
	}
}
//********************************************************************************
int CBCGPRibbonPanel::GetHeight (CDC* pDC) const
{
	const int nVertMargin = 3;

	ASSERT_VALID (pDC);

	((CBCGPRibbonPanel*)this)->m_btnDefault.OnCalcTextSize (pDC);

	int nRowHeight = 0;
	if (m_pParent != NULL)
	{
		ASSERT_VALID (m_pParent);

		TEXTMETRIC tm;
		pDC->GetTextMetrics (&tm);

		nRowHeight = max (m_pParent->GetImageSize (FALSE).cy, tm.tmHeight) + 2 * nVertMargin + m_sizePadding.cy;
	}

	int nMaxHeight = max (nRowHeight * m_nMaxRows, ((CBCGPRibbonPanel*)this)->m_btnDefault.GetRegularSize (pDC).cy);

	for (int i = 0; i < m_arElements.GetSize (); i++)
	{
		CBCGPBaseRibbonElement* pElem = m_arElements [i];
		ASSERT_VALID (pElem);

		pElem->OnCalcTextSize (pDC);
		nMaxHeight = max (nMaxHeight, pElem->GetRegularSize (pDC).cy);
	}

	return nMaxHeight + 2 * m_nYMargin + nVertMargin;
}
//********************************************************************************
void CBCGPRibbonPanel::Repos (CDC* pDC, const CRect& rect)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);

	m_rectCaption.SetRectEmpty ();
	m_rectMenuAreaTop.SetRectEmpty ();
	m_rectMenuAreaBottom.SetRectEmpty ();

	if (m_pPaletteButton != NULL)
	{
		ReposPalette (pDC, rect);
		return;
	}

	if (m_bMenuMode)
	{
		ReposMenu (pDC, rect);
		return;
	}

	m_btnDefault.m_pParent = m_pParent;
	m_btnLaunch.m_pParent = m_pParent;
	m_btnLaunch.m_pParentPanel = this;

	m_btnDefault.OnCalcTextSize (pDC);
	const int cxDefaultButton = m_btnDefault.GetRegularSize (pDC).cx;

	m_rect = rect;

	m_btnLaunch.SetRect (CRect (0, 0, 0, 0));

	if (m_bForceCollpapse)
	{
		ASSERT (!m_bIsQATPopup);

		ShowDefaultButton (pDC);
		return;
	}

	m_btnDefault.SetRect (CRect (0, 0, 0, 0));

	m_nFullWidth = 0;
	m_nRows = 0;
	m_bShowCaption = TRUE;

	const CSize sizeCaption = GetCaptionSize (pDC);

	if (!m_bTrancateCaption)
	{
		m_rect.right = m_rect.left + max (rect.Width (), sizeCaption.cx);
	}

	CSize size = rect.Size ();
	size.cx -= m_nXMargin;
	size.cy -= sizeCaption.cy + m_nYMargin;

	//---------------------------------------------
	// First, set all elements to the initial mode:
	//---------------------------------------------
	for (int i = 0; i < m_arElements.GetSize (); i++)
	{
		CBCGPBaseRibbonElement* pElem = m_arElements [i];
		ASSERT_VALID (pElem);

		pElem->OnCalcTextSize (pDC);

		if (!m_bFloatyMode)
		{
			pElem->SetInitialMode ();
		}
		else
		{
			pElem->SetCompactMode (TRUE);
		}

		pElem->m_bFloatyMode = m_bFloatyMode;
		pElem->m_nRow = -1;
	}

	//----------------------------------------------
	// Now, try to repos all elements inside bounds:
	//----------------------------------------------
	int x = 0;
	int y = 0;

	if (!m_bAlignByColumn || m_bFloatyMode)
	{
		int nRowHeight = 0;
		int nBottom = 0;
		int xStart = 0;
		int i = 0;
		BOOL bIsFullHeight = (m_rect.bottom == 32767);
		int cx = size.cx;

		int cxFull = 0;

		if (!m_bIsCalcWidth && m_mapNonOptWidths.Lookup (cx, cxFull))
		{
			cx = cxFull;
		}

		CArray<int,int>	arRowWidths;

		if (!m_bFloatyMode)
		{
			//-----------------------------
			// Put all large buttons first:
			//-----------------------------
			BOOL bPrevLarge = FALSE;
			BOOL bPrevSeparator = FALSE;

			CSize sizePrevLargeButton (0, 0);

			for (i = 0; i < m_arElements.GetSize (); i++)
			{
				CBCGPBaseRibbonElement* pElem = m_arElements [i];
				ASSERT_VALID (pElem);

				CSize sizeElem = pElem->GetSize (pDC);

				BOOL bIsLargeButton = pElem->HasLargeMode () &&
					!pElem->m_bCompactMode && !pElem->m_bIntermediateMode;

				BOOL bDrawSeparator = FALSE;

				if (pElem->IsSeparator ())
				{
					bDrawSeparator = bPrevLarge && !bPrevSeparator;
				}

				if (bIsLargeButton || bDrawSeparator)
				{
					if (pElem->IsSeparator ())
					{
						if (sizePrevLargeButton != CSize (0, 0))
						{
							sizeElem.cy = sizePrevLargeButton.cy;
						}
					}
					else
					{
						sizePrevLargeButton = sizeElem;
					}

					CRect rectElem (
						CPoint (rect.left + x + m_nXMargin, rect.top + m_nYMargin), 
						CSize (sizeElem.cx, bIsFullHeight ? sizeElem.cy : size.cy));

					pElem->SetRect (rectElem);
					pElem->m_nRow = 999;

					x += sizeElem.cx + m_nXMargin;
					xStart = x;
					y = 0;
				}

				bPrevLarge = bIsLargeButton;
				bPrevSeparator = bDrawSeparator;
			}
		}

		for (i = 0; i < m_arElements.GetSize (); i++)
		{
			CBCGPBaseRibbonElement* pElem = m_arElements [i];
			ASSERT_VALID (pElem);

			CSize sizeElem = pElem->GetSize (pDC);

			if (sizeElem == CSize (0, 0))
			{
				pElem->SetRect (CRect (0, 0, 0, 0));
				continue;
			}

			if (pElem->m_nRow != -1)
			{
				//--------------------
				// Already positioned
				//--------------------
				continue;
			}

			if (pElem->IsSeparator ())
			{
				pElem->SetRect (CRect (0, 0, 0, 0));
				continue;
			}

			if (x + sizeElem.cx + m_nXMargin - 1 > cx)
			{
				//------------------------------
				// We should start next row now:
				//------------------------------

				if (x == xStart)
				{
					ShowDefaultButton (pDC);
					return;
				}

				y += nRowHeight;

				if (m_bFloatyMode)
				{
					y += m_nYMargin;
				}

				arRowWidths.Add (x);

				m_nRows++;

				x = xStart;
				nRowHeight = 0;
			}

			if (y + sizeElem.cy > size.cy)
			{
				//------------------------------------------
				// Cannot repos elemnts: panel is too small:
				//------------------------------------------
				ShowDefaultButton (pDC);
				return;
			}

			CRect rectElem (CPoint (rect.left + x + m_nXMargin, rect.top + y + m_nYMargin), 
							sizeElem);

			pElem->SetRect (rectElem);
			pElem->m_nRow = m_nRows;

			nRowHeight = max (nRowHeight, sizeElem.cy);
			x += sizeElem.cx + m_nXMargin - 1;

			m_nFullWidth = max (m_nFullWidth, x - 1);
			
			nBottom = max (nBottom, rectElem.bottom);
		}

		arRowWidths.Add (x);
		m_nRows++;

		if (bIsFullHeight)
		{
			m_rect.bottom = nBottom + sizeCaption.cy + m_nYMargin;
			size.cy = m_rect.Height () - sizeCaption.cy - m_nYMargin;
		}

		if (!m_bIsQATPopup && m_nRows > 1 && !m_bPreserveElementOrder)
		{
			//-----------------------------
			// Optimize elemnents location:
			//-----------------------------
			BOOL bRecalcFullWidth = FALSE;

			while (TRUE)
			{
				//-----------------
				// Find widest row:
				//-----------------
				int nMaxRowWidth = 0;
				int nMaxRowIndex = -1;

				for (i = 0; i < arRowWidths.GetSize (); i++)
				{
					if (arRowWidths [i] > nMaxRowWidth)
					{
						nMaxRowWidth = arRowWidths [i];
						nMaxRowIndex = i;
					}
				}

				if (nMaxRowIndex < 0)
				{
					break;
				}

				//-----------------------------------------
				// Find smallest element in the widest row:
				//-----------------------------------------
				int nMinWidth = 9999;
				CBCGPBaseRibbonElement* pMinElem = NULL;

				for (i = 0; i < m_arElements.GetSize (); i++)
				{
					CBCGPBaseRibbonElement* pElem = m_arElements [i];
					ASSERT_VALID (pElem);

					if (pElem->m_nRow == nMaxRowIndex)
					{
						CRect rectElem = pElem->GetRect ();

						if (!rectElem.IsRectEmpty () && rectElem.Width () < nMinWidth)
						{
							nMinWidth = rectElem.Width ();
							pMinElem = pElem;
						}
					}
				}

				if (pMinElem == NULL)
				{
					break;
				}

				//----------------------------------------
				// Try to move this elemnt to another row:
				//----------------------------------------
				BOOL bMoved = FALSE;

				for (i = nMaxRowIndex + 1; i < arRowWidths.GetSize (); i++)
				{
					if (arRowWidths [i] + nMinWidth < nMaxRowWidth)
					{
						//--------------------------------------
						// There is enough space in current row,
						// move element to here
						//--------------------------------------

						int x = 0;
						int y = 0;

						for (int j = 0; j < m_arElements.GetSize (); j++)
						{
							CBCGPBaseRibbonElement* pElem = m_arElements [j];
							ASSERT_VALID (pElem);

							if (pElem->m_nRow == i)
							{
								x = max (pElem->GetRect ().right + m_nXMargin, x);
								y = pElem->GetRect ().top;
							}
							else if (pElem->m_nRow == nMaxRowIndex)
							{
								CRect rectElem = pElem->GetRect ();

								if (rectElem.left > pMinElem->GetRect ().left)
								{
									rectElem.OffsetRect (-(nMinWidth + m_nXMargin), 0);
									pElem->SetRect (rectElem);
								}
							}
						}

						pMinElem->SetRect (CRect (CPoint (x, y), pMinElem->GetRect ().Size ()));
						pMinElem->m_nRow = i;

						arRowWidths [i] += nMinWidth;
						arRowWidths [nMaxRowIndex] -= nMinWidth;

						bRecalcFullWidth = TRUE;
						bMoved = TRUE;
						break;
					}
				}

				if (!bMoved)
				{
					break;
				}
			}

			if (bRecalcFullWidth)
			{
				int nFullWidthSaved = m_nFullWidth;

				m_nFullWidth = 0;

				for (i = 0; i < m_arElements.GetSize (); i++)
				{
					CBCGPBaseRibbonElement* pElem = m_arElements [i];
					ASSERT_VALID (pElem);

					m_nFullWidth = max (m_nFullWidth, pElem->GetRect ().right);
				}

				m_nFullWidth -= m_rect.left + m_nXMargin;

				if (abs(nFullWidthSaved - m_nFullWidth) <= 32)
				{
					m_nFullWidth = nFullWidthSaved;
				}
			}
		}

		if (!bIsFullHeight && !m_bFloatyMode && m_nRows > 1)
		{
			//-----------------------------
			// Add some space between rows:
			//-----------------------------
			int yOffset = (size.cy - m_nRows * nRowHeight) / (m_nRows + 1);
			if (yOffset > 0)
			{
				for (i = 0; i < m_arElements.GetSize (); i++)
				{
					CBCGPBaseRibbonElement* pElem = m_arElements [i];
					ASSERT_VALID (pElem);

					int nRow = pElem->m_nRow;
					CRect rectElem = pElem->GetRect ();

					if (nRow != 999 && !rectElem.IsRectEmpty ())
					{
						rectElem.OffsetRect (0, yOffset * (nRow + 1) - nRow);
						pElem->SetRect (rectElem);
					}
				}
			}
		}

		if (m_bIsQATPopup && nRowHeight > 0 && m_arElements.GetSize () > 0)
		{
			//--------------------------------------------------------------------
			// Last element (customize button) should occopy the whole row height:
			//--------------------------------------------------------------------
			CBCGPBaseRibbonElement* pElem = m_arElements [m_arElements.GetSize () - 1];
			ASSERT_VALID (pElem);

			CRect rectElem = pElem->GetRect ();
			rectElem.bottom = rectElem.top + nRowHeight;

			pElem->SetRect (rectElem);
		}
	}
	else
	{
		const int nElementsInColumn = m_nMaxRows;

		while (TRUE)
		{
			int nColumnWidth = 0;
			int i = 0;
			x = 0;
			y = 0;

			CMap<int,int,int,int>	mapColumnElements;

			for (i = 0; i < m_arElements.GetSize (); i++)
			{
				CBCGPBaseRibbonElement* pElem = m_arElements [i];
				ASSERT_VALID (pElem);

				CSize sizeElem = pElem->GetSize (pDC);

				if (sizeElem == CSize (0, 0))
				{
					pElem->SetRect (CRect (0, 0, 0, 0));
					continue;
				}

				if (pElem->IsSeparator ())
				{
					x += nColumnWidth;

					CRect rectSeparator (
						CPoint (rect.left + x + m_nXMargin, rect.top + m_nYMargin), 
						CSize (sizeElem.cx, size.cy));

					pElem->SetRect (rectSeparator);

					x += sizeElem.cx + m_nXMargin;
					y = 0;
					nColumnWidth = 0;
					continue;
				}

				if (pElem->IsWholeRowHeight ())
				{
					sizeElem.cy = size.cy;
				}

				if (y + sizeElem.cy > size.cy)
				{
					//---------------------------------
					// We should start next column now:
					//---------------------------------
					if (y == 0)
					{
						ShowDefaultButton (pDC);
						return;
					}

					x += nColumnWidth;
					y = 0;

					nColumnWidth = 0;
				}

				const int xColumn = rect.left + x + m_nXMargin;

				CRect rectElem (CPoint (xColumn, rect.top + y + m_nYMargin), 
								sizeElem);
				pElem->SetRect (rectElem);

				int nCount = 1;
				
				if (mapColumnElements.Lookup (xColumn, nCount))
				{
					nCount++;
				}

				mapColumnElements.SetAt (xColumn, nCount);

				nColumnWidth = max (nColumnWidth, sizeElem.cx);
				y += sizeElem.cy;
			}

			const int nFullWidth = x + nColumnWidth;

			if (nFullWidth <= size.cx)
			{
				m_nFullWidth = nFullWidth;
				break;
			}

			if (nColumnWidth == 0)
			{
				ShowDefaultButton (pDC);
				return;
			}

			BOOL bChanged = FALSE;

			//-----------------------------------------------------
			// Find element that can can be stretched horizontally:
			//-----------------------------------------------------
			for (i = 0; i < m_arElements.GetSize () && !bChanged; i++)
			{
				CBCGPBaseRibbonElement* pElem = m_arElements [i];
				ASSERT_VALID (pElem);

				if (!pElem->GetRect ().IsRectEmpty () &&
					pElem->CanBeStretchedHorizontally ())
				{
					pElem->StretcheHorizontally ();
					bChanged = TRUE;
				}
			}

			if (bChanged)
			{
				continue;
			}

			//------------------------------------------------------------------------------
			// Find 'nElementsInColumn' large buttons and make them intermediate or compact:
			//------------------------------------------------------------------------------
			int nLargeCount = 0;
			int nLargeTotal = min (nElementsInColumn, (int) m_arElements.GetSize ());

			for (i = (int) m_arElements.GetSize () - 1; !bChanged && i >= 0; i--)
			{
				CBCGPBaseRibbonElement* pElem = m_arElements [i];
				ASSERT_VALID (pElem);

				if (pElem->GetRect ().IsRectEmpty ())
				{
					continue;
				}

				if (!pElem->IsLargeMode () || !pElem->CanBeCompacted ())
				{
					nLargeCount = 0;
					continue;
				}

				nLargeCount++;

				if (nLargeCount == nLargeTotal)
				{
					bChanged = TRUE;

					for (int j = 0; j < nLargeCount; j++)
					{
						pElem = m_arElements [i + j];
						ASSERT_VALID (pElem);

						if (pElem->GetRect ().IsRectEmpty ())
						{
							j++;
						}
						else
						{
							pElem->SetCompactMode ();
						}
					}
				}
			}

			if (bChanged)
			{
				continue;
			}

			//-----------------------------------------------------------------
			// Find 'nElementsInColumn' intermediate buttons in one column and 
			// make them compact:
			//-----------------------------------------------------------------
			int nIntermediateCount = 0;
			int xColumn = -1;

			for (i = (int) m_arElements.GetSize () - 1; !bChanged && i >= 0; i--)
			{
				CBCGPBaseRibbonElement* pElem = m_arElements [i];
				ASSERT_VALID (pElem);

				if (pElem->GetRect ().IsRectEmpty ())
				{
					continue;
				}

				if (xColumn != -1 && pElem->GetRect ().left != xColumn)
				{
					nIntermediateCount = 0;
					xColumn = -1;
					continue;
				}

				xColumn = pElem->GetRect ().left;

				if (!pElem->IsIntermediateMode () || !pElem->HasCompactMode ())
				{
					nIntermediateCount = 0;
					xColumn = -1;
					continue;
				}

				nIntermediateCount++;

				if (nIntermediateCount == nElementsInColumn)
				{
					bChanged = TRUE;

					for (int j = 0; j < nIntermediateCount; j++)
					{
						pElem = m_arElements [i + j];
						ASSERT_VALID (pElem);

						if (pElem->GetRect ().IsRectEmpty ())
						{
							j++;
						}
						else
						{
							pElem->SetCompactMode ();
						}
					}
				}
			}

			if (bChanged)
			{
				continue;
			}

			const int nStart = m_arElements.GetSize () < 3 ? 0 : 1;

			//--------------------------------------------------------------------
			// Find 1 large button near intermediate and make it intermediate too:
			//--------------------------------------------------------------------
			for (i = nStart; i < m_arElements.GetSize () && !bChanged; i++)
			{
				CBCGPBaseRibbonElement* pElem = m_arElements [i];
				ASSERT_VALID (pElem);

				if (pElem->GetRect ().IsRectEmpty ())
				{
					continue;
				}

				if (!pElem->IsLargeMode () || !pElem->CanBeCompacted ())
				{
					continue;
				}

				if (i < m_arElements.GetSize () - 1 && m_arElements [i + 1]->m_bIntermediateMode)
				{
					int nColumnElements = 0;

					if (mapColumnElements.Lookup (m_arElements [i + 1]->GetRect ().left, nColumnElements) &&
						nColumnElements < nElementsInColumn)
					{
						pElem->m_bIntermediateMode = TRUE;
						pElem->m_bCompactMode = FALSE;

						bChanged = TRUE;
					}
					break;
				}
			}

			if (bChanged)
			{
				continue;
			}

			//----------------------------------------------
			// Last step - try to compact rest of elements:
			//----------------------------------------------
			for (i = nStart; i < m_arElements.GetSize (); i++)
			{
				CBCGPBaseRibbonElement* pElem = m_arElements [i];
				ASSERT_VALID (pElem);

				if (!pElem->GetRect ().IsRectEmpty () &&
					pElem->m_bIntermediateMode &&
					pElem->HasCompactMode ())
				{
					pElem->m_bIntermediateMode = FALSE;
					pElem->m_bCompactMode = TRUE;
					bChanged = TRUE;
				}
			}

			if (bChanged)
			{
				continue;
			}

			ShowDefaultButton (pDC);
			return;
		}
	}

	if (m_bFloatyMode)
	{
		return;
	}

	if (m_bOnDialogBar && m_arElements.GetSize () == 1)
	{
		CBCGPBaseRibbonElement* pElem = m_arElements [0];
		ASSERT_VALID (pElem);

		if (pElem->CanBeStretchedOnDialogBar ())
		{
			CRect rectElem = pElem->m_rect;
			
			if (!rectElem.IsRectEmpty ())
			{
				rectElem.right = rectElem.left + size.cx;
				pElem->SetRect (rectElem);
			}
		}
	}

	if (m_bAlignByColumn && (m_bCenterColumnVert || m_bJustifyColumns))
	{
		int nFirstInColumnIndex = -1;
		int nLastInColumnIndex = -1;
		int x = -1;

		for (int i = 0; i < m_arElements.GetSize (); i++)
		{
			CBCGPBaseRibbonElement* pElem = m_arElements [i];
			ASSERT_VALID (pElem);

			CRect rectElem = pElem->m_rect;
			if (rectElem.IsRectEmpty ())
			{
				continue;
			}

			if (nFirstInColumnIndex == -1)
			{
				nLastInColumnIndex = nFirstInColumnIndex = i;
				x = rectElem.left;
			}

			if (x != rectElem.left)
			{
				if (m_bCenterColumnVert)
				{
					CenterElementsInColumn (nFirstInColumnIndex, nLastInColumnIndex, sizeCaption.cy);
				}

				if (m_bJustifyColumns)
				{
					JustifyElementsInColumn (nFirstInColumnIndex, nLastInColumnIndex);
				}

				nLastInColumnIndex = nFirstInColumnIndex = i;
				x = rectElem.left;
			}
			else
			{
				nLastInColumnIndex = i;
			}
		}

		if (m_bCenterColumnVert)
		{
			CenterElementsInColumn (nFirstInColumnIndex, nLastInColumnIndex, sizeCaption.cy);
		}

		if (m_bJustifyColumns)
		{
			JustifyElementsInColumn (nFirstInColumnIndex, nLastInColumnIndex);
		}
	}

	int nTotalWidth = !m_bAlignByColumn || m_bFloatyMode ? m_nFullWidth - 1 : CalcTotalWidth ();

	if (nTotalWidth < sizeCaption.cx && !m_bTrancateCaption)
	{
		//--------------------------------------------
		// Panel is too narrow: center it horizontaly:
		//--------------------------------------------
		const int xOffset = (sizeCaption.cx - nTotalWidth) / 2;

		for (int i = 0; i < m_arElements.GetSize (); i++)
		{
			CBCGPBaseRibbonElement* pElem = m_arElements [i];
			ASSERT_VALID (pElem);

			CRect rectElem = pElem->m_rect;
			rectElem.OffsetRect (xOffset, 0);

			pElem->SetRect (rectElem);
		}

		nTotalWidth = sizeCaption.cx;

		m_nFullWidth = max(m_nFullWidth, nTotalWidth);
	}

	if (m_arElements.GetSize () == 0)
	{
		m_nFullWidth = max(sizeCaption.cx, cxDefaultButton) + m_nXMargin;
	}

	if (nTotalWidth < cxDefaultButton)
	{
		m_rect.right = m_rect.left + cxDefaultButton + m_nXMargin;
	}
	else
	{
		m_rect.right = m_rect.left + nTotalWidth + 2 * m_nXMargin;
	}

	//-----------------------------
	// Set launch button rectangle:
	//-----------------------------
	if (m_btnLaunch.GetID () > 0 && CBCGPToolBar::IsCommandPermitted (m_btnLaunch.GetID ()))
	{
		CRect rectLaunch = m_rect;

		rectLaunch.DeflateRect (1, 1);

		rectLaunch.top = rectLaunch.bottom - sizeCaption.cy + 1;
		rectLaunch.left = rectLaunch.right - sizeCaption.cy;
		rectLaunch.bottom--;
		rectLaunch.right--;

		m_btnLaunch.SetRect (rectLaunch);
	}

	//-----------------------
	// Set caption rectangle:
	//-----------------------
	if (m_bShowCaption)
	{
		m_rectCaption = m_rect;
		m_rectCaption.top = m_rectCaption.bottom - sizeCaption.cy - 1;
	}
}
//********************************************************************************
void CBCGPRibbonPanel::ReposMenu (CDC* pDC, const CRect& rect)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);

	m_bScrollDnAvailable = FALSE;

	m_nXMargin = 0;
	m_nYMargin = 0;

	CSize size = rect.Size ();

	int y = 0;
	int i = 0;

	int nImageWidth = 0;

	if (m_pParent != NULL)
	{
		ASSERT_VALID (m_pParent);
		nImageWidth = m_pParent->GetImageSize (TRUE).cx;
	}

	int nColumnWidth = 0;
	int yOffset = 0;

	for (i = 0; i < m_arElements.GetSize (); i++)
	{
		CBCGPBaseRibbonElement* pElem = m_arElements [i];
		ASSERT_VALID (pElem);

		if (i == m_nScrollOffset)
		{
			yOffset = y;
		}

		pElem->OnCalcTextSize (pDC);
		pElem->SetCompactMode (FALSE);
		pElem->SetTextAlwaysOnRight ();

		CSize sizeElem = pElem->GetSize (pDC);

		if (sizeElem == CSize (0, 0))
		{
			pElem->SetRect (CRect (0, 0, 0, 0));
			continue;
		}

		if (!rect.IsRectEmpty ())
		{
			sizeElem.cx = rect.Width ();

			if (m_bIsDefaultMenuLook)
			{
				pElem->m_nImageOffset = CBCGPToolBar::GetMenuImageSize ().cx;
			}
		}

		CRect rectElem = CRect
			(CPoint (rect.left + m_nXMargin, rect.top + y + m_nYMargin), 
			sizeElem);

		pElem->SetRect (rectElem);

		nColumnWidth = max (nColumnWidth, sizeElem.cx);
		y += sizeElem.cy;
	}

	for (i = 0; i < m_arElements.GetSize (); i++)
	{
		CBCGPBaseRibbonElement* pElem = m_arElements [i];
		ASSERT_VALID (pElem);

		CRect rectElem = pElem->GetRect ();

		if (yOffset > 0 && !rectElem.IsRectEmpty ())
		{
			rectElem.OffsetRect (0, -yOffset);
			pElem->SetRect (rectElem);
		}

		if (rectElem.bottom > rect.bottom)
		{
			m_bScrollDnAvailable = TRUE;
		}
	}

	if (m_bIsDefaultMenuLook)
	{
		nColumnWidth += CBCGPToolBar::GetMenuImageSize ().cx + 
			2 * CBCGPVisualManager::GetInstance ()->GetMenuImageMargin ();
	}

	m_nFullWidth = nColumnWidth;

	if (rect.IsRectEmpty ())
	{
		//----------------------------------------------
		// All menu elements should have the same width:
		//----------------------------------------------
		for (i = 0; i < m_arElements.GetSize (); i++)
		{
			CBCGPBaseRibbonElement* pElem = m_arElements [i];
			ASSERT_VALID (pElem);

			CRect rectElem = pElem->GetRect ();

			if (!rectElem.IsRectEmpty ())
			{
				rectElem.right = rectElem.left + nColumnWidth;
				
				if (nImageWidth > 0 && 
					pElem->IsKindOf (RUNTIME_CLASS (CBCGPRibbonSeparator)))
				{
					rectElem.left += nImageWidth + m_nXMargin;
				}

				pElem->SetRect (rectElem);
			}

			pElem->OnAfterChangeRect (pDC);
		}
	}

	m_rect = rect;
	m_rect.bottom = m_rect.top + y;
	m_rect.right = m_rect.left + m_nFullWidth;
}
//********************************************************************************
void CBCGPRibbonPanel::ReposPalette (CDC* pDC, const CRect& rect)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);
	ASSERT_VALID (m_pScrollBar);
	ASSERT_VALID (m_pPaletteButton);

	const int yOffset = 4;

	if (rect == CRect (0, 0, 0, 0))
	{
		return;
	}

	const BOOL bNoSideBar = m_pPaletteButton->IsKindOf (RUNTIME_CLASS (CBCGPRibbonUndoButton));

	BOOL bShowAllItems = FALSE;

	m_nScrollOffset = 0;

	CRect rectInitial = rect;

	if (rectInitial.bottom <= 0)
	{
		rectInitial.bottom = rectInitial.top + 32676;
		bShowAllItems = TRUE;
	}

	m_nXMargin = 0;
	m_nYMargin = 0;

	const int cxScroll = bShowAllItems && !m_pPaletteButton->IsMenuResizeEnabled () ? 
		0 : ::GetSystemMetrics (SM_CXVSCROLL);

	int nScrollTotal = 0;

	int x = rectInitial.left;
	int y = rectInitial.top;

	m_rect = rectInitial;

	if (m_bSizeIsLocked)
	{
		rectInitial.right -= cxScroll;
	}
	else
	{
		m_rect.right += cxScroll;
	}

	int i = 0;
	BOOL bHasBottomItems = FALSE;
	BOOL bHasTopItems = FALSE;

	int nMaxItemHeight = 0;
	int nMaxImageWidth = 0;
		
	if (m_bSizeIsLocked)
	{
		int i = 0;

		// Calculate the total "bottom" elements height:
		int nBottomHeight = 0;

		for (i = 0; i < m_arElements.GetSize (); i++)
		{
			CBCGPBaseRibbonElement* pElem = m_arElements [i];
			ASSERT_VALID (pElem);

			BOOL bIsLabel = pElem->IsKindOf (RUNTIME_CLASS (CBCGPRibbonLabel));
			BOOL bIsIcon = pElem->IsKindOf (RUNTIME_CLASS (CBCGPRibbonPaletteIcon));

			if (bIsIcon || bIsLabel || pElem->m_bIsOnPaletteTop)
			{
				continue;
			}

			pElem->OnCalcTextSize (pDC);
			pElem->SetCompactMode (TRUE);

			if (pElem->GetImageSize (CBCGPBaseRibbonElement::RibbonImageLarge) == CSize (0, 0))
			{
				pElem->SetCompactMode (FALSE);
			}

			pElem->SetTextAlwaysOnRight ();

			nBottomHeight += pElem->GetSize (pDC).cy;
		}

		// Find all menu items and place them on top/bottom:
		int yTop = rectInitial.top;
		int yBottom = rectInitial.bottom - nBottomHeight;

		rectInitial.bottom = yBottom - yOffset;

		m_rectMenuAreaTop = m_rect;
		m_rectMenuAreaBottom = m_rect;
		m_rectMenuAreaBottom.top = yBottom;

		for (i = 0; i < m_arElements.GetSize (); i++)
		{
			CBCGPBaseRibbonElement* pElem = m_arElements [i];
			ASSERT_VALID (pElem);

			BOOL bIsLabel = pElem->IsKindOf (RUNTIME_CLASS (CBCGPRibbonLabel));
			BOOL bIsIcon = pElem->IsKindOf (RUNTIME_CLASS (CBCGPRibbonPaletteIcon));

			if (bIsIcon || bIsLabel)
			{
				continue;
			}

			if (pElem->m_bIsOnPaletteTop)
			{
				bHasTopItems = TRUE;
			}
			else
			{
				bHasBottomItems = TRUE;
			}

			pElem->OnCalcTextSize (pDC);
			pElem->SetCompactMode (TRUE);

			if (pElem->GetImageSize (CBCGPBaseRibbonElement::RibbonImageLarge) == CSize (0, 0))
			{
				pElem->SetCompactMode (FALSE);
			}

			pElem->SetTextAlwaysOnRight ();

			CSize sizeElem = pElem->GetSize (pDC);
			sizeElem.cx = m_rect.Width ();

			CRect rectElem (0, 0, 0, 0);

			if (pElem->m_bIsOnPaletteTop)
			{
				rectElem = CRect (CPoint (rectInitial.left, yTop), sizeElem);

				yTop += sizeElem.cy;
				rectInitial.top = yTop + yOffset;
				m_rectMenuAreaTop.bottom = yTop;

				y += sizeElem.cy;
			}
			else
			{
				rectElem = CRect (CPoint (rectInitial.left, yBottom), sizeElem);
				yBottom += sizeElem.cy;
			}

			pElem->SetRect (rectElem);
		}
	}
	else
	{
		// Repos all top items:
		m_rectMenuAreaTop = m_rect;

		for (i = 0; i < m_arElements.GetSize (); i++)
		{
			CBCGPBaseRibbonElement* pElem = m_arElements [i];
			ASSERT_VALID (pElem);

			if (pElem->IsKindOf (RUNTIME_CLASS (CBCGPRibbonPaletteIcon)) ||
				pElem->IsKindOf (RUNTIME_CLASS (CBCGPRibbonLabel)))
			{
				continue;
			}

			pElem->OnCalcTextSize (pDC);
			pElem->SetCompactMode (TRUE);

			if (pElem->GetImageSize (CBCGPBaseRibbonElement::RibbonImageLarge) == CSize (0, 0))
			{
				pElem->SetCompactMode (FALSE);
			}

			pElem->SetTextAlwaysOnRight ();

			CSize sizeElem = pElem->GetSize (pDC);
			sizeElem.cx += 2 * TEXT_MARGIN;

			nMaxItemHeight = max (nMaxItemHeight, sizeElem.cy);
			nMaxImageWidth = max (nMaxImageWidth, 
				pElem->GetImageSize (CBCGPBaseRibbonElement::RibbonImageSmall).cx);
		}

		if (nMaxImageWidth == 0 && !bNoSideBar)
		{
			nMaxImageWidth = CBCGPToolBar::GetMenuImageSize ().cx + 
				2 * CBCGPVisualManager::GetInstance ()->GetMenuImageMargin ();

			if (m_pParent != NULL)
			{
				nMaxImageWidth = m_pParent->GetImageSize (FALSE).cx;
			}
		}

		for (i = 0; i < m_arElements.GetSize (); i++)
		{
			CBCGPBaseRibbonElement* pElem = m_arElements [i];
			ASSERT_VALID (pElem);

			if (pElem->IsKindOf (RUNTIME_CLASS (CBCGPRibbonPaletteIcon)) ||
				pElem->IsKindOf (RUNTIME_CLASS (CBCGPRibbonLabel)) ||
				!pElem->m_bIsOnPaletteTop)
			{
				continue;
			}

			CSize sizeElem = pElem->GetSize (pDC);

			if (sizeElem == CSize (0, 0))
			{
				pElem->SetRect (CRect (0, 0, 0, 0));
				continue;
			}

			pElem->m_nImageOffset = nMaxImageWidth;

			sizeElem.cx = m_rect.Width ();
			sizeElem.cy = nMaxItemHeight;

			CRect rectElem = CRect
				(CPoint (rectInitial.left, rectInitial.top + y), 
				sizeElem);

			pElem->SetRect (rectElem);

			y += sizeElem.cy;
		}

		m_rectMenuAreaTop.bottom = y;
	}

	// Set palette icons location:
	int yNextLine = m_rect.bottom;
	BOOL bIsFirstPaletteElem = TRUE;

	if (!m_bSizeIsLocked)
	{
		m_rect.bottom = y;
	}

	for (i = 0; i < m_arElements.GetSize (); i++)
	{
		CBCGPBaseRibbonElement* pElem = m_arElements [i];
		ASSERT_VALID (pElem);

		BOOL bIsLabel = pElem->IsKindOf (RUNTIME_CLASS (CBCGPRibbonLabel));
		BOOL bIsIcon = pElem->IsKindOf (RUNTIME_CLASS (CBCGPRibbonPaletteIcon));

		if (!bIsIcon && !bIsLabel)
		{
			if (pElem->m_bIsOnPaletteTop)
			{
				bHasTopItems = TRUE;
			}
			else
			{
				bHasBottomItems = TRUE;
			}
		}

		CSize sizeElem (0, 0);

		if (bIsLabel)
		{
			if (x > rectInitial.left)
			{
				y = yNextLine;
			}

			if (i > 0)
			{
				y++;
			}

			CString strLabel = pElem->GetText ();
			CRect rectElem (0, 0, 0, 0);

			if (strLabel.IsEmpty ())
			{
				if (!bIsFirstPaletteElem)
				{
					y += m_pPaletteButton->GetGroupOffset ();
				}
			}
			else
			{
				pElem->OnCalcTextSize (pDC);

				sizeElem = pElem->GetSize (pDC);
				sizeElem.cx = rectInitial.Width ();

				rectElem = CRect
					(CPoint (rectInitial.left, y), sizeElem);

				y += sizeElem.cy + m_pPaletteButton->GetGroupOffset ();
				
				bIsFirstPaletteElem = FALSE;
			}

			pElem->SetRect (rectElem);

			if (!m_bSizeIsLocked)
			{
				m_rect.bottom = rectElem.bottom;
			}

			nScrollTotal = yNextLine = rectElem.bottom;

			x = rectInitial.left;
		}
		else if (bIsIcon)
		{
			bIsFirstPaletteElem = FALSE;

			pElem->SetCompactMode (FALSE);

			sizeElem = pElem->GetSize (pDC);

			if (m_pPaletteButton->m_bIsComboMode)
			{
				sizeElem.cx = m_rect.Width ();
			}

			if (x + sizeElem.cx > rectInitial.right && x != rectInitial.left)
			{
				x = rectInitial.left;
				y += sizeElem.cy;
			}

			CRect rectElem = CRect (CPoint (x, y), sizeElem);

			pElem->SetRect (rectElem);

			if (!m_bSizeIsLocked)
			{
				m_rect.bottom = rectElem.bottom;
			}

			nScrollTotal = yNextLine = rectElem.bottom;

			x += sizeElem.cx;
		}
	}

	if (!m_bSizeIsLocked)
	{
		m_rect.bottom = min (m_rect.bottom, rectInitial.bottom);
	}

	m_nFullWidth = m_rect.Width ();

	if (bHasBottomItems && !m_bSizeIsLocked)
	{
		// Set menu items location (on bottom):
		y = m_rect.bottom + yOffset;
		m_rectMenuAreaBottom = m_rect;
		m_rectMenuAreaBottom.top = y;

		for (i = 0; i < m_arElements.GetSize (); i++)
		{
			CBCGPBaseRibbonElement* pElem = m_arElements [i];
			ASSERT_VALID (pElem);

			if (pElem->IsKindOf (RUNTIME_CLASS (CBCGPRibbonPaletteIcon)) ||
				pElem->IsKindOf (RUNTIME_CLASS (CBCGPRibbonLabel)) ||
				pElem->m_bIsOnPaletteTop)
			{
				continue;
			}

			CSize sizeElem = pElem->GetSize (pDC);

			if (sizeElem == CSize (0, 0))
			{
				pElem->SetRect (CRect (0, 0, 0, 0));
				continue;
			}

			pElem->m_nImageOffset = nMaxImageWidth;

			sizeElem.cx = m_rect.Width ();
			sizeElem.cy = nMaxItemHeight;

			CRect rectElem = CRect
				(CPoint (rectInitial.left, rectInitial.top + y), 
				sizeElem);

			m_rect.bottom = rectElem.bottom;

			pElem->SetRect (rectElem);

			y += sizeElem.cy;
		}

		m_rectMenuAreaBottom.bottom = y;
	}

	if (!bHasBottomItems)
	{
		m_rectMenuAreaBottom.SetRectEmpty ();
	}

	if (!bHasTopItems)
	{
		m_rectMenuAreaTop.SetRectEmpty ();
	}

	// Define icon postions in matrix:
	for (i = 0; i < m_arElements.GetSize (); i++)
	{
		CBCGPRibbonPaletteIcon* pIcon = DYNAMIC_DOWNCAST (CBCGPRibbonPaletteIcon, m_arElements [i]);
		if (pIcon == NULL)
		{
			continue;
		}

		ASSERT_VALID (pIcon);

		pIcon->m_bIsFirstInRow = FALSE;
		pIcon->m_bIsLastInRow = FALSE;
		pIcon->m_bIsFirstInColumn = FALSE;
		pIcon->m_bIsLastInColumn = FALSE;

		CRect rectIcon = pIcon->GetRect ();

		if (rectIcon.IsRectEmpty ())
		{
			continue;
		}

		pIcon->m_bIsFirstInRow = DYNAMIC_DOWNCAST (
			CBCGPRibbonPaletteIcon, HitTest (CPoint (rectIcon.left - 2, rectIcon.CenterPoint ().y))) == NULL;

		pIcon->m_bIsLastInRow = DYNAMIC_DOWNCAST (
			CBCGPRibbonPaletteIcon, HitTest (CPoint (rectIcon.right + 2, rectIcon.CenterPoint ().y))) == NULL;

		pIcon->m_bIsFirstInColumn = DYNAMIC_DOWNCAST (
			CBCGPRibbonPaletteIcon, HitTest (CPoint (rectIcon.CenterPoint ().x, rectIcon.top - 2))) == NULL;

		pIcon->m_bIsLastInColumn = DYNAMIC_DOWNCAST (
			CBCGPRibbonPaletteIcon, HitTest (CPoint (rectIcon.CenterPoint ().x, rectIcon.bottom + 2))) == NULL;
	}

	if (!bShowAllItems || m_pPaletteButton->IsMenuResizeEnabled ())
	{
		const int ySB = bHasTopItems ? m_rectMenuAreaTop.bottom + 1 : rectInitial.top;
		const int ySBBottom = bHasBottomItems ? m_rectMenuAreaBottom.top - 1 : m_rect.bottom;

		m_pScrollBar->SetWindowPos (NULL, 
			m_rect.right - cxScroll, ySB,
			cxScroll, ySBBottom - ySB - 1,
			SWP_NOZORDER | SWP_NOACTIVATE);

		SCROLLINFO si;

		ZeroMemory (&si, sizeof (SCROLLINFO));
		si.cbSize = sizeof (SCROLLINFO);
		si.fMask = SIF_RANGE | SIF_POS | SIF_PAGE;

		si.nMin = 0;

		nScrollTotal -= m_rectMenuAreaTop.Height ();

		if (nScrollTotal > rectInitial.Height ())
		{
			si.nMax = nScrollTotal;
			si.nPage = rectInitial.Height ();

			m_pScrollBar->SetScrollInfo (&si, TRUE);
			m_pScrollBar->EnableScrollBar (ESB_ENABLE_BOTH);
			m_pScrollBar->EnableWindow ();
		}
		else if (!bShowAllItems)
		{
			m_pScrollBar->EnableScrollBar (ESB_DISABLE_BOTH);
		}
	}
}
//********************************************************************************
void CBCGPRibbonPanel::ShowDefaultButton (CDC* pDC)
{
	//-------------------------------
	// Show panel default button only
	//-------------------------------
	const int cxDefaultButton = m_btnDefault.GetRegularSize (pDC).cx;

	for (int i = 0; i < m_arElements.GetSize (); i++)
	{
		CBCGPBaseRibbonElement* pElem = m_arElements [i];
		ASSERT_VALID (pElem);

		pElem->SetRect (CRect (0, 0, 0, 0));
	}

	m_rect.right = m_rect.left + cxDefaultButton;

	m_btnDefault.SetRect (m_rect);
	m_nRows = 0;
	m_bShowCaption = FALSE;
	m_bForceCollpapse = FALSE;
}
//********************************************************************************
int CBCGPRibbonPanel::CalcTotalWidth ()
{
	//------------------------------------------------------------
	// Total width will be right edge of the last visible element
	// in the right column
	//------------------------------------------------------------
	int xRight = 0;

	for (int i = (int) m_arElements.GetSize () - 1; i >= 0; i--)
	{
		CBCGPBaseRibbonElement* pElem = m_arElements [i];
		ASSERT_VALID (pElem);

		if (pElem->m_rect.IsRectEmpty ())
		{
			continue;
		}

		xRight = max (xRight, pElem->m_rect.right);
	}

	return max (0, xRight - m_rect.left - m_nXMargin / 2 - 1);
}
//********************************************************************************
CSize CBCGPRibbonPanel::GetCaptionSize (CDC* pDC) const
{
	ASSERT_VALID (pDC);

	if (m_bFloatyMode)
	{
		return CSize (0, 0);
	}

	CString strName = GetDisplayName();

	CSize size = pDC->GetTextExtent (strName.IsEmpty () ? _T(" ") : strName);

	size.cy += m_nYMargin + 1;

	if (m_btnLaunch.GetID () > 0)
	{
		size.cx += size.cy + 1;
	}

	return size;
}
//********************************************************************************
void CBCGPRibbonPanel::CenterElementsInColumn (int nFirstInColumnIndex, int nLastInColumnIndex,
											   int nCaptionHeight)
{
	if (nFirstInColumnIndex > nLastInColumnIndex ||
		nFirstInColumnIndex < 0 ||
		nLastInColumnIndex < 0)
	{
		return;
	}

	//------------------------------------------
	// Center all elements in column vertically:
	//------------------------------------------
	CBCGPBaseRibbonElement* pLastElem = m_arElements [nLastInColumnIndex];
	ASSERT_VALID (pLastElem);

	const int nColumnHeight = m_rect.Height () - nCaptionHeight - 2 * m_nYMargin;
	const int nTotalHeight = pLastElem->m_rect.bottom - m_rect.top - m_nYMargin;
	const int nOffset = max (0, (nColumnHeight - nTotalHeight) / 2);

	for (int i = nFirstInColumnIndex; i <= nLastInColumnIndex; i++)
	{
		CBCGPBaseRibbonElement* pColumnElem = m_arElements [i];
		ASSERT_VALID (pColumnElem);

		CRect rectElem = pColumnElem->m_rect;
		rectElem.OffsetRect (0, nOffset);
		pColumnElem->SetRect (rectElem);
	}
}
//********************************************************************************
void CBCGPRibbonPanel::JustifyElementsInColumn (int nFirstInColumnIndex, int nLastInColumnIndex)
{
	if (nFirstInColumnIndex > nLastInColumnIndex ||
		nFirstInColumnIndex < 0 ||
		nLastInColumnIndex < 0)
	{
		return;
	}

	//-------------------------------------------------
	// Set same width (largets) to all column elements:
	//-------------------------------------------------
	int nColumnWidth = 0;
	int i = 0;

	for (i = nFirstInColumnIndex; i <= nLastInColumnIndex; i++)
	{
		CBCGPBaseRibbonElement* pColumnElem = m_arElements [i];
		ASSERT_VALID (pColumnElem);

		nColumnWidth = max (nColumnWidth, pColumnElem->m_rect.Width ());
	}

	for (i = nFirstInColumnIndex; i <= nLastInColumnIndex; i++)
	{
		CBCGPBaseRibbonElement* pColumnElem = m_arElements [i];
		ASSERT_VALID (pColumnElem);

		CRect rectElem = pColumnElem->m_rect;
		rectElem.right = rectElem.left + nColumnWidth;

		pColumnElem->SetRect (rectElem);
	}
}
//********************************************************************************
void CBCGPRibbonPanel::RecalcWidths (CDC* pDC, int nHeight)
{
	ASSERT_VALID (pDC);

	CRect rectScreen;
	::SystemParametersInfo (SPI_GETWORKAREA, 0, &rectScreen, 0);

	m_btnDefault.OnCalcTextSize (pDC);

	const int cxDefaultButton = m_btnDefault.GetRegularSize (pDC).cx;

	m_arWidths.RemoveAll ();
	m_mapNonOptWidths.RemoveAll ();

	m_nCurrWidthIndex = 0;
	m_bIsCalcWidth = TRUE;

	int nLastWidth = -1;
	const int dx = 16;

	//--------------------------
	// Check for the user tools:
	//--------------------------
	if (g_pUserToolsManager != NULL)
	{
		int iToolItemIndex = -1;
		
		for (int i = 0; i < m_arElements.GetSize(); i++)
		{
			CBCGPBaseRibbonElement* pButton = m_arElements[i];
			ASSERT_VALID(pButton);
			
			if (g_pUserToolsManager->GetToolsEntryCmd () == pButton->GetID())
			{
				iToolItemIndex = i + 1;
			}
			else if (g_pUserToolsManager->IsUserToolCmd(pButton->GetID()))
			{
				m_arElements.RemoveAt(i);
				i--;
				delete pButton;
			}
		}

		if (iToolItemIndex >= 0)
		{
			const CObList& lstTools = g_pUserToolsManager->GetUserTools();

			for (POSITION posTool = lstTools.GetHeadPosition (); posTool != NULL;)
			{
				CBCGPUserTool* pTool = (CBCGPUserTool*) lstTools.GetNext (posTool);
				ASSERT_VALID (pTool);

				CBCGPRibbonButton* pToolButton = new CBCGPRibbonButton(
					pTool->GetCommandId(), pTool->SetToolIcon(), pTool->m_strLabel);

				pToolButton->SetParentCategory(GetParentCategory());
				
				pToolButton->UpdateTooltipInfo();
				pToolButton->SetToolTipText(pTool->m_strLabel);

				m_arElements.InsertAt(iToolItemIndex, pToolButton);

				iToolItemIndex++;
			}
		}
	}

	if (m_bAlignByColumn && !m_bFloatyMode)
	{
		CRect rect (0, 0, 32767, nHeight);

		do
		{
			Repos (pDC, rect);

			if (!m_bShowCaption)
			{
				break;
			}

			if (nLastWidth == -1 || m_nFullWidth < nLastWidth)
			{
				nLastWidth = m_nFullWidth;

				if (nLastWidth <= cxDefaultButton ||
					(nLastWidth <= 3 * cxDefaultButton / 2 && m_arElements.GetSize () == 1))
				{
					if (m_arWidths.GetSize () == 0)
					{
						//-----------------------------------------------------------
						// Panel has onle one layout and it smaller then collapsed.
						// Use this layout only and don't allow to collapse the panel
						//-----------------------------------------------------------
						m_arWidths.Add (nLastWidth);
						m_bIsCalcWidth = FALSE;
						return;
					}

					break;
				}

				m_arWidths.Add (nLastWidth);
				rect.right = nLastWidth - dx;
			}
			else
			{
				rect.right -= dx;
			}
		}
		while (rect.Width () > 2 * m_nXMargin);
	}
	else if (m_bIsQATPopup)
	{
		CRect rect (0, 0, rectScreen.Width () - 10, nHeight);

		Repos (pDC, rect);
		m_arWidths.Add (m_nFullWidth);
	}
	else
	{
		const int nMaxRows = m_bIsQATPopup ? 50 : m_nMaxRows;

		for (int nRows = 1; nRows <= nMaxRows; nRows++)
		{
			CRect rect (0, 0, cxDefaultButton + 1, nHeight);

			for (;; rect.right += dx)
			{
				if (rect.Width () >= rectScreen.Width ())
				{
					if (m_arWidths.GetSize() == 0)
					{
						m_arWidths.Add(32767);
					}
					break;
				}

				Repos (pDC, rect);

				if (nLastWidth != -1 && m_nFullWidth > nLastWidth)
				{
					break;
				}

				if (m_nRows == nRows && m_nFullWidth > 0)
				{
					if (m_nRows == nMaxRows - 1 && !m_bFloatyMode)
					{
						//------------------------
						// Don't add 1 row layout:
						//------------------------
						m_arWidths.RemoveAll ();
					}

					m_arWidths.Add (m_nFullWidth);
					m_mapNonOptWidths.SetAt (m_nFullWidth + m_nXMargin, rect.Width ());
					nLastWidth = m_nFullWidth;
					break;
				}
			}
		}
	}

	VerifyNonCollapsibleState();

	if (!IsNonCollapsible())
	{
		m_arWidths.Add (cxDefaultButton);
	}

	m_bIsCalcWidth = FALSE;
}
//********************************************************************************
int CBCGPRibbonPanel::GetMinWidth (CDC* pDC)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);

	VerifyNonCollapsibleState();

	if (IsNonCollapsible() && m_arWidths.GetSize() > 0)
	{
		return m_arWidths[m_arWidths.GetSize() - 1];
	}

	m_btnDefault.OnCalcTextSize (pDC);
	return m_btnDefault.GetRegularSize (pDC).cx;
}
//********************************************************************************
void CBCGPRibbonPanel::Highlight (BOOL bHighlight, CPoint point)
{
	ASSERT_VALID (this);

	BOOL bRedrawPanel = m_bIsHighlighted != bHighlight;

	if (!bHighlight)
	{
		m_bMouseIsDown = FALSE;
	}

	BOOL bMouseIsDown = m_bMouseIsDown;

	m_bIsHighlighted = bHighlight;

	CBCGPBaseRibbonElement* pHighlighted = NULL;
	if (bHighlight)
	{
		pHighlighted = HitTest (point);

		if (pHighlighted != NULL)
		{
			ASSERT_VALID (pHighlighted);

			if (!bMouseIsDown || pHighlighted->IsPressed() || IsMenuMode ())
			{
				pHighlighted->OnMouseMove (point);
			}
		}
	}

	BOOL bNotifyParent = FALSE;
	BOOL bSetFocus = FALSE;

	if (pHighlighted != m_pHighlighted)
	{
		if (m_pParent != NULL && m_pParent->GetParentRibbonBar () != NULL &&
			pHighlighted != NULL)
		{
			m_pParent->GetParentRibbonBar ()->PopTooltip ();
		}

		if (m_pParentMenuBar != NULL)
		{
			ASSERT_VALID (m_pParentMenuBar);
			m_pParentMenuBar->PopTooltip ();
		}

		if (m_pHighlighted != NULL)
		{
			ASSERT_VALID (m_pHighlighted);

			m_pHighlighted->m_bIsHighlighted = FALSE;
			m_pHighlighted->OnHighlight (FALSE);

			if (IsMenuMode () && m_pHighlighted->m_bIsFocused)
			{
				bSetFocus = TRUE;
				m_pHighlighted->m_bIsFocused = FALSE;
				m_pHighlighted->OnSetFocus (FALSE);
			}

			RedrawElement (m_pHighlighted);
			m_pHighlighted = NULL;
		}

		bNotifyParent = TRUE;
	}

	if (pHighlighted != NULL)
	{
		ASSERT_VALID (pHighlighted);

		if (IsMenuMode() || !bMouseIsDown || pHighlighted->IsPressed ())
		{
			m_pHighlighted = pHighlighted;

			if (!m_pHighlighted->m_bIsHighlighted)
			{
				m_pHighlighted->OnHighlight (TRUE);
				m_pHighlighted->m_bIsHighlighted = TRUE;

				if (bSetFocus)
				{
					m_pHighlighted->m_bIsFocused = TRUE;
					m_pHighlighted->OnSetFocus (TRUE);
				}

				RedrawElement (m_pHighlighted);
			}
		}
	}

	if (bRedrawPanel && m_pParent != NULL && GetParentWnd () != NULL)
	{
		GetParentWnd ()->RedrawWindow (m_rect);
	}

	if (m_bFloatyMode && bRedrawPanel)
	{
		ASSERT_VALID (m_pParentMenuBar);
		m_pParentMenuBar->SetActive (m_bIsHighlighted);
	}

	if (bNotifyParent)
	{
		if (m_pParentMenuBar != NULL)
		{
			ASSERT_VALID (m_pParentMenuBar);
			m_pParentMenuBar->OnChangeHighlighted (m_pHighlighted);
		}
	}
}
//********************************************************************************
CBCGPBaseRibbonElement* CBCGPRibbonPanel::HitTest (CPoint point, BOOL bCheckPanelCaption)
{
	if (!m_btnDefault.m_rect.IsRectEmpty () &&
		m_btnDefault.m_rect.PtInRect (point))
	{
		return &m_btnDefault;
	}

	if (!m_btnLaunch.m_rect.IsRectEmpty () &&
		m_btnLaunch.m_rect.PtInRect (point))
	{
		return &m_btnLaunch;
	}

	for (int i = 0; i < m_arElements.GetSize (); i++)
	{
		CBCGPBaseRibbonElement* pElem = m_arElements [i];
		ASSERT_VALID (pElem);

		if (!pElem->m_rect.IsRectEmpty () &&
			pElem->m_rect.PtInRect (point))
		{
			return pElem->HitTest (point);
		}

		CBCGPBaseRibbonElement* pBackstageView = pElem->GetBackstageAttachedView();
		if (pBackstageView != NULL)
		{
			ASSERT_VALID(pBackstageView);

			if (!pBackstageView->m_rect.IsRectEmpty () &&
				pBackstageView->m_rect.PtInRect (point))
			{
				return pBackstageView->HitTest (point);
			}
		}
	}

	if (bCheckPanelCaption && m_rectCaption.PtInRect (point))
	{
		return &m_btnDefault;
	}

	return NULL;
}
int CBCGPRibbonPanel::HitTestEx (CPoint point)
{
	for (int i = 0; i < m_arElements.GetSize (); i++)
	{
		CBCGPBaseRibbonElement* pElem = m_arElements [i];
		ASSERT_VALID (pElem);

		if (!pElem->m_rect.IsRectEmpty () &&
			pElem->m_rect.PtInRect (point))
		{
			return i;
		}
	}

	return -1;
}
//********************************************************************************
int CBCGPRibbonPanel::GetIndex (CBCGPBaseRibbonElement* pElem)
{
	for (int i = 0; i < m_arElements.GetSize (); i++)
	{
		CBCGPBaseRibbonElement* pNextElem = m_arElements [i];
		ASSERT_VALID (pNextElem);

		if (pNextElem == pElem)
		{
			return i;
		}
	}

	return -1;
}
//********************************************************************************
CBCGPBaseRibbonElement* CBCGPRibbonPanel::MouseButtonDown (CPoint point)
{
	ASSERT_VALID (this);

	m_bMouseIsDown = TRUE;

	if (m_pHighlighted != NULL)
	{
		ASSERT_VALID (m_pHighlighted);

		BOOL bSetPressed = TRUE;

		if (m_pHighlighted->HasMenu () || m_pHighlighted->HasPin())
		{
			CBCGPRibbonButton* pButton = DYNAMIC_DOWNCAST (CBCGPRibbonButton, m_pHighlighted);
			if (pButton != NULL)
			{
				ASSERT_VALID (pButton);

				const CRect rectCmd = pButton->GetCommandRect ();
				bSetPressed = !rectCmd.IsRectEmpty () && rectCmd.PtInRect (point);
			}
		}

		if (bSetPressed)
		{
			m_pHighlighted->m_bIsPressed = TRUE;
			RedrawElement (m_pHighlighted);
		}

		HWND hwndMenu = m_pParentMenuBar->GetSafeHwnd ();

		m_pHighlighted->OnLButtonDown (point);

		if (hwndMenu != NULL && !::IsWindow (hwndMenu))
		{
			return NULL;
		}
	}

	return m_pHighlighted;
}
//********************************************************************************
void CBCGPRibbonPanel::MouseButtonUp (CPoint point)
{
	ASSERT_VALID (this);

	m_bMouseIsDown = FALSE;

	if (m_pHighlighted != NULL)
	{
		ASSERT_VALID (m_pHighlighted);

		HWND hwndParent = GetParentWnd ()->GetSafeHwnd ();

		CBCGPBaseRibbonElement* pHighlighted = m_pHighlighted;
		m_pHighlighted->OnLButtonUp (point);

		if (::IsWindow (hwndParent) && pHighlighted->m_bIsPressed)
		{
			pHighlighted->m_bIsPressed = FALSE;
			RedrawElement (pHighlighted);

			if (m_pHighlighted != NULL && m_pHighlighted != pHighlighted)
			{
				RedrawElement (m_pHighlighted);
			}
		}
	}
}
//********************************************************************************
void CBCGPRibbonPanel::CancelMode ()
{
	ASSERT_VALID (this);

	m_bMouseIsDown = FALSE;

	if (m_pHighlighted != NULL)
	{
		ASSERT_VALID (m_pHighlighted);

		m_pHighlighted->m_bIsHighlighted = FALSE;
		m_pHighlighted->OnHighlight (FALSE);
		m_pHighlighted->m_bIsPressed = FALSE;
		m_pHighlighted->m_bIsFocused = FALSE;
		m_pHighlighted->OnSetFocus (FALSE);

		RedrawElement (m_pHighlighted);
		m_pHighlighted = NULL;
	}

	if (m_bIsHighlighted)
	{
		m_bIsHighlighted = FALSE;

		if (GetParentWnd ()->GetSafeHwnd () != NULL)
		{
			GetParentWnd ()->RedrawWindow (m_rect);
		}
	}
}
//********************************************************************************
void CBCGPRibbonPanel::OnUpdateCmdUI (CBCGPRibbonCmdUI* pCmdUI,
									  CFrameWnd* pTarget,
									  BOOL bDisableIfNoHndler)
{
	ASSERT_VALID (this);

	for (int i = 0; i < m_arElements.GetSize (); i++)
	{
		CBCGPBaseRibbonElement* pElem = m_arElements [i];
		ASSERT_VALID (pElem);

		pElem->OnUpdateCmdUI (pCmdUI, pTarget, bDisableIfNoHndler);
	}

	m_btnLaunch.OnUpdateCmdUI (pCmdUI, pTarget, bDisableIfNoHndler);
}
//********************************************************************************
BOOL CBCGPRibbonPanel::NotifyControlCommand (
	BOOL bAccelerator, int nNotifyCode, WPARAM wParam, LPARAM lParam)
{
	ASSERT_VALID (this);

	for (int i = 0; i < m_arElements.GetSize (); i++)
	{
		CBCGPBaseRibbonElement* pElem = m_arElements [i];
		ASSERT_VALID (pElem);

		if (pElem->NotifyControlCommand (bAccelerator, nNotifyCode, wParam, lParam))
		{
			return TRUE;
		}
	}

	return FALSE;
}
//*********************************************************************************
void CBCGPRibbonPanel::OnAfterChangeRect (CDC* pDC)
{
	ASSERT_VALID (this);

	for (int i = 0; i < m_arElements.GetSize (); i++)
	{
		CBCGPBaseRibbonElement* pElem = m_arElements [i];
		ASSERT_VALID (pElem);

		pElem->OnShow (!pElem->GetRect ().IsRectEmpty ());
		pElem->OnAfterChangeRect (pDC);
	}

	m_btnDefault.OnShow (!m_btnDefault.GetRect ().IsRectEmpty ());
	m_btnDefault.OnAfterChangeRect (pDC);

	m_btnLaunch.OnAfterChangeRect (pDC);
}
//*********************************************************************************
void CBCGPRibbonPanel::OnShow (BOOL bShow)
{
	ASSERT_VALID (this);

	for (int i = 0; i < m_arElements.GetSize (); i++)
	{
		CBCGPBaseRibbonElement* pElem = m_arElements [i];
		ASSERT_VALID (pElem);

		pElem->OnShow (bShow && !pElem->GetRect ().IsRectEmpty ());
	}
}
//******************************************************************************
BOOL CBCGPRibbonPanel::IsCollapsed () const
{
	ASSERT_VALID (this);

	return !m_btnDefault.GetRect ().IsRectEmpty ();
}
//*************************************************************************************
CBCGPBaseRibbonElement* CBCGPRibbonPanel::FindByID (UINT uiCmdID, BOOL bNonCustomOnly) const
{
	ASSERT_VALID (this);

	for (int i = 0; i < m_arElements.GetSize (); i++)
	{
		CBCGPBaseRibbonElement* pElemFromList = m_arElements [i];
		ASSERT_VALID (pElemFromList);

		CBCGPBaseRibbonElement* pElem = bNonCustomOnly ? pElemFromList->FindByIDNonCustom(uiCmdID) : pElemFromList->FindByID (uiCmdID);
		if (pElem != NULL)
		{
			ASSERT_VALID (pElem);
			return pElem;
		}
	}

	CBCGPBaseRibbonElement* pElem = ((CBCGPRibbonPanel*) this)->m_btnLaunch.FindByID (uiCmdID);
	if (pElem != NULL)
	{
		ASSERT_VALID (pElem);
		return pElem;
	}

	if (m_btnDefault.GetID () == uiCmdID)
	{
		return &(((CBCGPRibbonPanel*) this)->m_btnDefault);
	}

	return NULL;
}
//*************************************************************************************
CBCGPBaseRibbonElement* CBCGPRibbonPanel::FindByData (DWORD_PTR dwData) const
{
	ASSERT_VALID (this);

	for (int i = 0; i < m_arElements.GetSize (); i++)
	{
		CBCGPBaseRibbonElement* pElemFromList = m_arElements [i];
		ASSERT_VALID (pElemFromList);

		CBCGPBaseRibbonElement* pElem = pElemFromList->FindByData (dwData);
		if (pElem != NULL)
		{
			ASSERT_VALID (pElem);
			return pElem;
		}
	}

	CBCGPBaseRibbonElement* pElem = ((CBCGPRibbonPanel*) this)->m_btnLaunch.FindByData (dwData);
	if (pElem != NULL)
	{
		ASSERT_VALID (pElem);
		return pElem;
	}

	if (m_btnDefault.GetData () == dwData)
	{
		return &(((CBCGPRibbonPanel*) this)->m_btnDefault);
	}

	return NULL;
}
//*************************************************************************************
BOOL CBCGPRibbonPanel::SetElementMenu (UINT uiCmdID, HMENU hMenu, 
	BOOL bIsDefautCommand, BOOL bRightAlign)
{
	ASSERT_VALID (this);

	CBCGPRibbonButton* pButton = DYNAMIC_DOWNCAST (
		CBCGPRibbonButton, FindByID (uiCmdID));

	if (pButton == NULL)
	{
		TRACE(_T("Cannot find element with ID: %d\n"), uiCmdID);
		return FALSE;
	}

	ASSERT_VALID (pButton);
	pButton->SetMenu (hMenu, bIsDefautCommand, bRightAlign);

	return TRUE;
}
//*************************************************************************************
BOOL CBCGPRibbonPanel::SetElementMenu (UINT uiCmdID, UINT uiMenuResID,
	BOOL bIsDefautCommand, BOOL bRightAlign)
{
	ASSERT_VALID (this);

	CBCGPRibbonButton* pButton = DYNAMIC_DOWNCAST (
		CBCGPRibbonButton, FindByID (uiCmdID));

	if (pButton == NULL)
	{
		TRACE(_T("Cannot find element with ID: %d\n"), uiCmdID);
		return FALSE;
	}

	ASSERT_VALID (pButton);
	pButton->SetMenu (uiMenuResID, bIsDefautCommand, bRightAlign);

	return TRUE;
}
//*************************************************************************************
BOOL CBCGPRibbonPanel::Replace (int nIndex, CBCGPBaseRibbonElement* pElem, BOOL bCopyContent)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pElem);

	if (nIndex < 0 || nIndex >= m_arElements.GetSize ())
	{
		ASSERT (FALSE);
		return FALSE;
	}

	CBCGPBaseRibbonElement* pOldElem = m_arElements [nIndex];
	ASSERT_VALID (pOldElem);

	if (bCopyContent)
	{
		pElem->CopyFrom (*pOldElem);
	}
	else
	{
		pElem->CopyBaseFrom(*pOldElem);
	}

	m_arElements [nIndex] = pElem;

	if (pOldElem == m_pHighlighted)
	{
		m_pHighlighted = NULL;
	}

	delete pOldElem;
	return TRUE;
}
//*************************************************************************************
BOOL CBCGPRibbonPanel::ReplaceByID (UINT uiCmdID, CBCGPBaseRibbonElement* pElem, BOOL bCopyContent)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pElem);

	if (uiCmdID == 0 || uiCmdID == (UINT)-1)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	for (int i = 0; i < m_arElements.GetSize (); i++)
	{
		CBCGPBaseRibbonElement* pElemFromList = m_arElements [i];
		ASSERT_VALID (pElemFromList);

		if (pElemFromList->GetID () == uiCmdID)
		{
			return Replace (i, pElem, bCopyContent);
		}

		if (pElemFromList->ReplaceByID (uiCmdID, pElem))
		{
			return TRUE;
		}
	}

	return FALSE;
}
//*************************************************************************************
BOOL CBCGPRibbonPanel::ReplaceSubitemByID (UINT uiCmdID, CBCGPBaseRibbonElement* pElem, BOOL bCopyContent)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pElem);

	if (uiCmdID == 0 || uiCmdID == (UINT)-1)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	for (int i = 0; i < m_arElements.GetSize (); i++)
	{
		CBCGPBaseRibbonElement* pElemFromList = m_arElements [i];
		ASSERT_VALID (pElemFromList);

		if (pElemFromList->ReplaceSubitemByID (uiCmdID, pElem, bCopyContent))
		{
			return TRUE;
		}
	}

	return FALSE;
}
//*************************************************************************************
CBCGPBaseRibbonElement* CBCGPRibbonPanel::SetElementRTC (int nIndex, 
														 CRuntimeClass* pRTC)
{
	ASSERT_VALID (this);
	ASSERT (pRTC != NULL);

	if (!pRTC->IsDerivedFrom (RUNTIME_CLASS (CBCGPBaseRibbonElement)))
	{
		ASSERT (FALSE);
		return NULL;
	}

	CBCGPBaseRibbonElement* pNewElement = DYNAMIC_DOWNCAST (
		CBCGPBaseRibbonElement, pRTC->CreateObject ());
	ASSERT_VALID (pNewElement);

	if (!Replace (nIndex, pNewElement))
	{
		delete pNewElement;
		return NULL;
	}

	return pNewElement;
}
//*************************************************************************************
CBCGPBaseRibbonElement* CBCGPRibbonPanel::SetElementRTCByID (UINT uiCmdID, 
	CRuntimeClass* pRTC)
{
	ASSERT_VALID (this);
	ASSERT (pRTC != NULL);

	if (!pRTC->IsDerivedFrom (RUNTIME_CLASS (CBCGPBaseRibbonElement)))
	{
		ASSERT (FALSE);
		return NULL;
	}

	CBCGPBaseRibbonElement* pNewElement = DYNAMIC_DOWNCAST (
		CBCGPBaseRibbonElement, pRTC->CreateObject ());
	ASSERT_VALID (pNewElement);

	if (!ReplaceByID (uiCmdID, pNewElement))
	{
		delete pNewElement;
		return NULL;
	}

	return pNewElement;
}
//*************************************************************************************
CWnd* CBCGPRibbonPanel::GetParentWnd () const
{
	ASSERT_VALID (this);

	CWnd* pParentWnd = NULL;

	if (m_pParentMenuBar != NULL)
	{
		ASSERT_VALID (m_pParentMenuBar);
		pParentWnd = m_pParentMenuBar;
	}
	else if (m_pParent != NULL)
	{
		ASSERT_VALID (m_pParent);
		pParentWnd = m_pParent->GetParentRibbonBar ();
	}

	return pParentWnd;
}
//*************************************************************************************
void CBCGPRibbonPanel::RedrawElement (CBCGPBaseRibbonElement* pElem)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pElem);

	const CRect rectElem = pElem->GetRect ();

	if (rectElem.IsRectEmpty ())
	{
		return;
	}

	CWnd* pParentWnd = GetParentWnd ();

	if (pParentWnd->GetSafeHwnd () != NULL)
	{
		ASSERT_VALID (pParentWnd);

		pParentWnd->InvalidateRect (rectElem);
		pParentWnd->UpdateWindow ();
	}
}
//*************************************************************************************
BOOL CBCGPRibbonPanel::HasElement (const CBCGPBaseRibbonElement* pElem) const
{
	ASSERT_VALID (this);
	ASSERT_VALID (pElem);

	for (int i = 0; i < m_arElements.GetSize (); i++)
	{
		CBCGPBaseRibbonElement* pElemFromList = m_arElements [i];
		ASSERT_VALID (pElemFromList);

		if (pElemFromList->Find (pElem))
		{
			return TRUE;
		}
	}

	return FALSE;
}
//*************************************************************************************
void CBCGPRibbonPanel::GetElements (
		CArray <CBCGPBaseRibbonElement*, CBCGPBaseRibbonElement*>& arElements)
{
	ASSERT_VALID (this);

	for (int i = 0; i < m_arElements.GetSize (); i++)
	{
		CBCGPBaseRibbonElement* pElem = m_arElements [i];
		ASSERT_VALID (pElem);

		pElem->GetElements (arElements);
	}

	if (m_btnLaunch.GetID () > 0)
	{
		arElements.Add (&m_btnLaunch);
	}

	if (!IsMainPanel ())
	{
		arElements.Add (&m_btnDefault);
	}
}
//*************************************************************************************
void CBCGPRibbonPanel::GetItemIDsList (CList<UINT,UINT>& lstItems) const
{
	ASSERT_VALID (this);

	for (int i = 0; i < m_arElements.GetSize (); i++)
	{
		CBCGPBaseRibbonElement* pElem = m_arElements [i];
		ASSERT_VALID (pElem);

		pElem->GetItemIDsList (lstItems);
	}

	m_btnDefault.GetItemIDsList (lstItems);
}
//*************************************************************************************
void CBCGPRibbonPanel::GetElementsByID (UINT uiCmdID,
		CArray <CBCGPBaseRibbonElement*, CBCGPBaseRibbonElement*>& arElements)
{
	ASSERT_VALID (this);

	for (int i = 0; i < m_arElements.GetSize (); i++)
	{
		CBCGPBaseRibbonElement* pElem = m_arElements [i];
		ASSERT_VALID (pElem);

		pElem->GetElementsByID (uiCmdID, arElements);
	}

	m_btnDefault.GetElementsByID (uiCmdID, arElements);
	m_btnLaunch.GetElementsByID (uiCmdID, arElements);
}
//*************************************************************************************
void CBCGPRibbonPanel::GetElementsByName (LPCTSTR lpszName, 
		CArray<CBCGPBaseRibbonElement*, CBCGPBaseRibbonElement*>& arButtons, DWORD dwFlags)
{
	ASSERT_VALID (this);

	for (int i = 0; i < m_arElements.GetSize (); i++)
	{
		CBCGPBaseRibbonElement* pElem = m_arElements [i];
		ASSERT_VALID (pElem);

		pElem->GetElementsByName (lpszName, arButtons, dwFlags);
	}

	if (!IsMainPanel() && !IsBackstageView())
	{
		m_btnDefault.GetElementsByName (lpszName, arButtons, dwFlags);
		m_btnLaunch.GetElementsByName (lpszName, arButtons, dwFlags);
	}
}
//*************************************************************************************
void CBCGPRibbonPanel::GetVisibleElements (
		CArray <CBCGPBaseRibbonElement*, CBCGPBaseRibbonElement*>& arElements)
{
	ASSERT_VALID (this);

	CBCGPBaseRibbonElement* pBackstageActiveView = NULL;

	for (int i = 0; i < m_arElements.GetSize (); i++)
	{
		CBCGPBaseRibbonElement* pElem = m_arElements [i];
		ASSERT_VALID (pElem);

		pElem->GetVisibleElements (arElements);

		CBCGPBaseRibbonElement* pBackstageView = pElem->GetBackstageAttachedView();
		if (pBackstageView != NULL && !pBackstageView->GetRect().IsRectEmpty())
		{
			pBackstageActiveView = pBackstageView;
		}
	}

	if (pBackstageActiveView != NULL)
	{
		pBackstageActiveView->GetVisibleElements(arElements);
	}

	m_btnDefault.GetVisibleElements (arElements);
	m_btnLaunch.GetVisibleElements (arElements);
}
//*************************************************************************************
CBCGPBaseRibbonElement* CBCGPRibbonPanel::GetPressed () const
{
	ASSERT_VALID (this);

	for (int i = 0; i < m_arElements.GetSize (); i++)
	{
		CBCGPBaseRibbonElement* pElem = m_arElements [i];
		ASSERT_VALID (pElem);

		CBCGPBaseRibbonElement* pPressedElem = pElem->GetPressed ();
		if (pPressedElem != NULL)
		{
			ASSERT_VALID (pPressedElem);
			return pPressedElem;
		}
	}

	return NULL;
}
//*************************************************************************************
CBCGPBaseRibbonElement* CBCGPRibbonPanel::GetDroppedDown () const
{
	ASSERT_VALID (this);

	if (!m_btnDefault.m_rect.IsRectEmpty ())
	{
		CBCGPBaseRibbonElement* pDroppedElem = 
			((CBCGPRibbonPanel*) this)->m_btnDefault.GetDroppedDown ();

		if (pDroppedElem != NULL)
		{
			ASSERT_VALID (pDroppedElem);
			return pDroppedElem;
		}
	}

	for (int i = 0; i < m_arElements.GetSize (); i++)
	{
		CBCGPBaseRibbonElement* pElem = m_arElements [i];
		ASSERT_VALID (pElem);

		CBCGPBaseRibbonElement* pDroppedElem = pElem->GetDroppedDown ();
		if (pDroppedElem != NULL)
		{
			ASSERT_VALID (pDroppedElem);
			return pDroppedElem;
		}
	}

	return NULL;
}
//*************************************************************************************
CBCGPBaseRibbonElement* CBCGPRibbonPanel::GetFocused () const
{
	ASSERT_VALID (this);

	if (!m_btnDefault.m_rect.IsRectEmpty () && m_btnDefault.IsFocused ())
	{
		return (CBCGPBaseRibbonElement*)&m_btnDefault;
	}

	if (m_btnLaunch.IsFocused ())
	{
		return (CBCGPBaseRibbonElement*)&m_btnLaunch;
	}

	for (int i = 0; i < m_arElements.GetSize (); i++)
	{
		CBCGPBaseRibbonElement* pElem = m_arElements [i];
		ASSERT_VALID (pElem);

		CBCGPBaseRibbonElement* pFocusedElem = pElem->GetFocused ();
		if (pFocusedElem != NULL)
		{
			ASSERT_VALID (pFocusedElem);
			return pFocusedElem;
		}
	}

	return NULL;
}
//*************************************************************************************
CBCGPBaseRibbonElement* CBCGPRibbonPanel::GetHighlighted () const
{
	ASSERT_VALID (this);
	return m_pHighlighted;
}
//*************************************************************************************
BOOL CBCGPRibbonPanel::OnKey (UINT nChar)
{
	ASSERT_VALID (this);

	if (m_arElements.GetSize () == NULL)
	{
		return FALSE;
	}

	if (m_pHighlighted != NULL)
	{
		ASSERT_VALID (m_pHighlighted);

		if (m_pHighlighted->OnProcessKey (nChar))
		{
			return TRUE;
		}
	}

	CBCGPBaseRibbonElement* pNewHighlighted = NULL;
	BOOL bInvokeCommand = FALSE;

	switch (nChar)
	{
	case VK_HOME:
		if (m_bMenuMode)
		{
			pNewHighlighted = GetFirstTabStop ();
		}
		break;

	case VK_END:
		if (m_bMenuMode)
		{
			pNewHighlighted = GetLastTabStop ();
		}
		break;

	case VK_RIGHT:
		if (m_bMenuMode &&
			m_pHighlighted != NULL && m_pHighlighted->HasMenu () &&
			!m_pHighlighted->IsDroppedDown ())
		{
			m_pHighlighted->OnShowPopupMenu ();

			if (m_pHighlighted->m_pPopupMenu != NULL)
			{
				ASSERT_VALID (m_pHighlighted->m_pPopupMenu);
				m_pHighlighted->m_pPopupMenu->SendMessage (WM_KEYDOWN, VK_HOME);
			}
			break;
		}

	case VK_LEFT:
		if (m_bMenuMode && nChar == VK_LEFT && m_pParentMenuBar != NULL)
		{
			ASSERT_VALID (m_pParentMenuBar);

			const BOOL bIsPaletteIcon = m_pHighlighted != NULL && m_pHighlighted->IsPaletteIcon ();

			CBCGPPopupMenu* pParentMenu = 
				DYNAMIC_DOWNCAST (CBCGPPopupMenu, m_pParentMenuBar->GetParent ());

			if (!bIsPaletteIcon && pParentMenu != NULL && pParentMenu->GetParentPopupMenu () != NULL)
			{
				CBCGPRibbonBar* pRibbonBar = m_pParentMenuBar->GetTopLevelRibbonBar ();
				if (pRibbonBar != NULL && pRibbonBar->GetKeyboardNavLevelCurrent () == this)
				{
					pRibbonBar->SetKeyboardNavigationLevel (pRibbonBar->GetKeyboardNavLevelParent ());
				}

				pParentMenu->CloseMenu ();
				return TRUE;
			}
		}

	case VK_DOWN:
	case VK_UP:
	case VK_TAB:
		if (m_pHighlighted != NULL)
		{
			if (m_pPaletteButton != NULL && (nChar == VK_DOWN || nChar == VK_UP) &&
				m_pParentMenuBar->GetSafeHwnd () != NULL &&
				m_pScrollBar->GetSafeHwnd () != NULL)
			{
				CBCGPRibbonPaletteIcon* pIcon = DYNAMIC_DOWNCAST (CBCGPRibbonPaletteIcon, m_pHighlighted);
				if (pIcon != NULL)
				{
					ASSERT_VALID (pIcon);
					ASSERT_VALID (m_pParentMenuBar);

					const CRect rectIcon = pIcon->GetRect ();
					const CRect rectPalette = GetPaletteRect ();

					int nDelta = 0;
					int nIconHeight = rectIcon.Height ();

					if (nChar == VK_DOWN)
					{
						if (rectIcon.bottom + nIconHeight > rectPalette.bottom &&
							m_pScrollBar->GetScrollPos () <= m_pScrollBar->GetScrollLimit () - nIconHeight)
						{
							nDelta = -nIconHeight;
						}
					}
					else
					{
						if (rectIcon.top - nIconHeight < rectPalette.top &&
							m_pScrollBar->GetScrollPos () >= nIconHeight)
						{
							nDelta = nIconHeight;
						}
					}

					if (nDelta != 0)
					{
						ScrollPalette (nDelta, TRUE);
						m_pParentMenuBar->RedrawWindow (rectPalette);

						m_pScrollBar->SetScrollPos (m_nScrollOffset);
					}
				}
			}

			CArray<CBCGPBaseRibbonElement*, CBCGPBaseRibbonElement*> arElems;
			GetVisibleElements (arElems);

			if (m_bNavigateSearchResultsOnly)
			{
				for (int i = 0; i < arElems.GetSize();)
				{
					if (!arElems[i]->IsSearchResultMode())
					{
						arElems.RemoveAt(i);
					}
					else
					{
						i++;
					}
				}
			}

			int nScroll = 0;

			pNewHighlighted = CBCGPRibbonBar::FindNextFocusedElement (
				nChar, arElems, m_rect, m_pHighlighted, FALSE, FALSE, nScroll);

			if (IsMenuMode () && pNewHighlighted == NULL && !m_bNavigateSearchResultsOnly)
			{
				pNewHighlighted = nChar == VK_DOWN ? 
					GetFirstTabStop () : GetLastTabStop ();
			}
		}
		else
		{
			pNewHighlighted = nChar == VK_RIGHT || nChar == VK_DOWN || nChar == VK_TAB ? 
				GetFirstTabStop () : GetLastTabStop ();
		}
		break;

	case VK_RETURN:
	case VK_SPACE:
		bInvokeCommand = TRUE;
		break;

	default:
		if (IsMenuMode ())
		{
			BOOL bKeyIsPrintable = CBCGPKeyboardManager::IsKeyPrintable (nChar);

			UINT nUpperChar = nChar;
			if (bKeyIsPrintable)
			{
				nUpperChar = CBCGPKeyboardManager::TranslateCharToUpper (nChar);
			}

			for (int i = 0; i < m_arElements.GetSize (); i++)
			{
				CBCGPBaseRibbonElement* pElem = m_arElements [i];
				ASSERT_VALID (pElem);

				if (pElem->OnMenuKey (nUpperChar))
				{
					return TRUE;
				}

				CString strLabel = pElem->GetText ();

				int iAmpOffset = strLabel.Find (_T('&'));
				if (iAmpOffset >= 0 && iAmpOffset < strLabel.GetLength () - 1)
				{
					TCHAR szChar [2] = { strLabel.GetAt (iAmpOffset + 1), '\0' };
					CharUpper (szChar);

					UINT uiHotKey = (UINT) (szChar [0]);

					if (uiHotKey == nUpperChar)
					{
						if (!pElem->IsDisabled ())
						{
							pNewHighlighted = pElem;
							bInvokeCommand = TRUE;
						}
						break;
					}
				}
			}
		}
	}

	BOOL bRes = FALSE;

	if (pNewHighlighted != NULL)
	{
		ASSERT_VALID (pNewHighlighted);

		if (m_pHighlighted != pNewHighlighted)
		{
			if (m_pHighlighted != NULL)
			{
				ASSERT_VALID (m_pHighlighted);

				if (m_pHighlighted->m_bIsHighlighted)
				{
					m_pHighlighted->m_bIsHighlighted = FALSE;
					m_pHighlighted->OnHighlight (FALSE);
				}

				if (m_pHighlighted->m_bIsFocused)
				{
					m_pHighlighted->m_bIsFocused = FALSE;
					m_pHighlighted->OnSetFocus (FALSE);
				}

				m_pHighlighted->Redraw ();
				m_pHighlighted = NULL;
			}

			if (globalData.IsAccessibilitySupport () && m_pParentMenuBar != NULL)
			{
				CArray<CBCGPBaseRibbonElement*, CBCGPBaseRibbonElement*> arButtons;
				GetVisibleElements (arButtons);

				for (int i = 0; i < arButtons.GetSize (); i++)
				{
					CBCGPBaseRibbonElement* pElem = arButtons [i];
					ASSERT_VALID (pElem);

					if (pElem == pNewHighlighted)
					{
						m_pParentMenuBar->OnSetAccData  (i+1);
						globalData.NotifyWinEvent (EVENT_OBJECT_FOCUS, m_pParentMenuBar->GetSafeHwnd (), OBJID_CLIENT , i + 1);
						break;
					}
				}
			}

			m_pHighlighted = pNewHighlighted;
			pNewHighlighted->OnHighlight (TRUE);

			if (m_pPaletteButton != NULL)
			{
				MakePaletteItemVisible (pNewHighlighted);
			}

			pNewHighlighted->m_bIsHighlighted = TRUE;
			pNewHighlighted->m_bIsFocused = TRUE;
			pNewHighlighted->OnSetFocus (TRUE);

			m_pHighlighted->Redraw ();

			if (m_pParentMenuBar != NULL)
			{
				ASSERT_VALID (m_pParentMenuBar);

				CBCGPRibbonPanelMenu* pParentMenu = 
					DYNAMIC_DOWNCAST (CBCGPRibbonPanelMenu, m_pParentMenuBar->GetParent ());

				if (pParentMenu != NULL)
				{
					ASSERT_VALID (pParentMenu);

					if (pParentMenu->IsScrollable ())
					{
						CRect rectHighlighted = m_pHighlighted->GetRect ();

						CRect rectParent;
						m_pParentMenuBar->GetClientRect (rectParent);

						if (rectHighlighted.bottom > rectParent.bottom)
						{
							while (pParentMenu->IsScrollDnAvailable ())
							{
								m_pParentMenuBar->SetOffset (m_pParentMenuBar->GetOffset () + 1);
								pParentMenu->AdjustScroll ();

								if (m_pHighlighted->GetRect ().bottom <= rectParent.bottom)
								{
									m_pParentMenuBar->RedrawWindow ();
									break;
								}
							}
						}
						else if (rectHighlighted.top < rectParent.top)
						{
							while (pParentMenu->IsScrollUpAvailable ())
							{
								m_pParentMenuBar->SetOffset (m_pParentMenuBar->GetOffset () - 1);
								pParentMenu->AdjustScroll ();

								if (m_pHighlighted->GetRect ().top >= rectParent.top)
								{
									m_pParentMenuBar->RedrawWindow ();
									break;
								}
							}
						}
					}

					if (pParentMenu->GetParentRibbonElement () != NULL)
					{
						ASSERT_VALID (pParentMenu->GetParentRibbonElement ());
						pParentMenu->GetParentRibbonElement ()->OnChangeMenuHighlight (m_pParentMenuBar, m_pHighlighted);
					}
				}
			}
		}

		bRes = TRUE;
	}

	if (bInvokeCommand && m_pHighlighted != NULL)
	{
		ASSERT_VALID (m_pHighlighted);

		CBCGPRibbonButton* pButton = DYNAMIC_DOWNCAST (
			CBCGPRibbonButton, m_pHighlighted);

		if (pButton != NULL)
		{
			if (GetParentMenuBar() != NULL)
			{
				GetParentMenuBar()->StopWaitForPopup();
			}

			if (pButton->HasMenu ())
			{
				HWND hWndParent = GetParentMenuBar ()->GetSafeHwnd ();

				pButton->OnShowPopupMenu ();

				if (hWndParent != NULL && !::IsWindow (hWndParent))
				{
					return TRUE;
				}

				if (pButton->m_pPopupMenu != NULL)
				{
					ASSERT_VALID (pButton->m_pPopupMenu);
					pButton->m_pPopupMenu->SendMessage (WM_KEYDOWN, VK_HOME);
				}
			}
			else if (!pButton->IsDisabled ())
			{
				CBCGPRibbonBar* pRibbonBar = m_pParentMenuBar->GetTopLevelRibbonBar ();
				if (pRibbonBar != NULL && pRibbonBar->GetKeyboardNavLevelCurrent () == this)
				{
					pRibbonBar->DeactivateKeyboardFocus (TRUE);
				}

				pButton->OnClick (pButton->GetRect ().TopLeft ());

				if (pRibbonBar != NULL && pRibbonBar->GetTopLevelFrame () != NULL)
				{
					pRibbonBar->GetTopLevelFrame ()->SetFocus ();
				}
			}

			bRes = TRUE;
		}
	}

	return bRes;
}
//***************************************************************************
CBCGPBaseRibbonElement* CBCGPRibbonPanel::GetFirstTabStop () const
{
	ASSERT_VALID (this);

	int i = 0;

	if (m_pPaletteButton != NULL)
	{
		ASSERT_VALID (m_pPaletteButton);

		// First, find the "top element":
		for (i = 0; i < m_arElements.GetSize (); i++)
		{
			CBCGPBaseRibbonElement* pElem = m_arElements [i];
			ASSERT_VALID (pElem);

			BOOL bIsLabel = pElem->IsKindOf (RUNTIME_CLASS (CBCGPRibbonLabel));
			BOOL bIsIcon = pElem->IsKindOf (RUNTIME_CLASS (CBCGPRibbonPaletteIcon));

			if (bIsIcon || bIsLabel)
			{
				continue;
			}

			if (pElem->m_bIsOnPaletteTop)
			{
				CBCGPBaseRibbonElement* pTabStop = pElem->GetFirstTabStop ();
				if (pTabStop != NULL)
				{
					return pTabStop;
				}
			}
		}

		// Not found, return the first icon:
		for (i = 0; i < m_arElements.GetSize (); i++)
		{
			CBCGPBaseRibbonElement* pElem = m_arElements [i];
			ASSERT_VALID (pElem);

			if (pElem->IsKindOf (RUNTIME_CLASS (CBCGPRibbonPaletteIcon)))
			{
				CBCGPBaseRibbonElement* pTabStop = pElem->GetFirstTabStop ();
				if (pTabStop != NULL)
				{
					return pTabStop;
				}
			}
		}
	}

	for (i = 0; i < m_arElements.GetSize (); i++)
	{
		CBCGPBaseRibbonElement* pElem = m_arElements [i];
		ASSERT_VALID (pElem);

		CBCGPBaseRibbonElement* pTabStop = pElem->GetFirstTabStop ();
		if (pTabStop != NULL)
		{
			return pTabStop;
		}
	}

	return NULL;
}
//***************************************************************************
CBCGPBaseRibbonElement* CBCGPRibbonPanel::GetLastTabStop () const
{
	ASSERT_VALID (this);

	int i = 0;

	if (m_pPaletteButton != NULL)
	{
		ASSERT_VALID (m_pPaletteButton);

		// First, find the "bottom element":
		for (i = (int) m_arElements.GetSize () - 1; i >= 0; i--)
		{
			CBCGPBaseRibbonElement* pElem = m_arElements [i];
			ASSERT_VALID (pElem);

			BOOL bIsLabel = pElem->IsKindOf (RUNTIME_CLASS (CBCGPRibbonLabel));
			BOOL bIsIcon = pElem->IsKindOf (RUNTIME_CLASS (CBCGPRibbonPaletteIcon));

			if (bIsIcon || bIsLabel)
			{
				continue;
			}

			if (!pElem->m_bIsOnPaletteTop)
			{
				CBCGPBaseRibbonElement* pTabStop = pElem->GetFirstTabStop ();
				if (pTabStop != NULL)
				{
					return pTabStop;
				}
			}
		}

		// Not found, return the last icon:
		for (i = (int) m_arElements.GetSize () - 1; i >= 0; i--)
		{
			CBCGPBaseRibbonElement* pElem = m_arElements [i];
			ASSERT_VALID (pElem);

			if (pElem->IsKindOf (RUNTIME_CLASS (CBCGPRibbonPaletteIcon)))
			{
				CBCGPBaseRibbonElement* pTabStop = pElem->GetFirstTabStop ();
				if (pTabStop != NULL)
				{
					return pTabStop;
				}
			}
		}
	}

	for (i = (int) m_arElements.GetSize () - 1; i >= 0; i--)
	{
		CBCGPBaseRibbonElement* pElem = m_arElements [i];
		ASSERT_VALID (pElem);

		CBCGPBaseRibbonElement* pTabStop = pElem->GetLastTabStop ();
		if (pTabStop != NULL)
		{
			return pTabStop;
		}
	}

	return NULL;
}
//***************************************************************************
void CBCGPRibbonPanel::CleanUpSizes ()
{
	ASSERT_VALID (this);
	m_arWidths.RemoveAll ();

	for (int i = 0; i < m_arElements.GetSize (); i++)
	{
		CBCGPBaseRibbonElement* pElem = m_arElements [i];
		ASSERT_VALID (pElem);

		pElem->CleanUpSizes ();
	}

	m_btnDefault.CleanUpSizes ();
}
//***************************************************************************
void CBCGPRibbonPanel::ScrollPalette (int nScrollOffset, BOOL bIsDelta)
{
	ASSERT_VALID (this);

	const int nDelta = bIsDelta ? nScrollOffset : m_nScrollOffset - nScrollOffset;

	if (nDelta == 0)
	{
		return;
	}

	m_nScrollOffset = bIsDelta ? m_nScrollOffset - nDelta : nScrollOffset;

	for (int i = 0; i < m_arElements.GetSize (); i++)
	{
		CBCGPBaseRibbonElement* pElem = m_arElements [i];
		ASSERT_VALID (pElem);

		BOOL bIsLabel = pElem->IsKindOf (RUNTIME_CLASS (CBCGPRibbonLabel));
		BOOL bIsIcon = pElem->IsKindOf (RUNTIME_CLASS (CBCGPRibbonPaletteIcon));

		if (bIsLabel || bIsIcon)
		{
			pElem->m_rect.OffsetRect (0, nDelta);
		}
	}
}
//***************************************************************************
CBCGPBaseRibbonElement* CBCGPRibbonPanel::GetParentButton () const
{
	ASSERT_VALID (this);

	if (m_pParentMenuBar == NULL)
	{
		return NULL;
	}

	return ((CBCGPPopupMenu*)m_pParentMenuBar->GetParent ())->GetParentRibbonElement ();
}
//***************************************************************************
CSize CBCGPRibbonPanel::GetPaletteMinSize () const
{
	ASSERT_VALID (this);

	if (m_pPaletteButton == NULL)
	{
		ASSERT (FALSE);
		return CSize (-1, -1);
	}

	ASSERT_VALID (m_pPaletteButton);

	const BOOL bNoSideBar = m_pPaletteButton->IsKindOf (RUNTIME_CLASS (CBCGPRibbonUndoButton));

	CBCGPRibbonBar* pRibbonBar = m_pPaletteButton->GetTopLevelRibbonBar ();
	ASSERT_VALID (pRibbonBar);

	CClientDC dc (pRibbonBar);

	CFont* pOldFont = dc.SelectObject (pRibbonBar->GetFont ());
	ASSERT (pOldFont != NULL);

	const int cxScroll = ::GetSystemMetrics (SM_CXVSCROLL);

	int cxIcon = m_pPaletteButton->GetIconSize ().cx;
	int cyIcon = m_pPaletteButton->GetIconSize ().cy;

	int cxLabel = 0;
	int cyLabel = 0;

	int cxBottom = 0;
	int cyBottom = 0;

	for (int i = 0; i < m_arElements.GetSize (); i++)
	{
		CBCGPBaseRibbonElement* pElem = m_arElements [i];
		ASSERT_VALID (pElem);

		pElem->SetInitialMode ();
		pElem->OnCalcTextSize (&dc);

		CSize sizeElem = pElem->GetSize (&dc);

		if (pElem->IsKindOf (RUNTIME_CLASS (CBCGPRibbonPaletteIcon)))
		{
		}
		else if (pElem->IsKindOf (RUNTIME_CLASS (CBCGPRibbonLabel)))
		{
			cxLabel = max (cxLabel, sizeElem.cx);
			cyLabel = max (cyLabel, sizeElem.cy + m_pPaletteButton->GetGroupOffset ());
		}
		else
		{
			if (!bNoSideBar)
			{
				sizeElem.cx += 4 * TEXT_MARGIN + CBCGPToolBar::GetMenuImageSize ().cx + 
					2 * CBCGPVisualManager::GetInstance ()->GetMenuImageMargin ();
			}

			cxBottom = max (cxBottom, sizeElem.cx);
			cyBottom += sizeElem.cy;
		}
	}

	dc.SelectObject (pOldFont);

	int cx = max (cxIcon, cxLabel);

	return CSize (max (cx + cxScroll, cxBottom), cyIcon + cyBottom + cyLabel);
}
//***************************************************************************
void CBCGPRibbonPanel::SetKeys (LPCTSTR lpszKeys)
{
	ASSERT_VALID (this);
	m_btnDefault.SetKeys (lpszKeys);
}
//***************************************************************************
void CBCGPRibbonPanel::OnRTLChanged (BOOL bIsRTL)
{
	ASSERT_VALID (this);

	for (int i = 0; i < m_arElements.GetSize (); i++)
	{
		CBCGPBaseRibbonElement* pElem = m_arElements [i];
		ASSERT_VALID (pElem);

		pElem->OnRTLChanged (bIsRTL);
	}

	m_btnDefault.OnRTLChanged (bIsRTL);
	m_btnLaunch.OnRTLChanged (bIsRTL);
}
//***************************************************************************
CBCGPRibbonPanelMenu* CBCGPRibbonPanel::ShowPopup (CBCGPRibbonDefaultPanelButton* pButton/* = NULL*/)
{
	ASSERT_VALID (this);

	if (pButton == NULL)
	{
		pButton = &m_btnDefault;
	}

	ASSERT_VALID (pButton);

	CWnd* pWndParent = pButton->GetParentWnd ();
	if (pWndParent == NULL)
	{
		return NULL;
	}

	if (m_pParent != NULL && !pButton->IsQATMode ())
	{
		ASSERT_VALID(m_pParent);
		m_pParent->EnsureVisible(pButton);
	}

	const BOOL bIsRTL = (pWndParent->GetExStyle () & WS_EX_LAYOUTRTL);

	if (m_arWidths.GetSize () == 0)
	{
		ASSERT_VALID (m_pParent);

		CBCGPRibbonBar* pRibbonBar = pButton->GetTopLevelRibbonBar ();
		ASSERT_VALID (pRibbonBar);

		CClientDC dc (pRibbonBar);

		CFont* pOldFont = dc.SelectObject (pRibbonBar->GetFont ());
		ASSERT (pOldFont != NULL);

		int nHeight = m_pParent->GetMaxHeight (&dc);
		RecalcWidths (&dc, nHeight);

		dc.SelectObject (pOldFont);
	}

	CRect rectBtn = pButton->m_rect;
	pWndParent->ClientToScreen (&rectBtn);

	CBCGPRibbonPanelMenu* pMenu = new CBCGPRibbonPanelMenu (this);
	pMenu->SetParentRibbonElement (pButton);

	pMenu->Create (pWndParent, 
		bIsRTL ? rectBtn.right : rectBtn.left, 
		rectBtn.bottom, (HMENU) NULL);

	pButton->SetDroppedDown (pMenu);

	return pMenu;
}
//***************************************************************************
CRect CBCGPRibbonPanel::GetPaletteRect ()
{
	ASSERT_VALID (this);

	CRect rectPalette = m_rect;

	if (!m_rectMenuAreaTop.IsRectEmpty ())
	{
		rectPalette.top = m_rectMenuAreaTop.bottom;
	}

	if (!m_rectMenuAreaBottom.IsRectEmpty ())
	{
		rectPalette.bottom = m_rectMenuAreaBottom.top;
	}

	return rectPalette;
}
//***************************************************************************
void CBCGPRibbonPanel::MakePaletteItemVisible (CBCGPBaseRibbonElement* pItem)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pItem);

	if (!pItem->IsKindOf (RUNTIME_CLASS (CBCGPRibbonPaletteIcon)))
	{
		return;
	}

	CRect rectPalette = GetPaletteRect ();

	if (pItem == GetFirstTabStop ())
	{
		ScrollPalette (0);

		if (GetParentWnd () != NULL)
		{
			GetParentWnd ()->RedrawWindow (rectPalette);
		}

		if (m_pScrollBar->GetSafeHwnd () != NULL)
		{
			m_pScrollBar->SetScrollPos (m_nScrollOffset);
		}

		return;
	}

	CRect rectItem = pItem->GetRect ();

	int nDelta = 0;

	if (rectItem.top < rectPalette.top)
	{
		nDelta = rectPalette.top - rectItem.top;
	}
	else if (rectItem.bottom > rectPalette.bottom)
	{
		nDelta = rectPalette.bottom - rectItem.bottom;
	}

	if (nDelta != 0)
	{
		ScrollPalette (nDelta, TRUE);

		if (GetParentWnd () != NULL)
		{
			GetParentWnd ()->RedrawWindow (rectPalette);
		}

		if (m_pScrollBar->GetSafeHwnd () != NULL)
		{
			m_pScrollBar->SetScrollPos (m_nScrollOffset);
		}
	}
}
//***************************************************************************
void CBCGPRibbonPanel::SetFocused (CBCGPBaseRibbonElement* pNewFocus)
{
	ASSERT_VALID (this);

	CBCGPBaseRibbonElement* pFocused = GetFocused ();

	if (pNewFocus == pFocused)
	{
		return;
	}
			
	if (pFocused != NULL)
	{
		ASSERT_VALID (pFocused);

		pFocused->m_bIsFocused = FALSE;
		pFocused->OnSetFocus (FALSE);

		if (pFocused->m_bIsHighlighted)
		{
			pFocused->m_bIsHighlighted = FALSE;
			pFocused->OnHighlight (FALSE);

			if (m_pHighlighted == pFocused)
			{
				m_pHighlighted = NULL;
			}
		}

		pFocused->Redraw ();
	}

	if (pNewFocus != NULL)
	{
		ASSERT_VALID (pNewFocus);

		pNewFocus->m_bIsFocused = pNewFocus->m_bIsHighlighted = TRUE;
		pNewFocus->OnSetFocus (TRUE);
		pNewFocus->OnHighlight (TRUE);
		pNewFocus->Redraw ();

		m_pHighlighted = pNewFocus;
	}
}
//***************************************************************************
BOOL CBCGPRibbonPanel::IsScenicLook () const
{
	CBCGPRibbonCategory* pCategory = GetParentCategory ();
	if (pCategory == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	return pCategory->IsScenicLook ();
}
//*************************************************************************************
void CBCGPRibbonPanel::OnChangeVisualManager()
{
	ASSERT_VALID (this);

	for (int i = 0; i < m_arElements.GetSize (); i++)
	{
		CBCGPBaseRibbonElement* pElem = m_arElements [i];
		ASSERT_VALID (pElem);

		pElem->OnChangeVisualManager();
	}

	m_btnDefault.OnChangeVisualManager();
	m_btnLaunch.OnChangeVisualManager();
}
//*************************************************************************************
CString CBCGPRibbonPanel::GetDisplayName() const
{
	CBCGPRibbonCategory* pParentCategory = GetParentCategory ();
	if (pParentCategory == NULL)
	{
		return m_strName;
	}

	CBCGPRibbonBar* pRibbonBar = pParentCategory->GetParentRibbonBar();
	if (pRibbonBar == NULL)
	{
		return m_strName;
	}

	CString strName;
	if (!pRibbonBar->m_CustomizationData.GetPanelName(this, strName))
	{
		return m_strName;
	}

	return strName;
}
//*************************************************************************************
void CBCGPRibbonPanel::SetCustom()
{
	ASSERT_VALID (this);

	m_bIsCustom = TRUE;

	for (int i = 0; i < m_arElements.GetSize (); i++)
	{
		CBCGPBaseRibbonElement* pElem = m_arElements [i];
		ASSERT_VALID (pElem);

		pElem->SetCustom();
	}
}
//*************************************************************************************
void CBCGPRibbonPanel::OnCloseCustomizePage(BOOL bIsOK)
{
	if (!bIsOK)
	{
		m_bToBeDeleted = FALSE;
	}

	for (int i = 0; i < m_arElements.GetSize (); i++)
	{
		CBCGPBaseRibbonElement* pElem = m_arElements [i];
		ASSERT_VALID (pElem);

		if (pElem->m_bIsNew && !bIsOK)
		{
			delete pElem;
			m_arElements.RemoveAt(i);

			i--;
		}
		else
		{
			pElem->m_bIsNew = FALSE;
		}
	}
}
//*************************************************************************************
void CBCGPRibbonPanel::ApplyCustomizationData(CBCGPRibbonBar* pRibbonBar, CBCGPRibbonCustomPanel& customPanel)
{
	ASSERT_VALID(pRibbonBar);
	
	if (!IsCustom())
	{
		ASSERT(FALSE);
		return;
	}

	RemoveAll();

	for (int i = 0; i < customPanel.m_arElements.GetSize(); i++)
	{
		CBCGPRibbonCustomElement* pCustomElement = customPanel.m_arElements[i];
		ASSERT_VALID(pCustomElement);

		CBCGPBaseRibbonElement* pElem = pCustomElement->CreateRibbonElement(pRibbonBar);
		if (pElem != NULL)
		{
			ASSERT_VALID(pElem);
			Add(pElem);
		}
	}
}
//**************************************************************************************
BOOL CBCGPRibbonPanel::OnSetAccData (long lVal)
{
	ASSERT_VALID(this);

	m_AccData.Clear();

	int nIndex = (int)lVal - 1;

	CArray<CBCGPBaseRibbonElement*, CBCGPBaseRibbonElement*> arElements;
	GetVisibleElements(arElements);

	if (nIndex < 0 || nIndex >= arElements.GetSize())
	{
		return FALSE;
	}

	if (GetParentWnd()->GetSafeHwnd() == NULL)
	{
		return FALSE;
	}

	ASSERT_VALID(arElements[nIndex]);
	return arElements[nIndex]->SetACCData(GetParentWnd(), m_AccData);
}
//**************************************************************************************
HRESULT CBCGPRibbonPanel::get_accParent(IDispatch **ppdispParent)
{
	if (ppdispParent)
	{
		if (m_pParent != NULL)
		{
			*ppdispParent = m_pParent->GetIDispatch(TRUE);
		}

		if (*ppdispParent)
		{
			return S_OK;
		}
	}

	return E_INVALIDARG;
}
//*******************************************************************************
HRESULT CBCGPRibbonPanel::get_accChildCount(long *pcountChildren)
{
	if (!pcountChildren)
    {
        return E_INVALIDARG;
    }

	CArray<CBCGPBaseRibbonElement*, CBCGPBaseRibbonElement*> arElements;
	GetVisibleElements (arElements);

	*pcountChildren = (long)arElements.GetSize(); 
	return S_OK;
}
//*******************************************************************************
HRESULT CBCGPRibbonPanel::accLocation(long *pxLeft, long *pyTop, long *pcxWidth, long *pcyHeight, VARIANT varChild)
{
    if (!pxLeft || !pyTop || !pcxWidth || !pcyHeight)
    {
        return E_INVALIDARG;
    }

	if (varChild.vt == VT_I4 && varChild.lVal == CHILDID_SELF)
	{	
		if (GetParentWnd()->GetSafeHwnd() == NULL)
		{
			return S_FALSE;
		}

		CRect rectPanel = m_rect;
		GetParentWnd ()->ClientToScreen(&rectPanel);

		*pxLeft = rectPanel.left;
		*pyTop = rectPanel.top;
		*pcxWidth = rectPanel.Width();
		*pcyHeight = rectPanel.Height();

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
HRESULT CBCGPRibbonPanel::accNavigate(long navDir, VARIANT varStart, VARIANT* pvarEndUpAt)
{
	if (pvarEndUpAt == NULL)
	{
        return E_INVALIDARG;
	}

	pvarEndUpAt->vt = VT_EMPTY;

    if (varStart.vt != VT_I4)
    {
        return E_INVALIDARG;
    }

	CArray<CBCGPBaseRibbonElement*, CBCGPBaseRibbonElement*> arButtons;
	GetVisibleElements(arButtons);

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
			pvarEndUpAt->lVal = (int)arButtons.GetSize();
			return S_OK;
		}
		break;

	case NAVDIR_NEXT:   
	case NAVDIR_RIGHT:
		if (varStart.lVal != CHILDID_SELF)
		{
			pvarEndUpAt->vt = VT_I4;
			pvarEndUpAt->lVal = varStart.lVal + 1;

			if (pvarEndUpAt->lVal > arButtons.GetSize())
			{
				pvarEndUpAt->vt = VT_EMPTY;
				return S_FALSE;
			}
			return S_OK;
		}
		else if (m_pParent != NULL)
		{
			ASSERT_VALID(m_pParent);

			// Next Panel:
			int nIndex =  m_pParent->GetPanelIndex(this) + 1;
			if (nIndex < m_pParent->GetPanelCount())
			{
				CBCGPRibbonPanel* pPanel = m_pParent->GetPanel(nIndex);
				if (pPanel != NULL)
				{
					ASSERT_VALID(pPanel);

					pvarEndUpAt->vt = VT_DISPATCH;
					pvarEndUpAt->pdispVal = pPanel->GetIDispatch(TRUE);

					return S_OK;
				}
			}
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
		else if (m_pParent != NULL)
		{
			ASSERT_VALID(m_pParent);

			// Prev Panel:
			int nIndex =  m_pParent->GetPanelIndex(this) - 1;
			if (nIndex >= 0)
			{
				CBCGPRibbonPanel* pPanel = m_pParent->GetPanel(nIndex);
				if (pPanel != NULL)
				{
					ASSERT_VALID(pPanel);

					pvarEndUpAt->vt = VT_DISPATCH;
					pvarEndUpAt->pdispVal = pPanel->GetIDispatch(TRUE);

					return S_OK;
				}
			}
		}
		break;
	}

	return S_FALSE;
}
//*******************************************************************************
HRESULT CBCGPRibbonPanel::accHitTest(long xLeft, long yTop, VARIANT *pvarChild)
{
	if (!pvarChild)
    {
        return E_INVALIDARG;
    }

	pvarChild->vt = VT_I4;
	pvarChild->lVal = CHILDID_SELF;

	if (GetParentWnd()->GetSafeHwnd() == NULL)
	{
		return S_FALSE;
	}

	CBCGPRibbonBar* pParentRibbonBar = NULL;

	if (m_pParent != NULL)
	{
		ASSERT_VALID(m_pParent);
		pParentRibbonBar = m_pParent->GetParentRibbonBar();
	}

	if (pParentRibbonBar->GetSafeHwnd() == NULL)
	{
		return S_FALSE;
	}

	ASSERT_VALID(pParentRibbonBar);

	CPoint pt(xLeft, yTop);
	GetParentWnd ()->ScreenToClient(&pt);

	CArray<CBCGPBaseRibbonElement*, CBCGPBaseRibbonElement*> arElements;
	GetVisibleElements(arElements);

	for (int i = 0; i < arElements.GetSize(); i++)
	{
		CBCGPBaseRibbonElement* pElem = arElements[i];
		if (pElem != NULL)
		{
			ASSERT_VALID(pElem);

			CRect rectElem = pElem->GetRect();
		
			if (rectElem.PtInRect(pt))
			{
				pvarChild->lVal = i + 1;
				pElem->SetACCData(pParentRibbonBar, m_AccData);
				break;
			}
		}
	}

	return S_OK;
}
//*******************************************************************************
HRESULT CBCGPRibbonPanel::accDoDefaultAction(VARIANT varChild)
{
	if (varChild.vt != VT_I4)
    {
        return E_INVALIDARG;
    }

	if (varChild.lVal != CHILDID_SELF)
	{
		CArray<CBCGPBaseRibbonElement*, CBCGPBaseRibbonElement*> arElements;
		GetVisibleElements(arElements);

		int nIndex = (int)varChild.lVal - 1;
		if (nIndex < 0 || nIndex >= arElements.GetSize())
		{
			return E_INVALIDARG;
		}

		CBCGPBaseRibbonElement* pElem = arElements[nIndex];
		if (pElem != NULL)
		{
			ASSERT_VALID(pElem);

			pElem->OnAccDefaultAction();
			return S_OK;
		}
	}

	return S_FALSE;
}
//*******************************************************************************
BOOL CBCGPRibbonPanel::SetACCData(CWnd* /*pParent*/, CBCGPAccessibilityData& data)
{
	ASSERT_VALID (this);

	data.m_strAccDefAction.Empty();
	data.m_strAccValue.Empty();
	data.m_strDescription.Empty();
	data.m_strAccKeys.Empty();
	data.m_strAccName = m_strName;
	data.m_nAccRole = ROLE_SYSTEM_TOOLBAR;
	data.m_strAccValue = _T("Group");
	data.m_rectAccLocation = GetRect();

	if (m_pParent != NULL)
	{
		ASSERT_VALID(m_pParent);
		
		CBCGPRibbonBar* pParentRibbonBar = m_pParent->GetParentRibbonBar();
		if (pParentRibbonBar->GetSafeHwnd() != NULL)
		{
			ASSERT_VALID(pParentRibbonBar);
			pParentRibbonBar->ClientToScreen(&data.m_rectAccLocation);
		}
	}

	data.m_bAccState = 0;
	return TRUE;
}
//*******************************************************************************
void CBCGPRibbonPanel::SetPadding(const CSize& size)
{
	ASSERT_VALID(this);

	for (int i = 0; i < m_arElements.GetSize(); i++)
	{
		CBCGPBaseRibbonElement* pElem = m_arElements[i];
		if (pElem != NULL)
		{
			ASSERT_VALID(pElem);
			pElem->SetPadding(size);
		}
	}

	m_btnLaunch.SetPadding(size);
	m_btnDefault.SetPadding(size);

	m_sizePadding = size;
}
//*******************************************************************************
void CBCGPRibbonPanel::ForceCollapse()
{
	ASSERT_VALID(this);

	VerifyNonCollapsibleState();

	if (!IsNonCollapsible())
	{
		m_bForceCollpapse = TRUE;
	}
	
	m_nCurrWidthIndex = (int)m_arWidths.GetSize() - 1;
}
//*******************************************************************************
void CBCGPRibbonPanel::SetNonCollapsible(BOOL bSet/* = TRUE*/)
{
	ASSERT_VALID(this);

	m_bNonCollapsible = bSet;
	VerifyNonCollapsibleState();
}
//*******************************************************************************
BOOL CBCGPRibbonPanel::VerifyNonCollapsibleState()
{
	ASSERT_VALID(this);

	if (!m_bNonCollapsible)
	{
		return FALSE;
	}

	for (int i = 0; i < m_arElements.GetSize(); i++)
	{
		CBCGPBaseRibbonElement* pElem = m_arElements[i];
		if (pElem != NULL)
		{
			ASSERT_VALID(pElem);
			
			if (!pElem->CanBePlacedOnNonCollapsiblePanel())
			{
				TRACE(_T("Ribbon panel '%s' cannot be non-collapsible\n"), m_strName);

				m_bNonCollapsible = FALSE;
				return FALSE;
			}
		}
	}

	return TRUE;
}

#endif // BCGP_EXCLUDE_RIBBON
