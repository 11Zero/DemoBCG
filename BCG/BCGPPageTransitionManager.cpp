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
// BCGPPageTransitionManager.cpp: implementation of the CBCGPPageTransitionManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BCGPDrawManager.h"
#include "BCGPPageTransitionManager.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CMap<UINT,UINT,CBCGPPageTransitionManager*,CBCGPPageTransitionManager*> CBCGPPageTransitionManager::m_mapManagers;
CCriticalSection CBCGPPageTransitionManager::g_cs;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGPPageTransitionManager::CBCGPPageTransitionManager()
{
	m_PageTransitionEffect = BCGPPageTransitionNone;
	m_nPageTransitionTime = 300;
	m_nPageTransitionOffset = 0;
	m_nPageTransitionStep = 0;
	m_nPageTransitionTotal = 0;
	m_nPageScreenshot1 = 0;
	m_nPageScreenshot2 = 0;
	m_rectPageTransition.SetRectEmpty();
	m_nTimerID = 0;
	m_hwndHost = NULL;
	m_clrFillFrame = (COLORREF)-1;
}
//*****************************************************************************************
CBCGPPageTransitionManager::~CBCGPPageTransitionManager()
{
	StopPageTransition();
}
//*****************************************************************************************
void CBCGPPageTransitionManager::StopPageTransition()
{
	if (m_nTimerID != 0)
	{
		::KillTimer(NULL, m_nTimerID);
		
		g_cs.Lock ();
		m_mapManagers.RemoveKey(m_nTimerID);
		g_cs.Unlock ();

		m_nTimerID = 0;
	}

	m_nPageTransitionOffset = 0;
	m_nPageTransitionStep = 0;
	m_nPageTransitionTotal = 0;
	m_rectPageTransition.SetRectEmpty();
	m_Panorama.Clear();
}
//*****************************************************************************************
BOOL CBCGPPageTransitionManager::StartPageTransition(HWND hwdHost, const CArray<HWND, HWND>& arPages, BOOL bReverseOrder, 
													 const CSize& szPageOffset, const CSize& szPageMax)
{
	StopPageTransition();

	if (hwdHost == NULL || arPages.GetSize() < 2 || m_nPageTransitionTime < 1 || m_PageTransitionEffect == BCGPPageTransitionNone)
	{
		return FALSE;
	}

	CWnd* pWndCurrPage = CWnd::FromHandle(arPages[0]);

	m_hwndHost = hwdHost;
	CWnd* pWndHost = CWnd::FromHandle(m_hwndHost);

	const BOOL bIsRTL = (pWndHost->GetExStyle() & WS_EX_LAYOUTRTL);

	pWndCurrPage->GetWindowRect(m_rectPageTransition);

	if (szPageMax.cx != 0)
	{
		m_rectPageTransition.right = m_rectPageTransition.left + szPageMax.cx;
	}

	if (szPageMax.cy != 0)
	{
		m_rectPageTransition.bottom = m_rectPageTransition.top + szPageMax.cy;
	}

	int cxOffset = bIsRTL ? 0 : szPageOffset.cx;
	int cx = m_rectPageTransition.Width() - cxOffset;
	int cy = m_rectPageTransition.Height() - szPageOffset.cy;

	pWndHost->ScreenToClient(&m_rectPageTransition);

	// Create panarama bitmap:
	m_Panorama.SetImageSize(CSize(cx, cy));

	pWndHost->SetRedraw(FALSE);

	BOOL bTwoPagesTransition = 
		m_PageTransitionEffect != BCGPPageTransitionSlide && 
		m_PageTransitionEffect != BCGPPageTransitionSlideVertical;

	m_nPageScreenshot1 = bReverseOrder ? 1 : 0;
	m_nPageScreenshot2 = bReverseOrder ? 0 : 1;

	HBITMAP hbmp = CBCGPDrawManager::CreateBitmap_32(CSize(cx, cy), NULL);
	if (hbmp == NULL)
	{
		StopPageTransition();
		return FALSE;
	}

	CDC dcMem;
	dcMem.CreateCompatibleDC(NULL);
	
	for (int i = 0; i < arPages.GetSize(); i++)
	{
		if (bTwoPagesTransition && (i != 0 && i != arPages.GetSize() - 1))
		{
			continue;
		}

		CWnd* pPage = CWnd::FromHandle(arPages[i]);
		ASSERT(pPage->GetSafeHwnd() != NULL);

		HBITMAP hbmpOld = (HBITMAP) dcMem.SelectObject(hbmp);

		if (m_clrFillFrame != (COLORREF)-1)
		{
			dcMem.FillSolidRect(CRect(0, 0, cx, cy), m_clrFillFrame);
		}

		pPage->SendMessage(WM_PRINT, (WPARAM)dcMem.GetSafeHdc(), (LPARAM)(PRF_CLIENT | PRF_CHILDREN | PRF_NONCLIENT | PRF_ERASEBKGND));
		dcMem.SelectObject (hbmpOld);

		m_Panorama.AddImage(hbmp, TRUE);
	}

	::DeleteObject(hbmp);

	pWndCurrPage->ShowWindow(SW_HIDE);

	pWndHost->SetRedraw(TRUE);

	return StartInternal(bReverseOrder);
}
//*****************************************************************************************
BOOL CBCGPPageTransitionManager::StartInternal(BOOL bReverseOrder)
{
	int nElapse = 10;

	if (m_PageTransitionEffect == BCGPPageTransitionFade)
	{
		m_nPageTransitionTotal = 255;
		m_nPageTransitionStep = max(1, m_nPageTransitionTotal * nElapse / m_nPageTransitionTime);
		m_nPageTransitionOffset = 0;
	}
	else if (m_PageTransitionEffect == BCGPPageTransitionPop)
	{
		m_nPageTransitionTotal = min(m_rectPageTransition.Width(), m_rectPageTransition.Height()); 
		m_nPageTransitionStep = max(1, m_nPageTransitionTotal * nElapse / m_nPageTransitionTime);
		m_nPageTransitionOffset = 0;
	}
	else
	{
		BOOL bIsVerticalSliding =
			m_PageTransitionEffect == BCGPPageTransitionSlideVertical ||
			m_PageTransitionEffect == BCGPPageTransitionSimpleSlideVertical;

		if (bIsVerticalSliding)
		{
			m_nPageTransitionTotal = (m_Panorama.GetCount() - 1) * m_Panorama.GetImageSize().cy;
		}
		else
		{
			m_nPageTransitionTotal = (m_Panorama.GetCount() - 1) * m_Panorama.GetImageSize().cx;
		}

		m_nPageTransitionStep = m_nPageTransitionTotal * nElapse / m_nPageTransitionTime;

		if (!bReverseOrder)
		{
			m_nPageTransitionOffset = 0;
			m_nPageTransitionStep = max(1, m_nPageTransitionStep);
		}
		else
		{
			m_nPageTransitionOffset = m_nPageTransitionTotal;
			m_nPageTransitionStep = -m_nPageTransitionStep;
			m_nPageTransitionStep = min(-1, m_nPageTransitionStep);
		}
	}

	if (m_nPageTransitionStep == 0 || m_nPageTransitionTotal == 0)
	{
		StopPageTransition();
		return FALSE;
	}

	m_nTimerID = (UINT)::SetTimer (NULL, 0, nElapse, PageTransitionTimerProc);

	g_cs.Lock ();
	m_mapManagers.SetAt(m_nTimerID, this);
	g_cs.Unlock ();

	OnTimerEvent();
	return TRUE;
}
//*****************************************************************************************
BOOL CBCGPPageTransitionManager::StartPageTransition(HWND hwdHost, HWND hwndPageFrom, HWND hwndPageTo, BOOL bReverseOrder, 
													 const CSize& szPageOffset, const CSize& szPageMax)
{
	CArray<HWND, HWND> arPages;

	if (bReverseOrder)
	{
		arPages.Add(hwndPageTo);
		arPages.Add(hwndPageFrom);
	}
	else
	{
		arPages.Add(hwndPageFrom);
		arPages.Add(hwndPageTo);
	}

	return StartPageTransition(hwdHost, arPages, bReverseOrder, szPageOffset, szPageMax);
}
//*****************************************************************************************
BOOL CBCGPPageTransitionManager::StartBitmapTransition(HWND hwdHost, const CArray<HBITMAP, HBITMAP>& arPages, const CRect& rectPageTransition, BOOL bReverseOrder)
{
	StopPageTransition();

	if (hwdHost == NULL || arPages.GetSize() < 2 || m_nPageTransitionTime < 1 || m_PageTransitionEffect == BCGPPageTransitionNone)
	{
		return FALSE;
	}

	m_hwndHost = hwdHost;
	m_rectPageTransition = rectPageTransition;

	int cx = m_rectPageTransition.Width();
	int cy = m_rectPageTransition.Height();

	// Create panarama bitmap:
	m_Panorama.SetImageSize(CSize(cx, cy));

	BOOL bTwoPagesTransition = 
		m_PageTransitionEffect != BCGPPageTransitionSlide && 
		m_PageTransitionEffect != BCGPPageTransitionSlideVertical;

	m_nPageScreenshot1 = bReverseOrder ? 1 : 0;
	m_nPageScreenshot2 = bReverseOrder ? 0 : 1;

	HBITMAP hbmp = CBCGPDrawManager::CreateBitmap_32(CSize(cx, cy), NULL);
	if (hbmp == NULL)
	{
		StopPageTransition();
		return FALSE;
	}

	CDC dcMem;
	dcMem.CreateCompatibleDC(NULL);
	
	for (int i = 0; i < arPages.GetSize(); i++)
	{
		if (bTwoPagesTransition && (i != 0 && i != arPages.GetSize() - 1))
		{
			continue;
		}

		HBITMAP hbmpOld = (HBITMAP) dcMem.SelectObject(hbmp);

		if (m_clrFillFrame != (COLORREF)-1)
		{
			dcMem.FillSolidRect(CRect(0, 0, cx, cy), m_clrFillFrame);
		}

		BITMAP bmp;
		::GetObject(arPages[i], sizeof(BITMAP), (LPVOID)&bmp);

		CSize sizeDraw;
		sizeDraw.cx = min(cx, bmp.bmWidth);
		sizeDraw.cy = min(cy, bmp.bmHeight);

		dcMem.DrawState(CPoint(0, 0), sizeDraw, arPages[i], DSS_NORMAL);

		dcMem.SelectObject (hbmpOld);

		m_Panorama.AddImage(hbmp, TRUE);
	}

	::DeleteObject(hbmp);

	return StartInternal(bReverseOrder);
}
//*****************************************************************************************
BOOL CBCGPPageTransitionManager::StartBitmapTransition(HWND hwdHost, HBITMAP hbmpFrom, HBITMAP hbmpTo, const CRect& rectPageTransition, BOOL bReverseOrder)
{
	CArray<HBITMAP, HBITMAP> arPages;
	
	if (bReverseOrder)
	{
		arPages.Add(hbmpTo);
		arPages.Add(hbmpFrom);
	}
	else
	{
		arPages.Add(hbmpFrom);
		arPages.Add(hbmpTo);
	}
	
	return StartBitmapTransition(hwdHost, arPages, rectPageTransition, bReverseOrder);
}
//*****************************************************************************************
VOID CALLBACK CBCGPPageTransitionManager::PageTransitionTimerProc(HWND /*hwnd*/, UINT /*uMsg*/, UINT_PTR idEvent, DWORD /*dwTime*/)
{
	CBCGPPageTransitionManager* pObject = NULL;
	
	g_cs.Lock ();
	
	if (!m_mapManagers.Lookup((UINT)idEvent, pObject))
	{
		g_cs.Unlock ();
		return;
	}
	
	ASSERT(pObject != NULL);
	
	g_cs.Unlock ();
	
	pObject->OnTimerEvent();
}
//*****************************************************************************************
void CBCGPPageTransitionManager::OnTimerEvent()
{
	m_nPageTransitionOffset += m_nPageTransitionStep;
	
	BOOL bStopTransition = FALSE;
	
	if (m_nPageTransitionStep < 0)
	{
		if (m_nPageTransitionOffset <= 0)
		{
			bStopTransition = TRUE;
		}
	}
	else
	{
		if (m_nPageTransitionOffset >= m_nPageTransitionTotal)
		{
			bStopTransition = TRUE;
		}
	}
	
	CRect rectPageTransition = m_rectPageTransition;
	
	if (bStopTransition)
	{
		StopPageTransition();
		OnPageTransitionFinished();
	}
	
	::RedrawWindow(m_hwndHost, rectPageTransition, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
}
//*****************************************************************************************
void CBCGPPageTransitionManager::DoDrawTransition(CDC* pDC, BOOL bIsMemDC)
{
	if (m_rectPageTransition.IsRectEmpty())
	{
		return;
	}
	
	CRgn rgnClip;
	rgnClip.CreateRectRgnIndirect(bIsMemDC ? CRect(CPoint(0, 0), m_rectPageTransition.Size()) : m_rectPageTransition);
	
	pDC->SelectClipRgn(&rgnClip);
	
	if (m_PageTransitionEffect == BCGPPageTransitionFade)
	{
		m_Panorama.DrawEx(pDC, m_rectPageTransition, m_nPageScreenshot1, CBCGPToolBarImages::ImageAlignHorzLeft, CBCGPToolBarImages::ImageAlignVertTop, 
			NULL, (BYTE)(255 - m_nPageTransitionOffset), TRUE);
		
		m_Panorama.DrawEx(pDC, m_rectPageTransition, m_nPageScreenshot2, CBCGPToolBarImages::ImageAlignHorzLeft, CBCGPToolBarImages::ImageAlignVertTop, 
			NULL, (BYTE)m_nPageTransitionOffset, TRUE);
	}
	else if (m_PageTransitionEffect == BCGPPageTransitionPop)
	{
		m_Panorama.DrawEx(pDC, m_rectPageTransition, m_nPageScreenshot1, CBCGPToolBarImages::ImageAlignHorzLeft, CBCGPToolBarImages::ImageAlignVertTop, 
			NULL, 255, TRUE);
		
		int nWidth = m_rectPageTransition.Width() * m_nPageTransitionOffset / m_nPageTransitionTotal;
		int nHeight = m_rectPageTransition.Height() * m_nPageTransitionOffset / m_nPageTransitionTotal;
		
		CRect rectNewPage(CPoint(m_rectPageTransition.CenterPoint().x - nWidth / 2, m_rectPageTransition.CenterPoint().y - nHeight / 2), CSize(nWidth, nHeight));
		
		m_Panorama.DrawEx(pDC, rectNewPage, m_nPageScreenshot2, CBCGPToolBarImages::ImageAlignHorzStretch, CBCGPToolBarImages::ImageAlignVertStretch, 
			NULL, 255, TRUE);
	}
	else
	{
		const CSize sizeSlide = m_Panorama.GetImageSize();
		const BOOL bIsVeritcal = m_PageTransitionEffect == BCGPPageTransitionSlideVertical || m_PageTransitionEffect == BCGPPageTransitionSimpleSlideVertical;
		
		for (int i = 0; i < m_Panorama.GetCount(); i++)
		{
			CRect rectFrame = m_rectPageTransition;

			if (bIsVeritcal)
			{
				rectFrame.OffsetRect(0, sizeSlide.cy * i - m_nPageTransitionOffset);
			}
			else
			{
				rectFrame.OffsetRect(sizeSlide.cx * i - m_nPageTransitionOffset, 0);
			}
			
			CRect rectInter;
			if (rectInter.IntersectRect(rectFrame, m_rectPageTransition))
			{
				m_Panorama.DrawEx(pDC, rectFrame, i, CBCGPToolBarImages::ImageAlignHorzLeft, CBCGPToolBarImages::ImageAlignVertTop, NULL, 255, TRUE);
			}
		}
	}
	
	pDC->SelectClipRgn (NULL);
}
