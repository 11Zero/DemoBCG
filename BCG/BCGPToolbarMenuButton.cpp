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

// BCGToolbarMenuButton.cpp: implementation of the CBCGPToolbarMenuButton class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "menuhash.h"
#include "BCGCBPro.h"
#include "BCGPToolbarMenuButton.h"
#include "BCGPMenuBar.h"
#include "BCGPPopupMenuBar.h"
#include "BCGPCommandManager.h"
#include "BCGGlobals.h"
#include "BCGPKeyboardManager.h"

#include "BCGPFrameWnd.h"
#include "BCGPMDIFrameWnd.h"
#include "BCGPOleIPFrameWnd.h"
#include "BCGPOleDocIPFrameWnd.h"

#include "MenuImages.h"
#include "BCGPUserToolsManager.h"
#include "BCGPTearOffManager.h"
#include "BCGPUserTool.h"
#include "BCGPRegistry.h"
#include "BCGPWorkspace.h"
#include "BCGPVisualManager.h"
#include "bcgprores.h"

#include "BCGPTabWnd.h"
#include "BCGPDropDownList.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_SERIAL(CBCGPToolbarMenuButton, CBCGPToolbarButton, VERSIONABLE_SCHEMA | 1)

extern CBCGPWorkspace* g_pWorkspace;

BOOL CBCGPToolbarMenuButton::m_bAlwaysCallOwnerDraw = FALSE;

static const CString strDummyAmpSeq = _T("\001\001");

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGPToolbarMenuButton::CBCGPToolbarMenuButton()
{
	Initialize ();
}
//*****************************************************************************************
CBCGPToolbarMenuButton::CBCGPToolbarMenuButton (UINT uiID, HMENU hMenu, 
								int iImage, LPCTSTR lpszText, BOOL bUserButton)
{
	Initialize (uiID, hMenu, iImage, lpszText, bUserButton);
}
//*****************************************************************************************
void CBCGPToolbarMenuButton::Initialize ()
{
	m_bDrawDownArrow = FALSE;
	m_bMenuMode = FALSE;
	m_pPopupMenu = NULL;
	m_bDefault = FALSE;
	m_bClickedOnMenu = FALSE;
	m_bHorz = TRUE;
	m_bMenuOnly	= FALSE;
	m_bToBeClosed = FALSE;
	m_uiTearOffBarID = 0;
	m_bIsRadio = FALSE;
	m_pWndMessage = NULL;
	m_bMenuPaletteMode = FALSE;
	m_nPaletteRows = 1;
	m_bQuickCustomMode = FALSE;
	m_bShowAtRightSide = FALSE;
	m_rectArrow.SetRectEmpty ();
	m_rectButton.SetRectEmpty ();
}
//*****************************************************************************************
void CBCGPToolbarMenuButton::Initialize (UINT uiID, HMENU hMenu, int iImage, LPCTSTR lpszText,
								BOOL bUserButton)
{
	Initialize ();

	m_nID = uiID;
	m_bUserButton = bUserButton;

	SetImage (iImage);
	m_strText = (lpszText == NULL) ? _T("") : lpszText;

	CreateFromMenu (hMenu);
}
//*****************************************************************************************
CBCGPToolbarMenuButton::CBCGPToolbarMenuButton (const CBCGPToolbarMenuButton& src)
{
	m_nID = src.m_nID;
	m_nStyle = src.m_nStyle;
	m_bUserButton = src.m_bUserButton;

	SetImage (src.GetImage ());
	m_strText = src.m_strText;
	m_bDragFromCollection = FALSE;
	m_bText = src.m_bText;
	m_bImage = src.m_bImage;
	m_bDrawDownArrow = src.m_bDrawDownArrow;
	m_bMenuMode = src.m_bMenuMode;
	m_bDefault = src.m_bDefault;
	m_bMenuOnly	= src.m_bMenuOnly;
	m_bIsRadio = src.m_bIsRadio;

	SetTearOff (src.m_uiTearOffBarID);

	HMENU hmenu = src.CreateMenu ();
	ASSERT (hmenu != NULL);

	CreateFromMenu (hmenu);
	::DestroyMenu (hmenu);

	m_rect.SetRectEmpty ();

	m_pPopupMenu = NULL;
	m_pWndParent = NULL;

	m_bClickedOnMenu = FALSE;
	m_bHorz = TRUE;

	m_bMenuPaletteMode = src.m_bMenuPaletteMode;
	m_nPaletteRows = src.m_nPaletteRows;
	m_bQuickCustomMode = src.m_bQuickCustomMode;
	m_bShowAtRightSide = src.m_bShowAtRightSide;
}
//*****************************************************************************************
CBCGPToolbarMenuButton::~CBCGPToolbarMenuButton()
{
	if (m_pPopupMenu != NULL)
	{
		m_pPopupMenu->m_pParentBtn = NULL;
	}

	while (!m_listCommands.IsEmpty ())
	{
		delete m_listCommands.RemoveHead ();
	}

	if (m_uiTearOffBarID != 0 && g_pBCGPTearOffMenuManager != NULL)
	{
		g_pBCGPTearOffMenuManager->SetInUse (m_uiTearOffBarID, FALSE);
	}
}

//////////////////////////////////////////////////////////////////////
// Overrides:

void CBCGPToolbarMenuButton::CopyFrom (const CBCGPToolbarButton& s)
{
	CBCGPToolbarButton::CopyFrom (s);

	const CBCGPToolbarMenuButton& src = (const CBCGPToolbarMenuButton&) s;

	m_bDefault = src.m_bDefault;
	m_bMenuOnly	= src.m_bMenuOnly;
	m_bIsRadio = src.m_bIsRadio;
	m_pWndMessage = src.m_pWndMessage;
	m_bMenuPaletteMode = src.m_bMenuPaletteMode;
	m_nPaletteRows = src.m_nPaletteRows;
	m_bQuickCustomMode = src.m_bQuickCustomMode;
	m_bShowAtRightSide = src.m_bShowAtRightSide;

	SetTearOff (src.m_uiTearOffBarID);

	while (!m_listCommands.IsEmpty ())
	{
		delete m_listCommands.RemoveHead ();
	}

	for (POSITION pos = src.m_listCommands.GetHeadPosition (); pos != NULL;)
	{
		CBCGPToolbarMenuButton* pItem = (CBCGPToolbarMenuButton*) src.m_listCommands.GetNext (pos);
		ASSERT (pItem != NULL);
		ASSERT_KINDOF (CBCGPToolbarMenuButton, pItem);

		CRuntimeClass* pSrcClass = pItem->GetRuntimeClass ();
		ASSERT (pSrcClass != NULL);

		CBCGPToolbarMenuButton* pNewItem = (CBCGPToolbarMenuButton*) pSrcClass->CreateObject ();
		ASSERT (pNewItem != NULL);
		ASSERT_KINDOF (CBCGPToolbarMenuButton, pNewItem);

		pNewItem->CopyFrom (*pItem);
		m_listCommands.AddTail (pNewItem);
	}
}
//*****************************************************************************************
void CBCGPToolbarMenuButton::Serialize (CArchive& ar)
{
	CBCGPToolbarButton::Serialize (ar);

	if (ar.IsLoading ())
	{
		while (!m_listCommands.IsEmpty ())
		{
			delete m_listCommands.RemoveHead ();
		}

		UINT uiTearOffBarID;
		ar >> uiTearOffBarID;

		SetTearOff (uiTearOffBarID);

		if (g_menuHash.IsActive () ||
			(g_pWorkspace != NULL && g_pWorkspace->GetDataVersion () >= 0x60520 && g_pWorkspace->GetDataVersion () != 0x70000))
		{
			ar >> m_bMenuPaletteMode;
			ar >> m_nPaletteRows;
		}
	}
	else
	{
		ar << m_uiTearOffBarID;

		if (g_menuHash.IsActive () ||
			(g_pWorkspace != NULL && g_pWorkspace->GetDataVersion () >= 0x60520))
		{
			ar << m_bMenuPaletteMode;
			ar << m_nPaletteRows;
		}
	}

	m_listCommands.Serialize (ar);
}
//*****************************************************************************************
void CBCGPToolbarMenuButton::OnDraw (CDC* pDC, const CRect& rect, CBCGPToolBarImages* pImages,
			BOOL bHorz, BOOL bCustomizeMode, BOOL bHighlight,
			BOOL bDrawBorder, BOOL bGrayDisabledButtons)
{
	m_rectArrow.SetRectEmpty ();
	m_rectButton.SetRectEmpty ();

	if (m_bMenuMode)
	{
		DrawMenuItem (pDC, rect, pImages, bCustomizeMode, bHighlight, bGrayDisabledButtons);
		return;
	}

	BOOL bIsFlatLook = CBCGPVisualManager::GetInstance ()->IsMenuFlatLook ();

	const int nSeparatorSize = 2;

	if (m_bMenuPaletteMode)
	{
		m_nStyle &= ~TBBS_CHECKED;
	}

	//----------------------
	// Fill button interior:
	//----------------------
	FillInterior (pDC, rect, bHighlight || IsDroppedDown ());

	CSize sizeImage = CBCGPMenuImages::Size ();
	if (CBCGPToolBar::IsLargeIcons ())
	{
		sizeImage.cx *= 2;
		sizeImage.cy *= 2;
	}

	CRect rectInternal = rect;
	CSize sizeExtra = m_bExtraSize ? 
		CBCGPVisualManager::GetInstance ()->GetButtonExtraBorder () : CSize (0, 0);

	if (sizeExtra != CSize (0, 0))
	{
		rectInternal.DeflateRect (sizeExtra.cx / 2 + 1, sizeExtra.cy / 2 + 1);
	}

	CRect rectParent = rect;
	m_rectArrow = rectInternal;

	const int nMargin = CBCGPVisualManager::GetInstance ()->GetMenuImageMargin ();
	const int xMargin = bHorz ? nMargin : 0;
	const int yMargin = bHorz ? 0 : nMargin;

	rectParent.DeflateRect (xMargin, yMargin);

	if (m_bDrawDownArrow)
	{
		if (bHorz)
		{
			rectParent.right -= sizeImage.cx + nSeparatorSize - 2 + sizeExtra.cx;
			m_rectArrow.left = rectParent.right + 1;

			if (sizeExtra != CSize (0, 0))
			{
				m_rectArrow.OffsetRect (
					-sizeExtra.cx / 2 + 1,
					-sizeExtra.cy / 2 + 1);
			}
		}
		else
		{
			rectParent.bottom -= sizeImage.cy + nSeparatorSize - 1;
			m_rectArrow.top = rectParent.bottom;
		}
	}

	UINT uiStyle = m_nStyle;

	if (bIsFlatLook)
	{
		m_nStyle &= ~(TBBS_PRESSED | TBBS_CHECKED);
	}
	else
	{
		if (m_bClickedOnMenu && m_nID != 0 && m_nID != (UINT) -1 && !m_bMenuOnly) 
		{
			m_nStyle &= ~TBBS_PRESSED;
		}
		else if (m_pPopupMenu != NULL)
		{
			m_nStyle |= TBBS_PRESSED;
		}
	}

	BOOL bDisableFill = m_bDisableFill;
	m_bDisableFill = TRUE;

	CBCGPToolbarButton::OnDraw (pDC, rectParent, pImages, bHorz, 
			bCustomizeMode, bHighlight, bDrawBorder, bGrayDisabledButtons);

	m_bDisableFill = bDisableFill;

	if (m_bDrawDownArrow)
	{
		if ((m_nStyle & (TBBS_PRESSED | TBBS_CHECKED)) && !bIsFlatLook)
		{
			m_rectArrow.OffsetRect (1, 1);
		}

		if ((bHighlight || (m_nStyle & TBBS_PRESSED) ||
			m_pPopupMenu != NULL) &&
			m_nID != 0 && m_nID != (UINT) -1 && !m_bMenuOnly)
		{
			//----------------
			// Draw separator:
			//----------------
			CRect rectSeparator = m_rectArrow;

			if (bHorz)
			{
				rectSeparator.right = rectSeparator.left + nSeparatorSize;
			}
			else
			{
				rectSeparator.bottom = rectSeparator.top + nSeparatorSize;
			}

			CBCGPVisualManager::BCGBUTTON_STATE state = CBCGPVisualManager::ButtonsIsRegular;

			if (bHighlight || (m_nStyle & (TBBS_PRESSED | TBBS_CHECKED)))
			{
				//-----------------------
				// Pressed in or checked:
				//-----------------------
				state = CBCGPVisualManager::ButtonsIsPressed;
			}

			if (!m_bClickedOnMenu)
			{
				CBCGPVisualManager::GetInstance ()->OnDrawButtonSeparator (pDC,
												this, rectSeparator, state, bHorz);
			}
		}

		BOOL bDisabled = (bCustomizeMode && !IsEditable ()) ||
			(!bCustomizeMode && (m_nStyle & TBBS_DISABLED));

		CBCGPMenuImages::IMAGE_STATE imageState = (CBCGPMenuImages::IMAGE_STATE) CBCGPVisualManager::GetInstance ()->
			GetMenuDownArrowState (this, bHighlight, (m_nStyle & (TBBS_PRESSED | TBBS_CHECKED)), bDisabled);

		if (CBCGPToolBarImages::m_bIsDrawOnGlass)
		{
			const BOOL bIsZoomed = m_pWndParent->GetSafeHwnd () != NULL &&
				m_pWndParent->GetParentFrame ()->GetSafeHwnd () != NULL &&
				m_pWndParent->GetParentFrame ()->IsZoomed ();

			if (bIsZoomed && CBCGPVisualManager::GetInstance ()->IsToolBarButtonDefaultBackground (this, 
				(bHighlight || (m_nStyle & (TBBS_PRESSED | TBBS_CHECKED))) ? 
					CBCGPVisualManager::ButtonsIsPressed : CBCGPVisualManager::ButtonsIsRegular))
			{
				imageState = CBCGPMenuImages::ImageWhite;
			}
		}

		CBCGPMenuImages::Draw (pDC, bHorz ? CBCGPMenuImages::IdArowDown : CBCGPMenuImages::IdArowRight, 
							m_rectArrow, imageState, 
							CBCGPToolBarImages::m_bIsDrawOnGlass ? CSize (0, 0) : sizeImage);
	}

	m_nStyle = uiStyle;

	if (!bCustomizeMode)
	{
		if ((m_nStyle & (TBBS_PRESSED | TBBS_CHECKED)) ||
			m_pPopupMenu != NULL)
		{
			//-----------------------
			// Pressed in or checked:
			//-----------------------
			if (!bIsFlatLook &&
				m_bClickedOnMenu && m_nID != 0 && m_nID != (UINT) -1 && !m_bMenuOnly)
			{
				rectParent.right++;

				CBCGPVisualManager::GetInstance ()->OnDrawButtonBorder (pDC,
					this, rectParent, CBCGPVisualManager::ButtonsIsHighlighted);

				CBCGPVisualManager::GetInstance ()->OnDrawButtonBorder (pDC,
					this, m_rectArrow, CBCGPVisualManager::ButtonsIsPressed);
			}
			else
			{
				CBCGPVisualManager::GetInstance ()->OnDrawButtonBorder (pDC,
					this, rect, CBCGPVisualManager::ButtonsIsPressed);
			}
		}
		else if (bHighlight && !(m_nStyle & TBBS_DISABLED) &&
			!(m_nStyle & (TBBS_CHECKED | TBBS_INDETERMINATE)))
		{
			CBCGPVisualManager::GetInstance ()->OnDrawButtonBorder (pDC,
				this, rect, CBCGPVisualManager::ButtonsIsHighlighted);
		}
	}
}
//*****************************************************************************************
SIZE CBCGPToolbarMenuButton::OnCalculateSize (CDC* pDC, const CSize& sizeDefault, BOOL bHorz)
{
	m_bHorz = bHorz;

	if (!IsVisible())
	{
		return CSize (0,0);
	}

	int nArrowSize = 0;
	const int nSeparatorSize = 2;

	if (m_bDrawDownArrow || m_bMenuMode)
	{
		if (m_bMenuMode)
		{
			nArrowSize = (bHorz) ? 
				globalData.GetTextWidth () : globalData.GetTextHeight ();
		}
		else
		{
			nArrowSize = (bHorz) ? 
				CBCGPMenuImages::Size ().cx : CBCGPMenuImages::Size ().cy;

			if (CBCGPToolBar::IsLargeIcons ())
			{
				nArrowSize *= 2;
			}
		}

		nArrowSize += nSeparatorSize - TEXT_MARGIN - 1;
	}

	//--------------------
	// Change accelerator:
	//--------------------
	if (g_pKeyboardManager != NULL &&
		m_bMenuMode &&
		(m_nID < 0xF000 || m_nID >= 0xF1F0))	// Not system.
	{
		//-----------------------------------
		// Remove standard aceleration label:
		//-----------------------------------
		int iTabOffset = m_strText.Find (_T('\t'));
		if (iTabOffset >= 0)
		{
			m_strText = m_strText.Left (iTabOffset);
		}

		//---------------------------------
		// Add an actual accelartion label:
		//---------------------------------
		CString strAccel;
		if (GetKeyboardAccelerator(strAccel))
		{
			m_strText += _T('\t');
			m_strText += strAccel;
		}
	}

	CFont* pOldFont = NULL;

	if (m_nID == nBCGPMenuGroupID)
	{
		pOldFont = pDC->SelectObject (&globalData.fontBold);
		ASSERT_VALID (pOldFont);
	}

	CSize size = CBCGPToolbarButton::OnCalculateSize (pDC, sizeDefault, bHorz);

	if (pOldFont != NULL)
	{
		pDC->SelectObject (pOldFont);
	}

	CBCGPPopupMenuBar* pParentMenu =
		DYNAMIC_DOWNCAST (CBCGPPopupMenuBar, m_pWndParent);

	if (pParentMenu != NULL)
	{
		size.cy = pParentMenu->GetRowHeight ();

		if (pParentMenu->IsDropDownListMode ())
		{
			CBCGPDropDownList* pList = DYNAMIC_DOWNCAST (CBCGPDropDownList, 
				pParentMenu->GetParent ());

			if (pList != NULL)
			{
				return pList->OnGetItemSize (pDC, this, size);
			}
		}
	}

	if (bHorz)
	{	
		size.cx += nArrowSize;
	}
	else
	{
		size.cy += nArrowSize;
	}

	if (m_bMenuMode)
	{
		size.cx += sizeDefault.cx + 2 * TEXT_MARGIN;
	}

	if (!m_bMenuMode)
	{
		const int nMargin = CBCGPVisualManager::GetInstance ()->GetMenuImageMargin ();

		if (bHorz)
		{
			size.cx += nMargin * 2;
		}
		else
		{
			size.cy += nMargin * 2;
		}
	}

	return size;
}
//*****************************************************************************************
BOOL CBCGPToolbarMenuButton::OnClick (CWnd* pWnd, BOOL bDelay)
{
	ASSERT_VALID (pWnd);

	m_bClickedOnMenu = FALSE;

	if (m_bDrawDownArrow && !bDelay && !m_bMenuMode)
	{
		if (m_nID == 0 || m_nID == (UINT) -1 || m_bMenuOnly)
		{
			m_bClickedOnMenu = TRUE;
		}
		else
		{
			CPoint ptMouse;
			::GetCursorPos (&ptMouse);
			pWnd->ScreenToClient (&ptMouse);

			m_bClickedOnMenu = m_rectArrow.PtInRect (ptMouse);
			if (!m_bClickedOnMenu)
			{
				return FALSE;
			}
		}
	}

	if (HasButton () && !bDelay)
	{
		CPoint ptMouse;
		::GetCursorPos (&ptMouse);
		pWnd->ScreenToClient (&ptMouse);

		if (m_rectButton.PtInRect (ptMouse))
		{
			return FALSE;
		}
	}

	if (!m_bClickedOnMenu && m_nID > 0 && m_nID != (UINT) -1 && !m_bDrawDownArrow &&
		!m_bMenuOnly)
	{
		return FALSE;
	}

	CBCGPMenuBar* pMenuBar = DYNAMIC_DOWNCAST (CBCGPMenuBar, m_pWndParent);

	if (m_pPopupMenu != NULL)
	{
		//-----------------------------------------------------
		// Second click to the popup menu item closes the menu:
		//-----------------------------------------------------		
		ASSERT_VALID(m_pPopupMenu);

		m_pPopupMenu->m_bAutoDestroyParent = FALSE;
		m_pPopupMenu->DestroyWindow ();
		m_pPopupMenu = NULL;

		if (pMenuBar != NULL)
		{
			pMenuBar->SetHot (NULL);
		}
	}
	else
	{
		CBCGPPopupMenuBar* pParentMenu =
			DYNAMIC_DOWNCAST (CBCGPPopupMenuBar, m_pWndParent);

		if (bDelay && pParentMenu != NULL && !CBCGPToolBar::IsCustomizeMode ())
		{
			pParentMenu->StartPopupMenuTimer (this);
		}
		else
		{
			if (pMenuBar != NULL)
			{
				CBCGPToolbarMenuButton* pCurrPopupMenuButton = 
					pMenuBar->GetDroppedDownMenu();
				if (pCurrPopupMenuButton != NULL)
				{
					pCurrPopupMenuButton->OnCancelMode ();
				}
			}
			
			if (!OpenPopupMenu (pWnd))
			{
				return FALSE;
			}
		}

		if (pMenuBar != NULL)
		{
			pMenuBar->SetHot (this);
		}
	}

	if (m_pWndParent != NULL)
	{
		CRect rect = m_rect;

		const int nShadowSize = 
			CBCGPVisualManager::GetInstance ()->GetMenuShadowDepth ();

		rect.InflateRect (nShadowSize, nShadowSize);
		m_pWndParent->RedrawWindow (rect, NULL, RDW_FRAME | RDW_INVALIDATE);
	}

	return TRUE;
}
//****************************************************************************************
void CBCGPToolbarMenuButton::OnChangeParentWnd (CWnd* pWndParent)
{
	CBCGPToolbarButton::OnChangeParentWnd (pWndParent);

	if (pWndParent != NULL)
	{
		if (pWndParent->IsKindOf (RUNTIME_CLASS (CBCGPMenuBar)))
		{
            m_bDrawDownArrow = (m_nID != 0 && !m_listCommands.IsEmpty ()) ||
                ((CBCGPMenuBar *)pWndParent)->GetForceDownArrows();
			m_bText = TRUE;
			m_bImage = FALSE;
		}
		else
		{
			m_bDrawDownArrow = (m_nID == 0 || !m_listCommands.IsEmpty ());
		}

		if (pWndParent->IsKindOf (RUNTIME_CLASS (CBCGPPopupMenuBar)))
		{
			m_bMenuMode = TRUE;
			m_bText = TRUE;
			m_bImage = FALSE;
			m_bDrawDownArrow = (m_nID == 0 || !m_listCommands.IsEmpty ()) || HasButton ();
		}
		else
		{
			m_bMenuMode = FALSE;
		}
	}
}
//****************************************************************************************
void CBCGPToolbarMenuButton::CreateFromMenu (HMENU hMenu)
{
	while (!m_listCommands.IsEmpty ())
	{
		delete m_listCommands.RemoveHead ();
	}

	if (!::IsMenu (hMenu))
	{
		return;
	}

	CMenu* pMenu = CMenu::FromHandle (hMenu);
	if (pMenu == NULL)
	{
		return;
	}

	UINT uiDefaultCmd = ::GetMenuDefaultItem (hMenu, FALSE, GMDI_USEDISABLED);

	int iCount = (int) pMenu->GetMenuItemCount ();
	for (int i = 0; i < iCount; i ++)
	{
		CBCGPToolbarMenuButton* pItem = STATIC_DOWNCAST(CBCGPToolbarMenuButton, GetRuntimeClass()->CreateObject());
		ASSERT_VALID (pItem);

		pItem->m_nID = pMenu->GetMenuItemID (i);
		pMenu->GetMenuString (i, pItem->m_strText, MF_BYPOSITION);

		if (pItem->m_nID == -1)	// Sub-menu...
		{
			if (g_pBCGPTearOffMenuManager != NULL)
			{
				pItem->SetTearOff (g_pBCGPTearOffMenuManager->Parse (pItem->m_strText));
			}

			CMenu* pSubMenu = pMenu->GetSubMenu (i);
			pItem->CreateFromMenu (pSubMenu->GetSafeHmenu ());
		}
		else if (pItem->m_nID == uiDefaultCmd)
		{
			pItem->m_bDefault = TRUE;
		}

		UINT uiState = pMenu->GetMenuState (i, MF_BYPOSITION);

		if (uiState & MF_MENUBREAK)
		{
			pItem->m_nStyle |= TBBS_BREAK;
		}

		if ((uiState & MF_DISABLED) || (uiState & MF_GRAYED))
		{
			pItem->m_nStyle |= TBBS_DISABLED;
		}

		if (uiState & MF_CHECKED)
		{
			if ((uiState & MFT_RADIOCHECK) == MFT_RADIOCHECK)
			{
				pItem->m_bIsRadio = TRUE;
			}

			pItem->m_nStyle |= TBBS_CHECKED;
		}

		m_listCommands.AddTail (pItem);
	}
}
//****************************************************************************************
HMENU CBCGPToolbarMenuButton::CreateMenu () const
{
	if (m_listCommands.IsEmpty () && m_nID != (UINT) -1 && m_nID != 0 && !m_bMenuOnly)
	{
		return NULL;
	}

	CMenu menu;
	if (!menu.CreatePopupMenu ())
	{
		TRACE(_T("CBCGPToolbarMenuButton::CreateMenu (): Can't create popup menu!\n"));
		return NULL;
	}

	BOOL bRes = TRUE;
	DWORD dwLastError = 0;

	UINT uiDefaultCmd = (UINT) -1;

	int i = 0;
	for (POSITION pos = m_listCommands.GetHeadPosition (); pos != NULL; i ++)
	{
		CBCGPToolbarMenuButton* pItem = (CBCGPToolbarMenuButton*) m_listCommands.GetNext (pos);
		ASSERT (pItem != NULL);
		ASSERT_KINDOF (CBCGPToolbarMenuButton, pItem);

		UINT uiStyle = MF_STRING;

		if (pItem->m_nStyle & TBBS_BREAK)
		{
			uiStyle |= MF_MENUBREAK;
		}

		if (pItem->m_nStyle & TBBS_DISABLED)   
		{
		   uiStyle |= MF_DISABLED;
		}

		if (pItem->m_nStyle & TBBS_CHECKED)
		{
		   uiStyle |= MF_CHECKED;
		}

		if (pItem->m_bIsRadio)
		{
		   uiStyle |= MFT_RADIOCHECK;
		}

		if (pItem->IsTearOffMenu ())
		{
			uiStyle |= MF_MENUBARBREAK;
		}

		switch (pItem->m_nID)
		{
		case 0:	// Separator
			bRes = menu.AppendMenu (MF_SEPARATOR);
			if (!bRes)
			{
				dwLastError = GetLastError ();
			}
			break;

		case -1:			// Sub-menu
			{
				HMENU hSubMenu = pItem->CreateMenu ();
				ASSERT (hSubMenu != NULL);

				CString strText = pItem->m_strText;
				if (pItem->m_uiTearOffBarID != 0 && g_pBCGPTearOffMenuManager != NULL)
				{
					g_pBCGPTearOffMenuManager->Build (pItem->m_uiTearOffBarID, strText);
				}

				bRes = menu.AppendMenu (uiStyle | MF_POPUP, (UINT_PTR) hSubMenu, strText);
				if (!bRes)
				{
					dwLastError = GetLastError ();
				}

				//--------------------------------------------------------
				// This is incompatibility between Windows 95 and 
				// NT API! (IMHO). CMenu::AppendMenu with MF_POPUP flag 
				// COPIES sub-menu resource under the Win NT and 
				// MOVES sub-menu under Win 95/98 and 2000!
				//--------------------------------------------------------
				if (globalData.bIsWindowsNT4)
				{
					::DestroyMenu (hSubMenu);
				}
			}
			break;

		default:
			if (pItem->m_bDefault)
			{
				uiDefaultCmd = pItem->m_nID;
			}

			bRes = menu.AppendMenu (uiStyle, pItem->m_nID, pItem->m_strText);
			if (!bRes)
			{
				dwLastError = GetLastError ();
			}
		}

		if (!bRes)
		{
			TRACE(_T("CBCGPToolbarMenuButton::CreateMenu (): Can't add menu item: %d\n Last error = %x\n"), pItem->m_nID, dwLastError);
			return NULL;
		}
	}

	HMENU hMenu = menu.Detach ();
	if (uiDefaultCmd != (UINT)-1)
	{
		::SetMenuDefaultItem (hMenu, uiDefaultCmd, FALSE);
	}

	return hMenu;
}
//*****************************************************************************************
void CBCGPToolbarMenuButton::DrawMenuItem (CDC* pDC, const CRect& rect, CBCGPToolBarImages* pImages, 
					BOOL bCustomizeMode, BOOL bHighlight, BOOL bGrayDisabledButtons,
					BOOL bContentOnly)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (this);

	if (m_nID == nBCGPMenuGroupID)
	{
		COLORREF clrText = 
			CBCGPVisualManager::GetInstance ()->OnDrawMenuLabel (pDC, rect);

		COLORREF clrTextOld = pDC->SetTextColor (clrText);

		CRect rectText = rect;
		rectText.DeflateRect (TEXT_MARGIN, 0);
		rectText.bottom -= 2;

		CFont* pOldFont = pDC->SelectObject (&globalData.fontBold);
		ASSERT_VALID (pOldFont);

		pDC->DrawText (m_strText, rectText, DT_SINGLELINE | DT_VCENTER);
		
		pDC->SetTextColor (clrTextOld);
		pDC->SelectObject (pOldFont);
		return;
	}

	BOOL bDisabled = (bCustomizeMode && !IsEditable ()) ||
		(!bCustomizeMode && (m_nStyle & TBBS_DISABLED));

	CBCGPToolBarImages* pLockedImages = NULL;
	CBCGPToolBarImages* pUserImages = NULL;
	CBCGPDrawState ds(CBCGPVisualManager::GetInstance()->IsAutoGrayscaleImages());

	CBCGPPopupMenuBar* pParentMenu =
		DYNAMIC_DOWNCAST (CBCGPPopupMenuBar, m_pWndParent);

	CSize sizeMenuImage = CBCGPToolBar::GetMenuImageSize ();

	if (pParentMenu != NULL)
	{
		if (pParentMenu->IsDropDownListMode ())
		{
			CBCGPDropDownList* pList = DYNAMIC_DOWNCAST (CBCGPDropDownList, 
				pParentMenu->GetParent ());

			if (pList != NULL)
			{
				COLORREF clrText = CBCGPVisualManager::GetInstance ()->GetMenuItemTextColor (
					this, bHighlight, FALSE);

				if (bHighlight)
				{
					CBCGPVisualManager::GetInstance ()->
						OnHighlightMenuItem (pDC, this, rect, clrText);
				}

				COLORREF clrTextOld = pDC->SetTextColor (clrText);

				pList->OnDrawItem (pDC, this, bHighlight);

				pDC->SetTextColor (clrTextOld);
				return;
			}
		}

		if (pParentMenu->m_pRelatedToolbar != NULL && 
			pParentMenu->m_pRelatedToolbar->IsLocked ())
		{
			pLockedImages = (CBCGPToolBarImages*) pParentMenu->m_pRelatedToolbar->GetLockedMenuImages ();

			if (pLockedImages != NULL)
			{
				CSize sizeDest (0, 0);

				if (sizeMenuImage != pParentMenu->GetCurrentMenuImageSize ())
				{
					sizeDest = sizeMenuImage;
				}

				pLockedImages->PrepareDrawImage (ds, sizeDest);

				pImages = pLockedImages;
			}
		}
	}

	BOOL bDisableImage = BCGPCMD_MGR.IsMenuItemWithoutImage (m_nID);
	if (m_nID == ID_BCGBARRES_TASKPANE_BACK ||
		m_nID == ID_BCGBARRES_TASKPANE_FORWARD)
	{
		bDisableImage = TRUE;
	}

	CBCGPUserTool* pUserTool = NULL;
	if (g_pUserToolsManager != NULL && !m_bUserButton)
	{
		pUserTool = g_pUserToolsManager->FindTool (m_nID);
	}

	HICON hDocIcon = CBCGPTabWnd::GetDocumentIcon (m_nID);

	CSize sizeImage = CBCGPMenuImages::Size ();

	if (m_pPopupMenu != NULL && !m_bToBeClosed)
	{
		bHighlight = TRUE;
	}

	COLORREF clrText = CBCGPVisualManager::GetInstance ()->GetMenuItemTextColor (
		this, bHighlight, bDisabled);

	BOOL bDrawImageFrame = !CBCGPVisualManager::GetInstance ()->IsHighlightWholeMenuItem ();

	if (bHighlight && !bContentOnly &&
		CBCGPVisualManager::GetInstance ()->IsHighlightWholeMenuItem ())
	{
		CBCGPVisualManager::GetInstance ()->OnHighlightMenuItem (pDC, this, rect, clrText);
		bDrawImageFrame = FALSE;
	}

	if ((m_nStyle & TBBS_CHECKED) &&
		!CBCGPVisualManager::GetInstance ()->IsOwnerDrawMenuCheck ())
	{
		bDrawImageFrame = TRUE;
	}

	CFont* pOldFont = NULL;

	if (m_nID != 0 && m_nID != (UINT) -1 && !m_bMenuOnly && 
		pParentMenu != NULL && pParentMenu->GetDefaultMenuId () == m_nID)
	{
		pOldFont = (CFont*) pDC->SelectObject (&globalData.fontBold);
	}

	CRect rectImage;
	rectImage = rect;
	rectImage.left += CBCGPVisualManager::GetInstance ()->GetMenuImageMargin ();
	rectImage.right = rectImage.left + sizeMenuImage.cx + 
		CBCGPVisualManager::GetInstance ()->GetMenuImageMargin ();

	CRect rectFrameBtn = rectImage;

	if (CBCGPVisualManager::GetInstance ()->IsHighlightWholeMenuItem ())
	{
		rectFrameBtn = rect;

		CRect rectOffset = CBCGPVisualManager::GetInstance ()->GetMenuImageFrameOffset ();

		rectFrameBtn.left += rectOffset.left;
		rectFrameBtn.top += rectOffset.top;
		rectFrameBtn.bottom -= rectOffset.bottom;
		rectFrameBtn.right = rectImage.right;
	}
	else
	{
		rectFrameBtn.InflateRect (1, -1);
	}

	BOOL bIsRarelyUsed = (CBCGPMenuBar::IsRecentlyUsedMenus () && 
		CBCGPToolBar::IsCommandRarelyUsed (m_nID));
	
	if (bIsRarelyUsed)
	{
		bIsRarelyUsed = FALSE;

		CBCGPPopupMenuBar* pParentMenuBar =
			DYNAMIC_DOWNCAST (CBCGPPopupMenuBar, m_pWndParent);

		if (pParentMenuBar != NULL)
		{
			CBCGPPopupMenu* pParentMenu = DYNAMIC_DOWNCAST (CBCGPPopupMenu, 
				pParentMenuBar->GetParent ());
			if (pParentMenu != NULL && pParentMenu->HideRarelyUsedCommands ())
			{
				bIsRarelyUsed = TRUE;
			}
		}
	}

	BOOL bLightImage = FALSE;
	BOOL bFadeImage = !bHighlight && CBCGPVisualManager::GetInstance ()->IsFadeInactiveImage ();

	if (bIsRarelyUsed)
	{
		bLightImage = TRUE;
		if (bHighlight && (m_nStyle & (TBBS_CHECKED | TBBS_INDETERMINATE)))
		{
			bLightImage = FALSE;
		}

		if (GetImage () < 0 && !(m_nStyle & (TBBS_CHECKED | TBBS_INDETERMINATE)))
		{
			bLightImage = FALSE;
		}
	}
	else if (m_nStyle & (TBBS_CHECKED | TBBS_INDETERMINATE))
	{
		bLightImage = !bHighlight;
	}

	//----------------
	// Draw the image:
	//----------------
	if (!IsDrawImage () && hDocIcon == NULL)	// Try to find a matched image
	{
		BOOL bImageSave = m_bImage;
		BOOL bUserButton = m_bUserButton;
		BOOL bSuccess = TRUE;

		m_bImage = TRUE;	// Always try to draw image!
		m_bUserButton = TRUE;

		if (GetImage () < 0)
		{
			m_bUserButton = FALSE;

			if (GetImage () < 0)
			{
				bSuccess = FALSE;
			}
		}

		if (!bSuccess)
		{
			m_bImage = bImageSave;
			m_bUserButton = bUserButton;
		}

		if (m_bUserButton && pImages != CBCGPToolBar::GetUserImages ())
		{
			pUserImages = CBCGPToolBar::GetUserImages ();

			if (pUserImages != NULL)
			{
				ASSERT_VALID (pUserImages);

				CSize sizeImageDest(0, 0);

				if (pImages != NULL)
				{
					CSize sizeImageDefault = pImages->GetImageSize();
				
					if (sizeImageDefault != pUserImages->GetImageSize())
					{
						sizeImageDest = sizeImageDefault;
					}
				}

				pUserImages->PrepareDrawImage (ds, sizeImageDest);
				pImages = pUserImages;
			}
		}
	}

	BOOL bImageIsReady = FALSE;

	CRgn rgnClip;
	rgnClip.CreateRectRgnIndirect (&rectImage);

	if (bDrawImageFrame && !bContentOnly)
	{
		FillInterior (pDC, rectFrameBtn, bHighlight, TRUE);
	}

	if (!bDisableImage && (IsDrawImage () && pImages != NULL) || hDocIcon != NULL)
	{
		BOOL bDrawImageShadow = 
			bHighlight && !bCustomizeMode &&
			CBCGPVisualManager::GetInstance ()->IsShadowHighlightedImage () &&
			!globalData.IsHighContastMode () &&
			((m_nStyle & TBBS_CHECKED) == 0) &&
			((m_nStyle & TBBS_DISABLED) == 0);

		pDC->SelectObject (&rgnClip);

		CPoint ptImageOffset (
			(rectImage.Width () - sizeMenuImage.cx) / 2, 
			(rectImage.Height () - sizeMenuImage.cy) / 2);

		if ((m_nStyle & TBBS_PRESSED) || 
			!(m_nStyle & TBBS_DISABLED) ||
			!bGrayDisabledButtons ||
			bCustomizeMode)
		{
			CRect rectIcon (CPoint (rectImage.left + ptImageOffset.x, 
							rectImage.top + ptImageOffset.y),
							sizeMenuImage);

			if (hDocIcon != NULL)
			{
				DrawDocumentIcon (pDC, rectIcon, hDocIcon);
			}
			else if (pUserTool != NULL)
			{
				pUserTool->DrawToolIcon (pDC, rectIcon);
			}
			else
			{
				CPoint pt = rectImage.TopLeft ();
				pt += ptImageOffset;

				if (globalData.GetRibbonImageScale () != 1. && CBCGPToolBar::m_bDontScaleImages)
				{
					pt.x += max (0, (rectImage.Width () - pImages->GetImageSize ().cx) / 2);
					pt.y += max (0, (rectImage.Width () - pImages->GetImageSize ().cy) / 2);
				}

				if (bDrawImageShadow)
				{
					pt.Offset (1, 1);

					pImages->Draw (pDC, 
						pt.x,
						pt.y, 
						GetImage (), FALSE, FALSE, FALSE, TRUE);

					pt.Offset (-2, -2);
				}

				pImages->Draw (pDC, 
					pt.x, 
					pt.y, 
					GetImage (),
					FALSE, bDisabled && bGrayDisabledButtons,
					FALSE, FALSE, bFadeImage);
			}

			bImageIsReady = TRUE;
		}

		if (!bImageIsReady)
		{
			CRect rectIcon (CPoint (rectImage.left + ptImageOffset.x, 
							rectImage.top + ptImageOffset.y),
							sizeMenuImage);

			if (hDocIcon != NULL)
			{
				DrawDocumentIcon (pDC, rectIcon, hDocIcon);
			}
			else if (pUserTool != NULL)
			{
				pUserTool->DrawToolIcon (pDC, rectIcon);
			}
			else
			{
				if (bDrawImageShadow)
				{
					rectImage.OffsetRect (1, 1);
					pImages->Draw (pDC, 
						rectImage.left + ptImageOffset.x,
						rectImage.top + ptImageOffset.y,
						GetImage (), FALSE, FALSE, FALSE, TRUE);

					rectImage.OffsetRect (-2, -2);
				}

				pImages->Draw (pDC, 
					rectImage.left + ptImageOffset.x, rectImage.top + ptImageOffset.y, 
					GetImage (), FALSE, bDisabled && bGrayDisabledButtons,
					FALSE, FALSE, bFadeImage);
			}

			bImageIsReady = TRUE;
		}
	}
	
	if (m_bAlwaysCallOwnerDraw || !bImageIsReady)
	{
		CFrameWnd* pParentFrame = m_pWndParent == NULL ?
			DYNAMIC_DOWNCAST (CFrameWnd, AfxGetMainWnd ()) :
			BCGCBProGetTopLevelFrame (m_pWndParent);

		//------------------------------------
		// Get chance to user draw menu image:
		//------------------------------------
		CBCGPMDIFrameWnd* pMainFrame = DYNAMIC_DOWNCAST (CBCGPMDIFrameWnd, pParentFrame);
		if (pMainFrame != NULL)
		{
			bImageIsReady = pMainFrame->OnDrawMenuImage (pDC, this, rectImage);
		}
		else	// Maybe, SDI frame...
		{
			CBCGPFrameWnd* pFrame = DYNAMIC_DOWNCAST (CBCGPFrameWnd, pParentFrame);
			if (pFrame != NULL)
			{
				bImageIsReady = pFrame->OnDrawMenuImage (pDC, this, rectImage);
			}
			else	// Maybe, OLE frame...
			{
				CBCGPOleIPFrameWnd* pOleFrame = 
					DYNAMIC_DOWNCAST (CBCGPOleIPFrameWnd, pParentFrame);
				if (pOleFrame != NULL)
				{
					bImageIsReady = pOleFrame->OnDrawMenuImage (pDC, this, rectImage);
				}
				else
				{
					CBCGPOleDocIPFrameWnd* pOleDocFrame = 
						DYNAMIC_DOWNCAST (CBCGPOleDocIPFrameWnd, pParentFrame);
					if (pOleDocFrame != NULL)
					{
						bImageIsReady = pOleDocFrame->OnDrawMenuImage (pDC, this, rectImage);
					}
				}

			}
		}
	}

	pDC->SelectClipRgn (NULL);

	if (m_nStyle & TBBS_CHECKED)
	{
		if (bDrawImageFrame)
		{
			UINT nStyleSaved = m_nStyle;

			if (bHighlight && 
				CBCGPVisualManager::GetInstance ()->IsFrameMenuCheckedItems ())
			{
				m_nStyle |= TBBS_MARKED;
			}

			CBCGPVisualManager::GetInstance ()->OnDrawMenuImageRectBorder (pDC,
				this, rectFrameBtn, CBCGPVisualManager::ButtonsIsPressed);

			m_nStyle = nStyleSaved;
		}

		if (!bImageIsReady)
		{
			CBCGPVisualManager::GetInstance ()->OnDrawMenuCheck (pDC, this, 
				rectFrameBtn, bHighlight, m_bIsRadio);
		}
	}
	else if (!bContentOnly && bImageIsReady && bHighlight && bDrawImageFrame)
	{
		CBCGPVisualManager::GetInstance ()->OnDrawMenuImageRectBorder (pDC,
			this, rectFrameBtn, CBCGPVisualManager::ButtonsIsHighlighted);
	}

	rectImage.InflateRect (1, 0);
	int iSystemImageId = -1;

	//-------------------------------
	// Try to draw system menu icons:
	//-------------------------------
	if (!bImageIsReady)
	{
		switch (m_nID)
		{
		case SC_MINIMIZE:
			iSystemImageId = CBCGPMenuImages::IdMinimize;
			break;

		case SC_RESTORE:
			iSystemImageId = CBCGPMenuImages::IdRestore;
			break;

		case SC_CLOSE:
			iSystemImageId = CBCGPMenuImages::IdClose;
			break;

		case SC_MAXIMIZE:
			iSystemImageId = CBCGPMenuImages::IdMaximize;
			break;
		}

		if (iSystemImageId != -1)
		{
			CRect rectSysImage = rectImage;
			rectSysImage.DeflateRect (CBCGPVisualManager::GetInstance ()->GetMenuImageMargin (), CBCGPVisualManager::GetInstance ()->GetMenuImageMargin ());

			if (!bContentOnly && bDrawImageFrame)
			{
				FillInterior (pDC, rectFrameBtn, bHighlight, TRUE);
			}

			CBCGPMenuImages::Draw (pDC, (CBCGPMenuImages::IMAGES_IDS) iSystemImageId, 
				rectSysImage,
				bDisabled ? CBCGPMenuImages::ImageGray : CBCGPMenuImages::ImageBlack);

			if (bHighlight && !bContentOnly && bDrawImageFrame)
			{
				CBCGPVisualManager::GetInstance ()->OnDrawMenuImageRectBorder (pDC,
					this, rectFrameBtn, CBCGPVisualManager::ButtonsIsHighlighted);
			}
		}
	}

	//-------------------------------
	// Fill text area if highlighted:
	//-------------------------------
	CRect rectText = rect;
	rectText.left = rectFrameBtn.right + CBCGPVisualManager::GetInstance ()->GetMenuImageMargin () + 2;

	if (bHighlight)
	{
		if (!CBCGPVisualManager::GetInstance ()->IsHighlightWholeMenuItem ())
		{
			CRect rectFill = rectFrameBtn;

			if ((m_nStyle & (TBBS_CHECKED) || bImageIsReady) ||
				iSystemImageId != -1)
			{
				rectFill.left = rectText.left - 1;
			}

			rectFill.right = rect.right - 1;

			if (!bContentOnly)
			{
				CBCGPVisualManager::GetInstance ()->OnHighlightMenuItem (pDC, this, rectFill, clrText);
			}
			else
			{
				clrText = CBCGPVisualManager::GetInstance ()->GetHighlightedMenuItemTextColor (this);
			}
		}
		else if (bContentOnly)
		{
			clrText = CBCGPVisualManager::GetInstance ()->GetHighlightedMenuItemTextColor (this);
		}
	}

	//-------------------------
	// Find acceleration label:
	//-------------------------
	CString strText = m_strText;
	CString strAccel;

	int iTabOffset = m_strText.Find (_T('\t'));
	if (iTabOffset >= 0)
	{
		strText = strText.Left (iTabOffset);
		strAccel = m_strText.Mid (iTabOffset + 1);
	}

	//-----------
	// Draw text:
	//-----------
	COLORREF clrTextOld = pDC->GetTextColor ();

	rectText.left += TEXT_MARGIN;

	if (!m_bWholeText)
	{
		CString strEllipses (_T("..."));
		while (strText.GetLength () > 0 &&
			pDC->GetTextExtent (strText + strEllipses).cx > rectText.Width ())
		{
			strText = strText.Left (strText.GetLength () - 1);
		}

		strText += strEllipses;
	}

	if (!globalData.m_bUnderlineKeyboardShortcuts && !CBCGPToolBar::IsCustomizeMode ())
	{
		strText.Replace (_T("&&"), strDummyAmpSeq);
		strText.Remove (_T('&'));
		strText.Replace (strDummyAmpSeq, _T("&&"));
	}

	if (bDisabled && !bHighlight && 
		CBCGPVisualManager::GetInstance ()->IsEmbossDisabledImage ())
	{
		pDC->SetTextColor (globalData.clrBtnHilite);

		CRect rectShft = rectText;
		rectShft.OffsetRect (1, 1);
		pDC->DrawText (strText, &rectShft, DT_SINGLELINE | DT_VCENTER);
	}

	pDC->SetTextColor (clrText);
	pDC->DrawText (strText, &rectText, DT_SINGLELINE | DT_VCENTER);

	//------------------------
	// Draw accelerator label:
	//------------------------
	if (!strAccel.IsEmpty ())
	{
		CRect rectAccel = rectText;
		rectAccel.right -= TEXT_MARGIN + sizeImage.cx;

		if (bDisabled && !bHighlight && CBCGPVisualManager::GetInstance ()->IsEmbossDisabledImage ())
		{
			pDC->SetTextColor (globalData.clrBtnHilite);

			CRect rectAccelShft = rectAccel;
			rectAccelShft.OffsetRect (1, 1);
			pDC->DrawText (strAccel, &rectAccelShft, DT_SINGLELINE | DT_RIGHT | DT_VCENTER);
		}

		pDC->SetTextColor (clrText);
		pDC->DrawText (strAccel, &rectAccel, DT_SINGLELINE | DT_RIGHT | DT_VCENTER);
	}

	//--------------------------------------------
	// Draw triangle image for the cascade menues:
	//--------------------------------------------
	if (m_nID == (UINT) -1 || m_bDrawDownArrow || m_bMenuOnly)
	{
		CFont* pRegFont = pDC->SelectObject (&globalData.fontMarlett);
		ASSERT (pRegFont != NULL);

		CRect rectTriangle = rect;

		CString strTriangle = (m_pWndParent->GetExStyle() & WS_EX_LAYOUTRTL) ?
			_T("3") : _T("4");	// Marlett's right arrow

		if (m_bQuickCustomMode)
		{
			strTriangle = _T("6");  	// Marlett's down arrow
		}
		
		if (HasButton ())
		{
			m_rectButton = rect;

			m_rectButton.left = m_rectButton.right - pDC->GetTextExtent (strTriangle).cx;

			CBCGPVisualManager::GetInstance ()->OnDrawMenuItemButton (pDC, this,
				m_rectButton, bHighlight, bDisabled);
		}

		pDC->DrawText (strTriangle, &rectTriangle, DT_SINGLELINE | DT_RIGHT | DT_VCENTER);

		pDC->SelectObject (pRegFont);
	}

	if (pOldFont != NULL)
	{
		pDC->SelectObject (pOldFont);
	}

	pDC->SetTextColor (clrTextOld);

	if (pLockedImages != NULL)
	{
		pLockedImages->EndDrawImage (ds);
	}

	if (pUserImages != NULL)
	{
		ASSERT_VALID (pUserImages);
		pUserImages->EndDrawImage (ds);
	}
}
//****************************************************************************************
void CBCGPToolbarMenuButton::OnCancelMode ()
{
	if (m_pPopupMenu != NULL && ::IsWindow (m_pPopupMenu->m_hWnd))
	{
		if (m_pPopupMenu->InCommand ())
		{
			return;
		}

		for (int i = 0; i < m_pPopupMenu->GetMenuItemCount (); i++)
		{
			CBCGPToolbarMenuButton* pSubItem = m_pPopupMenu->GetMenuItem (i);
			if (pSubItem != NULL)
			{
				pSubItem->OnCancelMode ();
			}
		}

		m_pPopupMenu->SaveState ();
		m_pPopupMenu->m_bAutoDestroyParent = FALSE;
		m_pPopupMenu->CloseMenu ();
	}

	m_pPopupMenu = NULL;

	if (m_pWndParent != NULL && ::IsWindow (m_pWndParent->m_hWnd))
	{
		CRect rect = m_rect;
		
		const int nShadowSize = 
			CBCGPVisualManager::GetInstance ()->GetMenuShadowDepth ();

		rect.InflateRect (nShadowSize, nShadowSize);

		m_pWndParent->InvalidateRect (rect);
		m_pWndParent->UpdateWindow ();
	}

	m_bToBeClosed = FALSE;
}
//****************************************************************************************
BOOL CBCGPToolbarMenuButton::OpenPopupMenu (CWnd* pWnd)
{
	if (m_pPopupMenu != NULL)
	{
		return FALSE;
	}

	if (pWnd == NULL)
	{
		pWnd = m_pWndParent;
	}

	ASSERT (pWnd != NULL);

	HMENU hMenu = CreateMenu ();
	if (hMenu == NULL && !IsEmptyMenuAllowed ())
	{
		return FALSE;
	}

	m_pPopupMenu = CreatePopupMenu ();
	
	if (m_pPopupMenu == NULL)
	{
		::DestroyMenu (hMenu);
		return FALSE;
	}

	if (m_pPopupMenu->GetMenuItemCount () > 0 && hMenu != NULL)
	{
		::DestroyMenu (hMenu);
		hMenu = NULL;
	}

	//---------------------------------------------------------------
	// Define a new menu position. Place the menu in the right side
	// of the current menu in the poup menu case or under the current 
	// item by default:
	//---------------------------------------------------------------
	CPoint point;
	CBCGPPopupMenu::DROP_DIRECTION dropDir = CBCGPPopupMenu::DROP_DIRECTION_NONE;

	CBCGPPopupMenuBar* pParentMenu =
		DYNAMIC_DOWNCAST (CBCGPPopupMenuBar, m_pWndParent);

	CBCGPMenuBar* pParentMenuBar =
		DYNAMIC_DOWNCAST (CBCGPMenuBar, m_pWndParent);

	if (pParentMenu != NULL)
	{
		point = CPoint (0, m_rect.top - 2);
		pWnd->ClientToScreen (&point);

		CRect rectParent;
		pParentMenu->GetWindowRect (rectParent);

		int nMenuGap = CBCGPVisualManager::GetInstance ()->GetPopupMenuGap ();

		if (pParentMenu->GetExStyle () & WS_EX_LAYOUTRTL)
		{
			point.x = rectParent.left - nMenuGap;
			dropDir = CBCGPPopupMenu::DROP_DIRECTION_LEFT;
		}
		else
		{
			point.x = rectParent.right + nMenuGap;
			dropDir = CBCGPPopupMenu::DROP_DIRECTION_RIGHT;
		}
	}
	else if (pParentMenuBar != NULL && 
		(pParentMenuBar->IsHorizontal ()) == 0)
	{
		//------------------------------------------------
		// Parent menu bar is docked vertical, place menu 
		// in the left or right side of the parent frame:
		//------------------------------------------------
		point = CPoint (m_rect.right, m_rect.top);
		pWnd->ClientToScreen (&point);

		dropDir = CBCGPPopupMenu::DROP_DIRECTION_RIGHT;
	}
	else
	{
		if (m_bShowAtRightSide)
		{
			point = CPoint (m_rect.right - 1, m_rect.top);
		}
		else
		{
			if (m_pPopupMenu->IsRightAlign())
			{
				point = CPoint (m_rect.right - 1, m_rect.bottom - 1);
			}
			else
			{
				point = CPoint (m_rect.left, m_rect.bottom - 1);
			}
		}

		dropDir = CBCGPPopupMenu::DROP_DIRECTION_BOTTOM;
		pWnd->ClientToScreen (&point);
	}

	m_pPopupMenu->m_pParentBtn = this;
	m_pPopupMenu->m_DropDirection = dropDir;

	if (!m_pPopupMenu->Create (pWnd, point.x, point.y, hMenu))
	{
		m_pPopupMenu = NULL;
		return FALSE;
	}

	OnAfterCreatePopupMenu ();

	if (m_pWndMessage != NULL)
	{
		ASSERT_VALID (m_pWndMessage);
		m_pPopupMenu->SetMessageWnd (m_pWndMessage);
	}
	else
	{
		// If parent menu has a message window, the child should have the same
		CBCGPPopupMenu* pCallerMenu = DYNAMIC_DOWNCAST (CBCGPPopupMenu, pWnd->GetParent ());
		if (pCallerMenu != NULL && pCallerMenu->GetMessageWnd() != NULL) 
		{
			m_pPopupMenu->SetMessageWnd (pCallerMenu->GetMessageWnd ());
		}
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPToolbarMenuButton diagnostics

#ifdef _DEBUG
void CBCGPToolbarMenuButton::AssertValid() const
{
	CObject::AssertValid();
}
//******************************************************************************************
void CBCGPToolbarMenuButton::Dump(CDumpContext& dc) const
{
	CObject::Dump (dc);

	CString strId;
	strId.Format (_T("%x"), m_nID);

	dc << "[" << m_strText << " >>>>> ]";
	dc.SetDepth (dc.GetDepth () + 1);

	dc << "{\n";
	for (POSITION pos = m_listCommands.GetHeadPosition (); pos != NULL;)
	{
		CBCGPToolbarButton* pButton = (CBCGPToolbarButton*) m_listCommands.GetNext (pos);
		ASSERT_VALID (pButton);

		pButton->Dump (dc);
		dc << "\n";
	}

	dc << "}\n";
	dc.SetDepth (dc.GetDepth () - 1);
	dc << "\n";
}

#endif

//******************************************************************************************
int CBCGPToolbarMenuButton::OnDrawOnCustomizeList (
	CDC* pDC, const CRect& rect, BOOL bSelected)
{
	CBCGPToolbarButton::OnDrawOnCustomizeList (pDC, rect, bSelected);

	if (m_nID == 0 || !m_listCommands.IsEmpty () || HasButton ())	// Popup menu
	{
		CBCGPVisualManager::GetInstance ()->OnDrawMenuArrowOnCustomizeList (
			pDC, rect, bSelected);
	}

	return rect.Width ();
}
//*******************************************************************************************
BOOL CBCGPToolbarMenuButton::OnBeforeDrag () const
{
	if (m_pPopupMenu != NULL)	// Is dropped down
	{
		m_pPopupMenu->CollapseSubmenus ();
		m_pPopupMenu->SendMessage (WM_CLOSE);
	}

	return CBCGPToolbarButton::OnBeforeDrag ();
}
//*******************************************************************************************
void CBCGPToolbarMenuButton::GetTextHorzOffsets (int& xOffsetLeft, int& xOffsetRight)
{
	xOffsetLeft = CBCGPToolBar::GetMenuImageSize ().cx / 2 + TEXT_MARGIN;
	xOffsetRight = CBCGPMenuImages::Size ().cx;
}
//*******************************************************************************************
void CBCGPToolbarMenuButton::SaveBarState ()
{
	if (m_pWndParent == NULL)
	{
		return;
	}

	CBCGPPopupMenu* pParentMenu =
		DYNAMIC_DOWNCAST (CBCGPPopupMenu, m_pWndParent->GetParent ());
	if (pParentMenu == NULL)
	{
		return;
	}

	ASSERT_VALID (pParentMenu);

	CBCGPPopupMenu* pTopLevelMenu = pParentMenu;
	while ((pParentMenu = DYNAMIC_DOWNCAST (CBCGPPopupMenu, pParentMenu->GetParent ()))
		!= NULL)
	{
		pTopLevelMenu = pParentMenu;
	}

	ASSERT_VALID (pTopLevelMenu);
	pTopLevelMenu->SaveState ();
}
//*************************************************************************************************
void CBCGPToolbarMenuButton::GetImageRect (CRect& rectImage)
{
	ASSERT_VALID (this);

	rectImage = m_rect;
	rectImage.left += CBCGPVisualManager::GetInstance ()->GetMenuImageMargin ();

	rectImage.right = rectImage.left + 
					CBCGPToolBar::GetMenuImageSize ().cx + CBCGPVisualManager::GetInstance ()->GetMenuImageMargin ();
}
//*************************************************************************************************
void CBCGPToolbarMenuButton::SetTearOff (UINT uiBarID)
{
	if (m_uiTearOffBarID == uiBarID)
	{
		return;
	}

	if (g_pBCGPTearOffMenuManager != NULL)
	{
		if (m_uiTearOffBarID != 0)
		{
			g_pBCGPTearOffMenuManager->SetInUse (m_uiTearOffBarID, FALSE);
		}

		if (uiBarID != 0)
		{
			g_pBCGPTearOffMenuManager->SetInUse (uiBarID);
		}
	}

	m_uiTearOffBarID = uiBarID;
}
//*************************************************************************************************
void CBCGPToolbarMenuButton::SetMenuPaletteMode (BOOL bMenuPaletteMode/* = TRUE*/, 
												 int nPaletteRows/* = 1*/)
{
	ASSERT_VALID (this);
	ASSERT (!IsDroppedDown ());

	m_bMenuPaletteMode = bMenuPaletteMode;
	m_nPaletteRows = nPaletteRows;
}
//***********************************************************************************
void CBCGPToolbarMenuButton::SetRadio ()
{
	m_bIsRadio = TRUE;

	if (m_pWndParent != NULL)
	{
		CRect rectImage;
		GetImageRect (rectImage);

		m_pWndParent->InvalidateRect (rectImage);
		m_pWndParent->UpdateWindow ();
	}
}
//*************************************************************************************
void CBCGPToolbarMenuButton::ResetImageToDefault ()
{
	ASSERT_VALID (this);

	CBCGPToolbarButton::ResetImageToDefault ();

	for (POSITION pos = m_listCommands.GetHeadPosition (); pos != NULL;)
	{
		CBCGPToolbarMenuButton* pItem = (CBCGPToolbarMenuButton*) m_listCommands.GetNext (pos);
		ASSERT_VALID (pItem);

		pItem->ResetImageToDefault ();
	}
}
//********************************************************************************
BOOL CBCGPToolbarMenuButton::CompareWith (const CBCGPToolbarButton& other) const
{
	if (m_nID != other.m_nID)
	{
		return FALSE;
	}

	const CBCGPToolbarMenuButton& otherMenuBtn = (const CBCGPToolbarMenuButton&) other;

	if (m_listCommands.GetCount () != otherMenuBtn.m_listCommands.GetCount ())
	{
		return FALSE;
	}

	POSITION pos1 = otherMenuBtn.m_listCommands.GetHeadPosition ();

	for (POSITION pos = m_listCommands.GetHeadPosition (); pos != NULL;)
	{
		ASSERT (pos1 != NULL);

		CBCGPToolbarMenuButton* pItem = (CBCGPToolbarMenuButton*) m_listCommands.GetNext (pos);
		ASSERT_VALID (pItem);

		CBCGPToolbarMenuButton* pItem1 = (CBCGPToolbarMenuButton*) otherMenuBtn.m_listCommands.GetNext (pos1);
		ASSERT_VALID (pItem1);

		if (!pItem->CompareWith (*pItem1))
		{
			return FALSE;
		}
	}

	return TRUE;
}
//********************************************************************************
void CBCGPToolbarMenuButton::DrawDocumentIcon (CDC* pDC, const CRect& rectImage, HICON hIcon)
{
	ASSERT_VALID (pDC);

	int cx = globalData.m_sizeSmallIcon.cx;
	int cy = globalData.m_sizeSmallIcon.cy;

	if (cx > rectImage.Width () ||
		cy > rectImage.Height ())
	{
		// Small icon is too large, stretch it
		cx = rectImage.Width ();
		cy = rectImage.Height ();
	}

	int x = max (0, (rectImage.Width () - cx) / 2);
	int y = max (0, (rectImage.Height () - cy) / 2);

	::DrawIconEx (pDC->GetSafeHdc (),
		rectImage.left + x, rectImage.top + y, hIcon,
		cx, cy, 0, NULL, DI_NORMAL);
}
//**************************************************************************************
BOOL CBCGPToolbarMenuButton::SetACCData (CWnd* pParent, CBCGPAccessibilityData& data)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pParent);

	if (!CBCGPToolbarButton::SetACCData (pParent, data))
	{
		return FALSE;
	}

	data.m_nAccRole = ROLE_SYSTEM_MENUITEM;
	data.m_bAccState = STATE_SYSTEM_FOCUSED |STATE_SYSTEM_FOCUSABLE;	
	if (m_nStyle & TBBS_CHECKED)
	{
		data.m_bAccState |= STATE_SYSTEM_CHECKED; 
	}

	if (m_nStyle & TBBS_DISABLED)
	{
		data.m_bAccState |= STATE_SYSTEM_UNAVAILABLE;
	}

	data.m_strAccHelp = L"BCGPToolbarMenuButton";
	data.m_strAccDefAction = m_bMenuMode ? L"Execute" : L"Open"; 

	return TRUE;
}
