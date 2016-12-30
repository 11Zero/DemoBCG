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
// BCGPRibbonColorButton.cpp: implementation of the CBCGPRibbonColorButton class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BCGPRibbonBar.h"
#include "BCGPRibbonCategory.h"
#include "BCGPRibbonColorButton.h"
#include "BCGPRibbonPanelMenu.h"
#include "BCGPDrawManager.h"
#include "BCGPRibbonFloaty.h"
#include "BCGPColorDialog.h"
#include "bcgprores.h"
#include "bcgglobals.h"
#include "BCGPLocalResource.h"
#include "BCGPColorButton.h"

#ifndef BCGP_EXCLUDE_RIBBON

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

BCGCBPRODLLEXPORT extern UINT BCGM_GETDOCUMENTCOLORS;

const int nMenuButtonAuto = 1;
const int nMenuButtonOther = 2;

class CBCGPRibbonColorMenuButton : public CBCGPRibbonButton
{
	friend class CBCGPRibbonColorButton;

	DECLARE_DYNCREATE(CBCGPRibbonColorMenuButton)

	CBCGPRibbonColorMenuButton(int nType = 0, CBCGPRibbonColorButton* pColorButton = NULL,
		LPCTSTR lpszLabel = NULL, BOOL bIsChecked = FALSE) :
		CBCGPRibbonButton (0, lpszLabel)
	{
		m_bIsChecked = bIsChecked;
		m_pColorButton = pColorButton;
		m_nType = nType;
	}

	virtual void CopyFrom (const CBCGPBaseRibbonElement& s)
	{
		ASSERT_VALID (this);
		CBCGPRibbonButton::CopyFrom (s);

		m_bIsChecked = s.IsChecked ();

		CBCGPRibbonColorMenuButton& src = (CBCGPRibbonColorMenuButton&) s;
		m_pColorButton = src.m_pColorButton;
		m_nType = src.m_nType;
	}

	virtual void OnDraw (CDC* pDC)
	{
		ASSERT_VALID (this);
		ASSERT_VALID (pDC);
		ASSERT_VALID (m_pColorButton);

		if (m_rect.IsRectEmpty ())
		{
			return;
		}

		const int cxImageBar = CBCGPToolBar::GetMenuImageSize ().cx + 
			2 * CBCGPVisualManager::GetInstance ()->GetMenuImageMargin () + 2;

		COLORREF clrText = OnFillBackground (pDC);
		COLORREF clrTextOld = (COLORREF)-1;

		if (m_bIsDisabled)
		{
			clrTextOld = pDC->SetTextColor (
				clrText == (COLORREF)-1 ? 
					CBCGPVisualManager::GetInstance ()->GetToolbarDisabledTextColor () : clrText);
		}
		else if (clrText != (COLORREF)-1)
		{
			clrTextOld = pDC->SetTextColor (clrText);
		}

		CRect rectText = m_rect;
		rectText.left += cxImageBar + TEXT_MARGIN;
		rectText.DeflateRect (m_szMargin.cx, m_szMargin.cx);

		pDC->DrawText (m_strText, rectText, DT_SINGLELINE | DT_END_ELLIPSIS | DT_VCENTER);

		if (clrTextOld != (COLORREF)-1)
		{
			pDC->SetTextColor (clrTextOld);
		}

		if (m_nType == nMenuButtonOther)
		{
			CRect rectImage = m_rect;
			rectImage.right = rectImage.left + cxImageBar;

			const int nIconSize = 16;

			if (globalData.m_hiconColors == NULL)
			{
				CBCGPLocalResource locaRes;

				globalData.m_hiconColors = (HICON) ::LoadImage (
					AfxGetResourceHandle (),
					MAKEINTRESOURCE (IDI_BCGRES_COLORS),
					IMAGE_ICON,
					16,
					16,
					LR_SHARED);
			}

			::DrawIconEx (pDC->GetSafeHdc (), 
				rectImage.left + (rectImage.Width () - nIconSize) / 2, 
				rectImage.top + (rectImage.Height () - nIconSize) / 2,
				globalData.m_hiconColors, 
				nIconSize, nIconSize, 0, NULL,
				DI_NORMAL);
		}
		else if (m_nType == nMenuButtonAuto)
		{
			CRect rectColorBox = m_rect;
			rectColorBox.right = rectColorBox.left + cxImageBar;
			rectColorBox.DeflateRect (2, 2);

			int nBoxSize = min (rectColorBox.Width (), rectColorBox.Height ());

			rectColorBox = CRect (CPoint (
				rectColorBox.left + (rectColorBox.Width () - nBoxSize) / 2,
				rectColorBox.top + (rectColorBox.Height () - nBoxSize) / 2),
							CSize (nBoxSize, nBoxSize));

			m_pColorButton->OnDrawPaletteIcon (pDC, rectColorBox, -1, NULL, (COLORREF)-1);
		}
	}

	CBCGPRibbonColorButton*	m_pColorButton;
	int						m_nType;
};

IMPLEMENT_DYNCREATE(CBCGPRibbonColorMenuButton, CBCGPRibbonButton)

IMPLEMENT_DYNCREATE(CBCGPRibbonColorButton, CBCGPRibbonPaletteButton)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGPRibbonColorButton::CBCGPRibbonColorButton()
{
	m_Color = (COLORREF)-1;
	CommonInit ();
}
//********************************************************************************
CBCGPRibbonColorButton::CBCGPRibbonColorButton(UINT nID, LPCTSTR lpszText, int nSmallImageIndex, COLORREF color) :
	CBCGPRibbonPaletteButton (nID, lpszText, nSmallImageIndex, -1)
{
	m_Color = color;
	CommonInit ();
}
//********************************************************************************
CBCGPRibbonColorButton::CBCGPRibbonColorButton(		
		UINT		nID,
		LPCTSTR		lpszText,
		BOOL		bSimpleButtonLook,
		int			nSmallImageIndex,
		int			nLargeImageIndex,
		COLORREF	color) : 
	CBCGPRibbonPaletteButton (nID, lpszText, nSmallImageIndex, nLargeImageIndex)
{
	CommonInit ();

	m_Color = color;
	m_bSimpleButtonLook = bSimpleButtonLook;
}
//********************************************************************************
void CBCGPRibbonColorButton::CommonInit ()
{
	m_ColorAutomatic = RGB (0, 0, 0);
	m_ColorHighlighted = (COLORREF)-1;
	m_bIsAutomaticButton = FALSE;
	m_bIsAutomaticButtonOnTop = TRUE;
	m_bIsAutomaticButtonBorder = FALSE;
	m_bIsOtherButton = FALSE;
	m_bIsDefaultCommand = TRUE;
	m_bSimpleButtonLook = FALSE;

	m_bIsOwnerDraw = TRUE;
	m_bDefaultButtonStyle = FALSE;

	SetButtonMode ();

	m_bHasGroups = FALSE;

	//--------------------
	// Add default colors:
	//--------------------
	SetPalette (NULL);
	m_nIconsInRow = 5;

	m_pOtherButton = NULL;
	m_pAutoButton = NULL;

	m_bSmallIcons = TRUE;

	SetColorBoxSize (CSize (22, 22));

	m_bTemporaryDocColors = TRUE;
}
//********************************************************************************
CBCGPRibbonColorButton::~CBCGPRibbonColorButton()
{
}
//********************************************************************************
void CBCGPRibbonColorButton::DrawImage (CDC* pDC, RibbonImageType type, CRect rectImage)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);

	if (m_bSimpleButtonLook)
	{
		CBCGPRibbonButton::DrawImage (pDC, type, rectImage);
		return;
	}

	CRect rectColor = rectImage;

	int nColorHeight = 5;

	if (globalData.GetRibbonImageScale () != 1.)
	{
		nColorHeight = (int) (globalData.GetRibbonImageScale () * nColorHeight);
	}

	rectColor.top = rectColor.bottom - nColorHeight + 1;

	if ((m_rect.Width () % 2) == 0)
	{
		rectColor.left++;
		rectColor.right++;
	}

	rectImage.OffsetRect (0, -1);
	CBCGPRibbonButton::DrawImage (pDC, type, rectImage);

	COLORREF color = (IsDisabled ()) ?
		globalData.clrBarShadow :
			(m_Color == (COLORREF)-1 ? m_ColorAutomatic : m_Color);

	COLORREF clrBorder = (COLORREF)-1;

	if (m_bIsAutomaticButtonBorder && m_Color == (COLORREF)-1)
	{
		clrBorder = RGB (197, 197, 197);
	}

	if (CBCGPToolBarImages::m_bIsDrawOnGlass)
	{
		CBCGPDrawManager dm (*pDC);

		rectColor.DeflateRect (1, 1);
		dm.DrawRect (rectColor, color, clrBorder);
	}
	else
	{
		CBrush br (PALETTERGB(	GetRValue (color),
								GetGValue (color), 
								GetBValue (color)));

		pDC->FillRect (rectColor, &br);

		if (clrBorder != (COLORREF)-1)
		{
			pDC->Draw3dRect (rectColor, clrBorder, clrBorder);
		}
	}
}
//*******************************************************************************
void CBCGPRibbonColorButton::UpdateColor (COLORREF color)
{
	ASSERT_VALID (this);

	if (m_Color == color)
	{
		return;
	}

	m_Color = color;

	if (m_pParentControl != NULL)
	{
		CBCGPColorButton* pColorButton = DYNAMIC_DOWNCAST(CBCGPColorButton, m_pParentControl);
		if (pColorButton != NULL)
		{
			ASSERT_VALID (pColorButton);
			pColorButton->UpdateColor (m_Color);
			return;
		}
	}

	CBCGPRibbonBar* pRibbonBar = GetTopLevelRibbonBar ();
	if (pRibbonBar != NULL)
	{
		ASSERT_VALID (pRibbonBar);

		CArray<CBCGPBaseRibbonElement*, CBCGPBaseRibbonElement*> arButtons;
		pRibbonBar->GetElementsByID (m_nID, arButtons, TRUE);

		for (int i = 0; i < arButtons.GetSize (); i++)
		{
			CBCGPRibbonColorButton* pOther =
				DYNAMIC_DOWNCAST (CBCGPRibbonColorButton, arButtons [i]);
			if (pOther != NULL && pOther != this)
			{
				ASSERT_VALID (pOther);

				pOther->m_Color = color;
				pOther->Redraw ();
			}
		}
	}

	if (m_pParentMenu != NULL)
	{
		ASSERT_VALID (m_pParentMenu);

		if (m_pParentMenu->IsFloaty ())
		{
			CBCGPRibbonFloaty* pFloaty = DYNAMIC_DOWNCAST (
				CBCGPRibbonFloaty, m_pParentMenu->GetParent ());

			if (pFloaty != NULL)
			{
				return;
			}
		}

		CFrameWnd* pParentFrame = BCGPGetParentFrame (m_pParentMenu);
		ASSERT_VALID (pParentFrame);

		pParentFrame->PostMessage (WM_CLOSE);
	}
	else
	{
		Redraw ();
	}
}
//*****************************************************************************************
void CBCGPRibbonColorButton::EnableAutomaticButton (LPCTSTR lpszLabel, COLORREF colorAutomatic, BOOL bEnable,
													LPCTSTR lpszToolTip,
													BOOL bOnTop,
													BOOL bDrawBorder)
{
	ASSERT_VALID (this);

	m_strAutomaticButtonLabel = (bEnable && lpszLabel == NULL) ? _T("") : lpszLabel;
	m_strAutomaticButtonToolTip = (lpszToolTip == NULL) ? m_strAutomaticButtonLabel : lpszToolTip;
	m_strAutomaticButtonToolTip.Remove (TCHAR('&'));
	m_ColorAutomatic = colorAutomatic;
	m_bIsAutomaticButton = bEnable;
	m_bIsAutomaticButtonOnTop = bOnTop;
	m_bIsAutomaticButtonBorder = bDrawBorder;
}
//*****************************************************************************************
void CBCGPRibbonColorButton::EnableOtherButton (LPCTSTR lpszLabel, LPCTSTR lpszToolTip)
{
	ASSERT_VALID (this);

	m_bIsOtherButton = (lpszLabel != NULL);
	m_strOtherButtonLabel = (lpszLabel == NULL) ? _T("") : lpszLabel;
	m_strOtherButtonToolTip = (lpszToolTip == NULL) ? m_strOtherButtonLabel : lpszToolTip;
	m_strOtherButtonToolTip.Remove (TCHAR('&'));
}
//*****************************************************************************************
void CBCGPRibbonColorButton::EnableDocumentColors (LPCTSTR lpszLabel)
{
	ASSERT_VALID (this);
	m_strDocumentColorsLabel = (lpszLabel == NULL) ? _T("") : lpszLabel;
}
//*****************************************************************************
void CBCGPRibbonColorButton::CopyFrom (const CBCGPBaseRibbonElement& s)
{
	ASSERT_VALID (this);

	int i = 0;

	CBCGPRibbonPaletteButton::CopyFrom (s);

	if (!s.IsKindOf (RUNTIME_CLASS (CBCGPRibbonColorButton)))
	{
		return;
	}

	CBCGPRibbonColorButton& src = (CBCGPRibbonColorButton&) s;

	m_Color = src.m_Color;
	m_ColorAutomatic = src.m_ColorAutomatic;

	m_Colors.RemoveAll ();
	m_DocumentColors.RemoveAll ();
	m_arContColumnsRanges.RemoveAll ();

	for (i = 0; i < src.m_Colors.GetSize (); i++)
	{
		m_Colors.Add (src.m_Colors [i]);
	}

	if (!m_bTemporaryDocColors)
	{
		for (i = 0; i < src.m_DocumentColors.GetSize (); i++)
		{
			m_DocumentColors.Add (src.m_DocumentColors [i]);
		}
	}

	for (i = 0; i < src.m_arContColumnsRanges.GetSize (); i++)
	{
		m_arContColumnsRanges.Add (src.m_arContColumnsRanges [i]);
	}

	m_bIsAutomaticButton = src.m_bIsAutomaticButton;
	m_bIsAutomaticButtonOnTop = src.m_bIsAutomaticButtonOnTop;
	m_bIsAutomaticButtonBorder = src.m_bIsAutomaticButtonBorder;
	m_bIsOtherButton = src.m_bIsOtherButton;

	m_strAutomaticButtonLabel = src.m_strAutomaticButtonLabel;
	m_strAutomaticButtonToolTip = src.m_strAutomaticButtonToolTip;
	m_strOtherButtonLabel = src.m_strOtherButtonLabel;
	m_strOtherButtonToolTip = src.m_strOtherButtonToolTip;
	m_strDocumentColorsLabel = src.m_strDocumentColorsLabel;

	m_bHasGroups = src.m_bHasGroups;

	m_sizeBox = src.m_sizeBox;
	m_bSimpleButtonLook = src.m_bSimpleButtonLook;

	m_imagesPalette.SetImageSize (src.m_imagesPalette.GetImageSize ());

	m_bTemporaryDocColors = src.m_bTemporaryDocColors;
}
//*****************************************************************************************
void CBCGPRibbonColorButton::SetPalette (CPalette* pPalette)
{
	ASSERT_VALID (this);

	if (m_bHasGroups)
	{
		// You cannot call this method when the color gallery has groups!
		ASSERT (FALSE);
		return;
	}

	if (pPalette != NULL)
	{
		// For backward compatibility
		SetColorBoxSize (CSize (16, 16));
	}

	m_Colors.RemoveAll ();
	CBCGPColorBar::InitColors (pPalette, m_Colors);
}
//***************************************************************************
COLORREF CBCGPRibbonColorButton::GetHighlightedColor () const
{
	ASSERT_VALID (this);
	return m_ColorHighlighted;
}
//***************************************************************************
void CBCGPRibbonColorButton::SetDocumentColors (LPCTSTR lpszLabel, CList<COLORREF,COLORREF>& lstColors)
{
	ASSERT_VALID (this);

	m_strDocumentColorsLabel = (lpszLabel == NULL) ? _T(" ") : lpszLabel;

	m_DocumentColors.RemoveAll ();

	for (POSITION pos = lstColors.GetHeadPosition (); pos != NULL;)
	{
		m_DocumentColors.Add (lstColors.GetNext (pos));
	}

	m_bTemporaryDocColors = m_DocumentColors.GetSize() == 0;
}
//***************************************************************************
void CBCGPRibbonColorButton::OnDrawPaletteIcon (CDC* pDC, CRect rectIcon, 
											int nIconIndex, CBCGPRibbonPaletteIcon* pIcon,
											COLORREF /*clrText*/)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);

	COLORREF color = (COLORREF)-1;
	
	BOOL bIsHighlighted = FALSE;
	BOOL bIsChecked = FALSE;
	BOOL bIsContColumn = FALSE;
	BOOL bDrawTopEdge = TRUE;
	BOOL bDrawBottomEdge = TRUE;

	int yMargin = m_arContColumnsRanges.GetSize () > 0 ? 0 : 2;

	if (pIcon == NULL)
	{
		color = m_ColorAutomatic;
		bIsChecked = (m_Color == (COLORREF)-1);
		yMargin = 2;
	}
	else
	{
		ASSERT_VALID (pIcon);

		color = GetColorByIndex (nIconIndex);

		bIsChecked = (m_Color == color);
		bIsHighlighted = pIcon->IsHighlighted ();

		if (nIconIndex < m_Colors.GetSize ())
		{
			for (int i = 0; i < m_arContColumnsRanges.GetSize (); i++)
			{
				int nIndex1 = LOWORD(m_arContColumnsRanges [i]);
				int nIndex2 = HIWORD(m_arContColumnsRanges [i]);

				if (nIconIndex >= nIndex1 && nIconIndex <= nIndex2)
				{
					bIsContColumn = TRUE;
					break;
				}
			}
		}

		if (bIsContColumn)
		{
			yMargin = 0;

			bDrawTopEdge = bDrawBottomEdge = FALSE;

			if (pIcon->IsFirstInColumn ())
			{
				rectIcon.top++;
				bDrawTopEdge = TRUE;
			}

			if (pIcon->IsLastInColumn ())
			{
				rectIcon.bottom--;
				bDrawBottomEdge = TRUE;
			}
		}
		else if (m_arContColumnsRanges.GetSize () > 0)
		{
			rectIcon.bottom--;
		}
	}

	rectIcon.DeflateRect (2, yMargin);

	CBCGPVisualManager::GetInstance ()->OnDrawRibbonColorPaletteBox (pDC, this, pIcon, color, rectIcon,
		bDrawTopEdge, bDrawBottomEdge, bIsHighlighted, bIsChecked, FALSE);
}
//***************************************************************************
void CBCGPRibbonColorButton::OnShowPopupMenu ()
{
	ASSERT_VALID (this);

	m_ColorHighlighted = (COLORREF)-1;

	CBCGPBaseRibbonElement::OnShowPopupMenu ();	// For BCGM_ON_BEFORE_SHOW_RIBBON_ITEM_MENU notification

	for (int i = 0; i < m_arSubItems.GetSize ();)
	{
		ASSERT_VALID (m_arSubItems [i]);

		CBCGPRibbonColorMenuButton* pMyButton = DYNAMIC_DOWNCAST (
			CBCGPRibbonColorMenuButton, m_arSubItems [i]);

		if (pMyButton != NULL)
		{
			ASSERT_VALID (pMyButton);
			
			delete pMyButton;
			m_arSubItems.RemoveAt (i);
		}
		else
		{
			i++;
		}
	}

	if (!m_bHasGroups)
	{
		Clear ();
		AddGroup (_T(""), (int) m_Colors.GetSize ());
	}

	if (m_bTemporaryDocColors && !m_strDocumentColorsLabel.IsEmpty())
	{
		m_DocumentColors.RemoveAll();

		CFrameWnd* pParentFrame = BCGCBProGetTopLevelFrame(GetParentWnd());
		ASSERT_VALID (pParentFrame);

		//---------------------------
		// Fill document colors list:
		//---------------------------
		CList<COLORREF, COLORREF> lstDocumentColors;

		pParentFrame->SendMessage (BCGM_GETDOCUMENTCOLORS, (WPARAM) m_nID, 
			(LPARAM) &lstDocumentColors);

		for (POSITION pos = lstDocumentColors.GetHeadPosition(); pos != NULL;)
		{
			m_DocumentColors.Add(lstDocumentColors.GetNext(pos));
		}
	}

	const int nDocColors = (int) m_DocumentColors.GetSize ();
	if (nDocColors > 0)
	{
		// Add temporary group
		AddGroup (m_strDocumentColorsLabel, nDocColors);
	}

	if (m_bIsOtherButton)
	{
		m_pOtherButton = new CBCGPRibbonColorMenuButton (nMenuButtonOther, this, m_strOtherButtonLabel);
		m_pOtherButton->SetToolTipText (m_strOtherButtonToolTip);

		AddSubItem (m_pOtherButton, 0);
	}

	if (m_bIsAutomaticButton)
	{
		m_pAutoButton = new CBCGPRibbonColorMenuButton (
			nMenuButtonAuto, this, m_strAutomaticButtonLabel, m_Color == (COLORREF)-1);

		m_pAutoButton->SetToolTipText (m_strAutomaticButtonToolTip);

		AddSubItem (m_pAutoButton, 0, m_bIsAutomaticButtonOnTop);	// Add to top
	}

	if (m_bHasGroups && m_arContColumnsRanges.GetSize () > 0)
	{
		m_imagesPalette.SetImageSize (CSize (m_sizeBox.cx, m_sizeBox.cy - 3));
	}
	else
	{
		m_imagesPalette.SetImageSize (m_sizeBox);
	}

	CBCGPRibbonPaletteButton::OnShowPopupMenu ();

	if (nDocColors > 0)
	{
		// Remove "Document Colors" group:
		m_arGroupNames.RemoveAt (m_arGroupNames.GetSize () - 1);
		m_arGroupLen.RemoveAt (m_arGroupLen.GetSize () - 1);

		m_nIcons -= nDocColors;
	}
}
//***************************************************************************
BOOL CBCGPRibbonColorButton::OnClickPaletteSubItem (CBCGPRibbonButton* pButton, CBCGPRibbonPanelMenuBar* pMenuBar)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pButton);

	if (pButton->GetOriginal () == m_pOtherButton && m_pOtherButton != NULL)
	{
		CBCGPRibbonColorButton* pColorButton = this;
		if (GetOriginal () != NULL)
		{
			pColorButton = (CBCGPRibbonColorButton*) GetOriginal ();
		}

		ASSERT_VALID (pColorButton);

		ClosePopupMenu ();

		CBCGPColorDialog dlg (m_Color, 0, m_pParentControl->GetSafeHwnd() != NULL ? m_pParentControl : GetTopLevelRibbonBar ());
		if (dlg.DoModal () == IDOK)
		{
			pColorButton->UpdateColor (dlg.GetColor ());
			pColorButton->NotifyCommand ();
		}

		return TRUE;
	}

	if (pButton->GetOriginal () == m_pAutoButton && m_pAutoButton != NULL)
	{
		UpdateColor ((COLORREF)-1);
		NotifyCommand (TRUE);
	}

	return CBCGPRibbonPaletteButton::OnClickPaletteSubItem (pButton, pMenuBar);
}
//***************************************************************************
void CBCGPRibbonColorButton::OnClickPaletteIcon (CBCGPRibbonPaletteIcon* pIcon)
{
	ASSERT_VALID (this);

	COLORREF color = GetColorByIndex (pIcon->GetIndex ());

	if (color != (COLORREF)-1)
	{
		UpdateColor (color);
	}

	CBCGPRibbonPaletteButton::OnClickPaletteIcon (pIcon);
}
//***************************************************************************
COLORREF CBCGPRibbonColorButton::GetColorByIndex (int nIconIndex) const
{
	if (nIconIndex < 0)
	{
		return (COLORREF)-1;
	}

	if (nIconIndex < m_Colors.GetSize ())
	{
		return m_Colors [nIconIndex];
	}

	nIconIndex -= (int) m_Colors.GetSize ();

	if (nIconIndex < m_DocumentColors.GetSize ())
	{
		return m_DocumentColors [nIconIndex];
	}

	return (COLORREF)-1;
}
//***************************************************************************
void CBCGPRibbonColorButton::AddColorsGroup (LPCTSTR lpszName, 
											 const CList<COLORREF,COLORREF>& lstColors,
											 BOOL bContiguousColumns)
{
	ASSERT_VALID (this);

	if (lstColors.IsEmpty ())
	{
		return;
	}

	if (!m_bHasGroups)
	{
		m_Colors.RemoveAll ();
		m_arContColumnsRanges.RemoveAll ();
		Clear ();
	}

	int nCurrSize = (int) m_Colors.GetSize ();

	for (POSITION pos = lstColors.GetHeadPosition (); pos != NULL;)
	{
		m_Colors.Add (lstColors.GetNext (pos));
	}

	AddGroup (lpszName == NULL ? _T("") : lpszName, (int) lstColors.GetCount ());

	if (bContiguousColumns)
	{
		m_arContColumnsRanges.Add (MAKELPARAM (nCurrSize, m_Colors.GetSize () - 1));
	}
	
	m_bHasGroups = TRUE;
}
//***************************************************************************
void CBCGPRibbonColorButton::RemoveAllColorGroups ()
{
	ASSERT_VALID (this);

	m_Colors.RemoveAll ();
	m_bHasGroups = FALSE;

	m_arContColumnsRanges.RemoveAll ();

	Clear ();
}
//***************************************************************************
void CBCGPRibbonColorButton::SetColorBoxSize (CSize sizeBox)
{
	ASSERT_VALID (this);

	if (globalData.GetRibbonImageScale () != 1.)
	{
		sizeBox.cx = (int) (.5 + globalData.GetRibbonImageScale () * sizeBox.cx);
		sizeBox.cy = (int) (.5 + globalData.GetRibbonImageScale () * sizeBox.cy);
	}

	m_sizeBox = sizeBox;

	if (m_bHasGroups && m_arContColumnsRanges.GetSize () > 0)
	{
		m_imagesPalette.SetImageSize (CSize (m_sizeBox.cx, m_sizeBox.cy - 3));
	}
	else
	{
		m_imagesPalette.SetImageSize (m_sizeBox);
	}
}
//***************************************************************************
void CBCGPRibbonColorButton::NotifyHighlightListItem (int nIndex)
{
	ASSERT_VALID (this);

	m_ColorHighlighted = GetColorByIndex (nIndex);

	CBCGPRibbonPaletteButton::NotifyHighlightListItem (nIndex);
}
//***************************************************************************
CString CBCGPRibbonColorButton::GetIconToolTip (const CBCGPRibbonPaletteIcon* pIcon) const
{
	ASSERT_VALID (this);
	ASSERT_VALID (pIcon);

	COLORREF color = GetColorByIndex (pIcon->GetIndex ());
	if (color != (COLORREF)-1)
	{
		CString str;

		if (!CBCGPColorBar::m_ColorNames.Lookup (color, str))
		{
			str.Format (_T("Hex={%02X,%02X,%02X}"), 
				GetRValue (color), GetGValue (color), GetBValue (color));
		}

		return str;
	}

	return CBCGPRibbonPaletteButton::GetIconToolTip (pIcon);
}

#endif // BCGP_EXCLUDE_RIBBON
