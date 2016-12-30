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

// BCGPPopupMenuBar.cpp : implementation file
//

#include "stdafx.h"
#include <afxpriv.h>

#pragma warning (disable : 4201)
	#include "mmsystem.h"
#pragma warning (default : 4201)

#include "BCGPWorkspace.h"
#include "BCGPPopupMenuBar.h"
#include "BCGPToolbarButton.h"
#include "BCGPToolbarMenuButton.h"
#include "BCGPPopupMenu.h"
#include "BCGPCommandManager.h"
#include "BCGPTearOffManager.h"
#include "BCGGlobals.h"
#include "BCGPToolbarMenuButton.h"
#include "bcgprores.h"
#include "BCGPLocalResource.h"
#include "BCGPMenuBar.h"
#include "BCGPToolbarComboBoxButton.h"
#include "BCGPUserToolsManager.h"
#include "BCGPRegistry.h"
#include "BCGPKeyboardManager.h"
#include "BCGPSound.h"
#include "BCGPFrameImpl.h"
#include "MenuHash.h"
#include "BCGPVisualManager.h"
#include "BCGPDrawManager.h"
#include "BCGPContextMenuManager.h"
#include "BCGPShowAllButton.h"
#include "BCGPCustomizeMenuButton.h"
#include "CustomizeButton.h"
#include "BCGPTooltipManager.h"
#include "BCGPDropDownList.h"
#include "BCGPBaseRibbonElement.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static const int iVertMargin = 1;
static const int iHorzMargin = 1;
static const int iSeparatorHeight = 8;
static const int iMinTabSpace = 10;
static const int iEmptyMenuWidth = 50;
static const int iEmptyMenuHeight = 20;

static const int uiPopupTimerEvent = 1;
static const int uiRemovePopupTimerEvent = 2;


UINT CBCGPPopupMenuBar::m_uiPopupTimerDelay = (UINT) -1;
int	CBCGPPopupMenuBar::m_nLastCommandIndex = -1;

/////////////////////////////////////////////////////////////////////////////
// CBCGPPopupMenuBar

IMPLEMENT_SERIAL(CBCGPPopupMenuBar, CBCGPToolBar, 1)

CBCGPPopupMenuBar::CBCGPPopupMenuBar() :
	m_uiDefaultMenuCmdId (0),
	m_pDelayedPopupMenuButton (NULL),
	m_pDelayedClosePopupMenuButton (NULL),
	m_bFirstClick (TRUE),
	m_bFirstMove (TRUE),
	m_iOffset (0),
	m_xSeparatorOffsetLeft (0),
	m_xSeparatorOffsetRight (0),
	m_iMaxWidth (-1),
	m_iMinWidth (-1),
	m_bAreAllCommandsShown (TRUE),
	m_bInCommand (FALSE),
	m_bTrackMode (FALSE)
{
	m_bMenuMode = TRUE;
	m_bIsClickOutsideItem = TRUE;
	m_bEnableIDChecking = FALSE;
	m_bDisableSideBarInXPMode = FALSE;
	m_bPaletteMode = FALSE;
	m_bPaletteRows = 1;
	m_pRelatedToolbar = NULL;
	m_bDropDownListMode = FALSE;
	m_bInScrollMode = FALSE;
	m_bResizeTracking = FALSE;
	m_bGutterLogo = FALSE;
	m_nDropDownPageSize = 0;
	m_ptCursorInit = CPoint (-1, -1);
}

CBCGPPopupMenuBar::~CBCGPPopupMenuBar()
{
}


BEGIN_MESSAGE_MAP(CBCGPPopupMenuBar, CBCGPToolBar)
	//{{AFX_MSG_MAP(CBCGPPopupMenuBar)
	ON_WM_NCPAINT()
	ON_WM_NCCALCSIZE()
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_CREATE()
	ON_MESSAGE(WM_IDLEUPDATECMDUI, OnIdleUpdateCmdUI)
	ON_COMMAND(ID_BCGBARRES_TOOLBAR_IMAGE_AND_TEXT, OnToolbarImageAndText)
	ON_COMMAND(ID_BCGBARRES_TOOLBAR_TEXT, OnToolbarText)
	ON_WM_LBUTTONDBLCLK()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGPPopupMenuBar message handlers

BOOL CBCGPPopupMenuBar::OnSendCommand (const CBCGPToolbarButton* pButton)
{
	ASSERT_VALID (pButton);

	if (pButton->m_nID == nBCGPMenuGroupID)
	{
		return TRUE;
	}

	CBCGPCustomizeMenuButton* pCustomMenuButton = 
		DYNAMIC_DOWNCAST (CBCGPCustomizeMenuButton, pButton);
	
	if ((pCustomMenuButton != NULL) &&
		((pButton->m_nStyle & TBBS_DISABLED) != 0 ))
	{
		pCustomMenuButton->OnClickMenuItem ();

		return TRUE;
	}

	if ((pButton->m_nStyle & TBBS_DISABLED) != 0 ||
		pButton->m_nID < 0 || pButton->m_nID == (UINT)-1)
	{
		return FALSE;
	}

	CBCGPToolbarMenuButton* pMenuButton = 
		DYNAMIC_DOWNCAST (CBCGPToolbarMenuButton, pButton);

	if (pMenuButton != NULL && pMenuButton->HasButton ())
	{
		CPoint ptCursor;
		::GetCursorPos (&ptCursor);
		ScreenToClient (&ptCursor);

		if (pMenuButton->m_rectButton.PtInRect (ptCursor))
		{
			return TRUE;
		}

		if (pMenuButton->m_pPopupMenu != NULL)
		{
			pMenuButton->m_pPopupMenu->PostMessage (WM_CLOSE);
			return FALSE;
		}
	}

	if (pMenuButton != NULL && pMenuButton->m_pPopupMenu != NULL)
	{
		return FALSE;
	}

	if (pMenuButton != NULL && pMenuButton->OnClickMenuItem ())
	{
		return TRUE;
	}

	if (pMenuButton != NULL && pMenuButton->IsKindOf (RUNTIME_CLASS (CBCGPShowAllButton)))
	{
		pMenuButton->OnClick (this, FALSE);		
		return TRUE;
	}

	InvokeMenuCommand (pButton->m_nID, pButton);
	return TRUE;
}
//**************************************************************************************
void CBCGPPopupMenuBar::InvokeMenuCommand (UINT uiCmdId, const CBCGPToolbarButton* pMenuItem)
{
	ASSERT (uiCmdId != (UINT) -1);

	CBCGPPopupMenu* pParentMenu = DYNAMIC_DOWNCAST (CBCGPPopupMenu, GetParent ());

	if (pParentMenu != NULL && pParentMenu->GetMessageWnd () != NULL) 
	{
		pParentMenu->GetMessageWnd()->SendMessage 
			(WM_SETMESSAGESTRING, AFX_IDS_IDLEMESSAGE);
	} 
	else 
	{
		GetOwner()->SendMessage(WM_SETMESSAGESTRING, AFX_IDS_IDLEMESSAGE);
	}

	//--------------------
	// Deactivate menubar:
	//--------------------
	if (pParentMenu != NULL)
	{
		CBCGPToolBar* pToolBar = NULL;
		for (CBCGPPopupMenu* pMenu = pParentMenu; pMenu != NULL;
			pMenu = pMenu->GetParentPopupMenu ())
		{
			CBCGPToolbarMenuButton* pParentButton = pMenu->GetParentButton ();
			if (pParentButton == NULL)
			{
				break;
			}
		
			pToolBar = 
				DYNAMIC_DOWNCAST (CBCGPToolBar, pParentButton->GetParentWnd ());
		}

		if (pToolBar != NULL)
		{
			pToolBar->Deactivate ();
		}
	}

	if (uiCmdId != 0)
	{
		SetInCommand ();

		BCGPlaySystemSound (BCGSOUND_MENU_COMMAND);

		if (m_bDropDownListMode)
		{
			if (pParentMenu != NULL)
			{
				pParentMenu->OnChooseItem (uiCmdId);
			}
		}
		else if (!m_bTrackMode)
		{
			BOOL bDone = FALSE;

			CBCGPPopupMenu* pParentMenu = DYNAMIC_DOWNCAST (CBCGPPopupMenu, GetParent ());
			if (pParentMenu != NULL)
			{
				ASSERT_VALID (pParentMenu);
				
				CCustomizeButton* pCustomizeButton = DYNAMIC_DOWNCAST (
					CCustomizeButton, pParentMenu->GetParentButton ());
				if (pCustomizeButton != NULL)
				{
					bDone = pCustomizeButton->InvokeCommand (this, pMenuItem);
				}
			}

			if (!bDone)
			{
				//----------------------------------
				// Send command to the parent frame:
				//----------------------------------
				AddCommandUsage (uiCmdId);

				if (!pParentMenu->PostCommand (uiCmdId) &&
					(g_pUserToolsManager == NULL ||
					!g_pUserToolsManager->InvokeTool (uiCmdId)))
				{
					BOOL bIsSysCommand = (uiCmdId >= 0xF000 && uiCmdId < 0xF1F0);
					GetOwner()->PostMessage (bIsSysCommand ? WM_SYSCOMMAND : WM_COMMAND, uiCmdId);

					if (pParentMenu != NULL)
					{
						ASSERT_VALID (pParentMenu);

#ifndef BCGP_EXCLUDE_RIBBON
						if (pParentMenu->m_pParentRibbonElement != NULL)
						{
							CBCGPBaseRibbonElement* pElement = pParentMenu->m_pParentRibbonElement;
							ASSERT_VALID (pElement);

							pParentMenu->m_pParentRibbonElement->SetDroppedDown (NULL);

							pParentMenu->m_pParentRibbonElement = NULL;
							pElement->PostMenuCommand (uiCmdId);
						}
#endif
					}
				}
			}
		}
		else
		{
			if (g_pContextMenuManager == NULL)
			{
				ASSERT (FALSE);
			}
			else
			{
				g_pContextMenuManager->m_nLastCommandID = uiCmdId;
			}
		}
	}

	m_nLastCommandIndex = pMenuItem == NULL ? -1 : ButtonToIndex (pMenuItem);

	if (m_bPaletteMode)
	{
		CBCGPPopupMenu* pParentMenu = DYNAMIC_DOWNCAST (CBCGPPopupMenu, GetParent ());
		if (pParentMenu != NULL)
		{
			ASSERT_VALID (pParentMenu);

			CBCGPToolbarMenuButton* pParentButton = pParentMenu->GetParentButton ();
			if (pParentButton != NULL && pParentButton->GetParentWnd () != NULL)
			{
				ASSERT_VALID (pParentButton);
				pParentButton->m_nID = uiCmdId;
				pParentButton->SetImage (CImageHash::GetImageOfCommand (uiCmdId));

				CRect rectImage;
				pParentButton->GetImageRect (rectImage);

				pParentButton->GetParentWnd ()->InvalidateRect (rectImage);
				pParentButton->GetParentWnd ()->UpdateWindow ();
			}
		}
	}

	CFrameWnd* pParentFrame = BCGPGetParentFrame (this);
	ASSERT_VALID (pParentFrame);

	SetInCommand (FALSE);
	pParentFrame->DestroyWindow ();
}
//***************************************************************
void CBCGPPopupMenuBar::AdjustLocations ()
{
	if (GetSafeHwnd () == NULL || !::IsWindow (m_hWnd) || m_bInUpdateShadow)
	{
		return;
	}

	if (m_bPaletteMode)
	{
		CBCGPToolBar::AdjustLocations ();
		UpdateTooltips ();
		return;
	}

	ASSERT_VALID(this);

	if (m_xSeparatorOffsetLeft == 0)
	{
		//-----------------------------------------------------------
		// To enable MS Office 2000 look, we'll draw the separators
		// bellow the menu text only (in the previous versions
		// separator has been drawn on the whole menu row). Ask
		// menu button about text area offsets:
		//-----------------------------------------------------------
		CBCGPToolbarMenuButton::GetTextHorzOffsets (
			m_xSeparatorOffsetLeft,
			m_xSeparatorOffsetRight);
	}

	CRect rectClient;	// Client area rectangle
	GetClientRect (&rectClient);

	CClientDC dc (this);
	CFont* pOldFont = (CFont*) dc.SelectObject (&globalData.fontRegular);
	ASSERT (pOldFont != NULL);

	int y = rectClient.top + iVertMargin - m_iOffset * GetRowHeight ();

	/// support for the menu with breaks:
	int origy = y;
	int x = rectClient.left;
	int right = (m_arColumns.GetSize() == 0 ||
		CBCGPToolBar::IsCustomizeMode ()) ?	
			rectClient.Width() :
			m_arColumns [0];
	int nColumn = 0;
	/////////

	CSize sizeMenuButton = GetMenuImageSize ();
	sizeMenuButton += CSize (2 * iHorzMargin, 2 * iVertMargin);

	sizeMenuButton.cy = max (sizeMenuButton.cy, 
							globalData.GetTextHeight ());

	for (POSITION pos = m_Buttons.GetHeadPosition (); pos != NULL;)
	{
		CBCGPToolbarButton* pButton = (CBCGPToolbarButton*) m_Buttons.GetNext (pos);
		ASSERT (pButton != NULL);

		/// support for the menu with breaks:
		if ((pButton->m_nStyle & TBBS_BREAK) && (y != origy) &&
			!CBCGPToolBar::IsCustomizeMode ())
		{
			y = origy;
			nColumn ++;
			x = right + iHorzMargin;
			right = m_arColumns [nColumn];
		}
		////////////////////
		
		CRect rectButton;
		rectButton.top = y;

		if (pButton->m_nStyle & TBBS_SEPARATOR)
		{
			rectButton.left = x + m_xSeparatorOffsetLeft;
			rectButton.right = right + rectClient.left - m_xSeparatorOffsetRight;
			rectButton.bottom = rectButton.top + iSeparatorHeight;
		}
		else
		{
			CSize sizeButton = pButton->OnCalculateSize (&dc, 
									sizeMenuButton, TRUE);

			rectButton.left = x;
			rectButton.right = right + rectClient.left;
			rectButton.bottom = rectButton.top + sizeButton.cy;
		}

		pButton->SetRect (rectButton);
		y += rectButton.Height ();
	}

	dc.SelectObject (pOldFont);

	//--------------------------------------------------
	// Something may changed, rebuild acceleration keys:
	//--------------------------------------------------
	RebuildAccelerationKeys ();
	
	CPoint ptCursor;
	::GetCursorPos (&ptCursor);
	ScreenToClient (&ptCursor);

	if (HitTest (ptCursor) >= 0)
	{
		m_bIsClickOutsideItem = FALSE;
	}

	UpdateTooltips ();
}
//***************************************************************************************
void CBCGPPopupMenuBar::DrawSeparator (CDC* pDC, const CRect& rect, BOOL /*bHorz*/)
{
	CBCGPVisualManager::GetInstance ()->OnDrawSeparator (pDC, this, rect, FALSE);
}
//***************************************************************************************
CSize CBCGPPopupMenuBar::GetEmptyMenuSize() const
{
	return CSize(iEmptyMenuWidth, iEmptyMenuHeight);
}
//***************************************************************************************
CSize CBCGPPopupMenuBar::CalcSize (BOOL /*bVertDock*/)
{
	if (m_bPaletteMode)
	{
		return CBCGPToolBar::CalcSize (FALSE);
	}

	CSize size (0, 0);

	CClientDC dc (this);
	CFont* pOldFont = (CFont*) dc.SelectObject (&globalData.fontRegular);
	ASSERT (pOldFont != NULL);

	if (m_Buttons.IsEmpty ())
	{
		size = GetEmptyMenuSize();
	}
	else
	{
		//support for the menu with breaks:
		CSize column (0, 0);
		m_arColumns.RemoveAll ();
		//////////////////////////

		CSize sizeMenuButton = GetMenuImageSize ();
		sizeMenuButton += CSize (2 * iHorzMargin, 2 * iVertMargin);

		sizeMenuButton.cy = max (sizeMenuButton.cy,
								globalData.GetTextHeight ());

		for (POSITION pos = m_Buttons.GetHeadPosition (); pos != NULL;)
		{
			CBCGPToolbarButton* pButton = (CBCGPToolbarButton*) m_Buttons.GetNext (pos);
			ASSERT (pButton != NULL);

			BOOL bRestoreFont = FALSE;

			if (m_uiDefaultMenuCmdId != 0 &&
				pButton->m_nID == m_uiDefaultMenuCmdId)
			{
				dc.SelectObject (&globalData.fontBold);
				bRestoreFont = TRUE;
			}

			CSize sizeButton = pButton->OnCalculateSize (&dc, 
				sizeMenuButton, TRUE);

			// support for the menu with breaks:
			if ((pButton->m_nStyle & TBBS_BREAK) &&
				!CBCGPToolBar::IsCustomizeMode ())
			{
				if ((column.cx != 0) && (column.cy != 0))
				{
					size.cy = max (column.cy, size.cy);
					size.cx += column.cx + iHorzMargin;
					m_arColumns.Add (size.cx);
				}
				column.cx = column.cy = 0;
			}
			///////////////////////////////

			int iHeight = sizeButton.cy;

			if (pButton->m_nStyle & TBBS_SEPARATOR)
			{
				iHeight = iSeparatorHeight;
			}
			else
			{
				if (pButton->IsDrawText () &&
					pButton->m_strText.Find (_T('\t')) > 0)
				{
					sizeButton.cx += iMinTabSpace;
				}

				pButton->m_bWholeText = 
					(m_iMaxWidth <= 0 || 
					sizeButton.cx <= m_iMaxWidth - 2 * iHorzMargin);

				column.cx = max (sizeButton.cx, column.cx);
			}

			column.cy += iHeight;

			if (bRestoreFont)
			{
				dc.SelectObject (&globalData.fontRegular);
			}
		}

		size.cy = max (column.cy, size.cy);
		size.cx += column.cx;
	}

	size.cy += 2 * iVertMargin;
	size.cx += 2 * iHorzMargin;

	if (m_iMaxWidth > 0 && size.cx > m_iMaxWidth)
	{
		size.cx = m_iMaxWidth;
	}

	if (m_iMinWidth > 0 && size.cx < m_iMinWidth)
	{
		size.cx = m_iMinWidth;
	}

	m_arColumns.Add (size.cx);

	dc.SelectObject (pOldFont);
	return size;
}
//***************************************************************************************
void CBCGPPopupMenuBar::OnNcPaint() 
{
	//--------------------------------------
	// Disable gripper and borders painting!
	//--------------------------------------
}
//***************************************************************************************
void CBCGPPopupMenuBar::OnNcCalcSize(BOOL /*bCalcValidRects*/, NCCALCSIZE_PARAMS FAR* /*lpncsp*/) 
{
	//-----------------------------------------------
	// Don't leave space for the gripper and borders!
	//-----------------------------------------------
}
//****************************************************************************************
void CBCGPPopupMenuBar::DrawDragMarker (CDC* pDC)
{
	if (m_bPaletteMode)
	{
		return;
	}

	CPen* pOldPen = (CPen*) pDC->SelectObject (&m_penDrag);

	for (int i = 0; i < 2; i ++)
	{
		pDC->MoveTo (m_rectDrag.left, m_rectDrag.top + m_rectDrag.Height () / 2 + i - 1);
		pDC->LineTo (m_rectDrag.right, m_rectDrag.top + m_rectDrag.Height () / 2 + i - 1);

		pDC->MoveTo (m_rectDrag.left + i, m_rectDrag.top + i);
		pDC->LineTo (m_rectDrag.left + i, m_rectDrag.bottom - i);

		pDC->MoveTo (m_rectDrag.right - i - 1, m_rectDrag.top + i);
		pDC->LineTo (m_rectDrag.right - i - 1, m_rectDrag.bottom - i);
	}

	pDC->SelectObject (pOldPen);
}
//********************************************************************************
int CBCGPPopupMenuBar::FindDropIndex (const CPoint p, CRect& rectDrag) const
{
	if (m_bPaletteMode)
	{
		return -1;
	}

	const int iCursorSize = 6;

	GetClientRect (&rectDrag);

	if (m_Buttons.IsEmpty ())
	{
		rectDrag.bottom = rectDrag.top + iCursorSize;
		return 0;
	}

	CPoint point = p;
	if (point.y < 0)
	{
		point.y = 0;
	}

	int iDragButton = -1;
	int iIndex = 0;
	for (POSITION pos = m_Buttons.GetHeadPosition (); pos != NULL; iIndex ++)
	{
		CBCGPToolbarButton* pButton = (CBCGPToolbarButton*) m_Buttons.GetNext (pos);
		ASSERT (pButton != NULL);

		CRect rect = pButton->Rect ();
		if (point.y < rect.top)
		{
			iDragButton = iIndex;
			rectDrag.top = rect.top;
			break;
		}
		else if (point.y <= rect.bottom)
		{
			rectDrag = rect;
			if (point.y - rect.top > rect.bottom - point.y)
			{
				iDragButton = iIndex + 1;
				rectDrag.top = rectDrag.bottom;
			}
			else
			{
				iDragButton = iIndex;
				rectDrag.top = rect.top;
			}
			break;
		}
	}

	if (iDragButton == -1)
	{
		rectDrag.top = rectDrag.bottom - iCursorSize;
		iDragButton = iIndex;
	}

	rectDrag.bottom = rectDrag.top + iCursorSize;
	rectDrag.OffsetRect (0, -iCursorSize / 2);

	return iDragButton;
}
//***************************************************************************************
CBCGPToolbarButton* CBCGPPopupMenuBar::CreateDroppedButton (COleDataObject* pDataObject)
{
	CBCGPToolbarButton* pButton = CBCGPToolbarButton::CreateFromOleData (pDataObject);
	ASSERT (pButton != NULL);

	CBCGPToolbarMenuButton* pMenuButton = 
		DYNAMIC_DOWNCAST (CBCGPToolbarMenuButton, pButton);

	if (pMenuButton == NULL)
	{
		pMenuButton = new CBCGPToolbarMenuButton (
			pButton->m_nID, NULL, 
				pButton->IsLocked () ? -1 : pButton->GetImage (), 
				pButton->m_strText,
			pButton->m_bUserButton);
		ASSERT (pMenuButton != NULL);

		pMenuButton->m_bText = TRUE;
		pMenuButton->m_bImage = !pButton->IsLocked ();

		BOOL bRes = pButton->ExportToMenuButton (*pMenuButton);
		delete pButton;
		
		if (!bRes || pMenuButton->m_strText.IsEmpty ())
		{
			delete pMenuButton;
			return NULL;
		}
	}

	return pMenuButton;
}
//****************************************************************************************
BOOL CBCGPPopupMenuBar::ImportFromMenu (HMENU hMenu, BOOL bShowAllCommands)
{
	RemoveAllButtons ();
	m_bAreAllCommandsShown = TRUE;
	m_HiddenItemsAccel.RemoveAll ();

	if (hMenu == NULL)
	{
		return FALSE;
	}

	CMenu* pMenu = CMenu::FromHandle (hMenu);
	if (pMenu == NULL)
	{
		return FALSE;
	}

	//***** MSI START *****
	// We need to update the menu items first (OnUpdate*** for the target message
	// window need to be invoked:
	CWnd* pMsgWindow = BCGCBProGetTopLevelFrame (this);

	if (pMsgWindow == NULL)
	{
		pMsgWindow = AfxGetMainWnd ();
	}

	if (GetSafeHwnd () != NULL)
	{
		CBCGPPopupMenu* pParentMenu = DYNAMIC_DOWNCAST (CBCGPPopupMenu, GetParent ());
		if (pParentMenu != NULL && pParentMenu->GetMessageWnd () != NULL) 
		{
			pMsgWindow = pParentMenu->GetMessageWnd ();
		}

		if (m_hookMouseHelp != NULL && 
			pParentMenu != NULL &&
			pParentMenu->GetParentPopupMenu () != NULL)
		{
			bShowAllCommands = TRUE;
		}
	}

    if (pMsgWindow != NULL)
	{
		WPARAM theMenu = WPARAM(hMenu);
		LPARAM theItem = MAKELPARAM(m_iOffset, 0);
		pMsgWindow->SendMessage(WM_INITMENUPOPUP, theMenu, theItem);
	}
	//***** MSI END ******

	int iCount = (int) pMenu->GetMenuItemCount ();
	BOOL bPrevWasSeparator = FALSE;
	BOOL bFirstItem = TRUE;

	int nPaletteColumns = 1;
	if (m_bPaletteMode)
	{
		nPaletteColumns = max (1, (int) (.5 + (double) iCount / m_bPaletteRows));
	}

	for (int i = 0; i < iCount; i ++)
	{
		UINT uiTearOffId = 0;

		HMENU hSubMenu = NULL;

		CString strText;
		pMenu->GetMenuString (i, strText, MF_BYPOSITION);

        MENUITEMINFO mii;
		ZeroMemory(&mii, sizeof(MENUITEMINFO));

        mii.cbSize = sizeof(mii);
        mii.cch = 0;
		mii.dwTypeData = 0;
        mii.fMask = MIIM_TYPE | MIIM_SUBMENU | MIIM_ID | MIIM_STATE | MIIM_DATA;
        pMenu->GetMenuItemInfo(i, &mii, TRUE);

        UINT uiCmd = mii.wID; 
		UINT uiState = pMenu->GetMenuState (i, MF_BYPOSITION);
		DWORD dwMenuItemData = (DWORD) mii.dwItemData;

        if (mii.fType == MFT_SEPARATOR)
        {
			if (!bPrevWasSeparator && !bFirstItem && i != iCount - 1 &&
				!m_bPaletteMode)
			{
				InsertSeparator ();
				bFirstItem = FALSE;
				bPrevWasSeparator = TRUE;
			}
        }
        else
        {
            if (mii.hSubMenu != NULL)
            {
                uiCmd = (UINT)-1;  // force value (needed due to Windows bug)
    			hSubMenu = mii.hSubMenu;
    			ASSERT (hSubMenu != NULL);

    			if (g_pBCGPTearOffMenuManager != NULL)
    			{
    				uiTearOffId = g_pBCGPTearOffMenuManager->Parse (strText);
    			}
            }

			if (m_bTrackMode || bShowAllCommands ||
				CBCGPMenuBar::IsShowAllCommands () ||
				!CBCGPToolBar::IsCommandRarelyUsed (uiCmd) ||
				m_bPaletteMode)
			{
				int iIndex = -1;

				if (m_bPaletteMode)
				{
					CBCGPToolbarButton item (uiCmd, 
						BCGPCMD_MGR.GetCmdImage (uiCmd, FALSE),
						strText);

					if (i > 0 && ((i + 1) % nPaletteColumns) == 0)
					{
						item.m_bWrap = TRUE;
					}

					iIndex = InsertButton (item);
				}
				else
				{
					CBCGPToolbarMenuButton item (uiCmd, hSubMenu,
												-1, strText);
					item.m_bText = TRUE;
					item.m_bImage = FALSE;
					item.m_iUserImage = BCGPCMD_MGR.GetMenuUserImage (uiCmd);

					if (item.m_iUserImage != -1)
					{
						item.m_bUserButton = TRUE;
					}

					iIndex = InsertButton (item);
				}

				if (iIndex >= 0)
				{
					CBCGPToolbarButton* pButton = GetButton (iIndex);
					ASSERT (pButton != NULL);

					pButton->m_bImage = (pButton->GetImage () >= 0);
					pButton->m_dwdItemData = dwMenuItemData;

					if (g_pUserToolsManager == NULL ||
						!g_pUserToolsManager->IsUserToolCmd (uiCmd))
					{
						if ((uiState & MF_DISABLED) || (uiState & MF_GRAYED))
						{
							pButton->m_nStyle |= TBBS_DISABLED;
						}
					}

					CBCGPToolbarMenuButton* pMenuButton = 
						DYNAMIC_DOWNCAST (CBCGPToolbarMenuButton, pButton);
					if (pMenuButton != NULL)
					{
						pMenuButton->SetTearOff (uiTearOffId);
					}

					if (uiState & MF_CHECKED)
					{
						if ((uiState & MFT_RADIOCHECK) == MFT_RADIOCHECK && pMenuButton != NULL)
						{
							pMenuButton->m_bIsRadio = TRUE;
						}

						pButton->m_nStyle |= TBBS_CHECKED;
					}

					//support for the menu with breaks:
					if (mii.fType & MF_MENUBREAK)
					{
						pButton->m_nStyle |= TBBS_BREAK;
					}
					///////////////
				}

				bPrevWasSeparator = FALSE;
				bFirstItem = FALSE;
			}
			else if (CBCGPToolBar::IsCommandRarelyUsed (uiCmd) &&
				CBCGPToolBar::IsCommandPermitted (uiCmd))
			{
				m_bAreAllCommandsShown = FALSE;
				
				int iAmpOffset = strText.Find (_T('&'));
				if (iAmpOffset >= 0 && iAmpOffset < strText.GetLength () - 1)
				{
					TCHAR szChar[2] = {strText.GetAt (iAmpOffset + 1), '\0'};
					CharUpper (szChar);

					UINT uiHotKey = (UINT) (szChar [0]);
					m_HiddenItemsAccel.SetAt (uiHotKey, uiCmd);
				}
			}
		}
	}

	m_uiDefaultMenuCmdId = ::GetMenuDefaultItem (hMenu, FALSE, GMDI_USEDISABLED);
	return TRUE;
}
//****************************************************************************************
HMENU CBCGPPopupMenuBar::ExportToMenu () const
{
	CMenu menu;
	menu.CreatePopupMenu ();

	for (POSITION pos = m_Buttons.GetHeadPosition (); pos != NULL;)
	{
		CBCGPToolbarButton* pButton = (CBCGPToolbarButton*) m_Buttons.GetNext (pos);
		ASSERT (pButton != NULL);

		if (pButton->m_nStyle & TBBS_SEPARATOR)
		{
			menu.AppendMenu (MF_SEPARATOR);
			continue;
		}

		if (!pButton->IsKindOf (RUNTIME_CLASS (CBCGPToolbarMenuButton)))
		{
			continue;
		}

		CBCGPToolbarMenuButton* pMenuButton = (CBCGPToolbarMenuButton*) pButton;

		HMENU hPopupMenu = pMenuButton->CreateMenu ();
		if (hPopupMenu != NULL)
		{
			UINT uiStyle = (MF_STRING | MF_POPUP);
			
			//support for the menu with breaks:
			if (pButton->m_nStyle & TBBS_BREAK)
			{
				uiStyle |= MF_MENUBREAK;
			}
			//////////////////////

			CString strText = pMenuButton->m_strText;
			if (pMenuButton->m_uiTearOffBarID != 0 && g_pBCGPTearOffMenuManager != NULL)
			{
				g_pBCGPTearOffMenuManager->Build (pMenuButton->m_uiTearOffBarID, strText);
			}

			menu.AppendMenu (uiStyle, (UINT_PTR) hPopupMenu, strText);

			//--------------------------------------------------------
			// This is incompatibility between Windows 95 and 
			// NT API! (IMHO). CMenu::AppendMenu with MF_POPUP flag 
			// COPIES sub-menu resource under the Win NT and 
			// MOVES sub-menu under Win 95/98 and 2000!
			//--------------------------------------------------------
			if (globalData.bIsWindowsNT4)
			{
				::DestroyMenu (hPopupMenu);
			}
		}
		else
		{
			menu.AppendMenu (MF_STRING, pMenuButton->m_nID, pMenuButton->m_strText);
		}
	}

	HMENU hMenu = menu.Detach ();

	::SetMenuDefaultItem (hMenu, m_uiDefaultMenuCmdId, FALSE);
	return hMenu;
}
//***************************************************************************************
void CBCGPPopupMenuBar::OnChangeHot (int iHot)
{
	ASSERT_VALID (this);
	ASSERT (::IsWindow (GetSafeHwnd ()));

	if (iHot == -1)
	{
		CPoint ptCursor;
		::GetCursorPos (&ptCursor);
		ScreenToClient (&ptCursor);

		if (HitTest (ptCursor) == m_iHot)
		{
			m_iHighlighted = m_iHot;
			return;
		}
	}

	CBCGPToolbarMenuButton* pCurrPopupMenu = NULL;

	for (POSITION pos = m_Buttons.GetHeadPosition (); pos != NULL;)
	{
		CBCGPToolbarButton* pButton = (CBCGPToolbarButton*) m_Buttons.GetNext (pos);
		ASSERT_VALID (pButton);

		CBCGPToolbarMenuButton* pMenuButton =
			DYNAMIC_DOWNCAST (CBCGPToolbarMenuButton, pButton);

		if (pMenuButton != NULL && pMenuButton->IsDroppedDown ())
		{
			pCurrPopupMenu = pMenuButton;
			break;
		}
	}

	CBCGPToolbarMenuButton* pMenuButton = NULL;
	if (iHot >= 0)
	{
		CBCGPToolbarButton* pButton = GetButton (iHot);
		if (pButton == NULL)
		{
			ASSERT (FALSE);
			return;
		}
		else
		{
			ASSERT_VALID (pButton);
			pMenuButton = DYNAMIC_DOWNCAST (CBCGPToolbarMenuButton, pButton);
		}
	}

	if (pMenuButton != pCurrPopupMenu)
	{
		CBCGPPopupMenu* pParentMenu = DYNAMIC_DOWNCAST (CBCGPPopupMenu, GetParent ());

		if (pCurrPopupMenu != NULL)
		{
			const MSG* pMsg = GetCurrentMessage ();

			if (CBCGPToolBar::IsCustomizeMode () ||
				(pMsg != NULL && pMsg->message == WM_KEYDOWN))
			{
				KillTimer (uiRemovePopupTimerEvent);
				m_pDelayedClosePopupMenuButton = NULL;

				pCurrPopupMenu->OnCancelMode ();

				if (pParentMenu != NULL)
				{
					CBCGPPopupMenu::ActivatePopupMenu (
						BCGCBProGetTopLevelFrame (this), pParentMenu);
				}
			}
			else
			{
				m_pDelayedClosePopupMenuButton = pCurrPopupMenu;
				m_pDelayedClosePopupMenuButton->m_bToBeClosed = TRUE;

				SetTimer (uiRemovePopupTimerEvent, 
						max (0, (int) m_uiPopupTimerDelay - 1), NULL);

				InvalidateRect (pCurrPopupMenu->Rect ());
				UpdateWindow ();
			}
		}

		if (pMenuButton != NULL && 
			(pMenuButton->m_nID == (UINT) -1 || pMenuButton->m_bDrawDownArrow))
		{
			pMenuButton->OnClick (this);
		}

		// Maybe, this menu will be closed by the parent menu bar timer proc.?
		CBCGPPopupMenuBar* pParentBar = NULL;

		if (pParentMenu != NULL && pParentMenu->GetParentPopupMenu () != NULL &&
			(pParentBar = pParentMenu->GetParentPopupMenu ()->GetMenuBar ()) != NULL &&
			pParentBar->m_pDelayedClosePopupMenuButton == pParentMenu->GetParentButton ())
		{
			pParentBar->RestoreDelayedSubMenu ();
		}
	}
	else if (pMenuButton != NULL &&
		pMenuButton == m_pDelayedClosePopupMenuButton)
	{
		m_pDelayedClosePopupMenuButton->m_bToBeClosed = FALSE;
		m_pDelayedClosePopupMenuButton = NULL;

		KillTimer (uiRemovePopupTimerEvent);
	}

	m_iHot = iHot;

	if (m_bDropDownListMode)
	{
		CBCGPPopupMenu* pParentMenu = DYNAMIC_DOWNCAST (CBCGPPopupMenu, GetParent ());
		if (pParentMenu != NULL)
		{
			pParentMenu->OnChangeHot (m_iHot);
		}
	}

	if (CBCGPPopupMenu::IsSendMenuSelectMsg ())
	{
		CWnd* pMsgWindow = BCGCBProGetTopLevelFrame (this);
		if (pMsgWindow == NULL)
		{
			pMsgWindow = AfxGetMainWnd ();
		}

		CBCGPPopupMenu* pParentMenu = DYNAMIC_DOWNCAST (CBCGPPopupMenu, GetParent ());
		if (pParentMenu != NULL && pParentMenu->GetMessageWnd () != NULL) 
		{
			pMsgWindow = pParentMenu->GetMessageWnd ();
		}

		if (pMsgWindow != NULL && pParentMenu != NULL)
		{
			UINT nFlags = MF_HILITE;
			UINT nItem = 0;

			if (pMenuButton != NULL)
			{
				if ((pMenuButton->m_nStyle & TBBS_DISABLED) != 0)
				{
					nFlags |= MF_DISABLED;
				}

				if ((pMenuButton->m_nStyle & TBBS_CHECKED) != 0)
				{
					nFlags |= MF_CHECKED;
				}

				if ((nItem = pMenuButton->m_nID) == (UINT)-1)
				{
					nItem = iHot;
					nFlags |= MF_POPUP;
				}
			}

			pMsgWindow->SendMessage (WM_MENUSELECT,
				MAKEWPARAM (nItem, nFlags),
				(WPARAM) pParentMenu->GetHMenu ());
		}
	}
}
//****************************************************************************************
void CBCGPPopupMenuBar::OnDestroy() 
{
	KillTimer (uiPopupTimerEvent);
	KillTimer (uiRemovePopupTimerEvent);

	m_pDelayedPopupMenuButton = NULL;
	m_pDelayedClosePopupMenuButton = NULL;

	for (POSITION pos = m_Buttons.GetHeadPosition (); pos != NULL;)
	{
		CBCGPToolbarButton* pButton = (CBCGPToolbarButton*) m_Buttons.GetNext (pos);
		ASSERT_VALID (pButton);

		CBCGPToolbarMenuButton* pMenuButton =
			DYNAMIC_DOWNCAST (CBCGPToolbarMenuButton, pButton);

		if (pMenuButton != NULL && pMenuButton->IsDroppedDown ())
		{
			CBCGPPopupMenu* pMenu = pMenuButton->m_pPopupMenu;
			if (pMenu != NULL && ::IsWindow (pMenu->m_hWnd))
			{
				pMenu->SaveState ();
				pMenu->PostMessage (WM_CLOSE);
			}
		}
	}

	CBCGPToolBar::OnDestroy();
}
//****************************************************************************************
BOOL CBCGPPopupMenuBar::OnKey(UINT nChar)
{
	BOOL bProcessed = FALSE;

	POSITION posSel = 
		(m_iHighlighted < 0) ? NULL : m_Buttons.FindIndex (m_iHighlighted);
	CBCGPToolbarButton* pOldSelButton = 
		(posSel == NULL) ? NULL : (CBCGPToolbarButton*) m_Buttons.GetAt (posSel);
	CBCGPToolbarButton* pNewSelButton = pOldSelButton;
	int iNewHighlight = m_iHighlighted;
	
	BOOL bSendEvent = FALSE;

	if (nChar == VK_TAB)
	{
		if (::GetKeyState(VK_SHIFT) & 0x80)
		{
			nChar = VK_UP;
		}
		else
		{
			nChar = VK_DOWN;
		}
	}

	const POSITION posSelSaved = posSel;

	switch (nChar)
	{
	case VK_RETURN:
		{
			bProcessed = TRUE;

			// Try to cascase a popup menu and, if failed 
			CBCGPToolbarMenuButton* pMenuButton = DYNAMIC_DOWNCAST (CBCGPToolbarMenuButton,
							pOldSelButton);
			if (pMenuButton != NULL &&
				(pMenuButton->HasButton () || !pMenuButton->OpenPopupMenu ()))
			{
				GetOwner()->SendMessage(WM_SETMESSAGESTRING, AFX_IDS_IDLEMESSAGE);
				OnSendCommand (pMenuButton);
			}
		}
		break;

	case VK_HOME:
		posSel = NULL;
		// Like "Before first"...

	case VK_DOWN:
		//-----------------------------
		// Find next "selecteble" item:
		//-----------------------------
		{
			if (m_bDropDownListMode && posSelSaved == m_Buttons.GetTailPosition () &&
				nChar != VK_HOME)
			{
				return TRUE;
			}

			bProcessed = TRUE;
			if (m_Buttons.IsEmpty ())
			{
				break;
			}

			POSITION pos = posSel;
			if (pos != NULL)
			{
				m_Buttons.GetNext (pos);
			}

			if (pos == NULL)
			{
				pos = m_Buttons.GetHeadPosition ();
				iNewHighlight = 0;
			}
			else
			{
				iNewHighlight ++;
			}

			POSITION posFound = NULL;
			while (pos != posSel)
			{
				posFound = pos;

				CBCGPToolbarButton* pButton = (CBCGPToolbarButton*) m_Buttons.GetNext (pos);
				ASSERT_VALID (pButton);

				if ((pButton->m_nStyle & TBBS_SEPARATOR) == 0 &&
					!pButton->Rect ().IsRectEmpty () &&
					pButton->m_nID != nBCGPMenuGroupID)
				{
					break;
				}

				iNewHighlight ++;
				if (pos == NULL)
				{
					if (m_bDropDownListMode)
					{
						return TRUE;
					}

					pos = m_Buttons.GetHeadPosition ();
					iNewHighlight = 0;
				}
			}

			if (posFound != NULL)
			{
				pNewSelButton = (CBCGPToolbarButton*) m_Buttons.GetAt (posFound);
				bSendEvent = TRUE;
			}
		}
		break;

	case VK_END:
		posSel = NULL;
		// Like "After last"....

	case VK_UP:
		//---------------------------------
		// Find previous "selecteble" item:
		//---------------------------------
		{
			if (m_bDropDownListMode && posSelSaved == m_Buttons.GetHeadPosition () &&
				nChar != VK_END)
			{
				return TRUE;
			}

			bProcessed = TRUE;
			if (m_Buttons.IsEmpty ())
			{
				break;
			}

			POSITION pos = posSel;
			if (pos != NULL)
			{
				m_Buttons.GetPrev (pos);
			}
			if (pos == NULL)
			{
				pos = m_Buttons.GetTailPosition ();
				iNewHighlight = (int) m_Buttons.GetCount () - 1;
			}
			else
			{
				iNewHighlight --;
			}

			POSITION posFound = NULL;
			while (pos != posSel)
			{
				posFound = pos;

				CBCGPToolbarButton* pButton = (CBCGPToolbarButton*) m_Buttons.GetPrev (pos);
				ASSERT_VALID (pButton);

				if ((pButton->m_nStyle & TBBS_SEPARATOR) == 0 &&
					!pButton->Rect ().IsRectEmpty () &&
					pButton->m_nID != nBCGPMenuGroupID)
				{
					break;
				}

				iNewHighlight --;
				if (pos == NULL)
				{
					if (m_bDropDownListMode)
					{
						return TRUE;
					}

					pos = m_Buttons.GetTailPosition ();
					iNewHighlight = (int) m_Buttons.GetCount () - 1;
				}
			}

			if (posFound != NULL)
			{
				pNewSelButton = (CBCGPToolbarButton*) m_Buttons.GetAt (posFound);
				bSendEvent = TRUE;
			}
		}
		break;

	case VK_PRIOR:
	case VK_NEXT:
		if (m_bDropDownListMode && m_nDropDownPageSize > 0)
		{
			m_bInScrollMode = TRUE;
			int iHighlightedPrev = m_iHighlighted;

			for (int i = 0; i < m_nDropDownPageSize; i++)
			{
				OnKey (nChar == VK_PRIOR ? VK_UP : VK_DOWN);
			}

			m_bInScrollMode = FALSE;

		    if (iHighlightedPrev != m_iHighlighted)
			{
				AccNotifyObjectFocusEvent (m_iHighlighted);
			}

			return TRUE;
		}
		break;

	default:
		// Process acceleration key:
		if (!IsCustomizeMode () &&
			(::GetAsyncKeyState (VK_CONTROL) & 0x8000) == 0)
		{
			BOOL bKeyIsPrintable = CBCGPKeyboardManager::IsKeyPrintable (nChar);

			UINT nUpperChar = nChar;
			if (bKeyIsPrintable)
			{
				nUpperChar = CBCGPKeyboardManager::TranslateCharToUpper (nChar);
			}

			CBCGPToolbarButton* pButton;
			if (bKeyIsPrintable && m_AcellKeys.Lookup (nUpperChar, pButton))
			{
				ASSERT_VALID (pButton);

				pNewSelButton = pButton;

				//-------------------
				// Find button index:
				//-------------------
				int iIndex = 0;
				for (POSITION pos = m_Buttons.GetHeadPosition ();
					pos != NULL; iIndex ++)
				{
					CBCGPToolbarButton* pListButton = (CBCGPToolbarButton*) m_Buttons.GetNext (pos);
					ASSERT (pListButton != NULL);

					if (pListButton == pButton)
					{
						iNewHighlight = iIndex;
						break;
					}
				}
				
				CBCGPToolbarMenuButton* pMenuButton =
					DYNAMIC_DOWNCAST (CBCGPToolbarMenuButton, pButton);

				if (pMenuButton != NULL)
				{
					if (pMenuButton->OpenPopupMenu ())
					{
						if (pMenuButton->m_pPopupMenu != NULL)
						{
							//--------------------------
							// Select a first menu item:
							//--------------------------
							pMenuButton->m_pPopupMenu->SendMessage (WM_KEYDOWN, VK_HOME);
						}
					}
					else
					{
						// If the newly selected item is not highlighted,
						// then make the menu go away.

						if ((pButton->m_nStyle & TBBS_DISABLED) != 0)
						{
							InvokeMenuCommand (0, pButton);
							return TRUE;
						}
						//-----------------------------------------

						bProcessed = OnSendCommand (pMenuButton);
						if (bProcessed)
						{
							return TRUE;
						}
					}
				}
			}
			else if (CBCGPMenuBar::m_bRecentlyUsedMenus &&
				!m_bAreAllCommandsShown)
			{
				///---------------------------------------------------
				// Maybe, this accelerator is belong to "hidden' item?
				//----------------------------------------------------
				UINT uiCmd = 0;
				if (m_HiddenItemsAccel.Lookup (nUpperChar, uiCmd))
				{
					InvokeMenuCommand (uiCmd, NULL);
					return TRUE;
				}
			}
		}
	}

	if (pNewSelButton != pOldSelButton)
	{
		ASSERT_VALID (pNewSelButton);
		ASSERT (iNewHighlight >= 0 && iNewHighlight < m_Buttons.GetCount ());
		ASSERT (GetButton (iNewHighlight) == pNewSelButton);

	    if (bSendEvent && !m_bInScrollMode)
		{
			AccNotifyObjectFocusEvent (iNewHighlight);
		}

		if (IsCustomizeMode ())
		{
			m_iSelected = iNewHighlight;
		}

		m_iHighlighted = iNewHighlight;

		CRect rectClient;
		GetClientRect (rectClient);

		CRect rectNew = pNewSelButton->Rect ();

		if (rectNew.top < rectClient.top || rectNew.bottom > rectClient.bottom)
		{
			// don't redraw items, popup menu will be scrolled now
		}
		else
		{
			if (pOldSelButton != NULL)
			{
				InvalidateRect (pOldSelButton->Rect ());
			}

			InvalidateRect (rectNew);
			UpdateWindow ();
		}

		if (pNewSelButton->m_nID != (UINT) -1)
		{
			ShowCommandMessageString (pNewSelButton->m_nID);
		}
	}

	return bProcessed;
}
//**************************************************************************************
void CBCGPPopupMenuBar::OnTimer(UINT_PTR nIDEvent) 
{
	CPoint ptCursor;
	::GetCursorPos (&ptCursor);
	ScreenToClient (&ptCursor);

	if (nIDEvent == uiPopupTimerEvent)
	{
		KillTimer (uiPopupTimerEvent);

		//---------------------------------
		// Remove current tooltip (if any):
		//---------------------------------
		if (m_pToolTip->GetSafeHwnd () != NULL)
		{
			m_pToolTip->ShowWindow (SW_HIDE);
		}

		if (m_pDelayedClosePopupMenuButton != NULL &&
			m_pDelayedClosePopupMenuButton->Rect ().PtInRect (ptCursor))
		{
			return;
		}

		CloseDelayedSubMenu ();

		CBCGPToolbarMenuButton* pDelayedPopupMenuButton = m_pDelayedPopupMenuButton;
		m_pDelayedPopupMenuButton = NULL;

		if (pDelayedPopupMenuButton != NULL &&
			m_iHighlighted >= 0 &&
			m_iHighlighted < m_Buttons.GetCount () &&
			GetButton (m_iHighlighted) == pDelayedPopupMenuButton)
		{
			ASSERT_VALID (pDelayedPopupMenuButton);
			pDelayedPopupMenuButton->OpenPopupMenu (this);
		}
	}
	else if (nIDEvent == uiRemovePopupTimerEvent)
	{
		KillTimer (uiRemovePopupTimerEvent);

		if (m_pDelayedClosePopupMenuButton != NULL)
		{
			ASSERT_VALID (m_pDelayedClosePopupMenuButton);
			CBCGPPopupMenu* pParentMenu = DYNAMIC_DOWNCAST (CBCGPPopupMenu, GetParent ());

			if (m_pDelayedClosePopupMenuButton->Rect ().PtInRect (ptCursor))
			{
				return;
			}

			m_pDelayedClosePopupMenuButton->OnCancelMode ();
			m_pDelayedClosePopupMenuButton = NULL;

			if (pParentMenu != NULL)
			{
				CBCGPPopupMenu::ActivatePopupMenu (BCGCBProGetTopLevelFrame (this), pParentMenu);
			}
		}
	}
	else if (nIDEvent == uiAccNotifyEvent)
	{
		KillTimer (uiAccNotifyEvent);

		CRect rc;
		GetClientRect (&rc);
		if (!rc.PtInRect (ptCursor))
		{
			return;
		}

		int nIndex = HitTest (ptCursor);
		if (m_iAccHotItem == nIndex && m_iAccHotItem != -1)
		{
			AccNotifyObjectFocusEvent (nIndex);
		}
	}
}
//**************************************************************************************
void CBCGPPopupMenuBar::StartPopupMenuTimer (CBCGPToolbarMenuButton* pMenuButton,
											int nDelayFactor/* = 1*/)
{
	ASSERT (nDelayFactor > 0);

	if (m_pDelayedPopupMenuButton != NULL)
	{
		KillTimer (uiPopupTimerEvent);
	}

	if ((m_pDelayedPopupMenuButton = pMenuButton) != NULL)
	{
		if (m_pDelayedPopupMenuButton == m_pDelayedClosePopupMenuButton)
		{
			RestoreDelayedSubMenu ();
			m_pDelayedPopupMenuButton = NULL;
		}
		else
		{
			SetTimer (uiPopupTimerEvent, m_uiPopupTimerDelay * nDelayFactor, NULL);
		}
	}
}
//**********************************************************************************
void CBCGPPopupMenuBar::OnLButtonDown(UINT nFlags, CPoint point) 
{
	m_bFirstClick = FALSE;
	m_bIsClickOutsideItem = TRUE;

	CRect rectClient;
	GetClientRect (&rectClient);

	if (!IsCustomizeMode () && 
		!rectClient.PtInRect (point))
	{
		CBCGPToolBar* pDestBar = FindDestBar (point);
		if (pDestBar != NULL)
		{
			ASSERT_VALID (pDestBar);

			CPoint ptDest = point;
			MapWindowPoints (pDestBar, &ptDest, 1);

			pDestBar->SendMessage (	WM_LBUTTONDOWN, 
									nFlags, 
									MAKELPARAM (ptDest.x, ptDest.y));
		}
	}

	CBCGPToolBar::OnLButtonDown(nFlags, point);
}
//**********************************************************************************
void CBCGPPopupMenuBar::OnLButtonUp(UINT nFlags, CPoint point) 
{
	CRect rectClient;
	GetClientRect (&rectClient);

	if (!m_bFirstClick &&
		!IsCustomizeMode () && 
		!rectClient.PtInRect (point))
	{
		CBCGPToolBar* pDestBar = FindDestBar (point);
		if (pDestBar != NULL)
		{
			MapWindowPoints (pDestBar, &point, 1);
			pDestBar->SendMessage (	WM_LBUTTONUP, 
									nFlags, 
									MAKELPARAM (point.x, point.y));
		}

		CFrameWnd* pParentFrame = BCGPGetParentFrame (this);
		ASSERT_VALID (pParentFrame);

		pParentFrame->DestroyWindow ();
		return;
	}

	if (!IsCustomizeMode () && m_iHighlighted >= 0)
	{
		m_iButtonCapture = m_iHighlighted;
	}

	m_bFirstClick = FALSE;
	if (m_bIsClickOutsideItem)
	{
		CBCGPToolBar::OnLButtonUp (nFlags, point);
	}
}
//**********************************************************************************
BOOL CBCGPPopupMenuBar::OnSetDefaultButtonText (CBCGPToolbarButton* pButton)
{
	ASSERT_VALID (pButton);

	CBCGPPopupMenu* pParentMenu = DYNAMIC_DOWNCAST (CBCGPPopupMenu, GetParent ());
	if (pParentMenu != NULL)
	{
		CBCGPToolBar* pToolBar = pParentMenu->GetParentToolBar ();
		if (pToolBar != NULL && pToolBar->OnSetDefaultButtonText (pButton))
		{
			return TRUE;
		}
	}

	return CBCGPToolBar::OnSetDefaultButtonText (pButton);
}
//****************************************************************************************
BOOL CBCGPPopupMenuBar::EnableContextMenuItems (CBCGPToolbarButton* pButton, CMenu* pPopup)
{
	if (!CBCGPToolBar::IsCustomizeMode ())
	{
		// Disable context menu
		return FALSE;
	}

	ASSERT_VALID (pButton);
	ASSERT_VALID (pPopup);

	pButton->m_bText = TRUE;
	CBCGPToolBar::EnableContextMenuItems (pButton, pPopup);

	pPopup->EnableMenuItem (ID_BCGBARRES_TOOLBAR_IMAGE, MF_GRAYED | MF_BYCOMMAND);
	pPopup->EnableMenuItem (ID_BCGBARRES_TOOLBAR_TEXT, MF_ENABLED | MF_BYCOMMAND);
	pPopup->EnableMenuItem (ID_BCGBARRES_TOOLBAR_IMAGE_AND_TEXT, MF_ENABLED | MF_BYCOMMAND);

	if (BCGPCMD_MGR.IsMenuItemWithoutImage (pButton->m_nID))
	{
		pPopup->CheckMenuItem (ID_BCGBARRES_TOOLBAR_TEXT, MF_CHECKED  | MF_BYCOMMAND);
		pPopup->CheckMenuItem (ID_BCGBARRES_TOOLBAR_IMAGE_AND_TEXT, MF_UNCHECKED  | MF_BYCOMMAND);
	}

	return TRUE;
}
//****************************************************************************************
void CBCGPPopupMenuBar::OnMouseMove(UINT nFlags, CPoint point) 
{
	if (m_bFirstMove)
	{
		m_bFirstMove = FALSE;
		return;
	}

	if (m_ptCursorInit != CPoint (-1, -1))
	{
		CPoint ptCursor;
		::GetCursorPos (&ptCursor);

		if (ptCursor == m_ptCursorInit)
		{
			return;
		}

		m_ptCursorInit = CPoint (-1, -1);
	}

	CRect rectClient;
	GetClientRect (&rectClient);

	if (IsCustomizeMode () ||
		rectClient.PtInRect (point))
	{
		CBCGPToolBar::OnMouseMove(nFlags, point);
	}
	else
	{
		CBCGPToolBar* pDestBar = FindDestBar (point);
		if (pDestBar != NULL)
		{
			MapWindowPoints (pDestBar, &point, 1);
			pDestBar->SendMessage (	WM_MOUSEMOVE, 
									nFlags, 
									MAKELPARAM (point.x, point.y));
		}
	}
}
//***************************************************************************************
CBCGPToolBar* CBCGPPopupMenuBar::FindDestBar (CPoint point)
{
	if (GetSafeHwnd() == NULL)
	{
		return NULL;
	}

	ScreenToClient (&point);

	CRect rectClient;

	CBCGPPopupMenu* pPopupMenu = DYNAMIC_DOWNCAST (CBCGPPopupMenu, GetParent ());
	if (pPopupMenu == NULL)
	{
		return NULL;
	}

	ASSERT_VALID (pPopupMenu);

	CBCGPPopupMenu* pLastPopupMenu = pPopupMenu;

	//-------------------------------
	// Go up trougth all popup menus:
	//-------------------------------
	while ((pPopupMenu = pPopupMenu->GetParentPopupMenu ()) != NULL)
	{
		CBCGPPopupMenuBar* pPopupMenuBar = pPopupMenu->GetMenuBar ();
		ASSERT_VALID (pPopupMenuBar);

		pPopupMenuBar->GetClientRect (&rectClient);
		pPopupMenuBar->MapWindowPoints (this, &rectClient);

		if (rectClient.PtInRect (point))
		{
			return pPopupMenuBar;
		}

		pLastPopupMenu = pPopupMenu;
	}

	ASSERT_VALID (pLastPopupMenu);

	//--------------------
	// Try parent toolbar:
	//--------------------
	CBCGPToolBar* pToolBar = pLastPopupMenu->GetParentToolBar ();
	if (pToolBar != NULL)
	{
		pToolBar->GetClientRect (&rectClient);
		pToolBar->MapWindowPoints (this, &rectClient);

		if (rectClient.PtInRect (point))
		{
			return pToolBar;
		}
	}

	return NULL;
}
//*********************************************************************************************
DROPEFFECT CBCGPPopupMenuBar::OnDragOver(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point)
{
	//-----------------------------------------------
	// Disable MOVING menu item into one of submenus!
	//-----------------------------------------------
	if ((dwKeyState & MK_CONTROL) == 0)
	{
		CBCGPPopupMenu* pParentMenu = DYNAMIC_DOWNCAST (CBCGPPopupMenu, GetParent ());
		if (pParentMenu != NULL)
		{
			CBCGPToolBar* pParentBar = pParentMenu->GetParentToolBar ();
			CBCGPToolbarMenuButton* pParentButton = pParentMenu->GetParentButton ();

			if (pParentBar != NULL && pParentButton != NULL &&
				pParentBar->IsDragButton (pParentButton))
			{
				return DROPEFFECT_NONE;
			}
		}
	}

	return CBCGPToolBar::OnDragOver(pDataObject, dwKeyState, point);
}
//*****************************************************************************************
void CBCGPPopupMenuBar::OnFillBackground (CDC* pDC)
{
	ASSERT_VALID (this);
	ASSERT (::IsWindow (GetSafeHwnd ()));
	ASSERT_VALID (pDC);

	if (CBCGPToolBar::IsCustomizeMode () ||
		!CBCGPMenuBar::m_bRecentlyUsedMenus ||
		m_bPaletteMode)
	{
		return;
	}

	//--------------------------------------------------------------
	// Only menubar first-level menus may hide rarely used commands:
	//--------------------------------------------------------------
	CBCGPPopupMenu* pParentMenu = DYNAMIC_DOWNCAST (CBCGPPopupMenu, GetParent ());
	if (pParentMenu == NULL || !pParentMenu->HideRarelyUsedCommands ())
	{
		return;
	}

	BOOL bFirstRarelyUsedButton = TRUE;
	CRect rectRarelyUsed;

	for (POSITION pos = m_Buttons.GetHeadPosition (); pos != NULL;)
	{
		CBCGPToolbarButton* pButton = (CBCGPToolbarButton*) m_Buttons.GetNext (pos);
		ASSERT (pButton != NULL);

		if (pButton->m_nStyle & TBBS_SEPARATOR)
		{
			if (pos != NULL &&
				CBCGPToolBar::IsCommandRarelyUsed (
					((CBCGPToolbarButton*) m_Buttons.GetAt (pos))->m_nID))
			{
				continue;
			}
		}

		BOOL bDraw = FALSE;

		if (CBCGPToolBar::IsCommandRarelyUsed (pButton->m_nID))
		{
			if (bFirstRarelyUsedButton)
			{
				bFirstRarelyUsedButton = FALSE;
				rectRarelyUsed = pButton->Rect ();
			}

			if (pos == NULL)	// Last button
			{
				rectRarelyUsed.bottom = pButton->Rect ().bottom;
				bDraw = TRUE;
			}
		}
		else
		{
			if (!bFirstRarelyUsedButton)
			{
				rectRarelyUsed.bottom = pButton->Rect ().top;
				bDraw = TRUE;
			}

			bFirstRarelyUsedButton = TRUE;
		}

		if (bDraw)
		{
			CBCGPVisualManager::GetInstance ()->OnHighlightRarelyUsedMenuItems (
				pDC, rectRarelyUsed);
		}
	}
}
//*************************************************************************************
INT_PTR CBCGPPopupMenuBar::OnToolHitTest(CPoint point, TOOLINFO* pTI) const
{
	ASSERT_VALID(this);

	if (m_bPaletteMode)
	{
		return CBCGPToolBar::OnToolHitTest (point, pTI);
	}

	int nHit = ((CBCGPPopupMenuBar*)this)->HitTest(point);
	if (nHit != -1)
	{
		CBCGPToolbarButton* pButton = 
			DYNAMIC_DOWNCAST (CBCGPToolbarButton, GetButton (nHit));

		if (pButton != NULL)
		{
			if (pTI != NULL)
			{
				pTI->uId = pButton->m_nID;
				pTI->hwnd = GetSafeHwnd ();
				pTI->rect = pButton->Rect ();
			}

			if (!pButton->OnToolHitTest (this, pTI))
			{
				nHit = pButton->m_nID;
			}
			else if (pTI->lpszText != NULL)
			{
				CString strText;

				if (pTI->lpszText != NULL)
				{
					strText = pTI->lpszText;
					::free (pTI->lpszText);
				}

				CString strDescr;
				CFrameWnd* pParent = GetParentFrame ();
				if (pParent->GetSafeHwnd () != NULL && pButton->m_nID != 0)
				{
					pParent->GetMessageString (pButton->m_nID, strDescr);
				}

				CBCGPTooltipManager::SetTooltipText (pTI,
					m_pToolTip, BCGP_TOOLTIP_TYPE_TOOLBAR, strText, strDescr);
			}
		}
	}

	return nHit;
}
//**********************************************************************************
int CBCGPPopupMenuBar::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CBCGPToolBar::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (m_uiPopupTimerDelay == (UINT) -1)	// Not defined yet
	{
		m_uiPopupTimerDelay = 500;

		CBCGPRegistrySP regSP;
		CBCGPRegistry& reg = regSP.Create (FALSE, TRUE);

		if (reg.Open (_T("Control Panel\\Desktop")))
		{
			CString strVal;

			if (reg.Read (_T("MenuShowDelay"), strVal))
			{
				m_uiPopupTimerDelay = (UINT) _ttol (strVal);

				//------------------------
				// Just limit it to 5 sec:
				//------------------------
				m_uiPopupTimerDelay = min (5000, m_uiPopupTimerDelay);
			}
		}
	}

	::GetCursorPos (&m_ptCursorInit);
	return 0;
}
//*****************************************************************
void CBCGPPopupMenuBar::SetButtonStyle(int nIndex, UINT nStyle)
{
	CBCGPToolbarButton* pButton = GetButton (nIndex);
	if (pButton == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	UINT nOldStyle = pButton->m_nStyle;
	if (nOldStyle != nStyle)
	{
		// update the style and invalidate
		pButton->m_nStyle = nStyle;

		// invalidate the button only if both styles not "pressed"
		if (!(nOldStyle & nStyle & TBBS_PRESSED))
		{
			CBCGPToolbarMenuButton* pMenuButton =
				DYNAMIC_DOWNCAST (CBCGPToolbarMenuButton, GetButton (nIndex));

			BOOL bWasChecked = nOldStyle & TBBS_CHECKED;
			BOOL bChecked = nStyle & TBBS_CHECKED;

			// If checked style was changed. redraw check box (or image) area only:
			if (pMenuButton != NULL && bWasChecked != bChecked)
			{
				CRect rectImage;
				pMenuButton->GetImageRect (rectImage);

				rectImage.InflateRect (afxData.cxBorder2 * 2, afxData.cyBorder2 * 2);

				InvalidateRect (rectImage);
				UpdateWindow ();
			}
			else if ((nOldStyle ^ nStyle) != TBSTATE_PRESSED)
			{
				InvalidateButton(nIndex);
			}
		}
	}
}
// ---------------------------------------------------------------
LRESULT CBCGPPopupMenuBar::OnIdleUpdateCmdUI(WPARAM, LPARAM)
{
	if (m_bTrackMode)
	{
		return 0;
	}

	// the style must be visible and if it is docked
	// the dockbar style must also be visible
	if (GetStyle() & WS_VISIBLE)
	{
		CFrameWnd* pTarget = (CFrameWnd*) GetCommandTarget ();
		if (pTarget == NULL || !pTarget->IsFrameWnd())
		{
			pTarget = BCGPGetParentFrame(this);
		}

		if (pTarget != NULL)
		{
			BOOL bAutoMenuEnable = FALSE;
			if (pTarget->IsFrameWnd ())
			{
				bAutoMenuEnable = ((CFrameWnd*) pTarget)->m_bAutoMenuEnable;
			}

			OnUpdateCmdUI (pTarget, bAutoMenuEnable);
		}
	}

	return 0L;
}
// ---------------------------------------------------------------
CWnd* CBCGPPopupMenuBar::GetCommandTarget () const
{
	if (m_bTrackMode)
	{
		return NULL;
	}

	CBCGPPopupMenu* pParentMenu = DYNAMIC_DOWNCAST (CBCGPPopupMenu, GetParent ());
	if (pParentMenu != NULL && pParentMenu->GetMessageWnd () != NULL)
	{
         return pParentMenu;
	}

	return CBCGPToolBar::GetCommandTarget ();
}
//*******************************************************************************
void CBCGPPopupMenuBar::CloseDelayedSubMenu ()
{
	ASSERT_VALID (this);

	if (m_pDelayedClosePopupMenuButton != NULL)
	{
		ASSERT_VALID (m_pDelayedClosePopupMenuButton);

		KillTimer (uiRemovePopupTimerEvent);

		m_pDelayedClosePopupMenuButton->OnCancelMode ();
		m_pDelayedClosePopupMenuButton = NULL;
	}
}
//*******************************************************************************
void CBCGPPopupMenuBar::RestoreDelayedSubMenu ()
{
	ASSERT_VALID (this);

	if (m_pDelayedClosePopupMenuButton == NULL)
	{
		return;
	}

	ASSERT_VALID (m_pDelayedClosePopupMenuButton);
	m_pDelayedClosePopupMenuButton->m_bToBeClosed = FALSE;

	int iPrevHighlighted = m_iHighlighted;

	SetHot (m_pDelayedClosePopupMenuButton);

	m_iHighlighted = m_iHot;

	m_pDelayedClosePopupMenuButton = NULL;

	if (iPrevHighlighted != m_iHighlighted)
	{
		if (iPrevHighlighted >= 0)
		{
			InvalidateButton (iPrevHighlighted);
		}

		InvalidateButton (m_iHighlighted);
		UpdateWindow ();
	}

	KillTimer (uiRemovePopupTimerEvent);
}
//*******************************************************************************
BOOL CBCGPPopupMenuBar::LoadFromHash(HMENU hMenu)
{
	return g_menuHash.LoadMenuBar(hMenu, this);
}
//*******************************************************************************
void CBCGPPopupMenuBar::SetInCommand (BOOL bInCommand)
{
	ASSERT_VALID (this);

	m_bInCommand = bInCommand;

	CBCGPPopupMenu* pMenu = DYNAMIC_DOWNCAST (CBCGPPopupMenu, GetParent ());
	if (pMenu != NULL)
	{
		while ((pMenu = pMenu->GetParentPopupMenu ()) != NULL)
		{
			CBCGPPopupMenuBar* pMenuBar = pMenu->GetMenuBar ();
			if (pMenuBar != NULL)
			{
				pMenuBar->SetInCommand (bInCommand);
			}
		}
	}
}
//*******************************************************************************************
void CBCGPPopupMenuBar::OnToolbarImageAndText() 
{
	ASSERT (m_iSelected >= 0);

	CBCGPToolbarButton* pButton = GetButton(m_iSelected);
	if (pButton == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	int iImage = pButton->GetImage ();

	if (iImage < 0)
	{
		OnToolbarAppearance ();
	}
	else
	{
		BCGPCMD_MGR.EnableMenuItemImage (pButton->m_nID, TRUE, iImage);
	}

	AdjustLayout ();
}
//*************************************************************************************
void CBCGPPopupMenuBar::OnToolbarText() 
{
	ASSERT (m_iSelected >= 0);

	CBCGPToolbarButton* pButton = GetButton(m_iSelected);
	if (pButton == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	BCGPCMD_MGR.EnableMenuItemImage (pButton->m_nID, FALSE);
	AdjustLayout ();
}
//************************************************************************************
void CBCGPPopupMenuBar::AdjustLayout ()
{
	ASSERT_VALID (this);

	if (GetSafeHwnd () == NULL)
	{
		return;
	}

	AdjustLocations ();

	Invalidate ();
	UpdateWindow ();

	if (!CBCGPToolBar::IsCustomizeMode ())
	{
		return;
	}

	CBCGPPopupMenu* pParentMenu = DYNAMIC_DOWNCAST (CBCGPPopupMenu, GetParent ());
	if (pParentMenu != NULL)
	{
		ASSERT_VALID (pParentMenu);
		pParentMenu->RecalcLayout (FALSE);
	}
}
//************************************************************************************
void CBCGPPopupMenuBar::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	int iItem = HitTest (point);

	if (iItem >= 0)
	{
		CBCGPToolbarMenuButton* pMenuItem = DYNAMIC_DOWNCAST (
			CBCGPToolbarMenuButton, GetButton (iItem));
		if (pMenuItem != NULL && (pMenuItem->m_nID == (UINT) -1 || pMenuItem->IsDroppedDown()))
		{
			CWnd::OnLButtonDblClk(nFlags, point);
			return;
		}
	}

	CBCGPToolBar::OnLButtonDblClk(nFlags, point);
}
//************************************************************************************
void CBCGPPopupMenuBar::OnCalcSeparatorRect (CBCGPToolbarButton* pButton, 
											 CRect& rectSeparator, 
											 BOOL bHorz)
{
	CRect rectClient;
	GetClientRect (rectClient);
	
	rectSeparator = pButton->Rect ();

	if (pButton->m_bWrap && bHorz && m_bPaletteMode)
	{
		rectSeparator.right = rectClient.right;

		rectSeparator.top = pButton->Rect ().bottom;
		rectSeparator.bottom = rectSeparator.top + LINE_OFFSET;
	}	

}
//*******************************************************************************************
void CBCGPPopupMenuBar::OnAfterButtonDelete ()
{
	AdjustLayout ();
	RedrawWindow ();
}
//*******************************************************************************************
BOOL CBCGPPopupMenuBar::BuildOrigItems (UINT uiMenuResID)
{
	ASSERT_VALID (this);

	while (!m_OrigButtons.IsEmpty ())
	{
		delete m_OrigButtons.RemoveHead ();
	}

	if (GetWorkspace () == NULL ||
		!GetWorkspace ()->IsResourceSmartUpdate ())
	{
		return FALSE;
	}
	
	CMenu menu;
	if (!menu.LoadMenu (uiMenuResID))
	{
		ASSERT (FALSE);
		return FALSE;
	}

	CMenu* pMenu = menu.GetSubMenu (0);
	if (pMenu == NULL)
	{
		return FALSE;
	}

	int iCount = (int) pMenu->GetMenuItemCount ();
	for (int i = 0; i < iCount; i ++)
	{
		UINT uiID = pMenu->GetMenuItemID (i);

		CString strText;

#ifdef _DEBUG
		pMenu->GetMenuString (i, strText, MF_BYPOSITION);
#endif

		switch (uiID)
		{
		case -1:	// Pop-up menu
			{
				CMenu* pPopupMenu = pMenu->GetSubMenu (i);
				ASSERT (pPopupMenu != NULL);

				CBCGPToolbarMenuButton*	pButton	= new CBCGPToolbarMenuButton;
				ASSERT_VALID (pButton);

				pButton->Initialize(0, pPopupMenu->GetSafeHmenu (), -1, strText);
				m_OrigButtons.AddTail (pButton);
			}
			break;

		case 0:		// Separator
			{
				CBCGPToolbarButton* pButton = new CBCGPToolbarButton;
				ASSERT (pButton != NULL);

				pButton->m_nStyle = TBBS_SEPARATOR;
				m_OrigButtons.AddTail (pButton);
			}
			break;

		default:	// Regular command

			m_OrigButtons.AddTail (new CBCGPToolbarButton (uiID, -1, strText));
			break;
		}
	}

	return TRUE;
}
//*****************************************************************************************
void CBCGPPopupMenuBar::ShowCommandMessageString (UINT uiCmdId)
{
	ASSERT_VALID (this);

	if (m_bDropDownListMode)
	{
		GetOwner()->SendMessage(WM_SETMESSAGESTRING, AFX_IDS_IDLEMESSAGE);
		return;
	}


	CBCGPToolBar::ShowCommandMessageString (uiCmdId);
}
//*****************************************************************************************
int CBCGPPopupMenuBar::GetGutterWidth ()
{
	ASSERT_VALID (this);

	if (m_bDisableSideBarInXPMode)
	{
		return 0;
	}

	BOOL bQuickMode = FALSE;

	CWnd* pWnd = GetParent();

	if (pWnd != NULL && pWnd->IsKindOf(RUNTIME_CLASS(CBCGPPopupMenu)))
	{
		CBCGPPopupMenu* pMenu = DYNAMIC_DOWNCAST (CBCGPPopupMenu, pWnd);

		if (pMenu->IsCustomizePane())
		{
			bQuickMode = TRUE;
		}
	}

	const int nImageMargin = CBCGPVisualManager::GetInstance ()->GetMenuImageMargin ();

	int cx = 0;
	int cxImage = CBCGPToolBar::GetMenuImageSize ().cx;

	if (bQuickMode)
	{
		cx = 2 * cxImage + 4 * nImageMargin + 4;

	}
	else
	{
		cx = cxImage + 2 * nImageMargin + 2;
	}

	return cx;
}
//**************************************************************************
HRESULT CBCGPPopupMenuBar::get_accRole(VARIANT varChild, VARIANT *pvarRole)
{
	if (pvarRole == NULL)
    {
        return E_INVALIDARG;
    }

	if (varChild.vt == VT_I4 && varChild.lVal == CHILDID_SELF)
	{
		pvarRole->vt = VT_I4;
		pvarRole->lVal = ROLE_SYSTEM_TOOLBAR;
		return S_OK;
	}

	return CBCGPToolBar::get_accRole (varChild, pvarRole);
}
//**************************************************************************
HRESULT CBCGPPopupMenuBar::get_accState(VARIANT varChild, VARIANT *pvarState)
{
	if (pvarState == NULL)
    {
        return E_INVALIDARG;
    }

	if (varChild.vt == VT_I4 && varChild.lVal == CHILDID_SELF)
	{
		pvarState->vt = VT_I4;
		pvarState->lVal = STATE_SYSTEM_DEFAULT;
		return S_OK;
	}

	return CBCGPToolBar::get_accState (varChild, pvarState);
}
//**************************************************************************
HRESULT CBCGPPopupMenuBar::get_accParent(IDispatch **ppdispParent)
{
    if (ppdispParent == NULL)
	{
		return E_INVALIDARG;
	}

	*ppdispParent = NULL;

	
	CBCGPPopupMenu* pParentMenu = DYNAMIC_DOWNCAST (CBCGPPopupMenu, GetParent());
	if (pParentMenu == NULL) 
	{
		return S_FALSE;
	}

	return AccessibleObjectFromWindow(pParentMenu->GetSafeHwnd(), (DWORD)OBJID_CLIENT, IID_IAccessible, (void**)ppdispParent);
}
//**************************************************************************

HRESULT CBCGPPopupMenuBar::get_accName(VARIANT varChild, BSTR *pszName)
{
	if (pszName == NULL)
	{
		return E_INVALIDARG;
	}

	if ((varChild.vt == VT_I4) && (varChild.lVal == CHILDID_SELF))
	{	
		CBCGPPopupMenu* pParentMenu = DYNAMIC_DOWNCAST (CBCGPPopupMenu, GetParent());
		if (pParentMenu == NULL) 
		{
			return S_FALSE;
		}

		CBCGPToolbarMenuButton* pMenuBtn = pParentMenu->GetParentButton();
		CString strName = _T("Popup");

		if (pMenuBtn != NULL)
		{
			strName = pMenuBtn->GetDisplayText();
		}
		else
		{
			CBCGPBaseRibbonElement*	pParentRibbonElement =  pParentMenu->GetParentRibbonElement ();

			if (pParentRibbonElement != NULL)
			{
				strName =  pParentRibbonElement->GetText();
				CString strNameCopy = strName;
				strNameCopy.Remove (_T(' '));

				if (strNameCopy.IsEmpty())
				{
					strName = pParentRibbonElement->GetToolTipText(); 
				}
			}
		}

		strName.Remove (_T('&'));
		strName.Replace (_T(' '), _T('_'));
		*pszName = strName.AllocSysString();

		return S_OK;
	}

	if (varChild.vt == VT_I4 && varChild.lVal > 0)
	{
		OnSetAccData(varChild.lVal);
		if (m_AccData.m_strAccName.IsEmpty())
		{
			return S_FALSE;
		}
		*pszName = m_AccData.m_strAccName.AllocSysString();
	}

	return S_OK;
}
