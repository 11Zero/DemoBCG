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
// BCGPStatusBar.cpp : implementation file
//

#include "stdafx.h"
#include "BCGGlobals.h"
#include "BCGPFrameWnd.h"
#include "BCGPMDIFrameWnd.h"
#include "BCGPOleIPFrameWnd.h"
#include "BCGPOleDocIPFrameWnd.h"
#include "BCGPMDIChildWnd.h"
#include "BCGPOleCntrFrameWnd.h"
#include "BCGPGlobalUtils.h"

#include "BCGPVisualManager.h"
#include "BCGPDrawManager.h"
#include "BCGPStatusBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBCGPStatusBar

inline CBCGStatusBarPaneInfo* CBCGPStatusBar::_GetPanePtr(int nIndex) const
{
	if (nIndex == 255 && m_nCount < 255)
	{
		// Special case for the simple pane
		for (int i = 0; i < m_nCount; i++)
		{
			CBCGStatusBarPaneInfo* pSBP = _GetPanePtr (i);
			ASSERT (pSBP != NULL);

			if (pSBP->nStyle & SBPS_STRETCH)
			{
				return pSBP;
			}
		}
	}

	if (nIndex < 0 || nIndex >= m_nCount)
	{
		return NULL;
	}

	if (m_pData == NULL)
	{
		ASSERT (FALSE);
		return NULL;
	}

	return ((CBCGStatusBarPaneInfo*)m_pData) + nIndex;
}

#ifdef AFX_INIT_SEG
#pragma code_seg(AFX_INIT_SEG)
#endif

const int nTextMargin = 4;	// Gap between image and text

CBCGPStatusBar::CBCGPStatusBar()
{
	m_hFont = NULL;

	// setup correct margins
	m_cxSizeBox = 0;

	m_cxLeftBorder = 4;
	m_cyTopBorder = 2;
	m_cyBottomBorder = 0;
	m_cxRightBorder = 0;

	m_bPaneDoubleClick = FALSE;
	m_bDrawExtendedArea = FALSE;

	m_rectSizeBox.SetRectEmpty ();
}
//********************************************************************************
void CBCGPStatusBar::OnSettingChange(UINT /*uFlags*/, LPCTSTR /* lpszSection */)
{
	RecalcLayout ();
}
//********************************************************************************
CBCGPStatusBar::~CBCGPStatusBar()
{
}
//********************************************************************************
void CBCGPStatusBar::OnDestroy() 
{
	for (int i = 0; i < m_nCount; i++)
	{
		VERIFY(SetPaneText(i, NULL, FALSE));    // no update
		SetTipText(i, NULL);
		SetPaneIcon(i, NULL, FALSE);
	}

	CBCGPControlBar::OnDestroy();
}
//********************************************************************************
BOOL CBCGPStatusBar::PreCreateWindow(CREATESTRUCT& cs)
{
	// in Win4, status bars do not have a border at all, since it is
	//  provided by the client area.
	if ((m_dwStyle & (CBRS_ALIGN_ANY|CBRS_BORDER_ANY)) == CBRS_BOTTOM)
	{
		m_dwStyle &= ~(CBRS_BORDER_ANY|CBRS_BORDER_3D);
	}

	return CBCGPControlBar::PreCreateWindow(cs);
}
//********************************************************************************
BOOL CBCGPStatusBar::Create(CWnd* pParentWnd, DWORD dwStyle, UINT nID)
{
	return CreateEx(pParentWnd, 0, dwStyle, nID);
}
//********************************************************************************
BOOL CBCGPStatusBar::CreateEx (CWnd* pParentWnd, DWORD /*dwCtrlStyle*/, DWORD dwStyle, 
							 UINT nID)
{
	ASSERT_VALID(pParentWnd);   // must have a parent

	// save the style
	SetBarAlignment (dwStyle & CBRS_ALL);

	// create the HWND
	CRect rect;
	rect.SetRectEmpty();

	m_dwBCGStyle = 0; // can't float, resize, close, slide

	if (pParentWnd->GetStyle() & WS_THICKFRAME)
	{
		dwStyle |= SBARS_SIZEGRIP;
	}

	if (!CWnd::Create (globalData.RegisterWindowClass (_T("BCGPStatusBar")),
		NULL, dwStyle | WS_CLIPSIBLINGS, rect, pParentWnd, nID))
	{
		return FALSE;
	}

	if (pParentWnd->IsKindOf (RUNTIME_CLASS (CBCGPFrameWnd)))
	{
		((CBCGPFrameWnd*) pParentWnd)->AddControlBar (this);
	}
	else if (pParentWnd->IsKindOf (RUNTIME_CLASS (CBCGPMDIFrameWnd)))
	{
		((CBCGPMDIFrameWnd*) pParentWnd)->AddControlBar (this);
	}
	else if (pParentWnd->IsKindOf (RUNTIME_CLASS (CBCGPOleIPFrameWnd)))
	{
		((CBCGPOleIPFrameWnd*) pParentWnd)->AddControlBar (this);
	}
	else if (pParentWnd->IsKindOf (RUNTIME_CLASS (CBCGPOleDocIPFrameWnd)))
	{
		((CBCGPOleDocIPFrameWnd*) pParentWnd)->AddControlBar (this);
	}
	else if (pParentWnd->IsKindOf (RUNTIME_CLASS (CBCGPMDIChildWnd)))
	{
		((CBCGPMDIChildWnd*) pParentWnd)->AddControlBar (this);
	}
	else if (pParentWnd->IsKindOf (RUNTIME_CLASS (CBCGPOleCntrFrameWnd)))
	{
		((CBCGPOleCntrFrameWnd*) pParentWnd)->AddControlBar (this);
	}
	else if (pParentWnd->IsKindOf (RUNTIME_CLASS (CDialog)))
	{
		if (pParentWnd->GetSafeHwnd() == AfxGetMainWnd()->GetSafeHwnd())
		{
			globalUtils.m_bDialogApp = TRUE;
		}
	}

	return TRUE;
}
//********************************************************************************
BOOL CBCGPStatusBar::SetIndicators(const UINT* lpIDArray, int nIDCount)
{
	ASSERT_VALID(this);
	ASSERT(nIDCount >= 1);  // must be at least one of them
	ASSERT(lpIDArray == NULL ||
		AfxIsValidAddress(lpIDArray, sizeof(UINT) * nIDCount, FALSE));

	// free strings before freeing array of elements
	for (int i = 0; i < m_nCount; i++)
	{
		VERIFY(SetPaneText(i, NULL, FALSE));    // no update
		 //free Imagelist if any exist
		SetPaneIcon(i, NULL, FALSE);

	}


	// first allocate array for panes and copy initial data
	if (!AllocElements(nIDCount, sizeof(CBCGStatusBarPaneInfo)))
		return FALSE;

	ASSERT(nIDCount == m_nCount);

	HFONT hFont = GetCurrentFont ();

	BOOL bOK = TRUE;
	if (lpIDArray != NULL)
	{
		ASSERT(hFont != NULL);        // must have a font !
		CString strText;
		CClientDC dcScreen(NULL);
		HGDIOBJ hOldFont = dcScreen.SelectObject(hFont);

		for (int i = 0; i < nIDCount; i++)
		{
			CBCGStatusBarPaneInfo* pSBP = _GetPanePtr(i);
			if (pSBP == NULL)
			{
				ASSERT (FALSE);
				return FALSE;
			}

			pSBP->nStyle = 0;
			pSBP->lpszText = NULL;
			pSBP->lpszToolTip = NULL;
			pSBP->clrText = (COLORREF)-1;
			pSBP->clrBackground = (COLORREF)-1;
			pSBP->hImage = NULL;
			pSBP->cxIcon = 0;
			pSBP->cyIcon = 0;
			pSBP->rect = CRect (0, 0, 0, 0);
			pSBP->nFrameCount = 0;
			pSBP->nCurrFrame = 0;
			pSBP->nProgressCurr = 0;
			pSBP->nProgressTotal = -1;
			pSBP->clrProgressBar = (COLORREF)-1;
			pSBP->clrProgressBarDest = (COLORREF)-1;
			pSBP->clrProgressText = (COLORREF)-1;
			pSBP->bProgressText = FALSE;

			pSBP->nID = *lpIDArray++;
			if (pSBP->nID != 0)
			{
				if (!strText.LoadString(pSBP->nID))
				{
					TRACE1("Warning: failed to load indicator string 0x%04X.\n",
						pSBP->nID);
					bOK = FALSE;
					break;
				}

				pSBP->cxText = dcScreen.GetTextExtent(strText,
						strText.GetLength()).cx;
				ASSERT(pSBP->cxText >= 0);

				if (!SetPaneText(i, strText, FALSE))
				{
					bOK = FALSE;
					break;
				}
			}
			else
			{
				// no indicator (must access via index)
				// default to 1/4 the screen width (first pane is stretchy)
				pSBP->cxText = ::GetSystemMetrics(SM_CXSCREEN) / 4;

				if (i == 0)
				{
					pSBP->nStyle |= (SBPS_STRETCH | SBPS_NOBORDERS);
				}
			}
		}

		dcScreen.SelectObject(hOldFont);
	}

	RecalcLayout ();
	return bOK;
}

#ifdef AFX_CORE3_SEG
#pragma code_seg(AFX_CORE3_SEG)
#endif

/////////////////////////////////////////////////////////////////////////////
// CBCGPStatusBar attribute access

int CBCGPStatusBar::CommandToIndex(UINT nIDFind) const
{
	ASSERT_VALID(this);

	if (m_nCount <= 0)
	{
		return -1;
	}

	CBCGStatusBarPaneInfo* pSBP = _GetPanePtr(0);
	if (pSBP == NULL)
	{
		ASSERT (FALSE);
		return -1;
	}

	for (int i = 0; i < m_nCount; i++, pSBP++)
	{
		if (pSBP->nID == nIDFind)
		{
			return i;
		}
	}

	return -1;
}
//*******************************************************************************
UINT CBCGPStatusBar::GetItemID(int nIndex) const
{
	ASSERT_VALID(this);
	
	CBCGStatusBarPaneInfo* pSBP = _GetPanePtr(nIndex);
	if (pSBP == NULL)
	{
		ASSERT (FALSE);
		return 0;
	}

	return pSBP->nID;
}
//*******************************************************************************
void CBCGPStatusBar::GetItemRect(int nIndex, LPRECT lpRect) const
{
	ASSERT_VALID(this);
	ASSERT(AfxIsValidAddress(lpRect, sizeof(RECT)));

	CBCGStatusBarPaneInfo* pSBP = _GetPanePtr(nIndex);
	if (pSBP == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	*lpRect = pSBP->rect;
}
//*******************************************************************************
UINT CBCGPStatusBar::GetPaneStyle(int nIndex) const
{
	CBCGStatusBarPaneInfo* pSBP = _GetPanePtr(nIndex);
	if (pSBP == NULL)
	{
		ASSERT (FALSE);
		return 0;
	}

	return pSBP->nStyle;
}
//*******************************************************************************
void CBCGPStatusBar::SetPaneStyle(int nIndex, UINT nStyle)
{
	CBCGStatusBarPaneInfo* pSBP = _GetPanePtr(nIndex);
	if (pSBP == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	if (pSBP->nStyle != nStyle)
	{
		// just change the style of 1 pane, and invalidate it
		pSBP->nStyle = nStyle;
		InvalidateRect (&pSBP->rect, FALSE);
		UpdateWindow ();
	}
}
//*******************************************************************************
int CBCGPStatusBar::GetPaneWidth (int nIndex) const
{
	CBCGStatusBarPaneInfo* pSBP = _GetPanePtr(nIndex);
	if (pSBP == NULL)
	{
		ASSERT (FALSE);
		return 0;
	}

	CRect rect = pSBP->rect;
	return rect.Width ();
}
//********************************************************************************
void CBCGPStatusBar::SetPaneWidth (int nIndex, int cx)
{
	CBCGStatusBarPaneInfo* pSBP = _GetPanePtr(nIndex);
	if (pSBP == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	CRect rect = pSBP->rect;
	int cxCurr = rect.Width () - CX_BORDER * 4;

	int cxTextNew = cx - pSBP->cxIcon;
	if (pSBP->cxIcon > 0)
	{
		cxTextNew -= nTextMargin;
	}

	pSBP->cxText = max (0, cxTextNew);

	if (cx != cxCurr)
	{
		RecalcLayout ();
		Invalidate();
		UpdateWindow ();
	}
}
//********************************************************************************
void CBCGPStatusBar::GetPaneInfo(int nIndex, UINT& nID, UINT& nStyle,
	int& cxWidth) const
{
	ASSERT_VALID(this);

	CBCGStatusBarPaneInfo* pSBP = _GetPanePtr(nIndex);
	if (pSBP == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	nID = pSBP->nID;
	nStyle = pSBP->nStyle;

	CRect rect = pSBP->rect;
	cxWidth = rect.Width ();
}
//*******************************************************************************
void CBCGPStatusBar::SetPaneInfo(int nIndex, UINT nID, UINT nStyle, int cxWidth)
{
	ASSERT_VALID(this);

	CBCGStatusBarPaneInfo* pSBP = _GetPanePtr(nIndex);
	if (pSBP == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	pSBP->nID = nID;
	SetPaneStyle(nIndex, nStyle);
	SetPaneWidth (nIndex, cxWidth);
}
//*******************************************************************************
void CBCGPStatusBar::GetPaneText(int nIndex, CString& s) const
{
	ASSERT_VALID(this);

	CBCGStatusBarPaneInfo* pSBP = _GetPanePtr(nIndex);
	if (pSBP == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	s = pSBP->lpszText == NULL ? _T("") : pSBP->lpszText;
}
//*******************************************************************************
CString CBCGPStatusBar::GetPaneText(int nIndex) const
{
	ASSERT_VALID(this);

	CBCGStatusBarPaneInfo* pSBP = _GetPanePtr(nIndex);
	if (pSBP == NULL)
	{
		ASSERT (FALSE);
		return _T("");
	}

	CString s = pSBP->lpszText == NULL ? _T("") : pSBP->lpszText;
	return s;
}
//*******************************************************************************
BOOL CBCGPStatusBar::SetPaneText(int nIndex, LPCTSTR lpszNewText, BOOL bUpdate)
{
	ASSERT_VALID(this);

	CBCGStatusBarPaneInfo* pSBP = _GetPanePtr(nIndex);
	if (pSBP == NULL)
	{
		return FALSE;
	}

	if (pSBP->lpszText != NULL)
	{
		if (lpszNewText != NULL && lstrcmp(pSBP->lpszText, lpszNewText) == 0)
		{
			return TRUE;        // nothing to change
		}

		free((LPVOID)pSBP->lpszText);
	}
	else if (lpszNewText == NULL || *lpszNewText == '\0')
	{
		return TRUE; // nothing to change
	}

	BOOL bOK = TRUE;
	if (lpszNewText == NULL || *lpszNewText == '\0')
	{
		pSBP->lpszText = NULL;
	}
	else
	{
		pSBP->lpszText = _tcsdup(lpszNewText);
		if (pSBP->lpszText == NULL)
			bOK = FALSE; // old text is lost and replaced by NULL
	}

	if (bUpdate)
	{
		InvalidatePaneContent (nIndex);
	}

	return bOK;
}
//*******************************************************************************
void CBCGPStatusBar::SetPaneIcon (int nIndex, HICON hIcon, BOOL bUpdate, BOOL bAlphaBlend/* = FALSE*/)
{
	ASSERT_VALID(this);

	CBCGStatusBarPaneInfo* pSBP = _GetPanePtr(nIndex);
	if (pSBP == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	// Disable animation (if exist):
	SetPaneAnimation (nIndex, NULL, 0, FALSE);

	if (hIcon == NULL)
	{
		if (pSBP->hImage != NULL)
		{
			::ImageList_Destroy (pSBP->hImage);
		}

		pSBP->hImage = NULL;

		if (bUpdate)
		{
			InvalidatePaneContent (nIndex);
		}

		return;
	}

	ICONINFO iconInfo;
	::GetIconInfo (hIcon, &iconInfo);

	BITMAP bitmap;
	::GetObject (iconInfo.hbmColor, sizeof (BITMAP), &bitmap);

	::DeleteObject (iconInfo.hbmColor);
	::DeleteObject (iconInfo.hbmMask);

	if (pSBP->hImage == NULL)
	{
		pSBP->cxIcon = bitmap.bmWidth;
		pSBP->cyIcon = bitmap.bmHeight;

		DWORD dwFlags = ILC_MASK | ILC_COLORDDB;
		if (bAlphaBlend)
		{
			dwFlags = ILC_COLOR32;
		}

		pSBP->hImage = ::ImageList_Create (pSBP->cxIcon, pSBP->cyIcon, dwFlags, 1, 0);
		::ImageList_AddIcon (pSBP->hImage, hIcon);

		RecalcLayout ();
	}
	else
	{
		ASSERT (pSBP->cxIcon == bitmap.bmWidth);
		ASSERT (pSBP->cyIcon == bitmap.bmHeight);

		::ImageList_ReplaceIcon (pSBP->hImage, 0, hIcon);
	}

	if (bUpdate)
	{
		InvalidatePaneContent (nIndex);
	}
}
//*******************************************************************************
void CBCGPStatusBar::SetPaneIcon (int nIndex, HBITMAP hBmp,
								 COLORREF clrTransparent, BOOL bUpdate, BOOL bAlphaBlend/* = FALSE*/)
{
	ASSERT_VALID(this);

	CBCGStatusBarPaneInfo* pSBP = _GetPanePtr(nIndex);
	if (pSBP == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	// Disable animation (if exist):
	SetPaneAnimation (nIndex, NULL, 0, FALSE);

	if (hBmp == NULL)
	{
		if (pSBP->hImage != NULL)
		{
			::ImageList_Destroy (pSBP->hImage);
		}

		pSBP->hImage = NULL;

		if (bUpdate)
		{
			InvalidatePaneContent (nIndex);
		}

		return;
	}

	BITMAP bitmap;
	::GetObject (hBmp, sizeof (BITMAP), &bitmap);

	if (pSBP->hImage == NULL)
	{
		pSBP->cxIcon = bitmap.bmWidth;
		pSBP->cyIcon = bitmap.bmHeight;

		DWORD dwFlags = ILC_MASK | ILC_COLORDDB;
		if (bAlphaBlend)
		{
			dwFlags = ILC_COLOR32;
		}

		pSBP->hImage = ::ImageList_Create (pSBP->cxIcon, pSBP->cyIcon, dwFlags, 1, 0);
		RecalcLayout ();
	}
	else
	{
		ASSERT (pSBP->cxIcon == bitmap.bmWidth);
		ASSERT (pSBP->cyIcon == bitmap.bmHeight);

		::ImageList_Remove (pSBP->hImage, 0);
	}

	//---------------------------------------------------------
	// Because ImageList_AddMasked changes the original bitmap,
	// we need to create a copy:
	//---------------------------------------------------------
	HBITMAP hbmpCopy = (HBITMAP) ::CopyImage (hBmp, IMAGE_BITMAP, 0, 0, 0);

	if (bAlphaBlend)
	{
		::ImageList_Add (pSBP->hImage, hbmpCopy, NULL);
	}
	else	
	{
		::ImageList_AddMasked (pSBP->hImage, hbmpCopy, clrTransparent);
	}

	::DeleteObject (hbmpCopy);

	if (bUpdate)
	{
		InvalidatePaneContent (nIndex);
	}
}
//*******************************************************************************
void CBCGPStatusBar::SetPaneAnimation (int nIndex, HIMAGELIST hImageList, 
							UINT nFrameRate, BOOL bUpdate, BOOL bAlphaBlend)
{
	ASSERT_VALID(this);

	CBCGStatusBarPaneInfo* pSBP = _GetPanePtr(nIndex);
	if (pSBP == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	if (pSBP->nFrameCount > 0)
	{
		KillTimer (pSBP->nID);
	}

	if (pSBP->hImage != NULL)
	{
		::ImageList_Destroy (pSBP->hImage);
		pSBP->hImage = NULL;
	}

	pSBP->nCurrFrame = 0;
	pSBP->nFrameCount = 0;

	if (hImageList == NULL)
	{
		if (bUpdate)
		{
			InvalidatePaneContent (nIndex);
		}

		return;
	}

	pSBP->nFrameCount = ::ImageList_GetImageCount (hImageList);
	if (pSBP->nFrameCount == 0)
	{
		if (bUpdate)
		{
			InvalidatePaneContent (nIndex);
		}

		return;
	}

	::ImageList_GetIconSize (hImageList, &pSBP->cxIcon, &pSBP->cyIcon);

	pSBP->hImage = ::ImageList_Create (pSBP->cxIcon, pSBP->cyIcon, 
			bAlphaBlend ? ILC_COLOR32 : ILC_MASK | ILC_COLORDDB, 1, 1);

	for (int i =0; i < pSBP->nFrameCount; i++)
	{
		HICON hIcon = ::ImageList_GetIcon (hImageList, i, ILD_TRANSPARENT);
		::ImageList_AddIcon (pSBP->hImage, hIcon);
		::DestroyIcon (hIcon);
	}

	RecalcLayout ();
	if (bUpdate)
	{
		InvalidatePaneContent (nIndex);
	}

	SetTimer (pSBP->nID, nFrameRate, NULL);
}
//*******************************************************************************
void CBCGPStatusBar::EnablePaneProgressBar (int nIndex, long nTotal, 
										   BOOL bDisplayText,
										   COLORREF clrBar, COLORREF clrBarDest,
										   COLORREF clrProgressText)
{
	ASSERT_VALID(this);

	CBCGStatusBarPaneInfo* pSBP = _GetPanePtr(nIndex);
	if (pSBP == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	pSBP->bProgressText = bDisplayText;
	pSBP->clrProgressBar = clrBar;
	pSBP->clrProgressBarDest = clrBarDest;
	pSBP->nProgressTotal = nTotal;
	pSBP->nProgressCurr = 0;
	pSBP->clrProgressText = clrProgressText;

	if (clrBarDest != (COLORREF)-1 && pSBP->bProgressText)
	{
		// Progress text is not available when the gradient is ON
		ASSERT (FALSE);
		pSBP->bProgressText = FALSE;
	}

	InvalidatePaneContent (nIndex);
}
//*******************************************************************************
void CBCGPStatusBar::SetPaneProgress (int nIndex, long nCurr, BOOL bUpdate)
{
	ASSERT_VALID(this);

	CBCGStatusBarPaneInfo* pSBP = _GetPanePtr(nIndex);
	if (pSBP == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	ASSERT (nCurr >= 0);
	ASSERT (nCurr <= pSBP->nProgressTotal);

	long lPos = min (max (0, nCurr), pSBP->nProgressTotal);
	if (pSBP->nProgressCurr != lPos)
	{
		pSBP->nProgressCurr = lPos;

		if (bUpdate)
		{
			InvalidatePaneContent (nIndex);
		}
	}
}
//*******************************************************************************
void CBCGPStatusBar::SetPaneTextColor (int nIndex, COLORREF clrText, BOOL bUpdate)
{
	ASSERT_VALID(this);

	CBCGStatusBarPaneInfo* pSBP = _GetPanePtr(nIndex);
	if (pSBP == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	if (pSBP->clrText != clrText)
	{
		pSBP->clrText = clrText;

		if (bUpdate)
		{
			InvalidatePaneContent (nIndex);
		}
	}
}
//*******************************************************************************
void CBCGPStatusBar::SetPaneBackgroundColor (int nIndex, COLORREF clrBackground, BOOL bUpdate)
{
	ASSERT_VALID(this);

	CBCGStatusBarPaneInfo* pSBP = _GetPanePtr(nIndex);
	if (pSBP == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	if (pSBP->clrBackground != clrBackground)
	{
		pSBP->clrBackground = clrBackground;

		if (bUpdate)
		{
			InvalidatePaneContent (nIndex);
		}
	}
}
//*******************************************************************************
CString CBCGPStatusBar::GetTipText(int nIndex) const
{
	ASSERT_VALID(this);

	CBCGStatusBarPaneInfo* pSBP = _GetPanePtr(nIndex);
	if (pSBP == NULL)
	{
		ASSERT (FALSE);
		return _T("");
	}

	CString s = pSBP->lpszToolTip == NULL ? _T("") : pSBP->lpszToolTip;
	return s;
}
//*******************************************************************************
void CBCGPStatusBar::SetTipText(int nIndex, LPCTSTR pszTipText)
{
	ASSERT_VALID(this);

	CBCGStatusBarPaneInfo* pSBP = _GetPanePtr(nIndex);
	if (pSBP == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	if (pSBP->lpszToolTip != NULL)
	{
		if (pszTipText != NULL && lstrcmp(pSBP->lpszToolTip, pszTipText) == 0)
		{
			return;        // nothing to change
		}

		free((LPVOID)pSBP->lpszToolTip);
	}
	else if (pszTipText == NULL || *pszTipText == '\0')
	{
		return; // nothing to change
	}

	if (pszTipText == NULL || *pszTipText == '\0')
	{
		pSBP->lpszToolTip = NULL;
	}
	else
	{
		pSBP->lpszToolTip = _tcsdup(pszTipText);
	}

	SetBarStyle (GetBarStyle() | CBRS_TOOLTIPS);
}
//*******************************************************************************
void CBCGPStatusBar::InvalidatePaneContent (int nIndex)
{
	if (GetSafeHwnd () == NULL)
	{
		return;
	}

	// invalidate the text of the pane - not including the border

	CBCGStatusBarPaneInfo* pSBP = _GetPanePtr(nIndex);
	if (pSBP == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	CRect rect = pSBP->rect;

	if (!(pSBP->nStyle & SBPS_NOBORDERS))
		rect.InflateRect(-CX_BORDER, -CY_BORDER);
	else
		rect.top -= CY_BORDER;  // base line adjustment

	InvalidateRect(rect, FALSE);
	UpdateWindow ();
}
//*********************************************************************************
void CBCGPStatusBar::EnablePaneDoubleClick (BOOL bEnable)
{
	m_bPaneDoubleClick = bEnable;
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPStatusBar implementation

CSize CBCGPStatusBar::CalcFixedLayout(BOOL, BOOL bHorz)
{
	ASSERT_VALID(this);

	// recalculate based on font height + icon height + borders
	TEXTMETRIC tm;
	{
		CClientDC dcScreen(NULL);
		HFONT hFont = GetCurrentFont ();

		HGDIOBJ hOldFont = dcScreen.SelectObject(hFont);
		VERIFY(dcScreen.GetTextMetrics(&tm));
		dcScreen.SelectObject(hOldFont);
	}

	int cyIconMax = 0;
	CBCGStatusBarPaneInfo* pSBP = (CBCGStatusBarPaneInfo*)m_pData;
	for (int i = 0; i < m_nCount; i++, pSBP++)
	{
		cyIconMax = max (cyIconMax, pSBP->cyIcon);
	}

	CRect rectSize;
	rectSize.SetRectEmpty();
	CalcInsideRect(rectSize, bHorz);    // will be negative size

	// sizeof text + 1 or 2 extra on top, 2 on bottom + borders
	return CSize(32767, max (cyIconMax, tm.tmHeight) +
		CY_BORDER * 4 - rectSize.Height());
}
//*******************************************************************************
void CBCGPStatusBar::DoPaint(CDC* pDCPaint)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDCPaint);

	CRect rectClip;
	pDCPaint->GetClipBox (rectClip);

	CRect rect;
	GetClientRect(rect);

	CBCGPMemDC memDC (*pDCPaint, this);
	CDC* pDC = &memDC.GetDC ();

	CBCGPControlBar::DoPaint(pDC);      // draw border

	HFONT hFont = GetCurrentFont ();
	HGDIOBJ hOldFont = pDC->SelectObject(hFont);

	int nOldMode = pDC->SetBkMode(TRANSPARENT);
	COLORREF crTextColor = pDC->SetTextColor(globalData.clrBtnText);
	COLORREF crBkColor = pDC->SetBkColor(globalData.clrBtnFace);

	CBCGStatusBarPaneInfo* pSBP = (CBCGStatusBarPaneInfo*)m_pData;
	for (int i = 0; i < m_nCount; i++, pSBP++)
	{
		OnDrawPane (pDC, pSBP);
	}

	pDC->SelectObject(hOldFont);

	// draw the size box in the bottom right corner
	if (!m_rectSizeBox.IsRectEmpty ())
	{
		CBCGPVisualManager::GetInstance ()->OnDrawStatusBarSizeBox (pDC, this,
			m_rectSizeBox);
	}

	pDC->SetTextColor (crTextColor);
	pDC->SetBkColor (crBkColor);
	pDC->SetBkMode (nOldMode);
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPStatusBar message handlers

BEGIN_MESSAGE_MAP(CBCGPStatusBar, CBCGPControlBar)
	//{{AFX_MSG_MAP(CBCGPStatusBar)
	ON_WM_NCHITTEST()
	ON_WM_SYSCOMMAND()
	ON_WM_SIZE()
	ON_MESSAGE(WM_SETFONT, OnSetFont)
	ON_MESSAGE(WM_GETFONT, OnGetFont)
	ON_MESSAGE(WM_SETTEXT, OnSetText)
	ON_MESSAGE(WM_GETTEXT, OnGetText)
	ON_MESSAGE(WM_GETTEXTLENGTH, OnGetTextLength)
	ON_WM_SETTINGCHANGE()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_WM_CREATE()
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_STYLECHANGED, OnStyleChanged)
END_MESSAGE_MAP()

int CBCGPStatusBar::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CBCGPControlBar::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	EnableToolTips ();
	return 0;
}
//****************************************************************************************
void CBCGPStatusBar::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	if (m_bPaneDoubleClick)
	{
		CBCGStatusBarPaneInfo* pSBP = HitTest (point);
		if (pSBP != NULL)
		{
			GetOwner()->PostMessage (WM_COMMAND, pSBP->nID);
		}
	}
	
	CBCGPControlBar::OnLButtonDblClk(nFlags, point);
}
//**********************************************************************************
void CBCGPStatusBar::OnTimer(UINT_PTR nIDEvent) 
{
	CBCGPControlBar::OnTimer(nIDEvent);

	int nIndex = CommandToIndex ((UINT)nIDEvent);
	if (nIndex < 0)
	{
		return;
	}

	CBCGStatusBarPaneInfo* pSBP = _GetPanePtr(nIndex);
	if (pSBP == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	if (++pSBP->nCurrFrame >= pSBP->nFrameCount)
	{
		pSBP->nCurrFrame = 0;
	}

	CRect rect = pSBP->rect;

	if (!(pSBP->nStyle & SBPS_NOBORDERS))
		rect.InflateRect(-CX_BORDER, -CY_BORDER);
	else
		rect.top -= CY_BORDER;  // base line adjustment

	rect.right = rect.left + pSBP->cxIcon;
	InvalidateRect(rect, FALSE);
	UpdateWindow ();

	CWnd* pMenu = CBCGPPopupMenu::GetActiveMenu ();

	if (pMenu != NULL && CWnd::FromHandlePermanent (pMenu->GetSafeHwnd ()) != NULL)
	{
		ClientToScreen (&rect);
		CBCGPPopupMenu::UpdateAllShadows (rect);
	}
}
//**********************************************************************************
BCGNcHitTestType CBCGPStatusBar::OnNcHitTest(CPoint point)
{
	BOOL bRTL = GetExStyle() & WS_EX_LAYOUTRTL;

	// hit test the size box - convert to HTCAPTION if so
	if (m_cxSizeBox != 0)
	{
		CRect rect;
		GetClientRect(rect);
		CalcInsideRect(rect, TRUE);
		int cxMax = min(m_cxSizeBox-1, rect.Height());
		rect.left = rect.right - cxMax;
		ClientToScreen(&rect);

		if (rect.PtInRect(point))
		{
			return bRTL ? HTBOTTOMLEFT : HTBOTTOMRIGHT;
		}
	}
	return CBCGPControlBar::OnNcHitTest(point);
}
//*******************************************************************************
void CBCGPStatusBar::OnSysCommand(UINT nID, LPARAM lParam)
{
	if (!m_cxSizeBox != 0 && (nID & 0xFFF0) == SC_SIZE)
	{
		CFrameWnd* pFrameWnd = BCGPGetParentFrame(this);
		if (pFrameWnd != NULL)
		{
			pFrameWnd->SendMessage(WM_SYSCOMMAND, (WPARAM)nID, lParam);
			return;
		}
	}

	CBCGPControlBar::OnSysCommand(nID, lParam);
}
//*******************************************************************************
void CBCGPStatusBar::OnSize(UINT nType, int cx, int cy)
{
	CBCGPControlBar::OnSize(nType, cx, cy);

	RecalcLayout ();

	// force repaint on resize (recalculate stretchy)
	Invalidate();
	UpdateWindow ();
}
//*******************************************************************************
LRESULT CBCGPStatusBar::OnSetFont(WPARAM wParam, LPARAM lParam)
{
	m_hFont = (HFONT)wParam;
	ASSERT(m_hFont != NULL);

	RecalcLayout ();

	if ((BOOL)lParam)
	{
		Invalidate();
		UpdateWindow ();
	}

	return 0L;      // does not re-draw or invalidate - resize parent instead
}
//*******************************************************************************
LRESULT CBCGPStatusBar::OnGetFont(WPARAM, LPARAM)
{
	HFONT hFont = GetCurrentFont ();
	return (LRESULT)(UINT_PTR)hFont;
}
//*******************************************************************************
LRESULT CBCGPStatusBar::OnSetText(WPARAM, LPARAM lParam)
{
	int nIndex = CommandToIndex(0);
	if (nIndex < 0)
		return -1;
	return SetPaneText(nIndex, (LPCTSTR)lParam) ? 0 : -1;
}
//*******************************************************************************
LRESULT CBCGPStatusBar::OnGetText(WPARAM wParam, LPARAM lParam)
{
	int nMaxLen = (int)wParam;
	if (nMaxLen == 0)
		return 0;       // nothing copied
	LPTSTR lpszDest = (LPTSTR)lParam;

	int nLen = 0;
	int nIndex = CommandToIndex(0); // use pane with ID zero
	if (nIndex >= 0)
	{
		CBCGStatusBarPaneInfo* pSBP = _GetPanePtr(nIndex);
		if (pSBP == NULL)
		{
			ASSERT (FALSE);
			return 0;
		}

		nLen = pSBP->lpszText != NULL ? lstrlen(pSBP->lpszText) : 0;
		if (nLen > nMaxLen)
			nLen = nMaxLen - 1; // number of characters to copy (less term.)
		memcpy(lpszDest, pSBP->lpszText, nLen*sizeof(TCHAR));
	}
	lpszDest[nLen] = '\0';
	return nLen+1;      // number of bytes copied
}
//*******************************************************************************
LRESULT CBCGPStatusBar::OnGetTextLength(WPARAM, LPARAM)
{
	int nLen = 0;
	int nIndex = CommandToIndex(0); // use pane with ID zero
	if (nIndex >= 0)
	{
		CBCGStatusBarPaneInfo* pSBP = _GetPanePtr(nIndex);
		if (pSBP == NULL)
		{
			ASSERT (FALSE);
			return 0;
		}

		if (pSBP->lpszText != NULL)
		{
			nLen = lstrlen(pSBP->lpszText);
		}
	}

	return nLen;
}
//*******************************************************************************
void CBCGPStatusBar::OnDrawPane (CDC* pDC, CBCGStatusBarPaneInfo* pPane)
{
	ASSERT_VALID (pDC);
	ASSERT (pPane != NULL);

	CRect rectPane = pPane->rect;
	if (rectPane.IsRectEmpty () || !pDC->RectVisible (rectPane))
	{
		return;
	}

	// Fill pane background:
	if (pPane->clrBackground != (COLORREF)-1)
	{
		CBrush brush (pPane->clrBackground);
		CBrush* pOldBrush = pDC->SelectObject(&brush);

		pDC->PatBlt (rectPane.left, rectPane.top, rectPane.Width(), rectPane.Height(), PATCOPY);

		pDC->SelectObject(pOldBrush);
	}

	// Draw pane border:
	CBCGPVisualManager::GetInstance ()->OnDrawStatusBarPaneBorder (pDC, this,
		rectPane, pPane->nID, pPane->nStyle);

	if (!(pPane->nStyle & SBPS_NOBORDERS)) // only adjust if there are borders
	{
		rectPane.DeflateRect (2 * CX_BORDER, CY_BORDER);
	}

	// Draw icon
	if (pPane->hImage != NULL && pPane->cxIcon > 0)
	{
		CRect rectIcon = rectPane;
		rectIcon.right = rectIcon.left + pPane->cxIcon;

		int x = max (0, (rectIcon.Width () - pPane->cxIcon) / 2);
		int y = max (0, (rectIcon.Height () - pPane->cyIcon) / 2);

		::ImageList_DrawEx (pPane->hImage, pPane->nCurrFrame, pDC->GetSafeHdc (),
				rectIcon.left + x, rectIcon.top + y,
				pPane->cxIcon, pPane->cyIcon, CLR_NONE, 0, ILD_NORMAL);
	}

	CRect rectText = rectPane;
	rectText.left += pPane->cxIcon;

	if (pPane->cxIcon > 0)
	{
		rectText.left += nTextMargin;
	}

	if (pPane->nProgressTotal > 0)
	{
		// Draw progress bar:
		CRect rectProgress = rectText;
		rectProgress.DeflateRect (1, 1);

		COLORREF clrBar = (pPane->clrProgressBar == (COLORREF)-1) ?
			globalData.clrHilite : pPane->clrProgressBar;

		CBCGPVisualManager::GetInstance ()->OnDrawStatusBarProgress (pDC, this,
			rectProgress, pPane->nProgressTotal, pPane->nProgressCurr,
			clrBar, pPane->clrProgressBarDest, pPane->clrProgressText,
			pPane->bProgressText);
	}
	else
	{
		// Draw text
		if (pPane->lpszText != NULL && pPane->cxText > 0)
		{
			COLORREF clrText = pDC->SetTextColor (
				CBCGPVisualManager::GetInstance ()->GetStatusBarPaneTextColor (
				this, pPane));

			pDC->DrawText (pPane->lpszText, lstrlen(pPane->lpszText), rectText,
				DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX);

			pDC->SetTextColor (clrText);
		}
	}
}
//**********************************************************************************
void CBCGPStatusBar::RecalcLayout ()
{
	ASSERT_VALID (this);
	ASSERT (GetSafeHwnd () != NULL);

	// get the drawing area for the status bar
	CRect rect;
	GetClientRect(rect);
	CalcInsideRect(rect, TRUE);

	// the size box is based off the size of a scrollbar
	m_cxSizeBox = min(GetSystemMetrics(SM_CXVSCROLL)+1, rect.Height());
	
	CFrameWnd* pFrameWnd = BCGPGetParentFrame(this);
	if (pFrameWnd->GetSafeHwnd() != NULL)
	{
		if (pFrameWnd->IsZoomed())
		{
			m_cxSizeBox = 0;
		}

		if ((pFrameWnd->GetStyle() & WS_THICKFRAME) == 0)
		{
			m_cxSizeBox = 0;
		}
	}

	if ((GetStyle() & SBARS_SIZEGRIP) == 0)
	{
		m_cxSizeBox = 0;
	}

	CClientDC dcScreen (NULL);

	int xMax = (rect.right -= m_cxSizeBox);
	if (m_cxSizeBox == 0)
		xMax += m_cxRightBorder + 1;

	// walk through to calculate extra space
	int cxExtra = rect.Width() + m_cxDefaultGap;
	CBCGStatusBarPaneInfo* pSBP = (CBCGStatusBarPaneInfo*)m_pData;
	int i = 0;

	for (i = 0; i < m_nCount; i++, pSBP++)
	{
		cxExtra -= (pSBP->cxText + pSBP->cxIcon + CX_BORDER * 4 + m_cxDefaultGap);
		
		if (pSBP->cxText > 0 && pSBP->cxIcon > 0)
		{
			cxExtra -= nTextMargin;
		}
	}
	// if cxExtra <= 0 then we will not stretch but just clip

	for (i = 0, pSBP = (CBCGStatusBarPaneInfo*)m_pData; i < m_nCount; i++, pSBP++)
	{
		ASSERT(pSBP->cxText >= 0);
		ASSERT(pSBP->cxIcon >= 0);

		if (rect.left >= xMax)
		{
			pSBP->rect = CRect (0, 0, 0, 0);
		}
		else
		{
			int cxPane = pSBP->cxText + pSBP->cxIcon;
			if (pSBP->cxText > 0 && pSBP->cxIcon > 0)
			{
				cxPane += nTextMargin;
			}

			if ((pSBP->nStyle & SBPS_STRETCH) && cxExtra > 0)
			{
				cxPane += cxExtra;
				cxExtra = 0;
			}

			rect.right = rect.left + cxPane + CX_BORDER * 4;
			rect.right = min(rect.right, xMax);

			pSBP->rect = rect;

			rect.left = rect.right + m_cxDefaultGap;
		}
	}

	if (m_cxSizeBox != 0)
	{
		int cxMax = min(m_cxSizeBox, rect.Height()+m_cyTopBorder);

		m_rectSizeBox = rect;
		m_rectSizeBox.left = rect.right;
		m_rectSizeBox.right = m_rectSizeBox.left + cxMax;
	}
	else
	{
		m_rectSizeBox.SetRectEmpty ();
	}
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPStatusBar idle update through CBCGStatusCmdUI class

class CBCGStatusCmdUI : public CCmdUI      // class private to this file!
{
public: // re-implementations only
	virtual void Enable(BOOL bOn);
	virtual void SetCheck(int nCheck);
	virtual void SetText(LPCTSTR lpszText);
};

void CBCGStatusCmdUI::Enable(BOOL bOn)
{
	m_bEnableChanged = TRUE;
	CBCGPStatusBar* pStatusBar = (CBCGPStatusBar*)m_pOther;
	ASSERT(pStatusBar != NULL);
	ASSERT_KINDOF(CBCGPStatusBar, pStatusBar);
	ASSERT(m_nIndex < m_nIndexMax);

	UINT nNewStyle = pStatusBar->GetPaneStyle(m_nIndex) & ~SBPS_DISABLED;
	if (!bOn)
		nNewStyle |= SBPS_DISABLED;
	pStatusBar->SetPaneStyle(m_nIndex, nNewStyle);
}
//*******************************************************************************
void CBCGStatusCmdUI::SetCheck(int nCheck) // "checking" will pop out the text
{
	CBCGPStatusBar* pStatusBar = (CBCGPStatusBar*)m_pOther;
	ASSERT(pStatusBar != NULL);
	ASSERT_KINDOF(CBCGPStatusBar, pStatusBar);
	ASSERT(m_nIndex < m_nIndexMax);

	UINT nNewStyle = pStatusBar->GetPaneStyle(m_nIndex) & ~SBPS_POPOUT;
	if (nCheck != 0)
		nNewStyle |= SBPS_POPOUT;
	pStatusBar->SetPaneStyle(m_nIndex, nNewStyle);
}
//*******************************************************************************
void CBCGStatusCmdUI::SetText(LPCTSTR lpszText)
{
	ASSERT(m_pOther != NULL);
	ASSERT_KINDOF(CBCGPStatusBar, m_pOther);
	ASSERT(m_nIndex < m_nIndexMax);

	((CBCGPStatusBar*)m_pOther)->SetPaneText(m_nIndex, lpszText);
}
//*******************************************************************************
void CBCGPStatusBar::OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler)
{
	CBCGStatusCmdUI state;
	state.m_pOther = this;
	state.m_nIndexMax = (UINT)m_nCount;
	for (state.m_nIndex = 0; state.m_nIndex < state.m_nIndexMax;
		state.m_nIndex++)
	{
		state.m_nID = _GetPanePtr(state.m_nIndex)->nID;
		state.DoUpdate(pTarget, bDisableIfNoHndler);
	}

	// update the dialog controls added to the status bar
	UpdateDialogControls(pTarget, bDisableIfNoHndler);
}
//*************************************************************************************
INT_PTR CBCGPStatusBar::OnToolHitTest(CPoint point, TOOLINFO* pTI) const
{
	ASSERT_VALID(this);

	// check child windows first by calling CBCGPControlBar
	INT_PTR nHit = (INT_PTR) CBCGPControlBar::OnToolHitTest(point, pTI);
	if (nHit != -1)
		return nHit;

	CBCGStatusBarPaneInfo* pSBP = HitTest (point);
	if (pSBP != NULL && pSBP->lpszToolTip != NULL)
	{
		nHit = pSBP->nID;

		if (pTI != NULL)
		{
			CString strTipText = pSBP->lpszToolTip;

			pTI->lpszText = (LPTSTR) ::calloc ((strTipText.GetLength () + 1), sizeof (TCHAR));
			lstrcpy (pTI->lpszText, strTipText);

			pTI->rect = pSBP->rect;
			pTI->uId = 0;
			pTI->hwnd = m_hWnd;
		}
	}

#if _MSC_VER >= 1300
	CToolTipCtrl* pToolTip = AfxGetModuleState()->m_thread.GetDataNA()->m_pToolTip;
#else
	_AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
	CToolTipCtrl* pToolTip = pThreadState->m_pToolTip;
#endif
	if (pToolTip != NULL && pToolTip->GetSafeHwnd () != NULL)
	{
		pToolTip->SetFont (&globalData.fontTooltip, FALSE);
	}

	return nHit;
}
//****************************************************************************************
CBCGStatusBarPaneInfo* CBCGPStatusBar::HitTest (CPoint pt) const
{
	ASSERT_VALID (this);

	for (int i = 0; i < m_nCount; i++)
	{
		CBCGStatusBarPaneInfo* pSBP = _GetPanePtr(i);
		ASSERT (pSBP != NULL);

		CRect rect = pSBP->rect;
		if (rect.PtInRect (pt))
		{
			return pSBP;
		}
	}

	return NULL;
}
//****************************************************************************************
long CBCGPStatusBar::GetPaneProgress (int nIndex) const
{
	ASSERT_VALID(this);

	CBCGStatusBarPaneInfo* pSBP = _GetPanePtr(nIndex);
	if (pSBP == NULL)
	{
		ASSERT (FALSE);
		return -1;
	}

	return pSBP->nProgressCurr;
}
//**************************************************************************************
HFONT CBCGPStatusBar::GetCurrentFont () const
{
	return m_hFont == NULL ? 
		(HFONT) globalData.fontRegular.GetSafeHandle () : 
		m_hFont;
}
//***************************************************************************************
LRESULT CBCGPStatusBar::OnStyleChanged(WPARAM wp, LPARAM lp)
{
	int nStyleType = (int) wp;
	LPSTYLESTRUCT lpStyleStruct = (LPSTYLESTRUCT) lp;

	CBCGPControlBar::OnStyleChanged (nStyleType, lpStyleStruct);

	if ((lpStyleStruct->styleNew & SBARS_SIZEGRIP) &&
		(lpStyleStruct->styleOld & SBARS_SIZEGRIP) == 0)
	{
		RecalcLayout ();
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPStatusBar diagnostics

#ifdef _DEBUG
void CBCGPStatusBar::AssertValid() const
{
	CBCGPControlBar::AssertValid();
}
//********************************************************************************
void CBCGPStatusBar::Dump(CDumpContext& dc) const
{
	CBCGPControlBar::Dump(dc);

	dc << "\nm_hFont = " << (UINT_PTR)m_hFont;

	if (dc.GetDepth() > 0)
	{
		for (int i = 0; i < m_nCount; i++)
		{
			dc << "\nstatus pane[" << i << "] = {";
			dc << "\n\tnID = " << _GetPanePtr(i)->nID;
			dc << "\n\tnStyle = " << _GetPanePtr(i)->nStyle;
			dc << "\n\tcxText = " << _GetPanePtr(i)->cxText;
			dc << "\n\tcxIcon = " << _GetPanePtr(i)->cxIcon;
			dc << "\n\tlpszText = " << _GetPanePtr(i)->lpszText;
			dc << "\n\t}";
		}
	}

	dc << "\n";
}
#endif //_DEBUG

#undef new
#ifdef AFX_INIT_SEG
#pragma code_seg(AFX_INIT_SEG)
#endif

IMPLEMENT_DYNAMIC(CBCGPStatusBar, CBCGPControlBar)

BOOL CBCGPStatusBar::GetExtendedArea (CRect& rect) const
{
	ASSERT_VALID (this);

	if (!m_bDrawExtendedArea)
	{
		return FALSE;
	}

	CRect rectClient;
	GetClientRect (rectClient);

	for (int i = m_nCount - 1; i >= 0; i--)
	{
		CBCGStatusBarPaneInfo* pSBP = _GetPanePtr (i);
		ASSERT (pSBP != NULL);

		if (pSBP->nStyle & SBPS_STRETCH)
		{
			rect = rectClient;
			rect.left = pSBP->rect.right;

			return TRUE;
		}
	}

	return FALSE;
}
//*********************************************************************
void CBCGPStatusBar::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CBCGPControlBar::OnShowWindow(bShow, nStatus);
	
	if (CBCGPVisualManager::GetInstance ()->IsStatusBarCoversFrame() && GetParentFrame () != NULL)
	{
		GetParentFrame ()->PostMessage (BCGM_CHANGEVISUALMANAGER);
	}
}
