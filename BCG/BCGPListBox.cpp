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
// BCGPListBox.cpp : implementation file
//

#include "stdafx.h"
#include "BCGPListBox.h"
#include "BCGPDlgImpl.h"
#include "BCGPVisualManager.h"
#include "BCGPPropertySheet.h"
#include "BCGPStatic.h"
#include "TrackMouse.h"
#include "BCGPLocalResource.h"
#include "bcgprores.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

struct BCGP_LB_ITEM_DATA
{
public:
	int			m_nCheck;
	DWORD_PTR	m_dwData;
	CString		m_strDescription;
	BOOL		m_bIsPinned;
	BOOL		m_bIsEnabled;
	int			m_nImageIndex;
	
	BCGP_LB_ITEM_DATA()
		: m_nCheck(BST_UNCHECKED)
		, m_dwData(0)
		, m_bIsPinned(FALSE)
		, m_bIsEnabled(TRUE)
		, m_nImageIndex(-1)
	{
	}
};

IMPLEMENT_DYNAMIC(CBCGPListBox, CListBox)

#ifndef _BCGSUITE_
#define PIN_AREA_WIDTH	(CBCGPVisualManager::GetInstance ()->GetPinSize(TRUE).cx + 10)
#else
#define PIN_AREA_WIDTH	(CMenuImages::Size().cx + 10)
#endif

#define BCGP_DEFAULT_TABS_STOP	32

UINT BCGM_ON_CLICK_LISTBOX_PIN = ::RegisterWindowMessage (_T("BCGM_ON_CLICK_LISTBOX_PIN"));

/////////////////////////////////////////////////////////////////////////////
// CBCGPListBox

CBCGPListBox::CBCGPListBox()
{
	m_bVisualManagerStyle = FALSE;
	m_bOnGlass = FALSE;
	m_nHighlightedItem = -1;
	m_bItemHighlighting = TRUE;
	m_bTracked = FALSE;
	m_hImageList = NULL;
	m_sizeImage = CSize(0, 0);
	m_bBackstageMode = FALSE;
	m_bPropertySheetNavigator = FALSE;
	m_bPins = FALSE;
	m_bIsPinHighlighted = FALSE;
	m_bIsCheckHighlighted = FALSE;
	m_bHasCheckBoxes = FALSE;
	m_bHasDescriptions = FALSE;
	m_nDescrRows = 0;
	m_nClickedItem = -1;
	m_hFont	= NULL;
	m_nTextHeight = -1;
	m_bInAddingCaption = FALSE;

	m_arTabStops.Add(BCGP_DEFAULT_TABS_STOP);
}

CBCGPListBox::~CBCGPListBox()
{
}

BEGIN_MESSAGE_MAP(CBCGPListBox, CListBox)
	//{{AFX_MSG_MAP(CBCGPListBox)
	ON_WM_MOUSEMOVE()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_VSCROLL()
	ON_WM_LBUTTONDOWN()
	ON_WM_KEYDOWN()
	ON_WM_MOUSEWHEEL()
	ON_WM_LBUTTONUP()
	ON_WM_CANCELMODE()
	ON_WM_HSCROLL()
	ON_WM_NCPAINT()
	ON_WM_CREATE()
	ON_MESSAGE(LB_ADDSTRING, OnLBAddString)
	ON_MESSAGE(LB_GETITEMDATA, OnLBGetItemData)
	ON_MESSAGE(LB_INSERTSTRING, OnLBInsertString)
	ON_MESSAGE(LB_SETITEMDATA, OnLBSetItemData)
	//}}AFX_MSG_MAP
	ON_REGISTERED_MESSAGE(BCGM_ONSETCONTROLVMMODE, OnBCGSetControlVMMode)
	ON_REGISTERED_MESSAGE(BCGM_ONSETCONTROLAERO, OnBCGSetControlAero)
	ON_REGISTERED_MESSAGE(BCGM_ONSETCONTROLBACKSTAGEMODE, OnBCGSetControlBackStageMode)
	ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
	ON_CONTROL_REFLECT_EX(LBN_SELCHANGE, OnSelchange)
	ON_MESSAGE(WM_PRINTCLIENT, OnPrintClient)
	ON_MESSAGE(WM_SETFONT, OnSetFont)
	ON_MESSAGE(LB_SETTABSTOPS, OnLBSetTabstops)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGPListBox message handlers

const BCGP_LB_ITEM_DATA* CBCGPListBox::_GetItemData(int nItem) const
{
	const BCGP_LB_ITEM_DATA* pState = NULL;

	LRESULT lResult = ((CBCGPListBox*)this)->DefWindowProc(LB_GETITEMDATA, nItem, 0);
	if (lResult != LB_ERR)
	{
		pState = (const BCGP_LB_ITEM_DATA*)lResult;
	}

	return pState;
}
//**************************************************************************
BCGP_LB_ITEM_DATA* CBCGPListBox::_GetAllocItemData(int nItem)
{
	BCGP_LB_ITEM_DATA* pState = NULL;
	
	LRESULT lResult = DefWindowProc(LB_GETITEMDATA, nItem, 0);
	if (lResult != LB_ERR)
	{
		pState = (BCGP_LB_ITEM_DATA*)lResult;
		if (pState == NULL)
		{
			pState = new BCGP_LB_ITEM_DATA;
			VERIFY(DefWindowProc(LB_SETITEMDATA, nItem, (LPARAM)pState) != LB_ERR);
		}
	}
	
	return pState;
}
//**************************************************************************
LRESULT CBCGPListBox::OnBCGSetControlVMMode (WPARAM wp, LPARAM)
{
	m_bVisualManagerStyle = (BOOL) wp;
	return 0;
}
//**************************************************************************
LRESULT CBCGPListBox::OnBCGSetControlAero (WPARAM wp, LPARAM)
{
	m_bOnGlass = (BOOL) wp;
	return 0;
}
//**************************************************************************
int CBCGPListBox::HitTest(CPoint pt, BOOL* pbPin, BOOL* pbCheck)
{
	if (pbPin != NULL)
	{
		*pbPin = FALSE;
	}

	if (pbCheck != NULL)
	{
		*pbCheck = FALSE;
	}

	if ((GetStyle() & LBS_NOSEL) == LBS_NOSEL)
	{
		return -1;
	}

	for (int i = 0; i < GetCount (); i++)
	{
		CRect rectItem;
		GetItemRect (i, rectItem);

		if (rectItem.PtInRect (pt))
		{
			if (IsCaptionItem(i) || IsSeparatorItem(i))
			{
				return -1;
			}

			BOOL bIsEnabled = IsEnabled(i);
			
			if (pbPin != NULL && m_bPins && bIsEnabled)
			{
				*pbPin = (pt.x > rectItem.right - PIN_AREA_WIDTH);
			}

			if (pbCheck != NULL && m_bHasCheckBoxes && bIsEnabled)
			{
				*pbCheck = (pt.x < rectItem.left + rectItem.Height());
			}

			return i;
		}
	}

	return -1;
}
//**************************************************************************
void CBCGPListBox::OnMouseMove(UINT nFlags, CPoint point)
{
	CListBox::OnMouseMove (nFlags, point);

	if ((GetStyle() & LBS_NOSEL) == LBS_NOSEL)
	{
		return;
	}

	ASSERT (IsWindowEnabled ());

	BOOL bIsPinHighlighted = FALSE;
	BOOL bIsCheckHighlighted = FALSE;

	int nHighlightedItem = HitTest(point, &bIsPinHighlighted, &bIsCheckHighlighted);

	if (!m_bTracked)
	{
		m_bTracked = TRUE;
		
		TRACKMOUSEEVENT trackmouseevent;
		trackmouseevent.cbSize = sizeof(trackmouseevent);
		trackmouseevent.dwFlags = TME_LEAVE;
		trackmouseevent.hwndTrack = GetSafeHwnd();
		trackmouseevent.dwHoverTime = HOVER_DEFAULT;
		BCGPTrackMouse (&trackmouseevent);
	}

	if (nHighlightedItem != m_nHighlightedItem || m_bIsPinHighlighted != bIsPinHighlighted || m_bIsCheckHighlighted != bIsCheckHighlighted)
	{
		CRect rectItem;

		if (m_nHighlightedItem >= 0 && nHighlightedItem != m_nHighlightedItem)
		{
			GetItemRect (m_nHighlightedItem, rectItem);
			InvalidateRect (rectItem);
		}

		m_nHighlightedItem = nHighlightedItem;
		m_bIsPinHighlighted = bIsPinHighlighted;
		m_bIsCheckHighlighted = bIsCheckHighlighted;

		if (m_nHighlightedItem >= 0)
		{
			GetItemRect (m_nHighlightedItem, rectItem);
			InvalidateRect (rectItem);
		}

		UpdateWindow ();
	}
}
//***********************************************************************************************	
LRESULT CBCGPListBox::OnMouseLeave(WPARAM,LPARAM)
{
	m_bTracked = FALSE;
	m_bIsPinHighlighted = FALSE;
	m_bIsCheckHighlighted = FALSE;

	if (m_nHighlightedItem >= 0)
	{
		CRect rectItem;
		GetItemRect (m_nHighlightedItem, rectItem);

		m_nHighlightedItem = -1;

		RedrawWindow (rectItem);
	}

	return 0;
}
//***********************************************************************************************	
void CBCGPListBox::DrawItem(LPDRAWITEMSTRUCT /*lpDIS*/) 
{
}
//***********************************************************************************************	
void CBCGPListBox::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct) 
{
	lpMeasureItemStruct->itemHeight = GetItemMinHeight();

	if ((GetStyle() & LBS_OWNERDRAWVARIABLE) == LBS_OWNERDRAWVARIABLE)
	{
		POSITION pos = m_lstCaptionIndexes.Find(lpMeasureItemStruct->itemID);
		if (pos != NULL || m_bInAddingCaption)
		{
			const int nSeparatorHeight = 10;

			CString strText;
			GetText(lpMeasureItemStruct->itemID, strText);

			if (strText.IsEmpty())
			{
				lpMeasureItemStruct->itemHeight = nSeparatorHeight;
			}
			else
			{
				lpMeasureItemStruct->itemHeight += nSeparatorHeight;
			}
		}
	}
}
//***********************************************************************************************	
void CBCGPListBox::OnDrawItemContent(CDC* pDC, CRect rect, int nIndex)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	const BOOL bIsCaption = IsCaptionItem(nIndex);

	CFont* pOldFont = NULL;

	int nHorzMargin = 4;
	if (globalData.GetRibbonImageScale () != 1.)
	{
		nHorzMargin = (int)(0.5 + globalData.GetRibbonImageScale() * nHorzMargin);
	}

	const int nTextRowHeight = m_nTextHeight == -1 ? globalData.GetTextHeight() : m_nTextHeight;
	const int xStart = rect.left;

	if (!bIsCaption)
	{
		if (m_bHasCheckBoxes)
		{
			const CSize sizeCheckBox = CBCGPVisualManager::GetInstance ()->GetCheckRadioDefaultSize();

			CRect rectCheck = rect;
			rectCheck.right = rectCheck.left + sizeCheckBox.cx + nHorzMargin;

			if (m_nDescrRows > 0)
			{
				rectCheck.DeflateRect(0, nTextRowHeight / 2);
				rectCheck.bottom = rectCheck.top + sizeCheckBox.cy;
			}

			int dx = max(0, (rectCheck.Width() - sizeCheckBox.cx) / 2);
			int dy = max(0, (rectCheck.Height() - sizeCheckBox.cy) / 2);

			rectCheck.DeflateRect(dx, dy);

			rect.left = rectCheck.right;

			BOOL bIsHighlighted = m_bIsCheckHighlighted && nIndex == m_nHighlightedItem;
			BOOL bIsEnabled = IsWindowEnabled() && IsEnabled(nIndex);

			CBCGPVisualManager::GetInstance ()->OnDrawCheckBoxEx
				(pDC, rectCheck, GetCheck(nIndex), bIsHighlighted, FALSE, bIsEnabled);
		}

		int nIcon = -1;
		if ((m_hImageList != NULL || m_ImageList.GetCount() > 0) && (nIcon = GetItemImage(nIndex)) >= 0)
		{
			CRect rectTop = rect;

			if (m_bHasDescriptions && m_nDescrRows > 0)
			{
				rectTop.DeflateRect(0, nTextRowHeight / 3);
				rectTop.bottom = rectTop.top + rect.Height() / (m_nDescrRows + 1);
			}

			CRect rectImage = rectTop;
			rectImage.top += (rectTop.Height () - m_sizeImage.cy) / 2;
			rectImage.bottom = rectImage.top + m_sizeImage.cy;

			rectImage.left += nHorzMargin;
			rectImage.right = rectImage.left + m_sizeImage.cx;

			if (m_hImageList != NULL)
			{
				CImageList* pImageList = CImageList::FromHandle(m_hImageList);
				pImageList->Draw (pDC, nIcon, rectImage.TopLeft (), ILD_TRANSPARENT);
			}
			else
			{
				m_ImageList.DrawEx(pDC, rectImage, nIcon);
			}

			rect.left += m_sizeImage.cx + max(2 * nHorzMargin, m_sizeImage.cx / 3);
			rect.right -= 2 * nHorzMargin;
		}
		else
		{
			rect.DeflateRect(nHorzMargin, 0);
		}
	}
#ifndef _BCGSUITE_
	else
	{
		pOldFont = pDC->SelectObject(&globalData.fontCaption);
	}
#endif

	CString strText;
	GetText (nIndex, strText);

	UINT uiDTFlags = DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX;

	if (bIsCaption)
	{
		int nTextHeight = OnDrawItemName(pDC, xStart, rect, nIndex, strText, uiDTFlags);

		CBCGPStatic ctrl;
		ctrl.m_bBackstageMode = m_bBackstageMode;

		CRect rectSeparator = rect;
		rectSeparator.top = rect.CenterPoint().y + nTextHeight / 2;
		rectSeparator.bottom = rectSeparator.top + 1;
		rectSeparator.right -= nHorzMargin + 1;

#ifndef _BCGSUITE_
		if (!globalData.IsHighContastMode () && CBCGPVisualManager::GetInstance ()->IsOwnerDrawDlgSeparator(&ctrl))
		{
			CBCGPVisualManager::GetInstance ()->OnDrawDlgSeparator(pDC, &ctrl, rectSeparator, TRUE);
		}
		else
#endif
		{
			CPen pen (PS_SOLID, 1, globalData.clrBtnShadow);
			CPen* pOldPen = (CPen*)pDC->SelectObject (&pen);

			pDC->MoveTo(rectSeparator.left, rectSeparator.top);
			pDC->LineTo(rectSeparator.right, rectSeparator.top);

			pDC->SelectObject(pOldPen);
		}

		pDC->SelectObject(pOldFont);
	}
	else if (!m_bHasDescriptions || m_nDescrRows == 0)
	{
		OnDrawItemName(pDC, xStart, rect, nIndex, strText, uiDTFlags);
	}
	else
	{
		if (m_hFont != NULL)
		{
			pOldFont = pDC->SelectObject (CFont::FromHandle(m_hFont));
		}
		else
		{
			pOldFont = pDC->SelectObject(&globalData.fontBold);
		}

		rect.DeflateRect(0, nTextRowHeight / 3);

		CRect rectTop = rect;
		rectTop.bottom = rectTop.top + rectTop.Height() / (m_nDescrRows + 1);

		OnDrawItemName(pDC, xStart, rectTop, nIndex, strText, uiDTFlags);

		pDC->SelectObject(pOldFont);

		LPCTSTR lpszDescr = GetItemDescription(nIndex);
		if (lpszDescr != NULL)
		{
			CString strDescr(lpszDescr);
			
			if (!strDescr.IsEmpty())
			{
				CRect rectBottom = rect;
				rectBottom.top = rectTop.bottom;

				uiDTFlags = DT_LEFT | DT_END_ELLIPSIS | DT_NOPREFIX | DT_WORDBREAK;
				pDC->DrawText(strDescr, rectBottom, uiDTFlags);
			}
		}
	}
}
//***************************************************************************************
int CBCGPListBox::DU2DP(int nDU)
{
	CRect rectDummy(0, 0, nDU, nDU);
	if (GetParent()->GetSafeHwnd() != NULL && MapDialogRect(GetParent()->GetSafeHwnd(), &rectDummy))
	{
		return rectDummy.right;
	}
	else
	{
		return MulDiv(nDU, LOWORD(::GetDialogBaseUnits()), 4);
	}
}
//***************************************************************************************
int CBCGPListBox::OnDrawItemName(CDC* pDC, int xStart, CRect rect, int /*nIndex*/, const CString& strName, UINT nDrawFlags)
{
	ASSERT_VALID(pDC);

	if ((GetStyle() & LBS_USETABSTOPS) == 0)
	{
		return pDC->DrawText (strName, rect, nDrawFlags);
	}

	int x = rect.left;
	int nTabStopIndex = 0;
	int cxCommonTabStop = (m_arTabStops.GetSize() == 1) ? DU2DP(m_arTabStops[0]) : 0;
	int cyMaxHeight = 0;

	for (int nStart = 0;; nTabStopIndex++)
	{
		int i = strName.Find(_T('\t'), nStart);
		CString strWord = (i < 0) ? strName.Mid(nStart) : strName.Mid(nStart, i - nStart);

		CRect rectWord = rect;
		rectWord.left = x;

		cyMaxHeight = max(pDC->DrawText(strWord, rectWord, nDrawFlags), cyMaxHeight);

		if (i < 0)
		{
			// Last part
			break;
		}

		int xTabStop = rect.left + max(0, cxCommonTabStop * (nTabStopIndex + 1));

		if (cxCommonTabStop <= 0 && nTabStopIndex < m_arTabStops.GetSize())
		{
			xTabStop = DU2DP(m_arTabStops[nTabStopIndex]);
		}

		int cxText = pDC->GetTextExtent(strWord).cx + 5;
		if (x + cxText > xTabStop + xStart)
		{
			x += cxText;
		}
		else
		{
			x = xTabStop + xStart;
		}

		nStart = i + 1;
	}

	return cyMaxHeight;
}
//***************************************************************************************
BOOL CBCGPListBox::SetImageList (HIMAGELIST hImageList, int nVertMargin)
{
	ASSERT (hImageList != NULL);

	m_hImageList = NULL;
	m_ImageList.Clear();
	m_sizeImage = CSize(0, 0);

	CImageList* pImageList = CImageList::FromHandle (hImageList);
	if (pImageList == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	IMAGEINFO info;
	pImageList->GetImageInfo (0, &info);

	CRect rectImage = info.rcImage;
	m_sizeImage = rectImage.Size ();

	m_hImageList = hImageList;

	SetItemHeight(-1, max(GetItemHeight(-1), m_sizeImage.cy + 2 * nVertMargin));
	return TRUE;
}
//***************************************************************************************
BOOL CBCGPListBox::SetImageList(UINT nImageListResID, int cxIcon, int nVertMargin)
{
	m_hImageList = NULL;
	m_ImageList.Clear();
	m_sizeImage = CSize(0, 0);

	if (nImageListResID == 0)
	{
		return TRUE;
	}

	if (!m_ImageList.Load(nImageListResID))
	{
		ASSERT (FALSE);
		return FALSE;
	}

	m_ImageList.SetSingleImage();

	int cyIcon = m_ImageList.GetImageSize().cy;
	m_sizeImage = CSize(cxIcon, cyIcon);

	m_ImageList.SetImageSize(m_sizeImage, TRUE);

	SetItemHeight(-1, max(GetItemHeight(-1), m_sizeImage.cy + 2 * nVertMargin));
	return TRUE;
}
//***********************************************************************************************************
void CBCGPListBox::SetItemImage(int nIndex, int nImageIndex)
{
	BCGP_LB_ITEM_DATA* pState = _GetAllocItemData(nIndex);
	if (pState != NULL)
	{
		pState->m_nImageIndex = nImageIndex;
	}
}
//***********************************************************************************************************
int CBCGPListBox::GetItemImage(int nIndex) const
{
	const BCGP_LB_ITEM_DATA* pState = _GetItemData(nIndex);
	if (pState != NULL)
	{
		return pState->m_nImageIndex;
	}

	return -1;
}
//***********************************************************************************************************
void CBCGPListBox::SetItemDescription(int nIndex, const CString& strDescription)
{
	BCGP_LB_ITEM_DATA* pState = _GetAllocItemData(nIndex);
	if (pState != NULL)
	{
		pState->m_strDescription = strDescription;

		if (!strDescription.IsEmpty())
		{
			m_bHasDescriptions = TRUE;
		}
	}
}
//***********************************************************************************************************
LPCTSTR CBCGPListBox::GetItemDescription(int nIndex) const
{
	const BCGP_LB_ITEM_DATA* pState = _GetItemData(nIndex);
	if (pState != NULL)
	{
		return pState->m_strDescription;
	}
	
	return NULL;
}
//***********************************************************************************************************
BOOL CBCGPListBox::OnEraseBkgnd(CDC* /*pDC*/)
{
	if (GetHorizontalExtent() > 0)
	{
		return (BOOL)Default();
	}

	return TRUE;
}
//***********************************************************************************************************
void CBCGPListBox::OnDraw(CDC* pDC) 
{
	ASSERT_VALID(pDC);

	if (m_hFont != NULL && ::GetObjectType (m_hFont) != OBJ_FONT)
	{
		m_hFont = NULL;
		m_nTextHeight = -1;
	}
	
	CRect rectClient;
	GetClientRect(rectClient);

	int cxScroll = GetScrollPos(SB_HORZ);
	BOOL bNoSel = ((GetStyle() & LBS_NOSEL) == LBS_NOSEL);

	COLORREF clrTextDefault = globalData.clrWindowText;

#ifndef _BCGSUITE_
	if (m_bVisualManagerStyle)
	{
		clrTextDefault = CBCGPVisualManager::GetInstance ()->OnFillListBox(pDC, this, rectClient);
	}
	else
#endif
	{
		pDC->FillRect(rectClient, &globalData.brWindow);
	}

#ifndef _BCGSUITE_
	if (m_bBackstageMode)
	{
		if (m_bPropertySheetNavigator)
		{
			CBCGPPropertySheet* pPropSheet = DYNAMIC_DOWNCAST(CBCGPPropertySheet, GetParent());
			if (pPropSheet != NULL)
			{
				ASSERT_VALID(pPropSheet);
				pPropSheet->OnDrawListBoxBackground(pDC, this);
			}

			CRect rectSeparator = rectClient;
			rectSeparator.left = rectSeparator.right - 3;
			rectSeparator.right -= 2;
			CBCGPStatic ctrl;
			ctrl.m_bBackstageMode = TRUE;

			CBCGPVisualManager::GetInstance ()->OnDrawDlgSeparator(pDC, &ctrl, rectSeparator, FALSE);
		}
	}
#endif
	int nStart = GetTopIndex ();
	int nCount = GetCount ();

	if (nStart != LB_ERR && nCount > 0)
	{
		pDC->SetBkMode (TRANSPARENT);

		CFont* pOldFont = NULL;
		
		if (m_hFont != NULL)
		{
			pOldFont = pDC->SelectObject (CFont::FromHandle(m_hFont));
		}
		else
		{
			pOldFont = pDC->SelectObject (&globalData.fontRegular);
		}

		ASSERT_VALID (pOldFont);

		CArray<int,int> arSelection;

		int nSelCount = GetSelCount();
		if (nSelCount != LB_ERR && !bNoSel)
		{
			if (nSelCount > 0)
			{
				arSelection.SetSize (nSelCount);
				GetSelItems (nSelCount, arSelection.GetData());	
			}
		}
		else
		{
			int nSel = GetCurSel();
			if (nSel != LB_ERR)
			{
				nSelCount = 1;
				arSelection.Add (nSel);
			}
		}

		nSelCount = (int)arSelection.GetSize ();

		const BOOL bIsFocused = (CWnd::GetFocus() == this);
		
		BOOL bFocusRectDrawn = FALSE;

		for (int nIndex = nStart; nIndex < nCount; nIndex++)
		{
			CRect rect;
			GetItemRect(nIndex, rect);

			if (rect.bottom < rectClient.top || rectClient.bottom < rect.top)
			{
				break;
			}

			int cxExtent = GetHorizontalExtent();
			if (cxExtent > rect.Width())
			{
				rect.right = rect.left + cxExtent;
			}

			rect.OffsetRect(-cxScroll, 0);
			rect.DeflateRect(2, 0);

			CRect rectPin(0, 0, 0, 0);
			if (m_bPins && !IsSeparatorItem(nIndex) && !IsCaptionItem(nIndex))
			{
				rectPin = rect;
				rect.right -= PIN_AREA_WIDTH;
				rectPin.left = rect.right;
			}

			BOOL bIsSelected = FALSE;
			for (int nSelIndex = 0; nSelIndex < nSelCount; nSelIndex++)
			{
				if (nIndex == arSelection[nSelIndex])
				{
					bIsSelected = TRUE;
					break;
				}
			}
		
			const BOOL bIsHighlihted = (nIndex == m_nHighlightedItem) || (bIsSelected && bIsFocused);

			COLORREF clrText = (COLORREF)-1;

			BOOL bIsCaptionItem = IsCaptionItem(nIndex);

			if (((bIsHighlihted && !m_bIsPinHighlighted && m_bItemHighlighting) || bIsSelected) && !bIsCaptionItem && !bNoSel)
			{
				if (m_bVisualManagerStyle)
				{
					clrText = CBCGPVisualManager::GetInstance ()->OnFillListBoxItem (
						pDC, this, nIndex, rect, bIsHighlihted && m_bItemHighlighting, bIsSelected);
				}
				else
				{
					pDC->FillRect (rect, &globalData.brHilite);
					
					if (bIsHighlihted && m_bItemHighlighting)
					{
						pDC->DrawFocusRect (rect);
						bFocusRectDrawn = TRUE;
					}

					clrText = globalData.clrTextHilite;
				}
			}

			BOOL bIsItemDisabled = !IsWindowEnabled() || !IsEnabled(nIndex);
			if (bIsItemDisabled)
			{
#ifndef _BCGSUITE_
				clrText = CBCGPVisualManager::GetInstance()->GetToolbarDisabledTextColor();
#else
				clrText = globalData.clrGrayedText;
#endif
			}

			if (clrText == (COLORREF)-1)
			{
				pDC->SetTextColor (clrTextDefault);
			}
			else
			{
				pDC->SetTextColor (clrText);
			}

			OnDrawItemContent(pDC, rect, nIndex);

			if (bIsFocused && !bFocusRectDrawn && !bIsCaptionItem && !bNoSel)
			{
				BOOL bDraw = FALSE;

				if ((GetStyle() & LBS_MULTIPLESEL) != 0)
				{
					bDraw = (nIndex == GetCurSel());
				}
				else if (GetStyle() & LBS_EXTENDEDSEL)
				{
					bDraw = !m_bItemHighlighting && (nIndex == m_nHighlightedItem) && ((GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0);
				}
				else
				{
					bDraw = nSelCount <= 0;
				}

				if (bDraw)
				{
					pDC->DrawFocusRect (rect);
					bFocusRectDrawn = TRUE;
				}
			}

			if (!rectPin.IsRectEmpty())
			{
				BOOL bIsPinHighlighted = FALSE;
				COLORREF clrTextPin = (COLORREF)-1;

				if (nIndex == m_nHighlightedItem && m_bIsPinHighlighted)
				{
					bIsPinHighlighted = TRUE;

					if (m_bVisualManagerStyle)
					{
						clrTextPin = CBCGPVisualManager::GetInstance ()->OnFillListBoxItem (
							pDC, this, nIndex, rectPin, TRUE, FALSE);
					}
					else
					{
						pDC->FillRect (rect, &globalData.brHilite);
					}
				}

				BOOL bIsDark = TRUE;

				if (clrTextPin != (COLORREF)-1)
				{
					if (GetRValue (clrTextPin) > 192 &&
						GetGValue (clrTextPin) > 192 &&
						GetBValue (clrTextPin) > 192)
					{
						bIsDark = FALSE;
					}
				}

				BOOL bIsPinned = IsItemPinned(nIndex);

#ifndef _BCGSUITE_
				CSize sizePin = CBCGPVisualManager::GetInstance ()->GetPinSize(bIsPinned);
				CRect rectPinImage(
					CPoint(
						rectPin.CenterPoint().x - sizePin.cx / 2,
						rectPin.CenterPoint().y - sizePin.cy / 2),
					sizePin);

				CBCGPVisualManager::GetInstance()->OnDrawPin(pDC, rectPinImage, bIsPinned,
					bIsDark, bIsPinHighlighted, FALSE, bIsItemDisabled);
#else
				CBCGPMenuImages::Draw(pDC, bIsPinned ? CBCGPMenuImages::IdPinVert : CBCGPMenuImages::IdPinHorz, rectPin);
#endif
			}
		}

		pDC->SelectObject (pOldFont);
	}
}
//***********************************************************************************************************
void CBCGPListBox::OnPaint() 
{
	CPaintDC dcPaint(this); // device context for painting

	CRect rectClient;
	GetClientRect(rectClient);

	CRgn rgn;
	rgn.CreateRectRgnIndirect (rectClient);
	dcPaint.SelectClipRgn (&rgn);

	{
		CBCGPMemDC memDC(dcPaint, this);
		OnDraw(&memDC.GetDC());
	}

	dcPaint.SelectClipRgn (NULL);
}
//************************************************************************************************************
BOOL CBCGPListBox::OnSelchange() 
{
	RedrawWindow();
	return FALSE;
}
//************************************************************************************************************
void CBCGPListBox::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	CListBox::OnVScroll(nSBCode, nPos, pScrollBar);
	RedrawWindow();
}
//**************************************************************************
LRESULT CBCGPListBox::OnBCGSetControlBackStageMode (WPARAM, LPARAM)
{
	m_bBackstageMode = TRUE;
	return 0;
}
//**************************************************************************
void CBCGPListBox::AddCaption(LPCTSTR lpszCaption)
{
	m_bInAddingCaption = TRUE;

	int nIndex = AddString(lpszCaption == NULL ? _T("") : lpszCaption);

	m_lstCaptionIndexes.AddTail(nIndex);

	m_bInAddingCaption = FALSE;
}
//**************************************************************************
void CBCGPListBox::AddSeparator()
{
	AddCaption(NULL);
}
//**************************************************************************
void CBCGPListBox::CleanUp()
{
	ResetContent();

	m_sizeImage = CSize(0, 0);
	m_hImageList = NULL;

	m_lstCaptionIndexes.RemoveAll();
}
//**************************************************************************
BOOL CBCGPListBox::IsCaptionItem(int nIndex) const
{
	return m_lstCaptionIndexes.Find(nIndex) != 0;
}
//**************************************************************************
BOOL CBCGPListBox::IsSeparatorItem(int nIndex) const
{
	return IsCaptionItem(nIndex);
}
//**************************************************************************
void CBCGPListBox::OnLButtonDown(UINT nFlags, CPoint point) 
{
	if ((GetStyle() & LBS_NOSEL) == LBS_NOSEL)
	{
		CListBox::OnLButtonDown(nFlags, point);
		return;
	}

	if (!m_lstCaptionIndexes.IsEmpty() && HitTest(point) == -1)
	{
		return;
	}
	
	CListBox::OnLButtonDown(nFlags, point);
	m_nClickedItem = HitTest(point);
}
//**************************************************************************
void CBCGPListBox::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if ((GetStyle() & LBS_NOSEL) == LBS_NOSEL)
	{
		CListBox::OnLButtonUp(nFlags, point);
		return;
	}

	HWND hwndThis = GetSafeHwnd();

	CListBox::OnLButtonUp(nFlags, point);

	if (!::IsWindow(hwndThis))
	{
		return;
	}

	if (IsSeparatorItem(m_nClickedItem))
	{
		return;
	}

	int nClickedItem = m_nClickedItem;
	m_nClickedItem = -1;

	BOOL bPin = FALSE;

	if (nClickedItem >= 0 && nClickedItem == HitTest(point, &bPin))
	{
		if (bPin)
		{
			OnClickPin(nClickedItem);
		}
		else
		{
			OnClickItem(nClickedItem);
		}
	}
}
//**************************************************************************
void CBCGPListBox::OnClickPin(int nClickedItem)
{
	if (GetOwner ()->GetSafeHwnd() != NULL)
	{
		GetOwner ()->SendMessage(BCGM_ON_CLICK_LISTBOX_PIN, GetDlgCtrlID (), LPARAM (nClickedItem));
	}
}
//**************************************************************************
void CBCGPListBox::OnCancelMode() 
{
	CListBox::OnCancelMode();
	m_nClickedItem = -1;
}
//**************************************************************************
void CBCGPListBox::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	const int nPrevSel = GetCurSel();

	CListBox::OnKeyDown(nChar, nRepCnt, nFlags);

	if ((GetStyle() & LBS_NOSEL) == LBS_NOSEL)
	{
		return;
	}

	if (m_lstCaptionIndexes.IsEmpty())
	{
		return;
	}

	const int nCurSel = GetCurSel();
	if (!IsCaptionItem(nCurSel) || nCurSel == nPrevSel)
	{
		return;
	}

	BOOL bFindNext = FALSE;

	switch (nChar)
	{
	case VK_HOME:
	case VK_DOWN:
	case VK_NEXT:
		bFindNext = TRUE;
		break;
	}

	int nNewSel = -1;

	if (bFindNext)
	{
		for(int i = nCurSel + 1; i < GetCount(); i++)
		{
			if (!IsCaptionItem(i))
			{
				nNewSel = i;
				break;
			}
		}
	}
	else
	{
		for(int i = nCurSel - 1; i >= 0; i--)
		{
			if (!IsCaptionItem(i))
			{
				nNewSel = i;
				break;
			}
		}
	}

	SetCurSel(nNewSel != -1 ? nNewSel : nPrevSel);
	RedrawWindow();
}
//**************************************************************************
BOOL CBCGPListBox::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
	if (CListBox::OnMouseWheel(nFlags, zDelta, pt))
	{
		RedrawWindow();
		return TRUE;
	}

	return FALSE;
}
//**************************************************************************
void CBCGPListBox::EnableItemDescription(BOOL bEnable, int nRows)
{
	m_nDescrRows = bEnable ? nRows : 0;

	if (GetSafeHwnd() != NULL && (GetStyle() & (LBS_OWNERDRAWFIXED | LBS_HASSTRINGS)) == (LBS_OWNERDRAWFIXED | LBS_HASSTRINGS))
	{
		SetItemHeight(0, GetItemMinHeight());
	}
}
//**************************************************************************
void CBCGPListBox::EnableItemHighlighting(BOOL bEnable)
{
	m_bItemHighlighting = bEnable;
}
//**************************************************************************
BOOL CBCGPListBox::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN && GetFocus()->GetSafeHwnd() == GetSafeHwnd())
	{
		if (OnReturnKey())
		{
			return TRUE;
		}
	}
	
	return CListBox::PreTranslateMessage(pMsg);
}
//**************************************************************************
void CBCGPListBox::EnablePins(BOOL bEnable)
{
	m_bPins = bEnable;

	if (GetSafeHwnd() != NULL)
	{
		RedrawWindow();
	}
}
//**********************************************************************************************
void CBCGPListBox::SetItemPinned(int nIndex, BOOL bSet, BOOL bRedraw)
{
	BCGP_LB_ITEM_DATA* pState = _GetAllocItemData(nIndex);
	if (pState != NULL)
	{
		pState->m_bIsPinned = bSet;
	}

	if (bRedraw && GetSafeHwnd() != NULL)
	{
		CRect rectItem;
		GetItemRect(nIndex, rectItem);

		RedrawWindow (rectItem);
	}
}
//**********************************************************************************************
BOOL CBCGPListBox::IsItemPinned(int nIndex)
{
	const BCGP_LB_ITEM_DATA* pState = _GetItemData(nIndex);
	if (pState != NULL)
	{
		return pState->m_bIsPinned;
	}
	
	return FALSE;
}
//**********************************************************************************************
void CBCGPListBox::ResetPins()
{
	for (int i = 0; i < GetCount(); i++)
	{
		SetItemPinned(i, FALSE, FALSE);
	}
}
//**************************************************************************
void CBCGPListBox::Enable(int nIndex, BOOL bEnabled/* = TRUE*/, BOOL bRedraw/* = TRUE*/)
{
	BCGP_LB_ITEM_DATA* pState = _GetAllocItemData(nIndex);
	if (pState != NULL)
	{
		pState->m_bIsEnabled = bEnabled;
	}

	if (bRedraw && GetSafeHwnd() != NULL)
	{
		CRect rectItem;
		GetItemRect(nIndex, rectItem);

		RedrawWindow (rectItem);
	}
}
//**************************************************************************
BOOL CBCGPListBox::IsEnabled(int nIndex) const
{
	const BCGP_LB_ITEM_DATA* pState = _GetItemData(nIndex);
	if (pState != NULL)
	{
		return pState->m_bIsEnabled;
	}

	return TRUE;
}
//**********************************************************************************************
void CBCGPListBox::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	CListBox::OnHScroll(nSBCode, nPos, pScrollBar);
	RedrawWindow();
}
//**********************************************************************************************
void CBCGPListBox::OnNcPaint() 
{
	CListBox::OnNcPaint();

	const BOOL bHasBorder = (GetExStyle () & WS_EX_CLIENTEDGE) || (GetStyle () & WS_BORDER);

	if (bHasBorder && (m_bVisualManagerStyle || m_bOnGlass))
	{
		CBCGPDrawOnGlass dog (m_bOnGlass);
		CBCGPVisualManager::GetInstance ()->OnDrawControlBorder (this);
	}
}
//*********************************************************************************************
LRESULT CBCGPListBox::OnPrintClient(WPARAM wp, LPARAM lp)
{
	if (lp & PRF_CLIENT)
	{
		CDC* pDC = CDC::FromHandle((HDC) wp);
		ASSERT_VALID(pDC);

		OnDraw(pDC);
	}

	return 0;
}
//*********************************************************************************************
int CBCGPListBox::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CListBox::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!(GetStyle() & LBS_OWNERDRAWVARIABLE)) //must be one or the other
	{
		ModifyStyle(0, LBS_OWNERDRAWFIXED | LBS_HASSTRINGS);
	}

	ModifyStyle(0, LBS_HASSTRINGS);

	if ((GetStyle() & (LBS_OWNERDRAWFIXED | LBS_HASSTRINGS)) == (LBS_OWNERDRAWFIXED | LBS_HASSTRINGS))
	{
		SetItemHeight(0, GetItemMinHeight());
	}

	return 0;
}
//*********************************************************************************************
void CBCGPListBox::PreSubclassWindow() 
{
#ifdef _DEBUG
	if (!(GetStyle() & (LBS_OWNERDRAWVARIABLE | LBS_OWNERDRAWFIXED))) //must be one or the other
	{
		TRACE(_T("Warning: CBCGPListBox must be owner drawn\n"));
	}
#endif

	CListBox::PreSubclassWindow();

	ModifyStyle(0, LBS_HASSTRINGS);

	if ((GetStyle() & (LBS_OWNERDRAWFIXED | LBS_HASSTRINGS)) == (LBS_OWNERDRAWFIXED | LBS_HASSTRINGS))
	{
		SetItemHeight(0, GetItemMinHeight());
	}
	
}
//*********************************************************************************************
int CBCGPListBox::GetItemMinHeight()
{
	int nVertMargin = 4;
	if (globalData.GetRibbonImageScale () != 1.)
	{
		nVertMargin = (int)(0.5 + globalData.GetRibbonImageScale() * nVertMargin);
	}

	int nCheckBoxHeight = !m_bHasCheckBoxes ? 0 : (CBCGPVisualManager::GetInstance ()->GetCheckRadioDefaultSize().cy + 4);
	int nImageHeight = m_sizeImage.cy == 0 ? 0 : m_sizeImage.cy + nVertMargin;
	int nTextHeight = 0;

	const int nTextRowHeight = m_nTextHeight == -1 ? globalData.GetTextHeight() : m_nTextHeight;
	
	if (IsPropertySheetNavigator())
	{
		nTextHeight = nTextRowHeight * 9 / 5;
	}
	else
	{
		if (m_nDescrRows == 0 || m_bInAddingCaption)
		{
			if ((GetStyle() & (LBS_OWNERDRAWFIXED | LBS_HASSTRINGS)) == (LBS_OWNERDRAWFIXED | LBS_HASSTRINGS))
			{
				nTextHeight = nTextRowHeight + nVertMargin;
			}
			else
			{
				nTextHeight = nTextRowHeight * 9 / 5;
			}
		}
		else
		{
			nTextHeight = (nTextRowHeight + nVertMargin) * (m_nDescrRows + 1);
		}
	}
	
	return max(nCheckBoxHeight, max(nImageHeight, nTextHeight));
}
//*********************************************************************************************
void CBCGPListBox::DeleteItem(LPDELETEITEMSTRUCT lpDeleteItemStruct)
{
	DELETEITEMSTRUCT deleteItem;
	memcpy(&deleteItem, lpDeleteItemStruct, sizeof(DELETEITEMSTRUCT));
	
	if (deleteItem.itemData == 0)
	{
		LRESULT lResult = DefWindowProc(LB_GETITEMDATA, deleteItem.itemID, 0);
		if (lResult != LB_ERR)
		{
			deleteItem.itemData = (UINT)lResult;
		}
	}
	
	if (deleteItem.itemData != 0 && deleteItem.itemData != LB_ERR)
	{
		BCGP_LB_ITEM_DATA* pState = (BCGP_LB_ITEM_DATA*)deleteItem.itemData;
		deleteItem.itemData = pState->m_dwData;
		delete pState;
	}
	
	CListBox::DeleteItem(&deleteItem);
}
//*********************************************************************************************
LRESULT CBCGPListBox::OnLBAddString(WPARAM wParam, LPARAM lParam)
{
	BCGP_LB_ITEM_DATA* pState = NULL;
	
	if (!(GetStyle() & LBS_HASSTRINGS))
	{
		pState = new BCGP_LB_ITEM_DATA;
		pState->m_dwData = lParam;
		lParam = (LPARAM)pState;
	}
	
	LRESULT lResult = DefWindowProc(LB_ADDSTRING, wParam, lParam);
	
	if (lResult == LB_ERR && pState != NULL)
	{
		delete pState;
	}
	
	return lResult;
}
//*********************************************************************************************
LRESULT CBCGPListBox::OnLBInsertString(WPARAM wParam, LPARAM lParam)
{
	BCGP_LB_ITEM_DATA* pState = NULL;
	
	if (!(GetStyle() & LBS_HASSTRINGS))
	{
		pState = new BCGP_LB_ITEM_DATA;
		pState->m_dwData = lParam;
		lParam = (LPARAM)pState;
	}
	
	LRESULT lResult = DefWindowProc(LB_INSERTSTRING, wParam, lParam);
	
	if (lResult == LB_ERR && pState != NULL)
	{
		delete pState;
	}
	
	return lResult;
}
//*********************************************************************************************
LRESULT CBCGPListBox::OnLBGetItemData(WPARAM wParam, LPARAM lParam)
{
	LRESULT lResult = DefWindowProc(LB_GETITEMDATA, wParam, lParam);
	
	if (lResult != LB_ERR)
	{
		BCGP_LB_ITEM_DATA* pState = (BCGP_LB_ITEM_DATA*)lResult;
		
		if (pState == NULL)
		{
			return 0;
		}
		
		lResult = pState->m_dwData;
	}
	
	return lResult;
}
//*********************************************************************************************
LRESULT CBCGPListBox::OnLBSetItemData(WPARAM wParam, LPARAM lParam)
{
	LRESULT lResult = DefWindowProc(LB_GETITEMDATA, wParam, 0);
	
	if (lResult != LB_ERR)
	{
		BCGP_LB_ITEM_DATA* pState = (BCGP_LB_ITEM_DATA*)lResult;
		if (pState == NULL)
		{
			pState = new BCGP_LB_ITEM_DATA;
		}
		
		pState->m_dwData = lParam;
		lResult = DefWindowProc(LB_SETITEMDATA, wParam, (LPARAM)pState);
		
		if (lResult == LB_ERR)
		{
			delete pState;
		}
	}
	
	return lResult;
}
//*****************************************************************************
LRESULT CBCGPListBox::OnSetFont(WPARAM wParam, LPARAM)
{
	m_hFont = (HFONT) wParam;

	if (m_hFont == 0)
	{
		m_nTextHeight = -1;
	}
	else
	{
		CClientDC dc(this);
		
		CFont* pOldFont = dc.SelectObject(CFont::FromHandle(m_hFont));
		ASSERT(pOldFont != NULL);
		
		TEXTMETRIC tm;
		dc.GetTextMetrics (&tm);
		
		int nTextMarginsHorz = tm.tmHeight < 15 ? 2 : 5;
		m_nTextHeight = tm.tmHeight + nTextMarginsHorz;
		
		dc.SelectObject (pOldFont);
	}

	if (GetSafeHwnd() != NULL && (GetStyle() & (LBS_OWNERDRAWFIXED | LBS_HASSTRINGS)) == (LBS_OWNERDRAWFIXED | LBS_HASSTRINGS))
	{
		SetItemHeight(0, GetItemMinHeight());
	}

	return Default();
}
//*****************************************************************************
LRESULT CBCGPListBox::OnLBSetTabstops(WPARAM wp, LPARAM lp)
{
	m_arTabStops.RemoveAll();

	int nTabStops = (int)wp;

	if (nTabStops <= 0)
	{
		m_arTabStops.Add(BCGP_DEFAULT_TABS_STOP);
	}
	else
	{
		LPINT arTabStops = (LPINT)lp;

		for (int i = 0; i < nTabStops; i++)
		{
			m_arTabStops.Add(arTabStops[i]);
		}
	}

	return Default();
}

//////////////////////////////////////////////////////////////////////////
// CBCGPCheckListBox

IMPLEMENT_DYNAMIC(CBCGPCheckListBox, CBCGPListBox)

CBCGPCheckListBox::CBCGPCheckListBox()
{
	m_nCheckStyle = BS_AUTOCHECKBOX;
	m_bHasCheckBoxes = TRUE;
}

CBCGPCheckListBox::~CBCGPCheckListBox()
{
}

BEGIN_MESSAGE_MAP(CBCGPCheckListBox, CBCGPListBox)
	//{{AFX_MSG_MAP(CBCGPCheckListBox)
	ON_WM_LBUTTONDOWN()
	ON_WM_KEYDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CBCGPCheckListBox::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if (nChar == VK_SPACE)
	{
		int nIndex = GetCaretIndex();
		
		CWnd* pParent = GetParent();
		ASSERT_VALID(pParent);
		
		if (nIndex != LB_ERR)
		{
			int nModulo = (m_nCheckStyle == BS_AUTO3STATE) ? 3 : 2;

			if (m_nCheckStyle != BS_CHECKBOX && m_nCheckStyle != BS_3STATE)
			{
				if ((GetStyle() & LBS_MULTIPLESEL) != 0)
				{
					if (IsEnabled(nIndex))
					{
						BOOL bSelected = GetSel(nIndex);
						if (bSelected)
						{
							int nCheck = GetCheck(nIndex);
							nCheck = (nCheck == nModulo) ? nCheck - 1 : nCheck;
							SetCheck(nIndex, (nCheck + 1) % nModulo);
							
							// Inform of check
							pParent->SendMessage(WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(), CLBN_CHKCHANGE), (LPARAM)m_hWnd);
						}

						SetSel(nIndex, !bSelected);
					}

					return;
				}
				else
				{
					// If there is a selection, the space bar toggles that check,
					// all other keys are the same as a standard listbox.
					
					if (IsEnabled(nIndex))
					{
						int nCheck = GetCheck(nIndex);
						nCheck = (nCheck == nModulo) ? nCheck - 1 : nCheck;
						
						int nNewCheck = (nCheck + 1) % nModulo;
						SetCheck(nIndex, nNewCheck);
						
						if (GetStyle() & LBS_EXTENDEDSEL)
						{
							// The listbox is a multi-select listbox, and the user
							// clicked on a selected check, so change the check on all
							// of the selected items.
							SetSelectionCheck(nNewCheck);
						}
						else
						{
							int nCurSel = GetCurSel();
							if (nCurSel < 0)
							{
								SetCurSel(nIndex);
							}
						}
						
						// Inform of check
						pParent->SendMessage(WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(), CLBN_CHKCHANGE), (LPARAM)m_hWnd);
					}
					
					return;
				}
			}
		}
	}
	
	CBCGPListBox::OnKeyDown(nChar, nRepCnt, nFlags);
}
//**********************************************************************************************
void CBCGPCheckListBox::SetCheckStyle(UINT nStyle)
{
	ASSERT(nStyle == 0 || nStyle == BS_CHECKBOX ||
		nStyle == BS_AUTOCHECKBOX || nStyle == BS_AUTO3STATE ||
		nStyle == BS_3STATE);

	m_nCheckStyle = nStyle;
	m_bHasCheckBoxes = (m_nCheckStyle != 0);

	if (GetSafeHwnd() != NULL)
	{
		RedrawWindow();
	}
}
//**********************************************************************************************
void CBCGPCheckListBox::SetCheck(int nIndex, int nCheck, BOOL bRedraw)
{
	BCGP_LB_ITEM_DATA* pState = _GetAllocItemData(nIndex);
	if (pState != NULL)
	{
		pState->m_nCheck = nCheck;
	}

	if (bRedraw && GetSafeHwnd() != NULL)
	{
		CRect rectItem;
		GetItemRect(nIndex, rectItem);

		RedrawWindow (rectItem);
	}
}
//**********************************************************************************************
int CBCGPCheckListBox::GetCheck(int nIndex) const
{
	const BCGP_LB_ITEM_DATA* pState = _GetItemData(nIndex);
	if (pState != NULL)
	{
		return pState->m_nCheck;
	}

	return BST_UNCHECKED;
}
//**********************************************************************************************
void CBCGPCheckListBox::OnLButtonDown(UINT nFlags, CPoint point)
{
	SetFocus();
	
	// determine where the click is
	BOOL bInCheck;
	int nIndex = HitTest(point, NULL, &bInCheck);
	
	// if the item is disabled, then eat the click
	if (!IsEnabled(nIndex))
	{
		return;
	}
	
	if (m_nCheckStyle != BS_CHECKBOX && m_nCheckStyle != BS_3STATE)
	{
		// toggle the check mark automatically if the check mark was hit
		if (bInCheck)
		{
			CWnd* pParent = GetParent();
			ASSERT_VALID(pParent);
			
			int nModulo = (m_nCheckStyle == BS_AUTO3STATE) ? 3 : 2;
			int nCheck = GetCheck(nIndex);
			nCheck = (nCheck == nModulo) ? nCheck - 1 : nCheck;

			int nNewCheck = (nCheck + 1) % nModulo;
			SetCheck(nIndex, nNewCheck);
			
			if ((GetStyle() & (LBS_EXTENDEDSEL | LBS_MULTIPLESEL)) && GetSel(nIndex))
			{
				// The listbox is a multi-select listbox, and the user clicked on
				// a selected check, so change the check on all of the selected
				// items.
				SetSelectionCheck(nNewCheck);
			}
			else
			{
				CBCGPListBox::OnLButtonDown(nFlags, point);
			}
			
			// Inform parent of check
			pParent->SendMessage(WM_COMMAND, MAKEWPARAM( GetDlgCtrlID(), CLBN_CHKCHANGE ), (LPARAM)m_hWnd);
			return;
		}
	}
	
	// do default listbox selection logic
	CBCGPListBox::OnLButtonDown( nFlags, point);
}
//**********************************************************************************************
void CBCGPCheckListBox::SetSelectionCheck(int nCheck)
{
	int nSelectedItems = GetSelCount();
	if (nSelectedItems > 0)
	{
		CArray<int,int> rgiSelectedItems;
		rgiSelectedItems.SetSize(nSelectedItems);
		int *piSelectedItems = rgiSelectedItems.GetData(); 

		GetSelItems(nSelectedItems, piSelectedItems);
		
		for (int iSelectedItem = 0; iSelectedItem < nSelectedItems; iSelectedItem++)
		{
			if (IsEnabled (piSelectedItems[iSelectedItem]))
			{
				SetCheck(piSelectedItems[iSelectedItem], nCheck);
			}
		}
	}
}
//**********************************************************************************************
int CBCGPCheckListBox::GetCheckCount() const
{
	int nCheckCount = 0;

	for (int i = 0; i < GetCount (); i++)
	{
		if (GetCheck(i) != 0)
		{
			nCheckCount++;
		}
	}

	return nCheckCount;
}
