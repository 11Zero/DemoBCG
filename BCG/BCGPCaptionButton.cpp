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
// BCGPCaptionButton.cpp: implementation of the CBCGPCaptionButton class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "bcgcbpro.h"
#include "BCGGlobals.h"
#include "BCGPCaptionButton.h"
#include "BCGPVisualManager.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

int CBCGPCaptionButton::m_nButtonMargin = 3;
int CBCGPCaptionButton::m_nButtonMarginVert = 4;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGPCaptionButton::CBCGPCaptionButton()
{
	m_bPushed = FALSE;
	m_bFocused = FALSE;
	m_bDroppedDown = FALSE;
	m_bHidden = FALSE;
	m_bEnabled = TRUE;
	m_nHit = HTNOWHERE;
	m_bLeftAlign = FALSE;
	m_clrForeground = (COLORREF)-1;
	m_bIsMiniFrameButton = FALSE;
}

CBCGPCaptionButton::CBCGPCaptionButton(UINT nHit, BOOL bLeftAlign)
{
	m_bPushed = FALSE;
	m_bFocused = FALSE;
	m_bDroppedDown = FALSE;
	m_bHidden = FALSE;
	m_bEnabled = TRUE;
	m_nHit = nHit;
	m_bLeftAlign = bLeftAlign;
	m_clrForeground = (COLORREF)-1;
	m_bIsMiniFrameButton = FALSE;
}

CBCGPCaptionButton::~CBCGPCaptionButton()
{
}

UINT CBCGPCaptionButton::GetHit () const
{
	return m_nHit;
}

void CBCGPCaptionButton::OnDraw (CDC* pDC, BOOL bActive,
								 BOOL /*bHorz*/, BOOL bMaximized, BOOL bDisabled)
{
	if (m_bHidden)
	{
		return;
	}

	CBCGPVisualManager::GetInstance ()->OnDrawCaptionButton (
			pDC, this, bActive, FALSE, bMaximized, bDisabled || !m_bEnabled);
}

CBCGPMenuImages::IMAGES_IDS CBCGPCaptionButton::GetIconID (BOOL bHorz, BOOL bMaximized) const
{
	switch (m_nHit)
	{
	case HTCLOSE:
	case HTCLOSE_BCG:
		return CBCGPMenuImages::IdClose;

	case HTMINBUTTON:
		return bHorz ? 
			bMaximized ? CBCGPMenuImages::IdArowLeft : CBCGPMenuImages::IdArowRight : 
			bMaximized ? CBCGPMenuImages::IdArowDownLarge :  CBCGPMenuImages::IdArowUp;

	case HTMAXBUTTON:
		return bMaximized ? CBCGPMenuImages::IdPinHorz : CBCGPMenuImages::IdPinVert;

	case HTMAXBUTTON_BCG:
		return CBCGPMenuImages::IdMaximize;
		
	case HTMINBUTTON_BCG:
		return CBCGPMenuImages::IdRestore;

	case HTLEFTBUTTON_BCG:
		return CBCGPMenuImages::IdArowBack;

	case HTRIGHTBUTTON_BCG:
		return CBCGPMenuImages::IdArowForward;

	case HTMENU_BCG:
		return CBCGPMenuImages::IdArowDownLarge;
	}

	return (CBCGPMenuImages::IMAGES_IDS)-1;
}

CBCGPFrameCaptionButton::CBCGPFrameCaptionButton (UINT nHit)
{
	m_nHit = nHit;
	m_rect.SetRectEmpty ();
}

CBCGPFrameCaptionButton::~CBCGPFrameCaptionButton()
{
}

CBCGPDockingBarScrollButton::CBCGPDockingBarScrollButton (UINT nHit)
{
	m_nHit = nHit;
	m_rect.SetRectEmpty ();
}

CBCGPDockingBarScrollButton::~CBCGPDockingBarScrollButton()
{
}

CBCGPMenuImages::IMAGES_IDS CBCGPDockingBarScrollButton::GetIconID (BOOL /*bHorz*/, BOOL /*bMaximized*/) const
{
	switch (m_nHit)
	{
	case HTSCROLLUPBUTTON_BCG:
		return CBCGPMenuImages::IdArowUpLarge;

	case HTSCROLLDOWNBUTTON_BCG:
		return CBCGPMenuImages::IdArowDownLarge;

	case HTSCROLLLEFTBUTTON_BCG:
		return CBCGPMenuImages::IdArowLeftLarge;

	case HTSCROLLRIGHTBUTTON_BCG:
		return CBCGPMenuImages::IdArowRightLarge;
	}

	return (CBCGPMenuImages::IMAGES_IDS)-1;
}

void CBCGPDockingBarScrollButton::OnDraw (CDC* pDC, BOOL /*bActive*/,
								 BOOL /*bHorz*/, BOOL /*bMaximized*/, BOOL /*bDisabled*/)
{
	if (m_bHidden || m_rect.IsRectEmpty())
	{
		return;
	}

	CBCGPVisualManager::GetInstance ()->OnDrawDockingBarScrollButton (pDC, this);
}
