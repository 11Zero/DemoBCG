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
// BCGPRibbonSlider.h: interface for the CBCGPRibbonSlider class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPRIBBONSLIDER_H__50A94B44_9870_4142_ABF2_4AE47D68EF16__INCLUDED_)
#define AFX_BCGPRIBBONSLIDER_H__50A94B44_9870_4142_ABF2_4AE47D68EF16__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"

#ifndef BCGP_EXCLUDE_RIBBON

#include "BCGPBaseRibbonElement.h"

class BCGCBPRODLLEXPORT CBCGPRibbonSlider : public CBCGPBaseRibbonElement
{
	DECLARE_DYNCREATE(CBCGPRibbonSlider)

// Construction
public:
	CBCGPRibbonSlider();

	CBCGPRibbonSlider (
		UINT	nID, 
		int		nWidth = 100,
		DWORD	dwStyle = 0);	// TBS_**** styles

	virtual ~CBCGPRibbonSlider();

// Attributes
public:
	void SetRange (int nMin, int nMax);

	int GetRangeMin () const	{	return m_nMin;	}
	int GetRangeMax () const	{	return m_nMax;	}

	void SetPos (int nPos, BOOL bRedraw = TRUE);

	int GetPos () const			{	return m_nPos;	}

	int GetPageSize () const	{	return m_nPageSize;	}
	void SetPageSize (int nPageSize);

	void SetZoomButtons (BOOL bSet = TRUE);
	BOOL HasZoomButtons () const
	{
		return m_bZoomButtons;
	}

	BOOL IsVert () const
	{
		return (m_dwStyle & TBS_VERT) && !IsQATMode ();
	}

	BOOL IsThumbLeftTop () const
	{
		return (m_dwStyle & TBS_BOTH) || (m_dwStyle & TBS_LEFT);
	}

	BOOL IsThumbRightBottom () const
	{
		return (m_dwStyle & TBS_BOTH) || ((m_dwStyle & TBS_LEFT) == 0);
	}

	DWORD GetStyle () const
	{
		return m_dwStyle;
	}
	int GetWidth () const
	{
		return m_nWidth;
	}

// Operations
protected:
	void CommonInit ();
	void SetThumbRect ();

	int GetPosFromPoint (CPoint pt);

	int GetHitTest (CPoint point) const;

	void NotifyCommand ();

// Overrides
public:
	virtual void OnDraw (CDC* pDC);
	virtual CSize GetRegularSize (CDC* pDC);

protected:
	virtual void CopyFrom (const CBCGPBaseRibbonElement& src);
	virtual void OnAfterChangeRect (CDC* pDC);

	virtual void OnLButtonDown (CPoint point);
	virtual void OnLButtonUp (CPoint point);
	virtual void OnMouseMove (CPoint point);

	virtual BOOL IsAutoRepeatMode (int& nDelay /* ms */) const;
	virtual BOOL OnAutoRepeat ();

	virtual void OnDrawOnList (CDC* pDC, CString strText, int nTextOffset, CRect rect, BOOL bIsSelected, BOOL bHighlighted);

	virtual BOOL OnProcessKey (UINT nChar);

	virtual BOOL IsWholeRowHeight () const
	{
		return IsVert ();
	}

	virtual BOOL IsCustomIconAllowed() const	
	{	
		return FALSE;	
	}

	virtual BOOL SetACCData(CWnd* pParent, CBCGPAccessibilityData& data);

// Attributes
protected:
	int		m_nMin;
	int		m_nMax;
	int		m_nPos;
	int		m_nPageSize;
	BOOL	m_bZoomButtons;
	int		m_nWidth;
	DWORD	m_dwStyle;

	CRect	m_rectZoomOut;
	CRect	m_rectZoomIn;
	CRect	m_rectSlider;
	CRect	m_rectThumb;
	CRect	m_rectLabel;

	int		m_nHighlighted;	// -1 - none, 0 - thumb, 1 -Zoom In, 2 - Zoom Out
	int		m_nPressed;		// -1 - none, 0 - thumb, 1 -Zoom In, 2 - Zoom Out
};

#endif // BCGP_EXCLUDE_RIBBON

#endif // !defined(AFX_BCGPRIBBONSLIDER_H__50A94B44_9870_4142_ABF2_4AE47D68EF16__INCLUDED_)
