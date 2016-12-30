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
// BCGPSmartDockingMarker.cpp: implementation of the CBCGPSmartDockingMarker class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "bcgcbpro.h"
#include "BCGPSmartDockingMarker.h"
#include "bcgprores.h"
#include "BCGPLocalResource.h"
#include "bcgglobals.h"
#include "BCGPDockManager.h"
#include "BCGPVisualManager.h"
#include "BCGPDrawManager.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

const int CBCGPSmartDockingMarker::m_nLeftOffsetX = 16;
const int CBCGPSmartDockingMarker::m_nRightOffsetX = 16;
const int CBCGPSmartDockingMarker::m_nTopOffsetY = 16;
const int CBCGPSmartDockingMarker::m_nBottomOffsetY = 16;

IMPLEMENT_DYNCREATE(CBCGPSmartDockingMarker, CObject)
IMPLEMENT_DYNCREATE(CBCGPSDCentralGroup, CObject)

#define COLOR_HIGHLIGHT_FRAME RGB (65, 112, 202)
#define ALPHA_TRANSPARENT 192

static BCGP_SMARTDOCK_THEME GetVMTheme ()
{
	CBCGPSmartDockingParams& params = CBCGPDockManager::GetSmartDockingParams ();
	if (params.m_uiMarkerBmpResID [0] != 0)
	{
		return BCGP_SDT_DEFAULT;
	}

	BCGP_SMARTDOCK_THEME theme = CBCGPDockManager::GetSmartDockingTheme ();
	if (theme == BCGP_SDT_DEFAULT)
	{
		theme = CBCGPVisualManager::GetInstance()->GetSmartDockingTheme ();
	}

	return theme;
}

static void ShadeRect (CDC* pDC, CRect rect, BOOL bIsVert)
{
	ASSERT_VALID (pDC);

	COLORREF colors [2] =
	{
		RGB (198, 198, 198),
		RGB (206, 206, 206),
	};

	rect.DeflateRect (1, 1);

	for (int i = 0; i < 2; i++)
	{
		CPen pen (PS_SOLID, 1, colors [i]);
		CPen* pOldPen = pDC->SelectObject (&pen);

		if (bIsVert)
		{
			pDC->MoveTo (rect.left + i, rect.top);
			pDC->LineTo (rect.left + i, rect.bottom);
		}
		else
		{
			pDC->MoveTo (rect.left, rect.top + i);
			pDC->LineTo (rect.right, rect.top + i);
		}

		pDC->SelectObject (pOldPen);
	}
}

//
//	CBCGPSmartDockingMarker
//

CBCGPSmartDockingMarker::CBCGPSmartDockingMarker() :
	m_nSideNo (sdNONE),
	m_cx (-1),
	m_cy (-1),
	m_nPadding (0),
	m_bHiLited (FALSE),
	m_bLayered (FALSE),
	m_bIsDefaultImage (TRUE)
{
}

CBCGPSmartDockingMarker::~CBCGPSmartDockingMarker()
{
    Destroy ();
}

void CBCGPSmartDockingMarker::Create (SDMarkerPlace nSideNo, CWnd* pwndOwner)
{
	ASSERT(nSideNo >= sdLEFT && nSideNo <= sdBOTTOM);

	m_nSideNo = nSideNo;

	CBCGPSmartDockingParams& params = CBCGPDockManager::GetSmartDockingParams ();
	const BOOL bAlphaMarkers = params.m_bIsAlphaMarkers || GetVMTheme () == BCGP_SDT_VS2008 || GetVMTheme () == BCGP_SDT_VS2010 || GetVMTheme () == BCGP_SDT_VS2012;

	InitImages (params);

	m_Rgn.Attach (CBCGPToolBarImages::CreateRegionFromImage (
		m_Image.GetImageWell (), params.m_clrTransparent));

	CRect rect;
	m_Rgn.GetRgnBox(rect);

	m_cx = rect.Width ();
	m_cy = rect.Height ();

	HBITMAP hBmp = m_Image.GetImageWellLight ();
	if (hBmp == NULL)
	{
		 hBmp = m_Image.GetImageWell ();
	}

	BOOL bIsVert = m_nSideNo == sdTOP || m_nSideNo == sdBOTTOM;

    if (globalData.IsWindowsLayerSupportAvailable ())
	{
		m_wndBmp.Create (&rect, hBmp, NULL, pwndOwner, m_bIsDefaultImage, bIsVert);
	    m_wndBmp.ModifyStyleEx (0, WS_EX_LAYERED);

		if (!bAlphaMarkers)
		{
			globalData.SetLayeredAttrib (m_wndBmp.GetSafeHwnd(), params.m_clrTransparent, 0, LWA_COLORKEY);
		}

		m_bLayered = TRUE;
	}
	else
	{
		m_wndBmp.Create (&rect, hBmp, m_Rgn, pwndOwner, m_bIsDefaultImage, bIsVert);
		m_bLayered = FALSE;
	}

    m_wndBmp.ModifyStyleEx (0, WS_EX_TOPMOST);
}

void CBCGPSmartDockingMarker::Destroy ()
{
	if (::IsWindow(m_wndBmp.m_hWnd))
	{
		m_wndBmp.DestroyWindow ();
	}
}

void CBCGPSmartDockingMarker::Show (BOOL bShow)
{
	if (::IsWindow(m_wndBmp.m_hWnd))
	{
		m_wndBmp.ShowWindow(bShow ? SW_SHOW : SW_HIDE);
	}
}

void CBCGPSmartDockingMarker::AdjustPos (CRect rcHost)
{
	int x = 0;
	int y = 0;

	switch (m_nSideNo)
	{
	case sdLEFT:
		x = rcHost.left + m_nLeftOffsetX;
		y = ((rcHost.bottom + rcHost.top) >> 1) - (m_cy>>1);
		break;

	case sdRIGHT:
		x = rcHost.right - m_nRightOffsetX - m_cx;
		y = ((rcHost.bottom + rcHost.top) >> 1) - (m_cy>>1);
		break;

	case sdTOP:
		x = ((rcHost.left + rcHost.right) >> 1) - (m_cx >> 1);
		y = rcHost.top + m_nTopOffsetY;
		break;

	case sdBOTTOM:
		x = ((rcHost.left + rcHost.right) >> 1) - (m_cx >> 1);
		y = rcHost.bottom - m_nBottomOffsetY - m_cy;
		break;

	default:
		ASSERT (FALSE);
		return;
	}

    if (m_wndBmp.GetSafeHwnd () != NULL)
    {
	    m_wndBmp.SetWindowPos(&CWnd::wndTopMost, x, y, -1, -1,
		    SWP_NOSIZE | SWP_NOACTIVATE);
    }
}

void CBCGPSmartDockingMarker::Highlight (BOOL bHiLite)
{
	if (m_bHiLited == bHiLite)
	{
		m_wndBmp.UpdateLayeredWindow ();
		return;
	}

	m_bHiLited = bHiLite;
	m_wndBmp.Highlight (m_bHiLited);

	if (!m_bIsDefaultImage || GetVMTheme () == BCGP_SDT_VS2008)
	{
		HBITMAP hBmpLight = m_Image.GetImageWellLight ();
		if (hBmpLight == NULL)
		{
			 hBmpLight = m_Image.GetImageWell ();
		}

		m_wndBmp.Assign(bHiLite ? 
			(m_ImageHot.IsValid () ? m_ImageHot.GetImageWell () : m_Image.GetImageWell ()) : hBmpLight, TRUE);
	}
}

BOOL CBCGPSmartDockingMarker::IsPtIn(CPoint point) const
{
	if (m_wndBmp.GetSafeHwnd () == NULL || !m_wndBmp.IsWindowVisible ())
	{
		return FALSE;
	}
	m_wndBmp.ScreenToClient(&point);

	if (m_bLayered)
	{
		return m_Rgn.PtInRegion(point);
	}
	else
	{
		CRgn rgn;
		rgn.CreateRectRgn(0, 0, 0, 0);

		m_wndBmp.GetWindowRgn(rgn);

		return rgn.PtInRegion(point);
	}
}

void CBCGPSmartDockingMarker::InitImages (CBCGPSmartDockingParams& params)
{
	CBCGPLocalResource* pLocaRes = NULL;

	static UINT uiDefaultMarkerIDs2005 [] =
	{
		IDB_BCGBARRES_SD_LEFT,
		IDB_BCGBARRES_SD_RIGHT,
		IDB_BCGBARRES_SD_TOP,
		IDB_BCGBARRES_SD_BOTTOM,
		IDB_BCGBARRES_SD_MIDDLE
	};

	static UINT uiDefaultMarkerIDs2008 [] =
	{
		IDB_BCGBARRES_SD2008_LEFT,
		IDB_BCGBARRES_SD2008_RIGHT,
		IDB_BCGBARRES_SD2008_TOP,
		IDB_BCGBARRES_SD2008_BOTTOM,
		IDB_BCGBARRES_SD2008_MIDDLE
	};

	static UINT uiDefaultMarkerHotIDs2008 [] =
	{
		IDB_BCGBARRES_SD2008_LEFT_HOT,
		IDB_BCGBARRES_SD2008_RIGHT_HOT,
		IDB_BCGBARRES_SD2008_TOP_HOT,
		IDB_BCGBARRES_SD2008_BOTTOM_HOT,
		IDB_BCGBARRES_SD2008_MIDDLE_HOT
	};

	static UINT uiDefaultMarkerIDs2010 [] =
	{
		IDB_BCGBARRES_SD2010_LEFT,
		IDB_BCGBARRES_SD2010_RIGHT,
		IDB_BCGBARRES_SD2010_TOP,
		IDB_BCGBARRES_SD2010_BOTTOM,
		IDB_BCGBARRES_SD2010_MIDDLE
	};

	static UINT uiDefaultMarkerIDs2012 [] =
	{
		IDB_BCGBARRES_SD2012_LEFT,
		IDB_BCGBARRES_SD2012_RIGHT,
		IDB_BCGBARRES_SD2012_TOP,
		IDB_BCGBARRES_SD2012_BOTTOM,
		IDB_BCGBARRES_SD2012_MIDDLE
	};

	m_Image.Clear ();
	m_Image.SetLightPercentage (-1);

	m_ImageHot.Clear ();

	m_nPadding = 0;

	int nIndex = -1;

	switch (m_nSideNo)
	{
	case sdLEFT:
	case sdCLEFT:
		nIndex = 0;
		break;

	case sdRIGHT:
	case sdCRIGHT:
		nIndex = 1;
		break;

	case sdTOP:
	case sdCTOP:
		nIndex = 2;
		break;

	case sdBOTTOM:
	case sdCBOTTOM:
		nIndex = 3;
		break;

	case sdCMIDDLE:
		nIndex = 4;
		break;

	default:
		ASSERT (FALSE);
		return;
	}

	UINT uiResID = params.m_uiMarkerBmpResID [nIndex];
	UINT uiResHotID = params.m_uiMarkerLightBmpResID [nIndex];

	m_bIsDefaultImage = uiResID == 0;

	if (m_bIsDefaultImage)
	{
		// Use default marker:

		BCGP_SMARTDOCK_THEME theme = GetVMTheme ();

		switch (theme)
		{
		case BCGP_SDT_VS2005:
			uiResID = uiDefaultMarkerIDs2005 [nIndex];
			break;

		case BCGP_SDT_VS2008:
			uiResID = uiDefaultMarkerIDs2008 [nIndex];
			uiResHotID = uiDefaultMarkerHotIDs2008 [nIndex];
			break;

		case BCGP_SDT_VS2010:
			uiResID = uiDefaultMarkerIDs2010 [nIndex];
			break;

		case BCGP_SDT_VS2012:
			uiResID = uiDefaultMarkerIDs2012 [nIndex];
			m_nPadding = 3;
			break;
		}

		pLocaRes = new CBCGPLocalResource ();
	}

	m_Image.SetMapTo3DColors (FALSE);
	m_Image.SetAlwaysLight (uiResHotID == 0 && GetVMTheme () != BCGP_SDT_VS2010 && GetVMTheme () != BCGP_SDT_VS2012);
	m_Image.Load (uiResID);
	m_Image.SetSingleImage ();

	m_Image.SetTransparentColor (params.m_clrTransparent);

	if (uiResHotID != 0)
	{
		m_ImageHot.SetMapTo3DColors (FALSE);
		m_ImageHot.Load (uiResHotID);
		m_ImageHot.SetSingleImage ();

		m_ImageHot.SetTransparentColor (params.m_clrTransparent);
	}

	COLORREF clrToneSrc = m_bIsDefaultImage ? (COLORREF)-1 : params.m_clrToneSrc;
	COLORREF clrToneDst = m_bIsDefaultImage && params.m_clrToneDest == -1 ? 
		CBCGPVisualManager::GetInstance ()->GetSmartDockingMarkerToneColor () : 
			params.m_clrToneDest;

	if (clrToneSrc != (COLORREF)-1 && clrToneDst != (COLORREF)-1)
	{
		m_Image.AddaptColors (clrToneSrc, clrToneDst);
	}

	HWND hwndBmp = m_wndBmp.GetSafeHwnd();
	if (hwndBmp != NULL)
	{
		HBITMAP hBmpLight = m_Image.GetImageWellLight ();
		if (hBmpLight == NULL)
		{
			 hBmpLight = m_Image.GetImageWell ();
		}

		m_wndBmp.Assign (hBmpLight, FALSE);

		if (!params.m_bIsAlphaMarkers && GetVMTheme () != BCGP_SDT_VS2008 && GetVMTheme () != BCGP_SDT_VS2010 && GetVMTheme () != BCGP_SDT_VS2012)
		{
			globalData.SetLayeredAttrib (hwndBmp, params.m_clrTransparent, 0, LWA_COLORKEY);
		}
	}

	if (pLocaRes != NULL)
	{
		delete pLocaRes;
	}
}

//
//	CBCGPCentralGroupWnd
//

CBCGPCentralGroupWnd::CBCGPCentralGroupWnd () :
	m_pCentralGroup (NULL)
{
	CBCGPSmartDockingParams& params = CBCGPDockManager::GetSmartDockingParams ();

	COLORREF clrBaseGroupBackground;
	COLORREF clrBaseGroupBorder;

	CBCGPVisualManager::GetInstance ()->GetSmartDockingBaseMarkerColors (
		clrBaseGroupBackground, clrBaseGroupBorder);

	if (GetVMTheme () == BCGP_SDT_VS2010 || GetVMTheme () == BCGP_SDT_VS2012)
	{
		clrBaseGroupBackground = RGB(255, 255, 255);
	}

	m_brBaseBackground.CreateSolidBrush (
		params.m_clrBaseBackground == -1 ? clrBaseGroupBackground : params.m_clrBaseBackground);
	m_brBaseBorder.CreateSolidBrush (
		params.m_clrBaseBorder == -1 ? clrBaseGroupBorder : params.m_clrBaseBorder);
}

void CBCGPCentralGroupWnd::Update ()
{
	CBCGPSmartDockingParams& params = CBCGPDockManager::GetSmartDockingParams ();
	if (!params.m_bIsAlphaMarkers && GetVMTheme () != BCGP_SDT_VS2008 && GetVMTheme () != BCGP_SDT_VS2010 && GetVMTheme () != BCGP_SDT_VS2012)
	{
		RedrawWindow ();
		return;
	}

	CRect rect;
	GetClientRect (rect);

	CPoint point (0, 0);
	CSize size (rect.Size ());

	LPBYTE pBits = NULL;
	HBITMAP hBitmap = CBCGPDrawManager::CreateBitmap_32 (size, (void**)&pBits);
	if (hBitmap == NULL)
	{
		return;
	}

	CBitmap bitmap;
	bitmap.Attach (hBitmap);

	CClientDC clientDC(this);
	CDC dc;
	dc.CreateCompatibleDC (&clientDC);

	CBitmap* pBitmapOld = (CBitmap*)dc.SelectObject (&bitmap);

	ASSERT_VALID (m_pCentralGroup);

	m_pCentralGroup->DrawCentralGroup (dc, m_brBaseBackground, m_brBaseBorder, rect);

	BLENDFUNCTION bf;
	bf.BlendOp             = AC_SRC_OVER;
	bf.BlendFlags          = 0;
	bf.SourceConstantAlpha = 255;
	bf.AlphaFormat         = LWA_COLORKEY;

	globalData.UpdateLayeredWindow (GetSafeHwnd (), NULL, 0, &size, dc.GetSafeHdc (), 
		&point, 0, &bf, 0x02);

	dc.SelectObject (pBitmapOld);
}

BEGIN_MESSAGE_MAP(CBCGPCentralGroupWnd, CWnd)
	ON_WM_PAINT()
	ON_WM_CLOSE()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

void CBCGPCentralGroupWnd::OnPaint () 
{
	CBCGPSmartDockingParams& params = CBCGPDockManager::GetSmartDockingParams ();

	CPaintDC dc (this); // device context for painting

	CBCGPMemDC memDC (dc, this);
	CDC* pDC = &memDC.GetDC ();

	CRect rectClient;
	GetClientRect (rectClient);

	if (globalData.IsWindowsLayerSupportAvailable ())
	{
		CBrush brBack;
		brBack.CreateSolidBrush (params.m_clrTransparent);

		pDC->FillRect (rectClient, &brBack);
	}

	ASSERT_VALID (m_pCentralGroup);

	m_pCentralGroup->DrawCentralGroup (*pDC, m_brBaseBackground, m_brBaseBorder, rectClient);
}

void CBCGPCentralGroupWnd::OnClose () 
{
	// so that it does not get destroyed
}

BOOL CBCGPCentralGroupWnd::OnEraseBkgnd (CDC* /*pDC*/)
{
	return TRUE;
}

//
//	CBCGPSDCentralGroupMarker
//

CBCGPSDCentralGroupMarker::CBCGPSDCentralGroupMarker () :
	m_pCentralGroup (NULL),
	m_bVisible (TRUE),
	m_clrFrame ((COLORREF)-1)
{
}

CBCGPSDCentralGroupMarker::~CBCGPSDCentralGroupMarker ()
{
}

void CBCGPSDCentralGroupMarker::Create (SDMarkerPlace, CWnd*)
{
	// should never be called
	ASSERT (FALSE);
}

void CBCGPSDCentralGroupMarker::Destroy ()
{
	// should never be called
	ASSERT (FALSE);
}

void CBCGPSDCentralGroupMarker::Show (BOOL)
{
	// should never be called
	ASSERT (FALSE);
}

void CBCGPSDCentralGroupMarker::AdjustPos (CRect)
{
	// should never be called
	ASSERT (FALSE);
}

void CBCGPSDCentralGroupMarker::Highlight (BOOL bHiLite)
{
	if (m_bHiLited == bHiLite)
	{
		return;
	}

	ASSERT_VALID (m_pCentralGroup);

	m_bHiLited = bHiLite;
	m_pCentralGroup->m_Wnd.Update ();

}

void CBCGPSDCentralGroupMarker::SetVisible (BOOL bVisible/* = TRUE*/,
											BOOL bRedraw/* = TRUE*/)
{
	m_bVisible = bVisible;

	if (bRedraw && m_pCentralGroup != NULL)
	{
		ASSERT_VALID (m_pCentralGroup);
		m_pCentralGroup->m_Wnd.Update ();
	}
}

BOOL CBCGPSDCentralGroupMarker::IsPtIn(CPoint point) const
{
	if (!m_bVisible)
	{
		return FALSE;
	}

	m_pCentralGroup->m_Wnd.ScreenToClient (&point);
	return m_Rgn.PtInRegion (point.x, point.y);
}

void CBCGPSDCentralGroupMarker::Create (SDMarkerPlace nSideNo, CBCGPSDCentralGroup* pCentralGroup)
{
	ASSERT(nSideNo >= sdCLEFT && nSideNo <= sdCMIDDLE);

	m_nSideNo = nSideNo;
	m_pCentralGroup = pCentralGroup;

	CBCGPSmartDockingParams& params = CBCGPDockManager::GetSmartDockingParams ();

	InitImages (params);

	if (m_bIsDefaultImage)
	{
		BCGP_SMARTDOCK_THEME theme = GetVMTheme ();

		switch (theme)
		{
		case BCGP_SDT_VS2005:
			params.m_nCentralGroupOffset = 9;
			params.m_sizeTotal = CSize (88, 88);
			break;

		case BCGP_SDT_VS2008:
			params.m_nCentralGroupOffset = 5;
			params.m_sizeTotal = CSize (110, 110);
			break;

		case BCGP_SDT_VS2010:
		case BCGP_SDT_VS2012:
			params.m_nCentralGroupOffset = 5;
			params.m_sizeTotal = CSize (114, 114);
			break;
		}
	}

	
	COLORREF clrBaseGroupBackground;
	CBCGPVisualManager::GetInstance ()->GetSmartDockingBaseMarkerColors (
		clrBaseGroupBackground, m_clrFrame);

	m_penFrame.CreatePen (PS_SOLID, 1, m_clrFrame);
	m_penHighlight.CreatePen (PS_SOLID, 1, COLOR_HIGHLIGHT_FRAME);

	const CSize sizeGroup = params.m_sizeTotal;
	const CSize sizeImage = m_Image.GetImageSize ();

	switch (m_nSideNo)
	{
	case sdCLEFT:
		m_nOffsetX = 0;
		m_nOffsetY = (sizeGroup.cy  - sizeImage.cy) / 2;
		break;

	case sdCRIGHT:
		m_nOffsetX = sizeGroup.cx  - sizeImage.cx;
		m_nOffsetY = (sizeGroup.cy  - sizeImage.cy) / 2;
		break;

	case sdCTOP:
		m_nOffsetX = (sizeGroup.cx  - sizeImage.cx) / 2;
		m_nOffsetY = 0;
		break;

	case sdCBOTTOM:
		m_nOffsetX = (sizeGroup.cx  - sizeImage.cx) / 2;
		m_nOffsetY = sizeGroup.cy  - sizeImage.cy;
		break;

	case sdCMIDDLE:
		m_nOffsetX = (sizeGroup.cx  - sizeImage.cx) / 2;
		m_nOffsetY = (sizeGroup.cy  - sizeImage.cy) / 2;
		break;
	}

	m_Rgn.Attach (CBCGPToolBarImages::CreateRegionFromImage (
		m_Image.GetImageWell (), params.m_clrTransparent));
	m_Rgn.OffsetRgn (m_nOffsetX, m_nOffsetY);
}

void CBCGPSDCentralGroupMarker::DestroyImages ()
{
	m_Rgn.DeleteObject ();
}

void CBCGPSDCentralGroupMarker::Draw (CDC& dc, BOOL bAlpha)
{
	CBCGPToolBarImages& image = m_bHiLited && m_ImageHot.IsValid () ? m_ImageHot : m_Image;

	int xImage = m_nOffsetX;
	int yImage = m_nOffsetY;

	if (GetVMTheme () == BCGP_SDT_VS2010 || GetVMTheme () == BCGP_SDT_VS2012)
	{
		switch (m_nSideNo)
		{
		case sdCLEFT:
			xImage += 2;
			break;

		case sdCRIGHT:
			xImage -= 2;
			break;

		case sdCTOP:
			yImage += 2;
			break;

		case sdCBOTTOM:
			yImage -= 2;
			break;

		}
	}

	if (bAlpha)
	{
		CRect rectImage(CPoint (xImage, yImage), image.GetImageSize ());
		CRect rectSrc(0, 0, 0, 0);

		if (m_nPadding != 0)
		{
			rectImage.DeflateRect(m_nPadding, m_nPadding);
		
			rectSrc.right = image.GetImageSize ().cx;
			rectSrc.bottom = image.GetImageSize ().cy;
			rectSrc.DeflateRect(m_nPadding, m_nPadding);
		}

		image.DrawEx (&dc, rectImage, 0, 
			CBCGPToolBarImages::ImageAlignHorzLeft,
			CBCGPToolBarImages::ImageAlignVertTop,
			rectSrc, (BYTE)(m_bHiLited ? 255 : ALPHA_TRANSPARENT));
		return;
	}
	
	const BOOL bFadeImage = !m_bHiLited && !m_ImageHot.IsValid ();

	CBCGPDrawState ds;
	image.PrepareDrawImage (ds, CSize (0, 0), bFadeImage);

	image.Draw (&dc, xImage, yImage, 0, FALSE, FALSE, FALSE, FALSE, bFadeImage);
	image.EndDrawImage (ds);

	if (!m_bIsDefaultImage)
	{
		return;
	}

	if (GetVMTheme () == BCGP_SDT_VS2008 || GetVMTheme () == BCGP_SDT_VS2010 || GetVMTheme () == BCGP_SDT_VS2012)
	{
		return;
	}

	//--------------------------------------------------
	// For the default image we need to draw the border:
	//--------------------------------------------------
	CRect rect;
	m_Rgn.GetRgnBox (rect);

	CPen* pOldPen = dc.SelectObject (m_bHiLited ? &m_penHighlight : &m_penFrame);

	int nMargin = 7;
	BOOL bShadeRect = TRUE;

	switch (m_nSideNo)
	{
	case sdCLEFT:
		rect.right -= nMargin;
		dc.MoveTo (rect.right, rect.top);
		dc.LineTo (rect.left, rect.top);
		dc.LineTo (rect.left, rect.bottom);
		dc.LineTo (rect.right, rect.bottom);

		if (bShadeRect)
		{
			ShadeRect (&dc, rect, FALSE);
		}
		break;

	case sdCRIGHT:
		rect.left += nMargin;
		dc.MoveTo (rect.left, rect.top);
		dc.LineTo (rect.right - 1, rect.top);
		dc.LineTo (rect.right - 1, rect.bottom);
		dc.LineTo (rect.left, rect.bottom);

		if (bShadeRect)
		{
			ShadeRect (&dc, rect, FALSE);
		}
		break;

	case sdCTOP:
		rect.bottom -= nMargin;
		dc.MoveTo (rect.left, rect.bottom);
		dc.LineTo (rect.left, rect.top);
		dc.LineTo (rect.right, rect.top);
		dc.LineTo (rect.right, rect.bottom);

		if (bShadeRect)
		{
			ShadeRect (&dc, rect, TRUE);
		}
		break;

	case sdCBOTTOM:
		rect.top += nMargin;
		dc.MoveTo (rect.left, rect.top);
		dc.LineTo (rect.left, rect.bottom - 1);
		dc.LineTo (rect.right, rect.bottom - 1);
		dc.LineTo (rect.right, rect.top);

		if (bShadeRect)
		{
			ShadeRect (&dc, rect, TRUE);
		}
		break;

	case sdCMIDDLE:
		break;
	}

	dc.SelectObject (pOldPen);
}

//
//	CBCGPSDCentralGroup
//

CBCGPSDCentralGroup::CBCGPSDCentralGroup () :
	m_bCreated (FALSE),
    m_bMiddleIsOn (FALSE),
	m_bLayered (FALSE)
{
}

CBCGPSDCentralGroup::~CBCGPSDCentralGroup ()
{
	Destroy ();
}

void CBCGPSDCentralGroup::Create (CWnd* pwndOwner)
{
	if (m_bCreated)
	{
		return;
	}

	CRgn rgnAll;
	rgnAll.CreateRectRgn (0, 0, 0, 0);

	CBCGPSmartDockingMarker::SDMarkerPlace i;
	for (i = CBCGPSmartDockingMarker::sdCLEFT; i <= CBCGPSmartDockingMarker::sdCMIDDLE; ++reinterpret_cast<int&>(i))
	{
		m_arMarkers[i - CBCGPSmartDockingMarker::sdCLEFT].Create (i, this);

		rgnAll.CombineRgn (&rgnAll, &m_arMarkers[i - CBCGPSmartDockingMarker::sdCLEFT].m_Rgn, RGN_OR);
	}

	CBCGPSmartDockingParams& params = CBCGPDockManager::GetSmartDockingParams ();

	UINT uiBaseResID = params.m_uiBaseBmpResID;

	if (uiBaseResID == 0)
	{
		BCGP_SMARTDOCK_THEME theme = GetVMTheme ();

		switch (theme)
		{
		case BCGP_SDT_VS2008:
			uiBaseResID = IDB_BCGBARRES_SD2008_BASE;
			break;

		case BCGP_SDT_VS2010:
			uiBaseResID = IDB_BCGBARRES_SD2010_BASE;
			break;

		case BCGP_SDT_VS2012:
			uiBaseResID = IDB_BCGBARRES_SD2012_BASE;
			break;
		}
	}

	if (uiBaseResID != 0)
	{
		m_Image.SetMapTo3DColors (FALSE);
		m_Image.SetAlwaysLight ();
		m_Image.Load (uiBaseResID);
		m_Image.SetSingleImage ();
		m_Image.SetTransparentColor (params.m_clrTransparent);
	}

	CRect rectBase;
	rgnAll.GetRgnBox (rectBase);
	rectBase.DeflateRect (params.m_nCentralGroupOffset, params.m_nCentralGroupOffset);

	#define BASE_PTS 4
	POINT ptBase [BASE_PTS];
	
	ptBase [0].x = rectBase.left;
	ptBase [0].y = rectBase.CenterPoint ().y;
	ptBase [1].x = rectBase.CenterPoint ().x;
	ptBase [1].y = rectBase.bottom;
	ptBase [2].x = rectBase.right;
	ptBase [2].y = rectBase.CenterPoint ().y;
	ptBase [3].x = rectBase.CenterPoint ().x;
	ptBase [3].y = rectBase.top;

	m_rgnBase.CreatePolygonRgn (ptBase, BASE_PTS, ALTERNATE);
	
	rgnAll.CombineRgn (&rgnAll, &m_rgnBase, RGN_OR);

	CRect rcGroup;
	rgnAll.GetRgnBox (rcGroup);

    BOOL bResult = FALSE;

	bResult = m_Wnd.CreateEx (WS_EX_TOPMOST, GetSDWndClassName<CS_OWNDC | CS_SAVEBITS> (), 
		_T(""),
		WS_POPUP, rcGroup, pwndOwner, NULL);

    if (bResult)
    {
		m_Wnd.m_pCentralGroup = this;

		if (globalData.IsWindowsLayerSupportAvailable ())
		{
			m_Wnd.ModifyStyleEx (0, WS_EX_LAYERED);

			if (!params.m_bIsAlphaMarkers && GetVMTheme () != BCGP_SDT_VS2008 && GetVMTheme () != BCGP_SDT_VS2010 && GetVMTheme () != BCGP_SDT_VS2012)
			{
				globalData.SetLayeredAttrib (m_Wnd.GetSafeHwnd(),
					params.m_clrTransparent, 0,
					LWA_COLORKEY);
			}
			else
			{
				m_Wnd.Update ();
			}

			m_bLayered = TRUE;
		}
		else
		{
			m_Wnd.SetWindowRgn (rgnAll, FALSE);
			m_bLayered = FALSE;
		}

		m_bCreated = TRUE;
	}
}

void CBCGPSDCentralGroup::Destroy ()
{
	if (!m_bCreated)
	{
		return;
	}

	CBCGPSmartDockingMarker::SDMarkerPlace i;
	for (i = CBCGPSmartDockingMarker::sdCLEFT; i <= CBCGPSmartDockingMarker::sdCMIDDLE; ++reinterpret_cast<int&>(i))
    {
		m_arMarkers[i - CBCGPSmartDockingMarker::sdCLEFT].DestroyImages ();
	}

	m_Wnd.DestroyWindow ();

	m_rgnBase.DeleteObject ();

	m_bCreated = FALSE;
}

void CBCGPSDCentralGroup::Show (BOOL bShow)
{
	if (::IsWindow(m_Wnd.m_hWnd))
	{
		m_Wnd.ShowWindow(bShow ? SW_SHOW : SW_HIDE);
	}
}
void CBCGPSDCentralGroup::GetWindowRect (CRect& rect)
{
	rect.SetRectEmpty ();
	if (m_Wnd.GetSafeHwnd () != NULL)
	{
		m_Wnd.GetWindowRect (rect);
	}
}

BOOL CBCGPSDCentralGroup::AdjustPos (CRect rcHost, int nMiddleIsOn)
{
	CRect rcWnd;
    if (m_Wnd.GetSafeHwnd () != NULL)
    {
        if (nMiddleIsOn != -1)
        {
            if (nMiddleIsOn == 0 && m_bMiddleIsOn)
            {
                m_bMiddleIsOn = FALSE;
				m_Wnd.Update ();
            }
            else
            if (nMiddleIsOn == 1 && !m_bMiddleIsOn)
            {
                m_bMiddleIsOn = TRUE;
				m_Wnd.Update ();
            }
        }

	    m_Wnd.GetClientRect (rcWnd);

	    int x = ((rcHost.right + rcHost.left) - rcWnd.Width()) >> 1;
	    int y = ((rcHost.bottom + rcHost.top) - rcWnd.Height()) >> 1;

        CRect rcCurrentPos;
        m_Wnd.GetWindowRect (rcCurrentPos);

        if (rcCurrentPos.left != x || rcCurrentPos.top != y)
        {
            m_Wnd.SetWindowPos (&CWnd::wndTopMost, x, y, -1, -1,
		        SWP_NOSIZE);

            return TRUE;
        }
    }

    return FALSE;
}

void CBCGPSDCentralGroup::ShowMarker (	CBCGPSmartDockingMarker::SDMarkerPlace nMarkerNo, 
										BOOL bShow/* = TRUE*/,
										BOOL bRedraw/* = TRUE*/)
{
	CBCGPSDCentralGroupMarker* pMarker = GetMarker (nMarkerNo);
	if (pMarker == NULL)
	{
		return;
	}

	if (pMarker->IsVisible () != bShow)
	{
		pMarker->SetVisible (bShow, bRedraw);
	}
}

void CBCGPSDCentralGroup::DrawCentralGroup (CDC& dc,
	CBrush& brBaseBackground, CBrush& brBaseBorder, CRect rectClient)
{
	CBCGPSmartDockingParams& params = CBCGPDockManager::GetSmartDockingParams ();
	const BOOL bAlphaMarkers = params.m_bIsAlphaMarkers || GetVMTheme () == BCGP_SDT_VS2008 || GetVMTheme () == BCGP_SDT_VS2010 || GetVMTheme () == BCGP_SDT_VS2012;

	CDC cmpdc;
	cmpdc.CreateCompatibleDC (&dc);

	if (!bAlphaMarkers)
	{
		// fill with the transparent color
		CRect rect;
		dc.GetBoundsRect (rect, 0);

		CBrush brBack (params.m_clrTransparent);
		dc.FillRect (rect, &brBack);
	}

	if (m_Image.IsValid ())
	{
		m_Image.DrawEx (&dc, rectClient, 0, 
			CBCGPToolBarImages::ImageAlignHorzCenter,
			CBCGPToolBarImages::ImageAlignVertCenter,
			CRect (0, 0, 0, 0), (BYTE)(bAlphaMarkers ? ALPHA_TRANSPARENT : 255));
	}
	else
	{
		dc.FillRgn (&m_rgnBase, &brBaseBackground);

		if (m_bMiddleIsOn &&
			params.m_uiMarkerBmpResID [0] == 0)	// Default images
		{
			CBCGPSDCentralGroupMarker& centerMarker = 
				m_arMarkers [CBCGPSmartDockingMarker::sdCMIDDLE - CBCGPSmartDockingMarker::sdCLEFT];

			if (centerMarker.IsVisible () && centerMarker.m_bHiLited)
			{
				CBrush br (COLOR_HIGHLIGHT_FRAME);
				dc.FrameRgn (&m_rgnBase, &br, 1, 1);
			}
			else
			{
				dc.FrameRgn (&m_rgnBase, &brBaseBorder, 1, 1);
			}
		}
		else
		{
			dc.FrameRgn (&m_rgnBase, &brBaseBorder, 1, 1);
		}
	}

	CBCGPSmartDockingMarker::SDMarkerPlace i;
    CBCGPSmartDockingMarker::SDMarkerPlace last = m_bMiddleIsOn ?
        CBCGPSmartDockingMarker::sdCMIDDLE :
        CBCGPSmartDockingMarker::sdCBOTTOM;

	for (i = CBCGPSmartDockingMarker::sdCLEFT; i <= last; ++reinterpret_cast<int&>(i))
    {
		CBCGPSDCentralGroupMarker& marker = m_arMarkers[i - CBCGPSmartDockingMarker::sdCLEFT];

		if (marker.IsVisible ())
		{
			marker.Draw (dc, bAlphaMarkers);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPMarkerBmpWnd

CBCGPMarkerBmpWnd::CBCGPMarkerBmpWnd ()
{
	m_bIsHighlighted = FALSE;
	m_bIsDefaultImage = FALSE;
	m_clrFrame = (COLORREF)-1;
	m_bIsVert = FALSE;
}

CBCGPMarkerBmpWnd::~CBCGPMarkerBmpWnd ()
{
}


BEGIN_MESSAGE_MAP(CBCGPMarkerBmpWnd, CWnd)
	//{{AFX_MSG_MAP(CBCGPMarkerBmpWnd)
	ON_WM_PAINT()
	ON_WM_CLOSE()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// if the window gets created, the region is deeded to Windows
BOOL CBCGPMarkerBmpWnd::Create (LPRECT pWndRect, HBITMAP hbmpFace, HRGN hrgnShape,
		CWnd* pwndOwner, BOOL bIsDefaultImage,
		BOOL bIsVert)
{
	// save data needed
	m_hbmpFace = hbmpFace;
	m_bIsDefaultImage = bIsDefaultImage;
	m_bIsVert = bIsVert;

	// create window with specified params
	BOOL res = CreateEx (0, GetSDWndClassName<CS_OWNDC | CS_SAVEBITS>(),
		_T(""), WS_POPUP, *pWndRect, pwndOwner, NULL);

	// if succeeded, set the region
	if (res)
	{
		SetWindowRgn (hrgnShape, FALSE);
	}

	COLORREF clrBaseGroupBackground;
	CBCGPVisualManager::GetInstance ()->GetSmartDockingBaseMarkerColors (
		clrBaseGroupBackground, m_clrFrame);

	return res;
}

void CBCGPMarkerBmpWnd::Highlight (BOOL bSet)
{
	m_bIsHighlighted = bSet;

	if (GetSafeHwnd () == NULL)
	{
		return;
	}

	RedrawWindow ();
	UpdateLayeredWindow ();
}

void CBCGPMarkerBmpWnd::UpdateLayeredWindow ()
{
	CBCGPSmartDockingParams& params = CBCGPDockManager::GetSmartDockingParams ();
	const BOOL bAlphaMarkers = params.m_bIsAlphaMarkers || GetVMTheme () == BCGP_SDT_VS2008 || GetVMTheme () == BCGP_SDT_VS2010 || GetVMTheme () == BCGP_SDT_VS2012;

	if (!bAlphaMarkers)
	{
		return;
	}

	CRect rect;
	GetClientRect (rect);

	CPoint point (0, 0);
	CSize size (rect.Size ());

	LPBYTE pBits = NULL;
	HBITMAP hBitmap = CBCGPDrawManager::CreateBitmap_32 (size, (void**)&pBits);
	if (hBitmap == NULL)
	{
		return;
	}

	CBitmap bitmap;
	bitmap.Attach (hBitmap);

	CClientDC clientDC(this);
	CDC dc;
	dc.CreateCompatibleDC (&clientDC);

	CBitmap* pBitmapOld = (CBitmap*)dc.SelectObject (&bitmap);

	dc.DrawState (point, size, m_hbmpFace, DSS_NORMAL);

	if (GetVMTheme () == BCGP_SDT_VS2010)
	{
		CBCGPDrawManager dm(dc);
		dm.DrawRect(rect, (COLORREF)-1, RGB(99, 104, 113));
	}

	BLENDFUNCTION bf;
	bf.BlendOp             = AC_SRC_OVER;
	bf.BlendFlags          = 0;
	bf.SourceConstantAlpha = (BYTE)(m_bIsHighlighted ? 255 : ALPHA_TRANSPARENT);
	bf.AlphaFormat         = LWA_COLORKEY;

	globalData.UpdateLayeredWindow (GetSafeHwnd (), NULL, 0, &size, dc.GetSafeHdc (), 
		&point, 0, &bf, 0x02);

	dc.SelectObject (pBitmapOld);
}

BOOL CBCGPMarkerBmpWnd::Assign (HBITMAP hbmpFace, BOOL bRedraw)
{
	if (hbmpFace != NULL)
	{
		m_hbmpFace = hbmpFace;
	}

	Invalidate (bRedraw);
	UpdateLayeredWindow ();

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPMarkerBmpWnd message handlers

// simply splash the bitmap onto window's surface
void CBCGPMarkerBmpWnd::OnPaint () 
{
	CPaintDC dc (this); // device context for painting

	CRect rectClient;
	GetClientRect (rectClient);

	dc.DrawState (CPoint (0, 0), rectClient.Size (), m_hbmpFace, DSS_NORMAL);

	if (!m_bIsDefaultImage || GetVMTheme () == BCGP_SDT_VS2008 || GetVMTheme () == BCGP_SDT_VS2010 || GetVMTheme () == BCGP_SDT_VS2012)
	{
		return;
	}

	COLORREF clrFrame = m_bIsHighlighted ? COLOR_HIGHLIGHT_FRAME : m_clrFrame;
	dc.Draw3dRect (rectClient, clrFrame, clrFrame);

	if (GetVMTheme () == BCGP_SDT_VS2005)
	{
		ShadeRect (&dc, rectClient, m_bIsVert);
	}
}

void CBCGPMarkerBmpWnd::OnClose () 
{
	// so that it does not get destroyed
}

BOOL CBCGPMarkerBmpWnd::OnEraseBkgnd (CDC* /*pDC*/)
{
	return TRUE;
}
