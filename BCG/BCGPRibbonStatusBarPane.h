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
// BCGPRibbonStatusBarPane.h: interface for the CBCGPRibbonStatusBarPane class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPRIBBONSTATUSBARPANE_H__DCBDC9D2_0542_47B0_A86A_CDCD65D0386B__INCLUDED_)
#define AFX_BCGPRIBBONSTATUSBARPANE_H__DCBDC9D2_0542_47B0_A86A_CDCD65D0386B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"

#ifndef BCGP_EXCLUDE_RIBBON

#include "BCGPRibbonButton.h"
#include "BCGPToolbarImages.h"

class BCGCBPRODLLEXPORT CBCGPRibbonStatusBarPane : public CBCGPRibbonButton  
{
	friend class CBCGPRibbonStatusBar;

	DECLARE_DYNCREATE(CBCGPRibbonStatusBarPane)

// Construction
public:
	CBCGPRibbonStatusBarPane();
	
	CBCGPRibbonStatusBarPane (
		UINT		nCmdID,						// Pane command id
		LPCTSTR		lpszText,					// Pane label
		BOOL		bIsStatic = FALSE,			// Pane is static (non-clickable)
		HICON		hIcon = NULL,				// Pane icon
		LPCTSTR		lpszAlmostLargeText = NULL,	// The almost large text in pane
		BOOL		bAlphaBlendIcon = FALSE);	// The icon is 32 bit

	CBCGPRibbonStatusBarPane (
		UINT		nCmdID,						// Pane command id
		LPCTSTR		lpszText,					// Pane label
		HBITMAP		hBmpAnimationList,			// Pane animation list
		int			cxAnimation = 16,			// Animation frame width
		COLORREF	clrTrnsp= RGB (192,192,192),// Transparent color
		HICON		hIcon = NULL,				// Pane icon
		BOOL		bIsStatic = FALSE,			// Pane is static (non-clickable)
		BOOL		bDontScaleInHighDPIMode = FALSE,
		BOOL		bAlphaBlendIcon = FALSE);	// The icon is 32 bit

	CBCGPRibbonStatusBarPane (
		UINT		nCmdID,						// Pane command id
		LPCTSTR		lpszText,					// Pane label
		UINT		uiAnimationListResID,		// Pane animation list
		int			cxAnimation = 16,			// Animation frame width
		COLORREF	clrTrnsp= RGB (192,192,192),// Transparent color
		HICON		hIcon = NULL,				// Pane icon
		BOOL		bIsStatic = FALSE,			// Pane is static (non-clickable)
		BOOL		bDontScaleInHighDPIMode = FALSE,
		BOOL		bAlphaBlendIcon = FALSE);	// The icon is 32 bit

	virtual ~CBCGPRibbonStatusBarPane();

// Attributes
public:
	BOOL IsExtended () const		// Is located on status bar extended (right) area?
	{
		return m_bIsExtended;
	}

	BOOL IsStatic () const
	{
		return m_bIsStatic;
	}

	LPCTSTR GetAlmostLargeText () const
	{
		return m_strAlmostLargeText;
	}

	void SetAlmostLargeText (LPCTSTR lpszAlmostLargeText)
	{
		m_strAlmostLargeText = (lpszAlmostLargeText == NULL) ? _T("") : lpszAlmostLargeText;
		CleanUpSizes ();
	}

	int GetTextAlign () const
	{
		return m_nTextAlign;
	}

	void SetTextAlign (int nAlign)
	{
		m_nTextAlign = nAlign;
	}

	BOOL IsAnimation () const
	{
		return m_nAnimTimerID != 0;
	}

protected:
	BOOL		m_bIsStatic;			// Pane is static (non-clickable)
	BOOL		m_bIsExtended;			// Is located on status bar extended (right) area
	CString		m_strAlmostLargeText;	// The almost large text in pane
	int			m_nTextAlign;			// TA_LEFT, TA_CENTER or TA_RIGHT
	BOOL		m_bIsTextTruncated;		// Is text truncated?
	
	CBCGPToolBarImages	m_AnimImages;	// List of animation frames
	UINT		m_nAnimTimerID;			// Animation timer ID
	int			m_nAnimationIndex;		// Current animation frame number
	UINT		m_nAnimationDuration;	// Animation duaration
	DWORD		m_dwAnimationStartTime;	// Animation start time

	static CMap<UINT,UINT,CBCGPRibbonStatusBarPane*,CBCGPRibbonStatusBarPane*>
		m_mapAnimations;
	static CCriticalSection g_cs;			// For multi-thread applications

// Operations
public:
	void SetAnimationList (	HBITMAP hBmpAnimationList, int cxAnimation = 16, 
							COLORREF clrTransp = RGB (192, 192, 192),
							BOOL bDontScaleInHighDPIMode = FALSE);
	BOOL SetAnimationList (	UINT uiAnimationListResID, int cxAnimation = 16,
							COLORREF clrTransp = RGB (192, 192, 192),
							BOOL bDontScaleInHighDPIMode = FALSE);

	void StartAnimation (	UINT nFrameDelay = 500,	// ms
							UINT nDuration = -1);	// ms
	void StopAnimation ();

protected:
	void CommonInit ();

	static VOID CALLBACK AnimTimerProc (HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

// Overrides
public:
	virtual COLORREF OnFillBackground (CDC* pDC);
	virtual void OnDrawBorder (CDC* /*pDC*/)	{}

protected:
	virtual int GetTextOffset () const	{	return m_szMargin.cx / 2;	}
	virtual void OnCalcTextSize (CDC* pDC);
	virtual int DoDrawText (CDC* pDC, const CString& strText, CRect rectText, UINT uiDTFlags);
	virtual CSize GetIntermediateSize (CDC* pDC);
	virtual void OnDraw (CDC* pDC);

	virtual CString GetToolTipText () const;

	virtual BOOL CanBeStretched ()
	{
		return TRUE;
	}

	virtual void CopyFrom (const CBCGPBaseRibbonElement& src);

	virtual void OnFinishAnimation ()	{}

	virtual void OnRTLChanged (BOOL bIsRTL);

	virtual BOOL IsDrawTooltipImage () const
	{	
		return m_AnimImages.GetCount () <= 0;
	}

	virtual void SetMargin (const CSize& /*sizeMargiin*/)
	{
		m_szMargin = CSize (9, 0);
	}

	virtual BOOL HasLargeMode () const
	{
		return FALSE;
	}

	virtual BOOL HasIntermediateMode () const
	{
		return TRUE;
	}

	virtual BOOL HasCompactMode () const
	{
		return FALSE;
	}

};

#endif // BCGP_EXCLUDE_RIBBON

#endif // !defined(AFX_BCGPRIBBONSTATUSBARPANE_H__DCBDC9D2_0542_47B0_A86A_CDCD65D0386B__INCLUDED_)
