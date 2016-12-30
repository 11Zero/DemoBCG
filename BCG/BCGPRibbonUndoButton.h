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
// BCGPRibbonUndoButton.h: interface for the CBCGPRibbonUndoButton class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPRIBBONUNDOBUTTON_H__C653CDA3_EB09_4BDD_8F20_419C827152C7__INCLUDED_)
#define AFX_BCGPRIBBONUNDOBUTTON_H__C653CDA3_EB09_4BDD_8F20_419C827152C7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGPRibbonPaletteButton.h"

#ifndef BCGP_EXCLUDE_RIBBON

class BCGCBPRODLLEXPORT CBCGPRibbonUndoButton : public CBCGPRibbonPaletteButton  
{
	DECLARE_DYNCREATE(CBCGPRibbonUndoButton)

// Construction
public:
	CBCGPRibbonUndoButton();

	CBCGPRibbonUndoButton (
		UINT	nID, 
		LPCTSTR lpszText, 
		int		nSmallImageIndex = -1,
		int		nLargeImageIndex = -1);

	CBCGPRibbonUndoButton (
		UINT	nID, 
		LPCTSTR lpszText, 
		HICON	hIcon);

	virtual ~CBCGPRibbonUndoButton();

// Attributes
public:
	virtual BOOL HasMenu () const
	{
		return TRUE;
	}

	int GetActionNumber () const
	{
		return m_nActionNumber;
	}

protected:
	CString			m_strCancel;
	CString			m_strUndoOne;
	CString			m_strUndoFmt;
	int				m_nActionNumber;
	CStringArray	m_arLabels;
	CSize			m_sizeMaxText;

// Overrides:
protected:
	virtual void CopyFrom (const CBCGPBaseRibbonElement& src);
	virtual void OnClick (CPoint point);
	virtual CSize GetIconSize () const;
	virtual void OnDrawPaletteIcon (CDC* pDC, CRect rectIcon, int nIconIndex, CBCGPRibbonPaletteIcon* pIcon, COLORREF clrText);
	virtual void NotifyHighlightListItem (int nIndex);
	virtual BOOL OnClickPaletteSubItem (CBCGPRibbonButton* pButton, CBCGPRibbonPanelMenuBar* pMenuBar);
	virtual void OnShowPopupMenu ();

	void CommonInit ();

// Operations
public:
	void AddUndoAction (LPCTSTR lpszLabel);
	void CleanUpUndoList ();
};

#endif // BCGP_EXCLUDE_RIBBON

#endif // !defined(AFX_BCGPRIBBONUNDOBUTTON_H__C653CDA3_EB09_4BDD_8F20_419C827152C7__INCLUDED_)
