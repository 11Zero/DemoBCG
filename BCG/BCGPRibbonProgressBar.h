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
// BCGPRibbonProgressBar.h: interface for the CBCGPRibbonProgressBar class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPRIBBONPROGRESSBAR_H__6B979AD6_202A_48E3_9AAB_C4572D59A57C__INCLUDED_)
#define AFX_BCGPRIBBONPROGRESSBAR_H__6B979AD6_202A_48E3_9AAB_C4572D59A57C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"

#ifndef BCGP_EXCLUDE_RIBBON

#include "BCGPBaseRibbonElement.h"

class BCGCBPRODLLEXPORT CBCGPRibbonProgressBar : public CBCGPBaseRibbonElement
{
	DECLARE_DYNCREATE(CBCGPRibbonProgressBar)

public:
	CBCGPRibbonProgressBar ();
	CBCGPRibbonProgressBar (
		UINT	nID, 
		int		nWidth = 90,
		int		nHeight = 22,
		BOOL	bIsVertical = FALSE);

	virtual ~CBCGPRibbonProgressBar();

// Attributes
public:
	void SetRange (int nMin, int nMax);

	int GetRangeMin () const		{	return m_nMin;	}
	int GetRangeMax () const		{	return m_nMax;	}

	void SetPos (int nPos, BOOL bRedraw = TRUE);
	int GetPos () const				{	return m_nPos;	}

	void SetInfiniteMode (BOOL bSet = TRUE);
	BOOL IsInfiniteMode () const	{	return m_bInfiniteMode;	}

	int GetWidth () const
	{
		return m_nWidth;
	}
	int GetHeight () const
	{
		return m_nHeight;
	}

	BOOL IsVertical() const { return m_bIsVertical && !IsQATMode(); }

// Operations
protected:
	void CommonInit ();

// Overrides
public:
	virtual void OnDraw (CDC* pDC);
	virtual CSize GetRegularSize (CDC* pDC);

	void CalculateChunkRect(const CRect& rectProgress, CRect& rectChunk);

protected:
	virtual void CopyFrom (const CBCGPBaseRibbonElement& src);

	virtual BOOL CanBeStretched ()
	{
		return FALSE;
	}

	virtual BOOL IsTabStop () const
	{
		return FALSE;
	}

	virtual BOOL IsCustomIconAllowed() const	
	{	
		return FALSE;	
	}

	virtual void OnDrawOnList(CDC* pDC, CString strText, int nTextOffset, CRect rect, BOOL bIsSelected, BOOL bHighlighted);
	virtual BOOL SetACCData(CWnd* pParent, CBCGPAccessibilityData& data);

// Attributes
protected:
	int		m_nMin;
	int		m_nMax;
	int		m_nPos;
	int		m_nWidth;
	int		m_nHeight;
	BOOL	m_bInfiniteMode;
	BOOL	m_bIsVertical;
};

#endif // BCGP_EXCLUDE_RIBBON

#endif // !defined(AFX_BCGPRIBBONPROGRESSBAR_H__6B979AD6_202A_48E3_9AAB_C4572D59A57C__INCLUDED_)
