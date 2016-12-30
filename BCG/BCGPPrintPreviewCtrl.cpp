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
//

#include "stdafx.h"
#include "bcgprores.h"
#include "BCGPPrintPreviewCtrl.h"
#include "BCGGlobals.h"
#include "BCGPPopupMenu.h"
#include "BCGPDlgImpl.h"
#include "BCGPMath.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CBCGPPrintPreviewCtrl, CBCGPWnd)

class BCGPView: public CView
{
	friend class CBCGPPrintPreviewCtrl;
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPPrintPreviewCtrl

CBCGPPrintPreviewCtrl::CBCGPPrintPreviewCtrl()
	: m_pPreviewInfo   (NULL)
	, m_pPrintView     (NULL)
	, m_bPreviewInfo   (FALSE)
	, m_pPreviewDC     (NULL)
	, m_rectThumbnail  (0, 0, 0, 0)
	, m_sizePrinter    (0, 0)
	, m_sizePrinterPPI (0, 0)
	, m_nCurrentPage   (1)
	, m_nOffsetPage    (0)
	, m_ptOffsetPage   (0, 0)
	, m_sizeGrid       (1, 1)
	, m_sizeGridMax    (0, 0)
	, m_nZoomType      (0)
	, m_dZoom          (0.0)
	, m_dScalePPIX     (1.0)
	, m_dScalePPIY     (1.0)
	, m_margPreview    (10, 10)
	, m_margThumbnail  (6, 6)
	, m_margThumbnailFrame(0, 0, 0, 0)
	, m_bFastPrint     (TRUE)
	, m_bInvalidate    (FALSE)
	, m_nNotifyZoom    (0)
	, m_bNotifyZoomPost(FALSE)
	, m_nNotifyPage    (0)
	, m_bNotifyPagePost(FALSE)
	, m_bScrollPage    (FALSE)
	, m_bInsideUpdate  (FALSE)
	, m_bDrawFocus     (TRUE)
	, m_bBackstageMode (FALSE)
{
	m_bScrollBars[0] = FALSE;
	m_bScrollBars[1] = FALSE;
}

CBCGPPrintPreviewCtrl::~CBCGPPrintPreviewCtrl()
{
	m_dcPrint.Detach();

	if (m_pPreviewDC != NULL)
	{
		delete m_pPreviewDC;
	}

	if (m_bPreviewInfo && m_pPreviewInfo != NULL)
	{
		delete m_pPreviewInfo;
	}
}

BOOL CBCGPPrintPreviewCtrl::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	dwStyle |= WS_CHILD;
	return CreateEx (0, dwStyle, rect, pParentWnd, nID);
}

BOOL CBCGPPrintPreviewCtrl::CreateEx(DWORD dwExStyle, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, LPVOID lpParam/* = NULL*/)
{
	dwStyle |= WS_CHILD;
	if ((dwStyle & WS_BORDER) == WS_BORDER)
	{
		dwStyle &= ~WS_BORDER;
		dwExStyle |= WS_EX_CLIENTEDGE;
	}

	return CWnd::CreateEx (dwExStyle, globalData.RegisterWindowClass (_T("BCGPPrintPreviewCtrl")), 
			_T(""), dwStyle, rect, pParentWnd, nID, lpParam);
}

CPrintInfo* CBCGPPrintPreviewCtrl::CreatePrintInfo() const
{
	CPrintInfo* pPrintInfo = new CPrintInfo;
	pPrintInfo->m_pPD->SetHelpID(AFX_IDD_PRINTSETUP);
	pPrintInfo->m_pPD->m_pd.Flags |= PD_PRINTSETUP;
	pPrintInfo->m_pPD->m_pd.Flags &= ~PD_RETURNDC;

	return pPrintInfo;
}

BOOL CBCGPPrintPreviewCtrl::ChangeSettings()
{
	if (m_pPrintView == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	if (m_pPreviewInfo == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	((BCGPView*)m_pPrintView)->OnEndPrinting (m_pPreviewDC, m_pPreviewInfo);

	m_dcPrint.Detach ();

	if (m_pPreviewInfo->m_pPD != NULL && m_pPreviewInfo->m_pPD->m_pd.hDC != NULL)
	{
		::DeleteDC(m_pPreviewInfo->m_pPD->m_pd.hDC);
		m_pPreviewInfo->m_pPD->m_pd.hDC = NULL;
	}

	if (m_pPreviewDC != NULL)
	{
		delete m_pPreviewDC;
		m_pPreviewDC = NULL;
	}

	m_pPreviewInfo->SetMinPage(1);
	m_pPreviewInfo->SetMaxPage(0xffff);
	m_pPreviewInfo->m_nCurPage = m_nCurrentPage;

	m_ptOffsetPage = CPoint(0, 0);

	if (!PrepareSettings())
	{
		return FALSE;
	}

	m_bInvalidate = TRUE;
	Invalidate();

	return TRUE;
}

BOOL CBCGPPrintPreviewCtrl::PrepareSettings()
{
	if (m_pPrintView == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	if (m_pPreviewInfo == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	m_pPreviewDC = new CPreviewDC;

	if (!((BCGPView*)m_pPrintView)->OnPreparePrinting(m_pPreviewInfo))
	{
		return FALSE;
	}

#ifdef _DEBUG
	if (m_pPreviewInfo->m_pPD->m_pd.hDC == NULL)
	{
		TRACE0("Error: hDC not set for printing --\n");
		TRACE0("\tDid you remember to call DoPreparePrinting?\n");
		ASSERT(FALSE);      // common mistake gets trapped here
	}
#endif //_DEBUG

	m_dcPrint.Attach(m_pPreviewInfo->m_pPD->m_pd.hDC);

	VERIFY(m_dcPrint.Escape(GETPHYSPAGESIZE, 0, NULL, (LPVOID)&m_sizePrinter));
	m_sizePrinterPPI.cx = m_dcPrint.GetDeviceCaps(LOGPIXELSX);
	m_sizePrinterPPI.cy = m_dcPrint.GetDeviceCaps(LOGPIXELSY);

	if (!CalculateScale())
	{
		m_dScalePPIX = (double)m_sizePrinterPPI.cx / (double)afxData.cxPixelsPerInch;
		m_dScalePPIY = (double)m_sizePrinterPPI.cy / (double)afxData.cyPixelsPerInch;
	}

	ASSERT(m_dScalePPIX > 0.0);
	ASSERT(m_dScalePPIY > 0.0);

	m_pPreviewDC->SetAttribDC(m_pPreviewInfo->m_pPD->m_pd.hDC);
	m_pPreviewDC->SetTopLeftOffset(CSize(0, 0));
	m_pPreviewDC->m_bPrinting = TRUE;
	m_dcPrint.m_bPrinting = TRUE;

	m_dcPrint.SaveDC();

	CDC* pDC = GetDC ();
	m_pPreviewDC->SetOutputDC(pDC->GetSafeHdc ());
	((BCGPView*)m_pPrintView)->OnBeginPrinting(m_pPreviewDC, m_pPreviewInfo);
	m_pPreviewDC->ReleaseOutputDC();
	ReleaseDC(pDC);

	m_dcPrint.RestoreDC(-1);

	RecalcLayout (FALSE);

	m_nCurrentPage = m_pPreviewInfo->m_nCurPage;

	if ((m_pPreviewInfo->GetMaxPage() - m_pPreviewInfo->GetMinPage()) > 32767U)
	{
		CSize sizePage((int)(m_sizePrinter.cx / m_dScalePPIX / 10.0), (int)(m_sizePrinter.cy / m_dScalePPIY / 10.0));
		CRect rectPage(CPoint(0, 0), sizePage);

		CDC* pDC = GetDC ();
		CDC pageDC;
		pageDC.CreateCompatibleDC (pDC);
		pageDC.m_bPrinting = TRUE;
		CBitmap pageBmp;
		pageBmp.CreateCompatibleBitmap (pDC, sizePage.cx, sizePage.cy);
		pageDC.SelectObject (&pageBmp);
		ReleaseDC(pDC);

		UINT nPage = m_pPreviewInfo->GetMinPage ();
		m_pPreviewInfo->m_bContinuePrinting = TRUE;
		while (DoDrawPage(&pageDC, rectPage, nPage))
		{
			nPage++;
		}
	}

	UpdateScrollBars (TRUE);

	m_pPreviewInfo->m_nCurPage = m_nCurrentPage;
	SetCurrentPage(m_pPreviewInfo->m_nCurPage, FALSE);

	return TRUE;
}

BOOL CBCGPPrintPreviewCtrl::CalculateScale()
{
	return FALSE;
}

BOOL CBCGPPrintPreviewCtrl::SetPrintView(CView* pPrintView, CPrintInfo* pPrintInfo)
{
	ASSERT_VALID(pPrintView);
	if (pPrintView == NULL)
	{
		return FALSE;
	}

	m_pPrintView = pPrintView;

	// allocate preview info
	m_pPreviewInfo = pPrintInfo;
	if (m_pPreviewInfo == NULL)
	{
		m_pPreviewInfo = CreatePrintInfo ();
		m_bPreviewInfo = TRUE;
	}

	m_pPreviewInfo->m_bPreview = TRUE;  // signal that this is preview
	ASSERT(m_pPreviewInfo->m_pPD != NULL);

	m_pPreviewInfo->SetMinPage(1);
	m_pPreviewInfo->SetMaxPage(0xffff);

	return PrepareSettings();
}

CSize CBCGPPrintPreviewCtrl::GetThumbnailMargins() const
{
	return m_margThumbnail;
}

CRect CBCGPPrintPreviewCtrl::GetThumbnailFrameMargins() const
{
	return m_margThumbnailFrame;
}

CSize CBCGPPrintPreviewCtrl::GetPreviewMargins() const
{
	return m_margPreview;
}

void CBCGPPrintPreviewCtrl::OnPreviewClose()
{
	if (m_pPrintView != NULL)
	{
		((BCGPView*)m_pPrintView)->OnEndPrinting(m_pPreviewDC, m_pPreviewInfo);
	}
}

BEGIN_MESSAGE_MAP(CBCGPPrintPreviewCtrl, CBCGPWnd)
	//{{AFX_MSG_MAP(CBCGPPrintPreviewCtrl)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_MOUSEWHEEL()
	ON_WM_MOUSEACTIVATE()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_PRINTCLIENT, OnPrintClient)
	ON_REGISTERED_MESSAGE(BCGM_ONSETCONTROLBACKSTAGEMODE, OnBCGSetControlBackStageMode)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CBCGPPrintPreviewCtrl message handlers

int CBCGPPrintPreviewCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CBCGPWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	m_ctrlFrame.Create(CBCGPControlRendererParams(IDB_BCGBARRES_PRINT_FRAME, CRect(0, 0, 42, 41), CRect(21, 20, 20, 20), CRect(10, 7, 9, 12)));
	m_margThumbnailFrame = m_ctrlFrame.GetParams ().m_rectSides;
	
	return 0;
}

BOOL CBCGPPrintPreviewCtrl::OnEraseBkgnd(CDC* /*pDC*/) 
{
	return FALSE;
}

void CBCGPPrintPreviewCtrl::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	if (!m_bInsideUpdate)
	{
		DoDraw(&dc);
	}
}

void CBCGPPrintPreviewCtrl::PrepareBackDC()
{
    ASSERT(GetSafeHwnd() != NULL);

	m_bInvalidate = FALSE;

    CRect rtClient;
    GetClientRect(rtClient);

    CSize sz(rtClient.Size());

	CDC* pDC = GetDC ();

    if(m_backDC.GetSafeHdc() == NULL)
    {
        m_backDC.CreateCompatibleDC(pDC);
    }

    CSize bmpSize(0, 0);
    if(m_backBmp.GetSafeHandle() != NULL)
    {
        BITMAP bmp;
        if (m_backBmp.GetBitmap(&bmp) != 0)
        {
            bmpSize = CSize(bmp.bmWidth, bmp.bmHeight);
        }
    }

    if (bmpSize != sz)
    {
        m_backBmp.DeleteObject();

        if(sz.cx != 0 && sz.cy != 0)
        {
            m_backBmp.CreateCompatibleBitmap(pDC, sz.cx, sz.cy);
            m_backDC.SelectObject(&m_backBmp);

			m_bInvalidate = TRUE;
        }
    }

	ReleaseDC (pDC);
}

void CBCGPPrintPreviewCtrl::OnSize(UINT nType, int cx, int cy) 
{
	CBCGPWnd::OnSize(nType, cx, cy);

	if (cx > 0 && cy > 0)
	{
		PrepareBackDC ();
		RecalcLayout();
	}
}

void CBCGPPrintPreviewCtrl::OnDestroy() 
{
	CBCGPWnd::OnDestroy();
	
	OnPreviewClose();	
}

void CBCGPPrintPreviewCtrl::OnLButtonDown(UINT /*nFlags*/, CPoint point) 
{
	UINT nPage = GetPageFromPoint (point);
	if (nPage == 0)
	{
		return;
	}

	SetCurrentPage (nPage);
}

void CBCGPPrintPreviewCtrl::OnLButtonDblClk(UINT /*nFlags*/, CPoint point) 
{
	if (m_nZoomType == 0)
	{
		return;
	}

	UINT nPage = GetPageFromPoint (point);
	if (nPage == 0)
	{
		return;
	}

	SetCurrentPage (nPage, FALSE);
	SetZoomType (0);
}

void CBCGPPrintPreviewCtrl::SetCurrentPage (UINT nPage, BOOL bRedraw)
{
	UINT nOldPage = m_nCurrentPage;
	nPage = max(min(nPage, m_pPreviewInfo->GetMaxPage()), m_pPreviewInfo->GetMinPage());

	m_nCurrentPage = nPage;

	LONG nStartPage = GetStartPage ();
	while ((LONG)m_nCurrentPage < nStartPage)
	{
		nStartPage -= m_sizeGrid.cx;
	}
	while ((nStartPage + m_sizeGrid.cx * m_sizeGrid.cy) <= (LONG)m_nCurrentPage)
	{
		nStartPage += m_sizeGrid.cx;
	}

	if ((UINT)nStartPage != GetStartPage ())
	{
		m_nOffsetPage = (nStartPage - m_pPreviewInfo->GetMinPage()) / m_sizeGrid.cx;
		UpdateScrollBars ();
	}

	if (m_nNotifyPage > 0 && nOldPage != m_nCurrentPage)
	{
		OnPageChanged ();
	}

	if (bRedraw)
	{
		m_bInvalidate = TRUE;
		Invalidate ();
	}
}

void CBCGPPrintPreviewCtrl::SetZoomType (int nZoomType, BOOL bRedraw)
{
	nZoomType = max(min(nZoomType, 100), 0);
	if (m_nZoomType == nZoomType)
	{
		return;
	}

	UINT nNotifyZoom = m_nNotifyZoom;
	m_nNotifyZoom = (nZoomType < 10) ? m_nNotifyZoom : 0;

	m_nZoomType = nZoomType;
	RecalcLayout ();

	m_nNotifyZoom = nNotifyZoom;

	SetCurrentPage (m_nCurrentPage, bRedraw);
}

UINT CBCGPPrintPreviewCtrl::GetPageFromPoint (const CPoint& point) const
{
	UINT nIndex = 0;

	CSize szMargins(GetThumbnailMargins ());
	CRect rectThumbnail(m_rectThumbnail);

	UINT nPage = GetStartPage ();

	for (int y = 0; y < m_sizeGrid.cy; y++)
	{
		rectThumbnail.left = m_rectThumbnail.left;
		rectThumbnail.right = m_rectThumbnail.right;
		for (int x = 0; x < m_sizeGrid.cx; x++)
		{
			if (rectThumbnail.PtInRect (point))
			{
				nIndex = nPage;
				break;
			}

			nPage++;
			rectThumbnail.OffsetRect (m_rectThumbnail.Width() + szMargins.cx, 0);
		}

		if (nIndex > 0)
		{
			break;
		}

		rectThumbnail.OffsetRect (0, m_rectThumbnail.Height() + szMargins.cy);
	}

	if (nIndex > m_pPreviewInfo->GetMaxPage())
	{
		nIndex = 0;
	}

	return nIndex;
}

UINT CBCGPPrintPreviewCtrl::GetStartPage () const
{
	return m_nOffsetPage * m_sizeGrid.cx + m_pPreviewInfo->GetMinPage();
}

void CBCGPPrintPreviewCtrl::DoDraw(CDC* pDC)
{
	ASSERT_VALID(pDC);

	if (m_pPrintView == NULL || m_dcPrint.m_hDC == NULL)
	{
		return;
	}

	CDC* pStoredDC = NULL;
	CRect rtClient;
	GetClientRect(rtClient);

  	if (m_backDC.GetSafeHdc () != NULL)
  	{
  		pStoredDC = pDC;
  		pDC = &m_backDC;
  	}

	UINT nMaxPage = m_pPreviewInfo->GetMaxPage();

	if (m_bInvalidate)
	{
		DoDrawBackground (pDC, rtClient);

		if (m_pageBmp.GetSafeHandle () != NULL)
		{
			int nStretchMode = pDC->SetStretchBltMode(HALFTONE);

			CSize sizePage((int)(m_sizePrinter.cx / m_dScalePPIX), (int)(m_sizePrinter.cy / m_dScalePPIY));
			if (m_bFastPrint && (m_rectThumbnail.Width() < sizePage.cx && m_rectThumbnail.Height() < sizePage.cy))
			{
				sizePage = m_rectThumbnail.Size();
			}
			CRect rectPage(CPoint(0, 0), sizePage);

			m_pPreviewInfo->m_bContinuePrinting = TRUE;

			CSize szMargins(GetThumbnailMargins ());
			CRect rtFrameMargins(GetThumbnailFrameMargins());
			CRect rectThumbnail(m_rectThumbnail);

			BOOL bMultiPages = (m_sizeGrid.cx * m_sizeGrid.cy) > 1;
			UINT nPage = GetStartPage ();
			BOOL bContinue = TRUE;

			if (bMultiPages)
			{
				CRgn rgn;
				rgn.CreateRectRgnIndirect (rtClient);
				pDC->SelectClipRgn (&rgn);
			}

			for (int y = 0; y < m_sizeGrid.cy; y++)
			{
				rectThumbnail.left = m_rectThumbnail.left;
				rectThumbnail.right = m_rectThumbnail.right;
				for (int x = 0; x < m_sizeGrid.cx; x++)
				{
					if (m_bScrollPage)
					{
						rectThumbnail.OffsetRect (-m_ptOffsetPage);
					}

					CRect rectFrame(rectThumbnail);
					rectFrame.InflateRect (rtFrameMargins);

					m_ctrlFrame.DrawFrame(pDC, rectFrame, (nPage == m_nCurrentPage) ? 1 : 0);

					bContinue = DoDrawPage(&m_pageDC, rectPage, nPage);
					if (bContinue)
					{
						DoDrawThumbnail (pDC, rectThumbnail, &m_pageDC, rectPage);

						nPage++;
						bContinue = nPage <= m_pPreviewInfo->GetMaxPage();
					}

					if (!bContinue)
					{
						break;
					}

					if (bMultiPages)
					{
						CRgn rgnThumbnail;
						rgnThumbnail.CreateRectRgnIndirect (rectThumbnail);
						pDC->SelectClipRgn (&rgnThumbnail, RGN_DIFF);
					}

					rectThumbnail.OffsetRect (m_rectThumbnail.Width() + szMargins.cx, 0);
				}

				if (!bContinue)
				{
					break;
				}

				rectThumbnail.OffsetRect (0, m_rectThumbnail.Height() + szMargins.cy);
			}

			pDC->SelectClipRgn (NULL);
			pDC->SetStretchBltMode(nStretchMode);
		}

		m_bInvalidate = FALSE;
	}

	if (nMaxPage != m_pPreviewInfo->GetMaxPage())
	{
		UpdateScrollBars ();
		OnPageChanged ();
	}

	if (pStoredDC != NULL)
	{
		pDC = pStoredDC;
		pDC->BitBlt(0, 0, rtClient.Width(), rtClient.Height(), &m_backDC, 0, 0, SRCCOPY);
	}

	if (m_bDrawFocus && GetFocus () == this)
	{
		pDC->DrawFocusRect (rtClient);
	}
}

void CBCGPPrintPreviewCtrl::DoDrawBackground(CDC* pDC, const CRect& rect)
{
	ASSERT_VALID(pDC);

	if (m_bBackstageMode)
	{
		CBCGPVisualManager::GetInstance()->OnFillRibbonBackstageForm(pDC, GetParent(), rect);
	}
	else if (m_bVisualManagerStyle)
	{
		CBrush& br = CBCGPVisualManager::GetInstance ()->GetDlgBackBrush (GetParent());
		pDC->FillRect(rect, &br);
	}
	else
	{
		pDC->FillRect(rect, &globalData.brBtnFace);
	}
}

BOOL CBCGPPrintPreviewCtrl::DoDrawPage(CDC* pPageDC, const CRect& rectPage, UINT nPage)
{
	ASSERT_VALID(pPageDC);

	int nSavedState = m_dcPrint.SaveDC();       // Save pristine state of DC

	// Use paint DC for print preview output
	m_pPreviewDC->SetOutputDC(pPageDC->GetSafeHdc());

	m_pPreviewInfo->m_nCurPage = nPage;

	// Only call PrepareDC if within page range, otherwise use default
	// rect to draw page rectangle
	if (nPage <= m_pPreviewInfo->GetMaxPage())
	{
		m_pPrintView->OnPrepareDC(m_pPreviewDC, m_pPreviewInfo);
	}

	// Set up drawing rect to entire page (in logical coordinates)
	m_pPreviewInfo->m_rectDraw.SetRect(0, 0,
		m_pPreviewDC->GetDeviceCaps(HORZRES),
		m_pPreviewDC->GetDeviceCaps(VERTRES));
	m_pPreviewDC->DPtoLP(&m_pPreviewInfo->m_rectDraw);

	// Draw empty page on screen

	pPageDC->SaveDC();          // save the output dc state

	pPageDC->SetMapMode(MM_TEXT);   // Page Rectangle is in screen device coords
	pPageDC->SetViewportOrg(CPoint (0, 0));
	pPageDC->SetWindowOrg(0, 0);

	::FillRect(pPageDC->GetSafeHdc (), rectPage, (HBRUSH)GetStockObject(WHITE_BRUSH));

	pPageDC->RestoreDC(-1);     // restore to synchronized state

	BOOL bContinue = FALSE;
	if (m_pPreviewInfo->m_bContinuePrinting && (nPage <= m_pPreviewInfo->GetMaxPage()))
	{
		if (m_bFastPrint)
		{
			m_pPreviewDC->SetScaleRatio(bcg_round(rectPage.Width() * m_dScalePPIX), 
						bcg_round(m_sizePrinter.cx * m_dScalePPIX * (double)afxData.cxPixelsPerInch / (double)m_sizePrinterPPI.cx));
		}

		CSize PrintOffset;
		VERIFY(m_pPreviewDC->Escape(GETPRINTINGOFFSET, 0, NULL, (LPVOID)&PrintOffset));
		m_pPreviewDC->PrinterDPtoScreenDP((LPPOINT)&PrintOffset);

		m_pPreviewDC->SetTopLeftOffset(PrintOffset);

		m_pPreviewDC->ClipToPage();
		((BCGPView*)m_pPrintView)->OnPrint(m_pPreviewDC, m_pPreviewInfo);

		bContinue = TRUE;
	}

	m_pPreviewDC->ReleaseOutputDC();
	m_dcPrint.RestoreDC(nSavedState);   // restore to untouched state

	return bContinue;
}

void CBCGPPrintPreviewCtrl::DoDrawThumbnail(CDC* pDC, const CRect& rectThumbnail, CDC* pPageDC, const CRect& rectPage)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pPageDC);

	pDC->StretchBlt (rectThumbnail.left, rectThumbnail.top, rectThumbnail.Width(), rectThumbnail.Height(),
					pPageDC, rectPage.left, rectPage.top, rectPage.Width (), rectPage.Height(), SRCCOPY);
}

void CBCGPPrintPreviewCtrl::RecalcLayout(BOOL bUpdateScrollBars/* = TRUE*/)
{
	if (m_sizePrinter.cx == 0 || m_sizePrinter.cy == 0)
	{
		return;
	}

	if (m_bInsideUpdate)
	{
		return;
	}

	m_bInsideUpdate = TRUE;

	CRect rtClient;
	GetClientRect(rtClient);

	rtClient.DeflateRect (GetPreviewMargins ());

	CSize szMargins(GetThumbnailMargins ());
	rtClient.DeflateRect (szMargins);

	double dZoom = m_dZoom;

	m_rectThumbnail.SetRect(0, 0, 0, 0);
	m_dZoom = 1.0;
	m_sizeGrid = CSize(1, 1);

	BOOL bScrollBars[2] = {FALSE, FALSE};

	BOOL bCanScrollPaqes = m_pPreviewInfo->GetMaxPage() < 0x8000 && 
						(m_pPreviewInfo->GetMaxPage() - m_pPreviewInfo->GetMinPage()) <= 32767U &&
						m_pPreviewInfo->GetMaxPage() != m_pPreviewInfo->GetMinPage();

	if (m_bScrollBars[0])
	{
		rtClient.bottom += afxData.cyHScroll;
	}
	if (m_bScrollBars[1])
	{
		rtClient.right += afxData.cxVScroll;
	}

	if (m_nZoomType == 0) // fit width and height
	{
		if (bCanScrollPaqes)
		{
			bScrollBars[1] = TRUE;
			if (m_bScrollBars[1])
			{
				rtClient.right -= afxData.cxVScroll;
			}
		}

		m_dZoom = min((double)rtClient.Width() / (double)m_sizePrinter.cx * m_dScalePPIX, 
					(double)rtClient.Height() / (double)m_sizePrinter.cy * m_dScalePPIY);
	}
	else if (m_nZoomType == 1) // fit width
	{
		if (bCanScrollPaqes)
		{
			bScrollBars[1] = TRUE;
			if (m_bScrollBars[1])
			{
				rtClient.right -= afxData.cxVScroll;
			}
		}

		m_dZoom = (double)rtClient.Width() / (double)m_sizePrinter.cx * m_dScalePPIX;
		if ((m_dZoom * (double)m_sizePrinter.cy * m_dScalePPIY) > rtClient.Height ())
		{
			bScrollBars[1] = TRUE;
			rtClient.right -= afxData.cxVScroll;
			m_dZoom = (double)rtClient.Width() / (double)m_sizePrinter.cx * m_dScalePPIX;
		}
	}
	else if (m_nZoomType == 2) // fit height
	{
		m_dZoom = (double)rtClient.Height() / (double)m_sizePrinter.cy * m_dScalePPIY;
		if ((m_dZoom * (double)m_sizePrinter.cx * m_dScalePPIX) > rtClient.Width () || bCanScrollPaqes)
		{
			bScrollBars[0] = TRUE;
			rtClient.bottom -= afxData.cyHScroll;
			m_dZoom = (double)rtClient.Height() / (double)m_sizePrinter.cy * m_dScalePPIY;
		}
	}
	else
	{
		m_dZoom = m_nZoomType / 100.0;
	}

	m_dZoom = min((int)(m_dZoom * 100.0) / 100.0, 1.0);

	m_rectThumbnail.right = (int)(m_sizePrinter.cx * m_dZoom / m_dScalePPIX);
	m_rectThumbnail.bottom = (int)(m_sizePrinter.cy * m_dZoom / m_dScalePPIY);

	CPoint ptOffset(0, 0);
	int nNumPage = 1;

	if (m_nZoomType >= 10)
	{
		if (bCanScrollPaqes)
		{
			bScrollBars[1] = TRUE;
			rtClient.right -= afxData.cxVScroll;
		}

		m_sizeGrid.cx = max(rtClient.Width() / (m_rectThumbnail.Width() + szMargins.cx), 1);
		m_sizeGrid.cy = max(rtClient.Height() / (m_rectThumbnail.Height() + szMargins.cy), 1);
		if (m_sizeGridMax.cx > 0)
		{
			m_sizeGrid.cx = min(m_sizeGrid.cx, m_sizeGridMax.cx);
		}
		if (m_sizeGridMax.cy > 0)
		{
			m_sizeGrid.cy = min(m_sizeGrid.cy, m_sizeGridMax.cy);
		}

		nNumPage = m_sizeGrid.cx * m_sizeGrid.cy;

		if (nNumPage > 1)
		{
			ptOffset.x = (rtClient.Width() - (m_rectThumbnail.Width() * m_sizeGrid.cx + szMargins.cx * (m_sizeGrid.cx - 1))) / 2;
			ptOffset.y = (rtClient.Height() - (m_rectThumbnail.Height() * m_sizeGrid.cy + szMargins.cy * (m_sizeGrid.cy - 1))) / 2;
		}
		else
		{
			if (rtClient.Width() < m_rectThumbnail.Width())
			{
				bScrollBars[0] = TRUE;
				rtClient.bottom -= afxData.cyHScroll;

				if (bCanScrollPaqes)
				{
					bCanScrollPaqes = FALSE;
					bScrollBars[1] = FALSE;
					rtClient.right += afxData.cxVScroll;
				}
			}

			if (rtClient.Height() < m_rectThumbnail.Height() && !bCanScrollPaqes)
			{
				bScrollBars[1] = TRUE;
				rtClient.right -= afxData.cxVScroll;
			}
		}
	}

	if (nNumPage == 1)
	{
		ptOffset.x = (rtClient.Width() - m_rectThumbnail.Width()) / 2;
		ptOffset.y = (rtClient.Height() - m_rectThumbnail.Height()) / 2;
	}

	ptOffset.x = max(ptOffset.x, 0) + rtClient.left;
	ptOffset.y = max(ptOffset.y, 0) + rtClient.top;

	m_rectThumbnail.OffsetRect (ptOffset);

	if (m_rectThumbnail.Width () > 0 && m_rectThumbnail.Height () > 0)
	{
		m_pageBmp.DeleteObject ();
		m_pageDC.DeleteDC ();

		CSize sizePage((int)(m_sizePrinter.cx / m_dScalePPIX), (int)(m_sizePrinter.cy / m_dScalePPIY));
		if (m_bFastPrint && (m_rectThumbnail.Width() < sizePage.cx && m_rectThumbnail.Height() < sizePage.cy))
		{
			sizePage = m_rectThumbnail.Size();
		}

		CDC* pDC = GetDC ();
		m_pageDC.CreateCompatibleDC (pDC);
		m_pageDC.m_bPrinting = TRUE;
		m_pageBmp.CreateCompatibleBitmap (pDC, sizePage.cx, sizePage.cy);
		m_pageDC.SelectObject (&m_pageBmp);
		ReleaseDC(pDC);
	}

	m_bScrollBars[0] = bScrollBars[0];
	m_bScrollBars[1] = bScrollBars[1];

	if (bUpdateScrollBars)
	{
		UpdateScrollBars();
	}

	if (dZoom != m_dZoom && m_nNotifyZoom > 0)
	{
		OnZoomChanged();
	}

	m_bInsideUpdate = FALSE;
}

void CBCGPPrintPreviewCtrl::UpdateScrollBars(BOOL bRedraw/* = TRUE*/)
{
	CRect rtClient;
	GetClientRect(rtClient);

	rtClient.DeflateRect (GetPreviewMargins ());

	CSize szMargins(GetThumbnailMargins ());
	rtClient.DeflateRect (szMargins);

	UINT nPages = m_sizeGrid.cx * m_sizeGrid.cy;
	m_bScrollPage = (rtClient.Width () < m_rectThumbnail.Width () || rtClient.Height () < m_rectThumbnail.Height ()) && nPages == 1;

	if (!m_bScrollPage && m_bScrollBars[1])
	{
		m_bScrollBars[1] = (nPages < (m_pPreviewInfo->GetMaxPage() - m_pPreviewInfo->GetMinPage() + 1)) &&
			(m_pPreviewInfo->GetMaxPage() < 0x8000) && 
			((m_pPreviewInfo->GetMaxPage() - m_pPreviewInfo->GetMinPage()) <= 32767U);
	}

	ShowScrollBar (SB_HORZ, m_bScrollBars[0]);
	ShowScrollBar (SB_VERT, m_bScrollBars[1]);

	if (m_bScrollPage)
	{
		m_nOffsetPage = m_nCurrentPage - m_pPreviewInfo->GetMinPage();

		if (m_bScrollBars[0])
		{
			int nMax = m_rectThumbnail.Width ();
			int nPage = rtClient.Width ();
			m_ptOffsetPage.x = min(m_ptOffsetPage.x, max(nMax - nPage, 0));

			SCROLLINFO info;
			info.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
			info.nMin = 0;
			info.nMax = nMax - 1;
			info.nPage = nPage;
			info.nPos = m_ptOffsetPage.x;
			SetScrollInfo(SB_HORZ, &info, bRedraw);
		}
		else
		{
			m_ptOffsetPage.x = 0;
		}

		if (m_bScrollBars[1])
		{
			int nMax = m_rectThumbnail.Height ();
			int nPage = rtClient.Height ();
			m_ptOffsetPage.y = min(m_ptOffsetPage.y, max(nMax - nPage, 0));

			SCROLLINFO info;
			info.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
			info.nMin = 0;
			info.nMax = nMax - 1;
			info.nPage = nPage;
			info.nPos = m_ptOffsetPage.y;
			SetScrollInfo(SB_VERT, &info, bRedraw);
		}
		else
		{
			m_ptOffsetPage.y = 0;
		}
	}
	else
	{
		m_ptOffsetPage = CPoint(0, 0);

		if (m_bScrollBars[1])
		{
			nPages = m_pPreviewInfo->GetMaxPage() - m_pPreviewInfo->GetMinPage() + 1;
			int nMax = nPages / m_sizeGrid.cx;
			if ((nMax * m_sizeGrid.cx) < (int)nPages)
			{
				nMax++;
			}
			int nPage = m_sizeGrid.cy;
			m_nOffsetPage = min(m_nOffsetPage, max(nMax - nPage, 0));

			SCROLLINFO info;
			info.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
			info.nMin = 0;
			info.nMax = nMax - 1;
			info.nPage = nPage;
			info.nPos = m_nOffsetPage;
			SetScrollInfo(SB_VERT, &info, bRedraw);
		}
		else
		{
			m_nOffsetPage = 0;
		}
	}
}

void CBCGPPrintPreviewCtrl::OnZoomChanged()
{
	CWnd* pParentWnd = GetParent ();
	if (pParentWnd != NULL && m_nNotifyZoom > 0)
	{
		if (m_bNotifyZoomPost)
		{
			pParentWnd->PostMessage (WM_COMMAND, m_nNotifyZoom, 0);
		}
		else
		{
			pParentWnd->SendMessage (WM_COMMAND, m_nNotifyZoom, 0);
		}
	}
}

void CBCGPPrintPreviewCtrl::OnPageChanged ()
{
	CWnd* pParentWnd = GetParent ();
	if (pParentWnd != NULL && m_nNotifyPage > 0)
	{
		if (m_bNotifyPagePost)
		{
			pParentWnd->PostMessage (WM_COMMAND, m_nNotifyPage, 0);
		}
		else
		{
			pParentWnd->SendMessage (WM_COMMAND, m_nNotifyPage, 0);
		}
	}
}

void CBCGPPrintPreviewCtrl::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* /*pScrollBar*/) 
{
	SCROLLINFO info;
	info.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
	if (!GetScrollInfo(SB_HORZ, &info, info.fMask))
	{
		return;
	}

	LONG nOldValue = m_ptOffsetPage.x;

	switch (nSBCode)
	{
	case SB_LEFT:
		m_ptOffsetPage.x = info.nMin;
		break;

	case SB_RIGHT:
		m_ptOffsetPage.x = info.nMax;
		break;

	case SB_PAGELEFT:
		m_ptOffsetPage.x -= info.nPage;
		break;

	case SB_PAGERIGHT:
		m_ptOffsetPage.x += info.nPage;
		break;

	case SB_LINELEFT:
		m_ptOffsetPage.x--;
		break;

	case SB_LINERIGHT:
		m_ptOffsetPage.x++;
		break;

	case SB_THUMBPOSITION:
		m_ptOffsetPage.x = nPos;
		break;
	}

	m_ptOffsetPage.x = max(m_ptOffsetPage.x, 0);
	UpdateScrollBars ();

	if (nOldValue != m_ptOffsetPage.x)
	{
		m_bInvalidate = TRUE;
		Invalidate();
	}
}

void CBCGPPrintPreviewCtrl::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* /*pScrollBar*/) 
{
	SCROLLINFO info;
	info.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
	if (!GetScrollInfo(SB_VERT, &info, info.fMask))
	{
		return;
	}

	LONG* pValue = m_bScrollPage ? &m_ptOffsetPage.y : &m_nOffsetPage;
	LONG nOldValue = *pValue;

	switch (nSBCode)
	{
	case SB_TOP:
		*pValue = info.nMin;
		break;

	case SB_BOTTOM:
		*pValue = info.nMax;
		break;

	case SB_PAGEUP:
		*pValue -= info.nPage;
		break;

	case SB_PAGEDOWN:
		*pValue += info.nPage;
		break;

	case SB_LINEUP:
		(*pValue)--;
		break;

	case SB_LINEDOWN:
		(*pValue)++;
		break;

	case SB_THUMBPOSITION:
		*pValue = nPos;
		break;
	}

	*pValue = max(*pValue, 0);

	UpdateScrollBars ();

	if (nOldValue != *pValue)
	{
		if (!m_bScrollPage)
		{
			SetCurrentPage (GetStartPage (), FALSE);
		}

		m_bInvalidate = TRUE;
		Invalidate();
	}
}

BOOL CBCGPPrintPreviewCtrl::OnMouseWheel(UINT nFlags, short zDelta, CPoint /*pt*/) 
{
	if (CBCGPPopupMenu::GetActiveMenu () != NULL)
	{
		return TRUE;
	}

	int nSteps = abs(zDelta) / WHEEL_DELTA;

	if (m_bScrollPage)
	{
		int nScrollLines = 3;
		::SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &nScrollLines, 0);
		nSteps *= nScrollLines;
	}

	if (m_bScrollBars[1] && ((nFlags & MK_SHIFT) != MK_SHIFT || !m_bScrollBars[0]))
	{
		for (int i = 0; i < nSteps; i++)
		{
			OnVScroll (zDelta < 0 ? SB_LINEDOWN : SB_LINEUP, 0, NULL);
		}
	}
	else if (m_bScrollBars[0])
	{
		for (int i = 0; i < nSteps; i++)
		{
			OnHScroll (zDelta < 0 ? SB_LINELEFT : SB_LINERIGHT, 0, NULL);
		}
	}

	return TRUE;
}

int CBCGPPrintPreviewCtrl::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message)
{
	int nResult = CBCGPWnd::OnMouseActivate (pDesktopWnd, nHitTest, message);
	if (nResult == MA_ACTIVATE)
	{
		SetFocus ();
	}

	return nResult;
}

void CBCGPPrintPreviewCtrl::OnSetFocus(CWnd* pOldWnd)
{
	CBCGPWnd::OnSetFocus (pOldWnd);

	if (m_bDrawFocus)
	{
		Invalidate ();
	}
}

void CBCGPPrintPreviewCtrl::OnKillFocus(CWnd* pNewWnd)
{
	CBCGPWnd::OnKillFocus (pNewWnd);

	if (m_bDrawFocus)
	{
		Invalidate ();
	}
}

void CBCGPPrintPreviewCtrl::EnableDrawFocus(BOOL bEnable/* = TRUE*/)
{
	if (m_bDrawFocus == bEnable)
	{
		return;
	}

	m_bDrawFocus = bEnable;
	if (GetSafeHwnd() != NULL)
	{
		Invalidate();
	}
}

LRESULT CBCGPPrintPreviewCtrl::OnPrintClient(WPARAM wp, LPARAM lp)
{
	if (lp & PRF_CLIENT)
	{
		CDC* pDC = CDC::FromHandle((HDC) wp);
		ASSERT_VALID(pDC);

		DoDraw(pDC);
	}

	return 0;
}

LRESULT CBCGPPrintPreviewCtrl::OnBCGSetControlBackStageMode (WPARAM, LPARAM)
{
	m_bBackstageMode = TRUE;
	return 0;
}
