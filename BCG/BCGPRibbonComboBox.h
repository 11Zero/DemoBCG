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
// BCGPRibbonComboBox.h: interface for the CBCGPRibbonComboBox class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPRIBBONCOMBOBOX_H__27A9CEFE_1C67_4345_B4EB_DD1E3FA5AD19__INCLUDED_)
#define AFX_BCGPRIBBONCOMBOBOX_H__27A9CEFE_1C67_4345_B4EB_DD1E3FA5AD19__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef BCGP_EXCLUDE_RIBBON

#include "BCGPRibbonEdit.h"
#include "BCGPToolbarFontCombo.h"
#include "BCGPToolBarImages.h"

class CBCGPRibbonEditCtrl;
class CBCGPCalculatorPopup;

/////////////////////////////////////////////////////////////////////////////
// CBCGPRibbonComboBox

class BCGCBPRODLLEXPORT CBCGPRibbonComboBox : public CBCGPRibbonEdit  
{
	friend class CBCGPRibbonEditCtrl;
	friend class CBCGPDropDownList;
	friend class CBCGPRibbonCollector;
	friend class CBCGPRibbonConstructor;
	friend class CBCGPCalculator;

	DECLARE_DYNCREATE(CBCGPRibbonComboBox)

// Construction:
public:
	enum BCGP_RIBBON_COMBO_SORT_ORDER
	{
		BCGP_RIBBON_COMBO_SORT_ORDER_NO_SORT	= 0,
		BCGP_RIBBON_COMBO_SORT_ORDER_ASC		= 1,
		BCGP_RIBBON_COMBO_SORT_ORDER_DESC		= 2,
	};

	CBCGPRibbonComboBox (UINT nID, BOOL bHasEditBox = TRUE, int nWidth = -1,
		LPCTSTR lpszLabel = NULL, int nImage = -1, BCGP_RIBBON_COMBO_SORT_ORDER sortOrder = BCGP_RIBBON_COMBO_SORT_ORDER_NO_SORT);

	virtual ~CBCGPRibbonComboBox();

protected:
	CBCGPRibbonComboBox();

// Attributes:
public:
	void EnableDropDownListResize (BOOL bEnable = FALSE)
	{
		m_bResizeDropDownList = bEnable;
	}

	BOOL IsResizeDropDownList () const
	{
		return m_bResizeDropDownList;
	}

	void EnableCalculator(BOOL bEnable = TRUE, const CStringList* plstAdditionalCommands = NULL,
		const CList<UINT, UINT>* plstExtCommands = NULL, LPCTSTR lpszDisplayFormat = NULL);
	BOOL IsCalculatorEnabled() const
	{
		return m_bIsCalculator;
	}

	void EnableAutoComplete(BOOL bEnable);
	BOOL IsAutoCompleteEnabled() const
	{
		return m_bIsAutoComplete;
	}

// Operations:
public:
	virtual INT_PTR AddItem (LPCTSTR lpszItem, DWORD_PTR dwData = 0);
	virtual INT_PTR InsertItem (int nPos, LPCTSTR lpszItem, DWORD_PTR dwData = 0);

	INT_PTR GetCount () const
	{
		return m_lstItems.GetCount ();
	}

	LPCTSTR GetItem (int iIndex) const;
	DWORD_PTR GetItemData (int iIndex) const;

	int GetCurSel () const
	{
		return m_iSelIndex;
	}

	void RemoveAllItems ();

	BOOL SelectItem (int iIndex);
	BOOL SelectItem (DWORD_PTR dwData);
	BOOL SelectItem (LPCTSTR lpszText);

	BOOL DeleteItem (int iIndex);
	BOOL DeleteItem (DWORD_PTR dwData);
	BOOL DeleteItem (LPCTSTR lpszText);

	int FindItem (LPCTSTR lpszText) const;

	int GetDropDownHeight () const
	{
		return m_nDropDownHeight;
	}
	void SetDropDownHeight (int nHeight);

	BOOL HasEditBox () const
	{
		return m_bHasEditBox;
	}

	BCGP_RIBBON_COMBO_SORT_ORDER GetSortOrder() const
	{ 
		return m_sortOrder;
	}

// Overrides
public:
	virtual CSize GetIntermediateSize (CDC* pDC);
	virtual void OnSelectItem (int nItem);

	virtual void OnCalculatorUserCommand (CBCGPCalculator* /*pCalculator*/, UINT /*uiCmd*/)
	{
		ASSERT (FALSE);	// Must be implemented in derived class
	}

	virtual int GetDropDownImageWidth () const;

protected:
	virtual void OnDraw (CDC* pDC);

	virtual void OnLButtonDown (CPoint point);
	virtual void OnLButtonUp (CPoint point);

	virtual void CopyFrom (const CBCGPBaseRibbonElement& src);
	virtual BOOL HasMenu () const
	{
		return TRUE;
	}

	virtual BOOL CanBeStretched ()
	{
		return FALSE;
	}

	virtual void OnAfterChangeRect (CDC* pDC);

	virtual void DropDownList ();
	void CommonInit ();

	virtual void ClearData ();

	virtual BOOL OnDrawDropListItem (CDC* pDC, int nIndex, CBCGPToolbarMenuButton* pItem, 
									BOOL bHighlight);
	virtual CSize OnGetDropListItemSize (CDC* pDC, int nIndex, CBCGPToolbarMenuButton* pItem,
									CSize sizeDefault);

	virtual int FindItemByPrefix(LPCTSTR lpszPrefix, CString& strResult) const;
	virtual BOOL SetACCData(CWnd* pParent, CBCGPAccessibilityData& data);

// Attributes:
protected:
	CStringList						m_lstItems;
	CList<DWORD_PTR, DWORD_PTR>		m_lstItemData;
	int								m_iSelIndex;
	
	BOOL							m_bHasEditBox;
	int								m_nDropDownHeight;

	BOOL							m_bResizeDropDownList;

	BOOL							m_bIsCalculator;
	CBCGPCalculatorPopup*			m_pCalcPopup;
	CStringList						m_lstCalcAdditionalCommands;
	CList<UINT, UINT>				m_lstCalcExtCommands;
	CString							m_strCalcDisplayFormat;

	CBCGPToolBarImages				m_ButtonImages;
	BCGP_RIBBON_COMBO_SORT_ORDER	m_sortOrder;
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPRibbonFontComboBox

class BCGCBPRODLLEXPORT CBCGPRibbonFontComboBox : public CBCGPRibbonComboBox  
{
	DECLARE_DYNCREATE(CBCGPRibbonFontComboBox)

// Construction:
public:
	CBCGPRibbonFontComboBox (
			UINT nID, 
			int nFontType = DEVICE_FONTTYPE | RASTER_FONTTYPE | TRUETYPE_FONTTYPE,
			BYTE nCharSet = DEFAULT_CHARSET,
			BYTE nPitchAndFamily = DEFAULT_PITCH,
			int nWidth = -1);

	virtual ~CBCGPRibbonFontComboBox();

protected:
	CBCGPRibbonFontComboBox ();

// Operations:
public:
	BOOL SetFont (LPCTSTR lpszName, BYTE nCharSet = DEFAULT_CHARSET, BOOL bExact = FALSE);
	const CBCGPFontDesc* GetFontDesc (int iIndex = -1) const
	{
		return (CBCGPFontDesc*) GetItemData (iIndex);
	}

	void BuildFonts (
			int nFontType = DEVICE_FONTTYPE | RASTER_FONTTYPE | TRUETYPE_FONTTYPE,
			BYTE nCharSet = DEFAULT_CHARSET,
			BYTE nPitchAndFamily = DEFAULT_PITCH);
	void RebuildFonts ();

	int GetFontType () const
	{
		return m_nFontType;
	}
	BYTE GetCharSet () const
	{
		return m_nCharSet;
	}
	BYTE GetPitchAndFamily () const
	{
		return m_nPitchAndFamily;
	}

protected:
	int GetFontsCount (LPCTSTR lpszName, const CObList& lstFonts);

// Overrides
protected:
	virtual void CopyFrom (const CBCGPBaseRibbonElement& src);
	virtual void ClearData ();
	virtual void DropDownList ();

	virtual BOOL OnDrawDropListItem (CDC* pDC, int nIndex, CBCGPToolbarMenuButton* pItem, 
									BOOL bHighlight);
	virtual CSize OnGetDropListItemSize (CDC* pDC, int nIndex, CBCGPToolbarMenuButton* pItem,
									CSize sizeDefault);
// Attributes:
public:
	static BOOL	m_bDrawUsingFont;

protected:
	int					m_nFontType;
	BYTE				m_nCharSet;
	BYTE				m_nPitchAndFamily;

	CBCGPToolBarImages	m_Images;
};

#endif // BCGP_EXCLUDE_RIBBON

#endif // !defined(AFX_BCGPRIBBONCOMBOBOX_H__27A9CEFE_1C67_4345_B4EB_DD1E3FA5AD19__INCLUDED_)
