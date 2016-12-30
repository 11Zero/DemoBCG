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
// BCGPCustomizeMenuButton.cpp: implementation of the CBCGPCustomizeMenuButton class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "bcgprores.h"
#include "bcgcbpro.h"
#include "BCGPCustomizeMenuButton.h"
#include "CustomizeButton.h"
#include "BCGPFrameWnd.h"
#include "BCGPMDIFrameWnd.h"
#include "BCGPKeyboardManager.h"
#include "BCGPLocalResource.h"
#include "afxtempl.h"
#include "BCGPVisualManager.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


IMPLEMENT_DYNCREATE(CBCGPCustomizeMenuButton, CBCGPToolbarMenuButton)

CMap<UINT, UINT, int, int>	 CBCGPCustomizeMenuButton::m_mapPresentIDs;
CBCGPToolBar* CBCGPCustomizeMenuButton::m_pWndToolBar = NULL;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

BOOL CBCGPCustomizeMenuButton::m_bRecentlyUsedOld = FALSE;


CBCGPCustomizeMenuButton::CBCGPCustomizeMenuButton()
{
	
}
//****************************************************************************************
CBCGPCustomizeMenuButton::~CBCGPCustomizeMenuButton()
{
	
}
//****************************************************************************************
CBCGPCustomizeMenuButton::CBCGPCustomizeMenuButton(UINT uiID,HMENU hMenu,int iImage,LPCTSTR lpszText,BOOL bUserButton):
	CBCGPToolbarMenuButton (uiID, hMenu/* HMENU */, iImage /*iImage*/, lpszText, bUserButton)
{
	m_uiIndex = (UINT)-1;
	bSeparator = FALSE;
	m_bAddSpr = FALSE;
	m_bIsEnabled = TRUE;
	m_bBrothersBtn = FALSE;
}
//****************************************************************************************
void CBCGPCustomizeMenuButton::SetItemIndex(UINT uiIndex, BOOL bExist, BOOL bAddSpr)
{
	m_uiIndex = uiIndex;
	m_bExist = bExist;
	m_bAddSpr = bAddSpr;
	
	if((uiIndex != ID_BCGBARRES_TOOLBAR_RESET_PROMT)
		&& !bSeparator && bExist)
	{
		CBCGPToolbarButton* pBtn = m_pWndToolBar->GetButton(uiIndex);
		m_bShow = pBtn->IsVisible();
		
	}else
	{
		m_bShow = FALSE;

		if (m_uiIndex == ID_BCGBARRES_TOOLBAR_RESET_PROMT && 
			m_pWndToolBar->IsUserDefined ())
		{
			m_bIsEnabled = FALSE;
		}
	}
}
//****************************************************************************************
void CBCGPCustomizeMenuButton::CopyFrom (const CBCGPToolbarButton& s)
{
	CBCGPToolbarButton::CopyFrom (s);
	const CBCGPCustomizeMenuButton& src = (const CBCGPCustomizeMenuButton&) s;
	
	m_uiIndex      =   src.m_uiIndex;
	m_bShow        =   src.m_bShow;
	m_pWndToolBar  =   src.m_pWndToolBar;
	bSeparator     =   src.bSeparator;
	m_bExist       =   src.m_bExist;
	m_bAddSpr      =   src.m_bAddSpr;
	m_bIsEnabled   =   src.m_bIsEnabled;
	m_bBrothersBtn =   src.m_bBrothersBtn;	
}
//****************************************************************************************
SIZE CBCGPCustomizeMenuButton::OnCalculateSize (CDC* pDC, const CSize& sizeDefault, BOOL bHorz)
{
	if (bSeparator)
	{
		return CSize(0,  4);
	}
	
	if (m_bBrothersBtn)
	{
		return CBCGPToolbarMenuButton::OnCalculateSize(pDC, sizeDefault, bHorz);
	}
	
	//-----------------------------
	//  Try to Find Buttons Text
	//-----------------------------
	if (m_strText.IsEmpty ())
	{
		//-------------------------------------------
		// Try to find the command name in resources:
		//-------------------------------------------
		CString strMessage;
		int iOffset;
		if (strMessage.LoadString (m_nID) &&
			(iOffset = strMessage.Find (_T('\n'))) != -1)
		{
			m_strText = strMessage.Mid (iOffset + 1);
		}
	}
	else
	{
		// m_strText.Remove (_T('&'));
		
		//----------------------------------------
		// Remove trailing label (ex.:"\tCtrl+S"):
		//----------------------------------------
		int iOffset = m_strText.Find (_T('\t'));
		if (iOffset != -1)
		{
			m_strText = m_strText.Left (iOffset);
		}
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
		GetKeyboardAccelerator(strAccel);

		{
			m_strText += _T('\t');
			m_strText += strAccel;
		}
	}
	
	
	
	int nTolalWidth = m_strText.GetLength();
	TEXTMETRIC tm;
	pDC->GetTextMetrics (&tm);
	nTolalWidth *= tm.tmAveCharWidth;
	CSize sizeImage = CBCGPToolBar::GetMenuButtonSize();
	nTolalWidth += 2*sizeImage.cx;	
	nTolalWidth += 3*CBCGPVisualManager::GetInstance ()->GetMenuImageMargin () + 50;
	
	CSize sizeStandard = CBCGPToolbarMenuButton::OnCalculateSize(pDC, sizeDefault, bHorz);
	
	int nTotalHeight = sizeStandard.cy + 2;
	
	if (!m_bMenuMode)
	{
		nTotalHeight += CBCGPVisualManager::GetInstance ()->GetMenuImageMargin ();
	}
	
	return CSize(nTolalWidth,  nTotalHeight);
}
//****************************************************************************************
BOOL CBCGPCustomizeMenuButton::OnClickMenuItem()
{
	if (bSeparator || !m_bIsEnabled)
	{
		return TRUE;
	}
	
	CBCGPPopupMenuBar* pMenuBar = (CBCGPPopupMenuBar*)m_pWndParent;
	int nIndex = pMenuBar->ButtonToIndex(this);
	if (nIndex !=-1)
	{
		if (pMenuBar->m_iHighlighted != nIndex)
		{
			pMenuBar->m_iHighlighted = nIndex;
			pMenuBar->InvalidateRect (this->Rect ());
		}
	}

	if (m_bBrothersBtn) 
	{
		if (m_pWndToolBar->IsOneRowWithBrother ())
		{
			m_pWndToolBar->SetTwoRowsWithBrother ();
		}
		else
		{
			m_pWndToolBar->SetOneRowWithBrother ();
		}

		return FALSE;
	}
	
	if (m_uiIndex == ID_BCGBARRES_TOOLBAR_RESET_PROMT) // reset pressed
	{
		//load default toolbar
		m_pWndToolBar->PostMessage (BCGM_RESETRPROMPT);
		return FALSE;	
	}
	
	if (!m_bExist)
	{	
		const CObList& lstOrignButtons = m_pWndToolBar->GetOrigResetButtons ();
		
		POSITION pos = lstOrignButtons.FindIndex (m_uiIndex);
		CBCGPToolbarButton* pButton = (CBCGPToolbarButton*)lstOrignButtons.GetAt (pos);
		if (pButton == NULL)
		{
			return TRUE;
		}
		
		UINT nIndex = m_pWndToolBar->InsertButton(*pButton, m_uiIndex);
		
		if (nIndex == -1)
		{
			nIndex = m_pWndToolBar->InsertButton(*pButton);	
		}
		else
		{
			CBCGPPopupMenuBar* pMenuBar = (CBCGPPopupMenuBar*)m_pWndParent;
			
			int nCount = pMenuBar->GetCount ();
			
			for (int i = 0; i < nCount; i++)
			{
				CBCGPCustomizeMenuButton* pBtn = DYNAMIC_DOWNCAST(CBCGPCustomizeMenuButton, pMenuBar->GetButton(i));
				if (pBtn != NULL &&
					(pBtn->m_uiIndex >= nIndex) && 
					(pBtn->m_uiIndex != ID_BCGBARRES_TOOLBAR_RESET_PROMT))
				{
					if (pBtn->m_bExist)
					{
						pBtn->m_uiIndex += 1; 
					}
				}
			}
		}

		m_uiIndex = nIndex;
		
		if (m_bAddSpr) 
		{
			if (nIndex < (UINT)m_pWndToolBar->GetCount ())
			{
				CBCGPToolbarButton* pBtn = m_pWndToolBar->GetButton (nIndex+1);
				if (!(pBtn->m_nStyle & TBBS_SEPARATOR))
				{
					m_pWndToolBar->InsertSeparator ();
				}	
			}
			else
			{
				m_pWndToolBar->InsertSeparator ();
			}
		}

		m_pWndToolBar->AdjustLayout ();
		m_pWndToolBar->AdjustSizeImmediate ();
		UpdateCustomizeButton ();
		
		m_bExist = TRUE;
		m_bShow = TRUE;
		CBCGPPopupMenuBar* pMenuBar = (CBCGPPopupMenuBar*)m_pWndParent;
		pMenuBar->Invalidate ();

		return TRUE;	
	}
	
	CBCGPToolbarButton* pBtn = m_pWndToolBar->GetButton (m_uiIndex);
	BOOL bVisible = pBtn->IsVisible ();
	pBtn->SetVisible (!bVisible);
	m_bShow = !bVisible;
	
	//-------------------------------------
	//  Make next Separator the same state
	//-------------------------------------
	int nNext = m_uiIndex + 1;
	if (nNext < m_pWndToolBar->GetCount ())
	{
		CBCGPToolbarButton* pBtnNext = m_pWndToolBar->GetButton (nNext);
		if (pBtnNext->m_nStyle & TBBS_SEPARATOR)
		{
			pBtnNext->SetVisible (!bVisible);	
		}	
	}

	CBCGPPopupMenu* pCustomizeMenu = NULL;

	for (CBCGPPopupMenu* pMenu = 
		DYNAMIC_DOWNCAST (CBCGPPopupMenu, pMenuBar->GetParent ());
		pMenu != NULL; pMenu = pMenu->GetParentPopupMenu ())
	{
		pCustomizeMenu = pMenu;
	}

	if (pCustomizeMenu != NULL)
	{
		pCustomizeMenu->ShowWindow (SW_HIDE);
	}

	m_pWndToolBar->AdjustLayout();
	m_pWndToolBar->AdjustSizeImmediate ();
	UpdateCustomizeButton();
	pMenuBar->Invalidate();
	
	if (pCustomizeMenu != NULL)
	{
		pCustomizeMenu->ShowWindow (SW_SHOWNOACTIVATE);

		CRect rectScreen;
		pCustomizeMenu->GetWindowRect (&rectScreen);
		CBCGPPopupMenu::UpdateAllShadows (rectScreen);
	}
	
	return TRUE;
}
//****************************************************************************************
void CBCGPCustomizeMenuButton::OnDraw (CDC* pDC, const CRect& rect, CBCGPToolBarImages* pImages,
									   BOOL bHorz, BOOL bCustomizeMode, BOOL bHighlight,
									   BOOL bDrawBorder, BOOL bGrayDisabledButtons)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);
	
	//----------------
	// Draw separator:
	//----------------
	if (bSeparator)
	{
		CRect rcSeparator(rect);
		rcSeparator.left = 2*CBCGPToolBar::GetMenuImageSize ().cx + 
			CBCGPVisualManager::GetInstance ()->GetMenuImageMargin ();
		
		CBCGPPopupMenuBar* pMenuBar = (CBCGPPopupMenuBar*)m_pWndParent;
		CBCGPVisualManager::GetInstance ()->OnDrawSeparator (pDC, pMenuBar, rcSeparator, FALSE);
		
		return;
	}

	if (m_bBrothersBtn)
	{
		CBCGPToolbarMenuButton::OnDraw (pDC, rect, NULL,
									   bHorz, bCustomizeMode, bHighlight,
									   bDrawBorder, bGrayDisabledButtons);
		return;
	}

	CRect rectItem = rect;
	rectItem.bottom--;
	
	if (m_bIsEnabled)
	{	  
		if (m_bShow && bHighlight)
		{		  
			SetStyle (TBBS_BUTTON|TBBS_CHECKED);
		}
		else
		{	  
			SetStyle (TBBS_BUTTON);
		 }		  
	}
	else
	{
		SetStyle (TBBS_DISABLED);
		bGrayDisabledButtons = TRUE;
		bHighlight = FALSE;
	}
		  
	BOOL bIsResetItem = m_uiIndex == ID_BCGBARRES_TOOLBAR_RESET_PROMT;

	if (bIsResetItem)
	{
		m_bImage = FALSE;
		m_iImage = -1;
	}

	//-----------------
	//	Highlight item:
	//-----------------
	if (bHighlight && m_bIsEnabled)
	{
		CRect rcHighlight = rectItem;
		rcHighlight.left += 2;
		rcHighlight.right--;
		
		if (!CBCGPVisualManager::GetInstance ()->IsHighlightWholeMenuItem () &&
			!bIsResetItem)
		{
			rcHighlight.left += 2 * CBCGPToolBar::GetMenuImageSize ().cx + 
				5 * CBCGPVisualManager::GetInstance ()->GetMenuImageMargin ();
		}
		
		COLORREF clrText;
		CBCGPVisualManager::GetInstance ()->OnHighlightMenuItem (pDC, this, rcHighlight, clrText);
	}
	
	//---------------
	// Draw checkbox:
	//---------------
	CSize sizeMenuImage = CBCGPToolBar::GetMenuImageSize ();
	
	CRect rectCheck = rectItem;
	rectCheck.left += CBCGPVisualManager::GetInstance ()->GetMenuImageMargin () + 1;
	rectCheck.right = rectCheck.left + sizeMenuImage.cx + 
		CBCGPVisualManager::GetInstance ()->GetMenuImageMargin () + 2;
	rectCheck.bottom--;
	
	DrawCheckBox (pDC, rectCheck, bHighlight);
	
	if (bHighlight && !(m_nStyle & TBBS_DISABLED) && !bIsResetItem)
	{
		SetStyle (TBBS_BUTTON);
	}
	
	//------------------
	// Draw icon + text:
	//------------------
	CRect rectStdMenu = rectItem;
	rectStdMenu.left = rectCheck.right;
	
	DrawMenuItem (pDC, rectStdMenu, pImages, bCustomizeMode, 
		bHighlight, bGrayDisabledButtons, TRUE);
}
//****************************************************************************************
CString CBCGPCustomizeMenuButton::SearchCommandText(CMenu* pMenu, UINT in_uiCmd)
{
	ASSERT (pMenu != NULL);
	
	int iCount = (int) pMenu->GetMenuItemCount ();
	
	for (int i = 0; i < iCount; i ++)
	{
		UINT uiCmd = pMenu->GetMenuItemID (i);
		if (uiCmd == in_uiCmd)
		{
			CString strText;
			pMenu->GetMenuString (i, strText, MF_BYPOSITION);
			return strText;
		}
		
		switch (uiCmd)
		{
		case 0:		// Separator, ignore it.
			break;
			
		case -1:	// Submenu
			{
				CMenu* pSubMenu = pMenu->GetSubMenu (i);
				
				CString strText = SearchCommandText (pSubMenu, in_uiCmd);
				if(strText != _T("")) return strText;
			}
			break;
			
		}//end switch
	}//end for
	
	return _T("");
}
//****************************************************************************************
void CBCGPCustomizeMenuButton::DrawCheckBox(CDC* pDC, const CRect& rect, BOOL bHighlight)
{	
	if (!m_bShow)
	{
		return;
	}

	CRect rectCheck = rect;
	rectCheck.DeflateRect (0, 1, 1, 1);

	if (!CBCGPVisualManager::GetInstance ()->IsOwnerDrawMenuCheck ())
	{
		UINT nStyle = m_nStyle;
		m_nStyle |= TBBS_CHECKED;
	
		FillInterior (pDC, rectCheck, bHighlight, TRUE);
	
		if (bHighlight && 
			CBCGPVisualManager::GetInstance ()->IsFrameMenuCheckedItems ())
		{
			m_nStyle |= TBBS_MARKED;
		}

		CBCGPVisualManager::GetInstance ()->OnDrawMenuImageRectBorder (pDC,
			this, rectCheck, CBCGPVisualManager::ButtonsIsPressed);

		m_nStyle = nStyle;
	}

	CBCGPVisualManager::GetInstance ()->OnDrawMenuCheck (pDC, this, 
		rectCheck, bHighlight, FALSE);
}
//****************************************************************************************
void CBCGPCustomizeMenuButton::UpdateCustomizeButton()
{
	ASSERT_VALID (m_pWndToolBar);

	if (m_pWndToolBar->GetParent ()->GetSafeHwnd () != NULL)
	{
		m_pWndToolBar->GetParent ()->RedrawWindow (
			NULL, NULL, 
			RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_FRAME);
	}

	m_pWndToolBar->RedrawCustomizeButton ();
}
//****************************************************************************************
BOOL CBCGPCustomizeMenuButton::IsCommandExist(UINT uiCmdId)
{
	int nTmp = 0;
	return m_mapPresentIDs.Lookup(uiCmdId, nTmp);
}
