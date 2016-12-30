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
// BCGPToolTipCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "multimon.h"
#include "BCGPToolTipCtrl.h"

#include "BCGPToolBarImages.h"
#include "BCGPToolBar.h"
#include "BCGPToolBarButton.h"
#include "BCGPDrawManager.h"
#include "BCGPVisualManager.h"
#include "BCGPOutlookBarPane.h"
#include "BCGPRibbonButton.h"
#include "BCGPRibbonBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifndef TTS_BALLOON
#define TTS_BALLOON 0x40
#endif

#ifndef TTM_SETTITLE
#define TTM_SETTITLE (WM_USER + 32)
#endif

/////////////////////////////////////////////////////////////////////////////
// CBCGPToolTipCtrl

IMPLEMENT_DYNCREATE(CBCGPToolTipCtrl, CToolTipCtrl)

CBCGPToolTipCtrl::CBCGPToolTipCtrl(CBCGPToolTipParams* pParams/* = NULL*/)
{
	m_bResetDelayTime = FALSE;

	SetParams (pParams);

	m_pToolBar = NULL;
	m_pToolBarImages = NULL;
	m_pHotButton = NULL;
	m_sizeImage = CSize (0, 0);
	m_sizeCornerRadius = CSize (0, 0);
	m_ptMargin = CPoint (0, 0);
	m_ptLocation = CPoint (-1, -1);
	m_pRibbonButton = NULL;
	m_nRibbonImageType = 0;
	m_nFixedWidthRegular = 0;
	m_nFixedWidthWithImage = 0;
}
//**********************************************************************
CBCGPToolTipCtrl::~CBCGPToolTipCtrl()
{
}

BEGIN_MESSAGE_MAP(CBCGPToolTipCtrl, CToolTipCtrl)
	//{{AFX_MSG_MAP(CBCGPToolTipCtrl)
	ON_WM_PAINT()
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
	ON_NOTIFY_REFLECT(TTN_SHOW, OnShow)
	ON_NOTIFY_REFLECT(TTN_POP, OnPop)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGPToolTipCtrl message handlers

void CBCGPToolTipCtrl::OnPaint() 
{
	if (m_Params.m_bBallonTooltip)
	{
		CToolTipCtrl::OnPaint ();
		return;
	}

	CPaintDC dcPaint(this); // device context for painting
	
	CBCGPMemDC memDC (dcPaint, this);
	CDC* pDC = &memDC.GetDC ();

	CRect rect;
	GetClientRect (rect);

	CRect rectMargin;
	GetMargin (rectMargin);

	CRect rectText = rect;

	rectText.DeflateRect (rectMargin);
	rectText.DeflateRect (m_ptMargin.x, m_ptMargin.y);

	COLORREF clrLine = m_Params.m_clrBorder == (COLORREF)-1 ?
		::GetSysColor (COLOR_INFOTEXT) : m_Params.m_clrBorder;

	COLORREF clrText = m_Params.m_clrText == (COLORREF)-1 ?
		::GetSysColor (COLOR_INFOTEXT) : m_Params.m_clrText;

	//-----------------
	// Fill background:
	//-----------------
	OnFillBackground (pDC, rect, clrText, clrLine);

	CPen penLine (PS_SOLID, 1, clrLine);
	CPen* pOldPen = pDC->SelectObject (&penLine);

	//-------------
	// Draw border:
	//-------------
	OnDrawBorder (pDC, rect, clrLine);

	//-----------
	// Draw icon:
	//-----------
	if (m_sizeImage != CSize (0, 0) && m_Params.m_bDrawIcon)
	{
		CRect rectImage = rectText;
		rectImage.right = rectImage.left + m_sizeImage.cx;
		rectImage.bottom = rectImage.top + m_sizeImage.cy;

		OnDrawIcon (pDC, rectImage);

		rectText.left += m_sizeImage.cx + m_ptMargin.x;
	}

	pDC->SetBkMode (TRANSPARENT);
	pDC->SetTextColor (clrText);

	//------------
	// Draw label:
	//------------
	int nTextHeight = OnDrawLabel (pDC, rectText, FALSE).cy;

	//------------------------------
	// Draw separator + description:
	//------------------------------
	if (!m_strDescription.IsEmpty () && m_Params.m_bDrawDescription)
	{
		CRect rectDescr = rectText;
		rectDescr.top += nTextHeight + 3 * m_ptMargin.y / 2;

		if (m_Params.m_bDrawSeparator)
		{
			OnDrawSeparator (pDC, rectDescr.left,
				rectDescr.right, rectDescr.top - m_ptMargin.y / 2);
		}

		OnDrawDescription (pDC, rectDescr, FALSE);
	}

	pDC->SelectObject (pOldPen);
}
//**********************************************************************
void CBCGPToolTipCtrl::OnShow(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	*pResult = 0;

	if (m_Params.m_bVislManagerTheme)
	{
		CBCGPVisualManager::GetInstance ()->GetToolTipParams (m_Params);
		m_Params.m_bVislManagerTheme = TRUE;
	}

	if (m_Params.m_bBallonTooltip)
	{
		return;
	}

	CPoint ptCursor;
	::GetCursorPos (&ptCursor);

	GetHotButton ();

	m_sizeImage = m_Params.m_bDrawIcon ? GetIconSize () : CSize (0, 0);
	m_ptMargin = CPoint (6, 4);

	m_ptMargin.x += m_Params.m_nPaddingX;
	m_ptMargin.y += m_Params.m_nPaddingY;

	CRect rectMargin;
	GetMargin (rectMargin);

	CRect rectText;
	GetClientRect (rectText);

	CClientDC dc (this);
	CSize sizeText = OnDrawLabel (&dc, rectText, TRUE);
	
	int cx = sizeText.cx;
	int cy = sizeText.cy;

	CSize sizeDescr (0, 0);
	
	if (!m_Params.m_bDrawDescription || m_strDescription.IsEmpty ())
	{
		cy = max (cy, m_sizeImage.cy);
	}
	else
	{
		sizeDescr = OnDrawDescription (&dc, rectText, TRUE);

		cy += sizeDescr.cy + 2 * m_ptMargin.y;
		cx = max (cx, sizeDescr.cx);

		cy = max (cy, m_sizeImage.cy);
	}

	if (m_sizeImage.cx > 0 && m_Params.m_bDrawIcon)
	{
		cx += m_sizeImage.cx + m_ptMargin.x;
	}

	cx += 2 * m_ptMargin.x;
	cy += 2 * m_ptMargin.y;

	int nFixedWidth = GetFixedWidth ();
	if (nFixedWidth > 0 && sizeDescr != CSize (0, 0))
	{
		cx = max (cx, nFixedWidth);
	}

	CRect rectWindow;
	GetWindowRect (rectWindow);

	int x = rectWindow.left;
	int y = rectWindow.top;

	if (m_ptLocation != CPoint (-1, -1))
	{
		x = m_ptLocation.x;
		y = m_ptLocation.y;

		*pResult = 1;
	}

	CRect rectScreen;

	MONITORINFO mi;
	mi.cbSize = sizeof (MONITORINFO);
	if (GetMonitorInfo (MonitorFromPoint (rectWindow.TopLeft (), MONITOR_DEFAULTTONEAREST),
		&mi))
	{
		rectScreen = mi.rcWork;
	}
	else
	{
		::SystemParametersInfo (SPI_GETWORKAREA, 0, &rectScreen, 0);
	}

	int nBottom = max (
		ptCursor.y + cy + ::GetSystemMetrics (SM_CYCURSOR),
		y + cy + 2);

	if (nBottom > rectScreen.bottom)
	{
		y = ptCursor.y - cy - 1;

#ifndef BCGP_EXCLUDE_RIBBON
		if (m_pRibbonButton != NULL && m_ptLocation != CPoint (-1, -1))
		{
			ASSERT_VALID (m_pRibbonButton);

			CBCGPRibbonBar* pRibbon = m_pRibbonButton->GetTopLevelRibbonBar ();
			if (pRibbon->GetSafeHwnd () != NULL)
			{
				CRect rectRibbon;
				pRibbon->GetWindowRect (rectRibbon);

				y = rectRibbon.top - cy;
			}
		}
#endif

		*pResult = 1;
	}

	if (x + cx + 2 > rectScreen.right)
	{
		if ((*pResult) == 1)	// Y has been changed
		{
			x = ptCursor.x - cx - 1;
		}
		else
		{
			x = rectScreen.right - cx - 1;
			*pResult = 1;
		}
	}

	if ((*pResult) == 1)
	{
		SetWindowPos (NULL, x, y, cx, cy, SWP_NOZORDER | SWP_NOACTIVATE);
	}
	else
	{
		SetWindowPos (NULL, -1, -1, cx, cy, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
	}

	m_sizeCornerRadius = CBCGPVisualManager::GetInstance()->GetSystemToolTipCornerRadius(this);
	
	SetWindowPos (&wndTop, -1, -1, -1, -1, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOSIZE | SWP_DRAWFRAME);
}
//**********************************************************************
void CBCGPToolTipCtrl::OnPop(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	m_pHotButton = NULL;
	m_pToolBarImages = NULL;
	m_strDescription.Empty ();
	m_ptLocation = CPoint (-1, -1);
	m_pRibbonButton = NULL;
	m_nRibbonImageType = 0;
	*pResult = 0;
}
//**********************************************************************
void CBCGPToolTipCtrl::OnFillBackground (CDC* pDC, CRect rect, COLORREF& clrText, COLORREF& clrLine)
{
	ASSERT_VALID (pDC);

	if (m_Params.m_clrFill == (COLORREF)-1)
	{
		CBCGPVisualManager::GetInstance ()->OnFillToolTip (pDC, this, rect,
			clrText, clrLine);
	}
	else
	{
		if (m_Params.m_clrFillGradient == (COLORREF)-1)
		{
			CBrush br (m_Params.m_clrFill);
			pDC->FillRect (rect, &br);
		}
		else
		{
			CBCGPDrawManager dm (*pDC);

			dm.FillGradient2 (rect, 
				m_Params.m_clrFillGradient, m_Params.m_clrFill,
				m_Params.m_nGradientAngle == -1 ? 90 : m_Params.m_nGradientAngle);
		}
	}
}
//**********************************************************************
CSize CBCGPToolTipCtrl::GetIconSize ()
{
#ifndef BCGP_EXCLUDE_RIBBON
	if (m_pRibbonButton != NULL)
	{
		ASSERT_VALID (m_pRibbonButton);
		return m_pRibbonButton->GetToolTipImageSize(m_nRibbonImageType);
	}
#endif

	if (m_pHotButton == NULL || m_pToolBarImages == NULL ||
		m_pToolBarImages->GetCount () == 0)
	{
		return CSize (0, 0);
	}

	ASSERT_VALID (m_pHotButton);

	CSize size = m_pHotButton->GetImage () >= 0 ? m_pToolBarImages->GetImageSize () : CSize (0, 0);
	if (size != CSize(0, 0))
	{
		double scale = globalData.GetRibbonImageScale ();
		if (scale > 1.)
		{
			size.cx = (int)(.5 + scale * size.cx);
			size.cy = (int)(.5 + scale * size.cy);
		}
	}

	return size;
}
//**********************************************************************
void CBCGPToolTipCtrl::OnDrawBorder (CDC* pDC, CRect rect,
									 COLORREF clrLine)
{
	ASSERT_VALID (pDC);

	const int nOffsetX = m_sizeCornerRadius.cx / 2;
	const int nOffsetY = m_sizeCornerRadius.cy / 2;

	if (nOffsetX == 0 || nOffsetY == 0)
	{
		pDC->Draw3dRect (rect, clrLine, clrLine);
		return;
	}

	pDC->MoveTo (rect.left + nOffsetX, rect.top);
	pDC->LineTo (rect.right - nOffsetX - 1, rect.top);

	pDC->LineTo (rect.right - 1, rect.top + nOffsetY);
	pDC->LineTo (rect.right - 1, rect.bottom - 1 - nOffsetY);

	pDC->LineTo (rect.right - nOffsetX - 1, rect.bottom - 1);
	pDC->LineTo (rect.left + nOffsetX, rect.bottom - 1);

	pDC->LineTo (rect.left, rect.bottom - 1 - nOffsetY);
	pDC->LineTo (rect.left, rect.top + nOffsetY);

	pDC->LineTo (rect.left + nOffsetX, rect.top);
}
//**********************************************************************
BOOL CBCGPToolTipCtrl::OnDrawIcon (CDC* pDC, CRect rectImage)
{
	ASSERT_VALID (pDC);

#ifndef BCGP_EXCLUDE_RIBBON
	if (m_pRibbonButton != NULL)
	{
		ASSERT_VALID (m_pRibbonButton);
		m_pRibbonButton->OnDrawTooltipImage(pDC, (CBCGPBaseRibbonElement::RibbonImageType) m_nRibbonImageType, rectImage);

		return TRUE;
	}
#endif

	if (m_pHotButton == NULL)
	{
		return FALSE;
	}

	ASSERT_VALID (m_pHotButton);

	CBCGPToolBarImages* pImages = m_pToolBarImages;
	
	CBCGPToolBar* pToolBar = DYNAMIC_DOWNCAST(CBCGPToolBar, m_pHotButton->GetParentWnd());
	if (pToolBar != NULL)
	{
		ASSERT_VALID(pToolBar);
		pToolBar->CheckForButtonImages(m_pHotButton, &pImages);
	}

	if (pImages == NULL)
	{
		return FALSE;
	}

	ASSERT_VALID(pImages);

	CBCGPDrawState ds(CBCGPVisualManager::GetInstance()->IsAutoGrayscaleImages());
	pImages->PrepareDrawImage (ds);

	UINT nSaveStyle = m_pHotButton->m_nStyle;
	BOOL bSaveText = m_pHotButton->m_bText;
	BOOL bSaveImage = m_pHotButton->m_bImage;

	BOOL bSaveLargeIcons = CBCGPToolBar::m_bLargeIcons;
	CBCGPToolBar::m_bLargeIcons = FALSE;

	m_pHotButton->m_bText = FALSE;
	m_pHotButton->m_bImage = TRUE;

	m_pHotButton->m_nStyle = 0;

	m_pHotButton->CBCGPToolbarButton::OnDraw (pDC, rectImage, pImages);

	m_pHotButton->m_nStyle = nSaveStyle;
	m_pHotButton->m_bText = bSaveText;
	m_pHotButton->m_bImage = bSaveImage;

	CBCGPToolBar::m_bLargeIcons = bSaveLargeIcons;

	pImages->EndDrawImage (ds);
	return TRUE;
}
//**********************************************************************
CString CBCGPToolTipCtrl::GetLabel ()
{
	CString strText;
	GetWindowText (strText);

	return strText;
}
//**********************************************************************
CSize CBCGPToolTipCtrl::OnDrawLabel (CDC* pDC, CRect rect, BOOL bCalcOnly)
{
	ASSERT_VALID (pDC);

	CSize sizeText (0, 0);

	CString strText (GetLabel ());

	strText.Replace (_T("\t"), _T("    "));

	BOOL bDrawDescr = m_Params.m_bDrawDescription && !m_strDescription.IsEmpty ();

	CFont* pOldFont = (CFont*) pDC->SelectObject (
		m_Params.m_bBoldLabel && bDrawDescr ?
			&globalData.fontBold : &globalData.fontTooltip);

	if (strText.Find (_T('\n')) >= 0)	// Multi-line text
	{
		UINT nFormat = DT_NOPREFIX;
		if (bCalcOnly)
		{
			nFormat |= DT_CALCRECT;
		}

		int nHeight = pDC->DrawText (strText, rect, nFormat);
		sizeText = CSize (rect.Width (), nHeight);
	}
	else
	{
		if (bCalcOnly)
		{
			sizeText = pDC->GetTextExtent (strText);
		}
		else
		{
			UINT nFormat = DT_LEFT | DT_NOCLIP | DT_SINGLELINE;
		
			if (!bDrawDescr)
			{
				nFormat |= DT_VCENTER;
			}

			if (m_pRibbonButton != NULL)
			{
				nFormat |= DT_NOPREFIX;
			}

			sizeText.cy = pDC->DrawText (strText, rect, nFormat);
			sizeText.cx = rect.Width ();
		}
	}

	pDC->SelectObject (pOldFont);

	return sizeText;
}
//**********************************************************************
CSize CBCGPToolTipCtrl::OnDrawDescription (CDC* pDC, CRect rect, BOOL bCalcOnly)
{
	ASSERT_VALID (pDC);

	CSize sizeText (0, 0);

	if (!m_Params.m_bDrawDescription)
	{
		return sizeText;
	}

	CFont* pOldFont = pDC->SelectObject (&globalData.fontTooltip);
	int nFixedWidth = GetFixedWidth ();

	if (nFixedWidth > 0 && m_sizeImage.cx <= 32)
	{
		rect.right = rect.left + nFixedWidth;

		if (m_sizeImage.cx > 0 && m_Params.m_bDrawIcon)
		{
			rect.right -= m_sizeImage.cx + m_ptMargin.x;
		}
	}
	else
	{
		rect.right = rect.left + m_Params.m_nMaxDescrWidth;
	}

	UINT nFormat = DT_WORDBREAK;
	if (bCalcOnly)
	{
		nFormat |= DT_CALCRECT;
	}

	int nDescrHeight = pDC->DrawText (m_strDescription, rect, nFormat);
	pDC->SelectObject (pOldFont);

	return CSize (rect.Width (), nDescrHeight);
}
//**********************************************************************
void CBCGPToolTipCtrl::OnDrawSeparator (CDC* pDC, int x1, int x2, int y)
{
	ASSERT_VALID (pDC);

	pDC->MoveTo (x1, y);
	pDC->LineTo (x2, y);
}
//**********************************************************************
int CBCGPToolTipCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CToolTipCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	m_pToolBar = DYNAMIC_DOWNCAST (CBCGPToolBar,
		CWnd::FromHandlePermanent (lpCreateStruct->hwndParent));

	if (m_pToolBar != NULL && m_pToolBar->IsKindOf (RUNTIME_CLASS (CBCGPOutlookBarPane)))
	{
		m_pToolBar = NULL;
	}

	ModifyStyle (WS_BORDER, 0);

	if (m_Params.m_bBallonTooltip)
	{
		ModifyStyle (0, TTS_BALLOON);
	}

	AdjustDelayTime();
	return 0;
}
//**********************************************************************
BOOL CBCGPToolTipCtrl::OnEraseBkgnd(CDC* pDC) 
{
	CRect rect;
	GetClientRect (rect);

	COLORREF clrDummy;

	OnFillBackground (pDC, rect, clrDummy, clrDummy);
	return TRUE;
}
//**********************************************************************
void CBCGPToolTipCtrl::SetParams (CBCGPToolTipParams* pParams)
{
	ASSERT_VALID (this);

	BOOL bBallonTooltipSaved = m_Params.m_bBallonTooltip;
	BOOL bDelayWasSet = m_Params.m_nDelayAutomatic != 0 || m_Params.m_nDelayAutoPop != 0 || m_Params.m_nDelayInitial != 0 || m_Params.m_nDelayReshow != 0;

	if (pParams == NULL)
	{
		if (bDelayWasSet)
		{
			m_bResetDelayTime = TRUE;
		}

		CBCGPToolTipParams paramsDefault;
		m_Params = paramsDefault;
	}
	else
	{
		m_Params = *pParams;

		if (bDelayWasSet && m_Params.m_nDelayAutomatic == 0 && m_Params.m_nDelayAutoPop == 0 && m_Params.m_nDelayInitial == 0 && m_Params.m_nDelayReshow == 0)
		{
			m_bResetDelayTime = TRUE;
		}
	}

	if (bBallonTooltipSaved != m_Params.m_bBallonTooltip && GetSafeHwnd() != NULL)
	{
		if (m_Params.m_bBallonTooltip)
		{
			ModifyStyle (0, TTS_BALLOON);
		}
		else
		{
			ModifyStyle (TTS_BALLOON, 0);
		}
	}

	AdjustDelayTime();
}
//**********************************************************************
void CBCGPToolTipCtrl::SetDescription (const CString strDescription)
{
	ASSERT_VALID (this);

	GetHotButton ();
	m_strDescription = strDescription;

	m_strDescription.Replace (_T("\t"), _T("    "));
}
//**********************************************************************
void CBCGPToolTipCtrl::GetHotButton ()
{
	m_pHotButton = NULL;
	m_pToolBarImages = NULL;

	if (m_pRibbonButton != NULL)
	{
		return;
	}

	if (m_pToolBar != NULL)
	{
		CPoint ptToolBar;

		::GetCursorPos (&ptToolBar);
		m_pToolBar->ScreenToClient (&ptToolBar);

		m_pHotButton = m_pToolBar->GetButton (m_pToolBar->HitTest (ptToolBar));

		if (m_pHotButton != NULL)
		{
			if (m_pToolBar->IsLocked ())
			{
				m_pToolBarImages = m_pToolBar->GetLockedMenuImages ();
			}
			else
			{
				if (m_pHotButton->m_bUserButton)
				{
					m_pToolBarImages = CBCGPToolBar::GetUserImages ();
				}
				else
				{
					m_pToolBarImages = CBCGPToolBar::GetMenuImages ();
					if (m_pToolBarImages == NULL || m_pToolBarImages->GetCount () <= 0)
					{
						m_pToolBarImages = CBCGPToolBar::GetImages ();
					}
				}
			}
		}
	}
}
//**********************************************************************
void CBCGPToolTipCtrl::SetLocation (CPoint pt)
{
	ASSERT_VALID (this);

	m_ptLocation = pt;
}
//**********************************************************************
void CBCGPToolTipCtrl::SetHotRibbonButton (CBCGPRibbonButton* pRibbonButton)
{
	ASSERT_VALID (this);

	m_pRibbonButton = pRibbonButton;
}
//**********************************************************************
int CBCGPToolTipCtrl::GetFixedWidth ()
{
	ASSERT_VALID (this);

	if (m_sizeImage.cx <= (int) (globalData.GetRibbonImageScale () * 32))
	{
		return m_nFixedWidthRegular;
	}
	else
	{
		return m_nFixedWidthWithImage;
	}
}
//**********************************************************************
void CBCGPToolTipCtrl::AdjustDelayTime()
{
	if (GetSafeHwnd() == NULL)
	{
		return;
	}
	
	if (m_bResetDelayTime)
	{
		SetDelayTime(TTDT_AUTOMATIC, -1);
		m_bResetDelayTime = FALSE;
		return;
	}
	
	if (m_Params.m_nDelayAutomatic != 0)
	{
		SetDelayTime(TTDT_AUTOMATIC, m_Params.m_nDelayAutomatic);
		return;
	}
	
	if (m_Params.m_nDelayAutoPop != 0)
	{
		SetDelayTime(TTDT_AUTOPOP, m_Params.m_nDelayAutoPop);
	}
	
	if (m_Params.m_nDelayInitial != 0)
	{
		SetDelayTime(TTDT_INITIAL, m_Params.m_nDelayInitial);
	}
	
	if (m_Params.m_nDelayReshow != 0)
	{
		SetDelayTime(TTDT_RESHOW, m_Params.m_nDelayReshow);
	}
}
