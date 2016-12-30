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
// BCGPRibbonLabel.h: interface for the CBCGPRibbonLabel class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPRIBBONLABEL_H__E0F28D16_0073_4E84_A17B_5FAB11B2164A__INCLUDED_)
#define AFX_BCGPRIBBONLABEL_H__E0F28D16_0073_4E84_A17B_5FAB11B2164A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGPRibbonButton.h"

#ifndef BCGP_EXCLUDE_RIBBON

class BCGCBPRODLLEXPORT CBCGPRibbonLabel : public CBCGPRibbonButton
{
	friend class CBCGPRibbonPaletteButton;
	friend class CBCGPRibbonComboGalleryCtrl;

	DECLARE_DYNCREATE(CBCGPRibbonLabel)

// Construction:
public:
	CBCGPRibbonLabel (LPCTSTR lpszText, BOOL bIsMultiLine = FALSE);
	virtual ~CBCGPRibbonLabel();

protected:
	CBCGPRibbonLabel();

// Overrides
protected:
	virtual void OnDraw (CDC* pDC);
	virtual void OnCalcTextSize (CDC* pDC);
	virtual void OnLButtonUp (CPoint /*point*/) {}
	virtual CSize GetIntermediateSize (CDC* pDC);
	
	virtual CSize GetRegularSize (CDC* pDC)
	{
		return GetIntermediateSize (pDC);
	}

	virtual CSize GetCompactSize (CDC* pDC)
	{
		return GetIntermediateSize (pDC);
	}

	virtual BOOL IsTabStop () const
	{
		return FALSE;
	}

	virtual BOOL CanBeAddedToQAT () const
	{
		return FALSE;
	}

	virtual int AddToListBox (CBCGPRibbonCommandsListBox* /*pWndListBox*/, BOOL /*bDeep*/)
	{
		return -1;
	}

	virtual CBCGPGridRow* AddToTree (CBCGPGridCtrl* /*pGrid*/, CBCGPGridRow* /*pParent*/)
	{
		return NULL;
	}

	virtual void GetElementsByName (LPCTSTR /*lpszName*/, 
		CArray<CBCGPBaseRibbonElement*, CBCGPBaseRibbonElement*>&, DWORD /*dwFlags*/ = 0)	{}

	virtual BOOL SetACCData(CWnd* pParent, CBCGPAccessibilityData& data);
	virtual void CopyFrom (const CBCGPBaseRibbonElement& src);

// Attributes:
protected:
	BOOL	m_bIsPaletteGroup;
};

#endif

#endif // !defined(AFX_BCGPRIBBONLABEL_H__E0F28D16_0073_4E84_A17B_5FAB11B2164A__INCLUDED_)
