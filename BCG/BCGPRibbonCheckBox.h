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
// BCGPRibbonCheckBox.h: interface for the CBCGPRibbonCheckBox class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPRIBBONCHECKBOX_H__25B7B9D5_DE79_4F40_9EB1_1031AF523290__INCLUDED_)
#define AFX_BCGPRIBBONCHECKBOX_H__25B7B9D5_DE79_4F40_9EB1_1031AF523290__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGPRibbonButton.h"

#ifndef BCGP_EXCLUDE_RIBBON

class BCGCBPRODLLEXPORT CBCGPRibbonCheckBox : public CBCGPRibbonButton  
{
	DECLARE_DYNCREATE(CBCGPRibbonCheckBox)

// Construction
public:
	CBCGPRibbonCheckBox ();

	CBCGPRibbonCheckBox (
		UINT	nID, 
		LPCTSTR lpszText);

	virtual ~CBCGPRibbonCheckBox();

// Overrides
public:
	virtual CSize GetIntermediateSize (CDC* pDC);

	virtual CSize GetCompactSize (CDC* pDC)
	{
		return GetIntermediateSize (pDC);
	}

	virtual CSize GetRegularSize (CDC* pDC)
	{
		return GetIntermediateSize (pDC);
	}

	virtual void OnDraw (CDC* pDC);
	virtual void OnDrawOnList (CDC* pDC, CString strText, int nTextOffset, CRect rect, BOOL bIsSelected, BOOL bHighlighted);

	virtual BOOL OnDrawMenuImage (CDC* /*pDC*/, CRect /*rect*/)	{	return FALSE;	}
	virtual BOOL IsDrawTooltipImage () const					{	return FALSE;	}

	virtual BOOL IsCustomIconAllowed() const					{	return FALSE;	}

protected:
	virtual void OnDrawMark (CDC* pDC, CRect rect);
	virtual void OnDrawMarkOnList (CDC* pDC, CRect rect, BOOL bIsSelected, BOOL bHighlighted);
	virtual BOOL SetACCData(CWnd* pParent, CBCGPAccessibilityData& data);
};


class BCGCBPRODLLEXPORT CBCGPRibbonRadioButton : public CBCGPRibbonCheckBox  
{
	DECLARE_DYNCREATE(CBCGPRibbonRadioButton)

// Construction
public:
	CBCGPRibbonRadioButton ();

	CBCGPRibbonRadioButton (
		UINT	nID, 
		LPCTSTR lpszText);

	virtual ~CBCGPRibbonRadioButton();

protected:
	virtual void OnDrawMark (CDC* pDC, CRect rect);
	virtual void OnDrawMarkOnList (CDC* pDC, CRect rect, BOOL bIsSelected, BOOL bHighlighted);
	virtual BOOL SetACCData(CWnd* pParent, CBCGPAccessibilityData& data);
};

#endif // BCGP_EXCLUDE_RIBBON

#endif // !defined(AFX_BCGPRIBBONCHECKBOX_H__25B7B9D5_DE79_4F40_9EB1_1031AF523290__INCLUDED_)
