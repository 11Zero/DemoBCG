#if !defined(AFX_BCGPPROPLIST_H__F6054FED_0317_4829_B7BF_4EBBDC27DF01__INCLUDED_)
#define AFX_BCGPPROPLIST_H__F6054FED_0317_4829_B7BF_4EBBDC27DF01__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//*******************************************************************************
// COPYRIGHT NOTES
// ---------------
// This is a part of the BCGControlBar Library
// Copyright (C) 1998-2014 BCGSoft Ltd.
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.
//*******************************************************************************
//
// BCGPPropList.h : header file
//

#ifndef __AFXTEMPL_H__
	#include "afxtempl.h"
#endif

#include "comdef.h"
#include "BCGCBPro.h"
#include "bcgprores.h"

#ifndef BCGP_EXCLUDE_PROP_LIST

#include "BCGGlobals.h"
#include "ColorPopup.h"
#include "BCGPToolBar.h"
#include "BCGPHeaderCtrl.h"
#include "BCGPEdit.h"
#define PROP_HEADER_CTRL_CLASS	CBCGPHeaderCtrl
#include "BCGPDateTimeCtrl.h"
#include "BCGPInplaceToolTipCtrl.h"
#include "BCGPScrollBar.h"
#include "BCGPGraphicsManager.h"
#include "BCGPEditBrushDlg.h"
#include "BCGPTextFormatDlg.h"
#include "BCGPToolBarImages.h"

/////////////////////////////////////////////////////////////////////////////
// CBCGPProp object

class BCGCBPRODLLEXPORT CBCGPProp : public CObject
{
	DECLARE_DYNAMIC(CBCGPProp)

	friend class CBCGPPropList;

// Construction
public:
	// Group constructor
	CBCGPProp(const CString& strGroupName, DWORD_PTR dwData = 0,
		BOOL bIsValueList = FALSE);

	// Simple property
	CBCGPProp(const CString& strName, const _variant_t& varValue, 
		LPCTSTR lpszDescr = NULL, DWORD_PTR dwData = 0,
		LPCTSTR lpszEditMask = NULL, LPCTSTR lpszEditTemplate = NULL,
		LPCTSTR lpszValidChars = NULL);

	CBCGPProp(const CString& strName, UINT nID, const _variant_t& varValue, 
		LPCTSTR lpszDescr = NULL, DWORD_PTR dwData = 0,
		LPCTSTR lpszEditMask = NULL, LPCTSTR lpszEditTemplate = NULL,
		LPCTSTR lpszValidChars = NULL);

	virtual ~CBCGPProp();

	enum ClickArea
	{
		ClickExpandBox,
		ClickName,
		ClickValue,
		ClickDescription,
		ClickMenuButton,
		ClickGroupArea,
	};

// Operations:
public:
	int GetExpandedSubItems (BOOL bIncludeHidden = TRUE) const;
	BOOL AddSubItem (CBCGPProp* pProp);
	BOOL RemoveSubItem (CBCGPProp*& pProp, BOOL bDelete = TRUE);
	void RemoveAllSubItems();

	BOOL AddOption (LPCTSTR lpszOption, BOOL bInsertUnique = TRUE);
	void RemoveAllOptions ();

	int GetOptionCount () const;
	LPCTSTR GetOption (int nIndex) const;

	BOOL SelectOption(int nIndex);
	int GetSelectedOption() const;

	virtual CBCGPProp* HitTest (CPoint point, CBCGPProp::ClickArea* pnArea = NULL);

	void Expand (BOOL bExpand = TRUE);
	void Redraw ();
	void RedrawButton ();
	void RedrawMenuButton ();
	void RedrawState();

	void EnableSpinControl (BOOL bEnable = TRUE, int nMin = 0, int nMax = 0);

	virtual void ResetOriginalValue ();
	virtual void CommitModifiedValue ();

	void Show (BOOL bShow = TRUE, BOOL bAdjustLayout = TRUE);

	void SetState(LPCTSTR lpszToolTip, TCHAR cIndicator = _T('!'), COLORREF clrState = RGB(255, 0, 0), BOOL bDrawBorder = TRUE, int nStateWidth = 0 /* Default is a row height */);
	void CleanState();

protected:
	void Init ();
	void SetFlags ();
	void SetOwnerList (CBCGPPropList* pWndList);
	void Repos (int& y);
	void AddTerminalProp (CList<CBCGPProp*, CBCGPProp*>& lstProps);

	BOOL IsSubItem (CBCGPProp* pProp) const;

	void ExpandDeep (BOOL bExpand = TRUE);
	void SetModifiedFlag ();

// Overrides
public:
	virtual void OnDrawName (CDC* pDC, CRect rect);
	virtual void OnDrawMenuButton(CDC* pDC, CRect rect);
	virtual void OnDrawStateIndicator(CDC* pDC, CRect rect);
	virtual void OnDrawValue (CDC* pDC, CRect rect);
	virtual void OnDrawExpandBox (CDC* pDC, CRect rectExpand);
	virtual void OnDrawButton (CDC* pDC, CRect rectButton);
	virtual void OnDrawDescription (CDC* pDC, CRect rect);

	virtual CString FormatProperty ();

	virtual BOOL OnUpdateValue ();
	virtual BOOL OnEdit (LPPOINT lptClick);
	virtual CWnd* CreateInPlaceEdit (CRect rectEdit, BOOL& bDefaultFormat);
	virtual CSpinButtonCtrl* CreateSpinControl (CRect rectSpin);

	virtual BOOL OnEndEdit ();

	virtual void OnClickButton (CPoint point);
	virtual BOOL OnClickValue (UINT uiMsg, CPoint point);
	virtual BOOL OnDblClick (CPoint point);

	virtual void OnSelectCombo ();
	virtual void OnCloseCombo();
	virtual void OnSetSelectedComboItem();

	virtual BOOL OnSetCursor () const;
	virtual BOOL PushChar (UINT nChar);

	virtual CString GetNameTooltip ();
	virtual CString GetValueTooltip ();

	virtual void OnClickGroupArea(CPoint /*point*/) {}
	virtual void OnRClickGroupArea(CPoint /*point*/) {}

	virtual void OnClickName (CPoint /*point*/) {}
	virtual void OnRClickName (CPoint /*point*/) {}
	
	virtual void OnClickMenuButton (CPoint point);
	virtual void OnRClickMenuButton (CPoint /*point*/) {}

	virtual void OnRClickValue (CPoint /*point*/, BOOL /*bSelChanged*/) {}

	virtual void OnPosSizeChanged (CRect /*rectOld*/) {}

	virtual void OnSetSelection (CBCGPProp* /*pOldSel*/) {}
	virtual void OnKillSelection (CBCGPProp* /*pNewSel*/) {}

	virtual void AdjustButtonRect ();
	virtual void AdjustInPlaceEditRect (CRect& rectEdit, CRect& rectSpin);

	virtual BOOL SetACCData (CWnd* pParent, CBCGPAccessibilityData& data);

	virtual BOOL NoInplaceEdit() const { return FALSE; }
	virtual BOOL IsButtonVisible() const;

	virtual BOOL IsCopyAvailable() const;
	virtual BOOL DoCopy();

	virtual BOOL IsPasteAvailable();
	virtual BOOL DoPaste();

	virtual BOOL IsEditAvailable();
	virtual BOOL DoEdit();

	virtual BOOL SerializeToBuffer(CString& str);
	virtual BOOL SerializeFromBuffer(const CString& str, BOOL bCheckOnly);

	virtual BOOL SerializeValue(CString& str);
	virtual BOOL ParseValue(const CString& str);

	virtual void OnMenuCustomItem(int nIndex);
	virtual UINT OnGetMenuCustomItemState(int nIndex);	// MF_GRAYED, MF_CHECKED

	virtual CBCGPProp* FindSubItemByData (DWORD_PTR dwData) const;
	virtual CBCGPProp* FindSubItemByID (UINT nID) const;

protected:
	virtual HBRUSH OnCtlColor(CDC* pDC, UINT nCtlColor);
	virtual CComboBox* CreateCombo (CWnd* pWndParent, CRect rect);
	virtual void OnDestroyWindow ();

	virtual BOOL OnKillFocus (CWnd* /*pNewWnd*/)
	{
		return TRUE;
	}

	virtual BOOL OnEditKillFocus ()
	{
		return TRUE;
	}

	virtual BOOL HasButton () const;

	virtual BOOL IsProcessFirstClick () const
	{
		return TRUE;
	}

	virtual BOOL HasValueField () const
	{
		return TRUE;
	}

	virtual BOOL TextToVar (const CString& strText);
	virtual BOOL IsValueChanged () const;

	virtual BOOL OnActivateByTab ();
	virtual BOOL OnRotateListValue (BOOL bForward);

	virtual BOOL OnCommand (WPARAM /*wParam*/)
	{
		return FALSE;
	}

	virtual void OnLeaveMouse();

	CLIPFORMAT GetClipboardFormat();

	BOOL PasteInternal(BOOL bCheckOnly);

	virtual void SetFilter(const CString& strFilter);

// Attributes
public:
	UINT GetID () const
	{
		return m_nID;
	}

	void SetID (UINT nID)
	{
		m_nID = nID;
	}

	LPCTSTR GetName () const
	{
		return m_strName;
	}

	void SetName (LPCTSTR lpszName, BOOL bRedraw = TRUE);

	virtual const _variant_t& GetValue () const
	{
		return m_varValue;
	}

	virtual void SetValue (const _variant_t& varValue);

	const _variant_t& GetOriginalValue () const
	{
		return m_varValueOrig;
	}

	void SetOriginalValue (const _variant_t& varValue);

	const CString& GetDescription () const
	{
		return m_strDescr;
	}

	void SetDescription (const CString& strDescr)
	{
		m_strDescr = strDescr;
	}

	DWORD_PTR GetData () const
	{
		return m_dwData;
	}

	void SetData (DWORD_PTR dwData)
	{
		m_dwData = dwData;
	}

	BOOL IsGroup () const
	{
		return m_bGroup;
	}

	BOOL IsExpanded () const
	{
		return m_bExpanded;
	}

	BOOL IsParentExpanded () const;

	virtual BOOL IsSelected () const;
	int GetHierarchyLevel () const;

	void Enable (BOOL bEnable = TRUE);
	BOOL IsEnabled () const
	{
		return m_bEnabled;
	}

	void AllowEdit (BOOL bAllow = TRUE)
	{
		ASSERT (m_varValue.vt != VT_BOOL);
		m_bAllowEdit = bAllow;
	}

	BOOL IsAllowEdit () const
	{
		return m_bAllowEdit;
	}

	CRect GetRect () const
	{
		return m_Rect;
	}

	int GetSubItemsCount () const
	{
		return (int) m_lstSubItems.GetCount ();
	}

	CBCGPProp* GetSubItem (int nIndex) const;

	CBCGPProp* GetParent () const
	{
		return m_pParent;
	}

	BOOL IsInPlaceEditing () const
	{
		return m_bInPlaceEdit;
	}

	BOOL IsModified () const
	{
		return m_bIsModified;
	}

	BOOL IsVisible () const
	{
		return m_bIsVisible;
	}

	BOOL IsVisibleInFilter () const
	{
		return m_bIsVisible && m_bInFilter;
	}

	void SetDropDownWidth (int nWidth)	// -1 - default
	{
		m_nDropDownWidth = nWidth;
	}

	const CString& GetStateToolTip() const { return m_strStateToolTip; }
	TCHAR GetStateIndicator() const { return m_cStateIndicator; }
	COLORREF GetStateColor() const { return m_clrState; }
	BOOL IsDrawStateColorBorder() const { return m_bDrawStateBorder; }

public:
	// Data output formats
	static CString	m_strFormatChar;
	static CString	m_strFormatShort;
	static CString	m_strFormatLong;
	static CString	m_strFormatUShort;
	static CString	m_strFormatULong;
	static CString	m_strFormatFloat;
	static CString	m_strFormatDouble;
	// Data input formats
	static CString	m_strScanFormatFloat;
	static CString	m_strScanFormatDouble;

protected:
	UINT			m_nID;			// Property ID
	CString			m_strName;		// Property name
	_variant_t		m_varValue;		// Property value
	_variant_t		m_varValueOrig;	// Property original value
	CBCGPPropList*	m_pWndList;		// Pointer to the PropertyList window
	DWORD_PTR		m_dwData;		// User-defined data
	CString			m_strDescr;		// Property description
	CString			m_strEditMask;	// Property edit mask (see CBCGPMaskEdit for description)
	CString			m_strEditTempl;	// Property edit template (see CBCGPMaskEdit for description)
	CString			m_strValidChars;// Property edit valid chars (see CBCGPMaskEdit for description)
	CString			m_strButtonText;// Purch button text (default - "...")

	CStringList		m_lstOptions;	// List of combobox items

	BOOL			m_bInPlaceEdit;	// Is in InPalce editing mode

	CWnd*			m_pWndInPlace;	// Pointer to InPlace editing window
	CComboBox*		m_pWndCombo;	// Pointer to combbox
	CSpinButtonCtrl*	m_pWndSpin;		// Pointer to spin button

	CRect			m_Rect;			// Property rectangle (in the prop.list coordinates)
	CRect			m_rectButton;	// Drop down/open button rectangle
	BOOL			m_bButtonIsDown;// Is button pressed?
	BOOL			m_bButtonIsFocused;	// Is button focused?
	BOOL			m_bButtonIsHighlighted;	// Is button highlighted?
	CRect			m_rectMenuButton;	// Menu button rectangle
	BOOL			m_bMenuButtonIsHighlighted;	// Is menu button highlighted?
	
	BOOL			m_bGroup;		// Is property group?
	BOOL			m_bExpanded;	// Is property expanded (for groups only)
	BOOL			m_bEnabled;		// Is propperty enabled?
	BOOL			m_bAllowEdit;	// Is property editable?
	BOOL			m_bIsValueList;	// This is a value list group?
	DWORD			m_dwFlags;		// Property flags

	CBCGPProp*		m_pParent;		// Parent property (NULL for top-level properties)
	CList<CBCGPProp*, CBCGPProp*>	m_lstSubItems;	// Sub-properies list

	BOOL			m_bNameIsTrancated;
	BOOL			m_bValueIsTrancated;

	int				m_nMinValue;
	int				m_nMaxValue;

	BOOL			m_bIsModified;	// Is property modified
	BOOL			m_bIsVisible;	// Is property visible
	BOOL			m_bInFilter;	// Is property name matched to filter?

	// State attributes:
	BOOL			m_bHasState;		// TRUE if property has a state; FALSE - otherwise
	CRect			m_rectState;		// State indicator rectangle
	int				m_nStateWidth;		// Specified state box width (default - row height)
	CString			m_strStateToolTip;	// State tooltip text
	TCHAR			m_cStateIndicator;	// State indicator character ('!' by default)
	COLORREF		m_clrState;			// State indicator color (red by default)
	BOOL			m_bDrawStateBorder;	// TRUE if draw border around edit box; FALSE - otherwise

	int				m_nDropDownWidth;
	BOOL			m_bDrawMenuButton;

	static CLIPFORMAT	m_cFormat;
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPColorProp object

class BCGCBPRODLLEXPORT CBCGPColorProp : public CBCGPProp
{
	friend class CBCGPPropList;

	DECLARE_DYNAMIC(CBCGPColorProp)

// Construction
public:
	CBCGPColorProp(const CString& strName, const COLORREF& color, 
		CPalette* pPalette = NULL, LPCTSTR lpszDescr = NULL, DWORD_PTR dwData = 0, BOOL bUseD2DColors = TRUE);

	CBCGPColorProp(const CString& strName, UINT nID, const COLORREF& color, 
		CPalette* pPalette = NULL, LPCTSTR lpszDescr = NULL, DWORD_PTR dwData = 0, BOOL bUseD2DColors = TRUE);

	virtual ~CBCGPColorProp();

// Overrides
public:
	virtual void OnDrawValue (CDC* pDC, CRect rect);
	virtual void OnClickButton (CPoint point);
	virtual BOOL OnEdit (LPPOINT lptClick);
	virtual BOOL OnUpdateValue ();
	virtual CString FormatProperty ();
	virtual BOOL ParseValue(const CString& str);

protected:
	virtual BOOL OnKillFocus (CWnd* pNewWnd)
	{
		return pNewWnd->GetSafeHwnd () != m_pPopup->GetSafeHwnd ();
	}

	virtual BOOL OnEditKillFocus ()
	{
		return m_pPopup == NULL;
	}

	virtual BOOL IsValueChanged () const
	{
		return m_Color != m_ColorOrig;
	}

	virtual void AdjustInPlaceEditRect (CRect& rectEdit, CRect& rectSpin);
	virtual void ResetOriginalValue ();
	virtual void CommitModifiedValue ();

	virtual BOOL IsCopyAvailable() const
	{
		return TRUE;
	}

// Attributes
public:
	COLORREF GetColor () const
	{
		return m_Color;
	}

	void SetColor (COLORREF color);

	// Color popup attributes:
	void EnableAutomaticButton (LPCTSTR lpszLabel, COLORREF colorAutomatic, BOOL bEnable = TRUE);
	void EnableOtherButton (LPCTSTR lpszLabel, BOOL bAltColorDlg = TRUE, BOOL bEnable = TRUE);
	void SetColors(const CArray<COLORREF, COLORREF>& colors);
	void SetColumnsNumber (int nColumnsNumber);

// Attributes
protected:
	COLORREF					m_Color;			// Color value
	COLORREF					m_ColorOrig;		// Original color value
	COLORREF					m_ColorAutomatic;	// Automatic (default) color value
	CString						m_strAutoColor;		// Atomatic color label
	BOOL						m_bStdColorDlg;		// Use standard Windows color dialog
	CString						m_strOtherColor;	// Alternative color label
	CColorPopup*				m_pPopup;
	CArray<COLORREF, COLORREF>	m_Colors;
	int							m_nColumnsNumber;	// Number of columns in dropped-down colors list
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPBrushProp object

class BCGCBPRODLLEXPORT CBCGPBrushProp : public CBCGPProp
{
	friend class CBCGPPropList;

	DECLARE_DYNAMIC(CBCGPBrushProp)

// Construction
public:
	CBCGPBrushProp(const CString& strName, const CBCGPBrush& brush, CBCGPEditBrushOptions* pOptions = NULL,
		LPCTSTR lpszDescr = NULL, DWORD_PTR dwData = 0);

	CBCGPBrushProp(const CString& strName, UINT nID, const CBCGPBrush& brush, CBCGPEditBrushOptions* pOptions = NULL,
		LPCTSTR lpszDescr = NULL, DWORD_PTR dwData = 0);

	virtual ~CBCGPBrushProp();

	void SetOptions(CBCGPEditBrushOptions& options)
	{
		m_Options = options;
	}

// Overrides
public:
	virtual void OnDrawValue (CDC* pDC, CRect rect);
	virtual void OnClickButton (CPoint point);
	virtual CString FormatProperty ();
	virtual BOOL NoInplaceEdit() const { return TRUE; }

protected:
	virtual BOOL IsValueChanged () const
	{
		return !m_Brush.CompareWith(m_BrushOrig);
	}

	virtual void ResetOriginalValue ();
	virtual void CommitModifiedValue ();

	virtual BOOL IsCopyAvailable() const
	{
		return TRUE;
	}

	virtual BOOL SerializeValue(CString& str);
	virtual BOOL ParseValue(const CString& str);
	virtual BOOL DoEdit();

// Attributes
public:
	CBCGPBrush& GetBrush()
	{
		return m_Brush;
	}

	void SetBrush (const CBCGPBrush& brush);

// Attributes
protected:
	CBCGPBrush				m_Brush;		// Brush value
	CBCGPBrush				m_BrushOrig;	// Original brush value
	HBITMAP					m_hbmp;			// Cached image
	CBCGPEditBrushOptions	m_Options;		// Property and dialog options
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPTextFormatProp object

class BCGCBPRODLLEXPORT CBCGPTextFormatProp : public CBCGPProp
{
	friend class CBCGPPropList;

	DECLARE_DYNAMIC(CBCGPTextFormatProp)

// Construction
public:
	CBCGPTextFormatProp(const CString& strName, const CBCGPTextFormat& tf,
		CBCGPTextFormatDlgOptions* pOptions = NULL,
		LPCTSTR lpszDescr = NULL, DWORD_PTR dwData = 0);

	CBCGPTextFormatProp(const CString& strName, UINT nID, const CBCGPTextFormat& tf,
		CBCGPTextFormatDlgOptions* pOptions = NULL,
		LPCTSTR lpszDescr = NULL, DWORD_PTR dwData = 0);

	virtual ~CBCGPTextFormatProp();

	void SetOptions(CBCGPTextFormatDlgOptions& options)
	{
		m_Options = options;
	}

// Overrides
public:
	virtual void OnClickButton (CPoint point);
	virtual CString FormatProperty ();

protected:
	virtual BOOL IsValueChanged () const
	{
		return !m_TextFormat.CompareWith(m_TextFormatOrig);
	}

	virtual void ResetOriginalValue ();
	virtual void CommitModifiedValue ();

	virtual BOOL IsCopyAvailable() const
	{
		return TRUE;
	}

	virtual BOOL SerializeValue(CString& str);
	virtual BOOL ParseValue(const CString& str);
	virtual BOOL DoEdit();

// Attributes
public:
	CBCGPTextFormat& GetTextFormat()
	{
		return m_TextFormat;
	}

	void SetTextFormat (const CBCGPTextFormat& tf);

// Attributes
protected:
	CBCGPTextFormat				m_TextFormat;		// Text format value
	CBCGPTextFormat				m_TextFormatOrig;	// Original text format value
	CBCGPTextFormatDlgOptions	m_Options;
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPLineStyleProp object

class BCGCBPRODLLEXPORT CBCGPLineStyleProp : public CBCGPProp
{
	friend class CBCGPPropList;

	DECLARE_DYNAMIC(CBCGPLineStyleProp)

// Construction
public:
	CBCGPLineStyleProp(const CString& strName, CBCGPStrokeStyle::BCGP_DASH_STYLE style,
		LPCTSTR lpszDescr = NULL, DWORD_PTR dwData = 0);

	CBCGPLineStyleProp(const CString& strName, UINT nID, CBCGPStrokeStyle::BCGP_DASH_STYLE style,
		LPCTSTR lpszDescr = NULL, DWORD_PTR dwData = 0);

	virtual ~CBCGPLineStyleProp();

protected:
	void CommonInit();

// Overrides
public:
	virtual CComboBox* CreateCombo (CWnd* pWndParent, CRect rect);
	virtual void OnSetSelectedComboItem();
	virtual BOOL TextToVar (const CString& strText);
	virtual CString FormatProperty ();

// Attributes
public:
	CBCGPStrokeStyle::BCGP_DASH_STYLE GetStyle() const
	{
		return (CBCGPStrokeStyle::BCGP_DASH_STYLE)(long)m_varValue;
	}

	void SetStyle (CBCGPStrokeStyle::BCGP_DASH_STYLE style);
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPFileProp object

class BCGCBPRODLLEXPORT CBCGPFileProp : public CBCGPProp
{
	DECLARE_DYNAMIC(CBCGPFileProp)

// Construction
public:
	CBCGPFileProp(const CString& strName, const CString& strFolderName, DWORD_PTR dwData = 0, LPCTSTR lpszDescr = NULL);

	CBCGPFileProp(const CString& strName, UINT nID, const CString& strFolderName, DWORD_PTR dwData = 0, LPCTSTR lpszDescr = NULL);

	CBCGPFileProp(const CString& strName, BOOL bOpenFileDialog, const CString& strFileName, 
				LPCTSTR lpszDefExt = NULL,
				DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, 
				LPCTSTR lpszFilter = NULL,
				LPCTSTR lpszDescr = NULL, DWORD_PTR dwData = 0);

	CBCGPFileProp(const CString& strName, UINT nID, BOOL bOpenFileDialog, const CString& strFileName, 
				LPCTSTR lpszDefExt = NULL,
				DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, 
				LPCTSTR lpszFilter = NULL,
				LPCTSTR lpszDescr = NULL, DWORD_PTR dwData = 0);

	virtual ~CBCGPFileProp();

// Overrides
public:
	virtual void OnClickButton (CPoint point);

// Attributes
protected:

	BOOL m_bIsFolder;

	// File open dialog atributes:
	BOOL	m_bOpenFileDialog;	// TRUE - use "File Open/Save" diaog; otherwise - folder selection dialog

	DWORD	m_dwFileOpenFlags;
	CString	m_strDefExt;
	CString m_strFilter;
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPFontProp object

class BCGCBPRODLLEXPORT CBCGPFontProp : public CBCGPProp
{
	DECLARE_DYNAMIC(CBCGPFontProp)

// Construction
public:
	CBCGPFontProp(	const CString& strName, LOGFONT& lf, 
					DWORD dwFontDialogFlags = CF_EFFECTS | CF_SCREENFONTS, 
					LPCTSTR lpszDescr = NULL, DWORD_PTR dwData = 0,
					COLORREF color = (COLORREF)-1);

	CBCGPFontProp(	const CString& strName, UINT nID, LOGFONT& lf, 
					DWORD dwFontDialogFlags = CF_EFFECTS | CF_SCREENFONTS, 
					LPCTSTR lpszDescr = NULL, DWORD_PTR dwData = 0,
					COLORREF color = (COLORREF)-1);

	virtual ~CBCGPFontProp();

// Overrides
public:
	virtual void OnClickButton (CPoint point);
	virtual CString FormatProperty ();
	virtual BOOL SerializeValue(CString& str);
	virtual BOOL ParseValue(const CString& str);

// Attributes
public:
	LPLOGFONT GetLogFont ()
	{
		return &m_lf;
	}

	void SetLogFont(const LOGFONT& lf);
	void SetColor(COLORREF color);

	COLORREF GetColor () const
	{
		return m_Color;
	}

protected:
	LOGFONT		m_lf;
	LOGFONT		m_lfOrig;
	DWORD_PTR	m_dwFontDialogFlags;
	COLORREF	m_Color;
	COLORREF	m_ColorOrig;

	virtual BOOL IsValueChanged () const
	{
		return CompareLFandColor(m_lf, m_lfOrig, m_Color, m_ColorOrig);
	}

	virtual void ResetOriginalValue ();
	virtual void CommitModifiedValue ();

	BOOL CompareLFandColor(const LOGFONT& lf1, const LOGFONT& lf2, COLORREF c1, COLORREF c2) const;
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPDateTimeProp object

class BCGCBPRODLLEXPORT CBCGPDateTimeProp : public CBCGPProp
{
	DECLARE_DYNAMIC(CBCGPDateTimeProp)

// Construction
public:
	CBCGPDateTimeProp(const CString& strName, const COleDateTime& date, 
		LPCTSTR lpszDescr = NULL, DWORD_PTR dwData = 0,
		UINT nFlags = CBCGPDateTimeCtrl::DTM_DATE | CBCGPDateTimeCtrl::DTM_TIME);

	CBCGPDateTimeProp(const CString& strName, UINT nID, const COleDateTime& date, 
		LPCTSTR lpszDescr = NULL, DWORD_PTR dwData = 0,
		UINT nFlags = CBCGPDateTimeCtrl::DTM_DATE | CBCGPDateTimeCtrl::DTM_TIME);

	virtual ~CBCGPDateTimeProp();

// Overrides
public:
	virtual void OnDrawValue (CDC* pDC, CRect rect);
	virtual CWnd* CreateInPlaceEdit (CRect rectEdit, BOOL& bDefaultFormat);
	virtual void OnPosSizeChanged (CRect rectOld);
	virtual BOOL OnUpdateValue ();

	virtual void OnSetSelection (CBCGPProp* pOldSel);
	virtual void OnKillSelection (CBCGPProp* pNewSel);

	virtual BOOL PushChar (UINT nChar);

	virtual CString FormatProperty ();
	virtual BOOL IsValueChanged () const;

	virtual BOOL SerializeValue(CString& str);
	virtual BOOL ParseValue(const CString& str);

	virtual void AdjustInPlaceEditRect (CRect& rectEdit, CRect& rectSpin);
	virtual BOOL OnCommand (WPARAM /*wParam*/);

	virtual void ResetOriginalValue ();

	virtual BOOL IsCopyAvailable() const
	{
		return TRUE;
	}

// Attributes
public:
	void SetDate (COleDateTime date);
	COleDateTime GetDate () const
	{
		return (COleDateTime) (DATE) m_varValue;
	}

protected:
	UINT				m_nFlags;
	CBCGPDateTimeCtrl	m_wndDateTime;

	void SetCtrlState(CBCGPDateTimeCtrl& wnd);
};

/////////////////////////////////////////////////////////////////////////////
// Item menu flags: 

#define BCGP_PROPLIST_MENU_COMMANDS		0x0001	// "Show/hide commands" menu item is visible
#define BCGP_PROPLIST_MENU_DESCRIPTION	0x0002	// "Show/hide description" menu item is visible
#define BCGP_PROPLIST_MENU_COPY_PASTE	0x0004	// Copy/paste menu items are visible
#define BCGP_PROPLIST_MENU_RESET		0x0008	// "Reset" menu item is visible
#define BCGP_PROPLIST_MENU_EDIT			0x0010	// "Edit" menu item is visible
#define BCGP_PROPLIST_MENU_ALL			0xFFFF	// All menu items are visible

/////////////////////////////////////////////////////////////////////////////
// CBCGPPropertiesToolBar window

class BCGCBPRODLLEXPORT CBCGPPropertiesToolBar : public CBCGPToolBar
{
	friend class CBCGPPropList;

	CBCGPPropertiesToolBar(CBCGPToolBarImages& customImages) : m_CustomImages(customImages)
	{
		m_nDefaultButtons = 0;
	}

	virtual void OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL bDisableIfNoHndler)	
	{		
		CBCGPToolBar::OnUpdateCmdUI ((CFrameWnd*) GetOwner (), bDisableIfNoHndler);
	}

	virtual void CheckForButtonImages(CBCGPToolbarButton* pButton, CBCGPToolBarImages** pNewImages);
	virtual void OnLargeIconsModeChanged();

	virtual BOOL AllowShowOnList () const			{	return FALSE;	}
	virtual BOOL IsButtonExtraSizeAvailable() const	{	return FALSE;	}

	virtual BOOL LoadState (LPCTSTR /*lpszProfileName*/ = NULL, int /*nIndex*/ = -1, UINT /*uiID*/ = (UINT) -1)	{	return TRUE;	}
	virtual BOOL SaveState (LPCTSTR /*lpszProfileName*/ = NULL, int /*nIndex*/ = -1, UINT /*uiID*/ = (UINT) -1)	{	return TRUE;	}

	virtual BOOL PreTranslateMessage(MSG* pMsg);

	int					m_nDefaultButtons;
	CBCGPToolBarImages&	m_CustomImages;
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPPropList window

#define BCGPROPLIST_ID_INPLACE 3

BCGCBPRODLLEXPORT extern UINT BCGM_PROPERTY_CHANGED;
BCGCBPRODLLEXPORT extern UINT BCGM_PROPERTY_COMMAND_CLICKED;
BCGCBPRODLLEXPORT extern UINT BCGM_PROPERTY_MENU_ITEM_SELECTED;
BCGCBPRODLLEXPORT extern UINT BCGM_PROPERTY_GET_MENU_ITEM_STATE;
BCGCBPRODLLEXPORT extern UINT BCGM_PROPERTYLIST_SORTING_MODE_CHANGED;
BCGCBPRODLLEXPORT extern UINT BCGM_PROPERTYLIST_LAYOUT_CHANGED;

class BCGCBPRODLLEXPORT CBCGPPropList : public CBCGPWnd
{
	DECLARE_DYNAMIC(CBCGPPropList)

	friend class CBCGPProp;
	friend class CBCGPColorProp;
	friend class CBCGPDateTimeProp;

// Construction
public:
	CBCGPPropList();

// Attributes
public:
	void EnableToolBar(BOOL bShowStandardButtons = TRUE, // Show/hide "Categorized"/"Alphabetical" buttons
		UINT nCustomToolbarID = 0,						// Each button should be 16x16, 
		UINT nCustomToolbarBitmapID = 0);				// Each image should be 16x16);

	BOOL IsToolBar () const
	{
		return m_bToolBar;
	}

	void EnableSearchBox(BOOL bEnable = TRUE, LPCTSTR lpszPrompt = NULL);
	BOOL IsSearchBox() const
	{
		return m_bFilterBox;
	}

	void EnableHeaderCtrl (BOOL bEnable = TRUE,
		LPCTSTR lpszLeftColumn = _T("Property"), 
		LPCTSTR lpszRightColumn = _T("Value"));
	BOOL IsHeaderCtrl () const
	{
		return m_bHeaderCtrl;
	}

	virtual void SetPropertiesFilter(LPCTSTR lpszFilter);
	virtual BOOL IsPropertyMatchedToFilter(CBCGPProp* pProp, const CString& strFilter) const;

	void EnableContextMenu(BOOL bEnable = TRUE, UINT nMenuFlags = BCGP_PROPLIST_MENU_ALL);
	BOOL IsContextMenuEnabled() const
	{
		return m_bContextMenu;
	}

	void EnableDescriptionArea (BOOL bEnable = TRUE);
	BOOL IsDescriptionArea () const
	{
		return m_bDescriptionArea;
	}

	int GetDescriptionHeight () const
	{
		return m_nDescrHeight;
	}

	int GetDescriptionRows () const
	{
		return m_nDescrRows;
	}

	void SetDescriptionRows (int nDescRows);

	void SetCommands (const CStringList& lstCmdNames, int nCommandRows = 1);
	void ClearCommands ();

	BOOL HasCommands () const
	{
		return !m_lstCommands.IsEmpty ();
	}

	int GetCommandsHeight () const
	{
		return m_nCommandsHeight;
	}

	void SetCommandsVisible(BOOL bSet = TRUE);
	BOOL AreCommandsVisible() const
	{
		return m_bAreCommandsVisible;
	}

	void SetAlphabeticMode (BOOL bSet = TRUE);
	BOOL IsAlphabeticMode () const
	{
		return m_bAlphabeticMode;
	}

	void SetVSDotNetLook (BOOL bSet = TRUE);
	BOOL IsVSDotNetLook () const
	{
		return m_bVSDotNetLook;
	}

	void MarkModifiedProperties (BOOL bMark = TRUE, BOOL bRedraw = TRUE);
	BOOL IsMarkModifiedProperties () const
	{
		return m_bMarkModifiedProperties;
	}

	void ResetOriginalValues (BOOL bRedraw = TRUE);
	void CommitModifiedValues (BOOL bRedraw = TRUE);

	void SetItemMenuFlags(UINT nFlags)
	{
		m_nItemMenuFlags = nFlags;
	}

	UINT GetItemMenuFlags() const
	{
		return m_nItemMenuFlags;
	}

	void SetCustomMenuItems(const CStringList& lstMenuItemNames);

	void SetBoolLabels (LPCTSTR lpszTrue, LPCTSTR lpszFalse);
	void SetListDelimiter (TCHAR c);

	CRect GetListRect () const
	{
		return m_rectList; 
	}

	int GetPropertyColumnWidth () const 
	{ 
		return m_nLeftColumnWidth; 
	}

	void SetPropertyColumnWidth (int nWidth)
	{ 
		m_nLeftColumnWidth = nWidth;
	}

	int GetHeaderHeight () const
	{
		return m_nHeaderHeight;
	}

	int	GetRowHeight () const
	{
		return m_nRowHeight;
	}

	virtual PROP_HEADER_CTRL_CLASS& GetHeaderCtrl ()
	{
		return m_wndHeader;
	}

	int GetLeftColumnWidth () const
	{
		return m_nLeftColumnWidth;
	}

	BOOL IsGroupNameFullWidth () const
	{
		return m_bGroupNameFullWidth;
	}

	void SetGroupNameFullWidth (BOOL bGroupNameFullWidth = TRUE, BOOL bRedraw = TRUE);

	BOOL IsShowDragContext () const
	{
		return m_bShowDragContext;
	}

	void SetShowDragContext (BOOL bShowDragContext = TRUE)
	{
		m_bShowDragContext = bShowDragContext;
	}

	void SetNameAlign(int nAlign /* DT_LEFT, DT_CENTER or DT_RIGHT */, BOOL bRedraw = TRUE);
	int GetNameAlign() const
	{
		return m_nNameAlign;
	}

	void SetCustomColors (			// Use (COLORREF)-1 for the default color
		COLORREF	clrBackground,
		COLORREF	clrText,
		COLORREF	clrGroupBackground,
		COLORREF	clrGroupText,
		COLORREF	clrDescriptionBackground,
		COLORREF	clrDescriptionText,
		COLORREF	clrLine);

	void GetCustomColors (
		COLORREF&	clrBackground,
		COLORREF&	clrText,
		COLORREF&	clrGroupBackground,
		COLORREF&	clrGroupText,
		COLORREF&	clrDescriptionBackground,
		COLORREF&	clrDescriptionText,
		COLORREF&	clrLine);

	COLORREF GetTextColor () const
	{
		return m_clrText == (COLORREF)-1 ? m_clrTextDefault : m_clrText;
	}

	COLORREF GetBkColor () const
	{
		return m_clrBackground == (COLORREF)-1 ? 
			m_clrFillDefault : m_clrBackground;
	}

	void SetCommandTextColor (COLORREF color)
	{
		m_clrCommandText = color;
	}

	COLORREF GetCommandTextColor () const
	{
		return m_clrCommandText;
	}

	CFont& GetBoldFont ()
	{
		return m_fontBold;
	}

	BOOL IsAlwaysShowUserToolTip () const 
	{
		return m_bAlwaysShowUserTT;
	}

	void AlwaysShowUserToolTip (BOOL bShow = TRUE)
	{
		m_bAlwaysShowUserTT = bShow;
	}

	BOOL DrawControlBarColors () const
	{
		return m_bVisualManagerStyle || m_bControlBarColors;
	}

	void SetScrollBarsStyle (CBCGPScrollBar::BCGPSB_STYLE style)
	{
		m_ScrollBarStyle = style;
		m_wndScrollVert.SetVisualStyle (style);
	}

	CBCGPScrollBar::BCGPSB_STYLE GetScrollBarsStyle () const
	{
		return m_ScrollBarStyle;
	}

	CBCGPGraphicsManager* GetGraphicsManager()
	{
		if (m_pGM == NULL)
		{
			m_pGM = CBCGPGraphicsManager::CreateInstance();
			if (m_pGM != NULL)
			{
				m_pGM->EnableTransparentGradient();
			}
		}

		return m_pGM;
	}

protected:
	BOOL					m_bToolBar;				// Show embedded toolbar
	CBCGPPropertiesToolBar	m_wndToolBar;			// Property list embedded toolbar
	int						m_nToolbarDefaultButtons;// Number of toolbar default buttons	
	BOOL					m_bToolbarCustomButtons; // TRUE is toolbar has custom buttons
	BOOL					m_bToolbarStandardButtons;
	CBCGPToolBarImages		m_ToolBarCustomImages;
	UINT					m_nCustomToolbarID;
	UINT					m_nCustomToolbarBitmapID;
	BOOL					m_bNeedToUpdate;
	
	BOOL					m_bFilterBox;	// Show serach box
	CBCGPEdit				m_wndFilter;	// Search box
	CString					m_strFilterPrompt;

	PROP_HEADER_CTRL_CLASS	m_wndHeader;// Property list header control
	BOOL		m_bHeaderCtrl;			// Is header control visible?

	BOOL		m_bDescriptionArea;		// Does description area enabled?
	int			m_nDescrHeight;			// Description area height
	int			m_nDescrRows;			// Number of rows in description area

	BOOL		m_bContextMenu;			// Is advanced context menu enabled?

	UINT		m_nItemMenuFlags;		// Item menu flags (0 - don't show the menu)
	CStringList	m_lstCustomMenuItems;	// List of custom menu items

	CStringList	m_lstCommands;			// List of command names
	int			m_nCommandsHeight;		// Commands area height
	int			m_nCommandRows;			// Number of rows in command area
	CArray<CRect,CRect>	m_arCommandRects;// Array of command rectangles
	BOOL		m_bAreCommandsVisible;	// Is commands area visible?

	CToolTipCtrl	m_ToolTip;			// Tooltip control
	int			m_nTooltipsCount;		// Number of tooltip items

	CBCGPInplaceToolTipCtrl	m_IPToolTip;// Inplace tooltip control

	BOOL		m_bAlwaysShowUserTT;	// Always show user tooltips, even if in-place TT should be activated

	BOOL		m_bAlphabeticMode;		// Use property list in alphabetic (non-"tree") mode
	BOOL		m_bVSDotNetLook;		// Provide MS Visual Studio.NET look (gray groups, flat look)

	CString		m_strTrue;				// Customized boolean value (e.g. "Yes")
	CString		m_strFalse;				// Customized boolean value (e.g. "No")

	TCHAR		m_cListDelimeter;		// Customized list delimeter character

	CBCGPScrollBar	m_wndScrollVert;		// Vertical scroll bar
	CBCGPScrollBar::BCGPSB_STYLE 
				m_ScrollBarStyle;		// Scroll bars style

	HFONT		m_hFont;				// Property list regular font
	CFont		m_fontBold;				// Property list bold font
	int			m_nEditLeftMargin;		// Edit control left margin
	int			m_nBoldEditLeftMargin;	// Edit control left margin (bold font)

	int			m_nBorderSize;			// Control border size

	int			m_nToolbarHeight;		// Toolbar height
	int			m_nHeaderHeight;		// Header control height
	CRect		m_rectList;				// Properies area
	int			m_nRowHeight;			// Height of the single row
	int			m_nLeftColumnWidth;		// Width of the left ("Name") column

	int			m_nVertScrollOffset;	// In rows
	int			m_nVertScrollTotal;
	int			m_nVertScrollPage;

	BOOL		m_bMarkModifiedProperties;	// Draw modified properties with bold

	int			m_nNameAlign;			// DT_LEFT, ...

	//-----------------------------------------------------------
	// Tracking attributes: used for the vertical and description 
	// area splitters and buttons tracking:
	//-----------------------------------------------------------
	CRect		m_rectTrackHeader;
	CRect		m_rectTrackDescr;
	CRect		m_rectTrackCommands;
	BOOL		m_bTracking;
	BOOL		m_bTrackingCommands;
	BOOL		m_bTrackingDescr;
	BOOL		m_bTrackButtons;

	CList<CBCGPProp*, CBCGPProp*>	m_lstProps;			// List of top-level properties
	CList<CBCGPProp*, CBCGPProp*>	m_lstTerminalProps;	// List of terminal properties

	CBCGPProp*	m_pSel;					// Current selection
	CBCGPProp*	m_pTracked;				// Currently tracked item
	
	BOOL		m_bFocused;				// Control has focus

	COLORREF	m_clrGray;				// Special gray color

	BOOL		m_bControlBarColors;	// Use colors of the parent control bar
	BOOL		m_bGroupNameFullWidth;	// Display group name in the whole row
	BOOL		m_bShowDragContext;		// Show context while dragging spliters

	CString		m_strFilter;
	BOOL		m_bOutOfFilter;
	CString		m_strOutOfFilter;

	COLORREF	m_clrTextDefault;
	COLORREF	m_clrFillDefault;

	//---------------
	// Custom colors:
	//---------------
	COLORREF	m_clrBackground;		// Control background color
	COLORREF	m_clrText;				// Control foreground color
	COLORREF	m_clrGroupBackground;	// Group background text
	COLORREF	m_clrGroupText;			// Group foreground text
	COLORREF	m_clrDescriptionBackground;	// Description background text
	COLORREF	m_clrDescriptionText;	// Description foreground text
	COLORREF	m_clrLine;				// Color of the grid lines
	COLORREF	m_clrCommandText;		// Command foreground color

	CBrush					m_brBackground;

	CBCGPGraphicsManager*	m_pGM;
	CBCGPToolBarImages		m_AdvImages;

// Operations
public:
	int AddProperty (CBCGPProp* pProp, BOOL bRedraw = TRUE, BOOL bAdjustLayout = TRUE);
	BOOL DeleteProperty (CBCGPProp*& pProp, BOOL bRedraw = TRUE, BOOL bAdjustLayout = TRUE);
	void RemoveAll ();

	CBCGPProp* GetProperty (int nIndex) const;
	int GetPropertyCount () const
	{
		return (int) m_lstProps.GetCount ();
	}

	CBCGPProp* FindItemByData (DWORD_PTR dwData, BOOL bSearchSubItems = TRUE) const;
	CBCGPProp* FindItemByID (UINT nID, BOOL bSearchSubItems = TRUE) const;

	virtual CBCGPProp* HitTest (CPoint pt, CBCGPProp::ClickArea* pnArea = NULL, BOOL bPropsOnly = FALSE) const;

	void SetCurSel (CBCGPProp* pProp, BOOL bRedraw = TRUE);
	CBCGPProp* GetCurSel () const
	{
		return m_pSel;
	}

	void EnsureVisible (CBCGPProp* pProp, BOOL bExpandParents = FALSE);
	void ExpandAll (BOOL bExapand = TRUE);

	BOOL UpdateProperty(UINT nID, const _variant_t& varValue);
	BOOL UpdateBrushProperty(UINT nID, const CBCGPBrush& brush);
	BOOL UpdateTextFormatProperty(UINT nID, const CBCGPTextFormat& tf);
	BOOL UpdateColorProperty(UINT nID, COLORREF color);
	BOOL UpdateDateTimeProperty(UINT nID, const COleDateTime& date);
	BOOL UpdateFontProperty(UINT nID, const LOGFONT& lf);

	const _variant_t& GetPropertyValue(UINT nID) const;
	const CBCGPBrush* GetBrushPropertyValue(UINT nID) const;
	const CBCGPTextFormat* GetTextFormatPropertyValue(UINT nID) const;
	COLORREF GetColorPropertyValue(UINT nID) const;
	COleDateTime GetDateTimePropertyValue(UINT nID) const;
	const LOGFONT* GetFontPropertyValue(UINT nID) const;

	BOOL EnableProperty(UINT nID, BOOL bEnable = TRUE);
	BOOL SelectPropertyOption(UINT nID, int nIndex);

	void RedrawProperty(UINT nID);

	virtual void CloseColorPopup ();
	virtual void UpdateColor (COLORREF color);

	CBCGPProp* GetFirstNonFilteredProperty();
	CBCGPProp* GetLastNonFilteredProperty();

	virtual void AdjustLayout ();

// Overrides
	virtual void OnPropertyChanged (CBCGPProp* pProp) const;

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPPropList)
	public:
	virtual BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual CScrollBar* GetScrollBarCtrl(int nBar) const;
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	protected:
	virtual void PreSubclassWindow();
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

public:
	virtual void OnChangeSelection (CBCGPProp* /*pNewSel*/, CBCGPProp* /*pOldSel*/) {}

	virtual BOOL EditItem (CBCGPProp* pProp, LPPOINT lptClick = NULL);
	virtual void OnClickButton (CPoint point);
	virtual BOOL EndEditItem (BOOL bUpdateData = TRUE);
	virtual BOOL ValidateItemData (CBCGPProp* /*pProp*/)
	{
		return TRUE;
	}

	virtual int OnDrawProperty (CDC* pDC, CBCGPProp* pProp) const;
	
	virtual void InitToolBar();
	virtual void InitHeader();

	virtual BOOL OnSetAccData (long lVal);

protected:
	virtual void OnDraw(CDC* pDC);
	virtual void Init ();

	virtual void OnFillBackground (CDC* pDC, CRect rectClient);

	virtual void OnDrawBorder (CDC* pDC);
	virtual void OnDrawList (CDC* pDC);
	virtual void OnDrawDescription (CDC* pDC, CRect rect);
	virtual void OnDrawCommands (CDC* pDC, CRect rect);
	
	virtual BOOL OnDrawMenuButton(CDC* pDC, CBCGPProp* pProp, CRect rect, BOOL bIsHighlighted, BOOL bIsModified);
	virtual BOOL OnDrawStateIndicator(CDC* pDC, CBCGPProp* pProp, CRect rect);

	virtual BOOL ProcessClipboardAccelerators (UINT nChar);

	virtual int CompareProps (const CBCGPProp* pProp1, const CBCGPProp* pProp2) const;

	virtual void NotifyAccessibility (CBCGPProp* pProp);

	virtual void SetupToolbarCustomButtons();

// Implementation
public:
	virtual ~CBCGPPropList();

	//----------------------
	// Accessibility support:
	//----------------------
	CBCGPProp* m_pAccProp;

// IAccessible
public:
	virtual HRESULT get_accChildCount(long *pcountChildren);
	virtual HRESULT get_accChild(VARIANT varChild, IDispatch **ppdispChild);
	virtual HRESULT get_accName(VARIANT varChild, BSTR *pszName);
	virtual HRESULT get_accValue(VARIANT varChild, BSTR *pszValue);
	virtual HRESULT get_accDescription(VARIANT varChild, BSTR *pszDescription);
	virtual HRESULT get_accRole(VARIANT varChild, VARIANT *pvarRole);
	virtual HRESULT get_accState(VARIANT varChild, VARIANT *pvarState);
	virtual HRESULT get_accHelp(VARIANT varChild, BSTR *pszHelp);
	virtual HRESULT get_accHelpTopic(BSTR *pszHelpFile, VARIANT varChild, long *pidTopic);
	virtual HRESULT get_accKeyboardShortcut(VARIANT varChild, BSTR *pszKeyboardShortcut);
	virtual HRESULT get_accFocus(VARIANT *pvarChild);
	virtual HRESULT get_accSelection(VARIANT *pvarChildren);
	virtual HRESULT get_accDefaultAction(VARIANT varChild, BSTR *pszDefaultAction);

	virtual HRESULT accSelect(long flagsSelect, VARIANT varChild);
	virtual HRESULT accLocation(long *pxLeft, long *pyTop, long *pcxWidth, long *pcyHeight, VARIANT varChild);
	virtual HRESULT accHitTest(long xLeft, long yTop, VARIANT *pvarChild);

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPPropList)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnCancelMode();
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg UINT OnGetDlgCode();
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnDestroy();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp);
	afx_msg void OnNcPaint();
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	//}}AFX_MSG
	afx_msg LRESULT OnSetFont (WPARAM, LPARAM);
	afx_msg LRESULT OnGetFont (WPARAM, LPARAM);
	afx_msg void OnHeaderItemChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnHeaderTrack(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnHeaderEndTrack(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSpinDeltaPos(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg LRESULT OnUpdateSpin (WPARAM, LPARAM);
	afx_msg void OnStyleChanged(int nStyleType, LPSTYLESTRUCT lpStyleStruct);
	afx_msg void OnSelectCombo();
	afx_msg void OnCloseCombo();
	afx_msg void OnEditKillFocus();
	afx_msg void OnComboKillFocus();
	afx_msg BOOL OnNeedTipText(UINT id, NMHDR* pNMH, LRESULT* pResult);
	afx_msg LRESULT OnGetObject (WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnPrintClient(WPARAM wp, LPARAM lp);
	afx_msg LRESULT OnPrint(WPARAM wp, LPARAM lp);
	afx_msg LRESULT OnMouseLeave(WPARAM wp,LPARAM lp);
	afx_msg void OnToolbarCategorized();
	afx_msg void OnUpdateToolbarCategorized(CCmdUI* pCmdUI);
	afx_msg void OnToolbarAlphabetical();
	afx_msg void OnUpdateToolbarAlphabetical(CCmdUI* pCmdUI);
	afx_msg void OnFilter();
	DECLARE_MESSAGE_MAP()

	//------------------
	// Internal helpres:
	//------------------
	HFONT SetCurrFont (CDC* pDC);
	void TrackHeader (int nOffset);
	void TrackDescr (int nOffset);
	void TrackCommands (int nOffset);
	void TrackToolTip (CPoint pt);

	void SetScrollSizes ();

	int GetTotalItems (BOOL bIncludeHidden = TRUE) const;
	void ReposProperties ();

	void CreateBoldFont ();
	void CalcEditMargin ();
};

#define GetDesciption		 GetDescription
#define SetDesciption		 SetDescription
#define m_bDesciptionArea	 m_bDescriptionArea
#define GetDesciptionHeight	 GetDescriptionHeight
#define OnDrawDesciption	 OnDrawDescription
#define EnableDesciptionArea EnableDescriptionArea
#define IsDesciptionArea	 IsDescriptionArea
#define GetDesciptionHeight	 GetDescriptionHeight
#define GetOrgiginalValue	 GetOriginalValue

#endif // BCGP_EXCLUDE_PROP_LIST

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPPROPLIST_H__F6054FED_0317_4829_B7BF_4EBBDC27DF01__INCLUDED_)
