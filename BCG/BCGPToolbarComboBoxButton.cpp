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

// BCGToolbarComboBoxButton.cpp: implementation of the CBCGPToolbarComboBoxButton class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BCGPToolbar.h"
#include "BCGGlobals.h"
#include "BCGPToolbarComboBoxButton.h"
#include "BCGPToolbarMenuButton.h"
#include "MenuImages.h"
#include "BCGPWorkspace.h"
#include "trackmouse.h"
#include "BCGPVisualManager.h"
#include "BCGPContextMenuManager.h"
#include "BCGPDrawManager.h"
#include "BCGPComboBox.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

extern CBCGPWorkspace* g_pWorkspace;

IMPLEMENT_SERIAL(CBCGPToolbarComboBoxButton, CBCGPToolbarButton, 1)

static const int iDefaultComboHeight = 150;
static const int iDefaultSize = 150;
static const int iHorzMargin = 1;

BOOL CBCGPToolbarComboBoxButton::m_bFlat = TRUE;
BOOL CBCGPToolbarComboBoxButton::m_bCenterVert = FALSE;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGPToolbarComboBoxButton::CBCGPToolbarComboBoxButton()
{
	m_dwStyle = WS_CHILD | WS_VISIBLE | CBS_NOINTEGRALHEIGHT | CBS_DROPDOWNLIST | WS_VSCROLL;
	m_iWidth = iDefaultSize;

	Initialize ();
}
//**************************************************************************************
CBCGPToolbarComboBoxButton::CBCGPToolbarComboBoxButton (UINT uiId,
			int iImage,
			DWORD dwStyle,
			int iWidth) :
			CBCGPToolbarButton (uiId, iImage)
{
	m_dwStyle = dwStyle | WS_CHILD | WS_VISIBLE | WS_VSCROLL;
	m_iWidth = (iWidth == 0) ? iDefaultSize : iWidth;

	Initialize ();
}
//**************************************************************************************
void CBCGPToolbarComboBoxButton::Initialize ()
{
	m_iSelIndex = -1;
	m_pWndCombo = NULL;
	m_pWndEdit = NULL;
	m_bHorz = TRUE;
	m_rectCombo.SetRectEmpty ();
	m_rectButton.SetRectEmpty ();
	m_nDropDownHeight = iDefaultComboHeight;
	m_bIsHotEdit = FALSE;
	m_uiMenuResID = 0;
	m_bIsRibbon = FALSE;
	m_bIsRibbonFloaty = FALSE;
	m_bIsCtrl = FALSE;
	m_clrPrompt = (COLORREF)-1;
}
//**************************************************************************************
CBCGPToolbarComboBoxButton::~CBCGPToolbarComboBoxButton()
{
	if (m_pWndCombo != NULL)
	{
		m_pWndCombo->DestroyWindow ();
		delete m_pWndCombo;
	}

	if (m_pWndEdit != NULL)
	{
		m_pWndEdit->DestroyWindow ();
		delete m_pWndEdit;
	}
}
//**************************************************************************************
void CBCGPToolbarComboBoxButton::CopyFrom (const CBCGPToolbarButton& s)
{
	CBCGPToolbarButton::CopyFrom (s);
	POSITION pos;

	m_lstItems.RemoveAll ();

	const CBCGPToolbarComboBoxButton& src = (const CBCGPToolbarComboBoxButton&) s;
	for (pos = src.m_lstItems.GetHeadPosition (); pos != NULL;)
	{
		m_lstItems.AddTail (src.m_lstItems.GetNext (pos));
	}

	ClearData ();

	m_lstItemData.RemoveAll ();
	for (pos = src.m_lstItemData.GetHeadPosition (); pos != NULL;)
	{
		m_lstItemData.AddTail (src.m_lstItemData.GetNext (pos));
	}

	DuplicateData ();
	ASSERT (m_lstItemData.GetCount () == m_lstItems.GetCount ());

	m_dwStyle = src.m_dwStyle;
	m_iWidth = src.m_iWidth;
	m_iSelIndex = src.m_iSelIndex;
	m_nDropDownHeight = src.m_nDropDownHeight;
	m_uiMenuResID = src.m_uiMenuResID;

	m_bIsRibbon = src.m_bIsRibbon;
	m_bIsRibbonFloaty = src.m_bIsRibbonFloaty;
	m_bIsCtrl = src.m_bIsCtrl;

	m_strPrompt = src.m_strPrompt;
	m_clrPrompt = src.m_clrPrompt;
}
//**************************************************************************************
void CBCGPToolbarComboBoxButton::Serialize (CArchive& ar)
{
	CBCGPToolbarButton::Serialize (ar);

	if (ar.IsLoading ())
	{
		ar >> m_iWidth;
		m_rect.right = m_rect.left + m_iWidth;
		ar >> m_dwStyle;
		ar >> m_iSelIndex;
		ar >> m_strEdit;
		ar >> m_nDropDownHeight;

		if (g_pWorkspace != NULL && g_pWorkspace->GetDataVersion () >= 0x60410)
		{
			ar >> m_uiMenuResID;
		}

		m_lstItems.Serialize (ar);

		ClearData ();
		m_lstItemData.RemoveAll ();

		for (int i = 0; i < m_lstItems.GetCount (); i ++)
		{
			long lData;
			ar >> lData;
			m_lstItemData.AddTail ((DWORD_PTR) lData);
		}

		DuplicateData ();
		ASSERT (m_lstItemData.GetCount () == m_lstItems.GetCount ());

		SelectItem (m_iSelIndex);

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
		ar << m_iSelIndex;
		ar << m_strEdit;
		ar << m_nDropDownHeight;

		if (g_pWorkspace != NULL && g_pWorkspace->GetDataVersion () >= 0x60410)
		{
			ar << m_uiMenuResID;
		}

		if (m_pWndCombo != NULL)
		{
			m_lstItems.RemoveAll ();
			ClearData ();
			m_lstItemData.RemoveAll ();

			for (int i = 0; i < m_pWndCombo->GetCount (); i ++)
			{
				CString str;
				m_pWndCombo->GetLBText (i, str);

				m_lstItems.AddTail (str);
				m_lstItemData.AddTail (m_pWndCombo->GetItemData (i));
			}
		}

		m_lstItems.Serialize (ar);

		for (POSITION pos = m_lstItemData.GetHeadPosition (); pos != NULL;)
		{
			DWORD_PTR dwData = m_lstItemData.GetNext (pos);
			ar << (long) dwData;
		}

		ASSERT (m_lstItemData.GetCount () == m_lstItems.GetCount ());

		ar << m_strPrompt;
		ar << (long)m_clrPrompt;
	}
}
//**************************************************************************************
SIZE CBCGPToolbarComboBoxButton::OnCalculateSize (CDC* pDC, const CSize& sizeDefault, BOOL bHorz)
{
	m_bHorz = bHorz;
	m_sizeText = CSize (0, 0);

	if (!IsVisible())
	{
	
		if (m_bFlat)
		{
			if (m_pWndEdit->GetSafeHwnd () != NULL &&
				(m_pWndEdit->GetStyle () & WS_VISIBLE))
			{
				m_pWndEdit->ShowWindow (SW_HIDE);
			}

		
		}
				
		if (m_pWndCombo->GetSafeHwnd () != NULL &&
			(m_pWndCombo->GetStyle () & WS_VISIBLE))
		{
			m_pWndCombo->ShowWindow (SW_HIDE);
		}
			


		return CSize(0,0);
	}


	if (m_bFlat &&
		m_pWndCombo->GetSafeHwnd () != NULL &&
		(m_pWndCombo->GetStyle () & WS_VISIBLE))
	{
		m_pWndCombo->ShowWindow (SW_HIDE);
	}

	if (bHorz)
	{
		if (!m_bFlat && m_pWndCombo->GetSafeHwnd () != NULL && !m_bIsHidden)
		{
			m_pWndCombo->ShowWindow (SW_SHOWNOACTIVATE);
		}

		if (m_bTextBelow && !m_strText.IsEmpty())
		{
			CRect rectText (0, 0, m_iWidth, sizeDefault.cy);
			pDC->DrawText (m_strText, rectText, DT_CENTER | DT_CALCRECT | DT_WORDBREAK);
			m_sizeText = rectText.Size ();
		}

		int cy = sizeDefault.cy;

		if (m_pWndCombo != NULL && m_pWndCombo->GetSafeHwnd () != NULL)
		{
			CRect rectCombo;
			m_pWndCombo->GetWindowRect (&rectCombo);

			cy = rectCombo.Height ();
		}

		if (!m_bIsHidden && m_pWndEdit->GetSafeHwnd () != NULL &&
			(m_pWndCombo->GetStyle () & WS_VISIBLE) == 0)
		{
			m_pWndEdit->ShowWindow (SW_SHOWNOACTIVATE);
		}

		return CSize (m_iWidth, cy + m_sizeText.cy);

	}
	else
	{
		if (m_pWndCombo->GetSafeHwnd () != NULL &&
			(m_pWndCombo->GetStyle () & WS_VISIBLE))
		{
			m_pWndCombo->ShowWindow (SW_HIDE);
		}

		if (m_pWndEdit->GetSafeHwnd () != NULL &&
			(m_pWndEdit->GetStyle () & WS_VISIBLE))
		{
			m_pWndEdit->ShowWindow (SW_HIDE);
		}

		return CBCGPToolbarButton::OnCalculateSize (pDC, sizeDefault, bHorz);
	}
}
//**************************************************************************************
void CBCGPToolbarComboBoxButton::OnMove ()
{
	if (m_pWndCombo->GetSafeHwnd () != NULL)
	{
		AdjustRect ();
	}
}
//**************************************************************************************
void CBCGPToolbarComboBoxButton::OnSize (int iSize)
{
	m_iWidth = iSize;
	m_rect.right = m_rect.left + m_iWidth;

	if (m_pWndCombo->GetSafeHwnd () != NULL)
	{
		AdjustRect ();
	}
}
//**************************************************************************************
void CBCGPToolbarComboBoxButton::OnChangeParentWnd (CWnd* pWndParent)
{
	CBCGPToolbarButton::OnChangeParentWnd (pWndParent);

	if (m_pWndCombo->GetSafeHwnd () != NULL)
	{
		CWnd* pWndParentCurr = m_pWndCombo->GetParent ();
		ASSERT (pWndParentCurr != NULL);

		if (pWndParent != NULL &&
			pWndParentCurr->GetSafeHwnd () == pWndParent->GetSafeHwnd ())
		{
			return;
		}

		m_pWndCombo->DestroyWindow ();
		delete m_pWndCombo;
		m_pWndCombo = NULL;

		if (m_pWndEdit != NULL)
		{
			m_pWndEdit->DestroyWindow ();
			delete m_pWndEdit;
			m_pWndEdit = NULL;
		}
	}

	if (pWndParent == NULL || pWndParent->GetSafeHwnd () == NULL)
	{
		return;
	}

	BOOL bDisabled = CBCGPToolBar::IsCustomizeMode () || (m_nStyle & TBBS_DISABLED);

	CRect rect = m_rect;
	rect.InflateRect (-2, 0);
	rect.bottom = rect.top + m_nDropDownHeight;

	if ((m_pWndCombo = CreateCombo (pWndParent, rect)) == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	if (m_pWndCombo != NULL && m_pWndCombo->GetSafeHwnd () != NULL)
	{
		m_pWndCombo->EnableWindow (!bDisabled);
		m_pWndCombo->RedrawWindow ();
	}

	if (m_bFlat && (m_pWndCombo->GetStyle () & CBS_DROPDOWNLIST) == CBS_DROPDOWN)
	{
		DWORD dwEditStyle = WS_CHILD | WS_VISIBLE | ES_WANTRETURN | ES_AUTOHSCROLL;
		if (m_pWndCombo->GetStyle () & WS_TABSTOP)
		{
			dwEditStyle |= WS_TABSTOP;
		}
		
		if ((m_pWndEdit = CreateEdit (pWndParent, rect, dwEditStyle)) == NULL)
		{
			ASSERT (FALSE);
			return;
		}

		m_pWndEdit->SetFont (&globalData.fontRegular);
		m_pWndEdit->SetOwner (m_pWndCombo->GetParent ()->GetOwner ());

		if (m_pWndEdit != NULL && m_pWndEdit->GetSafeHwnd () != NULL)
		{
			m_pWndEdit->EnableWindow (!bDisabled);
			m_pWndEdit->RedrawWindow ();
		}
	}

	AdjustRect ();

	m_pWndCombo->SetFont (&globalData.fontRegular);

	if (m_pWndCombo->GetCount () > 0)
	{
		m_lstItems.RemoveAll ();
		
		ClearData ();
		m_lstItemData.RemoveAll ();

		for (int i = 0; i < m_pWndCombo->GetCount (); i ++)
		{
			CString str;
			m_pWndCombo->GetLBText (i, str);

			m_lstItems.AddTail (str);
			m_lstItemData.AddTail (m_pWndCombo->GetItemData (i));
		}

		m_iSelIndex = m_pWndCombo->GetCurSel ();
	}
	else
	{
		m_pWndCombo->ResetContent ();
		ASSERT (m_lstItemData.GetCount () == m_lstItems.GetCount ());

		POSITION posData = m_lstItemData.GetHeadPosition ();
		for (POSITION pos = m_lstItems.GetHeadPosition (); pos != NULL;)
		{
			ASSERT (posData != NULL);

			CString strItem = m_lstItems.GetNext (pos);
			int iIndex = m_pWndCombo->AddString (strItem);
			
			m_pWndCombo->SetItemData (iIndex, m_lstItemData.GetNext (posData));
		}

		if (m_iSelIndex != CB_ERR)
		{
			m_pWndCombo->SetCurSel (m_iSelIndex);
		}
	}

	if (m_iSelIndex != CB_ERR &&
		m_iSelIndex < m_pWndCombo->GetCount ())
	{
		m_pWndCombo->GetLBText (m_iSelIndex, m_strEdit);
		m_pWndCombo->SetWindowText (m_strEdit);

		if (m_pWndEdit != NULL)
		{
			m_pWndEdit->SetWindowText (m_strEdit);
		}
	}
}
//**************************************************************************************
INT_PTR CBCGPToolbarComboBoxButton::AddItem (LPCTSTR lpszItem, DWORD_PTR dwData)
{
	ASSERT (lpszItem != NULL);

	if (m_strEdit.IsEmpty ())
	{
		m_strEdit = lpszItem;
		if (m_pWndEdit != NULL)
		{
			m_pWndEdit->SetWindowText (m_strEdit);
		}
	}

	if (FindItem (lpszItem) < 0)
	{
		m_lstItems.AddTail (lpszItem);
		m_lstItemData.AddTail (dwData);
	}

	if (m_pWndCombo->GetSafeHwnd () != NULL)
	{
		int iIndex = m_pWndCombo->FindStringExact (-1, lpszItem);

		if (iIndex == CB_ERR)
		{
			iIndex = m_pWndCombo->AddString (lpszItem);
		}

		m_pWndCombo->SetCurSel (iIndex);
		m_pWndCombo->SetItemData (iIndex, dwData);
		m_pWndCombo->SetEditSel (-1, 0);
	}

	return m_lstItems.GetCount () - 1;
}
//**************************************************************************************
LPCTSTR CBCGPToolbarComboBoxButton::GetItem (int iIndex) const
{
	if (iIndex == -1)	// Current selection
	{
		if (m_pWndCombo->GetSafeHwnd () == NULL)
		{
			if ((iIndex = m_iSelIndex) == -1)	
			{
				return 0;
			}
		}
		else
		{
			iIndex = m_pWndCombo->GetCurSel ();
		}
	}

	POSITION pos = m_lstItems.FindIndex (iIndex);
	if (pos == NULL)
	{
		return NULL;
	}

	return m_lstItems.GetAt (pos);
}
//**************************************************************************************
DWORD_PTR CBCGPToolbarComboBoxButton::GetItemData (int iIndex) const
{
	if (iIndex == -1)	// Current selection
	{
		if (m_pWndCombo->GetSafeHwnd () == NULL)
		{
			if ((iIndex = m_iSelIndex) == -1)	
			{
				return 0;
			}
		}
		else
		{
			iIndex = m_pWndCombo->GetCurSel ();
		}
	}

	if (m_pWndCombo->GetSafeHwnd () != NULL)
	{
		return m_pWndCombo->GetItemData (iIndex);
	}
	else
	{
		POSITION pos = m_lstItemData.FindIndex (iIndex);
		if (pos == NULL)
		{
			return 0;
		}

		return m_lstItemData.GetAt (pos);
	}
}
//**************************************************************************************
void CBCGPToolbarComboBoxButton::RemoveAllItems ()
{
	m_lstItems.RemoveAll ();
	
	ClearData ();
	m_lstItemData.RemoveAll ();

	if (m_pWndCombo->GetSafeHwnd () != NULL)
	{
		m_pWndCombo->ResetContent ();
	}

	m_strEdit.Empty ();

	if (m_pWndEdit->GetSafeHwnd () != NULL)
	{
		m_pWndEdit->SetWindowText (m_strEdit);
	}
}
//**************************************************************************************
INT_PTR CBCGPToolbarComboBoxButton::GetCount () const
{
	return m_lstItems.GetCount ();
}
//**************************************************************************************
void CBCGPToolbarComboBoxButton::AdjustRect ()
{
	if (m_pWndCombo->GetSafeHwnd () == NULL ||
		m_rect.IsRectEmpty () || !m_bHorz)
	{
		m_rectCombo.SetRectEmpty ();
		m_rectButton.SetRectEmpty ();
		return;
	}

	if (m_bCenterVert && (!m_bTextBelow || m_strText.IsEmpty ()))
	{
		CBCGPToolBar* pParentBar = NULL;
		CWnd* pNextBar = m_pWndCombo->GetParent ();

		while (pParentBar == NULL && pNextBar != NULL)
		{
			pParentBar = DYNAMIC_DOWNCAST (CBCGPToolBar, pNextBar);
			pNextBar = pNextBar->GetParent ();
		}

		if (pParentBar != NULL)
		{
			const int nRowHeight = pParentBar->GetRowHeight ();
			const int yOffset = max (0, (nRowHeight - m_rect.Height ()) / 2);

			m_rectButton.OffsetRect (0, yOffset);
			m_rectCombo.OffsetRect (0, yOffset);
			m_rect.OffsetRect (0, yOffset);
		}
	}

	CRect rectParent;
	m_pWndCombo->SetWindowPos (NULL,
		m_rect.left + iHorzMargin, m_rect.top,
		m_rect.Width () - 2 * iHorzMargin, m_nDropDownHeight,
		SWP_NOZORDER | SWP_NOACTIVATE);
	m_pWndCombo->SetEditSel (-1, 0);

	{
		CRect rect;
		m_pWndCombo->GetWindowRect (&m_rectCombo);
		m_pWndCombo->ScreenToClient (&m_rectCombo);
		m_pWndCombo->MapWindowPoints (m_pWndCombo->GetParent (), &m_rectCombo);

	}

	if (m_bFlat)
	{
		m_rectButton = m_rectCombo;
		m_rectButton.left = m_rectButton.right - CBCGPMenuImages::Size ().cx * 2;

		m_rectButton.DeflateRect (2, 2);

		m_rect.left = m_rectCombo.left - iHorzMargin;
		m_rect.right = m_rectCombo.right + iHorzMargin;

		if (!m_bTextBelow || m_strText.IsEmpty ())
		{
			m_rect.top = m_rectCombo.top;
			m_rect.bottom = m_rectCombo.bottom;
		}

		if (m_pWndEdit != NULL)
		{
			CRect rectEdit = m_rect;

			const int iBorderOffset = 3;

			m_pWndEdit->SetWindowPos (NULL,
				m_rect.left + iHorzMargin + iBorderOffset, m_rect.top + iBorderOffset,
				m_rect.Width () - 2 * iHorzMargin - m_rectButton.Width () - iBorderOffset - 3,
				m_rectCombo.Height () - 2 * iBorderOffset,
				SWP_NOZORDER | SWP_NOACTIVATE);
		}
	}
	else
	{
		m_rectButton.SetRectEmpty ();
	}
}
//**************************************************************************************
void CBCGPToolbarComboBoxButton::SetHotEdit (BOOL bHot)
{
	if (m_bIsHotEdit != bHot)
	{
		m_bIsHotEdit = bHot;

		if (m_pWndCombo->GetParent () != NULL)
		{
			m_pWndCombo->GetParent ()->InvalidateRect (m_rectCombo);
			m_pWndCombo->GetParent ()->UpdateWindow ();
		}
	}
}
//**************************************************************************************
BOOL CBCGPToolbarComboBoxButton::NotifyCommand (int iNotifyCode)
{
	if (m_pWndCombo->GetSafeHwnd () == NULL)
	{
		return FALSE;
	}

	if (m_bFlat && iNotifyCode == 0)
	{
		return TRUE;
	}

	if (m_bFlat && m_pWndCombo->GetParent () != NULL)
	{
		m_pWndCombo->GetParent ()->InvalidateRect (m_rectCombo);
		m_pWndCombo->GetParent ()->UpdateWindow ();
	}

	switch (iNotifyCode)
	{
	case CBN_SELENDOK:
		{
			m_iSelIndex = m_pWndCombo->GetCurSel ();
			if (m_iSelIndex < 0)
			{
				return FALSE;
			}

			m_pWndCombo->GetLBText (m_iSelIndex, m_strEdit);
			if (m_pWndEdit != NULL)
			{
				m_pWndEdit->SetWindowText (m_strEdit);
			}

			//------------------------------------------------------
			// Try set selection in ALL comboboxes with the same ID:
			//------------------------------------------------------
			CObList listButtons;
			if (CBCGPToolBar::GetCommandButtons (m_nID, listButtons) > 0)
			{
				for (POSITION posCombo = listButtons.GetHeadPosition (); posCombo != NULL;)
				{
					CBCGPToolbarComboBoxButton* pCombo = 
						DYNAMIC_DOWNCAST (CBCGPToolbarComboBoxButton, listButtons.GetNext (posCombo));

					if (pCombo != NULL && pCombo != this)
					{
						pCombo->SelectItem (m_pWndCombo->GetCurSel (), FALSE /* Don't notify */);

						if (pCombo->m_pWndCombo->GetSafeHwnd () != NULL &&
							pCombo->m_pWndCombo->GetParent () != NULL)
						{
							pCombo->m_pWndCombo->GetParent ()->InvalidateRect (pCombo->m_rectCombo);
							pCombo->m_pWndCombo->GetParent ()->UpdateWindow ();
						}
					}
				}
			}
		}

		if (m_pWndEdit != NULL)
		{
			m_pWndEdit->SetFocus ();
		}

		return TRUE;

	case CBN_KILLFOCUS:
	case CBN_EDITUPDATE:
		return TRUE;

	case CBN_SETFOCUS:
		if (m_pWndEdit != NULL)
		{
			m_pWndEdit->SetFocus ();
		}
		return TRUE;

	case CBN_SELCHANGE:
		if (m_pWndEdit->GetSafeHwnd() != NULL && m_pWndCombo->GetCurSel () >= 0)
		{
			CString strEdit;
			m_pWndCombo->GetLBText (m_pWndCombo->GetCurSel (), strEdit);
			m_pWndEdit->SetWindowText (strEdit);
		}

		return TRUE;

	case CBN_EDITCHANGE:
		{
			m_pWndCombo->GetWindowText (m_strEdit);

		    if (m_pWndEdit != NULL && m_pWndEdit->GetSafeHwnd() != NULL)
			{
			    CString str;
				m_pWndEdit->GetWindowText (str);
				CComboBox* pBox = GetComboBox ();
				if (pBox != NULL && pBox->GetSafeHwnd() !=	NULL)
				{
					int nCurSel = pBox->GetCurSel ();
					int nNextSel = pBox->FindStringExact (nCurSel + 1, str);
					if (nNextSel == -1)
					{
						nNextSel = pBox->FindString (nCurSel + 1, str);
					}

					if (nNextSel != -1)
					{
						pBox->SetCurSel (nNextSel);
					}

					pBox->SetWindowText(str);
				} 
			}

			//------------------------------------------------------
			// Try set text of ALL comboboxes with the same ID:
			//------------------------------------------------------
			CObList listButtons;
			if (CBCGPToolBar::GetCommandButtons (m_nID, listButtons) > 0)
			{
				for (POSITION posCombo = listButtons.GetHeadPosition (); posCombo !=
					NULL;)
				{
					CBCGPToolbarComboBoxButton* pCombo = 
						DYNAMIC_DOWNCAST (CBCGPToolbarComboBoxButton, listButtons.GetNext
						(posCombo));
					
					if (pCombo != NULL && pCombo != this)
					{
						if (pCombo->GetComboBox () != NULL)
						{
							pCombo->GetComboBox ()->SetWindowText(m_strEdit);
						}

						pCombo->m_strEdit = m_strEdit;
					}
				}
			}
		}
		return TRUE;
	}

	return FALSE;
}
//**************************************************************************************
void CBCGPToolbarComboBoxButton::OnAddToCustomizePage ()
{
	CObList listButtons;	// Existing buttons with the same command ID

	if (CBCGPToolBar::GetCommandButtons (m_nID, listButtons) == 0)
	{
		return;
	}

	CBCGPToolbarComboBoxButton* pOther = 
		(CBCGPToolbarComboBoxButton*) listButtons.GetHead ();
	ASSERT_VALID (pOther);
	ASSERT_KINDOF (CBCGPToolbarComboBoxButton, pOther);

	CopyFrom (*pOther);
}
//**************************************************************************************
HBRUSH CBCGPToolbarComboBoxButton::OnCtlColor (CDC* pDC, UINT /*nCtlColor*/)
{
	COLORREF clrText = globalData.clrWindowText;
	COLORREF clrBk = globalData.clrWindow;

	HBRUSH hbr = CBCGPVisualManager::GetInstance ()->GetToolbarEditColors(this, clrText, clrBk);

	pDC->SetTextColor (clrText);
	pDC->SetBkColor (clrBk);

	return hbr;
}
//**************************************************************************************
void CBCGPToolbarComboBoxButton::OnDraw (CDC* pDC, const CRect& rect, CBCGPToolBarImages* pImages,
						BOOL bHorz, BOOL bCustomizeMode,
						BOOL bHighlight,
						BOOL bDrawBorder, BOOL bGrayDisabledButtons)
{
	if (m_pWndCombo == NULL || m_pWndCombo->GetSafeHwnd () == NULL || !bHorz)
	{
		CBCGPToolbarButton::OnDraw (pDC, rect, pImages,
							bHorz, bCustomizeMode,
							bHighlight, bDrawBorder, bGrayDisabledButtons);
		return;
	}

	BOOL bDisabled = (bCustomizeMode && !IsEditable ()) ||
		(!bCustomizeMode && (m_nStyle & TBBS_DISABLED));
		
	pDC->SetTextColor (bDisabled ?
		globalData.clrGrayedText : 
			(bHighlight) ?	CBCGPToolBar::GetHotTextColor () :
							globalData.clrBarText);

	if (m_bFlat)
	{
		if (m_bIsHotEdit)
		{
			bHighlight = TRUE;
		}
		
		//--------------
		// Draw combbox:
		//--------------
		CRect rectCombo = m_rectCombo;

		//-------------
		// Draw border:
		//-------------
		CBCGPVisualManager::GetInstance ()->OnDrawComboBorder (
			pDC, rectCombo, bDisabled, m_pWndCombo->GetDroppedState (),
			bHighlight, this);

		rectCombo.DeflateRect (2, 2);

		int nPrevTextColor = pDC->GetTextColor ();

		CBCGPVisualManager::GetInstance ()->OnFillCombo (
			pDC, rectCombo, bDisabled, m_pWndCombo->GetDroppedState (),
			bHighlight, this);

		//-----------------------
		// Draw drop-down button:
		//-----------------------
		CRect rectButton = m_rectButton;
		if (globalData.m_bIsBlackHighContrast)
		{
			rectButton.DeflateRect (1, 1);
		}

		if (rectButton.left > rectCombo.left + 1)
		{
			CBCGPVisualManager::GetInstance ()->OnDrawComboDropButton (
				pDC, rectButton, bDisabled, m_pWndCombo->GetDroppedState (),
				bHighlight, this);
		}

		pDC->SetTextColor (nPrevTextColor);

		//-----------------
		// Draw combo text:
		//-----------------
		if (!m_strEdit.IsEmpty ())
		{
			CRect rectText = rectCombo;
			rectText.right = m_rectButton.left;
			rectText.DeflateRect (2, 2);

			if (m_pWndEdit == NULL)
			{
				if (m_pWndCombo->GetStyle () & (CBS_OWNERDRAWFIXED | CBS_OWNERDRAWVARIABLE))
				{
					DRAWITEMSTRUCT dis;
					memset (&dis, 0, sizeof (DRAWITEMSTRUCT));

					dis.hDC = pDC->GetSafeHdc ();
					dis.rcItem = rectText;
					dis.CtlID = m_nID;
					dis.itemID = m_pWndCombo->GetCurSel ();
					dis.hwndItem = m_pWndCombo->GetSafeHwnd ();
					dis.CtlType = ODT_COMBOBOX;
					dis.itemState |= ODS_COMBOBOXEDIT;
					dis.itemData = m_pWndCombo->GetItemData (dis.itemID);

					if (bDisabled)
					{
						dis.itemState |= ODS_DISABLED;
					}

					m_pWndCombo->DrawItem (&dis);
				}
				else
				{
					BOOL bDrawPrompt = m_strEdit.IsEmpty() && !m_strPrompt.IsEmpty();
					COLORREF clrText = bDrawPrompt ? (m_clrPrompt == (COLORREF)-1 ? CBCGPVisualManager::GetInstance()->GetToolbarEditPromptColor() : m_clrPrompt) : CBCGPVisualManager::GetInstance ()->GetComboboxTextColor(this, bDisabled, bHighlight, m_pWndCombo->GetDroppedState());
					const CString& strText = bDrawPrompt ? m_strPrompt : m_strEdit;

					COLORREF cltTextOld = pDC->SetTextColor(clrText);
					DrawButtonText (pDC, strText, &rectText, DT_VCENTER | DT_SINGLELINE, globalData.clrWindowText, (int)CBCGPVisualManager::ButtonsIsRegular);
					pDC->SetTextColor (cltTextOld);
				}
			}
			
		}

		pDC->SetTextColor (nPrevTextColor);
	}

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
		rectText.top = (m_rectCombo.bottom + rect.bottom - m_sizeText.cy) / 2;
		pDC->DrawText (m_strText, &rectText, DT_CENTER | DT_WORDBREAK);

		pDC->SetTextColor (cltTextOld);
	}
}
//**************************************************************************************
BOOL CBCGPToolbarComboBoxButton::OnClick (CWnd* pWnd, BOOL /*bDelay*/)
{	
	if (m_pWndCombo == NULL || m_pWndCombo->GetSafeHwnd () == NULL || !m_bHorz)
	{
		return FALSE;
	}

	if (m_bFlat)
	{
		if (m_pWndEdit == NULL)
		{
			m_pWndCombo->SetFocus ();
		}
		else
		{
			m_pWndEdit->SetFocus ();
		}

		m_pWndCombo->ShowDropDown ();

		if (pWnd != NULL)
		{
			pWnd->InvalidateRect (m_rectCombo);
		}
	}

	return TRUE;
}
//**************************************************************************************
BOOL CBCGPToolbarComboBoxButton::SelectItem (int iIndex, BOOL bNotify)
{
	if (iIndex >= m_lstItems.GetCount ())
	{
		return FALSE;
	}

	m_iSelIndex = max (-1, iIndex);

	if (m_pWndCombo->GetSafeHwnd () == NULL)
	{
		return TRUE;
	}

	if (m_iSelIndex >= 0)
	{
		m_pWndCombo->GetLBText (iIndex, m_strEdit);
	}
	else
	{
		m_strEdit.Empty ();
	}

	if (m_pWndEdit != NULL)
	{
		CString strEdit;
		m_pWndEdit->GetWindowText (strEdit);

		if (strEdit != m_strEdit)
		{
			m_pWndEdit->SetWindowText (m_strEdit);
		}
	}

	if (m_pWndCombo->GetCurSel () == iIndex)
	{
		// Already selected
		return TRUE;
	}

	if (m_pWndCombo->SetCurSel (iIndex) != CB_ERR)
	{
		if (bNotify)
		{
			NotifyCommand (CBN_SELENDOK);
		}

		return TRUE;
	}
	else
	{
		return FALSE;
	}
}
//**************************************************************************************
BOOL CBCGPToolbarComboBoxButton::SelectItem (DWORD_PTR dwData)
{
	int iIndex = 0;
	for (POSITION pos = m_lstItemData.GetHeadPosition (); pos != NULL; iIndex ++)
	{
		if (m_lstItemData.GetNext (pos) == dwData)
		{
			return SelectItem (iIndex);
		}
	}

	return FALSE;
}
//**************************************************************************************
BOOL CBCGPToolbarComboBoxButton::SelectItem (LPCTSTR lpszText)
{
	ASSERT (lpszText != NULL);

	int iIndex = FindItem (lpszText);
	if (iIndex < 0)
	{
		return FALSE;
	}

	return SelectItem (iIndex);
}
//******************************************************************************************
BOOL CBCGPToolbarComboBoxButton::DeleteItem (int iIndex)
{
	if (iIndex < 0 || iIndex >= m_lstItems.GetCount ())
	{
		return FALSE;
	}

	POSITION pos = m_lstItems.FindIndex (iIndex);
	if (pos == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	m_lstItems.RemoveAt (pos);

	pos = m_lstItemData.FindIndex (iIndex);
	if (pos == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	m_lstItemData.RemoveAt (pos);

	if (m_pWndCombo->GetSafeHwnd () != NULL)
	{
		m_pWndCombo->DeleteString (iIndex);
	}

	if (iIndex == m_iSelIndex)
	{
		int iSelIndex = m_iSelIndex;
		if (iSelIndex >= m_lstItems.GetCount ())
		{
			iSelIndex = (int) m_lstItems.GetCount () - 1;
		}

		SelectItem (iSelIndex, FALSE);
	}

	return TRUE;
}
//**************************************************************************************
BOOL CBCGPToolbarComboBoxButton::DeleteItem (DWORD_PTR dwData)
{
	int iIndex = 0;
	for (POSITION pos = m_lstItemData.GetHeadPosition (); pos != NULL; iIndex ++)
	{
		if (m_lstItemData.GetNext (pos) == dwData)
		{
			return DeleteItem (iIndex);
		}
	}

	return FALSE;
}
//**************************************************************************************
BOOL CBCGPToolbarComboBoxButton::DeleteItem (LPCTSTR lpszText)
{
	ASSERT (lpszText != NULL);

	int iIndex = FindItem (lpszText);
	if (iIndex < 0)
	{
		return FALSE;
	}

	return DeleteItem (iIndex);
}
//********************************************************************************
int CBCGPToolbarComboBoxButton::FindItem (LPCTSTR lpszText) const
{
	ASSERT (lpszText != NULL);

	int iIndex = 0;
	for (POSITION pos = m_lstItems.GetHeadPosition (); pos != NULL; iIndex++)
	{
		if (m_lstItems.GetNext (pos).CompareNoCase (lpszText) == 0)
		{
			return iIndex;
		}
	}

	return CB_ERR;
}
//******************************************************************************************
int CBCGPToolbarComboBoxButton::OnDrawOnCustomizeList (
	CDC* pDC, const CRect& rect, BOOL bSelected)
{
	int iWidth = CBCGPToolbarButton::OnDrawOnCustomizeList (pDC, rect, bSelected) + 10;

	//------------------------------
	// Simulate combobox appearance:
	//------------------------------
	CRect rectCombo = rect;
	int nComboWidth = max (20, rect.Width () - iWidth);

	rectCombo.left = rectCombo.right - nComboWidth;

	int nMargin = 1;
	rectCombo.DeflateRect (nMargin, nMargin);

	pDC->FillRect (rectCombo, &globalData.brWindow);

	pDC->Draw3dRect (rectCombo, globalData.clrBarShadow, globalData.clrBarShadow);

	CRect rectBtn = rectCombo;
	rectBtn.left = rectBtn.right - rectBtn.Height () + 2;
	rectBtn.DeflateRect (nMargin, nMargin);

	CBCGPVisualManager::GetInstance ()->OnDrawComboDropButton (
		pDC, rectBtn, FALSE, FALSE, FALSE, this);

	return rect.Width ();
}
//********************************************************************************************
CComboBox* CBCGPToolbarComboBoxButton::CreateCombo (CWnd* pWndParent, const CRect& rect)
{
	CComboBox* pWndCombo = new CBCGPComboBox;
	if (!pWndCombo->Create (m_dwStyle, rect, pWndParent, m_nID))
	{
		delete pWndCombo;
		return NULL;
	}

	return pWndCombo;
}
//****************************************************************************************
CBCGPComboEdit* CBCGPToolbarComboBoxButton::CreateEdit (CWnd* pWndParent, const CRect& rect, DWORD dwEditStyle)
{
	CBCGPComboEdit* pWndEdit = new CBCGPComboEdit (*this);

	if (!pWndEdit->Create (dwEditStyle, rect, pWndParent, m_nID))
	{
		delete pWndEdit;
		return NULL;
	}

	if (!m_strPrompt.IsEmpty())
	{
		pWndEdit->SetPrompt(m_strPrompt, m_clrPrompt, FALSE);
	}

	return pWndEdit;
}
//***************************************************************************************
void CBCGPToolbarComboBoxButton::OnShow (BOOL bShow)
{
	if (m_pWndCombo->GetSafeHwnd () != NULL)
	{
		if (bShow && m_bHorz)
		{
			OnMove ();
			m_pWndCombo->ShowWindow (m_bFlat ? SW_HIDE : SW_SHOWNOACTIVATE);
		}
		else
		{
			m_pWndCombo->ShowWindow (SW_HIDE);
		}
	}

	if (m_pWndEdit->GetSafeHwnd () != NULL)
	{
		if (bShow && m_bHorz)
		{
			m_pWndEdit->ShowWindow (SW_SHOWNOACTIVATE);
		}
		else
		{
			m_pWndEdit->ShowWindow (SW_HIDE);
		}
	}
}
//*************************************************************************************
BOOL CBCGPToolbarComboBoxButton::ExportToMenuButton (CBCGPToolbarMenuButton& menuButton) const
{
	CString strMessage;
	int iOffset;

	if (strMessage.LoadString (m_nID) &&
		(iOffset = strMessage.Find (_T('\n'))) != -1)
	{
		menuButton.m_strText = strMessage.Mid (iOffset + 1);
	}

	return TRUE;
}
//*********************************************************************************
void CBCGPToolbarComboBoxButton::SetDropDownHeight (int nHeight)
{
	if (m_nDropDownHeight == nHeight)
	{
		return;
	}

	m_nDropDownHeight = nHeight;
	OnMove ();
}
//*********************************************************************************
void CBCGPToolbarComboBoxButton::SetText (LPCTSTR lpszText)
{
	ASSERT (lpszText != NULL);
	
	if (!SelectItem (lpszText))
	{
		m_strEdit = lpszText;

		if (m_pWndCombo != NULL && !m_bFlat)
		{
			CString strText;
			m_pWndCombo->GetWindowText(strText);

			if (strText != lpszText)
			{
				m_pWndCombo->SetWindowText(lpszText);
				NotifyCommand (CBN_EDITCHANGE);
			}
		}

		if (m_pWndEdit != NULL)
		{
			CString strText;
			m_pWndEdit->GetWindowText(strText);

			if (strText != lpszText)
			{
				m_pWndEdit->SetWindowText (lpszText);
			}
		}
	}
}
//*********************************************************************************
CBCGPToolbarComboBoxButton* CBCGPToolbarComboBoxButton::GetByCmd (UINT uiCmd,
																BOOL bIsFocus)
{
	CBCGPToolbarComboBoxButton* pSrcCombo = NULL;

	CObList listButtons;
	if (CBCGPToolBar::GetCommandButtons (uiCmd, listButtons) > 0)
	{
		for (POSITION posCombo= listButtons.GetHeadPosition (); posCombo != NULL;)
		{
			CBCGPToolbarComboBoxButton* pCombo = DYNAMIC_DOWNCAST (CBCGPToolbarComboBoxButton, listButtons.GetNext (posCombo));
			ASSERT (pCombo != NULL);

			if (pCombo != NULL && (!bIsFocus || pCombo->HasFocus ()))
			{
				pSrcCombo = pCombo;
				break;
			}
		}
	}

	return pSrcCombo;
}
//*********************************************************************************
BOOL CBCGPToolbarComboBoxButton::SelectItemAll (UINT uiCmd, int iIndex)
{
	CBCGPToolbarComboBoxButton* pSrcCombo = GetByCmd (uiCmd);

	if (pSrcCombo)
	{
		pSrcCombo->SelectItem (iIndex);
	}

	return pSrcCombo != NULL;
}
//*********************************************************************************
BOOL CBCGPToolbarComboBoxButton::SelectItemAll (UINT uiCmd, DWORD_PTR dwData)
{
	CBCGPToolbarComboBoxButton* pSrcCombo = GetByCmd (uiCmd);

	if (pSrcCombo)
	{
		pSrcCombo->SelectItem (dwData);
	}

	return pSrcCombo != NULL;
}
//*********************************************************************************
BOOL CBCGPToolbarComboBoxButton::SelectItemAll (UINT uiCmd, LPCTSTR lpszText)
{
	CBCGPToolbarComboBoxButton* pSrcCombo = GetByCmd (uiCmd);

	if (pSrcCombo)
	{
		pSrcCombo->SelectItem (lpszText);
	}

	return pSrcCombo != NULL;
}
//*********************************************************************************
int CBCGPToolbarComboBoxButton::GetCountAll (UINT uiCmd)
{
	CBCGPToolbarComboBoxButton* pSrcCombo = GetByCmd (uiCmd);

	if (pSrcCombo)
	{
		return (int) pSrcCombo->GetCount ();
	}

	return CB_ERR;
}
//*********************************************************************************
int CBCGPToolbarComboBoxButton::GetCurSelAll (UINT uiCmd)
{
	CBCGPToolbarComboBoxButton* pSrcCombo = GetByCmd (uiCmd);

	if (pSrcCombo)
	{
		return pSrcCombo->GetCurSel ();
	}

	return CB_ERR;
}
//*********************************************************************************
LPCTSTR CBCGPToolbarComboBoxButton::GetItemAll (UINT uiCmd, int iIndex)
{
	CBCGPToolbarComboBoxButton* pSrcCombo = GetByCmd (uiCmd);

	if (pSrcCombo)
	{
		return pSrcCombo->GetItem (iIndex);
	}

	return NULL;
}
//*********************************************************************************
DWORD_PTR CBCGPToolbarComboBoxButton::GetItemDataAll (UINT uiCmd, int iIndex)
{
	CBCGPToolbarComboBoxButton* pSrcCombo = GetByCmd (uiCmd);

	if (pSrcCombo)
	{
		return pSrcCombo->GetItemData (iIndex);
	}

	return (DWORD_PTR)CB_ERR;
}
//*********************************************************************************
void* CBCGPToolbarComboBoxButton::GetItemDataPtrAll (UINT uiCmd, int iIndex)
{
	CBCGPToolbarComboBoxButton* pSrcCombo = GetByCmd (uiCmd);

	if (pSrcCombo)
	{
		return pSrcCombo->GetComboBox ()->GetItemDataPtr (iIndex);
	}

	return NULL;
}
//*********************************************************************************
LPCTSTR CBCGPToolbarComboBoxButton::GetTextAll (UINT uiCmd)
{
	CBCGPToolbarComboBoxButton* pSrcCombo = GetByCmd (uiCmd);

	if (pSrcCombo)
	{
		return pSrcCombo->GetText ();
	}

	return NULL;
}
//*********************************************************************************
void CBCGPToolbarComboBoxButton::SetStyle (UINT nStyle)
{
	CBCGPToolbarButton::SetStyle (nStyle);

	BOOL bDisabled = (CBCGPToolBar::IsCustomizeMode () && !IsEditable ()) ||
		(!CBCGPToolBar::IsCustomizeMode () && (m_nStyle & TBBS_DISABLED));


	if (m_pWndCombo != NULL && m_pWndCombo->GetSafeHwnd () != NULL)
	{
		m_pWndCombo->EnableWindow (!bDisabled);
		m_pWndCombo->RedrawWindow ();
	}

	if (m_pWndEdit != NULL && m_pWndEdit->GetSafeHwnd () != NULL)
	{
		m_pWndEdit->EnableWindow (!bDisabled);
		m_pWndEdit->RedrawWindow ();
	}
}
//*******************************************************************************
BOOL CBCGPToolbarComboBoxButton::HasFocus () const
{
	if (m_pWndCombo == NULL)
	{
		return FALSE;
	}

	CWnd* pWndFocus = CWnd::GetFocus ();

	if (m_pWndCombo->GetDroppedState () ||
		pWndFocus == m_pWndCombo || m_pWndCombo->IsChild (pWndFocus))
	{
		return TRUE;
	}

	if (m_pWndEdit == NULL)
	{
		return FALSE;
	}

	return pWndFocus == m_pWndEdit || m_pWndEdit->IsChild (pWndFocus);
}
//*******************************************************************************
void CBCGPToolbarComboBoxButton::OnCancelMode ()
{
	CBCGPToolbarButton::OnCancelMode();

	if (m_pWndCombo->GetSafeHwnd() != NULL && m_pWndCombo->GetDroppedState())
	{
		m_pWndCombo->ShowDropDown(FALSE);
	}

	if (m_pWndCombo->GetSafeHwnd() != NULL && HasFocus() && m_pWndCombo->GetTopLevelFrame () != NULL)
	{
		m_pWndCombo->GetTopLevelFrame ()->SetFocus ();
	}
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPComboEdit

CBCGPComboEdit::CBCGPComboEdit(CBCGPToolbarComboBoxButton& combo) :
	m_combo (combo)
{
	m_bTracked = FALSE;
}

CBCGPComboEdit::~CBCGPComboEdit()
{
}

BEGIN_MESSAGE_MAP(CBCGPComboEdit, CBCGPEdit)
	//{{AFX_MSG_MAP(CBCGPComboEdit)
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_CONTROL_REFLECT(EN_CHANGE, OnChange)
	ON_WM_MOUSEMOVE()
	ON_WM_CONTEXTMENU()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGPComboEdit message handlers

BOOL CBCGPComboEdit::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message == WM_MOUSEWHEEL &&
		m_combo.GetComboBox () != NULL &&
		m_combo.GetComboBox ()->GetDroppedState ())
	{
		m_combo.GetComboBox ()->SendMessage (pMsg->message, pMsg->wParam, pMsg->lParam);
		return TRUE;
	}

	if (pMsg->message == WM_KEYDOWN)
	{
		if ((GetKeyState(VK_MENU) >= 0) && (GetKeyState(VK_CONTROL) >= 0) &&
			m_combo.GetComboBox () != NULL)
		{
			switch (pMsg->wParam)
			{
			case VK_UP:
			case VK_DOWN:
			case VK_HOME:
			case VK_END:
			case VK_NEXT:
			case VK_PRIOR:
				if (!m_combo.GetComboBox ()->GetDroppedState ())
				{
					break;
				}

			case VK_RETURN:
				SetFocus ();

				if (m_combo.GetComboBox ()->GetDroppedState ())
				{
					m_combo.GetComboBox ()->SendMessage (pMsg->message, pMsg->wParam, pMsg->lParam);
				}
				else if (m_combo.GetComboBox ()->GetOwner () != NULL)
				{
					GetWindowText (m_combo.m_strEdit);

					m_combo.GetComboBox ()->GetOwner ()->PostMessage (
						WM_COMMAND, MAKEWPARAM (m_combo.m_nID, 0),
						(LPARAM) m_combo.GetComboBox ()->GetSafeHwnd ());
				}

				return TRUE;
			}
		}

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
			if (m_combo.GetComboBox () != NULL)
			{
				m_combo.GetComboBox ()->ShowDropDown (FALSE);
			}

			if (GetTopLevelFrame () != NULL)
			{
				GetTopLevelFrame ()->SetFocus ();
				return TRUE;
			}
			
			break;

		case VK_UP:
		case VK_DOWN:
			if ((GetKeyState(VK_MENU) >= 0) && (GetKeyState(VK_CONTROL) >=0) &&
				m_combo.GetComboBox () != NULL)
			{
				if (!m_combo.GetComboBox ()->GetDroppedState())
				{
					m_combo.GetComboBox ()->ShowDropDown();

					if (m_combo.GetComboBox ()->GetParent () != NULL)
					{
						m_combo.GetComboBox ()->GetParent ()->InvalidateRect (m_combo.m_rectCombo);
					}
				}
				return TRUE;
			}
		}
	}

	return CBCGPEdit::PreTranslateMessage(pMsg);
}
//*************************************************************************************
void CBCGPComboEdit::OnSetFocus(CWnd* pOldWnd) 
{
	CBCGPEdit::OnSetFocus(pOldWnd);
	m_combo.SetHotEdit ();
	m_combo.NotifyCommand (CBN_SETFOCUS);
}
//*************************************************************************************
void CBCGPComboEdit::OnKillFocus(CWnd* pNewWnd) 
{
	CBCGPEdit::OnKillFocus(pNewWnd);

	if (::IsWindow (m_combo.GetHwnd()))
	{
		m_combo.SetHotEdit (FALSE);
		m_combo.NotifyCommand (CBN_KILLFOCUS);
	}
}
//*************************************************************************************
void CBCGPComboEdit::OnChange() 
{
	if (m_bOnGlass)
	{
		InvalidateRect (NULL, FALSE);
		UpdateWindow ();
	}

	m_combo.NotifyCommand (CBN_EDITCHANGE);
}
//*************************************************************************************
void CBCGPComboEdit::OnMouseMove(UINT nFlags, CPoint point) 
{
	CBCGPEdit::OnMouseMove(nFlags, point);
	m_combo.SetHotEdit ();

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
afx_msg LRESULT CBCGPComboEdit::OnMouseLeave(WPARAM,LPARAM)
{
	m_bTracked = FALSE;

	if (CWnd::GetFocus () != this)
	{
		m_combo.SetHotEdit (FALSE);
	}

	return 0;
}
//********************************************************************************
void CBCGPComboEdit::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	if (m_combo.m_uiMenuResID != 0)
	{

		CWnd* pWndParent = pWnd->GetParent();

		HINSTANCE hInst = AfxFindResourceHandle (
		MAKEINTRESOURCE (m_combo.m_uiMenuResID), RT_MENU);

		if (hInst == NULL)
		{
			CBCGPEdit::OnContextMenu(pWnd, point) ;
			return;

		}

		HMENU hMenu = ::LoadMenu (hInst, MAKEINTRESOURCE (m_combo.m_uiMenuResID));
		if (hMenu == NULL)
		{
			CBCGPEdit::OnContextMenu(pWnd, point) ;
			return;
		}

		HMENU hPopupMenu = ::GetSubMenu(hMenu, 0);

		if (hPopupMenu == NULL)
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
	}
	else
	{
		CBCGPEdit::OnContextMenu(pWnd, point) ;
	}
}
//*****************************************************************************************
INT_PTR CBCGPToolbarComboBoxButton::AddSortedItem(LPCTSTR lpszItem, DWORD_PTR dwData)
{
	ASSERT (lpszItem != NULL);

	if (m_strEdit.IsEmpty ())
	{
		m_strEdit = lpszItem;
		if (m_pWndEdit != NULL)
		{
			m_pWndEdit->SetWindowText (m_strEdit);
		}
	}

	int nIndex = 0;
	BOOL bInserted = FALSE;

	if (FindItem (lpszItem) < 0)
	{
		for (nIndex =0; nIndex < m_lstItems.GetCount (); nIndex++)
		{
			POSITION pos = m_lstItems.FindIndex (nIndex);
			LPCTSTR str = (LPCTSTR) m_lstItems.GetAt (pos);
			if (Compare (lpszItem, str) < 0)
			{
				m_lstItems.InsertBefore (pos, lpszItem);
				POSITION posData = m_lstItemData.FindIndex (nIndex);
				m_lstItemData.InsertBefore (posData, dwData);
				bInserted = TRUE;
				break;
			};
		}

		if (!bInserted)
		{
			m_lstItems.AddTail (lpszItem);
			m_lstItemData.AddTail (dwData);
		}		
	}
		
	if (m_pWndCombo->GetSafeHwnd () != NULL)
	{
		int iIndex = m_pWndCombo->FindStringExact (-1, lpszItem);

		if (iIndex == CB_ERR)
		{
			if (!bInserted)
			{
				iIndex = m_pWndCombo->AddString (lpszItem);
			}
			else
			{
				iIndex = m_pWndCombo->InsertString (nIndex, lpszItem);

			}
		}

		m_pWndCombo->SetCurSel (iIndex);
		m_pWndCombo->SetItemData (iIndex, dwData);
		m_pWndCombo->SetEditSel (-1, 0);
	}

	if (bInserted)
	{
		return nIndex;
	}

	return m_lstItems.GetCount () - 1;

}
//*****************************************************************************************
int CBCGPToolbarComboBoxButton::Compare(LPCTSTR lpszItem1, LPCTSTR lpszItem2)
{
	return _tcscmp(lpszItem1, lpszItem2);
}
//*****************************************************************************************
void CBCGPToolbarComboBoxButton::OnGlobalFontsChanged()
{
	CBCGPToolbarButton::OnGlobalFontsChanged ();

	if (m_pWndEdit->GetSafeHwnd () != NULL)
	{
		m_pWndEdit->SetFont (&globalData.fontRegular);
	}

	if (m_pWndCombo->GetSafeHwnd () != NULL)
	{
		m_pWndCombo->SetFont (&globalData.fontRegular);
	}
}
//*********************************************************************************
BOOL CBCGPToolbarComboBoxButton::OnUpdateToolTip (CWnd* pWndParent, int iButtonIndex,
												  CToolTipCtrl& wndToolTip, CString& strTipText)
{
	if (!m_bHorz)
	{
		return FALSE;
	}

	if (!CBCGPToolBar::GetShowTooltips ())
	{
		return FALSE;
	}

	CString strTips;

	if (OnGetCustomToolTipText (strTips))
	{
		strTipText = strTips;
	}

	if (CBCGPToolbarComboBoxButton::IsFlatMode ())
	{
		CComboBox* pCombo = GetComboBox ();

		if (pCombo != NULL && (pCombo->GetStyle () & CBS_DROPDOWNLIST) == CBS_DROPDOWN)
		{
			CEdit* pEdit = GetEditCtrl ();
			if (pEdit != NULL)
			{
				wndToolTip.AddTool (pEdit, strTipText, NULL, 0);
			}
		}
		else
		{
			wndToolTip.AddTool (pWndParent, strTipText, Rect (), iButtonIndex + 1); 
		}
	}
	else
	{
		CComboBox* pCombo = GetComboBox ();
		if (pCombo != NULL)
		{
			wndToolTip.AddTool (pCombo, strTipText, NULL, 0);
		}
	}

	return TRUE;
}
//*******************************************************************************
void CBCGPToolbarComboBoxButton::SetOnGlass (BOOL bOnGlass)
{
	CBCGPToolbarButton::SetOnGlass (bOnGlass);

	CBCGPEdit* pEdit = DYNAMIC_DOWNCAST (CBCGPEdit, GetEditCtrl ());
	if (pEdit != NULL)
	{
		pEdit->m_bOnGlass = bOnGlass;
	}
}
//*******************************************************************************
BOOL CBCGPToolbarComboBoxButton::SetACCData (CWnd* pParent, CBCGPAccessibilityData& data)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pParent);

	if (!CBCGPToolbarButton::SetACCData (pParent, data))
	{
		return FALSE;
	}

	CComboBox* pCombo = GetComboBox ();
	if (pCombo != NULL && (pCombo->GetStyle () & CBS_DROPDOWNLIST) == CBS_DROPDOWNLIST)
	{
		data.m_nAccRole = ROLE_SYSTEM_DROPLIST;
	}
	else
	{
		data.m_nAccRole = ROLE_SYSTEM_COMBOBOX;
	}

	data.m_bAccState = STATE_SYSTEM_FOCUSABLE;
    if (HasFocus ())
	{
		data.m_bAccState |= STATE_SYSTEM_FOCUSED;
	}
	data.m_strAccDefAction = L"Open";
	data.m_strAccValue = GetText ();
		 
	return TRUE;
}
//*************************************************************************************
void CBCGPComboEdit::OnPaint() 
{
	CString str;
	GetWindowText (str);

	if (!str.IsEmpty () || m_combo.GetPrompt ().IsEmpty () || GetFocus () == this)
	{
		if (m_bOnGlass)
		{
			CPaintDC dc(this); // device context for painting

			CBCGPMemDC memDC (dc, this, 255 /* Opaque */);
			CDC* pDC = &memDC.GetDC ();
			
			SendMessage (WM_PRINTCLIENT, (WPARAM) pDC->GetSafeHdc (), (LPARAM) PRF_CLIENT);
		}
		else
		{
			Default ();
		}
		return;
	}

	CRect rect;
	GetClientRect (rect);

	CPaintDC dc (this);

	CFont* pOldFont = dc.SelectObject (&globalData.fontRegular);
	dc.SetBkMode (TRANSPARENT);

	UINT uiTextFormat = DT_LEFT | DT_SINGLELINE | DT_VCENTER;

	COLORREF clrText = globalData.clrWindowText;
	COLORREF clrBk = globalData.clrWindow;

	HBRUSH hbr = CBCGPVisualManager::GetInstance ()->GetToolbarEditColors(&m_combo, clrText, clrBk);

	if (m_combo.m_bOnGlass)
	{
		CBCGPDrawManager dm (dc);
		dm.DrawRect (rect, clrBk, (COLORREF)-1);

		rect.DeflateRect (1, 1);

		m_combo.DrawButtonText (&dc, m_combo.GetPrompt (), &rect, uiTextFormat, globalData.clrGrayedText, (int)CBCGPVisualManager::ButtonsIsRegular);
	}
	else
	{
		::FillRect (dc.GetSafeHdc(), rect, hbr);

		rect.DeflateRect (1, 1);

		COLORREF clrPrompt = m_clrPrompt == (COLORREF)-1 ? CBCGPVisualManager::GetInstance()->GetToolbarEditPromptColor() : m_clrPrompt;

		dc.SetTextColor (clrPrompt);
		dc.DrawText (m_combo.GetPrompt (), rect, uiTextFormat);
	}

	dc.SelectObject (pOldFont);
}
