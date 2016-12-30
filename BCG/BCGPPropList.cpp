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
// BCGPPropList.cpp : implementation file
//

#include "stdafx.h"
#include "bcgcbpro.h"
#include "BCGPPropList.h"

#ifndef BCGP_EXCLUDE_PROP_LIST

#include "BCGPColorBar.h"
#include "BCGPWorkspace.h"
#include "BCGPShellManager.h"
#include "MenuImages.h"
#include "BCGPVisualManager.h"
#include "BCGPToolbarComboBoxButton.h"
#include "BCGPPopupMenu.h"

#include "BCGPDrawManager.h"
#include "BCGPMaskEdit.h"
#include "BCGPSpinButtonCtrl.h"
#include "BCGPContextMenuManager.h"
#include "trackmouse.h"
#include "BCGPLocalResource.h"
#include "BCGProRes.h"
#include "BCGPTagManager.h"
#include "BCGPLineStyleComboBox.h"
#include "BCGPToolbarEditBoxButton.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define ID_HEADER		1
#define ID_SCROLL_VERT	2
#define STRETCH_DELTA	2
#define ID_FILTER		4

#define TEXT_MARGIN		4

#define UM_UPDATESPIN	(WM_USER + 101)

extern CBCGPWorkspace* g_pWorkspace;
#define visualManager	CBCGPVisualManager::GetInstance ()

/////////////////////////////////////////////////////////////////////////////
// CBCGPProp

IMPLEMENT_DYNAMIC(CBCGPProp, CObject)

#define PROP_HAS_LIST	0x0001
#define PROP_HAS_BUTTON	0x0002
#define PROP_HAS_SPIN	0x0004

CString CBCGPProp::m_strFormatChar = _T("%c");
CString CBCGPProp::m_strFormatShort = _T("%d");
CString CBCGPProp::m_strFormatLong = _T("%ld");
CString CBCGPProp::m_strFormatUShort = _T("%u");
CString CBCGPProp::m_strFormatULong = _T("%u");
CString CBCGPProp::m_strFormatFloat = _T("%f");
CString CBCGPProp::m_strFormatDouble = _T("%lf");
CString	CBCGPProp::m_strScanFormatFloat = _T("%f");
CString	CBCGPProp::m_strScanFormatDouble = _T("%lf");
CLIPFORMAT CBCGPProp::m_cFormat = 0;

CBCGPProp::CBCGPProp(const CString& strName, const _variant_t& varValue, 
				   LPCTSTR lpszDescr, DWORD_PTR dwData, LPCTSTR lpszEditMask,
				   LPCTSTR lpszEditTemplate, LPCTSTR lpszValidChars) :
	m_strName (strName),
	m_varValue (varValue),
	m_varValueOrig (varValue),
	m_strDescr (lpszDescr == NULL ? _T("") : lpszDescr),
	m_strEditMask (lpszEditMask == NULL ? _T("") : lpszEditMask),
	m_strEditTempl (lpszEditTemplate == NULL ? _T("") : lpszEditTemplate),
	m_strValidChars (lpszValidChars == NULL ? _T("") : lpszValidChars),
	m_dwData (dwData)
{
	m_nID = (UINT)-1;
	m_bGroup = FALSE;
	m_bIsValueList = FALSE;

	Init ();
	SetFlags ();

	if (m_varValue.vt == VT_BOOL)
	{
		m_bAllowEdit = FALSE;
	}
}
//******************************************************************************************
CBCGPProp::CBCGPProp(const CString& strName, UINT nID, const _variant_t& varValue, 
				   LPCTSTR lpszDescr, DWORD_PTR dwData, LPCTSTR lpszEditMask,
				   LPCTSTR lpszEditTemplate, LPCTSTR lpszValidChars) :
	m_nID (nID),
	m_strName (strName),
	m_varValue (varValue),
	m_varValueOrig (varValue),
	m_strDescr (lpszDescr == NULL ? _T("") : lpszDescr),
	m_strEditMask (lpszEditMask == NULL ? _T("") : lpszEditMask),
	m_strEditTempl (lpszEditTemplate == NULL ? _T("") : lpszEditTemplate),
	m_strValidChars (lpszValidChars == NULL ? _T("") : lpszValidChars),
	m_dwData (dwData)
{
	m_bGroup = FALSE;
	m_bIsValueList = FALSE;

	Init ();
	SetFlags ();

	if (m_varValue.vt == VT_BOOL)
	{
		m_bAllowEdit = FALSE;
	}
}
//******************************************************************************************
CBCGPProp::CBCGPProp(const CString& strGroupName, DWORD_PTR dwData,
					 BOOL bIsValueList) :
	m_strName (strGroupName),
	m_dwData (dwData),
	m_bIsValueList (bIsValueList)
{
	m_nID = (UINT)-1;
	m_bGroup = TRUE;

	Init ();
	SetFlags ();
}
//****************************************************************************************
void CBCGPProp::SetFlags ()
{
	m_dwFlags = 0;

	switch (m_varValue.vt)
	{
	case VT_BSTR:
	case VT_R4:
	case VT_R8:
	case VT_UI1:
	case VT_I2:
	case VT_I4:
	case VT_I8:
	case VT_UI8:
	case VT_INT:
	case VT_UINT:
	case VT_UI2:
	case VT_UI4:
		break;

	case VT_DATE:
		break;

    case VT_BOOL:
		m_dwFlags = PROP_HAS_LIST;
		break;

	default:
		break;
	}
}
//******************************************************************************************
void CBCGPProp::Init ()
{
	m_pWndList = NULL;
	m_bExpanded = !m_bIsValueList;
	m_bEnabled = TRUE;
	m_pParent = NULL;
	m_pWndInPlace = NULL;
	m_pWndCombo = NULL;
	m_pWndSpin = NULL;
	m_bInPlaceEdit = FALSE;
	m_bButtonIsFocused = FALSE;
	m_bButtonIsHighlighted = FALSE;
	m_bMenuButtonIsHighlighted = FALSE;
	m_bButtonIsDown = FALSE;
	m_bAllowEdit = TRUE;
	m_bNameIsTrancated = FALSE;
	m_bValueIsTrancated = FALSE;

	m_Rect.SetRectEmpty ();
	m_rectButton.SetRectEmpty ();
	m_rectMenuButton.SetRectEmpty();

	m_nMinValue = 0;
	m_nMaxValue = 0;

	m_bIsModified = FALSE;
	m_bIsVisible = TRUE;
	m_bInFilter = TRUE;

	m_bHasState = FALSE;
	m_nStateWidth = 0;
	m_bDrawStateBorder = FALSE;
	m_clrState = (COLORREF)-1;
	m_cStateIndicator = _T('!');
	m_rectState.SetRectEmpty();

	m_nDropDownWidth = -1;
	m_bDrawMenuButton = FALSE;

	m_strButtonText = _T("...");
}
//*******************************************************************************************
CBCGPProp::~CBCGPProp()
{
	while (!m_lstSubItems.IsEmpty ())
	{
		delete m_lstSubItems.RemoveHead ();
	}

	OnDestroyWindow ();
}
//******************************************************************************************
void CBCGPProp::OnDestroyWindow ()
{
	if (m_pWndCombo != NULL)
	{
		m_pWndCombo->DestroyWindow ();
		delete m_pWndCombo;
		m_pWndCombo = NULL;
	}

	if (m_pWndInPlace != NULL)
	{
		m_pWndInPlace->DestroyWindow ();
		delete m_pWndInPlace;
		m_pWndInPlace = NULL;
	}

	if (m_pWndSpin != NULL)
	{
		m_pWndSpin->DestroyWindow ();
		delete m_pWndSpin;
		m_pWndSpin = NULL;
	}

	if (m_varValue.vt == VT_BOOL)
	{
		m_lstOptions.RemoveAll ();
	}
}
//****************************************************************************************
BOOL CBCGPProp::HasButton () const
{
	return	(m_dwFlags & PROP_HAS_LIST) ||
			(m_dwFlags & PROP_HAS_BUTTON);
}
//****************************************************************************************
BOOL CBCGPProp::AddSubItem (CBCGPProp* pProp)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pProp);

	if (!IsGroup ())
	{
		ASSERT(FALSE);
		return FALSE;
	}

	if (pProp->m_pWndList != NULL)
	{
		for (POSITION pos = pProp->m_pWndList->m_lstProps.GetHeadPosition (); pos != NULL;)
		{
			CBCGPProp* pListProp = pProp->m_pWndList->m_lstProps.GetNext (pos);
			ASSERT_VALID (pListProp);

			if (pListProp == pProp || pListProp->IsSubItem (pProp))
			{
				// Can't ad the same property twice
				ASSERT (FALSE);
				return FALSE;
			}
		}
	}

	pProp->m_pParent = this;

	m_lstSubItems.AddTail (pProp);
	pProp->m_pWndList = m_pWndList;

	return TRUE;
}
//*******************************************************************************************
BOOL CBCGPProp::RemoveSubItem (CBCGPProp*& pProp, BOOL bDelete/* = TRUE*/)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pProp);

	for (POSITION pos = m_lstSubItems.GetHeadPosition (); pos != NULL;)
	{
		POSITION posSaved = pos;

		CBCGPProp* pListProp = m_lstSubItems.GetNext (pos);
		ASSERT_VALID (pListProp);

		if (pListProp == pProp)
		{
			m_lstSubItems.RemoveAt (posSaved);

			if (m_pWndList != NULL && m_pWndList->m_pSel == pProp)
			{
				m_pWndList->m_pSel = NULL;
			}

			if (m_pWndList != NULL && m_pWndList->m_pTracked == pProp)
			{
				m_pWndList->m_pTracked = NULL;
			}

			if (bDelete)
			{
				delete pProp;
				pProp = NULL;
			}

			return TRUE;
		}

		if (pListProp->RemoveSubItem (pProp, bDelete))
		{
			return TRUE;
		}
	}

	return FALSE;
}
//*******************************************************************************************
void CBCGPProp::RemoveAllSubItems()
{
	ASSERT_VALID (this);
	
	while (!m_lstSubItems.IsEmpty ())
	{
		delete m_lstSubItems.RemoveHead ();
	}
}
//*******************************************************************************************
BOOL CBCGPProp::AddOption (LPCTSTR lpszOption, BOOL bInsertUnique/* = TRUE*/)
{
	ASSERT_VALID (this);
	ASSERT (lpszOption != NULL);

	if (bInsertUnique)
	{
 		if (m_lstOptions.Find (lpszOption) != NULL)
		{
			return FALSE;
		}
	}

	m_lstOptions.AddTail (lpszOption);
	m_dwFlags = PROP_HAS_LIST;

	return TRUE;
}
//****************************************************************************************
BOOL CBCGPPropList::DeleteProperty (CBCGPProp*& pProp, BOOL bRedraw, BOOL bAdjustLayout)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pProp);

	BOOL bFound = FALSE;

	for (POSITION pos = m_lstProps.GetHeadPosition (); pos != NULL;)
	{
		POSITION posSaved = pos;

		CBCGPProp* pListProp = m_lstProps.GetNext (pos);
		ASSERT_VALID (pListProp);

		if (pListProp == pProp)	// Top level property
		{
			bFound = TRUE;

			m_lstProps.RemoveAt (posSaved);
			break;
		}

		if (pListProp->RemoveSubItem (pProp, FALSE))
		{
			bFound = TRUE;
			break;
		}
	}

	if (!bFound)
	{
		return FALSE;
	}

	if (m_pSel == pProp)
	{
		m_pSel = NULL;
	}

	if (m_pTracked == pProp)
	{
		m_pTracked = NULL;
	}

	if (m_pSel != NULL)
	{
		for (CBCGPProp* pParent = m_pSel; pParent != NULL; pParent = pParent->GetParent())
		{
			if (pParent == pProp)
			{
				m_pSel = NULL;
				break;
			}
		}
	}

	if (m_pTracked != NULL)
	{
		for (CBCGPProp* pParent = m_pTracked; pParent != NULL; pParent = pParent->GetParent())
		{
			if (pParent == pProp)
			{
				m_pTracked = NULL;
				break;
			}
		}
	}

	delete pProp;
	pProp = NULL;

	if (bAdjustLayout)
	{
		AdjustLayout ();

		if (bRedraw && GetSafeHwnd () != NULL)
		{
			RedrawWindow ();
		}
	}

	return TRUE;
}
//*****************************************************************************************
void CBCGPProp::RemoveAllOptions ()
{
	ASSERT_VALID (this);

	m_lstOptions.RemoveAll ();
	m_dwFlags = 0;
}
//****************************************************************************************
int CBCGPProp::GetOptionCount () const
{
	ASSERT_VALID (this);
	return (int) m_lstOptions.GetCount ();
}
//****************************************************************************************
LPCTSTR CBCGPProp::GetOption (int nIndex) const
{
	ASSERT_VALID (this);

	if (nIndex < 0 || nIndex >= m_lstOptions.GetCount ())
	{
		ASSERT (FALSE);
		return NULL;
	}

	POSITION pos = m_lstOptions.FindIndex (nIndex);
	if (pos == NULL)
	{
		ASSERT (FALSE);
		return NULL;
	}

	return m_lstOptions.GetAt (pos);
}
//*****************************************************************************************
BOOL CBCGPProp::SelectOption(int nIndex)
{
	ASSERT_VALID (this);

	if (m_varValue.vt == VT_BOOL)
	{
		SetValue(nIndex != 0);
	}
	else
	{
		LPCTSTR lpszOption = nIndex < 0 ? _T("") : GetOption(nIndex);
		if (lpszOption == NULL)
		{
			return FALSE;
		}

		SetValue(lpszOption);
	}

	SetModifiedFlag();
	return TRUE;
}
//*****************************************************************************************
int CBCGPProp::GetSelectedOption() const
{
	ASSERT_VALID (this);

	if (m_varValue.vt == VT_BOOL)
	{
		return (bool)m_varValue ? 1 : 0;
	}

	CString strText = ((CBCGPProp&)*this).FormatProperty();

	int nIndex = 0;

	for (POSITION pos = m_lstOptions.GetHeadPosition(); pos != NULL; nIndex++)
	{
		if (strText == m_lstOptions.GetNext(pos))
		{
			return nIndex;
		}
	}

	return -1;
}
//*****************************************************************************************
int CBCGPProp::GetExpandedSubItems (BOOL bIncludeHidden) const
{
	ASSERT_VALID (this);

	if (!m_bExpanded)
	{
		return 0;
	}

	int nCount = 0;

	for (POSITION pos = m_lstSubItems.GetHeadPosition (); pos != NULL;)
	{
		CBCGPProp* pProp = m_lstSubItems.GetNext (pos);
		ASSERT_VALID (pProp);

		if (bIncludeHidden || pProp->IsVisibleInFilter())
		{
			nCount += pProp->GetExpandedSubItems (bIncludeHidden) + 1;
		}
	}

	return nCount;
}
//*******************************************************************************************
CBCGPProp* CBCGPProp::HitTest (CPoint point, ClickArea* pnArea)
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pWndList);

	if (m_Rect.PtInRect (point))
	{
		int xCenter = m_pWndList->m_rectList.left + m_pWndList->m_nLeftColumnWidth;

		if (pnArea != NULL)
		{
			if (IsGroup () && point.x < m_Rect.left + m_Rect.Height ())
			{
				*pnArea = ClickExpandBox;
			}
			else if (IsGroup () && !m_bIsValueList && m_pWndList->IsGroupNameFullWidth() && point.x > m_Rect.left + m_Rect.Height ())
			{
				*pnArea = ClickGroupArea;
			}
			else if (HasValueField () && point.x > xCenter)
			{
				*pnArea = ClickValue;
			}
			else
			{
				if (m_bDrawMenuButton && point.x > xCenter - m_Rect.Height())
				{
					*pnArea = ClickMenuButton;
				}
				else
				{
					*pnArea = ClickName;
				}
			}
		}

		return this;
	}

	for (POSITION pos = m_lstSubItems.GetHeadPosition (); pos != NULL;)
	{
		CBCGPProp* pProp = m_lstSubItems.GetNext (pos);
		ASSERT_VALID (pProp);

		CBCGPProp* pHit = pProp->HitTest (point, pnArea);
		if (pHit != NULL)
		{
			return pHit;
		}
	}

	return NULL;
}
//*******************************************************************************************
void CBCGPProp::SetFilter(const CString& strFilter)
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pWndList);

	if (m_lstSubItems.IsEmpty())
	{
		m_bInFilter = m_pWndList->IsPropertyMatchedToFilter(this, strFilter);
		return;
	}

	m_bInFilter = strFilter.IsEmpty();

	for (POSITION pos = m_lstSubItems.GetHeadPosition (); pos != NULL;)
	{
		CBCGPProp* pProp = m_lstSubItems.GetNext (pos);
		ASSERT_VALID (pProp);

		pProp->SetFilter(strFilter);

		if (pProp->m_bInFilter)
		{
			m_bInFilter = TRUE;
		}
	}

	if (!strFilter.IsEmpty() && m_bInFilter && !m_bExpanded)
	{
		m_bExpanded = TRUE;
	}
}
//*******************************************************************************************
void CBCGPProp::OnLeaveMouse()
{
	ASSERT_VALID (this);

	if (m_bButtonIsHighlighted)
	{
		m_bButtonIsHighlighted = FALSE;
		RedrawButton();
	}

	if (m_bMenuButtonIsHighlighted)
	{
		m_bMenuButtonIsHighlighted = FALSE;
		RedrawMenuButton();
	}

	for (POSITION pos = m_lstSubItems.GetHeadPosition (); pos != NULL;)
	{
		CBCGPProp* pProp = m_lstSubItems.GetNext (pos);
		ASSERT_VALID (pProp);

		pProp->OnLeaveMouse();
	}
}
//*******************************************************************************************
void CBCGPProp::Expand (BOOL bExpand)
{
	ASSERT_VALID (this);
	ASSERT (IsGroup ());

	if (m_bExpanded == bExpand ||
		m_lstSubItems.IsEmpty ())
	{
		return;
	}

	m_bExpanded = bExpand;

	if (m_pWndList != NULL && m_pWndList->GetSafeHwnd () != NULL)
	{
		ASSERT_VALID (m_pWndList);
		m_pWndList->AdjustLayout ();

		CRect rectRedraw = m_pWndList->m_rectList;
		rectRedraw.top = m_Rect.top;

		m_pWndList->RedrawWindow (rectRedraw);
	}
}
//*******************************************************************************************
void CBCGPProp::ExpandDeep (BOOL bExpand)
{
	ASSERT_VALID (this);

	m_bExpanded = bExpand;

	for (POSITION pos = m_lstSubItems.GetHeadPosition (); pos != NULL;)
	{
		CBCGPProp* pProp = m_lstSubItems.GetNext (pos);
		ASSERT_VALID (pProp);

		pProp->ExpandDeep (bExpand);
	}
}
//*******************************************************************************************
void CBCGPProp::ResetOriginalValue ()
{
	ASSERT_VALID (this);

	m_bIsModified = FALSE;

	SetValue (m_varValueOrig);

	for (POSITION pos = m_lstSubItems.GetHeadPosition (); pos != NULL;)
	{
		CBCGPProp* pProp = m_lstSubItems.GetNext (pos);
		ASSERT_VALID (pProp);

		pProp->ResetOriginalValue ();
	}
}
//*******************************************************************************************
void CBCGPProp::CommitModifiedValue ()
{
	ASSERT_VALID (this);

	m_bIsModified = FALSE;

	SetOriginalValue (m_varValue);

	for (POSITION pos = m_lstSubItems.GetHeadPosition (); pos != NULL;)
	{
		CBCGPProp* pProp = m_lstSubItems.GetNext (pos);
		ASSERT_VALID (pProp);

		pProp->CommitModifiedValue ();
	}
}
//*******************************************************************************************
void CBCGPProp::Redraw ()
{
	ASSERT_VALID (this);

	if (m_pWndList != NULL)
	{
		ASSERT_VALID (m_pWndList);
		m_pWndList->InvalidateRect (m_Rect);

		if (m_pParent != NULL && m_pParent->m_bIsValueList)
		{
			m_pWndList->InvalidateRect (m_pParent->m_Rect);
		}

		if (m_bIsValueList)
		{
			for (POSITION pos = m_lstSubItems.GetHeadPosition (); pos != NULL;)
			{
				CBCGPProp* pProp = m_lstSubItems.GetNext (pos);
				ASSERT_VALID (pProp);

				m_pWndList->InvalidateRect (pProp->m_Rect);
			}
		}

		m_pWndList->UpdateWindow ();
	}
}
//*******************************************************************************************
void CBCGPProp::RedrawButton()
{
	ASSERT_VALID (this);

	if (m_pWndList != NULL && !m_rectButton.IsRectEmpty())
	{
		ASSERT_VALID (m_pWndList);
		m_pWndList->RedrawWindow(m_rectButton);
	}
}
//*******************************************************************************************
void CBCGPProp::RedrawMenuButton()
{
	ASSERT_VALID (this);

	if (m_pWndList != NULL && !m_rectMenuButton.IsRectEmpty())
	{
		ASSERT_VALID (m_pWndList);
		m_pWndList->RedrawWindow(m_rectMenuButton);
	}
}
//*******************************************************************************************
void CBCGPProp::RedrawState()
{
	ASSERT_VALID (this);

	if (m_pWndList != NULL && !m_rectState.IsRectEmpty() && m_bHasState)
	{
		ASSERT_VALID (m_pWndList);
		m_pWndList->RedrawWindow(m_rectState);
	}
}
//*******************************************************************************************
void CBCGPProp::EnableSpinControl (BOOL bEnable, int nMin, int nMax)
{
	ASSERT_VALID (this);

	switch (m_varValue.vt)
	{
	case VT_INT:
	case VT_UINT:
    case VT_I2:
	case VT_I4:
    case VT_UI2:
	case VT_UI4:
		break;

	default:
		ASSERT (FALSE);
		return;
	}

	m_nMinValue = nMin;
	m_nMaxValue = nMax;

	if (bEnable)
	{
		m_dwFlags |= PROP_HAS_SPIN;
	}
	else
	{
		m_dwFlags &= ~PROP_HAS_SPIN;
	}
}
//*******************************************************************************************
BOOL CBCGPProp::IsSelected () const
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pWndList);

	return m_pWndList->m_pSel == this;
}
//******************************************************************************************
void CBCGPProp::SetName (LPCTSTR lpszName, BOOL bRedraw)
{
	ASSERT_VALID (this);

	if (lpszName == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	m_strName = lpszName;

	if (bRedraw)
	{
		Redraw ();
	}
}
//******************************************************************************************
void CBCGPProp::SetValue (const _variant_t& varValue)
{
	ASSERT_VALID (this);

	if (m_varValue.vt != VT_EMPTY && m_varValue.vt != varValue.vt)
	{
		ASSERT (FALSE);
		return;
	}

	BOOL bInPlaceEdit = m_bInPlaceEdit;
	if (bInPlaceEdit)
	{
		OnEndEdit ();
	}

	m_varValue = varValue;
	Redraw ();

	if (bInPlaceEdit)
	{
		ASSERT_VALID (m_pWndList);
		m_pWndList->EditItem (this);
	}
}
//*****************************************************************************************
void CBCGPProp::SetOriginalValue (const _variant_t& varValue)
{
	ASSERT_VALID (this);

	if (m_varValueOrig.vt != VT_EMPTY && m_varValueOrig.vt != varValue.vt)
	{
		ASSERT (FALSE);
		return;
	}

	m_varValueOrig = varValue;
}
//*****************************************************************************************
BOOL CBCGPProp::IsParentExpanded () const
{
	ASSERT_VALID (this);

	for (CBCGPProp* pProp = m_pParent; pProp != NULL;)
	{
		ASSERT_VALID (pProp);

		if (!pProp->IsExpanded ())
		{
			return FALSE;
		}

		pProp = pProp->m_pParent;
	}

	return TRUE;
}
//******************************************************************************************
int CBCGPProp::GetHierarchyLevel () const
{
	ASSERT_VALID (this);

	int nLevel = 0;
	for (CBCGPProp* pParent = m_pParent; pParent != NULL;
		pParent = pParent->m_pParent)
	{
		nLevel++;
	}

	return nLevel;
}
//*******************************************************************************************
CBCGPProp* CBCGPProp::GetSubItem (int nIndex) const
{
	ASSERT_VALID (this);

	if (nIndex < 0 || nIndex >= m_lstSubItems.GetCount ())
	{
		ASSERT (FALSE);
		return NULL;
	}

	return m_lstSubItems.GetAt (m_lstSubItems.FindIndex (nIndex));
}
//*******************************************************************************************
void CBCGPProp::Enable (BOOL bEnable/* = TRUE*/)
{
	ASSERT_VALID (this);

	if (m_bEnabled != bEnable)
	{
		m_bEnabled = bEnable;

		if (m_pWndList->GetSafeHwnd () != NULL)
		{
			ASSERT_VALID (m_pWndList);
			m_pWndList->InvalidateRect (m_Rect);
			m_pWndList->UpdateWindow();
		}
	}
}
//*******************************************************************************************
void CBCGPProp::SetOwnerList (CBCGPPropList* pWndList)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pWndList);

	m_pWndList = pWndList;

	for (POSITION pos = m_lstSubItems.GetHeadPosition (); pos != NULL;)
	{
		CBCGPProp* pProp = m_lstSubItems.GetNext (pos);
		ASSERT_VALID (pProp);

		pProp->SetOwnerList (m_pWndList);
	}
}
//*******************************************************************************************
void CBCGPProp::Repos (int& y)
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pWndList);

	CRect rectOld = m_Rect;
	
	BOOL bShowProperty = IsVisibleInFilter() && (IsParentExpanded () || m_pWndList->m_bAlphabeticMode);

	if (m_pWndList->m_bAlphabeticMode && m_pParent != NULL &&
		m_pParent->m_bIsValueList && !IsParentExpanded ())
	{
		bShowProperty = FALSE;
	}

	if (bShowProperty)
	{
		int dx = m_pWndList->m_bAlphabeticMode ?
			m_pWndList->m_nRowHeight :
			GetHierarchyLevel () * m_pWndList->m_nRowHeight;

		if (m_pWndList->m_bAlphabeticMode && m_bIsValueList)
		{
			dx = 0;
		}

		m_Rect = CRect (
			m_pWndList->m_rectList.left + dx,
			y, 
			m_pWndList->m_rectList.right, 
			y + m_pWndList->m_nRowHeight);

		if (IsButtonVisible())
		{
			AdjustButtonRect();
		}

		if (!m_rectButton.IsRectEmpty ())
		{
			m_rectButton.top = m_Rect.top + 1;
			m_rectButton.bottom = m_Rect.bottom;
		}

		y += m_pWndList->m_nRowHeight;

		CRect rectName = m_Rect;
		rectName.right = m_pWndList->m_rectList.left + m_pWndList->m_nLeftColumnWidth;

		if (IsWindow (m_pWndList->m_ToolTip.GetSafeHwnd ()))
		{
			m_pWndList->m_ToolTip.AddTool (m_pWndList, LPSTR_TEXTCALLBACK, rectName, m_pWndList->m_nTooltipsCount + 1);
			m_pWndList->m_nTooltipsCount ++;

			if (!IsGroup ())
			{
				CRect rectValue = m_Rect;
				rectValue.left = rectName.right + 1;
				m_pWndList->m_ToolTip.AddTool (m_pWndList, LPSTR_TEXTCALLBACK, rectValue, m_pWndList->m_nTooltipsCount + 1);

				m_pWndList->m_nTooltipsCount ++;
			}
		}
	}
	else
	{
		m_Rect.SetRectEmpty ();
		m_rectButton.SetRectEmpty ();
	}

	for (POSITION pos = m_lstSubItems.GetHeadPosition (); pos != NULL;)
	{
		CBCGPProp* pProp = m_lstSubItems.GetNext (pos);
		ASSERT_VALID (pProp);

		pProp->Repos (y);
	}

	OnPosSizeChanged (rectOld);
}
//******************************************************************************************
void CBCGPProp::AddTerminalProp (CList<CBCGPProp*, CBCGPProp*>& lstProps)
{
	ASSERT_VALID (this);

	if (!m_bGroup || m_bIsValueList)
	{
		// Insert sorted:
		BOOL bInserted = FALSE;
		for (POSITION pos = lstProps.GetHeadPosition (); !bInserted && pos != NULL;)
		{
			POSITION posSave = pos;

			CBCGPProp* pProp = lstProps.GetNext (pos);

			if (m_pWndList->CompareProps (pProp, this) > 0)
			{
				lstProps.InsertBefore (posSave, this);
				bInserted = TRUE;
			}
		}

		if (!bInserted)
		{
			lstProps.AddTail (this);
		}
		return;
	}

	m_Rect.SetRectEmpty ();

	for (POSITION pos = m_lstSubItems.GetHeadPosition (); pos != NULL;)
	{
		CBCGPProp* pProp = m_lstSubItems.GetNext (pos);
		ASSERT_VALID (pProp);

		pProp->AddTerminalProp (lstProps);
	}
}
//****************************************************************************************
BOOL CBCGPProp::IsSubItem (CBCGPProp* pSubProp) const
{
	ASSERT_VALID (this);
	ASSERT_VALID (pSubProp);

	for (POSITION pos = m_lstSubItems.GetHeadPosition (); pos != NULL;)
	{
		CBCGPProp* pProp = m_lstSubItems.GetNext (pos);
		ASSERT_VALID (pProp);

		if (pSubProp == pProp || pProp->IsSubItem (pSubProp))
		{
			return TRUE;
		}
	}

	return FALSE;
}
//******************************************************************************************
CBCGPProp* CBCGPProp::FindSubItemByData (DWORD_PTR dwData) const
{
	ASSERT_VALID (this);
	
	for (POSITION pos = m_lstSubItems.GetHeadPosition (); pos != NULL;)
	{
		CBCGPProp* pProp = m_lstSubItems.GetNext (pos);
        ASSERT_VALID (pProp);
		
        if (pProp->m_dwData == dwData)
        {
			return pProp;
		}
		
		pProp = pProp->FindSubItemByData (dwData);
		
		if (pProp != NULL)
		{
			return pProp;
        }
    }
	
	return NULL;
}
//******************************************************************************************
CBCGPProp* CBCGPProp::FindSubItemByID(UINT nID) const
{
	ASSERT_VALID (this);

	if (nID == (UINT)-1)
	{
		return NULL;
	}
	
	for (POSITION pos = m_lstSubItems.GetHeadPosition (); pos != NULL;)
	{
		CBCGPProp* pProp = m_lstSubItems.GetNext (pos);
        ASSERT_VALID (pProp);
		
        if (pProp->m_nID == nID)
        {
			return pProp;
		}
		
		pProp = pProp->FindSubItemByID(nID);
		
		if (pProp != NULL)
		{
			return pProp;
        }
    }
	
	return NULL;
}
//*****************************************************************************************
CString CBCGPProp::FormatProperty ()
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pWndList);

	_variant_t& var = m_varValue;

	CString strVal;

	if (m_bIsValueList)
	{
		for (POSITION pos = m_lstSubItems.GetHeadPosition (); pos != NULL;)
		{
			CBCGPProp* pProp = m_lstSubItems.GetNext (pos);
			ASSERT_VALID (pProp);

			strVal += pProp->FormatProperty ();

			if (pos != NULL)
			{
				strVal += m_pWndList->m_cListDelimeter;
				strVal += _T(' ');
			}
		}
		
		return strVal;
	}

	switch (var.vt)
	{
	case VT_BSTR:
		strVal = (LPCTSTR)(_bstr_t)var;
		break;

    case VT_I2:
		strVal.Format (m_strFormatShort, (short) var);
		break;

	case VT_I4:
	case VT_INT:
		strVal.Format (m_strFormatLong, (long) var);
		break;

	case VT_UI1:
		if ((BYTE) var != 0)
		{
			strVal.Format (m_strFormatChar, (TCHAR)(BYTE) var);
		}
		break;

    case VT_UI2:
		strVal.Format( m_strFormatUShort, var.uiVal);
		break;

	case VT_UINT:
	case VT_UI4:
		strVal.Format (m_strFormatULong, var.ulVal);
		break;

#if _MSC_VER >= 1500
	case VT_I8:
		{
			TCHAR szBuffer[32];
			_i64tot_s ((LONGLONG) var, szBuffer, 32, 10);
			strVal = szBuffer;
		}
		break;
		
	case VT_UI8:
		{
			TCHAR szBuffer[32];
			_ui64tot_s ((ULONGLONG) var, szBuffer, 32, 10);
			strVal = szBuffer;
		}
		break;
#endif
		
    case VT_R4:
		strVal.Format (m_strFormatFloat, (float) var);
		break;

    case VT_R8:
		strVal.Format (m_strFormatDouble, (double) var);
		break;

    case VT_BOOL:
		{
			bool bVal = (bool) var;
			strVal = bVal ? m_pWndList->m_strTrue : m_pWndList->m_strFalse;
		}
		break;

	default:
		// Unsupported type
		strVal = _T("*** error ***");
	}

	return strVal;
}
//****************************************************************************************
void CBCGPProp::OnDrawName (CDC* pDC, CRect rect)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);
	ASSERT_VALID (m_pWndList);

	COLORREF clrTextOld = (COLORREF)-1;

	if (IsSelected () && (!m_pWndList->m_bVSDotNetLook || !IsGroup () ||  m_bIsValueList))
	{
		CRect rectFill = rect;
		rectFill.top++;

		if (m_bDrawMenuButton)
		{
			rectFill.right += rectFill.Height();
		}

		if (m_bHasState)
		{
			rectFill.right += m_rectState.Width();
		}

		COLORREF clrText = visualManager->OnFillPropertyListSelectedItem(pDC, this, m_pWndList, rectFill, m_pWndList->m_bFocused);
		clrTextOld = pDC->SetTextColor (clrText);
	}

	if (m_pWndList->m_bVSDotNetLook && IsGroup () && !m_bIsValueList)
	{
		if (m_pWndList->m_clrGroupText != (COLORREF)-1)
		{
			clrTextOld = pDC->SetTextColor (m_pWndList->m_clrGroupText);
		}
		else
		{
			clrTextOld = pDC->SetTextColor (visualManager->GetPropListGroupTextColor (m_pWndList));
		}
	}

	if (m_pParent != NULL && m_pParent->m_bIsValueList)
	{
		rect.left += rect.Height ();
	}

	rect.DeflateRect (TEXT_MARGIN, 0);

	int nNameAlign = IsGroup() ? DT_LEFT : m_pWndList->m_nNameAlign;

	int nTextHeight = pDC->DrawText (m_strName, rect, 
		nNameAlign | DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX | DT_END_ELLIPSIS);

	m_bNameIsTrancated = pDC->GetTextExtent (m_strName).cx > rect.Width ();

	if (IsSelected () && m_pWndList->m_bVSDotNetLook && IsGroup () && !m_bIsValueList)
	{
		CRect rectFocus = rect;
		rectFocus.top = rectFocus.CenterPoint ().y - nTextHeight / 2;
		rectFocus.bottom = rectFocus.top + nTextHeight;
		rectFocus.right = 
			min (rect.right, rectFocus.left + pDC->GetTextExtent (m_strName).cx);
		rectFocus.InflateRect (2, 0);

		COLORREF clrShadow = m_pWndList->DrawControlBarColors () ?
			globalData.clrBarShadow : globalData.clrBtnShadow;

		pDC->Draw3dRect (rectFocus, clrShadow, clrShadow);
	}

	if (clrTextOld != (COLORREF)-1)
	{
		pDC->SetTextColor (clrTextOld);
	}
}
//******************************************************************************************
void CBCGPProp::OnDrawMenuButton(CDC* pDC, CRect rect)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);
	ASSERT_VALID (m_pWndList);

	m_pWndList->OnDrawMenuButton(pDC, this, rect, m_bMenuButtonIsHighlighted, IsModified());
}
//******************************************************************************************
void CBCGPProp::OnDrawStateIndicator(CDC* pDC, CRect rect)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);
	ASSERT_VALID (m_pWndList);

	m_pWndList->OnDrawStateIndicator(pDC, this, rect);
}
//******************************************************************************************
void CBCGPProp::OnDrawValue (CDC* pDC, CRect rect)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);
	ASSERT_VALID (m_pWndList);

	if ((IsGroup () && !m_bIsValueList) || !HasValueField ())
	{
		return;
	}

	CFont* pOldFont = NULL;
	if (IsModified () && m_pWndList->m_bMarkModifiedProperties)
	{
		pOldFont = pDC->SelectObject (&m_pWndList->m_fontBold);
	}

	CString strVal = FormatProperty ();

	rect.DeflateRect (TEXT_MARGIN, 0);

	pDC->DrawText (strVal, rect, 
		DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX | DT_END_ELLIPSIS);

	m_bValueIsTrancated = pDC->GetTextExtent (strVal).cx > rect.Width ();

	if (pOldFont != NULL)
	{
		pDC->SelectObject (pOldFont);
	}
}
//******************************************************************************************
void CBCGPProp::OnDrawButton (CDC* pDC, CRect rect)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);
	ASSERT_VALID (m_pWndList);

	CBCGPToolbarComboBoxButton button;

	if (m_dwFlags & PROP_HAS_LIST)
	{
		rect.DeflateRect (0, 1);

		visualManager->OnDrawPropListComboButton(pDC, rect, this, m_pWndList->DrawControlBarColors (), m_bButtonIsFocused, m_bEnabled, m_bButtonIsDown, m_bButtonIsHighlighted);
		return;
	}

	COLORREF clrText = visualManager->OnDrawPropListPushButton(pDC, rect, this, m_pWndList->DrawControlBarColors(), m_bButtonIsFocused, m_bEnabled, m_bButtonIsDown, m_bButtonIsHighlighted);

    CString str = m_strButtonText;
    if (str.IsEmpty())
    {
        return;
    }

	COLORREF clrTextOld = pDC->SetTextColor (clrText);

	rect.DeflateRect(2, 2);
	rect.bottom--;

	pDC->DrawText (str, rect, DT_SINGLELINE | DT_CENTER | DT_VCENTER);

	pDC->SetTextColor (clrTextOld);
}
//****************************************************************************************
void CBCGPProp::OnDrawExpandBox (CDC* pDC, CRect rect)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);
	ASSERT_VALID (m_pWndList);
	ASSERT (IsGroup ());

	CPoint ptCenter = rect.CenterPoint ();

	int nMaxBoxSize = 9;
	if (globalData.GetRibbonImageScale () != 1.)
	{
		nMaxBoxSize = (int)(.5 + nMaxBoxSize * globalData.GetRibbonImageScale ());
	}

	int nBoxSize = min (nMaxBoxSize, rect.Width ());

	rect = CRect (ptCenter, CSize (1, 1));
	rect.InflateRect (nBoxSize / 2, nBoxSize / 2);

	if (!visualManager->IsExpandingBoxTransparent())
	{
		if (m_bIsValueList)
		{
			if (m_pWndList->m_brBackground.GetSafeHandle () != NULL)
			{
				pDC->FillRect (rect, &m_pWndList->m_brBackground);
			}
			else
			{
				pDC->FillRect (rect, &globalData.brWindow);
			}
		}
	}

	COLORREF clrShadow = m_pWndList->DrawControlBarColors () ? 
		globalData.clrBarDkShadow : globalData.clrBtnDkShadow;
	COLORREF clrText = m_pWndList->DrawControlBarColors () ? 
		globalData.clrBarText : globalData.clrBtnText;

	visualManager->OnDrawExpandingBox (pDC, rect, 
		m_bExpanded && !m_lstSubItems.IsEmpty (),
		m_pWndList->m_bVSDotNetLook ? clrText : clrShadow);
}
//******************************************************************************************
void CBCGPProp::OnDrawDescription (CDC* pDC, CRect rect)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);
	ASSERT_VALID (m_pWndList);

	HFONT hOldFont = (HFONT) ::SelectObject (pDC->GetSafeHdc (), m_pWndList->m_fontBold.GetSafeHandle ());
	int nHeight = pDC->DrawText (m_strName, rect, DT_SINGLELINE | DT_NOPREFIX);

	::SelectObject (pDC->GetSafeHdc (), hOldFont);

	rect.top += nHeight + 2;

	pDC->DrawText (m_strDescr, rect, DT_WORDBREAK | DT_NOPREFIX | DT_END_ELLIPSIS);
}
//******************************************************************************************
BOOL CBCGPProp::OnUpdateValue ()
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pWndInPlace);
	ASSERT_VALID (m_pWndList);
	ASSERT (::IsWindow (m_pWndInPlace->GetSafeHwnd ()));

	CString strText;
	m_pWndInPlace->GetWindowText (strText);

	BOOL bRes = FALSE;
	BOOL bIsChanged = FormatProperty () != strText;

	if (m_bIsValueList)
	{
		CString strDelimeter (m_pWndList->m_cListDelimeter);

		for (int i = 0; !strText.IsEmpty () && i < GetSubItemsCount (); i++)
		{
			CString strItem = strText.SpanExcluding (strDelimeter);

			if (strItem.GetLength () + 1 > strText.GetLength ())
			{
				strText.Empty ();
			}
			else
			{
				strText = strText.Mid (strItem.GetLength () + 1);
			}

			strItem.TrimLeft ();
			strItem.TrimRight ();

			CBCGPProp* pSubItem = GetSubItem (i);
			ASSERT_VALID (pSubItem);

			pSubItem->TextToVar (strItem);
		}

		bRes = TRUE;
	}
	else
	{
		bRes = TextToVar (strText);
	}

	if (bRes && bIsChanged)
	{
		m_pWndList->OnPropertyChanged (this);
	}

	return bRes;
}
//******************************************************************************************
BOOL CBCGPProp::TextToVar (const CString& strText)
{
	CString strVal = strText;

	switch (m_varValue.vt)
	{
	case VT_BSTR:
		m_varValue = (LPCTSTR) strVal;
		return TRUE;

	case VT_UI1:
		m_varValue = strVal.IsEmpty () ? (BYTE) 0 : (BYTE) strVal[0];
		return TRUE;

	case VT_I2:
		m_varValue = (short) _ttoi (strVal);
		return TRUE;

	case VT_I4:
		m_varValue = _ttol (strVal);
		return TRUE;

	case VT_INT:
		m_varValue.intVal = _ttoi (strVal);
		return TRUE;

	case VT_UI2:
		m_varValue.uiVal = unsigned short (_ttoi (strVal));
		return TRUE;

	case VT_UINT:
	case VT_UI4:
		m_varValue.ulVal = unsigned long (_ttol (strVal));
		return TRUE;

#if _MSC_VER >= 1500
	case VT_I8:
		m_varValue = _variant_t ( _ttoi64 (strVal) );
		return TRUE;
		
	case VT_UI8:
		m_varValue = ULONGLONG (_ttoi64 (strVal));
		return TRUE;
#else
	case VT_I8:
	case VT_UI8:
		return FALSE;
#endif

	case VT_R4:
		{
			float fVal = 0.;
			
			strVal.TrimLeft ();
			strVal.TrimRight ();

			if (!strVal.IsEmpty ())
			{
#if _MSC_VER < 1400
				_stscanf (strVal, m_strScanFormatFloat, &fVal);
#else
				_stscanf_s (strVal, m_strScanFormatFloat, &fVal);
#endif
			}

			m_varValue = fVal;
		}
		return TRUE;

	case VT_R8:
		{
			double dblVal = 0.;

			strVal.TrimLeft ();
			strVal.TrimRight ();

			if (!strVal.IsEmpty ())
			{
#if _MSC_VER < 1400
				_stscanf (strVal, m_strScanFormatDouble, &dblVal);
#else
				_stscanf_s (strVal, m_strScanFormatDouble, &dblVal);
#endif
			}

			m_varValue = dblVal;
		}
		return TRUE;

	case VT_BOOL:
		strVal.TrimRight ();
		m_varValue = (bool) (strVal == m_pWndList->m_strTrue);
		return TRUE;
	}

	return FALSE;
}
//*****************************************************************************
BOOL CBCGPProp::IsValueChanged () const
{
	ASSERT_VALID (this);

	if (m_varValueOrig.vt != m_varValue.vt)
	{
		return FALSE;
	}

	const _variant_t& var = m_varValue;
	const _variant_t& var1 = m_varValueOrig;

	switch (m_varValue.vt)
	{
	case VT_BSTR:
		{
			CString str1 = (LPCTSTR)(_bstr_t)var;
			CString str2 = (LPCTSTR)(_bstr_t)var1;

			return str1 != str2;
		}
		break;

    case VT_I2:
		return (short) var != (short) var1;

	case VT_I4:
	case VT_INT:
		return (long) var != (long) var1;

	case VT_UI1:
		return (BYTE) var != (BYTE) var1;

    case VT_UI2:
		return var.uiVal != var1.uiVal;

#if _MSC_VER >= 1500
	case VT_I8:
		return var.llVal != var1.llVal;
	case VT_UI8:
		return var.ullVal != var1.ullVal;
#endif

	case VT_UINT:
	case VT_UI4:
		return var.ulVal != var1.ulVal;

    case VT_R4:
		return (float) var != (float) var1;

    case VT_R8:
		return (double) var != (double) var1;

    case VT_BOOL:
		return (bool) var != (bool) var1;
	}

	return FALSE;
}
//*****************************************************************************
BOOL CBCGPProp::OnEdit (LPPOINT /*lptClick*/)
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pWndList);

	if (!HasValueField ())
	{
		return FALSE;
	}

	m_pWndInPlace = NULL;

	CRect rectEdit;
	CRect rectSpin;

	AdjustInPlaceEditRect (rectEdit, rectSpin);

	BOOL bDefaultFormat = FALSE;
	m_pWndInPlace = CreateInPlaceEdit (rectEdit, bDefaultFormat);

	if (m_pWndInPlace != NULL)
	{
		if (bDefaultFormat)
		{
			m_pWndInPlace->SetWindowText (FormatProperty ());
		}

		if (m_dwFlags & PROP_HAS_LIST)
		{
			CRect rectCombo = m_Rect;
			rectCombo.left = rectEdit.left - 4;

			m_pWndCombo = CreateCombo (m_pWndList, rectCombo);
			ASSERT_VALID (m_pWndCombo);

			if (m_nDropDownWidth > 0)
			{
				m_pWndCombo->SetDroppedWidth (m_nDropDownWidth);
			}

			m_pWndCombo->SetFont (m_pWndList->GetFont ());

			//-----------------------------------------------------------------------
			// Synchronize bottom edge of the combobox with the property bottom edge:
			//-----------------------------------------------------------------------
			m_pWndCombo->GetWindowRect (rectCombo);
			m_pWndList->ScreenToClient (&rectCombo);

			int dy = rectCombo.Height () - m_Rect.Height ();

			m_pWndCombo->SetWindowPos (NULL, rectCombo.left,
				rectCombo.top - dy + 1, -1, -1, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

			if (m_varValue.vt == VT_BOOL)
			{
				m_lstOptions.AddTail (m_pWndList->m_strTrue);
				m_lstOptions.AddTail (m_pWndList->m_strFalse);
			}

			for (POSITION pos = m_lstOptions.GetHeadPosition (); pos != NULL;)
			{
				m_pWndCombo->AddString (m_lstOptions.GetNext (pos));
			}
		}

		if (m_dwFlags & PROP_HAS_SPIN)
		{
			m_pWndSpin = CreateSpinControl (rectSpin);
		}

		if (DYNAMIC_DOWNCAST(CBCGPDateTimeCtrl, m_pWndInPlace) == NULL)
		{
			m_pWndInPlace->SetFont (
				IsModified () && m_pWndList->m_bMarkModifiedProperties ? 
					&m_pWndList->m_fontBold : m_pWndList->GetFont ());
		}

		m_pWndInPlace->SetFocus ();
		
		if (!m_bAllowEdit)
		{
			m_pWndInPlace->HideCaret ();
		}		

		m_bInPlaceEdit = TRUE;
		return TRUE;
	}

	return FALSE;
}
//******************************************************************************************
void CBCGPProp::AdjustButtonRect ()
{
	ASSERT_VALID (this);

	m_rectButton = m_Rect;
	m_rectButton.right--;
	m_rectButton.left = m_rectButton.right - m_rectButton.Height();
	m_rectButton.top++;
}
//******************************************************************************************
void CBCGPProp::AdjustInPlaceEditRect (CRect& rectEdit, CRect& rectSpin)
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pWndList);

	rectEdit = m_Rect;
	rectEdit.DeflateRect (0, 2);

	int nMargin = m_pWndList->m_bMarkModifiedProperties && m_bIsModified ?
		m_pWndList->m_nBoldEditLeftMargin : m_pWndList->m_nEditLeftMargin;

	rectEdit.left = m_pWndList->m_rectList.left + m_pWndList->m_nLeftColumnWidth 
		+ TEXT_MARGIN - nMargin + 1;

	if (HasButton ())
	{
		AdjustButtonRect ();
		rectEdit.right = m_rectButton.left;
	}
	else
	{
		rectEdit.right -= 2;
	}

	if (m_dwFlags & PROP_HAS_SPIN)
	{
		rectSpin = m_Rect;
		rectSpin.right = rectEdit.right;
		rectSpin.left = rectSpin.right - rectSpin.Height ();
		rectSpin.top ++;
		rectEdit.right = rectSpin.left;
	}
	else
	{
		rectSpin.SetRectEmpty ();
	}
}
//******************************************************************************************
CWnd* CBCGPProp::CreateInPlaceEdit (CRect rectEdit, BOOL& bDefaultFormat)
{
	if (NoInplaceEdit())
	{
		return NULL;
	}

	switch (m_varValue.vt)
	{
	case VT_BSTR:
	case VT_R4:
	case VT_R8:
	case VT_UI1:
	case VT_I2:
	case VT_INT:
	case VT_UINT:
	case VT_I4:
	case VT_I8:
	case VT_UI8:
	case VT_UI2:
	case VT_UI4:
	case VT_BOOL:
		break;

	default:
		if (!m_bIsValueList)
		{
			return NULL;
		}
	}

	CEdit* pWndEdit = NULL;

	if (!m_strEditMask.IsEmpty () || !m_strEditTempl.IsEmpty () ||
		!m_strValidChars.IsEmpty ())
	{
		CBCGPMaskEdit* pWndEditMask = new CBCGPMaskEdit;
		pWndEditMask->EnableSetMaskedCharsOnly (FALSE);
		pWndEditMask->EnableGetMaskedCharsOnly (FALSE);

		if (!m_strEditMask.IsEmpty () && !m_strEditTempl.IsEmpty ())
		{
			pWndEditMask->EnableMask (m_strEditMask, m_strEditTempl, _T(' '));
		}

		if (!m_strValidChars.IsEmpty ())
		{
			pWndEditMask->SetValidChars (m_strValidChars);
		}

		pWndEdit = pWndEditMask;
	}
	else
	{
		pWndEdit = new CEdit;
	}

	DWORD dwStyle = WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL;

	if (!m_bEnabled || !m_bAllowEdit)
	{
		dwStyle |= ES_READONLY;
	}

	pWndEdit->Create (dwStyle, rectEdit, m_pWndList, BCGPROPLIST_ID_INPLACE);

	bDefaultFormat = TRUE;
	return pWndEdit;
}
//*****************************************************************************
CSpinButtonCtrl* CBCGPProp::CreateSpinControl (CRect rectSpin)
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pWndList);

	if (!m_bEnabled)
	{
		return NULL;
	}

	CSpinButtonCtrl* pWndSpin = new CBCGPSpinButtonCtrl;

	if (!pWndSpin->Create (
		WS_CHILD | WS_VISIBLE | UDS_ARROWKEYS | UDS_SETBUDDYINT | UDS_NOTHOUSANDS,
		rectSpin, m_pWndList, BCGPROPLIST_ID_INPLACE))
	{
		return NULL;
	}

	pWndSpin->SetBuddy (m_pWndInPlace);

	if (m_nMinValue != 0 || m_nMaxValue != 0)
	{
		pWndSpin->SetRange32 (m_nMinValue, m_nMaxValue);
	}

	return pWndSpin;
}
//*****************************************************************************
BOOL CBCGPProp::OnEndEdit ()
{
	ASSERT_VALID (this);

	m_bInPlaceEdit = FALSE;
	m_bButtonIsFocused = FALSE;

	OnDestroyWindow ();
	return TRUE;
}
//*****************************************************************************************
CComboBox* CBCGPProp::CreateCombo (CWnd* pWndParent, CRect rect)
{
	ASSERT_VALID (this);

	rect.bottom = rect.top + 400;

	CComboBox* pWndCombo = new CComboBox;
	if (!pWndCombo->Create (WS_CHILD | CBS_NOINTEGRALHEIGHT | CBS_DROPDOWNLIST | WS_VSCROLL, 
		rect, pWndParent, BCGPROPLIST_ID_INPLACE))
	{
		delete pWndCombo;
		return NULL;
	}

	return pWndCombo;
}
//****************************************************************************************
BOOL CBCGPProp::IsButtonVisible() const
{
	ASSERT_VALID (this);

	if (!HasButton())
	{
		return FALSE;
	}

	if (m_pWndList == NULL)
	{
		return FALSE;
	}

	ASSERT_VALID (m_pWndList);

	return m_pWndList->GetCurSel() == this || (m_pWndList->IsContextMenuEnabled() && (m_dwFlags & PROP_HAS_LIST) == 0);
}
//****************************************************************************************
void CBCGPProp::OnClickButton (CPoint /*point*/)
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pWndList);

	if (m_pWndCombo != NULL && m_bEnabled)
	{
		m_bButtonIsDown = TRUE;
		m_bButtonIsHighlighted = FALSE;
		Redraw ();

		OnSetSelectedComboItem();

		m_pWndCombo->SetFocus ();
		m_pWndCombo->ShowDropDown ();
	}
}
//****************************************************************************************
void CBCGPProp::OnSetSelectedComboItem()
{
	ASSERT_VALID (this);
	ASSERT_VALID(m_pWndCombo);
	ASSERT_VALID(m_pWndInPlace);

	CString str;
	m_pWndInPlace->GetWindowText (str);

	m_pWndCombo->SetCurSel (m_pWndCombo->FindStringExact (-1, str));
}
//****************************************************************************************
BOOL CBCGPProp::OnClickValue (UINT uiMsg, CPoint point)
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pWndList);

	if (m_pWndInPlace == NULL)
	{
		return FALSE;
	}

	ASSERT_VALID (m_pWndInPlace);

	if (m_pWndSpin != NULL)
	{
		ASSERT_VALID (m_pWndSpin);
		ASSERT (m_pWndSpin->GetSafeHwnd () != NULL);

		CRect rectSpin;
		m_pWndSpin->GetClientRect (rectSpin);
		m_pWndSpin->MapWindowPoints (m_pWndList, rectSpin);

		if (rectSpin.PtInRect (point))
		{
			m_pWndList->MapWindowPoints (m_pWndSpin, &point, 1); 

			m_pWndSpin->SendMessage (uiMsg, 0, MAKELPARAM (point.x, point.y));
			return TRUE;
		}
	}

	CPoint ptEdit = point;
	::MapWindowPoints (	m_pWndList->GetSafeHwnd (), 
						m_pWndInPlace->GetSafeHwnd (), &ptEdit, 1);

	m_pWndInPlace->SendMessage (uiMsg, 0, MAKELPARAM (ptEdit.x, ptEdit.y));
	return TRUE;
}
//****************************************************************************************
BOOL CBCGPProp::OnDblClick (CPoint /*point*/)
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pWndList);

	if (CBCGPPopupMenu::GetActiveMenu () != NULL)
	{
		return TRUE;
	}

	if ((m_pWndInPlace == NULL && !NoInplaceEdit()) || !m_bEnabled)
	{
		return FALSE;
	}

	if (m_pWndInPlace != NULL)
	{
		ASSERT (::IsWindow (m_pWndInPlace->GetSafeHwnd ()));

		if (m_lstOptions.GetCount () > 1)
		{
			CString strText;
			m_pWndInPlace->GetWindowText (strText);

			POSITION pos = m_lstOptions.Find (strText);
			if (pos == NULL)
			{
				return FALSE;
			}

			m_lstOptions.GetNext (pos);
			if (pos == NULL)
			{
				pos = m_lstOptions.GetHeadPosition ();
			}

			ASSERT (pos != NULL);
			strText = m_lstOptions.GetAt (pos);

			m_pWndInPlace->SetWindowText (strText);
			OnUpdateValue ();

			if (m_pWndInPlace->GetSafeHwnd() != NULL && !m_pWndInPlace->IsWindowVisible())
			{
				Redraw();
			}

			return TRUE;
		}
	}

	if (m_dwFlags & PROP_HAS_BUTTON)
	{
		CWaitCursor wait;

		CString strPrevVal = FormatProperty ();

		OnClickButton (CPoint (-1, -1));

		if (strPrevVal != FormatProperty ())
		{
			m_pWndList->OnPropertyChanged (this);
		}

		return TRUE;
	}

	return FALSE;
}
//*****************************************************************************************
void CBCGPProp::OnClickMenuButton (CPoint point)
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pWndList);

	if (m_pWndList->m_bTracking || m_pWndList->m_bTrackingDescr || m_pWndList->m_bTrackingCommands)
	{
		return;
	}

	if (::GetCapture () == m_pWndList->GetSafeHwnd ())
	{
		ReleaseCapture ();
	}

	m_bMenuButtonIsHighlighted = FALSE;
	RedrawMenuButton();

	const UINT nFlags = m_pWndList->GetItemMenuFlags();
	if (nFlags == 0 && m_pWndList->m_lstCustomMenuItems.IsEmpty())
	{
		return;
	}

	const UINT idReset		= (UINT) -102;
	const UINT idCopy		= (UINT) -103;
	const UINT idPaste		= (UINT) -104;
	const UINT idEdit		= (UINT) -105;
	const UINT idCommands	= (UINT) -106;
	const UINT idDescr		= (UINT) -107;
	const UINT idCustomFirst= (UINT) -108;

	CMenu menu;
	menu.CreatePopupMenu ();

	{
		CBCGPLocalResource locaRes;
		CString strItem;

		if (nFlags & BCGP_PROPLIST_MENU_RESET)
		{
			strItem.LoadString(IDS_BCGBARRES_PROPLIST_MENU_RESET);

			menu.AppendMenu (MF_STRING, idReset, strItem);
			if (!IsModified() || !IsEnabled())
			{
				menu.EnableMenuItem (idReset, MF_GRAYED);
			}
		}

		if (nFlags & BCGP_PROPLIST_MENU_COPY_PASTE)
		{
			strItem.LoadString(IDS_BCGBARRES_PROPLIST_MENU_COPY);
			menu.AppendMenu (MF_STRING, idCopy, strItem);

			strItem.LoadString(IDS_BCGBARRES_PROPLIST_MENU_PASTE);
			menu.AppendMenu (MF_STRING, idPaste, strItem);

			if (!IsCopyAvailable())
			{
				menu.EnableMenuItem (idCopy, MF_GRAYED);
			}

			if (!IsPasteAvailable() || !IsEnabled())
			{
				menu.EnableMenuItem (idPaste, MF_GRAYED);
			}
		}

		if (nFlags & BCGP_PROPLIST_MENU_EDIT)
		{
			strItem.LoadString(IDS_BCGBARRES_PROPLIST_MENU_EDIT);

			menu.AppendMenu (MF_STRING, idEdit, strItem);
			if (!IsEditAvailable() || !IsEnabled())
			{
				menu.EnableMenuItem (idEdit, MF_GRAYED);
			}
		}

		if (menu.GetMenuItemCount() > 0)
		{
			menu.AppendMenu (MF_SEPARATOR);
		}

		BOOL bAddSeparator = FALSE;

		if (!m_pWndList->m_lstCustomMenuItems.IsEmpty())
		{
			UINT nID = idCustomFirst;
			int nCustomMenuIndex = 0;

			for (POSITION pos = m_pWndList->m_lstCustomMenuItems.GetHeadPosition (); pos != NULL;)
			{
				CString strItem = m_pWndList->m_lstCustomMenuItems.GetNext (pos);
				menu.AppendMenu (MF_STRING, nID, strItem);

				UINT nFlags = OnGetMenuCustomItemState(nCustomMenuIndex);
				
				if (nFlags & MF_GRAYED)
				{
					menu.EnableMenuItem(nID, MF_GRAYED);
				}

				if (nFlags & MF_CHECKED)
				{
					menu.CheckMenuItem(nID, MF_CHECKED);
				}

				nID--;
				nCustomMenuIndex++;
			}

			bAddSeparator = TRUE;
			menu.AppendMenu (MF_SEPARATOR);
		}

		if (m_pWndList->HasCommands() && (nFlags & BCGP_PROPLIST_MENU_COMMANDS))
		{
			if (bAddSeparator)
			{
				menu.AppendMenu (MF_SEPARATOR);
				bAddSeparator = FALSE;
			}

			strItem.LoadString(IDS_BCGBARRES_PROPLIST_MENU_VIEW_COMMANDS);

			menu.AppendMenu (MF_STRING, idCommands, strItem);
			if (m_pWndList->AreCommandsVisible())
			{
				menu.CheckMenuItem (idCommands, MF_CHECKED);
			}
		}

		if (m_pWndList->HasCommands() && (nFlags & BCGP_PROPLIST_MENU_DESCRIPTION))
		{
			if (bAddSeparator)
			{
				menu.AppendMenu (MF_SEPARATOR);
				bAddSeparator = FALSE;
			}

			strItem.LoadString(IDS_BCGBARRES_PROPLIST_MENU_VIEW_DESCR);

			menu.AppendMenu (MF_STRING, idDescr, strItem);
			if (m_pWndList->IsDescriptionArea())
			{
				menu.CheckMenuItem (idDescr, MF_CHECKED);
			}
		}
	}

	HWND hwndThis = m_pWndList->GetSafeHwnd ();

	CPoint pt = point;
	if (pt == CPoint(-1, -1))
	{
		pt  = CPoint(
			m_pWndList->m_rectList.left + m_pWndList->m_nLeftColumnWidth - m_Rect.Height(), 
			m_Rect.bottom + 1);
	}

	m_pWndList->ClientToScreen(&pt);

	int nMenuResult = 0;
	
	if (g_pContextMenuManager != NULL)
	{
		nMenuResult = g_pContextMenuManager->TrackPopupMenu(menu, pt.x, pt.y, m_pWndList);
	}
	else
	{
		nMenuResult = ::TrackPopupMenu (menu, 
			TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD, 
			pt.x, pt.y, 0, m_pWndList->GetSafeHwnd (), NULL);
	}

	if (!::IsWindow (hwndThis))
	{
		return;
	}

	switch (nMenuResult)
	{
	case 0:
		return;

	case idReset:
		ResetOriginalValue();
		Redraw();

		m_pWndList->GetOwner ()->SendMessage (BCGM_PROPERTY_CHANGED, m_pWndList->GetDlgCtrlID (),
			LPARAM (this));
		break;

	case idCopy:
		DoCopy();
		break;

	case idPaste:
		DoPaste();
		break;

	case idEdit:
		DoEdit();
		break;

	case idCommands:
		m_pWndList->SetCommandsVisible(!m_pWndList->AreCommandsVisible());
		m_pWndList->GetOwner ()->SendMessage (BCGM_PROPERTYLIST_LAYOUT_CHANGED);
		break;

	case idDescr:
		m_pWndList->EnableDescriptionArea(!m_pWndList->IsDescriptionArea());
		m_pWndList->GetOwner ()->SendMessage (BCGM_PROPERTYLIST_LAYOUT_CHANGED);
		break;

	default:
		{
			int nIndex = idCustomFirst - nMenuResult;
			OnMenuCustomItem(nIndex);
		}
		break;
	}
}
//*****************************************************************************************
void CBCGPProp::OnMenuCustomItem(int nIndex)
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pWndList);
	ASSERT(m_pWndList->GetOwner()->GetSafeHwnd() != NULL);

	m_pWndList->GetOwner()->SendMessage(BCGM_PROPERTY_MENU_ITEM_SELECTED, (WPARAM)nIndex, (LPARAM)this);
}
//*****************************************************************************************
UINT CBCGPProp::OnGetMenuCustomItemState(int nIndex)
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pWndList);
	ASSERT(m_pWndList->GetOwner()->GetSafeHwnd() != NULL);

	return (UINT)m_pWndList->GetOwner()->SendMessage (BCGM_PROPERTY_GET_MENU_ITEM_STATE, (WPARAM)nIndex, (LPARAM)this);
}
//*****************************************************************************************
void CBCGPProp::OnSelectCombo ()
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pWndCombo);
	ASSERT_VALID (m_pWndInPlace);

	int iSelIndex = m_pWndCombo->GetCurSel ();
	if (iSelIndex >= 0)
	{
		CString str;
		m_pWndCombo->GetLBText (iSelIndex, str);
		m_pWndInPlace->SetWindowText (str);
		
		OnUpdateValue ();
	}
}
//****************************************************************************************
void CBCGPProp::OnCloseCombo ()
{
	ASSERT_VALID (this);

	m_bButtonIsDown = FALSE;
	Redraw ();

	ASSERT_VALID (m_pWndInPlace);
	m_pWndInPlace->SetFocus ();
}
//****************************************************************************************
BOOL CBCGPProp::OnSetCursor () const
{
	if (m_bInPlaceEdit)
	{
		return FALSE;
	}

	if (m_bIsValueList)
	{
		SetCursor (AfxGetApp ()->LoadStandardCursor (IDC_IBEAM));
		return TRUE;
	}

	switch (m_varValue.vt)
	{
	case VT_BSTR:
	case VT_R4:
	case VT_R8:
	case VT_UI1:
	case VT_I2:
	case VT_INT:
	case VT_UINT:
	case VT_I4:
	case VT_I8:
	case VT_UI8:
	case VT_UI2:
	case VT_UI4:
		SetCursor (AfxGetApp ()->LoadStandardCursor (IDC_IBEAM));
		return TRUE;
	}

	return FALSE;
}
//*******************************************************************************************
BOOL CBCGPProp::PushChar (UINT nChar)
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pWndList);
	ASSERT (m_pWndList->m_pSel == this);

	if (NoInplaceEdit())
	{
		return FALSE;
	}

	ASSERT_VALID (m_pWndInPlace);

	if (m_bIsValueList)
	{
		if (m_bEnabled && m_bAllowEdit &&
			nChar != _T('+') && nChar != _T('-'))
		{
			m_pWndInPlace->SetWindowText (_T(""));
			m_pWndInPlace->SendMessage (WM_CHAR, (WPARAM) nChar);
			return TRUE;
		}
	}

	switch (m_varValue.vt)
	{
	case VT_BSTR:
    case VT_R4:
    case VT_R8:
    case VT_UI1:
    case VT_I2:
	case VT_INT:
	case VT_UINT:
	case VT_I4:
	case VT_I8:
	case VT_UI8:
    case VT_UI2:
	case VT_UI4:
		if (m_bEnabled && m_bAllowEdit)
		{
			m_pWndInPlace->SetWindowText (_T(""));
			m_pWndInPlace->SendMessage (WM_CHAR, (WPARAM) nChar);
			return TRUE;
		}
	}

	if (!m_bAllowEdit)
	{
		if (nChar == VK_SPACE)
		{
			OnDblClick (CPoint (-1, -1));
		}
		else if (m_lstOptions.GetCount () > 1)
		{
			CString strText;
			m_pWndInPlace->GetWindowText (strText);

			POSITION pos = m_lstOptions.Find (strText);
			if (pos == NULL)
			{
				return FALSE;
			}

			POSITION posSave = pos;
			CString strChar ((TCHAR) nChar);
			strChar.MakeUpper ();

			m_lstOptions.GetNext (pos);

			while (pos != posSave)
			{
				if (pos == NULL)
				{
					pos = m_lstOptions.GetHeadPosition ();
				}

				if (pos == posSave)
				{
					break;
				}

				strText = m_lstOptions.GetAt (pos);
				
				CString strUpper = strText;
				strUpper.MakeUpper ();

				if (strUpper.Left (1) == strChar)
				{
					m_pWndInPlace->SetWindowText (strText);
					OnUpdateValue ();
					break;
				}

				m_lstOptions.GetNext (pos);
			}
		}
	}

	OnEndEdit ();

	if (::GetCapture () == m_pWndList->GetSafeHwnd ())
	{
		ReleaseCapture ();
	}

	return FALSE;
}
//*******************************************************************************************
CString CBCGPProp::GetNameTooltip ()
{
	ASSERT_VALID (this);
	return _T("");
}
//*******************************************************************************************
CString CBCGPProp::GetValueTooltip ()
{
	ASSERT_VALID (this);
	return _T("");
}
//*******************************************************************************************
HBRUSH CBCGPProp::OnCtlColor(CDC* pDC, UINT /*nCtlColor*/)
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pWndList);

	pDC->SetTextColor (m_pWndList->GetTextColor());

	if (m_pWndList->m_brBackground.GetSafeHandle () != NULL)
	{
		pDC->SetBkColor (m_pWndList->GetBkColor());
		return (HBRUSH) m_pWndList->m_brBackground.GetSafeHandle ();
	}

	switch (m_varValue.vt)
	{
	case VT_BSTR:
    case VT_R4:
    case VT_R8:
    case VT_UI1:
    case VT_I2:
	case VT_I4:
	case VT_I8:
	case VT_UI8:
	case VT_INT:
	case VT_UINT:
    case VT_UI2:
	case VT_UI4:
	case VT_BOOL:
		if (!m_bEnabled || !m_bAllowEdit)
		{
			if (!m_bEnabled)
			{
				pDC->SetTextColor(globalData.clrGrayedText);
			}

			pDC->SetBkColor (globalData.clrWindow);
			return (HBRUSH) globalData.brWindow.GetSafeHandle ();
		}
	}

	return NULL;
}
//***************************************************************************************
void CBCGPProp::SetModifiedFlag ()
{
	BOOL bIsModified = IsValueChanged ();

	if (m_bIsModified == bIsModified && !m_bIsValueList)
	{
		return;
	}

	m_bIsModified = bIsModified;

	if (m_pParent != NULL && m_pParent->m_bIsValueList)
	{
		if (bIsModified)
		{
			m_pParent->m_bIsModified = TRUE;
		}
		else
		{
			m_pParent->m_bIsModified = FALSE;

			for (POSITION pos = m_pParent->m_lstSubItems.GetHeadPosition (); pos != NULL;)
			{
				CBCGPProp* pSubItem = (CBCGPProp*) m_pParent->m_lstSubItems.GetNext (pos);
				ASSERT_VALID (pSubItem);

				if (pSubItem->m_bIsModified)
				{
					m_pParent->m_bIsModified = TRUE;
					break;
				}
			}
		}
	}

	for (POSITION pos = m_lstSubItems.GetHeadPosition (); pos != NULL;)
	{
		CBCGPProp* pSubItem = (CBCGPProp*) m_lstSubItems.GetNext (pos);
		ASSERT_VALID (pSubItem);

		pSubItem->SetModifiedFlag ();
	}

	if (m_pWndList != NULL && m_pWndList->m_bMarkModifiedProperties)
	{
		ASSERT_VALID (m_pWndList);
		OnPosSizeChanged (m_Rect);

		if (m_pWndInPlace->GetSafeHwnd () != NULL && DYNAMIC_DOWNCAST(CBCGPDateTimeCtrl, m_pWndInPlace) == NULL)
		{
			if (m_bIsModified)
			{
				m_pWndInPlace->SetFont (&m_pWndList->m_fontBold);
			}
			else
			{
				m_pWndInPlace->SetFont (m_pWndList->GetFont ());
			}

			CRect rectInPlace;
			m_pWndInPlace->GetWindowRect (&rectInPlace);
			m_pWndList->ScreenToClient (&rectInPlace);

			int nXOffset = 
				m_pWndList->m_nBoldEditLeftMargin - m_pWndList->m_nEditLeftMargin;

			if (m_bIsModified)
			{
				nXOffset = -nXOffset;
			}

			if (HasButton ())
			{
				AdjustButtonRect ();
				rectInPlace.right = m_rectButton.left;
			}

			m_pWndInPlace->SetWindowPos (NULL, 
				rectInPlace.left + nXOffset, rectInPlace.top, 
				rectInPlace.Width () - nXOffset, rectInPlace.Height (),
				SWP_NOZORDER | SWP_NOACTIVATE);
		}

		Redraw ();
	}
}
//*******************************************************************************************
void CBCGPProp::SetState(LPCTSTR lpszToolTip, TCHAR cIndicator, COLORREF clrState, BOOL bDrawBorder, int nStateWidth)
{
	ASSERT_VALID(this);

	m_bHasState = TRUE;
	m_nStateWidth = nStateWidth;
	m_strStateToolTip = lpszToolTip != NULL ? lpszToolTip : _T("");
	m_clrState = clrState;
	m_cStateIndicator = cIndicator;
	m_bDrawStateBorder = bDrawBorder;

	Redraw();
}
//*******************************************************************************************
void CBCGPProp::CleanState()
{
	m_bHasState = FALSE;
	m_strStateToolTip.Empty();

	Redraw();
}
//*******************************************************************************************
void CBCGPProp::Show (BOOL bShow/* = TRUE*/, BOOL bAdjustLayout/* = TRUE*/)
{
	ASSERT_VALID (this);

	if (m_bIsVisible == bShow)
	{
		return;
	}

	m_bIsVisible = bShow;

	for (POSITION pos = m_lstSubItems.GetHeadPosition (); pos != NULL;)
	{
		CBCGPProp* pProp = m_lstSubItems.GetNext (pos);
		ASSERT_VALID (pProp);

		pProp->Show (bShow, FALSE);
	}

	if (bAdjustLayout && m_pWndList != NULL && m_pWndList->GetSafeHwnd () != NULL)
	{
		ASSERT_VALID (m_pWndList);
		m_pWndList->AdjustLayout ();
	}
}
//*******************************************************************************************
BOOL CBCGPProp::OnActivateByTab ()
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pWndList);

	if (m_bInPlaceEdit && (m_dwFlags & PROP_HAS_BUTTON))
	{
		m_bButtonIsFocused = !m_bButtonIsFocused;
		m_pWndList->RedrawWindow (m_rectButton);
		return TRUE;
	}

	if (!m_bInPlaceEdit && m_pWndList->EditItem (this))
	{
		CEdit* pEdit = DYNAMIC_DOWNCAST (CEdit, m_pWndInPlace);
		if (::IsWindow (pEdit->GetSafeHwnd ()))
		{
			pEdit->SetSel (0, -1);
		}

		return TRUE;
	}

	return FALSE;
}
//*******************************************************************************************					
BOOL CBCGPProp::OnRotateListValue (BOOL bForward)
{
	if (m_pWndInPlace == NULL)
	{
		return FALSE;
	}

	CString strText;
	m_pWndInPlace->GetWindowText (strText);

	POSITION pos = m_lstOptions.Find (strText);
	if (pos == NULL)
	{
		return FALSE;
	}

	if (bForward)
	{
		m_lstOptions.GetNext (pos);
		if (pos == NULL)
		{
			pos = m_lstOptions.GetHeadPosition ();
		}
	}
	else
	{
		m_lstOptions.GetPrev (pos);
		if (pos == NULL)
		{
			pos = m_lstOptions.GetTailPosition ();
		}
	}

	if (pos == NULL)
	{
		return FALSE;
	}

	strText = m_lstOptions.GetAt (pos);

	m_pWndInPlace->SetWindowText (strText);
	OnUpdateValue ();

	CEdit* pEdit = DYNAMIC_DOWNCAST (CEdit, m_pWndInPlace);
	if (::IsWindow (pEdit->GetSafeHwnd ()))
	{
		pEdit->SetSel (0, -1);
	}
	return TRUE;
}
//**********************************************************************************
BOOL CBCGPProp::SetACCData (CWnd* pParent, CBCGPAccessibilityData& data)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pParent);

	data.Clear ();

	data.m_strAccName = GetName ();
	data.m_strDescription = GetDescription();

	BOOL bGroup = (IsGroup () && !m_bIsValueList);
	if (!bGroup)
	{
		data.m_strAccValue = FormatProperty ();
	}

	
	data.m_nAccHit = 1;
	data.m_nAccRole = ROLE_SYSTEM_ROW;

	data.m_bAccState = STATE_SYSTEM_FOCUSABLE|STATE_SYSTEM_SELECTABLE;

	if (IsSelected ())
	{
		data.m_bAccState |= STATE_SYSTEM_FOCUSED;
		data.m_bAccState |= STATE_SYSTEM_SELECTED;	
	}

	if (!IsEnabled () || bGroup)
	{
		data.m_bAccState |= STATE_SYSTEM_READONLY;
	}
	
	data.m_rectAccLocation = m_Rect;
	pParent->ClientToScreen (&data.m_rectAccLocation);

	return TRUE;
}
//**********************************************************************************
BOOL CBCGPProp::SerializeValue(CString& str)
{
	str = FormatProperty();
	return str != _T("*** error ***");
}
//**********************************************************************************
BOOL CBCGPProp::ParseValue(const CString& str)
{
	return TextToVar(str);
}
//**********************************************************************************
BOOL CBCGPProp::SerializeToBuffer(CString& str)
{
	CBCGPTagManager tm;

	CRuntimeClass* pRTI = GetRuntimeClass();
	if (pRTI == NULL || pRTI->m_lpszClassName == NULL)
	{
		TRACE0("CBCGPProp::SerializeToBuffer: the class should give runtime info\n");
		ASSERT(FALSE);
		return FALSE;
	}

	CString strVal;
	if (!SerializeValue(strVal))
	{
		return FALSE;
	}

	void* lpszClassName = (void*)pRTI->m_lpszClassName;

#ifdef _UNICODE
	const size_t nChars = strlen (pRTI->m_lpszClassName) + 1;
	LPWSTR lpChars = (LPWSTR) new WCHAR[nChars];
	lpszClassName = (void*)AfxA2WHelper (lpChars, pRTI->m_lpszClassName, (int)nChars);
#endif

	tm.WriteStringTag(_T("Class"), (LPCTSTR)lpszClassName);
	tm.WriteIntTag(_T("VarType"), m_varValue.vt);
	tm.WriteStringTag(_T("Value"), CBCGPTagManager::Entity_ToTag(strVal));

	str = tm.GetBuffer();

#ifdef _UNICODE
	if (lpChars != NULL)
	{
		delete [] lpChars;
	}
#endif

	return TRUE;
}
//**********************************************************************************
BOOL CBCGPProp::SerializeFromBuffer(const CString& str, BOOL bCheckOnly)
{
	CRuntimeClass* pRTI = GetRuntimeClass();
	if (pRTI == NULL || pRTI->m_lpszClassName == NULL)
	{
		TRACE0("CBCGPProp::SerializeFromBuffer: the class should give runtime info\n");
		ASSERT(FALSE);
		return FALSE;
	}

	void* lpszClassName = (void*)pRTI->m_lpszClassName;

#ifdef _UNICODE
	const size_t nChars = strlen (pRTI->m_lpszClassName) + 1;
	LPWSTR lpChars = (LPWSTR) new WCHAR[nChars];
	lpszClassName = (void*)AfxA2WHelper (lpChars, pRTI->m_lpszClassName, (int)nChars);
#endif

	CBCGPTagManager tm(str);

	CString strClass;
	tm.ReadString(_T("Class"), strClass);
	BOOL bEquals = strClass.Compare ((LPCTSTR)lpszClassName) == 0;

#ifdef _UNICODE
	if (lpChars != NULL)
	{
		delete [] lpChars;
	}
#endif

	if (!bEquals)
	{
		return FALSE;
	}

	int nVarType = -1;
	tm.ReadInt(_T("VarType"), nVarType);
	if (nVarType != m_varValue.vt)
	{
		return FALSE;
	}

	CString strVal;
	tm.ReadString(_T("Value"), strVal);
	CBCGPTagManager::Entity_FromTag(strVal);

	if (bCheckOnly)
	{
		return TRUE;
	}

	return ParseValue(strVal);
}
//**********************************************************************************
CLIPFORMAT CBCGPProp::GetClipboardFormat()
{
	if (m_cFormat == 0)	// Not registered yet
	{
		m_cFormat = (CLIPFORMAT)::RegisterClipboardFormat(_T("CBCGPPropCF"));
		ASSERT (m_cFormat != NULL);
	}

	return m_cFormat;
}
//**********************************************************************************
BOOL CBCGPProp::IsCopyAvailable() const
{
	if (!m_lstOptions.IsEmpty())
	{
		return FALSE;
	}

	switch (m_varValue.vt)
	{
	case VT_BSTR:
    case VT_I2:
	case VT_I4:
	case VT_I8:
	case VT_UI8:
	case VT_INT:
	case VT_UI1:
    case VT_UI2:
	case VT_UINT:
	case VT_UI4:
    case VT_R4:
    case VT_R8:
		return TRUE;

    case VT_BOOL:
		return FALSE;
	}

	return FALSE;
}
//**********************************************************************************
BOOL CBCGPProp::DoCopy()
{
	ASSERT_VALID (m_pWndList);

	HANDLE hclipData = NULL;

	try
	{
		if (!m_pWndList->OpenClipboard())
		{
			return FALSE;
		}

		if (::EmptyClipboard())
		{
			CString strText;
			if (SerializeToBuffer(strText))
			{
				HGLOBAL hClipbuffer = ::GlobalAlloc (GMEM_DDESHARE, (strText.GetLength () + 1) * sizeof (TCHAR));
				if (hClipbuffer != NULL)
				{
					LPTSTR lpszBuffer = (LPTSTR) GlobalLock (hClipbuffer);

					lstrcpy (lpszBuffer, (LPCTSTR) strText);

					::GlobalUnlock (hClipbuffer);

					hclipData = ::SetClipboardData (GetClipboardFormat(), hClipbuffer);
				}
			}
		}

		::CloseClipboard ();
	}
	catch (...)
	{
		CBCGPLocalResource localRes;
		AfxMessageBox (IDP_BCGBARRES_INTERLAL_ERROR);
	}

	return hclipData != NULL;
}
//**********************************************************************************
BOOL CBCGPProp::PasteInternal(BOOL bCheckOnly)
{
	if (!m_lstOptions.IsEmpty())
	{
		return FALSE;
	}

	COleDataObject data;
	if (!data.AttachClipboard())
	{
		return FALSE;
	}
	
	if (!data.IsDataAvailable(GetClipboardFormat()))
	{
		return FALSE;
	}

	ASSERT_VALID(m_pWndList);

	BOOL bRes = FALSE;

	try
	{
		if (!m_pWndList->OpenClipboard ())
		{
			return FALSE;
		}

		HGLOBAL hClipbuffer = ::GetClipboardData(GetClipboardFormat ());

		if (hClipbuffer != NULL) 
		{ 
			LPCTSTR lpText = (LPCTSTR)::GlobalLock(hClipbuffer);
			if (lpText != NULL) 
			{ 
				CString strText = lpText;
				bRes = SerializeFromBuffer(strText, bCheckOnly);

				if (bRes && !bCheckOnly)
				{
					m_pWndList->OnPropertyChanged(this);
					Redraw();
				}

				::GlobalUnlock(hClipbuffer);
			} 
		} 

		::CloseClipboard ();
	}
	catch (...)
	{
		AfxMessageBox (IDP_BCGBARRES_INTERLAL_ERROR);
	}

	return bRes;
}
//**********************************************************************************
BOOL CBCGPProp::IsPasteAvailable()
{
	if (!IsEnabled())
	{
		return FALSE;
	}

	return PasteInternal(TRUE);
}
//**********************************************************************************
BOOL CBCGPProp::DoPaste()
{
	return PasteInternal(FALSE);
}
//**********************************************************************************
BOOL CBCGPProp::IsEditAvailable()
{
	if (m_dwFlags & PROP_HAS_LIST)
	{
		return FALSE;
	}

	if (IsGroup() && !m_bIsValueList)
	{
		return FALSE;
	}

	return IsEnabled();
}
//**********************************************************************************
BOOL CBCGPProp::DoEdit()
{
	m_pWndList->EditItem (this);

	CEdit* pEdit = DYNAMIC_DOWNCAST (CEdit, m_pWndInPlace);
	if (::IsWindow (pEdit->GetSafeHwnd ()))
	{
		pEdit->SetSel (0, -1);
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPColorProp object

IMPLEMENT_DYNAMIC(CBCGPColorProp, CBCGPProp)

CBCGPColorProp::CBCGPColorProp(const CString& strName, const COLORREF& color, 
							   CPalette* pPalette, LPCTSTR lpszDescr, DWORD_PTR dwData, BOOL bUseD2DColors) :
	CBCGPProp (strName, _variant_t(), lpszDescr, dwData),
	m_Color (color),
	m_ColorOrig (color)
{
	if (pPalette == NULL && bUseD2DColors && globalData.m_nBitsPerPixel >= 16)
	{
		SetColors(CBCGPColor::GetRGBArray());
		m_nColumnsNumber = 14;
	}
	else
	{
		CBCGPColorBar::InitColors (pPalette, m_Colors);
		m_nColumnsNumber = 5;
	}

	m_varValue = (LONG) color;
	m_varValueOrig = (LONG) color;

	m_dwFlags = PROP_HAS_LIST;

	m_pPopup = NULL;
	m_bStdColorDlg = FALSE;
	m_ColorAutomatic = RGB (0, 0, 0);
}
//*****************************************************************************************
CBCGPColorProp::CBCGPColorProp(const CString& strName, UINT nID, const COLORREF& color, 
							   CPalette* pPalette, LPCTSTR lpszDescr, DWORD_PTR dwData, BOOL bUseD2DColors) :
	CBCGPProp (strName, nID, _variant_t(), lpszDescr, dwData),
	m_Color (color),
	m_ColorOrig (color)
{
	if (pPalette == NULL && bUseD2DColors && globalData.m_nBitsPerPixel >= 16)
	{
		SetColors(CBCGPColor::GetRGBArray());
		m_nColumnsNumber = 14;
	}
	else
	{
		CBCGPColorBar::InitColors (pPalette, m_Colors);
		m_nColumnsNumber = 5;
	}

	m_varValue = (LONG) color;
	m_varValueOrig = (LONG) color;

	m_dwFlags = PROP_HAS_LIST;

	m_pPopup = NULL;
	m_bStdColorDlg = FALSE;
	m_ColorAutomatic = RGB (0, 0, 0);
}
//*****************************************************************************************
CBCGPColorProp::~CBCGPColorProp()
{
}
//*******************************************************************************************
void CBCGPColorProp::OnDrawValue (CDC* pDC, CRect rect)
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pWndList);

	CRect rectColor = rect;

	rect.left += rect.Height ();
	CBCGPProp::OnDrawValue (pDC, rect);

	rectColor.right = rectColor.left + rectColor.Height ();
	rectColor.DeflateRect (1, 1);
	rectColor.top++;
	rectColor.left++;

	COLORREF clrBorder = m_pWndList->DrawControlBarColors () ? globalData.clrBarDkShadow : globalData.clrBtnDkShadow;
	
	if (IsEnabled())
	{
		CBrush br (m_Color == (COLORREF)-1 ? m_ColorAutomatic : m_Color);
		pDC->FillRect (rectColor, &br);
	}
	else
	{
		clrBorder = m_pWndList->DrawControlBarColors () ? globalData.clrBarShadow : globalData.clrBtnShadow;
	}

	pDC->Draw3dRect(rectColor, clrBorder, clrBorder);
}
//****************************************************************************************
void CBCGPColorProp::OnClickButton (CPoint /*point*/)
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pWndList);

	m_bButtonIsDown = TRUE;
	m_bButtonIsHighlighted = FALSE;
	Redraw ();

	CList<COLORREF,COLORREF> lstDocColors;

	m_pPopup = new CColorPopup (NULL, m_Colors, m_Color,
		NULL, NULL, NULL, lstDocColors,
		m_nColumnsNumber, m_ColorAutomatic);
	m_pPopup->SetPropList (m_pWndList);

	if (!m_strOtherColor.IsEmpty ())	// Other color button
	{
		m_pPopup->m_wndColorBar.EnableOtherButton (m_strOtherColor, !m_bStdColorDlg);
	}

	if (!m_strAutoColor.IsEmpty ())	// Automatic color button
	{
		m_pPopup->m_wndColorBar.EnableAutomaticButton (m_strAutoColor, m_ColorAutomatic);
	}

	CPoint pt (
		m_pWndList->m_rectList.left + m_pWndList->m_nLeftColumnWidth + 1, 
		m_rectButton.bottom + 1);
	m_pWndList->ClientToScreen (&pt);

	if (!m_pPopup->Create (m_pWndList, pt.x, pt.y, NULL, FALSE))
	{
		ASSERT (FALSE);
		m_pPopup = NULL;
	}
	else
	{
		m_pPopup->GetMenuBar()->SetFocus ();
	}
}
//******************************************************************************************
BOOL CBCGPColorProp::OnEdit (LPPOINT /*lptClick*/)
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pWndList);

	m_pWndInPlace = NULL;

	CRect rectEdit;
	CRect rectSpin;

	AdjustInPlaceEditRect (rectEdit, rectSpin);

	CBCGPMaskEdit* pWndEdit = new CBCGPMaskEdit;
	DWORD dwStyle = WS_VISIBLE | WS_CHILD;

	if (!m_bEnabled)
	{
		dwStyle |= ES_READONLY;
	}

	pWndEdit->SetValidChars(_T("01234567890ABCDEFabcdef"));

	pWndEdit->Create (dwStyle, rectEdit, m_pWndList, BCGPROPLIST_ID_INPLACE);
	m_pWndInPlace = pWndEdit;

	m_pWndInPlace->SetWindowText (FormatProperty ());

	m_pWndInPlace->SetFont (
		IsModified () && m_pWndList->m_bMarkModifiedProperties ? 
			&m_pWndList->m_fontBold : m_pWndList->GetFont ());
	m_pWndInPlace->SetFocus ();

	m_bInPlaceEdit = TRUE;
	return TRUE;
}
//****************************************************************************************
void CBCGPColorProp::AdjustInPlaceEditRect (CRect& rectEdit, CRect& rectSpin)
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pWndList);

	rectSpin.SetRectEmpty ();

	rectEdit = m_Rect;
	rectEdit.DeflateRect (0, 2);

	int nMargin = m_pWndList->m_bMarkModifiedProperties && m_bIsModified ?
		m_pWndList->m_nBoldEditLeftMargin : m_pWndList->m_nEditLeftMargin;

	rectEdit.left = m_pWndList->m_rectList.left + m_pWndList->m_nLeftColumnWidth + 
		m_Rect.Height () + TEXT_MARGIN - nMargin + 1;

	AdjustButtonRect ();
	rectEdit.right = m_rectButton.left;
}
//****************************************************************************************
void CBCGPColorProp::ResetOriginalValue ()
{
	CBCGPProp::ResetOriginalValue ();
	m_Color = m_ColorOrig;
}
//****************************************************************************************
void CBCGPColorProp::CommitModifiedValue ()
{
	CBCGPProp::CommitModifiedValue ();
	m_ColorOrig = m_Color;
}
//****************************************************************************************
CString CBCGPColorProp::FormatProperty ()
{
	ASSERT_VALID (this);

	if (m_Color == (COLORREF)-1)
	{
		return m_strAutoColor;
	}

	CString str;
	str.Format (_T("%02x%02x%02x"),
		GetRValue (m_Color), GetGValue (m_Color), GetBValue (m_Color));

	str.MakeUpper();

	return str;
}
//******************************************************************************************
BOOL CBCGPColorProp::ParseValue(const CString& str)
{
	if (str.IsEmpty())
	{
		return FALSE;
	}

	int nR = 0, nG = 0, nB = 0;
#if _MSC_VER < 1400
	_stscanf (str, _T("%2x%2x%2x"), &nR, &nG, &nB);
#else
	_stscanf_s (str, _T("%2x%2x%2x"), &nR, &nG, &nB);
#endif
	m_Color = RGB (nR, nG, nB);
	return TRUE;
}
//******************************************************************************************
void CBCGPColorProp::SetColor (COLORREF color)
{
	ASSERT_VALID (this);

	m_Color = color;
	m_varValue = (LONG) color;

	if (::IsWindow (m_pWndList->GetSafeHwnd())) 
	{
		CRect rect = m_Rect;
		rect.DeflateRect (0, 1);

		m_pWndList->InvalidateRect (rect);
		m_pWndList->UpdateWindow ();
	}

	if (m_pWndInPlace != NULL)
	{
		ASSERT_VALID (m_pWndInPlace);
		m_pWndInPlace->SetWindowText (FormatProperty ());
	}
}
//********************************************************************************
void CBCGPColorProp::SetColors(const CArray<COLORREF, COLORREF>& colors)
{
	ASSERT_VALID (this);

	m_Colors.RemoveAll();
	m_Colors.Append(colors);
}
//********************************************************************************
void CBCGPColorProp::SetColumnsNumber (int nColumnsNumber)
{
	ASSERT_VALID (this);
	ASSERT (nColumnsNumber > 0);

	m_nColumnsNumber = nColumnsNumber;
}
//*************************************************************************************
void CBCGPColorProp::EnableAutomaticButton (LPCTSTR lpszLabel, COLORREF colorAutomatic, BOOL bEnable)
{
	ASSERT_VALID (this);

	m_ColorAutomatic = colorAutomatic;
	m_strAutoColor = (!bEnable || lpszLabel == NULL) ? _T("") : lpszLabel;
}
//*************************************************************************************
void CBCGPColorProp::EnableOtherButton (LPCTSTR lpszLabel, BOOL bAltColorDlg, BOOL bEnable)
{
	ASSERT_VALID (this);

	m_bStdColorDlg = !bAltColorDlg;
	m_strOtherColor = (!bEnable || lpszLabel == NULL) ? _T("") : lpszLabel;
}
//*****************************************************************************************
BOOL CBCGPColorProp::OnUpdateValue ()
{
	ASSERT_VALID (this);

	if (m_pWndInPlace == NULL)
	{
		return FALSE;
	}

	ASSERT_VALID (m_pWndInPlace);
	ASSERT (::IsWindow (m_pWndInPlace->GetSafeHwnd ()));

	CString strText;
	m_pWndInPlace->GetWindowText (strText);

	COLORREF colorCurr = m_Color;

	if (!strText.IsEmpty())
	{
		ParseValue(strText);
	}

	if (colorCurr != m_Color)
	{
		m_pWndList->OnPropertyChanged (this);
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPBrushProp object

IMPLEMENT_DYNAMIC(CBCGPBrushProp, CBCGPProp)

CBCGPBrushProp::CBCGPBrushProp(const CString& strName, const CBCGPBrush& brush, 
							   CBCGPEditBrushOptions* pOptions,
							   LPCTSTR lpszDescr, DWORD_PTR dwData) :
	CBCGPProp (strName, _variant_t(), lpszDescr, dwData),
	m_Brush (brush),
	m_BrushOrig (brush)
{
	m_dwFlags |= PROP_HAS_BUTTON;
	m_hbmp = NULL;

	if (pOptions != NULL)
	{
		m_Options = *pOptions;
	}
}
//*****************************************************************************************
CBCGPBrushProp::CBCGPBrushProp(const CString& strName, UINT nID, const CBCGPBrush& brush, 
							   CBCGPEditBrushOptions* pOptions,
							   LPCTSTR lpszDescr, DWORD_PTR dwData) :
	CBCGPProp (strName, nID, _variant_t(), lpszDescr, dwData),
	m_Brush (brush),
	m_BrushOrig (brush)
{
	m_dwFlags |= PROP_HAS_BUTTON;
	m_hbmp = NULL;

	if (pOptions != NULL)
	{
		m_Options = *pOptions;
	}
}
//*****************************************************************************************
CBCGPBrushProp::~CBCGPBrushProp()
{
	if (m_hbmp != NULL)
	{
		::DeleteObject(m_hbmp);
	}
}
//*******************************************************************************************
void CBCGPBrushProp::OnDrawValue (CDC* pDC, CRect rect)
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pWndList);

	CRect rectColor = rect;

	rect.left += rect.Height ();
	CBCGPProp::OnDrawValue (pDC, rect);

	rectColor.right = rectColor.left + rectColor.Height ();
	rectColor.DeflateRect (1, 1);
	rectColor.top++;
	rectColor.left++;

	if (!IsEnabled())
	{
		COLORREF clrBorder = m_pWndList->DrawControlBarColors () ? globalData.clrBarShadow : globalData.clrBtnShadow;
		pDC->Draw3dRect (rectColor, clrBorder, clrBorder);
		return;
	}

	if (m_hbmp == NULL && !m_Brush.IsEmpty())
	{
		CBCGPGraphicsManager* pGM = m_pWndList->GetGraphicsManager();
		if (pGM != NULL)
		{
			const CBCGPSize size = rectColor.Size();

			m_hbmp = CBCGPDrawManager::CreateBitmap_32(size, NULL);
			if (m_hbmp != NULL)
			{
				CBCGPRect rect(CBCGPPoint(), size);

				CDC dcMem;
				dcMem.CreateCompatibleDC (NULL);

				HBITMAP hbmpOld = (HBITMAP) dcMem.SelectObject (m_hbmp);

				pGM->BindDC(&dcMem, rect);
				pGM->BeginDraw();

				pGM->FillRectangle(rect, CBCGPBrush(CBCGPColor::White));
				pGM->FillRectangle(rect, m_Brush);

				pGM->EndDraw();

				dcMem.SelectObject (hbmpOld);

				pGM->BindDC(NULL);
			}
		}
	}

	if (m_hbmp != NULL)
	{
		pDC->DrawState (rectColor.TopLeft(), rectColor.Size(), m_hbmp, DSS_NORMAL);
	}
	else
	{
		CPen pen (PS_SOLID, 0, RGB (255, 0, 0));
		CPen* pOldPen = pDC->SelectObject (&pen);

		CRect rectInt = rectColor;

		pDC->MoveTo (rectInt.right - 1, rectInt.top);
		pDC->LineTo (rectInt.left - 1, rectInt.bottom);

		pDC->SelectObject (pOldPen);
	}

	COLORREF clrBorder = m_pWndList->DrawControlBarColors () ? globalData.clrBarDkShadow : globalData.clrBtnDkShadow;
	pDC->Draw3dRect (rectColor, clrBorder, clrBorder);
}
//****************************************************************************************
void CBCGPBrushProp::OnClickButton (CPoint /*point*/)
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pWndList);

	m_bButtonIsDown = TRUE;
	m_bButtonIsHighlighted = FALSE;
	Redraw ();

	CBCGPBrush brSaved = m_Brush;

	CBCGPEditBrushDlg dlg(m_Brush, m_pWndList, &m_Options);
	BOOL bRes = (dlg.DoModal() == IDOK);

	if (bRes && m_hbmp != NULL)
	{
		::DeleteObject(m_hbmp);
		m_hbmp = NULL;
	}

	m_bButtonIsDown = FALSE;
	Redraw ();

	if (bRes && !m_Brush.CompareWith(brSaved))
	{
		m_pWndList->OnPropertyChanged(this);
	}

	m_pWndList->SetFocus();
}
//****************************************************************************************
void CBCGPBrushProp::ResetOriginalValue ()
{
	CBCGPProp::ResetOriginalValue ();
	
	m_Brush = m_BrushOrig;

	if (m_hbmp != NULL)
	{
		::DeleteObject(m_hbmp);
		m_hbmp = NULL;
	}

	Redraw();
}
//****************************************************************************************
void CBCGPBrushProp::CommitModifiedValue ()
{
	CBCGPProp::CommitModifiedValue ();
	m_BrushOrig = m_Brush;
}
//****************************************************************************************
CString CBCGPBrushProp::FormatProperty ()
{
	ASSERT_VALID (this);
	return m_Options.m_strLabel;
}
//******************************************************************************************
void CBCGPBrushProp::SetBrush (const CBCGPBrush& brush)
{
	ASSERT_VALID (this);

	m_Brush = brush;

	if (m_hbmp != NULL)
	{
		::DeleteObject(m_hbmp);
		m_hbmp = NULL;
	}

	Redraw();
}
//**********************************************************************************
BOOL CBCGPBrushProp::SerializeValue(CString& str)
{
	str = CBCGPTagManager::FormatBrush(m_Brush);
	return !str.IsEmpty();
}
//**********************************************************************************
BOOL CBCGPBrushProp::ParseValue(const CString& str)
{
	if (m_hbmp != NULL)
	{
		::DeleteObject(m_hbmp);
		m_hbmp = NULL;
	}

	return CBCGPTagManager::ParseBrush(str, m_Brush);
}
//**********************************************************************************
BOOL CBCGPBrushProp::DoEdit()
{
	OnClickButton (CPoint (-1, -1));
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPTextFormatProp object

IMPLEMENT_DYNAMIC(CBCGPTextFormatProp, CBCGPProp)

CBCGPTextFormatProp::CBCGPTextFormatProp(const CString& strName, const CBCGPTextFormat& tf,
								CBCGPTextFormatDlgOptions* pOptions,
							   LPCTSTR lpszDescr, DWORD_PTR dwData) :
	CBCGPProp (strName, _variant_t((LPCTSTR)tf.GetFontFamily()), lpszDescr, dwData),
	m_TextFormat (tf),
	m_TextFormatOrig (tf)
{
	m_dwFlags = PROP_HAS_BUTTON;
	m_bAllowEdit = FALSE;

	if (pOptions != NULL)
	{
		m_Options = *pOptions;
	}
}
//*****************************************************************************************
CBCGPTextFormatProp::CBCGPTextFormatProp(const CString& strName, UINT nID, const CBCGPTextFormat& tf,
								CBCGPTextFormatDlgOptions* pOptions,
								LPCTSTR lpszDescr, DWORD_PTR dwData) :
	CBCGPProp (strName, nID, _variant_t((LPCTSTR)tf.GetFontFamily()), lpszDescr, dwData),
	m_TextFormat (tf),
	m_TextFormatOrig (tf)
{
	m_dwFlags = PROP_HAS_BUTTON;
	m_bAllowEdit = FALSE;

	if (pOptions != NULL)
	{
		m_Options = *pOptions;
	}
}
//*****************************************************************************************
CBCGPTextFormatProp::~CBCGPTextFormatProp()
{
}
//****************************************************************************************
void CBCGPTextFormatProp::OnClickButton (CPoint /*point*/)
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pWndList);

	m_bButtonIsDown = TRUE;
	m_bButtonIsHighlighted = FALSE;
	Redraw ();

	CWaitCursor wait;

	CBCGPTextFormatDlg dlg(m_TextFormat, &m_Options, m_pWndList);
	if (dlg.DoModal () == IDOK)
	{
		if (m_pWndInPlace->GetSafeHwnd() != NULL)
		{
			m_pWndInPlace->SetWindowText (FormatProperty ());
		}
		else
		{
			m_varValue = (LPCTSTR) FormatProperty ();
		}

		m_pWndList->OnPropertyChanged (this);
	}

	if (m_pWndInPlace->GetSafeHwnd() != NULL)
	{
		m_pWndInPlace->SetFocus ();
	}
	else
	{
		m_pWndList->SetFocus ();
	}

	m_bButtonIsDown = FALSE;
	Redraw ();
}
//****************************************************************************************
void CBCGPTextFormatProp::ResetOriginalValue ()
{
	CBCGPProp::ResetOriginalValue ();
	
	m_TextFormat = m_TextFormatOrig;
	Redraw();
}
//****************************************************************************************
void CBCGPTextFormatProp::CommitModifiedValue ()
{
	CBCGPProp::CommitModifiedValue ();
	m_TextFormatOrig = m_TextFormat;
}
//****************************************************************************************
CString CBCGPTextFormatProp::FormatProperty ()
{
	ASSERT_VALID (this);

	CString str;
	double dblSize = m_TextFormat.GetFontSize();

	if (dblSize < 0.0)
	{
		CWindowDC dc (m_pWndList);

		int nLogY = dc.GetDeviceCaps (LOGPIXELSY);
		if (nLogY != 0)
		{
			str.Format( _T("%s(%i)"), m_TextFormat.GetFontFamily(), 
				MulDiv (72, -(int)dblSize, nLogY));
		}
		else
		{
			str = m_TextFormat.GetFontFamily();
		}
	}
	else
	{
		str.Format( _T("%s(%i)"), m_TextFormat.GetFontFamily(), (int)dblSize);
	}

	return str;
}
//******************************************************************************************
void CBCGPTextFormatProp::SetTextFormat(const CBCGPTextFormat& tf)
{
	ASSERT_VALID (this);

	m_TextFormat = tf;
	Redraw();
}
//**********************************************************************************
BOOL CBCGPTextFormatProp::SerializeValue(CString& str)
{
	str = CBCGPTagManager::FormatTextFormat(m_TextFormat);
	return !str.IsEmpty();
}
//**********************************************************************************
BOOL CBCGPTextFormatProp::ParseValue(const CString& str)
{
	return CBCGPTagManager::ParseTextFormat(str, m_TextFormat);
}
//**********************************************************************************
BOOL CBCGPTextFormatProp::DoEdit()
{
	OnClickButton (CPoint (-1, -1));
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPLineStyleProp object

IMPLEMENT_DYNAMIC(CBCGPLineStyleProp, CBCGPProp)

CBCGPLineStyleProp::CBCGPLineStyleProp(const CString& strName, CBCGPStrokeStyle::BCGP_DASH_STYLE style, 
							   LPCTSTR lpszDescr, DWORD_PTR dwData) :
	CBCGPProp (strName, _variant_t((long)style), lpszDescr, dwData)
{
	CommonInit();
}
//*****************************************************************************************
CBCGPLineStyleProp::CBCGPLineStyleProp(const CString& strName, UINT nID, CBCGPStrokeStyle::BCGP_DASH_STYLE style, 
							   LPCTSTR lpszDescr, DWORD_PTR dwData) :
	CBCGPProp (strName, nID, _variant_t((long)style), lpszDescr, dwData)
{
	CommonInit();
}
//*****************************************************************************************
void CBCGPLineStyleProp::CommonInit()
{
	RemoveAllOptions();

	CString strNames;

	{
		CBCGPLocalResource locaRes;
		strNames.LoadString(IDS_BCGBARRES_LINE_STYLES);
	}

	while (!strNames.IsEmpty())
	{
		CString strName;
		int nIndex = strNames.Find(_T('\n'));
		if (nIndex >= 0)
		{
			strName = strNames.Left(nIndex);
			strNames = strNames.Mid(nIndex + 1);
		}
		else
		{
			strName = strNames;
			strNames.Empty();
		}

		if (!strName.IsEmpty())
		{
			AddOption (strName);
		}
	}

	AllowEdit (FALSE);
}
//*****************************************************************************************
CBCGPLineStyleProp::~CBCGPLineStyleProp()
{
}
//**********************************************************************************
void CBCGPLineStyleProp::SetStyle(CBCGPStrokeStyle::BCGP_DASH_STYLE style)
{
	ASSERT_VALID (this);

	m_varValue = (long)style;
	Redraw();
}
//**********************************************************************************
CComboBox* CBCGPLineStyleProp::CreateCombo (CWnd* pWndParent, CRect rect)
{
	rect.bottom = rect.top + 400;

	CBCGPLineStyleComboBox* pWndCombo = new CBCGPLineStyleComboBox();
	pWndCombo->m_bAutoFillItems = FALSE;

	if (!pWndCombo->Create (WS_CHILD | CBS_NOINTEGRALHEIGHT | CBS_DROPDOWNLIST | WS_VSCROLL | CBS_OWNERDRAWFIXED | CBS_HASSTRINGS,
		rect, pWndParent, BCGPROPLIST_ID_INPLACE))
	{
		delete pWndCombo;
		return NULL;
	}

	return pWndCombo;
}
//****************************************************************************************
void CBCGPLineStyleProp::OnSetSelectedComboItem()
{
	ASSERT_VALID (this);
	ASSERT_VALID(m_pWndCombo);

	m_pWndCombo->SetCurSel((long)m_varValue);
}
//**********************************************************************************
BOOL CBCGPLineStyleProp::TextToVar (const CString& strText)
{
	long nIndex = 0;

	for (POSITION pos = m_lstOptions.GetHeadPosition(); pos != NULL; nIndex++)
	{
		if (strText == m_lstOptions.GetNext(pos))
		{
			m_varValue = nIndex;
			return TRUE;
		}
	}

	return FALSE;
}
//**********************************************************************************
CString CBCGPLineStyleProp::FormatProperty()
{
	return GetOption((long)m_varValue);
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPFileProp object

IMPLEMENT_DYNAMIC(CBCGPFileProp, CBCGPProp)

CBCGPFileProp::CBCGPFileProp(const CString& strName, const CString& strFolderName, DWORD_PTR dwData, LPCTSTR lpszDescr) :
	CBCGPProp (strName, _variant_t((LPCTSTR)strFolderName), lpszDescr, dwData),
	m_bIsFolder (TRUE)
{
	m_dwFlags = PROP_HAS_BUTTON;
}
//**********************************************************************************
CBCGPFileProp::CBCGPFileProp(const CString& strName, UINT nID, const CString& strFolderName, DWORD_PTR dwData, LPCTSTR lpszDescr) :
	CBCGPProp (strName, nID, _variant_t((LPCTSTR)strFolderName), lpszDescr, dwData),
	m_bIsFolder (TRUE)
{
	m_dwFlags = PROP_HAS_BUTTON;
}
//**********************************************************************************
CBCGPFileProp::CBCGPFileProp (	const CString& strName, 
								BOOL bOpenFileDialog,
								const CString& strFileName, 
								LPCTSTR lpszDefExt,
								DWORD dwFileFlags, 
								LPCTSTR lpszFilter,
								LPCTSTR lpszDescr,
								DWORD_PTR dwData) :
	CBCGPProp (strName, _variant_t((LPCTSTR)strFileName), lpszDescr, dwData),
	m_bOpenFileDialog (bOpenFileDialog),
	m_dwFileOpenFlags (dwFileFlags)
{
	m_dwFlags = PROP_HAS_BUTTON;
	m_strDefExt = lpszDefExt == NULL ? _T("") : lpszDefExt;
	m_strFilter = lpszFilter == NULL ? _T("") : lpszFilter;

	m_bIsFolder  = FALSE;
}
//**********************************************************************************
CBCGPFileProp::CBCGPFileProp (	const CString& strName, 
								UINT nID,
								BOOL bOpenFileDialog,
								const CString& strFileName, 
								LPCTSTR lpszDefExt,
								DWORD dwFileFlags, 
								LPCTSTR lpszFilter,
								LPCTSTR lpszDescr,
								DWORD_PTR dwData) :
	CBCGPProp (strName, nID, _variant_t((LPCTSTR)strFileName), lpszDescr, dwData),
	m_bOpenFileDialog (bOpenFileDialog),
	m_dwFileOpenFlags (dwFileFlags)
{
	m_dwFlags = PROP_HAS_BUTTON;
	m_strDefExt = lpszDefExt == NULL ? _T("") : lpszDefExt;
	m_strFilter = lpszFilter == NULL ? _T("") : lpszFilter;

	m_bIsFolder  = FALSE;
}
//*****************************************************************************************
CBCGPFileProp::~CBCGPFileProp()
{
}
//****************************************************************************************
void CBCGPFileProp::OnClickButton (CPoint /*point*/)
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pWndList);
	ASSERT_VALID (m_pWndInPlace);
	ASSERT (::IsWindow (m_pWndInPlace->GetSafeHwnd ()));

	m_bButtonIsDown = TRUE;
	m_bButtonIsHighlighted = FALSE;
	Redraw ();

	CString strPath = (LPCTSTR)(_bstr_t)m_varValue;
	BOOL bUpdate = FALSE;

	if (m_bIsFolder)
	{
		if (g_pShellManager == NULL)
		{
			if (g_pWorkspace != NULL)
			{
				g_pWorkspace->InitShellManager ();
			}
		}

		if (g_pShellManager == NULL)
		{
			ASSERT (FALSE);
		}
		else
		{
			bUpdate = g_pShellManager->BrowseForFolder (strPath, m_pWndList, strPath);
		}
	}
	else
	{
		CFileDialog dlg (m_bOpenFileDialog, m_strDefExt, strPath, m_dwFileOpenFlags, m_strFilter,
			m_pWndList);
		if (dlg.DoModal () == IDOK)
		{
			bUpdate = TRUE;
			strPath = dlg.GetPathName ();
		}
	}

	if (bUpdate)
	{
		if (m_pWndInPlace != NULL)
		{
			m_pWndInPlace->SetWindowText (strPath);
		}

		m_varValue = (LPCTSTR) strPath;
	}

	m_bButtonIsDown = FALSE;
	Redraw ();

	if (m_pWndInPlace != NULL)
	{
		m_pWndInPlace->SetFocus ();
	}
	else
	{
		m_pWndList->SetFocus ();
	}
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPDateTimeProp object

IMPLEMENT_DYNAMIC(CBCGPDateTimeProp, CBCGPProp)

CBCGPDateTimeProp::CBCGPDateTimeProp(const CString& strName, const COleDateTime& date, 
		LPCTSTR lpszDescr/* = NULL*/, DWORD_PTR dwData/* = 0*/,
		UINT nFlags/* = CBCGPDateTimeCtrl::DTM_DATE | CBCGPDateTimeCtrl::DTM_TIME | CBCGPDateTimeCtrl::DTM_DROPCALENDAR*/) :
	CBCGPProp (strName, _variant_t(date, VT_DATE), lpszDescr, dwData)
{
	m_nFlags = nFlags;
}
//*****************************************************************************************
CBCGPDateTimeProp::CBCGPDateTimeProp(const CString& strName, UINT nID, const COleDateTime& date, 
		LPCTSTR lpszDescr/* = NULL*/, DWORD_PTR dwData/* = 0*/,
		UINT nFlags/* = CBCGPDateTimeCtrl::DTM_DATE | CBCGPDateTimeCtrl::DTM_TIME | CBCGPDateTimeCtrl::DTM_DROPCALENDAR*/) :
	CBCGPProp (strName, nID, _variant_t(date, VT_DATE), lpszDescr, dwData)
{
	m_nFlags = nFlags;
}
//*****************************************************************************************
CBCGPDateTimeProp::~CBCGPDateTimeProp()
{
}
//****************************************************************************************
void CBCGPDateTimeProp::OnDrawValue (CDC* pDC, CRect rect)
{
	ASSERT_VALID (m_pWndList);

	rect.DeflateRect (0, 2);

	rect.left = m_pWndList->GetListRect ().left + 
				m_pWndList->GetPropertyColumnWidth () + TEXT_MARGIN + 1;

	DRAWITEMSTRUCT dis;
	memset (&dis, 0, sizeof (DRAWITEMSTRUCT));

	dis.CtlType = ODT_BUTTON;
	dis.hDC = pDC->GetSafeHdc ();
	dis.rcItem = rect;

	m_wndDateTime.m_bDrawDateTimeOnly = !IsSelected () || !m_bEnabled;
	
	COLORREF clrTextOld = pDC->GetTextColor ();

	m_wndDateTime.SetTextColor (m_bEnabled ? 
		m_pWndList->GetTextColor () : visualManager->GetPropListDisabledTextColor(m_pWndList), FALSE);

	m_wndDateTime.SetBackgroundColor (m_pWndList->GetBkColor(), FALSE);

	m_wndDateTime.SetFont (
		IsModified () && m_pWndList->m_bMarkModifiedProperties ?
			&m_pWndList->m_fontBold : m_pWndList->GetFont ());
	
	m_wndDateTime.DrawItem (&dis);

	pDC->SetTextColor (clrTextOld);
}
//*****************************************************************************
CWnd* CBCGPDateTimeProp::CreateInPlaceEdit (CRect rectEdit, BOOL& bDefaultFormat)
{
	ASSERT_VALID (m_pWndList);

	CBCGPDateTimeCtrl* pDateTime = new CBCGPDateTimeCtrl;
	ASSERT_VALID (pDateTime);

	pDateTime->SetAutoResize (FALSE);
	pDateTime->m_bPropListMode = TRUE;
	pDateTime->EnableVisualManagerStyle(m_pWndList->DrawControlBarColors());

	CRect rectSpin;
	AdjustInPlaceEditRect (rectEdit, rectSpin);

	pDateTime->Create (_T(""), WS_CHILD | WS_VISIBLE, rectEdit, 
		m_pWndList, BCGPROPLIST_ID_INPLACE);
	pDateTime->SetFont (
		IsModified () && m_pWndList->m_bMarkModifiedProperties ? 
			&m_pWndList->m_fontBold : m_pWndList->GetFont ());

	SetCtrlState(*pDateTime);
	pDateTime->SetDate (GetDate ());

	pDateTime->SetTextColor (m_pWndList->GetTextColor (), FALSE);
	pDateTime->SetBackgroundColor (m_pWndList->GetBkColor(), FALSE);

	pDateTime->EnableWindow(m_bEnabled);

	bDefaultFormat = FALSE;

	return pDateTime;
}
//*******************************************************************************
void CBCGPDateTimeProp::OnPosSizeChanged (CRect /*rectOld*/)
{
	ASSERT_VALID (m_pWndList);

	CRect rectEdit;
	CRect rectSpin;

	AdjustInPlaceEditRect (rectEdit, rectSpin);

	if (m_wndDateTime.GetSafeHwnd () == NULL)
	{
		m_wndDateTime.SetAutoResize (FALSE);
		m_wndDateTime.m_bPropListMode = TRUE;

		m_wndDateTime.Create (_T(""), WS_CHILD, rectEdit,
								m_pWndList, (UINT)-1);

		m_wndDateTime.SetFont (
			IsModified () && m_pWndList->m_bMarkModifiedProperties ? 
				&m_pWndList->m_fontBold : m_pWndList->GetFont ());

		SetCtrlState(m_wndDateTime);
		m_wndDateTime.SetDate (GetDate ());
	}
	else
	{
		m_wndDateTime.SetFont (
			IsModified () && m_pWndList->m_bMarkModifiedProperties ? 
				&m_pWndList->m_fontBold : m_pWndList->GetFont (), FALSE);

		m_wndDateTime.SetWindowPos (NULL, rectEdit.left, rectEdit.top,
			rectEdit.Width (), rectEdit.Height (),
			SWP_NOZORDER | SWP_NOACTIVATE);
	}

	m_wndDateTime.AdjustControl (rectEdit);
}
//******************************************************************************************
BOOL CBCGPDateTimeProp::OnUpdateValue ()
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pWndInPlace);
	ASSERT_VALID (m_pWndList);
	ASSERT (::IsWindow (m_pWndInPlace->GetSafeHwnd ()));

	CBCGPDateTimeCtrl* pDateTime = DYNAMIC_DOWNCAST (CBCGPDateTimeCtrl, m_pWndInPlace);
	ASSERT_VALID (pDateTime);

	COleDateTime dateOld = GetDate ();
	COleDateTime dateNew = pDateTime->GetDate ();

	m_varValue = _variant_t (dateNew, VT_DATE);
	m_wndDateTime.SetDate (dateNew);

	if (dateOld != dateNew)
	{
		m_pWndList->OnPropertyChanged (this);
	}

	return TRUE;
}
//********************************************************************************
void CBCGPDateTimeProp::SetDate (COleDateTime date)
{
	ASSERT_VALID (this);
	SetValue (_variant_t (date, VT_DATE));

	if (m_wndDateTime.GetSafeHwnd () != NULL)
	{
		m_wndDateTime.SetDate (date);
	}
}
//*********************************************************************************
void CBCGPDateTimeProp::SetCtrlState(CBCGPDateTimeCtrl& wnd)
{
	ASSERT (wnd.GetSafeHwnd () != NULL);

	UINT stateFlags = 0;

	if (m_nFlags & (CBCGPDateTimeCtrl::DTM_DATE))
	{
		stateFlags |= (CBCGPDateTimeCtrl::DTM_DATE | CBCGPDateTimeCtrl::DTM_DROPCALENDAR);
	}
	
	if (m_nFlags & (CBCGPDateTimeCtrl::DTM_TIME))
	{
		stateFlags |= CBCGPDateTimeCtrl::DTM_TIME;

		if (m_nFlags & (CBCGPDateTimeCtrl::DTM_TIME24H))
		{
			stateFlags |= CBCGPDateTimeCtrl::DTM_TIME24H;
		}
		else
		{
			stateFlags |= CBCGPDateTimeCtrl::DTM_TIME24HBYLOCALE;
		}

		if (m_nFlags & (CBCGPDateTimeCtrl::DTM_SECONDS))
		{
			stateFlags |= CBCGPDateTimeCtrl::DTM_SECONDS;
		}
	}

	const UINT stateMask = 
		CBCGPDateTimeCtrl::DTM_SPIN |
		CBCGPDateTimeCtrl::DTM_DROPCALENDAR | 
		CBCGPDateTimeCtrl::DTM_DATE |
		CBCGPDateTimeCtrl::DTM_TIME24H |
		CBCGPDateTimeCtrl::DTM_CHECKBOX |
		CBCGPDateTimeCtrl::DTM_TIME | 
		CBCGPDateTimeCtrl::DTM_SECONDS |
		CBCGPDateTimeCtrl::DTM_TIME24HBYLOCALE;

	wnd.SetState (stateFlags, stateMask);
}
//********************************************************************************
void CBCGPDateTimeProp::OnSetSelection (CBCGPProp* /*pOldSel*/)
{
	Redraw ();
}
//********************************************************************************
void CBCGPDateTimeProp::OnKillSelection (CBCGPProp* /*pNewSel*/)
{
	Redraw ();
}
//********************************************************************************
BOOL CBCGPDateTimeProp::PushChar (UINT nChar)
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pWndList);
	ASSERT (m_pWndList->GetCurSel () == this);
	ASSERT_VALID (m_pWndInPlace);

	if (m_bEnabled && m_bAllowEdit)
	{
		CString str ((TCHAR) nChar);
		str.MakeUpper ();

		m_pWndInPlace->SendMessage (WM_KEYDOWN, (WPARAM) str [0]);
		return TRUE;
	}

	OnEndEdit ();

	if (::GetCapture () == m_pWndList->GetSafeHwnd ())
	{
		ReleaseCapture ();
	}

	return FALSE;
}
//********************************************************************************
CString CBCGPDateTimeProp::FormatProperty ()
{
	COleDateTime date = GetDate ();

	CString strTextDate;
	if ((m_nFlags & (CBCGPDateTimeCtrl::DTM_DATE)) &&
		(m_nFlags & (CBCGPDateTimeCtrl::DTM_TIME)))
	{
		strTextDate = date.Format ();	
	}
	else if (m_nFlags & (CBCGPDateTimeCtrl::DTM_DATE))
	{
		strTextDate = date.Format (VAR_DATEVALUEONLY);
	}
	else 
	{
		strTextDate = date.Format (VAR_TIMEVALUEONLY);
	}

	return strTextDate;
}
//********************************************************************************
BOOL CBCGPDateTimeProp::SerializeValue(CString& str)
{
	str.Format(_T("%lf"), (DATE)m_varValue);
	return TRUE;
}
//********************************************************************************
BOOL CBCGPDateTimeProp::ParseValue(const CString& str)
{
	if (str.IsEmpty())
	{
		return FALSE;
	}

	double dblVal = 0.0;

#if _MSC_VER < 1400
	_stscanf (str, _T("%lf"), &dblVal);
#else
	_stscanf_s (str, _T("%lf"), &dblVal);
#endif

	SetDate ((DATE)dblVal);
	return TRUE;
}
//********************************************************************************
BOOL CBCGPDateTimeProp::IsValueChanged () const
{
	COleDateTime date = (DATE) m_varValue;
	COleDateTime date1 = (DATE) m_varValueOrig;

	if ((m_nFlags & (CBCGPDateTimeCtrl::DTM_DATE)) &&
		(m_nFlags & (CBCGPDateTimeCtrl::DTM_TIME)))
	{
		return date != date1;
	}
	else if (m_nFlags & (CBCGPDateTimeCtrl::DTM_DATE))
	{
		return	date.GetYear () != date1.GetYear () ||
				date.GetMonth () != date1.GetMonth () ||
				date.GetDay () != date1.GetDay ();
	}

	if (m_nFlags & (CBCGPDateTimeCtrl::DTM_SECONDS))
	{
		if (date.GetSecond() != date1.GetSecond())
		{
			return TRUE;
		}
	}

	return	date.GetHour () != date1.GetHour () ||
			date.GetMinute () != date1.GetMinute ();
}
//********************************************************************************
void CBCGPDateTimeProp::AdjustInPlaceEditRect (CRect& rectEdit, CRect& rectSpin)
{
	rectSpin.SetRectEmpty ();

	rectEdit = m_Rect;
	rectEdit.DeflateRect (0, 2);

	const int nMargin = 
		m_bIsModified && m_pWndList->m_bMarkModifiedProperties ? 4 : 5;

	rectEdit.left = m_pWndList->GetListRect ().left + 
					m_pWndList->GetPropertyColumnWidth () + 
					nMargin;
	
	rectEdit.bottom++;
	rectEdit.right -= 2;
}
//********************************************************************************
BOOL CBCGPDateTimeProp::OnCommand (WPARAM /*wParam*/)
{
	OnUpdateValue ();
	return FALSE;
}
//****************************************************************************************
void CBCGPDateTimeProp::ResetOriginalValue ()
{
	CBCGPProp::ResetOriginalValue ();

	if (m_wndDateTime.GetSafeHwnd () != NULL)
	{
		m_wndDateTime.SetDate ((DATE) m_varValue);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPFontProp object

IMPLEMENT_DYNAMIC(CBCGPFontProp, CBCGPProp)

CBCGPFontProp::CBCGPFontProp(const CString& strName, LOGFONT& lf, DWORD dwFontDialogFlags, LPCTSTR lpszDescr, DWORD_PTR dwData, COLORREF color) :
	CBCGPProp (strName, _variant_t((LPCTSTR)lf.lfFaceName), lpszDescr, dwData),
	m_dwFontDialogFlags (dwFontDialogFlags)
{
	m_dwFlags = PROP_HAS_BUTTON;
	m_lf = lf;
	m_lfOrig = lf;
	m_bAllowEdit = FALSE;
	m_Color = color;
	m_ColorOrig = color;
}
//*****************************************************************************************
CBCGPFontProp::CBCGPFontProp(const CString& strName, UINT nID, LOGFONT& lf, DWORD dwFontDialogFlags, LPCTSTR lpszDescr, DWORD_PTR dwData, COLORREF color) :
	CBCGPProp (strName, nID, _variant_t((LPCTSTR)lf.lfFaceName), lpszDescr, dwData),
	m_dwFontDialogFlags (dwFontDialogFlags)
{
	m_dwFlags = PROP_HAS_BUTTON;
	m_lf = lf;
	m_lfOrig = lf;
	m_bAllowEdit = FALSE;
	m_Color = color;
	m_ColorOrig = color;
}
//*****************************************************************************************
CBCGPFontProp::~CBCGPFontProp()
{
}
//****************************************************************************************
BOOL CBCGPFontProp::CompareLFandColor(const LOGFONT& lf1, const LOGFONT& lf2, COLORREF c1, COLORREF c2) const
{
	return (lf1.lfItalic != lf2.lfItalic ||
			lf1.lfWeight != lf2.lfWeight ||
			lf1.lfHeight != lf2.lfHeight ||
			lf1.lfStrikeOut != lf2.lfStrikeOut ||
			lf1.lfUnderline != lf2.lfUnderline ||
			lf1.lfCharSet != lf2.lfCharSet ||
			_tcscmp(lf1.lfFaceName, lf2.lfFaceName) ||
			(c1 != c2 && c2 != 0));
}
//****************************************************************************************
void CBCGPFontProp::OnClickButton (CPoint /*point*/)
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pWndList);
	ASSERT_VALID (m_pWndInPlace);
	ASSERT (::IsWindow (m_pWndInPlace->GetSafeHwnd ()));

	LOGFONT lfPrev = m_lf;
	COLORREF nColorPrev = m_Color;

	m_bButtonIsDown = TRUE;
	m_bButtonIsHighlighted = FALSE;
	Redraw ();

#pragma warning (disable : 4244)

	CFontDialog dlg (&m_lf, m_dwFontDialogFlags, NULL, m_pWndList);

#pragma warning (default : 4244)

	if (m_Color != (COLORREF)-1)
	{
		dlg.m_cf.rgbColors = m_Color;
	}

	if (dlg.DoModal () == IDOK)
	{
		dlg.GetCurrentFont (&m_lf);
		m_Color = dlg.GetColor ();

		if (CompareLFandColor(lfPrev, m_lf, nColorPrev, m_Color))
		{
			m_pWndList->OnPropertyChanged (this);
		}

		if (m_pWndInPlace != NULL)
		{
			m_pWndInPlace->SetWindowText (FormatProperty ());
		}
		else
		{
			m_varValue = (LPCTSTR) FormatProperty ();
		}
	}
	else
	{
		m_lf = lfPrev;
		m_Color = nColorPrev;
	}

	if (m_pWndInPlace != NULL)
	{
		m_pWndInPlace->SetFocus ();
	}
	else
	{
		m_pWndList->SetFocus ();
	}

	m_bButtonIsDown = FALSE;
	Redraw ();
}
//*****************************************************************************
CString CBCGPFontProp::FormatProperty ()
{
	CString str;
	CWindowDC dc (m_pWndList);

	int nLogY = dc.GetDeviceCaps (LOGPIXELSY);
	if (nLogY != 0)
	{
		str.Format( _T("%s(%i)"), m_lf.lfFaceName, 
			MulDiv (72, -m_lf.lfHeight, nLogY));
	}
	else
	{
		str = m_lf.lfFaceName;
	}

	return str;
}
//****************************************************************************************
void CBCGPFontProp::SetLogFont(const LOGFONT& lf)
{
	m_lf = lf;
	Redraw();
}
//****************************************************************************************
void CBCGPFontProp::SetColor(COLORREF color)
{
	m_Color = color;
	Redraw();
}
//****************************************************************************************
void CBCGPFontProp::ResetOriginalValue ()
{
	CBCGPProp::ResetOriginalValue ();
	
	m_lf = m_lfOrig;
	m_Color = m_ColorOrig;
}
//****************************************************************************************
void CBCGPFontProp::CommitModifiedValue ()
{
	CBCGPProp::CommitModifiedValue ();

	m_lfOrig = m_lf;
	m_ColorOrig = m_Color;
}
//**********************************************************************************
BOOL CBCGPFontProp::SerializeValue(CString& str)
{
	str = CBCGPTagManager::FormatFont(m_lf);
	return !str.IsEmpty();
}
//**********************************************************************************
BOOL CBCGPFontProp::ParseValue(const CString& str)
{
	return CBCGPTagManager::ParseFont(str, m_lf);
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPPropertiesToolBar

void CBCGPPropertiesToolBar::CheckForButtonImages(CBCGPToolbarButton* pButton, CBCGPToolBarImages** pNewImages)
{
	ASSERT_VALID(pButton);

	if (ButtonToIndex(pButton) >= m_nDefaultButtons)
	{
		*pNewImages = &m_CustomImages;
	}
}
//**********************************************************************************
void CBCGPPropertiesToolBar::OnLargeIconsModeChanged()
{
	CBCGPPropList* pPropList = DYNAMIC_DOWNCAST(CBCGPPropList, GetParent());
	if (pPropList != NULL)
	{
		pPropList->AdjustLayout();
	}
}
//**********************************************************************************
BOOL CBCGPPropertiesToolBar::PreTranslateMessage(MSG* pMsg)
{
	if (CBCGPPopupMenu::GetActiveMenu () != NULL)
	{
		return CWnd::PreTranslateMessage(pMsg);
	}

	return CBCGPToolBar::PreTranslateMessage(pMsg);
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPPropList

IMPLEMENT_DYNAMIC(CBCGPPropList, CWnd)

/////////////////////////////////////////////////////////////////////////////
// CBCGPPropList notification messages:

UINT BCGM_PROPERTY_CHANGED = ::RegisterWindowMessage (_T("BCGM_PROPERTYCHANGED"));
UINT BCGM_PROPERTY_COMMAND_CLICKED = ::RegisterWindowMessage (_T("BCGM_PROPERTY_COMMAND_CLICKED"));
UINT BCGM_PROPERTY_MENU_ITEM_SELECTED = ::RegisterWindowMessage (_T("BCGM_PROPERTY_MENU_ITEM_SELECTED"));
UINT BCGM_PROPERTY_GET_MENU_ITEM_STATE = ::RegisterWindowMessage (_T("BCGM_PROPERTY_GET_MENU_ITEM_STATE"));
UINT BCGM_PROPERTYLIST_SORTING_MODE_CHANGED = ::RegisterWindowMessage (_T("BCGM_PROPERTYLIST_SORTING_MODE_CHANGED"));
UINT BCGM_PROPERTYLIST_LAYOUT_CHANGED = ::RegisterWindowMessage(_T("BCGM_PROPERTYLIST_LAYOUT_CHANGED"));

CBCGPPropList::CBCGPPropList() :
	m_wndToolBar(m_ToolBarCustomImages)
{
	m_bToolBar = FALSE;
	m_nToolbarDefaultButtons = 0;
	m_bToolbarStandardButtons = FALSE;
	m_bToolbarCustomButtons = FALSE;
	m_bNeedToUpdate = FALSE;
	m_nCustomToolbarID = 0;
	m_nCustomToolbarBitmapID = 0;
	m_bFilterBox = FALSE;
	m_hFont = NULL;
	m_nEditLeftMargin = 0;
	m_nBoldEditLeftMargin = 0;
	m_bHeaderCtrl = TRUE;
	m_bDescriptionArea = FALSE;
	m_bContextMenu = FALSE;
	m_nDescrHeight = -1;
	m_nCommandsHeight = -1;
	m_nCommandRows = 1;
	m_bAreCommandsVisible = TRUE;
	m_nDescrRows = 3;
	m_bAlphabeticMode = FALSE;
	m_bVSDotNetLook = FALSE;
	m_rectList.SetRectEmpty ();
	m_nLeftColumnWidth = 0;
	m_rectTrackHeader.SetRectEmpty ();
	m_rectTrackDescr.SetRectEmpty ();
	m_rectTrackCommands.SetRectEmpty ();
	m_nRowHeight = 0;
	m_nToolbarHeight = 0;
	m_nHeaderHeight = 0;
	m_nVertScrollOffset = 0;
	m_nVertScrollTotal = 0;
	m_nVertScrollPage = 0;
	m_pSel = NULL;
	m_pTracked = NULL;
	m_bFocused = FALSE;
	m_bOutOfFilter = FALSE;
	m_nTooltipsCount = 0;
	m_bAlwaysShowUserTT = TRUE;
	m_bTracking = FALSE;
	m_bTrackingDescr = FALSE;
	m_bTrackingCommands = FALSE;
	m_bTrackButtons = FALSE;

	m_strTrue = _T("True");
	m_strFalse = _T("False");

	m_cListDelimeter = _T(',');

	m_bControlBarColors = FALSE;
	m_bGroupNameFullWidth = TRUE;
	m_bShowDragContext = TRUE;

	m_clrBackground = (COLORREF)-1;
	m_clrText = (COLORREF)-1;
	m_clrTextDefault = globalData.clrWindowText;
	m_clrFillDefault = globalData.clrWindow;
	m_clrGroupBackground = (COLORREF)-1;
	m_clrGroupText = (COLORREF)-1;
	m_clrDescriptionBackground = (COLORREF)-1;
	m_clrDescriptionText = (COLORREF)-1;
	m_clrLine = (COLORREF)-1;
	m_clrCommandText = (COLORREF)-1;

	m_nNameAlign = DT_LEFT;

	m_bMarkModifiedProperties = FALSE;
	SetScrollBarsStyle (CBCGPScrollBar::BCGP_SBSTYLE_DEFAULT);

	m_pGM = NULL;
	m_nItemMenuFlags = 0;

	m_pAccProp = NULL;

#if _MSC_VER >= 1300
	EnableActiveAccessibility();
#endif

}

CBCGPPropList::~CBCGPPropList()
{
	if (m_pGM != NULL)
	{
		delete m_pGM;
	}
}

BEGIN_MESSAGE_MAP(CBCGPPropList, CWnd)
	//{{AFX_MSG_MAP(CBCGPPropList)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_CANCELMODE()
	ON_WM_KILLFOCUS()
	ON_WM_GETDLGCODE()
	ON_WM_VSCROLL()
	ON_WM_MOUSEWHEEL()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_SETCURSOR()
	ON_WM_KEYDOWN()
	ON_WM_CHAR()
	ON_WM_SETFOCUS()
	ON_WM_CTLCOLOR()
	ON_WM_DESTROY()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_NCCALCSIZE()
	ON_WM_NCPAINT()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_CONTEXTMENU()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_SETFONT, OnSetFont)
	ON_MESSAGE(WM_GETFONT, OnGetFont)
	ON_NOTIFY(HDN_ITEMCHANGED, ID_HEADER, OnHeaderItemChanged)
	ON_NOTIFY(HDN_TRACK, ID_HEADER, OnHeaderTrack)
	ON_NOTIFY(HDN_ENDTRACK, ID_HEADER, OnHeaderEndTrack)
	ON_NOTIFY(UDN_DELTAPOS, BCGPROPLIST_ID_INPLACE, OnSpinDeltaPos)
	ON_MESSAGE(UM_UPDATESPIN, OnUpdateSpin)
	ON_WM_STYLECHANGED()
	ON_CBN_SELENDOK(BCGPROPLIST_ID_INPLACE, OnSelectCombo)
	ON_CBN_CLOSEUP(BCGPROPLIST_ID_INPLACE, OnCloseCombo)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXT, 0, 0xFFFF, OnNeedTipText)
	ON_EN_KILLFOCUS(BCGPROPLIST_ID_INPLACE, OnEditKillFocus)
	ON_CBN_KILLFOCUS(BCGPROPLIST_ID_INPLACE, OnComboKillFocus)
	ON_MESSAGE (WM_GETOBJECT, OnGetObject)
	ON_MESSAGE(WM_PRINTCLIENT, OnPrintClient)
	ON_MESSAGE(WM_PRINT, OnPrint)
	ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
	ON_COMMAND(ID_BCGBARRES_PROPLIST_CATEGORIZED, OnToolbarCategorized)
	ON_UPDATE_COMMAND_UI(ID_BCGBARRES_PROPLIST_CATEGORIZED, OnUpdateToolbarCategorized)
	ON_COMMAND(ID_BCGBARRES_PROPLIST_ALPHABETICAL, OnToolbarAlphabetical)
	ON_UPDATE_COMMAND_UI(ID_BCGBARRES_PROPLIST_ALPHABETICAL, OnUpdateToolbarAlphabetical)
	ON_EN_CHANGE(ID_FILTER, OnFilter)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGPPropList message handlers

void CBCGPPropList::PreSubclassWindow() 
{
	CWnd::PreSubclassWindow();

	_AFX_THREAD_STATE* pThreadState = AfxGetThreadState ();
	if (pThreadState->m_pWndInit == NULL)
	{
		Init ();
	}
}
//******************************************************************************************
int CBCGPPropList::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	Init ();
	return 0;
}
//******************************************************************************************
void CBCGPPropList::Init ()
{
	CRect rectDummy;
	rectDummy.SetRectEmpty ();

	if (globalData.m_hcurStretch == NULL)
	{
		globalData.m_hcurStretch = AfxGetApp ()->LoadCursor (AFX_IDC_HSPLITBAR);
	}

	if (globalData.m_hcurStretchVert == NULL)
	{
		globalData.m_hcurStretchVert = AfxGetApp ()->LoadCursor (AFX_IDC_VSPLITBAR);
	}

	InitToolBar ();
	InitHeader ();

	HDITEM hdItem;
	hdItem.mask = HDI_TEXT | HDI_FORMAT;
	hdItem.fmt = (m_nNameAlign == DT_LEFT) ? HDF_LEFT : (m_nNameAlign == DT_CENTER) ? HDF_CENTER : HDF_RIGHT;
	hdItem.pszText = _T("Property");
	hdItem.cchTextMax = 100;

	GetHeaderCtrl ().InsertItem (0, &hdItem);

	hdItem.fmt = HDF_LEFT;
	hdItem.pszText = _T("Value");
	hdItem.cchTextMax = 100;

	GetHeaderCtrl ().InsertItem (1, &hdItem);

	m_wndScrollVert.Create (WS_CHILD | WS_VISIBLE | SBS_VERT, rectDummy, this, ID_SCROLL_VERT);

	m_ToolTip.Create (this, TTS_ALWAYSTIP);
	m_ToolTip.Activate (TRUE);

	if (globalData.m_nMaxToolTipWidth != -1)
	{
		m_ToolTip.SetMaxTipWidth(globalData.m_nMaxToolTipWidth);
	}

	m_ToolTip.SetWindowPos (&wndTop, -1, -1, -1, -1,
							SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOSIZE);

	m_IPToolTip.Create (this);

	CWnd* pWndParent = GetParent ();
	m_bControlBarColors = pWndParent == NULL ||
		!pWndParent->IsKindOf (RUNTIME_CLASS (CDialog));

	AdjustLayout ();
	CreateBoldFont ();
	CalcEditMargin ();

	ModifyStyle(0, WS_CLIPCHILDREN);

	SetControlVisualMode (pWndParent);
	m_wndScrollVert.SetVisualStyle(DrawControlBarColors () ? CBCGPScrollBar::BCGP_SBSTYLE_VISUAL_MANAGER : CBCGPScrollBar::BCGP_SBSTYLE_DEFAULT);

	m_wndHeader.m_bVisualManagerStyle = DrawControlBarColors ();

	m_wndToolBar.m_bIsDlgControl = !DrawControlBarColors ();
	m_wndToolBar.m_bVisualManagerStyle = DrawControlBarColors ();

	m_wndFilter.m_bVisualManagerStyle = DrawControlBarColors ();
}
//******************************************************************************************
void CBCGPPropList::InitToolBar()
{
	m_wndToolBar.Create(this);
	m_wndToolBar.SetControlVisualMode (this);

	const UINT uiToolbarHotID = globalData.Is32BitIcons () ? IDR_BCGBARRES_PROPLIST32 : 0;

	{
		CBCGPLocalResource locaRes;
		m_wndToolBar.LoadToolBar (IDR_BCGBARRES_PROPLIST, 0, 0, TRUE /* Locked bar */, 0, 0, uiToolbarHotID);
	}

	m_wndToolBar.m_nDefaultButtons = m_wndToolBar.GetCount();
	m_nToolbarDefaultButtons = m_wndToolBar.GetCount();

	m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);
		
	m_wndToolBar.SetBarStyle (
		m_wndToolBar.GetBarStyle () & 
			~(CBRS_GRIPPER | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));

	m_wndToolBar.SetOwner (this);

	for (int i = 0; i < m_wndToolBar.GetCount(); i++)
	{
		m_wndToolBar.GetButton(i)->SetVisible(m_bToolbarStandardButtons);
	}

	// All commands will be routed via this dialog, not via the parent frame:
	m_wndToolBar.SetRouteCommandsViaFrame (FALSE);
	m_wndToolBar.OnUpdateCmdUI ((CFrameWnd*) this, TRUE);

	SetupToolbarCustomButtons();

	m_wndFilter.Create(WS_CHILD | WS_VISIBLE, CRect(0, 0, 0, 0), this, ID_FILTER);
	m_wndFilter.EnableSearchMode(TRUE, m_strFilterPrompt);
	m_wndFilter.ModifyStyleEx(0, WS_EX_CLIENTEDGE);
}
//******************************************************************************************
void CBCGPPropList::SetupToolbarCustomButtons()
{
	m_bToolbarCustomButtons = FALSE;
	m_ToolBarCustomImages.Clear();

	if (m_nCustomToolbarID == 0)
	{
		m_wndToolBar.AdjustLayout();
		return;
	}

	CBCGPToolBar toolbar;
	if (!toolbar.LoadToolBar (m_nCustomToolbarID, 0, 0, TRUE /* Locked bar */))
	{
		ASSERT(FALSE);
		return;
	}

	m_ToolBarCustomImages.SetImageSize(CSize(16, 16));
	m_ToolBarCustomImages.Load(globalData.Is32BitIcons () && m_nCustomToolbarBitmapID != 0 ? m_nCustomToolbarBitmapID : m_nCustomToolbarID);

	for (int i = 0; i < toolbar.GetCount(); i++)
	{
		m_bToolbarCustomButtons = TRUE;
		m_bNeedToUpdate = TRUE;
		m_wndToolBar.InsertButton(*toolbar.GetButton(i));
	}

	m_wndToolBar.AdjustLayout();
}
//******************************************************************************************
void CBCGPPropList::InitHeader ()
{
	CRect rectDummy;
	rectDummy.SetRectEmpty ();

	m_wndHeader.Create (WS_CHILD | WS_VISIBLE | HDS_HORZ, rectDummy, this, ID_HEADER);
}
//*****************************************************************************************
void CBCGPPropList::AdjustLayout ()
{
	if (GetSafeHwnd () == NULL)
	{
		return;
	}

	CClientDC dc (this);
	HFONT hfontOld = SetCurrFont (&dc);

	TEXTMETRIC tm;
	dc.GetTextMetrics (&tm);
	m_nRowHeight = tm.tmHeight + 4;

	CRect rectClient;
	GetClientRect (rectClient);

	m_nToolbarHeight = 0;

	if (m_bToolBar || m_bFilterBox)
	{
		int nMarginX = 4;
		int nMarginY = 2;

		if (globalData.GetRibbonImageScale () != 1.)
		{
			nMarginX = (int) (globalData.GetRibbonImageScale () * nMarginX);
			nMarginY = (int) (globalData.GetRibbonImageScale () * nMarginY);
		}

		int nToolbarWidth = m_bToolBar ? (m_bFilterBox ? m_wndToolBar.CalcSize(FALSE).cx + 2 * nMarginX : rectClient.Width()) : 0;

		m_nToolbarHeight = m_bToolBar ? m_wndToolBar.GetButtonSize().cy + 2 * nMarginY : 0;

		if (m_bToolBar)
		{
			m_wndToolBar.SetWindowPos (NULL, rectClient.left + nMarginX, rectClient.top + nMarginY, nToolbarWidth - nMarginX, m_nToolbarHeight, SWP_NOZORDER | SWP_NOACTIVATE);
			m_wndToolBar.ShowWindow (SW_SHOWNOACTIVATE);
		}
		else
		{
			m_wndToolBar.ShowWindow(SW_HIDE);
		}

		if (m_bFilterBox)
		{
			m_wndFilter.SendMessage (WM_SETFONT, (WPARAM) (m_hFont != NULL ? m_hFont : ::GetStockObject (DEFAULT_GUI_FONT)));

			if (m_nToolbarHeight == 0)
			{
				m_nToolbarHeight = 3 * globalData.GetTextHeight() / 2 + 2 * nMarginY;
			}

			int nFilterBoxHeight = m_nToolbarHeight - 2 * nMarginY;
			if (CBCGPToolBar::IsLargeIcons())
			{
				nFilterBoxHeight = 3 * globalData.GetTextHeight() / 2;
			}

			m_wndFilter.SetWindowPos (NULL, rectClient.left + nToolbarWidth + nMarginX, rectClient.top + nMarginY + 1, 
				rectClient.Width() - nToolbarWidth - 2 * nMarginX, nFilterBoxHeight, SWP_NOZORDER | SWP_NOACTIVATE);

			m_wndFilter.ShowWindow (SW_SHOWNOACTIVATE);
		}
		else
		{
			m_wndFilter.ShowWindow(SW_HIDE);
		}

		m_nToolbarHeight += 2 * nMarginY;

		rectClient.top += m_nToolbarHeight;
	}
	else
	{
		m_wndToolBar.ShowWindow(SW_HIDE);
		m_wndFilter.ShowWindow(SW_HIDE);
	}

	m_nHeaderHeight = 0;

	if (m_bHeaderCtrl)
	{
		m_nHeaderHeight = m_nRowHeight + 4;

		GetHeaderCtrl ().SendMessage (WM_SETFONT,
			(WPARAM) (m_hFont != NULL ? m_hFont : ::GetStockObject (DEFAULT_GUI_FONT)));

		GetHeaderCtrl ().SetWindowPos (NULL, rectClient.left, rectClient.top, 
			rectClient.Width (), m_nHeaderHeight,
			SWP_NOZORDER | SWP_NOACTIVATE);

		HDITEM hdItem;
		hdItem.mask = HDI_WIDTH ;
		hdItem.cxy = m_nLeftColumnWidth + 2;

		GetHeaderCtrl ().SetItem (0, &hdItem);

		hdItem.cxy = rectClient.Width () + 10;
		GetHeaderCtrl ().SetItem (1, &hdItem);

		GetHeaderCtrl ().ShowWindow (SW_SHOWNOACTIVATE);
	}
	else
	{
		GetHeaderCtrl ().ShowWindow (SW_HIDE);
	}

	m_rectList = rectClient;
	m_rectList.top += m_nHeaderHeight;

	if (HasCommands () && m_bAreCommandsVisible && m_nCommandsHeight != -1 && rectClient.Height() > 0)
	{
		m_nCommandsHeight = max (m_nCommandsHeight, m_nRowHeight);
		m_nCommandsHeight = min (m_nCommandsHeight, rectClient.Height () - m_nRowHeight);
		m_rectList.bottom -= m_nCommandsHeight;
	}

	if (m_bDescriptionArea && m_nDescrHeight != -1 && rectClient.Height() > 0)
	{
		m_nDescrHeight = max (m_nDescrHeight, m_nRowHeight);
		m_nDescrHeight = min (m_nDescrHeight, rectClient.Height () - m_nRowHeight);
		m_rectList.bottom -= m_nDescrHeight;
	}

	if (HasCommands () && m_bAreCommandsVisible && m_nCommandsHeight != -1 && rectClient.Height() > 0)
	{
		CRect rectCommands = m_rectList;

		rectCommands.top = m_rectList.bottom; 
		rectCommands.bottom = rectCommands.top + m_nCommandsHeight;
		rectCommands.DeflateRect (2 * TEXT_MARGIN, TEXT_MARGIN);

		int i = 0;
		int x = rectCommands.left;
		int y = rectCommands.top + m_nRowHeight / 4;

		for (POSITION pos = m_lstCommands.GetHeadPosition (); pos != NULL; i++)
		{
			CString strCommand = m_lstCommands.GetNext (pos);
			CSize sizeLabel = dc.GetTextExtent (strCommand);

			CRect rectCommand (0, 0, 0, 0);

			if (x > rectCommands.left && x + sizeLabel.cx > rectCommands.right)
			{
				x = rectCommands.left;
				y += m_nRowHeight;
			}

			int x2 = min (rectCommands.right, x + sizeLabel.cx + 1);
			int y2 = min (rectCommands.bottom, y + sizeLabel.cy + 1);

			if (y < rectCommands.bottom)
			{
				rectCommand = CRect (x, y, x2, y2);
				x += sizeLabel.cx + TEXT_MARGIN;
			}

			m_arCommandRects [i] = rectCommand;
		}
	}
	else
	{
		for (int i = 0; i < m_arCommandRects.GetSize (); i++)
		{
			m_arCommandRects[i].SetRectEmpty();
		}
	}

	::SelectObject (dc.GetSafeHdc (), hfontOld);

	int cxScroll = ::GetSystemMetrics (SM_CXHSCROLL);
	SetScrollSizes ();

	if (m_nVertScrollTotal > 0)
	{
		m_rectList.right -= cxScroll;
		m_wndScrollVert.SetWindowPos (NULL, m_rectList.right, m_rectList.top,
			cxScroll, m_rectList.Height (), SWP_NOZORDER | SWP_NOACTIVATE);
	}
	else
	{
		m_wndScrollVert.SetWindowPos (NULL, 0, 0,
			0, 0, SWP_NOZORDER | SWP_NOACTIVATE);
	}

	ReposProperties ();

	if (m_pSel != NULL && m_pSel->HasButton ())
	{
		ASSERT_VALID (m_pSel);
		m_pSel->AdjustButtonRect ();
	}

	RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ALLCHILDREN);

	if (m_pSel != NULL && m_pSel->m_pWndInPlace->GetSafeHwnd () != NULL)
	{
		CRect rectEdit;
		CRect rectSpin;

		m_pSel->AdjustInPlaceEditRect (rectEdit, rectSpin);

		m_pSel->m_pWndInPlace->SetWindowPos (NULL, 
			rectEdit.left, rectEdit.top, 
			rectEdit.Width (), rectEdit.Height (),
			SWP_NOZORDER | SWP_NOACTIVATE);
	}
}
//******************************************************************************************
void CBCGPPropList::ReposProperties ()
{
	ASSERT_VALID (this);
	m_lstTerminalProps.RemoveAll ();

	if (m_ToolTip.GetSafeHwnd () != NULL)
	{
		while (m_nTooltipsCount > 0)
		{
			m_ToolTip.DelTool (this, m_nTooltipsCount);
			m_nTooltipsCount--;
		}
	}

	int y = m_rectList.top - m_nRowHeight * m_nVertScrollOffset - 1;

	m_bOutOfFilter = !m_strFilter.IsEmpty();

	if (!m_bAlphabeticMode)
	{
		for (POSITION pos = m_lstProps.GetHeadPosition (); pos != NULL;)
		{
			CBCGPProp* pProp = m_lstProps.GetNext (pos);
			ASSERT_VALID (pProp);

			pProp->Repos (y);

			if (m_bOutOfFilter && !pProp->GetRect().IsRectEmpty())
			{
				m_bOutOfFilter = FALSE;
			}
		}

		return;
	}

	POSITION pos = NULL;

	// Get sorted list of terminal properties:
	for (pos = m_lstProps.GetHeadPosition (); pos != NULL;)
	{
		CBCGPProp* pProp = m_lstProps.GetNext (pos);
		ASSERT_VALID (pProp);

		pProp->AddTerminalProp (m_lstTerminalProps);
	}

	for (pos = m_lstTerminalProps.GetHeadPosition (); pos != NULL;)
	{
		CBCGPProp* pProp = m_lstTerminalProps.GetNext (pos);
		ASSERT_VALID (pProp);

		pProp->Repos (y);

		if (m_bOutOfFilter && !pProp->GetRect().IsRectEmpty())
		{
			m_bOutOfFilter = FALSE;
		}
	}
}
//******************************************************************************************
void CBCGPPropList::OnSize(UINT nType, int cx, int cy) 
{
	CWnd::OnSize(nType, cx, cy);

	EndEditItem ();

	m_nLeftColumnWidth = cx / 2;
	AdjustLayout ();
}
//******************************************************************************************
LRESULT CBCGPPropList::OnSetFont (WPARAM wParam, LPARAM /*lParam*/)
{
	m_hFont = (HFONT) wParam;

	CreateBoldFont ();
	CalcEditMargin ();
	AdjustLayout ();
	return 0;
}
//******************************************************************************************
LRESULT CBCGPPropList::OnGetFont (WPARAM, LPARAM)
{
	return (LRESULT) (m_hFont != NULL ? m_hFont : ::GetStockObject (DEFAULT_GUI_FONT));
}
//******************************************************************************************
void CBCGPPropList::CreateBoldFont ()
{
	if (m_fontBold.GetSafeHandle () != NULL)
	{
		m_fontBold.DeleteObject ();
	}

	CFont* pFont = CFont::FromHandle (
		m_hFont != NULL ? m_hFont : (HFONT) ::GetStockObject (DEFAULT_GUI_FONT));
	ASSERT_VALID (pFont);

	LOGFONT lf;
	memset (&lf, 0, sizeof (LOGFONT));

	pFont->GetLogFont (&lf);

	lf.lfWeight = FW_BOLD;
	m_fontBold.CreateFontIndirect (&lf);
}
//******************************************************************************************
void CBCGPPropList::CalcEditMargin ()
{
	CEdit editDummy;
	editDummy.Create (WS_CHILD, CRect (0, 0, 100, 20), this, (UINT)-1);

	editDummy.SetFont (GetFont ());
	m_nEditLeftMargin = LOWORD (editDummy.GetMargins ());

	editDummy.SetFont (&m_fontBold);
	m_nBoldEditLeftMargin = LOWORD (editDummy.GetMargins ());

	editDummy.DestroyWindow ();
}
//******************************************************************************************
HFONT CBCGPPropList::SetCurrFont (CDC* pDC)
{
	ASSERT_VALID (pDC);
	
	return (HFONT) ::SelectObject (pDC->GetSafeHdc (), 
		m_hFont != NULL ? m_hFont : ::GetStockObject (DEFAULT_GUI_FONT));
}
//******************************************************************************************
void CBCGPPropList::OnPaint() 
{
	if (m_bNeedToUpdate)
	{
		m_wndToolBar.OnUpdateCmdUI ((CFrameWnd*) this, TRUE);
		m_bNeedToUpdate = FALSE;
	}

	CPaintDC dcPaint (this); // device context for painting
	OnDraw(&dcPaint);
}
//******************************************************************************************
void CBCGPPropList::OnDraw(CDC* pDCSrc) 
{
	ASSERT_VALID (pDCSrc);

	CBCGPMemDC memDC (*pDCSrc, this);
	CDC* pDC = &memDC.GetDC ();

	m_clrGray = visualManager->GetPropListGroupColor (this);

	CRect rectClient;
	GetClientRect (rectClient);

	OnFillBackground (pDC, rectClient);

	int yToolBarBottom = -1;

	if (m_nToolbarHeight > 0)
	{
		CRect rectToolBar = rectClient;
		yToolBarBottom = rectToolBar.bottom = rectToolBar.top + m_nToolbarHeight - 1;

		visualManager->OnFillPropListToolbarArea(pDC, this, rectToolBar);
	}

	HFONT hfontOld = SetCurrFont (pDC);
	pDC->SetTextColor (GetTextColor());
	pDC->SetBkMode (TRANSPARENT);

	OnDrawList (pDC);

	int yBottom = m_rectList.bottom;
	
	if (HasCommands () && m_bAreCommandsVisible)
	{
		CRect rectCommands = rectClient;
		rectCommands.top = yBottom;
		rectCommands.bottom = rectCommands.top + m_nCommandsHeight;
		yBottom = rectCommands.bottom;

		if (rectCommands.top > yToolBarBottom)
		{
			OnDrawCommands (pDC, rectCommands);
		}
	}

	if (m_bDescriptionArea)
	{
		CRect rectDescr = rectClient;
		rectDescr.top = yBottom;

		if (rectDescr.top > yToolBarBottom)
		{
			OnDrawDescription (pDC, rectDescr);
		}
	}

	::SelectObject (pDC->GetSafeHdc (), hfontOld);
}
//******************************************************************************************
void CBCGPPropList::OnFillBackground (CDC* pDC, CRect rectClient)
{
	ASSERT_VALID (pDC);

	if (m_clrBackground == (COLORREF)-1)
	{
		COLORREF clrFillDefaultOld = m_clrFillDefault;

		m_clrTextDefault = visualManager->OnFillPropList(pDC, this, rectClient, m_clrFillDefault);
		
		if (clrFillDefaultOld != m_clrFillDefault)
		{
			m_brBackground.DeleteObject();
			m_brBackground.CreateSolidBrush (m_clrFillDefault);
		}
	}
	else
	{
		pDC->FillRect (rectClient, &m_brBackground);
	}
}
//******************************************************************************************
void CBCGPPropList::OnDrawBorder (CDC* /*pDC*/)
{
	ASSERT (FALSE);	// This method is obsolete
}
//******************************************************************************************
void CBCGPPropList::OnDrawList (CDC* pDC)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);

	if (m_bOutOfFilter)
	{
		if (m_strOutOfFilter.IsEmpty())
		{
			CBCGPLocalResource locaRes;
			m_strOutOfFilter.LoadString(IDS_BCGBARRES_PROPLIST_OUT_OF_FILTER);
		}

		CRect rectText = m_rectList;
		
		rectText.top += min(m_nRowHeight * 3, rectText.Height() / 3);
		rectText.DeflateRect(10, 0);

		pDC->DrawText(m_strOutOfFilter, rectText, DT_CENTER | DT_WORDBREAK);
		return;
	}

	COLORREF clrShadow = DrawControlBarColors () ?
		globalData.clrBarShadow : globalData.clrBtnShadow;

	CPen penLine (PS_SOLID, 1, m_clrLine != (COLORREF)-1 ?
		m_clrLine : ((m_bVSDotNetLook && !globalData.IsHighContastMode()) ? m_clrGray : clrShadow));
	CPen* pOldPen = pDC->SelectObject (&penLine);

	int nXCenter = m_rectList.left + m_nLeftColumnWidth;

	pDC->MoveTo (nXCenter, m_rectList.top - 1);
	pDC->LineTo (nXCenter, m_rectList.bottom);

	const CList<CBCGPProp*, CBCGPProp*>& lst = m_bAlphabeticMode ? 
		m_lstTerminalProps : m_lstProps;

	for (POSITION pos = lst.GetHeadPosition (); pos != NULL;)
	{
		CBCGPProp* pProp = lst.GetNext (pos);
		ASSERT_VALID (pProp);

		if (!OnDrawProperty (pDC, pProp))
		{
			break;
		}
	}

	pDC->SelectObject (pOldPen);
}
//****************************************************************************************
void CBCGPPropList::OnDrawDescription (CDC* pDC, CRect rect)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);

	COLORREF clrBorder = DrawControlBarColors () ? globalData.clrBarShadow : globalData.clrBtnShadow;

	if (m_clrDescriptionBackground != (COLORREF)-1)
	{
		CBrush br (m_clrDescriptionBackground);
		pDC->FillRect (rect, &br);
	}
	else
	{
		clrBorder = visualManager->OnFillPropListDescriptionArea(pDC, this, rect);
	}

	rect.top += TEXT_MARGIN;

	pDC->Draw3dRect (rect, clrBorder, clrBorder);

	if (m_pSel == NULL || !m_pSel->IsVisible())
	{
		return;
	}

	rect.DeflateRect (TEXT_MARGIN, TEXT_MARGIN);

	ASSERT_VALID (m_pSel);

	COLORREF clrTextOld = (COLORREF)-1;
	COLORREF clrText = (COLORREF)-1;

	if (m_clrDescriptionText != (COLORREF)-1)
	{
		clrTextOld = pDC->SetTextColor (m_clrDescriptionText);
	}
	else if ((clrText = visualManager->GetPropListDesciptionTextColor (this)) != (COLORREF)-1)
	{
		clrTextOld = pDC->SetTextColor (clrText);
	}

	m_pSel->OnDrawDescription (pDC, rect);

	if (clrTextOld == (COLORREF)-1)
	{
		pDC->SetTextColor (clrTextOld);
	}
}
//****************************************************************************************
void CBCGPPropList::OnDrawCommands (CDC* pDC, CRect rect)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);

	COLORREF clrBorder = DrawControlBarColors () ? globalData.clrBarShadow : globalData.clrBtnShadow;

	if (m_clrDescriptionBackground != (COLORREF)-1)
	{
		CBrush br (m_clrDescriptionBackground);
		pDC->FillRect (rect, &br);
	}
	else
	{
		clrBorder = visualManager->OnFillPropListCommandsArea(pDC, this, rect);
	}

	rect.top += TEXT_MARGIN;

	// Set font:
	CFont* pOldFont = pDC->SelectObject (&globalData.fontDefaultGUIUnderline);
	ASSERT (pOldFont != NULL);

	// Set text parameters:
	COLORREF clrTextOld = pDC->SetTextColor (m_clrCommandText == (COLORREF)-1 ? 
		visualManager->GetPropListCommandTextColor (this) : m_clrCommandText);

	int i = 0;

	for (POSITION pos = m_lstCommands.GetHeadPosition (); pos != NULL; i++)
	{
		CString strCommand = m_lstCommands.GetNext (pos);
		CRect rectCommand = m_arCommandRects [i];

		if (!rectCommand.IsRectEmpty ())
		{
			pDC->DrawText (strCommand, rectCommand, DT_LEFT | DT_SINGLELINE | DT_NOCLIP);
		}
	}

	pDC->Draw3dRect (rect, clrBorder, clrBorder);

	pDC->SelectObject (pOldFont);
	pDC->SetTextColor (clrTextOld);
}
//******************************************************************************************
BOOL CBCGPPropList::OnDrawProperty (CDC* pDC, CBCGPProp* pProp) const
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);
	ASSERT_VALID (pProp);

	pProp->m_bDrawMenuButton = FALSE;

	if (!pProp->m_Rect.IsRectEmpty ())
	{
		if (pProp->m_Rect.top >= m_rectList.bottom)
		{
			return FALSE;
		}

		if (pProp->m_Rect.bottom >= m_rectList.top)
		{
			const int nXCenter = m_rectList.left + m_nLeftColumnWidth;
			COLORREF clrTextOld = (COLORREF)-1;

			if (m_bVSDotNetLook)
			{
				CRect rectLeft = pProp->m_Rect;

				if (!pProp->IsGroup ())
				{
					rectLeft.right = min (nXCenter, rectLeft.left);
				}

				if (pProp->m_bIsValueList)
				{
					rectLeft.right = rectLeft.left + rectLeft.Height ();
				}

				rectLeft.left = m_rectList.left;
				rectLeft.bottom = min (rectLeft.bottom, m_rectList.bottom);

				if (rectLeft.left < rectLeft.right)
				{
					CBrush br (m_clrGroupBackground != (COLORREF)-1 ? 
						m_clrGroupBackground : m_clrGray);
					pDC->FillRect (rectLeft, &br);
				}
			}

			if (!pProp->IsEnabled ())
			{
				clrTextOld = pDC->SetTextColor(visualManager->GetPropListDisabledTextColor((CBCGPPropList*)this));
			}

			CRect rectName = pProp->m_Rect;
			pProp->m_rectMenuButton.SetRectEmpty();
			pProp->m_rectState.SetRectEmpty();

			int nStateWidth = 0;
			if (pProp->m_bHasState)
			{
				nStateWidth = pProp->m_nStateWidth == 0 ? rectName.Height() : pProp->m_nStateWidth;
			}

			if ((!pProp->IsGroup () || pProp->m_bIsValueList || !m_bGroupNameFullWidth) &&
				pProp->HasValueField ())
			{
				rectName.right = nXCenter;

				if (m_bContextMenu && rectName.right - m_rectList.left >= 2 * rectName.Height())
				{
					pProp->m_rectMenuButton = rectName;
					rectName.right -= rectName.Height();
					pProp->m_rectMenuButton.left = rectName.right;
	
					pProp->m_bDrawMenuButton = TRUE;
				}

				if (rectName.right - m_rectList.left >= 2 * rectName.Height())
				{
					pProp->m_rectState = rectName;
					pProp->m_rectState.left = pProp->m_rectState.right - nStateWidth;
					pProp->m_rectState.top++;

					if (pProp->m_bHasState)
					{
						rectName.right = pProp->m_rectState.left;
					}
				}
			}

			if (pProp->IsGroup ())
			{
				if (m_bGroupNameFullWidth && !m_bVSDotNetLook && !pProp->m_bIsValueList)
				{
					CRect rectFill = rectName;
					rectFill.top++;

					if (m_brBackground.GetSafeHandle () != NULL)
					{
						CBrush& br = ((CBCGPPropList*) this)->m_brBackground;
						pDC->FillRect (rectFill, &br);
					}
					else
					{
						pDC->FillRect (rectFill, &globalData.brWindow);
					}
				}

				CRect rectExpand = rectName;
				rectName.left += m_nRowHeight;
				rectExpand.right = rectName.left;

				CRgn rgnClipExpand;
				CRect rectExpandClip = rectExpand;
				rectExpandClip.bottom = min (rectExpandClip.bottom, m_rectList.bottom);

				rgnClipExpand.CreateRectRgnIndirect (&rectExpandClip);
				pDC->SelectClipRgn (&rgnClipExpand);

				pProp->OnDrawExpandBox (pDC, rectExpand);
			}
			else if (!pProp->HasValueField ())
			{
				CRect rectFill = rectName;
				rectFill.top++;

				if (m_brBackground.GetSafeHandle () != NULL)
				{
					CBrush& br = ((CBCGPPropList*) this)->m_brBackground;
					pDC->FillRect (rectFill, &br);
				}
				else
				{
					pDC->FillRect (rectFill, &globalData.brWindow);
				}
			}

			if (rectName.right > rectName.left)
			{
				CRgn rgnClipName;
				CRect rectNameClip = rectName;
				rectNameClip.bottom = min (rectNameClip.bottom, m_rectList.bottom);

				if (!pProp->IsGroup())
				{
					rectNameClip.right = nXCenter;
				}

				rgnClipName.CreateRectRgnIndirect (&rectNameClip);
				pDC->SelectClipRgn (&rgnClipName);

				HFONT hOldFont = NULL;
				if (pProp->IsGroup () && !pProp->m_bIsValueList)
				{
					hOldFont = (HFONT) ::SelectObject (pDC->GetSafeHdc (), m_fontBold.GetSafeHandle ());
				}

				pProp->OnDrawName (pDC, rectName);

				pDC->SelectClipRgn (NULL);

				if (hOldFont != NULL)
				{
					::SelectObject (pDC->GetSafeHdc (), hOldFont);
				}
			}

			if (!pProp->m_rectMenuButton.IsRectEmpty() && pProp->m_rectMenuButton.right > pProp->m_rectMenuButton.left)
			{
				CRgn rgnClipAdvanced;
				CRect rectAdvancedClip = pProp->m_rectMenuButton;
				rectAdvancedClip.bottom = min (rectAdvancedClip.bottom, m_rectList.bottom);

				rgnClipAdvanced.CreateRectRgnIndirect (&rectAdvancedClip);

				pDC->SelectClipRgn (&rgnClipAdvanced);
				pProp->OnDrawMenuButton(pDC, pProp->m_rectMenuButton);
				pDC->SelectClipRgn (NULL);
			}

			if (!pProp->m_rectState.IsRectEmpty() && pProp->m_rectState.right > pProp->m_rectState.left && pProp->m_bHasState)
			{
				CRgn rgnClipAdvanced;
				CRect rectAdvancedClip = pProp->m_rectState;
				rectAdvancedClip.bottom = min (rectAdvancedClip.bottom, m_rectList.bottom);
				
				rgnClipAdvanced.CreateRectRgnIndirect (&rectAdvancedClip);

				pDC->SelectClipRgn (&rgnClipAdvanced);
				pProp->OnDrawStateIndicator(pDC, pProp->m_rectState);
				pDC->SelectClipRgn (NULL);
			}

			CRect rectValue = pProp->m_Rect;
			rectValue.left = nXCenter + 1;

			CRgn rgnClipVal;
			CRect rectValClip = rectValue;
			rectValClip.bottom = min (rectValClip.bottom, m_rectList.bottom);

			rgnClipVal.CreateRectRgnIndirect (&rectValClip);
			pDC->SelectClipRgn (&rgnClipVal);

			pProp->OnDrawValue (pDC, rectValue);

			if (!pProp->m_rectButton.IsRectEmpty ())
			{
				pProp->OnDrawButton (pDC, pProp->m_rectButton);
			}

			pDC->SelectClipRgn (NULL);

			pDC->MoveTo (m_rectList.left, pProp->m_Rect.bottom);
			pDC->LineTo (m_rectList.right, pProp->m_Rect.bottom);

			if (pProp->m_bHasState && pProp->m_bDrawStateBorder)
			{
				CRect rectBorder = rectValue;
				pDC->Draw3dRect(rectBorder, pProp->m_clrState, pProp->m_clrState);
			}
			
			if (clrTextOld != (COLORREF)-1)
			{
				pDC->SetTextColor (clrTextOld);
			}
		}
	}

	if (pProp->IsExpanded () || m_bAlphabeticMode)
	{
		for (POSITION pos = pProp->m_lstSubItems.GetHeadPosition (); pos != NULL;)
		{
			if (!OnDrawProperty (pDC, pProp->m_lstSubItems.GetNext (pos)))
			{
				return FALSE;
			}
		}
	}

	return TRUE;
}
//****************************************************************************************
void CBCGPPropList::OnPropertyChanged (CBCGPProp* pProp) const
{
	ASSERT_VALID (this);
	ASSERT_VALID (pProp);

	pProp->SetModifiedFlag ();

	GetOwner ()->SendMessage (BCGM_PROPERTY_CHANGED, GetDlgCtrlID (),
		LPARAM (pProp));
}
//*******************************************************************************************
BOOL CBCGPPropList::OnEraseBkgnd(CDC* /*pDC*/) 
{
	return TRUE;
}
//******************************************************************************************
void CBCGPPropList::OnHeaderItemChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMHEADER* pHeader = (NMHEADER*) pNMHDR;

	if (pHeader->iItem == 0)
	{
		HDITEM hdItem;
		hdItem.mask = HDI_WIDTH;

		GetHeaderCtrl ().GetItem (0, &hdItem);

		CRect rectClient;
		GetClientRect (rectClient);

		m_nLeftColumnWidth = min (max (m_nRowHeight, hdItem.cxy - 2), rectClient.Width () - ::GetSystemMetrics (SM_CXHSCROLL) - 5);

		ReposProperties ();

		InvalidateRect (m_rectList);
		UpdateWindow ();
	}
	
	*pResult = 0;
}
//******************************************************************************************
void CBCGPPropList::EnableToolBar(BOOL bShowStandardButtons, UINT nCustomToolbarID, UINT nCustomToolbarBitmapID)
{
	ASSERT_VALID (this);

	CBCGPProp* pProp = GetCurSel ();
	if (pProp != NULL)
	{
		pProp->OnEndEdit ();
	}

	m_bToolbarStandardButtons = bShowStandardButtons;

	m_nCustomToolbarID = nCustomToolbarID;
	m_nCustomToolbarBitmapID = nCustomToolbarBitmapID;

	m_bToolBar = m_bToolbarStandardButtons || m_nCustomToolbarID != 0;
	m_bToolbarCustomButtons = FALSE;

	if (m_wndToolBar.GetSafeHwnd() != NULL)
	{
		// Remove custom buttons:
		while (m_wndToolBar.GetCount() > m_nToolbarDefaultButtons)
		{
			m_wndToolBar.RemoveButton(m_wndToolBar.GetCount() - 1);
		}

		for (int i = 0; i < m_wndToolBar.GetCount(); i++)
		{
			m_wndToolBar.GetButton(i)->SetVisible(m_bToolbarStandardButtons);
		}

		SetupToolbarCustomButtons();
		m_wndToolBar.AdjustLocations();
	}

	AdjustLayout ();
}
//******************************************************************************************
void CBCGPPropList::EnableSearchBox(BOOL bEnable, LPCTSTR lpszPrompt)
{
	ASSERT_VALID (this);

	CBCGPProp* pProp = GetCurSel ();
	if (pProp != NULL)
	{
		pProp->OnEndEdit ();
	}

	m_bFilterBox = bEnable;

	if (lpszPrompt == NULL)
	{
		CBCGPLocalResource locaRes;
		m_strFilterPrompt.LoadString(IDS_BCGBARRES_SEARCH_PROMPT);
	}
	else
	{
		m_strFilterPrompt = lpszPrompt;
	}

	if (m_bFilterBox)
	{
		m_wndFilter.EnableSearchMode(TRUE, m_strFilterPrompt);
		AdjustLayout ();
	}
	else
	{
		m_wndFilter.SetWindowText(_T(""));
		SetPropertiesFilter(NULL);
	}
}
//******************************************************************************************
void CBCGPPropList::EnableHeaderCtrl (BOOL bEnable, LPCTSTR lpszLeftColumn,
									  LPCTSTR lpszRightColumn)
{
	ASSERT_VALID (this);
	ASSERT (lpszLeftColumn != NULL);
	ASSERT (lpszRightColumn != NULL);

	CBCGPProp* pProp = GetCurSel ();
	if (pProp != NULL)
	{
		pProp->OnEndEdit ();
	}

	m_bHeaderCtrl = bEnable;

	if (m_bHeaderCtrl)
	{
		HDITEM hdItem;
		hdItem.mask = HDI_TEXT;

		hdItem.pszText = (LPTSTR) lpszLeftColumn;
		hdItem.cchTextMax = lstrlen (lpszLeftColumn) + 1;
		GetHeaderCtrl ().SetItem (0, &hdItem);

		hdItem.pszText = (LPTSTR) lpszRightColumn;
		hdItem.cchTextMax = lstrlen (lpszRightColumn) + 1;
		GetHeaderCtrl ().SetItem (1, &hdItem);
	}

	AdjustLayout ();
}
//******************************************************************************************
void CBCGPPropList::EnableDescriptionArea (BOOL bEnable)
{
	ASSERT_VALID (this);

	m_bDescriptionArea = bEnable;

	AdjustLayout ();

	if (GetSafeHwnd () != NULL)
	{
		RedrawWindow ();
	}
}
//******************************************************************************************
void CBCGPPropList::EnableContextMenu(BOOL bEnable, UINT nMenuFlags)
{
	ASSERT_VALID (this);

	m_bContextMenu = bEnable;
	if (bEnable)
	{
		m_nItemMenuFlags = nMenuFlags;
	}

	AdjustLayout ();

	if (GetSafeHwnd () != NULL)
	{
		RedrawWindow ();
	}
}
//*****************************************************************************************
void CBCGPPropList::SetCommands (const CStringList& lstCommands, int nCommandRows)
{
	ASSERT_VALID (this);

	m_lstCommands.RemoveAll ();
	m_arCommandRects.RemoveAll ();

	m_lstCommands.AddTail ((CStringList*)&lstCommands);

	for (int i = 0; i < lstCommands.GetCount (); i++)
	{
		m_arCommandRects.Add (CRect (0, 0, 0, 0));
	}

	if (m_nCommandRows != nCommandRows)
	{
		m_nCommandRows = nCommandRows;
		m_nCommandsHeight = -1;
	}

	AdjustLayout ();
}
//*****************************************************************************************
void CBCGPPropList::ClearCommands ()
{
	ASSERT_VALID (this);

	m_lstCommands.RemoveAll ();
	m_arCommandRects.RemoveAll ();

	AdjustLayout ();

	if (GetSafeHwnd () != NULL)
	{
		RedrawWindow ();
	}
}
//*****************************************************************************************
void CBCGPPropList::SetCommandsVisible(BOOL bSet)
{
	ASSERT_VALID (this);
	
	m_bAreCommandsVisible = bSet;

	AdjustLayout ();

	if (GetSafeHwnd () != NULL)
	{
		RedrawWindow ();
	}
}
//*****************************************************************************************
void CBCGPPropList::OnHeaderTrack(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMHEADER* pHeader = (NMHEADER*) pNMHDR;

	pHeader->pitem->cxy = min (pHeader->pitem->cxy, m_rectList.Width ());

	TrackHeader (pHeader->pitem->cxy);
	*pResult = 0;
}
//******************************************************************************************
void CBCGPPropList::OnHeaderEndTrack(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	TrackHeader (-1);
	*pResult = 0;
}
//*****************************************************************************************
void CBCGPPropList::OnSpinDeltaPos(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	*pResult = 0;

	PostMessage (UM_UPDATESPIN);
}
//*****************************************************************************************
LRESULT CBCGPPropList::OnUpdateSpin (WPARAM, LPARAM)
{
	if (m_pSel != NULL && m_pSel->m_bInPlaceEdit && m_pSel->m_bEnabled)
	{
		m_pSel->OnUpdateValue ();
	}

	return 0;
}
//*****************************************************************************************
void CBCGPPropList::TrackHeader (int nOffset)
{
	CClientDC dc (this);

	if (!m_rectTrackHeader.IsRectEmpty () && !m_bShowDragContext)
	{
		dc.InvertRect (m_rectTrackHeader);
	}

	if (nOffset < 0)	// End of track
	{
		m_rectTrackHeader.SetRectEmpty ();
	}
	else
	{
		m_rectTrackHeader = m_rectList;
		m_rectTrackHeader.left += nOffset;
		m_rectTrackHeader.right = m_rectTrackHeader.left + 1;

		if (m_bShowDragContext)
		{
			CRect rectClient;
			GetClientRect (rectClient);

			m_nLeftColumnWidth = min (max (m_nRowHeight, nOffset), rectClient.Width () - ::GetSystemMetrics (SM_CXHSCROLL) - 5);

			HDITEM hdItem;
			hdItem.mask = HDI_WIDTH ;
			hdItem.cxy = m_nLeftColumnWidth + 2;

			GetHeaderCtrl ().SetItem (0, &hdItem);

			hdItem.cxy = rectClient.Width () + 10;
			GetHeaderCtrl ().SetItem (1, &hdItem);
		}
		else
		{
			dc.InvertRect (m_rectTrackHeader);
		}
	}
}
//*****************************************************************************************
void CBCGPPropList::TrackDescr (int nOffset)
{
	CClientDC dc (this);

	if (!m_rectTrackDescr.IsRectEmpty ())
	{
		dc.InvertRect (m_rectTrackDescr);
	}

	if (nOffset == INT_MIN)	// End of track
	{
		m_rectTrackDescr.SetRectEmpty ();
	}
	else
	{
		CRect rectClient;
		GetClientRect (rectClient);

		nOffset = max (nOffset, rectClient.top + m_nRowHeight + m_nToolbarHeight + m_nHeaderHeight);
		nOffset = min (nOffset, rectClient.bottom - m_nRowHeight);

		m_rectTrackDescr = rectClient;
		m_rectTrackDescr.top = nOffset - 1;
		m_rectTrackDescr.bottom = m_rectTrackDescr.top + 2;

		dc.InvertRect (m_rectTrackDescr);
	}
}
//*****************************************************************************************
void CBCGPPropList::TrackCommands (int nOffset)
{
	CClientDC dc (this);

	if (!m_rectTrackCommands.IsRectEmpty ())
	{
		dc.InvertRect (m_rectTrackCommands);
	}

	if (nOffset == INT_MIN)	// End of track
	{
		m_rectTrackCommands.SetRectEmpty ();
	}
	else
	{
		CRect rectClient;
		GetClientRect (rectClient);

		nOffset = max (nOffset, rectClient.top + m_nRowHeight + m_nToolbarHeight + m_nHeaderHeight);
		nOffset = min (nOffset, rectClient.bottom - m_nRowHeight);

		m_rectTrackCommands = rectClient;
		m_rectTrackCommands.top = nOffset - 1;
		m_rectTrackCommands.bottom = m_rectTrackCommands.top + 2;

		dc.InvertRect (m_rectTrackCommands);
	}
}
//******************************************************************************************
void CBCGPPropList::TrackToolTip (CPoint point)
{
	if (m_bTracking || m_bTrackingDescr || m_bTrackingCommands)
	{
		return;
	}

	CPoint ptScreen = point;
	ClientToScreen (&ptScreen);

	CRect rectTT;
	m_IPToolTip.GetWindowRect (&rectTT);

	if (rectTT.PtInRect (ptScreen) && m_IPToolTip.IsWindowVisible ())
	{
		return;
	}

	if (!m_IPToolTip.IsWindowVisible ())
	{
		rectTT.SetRectEmpty ();
	}

	if (::GetCapture () == GetSafeHwnd ())
	{
		ReleaseCapture ();
	}

	CBCGPProp* pProp = HitTest (point);
	if (pProp == NULL)
	{
		m_IPToolTip.Deactivate ();
		return;
	}
	
	int nMenuWidth = pProp->m_rectMenuButton.Width();

	if (abs (point.x - (m_rectList.left + m_nLeftColumnWidth)) <= STRETCH_DELTA)
	{
		m_IPToolTip.Deactivate ();
		return;
	}

	ASSERT_VALID (pProp);

	if (pProp->IsInPlaceEditing ())
	{
		return;
	}

	CString strTipText;
	CRect rectToolTip = pProp->m_Rect;

	BOOL bIsValueTT = FALSE;

	if (point.x < m_rectList.left + m_nLeftColumnWidth)
	{
		if (nMenuWidth > 0 && point.x > m_rectList.left + m_nLeftColumnWidth - nMenuWidth)
		{
			m_IPToolTip.Deactivate ();
			return;
		}

		if (pProp->IsGroup ())
		{
			rectToolTip.left += m_nRowHeight;

			if (point.x <= rectToolTip.left)
			{
				m_IPToolTip.Deactivate ();
				return;
			}
		}

		if (pProp->m_bNameIsTrancated)
		{
			if (!m_bAlwaysShowUserTT || pProp->GetNameTooltip ().IsEmpty ())
			{
				strTipText = pProp->m_strName;
			}
		}
	}
	else
	{
		if (pProp->m_bValueIsTrancated)
		{
			if (!m_bAlwaysShowUserTT || pProp->GetValueTooltip ().IsEmpty ())
			{
				strTipText = pProp->FormatProperty ();
			}
		}

		rectToolTip.left = m_rectList.left + m_nLeftColumnWidth + 1;
		bIsValueTT = TRUE;
	}

	if (!strTipText.IsEmpty ())
	{
		ClientToScreen (&rectToolTip);

		if (rectTT.TopLeft () == rectToolTip.TopLeft ())
		{
			// Tooltip on the same place, don't show it to prevent flashing
			return;
		}
		
		m_IPToolTip.SetTextMargin (TEXT_MARGIN);
		
		m_IPToolTip.SetFont (
			bIsValueTT && pProp->IsModified () && m_bMarkModifiedProperties ? 
				&m_fontBold : GetFont ());

		m_IPToolTip.Track (rectToolTip, strTipText);
		SetCapture ();
	}
	else
	{
		m_IPToolTip.Deactivate ();
	}
}
//******************************************************************************************
int CBCGPPropList::AddProperty (CBCGPProp* pProp, BOOL bRedraw, BOOL bAdjustLayout)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pProp);

	for (POSITION pos = m_lstProps.GetHeadPosition (); pos != NULL;)
	{
		CBCGPProp* pListProp = m_lstProps.GetNext (pos);
		ASSERT_VALID (pListProp);

		if (pListProp == pProp || pListProp->IsSubItem (pProp))
		{
			// Can't ad the same property twice
			ASSERT (FALSE);
			return -1;
		}
	}

	pProp->SetOwnerList (this);

	m_lstProps.AddTail (pProp);
	int nIndex = (int) m_lstProps.GetCount () - 1;

	if (bAdjustLayout)
	{
		AdjustLayout ();

		if (bRedraw && GetSafeHwnd () != NULL)
		{
			pProp->Redraw ();
		}
	}

	return nIndex;
}
//*****************************************************************************************
void CBCGPPropList::RemoveAll ()
{
	ASSERT_VALID (this);

	while (!m_lstProps.IsEmpty ())
	{
		delete m_lstProps.RemoveHead ();
	}

	while (m_nTooltipsCount > 0)
	{
		m_ToolTip.DelTool (this, m_nTooltipsCount);
		m_nTooltipsCount--;
	}

	m_lstTerminalProps.RemoveAll ();
	m_lstCommands.RemoveAll ();
	m_arCommandRects.RemoveAll ();

	m_pSel = NULL;
	m_pTracked = NULL;
}
//******************************************************************************************
CBCGPProp* CBCGPPropList::GetProperty (int nIndex) const
{
	ASSERT_VALID (this);

	if (nIndex < 0 || nIndex >= m_lstProps.GetCount ())
	{
		ASSERT (FALSE);
		return NULL;
	}

	return m_lstProps.GetAt (m_lstProps.FindIndex (nIndex));
}
//******************************************************************************************
CBCGPProp* CBCGPPropList::FindItemByData (DWORD_PTR dwData, BOOL bSearchSubItems/* = TRUE*/) const
{
	ASSERT_VALID (this);

	for (POSITION pos = m_lstProps.GetHeadPosition (); pos != NULL;)
	{
		CBCGPProp* pProp = m_lstProps.GetNext (pos);
		ASSERT_VALID (pProp);

		if (pProp->m_dwData == dwData)
		{
			return pProp;
		}

		if (bSearchSubItems) 
		{
			pProp = pProp->FindSubItemByData (dwData);

			if  (pProp != NULL)
			{
				ASSERT_VALID (pProp);
				return pProp;
			}
		}
	}

	return NULL;
}
//******************************************************************************************
CBCGPProp* CBCGPPropList::FindItemByID(UINT nID, BOOL bSearchSubItems/* = TRUE*/) const
{
	ASSERT_VALID (this);

	if (nID == (UINT)-1)
	{
		return NULL;
	}

	for (POSITION pos = m_lstProps.GetHeadPosition (); pos != NULL;)
	{
		CBCGPProp* pProp = m_lstProps.GetNext (pos);
		ASSERT_VALID (pProp);

		if (pProp->m_nID == nID)
		{
			return pProp;
		}

		if (bSearchSubItems) 
		{
			pProp = pProp->FindSubItemByID(nID);

			if  (pProp != NULL)
			{
				ASSERT_VALID (pProp);
				return pProp;
			}
		}
	}

	return NULL;
}
//*******************************************************************************************
CBCGPProp* CBCGPPropList::HitTest (CPoint pt, CBCGPProp::ClickArea* pnArea,
								   BOOL bPropsOnly) const
{
	ASSERT_VALID (this);

	if (!m_rectList.PtInRect (pt) && !bPropsOnly)
	{
		CRect rectClient;
		GetClientRect (rectClient);

		CRect rectDescr = rectClient;
		rectDescr.top = m_rectList.bottom;

		if (pnArea != NULL && rectDescr.PtInRect (pt))
		{
			*pnArea = CBCGPProp::ClickDescription;
		}

		return NULL;
	}

	const CList<CBCGPProp*, CBCGPProp*>& lst = m_bAlphabeticMode ? 
		m_lstTerminalProps : m_lstProps;

	for (POSITION pos = lst.GetHeadPosition (); pos != NULL;)
	{
		CBCGPProp* pProp = lst.GetNext (pos);
		ASSERT_VALID (pProp);

		CBCGPProp* pHit = pProp->HitTest (pt, pnArea);
		if (pHit != NULL)
		{
			return pHit;
		}
	}

	return NULL;
}
//*******************************************************************************************
BOOL CBCGPPropList::UpdateProperty(UINT nID, const _variant_t& varValue)
{
	ASSERT_VALID (this);

	CBCGPProp* pProp = FindItemByID(nID);
	if (pProp == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	pProp->SetValue(varValue);
	pProp->SetModifiedFlag();
	return TRUE;
}
//*******************************************************************************************
BOOL CBCGPPropList::UpdateBrushProperty(UINT nID, const CBCGPBrush& brush)
{
	ASSERT_VALID (this);

	CBCGPBrushProp* pProp = DYNAMIC_DOWNCAST(CBCGPBrushProp, FindItemByID(nID));
	if (pProp == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	pProp->SetBrush(brush);
	pProp->SetModifiedFlag();
	return TRUE;
}
//*******************************************************************************************
BOOL CBCGPPropList::UpdateTextFormatProperty(UINT nID, const CBCGPTextFormat& tf)
{
	ASSERT_VALID (this);

	CBCGPTextFormatProp* pProp = DYNAMIC_DOWNCAST(CBCGPTextFormatProp, FindItemByID(nID));
	if (pProp == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	pProp->SetTextFormat(tf);
	pProp->SetModifiedFlag();
	return TRUE;
}
//*******************************************************************************************
BOOL CBCGPPropList::UpdateColorProperty(UINT nID, COLORREF color)
{
	ASSERT_VALID (this);

	CBCGPColorProp* pProp = DYNAMIC_DOWNCAST(CBCGPColorProp, FindItemByID(nID));
	if (pProp == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	pProp->SetColor(color);
	pProp->SetModifiedFlag();
	return TRUE;
}
//*******************************************************************************************
BOOL CBCGPPropList::UpdateDateTimeProperty(UINT nID, const COleDateTime& date)
{
	ASSERT_VALID (this);

	CBCGPDateTimeProp* pProp = DYNAMIC_DOWNCAST(CBCGPDateTimeProp, FindItemByID(nID));
	if (pProp == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	pProp->SetDate(date);
	pProp->SetModifiedFlag();
	return TRUE;
}
//*******************************************************************************************
BOOL CBCGPPropList::UpdateFontProperty(UINT nID, const LOGFONT& lf)
{
	ASSERT_VALID (this);

	CBCGPFontProp* pProp = DYNAMIC_DOWNCAST(CBCGPFontProp, FindItemByID(nID));
	if (pProp == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	pProp->SetLogFont(lf);
	pProp->SetModifiedFlag();
	return TRUE;
}
//*******************************************************************************************
const _variant_t& CBCGPPropList::GetPropertyValue(UINT nID) const
{
	static _variant_t varEmpty;

	ASSERT_VALID (this);

	CBCGPProp* pProp = FindItemByID(nID);
	if (pProp == NULL)
	{
		ASSERT(FALSE);
		return varEmpty;
	}

	return pProp->GetValue();
}
//*******************************************************************************************
const CBCGPBrush* CBCGPPropList::GetBrushPropertyValue(UINT nID) const
{
	ASSERT_VALID (this);

	CBCGPBrushProp* pProp = DYNAMIC_DOWNCAST(CBCGPBrushProp, FindItemByID(nID));
	if (pProp == NULL)
	{
		ASSERT(FALSE);
		return NULL;
	}

	return &pProp->GetBrush();
}
//*******************************************************************************************
const CBCGPTextFormat* CBCGPPropList::GetTextFormatPropertyValue(UINT nID) const
{
	ASSERT_VALID (this);

	CBCGPTextFormatProp* pProp = DYNAMIC_DOWNCAST(CBCGPTextFormatProp, FindItemByID(nID));
	if (pProp == NULL)
	{
		ASSERT(FALSE);
		return NULL;
	}

	return &pProp->GetTextFormat();
}
//*******************************************************************************************
COLORREF CBCGPPropList::GetColorPropertyValue(UINT nID) const
{
	ASSERT_VALID (this);

	CBCGPColorProp* pProp = DYNAMIC_DOWNCAST(CBCGPColorProp, FindItemByID(nID));
	if (pProp == NULL)
	{
		ASSERT(FALSE);
		return (COLORREF)-1;
	}

	return pProp->GetColor();
}
//*******************************************************************************************
COleDateTime CBCGPPropList::GetDateTimePropertyValue(UINT nID) const
{
	ASSERT_VALID (this);

	CBCGPDateTimeProp* pProp = DYNAMIC_DOWNCAST(CBCGPDateTimeProp, FindItemByID(nID));
	if (pProp == NULL)
	{
		ASSERT(FALSE);
		return COleDateTime();
	}

	return (DATE)pProp->GetValue();
}
//*******************************************************************************************
const LOGFONT* CBCGPPropList::GetFontPropertyValue(UINT nID) const
{
	ASSERT_VALID (this);

	CBCGPFontProp* pProp = DYNAMIC_DOWNCAST(CBCGPFontProp, FindItemByID(nID));
	if (pProp == NULL)
	{
		ASSERT(FALSE);
		return NULL;
	}

	return pProp->GetLogFont();
}
//*******************************************************************************************
BOOL CBCGPPropList::EnableProperty(UINT nID, BOOL bEnable)
{
	ASSERT_VALID (this);

	CBCGPProp* pProp = FindItemByID(nID);
	if (pProp == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	pProp->Enable(bEnable);
	return TRUE;
}
//*******************************************************************************************
BOOL CBCGPPropList::SelectPropertyOption(UINT nID, int nIndex)
{
	ASSERT_VALID (this);

	CBCGPProp* pProp = FindItemByID(nID);
	if (pProp == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	return pProp->SelectOption(nIndex);
}
//*******************************************************************************************
void CBCGPPropList::RedrawProperty(UINT nID)
{
	ASSERT_VALID (this);

	CBCGPProp* pProp = FindItemByID(nID);
	if (pProp == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	pProp->Redraw();
}
//*******************************************************************************************
void CBCGPPropList::SetCurSel (CBCGPProp* pProp, BOOL bRedraw)
{
	ASSERT_VALID (this);

	CBCGPProp* pOldSelectedItem = m_pSel;
	if (pOldSelectedItem == pProp)
	{
		NotifyAccessibility (m_pSel);
		return;
	}

	if (m_pSel != NULL && m_pSel->m_bInPlaceEdit)
	{
		EndEditItem ();
	}

	m_pSel = pProp;
	OnChangeSelection (m_pSel, pOldSelectedItem);

	if (pOldSelectedItem != NULL)
	{
		pOldSelectedItem->OnKillSelection (pProp);

		CRect rectButton = pOldSelectedItem->m_rectButton;

		if (!pOldSelectedItem->IsButtonVisible())
		{
			pOldSelectedItem->m_rectButton.SetRectEmpty ();
		}

		if (bRedraw)
		{
			CRect rectOld = pOldSelectedItem->m_Rect;

			if (!pOldSelectedItem->IsGroup () || !m_bGroupNameFullWidth)
			{
				if (!pOldSelectedItem->IsGroup () && pOldSelectedItem->HasValueField ())
				{
					rectOld.right = rectOld.left + m_nLeftColumnWidth;
				}
			}

			rectOld.right = max (rectButton.right, rectOld.right);
			InvalidateRect (rectButton);
			InvalidateRect (rectOld);
		}
	}

	if (pProp != NULL)
	{
		pProp->OnSetSelection (pOldSelectedItem);

		if (pProp->HasButton ())
		{
			pProp->AdjustButtonRect ();
		}

		if (bRedraw)
		{
			CRect rect = pProp->m_Rect;

			if (!pProp->IsGroup () || !m_bGroupNameFullWidth)
			{
				if (!pProp->IsGroup () && pProp->HasValueField ())
				{
					rect.right = rect.left + m_nLeftColumnWidth;
				}
			}

			rect.right = max (pProp->m_rectButton.right, rect.right);
			InvalidateRect (rect);
			InvalidateRect (pProp->m_rectButton);
		}
	}

	if (bRedraw)
	{
		if (m_bDescriptionArea || (HasCommands () && m_bAreCommandsVisible))
		{
			CRect rectClient;
			GetClientRect (rectClient);

			CRect rect = rectClient;
			rect.top = m_rectList.bottom;
			InvalidateRect (rect);
		}

		UpdateWindow ();
	}

	NotifyAccessibility (m_pSel);
}
//******************************************************************************************
void CBCGPPropList::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CWnd::OnLButtonDown(nFlags, point);

	SetFocus ();

	CRect rectClient;
	GetClientRect (rectClient);

	int nCommandsHeight = 0;

	if (HasCommands () && m_bAreCommandsVisible)
	{
		nCommandsHeight = m_nCommandsHeight;

		if (abs (point.y - (m_rectList.bottom + TEXT_MARGIN)) <= STRETCH_DELTA)
		{
			SetCapture ();
			TrackCommands (point.y);
			m_bTrackingCommands = TRUE;
			return;
		}

		if (point.y > m_rectList.bottom && !m_bDescriptionArea)
		{
			return;
		}
	}

	if (m_bDescriptionArea)
	{
		if (abs (point.y - (m_rectList.bottom + nCommandsHeight + TEXT_MARGIN)) <= STRETCH_DELTA)
		{
			SetCapture ();
			TrackDescr (point.y);
			m_bTrackingDescr = TRUE;
			return;
		}

		if (point.y > m_rectList.bottom)
		{
			return;
		}
	}

	if (abs (point.x - (m_rectList.left + m_nLeftColumnWidth)) <= STRETCH_DELTA && !m_bOutOfFilter)
	{
		SetCapture ();
		TrackHeader (point.x);
		m_bTracking = TRUE;
		return;
	}

	CBCGPProp::ClickArea clickArea;
	CBCGPProp* pHit = HitTest (point, &clickArea);

	BOOL bSelChanged = pHit != GetCurSel ();

	SetCurSel (pHit);
	if (pHit == NULL)
	{
		return;
	}

	EnsureVisible (pHit);

	switch (clickArea)
	{
	case CBCGPProp::ClickExpandBox:
		pHit->Expand (!pHit->IsExpanded ());
		break;

	case CBCGPProp::ClickGroupArea:
		pHit->OnClickGroupArea (point);
		break;

	case CBCGPProp::ClickName:
		pHit->OnClickName (point);
		break;

	case CBCGPProp::ClickMenuButton:
		pHit->OnClickMenuButton (CPoint(-1, -1));
		break;

	case CBCGPProp::ClickValue:
		if (EditItem (pHit, &point) && (pHit->m_pWndInPlace != NULL || pHit->NoInplaceEdit()))
		{
			if (pHit->m_rectButton.PtInRect (point))
			{
				CString strPrevVal = pHit->FormatProperty ();

				if (::GetCapture () == GetSafeHwnd ())
				{
					ReleaseCapture ();
				}

				if (pHit->m_bEnabled)
				{
					pHit->OnClickButton (point);
				}

				if (strPrevVal != pHit->FormatProperty ())
				{
					OnPropertyChanged (pHit);
				}
			}
			else if (!bSelChanged || pHit->IsProcessFirstClick ())
			{
				pHit->OnClickValue (WM_LBUTTONDOWN, point);
			}
		}
		break;

	default:
		break;
	}
}
//************************************************************************************
void CBCGPPropList::OnRButtonDown(UINT nFlags, CPoint point) 
{
	CWnd::OnRButtonDown(nFlags, point);

	if (m_bTracking || m_bTrackingCommands || m_bTrackingDescr)
	{
		return;
	}

	SetFocus ();

	CRect rectClient;
	GetClientRect (rectClient);

	if (m_bDescriptionArea)
	{
		if (abs (point.y - (m_rectList.bottom + TEXT_MARGIN)) <= STRETCH_DELTA)
		{
			return;
		}

		if (point.y > m_rectList.bottom)
		{
			return;
		}
	}

	if (abs (point.x - (m_rectList.left + m_nLeftColumnWidth)) <= STRETCH_DELTA)
	{
		return;
	}

	CBCGPProp::ClickArea clickArea;
	CBCGPProp* pHit = HitTest (point, &clickArea);

	BOOL bSelChanged = pHit != GetCurSel ();

	SetCurSel (pHit);
	if (pHit == NULL)
	{
		return;
	}

	EnsureVisible (pHit);

	switch (clickArea)
	{
	case CBCGPProp::ClickExpandBox:
		break;

	case CBCGPProp::ClickGroupArea:
		pHit->OnRClickGroupArea(point);
		break;

	case CBCGPProp::ClickName:
		pHit->OnRClickName (point);
		break;

	case CBCGPProp::ClickMenuButton:
		pHit->OnRClickMenuButton (point);
		break;

	case CBCGPProp::ClickValue:
		pHit->OnRClickValue (point, bSelChanged);

		if (pHit->m_bEnabled && !bSelChanged)
		{
			if (EditItem (pHit, &point) && pHit->m_pWndInPlace != NULL)
			{
				if (pHit->m_rectButton.PtInRect (point))
				{
					return;
				}
				else if (pHit->IsProcessFirstClick ())
				{
					pHit->OnClickValue (WM_RBUTTONDOWN, point);
				}
			}
		}
		break;

	default:
		break;
	}
}
//******************************************************************************************
void CBCGPPropList::OnRButtonUp(UINT nFlags, CPoint point) 
{
	if (m_bTracking || m_bTrackingCommands || m_bTrackingDescr)
	{
		return;
	}

	CBCGPProp::ClickArea clickArea;
	CBCGPProp* pHit = HitTest (point, &clickArea);

	if (pHit != NULL && m_bContextMenu)
	{
		switch (clickArea)
		{
		case CBCGPProp::ClickName:
		case CBCGPProp::ClickMenuButton:
			if (!pHit->IsGroup () || pHit->m_bIsValueList)
			{
				pHit->OnClickMenuButton(point);
			}
		}

		return;
	}

	CWnd::OnRButtonUp(nFlags, point);
}
//******************************************************************************************
BOOL CBCGPPropList::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	return CWnd::Create (globalData.RegisterWindowClass (_T("BCGPPropList")),
			_T(""), dwStyle, rect, pParentWnd, nID, NULL);
}
//******************************************************************************************
BOOL CBCGPPropList::EditItem (CBCGPProp* pProp, LPPOINT lptClick)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pProp);

	if (!EndEditItem ())
	{
		return FALSE;
	}

	if (pProp->IsGroup () && !pProp->m_bIsValueList)
	{
		return FALSE;
	}

	EnsureVisible(pProp);

	if (pProp->OnEdit (lptClick))
	{
		pProp->Redraw ();
		SetCurSel (pProp);
		SetCapture ();
	}

	return TRUE;
}
//******************************************************************************************
void CBCGPPropList::OnClickButton (CPoint point)
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pSel);

	if (m_pSel->IsEnabled() && m_pSel->OnUpdateValue ())
	{
		CString strPrevVal = m_pSel->FormatProperty ();

		if (m_pSel->m_dwFlags & PROP_HAS_BUTTON)
		{
			CWaitCursor wait;
			m_pSel->OnClickButton (point);
		}
		else
		{
			m_pSel->OnClickButton (point);
		}

		if (strPrevVal != m_pSel->FormatProperty ())
		{
			OnPropertyChanged (m_pSel);
		}
	}
}
//******************************************************************************************
BOOL CBCGPPropList::EndEditItem (BOOL bUpdateData/* = TRUE*/)
{
	ASSERT_VALID (this);

	if (m_pSel == NULL)
	{
		return TRUE;
	}

	ASSERT_VALID (m_pSel);

	if (!m_pSel->m_bInPlaceEdit)
	{
		return TRUE;
	}

	if (bUpdateData)
	{
		if (!ValidateItemData (m_pSel) || !m_pSel->OnUpdateValue ())
		{
			return FALSE;
		}
	}

	if (m_pSel != NULL && !m_pSel->OnEndEdit ())
	{
		return FALSE;
	}

	if (::GetCapture () == GetSafeHwnd ())
	{
		ReleaseCapture ();
	}

	if (m_pSel != NULL)
	{
		m_pSel->Redraw ();
	}

	return TRUE;
}
//*******************************************************************************************
BOOL CBCGPPropList::PreTranslateMessage(MSG* pMsg) 
{
	if (CBCGPPopupMenu::GetActiveMenu () != NULL)
	{
		return CWnd::PreTranslateMessage(pMsg);
	}

   	switch (pMsg->message)
	{
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
	case WM_NCLBUTTONDOWN:
	case WM_NCRBUTTONDOWN:
	case WM_NCMBUTTONDOWN:
	case WM_NCLBUTTONUP:
	case WM_NCRBUTTONUP:
	case WM_NCMBUTTONUP:
		m_ToolTip.RelayEvent(pMsg);
		m_IPToolTip.Hide ();
		break;

	case WM_MOUSEMOVE:
		m_ToolTip.RelayEvent(pMsg);

		if (pMsg->wParam == 0)	// No buttons pressed
		{
			CPoint ptCursor;
			::GetCursorPos (&ptCursor);
			ScreenToClient (&ptCursor);

			TrackToolTip (ptCursor);
		}
		break;
	}

	if (pMsg->message == WM_KEYDOWN &&
		pMsg->wParam == VK_TAB &&
		(GetStyle() & WS_TABSTOP) == 0 &&
		m_pSel != NULL && 
		m_pSel->OnActivateByTab ())
	{
		return TRUE;
	}

	if (pMsg->message == WM_SYSKEYDOWN && 
		(pMsg->wParam == VK_DOWN || pMsg->wParam == VK_RIGHT) && 
		m_pSel != NULL && m_pSel->m_bEnabled && 
		((m_pSel->m_dwFlags) & PROP_HAS_BUTTON) &&
		EditItem (m_pSel))
	{
		CString strPrevVal = m_pSel->FormatProperty ();

		CWaitCursor wait;
		m_pSel->OnClickButton (CPoint (-1, -1));

		if (strPrevVal != m_pSel->FormatProperty ())
		{
			OnPropertyChanged (m_pSel);
		}

		return TRUE;
	}

	if (pMsg->message == WM_KEYDOWN && CWnd::GetFocus()->GetSafeHwnd() == m_wndFilter.GetSafeHwnd())
	{
		if (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_TAB || pMsg->wParam == VK_ESCAPE)
		{
			SetFocus();
			return TRUE;
		}

		return CWnd::PreTranslateMessage(pMsg);
	}

	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN &&
		m_pSel != NULL && m_pSel->IsGroup () && !m_pSel->IsInPlaceEditing ())
	{
		m_pSel->Expand (!m_pSel->IsExpanded ());
		return TRUE;
	}

	if (m_pSel != NULL && m_pSel->m_bInPlaceEdit)
	{
		ASSERT_VALID (m_pSel);

		const BOOL bIsDroppedDown = m_pSel->m_pWndCombo != NULL && m_pSel->m_pWndCombo->GetDroppedState ();

		if (pMsg->message == WM_KEYDOWN && (!bIsDroppedDown || pMsg->wParam == VK_RETURN))
		{
			switch (pMsg->wParam)
			{
			case VK_RETURN:
				if (!m_pSel->m_bEnabled)
				{
					EndEditItem (FALSE);
				}
				else
				{
					if (m_pSel->m_bButtonIsFocused)
					{
						CString strPrevVal = m_pSel->FormatProperty ();

						CWaitCursor wait;
						m_pSel->OnClickButton (CPoint (-1, -1));

						if (strPrevVal != m_pSel->FormatProperty ())
						{
							OnPropertyChanged (m_pSel);
						}
						return TRUE;
					}

					if (bIsDroppedDown)
					{
						HWND hwndInplace = m_pSel->m_pWndInPlace->GetSafeHwnd ();
						m_pSel->OnSelectCombo ();

						if (::IsWindow (hwndInplace))
						{
							m_pSel->m_pWndInPlace->SetFocus ();
						}
					}

					if (!EndEditItem ())
					{
						MessageBeep ((UINT)-1);
					}
				}

				SetFocus ();
				break;

			case VK_ESCAPE:
				EndEditItem (FALSE);
				SetFocus ();
				break;

			case VK_DOWN:
			case VK_UP:
				if (m_pSel->m_bEnabled && m_pSel->m_lstOptions.GetCount () > 1)
				{
					if (m_pSel->OnRotateListValue (pMsg->wParam == VK_UP))
					{
						return TRUE;
					}
				}
				else if (::IsWindow (m_pSel->m_pWndInPlace->GetSafeHwnd ()))
				{
					m_pSel->m_pWndInPlace->SendMessage (WM_KEYDOWN, pMsg->wParam, pMsg->lParam);
					return TRUE;
				}
				break;

			default:
				if (m_pSel->m_bEnabled)
				{
					if (!m_pSel->m_bAllowEdit)
					{
						m_pSel->PushChar ((UINT) pMsg->wParam);
						return TRUE;
					}

					if (ProcessClipboardAccelerators ((UINT) pMsg->wParam))
					{
						return TRUE;
					}
				}
				return FALSE;
			}

			return TRUE;
		}
		else if (pMsg->message >= WM_MOUSEFIRST &&
				 pMsg->message <= WM_MOUSELAST)
		{
			CPoint ptCursor;
			::GetCursorPos (&ptCursor);
			ScreenToClient (&ptCursor);

			if (m_pSel->m_pWndSpin != NULL)
			{
				ASSERT_VALID (m_pSel->m_pWndSpin);
				ASSERT (m_pSel->m_pWndSpin->GetSafeHwnd () != NULL);

				CRect rectSpin;
				m_pSel->m_pWndSpin->GetClientRect (rectSpin);
				m_pSel->m_pWndSpin->MapWindowPoints (this, rectSpin);

				if (rectSpin.PtInRect (ptCursor))
				{
					MapWindowPoints (m_pSel->m_pWndSpin, &ptCursor, 1); 

					m_pSel->m_pWndSpin->SendMessage (pMsg->message, pMsg->wParam, 
						MAKELPARAM (ptCursor.x, ptCursor.y));
					return TRUE;
				}
			}

			CWnd* pWndInPlaceEdit = m_pSel->m_pWndInPlace;
			ASSERT_VALID (pWndInPlaceEdit);

			if (!m_pSel->m_bAllowEdit)
			{
				m_pSel->m_pWndInPlace->HideCaret ();
			}

			CRect rectEdit;
			pWndInPlaceEdit->GetClientRect (rectEdit);
			pWndInPlaceEdit->MapWindowPoints (this, rectEdit);

			if (rectEdit.PtInRect (ptCursor) &&
				pMsg->message == WM_LBUTTONDBLCLK)
			{
				if (m_pSel->OnDblClick (ptCursor))
				{
					return TRUE;
				}
			}

			if (rectEdit.PtInRect (ptCursor) && 
				pMsg->message == WM_RBUTTONDOWN &&
				!m_pSel->m_bAllowEdit)
			{
				return TRUE;
			}

			if (!rectEdit.PtInRect (ptCursor) &&
				(pMsg->message == WM_LBUTTONDOWN ||
				pMsg->message == WM_NCLBUTTONDOWN ||
				pMsg->message == WM_RBUTTONDOWN ||
				pMsg->message == WM_MBUTTONDOWN))
			{
				if (m_pSel->m_rectButton.PtInRect (ptCursor))
				{
					CString strPrevVal = m_pSel->FormatProperty ();

					if (m_pSel->m_dwFlags & PROP_HAS_BUTTON)
					{
						CWaitCursor wait;
						OnClickButton (ptCursor);
					}
					else
					{
						OnClickButton (ptCursor);
					}

					if (strPrevVal != m_pSel->FormatProperty ())
					{
						OnPropertyChanged (m_pSel);
					}
					return TRUE;
				}

				if (!EndEditItem ())
				{
					return TRUE;
				}
			}
			else
			{
				MapWindowPoints (pWndInPlaceEdit, &ptCursor, 1); 
				pWndInPlaceEdit->SendMessage (pMsg->message, pMsg->wParam, 
					MAKELPARAM (ptCursor.x, ptCursor.y));
				return TRUE;
			}
		}
		else
		{
			return FALSE;
		}
	}
	
	return CWnd::PreTranslateMessage(pMsg);
}
//*******************************************************************************************
void CBCGPPropList::OnCancelMode() 
{
	if (m_bTracking)
	{
		TrackHeader (-1);
		m_bTracking = FALSE;

		if (::GetCapture () == GetSafeHwnd ())
		{
			ReleaseCapture ();
		}
	}

	if (m_bTrackingDescr)
	{
		TrackDescr (INT_MIN);
		m_bTrackingDescr = FALSE;

		if (::GetCapture () == GetSafeHwnd ())
		{
			ReleaseCapture ();
		}
	}

	if (m_bTrackingCommands)
	{
		TrackCommands (INT_MIN);
		m_bTrackingCommands = FALSE;

		if (::GetCapture () == GetSafeHwnd ())
		{
			ReleaseCapture ();
		}
	}

	//---------
	// Tooltip:
	//---------
	if (::GetCapture () == GetSafeHwnd ())
	{
		ReleaseCapture ();
	}

	m_IPToolTip.Deactivate ();
	
	EndEditItem ();

	if (m_pTracked != NULL)
	{
		m_pTracked->OnLeaveMouse();
		m_pTracked = NULL;
	}

	CWnd::OnCancelMode();
}
//******************************************************************************************
void CBCGPPropList::OnSetFocus(CWnd* pOldWnd) 
{
	CWnd::OnSetFocus(pOldWnd);
	
	m_bFocused = TRUE;
	
	if (m_pSel != NULL)
	{
		RedrawWindow (m_pSel->m_Rect);
	}
}
//******************************************************************************************
void CBCGPPropList::OnKillFocus(CWnd* pNewWnd) 
{
	if (!IsChild (pNewWnd))
	{
		if (m_pSel == NULL || m_pSel->OnKillFocus (pNewWnd))
		{
			EndEditItem ();
			m_bFocused = FALSE;

			if (m_pSel != NULL)
			{
				m_pSel->Redraw ();
			}
		}
	}

	CWnd::OnKillFocus(pNewWnd);
}
//******************************************************************************************
void CBCGPPropList::OnStyleChanged (int nStyleType, LPSTYLESTRUCT lpStyleStruct)
{
	CWnd::OnStyleChanged (nStyleType, lpStyleStruct);
	AdjustLayout ();
}
//******************************************************************************************
UINT CBCGPPropList::OnGetDlgCode() 
{
	return DLGC_WANTARROWS | DLGC_WANTCHARS;
}
//******************************************************************************************
void CBCGPPropList::SetScrollSizes ()
{
	ASSERT_VALID (this);

	if (m_wndScrollVert.GetSafeHwnd () == NULL)
	{
		return;
	}

	if (m_nRowHeight == 0)
	{
		m_nVertScrollPage = 0;
		m_nVertScrollTotal = 0;
		m_nVertScrollOffset = 0;
	}
	else
	{
		m_nVertScrollPage = m_rectList.Height () / m_nRowHeight - 1;
		m_nVertScrollTotal = GetTotalItems (FALSE /* Visible only */);

		if (m_nVertScrollTotal <= m_nVertScrollPage)
		{
			m_nVertScrollPage = 0;
			m_nVertScrollTotal = 0;
		}

		m_nVertScrollOffset = min (m_nVertScrollOffset, m_nVertScrollTotal);
	}

	SCROLLINFO si;

	ZeroMemory (&si, sizeof (SCROLLINFO));
	si.cbSize = sizeof (SCROLLINFO);

	si.fMask = SIF_RANGE | SIF_POS | SIF_PAGE;
	si.nMin = 0;
	si.nMax = m_nVertScrollTotal;
	si.nPage = m_nVertScrollPage;
	si.nPos = m_nVertScrollOffset;

	SetScrollInfo (SB_VERT, &si, TRUE);
	m_wndScrollVert.EnableScrollBar (m_nVertScrollTotal > 0 ? ESB_ENABLE_BOTH : ESB_DISABLE_BOTH);
	m_wndScrollVert.EnableWindow ();
}
//******************************************************************************************
int CBCGPPropList::GetTotalItems (BOOL bIncludeHidden) const
{
	ASSERT_VALID (this);

	int nCount = 0;

	if (m_bAlphabeticMode)
	{
		if (bIncludeHidden)
		{
			return (int) m_lstTerminalProps.GetCount ();
		}

		for (POSITION pos = m_lstTerminalProps.GetHeadPosition (); pos != NULL;)
		{
			CBCGPProp* pProp = m_lstTerminalProps.GetNext (pos);
			ASSERT_VALID (pProp);

			if (pProp->IsVisibleInFilter())
			{
				nCount++;

				if (pProp->IsExpanded())
				{
					nCount += pProp->GetExpandedSubItems();
				}
			}
		}

		return nCount;
	}

	for (POSITION pos = m_lstProps.GetHeadPosition (); pos != NULL;)
	{
		CBCGPProp* pProp = m_lstProps.GetNext (pos);
		ASSERT_VALID (pProp);

		nCount += pProp->GetExpandedSubItems (bIncludeHidden) + 1;
	}

	return nCount;
}
//******************************************************************************************
void CBCGPPropList::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	if (m_pSel != NULL && 
		pScrollBar->GetSafeHwnd () != NULL &&
		m_pSel->m_pWndSpin->GetSafeHwnd () == pScrollBar->GetSafeHwnd () || m_bOutOfFilter)
	{
		return;
	}

	m_IPToolTip.Hide ();
	EndEditItem ();

	int nPrevOffset = m_nVertScrollOffset;

	switch (nSBCode)
	{
	case SB_LINEUP:
		m_nVertScrollOffset--;
		break;

	case SB_LINEDOWN:
		m_nVertScrollOffset++;
		break;

	case SB_TOP:
		m_nVertScrollOffset = 0;
		break;

	case SB_BOTTOM:
		m_nVertScrollOffset = m_nVertScrollTotal;
		break;

	case SB_PAGEUP:
		m_nVertScrollOffset -= m_nVertScrollPage;
		break;

	case SB_PAGEDOWN:
		m_nVertScrollOffset += m_nVertScrollPage;
		break;

	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		m_nVertScrollOffset = nPos;
		break;

	default:
		return;
	}

	m_nVertScrollOffset = min (max (0, m_nVertScrollOffset), 
		m_nVertScrollTotal - m_nVertScrollPage + 1);

	if (m_nVertScrollOffset == nPrevOffset)
	{
		return;
	}

	SetScrollPos (SB_VERT, m_nVertScrollOffset);

	ReposProperties ();

	int dy = m_nRowHeight * (nPrevOffset - m_nVertScrollOffset);
	ScrollWindow (0, dy, m_rectList, m_rectList);

	if (m_pSel != NULL)
	{
		ASSERT_VALID (m_pSel);
		RedrawWindow (m_pSel->m_rectButton);
	}
}
//*******************************************************************************************
CScrollBar* CBCGPPropList::GetScrollBarCtrl(int nBar) const
{
	if (nBar == SB_HORZ || m_wndScrollVert.GetSafeHwnd () == NULL)
	{
		return NULL;
	}

	return (CScrollBar* ) &m_wndScrollVert;
}
//******************************************************************************************
BOOL CBCGPPropList::OnMouseWheel(UINT /*nFlags*/, short zDelta, CPoint /*pt*/) 
{
	if (CBCGPPopupMenu::GetActiveMenu () != NULL)
	{
		return TRUE;
	}

	if (m_pSel != NULL)
	{
		ASSERT_VALID(m_pSel);

		if (m_pSel->m_bButtonIsDown)
		{
			return FALSE;
		}
	}

	if (m_nVertScrollTotal > 0)
	{
		const int nSteps = abs(zDelta) / WHEEL_DELTA;

		for (int i = 0; i < nSteps; i++)
		{
			OnVScroll (zDelta < 0 ? SB_LINEDOWN : SB_LINEUP, 0, NULL);
		}
	}

	return TRUE;
}
//*******************************************************************************************
void CBCGPPropList::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	CWnd::OnLButtonDblClk(nFlags, point);

	if (CBCGPPopupMenu::GetActiveMenu () != NULL)
	{
		return;
	}

	if (point.y <= m_rectList.bottom &&
		abs (point.x - (m_rectList.left + m_nLeftColumnWidth)) <= STRETCH_DELTA && !m_bOutOfFilter)
	{
		//--------------------------------------------------------
		// Double click on vertical splitter, make splitter 50/50:
		//--------------------------------------------------------
		CRect rectClient;
		GetClientRect (rectClient);

		m_nLeftColumnWidth = rectClient.Width () / 2;

		HDITEM hdItem;
		hdItem.mask = HDI_WIDTH ;
		hdItem.cxy = m_nLeftColumnWidth + 2;

		GetHeaderCtrl ().SetItem (0, &hdItem);

		hdItem.cxy = rectClient.Width () + 10;
		GetHeaderCtrl ().SetItem (1, &hdItem);
		return;
	}

	if (m_pSel == NULL)
	{
		return;
	}

	ASSERT_VALID (m_pSel);

	if (m_pSel->IsGroup () &&
		(!m_pSel->m_bIsValueList || 
		point.x < m_rectList.left + m_nLeftColumnWidth))
	{
		m_pSel->Expand (!m_pSel->IsExpanded ());
	}
	else if (m_pSel->m_bEnabled)
	{
		if (EditItem (m_pSel) && m_pSel->m_pWndInPlace != NULL)
		{
			m_pSel->m_pWndInPlace->SendMessage (WM_LBUTTONDOWN);
			m_pSel->m_pWndInPlace->SendMessage (WM_LBUTTONUP);

			CEdit* pEdit = DYNAMIC_DOWNCAST (CEdit, m_pSel->m_pWndInPlace);
			if (::IsWindow (pEdit->GetSafeHwnd ()))
			{
				pEdit->SetSel (0, -1);
			}
		}

		if (m_pSel != NULL && m_pSel->GetRect ().PtInRect (point))
		{
			m_pSel->OnDblClick (point);
		}
	}
}
//*******************************************************************************************
BOOL CBCGPPropList::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	if (nHitTest == HTCLIENT)
	{
		CPoint point;

		::GetCursorPos (&point);
		ScreenToClient (&point);

		CRect rectClient;
		GetClientRect(rectClient);

		if (point.y <= rectClient.top + m_nToolbarHeight)
		{
			return CWnd::OnSetCursor(pWnd, nHitTest, message);
		}

		if (HasCommands () && m_bAreCommandsVisible &&
			abs (point.y - (m_rectList.bottom + TEXT_MARGIN)) <= STRETCH_DELTA)
		{
			::SetCursor (globalData.m_hcurStretchVert);
			return TRUE;
		}

		int nCommandsHeight = (HasCommands() && m_bAreCommandsVisible) ? m_nCommandsHeight : 0;

		if (m_bDescriptionArea &&
			abs (point.y - (m_rectList.bottom + nCommandsHeight + TEXT_MARGIN)) <= STRETCH_DELTA)
		{
			::SetCursor (globalData.m_hcurStretchVert);
			return TRUE;
		}

		for (int i = 0; i < m_arCommandRects.GetSize (); i++)
		{
			if (m_arCommandRects [i].PtInRect (point))
			{
				::SetCursor (globalData.GetHandCursor ());
				return TRUE;
			}
		}

		if (point.y <= m_rectList.bottom && !m_bOutOfFilter)
		{
			if (abs (point.x - (m_rectList.left + m_nLeftColumnWidth)) <= STRETCH_DELTA)
			{
				::SetCursor (globalData.m_hcurStretch);
				return TRUE;
			}

			CBCGPProp::ClickArea clickArea;
			CBCGPProp* pHit = HitTest (point, &clickArea);

			if (pHit != NULL && pHit == m_pSel && clickArea == CBCGPProp::ClickValue &&
				!pHit->m_rectButton.PtInRect (point) &&
				pHit->OnSetCursor ())
			{
				return TRUE;
			}
		}
	}
	
	return CWnd::OnSetCursor(pWnd, nHitTest, message);
}
//******************************************************************************************
void CBCGPPropList::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if (m_lstProps.IsEmpty ())
	{
		CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
		return;
	}

	switch (nChar)
	{
	case VK_F4:
		if (m_pSel != NULL && m_pSel->m_bEnabled && EditItem (m_pSel))
		{
			if ((m_pSel->m_dwFlags) & PROP_HAS_BUTTON)
			{
				CString strPrevVal = m_pSel->FormatProperty ();

				CWaitCursor wait;
				m_pSel->OnClickButton (CPoint (-1, -1));

				if (strPrevVal != m_pSel->FormatProperty ())
				{
					OnPropertyChanged (m_pSel);
				}
			}
			else if (m_pSel != NULL)
			{
				CEdit* pEdit = DYNAMIC_DOWNCAST (CEdit, m_pSel->m_pWndInPlace);
				if (::IsWindow (pEdit->GetSafeHwnd ()))
				{
					pEdit->SetSel (0, -1);
				}
			}
			return;
		}
		break;

	case VK_LEFT:
		if (::GetAsyncKeyState (VK_CONTROL) & 0x8000)
		{
			BOOL bShowDragContext = m_bShowDragContext;
			m_bShowDragContext = TRUE;

			TrackHeader (m_nLeftColumnWidth - 5);

			m_bShowDragContext = bShowDragContext;
			return;
		}
		else if (m_pSel != NULL && m_pSel->IsGroup () && m_pSel->IsExpanded ())
		{
			m_pSel->Expand (FALSE);
			return;
		}
		
		// else ==> act as VK_UP!

	case VK_UP:
		{
			if (m_pSel == NULL)
			{
				SetCurSel (GetFirstNonFilteredProperty());
				OnVScroll (SB_TOP, 0, NULL);
				return;
			}

			// Select prev. item:
			CPoint point (m_pSel->m_Rect.right - 1, m_pSel->m_Rect.top - 2);

			CBCGPProp* pHit = HitTest (point, NULL, TRUE);
			if (pHit != NULL)
			{
				SetCurSel (pHit);
				EnsureVisible (pHit);
			}
		}
		return;

	case VK_RIGHT:
		if (::GetAsyncKeyState (VK_CONTROL) & 0x8000)
		{
			BOOL bShowDragContext = m_bShowDragContext;
			m_bShowDragContext = TRUE;

			TrackHeader (m_nLeftColumnWidth + 5);

			m_bShowDragContext = bShowDragContext;
			return;
		}
		else if (m_pSel != NULL && m_pSel->IsGroup () && !m_pSel->IsExpanded ())
		{
			m_pSel->Expand ();
			return;
		}
		
		// else ==> act as VK_DOWN!

	case VK_DOWN:
		{
			if (m_pSel == NULL)
			{
				SetCurSel (GetFirstNonFilteredProperty());
				OnVScroll (SB_TOP, 0, NULL);
				return;
			}

			if ((::GetAsyncKeyState (VK_MENU) & 0x8000) && nChar == VK_DOWN)
			{
				CString strPrevVal = m_pSel->FormatProperty ();

				CWaitCursor wait;
				m_pSel->OnClickButton (CPoint (-1, -1));

				if (strPrevVal != m_pSel->FormatProperty ())
				{
					OnPropertyChanged (m_pSel);
				}

				return;
			}

			// Select next item:
			CPoint point (m_pSel->m_Rect.right - 1, m_pSel->m_Rect.bottom + 2);

			CBCGPProp* pHit = HitTest (point, NULL, TRUE);
			if (pHit != NULL)
			{
				SetCurSel (pHit);
				EnsureVisible (pHit);
			}
		}
		return;

	case VK_NEXT:
		{
			if (m_pSel == NULL)
			{
				SetCurSel (GetFirstNonFilteredProperty());
				OnVScroll (SB_TOP, 0, NULL);
				return;
			}

			if (m_nVertScrollPage != 0)
			{
				EnsureVisible (m_pSel);

				CPoint point (m_pSel->m_Rect.right - 1, 
					m_pSel->m_Rect.top + m_nVertScrollPage * m_nRowHeight);

				CBCGPProp* pHit = HitTest (point, NULL, TRUE);
				if (pHit != NULL)
				{
					SetCurSel (pHit);
					OnVScroll (SB_PAGEDOWN, 0, NULL);
					return;
				}
			}
		}

	case VK_END:
		{
			SetCurSel (GetLastNonFilteredProperty());
			OnVScroll (SB_BOTTOM, 0, NULL);
		}
		return;

	case VK_PRIOR:
		{
			if (m_pSel != NULL && m_nVertScrollPage != 0)
			{
				EnsureVisible (m_pSel);

				CPoint point (m_pSel->m_Rect.right - 1, 
					m_pSel->m_Rect.top - m_nVertScrollPage * m_nRowHeight);

				CBCGPProp* pHit = HitTest (point, NULL, TRUE);
				if (pHit != NULL)
				{
					SetCurSel (pHit);
					OnVScroll (SB_PAGEUP, 0, NULL);
					return;
				}
			}
		}

	case VK_HOME:
		SetCurSel (GetFirstNonFilteredProperty());
		OnVScroll (SB_TOP, 0, NULL);
		return;

	case VK_ADD:
		if (m_pSel != NULL && m_pSel->IsGroup () && !m_pSel->IsExpanded () &&
			!m_pSel->IsInPlaceEditing ())
		{
			m_pSel->Expand ();
		}
		return;

	case VK_SUBTRACT:
		if (m_pSel != NULL && m_pSel->IsGroup () && m_pSel->IsExpanded () &&
			!m_pSel->IsInPlaceEditing ())
		{
			m_pSel->Expand (FALSE);
		}
		return;
	}

	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}
//******************************************************************************************
void CBCGPPropList::EnsureVisible (CBCGPProp* pProp, BOOL bExpandParents/* = FALSE*/)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pProp);

	if (m_nRowHeight == 0)
	{
		return;
	}

	if (bExpandParents && pProp->GetParent () != NULL)
	{
		CBCGPProp* pParent = pProp;
        
		while ((pParent = pParent->GetParent ()) != NULL)
		{
			ASSERT_VALID (pParent);
			pParent->Expand (TRUE);
		}
	}

	CRect rect = pProp->m_Rect;

	if (rect.top >= m_rectList.top - 1 && rect.bottom <= m_rectList.bottom)
	{
		return;
	}

	CRect rectButton = pProp->m_rectButton;
	pProp->m_rectButton.SetRectEmpty ();
	RedrawWindow (rectButton);

	if (rect.top < m_rectList.top - 1 && rect.bottom >= m_rectList.top - 1)
	{
		OnVScroll (SB_LINEUP, 0, NULL);
	}
	else if (rect.bottom > m_rectList.bottom && rect.top <= m_rectList.bottom)
	{
		OnVScroll (SB_LINEDOWN, 0, NULL);
	}
	else
	{
		OnVScroll (SB_THUMBPOSITION, m_nVertScrollOffset + (-m_rectList.top + rect.top) / m_nRowHeight, NULL);
	}

	if (!rectButton.IsRectEmpty ())
	{
		pProp->AdjustButtonRect ();
		RedrawWindow (pProp->m_rectButton);
	}
}
//******************************************************************************************
void CBCGPPropList::ExpandAll (BOOL bExapand/* = TRUE*/)
{
	ASSERT_VALID (this);

	if (m_bAlphabeticMode)
	{
		return;
	}

	for (POSITION pos = m_lstProps.GetHeadPosition (); pos != NULL;)
	{
		CBCGPProp* pProp = m_lstProps.GetNext (pos);
		ASSERT_VALID (pProp);

		pProp->ExpandDeep (bExapand);
	}

	AdjustLayout ();

	if (GetSafeHwnd () != NULL)
	{
		RedrawWindow ();
	}
}
//******************************************************************************************
void CBCGPPropList::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	CWnd::OnChar(nChar, nRepCnt, nFlags);

	if (m_pSel == NULL || !m_pSel->m_bEnabled)
	{
		return;
	}

	ASSERT_VALID (m_pSel);

	if (!EditItem (m_pSel))
	{
		return;
	}

	m_pSel->PushChar (nChar);
}
//*******************************************************************************************
HBRUSH CBCGPPropList::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CWnd::OnCtlColor(pDC, pWnd, nCtlColor);
	
	if (m_pSel != NULL && 
		pWnd->GetSafeHwnd () == m_pSel->m_pWndInPlace->GetSafeHwnd ())
	{
		HBRUSH hbrProp = m_pSel->OnCtlColor (pDC, nCtlColor);
		if (hbrProp != NULL)
		{
			return hbrProp;
		}
	}

	return hbr;
}
//*******************************************************************************************
void CBCGPPropList::UpdateColor (COLORREF color)
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pSel);

	CBCGPColorProp* pColorProp = DYNAMIC_DOWNCAST(CBCGPColorProp, m_pSel);
	if (pColorProp == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	BOOL bChanged = color != pColorProp->GetColor ();
	pColorProp->SetColor (color);

	if (bChanged)
	{
		OnPropertyChanged (pColorProp);
	}

	if (color == (COLORREF)-1 && pColorProp->m_pWndInPlace != NULL && ::IsWindow(pColorProp->m_pWndInPlace->GetSafeHwnd()))
	{
		pColorProp->m_pWndInPlace->SetWindowText(_T(""));
	}

	pColorProp->OnUpdateValue ();
}
//****************************************************************************************
void CBCGPPropList::CloseColorPopup ()
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pSel);

	CBCGPColorProp* pColorProp = DYNAMIC_DOWNCAST(CBCGPColorProp, m_pSel);
	if (pColorProp == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	pColorProp->m_pPopup = NULL;

	pColorProp->m_bButtonIsDown = FALSE;
	pColorProp->Redraw ();

	if (pColorProp->m_pWndInPlace != NULL)
	{
		pColorProp->m_pWndInPlace->SetFocus ();
	}
}
//*******************************************************************************************
void CBCGPPropList::OnSelectCombo()
{
	ASSERT_VALID (this);
	
	if (m_pSel == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	ASSERT_VALID (m_pSel);
	m_pSel->OnSelectCombo ();
}
//****************************************************************************************
void CBCGPPropList::OnCloseCombo()
{
	ASSERT_VALID (this);
	
	if (m_pSel == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	ASSERT_VALID (m_pSel);
	m_pSel->OnCloseCombo ();
}
//****************************************************************************************
void CBCGPPropList::OnEditKillFocus()
{
	if (m_pSel != NULL && m_pSel->m_bInPlaceEdit != NULL && m_pSel->m_bEnabled)
	{
		ASSERT_VALID (m_pSel);

		if (!IsChild (GetFocus()) && m_pSel->OnEditKillFocus())
		{
			if (!EndEditItem ())
			{
				m_pSel->m_pWndInPlace->SetFocus();
			}
			else
			{
				OnKillFocus(GetFocus());
			}
		}
	}
}
//****************************************************************************************
void CBCGPPropList::OnComboKillFocus()
{
	if (m_pSel != NULL && m_pSel->m_pWndCombo != NULL && m_pSel->m_bEnabled)
	{
		ASSERT_VALID (m_pSel);

		if (!IsChild (GetFocus()))
		{
			if (!EndEditItem ())
			{
				m_pSel->m_pWndCombo->SetFocus();
			}
			else
			{
				OnKillFocus(GetFocus());
			}
		}
	}
}
//****************************************************************************************
void CBCGPPropList::SetBoolLabels (LPCTSTR lpszTrue, LPCTSTR lpszFalse)
{
	ASSERT_VALID (this);
	ASSERT (lpszTrue != NULL);
	ASSERT (lpszFalse != NULL);

	m_strTrue = lpszTrue;
	m_strFalse = lpszFalse;

	if (GetSafeHwnd () != NULL)
	{
		RedrawWindow ();
	}
}
//****************************************************************************************
void CBCGPPropList::SetListDelimiter (TCHAR c)
{
	ASSERT_VALID (this);
	ASSERT (GetSafeHwnd () == NULL);	// Should be called before window create

	m_cListDelimeter = c;
}
//******************************************************************************
void CBCGPPropList::SetDescriptionRows (int nDescRows)
{
	ASSERT_VALID (this);

	m_nDescrRows = nDescRows;

	if (GetSafeHwnd () != NULL)
	{
		AdjustLayout ();
		RedrawWindow ();
	}
}
//******************************************************************************
void CBCGPPropList::SetAlphabeticMode (BOOL bSet)
{
	ASSERT_VALID (this);

	if (m_bAlphabeticMode == bSet)
	{
		return;
	}

	m_bAlphabeticMode = bSet;
	m_nVertScrollOffset = 0;

	SetCurSel (NULL);

	if (GetSafeHwnd () != NULL)
	{
		if (m_bAlphabeticMode)
		{
			ReposProperties ();
		}

		AdjustLayout ();
		RedrawWindow ();
	}

	if (m_wndToolBar.IsWindowVisible())
	{
		m_wndToolBar.OnUpdateCmdUI ((CFrameWnd*) this, TRUE);
	}
}
//*****************************************************************************************
void CBCGPPropList::SetVSDotNetLook (BOOL bSet)
{
	ASSERT_VALID (this);
	m_bVSDotNetLook = bSet;

	if (GetSafeHwnd () != NULL)
	{
		RedrawWindow ();
	}
}
//****************************************************************************************
BOOL CBCGPPropList::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
	BOOL bRes = CWnd::OnNotify(wParam, lParam, pResult);

	NMHDR* pNMHDR = (NMHDR*)lParam;
	ASSERT (pNMHDR != NULL);

	if (pNMHDR->code == TTN_SHOW)
	{
		m_ToolTip.SetWindowPos (&wndTop, -1, -1, -1, -1,
			SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOSIZE);
	}

	return bRes;
}
//****************************************************************************************
void CBCGPPropList::OnDestroy() 
{
	while (!m_lstProps.IsEmpty ())
	{
		delete m_lstProps.RemoveHead ();
	}

	m_pSel = NULL;
	m_pTracked = NULL;

	m_IPToolTip.DestroyWindow ();
	m_ToolTip.DestroyWindow ();

	CWnd::OnDestroy();
}
//*****************************************************************************************
BOOL CBCGPPropList::OnNeedTipText(UINT /*id*/, NMHDR* pNMH, LRESULT* /*pResult*/)
{
	static CString strTipText;

	if (m_ToolTip.GetSafeHwnd () == NULL || pNMH->hwndFrom != m_ToolTip.GetSafeHwnd())
	{
		return FALSE;
	}

	CPoint point;
	::GetCursorPos (&point);
	ScreenToClient (&point);

	CBCGPProp* pProp = HitTest (point);
	if (pProp == NULL)
	{
		return FALSE;
	}

	if (point.x < m_rectList.left + m_nLeftColumnWidth)
	{
		if (pProp->m_bHasState && pProp->m_rectState.PtInRect(point) && !pProp->m_strStateToolTip.IsEmpty())
		{
			strTipText = pProp->m_strStateToolTip;
		}
		else if (!pProp->m_bNameIsTrancated || m_bAlwaysShowUserTT)
		{
			// User-defined tooltip
			strTipText = pProp->GetNameTooltip ();
		}
	}
	else
	{
		if (!pProp->m_bValueIsTrancated || m_bAlwaysShowUserTT)
		{
			// User-defined tooltip
			strTipText = pProp->GetValueTooltip ();
		}
	}

	if (strTipText.IsEmpty ())
	{
		return FALSE;
	}

	LPNMTTDISPINFO	pTTDispInfo	= (LPNMTTDISPINFO) pNMH;
	ASSERT((pTTDispInfo->uFlags & TTF_IDISHWND) == 0);

	pTTDispInfo->lpszText = const_cast<LPTSTR> ((LPCTSTR) strTipText);

	m_ToolTip.SetFont (GetFont (), FALSE);
	return TRUE;
}
//****************************************************************************************
void CBCGPPropList::OnMouseMove(UINT nFlags, CPoint point) 
{
	CWnd::OnMouseMove(nFlags, point);

	if (m_bTracking)
	{
		TrackHeader (point.x);
		return;
	}

	if (m_bTrackingDescr)
	{
		TrackDescr (point.y);
		return;
	}

	if (m_bTrackingCommands)
	{
		TrackCommands (point.y);
		return;
	}

	BOOL bNeedToTack = FALSE;

	if (m_bContextMenu)
	{
		CBCGPProp* pProp = HitTest(point);

		if (pProp != NULL)
		{
			BOOL bWasHighlighted = pProp->m_bButtonIsHighlighted;

			pProp->m_bButtonIsHighlighted = pProp->m_rectButton.PtInRect(point);
			if (pProp->m_bButtonIsHighlighted != bWasHighlighted)
			{
				pProp->RedrawButton();
			}

			bWasHighlighted = pProp->m_bMenuButtonIsHighlighted;

			pProp->m_bMenuButtonIsHighlighted = pProp->m_rectMenuButton.PtInRect(point);
			if (pProp->m_bMenuButtonIsHighlighted != bWasHighlighted)
			{
				pProp->RedrawMenuButton();
			}

			bNeedToTack = pProp->m_bButtonIsHighlighted || pProp->m_bMenuButtonIsHighlighted;
		}

		if (m_pTracked != NULL && m_pTracked != pProp)
		{
			ASSERT_VALID(m_pTracked);
			m_pTracked->OnLeaveMouse();
		}

		m_pTracked = pProp;
	}
	else
	{
		if (m_pSel != NULL && !m_pSel->m_rectButton.IsRectEmpty())
		{
			BOOL bWasHighlighted = m_pSel->m_bButtonIsHighlighted;

			m_pSel->m_bButtonIsHighlighted = m_pSel->m_rectButton.PtInRect(point);
			if (m_pSel->m_bButtonIsHighlighted != bWasHighlighted)
			{
				m_pSel->RedrawButton();
			}

			bNeedToTack = m_pSel->m_bButtonIsHighlighted;
		}
	}

	if (!m_bTrackButtons && bNeedToTack)
	{
		m_bTrackButtons = TRUE;
		
		TRACKMOUSEEVENT trackmouseevent;
		trackmouseevent.cbSize = sizeof(trackmouseevent);
		trackmouseevent.dwFlags = TME_LEAVE;
		trackmouseevent.hwndTrack = GetSafeHwnd();
		trackmouseevent.dwHoverTime = HOVER_DEFAULT;
		::BCGPTrackMouse (&trackmouseevent);	
	}
}
//*****************************************************************************************
LRESULT CBCGPPropList::OnMouseLeave(WPARAM,LPARAM)
{
	m_bTrackButtons = FALSE;

	if (m_pTracked != NULL)
	{
		ASSERT_VALID(m_pTracked);
		m_pTracked->OnLeaveMouse();

		m_pTracked = NULL;
	}

	if (m_pSel != NULL && m_pSel->m_bButtonIsHighlighted)
	{
		m_pSel->m_bButtonIsHighlighted = FALSE;
		m_pSel->RedrawButton();
	}

	return 0;
}
//*****************************************************************************************
void CBCGPPropList::OnLButtonUp(UINT nFlags, CPoint point) 
{
	CWnd::OnLButtonUp(nFlags, point);

	if (m_bTracking)
	{
		TrackHeader (-1);
		m_bTracking = FALSE;

		if (::GetCapture () == GetSafeHwnd ())
		{
			ReleaseCapture ();
		}

		CRect rectClient;
		GetClientRect (rectClient);

		m_nLeftColumnWidth = min (max (m_nRowHeight, point.x), rectClient.Width () - ::GetSystemMetrics (SM_CXHSCROLL) - 5);

		HDITEM hdItem;
		hdItem.mask = HDI_WIDTH ;
		hdItem.cxy = m_nLeftColumnWidth + 2;

		GetHeaderCtrl ().SetItem (0, &hdItem);

		hdItem.cxy = rectClient.Width () + 10;
		GetHeaderCtrl ().SetItem (1, &hdItem);
		return;
	}

	if (m_bTrackingDescr)
	{
		TrackDescr (INT_MIN);
		m_bTrackingDescr = FALSE;

		if (::GetCapture () == GetSafeHwnd ())
		{
			ReleaseCapture ();
		}

		CRect rectClient;
		GetClientRect (rectClient);

		point.y = max (point.y, m_nRowHeight + m_nToolbarHeight + m_nHeaderHeight);
		m_nDescrHeight = rectClient.Height () - point.y + 2;
		m_nDescrHeight = max (m_nRowHeight, m_nDescrHeight);

		AdjustLayout ();
		RedrawWindow ();
		return;
	}

	if (m_bTrackingCommands)
	{
		TrackCommands (INT_MIN);
		m_bTrackingCommands = FALSE;

		if (::GetCapture () == GetSafeHwnd ())
		{
			ReleaseCapture ();
		}

		CRect rectClient;
		GetClientRect (rectClient);

		point.y = max (point.y, m_nRowHeight + m_nToolbarHeight + m_nHeaderHeight);
		m_nCommandsHeight = rectClient.Height () - point.y + 2 - m_nDescrHeight;
		m_nCommandsHeight = max (m_nRowHeight, m_nCommandsHeight);

		AdjustLayout ();
		RedrawWindow ();
		return;
	}

	for (int i = 0; i < m_arCommandRects.GetSize (); i++)
	{
		if (m_arCommandRects [i].PtInRect (point))
		{
			GetOwner ()->SendMessage (BCGM_PROPERTY_COMMAND_CLICKED, GetDlgCtrlID (), LPARAM (i));
			return;
		}
	}
}
//****************************************************************************************
void CBCGPPropList::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp) 
{
	CWnd::OnNcCalcSize(bCalcValidRects, lpncsp);

	if (GetStyle() & WS_BORDER) 
	{
		lpncsp->rgrc[0].left++; 
		lpncsp->rgrc[0].top++ ;
		lpncsp->rgrc[0].right--;
		lpncsp->rgrc[0].bottom--;
	}
}
//****************************************************************************************
void CBCGPPropList::OnNcPaint() 
{
	BOOL bAdjustLayout = FALSE;

	if (m_nDescrHeight == -1)
	{
		m_nDescrHeight = m_nRowHeight * m_nDescrRows + m_nRowHeight / 2;
		bAdjustLayout = TRUE;
	}

	if (m_nCommandsHeight == -1 && !m_lstCommands.IsEmpty () && m_bAreCommandsVisible)
	{
		m_nCommandsHeight = m_nRowHeight * m_nCommandRows + m_nRowHeight / 2;
		bAdjustLayout = TRUE;
	}

	if (bAdjustLayout)
	{
		AdjustLayout ();
	}

	if (GetStyle () & WS_BORDER)
	{
		if (DrawControlBarColors ())
		{
			visualManager->OnDrawControlBorder (this);
		}
		else
		{
			visualManager->OnDrawControlBorderNoTheme (this);
		}
	}
}
//********************************************************************************
void CBCGPPropList::SetGroupNameFullWidth (BOOL bGroupNameFullWidth,
										   BOOL bRedraw/* = TRUE*/)
{
	m_bGroupNameFullWidth = bGroupNameFullWidth;

	if (bRedraw && GetSafeHwnd () != NULL)
	{
		RedrawWindow ();
	}
}
//********************************************************************************
void CBCGPPropList::SetCustomColors (
		COLORREF	clrBackground,
		COLORREF	clrText,
		COLORREF	clrGroupBackground,
		COLORREF	clrGroupText,
		COLORREF	clrDescriptionBackground,
		COLORREF	clrDescriptionText,
		COLORREF	clrLine)
{
	m_clrBackground = clrBackground;
	m_clrText = clrText;
	m_clrGroupBackground = clrGroupBackground;
	m_clrGroupText = clrGroupText;
	m_clrDescriptionBackground = clrDescriptionBackground;
	m_clrDescriptionText = clrDescriptionText;
	m_clrLine = clrLine;

	m_brBackground.DeleteObject ();

	if (m_clrBackground != (COLORREF)-1)
	{
		m_brBackground.CreateSolidBrush (m_clrBackground);
	}
	else if (m_clrFillDefault != (COLORREF)-1)
	{
		m_brBackground.CreateSolidBrush (m_clrFillDefault);
	}
}
//********************************************************************************
void CBCGPPropList::GetCustomColors (
		COLORREF&	clrBackground,
		COLORREF&	clrText,
		COLORREF&	clrGroupBackground,
		COLORREF&	clrGroupText,
		COLORREF&	clrDescriptionBackground,
		COLORREF&	clrDescriptionText,
		COLORREF&	clrLine)
{
	clrBackground = m_clrBackground;
	clrText = m_clrText;
	clrGroupBackground = m_clrGroupBackground;
	clrGroupText = m_clrGroupText;
	clrDescriptionBackground = m_clrDescriptionBackground;
	clrDescriptionText = m_clrDescriptionText;
	clrLine = m_clrLine;
}
//*********************************************************************************
BOOL CBCGPPropList::ProcessClipboardAccelerators (UINT nChar)
{
	if (m_pSel == NULL || m_pSel->m_pWndInPlace->GetSafeHwnd () == NULL)
	{
		return FALSE;
	}

	ASSERT_VALID (m_pSel);

	BOOL bIsCtrl = (::GetAsyncKeyState (VK_CONTROL) & 0x8000);
	BOOL bIsShift = (::GetAsyncKeyState (VK_SHIFT) & 0x8000);

	if (bIsCtrl && (nChar == _T('C') || nChar == VK_INSERT))
	{
		m_pSel->m_pWndInPlace->SendMessage (WM_COPY);
		return TRUE;
	}

	if (bIsCtrl && nChar == _T('V') || (bIsShift && nChar == VK_INSERT))
	{
		m_pSel->m_pWndInPlace->SendMessage (WM_PASTE);
		return TRUE;
	}

	if (bIsCtrl && nChar == _T('X') || (bIsShift && nChar == VK_DELETE))
	{
		m_pSel->m_pWndInPlace->SendMessage (WM_CUT);
		return TRUE;
	}

	return FALSE;
}
//************************************************************************************
int CBCGPPropList::CompareProps (const CBCGPProp* pProp1, const CBCGPProp* pProp2) const
{ 
	ASSERT_VALID (this);
	ASSERT_VALID (pProp1);
	ASSERT_VALID (pProp2);

	return pProp1->m_strName.Compare (pProp2->m_strName);
}
//************************************************************************************
void CBCGPPropList::NotifyAccessibility (CBCGPProp* pProp)
{
	if (!globalData.IsAccessibilitySupport () || pProp == NULL)
	{
		return;
	}

	m_pAccProp = pProp;

	CPoint pt(pProp->m_Rect.left, pProp->m_Rect.top);
	ClientToScreen(&pt);
	LPARAM lParam = MAKELPARAM(pt.x, pt.y);

	globalData.NotifyWinEvent (EVENT_OBJECT_FOCUS, GetSafeHwnd(), OBJID_CLIENT, (LONG)lParam);
}
//********************************************************************************
void CBCGPPropList::MarkModifiedProperties (BOOL bMark/* = TRUE*/,
											BOOL bRedraw/* = TRUE*/)
{
	m_bMarkModifiedProperties = bMark;

	if (bRedraw && GetSafeHwnd () != NULL)
	{
		RedrawWindow ();
	}
}
//********************************************************************************
void CBCGPPropList::ResetOriginalValues (BOOL bRedraw)
{
	for (POSITION pos = m_lstProps.GetHeadPosition (); pos != NULL;)
	{
		CBCGPProp* pProp = m_lstProps.GetNext (pos);
		ASSERT_VALID (pProp);

		pProp->ResetOriginalValue ();
	}

	if (bRedraw && GetSafeHwnd () != NULL)
	{
		RedrawWindow ();
	}
}
//********************************************************************************
void CBCGPPropList::CommitModifiedValues (BOOL bRedraw)
{
	for (POSITION pos = m_lstProps.GetHeadPosition (); pos != NULL;)
	{
		CBCGPProp* pProp = m_lstProps.GetNext (pos);
		ASSERT_VALID (pProp);

		pProp->CommitModifiedValue ();
	}

	if (bRedraw && GetSafeHwnd () != NULL)
	{
		RedrawWindow ();
	}
}
//********************************************************************************
void CBCGPPropList::SetCustomMenuItems(const CStringList& lstMenuItemNames)
{
	ASSERT_VALID (this);

	m_lstCustomMenuItems.RemoveAll ();
	m_lstCustomMenuItems.AddTail ((CStringList*)&lstMenuItemNames);
}
//********************************************************************************
BOOL CBCGPPropList::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	if (lParam != 0 && m_pSel != NULL)
	{
		ASSERT_VALID (m_pSel);

		CWnd* pWnd = CWnd::FromHandlePermanent ((HWND)lParam);
		if (pWnd != NULL && m_pSel->m_pWndInPlace == pWnd)
		{
			if (m_pSel->OnCommand (wParam))
			{
				return TRUE;
			}
		}
	}
	
	return CWnd::OnCommand(wParam, lParam);
}
//****************************************************************************
BOOL CBCGPPropList::OnSetAccData (long lVal)
{
	ASSERT_VALID (this);

	if (lVal == 0)
	{
		return FALSE;
	}

	CPoint pt(BCG_GET_X_LPARAM(lVal), BCG_GET_Y_LPARAM(lVal));
	ScreenToClient (&pt);

	CBCGPProp* pProp = HitTest (pt);
	if (pProp == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	m_AccData.Clear ();

	ASSERT_VALID (pProp);

	pProp->SetACCData (this, m_AccData);

	return TRUE;
}
//****************************************************************************
LRESULT CBCGPPropList::OnGetObject(WPARAM wParam, LPARAM lParam)
{
	return CBCGPWnd::OnGetObject (wParam, lParam);
}
//****************************************************************************
HRESULT CBCGPPropList::get_accChildCount(long *pcountChildren)
{
	if (!pcountChildren)
	{
		return E_INVALIDARG;
	}

	*pcountChildren = 0;
	return S_OK;
}
//****************************************************************************
HRESULT CBCGPPropList::get_accChild(VARIANT /*varChild*/, IDispatch **ppdispChild)
{
	if (!(*ppdispChild))
	{
		return E_INVALIDARG;
	}

	if (m_pStdObject != NULL)
	{
		*ppdispChild = m_pStdObject;
	}
	else
	{
		*ppdispChild = NULL;
	}
	return S_OK;
}
//****************************************************************************
HRESULT CBCGPPropList::get_accName(VARIANT varChild, BSTR *pszName)
{
	if ((varChild.vt == VT_I4) && (varChild.lVal == CHILDID_SELF))
	{
		CString strText;
		GetWindowText(strText);
		if (strText.GetLength() == 0)
		{
			*pszName  = SysAllocString(L"PropertyList");
			return S_OK;
		}

		*pszName = strText.AllocSysString();
		return S_OK;
	}

	if (m_pAccProp != NULL)
	{
		if (m_pAccProp->IsInPlaceEditing ())
		{
			CString strValue = m_pAccProp->FormatProperty();
			*pszName = strValue.AllocSysString();
			return S_OK;
		}
		CString strName = m_pAccProp->GetName();
		*pszName = strName.AllocSysString();
		return S_OK;
	}

	return S_OK;
}
//****************************************************************************
HRESULT CBCGPPropList::get_accValue(VARIANT varChild, BSTR *pszValue)
{
	if ((varChild.vt == VT_I4) && (varChild.lVal == CHILDID_SELF))
	{
		return S_FALSE;
	}

	if (m_pAccProp != NULL)
	{
		BOOL bGroup = (m_pAccProp->IsGroup() && !m_pAccProp->m_bIsValueList);
		if (!bGroup)
		{
			CString strValue = m_pAccProp->FormatProperty();
			*pszValue = strValue.AllocSysString();
			return S_OK;
		}
	}

	return S_FALSE;
}
//****************************************************************************
HRESULT CBCGPPropList::get_accDescription(VARIANT varChild, BSTR *pszDescription)
{
	if (((varChild.vt != VT_I4) && (varChild.lVal != CHILDID_SELF)) || (NULL == pszDescription))
	{
		return E_INVALIDARG;
	}

	if ((varChild.vt == VT_I4) && (varChild.lVal == CHILDID_SELF))
	{
		*pszDescription = SysAllocString(L"PropertyList");
		return S_OK;
	}

	if (m_pAccProp != NULL)
	{
		CString strName = m_pAccProp->GetName();
		*pszDescription = strName.AllocSysString();
	}

	return S_OK;
}
//****************************************************************************
HRESULT CBCGPPropList::get_accRole(VARIANT varChild, VARIANT *pvarRole)
{
	if (!pvarRole || ((varChild.vt != VT_I4) && (varChild.lVal != CHILDID_SELF)))
	{
		return E_INVALIDARG;
	}

	if ((varChild.vt == VT_I4) && (varChild.lVal == CHILDID_SELF))
	{
		pvarRole->vt = VT_I4;
		pvarRole->lVal = ROLE_SYSTEM_LIST;
		return S_OK;
	}

	pvarRole->vt = VT_I4;
	pvarRole->lVal = ROLE_SYSTEM_ROW;

	return S_OK;
}
//****************************************************************************
HRESULT CBCGPPropList::get_accState(VARIANT varChild, VARIANT *pvarState)
{
	if ((varChild.vt == VT_I4) && (varChild.lVal == CHILDID_SELF))
	{
		pvarState->vt = VT_I4;
		pvarState->lVal = STATE_SYSTEM_NORMAL;
		return S_OK;
	}

	pvarState->vt = VT_I4;
	pvarState->lVal = STATE_SYSTEM_FOCUSABLE;
	pvarState->lVal |= STATE_SYSTEM_SELECTABLE;
	
	if (m_pAccProp != NULL)
	{
		if (m_pAccProp->IsSelected())
		{
			pvarState->lVal |= STATE_SYSTEM_FOCUSED;
			pvarState->lVal |= STATE_SYSTEM_SELECTED;
		}

		BOOL bGroup = (m_pAccProp->IsGroup() && !m_pAccProp->m_bIsValueList);
		if (!m_pAccProp->IsEnabled() || bGroup)
		{
			pvarState->lVal |= STATE_SYSTEM_READONLY;
		}
	}

	return S_OK;
}
//****************************************************************************
HRESULT CBCGPPropList::get_accHelp(VARIANT /*varChild*/, BSTR * /*pszHelp*/)
{
	return S_FALSE;
}
//****************************************************************************
HRESULT CBCGPPropList::get_accHelpTopic(BSTR * /*pszHelpFile*/, VARIANT /*varChild*/, long * /*pidTopic*/)
{
	return S_FALSE;
}
//****************************************************************************
HRESULT CBCGPPropList::get_accKeyboardShortcut(VARIANT /*varChild*/, BSTR* /*pszKeyboardShortcut*/)
{
	return S_FALSE;
}
//****************************************************************************
HRESULT CBCGPPropList::get_accFocus(VARIANT *pvarChild)
{
	if (NULL == pvarChild)
	{
		return E_INVALIDARG;
	}

	return DISP_E_MEMBERNOTFOUND;
}
//****************************************************************************
HRESULT CBCGPPropList::get_accSelection(VARIANT *pvarChildren)
{
	if (NULL == pvarChildren)
	{
		return E_INVALIDARG;
	}
	return DISP_E_MEMBERNOTFOUND;
}
//****************************************************************************
HRESULT CBCGPPropList::get_accDefaultAction(VARIANT /*varChild*/, BSTR* /*pszDefaultAction*/)
{
	return DISP_E_MEMBERNOTFOUND; 
}
//****************************************************************************
HRESULT CBCGPPropList::accSelect(long flagsSelect, VARIANT varChild)
{
	if (m_pStdObject != NULL)
	{
		return m_pStdObject->accSelect(flagsSelect, varChild);
	}
	return E_INVALIDARG;
}
//****************************************************************************
HRESULT CBCGPPropList::accLocation(long *pxLeft, long *pyTop, long *pcxWidth, long *pcyHeight, VARIANT varChild)
{
	HRESULT hr = S_OK;

	if (!pxLeft || !pyTop || !pcxWidth || !pcyHeight)
	{
		return E_INVALIDARG;
	}

	if ((varChild.vt == VT_I4) && (varChild.lVal == CHILDID_SELF))
	{
		CRect rc;
		GetWindowRect(rc);

		*pxLeft = rc.left;
		*pyTop = rc.top;
		*pcxWidth = rc.Width();
		*pcyHeight = rc.Height();

		return S_OK;
	}
	else
	{
		if (m_pAccProp != NULL)
		{
			CRect rcProp = m_pAccProp->m_Rect;
			ClientToScreen(&rcProp);
			*pxLeft = rcProp.left;
			*pyTop = rcProp.top;
			*pcxWidth = rcProp.Width();
			*pcyHeight = rcProp.Height();
		}
	}

	return hr;
}
//****************************************************************************
HRESULT CBCGPPropList::accHitTest(long  xLeft, long yTop, VARIANT *pvarChild)
{
	if (!pvarChild)
	{
		return E_INVALIDARG;
	}

	CPoint pt(xLeft, yTop);
	ScreenToClient(&pt);

	CBCGPProp* pProp = HitTest(pt);
	if (pProp != NULL)
	{
		LPARAM lParam = MAKELPARAM((WORD)xLeft, (WORD)yTop);
		pvarChild->vt = VT_I4;
		pvarChild->lVal = (LONG)lParam;
	}
	else
	{
		pvarChild->vt = VT_I4;
		pvarChild->lVal = CHILDID_SELF;
	}

	m_pAccProp = pProp;
	return S_OK;
}
//****************************************************************************
LRESULT CBCGPPropList::OnPrintClient(WPARAM wp, LPARAM lp)
{
	if ((lp & PRF_CLIENT) == PRF_CLIENT)
	{
		CDC* pDC = CDC::FromHandle((HDC) wp);
		ASSERT_VALID(pDC);

		OnDraw(pDC);
		return 0;
	}

	return Default();
}
//****************************************************************************
LRESULT CBCGPPropList::OnPrint(WPARAM wp, LPARAM lp)
{
	if ((lp & PRF_NONCLIENT) == PRF_NONCLIENT)
	{
		CDC* pDC = CDC::FromHandle((HDC) wp);
		ASSERT_VALID(pDC);
		
		if (GetStyle () & WS_BORDER)
		{
			CRect rect;
			GetWindowRect(rect);
			
			rect.bottom -= rect.top;
			rect.right -= rect.left;
			rect.left = rect.top = 0;

			if (DrawControlBarColors ())
			{
				visualManager->OnDrawControlBorder(pDC, rect, this, m_bOnGlass);
			}
			else
			{
				visualManager->OnDrawControlBorderNoTheme(pDC, rect, this, m_bOnGlass);
			}
		}
	}

	return Default();
}
//****************************************************************************
BOOL CBCGPPropList::OnDrawStateIndicator(CDC* pDC, CBCGPProp* pProp, CRect rect)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	COLORREF clrTextOld = pDC->SetTextColor(pProp->m_clrState);
	CFont* pOldFont = pDC->SelectObject(&m_fontBold);

	const CString str(pProp->m_cStateIndicator);

	pDC->DrawText(str, rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

	pDC->SelectObject(pOldFont);
	pDC->SetTextColor(clrTextOld);

	return TRUE;
}
//****************************************************************************
BOOL CBCGPPropList::OnDrawMenuButton(CDC* pDC, CBCGPProp* /*pProp*/, CRect rect, BOOL bIsHighlighted, BOOL bIsModified)
{
	if (!m_AdvImages.IsValid())
	{
		CBCGPLocalResource locaRes;

		const CSize sizeImage(11, 11);

		m_AdvImages.SetImageSize (sizeImage);
		m_AdvImages.SetTransparentColor (globalData.clrBtnFace);
		
		m_AdvImages.Load (IDB_BCGBARRES_PROPLIST_ADVANCED);

		if (!m_AdvImages.IsValid())
		{
			return FALSE;
		}
	}

	int nIndex = 0;

	if (bIsModified)
	{
		nIndex = bIsHighlighted ? 3 : 2;
	}
	else
	{
		nIndex = bIsHighlighted ? 1 : 0;
	}

	m_AdvImages.DrawEx(pDC, rect, nIndex,
		CBCGPToolBarImages::ImageAlignHorzCenter,
		CBCGPToolBarImages::ImageAlignVertCenter);


	return TRUE;
}
//****************************************************************************
void CBCGPPropList::OnContextMenu(CWnd* /*pWnd*/, CPoint point) 
{
	if (m_bContextMenu && point == CPoint(-1, -1) && m_pSel != NULL)
	{
		ASSERT_VALID(m_pSel);

		if (!m_pSel->IsGroup () || m_pSel->m_bIsValueList)
		{
			m_pSel->OnClickMenuButton(CPoint(m_rectList.left, m_pSel->m_Rect.bottom + 1));
		}

		return;
	}

	Default();
}
//****************************************************************************
void CBCGPPropList::OnToolbarCategorized()
{
	SetAlphabeticMode(FALSE);
	GetOwner ()->SendMessage(BCGM_PROPERTYLIST_SORTING_MODE_CHANGED, (WPARAM)m_bAlphabeticMode);
}
//****************************************************************************
void CBCGPPropList::OnUpdateToolbarCategorized(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(!m_bAlphabeticMode);
}
//****************************************************************************
void CBCGPPropList::OnToolbarAlphabetical()
{
	SetAlphabeticMode();
	GetOwner ()->SendMessage(BCGM_PROPERTYLIST_SORTING_MODE_CHANGED, (WPARAM)m_bAlphabeticMode);
}
//****************************************************************************
void CBCGPPropList::OnUpdateToolbarAlphabetical(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_bAlphabeticMode);
}
//****************************************************************************
void CBCGPPropList::OnFilter()
{
	CString strFilter;
	m_wndFilter.GetWindowText(strFilter);

	SetPropertiesFilter(strFilter);
}
//****************************************************************************
void CBCGPPropList::SetPropertiesFilter(LPCTSTR lpszFilter)
{
	m_strFilter = lpszFilter == NULL ? _T("") : lpszFilter;
	m_strFilter.MakeUpper();

	for (POSITION pos = m_lstProps.GetHeadPosition (); pos != NULL;)
	{
		CBCGPProp* pProp = m_lstProps.GetNext (pos);
		ASSERT_VALID (pProp);

		pProp->SetFilter(m_strFilter);
	}

	if (m_pSel != NULL && !m_pSel->m_bInFilter)
	{
		m_pSel = NULL;
	}

	AdjustLayout();
}
//****************************************************************************
BOOL CBCGPPropList::IsPropertyMatchedToFilter(CBCGPProp* pProp, const CString& strFilter) const
{
	ASSERT_VALID(pProp);

	if (strFilter.IsEmpty())
	{
		return TRUE;
	}
	
	CString strName = pProp->GetName();
	strName.MakeUpper();
	
	return strName.Find(strFilter) >= 0;
}
//****************************************************************************
CBCGPProp* CBCGPPropList::GetFirstNonFilteredProperty()
{
	CList<CBCGPProp*, CBCGPProp*>& list = m_bAlphabeticMode ? m_lstTerminalProps : m_lstProps;

	for (POSITION pos = list.GetHeadPosition(); pos != NULL;)
	{
		CBCGPProp* pProp = list.GetNext(pos);
		ASSERT_VALID(pProp);

		if (pProp->IsVisibleInFilter())
		{
			return pProp;
		}
	}

	return NULL;
}
//****************************************************************************
CBCGPProp* CBCGPPropList::GetLastNonFilteredProperty()
{
	if (m_bAlphabeticMode)
	{
		for (POSITION pos = m_lstTerminalProps.GetTailPosition(); pos != NULL;)
		{
			CBCGPProp* pProp = m_lstTerminalProps.GetPrev(pos);
			ASSERT_VALID(pProp);

			if (pProp->IsVisibleInFilter())
			{
				return pProp;
			}
		}

		return NULL;
	}

	for (POSITION pos = m_lstProps.GetTailPosition(); pos != NULL;)
	{
		CBCGPProp* pProp = m_lstProps.GetPrev(pos);
		ASSERT_VALID(pProp);

		if (pProp->IsVisibleInFilter())
		{
			if (m_strFilter.IsEmpty())
			{
				while (!pProp->m_lstSubItems.IsEmpty () && pProp->IsExpanded ())
				{
					pProp = pProp->m_lstSubItems.GetTail ();
				}
			}

			return pProp;
		}
	}

	return NULL;
}
//****************************************************************************
BOOL CBCGPPropList::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo) 
{
	if (m_bToolbarCustomButtons && m_wndToolBar.GetSafeHwnd() != NULL && m_wndToolBar.CommandToIndex(nID) >= m_nToolbarDefaultButtons)
	{
		return GetParent()->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
	}
	
	return CWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}
//****************************************************************************
void CBCGPPropList::SetNameAlign(int nAlign, BOOL bRedraw)
{
	m_nNameAlign = nAlign;

	if (GetHeaderCtrl().GetSafeHwnd() != NULL)
	{
		HDITEM hdItem;
		hdItem.mask = HDI_FORMAT;
		hdItem.fmt = (m_nNameAlign == DT_LEFT) ? HDF_LEFT : (m_nNameAlign == DT_CENTER) ? HDF_CENTER : HDF_RIGHT;
		hdItem.fmt |= HDF_STRING;
		
		GetHeaderCtrl().SetItem(0, &hdItem);
	}

	if (bRedraw && GetSafeHwnd () != NULL)
	{
		RedrawWindow ();
	}
}

#endif // BCGP_EXCLUDE_PROP_LIST

