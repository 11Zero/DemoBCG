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

// BCGToolbarEditBoxButton.cpp: implementation of the CBCGPToolbarEditBoxButton class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BCGPToolbar.h"
#include "BCGPToolbarEditBoxButton.h"
#include "BCGPVisualManager.h"
#include "trackmouse.h"
#include "BCGPContextMenuManager.h"
#include "BCGPWorkspace.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

extern CBCGPWorkspace* g_pWorkspace;

IMPLEMENT_SERIAL(CBCGPToolbarEditBoxButton, CBCGPToolbarButton, 1)

static const int iDefaultSize = 150;
static const int iHorzMargin = 3;
static const int iVertMargin = 1;

BOOL CBCGPToolbarEditBoxButton::m_bFlat = TRUE;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGPToolbarEditBoxButton::CBCGPToolbarEditBoxButton()
{
	m_dwStyle = WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL;
	m_iWidth = iDefaultSize;

	Initialize ();
}
//**************************************************************************************
CBCGPToolbarEditBoxButton::CBCGPToolbarEditBoxButton (UINT uiId,
			int iImage,
			DWORD dwStyle,
			int iWidth) :
			CBCGPToolbarButton (uiId, iImage)
{
	m_dwStyle = dwStyle | WS_CHILD | WS_VISIBLE;
	m_iWidth = (iWidth == 0) ? iDefaultSize : iWidth;

	Initialize ();
}
//**************************************************************************************
void CBCGPToolbarEditBoxButton::Initialize ()
{
	m_pWndEdit = NULL;
	m_bHorz = TRUE;
	m_bChangingText = FALSE;
	m_bIsHotEdit = FALSE;
	m_uiMenuResID = 0;
	m_bCalculator = FALSE;
	m_clrPrompt = (COLORREF)-1;
}
//**************************************************************************************
CBCGPToolbarEditBoxButton::~CBCGPToolbarEditBoxButton()
{
	if (m_pWndEdit != NULL)
	{
		m_pWndEdit->DestroyWindow ();
		delete m_pWndEdit;
	}
}
//**************************************************************************************
void CBCGPToolbarEditBoxButton::CopyFrom (const CBCGPToolbarButton& s)
{
	CBCGPToolbarButton::CopyFrom (s);

	const CBCGPToolbarEditBoxButton& src = (const CBCGPToolbarEditBoxButton&) s;

	m_dwStyle = src.m_dwStyle;
	m_iWidth = src.m_iWidth;
	m_strContents = src.m_strContents;
	m_uiMenuResID = src.m_uiMenuResID;
	m_bCalculator = src.m_bCalculator;
	m_strPrompt = src.m_strPrompt;
	m_clrPrompt = src.m_clrPrompt;
}
//**************************************************************************************
void CBCGPToolbarEditBoxButton::Serialize (CArchive& ar)
{
	CBCGPToolbarButton::Serialize (ar);

	if (ar.IsLoading ())
	{
		ar >> m_iWidth;
		m_rect.right = m_rect.left + m_iWidth;
		ar >> m_dwStyle;
		ar >> m_strContents;

		if (g_pWorkspace != NULL && g_pWorkspace->GetDataVersion () >= 0x60410 && g_pWorkspace->GetDataVersion () != 0x70000)
		{
				ar >> m_uiMenuResID;
		}

		if (g_pWorkspace != NULL && g_pWorkspace->GetDataVersion () >= 0x60720 && g_pWorkspace->GetDataVersion () != 0x70000)
		{
			ar >> m_bCalculator;
		}

		if (g_pWorkspace != NULL && g_pWorkspace->GetDataVersionMajor() > 20)
		{
			ar >> m_strPrompt;
			ar >> m_clrPrompt;
		}
	}
	else
	{
		ar << m_iWidth;
		ar << m_dwStyle;

		if (m_pWndEdit != NULL)
		{
			m_pWndEdit->GetWindowText(m_strContents);
		}
		else
		{
			m_strContents.Empty();
		}

		ar << m_strContents;
		ar << m_uiMenuResID;
		ar << m_bCalculator;
		ar << m_strPrompt;
		ar << (long)m_clrPrompt;
	}
}
//**************************************************************************************
SIZE CBCGPToolbarEditBoxButton::OnCalculateSize (CDC* pDC, const CSize& sizeDefault, BOOL bHorz)
{
	if (!IsVisible())
	{
		if (m_pWndEdit->GetSafeHwnd () != NULL)
		{
			m_pWndEdit->ShowWindow (SW_HIDE);
		}

		OnShowEditbox (FALSE);
		return CSize (0,0);
	}

	m_bHorz = bHorz;

	if (bHorz)
	{
		if (m_pWndEdit->GetSafeHwnd () != NULL && !m_bIsHidden)
		{
			m_pWndEdit->ShowWindow (SW_SHOWNOACTIVATE);
			OnShowEditbox (TRUE);
		}

		if (m_bTextBelow && !m_strText.IsEmpty())
		{
			CRect rectText (0, 0, 
				m_iWidth, sizeDefault.cy);
			pDC->DrawText (	m_strText, rectText, 
							DT_CENTER | DT_CALCRECT | DT_WORDBREAK);
			m_sizeText = rectText.Size ();
		}
		else
			m_sizeText = CSize(0,0);

		return CSize (m_iWidth, sizeDefault.cy + m_sizeText.cy);
	}
	else
	{
		if (m_pWndEdit->GetSafeHwnd () != NULL)
		{
			m_pWndEdit->ShowWindow (SW_HIDE);
			OnShowEditbox (FALSE);
		}

		m_sizeText = CSize(0,0);

		return CBCGPToolbarButton::OnCalculateSize (pDC, sizeDefault, bHorz);
	}
}
//**************************************************************************************
void CBCGPToolbarEditBoxButton::OnMove ()
{
	if (m_pWndEdit->GetSafeHwnd () == NULL ||
		(m_pWndEdit->GetStyle () & WS_VISIBLE) == 0)
	{
		return;
	}

	int cy = globalData.GetTextHeight();

	if (!m_bFlat)
	{
		cy += 4;
	}

	int yOffset = max (0, (m_rect.Height () - m_sizeText.cy - cy) / 2);

	m_pWndEdit->SetWindowPos (NULL, 
		m_rect.left + iHorzMargin, 
		m_rect.top + yOffset,
		m_rect.Width () - 2 * iHorzMargin, 
		cy,
		SWP_NOZORDER | SWP_NOACTIVATE);

	m_pWndEdit->SetSel(-1, 0);
}
//**************************************************************************************
void CBCGPToolbarEditBoxButton::OnSize (int iSize)
{
	m_iWidth = iSize;
	m_rect.right = m_rect.left + m_iWidth;

	OnMove ();
}
//**************************************************************************************
void CBCGPToolbarEditBoxButton::OnChangeParentWnd (CWnd* pWndParent)
{
	CBCGPToolbarButton::OnChangeParentWnd (pWndParent);

	if (m_pWndEdit->GetSafeHwnd () != NULL)
	{
		CWnd* pWndParentCurr = m_pWndEdit->GetParent ();
		ASSERT (pWndParentCurr != NULL);

		if (pWndParent != NULL &&
			pWndParentCurr->GetSafeHwnd () == pWndParent->GetSafeHwnd ())
		{
			return;
		}
		
		m_pWndEdit->GetWindowText(m_strContents);

		m_pWndEdit->DestroyWindow ();
		delete m_pWndEdit;
		m_pWndEdit = NULL;
	}

	if (pWndParent == NULL || pWndParent->GetSafeHwnd () == NULL)
	{
		return;
	}

	CRect rect = m_rect;
	rect.DeflateRect (iHorzMargin, iVertMargin);
	rect.bottom = rect.top + globalData.GetTextHeight ();

	if ((m_pWndEdit = CreateEdit (pWndParent, rect)) == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	ASSERT_VALID (m_pWndEdit);

	OnMove ();
	m_pWndEdit->SetFont (&globalData.fontRegular);

	CString sText;
	m_pWndEdit->GetWindowText(sText);
	if (sText.IsEmpty())
	{
		m_bChangingText = TRUE;
		m_pWndEdit->SetWindowText(m_strContents);
		m_bChangingText = FALSE;
	}
	else
	{
		m_strContents = sText;
	}
}
//**************************************************************************************
BOOL CBCGPToolbarEditBoxButton::NotifyCommand (int iNotifyCode)
{
	if (m_pWndEdit->GetSafeHwnd () == NULL)
	{
		return FALSE;
	}

	switch (iNotifyCode)
	{
		case EN_UPDATE:
		{
			m_pWndEdit->GetWindowText(m_strContents);

			//------------------------------------------------------
			// Try set selection in ALL editboxes with the same ID:
			//------------------------------------------------------
			CObList listButtons;
			if (CBCGPToolBar::GetCommandButtons (m_nID, listButtons) > 0)
			{
				for (POSITION posCombo = listButtons.GetHeadPosition (); posCombo != NULL;)
				{
					CBCGPToolbarEditBoxButton* pEdit = 
						DYNAMIC_DOWNCAST (CBCGPToolbarEditBoxButton, listButtons.GetNext (posCombo));

					if ((pEdit != NULL) && (pEdit != this))
					{
						pEdit->SetContents(m_strContents);
					}
				}
			}
		}

		return !m_bChangingText;
	}

	return FALSE;
}
//**************************************************************************************
void CBCGPToolbarEditBoxButton::OnAddToCustomizePage ()
{
	CObList listButtons;	// Existing buttons with the same command ID

	if (CBCGPToolBar::GetCommandButtons (m_nID, listButtons) == 0)
	{
		return;
	}

	CBCGPToolbarEditBoxButton* pOther = 
		(CBCGPToolbarEditBoxButton*) listButtons.GetHead ();
	ASSERT_VALID (pOther);
	ASSERT_KINDOF (CBCGPToolbarEditBoxButton, pOther);

	CopyFrom (*pOther);
}
//**************************************************************************************
HBRUSH CBCGPToolbarEditBoxButton::OnCtlColor (CDC* pDC, UINT /*nCtlColor*/)
{
	COLORREF clrText = globalData.clrWindowText;
	COLORREF clrBk = globalData.clrWindow;

	HBRUSH hbr = CBCGPVisualManager::GetInstance ()->GetToolbarEditColors(this, clrText, clrBk);

	pDC->SetTextColor (clrText);
	pDC->SetBkColor (clrBk);

	return hbr;
}
//**************************************************************************************
void CBCGPToolbarEditBoxButton::OnDraw (CDC* pDC, const CRect& rect, CBCGPToolBarImages* pImages,
						BOOL bHorz, BOOL bCustomizeMode,
						BOOL bHighlight, BOOL bDrawBorder, 
						BOOL bGrayDisabledButtons)
{
	if (m_pWndEdit->GetSafeHwnd () == NULL ||
		(m_pWndEdit->GetStyle () & WS_VISIBLE) == 0)
	{
		CBCGPToolbarButton::OnDraw (pDC, rect, pImages,
							bHorz, bCustomizeMode,
							bHighlight, bDrawBorder, 
							bGrayDisabledButtons);
		return;
	}

	BOOL bDisabled = (bCustomizeMode && !IsEditable ()) ||
		(!bCustomizeMode && (m_nStyle & TBBS_DISABLED));
		
	CRect rectBorder;
	GetEditBorder (rectBorder);

	CBCGPVisualManager::GetInstance ()->OnDrawEditBorder (
		pDC, rectBorder, bDisabled, !m_bFlat || m_bIsHotEdit, this);

	if ((m_bTextBelow && bHorz) && !m_strText.IsEmpty())
	{
		//--------------------
		// Draw button's text:
		//--------------------
		CBCGPVisualManager::BCGBUTTON_STATE state = CBCGPVisualManager::ButtonsIsRegular;
		if (bHighlight)
		{
			state = CBCGPVisualManager::ButtonsIsHighlighted;
		}
		else if (m_nStyle & (TBBS_PRESSED | TBBS_CHECKED))
		{
			state = CBCGPVisualManager::ButtonsIsPressed;
		}

		COLORREF clrText = CBCGPVisualManager::GetInstance ()->GetToolbarButtonTextColor (
			this, state);

		COLORREF cltTextOld = pDC->SetTextColor (clrText);

		CRect rectText = rect;
		rectText.top = (rectBorder.bottom + rect.bottom - m_sizeText.cy) / 2;
		pDC->DrawText (m_strText, &rectText, DT_CENTER | DT_WORDBREAK);

		pDC->SetTextColor (cltTextOld);
	}
}
//**************************************************************************************
void CBCGPToolbarEditBoxButton::GetEditBorder (CRect& rectBorder)
{
	ASSERT (m_pWndEdit->GetSafeHwnd () != NULL);

	m_pWndEdit->GetWindowRect (rectBorder);
	m_pWndEdit->GetParent ()->ScreenToClient (rectBorder);
	rectBorder.InflateRect (1, 1);
}
//**************************************************************************************
BOOL CBCGPToolbarEditBoxButton::OnClick (CWnd* /*pWnd*/, BOOL /*bDelay*/)
{	
	return m_pWndEdit->GetSafeHwnd () != NULL &&
			(m_pWndEdit->GetStyle () & WS_VISIBLE);
}
//**************************************************************************************
int CBCGPToolbarEditBoxButton::OnDrawOnCustomizeList (
	CDC* pDC, const CRect& rect, BOOL bSelected)
{
	int iWidth = CBCGPToolbarButton::OnDrawOnCustomizeList (pDC, rect, bSelected) + 10;

	//------------------------------
	// Simulate editbox appearance:
	//------------------------------
	CRect rectEdit = rect;
	int nEditWidth = max (8, rect.Width () - iWidth);

	rectEdit.left = rectEdit.right - nEditWidth;
	rectEdit.DeflateRect (2, 2);

	pDC->FillRect (rectEdit, &globalData.brWindow);
	pDC->Draw3dRect (rectEdit, globalData.clrBarShadow, globalData.clrBarShadow);

	return rect.Width ();
}
//********************************************************************************************
CEdit* CBCGPToolbarEditBoxButton::CreateEdit (CWnd* pWndParent, const CRect& rect)
{
	ASSERT_VALID (this);

	CBCGPToolbarEditCtrl* pWndEdit = new CBCGPToolbarEditCtrl (*this);
	if (!pWndEdit->Create (m_dwStyle, rect, pWndParent, m_nID))
	{
		delete pWndEdit;
		return NULL;
	}

	pWndEdit->m_bVisualManagerStyle = TRUE;

	if (m_bCalculator)
	{
		pWndEdit->EnableCalculatorButton ();
	}

	if (!m_strPrompt.IsEmpty())
	{
		pWndEdit->SetPrompt(m_strPrompt, m_clrPrompt, FALSE);
	}

	return pWndEdit;
}
//****************************************************************************************
void CBCGPToolbarEditBoxButton::OnShow (BOOL bShow)
{
	if (m_pWndEdit->GetSafeHwnd () != NULL)
	{
		if (bShow)
		{
			m_pWndEdit->ShowWindow (SW_SHOWNOACTIVATE);
			OnMove ();
		}
		else
		{
			m_pWndEdit->ShowWindow (SW_HIDE);
		}

		OnShowEditbox (bShow);
	}
}
//*********************************************************************************
void CBCGPToolbarEditBoxButton::SetContents (const CString& sContents)
{
	if (m_strContents == sContents)
		return;

	m_strContents = sContents;
	if (m_pWndEdit != NULL)
	{
		m_bChangingText = TRUE;
		m_pWndEdit->SetWindowText(m_strContents);
		m_bChangingText = FALSE;
	}
}
//*********************************************************************************
const CRect CBCGPToolbarEditBoxButton::GetInvalidateRect () const
{
	if ((m_bTextBelow && m_bHorz) && !m_strText.IsEmpty())
	{
		CRect rect;
		rect.left = (m_rect.left + m_rect.right - m_sizeText.cx) / 2;
		rect.right = (m_rect.left + m_rect.right + m_sizeText.cx) / 2;
		rect.top = m_rect.top;
		rect.bottom = m_rect.bottom + m_rect.top + m_sizeText.cy;
		return rect;
	}
	else
		return m_rect;
}
//*********************************************************************************
CBCGPToolbarEditBoxButton* CBCGPToolbarEditBoxButton::GetByCmd (UINT uiCmd)
{
	CBCGPToolbarEditBoxButton* pSrcEdit = NULL;

	CObList listButtons;
	if (CBCGPToolBar::GetCommandButtons (uiCmd, listButtons) > 0)
	{
		for (POSITION posEdit= listButtons.GetHeadPosition (); pSrcEdit == NULL && posEdit != NULL;)
		{
			CBCGPToolbarEditBoxButton* pEdit= DYNAMIC_DOWNCAST (CBCGPToolbarEditBoxButton, listButtons.GetNext (posEdit));
			ASSERT (pEdit != NULL);

			pSrcEdit = pEdit;
		}
	}

	return pSrcEdit;
}
//*********************************************************************************
BOOL CBCGPToolbarEditBoxButton::SetContentsAll (UINT uiCmd, const CString& strContents)
{
	CBCGPToolbarEditBoxButton* pSrcEdit = GetByCmd (uiCmd);

	if (pSrcEdit)
	{
		pSrcEdit->SetContents (strContents);
	}

	return pSrcEdit != NULL;
}
//*********************************************************************************
CString CBCGPToolbarEditBoxButton::GetContentsAll (UINT uiCmd)
{
	CBCGPToolbarEditBoxButton* pSrcEdit = GetByCmd (uiCmd);
	CString str;

	if (pSrcEdit)
	{
		pSrcEdit->m_pWndEdit->GetWindowText (str);
	}

	return str;
}
//*********************************************************************************
void CBCGPToolbarEditBoxButton::SetStyle (UINT nStyle)
{
	CBCGPToolbarButton::SetStyle (nStyle);

	if (m_pWndEdit != NULL && m_pWndEdit->GetSafeHwnd () != NULL)
	{
		BOOL bDisabled = (CBCGPToolBar::IsCustomizeMode () && !IsEditable ()) ||
			(!CBCGPToolBar::IsCustomizeMode () && (m_nStyle & TBBS_DISABLED));

		m_pWndEdit->EnableWindow (!bDisabled);
	}
}
//**************************************************************************************
void CBCGPToolbarEditBoxButton::SetHotEdit (BOOL bHot)
{
	if (m_bIsHotEdit != bHot)
	{
		m_bIsHotEdit = bHot;

		if (m_pWndEdit->GetParent () != NULL)
		{
			CRect rect = m_rect;

			m_pWndEdit->GetParent ()->InvalidateRect (m_rect);
			m_pWndEdit->GetParent ()->UpdateWindow ();
		}
	}
}
//********************************************************************************
BOOL CBCGPToolbarEditBoxButton::OnUpdateToolTip (CWnd* /*pWndParent*/, int /*iButtonIndex*/,
												 CToolTipCtrl& wndToolTip, CString& str)
{
	CEdit* pEdit = GetEditBox ();

	if ((pEdit != NULL) && (::IsWindow (pEdit->GetSafeHwnd ())))
	{
		CString strTips;

		if (OnGetCustomToolTipText (strTips))
		{
			wndToolTip.AddTool (pEdit, strTips, NULL, 0);
		}
		else
		{
			wndToolTip.AddTool (pEdit, str, NULL, 0);
		}

		return TRUE;
	}

	return FALSE;
}
//*****************************************************************************************
void CBCGPToolbarEditBoxButton::OnGlobalFontsChanged()
{
	CBCGPToolbarButton::OnGlobalFontsChanged ();

	if (m_pWndEdit->GetSafeHwnd () != NULL)
	{
		m_pWndEdit->SetFont (&globalData.fontRegular);
	}
}
//******************************************************************************************
BOOL CBCGPToolbarEditBoxButton::SetACCData (CWnd* pParent, CBCGPAccessibilityData& data)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pParent);

	if (!CBCGPToolbarButton::SetACCData (pParent, data))
	{
		return FALSE;
	}

	data.m_nAccRole = ROLE_SYSTEM_TEXT;
	data.m_bAccState = STATE_SYSTEM_FOCUSABLE;
	if (HasFocus ())
	{
		data.m_bAccState |= STATE_SYSTEM_FOCUSED;
	}
	data.m_strAccDefAction = L"Edit";
	data.m_strAccValue = m_strText;
		 
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPToolbarEditCtrl

CBCGPToolbarEditCtrl::CBCGPToolbarEditCtrl(CBCGPToolbarEditBoxButton& edit) :
	m_buttonEdit (edit)
{
	m_bTracked = FALSE;
}

CBCGPToolbarEditCtrl::~CBCGPToolbarEditCtrl()
{
}

BEGIN_MESSAGE_MAP(CBCGPToolbarEditCtrl, CBCGPEdit)
	//{{AFX_MSG_MAP(CBCGPToolbarEditCtrl)
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_MOUSEMOVE()
	ON_WM_CONTEXTMENU()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGPToolbarEditCtrl message handlers

BOOL CBCGPToolbarEditCtrl::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message == WM_KEYDOWN)
	{
		switch (pMsg->wParam)
		{
		case VK_TAB:
			if (GetParent () != NULL)
			{
				ASSERT_VALID (GetParent ());
				GetParent ()->GetNextDlgTabItem (this)->SetFocus ();
				return TRUE;
			}
			break;

		case VK_ESCAPE:
			if (GetTopLevelFrame () != NULL)
			{
				GetTopLevelFrame ()->SetFocus ();
				return TRUE;
			}
			
			break;
		}

		if (GetFocus () == this && GetKeyState (VK_CONTROL) & 0x8000 )
		{
			switch (pMsg->wParam)
			{
			case 'V':
				Paste();
				return TRUE;

			case 'C':
				Copy();
				return TRUE;

			case 'X':
				Cut();
				return TRUE;

			case 'Z':
				Undo();
				return TRUE;

			case VK_DELETE:
				Clear();
				return TRUE;
			}
		}
	}

	return CBCGPEdit::PreTranslateMessage(pMsg);
}
//*************************************************************************************
void CBCGPToolbarEditCtrl::OnSetFocus(CWnd* pOldWnd) 
{
	CBCGPEdit::OnSetFocus(pOldWnd);
	m_buttonEdit.SetHotEdit (TRUE);
}
//*************************************************************************************
void CBCGPToolbarEditCtrl::OnKillFocus(CWnd* pNewWnd) 
{
	CBCGPEdit::OnKillFocus(pNewWnd);
	m_buttonEdit.SetHotEdit (FALSE);
}
//*************************************************************************************
void CBCGPToolbarEditCtrl::OnMouseMove(UINT nFlags, CPoint point) 
{
	CBCGPEdit::OnMouseMove(nFlags, point);
	m_buttonEdit.SetHotEdit (TRUE);

	if (!m_bTracked)
	{
		m_bTracked = TRUE;
		
		TRACKMOUSEEVENT trackmouseevent;
		trackmouseevent.cbSize = sizeof(trackmouseevent);
		trackmouseevent.dwFlags = TME_LEAVE;
		trackmouseevent.hwndTrack = GetSafeHwnd();
		trackmouseevent.dwHoverTime = HOVER_DEFAULT;
		::BCGPTrackMouse (&trackmouseevent);
	}
}
//*****************************************************************************************
LRESULT CBCGPToolbarEditCtrl::OnMouseLeave(WPARAM,LPARAM)
{
	m_bTracked = FALSE;

	if (CWnd::GetFocus () != this)
	{
		m_buttonEdit.SetHotEdit (FALSE);
	}

	return 0;
}

void CBCGPToolbarEditCtrl::OnContextMenu(CWnd* pWnd, CPoint point) 
{

		if(m_buttonEdit.m_uiMenuResID != 0)
		{

			CWnd* pWndParent = pWnd->GetParent();

			HINSTANCE hInst = AfxFindResourceHandle (
			MAKEINTRESOURCE (m_buttonEdit.m_uiMenuResID), RT_MENU);

			if(hInst == NULL)
			{
				CBCGPEdit::OnContextMenu(pWnd, point) ;
				return;

			}
			
			HMENU hMenu = ::LoadMenu (hInst, MAKEINTRESOURCE (m_buttonEdit.m_uiMenuResID));

			if(hMenu == NULL)
			{
				CBCGPEdit::OnContextMenu(pWnd, point) ;
				return;
			}

			HMENU hPopupMenu = ::GetSubMenu(hMenu, 0);

			if(hPopupMenu == NULL)
			{
				CBCGPEdit::OnContextMenu(pWnd, point) ;
				return;
			}


				if (g_pContextMenuManager != NULL)
				{
					g_pContextMenuManager->ShowPopupMenu (hPopupMenu, 
											point.x, point.y, pWndParent);

				}
				else
				{
					 ::TrackPopupMenu (hPopupMenu, 
						TPM_CENTERALIGN | TPM_LEFTBUTTON, 
						point.x, point.y, 0, pWndParent->GetSafeHwnd (), NULL);
				}
		}else
		{
			CBCGPEdit::OnContextMenu(pWnd, point) ;
		}
}
