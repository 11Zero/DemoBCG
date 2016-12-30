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
// BCGPIntelliSenseLB.cpp : implementation file
//

#include "stdafx.h"
#include "BCGPEditCtrl.h"

#ifndef BCGP_EXCLUDE_EDIT_CTRL

#include "BCGPIntelliSenseWnd.h"
#include "BCGPIntelliSenseLB.h"

#ifndef _BCGPEDIT_STANDALONE
#include "bcgglobals.h"
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE (CBCGPBaseIntelliSenseLB, CListBox)
IMPLEMENT_DYNCREATE (CBCGPIntelliSenseLB, CBCGPBaseIntelliSenseLB)
IMPLEMENT_DYNCREATE (CBCGPSymImagesLB, CBCGPBaseIntelliSenseLB)

int CBCGPBaseIntelliSenseLB::m_nNumVisibleItems = 10;

int	CBCGPBaseIntelliSenseLB::m_nImageToFocusRectSpacing = 2;
int	CBCGPBaseIntelliSenseLB::m_nFocusRectToTextSpacing = 1;
int	CBCGPBaseIntelliSenseLB::m_nRightSpacing = 3;

COLORREF CBCGPBaseIntelliSenseLB::m_clrSelectedItemBkColor = GetSysColor (COLOR_HIGHLIGHT);
COLORREF CBCGPBaseIntelliSenseLB::m_clrSelectedItemTextColor = GetSysColor (COLOR_HIGHLIGHTTEXT);
COLORREF CBCGPBaseIntelliSenseLB::m_clrWindow = GetSysColor (COLOR_WINDOW);
COLORREF CBCGPBaseIntelliSenseLB::m_clrWindowText = GetSysColor (COLOR_WINDOWTEXT);

BOOL CBCGPIntelliSenseLB::m_bComparenoCase = FALSE;


BEGIN_MESSAGE_MAP(CBCGPBaseIntelliSenseLB, CListBox)
	//{{AFX_MSG_MAP(CBCGPBaseIntelliSenseLB)
	ON_CONTROL_REFLECT(LBN_DBLCLK, OnDblclk)
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_CONTROL_REFLECT(LBN_KILLFOCUS, OnKillfocus)
	ON_WM_SYSKEYDOWN()
	ON_WM_NCDESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGPBaseIntelliSenseLB

CBCGPBaseIntelliSenseLB::CBCGPBaseIntelliSenseLB () : m_pImageList (NULL), m_nLBHeight (0), 
											 m_pFont (NULL)
{
	m_sizeMaxItem.cx = m_sizeMaxItem.cy = 0;

	UpdateColors ();
}
//*****************************************************************************************
CBCGPBaseIntelliSenseLB::~CBCGPBaseIntelliSenseLB()
{
}
//*****************************************************************************************
CSize CBCGPBaseIntelliSenseLB::GetTextSize (CString& str)
{
	CDC* pDC = GetDC ();

	CFont* pOldFont = pDC->SelectObject (m_pFont);
	CSize size = pDC->GetTextExtent (str, str.GetLength ());
	pDC->SelectObject (pOldFont);

	ReleaseDC(pDC);
	return size;
}
//*****************************************************************************************
CSize CBCGPBaseIntelliSenseLB::GetImageSize (int /*iIdx*/)
{
	if (m_pImageList != NULL)
	{
		IMAGEINFO imageInfo;
		m_pImageList->GetImageInfo (0, &imageInfo);

		CRect rect (imageInfo.rcImage);
		return rect.Size ();
	}

	return CSize (0, 0);
}
//*****************************************************************************************
CBCGPEditCtrl* CBCGPBaseIntelliSenseLB::GetParentEditCtrl ()
{
	CBCGPIntelliSenseWnd* pParent = DYNAMIC_DOWNCAST (CBCGPIntelliSenseWnd, GetParent ());
	ASSERT_VALID (pParent);
	return pParent->GetParentEditCtrl ();
}
//*****************************************************************************************
BOOL CBCGPBaseIntelliSenseLB::PreTranslateMessage(MSG* pMsg) 
{
	switch (pMsg->message)
	{
	case WM_SYSKEYUP:
		if (VK_MENU == pMsg->wParam)
		{
			return TRUE;
		}
		break;		
	}
	
	return CListBox::PreTranslateMessage(pMsg);
}
//*****************************************************************************************
void CBCGPBaseIntelliSenseLB::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	CDC* pDC = GetDC ();

	CRect rectItem (lpDrawItemStruct->rcItem);

	pDC->FillSolidRect (&rectItem, m_clrWindow);

	if (lpDrawItemStruct->itemData != NULL)
	{
		CBCGPIntelliSenseData* pData = (CBCGPIntelliSenseData*)
											lpDrawItemStruct->itemData;
		ASSERT_VALID (pData);
		
		if (m_pImageList != NULL)
		{
			m_pImageList->Draw (pDC, pData->m_nImageListIndex, rectItem.TopLeft (), 
								ILD_NORMAL);
			rectItem.left += GetImageSize (pData->m_nImageListIndex).cx + 
							 m_nImageToFocusRectSpacing;  
		}

		if (lpDrawItemStruct->itemAction & ODA_FOCUS && 
			lpDrawItemStruct->itemState & ODS_FOCUS)
		{
			pDC->DrawFocusRect (rectItem);
		}

		rectItem.DeflateRect (1, 1, 1, 1);

		if (lpDrawItemStruct->itemState & ODS_SELECTED)
		{
			pDC->FillSolidRect (&rectItem, m_clrSelectedItemBkColor);
			pDC->SetTextColor (m_clrSelectedItemTextColor);
		}
		else
		{ 
			pDC->FillSolidRect (&rectItem, m_clrWindow);
			pDC->SetTextColor (m_clrWindowText);
		}

		pDC->SetBkMode (TRANSPARENT);
		CFont* pOldFont = pDC->SelectObject (m_pFont);
		rectItem.left += m_nFocusRectToTextSpacing;
		pDC->DrawText (pData->m_strItemName, &rectItem, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
		pDC->SelectObject (pOldFont);
	}

	ReleaseDC (pDC);
}
//*****************************************************************************************
void CBCGPBaseIntelliSenseLB::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct) 
{
	CBCGPIntelliSenseData* pData = (CBCGPIntelliSenseData*)
										lpMeasureItemStruct->itemData;
	ASSERT_VALID (pData);

	CSize sizeText = GetTextSize (pData->m_strItemName);
	CSize sizeImage = GetImageSize (pData->m_nImageListIndex);

	lpMeasureItemStruct->itemHeight = max (sizeText.cy, sizeImage.cy) + 2;
	lpMeasureItemStruct->itemWidth  = sizeText.cx + sizeImage.cx + 
									  m_nImageToFocusRectSpacing + 
									  m_nFocusRectToTextSpacing + 
									  m_nRightSpacing + 1;

	m_sizeMaxItem.cx = max ((UINT) m_sizeMaxItem.cx, lpMeasureItemStruct->itemWidth);
	m_sizeMaxItem.cy = max ((UINT) m_sizeMaxItem.cy, lpMeasureItemStruct->itemHeight);

	if (GetCount () <= m_nNumVisibleItems)
	{
		m_nLBHeight += lpMeasureItemStruct->itemHeight;
	}
}
//*****************************************************************************************
void CBCGPBaseIntelliSenseLB::DeleteItem(LPDELETEITEMSTRUCT lpDeleteItemStruct)
{
	if (lpDeleteItemStruct->itemData != NULL)
	{
		CBCGPIntelliSenseData* pData = (CBCGPIntelliSenseData*)
											lpDeleteItemStruct->itemData;
		ASSERT_VALID (pData);
		
		delete pData;
		SetItemDataPtr (lpDeleteItemStruct->itemID, NULL);
	}

	CListBox::DeleteItem (lpDeleteItemStruct);
}
//*****************************************************************************************
void CBCGPBaseIntelliSenseLB::OnDblclk() 
{
	InsertDataToEditCtrl ();
	if (!::IsWindow(m_hWnd))
	{
		return;
	}
	PostMessage (WM_CLOSE);
}
//*****************************************************************************************
void CBCGPBaseIntelliSenseLB::OnDestroy() 
{
	for (int i = 0; i < GetCount (); i++)
	{
		CBCGPIntelliSenseData* pData = 
			(CBCGPIntelliSenseData*) GetItemDataPtr (i);
		delete pData;
		SetItemDataPtr (i, NULL);
	}
	CListBox::OnDestroy();
}
//*****************************************************************************************
BOOL CBCGPBaseIntelliSenseLB::OnEraseBkgnd(CDC* /*pDC*/) 
{
	return TRUE;
}
//*****************************************************************************************
void CBCGPBaseIntelliSenseLB::OnKillfocus() 
{
	GetParent ()->PostMessage (WM_CLOSE);
}
//*****************************************************************************************
void CBCGPBaseIntelliSenseLB::OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if (nChar == VK_MENU)	
	{
		GetParent ()->PostMessage (WM_CLOSE);
		return;
	}
	
	CListBox::OnSysKeyDown(nChar, nRepCnt, nFlags);
}
//*****************************************************************************************
void CBCGPBaseIntelliSenseLB::OnNcDestroy() 
{
	CListBox::OnNcDestroy();
	delete this;
}
//*****************************************************************************************
void CBCGPBaseIntelliSenseLB::OnSysColorChange()
{
	CListBox::OnSysColorChange();
	UpdateColors ();
}
//*****************************************************************************************
void CBCGPBaseIntelliSenseLB::UpdateColors ()
{
#ifdef _BCGPEDIT_STANDALONE
	CBCGPBaseIntelliSenseLB::m_clrSelectedItemBkColor = GetSysColor (COLOR_HIGHLIGHT);
	CBCGPBaseIntelliSenseLB::m_clrSelectedItemTextColor = GetSysColor (COLOR_HIGHLIGHTTEXT);
	CBCGPBaseIntelliSenseLB::m_clrWindow = GetSysColor (COLOR_WINDOW);
	CBCGPBaseIntelliSenseLB::m_clrWindowText = GetSysColor (COLOR_WINDOWTEXT);
#else
	CBCGPBaseIntelliSenseLB::m_clrSelectedItemBkColor = globalData.clrHilite;
	CBCGPBaseIntelliSenseLB::m_clrSelectedItemTextColor = globalData.clrTextHilite;
	CBCGPBaseIntelliSenseLB::m_clrWindow = globalData.clrWindow;
	CBCGPBaseIntelliSenseLB::m_clrWindowText = globalData.clrWindowText;
#endif // _BCGPEDIT_STANDALONE
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPIntelliSenseLB

CBCGPIntelliSenseLB::CBCGPIntelliSenseLB() 
{
	m_nInitOffset = m_nInitRow = -1;
}

CBCGPIntelliSenseLB::~CBCGPIntelliSenseLB()
{
}


BEGIN_MESSAGE_MAP(CBCGPIntelliSenseLB, CBCGPBaseIntelliSenseLB)
	//{{AFX_MSG_MAP(CBCGPIntelliSenseLB)
	ON_WM_CHAR()
	ON_WM_KEYDOWN()
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGPIntelliSenseLB message handlers
int CBCGPIntelliSenseLB::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CBCGPBaseIntelliSenseLB::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	CBCGPEditCtrl* pParentEdit = GetParentEditCtrl ();
	ASSERT_VALID (pParentEdit);

	int nCurrOffset = pParentEdit->GetCurOffset ();
	int nFinish = 0;
	pParentEdit->FindWordStartFinish (nCurrOffset, pParentEdit->m_strBuffer, m_nInitOffset, nFinish);
	
    // If we are at the end of the word (and the next symbol is not in m_strNonSelectableChars)
    if (m_nInitOffset > nFinish)
	{
		pParentEdit->FindWordStartFinish (nCurrOffset - 1, pParentEdit->m_strBuffer, m_nInitOffset, nFinish);
	}

	if (nFinish < nCurrOffset)
	{
		m_nInitOffset = nFinish = nCurrOffset;
	}
	
	m_nInitOffset = min (nCurrOffset, m_nInitOffset);

	CPoint pt;
	CPoint ptRowColumn;
	pParentEdit->OffsetToPoint (nCurrOffset, pt, &ptRowColumn);
	m_nInitRow = ptRowColumn.y;
	
	return 0;
}
//*****************************************************************************************
int CBCGPIntelliSenseLB::CompareItem(LPCOMPAREITEMSTRUCT lpCompareItemStruct) 
{
	// TODO: Add your code to determine the sorting order of the specified items
	// return -1 = item 1 sorts before item 2
	// return 0 = item 1 and item 2 sort the same
	// return 1 = item 1 sorts after item 2

	CString strToCompare;
	BOOL bFindSubstring = FALSE;
	if (lpCompareItemStruct->itemID1 == -1)
	{
		strToCompare = (LPCTSTR) lpCompareItemStruct->itemData1;
		bFindSubstring = TRUE;
	}
	else
	{
		CBCGPIntelliSenseData* pData1 = (CBCGPIntelliSenseData*)
										lpCompareItemStruct->itemData1;
		ASSERT_VALID (pData1);
		strToCompare = pData1->m_strItemName;
	}

	
	CBCGPIntelliSenseData* pData2 = (CBCGPIntelliSenseData*)
										lpCompareItemStruct->itemData2;

	
	ASSERT_VALID (pData2);
	
	if (bFindSubstring)
	{
		if (strToCompare.GetLength () < pData2->m_strItemName.GetLength ())
		{
			return CBCGPIntelliSenseLB::m_bComparenoCase ?
				lstrcmpi (strToCompare, pData2->m_strItemName.Left (strToCompare.GetLength ())) :
				lstrcmp  (strToCompare, pData2->m_strItemName.Left (strToCompare.GetLength ()));
		}
		return CBCGPIntelliSenseLB::m_bComparenoCase ? 
				lstrcmpi (strToCompare, pData2->m_strItemName) :
				lstrcmp  (strToCompare, pData2->m_strItemName);
		
	}

	return CBCGPIntelliSenseLB::m_bComparenoCase ? 
				lstrcmpi (strToCompare, pData2->m_strItemName) :
				lstrcmp (strToCompare, pData2->m_strItemName);
}
//*****************************************************************************************
void CBCGPIntelliSenseLB::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	CBCGPEditCtrl* pEditCtrl = GetParentEditCtrl ();	
	ASSERT_VALID (pEditCtrl);

	if (nChar == VK_TAB)
	{
		return;
	}

	pEditCtrl->SendMessage (WM_CHAR, nChar, MAKELPARAM (nRepCnt, nFlags));
	SelectCurrentWord ();
}
//*****************************************************************************************
void CBCGPIntelliSenseLB::SelectCurrentWord ()
{
	CBCGPEditCtrl* pEditCtrl = GetParentEditCtrl ();	
	ASSERT_VALID (pEditCtrl);
	
	int nWordStart = 0;
	int nWordFinish = 0;

	pEditCtrl->FindWordStartFinish (m_nInitOffset, pEditCtrl->m_strBuffer, nWordStart, nWordFinish);

	int nWordLen = nWordFinish - nWordStart;

	if (nWordLen < 1 || nWordStart < 0)
	{
		SetCurSel (-1); 
		SetCaretIndex (0, TRUE);
		return;
	}

	CString str = pEditCtrl->m_strBuffer.Mid (nWordStart, nWordLen);

	if (str.IsEmpty ())
	{
		SetCurSel (-1); 
		SetCaretIndex (0, TRUE);
		return;
	}

	CBCGPIntelliSenseLB::m_bComparenoCase = TRUE;

	int iIdx = FindString (-1, str); 
	if (iIdx != -1)
	{
		SetCurSel (iIdx);
	}
	else
	{
		int nIdxPrev = 0;
		for (int i = 0; i < str.GetLength () - 1; i++)
		{
			CString strSub = str.Mid (0, i + 1);
			iIdx = FindString (nIdxPrev - 1, strSub); 
			if (iIdx == -1)
			{
				break;
			}
			nIdxPrev = iIdx;
		}
		SetCurSel (-1); 
		SetCaretIndex (nIdxPrev, TRUE);
	}

	CBCGPIntelliSenseLB::m_bComparenoCase = FALSE;
}
//*****************************************************************************************
void CBCGPIntelliSenseLB::InsertDataToEditCtrl ()
{
	CBCGPEditCtrl* pEditCtrl = GetParentEditCtrl ();
	if (pEditCtrl != NULL)
	{
		int nWordStart = 0;
		int nWordFinish = 0;

		pEditCtrl->FindWordStartFinish (m_nInitOffset, pEditCtrl->m_strBuffer, nWordStart, 
										nWordFinish);

		if (nWordFinish < m_nInitOffset)
		{
			nWordStart = nWordFinish = m_nInitOffset;
		}

		nWordStart = min (m_nInitOffset, nWordStart);

		pEditCtrl->DeleteText (nWordStart, pEditCtrl->GetCurOffset ());

        int nIdx = GetCurSel ();
        CBCGPIntelliSenseData* pData = (CBCGPIntelliSenseData*) GetItemDataPtr (nIdx);

		ASSERT_VALID (pData);
        CString strText = pData->m_strItemName;

        if (pEditCtrl->OnIntelliSenseComplete (nIdx, pData, strText))
        {
    		pEditCtrl->RemoveSelection (FALSE, TRUE, FALSE);
		    pEditCtrl->InsertText (strText, -1, TRUE, FALSE, TRUE, FALSE, TRUE);
		    pEditCtrl->m_nSavedOffset = pEditCtrl->GetCurOffset ();
        }
    }
}
//*****************************************************************************************
void CBCGPIntelliSenseLB::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	CBCGPEditCtrl* pEditCtrl = GetParentEditCtrl ();
	ASSERT_VALID (pEditCtrl);


	switch (nChar)
	{
	case VK_LEFT:
	case VK_RIGHT:
	case VK_BACK:
		{
			pEditCtrl->SendMessage (WM_KEYDOWN, nChar, MAKELPARAM (nRepCnt, nFlags));

			int nCurrOffset = pEditCtrl->GetCurOffset (); 

			CPoint pt;
			CPoint ptRowColumn;
			pEditCtrl->OffsetToPoint (nCurrOffset, pt, &ptRowColumn);

			int nWordStart = 0;
			int nWordFinish = 0;

			pEditCtrl->FindWordStartFinish (m_nInitOffset, pEditCtrl->m_strBuffer,
				                            nWordStart, nWordFinish);

			if (nCurrOffset <= m_nInitOffset - 1 || 
				nCurrOffset > nWordFinish ||
				ptRowColumn.y != m_nInitRow)
			{
				GetParent ()->PostMessage (WM_CLOSE);
			}
			
			return;
		}

	case VK_DELETE:
		pEditCtrl->SendMessage (WM_KEYDOWN, nChar, MAKELPARAM (nRepCnt, nFlags));
		GetParent ()->PostMessage (WM_CLOSE);
		return;

	case VK_ESCAPE:
		GetParent ()->PostMessage (WM_CLOSE);
		return;

	case VK_RETURN: 
	case VK_TAB:
	case VK_SPACE:		
		{
			int nIdx = GetCurSel ();
			if (nIdx != -1)
			{
				InsertDataToEditCtrl ();
				if (!::IsWindow(m_hWnd))
				{
					return;
				}
				GetParent ()->PostMessage (WM_CLOSE);
			}
			else
			{
				if (nChar == VK_TAB)
				{
					// tab selects first item
					SetCurSel (0);
					InsertDataToEditCtrl ();
					if (!::IsWindow(m_hWnd))
					{
						return;
					}
				}
				else
				{
					pEditCtrl->SendMessage (WM_KEYDOWN, nChar, MAKELPARAM (nRepCnt, nFlags));
				}
				GetParent ()->PostMessage (WM_CLOSE);
				return;
			}
		}
		break;
	}

	CBCGPBaseIntelliSenseLB::OnKeyDown(nChar, nRepCnt, nFlags);
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPSymImagesLB

BEGIN_MESSAGE_MAP(CBCGPSymImagesLB, CBCGPBaseIntelliSenseLB)
	//{{AFX_MSG_MAP(CBCGPSymImagesLB)
	ON_WM_CHAR()
	ON_WM_KEYDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//*****************************************************************************************
void CBCGPSymImagesLB::OnChar(UINT nChar, UINT /*nRepCnt*/, UINT /*nFlags*/)
{
	int nIdx = GetCurSel ();
	
	for (int i = nIdx + 1; i != nIdx; i++)
	{
		if (i == GetCount ())
		{
			if (nIdx == -1)
			{
				break;
			}
			i = -1;
			continue;
		}

		CBCGPIntelliSenseData* pData = 
			(CBCGPIntelliSenseData*) GetItemDataPtr (i);

		if (pData == NULL)
		{
			continue;
		}

		if (pData->m_strItemName.GetAt (0) == (TCHAR) nChar)
		{
			SetCurSel (i);
			break;
		}
	}
}
//*****************************************************************************************
void CBCGPSymImagesLB::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	CBCGPEditCtrl* pEditCtrl = GetParentEditCtrl ();
	ASSERT_VALID (pEditCtrl);

	switch (nChar)
	{
	case VK_ESCAPE:
		GetParent ()->PostMessage (WM_CLOSE);
		return;

	case VK_RETURN: 
	case VK_TAB:
	case VK_SPACE:		
		{
			int nIdx = GetCurSel ();
			if (nIdx != -1)
			{
				InsertDataToEditCtrl ();
				if (!::IsWindow(m_hWnd))
				{
					return;
				}

				GetParent ()->PostMessage (WM_CLOSE);
			}
			else
			{
				if (nChar == VK_TAB)
				{
					// tab selects first item
					SetCurSel (0);
					InsertDataToEditCtrl ();
					if (!::IsWindow(m_hWnd))
					{
						return;
					}

				}
				else
				{
					pEditCtrl->SendMessage (WM_KEYDOWN, nChar, MAKELPARAM (nRepCnt, nFlags));
				}
				GetParent ()->PostMessage (WM_CLOSE);
				return;
			}
		}
		break;
	}

	CBCGPBaseIntelliSenseLB::OnKeyDown(nChar, nRepCnt, nFlags);
}
//*****************************************************************************************
void CBCGPSymImagesLB::InsertDataToEditCtrl ()
{
	CBCGPEditCtrl* pEditCtrl = GetParentEditCtrl ();
	ASSERT_VALID (pEditCtrl);

	if (pEditCtrl != NULL)
	{
		int nIdx = GetCurSel ();

		CBCGPIntelliSenseData* pData = 
			(CBCGPIntelliSenseData*) GetItemDataPtr (nIdx);
 
		ASSERT_VALID (pData);
		pEditCtrl->RemoveSelection (FALSE, TRUE, FALSE);
		ASSERT (pData->m_dwData != 0);
		pEditCtrl->InsertText (CString ((TCHAR)pData->m_dwData), -1, TRUE, FALSE, TRUE, FALSE, TRUE);
	}
}

#endif	// BCGP_EXCLUDE_EDIT_CTRL
