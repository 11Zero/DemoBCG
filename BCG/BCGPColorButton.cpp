// BCGColorButton.cpp : implementation file
// This is a part of the BCGControlBar Library
// Copyright (C) 1998-2014 BCGSoft Ltd.
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.
//

#include "stdafx.h"

#include "BCGCBPro.h"
#include "BCGPColorButton.h"
#include "BCGPColorBar.h"
#include "ColorPopup.h"
#include "MenuImages.h"
#include "BCGPVisualManager.h"
#include "BCGPToolbarComboBoxButton.h"
#include "BCGPDrawManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static const int nImageHorzMargin = 8;

/////////////////////////////////////////////////////////////////////////////
// CBCGPColorButton

IMPLEMENT_DYNAMIC(CBCGPColorButton, CBCGPButton)

CBCGPColorButton::CBCGPColorButton(BOOL bRibbonMode)
{
	m_Color = RGB (0, 0, 0);
	m_ColorAutomatic = (COLORREF)-1;
	m_nColumns = -1;
	m_pPopup = NULL;
	m_bAltColorDlg = TRUE;
	m_pPalette = NULL;
	m_bEnabledInCustomizeMode = FALSE;
	m_bAutoSetFocus = TRUE;
	m_bRibbonMode = bRibbonMode;
}

CBCGPColorButton::~CBCGPColorButton()
{
	if (m_pPalette != NULL)
	{
		delete m_pPalette;
	}
}


BEGIN_MESSAGE_MAP(CBCGPColorButton, CBCGPButton)
	//{{AFX_MSG_MAP(CBCGPColorButton)
	ON_WM_KEYDOWN()
	ON_WM_LBUTTONDOWN()
	ON_WM_GETDLGCODE()
	ON_WM_SYSCOLORCHANGE()
	ON_WM_MOUSEMOVE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGPColorButton message handlers

CSize CBCGPColorButton::SizeToContent (BOOL bCalcOnly)
{
	CSize size = CBCGPButton::SizeToContent (FALSE);
	size.cx += CBCGPMenuImages::Size ().cx;

	if (!bCalcOnly)
	{
		SetWindowPos (NULL, -1, -1, size.cx, size.cy,
			SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
	}

	return size;
}
//*****************************************************************************************
void CBCGPColorButton::OnFillBackground (CDC* pDC, const CRect& rectClient)
{
	if (!IsDrawXPTheme ())
	{
		CBCGPButton::OnFillBackground (pDC, rectClient);
		return;
	}

	ASSERT_VALID (pDC);
	pDC->FillRect (rectClient, &globalData.brWindow);
}
//*****************************************************************************************
void CBCGPColorButton::OnDraw (CDC* pDC, const CRect& rect, UINT uiState)
{
	ASSERT_VALID (pDC);

	if (m_pPalette == NULL)
	{
		RebuildPalette (NULL);
	}

	CPalette* pCurPalette = pDC->SelectPalette (m_pPalette, FALSE);
	pDC->RealizePalette();

	CSize sizeArrow = CBCGPMenuImages::Size ();

	CRect rectColor = rect;
	rectColor.right -= sizeArrow.cx + nImageHorzMargin;

	CRect rectArrow = rect;
	rectArrow.left = rectColor.right;

	COLORREF color = m_Color;
	if (color == (COLORREF) -1)	// Automatic
	{
		//---------------------------
		// Draw automatic text label:
		//---------------------------
		color = m_ColorAutomatic;
		
		if (!m_strAutoColorText.IsEmpty ())
		{
			rectColor.right = rectColor.left + rectColor.Height ();

			CRect rectText = rect;
			rectText.left = rectColor.right;
			rectText.right = rectArrow.left;

			CFont* pOldFont = SelectFont (pDC);
			ASSERT(pOldFont != NULL);

			pDC->SetBkMode (TRANSPARENT);

			if (m_clrText == (COLORREF)-1)
			{
				if (m_bVisualManagerStyle && !m_bDontSkin)
				{
					pDC->SetTextColor (IsWindowEnabled() ? globalData.clrBarText : globalData.clrGrayedText);
				}
				else
				{
					pDC->SetTextColor (IsWindowEnabled() ? globalData.clrBtnText : globalData.clrGrayedText);
				}
			}
			else
			{
				pDC->SetTextColor (m_clrText);
			}

			UINT nFormat = DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS;

			if (m_bOnGlass)
			{
				CBCGPVisualManager::GetInstance ()->DrawTextOnGlass (pDC, 
					m_strAutoColorText, rectText, nFormat, 0);
			}
			else
			{
				pDC->DrawText (m_strAutoColorText, rectText, nFormat);
			}

			pDC->SelectObject (pOldFont);
		}
	}

	//----------------
	// Draw color box:
	//----------------
	rectColor.DeflateRect (2, 2);

	if (m_bOnGlass)
	{
		CBCGPDrawManager dm (*pDC);
		dm.DrawRect (rectColor, (COLORREF)-1, globalData.clrBtnDkShadow);
	}
	else
	{
		pDC->Draw3dRect (rectColor, globalData.clrBtnHilite, globalData.clrBtnHilite);
		rectColor.DeflateRect (1, 1);
		pDC->Draw3dRect (rectColor, globalData.clrBtnDkShadow, globalData.clrBtnDkShadow);
	}

	rectColor.DeflateRect (1, 1);

	if (color != (COLORREF)-1 && (uiState & ODS_DISABLED) == 0)
	{
		if (globalData.m_nBitsPerPixel == 8) // 256 colors
		{
			ASSERT_VALID (m_pPalette);
			color =  PALETTEINDEX (m_pPalette->GetNearestPaletteIndex (color));
		}

		if (m_bOnGlass)
		{
			CBCGPDrawManager dm (*pDC);
			dm.DrawRect (rectColor, color, (COLORREF)-1);
		}
		else
		{
			CBrush br (color);
			pDC->FillRect (rectColor, &br);
		}
	}

	//----------------------
	// Draw drop-down arrow:
	//----------------------
	CRect rectArrowWinXP = rectArrow;
	rectArrowWinXP.DeflateRect (2, 2);

	if (m_bVisualManagerStyle && !m_bDontSkin)
	{
		CBCGPDrawOnGlass dog (m_bOnGlass);

		BOOL bDisabled    = !IsWindowEnabled ();
		BOOL bFocused     = GetSafeHwnd () == ::GetFocus ();
		BOOL bHighlighted = IsHighlighted ();

		CBCGPToolbarComboBoxButton buttonDummy;
#ifndef _BCGSUITE_
		buttonDummy.m_bIsCtrl = TRUE;

		CBCGPVisualManager::GetInstance ()->OnDrawComboDropButton (
			pDC, rectArrowWinXP, bDisabled, FALSE,
			bHighlighted || bFocused,
			&buttonDummy);
#else
		CMFCVisualManager::GetInstance ()->OnDrawComboDropButton (
			pDC, rectArrowWinXP, bDisabled, FALSE,
			bHighlighted || bFocused,
			&buttonDummy);
#endif
	}
	else
	{
		if (!m_bWinXPTheme || !CBCGPVisualManager::GetInstance ()->DrawComboDropButtonWinXP (
										pDC, rectArrowWinXP,
										(uiState & ODS_DISABLED), m_bPushed,
										m_bHighlighted))
		{
			pDC->FillRect (rectArrow, &globalData.brBtnFace);

			CBCGPMenuImages::Draw (pDC, CBCGPMenuImages::IdArowDownLarge, rectArrow,
				(uiState & ODS_DISABLED) ? CBCGPMenuImages::ImageGray : CBCGPMenuImages::ImageBlack);

			pDC->Draw3dRect (rectArrow, globalData.clrBtnLight, globalData.clrBtnDkShadow);
			rectArrow.DeflateRect (1, 1);
			pDC->Draw3dRect (rectArrow, globalData.clrBtnHilite, globalData.clrBtnShadow);
		}
	}

	if (pCurPalette != NULL)
	{
		pDC->SelectPalette (pCurPalette, FALSE);
	}
}
//*****************************************************************************************
void CBCGPColorButton::OnDrawBorder (CDC* pDC, CRect& rectClient, UINT /*uiState*/)
{
	ASSERT_VALID (pDC);
	ASSERT (m_nFlatStyle != BUTTONSTYLE_NOBORDERS);	// Always has borders

	if (!m_bWinXPTheme || !CBCGPVisualManager::GetInstance ()->DrawComboBorderWinXP (
			pDC, rectClient, !IsWindowEnabled (), FALSE, TRUE))
	{
		pDC->Draw3dRect (rectClient,
					globalData.clrBtnDkShadow, globalData.clrBtnHilite);

		rectClient.DeflateRect (1, 1);

		if (m_nFlatStyle == BUTTONSTYLE_3D || m_bHighlighted)
		{
			pDC->Draw3dRect (rectClient,
						globalData.clrBtnShadow, globalData.clrBtnLight);
		}
	}
}
//*****************************************************************************************
void CBCGPColorButton::OnDrawFocusRect (CDC* pDC, const CRect& rectClient)
{
	CSize sizeArrow = CBCGPMenuImages::Size ();

	CRect rectColor = rectClient;
	rectColor.right -= sizeArrow.cx + nImageHorzMargin;

	CBCGPButton::OnDrawFocusRect (pDC, rectColor);
}
//*****************************************************************************************
void CBCGPColorButton::OnShowColorPopup () 
{
#ifndef BCGP_EXCLUDE_RIBBON
	if (m_bRibbonMode)
	{
		if (m_RibbonButton.m_pPopupMenu != NULL)
		{
			m_RibbonButton.ClosePopupMenu ();
		}
		else
		{
			m_RibbonButton.m_pParentControl = this;
			m_RibbonButton.OnShowPopupMenu ();
		}

		return;
	}
#endif

	if (m_pPopup != NULL)
	{
		m_pPopup->SendMessage (WM_CLOSE);
		m_pPopup = NULL;
		RedrawWindow();
		return;
	}

	if (m_Colors.GetSize () == 0)
	{
		// Use default pallete:
		CBCGPColorBar::InitColors (NULL, m_Colors);
	}

	m_pPopup = new CColorPopup (this, m_Colors, m_Color,
		m_strAutoColorText, m_strOtherText, m_strDocColorsText, m_lstDocColors,
		m_nColumns, m_ColorAutomatic);
	m_pPopup->m_bEnabledInCustomizeMode = m_bEnabledInCustomizeMode;

	CRect rectWindow;
	GetWindowRect (rectWindow);

	if (!m_pPopup->Create (this, rectWindow.left, rectWindow.bottom, NULL,
		m_bEnabledInCustomizeMode))
	{
		ASSERT (FALSE);
		m_pPopup = NULL;

		TRACE(_T ("Color menu can't be used in the customization mode. You need to set CBCGPColorButton::m_bEnabledInCustomizeMode\n"));
	}
	else
	{
		if (m_bEnabledInCustomizeMode)
		{
			CBCGPColorBar* pColorBar = DYNAMIC_DOWNCAST (
				CBCGPColorBar, m_pPopup->GetMenuBar());

			if (pColorBar != NULL)
			{
				ASSERT_VALID (pColorBar);
				pColorBar->m_bInternal = TRUE;
			}
		}

		CRect rect;
		m_pPopup->GetWindowRect (&rect);
		m_pPopup->UpdateShadow (&rect);

		if (m_bAutoSetFocus)
		{
			m_pPopup->GetMenuBar()->SetFocus ();
		}
	}

	if (m_bCaptured)
	{
		ReleaseCapture ();
		m_bCaptured = FALSE;
	}
}
//*****************************************************************************************
void CBCGPColorButton::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if (nChar == VK_SPACE || nChar == VK_DOWN)
	{
		OnShowColorPopup ();
		return;
	}
	
	CButton::OnKeyDown(nChar, nRepCnt, nFlags);
}
//*****************************************************************************************
void CBCGPColorButton::OnLButtonDown(UINT /*nFlags*/, CPoint /*point*/) 
{
	SetFocus ();
	OnShowColorPopup ();
}
//****************************************************************************************
void CBCGPColorButton::OnMouseMove(UINT nFlags, CPoint point) 
{
	FlatStyle nFlatStyle = m_nFlatStyle;
	if (IsDrawXPTheme ())
	{
		m_nFlatStyle = BUTTONSTYLE_SEMIFLAT;
	}

	CBCGPButton::OnMouseMove(nFlags, point);
	m_nFlatStyle = nFlatStyle;
}
//*****************************************************************************************
UINT CBCGPColorButton::OnGetDlgCode() 
{
	return DLGC_WANTARROWS;
}
//*****************************************************************************************
void CBCGPColorButton::EnableAutomaticButton (LPCTSTR lpszLabel, COLORREF colorAutomatic, BOOL bEnable)
{
	m_strAutoColorText = (bEnable && lpszLabel == NULL) ? _T("") : lpszLabel;
	m_ColorAutomatic = colorAutomatic;
}
//*****************************************************************************************
void CBCGPColorButton::EnableOtherButton (LPCTSTR lpszLabel, BOOL bAltColorDlg, BOOL bEnable)
{
	m_strOtherText = (bEnable && lpszLabel == NULL) ? _T("") : lpszLabel;
	m_bAltColorDlg = bAltColorDlg;
}
//*****************************************************************************************
void CBCGPColorButton::SetDocumentColors (LPCTSTR lpszLabel, CList<COLORREF,COLORREF>& lstColors)
{
	m_lstDocColors.RemoveAll ();
	m_strDocColorsText = (lpszLabel == NULL) ? _T("") : lpszLabel;

	if (!m_strDocColorsText.IsEmpty ())
	{
		m_lstDocColors.AddTail (&lstColors);
	}
}
//*****************************************************************************************
void CBCGPColorButton::SetPalette (CPalette* pPalette)
{
	if (m_Colors.GetSize () != 0)
	{
		m_Colors.SetSize (0);
		m_Colors.FreeExtra ();
	}

	CBCGPColorBar::InitColors (pPalette, m_Colors);
	RebuildPalette (pPalette);
}
//*****************************************************************************************
void CBCGPColorButton::SetColors(const CArray<COLORREF, COLORREF>& colors)
{
	if (m_Colors.GetSize () != 0)
	{
		m_Colors.SetSize (0);
		m_Colors.FreeExtra ();
	}

	m_Colors.Append(colors);
}
//*****************************************************************************************
void CBCGPColorButton::SetColor (COLORREF color /* -1 - automatic*/)
{
	m_Color = color;

#ifndef BCGP_EXCLUDE_RIBBON
	if (m_bRibbonMode)
	{
		m_RibbonButton.SetColor (m_Color);
	}
#endif

	if (GetSafeHwnd () != NULL)
	{
		Invalidate ();
		UpdateWindow ();
	}
}
//*****************************************************************************************
void CBCGPColorButton::UpdateColor (COLORREF color)
{
	SetColor (color);

		//-------------------------------------------------------
	// Trigger mouse up event (to button click notification):
	//-------------------------------------------------------
	CWnd* pParent = GetParent ();
	if (pParent != NULL)
	{
		pParent->SendMessage (	WM_COMMAND,
								MAKEWPARAM (GetDlgCtrlID (), BN_CLICKED),
								(LPARAM) m_hWnd);
	}
}
//****************************************************************************************
void CBCGPColorButton::RebuildPalette (CPalette* pPal)
{
	if (m_pPalette != NULL)
	{
		delete m_pPalette;
	}

	m_pPalette = new CPalette ();

	// Create palette:
	CClientDC dc (this);

	if (pPal == NULL)
	{
		int nColors = 256;	// Use 256 first entries
		UINT nSize = sizeof(LOGPALETTE) + (sizeof(PALETTEENTRY) * nColors);
		LOGPALETTE *pLP = (LOGPALETTE *) new BYTE[nSize];

		::GetSystemPaletteEntries (dc.GetSafeHdc (), 0, nColors, pLP->palPalEntry);

		pLP->palVersion = 0x300;
		pLP->palNumEntries = (USHORT) nColors;

		m_pPalette->CreatePalette (pLP);

		delete[] pLP;
	}
	else
	{
		ASSERT_VALID (pPal);
		int nColors = pPal->GetEntryCount ();
		UINT nSize = sizeof(LOGPALETTE) + (sizeof(PALETTEENTRY) * nColors);
		LOGPALETTE *pLP = (LOGPALETTE *) new BYTE[nSize];

		pPal->GetPaletteEntries (0, nColors, pLP->palPalEntry);

		pLP->palVersion = 0x300;
		pLP->palNumEntries = (USHORT) nColors;

		m_pPalette->CreatePalette (pLP);

		delete[] pLP;
	}
}
//*****************************************************************************************
void CBCGPColorButton::OnSysColorChange() 
{
	CBCGPButton::OnSysColorChange();
	RebuildPalette (NULL);

	Invalidate ();
	UpdateWindow ();
}
//*****************************************************************************************
BOOL CBCGPColorButton::IsDrawXPTheme () const
{
	return m_bWinXPTheme &&
		CBCGPVisualManager::GetInstance ()->IsWinXPThemeSupported ();
}

