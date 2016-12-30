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
// BCGPRibbonStatusBarPane.cpp: implementation of the CBCGPRibbonStatusBarPane class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "bcgcbpro.h"
#include "bcgglobals.h"
#include "BCGPRibbonStatusBar.h"
#include "BCGPRibbonStatusBarPane.h"
#include "BCGPVisualManager.h"
#include "BCGPPngImage.h"

#ifndef BCGP_EXCLUDE_RIBBON

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CMap<UINT,UINT,CBCGPRibbonStatusBarPane*,CBCGPRibbonStatusBarPane*> CBCGPRibbonStatusBarPane::m_mapAnimations;
CCriticalSection CBCGPRibbonStatusBarPane::g_cs;

IMPLEMENT_DYNCREATE(CBCGPRibbonStatusBarPane, CBCGPRibbonButton)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGPRibbonStatusBarPane::CBCGPRibbonStatusBarPane()
{
	CommonInit ();
}
//***********************************************************************************************
CBCGPRibbonStatusBarPane::CBCGPRibbonStatusBarPane (UINT nCmdID, 
	LPCTSTR lpszText, BOOL bIsStatic, HICON hIcon, LPCTSTR lpszAlmostLargeText, BOOL bAlphaBlendIcon) :
	CBCGPRibbonButton (nCmdID, lpszText, hIcon, FALSE, NULL, FALSE, bAlphaBlendIcon)
{
	CommonInit ();

	m_strAlmostLargeText = (lpszAlmostLargeText == NULL) ? _T("") : lpszAlmostLargeText;
	m_bIsStatic = bIsStatic;
}
//***********************************************************************************************
CBCGPRibbonStatusBarPane::CBCGPRibbonStatusBarPane (UINT nCmdID, 
	LPCTSTR lpszText, HBITMAP hBmpAnimationList, int cxAnimation,
	COLORREF clrTransp, HICON hIcon, BOOL bIsStatic, BOOL bDontScaleInHighDPIMode, BOOL bAlphaBlendIcon) :
	CBCGPRibbonButton (nCmdID, lpszText, hIcon, FALSE, NULL, FALSE, bAlphaBlendIcon)
{
	CommonInit ();

	m_bIsStatic = bIsStatic;

	SetAnimationList (hBmpAnimationList, cxAnimation, clrTransp, bDontScaleInHighDPIMode);
}
//***********************************************************************************************
CBCGPRibbonStatusBarPane::CBCGPRibbonStatusBarPane (UINT nCmdID, 
	LPCTSTR lpszText, UINT uiAnimationListResID, int cxAnimation, 
	COLORREF clrTransp, HICON hIcon, BOOL bIsStatic, BOOL bDontScaleInHighDPIMode, BOOL bAlphaBlendIcon) :
	CBCGPRibbonButton (nCmdID, lpszText, hIcon, FALSE, NULL, FALSE, bAlphaBlendIcon)
{
	CommonInit ();

	m_bIsStatic = bIsStatic;

	SetAnimationList (uiAnimationListResID, cxAnimation, clrTransp, bDontScaleInHighDPIMode);
}
//***********************************************************************************************
void CBCGPRibbonStatusBarPane::CommonInit ()
{
	m_bIsExtended = FALSE;
	m_bIsStatic = TRUE;
	m_szMargin = CSize (9, 0);
	m_bTextAlwaysOnRight = TRUE;
	m_nTextAlign = TA_LEFT;
	m_bIsTextTruncated = FALSE;
	m_nAnimTimerID = 0;
	m_nAnimationIndex = -1;
	m_nAnimationDuration = 0;
	m_dwAnimationStartTime = 0;
}
//***********************************************************************************************
CBCGPRibbonStatusBarPane::~CBCGPRibbonStatusBarPane()
{
	StopAnimation ();
}
//*****************************************************************************
COLORREF CBCGPRibbonStatusBarPane::OnFillBackground (CDC* pDC)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);

	BOOL bIsHighlighted = m_bIsHighlighted;
	BOOL bIsPressed = m_bIsPressed;
	BOOL bIsDisabled = m_bIsDisabled;

	if (m_bIsStatic)
	{
		m_bIsDisabled = FALSE;
	}

	if (m_bIsStatic || m_bIsDisabled)
	{
		m_bIsHighlighted = FALSE;
		m_bIsPressed = FALSE;
	}

	COLORREF clrText = CBCGPVisualManager::GetInstance ()->OnDrawRibbonStatusBarPane (pDC, 
				DYNAMIC_DOWNCAST (CBCGPRibbonStatusBar, m_pRibbonBar),
				this);

	m_bIsHighlighted = bIsHighlighted;
	m_bIsPressed = bIsPressed;
	m_bIsDisabled = bIsDisabled;

	return clrText;
}
//*****************************************************************************
void CBCGPRibbonStatusBarPane::OnCalcTextSize (CDC* pDC)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);

	CBCGPRibbonButton::OnCalcTextSize (pDC);

	if (!m_strAlmostLargeText.IsEmpty ())
	{
		const int nTextWidth = pDC->GetTextExtent (m_strAlmostLargeText).cx;

		m_bIsTextTruncated = nTextWidth < m_sizeTextRight.cx;
		m_sizeTextRight.cx = nTextWidth;
	}
}
//*****************************************************************************
int CBCGPRibbonStatusBarPane::DoDrawText (CDC* pDC, const CString& strText, CRect rectText, UINT /*uiDTFlags*/)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);

	UINT uiDTFlags = DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS;

	if (m_nTextAlign == TA_CENTER)
	{
		uiDTFlags |= DT_CENTER;
	}
	else if (m_nTextAlign == TA_RIGHT)
	{
		uiDTFlags |= DT_RIGHT;
		rectText.right -= m_szMargin.cx;
	}

	return CBCGPRibbonButton::DoDrawText (pDC, strText, rectText, uiDTFlags);
}
//******************************************************************************
CString CBCGPRibbonStatusBarPane::GetToolTipText () const
{
	ASSERT_VALID (this);

	CString str = CBCGPRibbonButton::GetToolTipText ();

	if (!str.IsEmpty ())
	{
		return str;
	}

	if (m_bIsTextTruncated || m_AnimImages.GetCount () > 0)
	{
		str = m_strText;
	}

	if (str.IsEmpty () && !m_strDescription.IsEmpty ())
	{
		str = m_strText;
	}

	return str;
}
//*****************************************************************************
void CBCGPRibbonStatusBarPane::CopyFrom (const CBCGPBaseRibbonElement& s)
{
	ASSERT_VALID (this);

	CBCGPBaseRibbonElement::CopyFrom (s);

	CBCGPRibbonStatusBarPane& src = (CBCGPRibbonStatusBarPane&) s;

	m_bIsStatic = src.m_bIsStatic;
	m_bIsExtended = src.m_bIsExtended;
	m_strAlmostLargeText = src.m_strAlmostLargeText;
	m_nTextAlign = src.m_nTextAlign;
	m_bIsTextTruncated = src.m_bIsTextTruncated;
	
	src.m_AnimImages.CopyTo (m_AnimImages);
}
//*****************************************************************************
void CBCGPRibbonStatusBarPane::SetAnimationList (HBITMAP hBmpAnimationList, int cxAnimation,
												 COLORREF clrTransp,
												 BOOL bDontScaleInHighDPIMode)
{
	ASSERT_VALID (this);

	if (m_AnimImages.IsValid ())
	{
		m_AnimImages.Clear ();
	}

	if (hBmpAnimationList == NULL)
	{
		return;
	}

	BITMAP bitmap;
	::GetObject (hBmpAnimationList, sizeof(BITMAP), &bitmap);

	int cy = bitmap.bmHeight;

	m_AnimImages.SetImageSize (CSize (cxAnimation, cy));
	m_AnimImages.SetTransparentColor (clrTransp);
	m_AnimImages.AddImage (hBmpAnimationList, TRUE);

	if (!bDontScaleInHighDPIMode)
	{
		m_AnimImages.SmoothResize (globalData.GetRibbonImageScale ());
	}
}
//*****************************************************************************
BOOL CBCGPRibbonStatusBarPane::SetAnimationList (UINT uiAnimationListResID, int cxAnimation,
												 COLORREF clrTransp, BOOL bDontScaleInHighDPIMode)
{
	ASSERT_VALID (this);

	if (m_AnimImages.IsValid ())
	{
		m_AnimImages.Clear ();
	}

	if (uiAnimationListResID == 0)
	{
		return TRUE;
	}

	LPCTSTR lpszResourceName = MAKEINTRESOURCE (uiAnimationListResID);
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
			return FALSE;
		}

		UINT uiLoadImageFlags = LR_CREATEDIBSECTION | LR_LOADMAP3DCOLORS;

		hbmp = (HBITMAP) ::LoadImage (
			hinstRes,
			lpszResourceName,
			IMAGE_BITMAP,
			0, 0,
			uiLoadImageFlags);
	}

	if (hbmp == NULL)
	{
		return FALSE;
	}

	SetAnimationList (hbmp, cxAnimation, clrTransp, bDontScaleInHighDPIMode);
	return TRUE;
}
//*******************************************************************************
CSize CBCGPRibbonStatusBarPane::GetIntermediateSize (CDC* pDC)
{
	ASSERT_VALID (this);

	if (m_AnimImages.GetCount () > 0)
	{
		CSize imageSize = m_AnimImages.GetImageSize ();

		return CSize (
			imageSize.cx + 2 * m_szMargin.cx + m_sizePadding.cx,
			imageSize.cy + 2 * m_szMargin.cy + m_sizePadding.cy);
	}

	CSize size = CBCGPRibbonButton::GetIntermediateSize (pDC);
	size.cx -= GetTextOffset () + 1;

	return size;
}
//*******************************************************************************
void CBCGPRibbonStatusBarPane::OnDraw (CDC* pDC)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);

	if (m_AnimImages.GetCount () == 0)
	{
		CBCGPRibbonButton::OnDraw (pDC);
		return;
	}

	if (m_rect.IsRectEmpty ())
	{
		return;
	}

	OnFillBackground (pDC);

	if (m_nAnimationIndex < 0)
	{
		CString strText = m_strText;
		m_strText.Empty ();

		CBCGPRibbonButton::OnDraw (pDC);
		m_strText = strText;
	}
	else
	{
		m_AnimImages.DrawEx (pDC, m_rect, m_nAnimationIndex,
			CBCGPToolBarImages::ImageAlignHorzCenter, CBCGPToolBarImages::ImageAlignVertCenter);
	}

	OnDrawBorder (pDC);
}
//*******************************************************************************
void CBCGPRibbonStatusBarPane::StartAnimation (UINT nFrameDelay, UINT nDuration)
{
	ASSERT_VALID (this);

	if (m_AnimImages.GetCount () == 0)
	{
		ASSERT (FALSE);
		return;
	}

	StopAnimation ();

	m_nAnimationIndex = 0;

	if ((m_nAnimationDuration = nDuration) > 0)
	{
		m_dwAnimationStartTime = ::GetTickCount ();
	}

	m_nAnimTimerID = (UINT) ::SetTimer (NULL, 0, nFrameDelay, AnimTimerProc);

	g_cs.Lock ();
	m_mapAnimations.SetAt (m_nAnimTimerID, this);
	g_cs.Unlock ();
}
//*******************************************************************************
void CBCGPRibbonStatusBarPane::StopAnimation ()
{
	ASSERT_VALID (this);

	if (m_nAnimTimerID == 0)
	{
		return;
	}

	::KillTimer (NULL, m_nAnimTimerID);

	g_cs.Lock ();
	m_mapAnimations.RemoveKey (m_nAnimTimerID);
	g_cs.Unlock ();

	m_nAnimTimerID = 0;
	m_nAnimationIndex = -1;

	OnFinishAnimation ();

	Redraw ();
}
//*******************************************************************************
VOID CALLBACK CBCGPRibbonStatusBarPane::AnimTimerProc (HWND /*hwnd*/, UINT /*uMsg*/,
													   UINT_PTR idEvent, DWORD dwTime)
{
	CBCGPRibbonStatusBarPane* pPane = NULL;

	g_cs.Lock ();

	if (!m_mapAnimations.Lookup ((UINT) idEvent, pPane))
	{
		g_cs.Unlock ();
		return;
	}

	ASSERT_VALID (pPane);

	g_cs.Unlock ();

	if (pPane->m_nAnimationDuration > 0)
	{
		if (dwTime - pPane->m_dwAnimationStartTime > (DWORD) pPane->m_nAnimationDuration)
		{
			pPane->StopAnimation ();
			return;
		}
	}

	pPane->m_nAnimationIndex++;

	if (pPane->m_nAnimationIndex >= pPane->m_AnimImages.GetCount ())
	{
		pPane->m_nAnimationIndex = 0;
	}

	pPane->Redraw ();
}
//*******************************************************************************
void CBCGPRibbonStatusBarPane::OnRTLChanged (BOOL bIsRTL)
{
	ASSERT_VALID (this);

	CBCGPRibbonButton::OnRTLChanged (bIsRTL);

	m_AnimImages.Mirror ();
}

#endif // BCGP_EXCLUDE_RIBBON
