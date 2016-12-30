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

// CustomizeButton.cpp: implementation of the CCustomizeButton class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CustomizeButton.h"
#include "BCGGlobals.h"
#include "BCGPToolbar.h"
#include "MenuImages.h"
#include "BCGPToolbarComboBoxButton.h"
#include "bcgprores.h"
#include "BCGPLocalResource.h"
#include "BCGPVisualManager.h"
#include "BCGPDockBarRow.h"
#include "BCGPCustomizeMenuButton.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

BOOL CCustomizeButton::m_bIgnoreLargeIconsMode = FALSE;

IMPLEMENT_SERIAL(CCustomizeButton, CBCGPToolbarMenuButton, VERSIONABLE_SCHEMA | 1)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCustomizeButton::CCustomizeButton()
{
	CommonInit ();
}
//****************************************************************************************
CCustomizeButton::CCustomizeButton(UINT uiCustomizeCmdId, const CString& strCustomizeText)
{
	CommonInit ();

	m_uiCustomizeCmdId = uiCustomizeCmdId;
	m_strCustomizeText = strCustomizeText;
}
//****************************************************************************************
void CCustomizeButton::CommonInit ()
{
	m_uiCustomizeCmdId = 0;
	m_bIsEmpty = FALSE;
	m_bDefaultDraw = TRUE;
	m_sizeExtra = CSize (0, 0);
	m_pWndParentToolbar = NULL;
	m_bIsPipeStyle = TRUE;
	m_bOnRebar = FALSE;
	m_bMenuRightAlign = TRUE;
}
//****************************************************************************************
CCustomizeButton::~CCustomizeButton()
{
}
//****************************************************************************************
void CCustomizeButton::OnChangeParentWnd (CWnd* pWndParent)
{
	CBCGPToolbarButton::OnChangeParentWnd (pWndParent);

	m_pWndParentToolbar = DYNAMIC_DOWNCAST (CBCGPToolBar, pWndParent);
	m_pWndParent = pWndParent;
	m_bText = FALSE;
	m_bIsEmpty = FALSE;
	m_bOnRebar = DYNAMIC_DOWNCAST (CReBar, pWndParent->GetParent ()) != NULL;
}
//****************************************************************************************
void CCustomizeButton::OnDraw (CDC* pDC, const CRect& rect, CBCGPToolBarImages* /*pImages*/,
			BOOL bHorz, BOOL bCustomizeMode, BOOL bHighlight,
			BOOL /*bDrawBorder*/, BOOL /*bGrayDisabledButtons*/)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (this);

	if (m_bMenuMode)
	{
		ASSERT (FALSE);	// Customize button is available for 
						// the "pure" toolbars only!
		return;
	}

	ASSERT_VALID (pDC);
	ASSERT_VALID (this);

	CBCGPMenuImages::IMAGE_STATE imageState = CBCGPMenuImages::ImageBlack;
	COLORREF clrText = globalData.clrBarText;

	if (!bCustomizeMode)
	{
		clrText = CBCGPVisualManager::GetInstance ()->GetToolbarButtonTextColor(this, CBCGPVisualManager::ButtonsIsRegular);
	}

	if (GetRValue (clrText) > 192 &&
		GetGValue (clrText) >192 &&
		GetBValue (clrText) > 192)
	{
		imageState = CBCGPMenuImages::ImageWhite;
	}

	CRect rectBorder = rect;

	//----------------------
	// Fill button interior:
	//----------------------
	m_bDefaultDraw = TRUE;

	FillInterior (pDC, rectBorder, bHighlight || IsDroppedDown ());

	int nMargin = CBCGPVisualManager::GetInstance ()->GetToolBarCustomizeButtonMargin ();

	if (m_bDefaultDraw)
	{
		CSize sizeImage = CBCGPMenuImages::Size ();
		if (CBCGPToolBar::IsLargeIcons () && !m_bIgnoreLargeIconsMode)
		{
			sizeImage.cx *= 2;
			sizeImage.cy *= 2;
		}

		if ((int) m_uiCustomizeCmdId > 0)
		{
			//-----------------
			// Draw menu image:
			//-----------------
			CRect rectMenu = rect;
			if (bHorz)
			{
				rectMenu.top = rectMenu.bottom - sizeImage.cy - 2 * nMargin;
			}
			else
			{
				rectMenu.right = rectMenu.left + sizeImage.cx + 2 * nMargin;
			}

			if ((m_nStyle & (TBBS_PRESSED | TBBS_CHECKED)) || m_pPopupMenu != NULL)
			{
				if (!CBCGPVisualManager::GetInstance ()->IsMenuFlatLook ())
				{
					rectMenu.OffsetRect (1, 1);
				}
			}

			CBCGPMenuImages::Draw (	pDC, 
				bHorz ? CBCGPMenuImages::IdArowDown : CBCGPMenuImages::IdArowLeft,
								rectMenu,
								imageState, sizeImage);
		}

		if (!m_lstInvisibleButtons.IsEmpty ())
		{
			//-------------------
			// Draw "more" image:
			//-------------------
			CRect rectMore = rect;
			if (bHorz)
			{
				rectMore.bottom = rectMore.top + sizeImage.cy + 2 * nMargin;
			}
			else
			{
				rectMore.left = rectMore.right - sizeImage.cx - 2 * nMargin;
			}

			if ((m_nStyle & (TBBS_PRESSED | TBBS_CHECKED)) || m_pPopupMenu != NULL)
			{
				if (!CBCGPVisualManager::GetInstance ()->IsMenuFlatLook ())
				{
					rectMore.OffsetRect (1, 1);
				}
			}

			CBCGPMenuImages::Draw (	pDC, 
								bHorz ? CBCGPMenuImages::IdMoreButtons : CBCGPMenuImages::IdArowShowAll, 
								rectMore,
								imageState, sizeImage);
		}
	}

	//--------------------
	// Draw button border:
	//--------------------
	if (!bCustomizeMode)
	{
		if ((m_nStyle & (TBBS_PRESSED | TBBS_CHECKED)) ||
			m_pPopupMenu != NULL)
		{
			//-----------------------
			// Pressed in or checked:
			//-----------------------
			CBCGPVisualManager::GetInstance ()->OnDrawButtonBorder (pDC,
				this, rectBorder, CBCGPVisualManager::ButtonsIsPressed);
		}
		else if (bHighlight && !(m_nStyle & TBBS_DISABLED) &&
			!(m_nStyle & (TBBS_CHECKED | TBBS_INDETERMINATE)))
		{
			CBCGPVisualManager::GetInstance ()->OnDrawButtonBorder (pDC,
				this, rectBorder, CBCGPVisualManager::ButtonsIsHighlighted);
		}
	}
}
//*****************************************************************************************
CBCGPPopupMenu* CCustomizeButton::CreatePopupMenu ()
{
	if (CBCGPToolBar::m_bAltCustomizeMode ||
		CBCGPToolBar::IsCustomizeMode ())
	{
		return NULL;
	}

	CBCGPPopupMenu* pMenu = CBCGPToolbarMenuButton::CreatePopupMenu ();
	if (pMenu == NULL)
	{
		ASSERT (FALSE);
		return NULL;
	}

	if (m_pWndParentToolbar->IsLocked ())
	{
		pMenu->GetMenuBar ()->m_pRelatedToolbar = m_pWndParentToolbar;
	}

	pMenu->m_bRightAlign = m_bMenuRightAlign && 
		(m_pWndParentToolbar->GetExStyle() & WS_EX_LAYOUTRTL) == 0;

	BOOL bIsFirst = TRUE;

	for (POSITION pos = m_lstInvisibleButtons.GetHeadPosition (); pos != NULL;)
	{
		CBCGPToolbarButton* pButton = (CBCGPToolbarButton*) m_lstInvisibleButtons.GetNext (pos);
		ASSERT_VALID (pButton);

		//--------------------------------------
		// Don't insert first or last separator:
		//--------------------------------------
		if (pButton->m_nStyle & TBBS_SEPARATOR)
		{
			if (bIsFirst)
			{
				continue;
			}

			if (pos == NULL)	// Last
			{
				break;
			}
		}

		int iIndex = -1;

		bIsFirst = FALSE;

		if (pButton->IsKindOf (RUNTIME_CLASS (CBCGPToolbarMenuButton)))
		{
			iIndex = pMenu->InsertItem (*((CBCGPToolbarMenuButton*) pButton));
		}
		else
		{
			if (pButton->m_nID == 0)
			{
				iIndex = pMenu->InsertSeparator ();
			}
			else
			{
				iIndex = pMenu->InsertItem (
					CBCGPToolbarMenuButton (pButton->m_nID, NULL, 
						pButton->GetImage (),
						pButton->m_strText,
						pButton->m_bUserButton));
			}
		}

		if (iIndex < 0)
		{
			continue;
		}

		CBCGPToolbarMenuButton* pMenuButton = pMenu->GetMenuItem (iIndex);
		if (pMenuButton == NULL)
		{
			continue;
		}

		//-----------------------------------------------------
		// Text may be undefined, bring it from the tooltip :-(
		//-----------------------------------------------------
		if ((pMenuButton->m_strText.IsEmpty () || 
			pButton->IsKindOf (RUNTIME_CLASS (CBCGPToolbarComboBoxButton)))
				&& pMenuButton->m_nID != 0)
		{
			CString strMessage;
			int iOffset;
			if (strMessage.LoadString (pMenuButton->m_nID) &&
				(iOffset = strMessage.Find (_T('\n'))) != -1)
			{
				pMenuButton->m_strText = strMessage.Mid (iOffset + 1);
				if ((iOffset = pMenuButton->m_strText.Find (_T('\n'))) != -1)
				{
					pMenuButton->m_strText = pMenuButton->m_strText.Left( iOffset );
				}
			}
		}

        pMenuButton->m_bText = TRUE;
	}

	if ((int) m_uiCustomizeCmdId > 0)
	{
		if (!m_lstInvisibleButtons.IsEmpty ())
		{
			pMenu->InsertSeparator ();
		}

		if (m_pWndParentToolbar->IsAddRemoveQuickCustomize())
		{
			//--------------------------------
			// Prepare Quick Customize Items
			//--------------------------------
			CBCGPPopupMenu menuCustomize;

			CBCGPDockBarRow* pDockRow = m_pWndParentToolbar->GetDockRow ();
			if (pDockRow != NULL)
			{
				const CObList& list = pDockRow->GetControlBarList ();

				for (POSITION pos = list.GetHeadPosition (); pos != NULL;)
				{
					CBCGPToolBar* pToolBar = DYNAMIC_DOWNCAST (CBCGPToolBar, list.GetNext (pos));

					if (pToolBar != NULL && pToolBar->IsVisible () && 
								pToolBar->IsExistCustomizeButton())
					{
						CString strCaption;
						pToolBar->GetWindowText(strCaption);

						strCaption.TrimLeft ();
						strCaption.TrimRight ();

						if (!strCaption.GetLength ())
						{
							CBCGPLocalResource locaRes;
							strCaption.LoadString (IDS_BCGBARRES_UNTITLED_TOOLBAR);
						}

						CString strToolId;
						strToolId.Format (_T("%d"), pToolBar->GetDlgCtrlID ());
							
						//------------------------
						// Insert Dummy Menu Item
						//------------------------
						CBCGPPopupMenu* pMenuDummy = new CBCGPPopupMenu ();
						pMenuDummy->InsertItem (CBCGPToolbarMenuButton (1, NULL, -1, strToolId)); 

						CBCGPToolbarMenuButton btnToolCaption ((UINT)-1, 
							pMenuDummy->GetMenuBar ()->ExportToMenu (), -1, strCaption); 

						menuCustomize.InsertItem (btnToolCaption);
						delete pMenuDummy;
					}
				}
			}
			else
			{
				CString strCaption;
				m_pWndParentToolbar->GetWindowText(strCaption);

				strCaption.TrimLeft();
				strCaption.TrimRight();

				if (!strCaption.GetLength())
				{
					CBCGPLocalResource locaRes;
					strCaption.LoadString(IDS_BCGBARRES_UNTITLED_TOOLBAR);
				}

				CString strToolId;
				strToolId.Format(_T("%d"), m_pWndParentToolbar->GetDlgCtrlID());	
					
				//------------------------
				// Insert Dummy Menu Item
				//------------------------
				CBCGPPopupMenu* pMenuDummy = new CBCGPPopupMenu();
				pMenuDummy->InsertItem(CBCGPToolbarMenuButton(1, NULL, -1, strToolId)); //_T("DUMMY")

				CBCGPToolbarMenuButton btnToolCaption((UINT)-1, 
					pMenuDummy->GetMenuBar()->ExportToMenu(), -1, strCaption); 

				menuCustomize.InsertItem(btnToolCaption);
				delete pMenuDummy;
			}

			CBCGPToolbarMenuButton btnStandard (m_uiCustomizeCmdId, NULL, -1,
				m_strCustomizeText);

			menuCustomize.InsertItem (btnStandard);

			CString strLabel;

			{
				CBCGPLocalResource locaRes;
				strLabel.LoadString (IDS_BCGBARRES_ADD_REMOVE_BTNS);
			}

			CBCGPToolbarMenuButton	btnAddRemove((UINT)-1,
				menuCustomize.GetMenuBar()->ExportToMenu(),	-1,	strLabel);

			btnAddRemove.EnableQuickCustomize();

			//-----------------
			//Brothers Support
			//-----------------
			if (m_pWndParentToolbar != NULL && m_pWndParentToolbar->IsBrother ())
			{
				if (m_pWndParentToolbar->CanHandleBrothers ())
				{
					BOOL nOneRow = m_pWndParentToolbar->IsOneRowWithBrother();
					CString strText;
					if (nOneRow)
					{
						CBCGPLocalResource locaRes;
						strText.LoadString (IDS_BCGBARRES_SHOWTWOROWS);
					}
					else
					{
						CBCGPLocalResource locaRes;
						strText.LoadString (IDS_BCGBARRES_SHOWONEROW);
					}
				
					CBCGPCustomizeMenuButton btnBrother (BCGPCUSTOMIZE_INTERNAL_ID, NULL, -1, strText, FALSE);
					CBCGPCustomizeMenuButton::SetParentToolbar(m_pWndParentToolbar);
					btnBrother.SetBrothersButton();
					pMenu->InsertItem(btnBrother);
				}
			}

			pMenu->InsertItem(btnAddRemove);
			pMenu->SetQuickMode();
			pMenu->SetQuickCustomizeType(CBCGPPopupMenu::QUICK_CUSTOMIZE_ADDREMOVE);
		}
		else // for old version (< 6.5) compatibility.
		{
			CBCGPToolbarMenuButton btnStandard(m_uiCustomizeCmdId, NULL, -1,
			m_strCustomizeText);

			pMenu->InsertItem(btnStandard);
		}	
	}

	//-----------------------------------------------------------
	// All menu commands should be routed via the same window as
	// parent toolbar commands:
	//-----------------------------------------------------------
	if (m_pWndParentToolbar != NULL)
	{
		pMenu->m_pMessageWnd = m_pWndParentToolbar->GetOwner ();
	}

	return pMenu;
}
//*****************************************************************************************
SIZE CCustomizeButton::OnCalculateSize (CDC* /*pDC*/, const CSize& sizeDefault, BOOL bHorz)
{
	if (m_bIsEmpty)
	{
		return CSize (0, 0);
	}

	if (m_strText.IsEmpty ())
	{
		CBCGPLocalResource locaRes;
		m_strText.LoadString (IDS_BCGBARRES_TOOLBAR_OPTIONS);

		ASSERT (!m_strText.IsEmpty ());
	}

	if (m_pWndParentToolbar != NULL && !m_pWndParentToolbar->IsDocked ())
	{
		return CSize (0, 0);
	}

	int nMargin = CBCGPVisualManager::GetInstance ()->GetToolBarCustomizeButtonMargin ();
	const int xLargeIcons = 
		CBCGPToolBar::IsLargeIcons () && !m_bIgnoreLargeIconsMode ? 2 : 1;

	if (bHorz)
	{
		return CSize (	CBCGPMenuImages::Size ().cx * xLargeIcons + 2 * nMargin, 
						sizeDefault.cy);
	}
	else
	{
		return CSize (	sizeDefault.cx, 
						CBCGPMenuImages::Size ().cy * xLargeIcons + 2 * nMargin);
	}
}
//*****************************************************************************************
void CCustomizeButton::CopyFrom (const CBCGPToolbarButton& s)
{
	CBCGPToolbarMenuButton::CopyFrom (s);
	const CCustomizeButton& src = (const CCustomizeButton&) s;

	m_uiCustomizeCmdId = src.m_uiCustomizeCmdId;
	m_strCustomizeText = src.m_strCustomizeText;
	m_bIsEmpty = src.m_bIsEmpty;
	m_bIsPipeStyle = src.m_bIsPipeStyle;
	m_bMenuRightAlign = src.m_bMenuRightAlign;
}
//*********************************************************************************
void CCustomizeButton::OnCancelMode ()
{
	CBCGPToolbarMenuButton::OnCancelMode ();

	if (m_sizeExtra != CSize (0, 0) && m_pWndParentToolbar != NULL)
	{
		int nIndex = m_pWndParentToolbar->ButtonToIndex (this);
		if (nIndex >= 0)
		{
			m_pWndParentToolbar->InvalidateButton (nIndex);
		}
	}
}
//********************************************************************************
BOOL CCustomizeButton::InvokeCommand (CBCGPPopupMenuBar* pMenuBar, 
		const CBCGPToolbarButton* pButton)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pMenuBar);
	ASSERT_VALID (pButton);

	if (m_pWndParentToolbar == NULL)
	{
		return FALSE;
	}

	ASSERT_VALID (m_pWndParentToolbar);

	int nIndex = pMenuBar->ButtonToIndex (pButton);
	if (nIndex < 0)
	{
		return FALSE;
	}

	if (m_lstInvisibleButtons.GetCount ()  > 0 )
	{
		CBCGPToolbarButton* pButtonHead = (CBCGPToolbarButton*)m_lstInvisibleButtons.GetHead ();
		if (pButtonHead->m_nStyle & TBBS_SEPARATOR)
		{
			nIndex++;
		}
	}

	POSITION pos = m_lstInvisibleButtons.FindIndex (nIndex);
	if (pos == NULL)
	{
		return FALSE;
	}

	CBCGPToolbarButton* pToolbarButton = 
		(CBCGPToolbarButton*) m_lstInvisibleButtons.GetAt (pos);
	ASSERT_VALID (pToolbarButton);

	UINT nIDCmd = pToolbarButton->m_nID;

	if (!m_pWndParentToolbar->OnSendCommand (pToolbarButton) &&
		nIDCmd != 0 && nIDCmd != (UINT) -1)
	{
		CBCGPToolBar::AddCommandUsage (nIDCmd);

		if (!pToolbarButton->OnClickUp () && 
			(g_pUserToolsManager == NULL ||
			!g_pUserToolsManager->InvokeTool (nIDCmd)))
		{
			m_pWndParentToolbar->GetOwner()->PostMessage (WM_COMMAND, nIDCmd);    // send command
		}
	}

	return TRUE;
}
