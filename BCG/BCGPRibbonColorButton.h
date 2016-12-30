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
// BCGPRibbonColorButton.h: interface for the CBCGPRibbonColorButton class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPRIBBONCOLORBUTTON_H__C89F027E_3034_4699_BA86_BC4B601F1681__INCLUDED_)
#define AFX_BCGPRIBBONCOLORBUTTON_H__C89F027E_3034_4699_BA86_BC4B601F1681__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "bcgcbpro.h"

#ifndef BCGP_EXCLUDE_RIBBON

#include "BCGPColorBar.h"
#include "BCGPRibbonPaletteButton.h"

class BCGCBPRODLLEXPORT CBCGPRibbonColorButton : public CBCGPRibbonPaletteButton  
{
	friend class CBCGPColorBar;
	friend class CBCGPRibbonColorMenuButton;
	friend class CBCGPColorButton;
	friend class CBCGPRibbonCollector;
	friend class CBCGPRibbonConstructor;

	DECLARE_DYNCREATE(CBCGPRibbonColorButton)

// Construction:
public:
	CBCGPRibbonColorButton ();

	CBCGPRibbonColorButton (
		UINT		nID,
		LPCTSTR		lpszText,
		int			nSmallImageIndex,
		COLORREF	color = RGB (0, 0, 0));
	
	CBCGPRibbonColorButton (
		UINT		nID,
		LPCTSTR		lpszText,
		BOOL		bSimpleButtonLook,
		int			nSmallImageIndex,
		int			nLargeImageIndex,
		COLORREF	color = RGB (0, 0, 0));

	virtual ~CBCGPRibbonColorButton();

protected:
	void CommonInit ();

// Attributes:
public:
	void EnableAutomaticButton (LPCTSTR lpszLabel, COLORREF colorAutomatic, 
		BOOL bEnable = TRUE, LPCTSTR lpszToolTip = NULL, BOOL bOnTop = TRUE, BOOL bDrawBorder = FALSE);
	void EnableOtherButton (LPCTSTR lpszLabel, LPCTSTR lpszToolTip = NULL);
	void EnableDocumentColors (LPCTSTR lpszLabel);
	
	void SetDocumentColors (LPCTSTR lpszLabel, CList<COLORREF,COLORREF>& lstColors);

	static void SetColorName (COLORREF color, const CString& strName)
	{
		CBCGPColorBar::SetColorName (color, strName);
	}

	COLORREF GetAutomaticColor () const
	{
		return m_ColorAutomatic;
	}

	void SetColor (COLORREF color)
	{
		m_Color = color;
	}

	COLORREF GetColor () const
	{
		return m_Color;
	}

	void SetPalette (CPalette* pPalette);

	int	GetColumns () const
	{
		return m_nIconsInRow;
	}

	void SetColumns (int nColumns)
	{
		m_nPanelColumns = m_nIconsInRow = nColumns;
	}

	COLORREF GetHighlightedColor () const;

	void SetColorBoxSize (CSize sizeBox);
	CSize GetColorBoxSize () const
	{
		return m_sizeBox;
	}

	BOOL IsSimpleButtonLook () const
	{
		return m_bSimpleButtonLook;
	}

protected:
	virtual BOOL HasMenu () const
	{
		return TRUE;
	}

// Opeartions:
public:
	void AddColorsGroup (	LPCTSTR lpszName, 
							const CList<COLORREF,COLORREF>& lstColors,
							BOOL bContiguousColumns = FALSE);
	void RemoveAllColorGroups ();

	void UpdateColor (COLORREF color);

protected:
	COLORREF GetColorByIndex (int nIconIndex) const;

// Overrides
protected:
	virtual void OnDrawPaletteIcon (CDC* pDC, CRect rectIcon, int nIconIndex, CBCGPRibbonPaletteIcon* pIcon, COLORREF clrText);
	virtual void DrawImage (CDC* pDC, RibbonImageType type, CRect rectImage);
	virtual void CopyFrom (const CBCGPBaseRibbonElement& src);
	virtual void OnShowPopupMenu ();
	virtual BOOL OnClickPaletteSubItem (CBCGPRibbonButton* pButton, CBCGPRibbonPanelMenuBar* pMenuBar);
	virtual void OnClickPaletteIcon (CBCGPRibbonPaletteIcon* pIcon);
	virtual void NotifyHighlightListItem (int nIndex);
	virtual CString GetIconToolTip (const CBCGPRibbonPaletteIcon* pIcon) const;

	virtual int GetGroupOffset () const
	{
		return 2;
	}

	COLORREF					m_Color;	// Currently selected color
	COLORREF					m_ColorHighlighted;
	COLORREF					m_ColorAutomatic;

	CArray<COLORREF, COLORREF>	m_Colors;
	CArray<COLORREF, COLORREF>	m_DocumentColors;
	BOOL						m_bTemporaryDocColors;

	BOOL						m_bIsAutomaticButton;
	BOOL						m_bIsAutomaticButtonOnTop;
	BOOL						m_bIsAutomaticButtonBorder;
	BOOL						m_bIsOtherButton;

	CString						m_strAutomaticButtonLabel;
	CString						m_strAutomaticButtonToolTip;
	CString						m_strOtherButtonLabel;
	CString						m_strOtherButtonToolTip;
	CString						m_strDocumentColorsLabel;

	CBCGPRibbonButton*			m_pOtherButton;
	CBCGPRibbonButton*			m_pAutoButton;

	BOOL						m_bHasGroups;
	BOOL						m_bSimpleButtonLook;

	CSize						m_sizeBox;
	CArray<LPARAM, LPARAM>		m_arContColumnsRanges;
};

#endif // BCGP_EXCLUDE_RIBBON

#endif // !defined(AFX_BCGPRIBBONCOLORBUTTON_H__C89F027E_3034_4699_BA86_BC4B601F1681__INCLUDED_)
