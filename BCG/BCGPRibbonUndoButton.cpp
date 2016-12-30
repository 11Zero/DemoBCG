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
// BCGPRibbonUndoButton.cpp: implementation of the CBCGPRibbonUndoButton class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "bcgcbpro.h"
#include "BCGPRibbonBar.h"
#include "BCGPRibbonUndoButton.h"
#include "BCGPRibbonLabel.h"
#include "BCGPRibbonPanelMenu.h"
#include "BCGPLocalResource.h"
#include "bcgprores.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#ifndef BCGP_EXCLUDE_RIBBON

const int nTextMarginHorz = 5;

class CBCGPRibbonUndoLabel : public CBCGPRibbonButton
{
public:
	DECLARE_DYNCREATE(CBCGPRibbonUndoLabel)

	CBCGPRibbonUndoLabel (LPCTSTR lpszText = NULL) :
		CBCGPRibbonButton (0, lpszText)
	{
		m_szMargin = CSize (0, 0);	// Make it smaller
	}

	virtual BOOL IsTabStop () const
	{
		return FALSE;	// User can't activate it by keyboard
	}

	virtual void OnDraw (CDC* pDC)
	{
		ASSERT_VALID (this);
		ASSERT_VALID (pDC);

		CRect rectText = m_rect;
		rectText.DeflateRect (nTextMarginHorz, 0);

		DoDrawText (pDC, m_strText, rectText, DT_SINGLELINE | DT_VCENTER);
	}
};

IMPLEMENT_DYNCREATE(CBCGPRibbonUndoLabel, CBCGPRibbonButton)

IMPLEMENT_DYNCREATE(CBCGPRibbonUndoButton, CBCGPRibbonPaletteButton)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGPRibbonUndoButton::CBCGPRibbonUndoButton()
{
	CommonInit ();
}
//********************************************************************************
CBCGPRibbonUndoButton::CBCGPRibbonUndoButton (UINT nID, LPCTSTR lpszText, 
									  int nSmallImageIndex, 
									  int nLargeImageIndex) :
	CBCGPRibbonPaletteButton (nID, lpszText, nSmallImageIndex, nLargeImageIndex, CSize (0, 0), 0, FALSE)
{
	CommonInit ();
}
//********************************************************************************
CBCGPRibbonUndoButton::CBCGPRibbonUndoButton (
		UINT	nID, 
		LPCTSTR lpszText, 
		HICON	hIcon) :
	CBCGPRibbonPaletteButton (nID, lpszText, -1, -1, CSize (0, 0), 0, FALSE)
{
	CommonInit ();

	m_hIcon = hIcon;
}
//********************************************************************************
void CBCGPRibbonUndoButton::CommonInit ()
{
	m_nActionNumber = -1;

	SetButtonMode (TRUE);
	SetIconsInRow (1);
	SetDefaultCommand ();

	m_sizeMaxText = CSize (0, 0);

	CBCGPLocalResource locaRes;
	
	m_strCancel.LoadString (IDS_BCGBARRES_CANCEL);
	m_strUndoOne.LoadString (IDS_BCGBARRES_UNDO_ONE);
	m_strUndoFmt.LoadString (IDS_BCGBARRES_UNDO_FMT);

	AddSubItem (new CBCGPRibbonUndoLabel (m_strCancel));
}
//********************************************************************************
CBCGPRibbonUndoButton::~CBCGPRibbonUndoButton()
{
}
//********************************************************************************
void CBCGPRibbonUndoButton::AddUndoAction (LPCTSTR lpszLabel)
{
	ASSERT_VALID (this);
	ASSERT (lpszLabel != NULL);

	Clear ();

	m_arLabels.Add (lpszLabel);
	m_nIcons = (int) m_arLabels.GetSize ();

	m_sizeMaxText = CSize (0, 0);
}
//********************************************************************************
void CBCGPRibbonUndoButton::CleanUpUndoList ()
{
	ASSERT_VALID (this);

	Clear ();

	m_arLabels.RemoveAll ();
	m_sizeMaxText = CSize (0, 0);
}
//********************************************************************************
void CBCGPRibbonUndoButton::NotifyHighlightListItem (int nIndex)
{
	ASSERT_VALID (this);

	if (m_pPopupMenu != NULL)
	{
		m_nActionNumber = nIndex + 1;

		//--------------
		// Change label:
		//--------------
		CString strLabel = m_strCancel;

		if (m_nActionNumber > 0)
		{
			if (m_nActionNumber == 1)
			{
				strLabel = m_strUndoOne;
			}
			else
			{
				strLabel.Format (m_strUndoFmt, m_nActionNumber);
			}
		}

		CBCGPRibbonPanelMenu* pPanelMenu = DYNAMIC_DOWNCAST (CBCGPRibbonPanelMenu, m_pPopupMenu);
		if (pPanelMenu != NULL)
		{
			ASSERT_VALID (pPanelMenu);

			if (pPanelMenu->GetPanel () != NULL)
			{
				CBCGPBaseRibbonElement* pMenuElem = pPanelMenu->GetPanel ()->FindByID (0);
				
				if (pMenuElem != NULL)
				{
					pMenuElem->SetText (strLabel);
					pMenuElem->Redraw ();
				}
			}
		}

		RedrawIcons ();
	}

	CBCGPRibbonPaletteButton::NotifyHighlightListItem (nIndex);
}
//********************************************************************************
void CBCGPRibbonUndoButton::CopyFrom (const CBCGPBaseRibbonElement& s)
{
	ASSERT_VALID (this);

	CBCGPRibbonPaletteButton::CopyFrom (s);

	CBCGPRibbonUndoButton& src = (CBCGPRibbonUndoButton&) s;

	m_nActionNumber = src.m_nActionNumber;

	m_arLabels.RemoveAll ();
	m_arLabels.Copy (src.m_arLabels);
	m_nIcons = src.m_nIcons;

	m_sizeMaxText = src.m_sizeMaxText;
}
//********************************************************************************
void CBCGPRibbonUndoButton::OnClick (CPoint point)
{
	ASSERT_VALID (this);

	m_nActionNumber = -1;

	CBCGPRibbonPaletteButton::OnClick (point);
}
//********************************************************************************
CSize CBCGPRibbonUndoButton::GetIconSize () const
{
	ASSERT_VALID (this);
	return m_sizeMaxText;
}
//********************************************************************************
void CBCGPRibbonUndoButton::OnDrawPaletteIcon (	CDC* pDC, CRect rectIcon, 
												int nIconIndex, CBCGPRibbonPaletteIcon* pIcon,
												COLORREF clrText)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);
	ASSERT_VALID (pIcon);
	ASSERT (nIconIndex >= 0);
	ASSERT (nIconIndex < m_nIcons);

	BOOL bIsChecked = pIcon->m_bIsChecked;
	BOOL bIsHighlighted = pIcon->m_bIsHighlighted;

	pIcon->m_bIsChecked = FALSE;
	pIcon->m_bIsHighlighted = nIconIndex < m_nActionNumber;

	pIcon->OnFillBackground (pDC);

	CRect rectText = rectIcon;
	rectText.DeflateRect (nTextMarginHorz, 0);

	COLORREF clrOld = (COLORREF)-1;
	if (clrText != (COLORREF)-1)
	{
		clrOld = pDC->SetTextColor (clrText);
	}

	pDC->DrawText (m_arLabels [nIconIndex], rectText, DT_VCENTER | DT_SINGLELINE | DT_LEFT);

	if (clrText != (COLORREF)-1)
	{
		pDC->SetTextColor (clrOld);
	}

	pIcon->OnDrawBorder (pDC);

	pIcon->m_bIsChecked = bIsChecked;
	pIcon->m_bIsHighlighted = bIsHighlighted;
}
//***************************************************************************
BOOL CBCGPRibbonUndoButton::OnClickPaletteSubItem (CBCGPRibbonButton* pButton, CBCGPRibbonPanelMenuBar* pMenuBar)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pButton);

	if (pButton->IsKindOf (RUNTIME_CLASS (CBCGPRibbonUndoLabel)))
	{
		ClosePopupMenu ();
		return TRUE;
	}

	return CBCGPRibbonPaletteButton::OnClickPaletteSubItem (pButton, pMenuBar);
}
//***************************************************************************
void CBCGPRibbonUndoButton::OnShowPopupMenu ()
{
	ASSERT_VALID (this);

	CBCGPBaseRibbonElement::OnShowPopupMenu ();

	m_bSmallIcons = FALSE;

	if (m_sizeMaxText == CSize (0, 0))
	{
		CBCGPRibbonBar* pRibbonBar = GetTopLevelRibbonBar ();
		ASSERT_VALID (pRibbonBar);

		CClientDC dc (pRibbonBar);

		CFont* pOldFont = dc.SelectObject (pRibbonBar->GetFont ());
		ASSERT (pOldFont != NULL);

		for (int i = 0; i < m_arLabels.GetSize (); i++)
		{
			CSize szText = dc.GetTextExtent (m_arLabels [i]);

			m_sizeMaxText.cx = max (m_sizeMaxText.cx, szText.cx);
			m_sizeMaxText.cy = max (m_sizeMaxText.cy, szText.cy);
		}

		m_sizeMaxText.cx = max (m_sizeMaxText.cx, dc.GetTextExtent (m_strCancel).cx);
		m_sizeMaxText.cx = max (m_sizeMaxText.cx, dc.GetTextExtent (m_strUndoOne).cx);
		m_sizeMaxText.cx = max (m_sizeMaxText.cx, dc.GetTextExtent (m_strUndoFmt).cx);

		m_sizeMaxText.cx += 2 * nTextMarginHorz;

		dc.SelectObject (pOldFont);
	}

	m_nActionNumber = -1;

	CBCGPRibbonPaletteButton::OnShowPopupMenu ();
}

#endif // BCGP_EXCLUDE_RIBBON
