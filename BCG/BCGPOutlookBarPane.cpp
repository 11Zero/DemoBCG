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
// BCGPOutlookBarPane.cpp : implementation file
//

#include "stdafx.h"
#include "bcgcbpro.h"
#include "MenuImages.h"
#include "bcgprores.h"
#include "BCGPLocalResource.h"
#include "BCGPWorkspace.h"
#include "BCGPRegistry.h"
#include "BCGCBProVer.h"
#include "BCGPVisualManager.h"
#include "BCGPMiniFrameWnd.h"
#include "BCGPOutlookBarPane.h"
#include "BCGPOutlookButton.h"
#include "BCGPOutlookBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define SCROLL_BUTTON_OFFSET	5

//------------------
// Timer event IDs:
//------------------
static const UINT idScrollUp	= 1;
static const UINT idScrollDn	= 2;

static const int nScrollButtonMargin = 3;

static const UINT uiScrollDelay = 200;		// ms

extern CBCGPWorkspace* g_pWorkspace;

CBCGPToolBarImages CBCGPOutlookBarPane::m_Images;
CSize CBCGPOutlookBarPane::m_csImage = CSize (0, 0);

/////////////////////////////////////////////////////////////////////////////
// CBCGPOutlookBarPane

IMPLEMENT_SERIAL(CBCGPOutlookBarPane, CBCGPToolBar, 1)

CBCGPOutlookBarPane::CBCGPOutlookBarPane()
{
	m_nSize = -1;

	m_iScrollOffset = 0;
	m_iFirstVisibleButton = 0;
	m_bScrollDown = FALSE;

	m_clrRegText = (COLORREF)-1;
	m_clrBackColor = (COLORREF)-1;

	m_clrTransparentColor = RGB (255, 0, 255);
	m_Images.SetTransparentColor (m_clrTransparentColor);

	m_uiBackImageId = 0;

	m_btnUp.m_nFlatStyle = CBCGPButton::BUTTONSTYLE_3D;
	m_btnUp.m_bDrawFocus = FALSE;
	m_btnUp.m_bVisualManagerStyle = TRUE;

	m_btnDown.m_nFlatStyle = CBCGPButton::BUTTONSTYLE_3D;
	m_btnDown.m_bDrawFocus = FALSE;
	m_btnDown.m_bVisualManagerStyle = TRUE;

	m_bDrawShadedHighlight = FALSE;

	m_bDisableControlsIfNoHandler = FALSE;
	m_nExtraSpace = 0;
	m_hRecentOutlookWnd = NULL;

	m_bPageScrollMode = FALSE;
	m_bDontAdjustLayout = FALSE;

	m_bLocked = TRUE;
}

CBCGPOutlookBarPane::~CBCGPOutlookBarPane()
{
}

BEGIN_MESSAGE_MAP(CBCGPOutlookBarPane, CBCGPToolBar)
	//{{AFX_MSG_MAP(CBCGPOutlookBarPane)
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_CREATE()
	ON_WM_NCCALCSIZE()
	ON_WM_SYSCOLORCHANGE()
	ON_WM_TIMER()
	ON_WM_LBUTTONUP()
	ON_WM_SETFOCUS()
	ON_WM_NCDESTROY()
	ON_WM_CONTEXTMENU()
    ON_WM_NCPAINT()
	ON_WM_MOUSEWHEEL()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


BOOL CBCGPOutlookBarPane::Create(CWnd* pParentWnd,
		DWORD dwStyle/* = dwDefaultToolbarStyle*/,
		UINT uiID/* = (UINT)-1*/,
		DWORD dwBCGStyle/* = 0*/)
{
	if (!CBCGPToolBar::Create (pParentWnd, dwStyle, uiID))
	{
		return FALSE;
	}

	m_dwBCGStyle = dwBCGStyle;
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPOutlookBarPane message handlers

//*************************************************************************************
BOOL CBCGPOutlookBarPane::OnEraseBkgnd(CDC* /*pDC*/) 
{
	return TRUE;
}
//*************************************************************************************
BOOL CBCGPOutlookBarPane::AddButton (LPCTSTR szBmpFileName, LPCTSTR szLabel, 
								UINT iIdCommand, int iInsertAt)
//
// Adds a button by loading the image from disk instead of a resource
//
{
	ASSERT (szBmpFileName != NULL);

	HBITMAP hBmp = (HBITMAP) ::LoadImage (	
								NULL, szBmpFileName, IMAGE_BITMAP, 0, 0, 
								LR_DEFAULTSIZE | LR_LOADFROMFILE); 
	if (hBmp == NULL)
	{
		TRACE(_T("Can't load bitmap resource: %s"), szBmpFileName);
		ASSERT (FALSE);

		return FALSE;
	}

	int iImageIndex = AddBitmapImage (hBmp);
	ASSERT (iImageIndex >= 0);

	::DeleteObject (hBmp);

	return InternalAddButton (iImageIndex, szLabel, iIdCommand, iInsertAt);
}
//*************************************************************************************
BOOL CBCGPOutlookBarPane::AddButton (UINT uiImage, UINT uiLabel, UINT iIdCommand, 
								int iInsertAt)
{
	CString strLable;
	strLable.LoadString (uiLabel);

	return AddButton (uiImage, strLable, iIdCommand, iInsertAt);
}
//*************************************************************************************
BOOL CBCGPOutlookBarPane::AddButton (UINT uiImage, LPCTSTR lpszLabel, UINT iIdCommand,
								int iInsertAt)
{
	int iImageIndex = -1;
	if (uiImage != 0)
	{
		CBitmap bmp;
		if (!bmp.LoadBitmap (uiImage))
		{
			TRACE(_T("Can't load bitmap resource: %d"), uiImage);
			return FALSE;
		}

		iImageIndex = AddBitmapImage ((HBITMAP) bmp.GetSafeHandle ());
	}

	return InternalAddButton (iImageIndex, lpszLabel, iIdCommand, iInsertAt);
}
//**************************************************************************************
BOOL CBCGPOutlookBarPane::AddButton (HBITMAP hBmp, LPCTSTR lpszLabel, UINT iIdCommand, int iInsertAt)
{
	ASSERT (hBmp != NULL);

	int iImageIndex = AddBitmapImage (hBmp);
	return InternalAddButton (iImageIndex, lpszLabel, iIdCommand, iInsertAt);
}
//**************************************************************************************
BOOL CBCGPOutlookBarPane::AddButton (HICON hIcon, LPCTSTR lpszLabel, UINT iIdCommand, int iInsertAt,
									 BOOL bAlphaBlend)
{
	ASSERT (hIcon != NULL);

	int iImageIndex = -1;

	ICONINFO iconInfo;
	::GetIconInfo (hIcon, &iconInfo);

	BITMAP bitmap;
	::GetObject (iconInfo.hbmColor, sizeof (BITMAP), &bitmap);

	CSize size (bitmap.bmWidth, bitmap.bmHeight);

	if (bAlphaBlend)
	{
		if (m_Images.GetCount() == 0)	// First image
		{
			m_csImage = size;
			m_Images.SetImageSize (size);
		}

		iImageIndex = m_Images.AddIcon (hIcon, TRUE);
	}
	else
	{
		CClientDC dc (this);

		CDC dcMem;
		dcMem.CreateCompatibleDC (&dc);

		CBitmap bmp;
		bmp.CreateCompatibleBitmap (&dc, size.cx, size.cy);

		CBitmap* pOldBmp = dcMem.SelectObject (&bmp);

		if (m_clrTransparentColor != (COLORREF)-1)
		{
			dcMem.FillSolidRect (0, 0, size.cx, size.cy, m_clrTransparentColor);
		}

		::DrawIconEx (dcMem.GetSafeHdc (), 0, 0, hIcon, size.cx, size.cy,
			0, NULL, DI_NORMAL);

		dcMem.SelectObject (pOldBmp);

		iImageIndex = AddBitmapImage ((HBITMAP) bmp.GetSafeHandle ());
	}

	::DeleteObject (iconInfo.hbmColor);
	::DeleteObject (iconInfo.hbmMask);

	return InternalAddButton (iImageIndex, lpszLabel, iIdCommand, iInsertAt);
}
//**************************************************************************************
BOOL CBCGPOutlookBarPane::RemoveButton (UINT iIdCommand)
{
	for (POSITION pos = m_Buttons.GetHeadPosition (); pos != NULL;)
	{
		POSITION posSave = pos;

		CBCGPOutlookButton* pButton = (CBCGPOutlookButton*) m_Buttons.GetNext (pos);
		ASSERT (pButton != NULL);

		if (pButton->m_nID == iIdCommand)
		{
			m_Buttons.RemoveAt (posSave);
			delete pButton;

			if (GetSafeHwnd () != NULL)
			{
				AdjustLocations ();
				UpdateWindow ();
				Invalidate ();
			}

			return TRUE;
		}
	}

	return FALSE;
}
//**************************************************************************************
BOOL CBCGPOutlookBarPane::RemoveButtonByIndex(int nIndex)
{
	POSITION pos = m_Buttons.FindIndex(nIndex);
	if (pos == NULL)
	{
		return FALSE;
	}

	CBCGPOutlookButton* pButton = (CBCGPOutlookButton*)m_Buttons.GetAt(pos);
	ASSERT_VALID(pButton);

	m_Buttons.RemoveAt(pos);
	delete pButton;

	if (GetSafeHwnd () != NULL)
	{
		AdjustLocations();
		UpdateWindow();
		Invalidate();
	}

	return TRUE;
}
//*************************************************************************************
BOOL CBCGPOutlookBarPane::InternalAddButton (int iImageIndex, LPCTSTR lpszLabel,
										UINT iIdCommand, int iInsertAt)
{
	CBCGPOutlookButton* pButton = new CBCGPOutlookButton;
	ASSERT (pButton != NULL);

	pButton->m_nID = iIdCommand;
	pButton->m_strText = (lpszLabel == NULL) ? _T("") : lpszLabel;
	pButton->SetImage (iImageIndex);
	pButton->m_bTextBelow = m_bTextLabels;

	if (iInsertAt == -1)
	{
		iInsertAt = (int) m_Buttons.GetCount ();
	}
	
	InsertButton (pButton, iInsertAt);

	AdjustLayout ();
	return TRUE;
}
//*************************************************************************************
int CBCGPOutlookBarPane::AddBitmapImage (HBITMAP hBitmap)
{
	ASSERT (hBitmap != NULL);

	BITMAP	bitmap;
	::GetObject (hBitmap, sizeof (BITMAP), &bitmap);

	CSize csImage = CSize (bitmap.bmWidth, bitmap.bmHeight);

	if (m_Images.GetCount() == 0)	// First image
	{
		m_csImage = csImage;
		m_Images.SetImageSize(csImage);
	}
	else
	{
		ASSERT (m_csImage == csImage);	// All buttons should be of the same size!
	}

	return m_Images.AddImage (hBitmap);
}
//*************************************************************************************
void CBCGPOutlookBarPane::OnSize(UINT nType, int cx, int cy) 
{
	CBCGPToolBar::OnSize(nType, cx, cy);

	if (!m_bDontAdjustLayout)
	{
		AdjustLayout ();
	}
	else
	{
		AdjustLocations ();
	}

	int iButtons = (int) m_Buttons.GetCount ();
	if (iButtons > 0)
	{
		POSITION posLast = m_Buttons.FindIndex (iButtons - 1);
		CBCGPOutlookButton* pButtonLast = (CBCGPOutlookButton*) m_Buttons.GetAt (posLast);
		ASSERT (pButtonLast != NULL);

		while (m_iScrollOffset > 0 &&
			pButtonLast->Rect ().bottom < cy)
		{
			ScrollUp ();
		}
	}
}
//*************************************************************************************
void CBCGPOutlookBarPane::SetTextColor (COLORREF clrRegText, COLORREF/* clrSelText obsolete*/)
{
	m_clrRegText = clrRegText;
	if (GetSafeHwnd () != NULL)
	{
		Invalidate ();
		UpdateWindow ();
	}
}
//*************************************************************************************
int CBCGPOutlookBarPane::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CBCGPToolBar::OnCreate(lpCreateStruct) == -1)
		return -1;

	CBCGPGestureConfig gestureConfig;
	gestureConfig.EnablePan(TRUE, BCGP_GC_PAN_WITH_SINGLE_FINGER_VERTICALLY | BCGP_GC_PAN_WITH_GUTTER | BCGP_GC_PAN_WITH_INERTIA);
	
	bcgpGestureManager.SetGestureConfig(GetSafeHwnd(), gestureConfig);

	SetBarStyle (m_dwStyle & ~(CBRS_BORDER_ANY | CBRS_GRIPPER));

	m_cxLeftBorder = m_cxRightBorder = 0;
	m_cyTopBorder = m_cyBottomBorder = 0;

	//-------------------------------------------
	// Adjust Z-order in the parent frame window:
	//-------------------------------------------
	SetWindowPos(&wndBottom, 0,0,0,0,SWP_NOSIZE|SWP_NOMOVE | SWP_NOACTIVATE);

	//-----------------------
	// Create scroll buttons:
	//-----------------------
	CRect rectDummy (CPoint (0, 0), CBCGPMenuImages::Size ());
	rectDummy.InflateRect (nScrollButtonMargin, nScrollButtonMargin);

	m_btnUp.Create (_T(""), WS_CHILD | BS_PUSHBUTTON, rectDummy,
					this, (UINT)-1);
	m_btnUp.SetStdImage (CBCGPMenuImages::IdArowUpLarge);

	m_btnDown.Create (_T(""), WS_CHILD | BS_PUSHBUTTON, rectDummy,
					this, (UINT)-1);
	m_btnDown.SetStdImage (CBCGPMenuImages::IdArowDownLarge);

	return 0;
}
//*********************************************************************************
BOOL CBCGPOutlookBarPane::OnGestureEventPan(const CPoint& ptFrom, const CPoint& ptTo, CSize& sizeOverPan)
{
	return InternalScroll(ptTo.y - ptFrom.y, sizeOverPan);
}
//*************************************************************************************
BOOL CBCGPOutlookBarPane::InternalScroll(int nDelta, CSize& sizeOverPan)
{
	if (!m_bScrollDown && nDelta < 0)
	{
		sizeOverPan.cy = nDelta;
		return TRUE;
	}
	
	int nOffsetOld = m_iScrollOffset;
	int nOffsetNew = nOffsetOld - nDelta;
	int nOverPan = 0;
	
	if (nOffsetNew < 0)
	{
		nOverPan = -nOffsetNew;
		nOffsetNew = 0;
		nDelta = nOffsetOld;
	}
	
	if (nOffsetNew != m_iScrollOffset)
	{
		m_iScrollOffset = nOffsetNew;
		m_iFirstVisibleButton = 0;
		
		if (nOffsetNew > 0)
		{
			CRect rectClient;
			GetClientRect(rectClient);
			
			for (POSITION pos = m_Buttons.GetHeadPosition (); pos != NULL; m_iFirstVisibleButton++)
			{
				CBCGPOutlookButton* pButton = (CBCGPOutlookButton*) m_Buttons.GetNext (pos);
				ASSERT (pButton != NULL);
				
				if (pButton->Rect().top + nDelta >= rectClient.top - 1)
				{
					break;
				}
			}
		}
		
		AdjustLocations ();
		RedrawWindow();
		return TRUE;
	}
	
	if (nOverPan != 0)
	{
		sizeOverPan.cy = nOverPan;
		return TRUE;
	}
	
	return FALSE;
}
void CBCGPOutlookBarPane::ScrollUp ()
{
	if (m_iScrollOffset <= 0 ||
		m_iFirstVisibleButton <= 0)
	{
		m_iScrollOffset = 0;
		m_iFirstVisibleButton = 0;

		KillTimer (idScrollUp);
		return;
	}

	CBCGPToolbarButton* pFirstVisibleButton = GetButton (m_iFirstVisibleButton);
	if (pFirstVisibleButton == NULL)
	{
		KillTimer (idScrollDn);
		return;
	}

	m_iFirstVisibleButton--;
	m_iScrollOffset -= pFirstVisibleButton->Rect ().Height ();

	if (m_iFirstVisibleButton == 0)
	{
		m_iScrollOffset = 0;
	}

	ASSERT (m_iScrollOffset >= 0);

	AdjustLocations ();
	Invalidate ();
	UpdateWindow ();
}
//*************************************************************************************
void CBCGPOutlookBarPane::ScrollDown ()
{
	if (!m_bScrollDown ||
		m_iFirstVisibleButton + 1 >= GetCount ())
	{
		KillTimer (idScrollDn);
		return;
	}

	CBCGPToolbarButton* pFirstVisibleButton = GetButton (m_iFirstVisibleButton);
	if (pFirstVisibleButton == NULL)
	{
		KillTimer (idScrollDn);
		return;
	}

	m_iFirstVisibleButton++;
	m_iScrollOffset += pFirstVisibleButton->Rect ().Height ();

	AdjustLocations ();
	Invalidate ();
	UpdateWindow ();
}
//*************************************************************************************
CSize CBCGPOutlookBarPane::CalcFixedLayout (BOOL /*bStretch*/, BOOL /*bHorz*/)
{
	CRect rect;
	GetClientRect (rect);

	return rect.Size ();
}
//*************************************************************************************
void CBCGPOutlookBarPane::OnNcCalcSize(BOOL /*bCalcValidRects*/, NCCALCSIZE_PARAMS FAR* lpncsp) 
{
	CRect rect; 
	rect.SetRectEmpty();

	CBCGPControlBar::CalcInsideRect(rect, FALSE);

	// adjust non-client area for border space
	lpncsp->rgrc[0].left += rect.left;
	lpncsp->rgrc[0].top += rect.top;
	lpncsp->rgrc[0].right += rect.right;
	lpncsp->rgrc[0].bottom += rect.bottom;
}
//*************************************************************************************
void CBCGPOutlookBarPane::SetBackImage (UINT uiImageID)
{
	if (m_uiBackImageId == uiImageID)
	{
		return;
	}

	m_bDrawShadedHighlight = FALSE;
	if (m_bmpBack.GetCount () > 0)
	{
		m_bmpBack.Clear ();
	}

	m_uiBackImageId = 0;
	if (uiImageID != 0)
	{
		HBITMAP hbmp = (HBITMAP) ::LoadImage (
				AfxGetResourceHandle (),
				MAKEINTRESOURCE (uiImageID),
				IMAGE_BITMAP,
				0, 0,
				LR_CREATEDIBSECTION | LR_LOADMAP3DCOLORS);
		if (hbmp != NULL)
		{
			BITMAP bitmap;
			::GetObject (hbmp, sizeof (BITMAP), (LPVOID) &bitmap);

			m_bmpBack.SetImageSize (CSize (bitmap.bmWidth, bitmap.bmHeight));
			m_bmpBack.AddImage (hbmp);
			m_uiBackImageId = uiImageID;
		}

		m_bDrawShadedHighlight = (globalData.m_nBitsPerPixel > 8);	// For 16 bits or greater
	}

	if (GetSafeHwnd () != NULL)
	{
		Invalidate ();
		UpdateWindow ();
	}
}
//*************************************************************************************
void CBCGPOutlookBarPane::SetBackColor (COLORREF color)
{
	m_clrBackColor = color;

	if (GetSafeHwnd () != NULL)
	{
		Invalidate ();
		UpdateWindow ();
	}
}
//*************************************************************************************
void CBCGPOutlookBarPane::SetTransparentColor (COLORREF color)
{
	m_clrTransparentColor = color;
	if (GetSafeHwnd () != NULL)
	{
		m_Images.SetTransparentColor (m_clrTransparentColor);
		Invalidate ();
		UpdateWindow ();
	}
}
//*************************************************************************************
void CBCGPOutlookBarPane::OnSysColorChange() 
{
	CBCGPToolBar::OnSysColorChange();
	
	if (m_uiBackImageId != 0)
	{
		int uiImage = m_uiBackImageId;
		m_uiBackImageId = (UINT) -1;

		SetBackImage (uiImage);
	}
	else
	{
		Invalidate ();
	}
}
//*****************************************************************************************
void CBCGPOutlookBarPane::AdjustLocations ()
{
	if (GetSafeHwnd () == NULL)
	{
		return;
	}

	ASSERT_VALID(this);


	CSize sizeBtn = CBCGPMenuImages::Size () + CSize (2 * nScrollButtonMargin, 2 * nScrollButtonMargin);

	CClientDC dc (this);
	CFont* pOldFont = dc.SelectObject (&globalData.fontRegular);

	CRect rectClient;
	GetClientRect (rectClient);

	CSize sizeDefault = CSize (rectClient.Width () - 2, m_csImage.cy);

	if (IsButtonExtraSizeAvailable ())
	{
		sizeDefault += CBCGPVisualManager::GetInstance ()->GetButtonExtraBorder ();
	}

	int iOffset = rectClient.top - m_iScrollOffset + m_nExtraSpace;

	if (m_iFirstVisibleButton > 0 && 
		sizeBtn.cx <= rectClient.Width () - SCROLL_BUTTON_OFFSET && 
		sizeBtn.cy <= rectClient.Height () - SCROLL_BUTTON_OFFSET)
	{
		int nAdjButton = SCROLL_BUTTON_OFFSET;

		m_btnUp.SetWindowPos (NULL, 
				rectClient.right - sizeBtn.cx - nAdjButton,
				rectClient.top + SCROLL_BUTTON_OFFSET,
				-1, -1,
				SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);

		m_btnUp.ShowWindow (SW_SHOWNOACTIVATE);
	}
	else
	{
		m_btnUp.ShowWindow (SW_HIDE);
	}

	for (POSITION pos = m_Buttons.GetHeadPosition (); pos != NULL;)
	{
		CBCGPOutlookButton* pButton = (CBCGPOutlookButton*) m_Buttons.GetNext (pos);
		ASSERT (pButton != NULL);

		pButton->m_bTextBelow = m_bTextLabels;
		pButton->m_sizeImage = m_csImage;

		CSize sizeButton = pButton->OnCalculateSize (&dc, sizeDefault, FALSE);

		CRect rectButton;

		int nWidth = rectClient.Width () - 1;
		sizeButton.cx = min (nWidth, sizeButton.cx);

		rectButton = CRect (
			CPoint (rectClient.left + (nWidth - sizeButton.cx) / 2, iOffset), 
			sizeButton);
		iOffset = rectButton.bottom + m_nExtraSpace;

		pButton->SetRect (rectButton);
	}

	m_bScrollDown = (iOffset > rectClient.bottom);

	if (m_bScrollDown && 
		sizeBtn.cx <= rectClient.Width () - SCROLL_BUTTON_OFFSET && 
		sizeBtn.cy <= rectClient.Height () - SCROLL_BUTTON_OFFSET)
	{
		int nAdjButton = SCROLL_BUTTON_OFFSET;

		m_btnDown.SetWindowPos (&wndTop, rectClient.right - sizeBtn.cx - nAdjButton,
					rectClient.bottom - sizeBtn.cy - SCROLL_BUTTON_OFFSET,
					-1, -1,
					SWP_NOSIZE | SWP_NOACTIVATE);

		m_btnDown.ShowWindow (SW_SHOWNOACTIVATE);
	}
	else
	{
		m_btnDown.ShowWindow (SW_HIDE);
	}

	dc.SelectObject (pOldFont);

	m_btnUp.RedrawWindow ();
	m_btnDown.RedrawWindow ();

	OnMouseLeave (0, 0);
	UpdateTooltips ();
}
//*************************************************************************************
void CBCGPOutlookBarPane::DoPaint(CDC* pDCPaint)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDCPaint);

	CRect rectClip;

	GetClientRect (rectClip);

	CRect rectClient;
	GetClientRect (rectClient);

	CBCGPMemDC memDC (*pDCPaint, this);
	CDC* pDC = &memDC.GetDC ();

	if (m_bmpBack.GetCount () > 0)
	{
		ASSERT (m_bmpBack.GetCount () == 1);
		
		CBCGPDrawState ds;
		m_bmpBack.PrepareDrawImage (ds);
		CSize sizeBack = m_bmpBack.GetImageSize ();
		
		for (int x = rectClient.left; x < rectClient.right; x += sizeBack.cx)
		{
			for (int y = rectClient.top; y < rectClient.bottom; y += sizeBack.cy)
			{
				m_bmpBack.Draw (pDC, x, y, 0);
			}
		}
		
		m_bmpBack.EndDrawImage (ds);
	}
	else if (m_clrBackColor != (COLORREF)-1)
	{
		CBrush br (m_clrBackColor);
		pDC->FillRect (rectClient, &br);
	}
	else
	{
		CBCGPVisualManager::GetInstance ()->OnFillBarBackground (pDC, this, rectClient, rectClient);
	}

	if (!m_Buttons.IsEmpty ())
	{
		pDC->SetTextColor (globalData.clrBtnText);
		pDC->SetBkMode (TRANSPARENT);

		CBCGPDrawState ds(CBCGPVisualManager::GetInstance()->IsAutoGrayscaleImages());
		if (!m_Images.PrepareDrawImage (ds))
		{
			ASSERT (FALSE);
			return;     // something went wrong
		}

		CFont* pOldFont = pDC->SelectObject (&globalData.fontRegular);

		//--------------
		// Draw buttons:
		//--------------
		int iButton = 0;
		for (POSITION pos = m_Buttons.GetHeadPosition (); pos != NULL; iButton ++)
		{
			CBCGPOutlookButton* pButton = (CBCGPOutlookButton*) m_Buttons.GetNext (pos);
			ASSERT_VALID (pButton);

			CRect rect = pButton->Rect ();

			BOOL bHighlighted = FALSE;

			if (IsCustomizeMode () && !m_bLocked)
			{
				bHighlighted = FALSE;
			}
			else
			{
				bHighlighted = ((iButton == m_iHighlighted ||
								iButton == m_iButtonCapture) &&
								(m_iButtonCapture == -1 ||
								iButton == m_iButtonCapture));
			}

			CRect rectInter;
			if (rectInter.IntersectRect (rect, rectClip))
			{
				pButton->OnDraw (pDC, rect, &m_Images, FALSE, IsCustomizeMode (),
								bHighlighted);
			}
		}

		//-------------------------------------------------------------
		// Highlight selected button in the toolbar customization mode:
		//-------------------------------------------------------------
		if (m_iSelected >= m_Buttons.GetCount ())
		{
			m_iSelected = -1;
		}

		if (IsCustomizeMode () && m_iSelected >= 0 && !m_bLocked)
		{
			CBCGPToolbarButton* pSelButton = GetButton (m_iSelected);
			ASSERT (pSelButton != NULL);

			if (pSelButton != NULL && pSelButton->CanBeStored ())
			{
				CRect rectDrag = pSelButton->Rect ();
				if (pSelButton->GetHwnd () != NULL)
				{
					rectDrag.InflateRect (0, 1);
				}

				pDC->DrawDragRect (&rectDrag, CSize (2, 2), NULL, CSize (2, 2));
			}
		}

		if (IsCustomizeMode () && m_iDragIndex >= 0 && !m_bLocked)
		{
			DrawDragMarker (pDC);
		}

		pDC->SelectClipRgn (NULL);
		pDC->SelectObject (pOldFont);

		m_Images.EndDrawImage (ds);
	}
}
//****************************************************************************************
DROPEFFECT CBCGPOutlookBarPane::OnDragOver(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point)
{
	CBCGPToolbarButton* pButton = CBCGPToolbarButton::CreateFromOleData (pDataObject);
	if (pButton == NULL)
	{
		return DROPEFFECT_NONE;
	}

	BOOL bAllowDrop = pButton->IsKindOf (RUNTIME_CLASS (CBCGPOutlookButton));
	delete pButton;

	if (!bAllowDrop)
	{
		return DROPEFFECT_NONE;
	}

	CRect rectUp;
	m_btnUp.GetWindowRect (rectUp);
	ScreenToClient (rectUp);

	if (rectUp.PtInRect (point))
	{
		ScrollUp ();
		return DROPEFFECT_NONE;
	}

	CRect rectDown;
	m_btnDown.GetWindowRect (rectDown);
	ScreenToClient (rectDown);

	if (rectDown.PtInRect (point))
	{
		ScrollDown ();
		return DROPEFFECT_NONE;
	}

	return CBCGPToolBar::OnDragOver (pDataObject, dwKeyState, point);
}
//***************************************************************************************
CBCGPToolbarButton* CBCGPOutlookBarPane::CreateDroppedButton (COleDataObject* pDataObject)
{
	CBCGPToolbarButton* pButton = CBCGPToolBar::CreateDroppedButton (pDataObject);
	ASSERT (pButton != NULL);

	CBCGPOutlookButton* pOutlookButton = DYNAMIC_DOWNCAST (CBCGPOutlookButton, pButton);
	if (pOutlookButton == NULL)
	{
		delete pButton;

		ASSERT (FALSE);
		return NULL;
	}

	return pButton;
}
//***************************************************************************************
BOOL CBCGPOutlookBarPane::EnableContextMenuItems (CBCGPToolbarButton* pButton, CMenu* pPopup)
{
	ASSERT_VALID (pButton);
	ASSERT_VALID (pPopup);

	if (IsCustomizeMode ())
	{
		pPopup->EnableMenuItem (ID_BCGBARRES_TOOLBAR_IMAGE, MF_GRAYED | MF_BYCOMMAND);
		pPopup->EnableMenuItem (ID_BCGBARRES_TOOLBAR_TEXT, MF_GRAYED | MF_BYCOMMAND);
		pPopup->EnableMenuItem (ID_BCGBARRES_TOOLBAR_IMAGE_AND_TEXT, MF_GRAYED | MF_BYCOMMAND);
		pPopup->EnableMenuItem (ID_BCGBARRES_TOOLBAR_APPEARANCE, MF_GRAYED | MF_BYCOMMAND);
		pPopup->EnableMenuItem (ID_BCGBARRES_TOOLBAR_START_GROUP, MF_GRAYED | MF_BYCOMMAND);
		pPopup->EnableMenuItem (ID_BCGBARRES_TOOLBAR_RESET, MF_GRAYED | MF_BYCOMMAND);
		pPopup->EnableMenuItem (ID_BCGBARRES_COPY_IMAGE, MF_GRAYED | MF_BYCOMMAND);
	}

	CBCGPToolBar::EnableContextMenuItems (pButton, pPopup);
	return TRUE;
}
//**************************************************************************************
BOOL CBCGPOutlookBarPane::PreTranslateMessage(MSG* pMsg) 
{
   	switch (pMsg->message)
	{
	case WM_LBUTTONUP:
		KillTimer (idScrollUp);
		KillTimer (idScrollDn);

	case WM_LBUTTONDOWN:
	case WM_MOUSEMOVE:
		{
			CPoint ptCursor;
			::GetCursorPos (&ptCursor);
			ScreenToClient (&ptCursor);

			CRect rect;
			m_btnDown.GetClientRect (rect);
			m_btnDown.MapWindowPoints (this, rect);

			if (rect.PtInRect (ptCursor))
			{
				m_btnDown.SendMessage (pMsg->message, pMsg->wParam, pMsg->wParam);
				if (pMsg->message == WM_LBUTTONDOWN)
				{
					SetTimer (idScrollDn, uiScrollDelay, NULL);

					if(m_bPageScrollMode)
					{
						ScrollPageDown ();

					}else
					{
						ScrollDown ();
					}
				}
			}

			m_btnUp.GetClientRect (rect);
			m_btnUp.MapWindowPoints (this, rect);

			if (rect.PtInRect (ptCursor))
			{
				m_btnUp.SendMessage (pMsg->message, pMsg->wParam, pMsg->wParam);

				if (pMsg->message == WM_LBUTTONDOWN)
				{
					SetTimer (idScrollUp, uiScrollDelay, NULL);

					if(m_bPageScrollMode)
					{
						ScrollPageUp ();

					}else
					{
						ScrollUp ();
					}
				}
			}
		}
		break;
	}

	return CBCGPToolBar::PreTranslateMessage(pMsg);
}
//**************************************************************************************
void CBCGPOutlookBarPane::OnTimer(UINT_PTR nIDEvent) 
{
	switch (nIDEvent)
	{
	case idScrollUp:
		if (m_btnUp.IsPressed ())
		{
			if(m_bPageScrollMode)
			{
				ScrollPageUp ();

			}else
			{
				ScrollUp ();
			}
		}
		return;

	case idScrollDn:
		if (m_btnDown.IsPressed ())
		{
			if(m_bPageScrollMode)
			{
				ScrollPageDown ();

			}else
			{
				ScrollDown ();
			}
		}
		return;
	}

	CBCGPToolBar::OnTimer(nIDEvent);
}
//**************************************************************************************
void CBCGPOutlookBarPane::OnLButtonUp(UINT nFlags, CPoint point) 
{
	HWND hWnd = GetSafeHwnd ();
	CBCGPToolBar::OnLButtonUp(nFlags, point);

	if (::IsWindow (hWnd))
	{
		OnMouseLeave (0, 0);
	}
}
//*********************************************************************************
void CBCGPOutlookBarPane::RemoveAllButtons()
{
	CBCGPToolBar::RemoveAllButtons();

	m_iFirstVisibleButton = 0; 
	m_iScrollOffset = 0; 

	AdjustLocations();

	if (m_hWnd != NULL)
	{
		UpdateWindow();
		Invalidate();
	}
}
//*************************************************************************************
void CBCGPOutlookBarPane::ClearAll ()
{
	m_Images.Clear ();
}
//*************************************************************************************
BCGP_CS_STATUS CBCGPOutlookBarPane::IsChangeState (int /*nOffset*/,
												   CBCGPBaseControlBar** ppTargetBar) const
{
	ASSERT_VALID (this);
	ASSERT (ppTargetBar != NULL);

	CPoint	ptMousePos;
	GetCursorPos (&ptMousePos);

	*ppTargetBar = NULL;

	// check whether the mouse is around a dock bar
	CBCGPOutlookBar* pOutlookBar = DYNAMIC_DOWNCAST (CBCGPOutlookBar,
			ControlBarFromPoint (ptMousePos, 0, FALSE, RUNTIME_CLASS (CBCGPOutlookBar))); 

	if (pOutlookBar != NULL)
	{
		*ppTargetBar = pOutlookBar;
		return BCGP_CS_DOCK_IMMEDIATELY;
	}
	return BCGP_CS_NOTHING;
}
//*************************************************************************************
BOOL CBCGPOutlookBarPane::Dock (CBCGPBaseControlBar* pDockBar, LPCRECT /*lpRect*/,
								BCGP_DOCK_METHOD dockMethod)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDockBar);

	CBCGPMiniFrameWnd* pParentMiniFrame = GetParentMiniFrame ();

	CString strText;
	GetWindowText (strText);

	CBCGPOutlookBar* pOutlookBar = NULL;

	if (dockMethod == BCGP_DM_DBL_CLICK)
	{
		pOutlookBar = DYNAMIC_DOWNCAST (CBCGPOutlookBar, 
										CWnd::FromHandlePermanent (m_hRecentOutlookWnd));
	}
	else if (dockMethod == BCGP_DM_MOUSE)
	{
		pOutlookBar = DYNAMIC_DOWNCAST (CBCGPOutlookBar, pDockBar);
	}

	if (pOutlookBar == NULL)
	{
		return FALSE;
	}

	ASSERT_VALID (pOutlookBar);
	if (pParentMiniFrame != NULL)
	{
		pParentMiniFrame->RemoveControlBar (this);
	}
	
	pOutlookBar->AddTab (this);

	CBCGPBaseTabWnd* pTabWnd = pOutlookBar->GetUnderlinedWindow ();
	ASSERT_VALID (pTabWnd);

	int nAddedTab = pTabWnd->GetTabsNum () - 1;
	pTabWnd->SetTabLabel (nAddedTab, strText);
	pTabWnd->SetActiveTab (nAddedTab); 

	return TRUE;
}
//*************************************************************************************
BOOL CBCGPOutlookBarPane::OnBeforeFloat (CRect& /*rectFloat*/, BCGP_DOCK_METHOD dockMethod)
{
	if (dockMethod == BCGP_DM_MOUSE)
	{
		CPoint ptMouse;
		GetCursorPos (&ptMouse);

		CWnd* pParent = GetParent ();

		// make it float only when the mouse is out of parent bounds
		CRect rect;
		pParent->GetWindowRect (rect);
		BOOL bFloat = !rect.PtInRect (ptMouse);
		if (bFloat)
		{
			if (pParent->IsKindOf (RUNTIME_CLASS (CBCGPOutlookBar)))
			{
				m_hRecentOutlookWnd = pParent->GetSafeHwnd ();
			}
			else
			{
				m_hRecentOutlookWnd = pParent->GetParent ()->GetSafeHwnd ();
			}
		}
		return bFloat;
	}

	return TRUE;
}
//*************************************************************************************
void CBCGPOutlookBarPane::OnSetFocus(CWnd* pOldWnd) 
{
	// bypass the standard toolbar's set focus, because it sets the focus back
	// to old focused wnd
	CBCGPControlBar::OnSetFocus(pOldWnd);
}
//*************************************************************************************
void CBCGPOutlookBarPane::OnNcDestroy() 
{
	CBCGPToolBar::OnNcDestroy();
}
//****************************************************************************************
void CBCGPOutlookBarPane::SetDefaultState ()
{
	CopyButtonsList (m_Buttons, m_OrigButtons);
}
//****************************************************************************************
BOOL CBCGPOutlookBarPane::RestoreOriginalstate ()
{
	if (m_OrigButtons.IsEmpty ())
	{
		return FALSE;
	}

	CopyButtonsList (m_OrigButtons, m_Buttons);

	AdjustLayout ();

	RedrawWindow ();
	return TRUE;
}
//************************************************************************************
BOOL CBCGPOutlookBarPane::SmartUpdate (const CObList& lstPrevButtons)
{
	if (lstPrevButtons.IsEmpty ())
	{
		return FALSE;
	}

	m_bResourceWasChanged = FALSE;	// Outlook bar has its own resources

	BOOL bIsModified = FALSE;

	//-----------------------------------
	// Compare current and prev. buttons:
	//------------------------------------
	if (lstPrevButtons.GetCount () != m_OrigButtons.GetCount ())
	{
		bIsModified = TRUE;
	}
	else
	{
		POSITION posCurr, posPrev;
		for (posCurr = m_OrigButtons.GetHeadPosition (),
			posPrev = lstPrevButtons.GetHeadPosition (); posCurr != NULL;)
		{
			ASSERT (posPrev != NULL);

			CBCGPToolbarButton* pButtonCurr = 
				DYNAMIC_DOWNCAST (CBCGPToolbarButton, m_OrigButtons.GetNext (posCurr));
			ASSERT_VALID (pButtonCurr);

			CBCGPToolbarButton* pButtonPrev = 
				DYNAMIC_DOWNCAST (CBCGPToolbarButton, lstPrevButtons.GetNext (posPrev));
			ASSERT_VALID (pButtonPrev);

			if (!pButtonCurr->CompareWith (*pButtonPrev))
			{
				bIsModified = TRUE;
				break;
			}
		}
	}

	if (bIsModified)
	{
		RestoreOriginalstate ();
	}

	return bIsModified;
}
//************************************************************************************
void CBCGPOutlookBarPane::CopyButtonsList (const CObList& lstSrc, CObList& lstDst)
{
	while (!lstDst.IsEmpty ())
	{
		delete lstDst.RemoveHead ();
	}

	for (POSITION pos = lstSrc.GetHeadPosition (); pos != NULL;)
	{
		CBCGPToolbarButton* pButtonSrc = (CBCGPToolbarButton*) lstSrc.GetNext (pos);
		ASSERT_VALID (pButtonSrc);

		CRuntimeClass* pClass = pButtonSrc->GetRuntimeClass ();
		ASSERT (pClass != NULL);

		CBCGPToolbarButton* pButton = (CBCGPToolbarButton*) pClass->CreateObject ();
		ASSERT_VALID(pButton);

		pButton->CopyFrom (*pButtonSrc);
		pButton->OnChangeParentWnd (this);

		lstDst.AddTail (pButton);
	}
}
//*************************************************************************************
void CBCGPOutlookBarPane::ScrollPageUp ()
{
	if (m_iScrollOffset <= 0 ||
		m_iFirstVisibleButton <= 0)
	{
		m_iScrollOffset = 0;
		m_iFirstVisibleButton = 0;

		KillTimer (idScrollUp);
		return;
	}

	CBCGPToolbarButton* pFirstVisibleButton = GetButton (m_iFirstVisibleButton);
	if (pFirstVisibleButton == NULL)
	{
		KillTimer (idScrollDn);
		return;
	}

	CRect rcArea;
	GetClientRect(rcArea);
	int nVisibleCount = 0;
	
	nVisibleCount = (rcArea.Height())/(pFirstVisibleButton->Rect ().Height () + m_nExtraSpace);

	for(int i=0; i<nVisibleCount; i++)
	{
		 ScrollUp();
	}
	

}
//*************************************************************************************
void CBCGPOutlookBarPane::ScrollPageDown ()
{
	if (!m_bScrollDown ||
		m_iFirstVisibleButton + 1 >= GetCount ())
	{
		KillTimer (idScrollDn);
		return;
	}

	CBCGPToolbarButton* pFirstVisibleButton = GetButton (m_iFirstVisibleButton);
	if (pFirstVisibleButton == NULL)
	{
		KillTimer (idScrollDn);
		return;
	}

	CRect rcArea;
	GetClientRect(rcArea);
	int nVisibleCount = 0;

	
	nVisibleCount = (rcArea.Height())/(pFirstVisibleButton->Rect ().Height () + m_nExtraSpace);

	 for(int i=0; i<nVisibleCount; i++)
	 {
		 ScrollDown();
	 }

}
//*************************************************************************************
BOOL CBCGPOutlookBarPane::OnMouseWheel(UINT /*nFlags*/, short zDelta, CPoint /*pt*/) 
{
	CSize sizeOverPan(0, 0);
	return InternalScroll((int)(zDelta * GetRowHeight() / WHEEL_DELTA), sizeOverPan);
}
