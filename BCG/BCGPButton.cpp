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

// BCGPButton.cpp : implementation file
//

#include "stdafx.h"
#include "BCGCBPro.h"
#include "BCGPLocalResource.h"
#include "bcgprores.h"
#include "BCGPButton.h"
#include "BCGGlobals.h"
#ifndef _BCGSUITE_
#include "BCGPToolBarImages.h"
#include "BCGPTooltipManager.h"
#include "BCGPPngImage.h"
#include "BCGPPopupMenu.h"
#endif
#include "BCGPVisualManager.h"
#include "BCGPDrawManager.h"
#include "BCGPDlgImpl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static const int nImageHorzMargin = 10;
static const int nVertMargin = 5;
static const COLORREF clrDefault = (COLORREF) -1;
static const UINT IdAutoCommand = 1;

BOOL CBCGPButton::m_bWinXPTheme = FALSE;
BOOL CBCGPButton::m_bWinXPThemeWasChecked = FALSE;
BOOL CBCGPButton::m_bDontSkin = FALSE;

/////////////////////////////////////////////////////////////////////////////
// CBCGPButton

IMPLEMENT_DYNCREATE(CBCGPButton, CButton)

CBCGPButton::CBCGPButton()
{
	m_bPushed			= FALSE;
	m_bClickiedInside	= FALSE;
	m_bHighlighted		= FALSE;
	m_bCaptured			= FALSE;
	m_nFlatStyle		= BUTTONSTYLE_3D;
	m_nAlignStyle		= ALIGN_CENTER;
	m_sizeImage			= CSize (0, 0);
	m_nStdImageId		= (CBCGPMenuImages::IMAGES_IDS) -1;
	m_nStdImageDisabledId = (CBCGPMenuImages::IMAGES_IDS) -1;
	m_StdImageState		= CBCGPMenuImages::ImageBlack;
	m_StdImageStateDisabled= CBCGPMenuImages::ImageBlack;
	m_bFullTextTooltip	= FALSE;
	m_bRighImage		= FALSE;
	m_bTopImage			= FALSE;
	m_hCursor			= NULL;
	m_sizePushOffset	= CSize (2, 2);
	m_bHover			= FALSE;
	m_clrRegular		= clrDefault;
	m_clrHover			= clrDefault;
	m_clrText			= clrDefault;
	m_clrFace			= (COLORREF)-1;
	m_bDrawFocus		= TRUE;
	m_bTransparent		= FALSE;
	m_hFont				= NULL;
	m_bDelayFullTextTooltipSet = FALSE;
	m_bGrayDisabled		= TRUE;
	m_bChecked			= FALSE;
	m_bIndeterminate	= FALSE;
	m_bCheckButton		= FALSE;
	m_b3State			= FALSE;
	m_bRadioButton		= FALSE;
	m_bAutoCheck		= FALSE;
	m_bHighlightChecked	= TRUE;
	m_bDefaultButton	= FALSE;
	m_bIsLeftText		= FALSE;
	m_bDrawText			= TRUE;
	m_nAutoRepeatTimeDelay = 0;
	m_bResponseOnButtonDown = FALSE;
	m_bDontUseWinXPTheme = FALSE;
	m_bWasDblClk		= FALSE;
	m_pToolTip			= NULL;
	m_bVisualManagerStyle = FALSE;
	m_bOnGlass			= FALSE;
	m_bBackstageMode	= FALSE;
	m_bDontAutoGrayImage = FALSE;
}
//****************************************************************************
CBCGPButton::~CBCGPButton()
{
	CBCGPTooltipManager::DeleteToolTip (m_pToolTip);
	CleanUp ();
}
//****************************************************************************
void CBCGPButton::CleanUp ()
{
	m_nStdImageId = (CBCGPMenuImages::IMAGES_IDS) -1;
	m_nStdImageDisabledId = (CBCGPMenuImages::IMAGES_IDS) -1;

	m_sizeImage = CSize (0, 0);

	m_Image.Clear ();
	m_ImageHot.Clear ();
	m_ImageDisabled.Clear ();

	m_ImageChecked.Clear ();
	m_ImageCheckedHot.Clear ();
	m_ImageCheckedDisabled.Clear ();
}


BEGIN_MESSAGE_MAP(CBCGPButton, CButton)
	//{{AFX_MSG_MAP(CBCGPButton)
	ON_WM_ERASEBKGND()
	ON_WM_CANCELMODE()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_SETCURSOR()
	ON_WM_ENABLE()
	ON_WM_SIZE()
	ON_WM_KILLFOCUS()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_WM_GETDLGCODE()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_PRINTCLIENT, OnPrintClient)
	ON_MESSAGE(WM_SETFONT, OnSetFont)
	ON_MESSAGE(WM_GETFONT, OnGetFont)
	ON_MESSAGE(BM_GETCHECK, OnGetCheck)
	ON_MESSAGE(BM_SETCHECK, OnSetCheck)
	ON_MESSAGE(BM_SETIMAGE, OnSetImage)
	ON_MESSAGE(BM_GETIMAGE, OnGetImage)
	ON_REGISTERED_MESSAGE(BCGM_UPDATETOOLTIPS, OnBCGUpdateToolTips)
	ON_REGISTERED_MESSAGE(BCGM_ONSETCONTROLVMMODE, OnBCGSetControlVMMode)
	ON_REGISTERED_MESSAGE(BCGM_ONSETCONTROLAERO, OnBCGSetControlAero)
	ON_REGISTERED_MESSAGE(BCGM_ONSETCONTROLBACKSTAGEMODE, OnBCGSetControlBackStageMode)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGPButton message handlers

void CBCGPButton::DrawItem(LPDRAWITEMSTRUCT lpDIS) 
{
	ASSERT (lpDIS != NULL);
	ASSERT (lpDIS->CtlType == ODT_BUTTON);

	CDC* pDCPaint = CDC::FromHandle (lpDIS->hDC);
	ASSERT_VALID (pDCPaint);

	DoDrawItem (pDCPaint, lpDIS->rcItem, lpDIS->itemState);
}
//****************************************************************************
void CBCGPButton::DoDrawItem (CDC* pDCPaint, CRect rectClient, UINT itemState)
{
	CBCGPMemDC memDC (*pDCPaint, this);
	CDC* pDC = &memDC.GetDC ();

	m_clrText = clrDefault;

	BOOL bDefaultDraw = TRUE;
	BOOL bDefaultCheckRadio = (m_bCheckButton || m_bRadioButton) && (GetStyle () & BS_PUSHLIKE) == 0;

	if (((m_bVisualManagerStyle && !m_bDontSkin) || bDefaultCheckRadio) && !m_bTransparent)
	{
		if (bDefaultCheckRadio)
		{
			globalData.DrawParentBackground (this, pDC);

			//----------------
			// Draw check box:
			//----------------
			CRect rectCheck = rectClient;
			const CSize sizeCheck = CBCGPVisualManager::GetInstance ()->GetCheckRadioDefaultSize ();

			if (m_bIsLeftText)
			{
				rectCheck.left = rectCheck.right - sizeCheck.cx;
			}
			else
			{
				rectCheck.right = rectCheck.left + sizeCheck.cx;
			}

			BOOL bMultiLine = (GetStyle () & BS_MULTILINE) != 0;

			if (bMultiLine)
			{
				if ((GetStyle () & BS_VCENTER) == BS_VCENTER)
				{
					rectCheck.top = rectCheck.CenterPoint ().y - sizeCheck.cy / 2;
				}
				else if ((GetStyle () & BS_BOTTOM) == BS_BOTTOM)
				{
					rectCheck.top = rectCheck.bottom - sizeCheck.cy;
				}
				else if ((GetStyle () & BS_TOP) == BS_TOP)
				{
				}
				else
				{
					rectCheck.top = rectCheck.CenterPoint ().y - sizeCheck.cy / 2;
				}
			}
			else
			{
				rectCheck.top = rectCheck.CenterPoint ().y - sizeCheck.cy / 2;
			}

			rectCheck.bottom = rectCheck.top + sizeCheck.cy;

			const BOOL bIsHighlighted = IsHighlighted () && IsWindowEnabled ();
			BOOL bDrawThemed = TRUE;

			if (m_bCheckButton)
			{
				int nCheck = IsChecked () ? 1 : 0;

				if (m_b3State && m_bIndeterminate)
				{
					nCheck = 2;
				}

				if (m_bVisualManagerStyle && !m_bDontSkin)
				{
					bDrawThemed = FALSE;
				}
				else
				{
					bDrawThemed = CBCGPVisualManager::GetInstance ()->CBCGPVisualManager::DrawCheckBox(pDC,
						rectCheck, bIsHighlighted, nCheck,
						IsWindowEnabled (), IsPressed () && bIsHighlighted);
				}

				if (!bDrawThemed)
				{
					CBCGPVisualManager::GetInstance ()->OnDrawCheckBoxEx (pDC,
						rectCheck, nCheck,
						bIsHighlighted, IsPressed () && bIsHighlighted, IsWindowEnabled ());
				}
			}
			else
			{
				if (m_bVisualManagerStyle && !m_bDontSkin)
				{
					bDrawThemed = FALSE;
				}
				else
				{
					bDrawThemed = CBCGPVisualManager::GetInstance ()->CBCGPVisualManager::DrawRadioButton(pDC,
						rectCheck, bIsHighlighted, IsChecked () || (IsPressed () && bIsHighlighted),
						IsWindowEnabled (), IsPressed () && bIsHighlighted);
				}

				if (!bDrawThemed)
				{
					CBCGPVisualManager::GetInstance ()->OnDrawRadioButton (pDC,
						rectCheck, IsChecked () || (IsPressed () && bIsHighlighted),
						bIsHighlighted, IsPressed () && bIsHighlighted, IsWindowEnabled ());
				}
			}

			double dblScale = pDCPaint->GetDeviceCaps(LOGPIXELSX) / 96.0f;

			int nMargin = dblScale <= 1.1 ? 2 : 2 + (int)dblScale;

			if (m_bIsLeftText)
			{
				rectClient.right = rectCheck.left - nMargin;
				rectClient.left += nMargin;
			}
			else
			{
				rectClient.left = rectCheck.right + nMargin;
			}

			bDefaultDraw = FALSE;
		}
		else
		{
			if (CBCGPVisualManager::GetInstance ()->OnDrawPushButton (pDC, rectClient, this, m_clrText))
			{
				if (m_clrFace != -1 && globalData.m_nBitsPerPixel > 8)
				{
					CBCGPDrawManager dm(*pDC);

					CRect rectHighlight = rectClient;
					int nPercentage = 50;
					BOOL bIsFocused = (itemState & ODS_FOCUS);

					if (IsHighlighted() || IsPressed() || IsChecked() || bIsFocused)
					{
						rectHighlight.DeflateRect(2, 2);
						nPercentage = 20;
					}
					else if (m_bOnGlass)
					{
						rectHighlight.DeflateRect(1, 1);
					}

					dm.HighlightRect(rectHighlight, nPercentage, (COLORREF)-1, 0, m_clrFace);
				}

				rectClient.DeflateRect (2, 2);
				bDefaultDraw = FALSE;
			}
		}
	}

	if (bDefaultDraw)
	{
		OnFillBackground (pDC, rectClient);
		OnDrawBorder (pDC, rectClient, itemState);
	}

	//---------------------
	// Draw button content:
	//---------------------
	OnDraw (pDC, rectClient, itemState);

	if ((itemState & ODS_FOCUS) && ((itemState & ODS_NOFOCUSRECT) == 0) && m_bDrawFocus)
	{
		OnDrawFocusRect (pDC, rectClient);
	}
}
//****************************************************************************
void CBCGPButton::PreSubclassWindow() 
{
	InitStyle (GetStyle ());

	ModifyStyle (BS_DEFPUSHBUTTON, BS_OWNERDRAW);
	CButton::PreSubclassWindow();

	LRESULT lres = ::SendMessage (m_hWnd, BM_GETCHECK, 0, 0);

	m_bChecked = (BOOL) lres;

	if (m_b3State && lres == BST_INDETERMINATE)
	{
		m_bIndeterminate = TRUE;
	}
}
//****************************************************************************
BOOL CBCGPButton::PreCreateWindow(CREATESTRUCT& cs) 
{
	InitStyle (cs.style);

	cs.style |= BS_OWNERDRAW;
	cs.style &= ~BS_DEFPUSHBUTTON;

	return CButton::PreCreateWindow(cs);
}
//****************************************************************************
void CBCGPButton::InitStyle (DWORD dwStyle)
{
	switch (dwStyle & 0x0F) 
	{
	case BS_CHECKBOX:
		m_bCheckButton = TRUE;
		break;
	case BS_AUTOCHECKBOX:
		m_bCheckButton = m_bAutoCheck = TRUE;
		break;
	case BS_3STATE:
		m_bCheckButton = m_b3State = TRUE;
		break;
	case BS_AUTO3STATE:
		m_bCheckButton = m_bAutoCheck = m_b3State = TRUE;
		break;
	case BS_RADIOBUTTON:
		m_bRadioButton = TRUE;
		break;
	case BS_AUTORADIOBUTTON:
		m_bRadioButton = m_bAutoCheck = TRUE;
		break;
	case BS_DEFPUSHBUTTON:
		m_bDefaultButton = TRUE;
		break;
	}

	if ((m_bCheckButton || m_bRadioButton) && (dwStyle & BS_PUSHLIKE) == 0)
	{
		if (dwStyle & BS_LEFTTEXT)
		{
			m_bIsLeftText = TRUE;
		}

		switch (dwStyle & BS_CENTER) 
		{
		case BS_LEFT:
		default:
			m_nAlignStyle = CBCGPButton::ALIGN_LEFT;
			break;

		case BS_RIGHT:
			m_nAlignStyle = CBCGPButton::ALIGN_RIGHT;
			break;

		case BS_CENTER:
			m_nAlignStyle = CBCGPButton::ALIGN_CENTER;
			break;
		}
	}

	if (!m_bWinXPThemeWasChecked)
	{
		if (!m_bWinXPTheme)
		{
			EnableWinXPTheme (AfxFindResourceHandle (
				MAKEINTRESOURCE (1), _T("24")) != NULL);
		}

		m_bWinXPThemeWasChecked = TRUE;
	}
}
//****************************************************************************
BOOL CBCGPButton::OnEraseBkgnd(CDC* /*pDC*/) 
{
	return TRUE;
}
//****************************************************************************
void CBCGPButton::OnFillBackground (CDC* pDC, const CRect& rectClient)
{
	if (m_bTransparent)
	{
		// Copy background from the parent window
		globalData.DrawParentBackground (this, pDC);
	}
	else
	{
		if (m_clrFace == (COLORREF)-1)
		{
			pDC->FillRect (rectClient, &globalData.brBtnFace);
		}
		else
		{
			pDC->FillSolidRect (rectClient, m_clrFace);
		}
	}

	if (m_bChecked && m_bHighlightChecked && !(m_bPushed && m_bHighlighted))
	{
		CBCGPDrawManager dm (*pDC);
		dm.HighlightRect (rectClient);
	}
}
//****************************************************************************
void CBCGPButton::OnDraw (CDC* pDC, const CRect& rect, UINT uiState)
{
	CRect rectText = rect;
	CRect rectImage = rect;

	CString strText;
	if (m_bDrawText)
	{
		GetWindowText (strText);
	}

	if (m_sizeImage.cx != 0)
	{
		if (!strText.IsEmpty ())
		{
			if (m_bTopImage)
			{
				rectImage.bottom = rectImage.top + m_sizeImage.cy + GetVertMargin ();
				rectText.top = rectImage.bottom;
				rectText.bottom -= GetVertMargin ();
			}
			else if (m_bRighImage)
			{
				rectText.right -= m_sizeImage.cx + GetImageHorzMargin () / 2;
				rectImage.left = rectText.right;
				rectImage.right -= GetImageHorzMargin () / 2;
			}
			else
			{
				rectText.left +=  m_sizeImage.cx + GetImageHorzMargin () / 2;
				rectImage.left += GetImageHorzMargin () / 2;
				rectImage.right = rectText.left;
			}
		}

		// Center image:
		rectImage.DeflateRect ((rectImage.Width () - m_sizeImage.cx) / 2,
			max (0, (rectImage.Height () - m_sizeImage.cy) / 2));
	}
	else
	{
		rectImage.SetRectEmpty ();
	}

	//-----------
	// Draw text:
	//-----------
	CFont* pOldFont = SelectFont (pDC);
	ASSERT(pOldFont != NULL);

	pDC->SetBkMode (TRANSPARENT);

	UINT uiDTFlags = DT_END_ELLIPSIS;
	BOOL bIsSingleLine = FALSE;

	BOOL bDefaultCheckRadio = (m_bCheckButton || m_bRadioButton) && (GetStyle () & BS_PUSHLIKE) == 0;
	BOOL bMultiLine = (GetStyle () & BS_MULTILINE) != 0 && m_strDescription.IsEmpty();

	COLORREF clrTextDefault = (bDefaultCheckRadio && m_bVisualManagerStyle) ? globalData.clrBarText : globalData.clrBtnText;
	COLORREF clrText = m_clrRegular == clrDefault ? clrTextDefault : m_clrRegular;
	
	if (m_bHighlighted && m_clrHover != clrDefault)
	{
		clrText = m_clrHover;
	}

	if (bDefaultCheckRadio && !bMultiLine)
	{
		uiDTFlags |= DT_VCENTER | DT_SINGLELINE;
		bIsSingleLine = TRUE;

		switch (m_nAlignStyle)
		{
		case ALIGN_LEFT:
			uiDTFlags |= DT_LEFT;
			break;
			
		case ALIGN_RIGHT:
			uiDTFlags |= DT_RIGHT;
			break;
			
		case ALIGN_CENTER:
			uiDTFlags |= DT_CENTER;
			break;
		}
	}
	else
	{
		if (bMultiLine)
		{
			uiDTFlags |= DT_WORDBREAK;

			if (!bDefaultCheckRadio)
			{
				rectText.DeflateRect (GetVertMargin () / 2, GetVertMargin () / 2);
			}

			CRect rectCalc = rectText;
			pDC->DrawText (strText, rectCalc, DT_CALCRECT | DT_WORDBREAK);

			if ((GetStyle () & BS_VCENTER) == BS_VCENTER)
			{
				rectText.top = rectText.CenterPoint ().y - rectCalc.Height () / 2;
			}
			else if ((GetStyle () & BS_BOTTOM) == BS_BOTTOM)
			{
				rectText.top = rectText.bottom - rectCalc.Height ();
			}
			else if ((GetStyle () & BS_TOP) == BS_TOP)
			{
			}
			else
			{
				rectText.top = rectText.CenterPoint ().y - rectCalc.Height () / 2;
			}

			rectText.bottom = rectText.top + rectCalc.Height ();
		}
		else if (strText.Find (_T('\n')) < 0 || !m_strDescription.IsEmpty())
		{
			uiDTFlags |= DT_VCENTER | DT_SINGLELINE;
			bIsSingleLine = TRUE;
		}
		else
		{
			rectText.DeflateRect (0, GetVertMargin () / 2);
		}

		switch (m_nAlignStyle)
		{
		case ALIGN_LEFT:
			uiDTFlags |= DT_LEFT;

			if (!bDefaultCheckRadio)
			{
				rectText.left += GetImageHorzMargin () / 2;
			}

			break;

		case ALIGN_RIGHT:
			uiDTFlags |= DT_RIGHT;

			if (!bDefaultCheckRadio)
			{
				rectText.right -= GetImageHorzMargin () / 2;
			}

			break;

		case ALIGN_CENTER:
			uiDTFlags |= DT_CENTER;
			break;
		}
	}

	if (GetExStyle() & WS_EX_LAYOUTRTL)
	{
		uiDTFlags |= DT_RTLREADING;
	}

	if ((uiState & ODS_DISABLED) && m_bGrayDisabled)
	{
#ifndef _BCGSUITE_
		if (!m_bOnGlass && CBCGPVisualManager::GetInstance ()->IsEmbossDisabledBtnText () && m_strDescription.IsEmpty())
#else
		if (!m_bOnGlass && m_strDescription.IsEmpty())
#endif
		{
			pDC->SetTextColor (globalData.clrBtnHilite);

			CRect rectShft = rectText;
			rectShft.OffsetRect (1, 1);
			OnDrawText (pDC, rectShft, strText, uiDTFlags, uiState);
		}

#ifndef _BCGSUITE_
		pDC->SetTextColor (CBCGPVisualManager::GetInstance ()->GetToolbarDisabledTextColor ());
#else
		pDC->SetTextColor (globalData.clrGrayedText);
#endif
	}
	else
	{
		if (m_clrText == clrDefault)
		{
			pDC->SetTextColor (clrText);
		}
		else
		{
			pDC->SetTextColor (m_bHighlighted && m_clrHover != clrDefault ? m_clrHover : m_clrText);
		}
	}

	if (m_bDelayFullTextTooltipSet)
	{
		BOOL bIsFullText = pDC->GetTextExtent (strText).cx <= rectText.Width ();
		SetTooltip (bIsFullText || !bIsSingleLine ? NULL : (LPCTSTR) strText);
		m_bDelayFullTextTooltipSet = FALSE;
	}

	OnDrawText (pDC, rectText, strText, uiDTFlags, uiState);

	//------------
	// Draw image:
	//------------
	if (!rectImage.IsRectEmpty ())
	{
		if (m_nStdImageId != (CBCGPMenuImages::IMAGES_IDS) -1)
		{
			CBCGPMenuImages::IMAGES_IDS id = m_nStdImageId;
			CBCGPMenuImages::IMAGE_STATE state = m_StdImageState;

			if ((uiState & ODS_DISABLED) && m_bGrayDisabled)
			{
				if (m_nStdImageDisabledId != (CBCGPMenuImages::IMAGES_IDS) -1)
				{
					id = m_nStdImageDisabledId;
				}

				if (m_StdImageStateDisabled != (CBCGPMenuImages::IMAGE_STATE)-1)
				{
					state = m_StdImageStateDisabled;
				}
			}

			CBCGPMenuImages::Draw (pDC, id, rectImage.TopLeft (), state);
		}
		else
		{
			BOOL bIsDisabled = (uiState & ODS_DISABLED) && m_bGrayDisabled;

			CBCGPToolBarImages& imageChecked = 
				(bIsDisabled && m_ImageCheckedDisabled.GetCount () != 0) ?
				m_ImageCheckedDisabled :
				(m_bHighlighted && m_ImageCheckedHot.GetCount () != 0) ?
				m_ImageCheckedHot : m_ImageChecked;

			CBCGPToolBarImages& image = 
				(bIsDisabled && m_ImageDisabled.GetCount () != 0) ?
				m_ImageDisabled :
				(m_bHighlighted && m_ImageHot.GetCount () != 0) ?
				m_ImageHot : m_Image;

			if (m_bChecked && imageChecked.GetCount () != 0)
			{
#ifndef _BCGSUITE_
				CBCGPDrawState ds(CBCGPVisualManager::GetInstance()->IsAutoGrayscaleImages());
#else
				CBCGPDrawState ds;
#endif
				imageChecked.PrepareDrawImage (ds);
				imageChecked.Draw (pDC, rectImage.left, rectImage.top, 0, FALSE, 
					bIsDisabled && m_ImageCheckedDisabled.GetCount () == 0);
				imageChecked.EndDrawImage (ds);
			}
			else if (image.GetCount () != 0)
			{
#ifndef _BCGSUITE_
				CBCGPDrawState ds(!m_bDontAutoGrayImage && CBCGPVisualManager::GetInstance()->IsAutoGrayscaleImages());
#else
				CBCGPDrawState ds;
#endif

				image.PrepareDrawImage (ds);
				image.Draw (pDC, rectImage.left, rectImage.top, 0, FALSE, 
					bIsDisabled && m_ImageDisabled.GetCount () == 0);
				image.EndDrawImage (ds);
			}
		}
	}

	pDC->SelectObject (pOldFont);
}
//****************************************************************************
void CBCGPButton::OnDrawText (CDC* pDC, const CRect& rect, const CString& strText,
							 UINT uiDTFlags, UINT /*uiState*/)
{
	ASSERT_VALID (pDC);

	CRect rectText = rect;

	CFont* pOldFont = NULL;
	int nTextHeight = 0;

	COLORREF clrTextOnGlass = IsWindowEnabled () ? globalData.clrBarText : globalData.clrGrayedText;

	if (!m_strDescription.IsEmpty())
	{
#ifndef _BCGSUITE_
		pOldFont = pDC->SelectObject(&globalData.fontCaption);
#else
		pOldFont = pDC->SelectObject(&globalData.fontBold);
#endif
		ASSERT_VALID(pOldFont);

		nTextHeight = pDC->GetTextExtent(strText).cy;

		uiDTFlags &= ~DT_VCENTER;
	}

	if (m_bOnGlass)
	{
		CBCGPVisualManager::GetInstance ()->DrawTextOnGlass (pDC, strText, rectText, uiDTFlags, 6, clrTextOnGlass);
	}
	else
	{
		pDC->DrawText (strText, rectText, uiDTFlags);
	}

	if (pOldFont != NULL)
	{
		pDC->SelectObject(pOldFont);
	}

	if (m_strDescription.IsEmpty())
	{
		return;
	}

	CRect rectDescr = rect;
	rectDescr.top += nTextHeight + GetVertMargin ();

	uiDTFlags &= ~DT_SINGLELINE;
	uiDTFlags |= DT_WORDBREAK;;

	if (m_bOnGlass)
	{
		CBCGPVisualManager::GetInstance ()->DrawTextOnGlass (pDC, m_strDescription, rectDescr, uiDTFlags, 6, clrTextOnGlass);
	}
	else
	{
		pDC->DrawText (m_strDescription, rectDescr, uiDTFlags);
	}
}
//****************************************************************************
void CBCGPButton::SetImage (HICON hIconCold, BOOL bAutoDestroy, HICON hIconHot, HICON hIconDisabled,
							BOOL bAlphaBlend)
{
	SetImageInternal (hIconCold, bAutoDestroy, hIconHot, FALSE /* Not checked */, hIconDisabled, bAlphaBlend);
}
//****************************************************************************
void CBCGPButton::SetImage (HBITMAP hBitmapCold, BOOL bAutoDestroy, HBITMAP hBitmapHot, BOOL bMap3dColors,
							HBITMAP hBitmapDisabled)
{
	SetImageInternal (hBitmapCold, bAutoDestroy, hBitmapHot, bMap3dColors, FALSE /* Not checked */,
		hBitmapDisabled);
}
//****************************************************************************
void CBCGPButton::SetImage (UINT uiBmpResId, UINT uiBmpHotResId, UINT uiBmpDsblResID)
{
	SetImageInternal (uiBmpResId, uiBmpHotResId, FALSE /* Not checked */, uiBmpDsblResID);
}
//****************************************************************************
void CBCGPButton::SetCheckedImage (HICON hIconCold, BOOL bAutoDestroy, HICON hIconHot, HICON hIconDisabled,
								   BOOL bAlphaBlend)
{
	SetImageInternal (hIconCold, bAutoDestroy, hIconHot, TRUE /* Checked */, hIconDisabled, bAlphaBlend);
}
//****************************************************************************
void CBCGPButton::SetCheckedImage (HBITMAP hBitmapCold, BOOL bAutoDestroy, HBITMAP hBitmapHot, BOOL bMap3dColors, HBITMAP hBitmapDisabled)
{
	SetImageInternal (hBitmapCold, bAutoDestroy, hBitmapHot, bMap3dColors, TRUE /* Checked */, hBitmapDisabled);
}
//****************************************************************************
void CBCGPButton::SetCheckedImage (UINT uiBmpResId, UINT uiBmpHotResId, UINT uiBmpDsblResID)
{
	SetImageInternal (uiBmpResId, uiBmpHotResId, TRUE /* Checked */, uiBmpDsblResID);
}
//****************************************************************************
void CBCGPButton::SetImageInternal (HICON hIconCold, BOOL bAutoDestroy, HICON hIconHot, BOOL bChecked, HICON hIconDisabled,
									BOOL bAlphaBlend)
{
	ClearImages (bChecked);

	if (hIconCold == NULL)
	{
		return;
	}

	const int nCount = hIconDisabled == NULL ? 2 : 3;

	for (int i = 0; i < nCount; i++)
	{
		HICON hIcon = (i == 0) ? hIconCold : (i == 1) ? hIconHot : hIconDisabled;
		CBCGPToolBarImages& image = bChecked ? ((i == 0) ? 
			m_ImageChecked : (i == 1) ? m_ImageCheckedHot : m_ImageCheckedDisabled) : 
			((i == 0) ? m_Image : (i == 1) ? m_ImageHot : m_ImageDisabled);

		if (hIcon == NULL)
		{
			continue;
		}

		ICONINFO info;
		::GetIconInfo (hIcon, &info);

		BITMAP bmp;
		::GetObject (info.hbmColor, sizeof (BITMAP), (LPVOID) &bmp);

		m_sizeImage.cx = bmp.bmWidth;
		m_sizeImage.cy = bmp.bmHeight;

		if (i == 0)
		{
			//--------------------------------------------
			// Create disabled image from the "cold" icon:
			//--------------------------------------------
			CDC dcMem;
			dcMem.CreateCompatibleDC (NULL);

			HBITMAP hBmp = (HBITMAP) ::CopyImage (info.hbmColor, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
			if (hBmp == NULL)
			{
				ASSERT (FALSE);
			}
			else
			{
				HBITMAP hOldBmp = (HBITMAP) dcMem.SelectObject (hBmp);

				dcMem.FillRect (CRect (0, 0, m_sizeImage.cx, m_sizeImage.cy), 
									&globalData.brBtnFace);

				::DrawIconEx (dcMem.GetSafeHdc (), 0, 0, hIcon, m_sizeImage.cx, m_sizeImage.cy,
								0, NULL, DI_NORMAL);

				dcMem.SelectObject (hOldBmp);
				::DeleteObject (hBmp);
			}
		}

		::DeleteObject (info.hbmColor);
		::DeleteObject (info.hbmMask);

		image.SetImageSize (CSize (bmp.bmWidth, bmp.bmHeight));

		if (!bAlphaBlend)
		{
			image.SetTransparentColor (globalData.clrBtnFace);
		}

		image.AddIcon (hIcon, bAlphaBlend);
	}

	if (bAutoDestroy)
	{
		if (hIconCold != NULL)
		{
			::DestroyIcon (hIconCold);
		}

		if (hIconHot != NULL)
		{
			::DestroyIcon (hIconHot);
		}

		if (hIconDisabled != NULL)
		{
			::DestroyIcon (hIconDisabled);
		}
	}
}
//****************************************************************************
void CBCGPButton::SetImageInternal (HBITMAP hBitmapCold, BOOL bAutoDestroy, HBITMAP hBitmapHot, BOOL bMap3dColors, BOOL bChecked,
									HBITMAP hBitmapDisabled)
{
	ClearImages (bChecked);

	if (hBitmapCold == NULL)
	{
		return;
	}

	const int nCount = hBitmapDisabled == NULL ? 2 : 3;

	for (int i = 0; i < nCount; i++)
	{
		HBITMAP hBitmap = (i == 0) ? hBitmapCold : (i == 1) ? hBitmapHot : hBitmapDisabled;
		CBCGPToolBarImages& image = bChecked ? ((i == 0) ? 
			m_ImageChecked : (i == 1) ? m_ImageCheckedHot : m_ImageCheckedDisabled) : 
			((i == 0) ? m_Image : (i == 1) ? m_ImageHot : m_ImageDisabled);

		if (hBitmap == NULL)
		{
			break;
		}

		BITMAP bmp;
		::GetObject (hBitmap, sizeof (BITMAP), (LPVOID) &bmp);

		BOOL bMap3dColorsCurr = bMap3dColors || (bmp.bmBitsPixel > 8 && bmp.bmBitsPixel < 32);
		BOOL bAlpha = !bMap3dColors && 
					bmp.bmBitsPixel == 32 && CBCGPToolBarImages::Is32BitTransparencySupported ();

		if (i == 0)
		{
			m_sizeImage.cx = bmp.bmWidth;
			m_sizeImage.cy = bmp.bmHeight;
		}
		else
		{
			// Hot and cold bitmaps should have the same size!
			ASSERT (m_sizeImage.cx == bmp.bmWidth);
			ASSERT (m_sizeImage.cy == bmp.bmHeight);
		}

		if (bAlpha)
		{
			CBCGPToolBarImages::PreMultiplyAlpha (hBitmap, TRUE);
		}

		image.SetImageSize (CSize (bmp.bmWidth, bmp.bmHeight));
		image.SetTransparentColor (bMap3dColorsCurr
									? RGB (192, 192, 192)
									: bAlpha
										? (COLORREF) -1
										: globalData.clrBtnFace);
		image.AddImage (hBitmap, TRUE);
	}

	if (bAutoDestroy)
	{
		if (hBitmapHot != NULL)
		{
			::DeleteObject (hBitmapHot);
		}

		if (hBitmapCold != NULL)
		{
			::DeleteObject (hBitmapCold);
		}

		if (hBitmapDisabled != NULL)
		{
			::DeleteObject (hBitmapDisabled);
		}
	}
}
//****************************************************************************
static HBITMAP ButtonLoadBitmap (UINT uiBmpResId)
{
	if (uiBmpResId == 0)
	{
		return NULL;
	}


	LPCTSTR lpszResourceName = MAKEINTRESOURCE (uiBmpResId);
	ASSERT(lpszResourceName != NULL);

	HBITMAP hbmp = NULL;

	//-----------------------------
	// Try to load PNG image first:
	//-----------------------------
	CBCGPPngImage pngImage;
	if (pngImage.Load (lpszResourceName))
	{
		hbmp = (HBITMAP) pngImage.Detach ();
	}
	else
	{
		HINSTANCE hinstRes = AfxFindResourceHandle (lpszResourceName, RT_BITMAP);
		if (hinstRes == NULL)
		{
			return NULL;
		}

		UINT uiLoadImageFlags = LR_CREATEDIBSECTION | LR_LOADMAP3DCOLORS;

		hbmp = (HBITMAP) ::LoadImage (
			hinstRes,
			lpszResourceName,
			IMAGE_BITMAP,
			0, 0,
			uiLoadImageFlags);
	}

	return hbmp;
}
//********************************************************************************
void CBCGPButton::SetImageInternal (UINT uiBmpResId, UINT uiBmpHotResId, BOOL bChecked, UINT uiBmpDsblResID)
{
	ClearImages (bChecked);

	if (uiBmpResId == 0)
	{
		return;
	}

	HBITMAP hbmp = ButtonLoadBitmap (uiBmpResId);
	HBITMAP hbmpHot = ButtonLoadBitmap (uiBmpHotResId);
	HBITMAP hbmpDisabled = ButtonLoadBitmap (uiBmpDsblResID);

	SetImageInternal (hbmp, TRUE /* AutoDestroy */, hbmpHot, FALSE, bChecked, hbmpDisabled);
}
//****************************************************************************
void CBCGPButton::SetStdImage (CBCGPMenuImages::IMAGES_IDS id, CBCGPMenuImages::IMAGE_STATE state,
							   CBCGPMenuImages::IMAGES_IDS idDisabled, CBCGPMenuImages::IMAGE_STATE stateDisabled)
{
	CleanUp ();

	m_sizeImage = CBCGPMenuImages::Size ();
	m_nStdImageId = id;
	m_nStdImageDisabledId = idDisabled;
	m_StdImageState = state;
	m_StdImageStateDisabled = stateDisabled;
}
//****************************************************************************
void CBCGPButton::OnCancelMode() 
{
	CButton::OnCancelMode();
	
	if (m_bCaptured)
	{
		ReleaseCapture ();

		m_bCaptured = FALSE;
		m_bPushed = FALSE;
		m_bClickiedInside = FALSE;
		m_bHighlighted = FALSE;
		m_bHover = FALSE;

		Invalidate ();
		UpdateWindow ();
	}

	if (m_nAutoRepeatTimeDelay >= 0)
	{
		KillTimer (IdAutoCommand);
	}
}
//****************************************************************************
void CBCGPButton::OnMouseMove(UINT nFlags, CPoint point) 
{
	m_bHover = FALSE;

	if ((nFlags & MK_LBUTTON) || m_nFlatStyle != BUTTONSTYLE_3D ||
		(m_bWinXPTheme && !m_bDontUseWinXPTheme))
	{
		BOOL bRedraw = FALSE;

		CRect rectClient;
		GetClientRect (rectClient);

		CPoint ptScreen = point;
		ClientToScreen(&ptScreen);

		if (rectClient.PtInRect (point) && WindowFromPoint(ptScreen)->GetSafeHwnd() == GetSafeHwnd())
		{
			m_bHover = TRUE;

			if (!m_bHighlighted)
			{
				m_bHighlighted = TRUE;
				bRedraw = TRUE;
			}

			if ((nFlags & MK_LBUTTON) && !m_bPushed && m_bClickiedInside)
			{
				m_bPushed = TRUE;
				bRedraw = TRUE;
			}

			if (!m_bCaptured)
			{
				SetCapture ();
				m_bCaptured = TRUE;
				bRedraw = TRUE;
			}
		}
		else
		{
			if (nFlags & MK_LBUTTON)
			{
				if (m_bPushed)
				{
					m_bPushed = FALSE;
					bRedraw = TRUE;
				}
			}
			else if (m_bHighlighted)
			{
				m_bHighlighted = FALSE;
				bRedraw = TRUE;
			}

			if (m_bCaptured && !(nFlags & MK_LBUTTON))
			{
				ReleaseCapture ();
				m_bCaptured = FALSE;

				bRedraw = TRUE;
			}
		}

		if (bRedraw)
		{
			Invalidate ();
			UpdateWindow ();
		}
	}
	
	CButton::OnMouseMove(nFlags, point);
}
//****************************************************************************
void CBCGPButton::OnLButtonDown(UINT nFlags, CPoint point) 
{
	if (m_bResponseOnButtonDown)
	{
		CWnd* pParent = GetParent ();
		if (pParent != NULL)
		{
			pParent->SendMessage (	WM_COMMAND,
									MAKEWPARAM (GetDlgCtrlID (), BN_CLICKED),
									(LPARAM) m_hWnd);
		}
	}
	else
	{
		m_bPushed = TRUE;
		m_bClickiedInside = TRUE;
		m_bHighlighted = TRUE;

		if (!m_bCaptured)
		{
			SetCapture ();
			m_bCaptured = TRUE;
		}

		Invalidate ();
		UpdateWindow ();

		if (m_nAutoRepeatTimeDelay > 0)
		{
			SetTimer (IdAutoCommand, m_nAutoRepeatTimeDelay, NULL);
		}
	}

	CButton::OnLButtonDown(nFlags, point);
}
//****************************************************************************
void CBCGPButton::OnLButtonUp(UINT nFlags, CPoint point) 
{
	BOOL bClicked = m_bPushed && m_bClickiedInside && m_bHighlighted;

	m_bPushed = FALSE;
	m_bClickiedInside = FALSE;
	m_bHighlighted = FALSE;

	if (bClicked && m_bAutoCheck)
	{
		if (m_b3State)
		{
			if (m_bIndeterminate)
			{
				m_bIndeterminate = FALSE;
				m_bChecked = FALSE;
			}
			else if (m_bChecked)
			{
				m_bIndeterminate = TRUE;
			}
			else
			{
				m_bChecked = TRUE;
			}
		}
		else if (m_bCheckButton)
		{
			m_bChecked = !m_bChecked;
		}
		else if (m_bRadioButton && !m_bChecked)
		{
			m_bChecked = TRUE;
			UncheckRadioButtonsInGroup ();
		}
	}

	HWND hWnd = GetSafeHwnd();

	if (m_bWasDblClk)
	{
		m_bWasDblClk = FALSE;
		CWnd* pParent = GetParent ();
		if (pParent != NULL)
		{
			pParent->SendMessage (	WM_COMMAND,
									MAKEWPARAM (GetDlgCtrlID (), BN_CLICKED),
									(LPARAM) m_hWnd);
		}
	}

	if (!::IsWindow (hWnd))
	{
		// The button was destroyed after send message
		return;
	}

	RedrawWindow ();

	CButton::OnLButtonUp(nFlags, point);

	if (!::IsWindow (hWnd))
	{
		// The button was destroyed after the OnLButtonUp handler
		return;
	}

	if (m_bCaptured)
	{
		ReleaseCapture ();
		m_bCaptured = FALSE;
	}

	if (m_nAutoRepeatTimeDelay > 0)
	{
		KillTimer (IdAutoCommand);
	}

	if (m_pToolTip->GetSafeHwnd () != NULL)
	{
		m_pToolTip->Pop ();

		CString str;
		m_pToolTip->GetText (str, this);
		m_pToolTip->UpdateTipText (str, this);
	}
}
//****************************************************************************
CSize CBCGPButton::SizeToContent (BOOL bCalcOnly)
{
	ASSERT (GetSafeHwnd () != NULL);

	CClientDC dc (this);

	CFont* pOldFont = SelectFont (&dc);
	ASSERT(pOldFont != NULL);

	CString strText;
	if (m_bDrawText)
	{
		GetWindowText (strText);
	}

	CSize sizeText (0, 0);

	if (!m_strDescription.IsEmpty())
	{
#ifndef _BCGSUITE_
		CFont* pOldFont = dc.SelectObject(&globalData.fontCaption);
#else
		CFont* pOldFont = dc.SelectObject(&globalData.fontBold);
#endif
		ASSERT_VALID(pOldFont);

		CSize sizeCaption = dc.GetTextExtent (strText);

		dc.SelectObject(pOldFont);

		CRect rectDesrc;
		GetClientRect (rectDesrc);
		
		dc.DrawText (m_strDescription, rectDesrc, DT_CALCRECT);
		
		CSize sizeDescr = rectDesrc.Size ();

		sizeText.cx = max(sizeCaption.cx, sizeDescr.cx);
		sizeText.cy = sizeCaption.cy + sizeDescr.cy + GetVertMargin ();
	}
	else
	{
		if (strText.Find (_T('\n')) < 0)
		{	
			sizeText = dc.GetTextExtent (strText);
		}
		else
		{
			CRect rectText;
			GetClientRect (rectText);
			
			dc.DrawText (strText, rectText, DT_CALCRECT);
			sizeText = rectText.Size ();
		}
	}

	int cx = 0;
	int cy = 0;

	if (m_bTopImage)
	{
		cx = max (sizeText.cx, m_sizeImage.cx) + GetImageHorzMargin ();
		if (sizeText.cx > 0)
		{
			cx += GetImageHorzMargin ();
		}

		cy = sizeText.cy + m_sizeImage.cy + 2 * GetVertMargin ();
		if (sizeText.cy > 0)
		{
			cy += GetVertMargin ();
		}
	}
	else
	{
		cx = sizeText.cx + m_sizeImage.cx + GetImageHorzMargin ();
		if (sizeText.cx > 0)
		{
			cx += GetImageHorzMargin ();
		}

		cy = max (sizeText.cy, m_sizeImage.cy) + GetVertMargin () * 2;
	}

	if (!bCalcOnly)
	{
		SetWindowPos (NULL, -1, -1, cx, cy,
			SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
	}

	dc.SelectObject (pOldFont);

	return CSize (cx, cy);
}
//****************************************************************************
BOOL CBCGPButton::PreTranslateMessage(MSG* pMsg) 
{
	if (m_pToolTip->GetSafeHwnd () != NULL)
	{
		if (pMsg->message == WM_LBUTTONDOWN ||
			pMsg->message == WM_LBUTTONUP ||
			pMsg->message == WM_MOUSEMOVE)
		{
			m_pToolTip->RelayEvent(pMsg);
		}
	}

	if (pMsg->message == WM_LBUTTONDOWN)
	{
		SetFocus();
	}

	if (pMsg->message == WM_KEYDOWN &&
		pMsg->wParam == VK_RETURN &&
		CBCGPPopupMenu::GetActiveMenu () == NULL && !m_bCheckButton && !m_bRadioButton)
	{
		CWnd* pParent = GetParent ();
		if (pParent != NULL)
		{
			pParent->SendMessage (	WM_COMMAND,
									MAKEWPARAM (GetDlgCtrlID (), BN_CLICKED),
									(LPARAM) m_hWnd);
			return TRUE;
		}
	}
	
	if (pMsg->message == WM_KEYDOWN && m_bAutoCheck && GetParent () != NULL)
	{
		switch (pMsg->wParam) 
		{
		case VK_SPACE:
			if (m_bCheckButton) 
			{
				if (m_b3State)
				{
					if (m_bIndeterminate)
					{
						m_bIndeterminate = FALSE;
						m_bChecked = FALSE;
					}
					else if (m_bChecked)
					{
						m_bIndeterminate = TRUE;
					}
					else
					{
						m_bChecked = TRUE;
					}
				}
				else
				{
					m_bChecked = !m_bChecked;
				}

				RedrawWindow ();

				CWnd* pWndParent = GetParent ();
				ASSERT_VALID (pWndParent);

				::SendMessage (pWndParent->GetSafeHwnd(), WM_COMMAND, 
					MAKELONG (::GetWindowLong(m_hWnd, GWL_ID), BN_CLICKED), 
					(LPARAM) m_hWnd);

				return TRUE	;
			}
			break;

		case VK_UP:
		case VK_LEFT:
			if (CheckNextPrevRadioButton (FALSE))
			{
				return TRUE;
			}
			break;

		case VK_DOWN:
		case VK_RIGHT:
			if (CheckNextPrevRadioButton (TRUE))
			{
				return TRUE;
			}
			break	;
		}
	}

	return CButton::PreTranslateMessage(pMsg);
}
//****************************************************************************
void CBCGPButton::SetTooltip (LPCTSTR lpszToolTipText)
{
	ASSERT (GetSafeHwnd () != NULL);

	if (lpszToolTipText == NULL)
	{
		if (m_pToolTip->GetSafeHwnd () != NULL)
		{
			m_pToolTip->Activate (FALSE);
		}
	}
	else
	{
		if (m_pToolTip->GetSafeHwnd () != NULL)
		{
			m_pToolTip->UpdateTipText (lpszToolTipText, this);
		}
		else
		{
			if (CBCGPTooltipManager::CreateToolTip (m_pToolTip, this,
											BCGP_TOOLTIP_TYPE_BUTTON))
			{
				m_pToolTip->AddTool (this, lpszToolTipText);
			}
		}

		if (m_pToolTip->GetSafeHwnd () != NULL)
		{
			m_pToolTip->Activate (TRUE);
		}
	}
}
//****************************************************************************
void CBCGPButton::GetTooltip (CString& strTooltip)
{
	strTooltip.Empty ();

	if (m_pToolTip->GetSafeHwnd () != NULL)
	{
		m_pToolTip->GetText (strTooltip, this);
	}
}
//****************************************************************************
void CBCGPButton::SetDescription (LPCTSTR lpszDescription)
{
	m_strDescription = (lpszDescription == NULL) ? _T("") : lpszDescription;

	if (!m_strDescription.IsEmpty())
	{
		m_nAlignStyle = ALIGN_LEFT;
	}
	else
	{
		m_nAlignStyle = ALIGN_CENTER;
	}
}
//****************************************************************************
void CBCGPButton::GetDescription (CString& strDescription) const
{
	strDescription = m_strDescription;
}
//****************************************************************************
void CBCGPButton::SetMouseCursor (HCURSOR hcursor)
{
	m_hCursor = hcursor;
}
//****************************************************************************
void CBCGPButton::SetMouseCursorHand ()
{
	SetMouseCursor (globalData.GetHandCursor ());
}
//*****************************************************************************
BOOL CBCGPButton::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	if (m_hCursor != NULL)
	{
		CRect rectClient;
		GetClientRect (rectClient);

		CPoint ptCursor;
		::GetCursorPos (&ptCursor);
		ScreenToClient (&ptCursor);

		if (rectClient.PtInRect (ptCursor))
		{
			::SetCursor (m_hCursor);
			return TRUE;
		}
	}
	
	return CButton::OnSetCursor(pWnd, nHitTest, message);
}
//*****************************************************************************
void CBCGPButton::OnDrawFocusRect (CDC* pDC, const CRect& rectClient)
{
	ASSERT_VALID (pDC);

	BOOL bDefaultCheckRadio = (m_bCheckButton || m_bRadioButton) && (GetStyle () & BS_PUSHLIKE) == 0;
	BOOL bMultiLineCheckRadio = bDefaultCheckRadio && (GetStyle () & BS_MULTILINE) != 0;

	CRect rectFocus = rectClient;

	if (bDefaultCheckRadio)
	{
		CString strText;
		if (m_bDrawText)
		{
			GetWindowText (strText);
		}

		strText.Remove (_T('&'));

		CFont* pOldFont = SelectFont (pDC);
		ASSERT(pOldFont != NULL);

		if (bMultiLineCheckRadio)
		{
			CRect rectCalc = rectFocus;
			pDC->DrawText (strText, rectCalc, DT_CALCRECT | DT_WORDBREAK);

			if ((GetStyle () & BS_VCENTER) == BS_VCENTER)
			{
				rectFocus.top = rectFocus.CenterPoint ().y - rectCalc.Height () / 2;
			}
			else if ((GetStyle () & BS_BOTTOM) == BS_BOTTOM)
			{
				rectFocus.top = rectFocus.bottom - rectCalc.Height ();
			}
			else if ((GetStyle () & BS_TOP) == BS_TOP)
			{
			}
			else
			{
				rectFocus.top = rectFocus.CenterPoint ().y - rectCalc.Height () / 2;
			}

			rectFocus.right = rectFocus.left + rectCalc.Width ();
			rectFocus.left--;
			rectFocus.bottom = rectFocus.top + rectCalc.Height ();
			
			if ((GetStyle () & BS_BOTTOM) == 0)
			{
				rectFocus.bottom++;
			}
		}
		else
		{
			int nTextWidth = pDC->GetTextExtent(strText).cx;
			if (m_sizeImage.cx != 0)
			{
				nTextWidth += m_sizeImage.cx + GetImageHorzMargin() / 2;
			}

			switch (m_nAlignStyle)
			{
			case ALIGN_LEFT:
			default:
				rectFocus.right = rectFocus.left + nTextWidth;
				break;
				
			case ALIGN_RIGHT:
				rectFocus.left = rectFocus.right - nTextWidth;
				break;
				
			case ALIGN_CENTER:
				rectFocus.left = rectFocus.CenterPoint().x - nTextWidth / 2;
				rectFocus.right = rectFocus.left + nTextWidth;
				break;
			}

			rectFocus.top = rectFocus.CenterPoint ().y - pDC->GetTextExtent (strText).cy / 2;
			rectFocus.bottom = rectFocus.top + pDC->GetTextExtent (strText).cy;
			rectFocus.InflateRect (2, 1);
		}

		rectFocus.top = max (rectFocus.top, rectClient.top);
		rectFocus.bottom = min (rectFocus.bottom, rectClient.bottom);

		COLORREF clrBk = pDC->SetBkColor(m_bVisualManagerStyle && !m_bDontSkin ? globalData.clrBarFace : globalData.clrBtnFace);

		if (m_bOnGlass)
		{
			CBCGPDrawManager dm (*pDC);
			dm.DrawFocusRect(rectFocus);
		}
		else
		{
			pDC->DrawFocusRect (rectFocus);
		}

		if (clrBk != (COLORREF)-1)
		{
			pDC->SetBkColor(clrBk);
		}

		pDC->SelectObject (pOldFont);
		return;
	}

#ifndef _BCGSUITE_
	if (m_bVisualManagerStyle && !CBCGPVisualManager::GetInstance()->IsDrawFocusRectOnPushButton(this))
	{
		return;
	}
#endif

	rectFocus.DeflateRect (1, 1);

	if (m_bOnGlass)
	{
		CBCGPDrawManager dm (*pDC);
		dm.DrawFocusRect(rectFocus);
	}
	else
	{
		if (!m_bWinXPTheme || m_bDontUseWinXPTheme)
		{
			COLORREF clrBckgr = (m_clrFace == (COLORREF)-1) ? globalData.clrBtnFace : m_clrFace;

			rectFocus.DeflateRect (1, 1);
			pDC->Draw3dRect (rectFocus, clrBckgr, clrBckgr);
		}

		pDC->DrawFocusRect (rectFocus);
	}
}
//******************************************************************************
void CBCGPButton::OnDrawParentBackground (CDC* pDC, CRect rectClient)
{
	ASSERT_VALID (pDC);
	globalData.DrawParentBackground (this, pDC, rectClient);
}
//******************************************************************************
void CBCGPButton::OnEnable(BOOL bEnable) 
{
	if (!bEnable)
	{
		// control disabled
		m_bPushed = FALSE;
		m_bClickiedInside = FALSE;
		m_bHighlighted = FALSE;
		
		if (m_bCaptured)
		{
			ReleaseCapture ();
			m_bCaptured = FALSE;
		}
	}
	
	RedrawWindow ();
	CButton::OnEnable(bEnable);
}
//******************************************************************************
void CBCGPButton::SetFaceColor (COLORREF crFace, BOOL bRedraw)
{
	m_clrFace = crFace;

	if (bRedraw && GetSafeHwnd () != NULL)
	{
		Invalidate ();
		UpdateWindow ();
	}
}
//*****************************************************************************
void CBCGPButton::SetDrawText(BOOL bSet, BOOL bRedraw)
{
	m_bDrawText = bSet;
	
	if (bRedraw && GetSafeHwnd () != NULL)
	{
		Invalidate ();
		UpdateWindow ();
	}
}
//*****************************************************************************
CFont* CBCGPButton::SelectFont (CDC* pDC)
{
	ASSERT_VALID (pDC);

	if (m_hFont != NULL && ::GetObjectType (m_hFont) != OBJ_FONT)
	{
		m_hFont = NULL;
	}

	CFont* pOldFont = m_hFont == NULL ?
		(CFont*) pDC->SelectStockObject (DEFAULT_GUI_FONT) :
		pDC->SelectObject (CFont::FromHandle (m_hFont));

	ASSERT(pOldFont != NULL);
	return pOldFont;
}
//*****************************************************************************
afx_msg LRESULT CBCGPButton::OnSetFont (WPARAM wParam, LPARAM lParam)
{
	BOOL bRedraw = (BOOL) LOWORD (lParam);

	m_hFont = (HFONT) wParam;

	if (bRedraw)
	{
		Invalidate ();
		UpdateWindow ();
	}

	return 0;
}
//*****************************************************************************
afx_msg LRESULT CBCGPButton::OnGetFont (WPARAM, LPARAM)
{
	return (LRESULT) m_hFont;
}
//*****************************************************************************
void CBCGPButton::EnableMenuFont (BOOL bOn, BOOL bRedraw)
{
	m_hFont = bOn ? (HFONT) globalData.fontRegular.GetSafeHandle () : NULL;

	if (bRedraw && GetSafeHwnd () != NULL)
	{
		Invalidate ();
		UpdateWindow ();
	}
}
//******************************************************************************
void CBCGPButton::EnableFullTextTooltip (BOOL bOn)
{
	m_bFullTextTooltip = bOn;
	m_bDelayFullTextTooltipSet = bOn;
}
//******************************************************************************
void CBCGPButton::OnSize(UINT nType, int cx, int cy) 
{
	m_bDelayFullTextTooltipSet = m_bFullTextTooltip;
	CButton::OnSize(nType, cx, cy);
}
//******************************************************************************
void CBCGPButton::OnDrawBorder (CDC* pDC, CRect& rectClient, UINT uiState)
{
	DrawBorder (pDC, rectClient, uiState);
}
//****************************************************************************************
void CBCGPButton::OnKillFocus(CWnd* pNewWnd) 
{
	CButton::OnKillFocus(pNewWnd);
	
	if (m_bCaptured)
	{
		ReleaseCapture ();
		m_bCaptured = FALSE;
	}
	
	m_bPushed = FALSE;
	m_bClickiedInside = FALSE;
	m_bHighlighted = FALSE;
	m_bHover = FALSE;

	Invalidate ();
	UpdateWindow ();
}
//*****************************************************************************************
void CBCGPButton::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	m_bPushed = TRUE;
	m_bClickiedInside = TRUE;
	m_bHighlighted = TRUE;

	Invalidate ();
	UpdateWindow ();

	CButton::OnLButtonDblClk(nFlags, point);
	m_bWasDblClk = TRUE;
}
//***************************************************************************************
void CBCGPButton::EnableWinXPTheme (BOOL bEnable/* = TRUE*/)
{
	m_bWinXPTheme = bEnable;
}
//***************************************************************************************
LRESULT CBCGPButton::OnGetCheck(WPARAM, LPARAM)
{
	if (m_b3State && m_bIndeterminate)
	{
		return BST_INDETERMINATE;
	}

	if (m_bCheckButton || m_bRadioButton)
	{
		return m_bChecked ? BST_CHECKED	: BST_UNCHECKED;
	}

	return 0;
}
//***************************************************************************************
LRESULT CBCGPButton::OnSetCheck(WPARAM fCheck, LPARAM)
{
	if (m_b3State)
	{
		switch (fCheck)
		{
		case BST_UNCHECKED:
			m_bChecked = FALSE;
			m_bIndeterminate = FALSE;
			break;

		case BST_CHECKED:
			m_bChecked = TRUE;
			m_bIndeterminate = FALSE;
			break;

		case BST_INDETERMINATE:
			m_bChecked = TRUE;
			m_bIndeterminate = TRUE;
			break;
		}

		Invalidate();
		UpdateWindow();
		return 0;
	}

	ASSERT (fCheck != BST_INDETERMINATE);

	if ((m_bCheckButton || m_bRadioButton) && (!m_bChecked) != (fCheck == BST_UNCHECKED)) 
	{
		m_bChecked = fCheck != BST_UNCHECKED;
		
		if (m_bRadioButton)
		{
			UncheckRadioButtonsInGroup ();
		}

		Invalidate();
		UpdateWindow();
	}

	return 0;
}
//****************************************************************************************
void CBCGPButton::ClearImages (BOOL bChecked)
{
	m_nStdImageId = (CBCGPMenuImages::IMAGES_IDS) -1;
	m_nStdImageDisabledId = (CBCGPMenuImages::IMAGES_IDS) -1;
	m_sizeImage = CSize (0, 0);

	if (bChecked)
	{
		m_ImageChecked.Clear ();
		m_ImageCheckedHot.Clear ();
		m_ImageCheckedDisabled.Clear ();
	}
	else
	{
		m_Image.Clear ();
		m_ImageHot.Clear ();
		m_ImageDisabled.Clear ();
	}
}
//****************************************************************************************
BOOL CBCGPButton::CheckNextPrevRadioButton (BOOL bNext)
{
	ASSERT_VALID (this);

	if (!m_bRadioButton)
	{
		return FALSE;
	}

	if (CBCGPPopupMenu::GetActiveMenu() != NULL)
	{
		return FALSE;
	}

	CWnd* pWndParent = GetParent ();
	ASSERT_VALID (pWndParent);

	CBCGPButton* pBtn = NULL;

	for (CWnd* pWnd = pWndParent->GetNextDlgGroupItem (this, !bNext); 
		pWnd != this; 
		pWnd = pWndParent->GetNextDlgGroupItem (pWnd, !bNext))
	{
		if ((pBtn = DYNAMIC_DOWNCAST(CBCGPButton, pWnd)) != NULL &&
			pBtn->m_bRadioButton &&
			(pBtn->GetStyle() & (WS_DISABLED | WS_VISIBLE)) == WS_VISIBLE)
		{
			break;
		}
	}

	if (pBtn != NULL && pBtn != this && !pBtn->m_bChecked)
	{
		pBtn->SetCheck (TRUE);
		pBtn->SetFocus ();

		::SendMessage (pBtn->GetParent()->GetSafeHwnd(), WM_COMMAND,
			MAKELONG (::GetWindowLong(pBtn->m_hWnd, GWL_ID), BN_CLICKED),
			(LPARAM) pBtn->m_hWnd);
		return TRUE;
	}

	return FALSE;
}
//****************************************************************************************
void CBCGPButton::UncheckRadioButtonsInGroup ()
{
	CWnd* pWndParent = GetParent ();
	if (pWndParent == NULL)
	{
		return;
	}

	ASSERT_VALID (pWndParent);

	if (!IsWindowEnabled () || !IsWindowVisible())
	{
		return;
	}

	BOOL bResetTabStop = FALSE;

	//--------------------------------------------------------
	// Walk through group and clear radio buttons check state
	//--------------------------------------------------------
	for (CWnd * pCtl = pWndParent->GetNextDlgGroupItem (this); 
		pCtl != this && pCtl != NULL;
		pCtl = pWndParent->GetNextDlgGroupItem (pCtl))
	{
		CBCGPButton* pBtn = DYNAMIC_DOWNCAST (CBCGPButton, pCtl);

		if (pBtn != NULL && pBtn->m_bRadioButton)
		{	
			if ((pBtn->GetStyle() & WS_TABSTOP) == WS_TABSTOP)
			{
				pBtn->ModifyStyle(WS_TABSTOP, 0);
				bResetTabStop = TRUE;
			}

			if (pBtn->m_bChecked) 
			{
				pBtn->m_bChecked = FALSE;
				pBtn->RedrawWindow ();
			}
		}
	}

	if (bResetTabStop)
	{
		ModifyStyle(0, WS_TABSTOP);
	}
}
//***********************************************************************************
void CBCGPButton::SetAutorepeatMode (int nTimeDelay)
{
	ASSERT (nTimeDelay >= 0);
	m_nAutoRepeatTimeDelay = nTimeDelay;
}
//***********************************************************************************
void CBCGPButton::OnTimer(UINT_PTR nIDEvent) 
{
	if (nIDEvent == IdAutoCommand)
	{
		if (m_bPushed && m_bHighlighted)
		{
			CWnd* pParent = GetParent ();
			if (pParent != NULL)
			{
				pParent->SendMessage (	WM_COMMAND,
										MAKEWPARAM (GetDlgCtrlID (), BN_CLICKED),
										(LPARAM) m_hWnd);
			}
		}
	}

	CButton::OnTimer(nIDEvent);
}
//****************************************************************************************
void CBCGPButton::DrawBorder (CDC* pDC, CRect& rectClient, UINT uiState)
{
	ASSERT_VALID (pDC);

	BOOL bBorderIsReady = FALSE;

	//----------------
	// Draw 3d border:
	//----------------
	if (m_nFlatStyle != BUTTONSTYLE_NOBORDERS)
	{
		if (m_bWinXPTheme && !m_bDontUseWinXPTheme &&
			CBCGPVisualManager::GetInstance ()->DrawPushButtonWinXP (pDC, rectClient, this, uiState))
		{
			bBorderIsReady = TRUE;
		}

		if (m_bPushed && m_bHighlighted || (uiState & ODS_SELECTED) || m_bChecked)
		{
			if (!bBorderIsReady)
			{
				pDC->Draw3dRect (rectClient,
							globalData.clrBtnDkShadow, globalData.clrBtnHilite);

				rectClient.DeflateRect (1, 1);

				if (m_nFlatStyle != BUTTONSTYLE_FLAT)
				{
					pDC->Draw3dRect (rectClient,
								globalData.clrBtnShadow, globalData.clrBtnLight);
				}

				rectClient.DeflateRect (1, 1);
			}
			else
			{
				rectClient.DeflateRect (2, 2);
			}

			if (!m_bWinXPTheme || m_bDontUseWinXPTheme || globalData.IsHighContastMode())
			{
				rectClient.left += m_sizePushOffset.cx;
				rectClient.top += m_sizePushOffset.cy;
			}
		}
		else if (!bBorderIsReady && (m_nFlatStyle != BUTTONSTYLE_FLAT || m_bHighlighted))
		{
			pDC->Draw3dRect (rectClient,
						globalData.clrBtnHilite, 
						globalData.clrBtnDkShadow);
			rectClient.DeflateRect (1, 1);

			if (m_nFlatStyle == BUTTONSTYLE_3D ||
				(m_nFlatStyle == BUTTONSTYLE_SEMIFLAT && m_bHighlighted))
			{
				pDC->Draw3dRect (rectClient,
							globalData.clrBtnLight, globalData.clrBtnShadow);
			}

			rectClient.DeflateRect (1, 1);
		}
		else
		{
			rectClient.DeflateRect (2, 2);
		}
	}
	else
	{
		rectClient.DeflateRect (2, 2);
	}
}
//*********************************************************************************
LRESULT CBCGPButton::OnSetImage (WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case IMAGE_BITMAP:
		SetImage ((HBITMAP) lParam, FALSE);
		break;
		
	case IMAGE_ICON:
		SetImage ((HICON) lParam, FALSE);
		break;
		
	default:
		TRACE1 ("Error: unknown image type '%u'\n", (unsigned) wParam);
	}

	return 0;
}
//*********************************************************************************
LRESULT CBCGPButton::OnGetImage (WPARAM wParam, LPARAM)
{
	switch (wParam)
	{
	case IMAGE_BITMAP:
		return (LRESULT) m_Image.GetImageWell ();
		
	case IMAGE_ICON:
		return (LRESULT) m_Image.ExtractIcon (0);
		
	default:
		TRACE1 ("Error: unknown image type '%u'\n", (unsigned) wParam);
	}

	return 0;
}
//************************************************************************************
int CBCGPButton::GetImageHorzMargin () const
{ 
	return nImageHorzMargin; 
}
//************************************************************************************
int CBCGPButton::GetVertMargin () const
{ 
	return nVertMargin; 
}
//*************************************************************************************
void CBCGPButton::OnDestroy() 
{
	CBCGPTooltipManager::DeleteToolTip (m_pToolTip);
	CButton::OnDestroy();
}
//**************************************************************************
LRESULT CBCGPButton::OnBCGUpdateToolTips (WPARAM wp, LPARAM)
{
	UINT nTypes = (UINT) wp;

	if (m_pToolTip->GetSafeHwnd () == NULL)
	{
		return 0;
	}

	if (nTypes & BCGP_TOOLTIP_TYPE_BUTTON)
	{
		CString str;
		m_pToolTip->GetText (str, this);

		CBCGPTooltipManager::CreateToolTip (m_pToolTip, this,
			BCGP_TOOLTIP_TYPE_BUTTON);

		if (m_pToolTip != NULL)
		{
			m_pToolTip->AddTool (this, str);
		}
	}

	return 0;
}
//**************************************************************************
LRESULT CBCGPButton::OnBCGSetControlVMMode (WPARAM wp, LPARAM)
{
	m_bVisualManagerStyle = (BOOL) wp;
	return 0;
}
//**************************************************************************
LRESULT CBCGPButton::OnBCGSetControlAero (WPARAM wp, LPARAM)
{
	m_bOnGlass = (BOOL) wp;
	return 0;
}
//**************************************************************************
LRESULT CBCGPButton::OnBCGSetControlBackStageMode (WPARAM, LPARAM)
{
	m_bBackstageMode = TRUE;
	return 0;
}
//**************************************************************************
UINT CBCGPButton::OnGetDlgCode() 
{
	UINT uRes = CButton::OnGetDlgCode();

	if (m_bRadioButton)
	{
		uRes |= DLGC_RADIOBUTTON;
	}

	return uRes;
}
//**************************************************************************
void CBCGPButton::OnPaint() 
{
	const BOOL bIsRadio = m_bRadioButton && !m_bAutoCheck;

	if (!m_b3State && !bIsRadio)
	{
		Default ();
		return;
	}

	CPaintDC dc(this); // device context for painting
	
	CRect rectClient;
	GetClientRect (&rectClient);

	DoDrawItem (&dc, rectClient, (CWnd::GetFocus () == this) ? ODS_FOCUS : 0);
}
//**************************************************************************
void CBCGPButton::SetAutoCheck(BOOL bSet)
{
	m_bAutoCheck = bSet;

	if (GetSafeHwnd() != NULL)
	{
		RedrawWindow();
	}
}
//**************************************************************************
LRESULT CBCGPButton::OnPrintClient(WPARAM wp, LPARAM lp)
{
	if ((lp & PRF_CLIENT) == PRF_CLIENT && m_b3State)
	{
		CDC* pDC = CDC::FromHandle((HDC) wp);
		ASSERT_VALID(pDC);
		
		CRect rectClient;
		GetClientRect (&rectClient);
		
		DoDrawItem (pDC, rectClient, (CWnd::GetFocus () == this) ? ODS_FOCUS : 0);
		return 0;
	}
	
	return Default();
}

