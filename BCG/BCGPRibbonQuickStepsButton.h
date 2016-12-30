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
// BCGPRibbonQuickStepsButton.h: interface for the CBCGPRibbonQuickStepsButton class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPRIBBONQUICKSTEPSBUTTON_H__9DDE61A6_AA1F_49ED_AD9D_1F4806397D04__INCLUDED_)
#define AFX_BCGPRIBBONQUICKSTEPSBUTTON_H__9DDE61A6_AA1F_49ED_AD9D_1F4806397D04__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"

#ifndef BCGP_EXCLUDE_RIBBON

#include "BCGPRibbonPaletteButton.h"

/////////////////////////////////////////////////////////////////////////////////
// CBCGPRibbonQuickStep

class BCGCBPRODLLEXPORT CBCGPRibbonQuickStep : public CBCGPRibbonPaletteIcon
{
	friend class CBCGPRibbonQuickStepsButton;

	DECLARE_DYNCREATE(CBCGPRibbonQuickStep)

// Construction
public:
	CBCGPRibbonQuickStep(
		int			nImageIndex,
		LPCTSTR		lpszLabel,
		LPCTSTR		lpszDescription = NULL,
		BOOL		bIsEnabled = TRUE);

	virtual ~CBCGPRibbonQuickStep();

protected:
	CBCGPRibbonQuickStep();

// Attributes
public:
	void Enable(BOOL bEnable = TRUE);

	virtual BOOL IsChecked () const
	{
		return FALSE;
	}

	virtual CString GetToolTipText () const	{ return m_strText; }
	virtual CString GetDescription () const	{ return m_strDescription; }
	
	virtual CSize GetToolTipImageSize(int& nRibbonImageType) const;
	virtual void OnDrawTooltipImage(CDC* pDC, RibbonImageType type, const CRect& rectImage);

	virtual CSize GetKeyTipSize (CDC* /*pDC*/) { return CSize(0, 0); }

	virtual void CopyFrom(const CBCGPBaseRibbonElement& src);
};

/////////////////////////////////////////////////////////////////////////////////
// CBCGPRibbonQuickStepsButton

class BCGCBPRODLLEXPORT CBCGPRibbonQuickStepsButton : public CBCGPRibbonPaletteButton  
{
	DECLARE_DYNCREATE(CBCGPRibbonQuickStepsButton)

// Construction
public:
	CBCGPRibbonQuickStepsButton();

	CBCGPRibbonQuickStepsButton (
		UINT				nID,
		LPCTSTR				lpszText, 
		int					nSmallImageIndex,
		int					nLargeImageIndex,
		CBCGPToolBarImages& imagesPalette);
	
	CBCGPRibbonQuickStepsButton (
		UINT				nID,
		LPCTSTR				lpszText, 
		int					nSmallImageIndex,
		int					nLargeImageIndex,
		UINT				uiImagesPaletteResID = 0,
		int					cxPaletteImage = 16);

	virtual ~CBCGPRibbonQuickStepsButton();

protected:
	void CommonInit();

// Attributes
public:
	virtual BOOL IsButtonLook () const
	{
		return	m_bQuickAccessMode || m_bFloatyMode || m_bIsButtonMode || m_bIsCollapsed || m_bSearchResultMode ||
			m_pParentGroup != NULL || IsCustom();
	}

// Operations:
public:
	void AddQuickStep(CBCGPRibbonQuickStep* pQuickStep);

// Overrides:
protected:
	virtual void OnDrawPaletteIcon (CDC* pDC, CRect rectIcon, int nIconIndex, CBCGPRibbonPaletteIcon* pIcon, COLORREF clrText);
	virtual CSize GetItemSize() const;
	virtual void CopyFrom (const CBCGPBaseRibbonElement& src);
};

#endif // BCGP_EXCLUDE_RIBBON

#endif // !defined(AFX_BCGPRIBBONQUICKSTEPSBUTTON_H__9DDE61A6_AA1F_49ED_AD9D_1F4806397D04__INCLUDED_)
