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
// BCGPProgressCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "BCGPVisualManager.h"
#ifndef _BCGSUITE_
#include "BCGPRibbonProgressBar.h"
#include "BCGPToolBarImages.h"
#endif
#include "bcgglobals.h"
#include "BCGPProgressCtrl.h"
#include "BCGPDlgImpl.h"

#ifndef _BCGSUITE_
	#define visualManagerMFC	CBCGPVisualManager::GetInstance ()
#else
	#define visualManagerMFC	CMFCVisualManager::GetInstance ()
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static const int idTimerMarquee = 1001;

/////////////////////////////////////////////////////////////////////////////
// CBCGPProgressCtrl

IMPLEMENT_DYNAMIC(CBCGPProgressCtrl, CProgressCtrl)

CBCGPProgressCtrl::CBCGPProgressCtrl()
{
	m_bVisualManagerStyle = FALSE;
	m_bOnGlass = FALSE;
	m_nMarqueeStep = 0;
}

CBCGPProgressCtrl::~CBCGPProgressCtrl()
{
}

BEGIN_MESSAGE_MAP(CBCGPProgressCtrl, CProgressCtrl)
	//{{AFX_MSG_MAP(CBCGPProgressCtrl)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_NCCALCSIZE()
	ON_WM_NCPAINT()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
	ON_REGISTERED_MESSAGE(BCGM_ONSETCONTROLVMMODE, OnBCGSetControlVMMode)
	ON_REGISTERED_MESSAGE(BCGM_ONSETCONTROLAERO, OnBCGSetControlAero)
	ON_MESSAGE(PBM_SETMARQUEE, OnSetMarquee)
	ON_MESSAGE(WM_PRINTCLIENT, OnPrintClient)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGPProgressCtrl message handlers

BOOL CBCGPProgressCtrl::OnEraseBkgnd(CDC* /*pDC*/) 
{
	return TRUE;
}
//**************************************************************************
void CBCGPProgressCtrl::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	CBCGPMemDC memDC (dc, this);
	CDC* pDC = &memDC.GetDC ();

	DoPaint(pDC);
}
//**************************************************************************
void CBCGPProgressCtrl::DoPaint(CDC* pDC) 
{
	ASSERT_VALID(pDC);

	if (!CBCGPVisualManager::GetInstance ()->OnFillParentBarBackground (this, pDC))
	{
		globalData.DrawParentBackground (this, pDC);
	}

	CRect rectProgress;
	GetClientRect(rectProgress);

	BOOL bInfiniteMode = (GetStyle() & PBS_MARQUEE) != 0;

	int nMin = 0;
	int nMax = 100;
	int nPos = m_nMarqueeStep;
	
	if (!bInfiniteMode)
	{
		GetRange (nMin, nMax);
		nPos = GetPos();
	}

	CBCGPDrawOnGlass dog (m_bOnGlass);

#if (!defined BCGP_EXCLUDE_RIBBON) && (!defined _BCGSUITE_)

	CBCGPRibbonProgressBar dummy(0, 0, 0, (GetStyle() & PBS_VERTICAL) != 0);
	dummy.SetRange(nMin, nMax);
	dummy.SetPos(nPos, FALSE);
	dummy.SetInfiniteMode(bInfiniteMode);
	
	CRect rectChunk(0, 0, 0, 0);
	dummy.CalculateChunkRect(rectProgress, rectChunk);

	visualManagerMFC->OnDrawRibbonProgressBar(pDC, &dummy, rectProgress, rectChunk, bInfiniteMode);
#else
	visualManagerMFC->OnDrawStatusBarProgress (pDC, NULL,
										rectProgress, nMax - nMin, 
										GetPos (),
										globalData.clrHilite, 
										(COLORREF)-1, 
										(COLORREF)-1,
										FALSE);
#endif
}
//**************************************************************************
void CBCGPProgressCtrl::OnNcCalcSize(BOOL /*bCalcValidRects*/, NCCALCSIZE_PARAMS FAR* /*lpncsp*/) 
{
}
//**************************************************************************
void CBCGPProgressCtrl::OnNcPaint() 
{
}
//**************************************************************************
LRESULT CBCGPProgressCtrl::OnBCGSetControlVMMode (WPARAM wp, LPARAM)
{
	m_bVisualManagerStyle = (BOOL) wp;
	return 0;
}
//**************************************************************************
LRESULT CBCGPProgressCtrl::OnBCGSetControlAero (WPARAM wp, LPARAM)
{
	m_bOnGlass = (BOOL) wp;
	return 0;
}
//**************************************************************************
LRESULT CBCGPProgressCtrl::OnSetMarquee(WPARAM wp, LPARAM lp)
{
	BOOL bStart = (BOOL)wp;
	int nElapse = (lp == 0) ? 30 : (int)lp;

	if (bStart)
	{
		SetTimer(idTimerMarquee, nElapse, NULL);
	}
	else
	{
		KillTimer(idTimerMarquee);
	}

	return Default();
}
//**************************************************************************
void CBCGPProgressCtrl::OnTimer(UINT_PTR nIDEvent) 
{
	CProgressCtrl::OnTimer(nIDEvent);

	if (nIDEvent == idTimerMarquee)
	{
		m_nMarqueeStep++;
		if (m_nMarqueeStep > 100)
		{
			m_nMarqueeStep = 0;
		}

		RedrawWindow();
	}
}
//**************************************************************************
LRESULT CBCGPProgressCtrl::OnPrintClient(WPARAM wp, LPARAM lp)
{
	if (lp & PRF_CLIENT)
	{
		CDC* pDC = CDC::FromHandle((HDC) wp);
		ASSERT_VALID(pDC);
		
		CRect rectClient;
		GetClientRect(rectClient);
		
		CBCGPMemDC memDC(*pDC, rectClient);
		
		DoPaint(&memDC.GetDC());
	}
	
	return 0;
}
