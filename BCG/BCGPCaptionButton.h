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
// BCGPCaptionButton.h: interface for the CBCGPCaptionButton class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPCAPTIONBUTTON_H__30374C20_EBE0_479E_9F48_B116A6E2524C__INCLUDED_)
#define AFX_BCGPCAPTIONBUTTON_H__30374C20_EBE0_479E_9F48_B116A6E2524C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"
#include "MenuImages.h"
#include "BCGPVisualManager.h"

#define HTCLOSE_BCG				HTOBJECT	// To prevent standard Windows 98/2000 close tooltip
#define HTLEFTBUTTON_BCG		HTHELP + 2
#define HTRIGHTBUTTON_BCG		HTHELP + 3
#define HTMENU_BCG				HTHELP + 4
#define HTMAXBUTTON_BCG			HTHELP + 5
#define HTMINBUTTON_BCG			HTHELP + 6
#define HTHELPBUTTON_BCG		HTHELP + 7
#define HTSCROLLUPBUTTON_BCG	HTHELP + 8
#define HTSCROLLDOWNBUTTON_BCG	HTHELP + 9
#define HTSCROLLLEFTBUTTON_BCG	HTHELP + 10
#define HTSCROLLRIGHTBUTTON_BCG	HTHELP + 11

class BCGCBPRODLLEXPORT CBCGPCaptionButton : public CObject
{
public:
	CBCGPCaptionButton();
	CBCGPCaptionButton(UINT nHit, BOOL bLeftAlign = FALSE);
	virtual ~CBCGPCaptionButton();

	void Move (const CPoint& ptTo, BOOL bHide = FALSE, const CSize& size = CSize(0, 0))
	{
		m_ptOrg = ptTo; 
		m_bHidden = bHide;
		m_size = size;
	}

	virtual CRect GetRect () const
	{ 
		return CRect (m_ptOrg, m_bHidden ? CSize (0, 0) : GetSize ());
	}

	static CSize GetSize ()
	{
		return CBCGPMenuImages::Size () + CSize (2 * m_nButtonMargin, 2 * m_nButtonMarginVert) +
			CBCGPVisualManager::GetInstance ()->GetCaptionButtonExtraBorder ();
	}

	virtual void OnDraw (CDC* pDC, BOOL bActive, 
		BOOL bHorz = TRUE, BOOL bMaximized = TRUE, BOOL bDisabled = FALSE);

	UINT GetHit () const;

	virtual CBCGPMenuImages::IMAGES_IDS GetIconID (BOOL bHorz, BOOL bMaximized = FALSE) const;

	void SetMiniFrameButton (BOOL bSet = TRUE)
	{
		m_bIsMiniFrameButton = bSet;
	}

	BOOL IsMiniFrameButton () const
	{
		return m_bIsMiniFrameButton;
	}

	virtual BOOL GetCustomToolTip (CString& /*strTipText*/)	{	return FALSE;	}

	BOOL		m_bPushed;
    BOOL		m_bFocused;
	BOOL		m_bHidden;
	BOOL		m_bEnabled;
	BOOL		m_bDroppedDown;

	COLORREF	m_clrForeground;

	UINT		m_nHit;

	BOOL		m_bLeftAlign;

	static int  m_nButtonMargin;
	static int  m_nButtonMarginVert;

protected:
	CPoint		m_ptOrg;
	BOOL		m_bIsMiniFrameButton;
	CSize		m_size;
};

class BCGCBPRODLLEXPORT CBCGPFrameCaptionButton : public CBCGPCaptionButton
{
public:
	CBCGPFrameCaptionButton (UINT nHit);
	~CBCGPFrameCaptionButton();

	void SetRect (const CRect& rect)
	{
		m_rect = rect;
	}

	virtual CRect GetRect () const
	{
		return m_rect;
	}

protected:
	CRect	m_rect;
};

class BCGCBPRODLLEXPORT CBCGPDockingBarScrollButton : public CBCGPCaptionButton
{
public:
	CBCGPDockingBarScrollButton (UINT nHit);
	~CBCGPDockingBarScrollButton();

	void SetRect (const CRect& rect)
	{
		m_rect = rect;
	}

	virtual CRect GetRect () const
	{
		return m_rect;
	}

	virtual CBCGPMenuImages::IMAGES_IDS GetIconID (BOOL bHorz, BOOL bMaximized = FALSE) const;

	virtual void OnDraw (CDC* pDC, BOOL bActive, 
		BOOL bHorz = TRUE, BOOL bMaximized = TRUE, BOOL bDisabled = FALSE);

protected:
	CRect	m_rect;
};

#endif // !defined(AFX_BCGPCAPTIONBUTTON_H__30374C20_EBE0_479E_9F48_B116A6E2524C__INCLUDED_)
