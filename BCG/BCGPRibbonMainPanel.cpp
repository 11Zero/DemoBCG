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
// BCGPRibbonMainPanel.cpp: implementation of the CBCGPRibbonMainPanel class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "bcgcbpro.h"
#include "BCGPRibbonBar.h"
#include "BCGPRibbonMainPanel.h"
#include "BCGPRibbonButtonsGroup.h"
#include "BCGPRibbonCategory.h"
#include "BCGPVisualManager.h"
#include "BCGPRibbonLabel.h"
#include "BCGPRibbonPanelMenu.h"
#include "BCGPRibbonEdit.h"
#include "BCGPWorkspace.h"
#include "BCGPLocalResource.h"
#include "BCGProRes.h"

#ifndef BCGP_EXCLUDE_RIBBON

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define LABEL_MARGIN_X				8
#define LABEL_MARGIN_Y				4
#define BACKSTAGE_BUTTON_MARGIN_X	6
#define PINNED_FILE_ID				(UINT)-2

extern CBCGPWorkspace*	g_pWorkspace;

//////////////////////////////////////////////////////////////////////////////
// CBCGPRecentFileLabel

class CBCGPRecentFileLabel : public CBCGPRibbonLabel
{
	DECLARE_DYNCREATE(CBCGPRecentFileLabel)

	CBCGPRecentFileLabel()
	{
	}

	CBCGPRecentFileLabel(LPCTSTR lpszText) : CBCGPRibbonLabel(lpszText)
	{
	}

protected:

	virtual CSize GetRegularSize (CDC* pDC)
	{
		CSize size = CBCGPRibbonButton::GetRegularSize(pDC);
		size.cy = globalData.GetTextHeight() + 2 * LABEL_MARGIN_Y - 1;
		return size;
	}

	virtual void OnDraw (CDC* pDC)
	{
		ASSERT_VALID (this);
		ASSERT_VALID (pDC);

		if (m_rect.IsRectEmpty ())
		{
			return;
		}

		CRect rectText = m_rect;
		rectText.DeflateRect (LABEL_MARGIN_X, 0);

		CFont* pOldFont = pDC->SelectObject (&globalData.fontBold);;
		ASSERT_VALID (pOldFont);

		DoDrawText (pDC, m_strText, rectText, DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX);

		pDC->SelectObject (pOldFont);

		CRect rectSeparator = m_rect;
		rectSeparator.top = rectSeparator.bottom - 2;

		if (m_pParentMenu != NULL)
		{
			CBCGPVisualManager::GetInstance ()->OnDrawSeparator(pDC, m_pParentMenu, rectSeparator, FALSE);
		}
	}
};

IMPLEMENT_DYNCREATE(CBCGPRecentFileLabel, CBCGPRibbonLabel)

//////////////////////////////////////////////////////////////////////////////
// CBCGPRecentFileButton

class CBCGPRecentFileButton : public CBCGPRibbonButton
{
	friend class CBCGPRibbonRecentFilesList;

	DECLARE_DYNCREATE(CBCGPRecentFileButton)

public:
	CBCGPRecentFileButton()
	{
		m_bHasPin = FALSE;
		m_bIsPinned = FALSE;
		m_bPinIsDark = TRUE;
	}

protected:
	virtual BOOL CanBeAddedToQAT () const
	{
		return FALSE;
	}

	virtual CSize GetRegularSize (CDC* pDC)
	{
		CSize size = CBCGPRibbonButton::GetRegularSize(pDC);

		if (IsBackstageViewMode())
		{
			size.cy = 100;
		}
		else
		{
			size.cy = globalData.GetTextHeight() + 2 * LABEL_MARGIN_Y - 1;
		}
		return size;
	}

	virtual int DoDrawText (CDC* pDC, const CString& strText, CRect rectText, UINT uiDTFlags,
							COLORREF clrText = (COLORREF)-1)
	{
		rectText.DeflateRect (LABEL_MARGIN_X - 1, 0);
		rectText.right -= m_rectMenu.Width();

		return CBCGPRibbonButton::DoDrawText (pDC, strText, rectText, uiDTFlags, clrText);
	}

	virtual BOOL HasPin() const
	{
		return m_bHasPin;
	}

	virtual void CopyFrom(const CBCGPBaseRibbonElement& s)
	{
		ASSERT_VALID (this);

		CBCGPRibbonButton::CopyFrom (s);

		CBCGPRecentFileButton& src = (CBCGPRecentFileButton&) s;
		
		m_bHasPin = src.m_bHasPin;
		m_bIsPinned = src.m_bIsPinned;
	}

	virtual void OnDraw (CDC* pDC)
	{
		CRect rectSaved = m_rect;
		m_rect.DeflateRect(1, 1);

		CBCGPRibbonButton::OnDraw(pDC);

		m_rect = rectSaved;
	}

	virtual int GetDropDownImageWidth () const
	{
		return CBCGPVisualManager::GetInstance ()->GetPinSize(TRUE).cx + 8;
	}

	virtual void OnDrawMenuArrow(CDC* pDC, const CRect& rectMenuArrow)
	{
		CSize sizePin = CBCGPVisualManager::GetInstance ()->GetPinSize(m_bIsPinned);
		CRect rectPinImage(
			CPoint(
				rectMenuArrow.CenterPoint().x - sizePin.cx / 2,
				rectMenuArrow.CenterPoint().y - sizePin.cy / 2),
			sizePin);

		CBCGPVisualManager::GetInstance()->OnDrawPin(pDC, rectPinImage, m_bIsPinned, m_bPinIsDark,
			IsHighlighted(), IsPressed(), IsDisabled());
	}

	virtual void OnLButtonDown (CPoint point)
	{
		if (m_bHasPin && m_rectMenu.PtInRect (point))
		{
			return;
		}

		CBCGPRibbonButton::OnLButtonDown(point);
	}

	virtual void OnLButtonUp (CPoint point)
	{
		if (m_bHasPin && m_rectMenu.PtInRect (point))
		{
			m_bIsPinned = !m_bIsPinned;
			Redraw();

			if (g_pWorkspace != NULL)
			{
				g_pWorkspace->PinPath(TRUE, GetToolTip(), m_bIsPinned);
			}

			return;
		}

		CBCGPRibbonButton::OnLButtonUp(point);
	}

	virtual CString GetToolTipText () const
	{
		if (m_bHasPin && IsMenuAreaHighlighted())
		{
			CBCGPLocalResource locaRes;
			CString strTT;

			strTT.LoadString(m_bIsPinned ? IDS_BCGBARRES_UNPIN_ITEM : IDS_BCGBARRES_PIN_ITEM);
			return strTT;
		}

		return CBCGPRibbonButton::GetToolTipText();
	}

	virtual BOOL IsDroppedDown () const
	{
		return FALSE;
	}

	virtual COLORREF OnFillBackground (CDC* pDC)
	{
		if (!m_bHasPin)
		{
			return CBCGPRibbonButton::OnFillBackground(pDC);
		}

		return CBCGPVisualManager::GetInstance ()->OnFillRibbonPinnedButton(pDC, this, m_bPinIsDark);
	}

	virtual void OnDrawBorder (CDC* pDC)
	{
		if (!m_bHasPin)
		{
			CBCGPRibbonButton::OnDrawBorder(pDC);
			return;
		}

		CBCGPVisualManager::GetInstance ()->OnDrawRibbonPinnedButtonBorder (pDC, this);
	}

	virtual BOOL NotifyCommand (BOOL bWithDelay = FALSE)
	{
		if (m_nID != PINNED_FILE_ID || AfxGetApp() == NULL)
		{
			return CBCGPRibbonButton::NotifyCommand(bWithDelay);
		}

		AfxGetApp()->OpenDocumentFile(GetToolTip());
		return TRUE;
	}

	BOOL	m_bHasPin;
	BOOL	m_bIsPinned;
	BOOL	m_bPinIsDark;
};

IMPLEMENT_DYNCREATE(CBCGPRecentFileButton, CBCGPRibbonButton)

//////////////////////////////////////////////////////////////////////////////
// CBCGPRibbonSearchBox

class CBCGPRibbonSearchBox : public CBCGPRibbonEdit
{
	DECLARE_DYNCREATE(CBCGPRibbonSearchBox)

// Construction:
public:
	CBCGPRibbonSearchBox() :
		CBCGPRibbonEdit (0, 0)
	{
		m_bDontScaleInHighDPI = TRUE;
	}

protected:
	virtual BOOL OnEditChange ()
	{
		CBCGPRibbonMainPanel* pMainPanel = DYNAMIC_DOWNCAST(CBCGPRibbonMainPanel, GetParentPanel ());
		if (pMainPanel != NULL)
		{
			pMainPanel->OnSearch (m_strEdit);
		}
		return FALSE;
	}

	virtual BOOL OnProcessKey (UINT nChar)
	{
		CBCGPRibbonMainPanel* pMainPanel = DYNAMIC_DOWNCAST(CBCGPRibbonMainPanel, GetParentPanel ());
		if (pMainPanel != NULL && pMainPanel->GetHighlighted () != this)
		{
			CBCGPBaseRibbonElement* pHighlighted = pMainPanel->GetHighlighted ();
			if (pHighlighted != NULL && pHighlighted->IsDroppedDown())
			{
				return FALSE;
			}

			return pMainPanel->OnSearchNavigate (nChar);
		}

		return FALSE;
	}
};

IMPLEMENT_DYNCREATE(CBCGPRibbonSearchBox, CBCGPRibbonEdit)

//////////////////////////////////////////////////////////////////////////////
// CBCGPRibbonRecentFilesList

#define SEPARATOR_HEIGHT	4

IMPLEMENT_DYNCREATE(CBCGPRibbonRecentFilesList, CBCGPRibbonButtonsGroup)

void CBCGPRibbonRecentFilesList::OnAfterChangeRect (CDC* pDC)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);

	if (GetCount () == 0)
	{
		FillList ();
	}

	int y = m_rect.top + 2;

	for (int i = 0; i < m_arButtons.GetSize (); i++)
	{
		CBCGPBaseRibbonElement* pButton = m_arButtons [i];
		ASSERT_VALID (pButton);

		pButton->SetParentMenu (m_pParentMenu);

		pButton->OnCalcTextSize (pDC);
		CSize sizeButton = pButton->GetSize (pDC);

		CRect rectButton = m_rect;
		rectButton.DeflateRect (1, 0);

		rectButton.top = y;
		rectButton.bottom = y + sizeButton.cy;

		pButton->SetRect (rectButton);
		pButton->OnAfterChangeRect (pDC);

		y = rectButton.bottom;
	}
}
//*****************************************************************************
void CBCGPRibbonRecentFilesList::CopyFrom (const CBCGPBaseRibbonElement& s)
{
	ASSERT_VALID (this);

	CBCGPRibbonButtonsGroup::CopyFrom (s);

	CBCGPRibbonRecentFilesList& src = (CBCGPRibbonRecentFilesList&) s;
	m_bShowPins = src.m_bShowPins;
}
//**********************************************************************************
CSize CBCGPRibbonRecentFilesList::GetRegularSize (CDC* pDC)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);

	int cy = 4;

	for (int i = 0; i < m_arButtons.GetSize (); i++)
	{
		CBCGPBaseRibbonElement* pButton = m_arButtons [i];
		ASSERT_VALID (pButton);

		pButton->OnCalcTextSize (pDC);
		CSize sizeButton = pButton->GetSize (pDC);

		cy += sizeButton.cy;
	}

	const int nDefaultSize = 300;

	return CSize (
		globalData.GetRibbonImageScale () == 1. ? nDefaultSize : (int) (globalData.GetRibbonImageScale () *  nDefaultSize), 
		cy);
}

//-----------------------------------------------------
// My "classic " trick - how I can access to protected
// member m_pRecentFileList?
//-----------------------------------------------------
class CBCGPApp : public CWinApp
{
	friend class CBCGPRibbonRecentFilesList;
};

static CString PrepareDisplayName(int nIndex, const CString & strName)
{
	CString strOut;

	if (nIndex == 9)
	{
		strOut.Format (_T("1&0 %s"), strName);
	}
	else if (nIndex < 9)
	{
		strOut.Format (_T("&%d %s"), nIndex + 1, strName);
	}
	else
	{
		strOut = strName;
	}

	return strOut;
}

void CBCGPRibbonRecentFilesList::FillList ()
{
	ASSERT_VALID (this);

	RemoveAll ();

	//-----------
	// Add label:
	//-----------
	AddButton (new CBCGPRecentFileLabel (m_strText));

	int iNumOfFiles = 0;	// Actual added to menu

	TCHAR szCurDir [_MAX_PATH + 1];
	::GetCurrentDirectory (_MAX_PATH, szCurDir);

	int nCurDir = lstrlen (szCurDir);
	ASSERT (nCurDir >= 0);

	szCurDir [nCurDir] = _T('\\');
	szCurDir [++ nCurDir] = _T('\0');

	//--------------------------
	// Add "pinned" items first:
	//--------------------------
	if (g_pWorkspace != NULL && m_bShowPins)
	{
		const CStringArray& ar = g_pWorkspace->GetPinnedPaths(TRUE);

		CString strCurrDir = szCurDir;
		strCurrDir.MakeUpper();

		for (int i = 0; i < (int)ar.GetSize(); i++)
		{
			CBCGPRecentFileButton* pFile = new CBCGPRecentFileButton;

			CString strName = ar[i];
			CString strNameUpper = strName;
			strNameUpper.MakeUpper();

			if (strNameUpper.Find(strCurrDir) == 0)
			{
				strName = strName.Mid(strCurrDir.GetLength());
			}

			pFile->SetText (PrepareDisplayName(iNumOfFiles, strName));
			pFile->SetID (PINNED_FILE_ID);
			pFile->SetToolTipText(ar[i]);
			pFile->m_bHasPin = TRUE;
			pFile->m_bIsPinned = TRUE;

			AddButton (pFile);

			iNumOfFiles++;
		}
	}

	//---------------
	// Add MRU items:
	//---------------
	CRecentFileList* pMRUFiles = 
		((CBCGPApp*) AfxGetApp ())->m_pRecentFileList;

	if (pMRUFiles != NULL)
	{
		for (int i = 0; i < pMRUFiles->GetSize (); i++)
		{
			CString strName;
			if (pMRUFiles->GetDisplayName (strName, i, szCurDir, nCurDir))
			{
				CString strPath = (*pMRUFiles)[i];
				BOOL bAlreadyExist = FALSE;

				for (int j = 0; j < m_arButtons.GetSize(); j++)
				{
					if (strPath.CompareNoCase(m_arButtons[j]->GetToolTip()) == 0)
					{
						bAlreadyExist = TRUE;
						break;
					}
				}

				if (!bAlreadyExist)
				{
					CBCGPRecentFileButton* pFile = new CBCGPRecentFileButton;
					
					pFile->SetText (PrepareDisplayName(iNumOfFiles, strName));
					pFile->SetID (ID_FILE_MRU_FILE1 + i);
					pFile->SetToolTipText (strPath);
					pFile->m_bHasPin = m_bShowPins;

					AddButton (pFile);

					iNumOfFiles++;
				}
			}
		}
	}
}
//********************************************************************************
void CBCGPRibbonRecentFilesList::OnDraw (CDC* pDC)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);

	if (m_rect.IsRectEmpty ())
	{
		return;
	}

	for (int i = 0; i < m_arButtons.GetSize (); i++)
	{
		CBCGPBaseRibbonElement* pButton = m_arButtons [i];
		ASSERT_VALID (pButton);

		pButton->OnDraw (pDC);
	}
}
//********************************************************************************
BOOL CBCGPRibbonRecentFilesList::OnMenuKey (UINT nUpperChar)
{
	ASSERT_VALID (this);

	for (int i = 0; i < m_arButtons.GetSize (); i++)
	{
		CBCGPRibbonButton* pButton = DYNAMIC_DOWNCAST (
			CBCGPRibbonButton, m_arButtons [i]);

		if (pButton == NULL)
		{
			continue;
		}

		ASSERT_VALID (pButton);

		CString strLabel = pButton->GetText ();

		int iAmpOffset = strLabel.Find (_T('&'));
		if (iAmpOffset >= 0 && iAmpOffset < strLabel.GetLength () - 1)
		{
			TCHAR szChar [2] = { strLabel.GetAt (iAmpOffset + 1), '\0' };
			CharUpper (szChar);

			if ((UINT) (szChar [0]) == nUpperChar && !pButton->IsDisabled ())
			{
				pButton->OnClick (pButton->GetRect ().TopLeft ());
				return TRUE;
			}
		}
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////////////
// CBCGPRibbonMainPanel

IMPLEMENT_DYNCREATE (CBCGPRibbonMainPanel, CBCGPRibbonPanel)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGPRibbonMainPanel::CBCGPRibbonMainPanel()
{
	m_nBottomElementsNum = 0;
	m_nTopMargin = 0;
	m_pElemOnRight = NULL;
	m_pSearchBox = NULL;
	m_nSearchBoxWidth = 0;
	m_bHideDisabledInSearchResult = FALSE;
	m_nRightPaneWidth = 0;
	m_nSearchResults = 0;
	m_bSearchMode = FALSE;
	m_bReposAfterSearch = FALSE;
	m_bMenuMode = TRUE;
	m_pMainButton = NULL;
	m_nTotalHeight = 0;

	m_rectMenuElements.SetRectEmpty ();
	m_rectScrollCorner.SetRectEmpty();
}
//********************************************************************************
CBCGPRibbonMainPanel::~CBCGPRibbonMainPanel()
{
}
//********************************************************************************
void CBCGPRibbonMainPanel::RecalcWidths (CDC* pDC, int /*nHeight*/)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);

	if (m_pSearchBox != NULL)
	{
		ASSERT_VALID(m_pSearchBox);
		m_pSearchBox->SetEditText (_T(""));
	}

	m_arWidths.RemoveAll ();
	m_nCurrWidthIndex = 0;
	m_bIsCalcWidth = TRUE;

	Repos (pDC, CRect (0, 0, 32767, 32767));
	m_arWidths.Add (m_nFullWidth);

	m_bIsCalcWidth = FALSE;
	m_bReposAfterSearch = FALSE;
}
//********************************************************************************
void CBCGPRibbonMainPanel::Repos (CDC* pDC, const CRect& rect)
{
	ASSERT_VALID (pDC);

	m_nTotalHeight = 0;

	CSize size = rect.Size ();
	size.cx -= m_nXMargin;
	size.cy -= m_nYMargin;

	int nTopMargin = m_nTopMargin;
	
	if (IsBackstageView())
	{
		nTopMargin -= m_nScrollOffset;
	}

	int y = m_bSearchMode ? m_rectMenuElements.top : nTopMargin;
	int i = 0;

	const int nMenuElements = GetMenuElements ();

	if (!m_bReposAfterSearch)
	{
		m_rectMenuElements = rect;
		m_rectMenuElements.OffsetRect(-m_nScrollOffsetHorz, 0);

		if (!IsBackstageView())
		{
			m_rectMenuElements.DeflateRect (m_nXMargin, m_nYMargin);
			m_rectMenuElements.top += m_nTopMargin;
		}
	}

	int nImageWidth = 0;

	if (m_pParent != NULL)
	{
		ASSERT_VALID (m_pParent);
		nImageWidth = m_pParent->GetImageSize (TRUE).cx;
	}

	if (m_bSearchMode)
	{
		for (i = 0; i < nMenuElements; i++)
		{
			CBCGPBaseRibbonElement* pElem = m_arElements [i];
			ASSERT_VALID (pElem);

			pElem->SetRect (CRect (0, 0, 0, 0));
		}
	}

	//----------------------------------------
	// Repos menu elements (on the left side):
	//----------------------------------------
	int nColumnWidth = m_nSearchBoxWidth;
	int iMenuStart = m_bSearchMode ? (int)m_arElements.GetSize () - m_nSearchResults : 0;
	int iMenuFinish = m_bSearchMode ? (int)m_arElements.GetSize () : nMenuElements;

	for (i = iMenuStart; i < iMenuFinish; i++)
	{
		CBCGPBaseRibbonElement* pElem = m_arElements [i];
		ASSERT_VALID (pElem);

		if (pElem->IsBackstageViewMode() && pElem->HasSubitems())
		{
			pElem->m_nExtraMaginX = BACKSTAGE_BUTTON_MARGIN_X;
		}

		pElem->OnCalcTextSize (pDC);
		pElem->SetTextAlwaysOnRight ();

		CSize sizeElem = pElem->GetSize (pDC);

		if (sizeElem == CSize (0, 0))
		{
			pElem->SetRect (CRect (0, 0, 0, 0));
			continue;
		}

		if (pElem->IsBackstageViewMode() && !pElem->HasSubitems())
		{
			sizeElem.cx += 2 * BACKSTAGE_BUTTON_MARGIN_X;
		}

		CRect rectElem = CRect
			(CPoint (rect.left - m_nScrollOffsetHorz + m_nXMargin, rect.top + y + m_nYMargin), 
			sizeElem);

		if (m_bSearchMode && rectElem.bottom > m_rectMenuElements.bottom)
		{
			rectElem = CRect (0, 0, 0, 0);
		}
		else if (pElem->IsBackstageViewMode() && !pElem->HasSubitems())
		{
			rectElem.DeflateRect(BACKSTAGE_BUTTON_MARGIN_X, 0);
		}

		pElem->SetRect (rectElem);

		nColumnWidth = max (nColumnWidth, sizeElem.cx);
		y += sizeElem.cy;

		if (IsBackstageView())
		{
			y += 2;
		}

	}

	nColumnWidth += 2 * m_nXMargin;

	if (!m_bReposAfterSearch)
	{
		m_rectMenuElements.right = m_rectMenuElements.left + nColumnWidth;

		if (IsBackstageView())
		{
			m_rectMenuElements.right += 2 * m_nXMargin;
		}
		else
		{
			m_rectMenuElements.bottom = y + m_nYMargin;
			m_rectMenuElements.InflateRect (1, 1);
		}

		m_nFullWidth = nColumnWidth + 2 * m_nXMargin;
	}

	//----------------------------------------------
	// All menu elements should have the same width:
	//----------------------------------------------
	for (i = iMenuStart; i < iMenuFinish; i++)
	{
		CBCGPBaseRibbonElement* pElem = m_arElements [i];
		ASSERT_VALID (pElem);

		CRect rectElem = pElem->GetRect ();

		if (!rectElem.IsRectEmpty ())
		{
			if (m_bSearchMode)
			{
				rectElem.right = m_rectMenuElements.right;
			}
			else
			{
				rectElem.right = rectElem.left + nColumnWidth;

				if (pElem->IsBackstageViewMode() && !pElem->HasSubitems())
				{
					rectElem.right -= 2 * BACKSTAGE_BUTTON_MARGIN_X;
				}
			}
			
			if (nImageWidth > 0 && 
				pElem->IsKindOf (RUNTIME_CLASS (CBCGPRibbonSeparator)))
			{
				rectElem.left += nImageWidth + LABEL_MARGIN_Y;
			}

			pElem->SetRect (rectElem);
		}
	}

	//----------------------
	// Put element on right:
	//----------------------
	if (m_pElemOnRight != NULL && !m_bSearchMode)
	{
		CBCGPRibbonRecentFilesList* pRecentList = 
			DYNAMIC_DOWNCAST (CBCGPRibbonRecentFilesList, m_pElemOnRight);

		if (pRecentList != NULL)
		{
			ASSERT_VALID (pRecentList);

			if (pRecentList->GetCount () == 0)
			{
				pRecentList->FillList ();
			}
		}

		m_pElemOnRight->SetInitialMode ();
		m_pElemOnRight->OnCalcTextSize (pDC);
		
		CSize sizeRecentList = m_pElemOnRight->GetSize (pDC);

		int nDefaultWidth =
			globalData.GetRibbonImageScale () == 1. ? m_nRightPaneWidth : (int) (globalData.GetRibbonImageScale () *  m_nRightPaneWidth);

		sizeRecentList.cx = max (sizeRecentList.cx, nDefaultWidth);

		if (m_rectMenuElements.Height () < sizeRecentList.cy && !IsBackstageView())
		{
			m_rectMenuElements.bottom = m_rectMenuElements.top + sizeRecentList.cy;
		}

		CRect rectRecentList = CRect
			(m_rectMenuElements.right, m_rectMenuElements.top, 
			m_rectMenuElements.right + sizeRecentList.cx, m_rectMenuElements.bottom);

		if (pRecentList == NULL)
		{
			rectRecentList.DeflateRect (0, 1);
		}

		m_pElemOnRight->SetRect (rectRecentList);

		if (!m_bReposAfterSearch)
		{
			m_nFullWidth += sizeRecentList.cx;
		}
	}

	//---------------------------------
	// Put "bottom" elements on bottom:
	//---------------------------------
	if (m_nBottomElementsNum > 0 && !m_bSearchMode)
	{
		int x = rect.left + m_nFullWidth - m_nXMargin;
		int nRowHeight = 0;

		y = m_rectMenuElements.bottom + m_nYMargin;

		int xLeft = rect.left + m_nXMargin;

		if (m_pSearchBox != NULL)
		{
			ASSERT_VALID(m_pSearchBox);

			m_pSearchBox->OnCalcTextSize (pDC);

			int xMargin = m_pParent == NULL ? 0 : max(0, 4 - CBCGPVisualManager::GetInstance()->GetRibbonPanelMargin(m_pParent));

			m_pSearchBox->SetWidth (m_rectMenuElements.Width () - xMargin);

			CSize sizeElem = m_pSearchBox->GetSize (pDC);
			sizeElem.cx = m_rectMenuElements.Width () - xMargin;

			CRect rectElem = CRect (CPoint (m_rectMenuElements.left + xMargin, y), sizeElem);
			m_pSearchBox->SetRect (rectElem);

			nRowHeight = max (nRowHeight, sizeElem.cy);
			xLeft = m_rectMenuElements.left + sizeElem.cx;
		}

		for (int nCount = 0; nCount < m_nBottomElementsNum; nCount++)
		{
			int nIndex = (int) m_arElements.GetSize () - nCount - 1 - m_nSearchResults;

			CBCGPBaseRibbonElement* pElem = m_arElements [nIndex];
			ASSERT_VALID (pElem);

			if (pElem == m_pSearchBox)
			{
				// Already positioned
				continue;
			}

			pElem->OnCalcTextSize (pDC);

			CSize sizeElem = pElem->GetSize (pDC);

			if (sizeElem == CSize (0, 0))
			{
				pElem->SetRect (CRect (0, 0, 0, 0));
				continue;
			}

			sizeElem.cx += LABEL_MARGIN_Y - 1;

			if (x - sizeElem.cx < xLeft)
			{
				x = rect.left + m_nFullWidth - m_nXMargin;
				y += nRowHeight; 
				nRowHeight = 0;
				xLeft = rect.left + m_nXMargin;
			}

			CRect rectElem = CRect (CPoint (x - sizeElem.cx, y), sizeElem);
			pElem->SetRect (rectElem);

			nRowHeight = max (nRowHeight, sizeElem.cy);
			x = rectElem.left - LABEL_MARGIN_Y;
		}

		y += nRowHeight;
	}
	else if (m_nBottomElementsNum == 0)
	{
		y += globalData.GetTextHeight();
	}

	for (i = 0; i < m_arElements.GetSize (); i++)
	{
		CBCGPBaseRibbonElement* pElem = m_arElements [i];
		ASSERT_VALID (pElem);

		pElem->OnAfterChangeRect (pDC);
	}

	m_nTotalHeight = y + m_nScrollOffset;

	if (!m_bReposAfterSearch)
	{
		m_rect = rect;

		if (IsBackstageView())
		{
			m_nFullWidth = m_rect.Width();
			m_rectMenuElements.bottom = m_rect.bottom + 3;
		}
		else
		{
			m_rect.bottom = m_rect.top + y + m_nYMargin;
			m_rect.right = m_rect.left + m_nFullWidth + m_nXMargin;
		}
	}
}
//**************************************************************************************
void CBCGPRibbonMainPanel::AddRecentFilesList (LPCTSTR lpszLabel, int nWidth, BOOL bShowPins)
{
	ASSERT_VALID (this);
	ASSERT (lpszLabel != NULL);

	if (IsBackstageView())
	{
		ASSERT(FALSE);
		return;
	}

	AddToRight (new CBCGPRibbonRecentFilesList (lpszLabel, bShowPins), nWidth);
}
//**************************************************************************************
void CBCGPRibbonMainPanel::AddToRight (CBCGPBaseRibbonElement* pElem, int nWidth)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pElem);

	if (IsBackstageView())
	{
		ASSERT(FALSE);
		return;
	}

	if (m_pElemOnRight != NULL)
	{
		// Already exist, delete previous
		m_arElements.RemoveAt (GetMenuElements ());

		ASSERT_VALID (m_pElemOnRight);
		delete m_pElemOnRight;

		m_pElemOnRight = NULL;
	}

	pElem->SetParentCategory (m_pParent);

	m_arElements.InsertAt (GetMenuElements (), pElem);
	
	m_pElemOnRight = pElem;
	m_nRightPaneWidth = nWidth;
}
//**************************************************************************************
void CBCGPRibbonMainPanel::AddToBottom (CBCGPRibbonMainPanelButton* pElem)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pElem);

	if (IsBackstageView())
	{
		ASSERT(FALSE);
		return;
	}

	m_nBottomElementsNum++;

	pElem->SetParentCategory (m_pParent);
	m_arElements.Add (pElem);
}
//********************************************************************************
void CBCGPRibbonMainPanel::Add (CBCGPBaseRibbonElement* pElem)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pElem);

	pElem->SetParentCategory (m_pParent);
	m_arElements.InsertAt (GetMenuElements (), pElem);
}
//********************************************************************************
int CBCGPRibbonMainPanel::GetMenuElements () const
{
	ASSERT_VALID (this);

	int nMenuElements = (int) m_arElements.GetSize () - m_nBottomElementsNum - m_nSearchResults;
	if (m_pElemOnRight != NULL)
	{
		nMenuElements--;
	}

	ASSERT (nMenuElements >= 0);
	return nMenuElements;
}
//********************************************************************************
void CBCGPRibbonMainPanel::CopyFrom (CBCGPRibbonPanel& s)
{
	ASSERT_VALID (this);

	CBCGPRibbonPanel::CopyFrom (s);

	CBCGPRibbonMainPanel& src = (CBCGPRibbonMainPanel&) s;

	m_nBottomElementsNum = src.m_nBottomElementsNum;
	m_nTopMargin = src.m_nTopMargin;
	m_pMainButton = src.m_pMainButton;
	m_nSearchBoxWidth = src.m_nSearchBoxWidth;
	m_bHideDisabledInSearchResult = src.m_bHideDisabledInSearchResult;

	m_pElemOnRight = NULL;
	m_pSearchBox = NULL;

	m_nRightPaneWidth = src.m_nRightPaneWidth;

	if (src.m_pElemOnRight != NULL)
	{
		ASSERT_VALID (src.m_pElemOnRight);

		for (int i = 0; i < src.m_arElements.GetSize (); i++)
		{
			if (src.m_arElements [i] == src.m_pElemOnRight)
			{
				m_pElemOnRight = m_arElements [i];
				break;
			}
		}

		ASSERT_VALID (m_pElemOnRight);

		CBCGPRibbonRecentFilesList* pRecentList = 
			DYNAMIC_DOWNCAST (CBCGPRibbonRecentFilesList, m_pElemOnRight);

		if (pRecentList != NULL)
		{
			ASSERT_VALID (pRecentList);
			pRecentList->RemoveAll ();
		}
	}

	if (src.m_pSearchBox != NULL)
	{
		ASSERT_VALID (src.m_pSearchBox);

		for (int i = 0; i < src.m_arElements.GetSize (); i++)
		{
			if (src.m_arElements [i] == src.m_pSearchBox)
			{
				m_pSearchBox = DYNAMIC_DOWNCAST(CBCGPRibbonSearchBox, m_arElements [i]);
				break;
			}
		}
	}
}
//********************************************************************************
BOOL CBCGPRibbonMainPanel::GetPreferedMenuLocation (CRect& rect)
{
	ASSERT_VALID (this);

	if (m_pElemOnRight == NULL)
	{
		return FALSE;
	}

	ASSERT_VALID (m_pElemOnRight);

	rect = m_pElemOnRight->GetRect ();
	rect.DeflateRect (1, 1);

	const int nShadowSize = CBCGPMenuBar::IsMenuShadows () &&
					!CBCGPToolBar::IsCustomizeMode () &&
					globalData.m_nBitsPerPixel > 8 ? // Don't draw shadows in 256 colors or less
						CBCGPVisualManager::GetInstance ()->GetMenuShadowDepth () : 0;


	rect.right -= nShadowSize + 3;
	rect.bottom -= nShadowSize + 3;

	return TRUE;
}
//********************************************************************************
void CBCGPRibbonMainPanel::DoPaint (CDC* pDC)
{
	ASSERT_VALID (pDC);

	if (m_rect.IsRectEmpty ())
	{
		return;
	}

	CRect rectClip;
	pDC->GetClipBox (rectClip);

	CRect rectInter;

	if (!rectInter.IntersectRect (m_rect, rectClip))
	{
		return;
	}

	COLORREF clrTextOld = pDC->GetTextColor ();

	//-----------------------
	// Fill panel background:
	//-----------------------
	COLORREF clrText = CBCGPVisualManager::GetInstance ()->OnDrawRibbonPanel (pDC, this, m_rect, CRect (0, 0, 0, 0));

	DrawMainButton (pDC, GetParentWnd ());

	if (!IsBackstageView())
	{
		CRect rectRecentFiles;
		rectRecentFiles.SetRectEmpty ();

		if (m_pElemOnRight != NULL)
		{
			ASSERT_VALID (m_pElemOnRight);

			rectRecentFiles = m_pElemOnRight->GetRect ();

			CBCGPVisualManager::GetInstance ()->OnDrawRibbonRecentFilesFrame (
				pDC, this, rectRecentFiles);
		}

		CRect rectFrame = m_rectMenuElements;
		if (!rectRecentFiles.IsRectEmpty ())
		{
			rectFrame.right = rectRecentFiles.right;
		}

		CBCGPVisualManager::GetInstance ()->OnFillRibbonMenuFrame (pDC, this, m_rectMenuElements);
		CBCGPVisualManager::GetInstance ()->OnDrawRibbonMainPanelFrame (pDC, this, rectFrame);
	}
	else if (!IsBackstageRightPaneActive())
	{
		CRect rectRight = m_rect;
		rectRight.left = m_rectMenuElements.right;

		CBCGPVisualManager::GetInstance ()->OnFillRibbonMenuFrame (pDC, this, rectRight);
	}

	pDC->SetTextColor (clrText);

	//---------------------
	// Draw panel elements:
	//---------------------
	for (int i = 0; i < m_arElements.GetSize (); i++)
	{
		CBCGPBaseRibbonElement* pElem = m_arElements [i];
		ASSERT_VALID (pElem);

		if (rectInter.IntersectRect (pElem->GetRect (), rectClip))
		{
			pDC->SetTextColor (clrText);
			pElem->OnDraw (pDC);
		}

		CBCGPBaseRibbonElement* pView = pElem->GetBackstageAttachedView();
		if (pView != NULL)
		{
			ASSERT_VALID(pView);
			
			if (!pView->GetRect().IsRectEmpty())
			{
				pView->OnDraw (pDC);
			}
		}
	}

	if (!m_rectScrollCorner.IsRectEmpty())
	{
		::FillRect(pDC->GetSafeHdc(), m_rectScrollCorner, (HBRUSH)::GetStockObject(WHITE_BRUSH));
	}

	pDC->SetTextColor (clrTextOld);
}
//**********************************************************************************
CRect CBCGPRibbonMainPanel::GetCommandsFrame () const
{
	ASSERT_VALID (this);

	CRect rectFrame = m_rectMenuElements;

	if (m_pElemOnRight != NULL)
	{
		ASSERT_VALID (m_pElemOnRight);

		CRect rectRecentFiles = m_pElemOnRight->GetRect ();
		if (!rectRecentFiles.IsRectEmpty ())
		{
			rectFrame.right = rectRecentFiles.right;
		}
	}

	return rectFrame;
}
//********************************************************************************
void CBCGPRibbonMainPanel::OnDrawMenuBorder (CDC* pDC, CBCGPRibbonPanelMenuBar* pMenuBar)
{
	ASSERT_VALID (pMenuBar);

	if (m_pMainButton != NULL && m_pMainButton->GetParentRibbonBar () != NULL &&
		!m_pMainButton->GetParentRibbonBar ()->IsScenicLook())
	{
		DrawMainButton (pDC, pMenuBar->GetParent ());
	}
}
//********************************************************************************
void CBCGPRibbonMainPanel::DrawMainButton (CDC* pDC, CWnd* pWnd)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);
	ASSERT_VALID (pWnd);

	if (m_pMainButton == NULL)
	{
		return;
	}

	ASSERT_VALID (m_pMainButton);
	ASSERT_VALID (m_pMainButton->GetParentRibbonBar ());

	CRect rectMainButtonSaved = m_pMainButton->GetRect ();
	CRect rectMainButton = rectMainButtonSaved;

	m_pMainButton->GetParentRibbonBar ()->ClientToScreen (&rectMainButton);
	pWnd->ScreenToClient (&rectMainButton);

	if (rectMainButton.top > m_rectMenuElements.bottom)
	{
		return;
	}

	m_pMainButton->SetRect (rectMainButton);

	CBCGPVisualManager::GetInstance ()->OnDrawRibbonMainButton(pDC, m_pMainButton);

	m_pMainButton->OnDraw (pDC);

	m_pMainButton->SetRect (rectMainButtonSaved);
}
//*****************************************************************************
CBCGPBaseRibbonElement* CBCGPRibbonMainPanel::MouseButtonDown (CPoint point)
{
	ASSERT_VALID (this);

	CBCGPBaseRibbonElement* pElement = CBCGPRibbonPanel::MouseButtonDown (point);

	if (m_pMainButton != NULL)
	{
		ASSERT_VALID (m_pMainButton);
		ASSERT_VALID (m_pMainButton->GetParentRibbonBar ());
		ASSERT_VALID (GetParentWnd ());
		
		CRect rectMainButton = m_pMainButton->GetRect ();

		m_pMainButton->GetParentRibbonBar ()->ClientToScreen (&rectMainButton);
		GetParentWnd ()->ScreenToClient (&rectMainButton);

		if (rectMainButton.PtInRect (point))
		{
			m_pMainButton->ClosePopupMenu ();
			return NULL;
		}
	}

	return pElement;
}
//*****************************************************************************
void CBCGPRibbonMainPanel::EnableCommandSearch (BOOL bEnable, LPCTSTR lpszLabel, LPCTSTR lpszKeyTip, int nWidth, BOOL bHideDisabled)
{
	ASSERT_VALID (this);

	m_bHideDisabledInSearchResult = bHideDisabled;

	if (bEnable)
	{
		ASSERT (lpszLabel != NULL);

		if (m_pSearchBox == NULL)
		{
			m_pSearchBox = new CBCGPRibbonSearchBox;
			m_pSearchBox->SetKeys (lpszKeyTip);
			m_pSearchBox->SetParentCategory (m_pParent);
			m_arElements.Add (m_pSearchBox);
			m_nBottomElementsNum++;
			m_nSearchBoxWidth = nWidth;
		}

		ASSERT_VALID (m_pSearchBox);
		m_pSearchBox->EnableSearchMode(TRUE, lpszLabel);
		return;
	}

	m_nSearchBoxWidth = 0;

	if (m_pSearchBox == NULL)
	{
		return;
	}

	ASSERT_VALID (m_pSearchBox);

	for (int i = 0; i < m_arElements.GetSize (); i++)
	{
		CBCGPBaseRibbonElement* pElem = m_arElements [i];
		ASSERT_VALID (pElem);

		if (pElem == m_pSearchBox)
		{
			m_arElements.RemoveAt (i);
			delete m_pSearchBox;
			m_pSearchBox = NULL;
			
			m_nBottomElementsNum--;
			return;
		}
	}
}
//******************************************************************************
void CBCGPRibbonMainPanel::OnSearch (const CString& str)
{
	ASSERT_VALID (this);

	if (m_pMainButton == NULL || m_pParentMenuBar == NULL)
	{
		return;
	}

	ASSERT_VALID (m_pMainButton);

	CBCGPRibbonBar* pRibbonBar = m_pMainButton->GetParentRibbonBar ();
	ASSERT_VALID (pRibbonBar);

	while (m_nSearchResults > 0)
	{
		CBCGPBaseRibbonElement* pElement = m_arElements [m_arElements.GetSize () - 1];
		if (pElement == m_pHighlighted)
		{
			m_pHighlighted = NULL;
		}

		delete pElement;
		m_arElements.RemoveAt (m_arElements.GetSize () - 1);

		m_nSearchResults--;
	}

	m_bSearchMode = FALSE;

	CBCGPBaseRibbonElement* pHighlightedElement = NULL;

	if (str.GetLength () >= 2)
	{
		m_bSearchMode = TRUE;

		CArray<CBCGPBaseRibbonElement*, CBCGPBaseRibbonElement*> arSearchResults;
		pRibbonBar->GetElementsByName (str, arSearchResults);

		for (int i = 0; i < arSearchResults.GetSize (); i++)
		{
			if (!pRibbonBar->OnFilterSearchResult(arSearchResults[i]))
			{
				continue;
			}

			if (m_bHideDisabledInSearchResult)
			{
				CFrameWnd* pTarget = (CFrameWnd*)pRibbonBar->GetOwner();
				if (pTarget == NULL || !pTarget->IsFrameWnd())
					pTarget = BCGPGetParentFrame(pRibbonBar);
				if (pTarget != NULL)
				{
					CBCGPRibbonCmdUI state;
					state.m_pOther = m_pParentMenuBar;

					arSearchResults[i]->OnUpdateCmdUI(&state, pTarget, TRUE);
					
					if (arSearchResults[i]->IsDisabled())
					{
						continue;
					}
				}
			}

			CBCGPBaseRibbonElement* pElement = (CBCGPBaseRibbonElement*)
				arSearchResults [i]->GetRuntimeClass()->CreateObject ();
			ASSERT_VALID(pElement);

			pElement->m_bSearchResultMode = TRUE;
			pElement->CopyFrom (*arSearchResults [i]);
			pElement->SetParentMenu (DYNAMIC_DOWNCAST (CBCGPRibbonPanelMenuBar, GetParentWnd ()));
			pElement->m_pOriginal = arSearchResults [i];

			if (pElement->m_strText.IsEmpty ())
			{
				pElement->m_strText = pElement->m_strToolTip;
			}

			m_arElements.Add (pElement);

			if (pHighlightedElement == NULL)
			{
				pHighlightedElement = pElement;
			}

			m_nSearchResults++;
		}
	}

	CClientDC dc (pRibbonBar);

	CFont* pOldFont = dc.SelectObject (pRibbonBar->GetFont ());
	ASSERT (pOldFont != NULL);

	m_bReposAfterSearch = TRUE;

	Repos (&dc, m_rect);

	if (pHighlightedElement != NULL)
	{
		m_pHighlighted = pHighlightedElement;
		m_pHighlighted->m_bIsHighlighted = m_pHighlighted->m_bIsFocused = TRUE;
	}

	m_bReposAfterSearch = FALSE;

	dc.SelectObject (pOldFont);

	CWnd* pParentWnd = GetParentWnd ();

	if (pParentWnd->GetSafeHwnd () != NULL)
	{
		ASSERT_VALID (pParentWnd);

		pParentWnd->Invalidate ();
		pParentWnd->UpdateWindow ();
	}
}
//******************************************************************************
BOOL CBCGPRibbonMainPanel::OnSearchNavigate (UINT nChar)
{
	if (m_bSearchMode)
	{
		switch (nChar)
		{
		case VK_UP:
		case VK_DOWN:
			m_bNavigateSearchResultsOnly = TRUE;

		case VK_RETURN:
			{
				BOOL bRes = CBCGPRibbonPanel::OnKey (nChar);
				m_bNavigateSearchResultsOnly = FALSE;
				return bRes;
			}
		}
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////////////////
// CBCGPRibbonMainPanelButton

IMPLEMENT_DYNCREATE(CBCGPRibbonMainPanelButton, CBCGPRibbonButton)

CBCGPRibbonMainPanelButton::CBCGPRibbonMainPanelButton()
{
}
//********************************************************************************
CBCGPRibbonMainPanelButton::CBCGPRibbonMainPanelButton (UINT nID, LPCTSTR lpszText, 
									  int nSmallImageIndex) :
	CBCGPRibbonButton (nID, lpszText, nSmallImageIndex)
{
}
//********************************************************************************
CBCGPRibbonMainPanelButton::CBCGPRibbonMainPanelButton (
		UINT	nID, 
		LPCTSTR lpszText, 
		HICON	hIcon) :
	CBCGPRibbonButton (nID, lpszText, hIcon)
{
}
//********************************************************************************
CBCGPRibbonMainPanelButton::~CBCGPRibbonMainPanelButton()
{
}
//********************************************************************************
COLORREF CBCGPRibbonMainPanelButton::OnFillBackground (CDC* pDC)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);

	if (DYNAMIC_DOWNCAST (CBCGPRibbonMainPanel, GetParentPanel ()) == NULL)
	{
		return CBCGPRibbonButton::OnFillBackground (pDC);
	}

	return CBCGPVisualManager::GetInstance ()->OnFillRibbonMainPanelButton (pDC, this);
}
//*****************************************************************************
void CBCGPRibbonMainPanelButton::OnDrawBorder (CDC* pDC)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);

	if (DYNAMIC_DOWNCAST (CBCGPRibbonMainPanel, GetParentPanel ()) == NULL)
	{
		CBCGPRibbonButton::OnDrawBorder (pDC);
		return;
	}

	CBCGPVisualManager::GetInstance ()->OnDrawRibbonMainPanelButtonBorder (pDC, this);
}

#endif // BCGP_EXCLUDE_RIBBON
