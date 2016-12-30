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
// BCGPRibbonComboBox.cpp: implementation of the CBCGPRibbonComboBox class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "bcgcbpro.h"
#include "BCGPRibbonComboBox.h"
#include "BCGPRibbonCategory.h"
#include "BCGPRibbonPanelMenu.h"
#include "BCGPVisualManager.h"
#include "BCGPDropDownList.h"
#include "BCGPRibbonBar.h"
#include "BCGPRibbonPanel.h"
#include "MenuImages.h"
#include "trackmouse.h"
#include "BCGPLocalResource.h"
#include "BCGPToolbarMenuButton.h"
#include "bcgprores.h"
#include "BCGPRibbonEdit.h"
#include "BCGPRibbonFloaty.h"
#include "BCGPCalculator.h"

#ifndef BCGP_EXCLUDE_RIBBON

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

static const int iDefaultComboHeight = 150;
static const int iDefaultWidth = 108;

/////////////////////////////////////////////////////////////////////////////
// CBCGPRibbonComboBox

IMPLEMENT_DYNCREATE(CBCGPRibbonComboBox, CBCGPRibbonEdit)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGPRibbonComboBox::CBCGPRibbonComboBox(
	UINT	uiID,
	BOOL	bHasEditBox,
	int		nWidth,
	LPCTSTR	lpszLabel,
	int		nImage,
	BCGP_RIBBON_COMBO_SORT_ORDER sortOrder) :
	CBCGPRibbonEdit (uiID, nWidth == -1 ? iDefaultWidth : nWidth, lpszLabel, nImage)
{
	CommonInit ();
	
	m_bHasEditBox = bHasEditBox;
	m_sortOrder = sortOrder;
}
//**************************************************************************
CBCGPRibbonComboBox::CBCGPRibbonComboBox()
{
	CommonInit ();
}
//**************************************************************************
void CBCGPRibbonComboBox::CommonInit ()
{
	m_iSelIndex = -1;
	m_nDropDownHeight = iDefaultComboHeight;
	m_bHasEditBox = FALSE;
	m_bHasDropDownList = TRUE;
	m_nMenuArrowMargin = 3;
	m_bResizeDropDownList = TRUE;
	m_pCalcPopup = NULL;
	m_bIsCalculator = FALSE;
	m_sortOrder = BCGP_RIBBON_COMBO_SORT_ORDER_NO_SORT;
}
//**************************************************************************
CBCGPRibbonComboBox::~CBCGPRibbonComboBox()
{
	ClearData ();
}
//**************************************************************************
void CBCGPRibbonComboBox::ClearData ()
{
}
//**************************************************************************
void CBCGPRibbonComboBox::EnableCalculator(BOOL bEnable, const CStringList* plstAdditionalCommands,
										const CList<UINT, UINT>* plstExtCommands,
										LPCTSTR lpszDisplayFormat)
{
	ASSERT_VALID (this);

	if (!m_bHasEditBox)
	{
		ASSERT(FALSE);
		return;
	}

	m_bIsCalculator = bEnable;

	m_lstCalcAdditionalCommands.RemoveAll ();
	m_lstCalcExtCommands.RemoveAll();

	if (m_bIsCalculator && plstAdditionalCommands != NULL)
	{
		m_lstCalcAdditionalCommands.AddTail ((CStringList*) plstAdditionalCommands);
	}

	if (plstExtCommands != NULL)
	{
		m_lstCalcExtCommands.AddTail ((CList<UINT, UINT>*) plstExtCommands);
	}

	if (lpszDisplayFormat != NULL)
	{
		m_strCalcDisplayFormat = lpszDisplayFormat;
	}

	if (bEnable && m_ButtonImages.GetCount() == NULL)
	{
		m_ButtonImages.SetTransparentColor (RGB(255, 0, 255));
		m_ButtonImages.SetImageSize(CSize(16, 15));
		m_ButtonImages.Load(globalData.Is32BitIcons () ? IDB_BCGBARRES_BROWSE32 : IDB_BCGBARRES_BROWSE);

		if (globalData.Is32BitIcons ())
		{
			m_ButtonImages.ConvertTo32Bits(RGB(255, 0, 255));
		}
	}
}
//**************************************************************************
void CBCGPRibbonComboBox::EnableAutoComplete(BOOL bEnable)
{
	ASSERT_VALID (this);

	if (!m_bHasEditBox)
	{
		ASSERT(FALSE);
		return;
	}

	m_bIsAutoComplete = bEnable;
}
//**************************************************************************
INT_PTR CBCGPRibbonComboBox::AddItem (LPCTSTR lpszItem, DWORD_PTR dwData)
{
	ASSERT_VALID(this);
	ASSERT(lpszItem != NULL);

	int nCount = (int)m_lstItems.GetCount () - 1;

	if (FindItem(lpszItem) >= 0)
	{
		return nCount;
	}

	BOOL bIsReady = FALSE;

	if (m_sortOrder == BCGP_RIBBON_COMBO_SORT_ORDER_ASC)
	{
		int nIndex = 0;

		for (POSITION pos = m_lstItems.GetHeadPosition(); pos != NULL; nIndex++)
		{
			POSITION posSaved = pos;
			CString strCurrItem = m_lstItems.GetNext(pos);
			
			if (strCurrItem.CompareNoCase(lpszItem) > 0)
			{
				m_lstItems.InsertBefore(posSaved, lpszItem);

				POSITION posData = m_lstItemData.FindIndex ((INT_PTR)nIndex);
				ASSERT(posData != NULL);

				m_lstItemData.InsertBefore(posData, dwData);

				bIsReady = TRUE;
				break;
			}
		}

		if (!bIsReady)
		{
			m_lstItems.AddTail(lpszItem);
			m_lstItemData.AddTail(dwData);
		}
	}
	else if (m_sortOrder == BCGP_RIBBON_COMBO_SORT_ORDER_DESC)
	{
		int nIndex = nCount;

		for (POSITION pos = m_lstItems.GetTailPosition(); pos != NULL; nIndex--)
		{
			POSITION posSaved = pos;
			CString strCurrItem = m_lstItems.GetPrev(pos);
			
			if (strCurrItem.CompareNoCase(lpszItem) > 0)
			{
				m_lstItems.InsertAfter(posSaved, lpszItem);
				
				POSITION posData = m_lstItemData.FindIndex ((INT_PTR)nIndex);
				ASSERT(posData != NULL);
				
				m_lstItemData.InsertAfter(posData, dwData);
				
				bIsReady = TRUE;
				break;
			}
		}

		if (!bIsReady)
		{
			m_lstItems.AddHead(lpszItem);
			m_lstItemData.AddHead(dwData);
		}
	}
	else // No sort
	{
		m_lstItems.AddTail(lpszItem);
		m_lstItemData.AddTail(dwData);
	}

	return nCount + 1;
}
//**************************************************************************
INT_PTR CBCGPRibbonComboBox::InsertItem (int nPos, LPCTSTR lpszItem, DWORD_PTR dwData)
{
	ASSERT_VALID (this);
	ASSERT (lpszItem != NULL);
	ASSERT(m_sortOrder == BCGP_RIBBON_COMBO_SORT_ORDER_NO_SORT);

	POSITION pos = m_lstItems.FindIndex ((INT_PTR)nPos);
	if (pos != NULL)
	{
		POSITION posData = m_lstItemData.FindIndex ((INT_PTR)nPos);
		ASSERT (posData != NULL); // m_lstItems and m_lstItemData are not synchronized!

		if (posData != NULL)
		{
			if (FindItem (lpszItem) < 0)
			{
				m_lstItems.InsertBefore (pos, lpszItem);
				m_lstItemData.InsertBefore (posData, dwData);
			}

			return m_lstItems.GetCount () - 1;
		}
	}

	return AddItem (lpszItem, dwData);
}
//**************************************************************************
LPCTSTR CBCGPRibbonComboBox::GetItem (int iIndex) const
{
	ASSERT_VALID (this);

	if (iIndex == -1)	// Current selection
	{
		if ((iIndex = m_iSelIndex) == -1)	
		{
			return NULL;
		}
	}

	POSITION pos = m_lstItems.FindIndex (iIndex);
	if (pos == NULL)
	{
		return NULL;
	}

	return m_lstItems.GetAt (pos);
}
//**************************************************************************
DWORD_PTR CBCGPRibbonComboBox::GetItemData (int iIndex) const
{
	ASSERT_VALID (this);

	if (iIndex == -1)	// Current selection
	{
		if ((iIndex = m_iSelIndex) == -1)	
		{
			return 0;
		}
	}

	POSITION pos = m_lstItemData.FindIndex (iIndex);
	if (pos == NULL)
	{
		return 0;
	}

	return m_lstItemData.GetAt (pos);
}
//**************************************************************************
void CBCGPRibbonComboBox::RemoveAllItems ()
{
	ASSERT_VALID (this);

	ClearData ();

	m_lstItems.RemoveAll ();
	m_lstItemData.RemoveAll ();
	m_strEdit.Empty ();

	m_iSelIndex = -1;

	Redraw ();
}
//**************************************************************************
BOOL CBCGPRibbonComboBox::SelectItem (int iIndex)
{
	ASSERT_VALID (this);

	if (iIndex >= m_lstItems.GetCount ())
	{
		return FALSE;
	}

	m_iSelIndex = max (-1, iIndex);
	
	LPCTSTR lpszText = GetItem (m_iSelIndex);

	m_strEdit = lpszText == NULL ? _T("") : lpszText;

	if (m_pWndEdit->GetSafeHwnd () != NULL)
	{
		m_pWndEdit->SetWindowText (m_strEdit);
	}

	if (!m_bDontNotify)
	{
		CBCGPRibbonBar* pRibbonBar = GetTopLevelRibbonBar ();
		if (pRibbonBar != NULL)
		{
			ASSERT_VALID (pRibbonBar);

			CArray<CBCGPBaseRibbonElement*, CBCGPBaseRibbonElement*> arButtons;
			pRibbonBar->GetElementsByID (m_nID, arButtons, TRUE);

			for (int i = 0; i < arButtons.GetSize (); i++)
			{
				CBCGPRibbonComboBox* pOther =
					DYNAMIC_DOWNCAST (CBCGPRibbonComboBox, arButtons [i]);

				if (pOther != NULL && pOther != this)
				{
					ASSERT_VALID (pOther);

					pOther->m_bDontNotify = TRUE;
					pOther->SelectItem (iIndex);
					pOther->m_bDontNotify = FALSE;
					pOther->m_RecentChangeEvt = m_RecentChangeEvt;
				}
			}
		}
	}

	Redraw ();
	return TRUE;
}
//**************************************************************************
BOOL CBCGPRibbonComboBox::SelectItem (DWORD_PTR dwData)
{
	ASSERT_VALID (this);

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
//**************************************************************************
BOOL CBCGPRibbonComboBox::SelectItem (LPCTSTR lpszText)
{
	ASSERT_VALID (this);
	ASSERT (lpszText != NULL);

	int iIndex = FindItem (lpszText);
	if (iIndex < 0)
	{
		return FALSE;
	}

	return SelectItem (iIndex);
}
//**************************************************************************
BOOL CBCGPRibbonComboBox::DeleteItem (int iIndex)
{
	ASSERT_VALID (this);

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

	if (iIndex == m_iSelIndex)
	{
		int iSelIndex = m_iSelIndex;
		if (iSelIndex >= m_lstItems.GetCount ())
		{
			iSelIndex = (int) m_lstItems.GetCount () - 1;
		}

		SelectItem (iSelIndex);
	}

	return TRUE;
}
//**************************************************************************
BOOL CBCGPRibbonComboBox::DeleteItem (DWORD_PTR dwData)
{
	ASSERT_VALID (this);

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
//**************************************************************************
BOOL CBCGPRibbonComboBox::DeleteItem (LPCTSTR lpszText)
{
	ASSERT_VALID (this);
	ASSERT (lpszText != NULL);

	int iIndex = FindItem (lpszText);
	if (iIndex < 0)
	{
		return FALSE;
	}

	return DeleteItem (iIndex);
}
//**************************************************************************
int CBCGPRibbonComboBox::FindItem (LPCTSTR lpszText) const
{
	ASSERT_VALID (this);
	ASSERT (lpszText != NULL);

	int iIndex = 0;
	for (POSITION pos = m_lstItems.GetHeadPosition (); pos != NULL; iIndex++)
	{
		if (m_lstItems.GetNext (pos).CompareNoCase (lpszText) == 0)
		{
			return iIndex;
		}
	}

	return -1;
}
//**************************************************************************
int CBCGPRibbonComboBox::FindItemByPrefix(LPCTSTR lpszPrefix, CString& strResult) const
{
	ASSERT_VALID (this);
	ASSERT (lpszPrefix != NULL);

	strResult.Empty();

	CString strPrefix = lpszPrefix;
	strPrefix.MakeUpper();

	int iIndex = 0;
	for (POSITION pos = m_lstItems.GetHeadPosition (); pos != NULL; iIndex++)
	{
		CString strItem = m_lstItems.GetNext (pos);
		strItem.MakeUpper();

		if (strItem.Find(strPrefix) == 0)
		{
			strResult = GetItem(iIndex);
			return iIndex;
		}
	}

	return -1;
}
//**************************************************************************
void CBCGPRibbonComboBox::SetDropDownHeight (int nHeight)
{
	ASSERT_VALID (this);
	m_nDropDownHeight = nHeight;
}
//**************************************************************************
CSize CBCGPRibbonComboBox::GetIntermediateSize (CDC* pDC)
{
	ASSERT_VALID (this);

	CSize size = CBCGPRibbonEdit::GetIntermediateSize (pDC);

	size.cx += GetDropDownImageWidth () + 2 * m_nMenuArrowMargin + 1;

	return size;
}
//**************************************************************************
int CBCGPRibbonComboBox::GetDropDownImageWidth () const
{
	if (m_bIsCalculator && m_ButtonImages.GetCount() > 0)
	{
		return m_ButtonImages.GetImageSize().cx;
	}

	return CBCGPRibbonEdit::GetDropDownImageWidth();
}
//**************************************************************************
void CBCGPRibbonComboBox::OnDraw (CDC* pDC)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);

	OnDrawLabelAndImage (pDC);

	BOOL bIsHighlighted = m_bIsHighlighted;

	if (m_bIsFocused)
	{
		m_bIsHighlighted = TRUE;
	}

	if (IsDisabled ())
	{
		m_bIsHighlighted = FALSE;
	}

	CRect rectSaved = m_rect;
	m_rect.left = m_rectCommand.left;
	
	CBCGPVisualManager::GetInstance ()->OnFillRibbonButton (pDC, this);

	if (m_pWndEdit->GetSafeHwnd () == NULL)
	{
		CRect rectText = m_rectCommand;
		rectText.DeflateRect (m_szMargin);

		UINT uiDTFlags = DT_SINGLELINE | DT_VCENTER;

		if (!m_bSearchResultMode)
		{
			if (m_nAlign == ES_CENTER)
			{
				uiDTFlags |= DT_CENTER;
			}
			else if (m_nAlign == ES_RIGHT)
			{
				uiDTFlags |= DT_RIGHT;
			}
		}

		BOOL bDrawPrompt = m_strEdit.IsEmpty() && !m_strSearchPrompt.IsEmpty() && !IsDroppedDown();
		const CString& str =  bDrawPrompt ? m_strSearchPrompt : m_strEdit;
		COLORREF clrText = bDrawPrompt ? CBCGPVisualManager::GetInstance()->GetToolbarEditPromptColor() : (COLORREF)-1;

		if (IsDisabled())
		{
			clrText = globalData.clrGrayedText;
		}

		DoDrawText (pDC, str, rectText, uiDTFlags, clrText);
	}

	if (m_bSearchMode && m_strEdit.IsEmpty() && m_ImageSearch.IsValid())
	{
		CRect rectIcon = m_rectMenu;
		rectIcon.right = rectIcon.left - 1;
		rectIcon.left = rectIcon.right - rectIcon.Height();

		m_ImageSearch.DrawEx(pDC, rectIcon, 0, CBCGPToolBarImages::ImageAlignHorzCenter, CBCGPToolBarImages::ImageAlignVertCenter,
			CRect(0, 0, 0, 0), IsDisabled() ? (BYTE)127 : (BYTE)255);
	}

	CBCGPVisualManager::GetInstance ()->OnDrawRibbonButtonBorder 
			(pDC, this);

	CBCGPToolbarComboBoxButton buttonDummy;
	buttonDummy.m_bIsRibbon = TRUE;
	buttonDummy.m_bIsRibbonFloaty = IsFloatyMode ();

	BOOL bIsDropDownHighlighted = IsMenuAreaHighlighted () || m_bIsFocused ||
		m_bIsEditFocused ||
		(bIsHighlighted && !m_bHasEditBox);

	CBCGPVisualManager::GetInstance ()->OnDrawComboDropButton (
		pDC, m_rectMenu, IsDisabled (), IsDroppedDown (),
		bIsDropDownHighlighted,
		&buttonDummy);

	if (m_bIsCalculator)
	{
		m_ButtonImages.DrawEx(pDC, m_rectMenu, 2, CBCGPToolBarImages::ImageAlignHorzCenter, CBCGPToolBarImages::ImageAlignVertCenter);
	}

	m_bIsHighlighted = bIsHighlighted;
	m_rect = rectSaved;
}
//**************************************************************************
void CBCGPRibbonComboBox::OnLButtonDown (CPoint point)
{
	ASSERT_VALID (this);

	CBCGPBaseRibbonElement::OnLButtonDown (point);

	if ((m_rectMenu.PtInRect (point) || !m_bHasEditBox) && !IsDroppedDown ())
	{
		DropDownList ();
	}
}
//**************************************************************************
void CBCGPRibbonComboBox::OnLButtonUp (CPoint /*point*/)
{
	ASSERT_VALID (this);
}
//**************************************************************************
void CBCGPRibbonComboBox::CopyFrom (const CBCGPBaseRibbonElement& s)
{
	ASSERT_VALID (this);

	CBCGPRibbonEdit::CopyFrom (s);

	CBCGPRibbonComboBox& src = (CBCGPRibbonComboBox&) s;

	m_strEdit = src.m_strEdit;
	m_bHasEditBox = src.m_bHasEditBox;

	if (m_pWndEdit != NULL)
	{
		m_pWndEdit->DestroyWindow ();
		delete m_pWndEdit;
		m_pWndEdit = NULL;
	}

	m_lstItems.RemoveAll ();
	m_lstItems.AddTail (&src.m_lstItems);

	m_lstItemData.RemoveAll ();
	m_lstItemData.AddTail (&src.m_lstItemData);

	m_iSelIndex = src.m_iSelIndex;
	m_nDropDownHeight = src.m_nDropDownHeight;

	m_bResizeDropDownList = src.m_bResizeDropDownList;

	m_bIsCalculator = src.m_bIsCalculator;

	m_lstCalcAdditionalCommands.RemoveAll();
	m_lstCalcAdditionalCommands.AddTail(&src.m_lstCalcAdditionalCommands);

	m_lstCalcExtCommands.RemoveAll();
	m_lstCalcExtCommands.AddTail(&src.m_lstCalcExtCommands);

	m_strCalcDisplayFormat = src.m_strCalcDisplayFormat;

	src.m_ButtonImages.CopyTo(m_ButtonImages);

	m_sortOrder = src.m_sortOrder;
}
//**************************************************************************
void CBCGPRibbonComboBox::DropDownList ()
{
	ASSERT_VALID (this);

	if (IsDisabled ())
	{
		return;
	}

	if (m_pWndEdit->GetSafeHwnd () != NULL && !m_pWndEdit->IsWindowVisible ())
	{
		return;
	}

	if (CBCGPPopupMenu::GetActiveMenu () != NULL)
	{
		if (CBCGPPopupMenu::GetActiveMenu ()->GetMenuBar () != m_pParentMenu)
		{
			CBCGPPopupMenu::GetActiveMenu ()->SendMessage (WM_CLOSE);
			return;
		}
	}

	CBCGPBaseRibbonElement::OnShowPopupMenu ();

	if (m_bIsCalculator)
	{
		if (m_pCalcPopup != NULL)
		{
			m_pCalcPopup->SendMessage (WM_CLOSE);
			m_pCalcPopup = NULL;

			SetDroppedDown(NULL);
		}
		else
		{
			if (CBCGPPopupMenu::GetActiveMenu () != NULL)
			{
				if (CBCGPPopupMenu::GetActiveMenu ()->GetMenuBar () != m_pParentMenu)
				{
					CBCGPPopupMenu::GetActiveMenu ()->SendMessage (WM_CLOSE);
					return;
				}
			}

			CBCGPBaseRibbonElement::OnShowPopupMenu ();

			double dblValue = 0.;

			CString strValue = GetEditText ();
			if (!strValue.IsEmpty ())
			{
				strValue.Replace (_T(','), _T('.'));
	#if _MSC_VER < 1400
				_stscanf (strValue, _T("%lf"), &dblValue);
	#else
				_stscanf_s (strValue, _T("%lf"), &dblValue);
	#endif
			}

			m_pCalcPopup = new CBCGPCalculatorPopup (dblValue, 0, this);
			m_pCalcPopup->m_bAutoDestroyParent = FALSE;
			m_pCalcPopup->SetParentRibbonElement (this);

			CBCGPCalculator* pCalc = DYNAMIC_DOWNCAST (CBCGPCalculator, m_pCalcPopup->GetMenuBar());
			if (pCalc != NULL)
			{
				ASSERT_VALID (pCalc);

				if (!m_lstCalcAdditionalCommands.IsEmpty ())
				{
					pCalc->SetAdditionalCommands (m_lstCalcAdditionalCommands);
				}

				if (!m_lstCalcExtCommands.IsEmpty ())
				{
					pCalc->SetExtendedCommands (m_lstCalcExtCommands);
				}

				if (!m_strCalcDisplayFormat.IsEmpty())
				{
					pCalc->SetDisplayFormat(m_strCalcDisplayFormat);
				}
			}

			CRect rectWindow;
			m_pWndEdit->GetWindowRect (rectWindow);

			if (!m_pCalcPopup->Create (m_pWndEdit, rectWindow.left - m_szMargin.cx, rectWindow.bottom + m_szMargin.cy, NULL, TRUE))
			{
				ASSERT (FALSE);
				m_pCalcPopup = NULL;
			}
			else
			{
				SetDroppedDown(m_pCalcPopup);

				m_pCalcPopup->GetMenuBar()->SetFocus ();
				
				CRect rect;
				m_pCalcPopup->GetWindowRect (&rect);
				m_pCalcPopup->UpdateShadow (&rect);
			}
		}

		return;
	}

	CBCGPDropDownList* pList = new CBCGPDropDownList (this);
	pList->SetParentRibbonElement (this);

	int i = 0;
	for (POSITION pos = m_lstItems.GetHeadPosition (); pos != NULL; i++)
	{
		CString strItem = m_lstItems.GetNext (pos);
		pList->AddString (strItem);

		if (m_bHasEditBox && strItem == m_strEdit)
		{
			m_iSelIndex = i;
		}
	}

	pList->SetCurSel (m_iSelIndex);
	pList->SetMaxHeight (m_nDropDownHeight);
	pList->SetMinWidth (m_rect.Width ());

	CWnd* pWndParent = GetParentWnd ();
	if (pWndParent == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	const BOOL bIsRTL = (pWndParent->GetExStyle () & WS_EX_LAYOUTRTL);

	CRect rect = m_rectCommand.IsRectEmpty () ? m_rect : m_rectCommand;
	pWndParent->ClientToScreen (&rect);

	SetDroppedDown (pList);

	if (m_pParent != NULL)
	{
		ASSERT_VALID (m_pParent);
		m_pParent->HighlightPanel (NULL, CPoint (-1, -1));
	}

	if (m_pWndEdit->GetSafeHwnd () != NULL)
	{
		m_pWndEdit->SetFocus ();
		m_pWndEdit->SetSel (0, -1);
	}

	if (m_bResizeDropDownList)
	{
		pList->EnableVertResize (2 * globalData.GetTextHeight ());
	}

	pList->Track (CPoint (
		bIsRTL ? rect.right : rect.left, rect.bottom), pWndParent->GetOwner ());
}
//**************************************************************************
void CBCGPRibbonComboBox::OnSelectItem (int nItem)
{
	ASSERT_VALID (this);

	SelectItem (nItem);
	m_RecentChangeEvt = CBCGPRibbonEdit::BCGPRIBBON_EDIT_CHANGED_FROM_DROPDOWN;

	NotifyCommand (TRUE);

	if (m_pParentMenu != NULL)
	{
		ASSERT_VALID (m_pParentMenu);

		if (m_pParentMenu->IsFloaty ())
		{
			CBCGPRibbonFloaty* pFloaty = DYNAMIC_DOWNCAST (
				CBCGPRibbonFloaty, m_pParentMenu->GetParent ());

			if (pFloaty != NULL && !pFloaty->IsContextMenuMode ())
			{
				if (m_pWndEdit->GetSafeHwnd () != NULL && m_pWndEdit->GetTopLevelFrame () != NULL)
				{
					m_pWndEdit->GetTopLevelFrame ()->SetFocus ();
				}

				Redraw ();
				return;
			}
		}

		CFrameWnd* pParentFrame = BCGPGetParentFrame (m_pParentMenu);
		ASSERT_VALID (pParentFrame);

		pParentFrame->PostMessage (WM_CLOSE);
	}
	else
	{
		if (m_pWndEdit->GetSafeHwnd () != NULL && m_pWndEdit->GetTopLevelFrame () != NULL)
		{
			m_bNotifyCommand = FALSE;
			m_pWndEdit->GetTopLevelFrame ()->SetFocus ();
		}

		Redraw ();
	}
}
//**************************************************************************
void CBCGPRibbonComboBox::OnAfterChangeRect (CDC* pDC)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);

	CBCGPRibbonButton::OnAfterChangeRect (pDC);

	if (m_rect.IsRectEmpty ())
	{
		if (m_pWndEdit->GetSafeHwnd () != NULL)
		{
			m_pWndEdit->ShowWindow (SW_HIDE);
		}

		if (m_pBtnClear->GetSafeHwnd () != NULL)
		{
			m_pBtnClear->ShowWindow (SW_HIDE);
		}
		return;
	}

	CRect rectCommandOld = m_rectCommand;

	m_Location = RibbonElementSingleInGroup;

	m_rectMenu = m_rect;
	m_rectMenu.left = m_rectMenu.right - GetDropDownImageWidth () - 2 * m_nMenuArrowMargin;
	
	m_rectCommand = m_rect;
	m_rectCommand.right = m_rectMenu.left;
	m_rectCommand.left += m_nLabelImageWidth;

	int cx = m_bFloatyMode ? m_nWidthFloaty : m_nWidth;
	if (globalData.GetRibbonImageScale () > 1.)
	{
		cx = (int)(.5 + globalData.GetRibbonImageScale () * cx);
	}

	if (m_rectCommand.Width () > cx)
	{
		m_rectCommand.left = m_rectCommand.right - cx;
	}

	m_rectMenu.DeflateRect (1, 1);

	m_bMenuOnBottom = FALSE;

	if (!m_bHasEditBox)
	{
		return;
	}

	if (m_pWndEdit == NULL)
	{
		DWORD dwEditStyle = WS_CHILD | ES_WANTRETURN | 
							ES_AUTOHSCROLL | WS_TABSTOP;

		dwEditStyle |= m_nAlign;

		CWnd* pWndParent = GetParentWnd ();
		ASSERT_VALID (pWndParent);

		if ((m_pWndEdit = CreateEdit (pWndParent, dwEditStyle)) == NULL)
		{
			return;
		}

		m_pWndEdit->SendMessage (EM_SETTEXTMODE, TM_PLAINTEXT);
		m_pWndEdit->SetEventMask (m_pWndEdit->GetEventMask () | ENM_CHANGE);
		m_pWndEdit->SetFont (GetTopLevelRibbonBar ()->GetFont ());
		m_pWndEdit->SetWindowText (m_strEdit);
	}

	if (rectCommandOld != m_rectCommand || !m_pWndEdit->IsWindowVisible ())
	{
		CRect rectEdit = m_rectCommand;
		rectEdit.DeflateRect (m_szMargin.cx, m_szMargin.cy, 0, m_szMargin.cy);
		rectEdit.DeflateRect(0, m_sizePadding.cy / 2);

		if (m_bSearchMode)
		{
			rectEdit.right -= m_rect.Height();
		}

		m_pWndEdit->SetWindowPos (NULL, 
			rectEdit.left, rectEdit.top,
			rectEdit.Width (), rectEdit.Height (),
			SWP_NOZORDER | SWP_NOACTIVATE);

		m_pWndEdit->ShowWindow (SW_SHOWNOACTIVATE);

		if (m_bSearchMode && m_pBtnClear->GetSafeHwnd () != NULL)
		{
			m_pBtnClear->SetWindowPos (NULL, 
				m_rectMenu.left - m_rect.Height() + 2, m_rect.top + 1,
				m_rect.Height () - 2, m_rect.Height () - 2,
				SWP_NOZORDER | SWP_NOACTIVATE);
			m_pBtnClear->ShowWindow (m_strEdit.IsEmpty() ? SW_HIDE : SW_SHOWNOACTIVATE);
		}
	}
}
//********************************************************************************
BOOL CBCGPRibbonComboBox::OnDrawDropListItem (CDC* /*pDC*/, 
											  int /*nIndex*/, 
											  CBCGPToolbarMenuButton* /*pItem*/,
											  BOOL /*bHighlight*/)
{
	ASSERT_VALID (this);
	return FALSE;
}
//********************************************************************************
CSize CBCGPRibbonComboBox::OnGetDropListItemSize (
		CDC* /*pDC*/, 
		int /*nIndex*/, 
		CBCGPToolbarMenuButton* /*pItem*/,
		CSize /*sizeDefault*/)
{
	ASSERT_VALID (this);
	return CSize (0, 0);
}
//********************************************************************************
BOOL CBCGPRibbonComboBox::SetACCData(CWnd* pParent, CBCGPAccessibilityData& data)
{
	ASSERT_VALID(this);

	CBCGPRibbonEdit::SetACCData(pParent, data);

	data.m_nAccRole = ROLE_SYSTEM_COMBOBOX;

	if (!m_bHasEditBox)
	{
		data.m_bAccState = STATE_SYSTEM_READONLY;
	}

	if (m_bIsDisabled)
	{
		data.m_bAccState = STATE_SYSTEM_UNAVAILABLE;
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPRibbonFontComboBox

BOOL CBCGPRibbonFontComboBox::m_bDrawUsingFont = FALSE;

const int nImageHeight = 15;
const int nImageWidth = 16;
const int nImageMargin = 6;

IMPLEMENT_DYNCREATE(CBCGPRibbonFontComboBox, CBCGPRibbonComboBox)

CBCGPRibbonFontComboBox::CBCGPRibbonFontComboBox(
	UINT	uiID,
	int		nFontType,
	BYTE	nCharSet,
	BYTE	nPitchAndFamily,
	int		nWidth) :
	CBCGPRibbonComboBox (uiID, TRUE, nWidth),
	m_nFontType (DEVICE_FONTTYPE | RASTER_FONTTYPE | TRUETYPE_FONTTYPE),
	m_nCharSet (DEFAULT_CHARSET),
	m_nPitchAndFamily (DEFAULT_PITCH)
{
	BuildFonts (nFontType, nCharSet, nPitchAndFamily);

	m_bIsAutoComplete = TRUE;
}
//**************************************************************************
CBCGPRibbonFontComboBox::CBCGPRibbonFontComboBox() :
	m_nFontType (DEVICE_FONTTYPE | RASTER_FONTTYPE | TRUETYPE_FONTTYPE),
	m_nCharSet (DEFAULT_CHARSET),
	m_nPitchAndFamily (DEFAULT_PITCH)
{
	m_bIsAutoComplete = TRUE;
}
//**************************************************************************
CBCGPRibbonFontComboBox::~CBCGPRibbonFontComboBox()
{
	ClearData ();
}
//**************************************************************************
void CBCGPRibbonFontComboBox::CopyFrom (const CBCGPBaseRibbonElement& s)
{
	ASSERT_VALID (this);

	CBCGPRibbonComboBox::CopyFrom (s);

	CBCGPRibbonFontComboBox& src = (CBCGPRibbonFontComboBox&) s;

	m_nFontType = src.m_nFontType;
	m_nCharSet = src.m_nCharSet;
	m_nPitchAndFamily = src.m_nPitchAndFamily;
}
//**************************************************************************
void CBCGPRibbonFontComboBox::DropDownList ()
{
	ASSERT_VALID (this);

	BuildFonts (m_nFontType, m_nCharSet, m_nPitchAndFamily);

	CBCGPRibbonComboBox::DropDownList ();
}
//****************************************************************************************
void CBCGPRibbonFontComboBox::BuildFonts (
		int nFontType/* = DEVICE_FONTTYPE | RASTER_FONTTYPE | TRUETYPE_FONTTYPE*/,
		BYTE nCharSet/* = DEFAULT_CHARSET*/,
		BYTE nPitchAndFamily/* = DEFAULT_PITCH*/)
{
	if (m_lstItems.IsEmpty () || 
		m_nFontType       != nFontType || 
		m_nCharSet        != nCharSet || 
		m_nPitchAndFamily != nPitchAndFamily)
	{
		m_nFontType       = nFontType;
		m_nCharSet        = nCharSet;
		m_nPitchAndFamily = nPitchAndFamily;

		RebuildFonts ();
	}
}
//****************************************************************************************
void CBCGPRibbonFontComboBox::RebuildFonts ()
{
	RemoveAllItems ();

	CObList lstFonts;

	CBCGPToolbarFontCombo tlbFontCombo (
		&lstFonts, m_nFontType, m_nCharSet, m_nPitchAndFamily);

	POSITION pos = NULL;

	for (pos = lstFonts.GetHeadPosition (); pos != NULL;)
	{
		CBCGPFontDesc* pDesc = (CBCGPFontDesc*) lstFonts.GetNext (pos);
		ASSERT_VALID (pDesc);

		if ((m_nFontType & pDesc->m_nType) != 0)
		{
			BOOL bIsUnique = GetFontsCount (pDesc->m_strName, lstFonts) <= 1;
			AddItem (bIsUnique ? pDesc->m_strName : pDesc->GetFullName (), (DWORD_PTR) pDesc);
		}
	}

	// Delete unused items:
	for (pos = lstFonts.GetHeadPosition (); pos != NULL;)
	{
		CBCGPFontDesc* pDesc = (CBCGPFontDesc*) lstFonts.GetNext (pos);
		ASSERT_VALID (pDesc);

		if ((m_nFontType & pDesc->m_nType) == 0)
		{
			delete pDesc;
		}
	}
}
//***************************************************************************************
int CBCGPRibbonFontComboBox::GetFontsCount (LPCTSTR lpszName,
											  const CObList& lstFonts)
{
	ASSERT (!lstFonts.IsEmpty ());

	int nCount = 0;

	for (POSITION pos = lstFonts.GetHeadPosition (); pos != NULL;)
	{
		CBCGPFontDesc* pDesc = (CBCGPFontDesc*) lstFonts.GetNext (pos);
		ASSERT_VALID (pDesc);

		if (pDesc->m_strName == lpszName)
		{
			nCount++;
		}
	}

	return nCount;
}
//***************************************************************************************
void CBCGPRibbonFontComboBox::ClearData ()
{
	ASSERT_VALID (this);

	if (m_pOriginal != NULL || m_bIsCustom)
	{
		return;
	}

	for (POSITION pos = m_lstItemData.GetHeadPosition (); pos != NULL;)
	{
		CBCGPFontDesc* pDesc = (CBCGPFontDesc*) m_lstItemData.GetNext (pos);
		ASSERT_VALID (pDesc);

		delete pDesc;
	}
}
//****************************************************************************************
static BOOL CompareFonts (const CBCGPFontDesc* pDesc, LPCTSTR lpszName, BYTE nCharSet, BOOL bExact)
{
	ASSERT_VALID (pDesc);

	CString strName = pDesc->GetFullName ();
	strName.MakeLower ();

	if (bExact)
	{
		if (strName == lpszName ||
			(pDesc->m_strName.CompareNoCase(lpszName) == 0 && 
			(nCharSet == pDesc->m_nCharSet || nCharSet == DEFAULT_CHARSET)))
		{
			return TRUE;
		}
	}
	else if (strName.Find (lpszName) == 0 && 
		(nCharSet == DEFAULT_CHARSET || pDesc->m_nCharSet == nCharSet))
	{
		return TRUE;
	}

	return FALSE;
}
//****************************************************************************************
BOOL CBCGPRibbonFontComboBox::SetFont (LPCTSTR lpszName, BYTE nCharSet, BOOL bExact)
{
	ASSERT_VALID (this);
	ASSERT (lpszName != NULL);

	CString strNameFind = lpszName;
	strNameFind.MakeLower ();

	const CBCGPFontDesc* pCurrFont = GetFontDesc ();
	if (pCurrFont != NULL && CompareFonts (pCurrFont, strNameFind, nCharSet, bExact))
	{
		// Font is already selected
		return TRUE;
	}

	for (POSITION pos = m_lstItemData.GetHeadPosition (); pos != NULL;)
	{
		CBCGPFontDesc* pDesc = (CBCGPFontDesc*) m_lstItemData.GetNext (pos);

		if (CompareFonts (pDesc, strNameFind, nCharSet, bExact))
		{
			SelectItem ((DWORD_PTR) pDesc);
			return TRUE;
		}
	}

	return FALSE;
}
//********************************************************************************
BOOL CBCGPRibbonFontComboBox::OnDrawDropListItem (CDC* pDC, 
											  int nIndex, 	
											  CBCGPToolbarMenuButton* pItem,
											  BOOL /*bHighlight*/)
{
	ASSERT_VALID (this);

	if (m_Images.GetCount() == 0)
	{
		CBCGPLocalResource locaRes;
		
		m_Images.SetImageSize(CSize(nImageWidth, nImageHeight));

		if (globalData.Is32BitIcons())
		{
			m_Images.Load(IDB_BCGBARRES_FONT32);
		}
		else
		{
			m_Images.SetTransparentColor(RGB (255, 255, 255));
			m_Images.Load(IDB_BCGBARRES_FONT);
		}
	}

	CRect rc = pItem->Rect ();

	CBCGPFontDesc* pDesc = (CBCGPFontDesc*) GetItemData (nIndex);
	LPCTSTR lpszText = GetItem (nIndex);

	if (pDesc == NULL || lpszText == NULL)
	{
		return FALSE;
	}

	ASSERT_VALID (pDesc);

	CFont fontSelected;
	CFont* pOldFont = NULL;

	if (pDesc->m_nType & (DEVICE_FONTTYPE | TRUETYPE_FONTTYPE))
	{
		m_Images.DrawEx(pDC, rc, pDesc->GetImageIndex (), CBCGPToolBarImages::ImageAlignHorzLeft, CBCGPToolBarImages::ImageAlignVertCenter);
	}

	rc.left += nImageWidth + nImageMargin;
	
	if (m_bDrawUsingFont && pDesc->m_nCharSet != SYMBOL_CHARSET)
	{
		LOGFONT lf;
		globalData.fontRegular.GetLogFont (&lf);

		lstrcpy (lf.lfFaceName, pDesc->m_strName);

		if (pDesc->m_nCharSet != DEFAULT_CHARSET)
		{
			lf.lfCharSet = pDesc->m_nCharSet;
		}

		if (lf.lfHeight < 0)
		{
			lf.lfHeight -= 4;
		}
		else
		{
			lf.lfHeight += 4;
		}

		fontSelected.CreateFontIndirect (&lf);
		pOldFont = pDC->SelectObject (&fontSelected);
	}

	CString strText = lpszText;

	pDC->DrawText (strText, rc, DT_SINGLELINE | DT_VCENTER);

	if (pOldFont != NULL)
	{
		pDC->SelectObject (pOldFont);
	}

	return TRUE;
}
//********************************************************************************
CSize CBCGPRibbonFontComboBox::OnGetDropListItemSize (
		CDC* /*pDC*/, 
		int /*nIndex*/,
		CBCGPToolbarMenuButton* /*pItem*/,
		CSize sizeDefault)
{
	ASSERT_VALID (this);

	CSize size = sizeDefault;
	size.cx += nImageWidth + nImageMargin;

	return size;
}

#endif // BCGP_EXCLUDE_RIBBON
