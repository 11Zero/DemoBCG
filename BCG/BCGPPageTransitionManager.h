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
// BCGPPageTransitionManager.h: interface for the CBCGPPageTransitionManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPPAGETRANSITIONMANAGER_H__43473BD8_A527_4EAE_B561_19A398B49681__INCLUDED_)
#define AFX_BCGPPAGETRANSITIONMANAGER_H__43473BD8_A527_4EAE_B561_19A398B49681__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"

#if (!defined _BCGSUITE_) && (!defined _BCGSUITE_INC_)
	#include "BCGPToolBarImages.h"
#endif

class BCGCBPRODLLEXPORT CBCGPPageTransitionManager
{
public:
	enum BCGPPageTransitionEffect
	{
		BCGPPageTransitionNone,
		BCGPPageTransitionSlide,
		BCGPPageTransitionSimpleSlide,
		BCGPPageTransitionPop,
		BCGPPageTransitionFade,
		BCGPPageTransitionSlideVertical,
		BCGPPageTransitionSimpleSlideVertical,
	};
	
	CBCGPPageTransitionManager();
	virtual ~CBCGPPageTransitionManager();

	void SetPageTransitionEffect(BCGPPageTransitionEffect effect, int nAnimationTime = 300 /* ms */)
	{
		m_PageTransitionEffect = effect;
		m_nPageTransitionTime = nAnimationTime;
	}

	BCGPPageTransitionEffect GetPageTransitionEffect() const 
	{
		return m_PageTransitionEffect;
	}

	BOOL StartPageTransition(HWND hwdHost, const CArray<HWND, HWND>& arPages, BOOL bReverseOrder = FALSE, const CSize& szPageOffset = CSize(0, 0), const CSize& szPageMax = CSize(0, 0));
	BOOL StartPageTransition(HWND hwdHost, HWND hwndPageFrom, HWND hwndPageTo, BOOL bReverseOrder = FALSE, const CSize& szPageOffset = CSize(0, 0), const CSize& szPageMax = CSize(0, 0));

	BOOL StartBitmapTransition(HWND hwdHost, const CArray<HBITMAP, HBITMAP>& arPages, const CRect& rectPageTransition, BOOL bReverseOrder = FALSE);
	BOOL StartBitmapTransition(HWND hwdHost, HBITMAP hbmpFrom, HBITMAP hbmpTo, const CRect& rectPageTransition, BOOL bReverseOrder = FALSE);

	void StopPageTransition();

	virtual void OnPageTransitionFinished() {}

protected:
	virtual void OnTimerEvent();

	void DoDrawTransition(CDC* pDC, BOOL bIsMemDC);
	BOOL StartInternal(BOOL bReverseOrder);

protected:
	static VOID CALLBACK PageTransitionTimerProc (HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

	BCGPPageTransitionEffect	m_PageTransitionEffect;
	int							m_nPageTransitionTime;
#if (!defined _BCGSUITE_) && (!defined _BCGSUITE_INC_)
	CBCGPToolBarImages			m_Panorama;
#else
	CBCGPToolBarImagesSuite		m_Panorama;
#endif
	CRect						m_rectPageTransition;
	int							m_nPageTransitionOffset;
	int							m_nPageTransitionStep;
	int							m_nPageTransitionTotal;
	int							m_nPageScreenshot1;
	int							m_nPageScreenshot2;
	UINT						m_nTimerID;
	HWND						m_hwndHost;
	COLORREF					m_clrFillFrame;

	static CMap<UINT,UINT,CBCGPPageTransitionManager*,CBCGPPageTransitionManager*> m_mapManagers;
	static CCriticalSection g_cs;			// For multi-thread applications
};

#endif // !defined(AFX_BCGPPAGETRANSITIONMANAGER_H__43473BD8_A527_4EAE_B561_19A398B49681__INCLUDED_)
