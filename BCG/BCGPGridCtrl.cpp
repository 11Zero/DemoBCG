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
// BCGPGridCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "multimon.h"
#include "trackmouse.h"
#include "BCGPReportCtrl.h"
#include "BCGPGridCtrl.h"
#include "BCGPDlgImpl.h"
#include "BCGPMath.h"

#ifndef BCGP_EXCLUDE_GRID_CTRL

#include "BCGPLocalResource.h"
#include "BCGPGlobalUtils.h"

#ifndef _BCGPGRID_STANDALONE
#ifndef _BCGSUITE_
	#include "BCGPColorBar.h"
	#include "BCGPWorkspace.h"
	#include "BCGPToolbarComboBoxButton.h"
	#include "BCGPRegistry.h"
	#include "BCGPPopupMenu.h"
	#include "BCGPToolBar.h"
	#include "BCGPMaskEdit.h"
	#include "RegPath.h"
	#include "BCGPTooltipManager.h"
#endif
	#include "BCGPVisualManager.h"
	#include "BCGPVisualManager2007.h"	//
	#include "BCGPVisualManagerVS2010.h"//
	#include "MenuImages.h"
	#include "bcgprores.h"
#else
	#include "BCGPGridVisualManager.h"
	#include "resource.h"
#endif

#include "BCGPDrawManager.h"
#include "BCGPSpinButtonCtrl.h"
#include "BCGPGridView.h"
#include "BCGPGridFilter.h"
#include "BCGPGridFilterMenu.h"
#include "BCGPGridSerialize.h"

#include "BCGPChartVisualObject.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef _UNICODE
#define _TCF_TEXT	CF_UNICODETEXT
#else
#define _TCF_TEXT	CF_TEXT
#endif

BOOL CBCGPGridCtrl::m_bEnableAssertValidInDebug = FALSE;

#ifdef _DEBUG
#undef ASSERT_VALID
#define ASSERT_VALID(pOb) !CBCGPGridCtrl::m_bEnableAssertValidInDebug ? ((void)0) : (::AfxAssertValidObject(pOb, THIS_FILE, __LINE__))
#endif

#define ID_HEADER		1
#define ID_SCROLL_VERT	2
#define ID_SCROLL_HORZ	3
#define STRETCH_DELTA	2

#define TEXT_MARGIN		3
#define TEXT_VMARGIN	2

#define	BCGPGRIDCTRL_ID_INPLACE	1
#define ID_FILTERBAR_BUTTON		101
#define ID_DEFAULTFILTER_APPLY	102

#ifndef _BCGPGRID_STANDALONE
#ifndef _BCGSUITE_
	extern CBCGPWorkspace*		g_pWorkspace;
	#define visualManagerMFC	CBCGPVisualManager::GetInstance ()
#else
	#define visualManagerMFC	CMFCVisualManager::GetInstance ()
#endif

	#define visualManager		CBCGPVisualManager::GetInstance ()
#else
	#define visualManager		CBCGPGridVisualManager::GetInstance ()
	#define visualManagerMFC	CBCGPGridVisualManager::GetInstance ()
#endif

static const CString strGridsProfile = _T ("BCGPGrids");
static const CString g_strEOL = _T ("\r\n");
static const CString g_chSpace = _T (" ");

#define REG_SECTION_FMT	_T("%sBCGPGrid-%d")

/////////////////////////////////////////////////////////////////////////////
// Grid sort helper class

typedef int (__cdecl *GENERICCOMPAREFN)(const void * elem1, const void * elem2);
typedef int (__cdecl *STRINGCOMPAREFN)(const CBCGPGridRow** pElem1, const CBCGPGridRow** pElem2);

typedef CArray<CBCGPGridRow*, CBCGPGridRow*> CBCGPGridRowArray;

class CBCGPGridCtrl;

class CBCGPSortableArray : public CBCGPGridRowArray
{
public:
	CBCGPSortableArray()
	{
	}

	virtual ~CBCGPSortableArray()
	{
	}

public:
	void Sort(STRINGCOMPAREFN pfnCompare = Compare)
	{
		CBCGPGridRow** prgstr = GetData();
		qsort(prgstr, GetSize(), sizeof(CBCGPGridRow*),(GENERICCOMPAREFN)pfnCompare);
	}

protected:
	static int __cdecl Compare(const CBCGPGridRow** pElem1, const CBCGPGridRow** pElem2)
	{
		ASSERT(pElem1);
		ASSERT(pElem2);

		CBCGPGridRow* pRow1 = * ( CBCGPGridRow** ) pElem1;
		CBCGPGridRow* pRow2 = * ( CBCGPGridRow** ) pElem2;
		ASSERT_VALID (pRow1);
		ASSERT_VALID (pRow2);

		CBCGPGridCtrl* pGrid = pRow1->GetOwnerList ();
		ASSERT_VALID (pGrid);
		if (!pGrid)
		{
			TRACE0 ("\nCBCGPSortableArray::Compare: Owner grid - Null");
			return 0;
		}

		return pGrid->DoMultiColumnCompare (pRow1, pRow2);
	}
};

/////////////////////////////////////////////////////////////////////////////
// Grid custom colors

BOOL BCGP_GRID_COLOR_DATA::ColorData::Draw (CDC* pDC, CRect rect, BOOL bNoBorder)
{
	if (m_clrBackground == (COLORREF)-1)
	{
		return FALSE;
	}

	if (m_clrGradient == (COLORREF)-1)
	{
		CBrush br (m_clrBackground);
		pDC->FillRect (rect, &br);
	}
	else
	{
		CBCGPDrawManager dm (*pDC);
		dm.FillGradient2 (rect, 
			m_clrBackground, 
			m_clrGradient,
			m_nGradientAngle);
	}

	if (m_clrBorder != (COLORREF)-1 && !bNoBorder)
	{
		pDC->Draw3dRect (rect, m_clrBorder, m_clrBorder);
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Find/Replace dialog

static UINT WM_FINDREPLACE = ::RegisterWindowMessage(FINDMSGSTRING);

class CBCGPGridFindDlg : public CFindReplaceDialog
{
	friend class CBCGPGridCtrl;

	CBCGPGridFindDlg (CBCGPGridCtrl* pParent) : CFindReplaceDialog (), m_pParent (pParent) {}
	virtual ~CBCGPGridFindDlg ();

protected:
	CBCGPGridCtrl* m_pParent;
};

CBCGPGridFindDlg::~CBCGPGridFindDlg ()
{
	if (m_pParent != NULL)
	{
		ASSERT_VALID (m_pParent);
		
		m_pParent->m_pFindDlg = NULL;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPGridColumnListBox window

class CBCGPGridColumnListBox : public CListBox
{
// Construction
public:
	CBCGPGridColumnListBox(CBCGPGridColumnsInfo& columns);

// Attributes
public:
	CBCGPGridColumnsInfo& m_Columns;
	BOOL m_bVisualManagerStyle;
	COLORREF m_clrText;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPGridColumnListBox)
	public:
	virtual int CompareItem(LPCOMPAREITEMSTRUCT lpCompareItemStruct);
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBCGPGridColumnListBox();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPGridColumnListBox)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPGridColumnChooser frame

class CBCGPGridColumnChooser : public CMiniFrameWnd
{
	DECLARE_DYNAMIC(CBCGPGridColumnChooser)

public:
	CBCGPGridColumnChooser(CBCGPGridColumnsInfo& columns);

// Attributes
public:
	CBCGPGridColumnListBox	m_wndList;
	CBCGPGridCtrl*			m_pOwnerGrid;
	BOOL					m_bIsEmpty;

protected:
	CString					m_strNoFields;

// Operations
public:
	void UpdateList ();
	int GetColumnWidth () const;

	BOOL IsVisualManagerStyle() const
	{
		return m_wndList.m_bVisualManagerStyle;
	}

	void EnableVisualManagerStyle(BOOL bEnable)
	{
		m_wndList.m_bVisualManagerStyle = bEnable;
	}

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPGridColumnChooser)
	public:
	virtual BOOL Create(CBCGPGridCtrl* pOwnerGrid, const RECT& rect, CWnd* pParentWnd);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CBCGPGridColumnChooser();

	// Generated message map functions
	//{{AFX_MSG(CBCGPGridColumnChooser)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnClose();
	afx_msg void OnWindowPosChanged(WINDOWPOS FAR* lpwndpos);
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPGridItem

IMPLEMENT_SERIAL(CBCGPGridItem, CObject, VERSIONABLE_SCHEMA | 1)

CBCGPGridItem::CBCGPGridItem ():
	m_varValue (),
	m_strLabel (_T("")),
	m_strEditMask (_T("")),
	m_strEditTempl (_T("")),
	m_strValidChars (_T("")),
	m_dwData (0)
{
	Init ();
	SetFlags ();
}
//****************************************************************************************
CBCGPGridItem::CBCGPGridItem(const _variant_t& varValue, DWORD_PTR dwData,
		LPCTSTR lpszEditMask, LPCTSTR lpszEditTemplate,
		LPCTSTR lpszValidChars) :
	m_varValue (varValue),
	m_strEditMask (lpszEditMask == NULL ? _T("") : lpszEditMask),
	m_strEditTempl (lpszEditTemplate == NULL ? _T("") : lpszEditTemplate),
	m_strValidChars (lpszValidChars == NULL ? _T("") : lpszValidChars),
	m_dwData (dwData)
{
	Init ();
	SetFlags ();

	m_bIsChanged = TRUE;

	if (m_varValue.vt == VT_BOOL)
	{
		m_bAllowEdit = FALSE;
	}
}
//****************************************************************************************
void CBCGPGridItem::SetFlags ()
{
	m_dwFlags = BCGP_GRID_ITEM_VCENTER;

	switch (m_varValue.vt)
	{
	case VT_BSTR:
	case VT_R4:
	case VT_R8:
	case VT_UI1:
	case VT_I2:
	case VT_I4:
	case VT_I8:
	case VT_UI8:
	case VT_INT:
	case VT_UINT:
	case VT_UI2:
	case VT_UI4:
		break;

	case VT_DATE:
		break;

    case VT_BOOL:
		m_dwFlags |= BCGP_GRID_ITEM_HAS_LIST;
		break;

	default:
		break;
	}
}
//******************************************************************************************
void CBCGPGridItem::Init ()
{
	m_pGridRow = NULL;
	m_nIdColumn = -1;

	m_bEnabled = TRUE;
	m_bAllowEdit = TRUE;
	m_bReadOnly = FALSE;
	m_bSelected = FALSE;

	m_pWndInPlace = NULL;
	m_pWndCombo = NULL;
	m_pWndSpin = NULL;
	m_bInPlaceEdit = FALSE;
	m_bButtonIsDown = FALSE;
	m_bValueIsTrancated = FALSE;

	m_sizeCombo.cx = 50;
	m_sizeCombo.cy = 400;

	m_Rect.SetRectEmpty ();
	m_rectButton.SetRectEmpty ();

	m_nMinValue = 0;
	m_nMaxValue = 0;

	m_clrBackground = (COLORREF)-1;
	m_clrText = (COLORREF)-1;

	m_iImage = -1;

	m_pMerged = NULL;

	m_nDataBarPerc = -1;
	m_nDataColorScalePerc = -1;
	m_nDataIconPerc = -1;

	m_bIsChanged = FALSE;
}
//******************************************************************************************
CBCGPGridItem::~CBCGPGridItem()
{
	if (m_pMerged != NULL)
	{
		m_pMerged->Release ();
	}

	OnDestroyWindow ();
}
//******************************************************************************************
void CBCGPGridItem::OnDestroyWindow ()
{
	if (m_pWndCombo != NULL)
	{
		m_pWndCombo->DestroyWindow ();
		delete m_pWndCombo;
		m_pWndCombo = NULL;
	}

	if (m_pWndInPlace != NULL)
	{
		m_pWndInPlace->DestroyWindow ();
		delete m_pWndInPlace;
		m_pWndInPlace = NULL;
	}

	if (m_pWndSpin != NULL)
	{
		m_pWndSpin->DestroyWindow ();
		delete m_pWndSpin;
		m_pWndSpin = NULL;
	}

	if (m_varValue.vt == VT_BOOL)
	{
		m_lstOptions.RemoveAll ();
	}
}
//****************************************************************************************
BOOL CBCGPGridItem::HasButton () const
{
	return	(m_dwFlags & BCGP_GRID_ITEM_HAS_LIST) ||
			(m_dwFlags & BCGP_GRID_ITEM_HAS_BUTTON);
}
//*******************************************************************************************
BOOL CBCGPGridItem::AddOption (LPCTSTR lpszOption, BOOL bInsertUnique/* = TRUE*/)
{
	ASSERT_VALID (this);
	ASSERT (lpszOption != NULL);

	if (bInsertUnique)
	{
 		if (m_lstOptions.Find (lpszOption) != NULL)
		{
			return FALSE;
		}
	}

	m_lstOptions.AddTail (lpszOption);
	m_dwFlags |= BCGP_GRID_ITEM_HAS_LIST;

	return TRUE;
}
//****************************************************************************************
void CBCGPGridItem::RemoveAllOptions ()
{
	ASSERT_VALID (this);

	m_lstOptions.RemoveAll ();
	m_dwFlags = 0;
}
//****************************************************************************************
int CBCGPGridItem::GetOptionCount () const
{
	ASSERT_VALID (this);
	return (int) m_lstOptions.GetCount ();
}
//****************************************************************************************
LPCTSTR CBCGPGridItem::GetOption (int nIndex) const
{
	ASSERT_VALID (this);

	if (nIndex < 0 || nIndex >= m_lstOptions.GetCount ())
	{
		ASSERT (FALSE);
		return NULL;
	}

	POSITION pos = m_lstOptions.FindIndex (nIndex);
	if (pos == NULL)
	{
		ASSERT (FALSE);
		return NULL;
	}

	return m_lstOptions.GetAt (pos);
}
//*******************************************************************************************
CBCGPGridItem* CBCGPGridItem::HitTest (CPoint point, CBCGPGridRow::ClickArea* pnArea)
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pGridRow);
	
	CRect rect = m_Rect;
	rect.right++;

	if (rect.PtInRect (point))
	{
		CBCGPGridCtrl* pGrid = GetOwnerList ();
		if (pGrid != NULL && pGrid->m_nHorzScrollOffset > 0 && pGrid->GetColumnsInfo ().IsFreezeColumnsEnabled ())
		{
			// Check if the item is scrolled left out of the visible area
			if (point.x < pGrid->m_rectList.left + pGrid->GetColumnsInfo ().GetFreezeOffset () &&
				!pGrid->GetColumnsInfo ().IsColumnFrozen (m_nIdColumn))
			{
				return NULL;
			}
		}

		if (pnArea != NULL)
		{
			*pnArea = CBCGPGridRow::ClickValue;
		}

		return this;
	}

	return NULL;
}
//*******************************************************************************************
void CBCGPGridItem::Redraw ()
{
	ASSERT_VALID (this);

	CBCGPGridCtrl* pWndList = NULL;
	if (m_pGridRow != NULL)
	{
		ASSERT_VALID (m_pGridRow);
		pWndList = m_pGridRow->m_pWndList;
	}

	if (pWndList != NULL && pWndList->GetSafeHwnd () != NULL)
	{
		ASSERT_VALID (pWndList);

		pWndList->InvalidateRect (m_Rect);

		if (!pWndList->m_bNoUpdateWindow)
		{
			pWndList->UpdateWindow ();
		}
	}
}
//*******************************************************************************************
void CBCGPGridItem::EnableSpinControl (BOOL bEnable, int nMin, int nMax)
{
	ASSERT_VALID (this);

	switch (m_varValue.vt)
	{
	case VT_INT:
	case VT_UINT:
    case VT_I2:
	case VT_I4:
    case VT_UI2:
	case VT_UI4:
		break;

	default:
		ASSERT (FALSE);
		return;
	}

	m_nMinValue = nMin;
	m_nMaxValue = nMax;

	if (bEnable)
	{
		m_dwFlags |= BCGP_GRID_ITEM_HAS_SPIN;
	}
	else
	{
		m_dwFlags &= ~BCGP_GRID_ITEM_HAS_SPIN;
	}
}
//*******************************************************************************************
void CBCGPGridItem::Select (BOOL bSelect)
{
	m_bSelected = bSelect;

	CBCGPGridCtrl* pWndList = GetOwnerList ();

	if (bSelect && HasButton () && pWndList != NULL && pWndList->GetCurSelItemID () == GetGridItemID ())
	{
		AdjustButtonRect ();
	}
	else
	{
		m_rectButton.SetRectEmpty ();
	}
}
//*******************************************************************************************
BOOL CBCGPGridItem::IsSelected () const
{
	return m_bSelected;
}
//******************************************************************************************
void CBCGPGridItem::SetValue (const _variant_t& varValue, BOOL bRedraw)
{
	ASSERT_VALID (this);

	if (m_varValue.vt != VT_EMPTY && m_varValue.vt != varValue.vt)
	{
		ASSERT (FALSE);
		return;
	}

	BOOL bInPlaceEdit = m_bInPlaceEdit;
	if (bInPlaceEdit)
	{
		OnEndEdit ();
	}

	m_varValue = varValue;
	m_bIsChanged = TRUE;

	if (bRedraw)
	{
		Redraw ();
	}

	if (bInPlaceEdit)
	{
		ASSERT_VALID (m_pGridRow);

		CBCGPGridCtrl* pWndList = m_pGridRow->m_pWndList;
		ASSERT_VALID (m_pGridRow);

		CPoint pt = m_Rect.TopLeft ();
		pWndList->SetBeginEditReason (CBCGPGridCtrl::BeginEdit_ReOpen);
		pWndList->EditItem (m_pGridRow, &pt);

		pWndList->DoInplaceEditSetSel (pWndList->OnInplaceEditSetSel (this, CBCGPGridCtrl::BeginEdit_ReOpen));
	}
}
//******************************************************************************************
void CBCGPGridItem::EmptyValue (BOOL bRedraw)
{
	ASSERT_VALID (this);

	if (m_bInPlaceEdit)
	{
		OnEndEdit ();
	}

	m_varValue.Clear ();
	m_bIsChanged = TRUE;

	if (bRedraw)
	{
		Redraw ();
	}
}
//*******************************************************************************************
void CBCGPGridItem::SetImage (int iImage, BOOL bRedraw/* = TRUE*/)
{
	ASSERT_VALID (this);

	m_iImage = iImage;

	if (bRedraw)
	{
		Redraw ();
	}
}
//*******************************************************************************************
void CBCGPGridItem::Enable (BOOL bEnable)
{
	ASSERT_VALID (this);

	if (m_bEnabled != bEnable)
	{
		m_bEnabled = bEnable;

		CBCGPGridCtrl* pWndList = NULL;
		if (m_pGridRow != NULL)
		{
			ASSERT_VALID (m_pGridRow);
			pWndList = m_pGridRow->m_pWndList;
		}

		if (pWndList != NULL && pWndList->GetSafeHwnd () != NULL)
		{
			ASSERT_VALID (pWndList);
			pWndList->InvalidateRect (m_Rect);
		}
	}
}
//*******************************************************************************************
void CBCGPGridItem::SetOwnerRow (CBCGPGridRow* pRow)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pRow);

	m_pGridRow = pRow;

	if (m_pGridRow->GetLinesNumber() > 1)
	{
		SetMultiline(TRUE);
		m_dwFlags &= ~(BCGP_GRID_ITEM_VCENTER);
		m_dwFlags |= (BCGP_GRID_ITEM_VTOP);
	}
}
//*****************************************************************************************
CString CBCGPGridItem::FormatItem ()
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pGridRow);

	CBCGPGridCtrl* pWndList = m_pGridRow->m_pWndList;
	ASSERT_VALID (pWndList);

	// If merged, refer to main item
	CBCGPGridItem* pMergedItem = GetMergedMainItem ();

	const _variant_t& var = (pMergedItem != NULL) ? pMergedItem->GetValue () : m_varValue;

	CString strVal;

	switch (var.vt)
	{
	case VT_BSTR:
		strVal = (LPCTSTR)(_bstr_t)var;
		break;

	case VT_DECIMAL:
		{
			DECIMAL num = (DECIMAL) var;
			globalUtils.StringFromDecimal (strVal, num);
		}
		break;

	case VT_CY:
		{
			CY cy = (CY) var;
			globalUtils.StringFromCy (strVal, cy);
		}
		break;

    case VT_I2:
		strVal.Format (CBCGPGridRow::m_strFormatShort, (short) var);
		break;

	case VT_I4:
	case VT_INT:
		strVal.Format (CBCGPGridRow::m_strFormatLong, (long) var);
		break;

	case VT_UI1:
		strVal.Format (CBCGPGridRow::m_strFormatUShort, (BYTE) var);
		break;

    case VT_UI2:
		strVal.Format (CBCGPGridRow::m_strFormatUShort, var.uiVal);
		break;

	case VT_UINT:
	case VT_UI4:
		strVal.Format (CBCGPGridRow::m_strFormatULong, var.ulVal);
		break;

#if _MSC_VER >= 1500
	case VT_I8:
		{
			TCHAR szBuffer[32];
			_i64tot_s ((LONGLONG) var, szBuffer, 32, 10);
			strVal = szBuffer;
		}
		break;
		
	case VT_UI8:
		{
			TCHAR szBuffer[32];
			_ui64tot_s ((ULONGLONG) var, szBuffer, 32, 10);
			strVal = szBuffer;
		}
		break;
#endif

    case VT_R4:
		strVal.Format (CBCGPGridRow::m_strFormatFloat, (float) var);
		break;

    case VT_R8:
		strVal.Format (CBCGPGridRow::m_strFormatDouble, (double) var);
		break;

    case VT_BOOL:
		{
			bool bVal = (bool) var;
			strVal = bVal ? pWndList->m_strTrue : pWndList->m_strFalse;
		}
		break;

	case VT_DATE:
		{
			COleDateTime date = (DATE) var;
			strVal = date.Format ();
		}
		break;

	default:
		break;
	}

	return strVal;
}
//******************************************************************************************
void CBCGPGridItem::OnDrawValue (CDC* pDC, CRect rect)
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pGridRow);
	
	CBCGPGridCtrl* pWndList = m_pGridRow->m_pWndList;
	ASSERT_VALID (pWndList);

	COLORREF clrTextOld = (COLORREF)-1;
	COLORREF clrText = OnFillBackground (pDC, rect);

	if (clrText != (COLORREF)-1)
	{
		clrTextOld = pDC->SetTextColor (clrText);
	}

	//-----------------------
	// Draw data color scale:
	//-----------------------
	if (!m_bInPlaceEdit)
	{
		OnDrawDataColorScale(pDC, rect);
	}

	//---------------
	// Draw data bar:
	//---------------
	if (!m_bInPlaceEdit)
	{
		OnDrawDataBar (pDC, rect);
	}

	//-----------
	// Draw icon:
	//-----------
	OnDrawIcon (pDC, rect);

	// -----------
	// Draw value:
	// -----------
	rect.DeflateRect (TEXT_MARGIN, TEXT_VMARGIN);

	// If merged, refer to main item
	CBCGPGridItem* pMergedItem = GetMergedMainItem ();
	const CString& strText = (pMergedItem != NULL) ? pMergedItem->GetLabel () : GetLabel ();
	const DWORD dwFlags = (pMergedItem != NULL) ? pMergedItem->GetFlags () : GetFlags ();
	int nTextAlign = (pMergedItem != NULL) ? pMergedItem->GetAlign () : GetAlign ();

	UINT uiTextFlags = DT_NOPREFIX | DT_END_ELLIPSIS;

	if (nTextAlign & HDF_CENTER)
	{
		uiTextFlags |= DT_CENTER;
	}
	else if (nTextAlign & HDF_RIGHT)
	{
		uiTextFlags |= DT_RIGHT;
	}
	else // nTextAlign & HDF_LEFT
	{
		uiTextFlags |= DT_LEFT;
	}

	if (dwFlags & BCGP_GRID_ITEM_VTOP)
	{
		uiTextFlags |= DT_TOP;
	}
	else if (dwFlags & BCGP_GRID_ITEM_VBOTTOM)
	{
		uiTextFlags |= DT_BOTTOM;
	}
	else // dwFlags & BCGP_GRID_ITEM_VCENTER
	{
		uiTextFlags |= DT_VCENTER;
	}

	if (!(dwFlags & BCGP_GRID_ITEM_MULTILINE))
	{
		uiTextFlags |= DT_SINGLELINE;
	}
	else
	{
		if (dwFlags & BCGP_GRID_ITEM_WORDWRAP)
		{
			uiTextFlags |= DT_WORDBREAK;
		}
	}

	m_bValueIsTrancated = pWndList->DoDrawText (pDC, strText, rect, uiTextFlags);

	if (clrTextOld != (COLORREF)-1)
	{
		pDC->SetTextColor (clrTextOld);
	}
}
//******************************************************************************************
void CBCGPGridItem::OnDrawIcon (CDC* pDC, CRect& rect)
{
	if (m_iImage < 0 && (m_nDataIconPerc < 0 || m_nDataIconPerc > 100))
	{
		return;
	}

	ASSERT_VALID (m_pGridRow);

	CBCGPGridCtrl* pWndList = m_pGridRow->m_pWndList;
	ASSERT_VALID (pWndList);

	if (m_nDataIconPerc >= 0 && m_nDataIconPerc <= 100 && !m_bInPlaceEdit)
	{
		OnDrawStateIcon(pDC, rect);
	}

	if (pWndList->GetImageList () != NULL && m_iImage >= 0)
	{
		int cx = 0;
		int cy = 0;

		VERIFY (::ImageList_GetIconSize (*pWndList->m_pImages, &cx, &cy));

		if (rect.left + cx <= rect.right)
		{
			CPoint pt = rect.TopLeft ();

			pt.x++;
			pt.y = (rect.top + 1 + rect.bottom - cy) / 2;

			VERIFY (pWndList->m_pImages->Draw (pDC, m_iImage, pt, ILD_NORMAL));

			rect.left += cx;
		}
	}
}
//******************************************************************************************
void CBCGPGridItem::OnDrawStateIcon (CDC* pDC, CRect& rect)
{
	ASSERT_VALID (m_pGridRow);

	if (m_nDataIconPerc < 0)
	{
		return;
	}

	CBCGPGridCtrl* pWndList = m_pGridRow->m_pWndList;
	ASSERT_VALID (pWndList);

	CBCGPGridDataStateIconSet* pInfo = NULL;
	CBCGPToolBarImages* pImages = pWndList->GetCustomDataIconSet (m_nIdColumn, &pInfo);

	if (pImages == NULL || !pImages->IsValid())
	{
		// Use default icons
		pImages = &pWndList->m_StateImages;
		pInfo = NULL;
	}

	ASSERT_VALID (pImages);

	if (pImages->GetCount() != 0)
	{
		int iIcon = bcg_clamp(pImages->GetCount () * m_nDataIconPerc / 100, 0, pImages->GetCount () - 1);

		if (pInfo != NULL && pInfo->m_bReverseOrder) // Reverse Order
		{
			iIcon = pImages->GetCount () - 1 - iIcon;
		}

		BOOL bRightAlign = (pInfo != NULL && pInfo->m_Placement == CBCGPGridDataStateIconSet::ImagePlacementHorzRight);

		CRect rectIcon = rect;
		rectIcon.DeflateRect(TEXT_MARGIN, 0);
		rectIcon.top++;

		if (pImages->GetImageSize().cx > rectIcon.Width())
		{
			if (bRightAlign)
			{
				rectIcon.left = rect.left;
			}
			else
			{
				rectIcon.right = rect.right;
			}
		}
		
		pImages->DrawEx (pDC, rectIcon, iIcon, 
			bRightAlign ? CBCGPToolBarImages::ImageAlignHorzRight : CBCGPToolBarImages::ImageAlignHorzLeft, 
			CBCGPToolBarImages::ImageAlignVertCenter);

		if (bRightAlign)
		{
			rect.right -= pImages->GetImageSize().cx + TEXT_MARGIN;
		}
		else
		{
			rect.left += pImages->GetImageSize().cx + TEXT_MARGIN;
		}
	}
}
//******************************************************************************************
COLORREF CBCGPGridItem::OnFillBackground (CDC* pDC, CRect rect)
{
	CBCGPGridCtrl* pWndList = m_pGridRow->m_pWndList;
	ASSERT_VALID (pWndList);

	COLORREF clrText = m_clrText != (COLORREF)-1 ? m_clrText : pWndList->m_ColorData.m_clrText;

	CBCGPGridItem* pCurSelItem = pWndList->GetCurSelItem ();

	BOOL bActiveItem =	pCurSelItem == this && 
						pWndList->AllowInPlaceEdit ();

	if (GetMergedCells () != NULL && pCurSelItem != NULL)
	{
		if (pCurSelItem->GetMergedCells () == GetMergedCells () &&
			pWndList->AllowInPlaceEdit ())
		{
			bActiveItem = TRUE;
		}
	}

	BOOL bNoHighlightActiveItem = !pWndList->IsHighlightActiveItem ();// || 
		//pWndList->IsSelectionBorderEnabled (); // BUG
	BOOL bSelectionBorderActiveItem = pWndList->IsSelectionBorderForActiveItem () && 
		!pWndList->IsSelectionBorderEnabled ();

	CRect rectFill = rect;
	rectFill.top++;

	BOOL bSelected = IsChangeSelectedBackground() && (
		(pWndList->m_bSingleSel && pWndList->m_bWholeRowSel) ?
			m_pGridRow->IsSelected () :
			IsSelected ());

	if (bSelected &&
		m_pGridRow->HasValueField () && 
		!(bActiveItem && bNoHighlightActiveItem || m_bInPlaceEdit))
	{
		if (pWndList->m_bFocused && pWndList->IsWholeRowSel ())
		{
			rectFill.right++;
		}
		clrText = pWndList->OnFillSelItem (pDC, rectFill, this);

		if (m_nDataColorScalePerc != -1 && 
			clrText != (COLORREF)-1 && CBCGPDrawManager::IsLightColor(clrText))
		{
			clrText = globalData.clrBtnText;
		}

	}
	else if (bActiveItem && bNoHighlightActiveItem)
	{
		if (!m_rectButton.IsRectEmpty ())
		{
			rectFill.right = m_rectButton.left;
		}

		if (bSelectionBorderActiveItem || pWndList->IsSelectionBorderEnabled () &&
			(pWndList->m_lstSel.GetCount () > 1 || pWndList->IsGrouping ()))
		{
			pDC->FillRect (rectFill, &globalData.brBlack);

			rectFill.DeflateRect (1, 1);
		}

		if (pWndList->m_brBackground.GetSafeHandle () != NULL)
		{
			pDC->FillRect (rectFill, &pWndList->m_brBackground);
		}
		else
		{
			COLORREF clr = visualManager->OnFillGridItem (
				pWndList, pDC, rectFill, bSelected, bActiveItem, FALSE);
			if (clrText == (COLORREF)-1)
			{
				clrText = clr;
			}
		}
	}
	else 
	{
		CBCGPGridItemID id = pWndList->GetGridItemID (this);
		BOOL bCustomColors = FALSE;

		// Item has own color - first priority
		if (m_clrBackground != (COLORREF)-1)
		{
			CBrush br (m_clrBackground);
			pDC->FillRect (rectFill, &br);
			bCustomColors = TRUE;
		}

		// Use m_ColorData to get colors
		else if (!id.IsNull ())
		{
			if (pWndList->OnAlternateColor (id))
			{
				bCustomColors = pWndList->m_ColorData.m_EvenColors.Draw (pDC, rectFill);
				if (m_clrText == (COLORREF)-1 &&
					pWndList->m_ColorData.m_EvenColors.m_clrText != (COLORREF)-1)
				{
					clrText = pWndList->m_ColorData.m_EvenColors.m_clrText;
				}
			}
			else
			{
				bCustomColors = pWndList->m_ColorData.m_OddColors.Draw (pDC, rectFill);
				if (m_clrText == (COLORREF)-1 &&
					pWndList->m_ColorData.m_OddColors.m_clrText != (COLORREF)-1)
				{
					clrText = pWndList->m_ColorData.m_OddColors.m_clrText;
				}
			}
		}

		if (!bCustomColors)
		{
			// If the column of this item is sorted
			BOOL bSortedColumn = (pWndList->m_bMarkSortedColumn &&
				!id.IsNull () &&
				(pWndList->GetColumnsInfo ().GetColumnState (id.m_nColumn) != 0));

			COLORREF clr = visualManager->OnFillGridItem (
				pWndList, pDC, rectFill, bSelected, bActiveItem && bNoHighlightActiveItem, bSortedColumn);
			if (clrText == (COLORREF)-1)
			{
				clrText = clr;
			}
		}
	}

	return clrText;
}
//******************************************************************************************
void CBCGPGridItem::OnDrawButton (CDC* pDC, CRect rect)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);
	ASSERT_VALID (m_pGridRow);

	CBCGPGridCtrl* pWndList = m_pGridRow->m_pWndList;
	ASSERT_VALID (pWndList);

	CBCGPToolbarComboBoxButton button;

	pDC->FillRect (rect, pWndList->m_bControlBarColors ?
		&globalData.brBarFace : &globalData.brBtnFace);

	if (m_dwFlags & BCGP_GRID_ITEM_HAS_LIST)
	{
		visualManagerMFC->OnDrawComboDropButton (pDC,
			rect, !m_bEnabled, m_bButtonIsDown, m_bButtonIsDown, &button);
		return;
	}

	CString str = _T("...");
	pDC->DrawText (str, rect, DT_SINGLELINE | DT_CENTER | DT_VCENTER);

#ifndef _BCGPGRID_STANDALONE
#ifndef _BCGSUITE_
	CBCGPVisualManager::BCGBUTTON_STATE state = 
					m_bButtonIsDown ?
					CBCGPVisualManager::ButtonsIsPressed :
					CBCGPVisualManager::ButtonsIsRegular;
#else
	CMFCVisualManager::AFX_BUTTON_STATE state = 
					m_bButtonIsDown ?
					CMFCVisualManager::ButtonsIsPressed :
					CMFCVisualManager::ButtonsIsRegular;
#endif
	visualManagerMFC->OnDrawButtonBorder (pDC, &button, rect, state);
#else
	COLORREF colorBorder = globalData.IsHighContastMode () ?
		globalData.clrBtnDkShadow : globalData.clrHilite;
	pDC->Draw3dRect (rect, colorBorder, colorBorder);
#endif
}
//******************************************************************************************
void CBCGPGridItem::OnDrawBorders (CDC* pDC, CRect rect)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);

	CBCGPGridCtrl* pWndList = GetOwnerList ();
	if (pWndList->GetSafeHwnd () == NULL)
	{
		return;
	}

	if (!pWndList->IsGridLinesEnabled())
	{
		return;
	}

	GRID_BORDERS borders;
	OnGetBorders (borders);

	CPen* pOldPen = NULL;

	if (borders.top != GRID_BORDERSTYLE_EMPTY)
	{
	}

	if (borders.left != GRID_BORDERSTYLE_EMPTY)
	{
	}

	if (borders.bottom != GRID_BORDERSTYLE_EMPTY)
	{
		CPen* pPen = pDC->SelectObject (&pWndList->m_penHLine);
		if (pOldPen == NULL)
		{
			pOldPen = pPen;
		}

		pDC->MoveTo (rect.left, rect.bottom);
		pDC->LineTo (rect.right + 1, rect.bottom);

		if (borders.top == GRID_BORDERSTYLE_EMPTY && pWndList->GetLeftItemBorderOffset () > rect.left)
		{
			// repeat bottom border of the previous row (draw top border)
			pDC->MoveTo (rect.left, rect.top);
			pDC->LineTo (rect.right + 1, rect.top);
		}
	}

	if (borders.right != GRID_BORDERSTYLE_EMPTY)
	{
		CPen* pPen = pDC->SelectObject (&pWndList->m_penVLine);
		if (pOldPen == NULL)
		{
			pOldPen = pPen;
		}

		pDC->MoveTo (rect.right, rect.top);
		pDC->LineTo (rect.right, rect.bottom);
	}

	if (pOldPen != NULL)
	{
		pDC->SelectObject (pOldPen);
	}
}
//******************************************************************************************
void CBCGPGridItem::OnPrintValue (CDC* pDC, CRect rect)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);
	ASSERT_VALID (m_pGridRow);

	CBCGPGridCtrl* pWndList = m_pGridRow->m_pWndList;
	ASSERT_VALID (pWndList);
	ASSERT (pWndList->m_bIsPrinting);

	// map to printer metrics
	HDC hDCFrom = ::GetDC(NULL);
	int nXMul = pDC->GetDeviceCaps(LOGPIXELSX); // pixels in print dc
	int nXDiv = ::GetDeviceCaps(hDCFrom, LOGPIXELSX); // pixels in screen dc
	::ReleaseDC(NULL, hDCFrom);

	const int CALCULATED_TEXT_MARGIN = ::MulDiv (TEXT_MARGIN, nXMul, nXDiv);
	const CRect& rectClip = pWndList->m_PrintParams.m_pageInfo.m_rectPageItems;

	// -----------
	// Draw value:
	// -----------
	COLORREF clrTextOld = pDC->SetTextColor (pWndList->m_clrPrintText);

	CRect rectText = rect;
	rectText.DeflateRect (CALCULATED_TEXT_MARGIN, 0);

	// If merged, refer to main item
	CBCGPGridItem* pMergedItem = GetMergedMainItem ();
	const CString& strText = (pMergedItem != NULL) ? pMergedItem->GetLabel () : GetLabel ();
	const DWORD dwFlags = (pMergedItem != NULL) ? pMergedItem->GetFlags () : GetFlags ();
	int nTextAlign = (pMergedItem != NULL) ? pMergedItem->GetAlign () : GetAlign ();

	UINT uiTextFlags = DT_NOPREFIX | DT_END_ELLIPSIS;
	
	if (nTextAlign & HDF_CENTER)
	{
		uiTextFlags |= DT_CENTER;
	}
	else if (nTextAlign & HDF_RIGHT)
	{
		uiTextFlags |= DT_RIGHT;
	}
	else // nTextAlign & HDF_LEFT
	{
		uiTextFlags |= DT_LEFT;
	}
	
	if (dwFlags & BCGP_GRID_ITEM_VTOP)
	{
		uiTextFlags |= DT_TOP;
	}
	else if (dwFlags & BCGP_GRID_ITEM_VBOTTOM)
	{
		uiTextFlags |= DT_BOTTOM;
	}
	else // dwFlags & BCGP_GRID_ITEM_VCENTER
	{
		uiTextFlags |= DT_VCENTER;
	}
	
	if (!(dwFlags & BCGP_GRID_ITEM_MULTILINE))
	{
		uiTextFlags |= DT_SINGLELINE;
	}
	else
	{
		if (dwFlags & BCGP_GRID_ITEM_WORDWRAP)
		{
			uiTextFlags |= DT_WORDBREAK;
		}
	}

	CRect rectClipText = rectText;
	rectClipText.NormalizeRect ();
	if (rectClipText.IntersectRect (&rectClipText, &rectClip))
	{
		pWndList->DoDrawText (pDC, strText, rectText, uiTextFlags, rectClipText);
	}

	pDC->SetTextColor (clrTextOld);
}
//*****************************************************************************************
void CBCGPGridItem::OnPrintBorders (CDC* pDC, CRect rect)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pDC);
	ASSERT_VALID (m_pGridRow);
	
	CBCGPGridCtrl* pWndList = m_pGridRow->m_pWndList;
	ASSERT_VALID (pWndList);
	ASSERT (pWndList->m_bIsPrinting);

	if (pWndList->IsKindOf (RUNTIME_CLASS (CBCGPReportCtrl)))
	{
		return;
	}

	const CRect& rectClip = pWndList->m_PrintParams.m_pageInfo.m_rectPageItems;
	CRect rectBorders = rect;
	rectBorders.NormalizeRect ();

	if (!rectBorders.IntersectRect(&rectBorders, &rectClip))
	{
		return;
	}

	// top border:
	if (rect.top >= rectClip.top && rect.top <= rectClip.bottom)
	{
		pDC->MoveTo (rectBorders.left, rect.top);
		pDC->LineTo (rectBorders.right, rect.top);
	}
	
	// left border:
	if (rect.left >= rectClip.left && rect.left <= rectClip.right)
	{
		pDC->MoveTo (rect.left, rectBorders.top);
		pDC->LineTo (rect.left, rectBorders.bottom);
	}
	
	// bottom border:
	if (rect.bottom >= rectClip.top && rect.bottom <= rectClip.bottom)
	{
		pDC->MoveTo (rectBorders.left, rect.bottom);
		pDC->LineTo (rectBorders.right, rect.bottom);
	}
	
	// right border:
	if (rect.right >= rectClip.left && rect.right <= rectClip.right)
	{
		pDC->MoveTo (rect.right, rectBorders.top);
		pDC->LineTo (rect.right, rectBorders.bottom);
	}
}
//*****************************************************************************************
void CBCGPGridItem::Serialize (CArchive& ar)
{
	if (ar.IsStoring ())
	{
		WriteToArchive (ar);
	}
	else
	{
		ReadFromArchive (ar, FALSE);
	}
}
//*****************************************************************************************
BOOL CBCGPGridItem::ReadFromArchive(CArchive& ar, BOOL bTestMode)
{
	ASSERT_VALID (this);

	COleVariant variant;
	ar >> variant;
	_variant_t	varValue = variant;	// Item value

	DWORD_PTR		dwData;			// User-defined data
	ar >> dwData;

	CString			strEditMask;	// Item edit mask (see CBCGPMaskEdit for description)
	CString			strEditTempl;	// Item edit template (see CBCGPMaskEdit for description)
	CString			strValidChars;	// Item edit valid chars (see CBCGPMaskEdit for description)
	ar >> strEditMask;
	ar >> strEditTempl;
	ar >> strValidChars;
	
	CStringList		lstOptions;	// List of combobox items
	int nListCount = 0;
	ar >> nListCount;
	for (int i = 0; i < nListCount; i++)
	{
		CString str;
		ar >> str;
		lstOptions.AddTail (str);
	}

	CSize			sizeCombo;	// Dimension of listbox	(400)
	ar >> sizeCombo.cx;
	ar >> sizeCombo.cy;
	
	BOOL			bEnabled;		// Is item enabled?
	BOOL			bAllowEdit;		// Is item editable?
	DWORD			dwFlags;		// Item flags
	ar >> bEnabled;
	ar >> bAllowEdit;
	ar >> dwFlags;
	
	int				nMinValue;
	int				nMaxValue;
	ar >> nMinValue;
	ar >> nMaxValue;
	
	COLORREF		clrBackground;	// Custom item background color
	COLORREF		clrText;		// Custom item foreground color
	ar >> clrBackground;
	ar >> clrText;
	
	int				iImage;		// Image index
	ar >> iImage;

	if (!bTestMode)
	{
		m_varValue = varValue;
		m_dwData = dwData;
		m_strEditMask = strEditMask;
		m_strEditTempl = strEditTempl;
		m_strValidChars = strValidChars;
		m_lstOptions.RemoveAll ();
		m_lstOptions.AddTail (&lstOptions);
		m_sizeCombo = sizeCombo;
		m_bEnabled = bEnabled;
		m_bAllowEdit = bAllowEdit;
		m_dwFlags = dwFlags;
		m_nMinValue = nMinValue;
		m_nMaxValue = nMaxValue;
		m_clrBackground = clrBackground;
		m_clrText = clrText;
		m_iImage = iImage;
	}

	return TRUE;
}
//*****************************************************************************************
void CBCGPGridItem::WriteToArchive(CArchive& ar)
{
	ASSERT_VALID (this);

	COleVariant variant = m_varValue; // Item value
	ar << variant;

	ar << (DWORD_PTR)m_dwData;	// User-defined data
	ar << m_strEditMask;	// Item edit mask (see CBCGPMaskEdit for description)
	ar << m_strEditTempl;	// Item edit template (see CBCGPMaskEdit for description)
	ar << m_strValidChars;	// Item edit valid chars (see CBCGPMaskEdit for description)
	
	// List of combobox items
	ar << (int) m_lstOptions.GetCount ();
	for (POSITION pos = m_lstOptions.GetHeadPosition (); pos != NULL; )
	{
		ar << m_lstOptions.GetNext (pos);
	}

	ar << m_sizeCombo;		// Dimension of listbox	(400)
	
	ar << m_bEnabled;		// Is item enabled?
	ar << m_bAllowEdit;		// Is item editable?
	ar << m_dwFlags;		// Item flags
	
	ar << m_nMinValue;
	ar << m_nMaxValue;
	
	ar << m_clrBackground;	// Custom item background color
	ar << m_clrText;		// Custom item foreground color
	
	ar << m_iImage;			// Image index
}
//******************************************************************************************
BOOL CBCGPGridItem::ClearContent (BOOL bRedraw)
{
	ASSERT_VALID (this);

	// 	if (m_varValue.vt != VT_EMPTY && !CanChangeType ())
	// 	{
	// 		return FALSE;
	// 	}

	EmptyValue (bRedraw);

	m_dwData = 0;
	m_strEditMask.Empty ();
	m_strEditTempl.Empty ();
	m_strValidChars.Empty ();

	m_lstOptions.RemoveAll ();

	m_sizeCombo.cx = 50;
	m_sizeCombo.cy = 400;

	m_bEnabled = TRUE;
	m_bAllowEdit = TRUE;
	m_dwFlags = 0;

	m_nMinValue = 0;
	m_nMaxValue = 0;
	
	m_clrBackground = (COLORREF)-1;
	m_clrText = (COLORREF)-1;
	
	m_iImage = -1;
	
	SetItemChanged ();

	return TRUE;
}
//******************************************************************************************
BOOL CBCGPGridItem::ChangeType (const _variant_t& var)
{
	try
	{
		m_varValue.ChangeType (var.vt, &var);
	}
	catch (...)
	{
		return FALSE;		
	}

	return TRUE;
}
//******************************************************************************************
BOOL CBCGPGridItem::CanUpdateData () const
{
	if (m_pGridRow == NULL || m_pGridRow->m_pWndList == NULL)
	{
		return TRUE;
	}
	return m_pGridRow->m_pWndList->m_bUpdateItemData;
}
//******************************************************************************************
BOOL CBCGPGridItem::OnUpdateValue ()
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pWndInPlace);
	ASSERT (::IsWindow (m_pWndInPlace->GetSafeHwnd ()));

	CString strText;
	m_pWndInPlace->GetWindowText (strText);

	// If merged, refer to main item
	CBCGPGridItem* pMergedItem = GetMergedMainItem ();
	CBCGPGridItem* pUpdateItem = (pMergedItem != NULL) ? pMergedItem : this;
	ASSERT_VALID (pUpdateItem);

	BOOL bRes = TRUE;
	BOOL bIsChanged = FormatItem () != strText;
	BOOL bUpdateData = bIsChanged && pUpdateItem->CanUpdateData ();

	if (bUpdateData)
	{
		bRes = pUpdateItem->TextToVar (strText);
	}

	if (bRes && bUpdateData)
	{
		pUpdateItem->SetItemChanged ();
	}

	return bRes;
}
//******************************************************************************************
void CBCGPGridItem::SetItemChanged ()
{
	ASSERT_VALID (m_pGridRow);

		CBCGPGridItemID id = GetGridItemID ();
		m_pGridRow->OnItemChanged (this, id.m_nRow, id.m_nColumn);
		m_bIsChanged = TRUE;
}
//******************************************************************************************
BOOL CBCGPGridItem::TextToVar (const CString& strText)
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pGridRow);

	CBCGPGridCtrl* pWndList = m_pGridRow->m_pWndList;
	ASSERT_VALID (pWndList);

	switch (m_varValue.vt)
	{
	case VT_BSTR:
		m_varValue = (LPCTSTR) strText;
		return TRUE;

	case VT_UI1:
		{
			int nVal = _ttoi (strText);
			if (nVal < 0 || nVal > 255)
			{
				return FALSE;
			}

			m_varValue = (BYTE) nVal;
		}
		return TRUE;

	case VT_DECIMAL:
		{
			DECIMAL num;
			if (globalUtils.DecimalFromString (num, strText))
			{
				m_varValue = num;
				return TRUE;
			}

			return FALSE;
		}
		break;

	case VT_CY:
		{
			CY cy;
			if (globalUtils.CyFromString (cy, strText))
			{
				m_varValue = cy;
				return TRUE;
			}

			return FALSE;
		}

	case VT_I2:
		m_varValue = (short) _ttoi (strText);
		return TRUE;

	case VT_INT:
	case VT_I4:
		m_varValue = _ttol (strText);
		return TRUE;

	case VT_UI2:
		m_varValue.uiVal = unsigned short (_ttoi (strText));
		return TRUE;

	case VT_UINT:
	case VT_UI4:
		m_varValue.ulVal = unsigned long (_ttol (strText));
		return TRUE;

#if _MSC_VER >= 1500
	case VT_I8:
		m_varValue = _variant_t ( _ttoi64 (strText) );
		return TRUE;

	case VT_UI8:
		m_varValue = ULONGLONG (_ttoi64 (strText));
		return TRUE;
#else
	case VT_I8:
	case VT_UI8:
		return FALSE;
#endif

	case VT_R4:
		{
			float fVal = 0.;
			if (!strText.IsEmpty ())
			{
#if _MSC_VER < 1400
				_stscanf (strText, CBCGPGridRow::m_strScanFormatFloat, &fVal);
#else
				_stscanf_s (strText, CBCGPGridRow::m_strScanFormatFloat, &fVal);
#endif
			}

			m_varValue = fVal;
		}
		return TRUE;

	case VT_R8:
		{
			double dblVal = 0.;
			if (!strText.IsEmpty ())
			{
#if _MSC_VER < 1400
				_stscanf (strText, CBCGPGridRow::m_strScanFormatDouble, &dblVal);
#else
				_stscanf_s (strText, CBCGPGridRow::m_strScanFormatDouble, &dblVal);
#endif
			}

			m_varValue = dblVal;
		}
		return TRUE;

	case VT_BOOL:
		m_varValue = (bool) (strText == pWndList->m_strTrue);
		return TRUE;
	}

	return FALSE;
}
//*****************************************************************************
BOOL CBCGPGridItem::OnEdit (LPPOINT)
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pGridRow);

	if (!m_bAllowEdit)
	{
		return FALSE;
	}

	CBCGPGridCtrl* pWndList = m_pGridRow->m_pWndList;
	ASSERT_VALID (pWndList);

	if (m_bReadOnly && (pWndList->m_dwBeginEditReason & CBCGPGridCtrl::BeginEdit_Char) != 0)
	{
		if ((m_dwFlags & BCGP_GRID_ITEM_HAS_LIST) == 0 && m_lstOptions.GetCount () == 0)
		{
			return FALSE;
		}
	}

	m_pWndInPlace = NULL;

	OnBeginInplaceEdit ();

	CRect rectEdit;
	CRect rectSpin;

	AdjustInPlaceEditRect (rectEdit, rectSpin);

	BOOL bDefaultFormat = FALSE;
	m_pWndInPlace = CreateInPlaceEdit (rectEdit, bDefaultFormat);

	pWndList->OnAfterInplaceEditCreated (this, m_pWndInPlace);

	if (m_pWndInPlace != NULL)
	{
		if (bDefaultFormat)
		{
			m_pWndInPlace->SetWindowText (FormatItem ());
		}

		if (m_dwFlags & BCGP_GRID_ITEM_HAS_LIST)
		{
			CRect rectCombo = m_Rect;
			rectCombo.left = rectEdit.left - 4;

			m_pWndCombo = CreateCombo (pWndList, rectCombo);
			ASSERT_VALID (m_pWndCombo);

			SetComboFont ();

			//-------------------------------------------------------------------
			// Synchronize bottom edge of the combobox with the item bottom edge:
			//-------------------------------------------------------------------
			m_pWndCombo->GetWindowRect (rectCombo);
			pWndList->ScreenToClient (&rectCombo);

			int dy = rectCombo.Height () - m_Rect.Height ();

			m_pWndCombo->SetWindowPos (NULL, rectCombo.left,
				rectCombo.top - dy + 1, -1, -1, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

			if (m_varValue.vt == VT_BOOL)
			{
				m_lstOptions.AddTail (pWndList->m_strTrue);
				m_lstOptions.AddTail (pWndList->m_strFalse);
			}

			for (POSITION pos = m_lstOptions.GetHeadPosition (); pos != NULL;)
			{
				m_pWndCombo->AddString (m_lstOptions.GetNext (pos));
			}
		}

		if (m_dwFlags & BCGP_GRID_ITEM_HAS_SPIN)
		{
			m_pWndSpin = CreateSpinControl (rectSpin);
		}

		SetInPlaceEditFont ();
		m_pWndInPlace->SetFocus ();

		if (!m_bAllowEdit)
		{
			m_pWndInPlace->HideCaret ();
		}

		m_bInPlaceEdit = TRUE;
		return TRUE;
	}

	return FALSE;
}
//******************************************************************************************
void CBCGPGridItem::SetInPlaceEditFont ()
{
	ASSERT_VALID (this);

	CBCGPGridCtrl* pWndList = GetOwnerList ();

	if (m_pWndInPlace != NULL && pWndList != NULL)
	{
		ASSERT_VALID (m_pWndInPlace);
		ASSERT_VALID (pWndList);
		m_pWndInPlace->SetFont (pWndList->GetFont ());
	}
}
//******************************************************************************************
void CBCGPGridItem::SetComboFont ()
{
	ASSERT_VALID (this);
	
	CBCGPGridCtrl* pWndList = GetOwnerList ();
	
	if (m_pWndCombo != NULL && pWndList != NULL)
	{
		ASSERT_VALID (m_pWndCombo);
		ASSERT_VALID (pWndList);
		m_pWndCombo->SetFont (pWndList->GetFont ());
	}
}
//******************************************************************************************
void CBCGPGridItem::AdjustButtonRect ()
{
	CBCGPGridCtrl* pWndList = m_pGridRow->m_pWndList;
	ASSERT_VALID (pWndList);

	if (pWndList->AllowInPlaceEdit ())
	{
		m_rectButton = m_Rect;
		m_rectButton.left = m_rectButton.right - pWndList->GetButtonWidth () + 3;
		m_rectButton.top ++;
	}
	else
	{
		m_rectButton.SetRectEmpty ();
	}
}
//******************************************************************************************
void CBCGPGridItem::AdjustInPlaceEditRect (CRect& rectEdit, CRect& rectSpin)
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pGridRow);
	CBCGPGridCtrl* pWndList = m_pGridRow->m_pWndList;
	ASSERT_VALID (pWndList);

	if (GetMergedCells () != NULL)
	{
		rectEdit = GetMergedRect ();
		if (rectEdit.top < pWndList->m_rectList.top)
		{
			rectEdit.top = min (rectEdit.bottom, pWndList->m_rectList.top);
		}
		if (rectEdit.bottom > pWndList->m_rectList.bottom)
		{
			rectEdit.bottom = max (rectEdit.top, pWndList->m_rectList.bottom);
		}
	}
	else
	{
		rectEdit = m_Rect;
	}

	if (rectEdit.left < pWndList->m_rectList.left)
	{
		rectEdit.left = pWndList->m_rectList.left;
	}
	if (rectEdit.right > pWndList->m_rectList.right)
	{
		rectEdit.right = max (rectEdit.left, pWndList->m_rectList.right);
	}

	rectEdit.DeflateRect (0, TEXT_VMARGIN);

	if (m_iImage >= 0 && pWndList->GetImageList () != NULL)
	{
		int cx = 0;
		int cy = 0;

		VERIFY (::ImageList_GetIconSize (*pWndList->m_pImages, &cx, &cy));

		rectEdit.left += cx;
	}

	int nMargin = pWndList->m_nEditLeftMargin;

	rectEdit.left += TEXT_MARGIN - nMargin;

	if (HasButton ())
	{
		AdjustButtonRect ();
		rectEdit.right = m_rectButton.left - 1;
	}
	else
	{
		rectEdit.right -= TEXT_MARGIN - pWndList->m_nEditRightMargin;
	}

	if (m_dwFlags & BCGP_GRID_ITEM_HAS_SPIN)
	{
		rectSpin = m_Rect;
		rectSpin.right = rectEdit.right;
		rectSpin.left = rectSpin.right - pWndList->GetButtonWidth ();
		rectSpin.top ++;
		rectEdit.right = rectSpin.left;
		rectSpin.DeflateRect (0, 1);
	}
	else
	{
		rectSpin.SetRectEmpty ();
	}
}
//******************************************************************************************
CWnd* CBCGPGridItem::CreateInPlaceEdit (CRect rectEdit, BOOL& bDefaultFormat)
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pGridRow);

	CBCGPGridCtrl* pWndList = m_pGridRow->m_pWndList;
	ASSERT_VALID (pWndList);

	switch (m_varValue.vt)
	{
	case VT_EMPTY:
		if (!pWndList->OnEditEmptyValue (m_pGridRow->GetRowId (), GetColumnId (), this))
		{
			return NULL;
		}
		break;
		
	case VT_BSTR:
	case VT_R4:
	case VT_R8:
	case VT_UI1:
	case VT_I2:
	case VT_INT:
	case VT_UINT:
	case VT_I4:
	case VT_I8:
	case VT_UI8:
	case VT_UI2:
	case VT_UI4:
	case VT_BOOL:
		break;

	}

	CEdit* pWndEdit = NULL;

	if (!m_strEditMask.IsEmpty () || !m_strEditTempl.IsEmpty () ||
		!m_strValidChars.IsEmpty ())
	{
		CBCGPMaskEdit* pWndEditMask = new CBCGPMaskEdit;

		if (!m_strEditMask.IsEmpty () && !m_strEditTempl.IsEmpty ())
		{
			pWndEditMask->EnableMask (m_strEditMask, m_strEditTempl, _T(' '));
		}

		if (!m_strValidChars.IsEmpty ())
		{
			pWndEditMask->SetValidChars (m_strValidChars);
		}
		pWndEditMask->EnableSetMaskedCharsOnly (FALSE);
		pWndEditMask->EnableGetMaskedCharsOnly (FALSE);

		pWndEdit = pWndEditMask;
	}
	else
	{
		pWndEdit = NewInPlaceEdit ();
	}

	DWORD dwStyle = WS_VISIBLE | WS_CHILD | ES_AUTOVSCROLL;

	if (IsMultiline ())
	{
		dwStyle |= ES_MULTILINE;
	}

	if (!IsWordWrap ())
	{
		dwStyle |= ES_AUTOHSCROLL; // no wrapping, use scrolling instead
	}

	if (!m_bEnabled || !m_bAllowEdit || m_bReadOnly)
	{
		dwStyle |= ES_READONLY;
	}

	switch (GetAlign ())
	{
	case HDF_RIGHT:
		dwStyle |= ES_RIGHT;
		break;

	case HDF_CENTER:
		dwStyle |= ES_CENTER;
		break;
	}

	pWndEdit->Create (dwStyle, rectEdit, pWndList, BCGPGRIDCTRL_ID_INPLACE);

	if (m_bReadOnly)
	{
		pWndEdit->SetReadOnly (TRUE);
	}

	bDefaultFormat = TRUE;
	return pWndEdit;
}
//*****************************************************************************
CEdit* CBCGPGridItem::NewInPlaceEdit ()
{
	return new CBCGPEdit;
}
//*****************************************************************************
CSpinButtonCtrl* CBCGPGridItem::CreateSpinControl (CRect rectSpin)
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pGridRow);

	CBCGPGridCtrl* pWndList = m_pGridRow->m_pWndList;
	ASSERT_VALID (pWndList);

	CSpinButtonCtrl* pWndSpin = new CBCGPSpinButtonCtrl;

	if (!pWndSpin->Create (
		WS_CHILD | WS_VISIBLE | UDS_ARROWKEYS | UDS_SETBUDDYINT | UDS_NOTHOUSANDS,
		rectSpin, pWndList, BCGPGRIDCTRL_ID_INPLACE))
	{
		return NULL;
	}

	pWndSpin->SetBuddy (m_pWndInPlace);

	if (m_nMinValue != 0 || m_nMaxValue != 0)
	{
		pWndSpin->SetRange32 (m_nMinValue, m_nMaxValue);
	}

	return pWndSpin;
}
//*****************************************************************************
BOOL CBCGPGridItem::OnEndEdit ()
{
	ASSERT_VALID (this);

	m_bInPlaceEdit = FALSE;
	OnDestroyWindow ();

	OnEndInplaceEdit ();
	return TRUE;
}
//*****************************************************************************************
void CBCGPGridItem::OnBeginInplaceEdit ()
{
	ASSERT_VALID (this);

	CBCGPGridCtrl* pWndList = GetOwnerList ();
	ASSERT_VALID (pWndList);

	pWndList->OnBeginInplaceEdit (this);
}
//*****************************************************************************************
void CBCGPGridItem::OnEndInplaceEdit ()
{
	ASSERT_VALID (this);
	
	CBCGPGridCtrl* pWndList = GetOwnerList ();
	
	if (pWndList->GetSafeHwnd () != NULL)
	{
		pWndList->OnEndInplaceEdit (this);
	}
}
//*****************************************************************************************
CComboBox* CBCGPGridItem::CreateCombo (CWnd* pWndParent, CRect rect)
{
	ASSERT_VALID (this);

	rect.right = max (rect.left + m_sizeCombo.cx, rect.right);
	rect.bottom = rect.top + m_sizeCombo.cy;

	CComboBox* pWndCombo = new CComboBox;
	if (!pWndCombo->Create (WS_CHILD | CBS_NOINTEGRALHEIGHT | CBS_DROPDOWNLIST | WS_VSCROLL, 
		rect, pWndParent, BCGPGRIDCTRL_ID_INPLACE))
	{
		delete pWndCombo;
		return NULL;
	}

	return pWndCombo;
}
//****************************************************************************************
void CBCGPGridItem::DoClickButton (CPoint point)
{
	ASSERT_VALID (this);

	CString strPrevVal = FormatItem ();

	CWaitCursor wait;
	OnClickButton (point);

	if (strPrevVal != FormatItem ())
	{
		SetItemChanged ();
	}
}
//****************************************************************************************
void CBCGPGridItem::OnClickButton (CPoint)
{
	ASSERT_VALID (this);

	if (m_pWndCombo != NULL)
	{
		m_bButtonIsDown = TRUE;
		Redraw ();

		CString str;
		m_pWndInPlace->GetWindowText (str);

		m_pWndCombo->SetCurSel (m_pWndCombo->FindStringExact (-1, str));

		m_pWndCombo->SetFocus ();
		m_pWndCombo->ShowDropDown ();
	}
}
//****************************************************************************************
BOOL CBCGPGridItem::OnClickValue (UINT uiMsg, CPoint point)
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pGridRow);

	CBCGPGridCtrl* pWndList = m_pGridRow->m_pWndList;
	ASSERT_VALID (pWndList);

	if (m_pWndInPlace == NULL)
	{
		return FALSE;
	}

	ASSERT_VALID (m_pWndInPlace);

	if (m_pWndSpin != NULL)
	{
		ASSERT_VALID (m_pWndSpin);
		ASSERT (m_pWndSpin->GetSafeHwnd () != NULL);

		CRect rectSpin;
		m_pWndSpin->GetClientRect (rectSpin);
		m_pWndSpin->MapWindowPoints (pWndList, rectSpin);

		if (rectSpin.PtInRect (point))
		{
			pWndList->MapWindowPoints (m_pWndSpin, &point, 1); 

			m_pWndSpin->SendMessage (uiMsg, 0, MAKELPARAM (point.x, point.y));
			return TRUE;
		}
	}

	CPoint ptEdit = point;
	::MapWindowPoints (	pWndList->GetSafeHwnd (), 
						m_pWndInPlace->GetSafeHwnd (), &ptEdit, 1);

	m_pWndInPlace->SendMessage (uiMsg, 0, MAKELPARAM (ptEdit.x, ptEdit.y));
	return TRUE;
}
//****************************************************************************************
BOOL CBCGPGridItem::OnDblClick (CPoint)
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pGridRow);

	if (m_pWndInPlace == NULL)
	{
		return FALSE;
	}

	ASSERT (::IsWindow (m_pWndInPlace->GetSafeHwnd ()));

	if (m_lstOptions.GetCount () > 1)
	{
		CString strText;
		m_pWndInPlace->GetWindowText (strText);

		POSITION pos = m_lstOptions.Find (strText);
		if (pos == NULL)
		{
			return FALSE;
		}

		m_lstOptions.GetNext (pos);
		if (pos == NULL)
		{
			pos = m_lstOptions.GetHeadPosition ();
		}

		ASSERT (pos != NULL);
		strText = m_lstOptions.GetAt (pos);

		m_pWndInPlace->SetWindowText (strText);
		OnUpdateValue ();

		return TRUE;
	}

	if (m_dwFlags & BCGP_GRID_ITEM_HAS_LIST)
	{
		CWaitCursor wait;

		CString strPrevVal = GetLabel ();

		OnClickButton (CPoint (-1, -1));

		if (strPrevVal != FormatItem ())
		{
			SetItemChanged ();
		}

		return TRUE;
	}

	return FALSE;
}
//*****************************************************************************************
void CBCGPGridItem::OnSelectCombo ()
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pWndCombo);
	ASSERT_VALID (m_pWndInPlace);

	int iSelIndex = m_pWndCombo->GetCurSel ();
	if (iSelIndex >= 0)
	{
		CString str;
		m_pWndCombo->GetLBText (iSelIndex, str);
		m_pWndInPlace->SetWindowText (str);
		OnUpdateValue ();
	}
}
//****************************************************************************************
void CBCGPGridItem::OnCloseCombo ()
{
	ASSERT_VALID (this);

	m_bButtonIsDown = FALSE;
	Redraw ();

	ASSERT_VALID (m_pWndInPlace);
	m_pWndInPlace->SetFocus ();

	ASSERT_VALID (m_pGridRow);
	CBCGPGridCtrl* pWndList = m_pGridRow->m_pWndList;

	ASSERT_VALID (pWndList);
	pWndList->DoInplaceEditSetSel (pWndList->OnInplaceEditSetSel (this, CBCGPGridCtrl::BeginEdit_ComboReturn));
}
//****************************************************************************************
BOOL CBCGPGridItem::OnSetCursor () const
{
	if (m_bInPlaceEdit)
	{
		return FALSE;
	}

	ASSERT_VALID (m_pGridRow);
	CBCGPGridCtrl* pWndList = GetOwnerList ();
	ASSERT_VALID (pWndList);

	BOOL bActiveItem =	pWndList->GetCurSelItem () == this;

	if (pWndList->AllowInPlaceEdit () && IsAllowEdit () &&
		(bActiveItem || pWndList->IsEditFirstClick ()))
	{
		switch (m_varValue.vt)
		{
		case VT_BSTR:
		case VT_R4:
		case VT_R8:
		case VT_UI1:
		case VT_I2:
		case VT_INT:
		case VT_UINT:
		case VT_I4:
		case VT_UI2:
		case VT_UI4:
		case VT_I8:
		case VT_UI8:
			SetCursor (AfxGetApp ()->LoadStandardCursor (IDC_IBEAM));
			return TRUE;
		}
	}

	return FALSE;
}
//*******************************************************************************************
BOOL CBCGPGridItem::PushChar (UINT nChar)
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pGridRow);
	ASSERT (m_pGridRow->IsSelected ());//ASSERT (m_pWndList->m_pSel == this);
	ASSERT_VALID (m_pWndInPlace);

	switch (m_varValue.vt)
	{
	case VT_BSTR:
    case VT_R4:
    case VT_R8:
    case VT_UI1:
    case VT_I2:
	case VT_INT:
	case VT_UINT:
	case VT_I4:
    case VT_UI2:
	case VT_UI4:
	case VT_I8:
	case VT_UI8:
		if (m_bEnabled && m_bAllowEdit && !m_bReadOnly)
		{
			if (nChar == VK_RETURN && 
				m_pGridRow->m_pWndList != NULL && !m_pGridRow->m_pWndList->m_bClearInplaceEditOnEnter)
			{
				m_pGridRow->m_pWndList->DoInplaceEditSetSel (
					m_pGridRow->m_pWndList->OnInplaceEditSetSel (this, CBCGPGridCtrl::BeginEdit_Return));
				return TRUE;
			}

			m_pWndInPlace->SetWindowText (_T(""));
			m_pWndInPlace->SendMessage (WM_CHAR, (WPARAM) nChar);
			return TRUE;
		}
	}

	if (!m_bAllowEdit || m_bReadOnly)
	{
		if (nChar == VK_SPACE)
		{
			OnDblClick (CPoint (-1, -1));
		}
		else if (m_lstOptions.GetCount () > 0)
		{
			CString strChar;
			strChar += (TCHAR) nChar;
			strChar.MakeUpper ();

			CString strText;
			m_pWndInPlace->GetWindowText (strText);

			POSITION pos = m_lstOptions.Find (strText);
			if (m_lstOptions.GetCount () > 1 && pos != NULL)
			{
				POSITION posSave = pos;

				m_lstOptions.GetNext (pos);

				while (pos != posSave)
				{
					if (pos == NULL)
					{
						pos = m_lstOptions.GetHeadPosition ();
					}

					if (pos == posSave)
					{
						break;
					}

					strText = m_lstOptions.GetAt (pos);
					
					CString strUpper = strText;
					strUpper.MakeUpper ();

					if (strUpper.Left (1) == strChar)
					{
						m_pWndInPlace->SetWindowText (strText);
						OnUpdateValue ();
						break;
					}

					m_lstOptions.GetNext (pos);
				}
			}
			else
			{
				for (pos = m_lstOptions.GetHeadPosition (); pos != NULL; )
				{
					strText = m_lstOptions.GetNext (pos);

					CString strUpper = strText;
					strUpper.MakeUpper ();

					if (strUpper.Left (1) == strChar)
					{
						m_pWndInPlace->SetWindowText (strText);
						OnUpdateValue ();
						break;
					}
				}
			}

		}
	}

	OnEndEdit ();

	CBCGPGridCtrl* pWndList = GetOwnerList ();
	ASSERT_VALID (pWndList);

	if (::GetCapture () == pWndList->GetSafeHwnd ())
	{
		ReleaseCapture ();
	}

	return FALSE;
}
//*******************************************************************************************
CString CBCGPGridItem::GetValueTooltip ()
{
	ASSERT_VALID (this);
	return m_bValueIsTrancated ? FormatItem () : _T("");
}
//*******************************************************************************************
CRect CBCGPGridItem::GetTooltipRect () const
{
	return GetRect ();
}
//*******************************************************************************************
void CBCGPGridItem::OnPosSizeChanged (CRect /*rectOld*/)
{
}
//*******************************************************************************************
void CBCGPGridItem::Merge (CBCGPGridMergedCells* pMergedCells)
{
	if (m_pMerged != NULL)
	{
		m_pMerged->Release ();
		m_pMerged = NULL;
	}

	if (pMergedCells != NULL)
	{
		pMergedCells->AddRef ();
		m_pMerged = pMergedCells;
	}

	m_bIsChanged = TRUE;
}
//*******************************************************************************************
CBCGPGridMergedCells* CBCGPGridItem::GetMergedCells ()
{
	return m_pMerged;
}
//*******************************************************************************************
CRect CBCGPGridItem::GetMergedRect ()
{
	CBCGPGridMergedCells* pMerged = GetMergedCells ();
	if (pMerged != NULL)
	{
		return pMerged->GetRect ();
	}

	CRect rect (0, 0, 0, 0);
	return rect;
}
//*******************************************************************************************
BOOL CBCGPGridItem::GetMergedRange (CBCGPGridRange& range)
{
	CBCGPGridMergedCells* pMerged = GetMergedCells ();
	if (pMerged != NULL)
	{
		range = pMerged->GetRange ();
		return TRUE;
	}

	return FALSE;
}
//*******************************************************************************************
CBCGPGridItem* CBCGPGridItem::GetMergedMainItem () const
{
	if (m_pMerged != NULL)
	{
		ASSERT_VALID (m_pMerged);

		CBCGPGridItemID id = m_pMerged->GetMainItemID ();
		CBCGPGridCtrl* pGrid = GetOwnerList ();
		if (pGrid != NULL)
		{
			ASSERT_VALID (pGrid);

			CBCGPGridRow* pRow = pGrid->GetRow (id.m_nRow);
			if (pRow != NULL)
			{
				ASSERT_VALID (pRow);

				CBCGPGridItem* pItem = pRow->GetItem (id.m_nColumn);
				if (pItem != NULL)
				{
					ASSERT_VALID (pItem);

					return pItem;
				}
			}
		}
	}

	return NULL;
}
//*******************************************************************************************
void CBCGPGridItem::ReposMergedItem ()
{
	CBCGPGridMergedCells* pMerged = GetMergedCells ();
	if (pMerged != NULL)
	{
		CRect rectCurr = CBCGPGridItem::GetRect ();
		if (!rectCurr.IsRectEmpty ())
		{
			// Calculate merged rectangle by the first visible item of the merged range
			CBCGPGridCtrl* pWndList = GetOwnerList ();
			ASSERT_VALID (pWndList);

			// save id of the first visible item in range
			pWndList->MarkMergedItemChanged (rectCurr, this);
		}

		// If the first visible item is not set yet - clear merged rectangle
		if (pMerged->GetVisibleItemID () == GetGridItemID () && !pMerged->IsChanged ())
		{
			pMerged->SetRectEmpty ();
		}
	}
}
//*******************************************************************************************
void CBCGPGridItem::OnGetBorders (GRID_BORDERS& borders)
{
	borders.top = GRID_BORDERSTYLE_EMPTY;
	borders.left = GRID_BORDERSTYLE_EMPTY;
	borders.bottom = GRID_BORDERSTYLE_DEFAULT;
	borders.right = GRID_BORDERSTYLE_DEFAULT;
}
//*******************************************************************************************
void CBCGPGridItem::OnGetBorders (CRect& rect)
{
	rect.SetRectEmpty ();

	GRID_BORDERS borders;
	OnGetBorders (borders);

	if (borders.left != GRID_BORDERSTYLE_EMPTY)
	{
		rect.left = 1;
	}
	if (borders.top != GRID_BORDERSTYLE_EMPTY)
	{
		rect.top = 1;
	}
	if (borders.right != GRID_BORDERSTYLE_EMPTY)
	{
		rect.right = 1;
	}
	if (borders.bottom != GRID_BORDERSTYLE_EMPTY)
	{
		rect.bottom = 1;
	}
}
//*******************************************************************************************
HBRUSH CBCGPGridItem::OnCtlColor(CDC* pDC, UINT /*nCtlColor*/)
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pGridRow);

	CBCGPGridCtrl* pWndList = m_pGridRow->m_pWndList;
	ASSERT_VALID (pWndList);

	if (pWndList->m_ColorData.m_clrText != (COLORREF)-1)
	{
		pDC->SetTextColor (pWndList->m_ColorData.m_clrText);
	}

	if (pWndList->m_brBackground.GetSafeHandle () != NULL)
	{
		if (pWndList->m_ColorData.m_clrBackground != -1)
		{
			pDC->SetBkColor (pWndList->m_ColorData.m_clrBackground);
		}

		return (HBRUSH) pWndList->m_brBackground.GetSafeHandle ();
	}

	switch (m_varValue.vt)
	{
	case VT_BSTR:
    case VT_R4:
    case VT_R8:
    case VT_UI1:
    case VT_I2:
	case VT_I4:
	case VT_INT:
	case VT_UINT:
    case VT_UI2:
	case VT_UI4:
	case VT_I8:
	case VT_UI8:
	case VT_BOOL:
		if (!m_bEnabled || !m_bAllowEdit)
		{
			pDC->SetBkColor (globalData.clrWindow);
			return (HBRUSH) globalData.brWindow.GetSafeHandle ();
		}
	}

	return NULL;
}
//*******************************************************************************************
void CBCGPGridItem::SetBackgroundColor (COLORREF color, BOOL bRedraw)
{
	m_clrBackground = color;
	if (bRedraw)
	{
		Redraw ();
	}
}
//*******************************************************************************************
void CBCGPGridItem::SetTextColor (COLORREF color, BOOL bRedraw)
{
	m_clrText = color;
	if (bRedraw)
	{
		Redraw ();
	}
}
//*******************************************************************************************
BOOL CBCGPGridItem::IsWordWrap () const
{
	BOOL bWordWrap = IsMultiline () && ((m_dwFlags & BCGP_GRID_ITEM_WORDWRAP) != 0);

	// If merged then enable word-wrapping
	if (m_pMerged != NULL)
	{
		bWordWrap = TRUE;
	}

	return bWordWrap;
}
//*******************************************************************************************
int CBCGPGridItem::GetAlign () const
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pGridRow);
	
	CBCGPGridCtrl* pWndList = m_pGridRow->m_pWndList;
	ASSERT_VALID (pWndList);

	if (m_nIdColumn < 0 || m_nIdColumn >= pWndList->GetColumnCount())
	{
		return HDF_LEFT;
	}

	return pWndList->GetColumnAlign (m_nIdColumn);
}
//*******************************************************************************************
BOOL CBCGPGridItem::SetACCData (CWnd* pParent, CBCGPAccessibilityData& data)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pParent);

	data.Clear ();

	data.m_strAccName = FormatItem ();
	data.m_strDescription = FormatItem ();
	data.m_strAccValue = FormatItem();

	
	data.m_nAccHit = 1;
	data.m_nAccRole = ROLE_SYSTEM_CELL;

	data.m_bAccState = STATE_SYSTEM_FOCUSABLE|STATE_SYSTEM_SELECTABLE;

	if (IsSelected ())
	{
		data.m_bAccState |= STATE_SYSTEM_FOCUSED;
		data.m_bAccState |= STATE_SYSTEM_SELECTED;	
	}

	if (!IsEnabled () || IsReadOnly ())
	{
		data.m_bAccState |= STATE_SYSTEM_READONLY;
	}

	data.m_rectAccLocation = m_Rect;
	pParent->ClientToScreen (&data.m_rectAccLocation);

	return TRUE;
}
//*******************************************************************************************
void CBCGPGridItem::SetDataBar (int nPercentage)
{
	m_nDataBarPerc = nPercentage;
}
//*******************************************************************************************
void CBCGPGridItem::SetDataColorScale (int nPercentage)
{
	m_nDataColorScalePerc = nPercentage;
}
//*******************************************************************************************
void CBCGPGridItem::SetDataIcon(int nPercentage)
{
	m_nDataIconPerc = nPercentage;
}
//*******************************************************************************************
void CBCGPGridItem::OnDrawDataColorScale (CDC* pDC, CRect rect)
{
	ASSERT_VALID (this);

	if (m_nDataColorScalePerc < 0)
	{
		return;
	}

	CBCGPGridCtrl* pWndList = GetOwnerList ();
	ASSERT_VALID (pWndList);

	int nDataColorScalePerc = bcg_clamp(m_nDataColorScalePerc, 0, 100);

	COLORREF clrScaleLow = pWndList->GetColorTheme ().m_clrScaleLow;
	COLORREF clrScaleMid = pWndList->GetColorTheme ().m_clrScaleMid;
	COLORREF clrScaleHigh = pWndList->GetColorTheme ().m_clrScaleHigh;

	if (clrScaleLow == (COLORREF)-1 || clrScaleHigh == (COLORREF)-1)
	{
		if (pWndList->IsVisualManagerStyle())
		{
			CBCGPVisualManager::GetInstance()->GetGridColorScaleBaseColors(pWndList, clrScaleLow, clrScaleMid, clrScaleHigh);
		}
		else
		{
			clrScaleLow = RGB(250, 149, 150);
			clrScaleMid = RGB(255, 235, 132);
			clrScaleHigh = RGB(99, 190, 123);
		}
	}

	if (clrScaleMid == (COLORREF)-1)
	{
		clrScaleMid = CBCGPDrawManager::MixColors(clrScaleLow, clrScaleHigh, .5);
	}

	COLORREF clr = (COLORREF)-1;
	double dblLumRatio = 1.0;
	
	if (nDataColorScalePerc < 50)
	{
		clr = CBCGPDrawManager::SmartMixColors(clrScaleLow, clrScaleMid, dblLumRatio, 50 - nDataColorScalePerc, nDataColorScalePerc);
	}
	else
	{
		clr = CBCGPDrawManager::SmartMixColors(clrScaleHigh, clrScaleMid, dblLumRatio, nDataColorScalePerc - 50, 100 - nDataColorScalePerc);
	}

	if (globalData.m_bIsBlackHighContrast && !pWndList->IsVisualManagerStyle())
	{
		clr = CBCGPDrawManager::ColorMakeDarker(clr, .5);
	}

	if (IsSelected())
	{
		clr = CBCGPDrawManager::ColorMakeDarker(clr);
	}

	CBrush br (clr);
	pDC->FillRect(rect, &br);
}
//*******************************************************************************************
void CBCGPGridItem::OnDrawDataBar (CDC* pDC, CRect rect)
{
	ASSERT_VALID (this);

	if (m_nDataBarPerc < 0 || m_nDataBarPerc > 100)
	{
		return;
	}

	CRect rectBar = rect;
	rectBar.top ++;
	rectBar.DeflateRect (1, 1);
	rectBar.right = rectBar.left + max (1, rectBar.Width() * m_nDataBarPerc / 100);

	CBCGPGridCtrl* pWndList = GetOwnerList ();
	ASSERT_VALID (pWndList);

	//-------------------
	// Use custom colors:
	//-------------------
	BCGP_GRID_COLOR_DATA::ColorData* pDataBarColors = pWndList->GetCustomDataBarColors (m_nIdColumn);

	if (pDataBarColors != NULL && pDataBarColors->Draw (pDC, rectBar))
	{
		return;
	}

	//------------------------
	// Use grid's color theme:
	//------------------------
	if (((BCGP_GRID_COLOR_DATA::ColorData&) pWndList->GetColorTheme ().m_DataBarColors).Draw (pDC, rectBar))
	{
		return;
	}

	//--------------------
	// Use default colors:
	//--------------------
	if (pWndList->IsVisualManagerStyle ())
	{
		visualManager->OnDrawGridDataBar (pWndList, pDC, rectBar);
	}
	else
	{
		CBrush br (RGB (205, 221, 249));
		pDC->FillRect (rectBar, &br);
		
		COLORREF clrBorder = RGB (156, 187, 243);
		pDC->Draw3dRect (rectBar, clrBorder, clrBorder);
	}
}

#ifndef _BCGPGRID_STANDALONE
#ifndef _BCGSUITE_

/////////////////////////////////////////////////////////////////////////////
// CBCGPGridColorItem

IMPLEMENT_SERIAL(CBCGPGridColorItem, CBCGPGridItem, VERSIONABLE_SCHEMA | 1)

CBCGPGridColorItem::CBCGPGridColorItem () :
	CBCGPGridItem (_variant_t(), 0),
	m_Color ((COLORREF) -1)
{
	m_varValue = (LONG) (COLORREF) -1;
	m_dwFlags = BCGP_GRID_ITEM_VCENTER | BCGP_GRID_ITEM_HAS_LIST;

	m_pPopup = NULL;
	m_bStdColorDlg = FALSE;
	m_ColorAutomatic = RGB (0, 0, 0);
	m_nColumnsNumber = 5;
	m_bIsPainting = FALSE;
}
//*****************************************************************************************
CBCGPGridColorItem::CBCGPGridColorItem(const COLORREF& color, 
							   CPalette* /*pPalette*/, DWORD_PTR dwData) :
	CBCGPGridItem (_variant_t(), dwData),
	m_Color (color)
{
	m_varValue = (LONG) color ;
	m_dwFlags = BCGP_GRID_ITEM_VCENTER | BCGP_GRID_ITEM_HAS_LIST;

	m_pPopup = NULL;
	m_bStdColorDlg = FALSE;
	m_ColorAutomatic = RGB (0, 0, 0);
	m_nColumnsNumber = 5;
	m_bIsPainting = FALSE;
}
//*****************************************************************************************
CBCGPGridColorItem::~CBCGPGridColorItem()
{
}
//*******************************************************************************************
void CBCGPGridColorItem::OnDrawValue (CDC* pDC, CRect rect)
{
	CRect rectColor = rect;

	if (m_Color == (COLORREF)-1)	// draw the prompt for Automatic color
	{
		m_bIsPainting = TRUE;
		m_bIsChanged = TRUE;
	}

	rect.left += rect.Height ();
	CBCGPGridItem::OnDrawValue (pDC, rect);

	rectColor.right = rectColor.left + rectColor.Height ();
	rectColor.DeflateRect (1, 1);
	rectColor.top++;
	rectColor.left++;

	CBrush br (m_Color == (COLORREF)-1 ? m_ColorAutomatic : m_Color);
	pDC->FillRect (rectColor, &br);
	pDC->Draw3dRect (rectColor, 0, 0);

	if (m_Color == (COLORREF)-1)
	{
		m_bIsPainting = FALSE;
		m_bIsChanged = TRUE;
	}
}
//****************************************************************************************
void CBCGPGridColorItem::OnPrintValue (CDC* pDC, CRect rect)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);
	ASSERT_VALID (m_pGridRow);

	CBCGPGridCtrl* pWndList = GetOwnerList ();
	ASSERT_VALID (pWndList);
	ASSERT (pWndList->m_bIsPrinting);

	CRect rectColor = rect;

	if (m_Color == (COLORREF)-1)	// draw the prompt for Automatic color
	{
		m_bIsPainting = TRUE;
		m_bIsChanged = TRUE;
	}

	rect.left += rect.Height ();
	CBCGPGridItem::OnPrintValue (pDC, rect);

	const CRect& rectClip = pWndList->m_PrintParams.m_pageInfo.m_rectPageItems;
	const CSize& szOne = pWndList->m_PrintParams.m_pageInfo.m_szOne;

	rectColor.right = rectColor.left + rectColor.Height ();
	rectColor.DeflateRect (szOne.cx, szOne.cy);
	rectColor.top += szOne.cy;
	rectColor.left += szOne.cx;

	rectColor.NormalizeRect ();
	if (rectColor.IntersectRect (&rectColor, &rectClip))
	{

		CBrush br (m_Color == (COLORREF)-1 ? m_ColorAutomatic : m_Color);
		pDC->FillRect (rectColor, &br);

		CPen* pOldPen = pDC->SelectObject (&pWndList->m_penVLine);

		pDC->MoveTo (rectColor.TopLeft ());
		pDC->LineTo (rectColor.right, rectColor.top);
		pDC->LineTo (rectColor.BottomRight ());
		pDC->LineTo (rectColor.left, rectColor.bottom);
		pDC->LineTo (rectColor.TopLeft ());

		pDC->SelectObject (pOldPen);
	}

	if (m_Color == (COLORREF)-1)
	{
		m_bIsPainting = FALSE;
		m_bIsChanged = TRUE;
	}
}
//****************************************************************************************
void CBCGPGridColorItem::OnClickButton (CPoint /*point*/)
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pGridRow);

	CBCGPGridCtrl* pWndList = GetOwnerList ();
	ASSERT_VALID (pWndList);

	m_bButtonIsDown = TRUE;
	Redraw ();

	CList<COLORREF,COLORREF> lstDocColors;

	m_pPopup = new CColorPopup (NULL, m_Colors, m_Color,
		NULL, NULL, NULL, lstDocColors,
		m_nColumnsNumber, m_ColorAutomatic);

	m_pPopup->SetParentGrid (pWndList);

	if (!m_strOtherColor.IsEmpty ())	// Other color button
	{
		((CBCGPColorBar*)m_pPopup->GetMenuBar ())->EnableOtherButton (m_strOtherColor, !m_bStdColorDlg);
	}

	if (!m_strAutoColor.IsEmpty ())	// Automatic color button
	{
		((CBCGPColorBar*)m_pPopup->GetMenuBar ())->EnableAutomaticButton (m_strAutoColor, m_ColorAutomatic);
	}

	CPoint pt (
		m_Rect.left + 1, 
		m_rectButton.bottom + 1);
	pWndList->ClientToScreen (&pt);

	if (!m_pPopup->Create (pWndList, pt.x, pt.y, NULL, FALSE))
	{
		ASSERT (FALSE);
		m_pPopup = NULL;
	}
	else
	{
		m_pPopup->GetMenuBar()->SetFocus ();
	}
}
//******************************************************************************************
BOOL CBCGPGridColorItem::OnEdit (LPPOINT /*lptClick*/)
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pGridRow);

	CBCGPGridCtrl* pWndList = GetOwnerList ();
	ASSERT_VALID (pWndList);

	m_pWndInPlace = NULL;

	CRect rectEdit;
	CRect rectSpin;

	AdjustInPlaceEditRect (rectEdit, rectSpin);

	CBCGPMaskEdit* pWndEdit = new CBCGPMaskEdit;
	DWORD dwStyle = WS_VISIBLE | WS_CHILD;

	if (!m_bEnabled)
	{
		dwStyle |= ES_READONLY;
	}

	pWndEdit->EnableMask(
		_T("AAAAAA"), 
		_T("______"), 
		_T(' ')); 
	pWndEdit->SetValidChars(_T("01234567890ABCDEFabcdef"));
	pWndEdit->SetPrompt (m_strAutoColor);

	pWndEdit->Create (dwStyle, rectEdit, pWndList, BCGPGRIDCTRL_ID_INPLACE);
	m_pWndInPlace = pWndEdit;

	m_pWndInPlace->SetWindowText (FormatItem ());

	m_pWndInPlace->SetFont (pWndList->GetFont ());
	m_pWndInPlace->SetFocus ();

	m_bInPlaceEdit = TRUE;
	return TRUE;
}
//****************************************************************************************
void CBCGPGridColorItem::AdjustInPlaceEditRect (CRect& rectEdit, CRect& rectSpin)
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pGridRow);
	CBCGPGridCtrl* pWndList = GetOwnerList ();
	ASSERT_VALID (pWndList);

	rectSpin.SetRectEmpty ();

	rectEdit = m_Rect;

	if (rectEdit.right > pWndList->GetListRect ().right)
	{
		rectEdit.right = max (rectEdit.left, pWndList->GetListRect ().right);
	}

	rectEdit.DeflateRect (0, TEXT_VMARGIN);

	int nMargin = pWndList->m_nEditLeftMargin;

	rectEdit.left = rectEdit.left + m_Rect.Height () + TEXT_MARGIN - nMargin + 1;

	AdjustButtonRect ();
	rectEdit.right = m_rectButton.left - 1;
}
//****************************************************************************************
CString CBCGPGridColorItem::FormatItem ()
{
	ASSERT_VALID (this);

	CString str;
	if (m_Color != (COLORREF)-1) // Not automatic (default) color value
	{
		str.Format (_T("%02x%02x%02x"),
			GetRValue (m_Color), GetGValue (m_Color), GetBValue (m_Color));
	}
	else if (m_bIsPainting)
	{
		return m_strAutoColor;
	}

	return str;
}
//******************************************************************************************
CRect CBCGPGridColorItem::GetTooltipRect () const
{
	CRect rect = GetRect ();
	rect.left = rect.left + rect.Height ();
	return rect;
}
//******************************************************************************************
void CBCGPGridColorItem::SetColor (COLORREF color)
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pGridRow);

	CBCGPGridCtrl* pWndList = GetOwnerList ();
	ASSERT_VALID (pWndList);

	m_Color = color;
	m_varValue = (LONG) color;

	if (::IsWindow (pWndList->GetSafeHwnd())) 
	{
		CRect rect = m_Rect;
		rect.DeflateRect (0, 1);

		pWndList->InvalidateRect (rect);
		pWndList->UpdateWindow ();
	}

	m_bIsChanged = TRUE;

	if (m_pWndInPlace != NULL)
	{
		ASSERT_VALID (m_pWndInPlace);
		m_pWndInPlace->SetWindowText (GetLabel ());
	}
}
//********************************************************************************
void CBCGPGridColorItem::SetColumnsNumber (int nColumnsNumber)
{
	ASSERT_VALID (this);
	ASSERT (nColumnsNumber > 0);

	m_nColumnsNumber = nColumnsNumber;
}
//*************************************************************************************
void CBCGPGridColorItem::EnableAutomaticButton (LPCTSTR lpszLabel, COLORREF colorAutomatic, BOOL bEnable)
{
	ASSERT_VALID (this);

	m_ColorAutomatic = colorAutomatic;
	m_strAutoColor = (!bEnable || lpszLabel == NULL) ? _T("") : lpszLabel;
}
//*************************************************************************************
void CBCGPGridColorItem::EnableOtherButton (LPCTSTR lpszLabel, BOOL bAltColorDlg, BOOL bEnable)
{
	ASSERT_VALID (this);

	m_bStdColorDlg = !bAltColorDlg;
	m_strOtherColor = (!bEnable || lpszLabel == NULL) ? _T("") : lpszLabel;
}
//*****************************************************************************************
BOOL CBCGPGridColorItem::OnUpdateValue ()
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pWndInPlace);
	ASSERT (::IsWindow (m_pWndInPlace->GetSafeHwnd ()));
	ASSERT_VALID (m_pGridRow);

	CString strText;
	m_pWndInPlace->GetWindowText (strText);

	COLORREF colorCurr = m_Color;
	UINT nR = 0, nG = 0, nB = 0;
#if _MSC_VER < 1400
	int nRes = _stscanf (strText, _T("%2x%2x%2x"), &nR, &nG, &nB);
#else
	int nRes = _stscanf_s (strText, _T("%2x%2x%2x"), &nR, &nG, &nB);
#endif

	COLORREF color = (nRes == EOF || nRes < 3) ? (COLORREF)-1: RGB (nR, nG, nB);

	if (CanUpdateData ())
	{
		SetColor (color);
	}

	if (colorCurr != m_Color)
	{
		SetItemChanged ();
	}

	return TRUE;
}
//*****************************************************************************************
BOOL CBCGPGridColorItem::ReadFromArchive(CArchive& ar, BOOL bTestMode)
{
	BOOL bReadResult = CBCGPGridItem::ReadFromArchive(ar, bTestMode);

	COLORREF	color;			// Color value
	COLORREF	colorAutomatic;	// Automatic (default) color value
	ar >> color;
	ar >> colorAutomatic;

	CString		strAutoColor;	// Atomatic color label
	BOOL		bStdColorDlg;	// Use standard Windows color dialog
	CString		strOtherColor;	// Alternative color label

	ar >> strAutoColor;
	ar >> bStdColorDlg;
	ar >> strOtherColor;

	int nColumnsNumber;	// Number of columns in dropped-down colors list
	ar >> nColumnsNumber;

 	if (!bTestMode)
 	{
		m_Color = color;
		m_ColorAutomatic = colorAutomatic;

		m_strAutoColor = strAutoColor;
		m_bStdColorDlg = bStdColorDlg;
		m_strOtherColor = strOtherColor;

		m_nColumnsNumber = nColumnsNumber;
	}

	return bReadResult;
}
//*****************************************************************************************
void CBCGPGridColorItem::WriteToArchive(CArchive& ar)
{
	CBCGPGridItem::WriteToArchive(ar);

	ar << m_Color;
	ar << m_ColorAutomatic;

	ar << m_strAutoColor;
	ar << m_bStdColorDlg;
	ar << m_strOtherColor;

	ar << m_nColumnsNumber;
}

#endif // _BCGSUITE_

/////////////////////////////////////////////////////////////////////////////
// CBCGPGridDateTimeItem

IMPLEMENT_SERIAL(CBCGPGridDateTimeItem, CBCGPGridItem, VERSIONABLE_SCHEMA | 1)

CBCGPGridDateTimeItem::CBCGPGridDateTimeItem() :
	CBCGPGridItem (_variant_t(COleDateTime (), VT_DATE), 0)
{
	m_nFlags = 0;
	m_dwFlags = BCGP_GRID_ITEM_VCENTER;
}
//*****************************************************************************************
CBCGPGridDateTimeItem::CBCGPGridDateTimeItem(const COleDateTime& date, DWORD_PTR dwData/* = 0*/,
		UINT nFlags/* = CBCGPDateTimeCtrl::DTM_DATE | CBCGPDateTimeCtrl::DTM_TIME*/) :
	CBCGPGridItem (_variant_t(date, VT_DATE), dwData)
{
	m_nFlags = nFlags;
	m_dwFlags = BCGP_GRID_ITEM_VCENTER;
}
//*****************************************************************************************
CBCGPGridDateTimeItem::~CBCGPGridDateTimeItem()
{
}
//*****************************************************************************************
void CBCGPGridDateTimeItem::SetDateTime (const COleDateTime& date, DWORD_PTR dwData,
	UINT nFlags, BOOL bRedraw)
{
	m_nFlags = nFlags;
	SetData (dwData);
	SetValue (_variant_t(date, VT_DATE), bRedraw);
}
//*****************************************************************************************
void CBCGPGridDateTimeItem::OnDrawValue (CDC* pDC, CRect rect)
{
	CBCGPGridItem::OnDrawValue (pDC, rect);
}
//*****************************************************************************************
CWnd* CBCGPGridDateTimeItem::CreateInPlaceEdit (CRect rectEdit, BOOL& bDefaultFormat)
{
	ASSERT_VALID (m_pGridRow);
	CBCGPGridCtrl* pWndList = GetOwnerList ();
	ASSERT_VALID (pWndList);

	CBCGPDateTimeCtrl* pDateTime = new CBCGPDateTimeCtrl;
	ASSERT_VALID (pDateTime);

	pDateTime->SetAutoResize (FALSE);

	CRect rectSpin;
	AdjustInPlaceEditRect (rectEdit, rectSpin);

	pDateTime->Create (_T(""), WS_CHILD | WS_VISIBLE, rectEdit, 
		pWndList, BCGPGRIDCTRL_ID_INPLACE);
	pDateTime->SetFont (pWndList->GetFont ());

	CString strFormat;
	::GetLocaleInfo (LOCALE_USER_DEFAULT, LOCALE_SSHORTDATE, strFormat.GetBuffer (_MAX_PATH), _MAX_PATH);
	strFormat.ReleaseBuffer ();

	if (strFormat.Find (_T("MMMM")) != -1)
	{
		pDateTime->m_monthFormat = 1;
	}
	else if (strFormat.Find (_T("MMM")) != -1)
	{
		pDateTime->m_monthFormat = 0;
	}
	else
	{
		pDateTime->m_monthFormat = 2;
	}

	SetState (*pDateTime);
	pDateTime->SetDate (GetDate ());

	if (pWndList->m_bVisualManagerStyle)
	{
		pDateTime->SendMessage(BCGM_ONSETCONTROLVMMODE, TRUE);
	}

	pDateTime->SetTextColor (pWndList->GetTextColor (), FALSE);
	pDateTime->SetBackgroundColor (pWndList->GetBkColor (), FALSE);

	bDefaultFormat = FALSE;

	return pDateTime;
}
//*****************************************************************************************
BOOL CBCGPGridDateTimeItem::OnUpdateValue ()
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pWndInPlace);
	ASSERT (::IsWindow (m_pWndInPlace->GetSafeHwnd ()));
	ASSERT_VALID (m_pGridRow);

	CBCGPDateTimeCtrl* pDateTime = DYNAMIC_DOWNCAST (CBCGPDateTimeCtrl, m_pWndInPlace);
	ASSERT_VALID (pDateTime);

	COleDateTime dateOld = GetDate ();
	COleDateTime dateNew = pDateTime->GetDate ();
	
	if (CanUpdateData ())
	{
		m_varValue = _variant_t (dateNew, VT_DATE);
	}

	if (dateOld != dateNew)
	{
		SetItemChanged ();
	}

	return TRUE;
}
//****************************************************************************************
CString CBCGPGridDateTimeItem::FormatItem ()
{
	ASSERT_VALID (this);

	COleDateTime date = (DATE) m_varValue;

	SYSTEMTIME st;
	date.GetAsSystemTime (st);

	CString str;

	CString strDate;
	::GetDateFormat (LOCALE_USER_DEFAULT, DATE_SHORTDATE, &st, NULL, strDate.GetBuffer (_MAX_PATH), _MAX_PATH);
	strDate.ReleaseBuffer ();

	CString strTime;
	::GetTimeFormat (LOCALE_USER_DEFAULT, TIME_NOSECONDS, &st, NULL, strTime.GetBuffer (_MAX_PATH), _MAX_PATH);
	strTime.ReleaseBuffer ();

	if (m_nFlags == CBCGPDateTimeCtrl::DTM_DATE)
	{
		str = strDate;
	}
	else if (m_nFlags == CBCGPDateTimeCtrl::DTM_TIME)
	{
		str = strTime;
	}
	else
	{
		if (!strDate.IsEmpty ())
		{
			str = strDate;
		}

		if (!strTime.IsEmpty ())
		{
			if (!str.IsEmpty ())
			{
				str += _T(" ");
			}

			str += strTime;
		}
	}

	return str;
}
//*****************************************************************************************
void CBCGPGridDateTimeItem::OnSetSelection (CBCGPGridItem*)
{
	Redraw ();
}
//*****************************************************************************************
void CBCGPGridDateTimeItem::OnKillSelection (CBCGPGridItem*)
{
	Redraw ();
}
//*****************************************************************************************
BOOL CBCGPGridDateTimeItem::PushChar (UINT nChar)
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pGridRow);
	ASSERT (m_pGridRow->IsSelected ());
	ASSERT_VALID (m_pWndInPlace);

	if (m_bEnabled && m_bAllowEdit)
	{
		CString str;
		str += (TCHAR) nChar;
		str.MakeUpper ();

		m_pWndInPlace->SendMessage (WM_KEYDOWN, (WPARAM) str [0]);
		return TRUE;
	}

	OnEndEdit ();

	CBCGPGridCtrl* pWndList = GetOwnerList ();
	ASSERT_VALID (pWndList);

	if (::GetCapture () == pWndList->GetSafeHwnd ())
	{
		ReleaseCapture ();
	}

	return FALSE;
}
//*****************************************************************************************
void CBCGPGridDateTimeItem::SetDate (COleDateTime date)
{
	ASSERT_VALID (this);
	SetValue (_variant_t (date, VT_DATE));
}
//*****************************************************************************************
void CBCGPGridDateTimeItem::SetState (CBCGPDateTimeCtrl& wnd)
{
	ASSERT (wnd.GetSafeHwnd () != NULL);

	UINT nFlags = m_nFlags;
	if(!(nFlags & CBCGPDateTimeCtrl::DTM_DATE) && !(nFlags & CBCGPDateTimeCtrl::DTM_TIME))
	{
		nFlags |= (CBCGPDateTimeCtrl::DTM_DATE | CBCGPDateTimeCtrl::DTM_TIME);
	}

	UINT stateFlags = 0;

	if (nFlags & (CBCGPDateTimeCtrl::DTM_DATE))
	{
		stateFlags |= (CBCGPDateTimeCtrl::DTM_DATE | CBCGPDateTimeCtrl::DTM_DROPCALENDAR);
	}
	
	if (nFlags & (CBCGPDateTimeCtrl::DTM_TIME))
	{
		stateFlags |= (CBCGPDateTimeCtrl::DTM_TIME | CBCGPDateTimeCtrl::DTM_TIME24HBYLOCALE);
	}

	const UINT stateMask = 
		CBCGPDateTimeCtrl::DTM_SPIN |
		CBCGPDateTimeCtrl::DTM_DROPCALENDAR | 
		CBCGPDateTimeCtrl::DTM_DATE |
		CBCGPDateTimeCtrl::DTM_TIME24H |
		CBCGPDateTimeCtrl::DTM_CHECKBOX |
		CBCGPDateTimeCtrl::DTM_TIME | 
		CBCGPDateTimeCtrl::DTM_TIME24HBYLOCALE;

	wnd.SetState (stateFlags, stateMask);
}
//*****************************************************************************************
void CBCGPGridDateTimeItem::AdjustInPlaceEditRect (CRect& rectEdit, CRect& rectSpin)
{
	CBCGPGridItem::AdjustInPlaceEditRect (rectEdit, rectSpin);

	rectEdit.bottom++;
}
//*****************************************************************************************
BOOL CBCGPGridDateTimeItem::ReadFromArchive(CArchive& ar, BOOL bTestMode)
{
	BOOL bReadResult = CBCGPGridItem::ReadFromArchive(ar, bTestMode);

	UINT nFlags;	// DateTime flags
	ar >> nFlags;

	if (!bTestMode)
	{
		m_nFlags = nFlags;
	}

	return bReadResult;
}
//*****************************************************************************************
void CBCGPGridDateTimeItem::WriteToArchive(CArchive& ar)
{
	CBCGPGridItem::WriteToArchive(ar);

	ar << m_nFlags;	// DateTime flags
}

#endif // _BCGPGRID_STANDALONE

/////////////////////////////////////////////////////////////////////////////
// CBCGPGridCheckItem

IMPLEMENT_SERIAL (CBCGPGridCheckItem, CBCGPGridItem, VERSIONABLE_SCHEMA | 1)

CBCGPGridCheckItem::CBCGPGridCheckItem () :
	CBCGPGridItem (_variant_t (false), 0)
{
	m_bAllowEdit = FALSE;
	m_dwFlags = m_dwFlags & ~BCGP_GRID_ITEM_HAS_LIST;
}
//*****************************************************************************************
CBCGPGridCheckItem::CBCGPGridCheckItem (bool bVal, DWORD_PTR dwData/* = 0*/) :
	CBCGPGridItem (_variant_t (bVal), dwData)
{
	m_bAllowEdit = FALSE;
	m_dwFlags = m_dwFlags & ~BCGP_GRID_ITEM_HAS_LIST;
}
//*****************************************************************************************
CBCGPGridCheckItem::~CBCGPGridCheckItem()
{
}
//*****************************************************************************************
void CBCGPGridCheckItem::SetLabel(const CString& strLabel)
{
	m_strCheckLabel = strLabel;
}
//*****************************************************************************************
void CBCGPGridCheckItem::OnDrawValue (CDC* pDC, CRect rect)
{
	ASSERT_VALID (this);
	
	CBCGPGridCtrl* pWndList = GetOwnerList ();
	ASSERT_VALID (pWndList);

	COLORREF clrTextOld = (COLORREF)-1;
	COLORREF clrText = OnFillBackground (pDC, rect);

	CRect rectCheck = rect;
	
	rectCheck.DeflateRect (0, TEXT_VMARGIN);

	int nWidth = pWndList->GetButtonWidth ();

	if (m_strCheckLabel.IsEmpty())
	{
		rectCheck.left = rectCheck.CenterPoint ().x - nWidth / 2;
	}

	rectCheck.right = rectCheck.left + nWidth;

	if (rectCheck.Width() != rectCheck.Height())
	{
		if (rectCheck.Width() > rectCheck.Height())
		{
			rectCheck.left = rectCheck.CenterPoint().x - (int)(0.5 * rectCheck.Height());
			rectCheck.right = rectCheck.left + rectCheck.Height();
		}
		else
		{
			rectCheck.top = rectCheck.CenterPoint().y - (int)(0.5 * rectCheck.Width());
			rectCheck.bottom = rectCheck.top + rectCheck.Width();
		}
	}

	visualManagerMFC->OnDrawCheckBox (pDC, rectCheck, FALSE, (bool) m_varValue, m_bEnabled);

	if (clrText != (COLORREF)-1)
	{
		clrTextOld = pDC->SetTextColor (clrText);
	}

	if (!m_strCheckLabel.IsEmpty())
	{
		int nTextMargin = 4;
		if (globalData.GetRibbonImageScale () != 1.)
		{
			nTextMargin = (int)(.5 + nTextMargin * globalData.GetRibbonImageScale ());
		}

		CRect rectLabel = rect;
		rectLabel.left = rectCheck.right + nTextMargin;

		m_bValueIsTrancated = pWndList->DoDrawText (pDC, m_strCheckLabel, rectLabel, DT_NOPREFIX | DT_END_ELLIPSIS | DT_SINGLELINE | DT_VCENTER);
	}

	if (clrTextOld != (COLORREF)-1)
	{
		pDC->SetTextColor (clrTextOld);
	}
}
//*****************************************************************************************
void CBCGPGridCheckItem::OnPrintValue (CDC* pDC, CRect rect)
{
	const bool bVal = (bool) m_varValue;
	if (!bVal)
	{
		return;
	}

	CBCGPGridCtrl* pWndList = GetOwnerList ();
	ASSERT_VALID (pWndList);

	const CRect& rectClip = pWndList->m_PrintParams.m_pageInfo.m_rectPageItems;
	const CSize& szOne = pWndList->m_PrintParams.m_pageInfo.m_szOne;

	CRect rectCheck = rect;

	rectCheck.right = rectCheck.left + pWndList->m_PrintParams.m_nButtonWidth;
	rectCheck.DeflateRect (szOne.cx, szOne.cy);
	rectCheck.top += szOne.cy;
	rectCheck.left += szOne.cx;

	rectCheck.NormalizeRect ();
	if (rectCheck.IntersectRect (&rectCheck, &rectClip))
	{
		CBCGPMenuImages::Draw (pDC, CBCGPMenuImages::IdCheck, rectCheck, CBCGPMenuImages::ImageBlack, rectCheck.Size());
	}
}
//*****************************************************************************************
BOOL CBCGPGridCheckItem::PushChar (UINT nChar)
{
	ASSERT_VALID (m_pGridRow);

	if (nChar == VK_SPACE && IsEnabled () && !IsReadOnly())
	{
		BOOL bOldValue = (bool)GetValue ();
		SetValue (!bOldValue);

		SetItemChanged ();
		return TRUE;
	}
	
	return FALSE;
}
//*****************************************************************************************
CString CBCGPGridCheckItem::GetValueTooltip ()
{
	return m_bValueIsTrancated ? m_strCheckLabel : _T("");
}
//*****************************************************************************************
CRect CBCGPGridCheckItem::GetTooltipRect () const
{
	if (m_strCheckLabel.IsEmpty())
	{
		return CBCGPGridItem::GetTooltipRect ();
	}

	CBCGPGridCtrl* pWndList = GetOwnerList ();
	ASSERT_VALID (pWndList);
	
	CRect rectLabel = GetRect ();
	rectLabel.left += pWndList->GetButtonWidth ();

	return rectLabel;
}
//*****************************************************************************************
BOOL CBCGPGridCheckItem::OnClickValue (UINT uiMsg, CPoint point)
{
	ASSERT_VALID (m_pGridRow);
	CBCGPGridCtrl* pWndList = GetOwnerList ();
	ASSERT_VALID (pWndList);

	if ((uiMsg == WM_LBUTTONDOWN || uiMsg == WM_RBUTTONDOWN) &&
		m_pWndInPlace == NULL && pWndList->AllowInPlaceEdit () && !IsReadOnly ())
	{
		if (!m_strCheckLabel.IsEmpty())
		{
			CRect rectCheck = m_Rect;
			rectCheck.right = rectCheck.left + pWndList->GetButtonWidth ();

			if (!rectCheck.PtInRect(point))
			{
				return CBCGPGridItem::OnClickValue (uiMsg, point);
			}
		}

		BOOL bOldValue = (bool)GetValue ();
		SetValue (!bOldValue);

		SetItemChanged ();
		return TRUE;
	}

	return CBCGPGridItem::OnClickValue (uiMsg, point);
}
//*****************************************************************************************
BOOL CBCGPGridCheckItem::OnDblClick (CPoint point)
{
	return CBCGPGridItem::OnDblClick (point);
}
//*****************************************************************************************
BOOL CBCGPGridCheckItem::ReadFromArchive(CArchive& ar, BOOL bTestMode)
{
	BOOL bReadResult = CBCGPGridItem::ReadFromArchive(ar, bTestMode);

	CString strCheckLabel;
	ar >> strCheckLabel;

	if (!bTestMode)
	{
		m_strCheckLabel = strCheckLabel;
	}

	return bReadResult;
}
//*****************************************************************************************
void CBCGPGridCheckItem::WriteToArchive(CArchive& ar)
{
	CBCGPGridItem::WriteToArchive(ar);

	ar << m_strCheckLabel;
}
//*****************************************************************************************
BOOL CBCGPGridCheckItem::SetACCData (CWnd* pParent, CBCGPAccessibilityData& data)
{
	CBCGPGridItem::SetACCData (pParent, data);

	if (!GetLabel ().IsEmpty ())
	{
		data.m_strAccName = GetLabel ();
	}

	if ((bool)GetValue ())
	{
		data.m_bAccState |= STATE_SYSTEM_CHECKED;
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPGridURLItem

IMPLEMENT_DYNCREATE (CBCGPGridURLItem, CBCGPGridItem)

CBCGPGridURLItem::CBCGPGridURLItem () :
	CBCGPGridItem (_variant_t ((LPCTSTR) _T("")), 0),
	m_strURL ()
{
}
//*****************************************************************************************
CBCGPGridURLItem::CBCGPGridURLItem (CString str, CString strURL, DWORD_PTR dwData/* = 0*/) :
	CBCGPGridItem (_variant_t ((LPCTSTR) str), dwData),
	m_strURL (strURL)
{
}
//*****************************************************************************************
CBCGPGridURLItem::~CBCGPGridURLItem()
{
}
//*****************************************************************************************
void CBCGPGridURLItem::OnDrawValue (CDC* pDC, CRect rect)
{
	ASSERT_VALID (m_pGridRow);

	COLORREF clrText = OnFillBackground (pDC, rect);

	//-----------
	// Draw icon:
	//-----------
	OnDrawIcon (pDC, rect);

	//-----------
	// Draw link:
	//-----------

#ifndef _BCGSUITE_
	COLORREF clrLink = globalData.clrHotText;
#else
	COLORREF clrLink = globalData.clrHotLinkNormalText;
#endif

	COLORREF clrTextOld = 
		pDC->SetTextColor (clrText == (COLORREF)-1 ? clrLink : clrText);

	CFont* pOldFont = pDC->SelectObject (CBCGPGridCtrl::m_bUseSystemFont ? &globalData.fontDefaultGUIUnderline : &globalData.fontUnderline);
	ASSERT (pOldFont != NULL);

	CString str = (LPCTSTR)(_bstr_t) m_varValue;

	rect.DeflateRect (TEXT_MARGIN, TEXT_VMARGIN);
	DWORD dwFlags = GetFlags ();

	UINT uiTextFlags = DT_LEFT | DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS;

	if (dwFlags & BCGP_GRID_ITEM_VTOP)
	{
		uiTextFlags |= DT_TOP;
	}
	else if (dwFlags & BCGP_GRID_ITEM_VBOTTOM)
	{
		uiTextFlags |= DT_BOTTOM;
	}
	else // dwFlags & BCGP_GRID_ITEM_VCENTER
	{
		uiTextFlags |= DT_VCENTER;
	}

	pDC->DrawText (str, rect, uiTextFlags);

	m_bValueIsTrancated = pDC->GetTextExtent (str).cx > rect.Width ();

	pDC->SelectObject (pOldFont);
	if (clrTextOld != (COLORREF)-1)
	{
		pDC->SetTextColor (clrTextOld);
	}
}
//*****************************************************************************************
BOOL CBCGPGridURLItem::OnSetCursor () const
{
#ifndef _BCGPGRID_STANDALONE
	CBCGPGridCtrl* pWndList = GetOwnerList ();
	ASSERT_VALID (pWndList);

	if (pWndList->AllowInPlaceEdit () && IsAllowEdit () && 
		(::GetAsyncKeyState (VK_CONTROL) & 0x8000) == 0 || IsInPlaceEditing ())
	{
		return CBCGPGridItem::OnSetCursor ();
	}
	
	::SetCursor (globalData.GetHandCursor ());
	return TRUE;
#else
	return FALSE;
#endif
}
//*****************************************************************************************
BOOL CBCGPGridURLItem::OnClickValue (UINT uiMsg, CPoint point)
{
	if (uiMsg == WM_LBUTTONDOWN && m_pWndInPlace == NULL)
	{
		CWaitCursor wait;

		CString strURL = m_strURL;

		if (strURL.IsEmpty ())
		{
			strURL = _T("http:\\\\");
			strURL += (LPCTSTR)(_bstr_t) m_varValue;
		}

		if (::ShellExecute (NULL, NULL, strURL, NULL, NULL, NULL) < (HINSTANCE) 32)
		{
			TRACE(_T("Can't open URL: %s\n"), m_strURL);
			return FALSE;
		}

		return TRUE;
	}

	return CBCGPGridItem::OnClickValue (uiMsg, point);
}
//*****************************************************************************************
BOOL CBCGPGridURLItem::OnEdit (LPPOINT lptClick)
{
	if ((::GetAsyncKeyState (VK_CONTROL) & 0x8000) != 0)
	{
		return FALSE;
	}

	return CBCGPGridItem::OnEdit (lptClick);
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPGridSparklineItem

IMPLEMENT_DYNCREATE (CBCGPGridSparklineItem, CBCGPGridItem)

CBCGPGridSparklineItem::CBCGPGridSparklineItem (SparklineType type, DWORD_PTR dwData) :
	CBCGPGridItem (_variant_t ((LPCTSTR) _T("")), dwData)
{
	m_bAllowEdit = FALSE;
	m_pChart = NULL;
	m_nMaxMarkerSize = 0;
	m_bDefaultSelColor = TRUE;

	if (type != SparklineTypeLine)
	{
		SetType(type);
	}
}
//*****************************************************************************************
CBCGPGridSparklineItem::CBCGPGridSparklineItem (const CBCGPDoubleArray& arData, SparklineType type, DWORD_PTR dwData) :
	CBCGPGridItem (_variant_t ((LPCTSTR) _T("")), dwData)
{
	m_bAllowEdit = FALSE;
	m_pChart = NULL;
	m_nMaxMarkerSize = 0;
	m_bDefaultSelColor = TRUE;

	if (type != SparklineTypeLine)
	{
		SetType(type);
	}

	if (arData.GetSize() > 0)
	{
		AddData(arData);
	}
}
//*****************************************************************************************
CBCGPGridSparklineItem::CBCGPGridSparklineItem(const CBCGPGridSparklineDataArray& arData, SparklineType type, DWORD_PTR dwData) :
	CBCGPGridItem (_variant_t ((LPCTSTR) _T("")), dwData)
{
	m_bAllowEdit = FALSE;
	m_pChart = NULL;
	m_nMaxMarkerSize = 0;
	m_bDefaultSelColor = TRUE;
	
	if (type != SparklineTypeLine)
	{
		SetType(type);
	}
	
	if (arData.GetSize() > 0)
	{
		AddData(arData);
	}
}
//*****************************************************************************************
CBCGPGridSparklineItem::~CBCGPGridSparklineItem()
{
	if (m_pChart != NULL)
	{
		delete m_pChart;
	}
}
//*****************************************************************************
CBCGPChartVisualObject* CBCGPGridSparklineItem::GetChart()
{
	if (m_pChart == NULL)
	{
		m_pChart = new CBCGPChartVisualObject;
		InitChart();
	}

	return m_pChart;
}
//*****************************************************************************
int CBCGPGridSparklineItem::SparklineTypeToChartType (SparklineType type)
{
	switch (type)
	{
	case SparklineTypeLine:
		return (int)BCGPChartLine;
		
	case SparklineTypePie:
		return (int)BCGPChartPie;
		
	case SparklineTypeColumn:
		return (int)BCGPChartColumn;
		
	case SparklineTypeBar:
		return (int)BCGPChartBar;
		
	case SparklineTypeArea:
		return (int)BCGPChartArea;
		
	case SparklineTypeBubble:
		return (int)BCGPChartBubble;
		
	case SparklineTypeDoughnut:
		return (int)BCGPChartDoughnut;
	}

	return (int)BCGPChartDefault;
}
//*****************************************************************************
void CBCGPGridSparklineItem::SetType(SparklineType type)
{
	CBCGPChartVisualObject*	pChart = GetChart();
	ASSERT_VALID(pChart);

	pChart->SetChartType((BCGPChartCategory)SparklineTypeToChartType(type));
}
//*****************************************************************************
CBCGPGridSparklineItem::SparklineType CBCGPGridSparklineItem::GetType()
{
	CBCGPChartVisualObject*	pChart = GetChart();
	ASSERT_VALID(pChart);

	switch (pChart->GetChartCategory())
	{
	case BCGPChartLine:
		return SparklineTypeLine;

	case BCGPChartPie:
		return SparklineTypePie;
		
	case BCGPChartColumn:
		return SparklineTypeColumn;
		
	case BCGPChartBar:
		return SparklineTypeBar;
		
	case BCGPChartArea:
		return SparklineTypeArea;
		
	case BCGPChartBubble:
		return SparklineTypeBubble;
		
	case BCGPChartDoughnut:
		return SparklineTypeDoughnut;
	}

	return (SparklineType)-1;
}
//*****************************************************************************
COLORREF CBCGPGridSparklineItem::GetFillColor()
{
	return (COLORREF)-1;
}
//*****************************************************************************
CBCGPChartSeries* CBCGPGridSparklineItem::GetSeries(int nSeries, SparklineType type)
{
	CBCGPChartVisualObject*	pChart = GetChart();
	ASSERT_VALID(pChart);

	if (type != SparklineTypeDefault && type != GetType())
	{
		return pChart->CreateSeries(_T(""), CBCGPColor(), BCGP_CT_DEFAULT,
			(BCGPChartCategory)SparklineTypeToChartType(type), nSeries);
	}
	else
	{
		return pChart->GetSeries(nSeries, TRUE);
	}
}
//*****************************************************************************
void CBCGPGridSparklineItem::AddData(const CBCGPDoubleArray& arData, int nSeries, SparklineType type)
{
	CBCGPChartSeries* pSeries = GetSeries(nSeries, type);
	if (pSeries == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	ASSERT_VALID(pSeries);

	pSeries->AddDataPoints(arData);
}
//*****************************************************************************
void CBCGPGridSparklineItem::AddData(const CBCGPGridSparklineDataArray& arData, int nSeries, SparklineType type)
{
	if (arData.GetSize() == 0)
	{
		return;
	}

	CBCGPChartVisualObject*	pChart = GetChart();
	ASSERT_VALID(pChart);

	CBCGPChartSeries* pSeries = GetSeries(nSeries, type);
	if (pSeries == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	ASSERT_VALID(pSeries);

	if (type != SparklineTypeDefault && type != GetType())
	{
		pSeries->SetChartType((BCGPChartCategory)SparklineTypeToChartType(type));
	}

	int nMaxMarkerSizePrev = m_nMaxMarkerSize;

	for (int i = 0; i < (int)arData.GetSize(); i++)
	{
		const CBCGPBrush& brDataPointFill = arData[i].m_brDataPointFill;
		const CBCGPBrush& brDataPointBorder = arData[i].m_brDataPointBorder;
		
		BOOL bShowMarker = arData[i].m_MarkerOptions.m_bShowMarker;
		
		int nDPIndex = -1;

		if (!brDataPointFill.IsEmpty() || !brDataPointBorder.IsEmpty() || bShowMarker)
		{
			BCGPChartFormatSeries fs;
			
			if (!brDataPointFill.IsEmpty() || !brDataPointBorder.IsEmpty())
			{
				fs.SetSeriesFill(brDataPointFill);
				fs.SetSeriesLineColor(brDataPointBorder.IsEmpty() ? brDataPointFill : brDataPointBorder);
			}
			
			if (bShowMarker)
			{
				const CBCGPBrush& brMarkerFill = arData[i].m_brMarkerFill;
				const CBCGPBrush& brMarkerBorder = arData[i].m_brMarkerBorder;

				fs.m_markerFormat.m_options = arData[i].m_MarkerOptions;
				
				if (!brMarkerFill.IsEmpty() || !brMarkerBorder.IsEmpty())
				{
					fs.SetMarkerFill(brMarkerFill);
					fs.SetMarkerLineColor(brMarkerBorder.IsEmpty() ? brMarkerFill : brMarkerBorder);
				}

				m_nMaxMarkerSize = max(m_nMaxMarkerSize, fs.m_markerFormat.m_options.GetMarkerSize());
			}
			
			nDPIndex = pSeries->AddDataPoint(arData[i].m_dblValue, &fs);
		}
		else
		{
			nDPIndex = pSeries->AddDataPoint(arData[i].m_dblValue);
		}

		if (arData[i].m_dblValue1 != 0. && nDPIndex >= 0)
		{
			pSeries->SetDataPointValue(nDPIndex, arData[i].m_dblValue1, CBCGPChartData::CI_Y1);
		}
	}

	if (nMaxMarkerSizePrev != m_nMaxMarkerSize)
	{
		const double dblPadding = 0.5 * (double)(1.0 + m_nMaxMarkerSize);

		pChart->GetChartAreaFormat().SetContentPadding(CBCGPSize(dblPadding, dblPadding));
	}
}
//*****************************************************************************
BOOL CBCGPGridSparklineItem::UpdateDataPoint(int nIndex, double dblValue, int nSeries/* = 0*/)
{
	CBCGPChartVisualObject*	pChart = GetChart();
	ASSERT_VALID(pChart);

	CBCGPChartSeries* pSeries = pChart->GetSeries(nSeries);
	if (pSeries == NULL)
	{
		return FALSE;
	}

	if (!pSeries->SetDataPointValue(nIndex, dblValue))
	{
		return FALSE;
	}

	pChart->RecalcMinMaxValues();
	pChart->SetDirty();

	return TRUE;
}
//*****************************************************************************
BOOL CBCGPGridSparklineItem::ShowDataPointMarker(int nIndex, const BCGPChartMarkerOptions& markerOptions, 
						 const CBCGPBrush& brMarkerFill, const CBCGPBrush& brMarkerBorder, int nSeries)
{
	CBCGPChartVisualObject*	pChart = GetChart();
	ASSERT_VALID(pChart);

	CBCGPChartSeries* pSeries = pChart->GetSeries(nSeries);
	if (pSeries == NULL)
	{
		return FALSE;
	}

	if (nIndex < 0 || nIndex >= pSeries->GetDataPointCount())
	{
		return FALSE;
	}

	int nMaxMarkerSizePrev = m_nMaxMarkerSize;

	BCGPChartFormatSeries fs;
	fs.m_markerFormat.m_options = markerOptions;
	
	if (!brMarkerFill.IsEmpty() || !brMarkerBorder.IsEmpty())
	{
		fs.SetMarkerFill(brMarkerFill);
		fs.SetMarkerLineColor(brMarkerBorder.IsEmpty() ? brMarkerFill : brMarkerBorder);
	}

	m_nMaxMarkerSize = max(m_nMaxMarkerSize, fs.m_markerFormat.m_options.GetMarkerSize());

	pSeries->SetMarkerOptions(markerOptions, nIndex);
	pSeries->SetMarkerFill(brMarkerFill, nIndex);
	pSeries->SetMarkerLineColor(brMarkerBorder, nIndex);

	if (nMaxMarkerSizePrev != m_nMaxMarkerSize)
	{
		const double dblPadding = 0.5 * (double)(1.0 + m_nMaxMarkerSize);
		pChart->GetChartAreaFormat().SetContentPadding(CBCGPSize(dblPadding, dblPadding));
	}

	pChart->RecalcMinMaxValues();
	pChart->SetDirty();

	return TRUE;
}
//*****************************************************************************
void CBCGPGridSparklineItem::RemoveData(int nSeries)
{
	CBCGPChartVisualObject*	pChart = GetChart();
	ASSERT_VALID(pChart);

	pChart->CleanUpChartData(nSeries);

	if (nSeries == -1 || pChart->GetSeriesCount() == 1)
	{
		if (m_nMaxMarkerSize != 0)
		{
			m_nMaxMarkerSize = 0;
			pChart->GetChartAreaFormat().SetContentPadding(CBCGPSize());
		}
	}
}
//*****************************************************************************
void CBCGPGridSparklineItem::OnDrawValue (CDC* pDC, CRect rect)
{
	ASSERT_VALID (this);

	OnFillBackground(pDC, rect);

	CBCGPGridCtrl* pGrid = GetOwnerList();

	CBCGPChartVisualObject*	pChart = GetChart();
	ASSERT_VALID(pChart);

	if (pChart->GetRect().IsRectEmpty() || pGrid == NULL)
	{
		return;
	}

	CBCGPGraphicsManager* pGM = pGrid->GetGraphicsManager();
	if (pGM == NULL)
	{
		return;
	}

	CRect rectClient;
	pGrid->GetClientRect(rectClient);

	pGM->BindDC(pDC, rectClient);
	
	if (!pGM->BeginDraw())
	{
		return;
	}

	pChart->OnDraw(pGM, rect);

	pGM->EndDraw();
}
//*****************************************************************************
void CBCGPGridSparklineItem::OnPrintValue (CDC* pDC, CRect rect)
{
	ASSERT_VALID (this);

	CBCGPGridCtrl* pGrid = GetOwnerList();

	CBCGPChartVisualObject*	pChart = GetChart();
	ASSERT_VALID(pChart);

	if (pChart->GetRect().IsRectEmpty() || pGrid == NULL)
	{
		return;
	}

	CBCGPGraphicsManager* pGM = pGrid->GetGraphicsManager();
	if (pGM == NULL)
	{
		return;
	}


	ASSERT_VALID(pGM);

	const CBCGPSize sizeScaleOld = pChart->GetScaleRatio ();

	HDC hDCFrom = ::GetDC(NULL);
	double dXMul = (double) pDC->GetDeviceCaps(LOGPIXELSX);			// pixels in print dc
	double dXDiv = (double) ::GetDeviceCaps(hDCFrom, LOGPIXELSX);	// pixels in screen dc
	double dYMul = (double) pDC->GetDeviceCaps(LOGPIXELSY);			// pixels in print dc
	double dYDiv = (double) ::GetDeviceCaps(hDCFrom, LOGPIXELSY);	// pixels in screen dc
	::ReleaseDC(NULL, hDCFrom);

	pChart->SetScaleRatio (CBCGPSize (dXMul / dXDiv, dYMul / dYDiv));
	
	HBITMAP hmbpDib = pChart->ExportToBitmap(pGM);

	pChart->SetScaleRatio (sizeScaleOld);

	if (hmbpDib == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	const CSize size = rect.Size();
	const CSize sizeImage = size;

	CDC dcMem;
	dcMem.CreateCompatibleDC (pDC);

	dcMem.SelectObject (hmbpDib);

	pDC->StretchBlt (rect.left, rect.top, rect.Width (), rect.Height (), &dcMem, 0, 0, sizeImage.cx, sizeImage.cy, SRCCOPY);
}
//*****************************************************************************
BOOL CBCGPGridSparklineItem::OnEdit (LPPOINT)
{
	return FALSE;
}
//*****************************************************************************
void CBCGPGridSparklineItem::OnPosSizeChanged (CRect rectOld)
{
	ASSERT_VALID (this);

	CBCGPGridItem::OnPosSizeChanged(rectOld);

	CBCGPGridCtrl* pGrid = GetOwnerList ();
	if (pGrid != NULL && !pGrid->IsColumnBeingResized())
	{
		CBCGPChartVisualObject*	pChart = GetChart();
		ASSERT_VALID(pChart);

		CBCGPRect rect = m_Rect;

		if (pChart->GetRect() != m_Rect)
		{
			pChart->SetRect(rect, TRUE);
			pChart->SetDirty (TRUE);
		}
	}
}
//*****************************************************************************
void CBCGPGridSparklineItem::InitChart()
{
	ASSERT_VALID(m_pChart);

	OnAfterChangeGridColors();

	m_pChart->SetThumbnailMode(TRUE, BCGP_CHART_THUMBNAIL_DRAW_MARKERS);
	m_pChart->ShowChartTitle(FALSE);
	m_pChart->GetChartAreaFormat().SetContentPadding(CBCGPSize());
	m_pChart->SetPlotAreaPadding(CBCGPRect());
	m_pChart->SetLegendAreaPadding(CBCGPSize());

	m_pChart->ShowAxisGridLines(BCGP_CHART_X_PRIMARY_AXIS, FALSE, FALSE);
	m_pChart->ShowAxisGridLines(BCGP_CHART_Y_PRIMARY_AXIS, FALSE, FALSE);

	m_pChart->GetChartAxis(BCGP_CHART_X_PRIMARY_AXIS)->m_axisLabelType = CBCGPChartAxis::ALT_NO_LABELS;
	m_pChart->GetChartAxis(BCGP_CHART_Y_PRIMARY_AXIS)->m_axisLabelType = CBCGPChartAxis::ALT_NO_LABELS;

	m_pChart->ShowAxis(BCGP_CHART_X_PRIMARY_AXIS, FALSE);
	m_pChart->ShowAxis(BCGP_CHART_Y_PRIMARY_AXIS, FALSE);
}
//*****************************************************************************
void CBCGPGridSparklineItem::OnAfterChangeGridColors()
{
	CBCGPGridItem::OnAfterChangeGridColors();

	CBCGPGridCtrl* pGrid = GetOwnerList();
	if (pGrid == NULL)
	{
		return;
	}

	CBCGPChartVisualObject*	pChart = GetChart();
	ASSERT_VALID(pChart);

	const CBCGPGridColors& colorsGrid = pGrid->GetColorTheme();

	CBCGPChartTheme colorsChart;
	CBCGPColor clrFill = colorsGrid.m_EvenColors.m_clrBackground == (COLORREF)-1 ? colorsGrid.m_clrBackground : colorsGrid.m_EvenColors.m_clrBackground;

	COLORREF clrGridText = globalData.clrWindowText;

	if (colorsGrid.m_EvenColors.m_clrText != (COLORREF)-1)
	{
		clrGridText = colorsGrid.m_EvenColors.m_clrText;
	}
	else if (colorsGrid.m_clrText != (COLORREF)-1)
	{
		clrGridText = colorsGrid.m_clrText;
	}

	CBCGPColor clrOutline = clrGridText;
	CBCGPColor clrText = clrGridText;

	if (clrOutline.IsDark())
	{
		clrOutline.MakeLighter(.5);
	}
	else
	{
		clrOutline.MakeDarker(.5);
	}

	CBCGPChartTheme::InitChartColors(colorsChart, 
		CBCGPColor(), clrOutline, clrText,
		CBCGPColor(), CBCGPColor(), .04, clrFill.IsDark());

	colorsChart.m_brPlotLineColor.Empty();
	colorsChart.m_brPlotFillColor.Empty();
	colorsChart.m_brChartFillColor.Empty();

	for (int i = 0; i < BCGP_GRID_SPARKLINES_CHART_SERIES_NUM; i++)
	{
		COLORREF clrSeriesFill = colorsGrid.m_SparklineSeriesColors[i].m_clrBackground;
		COLORREF clrSeriesBorder = colorsGrid.m_SparklineSeriesColors[i].m_clrBorder;

		if (clrSeriesFill == (COLORREF)-1 || clrSeriesBorder == (COLORREF)-1)
		{
			switch (i)
			{
			case 0:
			default:
				clrSeriesBorder = clrSeriesFill = RGB (1, 168, 220);
				break;

			case 1:
				clrSeriesBorder = clrSeriesFill = RGB(237, 125, 49);
				break;

			case 2:
				clrSeriesBorder = clrSeriesFill = RGB(112, 173, 71);
				break;

			case 3:
				clrSeriesBorder = clrSeriesFill = RGB(165, 165, 165);
				break;

			case 4:
				clrSeriesBorder = clrSeriesFill = RGB(255, 192, 0);
				break;
			}
		}

		colorsChart.m_seriesColors[i].m_brElementFillColor.SetColors(clrSeriesFill, clrSeriesFill, CBCGPBrush::BCGP_NO_GRADIENT);
		colorsChart.m_seriesColors[i].m_brElementLineColor.SetColor(clrSeriesBorder);
	}
	
	m_bDefaultSelColor = colorsGrid.m_bSparklineDefaultSelColor;

	pChart->SetColors(colorsChart);
	pChart->SetDirty (TRUE);
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPGridMergedCells

CBCGPGridMergedCells::CBCGPGridMergedCells (BOOL bAutoDelete/* = TRUE*/)
{
	m_rectMerged.SetRectEmpty ();
	m_bChanged = FALSE;
	m_bNeedRedraw = TRUE;
	m_idFirstVisible.SetNull ();

	m_nRefCount = 1;
	m_bAutoDelete = bAutoDelete;
}
//*****************************************************************************
CBCGPGridMergedCells::~CBCGPGridMergedCells ()
{
	if (m_nRefCount > 0)
	{
		TRACE0 ("CBCGPGridMergedCells: refCount != 0\n");
	}
}
//*****************************************************************************
const CBCGPGridItemID CBCGPGridMergedCells::GetMainItemID () const
{
	CBCGPGridItemID id (m_range.m_nTop, m_range.m_nLeft);
	return id;
}
//*****************************************************************************
const CBCGPGridItemID CBCGPGridMergedCells::GetVisibleItemID () const
{
	return m_idFirstVisible;
}
//*****************************************************************************
void CBCGPGridMergedCells::SetRect (const CRect& rect)
{
	m_rectMerged = rect;
}
//*****************************************************************************
void CBCGPGridMergedCells::SetRectEmpty ()
{
	m_rectMerged.SetRectEmpty ();
}
//*****************************************************************************
void CBCGPGridMergedCells::SetRange (const CBCGPGridRange& r)
{
	m_range.Set (r);
}
//*****************************************************************************
CRect& CBCGPGridMergedCells::GetRect ()
{
	return m_rectMerged;
}
//*****************************************************************************
const CRect& CBCGPGridMergedCells::GetRect () const
{
	return m_rectMerged;
}
//*****************************************************************************
CBCGPGridRange& CBCGPGridMergedCells::GetRange ()
{
	return m_range;
}
//*****************************************************************************
const CBCGPGridRange& CBCGPGridMergedCells::GetRange () const
{
	return m_range;
}
//*****************************************************************************
void CBCGPGridMergedCells::MarkChanged (const CRect& rectNew, const CBCGPGridItemID& id)
{
	ASSERT (m_range.IsInRange (id));
	if (!rectNew.IsRectEmpty () && !id.IsNull ())
	{
		if (!m_bChanged)
		//if (idFirstVisible.IsNull ())
		{
			m_idFirstVisible = id;
		}
		m_bChanged = TRUE;
		m_bNeedRedraw = TRUE;
	}
}
//*****************************************************************************
void CBCGPGridMergedCells::MarkUpdated ()
{
	m_bChanged = FALSE;
}
//*****************************************************************************
void CBCGPGridMergedCells::AddRef ()
{
	m_nRefCount++;
}
//*****************************************************************************
void CBCGPGridMergedCells::Release ()
{
	--m_nRefCount;
	if (m_bAutoDelete && m_nRefCount <= 0)
	{
		delete this;
	}
}
//*****************************************************************************
void CBCGPGridMergedCells::SetAutoDelete (BOOL bAutoDelete)
{
	m_bAutoDelete = bAutoDelete;
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPGridRow

IMPLEMENT_SERIAL(CBCGPGridRow, CObject, VERSIONABLE_SCHEMA | 1)

CString CBCGPGridRow::m_strFormatChar = _T("%c");
CString CBCGPGridRow::m_strFormatShort = _T("%d");
CString CBCGPGridRow::m_strFormatLong = _T("%ld");
CString CBCGPGridRow::m_strFormatUShort = _T("%u");
CString CBCGPGridRow::m_strFormatULong = _T("%u");
CString CBCGPGridRow::m_strFormatFloat = _T("%.2f");
CString CBCGPGridRow::m_strFormatDouble = _T("%.2lf");
CString CBCGPGridRow::m_strScanFormatFloat = _T("%f");
CString CBCGPGridRow::m_strScanFormatDouble = _T("%lf");

CBCGPGridRow::CBCGPGridRow (const CString& /*strGroupName*/, DWORD_PTR dwData, int nBlockSize) :
	m_dwData (dwData), m_lstSubItems (nBlockSize)
{
	m_bGroup = TRUE;
	m_bExpanded = TRUE;

	m_nIdRow = -1;

	Init ();
	SetFlags ();
}
//******************************************************************************************
CBCGPGridRow::CBCGPGridRow (int /*nColumnsNum*/, DWORD_PTR dwData, int nBlockSize) :
	m_dwData (dwData), m_lstSubItems (nBlockSize)
{
	m_bGroup = FALSE;
	m_bExpanded = TRUE;

	m_nIdRow = -1;

	Init ();
	SetFlags ();
}
//******************************************************************************************
void CBCGPGridRow::SetFlags ()
{
	m_dwFlags = 0;
}
//******************************************************************************************
void CBCGPGridRow::Init ()
{
	m_pWndList = NULL;
	m_nLines = 1;
	m_bSelected = FALSE;
	m_bEnabled = TRUE;
	m_bInPlaceEdit = FALSE;
	m_bAllowEdit = TRUE;
	m_bNameIsTrancated = FALSE;
	m_pParent = NULL;
	m_bDestroySubItems = FALSE;
	m_nMultiLineSubitemsCount = 0;

	m_Rect.SetRectEmpty ();
}
//*******************************************************************************************
CBCGPGridRow::~CBCGPGridRow()
{
	if (m_nLines > 1)
	{
		if (m_pParent != NULL)
		{
			ASSERT_VALID(m_pParent);
			m_pParent->m_nMultiLineSubitemsCount--;
		}

		if (m_pWndList != NULL)
		{
			ASSERT_VALID(m_pWndList);
			m_pWndList->m_nMultiLineExtraRows -= m_nLines - 1;
		}
	}

	while (!m_lstSubItems.IsEmpty ())
	{
		CBCGPGridRow* pSubItem = m_lstSubItems.RemoveHead ();

		if (m_bDestroySubItems)
		{
			delete pSubItem;
		}
	}

	for (int i = (int) m_arrRowItems.GetUpperBound (); i >= 0; i--)
	{
		delete m_arrRowItems [i];
	}

	m_arrRowItems.RemoveAll ();
}
//******************************************************************************************
CBCGPGridItem* CBCGPGridRow::CreateItem (int nRow, int nColumn)
{
	if (m_pWndList == NULL)
	{
		ASSERT (FALSE);
		return NULL;
	}

	ASSERT_VALID (m_pWndList);
	return m_pWndList->CreateItem (nRow, nColumn);
}
//******************************************************************************************
BOOL CBCGPGridRow::CanReplaceItem (int nColumn, CBCGPGridItem* pNewItem)
{
	if (pNewItem == NULL)
	{
		return FALSE;
	}

	ASSERT_VALID (pNewItem);
	if (m_pWndList == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	CBCGPGridItem* pOldItem = GetItem (nColumn);
	if (pOldItem != NULL && pOldItem->IsReadOnly ())
	{
		return FALSE;
	}
	
	ASSERT_VALID (m_pWndList);
	return m_pWndList->CanReplaceItem (GetRowId (), nColumn, pNewItem->GetRuntimeClass ());
}
//******************************************************************************************
BOOL CBCGPGridRow::ReplaceItem (int nColumn, CBCGPGridItem* pNewItem, BOOL bRedraw, BOOL bRepos)
{
	ASSERT_VALID (m_pWndList);

	if (nColumn >= 0 && nColumn < m_arrRowItems.GetSize ())
	{
		CBCGPGridItem* pOldItem = m_arrRowItems [nColumn];
		pNewItem->SetOwnerRow (this);
		pNewItem->m_nIdColumn = nColumn;
		m_arrRowItems [nColumn] = pNewItem;
		delete pOldItem;

		if (m_pWndList != NULL)
		{
			if (m_pWndList->m_pSelItem == pOldItem)
			{
				m_pWndList->m_pSelItem = pNewItem;
			}

			m_pWndList->m_idCur.m_nRow = GetRowId ();
		}

		int y = m_Rect.top;

		if (bRepos)
		{
			Repos (y);
		}

		if (bRedraw)
		{
			pNewItem->Redraw ();
		}

		return TRUE;
	}

	return FALSE;
}
//******************************************************************************************
void CBCGPGridRow::AllowSubItems (BOOL bGroup)
{
	m_bGroup = bGroup;
}
//******************************************************************************************
void CBCGPGridRow::AddItem (CBCGPGridItem* pItem)
{
	pItem->SetOwnerRow (this);
	int nIndex = (int) m_arrRowItems.Add (pItem);
	pItem->m_nIdColumn = nIndex;
}
//******************************************************************************************
void CBCGPGridRow::SetItemRTC (int /*nColumn*/, CRuntimeClass* /*pRuntimeClass*/)
{
}
//******************************************************************************************
void CBCGPGridRow::SetDefaultItemRTC (CRuntimeClass* /*pRTC*/)
{
}
//******************************************************************************************
void CBCGPGridRow::SetLinesNumber(int nLines, BOOL bRecalcLayout)
{
	ASSERT_VALID (this);

	nLines = max(nLines, 1);

	if (m_nLines == nLines)
	{
		return;
	}

	int nLinesPrev = m_nLines;

	int nDelta = (m_nLines - nLines);

	m_nLines = nLines;

	if (m_pParent != NULL)
	{
		ASSERT_VALID(m_pParent);
		
		if (m_nLines > 1 && nLinesPrev == 1)
		{
			m_pParent->m_nMultiLineSubitemsCount++;
		}
		else if (m_nLines == 1 && nLinesPrev > 1)
		{
			m_pParent->m_nMultiLineSubitemsCount--;
		}
	}

	if (m_pWndList != NULL)
	{
		ASSERT_VALID(m_pWndList);

		m_pWndList->m_nMultiLineExtraRows += nDelta;

		if (bRecalcLayout)
		{
			m_pWndList->AdjustLayout();
		}
	}
}
//******************************************************************************************
BOOL CBCGPGridRow::AddSubItem (CBCGPGridRow* pItem, BOOL bRedraw)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pItem);

	if (!IsGroup ())
	{
		ASSERT(FALSE);
		return FALSE;
	}

#ifdef _DEBUG	
	if (pItem->m_pWndList != NULL && CBCGPGridCtrl::m_bEnableAssertValidInDebug)
	{
		ASSERT_VALID (pItem->m_pWndList);

		for (POSITION pos = pItem->m_pWndList->m_lstItems.GetHeadPosition (); pos != NULL;)
		{
			CBCGPGridRow* pListItem = pItem->m_pWndList->m_lstItems.GetNext (pos);
			ASSERT_VALID (pListItem);

			if (pListItem == pItem || pListItem->IsSubItem (pItem))
			{
				// Can't add the same item twice
				ASSERT (FALSE);
				return FALSE;
			}
		}
	}
#endif // _DEBUG

	pItem->SetParent(this);

	int nPosParent = m_nIdRow;
	int nSubItemsCount = GetSubItemsCount (TRUE);
	int nPosInsertAfter = nPosParent + nSubItemsCount;
	m_pWndList->InsertRowAfter (nPosInsertAfter, pItem, bRedraw);

	m_lstSubItems.AddTail (pItem);
	pItem->m_pWndList = m_pWndList;

	return TRUE;
}
//*******************************************************************************************
int CBCGPGridRow::GetSubItemsCount (BOOL bRecursive) const
{
	ASSERT_VALID (this);

	if (!bRecursive)
	{
		return (int) m_lstSubItems.GetCount ();
	}

	int nCount = 0;

	for (POSITION pos = m_lstSubItems.GetHeadPosition (); pos != NULL;)
	{
		CBCGPGridRow* pItem = m_lstSubItems.GetNext (pos);
		ASSERT_VALID (pItem);

		nCount += pItem->GetSubItemsCount (TRUE) + 1;
	}

	return nCount;
}
//*******************************************************************************************
void CBCGPGridRow::GetSubItems (CList<CBCGPGridRow*, CBCGPGridRow*>& lst,
								BOOL bRecursive)
{
	ASSERT_VALID (this);

	if (!bRecursive)
	{
		lst.AddTail (&m_lstSubItems);
		return;
	}

	for (POSITION pos = m_lstSubItems.GetHeadPosition (); pos != NULL;)
	{
		CBCGPGridRow* pItem = m_lstSubItems.GetNext (pos);
		ASSERT_VALID (pItem);

		lst.AddTail (pItem);
		pItem->GetSubItems (lst, TRUE);
	}
}
//*******************************************************************************************
CBCGPGridRow* CBCGPGridRow::HitTest (CPoint point, int &iColumn, 
									 CBCGPGridItem*& pGridItem,
									 CBCGPGridRow::ClickArea* pnArea)
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pWndList);

	if (m_Rect.PtInRect (point))
	{
		int dx = m_pWndList->IsSortingMode () && !m_pWndList->IsGrouping () ? 0 : 
			GetHierarchyLevel () * m_pWndList->GetHierarchyLevelOffset ();

		CRect rectExpand = m_Rect;
		if ((IsGroup () && m_pWndList->m_bFreezeGroups) || (m_pWndList->m_nHorzScrollOffset > 0 && m_pWndList->GetColumnsInfo ().GetFrozenColumnCount () > 0))
		{	// do not scroll expandbox of frozen columns
			rectExpand.OffsetRect (m_pWndList->m_nHorzScrollOffset, 0);
		}
		rectExpand.DeflateRect (dx, 0, 0, 0);
		rectExpand.right = min (rectExpand.left + max(m_pWndList->GetExtraHierarchyOffset (), m_pWndList->GetButtonWidth ()), rectExpand.right);
		
		if (IsGroup () && (!m_pWndList->IsSortingMode () || m_pWndList->IsGrouping ()) 
			&& rectExpand.PtInRect (point))
		{
			if (pnArea != NULL)
			{
				*pnArea = ClickExpandBox;
			}
		}
		else
		{
			if (HasValueField ())
			{
				for (int i = 0; i < m_arrRowItems.GetSize (); i++)
				{
					CBCGPGridItem* pItem = m_arrRowItems [i];
					ASSERT_VALID (pItem);

					CBCGPGridItem* pHit = pItem->HitTest (point, pnArea);
					if (pHit != NULL)
					{
						iColumn = i;
						pGridItem = pHit;
						return this;
					}
				}
			}

			if (pnArea != NULL)
			{
				*pnArea = ClickName;
			}
		}

		iColumn = -1;
		pGridItem = NULL;
		return this;
	}

	return NULL;
}
//*******************************************************************************************
void CBCGPGridRow::Expand (BOOL bExpand)
{
	ASSERT_VALID (this);
	ASSERT (IsGroup ());

	bExpand = (bExpand != FALSE);

	if (m_bExpanded == bExpand || m_lstSubItems.IsEmpty ())
	{
		return;
	}

	CWaitCursor wait;

	OnExpand (bExpand);

	m_bExpanded = bExpand;

	if (m_pWndList != NULL && m_pWndList->GetSafeHwnd () != NULL)
	{
		ASSERT_VALID (m_pWndList);

		if (m_nMultiLineSubitemsCount > 0)
		{
			int nDelta = 0;

			for (POSITION pos = m_lstSubItems.GetHeadPosition(); pos != NULL;)
			{
				CBCGPGridRow* pSubItem = m_lstSubItems.GetNext(pos);
				ASSERT_VALID(pSubItem);

				nDelta += pSubItem->GetLinesNumber() - 1;
			}

			if (bExpand)
			{
				m_pWndList->m_nMultiLineExtraRows += nDelta;
			}
			else
			{
				m_pWndList->m_nMultiLineExtraRows -= nDelta;
			}
		}

		BOOL bRebuildTerminalItemsOld = m_pWndList->m_bRebuildTerminalItems;
		m_pWndList->m_bRebuildTerminalItems = FALSE;

		m_pWndList->AdjustLayout ();

		if (!m_pWndList->IsVirtualMode () && (m_pWndList->IsRowExtraHeightAllowed () || m_nLines > 1))
		{
			// Recalc extra height for row with specific height
			m_pWndList->AdjustLayout ();
		}

		m_pWndList->m_bRebuildTerminalItems = bRebuildTerminalItemsOld;

		CRect rectRedraw = m_pWndList->m_rectList;
		rectRedraw.top = m_Rect.top;

		m_pWndList->RedrawWindow (rectRedraw);
	}
}
//*******************************************************************************************
void CBCGPGridRow::ExpandDeep (BOOL bExpand)
{
	ASSERT_VALID (this);

	m_bExpanded = bExpand;

	for (POSITION pos = m_lstSubItems.GetHeadPosition (); pos != NULL;)
	{
		CBCGPGridRow* pItem = m_lstSubItems.GetNext (pos);
		ASSERT_VALID (pItem);

		pItem->ExpandDeep (bExpand);
	}
}
//*******************************************************************************************
BOOL CBCGPGridRow::IsItemFiltered () const
{
	return m_pWndList->FilterItem (this);
}
//*******************************************************************************************
void CBCGPGridRow::Redraw ()
{
	ASSERT_VALID (this);

	if (m_pWndList != NULL)
	{
		ASSERT_VALID (m_pWndList);
		m_pWndList->InvalidateRect (m_Rect);

		for (int i = 0; i < m_arrRowItems.GetSize (); i++)
		{
			CBCGPGridMergedCells* pMerged = m_arrRowItems[i]->GetMergedCells ();
			if (pMerged != NULL && pMerged->GetVisibleItemID ().m_nRow != m_nIdRow)
			{
				m_pWndList->InvalidateRect (pMerged->GetRect ());
			}
		}

		for (POSITION pos = m_lstSubItems.GetHeadPosition (); pos != NULL;)
		{
			CBCGPGridRow* pItem = m_lstSubItems.GetNext (pos);
			ASSERT_VALID (pItem);

			m_pWndList->InvalidateRect (pItem->m_Rect);
		}

		if (!m_pWndList->m_bNoUpdateWindow)
		{
			m_pWndList->UpdateWindow ();
		}
	}
}
//*****************************************************************************************
void CBCGPGridRow::AdjustButtonRect ()
{
	ASSERT_VALID (m_pWndList);
	// adjust all buttons in the row
	for (int i = 0; i < m_arrRowItems.GetSize (); i++)
	{
		CBCGPGridItem* pItem = m_arrRowItems [i];
		ASSERT_VALID (pItem);

		if (!pItem->m_rectButton.IsRectEmpty () && pItem->HasButton ())
		{
			pItem->AdjustButtonRect ();
			m_pWndList->InvalidateRect (pItem->m_rectButton);
		}
	}
}
//*******************************************************************************************
void CBCGPGridRow::Select (BOOL bSelect)
{
	m_bSelected = bSelect;
}
//*******************************************************************************************
BOOL CBCGPGridRow::IsSelected () const
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pWndList);

	return m_pWndList->GetCurSel () == this || m_bSelected;
}
//*****************************************************************************************
BOOL CBCGPGridRow::IsParentExpanded () const
{
	ASSERT_VALID (this);

	for (CBCGPGridRow* pItem = m_pParent; pItem != NULL;)
	{
		ASSERT_VALID (pItem);

		if (!pItem->IsExpanded ())
		{
			return FALSE;
		}

		pItem = pItem->m_pParent;
	}

	return TRUE;
}
//******************************************************************************************
int CBCGPGridRow::GetHierarchyLevel () const
{
	ASSERT_VALID (this);

	int nLevel = 0;
	for (CBCGPGridRow* pParent = m_pParent; pParent != NULL;
		pParent = pParent->m_pParent)
	{
		nLevel++;
	}

	return nLevel;
}
//*******************************************************************************************
CBCGPGridRow* CBCGPGridRow::GetSubItem (int nIndex) const
{
	ASSERT_VALID (this);

	if (nIndex < 0 || nIndex >= m_lstSubItems.GetCount ())
	{
		ASSERT (FALSE);
		return NULL;
	}

	return m_lstSubItems.GetAt (m_lstSubItems.FindIndex (nIndex));
}
//*******************************************************************************************
CWnd* CBCGPGridRow::GetInPlaceWnd () const
{
	if (!m_bInPlaceEdit)
	{
		return NULL;
	}

	const CArray<CBCGPGridItem*, CBCGPGridItem*>& arr = m_arrRowItems;

	for (int i = 0; i < arr.GetSize (); i++)
	{
		CBCGPGridItem* pItem = arr [i];
		ASSERT_VALID (pItem);

		if (pItem->m_pWndInPlace != NULL)
		{
			ASSERT_VALID (pItem->m_pWndInPlace);
			return pItem->m_pWndInPlace;
		}
	}

	ASSERT (FALSE);
	return NULL;
}
//*******************************************************************************************
CComboBox* CBCGPGridRow::GetComboWnd () const
{
	if (!m_bInPlaceEdit)
	{
		return NULL;
	}

	const CArray<CBCGPGridItem*, CBCGPGridItem*>& arr = m_arrRowItems;

	for (int i = 0; i < arr.GetSize (); i++)
	{
		CBCGPGridItem* pItem = arr [i];
		ASSERT_VALID (pItem);

		if (pItem->m_pWndCombo != NULL)
		{
			ASSERT_VALID (pItem->m_pWndCombo);
			return pItem->m_pWndCombo;
		}
	}

	return NULL;
}
//*******************************************************************************************
CSpinButtonCtrl* CBCGPGridRow::GetSpinWnd () const
{
	if (!m_bInPlaceEdit)
	{
		return NULL;
	}

	const CArray<CBCGPGridItem*, CBCGPGridItem*>& arr = m_arrRowItems;

	for (int i = 0; i < arr.GetSize (); i++)
	{
		CBCGPGridItem* pItem = arr [i];
		ASSERT_VALID (pItem);

		if (pItem->m_pWndSpin != NULL)
		{
			ASSERT_VALID (pItem->m_pWndSpin);
			return pItem->m_pWndSpin;
		}
	}

	return NULL;
}
//*******************************************************************************************
void CBCGPGridRow::Enable (BOOL bEnable/* = TRUE*/)
{
	ASSERT_VALID (this);

	if (m_bEnabled != bEnable)
	{
		m_bEnabled = bEnable;

		if (m_pWndList->GetSafeHwnd () != NULL)
		{
			ASSERT_VALID (m_pWndList);
			m_pWndList->InvalidateRect (m_Rect);
		}
	}
}
//*******************************************************************************************
void CBCGPGridRow::SetOwnerList (CBCGPGridCtrl* pWndList)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pWndList);

	if (m_pWndList != pWndList)
	{
		m_pWndList = pWndList;
		m_pWndList->m_nMultiLineExtraRows += m_nLines - 1;
	}

	for (POSITION pos = m_lstSubItems.GetHeadPosition (); pos != NULL;)
	{
		CBCGPGridRow* pItem = m_lstSubItems.GetNext (pos);
		ASSERT_VALID (pItem);

		pItem->SetOwnerList (m_pWndList);
	}
}
//*******************************************************************************************
void CBCGPGridRow::SetVertAlign(DWORD nAlign)
{
	ASSERT_VALID (this);

	const CArray<CBCGPGridItem*, CBCGPGridItem*>& arr = m_arrRowItems;

	for (int i = 0; i < arr.GetSize(); i++)
	{
		CBCGPGridItem* pItem = arr[i];
		ASSERT_VALID(pItem);

		pItem->SetVertAlign(nAlign);
	}
}
//*******************************************************************************************
void CBCGPGridRow::SetParent(CBCGPGridRow* pParent)
{
	ASSERT_VALID (this);

	if (m_nLines > 1 && m_pParent != pParent)
	{
		if (m_pParent != NULL)
		{
			ASSERT_VALID(m_pParent);
			m_pParent->m_nMultiLineSubitemsCount--;
		}

		if (pParent != NULL)
		{
			ASSERT_VALID(pParent);
			pParent->m_nMultiLineSubitemsCount++;
		}
	}

	m_pParent = pParent;
}
//*******************************************************************************************
void CBCGPGridRow::Repos (int& y)
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pWndList);

	if (m_pWndList->m_bIsPrinting)
	{
		m_nIdRow = m_pWndList->m_PrintParams.m_idCur.m_nRow;
		m_pWndList->m_PrintParams.m_idCur.m_nRow ++;
	}
	else
	{
		m_nIdRow = m_pWndList->m_idCur.m_nRow;
		m_pWndList->m_idCur.m_nRow ++;
	}

	CRect rectOld = m_Rect;

	BOOL bShowAllItems = (m_pWndList->IsSortingMode () && !m_pWndList->IsGrouping ());
	BOOL bShowItem = bShowAllItems ? !IsItemFiltered () : IsItemVisible ();
	if (bShowItem)
	{
		int nXMul = 1, nXDiv = 1;
		if (m_pWndList->m_bIsPrinting)
		{
			// map to printer metrics
			ASSERT_VALID (m_pWndList->m_pPrintDC);
			HDC hDCFrom = ::GetDC(NULL);

			nXMul = m_pWndList->m_pPrintDC->GetDeviceCaps(LOGPIXELSX); // pixels in print dc
			nXDiv = ::GetDeviceCaps(hDCFrom, LOGPIXELSX); // pixels in screen dc

			::ReleaseDC(NULL, hDCFrom);
		}

		int nHorzScrollOffset = m_pWndList->m_nHorzScrollOffset;
		int nHierarchyLevelOffset = m_pWndList->GetHierarchyLevelOffset ();
		if (m_pWndList->m_bIsPrinting)
		{
			nHorzScrollOffset = m_pWndList->m_PrintParams.m_nHorzScrollOffset;
			nHierarchyLevelOffset *= m_pWndList->m_PrintParams.m_pageInfo.m_szOne.cx;
		}

		int nRowHeight = (m_pWndList->m_bIsPrinting ? m_pWndList->m_PrintParams.m_nRowHeight : m_pWndList->m_nRowHeight) * m_nLines;
		int nLargeRowHeight = m_pWndList->m_bIsPrinting ? m_pWndList->m_PrintParams.m_nLargeRowHeight : m_pWndList->m_nLargeRowHeight;
		CRect& rectList = m_pWndList->m_bIsPrinting ? m_pWndList->m_PrintParams.m_rectList: m_pWndList->m_rectList;


		int dx = m_pWndList->IsSortingMode () && !m_pWndList->IsGrouping () ? 0 :
			GetHierarchyLevel () * nHierarchyLevelOffset;

		int nRowLeft = rectList.left;
		int nRowLeftScrolled = rectList.left - nHorzScrollOffset;
		int nRowWidth = m_pWndList->GetColumnsInfo ().GetTotalWidth ();
		
		if (m_pWndList->m_bIsPrinting)
		{
			// map to printer metrics
			nRowWidth = ::MulDiv(nRowWidth, nXMul, nXDiv);
		}

		CRect rectRow (
			nRowLeftScrolled,
			y,
			nRowLeftScrolled + nRowWidth,
			y + (IsGroup () ? nLargeRowHeight : nRowHeight));
		OnMeasureGridRowRect (rectRow);
		m_Rect = rectRow;

		// -----------------
		// Repos grid items:
		// -----------------

		for (int i = 0; i < m_arrRowItems.GetSize (); i++)
		{
			CBCGPGridItem* pItem = m_arrRowItems [i];
			ASSERT_VALID (pItem);

			pItem->m_Rect.SetRectEmpty ();
			pItem->m_rectButton.SetRectEmpty ();
		}
			
		CSize szOne = m_pWndList->m_bIsPrinting ? 
			m_pWndList->m_PrintParams.m_pageInfo.m_szOne : CSize (1, 1);

		int nXLeft = nRowLeft;
		int nXLeftScrolled = nRowLeftScrolled;
		int nCount = 0;

		int nPos = m_pWndList->GetColumnsInfo ().Begin ();
		while (nPos != m_pWndList->GetColumnsInfo ().End ())
		{
			int iColumn = m_pWndList->GetColumnsInfo ().Next (nPos);
			if (iColumn == -1)
			{
				break; // no more visible columns
			}

			ASSERT (iColumn >= 0);
			if (m_arrRowItems.GetSize () <= iColumn)
			{
				continue; // row has no item for this column
			}

			CBCGPGridItem* pItem = m_arrRowItems [iColumn];
			ASSERT_VALID (pItem);

			CRect rectItemOld = pItem->m_Rect;

			BOOL bIsTreeColumn = (m_pWndList->m_nTreeColumn == -1) ? (nCount == 0):
				(m_pWndList->m_nTreeColumn == iColumn);

			int nWidth = m_pWndList->GetColumnsInfo ().GetColumnWidth (iColumn);
			if (m_pWndList->m_bIsPrinting)
			{
				// map to printer metrics
				nWidth = ::MulDiv(nWidth, nXMul, nXDiv);
			}

			int nTreeOffset = bIsTreeColumn ? m_pWndList->GetExtraHierarchyOffset () * szOne.cx + dx : 0; 
			if (bIsTreeColumn)
			{
				nWidth += m_pWndList->GetExtraHierarchyOffset () * szOne.cx +
					m_pWndList->GetHierarchyOffset () * szOne.cx;
			}

			BOOL bShowColumn = (nWidth > 0);

			if (bShowItem && bShowColumn)
			{
				// Item, which is inside frozen area, can't be scrolled
				int nLeft = (nCount < m_pWndList->GetColumnsInfo ().GetFrozenColumnCount ()) ? nXLeft : nXLeftScrolled;
				if (m_pWndList->m_bIsPrinting)
				{
					nLeft = nXLeftScrolled;
				}
				CRect rectItem (
					nLeft + nTreeOffset,
					y, 
					min (nLeft + nWidth - 1, m_Rect.right),
					y + (IsGroup () ? nLargeRowHeight : nRowHeight));
				OnMeasureGridItemRect (rectItem, pItem);
				pItem->m_Rect = rectItem;

				if (!pItem->m_rectButton.IsRectEmpty ())
				{
					pItem->m_rectButton.top = pItem->m_Rect.top + 1;
					pItem->m_rectButton.bottom = pItem->m_Rect.bottom;
				}
			}
			else
			{
				pItem->m_Rect.SetRectEmpty ();
				pItem->m_rectButton.SetRectEmpty ();
			}

			if (pItem->GetMergedCells () != NULL)
			{
				pItem->ReposMergedItem ();
			}
			else
			{
				pItem->OnPosSizeChanged (rectItemOld);
			}

			nXLeft += nWidth;
			nXLeftScrolled += nWidth;
			nCount ++;
		}

		y += m_Rect.Height ();
	}
	else
	{
		m_Rect.SetRectEmpty ();
	}

	OnPosSizeChanged (rectOld);
}
//*******************************************************************************************
void CBCGPGridRow::Shift (int dx, int dy)
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pWndList);

	for (int iColumn = 0; iColumn < m_arrRowItems.GetSize (); iColumn++)
	{
		CBCGPGridItem* pItem = m_arrRowItems [iColumn];
		ASSERT_VALID (pItem);

		if (!pItem->m_Rect.IsRectEmpty ())
		{
			CRect rectItemOld = pItem->m_Rect;
			
			if (m_pWndList->GetColumnsInfo ().IsFreezeColumnsEnabled () &&
				m_pWndList->GetColumnsInfo ().IsColumnFrozen (iColumn) && dy == 0)
			{
				// not shift horizontaly inside frozen area
				rectItemOld.right += 2;
				m_pWndList->InvalidateRect (rectItemOld);
				continue;
			}

			pItem->m_Rect.OffsetRect (dx, dy);

			if (pItem->GetMergedCells () != NULL)
			{
				pItem->ReposMergedItem ();
			}
			else
			{
				pItem->OnPosSizeChanged (rectItemOld);
			}
		}
	}

	if (IsGroup () && m_pWndList->GetColumnsInfo ().IsFreezeColumnsEnabled () && dy == 0)
	{
		const int nFreezeOffset = m_pWndList->m_rectList.left + m_pWndList->GetColumnsInfo ().GetFreezeOffset ();
		CRect rect = m_Rect;
		rect.right = nFreezeOffset;
		m_pWndList->InvalidateRect (rect);
	}

	if (!m_Rect.IsRectEmpty ())
	{
		CRect rectOld = m_Rect;
		m_Rect.OffsetRect (dx, dy);

		OnPosSizeChanged (rectOld);
	}
}
//******************************************************************************************
void CBCGPGridRow::AddTerminalItem (CList<CBCGPGridRow*, CBCGPGridRow*>& lstItems)
{
	ASSERT_VALID (this);
	ASSERT (!m_pWndList->IsGrouping ());

	if (!m_bGroup || HasValueField ())
	{
		// ---------------------------------
		// Simple sorting (one sort column):
		// ---------------------------------
		if (!m_pWndList->IsMultipleSort ())
		{
			// Insert sorted:
			BOOL bInserted = FALSE;
			for (POSITION pos = lstItems.GetHeadPosition (); !bInserted && pos != NULL;)
			{
				POSITION posSave = pos;

				CBCGPGridRow* pItem = lstItems.GetNext (pos);

				int iSortedColumn = m_pWndList->GetSortColumn ();
				if (m_pWndList->CompareItems (pItem, this, iSortedColumn) > 0)
				{
					lstItems.InsertBefore (posSave, this);
					bInserted = TRUE;
				}
			}

			if (!bInserted)
			{
				lstItems.AddTail (this);
			}
		}

		// -------------
		// MultiSorting:
		// -------------
		else
		{
			// Get sort order:
			const int nSortCount = m_pWndList->GetColumnsInfo ().GetGroupColumnCount () + m_pWndList->GetColumnsInfo ().GetSortColumnCount ();
			int* aSortOrder = new int [nSortCount];
			memset (aSortOrder, 0, nSortCount * sizeof (int));

			if (!m_pWndList->GetColumnsInfo ().GetGroupingColumnOrderArray ((LPINT) aSortOrder, nSortCount))
			{
				ASSERT (FALSE);	// error getting sort order

				lstItems.AddTail (this);
				delete [] aSortOrder;
				return;
			}

			// Insert sorted:
			BOOL bInserted = FALSE;
			for (POSITION pos = lstItems.GetHeadPosition (); !bInserted && pos != NULL;)
			{
				POSITION posSave = pos;

				CBCGPGridRow* pItem = lstItems.GetNext (pos);

				BOOL bTryNextSortedColumn = TRUE;
				int iLevel = 0;			// 0, ..., nSortCount - 1
				while (!bInserted && bTryNextSortedColumn && iLevel < nSortCount)
				{
					int iSortedColumn = aSortOrder [iLevel];
					
					int nCompare = m_pWndList->CompareItems (pItem, this, iSortedColumn);
					if (nCompare > 0)
					{
						lstItems.InsertBefore (posSave, this);
						bInserted = TRUE;
						bTryNextSortedColumn = FALSE;
					}
					else if (nCompare == 0)
					{
						iLevel++;
					}
					else
					{
						bTryNextSortedColumn = FALSE;
					}
				} // while
			}

			delete [] aSortOrder;

			if (!bInserted)
			{
				lstItems.AddTail (this);
			}
		}
	}
	else
	{
		m_Rect.SetRectEmpty ();
	}
}
//****************************************************************************************
void CBCGPGridRow::AddGroupedItem (CList<CBCGPGridRow*, CBCGPGridRow*>& lstItems)
{
	ASSERT_VALID (this);
	ASSERT (m_pWndList->IsGrouping ());

	if (!m_bGroup || HasValueField ())
	{
		CList <int, int> & lstGroupingColumns = m_pWndList->GetColumnsInfo ().m_lstGroupingColumns;
		const int nGroupingColumnsNum = (int) lstGroupingColumns.GetCount ();

		// Get sort order:
		const int nSortCount = m_pWndList->GetColumnsInfo ().GetGroupColumnCount () + m_pWndList->GetColumnsInfo ().GetSortColumnCount ();
		int* aSortOrder = new int [nSortCount];
		memset (aSortOrder, 0, nSortCount * sizeof (int));

		if (!m_pWndList->GetColumnsInfo ().GetGroupingColumnOrderArray ((LPINT) aSortOrder, nSortCount))
		{
			ASSERT (FALSE);	// error getting sort order

			lstItems.AddTail (this);
			delete [] aSortOrder;
			return;
		}

		CArray <POSITION, POSITION> arrPositions;
		arrPositions.SetSize (nGroupingColumnsNum);
		for (int i = 0; i < arrPositions.GetSize (); i++)
		{
			arrPositions [i] = NULL;
		}

		POSITION posGroup = NULL;

		// Insert sorted:
		BOOL bInserted = FALSE;
		for (POSITION pos = lstItems.GetHeadPosition (); !bInserted && pos != NULL;)
		{
			POSITION posSave = pos;

			CBCGPGridRow* pItem = lstItems.GetNext (pos);
			ASSERT_VALID (pItem);

			BOOL bIsAutoGroup = pItem->IsGroup () &&
				(pItem->m_dwFlags & BCGP_GRID_ITEM_AUTOGROUP) != 0;
			if (bIsAutoGroup)
			{
				continue; // skip autogroup item
			}

			BOOL bTryNextSortedColumn = TRUE;
			int iLevel = 0;			// 0, ..., nGroupingColumnsNum - 1
			while (!bInserted && bTryNextSortedColumn && iLevel < nSortCount)
			{
				int iSortedColumn = aSortOrder [iLevel];

				int nCompare = 0;
				if (iLevel < nGroupingColumnsNum) // grouping
				{
					nCompare = m_pWndList->CompareGroup (pItem, this, iSortedColumn);
				}
				else	// compare items inside same group
				{
					nCompare = m_pWndList->CompareItems (pItem, this, iSortedColumn);
				}

				if (nCompare > 0)
				{
					POSITION posInserted = NULL;
					if (iLevel < nGroupingColumnsNum)
					{
						POSITION posFirstItemInGroup = NULL;
						int nParentLevel = -1;
						for (int i = 0; i < nGroupingColumnsNum; i++)
						{
							if (arrPositions [i] != NULL)
							{
								nParentLevel = i;
								posFirstItemInGroup = arrPositions [i];
							}
						}

						POSITION posInsertBefore = posSave;
						posGroup = InsertAutoGroupBefore (lstItems, posInsertBefore, posFirstItemInGroup, nParentLevel, this);
						posInserted = lstItems.InsertBefore (posInsertBefore, this);
					}
					else
					{
						ASSERT (iLevel >= nGroupingColumnsNum);
						POSITION posFirstItemInGroup = arrPositions [nGroupingColumnsNum - 1];
						POSITION posInsertBefore = posSave;
						posGroup = InsertAutoGroupBefore (lstItems, posInsertBefore, posFirstItemInGroup, nGroupingColumnsNum - 1, this);

						posInserted = lstItems.InsertBefore (posSave, this);
					}
					bInserted = TRUE;
					bTryNextSortedColumn = FALSE;

					for (int i = 0; i < arrPositions.GetSize (); i++)
					{
						if (posSave == arrPositions [i])
						{
							arrPositions [i] = posInserted;
						}
					}
				}
				else if (nCompare == 0)
				{
					if (iLevel < nGroupingColumnsNum && 
						arrPositions [iLevel] == NULL)
					{
						arrPositions [iLevel] = posSave;
					}
					iLevel++;
				}
				else
				{
					bTryNextSortedColumn = FALSE;
				}
			} // while
		}

		if (!bInserted)
		{
			POSITION posFirstItemInGroup = NULL;
			int nParentLevel = -1;
			for (int i = 0; i < nGroupingColumnsNum; i++)
			{
				if (arrPositions [i] != NULL)
				{
					nParentLevel = i;
					posFirstItemInGroup = arrPositions [i];
				}
			}

			POSITION posInsertBefore = NULL;
			posGroup = InsertAutoGroupBefore (lstItems, posInsertBefore, posFirstItemInGroup, nParentLevel, this);
			lstItems.AddTail (this);
		}

		//POSITION posGroup = arrPositions [nGroupingColumnsNum - 1];
		// Add item as subitem for the group
		if (posGroup != NULL)
		{
			CBCGPGridRow* pGroup = lstItems.GetAt (posGroup);
			BOOL bIsAutoGroup = pGroup != NULL && pGroup->IsGroup () && (pGroup->m_dwFlags & BCGP_GRID_ITEM_AUTOGROUP) != 0;
			if (bIsAutoGroup)
			{
				ASSERT_VALID (pGroup);

				// Add item as subitem for the group
				pGroup->m_lstSubItems.AddTail (this);

				SetParent(pGroup);
			}
		}

		delete [] aSortOrder;
	}
	else
	{
		m_Rect.SetRectEmpty ();
	}
}
//****************************************************************************************
POSITION CBCGPGridRow::InsertAutoGroupBefore (
    CList<CBCGPGridRow*, CBCGPGridRow*>& lstItems, 
    POSITION& posInsertBefore, POSITION posFirstItemInGroup, int nParentLevel,
	CBCGPGridRow* pGroupedItem)
/*	if posInsertBefore - NULL, add last
	else use nParentLevel to skip not parent autogroups
		nParentLevel (0 <= nParentLevel <= ColumnNum-1)
		nParentLevel - -1, skip all autogroups (ColumnNum autogroups)
		nParentLevel - 0, skip ColumnNum-1 autogroups
		nParentLevel - ColumnNum-1, skip 0 autogroup
	if posFirstItemInGroup - NULL, create full hierarchy (all autogroups), nParentLevel is ignored
	else uses nParentLevel to search parent autogroup
		nParentLevel (0 <= nParentLevel <= ColumnNum-1)
		nParentLevel - -1, create full hierarchy (nParentLevel autogroups), do not search parent
		nParentLevel - 0, if parent found then create nParentLevel-1 autogroups, else create full hierarchy
		nParentLevel - ColumnNum-1, if parent found then do not create autogroup, else create full hierarchy

	nParentLevel - search parent autogroup at this level
	if nParentLevel - -1, 
*/
{
    ASSERT_VALID (this);
	ASSERT_VALID (m_pWndList);
    ASSERT (nParentLevel >= -1);
	ASSERT_VALID (pGroupedItem);

	const int nGroupingColumnsNum = 
		(int) m_pWndList->GetColumnsInfo ().m_lstGroupingColumns.GetCount ();
    CBCGPGridRow* pParentGroup = NULL;
    POSITION posParentGroup = NULL;

    // Find parent group:
	if (posFirstItemInGroup != NULL)
	{
		POSITION pos = posFirstItemInGroup;
		POSITION posSave = posFirstItemInGroup;

        CBCGPGridRow* pItem = lstItems.GetPrev (pos);
        ASSERT_VALID (pItem);

		CBCGPGridRow* pGroup = NULL;
        if (pos != NULL)
        {
			posSave = pos;

            pGroup = lstItems.GetPrev (pos);
            ASSERT_VALID (pGroup);
        }

		int iLevel = nGroupingColumnsNum - 1;
		do
		{
			BOOL bIsAutoGroup = pGroup != NULL && pGroup->IsGroup () && (pGroup->m_dwFlags & BCGP_GRID_ITEM_AUTOGROUP) != 0;
			if (bIsAutoGroup && pItem != NULL && pGroup->IsSubItem (pItem))
			{
				if (iLevel == nParentLevel && iLevel >= 0)
				{
					pParentGroup = pGroup;
					posParentGroup = posSave;
				}

				// go level up
				pItem = pGroup;
				if (pos != NULL)
				{
					posSave = pos;

					pGroup = lstItems.GetPrev (pos);
					ASSERT_VALID (pGroup);
				}
				else
				{
					pGroup = NULL;
				}
			}

			// go level up
			iLevel--;
		}
		while (iLevel >= nParentLevel);
	}

	// Skip autogroups
    if (posInsertBefore != NULL)
    {
		POSITION pos = posInsertBefore;
		POSITION posSave = posInsertBefore;

        CBCGPGridRow* pItem = lstItems.GetPrev (pos);
        ASSERT_VALID (pItem);

		CBCGPGridRow* pGroup = NULL;
        if (pos != NULL)
        {
			posSave = pos;

            pGroup = lstItems.GetPrev (pos);
            ASSERT_VALID (pGroup);
        }

		BOOL bSkipAllLevels = FALSE;
		int iLevel = nGroupingColumnsNum - 1;
		do
		{
			BOOL bIsAutoGroup = pGroup != NULL && pGroup->IsGroup () && (pGroup->m_dwFlags & BCGP_GRID_ITEM_AUTOGROUP) != 0;
			if (bIsAutoGroup && pItem != NULL && pGroup->IsSubItem (pItem))
			{
				if (iLevel == nParentLevel && iLevel >= 0)
				{
					if (pParentGroup == NULL)
					{
						pParentGroup = pGroup;
					}
					bSkipAllLevels = pParentGroup != pGroup;
				}

				if (iLevel > nParentLevel || bSkipAllLevels)
				{
					posInsertBefore = posSave;	// insert before not parent autogroup
					ASSERT (posInsertBefore != NULL);
				}

				// go level up
				pItem = pGroup;
				if (pos != NULL)
				{
					posSave = pos;

					pGroup = lstItems.GetPrev (pos);
					ASSERT_VALID (pGroup);
				}
				else
				{
					pGroup = NULL;
				}
			}

			// go level up
			iLevel--;
		}
		while (iLevel >= nParentLevel || bSkipAllLevels && iLevel >= 0);
    }

	if (posParentGroup != NULL)
	{
		// parent group found - skip one level
		nParentLevel++;
	}
	else
	{
		// parent group not found - create full hierarchy
		nParentLevel = 0;
	}

	if (nParentLevel >= nGroupingColumnsNum)
	{
		return posParentGroup;
	}

    POSITION posGroup = NULL;
    for (int iLevel = nParentLevel; iLevel < nGroupingColumnsNum; iLevel++)
    {
	    // Create auto group:
	    CString strGroup;

		int nGroupCol = m_pWndList->GetColumnsInfo ().GetGroupColumn (iLevel);
		int nGroupNumber = m_pWndList->OnGetGroupNumber (pGroupedItem, nGroupCol);
		if (nGroupNumber != -1)
		{
			strGroup = m_pWndList->OnGetGroupText (nGroupNumber, nGroupCol);
		}
		else
		{
			CBCGPGridItem* pItem = pGroupedItem->GetItem (nGroupCol);
			if (pItem != NULL)
			{
				ASSERT_VALID (pItem);
				strGroup = m_pWndList->GetGroupName (nGroupCol, pItem);
			}
		}

		CBCGPGridRow* pGroup = m_pWndList->CreateRow (strGroup);
		ASSERT_VALID (pGroup);

		pGroup->m_dwFlags |= BCGP_GRID_ITEM_AUTOGROUP;
		pGroup->m_bExpanded = IsAutoGroupExpanded (pGroupedItem, nGroupingColumnsNum - 1 - iLevel);
		m_pWndList->m_lstAutoGroups.AddTail (pGroup);

        if (posInsertBefore != NULL)
        {
	        posGroup = lstItems.InsertBefore (posInsertBefore, pGroup);
        }
        else
        {
            lstItems.AddTail (pGroup);
            posGroup = lstItems.GetTailPosition ();
        }
        ASSERT (posGroup != NULL);
	    pGroup->SetOwnerList (m_pWndList);

	    // Add group as subitem for the parent group:
	    if (pParentGroup != NULL)
        {
			pParentGroup->m_lstSubItems.AddTail (pGroup);
			pGroup->SetParent(pParentGroup);
        }

        pParentGroup = pGroup;
    }

    return posGroup;
}
//****************************************************************************************
BOOL CBCGPGridRow::IsAutoGroupExpanded (CBCGPGridRow* pGroupedItem, int iLevel) const
{
	ASSERT_VALID (pGroupedItem);
	ASSERT (iLevel >= 0);
	ASSERT_VALID (m_pWndList);

	CList<CBCGPGridRow*, CBCGPGridRow*>& lstOldAutoGroups = 
		m_pWndList->m_lstOldAutoGroups;

	CBCGPGridRow* pParentAutoGroup = NULL;

	// Find direct parent:
	for (POSITION pos = lstOldAutoGroups.GetHeadPosition (); 
		pos != NULL && pParentAutoGroup == NULL; )
	{
		CBCGPGridRow* pAutoGroup = lstOldAutoGroups.GetNext (pos);
		ASSERT_VALID (pAutoGroup);

		for (POSITION posSub = pAutoGroup->m_lstSubItems.GetHeadPosition (); 
			posSub != NULL && pParentAutoGroup == NULL;)
		{
			CBCGPGridRow* pItem = pAutoGroup->m_lstSubItems.GetNext (posSub);
			ASSERT_VALID (pItem);
			
			if (pGroupedItem == pItem)
			{
				pParentAutoGroup = pAutoGroup;
			}
		}
	}

	// Find parent for specified level:
	if (pParentAutoGroup != NULL)
	{
		int nLevelCount = iLevel;
		while (nLevelCount > 0)
		{
			CBCGPGridRow* pGroup = pParentAutoGroup->m_pParent;
			BOOL bIsAutoGroup = pGroup != NULL && pGroup->IsGroup () && (pGroup->m_dwFlags & BCGP_GRID_ITEM_AUTOGROUP) != 0;
			if (bIsAutoGroup)
			{
				pParentAutoGroup = pGroup;
				ASSERT_VALID (pParentAutoGroup);
			}
			else // can't find parent group at iLevel
			{
				return TRUE;
			}

			nLevelCount--;
		}

		return pParentAutoGroup->m_bExpanded;
	}

	return TRUE;
}
//****************************************************************************************
BOOL CBCGPGridRow::IsSubItem (CBCGPGridRow* pSubItem) const
{
	ASSERT_VALID (this);
	ASSERT_VALID (pSubItem);

	for (POSITION pos = m_lstSubItems.GetHeadPosition (); pos != NULL;)
	{
		CBCGPGridRow* pItem = m_lstSubItems.GetNext (pos);
		ASSERT_VALID (pItem);

		if (pSubItem == pItem || pItem->IsSubItem (pSubItem))
		{
			return TRUE;
		}
	}

	return FALSE;
}
//******************************************************************************************
CBCGPGridRow* CBCGPGridRow::FindSubItemByData (DWORD_PTR dwData) const
{
	ASSERT_VALID (this);
	
	for (POSITION pos = m_lstSubItems.GetHeadPosition (); pos != NULL;)
	{
		CBCGPGridRow* pItem = m_lstSubItems.GetNext (pos);
        ASSERT_VALID (pItem);
		
        if (pItem->m_dwData == dwData)
        {
			return pItem;
		}
		
		pItem = pItem->FindSubItemByData (dwData);
		
		if (pItem != NULL)
		{
			return pItem;
        }
    }
	
	return NULL;
}
//*****************************************************************************************
CBCGPGridRow* CBCGPGridRow::FindSubItemById (int nIndex) const
{
	ASSERT_VALID (this);
	
	for (POSITION pos = m_lstSubItems.GetHeadPosition (); pos != NULL;)
	{
		CBCGPGridRow* pItem = m_lstSubItems.GetNext (pos);
        ASSERT_VALID (pItem);
		
        if (pItem->m_nIdRow == nIndex)
        {
			return pItem;
		}
		
		pItem = pItem->FindSubItemById (nIndex);
		
		if (pItem != NULL)
		{
			return pItem;
        }
    }
	
	return NULL;
}
//*****************************************************************************************
CString CBCGPGridRow::FormatItem ()
{
	ASSERT_VALID (this);

	CString strVal;

	if (HasValueField ())
	{
		for (int i = 0; i < m_arrRowItems.GetSize (); i++)
		{
			CBCGPGridItem* pItem = m_arrRowItems [i];
			ASSERT_VALID (pItem);

			strVal += pItem->FormatItem ();

			if (i < m_arrRowItems.GetUpperBound ())
			{
				strVal += m_pWndList->m_cListDelimeter;
				strVal += _T(' ');
			}
		}
		
		if (m_arrRowItems.GetSize () > 0)
		{
			return strVal;
		}
	}

	strVal = _T("There is no items in this row.");
	return strVal;
}
//****************************************************************************************
CString CBCGPGridRow::GetName ()
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pWndList);

	BOOL bIsAutoGroup = IsGroup () && (m_dwFlags & BCGP_GRID_ITEM_AUTOGROUP) != 0;

	CString strName;
	if (!bIsAutoGroup || !m_pWndList->GetRowName (this, strName))
	{
		if (IsGroup ())
		{
			strName = _T("Group");//m_strName;
		}
	}

	return strName;
}
//****************************************************************************************
void CBCGPGridRow::OnDrawName (CDC* pDC, CRect rect)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);
	ASSERT_VALID (m_pWndList);

	COLORREF clrTextOld = (COLORREF)-1;

	BOOL bIsAutoGroup = IsGroup () && (m_dwFlags & BCGP_GRID_ITEM_AUTOGROUP) != 0;
	
	if (!HasValueField () || bIsAutoGroup)
	{
		COLORREF clrText = m_pWndList->GetGroupTextColor ();

		if (!m_pWndList->m_bHighlightGroups && IsSelected ())
		{
			CRect rectFill = rect;
			rectFill.top++;
			BOOL bGroupUnderline = !m_pWndList->m_bHighlightGroups && !HasValueField ();
			if (bGroupUnderline && IsGroup ())
			{
				rectFill.DeflateRect (0, 0, 0, 1);
			}
			
#ifndef _BCGSUITE_
	COLORREF clrHighlight = globalData.clrHotText;
#else
	COLORREF clrHighlight = globalData.clrHotLinkNormalText;
#endif

			COLORREF clrTextDefault = !m_pWndList->IsFocused () ?
					clrHighlight : globalData.clrTextHilite;

			if (!m_pWndList->IsFocused () && m_pWndList->m_ColorData.m_GroupSelColorsInactive.m_clrBackground != (COLORREF)-1)
			{
				m_pWndList->m_ColorData.m_GroupSelColorsInactive.Draw (pDC, rectFill);

				if (m_pWndList->m_ColorData.m_GroupSelColorsInactive.m_clrText != (COLORREF)-1)
				{
					clrText = m_pWndList->m_ColorData.m_GroupSelColorsInactive.m_clrText;
				}
				else
				{
					clrText = clrTextDefault;
				}
			}
			else
			{
				if (!m_pWndList->m_ColorData.m_GroupSelColors.Draw (pDC, rectFill))
				{
					clrTextDefault = visualManager->OnFillGridRowBackground (
						m_pWndList, pDC, rectFill, IsSelected ());
				}
			
				if (m_pWndList->m_ColorData.m_GroupSelColors.m_clrText != (COLORREF)-1)
				{
					clrText = m_pWndList->m_ColorData.m_GroupSelColors.m_clrText;
				}
				else
				{
					clrText = clrTextDefault;
				}
			}
		}

		if (clrTextOld == (COLORREF)-1)
		{
			clrTextOld = pDC->SetTextColor (clrText);
		}

		rect.DeflateRect (TEXT_MARGIN, 0);
		
		CString strName = GetName ();
		
		int nTextHeight = pDC->DrawText (strName, rect, 
			DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX | DT_END_ELLIPSIS);

		if (m_pWndList->m_bHighlightGroups && IsSelected ())
		{
			CRect rectFocus = rect;
			rectFocus.top = rectFocus.CenterPoint ().y - nTextHeight / 2;
			rectFocus.bottom = rectFocus.top + nTextHeight;
			rectFocus.right = rect.right;
			rectFocus.right = 
				min (rect.right, rectFocus.left + pDC->GetTextExtent (strName).cx);
			rectFocus.InflateRect (2, 0);

			COLORREF clrShadow = 
				(m_pWndList->m_ColorData.m_GroupColors.m_clrText != (COLORREF)-1) ?
				m_pWndList->m_ColorData.m_GroupColors.m_clrText :
				(m_pWndList->m_bControlBarColors ?
				globalData.clrBarShadow : globalData.clrBtnShadow);

			pDC->Draw3dRect (rectFocus, clrShadow, clrShadow);
		}

		m_bNameIsTrancated = pDC->GetTextExtent (strName).cx > rect.Width ();
	}
	else
	{
		if (m_pWndList->IsPreviewRowEnabled ())
		{
			OnDrawPreview (pDC, rect);
		}
	}

	if (clrTextOld != (COLORREF)-1)
	{
		pDC->SetTextColor (clrTextOld);
	}

}
//******************************************************************************************
void CBCGPGridRow::OnDrawPreview (CDC* /*pDC*/, CRect /*rect*/)
{
}
//******************************************************************************************
void CBCGPGridRow::OnDrawItems (CDC* pDC, CRect rect)
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pWndList);

	rect.NormalizeRect ();
	const int nFreezeOffset = m_pWndList->m_rectList.left + m_pWndList->GetColumnsInfo ().GetFreezeOffset ();

	for (int i = 0; i < m_arrRowItems.GetSize (); i++)
	{
		CBCGPGridItem* pItem = m_arrRowItems [i];
		ASSERT_VALID (pItem);

		CRect rectValue = pItem->GetRect ();
		if (!rectValue.IsRectEmpty ())
		{
			if (pItem->GetMergedCells () != NULL)
			{
				// Item is a part of merged item - Draw item later.
				m_pWndList->RedrawMergedItem (pItem);
				continue;
			}

			rectValue.NormalizeRect ();

			CRect rectClipItem = rectValue;

			// frozen columns:
			if (m_pWndList->GetColumnsInfo ().IsFreezeColumnsEnabled () &&
				!m_pWndList->GetColumnsInfo ().IsColumnFrozen (i))
			{
				// Do not allow unfrozen columns to draw inside the frozen area
				rectClipItem.left = max (nFreezeOffset, rectClipItem.left);
				if (rectClipItem.left >= rectClipItem.right)
				{
					continue;
				}
			}

			rectClipItem.IntersectRect (rectClipItem, rect);

			CRect rectBordersSize (0, 0, 0, 0);
			if (m_pWndList->m_bGridItemBorders)
			{
				pItem->OnGetBorders (rectBordersSize);

				if (rectBordersSize.top == 0 && m_pWndList->GetLeftItemBorderOffset () > rectValue.left)
				{
					rectBordersSize.top ++;
				}

				rectClipItem.InflateRect (rectBordersSize);
			}

			m_pWndList->m_rgnClipItem.CreateRectRgnIndirect (&rectClipItem);
			pDC->SelectClipRgn (&m_pWndList->m_rgnClipItem);

			pItem->OnDrawValue (pDC, rectValue);

			if (m_pWndList->m_bGridItemBorders)
			{
				pItem->OnDrawBorders (pDC, rectValue);
			}

			if (!pItem->m_rectButton.IsRectEmpty ())
			{
				pItem->OnDrawButton (pDC, pItem->m_rectButton);
			}

			pDC->SelectClipRgn (&m_pWndList->m_rgnClipRow);
			m_pWndList->m_rgnClipItem.DeleteObject ();
		}
	}
}
//******************************************************************************************
void CBCGPGridRow::OnPrintName (CDC* pDC, CRect rect)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);

	if (!HasValueField ())
	{
		ASSERT_VALID (m_pWndList);
		ASSERT (m_pWndList->m_bIsPrinting);

		// map to printer metrics
		HDC hDCFrom = ::GetDC(NULL);
		int nXMul = pDC->GetDeviceCaps(LOGPIXELSX);			// pixels in print dc
		int nXDiv = ::GetDeviceCaps(hDCFrom, LOGPIXELSX);	// pixels in screen dc
		::ReleaseDC(NULL, hDCFrom);

		const int CALCULATED_TEXT_MARGIN = ::MulDiv (TEXT_MARGIN, nXMul, nXDiv);
		const CRect& rectClip = m_pWndList->m_PrintParams.m_pageInfo.m_rectPageItems;

		CRect rectText = rect;
		rectText.DeflateRect (CALCULATED_TEXT_MARGIN, 0);
		CString strName = GetName ();

		CRect rectClipText = rectText;
		rectClipText.NormalizeRect ();
		if (rectClipText.IntersectRect (&rectClipText, &rectClip))
		{
			COLORREF clrTextOld = pDC->SetTextColor (m_pWndList->m_clrPrintText);

			// Draw text vertically centered
			ASSERT_VALID (m_pWndList->m_pPrintDC);

			TEXTMETRIC tm;
			m_pWndList->m_pPrintDC->GetTextMetrics (&tm);
			int nDescent = tm.tmDescent;
			int nVCenterOffset = (rectText.Height () - pDC->GetTextExtent (strName).cy + nDescent) / 2;

			pDC->SetTextAlign (TA_LEFT | TA_TOP);
			pDC->ExtTextOut (rectText.left, rectText.top + nVCenterOffset, ETO_CLIPPED, &rectClipText, strName, NULL);

			pDC->SetTextColor (clrTextOld);
		}
	}
}
//******************************************************************************************
void CBCGPGridRow::OnPrintItems (CDC* pDC, CRect /*rectItems*/)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);
	
	ASSERT_VALID (m_pWndList);
	const CRect rectClipPage = m_pWndList->m_PrintParams.m_pageInfo.m_rectPageItems;
	const int nFirstColumn = m_pWndList->m_PrintParams.m_pageInfo.m_nFirstColumnInPage;
	const int nLastColumn = m_pWndList->m_PrintParams.m_pageInfo.m_nLastColumnInPage;
	
	int nPos = m_pWndList->GetColumnsInfo ().Begin ();
	for (int nCount = 0; nPos != m_pWndList->GetColumnsInfo ().End (); nCount++)
	{
		int iColumn = m_pWndList->GetColumnsInfo ().Next (nPos);
		if (iColumn == -1)
		{
			break; // no more visible columns
		}
		
		if (nCount < nFirstColumn)
		{
			continue;
		}
		
		if (nCount > nLastColumn ||
			iColumn > m_arrRowItems.GetUpperBound ())
		{
			break;
		}
		
		CBCGPGridItem* pItem = m_arrRowItems [iColumn];
		ASSERT_VALID (pItem);
		
		CRect rectClipRect = pItem->m_Rect;
		if (!rectClipRect.IsRectEmpty ())
		{
			if (pItem->GetMergedCells () != NULL)
			{
				// Item is a part of merged item - Draw item later.
				m_pWndList->RedrawMergedItem (pItem);
				continue;
			}

			rectClipRect.NormalizeRect ();
			if (rectClipRect.IntersectRect (&rectClipRect, &rectClipPage))
			{
				pItem->OnPrintValue (pDC, pItem->m_Rect);
				pItem->OnPrintBorders (pDC, pItem->m_Rect);
			}
		}
	}
}
//****************************************************************************************
void CBCGPGridRow::OnDrawExpandBox (CDC* pDC, CRect rect)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);
	ASSERT_VALID (m_pWndList);
	ASSERT (IsGroup ());

	BOOL bIsOpened = m_bExpanded && !m_lstSubItems.IsEmpty ();

	if (m_pWndList->m_bIsPrinting)
	{
		// map to printer metrics
		ASSERT_VALID (m_pWndList->m_pPrintDC);
		CSize szOne = m_pWndList->m_PrintParams.m_pageInfo.m_szOne;

		CPoint ptCenter = rect.CenterPoint ();
		// align to integral logical point
		ptCenter.Offset (-(ptCenter.x % szOne.cx), -(ptCenter.y % szOne.cy));

		int nMinSize = (szOne.cx != 0 && szOne.cy != 0) ?
			min (rect.Width () / szOne.cx, rect.Height () / szOne.cy) : 0;
		int nBoxSize = min (9, nMinSize); // in logical units

		rect = CRect (ptCenter, szOne);
		rect.InflateRect ((nBoxSize / 2) * szOne.cx, (nBoxSize / 2) * szOne.cy);

		CPen penLine (PS_SOLID, 1, m_pWndList->m_clrPrintText);
		CPen* pOldPen = pDC->SelectObject (&penLine);

		pDC->MoveTo (rect.TopLeft ());
		pDC->LineTo (rect.right, rect.top);
		pDC->LineTo (rect.BottomRight ());
		pDC->LineTo (rect.left, rect.bottom);
		pDC->LineTo (rect.TopLeft ());

		rect.DeflateRect (2 * szOne.cx, 2 * szOne.cy);

		pDC->MoveTo (rect.left, ptCenter.y + szOne.cy / 2);
		pDC->LineTo (rect.right, ptCenter.y + szOne.cy / 2);

		if (!bIsOpened)
		{
			pDC->MoveTo (ptCenter.x + szOne.cx / 2, rect.top);
			pDC->LineTo (ptCenter.x + szOne.cx / 2, rect.bottom);
		}

		pDC->SelectObject (pOldPen);
	}
	else
	{
		CPoint ptCenter = rect.CenterPoint ();

		int nMaxBoxSize = 9;
		if (globalData.GetRibbonImageScale () != 1.)
		{
			nMaxBoxSize = (int)(.5 + nMaxBoxSize * globalData.GetRibbonImageScale ());
		}

		int nBoxSize = min (nMaxBoxSize, rect.Width ());

		rect = CRect (ptCenter, CSize (1, 1));
		rect.InflateRect (nBoxSize / 2, nBoxSize / 2);

		COLORREF clrText = globalData.clrBtnText;

		if (m_pWndList->m_ColorData.m_GroupColors.m_clrBackground != (COLORREF)-1 &&
			m_pWndList->m_ColorData.m_GroupColors.m_clrText != (COLORREF)-1)
		{
			clrText = m_pWndList->m_ColorData.m_GroupColors.m_clrText;
		}

		if (m_pWndList->m_bControlBarColors)
		{
			visualManager->OnDrawGridExpandingBox (pDC, rect, 
				bIsOpened, clrText);
		}
		else
		{
			visualManagerMFC->OnDrawExpandingBox (pDC, rect,
				bIsOpened, clrText);
		}
	}
}
//******************************************************************************************
void CBCGPGridRow::OnDrawRowMarker (CDC* pDC, CRect rect)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);
	ASSERT_VALID (m_pWndList);
	ASSERT (!IsGroup () || m_pWndList->IsRowMarkerOnRowHeader ());

	if (m_pWndList->m_bIsPrinting)
	{
		return;
	}

	CPoint ptCenter = rect.CenterPoint ();

	int nBoxSize = min (9, rect.Width ());

	rect = CRect (ptCenter, CSize (1, 1));
	rect.InflateRect (nBoxSize / 2, nBoxSize / 2);

	BOOL bRTL = (m_pWndList->GetExStyle () & WS_EX_LAYOUTRTL);

#ifndef _BCGPGRID_STANDALONE
	COLORREF clr = globalData.clrWindow;

	if (m_pWndList->m_ColorData.m_LeftOffsetColors.m_clrBackground != (COLORREF)-1)
	{
		clr = m_pWndList->m_ColorData.m_LeftOffsetColors.m_clrBackground;
	}

	CBCGPMenuImages::DrawByColor(pDC, bRTL ? CBCGPMenuImages::IdArowLeftTab3d : CBCGPMenuImages::IdArowRightTab3d, rect.TopLeft (), clr);
#else
	visualManager->OnDrawRowMarker (pDC, rect);
#endif
}
//******************************************************************************************
BOOL CBCGPGridRow::OnUpdateValue ()
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pWndList);

	if (m_bInPlaceEdit)
	{
		// ---------------------------------------
		// Find inplace edited item and update it:
		// ---------------------------------------
		const CArray<CBCGPGridItem*, CBCGPGridItem*>& arr = m_arrRowItems;

		for (int i = 0; i < arr.GetSize (); i++)
		{
			CBCGPGridItem* pItem = (CBCGPGridItem*) arr [i];
			ASSERT_VALID (pItem);

			if (pItem->m_pWndInPlace != NULL)
			{
				ASSERT_VALID (pItem->m_pWndInPlace);
				return pItem->OnUpdateValue ();
			}
		}
	}
	else
	{
		CBCGPGridItem* pSelItem = m_pWndList->GetCurSelItem (this);
		if (pSelItem != NULL)
		{
			CBCGPGridItemID id = pSelItem->GetGridItemID ();
			m_pWndList->OnItemChanged (pSelItem, id.m_nRow, id.m_nColumn);
		}
	}

	return TRUE;
}
//*****************************************************************************
void CBCGPGridRow::OnItemChanged (CBCGPGridItem* pItem, int nRow, int nColumn)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pItem);
	ASSERT_VALID (m_pWndList);

	m_pWndList->OnItemChanged (pItem, nRow, nColumn);
}
//*****************************************************************************
BOOL CBCGPGridRow::IsItemVisible () const
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pWndList);

	BOOL bIsVisible = IsParentExpanded ();

	if (bIsVisible)
	{
		return !IsItemFiltered (); // visible if not filtered
	}

	return bIsVisible;
}
//*****************************************************************************
BOOL CBCGPGridRow::OnEdit (LPPOINT lptClick)
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pWndList);

	if (!HasValueField () || !m_pWndList->AllowInPlaceEdit ())
	{
		return FALSE;
	}

	if (lptClick == NULL)
	{
		CBCGPGridItem* pItem = m_pWndList->GetCurSelItem (this);
		if (pItem == NULL)
		{
			return FALSE;
		}

		_variant_t varValue = pItem->GetValue();
		if (varValue.vt == VT_BOOL)
		{
			return TRUE;
		}

		if (!pItem->IsAllowEdit ())
		{
			return FALSE;
		}

		m_bInPlaceEdit = pItem->OnEdit (lptClick);
		return m_bInPlaceEdit;
	}

	for (int i = 0; i < m_arrRowItems.GetSize (); i++)
	{
		CBCGPGridItem* pItem = m_arrRowItems [i];
		ASSERT_VALID (pItem);

		if (pItem->m_Rect.PtInRect (*lptClick) && pItem->m_bEnabled)
		{
			m_bInPlaceEdit = pItem->OnEdit (lptClick);

			if (m_bInPlaceEdit)
			{
				break;
			}
		}
	}

	return m_bInPlaceEdit;
}
//*****************************************************************************
BOOL CBCGPGridRow::OnEndEdit ()
{
	ASSERT_VALID (this);

	for (int i = 0; i < m_arrRowItems.GetSize () && m_bInPlaceEdit; i++)
	{
		CBCGPGridItem* pItem = m_arrRowItems [i];
		ASSERT_VALID (pItem);

		if (pItem->m_bInPlaceEdit)
		{
			pItem->OnEndEdit ();
		}
	}

	m_bInPlaceEdit = FALSE;
	return TRUE;
}
//****************************************************************************************
BOOL CBCGPGridRow::OnKillFocus (CWnd* pNewWnd)
{
	ASSERT_VALID (m_pWndList);
	
	CBCGPGridItem* pItem = m_pWndList->GetCurSelItem (this);
	if (pItem != NULL)
	{
		return pItem->OnKillFocus (pNewWnd);
	}

	return TRUE;
}
//****************************************************************************************
BOOL CBCGPGridRow::OnEditKillFocus ()
{
	ASSERT_VALID (m_pWndList);

	CBCGPGridItem* pItem = m_pWndList->GetCurSelItem (this);
	if (pItem != NULL && pItem->m_bInPlaceEdit && pItem->m_pWndInPlace != NULL)
	{
		return pItem->OnEditKillFocus ();
	}

	return TRUE;
}
//****************************************************************************************
BOOL CBCGPGridRow::OnClickValue (UINT uiMsg, CPoint point)
{
	ASSERT_VALID (this);

	if (!m_bInPlaceEdit)
	{
		return FALSE;
	}

	for (int i = 0; i < m_arrRowItems.GetSize (); i++)
	{
		CBCGPGridItem* pItem = m_arrRowItems [i];
		ASSERT_VALID (pItem);

		if (pItem->m_Rect.PtInRect (point))
		{
			return pItem->OnClickValue (uiMsg, point);
		}
	}

	return FALSE;
}
//****************************************************************************************
BOOL CBCGPGridRow::OnDblClick (CPoint point)
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pWndList);

	CBCGPGridItem* pClickedItem = NULL;

	for (int i = 0; i < m_arrRowItems.GetSize (); i++)
	{
		CBCGPGridItem* pItem = m_arrRowItems [i];
		ASSERT_VALID (pItem);

		if (pItem->m_Rect.PtInRect (point))
		{
			pClickedItem = pItem;
			break;
		}
	}

	if (m_pWndList->GetOwner ()->SendMessage (
			BCGM_GRID_ITEM_DBLCLICK, 0, (LPARAM) pClickedItem) != 0)
	{
		return TRUE;
	}

	if (pClickedItem != NULL)
	{
		return pClickedItem->OnDblClick (point);
	}

	return FALSE;
}
//****************************************************************************************
void CBCGPGridRow::OnRClickValue (CPoint point, BOOL bSelChanged)
{
	ASSERT_VALID (this);

	for (int i = 0; i < m_arrRowItems.GetSize () && !m_bInPlaceEdit; i++)
	{
		CBCGPGridItem* pItem = m_arrRowItems [i];
		ASSERT_VALID (pItem);

		if (pItem->m_Rect.PtInRect (point))
		{
			pItem->OnRClickValue (point, bSelChanged);
			break;;
		}
	}
}
//*****************************************************************************************
void CBCGPGridRow::OnSelectCombo ()
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pWndList);

	CBCGPGridItem* pItem = m_pWndList->GetCurSelItem (this);
	if (pItem != NULL && pItem->m_bInPlaceEdit && pItem->m_pWndCombo != NULL)
	{
		pItem->OnSelectCombo ();
	}
}
//****************************************************************************************
void CBCGPGridRow::OnCloseCombo ()
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pWndList);

	CBCGPGridItem* pItem = m_pWndList->GetCurSelItem (this);
	if (pItem != NULL && pItem->m_bInPlaceEdit)
	{
		pItem->OnCloseCombo ();
	}
}
//****************************************************************************************
BOOL CBCGPGridRow::PushChar (UINT nChar)
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pWndList);

	CBCGPGridItem* pItem = m_pWndList->GetCurSelItem (this);
	if (pItem != NULL)
	{
		ASSERT_VALID (pItem);

		if (pItem->m_pGridRow == this)
		{
			return pItem->PushChar (nChar);
		}
	}

	return FALSE;
}
//*******************************************************************************************
CString CBCGPGridRow::GetNameTooltip ()
{
	ASSERT_VALID (this);
	return m_bNameIsTrancated ? GetName () : _T("");
}
//*******************************************************************************************
CRect CBCGPGridRow::GetNameTooltipRect ()
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pWndList);

	if (!m_Rect.IsRectEmpty ())
	{
		CRect rectName = GetRect ();

		// --------------------
		// space for expandbox:
		// --------------------
		if (IsGroup () && (!m_pWndList->IsSortingMode () || m_pWndList->IsGrouping ()) &&
			!m_lstSubItems.IsEmpty ())
		{
			int dx = GetHierarchyLevel () * m_pWndList->GetHierarchyLevelOffset ();
			int nLeftMargin = max (m_pWndList->GetExtraHierarchyOffset (), m_pWndList->GetButtonWidth ());
			rectName.left += nLeftMargin + dx;
		}

		if (rectName.right > rectName.left)
		{
			return rectName;
		}
	}

	CRect rect (0, 0, 0, 0);
	return rect;
}
//*******************************************************************************************
HBRUSH CBCGPGridRow::OnCtlColor(CDC* pDC, UINT nCtlColor)
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pWndList);

	if (m_pWndList->m_ColorData.m_clrText != (COLORREF)-1)
	{
		pDC->SetTextColor (m_pWndList->m_ColorData.m_clrText);
	}

	if (IsGroup () && m_pWndList->m_brGroupBackground.GetSafeHandle () != NULL)
	{
		if (m_pWndList->m_ColorData.m_GroupColors.m_clrBackground != -1)
		{
			pDC->SetBkColor (m_pWndList->m_ColorData.m_GroupColors.m_clrBackground);
		}

		return (HBRUSH) m_pWndList->m_brGroupBackground.GetSafeHandle ();
	}
	else if (m_pWndList->m_brBackground.GetSafeHandle () != NULL)
	{
		if (m_pWndList->m_ColorData.m_clrBackground != -1)
		{
			pDC->SetBkColor (m_pWndList->m_ColorData.m_clrBackground);
		}

		return (HBRUSH) m_pWndList->m_brBackground.GetSafeHandle ();
	}

	// Use the first cell in the row
	if (m_arrRowItems.GetSize () > 0)
	{
		CBCGPGridItem* pSelItem = m_arrRowItems [0];
		ASSERT_VALID (pSelItem);

		return pSelItem->OnCtlColor(pDC, nCtlColor);
	}

	return NULL;
}
//*******************************************************************************************
void CBCGPGridRow::GetPreviewText (CString& str) const
{
	str.Empty ();
}
//*******************************************************************************************
void CBCGPGridRow::OnMeasureGridRowRect (CRect& /*rect*/)
{
}
//*******************************************************************************************
void CBCGPGridRow::OnMeasureGridItemRect (CRect& /*rect*/, CBCGPGridItem* /*pItem*/)
{
}
//*******************************************************************************************
void CBCGPGridRow::Serialize (CArchive& ar)
{
	if (ar.IsStoring ())
	{
		WriteToArchive (ar);
	}
	else
	{
		ReadFromArchive (ar, FALSE);
	}
}
//*******************************************************************************************
void CBCGPGridRow::ReadFromArchive(CArchive& ar, BOOL bTestMode)
{
	ASSERT_VALID (this);
	
	DWORD_PTR		dwData;		// User-defined data
	ar >> dwData;

	BOOL			bGroup;		// Is item group?
	BOOL			bEnabled;	// Is item enabled?
	BOOL			bAllowEdit;	// Is item editable?
	DWORD			dwFlags;	// Item flags
	int				nLinesNumber = 1;
	
	ar >> bGroup;
	ar >> bEnabled;
	ar >> bAllowEdit;
	ar >> dwFlags;

#ifndef _BCGSUITE_
	if ((g_pWorkspace != NULL && g_pWorkspace->GetDataVersion () > 0x18000))
	{
		ar >> nLinesNumber;
	}
#else
	ar >> nLinesNumber;
#endif

	if (!bTestMode)
	{
		m_dwData = dwData;
		m_bGroup = bGroup;
		m_bEnabled = bEnabled;
		m_bAllowEdit = bAllowEdit;
		m_dwFlags = dwFlags;
		m_nLines = nLinesNumber;
	}
}
//*******************************************************************************************
void CBCGPGridRow::WriteToArchive(CArchive& ar)
{
	ASSERT_VALID (this);
	
	ar << (DWORD_PTR)m_dwData;	// User-defined data
	ar << m_bGroup;			// Is item group?
	ar << m_bEnabled;		// Is item enabled?
	ar << m_bAllowEdit;		// Is item editable?
	ar << m_dwFlags;		// Item flags
	ar << m_nLines;
}
//*******************************************************************************************
BOOL CBCGPGridRow::SetACCData (CWnd* pParent, CBCGPAccessibilityData& data)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pParent);

	data.Clear ();

	data.m_strAccName = GetName ();
	data.m_strDescription = GetName ();

	BOOL bHasItems = HasValueField () && GetItemCount () > 0;
	if (IsGroup() || !bHasItems)
	{
		data.m_strAccValue = GetName();
	}
	else
	{
		data.m_strAccValue = FormatItem();
	}

	
	data.m_nAccHit = 1;
	data.m_nAccRole = ROLE_SYSTEM_ROW;

	data.m_bAccState = STATE_SYSTEM_FOCUSABLE|STATE_SYSTEM_SELECTABLE;

	if (IsSelected ())
	{
		data.m_bAccState |= STATE_SYSTEM_FOCUSED;
		data.m_bAccState |= STATE_SYSTEM_SELECTED;	
	}

	if (IsGroup())
	{
		if (IsExpanded ())
		{
			data.m_bAccState |= STATE_SYSTEM_EXPANDED;
		}
		else
		{
			data.m_bAccState |= STATE_SYSTEM_COLLAPSED;
		}
	}

	if (!IsEnabled () || (IsGroup() && !HasValueField ()))
	{
		data.m_bAccState |= STATE_SYSTEM_READONLY;
	}
	
	data.m_rectAccLocation = m_Rect;
	pParent->ClientToScreen (&data.m_rectAccLocation);

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPGridCaptionRow

IMPLEMENT_SERIAL(CBCGPGridCaptionRow, CBCGPGridRow, VERSIONABLE_SCHEMA | 1)

CBCGPGridCaptionRow::CBCGPGridCaptionRow () : CBCGPGridRow (_T(""))
{
	m_bGroup = FALSE;
}
//*******************************************************************************************
CBCGPGridCaptionRow::CBCGPGridCaptionRow (const CString& strCaption, DWORD_PTR dwData)
 : CBCGPGridRow (strCaption, dwData)
{
	m_bGroup = FALSE;
	m_strCaption = strCaption;
}
//*******************************************************************************************
void CBCGPGridCaptionRow::SetCaption (const CString& strCaption, BOOL bRedraw)
{
    m_strCaption = strCaption;

	if (bRedraw)
	{
		Redraw ();
	}
}
//*******************************************************************************************
void CBCGPGridCaptionRow::OnDrawName (CDC* pDC, CRect rect)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);
	ASSERT_VALID (m_pWndList);

	COLORREF clrText = visualManager->OnFillGridCaptionRow (m_pWndList, pDC, rect);

	rect.DeflateRect (TEXT_MARGIN, 0);

	CString strCaption = GetCaption ();

	COLORREF clrTextOld = pDC->SetTextColor (clrText);

	CFont* pOldFont = pDC->SelectObject(&m_pWndList->GetBoldFont());
	ASSERT_VALID(pOldFont);
	
	pDC->DrawText (strCaption, rect, 
		DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX | DT_END_ELLIPSIS);

	pDC->SetTextColor (clrTextOld);
	pDC->SelectObject(pOldFont);

	m_bNameIsTrancated = pDC->GetTextExtent (strCaption).cx > rect.Width ();
}
//*******************************************************************************************
void CBCGPGridCaptionRow::OnPrintName (CDC* pDC, CRect rect)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);
	ASSERT_VALID (m_pWndList);

	// map to printer metrics
	HDC hDCFrom = ::GetDC(NULL);
	int nXMul = pDC->GetDeviceCaps(LOGPIXELSX);			// pixels in print dc
	int nXDiv = ::GetDeviceCaps(hDCFrom, LOGPIXELSX);	// pixels in screen dc
	::ReleaseDC(NULL, hDCFrom);

	const int CALCULATED_TEXT_MARGIN = ::MulDiv (TEXT_MARGIN, nXMul, nXDiv);
	const CRect& rectClip = m_pWndList->GetPrintParams().m_pageInfo.m_rectPageItems;

	CRect rectText = rect;
	rectText.DeflateRect (CALCULATED_TEXT_MARGIN, 0);
	CString strCaption = GetCaption ();

	CRect rectClipText = rectText;
	rectClipText.NormalizeRect ();
	if (rectClipText.IntersectRect (&rectClipText, &rectClip))
	{
 		COLORREF clrTextOld = pDC->SetTextColor (m_pWndList->m_clrPrintText);

		// Draw text vertically centered
		CDC* pPrintDC = m_pWndList->GetPrintDC ();
		ASSERT_VALID (pPrintDC);

		TEXTMETRIC tm;
		pPrintDC->GetTextMetrics (&tm);
		int nDescent = tm.tmDescent;
		int nVCenterOffset = (rectText.Height () - pDC->GetTextExtent (strCaption).cy + nDescent) / 2;

		pDC->SetTextAlign (TA_LEFT | TA_TOP);
		pDC->ExtTextOut (rectText.left, rectText.top + nVCenterOffset, ETO_CLIPPED, &rectClipText, strCaption, NULL);

 		pDC->SetTextColor (clrTextOld);
	}
}
//*******************************************************************************************
BOOL CBCGPGridCaptionRow::OnEdit (LPPOINT)
{
	return FALSE;
}
//*******************************************************************************************
BOOL CBCGPGridCaptionRow::PushChar (UINT)
{
	return FALSE;
}
//*******************************************************************************************
void CBCGPGridCaptionRow::ReadFromArchive(CArchive& ar, BOOL bTestMode)
{
	CBCGPGridRow::ReadFromArchive(ar, bTestMode);

	CString strCaption;
	ar >> strCaption;

	if (!bTestMode)
	{
		m_strCaption = strCaption;
	}
}
//*******************************************************************************************
void CBCGPGridCaptionRow::WriteToArchive(CArchive& ar)
{
	CBCGPGridRow::WriteToArchive(ar);

	ar << m_strCaption;
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPGridHeaderParams object

CBCGPGridHeaderParams::CBCGPGridHeaderParams ()
{
	m_nHeaderPart = CBCGPGridHeaderParams::HeaderTop;
	m_nItemState = CBCGPGridHeaderParams::Normal;
	m_nItemSelected = CBCGPGridHeaderParams::NotSelected;
	m_rect.SetRectEmpty ();
	m_rectInnerBorders.SetRectEmpty ();
	m_rectOuterBorders.SetRectEmpty ();
	m_nColumn = 0;
	m_pRow = NULL;
	m_dwData = 0;
}

CBCGPGridHeaderParams::~CBCGPGridHeaderParams ()
{
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPGridColumnsItem object

CBCGPGridColumnsItem::~CBCGPGridColumnsItem ()
{
	if (m_pFilterBarCtrl->GetSafeHwnd () != NULL)
	{
		m_pFilterBarCtrl->DestroyWindow ();
	}

	delete m_pFilterBarCtrl;
	m_pFilterBarCtrl = NULL;
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPGridColumnsInfo object

CBCGPGridColumnsInfo::CBCGPGridColumnsInfo () :
	m_bAutoSize (FALSE), m_nTotalWidth (0), m_bMultipleSort (FALSE),
	m_nFreezeColumns (-1), m_nFreezeOffset (0),
	m_bDrawingDraggedColumn (FALSE), m_nHighlightedItem (-1), m_nHighlightedItemBtn (-1), m_pWndList (NULL), m_bInvertPressedColumn (TRUE)
{
}
//*******************************************************************************************
CBCGPGridColumnsInfo::~CBCGPGridColumnsInfo ()
{
	DeleteAllColumns ();
}
//*******************************************************************************************
void CBCGPGridColumnsInfo::DrawColumn (CDC* pDC, int nCol, CRect rect,
									   int nTextMargin, int nArrowMargin,
									   BOOL bIsPrinting,
									   BOOL bNoSortArrow,
									   BOOL bIsGroupBox)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);
	ASSERT_VALID (m_pWndList);

	CRect rectColumn = rect;
	m_bInvertPressedColumn = TRUE;

	//-------------
	// Draw border:
	//-------------
	if (bIsPrinting)
	{
		CSize szOne = m_pWndList->m_PrintParams.m_pageInfo.m_szOne;
		CRect rectBorder = rect;
		rectBorder.bottom -= szOne.cy;

		pDC->MoveTo (rect.TopLeft ());
		pDC->LineTo (rect.right, rect.top);
		pDC->LineTo (rect.BottomRight ());
		pDC->MoveTo (rect.TopLeft ());
		pDC->LineTo (rect.left, rect.bottom);
		pDC->LineTo (rect.BottomRight ());
	}
	else
	{
		if (bIsGroupBox)
		{
			visualManager->OnDrawGridGroupByBoxItemBorder (m_pWndList, pDC, rect);
		}
		else
		{
			m_pWndList->OnDrawHeaderItemBorder (pDC, rect, nCol);
		}
	}

	if (nCol >= m_arrColumns.GetSize ())
	{
		return; // last
	}

	if (!bIsGroupBox && !bIsPrinting)
	{
		int bFirstVisibleColumn = FALSE;
		int nPos = Begin ();
		if (nPos != End ())
		{
			int nFirstColumn = Next (nPos);
			bFirstVisibleColumn = (nFirstColumn != -1 && nFirstColumn == nCol);
		}

		if (bFirstVisibleColumn)
		{
			int nHierarchyOffset = m_pWndList->GetHierarchyOffset () + 
				m_pWndList->GetExtraHierarchyOffset ();
			rect.DeflateRect (nHierarchyOffset, 0, 0, 0);
		}
	}

	BOOL bNotDrawButton = (nCol == m_pWndList->m_nDraggedColumn || bIsGroupBox);

	int nSortVal = GetColumnState (nCol);

	//------------
	// Draw image:
	//------------
	int nImage = GetColumnImage (nCol);

	if (nImage >= 0 && !(bIsGroupBox && 
		// ignore bIsGroupBox parameter when dragging from header
		(!m_bDrawingDraggedColumn ||
		m_pWndList->m_bDragGroupItem ||
		m_pWndList->m_bDragFromChooser) ) ) 
	{
		//---------------------------------------
		// The column has a image from imagelist:
		//---------------------------------------
		if (m_pWndList->m_pImagesHeader != NULL)
		{			
			int cx = 0;
			int cy = 0;

			VERIFY (::ImageList_GetIconSize (*m_pWndList->m_pImagesHeader, &cx, &cy));

			CPoint pt = rect.TopLeft ();
			pt.x ++;
			pt.y = (rect.top + rect.bottom - cy) / 2;

			int nArrow = 0; // width of the arrow or the button

			if (nSortVal != 0 && !bNoSortArrow)
			{
				if (rect.Width () >= m_pWndList->GetButtonWidth () + cx)
				{
					nArrow = m_pWndList->GetButtonWidth ();
				}
			}

			int nBtnWidth = m_pWndList->GetHeaderMenuButtonRect (rect, nCol).Width ();
			if (nBtnWidth > 0 && !bNotDrawButton)
			{
				nArrow = nBtnWidth;
			}

			if (rect.Width () > cx + nArrow)
			{
				if (!m_bDrawingDraggedColumn &&
					rect.Width () > cx)
				{
					int nAlign = GetHeaderAlign (nCol);

					if (nAlign & HDF_CENTER)
					{
						pt.x += (rect.Width () - nArrow - cx) / 2;
					}
					else if (nAlign & HDF_RIGHT)
					{
						pt.x = rect.right - nArrow - cx - 1;
					}

					rect.left = pt.x;
				}

				VERIFY (m_pWndList->m_pImagesHeader->Draw (pDC, nImage, pt, ILD_NORMAL));

				rect.left += cx;
			}
		}
	}

	if (m_pWndList->IsHeaderMenuButtonEnabled (nCol) && !bNotDrawButton)
	{
		CRect rectBtn = m_pWndList->GetHeaderMenuButtonRect (rect, nCol);
		m_pWndList->OnDrawHeaderMenuButton (pDC, rectBtn, nCol, !bNoSortArrow);
		rect.right = rectBtn.left - 1;
	}
	else if (nSortVal != 0 && !bNoSortArrow)
	{
		//-----------------
		// Draw sort arrow:
		//-----------------
		int nArrowHeight = m_pWndList->GetButtonWidth () - 2 * nArrowMargin;

		CRect rectArrow = rect;
		rectArrow.DeflateRect (nArrowMargin, nArrowMargin);
		if (rectArrow.Width () >= nArrowHeight)
		{
			rectArrow.left = rectArrow.right - nArrowHeight;
			rect.right = rectArrow.left - 1;

			int dy2 = (int) (.134 * rectArrow.Width ());
			rectArrow.DeflateRect (0, dy2);

			m_pWndList->OnDrawSortArrow (pDC, rectArrow, nSortVal > 0);
		}
	}

	CRect rectLabel = rect;
	rectLabel.DeflateRect (nTextMargin, 0);

	//-----------
	// Draw text:
	//-----------
	if (bIsGroupBox || !GetColumnTextHidden (nCol))
	{
		COLORREF clrText = (m_pWndList->IsColumnSelected (nCol) && m_pWndList->m_ColorData.m_HeaderSelColors.m_clrText != -1)
								? m_pWndList->m_ColorData.m_HeaderSelColors.m_clrText
								: m_pWndList->m_ColorData.m_HeaderColors.m_clrText;
		COLORREF clrTextOld = (COLORREF)-1;
		if (clrText != (COLORREF)-1 && !bIsPrinting && !bIsGroupBox)
		{
			clrTextOld = pDC->SetTextColor (clrText);
		}

		CRect rectLabel = rect;
		rectLabel.DeflateRect (nTextMargin, 0);

		CString strLabel = GetColumnName (nCol);
		int nTextAlign = GetHeaderAlign (nCol);

		UINT uiTextFlags = DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX;

		if (!bIsGroupBox)	// ignore align flags in groupbox
		{
			if (nTextAlign & HDF_CENTER)
			{
				uiTextFlags |= DT_CENTER;
			}
			else if (nTextAlign & HDF_RIGHT)
			{
				uiTextFlags |= DT_RIGHT;
			}
		}

		if (m_pWndList->GetColumnsInfo ().GetHeaderMultiLine (nCol) && !bIsGroupBox)
		{
			uiTextFlags |= DT_WORDBREAK;
		}
		else
		{
			uiTextFlags |= DT_SINGLELINE;
		}

		BOOL bTrancated = m_pWndList->DoDrawText (pDC, strLabel, rectLabel, uiTextFlags);

		if (clrTextOld != (COLORREF)-1)
		{
			pDC->SetTextColor (clrTextOld);
		}

		SetColumnNameTrancated (nCol, bTrancated);
	}

	if (m_bInvertPressedColumn &&
		nCol == m_pWndList->m_nDraggedColumn && !bIsGroupBox &&
		!m_pWndList->m_bDragGroupItem && !m_pWndList->m_bDragFromChooser)
	{
		pDC->InvertRect (rectColumn);
	}
}
//*******************************************************************************************
void CBCGPGridColumnsInfo::PrintColumn (CDC* pDC, int nColumn, CRect rectItem, CRect rectClipItem)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);
	ASSERT_VALID (m_pWndList);

	// map to printer metrics
	HDC hDCFrom = ::GetDC(NULL);
	int nXMul = pDC->GetDeviceCaps(LOGPIXELSX);			// pixels in print dc
	int nXDiv = ::GetDeviceCaps(hDCFrom, LOGPIXELSX);	// pixels in screen dc
	::ReleaseDC(NULL, hDCFrom);

	//-----------
	// Draw text:
	//-----------
	CRect rectText = rectItem;
	int nTextMargin = ::MulDiv(TEXT_MARGIN, nXMul, nXDiv);
	rectText.DeflateRect (nTextMargin, nTextMargin, nTextMargin, 0);
	
	pDC->SetTextColor (m_pWndList->m_clrPrintText);
	
	UINT uiTextFlags = DT_TOP | DT_END_ELLIPSIS | DT_NOPREFIX;
	
	int nAlign = GetHeaderAlign (nColumn);
	if (nAlign & HDF_CENTER)
	{
		uiTextFlags |= DT_CENTER;
	}
	else if (nAlign & HDF_RIGHT)
	{
		uiTextFlags |= DT_RIGHT;
	}
	
	if (GetHeaderMultiLine (nColumn))
	{
		uiTextFlags |= DT_WORDBREAK;
	}
	else
	{
		uiTextFlags |= DT_SINGLELINE;
	}
	
	m_pWndList->DoDrawText (pDC, GetColumnName (nColumn), rectText, uiTextFlags, rectClipItem);
	
	//-----------------
	// Draw sort arrow:
	//-----------------
	int nColState = GetColumnState (nColumn);
	if (nColState != 0)
	{
		CRect rectArrow = rectItem;
		rectArrow.bottom = min (rectArrow.bottom, rectArrow.top + m_pWndList->GetPrintParams ().m_nRowHeight);
		int nMargin = ::MulDiv(5, nXMul, nXDiv);
		rectArrow.DeflateRect (nMargin, nMargin);
		rectArrow.left = max (rectArrow.right - rectArrow.Height (), rectArrow.left);
		
		int dy2 = (int) (.134 * rectArrow.Width ());
		rectArrow.DeflateRect (0, dy2);
		
		BOOL bAscending = nColState >= 0;

		rectArrow.NormalizeRect ();
		if (rectArrow.IntersectRect (&rectArrow, &rectClipItem))
		{
			CBCGPMenuImages::Draw (pDC, bAscending ? CBCGPMenuImages::IdArowUp: CBCGPMenuImages::IdArowDown, 
				rectArrow, CBCGPMenuImages::ImageBlack, rectArrow.Size());
		}
	}
	
	//-------------
	// Draw border:
	//-------------
	if (rectClipItem.left <= rectItem.left && rectClipItem.right >= rectItem.left)
	{
		pDC->MoveTo (rectItem.TopLeft ());
		pDC->LineTo (rectItem.left, rectItem.bottom);
	}
}
//*******************************************************************************************
void CBCGPGridColumnsInfo::GetHeaderTooltip (int nColumn, CPoint, CString& strText) const
{
	strText.Empty ();

	if (nColumn >= 0 && nColumn < GetColumnCount () &&
		GetColumnNameTrancated (nColumn))
	{
		strText = GetColumnName (nColumn);
	}
}
//*******************************************************************************************
void CBCGPGridColumnsInfo::SetHighlightColumn (int nColumn)
{
	m_nHighlightedItem = nColumn;
}
//*******************************************************************************************
int CBCGPGridColumnsInfo::GetHighlightColumn () const
{
	return m_nHighlightedItem;
}
//*******************************************************************************************
void CBCGPGridColumnsInfo::SetHighlightColumnBtn (int nColumn)
{
	m_nHighlightedItemBtn = nColumn;
}
//*******************************************************************************************
int CBCGPGridColumnsInfo::GetHighlightColumnBtn () const
{
	return m_nHighlightedItemBtn;
}
//*******************************************************************************************
BOOL CBCGPGridColumnsInfo::IsTextColumn (int nColumn) const
{
	ASSERT_VALID (this);

	if (nColumn >= 0 && nColumn < m_arrColumns.GetSize ())
	{
		CBCGPGridColumnsItem* pColumn = m_arrColumns[nColumn];
		ASSERT_VALID (pColumn);
		
		return pColumn->m_bText;
	}

	return FALSE;
}
//*******************************************************************************************
BOOL CBCGPGridColumnsInfo::SetTextColumn (int nColumn, BOOL bText)
{
	ASSERT_VALID (this);

	if (nColumn >= 0 && nColumn < m_arrColumns.GetSize ())
	{
		CBCGPGridColumnsItem* pColumnsItem = m_arrColumns[nColumn];
		ASSERT_VALID (pColumnsItem);

		pColumnsItem->m_bText = bText;
		return TRUE;
	}

	return FALSE;
}
//*******************************************************************************************
int CBCGPGridColumnsInfo::GetLeftTextOffset () const
{
	ASSERT_VALID (this);

	int x = 0;

	int nPos = Begin ();
	while (nPos != End ())
	{
		int iColumn = Next (nPos);
		if (iColumn == -1)
		{
			return 0; // column not found - hidden column
		}

		ASSERT (iColumn >= 0);
		ASSERT (iColumn < GetColumnCount ());

		if (IsTextColumn (iColumn))
		{
			return x;
		}

		x += GetColumnWidth (iColumn);
	}

	return 0;
}
//*******************************************************************************************
void CBCGPGridColumnsInfo::EnableFreezeColumns (int nColumnCount)
{
	ASSERT (nColumnCount >= -1);
	if (!m_bAutoSize)
	{
		m_nFreezeColumns = nColumnCount;

		RecalcFreezeOffset ();
	}
}
//*******************************************************************************************
BOOL CBCGPGridColumnsInfo::IsColumnFrozen (int nColumn) const
{
	if (nColumn < 0 || nColumn >= m_arrColumns.GetSize ())
	{
		return FALSE;
	}

	int nPos = IndexToOrder (nColumn);
	if (nPos >= 0)
	{
		return (nPos < m_nFreezeColumns);
	}

	return FALSE;
}
//*******************************************************************************************
void CBCGPGridColumnsInfo::RecalcFreezeOffset ()
{
	m_nFreezeOffset = 0;

	if (m_pWndList == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	if (IsFreezeColumnsEnabled ())
	{
		//-------------------------------------------
		// Calculate the space for the frozen columns
		//-------------------------------------------
		int nXLeft = 0;
		int nColumnCount = 0;

		int nPos = Begin ();
		while (nPos != End () && nColumnCount < m_nFreezeColumns)
		{
			int iColumn = Next (nPos);
			if (iColumn == -1)
			{
				break; // no more visible columns
			}

			ASSERT (iColumn >= 0);
			int nWidth = GetColumnWidth (iColumn);

			if (nColumnCount == 0)
			{
				nWidth += m_pWndList->GetHierarchyOffset () +
					m_pWndList->GetExtraHierarchyOffset ();
			}

			nXLeft += nWidth;
			nColumnCount++;
		}

		m_nFreezeOffset = nXLeft;
	}
}
//*******************************************************************************************
BOOL CBCGPGridColumnsInfo::SetColumnFilterBarCtrl (int nColumn, CWnd* pCtrl)
{
	ASSERT_VALID (this);

	if (pCtrl != NULL)
	{
		ASSERT_VALID (pCtrl);
		if (pCtrl->GetSafeHwnd () == NULL)
		{
			return FALSE;
		}
	}
	
	if (nColumn >= 0 && nColumn < m_arrColumns.GetSize ())
	{
		CBCGPGridColumnsItem* pColumnsItem = m_arrColumns[nColumn];
		ASSERT_VALID (pColumnsItem);
		
		pColumnsItem->m_pFilterBarCtrl = pCtrl;
		return TRUE;
	}
	
	return FALSE;
}
//*******************************************************************************************
CWnd* CBCGPGridColumnsInfo::GetColumnFilterBarCtrl (int nColumn) const
{
	ASSERT_VALID (this);
	
	if (nColumn >= 0 && nColumn < m_arrColumns.GetSize ())
	{
		CBCGPGridColumnsItem* pColumn = m_arrColumns[nColumn];
		ASSERT_VALID (pColumn);
		
		return pColumn->m_pFilterBarCtrl;
	}
	
	return NULL;
}
//*******************************************************************************************
void CBCGPGridColumnsInfo::SetOwnerList (CBCGPGridCtrl* pWndList)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pWndList);

	m_pWndList = pWndList;
}
//*******************************************************************************************
int CBCGPGridColumnsInfo::InsertColumn (int nPos, LPCTSTR lpszColumn, int nWidth, int iImage, BOOL bHideNameWithImage)
{
	ASSERT_VALID (this);

	//------------------
	// Insert new column
	//------------------
	// Create new item:
	CBCGPGridColumnsItem* pColumnsItem = new CBCGPGridColumnsItem (lpszColumn, nWidth, iImage, bHideNameWithImage);
	ASSERT_VALID (pColumnsItem);

	// Insert the item at the specified index:
	if (m_arrColumns.GetSize () == nPos)
	{
		m_arrColumns.Add (pColumnsItem);
		OnInsertColumn (nPos);
		return nPos;
	}

	if (nPos >= 0 && nPos < m_arrColumns.GetSize ())
	{
		m_arrColumns.InsertAt (nPos, pColumnsItem);
		OnInsertColumn (nPos);
		return nPos;
	}

	ASSERT (FALSE);
	return -1;
}
//*******************************************************************************************
void CBCGPGridColumnsInfo::OnInsertColumn (int nPos)
{
	//----------------------------------------------------
	// Shift all column indexes in the internal containers
	//----------------------------------------------------
	//m_mapSortColumn;
	int i;
	for (i = (int) m_arrColumns.GetSize () - 2; i >= 0 && i >= nPos; i--)
	{
		int nState = 0;
		if (m_mapSortColumn.Lookup (i, nState))
		{
			m_mapSortColumn.RemoveKey (i);
			m_mapSortColumn.SetAt (i + 1, nState);
		}
	}
	//m_lstGroupingColumns;
	for (POSITION posGroup = m_lstGroupingColumns.GetHeadPosition (); posGroup != NULL;)
	{
		int iCol = m_lstGroupingColumns.GetAt (posGroup);
		if (iCol >= nPos)
		{
			iCol++;
			m_lstGroupingColumns.GetAt (posGroup) = iCol;
		}

		m_lstGroupingColumns.GetNext (posGroup);
	}
	//m_arrColumnOrder;
	for (i = nPos; i < m_arrColumnOrder.GetSize (); i++)
	{
		m_arrColumnOrder [i]++;
	}

	//-----------------------------------------
	// Add new column in the column order array
	//-----------------------------------------
	if (m_arrColumnOrder.GetSize () > 0)
	{
		m_arrColumnOrder.Add (nPos);

		ASSERT (m_arrColumnOrder.GetSize () == GetColumnCount (TRUE));
	}
}
//*******************************************************************************************
void CBCGPGridColumnsInfo::OnColumnsOrderChanged ()
{
// 	if (m_pWndList->GetSafeHwnd () != NULL)
// 	{
// 		m_pWndList->OnHeaderChanged (); // Sends BCGM_GRID_HEADER_CHANGED
// 	}
}
//*******************************************************************************************
BOOL CBCGPGridColumnsInfo::DeleteColumn (int nPos)
{
	ASSERT_VALID (this);

	if (nPos >= 0 && nPos < m_arrColumns.GetSize ())
	{
		RemoveSortColumn (nPos);
		RemoveGroupColumn (nPos);
		m_arrColumnOrder.RemoveAll ();

		CBCGPGridColumnsItem* pColumnsItem = m_arrColumns[nPos];
		m_arrColumns.RemoveAt (nPos);
		delete pColumnsItem;
		
		OnColumnsOrderChanged ();

		return TRUE;
	}

	return FALSE;
}
//*******************************************************************************************
void CBCGPGridColumnsInfo::InsertColumns (int nColumnsNum, int nDefaultWidth)
{
	ASSERT_VALID (this);

	for (int i = 0; i < nColumnsNum; i++)
	{
		CBCGPGridColumnsItem* pColumnsItem = new CBCGPGridColumnsItem (NULL, nDefaultWidth);
		ASSERT_VALID (pColumnsItem);

		m_arrColumns.Add (pColumnsItem);
	}

	OnColumnsOrderChanged ();
}
//*******************************************************************************************
void CBCGPGridColumnsInfo::DeleteAllColumns ()
{
	ASSERT_VALID (this);

	for (int i = (int) m_arrColumns.GetUpperBound (); i >= 0; i--)
	{
		delete m_arrColumns[i];
	}
	m_arrColumns.RemoveAll ();

	m_mapSortColumn.RemoveAll ();
	m_lstGroupingColumns.RemoveAll ();
	m_arrColumnOrder.RemoveAll ();

	OnColumnsOrderChanged ();
}
//*******************************************************************************************
int CBCGPGridColumnsInfo::GetColumnCount (BOOL bCalcVisibleOnly) const
{
	ASSERT_VALID (this);

	if (bCalcVisibleOnly)
	{
		int nCount = 0;

		for (int i = 0; i < m_arrColumns.GetSize (); i++)
		{
			CBCGPGridColumnsItem* pColumnsItem = m_arrColumns[i];
			ASSERT_VALID (pColumnsItem);

			if (pColumnsItem->m_bVisible)
			{
				nCount++;
			}
		}

		return nCount;
	}

	return (int) m_arrColumns.GetSize ();
}
//*******************************************************************************************
int CBCGPGridColumnsInfo::GetColumnWidth (int nColumn) const
{
	ASSERT_VALID (this);

	if (nColumn >= 0 && nColumn < m_arrColumns.GetSize ())
	{
		CBCGPGridColumnsItem* pColumn = m_arrColumns[nColumn];
		ASSERT_VALID (pColumn);

		if (m_bAutoSize)
		{
			return pColumn->m_nAutoSize;
		}

		int nSize = 0;
		if (pColumn->m_bVisible)
		{
			if (pColumn->m_bFixedSize)
			{
				// Specific size
				nSize = pColumn->m_nDefaultSize;
			}
			else
			{
				// Best fit
				nSize = BestFitColumn (nColumn);
			}
		}
		
		return nSize;
	}

	ASSERT (FALSE);
	return 0;
}
//*******************************************************************************************
BOOL CBCGPGridColumnsInfo::SetColumnWidth(int nColumn, int nWidth)
{
	ASSERT_VALID (this);

	if (nColumn >= 0 && nColumn < m_arrColumns.GetSize ())
	{
		CBCGPGridColumnsItem* pColumnsItem = m_arrColumns[nColumn];
		ASSERT_VALID (pColumnsItem);

		pColumnsItem->m_nDefaultSize = nWidth;

		return TRUE;
	}

	return FALSE;
}
//*******************************************************************************************
LPCTSTR CBCGPGridColumnsInfo::GetColumnName(int nColumn) const
{
	ASSERT_VALID (this);

	if (nColumn >= 0 && nColumn < m_arrColumns.GetSize ())
	{
		CBCGPGridColumnsItem* pColumnsItem = m_arrColumns[nColumn];
		ASSERT_VALID (pColumnsItem);

		return pColumnsItem->m_strName;
	}

	ASSERT (FALSE);
	return NULL;
}
//*******************************************************************************************
BOOL CBCGPGridColumnsInfo::SetColumnName(int nColumn, LPCTSTR lpszColumn)
{
	ASSERT_VALID (this);
	ASSERT (lpszColumn != NULL);

	if (nColumn >= 0 && nColumn < m_arrColumns.GetSize ())
	{
		CBCGPGridColumnsItem* pColumnsItem = m_arrColumns[nColumn];
		ASSERT_VALID (pColumnsItem);

		pColumnsItem->m_strName = lpszColumn;

		return TRUE;
	}

	return FALSE;
}
//*******************************************************************************************
BOOL CBCGPGridColumnsInfo::GetColumnTextHidden (int nColumn) const
{
	ASSERT_VALID (this);
	
	if (nColumn >= 0 && nColumn < m_arrColumns.GetSize ())
	{
		CBCGPGridColumnsItem* pColumnsItem = m_arrColumns[nColumn];
		ASSERT_VALID (pColumnsItem);
		
		return pColumnsItem->m_bHideName;
	}
	
	ASSERT (FALSE);
	return FALSE;
}
//*******************************************************************************************
BOOL CBCGPGridColumnsInfo::SetColumnTextHidden (int nColumn, BOOL bHideTextInHeader)
{
	ASSERT_VALID (this);
	
	if (nColumn >= 0 && nColumn < m_arrColumns.GetSize ())
	{
		CBCGPGridColumnsItem* pColumnsItem = m_arrColumns[nColumn];
		ASSERT_VALID (pColumnsItem);
		
		pColumnsItem->m_bHideName = bHideTextInHeader;
		
		return TRUE;
	}
	
	return FALSE;
}
//*******************************************************************************************
int CBCGPGridColumnsInfo::GetColumnAlign (int nColumn) const
{
	ASSERT_VALID (this);

	if (nColumn >= 0 && nColumn < m_arrColumns.GetSize ())
	{
		CBCGPGridColumnsItem* pColumnsItem = m_arrColumns[nColumn];
		ASSERT_VALID (pColumnsItem);

		return pColumnsItem->m_nAlignment;
	}

	ASSERT (FALSE);
	return HDF_LEFT;
}
//*******************************************************************************************
BOOL CBCGPGridColumnsInfo::SetColumnAlign (int nColumn, int nAlign)
{
	ASSERT_VALID (this);

	if (nColumn >= 0 && nColumn < m_arrColumns.GetSize ())
	{
		CBCGPGridColumnsItem* pColumnsItem = m_arrColumns[nColumn];
		ASSERT_VALID (pColumnsItem);

		pColumnsItem->m_nAlignment = nAlign;

		return TRUE;
	}

	return FALSE;
}
//*******************************************************************************************
int CBCGPGridColumnsInfo::GetHeaderAlign (int nColumn) const
{
	ASSERT_VALID (this);

	if (nColumn >= 0 && nColumn < m_arrColumns.GetSize ())
	{
		CBCGPGridColumnsItem* pColumnsItem = m_arrColumns[nColumn];
		ASSERT_VALID (pColumnsItem);

		return pColumnsItem->m_nHeaderAlignment;
	}

	ASSERT (FALSE);
	return HDF_LEFT;
}
//*******************************************************************************************
BOOL CBCGPGridColumnsInfo::SetHeaderAlign (int nColumn, int nAlign)
{
	ASSERT_VALID (this);

	if (nColumn >= 0 && nColumn < m_arrColumns.GetSize ())
	{
		CBCGPGridColumnsItem* pColumnsItem = m_arrColumns[nColumn];
		ASSERT_VALID (pColumnsItem);

		pColumnsItem->m_nHeaderAlignment = nAlign;

		return TRUE;
	}

	return FALSE;
}
//*******************************************************************************************
BOOL CBCGPGridColumnsInfo::GetHeaderMultiLine (int nColumn) const
{
	ASSERT_VALID (this);
	
	if (nColumn >= 0 && nColumn < m_arrColumns.GetSize ())
	{
		CBCGPGridColumnsItem* pColumnsItem = m_arrColumns[nColumn];
		ASSERT_VALID (pColumnsItem);
		
		return pColumnsItem->m_bHeaderMultiLine;
	}
	
	ASSERT (FALSE);
	return FALSE;
}
//*******************************************************************************************
BOOL CBCGPGridColumnsInfo::SetHeaderMultiLine (int nColumn, BOOL bMultiLine)
{
	ASSERT_VALID (this);
	
	if (nColumn >= 0 && nColumn < m_arrColumns.GetSize ())
	{
		CBCGPGridColumnsItem* pColumnsItem = m_arrColumns[nColumn];
		ASSERT_VALID (pColumnsItem);
		
		pColumnsItem->m_bHeaderMultiLine = bMultiLine;
		
		return TRUE;
	}
	
	return FALSE;
}
//*******************************************************************************************
BOOL CBCGPGridColumnsInfo::GetColumnNameTrancated (int nColumn) const
{
	ASSERT_VALID (this);
	
	if (nColumn >= 0 && nColumn < m_arrColumns.GetSize ())
	{
		CBCGPGridColumnsItem* pColumnsItem = m_arrColumns[nColumn];
		ASSERT_VALID (pColumnsItem);
		
		return pColumnsItem->m_bNameIsTrancated;
	}
	
	ASSERT (FALSE);
	return FALSE;
}
//*******************************************************************************************
BOOL CBCGPGridColumnsInfo::SetColumnNameTrancated (int nColumn, BOOL bIsTrancated)
{
	ASSERT_VALID (this);
	
	if (nColumn >= 0 && nColumn < m_arrColumns.GetSize ())
	{
		CBCGPGridColumnsItem* pColumnsItem = m_arrColumns[nColumn];
		ASSERT_VALID (pColumnsItem);
		
		pColumnsItem->m_bNameIsTrancated = bIsTrancated;
		
		return TRUE;
	}
	
	return FALSE;
}
//*******************************************************************************************
int CBCGPGridColumnsInfo::GetColumnImage (int nColumn) const
{
	ASSERT_VALID (this);

	if (nColumn >= 0 && nColumn < m_arrColumns.GetSize ())
	{
		CBCGPGridColumnsItem* pColumnsItem = m_arrColumns[nColumn];
		ASSERT_VALID (pColumnsItem);

		return pColumnsItem->m_iImage;
	}

	ASSERT (FALSE);
	return -1;
}
//*******************************************************************************************
BOOL CBCGPGridColumnsInfo::SetColumnImage (int nColumn, int nImage)
{
	ASSERT_VALID (this);

	if (nColumn >= 0 && nColumn < m_arrColumns.GetSize ())
	{
		CBCGPGridColumnsItem* pColumnsItem = m_arrColumns[nColumn];
		ASSERT_VALID (pColumnsItem);

		pColumnsItem->m_iImage = nImage;
		return TRUE;
	}

	return FALSE;
}
//*******************************************************************************************
BOOL CBCGPGridColumnsInfo::SetColumnOrderArray (int iCount, LPINT piArray)
{
	ASSERT_VALID (this);
	ASSERT (iCount <= GetColumnCount ());
	ASSERT (AfxIsValidAddress (piArray, iCount * sizeof(int), FALSE)); //for readonly

	// -----------------------
	// Set all columns hidden:
	// -----------------------
	int i;
	for (i = 0; i < m_arrColumns.GetSize (); i++)
	{
		CBCGPGridColumnsItem* pColumn = m_arrColumns[i];
		ASSERT_VALID (pColumn);

		pColumn->m_bVisible = FALSE;
	}

	m_arrColumnOrder.SetSize (iCount);
	for (i = 0; i < iCount; i++)
	{
		m_arrColumnOrder [i] = piArray [i];

		// ------------------------------------------------------------
		// Set visible the items which are specified in the order array
		// ------------------------------------------------------------
		if (piArray [i] >= 0 && piArray [i] < m_arrColumns.GetSize ())
		{
			CBCGPGridColumnsItem* pColumn = m_arrColumns[piArray [i]];
			ASSERT_VALID (pColumn);
			
			pColumn->m_bVisible = TRUE;
		}
	}

	OnColumnsOrderChanged ();
	return TRUE;
}
//*******************************************************************************************
int CBCGPGridColumnsInfo::GetColumnOrderArray (LPINT piArray, int iCount) const
{
	ASSERT_VALID (this);

	// if -1 was passed, find the count ourselves
	int nCount = iCount;
	if (nCount == -1)
	{
		nCount = GetColumnCount ();

		if (nCount == -1)
			return 0;
	}

	ASSERT (AfxIsValidAddress (piArray, nCount * sizeof(int)));
	ASSERT (nCount <= GetColumnCount ());

	// copy column indexes skipping hidden columns
	int i = 0;
	int iCol = 0;

	for (iCol = 0; iCol < m_arrColumnOrder.GetSize () && i < nCount; iCol++)
	{
		if (GetColumnVisible (m_arrColumnOrder [iCol])) // visible
		{
			int nSortCol = m_arrColumnOrder [iCol];
			piArray [i] = nSortCol;
			i++;
		}
	}
	if (m_arrColumnOrder.GetSize () == 0)
	{
		for (int nCol = 0; nCol < GetColumnCount () && i < nCount; nCol++)
		{
			if (GetColumnVisible (nCol)) // visible
			{
				int nSortCol = nCol;
				piArray [i] = nSortCol;
				i++;
			}
		}
	}

	return i;
}
//*******************************************************************************************
int CBCGPGridColumnsInfo::GetGroupingColumnOrderArray (LPINT piArray, int iCount) const
{
	ASSERT_VALID (this);

	// if -1 was passed, find the count ourselves
	int nCount = iCount;
	if (nCount == -1)
	{
		nCount = GetGroupColumnCount () + GetSortColumnCount ();

		if (nCount == -1)
			return 0;
	}

	//ASSERT (AfxIsValidAddress (piArray, nCount * sizeof(int), FALSE)); //for readonly
	ASSERT (AfxIsValidAddress (piArray, nCount * sizeof(int)));
	ASSERT (nCount <= GetGroupColumnCount () + GetSortColumnCount ());

	// Copy list of sorted columns to ensure every one will be recorded:
	CMap<int, int, int, int> mapNotReferenced;
	int nColSave, nStateSave;
	for (POSITION pos = m_mapSortColumn.GetStartPosition (); pos != NULL; )
	{
		m_mapSortColumn.GetNextAssoc (pos, nColSave, nStateSave);
		mapNotReferenced.SetAt (nColSave, nStateSave);
	}

	int i = 0;
	// first grouped columns
	POSITION posGroup = m_lstGroupingColumns.GetHeadPosition ();
	for (; posGroup != NULL && i < nCount; i++)
	{
		int nGroupCol = m_lstGroupingColumns.GetNext (posGroup);
		piArray [i] = nGroupCol;
	}
	// then sorted columns
	for (int iCol = 0; iCol < m_arrColumnOrder.GetSize () && i < nCount; iCol++)
	{
		if (GetColumnState (m_arrColumnOrder [iCol]) != 0) // the column is sorted
		{
			int nSortCol = m_arrColumnOrder [iCol];
			piArray [i] = nSortCol;
			i++;
			mapNotReferenced.RemoveKey (nSortCol);
		}
	}
	if (m_arrColumnOrder.GetSize () == 0)
	{
		for (int nCol = 0; nCol < GetColumnCount () && i < nCount; nCol++)
		{
			if (GetColumnState (nCol) != 0) // the column is sorted
			{
				int nSortCol = nCol;
				piArray [i] = nSortCol;
				i++;
			}
		}
	}
	else
	{
		// then sorted columns which are not referenced before (hidden sorted columns)
		for (int nCol = 0; nCol < GetColumnCount () && i < nCount; nCol++)
		{
			int nState;
			if (!mapNotReferenced.Lookup (nCol, nState) && nState != 0) // the column is sorted
			{
				int nSortCol = nCol;
				piArray [i] = nSortCol;
				i++;
			}
		}
	}

	mapNotReferenced.RemoveAll ();

	return i;
}
//*******************************************************************************************
int CBCGPGridColumnsInfo::GetGroupColumnCount () const
{
	return (int) m_lstGroupingColumns.GetCount ();
}
//*******************************************************************************************
int CBCGPGridColumnsInfo::GetSortColumnCount () const
{
	int nCount = 0;

	int nColumn, nState;
	for (POSITION pos = m_mapSortColumn.GetStartPosition (); pos != NULL; )
	{
		m_mapSortColumn.GetNextAssoc (pos, nColumn, nState);

		if (nState != 0) // column is sorted
		{
			nCount++;
		}
	}

	return nCount;
}
//*******************************************************************************************
BOOL CBCGPGridColumnsInfo::IsGroupColumn (int nCol) const
{
	return (m_lstGroupingColumns.Find (nCol) != NULL);
}
//*******************************************************************************************
int CBCGPGridColumnsInfo::GetColumnState (int nCol) const
	// Returns: 0 - not not sorted, -1 - descending, 1 - ascending
{
	int nState = 0;
	if (!m_mapSortColumn.Lookup (nCol, nState))
	{
		return 0;
	}

	return nState;
}
//*******************************************************************************************
int CBCGPGridColumnsInfo::InsertGroupColumn (int nPos, int nColumn)
{
	ASSERT_VALID (this);
	
	POSITION posFound = m_lstGroupingColumns.Find (nColumn);

	if (m_lstGroupingColumns.GetCount () == nPos)
	{
		m_lstGroupingColumns.AddTail (nColumn);
		if (posFound != NULL)
		{
			m_lstGroupingColumns.RemoveAt (posFound);
		}
		return nPos;
	}

	POSITION pos = m_lstGroupingColumns.FindIndex (nPos);
	if (pos != NULL)
	{
		m_lstGroupingColumns.InsertBefore (pos, nColumn);
		if (posFound != NULL)
		{
			m_lstGroupingColumns.RemoveAt (posFound);
		}
		return nPos;
	}

	ASSERT (FALSE);
	return -1;
}
//*******************************************************************************************
BOOL CBCGPGridColumnsInfo::RemoveGroupColumn (int nPos)
{
	POSITION pos = m_lstGroupingColumns.FindIndex (nPos);
	if (pos == NULL)
	{
		return FALSE;
	}

	m_lstGroupingColumns.RemoveAt (pos);
	return TRUE;
}
//*******************************************************************************************
BOOL CBCGPGridColumnsInfo::RemoveGroupColumnByVal (int nColumn)
{
	POSITION pos = m_lstGroupingColumns.Find (nColumn);
	if (pos == NULL)
	{
		return FALSE;
	}

	m_lstGroupingColumns.RemoveAt (pos);
	return TRUE;
}
//*******************************************************************************************
int CBCGPGridColumnsInfo::GetGroupColumn (int nPos) const
{
	POSITION pos = m_lstGroupingColumns.FindIndex (nPos);
	if (pos != NULL)
	{
		return m_lstGroupingColumns.GetAt (pos);
	}

	return -1;
}
//*******************************************************************************************
int CBCGPGridColumnsInfo::GetGroupColumnPos (int nColumn)
{
	int nPos = 0;
	for (POSITION pos = m_lstGroupingColumns.GetHeadPosition (); pos != NULL; nPos++)
	{
		int nGroupColumn = m_lstGroupingColumns.GetNext (pos);

		if (nGroupColumn == nColumn)
		{
			return nPos;
		}
	}

	return -1;
}
//*******************************************************************************************
void CBCGPGridColumnsInfo::SetSortColumn (int nColumn, BOOL bAscening, BOOL bAdd)
{
	if (bAdd)
	{
		if (!m_bMultipleSort)
		{
			ASSERT (FALSE);
			bAdd = FALSE;
		}
	}

	if (!bAdd)
	{
		m_mapSortColumn.RemoveAll ();
	}

	m_mapSortColumn.SetAt (nColumn, bAscening ? 1 : -1);
}
//*******************************************************************************************
BOOL CBCGPGridColumnsInfo::RemoveSortColumn (int nColumn)
{
	return m_mapSortColumn.RemoveKey (nColumn);
}
//*******************************************************************************************
void CBCGPGridColumnsInfo::RemoveAllSortColumns ()
{
	m_mapSortColumn.RemoveAll ();
}
//*******************************************************************************************
int CBCGPGridColumnsInfo::GetSortColumn () const
{
	ASSERT_VALID (this);

	if (m_bMultipleSort)
	{
		TRACE0("Call CBCGPGridColumnsInfo::GetColumnState for muliple sort\n");
		ASSERT (FALSE);
		return -1;
	}

	POSITION pos = m_mapSortColumn.GetStartPosition ();
	if (pos != NULL)
	{
		int nColumn, nState;
		m_mapSortColumn.GetNextAssoc (pos, nColumn, nState);

		if (nState != 0) // column is sorted
		{
			return nColumn;
		}
	}

	return -1;
}
//*******************************************************************************************
void CBCGPGridColumnsInfo::EnableMultipleSort (BOOL bEnable)
{
	ASSERT_VALID (this);

	if (m_bMultipleSort == bEnable)
	{
		return;
	}

	m_bMultipleSort = bEnable;

	if (!m_bMultipleSort)
	{
		m_mapSortColumn.RemoveAll ();
	}
}
//*******************************************************************************************
void CBCGPGridColumnsInfo::EnableAutoSize (BOOL bEnable)
{
	m_bAutoSize = bEnable;
}
//*******************************************************************************************
void CBCGPGridColumnsInfo::Resize (int nTotalWidth, int nStartColumn)
{
	ASSERT_VALID (this);
	ASSERT (nStartColumn >= -1);
	
	int nPosStartColumn = (nStartColumn != -1) ? IndexToOrder (nStartColumn) : -1;

	int nSum = 0;
	int nSumMin = 0;
	int nSumLocked = 0;

	int posFirstSizingColumn = -1;
	int posLastSizingColumn = -1;
	
	int nCount = 0; // count visible columns
	int nPos = Begin ();
	while (nPos != End ())
	{
		int nColumn = Next (nPos);
		if (nColumn == -1)
		{
			break; // no more visible columns
		}

		ASSERT (nColumn >= 0);
		ASSERT (nColumn < m_arrColumns.GetSize ());

		int posSave = nColumn;
		CBCGPGridColumnsItem* pColumn = m_arrColumns[nColumn];
		ASSERT_VALID (pColumn);

		if (pColumn->m_bVisible)
		{
			if (pColumn->m_bFixedSize)
			{
				// Specific size
				nSum += pColumn->m_nDefaultSize;
			}
			else
			{
				// Best fit
				nSum += BestFitColumn (nColumn);
			}
			
			if (CanChangeWidth (nColumn) && nCount > nPosStartColumn)
			{
				// Can sizing
				nSumMin += GetMinWidth (nColumn);

				if (posFirstSizingColumn == -1)
				{
					posFirstSizingColumn = posSave;
				}
				posLastSizingColumn = posSave;
			}
			else
			{
				// Locked size
				nSumMin += pColumn->m_nDefaultSize;
				nSumLocked += pColumn->m_nDefaultSize;
			}
			
		}
		
		nCount++;
	}
	
	int nHierarchyOffset = 0;
	if (m_pWndList != NULL)
	{
		ASSERT_VALID (m_pWndList);
		nHierarchyOffset = 
			m_pWndList->GetHierarchyOffset () + 
			m_pWndList->GetExtraHierarchyOffset ();
	}

	const int nDefaultWidth = nSum;
	const int nMinWidth = nSumMin;
	const int nLockedWidth = nSumLocked;

	ASSERT (nDefaultWidth >= nLockedWidth);
	
	if (m_bAutoSize)
	{
		m_nTotalWidth = nTotalWidth - nHierarchyOffset;
		
		if (nMinWidth >= m_nTotalWidth ||
			nMinWidth >= nDefaultWidth)
		{
			// use minimal width for all columns
			nCount = 0;	// count visible columns
			int nPos = Begin ();
			while (nPos != End ())
			{
				int nColumn = Next (nPos);
				if (nColumn == -1)
				{
					break; // no more visible columns
				}

				ASSERT (nColumn >= 0);
				ASSERT (nColumn < m_arrColumns.GetSize ());

				CBCGPGridColumnsItem* pColumn = m_arrColumns[nColumn];
				ASSERT_VALID (pColumn);
	
				if (pColumn->m_bVisible)
				{
					if (CanChangeWidth (nColumn) && nCount > nPosStartColumn)
					{
						pColumn->m_nAutoSize = GetMinWidth (nColumn);
					}
					else
					{
						pColumn->m_nAutoSize = pColumn->m_nDefaultSize;
					}
				}

				nCount++;
			}
			
		}
		
		else if (m_nTotalWidth >= nDefaultWidth)
		{
			// enlarge columns in proporsion

			ASSERT (nLockedWidth >= 0);
			ASSERT (nDefaultWidth > nLockedWidth);
			ASSERT (m_nTotalWidth > nLockedWidth);
			float k = ((float) (m_nTotalWidth - nLockedWidth)) / 
				(nDefaultWidth - nLockedWidth);
			ASSERT (k >= 1);


			nSum = 0;
			
			nCount = 0;	// count visible columns
			int nPos = Begin ();
			while (nPos != End ())
			{
				int nColumn = Next (nPos);
				if (nColumn == -1)
				{
					break; // no more visible columns
				}

				ASSERT (nColumn >= 0);
				ASSERT (nColumn < m_arrColumns.GetSize ());

				CBCGPGridColumnsItem* pColumn = m_arrColumns[nColumn];
				ASSERT_VALID (pColumn);

				if (pColumn->m_bVisible)
				{
					if (CanChangeWidth (nColumn) && nCount > nPosStartColumn)
					{
						// Can sizing
						int nSize = 0;
						if (pColumn->m_bFixedSize)
						{
							// Specific size
							nSize = pColumn->m_nDefaultSize;
						}
						else
						{
							// Best fit
							nSize = BestFitColumn (nColumn);
						}
						
						int nMin = GetMinWidth (nColumn);
						nSize = max (nSize, nMin);
						
						pColumn->m_nAutoSize = (int) (k * nSize);
						pColumn->m_nAutoSize = max (GetMinWidth (nColumn), pColumn->m_nAutoSize);
					}
					else
					{
						// Locked size
						pColumn->m_nAutoSize = pColumn->m_nDefaultSize;
					}
				}
				
				nSum += pColumn->m_nAutoSize;
			
				nCount++;
			}
			
			int nDx = m_nTotalWidth - nSum;
			if (nDx > 0 && posLastSizingColumn != -1)
			{
				CBCGPGridColumnsItem* pColumn = m_arrColumns[posLastSizingColumn];
				ASSERT_VALID (pColumn);
				
				pColumn->m_nAutoSize += nDx;
			}
			
		}
		
		else if (m_nTotalWidth < nDefaultWidth)
		{
			// compact columns in proporsion
			
			ASSERT (nLockedWidth >= 0);
			ASSERT (nDefaultWidth > nLockedWidth);
			ASSERT (m_nTotalWidth > nLockedWidth);
			float k = ((float) (m_nTotalWidth - nLockedWidth)) / 
				(nDefaultWidth - nLockedWidth);
			ASSERT (k < 1);

	
			nSum = 0;
	
			nCount = 0;	// count visible columns
			int nPos = Begin ();
			while (nPos != End ())
			{
				int nColumn = Next (nPos);
				if (nColumn == -1)
				{
					break; // no more visible columns
				}

				ASSERT (nColumn >= 0);
				ASSERT (nColumn < m_arrColumns.GetSize ());

				CBCGPGridColumnsItem* pColumn = m_arrColumns[nColumn];
				ASSERT_VALID (pColumn);

				if (pColumn->m_bVisible)
				{
					if (CanChangeWidth (nColumn) && nCount > nPosStartColumn)
					{
						// Can sizing
						int nSize = 0;
						if (pColumn->m_bFixedSize)
						{
							// Specific size
							nSize = pColumn->m_nDefaultSize;
						}
						else
						{
							// Best fit
							nSize = BestFitColumn (nColumn);
						}
						
						int nMin = GetMinWidth (nColumn);
						nSize = max (nSize, nMin);
						
						pColumn->m_nAutoSize = (int) (k * nSize);
						pColumn->m_nAutoSize = max (GetMinWidth (nColumn), pColumn->m_nAutoSize);
					}
					else
					{
						// Locked size
						pColumn->m_nAutoSize = pColumn->m_nDefaultSize;
					}
				}
				
				nSum += pColumn->m_nAutoSize;
			
				nCount++;
			}
			
			int nDx = m_nTotalWidth - nSum;
			if (nDx > 0 && posFirstSizingColumn != -1)
			{
				CBCGPGridColumnsItem* pColumn = m_arrColumns[posFirstSizingColumn];
				ASSERT_VALID (pColumn);
				
				pColumn->m_nAutoSize += nDx;
			}
			
		}

		m_nTotalWidth += nHierarchyOffset;
	}
	else
	{
		// No auto size - ignore nTotalWidth param
		m_nTotalWidth = nDefaultWidth + nHierarchyOffset;
	}

	RecalcFreezeOffset ();
}
//*******************************************************************************************
BOOL CBCGPGridColumnsInfo::OnChangeColumnWidth (int nColumn, int &nWidth) const
	// returns FALSE, if width is not changing
{
	ASSERT_VALID (this);
	
	if (m_bAutoSize)
	{
		int nMin = nWidth;
		int nMax = nWidth;

		if (!GetColumnWidthMinMax (nColumn, nMin, nMax))
		{
			// column can't be resized
			return FALSE;
		}

		nWidth = max (nMin, min (nMax, nWidth));
	}
	else
	{
		int nMin = GetMinWidth (nColumn);
		nWidth = max (nMin, nWidth);
	}

	return TRUE;
}
//*******************************************************************************************
BOOL CBCGPGridColumnsInfo::ResizeColumn (int nColumn, int nWidth)
{
	ASSERT_VALID (this);

	if (nColumn < 0 || nColumn >= m_arrColumns.GetSize ())
	{
		return FALSE;
	}

	CBCGPGridColumnsItem* pResizeColumn = m_arrColumns[nColumn];
	ASSERT_VALID (pResizeColumn);

	if (m_bAutoSize)
	{
		int i;
		for (i = 0; i < m_arrColumns.GetSize (); i++)
		{
			CBCGPGridColumnsItem* pColumn = m_arrColumns[i];
			ASSERT_VALID (pColumn);

			if (pColumn->m_bVisible)
			{
				pColumn->m_nDefaultSize = pColumn->m_nAutoSize;
			}
		}

		pResizeColumn->m_nDefaultSize = nWidth;
		if (pResizeColumn->m_bVisible)
		{
			Resize (m_nTotalWidth, nColumn);
		}
		
		for (i = 0; i < m_arrColumns.GetSize (); i++)
		{
			CBCGPGridColumnsItem* pColumn = m_arrColumns[i];
			ASSERT_VALID (pColumn);

			if (pColumn->m_bVisible)
			{
				pColumn->m_nDefaultSize = pColumn->m_nAutoSize;
			}
		}
	}
	else
	{
		pResizeColumn->m_nDefaultSize = nWidth;
	}
	
	return TRUE;
}
//*******************************************************************************************
BOOL CBCGPGridColumnsInfo::GetColumnWidthMinMax (int nColumn, int& nMin, int& nMax) const
	// returns FALSE, if column's not found or column can't be resized
{
	ASSERT_VALID (this);
	
	if (!m_bAutoSize)
	{
		return FALSE;
	}

	if (nColumn < 0 || nColumn >= m_arrColumns.GetSize ())
	{
		return FALSE;
	}
	
	CBCGPGridColumnsItem* pColumnTest = m_arrColumns[nColumn];
	ASSERT_VALID (pColumnTest);
	
	if (!CanChangeWidth (nColumn))
	{
		nMin = nMax = pColumnTest->m_nDefaultSize;
		return FALSE;
	}

	int nSumMin = 0;
	for (int i = 0; i < m_arrColumns.GetSize (); i++)
	{
		CBCGPGridColumnsItem* pColumn = m_arrColumns[i];
		ASSERT_VALID (pColumn);
		
		if (pColumn->m_bVisible)
		{
			if (CanChangeWidth (i) && IndexToOrder (i) > IndexToOrder (nColumn))
			{
				// Can sizing
				nSumMin += GetMinWidth (i);
			}
			else if (i != nColumn)
			{
				// Locked size
				nSumMin += pColumn->m_nDefaultSize;
			}
		}
	}

	nMin = GetMinWidth (nColumn);
	nMax = m_nTotalWidth - nSumMin;
	
	if (nMin >= nMax)
	{
		nMax = nMin;
		return FALSE;
	}
	
	return TRUE;		
}
//*******************************************************************************************
BOOL CBCGPGridColumnsInfo::GetColumnLocked (int nColumn) const
{
	ASSERT_VALID (this);

	if (nColumn >= 0 && nColumn < m_arrColumns.GetSize ())
	{
		CBCGPGridColumnsItem* pColumnsItem = m_arrColumns[nColumn];
		ASSERT_VALID (pColumnsItem);

		return pColumnsItem->m_bLocked;
	}

	ASSERT (FALSE);
	return TRUE;
}
//*******************************************************************************************
BOOL CBCGPGridColumnsInfo::SetColumnLocked (int nColumn, BOOL bLocked)
{
	ASSERT_VALID (this);

	if (nColumn >= 0 && nColumn < m_arrColumns.GetSize ())
	{
		CBCGPGridColumnsItem* pColumnsItem = m_arrColumns[nColumn];
		ASSERT_VALID (pColumnsItem);

		pColumnsItem->m_bLocked = bLocked;
		return TRUE;
	}

	ASSERT (FALSE);
	return FALSE;
}
//*******************************************************************************************
BOOL CBCGPGridColumnsInfo::GetColumnWidthAutoSize (int nColumn) const
{
	ASSERT_VALID (this);
	
	if (nColumn >= 0 && nColumn < m_arrColumns.GetSize ())
	{
		CBCGPGridColumnsItem* pColumnsItem = m_arrColumns[nColumn];
		ASSERT_VALID (pColumnsItem);
		
		return !pColumnsItem->m_bFixedSize;
	}
	
	ASSERT (FALSE);
	return FALSE;
}
//*******************************************************************************************
BOOL CBCGPGridColumnsInfo::SetColumnWidthAutoSize (int nColumn, BOOL bBestFit)
{
	ASSERT_VALID (this);
	
	if (nColumn >= 0 && nColumn < m_arrColumns.GetSize ())
	{
		CBCGPGridColumnsItem* pColumnsItem = m_arrColumns[nColumn];
		ASSERT_VALID (pColumnsItem);
		
		pColumnsItem->m_bFixedSize = !bBestFit;
		return TRUE;
	}
	
	ASSERT (FALSE);
	return FALSE;
}
//*******************************************************************************************
BOOL CBCGPGridColumnsInfo::GetColumnVisible (int nColumn) const
{
	ASSERT_VALID (this);

	if (nColumn >= 0 && nColumn < m_arrColumns.GetSize ())
	{
		CBCGPGridColumnsItem* pColumnsItem = m_arrColumns[nColumn];
		ASSERT_VALID (pColumnsItem);

		return pColumnsItem->m_bVisible;
	}

	ASSERT (FALSE);
	return FALSE;
}
//*******************************************************************************************
BOOL CBCGPGridColumnsInfo::SetColumnVisible (int nCol, BOOL bVisible)
{
	ASSERT_VALID (this);

	if (nCol >= 0 && nCol < m_arrColumns.GetSize ())
	{
		CBCGPGridColumnsItem* pColumnsItem = m_arrColumns[nCol];
		ASSERT_VALID (pColumnsItem);

		if (pColumnsItem->m_bVisible != bVisible)
		{
			pColumnsItem->m_bVisible = bVisible;

			if (m_arrColumnOrder.GetSize () > 0)
			{
				// -------------------------------
				// Add a column to the order array
				// -------------------------------
				if (bVisible)
				{
					m_arrColumnOrder.Add (nCol);
				}

				// ------------------------------------
				// Remove a column from the order array
				// ------------------------------------
				else
				{
					int iToDel = -1;
					for (int i = 0; i < m_arrColumnOrder.GetSize (); i++)
					{
						if (m_arrColumnOrder [i] == nCol)
						{
							iToDel = i;
							break;
						}
					}

					if (iToDel != -1)
					{
						m_arrColumnOrder.RemoveAt (iToDel);
					}
				}
			}
		}

		OnColumnsOrderChanged ();
		return TRUE;
	}

	ASSERT (FALSE);
	return FALSE;
}
//*******************************************************************************************
DWORD_PTR CBCGPGridColumnsInfo::GetColumnData (int nColumn) const
{
	ASSERT_VALID (this);
	
	if (nColumn >= 0 && nColumn < m_arrColumns.GetSize ())
	{
		CBCGPGridColumnsItem* pColumnsItem = m_arrColumns[nColumn];
		ASSERT_VALID (pColumnsItem);
		
		return pColumnsItem->m_dwData;
	}
	
	ASSERT (FALSE);
	return 0;
}
//*******************************************************************************************
BOOL CBCGPGridColumnsInfo::SetColumnData (int nColumn, DWORD_PTR dwData)
{
	ASSERT_VALID (this);
	
	if (nColumn >= 0 && nColumn < m_arrColumns.GetSize ())
	{
		CBCGPGridColumnsItem* pColumnsItem = m_arrColumns[nColumn];
		ASSERT_VALID (pColumnsItem);
		
		pColumnsItem->m_dwData = dwData;
		return TRUE;
	}
	
	ASSERT (FALSE);
	return FALSE;
}
//*******************************************************************************************
BOOL CBCGPGridColumnsInfo::CanChangeWidth (int nColumn) const // can't change width for icons etc
{
	return !GetColumnLocked (nColumn);
}
//*******************************************************************************************
int CBCGPGridColumnsInfo::BestFitColumn (int nColumn) const
{
	if (m_pWndList != NULL)
	{
		ASSERT_VALID (m_pWndList);

		// Best fit column
		int nAutoSize = m_pWndList->OnGetColumnAutoSize (nColumn);
		if (nAutoSize >= 0)
		{
			return nAutoSize;
		}
	}

	if (nColumn >= 0 && nColumn < m_arrColumns.GetSize ())
	{
		CBCGPGridColumnsItem* pColumn = m_arrColumns[nColumn];
		ASSERT_VALID (pColumn);

		return pColumn->m_nDefaultSize;
	}

	ASSERT (FALSE);
	return 0;
}
//*******************************************************************************************
int CBCGPGridColumnsInfo::GetMinWidth (int nColumn) const
{
	if (m_pWndList != NULL)
	{
		ASSERT_VALID (m_pWndList);

		return m_pWndList->OnGetColumnMinWidth (nColumn);
	}

	ASSERT (FALSE);
	return 0;
}

//*******************************************************************************************
void CBCGPGridColumnsInfo::GetColumnRect (int nColumn, CRect& rect) const
{
	if (m_pWndList == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	rect.SetRectEmpty ();

	BOOL bLast = (nColumn >= m_arrColumns.GetSize ());

	if (!bLast && !GetColumnVisible (nColumn))
	{
		return;
	}

	CRect rectColumn = m_pWndList->m_rectHeader;
	int x = rectColumn.left;

	BOOL bFirst = TRUE;
	int nCount = 0;	// count visible columns

	int nPos = Begin ();
	while (nPos != End ())
	{
		int iColumn = Next (nPos);
		if (iColumn == -1)
		{
			break;	// column not found - hidden column
		}

		ASSERT (iColumn >= 0);
		ASSERT (iColumn < GetColumnCount ());

		rectColumn.left = x;
		rectColumn.right = rectColumn.left + GetColumnWidth (iColumn);

		if (bFirst)
		{
			rectColumn.right += 
				m_pWndList->GetHierarchyOffset () + 
				m_pWndList->GetExtraHierarchyOffset ();
			bFirst = FALSE;
		}

		if (iColumn == nColumn)
		{
			rect = rectColumn;

			if (nCount >= m_pWndList->GetColumnsInfo ().GetFrozenColumnCount ()) // frozen column can't be scrolled
			{
				rect.OffsetRect (-m_pWndList->m_nHorzScrollOffset, 0);
			}
			return;
		}

		x = rectColumn.right;
		nCount++;
	}

	ASSERT (bLast);

	rectColumn.left = x - m_pWndList->m_nHorzScrollOffset;
	rectColumn.right = m_pWndList->m_rectHeader.right + 2;
	rect = rectColumn;
}
//********************************************************************************
int CBCGPGridColumnsInfo::HitTestColumn (CPoint point, BOOL bDelimiter, int nDelta,
										 CBCGPGridColumnsInfo::ClickArea* pnArea) const
{
	if (m_pWndList == NULL)
	{
		ASSERT (FALSE);
		return -1;
	}

	if (!m_pWndList->m_rectHeader.PtInRect (point))
	{
		return -1;
	}

	int nXLeft = m_pWndList->m_rectHeader.left;
	int nXLeftScrolled = m_pWndList->m_rectHeader.left -m_pWndList->m_nHorzScrollOffset;
	BOOL bFirst = TRUE;
	int nCount = 0;	// count visible columns

	int nPos = Begin ();
	while (nPos != End ())
	{
		int iColumn = Next (nPos);
		if (iColumn == -1)
		{
			break;	// no more visible columns
		}

		ASSERT (iColumn >= 0);
		ASSERT (iColumn < GetColumnCount ());

		// frozen column can't be scrolled
		int x = (nCount < m_pWndList->GetColumnsInfo ().GetFrozenColumnCount ()) ? nXLeft : nXLeftScrolled;
		int nColumnWidth = GetColumnWidth (iColumn);

		if (bFirst)
		{
			nColumnWidth += 
				m_pWndList->GetHierarchyOffset () + 
				m_pWndList->GetExtraHierarchyOffset ();
			bFirst = FALSE;
		}

		if (point.x <= x + nColumnWidth + nDelta)
		{
			if ((abs (point.x - (x + nColumnWidth)) <= nDelta))
			{
				if (pnArea != NULL)
				{
					*pnArea = CBCGPGridColumnsInfo::ClickDivider;
				}
			}
			else
			{
				if (bDelimiter)
				{
					return -1;
				}

				if (pnArea != NULL)
				{
					CRect rectColumn = m_pWndList->m_rectHeader;
					rectColumn.left = x;
					rectColumn.right = x + nColumnWidth;

					if (m_pWndList->GetHeaderMenuButtonRect (rectColumn, iColumn).PtInRect (point))
					{
						*pnArea = CBCGPGridColumnsInfo::ClickHeaderButton;
					}
					else
					{
						*pnArea = CBCGPGridColumnsInfo::ClickHeader;
					}
				}
			}

			return iColumn;
		}

		nXLeft += nColumnWidth;
		nXLeftScrolled += nColumnWidth;
		nCount++;
	}

	return -1;
}
//*******************************************************************************************
int CBCGPGridColumnsInfo::Begin () const
{
	return 0;
}
//*******************************************************************************************
int CBCGPGridColumnsInfo::End () const
{
	// specified column order
	if (m_arrColumnOrder.GetSize () > 0)
	{
		return (int) m_arrColumnOrder.GetSize ();
	}

	// default column order
	else
	{
		return (int) m_arrColumns.GetSize ();
	}
}
//*******************************************************************************************
int CBCGPGridColumnsInfo::Next (int& i) const
// Returns next visible column.
// Return value specifies the index in CBCGPGridColumnsInfo::m_arrColumns if succeed, 
// or -1 if visible column is not found.
{
	// specified column order
	if (m_arrColumnOrder.GetSize () > 0)
	{
        ASSERT (i < (int) m_arrColumnOrder.GetSize ());

        int nColumn = m_arrColumnOrder[i++];
        ASSERT (GetColumnVisible (nColumn));
        
        return nColumn;
	}

	// default column order
	else
	{
        ASSERT (i < (int) m_arrColumns.GetSize ());

        BOOL bVisible;

        do
        {
			if (i < 0 || i == (int) m_arrColumns.GetSize ())
            {
                return -1;  // no more visible columns
            }

			ASSERT (i >= 0);
            ASSERT (i < m_arrColumns.GetSize ());

	        CBCGPGridColumnsItem* pColumnsItem = m_arrColumns[i];
	        ASSERT_VALID (pColumnsItem);

            bVisible = pColumnsItem->m_bVisible;

            ++i;
        }
        while (!bVisible); // skip hidden columns

        int nColumn = i - 1;

        return nColumn;
	}
}
//*******************************************************************************************
BOOL CBCGPGridColumnsInfo::ChangeColumnOrder (int nNewPos, int nColumn)
{
	ASSERT_VALID (this);

	if (!GetColumnVisible (nColumn))
	{
		return FALSE;
	}
	
	// -----------------
	// Get column order:
	// -----------------
	const int nColumnCount = GetColumnCount (TRUE);
	int* aColumnsOrder = new int [nColumnCount];
	memset (aColumnsOrder, 0, nColumnCount * sizeof (int));

	if (!GetColumnOrderArray ((LPINT) aColumnsOrder, nColumnCount))
	{
		ASSERT (FALSE);	// error getting columns order

		delete [] aColumnsOrder;
		return FALSE;
	}

	ASSERT (nNewPos >= 0);
	ASSERT (nNewPos < nColumnCount);

	// -----------------------------------------------
	// Move value (nColumn) saved in m_arrColumnOrder 
	// to m_arrColumnOrder[nNewPos]:
	// -----------------------------------------------
	m_arrColumnOrder.RemoveAll ();
	m_arrColumnOrder.SetSize (nColumnCount);

	int iSource = 0;
	int iTarget = 0;
	while (iTarget < nColumnCount && iSource <= nColumnCount)
	{
		if (iTarget == nNewPos)
		{
			m_arrColumnOrder[iTarget++] = nColumn;
		}
		else
		{
			if (aColumnsOrder[iSource] == nColumn)
			{
				iSource++;
			}
			else
			{
				m_arrColumnOrder[iTarget++] = aColumnsOrder[iSource++];
			}
		}
	}

	delete [] aColumnsOrder;

	OnColumnsOrderChanged ();
	return TRUE;
}
//*******************************************************************************************
int CBCGPGridColumnsInfo::IndexToOrder (int nColumn) const
// Returns -1, if column is hidden or not found.
{
	ASSERT_VALID (this);

	if (nColumn < 0 || nColumn >= m_arrColumns.GetSize ())
	{
		// column not found
		ASSERT (FALSE);
		return -1;
	}

	if (!GetColumnVisible (nColumn))
	{
		return -1; // column is hidden
	}
	
	int nCount = 0;	// count visible columns

	int nPos = Begin ();
	while (nPos != End ())
	{
		int iColumn = Next (nPos);
		if (iColumn == -1)
		{
			break;	// no more visible columns
		}

		if (iColumn == nColumn)
		{
			return nCount;
		}

		nCount++;
	}

	return -1; // column is not found
}
//*******************************************************************************************
int CBCGPGridColumnsInfo::OrderToIndex (int nPosition) const
// Returns -1, if column is not found.
{
	ASSERT_VALID (this);

	if (nPosition < 0 || nPosition >= GetColumnCount (TRUE))
	{
		// column not found
		ASSERT (FALSE);
		return -1;
	}

	int nCount = 0;	// count visible columns

	int nPos = Begin ();
	while (nPos != End ())
	{
		int iColumn = Next (nPos);
		if (iColumn == -1)
		{
			break;	// no more visible columns
		}

		if (nCount == nPosition)
		{
			ASSERT (iColumn >= 0);
			ASSERT (iColumn < GetColumnCount ());

			return iColumn;
		}

		nCount++;
	}

	return -1;
}
//******************************************************************************************
int CBCGPGridColumnsInfo::GetFirstVisibleColumn () const
{
	int nPos = Begin ();
	if (nPos != End ())
	{
		return Next (nPos);
	}

	return -1;
}
//******************************************************************************************
int CBCGPGridColumnsInfo::GetLastVisibleColumn () const
{
	int iLastColumn = -1;

	int nPos = Begin ();
	while (nPos != End ())
	{
		int iColumn = Next (nPos);
		if (iColumn == -1)
		{
			break;	// no more visible columns
		}

		iLastColumn = iColumn;
	}

	return iLastColumn;
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPGridColumnsInfoEx - implements multiline header

IMPLEMENT_DYNCREATE (CBCGPHeaderItem, CObject);

CBCGPHeaderItem::CBCGPHeaderItem () :
	m_strName (), m_iImage (-1), m_nAlign (HDF_LEFT),
	m_bMultiLine (FALSE), m_bVertical (FALSE), m_nColumn (0)
{
	Init ();
}

CBCGPHeaderItem::CBCGPHeaderItem (int nColumn, LPCTSTR lpszLabel, int nAlign, int iImage) :
	m_strName (), m_iImage (iImage), m_nAlign (nAlign),
	m_bMultiLine (FALSE), m_bVertical (FALSE), m_nColumn (nColumn)
{
	Init ();

	if (lpszLabel != NULL)
	{
		m_strName = lpszLabel;
	}
}

CBCGPHeaderItem::~CBCGPHeaderItem ()
{
}

void CBCGPHeaderItem::Init ()
{
	m_nTextMargin = 5;
	m_nArrowMargin = 5;
	m_nHierarchyOffset = 0; // default calculated before loop
	m_bIsGroupBox = FALSE;
	m_bIsHeaderItemDragWnd = FALSE;
	m_bNoText = FALSE; // default draw text if (bIsGroupBox || !GetColumnTextHidden (nCol))
	m_bNoImage = FALSE;
	m_bNoSortArrow = TRUE;
	m_bNoButton = TRUE; // default (nCol == m_pWndList->m_nDraggedColumn || bIsGroupBox);
	m_bNotPressed = FALSE;

	m_bNameIsTrancated = FALSE;
}
//******************************************************************************************
IMPLEMENT_DYNCREATE (CBCGPMergedHeaderItem, CBCGPHeaderItem);

CBCGPMergedHeaderItem::CBCGPMergedHeaderItem () : CBCGPHeaderItem ()
{
}

CBCGPMergedHeaderItem::CBCGPMergedHeaderItem (const CArray<int, int>* pCols,
									  const CArray<int, int>* pLines,
									  int nColumn,
									  LPCTSTR lpszLabel, int nAlign, int iImage) : 
	CBCGPHeaderItem (nColumn, lpszLabel, nAlign, iImage)
{
	if (pCols != NULL)
	{
		m_arrColumns.Append (*pCols);
	}
	if (pLines != NULL)
	{
		m_arrHeaderLines.Append (*pLines);
	}
}

CBCGPMergedHeaderItem::~CBCGPMergedHeaderItem ()
{
}
//******************************************************************************************
CBCGPGridColumnsInfoEx::CBCGPGridColumnsInfoEx () : m_nMultiLineCount (1)
{
}

CBCGPGridColumnsInfoEx::~CBCGPGridColumnsInfoEx ()
{
	RemoveAllHeaderItems ();
}
//******************************************************************************************
void CBCGPGridColumnsInfoEx::RemoveAllHeaderItems ()
{
	int nSaveMergedHeaderItemsCount = (int)m_arrMergedHeaderItems.GetSize ();
	
	for (int i = 0; i < nSaveMergedHeaderItemsCount; i++)
	{
		delete m_arrMergedHeaderItems [i];
		m_arrMergedHeaderItems [i] = NULL;
	}
	m_arrMergedHeaderItems.RemoveAll ();
	
	if (nSaveMergedHeaderItemsCount > 0)
	{
		m_nMultiLineCount = 1;
	}

	m_layout.SetLayout (0, 0);
}
//******************************************************************************************
void CBCGPGridColumnsInfoEx::SetHeaderLineCount (int nLineCount)
{
	ASSERT (nLineCount > 0);
	m_nMultiLineCount = nLineCount;

	ASSERT_VALID (m_pWndList);
	if (nLineCount > 1)
	{
		m_pWndList->m_dwHeaderFlags |= BCGP_GRID_HEADER_FORCESIMPLEBORDERS;
	}
	else
	{
		m_pWndList->m_dwHeaderFlags &= ~BCGP_GRID_HEADER_FORCESIMPLEBORDERS;
	}
}
//******************************************************************************************
BOOL CBCGPGridColumnsInfoEx::AddHeaderItem (const CArray<int, int>* pCols,
											const CArray<int, int>* pLines,
											int nColumn,
											LPCTSTR lpszLabel, int nAlign, int iImage)
{
	//-----------------------
	// Check duplicated items
	//-----------------------
	// Iterate columns
	int nCol = -1;
	int i = 0;
	do 
	{
		if (pCols != NULL && pCols->GetSize () > 0)
		{
			nCol = (*pCols)[i];
		}

		// Iterate lines
		int nLine = -1;
		int j = 0;
		do 
		{
			if (pLines != NULL && pLines->GetSize () > 0)
			{
				nLine = (*pLines)[j];
			}

			// Check header position
			if (GetMergedHeaderItem (nCol, nLine) != NULL)
			{
				ASSERT(FALSE); // Column is already in other merged item
				return FALSE;
			}

		} while (pLines != NULL && ++j < pLines->GetSize ());

	} while (pCols != NULL && ++i < pCols->GetSize ());

	//---------
	// Add item
	//---------
	CBCGPMergedHeaderItem* pHeaderItem = 
		new CBCGPMergedHeaderItem (pCols, pLines, nColumn, lpszLabel, nAlign, iImage);

	m_arrMergedHeaderItems.Add (pHeaderItem);
	return TRUE;
}
//******************************************************************************************
CBCGPMergedHeaderItem* CBCGPGridColumnsInfoEx::GetMergedHeaderItem (int nColumn, int nLine) const
{
	if (nColumn < 0 || nLine < 0)
	{
		return NULL;
	}

	CBCGPMergedHeaderItem* pMergedItem = NULL;

	// Get merged header item by nColumn and nLine
	for (int i = 0; i < m_arrMergedHeaderItems.GetSize (); i++)
	{
		BOOL bMergedColumn = (m_arrMergedHeaderItems [i]->m_arrColumns.GetSize () == 0);
		for (int j = 0; j < m_arrMergedHeaderItems [i]->m_arrColumns.GetSize (); j++)
		{
			if (nColumn == m_arrMergedHeaderItems [i]->m_arrColumns [j] || nColumn == -1)
			{
				bMergedColumn = TRUE;
				break;
			}
		}
		
		BOOL bMergedLine = (m_arrMergedHeaderItems [i]->m_arrHeaderLines.GetSize () == 0);
		for (int k = 0; k < m_arrMergedHeaderItems [i]->m_arrHeaderLines.GetSize (); k++)
		{
			if (nLine == m_arrMergedHeaderItems [i]->m_arrHeaderLines [k] || nLine == -1)
			{
				bMergedLine = TRUE;
				break;
			}
		}
		
		if (bMergedColumn && bMergedLine)
		{
			pMergedItem = m_arrMergedHeaderItems [i];
			ASSERT_VALID (pMergedItem);
			break;
		}
	}

	return pMergedItem;
}
//******************************************************************************************
void CBCGPGridColumnsInfoEx::UpdateLayout ()
{
	ASSERT_VALID (this);

	if (m_nMultiLineCount <= 0)
	{
		ASSERT(FALSE);
		return;
	}
	
	//-------------------
	// Set header layout:
	//-------------------
	m_layout.SetLayout (GetColumnCount (), m_nMultiLineCount);
	
	int nPrevColumn = -1;
	int nPos = Begin ();
	while (nPos != End ())
	{
		int nColumn = Next (nPos);
		if (nColumn == -1)
		{
			break; // no more visible columns
		}
		
		for (int nLine = 0; nLine < m_nMultiLineCount; nLine++)
		{
			CBCGPHeaderItem* pPrevColumnItem = GetMergedHeaderItem (nPrevColumn, nLine);
			CBCGPHeaderItem* pPrevLineItem = GetMergedHeaderItem (nColumn, nLine - 1);
			CBCGPHeaderItem* pItem = GetMergedHeaderItem (nColumn, nLine);
			
			int nIndex = -1;
			
			// merge with prev line
			if (nLine > 0 && ((pItem != NULL) ? (pItem == pPrevLineItem) : (pPrevLineItem == NULL)))
			{
				nIndex = m_layout.GetIndex (nColumn, nLine - 1);
			}
			// merge with prev column
			else if (nPrevColumn >= 0 && ((pItem != NULL) ? (pItem == pPrevColumnItem) : FALSE))
			{
				nIndex = m_layout.GetIndex (nPrevColumn, nLine);
			}
			// do not merge
			if (nIndex == -1)
			{
				nIndex = m_layout.AddData (CHeaderLayout::HeaderLayoutData (pItem));
			}
			
			ASSERT(nIndex != -1);
			m_layout.SetIndex (nColumn, nLine, nIndex);
		}
		
		nPrevColumn = nColumn;
	}

	ASSERT (m_layout.IsLayoutValid (GetColumnCount (), m_nMultiLineCount));
}
//******************************************************************************************
void CBCGPGridColumnsInfoEx::ReposHeaderItems ()
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pWndList);

	if (!m_layout.IsLayoutValid (GetColumnCount (), m_nMultiLineCount))
	{
		UpdateLayout ();
	}
	else
	{
		// Clear all rects, old rects must not influence min/max
		m_layout.SetRectsEmpty ();
	}
	
	CBCGPGridCtrl* pWndList = m_pWndList;
	const CRect rectHeader = pWndList->m_bIsPrinting ? 
		pWndList->GetPrintParams ().m_rectHeader : pWndList->m_rectHeader;

	BOOL bInflateBottom =
		visualManager->IsKindOf (RUNTIME_CLASS (CBCGPVisualManager2007)) ||
		visualManager->IsKindOf (RUNTIME_CLASS (CBCGPVisualManagerVS2010));
	
	const int nLineHeight = rectHeader.Height () / m_nMultiLineCount;

	//-------------------------------------------
	// Calculate position of merged header items:
	//-------------------------------------------
	for (int nColumn = 0; nColumn < GetColumnCount (); nColumn++)
	{
		CRect rectColumnHeader;
		if (pWndList->m_bIsPrinting)
		{
			GetColumnPrintRect (nColumn, rectColumnHeader);
		}
		else
		{
			GetColumnRect (nColumn, rectColumnHeader);
		}

		for (int nLine = 0; nLine < m_nMultiLineCount; nLine++)
		{
			if (nLine >= m_nMultiLineCount)
			{
				continue;
			}

			int nMinLeft = rectHeader.right;
			int nMaxRight = rectHeader.left;

			if (!rectColumnHeader.IsRectEmpty ())
			{
				nMinLeft = min (rectColumnHeader.left, nMinLeft);
				nMaxRight = max (rectColumnHeader.right, nMaxRight);
			}

			int nMinTop = rectHeader.bottom;
			int nMaxBottom = rectHeader.top;

			int nTop = rectHeader.top + nLine * nLineHeight;
			int nBottom = (nLine == m_nMultiLineCount - 1) ? rectHeader.bottom: nTop + nLineHeight + (bInflateBottom ? 1 : 0);

			nMinTop = min (nTop, nMinTop);
			nMaxBottom = max (nBottom, nMaxBottom);

			int nIndex = m_layout.GetIndex (nColumn, nLine);
			if (nIndex >= 0)
			{
				CHeaderLayout::HeaderLayoutData& data = m_layout.GetData (nIndex);

				if (rectHeader.IsRectEmpty ())
				{
					data.m_rect.SetRectEmpty ();
					continue;
				}

				if (!data.m_rect.IsRectEmpty ())
				{
					nMinLeft = min (data.m_rect.left, nMinLeft);
					nMaxRight = max (data.m_rect.right, nMaxRight);
					nMinTop = min (data.m_rect.top, nMinTop);
					nMaxBottom = max (data.m_rect.bottom, nMaxBottom);
				}

				CRect rectItem = rectHeader;
				if (nMinLeft <= nMaxRight)
				{
					rectItem.left = nMinLeft;
					rectItem.right = nMaxRight;
				}
				if (nMinTop <= nMaxBottom)
				{
					rectItem.top = nMinTop;
					rectItem.bottom = nMaxBottom;
				}

				data.m_rect = rectItem;
			}

		}
	}
}
//******************************************************************************************
void CBCGPGridColumnsInfoEx::PrepareDrawHeader ()
{
	m_layout.CleanPaintedFlags ();
}
//******************************************************************************************
void CBCGPGridColumnsInfoEx::PreparePrintHeader ()
{
	ReposHeaderItems ();
}
//******************************************************************************************
void CBCGPGridColumnsInfoEx::GetColumnPrintRect (int nColumn, CRect& rect) const
{
	if (m_pWndList == NULL)
	{
		ASSERT (FALSE);
		return;
	}
	
	rect.SetRectEmpty ();
	
	BOOL bLast = (nColumn >= m_arrColumns.GetSize ());
	
	if (!bLast && !GetColumnVisible (nColumn))
	{
		return;
	}
	
	// map to printer metrics
	ASSERT_VALID (m_pWndList->GetPrintDC ());
	HDC hDCFrom = ::GetDC(NULL);
	int nXMul = m_pWndList->GetPrintDC ()->GetDeviceCaps(LOGPIXELSX);	// pixels in print dc
	int nXDiv = ::GetDeviceCaps(hDCFrom, LOGPIXELSX);				// pixels in screen dc
	::ReleaseDC(NULL, hDCFrom);

	CSize szOne = m_pWndList->GetPrintParams ().m_pageInfo.m_szOne;
	int nHorzScrollOffset = m_pWndList->GetPrintParams ().m_nHorzScrollOffset;
	CRect rectHeader = m_pWndList->GetPrintParams ().m_rectHeader;

	int nXLeft = rectHeader.left - nHorzScrollOffset;
	
	BOOL bFirst = TRUE;
	int nCount = 0;	// count visible columns
	
	int nPos = Begin ();
	while (nPos != End ())
	{
		int iColumn = Next (nPos);
		if (iColumn == -1)
		{
			break;	// column not found - hidden column
		}
		
		ASSERT (iColumn >= 0);
		ASSERT (iColumn < GetColumnCount ());
		
		int nWidth = GetColumnWidth (iColumn);
		nWidth = ::MulDiv(nWidth, nXMul, nXDiv);

		if (bFirst)
		{
			nWidth += m_pWndList->GetHierarchyOffset () * szOne.cx +
				m_pWndList->GetExtraHierarchyOffset () * szOne.cx;
			bFirst = FALSE;
		}

		if (iColumn == nColumn)
		{
			rect = rectHeader;
			rect.left = nXLeft;
			rect.right = nXLeft + nWidth;
			return;
		}
		
		nXLeft += nWidth;
		nCount++;
	}
	
	ASSERT (bLast);
	
	rect = rectHeader;
	rect.left = nXLeft;
	rect.right = rectHeader.right;
}
//******************************************************************************************
void CBCGPGridColumnsInfoEx::GetHeaderTooltip (int nColumn, CPoint pt, CString& strText) const
{
	if (m_nMultiLineCount < 1)
	{
		CBCGPGridColumnsInfo::GetHeaderTooltip (nColumn, pt, strText);
		return;
	}

	strText.Empty ();
	
	if (m_pWndList->GetSafeHwnd () == NULL)
	{
		return;
	}

	const CRect rectHeader = m_pWndList->GetHeaderRect ();
	if (rectHeader.IsRectEmpty () || !rectHeader.PtInRect (pt))
	{
		return;
	}

	const int nLineHeight = rectHeader.Height () / m_nMultiLineCount; 
	int nLine = (pt.y - rectHeader.top) / nLineHeight;

	CRect rectDummy;
	CBCGPHeaderItem* pItem = NULL;
	if (nLine < m_nMultiLineCount &&
		m_layout.GetHeaderItem (nColumn, nLine, rectDummy, pItem))
	{
		if ((pItem != NULL) ? pItem->m_bNameIsTrancated : GetColumnNameTrancated (nColumn))
		{
			strText = (pItem != NULL) ? pItem->m_strName : GetColumnName (nColumn);
		}
	}
}
//******************************************************************************************
void CBCGPGridColumnsInfoEx::OnColumnsOrderChanged ()
{
	UpdateLayout ();

	CBCGPGridColumnsInfo::OnColumnsOrderChanged ();
}
//******************************************************************************************
void CBCGPGridColumnsInfoEx::DrawColumn (CDC* pDC, int nColumn, CRect rectDraw,
						 int nTextMargin, int nArrowMargin, BOOL bIsPrinting,
						 BOOL bNoSortArrow,
						 BOOL bIsGroupBox)
{
	if (bIsGroupBox || m_bDrawingDraggedColumn || m_nMultiLineCount == 0 || bIsPrinting ||
		nColumn < 0 || nColumn >= m_arrColumns.GetSize () || m_pWndList == NULL)
	{
 		CBCGPGridColumnsInfo::DrawColumn (pDC, nColumn, rectDraw, 
 			nTextMargin, nArrowMargin, bIsPrinting, bNoSortArrow, bIsGroupBox);
		return;
	}

	ASSERT_VALID (m_pWndList);
	CBCGPGridCtrl* pWndList = m_pWndList;

	for (int nLine = 0; nLine < m_nMultiLineCount; nLine++)
	{
		CRect rect;
		CBCGPHeaderItem* pHeaderItem = NULL;

		if (m_layout.GetPaintedFlag (nColumn, nLine) ||
			!m_layout.GetHeaderItem (nColumn, nLine, rect, pHeaderItem) || 
			rect.IsRectEmpty ())
		{
			continue;
		}

		CBCGPHeaderItem hiDefault (nColumn);
		if (nColumn >= 0 && nColumn < m_arrColumns.GetSize ())
		{
			hiDefault.m_strName = GetColumnName (nColumn);
			hiDefault.m_nAlign = GetHeaderAlign (nColumn);
			hiDefault.m_iImage = GetColumnImage (nColumn);
			hiDefault.m_bMultiLine = GetHeaderMultiLine (nColumn);
		}

		CBCGPHeaderItem& hi = (pHeaderItem != NULL) ? *pHeaderItem : hiDefault;
		
		hi.m_nTextMargin = nTextMargin;
		hi.m_nArrowMargin = nArrowMargin;
		hi.m_bIsGroupBox = bIsGroupBox;
		hi.m_bIsHeaderItemDragWnd = m_bDrawingDraggedColumn;
		hi.m_bNoText = !bIsGroupBox;
		hi.m_bNoImage = bIsGroupBox && 
			// ignore bIsGroupBox parameter when dragging from header
			(!m_bDrawingDraggedColumn || pWndList->m_bDragGroupItem || pWndList->m_bDragFromChooser);
		hi.m_bNoSortArrow = bNoSortArrow;
		hi.m_bNoButton = (nColumn == pWndList->m_nDraggedColumn || bIsGroupBox);
		
		if (!bIsGroupBox && !bIsPrinting && GetFirstVisibleColumn () == nColumn && nColumn != -1)
		{
			hi.m_nHierarchyOffset =
				pWndList->GetHierarchyOffset () + pWndList->GetExtraHierarchyOffset ();
		}
		
		pWndList->DrawHeaderItem (pDC, rect, &hi);

		if (pHeaderItem == NULL && nColumn >= 0 && nColumn < m_arrColumns.GetSize ())
		{
			SetColumnNameTrancated (nColumn, hi.m_bNameIsTrancated);
		}

		m_layout.SetPaintedFlag (nColumn, nLine, TRUE);
	}
}
//******************************************************************************************
void CBCGPGridColumnsInfoEx::PrintColumn (CDC* pDC, int nColumn, CRect rectItem, CRect rectClip)
{
	if (m_nMultiLineCount == 0 ||
		nColumn < 0 || nColumn >= m_arrColumns.GetSize () || m_pWndList == NULL)
	{
		CBCGPGridColumnsInfo::PrintColumn (pDC, nColumn, rectItem, rectClip);
		return;
	}

	// map to printer metrics
	HDC hDCFrom = ::GetDC(NULL);
	int nXMul = pDC->GetDeviceCaps(LOGPIXELSX);			// pixels in print dc
	int nXDiv = ::GetDeviceCaps(hDCFrom, LOGPIXELSX);	// pixels in screen dc
	::ReleaseDC(NULL, hDCFrom);

	ASSERT_VALID (m_pWndList);
	CBCGPGridCtrl* pWndList = m_pWndList;

	m_layout.CleanPaintedFlags ();
	
	for (int nLine = 0; nLine < m_nMultiLineCount; nLine++)
	{
		CRect rect;
		CBCGPHeaderItem* pHeaderItem = NULL;
		
		if (m_layout.GetPaintedFlag (nColumn, nLine) ||
			!m_layout.GetHeaderItem (nColumn, nLine, rect, pHeaderItem) || 
			rect.IsRectEmpty ())
		{
			continue;
		}

		CBCGPHeaderItem hiDefault (nColumn);
		if (nColumn >= 0 && nColumn < m_arrColumns.GetSize ())
		{
			hiDefault.m_strName = GetColumnName (nColumn);
			hiDefault.m_nAlign = GetHeaderAlign (nColumn);
			//hiDefault.m_iImage = GetColumnImage (nColumn);
			hiDefault.m_bMultiLine = GetHeaderMultiLine (nColumn);
		}
		
		CBCGPHeaderItem& hi = (pHeaderItem != NULL) ? *pHeaderItem : hiDefault;
		
		hi.m_nTextMargin = ::MulDiv(5, nXMul, nXDiv);
		hi.m_nArrowMargin = ::MulDiv(5, nXMul, nXDiv);
		hi.m_bIsGroupBox = FALSE;
		hi.m_bIsHeaderItemDragWnd = FALSE;
		hi.m_bNoText = FALSE;
		hi.m_bNoImage = TRUE;
		hi.m_bNoSortArrow = FALSE;
		hi.m_bNoButton = TRUE;
		
		pWndList->PrintHeaderItem (pDC, rect, rectClip, &hi);

		m_layout.SetPaintedFlag (nColumn, nLine, TRUE);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPGridColumnsInfoEx::CHeaderLayout - helper class to implement multiline header

void CBCGPGridColumnsInfoEx::CHeaderLayout::SetLayout (int nColumns, int nLines)
{
	ASSERT (nLines >= 0);
	ASSERT (nColumns >= 0);

	m_arrLayout.RemoveAll ();
	m_arrLayoutData.RemoveAll ();

	m_nLines = nLines;
	m_nColumns = nColumns;

	m_arrLayout.SetSize (m_nLines * m_nColumns);
	for (int i = 0; i < m_arrLayout.GetSize (); i++)
	{
		m_arrLayout[i] = -1;
	}
}
//******************************************************************************************
BOOL CBCGPGridColumnsInfoEx::CHeaderLayout::IsLayoutValid (int nColumns, int nLines) const
{
	return	(nColumns == m_nColumns) &&
			(nLines == m_nLines) &&
			(m_nLines >= 0) && 
			(m_nColumns >= 0) &&
			(m_arrLayout.GetSize () == m_nLines * m_nColumns);
}
//******************************************************************************************
int CBCGPGridColumnsInfoEx::CHeaderLayout::GetIndex (int nCol, int nLine) const
{
	if (nCol < 0 || nCol >= m_nColumns || 
		nLine < 0 || nLine >= m_nLines ||
		nCol * m_nLines + nLine >= m_arrLayout.GetSize ())
	{
		ASSERT(FALSE);
		return -1;
	}

	return m_arrLayout [nCol * m_nLines + nLine];
}
//******************************************************************************************
void CBCGPGridColumnsInfoEx::CHeaderLayout::SetIndex (int nCol, int nLine, int nIndex)
{
	if (nCol < 0 || nCol >= m_nColumns || 
		nLine < 0 || nLine >= m_nLines ||
		nCol * m_nLines + nLine >= m_arrLayout.GetSize ())
	{
		ASSERT(FALSE);
		return;
	}

	if (nIndex < 0 || nIndex >= m_arrLayoutData.GetSize ())
	{
		ASSERT(FALSE);
		return;
	}
	
	m_arrLayout [nCol * m_nLines + nLine] = nIndex;
}
//******************************************************************************************
int CBCGPGridColumnsInfoEx::CHeaderLayout::AddData (const HeaderLayoutData& data)
{
	return (int)m_arrLayoutData.Add (data);
}
//******************************************************************************************
CBCGPGridColumnsInfoEx::CHeaderLayout::HeaderLayoutData& CBCGPGridColumnsInfoEx::CHeaderLayout::GetData (int nIndex)
{
	ASSERT(nIndex >= 0);
	ASSERT(nIndex < m_arrLayoutData.GetSize ());
	return m_arrLayoutData [nIndex];
}
//******************************************************************************************
BOOL CBCGPGridColumnsInfoEx::CHeaderLayout::GetHeaderItem (int nCol, int nLine, CRect& rect, CBCGPHeaderItem*& pItem) const
{
	ASSERT(m_arrLayout.GetSize () == m_nLines * m_nColumns);
	rect.SetRectEmpty ();
	pItem = NULL;
	
	int nIndex = GetIndex (nCol, nLine);
	
	if (nIndex < 0 || nIndex >= m_arrLayoutData.GetSize ())
	{
		return FALSE;
	}
	
	rect = m_arrLayoutData [nIndex].m_rect;
	pItem = m_arrLayoutData [nIndex].m_pItem;
	return TRUE;
}
//******************************************************************************************
BOOL CBCGPGridColumnsInfoEx::CHeaderLayout::GetPaintedFlag (int nCol, int nLine) const
{
	ASSERT(m_arrLayout.GetSize () == m_nLines * m_nColumns);
	
	int nIndex = GetIndex (nCol, nLine);
	
	if (nIndex < 0 || nIndex >= m_arrLayoutData.GetSize ())
	{
		ASSERT(FALSE);
		return FALSE;
	}
	
	return m_arrLayoutData [nIndex].m_bPainted;
}
//******************************************************************************************
BOOL CBCGPGridColumnsInfoEx::CHeaderLayout::SetPaintedFlag (int nCol, int nLine, BOOL bPainted)
{
	ASSERT(m_arrLayout.GetSize () == m_nLines * m_nColumns);
	
	int nIndex = GetIndex (nCol, nLine);
	
	if (nIndex < 0 || nIndex >= m_arrLayoutData.GetSize ())
	{
		ASSERT(FALSE);
		return FALSE;
	}
	
	m_arrLayoutData [nIndex].m_bPainted = bPainted;
	return TRUE;
}
//******************************************************************************************
void CBCGPGridColumnsInfoEx::CHeaderLayout::CleanPaintedFlags ()
{
	for (int i = 0; i < m_arrLayoutData.GetSize (); i++)
	{
		m_arrLayoutData[i].m_bPainted = FALSE;
	}
}
//******************************************************************************************
void CBCGPGridColumnsInfoEx::CHeaderLayout::SetRectsEmpty ()
{
	for (int i = 0; i < m_arrLayoutData.GetSize (); i++)
	{
		m_arrLayoutData[i].m_rect.SetRectEmpty ();
	}
}


/////////////////////////////////////////////////////////////////////////////
// CBCGPHeaderItemDragWnd window

class CBCGPHeaderItemDragWnd : public CWnd
{
// Construction
public:
	CBCGPHeaderItemDragWnd();

// Attributes
public:
	CBCGPGridCtrl*		m_pWndGrid;
	int					m_nItem;
	BOOL				m_bDrop;

	static CString		m_strClassName;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPHeaderItemDragWnd)
	public:
	virtual BOOL Create(CBCGPGridCtrl* pGrid, int nItem);
	protected:
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBCGPHeaderItemDragWnd();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPHeaderItemDragWnd)
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CBCGPHeaderItemDropWnd window

class CBCGPHeaderItemDropWnd : public CWnd
{
// Construction
public:
	CBCGPHeaderItemDropWnd();

// Attributes
public:

	static CString	m_strClassName;

// Operations
public:
	void Show (CPoint point);
	void Hide ();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPHeaderItemDropWnd)
	public:
	virtual BOOL Create(int nColumnHeight);
	protected:
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBCGPHeaderItemDropWnd();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPHeaderItemDropWnd)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPGridCtrl

IMPLEMENT_DYNAMIC(CBCGPGridCtrl, CWnd)

#define BCGPGRID_GROUPBYBOX_VMARGIN			7
#define BCGPGRID_GROUPBYBOX_HMARGIN			9
#define BCGPGRID_GROUPBYBOX_VSPACING		10
#define BCGPGRID_GROUPBYBOX_HSPACING		4
#define BCGPGRID_GROUPBYBOX_COLUMNWIDTH		49

HCURSOR CBCGPGridCtrl::m_hcurDeleteColumn = NULL;
HCURSOR CBCGPGridCtrl::m_hcurNoDropColumn = NULL;
BOOL	CBCGPGridCtrl::m_bUseSystemFont = TRUE;

CString CBCGPGridCtrl::m_strFindText;
DWORD CBCGPGridCtrl::m_dwFindMask = FR_DOWN;
DWORD CBCGPGridCtrl::m_dwFindParamsEx = BCGP_GRID_FINDREPLACE_PARAM::FR_LOOKIN_LABELS;

const int nIdToolTipHeader = 1;

/////////////////////////////////////////////////////////////////////////////
// CBCGPGridCtrl notification messages:

UINT BCGM_GRID_ITEM_CHANGED = ::RegisterWindowMessage (_T("BCGM_GRID_ITEM_CHANGED"));
UINT BCGM_GRID_ITEM_BEGININPLACEEDIT = ::RegisterWindowMessage (_T("BCGM_GRID_ITEM_BEGININPLACEEDIT"));
UINT BCGM_GRID_ITEM_ENDINPLACEEDIT = ::RegisterWindowMessage (_T("BCGM_GRID_ITEM_ENDINPLACEEDIT"));
UINT BCGM_GRID_SEL_CHANGED = ::RegisterWindowMessage (_T("BCGM_GRID_SEL_CHANGED"));
UINT BCGM_GRID_ITEM_DBLCLICK = ::RegisterWindowMessage (_T("BCGM_GRID_ITEM_DBLCLICK"));
UINT BCGM_GRID_ON_HIDE_COLUMNCHOOSER = ::RegisterWindowMessage (_T("BCGM_GRID_ON_HIDE_COLUMNCHOOSER"));
UINT BCGM_GRID_BEGINDRAG = ::RegisterWindowMessage (_T("BCGM_GRID_BEGINDRAG"));
UINT BCGM_GRID_COLUMN_CLICK = ::RegisterWindowMessage (_T("BCGM_GRID_COLUMN_CLICK"));
UINT BCGM_GRID_ADJUST_LAYOUT = ::RegisterWindowMessage (_T("BCGM_GRID_ADJUST_LAYOUT"));
UINT BCGM_GRID_FIND_RESULT = ::RegisterWindowMessage (_T("BCGM_GRID_FIND_RESULT"));
UINT BCGM_GRID_COLUMN_BTN_CLICK = ::RegisterWindowMessage (_T("BCGM_GRID_COLUMN_BTN_CLICK"));

CBCGPGridCtrl::CBCGPGridCtrl()
{
	m_nRowsBlockSize = 10; // Specifies the memory-allocation granularity for internal containers
	InitConstructor ();
}

CBCGPGridCtrl::CBCGPGridCtrl(int nMemBlockSize) :
	m_lstItems (nMemBlockSize), m_lstTerminalItems (nMemBlockSize), m_lstGroupedItems (nMemBlockSize),
	m_lstAutoGroups (nMemBlockSize), m_lstOldAutoGroups (nMemBlockSize)
{
	m_nRowsBlockSize = nMemBlockSize;
	InitConstructor ();
}

void CBCGPGridCtrl::InitConstructor ()
{
	m_bScrollVert = TRUE;
	m_bScrollHorz = TRUE;
	m_bScrollHorzShowAlways = FALSE;
	m_fScrollRemainder = 0;
	m_nMouseWheelSmoothScrollMaxLimit = 50;
	m_bFreezeGroups = FALSE;
	m_hFont = NULL;
	m_nEditLeftMargin = 0;
	m_nEditRightMargin = 0;
	m_bTrimTextLeft = TRUE;
	m_nLeftItemBorderOffset = 0;
	m_bHeader = TRUE;
	m_bHeaderSelectAllMarker = TRUE;
	m_dwHeaderFlags = (DWORD)-1;
	m_bRowHeader = FALSE;
	m_dwRowHeaderFlags = (DWORD)-1;
	m_bGroupByBox = FALSE;
	m_bVirtualMode = FALSE;
	m_nVirtualRows = 0;
	m_nMultiLineExtraRows = 0;
	m_nMultiLineExtraRowsCount = 0;
	m_pfnCallback = NULL;
	m_lParamCallback = 0;
	m_rectList.SetRectEmpty ();
	m_rectClip.SetRectEmpty ();
	m_rectTrackHeader.SetRectEmpty ();
	m_rectTrackHeaderLeft.SetRectEmpty ();
	m_rectHeader.SetRectEmpty ();
	m_rectRowHeader.SetRectEmpty ();
	m_rectSelectAllArea.SetRectEmpty ();
	m_rectFilterBar.SetRectEmpty ();

	m_pToolTip = NULL;

	m_pImagesHeader = NULL;
	m_pImagesHeaderBtn = NULL;
	m_pImages = NULL;

	m_nBaseHeight = 0;
	m_nRowHeight = 0;
	m_nLargeRowHeight = 0;
	m_bAllowRowExtraHeight = FALSE;
	m_nGroupByBoxHeight = 0;
	m_nRowHeaderWidth = 0;
	m_nButtonWidth = 0;
	m_nGridHeaderHeight = 0;
	m_nGridFooterHeight = 0;
	m_nTreeColumn = -1;
	m_nVertScrollOffset = 0;
	m_nVertScrollTotal = 0;
	m_nVertScrollPage = 0;
	m_nHorzScrollOffset = 0;
	m_nHorzScrollTotal = 0;
	m_nHorzScrollPage = 0;
	m_nFirstVisibleItem = -1;

	m_pDefaultItemRTC = NULL;

	m_pSelRow = NULL;
	m_pSelItem = NULL;
	m_pLastSelRow = NULL;
	m_pLastSelItem = NULL;
	m_pSetSelItem = NULL;
	m_rectClipSel.SetRectEmpty ();
	m_bFocused = FALSE;
	m_bTracking = FALSE;
	m_nTrackColumn = 0;

	m_bMouseTracked = FALSE;
	m_bHeaderItemHovered = FALSE;

	m_bUpdateItemData = TRUE;
	m_bClearInplaceEditOnEnter = TRUE;
	m_dwEndEditResult = EndEdit_NoResult;
	m_dwBeginEditReason = BeginEdit_None;

	memset (&m_CurrentItemInfo, 0, sizeof (BCGPGRID_ITEM_INFO));

	m_strTrue = _T("True");
	m_strFalse = _T("False");

	m_cListDelimeter = _T(',');

	m_bReadOnly = FALSE;
	m_bSelectionBorder = TRUE;
	m_bEditFirstClick = TRUE;
	m_bHighlightGroups = FALSE;
	m_bControlBarColors = FALSE;
	m_bShowDragContext = TRUE;
	m_bSingleSel = FALSE;
	m_bWholeRowSel = FALSE;
	m_bInvertSelOnCtrl = FALSE;
	m_bMarkSortedColumn = FALSE;
	m_bDrawFocusRect = FALSE;
	m_bShowInPlaceToolTip = TRUE;
	m_bRowMarker = TRUE;
	m_bLineNumbers = FALSE;
	m_bGridLines = TRUE;
	m_bSelectionBorderActiveItem = TRUE;
	m_bHighlightActiveItem = FALSE;
	m_bGridItemBorders = TRUE;	// TODO FALSE by default
	m_bUseQuickSort = TRUE;
	m_bSelecting = FALSE;
	m_bClickTimer = FALSE;
	m_ptClickOnce = CPoint (0, 0);
	m_bIsFirstClick = FALSE;
	m_bIsButtonClick = FALSE;
	m_bHeaderRowSelecting = FALSE;
	m_bHeaderColSelecting = FALSE;
	m_bRebuildTerminalItems = TRUE;
	m_bNoUpdateWindow = FALSE;

	m_clrSortedColumn = (COLORREF)-1;

	m_hPrinterFont = NULL;
	m_hPrinterBoldFont = NULL;
	m_hMirrorFont = NULL;
	m_hMirrorBoldFont = NULL;
	m_bIsPrinting = FALSE;
	m_pPrintDC = NULL;
	
	m_clrPrintBorder = (COLORREF)-1;
	m_clrPrintHeader = (COLORREF)-1;
	m_clrPrintHeaderBG = (COLORREF)-1;
	m_clrPrintGroup = (COLORREF)-1;
	m_clrPrintGroupBG = (COLORREF)-1;
	m_clrPrintLeftOffset = (COLORREF)-1;
	m_clrPrintBG = (COLORREF)-1;
	m_clrPrintLine = (COLORREF)-1;
	m_clrPrintText = (COLORREF)-1;

	m_bBreakColumnsAcrossPrintPages = TRUE;

	m_bInAdjustLayout = FALSE;
	m_bPostAdjustLayout = FALSE;
	m_bIgnoreShiftBtn = FALSE;
	m_bIgnoreCtrlBtn = FALSE;

	m_pWndHeaderDrag = NULL;
	m_pWndHeaderDrop = NULL;

	m_pColumnChooser = NULL;
	m_rectColumnChooser.SetRectEmpty ();
	m_bColumnChooserVisible = FALSE;
	m_bFieldChooserThemed = FALSE;

	m_bDragHeaderItems = TRUE;
	m_nDraggedColumn = -1;
	m_rectStartDrag.SetRectEmpty ();
	m_ptStartDrag = CPoint (0, 0);
	m_bDragGroupItem = FALSE;
	m_bDragFromChooser = FALSE;

	m_bDragSelection = FALSE;
	m_bDragSelectionBorder = FALSE;
	m_bDragDrop = FALSE;
	m_bDragEnter = FALSE;
	m_bDragRowHeader = FALSE;
	m_idDragFrom = CBCGPGridItemID (0, 0);
	m_idDropTo = CBCGPGridItemID (0, 0);
	m_DropEffect = DROPEFFECT_NONE;
	m_DropArea = DropAt;
	m_rectDragFrame.SetRectEmpty ();
	m_rectDragMarker.SetRectEmpty ();
	m_bCut = FALSE;

	m_pSerializeManager = NULL;
	
	m_bFilter = FALSE;
	m_pfnFilterCallback = NULL;
	m_lFilterParam = 0;
	m_pDefaultFilter = NULL;
	m_bDefaultFilterMenuPopup = FALSE;
	m_uiDefaultFilterMenuResId = 0;
	m_uiDefaultFilterApplyCmd = 0;
	m_bFilterBar = FALSE;
	m_bNoFilterBarUpdate = FALSE;
	m_nFocusedFilter = -1;

	m_pFindDlg = NULL;

	m_aSortOrder = NULL;
	m_nSortCount = 0;
	m_nGroupCount = 0;

	SetScrollBarsStyle (CBCGPScrollBar::BCGP_SBSTYLE_DEFAULT);

	m_bVisualManagerStyle = FALSE;

	m_strExportCSVSeparator = _T(";"); // ";" or ","

	m_pAccRow = NULL;
	m_pAccItem = NULL;

	m_pGM = NULL;
}

CBCGPGridCtrl::~CBCGPGridCtrl()
{
	if (m_pColumnChooser->GetSafeHwnd ())
	{
		m_pColumnChooser->DestroyWindow ();
		m_pColumnChooser = NULL;
	}

	if (m_pWndHeaderDrag != NULL)
	{
		m_pWndHeaderDrag->DestroyWindow ();
		m_pWndHeaderDrag = NULL;
	}

	if (m_pWndHeaderDrop != NULL)
	{
		m_pWndHeaderDrop->DestroyWindow ();
		m_pWndHeaderDrop = NULL;
	}
	
	if (m_pSerializeManager != NULL)
	{
		delete m_pSerializeManager;
		m_pSerializeManager = NULL;
	}

	if (m_pDefaultFilter != NULL)
	{
		delete m_pDefaultFilter;
		m_pDefaultFilter = NULL;
	}

	if (m_pGM != NULL)
	{
		delete m_pGM;
	}
}

BEGIN_MESSAGE_MAP(CBCGPGridCtrl, CWnd)
	//{{AFX_MSG_MAP(CBCGPGridCtrl)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_CANCELMODE()
	ON_WM_KILLFOCUS()
	ON_WM_GETDLGCODE()
	ON_WM_VSCROLL()
	ON_WM_HSCROLL()
	ON_WM_MOUSEWHEEL()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_SETCURSOR()
	ON_WM_KEYDOWN()
	ON_WM_CHAR()
	ON_WM_SETFOCUS()
	ON_WM_CTLCOLOR()
	ON_WM_DESTROY()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_NCCALCSIZE()
	ON_WM_NCPAINT()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_SYSCOLORCHANGE()
	ON_WM_SETTINGCHANGE()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_SETFONT, OnSetFont)
	ON_MESSAGE(WM_GETFONT, OnGetFont)
	ON_WM_STYLECHANGED()
	ON_CBN_SELENDOK(BCGPGRIDCTRL_ID_INPLACE, OnSelectCombo)
	ON_CBN_CLOSEUP(BCGPGRIDCTRL_ID_INPLACE, OnCloseCombo)
	ON_EN_KILLFOCUS(BCGPGRIDCTRL_ID_INPLACE, OnEditKillFocus)
	ON_CBN_KILLFOCUS(BCGPGRIDCTRL_ID_INPLACE, OnComboKillFocus)
	ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
	ON_MESSAGE(WM_PRINTCLIENT, OnPrintClient)
	ON_REGISTERED_MESSAGE(BCGM_GRID_ADJUST_LAYOUT, OnGridAdjustLayout)
	ON_REGISTERED_MESSAGE(BCGM_UPDATETOOLTIPS, OnBCGUpdateToolTips)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXT, 0, 0xFFFF, OnNeedTipText)
	ON_COMMAND(ID_DEFAULTFILTER_APPLY, OnDefaultFilterMenuApply)
	ON_COMMAND(ID_FILTERBAR_BUTTON, OnFilterBarClearAll)
	ON_REGISTERED_MESSAGE(WM_FINDREPLACE, OnFindReplace)
	ON_REGISTERED_MESSAGE(BCGM_ONSETCONTROLVMMODE, OnBCGSetControlVMMode)
	ON_MESSAGE (WM_GETOBJECT, OnGetObject)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Virtual mode methods:

void CBCGPGridCtrl::EnableVirtualMode (BCGPGRID_CALLBACK pCallback/* = NULL*/, 
									   LPARAM lParam/* = 0*/)
{
	m_pfnCallback = pCallback;
	m_lParamCallback = lParam;

	if (m_bVirtualMode)
	{
		// Already in virtual mode
		return;
	}

	if (!m_lstItems.IsEmpty ())
	{
		// you cannot set virtual mode after items were added
		ASSERT (FALSE);
		return;		
	}

	m_bVirtualMode = TRUE;
	m_nVirtualRows = 0;
}
//******************************************************************************************
void CBCGPGridCtrl::SetVirtualRows (int nRowsNum)
{
	if (!m_bVirtualMode)
	{
		ASSERT (FALSE);
		return;
	}

	RemoveAll ();

	m_nVirtualRows = nRowsNum;

	AdjustLayout ();
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPGridCtrl message handlers

void CBCGPGridCtrl::PreSubclassWindow() 
{
	CBCGPWnd::PreSubclassWindow();

	_AFX_THREAD_STATE* pThreadState = AfxGetThreadState ();
	if (pThreadState->m_pWndInit == NULL)
	{
		Init ();
	}
}
//******************************************************************************************
int CBCGPGridCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CBCGPWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_DropTarget.Register (this);

#ifndef _BCGPGRID_STANDALONE
  	CBCGPTooltipManager::CreateToolTip (m_pToolTip, this, BCGP_TOOLTIP_TYPE_GRID);
#else
	m_pToolTip = new CToolTipCtrl;
	m_pToolTip->Create (this);
#endif

	if (m_pToolTip->GetSafeHwnd () != NULL)
	{
		CRect rectDummy (0, 0, 0, 0);
		m_pToolTip->AddTool (this, LPSTR_TEXTCALLBACK, &rectDummy, nIdToolTipHeader);
	}

	Init ();
	return 0;
}
//******************************************************************************************
void CBCGPGridCtrl::Init ()
{
	CBCGPGestureConfig gestureConfig;
	gestureConfig.EnablePan(TRUE, BCGP_GC_PAN_WITH_SINGLE_FINGER_VERTICALLY | BCGP_GC_PAN_WITH_SINGLE_FINGER_HORIZONTALLY | BCGP_GC_PAN_WITH_GUTTER | BCGP_GC_PAN_WITH_INERTIA);

	bcgpGestureManager.SetGestureConfig(GetSafeHwnd(), gestureConfig);

	CRect rectDummy;
	rectDummy.SetRectEmpty ();

	if (globalData.m_hcurStretch == NULL)
	{
		globalData.m_hcurStretch = AfxGetApp ()->LoadCursor (AFX_IDC_HSPLITBAR);
	}

	if (globalData.m_hcurStretchVert == NULL)
	{
		globalData.m_hcurStretchVert = AfxGetApp ()->LoadCursor (AFX_IDC_VSPLITBAR);
	}

	if (m_hcurDeleteColumn == NULL)
	{
		CBCGPLocalResource locaRes;
		m_hcurDeleteColumn = AfxGetApp ()->LoadCursor (IDC_BCGBARRES_DELETE_COLUMN);
	}

	if (m_hcurNoDropColumn == NULL)
	{
		m_hcurNoDropColumn = AfxGetApp ()->LoadCursor (AFX_IDC_NODROPCRSR);
	}

	if (m_StateImages.GetCount() == 0)
	{
		CBCGPLocalResource locaRes;

		m_StateImages.SetImageSize(CSize(14, 14));
		m_StateImages.Load(IDB_BCGBARRES_GRID_STATE_ICONS32);
	}

	InitColors ();

	GetColumnsInfo ().SetOwnerList (this);

	m_wndScrollVert.Create (WS_CHILD | WS_VISIBLE | SBS_VERT, rectDummy, this, ID_SCROLL_VERT);
	m_wndScrollHorz.Create (WS_CHILD | WS_VISIBLE | SBS_HORZ, rectDummy, this, ID_SCROLL_HORZ);

	m_ToolTip.Create (this);

	CreateSerializeManager ();

	CWnd* pWndParent = GetParent ();
	m_bControlBarColors = pWndParent == NULL ||
		!pWndParent->IsKindOf (RUNTIME_CLASS (CDialog));

	AdjustLayout ();
	UpdateFonts ();
	CalcEditMargin ();
}
//******************************************************************************************
void CBCGPGridCtrl::InitColors ()
{
	CleanUpColors ();

	m_clrSortedColumn = visualManager->GetGridItemSortedColor (this);

	if (m_ColorData.m_clrBackground != (COLORREF)-1)
	{
		m_brBackground.CreateSolidBrush (m_ColorData.m_clrBackground);
	}

	if (m_ColorData.m_GroupColors.m_clrBackground != (COLORREF)-1)
	{
		m_brGroupBackground.CreateSolidBrush (m_ColorData.m_GroupColors.m_clrBackground);
	}

	if (m_ColorData.m_SelColors.m_clrBackground != (COLORREF)-1)
	{
		m_brSelBackground.CreateSolidBrush (m_ColorData.m_SelColors.m_clrBackground);
	}
	
	m_penHLine.CreatePen (PS_SOLID, 1, 
		m_ColorData.m_clrHorzLine != (COLORREF)-1 ? m_ColorData.m_clrHorzLine : globalData.clrBtnFace);

	m_penVLine.CreatePen (PS_SOLID, 1, 
		m_ColorData.m_clrVertLine != (COLORREF)-1 ? m_ColorData.m_clrVertLine : globalData.clrBtnFace);

	m_penDrag.CreatePen (PS_SOLID, 1, globalData.clrBtnText);
}
//*****************************************************************************************
void CBCGPGridCtrl::CleanUpColors ()
{
	if (m_brBackground.GetSafeHandle () != NULL)
	{
		m_brBackground.DeleteObject ();
	}
	if (m_brGroupBackground.GetSafeHandle () != NULL)
	{
		m_brGroupBackground.DeleteObject ();
	}
	if (m_brSelBackground.GetSafeHandle () != NULL)
	{
		m_brSelBackground.DeleteObject ();
	}
	if (m_penHLine.GetSafeHandle () != NULL)
	{
		m_penHLine.DeleteObject ();
	}
	if (m_penVLine.GetSafeHandle () != NULL)
	{
		m_penVLine.DeleteObject ();
	}
	if (m_penDrag.GetSafeHandle () != NULL)
	{
		m_penDrag.DeleteObject ();
	}
}
//*****************************************************************************************
void CBCGPGridCtrl::OnSysColorChange() 
{
	CBCGPWnd::OnSysColorChange();
	
	InitColors ();
	RedrawWindow ();
}
//*****************************************************************************************
void CBCGPGridCtrl::AdjustLayout ()
{
	if (GetSafeHwnd () == NULL || m_bInAdjustLayout)
	{
		return;
	}

	m_bInAdjustLayout = TRUE;
	
	if (m_bRebuildTerminalItems)
	{
		ReposItems ();
	}

	SetRowHeight ();
	SetRowHeaderWidth ();

	CRect rectClient;
	GetClientRect (rectClient);

	m_nGroupByBoxHeight = OnGetGroupByBoxRect (NULL, rectClient).Height ();
	m_rectHeader = OnGetHeaderRect (NULL, rectClient);
	m_rectFilterBar = OnGetFilterBarRect (NULL, rectClient);
	m_rectRowHeader = OnGetRowHeaderRect (NULL, rectClient);
	m_rectSelectAllArea = OnGetSelectAllAreaRect (NULL, rectClient);
	m_rectList = OnGetGridRect (NULL, rectClient);

	GetColumnsInfo ().Resize (m_rectList.Width ());

	int cxScroll = m_bScrollVert ? ::GetSystemMetrics (SM_CXVSCROLL) : 0;
	int cyScroll = m_bScrollHorz ? ::GetSystemMetrics (SM_CYHSCROLL) : 0;
	SetScrollSizes ();

	CSize szScroll (0, 0);
	CPoint ptScroll (rectClient.left, rectClient.top);
	if (m_nVertScrollTotal > 0)
	{
		m_rectList.right -= cxScroll;
		m_rectHeader.right -= cxScroll;
		szScroll.cy = rectClient.Height ();

		if (GetColumnsInfo ().IsAutoSize ())
		{
			GetColumnsInfo ().Resize (m_rectList.Width ());
		}
	}
	if (m_nHorzScrollTotal > 0 || m_bScrollHorzShowAlways)
	{
		m_rectRowHeader.bottom -= cyScroll;
		m_rectList.bottom -= cyScroll;
		szScroll.cx = max (0, m_rectRowHeader.Width () + m_rectList.Width ());
		szScroll.cy = max (0, szScroll.cy - cyScroll);
	}

	m_wndScrollVert.EnableScrollBar (szScroll.cy > 0 ? ESB_ENABLE_BOTH : ESB_DISABLE_BOTH);

	if (szScroll.cy > 0)
	{
		m_wndScrollVert.SetWindowPos (NULL, m_rectList.right, ptScroll.y,
			cxScroll, szScroll.cy, SWP_NOZORDER | SWP_NOACTIVATE);
	}
	else
	{
		m_wndScrollVert.SetWindowPos (NULL, 0, 0,
			0, 0, SWP_NOZORDER | SWP_NOACTIVATE);
	}
	
	if (szScroll.cx > 0)
	{
		m_wndScrollHorz.SetWindowPos (NULL, ptScroll.x, rectClient.bottom - cyScroll,
			szScroll.cx, cyScroll, SWP_NOZORDER | SWP_NOACTIVATE);
	}
	else
	{
		m_wndScrollHorz.SetWindowPos (NULL, 0, 0,
			0, 0, SWP_NOZORDER | SWP_NOACTIVATE);
	}

	OnResizeColumns ();

	if (m_pToolTip->GetSafeHwnd () != NULL)
	{
		m_pToolTip->SetToolRect (this, nIdToolTipHeader, m_rectHeader);
	}

	ReposItems ();

	OnPosSizeChanged ();
	m_rectTrackSel = OnGetSelectionRect ();

	CBCGPGridRow* pCurSel = GetCurSel ();
	if (pCurSel != NULL)
	{
		pCurSel->AdjustButtonRect ();
	}

	AdjustFilterBarCtrls ();
	RedrawWindow ();

	m_bInAdjustLayout = FALSE;
}
//******************************************************************************************
void CBCGPGridCtrl::OnPosSizeChanged ()
{
}
//******************************************************************************************
void CBCGPGridCtrl::SetRowHeight ()
{
	if (m_bIsPrinting)
	{
		ASSERT_VALID (m_pPrintDC);

		// map to printer metrics
		HDC hDCFrom = ::GetDC(NULL);
		int nYMul = m_pPrintDC->GetDeviceCaps(LOGPIXELSY);	// pixels in print dc
		int nYDiv = ::GetDeviceCaps(hDCFrom, LOGPIXELSY);	// pixels in screen dc
		::ReleaseDC(NULL, hDCFrom);

		TEXTMETRIC tm;
		m_pPrintDC->GetTextMetrics (&tm);
		m_PrintParams.m_nBaseHeight = tm.tmHeight + ::MulDiv (2 * TEXT_VMARGIN, nYMul, nYDiv);
		m_PrintParams.m_nRowHeight = m_PrintParams.m_nBaseHeight;
		m_PrintParams.m_nLargeRowHeight = m_PrintParams.m_nBaseHeight;
		m_PrintParams.m_nButtonWidth = m_PrintParams.m_nBaseHeight;
	}
	else
	{
		CClientDC dc (this);
		HFONT hfontOld = SetCurrFont (&dc);

		TEXTMETRIC tm;
		dc.GetTextMetrics (&tm);
		m_nBaseHeight = tm.tmHeight + 2 * TEXT_VMARGIN;
		m_nRowHeight = m_nBaseHeight;
		m_nLargeRowHeight = m_nBaseHeight;
		m_nButtonWidth = m_nBaseHeight;

		::SelectObject (dc.GetSafeHdc (), hfontOld);
	}
}
//******************************************************************************************
void CBCGPGridCtrl::SetRowHeaderWidth ()
{
	if (m_bIsPrinting)
	{
		ASSERT_VALID (m_pPrintDC);

		// map to printer metrics
		HDC hDCFrom = ::GetDC(NULL);
		int nXMul = m_pPrintDC->GetDeviceCaps(LOGPIXELSX);	// pixels in print dc
		int nXDiv = ::GetDeviceCaps(hDCFrom, LOGPIXELSX);	// pixels in screen dc
		::ReleaseDC(NULL, hDCFrom);

		if (m_nRowHeaderWidth == 0)
		{
			m_PrintParams.m_nRowHeaderWidth = ::MulDiv (30, nXMul, nXDiv);;
		}
		else
		{
			m_PrintParams.m_nRowHeaderWidth = ::MulDiv (m_nRowHeaderWidth, nXMul, nXDiv);;
		}
	}
	else
	{
		if (m_nRowHeaderWidth == 0)
		{
			m_nRowHeaderWidth = 30;
		}
	}
}
//******************************************************************************************
void CBCGPGridCtrl::ReposItems ()
{
	ASSERT_VALID (this);

	m_nFirstVisibleItem = -1;

	if (m_bVirtualMode)
	{
		// iterate through cached items
		for (POSITION pos = m_CachedItems.m_lstCache.GetHeadPosition (); pos != NULL; )
		{
			CBCGPGridCachePageInfo& cpi = m_CachedItems.m_lstCache.GetNext (pos);
			ASSERT (cpi.pArrCachePage->GetSize() == cpi.nSize);
			for (int i = 0; i < cpi.nSize; i++)
			{
				CBCGPGridRow* pItem = cpi.pArrCachePage->GetAt (i);
				if (pItem != NULL)
				{
					ASSERT_VALID (pItem);

					int nIndex = cpi.nFirst + i;
					int y = m_rectList.top - 1 + nIndex * m_nRowHeight - m_nVertScrollOffset;
					m_idCur.m_nRow = nIndex;
					pItem->Repos (y);

					if (m_nFirstVisibleItem == -1 &&
						pItem->GetRect ().bottom >= m_rectList.top)
					{
						m_nFirstVisibleItem = nIndex;
					}
				}
			}
		}

		UpdateMergedItems ();
		return;
	}

	if (m_bRebuildTerminalItems)
	{
		// Cleanup selection
		CBCGPGridItemID id;
		SetCurSel (id, SM_NONE, FALSE);
		m_pLastSelItem = NULL;

		CleanUpAutoGroups (AG_COPY_AUTOGROUPS);
		m_lstTerminalItems.RemoveAll ();
	}

	m_ToolTip.Hide ();

	m_idCur.m_nRow = 0;
	m_idCur.m_nColumn = -1;

	int y = m_rectList.top - m_nVertScrollOffset - 1;
	int nIndex = 0;

	if (!IsSortingMode () && !IsGrouping ())
	{
		m_bRebuildTerminalItems = FALSE;

		for (POSITION pos = m_lstItems.GetHeadPosition (); pos != NULL;
			nIndex++)
		{
			CBCGPGridRow* pItem = m_lstItems.GetNext (pos);
			ASSERT_VALID (pItem);

			pItem->Repos (y);

			if (m_nFirstVisibleItem == -1 &&
				pItem->GetRect ().bottom >= m_rectList.top)
			{
				m_nFirstVisibleItem = nIndex;
			}
		}

		UpdateMergedItems ();
		return;
	}

	if (m_bRebuildTerminalItems)
	{
		// Get sorted list of terminal items:
		DoRebuildTerminalItems ();
	}

	m_bRebuildTerminalItems = FALSE;

	m_nFirstVisibleItem = -1;
	nIndex = 0;

	for (POSITION pos = m_lstTerminalItems.GetHeadPosition (); pos != NULL;
		nIndex++)
	{
		CBCGPGridRow* pItem = m_lstTerminalItems.GetNext (pos);
		ASSERT_VALID (pItem);

		pItem->Repos (y);

		if (m_nFirstVisibleItem == -1 &&
			pItem->GetRect ().bottom >= m_rectList.top)
		{
			m_nFirstVisibleItem = nIndex;
		}
	}

	UpdateMergedItems ();
}
//******************************************************************************************
void CBCGPGridCtrl::ShiftItems (int dx, int dy)
{
	m_ToolTip.Deactivate ();

	m_nFirstVisibleItem = -1;

	if (m_bVirtualMode)
	{
		// iterate through cached items
		for (POSITION pos = m_CachedItems.m_lstCache.GetHeadPosition (); pos != NULL; )
		{
			CBCGPGridCachePageInfo& cpi = m_CachedItems.m_lstCache.GetNext (pos);
			ASSERT (cpi.pArrCachePage->GetSize() == cpi.nSize);
			for (int i = 0; i < cpi.nSize; i++)
			{
				CBCGPGridRow* pItem = cpi.pArrCachePage->GetAt (i);
				if (pItem != NULL)
				{
					ASSERT_VALID (pItem);

					pItem->Shift (dx, dy);

					if (m_nFirstVisibleItem == -1 &&
						pItem->GetRect ().bottom >= m_rectList.top)
					{
						int nIndex = cpi.nFirst + i;
						m_nFirstVisibleItem = nIndex;
					}
				}
			}
		}

		UpdateMergedItems ();
		return;
	}

	const CList<CBCGPGridRow*, CBCGPGridRow*>& lst =
		(!IsSortingMode () && !IsGrouping ()) ? m_lstItems : m_lstTerminalItems;

	POSITION pos = NULL;

	int nIndex = 0;

	for (pos = lst.GetHeadPosition (); pos != NULL; nIndex++)
	{
		CBCGPGridRow* pItem = lst.GetNext (pos);
		if (pItem == NULL)
		{
			continue;
		}

		ASSERT_VALID (pItem);

		pItem->Shift (dx, dy);

		if (m_nFirstVisibleItem == -1 &&
			pItem->GetRect ().bottom >= m_rectList.top)
		{
			m_nFirstVisibleItem = nIndex;
		}
	}

	UpdateMergedItems ();
}
//******************************************************************************************
// Get sorted list of terminal items:
void CBCGPGridCtrl::DoRebuildTerminalItems ()
{
	if (GetColumnsInfo ().GetGroupColumnCount () + GetColumnsInfo ().GetSortColumnCount () <= 0)
	{
		return;
	}

	//----------------------------
	// old version: insertion sort
	//----------------------------
	if (!m_bUseQuickSort)
	{
		for (POSITION pos = m_lstItems.GetHeadPosition (); pos != NULL;)
		{
			CBCGPGridRow* pItem = m_lstItems.GetNext (pos);
			ASSERT_VALID (pItem);

			pItem->m_nIdRow = -1;

			if (IsGrouping ())
			{
				pItem->AddGroupedItem (m_lstTerminalItems);
			}
			else
			{
				pItem->AddTerminalItem (m_lstTerminalItems);
			}
		}

		CleanUpAutoGroups (AG_CLEANUP_OLDAUTOGROUPS_ONLY);
	}

	//-------------------
	// Quick-sort version
	//-------------------
	else
	{
		CBCGPSortableArray arrSort;
		arrSort.SetSize (m_lstItems.GetCount ());

		//-------------
		// copy buffer:
		//-------------
		int i = 0;
		for (POSITION pos = m_lstItems.GetHeadPosition (); pos != NULL; i++)
		{
			CBCGPGridRow* pItem = m_lstItems.GetNext (pos);
			ASSERT_VALID (pItem);

			arrSort[i] = (pItem);
		}
		
		//----------------
		// Get sort order:
		//----------------
		const int nGroupCount = GetColumnsInfo ().GetGroupColumnCount ();
		const int nSortCount = GetColumnsInfo ().GetGroupColumnCount () + GetColumnsInfo ().GetSortColumnCount ();
		int* aSortOrder = new int [nSortCount];
		memset (aSortOrder, 0, nSortCount * sizeof (int));

		if (!GetColumnsInfo ().GetGroupingColumnOrderArray ((LPINT) aSortOrder, nSortCount))
		{
			ASSERT (FALSE);	// error getting sort order

			delete [] aSortOrder;
			return;
		}
		SetSortOrder (aSortOrder, nSortCount, nGroupCount);

		//-----
		// Sort
		//-----
		arrSort.Sort ();

		//--------------------
		// Copy results, group
		//--------------------
		CArray <POSITION, POSITION> arrPositions; // first item in group
		arrPositions.SetSize (nGroupCount);
		for (i = 0; i < arrPositions.GetSize (); i++)
		{
			arrPositions [i] = NULL;
		}

		CList<CBCGPGridRow*, CBCGPGridRow*>& lstItems = m_lstTerminalItems;
		for (i = 0; i < arrSort.GetSize (); i++)
		{
			CBCGPGridRow* pRowLast = (lstItems.IsEmpty ()) ? NULL : lstItems.GetTail ();

			POSITION posInserted = lstItems.AddTail (arrSort [i]);
			int nParentLevel = -1;
			POSITION posFirstItemInGroup = NULL;

			if (pRowLast == NULL) // list empty
			{
				// nParentLevel == -1 - create full hierarchy
				// this is the first item in group for all levels
				for (int j = 0; j < nGroupCount; j++)
				{
					arrPositions [j] = posInserted;
				}
				// posFirstItemInGroup == NULL
			}
			else
			{
				ASSERT_VALID (pRowLast);

				// find out nParentLevel:
				BOOL bTryNextSortedColumn = TRUE;
				int iLevel = 0;			// 0, ..., nGroupCount - 1
				while (bTryNextSortedColumn && iLevel < nGroupCount)
				{
					int iSortedColumn = aSortOrder [iLevel];
					int nCompare = CompareGroup (pRowLast, arrSort [i], iSortedColumn);
					if (nCompare == 0) // the same group
					{
						nParentLevel = iLevel;
						iLevel++;
					}
					else
					{
						bTryNextSortedColumn = FALSE;
					}
				}

				// this is the first item in group for next levels
				for (int j = max (0, nParentLevel + 1); j < nGroupCount; j++)
				{
					arrPositions [j] = posInserted;
				}
				// get FirstItemInGroup for nParentLevel
				if (nParentLevel >= 0 && nParentLevel < nGroupCount)
				{
					posFirstItemInGroup = arrPositions [nParentLevel];
				}
			}

			POSITION posInsertBefore = posInserted;	// insert autogroups before item
			POSITION posGroup = arrSort [i]->InsertAutoGroupBefore (lstItems, posInsertBefore, posFirstItemInGroup, nParentLevel, arrSort [i]);

			// Add item as subitem for the group
			if (posGroup != NULL)
			{
				CBCGPGridRow* pGroup = lstItems.GetAt (posGroup);
				BOOL bIsAutoGroup = pGroup != NULL && pGroup->IsGroup () && (pGroup->m_dwFlags & BCGP_GRID_ITEM_AUTOGROUP) != 0;
				if (bIsAutoGroup)
				{
					ASSERT_VALID (pGroup);

					// Add item as subitem for the group
					pGroup->m_lstSubItems.AddTail (arrSort [i]);
					arrSort [i]->SetParent(pGroup);
				}
			}
		}
		
		SetSortOrder (NULL, 0, 0);
		arrSort.RemoveAll ();
		delete [] aSortOrder;

		CleanUpAutoGroups (AG_CLEANUP_OLDAUTOGROUPS_ONLY);
	}
}
//******************************************************************************************
void CBCGPGridCtrl::SetSortOrder (int* aSortOrder, int nSortCount, int nGroupCount)
{
	ASSERT_VALID (this);
	m_aSortOrder = aSortOrder;
	m_nSortCount = nSortCount;
	m_nGroupCount = nGroupCount;
}
//******************************************************************************************
BOOL CBCGPGridCtrl::GetSortOrder (int*& aSortOrder, int& nSortCount, int& nGroupCount) const
{
	ASSERT_VALID (this);
	aSortOrder = m_aSortOrder;
	nSortCount = m_nSortCount;
	nGroupCount = m_nGroupCount;

	return (aSortOrder != NULL);
}
//****************************************************************************************
// Called from CBCGPSortableArray::Compare
int CBCGPGridCtrl::DoMultiColumnCompare (const CBCGPGridRow* pRow1, const CBCGPGridRow* pRow2)
{
	int* aSortOrder = NULL;
	int nSortCount = 0;
	int nGroupCount = 0;
	if (!GetSortOrder (aSortOrder, nSortCount, nGroupCount))
	{
		return 0;
	}

	if (nSortCount <= 0)
	{
		return 0;
	}

	ASSERT (aSortOrder != NULL);
	ASSERT (AfxIsValidAddress (aSortOrder, nSortCount * sizeof(int)));

	int nCompare = 0;

	BOOL bTryNextSortedColumn = TRUE;
	int iLevel = 0;			// 0, ..., nSortCount - 1
	while (bTryNextSortedColumn && iLevel < nSortCount)
	{
		int iSortedColumn = aSortOrder [iLevel];

		if (iLevel < nGroupCount) // grouping
		{
			nCompare = CompareGroup (pRow1, pRow2, iSortedColumn);
		}
		else	// compare items inside same group
		{
			nCompare = CompareItems (pRow1, pRow2, iSortedColumn);
		}
		if (nCompare == 0)
		{
			iLevel++;
		}
		else
		{
			bTryNextSortedColumn = FALSE;
		}
	}

	return nCompare;
}
//******************************************************************************************
void CBCGPGridCtrl::CleanUpAutoGroups (CBCGPGridCtrl::AUTOGROUP_CLEANUP_MODE nMode)
{
	ASSERT_VALID (this);

	if (nMode == AG_COPY_AUTOGROUPS)
	{
		// Make copy and remove all auto groups (do not destroy auto groups):
		m_lstOldAutoGroups.AddTail (&m_lstAutoGroups);

		for (POSITION pos = m_lstAutoGroups.GetHeadPosition (); pos != NULL; )
		{
			CBCGPGridRow* pRow = m_lstAutoGroups.GetNext (pos);
			ASSERT_VALID (pRow);

			// Remove parent-child links:
			for (POSITION pos = pRow->m_lstSubItems.GetHeadPosition (); pos != NULL; )
			{
				CBCGPGridRow* pChildRow = pRow->m_lstSubItems.GetNext (pos);
				ASSERT_VALID (pChildRow);

				BOOL bIsAutoGroup = pChildRow->IsGroup () && 
					(pChildRow->m_dwFlags & BCGP_GRID_ITEM_AUTOGROUP) != 0;
				if (!bIsAutoGroup)
				{
					pChildRow->SetParent(NULL);
				}
			}
		}

		m_lstAutoGroups.RemoveAll ();
	}

	if (nMode == AG_CLEANUP_OLDAUTOGROUPS_ONLY || nMode == AG_FULL_CLEANUP)
	{
		// Destroy old auto groups:
		for (POSITION pos = m_lstOldAutoGroups.GetHeadPosition (); pos != NULL; )
		{
			CBCGPGridRow* pRow = m_lstOldAutoGroups.GetNext (pos);
			ASSERT_VALID (pRow);

			pRow->m_lstSubItems.RemoveAll ();
		}

		while (!m_lstOldAutoGroups.IsEmpty ())
		{
			delete m_lstOldAutoGroups.RemoveTail ();
		}
	}

	if (nMode == AG_FULL_CLEANUP)
	{
		// Destroy auto groups:
		for (POSITION pos = m_lstAutoGroups.GetHeadPosition (); pos != NULL; )
		{
			CBCGPGridRow* pRow = m_lstAutoGroups.GetNext (pos);
			ASSERT_VALID (pRow);

			// Remove parent-child links:
			for (POSITION pos = pRow->m_lstSubItems.GetHeadPosition (); pos != NULL; )
			{
				CBCGPGridRow* pChildRow = pRow->m_lstSubItems.GetNext (pos);
				ASSERT_VALID (pChildRow);

				pChildRow->SetParent(NULL);
			}

			pRow->m_lstSubItems.RemoveAll ();
		}

		while (!m_lstAutoGroups.IsEmpty ())
		{
			delete m_lstAutoGroups.RemoveTail ();
		}
	}
}
//******************************************************************************************
BOOL CBCGPGridCtrl::SetBeginEditReason (DWORD dwReason, BOOL bShift, BOOL bCtrl)
{
	if (bShift)
	{
		dwReason |= BeginEdit_Shift;
	}
	if (bCtrl)
	{
		dwReason |= BeginEdit_Ctrl;
	}

	m_dwBeginEditReason = dwReason;

	return TRUE;
}
//******************************************************************************************
void CBCGPGridCtrl::ClearBeginEditReason ()
{
	m_dwBeginEditReason = BeginEdit_None;
}
//******************************************************************************************
BOOL CBCGPGridCtrl::SetEndEditReason (DWORD dwResult, BOOL bShift, BOOL bCtrl)
{
	CBCGPGridRow* pSel = GetCurSel ();
	if (pSel != NULL && pSel->IsInPlaceEditing () && m_dwEndEditResult == EndEdit_NoResult)
	{
		if (bShift)
		{
			dwResult |= EndEdit_Shift;
		}
		if (bCtrl)
		{
			dwResult |= EndEdit_Ctrl;
		}

		m_dwEndEditResult = dwResult;

		return TRUE;
	}

	return FALSE;
}
//******************************************************************************************
void CBCGPGridCtrl::ClearEndEditReason ()
{
	m_dwEndEditResult = EndEdit_NoResult;
}
//******************************************************************************************
void CBCGPGridCtrl::OnSize(UINT nType, int cx, int cy) 
{
	CBCGPWnd::OnSize(nType, cx, cy);

	SetEndEditReason (EndEdit_Cancel | EndEdit_Layout);
	EndEditItem (FALSE);

	AdjustLayout ();
}
//******************************************************************************************
LRESULT CBCGPGridCtrl::OnSetFont (WPARAM wParam, LPARAM /*lParam*/)
{
	m_hFont = (HFONT) wParam;

	UpdateFonts ();
	CalcEditMargin ();
	AdjustLayout ();

	return 0;
}
//******************************************************************************************
LRESULT CBCGPGridCtrl::OnGetFont (WPARAM, LPARAM)
{
	return (LRESULT) (m_hFont != NULL ? m_hFont : GetDefaultFont ());
}
//******************************************************************************************
void CBCGPGridCtrl::OnSettingChange(UINT uFlags, LPCTSTR lpszSection) 
{
	CBCGPWnd::OnSettingChange(uFlags, lpszSection);

	if (uFlags == SPI_SETNONCLIENTMETRICS ||
		uFlags == SPI_SETWORKAREA ||
		uFlags == SPI_SETICONTITLELOGFONT)
	{
#ifndef _BCGPGRID_STANDALONE
		if (CBCGPToolBar::GetAllToolbars ().IsEmpty ())
#endif
		{
			globalData.UpdateFonts();
		}

		UpdateFonts ();
		CalcEditMargin ();
		AdjustLayout ();
	}
}
//******************************************************************************************
void CBCGPGridCtrl::UpdateFonts ()
{
	CleanUpFonts ();

	// Create bold font:
	CFont* pFont = CFont::FromHandle (
		m_hFont != NULL ? m_hFont : GetDefaultFont ());
	ASSERT_VALID (pFont);

	LOGFONT lf;
	memset (&lf, 0, sizeof (LOGFONT));

	pFont->GetLogFont (&lf);

	lf.lfWeight = FW_BOLD;
	m_fontBold.CreateFontIndirect (&lf);
}
//******************************************************************************************
void CBCGPGridCtrl::CleanUpFonts ()
{
	// Clean up bold font:
	if (m_fontBold.GetSafeHandle () != NULL)
	{
		m_fontBold.DeleteObject ();
	}
}
//******************************************************************************************
void CBCGPGridCtrl::CalcEditMargin ()
{
	CEdit editDummy;
	editDummy.Create (WS_CHILD, CRect (0, 0, 100, 20), this, (UINT)-1);

	editDummy.SetFont (GetFont ());
	DWORD dwMargins = editDummy.GetMargins ();
	m_nEditLeftMargin = LOWORD (dwMargins);
	m_nEditRightMargin = HIWORD (dwMargins);

	editDummy.DestroyWindow ();
}
//******************************************************************************************
HFONT CBCGPGridCtrl::GetDefaultFont ()
{
	return CBCGPGridCtrl::m_bUseSystemFont ? 
		(HFONT) ::GetStockObject (DEFAULT_GUI_FONT) : (HFONT) globalData.fontRegular.GetSafeHandle ();
}
//******************************************************************************************
HFONT CBCGPGridCtrl::SetCurrFont (CDC* pDC)
{
	ASSERT_VALID (pDC);

	if (pDC->IsPrinting ())
	{
		if (m_hPrinterFont != NULL)
		{
			return (HFONT) ::SelectObject (pDC->GetSafeHdc (), m_hPrinterFont);
		}
	}
	
	return (HFONT) ::SelectObject (pDC->GetSafeHdc (), 
		m_hFont != NULL ? m_hFont : GetDefaultFont ());
}
//******************************************************************************************
void CBCGPGridCtrl::OnPaint() 
{
	CPaintDC dcPaint (this); // device context for painting
	OnDraw(&dcPaint);
}
//******************************************************************************************
void CBCGPGridCtrl::OnDraw(CDC* pDCPaint)
{
	pDCPaint->GetClipBox (m_rectClip);

	if (m_rectClip.IsRectEmpty ())
	{
		m_rectClip = m_rectList;
	}

	m_rectClip.top = max (m_rectClip.top, m_rectList.top);

	CRect rect;
	GetClientRect (rect);
	rect.right = m_rectList.right;
	rect.bottom = m_rectList.bottom + m_nGridFooterHeight;

	CRect rectGripper;
	GetClientRect (rectGripper);
	rectGripper.left = m_rectList.right;
	rectGripper.top = m_rectList.bottom;
	pDCPaint->FillRect (rectGripper, 
		m_bControlBarColors ? &globalData.brBarFace : &globalData.brBtnFace);

	if (rect.IsRectEmpty ())
	{
		return;
	}

	CBCGPMemDC memDC (*pDCPaint, rect);
	CDC* pDC = &memDC.GetDC ();

	m_clrGray = visualManager->GetGridLeftOffsetColor (this);

	CRect rectClient;
	GetClientRect (rectClient);

	CRect rectFill = rectClient;
	rectFill.top += m_rectHeader.Height () + m_nGroupByBoxHeight;
	rectFill.left += m_rectRowHeader.Width ();

	OnFillBackground (pDC, rectFill);
	OnFillRowHeaderBackground (pDC, m_rectRowHeader);

	if (m_bFilterBar)
	{
		OnFillFilterBar (pDC);
	}

	HFONT hfontOld = SetCurrFont (pDC);
	pDC->SetTextColor (GetTextColor ());
	pDC->SetBkMode (TRANSPARENT);

	CRect rectList = m_rectList;
	rectList.left = m_rectRowHeader.left;
	m_rectClip.NormalizeRect ();
	BOOL bDrawList = m_rectClip.IntersectRect (&rectList, &m_rectClip);

	m_rgnClip.CreateRectRgnIndirect (m_rectClip);
	pDC->SelectClipRgn (&m_rgnClip);

	if (bDrawList)
	{
		OnDrawList (pDC);
		OnDrawSelectionBorder (pDC);

		if (m_bDragDrop || m_bDragEnter)
		{
			OnDrawDragMarker (pDC);
			OnDrawDragFrame (pDC);
		}
	}

	pDC->SelectClipRgn (NULL);

	m_rectClip.SetRectEmpty ();
	m_rgnClip.DeleteObject ();

	OnDrawGridHeader (pDC);
	OnDrawGridFooter (pDC);

	CRect rectGroupByBox = rectClient;
	rectGroupByBox.bottom = min (rectGroupByBox.top + m_nGroupByBoxHeight, rectClient.bottom);
	OnDrawGroupByBox (pDC, rectGroupByBox);

	OnDrawSelectAllArea (pDC);
	OnDrawHeader (pDC);

	::SelectObject (pDC->GetSafeHdc (), hfontOld);

	if (m_pColumnChooser == NULL && m_bColumnChooserVisible)
	{
		ShowColumnsChooser (TRUE, m_bFieldChooserThemed);
	}
}
//******************************************************************************************
void CBCGPGridCtrl::OnFillBackground (CDC* pDC, CRect rectClient)
{
	ASSERT_VALID (pDC);

	if (m_brBackground.GetSafeHandle () == NULL)
	{
		visualManager->OnFillGridBackground (pDC, rectClient);
	}
	else
	{
		pDC->FillRect (rectClient, &m_brBackground);
	}
}
//******************************************************************************************
COLORREF CBCGPGridCtrl::OnFillSelItem (CDC* pDC, CRect rectFill, CBCGPGridItem* /*pItem*/)
{
	ASSERT_VALID (pDC);

	if (!m_bFocused && m_ColorData.m_SelColorsInactive.Draw (pDC, rectFill, !m_bGridLines))
	{
		return m_ColorData.m_SelColorsInactive.m_clrText;
	}

	if (m_ColorData.m_SelColors.Draw (pDC, rectFill, !m_bGridLines))
	{
		return m_ColorData.m_SelColors.m_clrText;
	}

	if (!m_bFocused)
	{
		if (m_brSelBackground.GetSafeHandle () != NULL)
		{
			pDC->FillRect (rectFill, &m_brSelBackground);
			return globalData.clrBtnText;
		}
		else
		{
			return visualManager->OnFillGridItem (this, pDC, rectFill,
				TRUE/*bSelected*/, FALSE/*bActive*/, FALSE/*bSortedColumn*/);
		}
	}
	else
	{
		if (m_brSelBackground.GetSafeHandle () != NULL)
		{
			pDC->FillRect (rectFill, &m_brSelBackground);
			return globalData.clrTextHilite;
		}
		else
		{
			return visualManager->OnFillGridItem (this, pDC, rectFill,
				TRUE/*bSelected*/, FALSE/*bActive*/, FALSE/*bSortedColumn*/);
		}
	}
}
//******************************************************************************************
void CBCGPGridCtrl::OnFillLeftOffset (CDC* pDC, CRect rectFill, CBCGPGridRow* /*pRow*/,
									  BOOL bDrawRightBorder)
{
	ASSERT_VALID (pDC);

	if (globalData.IsHighContastMode ())
	{
		return;
	}

	const int nRightBorder = rectFill.left + GetHierarchyOffset () + GetExtraHierarchyOffset ();

	// special drawing if custom gradient colors
	if (m_ColorData.m_LeftOffsetColors.m_clrBackground != (COLORREF)-1 &&
		m_ColorData.m_LeftOffsetColors.m_clrGradient != (COLORREF)-1 &&
		m_ColorData.m_LeftOffsetColors.m_nGradientAngle == 0)
	{
		CRect rectGradient = rectFill;
		rectGradient.right = nRightBorder;
		m_ColorData.m_LeftOffsetColors.Draw (pDC, rectGradient, TRUE);

		if (rectGradient.right < rectFill.right)
		{
			CRect rectOneColor = rectFill;
			rectOneColor.left = rectGradient.right;
			CBrush br (m_ColorData.m_LeftOffsetColors.m_clrGradient);
			pDC->FillRect (rectOneColor, &br);
		}
	}

	else if (!m_ColorData.m_LeftOffsetColors.Draw (pDC, rectFill, TRUE))
	{
		CBrush br (m_clrGray);
		pDC->FillRect (rectFill, &br);
	}

	if (bDrawRightBorder &&
		m_ColorData.m_LeftOffsetColors.m_clrBorder != (COLORREF)-1)
	{
		CPen pen (PS_SOLID, 1, m_ColorData.m_LeftOffsetColors.m_clrBorder);
		CPen* pOldPen = pDC->SelectObject (&pen);

		int nTopLeftConer = (GetLeftItemBorderOffset () > rectFill.right) ? 1 : 0;
		pDC->MoveTo (rectFill.right - 1, rectFill.top - nTopLeftConer);
		pDC->LineTo (rectFill.right - 1, rectFill.bottom);

		pDC->SelectObject (pOldPen);
	}
}
//******************************************************************************************
BOOL CBCGPGridCtrl::OnAlternateColor (const CBCGPGridItemID& id)
{
	return (id.m_nRow % 2 == 0); // TRUE - even row, FALSE - odd row
}
//******************************************************************************************
void CBCGPGridCtrl::OnDrawList (CDC* pDC)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);

	CPen* pOldPen = pDC->SelectObject (&m_penVLine);
	ASSERT_VALID (pOldPen);

	int nXLeft = m_rectList.left - m_nHorzScrollOffset;
	int nCount = 0;	// count visible columns
	if (m_bGridLines)
	{
		int nPos = GetColumnsInfo ().Begin ();
		while (nPos != GetColumnsInfo ().End ())
		{
			int iColumn = GetColumnsInfo ().Next (nPos);
			if (iColumn == -1)
			{
				break; // no more visible columns
			}

			ASSERT (iColumn >= 0);
			ASSERT (iColumn < GetColumnsInfo ().GetColumnCount ());

			BOOL bIsTreeColumn = (m_nTreeColumn == -1) ? (nCount == 0):
				(m_nTreeColumn == iColumn);

			int nWidth = GetColumnsInfo ().GetColumnWidth (iColumn);
			if (bIsTreeColumn)
			{
				nWidth += GetHierarchyOffset () + GetExtraHierarchyOffset ();
			}

			nXLeft += nWidth;
			nCount ++;

			if (!m_bGridItemBorders)
			{
				// Draw default vertical grid lines
				pDC->MoveTo (nXLeft - 1, m_rectList.top - 1);
				pDC->LineTo (nXLeft - 1, m_rectList.bottom);
			}
		}
	}

	pDC->SelectObject (&m_penHLine);
	
	if (m_bVirtualMode)
	{
		int nIndex = m_nVertScrollOffset / m_nRowHeight;

		for (; nIndex < m_nVirtualRows; nIndex++)
		{
			//-----------------
			// Get virtual row:
			//-----------------
			CBCGPGridRow* pRow = GetVirtualRow (nIndex);
			if (pRow == NULL)
			{
				break;
			}

			//----------
			// Draw row:
			//----------
			if (!OnDrawItem (pDC, pRow))
			{
				break;
			}
			OnDrawRowHeaderItem (pDC, pRow);
		}
	}
	else
	{
		CList<CBCGPGridRow*, CBCGPGridRow*>& lst =
			m_bVirtualMode || (!IsSortingMode () && !IsGrouping ()) ? 
				m_lstItems : m_lstTerminalItems;

		POSITION pos = m_nFirstVisibleItem == -1 ?
			lst.GetHeadPosition () : lst.FindIndex (m_nFirstVisibleItem);

		while (pos != NULL)
		{
			CBCGPGridRow* pItem = lst.GetNext (pos);
			if (pItem == NULL)
			{
				break;
			}

			ASSERT_VALID (pItem);

			if (!OnDrawItem (pDC, pItem))
			{
				break;
			}
			OnDrawRowHeaderItem (pDC, pItem);
		}
	}

	pDC->SelectObject (pOldPen);

	RedrawMergedItems (pDC);
}
//******************************************************************************************
BOOL CBCGPGridCtrl::OnDrawItem (CDC* pDC, CBCGPGridRow* pItem)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);
	ASSERT_VALID (pItem);

	if (!pItem->m_Rect.IsRectEmpty ())
	{
		if (pItem->m_Rect.top >= m_rectClip.bottom)
		{
			return FALSE;
		}

		if (pItem->m_Rect.bottom >= m_rectClip.top)
		{
			int dx = IsSortingMode () && !IsGrouping () ? 0 : pItem->GetHierarchyLevel () * GetHierarchyLevelOffset ();
			
			BOOL bIsAutoGroup = pItem->IsGroup () && (pItem->m_dwFlags & BCGP_GRID_ITEM_AUTOGROUP) != 0;
			BOOL bGroupUnderline = !m_bHighlightGroups && !pItem->HasValueField () && visualManager->IsGridGroupUnderline ();

			COLORREF clrTextOld = (COLORREF)-1;

			BOOL bNoScrollCol0 = (pItem->IsGroup () && m_bFreezeGroups) || (m_nHorzScrollOffset > 0 && GetColumnsInfo ().GetFrozenColumnCount () > 0);
			BOOL bNoScrollGroups = pItem->IsGroup () && m_bFreezeGroups && m_nHorzScrollOffset > 0;

			// ----------------
			// Draw left margin
			// ----------------
			CRect rectLeft = pItem->m_Rect;
			rectLeft.top++;
			if (bNoScrollCol0)
			{	// do not scroll left margin of frozen columns
				rectLeft.OffsetRect (m_nHorzScrollOffset, 0);
			}
			if (!m_bHighlightGroups || !pItem->IsGroup ())
			{
				rectLeft.right = rectLeft.left + GetExtraHierarchyOffset () + dx;
			}
			rectLeft.bottom++;

			CRgn rgnClipLeft;
			CRect rectLeftClip = rectLeft;
			rectLeftClip.left = max (rectLeftClip.left, m_rectList.left);
			rectLeftClip.bottom = min (rectLeftClip.bottom + 1, m_rectList.bottom);

			if (GetColumnsInfo ().GetColumnCount (TRUE) > 0)
			{
				int nCol0Idx = GetColumnsInfo ().OrderToIndex (0);
				if (nCol0Idx != -1)
				{
					int nCol0Width = GetColumnsInfo ().GetColumnWidth (nCol0Idx);
					nCol0Width += GetHierarchyOffset () + GetExtraHierarchyOffset ();
					if (nCol0Width > 0)
					{
						rectLeftClip.right = min (rectLeftClip.right, pItem->m_Rect.left + nCol0Width + (bNoScrollCol0? m_nHorzScrollOffset : 0));
					}
				}
			}

			if (rectLeftClip.left < rectLeftClip.right)
			{
				if (GetLeftItemBorderOffset () > rectLeft.right)
				{
					rectLeftClip.top = max (rectLeftClip.top - 1, m_rectList.top);
				}

				rgnClipLeft.CreateRectRgnIndirect (&rectLeftClip);
				pDC->SelectClipRgn (&rgnClipLeft);

				OnFillLeftOffset (pDC, rectLeft, pItem, 
					m_bGridLines && pItem->HasValueField () && !bIsAutoGroup);

				pDC->SelectClipRgn (&m_rgnClip);
			}

			if (!pItem->IsEnabled ())
			{
				clrTextOld = pDC->SetTextColor (globalData.clrGrayedText);
			}
			
			CRect rectName = pItem->m_Rect;
			if (bNoScrollGroups)
			{
				rectName.OffsetRect (m_nHorzScrollOffset, 0);
			}

			if (!pItem->HasValueField () || bIsAutoGroup)
			{
				// fill group background
				CRect rectFill = rectName;
				rectFill.top++;
				rectFill.DeflateRect (dx, 0, 0, 0);

				if (!bGroupUnderline)
				{
					rectFill.DeflateRect (GetExtraHierarchyOffset (), 0, 0, 0);
				}

				if (!m_bHighlightGroups)
				{
					if (IsKindOf (RUNTIME_CLASS (CBCGPReportCtrl)) ||
						!m_ColorData.m_GroupColors.Draw (pDC, rectFill))
					{
						if (m_brGroupBackground.GetSafeHandle () != NULL ||
							m_brBackground.GetSafeHandle () != NULL)
						{
							CBrush& br = (m_brGroupBackground.GetSafeHandle () != NULL) ? 
								m_brGroupBackground : m_brBackground;
							pDC->FillRect (rectFill, &br);
						}
						else
						{
							visualManager->OnFillGridGroupBackground (this, pDC, rectFill);
						}
					}
				}

				// draw group underline
				if (bGroupUnderline && pItem->IsGroup ())
				{
					rectFill.top = rectFill.bottom;
					rectFill.InflateRect (0, 1);

					visualManager->OnDrawGridGroupUnderline (this, pDC, rectFill);
				}
			}

			// ---------------
			// draw expandbox:
			// ---------------
			if (pItem->IsGroup () && (!IsSortingMode () || IsGrouping ()) &&
				!pItem->m_lstSubItems.IsEmpty ())
			{
				CRect rectExpand = pItem->m_Rect;
				if (bNoScrollCol0)
				{	// do not scroll expandbox of frozen columns
					rectExpand.OffsetRect (m_nHorzScrollOffset, 0);
				}

				int nLeftMargin = max(GetExtraHierarchyOffset (), m_nButtonWidth) + dx;
				rectName.left += nLeftMargin;
				rectExpand.right = rectExpand.left + nLeftMargin;
				rectExpand.DeflateRect (dx, 0, 0, 0);

				CRgn rgnClipExpand;
				CRect rectExpandClip = rectExpand;
				rectExpandClip.bottom = min (rectExpandClip.bottom, m_rectList.bottom);

				rgnClipExpand.CreateRectRgnIndirect (&rectExpandClip);
				pDC->SelectClipRgn (&rgnClipExpand);

				pItem->OnDrawExpandBox (pDC, rectExpand);

				pDC->SelectClipRgn (&m_rgnClip);
			}

			// ----------------
			// Draw row marker:
			// ----------------
			BOOL bActiveItem = (GetCurSel () == pItem);
			if (m_bRowMarker && !IsRowMarkerOnRowHeader () && bActiveItem && 
				!pItem->IsGroup() && GetExtraHierarchyOffset () > 0)
			{
				CRect rectRowMarker = rectLeft;
				rectRowMarker.left = max (
					rectRowMarker.right - GetExtraHierarchyOffset (), 
					rectRowMarker.left);

				CRgn rgnClipMarker;
				CRect rectMarkerClip = rectRowMarker;
				rectMarkerClip.left = min (rectMarkerClip.left, m_rectList.left);
				rectMarkerClip.bottom = min (rectMarkerClip.bottom, m_rectList.bottom);

				rgnClipMarker.CreateRectRgnIndirect (&rectMarkerClip);
				pDC->SelectClipRgn (&rgnClipMarker);

				pItem->OnDrawRowMarker (pDC, rectRowMarker);

				pDC->SelectClipRgn (&m_rgnClip);
			}

			// ----------
			// draw name:
			// ----------
			if (rectName.right > rectName.left)
			{
				CRgn rgnClipName;
				CRect rectNameClip = rectName;
				rectNameClip.bottom = min (rectNameClip.bottom, m_rectList.bottom);

				rgnClipName.CreateRectRgnIndirect (&rectNameClip);
				pDC->SelectClipRgn (&rgnClipName);

				HFONT hOldFont = NULL;
				if (pItem->IsGroup ())
				{
					hOldFont = (HFONT) ::SelectObject (pDC->GetSafeHdc (), m_fontBold.GetSafeHandle ());
				}

				pItem->OnDrawName (pDC, rectName);

				if (hOldFont != NULL)
				{
					::SelectObject (pDC->GetSafeHdc (), hOldFont);
				}

				pDC->SelectClipRgn (&m_rgnClip);
			}

			// ------------
			// draw values:
			// ------------
			if (pItem->HasValueField () && !bIsAutoGroup)
			{
				CRect rectItems = pItem->m_Rect;

				CRect rectValClip = rectItems;
				rectValClip.bottom = min (rectValClip.bottom, m_rectList.bottom);
				rectValClip.bottom++;

				if (GetLeftItemBorderOffset () > rectLeft.right)
				{
					rectValClip.top = max (rectValClip.top - 1, m_rectList.top);
				}

				m_rgnClipRow.CreateRectRgnIndirect (&rectValClip);
				pDC->SelectClipRgn (&m_rgnClipRow);

				pItem->OnDrawItems (pDC, rectItems);

				if (m_bGridLines && !m_bGridItemBorders)
				{
					// Draw default horizontal grid lines
					pDC->MoveTo (m_rectList.left + GetExtraHierarchyOffset () + dx - m_nHorzScrollOffset, pItem->m_Rect.bottom);
					pDC->LineTo (m_rectList.right, pItem->m_Rect.bottom);

					if (GetLeftItemBorderOffset () > rectLeft.right)
					{
						// repeat line of the previous row
						pDC->MoveTo (m_rectList.left + GetExtraHierarchyOffset () + dx - m_nHorzScrollOffset, pItem->m_Rect.top);
						pDC->LineTo (m_rectList.right, pItem->m_Rect.top);
					}
				}

				pDC->SelectClipRgn (NULL);
				m_rgnClipRow.DeleteObject ();
			}
			else if (m_bGridLines && !m_bHighlightGroups && !bGroupUnderline)
			{
				pDC->SelectClipRgn (NULL);
				
				pDC->MoveTo (m_rectList.left + GetExtraHierarchyOffset () + dx - m_nHorzScrollOffset, pItem->m_Rect.bottom);
				pDC->LineTo (m_rectList.right, pItem->m_Rect.bottom);

				if (GetLeftItemBorderOffset () > rectLeft.right)
				{
					// repeat line of the previous row
					pDC->MoveTo (m_rectList.left + GetExtraHierarchyOffset () + dx - m_nHorzScrollOffset, pItem->m_Rect.top);
					pDC->LineTo (m_rectList.right, pItem->m_Rect.top);
				}
			}

			m_nLeftItemBorderOffset = (!pItem->HasValueField () || bIsAutoGroup) ? 0 : rectLeft.right;

			pDC->SelectClipRgn (&m_rgnClip);

			if (m_bDrawFocusRect &&
				IsFocused () && IsWholeRowSel () && GetCurSel () == pItem)
			{
				CRect rect = pItem->m_Rect;
				if (bNoScrollGroups)
				{
					rect.OffsetRect (m_nHorzScrollOffset, 0);
				}
				rect.top++;
				rect.DeflateRect (dx, 0, 0, 0);
				pDC->DrawFocusRect (rect);
			}

			if (clrTextOld != (COLORREF)-1)
			{
				pDC->SetTextColor (clrTextOld);
			}
		}
	}

	return TRUE;
}
//****************************************************************************************
void CBCGPGridCtrl::OnDrawGroupByBox (CDC* pDC, CRect rect)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);

	if (!m_bGroupByBox)
	{
		return;
	}

	int nXMul = 1, nXDiv = 1;
	int nYMul = 1, nYDiv = 1;
	if (m_bIsPrinting)
	{
		// map to printer metrics
		HDC hDCFrom = ::GetDC(NULL);
		nXMul = pDC->GetDeviceCaps(LOGPIXELSX);			// pixels in print dc
		nXDiv = ::GetDeviceCaps(hDCFrom, LOGPIXELSX);	// pixels in screen dc
		nYMul = pDC->GetDeviceCaps(LOGPIXELSY);			// pixels in print dc
		nYDiv = ::GetDeviceCaps(hDCFrom, LOGPIXELSY);	// pixels in screen dc
		::ReleaseDC(NULL, hDCFrom);
	}

	const int CALCULATED_TEXT_MARGIN = m_bIsPrinting ? 
		::MulDiv (TEXT_MARGIN, nXMul, nXDiv) : TEXT_MARGIN;

	COLORREF clrText = (m_bIsPrinting ? m_clrPrintText : globalData.clrBtnText);
	CBrush br (m_clrPrintHeaderBG);
	if (m_bIsPrinting)
	{
		pDC->FillRect (rect, &br);
	}
	else
	{
		clrText = visualManager->OnFillGridGroupByBoxBackground (pDC, rect);
	}

	COLORREF clrTextOld = pDC->GetTextColor ();

	if (GetColumnsInfo ().GetGroupColumnCount () > 0)
	{
		// map to printer metrics
		const int CALCULATED_HMARGIN = m_bIsPrinting ? 
			::MulDiv (BCGPGRID_GROUPBYBOX_HMARGIN, nXMul, nXDiv) : 
			BCGPGRID_GROUPBYBOX_HMARGIN;
		const int CALCULATED_VMARGIN = m_bIsPrinting ? 
			::MulDiv (BCGPGRID_GROUPBYBOX_VMARGIN, nYMul, nYDiv) : 
			BCGPGRID_GROUPBYBOX_VMARGIN;

		const int CALCULATED_COLUMNWIDTH = m_bIsPrinting ? 
			::MulDiv (BCGPGRID_GROUPBYBOX_COLUMNWIDTH, nXMul, nXDiv) : 
			BCGPGRID_GROUPBYBOX_COLUMNWIDTH;

		const int CALCULATED_HSPACING = m_bIsPrinting ? 
			::MulDiv (BCGPGRID_GROUPBYBOX_HSPACING, nXMul, nXDiv) : 
			BCGPGRID_GROUPBYBOX_HSPACING;
		const int CALCULATED_VSPACING = m_bIsPrinting ? 
			::MulDiv (BCGPGRID_GROUPBYBOX_VSPACING, nYMul, nYDiv) : 
			BCGPGRID_GROUPBYBOX_VSPACING;
		
		rect.DeflateRect (CALCULATED_HMARGIN, CALCULATED_VMARGIN, 0, 0);

		int nItemHeight = (m_bIsPrinting ? m_PrintParams.m_nBaseHeight : m_nBaseHeight) +
			CALCULATED_TEXT_MARGIN;

		CPen pen (PS_SOLID, 1, m_bIsPrinting ? m_clrPrintHeader : 
			visualManager->GetGridGroupByBoxLineColor ());
		CPen* pOldPen = pDC->SelectObject (&pen);

		for (int i = 0; i < GetColumnsInfo ().GetGroupColumnCount (); i++)
		{
			int nCol = GetColumnsInfo ().GetGroupColumn (i);
			if (nCol != -1)
			{
				CRect rectItem = rect;

				int nItemWidth = pDC->GetTextExtent (GetColumnName (nCol)).cx + 
					CALCULATED_COLUMNWIDTH + CALCULATED_TEXT_MARGIN;

				rectItem.bottom = min (rectItem.top + nItemHeight, rect.bottom);
				rectItem.right = min (rectItem.left + nItemWidth, rect.right);

				if (m_bIsPrinting)
				{
					pDC->FillRect (rectItem, &br);
				}

				pDC->SetTextColor (clrText);

				GetColumnsInfo ().DrawColumn (pDC, nCol, rectItem,
					CALCULATED_TEXT_MARGIN, ::MulDiv(5, nXMul, nXDiv),
					m_bIsPrinting, FALSE, TRUE);

				//-----------------------
				// Draw connection lines:
				//-----------------------
				if (i < GetColumnsInfo ().GetGroupColumnCount () - 1)
				{
					CPoint pt = rectItem.BottomRight ();
					pt.Offset (CALCULATED_HSPACING - CALCULATED_VSPACING + 1, 0);
					pDC->MoveTo (pt);
					pt.Offset (0, CALCULATED_HSPACING);
					pDC->LineTo (pt);
					pt.Offset (CALCULATED_VSPACING, 0);
					pDC->LineTo (pt);
				}

				rect.left += nItemWidth;
			}

			rect.DeflateRect (CALCULATED_HSPACING, CALCULATED_VSPACING, 0, 0);
		}

		pDC->SelectObject (pOldPen);
	}
	else if (!m_bIsPrinting)
	{
		CBCGPLocalResource locaRes;

		int nMargin = ::MulDiv (5, nXMul, nXDiv);
		rect.DeflateRect (nMargin, nMargin);

		CString strTitle;
		strTitle.LoadString (IDS_BCGBARRES_GRID_GROUP_TITLE);

		int nTextWidth = pDC->GetTextExtent (strTitle).cx;
		rect.right = min (rect.right, rect.left + nTextWidth + 2 * CALCULATED_TEXT_MARGIN);

		COLORREF clrLabelText = (m_bIsPrinting ? m_clrPrintText : globalData.clrBtnShadow);
		if (m_bIsPrinting)
		{
			CBrush br (m_clrPrintHeaderBG);
			pDC->FillRect (rect, &br);
		}
		else
		{
			clrLabelText = visualManager->OnFillGridGroupByBoxTitleBackground (pDC, rect);
		}

		CRect rectText = rect;
		rectText.DeflateRect (CALCULATED_TEXT_MARGIN, CALCULATED_TEXT_MARGIN, CALCULATED_TEXT_MARGIN, 0);
		
		pDC->SetTextColor (clrLabelText);

		pDC->DrawText (strTitle, rectText,
			DT_LEFT | DT_SINGLELINE | DT_TOP | DT_NOPREFIX | DT_END_ELLIPSIS);
	}

	pDC->SetTextColor (clrTextOld);
}
//****************************************************************************************
void CBCGPGridCtrl::OnDrawHeader (CDC* pDC)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);

	if (m_rectHeader.IsRectEmpty ())
	{
		return;
	}

	OnFillHeaderBackground (pDC, m_rectHeader);

	const int nFreezeOffset = m_rectList.left + GetColumnsInfo ().GetFreezeOffset ();

	for (int i = 0; i <= GetColumnsInfo ().GetColumnCount(); i++)
	{
		if (i < GetColumnsInfo ().GetColumnCount() &&
			!GetColumnsInfo ().GetColumnVisible (i))
		{
			continue;
		}

		CRect rectColumn;
		GetColumnsInfo ().GetColumnRect (i, rectColumn);

		if (m_nHorzScrollOffset > 0)
		{
			CRect rectColumnClip = rectColumn;
			if (m_rectHeader.left > rectColumnClip.left)
			{
				rectColumnClip.left = min (rectColumnClip.right, m_rectHeader.left);
			}

			if (GetColumnsInfo ().IsFreezeColumnsEnabled () && !GetColumnsInfo ().IsColumnFrozen (i))
			{
				if (nFreezeOffset > rectColumnClip.left)
				{
					rectColumnClip.left = min (rectColumnClip.right, nFreezeOffset);
				}
			}

			CRgn rgnClipColumn;
			rgnClipColumn.CreateRectRgnIndirect (&rectColumnClip);
			pDC->SelectClipRgn (&rgnClipColumn);

			GetColumnsInfo ().DrawColumn (pDC, i, rectColumn);

			pDC->SelectClipRgn (NULL);
		}
		else
		{
			GetColumnsInfo ().DrawColumn (pDC, i, rectColumn);
		}
	}
}
//****************************************************************************************
BOOL CBCGPGridCtrl::DoDrawText (CDC* pDC, CString strText, CRect rect, UINT uiDrawTextFlags, LPRECT lpRectClip, BOOL bNoCalcExtent)
{
	ASSERT_VALID (pDC);

	if (lpRectClip != NULL)
	{
		pDC->SaveDC();
		pDC->IntersectClipRect(lpRectClip);
	}

	BOOL bTextTrancatedHorz = FALSE;
	BOOL bTextTrancatedVert = FALSE;

	TEXTMETRIC tm;
	pDC->GetTextMetrics (&tm);
	ASSERT(tm.tmHeight >= 0);

	CRect rectText = rect;
	UINT uiFlags = uiDrawTextFlags;

	if ((uiFlags & DT_SINGLELINE) == 0 && !bNoCalcExtent) // multiline
	{
		uiFlags |= DT_WORDBREAK | DT_EDITCONTROL | DT_EXTERNALLEADING;

		CRect rectTextExtent = rect;
		pDC->DrawText (strText, rectTextExtent, DT_CALCRECT | uiFlags);

		int nOffset = 0;
		int nDecreseBottom = 0;
		if (rect.Height () < rectTextExtent.Height ())
		{
			// trancate partialy visible lines
			nDecreseBottom = rect.Height () % (int)tm.tmHeight;
			rectTextExtent.bottom = rect.bottom - nDecreseBottom;
			bTextTrancatedVert = TRUE;
		}

		if (rect.Height () > rectTextExtent.Height ())
		{
			if (uiFlags & DT_TOP)
			{
				// do nothing
			}
			if (uiFlags & DT_VCENTER)
			{
				nOffset = (rect.Height () - rectTextExtent.Height ()) / 2;
				nDecreseBottom = max (0, nDecreseBottom - nOffset);
			}
			else if (uiFlags & DT_BOTTOM)
			{
				nOffset = rect.Height () - rectTextExtent.Height ();
				nDecreseBottom = 0;
			}
		}

		rectText.top += nOffset;
		rectText.bottom -= nDecreseBottom;

		if (rectText.Height () / tm.tmHeight <= 1)
		{
			uiFlags = uiDrawTextFlags | DT_SINGLELINE;
			rectText = rect;
		}
	}

	if (uiFlags & DT_SINGLELINE)
	{
		if (m_bTrimTextLeft)
		{
			strText.TrimLeft ();
		}

		strText.Replace (_T('\n'), _T(' '));

		if (!bNoCalcExtent)
		{
			bTextTrancatedHorz = pDC->GetTextExtent (strText).cx > rectText.Width ();
		}
	}

	if (bTextTrancatedHorz)
	{
		uiFlags &= ~(DT_CENTER | DT_RIGHT);
		uiFlags |= DT_LEFT;
	}

	if (bTextTrancatedVert && (uiFlags & DT_SINGLELINE) == 0)
	{
		int nLinesTotal = rectText.Height () / (int)tm.tmHeight;
		int nLine = 1;
		for (int iEOL = 0; iEOL < strText.GetLength (); iEOL++)
		{
			iEOL = strText.Find (_T('\n'), iEOL);
			if (iEOL < 0)
			{
				break;
			}

			if (nLine++ == nLinesTotal)
			{
				strText = strText.Left (iEOL);
				if (uiDrawTextFlags & (DT_PATH_ELLIPSIS | DT_END_ELLIPSIS | DT_WORD_ELLIPSIS))
				{
					strText += _T("...");
				}
				break;
			}
		}
	}

	pDC->DrawText (strText, rectText, uiFlags);

	if (lpRectClip != NULL)
	{
		pDC->RestoreDC(-1);
	}

	return bTextTrancatedHorz || bTextTrancatedVert;
}
//****************************************************************************************
void CBCGPGridCtrl::OnDrawSortArrow (CDC* pDC, CRect rectArrow, BOOL bAscending)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);

	visualManager->OnDrawGridSortArrow (this, pDC, rectArrow, bAscending);
}
//****************************************************************************************
void CBCGPGridCtrl::OnFillHeaderBackground (CDC* pDC, CRect rect)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (this);

	if (!m_ColorData.m_HeaderColors.Draw (pDC, rect))
	{
		visualManager->OnFillGridHeaderBackground (this, pDC, rect);
	}
}
//****************************************************************************************
void CBCGPGridCtrl::OnDrawHeaderItemBorder (CDC* pDC, CRect rect, int nCol)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (this);

	CBCGPGridHeaderParams params;
	params.m_nHeaderPart = CBCGPGridHeaderParams::HeaderTop;

	CRect rectInnerBorders;
	CRect rectOuterBorders;
	OnGetHeaderBorders (rectInnerBorders, rectOuterBorders, params);
	params.m_rectInnerBorders = rectInnerBorders;
	params.m_rectOuterBorders = rectOuterBorders;

	params.m_rect = rect;
	params.m_nColumn = nCol;
	params.m_nItemSelected = (IsColumnSelected (nCol)) ? CBCGPGridHeaderParams::Selected : CBCGPGridHeaderParams::NotSelected;

	if (nCol == m_nDraggedColumn && m_nDraggedColumn != -1 && !m_bDragGroupItem && !m_bDragFromChooser)
	{
		params.m_nItemState = CBCGPGridHeaderParams::Pressed;
	}
	else
	{
		if (nCol == GetColumnsInfo ().GetHighlightColumn () && nCol != -1)
		{
			params.m_nItemState = CBCGPGridHeaderParams::Hot;
		}
		else
		{
			params.m_nItemState = CBCGPGridHeaderParams::Normal;
		}
	}

	DrawHeaderPart (pDC, params);
}
//****************************************************************************************
void CBCGPGridCtrl::OnDrawSelectAllArea (CDC* pDC)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (this);

	if (m_rectSelectAllArea.IsRectEmpty ())
	{
		return;
	}

	CBCGPGridHeaderParams params;
	params.m_nHeaderPart = CBCGPGridHeaderParams::HeaderTopLeft;

	CRect rectInnerBorders;
	CRect rectOuterBorders;
	OnGetHeaderBorders (rectInnerBorders, rectOuterBorders, params);

	params.m_rect = m_rectSelectAllArea;
	params.m_rectInnerBorders = rectInnerBorders;
	params.m_rectOuterBorders = rectOuterBorders;

	params.m_nItemState = CBCGPGridHeaderParams::Normal;
	params.m_nItemSelected = (IsAllSelected ()) ? CBCGPGridHeaderParams::Selected : CBCGPGridHeaderParams::NotSelected;

	FillHeaderPartBackground (pDC, params);
	DrawHeaderPart (pDC, params);
}
//****************************************************************************************
void CBCGPGridCtrl::OnFillRowHeaderBackground (CDC* pDC, CRect rect)
{
	ASSERT_VALID (pDC);

	if (TRUE) // fill rest of header same as control background
	{
		OnFillBackground (pDC, rect);
	}
	else
	{
		if (!m_ColorData.m_HeaderColors.Draw (pDC, rect))
		{
			visualManager->OnFillGridHeaderBackground (this, pDC, rect);
		}
	}
}
//****************************************************************************************
void CBCGPGridCtrl::OnDrawRowHeaderItem (CDC* pDC, CBCGPGridRow* pItem)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);
	ASSERT_VALID (pItem);

	if (m_rectRowHeader.IsRectEmpty ())
	{
		return;
	}

 	CRect rectHeader = m_rectRowHeader;

	// ----------------------------------
	// Draw row header inside rectHeader:
	// ----------------------------------
	{
		CRect rectItem = rectHeader;
		rectItem.top = pItem->m_Rect.top;
		rectItem.bottom = pItem->m_Rect.bottom;

		CBCGPGridHeaderParams params;
		params.m_nHeaderPart = CBCGPGridHeaderParams::HeaderLeft;
		params.m_pRow = pItem;

		CRect rectInnerBorders;
		CRect rectOuterBorders;
		OnGetHeaderBorders (rectInnerBorders, rectOuterBorders, params);

		params.m_rect = rectItem;
		params.m_rectInnerBorders = rectInnerBorders;
		params.m_rectOuterBorders = rectOuterBorders;
		params.m_nItemSelected = (IsRowSelected (pItem->GetRowId ())) ? CBCGPGridHeaderParams::Selected : CBCGPGridHeaderParams::NotSelected;

		CRect rectClipItem = rectItem;
		rectClipItem.InflateRect (params.m_rectOuterBorders);
		rectClipItem.NormalizeRect ();
		if (rectClipItem.IntersectRect (&rectClipItem, &rectHeader))
		{
			CRgn rgnClipRowHeader;
			rgnClipRowHeader.CreateRectRgnIndirect (&rectClipItem);
			pDC->SelectClipRgn (&rgnClipRowHeader);

			FillHeaderPartBackground (pDC, params);
			DrawHeaderPart (pDC, params);

			if (IsRowMarkerOnRowHeader () && GetCurSel () == pItem)
			{
				pItem->OnDrawRowMarker (pDC, rectItem);
			}

			pDC->SelectClipRgn (&m_rgnClip);
		}
	}
}
//****************************************************************************************
void CBCGPGridCtrl::OnDrawLineNumber (CDC* pDC, CBCGPGridRow* pRow, CRect rect, BOOL bSelected, BOOL/* bPressed*/)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (this);

	if (pRow != NULL)
	{
		ASSERT_VALID (pRow);

		//-----------
		// Draw text:
		//-----------
		COLORREF clrText = (bSelected && m_ColorData.m_HeaderSelColors.m_clrText != -1)
								? m_ColorData.m_HeaderSelColors.m_clrText
								: m_ColorData.m_HeaderColors.m_clrText;
		COLORREF clrTextOld = (COLORREF)-1;
		if (clrText != (COLORREF)-1)
		{
			clrTextOld = pDC->SetTextColor (clrText);
		}

		CRect rectLabel = rect;
		rectLabel.DeflateRect (TEXT_MARGIN, 0);

		CString strLabel;
		if (pRow->GetRowId () >= 0)
		{
			strLabel.Format (_T("%d"), pRow->GetRowId () + 1);
		}

		UINT uiTextFlags = DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX | DT_CENTER;

		if (pRow->GetLinesNumber() == 1)
		{
			uiTextFlags |= DT_VCENTER;
		}
		else
		{
			rectLabel.DeflateRect(0, TEXT_VMARGIN);
		}

		pDC->DrawText (strLabel, rectLabel, uiTextFlags);

		if (clrTextOld != (COLORREF)-1)
		{
			pDC->SetTextColor (clrTextOld);
		}
	}
}
//****************************************************************************************
void CBCGPGridCtrl::OnFillFilterBar (CDC* pDC)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (this);
	
	CRect rectRowHeader = m_rectFilterBar;
	rectRowHeader.right = rectRowHeader.left + m_rectRowHeader.Width ();

	//-----------------
	// Draw row header:
	//-----------------
	{
		CBCGPGridHeaderParams params;
		params.m_nHeaderPart = CBCGPGridHeaderParams::HeaderLeft;
		
		CRect rectInnerBorders;
		CRect rectOuterBorders;
		OnGetHeaderBorders (rectInnerBorders, rectOuterBorders, params);

		params.m_rect = rectRowHeader;
		params.m_rect.OffsetRect (0, -1);
		params.m_rectInnerBorders = rectInnerBorders;
		params.m_rectOuterBorders = rectOuterBorders;

		CRect rectClipItem = params.m_rect;
		rectClipItem.InflateRect (params.m_rectOuterBorders);

		if (!rectClipItem.IsRectEmpty ())
		{
			CRgn rgnClipRowHeader;
			rgnClipRowHeader.CreateRectRgnIndirect (&rectClipItem);
			pDC->SelectClipRgn (&rgnClipRowHeader);

			FillHeaderPartBackground (pDC, params);
			DrawHeaderPart (pDC, params);

			if (m_paramsFilterBar.lstFilter.IsEmpty ())
			{
				if (m_ImageSearch.IsValid())
				{
					m_ImageSearch.DrawEx(pDC, rectRowHeader, 0, CBCGPToolBarImages::ImageAlignHorzCenter, CBCGPToolBarImages::ImageAlignVertCenter);
				}
			}

			pDC->SelectClipRgn (NULL);
		}
	}

	//--------------------------
	// Draw filter bar controls:
	//--------------------------
	if (!m_rectFilterBar.IsRectEmpty ())
	{
		CRect rectFill = m_rectFilterBar;
		rectFill.left += m_rectRowHeader.Width ();
		
		OnFillBackground (pDC, rectFill);
		
		CPen* pOldPen = pDC->SelectObject (&m_penHLine);
		
		pDC->MoveTo (rectFill.left, rectFill.bottom - 1);
		pDC->LineTo (rectFill.right, rectFill.bottom - 1);
		
		pDC->SelectObject (&m_penVLine);
		
		for (int iColumn = 0; iColumn < GetColumnsInfo ().GetColumnCount (); iColumn++)
		{
			CRect rect;
			GetColumnsInfo ().GetColumnRect (iColumn, rect);
			rect.top = m_rectFilterBar.top;
			rect.bottom = m_rectFilterBar.bottom;
			
			pDC->MoveTo (rect.right - 1, rect.top);
			pDC->LineTo (rect.right - 1, rect.bottom);
		}
		
		pDC->SelectObject (pOldPen);
	}
}
//****************************************************************************************
void CBCGPGridCtrl::OnDrawGridHeader (CDC* pDC)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (this);

	CRect rectFill = GetGridHeaderRect ();
	pDC->FillRect (rectFill, &globalData.brBtnFace);
}
//****************************************************************************************
void CBCGPGridCtrl::OnDrawGridFooter (CDC* pDC)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (this);

	CRect rectFill = GetGridFooterRect ();
	pDC->FillRect (rectFill, &globalData.brBtnFace);
}
//****************************************************************************************
void CBCGPGridCtrl::DrawHeaderItem (CDC* pDC, CRect rect, CBCGPHeaderItem* pHeaderItem)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);

	if (pHeaderItem == NULL)
	{
		return;
	}

	ASSERT_VALID (pHeaderItem);

	//-------------
	// Draw border:
	//-------------
	if (pHeaderItem->m_bIsGroupBox)
	{
		visualManager->OnDrawGridGroupByBoxItemBorder (this, pDC, rect);
	}
	else
	{
		OnDrawHeaderItemBorder (pDC, rect, pHeaderItem->m_bNotPressed ? -1 : pHeaderItem->m_nColumn);
	}

	BOOL bValidColumn = TRUE;
	if (pHeaderItem->m_nColumn < 0 ||
		pHeaderItem->m_nColumn >= GetColumnsInfo ().GetColumnCount (FALSE))
	{
		// last
		bValidColumn = FALSE;
	}

	if (pHeaderItem->m_nHierarchyOffset > 0 && !pHeaderItem->m_bIsGroupBox)
	{
		rect.DeflateRect (pHeaderItem->m_nHierarchyOffset, 0, 0, 0);
	}

	int nSortVal = bValidColumn ? GetColumnsInfo ().GetColumnState (pHeaderItem->m_nColumn) : 0;
	BOOL bDrawText = !pHeaderItem->m_strName.IsEmpty () &&
		(!pHeaderItem->m_bNoText || !bValidColumn || !GetColumnTextHidden (pHeaderItem->m_nColumn));

	//------------
	// Draw image:
	//------------
	if (pHeaderItem->m_iImage >= 0 && !pHeaderItem->m_bNoImage)
	{
		//---------------------------------------
		// The column has a image from imagelist:
		//---------------------------------------
		if (m_pImagesHeader != NULL)
		{			
			int cx = 0;
			int cy = 0;

			VERIFY (::ImageList_GetIconSize (*m_pImagesHeader, &cx, &cy));

			CPoint pt = rect.TopLeft ();
			pt.x ++;
			pt.y = (rect.top + rect.bottom - cy) / 2;

			int nArrow = 0; // width of the arrow or the button

			if (nSortVal != 0 && !pHeaderItem->m_bNoSortArrow)
			{
				if (rect.Width () >= GetRowHeight () + cx)
				{
					nArrow = GetRowHeight ();
				}
			}

			int nBtnWidth = bValidColumn ? GetHeaderMenuButtonRect (rect, pHeaderItem->m_nColumn).Width () : 0;
			if (nBtnWidth > 0 && !pHeaderItem->m_bNoButton)
			{
				nArrow = nBtnWidth;
			}

			if (rect.Width () > cx + nArrow)
			{
				if (!pHeaderItem->m_bIsHeaderItemDragWnd &&
					rect.Width () > cx)
				{
					if (bDrawText || pHeaderItem->m_bIsGroupBox)
					{
					}
					else if (pHeaderItem->m_nAlign & HDF_CENTER)
					{
						pt.x += (rect.Width () - nArrow - cx) / 2;
					}
					else if (pHeaderItem->m_nAlign & HDF_RIGHT)
					{
						pt.x = rect.right - nArrow - cx - 1;
					}
					
					rect.left = pt.x;
				}

				VERIFY (m_pImagesHeader->Draw (pDC, pHeaderItem->m_iImage, pt, ILD_NORMAL));

				rect.left += cx;
			}
		}
	}

	if (!pHeaderItem->m_bNoButton && bValidColumn && IsHeaderMenuButtonEnabled (pHeaderItem->m_nColumn))
	{
		CRect rectBtn = GetHeaderMenuButtonRect (rect, pHeaderItem->m_nColumn);
		OnDrawHeaderMenuButton (pDC, rectBtn, pHeaderItem->m_nColumn, !pHeaderItem->m_bNoSortArrow);
		rect.right = rectBtn.left - 1;
	}
	else if (nSortVal != 0 && !pHeaderItem->m_bNoSortArrow)
	{
		//-----------------
		// Draw sort arrow:
		//-----------------
		int nArrowHeight = GetRowHeight () - 2 * pHeaderItem->m_nArrowMargin;

		CRect rectArrow = rect;
		rectArrow.DeflateRect (pHeaderItem->m_nArrowMargin, pHeaderItem->m_nArrowMargin);
		if (rectArrow.Width () >= nArrowHeight)
		{
			rectArrow.left = rectArrow.right - nArrowHeight;
			rect.right = rectArrow.left - 1;

			int dy2 = (int) (.134 * rectArrow.Width ());
			rectArrow.DeflateRect (0, dy2);

			OnDrawSortArrow (pDC, rectArrow, nSortVal > 0);
		}
	}

	//-----------
	// Draw text:
	//-----------
	if (bDrawText)
	{
		COLORREF clrText = (bValidColumn && IsColumnSelected (pHeaderItem->m_nColumn) && m_ColorData.m_HeaderSelColors.m_clrText != -1)
			? m_ColorData.m_HeaderSelColors.m_clrText
			: m_ColorData.m_HeaderColors.m_clrText;
		COLORREF clrTextOld = (COLORREF)-1;
		if (clrText != (COLORREF)-1 && !pHeaderItem->m_bIsGroupBox)
		{
			clrTextOld = pDC->SetTextColor (clrText);
		}

		CRect rectLabel = rect;
		rectLabel.DeflateRect (pHeaderItem->m_nTextMargin, 0);
		
		UINT uiTextFlags = DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX;

		if (!pHeaderItem->m_bIsGroupBox)	// ignore align flags in groupbox
		{
			if (pHeaderItem->m_nAlign & HDF_CENTER)
			{
				uiTextFlags |= DT_CENTER;
			}
			else if (pHeaderItem->m_nAlign & HDF_RIGHT)
			{
				uiTextFlags |= DT_RIGHT;
			}
		}

		if (pHeaderItem->m_bMultiLine && !pHeaderItem->m_bIsGroupBox)
		{
			uiTextFlags |= DT_WORDBREAK;
		}
		else
		{
			uiTextFlags |= DT_SINGLELINE;
		}

		pHeaderItem->m_bNameIsTrancated = 
			DoDrawText (pDC, pHeaderItem->m_strName, rectLabel, uiTextFlags);

		if (clrTextOld != (COLORREF)-1)
		{
			pDC->SetTextColor (clrTextOld);
		}
	}
}
//****************************************************************************************
void CBCGPGridCtrl::PrintHeaderItem (CDC* pDC, CRect rectItem, CRect rectClipItem, CBCGPHeaderItem* pHeaderItem)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);
	
	if (pHeaderItem == NULL)
	{
		return;
	}
	
	ASSERT_VALID (pHeaderItem);

	BOOL bValidColumn = TRUE;
	if (pHeaderItem->m_nColumn < 0 ||
		pHeaderItem->m_nColumn >= GetColumnsInfo ().GetColumnCount (FALSE))
	{
		bValidColumn = FALSE;
	}

	// map to printer metrics
	HDC hDCFrom = ::GetDC(NULL);
	int nXMul = pDC->GetDeviceCaps(LOGPIXELSX);			// pixels in print dc
	int nXDiv = ::GetDeviceCaps(hDCFrom, LOGPIXELSX);	// pixels in screen dc
	::ReleaseDC(NULL, hDCFrom);

	//------------
	// Print text:
	//------------
	CRect rectText = rectItem;
	int nTextMargin = ::MulDiv(TEXT_MARGIN, nXMul, nXDiv);
	rectText.DeflateRect (nTextMargin, nTextMargin, nTextMargin, 0);
	
	pDC->SetTextColor (m_clrPrintText);

	UINT uiTextFlags = DT_TOP | DT_END_ELLIPSIS | DT_NOPREFIX;
	
	if (!pHeaderItem->m_bIsGroupBox)	// ignore align flags in groupbox
	{
		if (pHeaderItem->m_nAlign & HDF_CENTER)
		{
			uiTextFlags |= DT_CENTER;
		}
		else if (pHeaderItem->m_nAlign & HDF_RIGHT)
		{
			uiTextFlags |= DT_RIGHT;
		}
	}
	
	if (pHeaderItem->m_bMultiLine && !pHeaderItem->m_bIsGroupBox)
	{
		uiTextFlags |= DT_WORDBREAK;
	}
	else
	{
		uiTextFlags |= DT_SINGLELINE;
	}

	DoDrawText (pDC, pHeaderItem->m_strName, rectText, uiTextFlags, rectClipItem);

	//------------------
	// Print sort arrow:
	//------------------
	int nColState = bValidColumn ? GetColumnsInfo ().GetColumnState (pHeaderItem->m_nColumn) : 0;
	if (nColState != 0)
	{
		CRect rectArrow = rectItem;
		rectArrow.bottom = min (rectArrow.bottom, rectArrow.top + GetPrintParams ().m_nRowHeight);
		int nMargin = ::MulDiv(5, nXMul, nXDiv);
		rectArrow.DeflateRect (nMargin, nMargin);
		rectArrow.left = max (rectArrow.right - rectArrow.Height (), rectArrow.left);
		
		int dy2 = (int) (.134 * rectArrow.Width ());
		rectArrow.DeflateRect (0, dy2);
		
		BOOL bAscending = nColState >= 0;

		rectArrow.NormalizeRect ();
		if (rectArrow.IntersectRect (&rectArrow, &rectClipItem))
		{
			CBCGPMenuImages::Draw (pDC, bAscending ? CBCGPMenuImages::IdArowUp: CBCGPMenuImages::IdArowDown, 
				rectArrow, CBCGPMenuImages::ImageBlack, rectArrow.Size());
		}
	}

	//--------------
	// Print border:
	//--------------
	CRect rectBorders = rectItem;
	rectBorders.NormalizeRect ();
	
	if (rectBorders.IntersectRect(&rectBorders, &rectClipItem))
	{
		// top border:
		if (rectItem.top >= rectClipItem.top && rectItem.top <= rectClipItem.bottom)
		{
			pDC->MoveTo (rectBorders.left, rectItem.top);
			pDC->LineTo (rectBorders.right, rectItem.top);
		}
		
		// left border:
		if (rectItem.left >= rectClipItem.left && rectItem.left <= rectClipItem.right)
		{
			pDC->MoveTo (rectItem.left, rectBorders.top);
			pDC->LineTo (rectItem.left, rectBorders.bottom);
		}
		
		// bottom border:
		if (rectItem.bottom >= rectClipItem.top && rectItem.bottom <= rectClipItem.bottom)
		{
			pDC->MoveTo (rectBorders.left, rectItem.bottom);
			pDC->LineTo (rectBorders.right, rectItem.bottom);
		}
		
		// right border:
		if (rectItem.right >= rectClipItem.left && rectItem.right <= rectClipItem.right)
		{
			pDC->MoveTo (rectItem.right, rectBorders.top);
			pDC->LineTo (rectItem.right, rectBorders.bottom);
		}
	}
}
//****************************************************************************************
void CBCGPGridCtrl::DrawHeaderPart (CDC* pDC, const CBCGPGridHeaderParams& params)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);

	switch (params.m_nHeaderPart)
	{
	case CBCGPGridHeaderParams::HeaderTop:
		//if (params.m_nColumn != -1)
		{
			CBCGPGridColors::ColorData colorData = 
				params.IsSelected () && m_ColorData.m_HeaderSelColors.m_clrBackground != -1 ? 
					m_ColorData.m_HeaderSelColors :
					m_ColorData.m_HeaderColors;

			if (!colorData.Draw (pDC, params.m_rect))
			{
				m_bHeaderItemHovered = params.IsHighlighted ();
				GetColumnsInfo ().m_bInvertPressedColumn = 
					visualManager->OnDrawGridHeaderItemBorder (this, pDC, params.m_rect, params.IsPressed ());
			}
		}
		break;

	case CBCGPGridHeaderParams::HeaderLeft:
		{
			CBCGPGridColors::ColorData colorData = 
				params.IsSelected () && m_ColorData.m_HeaderSelColors.m_clrBackground != -1 ? 
					m_ColorData.m_HeaderSelColors :
					m_ColorData.m_HeaderColors;

			if (!colorData.Draw (pDC, params.m_rect))
			{
				m_bHeaderItemHovered = params.IsHighlighted ();
				GetColumnsInfo ().m_bInvertPressedColumn = 
					visualManager->OnDrawGridRowHeaderItemBorder (this, pDC, params.m_rect, params.IsPressed ());
			}

			if (m_bLineNumbers && params.m_pRow != NULL)
			{
				m_bHeaderItemHovered = params.IsHighlighted ();
				OnDrawLineNumber (pDC, params.m_pRow, params.m_rect, params.IsSelected (), params.IsPressed ());
			}
		}
		break;

	case CBCGPGridHeaderParams::HeaderTopLeft:
		//----------------------
		// Draw Select All area:
		//----------------------
		{
			CBCGPGridColors::ColorData colorData = 
				params.IsSelected () && m_ColorData.m_HeaderSelColors.m_clrBackground != -1 ? 
					m_ColorData.m_HeaderSelColors :
					m_ColorData.m_HeaderColors;

			if (!colorData.Draw (pDC, params.m_rect))
			{
				m_bHeaderItemHovered = params.IsHighlighted ();
				GetColumnsInfo ().m_bInvertPressedColumn = 
					visualManager->OnDrawGridSelectAllAreaBorder (this, pDC, params.m_rect, params.IsPressed ());
			}

			if (m_bHeaderSelectAllMarker)
			{
				CRect rectMarker = params.m_rect;

				const int nPadding = 5;
				
				rectMarker.DeflateRect(nPadding, nPadding);
				int nMin = min (rectMarker.Width (), rectMarker.Height ());
				nMin = min (nMin, GetButtonWidth () - nPadding);
				rectMarker.left = rectMarker.right  - nMin;
				rectMarker.top  = rectMarker.bottom - nMin;
				
				visualManager->OnDrawGridSelectAllMarker(this, pDC, rectMarker, nPadding, IsAllSelected());
			}
		}
		break;
	}
}
//****************************************************************************************
void CBCGPGridCtrl::FillHeaderPartBackground (CDC* pDC, const CBCGPGridHeaderParams& params)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);

	switch (params.m_nHeaderPart)
	{
	case CBCGPGridHeaderParams::HeaderTop:
		break;

	case CBCGPGridHeaderParams::HeaderLeft:
		//---------------------------
		// Row header item background
		//---------------------------
		if (!m_ColorData.m_HeaderColors.Draw (pDC, params.m_rect))
		{
			visualManager->OnFillGridRowHeaderBackground (this, pDC, params.m_rect);
		}

		break;

	case CBCGPGridHeaderParams::HeaderTopLeft:
		if (!m_ColorData.m_HeaderColors.Draw (pDC, params.m_rect))
		{
			visualManager->OnFillGridSelectAllAreaBackground (this, pDC, params.m_rect, params.IsPressed ());
		}

		break;
	}
}
//****************************************************************************************
void CBCGPGridCtrl::OnGetHeaderBorders (CRect& rectInner, CRect& rectOuter, const CBCGPGridHeaderParams& params)
// returns extra size of the border to extend the item rect
{
	rectInner.SetRectEmpty ();
	rectOuter.SetRectEmpty ();

	switch (params.m_nHeaderPart)
	{
	case CBCGPGridHeaderParams::HeaderTop:
	case CBCGPGridHeaderParams::HeaderLeft:
	case CBCGPGridHeaderParams::HeaderTopLeft:
		rectInner.SetRect (1, 1, 1, 0);
 		rectOuter.SetRect (0, 0, 0, 1);
		break;
	}
}
//****************************************************************************************
CRect CBCGPGridCtrl::OnGetSelectionRect ()
{
	m_rectClipSel.SetRectEmpty ();

	if (!IsSelectionBorderEnabled () || 
		m_lstSel.GetCount () != 1 || m_lstSel.GetTail () == NULL)
	{
		return CRect (0, 0, 0, 0);
	}

	BOOL bFreeze = (GetColumnsInfo ().IsFreezeColumnsEnabled () && m_nHorzScrollOffset > 0);

	const CBCGPGridRange rangeTrackSel = *(m_lstSel.GetTail ());

	CBCGPGridRow* pTopRow = (rangeTrackSel.m_nTop != -1) ? GetRow (rangeTrackSel.m_nTop) : NULL;
	CBCGPGridItem* pLeftItem = (pTopRow != NULL) ? pTopRow->GetItem (rangeTrackSel.m_nLeft) : NULL;
	CBCGPGridRow* pBottomRow = (rangeTrackSel.m_nBottom != -1) ? GetRow (rangeTrackSel.m_nBottom) : NULL;
	CBCGPGridItem* pRightItem = (pBottomRow != NULL) ? pBottomRow->GetItem (rangeTrackSel.m_nRight) : NULL;

	//-------------------------------
	// Calculate selection rectangle:
	//-------------------------------
	const int nLeftOffset = GetHierarchyOffset () + GetExtraHierarchyOffset ();

	CRect rectTopLeft (0, 0, 0, 0);
	if (pTopRow != NULL)
	{
		ASSERT_VALID (pTopRow);

		rectTopLeft = (pLeftItem != NULL) ? pLeftItem->GetRect () : pTopRow->GetRect ();

		if (IsFilterEnabled () && m_idActive.IsColumn ())
		{
			GetColumnsInfo ().GetColumnRect (m_idActive.m_nColumn, rectTopLeft);
			rectTopLeft.top = m_rectList.top;
			rectTopLeft.bottom = m_rectList.bottom;
		}

		if (pLeftItem == NULL)
		{
			rectTopLeft.right = max (rectTopLeft.right - 1, rectTopLeft.left);
			rectTopLeft.left = min (rectTopLeft.right, rectTopLeft.left + nLeftOffset);

			if (bFreeze)
			{
				rectTopLeft.left = min (rectTopLeft.right, m_rectList.left + nLeftOffset);
			}
		}
		rectTopLeft.NormalizeRect ();

		// Extend if merged:
		if (pLeftItem != NULL && pLeftItem->GetMergedCells () != NULL)
		{
			CRect rectMerged = pLeftItem->GetMergedRect ();
			if (!rectMerged.IsRectEmpty ())
			{
				rectMerged.NormalizeRect ();
				rectTopLeft.UnionRect (rectTopLeft, rectMerged);
			}
		}
	}

	CRect rectBottomRight (0, 0, 0, 0);
	if (pBottomRow != NULL)
	{
		ASSERT_VALID (pBottomRow);

		rectBottomRight = (pRightItem != NULL) ? pRightItem->GetRect () : pBottomRow->GetRect ();

		if (IsFilterEnabled () && m_idLastSel.IsColumn ())
		{
			GetColumnsInfo ().GetColumnRect (m_idLastSel.m_nColumn, rectBottomRight);
			rectBottomRight.top = m_rectList.top;
			rectBottomRight.bottom = m_rectList.bottom;
		}

		if (pRightItem == NULL)
		{
			rectBottomRight.left = min (rectBottomRight.right, rectBottomRight.left + nLeftOffset);

			if (bFreeze)
			{
				rectBottomRight.left = min (rectBottomRight.right, m_rectList.left + nLeftOffset);
			}
		}
		rectBottomRight.NormalizeRect ();

		// Extend if merged:
		if (pRightItem != NULL && pRightItem->GetMergedCells () != NULL)
		{
			CRect rectMerged = pRightItem->GetMergedRect ();
			if (!rectMerged.IsRectEmpty ())
			{
				rectMerged.NormalizeRect ();
				rectBottomRight.UnionRect (rectBottomRight, rectMerged);
			}
		}
	}

	CRect rectTrackSel (0, 0, 0, 0);
	rectTrackSel.UnionRect (rectTopLeft, rectBottomRight);

	if (!rectTrackSel.IsRectEmpty ())
	{
		rectTrackSel.InflateRect (2, 1, 2, 2);
	}

	// Frozen columns support:
	m_rectClipSel = rectTrackSel;
	if (bFreeze && pLeftItem != NULL && pRightItem != NULL)
	{
		const int nFreezeOffset = m_rectList.left + GetColumnsInfo ().GetFreezeOffset ();
		BOOL bA = (GetColumnsInfo ().IsColumnFrozen (pLeftItem->GetColumnId ()));
		BOOL bB = (GetColumnsInfo ().IsColumnFrozen (pRightItem->GetColumnId ()));

		if (!bA && !bB) // Clip Selection
		{
			m_rectClipSel.left = min (nFreezeOffset, m_rectClipSel.right);
		}
		else if (bA && bB)
		{
		}
		else
		{
			rectTrackSel = CRect(0, 0, 0, 0);
			rectTrackSel.UnionRect (rectTopLeft, rectBottomRight);

			if (bA && !bB)
			{
				// scrolled column must not change left border of selection
				if (rectTrackSel.left < rectTopLeft.left)
				{
					rectTrackSel.left = rectTopLeft.left;
				}
			}
			else if (!bA && bB)
			{
				// scrolled column must not change left border of selection
				if (rectTrackSel.left < rectBottomRight.left)
				{
					rectTrackSel.left = rectBottomRight.left;
				}
			}

			if (!rectTrackSel.IsRectEmpty ())
			{
				rectTrackSel.InflateRect (2, 1, 2, 2);
			}

			// scrolled column must not change right border of selection
			if (rectTrackSel.right < nFreezeOffset)
			{
				rectTrackSel.right = nFreezeOffset;
			}

			m_rectClipSel = rectTrackSel;
		}
	}
	else if (IsRowHeaderEnabled ())
	{
		m_rectClipSel.left = min (m_rectList.left, m_rectClipSel.right);
	}

	// Filterbar support:
	if (IsFilterBarEnabled () && !m_rectFilterBar.IsRectEmpty () && m_rectFilterBar.bottom > m_rectClipSel.top)
	{
		m_rectClipSel.top = min (m_rectFilterBar.bottom, m_rectClipSel.bottom);
	}

	return rectTrackSel;
}
//****************************************************************************************
void CBCGPGridCtrl::OnDrawSelectionBorder (CDC* pDC)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);

	if (m_bIsPrinting || IsGrouping ())
	{
		return;
	}

	if (!m_rectTrackSel.IsRectEmpty () && !m_rectClipSel.IsRectEmpty ())
	{
		CRect rectClip;
		if (!rectClip.IntersectRect (m_rectClipSel, m_rectList))
		{
			return;
		}

		CRgn rgnClipSel;
		rgnClipSel.CreateRectRgnIndirect (&rectClip);
		pDC->SelectClipRgn (&rgnClipSel);

		// paint clipped
		visualManager->OnDrawGridSelectionBorder (this, pDC, m_rectTrackSel);

		pDC->SelectClipRgn (NULL);
	}
}
//****************************************************************************************
void CBCGPGridCtrl::OnDrawDragMarker (CDC* pDC)
{
	ASSERT_VALID (pDC);
	
	if (!m_rectDragMarker.IsRectEmpty ())
	{
		CPen* pOldPen = (CPen*) pDC->SelectObject (&m_penDrag);
		
		for (int i = 0; i < 2; i ++)
		{
			pDC->MoveTo (m_rectDragMarker.left, m_rectDragMarker.top + m_rectDragMarker.Height () / 2 + i - 1);
			pDC->LineTo (m_rectDragMarker.right, m_rectDragMarker.top + m_rectDragMarker.Height () / 2 + i - 1);
			
			pDC->MoveTo (m_rectDragMarker.left + i, m_rectDragMarker.top + i);
			pDC->LineTo (m_rectDragMarker.left + i, m_rectDragMarker.bottom - i);
			
			pDC->MoveTo (m_rectDragMarker.right - i - 1, m_rectDragMarker.top + i);
			pDC->LineTo (m_rectDragMarker.right - i - 1, m_rectDragMarker.bottom - i);
		}
		
		pDC->SelectObject (pOldPen);
	}
}
//****************************************************************************************
void CBCGPGridCtrl::OnDrawDragFrame (CDC* pDC)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);
	
	if (!m_rectDragFrame.IsRectEmpty ())
	{
 		CRgn rgnClip;
 		rgnClip.CreateRectRgnIndirect (&m_rectList);
		pDC->SelectClipRgn (&rgnClip);

		// paint clipped
		CRect rect;

		rect = m_rectDragFrame;
		rect.right = m_rectDragFrame.left;
		rect.InflateRect (0, 0, 3, 0);
		pDC->FillRect (&rect, pDC->GetHalftoneBrush ());

		rect = m_rectDragFrame;
		rect.left = m_rectDragFrame.right;
		rect.InflateRect (3, 0, 0, 0);
		pDC->FillRect (&rect, pDC->GetHalftoneBrush ());

		rect = m_rectDragFrame;
		rect.bottom = m_rectDragFrame.top;
		rect.InflateRect (0, 0, 0, 3);
		pDC->FillRect (&rect, pDC->GetHalftoneBrush ());

		rect = m_rectDragFrame;
		rect.top = m_rectDragFrame.bottom;
		rect.InflateRect (0, 3, 0, 0);
		pDC->FillRect (&rect, pDC->GetHalftoneBrush ());

		pDC->SelectClipRgn (NULL);
		return;
	}
}
//****************************************************************************************
void CBCGPGridCtrl::OnItemChanged (CBCGPGridItem* pItem, int nRow, int nColumn)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pItem);

	CWnd* pOwner = GetOwner ();
	if (pOwner == NULL)
	{
		return;
	}

	BCGPGRID_ITEM_INFO ii;
	memset (&ii, 0, sizeof (BCGPGRID_ITEM_INFO));
	ii.pItem = pItem;
	ii.nRow = nRow;
	ii.nCol = nColumn;
	ii.dwResultCode = 0;

	pOwner->SendMessage (BCGM_GRID_ITEM_CHANGED, GetDlgCtrlID (), LPARAM (&ii));
}
//*******************************************************************************************
BOOL CBCGPGridCtrl::OnBeginDrag (CPoint point)
{
	ASSERT_VALID (this);
	
	CWnd* pOwner = GetOwner ();
	if (pOwner == NULL)
	{
		return FALSE;
	}

	return (pOwner->SendMessage (BCGM_GRID_BEGINDRAG, GetDlgCtrlID (), MAKELPARAM (point.x, point.y)) != 0);
}
//*******************************************************************************************
BOOL CBCGPGridCtrl::OnEraseBkgnd(CDC* /*pDC*/) 
{
	return TRUE;
}
//******************************************************************************************
void CBCGPGridCtrl::EnableHeader (BOOL bEnable, DWORD dwFlags)
{
	ASSERT_VALID (this);

	CBCGPGridRow* pItem = GetCurSel ();
	if (pItem != NULL)
	{
		pItem->OnEndEdit ();
	}

	if (dwFlags == (DWORD)-1) // default value
	{
		dwFlags &= ~BCGP_GRID_HEADER_SELECT; // disabled by default
		dwFlags &= ~BCGP_GRID_HEADER_FORCESIMPLEBORDERS;
	}

	m_bHeader = bEnable;
	m_dwHeaderFlags = dwFlags;

	if ((m_dwHeaderFlags & BCGP_GRID_HEADER_SORT) != 0 &&
		(m_dwHeaderFlags & BCGP_GRID_HEADER_MOVE_ITEMS) == 0)
	{
		EnableDragHeaderItems (FALSE);
	}

	if ((m_dwHeaderFlags & BCGP_GRID_HEADER_SELECT) != 0)
	{
		if ((m_dwHeaderFlags & BCGP_GRID_HEADER_SORT) != 0)
		{
			ASSERT (FALSE);
			m_dwHeaderFlags &= ~BCGP_GRID_HEADER_SELECT;
		}
	}

	AdjustLayout ();
}
//******************************************************************************************
void CBCGPGridCtrl::EnableHeaderSelectAllMarker(BOOL bEnable/* = TRUE*/)
{
	m_bHeaderSelectAllMarker = bEnable;
}
//******************************************************************************************
void CBCGPGridCtrl::EnableRowHeader (BOOL bEnable, DWORD dwFlags, int nWidth)
{
	ASSERT_VALID (this);

	if (dwFlags == (DWORD)-1) // default value
	{
		dwFlags &= ~BCGP_GRID_HEADER_MOVE_ITEMS; // disabled by default
	}

	m_bRowHeader = bEnable;
	m_dwRowHeaderFlags = dwFlags;

	SetRowHeaderWidth (nWidth, FALSE);

	AdjustLayout ();
}
//*****************************************************************************************
void CBCGPGridCtrl::SetRowHeaderWidth (int nWidth, BOOL bRedraw)
{
	m_nRowHeaderWidth = nWidth;

	if (bRedraw)
	{
		AdjustLayout ();
	}
}
//*****************************************************************************************
void CBCGPGridCtrl::SetHeaderImageList (CImageList* pImagesHeader)
{
	m_pImagesHeader = pImagesHeader;
}
//*****************************************************************************************
void CBCGPGridCtrl::SetHeaderBtnImageList (CBCGPToolBarImages* pImagesHeaderBtn)
{
	m_pImagesHeaderBtn = pImagesHeaderBtn;
}
//*****************************************************************************************
void CBCGPGridCtrl::SetImageList (CImageList* pImages)
{
	m_pImages = pImages;
}
//*****************************************************************************************
void CBCGPGridCtrl::EnableGroupByBox (BOOL bEnable)
{
	ASSERT_VALID (this);

	CBCGPGridRow* pItem = GetCurSel ();
	if (pItem != NULL)
	{
		pItem->OnEndEdit ();
	}

	m_bGroupByBox = bEnable;

	AdjustLayout ();
}
//*****************************************************************************************
void CBCGPGridCtrl::TrackHeader (int nOffset)
{
	CClientDC dc (this);

	BOOL bShowDragContext = m_bShowDragContext && !IsColumnAutoSizeEnabled ();

	if (!m_rectTrackHeader.IsRectEmpty () && !bShowDragContext)
	{
		dc.InvertRect (m_rectTrackHeader);
	}

	if (nOffset < 0)	// End of track
	{
		m_rectTrackHeader.SetRectEmpty ();

		dc.InvertRect (m_rectTrackHeaderLeft);
		m_rectTrackHeaderLeft.SetRectEmpty ();
	}
	else
	{
		m_rectTrackHeader = m_rectList;
		m_rectTrackHeader.left += (nOffset - m_rectList.left);
		m_rectTrackHeader.right = m_rectTrackHeader.left + 1;

		// Calculate column left offset:
		int nLeftOffset = m_rectHeader.left;
		int nCount = 0;	// count visible columns
		
		int nPos = GetColumnsInfo ().Begin ();
		while (nPos != GetColumnsInfo ().End ())
		{
			int i = GetColumnsInfo ().Next (nPos);
			if (i == -1)
			{
				break; // no more visible columns
			}

			if (i == m_nTrackColumn)
			{
				break;
			}

			int nWidth = GetColumnsInfo ().GetColumnWidth (i);
			if (nCount == 0)
			{
				nWidth += GetHierarchyOffset () + GetExtraHierarchyOffset ();
			}

			nLeftOffset += nWidth;
			nCount++;
		}

		int nNewOffset = GetColumnsInfo ().IsColumnFrozen (m_nTrackColumn) ? nLeftOffset : nLeftOffset - m_nHorzScrollOffset;

		CRect rectTrackHeaderLeft = m_rectList;
		rectTrackHeaderLeft.left += (nNewOffset - m_rectList.left);
		rectTrackHeaderLeft.right = rectTrackHeaderLeft.left + 1;

		if (bShowDragContext)
		{
			int nNewWidth = nOffset - nNewOffset;
			if (nCount == 0)
			{
				nNewWidth -= GetHierarchyOffset () + GetExtraHierarchyOffset ();
			}
			nNewWidth = max (m_nBaseHeight, nNewWidth);
			SetHeaderItemWidth (m_nTrackColumn, nNewWidth);

			AdjustLayout ();
			
			CBCGPGridRow* pSel = GetCurSel ();
			if (pSel != NULL)
			{
				pSel->AdjustButtonRect ();
			}
		}
		else
		{
			if (rectTrackHeaderLeft != m_rectTrackHeaderLeft)
			{
				if (!m_rectTrackHeaderLeft.IsRectEmpty ())
				{
					dc.InvertRect (m_rectTrackHeaderLeft);
				}

				m_rectTrackHeaderLeft = rectTrackHeaderLeft;
				dc.InvertRect (m_rectTrackHeaderLeft);
			}

			dc.InvertRect (m_rectTrackHeader);
		}
	}
}
//*****************************************************************************************
void CBCGPGridCtrl::TrackToolTip (CPoint point)
{
	if (m_nDraggedColumn >= 0 || m_bTracking || m_bSelecting)
	{
		return;
	}

#ifndef _BCGPGRID_STANDALONE
	if (CBCGPPopupMenu::GetActiveMenu () != NULL)
	{
		return;
	}
#endif

	CPoint ptScreen = point;
	ClientToScreen (&ptScreen);

	CRect rectTT;
	m_ToolTip.GetLastRect (rectTT);

	if (rectTT.PtInRect (ptScreen) && m_ToolTip.IsWindowVisible ())
	{
		return;
	}

	if (::GetCapture () == GetSafeHwnd ())
	{
		ReleaseCapture ();
	}

	if (!m_bShowInPlaceToolTip)
	{
		m_ToolTip.Deactivate ();
		return;
	}

	if (m_rectColumnChooser.PtInRect (ptScreen))
	{
		m_ToolTip.Deactivate ();
		return;
	}

	CBCGPGridRow::ClickArea clickArea;
	CBCGPGridItemID id;
	CBCGPGridItem* pHitItem = NULL;
	CBCGPGridRow* pHitRow = HitTest (point, id, pHitItem, &clickArea);

	if (pHitRow == NULL)
	{
		m_ToolTip.Deactivate ();
		return;
	}

	ASSERT_VALID (pHitRow);

	CString strTipText;
	CRect rectToolTip;
	rectToolTip.SetRectEmpty ();

	if (clickArea == CBCGPGridRow::ClickName)
	{
		strTipText = pHitRow->GetNameTooltip ();
		rectToolTip = pHitRow->GetNameTooltipRect ();
		if (point.x < rectToolTip.left)
		{
			strTipText.Empty ();
		}
	}
	else if (clickArea == CBCGPGridRow::ClickValue)
	{
		BOOL bIsAutoGroup = pHitRow->IsGroup () && (pHitRow->m_dwFlags & BCGP_GRID_ITEM_AUTOGROUP) != 0;
		if (pHitRow->IsGroup () && 
			(!pHitRow->HasValueField () || bIsAutoGroup))
		{
			strTipText = pHitRow->GetValueTooltip ();
			rectToolTip = pHitRow->GetRect ();
		}
		else
		{
			if (pHitItem != NULL)
			{
				ASSERT_VALID (pHitItem);
				if (pHitItem->IsInPlaceEditing ())
				{
					return;
				}

				strTipText = pHitItem->GetValueTooltip ();
				rectToolTip = pHitItem->GetTooltipRect ();
			}
		}
	}
	
	if (!strTipText.IsEmpty ())
	{
		ClientToScreen (&rectToolTip);
		
		if (rectTT.TopLeft () == rectToolTip.TopLeft ())
		{
			// Tooltip on the same place, don't show it to prevent flashing
			return;
		}

		m_ToolTip.SetTextMargin (TEXT_MARGIN);
		m_ToolTip.SetFont (GetFont ());
		m_ToolTip.SetMultiline (pHitRow->m_nLines > 1);

		m_ToolTip.Track (rectToolTip, strTipText);
		SetCapture ();
	}
	else
	{
		m_ToolTip.Deactivate ();
	}
}
//*******************************************************************************************
BOOL CBCGPGridCtrl::SetHeaderItemWidth (int nPos, int nWidth)
{
	if (!GetColumnsInfo ().OnChangeColumnWidth (nPos, nWidth))
	{	// column can't be resized
		return FALSE;
	}

	BOOL bRes = GetColumnsInfo ().ResizeColumn (nPos, nWidth);
	OnResizeColumns ();
	AdjustFilterBarCtrls ();
	return bRes;
}
//*****************************************************************************************
void CBCGPGridCtrl::Sort (int nColumn, BOOL bAscending, BOOL bAdd)
{
	CWaitCursor wait;
	
	SetCurSel (NULL);
	GetColumnsInfo ().SetSortColumn (nColumn, bAscending, bAdd);

	if (GetSafeHwnd () != NULL)
	{
		SetRebuildTerminalItems ();
		ReposItems ();

		AdjustLayout ();
	}
}
//****************************************************************************************
void CBCGPGridCtrl::ToggleSortColumn (int nColumn)
{
	ASSERT_VALID (this);

	if ((m_dwHeaderFlags & BCGP_GRID_HEADER_SORT) == 0)
	{
		return;
	}

	int nColumnState = GetColumnsInfo ().GetColumnState (nColumn);
	
	if (IsMultipleSort () && nColumnState < 0) // if descending, the third click clears sorting
	{
		RemoveSortColumn (nColumn);

		if (GetSafeHwnd () != NULL)
		{
			SetRebuildTerminalItems ();
			ReposItems ();

			AdjustLayout ();
		}
		return;
	}

	BOOL bAscending = TRUE;

	if (nColumnState != 0)
	{
		bAscending = nColumnState <= 0;
	}

	Sort (nColumn, bAscending, IsMultipleSort ());
}
//****************************************************************************************
void CBCGPGridCtrl::SetSortColumn (int nColumn, BOOL bAscending, BOOL bAdd)
{
	GetColumnsInfo ().SetSortColumn (nColumn, bAscending, bAdd);
	SetRebuildTerminalItems ();
}
//****************************************************************************************
void CBCGPGridCtrl::RemoveSortColumn (int nColumn)
{
	GetColumnsInfo ().RemoveSortColumn (nColumn);
	SetRebuildTerminalItems ();
}
//****************************************************************************************
void CBCGPGridCtrl::EnableMultipleSort (BOOL bEnable)
{
	if (GetColumnsInfo ().IsMultipleSort () == bEnable)
	{
		return;
	}

	GetColumnsInfo ().EnableMultipleSort (bEnable);
	if (!bEnable)
	{
		SetRebuildTerminalItems ();
	}
}
//****************************************************************************************
BOOL CBCGPGridCtrl::IsMultipleSort () const
{
	return GetColumnsInfo ().IsMultipleSort ();
}
//****************************************************************************************
void CBCGPGridCtrl::EnableMarkSortedColumn (BOOL bMark, BOOL bRedraw)
{
	m_bMarkSortedColumn = bMark;

	if (GetSafeHwnd () != NULL && bRedraw)
	{
		RedrawWindow ();
	}
}
//****************************************************************************************
void CBCGPGridCtrl::EnableFilter (BCGPGRID_FILTERCALLBACK pfnCallback, LPARAM lParam)
{
	m_bFilter = TRUE;
	m_pfnFilterCallback = pfnCallback;	// callback function
	m_lFilterParam = lParam;			// filter info which is used by the callback function

	if (m_bFilter && !m_DefaultHeaderBtnImages.IsValid())
	{
		CBCGPLocalResource locaRes;

		m_DefaultHeaderBtnImages.SetImageSize (CSize (12, 12));
		m_DefaultHeaderBtnImages.Load(globalData.Is32BitIcons() ? IDB_BCGBARRES_GRID_FILTER_ICONS32 : IDB_BCGBARRES_GRID_FILTER_ICONS);
	}

	if (m_pfnFilterCallback == NULL)
	{
		//-------------------------------------------------
		// Use the default filter instead of the user's one
		//-------------------------------------------------
		if (m_pDefaultFilter == NULL)
		{
			m_pDefaultFilter = CreateFilter ();
			ASSERT_VALID (m_pDefaultFilter);
		}

		m_pDefaultFilter->SetFilter (this);
	}
}
//****************************************************************************************
BCGPGRID_FILTERCALLBACK CBCGPGridCtrl::GetFilterCallbackFunct ()
{
	return m_pfnFilterCallback;
}
//****************************************************************************************
BOOL CBCGPGridCtrl::FilterItem (const CBCGPGridRow* pRow)
	// returns TRUE, if item is hidden (filtered)
{
	ASSERT_VALID (this);

	if (IsRowFilteredByFilterBar (pRow))
	{
		return TRUE;
	}

	if (!m_bFilter)
	{
		return FALSE; // show item
	}

	ASSERT_VALID (pRow);

	if (m_pfnFilterCallback != NULL)
	{
		return (BOOL)m_pfnFilterCallback ((WPARAM) pRow, (LPARAM) m_lFilterParam);
	}
// 	else if (m_pDefaultFilter != NULL)
// 	{
// 		return (BOOL)m_pDefaultFilter->pfnFilterCallback((WPARAM) pRow, (LPARAM)m_pDefaultFilter);
// 	}

	return FALSE; // show item
}
//****************************************************************************************
// Helper structures for Filter Bar feature:

void BCGP_GRID_FILTER_PARAM::Copy (const BCGP_GRID_FILTER_PARAM& src)
{
	nColumn = src.nColumn;
	strFilter = src.strFilter;
}

BOOL BCGP_GRID_FILTER_PARAM::IsEmpty () const
{
	if (nColumn == -1)
	{
		return TRUE;
	}
	
	return strFilter.IsEmpty ();
}

void BCGP_GRID_FILTERBAR_PARAM::Clear ()
{
	bNoCase = FALSE;
	lstFilter.RemoveAll ();
}

IMPLEMENT_DYNAMIC(CBCGPGridFilterEdit, CBCGPEdit);

CBCGPGridFilterEdit::CBCGPGridFilterEdit (CBCGPGridCtrl* pGrid, int nColumn) : CBCGPEdit()
{
	m_pOwnerGrid = pGrid;
	m_nColumnId = nColumn;
}

BEGIN_MESSAGE_MAP(CBCGPGridFilterEdit, CBCGPEdit)
	//{{AFX_MSG_MAP(CBCGPGridFilterEdit)
	ON_CONTROL_REFLECT_EX(EN_CHANGE, OnChange)
 	ON_WM_SETFOCUS()
	ON_WM_KEYDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CBCGPGridFilterEdit::OnChange() 
{
	BOOL bResult = CBCGPEdit::OnChange ();

	if (GetSafeHwnd () != NULL && m_pOwnerGrid->GetSafeHwnd () != NULL)
	{
		CString strNew;
		GetWindowText (strNew);

		m_pOwnerGrid->OnFilterBarChanged (m_nColumnId, strNew);
	}

	return bResult;
}

void CBCGPGridFilterEdit::OnSetFocus(CWnd* pOldWnd)
{
	CBCGPEdit::OnSetFocus(pOldWnd);

	if (m_pOwnerGrid->GetSafeHwnd () != NULL)
	{
		m_pOwnerGrid->OnFilterBarSetFocus (m_nColumnId);
	}
}

void CBCGPGridFilterEdit::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if (nChar == VK_ESCAPE)
	{
		if (!m_bTextIsEmpty)
		{
			SetWindowText(_T(""));
		}
		else if (m_pOwnerGrid->GetSafeHwnd () != NULL)
		{
			m_pOwnerGrid->OnFilterBarClearAll ();
		}
	}
	else if (nChar == VK_TAB)
	{
		if (m_pOwnerGrid->GetSafeHwnd () != NULL)
		{
			m_pOwnerGrid->OnFilterBarTab (::GetAsyncKeyState (VK_SHIFT) & 0x8000);
		}
	}

	CBCGPEdit::OnKeyDown(nChar, nRepCnt, nFlags);
}
//****************************************************************************************
void CBCGPGridCtrl::EnableFilterBar (BOOL bEnable, LPCTSTR lpszPrompt, BOOL bCaseSensitive,
									 const CList<int, int>* pListOfColumnIndexes,
									 BOOL bUpdate)
{
	if (bEnable)
	{
		ASSERT(lpszPrompt != NULL);
	}

	m_bFilterBar = bEnable;
	m_paramsFilterBar.Clear ();
	m_paramsFilterBar.bNoCase = !bCaseSensitive;

	if (bEnable)
	{
		if (!m_ImageSearch.IsValid())
		{
			CBCGPLocalResource localRes;

			m_ImageSearch.Load(globalData.Is32BitIcons () ?
				IDB_BCGBARRES_SEARCH32 : IDB_BCGBARRES_SEARCH);
			m_ImageSearch.SetSingleImage();
			m_ImageSearch.SetTransparentColor(globalData.clrBtnFace);
		}
		
		if (m_btnFilterClear.GetSafeHwnd () == NULL)
		{
			CBCGPLocalResource localRes;

			CRect rectBtn;
			m_btnFilterClear.Create (_T(""), WS_CHILD | WS_VISIBLE, rectBtn, this, ID_FILTERBAR_BUTTON);
			m_btnFilterClear.m_nFlatStyle = CBCGPButton::BUTTONSTYLE_FLAT;
			m_btnFilterClear.SetImage (globalData.Is32BitIcons () ? IDB_BCGBARRES_CLEAR32 : IDB_BCGBARRES_CLEAR);
			m_btnFilterClear.m_bDrawFocus = FALSE;
			m_btnFilterClear.m_bVisualManagerStyle = TRUE;
			
			CString strTooltip;
			strTooltip.LoadString (IDS_BCGBARRES_GRID_CLEARFILTER);
			m_btnFilterClear.SetTooltip (strTooltip);
		}
	}
	else
	{
		if (m_btnFilterClear.GetSafeHwnd () != NULL)
		{
			m_btnFilterClear.ShowWindow(SW_HIDE);
		}

		if (m_ImageSearch.IsValid())
		{
			m_ImageSearch.Clear();
		}
	}

	for (int iColumn = 0; iColumn < GetColumnsInfo ().GetColumnCount (); iColumn++)
	{
		EnableColumnFilterEdit (
			bEnable && ((pListOfColumnIndexes == NULL) || (pListOfColumnIndexes->Find(iColumn) != NULL)), 
			iColumn, lpszPrompt);
	}

	if (bEnable)
	{
		ModifyStyle(0, WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
	}
	
	if (bUpdate)
	{
		AdjustLayout ();
	}
}
//****************************************************************************************
BOOL CBCGPGridCtrl::EnableColumnFilterEdit (BOOL bEnable, int nColumn, LPCTSTR lpszPrompt)
{
	ASSERT_VALID (this);
	ASSERT(lpszPrompt != NULL);

	if (GetSafeHwnd () == NULL)
	{
		ASSERT (FALSE); // the grid must be created
		return FALSE;
	}

	if (!m_bFilterBar && bEnable)
	{
		return FALSE;
	}

	CWnd* pCtrl = bEnable ? OnCreateFilterBarCtrl (nColumn, lpszPrompt) : NULL;

	CWnd* pOldCtrl = GetColumnsInfo ().GetColumnFilterBarCtrl (nColumn);
	if (pOldCtrl->GetSafeHwnd () != NULL)
	{
		pOldCtrl->DestroyWindow ();
		delete pOldCtrl;
	}

	return GetColumnsInfo ().SetColumnFilterBarCtrl (nColumn, pCtrl);
}
//****************************************************************************************
CWnd* CBCGPGridCtrl::OnCreateFilterBarCtrl (int nColumn, LPCTSTR lpszPrompt)
{
	ASSERT(lpszPrompt != NULL);

	CBCGPEdit* pEdit = new CBCGPGridFilterEdit (this, nColumn);

	CRect rectDummy;
	pEdit->Create (WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, rectDummy, this, (UINT)-1);

	pEdit->EnableSearchMode (TRUE, lpszPrompt);
	pEdit->SetFont (GetFont ());
	pEdit->SetMargins (TEXT_MARGIN - m_nEditLeftMargin, TEXT_MARGIN - m_nEditRightMargin);

	return pEdit;
}
//****************************************************************************************
void CBCGPGridCtrl::SetFocusToFilterBar (int nColumn)
{
	ASSERT_VALID (this);
	
	if (!m_bFilterBar || GetSafeHwnd () == NULL)
	{
		return;
	}

	CWnd* pCtrl = GetColumnsInfo ().GetColumnFilterBarCtrl (nColumn);
	if (pCtrl->GetSafeHwnd () != NULL)
	{
		if (pCtrl->IsWindowVisible ())
		{
			pCtrl->SetFocus ();
		}
		else
		{
			SetFocus ();
		}

		m_nFocusedFilter = nColumn;

		EnsureVisibleColumn (nColumn);
	}
}
//****************************************************************************************
BOOL CBCGPGridCtrl::IsRowFilteredByFilterBar (const CBCGPGridRow* pRow)
	// returns TRUE, if item is hidden (filtered)
{
	ASSERT_VALID (this);
	
	if (!m_bFilterBar)
	{
		return FALSE;	// show item
	}

	ASSERT_VALID (pRow);

	if (pRow->IsGroup ())
	{
		return FALSE; // do not hide groups
	}
	
	//--------------------------------------------------------------
	// Compare m_paramsFilterBar.lstFilter with content in columns:
	//--------------------------------------------------------------
	for (POSITION pos = m_paramsFilterBar.lstFilter.GetHeadPosition (); pos != NULL; )
	{
		BCGP_GRID_FILTER_PARAM& param = m_paramsFilterBar.lstFilter.GetNext (pos);
		
		if (!param.IsEmpty ())
		{
			CBCGPGridItem* pItem = pRow->GetItem (param.nColumn);
			if (pItem == NULL)
			{
				continue;
			}
			
			if (IsItemFilteredByFilterBar (pItem, param.nColumn, param.strFilter))
			{
				return TRUE; // hide
			}
		}
	}
	
	return FALSE; // show all
}
//****************************************************************************************
BOOL CBCGPGridCtrl::IsItemFilteredByFilterBar (CBCGPGridItem* pItem, int /*nColumn*/, const CString& strFilter)
{
	ASSERT_VALID (pItem);
	
	if (m_paramsFilterBar.bNoCase)
	{
		CString strItem = pItem->GetLabel ();
		strItem.MakeLower ();
		
		if (strItem.Find (strFilter) == -1)
		{
			return TRUE; // hide
		}
	}
	else
	{
		if (pItem->GetLabel ().Find (strFilter) == -1)
		{
			return TRUE; // hide
		}
	}

	return FALSE; // show
}
//****************************************************************************************
void CBCGPGridCtrl::OnFilterBarChanged (int nColumn, const CString& strNewSearch)
{
	ASSERT_VALID (this);
	
	if (GetSafeHwnd () == NULL)
	{
		return;
	}

	if (nColumn < 0 || nColumn > GetColumnsInfo ().GetColumnCount ())
	{
		return;
	}
	
	// find search item by column
	POSITION posFind = NULL;
	for (POSITION pos = m_paramsFilterBar.lstFilter.GetHeadPosition (); pos != NULL; )
	{
		POSITION posSave = pos;
		if (m_paramsFilterBar.lstFilter.GetNext (pos).nColumn == nColumn)
		{
			posFind = posSave;
		}
	}
	
	BOOL bChanged = FALSE;

	CString strNew = strNewSearch;
	
	if (strNew.IsEmpty ())
	{
		// remove
		if (posFind != NULL)
		{
			bChanged = !m_paramsFilterBar.lstFilter.GetAt (posFind).IsEmpty ();
			m_paramsFilterBar.lstFilter.RemoveAt (posFind);
			posFind = NULL;
		}
	}
	else
	{
		if (m_paramsFilterBar.bNoCase)
		{
			strNew.MakeLower ();
		}
		
		// update
		if (posFind != NULL)
		{
			BCGP_GRID_FILTER_PARAM& search = m_paramsFilterBar.lstFilter.GetAt (posFind);
			if (search.strFilter.Compare (strNew) != 0)
			{
				bChanged = TRUE;
				search.strFilter = strNew;
			}
		}
		
		// add
		else
		{
			bChanged = TRUE;
			BCGP_GRID_FILTER_PARAM paramNew (nColumn, strNew);
			m_paramsFilterBar.lstFilter.AddTail (paramNew);
		}
	}
	
	if (bChanged)
	{
		OnFilterBarUpdate (nColumn);
	}
}
//****************************************************************************************
void CBCGPGridCtrl::OnFilterBarClearAll ()
{
	ASSERT_VALID (this);
	
	if (GetSafeHwnd () == NULL)
	{
		return;
	}

	BOOL m_bNoFilterBarUpdate = TRUE;

	for (int iColumn = 0; iColumn < GetColumnsInfo ().GetColumnCount (); iColumn++)
	{
		CWnd* pCtrl = GetColumnsInfo ().GetColumnFilterBarCtrl (iColumn);
		if (pCtrl->GetSafeHwnd () != NULL)
		{
			pCtrl->SetWindowText (_T(""));
		}
	}
	m_nFocusedFilter = -1;

	m_bNoFilterBarUpdate = FALSE;
	OnFilterBarUpdate (-1);

	SetFocus ();
}
//****************************************************************************************
void CBCGPGridCtrl::OnFilterBarSetFocus (int nColumn)
{
	ASSERT_VALID (this);
	
	if (nColumn == -1 || nColumn >= 0 && nColumn < GetColumnsInfo ().GetColumnCount (FALSE))
	{
		m_nFocusedFilter = nColumn;
	}
}
//****************************************************************************************
void CBCGPGridCtrl::OnFilterBarTab (BOOL bShift)
{
	ASSERT_VALID (this);

	if (m_nFocusedFilter < 0 || m_nFocusedFilter >= GetColumnsInfo ().GetColumnCount (FALSE))
	{
		return;
	}

	int nPos = GetColumnsInfo ().IndexToOrder (m_nFocusedFilter);
	int nPosLastVisibleColumn = GetColumnsInfo ().GetColumnCount (TRUE) - 1;

	if (!bShift && nPos >= 0 && nPos < nPosLastVisibleColumn)
	{
		m_nFocusedFilter = GetColumnsInfo ().OrderToIndex (++nPos);
	}
	else if (bShift && nPos > 0 && nPos <= nPosLastVisibleColumn)
	{
		m_nFocusedFilter = GetColumnsInfo ().OrderToIndex (--nPos);
	}

	SetFocusToFilterBar (m_nFocusedFilter);
}
//****************************************************************************************
void CBCGPGridCtrl::OnFilterBarUpdate (int /*nColumn*/)
{
	if (m_bNoFilterBarUpdate)
	{
		return;
	}

	SetCurSel (NULL, FALSE);
	AdjustLayout ();
}
//****************************************************************************************
void CBCGPGridCtrl::EnableDefaultFilterMenuPopup (BOOL bEnable, UINT uiMenuResId, UINT uiFilterCmd)
{
	m_bDefaultFilterMenuPopup = bEnable;

	m_uiDefaultFilterMenuResId = uiMenuResId;	// if 0, menu is created on the fly
	m_uiDefaultFilterApplyCmd = uiFilterCmd;	// if 0, uses ID_DEFAULTFILTER_APPLY
}
//****************************************************************************************
BOOL CBCGPGridCtrl::ShowFilterMenu(HMENU hMenu, UINT uiFilterCmd, int nColumn, 
								   BCGP_FILTER_COLUMN_INFO* pFilterColumnInfo,
								   const CStringList& lstValues,
								   CPoint pt)
{
	if (pFilterColumnInfo == NULL)
	{
		if (m_pDefaultFilter == NULL)
		{
			ASSERT(FALSE); // Call EnableFilter() first or specify pFilterColumnInfo
			return FALSE;
		}

		ASSERT_VALID (m_pDefaultFilter);
		pFilterColumnInfo = m_pDefaultFilter->GetColumnInfo (nColumn);
	}

	CStringList lstColumnValues;
	if (lstValues.IsEmpty())
	{
		GetColumnValuesList(nColumn, lstColumnValues);
	}

	CBCGPGridFilterPopupMenu* pPopupMenu = new CBCGPGridFilterPopupMenu(
		uiFilterCmd, lstColumnValues.IsEmpty() ? lstValues : lstColumnValues, *pFilterColumnInfo, this);

	CBCGPPopupMenu* pMenuActive = CBCGPPopupMenu::GetActiveMenu ();
	if (pMenuActive != NULL)
	{
		pMenuActive->SendMessage (WM_CLOSE);
	}

	if (pt == CPoint(-1, -1) && nColumn >= 0)
	{
		CRect rectHeaderColumn;
		GetColumnsInfo().GetColumnRect(nColumn, rectHeaderColumn);
		
		CRect rectButton = GetHeaderMenuButtonRect(rectHeaderColumn, nColumn);
		ClientToScreen(&rectButton);

		pt.x = rectButton.left;
		pt.y = rectButton.bottom;
	}


	return pPopupMenu->Create(this, pt.x, pt.y, hMenu, FALSE, FALSE);
}
//****************************************************************************************
int CBCGPGridCtrl::GetColumnValuesList(int nColumn, CStringList& lstValues, BOOL bSorted, BOOL bVisibleOnly)
{
	lstValues.RemoveAll();

	if (IsVirtualMode())
	{
		return 0;
	}

	for (int nRow = 0; nRow < GetRowCount(); nRow++)
	{
		CBCGPGridRow* pRow = GetRow(nRow);
		if (pRow != NULL)
		{
			ASSERT_VALID(pRow);

			if (bVisibleOnly && !pRow->IsItemVisible ())
			{
				continue;
			}

			CBCGPGridItem* pItem = pRow->GetItem(nColumn);
			if (pItem != NULL)
			{
				ASSERT_VALID(pItem);

				CString str = pItem->GetLabel();

				if (lstValues.Find(str) == NULL)
				{
					if (!bSorted)
					{
						lstValues.AddTail(str);
					}
					else
					{
						BOOL bAdded = FALSE;
						for (POSITION pos = lstValues.GetHeadPosition(); pos != NULL;)
						{
							POSITION posSaved = pos;

							if (str < lstValues.GetNext(pos))
							{
								lstValues.InsertBefore(posSaved, str);
								bAdded = TRUE;
								break;
							}
						}

						if (!bAdded)
						{
							lstValues.AddTail(str);
						}
					}
				}
			}
		}
	}

	return (int)lstValues.GetCount();
}
//****************************************************************************************
void CBCGPGridCtrl::AdjustFilterBarCtrls ()
{
	if (!m_bFilterBar)
	{
		return;
	}

	CRect rectWorkArea = m_rectFilterBar;
	rectWorkArea.left += m_rectRowHeader.Width ();

	if (m_bScrollVert && m_nVertScrollTotal > 0)
	{
		rectWorkArea.right -= GetSystemMetrics (SM_CXVSCROLL);
	}

	//----------------------------
	// "Clear All Filters" Button:
	//----------------------------
	if (m_btnFilterClear.GetSafeHwnd () != NULL)
	{
		CRect rectLeft = m_rectFilterBar;
		rectLeft.right = min (rectLeft.left + m_rectRowHeader.Width (), rectWorkArea.right);

		if (m_rectFilterBar.IsRectEmpty () || rectLeft.Width () < GetBaseHeight () ||
			m_paramsFilterBar.lstFilter.IsEmpty ())
		{
			m_btnFilterClear.ShowWindow(SW_HIDE);
		}
		else
		{
			if (!m_btnFilterClear.IsWindowVisible())
			{
				m_btnFilterClear.ShowWindow(SW_SHOWNOACTIVATE);
			}
			
			m_btnFilterClear.SetWindowPos (NULL, rectLeft.left, rectLeft.top, rectLeft.Width (), rectLeft.Height (),
				SWP_NOZORDER | SWP_NOACTIVATE);
			m_btnFilterClear.RedrawWindow();
		}
	}

	//---------------------
	// Filter Bar controls:
	//---------------------
	CRect rect;
	for (int iColumn = 0; iColumn < GetColumnsInfo ().GetColumnCount (); iColumn++)
	{
		CWnd* pCtrl = GetColumnsInfo ().GetColumnFilterBarCtrl (iColumn);
		if (pCtrl->GetSafeHwnd () != NULL)
		{
			ASSERT_VALID (pCtrl);

			rect.SetRectEmpty ();
			GetColumnsInfo ().GetColumnRect (iColumn, rect);
			rect.top = m_rectFilterBar.top;
			rect.bottom = m_rectFilterBar.bottom - 1;

			int nFreezeOffset = (GetColumnsInfo ().IsFreezeColumnsEnabled () &&
				GetColumnsInfo ().IsColumnFrozen (iColumn)) ? 0 : GetColumnsInfo ().GetFreezeOffset ();

 			rect.left = max (rect.left, rectWorkArea.left + nFreezeOffset);
 			rect.right = min (rect.right - 1, rectWorkArea.right);

			if (rect.Width () < 2 * GetBaseHeight () || m_rectFilterBar.IsRectEmpty ())
 			{
 				pCtrl->ShowWindow(SW_HIDE);
 			}
			else
			{
				if (!pCtrl->IsWindowVisible())
				{
 					pCtrl->ShowWindow(SW_SHOWNOACTIVATE);
					if (m_nFocusedFilter == iColumn)
					{
						pCtrl->SetFocus ();
					}
				}

				pCtrl->SetWindowPos (NULL, rect.left, rect.top, rect.Width (), rect.Height (),
					SWP_NOZORDER | SWP_NOACTIVATE);
				pCtrl->RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_FRAME);
			}
		}
	}
}
//****************************************************************************************
BOOL CBCGPGridCtrl::IsHeaderMenuButtonEnabled (int /*nColumn*/) const
{
	return IsFilterEnabled ();
}
//****************************************************************************************
int CBCGPGridCtrl::GetHeaderMenuButtonImageIndex (int nColumn, BOOL bSortArrow) const
{
	BOOL bFilterColumn = /*(m_param.nCol == nColumn && !m_param.bAll)*/FALSE;
	int nIndex = bFilterColumn ? 3 : 0;
	
	if (bSortArrow)
	{
		int nSortVal = m_Columns.GetColumnState (nColumn);
		if (nSortVal > 0)
		{
			nIndex += 1;
		}
		else if (nSortVal < 0)
		{
			nIndex += 2;
		}
	}

	if (nIndex >= m_DefaultHeaderBtnImages.GetCount())
	{
		ASSERT(FALSE);
		return -1;
	}

	return nIndex;
}
//****************************************************************************************
void CBCGPGridCtrl::OnDrawHeaderMenuButton (CDC* pDC, CRect rect, int nColumn, BOOL bSortArrow)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);

	//------------------------
	// Draw the header button:
	//------------------------
	int nImage = GetHeaderMenuButtonImageIndex (nColumn, bSortArrow);
	CBCGPToolBarImages* pImagesHeaderBtn = (m_pImagesHeaderBtn == NULL) ? &m_DefaultHeaderBtnImages : m_pImagesHeaderBtn;

	if (nImage >= 0 && nImage < pImagesHeaderBtn->GetCount())
	{			
		// draw button border:
		visualManager->OnDrawGridHeaderMenuButton (this, pDC, rect, nColumn == GetColumnsInfo ().GetHighlightColumnBtn (),
			FALSE, FALSE);

		// draw image:
		const CSize sizeImage = pImagesHeaderBtn->GetImageSize ();

		CPoint pt = rect.TopLeft ();
		pt.x ++;
		pt.y += max (0, (rect.Height () - sizeImage.cy) / 2);

		if (rect.Width () > sizeImage.cx)
		{
			pt.x += (rect.Width () - sizeImage.cx) / 2;

			CBCGPDrawState ds;
			pImagesHeaderBtn->PrepareDrawImage (ds, sizeImage);
			pImagesHeaderBtn->Draw (pDC, pt.x, pt.y, nImage, FALSE);
			pImagesHeaderBtn->EndDrawImage (ds);
		}

		return;
	}

	if (bSortArrow)
	{
		int nSortVal = GetColumnsInfo ().GetColumnState (nColumn);
		if (nSortVal != 0)
		{
			//-----------------
			// Draw sort arrow:
			//-----------------
			int nArrowMargin = 5;

			CRect rectArrow = rect;
			rectArrow.DeflateRect (nArrowMargin, nArrowMargin);

			if (rectArrow.Width () >= rectArrow.Height ())
			{
				rectArrow.left = rectArrow.right - rectArrow.Height ();
				rect.right = rectArrow.left - 1;

				int dy2 = (int) (.134 * rectArrow.Width ());
				rectArrow.DeflateRect (0, dy2);
			}

			OnDrawSortArrow (pDC, rectArrow, nSortVal > 0);
		}
	}
}
//****************************************************************************************
void CBCGPGridCtrl::OnHeaderMenuButtonClick (int nColumn, CRect /*rectMenuButton*/)
{
	ASSERT_VALID (this);
	
	CWnd* pOwner = GetOwner();
	if (pOwner->GetSafeHwnd() != NULL)
	{
		pOwner->SendMessage(BCGM_GRID_COLUMN_BTN_CLICK, GetDlgCtrlID (), nColumn);
	}

	//-------------------
	// Popup filter menu:
	//-------------------
	if (IsFilterEnabled() && m_bDefaultFilterMenuPopup)
	{
		CMenu menu;
		UINT uiCmd = (m_uiDefaultFilterApplyCmd != 0) ? m_uiDefaultFilterApplyCmd : ID_DEFAULTFILTER_APPLY;

		if (m_uiDefaultFilterMenuResId != 0 && menu.LoadMenu(m_uiDefaultFilterMenuResId))
		{
			ShowFilterMenu(menu.GetSubMenu(0)->GetSafeHmenu(), uiCmd, nColumn);
		}
		else if (menu.CreatePopupMenu ())
		{
			ShowFilterMenu(menu.GetSafeHmenu(), uiCmd, nColumn);
		}
		else
		{
			TRACE(_T("CBCGPGridCtrl::OnHeaderMenuButtonClick: Can't create popup menu!\n"));
		}
	}
}
//****************************************************************************************
void CBCGPGridCtrl::OnDefaultFilterMenuApply ()
{
	AdjustLayout ();
}
//****************************************************************************************
CRect CBCGPGridCtrl::GetHeaderMenuButtonRect (CRect rectItem, int nColumn) const
{
	CRect rectBtn;
	rectBtn.SetRectEmpty ();

	if (IsHeaderMenuButtonEnabled (nColumn))
	{
		rectBtn = rectItem;
		rectBtn.left = max (rectBtn.right - GetButtonWidth (), rectBtn.left);
	}

	return rectBtn;
}
//****************************************************************************************
void CBCGPGridCtrl::GetHeaderMenuButtonTooltip (int /*nColumn*/, CString& strText) const
{
	strText.Empty ();
}
//****************************************************************************
void CBCGPGridCtrl::GetHeaderTooltip (int nColumn, CPoint pt, CString& strText) const
{
	GetColumnsInfo ().GetHeaderTooltip (nColumn, pt, strText);
}
//****************************************************************************************
void CBCGPGridCtrl::OnSelectAllClick ()
{
	SelectAll ();
}
//****************************************************************************************
void CBCGPGridCtrl::DoColumnHeaderClick (int nColumnHit, CPoint point, CBCGPGridColumnsInfo::ClickArea clickAreaHeader)
{
	const BOOL bShift = (::GetAsyncKeyState (VK_SHIFT) & 0x8000) != 0 && !m_bIgnoreShiftBtn;
	const BOOL bCtrl = (::GetAsyncKeyState (VK_CONTROL) & 0x8000) != 0 && !m_bIgnoreCtrlBtn;

	if (clickAreaHeader == CBCGPGridColumnsInfo::ClickHeaderButton)
	{
		CRect rectHeaderColumn;
		GetColumnsInfo ().GetColumnRect (nColumnHit, rectHeaderColumn);
		
		OnHeaderMenuButtonClick (nColumnHit, GetHeaderMenuButtonRect (rectHeaderColumn, nColumnHit));
		return;
	}
	
	if ((m_dwHeaderFlags & BCGP_GRID_HEADER_SELECT) != 0 &&
		(m_dwHeaderFlags & BCGP_GRID_HEADER_MOVE_ITEMS) == 0 &&
		!m_bWholeRowSel)
	{
		// Select column
		CBCGPGridItemID idHeader (-1, nColumnHit);
		
		DWORD dwSelMode = SM_NONE;
		if (!idHeader.IsNull ())
		{
			dwSelMode = SM_FIRST_CLICK |
				(bCtrl ? SM_ADD_SEL_GROUP :
			(bShift ? SM_CONTINUE_SEL_GROUP : SM_SINGE_SEL_GROUP));
			
			if (bCtrl && m_bInvertSelOnCtrl)
			{
				dwSelMode |= SM_INVERT_SEL;
			}
		}
		m_bNoUpdateWindow = TRUE; // prevent flickering
		SetCurSel (idHeader, dwSelMode);
		m_bNoUpdateWindow = FALSE;
		
		m_bHeaderRowSelecting = FALSE;
		m_bHeaderColSelecting = TRUE;
		StartSelectItems ();
		return;
	}
	else
	{
		CRect rectHeaderColumn;
		GetColumnsInfo ().GetColumnRect (nColumnHit, rectHeaderColumn);
		
		m_ptStartDrag = point;
		StartDragColumn (nColumnHit, rectHeaderColumn, FALSE, FALSE);
		return;
	}
}
//****************************************************************************************
void CBCGPGridCtrl::DoRowHeaderClick (CBCGPGridRow* pHitHeaderRow, CPoint point, CRect rectRowHeader)
{
	ASSERT_VALID (pHitHeaderRow);
	
	if ((m_dwRowHeaderFlags & BCGP_GRID_HEADER_MOVE_ITEMS) != 0)
	{
		// drag selected items only
		if (IsRowSelected (pHitHeaderRow->GetRowId (), TRUE))
		{
			OnRowHeaderClick (pHitHeaderRow, rectRowHeader);

			StartDragItems (point);
		}
		else if ((m_dwRowHeaderFlags & BCGP_GRID_HEADER_SELECT) != 0)
		{
			DoRowHeaderSelectClick (pHitHeaderRow, point, rectRowHeader);
		}
		else
		{
			OnRowHeaderClick (pHitHeaderRow, rectRowHeader);
		}
		
	}
	else if ((m_dwRowHeaderFlags & BCGP_GRID_HEADER_SELECT) != 0)
	{
		DoRowHeaderSelectClick (pHitHeaderRow, point, rectRowHeader);
	}
	else
	{
		OnRowHeaderClick (pHitHeaderRow, rectRowHeader);
	}
}
//****************************************************************************************
void CBCGPGridCtrl::DoRowHeaderSelectClick (CBCGPGridRow* pHeaderRow, CPoint point, CRect rect)
{
	ASSERT_VALID (pHeaderRow);

	if (m_bSingleSel)
	{
		OnRowHeaderClick (pHeaderRow, rect);
		SelectRow (pHeaderRow->GetRowId ());
	}
	else
	{
		const BOOL bShift = (::GetAsyncKeyState (VK_SHIFT) & 0x8000) != 0 && !m_bIgnoreShiftBtn;
		const BOOL bCtrl = (::GetAsyncKeyState (VK_CONTROL) & 0x8000) != 0 && !m_bIgnoreCtrlBtn;

		// Set selection (first click):
		CBCGPGridItemID idHeader (pHeaderRow->GetRowId ());
		BOOL bSelChanged = idHeader != m_idLastSel;
		
		DWORD dwSelMode = SM_NONE;
		if (!idHeader.IsNull ())
		{
			dwSelMode = SM_FIRST_CLICK |
				(m_bSingleSel ? SM_SINGE_SEL_GROUP :
				(bCtrl ? SM_ADD_SEL_GROUP :
				(bShift ? SM_CONTINUE_SEL_GROUP : SM_SINGE_SEL_GROUP)));
			
			if (bCtrl && m_bInvertSelOnCtrl)
			{
				dwSelMode |= SM_INVERT_SEL;
			}
		}
		m_bNoUpdateWindow = TRUE; // prevent flickering
		SetCurSel (idHeader, dwSelMode);
		m_bNoUpdateWindow = FALSE;
		
		// Defer the row header click:
		SetTimer (GRID_CLICKVALUE_TIMER_ID, GRID_CLICKVALUE_TIMER_INTERVAL, NULL);
		m_bClickTimer = TRUE;
		m_ptClickOnce = point;
		m_bIsFirstClick = bSelChanged;
		m_bIsButtonClick = FALSE;
		m_bHeaderRowSelecting = TRUE;
		m_bHeaderColSelecting = FALSE;
		
		StartSelectItems ();
	}
}
//****************************************************************************************
void CBCGPGridCtrl::OnRowHeaderClick (CBCGPGridRow* /*pRow*/, CRect)
{
}
//****************************************************************************************
void CBCGPGridCtrl::OnRowHeaderDblClick (CBCGPGridRow* pRow, CPoint point, CRect)
{
	ASSERT_VALID(pRow);
	pRow->OnDblClick (point);
}
//****************************************************************************************
int CBCGPGridCtrl::CompareItems (const CBCGPGridRow* pRow1, const CBCGPGridRow* pRow2, int iColumn) const
{ 
	ASSERT_VALID (this);
	ASSERT_VALID (pRow1);
	ASSERT_VALID (pRow2);

	if (!pRow1->HasValueField () ||
		!pRow2->HasValueField ())
	{
		return 0;
	}

	int nColumnState = GetColumnsInfo ().GetColumnState (iColumn);
	BOOL bIsGroupColumn = GetColumnsInfo ().IsGroupColumn (iColumn);

	if (nColumnState == 0 && !bIsGroupColumn)
	{
		// not sorted
		return 0;
	}

	BOOL bAscending = (nColumnState >= 0);
	int nRes = 0;

	if (pRow1->m_arrRowItems.GetSize () > iColumn &&
		pRow2->m_arrRowItems.GetSize () > iColumn)
	{
		// compare grid items
		nRes = CompareItems (pRow1->m_arrRowItems [iColumn], pRow2->m_arrRowItems [iColumn]);
	}

	if (!bAscending)
	{
		nRes = -nRes;
	}
		
	return nRes;
}
//*****************************************************************************************

#define DoCompareItems(item1, item2) ((item1) < (item2) ? -1 : (item1 == item2) ? 0 : 1)

int CBCGPGridCtrl::CompareItems (const CBCGPGridItem* pItem1, const CBCGPGridItem* pItem2) const
{ 
	ASSERT_VALID (this);
	ASSERT_VALID (pItem1);
	ASSERT_VALID (pItem2);

	const _variant_t& var1 = pItem1->m_varValue;
	const _variant_t& var2 = pItem2->m_varValue;

	if (var1.vt == var2.vt)
	{
		switch (var1.vt)
		{
		case VT_BSTR:
			{
                HRESULT hr = ::VarBstrCmp(var1.bstrVal, var2.bstrVal, LOCALE_USER_DEFAULT, 0);
				switch(hr)
				{
				case VARCMP_LT:
					return -1;
				case VARCMP_EQ:
					return 0;
				case VARCMP_GT:
					return 1;
				default:
					return 1;
				}
			}

		case VT_I2:
		case VT_UI2:
			return DoCompareItems ((short) var1, (short) var2);

		case VT_I4:
		case VT_INT:
		case VT_UINT:
		case VT_UI4:
			return DoCompareItems ((long) var1, (long) var2);

#if _MSC_VER >= 1500
		case VT_I8:
		case VT_UI8:
			return DoCompareItems ((LONGLONG) var1, (LONGLONG) var2);
#endif

		case VT_UI1:
			return DoCompareItems ((short)(BYTE) var1, (short)(BYTE) var2);

		case VT_R4:
			return DoCompareItems ((float)  var1, (float) var2);

		case VT_R8:
			return DoCompareItems ((double)  var1, (double) var2);

		case VT_BOOL:
			return DoCompareItems ((bool)  var1, (bool) var2);

		case VT_DATE:
			return DoCompareItems ((DATE)  var1, (DATE) var2);
		}
	}

	CString str1 = ((CBCGPGridItem*) pItem1)->GetLabel ();
	CString str2 = ((CBCGPGridItem*) pItem2)->GetLabel ();

	// if image without text - sort by image index
	if (str1.IsEmpty () && str2.IsEmpty ())
	{
		return DoCompareItems (pItem1->m_iImage, pItem2->m_iImage);
	}

	return str1.Compare (str2);
}
//*****************************************************************************************
int CBCGPGridCtrl::CompareGroup (const CBCGPGridRow* pRow1, const CBCGPGridRow* pRow2, int iColumn)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pRow1);
	ASSERT_VALID (pRow2);

	int nGroup1 = OnGetGroupNumber (pRow1, iColumn);
	int nGroup2 = OnGetGroupNumber (pRow2, iColumn);

	if (nGroup1 != -1 || nGroup2 != -1)
	{
		int nColumnState = GetColumnsInfo ().GetColumnState (iColumn);
		BOOL bIsGroupColumn = GetColumnsInfo ().IsGroupColumn (iColumn);

		if (nColumnState == 0 && !bIsGroupColumn)
		{
			// not sorted
			return 0;
		}

		int nRes = nGroup1 - nGroup2;

		BOOL bAscending = (nColumnState >= 0);
		if (!bAscending)
		{
			nRes = -nRes;
		}

		return nRes;
	}

	return CBCGPGridCtrl::CompareItems (pRow1, pRow2, iColumn);
}
//*****************************************************************************************
CString CBCGPGridCtrl::GetGroupName (int nGroupCol, CBCGPGridItem* pItem)
{
	CString strValue = pItem->GetLabel ();
	CString strColumn = GetColumnsInfo ().GetColumnName (nGroupCol);

	CString strGroup;
	strGroup.Format (_T("%s: %s"), strColumn, strValue);

	return strGroup;
}
//*****************************************************************************************
BOOL CBCGPGridCtrl::GetRowName (CBCGPGridRow* pRow, CString& strName)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pRow);

    BOOL bIsAutoGroup = pRow->IsGroup () && (pRow->m_dwFlags & BCGP_GRID_ITEM_AUTOGROUP) != 0;
	if (!bIsAutoGroup)
	{
		return FALSE;
	}

    CBCGPGridItem* pItem = pRow->GetItem (0);
	if (pItem != NULL)
	{
		ASSERT_VALID (pItem);

		strName = pItem->FormatItem ();
		return !strName.IsEmpty ();
	}

	return FALSE;
}
//*****************************************************************************************
int CBCGPGridCtrl::InsertGroupColumn (int nPos, int nColumn)
{
	int nRes = GetColumnsInfo ().InsertGroupColumn (nPos, nColumn);

	if (nRes != -1)
	{
		SetRebuildTerminalItems ();
		AdjustLayout ();
	}

	return nRes;
}
//*****************************************************************************************
BOOL CBCGPGridCtrl::RemoveGroupColumn (int nPos)
{
	BOOL bRes = GetColumnsInfo ().RemoveGroupColumn (nPos);

	if (bRes)
	{
		SetRebuildTerminalItems ();
		AdjustLayout ();
	}

	return bRes;
}
//*****************************************************************************************
BOOL CBCGPGridCtrl::RemoveGroupColumnByVal (int nColumn)
{
	BOOL bRes = GetColumnsInfo ().RemoveGroupColumnByVal (nColumn);

	if (bRes)
	{
		SetRebuildTerminalItems ();
		ReposItems ();
		
		AdjustLayout ();
	}

	return TRUE;
}
//*******************************************************************************************
int CBCGPGridCtrl::GetGroupColumnRect (int nPos, CRect& rect, CDC* pDC)
{
	ASSERT_VALID (pDC);

	rect.SetRectEmpty ();

	CRect rectClient;
	GetClientRect (rectClient);

	CRect rectGroupByBox = rectClient;
	rectGroupByBox.bottom = min (rectGroupByBox.top + m_nGroupByBoxHeight, rectClient.bottom);

	rectGroupByBox.DeflateRect (BCGPGRID_GROUPBYBOX_HMARGIN, BCGPGRID_GROUPBYBOX_VMARGIN, 0, 0);

	int nItemHeight = m_nBaseHeight + TEXT_MARGIN;
	int nIndex = 0;

	for (int i = 0; i < GetColumnsInfo ().GetGroupColumnCount (); i++)
	{
		int nCol = GetColumnsInfo ().GetGroupColumn (i);
		if (nCol != -1)
		{
			CRect rectItem = rectGroupByBox;
			CString strColumn = GetColumnsInfo ().GetColumnName (nCol);

			int nItemWidth = pDC->GetTextExtent (strColumn).cx + 
				BCGPGRID_GROUPBYBOX_COLUMNWIDTH + TEXT_MARGIN;

			rectItem.bottom = min (rectItem.top + nItemHeight, rectGroupByBox.bottom);
			rectItem.right = min (rectItem.left + nItemWidth, rectGroupByBox.right);

			if (nIndex == nPos)
			{
				rect = rectItem;
				return nCol;
			}

			rectGroupByBox.left += nItemWidth;
			nIndex++;
		}

		rectGroupByBox.DeflateRect (BCGPGRID_GROUPBYBOX_HSPACING, BCGPGRID_GROUPBYBOX_VSPACING, 0, 0);
	}

	return -1;
}
//*****************************************************************************************
BOOL CBCGPGridCtrl::IsGrouping () const
{
	return GetColumnsInfo ().GetGroupColumnCount () > 0;
}
//*****************************************************************************************
int CBCGPGridCtrl::GetExtraHierarchyOffset () const
{
	if (!IsGrouping ())
	{
		if (IsRowHeaderEnabled ())
		{
			return 0; // Do not use left margin if the row header is enabled
		}
	}

	return GetLeftMarginWidth ();
}
//*****************************************************************************************
int CBCGPGridCtrl::AddRow (BOOL bRedraw)
{
	ASSERT_VALID (this);
	
	CBCGPGridRow* pRow = CreateRow ();
	ASSERT_VALID (pRow);

	pRow->SetOwnerList (this);

	const int nColumnsNum = GetColumnsInfo ().GetColumnCount ();
	for (int i = 0; i < nColumnsNum ; i++)
	{
		CBCGPGridItem* pItem = pRow->CreateItem (
			(int) pRow->m_arrRowItems.GetSize (), i);
		ASSERT_VALID (pItem);

		pItem->SetOwnerRow (pRow);
		int nIndex = (int) pRow->m_arrRowItems.Add (pItem);
		pItem->m_nIdColumn = nIndex;
	}

	return AddRow (pRow, bRedraw);
}
//*****************************************************************************************
int CBCGPGridCtrl::AddRow (CBCGPGridRow* pItem, BOOL bRedraw)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pItem);

#ifdef _DEBUG
	if (CBCGPGridCtrl::m_bEnableAssertValidInDebug)
	{
		for (POSITION pos = m_lstItems.GetHeadPosition (); pos != NULL; )
		{
			CBCGPGridRow* pListItem = m_lstItems.GetNext (pos);
			ASSERT_VALID (pListItem);

			if (pListItem == pItem || pListItem->IsSubItem (pItem))
			{
				// Can't ad the same item twice
				ASSERT (FALSE);
				return -1;
			}
		}
	}
#endif // _DEBUG

	pItem->SetOwnerList (this);

	m_lstItems.AddTail (pItem);
	int nIndex = (int) m_lstItems.GetCount () - 1;
	pItem->m_nIdRow = nIndex;

	SetRebuildTerminalItems ();

	if (bRedraw && !m_bPostAdjustLayout && GetSafeHwnd() != NULL)
	{
		PostMessage(BCGM_GRID_ADJUST_LAYOUT);
		m_bPostAdjustLayout = TRUE;
	}

	return nIndex;
}
//*****************************************************************************************
int CBCGPGridCtrl::AddCaptionRow (const CString& strCaption, BOOL bRedraw)
{
	CBCGPGridRow* pRow = CreateCaptionRow (strCaption);
	ASSERT_VALID (pRow);

	pRow->SetOwnerList (this);

	return AddRow (pRow, bRedraw);
}
//*****************************************************************************************
void CBCGPGridCtrl::RebuildIndexes (int nStartFrom)
{
	if (m_bVirtualMode)
	{
		ASSERT (FALSE);
		return;
	}

	const CList<CBCGPGridRow*, CBCGPGridRow*>& lst = 
		(!IsSortingMode () && !IsGrouping ()) ? m_lstItems : m_lstTerminalItems;

	POSITION posStartFrom = NULL;
	int nRowIndex = 0;
	if (nStartFrom >= 0 && nStartFrom < lst.GetCount ())
	{
		posStartFrom = lst.FindIndex (nStartFrom);
		nRowIndex = nStartFrom;
	}

	if (posStartFrom == NULL)
	{
		posStartFrom = lst.GetHeadPosition ();
		nRowIndex = 0;
	}

	for (POSITION pos = posStartFrom; pos != NULL; )
	{
		CBCGPGridRow* pGridItem = lst.GetNext (pos);
		ASSERT_VALID (pGridItem);

		if (pGridItem->IsItemVisible ())
		{
			pGridItem->m_nIdRow = nRowIndex;
			nRowIndex++;
		}
	}
}
//*****************************************************************************************
int CBCGPGridCtrl::InsertRowBefore (int nPos, CBCGPGridRow* pItem, BOOL bRedraw)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pItem);

	if (m_bVirtualMode)
	{
		// Cannot insert row in the virtual mode
		ASSERT (FALSE);
		return -1;
	}

#ifdef _DEBUG
	if (CBCGPGridCtrl::m_bEnableAssertValidInDebug)
	{
		for (POSITION pos = m_lstItems.GetHeadPosition (); pos != NULL; )
		{
			CBCGPGridRow* pListItem = m_lstItems.GetNext (pos);
			ASSERT_VALID (pListItem);

			if (pListItem == pItem || pListItem->IsSubItem (pItem))
			{
				// Can't ad the same item twice
				ASSERT (FALSE);
				return -1;
			}
		}
	}
#endif // _DEBUG

	pItem->SetOwnerList (this);

	POSITION posInsertBefore = NULL;
	if (nPos >= 0 && nPos < m_lstItems.GetCount ())
	{
		posInsertBefore = m_lstItems.FindIndex (nPos);
	}

	int nIndex = -1;
	if (posInsertBefore != NULL)
	{
		// insert before
		m_lstItems.InsertBefore (posInsertBefore, pItem);
		nIndex = nPos;
	}
	else
	{
		// insert first
		m_lstItems.AddHead (pItem);
		nIndex = 0;
	}
	pItem->m_nIdRow = nIndex;

	SetRebuildTerminalItems ();

	if (bRedraw && !m_bPostAdjustLayout && GetSafeHwnd() != NULL)
	{
		PostMessage(BCGM_GRID_ADJUST_LAYOUT);
		m_bPostAdjustLayout = TRUE;
	}

	return nIndex;
}
//*****************************************************************************************
int CBCGPGridCtrl::InsertRowAfter (int nPos, CBCGPGridRow* pItem, BOOL bRedraw)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pItem);

	if (m_bVirtualMode)
	{
		// Cannot insert row in the virtual mode
		ASSERT (FALSE);
		return -1;
	}

#ifdef _DEBUG
	if (CBCGPGridCtrl::m_bEnableAssertValidInDebug)
	{
		for (POSITION pos = m_lstItems.GetHeadPosition (); pos != NULL; )
		{
			CBCGPGridRow* pListItem = m_lstItems.GetNext (pos);
			ASSERT_VALID (pListItem);

			if (pListItem == pItem || pListItem->IsSubItem (pItem))
			{
				// Can't ad the same item twice
				ASSERT (FALSE);
				return -1;
			}
		}
	}
#endif // _DEBUG

	pItem->SetOwnerList (this);

	POSITION posInsertAfter = NULL;
	if (nPos >= 0 && nPos < m_lstItems.GetCount ())
	{
		posInsertAfter = m_lstItems.FindIndex (nPos);
	}

	int nIndex = -1;
	if (posInsertAfter != NULL)
	{
		// insert after
		m_lstItems.InsertAfter (posInsertAfter, pItem);
		nIndex = nPos + 1;
	}
	else
	{
		// add last
		m_lstItems.AddTail (pItem);
		nIndex = (int) m_lstItems.GetCount () - 1;
	}
	pItem->m_nIdRow = nIndex;

	SetRebuildTerminalItems ();

	if (bRedraw && !m_bPostAdjustLayout && GetSafeHwnd() != NULL)
	{
		PostMessage(BCGM_GRID_ADJUST_LAYOUT);
		m_bPostAdjustLayout = TRUE;
	}

	return nIndex;
}
//*****************************************************************************************
int CBCGPGridCtrl::RemoveRow (int nPos, BOOL bRedraw)
{
	ASSERT_VALID (this);

	if (m_bVirtualMode)
	{
		// Cannot remove row in the virtual mode
		ASSERT (FALSE);
		return 0;
	}

	//--------------------------------
	// Collect list of items to delete
	//--------------------------------
	CList <POSITION, POSITION> lstDelItemsPos;
	CList <CBCGPGridRow*, CBCGPGridRow*> lstDelItemsPtr;

	PrepareRemoveRow (nPos, lstDelItemsPos, lstDelItemsPtr);

	//--------------
	// Remove items:
	//--------------
	int nCount = DoRemoveRows (lstDelItemsPos, lstDelItemsPtr);

	if (bRedraw && !m_bPostAdjustLayout && GetSafeHwnd() != NULL)
	{
		PostMessage(BCGM_GRID_ADJUST_LAYOUT);
		m_bPostAdjustLayout = TRUE;
	}

	return nCount;
}
//*****************************************************************************************
void CBCGPGridCtrl::PrepareRemoveRow (int nPos,
									  CList <POSITION, POSITION>& lstDelItemsPos, 
									  CList <CBCGPGridRow*, CBCGPGridRow*>& lstDelItemsPtr)
{
	ASSERT_VALID (this);

	if (m_bVirtualMode)
	{
		// Cannot remove row in the virtual mode
		ASSERT (FALSE);
		return;
	}

	const CList<CBCGPGridRow*, CBCGPGridRow*>& lst = 
		(!IsSortingMode () && !IsGrouping ()) ? m_lstItems : m_lstTerminalItems;
	
	POSITION posRemove = NULL;
	if (nPos >= 0 && nPos < lst.GetCount ())
	{
		posRemove = lst.FindIndex (nPos);
	}

	//--------------------------------
	// Collect list of items to delete
	//--------------------------------
	if (posRemove != NULL)
	{
		CBCGPGridRow* pItemDel = m_lstItems.GetAt (posRemove);
		ASSERT_VALID (pItemDel);

		// if possible store position
		if (!IsSortingMode () && !IsGrouping ())
		{
			lstDelItemsPos.AddTail (posRemove);	// position in m_lstItems
		}
		// store a pointer
		else
		{
			lstDelItemsPtr.AddTail (pItemDel);
		}

		CList<CBCGPGridRow*, CBCGPGridRow*>	lstSubItems;
		pItemDel->GetSubItems (lstSubItems, TRUE);

		for (POSITION posSubItem = lstSubItems.GetHeadPosition ();
			posSubItem != NULL; )
		{
			CBCGPGridRow* pSubItemDel = lstSubItems.GetNext (posSubItem);
			ASSERT_VALID (pSubItemDel);

			// if possible store position
			if (!IsSortingMode () && !IsGrouping ())
			{
				int nPos2 = pSubItemDel->GetRowId ();
				POSITION posRemove2 = NULL;
				if (nPos2 >= 0 && nPos2 < lst.GetCount ())
				{
					posRemove2 = lst.FindIndex (nPos2);
				}

				if (posRemove2 != NULL)
				{
					lstDelItemsPos.AddTail (posRemove2);	// position in m_lstItems
				}
			}
			// store a pointer
			else
			{
				lstDelItemsPtr.AddTail (pSubItemDel);
			}
		}

	}
}
//*****************************************************************************************
int CBCGPGridCtrl::DoRemoveRows (CList <POSITION, POSITION>& lstDelItemsPos,
								 CList <CBCGPGridRow*, CBCGPGridRow*>& lstDelItemsPtr)
{
	ASSERT_VALID (this);

	if (m_bVirtualMode)
	{
		// Cannot remove row in the virtual mode
		ASSERT (FALSE);
		return 0;
	}

	//--------------
	// Remove items:
	//--------------
	int nCount = 0;
	POSITION pos;
	for (pos = lstDelItemsPos.GetHeadPosition (); pos != NULL;)
	{
		POSITION posDel = lstDelItemsPos.GetNext (pos);
		if (DoRemoveRow (posDel, FALSE))
		{
			nCount++;
		}
	}
	for (pos = lstDelItemsPtr.GetHeadPosition (); pos != NULL;)
	{
		CBCGPGridRow* pItemDel = lstDelItemsPtr.GetNext (pos);
		if (DoRemoveRow (pItemDel, FALSE))
		{
			nCount++;
		}
	}

	return nCount;
}
//*****************************************************************************************
BOOL CBCGPGridCtrl::DoRemoveRow (CBCGPGridRow* pItemDel, BOOL bRedraw)
{
	if (pItemDel == NULL)
	{
		return FALSE;
	}

	ASSERT_VALID (pItemDel);

	BOOL bIsAutoGroup = pItemDel->IsGroup () && (pItemDel->m_dwFlags & BCGP_GRID_ITEM_AUTOGROUP) != 0;
	if (bIsAutoGroup)
	{
		return FALSE;
	}

	POSITION posDel = m_lstItems.Find (pItemDel);
	return DoRemoveRow (posDel, bRedraw);
}
//*****************************************************************************************
BOOL CBCGPGridCtrl::DoRemoveRow (POSITION posDel, BOOL bRedraw)
{
	ASSERT_VALID (this);

	if (posDel != NULL)
	{
		CBCGPGridRow* pItemDel = m_lstItems.GetAt (posDel);
		ASSERT_VALID (pItemDel);

		if (pItemDel->IsSelected ())
		{
			// Cleanup selection:
			m_idActive.SetNull ();
			m_idLastSel.SetNull ();
			SetCurSel (m_idActive, SM_NONE, FALSE);
		}

		// Remove parent-child links:
		CBCGPGridRow* pParent = pItemDel->GetParent ();
		if (pParent != NULL)
		{
			for (POSITION pos = pParent->m_lstSubItems.GetHeadPosition (); pos != NULL; )
			{
				POSITION posSave = pos;

				CBCGPGridRow* pChildRow = pParent->m_lstSubItems.GetNext (pos);
				ASSERT_VALID (pChildRow);

				if (pChildRow == pItemDel)
				{
					pParent->m_lstSubItems.RemoveAt (posSave);
					break;
				}
			}
		}

		for (POSITION pos = pItemDel->m_lstSubItems.GetHeadPosition (); pos != NULL; )
		{
			CBCGPGridRow* pChildRow = pItemDel->m_lstSubItems.GetNext (pos);
			ASSERT_VALID (pChildRow);

			pChildRow->SetParent(NULL);
		}


		m_lstItems.RemoveAt (posDel);

		delete pItemDel;

		SetRebuildTerminalItems ();

		if (bRedraw && !m_bPostAdjustLayout && GetSafeHwnd() != NULL)
		{
			PostMessage(BCGM_GRID_ADJUST_LAYOUT);
			m_bPostAdjustLayout = TRUE;
		}

		return TRUE;
	}

	return FALSE;
}
//*****************************************************************************************
void CBCGPGridCtrl::RemoveAll ()
{
	ASSERT_VALID (this);

	m_idActive.SetNull ();
	m_idLastSel.SetNull ();
	SetCurSel (m_idActive, SM_NONE, FALSE);

	CleanUpAutoGroups ();

	while (!m_lstItems.IsEmpty ())
	{
		delete m_lstItems.RemoveTail ();
	}

	m_ToolTip.Hide ();

	m_lstTerminalItems.RemoveAll ();

	m_pSetSelItem = NULL;
	m_pSelRow = NULL;
	m_pSelItem = NULL;
	m_pLastSelRow = NULL;
	m_pLastSelItem = NULL;

	SetRebuildTerminalItems ();
	m_CachedItems.CleanUpCache ();
}
//******************************************************************************************
CBCGPGridRow* CBCGPGridCtrl::GetRow (int nIndex) const
{
	ASSERT_VALID (this);

	if (m_bVirtualMode)
	{
		return ((CBCGPGridCtrl*) this)->GetVirtualRow (nIndex);
	}

	if (IsSortingMode () || IsGrouping ())
	{
		if (nIndex >= 0 && nIndex < m_lstTerminalItems.GetCount ())
		{
			POSITION pos = m_lstTerminalItems.FindIndex (nIndex);
			if (pos != NULL)
			{
				CBCGPGridRow* pItem = m_lstTerminalItems.GetAt (pos);
				ASSERT_VALID (pItem);

				if (pItem->m_nIdRow == nIndex)
				{
					return pItem;
				}
			}

			ASSERT (FALSE);
		}

		if (IsGrouping () && nIndex >= m_lstTerminalItems.GetCount ())
		{
			int nAutoGroupIdx = nIndex - (int) m_lstTerminalItems.GetCount ();
			POSITION pos = m_lstAutoGroups.FindIndex (nAutoGroupIdx);
			if (pos != NULL)
			{
				return m_lstAutoGroups.GetAt (pos);
			}

			ASSERT (FALSE);
		}


		return NULL;
	}

	BOOL bSearchSubItems = TRUE;

	for (POSITION pos = m_lstItems.GetHeadPosition (); pos != NULL; )
	{
		CBCGPGridRow* pItem = m_lstItems.GetNext (pos);
		ASSERT_VALID (pItem);

		if (pItem->m_nIdRow == nIndex)
		{
			return pItem;
		}

		if (bSearchSubItems) 
		{
			pItem = pItem->FindSubItemById (nIndex);

			if  (pItem != NULL)
			{
				ASSERT_VALID (pItem);
				return pItem;
			}
		}
	}

	return NULL;
}
//******************************************************************************************
int CBCGPGridCtrl::GetRowCount (BOOL bIncludeAutoGroups) const
{
	int nRowCount = (int) m_lstItems.GetCount ();
	if (bIncludeAutoGroups)
	{
		nRowCount += (int) m_lstAutoGroups.GetCount ();
	}
	return nRowCount;
}
//******************************************************************************************
CBCGPGridRow* CBCGPGridCtrl::FindRowByData (DWORD_PTR dwData, BOOL bSearchSubItems/* = TRUE*/) const
{
	ASSERT_VALID (this);

	for (POSITION pos = m_lstItems.GetHeadPosition (); pos != NULL; )
	{
		CBCGPGridRow* pItem = m_lstItems.GetNext (pos);
		ASSERT_VALID (pItem);

		if (!bSearchSubItems && pItem->m_pParent != NULL)
		{
			continue; // scan only top-level items
		}

		if (pItem->m_dwData == dwData)
		{
			return pItem;
		}

		if (bSearchSubItems) 
		{
			pItem = pItem->FindSubItemByData (dwData);

			if  (pItem != NULL)
			{
				ASSERT_VALID (pItem);
				return pItem;
			}
		}
	}

	return NULL;
}
//*******************************************************************************************
CBCGPGridRow* CBCGPGridCtrl::HitTest (
							CPoint pt, CBCGPGridItemID &id,
							CBCGPGridItem*& pGridItem,
							CBCGPGridRow::ClickArea* pnArea,
							BOOL bPropsOnly)
{
	ASSERT_VALID (this);

	if (!m_rectList.PtInRect (pt) && !bPropsOnly)
	{
		id.SetNull ();
		pGridItem = NULL;
		return NULL;
	}

	if (m_bVirtualMode)
	{
		//-----------------
		// Get virtual row:
		//-----------------
		id = HitTestVirtual (pt, pnArea);

		CBCGPGridRow* pRow = GetVirtualRow (id.m_nRow);

		if (pRow != NULL)
		{
			ASSERT_VALID (pRow);
			pGridItem = pRow->GetItem (id.m_nColumn);
		}

		return pRow;
	}

	const CList<CBCGPGridRow*, CBCGPGridRow*>& lst = 
		(!IsSortingMode () && !IsGrouping ()) ? m_lstItems : m_lstTerminalItems;

	int nFirstItem = m_bVirtualMode ?
		m_nVertScrollOffset / m_nRowHeight : m_nFirstVisibleItem;

	POSITION pos = nFirstItem == -1 ?
		lst.GetHeadPosition () : lst.FindIndex (nFirstItem);

	int nExtraLoopCount = 1;

	while (pos != NULL)
	{
		CBCGPGridRow* pItem = lst.GetNext (pos);
		if (pItem == NULL)
		{
			continue;
		}

		ASSERT_VALID (pItem);

		int iColumn = -1;
		CBCGPGridRow* pHit = pItem->HitTest (pt, iColumn, pGridItem, pnArea);
		if (pHit != NULL)
		{
			id = CBCGPGridItemID (pHit->m_nIdRow, iColumn);

			BOOL bIsAutoGroup = pItem->IsGroup () && (pItem->m_dwFlags & BCGP_GRID_ITEM_AUTOGROUP) != 0;
			if (bIsAutoGroup)
			{
				id.m_nColumn = -1;
			}

			return pHit;
		}

		CRect rectRow = pItem->GetRect ();
		if (rectRow.top > m_rectList.bottom)
		{
			if (--nExtraLoopCount < 0)
			{
				break;
			}	
		}
    }

	id.SetNull ();
	pGridItem = NULL;
	return NULL;
}
//*******************************************************************************************
void CBCGPGridCtrl::SetCurSel (CBCGPGridRow* pItem, BOOL bRedraw)
{
	ASSERT_VALID (this);

	CBCGPGridItemID id;

	if (pItem == NULL)
	{
		SetCurSel (id, SM_NONE, bRedraw);
	}
	else
	{
		ASSERT_VALID (pItem);

		id.m_nRow =	pItem->GetRowId ();
		id.m_nColumn = -1;

		SetCurSel (id, SM_SINGE_SEL_GROUP | SM_SINGLE_ITEM | SM_ROW, bRedraw);
	}
}
//******************************************************************************************
CBCGPGridRow* CBCGPGridCtrl::GetCurSel () const
{
	if (m_bVirtualMode)
	{
		return ((CBCGPGridCtrl*) this)->GetVirtualRow (m_idActive.m_nRow);
	}

	if (m_pSelItem != NULL && m_pSelItem->GetMergedCells () != NULL)
	{
		CBCGPGridItem* pSelItem = m_pSelItem->GetMergedMainItem ();
		return (pSelItem != NULL) ? pSelItem->GetParentRow () : NULL;
	}

	if (m_pSelRow == NULL)
	{
		if (m_pSelItem != NULL)
		{
			return m_pSelItem->m_pGridRow;
		}

		return NULL;
	}

	return m_pSelRow;
}
//******************************************************************************************
CBCGPGridItem* CBCGPGridCtrl::GetCurSelItem (CBCGPGridRow* pCurRow) const
{
	if (m_bVirtualMode)
	{
		if (pCurRow == NULL)
		{
			pCurRow = GetCurSel ();
		}
		return (pCurRow != NULL) ? pCurRow->GetItem (m_idActive.m_nColumn) : NULL;
	}

	if (m_pSelItem != NULL && m_pSelItem->GetMergedCells () != NULL)
	{
		return m_pSelItem->GetMergedMainItem ();
	}

	return m_pSelItem;
}
//******************************************************************************************
BOOL CBCGPGridCtrl::SelectColumn (int nColumn, BOOL bRedraw)
{
	CBCGPGridItemID idItem;
	idItem.m_nRow = -1;
	idItem.m_nColumn = nColumn;

	SetCurSel (idItem, SM_SINGE_SEL_GROUP | SM_SINGLE_ITEM, bRedraw);

	return TRUE;
}
//******************************************************************************************
BOOL CBCGPGridCtrl::SelectRow (int nRow, BOOL bRedraw)
{
	ASSERT_VALID (this);

	CBCGPGridItemID idItem;
	idItem.m_nRow = nRow;
	idItem.m_nColumn = -1;

	SetCurSel (idItem, SM_SINGE_SEL_GROUP | SM_SINGLE_ITEM, bRedraw);

	return TRUE;
}
//******************************************************************************************
BOOL CBCGPGridCtrl::SelectAll (BOOL bRedraw)
{
	ASSERT_VALID (this);

	CBCGPGridItemID id;
	SetCurSel (id, SM_ALL, FALSE);

	if (bRedraw)
	{
		RedrawWindow ();
	}

	return TRUE;
}
//******************************************************************************************
BOOL CBCGPGridCtrl::IsRowSelected (int nRow, BOOL bAllItemsSelected) const
{
	int nFirstColumn = -1;
	int nLastColumn = -1;
	if (bAllItemsSelected)
	{
		nFirstColumn = GetColumnsInfo ().GetFirstVisibleColumn ();
		nLastColumn = GetColumnsInfo ().GetLastVisibleColumn ();
	}

	for (POSITION pos = m_lstSel.GetHeadPosition (); pos != NULL;)
	{
		CBCGPGridRange* pSelRange = m_lstSel.GetNext (pos);
		ASSERT (pSelRange != NULL);

		if (pSelRange->m_nTop <= nRow && pSelRange->m_nBottom >= nRow)
		{
			if (!bAllItemsSelected)
			{
				return TRUE;
			}

			if (pSelRange->m_nLeft == nFirstColumn && pSelRange->m_nRight == nLastColumn)
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}
//******************************************************************************************
BOOL CBCGPGridCtrl::IsColumnSelected (int nColumn, BOOL bAllItemsSelected) const
{
	if (nColumn < 0 || nColumn >= GetColumnsInfo ().GetColumnCount ())
	{
		// column not found
		return FALSE;
	}

	int nFirstRow = -1;
	int nLastRow = -1;
	if (bAllItemsSelected)
	{
		nFirstRow = 0;
		nLastRow = GetTotalItems () - 1;
	}

	for (POSITION pos = m_lstSel.GetHeadPosition (); pos != NULL;)
	{
		CBCGPGridRange* pSelRange = m_lstSel.GetNext (pos);
		ASSERT (pSelRange != NULL);

		int nPos = GetColumnsInfo ().IndexToOrder (nColumn);
		int nPosLeft = GetColumnsInfo ().IndexToOrder (pSelRange->m_nLeft);
		int nPosRight = GetColumnsInfo ().IndexToOrder (pSelRange->m_nRight);

		if (nPos != -1 && nPosLeft != -1 && nPosRight != -1 &&
			min (nPosLeft, nPosRight) <= nPos && max (nPosLeft, nPosRight) >= nPos)
		{
			if (!bAllItemsSelected)
			{
				return TRUE;
			}

			if (pSelRange->m_nTop == nFirstRow && pSelRange->m_nBottom == nLastRow)
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}
//******************************************************************************************
BOOL CBCGPGridCtrl::IsAllSelected () const
{
	const int nFirstColumn = GetColumnsInfo ().GetFirstVisibleColumn ();
	const int nFirstRow = 0;
	const int nLastColumn = GetColumnsInfo ().GetLastVisibleColumn ();
	const int nLastRow = GetTotalItems () - 1;

	for (POSITION pos = m_lstSel.GetHeadPosition (); pos != NULL;)
	{
		CBCGPGridRange* pSelRange = m_lstSel.GetNext (pos);
		ASSERT (pSelRange != NULL);

		// check if whole grid is selected
		if (pSelRange->m_nTop == nFirstRow && pSelRange->m_nBottom == nLastRow &&
			pSelRange->m_nLeft == nFirstColumn && pSelRange->m_nRight == nLastColumn)
		{
			return TRUE;
		}
	}

	return FALSE;
}
//******************************************************************************************
BOOL CBCGPGridCtrl::IsItemSelected (const CBCGPGridItemID& idItem) const
{
	for (POSITION pos = m_lstSel.GetHeadPosition (); pos != NULL; )
	{
		CBCGPGridRange* pSelRange = (CBCGPGridRange*) m_lstSel.GetNext (pos);
		ASSERT (pSelRange != NULL);
		
		if (IsItemInRange (*pSelRange, idItem))
		{
			return TRUE;
		}
	}

	return FALSE;
}
//******************************************************************************************
// Retrieves list of visible selected items
void CBCGPGridCtrl::GetSelectedItems (CList <CBCGPGridItem*, CBCGPGridItem*> &lstSelected)
{
	if (m_bVirtualMode)
	{
		// Can't work in virtual mode.
		// Storing a pointer is not safe.
		// Use m_lstSel member to get list of selected ranges
		ASSERT (FALSE);
		return;
	}

	const CList<CBCGPGridRow*, CBCGPGridRow*>& lst =
		(!IsSortingMode () && !IsGrouping ()) ? m_lstItems : m_lstTerminalItems;

	for (POSITION posRange = m_lstSel.GetHeadPosition (); posRange != NULL; )
	{
		CBCGPGridRange* pRange = (CBCGPGridRange*) m_lstSel.GetNext (posRange);
		ASSERT (pRange != NULL);

		POSITION pos = lst.FindIndex (pRange->m_nTop);
		for (int i = pRange->m_nTop; pos != NULL && i <= pRange->m_nBottom; i++)
		{
			CBCGPGridRow* pRow = lst.GetNext (pos);
			ASSERT_VALID (pRow);

			if (pRow->IsItemVisible ())
			{
				BOOL bFirstItem = TRUE; // first visible grid item in the specified range
				BOOL bInRange = FALSE;	// is inside the specified range?

				int nPos = GetColumnsInfo ().Begin ();
				while (nPos != GetColumnsInfo ().End ())
				{
					int iColumn = GetColumnsInfo ().Next (nPos);
					if (iColumn == -1)
					{
						break; // no more visible columns
					}

					BOOL bIsRangeBound = (iColumn == pRange->m_nLeft || iColumn == pRange->m_nRight);
					if (bIsRangeBound || bInRange)
					{
						CBCGPGridItem* pItem = pRow->GetItem (iColumn);
						if (pItem != NULL)
						{
							if (!m_bWholeRowSel || bFirstItem)
							{
								lstSelected.AddTail (pItem);
								bFirstItem = FALSE;
							}
						}
					}

					if (bIsRangeBound)
					{
						if (bInRange || pRange->m_nLeft == pRange->m_nRight)
						{
							break;	// last visible column in range
						}

						bInRange = TRUE;
					}

				}
			}
		}
	}
}
//******************************************************************************************
int CBCGPGridCtrl::GetSelectionCount () const
{
	ASSERT_VALID (this);
	return (int) m_lstSel.GetCount ();
}
//******************************************************************************************
BOOL CBCGPGridCtrl::GetSelection (int nIndex, CBCGPGridRange& range) const
{
	POSITION pos = m_lstSel.FindIndex (nIndex);
	if (pos != NULL)
	{
		CBCGPGridRange* pRange = m_lstSel.GetAt (pos);
		if (pRange != NULL)
		{
			range = *pRange;
			return TRUE;
		}
	}

	return FALSE;
}
//******************************************************************************************
// Implementation of ClearRange:
class CallbackClearItemParams
{
public:
	enum Result
	{
		NoResult = -1,
		NotAllowClear = 0,
		AllowClear = 1,
		Ok = 1
	};

	CallbackClearItemParams (CBCGPGridCtrl* p, BOOL bTestOnly, BOOL bQuery, BOOL bReplace)
		: pGrid (p), bTest (bTestOnly), bReplaceOperation (bReplace), bQueryNonEmptyItems (bQuery), nSavedQueryResult (NoResult), nResult (NoResult)
	{
	}

	CBCGPGridCtrl* pGrid;
	BOOL	bTest;
	BOOL	bReplaceOperation;		// TRUE - replace, FALSE - clear
	BOOL	bQueryNonEmptyItems;
	Result	nSavedQueryResult;		// NoResult, NotAllowClear, AllowClear
	Result	nResult;				// NoResult, NotAllowClear, Ok
};
void CBCGPGridCtrl::pfnCallbackClearItem (CBCGPGridItem* pItem, const CBCGPGridRange&, LPARAM lParam)
{
	ASSERT_VALID (pItem);

	CallbackClearItemParams* params = (CallbackClearItemParams*)lParam;
	if (params != NULL)
	{
		ASSERT_VALID (params->pGrid);

		if (!pItem->IsAllowEdit () || !pItem->IsEnabled ())
		{
			params->nResult = CallbackClearItemParams::NotAllowClear;
			return;
		}

		if (!pItem->IsEmpty ())
		{
			// Ask grid first time: Can clear non-empty item?
			if (params->bQueryNonEmptyItems && 
				params->nSavedQueryResult == CallbackClearItemParams::NoResult)
			{
				BOOL bQueryResult = params->bReplaceOperation ?
					params->pGrid->OnQueryReplaceNonEmptyItem (pItem) :
					params->pGrid->OnQueryClearNonEmptyItem (pItem);
				params->nSavedQueryResult = bQueryResult ?
					CallbackClearItemParams::AllowClear :
					CallbackClearItemParams::NotAllowClear;
			}
		}

		if (params->nSavedQueryResult == CallbackClearItemParams::NotAllowClear ||
			params->nResult == CallbackClearItemParams::NotAllowClear)
		{
			params->nResult = CallbackClearItemParams::NotAllowClear;
			return;
		}

		if (!params->bTest)
		{
			params->pGrid->InvalidateRect (pItem->GetRect ());

			if (!pItem->ClearContent (FALSE))
			{
				params->nResult = CallbackClearItemParams::NotAllowClear;
				return;
			}
		}
		
		params->nResult = CallbackClearItemParams::Ok;
	}
}
//******************************************************************************************
BOOL CBCGPGridCtrl::CanClearRange (const CBCGPGridRange& range, 
								   BOOL bQueryNonEmptyItems, BOOL* pbPrevQuery, BOOL bOnReplace)
{
	ASSERT_VALID (this);

	if (IsReadOnly ())
	{
		return FALSE;
	}

	CList <CBCGPGridRange, CBCGPGridRange&> lstMergedRanges;
	if (GetMergedItemsInRange (range, lstMergedRanges) > 0)
	{
		return FALSE;
	}

	BOOL bForceNoQuery = (pbPrevQuery != NULL) && (*pbPrevQuery);

	CallbackClearItemParams params (this, TRUE, bQueryNonEmptyItems && !bForceNoQuery, bOnReplace);
	IterateInRange (range, NULL, 0, NULL, 0, &CBCGPGridCtrl::pfnCallbackClearItem, (LPARAM )&params);
	
	if (pbPrevQuery != NULL)
	{
		*pbPrevQuery = (*pbPrevQuery) || (params.nSavedQueryResult == CallbackClearItemParams::AllowClear);
	}

	return (params.nResult == CallbackClearItemParams::Ok);
}
//******************************************************************************************
BOOL CBCGPGridCtrl::ClearRange (const CBCGPGridRange& range, BOOL bRedraw, 
								BOOL bQueryNonEmptyItems, BOOL* pbPrevQuery)
{
	ASSERT_VALID (this);

	if (IsReadOnly ())
	{
		return FALSE;
	}

	CList <CBCGPGridRange, CBCGPGridRange&> lstMergedRanges;
	if (GetMergedItemsInRange (range, lstMergedRanges) > 0)
	{
		return FALSE;
	}

	BOOL bForceNoQuery = (pbPrevQuery != NULL) && (*pbPrevQuery);

	CallbackClearItemParams params (this, FALSE, bQueryNonEmptyItems && !bForceNoQuery, FALSE);
	IterateInRange (range, NULL, 0, NULL, 0, &CBCGPGridCtrl::pfnCallbackClearItem, (LPARAM )&params);

	if (bRedraw)
	{
		UpdateWindow ();
	}

	if (pbPrevQuery != NULL)
	{
		*pbPrevQuery = (*pbPrevQuery) || (params.nSavedQueryResult == CallbackClearItemParams::AllowClear);
	}

	return (params.nResult == CallbackClearItemParams::Ok);
}
//******************************************************************************************
BOOL CBCGPGridCtrl::NormalizeSelectionList ()
{
	if (!IsWholeRowSel () || m_pSerializeManager == NULL ||
		m_pSerializeManager->m_ClipboardFormatType != CBCGPGridSerializeManager::CF_Rows)
	{
		return FALSE;
	}

	CList <CBCGPGridRange, CBCGPGridRange&> lstNormalizedSel;

	for (int i = GetSelectionCount () - 1; i >= 0; i--)
	{
		CBCGPGridRange rangeSel;
		if (!GetSelection (i, rangeSel) || rangeSel.m_nTop < 0 || rangeSel.m_nBottom < rangeSel.m_nTop)
		{
			return FALSE;
		}

		// coverage
		CList <CBCGPGridRange, CBCGPGridRange&> lstCoverage;
		lstCoverage.AddTail (rangeSel);

		// verify against others
		for (POSITION posOthers = lstNormalizedSel.GetHeadPosition (); posOthers != NULL && !lstCoverage.IsEmpty (); )
		{
			const CBCGPGridRange rangeOther = lstNormalizedSel.GetNext (posOthers);
			
			for (POSITION posCoverage = lstCoverage.GetHeadPosition (); posCoverage != NULL; )
			{
				POSITION posSave = posCoverage;
				CBCGPGridRange& range = lstCoverage.GetNext (posCoverage);
				
				// no intersection - keep range
				if (range.m_nTop > rangeOther.m_nBottom || range.m_nBottom < rangeOther.m_nTop)
				{
					continue;
				}
				
				// keep left side
				if (range.m_nBottom >= rangeOther.m_nTop && range.m_nTop < rangeOther.m_nTop)
				{
					CBCGPGridRange rangeLeftSide = range;
					rangeLeftSide.m_nBottom = rangeOther.m_nTop - 1;
					
					lstCoverage.InsertBefore (posSave, rangeLeftSide);
				}
				
				// keep right side
				if (range.m_nTop <= rangeOther.m_nBottom && range.m_nBottom > rangeOther.m_nBottom)
				{
					CBCGPGridRange rangeRightSide = range;
					rangeRightSide.m_nTop = rangeOther.m_nBottom + 1;
					
					lstCoverage.InsertAfter (posSave, rangeRightSide);
				}

				// remove duplicated range
				lstCoverage.RemoveAt (posSave);
				posCoverage = lstCoverage.GetHeadPosition (); // start loop again
			}			
		}
		
		// add
		lstNormalizedSel.AddHead (&lstCoverage);
		lstCoverage.RemoveAll ();
	}

	while (!m_lstSel.IsEmpty ())
	{
		delete m_lstSel.RemoveTail ();
	}

	while (!lstNormalizedSel.IsEmpty ())
	{
		CBCGPGridRange* pRange = new CBCGPGridRange;
		pRange->Set (lstNormalizedSel.RemoveHead ());

		m_lstSel.AddTail (pRange);
	}

	return TRUE;
}
//******************************************************************************************
int CBCGPGridCtrl::RemoveSelectedRows (BOOL bRedraw)
{
	if (!NormalizeSelectionList ())
	{
		return 0;
	}

	int nCount = 0;
	
	//--------------------------------
	// Collect list of items to delete
	//--------------------------------
	CList <POSITION, POSITION> lstDelItemsPos;
	CList <CBCGPGridRow*, CBCGPGridRow*> lstDelItemsPtr;

	for (int i = 0; i < GetSelectionCount (); i++)
	{
		CBCGPGridRange range;
		if (GetSelection (i, range))
		{
			const int nRowOffset = range.m_nTop;
			const int nRowCount = range.m_nBottom - range.m_nTop + 1;
			
			for (int nRow = 0; nRow < nRowCount; nRow++)
			{
				int nRowIndex = nRow + nRowOffset;
				ASSERT(nRowIndex >= 0);
				
				PrepareRemoveRow (nRowIndex, lstDelItemsPos, lstDelItemsPtr);
			}
		}
	}

	//--------------
	// Remove items:
	//--------------
	nCount += DoRemoveRows (lstDelItemsPos, lstDelItemsPtr);

	SetRebuildTerminalItems ();
	
	if (bRedraw && GetSafeHwnd() != NULL)
	{
		AdjustLayout ();
	}

	return nCount;
}
//******************************************************************************************
void CBCGPGridCtrl::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CBCGPWnd::OnLButtonDown(nFlags, point);

	OnFilterBarSetFocus (-1);
	SetFocus ();

	//--------------------
	// Group-by-box click:
	//--------------------
	CRect rectColumn;
	int nGroupHit = HitTestGroupByBox (point, rectColumn);

	if (nGroupHit >= 0)
	{
		m_ptStartDrag = point;
		StartDragColumn (nGroupHit, rectColumn, TRUE, FALSE);
		return;
	}

	// -------------
	// Track header:
	// -------------
	CPoint ptHeader = point;
	if (OnTrackHeader () && m_rectList.PtInRect (point))
	{
		ptHeader.y = m_rectHeader.top;
	}
	int nColumnHit = GetColumnsInfo ().HitTestColumn (ptHeader, TRUE, STRETCH_DELTA);
	if (nColumnHit != -1)
	{
		int nLButton = GetSystemMetrics (SM_SWAPBUTTON) ? VK_RBUTTON : VK_LBUTTON;
		if ((GetAsyncKeyState (nLButton) & 0x8000) == 0)
		{
			return;
		}

		if (!GetColumnsInfo ().CanChangeWidth (nColumnHit))
		{
			// column can't be resized
			return;
		}

		CRect rectHeaderColumn;
		GetColumnsInfo ().GetColumnRect (nColumnHit, rectHeaderColumn);

		SetCapture ();
		m_nTrackColumn = nColumnHit;
		TrackHeader (rectHeaderColumn.right);
		m_bTracking = TRUE;
		return;
	}

	if (IsDragSelectionBorderEnabled () && HitTestSelectionBorder (point))
	{
		if (StartDragItems (point))
		{
			return;
		}
	}

	//--------------
	// Header click:
	//--------------
	CBCGPGridColumnsInfo::ClickArea clickAreaHeader;
	nColumnHit = GetColumnsInfo ().HitTestColumn (point, FALSE, STRETCH_DELTA, &clickAreaHeader);

	if (nColumnHit >= 0)
	{
		DoColumnHeaderClick (nColumnHit, point, clickAreaHeader);
		return;
	}

	//-------------------------
	// "Select all" area click:
	//-------------------------
	if (m_rectSelectAllArea.PtInRect (point))
	{
		OnSelectAllClick ();
		return;
	}

	//------------------
	// Row header click:
	//------------------
	CRect rectRowHeader;
	CBCGPGridRow* pHitHeaderRow = HitTestRowHeader (point, rectRowHeader);
	if (pHitHeaderRow != NULL)
	{
		if (!IsSingleSel() || DYNAMIC_DOWNCAST(CBCGPGridCaptionRow, pHitHeaderRow) == NULL)
		{
			DoRowHeaderClick (pHitHeaderRow, point, rectRowHeader);
		}
		return;
	}

	// ---------------------------
	// Set selection (first click):
	// ---------------------------
	CBCGPGridRow::ClickArea clickArea;
	CBCGPGridItemID id;
	CBCGPGridItem* pHitItem = NULL;
	CBCGPGridRow* pHitRow = HitTest (point, id, pHitItem, &clickArea);

	if (IsSingleSel() && DYNAMIC_DOWNCAST(CBCGPGridCaptionRow, pHitRow) != NULL)
	{
		return;
	}

	BOOL bSelChanged = id != m_idLastSel;
	BOOL bIsButtonClick = pHitItem != NULL && pHitItem->m_rectButton.PtInRect (point);

	const BOOL bShift = (::GetAsyncKeyState (VK_SHIFT) & 0x8000) != 0 && !m_bIgnoreShiftBtn;
	const BOOL bCtrl = (::GetAsyncKeyState (VK_CONTROL) & 0x8000) != 0 && !m_bIgnoreCtrlBtn;

	if (IsDragSelectionEnabled () && IsItemSelected (id) && !(bCtrl && m_bInvertSelOnCtrl) && !bIsButtonClick && clickArea != CBCGPGridRow::ClickExpandBox)
	{
		if (StartDragItems (point))
		{
			return;
		}
	}

	DWORD dwSelMode = SM_NONE;
	if (!id.IsNull ())
	{
		dwSelMode = SM_FIRST_CLICK |
			(m_bSingleSel ? SM_SINGE_SEL_GROUP :
			(bCtrl ? SM_ADD_SEL_GROUP :
			(bShift ? SM_CONTINUE_SEL_GROUP : SM_SINGE_SEL_GROUP)));

		if (bCtrl && m_bInvertSelOnCtrl)
		{
			dwSelMode |= SM_INVERT_SEL;
		}

		if (pHitRow != NULL && id.IsRow ())
		{
			dwSelMode |= SM_ROW;
		}
		if (id.IsColumn ())
		{
			dwSelMode |= SM_COLUMN;
		}
	}
	
	m_bNoUpdateWindow = TRUE; // prevent flickering
	m_pSetSelItem = m_bVirtualMode ? NULL : pHitItem;

	SetCurSel (id, dwSelMode);

	m_pSetSelItem = NULL;
	m_bNoUpdateWindow = FALSE;

	if (id.IsNull () || pHitRow == NULL)
	{
		return;
	}

	ASSERT_VALID (pHitRow);
	EnsureVisible (pHitRow);

	CBCGPGridRow* pCurSel = GetCurSel ();
	CBCGPGridItem* pItem = GetCurSelItem (pCurSel);
	if (id != m_idActive || pCurSel == NULL || (pItem == NULL && clickArea == CBCGPGridRow::ClickValue))
	{
		// The hitten item is not active - do not translate a click to the grid item.
		// Translate a click for single item only.
		return;
	}

	// ------------------------------
	// Translate a click to the item:
	// ------------------------------
	ASSERT_VALID (pCurSel); // pCurSel - hitten row

	switch (clickArea)
	{
	case CBCGPGridRow::ClickExpandBox:
		pCurSel->Expand (!pCurSel->IsExpanded ());
		break;

	case CBCGPGridRow::ClickName:
		pCurSel->OnClickName (point);

		// Start selecting range of items:
		StartSelectItems ();
		break;

	case CBCGPGridRow::ClickValue:
		ASSERT_VALID (pItem);	// pItem - hitten active item
		if (pCurSel->m_bEnabled && pItem->IsEnabled ())
		{
			if (bIsButtonClick || m_bSingleSel)
			{
				DoClickValue (pItem, WM_LBUTTONDOWN, point, bSelChanged, bIsButtonClick);
			}
			else
			{
				// Defer the item click:
				SetTimer (GRID_CLICKVALUE_TIMER_ID, GRID_CLICKVALUE_TIMER_INTERVAL, NULL);
				m_bClickTimer = TRUE;
				m_ptClickOnce = point;
				m_bIsFirstClick = bSelChanged;
				m_bIsButtonClick = bIsButtonClick;
				m_bHeaderRowSelecting = FALSE;
				m_bHeaderColSelecting = FALSE;

				StartSelectItems ();
				return;
			}
		}
		break;

	default:
		break;
	}
}
//************************************************************************************
void CBCGPGridCtrl::OnRButtonDown(UINT nFlags, CPoint point) 
{
	CBCGPWnd::OnRButtonDown(nFlags, point);

	if (m_nDraggedColumn >= 0 || m_bTracking || m_bSelecting)
	{
		return;
	}
	
	SetFocus ();

	// -------------
	// Track header:
	// -------------
	if (OnTrackHeader () && point.y >= m_rectList.top)
	{
		CPoint ptHeader = point;
		ptHeader.y = m_rectHeader.top;

		if (GetColumnsInfo ().HitTestColumn (ptHeader, TRUE, STRETCH_DELTA) != -1)
		{
			return;
		}
	}

	CBCGPGridRow::ClickArea clickArea;
	CBCGPGridItemID id;
	CBCGPGridItem* pHitItem = NULL;
	CBCGPGridRow* pHitRow = HitTest (point, id, pHitItem, &clickArea);

	if (DYNAMIC_DOWNCAST(CBCGPGridCaptionRow, pHitRow) != NULL)
	{
		return;
	}

	BOOL bSaveSelection = pHitRow != NULL &&
		(pHitItem != NULL ? pHitItem->IsSelected () : pHitRow->IsSelected () );

	BOOL bSelChanged = id != m_idLastSel;

	const BOOL bShift = (::GetAsyncKeyState (VK_SHIFT) & 0x8000) != 0 && !m_bIgnoreShiftBtn;
	const BOOL bCtrl = (::GetAsyncKeyState (VK_CONTROL) & 0x8000) != 0 && !m_bIgnoreCtrlBtn;

	DWORD dwSelMode = SM_NONE;
	if (!id.IsNull ())
	{
		dwSelMode = SM_FIRST_CLICK |
			(m_bSingleSel ? SM_SINGE_SEL_GROUP :
			(bCtrl ? SM_ADD_SEL_GROUP :
			(bShift ? SM_CONTINUE_SEL_GROUP : SM_SINGE_SEL_GROUP)));

		if (bCtrl && m_bInvertSelOnCtrl)
		{
			dwSelMode |= SM_INVERT_SEL;
		}
	}

	if (!bSaveSelection)
	{
		m_pSetSelItem = m_bVirtualMode ? NULL : pHitItem;

		SetCurSel (id, dwSelMode);

		m_pSetSelItem = NULL;
	}

	if (pHitRow != NULL)
	{
		ASSERT_VALID (pHitRow);
		EnsureVisible (pHitRow);
	}

	CBCGPGridRow* pCurSel = GetCurSel ();
	CBCGPGridItem* pItem = GetCurSelItem (pCurSel);
	if (id != m_idActive || pCurSel == NULL || pItem == NULL)
	{
		// The hitten item is not active - do not translate a click to the grid item.
		// Translate a click for single item only.
		return;
	}

	ASSERT_VALID (pCurSel); // pCurSel - hitten row
	ASSERT_VALID (pItem);	// pItem - hitten active item

	switch (clickArea)
	{
	case CBCGPGridRow::ClickExpandBox:
		break;

	case CBCGPGridRow::ClickName:
		pCurSel->OnRClickName (point);
		break;

	case CBCGPGridRow::ClickValue:
		pCurSel->OnRClickValue (point, bSelChanged);

		if (pCurSel->m_bEnabled && !bSelChanged && !bSaveSelection)
		{
			SetBeginEditReason (BeginEdit_MouseClick, bShift, bCtrl);
			if (EditItem (pCurSel, &point) && pCurSel->m_bInPlaceEdit)
			{
				if (pItem->m_rectButton.PtInRect (point))
				{
					return;
				}

				pItem->OnClickValue (WM_RBUTTONDOWN, point);
			}
		}
		break;

	default:
		break;
	}
}
//******************************************************************************************
void CBCGPGridCtrl::OnRButtonUp(UINT nFlags, CPoint point) 
{
	CBCGPWnd::OnRButtonUp(nFlags, point);

	if (m_nDraggedColumn >= 0)
	{
		// Cancel dragging a column:
		CPoint pt(-1, -1);
		StopDragColumn (pt, FALSE);
	}
	if (m_bTracking)
	{
		TrackHeader (-1);
		m_bTracking = FALSE;

		if (::GetCapture () == GetSafeHwnd ())
		{
			ReleaseCapture ();
		}
	}
	if (m_bSelecting)
	{
		KillTimer (GRID_CLICKVALUE_TIMER_ID);
		m_bClickTimer = FALSE;
		m_ptClickOnce = CPoint (0, 0);

		StopSelectItems ();
	}
}
//******************************************************************************************
BOOL CBCGPGridCtrl::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	return CBCGPWnd::Create (
		globalData.RegisterWindowClass (_T("BCGPGridCtrl")),
		_T(""), dwStyle, rect, pParentWnd, nID, NULL);
}
//******************************************************************************************
BOOL CBCGPGridCtrl::EditItem (CBCGPGridRow* pItem, LPPOINT lptClick)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pItem);

	ClearEndEditReason ();

	if (!EndEditItem ())
	{
		return FALSE;
	}

	if (pItem->IsGroup () && !pItem->HasValueField ())
	{
		return FALSE;
	}

	ASSERT (pItem == GetCurSel ());

	if (pItem->OnEdit (lptClick))
	{
		pItem->Redraw ();
		//SetCurSel (m_idActive, SM_SET_ACTIVE_ITEM);
		SetCapture ();
	}
	else
	{
		return FALSE;
	}

	return TRUE;
}
//******************************************************************************************
void CBCGPGridCtrl::OnClickButton (CPoint point)
{
	ASSERT_VALID (this);

	CBCGPGridRow* pSel = GetCurSel ();
	ASSERT_VALID (pSel);

	if (pSel->OnUpdateValue ())
	{
		CBCGPGridItem* pItem = GetCurSelItem (pSel);
		if (pItem!= NULL)
		{
			pItem->DoClickButton (point);
		}
	}
}
//******************************************************************************************
BOOL CBCGPGridCtrl::EndEditItem (BOOL bUpdateData/* = TRUE*/)
{
	ASSERT_VALID (this);

	CBCGPGridRow* pSel = GetCurSel ();

	if (pSel == NULL)
	{
		return TRUE;
	}

	ASSERT_VALID (pSel);

	if (!pSel->m_bInPlaceEdit)
	{
		return TRUE;
	}

	// Update cell data from in-place edit
	if (bUpdateData)
	{
		if (!ValidateItemData (pSel) || !pSel->OnUpdateValue ())
		{
			ClearEndEditReason ();
			return FALSE;
		}
	}
	else
	{
		m_bUpdateItemData = FALSE;
	}

	// Close in-place edit
	if (!pSel->OnEndEdit ())
	{
		if (!bUpdateData)
		{
			m_bUpdateItemData = TRUE;
		}

		ClearEndEditReason ();
		return FALSE;
	}

	if (!bUpdateData)
	{
		m_bUpdateItemData = TRUE;
	}

	if (::GetCapture () == GetSafeHwnd ())
	{
		ReleaseCapture ();
	}

	pSel->Redraw ();

	return TRUE;
}
//*******************************************************************************************
BOOL CBCGPGridCtrl::PreTranslateMessage(MSG* pMsg) 
{
	if (m_nDraggedColumn >= 0 || m_bTracking || m_bSelecting)
	{
		return CBCGPWnd::PreTranslateMessage(pMsg);
	}

   	switch (pMsg->message)
	{
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
	case WM_MOUSEMOVE:
		if (m_pToolTip->GetSafeHwnd () != NULL)
		{
			m_pToolTip->RelayEvent(pMsg);
		}
		break;
	}

   	switch (pMsg->message)
	{
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
	case WM_NCLBUTTONDOWN:
	case WM_NCRBUTTONDOWN:
	case WM_NCMBUTTONDOWN:
	case WM_NCLBUTTONUP:
	case WM_NCRBUTTONUP:
	case WM_NCMBUTTONUP:
		if (m_ToolTip.GetSafeHwnd () != NULL &&
			m_ToolTip.IsWindowVisible ())
		{
			m_ToolTip.Hide ();

			if (::GetCapture () == GetSafeHwnd ())
			{
				ReleaseCapture ();
			}

			return CBCGPWnd::PreTranslateMessage(pMsg);
		}
		break;
		
	case WM_MOUSEMOVE:
		if (pMsg->wParam == 0)	// No buttons pressed
		{
			CPoint ptCursor;
			::GetCursorPos (&ptCursor);
			ScreenToClient (&ptCursor);

			TrackToolTip (ptCursor);
		}
		break;
	}

	CBCGPGridRow* pSel = GetCurSel ();

	if (pMsg->message == WM_SYSKEYDOWN && 
		(pMsg->wParam == VK_DOWN || pMsg->wParam == VK_RIGHT) && 
		pSel != NULL && pSel->m_bEnabled)
	{
		CBCGPGridItem* pItem = GetCurSelItem (pSel);

		DWORD dwEditReason = BeginEdit_ComboOpen;
		if (pMsg->wParam == VK_DOWN)
		{
			dwEditReason |= BeginEdit_Down;
		}
		if (pMsg->wParam == VK_RIGHT)
		{
			dwEditReason |= BeginEdit_Right;
		}
		SetBeginEditReason (dwEditReason);

  		if (pItem != NULL && 
			((pItem->m_dwFlags) & BCGP_GRID_ITEM_HAS_LIST) &&
			EditItem (pSel))
		{
			pItem->DoClickButton (CPoint (-1, -1));

			DoInplaceEditSetSel (OnInplaceEditSetSel (pItem, dwEditReason));
		}

		return TRUE;
	}

	if (pSel != NULL && pSel->m_bInPlaceEdit && pSel->m_bEnabled)
	{
		ASSERT_VALID (pSel);

		if (pMsg->message == WM_KEYDOWN)
		{
			return OnInplaceEditKeyDown (pSel, pMsg);
		}
		else if (pMsg->message >= WM_MOUSEFIRST &&
				 pMsg->message <= WM_MOUSELAST)
		{
			CPoint ptCursor;
			::GetCursorPos (&ptCursor);
			ScreenToClient (&ptCursor);

			CSpinButtonCtrl* pWndSpin = pSel->GetSpinWnd ();
			if (pWndSpin != NULL)
			{
				ASSERT_VALID (pWndSpin);
				ASSERT (pWndSpin->GetSafeHwnd () != NULL);

				CRect rectSpin;
				pWndSpin->GetClientRect (rectSpin);
				pWndSpin->MapWindowPoints (this, rectSpin);

				if (rectSpin.PtInRect (ptCursor))
				{
					MapWindowPoints (pWndSpin, &ptCursor, 1); 

					pWndSpin->SendMessage (pMsg->message, pMsg->wParam, 
						MAKELPARAM (ptCursor.x, ptCursor.y));
					return TRUE;
				}
			}

			CWnd* pWndInPlaceEdit = pSel->GetInPlaceWnd ();
			if (pWndInPlaceEdit == NULL)
			{
				return CBCGPWnd::PreTranslateMessage(pMsg);
			}

			ASSERT_VALID (pWndInPlaceEdit);

			if (!pSel->m_bAllowEdit)
			{
				pWndInPlaceEdit->HideCaret ();
			}

			CRect rectEdit;
			pWndInPlaceEdit->GetClientRect (rectEdit);
			pWndInPlaceEdit->MapWindowPoints (this, rectEdit);

			if (rectEdit.PtInRect (ptCursor) &&
				pMsg->message == WM_LBUTTONDBLCLK)
			{
				if (pSel->OnDblClick (ptCursor))
				{
					return TRUE;
				}
			}

			if (rectEdit.PtInRect (ptCursor) && 
				pMsg->message == WM_RBUTTONDOWN &&
				!pSel->m_bAllowEdit)
			{
				return TRUE;
			}

			if (!rectEdit.PtInRect (ptCursor) &&
				(pMsg->message == WM_LBUTTONDOWN ||
				pMsg->message == WM_NCLBUTTONDOWN))
			{
				CBCGPGridItem* pItem = GetCurSelItem (pSel);
				if (pItem!= NULL && pItem->m_rectButton.PtInRect (ptCursor))
				{
					pItem->DoClickButton (ptCursor);
					return TRUE;
				}

				SetEndEditReason (EndEdit_AutoApply | EndEdit_Selection);
				if (!EndEditItem ())
				{
					m_bNoUpdateWindow = FALSE;
					return TRUE;
				}
			}
			else
			{
				MapWindowPoints (pWndInPlaceEdit, &ptCursor, 1); 
				pWndInPlaceEdit->SendMessage (pMsg->message, pMsg->wParam, 
					MAKELPARAM (ptCursor.x, ptCursor.y));
				return TRUE;
			}
		}
	}

	return CBCGPWnd::PreTranslateMessage(pMsg);
}
//*******************************************************************************************
void CBCGPGridCtrl::OnCancelMode() 
{
	if (m_nDraggedColumn >= 0)
	{
		// Cancel dragging a column:
		CPoint pt(-1, -1);
		StopDragColumn (pt, FALSE);
	}
	if (m_bTracking)
	{
		TrackHeader (-1);
		m_bTracking = FALSE;

		if (::GetCapture () == GetSafeHwnd ())
		{
			ReleaseCapture ();
		}
	}
	if (m_bSelecting)
	{
		KillTimer (GRID_CLICKVALUE_TIMER_ID);
		m_bClickTimer = FALSE;
		m_ptClickOnce = CPoint (0, 0);

		StopSelectItems ();
	}

	//---------------------
	// Header highlighting:
	//---------------------
	BOOL bRedraw = FALSE;
	if (GetColumnsInfo ().GetHighlightColumn () >= 0)
	{
		GetColumnsInfo ().SetHighlightColumn (-1);
		bRedraw = TRUE;
	}
	if (GetColumnsInfo ().GetHighlightColumnBtn () >= 0)
	{
		GetColumnsInfo ().SetHighlightColumnBtn (-1);
		bRedraw = TRUE;
	}

	if (bRedraw)
	{
		RedrawWindow ();
	}

	//---------
	// Tooltip:
	//---------
	if (::GetCapture () == GetSafeHwnd ())
	{
		ReleaseCapture ();
	}
	m_ToolTip.Deactivate ();

	SetEndEditReason (EndEdit_Cancel);
	EndEditItem (FALSE);
	CBCGPWnd::OnCancelMode();
}
//******************************************************************************************
void CBCGPGridCtrl::OnSetFocus(CWnd* pOldWnd) 
{
	CBCGPWnd::OnSetFocus(pOldWnd);
	
	m_bFocused = TRUE;
	
	if (m_bFilterBar && m_nFocusedFilter != -1)
	{
		CWnd* pCtrl = GetColumnsInfo ().GetColumnFilterBarCtrl (m_nFocusedFilter);
		if (pCtrl->GetSafeHwnd () != NULL && pCtrl->IsWindowVisible ())
		{
			pCtrl->SetFocus ();
		}
	}

	CBCGPGridRow* pSel = GetCurSel ();
	if (pSel != NULL)
	{
		InvalidateRect (pSel->m_Rect);

		for (POSITION pos = m_lstSel.GetHeadPosition (); pos != NULL; )
		{
			CBCGPGridRange* pRange = (CBCGPGridRange*) m_lstSel.GetNext (pos);
			ASSERT (pRange != NULL);

			InvalidateRange (*pRange);
		}

		if (!m_bNoUpdateWindow)
		{
			UpdateWindow ();
		}
	}
}
//******************************************************************************************
void CBCGPGridCtrl::OnKillFocus(CWnd* pNewWnd) 
{
	BOOL bIsParentView =
		pNewWnd != NULL &&
		pNewWnd->IsKindOf (RUNTIME_CLASS (CBCGPGridView)) &&
		GetParent ()->GetSafeHwnd () == pNewWnd->GetSafeHwnd ();

	HWND hwndColumnChooser = 
		(m_pColumnChooser != NULL) ? m_pColumnChooser->GetSafeHwnd () : NULL;

	if (pNewWnd->GetSafeHwnd () == NULL ||
		(!bIsParentView && !IsChild (pNewWnd) &&
		pNewWnd->GetSafeHwnd () != hwndColumnChooser))
	{
		CBCGPGridRow* pSel = GetCurSel ();

		if (pSel == NULL || pSel->OnKillFocus (pNewWnd))
		{
			SetEndEditReason (EndEdit_AutoApply | EndEdit_KillFocus);
			EndEditItem (FALSE);
			m_bFocused = FALSE;

			for (POSITION pos = m_lstSel.GetHeadPosition (); pos != NULL; )
			{
				CBCGPGridRange* pRange = (CBCGPGridRange*) m_lstSel.GetNext (pos);
				ASSERT (pRange != NULL);

				InvalidateRange (*pRange);
			}

			if (pSel != NULL)
			{
				if (m_bVirtualMode)
				{
					pSel = GetCurSel ();

					if (pSel != NULL)
					{
						ASSERT_VALID (pSel);
						pSel->Redraw ();
					}
				}
				else
				{
					pSel->Redraw ();
				}
			}
		}
	}

	CBCGPWnd::OnKillFocus(pNewWnd);
}
//******************************************************************************************
void CBCGPGridCtrl::OnStyleChanged (int nStyleType, LPSTYLESTRUCT lpStyleStruct)
{
	CBCGPWnd::OnStyleChanged (nStyleType, lpStyleStruct);
	SetRebuildTerminalItems ();
	AdjustLayout ();
}
//******************************************************************************************
UINT CBCGPGridCtrl::OnGetDlgCode() 
{
	return DLGC_WANTARROWS | DLGC_WANTCHARS;
}
//******************************************************************************************
void CBCGPGridCtrl::OnMeasureListRect (CRect& rect)
{
	int nGroupsCount = GetGroupsCount (TRUE);
	int nItemsCount = GetTotalItems (TRUE) - nGroupsCount;

	if (!IsKindOf (RUNTIME_CLASS (CBCGPReportCtrl)))
	{
		nItemsCount += m_nMultiLineExtraRowsCount;
	}
		
	int nPreviewHeightTotal = CalcExtraHeightTotal ();
	
	rect.bottom = rect.top + 
		nGroupsCount * m_nLargeRowHeight + 
		nItemsCount * m_nRowHeight + nPreviewHeightTotal;
}
//******************************************************************************************
void CBCGPGridCtrl::SetScrollSizes ()
{
	ASSERT_VALID (this);

	// -----------------------
	// Calculate scroll sizes:
	// -----------------------
	if (m_nRowHeight == 0)
	{
		m_nVertScrollPage = 0;
		m_nVertScrollTotal = 0;
	}
	else
	{
		m_nVertScrollPage = m_rectList.Height ();

		CRect rect (0, 0, 0, 0);
		OnMeasureListRect (rect);

		m_nVertScrollTotal = rect.Height ();
	}

	if (FALSE)
	{
		m_nHorzScrollPage = 0;
		m_nHorzScrollTotal = 0;
	}
	else
	{
		m_nHorzScrollPage = m_rectList.Width ();
		m_nHorzScrollTotal = GetColumnsInfo ().GetTotalWidth ();
	}

	int cxScroll = m_bScrollVert ? ::GetSystemMetrics (SM_CXVSCROLL) : 0;
	int cyScroll = m_bScrollHorz ? ::GetSystemMetrics (SM_CYHSCROLL) : 0;

	// Scrollbars deflate the visible area
	if (m_nVertScrollTotal + cxScroll > m_nVertScrollPage && !GetColumnsInfo ().IsAutoSize ())
	{
		m_nHorzScrollPage -= cxScroll;
	}

	// Frozen columns deflate scrollable area
	if (GetColumnsInfo ().IsFreezeColumnsEnabled () && m_nHorzScrollPage > 0)
	{
		if (GetColumnsInfo ().GetFreezeOffset () >= m_nHorzScrollPage)
		{	// all available area is frozen - no horz scroll
			m_nHorzScrollPage = 0;
			m_nHorzScrollTotal = 0;
		}
		else
		{
			m_nHorzScrollPage -= GetColumnsInfo ().GetFreezeOffset ();
			m_nHorzScrollTotal -= GetColumnsInfo ().GetFreezeOffset ();
		}
	}

	if (m_nHorzScrollTotal > m_nHorzScrollPage || m_bScrollHorzShowAlways)
	{
		m_nVertScrollPage -= cyScroll;
	}

	// --------------------------
	// Set vertical scroll sizes:
	// --------------------------
	if (m_wndScrollVert.GetSafeHwnd () == NULL)
	{
		return;
	}
	
	if (m_nRowHeight == 0)
	{
		m_nVertScrollOffset = 0;
	}
	else
	{
		if (m_nVertScrollTotal <= m_nVertScrollPage)
		{
			m_nVertScrollPage = 0;
			m_nVertScrollTotal = 0;
		}

		int nPrevOffset = m_nVertScrollOffset;
		m_nVertScrollOffset = max (0, min (m_nVertScrollOffset, 
			m_nVertScrollTotal - m_nVertScrollPage + 1));
		if (m_bVirtualMode)
		{
			// row alignment
			int nResidue = m_nVertScrollOffset % m_nRowHeight;
			if (nResidue > 0)
			{
				m_nVertScrollOffset -= nResidue;
				if (m_nVertScrollOffset + m_nVertScrollPage < m_nVertScrollTotal)
				{
					m_nVertScrollOffset += m_nRowHeight;
				}
			}
		}

		if (m_nVertScrollOffset != nPrevOffset)
		{
			OnUpdateVScrollPos (m_nVertScrollOffset, nPrevOffset);
		}
	}

	SCROLLINFO si;

	ZeroMemory (&si, sizeof (SCROLLINFO));
	si.cbSize = sizeof (SCROLLINFO);

	si.fMask = SIF_RANGE | SIF_POS | SIF_PAGE;
	si.nMin = 0;
	si.nMax = m_nVertScrollTotal;
	si.nPage = m_nVertScrollPage;
	si.nPos = m_nVertScrollOffset;

	SetScrollInfo (SB_VERT, &si, TRUE);
	m_wndScrollVert.EnableScrollBar (m_nVertScrollTotal > 0 ? ESB_ENABLE_BOTH : ESB_DISABLE_BOTH);
	m_wndScrollVert.EnableWindow ();

	// ----------------------------
	// Set horizontal scroll sizes:
	// ----------------------------
	if (m_wndScrollHorz.GetSafeHwnd () == NULL)
	{
		return;
	}

	if (FALSE)
	{
		m_nHorzScrollOffset = 0;
	}
	else
	{
		if (m_nHorzScrollTotal <= m_nHorzScrollPage || GetColumnsInfo ().IsAutoSize ())
		{
			m_nHorzScrollPage = 0;
			m_nHorzScrollTotal = 0;
		}

		m_nHorzScrollOffset = min (max (0, m_nHorzScrollOffset), 
			m_nHorzScrollTotal - m_nHorzScrollPage + 1);
	}

	ZeroMemory (&si, sizeof (SCROLLINFO));
	si.cbSize = sizeof (SCROLLINFO);

	si.fMask = SIF_RANGE | SIF_POS | SIF_PAGE;
	si.nMin = 0;
	si.nMax = m_nHorzScrollTotal;
	si.nPage = m_nHorzScrollPage;
	si.nPos = m_nHorzScrollOffset;

	SetScrollInfo (SB_HORZ, &si, TRUE);
	m_wndScrollHorz.EnableScrollBar (m_nHorzScrollTotal > 0 ? ESB_ENABLE_BOTH : ESB_DISABLE_BOTH);
	m_wndScrollHorz.EnableWindow ();
}
//******************************************************************************************
int CBCGPGridCtrl::CalcExtraHeightTotal ()
{
	ASSERT_VALID (this);

	if (!IsRowExtraHeightAllowed ())
	{
		return 0;
	}

	if (IsVirtualMode ())
	{
		return 0;
	}

	int nPreviewHeightTotal = 0;
	int nIndex = 0;

	if (!IsSortingMode () && !IsGrouping ())
	{
		for (POSITION pos = m_lstItems.GetHeadPosition (); pos != NULL;
			nIndex++)
		{
			CBCGPGridRow* pItem = m_lstItems.GetNext (pos);
			ASSERT_VALID (pItem);

			if (!pItem->IsGroup () && pItem->IsItemVisible ())
			{
				int nHeight = pItem->m_Rect.Height ();
				if (nHeight > m_nRowHeight)
				{
					nPreviewHeightTotal += nHeight - m_nRowHeight;
				}
			}
		}
	}
	else
	{
		for (POSITION pos = m_lstTerminalItems.GetHeadPosition (); pos != NULL;
			nIndex++)
		{
			CBCGPGridRow* pItem = m_lstTerminalItems.GetNext (pos);
			ASSERT_VALID (pItem);

			if (!pItem->IsGroup () && pItem->IsItemVisible ())
			{
				int nHeight = pItem->m_Rect.Height ();
				if (nHeight > m_nRowHeight)
				{
					nPreviewHeightTotal += nHeight - m_nRowHeight;
				}
			}
		}
	}

	return nPreviewHeightTotal;
}
//******************************************************************************************
int CBCGPGridCtrl::GetPageItems (int& nFirst, int& nLast, int nSearchFrom) const
{	
	ASSERT_VALID (this);
	ASSERT (nSearchFrom >= 0);

	int nCount = 0;
	nFirst = -1;

	const CList<CBCGPGridRow*, CBCGPGridRow*>& lst =
		(!IsSortingMode () && !IsGrouping ()) ? m_lstItems : m_lstTerminalItems;

	int i = 0;
	POSITION pos = lst.GetHeadPosition ();
	if (nSearchFrom > 0)
	{
		i = nSearchFrom;
		pos = lst.FindIndex (nSearchFrom);
	}
	for (; pos != NULL; i++)
	{
		CBCGPGridRow* pItem = lst.GetNext (pos);
		ASSERT_VALID (pItem);

		if (!pItem->m_Rect.IsRectEmpty ())
		{
			if (pItem->m_Rect.bottom >= m_rectList.bottom)
			{
				nLast = i - 1;
				return nCount;
			}

			if (pItem->m_Rect.bottom >= m_rectList.top)
			{
				nCount ++;
				
				if (nFirst == -1)
				{
					nFirst = i;
				}
			}
		}
	}

	nLast = i - 1;

	if (nFirst == -1)
	{
		nLast = -1;
	}
	return nCount;
}
//******************************************************************************************
int CBCGPGridCtrl::GetTotalItems (BOOL bCalcVisibleOnly) const
{
	ASSERT_VALID (this);

	m_nMultiLineExtraRowsCount = 0;

	if (m_bVirtualMode)
	{
		return m_nVirtualRows;
	}

	if (bCalcVisibleOnly)
	{
		int nCount = 0;
		
		const CList<CBCGPGridRow*, CBCGPGridRow*>& lst =
			(!IsSortingMode () && !IsGrouping ()) ? m_lstItems : m_lstTerminalItems;

		BOOL bShowAllItems = (IsSortingMode () && !IsGrouping ());

		for (POSITION pos = lst.GetHeadPosition (); pos != NULL; )
		{
			CBCGPGridRow* pItem = lst.GetNext (pos);
			ASSERT_VALID (pItem);

			BOOL bShowItem = bShowAllItems ? !pItem->IsItemFiltered () : pItem->IsItemVisible ();
			if (bShowItem)
			{
				nCount++;

				if (pItem->m_nLines > 1)
				{
					m_nMultiLineExtraRowsCount += (pItem->m_nLines - 1);
				}
			}
		}

		return nCount;
	}

	if (!IsSortingMode () && !IsGrouping ())
	{
		return (int) m_lstItems.GetCount ();
	}
	else
	{
		return (int) m_lstTerminalItems.GetCount ();
	}
}
//******************************************************************************************
int CBCGPGridCtrl::GetTotalItems (int nCountFrom, int nCountTo, 
								  BOOL bCalcVisibleOnly) const
{
	ASSERT_VALID (this);
	
	m_nMultiLineExtraRowsCount = 0;

	if (nCountTo < 0 || nCountFrom < 0)
	{
		return 0;
	}

	if (nCountTo < nCountFrom)
	{
		int nTmp = nCountFrom;
		nCountFrom = nCountTo;
		nCountTo = nTmp;
	}

	if (m_bVirtualMode)
	{
		nCountFrom = min (m_nVirtualRows, nCountFrom);
		nCountTo = min (m_nVirtualRows, nCountTo);
		return nCountTo - nCountFrom;
	}

	int nCount = 0;

	const CList<CBCGPGridRow*, CBCGPGridRow*>& lst =
		(!IsSortingMode () && !IsGrouping ()) ? m_lstItems : m_lstTerminalItems;

	BOOL bShowAllItems = (IsSortingMode () && !IsGrouping ());

	int i = 0;
	for (POSITION pos = lst.GetHeadPosition (); pos != NULL && i < nCountTo; i++)
	{
		CBCGPGridRow* pItem = lst.GetNext (pos);
		ASSERT_VALID (pItem);

		if (i >= nCountFrom)
		{
			if (bCalcVisibleOnly)
			{
				BOOL bShowItem = bShowAllItems ? !pItem->IsItemFiltered () : pItem->IsItemVisible ();
				if (bShowItem)
				{
					nCount++;

					if (pItem->m_nLines > 1)
					{
						m_nMultiLineExtraRowsCount += (pItem->m_nLines - 1);
					}
				}
			}
			else
			{
				nCount++;
			}
		}
	}

	return nCount;
}
//******************************************************************************************
int CBCGPGridCtrl::GetGroupsCount (BOOL bCalcVisibleOnly) const
{
	ASSERT_VALID (this);

	if (m_bVirtualMode)
	{
		return 0;
	}

	int nCount = 0;
	
	const CList<CBCGPGridRow*, CBCGPGridRow*>& lst =
		(!IsSortingMode () && !IsGrouping ()) ? m_lstItems : m_lstTerminalItems;

	BOOL bShowAllItems = (IsSortingMode () && !IsGrouping ());

	for (POSITION pos = lst.GetHeadPosition (); pos != NULL; )
	{
		CBCGPGridRow* pItem = lst.GetNext (pos);
		ASSERT_VALID (pItem);

        if (bCalcVisibleOnly)
        {
			BOOL bShowItem = bShowAllItems ? !pItem->IsItemFiltered () : pItem->IsItemVisible ();
			if (pItem->IsGroup () && bShowItem)
			{
				nCount++;
			}
        }
        else
        {
			if (pItem->IsGroup ())
			{
				nCount++;
			}
        }
	}

	return nCount;
}
//******************************************************************************************
int CBCGPGridCtrl::GetGroupsCount (int nCountFrom, int nCountTo,
								   BOOL bCalcVisibleOnly) const
{
	ASSERT_VALID (this);
	ASSERT (nCountFrom >= 0);
	ASSERT (nCountTo >= 0);

	if (m_bVirtualMode)
	{
		return 0;
	}

	if (nCountTo < nCountFrom)
	{
		int nTmp = nCountFrom;
		nCountFrom = nCountTo;
		nCountTo = nTmp;
	}

	int nCount = 0;

	const CList<CBCGPGridRow*, CBCGPGridRow*>& lst =
		(!IsSortingMode () && !IsGrouping ()) ? m_lstItems : m_lstTerminalItems;

	BOOL bShowAllItems = (IsSortingMode () && !IsGrouping ());

	int i = 0;
	for (POSITION pos = lst.GetHeadPosition (); pos != NULL && i < nCountTo; i++)
	{
		CBCGPGridRow* pItem = lst.GetNext (pos);
		ASSERT_VALID (pItem);

		if (i >= nCountFrom)
		{
			if (bCalcVisibleOnly)
			{
				BOOL bShowItem = bShowAllItems ? !pItem->IsItemFiltered () : pItem->IsItemVisible ();
				if (pItem->IsGroup () && bShowItem)
				{
					nCount++;
				}
			}
			else
			{
				if (pItem->IsGroup ())
				{
					nCount++;
				}
			}
		}
	}

	return nCount;
}
//******************************************************************************************
// Goes up or down from nStartFrom, skips invisible rows.
// nOffsetCount - count of the visible rows
// Returns an index of the row
int CBCGPGridCtrl::OffsetVisibleRow (int nStartFrom, int nOffsetCount, BOOL bDirForward) const
{
	ASSERT_VALID (this);

	if (nOffsetCount <= 0 || nStartFrom < 0)
	{
		return nStartFrom;
	}

	if (m_bVirtualMode)
	{
		int nResult = ((bDirForward) ? nStartFrom + nOffsetCount : nStartFrom - nOffsetCount);
		nResult = max (0, nResult);
		return min (m_nVirtualRows, nResult);
	}

	int nCount = 0;

	const CList<CBCGPGridRow*, CBCGPGridRow*>& lst =
		(!IsSortingMode () && !IsGrouping ()) ? m_lstItems : m_lstTerminalItems;

	BOOL bShowAllItems = (IsSortingMode () && !IsGrouping ());

	int iResult = nStartFrom;
	int index = nStartFrom;
	for (POSITION pos = lst.FindIndex (nStartFrom); pos != NULL && nCount < nOffsetCount; )
	{
		CBCGPGridRow* pItem = bDirForward ? lst.GetNext (pos) : lst.GetPrev (pos);
		ASSERT_VALID (pItem);

		BOOL bShowItem = bShowAllItems ? !pItem->IsItemFiltered () : pItem->IsItemVisible ();
		if (bShowItem)
		{
			nCount++;
			iResult = index;
		}
		if (bDirForward) 
		{
			index++;
		}
		else
		{
			index--;
		}
	}

	return iResult;
}
//******************************************************************************************
void CBCGPGridCtrl::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	if (m_nDraggedColumn >= 0 || m_bTracking || m_bSelecting)
	{
		return;
	}

	CBCGPGridRow* pSel = GetCurSel ();

	if (pSel != NULL && 
		pScrollBar->GetSafeHwnd () != NULL)
	{
		CSpinButtonCtrl* pWndSpin = pSel->GetSpinWnd ();
		if (pWndSpin != NULL && 
			pWndSpin->GetSafeHwnd () == pScrollBar->GetSafeHwnd ())
		{
			return;
		}
	}

	SetEndEditReason (EndEdit_AutoApply | EndEdit_Layout);
	EndEditItem ();

	SCROLLINFO info;
	info.cbSize = sizeof(SCROLLINFO);
	info.fMask = SIF_TRACKPOS;

	if (nSBCode == SB_THUMBTRACK || nSBCode == SB_THUMBPOSITION)
	{
		GetScrollInfo(SB_VERT, &info);
		nPos = info.nTrackPos;
	}

	int nPrevOffset = m_nVertScrollOffset;

	int nVertScrollPage = m_nVertScrollPage;
	if (m_bVirtualMode)
	{
		int nItemsCount = max (1, m_nVertScrollPage / m_nRowHeight);
		nVertScrollPage = nItemsCount * m_nRowHeight;
	}

	switch (nSBCode)
	{
	case SB_LINEUP:
		m_nVertScrollOffset -= m_nRowHeight;
		break;

	case SB_LINEDOWN:
		m_nVertScrollOffset += m_nRowHeight;
		break;

	case SB_TOP:
		m_nVertScrollOffset = 0;
		break;

	case SB_BOTTOM:
		m_nVertScrollOffset = m_nVertScrollTotal;
		break;

	case SB_PAGEUP:
		m_nVertScrollOffset -= nVertScrollPage;
		break;

	case SB_PAGEDOWN:
		m_nVertScrollOffset += nVertScrollPage;
		break;

	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		m_nVertScrollOffset = nPos;
		break;

	default:
		return;
	}

	m_nVertScrollOffset = max (0, min (m_nVertScrollOffset, 
		m_nVertScrollTotal - nVertScrollPage + 1));
	if (m_bVirtualMode)
	{
		// row alignment
		int nResidue = m_nVertScrollOffset % m_nRowHeight;
		if (nResidue > 0)
		{
			m_nVertScrollOffset -= nResidue;
			if (m_nVertScrollOffset + m_nVertScrollPage < m_nVertScrollTotal)
			{
				m_nVertScrollOffset += m_nRowHeight;
			}
		}
	}

	if (m_nVertScrollOffset == nPrevOffset)
	{
		return;
	}

	OnUpdateVScrollPos (m_nVertScrollOffset, nPrevOffset);

	SetScrollPos (SB_VERT, m_nVertScrollOffset);

	int dy = nPrevOffset - m_nVertScrollOffset;

	ShiftItems (0, dy);
	OnPosSizeChanged ();
	m_rectTrackSel = OnGetSelectionRect ();
	ScrollWindow (0, dy, m_rectList, m_rectList);
	ScrollWindow (0, dy, m_rectRowHeader, m_rectRowHeader);

	pSel = GetCurSel ();
	if (pSel != NULL)
	{
		pSel->AdjustButtonRect ();
	}

	OnAfterVScroll (m_nVertScrollOffset, nPrevOffset);
	
	UpdateWindow ();
}
//*******************************************************************************************
void CBCGPGridCtrl::OnUpdateVScrollPos (int /*nVOffset*/, int /*nPrevVOffset*/)
{
}
//*******************************************************************************************
void CBCGPGridCtrl::OnAfterVScroll (int /*nVOffset*/, int /*nPrevVOffset*/)
{
}
//*******************************************************************************************
void CBCGPGridCtrl::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* /*pScrollBar*/)
{
	if (m_nDraggedColumn >= 0 || m_bTracking || m_bSelecting)
	{
		return;
	}

	SetEndEditReason (EndEdit_Cancel | EndEdit_Layout);
	EndEditItem (FALSE);

	int nPrevOffset = m_nHorzScrollOffset;

	SCROLLINFO info;
	info.cbSize = sizeof(SCROLLINFO);
	info.fMask = SIF_TRACKPOS;

	if (nSBCode == SB_THUMBTRACK || nSBCode == SB_THUMBPOSITION)
	{
		GetScrollInfo(SB_HORZ, &info);
		nPos = info.nTrackPos;
	}

	switch (nSBCode)
	{
	case SB_LINEUP:
		m_nHorzScrollOffset -= max (1, m_nBaseHeight);
		break;

	case SB_LINEDOWN:
		m_nHorzScrollOffset += max (1, m_nBaseHeight);
		break;

	case SB_TOP:
		m_nHorzScrollOffset = 0;
		break;

	case SB_BOTTOM:
		m_nHorzScrollOffset = m_nHorzScrollTotal;
		break;

	case SB_PAGEUP:
		m_nHorzScrollOffset -= m_nHorzScrollPage;
		break;

	case SB_PAGEDOWN:
		m_nHorzScrollOffset += m_nHorzScrollPage;
		break;

	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		m_nHorzScrollOffset = nPos;
		break;

	default:
		return;
	}

	m_nHorzScrollOffset = min (max (0, m_nHorzScrollOffset), 
		m_nHorzScrollTotal - m_nHorzScrollPage + 1);

	if (m_nHorzScrollOffset == nPrevOffset)
	{
		return;
	}

	OnUpdateHScrollPos (m_nHorzScrollOffset, nPrevOffset);

	SetScrollPos (SB_HORZ, m_nHorzScrollOffset);

	int dx = nPrevOffset - m_nHorzScrollOffset;

	CRect rectClip = m_rectList;
	if (GetColumnsInfo ().IsFreezeColumnsEnabled ())
	{
		rectClip.left = min (m_rectList.left + GetColumnsInfo ().GetFreezeOffset (), m_rectList.right);
	}

	ShiftItems (dx, 0);
	OnPosSizeChanged ();
	m_rectTrackSel = OnGetSelectionRect ();

	if (m_bFreezeGroups)
	{
		InvalidateRect (m_rectList);
	}
	else
	{
		ScrollWindow (dx, 0, m_rectList, rectClip);
	}

	OnAfterHScroll (m_nHorzScrollOffset, nPrevOffset);

	if (!m_rectHeader.IsRectEmpty ())
	{
		RedrawWindow (m_rectHeader);
	}

	if (m_nGridHeaderHeight > 0)
	{
		RedrawWindow (GetGridHeaderRect ());
	}

	if (m_nGridFooterHeight > 0)
	{
		RedrawWindow (GetGridFooterRect ());
	}

	AdjustFilterBarCtrls ();

	if (!m_rectFilterBar.IsRectEmpty ())
	{
		RedrawWindow (m_rectFilterBar);
	}
}
//*******************************************************************************************
void CBCGPGridCtrl::OnUpdateHScrollPos (int /*nHOffset*/, int /*nPrevHOffset*/)
{
}
//*******************************************************************************************
void CBCGPGridCtrl::OnAfterHScroll (int /*nHOffset*/, int /*nPrevHOffset*/)
{
}
//*******************************************************************************************
CScrollBar* CBCGPGridCtrl::GetScrollBarCtrl(int nBar) const
{
	if (nBar == SB_HORZ && m_wndScrollHorz.GetSafeHwnd () != NULL)
	{
		return (CScrollBar* ) &m_wndScrollHorz;
	}
	
	if (nBar == SB_VERT && m_wndScrollVert.GetSafeHwnd () != NULL)
	{
		return (CScrollBar* ) &m_wndScrollVert;
	}

	return NULL;
}
//******************************************************************************************
BOOL CBCGPGridCtrl::OnMouseWheel(UINT /*nFlags*/, short zDelta, CPoint /*pt*/) 
{
#ifndef _BCGPGRID_STANDALONE
	if (CBCGPPopupMenu::GetActiveMenu () != NULL)
	{
		return TRUE;
	}
#endif

	if (m_nDraggedColumn >= 0 || m_bTracking || m_bSelecting)
	{
		return FALSE;
	}

	if (m_nVertScrollTotal <= m_nVertScrollPage)
	{
		return FALSE;
	}

    if ((m_fScrollRemainder > 0) != (zDelta > 0))
    {
        m_fScrollRemainder = 0;
    }

	UINT nLinesToScrollUserSetting;
	if (!::SystemParametersInfo (SPI_GETWHEELSCROLLLINES, 0, &nLinesToScrollUserSetting, 0))
	{
		nLinesToScrollUserSetting = 1;
	}

	if(nLinesToScrollUserSetting == WHEEL_PAGESCROLL) // scroll one page at a time
	{
		OnVScroll (zDelta < 0 ? SB_PAGEDOWN : SB_PAGEUP, 0, NULL);
		return TRUE;
	}

    float fTotalLinesToScroll = ((float)zDelta / WHEEL_DELTA) * nLinesToScrollUserSetting + m_fScrollRemainder;

    int nSteps = abs((int)fTotalLinesToScroll);
    m_fScrollRemainder = fTotalLinesToScroll - (int)fTotalLinesToScroll;

	if (m_nMouseWheelSmoothScrollMaxLimit != -1 && nSteps >= m_nMouseWheelSmoothScrollMaxLimit)
	{
		int nNewVertOffset = (zDelta < 0) ? (m_nVertScrollOffset + m_nRowHeight * nSteps): (m_nVertScrollOffset - m_nRowHeight * nSteps);
		OnVScroll (SB_THUMBPOSITION, nNewVertOffset, NULL);
		return TRUE;
	}

	for (int i = 0; i < nSteps; i++)
	{
		OnVScroll (zDelta < 0 ? SB_LINEDOWN : SB_LINEUP, 0, NULL);
	}

	return TRUE;
}
//*******************************************************************************************
void CBCGPGridCtrl::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	CBCGPWnd::OnLButtonDblClk(nFlags, point);

	// if header double click:
    if (m_rectHeader.PtInRect (point))
    {
		if (m_nDraggedColumn >= 0 || m_bTracking || m_bSelecting)
		{
			// do nothing
		}
		else
		{
			int nClickColumn = GetColumnsInfo ().HitTestColumn (point, TRUE, STRETCH_DELTA);
			if (nClickColumn >= 0)
			{
				OnHeaderDividerDblClick (nClickColumn);
			}
		}

		return;
    }

	//-------------------------
	// Row header double click:
	//-------------------------
	CRect rectRowHeader;
	CBCGPGridRow* pHitHeaderRow = HitTestRowHeader (point, rectRowHeader);
	if (pHitHeaderRow != NULL)
	{
		OnRowHeaderDblClick (pHitHeaderRow, point, rectRowHeader);
		return;
	}
	
	CBCGPGridRow* pSel = GetCurSel ();

	if (pSel == NULL || !pSel->GetRect ().PtInRect (point))
	{
		return;
	}

	ASSERT_VALID (pSel);

	if (pSel->IsGroup ())
	{
		CBCGPGridRow::ClickArea clickArea;
		CBCGPGridItemID id;
		CBCGPGridItem* pHitItem = NULL;
		HitTest (point, id, pHitItem, &clickArea);

		if (clickArea != CBCGPGridRow::ClickExpandBox)
		{
			pSel->Expand (!pSel->IsExpanded ());
		}
	}
	else if (pSel->m_bEnabled)
	{
		CWnd* pWndInPlace = pSel->GetInPlaceWnd ();
		SetBeginEditReason (BeginEdit_MouseClick | BeginEdit_MouseDblClick);
		if (EditItem (pSel) && pWndInPlace != NULL)
		{
			pWndInPlace->SendMessage (WM_LBUTTONDOWN);
			pWndInPlace->SendMessage (WM_LBUTTONUP);

			DoInplaceEditSetSel (OnInplaceEditSetSel (GetCurSelItem (pSel), BeginEdit_MouseClick | BeginEdit_MouseDblClick));
		}

		pSel->OnDblClick (point);
	}
}
//*******************************************************************************************
BOOL CBCGPGridCtrl::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	if (nHitTest == HTCLIENT)
	{
		CPoint point;

		::GetCursorPos (&point);
		ScreenToClient (&point);

 		if (m_rectList.PtInRect (point) || m_rectHeader.PtInRect (point))
		{
			if (OnTrackHeader () || m_rectHeader.PtInRect (point))
			{
				CPoint ptHeader = point;
				ptHeader.y = m_rectHeader.top;

				int nColumnHit = GetColumnsInfo ().HitTestColumn (ptHeader, TRUE, STRETCH_DELTA);
				if (nColumnHit != -1)
				{
					if (GetColumnsInfo ().GetColumnWidth (nColumnHit) > 0)
					{
						if (GetColumnsInfo ().CanChangeWidth (nColumnHit))
						{
							::SetCursor (globalData.m_hcurStretch);
							return TRUE;
						}
					}
				}
			}

			if (IsDragSelectionBorderEnabled () && HitTestSelectionBorder (point))
			{
				::SetCursor (globalData.m_hcurSizeAll);
				return TRUE;
			}

			if (m_DropEffect != DROPEFFECT_NONE)
			{
				return CBCGPWnd::OnSetCursor(pWnd, nHitTest, message);
			}

			CBCGPGridRow::ClickArea clickArea;
			CBCGPGridItemID id;
			CBCGPGridItem* pHitItem = NULL;
			HitTest (point, id, pHitItem, &clickArea);

			if (pHitItem != NULL && clickArea == CBCGPGridRow::ClickValue)
			{
				BOOL bDragCursor = IsDragSelectionEnabled () && IsItemSelected (pHitItem->GetGridItemID ());
				BOOL bIsOverButton = pHitItem->m_rectButton.PtInRect (point);
				if (!bDragCursor&& !bIsOverButton && pHitItem->OnSetCursor ())
				{
					return TRUE;
				}
			}
		}
	}
	
	return CBCGPWnd::OnSetCursor(pWnd, nHitTest, message);
}
//******************************************************************************************
void CBCGPGridCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if (nChar == VK_ESCAPE)
	{
		if (m_nDraggedColumn >= 0)
		{
			// Cancel dragging a column:
			CPoint pt(-1, -1);
			StopDragColumn (pt, FALSE);
		}
		if (m_bTracking)
		{
			// Cancel resizing a column:
			TrackHeader (-1);
			m_bTracking = FALSE;

			if (::GetCapture () == GetSafeHwnd ())
			{
				ReleaseCapture ();
			}
		}
	}

	if (m_nDraggedColumn >= 0 || m_bTracking || m_bSelecting)
	{
		CBCGPWnd::OnKeyDown(nChar, nRepCnt, nFlags);
		return;
	}
	
	if (m_lstItems.GetCount() == 0 && !m_bVirtualMode)
	{
		CBCGPWnd::OnKeyDown(nChar, nRepCnt, nFlags);
		return;
	}

	CBCGPGridRow* pSel = GetCurSel ();

	switch (nChar)
	{
	case VK_F2:
		if (pSel != NULL && pSel->m_bEnabled)
		{
			EnsureVisible (pSel);
			SetBeginEditReason (BeginEdit_F2);
			if (EditItem (pSel))
			{
				DoInplaceEditSetSel (OnInplaceEditSetSel (GetCurSelItem (pSel), BeginEdit_F2));
			}
		}
		break;

	case VK_F4:
		if (pSel != NULL && pSel->m_bEnabled)
		{
			EnsureVisible (pSel);
			SetBeginEditReason (BeginEdit_F4);
			if (EditItem (pSel))
			{
				CBCGPGridItem* pItem = GetCurSelItem (pSel);
				if (pItem != NULL && 
					((pItem->m_dwFlags) & (BCGP_GRID_ITEM_HAS_BUTTON | BCGP_GRID_ITEM_HAS_LIST)))
				{
					pItem->DoClickButton (CPoint (-1, -1));
				}

				DoInplaceEditSetSel (OnInplaceEditSetSel (pItem, BeginEdit_F4 | BeginEdit_ComboOpen));
			}
			return;
		}
		break;

	case VK_SPACE:
		if (!m_bWholeRowSel && !m_bSingleSel)
		{
			const BOOL bShift = (::GetAsyncKeyState (VK_SHIFT) & 0x8000) != 0 && !m_bIgnoreShiftBtn;
			const BOOL bCtrl = (::GetAsyncKeyState (VK_CONTROL) & 0x8000) != 0 && !m_bIgnoreCtrlBtn;

			if (bShift && bCtrl) 
			{
				if (!m_idActive.IsNull ())
				{
					SetCurSel (m_idActive, SM_ALL);
				}
				return;
			}
			else if (bShift)
			{
				if (!m_idLastSel.IsNull ())
				{
					SetCurSel (m_idLastSel, SM_SINGLE_ITEM | SM_CONTINUE_SEL_GROUP | SM_ROW);
				}
				return;
			}
			else if (bCtrl)
			{
				if (!m_idLastSel.IsNull ())
				{
					SetCurSel (m_idLastSel, SM_SINGLE_ITEM | SM_CONTINUE_SEL_GROUP | SM_COLUMN);
				}
				return;
			}
		}
		break;

	case 0x41: // Ctrl+ A
		if (!m_bSingleSel && (::GetAsyncKeyState (VK_CONTROL) & 0x8000) != 0)
		{
			SetCurSel (m_idActive, SM_ALL);
			return;
		}
		break;

	case VK_LEFT:
		if ((::GetAsyncKeyState (VK_CONTROL) & 0x8000) != 0 && !m_bIgnoreCtrlBtn)
		{
			return;
		}
		else if (pSel != NULL && pSel->IsGroup () && pSel->IsExpanded () && 
			(!IsSortingMode () || (pSel->m_dwFlags & BCGP_GRID_ITEM_AUTOGROUP) != 0) &&
			!pSel->m_lstSubItems.IsEmpty ())
		{
			pSel->Expand (FALSE);
			return;
		}
		else if (!m_bWholeRowSel)
		{
			const BOOL bShift = (::GetAsyncKeyState (VK_SHIFT) & 0x8000) != 0 && !m_bIgnoreShiftBtn;

			// Go left to the nearest item:
			CBCGPGridRow* pRowCurSel = (bShift && !m_bSingleSel) ? m_pLastSelRow : m_pSelRow;
			CBCGPGridItem* pItemCurSel = (bShift && !m_bSingleSel) ? m_pLastSelItem : m_pSelItem;
			if (m_bVirtualMode)
			{
				CBCGPGridItemID idCurSel = (bShift && !m_bSingleSel) ? m_idLastSel: m_idActive;
				pRowCurSel = GetVirtualRow (idCurSel.m_nRow);
				pItemCurSel = (pRowCurSel != NULL) ? pRowCurSel->GetItem (idCurSel.m_nColumn) : NULL;
			}

			CBCGPGridItemID id;
			CBCGPGridItem* pItem = NULL;

			if (pItemCurSel != NULL)
			{
				if (m_bVirtualMode)
				{
					id = pItemCurSel->GetGridItemID ();
					//id.m_nColumn --;
					int nPos = GetColumnsInfo ().IndexToOrder (id.m_nColumn);

					// Get position order of the merged cell
					if (pItemCurSel->GetMergedCells () != NULL)
					{
						CBCGPGridRange range;
						if (pItemCurSel->GetMergedRange (range))
						{
							int nPosLeft = GetColumnsInfo ().IndexToOrder (range.m_nLeft);
							int nPosRight = GetColumnsInfo ().IndexToOrder (range.m_nRight);
							if (nPosLeft != -1 && nPosRight != -1)
							{
								nPos = max (nPosLeft, nPosRight);
							}
							else
							{
								nPos = -1;
							}
						}
					}

					if (nPos > 0)
					{
						id.m_nColumn = GetColumnsInfo ().OrderToIndex (--nPos);
					}
					else
					{
						id.m_nColumn = -1;
					}
				}
				else
				{
					CRect rect = pItemCurSel->GetRect ();
					if (pItemCurSel->GetMergedCells () != NULL)
					{
						rect.left = pItemCurSel->GetMergedRect ().left;
					}
					CPoint point (rect.left - 2, rect.bottom - 1);

					int nColumn = -1;
					if (pItemCurSel->m_pGridRow->HitTest (point, nColumn, pItem) != NULL)
					{
						id = CBCGPGridItemID (pItemCurSel->m_pGridRow->GetRowId (), nColumn);
					}
				}
			}
				
			if (!id.IsNull () && pRowCurSel != NULL &&
				!(id.IsRow () && m_nHorzScrollOffset > 0)) // first scroll, then select thole row
			{
				if (id.IsRow () && (bShift && !m_bSingleSel))
				{
					// do not select thole row when "shift")
				}
				else
				{
					DWORD dwSelMode = SM_SINGLE_ITEM |
						((bShift && !m_bSingleSel) ? SM_CONTINUE_SEL_GROUP : SM_SINGE_SEL_GROUP);
					m_pSetSelItem = m_bVirtualMode ? NULL : pItem;

					SetCurSel (id, dwSelMode);

					m_pSetSelItem = NULL;

					EnsureVisible (pRowCurSel);
				}
			}
			else
			{
				OnHScroll (SB_TOP, 0, NULL);
			}
			return;
		}
		
		// else ==> act as VK_UP!

	case VK_UP:
		{
			if (pSel == NULL)
			{
				SetCurSel (CBCGPGridItemID (0, 0));
				OnVScroll (SB_TOP, 0, NULL);
				return;
			}

			// Select prev. item:
			const BOOL bShift = (::GetAsyncKeyState (VK_SHIFT) & 0x8000) != 0 && !m_bIgnoreShiftBtn;

			CBCGPGridItemID idCurSel = (bShift && !m_bSingleSel) ? m_idLastSel: m_idActive;
			CBCGPGridRow* pRowCurSel = (bShift && !m_bSingleSel) ? m_pLastSelRow : m_pSelRow;
			CBCGPGridItem* pItemCurSel = (bShift && !m_bSingleSel) ? m_pLastSelItem : m_pSelItem;
			if (m_bVirtualMode)
			{
				pRowCurSel = GetVirtualRow (idCurSel.m_nRow);
				pItemCurSel = (pRowCurSel != NULL) ? pRowCurSel->GetItem (idCurSel.m_nColumn) : NULL;
			}
			if (pRowCurSel != NULL)
			{
				CBCGPGridRow* pRowNewSel = pRowCurSel;
				BOOL bSkipNonSelectableRow = FALSE;
				int nMaxScroll = m_nRowHeight;
				do
				{
					ASSERT_VALID (pRowNewSel);
					CRect rect = pRowNewSel->GetRect ();
					if (pItemCurSel != NULL && pItemCurSel->GetMergedCells () != NULL)
					{
						rect.top = pItemCurSel->GetMergedRect ().top;
					}
					CPoint point (rect.right - 1, rect.top - 2);

					if (bSkipNonSelectableRow && m_nVertScrollOffset > 0 &&
						point.y < m_rectList.top && point.y >= m_rectList.top - nMaxScroll)
					{
						OnVScroll (SB_LINEUP, 0, NULL);
						continue;
					}

					pRowNewSel = HitTest (point, NULL, TRUE);

					bSkipNonSelectableRow = (pRowNewSel != NULL && !pRowNewSel->CanSelect ());

					if (bSkipNonSelectableRow)
					{
						pItemCurSel = pRowNewSel->GetItem (idCurSel.m_nColumn);
						nMaxScroll += m_nRowHeight;
					}
				}
				while (bSkipNonSelectableRow);

				if (pRowNewSel != NULL)
				{
					ASSERT_VALID (pRowNewSel);

					CBCGPGridItemID id (pRowNewSel->GetRowId (), idCurSel.m_nColumn);

					if (!id.IsNull ())
					{
						DWORD dwSelMode = SM_SINGLE_ITEM |
							((bShift && !m_bSingleSel) ? SM_CONTINUE_SEL_GROUP : SM_SINGE_SEL_GROUP);

						m_bNoUpdateWindow = TRUE; // prevent flickering
						SetCurSel (id, dwSelMode);
						m_bNoUpdateWindow = FALSE;

						EnsureVisible (pRowNewSel);
					}
				}
				else
				{
					OnVScroll (SB_TOP, 0, NULL);
				}
			}
			return;
		}

	case VK_RIGHT:
		if ((::GetAsyncKeyState (VK_CONTROL) & 0x8000) != 0 && !m_bIgnoreCtrlBtn)
		{
			return;
		}
		else if (pSel != NULL && pSel->IsGroup () && !pSel->IsExpanded () && 
			(!IsSortingMode () || (pSel->m_dwFlags & BCGP_GRID_ITEM_AUTOGROUP) != 0))
		{
			pSel->Expand ();
			return;
		}
		else if (!m_bWholeRowSel)
		{
			const BOOL bShift = (::GetAsyncKeyState (VK_SHIFT) & 0x8000) != 0 && !m_bIgnoreShiftBtn;

			// Go right to the nearest item:
			CBCGPGridRow* pRowCurSel = (bShift && !m_bSingleSel) ? m_pLastSelRow : m_pSelRow;
			CBCGPGridItem* pItemCurSel = (bShift && !m_bSingleSel) ? m_pLastSelItem : m_pSelItem;
			if (m_bVirtualMode)
			{
				CBCGPGridItemID idCurSel = (bShift && !m_bSingleSel) ? m_idLastSel: m_idActive;
				pRowCurSel = GetVirtualRow (idCurSel.m_nRow);
				pItemCurSel = (pRowCurSel != NULL) ? pRowCurSel->GetItem (idCurSel.m_nColumn) : NULL;
			}

			CBCGPGridItemID id;
			CBCGPGridItem* pItem = NULL;

			if (pItemCurSel != NULL)
			{
				if (m_bVirtualMode)
				{
					id = pItemCurSel->GetGridItemID ();
					//id.m_nColumn ++;
					int nPos = GetColumnsInfo ().IndexToOrder (id.m_nColumn);

					// Get position order of the merged cell
					if (pItemCurSel->GetMergedCells () != NULL)
					{
						CBCGPGridRange range;
						if (pItemCurSel->GetMergedRange (range))
						{
							int nPosLeft = GetColumnsInfo ().IndexToOrder (range.m_nLeft);
							int nPosRight = GetColumnsInfo ().IndexToOrder (range.m_nRight);
							if (nPosLeft != -1 && nPosRight != -1)
							{
								nPos = max (nPosLeft, nPosRight);
							}
							else
							{
								nPos = -1;
							}
						}
					}

					if (nPos >= 0 && nPos < GetColumnsInfo ().GetColumnCount (TRUE) - 1)
					{
						id.m_nColumn = GetColumnsInfo ().OrderToIndex (++nPos);
					}
					else
					{
						id.m_nColumn = -1;
					}
				}
				else
				{
					CRect rect = pItemCurSel->GetRect ();
					if (pItemCurSel->GetMergedCells () != NULL)
					{
						rect.right = pItemCurSel->GetMergedRect ().right;
					}
					CPoint point (rect.right + 2, rect.bottom - 1);

					int nColumn = -1;
					if (pItemCurSel->m_pGridRow->HitTest (point, nColumn, pItem) != NULL)
					{
						id = CBCGPGridItemID (pItemCurSel->m_pGridRow->GetRowId (), nColumn);
					}

				}
			}
			else if (pRowCurSel != NULL)
			{
				id = CBCGPGridItemID (pRowCurSel->GetRowId (), GetColumnsInfo ().GetFirstVisibleColumn ());
			}

			if (!id.IsNull () && pRowCurSel != NULL &&
				!id.IsRow ()) // scroll, never select thole row
			{
				DWORD dwSelMode = SM_SINGLE_ITEM |
					((bShift && !m_bSingleSel) ? SM_CONTINUE_SEL_GROUP : SM_SINGE_SEL_GROUP);
				m_pSetSelItem = m_bVirtualMode ? NULL : pItem;

				SetCurSel (id, dwSelMode);

				m_pSetSelItem = NULL;

				EnsureVisible (pRowCurSel);
			}
			else
			{
				OnHScroll (SB_BOTTOM, 0, NULL);
			}
			return;
		}
		
		// else ==> act as VK_DOWN!

	case VK_DOWN:
		{
			if (pSel == NULL)
			{
				SetCurSel (CBCGPGridItemID (0, 0));
				OnVScroll (SB_TOP, 0, NULL);
				return;
			}

			if ((::GetAsyncKeyState (VK_MENU) & 0x8000) && nChar == VK_DOWN)
			{
				CBCGPGridItem* pItem = GetCurSelItem (pSel);
				if (pItem != NULL)
				{
					pItem->DoClickButton (CPoint (-1, -1));
				}

				return;
			}

			// Select next item:
			const BOOL bShift = (::GetAsyncKeyState (VK_SHIFT) & 0x8000) != 0 && !m_bIgnoreShiftBtn;

			CBCGPGridItemID idCurSel = (bShift && !m_bSingleSel) ? m_idLastSel: m_idActive;
			CBCGPGridRow* pRowCurSel = (bShift && !m_bSingleSel) ? m_pLastSelRow : m_pSelRow;
			CBCGPGridItem* pItemCurSel = (bShift && !m_bSingleSel) ? m_pLastSelItem : m_pSelItem;
			if (m_bVirtualMode)
			{
				pRowCurSel = GetVirtualRow (idCurSel.m_nRow);
				pItemCurSel = (pRowCurSel != NULL) ? pRowCurSel->GetItem (idCurSel.m_nColumn) : NULL;
			}
			if (pRowCurSel != NULL)
			{
				CBCGPGridRow* pRowNewSel = pRowCurSel;
				BOOL bSkipNonSelectableRow = FALSE;
				int nMaxScroll = m_nRowHeight;
				do 
				{
					ASSERT_VALID (pRowNewSel);
					CRect rect = pRowNewSel->GetRect ();
					if (pItemCurSel != NULL && pItemCurSel->GetMergedCells () != NULL)
					{
						rect.bottom = pItemCurSel->GetMergedRect ().bottom;
					}
					CPoint point (rect.right - 1, rect.bottom + 2);

					if (bSkipNonSelectableRow && m_nVertScrollOffset < m_nVertScrollTotal &&
						point.y > m_rectList.bottom && point.y <= m_rectList.bottom + nMaxScroll)
					{
						OnVScroll (SB_LINEDOWN, 0, NULL);
						continue;
					}

					pRowNewSel = HitTest (point, NULL, TRUE);

					bSkipNonSelectableRow = (pRowNewSel != NULL && !pRowNewSel->CanSelect ());

					if (bSkipNonSelectableRow)
					{
						pItemCurSel = pRowNewSel->GetItem (idCurSel.m_nColumn);
						nMaxScroll += m_nRowHeight;
					}
				} 
				while (bSkipNonSelectableRow);

				if (pRowNewSel != NULL)
				{
					ASSERT_VALID (pRowNewSel);

					CBCGPGridItemID id (pRowNewSel->GetRowId (), idCurSel.m_nColumn);

					if (!id.IsNull ())
					{
						DWORD dwSelMode = SM_SINGLE_ITEM |
							((bShift && !m_bSingleSel) ? SM_CONTINUE_SEL_GROUP : SM_SINGE_SEL_GROUP);

						m_bNoUpdateWindow = TRUE; // prevent flickering
						SetCurSel (id, dwSelMode);
						m_bNoUpdateWindow = FALSE;

						EnsureVisible (pRowNewSel);
					}
				}
				else
				{
					//OnVScroll (SB_BOTTOM, 0, NULL);
				}
			}
			return;
		}

	case VK_NEXT:
		{
			if (pSel == NULL)
			{
				SetCurSel (CBCGPGridItemID (0, 0));
				OnVScroll (SB_TOP, 0, NULL);
				return;
			}

			if (m_nVertScrollPage != 0)
			{
				const BOOL bShift = (::GetAsyncKeyState (VK_SHIFT) & 0x8000) != 0 && !m_bIgnoreShiftBtn;

				CBCGPGridItemID idCurSel = (bShift && !m_bSingleSel) ? m_idLastSel: m_idActive;
				CBCGPGridRow* pRowCurSel = (bShift && !m_bSingleSel) ? m_pLastSelRow : m_pSelRow;
				if (m_bVirtualMode)
				{
					pRowCurSel = GetVirtualRow (idCurSel.m_nRow);
				}

				if (pRowCurSel != NULL)
				{
					//OnVScroll (SB_PAGEDOWN, 0, NULL);

					// Get row id:
					ASSERT_VALID (pRowCurSel);
					CRect rect = pRowCurSel->GetRect ();
					CPoint point (rect.right - 1,
								  max (rect.top + m_nVertScrollPage - m_nRowHeight,
									   rect.bottom + 1));

					CBCGPGridRow* pRowNewSel = HitTest (point, NULL, TRUE);

					CBCGPGridItemID id;
					if (pRowNewSel != NULL)
					{
						ASSERT_VALID (pRowNewSel);
						id = CBCGPGridItemID (pRowNewSel->GetRowId (), idCurSel.m_nColumn);
					}
					else
					{
						//---------------------
						// Calculate page size:
						//---------------------
						int nFirst = -1, nLast = -1;
						int nPageItems = GetPageItems (nFirst, nLast, m_nFirstVisibleItem);
						if (nPageItems <= 0)
						{
							nPageItems = 1;
						}

						const int nLastRow = GetTotalItems () - 1;
						const int nNextRow = OffsetVisibleRow (pRowCurSel->GetRowId (), nPageItems, TRUE);
						id = CBCGPGridItemID (min (nNextRow, nLastRow), idCurSel.m_nColumn);
					}

					if (!id.IsNull ())
					{
						// Change selection:
						DWORD dwSelMode = SM_SINGLE_ITEM |
							((bShift && !m_bSingleSel) ? SM_CONTINUE_SEL_GROUP : SM_SINGE_SEL_GROUP);
						SetCurSel (id, dwSelMode);
						
						pRowNewSel = (bShift && !m_bSingleSel) ? m_pLastSelRow : m_pSelRow;
						if (m_bVirtualMode)
						{
							CBCGPGridItemID idNewSel = (bShift && !m_bSingleSel) ? m_idLastSel: m_idActive;
							pRowNewSel = GetVirtualRow (idNewSel.m_nRow);
						}

						// Scroll:
						if (pRowNewSel != NULL)
						{
							EnsureVisible (pRowNewSel);

							if (!pRowNewSel->CanSelect ())
							{
								SendMessage (WM_KEYDOWN, VK_UP);
							}
						}
						else
						{
							OnVScroll (SB_BOTTOM, 0, NULL);
						}
					}
				}
			}
			return;
		}

	case VK_END:
		{
			const BOOL bShift = (::GetAsyncKeyState (VK_SHIFT) & 0x8000) != 0 && !m_bIgnoreShiftBtn;

			const int nRowCount = GetTotalItems ();

			CBCGPGridItemID id;
			id.m_nRow = nRowCount - 1;
			id.m_nColumn = (bShift && !m_bSingleSel && !m_idLastSel.IsNull ()) ?
				m_idLastSel.m_nColumn :
				((!m_idActive.IsNull ()) ? m_idActive.m_nColumn : 0);

			CBCGPGridRow* pLastRow = GetRow (id.m_nRow);
			if (pLastRow != NULL)
			{
				ASSERT_VALID (pLastRow);
				if (!IsRowVisible (pLastRow))
				{
					id.m_nRow = OffsetVisibleRow (id.m_nRow, 1, FALSE);
				}
			}

			DWORD dwSelMode = SM_SINGLE_ITEM |
				((bShift && !m_bSingleSel) ? SM_CONTINUE_SEL_GROUP : SM_SINGE_SEL_GROUP);

			SetCurSel (id, dwSelMode);
			OnVScroll (SB_BOTTOM, 0, NULL);

			CBCGPGridRow* pRowNewSel = GetCurSel ();
			if (pRowNewSel != NULL && !pRowNewSel->CanSelect ())
			{
				SendMessage (WM_KEYDOWN, VK_UP);
			}
		}
		return;

	case VK_PRIOR:
		{
			if (pSel == NULL)
			{
				SetCurSel (CBCGPGridItemID (0, 0));
				OnVScroll (SB_TOP, 0, NULL);
				return;
			}

			if (m_nVertScrollPage != 0)
			{
				const BOOL bShift = (::GetAsyncKeyState (VK_SHIFT) & 0x8000) != 0 && !m_bIgnoreShiftBtn;

				CBCGPGridItemID idCurSel = (bShift && !m_bSingleSel) ? m_idLastSel: m_idActive;
				CBCGPGridRow* pRowCurSel = (bShift && !m_bSingleSel) ? m_pLastSelRow : m_pSelRow;
				if (m_bVirtualMode)
				{
					pRowCurSel = GetVirtualRow (idCurSel.m_nRow);
				}
				
				if (pRowCurSel != NULL)
				{
					//OnVScroll (SB_PAGEUP, 0, NULL);
					
					// Get row id:
					ASSERT_VALID (pRowCurSel);
					CRect rect = pRowCurSel->GetRect ();
					CPoint point (rect.right - 1,
								  min (rect.bottom - m_nVertScrollPage + m_nRowHeight,
									   rect.top - 1));

					CBCGPGridRow* pRowNewSel = HitTest (point, NULL, TRUE);

					CBCGPGridItemID id;
					if (pRowNewSel != NULL)
					{
						ASSERT_VALID (pRowNewSel);
						id = CBCGPGridItemID (pRowNewSel->GetRowId (), idCurSel.m_nColumn);
					}
					else
					{
						//---------------------
						// Calculate page size:
						//---------------------
						int nFirst = -1, nLast = -1;
						int nPageItems = GetPageItems (nFirst, nLast, m_nFirstVisibleItem);
						if (nPageItems <= 0)
						{
							nPageItems = 1;
						}

						const int nFirstRow = 0;
						const int nPrevRow = OffsetVisibleRow (pRowCurSel->GetRowId (), nPageItems, FALSE);
						id = CBCGPGridItemID (max (nPrevRow, nFirstRow), idCurSel.m_nColumn);
					}
					
					if (!id.IsNull ())
					{
						// Change selection:
						DWORD dwSelMode = SM_SINGLE_ITEM |
							((bShift && !m_bSingleSel) ? SM_CONTINUE_SEL_GROUP : SM_SINGE_SEL_GROUP);
						SetCurSel (id, dwSelMode);
			
						pRowNewSel = (bShift && !m_bSingleSel) ? m_pLastSelRow : m_pSelRow;
						if (m_bVirtualMode)
						{
							CBCGPGridItemID idNewSel = (bShift && !m_bSingleSel) ? m_idLastSel: m_idActive;
							pRowNewSel = GetVirtualRow (idNewSel.m_nRow);
						}

						// Scroll:
						if (pRowNewSel != NULL)
						{
							EnsureVisible (pRowNewSel);

							if (!pRowNewSel->CanSelect ())
							{
								SendMessage (WM_KEYDOWN, VK_DOWN);
							}
						}
						else
						{
							OnVScroll (SB_TOP, 0, NULL);
						}
					}
				}
			}
			return;
		}

	case VK_HOME:
		{
			const BOOL bShift = (::GetAsyncKeyState (VK_SHIFT) & 0x8000) != 0 && !m_bIgnoreShiftBtn;

			CBCGPGridItemID id;
			id.m_nRow = 0;
			id.m_nColumn = (bShift && !m_bSingleSel && !m_idLastSel.IsNull ()) ?
				m_idLastSel.m_nColumn :
				((!m_idActive.IsNull ()) ? m_idActive.m_nColumn : 0);

			CBCGPGridRow* pLastRow = GetRow (id.m_nRow);
			if (pLastRow != NULL)
			{
				ASSERT_VALID (pLastRow);
				if (!IsRowVisible (pLastRow))
				{
					id.m_nRow = OffsetVisibleRow (id.m_nRow, 1, TRUE);
				}
			}

			DWORD dwSelMode = SM_SINGLE_ITEM |
				((bShift && !m_bSingleSel) ? SM_CONTINUE_SEL_GROUP : SM_SINGE_SEL_GROUP);
			
			SetCurSel (id, dwSelMode);
			OnVScroll (SB_TOP, 0, NULL);

			CBCGPGridRow* pRowNewSel = GetCurSel ();
			if (pRowNewSel != NULL && !pRowNewSel->CanSelect ())
			{
				SendMessage (WM_KEYDOWN, VK_DOWN);
			}
		}

		return;

	case VK_ADD:
		if (pSel != NULL && pSel->IsGroup () && !pSel->IsExpanded () &&
			!pSel->IsInPlaceEditing () &&
			(!IsSortingMode () || (pSel->m_dwFlags & BCGP_GRID_ITEM_AUTOGROUP) != 0))
		{
			pSel->Expand ();
		}
		return;

	case VK_SUBTRACT:
		if (pSel != NULL && pSel->IsGroup () && pSel->IsExpanded () && 
			!pSel->IsInPlaceEditing () &&
			(!IsSortingMode () || (pSel->m_dwFlags & BCGP_GRID_ITEM_AUTOGROUP) != 0))
		{
			pSel->Expand (FALSE);
		}
		return;

	case VK_RETURN:
		if (pSel != NULL && pSel->IsGroup () && !pSel->IsInPlaceEditing () &&
			(!IsSortingMode () || (pSel->m_dwFlags & BCGP_GRID_ITEM_AUTOGROUP) != 0))
		{
			pSel->Expand (!pSel->IsExpanded ());
		}
		return;

	case VK_TAB:
		{
			const BOOL bShift = (::GetAsyncKeyState (VK_SHIFT) & 0x8000) != 0 && !m_bIgnoreShiftBtn;

			if (m_bFilterBar && m_nFocusedFilter != -1)
			{
				OnFilterBarTab (bShift);
			}
			else if (pSel != NULL && GetCurSelItem (pSel) == NULL)
			{
				GoToNextItem (!bShift ? NextRow : PrevRow );
			}
			else
			{
				GoToNextItem (!bShift ? 
					(NextColumn | Down): 
					(PrevColumn | Up) );
			}
		}
		return;
	}

	CBCGPWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}
//******************************************************************************************
BOOL CBCGPGridCtrl::IsRowVisible (CBCGPGridRow* pRow) const
{
	ASSERT_VALID (this);
	ASSERT_VALID (pRow);

	if (m_bVirtualMode)
	{
		return !pRow->IsItemFiltered ();
	}

	BOOL bShowAllItems = (IsSortingMode () && !IsGrouping ());
	return bShowAllItems ? !pRow->IsItemFiltered () : pRow->IsItemVisible ();
}
//******************************************************************************************
void CBCGPGridCtrl::EnsureVisible (CBCGPGridRow* pRowItem, BOOL bExpandParents/* = FALSE*/)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pRowItem);

	if (m_nRowHeight == 0)
	{
		return;
	}

	if (bExpandParents && (!IsSortingMode () || IsGrouping ()) && pRowItem->GetParent () != NULL)
	{
		CBCGPGridRow* pParent = pRowItem;
        
		while ((pParent = pParent->GetParent ()) != NULL)
		{
			ASSERT_VALID (pParent);
			pParent->Expand (TRUE);
		}
	}

	// ------------------
	// Scroll vertically:
	// ------------------
	CRect rectRow = pRowItem->m_Rect;

	if (rectRow.top < m_rectList.top - 1 || rectRow.bottom > m_rectList.bottom)
	{
		CBCGPGridItem* pItem = GetCurSelItem ();
		if (pItem != NULL)
		{
			CRect rectButton = pItem->m_rectButton;
			pItem->m_rectButton.SetRectEmpty ();
			RedrawWindow (rectButton);
		}

		int nNewVertOffset = (rectRow.bottom > m_rectList.bottom) ?
			// scroll down
			m_nVertScrollOffset + (rectRow.bottom - m_rectList.bottom) :
			// scroll up
			m_nVertScrollOffset + (rectRow.top - m_rectList.top);

		SetScrollPos (SB_VERT, nNewVertOffset, FALSE);
		OnVScroll (SB_THUMBPOSITION, nNewVertOffset, NULL);

		if (pItem != NULL && !pItem->m_rectButton.IsRectEmpty ())
		{
			pItem->AdjustButtonRect ();
			RedrawWindow (pItem->m_rectButton);
		}
	}

	// --------------------
	// Scroll horizontally:
	// --------------------
	CBCGPGridItem* pItem = (!m_bSingleSel && !m_pLastSelItem != NULL) ? m_pLastSelItem : m_pSelItem;
	if (m_bVirtualMode)
	{
		CBCGPGridItemID idItem = (!m_bSingleSel && !m_idLastSel.IsNull ()) ? m_idLastSel : m_idActive;
		CBCGPGridRow* pRow = GetVirtualRow (idItem.m_nRow);
		pItem = (pRow != NULL) ? pRow->GetItem (idItem.m_nColumn) : NULL;
	}

	if (pItem == NULL)
	{
		return;
	}

	ASSERT_VALID (pItem);

	CRect rectItem = pItem->m_Rect;

	int nLeftLimit = m_rectList.left;
	if (m_nHorzScrollOffset > 0 && GetColumnsInfo ().IsFreezeColumnsEnabled () &&
		!GetColumnsInfo ().IsColumnFrozen (pItem->GetColumnId ()))
	{
		nLeftLimit += GetColumnsInfo ().GetFreezeOffset ();
	}

	if (rectItem.left < nLeftLimit - 1 || rectItem.right > m_rectList.right)
	{
		int nNewHorzOffset = (rectItem.right > m_rectList.right) ?
			// scroll rigth
			m_nHorzScrollOffset + (rectItem.right - m_rectList.right) :
			// scroll left
			m_nHorzScrollOffset + (rectItem.left - nLeftLimit);

		SetScrollPos (SB_HORZ, nNewHorzOffset, FALSE);
		OnHScroll (SB_THUMBPOSITION, nNewHorzOffset, NULL);

		if (pItem != NULL && !pItem->m_rectButton.IsRectEmpty ())
		{
			pItem->AdjustButtonRect ();
			RedrawWindow (pItem->m_rectButton);
		}
	}
}
//******************************************************************************************
void CBCGPGridCtrl::EnsureVisibleColumn (int nColumn)
{
	ASSERT_VALID (this);

	// --------------------
	// Scroll horizontally:
	// --------------------
	CRect rectColumn;
	GetColumnsInfo ().GetColumnRect (nColumn, rectColumn);

	int nLeftLimit = m_rectList.left;
	if (m_nHorzScrollOffset > 0 && GetColumnsInfo ().IsFreezeColumnsEnabled () &&
		!GetColumnsInfo ().IsColumnFrozen (nColumn))
	{
		nLeftLimit += GetColumnsInfo ().GetFreezeOffset ();
	}
	
	if (rectColumn.left < nLeftLimit - 1 || rectColumn.right > m_rectList.right)
	{
		int nNewHorzOffset = (rectColumn.right > m_rectList.right) ?
			// scroll rigth
			m_nHorzScrollOffset + (rectColumn.right - m_rectList.right) :
			// scroll left
			m_nHorzScrollOffset + (rectColumn.left - nLeftLimit);
		
		SetScrollPos (SB_HORZ, nNewHorzOffset, FALSE);
		OnHScroll (SB_THUMBPOSITION, nNewHorzOffset, NULL);
	}
}
//******************************************************************************************
void CBCGPGridCtrl::ExpandAll (BOOL bExpand/* = TRUE*/)
{
	ASSERT_VALID (this);

	CWaitCursor wait;

	if (IsGrouping ())
	{
		for (POSITION pos = m_lstAutoGroups.GetHeadPosition (); pos != NULL; )
		{
			CBCGPGridRow* pItem = m_lstAutoGroups.GetNext (pos);
			ASSERT_VALID (pItem);

			pItem->ExpandDeep (bExpand);
		}
	}

	for (POSITION pos = m_lstItems.GetHeadPosition (); pos != NULL; )
	{
		CBCGPGridRow* pItem = m_lstItems.GetNext (pos);
		ASSERT_VALID (pItem);

		pItem->ExpandDeep (bExpand);
	}

	BOOL bRebuildTerminalItemsOld = m_bRebuildTerminalItems;
	m_bRebuildTerminalItems = FALSE;

	wait.Restore ();

	AdjustLayout ();

	m_bRebuildTerminalItems = bRebuildTerminalItemsOld;

	if (GetSafeHwnd () != NULL)
	{
		RedrawWindow ();
	}
}
//******************************************************************************************
void CBCGPGridCtrl::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	CBCGPWnd::OnChar(nChar, nRepCnt, nFlags);

	if (m_nDraggedColumn >= 0 || m_bTracking || m_bSelecting)
	{
		return;
	}

	if (!CanBeginInplaceEditOnChar (nChar, nRepCnt, nFlags))
	{
		return;
	}

	CBCGPGridRow* pSel = GetCurSel ();

	if (pSel == NULL || !pSel->m_bEnabled)
	{
		return;
	}

	ASSERT_VALID (pSel);
	EnsureVisible (pSel, TRUE);

	SetBeginEditReason (BeginEdit_Char);
	if (!EditItem (pSel))
	{
		return;
	}

	DoInplaceEditSetSel (OnInplaceEditSetSel (GetCurSelItem (pSel), BeginEdit_Char));

	pSel->PushChar (nChar);
}
//*******************************************************************************************
HBRUSH CBCGPGridCtrl::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CBCGPWnd::OnCtlColor(pDC, pWnd, nCtlColor);
	
	CBCGPGridItem* pItem = GetCurSelItem ();
	if (pItem != NULL && pItem->m_pWndInPlace != NULL &&
		pWnd->GetSafeHwnd () == pItem->m_pWndInPlace->GetSafeHwnd ())
	{
		HBRUSH hbrProp = pItem->OnCtlColor (pDC, nCtlColor);
		if (hbrProp != NULL)
		{
			return hbrProp;
		}
	}

	return hbr;
}

#ifndef _BCGPGRID_STANDALONE
#ifndef _BCGSUITE_

void CBCGPGridCtrl::UpdateColor (COLORREF color)
{
	ASSERT_VALID (this);

	CBCGPGridRow* pSel = GetCurSel ();
	ASSERT_VALID (pSel);

	CBCGPGridColorItem* pColorItem = DYNAMIC_DOWNCAST(CBCGPGridColorItem, GetCurSelItem (pSel));
	if (pColorItem == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	BOOL bChanged = color != pColorItem->GetColor ();
	pColorItem->SetColor (color);

	if (bChanged)
	{
		CBCGPGridItemID id = pColorItem->GetGridItemID ();
		pSel->OnItemChanged (pColorItem, id.m_nRow, id.m_nColumn);
		pColorItem->m_strLabel = pColorItem->FormatItem ();
	}

	if (pColorItem->m_pWndInPlace != NULL)
	{
		pColorItem->OnUpdateValue ();
	}
}
//****************************************************************************************
void CBCGPGridCtrl::CloseColorPopup ()
{
	ASSERT_VALID (this);

	CBCGPGridRow* pSel = GetCurSel ();
	ASSERT_VALID (pSel);

	CBCGPGridColorItem* pColorItem = DYNAMIC_DOWNCAST(CBCGPGridColorItem, GetCurSelItem (pSel));
	if (pColorItem == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	pColorItem->m_pPopup = NULL;

	pColorItem->m_bButtonIsDown = FALSE;
	pColorItem->Redraw ();

	if (pColorItem->m_pWndInPlace != NULL)
	{
		pColorItem->m_pWndInPlace->SetFocus ();
	}
}

#endif
#endif

void CBCGPGridCtrl::OnSelectCombo()
{
	ASSERT_VALID (this);
	
	CBCGPGridRow* pSel = GetCurSel ();

	if (pSel == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	ASSERT_VALID (pSel);
	pSel->OnSelectCombo ();
}
//****************************************************************************************
void CBCGPGridCtrl::OnCloseCombo()
{
	ASSERT_VALID (this);

	CBCGPGridRow* pSel = GetCurSel ();

	if (pSel == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	ASSERT_VALID (pSel);
	pSel->OnCloseCombo ();
}
//****************************************************************************************
void CBCGPGridCtrl::OnEditKillFocus()
{
	CBCGPGridRow* pSel = GetCurSel ();

	if (pSel != NULL && pSel->m_bInPlaceEdit && pSel->m_bEnabled)
	{
		ASSERT_VALID (pSel);

		if (!IsChild (GetFocus()) && pSel->OnEditKillFocus())
		{
			SetEndEditReason (EndEdit_AutoApply | EndEdit_KillFocus);
			if (!EndEditItem ())
			{
				CWnd* pWndInPlace = pSel->GetInPlaceWnd ();
				if (pWndInPlace != NULL)
				{
					pWndInPlace->SetFocus();
				}
			}
			else
			{
				OnKillFocus(GetFocus());
			}
		}
	}
}
//****************************************************************************************
void CBCGPGridCtrl::OnComboKillFocus()
{
	CBCGPGridRow* pSel = GetCurSel ();

	if (pSel != NULL && pSel->m_bEnabled)
	{
		ASSERT_VALID (pSel);

		CComboBox* pWndCombo = pSel->GetComboWnd ();
		if (pWndCombo != NULL)
		{
			ASSERT_VALID (pWndCombo);

			if (!IsChild (GetFocus()))
			{
				SetEndEditReason (EndEdit_AutoApply | EndEdit_KillFocus);
				if (!EndEditItem ())
				{
					pWndCombo->SetFocus();
				}
				else
				{
					OnKillFocus(GetFocus());
				}
			}
		}
	}
}
//****************************************************************************************
void CBCGPGridCtrl::SetBoolLabels (LPCTSTR lpszTrue, LPCTSTR lpszFalse)
{
	ASSERT_VALID (this);
	ASSERT (lpszTrue != NULL);
	ASSERT (lpszFalse != NULL);

	m_strTrue = lpszTrue;
	m_strFalse = lpszFalse;

	if (GetSafeHwnd () != NULL)
	{
		RedrawWindow ();
	}
}
//****************************************************************************************
void CBCGPGridCtrl::SetListDelimiter (TCHAR c)
{
	ASSERT_VALID (this);
	ASSERT (GetSafeHwnd () != NULL);	// Should be called before window create

	m_cListDelimeter = c;
}
//****************************************************************************************
void CBCGPGridCtrl::OnDestroy() 
{
	while (!m_lstSel.IsEmpty ())
	{
		delete m_lstSel.RemoveTail ();
	}

	CleanUpAutoGroups ();

	while (!m_lstItems.IsEmpty ())
	{
		delete m_lstItems.RemoveTail ();
	}

	m_lstTerminalItems.RemoveAll ();

	CleanUpFonts ();
	CleanUpColors ();

	if (m_pFindDlg != NULL)
	{
		CloseFindReplaceDlg ();

		((CBCGPGridFindDlg*) m_pFindDlg)->m_pParent = NULL;
	}

#ifndef _BCGPGRID_STANDALONE
	CBCGPTooltipManager::DeleteToolTip (m_pToolTip);
#else
	if (m_pToolTip != NULL)
	{
		if (m_pToolTip->GetSafeHwnd () != NULL)
		{
			m_pToolTip->DestroyWindow ();
		}
		
		delete m_pToolTip;
	}
#endif

	m_pSelRow = NULL;
	m_pSelItem = NULL;
	m_pLastSelRow = NULL;
	m_pLastSelItem = NULL;
	m_ToolTip.DestroyWindow ();

	CBCGPWnd::OnDestroy();
}
//****************************************************************************************
void CBCGPGridCtrl::OnMouseMove(UINT nFlags, CPoint point) 
{
	CBCGPWnd::OnMouseMove(nFlags, point);

	const int nXDelta = 5;
	const int nYDelta = 5;

	if (m_nDraggedColumn >= 0)
	{
		CPoint pt = point;
		ClientToScreen (&pt);
		DragColumn (pt);
	}
	else if (m_bTracking)
	{
		TrackHeader (point.x);
	}
	else if (m_bSelecting && !m_bSingleSel)
	{
		if (abs (point.x - m_ptClickOnce.x) > nXDelta ||
			abs (point.y - m_ptClickOnce.y) > nYDelta)
		{
			KillTimer (GRID_CLICKVALUE_TIMER_ID);
			m_bClickTimer = FALSE;
			m_ptClickOnce = CPoint (0, 0);

			SetCursor (::LoadCursor (NULL, IDC_ARROW));

			SelectItems (point);
		}
	}
	else if (m_bClickDrag)
	{
		if (abs (point.x - m_ptClickOnce.x) > nXDelta ||
			abs (point.y - m_ptClickOnce.y) > nYDelta)
		{
			if (!DragItems (point))
			{
				StopDragItems ();
			}

			m_bClickDrag = FALSE;
			m_ptClickOnce = CPoint (0, 0);
		}
	}

	if (!(m_nDraggedColumn >= 0 || m_bTracking || m_bSelecting))
	{
		//----------------------
		// Highlight header item
		//----------------------
		int nHighlightColumn = -1;
		int nHighlightColumnBtn = -1;

		CBCGPGridColumnsInfo::ClickArea clickAreaHeader;
		int nHitColumn = GetColumnsInfo ().HitTestColumn (point, FALSE, STRETCH_DELTA, &clickAreaHeader);
		if (nHitColumn != -1)
		{
			if (clickAreaHeader == CBCGPGridColumnsInfo::ClickHeader)
			{
				nHighlightColumn = nHitColumn;
			}
			else if (clickAreaHeader == CBCGPGridColumnsInfo::ClickHeaderButton)
			{
				nHighlightColumnBtn = nHitColumn;
			}
		}

		BOOL bChanged = (GetColumnsInfo ().GetHighlightColumn () != nHighlightColumn ||
			GetColumnsInfo ().GetHighlightColumnBtn () != nHighlightColumnBtn);
		if (bChanged)
		{
			GetColumnsInfo ().SetHighlightColumn (nHighlightColumn);
			GetColumnsInfo ().SetHighlightColumnBtn (nHighlightColumnBtn);

			if (m_pToolTip->GetSafeHwnd () != NULL)
			{
				m_pToolTip->Pop ();
			}
		}

		if (!m_bMouseTracked)
		{
			m_bMouseTracked = TRUE;
			
			TRACKMOUSEEVENT trackmouseevent;
			trackmouseevent.cbSize = sizeof(trackmouseevent);
			trackmouseevent.dwFlags = TME_LEAVE;
			trackmouseevent.hwndTrack = GetSafeHwnd();
			trackmouseevent.dwHoverTime = HOVER_DEFAULT;
			::BCGPTrackMouse (&trackmouseevent);
		}

		if (bChanged)
		{
			RedrawWindow ();
		}
	}
}
//*****************************************************************************************
LRESULT CBCGPGridCtrl::OnMouseLeave(WPARAM,LPARAM)
{
	m_bMouseTracked = FALSE;

	//---------------------
	// Header highlighting:
	//---------------------
	BOOL bRedraw = FALSE;
	if (GetColumnsInfo ().GetHighlightColumn () >= 0)
	{
		GetColumnsInfo ().SetHighlightColumn (-1);
		bRedraw = TRUE;
	}
	if (GetColumnsInfo ().GetHighlightColumnBtn () >= 0)
	{
		GetColumnsInfo ().SetHighlightColumnBtn (-1);
		bRedraw = TRUE;
	}

	if (bRedraw)
	{
		RedrawWindow ();
	}

	return 0;
}
//*****************************************************************************************
void CBCGPGridCtrl::OnLButtonUp(UINT nFlags, CPoint point) 
{
	CBCGPWnd::OnLButtonUp(nFlags, point);

	if (m_nDraggedColumn >= 0)
	{
		StopDragColumn (point, TRUE);
	}
	else if (m_bTracking)
	{
		CRect rectTrackHeader = m_rectTrackHeader;

		TrackHeader (-1);
		m_bTracking = FALSE;

		if (::GetCapture () == GetSafeHwnd ())
		{
			ReleaseCapture ();
		}

		// ------------------
		// rearrange columns:
		// ------------------
		int nLeftOffset = 0;
		int nCount = 0;	// count visible columns

		int nPos = GetColumnsInfo ().Begin ();
		while (nPos != GetColumnsInfo ().End ())
		{
			int i = GetColumnsInfo ().Next (nPos);
			if (i == -1)
			{
				break; // no more visible columns
			}

			if (i == m_nTrackColumn)
			{
				break;
			}

			int nWidth = GetColumnsInfo ().GetColumnWidth (i);
			if (nCount == 0)
			{
				nWidth += GetHierarchyOffset () + GetExtraHierarchyOffset ();
			}

			nLeftOffset += nWidth;
			nCount++;
		}

		int nNewOffset = GetColumnsInfo ().IsColumnFrozen (m_nTrackColumn) ? nLeftOffset : nLeftOffset - m_nHorzScrollOffset;
		int nNewWidth = (point.x - m_rectList.left) - nNewOffset;
		if (nCount == 0)
		{
			nNewWidth -= GetHierarchyOffset () + GetExtraHierarchyOffset ();
		}

		BOOL bUpdate = (rectTrackHeader.left == point.x);
		if (bUpdate)
		{
			nNewWidth = max (m_nBaseHeight, nNewWidth);
			SetHeaderItemWidth (m_nTrackColumn, nNewWidth);

			if (!m_bInAdjustLayout)
			{
				ReposItems ();

				OnPosSizeChanged ();
				m_rectTrackSel = OnGetSelectionRect ();

				RedrawWindow ();
			}
		}
	}
	else if (m_bSelecting)
	{
		StopSelectItems ();

		if (m_bClickTimer)
		{
			// "Click once" event
			KillTimer (GRID_CLICKVALUE_TIMER_ID);
			m_bClickTimer = FALSE;

			if (m_bHeaderRowSelecting)
			{
				m_bHeaderRowSelecting = FALSE;

				//------------------------------------
				// Translate a click to the row header:
				//------------------------------------
				CRect rectRowHeader;
				CBCGPGridRow* pHitHeaderRow = HitTestRowHeader (point, rectRowHeader);
				if (pHitHeaderRow != NULL)
				{
					OnRowHeaderClick (pHitHeaderRow, rectRowHeader);
				}

			}
			else if (m_bHeaderColSelecting)
			{
				m_bHeaderColSelecting = FALSE;

				//----------------------------------------
				// Translate a click to the column header:
				//----------------------------------------
				int nColumnHit = GetColumnsInfo ().HitTestColumn (point, FALSE, STRETCH_DELTA);
				if (nColumnHit >= 0)
				{
					OnHeaderColumnClick (nColumnHit);
				}

			}
			else
			{	
				//------------------------------
				// Translate a click to the item:
				//------------------------------
				CBCGPGridItem* pItem = GetCurSelItem ();
				if (pItem != NULL)
				{
					DoClickValue (pItem, WM_LBUTTONDOWN, m_ptClickOnce, m_bIsFirstClick, m_bIsButtonClick);
					if (!m_bIsButtonClick)
					{
						pItem->OnClickValue (WM_LBUTTONUP, point);
					}
				}
			}

			m_ptClickOnce = CPoint (0, 0);
		}
	}
	else if (m_bClickDrag)
	{
		// "Click once" event
		m_bClickDrag = FALSE;

		CRect rectRowHeader;
		CBCGPGridRow* pHitHeaderRow = HitTestRowHeader (point, rectRowHeader);
		if (pHitHeaderRow != NULL)
		{
			//------------------------------------
			// Translate a click to the row header:
			//------------------------------------
			OnRowHeaderClick (pHitHeaderRow, rectRowHeader);
			SelectRow (pHitHeaderRow->GetRowId ());
		}
		else
		{
			//------------------------------
			// Translate a click to the item:
			//------------------------------
			CBCGPGridItemID id;
			CBCGPGridItem* pHitItem = NULL;
			HitTest (point, id, pHitItem);
			
			m_bNoUpdateWindow = TRUE; // prevent flickering
			m_pSetSelItem = m_bVirtualMode ? NULL : pHitItem;
			
			SetCurSel (id, SM_FIRST_CLICK | SM_SINGE_SEL_GROUP);
			
			m_pSetSelItem = NULL;
			m_bNoUpdateWindow = FALSE;

			CBCGPGridItem* pItem = GetCurSelItem ();
			if (pItem != NULL && pItem->IsEnabled ())
			{
				DoClickValue (pItem, WM_LBUTTONDOWN, m_ptClickOnce, m_bIsFirstClick, m_bIsButtonClick);
				if (!m_bIsButtonClick)
				{
					pItem->OnClickValue (WM_LBUTTONUP, point);
				}
			}
		}

		StopDragItems ();
	}
}
//****************************************************************************************
void CBCGPGridCtrl::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp) 
{
	CBCGPWnd::OnNcCalcSize(bCalcValidRects, lpncsp);

	if (GetStyle() & WS_BORDER) 
	{
		lpncsp->rgrc[0].left++; 
		lpncsp->rgrc[0].top++ ;
		lpncsp->rgrc[0].right--;
		lpncsp->rgrc[0].bottom--;
	}
}
//****************************************************************************************
void CBCGPGridCtrl::OnNcPaint() 
{
	if (GetStyle () & WS_BORDER)
	{
		visualManager->OnDrawControlBorder (this);
	}
}
//********************************************************************************
void CBCGPGridCtrl::SetShowInPlaceToolTip (BOOL bShow)
{
	m_bShowInPlaceToolTip = bShow;

	if (!bShow)
	{
		m_ToolTip.Deactivate ();
	}
}
//********************************************************************************
void CBCGPGridCtrl::SetVisualManagerColorTheme(BOOL bSet, BOOL bRedraw)
{
	CBCGPGridColors theme;

	if (bSet)
	{
		CBCGPVisualManager::GetInstance ()->OnSetGridColorTheme(this, theme);
	}

	m_bVisualManagerStyle = bSet;

	SetColorTheme (theme, bRedraw);
}
//********************************************************************************
void CBCGPGridCtrl::SetColorTheme (const CBCGPGridColors& theme, BOOL bRedraw)
{
	m_ColorData = theme;

	InitColors ();

	if (IsVisualManagerStyle())
	{
		m_ColorData.m_bSparklineDefaultSelColor = CBCGPVisualManager::GetInstance()->IsGridSparklineDefaultSelColor();
	}
	else
	{
		m_ColorData.m_bSparklineDefaultSelColor = FALSE;
	}

	OnColorThemeChanged ();

	if (bRedraw && GetSafeHwnd () != NULL)
	{
		RedrawWindow ();
	}
}
//********************************************************************************
void CBCGPGridCtrl::OnColorThemeChanged ()
{
	for (int nRow = 0; nRow < GetRowCount(); nRow++)
	{
		CBCGPGridRow* pRow = GetRow(nRow);
		if (pRow != NULL)
		{
			for (int nColumn = 0; nColumn < pRow->GetItemCount(); nColumn++)
			{
				CBCGPGridItem* pCell = pRow->GetItem(nColumn);
				if (pCell != NULL)
				{
					pCell->OnAfterChangeGridColors();
				}
			}
		}
	}
}
//********************************************************************************
void CBCGPGridCtrl::SetCustomColors (
		COLORREF	clrBackground,
		COLORREF	clrText,
		COLORREF	clrGroupBackground,
		COLORREF	clrGroupText,
		COLORREF	clrLeftOffset,
		COLORREF	clrLine)
{
	CBCGPGridColors theme;
	theme.m_clrBackground = clrBackground;
	theme.m_clrText = clrText;
	theme.m_GroupColors.m_clrBackground = clrGroupBackground;
	theme.m_GroupColors.m_clrText = clrGroupText;
	theme.m_LeftOffsetColors.m_clrBackground = clrLeftOffset;
	theme.m_clrHorzLine = clrLine;
	theme.m_clrVertLine = clrLine;

	SetColorTheme (theme);
}
//********************************************************************************
void CBCGPGridCtrl::GetCustomColors (
		COLORREF&	clrBackground,
		COLORREF&	clrText,
		COLORREF&	clrGroupBackground,
		COLORREF&	clrGroupText,
		COLORREF&	clrLeftOffset,
		COLORREF&	clrLine)
{
	CBCGPGridColors theme = GetColorTheme ();

	clrBackground = theme.m_clrBackground;
	clrText = theme.m_clrText;
	clrGroupBackground = theme.m_GroupColors.m_clrBackground;
	clrGroupText = theme.m_GroupColors.m_clrText;
	clrLeftOffset = theme.m_LeftOffsetColors.m_clrBackground;
	clrLine = theme.m_clrHorzLine;
}
//********************************************************************************
void CBCGPGridCtrl::SetPreviewTextColor (COLORREF clr)
{
	m_ColorData.m_clrPreviewText = clr;
}
//********************************************************************************
COLORREF CBCGPGridCtrl::GetPreviewTextColor (BOOL bSelected) const
{
	if (bSelected)
	{
		return (COLORREF)-1; // color is not set, use default
	}

	return m_ColorData.m_clrPreviewText;
}
//********************************************************************************
void CBCGPGridCtrl::SetDataBarColors(COLORREF clrBorder, COLORREF clrFill, COLORREF clrGradient, int nGradientAngle)
{
	m_ColorData.m_DataBarColors.m_clrBorder = clrBorder;
	m_ColorData.m_DataBarColors.m_clrBackground = clrFill;
	m_ColorData.m_DataBarColors.m_clrGradient = clrGradient;
	m_ColorData.m_DataBarColors.m_nGradientAngle = nGradientAngle;
}
//********************************************************************************
HFONT CBCGPGridCtrl::SetBoldFont (CDC* pDC)
{
	ASSERT_VALID (pDC);
	
	if (pDC->IsPrinting ())
	{
		if (m_hPrinterBoldFont != NULL)
		{
			return (HFONT) ::SelectObject (pDC->GetSafeHdc (), m_hPrinterBoldFont);
		}
		return SetCurrFont (pDC);
	}
	
	return (HFONT) ::SelectObject (pDC->GetSafeHdc (), m_fontBold.GetSafeHandle());
}
//********************************************************************************
BOOL CBCGPGridCtrl::ProcessClipboardAccelerators (UINT nChar)
{
	CBCGPGridItem* pSelItem = GetCurSelItem ();
	if (pSelItem == NULL || pSelItem->m_pWndInPlace == NULL ||
		pSelItem->m_pWndInPlace->GetSafeHwnd () == NULL)
	{
		return FALSE;
	}

	ASSERT_VALID (pSelItem);

	BOOL bIsCtrl = (::GetAsyncKeyState (VK_CONTROL) & 0x8000) && !m_bIgnoreCtrlBtn;
	BOOL bIsShift = (::GetAsyncKeyState (VK_SHIFT) & 0x8000) && !m_bIgnoreShiftBtn;

	if (bIsCtrl && (nChar == _T('C') || nChar == VK_INSERT))
	{
		pSelItem->m_pWndInPlace->SendMessage (WM_COPY);
		return TRUE;
	}

	if (bIsCtrl && nChar == _T('V') || (bIsShift && nChar == VK_INSERT))
	{
		pSelItem->m_pWndInPlace->SendMessage (WM_PASTE);
		return TRUE;
	}

	if (bIsCtrl && nChar == _T('X') || (bIsShift && nChar == VK_DELETE))
	{
		pSelItem->m_pWndInPlace->SendMessage (WM_CUT);
		return TRUE;
	}

	if (bIsCtrl && nChar == _T('Z'))
	{
		pSelItem->m_pWndInPlace->SendMessage (WM_UNDO);
		return TRUE;
	}

	if (bIsCtrl && nChar == _T('A'))
	{
		pSelItem->m_pWndInPlace->SendMessage (EM_SETSEL, (WPARAM)0, (LPARAM)-1); // select all
		return TRUE;
	}

	if (nChar == VK_DELETE)
	{
		pSelItem->m_pWndInPlace->SendMessage (WM_KEYDOWN, VK_DELETE);
		return TRUE;
	}

	return FALSE;
}
//*******************************************************************************************
int CBCGPGridCtrl::InsertColumn (int nPos, LPCTSTR lpszColumn, int nWidth, int iImage, BOOL bHideNameWithImage)
{
	ASSERT_VALID (this);
	ASSERT (lpszColumn != NULL);
	ASSERT (nWidth >= 0);

	if (nWidth < OnGetColumnMinWidth (nPos) && nWidth > 0)
	{
		// override CBCGPGridCtrl::OnGetColumnMinWidth to allow less width
		nWidth = OnGetColumnMinWidth (nPos);
	}

	SetRebuildTerminalItems ();
	return GetColumnsInfo ().InsertColumn (nPos, lpszColumn, nWidth, iImage, bHideNameWithImage);
}
//*******************************************************************************************
BOOL CBCGPGridCtrl::DeleteColumn (int nPos)
{
	ASSERT_VALID (this);

	SetRebuildTerminalItems ();
	return GetColumnsInfo ().DeleteColumn (nPos);
}
//*******************************************************************************************
void CBCGPGridCtrl::DeleteAllColumns ()
{
	ASSERT_VALID (this);

	SetRebuildTerminalItems ();
	GetColumnsInfo ().DeleteAllColumns ();
}
//*******************************************************************************************
int CBCGPGridCtrl::GetColumnCount() const
{
	return GetColumnsInfo ().GetColumnCount ();
}
//*******************************************************************************************
int CBCGPGridCtrl::GetColumnWidth (int nCol) const
{
	return GetColumnsInfo ().GetColumnWidth (nCol);
}
//*******************************************************************************************
BOOL CBCGPGridCtrl::SetColumnWidth (int nCol, int nWidth)
{
	ASSERT_VALID (this);
	ASSERT (nCol >= 0);
	ASSERT (nWidth >= 0);

	if (nWidth < OnGetColumnMinWidth (nCol) && nWidth > 0)
	{
		ASSERT (FALSE); // override CBCGPGridCtrl::OnGetColumnMinWidth to allow less width
		nWidth = OnGetColumnMinWidth (nCol);
	}

	return GetColumnsInfo ().SetColumnWidth (nCol, nWidth);
}
//*******************************************************************************************
CString CBCGPGridCtrl::GetColumnName (int nCol) const
{
	return GetColumnsInfo ().GetColumnName (nCol);
}
//*******************************************************************************************
BOOL CBCGPGridCtrl::SetColumnName (int nCol, LPCTSTR lpszColumn)
{
	ASSERT_VALID (this);
	ASSERT (nCol >= 0);
	ASSERT (lpszColumn != NULL);

	return GetColumnsInfo ().SetColumnName (nCol, lpszColumn);
}
//*******************************************************************************************
BOOL CBCGPGridCtrl::GetColumnTextHidden (int nColumn) const
{
	return GetColumnsInfo ().GetColumnTextHidden (nColumn);
}
//*******************************************************************************************
BOOL CBCGPGridCtrl::SetColumnTextHidden (int nColumn, BOOL bHideTextInHeader)
{
	ASSERT_VALID (this);
	ASSERT (nColumn >= 0);
	
	return GetColumnsInfo ().SetColumnTextHidden (nColumn, bHideTextInHeader);
}
//*******************************************************************************************
int CBCGPGridCtrl::GetColumnAlign (int nCol) const
{
	return GetColumnsInfo ().GetColumnAlign (nCol);
}
//*******************************************************************************************
BOOL CBCGPGridCtrl::SetColumnAlign (int nCol, int nAlign)
{
	ASSERT_VALID (this);
	ASSERT (nCol >= 0);

	return GetColumnsInfo ().SetColumnAlign (nCol, nAlign);
}
//*******************************************************************************************
int CBCGPGridCtrl::GetHeaderAlign (int nCol) const
{
	return GetColumnsInfo ().GetHeaderAlign (nCol);
}
//*******************************************************************************************
BOOL CBCGPGridCtrl::SetHeaderAlign (int nCol, int nAlign)
{
	ASSERT_VALID (this);
	ASSERT (nCol >= 0);

	return GetColumnsInfo ().SetHeaderAlign (nCol, nAlign);
}
//*******************************************************************************************
BOOL CBCGPGridCtrl::GetColumnLocked (int nCol) const
{
	ASSERT_VALID (this);
	ASSERT (nCol >= 0);

	return GetColumnsInfo ().GetColumnLocked (nCol);
}
//*******************************************************************************************
BOOL CBCGPGridCtrl::SetColumnLocked (int nCol, BOOL bLockedSize)
{
	ASSERT_VALID (this);
	ASSERT (nCol >= 0);

	return GetColumnsInfo ().SetColumnLocked (nCol, bLockedSize);
}
//*******************************************************************************************
BOOL CBCGPGridCtrl::GetColumnVisible (int nColumn) const
{
	ASSERT_VALID (this);
	ASSERT (nColumn >= 0);

	return GetColumnsInfo ().GetColumnVisible (nColumn);
}
//*******************************************************************************************
BOOL CBCGPGridCtrl::SetColumnVisible (int nColumn, BOOL bVisible)
{
	ASSERT_VALID (this);
	ASSERT (nColumn >= 0);

	return GetColumnsInfo ().SetColumnVisible (nColumn, bVisible);
}
//*******************************************************************************************
BOOL CBCGPGridCtrl::IsTextColumn (int nColumn) const
{
	ASSERT_VALID (this);
	ASSERT (nColumn >= 0);

	return GetColumnsInfo ().IsTextColumn (nColumn);
}
//*******************************************************************************************
BOOL CBCGPGridCtrl::SetTextColumn (int nColumn, BOOL bText)
{
	ASSERT_VALID (this);
	ASSERT (nColumn >= 0);

	return GetColumnsInfo ().SetTextColumn (nColumn, bText);
}
//*******************************************************************************************
int CBCGPGridCtrl::GetColumnOrderArray (LPINT piArray, int iCount) const
{
	return GetColumnsInfo ().GetColumnOrderArray (piArray, iCount);
}
//*******************************************************************************************
BOOL CBCGPGridCtrl::SetColumnOrderArray (int iCount, LPINT piArray)
{
	return GetColumnsInfo ().SetColumnOrderArray (iCount, piArray);
}
//*******************************************************************************************
void CBCGPGridCtrl::EnableColumnAutoSize (BOOL bEnable)
{
	GetColumnsInfo ().EnableAutoSize (bEnable);
	
	AdjustLayout ();
}
//*******************************************************************************************
void CBCGPGridCtrl::ShowColumnsChooser (BOOL bShow/* = TRUE*/, BOOL bVisualManagerStyle/* = FALSE*/)
{
	if (!bShow)
	{
		if (m_pColumnChooser != NULL &&
			m_pColumnChooser->GetSafeHwnd () != NULL)
		{
			m_pColumnChooser->ShowWindow (SW_HIDE);
		}

		m_bColumnChooserVisible = FALSE;
		return;
	}

	if (m_pColumnChooser == NULL)
	{
		m_pColumnChooser = new CBCGPGridColumnChooser (GetColumnsInfo ());

		if (m_rectColumnChooser.IsRectEmpty ())
		{
			CRect rectWindow;
			GetWindowRect (rectWindow);

			CSize sizeChooser (100, 100);

			m_rectColumnChooser =
				CRect (CPoint (
					rectWindow.right - sizeChooser.cx,
					rectWindow.top + 50),
				sizeChooser);
		}

		m_pColumnChooser->Create (this, m_rectColumnChooser, this);
	}

	if (bShow && m_pColumnChooser->IsVisualManagerStyle() != bVisualManagerStyle)
	{
		m_bFieldChooserThemed = bVisualManagerStyle;
		m_pColumnChooser->EnableVisualManagerStyle(bVisualManagerStyle);

		if (m_pColumnChooser->IsWindowVisible())
		{
			m_pColumnChooser->RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_FRAME | RDW_ALLCHILDREN);
		}
	}

	m_pColumnChooser->ShowWindow (SW_SHOWNOACTIVATE);
	m_bColumnChooserVisible = TRUE;
}
//*******************************************************************************************
BOOL CBCGPGridCtrl::IsColumnsChooserVisible () const
{
	return m_pColumnChooser->GetSafeHwnd () != NULL &&
		m_pColumnChooser->IsWindowVisible ();
}
//*******************************************************************************************
void CBCGPGridCtrl::SetFieldChooserEmptyContentLabel(const CString& strFieldChooserEmptyContentLabel)
{
	m_strFieldChooserEmptyContentLabel = strFieldChooserEmptyContentLabel;

	if (IsColumnsChooserVisible() && m_pColumnChooser->m_bIsEmpty)
	{
		m_pColumnChooser->RedrawWindow();
	}
}
//*******************************************************************************************
BOOL CBCGPGridCtrl::IsValidID (const CBCGPGridItemID &id) const
{
	return (id.m_nColumn >= 0 && id.m_nColumn < GetColumnsInfo ().GetColumnCount () &&
			id.m_nRow >= 0 && id.m_nRow < GetTotalItems ());
}
//*******************************************************************************************
BOOL CBCGPGridCtrl::IsValidRange (const CBCGPGridRange &range) const
{
	return (range.IsValid () &&
			range.m_nLeft >= 0 && range.m_nRight < GetColumnsInfo ().GetColumnCount () &&
			range.m_nTop >= 0 && range.m_nBottom < GetTotalItems ());
}
//*******************************************************************************************
BOOL CBCGPGridCtrl::IsItemInRange (const CBCGPGridRange &rangeIndex, const CBCGPGridItemID &idItem) const
{
	if (idItem.IsNull ())
	{
		return FALSE;
	}

	if (idItem.IsRow ())
	{
		int nFirstColumn = GetColumnsInfo ().GetFirstVisibleColumn ();
		int nLastColumn = GetColumnsInfo ().GetLastVisibleColumn ();
			
		return (rangeIndex.IsInRange (idItem.m_nRow, nFirstColumn) &&
				rangeIndex.IsInRange (idItem.m_nRow, nLastColumn));
	}

	else
	{
		if (idItem.m_nColumn < 0 || idItem.m_nColumn >= GetColumnsInfo ().GetColumnCount ())
		{
			// column not found
			return FALSE;
		}

		int nPosColumn = GetColumnsInfo ().IndexToOrder (idItem.m_nColumn);
		if (nPosColumn == -1)
		{
			// hidden column
			return FALSE;
		}

		CBCGPGridItemID idPos (idItem.m_nRow, nPosColumn);

		CBCGPGridRange rangeOrder = rangeIndex;
		if (IndexToOrder (rangeOrder) && rangeOrder.IsInRange (idPos))
		{
			return TRUE;
		}
	}

	return FALSE;
}
//*******************************************************************************************
BOOL CBCGPGridCtrl::IndexToOrder (CBCGPGridRange &rangeIndex) const
{
	const int nColumnCount = GetColumnsInfo ().GetColumnCount ();

	if (rangeIndex.m_nLeft >= 0 && rangeIndex.m_nLeft < nColumnCount && 
		rangeIndex.m_nRight >= 0 && rangeIndex.m_nRight < nColumnCount)
	{
		int nPosLeft = GetColumnsInfo ().IndexToOrder (rangeIndex.m_nLeft);
		int nPosRight = GetColumnsInfo ().IndexToOrder (rangeIndex.m_nRight);

		if (nPosLeft != -1 && nPosRight != -1)
		{
			rangeIndex.Set (min (nPosLeft, nPosRight), rangeIndex.m_nTop, 
							max (nPosLeft, nPosRight), rangeIndex.m_nBottom);
			return TRUE;
		}
	}

	return FALSE;
}
//*******************************************************************************************
void CBCGPGridCtrl::SelectRange (const CBCGPGridRange &range, BOOL bSelect, BOOL bRedraw)
{
	ASSERT_VALID (this);

	OnSelChanging (range, bSelect);

	if (range.m_nLeft < 0 || range.m_nTop < 0 ||
		range.m_nRight < 0 || range.m_nBottom < 0)
	{
		OnSelChanged (range, bSelect);
		return;
	}

	CBCGPGridRange rangeNormalized (
			min (range.m_nLeft, range.m_nRight),
			min (range.m_nBottom, range.m_nTop),
			max (range.m_nLeft, range.m_nRight),
			max (range.m_nBottom, range.m_nTop));

	if (m_bVirtualMode)
	{
		// iterate through cached items
		for (POSITION pos = m_CachedItems.m_lstCache.GetHeadPosition (); pos != NULL; )
		{
			CBCGPGridCachePageInfo& cpi = m_CachedItems.m_lstCache.GetNext (pos);
			ASSERT (cpi.pArrCachePage->GetSize() == cpi.nSize);

			int nOffset = max (0, rangeNormalized.m_nTop - cpi.nFirst);
			while (nOffset >= 0 && nOffset < cpi.nSize && 
				nOffset <= rangeNormalized.m_nBottom - cpi.nFirst)
			{
				CBCGPGridRow* pRow = cpi.pArrCachePage->GetAt (nOffset);
				if (pRow != NULL)
				{
					ASSERT_VALID (pRow);
					DoSelectRowInRange (pRow, rangeNormalized, bSelect, bRedraw);
				}

				nOffset++;
			}
		}
	}
	else
	//if (IsValidRange (range))
	{
		const CList<CBCGPGridRow*, CBCGPGridRow*>& lst =
			(!IsSortingMode () && !IsGrouping ()) ? m_lstItems : m_lstTerminalItems;

		POSITION pos = lst.FindIndex (rangeNormalized.m_nTop);
		for (int i = rangeNormalized.m_nTop; pos != NULL && i <= rangeNormalized.m_nBottom; i++)
		{
			CBCGPGridRow* pRow = lst.GetNext (pos);
			ASSERT_VALID (pRow);

			DoSelectRowInRange (pRow, rangeNormalized, bSelect, bRedraw);
		}
	}

	OnSelChanged (range, bSelect);
}
//*******************************************************************************************
void CBCGPGridCtrl::DoSelectRowInRange (CBCGPGridRow* pRow, const CBCGPGridRange &range, 
										BOOL bSelect, BOOL bRedraw)
{
	ASSERT_VALID (pRow);

	BOOL bInRange = FALSE;

	int nPos = GetColumnsInfo ().Begin ();
	while (nPos != GetColumnsInfo ().End ())
	{
		int iColumn = GetColumnsInfo ().Next (nPos);
		if (iColumn == -1)
		{
			break; // no more visible columns
		}

		BOOL bIsRangeBound = (iColumn == range.m_nLeft || iColumn == range.m_nRight);

		if (bIsRangeBound || bInRange)
		{
			CBCGPGridItem* pItem = pRow->GetItem (iColumn);

			if (pItem != NULL)
			{
				ASSERT_VALID (pItem);
				CRect rectButton = pItem->m_rectButton;
				pItem->Select (bSelect);

				if (bRedraw)
				{
					CRect rect = pItem->m_Rect;
					rect.InflateRect (1, 1);
					InvalidateRect (rect);
					InvalidateRect (rectButton);
				}
			}
		}

		if (bIsRangeBound)
		{
			if (bInRange || range.m_nLeft == range.m_nRight)
			{
				break;	// last visible column in range
			}

			bInRange = TRUE;
		}
	}

	if (!bSelect || pRow->CanSelect ())
	{
		pRow->Select (bSelect);
	}

	if (bRedraw)
	{
		CRect rect = pRow->m_Rect;
		if (pRow->IsGroup ())
		{
			rect.InflateRect (1, 1);
		}
		InvalidateRect (rect);
	}
	if (m_bRowMarker)
	{
		if (IsRowMarkerOnRowHeader ())
		{
			// update row marker:
			CRect rect = pRow->m_Rect;
			rect.left = m_rectRowHeader.left;
			rect.right = m_rectRowHeader.right;
			InvalidateRect (rect);
		}
		else if (!pRow->IsGroup () && GetExtraHierarchyOffset () > 0)
		{
			// update row marker:
			int dx = IsSortingMode () && !IsGrouping () ? 0 : pRow->GetHierarchyLevel () * GetHierarchyLevelOffset ();
			CRect rect = pRow->m_Rect;
			if (m_nHorzScrollOffset > 0 && GetColumnsInfo ().IsFreezeColumnsEnabled ())
			{
				rect.right = m_rectList.left + GetExtraHierarchyOffset () + dx;
			}
			else
			{
				rect.right = rect.left + GetExtraHierarchyOffset () + dx;
			}
			InvalidateRect (rect);
		}
	}
}
//*******************************************************************************************
void CBCGPGridCtrl::IncludeRect (CRect& rect, const CRect& rectNew)
{
	if (rectNew.IsRectEmpty ())
	{
		return;
	}

	if (rect.IsRectEmpty ())
	{
		rect = rectNew;
		rect.NormalizeRect ();
	}
	else
	{
		CRect rect2 = rectNew;
		rect2.NormalizeRect ();
		rect.UnionRect (rect, rect2);
	}
}
//*******************************************************************************************
void CBCGPGridCtrl::DoInvalidateRowInRange (CBCGPGridRow* pRow, const CBCGPGridRange &range,
											CRect& rectUpdate, BOOL bRangeOnly) const
{
	if (pRow != NULL)
	{
		ASSERT_VALID (pRow);

		if (!pRow->IsGroup () || bRangeOnly)
		{
			CBCGPGridItem* pItemLeft = pRow->GetItem (range.m_nLeft);
			if (pItemLeft != NULL)
			{
				IncludeRect (rectUpdate, pItemLeft->GetRect ());
				IncludeRect (rectUpdate, pItemLeft->m_rectButton);
			}

			CBCGPGridItem* pItemRight = pRow->GetItem (range.m_nRight);
			if (pItemRight != NULL)
			{
				IncludeRect (rectUpdate, pItemRight->GetRect ());
				IncludeRect (rectUpdate, pItemRight->m_rectButton);
			}
		}
		
		if (bRangeOnly)
		{
			return;
		}

		if (pRow->IsGroup ())
		{
			// update the row:
			IncludeRect (rectUpdate, pRow->GetRect ());
		}
		else if (m_bRowMarker && GetExtraHierarchyOffset () > 0)
		{
			// update row marker:
			int dx = IsSortingMode () && !IsGrouping () ? 0 : pRow->GetHierarchyLevel () * GetHierarchyLevelOffset ();
			CRect rect = pRow->GetRect ();
			rect.right = rect.left + GetExtraHierarchyOffset () + dx;
			IncludeRect (rectUpdate, rect);
		}
	}
}
//*******************************************************************************************
CRect CBCGPGridCtrl::GetRect (const CBCGPGridRange &range, BOOL bUpdateRect)
{
	ASSERT_VALID (this);

	if (m_bVirtualMode)
	{
		CRect rectUpdate;
		rectUpdate.SetRectEmpty ();

		DoInvalidateRowInRange (GetVirtualRow (range.m_nTop), range, rectUpdate, !bUpdateRect);
		DoInvalidateRowInRange (GetVirtualRow (range.m_nBottom), range, rectUpdate, !bUpdateRect);

		return rectUpdate;
	}
	else
	{
		const CList<CBCGPGridRow*, CBCGPGridRow*>& lst =
			(!IsSortingMode () && !IsGrouping ()) ? m_lstItems : m_lstTerminalItems;

		CRect rectUpdate;
		rectUpdate.SetRectEmpty ();

		POSITION pos = lst.FindIndex (range.m_nTop);
		if (pos != NULL)
		{
			DoInvalidateRowInRange (lst.GetAt (pos), range, rectUpdate, !bUpdateRect);
		}

		pos = lst.FindIndex (range.m_nBottom);
		if (pos != NULL)
		{
			DoInvalidateRowInRange (lst.GetAt (pos), range, rectUpdate, !bUpdateRect);
		}

		return rectUpdate;
	}
}
//*******************************************************************************************
void CBCGPGridCtrl::InvalidateRange (const CBCGPGridRange &range)
{
	ASSERT_VALID (this);

	CRect rectUpdate = GetRect (range);
	rectUpdate.InflateRect (1, 1);
	InvalidateRect (rectUpdate);
}
//*******************************************************************************************
CBCGPGridRow* CBCGPGridCtrl::GetCurSel (CBCGPGridItemID &id) const
{
	ASSERT_VALID (this);

	id = m_idActive;

	if (m_bVirtualMode)
	{
		return ((CBCGPGridCtrl*) this)->GetVirtualRow (id.m_nRow);
	}

	if (m_pSelItem != NULL && m_pSelItem->GetMergedCells () != NULL)
	{
		id = m_pSelItem->GetMergedCells ()->GetMainItemID ();
		CBCGPGridItem* pSelItem = m_pSelItem->GetMergedMainItem ();
		return (pSelItem != NULL) ? pSelItem->GetParentRow () : NULL;
	}

	if (m_pSelRow == NULL)
	{
		if (m_pSelItem != NULL)
		{
			return m_pSelItem->m_pGridRow;
		}

		return NULL;
	}

	return m_pSelRow;
}
//*******************************************************************************************
CBCGPGridItemID CBCGPGridCtrl::GetCurSelItemID () const
{
	return m_idActive;
}
//*******************************************************************************************
BOOL CBCGPGridCtrl::SetCurSel (CBCGPGridItemID idItem,
							   DWORD dwSelMode, BOOL bRedraw)
{
	ASSERT_VALID (this);

	CBCGPGridRow* pSel = GetCurSel ();
	CBCGPGridRow* pOldSelectedItem = pSel;

	if ( m_idLastSel == idItem &&
		 !(dwSelMode == SM_NONE) &&
		 !( ((SM_ALL | SM_COLUMN | SM_ROW | SM_SINGLE_ITEM | 
			  SM_FIRST_CLICK | SM_SECOND_CLICK | SM_INVERT_SEL) & dwSelMode) != 0) )
	{
		return FALSE;
	}

	if (pSel != NULL && pSel->m_bInPlaceEdit)
	{
		SetEndEditReason (EndEdit_Cancel | EndEdit_Selection);
		EndEditItem (FALSE);
	}

	BOOL	bExcelSel = TRUE;			// Excell behavior

	const int nFirstColumn = GetColumnsInfo ().GetFirstVisibleColumn ();
	const int nFirstRow = 0;
	const int nLastColumn = GetColumnsInfo ().GetLastVisibleColumn ();
	const int nLastRow = GetTotalItems () - 1;

	ASSERT (nLastColumn <= GetColumnsInfo ().GetColumnCount () - 1);

	if (nLastColumn < 0 || nLastRow < 0)
	{
		dwSelMode = SM_NONE;
	}

	// -----------------
	// Change selection:
	// -----------------
	if ((dwSelMode & SM_ALL) != 0 || idItem.IsAll ())
	{
		idItem.SetAll ();

		// Store previous active item, select all items
		if (m_idActive.IsNull ())
		{
			m_idActive = idItem;
		}
		m_idLastSel = idItem;

		while (!m_lstSel.IsEmpty ())
		{
			delete m_lstSel.RemoveTail ();
		}

		CBCGPGridRange* pSelRange = 
			new CBCGPGridRange (nFirstColumn, nFirstRow, nLastColumn, nLastRow);
		m_lstSel.AddTail (pSelRange);
	}
	else if ((SM_NONE == dwSelMode) ||
		idItem.IsNull () || 
		nLastColumn < 0 || nLastRow < 0)
	{
		// Remove selection
		m_idActive.SetNull ();
		m_idLastSel.SetNull ();

		while (!m_lstSel.IsEmpty ())
		{
			CBCGPGridRange* pRange = m_lstSel.RemoveTail ();
			ASSERT (pRange != NULL);

			SelectRange (*pRange, FALSE, bRedraw);
			delete pRange;
		}
	}
	else if ((dwSelMode & SM_SET_ACTIVE_ITEM) != 0)
	{
		// idItem should be within selection only
		// Store selection, change active item
		if (IsItemSelected (idItem))
		{
			m_idActive = idItem;
		}
	}

	else if ((dwSelMode & SM_SINGLE_ITEM) != 0 && !bExcelSel)
	{
	}
	else if ((dwSelMode & SM_FIRST_CLICK) != 0 ||
			(dwSelMode & SM_SINGLE_ITEM) != 0 ||
			(dwSelMode & SM_INVERT_SEL) != 0)
	{
		// Regarding nSelMode, modify selection or set new selection.
		// If not INVERT_SEL, the active item is always selected.

		if ((dwSelMode & SM_INVERT_SEL) != 0 && IsItemSelected (idItem))
		{
			DoInvertSelection (idItem, dwSelMode, bRedraw, 
							nFirstColumn, nFirstRow, nLastColumn, nLastRow);
		}
		else if ((dwSelMode & SM_SINGE_SEL_GROUP) != 0)
		{
			// Remove selection, set active item, add selected block
			while (!m_lstSel.IsEmpty ())
			{
				CBCGPGridRange* pRange = m_lstSel.RemoveTail ();
				ASSERT (pRange != NULL);

				SelectRange (*pRange, FALSE, bRedraw);
				delete pRange;
			}

            BOOL bAddSelGroup = TRUE;
            DoSetSelection (idItem, dwSelMode, bAddSelGroup, bRedraw, 
							nFirstColumn, nFirstRow, nLastColumn, nLastRow);
		}
		else if ((dwSelMode & SM_CONTINUE_SEL_GROUP) != 0)
		{
			// Store previous active item, modify selected block
            BOOL bAddSelGroup = FALSE;
            DoSetSelection (idItem, dwSelMode, bAddSelGroup, bRedraw,
							nFirstColumn, nFirstRow, nLastColumn, nLastRow);
		}
		else if ((dwSelMode & SM_ADD_SEL_GROUP) != 0)
		{
			// Store selection, set new active item, add new selected block
            BOOL bAddSelGroup = TRUE;
            DoSetSelection (idItem, dwSelMode, bAddSelGroup, bRedraw,
							nFirstColumn, nFirstRow, nLastColumn, nLastRow);
		}
		else
		{
			return FALSE;
		}
	}
	else if ((dwSelMode & SM_SECOND_CLICK) != 0)
	{
		// Store previous active item, modify selected block
        BOOL bAddSelGroup = FALSE;
        DoSetSelection (idItem, dwSelMode, bAddSelGroup, bRedraw,
						nFirstColumn, nFirstRow, nLastColumn, nLastRow);
	}
	else
	{
		return FALSE;
	}

	if (m_bWholeRowSel && !m_lstSel.IsEmpty ())
	{
		// Inflate last selected block to the whole row
		CBCGPGridRange* pRange = (CBCGPGridRange*) m_lstSel.GetTail ();
		ASSERT (pRange != NULL);

		pRange->m_nLeft = nFirstColumn;
		pRange->m_nRight = nLastColumn;
	}

	CBCGPGridItem* pPrevItem = GetCurSelItem ();
	if (m_bVirtualMode)
	{
		m_pSelRow = NULL;
		m_pSelItem = NULL;

		m_pLastSelRow = NULL;
		m_pLastSelItem = NULL;
	}
	else
	{
		m_pSelRow = (m_idActive.m_nRow != -1) ? GetRow (m_idActive.m_nRow) : NULL;
		m_pSelItem = (m_pSelRow != NULL) ? m_pSelRow->GetItem (m_idActive.m_nColumn) : NULL;
		
		if (m_idActive.IsAll ())
		{
			m_pSelRow = GetRow (0);
			m_pSelItem = (m_pSelRow != NULL && !IsWholeRowSel ()) ? m_pSelRow->GetItem (0) : NULL;
		}

		m_pLastSelRow = (m_idLastSel.m_nRow != -1) ? GetRow (m_idLastSel.m_nRow) : NULL;
		m_pLastSelItem = (m_pLastSelRow != NULL) ? m_pLastSelRow->GetItem (m_idLastSel.m_nColumn) : NULL;

		if (m_idLastSel.IsAll ())
		{
			m_pLastSelRow = GetRow (GetRowCount (TRUE) - 1);
			m_pLastSelItem = (m_pLastSelRow != NULL && !IsWholeRowSel ()) ? m_pLastSelRow->GetItem (GetColumnCount () - 1) : NULL;
		}
	}

	// Update RowMarker for the previously active row
	if (bRedraw && pOldSelectedItem != NULL && !pOldSelectedItem->IsSelected())
	{
		InvalidateRect (pOldSelectedItem->GetRect ());
	}

	CRect rectTrackSelOld = m_rectTrackSel;
	m_rectTrackSel = OnGetSelectionRect ();
	if (bRedraw)
	{
		InvalidateRect (rectTrackSelOld);
		InvalidateRect (m_rectTrackSel);
	}

	// -------------------------
	// Notify control and items:
	// -------------------------
	if (m_bWholeRowSel && m_bSingleSel)
	{
		CBCGPGridRow* pSel = GetCurSel ();
		OnChangeSelection (pSel, pOldSelectedItem);

		if (pOldSelectedItem != NULL)
		{
			ASSERT_VALID (pOldSelectedItem);

			pOldSelectedItem->OnKillSelection (pSel);

			if (bRedraw)
			{
				CRect rectOld = pOldSelectedItem->m_Rect;
				InvalidateRect (rectOld);
			}
		}

		if (pSel != NULL)
		{
			ASSERT_VALID (pSel);

			pSel->OnSetSelection (pOldSelectedItem);

			if (bRedraw)
			{
				CRect rect = pSel->m_Rect;
				InvalidateRect (rect);
			}
		}
	}

	for (POSITION pos = m_lstSel.GetHeadPosition (); pos != NULL; )
	{
		CBCGPGridRange* pRange = (CBCGPGridRange*) m_lstSel.GetNext (pos);
		ASSERT (pRange != NULL);

		SelectRange (*pRange, TRUE, bRedraw);
	}

	if (pPrevItem != NULL)
	{
		CRect rectButton = pPrevItem->m_rectButton;
		pPrevItem->m_rectButton.SetRectEmpty ();

		InvalidateRect (rectButton);
	}

	CBCGPGridItem* pCurItem = GetCurSelItem ();
	if (pCurItem != NULL && pCurItem->HasButton ())
	{
		pCurItem->AdjustButtonRect ();

		InvalidateRect (pCurItem->m_rectButton);
	}

	if (m_bRowHeader)
	{
		InvalidateRect (m_rectSelectAllArea);
	}
	
	// ---------------
	// Update control:
	// ---------------
	if (bRedraw && !m_bNoUpdateWindow)
	{
		UpdateWindow ();
	}

/*
	//ListCtrl behavior
		// arrows
		// Remove selection, set active item, add selected block
		// Ctrl + arrows
		// Change current item. Active item can be not selected.
		// Ctrl + Space
		// Invert selection for the active item.
		// Ctrl + Shift + arrows
		// Invert selection for range from active item to current item.

	//Excell behavior
	// change selection:
	switch (nSelMode)
	case SM_INVERT_SEL:		// non-Excell behavior
		// Add items to m_arrSel at first time.
		// If items are already selected then remove them from m_arrSel .

	case SM_SINGE_SEL_GROUP:// click, arrows
		// Remove selection, set active item, add selected block
	case SM_CONTINUE_SEL_GROUP:// Shift + click, Shift + arrows
		// Store previous active item, modify selected block
	case SM_ADD_SEL_GROUP:	// Ctrl + click
		// Store selection, set new active item, add new selected block

	case SM_SET_ACTIVE_ITEM:// Tab, Enter
		// Should be within selection only
		// Store selection, change active item

	case SM_NONE:
		// Remove selection
	case SM_ALL:			// Ctrl + A
		// Store previous active item, select all items
	case SM_SINGLE_ROW:		// Shift + Space
		// Store previous active item, inflate selected block to the whole row
	case SM_SINGLE_COLUMN:	// Ctrl + Space
		// Store previous active item, inflate selected block to the whole column

	case SM_SINGLE_ITEM:	// arrows
	case SM_FIRST_CLICK:
		// Regarding nSelMode, set active item or set new selection.
		// If not INVERT_SEL, the active item is always selected.
	case SM_SECOND_CLICK:
		// Store previous active item, modify selected block
		m_arrSel.Add (pHitItem);
*/
	if (GetSafeHwnd () != NULL)
	{
		CWnd* pOwnerWnd = GetOwner ();
		if (pOwnerWnd != NULL)
		{
			pOwnerWnd->SendMessage (BCGM_GRID_SEL_CHANGED, 0, (LPARAM) this);
		}

		CBCGPGridRow* pRow = GetCurSel ();
		NotifyAccessibility (pRow, GetCurSelItem (pRow));
	}
	return TRUE;
}
//*******************************************************************************************
BOOL CBCGPGridCtrl::DoSetSelection (const CBCGPGridItemID& idItem, const DWORD dwSelMode,
									BOOL bAddSelGroup, BOOL bRedraw,
									const int nFirstColumn, const int nFirstRow,
									const int nLastColumn, const int nLastRow)
{
	ASSERT_VALID (this);
	ASSERT (!idItem.IsNull ());
	ASSERT (nFirstColumn >= 0);
	ASSERT (nFirstRow >= 0);
	ASSERT (nLastColumn >= 0);
	ASSERT (nLastRow >= 0);

    if (bAddSelGroup)
    {
        // --------------------
        // Set new active item.
        // --------------------
   		m_idActive = idItem;
		m_idLastSel = idItem;

        // ---------------------------------
        // Store previously selected blocks.
        // Add new selected block.
        // ---------------------------------

		BOOL bRow = ((dwSelMode & SM_ROW) != 0 || m_idActive.IsRow () || IsWholeRowSel ());
		BOOL bColumn = ((dwSelMode & SM_COLUMN) != 0 || m_idActive.IsColumn ());

		CBCGPGridRange* pSelRange = new CBCGPGridRange (
			bRow ? nFirstColumn : m_idActive.m_nColumn,
			bColumn ? nFirstRow : m_idActive.m_nRow,
			bRow ? nLastColumn : m_idActive.m_nColumn,
			bColumn ? nLastRow : m_idActive.m_nRow );

		CBCGPGridRange range;
		UnionRange (pSelRange, GetMergedRange (idItem, m_pSetSelItem, range));
		ExtendMergedRange (*pSelRange);

		for (POSITION pos = m_lstSel.GetHeadPosition(); pos != NULL; )
		{
			CBCGPGridRange* pRange = m_lstSel.GetNext(pos);
			ASSERT (pRange != NULL);

			if (*pRange == *pSelRange)
			{
				return FALSE;		
			}
		}

		m_lstSel.AddTail (pSelRange);
    }
    else
    {
        // ---------------------------
        // Store previous active item.
        // ---------------------------
		if (m_idActive.IsNull ())
		{
			m_idActive = idItem;
		}
		m_idLastSel = idItem;

		if (m_lstSel.IsEmpty ())
		{
			BOOL bRow = ((dwSelMode & SM_ROW) != 0 || m_idActive.IsRow ());
			BOOL bColumn = ((dwSelMode & SM_COLUMN) != 0 || m_idActive.IsColumn ());

			CBCGPGridRange* pSelRange = new CBCGPGridRange (
				bRow ? nFirstColumn : m_idActive.m_nColumn,
				bColumn ? nFirstRow : m_idActive.m_nRow,
				bRow ? nLastColumn : m_idActive.m_nColumn,
				bColumn ? nLastRow : m_idActive.m_nRow );
			
			CBCGPGridRange range;
			UnionRange (pSelRange, GetMergedRange (m_idActive, GetCurSelItem (), range));

			m_lstSel.AddTail (pSelRange);
		}

        // ---------------------------
        // Modify last selected block.
        // ---------------------------
		CBCGPGridRange* pRangeLast = (CBCGPGridRange*) m_lstSel.GetTail ();
		ASSERT (pRangeLast != NULL);

		SelectRange (*pRangeLast, FALSE, bRedraw);

		BOOL bRow = ((dwSelMode & SM_ROW) != 0 || m_idActive.IsRow () || idItem.IsRow () || IsWholeRowSel ());
		BOOL bColumn = ((dwSelMode & SM_COLUMN) != 0 || m_idActive.IsColumn () || idItem.IsColumn ());

		int nLeftColumn = pRangeLast->m_nLeft;
		int nRightColumn = pRangeLast->m_nRight;

		if (!bRow && !idItem.IsRow ())
		{
			nLeftColumn = min (m_idActive.m_nColumn, idItem.m_nColumn);
			nRightColumn = max (m_idActive.m_nColumn, idItem.m_nColumn);
		}

		pRangeLast->Set (
			bRow ? nFirstColumn : nLeftColumn,
			bColumn ? nFirstRow : min (m_idActive.m_nRow, idItem.m_nRow),
			bRow ? nLastColumn : nRightColumn,
			bColumn ? nLastRow : max (m_idActive.m_nRow, idItem.m_nRow) );

		CBCGPGridRange range1;
		UnionRange (pRangeLast, GetMergedRange (m_idActive, GetCurSelItem (), range1));
		CBCGPGridRange range2;
		UnionRange (pRangeLast, GetMergedRange (idItem, m_pSetSelItem, range2));
		ExtendMergedRange (*pRangeLast);
    }

	return TRUE;
}
//*******************************************************************************************
BOOL CBCGPGridCtrl::DoInvertSelection (const CBCGPGridItemID& idItem, const DWORD dwSelMode,
									BOOL bRedraw,
									const int nFirstColumn, const int nFirstRow,
									const int nLastColumn, const int nLastRow)
{
	ASSERT_VALID (this);
	ASSERT (!idItem.IsNull ());
	ASSERT ((SM_INVERT_SEL & dwSelMode) != 0);
	ASSERT (nFirstColumn >= 0);
	ASSERT (nFirstRow >= 0);
	ASSERT (nLastColumn >= 0);
	ASSERT (nLastRow >= 0);
	
	BOOL bIsRow = idItem.IsRow () || ((dwSelMode & SM_ROW) != 0) || m_bWholeRowSel;

	for (POSITION pos = m_lstSel.GetHeadPosition (); pos != NULL; )
	{
		POSITION posSave = pos; // save position to remove it later if needed
		CBCGPGridRange* pRange = (CBCGPGridRange*) m_lstSel.GetNext (pos);
		ASSERT (pRange != NULL);
		
		// if pRange intersects idItem
		if (pRange->m_nTop <= idItem.m_nRow && pRange->m_nBottom >= idItem.m_nRow &&
			(bIsRow || pRange->m_nLeft <= idItem.m_nColumn && pRange->m_nRight >= idItem.m_nColumn))
		{
			SelectRange (*pRange, FALSE, bRedraw);

			// Devide range to several ranges or deflate one of its side.
			if (pRange->m_nTop < idItem.m_nRow && idItem.m_nRow > nFirstRow)
			{
				CBCGPGridRange* pRangeNew1 = new CBCGPGridRange (*pRange);
				pRangeNew1->m_nBottom = idItem.m_nRow - 1;

				m_lstSel.InsertAfter (posSave, pRangeNew1);
			}

			if (!bIsRow && pRange->m_nLeft < idItem.m_nColumn && idItem.m_nColumn > nFirstColumn)
			{
				CBCGPGridRange* pRangeNew2 = new CBCGPGridRange (*pRange);
				pRangeNew2->m_nRight = idItem.m_nColumn - 1;
				
				m_lstSel.InsertAfter (posSave, pRangeNew2);
			}

			if (!bIsRow && pRange->m_nRight > idItem.m_nColumn && idItem.m_nColumn < nLastColumn)
			{
				CBCGPGridRange* pRangeNew3 = new CBCGPGridRange (*pRange);
				pRangeNew3->m_nLeft = idItem.m_nColumn + 1;
				
				m_lstSel.InsertAfter (posSave, pRangeNew3);
			}

			if (pRange->m_nBottom > idItem.m_nRow && idItem.m_nRow < nLastRow)
			{
				CBCGPGridRange* pRangeNew4 = new CBCGPGridRange (*pRange);
				pRangeNew4->m_nTop = idItem.m_nRow + 1;

				m_lstSel.InsertAfter (posSave, pRangeNew4);
			}

			m_lstSel.RemoveAt (posSave); // remove old one
			delete pRange;
		}
	}

	return TRUE;
}
//*******************************************************************************************
void CBCGPGridCtrl::UnionRange (CBCGPGridRange* pRange, const CBCGPGridRange* pRange2)
{
	if (pRange != NULL && pRange2 != NULL)
	{
		if (pRange->IsEmpty ())
		{
			pRange->Set (*pRange2);
		}

//		int nPosLeft1 = GetColumnsInfo ().IndexToOrder (pRange->m_nLeft);
//		int nPosRight1 = GetColumnsInfo ().IndexToOrder (pRange->m_nRight);
//		int nPosLeft2 = GetColumnsInfo ().IndexToOrder (pRange2->m_nLeft);
//		int nPosRight2 = GetColumnsInfo ().IndexToOrder (pRange2->m_nRight);
		int nPosLeft1 = pRange->m_nLeft;
		int nPosRight1 = pRange->m_nRight;
		int nPosLeft2 = pRange2->m_nLeft;
		int nPosRight2 = pRange2->m_nRight;

		if (nPosLeft2 != -1 && nPosRight2 != -1)
		{
			if (nPosLeft1 != -1 && min (nPosLeft2, nPosRight2) < nPosLeft1)
			{
				pRange->m_nLeft = pRange2->m_nLeft;
			}

			if (nPosRight1 != -1 && max (nPosLeft2, nPosRight2) > nPosRight1)
			{
				pRange->m_nRight = pRange2->m_nRight;
			}
		}

		int nTop = min (pRange2->m_nTop, pRange2->m_nBottom);
		int nBottom = max (pRange2->m_nTop, pRange2->m_nBottom);

		if (nTop < pRange->m_nTop)
		{
			pRange->m_nTop = nTop;
		}

		if (nBottom > pRange->m_nBottom)
		{
			pRange->m_nBottom = nBottom;
		}
	}
}
//*******************************************************************************************
CBCGPGridRow* CBCGPGridCtrl::CreateRow (int nColumns)
{
	ASSERT_VALID (this);
	
	CBCGPGridRow* pRow = CreateRow ();
	ASSERT_VALID (pRow);

	SetupRow(pRow, nColumns);

	return pRow;
}
//*******************************************************************************************
CBCGPGridRow* CBCGPGridCtrl::CreateRow (CString strName)
{
	ASSERT_VALID (this);

	CBCGPGridRow* pGroup = CreateRow ();
	ASSERT_VALID (pGroup);
	pGroup->AllowSubItems (TRUE);

	CBCGPGridItem* pGridItem = new CBCGPGridItem (_variant_t (strName));
	pGridItem->m_bEnabled = FALSE;
	pGroup->AddItem (pGridItem);

	return pGroup;
}
//*******************************************************************************************
CBCGPGridRow* CBCGPGridCtrl::CreateMultiLineRow (int nLines)
{
	ASSERT_VALID (this);

	if (IsKindOf (RUNTIME_CLASS (CBCGPReportCtrl)))
	{
		TRACE0("CBCGPGridCtrl::CreateMultiLineRow: Report control cannot have multi-line rows\n");
		return CreateRow(GetColumnCount());
	}

	nLines = max(nLines, 1);

	CBCGPGridRow* pRow = CreateRow();
	ASSERT_VALID (pRow);

	pRow->m_nLines = nLines;

	SetupRow(pRow, GetColumnCount());
	
	return pRow;
}
//*******************************************************************************************
void CBCGPGridCtrl::SetupRow(CBCGPGridRow* pRow, int nColumns)
{
	ASSERT_VALID (pRow);
	
	pRow->SetOwnerList (this);
	
	for (int i = 0; i < nColumns; i++)
	{
		CBCGPGridItem* pItem = pRow->CreateItem((int) GetRowCount (), i);
		
		pItem->SetOwnerRow (pRow);
		int nIndex = (int) pRow->m_arrRowItems.Add (pItem);

		pItem->m_nIdColumn = nIndex;
	}
}
//*******************************************************************************************
CBCGPGridRow* CBCGPGridCtrl::CreateCaptionRow (CString strCaption)
{
	return new CBCGPGridCaptionRow (strCaption, 0);
}
//*******************************************************************************************
CBCGPGridItem* CBCGPGridCtrl::CreateItem (int nRow, int nColumn)
{
	CBCGPGridItem* pItem = NULL;

	CRuntimeClass* pItemRTC = NULL;
	if (m_mapItemsRTC.Lookup (nColumn, pItemRTC))
	{
		ASSERT (pItemRTC != NULL);

		pItem = DYNAMIC_DOWNCAST (CBCGPGridItem, pItemRTC->CreateObject ());
	}
	else if (m_pDefaultItemRTC != NULL)
	{
		pItem = DYNAMIC_DOWNCAST (CBCGPGridItem, m_pDefaultItemRTC->CreateObject ());
	}
	else
	{
		pItem = new CBCGPGridItem ();
	}

	if (pItem == NULL)
	{
		ASSERT (FALSE);
		return NULL;
	}

	ASSERT_VALID (pItem);
	
	if (!pItem->OnCreate (nRow, nColumn))
	{
		delete pItem;
		return NULL;
	}

	return pItem;
}
//*******************************************************************************************
void CBCGPGridCtrl::SetItemRTC (int nColumn, CRuntimeClass* pRuntimeClass)
{
	if (pRuntimeClass != NULL)
	{
		m_mapItemsRTC.SetAt (nColumn, pRuntimeClass);
	}
	else
	{
		m_mapItemsRTC.RemoveKey (nColumn);
	}
}
//*******************************************************************************************
CRuntimeClass* CBCGPGridCtrl::GetItemRTC (int nColumn) const
{
	CRuntimeClass* pItemRTC = NULL;
	m_mapItemsRTC.Lookup (nColumn, pItemRTC);

	return pItemRTC;
}
//*******************************************************************************************
BOOL CBCGPGridCtrl::CanReplaceItem (int /*nRow*/, int /*nColumn*/, CRuntimeClass* /*pNewRTC*/)
{
	return TRUE;
}
//*******************************************************************************************
int CBCGPGridCtrl::HitTestGroupByBox (CPoint point, LPRECT lprectItem/* = NULL*/)
{
	ASSERT_VALID (this);

	CClientDC dc (this);
	HFONT hfontOld = SetCurrFont (&dc);

	int nResult = -1;

	for (int i = 0; i < GetColumnsInfo ().GetGroupColumnCount (); i++)
	{
		CRect rectColumn;
		int nColumn = GetGroupColumnRect (i, rectColumn, &dc);

		if (nColumn >= 0 && rectColumn.PtInRect (point))
		{
			nResult = nColumn;
			if (lprectItem != NULL)
			{
				*lprectItem = rectColumn;
			}
			break;
		}
	}

	::SelectObject (dc.GetSafeHdc (), hfontOld);
	return nResult;
}
//*******************************************************************************************
int CBCGPGridCtrl::GetGroupByBoxDropIndex (CPoint point, LPPOINT lpptDrop/* = NULL*/)
{
	ASSERT_VALID(this);

	CRect rectClient;
	GetClientRect (rectClient);

	CRect rectGroupByBox = rectClient;
	rectGroupByBox.bottom = min (rectGroupByBox.top + m_nGroupByBoxHeight, rectClient.bottom);

	rectGroupByBox.DeflateRect (BCGPGRID_GROUPBYBOX_HMARGIN, BCGPGRID_GROUPBYBOX_VMARGIN, 0, 0);

	if (!rectGroupByBox.PtInRect (point))
	{
		return -1;
	}

	CClientDC dc (this);
	HFONT hfontOld = SetCurrFont (&dc);

	CPoint ptDrop;

	ptDrop.x = rectGroupByBox.left;
	ptDrop.y = (rectGroupByBox.top + rectGroupByBox.bottom - BCGPGRID_GROUPBYBOX_VMARGIN) / 2;

	int i = 0;
	BOOL bFirst = TRUE;

	for (i = 0; i < GetColumnsInfo ().GetGroupColumnCount (); i++)
	{
		CRect rectColumn;
		if (GetGroupColumnRect (i, rectColumn, &dc) < 0)
		{
			continue;
		}

		if (bFirst)
		{
			ptDrop.x = rectColumn.left;
			ptDrop.y = (rectColumn.top + rectColumn.bottom) / 2;

			bFirst = FALSE;
		}

		if (point.x <= rectColumn.CenterPoint ().x)
		{
			break;
		}

		ptDrop.x = rectColumn.right;
		ptDrop.y = (rectColumn.top + rectColumn.bottom) / 2;
	}

	::SelectObject (dc.GetSafeHdc (), hfontOld);

	if (lpptDrop != NULL)
	{
		*lpptDrop = ptDrop;
	}

	return i;
}
//*******************************************************************************************
CBCGPGridRow* CBCGPGridCtrl::HitTestRowHeader (CPoint point, LPRECT lprectItem)
{
	ASSERT_VALID (this);

	if (!m_bRowHeader)
	{
		return NULL;
	}

	if (m_rectRowHeader.PtInRect (point))
	{
		CPoint pt = point;
		pt.x = m_rectList.left + 1;

		CBCGPGridRow* pHitRow = HitTest (pt);

		if (pHitRow != NULL)
		{
			if (lprectItem != NULL)
			{
				*lprectItem = pHitRow->GetRect ();
				lprectItem->left = m_rectRowHeader.left;
				lprectItem->right = m_rectRowHeader.right;
			}

			return pHitRow;
		}
	}

	return NULL;
}
//*******************************************************************************************
BOOL CBCGPGridCtrl::HitTestSelectionBorder (CPoint point) const
{
	if (m_rectTrackSel.IsRectEmpty ())
	{
		return FALSE;
	}

	CRect rect = m_rectTrackSel;
	if (rect.PtInRect (point))
	{
		rect.DeflateRect (2, 2);
		if (!rect.PtInRect (point))
		{
			return TRUE;
		}
	}

	return FALSE;
}
//*******************************************************************************************
void CBCGPGridCtrl::SetPrinterFont(CFont* pFont, CFont* pFontBold)
{
	ASSERT_VALID(this);

	m_hPrinterFont = (HFONT)pFont->GetSafeHandle();
	m_hPrinterBoldFont = (HFONT)pFontBold->GetSafeHandle();
}
//*******************************************************************************************
void CBCGPGridCtrl::Print ()
{
	// Printing without the Document/View framework
	ASSERT_VALID(this);

    CDC dc;
    CPrintDialog printDlg(FALSE);
	 
    if (printDlg.DoModal() == IDCANCEL)
        return;
	
    dc.Attach(printDlg.GetPrinterDC());
    dc.m_bPrinting = TRUE;

    CString strTitle;
    strTitle.LoadString(AFX_IDS_APP_TITLE);

    DOCINFO di;
    ::ZeroMemory (&di, sizeof (DOCINFO));
    di.cbSize = sizeof (DOCINFO);
    di.lpszDocName = strTitle;

    BOOL bPrintingOK = dc.StartDoc(&di);

	m_bIsPrinting = TRUE;
	m_pPrintDC = &dc;

    CPrintInfo printInfo;
    printInfo.m_rectDraw.SetRect(0,0,
							dc.GetDeviceCaps(HORZRES), 
							dc.GetDeviceCaps(VERTRES));

	// -----------------------------------------
	// Prepare page for printing, calc page size
	// -----------------------------------------
	int nFirstItem = 0;						// By default print all grid items
	int nLastItem = GetTotalItems () - 1;	// from first row to the last
	OnPreparePrintPages (&printInfo, nFirstItem, nLastItem);

    OnBeginPrinting(&dc, &printInfo);

	// ---------------
	// Set up margins:
	// ---------------
	CRect rectMargins = OnGetPageMargins (&dc, &printInfo);
	printInfo.m_rectDraw.DeflateRect (&rectMargins);

	int nPagesCount = OnCalcPrintPages (&dc, &printInfo);
	printInfo.SetMaxPage (nPagesCount);

	CRect rectDraw = printInfo.m_rectDraw;

    for (printInfo.m_nCurPage = printInfo.GetMinPage(); 
         printInfo.m_nCurPage <= printInfo.GetMaxPage() && bPrintingOK; 
         printInfo.m_nCurPage++)
    {
		printInfo.m_rectDraw = rectDraw;

        dc.StartPage();

        OnPrint(&dc, &printInfo);
        bPrintingOK = (dc.EndPage() > 0);
    }
    OnEndPrinting(&dc, &printInfo);

    if (bPrintingOK)
        dc.EndDoc();
    else
        dc.AbortDoc();

	m_bIsPrinting = FALSE;
	m_pPrintDC = NULL;

    dc.DeleteDC();

	AdjustLayout ();
}
//*******************************************************************************************
void CBCGPGridCtrl::OnPreparePrintPages (CPrintInfo* pInfo,
										 int nFirstItem, int nLastItem)
{
	ASSERT_VALID(this);
	ASSERT (pInfo != NULL);
	ASSERT (nFirstItem >= 0);
	ASSERT (nLastItem >= 0);

	m_PrintParams.m_nBaseHeight = 0;
	m_PrintParams.m_nRowHeight = 0;
	m_PrintParams.m_nLargeRowHeight = 0;
	m_PrintParams.m_nGroupByBoxHeight = 0;
	m_PrintParams.m_nRowHeaderWidth = 0;
	m_PrintParams.m_nPageHeaderHeight = 0;
	m_PrintParams.m_nPageFooterHeight = 0;
	m_PrintParams.m_rectHeader.SetRectEmpty ();
	m_PrintParams.m_rectRowHeader.SetRectEmpty ();
	m_PrintParams.m_rectSelectAllArea.SetRectEmpty ();
	m_PrintParams.m_rectList.SetRectEmpty ();
	m_PrintParams.m_CachedPrintItems.CleanUpCache ();
	m_PrintParams.m_nVertScrollOffset = 0;
	m_PrintParams.m_nHorzScrollOffset = 0;

	m_PrintParams.m_pPrintInfo = pInfo;
	m_PrintParams.m_pageInfo.Init (		// set up printing range
		min (nFirstItem, nLastItem), max (nFirstItem, nLastItem),
		0, GetColumnsInfo ().GetColumnCount (TRUE) - 1);

	m_PrintParams.m_idCur.SetNull ();

	pInfo->m_lpUserData = &m_PrintParams.m_pageInfo;
}
//*******************************************************************************************
int CBCGPGridCtrl::OnCalcPrintPages (CDC* pDC, CPrintInfo* pInfo)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);
	ASSERT (pInfo != NULL);
	ASSERT (pInfo == m_PrintParams.m_pPrintInfo);
	ASSERT (m_bIsPrinting);

	ASSERT (m_hPrinterFont != NULL);		// fonts must be initialized
	ASSERT (m_hPrinterBoldFont != NULL);

	// -------------------------
	// Init printing parameters:
	// -------------------------
	SetRowHeight ();
	SetRowHeaderWidth ();

    //----------------------------------
    // Get page header and footer sizes:
    //----------------------------------
	CRect rectPageHeader;
	rectPageHeader.SetRectEmpty ();
	OnGetPageHeaderRect (pDC, pInfo, rectPageHeader);
	m_PrintParams.m_nPageHeaderHeight = rectPageHeader.Height ();

	CRect rectPageFooter;
	rectPageFooter.SetRectEmpty ();
	OnGetPageFooterRect (pDC, pInfo, rectPageFooter);
	m_PrintParams.m_nPageFooterHeight = rectPageFooter.Height ();

	CRect rectDraw = pInfo->m_rectDraw;
	rectDraw.top += m_PrintParams.m_nPageHeaderHeight;
	rectDraw.bottom -= m_PrintParams.m_nPageFooterHeight;

	//--------------
	// Layout parts:
	//--------------
	m_PrintParams.m_nGroupByBoxHeight = OnGetGroupByBoxRect (pDC, rectDraw).Height ();
	m_PrintParams.m_rectHeader = OnGetHeaderRect (pDC, rectDraw);
	m_PrintParams.m_rectRowHeader = OnGetRowHeaderRect (pDC, rectDraw);
	m_PrintParams.m_rectSelectAllArea = OnGetSelectAllAreaRect (pDC, rectDraw);
 	m_PrintParams.m_rectList = OnGetGridRect (pDC, rectDraw);

	// ----------------
	// Calc pages info:
	// ----------------
	return CalcPages (pDC, pInfo);
}
//*******************************************************************************************
void CBCGPGridCtrl::OnBeginPrinting(CDC* pDC, CPrintInfo* /*pInfo*/)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	if (m_hPrinterFont == NULL || m_hPrinterBoldFont == NULL)
	{
		// get current screen font object metrics
		CFont* pFont = GetFont();
		LOGFONT lf;
		ZeroMemory(&lf, sizeof(lf));
		LOGFONT lfSys;
		ZeroMemory(&lfSys, sizeof(lfSys));

		if (pFont == NULL)
		{
			pFont = CFont::FromHandle (GetDefaultFont ());
			if (pFont == NULL)
			{
				return;
			}
		}

		VERIFY(pFont->GetObject(sizeof(LOGFONT), &lf));
		VERIFY(::GetObject(::GetStockObject(SYSTEM_FONT), sizeof(LOGFONT),
			&lfSys));
		if (lstrcmpi((LPCTSTR)lf.lfFaceName, (LPCTSTR)lfSys.lfFaceName) == 0)
			return;

		// map to printer font metrics
		HDC hDCFrom = ::GetDC(NULL);
		lf.lfHeight = ::MulDiv(lf.lfHeight, pDC->GetDeviceCaps(LOGPIXELSY),
			::GetDeviceCaps(hDCFrom, LOGPIXELSY));
		lf.lfWidth = ::MulDiv(lf.lfWidth, pDC->GetDeviceCaps(LOGPIXELSX),
			::GetDeviceCaps(hDCFrom, LOGPIXELSX));
		::ReleaseDC(NULL, hDCFrom);

		if (m_hPrinterFont == NULL)
		{
			// create it, if it fails we just use the printer's default.
			m_hMirrorFont = ::CreateFontIndirect(&lf);
			m_hPrinterFont = m_hMirrorFont;
		}

		if (m_hPrinterBoldFont == NULL)
		{
			lf.lfWeight = FW_BOLD;
			m_hMirrorBoldFont = ::CreateFontIndirect(&lf);
			m_hPrinterBoldFont = m_hMirrorBoldFont;
		}
	}

	SetPrintColors ();

	ASSERT_VALID(this);
}
//*******************************************************************************************
void CBCGPGridCtrl::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	ASSERT_VALID(this);

	if (m_hMirrorFont != NULL && m_hPrinterFont == m_hMirrorFont)
	{
		AfxDeleteObject((HGDIOBJ*)&m_hMirrorFont);
		m_hPrinterFont = NULL;
	}

	if (m_hMirrorBoldFont != NULL && m_hPrinterBoldFont == m_hMirrorBoldFont)
	{
		AfxDeleteObject((HGDIOBJ*)&m_hMirrorBoldFont);
		m_hPrinterBoldFont = NULL;
	}
}
//*******************************************************************************************
void CBCGPGridCtrl::SetPrintColors ()
{
	ASSERT_VALID (this);

	m_clrPrintBorder = RGB (0, 0, 0);			// black
	m_clrPrintHeader = RGB (128, 128, 128);		// grey
	m_clrPrintHeaderBG = RGB (255, 255, 255);	// white
	m_clrPrintGroup = m_clrPrintHeader;
	m_clrPrintGroupBG = m_clrPrintHeaderBG;
	m_clrPrintLeftOffset = RGB (192, 192, 192);	// light grey
	m_clrPrintBG = RGB (255, 255, 255);			// white
	m_clrPrintLine = RGB (192, 192, 192);			// light grey
	m_clrPrintText = RGB (0, 0, 0);				// black
}
//*******************************************************************************************
void CBCGPGridCtrl::OnPrint(CDC* pDC, CPrintInfo* pInfo)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);
	ASSERT(pInfo != NULL);
	ASSERT(pInfo->m_bContinuePrinting);

	HFONT hfontOld = SetCurrFont (pDC);
	pDC->SetTextColor (m_clrPrintText);
	pDC->SetBkMode (TRANSPARENT);

	// ---------------------------
	// Layout page, calc page size
	// ---------------------------
	PrintLayout (pDC, pInfo);

	// ----------------------------
	// print page header and footer
	// ----------------------------
	OnPrintPageHeader(pDC, pInfo);
	OnPrintPageFooter(pDC, pInfo);

	// --------------------------------------------
	// print current page inside pInfo->m_rectDraw:
	// --------------------------------------------
	CRect rectGroupByBox = pInfo->m_rectDraw;
	rectGroupByBox.bottom = rectGroupByBox.top + m_PrintParams.m_nGroupByBoxHeight;
	rectGroupByBox.bottom = min (rectGroupByBox.bottom, pInfo->m_rectDraw.bottom);
	OnDrawGroupByBox (pDC, rectGroupByBox);

	OnPrintHeader (pDC, pInfo);
	OnPrintSelectAllArea (pDC, pInfo);

	OnPrintList (pDC, pInfo);

	::SelectObject (pDC->GetSafeHdc (), hfontOld);
}
//*******************************************************************************************
void CBCGPGridCtrl::OnPrintPageHeader(CDC* pDC, CPrintInfo* pInfo)
{
	ASSERT(pInfo != NULL);

	CRect rectHeader;
	if (OnGetPageHeaderRect (pDC, pInfo, rectHeader))
	{
		// use page header - deflate drawing area at top
		if (pInfo->m_rectDraw.top < rectHeader.bottom)
		{
			pInfo->m_rectDraw.top = min (rectHeader.bottom, pInfo->m_rectDraw.bottom);
		}
	}
}
//*******************************************************************************************
void CBCGPGridCtrl::OnPrintPageFooter(CDC* pDC, CPrintInfo* pInfo)
{
	ASSERT(pInfo != NULL);

	CRect rectFooter;
	if (OnGetPageFooterRect (pDC, pInfo, rectFooter))
	{
		// use page footer - deflate drawing area at bottom
		if (pInfo->m_rectDraw.bottom > rectFooter.top)
		{
			pInfo->m_rectDraw.bottom = max (rectFooter.top, pInfo->m_rectDraw.top);
		}
	}
}
//*******************************************************************************************
CRect CBCGPGridCtrl::OnGetPageMargins (CDC* pDC, CPrintInfo* /*pInfo*/)
{
	ASSERT_VALID (pDC);

	// Page margins:
	double dLeftOffset = 0.5;
	double dTopOffset = 0.5;
	double dRightOffset = 0.5;
	double dBottomOffset = 0.5;
	CRect rectMargins (
		(int)(pDC->GetDeviceCaps(LOGPIXELSX) * dLeftOffset),
		(int)(pDC->GetDeviceCaps(LOGPIXELSY) * dTopOffset),
		(int)(pDC->GetDeviceCaps(LOGPIXELSX) * dRightOffset),
		(int)(pDC->GetDeviceCaps(LOGPIXELSY) * dBottomOffset));

	return rectMargins;
}
//*******************************************************************************************
BOOL CBCGPGridCtrl::OnGetPageHeaderRect (CDC* /*pDC*/, CPrintInfo* /*pInfo*/, CRect& /*rect*/)
{
	return FALSE;
}
//*******************************************************************************************
BOOL CBCGPGridCtrl::OnGetPageFooterRect (CDC* /*pDC*/, CPrintInfo* /*pInfo*/, CRect& /*rect*/)
{
	return FALSE;
}
//*******************************************************************************************
CRect CBCGPGridCtrl::OnGetGroupByBoxRect (CDC* pDC, const CRect& rectDraw)
{
	CRect rect;
	if (m_bGroupByBox)
	{
		int nGroupByBoxHeight = 0;

		int nYMul = 1;
		int nYDiv = 1;
		if (m_bIsPrinting && pDC != NULL)
		{
			ASSERT_VALID (pDC);

			// map to printer metrics
			HDC hDCFrom = ::GetDC(NULL);
			nYMul = pDC->GetDeviceCaps(LOGPIXELSY);			// pixels in print dc
			nYDiv = ::GetDeviceCaps(hDCFrom, LOGPIXELSY);	// pixels in screen dc
			::ReleaseDC(NULL, hDCFrom);
		}

		if (!m_bIsPrinting && GetColumnsInfo ().GetGroupColumnCount () > 1 ||
			m_bIsPrinting && GetColumnsInfo ().GetGroupColumnCount () >= 1)
		{
			int nItemHeight = (m_bIsPrinting ? m_PrintParams.m_nBaseHeight : m_nBaseHeight) +
				::MulDiv (TEXT_MARGIN, nYMul, nYDiv);
			nGroupByBoxHeight = nItemHeight +
				2 * ::MulDiv (BCGPGRID_GROUPBYBOX_VMARGIN, nYMul, nYDiv) + 
				(GetColumnsInfo ().GetGroupColumnCount () - 1) * 
				::MulDiv (BCGPGRID_GROUPBYBOX_VSPACING, nYMul, nYDiv);
		}
		else if (!m_bIsPrinting)
		{
			nGroupByBoxHeight = m_nBaseHeight + 16;
		}

		rect = rectDraw;
		rect.bottom = rect.top + nGroupByBoxHeight;
	}
	else
	{
		rect.SetRectEmpty ();
	}

	return rect;
}
//*******************************************************************************************
CRect CBCGPGridCtrl::OnGetHeaderRect (CDC* pDC, const CRect& rectDraw)
{
	CRect rect;
	if (m_bHeader)
	{
		int nYMul = 1;
		int nYDiv = 1;
		if (m_bIsPrinting && pDC != NULL)
		{
			ASSERT_VALID (pDC);

			// map to printer metrics
			HDC hDCFrom = ::GetDC(NULL);
			nYMul = pDC->GetDeviceCaps(LOGPIXELSY);			// pixels in print dc
			nYDiv = ::GetDeviceCaps(hDCFrom, LOGPIXELSY);	// pixels in screen dc
			::ReleaseDC(NULL, hDCFrom);
		}

		int nHeaderHeight = (m_bIsPrinting ? m_PrintParams.m_nBaseHeight : m_nBaseHeight) + ::MulDiv (4, nYMul, nYDiv);

		rect = rectDraw;
		rect.top += (m_bIsPrinting ? m_PrintParams.m_nGroupByBoxHeight : m_nGroupByBoxHeight);
		rect.bottom = rect.top + nHeaderHeight;

		if (m_bRowHeader)
		{
			rect.left += (m_bIsPrinting ? m_PrintParams.m_nRowHeaderWidth : m_nRowHeaderWidth);
		}
	}
	else
	{
		rect.SetRectEmpty ();
	}

	return rect;
}
//*******************************************************************************************
CRect CBCGPGridCtrl::OnGetFilterBarRect (CDC*, const CRect& rectDraw)
{
	CRect rect;
	if (m_bFilterBar && !m_bIsPrinting)
	{
		rect = rectDraw;
		rect.top = m_rectHeader.bottom;
		rect.bottom = rect.top + m_nBaseHeight + 2;

		if (rect.bottom > rectDraw.bottom - m_nGridFooterHeight)
		{
			rect.SetRectEmpty ();
		}
	}
	else
	{
		rect.SetRectEmpty ();
	}
	
	return rect;
}
//*******************************************************************************************
CRect CBCGPGridCtrl::OnGetRowHeaderRect (CDC*, const CRect& rectDraw)
{
	CRect rect;
 	if (m_bRowHeader)
	{
		rect = rectDraw;

		if (m_bIsPrinting)
		{
			rect.top += m_PrintParams.m_nGroupByBoxHeight;
			rect.top += m_rectFilterBar.Height ();
			rect.right = rect.left + m_PrintParams.m_nRowHeaderWidth;
			
			if (!m_PrintParams.m_rectHeader.IsRectEmpty ())
			{
				rect.top = m_PrintParams.m_rectHeader.bottom;
			}
		}
		else
		{
			rect.top += m_nGroupByBoxHeight;
			rect.top += m_rectHeader.Height ();
			rect.top += m_rectFilterBar.Height ();
			rect.top += m_nGridHeaderHeight;
			if (rect.top + m_nRowHeight < rect.bottom)
			{
				rect.bottom = 
					max (rect.top + m_nRowHeight, rect.bottom - m_nGridFooterHeight);
			}

			rect.right = rect.left + m_nRowHeaderWidth;
		}
	}
	else
	{
		rect.SetRectEmpty ();
	}

	return rect;
}
//*******************************************************************************************
CRect CBCGPGridCtrl::OnGetSelectAllAreaRect (CDC*, const CRect&)
{
	CRect rect;
	if (m_bIsPrinting)
	{
		rect.SetRect (m_PrintParams.m_rectRowHeader.left, m_PrintParams.m_rectHeader.top, m_PrintParams.m_rectRowHeader.right, m_PrintParams.m_rectHeader.bottom);
	}
	else
	{
		rect.SetRect (m_rectRowHeader.left, m_rectHeader.top, m_rectRowHeader.right, m_rectHeader.bottom);
	}

	return rect;
}
//*******************************************************************************************
CRect CBCGPGridCtrl::OnGetGridRect (CDC*, const CRect& rectDraw)
{
	CRect rectList = rectDraw;

	if (m_bIsPrinting)
	{
		rectList.top += m_PrintParams.m_nGroupByBoxHeight;
		rectList.top += m_PrintParams.m_rectHeader.Height ();
		rectList.left += m_PrintParams.m_rectRowHeader.Width ();
	}
	else
	{
		rectList.top += m_nGroupByBoxHeight;
		rectList.top += m_rectHeader.Height ();
		rectList.top += m_rectFilterBar.Height ();
		rectList.top += m_nGridHeaderHeight;
		if (rectList.top + m_nRowHeight < rectList.bottom)
		{
			rectList.bottom = 
				max (rectList.top + m_nRowHeight, rectList.bottom - m_nGridFooterHeight);
		}

		rectList.left += m_rectRowHeader.Width ();
	}

	return rectList;
}
//*******************************************************************************************
CRect CBCGPGridCtrl::GetGridHeaderRect () const
{
	if (m_bIsPrinting)
	{
		return CRect (0, 0, 0, 0);
	}

	CRect rectClient;
	GetClientRect (rectClient);

	CRect rectHeader = rectClient;
	rectHeader.top += m_nGroupByBoxHeight + m_rectHeader.Height () + m_rectFilterBar.Height ();
	rectHeader.bottom = min (rectHeader.top + m_nGridHeaderHeight, rectHeader.bottom);
	return rectHeader;
}
//****************************************************************************************
CRect CBCGPGridCtrl::GetGridFooterRect () const
{
	if (m_bIsPrinting)
	{
		return CRect (0, 0, 0, 0);
	}

	CRect rectClient;
	GetClientRect (rectClient);

	CRect rectFooter = rectClient;
	if (m_nHorzScrollTotal > 0)
	{
		int cyScroll = ::GetSystemMetrics (SM_CYHSCROLL);
		rectFooter.bottom -= cyScroll;
	}

	rectFooter.top = m_rectList.bottom;
	rectFooter.bottom = max (rectFooter.top + m_nGridFooterHeight, rectFooter.bottom);

	return rectFooter;
}
//****************************************************************************************
void CBCGPGridCtrl::PrintLayout (CDC* pDC, CPrintInfo* pInfo)
{
	ASSERT_VALID (pDC);
	ASSERT (pInfo != NULL);
	ASSERT (pInfo == m_PrintParams.m_pPrintInfo);

	int nPage = (int) pInfo->m_nCurPage - 1;
	if (nPage < 0 || nPage >= m_arrPages.GetSize ())
	{
		//ASSERT (FALSE); // wrong page
		return;
	}

	// -------------------------------
	// Count items before current page
	// -------------------------------
	ASSERT (m_PrintParams.m_pageInfo.m_nFirstItem >= 0);
	ASSERT (m_PrintParams.m_pageInfo.m_nLastItem >= m_PrintParams.m_pageInfo.m_nFirstItem);
	
	int nTop = 0;
	int nBottom = m_PrintParams.m_pageInfo.m_nFirstItem;
	int nCountVisible = GetTotalItems (nTop, nBottom, TRUE);
	
	m_PrintParams.m_pageInfo.m_nGroupsCount = GetGroupsCount (nTop, nBottom, TRUE);
    m_PrintParams.m_pageInfo.m_nItemsCount = nCountVisible - m_PrintParams.m_pageInfo.m_nGroupsCount;

	for (int i = 0; i < nPage && i < m_arrPages.GetSize (); i++)
	{
		CBCGPGridPage& page = m_arrPages [i];

		if (page.m_nHorzOffset == 0)
		{
			m_PrintParams.m_pageInfo.m_nItemsCount += page.m_nItems;
			m_PrintParams.m_pageInfo.m_nGroupsCount += page.m_nGroups;
		}
	}

	// ----------------------
	// Get current page info:
	// ----------------------
    m_PrintParams.m_pageInfo.m_nPage = pInfo->m_nCurPage;

	CBCGPGridPage& page = m_arrPages [nPage];
	m_PrintParams.m_pageInfo.m_nFirstInPage = page.m_nFirstInPage;
	m_PrintParams.m_pageInfo.m_nItemsInPage = page.m_nItems;
	m_PrintParams.m_pageInfo.m_nGroupsInPage = page.m_nGroups;
	m_PrintParams.m_pageInfo.m_nTotalInPage = page.m_nTotal;
	m_PrintParams.m_pageInfo.m_nPageWidth = page.m_nWidth;
	m_PrintParams.m_pageInfo.m_nFirstColumnInPage = page.m_nFirstColumnInPage;
	m_PrintParams.m_pageInfo.m_nLastColumnInPage = page.m_nLastColumnInPage;

	// --------------------------
	// Prepare page for printing:
	// --------------------------
	// map to printer metrics
	HDC hDCFrom = ::GetDC(NULL);
	int nXMul = pDC->GetDeviceCaps(LOGPIXELSX);			// pixels in print dc
	int nXDiv = ::GetDeviceCaps(hDCFrom, LOGPIXELSX);	// pixels in screen dc
	int nYMul = pDC->GetDeviceCaps(LOGPIXELSY);			// pixels in print dc
	int nYDiv = ::GetDeviceCaps(hDCFrom, LOGPIXELSY);	// pixels in screen dc
	::ReleaseDC(NULL, hDCFrom);

	m_PrintParams.m_pageInfo.m_szOne.cx = ::MulDiv (1, nXMul, nXDiv);;
	m_PrintParams.m_pageInfo.m_szOne.cy = ::MulDiv (1, nYMul, nYDiv);;

	m_PrintParams.m_nVertScrollOffset = 
		m_PrintParams.m_pageInfo.m_nGroupsCount * m_PrintParams.m_nLargeRowHeight + 
		m_PrintParams.m_pageInfo.m_nItemsCount * m_PrintParams.m_nRowHeight;
	m_PrintParams.m_nHorzScrollOffset = page.m_nHorzOffset;

	// ----------------
	// Reposition items
	// ----------------
	if (m_bVirtualMode)
	{
		return;
	}

	m_PrintParams.m_idCur.m_nRow = m_PrintParams.m_pageInfo.m_nFirstInPage;
	m_PrintParams.m_idCur.m_nColumn = -1;

	int y = m_PrintParams.m_rectList.top - 1;
	
	const CList<CBCGPGridRow*, CBCGPGridRow*>& lst =
		(!IsSortingMode () && !IsGrouping ()) ? m_lstItems : m_lstTerminalItems;

	int nRow = m_PrintParams.m_pageInfo.m_nFirstInPage;
	POSITION pos = (nRow < lst.GetCount ()) ? lst.FindIndex (nRow) : NULL;
	for (; pos != NULL && nRow <= m_PrintParams.m_pageInfo.m_nLastItem; nRow++)
	{
		CBCGPGridRow* pItem = lst.GetNext (pos);
		ASSERT_VALID (pItem);

		pItem->Repos (y);
	}

	UpdateMergedItems ();
}
//*******************************************************************************************
int CBCGPGridCtrl::CalcPages (CDC* pDC, CPrintInfo* /*pInfo*/)
{
	ASSERT (m_bIsPrinting);

	// map to printer metrics
	HDC hDCFrom = ::GetDC(NULL);
	int nXMul = pDC->GetDeviceCaps(LOGPIXELSX);			// pixels in print dc
	int nXDiv = ::GetDeviceCaps(hDCFrom, LOGPIXELSX);	// pixels in screen dc
	int nYMul = pDC->GetDeviceCaps(LOGPIXELSY);			// pixels in print dc
	int nYDiv = ::GetDeviceCaps(hDCFrom, LOGPIXELSY);	// pixels in screen dc
	::ReleaseDC(NULL, hDCFrom);

	m_PrintParams.m_pageInfo.m_szOne.cx = ::MulDiv (1, nXMul, nXDiv);;
	m_PrintParams.m_pageInfo.m_szOne.cy = ::MulDiv (1, nYMul, nYDiv);;

	if (m_bVirtualMode)
	{
		//-----------------
		// Calculate pages:
		//-----------------
		m_arrPages.RemoveAll ();

		const int nLastVirtualRow = (m_nVirtualRows > 0) ? m_nVirtualRows - 1 : 0;

		int nFirstInPage = min (nLastVirtualRow, m_PrintParams.m_pageInfo.m_nFirstItem);
		const int nLast = min (nLastVirtualRow, m_PrintParams.m_pageInfo.m_nLastItem);
		const int nItems = m_PrintParams.m_nRowHeight != 0 ?
			m_PrintParams.m_rectList.Height () / m_PrintParams.m_nRowHeight : 0;

		CRect rectPage = m_PrintParams.m_rectList;
		rectPage.top = m_PrintParams.m_rectList.top - 1;

		do
		{
			if (nFirstInPage <= nLast)
			{
				int nPageItems = min (nItems, nLast - nFirstInPage + 1);

				CBCGPGridPage page;
				page.m_nFirstInPage = nFirstInPage;
				page.m_nItems = nPageItems;
				page.m_nGroups = 0;
				page.m_nTotal = nPageItems;

				page.m_nFirstColumnInPage = m_PrintParams.m_pageInfo.m_nFirstCol;
				page.m_nLastColumnInPage = m_PrintParams.m_pageInfo.m_nLastCol;
				
				//---------------------------------------------
				// Add horizontal row of one or more GridPages:
				//---------------------------------------------
				OnAddGridPageRowForPrint (page, rectPage);

				nFirstInPage += nPageItems;
			}
		}
		while (nFirstInPage <= nLast);

		return (int) m_arrPages.GetSize ();
	}


	m_PrintParams.m_idCur.m_nRow = 0;
	m_PrintParams.m_idCur.m_nColumn = -1;

	int y = m_PrintParams.m_rectList.top - 1;
	
	const CList<CBCGPGridRow*, CBCGPGridRow*>& lst =
		(!IsSortingMode () && !IsGrouping ()) ? m_lstItems : m_lstTerminalItems;

	POSITION pos = NULL;
	for (pos = lst.GetHeadPosition (); pos != NULL;)
	{
		CBCGPGridRow* pItem = lst.GetNext (pos);
		ASSERT_VALID (pItem);

		pItem->Repos (y);
	}

	//-----------------
	// Calculate pages:
	//-----------------
	m_arrPages.RemoveAll ();

	CRect rectPage = m_PrintParams.m_rectList;
	rectPage.top = m_PrintParams.m_rectList.top - 1;

	int nFirstInPage = m_PrintParams.m_pageInfo.m_nFirstItem;
	int nItems = 0;		// visible item count
	int nGroups = 0;	// visible group count
	int nTotal = 0;		// total item count

	pos = (nFirstInPage < lst.GetCount ()) ? lst.FindIndex (nFirstInPage) : NULL;
	while (pos != NULL && nFirstInPage + nTotal <= m_PrintParams.m_pageInfo.m_nLastItem)
	{
		POSITION posSave = pos;

		CBCGPGridRow* pItem = lst.GetNext (pos);
		ASSERT_VALID (pItem);

		if (!pItem->m_Rect.IsRectEmpty ())
		{
			//if (pItem->m_Rect.top >= rectPage.bottom)
			if (pItem->m_Rect.bottom >= rectPage.bottom)
			{
				if (nTotal == 0)
				{
					ASSERT (FALSE);
					break;
				}

				// item is above page bottom - next page
				CBCGPGridPage page;
				page.m_nFirstInPage = nFirstInPage;
				page.m_nItems = nItems;
				page.m_nGroups = nGroups;
				page.m_nTotal = nTotal;

				page.m_nFirstColumnInPage = m_PrintParams.m_pageInfo.m_nFirstCol;
				page.m_nLastColumnInPage = m_PrintParams.m_pageInfo.m_nLastCol;

				//---------------------------------------------
				// Add horizontal row of one or more GridPages:
				//---------------------------------------------
				OnAddGridPageRowForPrint (page, rectPage);

				rectPage.OffsetRect (0, nItems * m_PrintParams.m_nRowHeight + nGroups * m_PrintParams.m_nLargeRowHeight);
					
				nFirstInPage += nTotal;
				nItems = 0;
				nGroups = 0;
				nTotal = 0;

				pos = posSave;
				continue;
			}
			else if (pItem->m_Rect.bottom >= rectPage.top)
			{
				// item is below page top - item fits in the current page
				if (pItem->IsGroup ())
				{
					nGroups++;
				}
// 				else if (pItem->GetLinesNumber () > 1)
// 				{
// 					nItems += (pItem->GetLinesNumber () - 1);
// 				}
				else
				{
					nItems++;
				}
			}
			else
			{
				// else - item at previous pages - skip item
				ASSERT (FALSE);
			}
		}

		nTotal++;
	}

	if (nTotal > 0)
	{
		CBCGPGridPage page;
		page.m_nFirstInPage = nFirstInPage;
		page.m_nItems = nItems;
		page.m_nGroups = nGroups;
		page.m_nTotal = nTotal;

		page.m_nFirstColumnInPage = m_PrintParams.m_pageInfo.m_nFirstCol;
		page.m_nLastColumnInPage = m_PrintParams.m_pageInfo.m_nLastCol;

		OnAddGridPageRowForPrint (page, rectPage);
	}

	return (int) m_arrPages.GetSize ();
}
//*******************************************************************************************
int CBCGPGridCtrl::OnAddGridPageRowForPrint (CBCGPGridPage& page, CRect rectPage)
{
	page.m_nHorzOffset = 0;
	page.m_nWidth = rectPage.Width ();
	
	//---------------------------------------------
	// Add horizontal row of one or more GridPages:
	//---------------------------------------------
	// map to printer metrics
	HDC hDCFrom = ::GetDC(NULL);
	int nXMul = m_pPrintDC->GetDeviceCaps(LOGPIXELSX);	// pixels in print dc
	int nXDiv = ::GetDeviceCaps(hDCFrom, LOGPIXELSX);	// pixels in screen dc
	::ReleaseDC(NULL, hDCFrom);

	CSize szOne = m_PrintParams.m_pageInfo.m_szOne.cx;
	const int nMaxColumnOrder = GetColumnsInfo ().GetColumnCount (TRUE) - 1;
	
	// columns
	int nFirstColumn, nLastColumn;
	int nColumnOrder = nFirstColumn = nLastColumn = m_PrintParams.m_pageInfo.m_nFirstCol;
	int nXLeft = 0;
	
	// pages
	int nHorzOffset = 0;
	int nHorzPage = rectPage.Width ();
	int nHorzTotal = ::MulDiv (GetColumnsInfo ().GetTotalWidth (), nXMul, nXDiv);
	int nPageCount = 0;
	
	do
	{
		//-----------------------
		// Calculate column width
		//-----------------------
		int iColumn = (nColumnOrder >= 0 && nColumnOrder <= nMaxColumnOrder ? 
			GetColumnsInfo ().OrderToIndex (nColumnOrder) : -1);
		if (iColumn == -1)
		{
			break;
		}
		
		int nColumnWidth = ::MulDiv (GetColumnsInfo ().GetColumnWidth (iColumn), nXMul, nXDiv);
		
		BOOL bIsTreeColumn = (m_nTreeColumn == -1) ? (nColumnOrder == 0):
			(m_nTreeColumn == iColumn);
		if (bIsTreeColumn)
		{
			nColumnWidth += GetHierarchyOffset () * szOne.cx
				+ GetExtraHierarchyOffset () * szOne.cx;
		}
		
		//-------------------------
		// Next column or next page
		//-------------------------
		BOOL bNextPage = FALSE;
		BOOL bBreakColumn = FALSE;
		if (nXLeft <= 0 ||
			(!OnStartColumnOnNewPrintPage (iColumn)	&&
			(nXLeft + nColumnWidth <= nHorzPage ||	// A column fits into the page
			nColumnWidth > nHorzPage ||				// A column that is larger than the page must break
			OnBreakColumnAcrossPrintPages (iColumn))	))
		{
			// start column at the current page
			nLastColumn = nColumnOrder;
			
			if (nXLeft + nColumnWidth <= nHorzPage)
			{
				// next column
				nColumnOrder++;
				nXLeft += nColumnWidth;
			}
			else
			{
				bBreakColumn = TRUE;
				bNextPage = TRUE;
			}
		}
		else
		{
			// start column at new page
			bNextPage = TRUE;
		}

		if (bNextPage)
		{
			int nActualPageWidth = nHorzPage;
			if (nXLeft > 0 && !bBreakColumn)
			{
				nActualPageWidth = min (nHorzPage, nXLeft);
			}
			
			// add page
			page.m_nHorzOffset = nHorzOffset;
			page.m_nWidth = nActualPageWidth + 1;
			page.m_nFirstColumnInPage = nFirstColumn;
			page.m_nLastColumnInPage = nLastColumn;
			m_arrPages.Add (page);
			nPageCount++;
			
			// next page
			if (bBreakColumn)
			{
				nXLeft -= page.m_nWidth;
			}
			else
			{
				nXLeft = 0;
			}
			nHorzOffset += nActualPageWidth;

			nFirstColumn = nLastColumn = nColumnOrder;
		}
	}
	while (
		// stop rule by columns
		nColumnOrder <= m_PrintParams.m_pageInfo.m_nLastCol &&
		// stop rule by pages
		nHorzOffset < nHorzTotal);

	// add page
	page.m_nHorzOffset = nHorzOffset;
	page.m_nWidth = rectPage.Width ();
	page.m_nFirstColumnInPage = nFirstColumn;
	page.m_nLastColumnInPage = nLastColumn;
	m_arrPages.Add (page);
	nPageCount++;

	return nPageCount;
}
//*******************************************************************************************
BOOL CBCGPGridCtrl::OnStartColumnOnNewPrintPage (int /*nColumn*/)
{
	return FALSE;
}
//*******************************************************************************************
BOOL CBCGPGridCtrl::OnBreakColumnAcrossPrintPages (int /*nColumn*/)
{
	return m_bBreakColumnsAcrossPrintPages;
}
//*******************************************************************************************
void CBCGPGridCtrl::OnPrintHeader(CDC* pDC, CPrintInfo* /*pInfo*/)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);

	// map to printer metrics
	HDC hDCFrom = ::GetDC(NULL);
	int nXMul = pDC->GetDeviceCaps(LOGPIXELSX);			// pixels in print dc
	int nXDiv = ::GetDeviceCaps(hDCFrom, LOGPIXELSX);	// pixels in screen dc
	::ReleaseDC(NULL, hDCFrom);

	ASSERT (m_bIsPrinting);

	CRect rectHeader = m_PrintParams.m_rectHeader;

	// ------------------------------------
	// Print grid header inside rectHeader:
	// ------------------------------------
	CBrush brHeaderBG (m_clrPrintHeaderBG);
	pDC->FillRect (rectHeader, &brHeaderBG);

	CSize szOne = m_PrintParams.m_pageInfo.m_szOne;
	int nHorzScrollOffset = m_PrintParams.m_nHorzScrollOffset;

	CPen penHeader (PS_SOLID, szOne.cx, m_clrPrintHeader);
	CPen* pOldPen = pDC->SelectObject (&penHeader);
	COLORREF clrOld = pDC->GetTextColor ();

	int nColumnCount = 0; // count visible columns
	int nXLeft = m_PrintParams.m_rectList.left - nHorzScrollOffset;
	int nPos = GetColumnsInfo ().Begin ();
	while (nPos != GetColumnsInfo ().End ())
	{
		int iColumn = GetColumnsInfo ().Next (nPos);
		if (iColumn == -1)
		{
			break; // no more visible columns
		}

		ASSERT (iColumn >= 0);

		int nWidth = GetColumnsInfo ().GetColumnWidth (iColumn);
		nWidth = ::MulDiv(nWidth, nXMul, nXDiv);

		BOOL bIsTreeColumn = (m_nTreeColumn == -1) ? (nColumnCount == 0):
			(m_nTreeColumn == iColumn);
		if (bIsTreeColumn)
		{
			nWidth += GetHierarchyOffset () * szOne.cx +
				GetExtraHierarchyOffset () * szOne.cx;
		}

		if (nWidth > 0) // if column visible
		{
			CRect rectItem = rectHeader;
			rectItem.left = nXLeft;
			rectItem.right = rectItem.left + nWidth;

			CRect rectClipItem = rectItem;
			if (rectItem.left < rectHeader.left)
			{
				rectClipItem.left = max (rectHeader.left, rectClipItem.left);
			}
			if (rectItem.right > rectHeader.right)
			{
				rectClipItem.right = min (rectHeader.right, rectClipItem.right);
			}
			rectClipItem.right = min (m_PrintParams.m_rectList.left + m_PrintParams.m_pageInfo.m_nPageWidth, rectClipItem.right);

			if (rectClipItem.Width () > 0)
			{
				//-----------------
				// Fill background:
				//-----------------
				pDC->FillRect (rectClipItem, &brHeaderBG);

				//-------------
				// Draw column:
				//-------------
				GetColumnsInfo ().PrintColumn (pDC, iColumn, rectItem, rectClipItem);
			}
		}

		nXLeft += nWidth;
		nColumnCount++;
	}

	//-------------
	// Draw border:
	//-------------
	CRect rectItems = rectHeader;
	rectItems.left = max (m_PrintParams.m_rectList.left - nHorzScrollOffset, rectHeader.left);
	rectItems.right = min (nXLeft, rectHeader.right);
	rectItems.right = min (m_PrintParams.m_rectList.left + m_PrintParams.m_pageInfo.m_nPageWidth, rectItems.right);

	if (rectHeader.left <= nXLeft && rectHeader.right >= nXLeft)
	{
		pDC->MoveTo (rectItems.right, rectItems.top);
		pDC->LineTo (rectItems.BottomRight ());
	}

	if (rectItems.Width () > 0)
	{
		pDC->MoveTo (rectItems.TopLeft ());
		pDC->LineTo (rectItems.right, rectItems.top);
		pDC->MoveTo (rectItems.left, rectItems.bottom);
		pDC->LineTo (rectItems.BottomRight ());
	}

	pDC->SelectObject (pOldPen);
	pDC->SetTextColor (clrOld);
}
//*******************************************************************************************
void CBCGPGridCtrl::OnPrintRowHeaderItem(CDC* pDC, CPrintInfo* pInfo, CBCGPGridRow* pItem)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);
	ASSERT_VALID (pItem);
	ASSERT (pInfo != NULL);

	ASSERT (m_bIsPrinting);

 	CRect rectHeader = m_PrintParams.m_rectRowHeader;

	// -----------------------------------
	// Print row header inside rectHeader:
	// -----------------------------------
	CBrush brHeaderBG (m_clrPrintHeaderBG);

	CPen penHeader (PS_SOLID, m_PrintParams.m_pageInfo.m_szOne.cx, m_clrPrintHeader);
	CPen* pOldPen = pDC->SelectObject (&penHeader);
	COLORREF clrOld = pDC->GetTextColor ();

	{
		CRect rectItem = rectHeader;
		rectItem.top = pItem->m_Rect.top;
		rectItem.bottom = pItem->m_Rect.bottom;

		CRect rectClipItem = rectItem;
		rectClipItem.NormalizeRect ();
		if (rectClipItem.IntersectRect (&rectClipItem, &rectHeader))
		{
			//-----------------
			// Fill background:
			//-----------------
			pDC->FillRect (rectClipItem, &brHeaderBG);

			//-------------
			// Draw border:
			//-------------
			pDC->MoveTo (rectClipItem.TopLeft ());
			pDC->LineTo (rectClipItem.left, rectClipItem.bottom);
			pDC->MoveTo (rectClipItem.TopLeft ());
			pDC->LineTo (rectClipItem.right, rectClipItem.top);
			pDC->MoveTo (rectClipItem.left, rectClipItem.bottom);
			pDC->LineTo (rectClipItem.BottomRight ());
			pDC->MoveTo (rectClipItem.right, rectClipItem.top);
			pDC->LineTo (rectClipItem.BottomRight ());
			
			//------------------
			// Draw line number:
			//------------------
			if (m_bLineNumbers)
			{
				OnPrintLineNumber (pDC, pInfo, pItem, rectItem);
			}
		}
	}

	pDC->SelectObject (pOldPen);
	pDC->SetTextColor (clrOld);
}
//*******************************************************************************************
void CBCGPGridCtrl::OnPrintSelectAllArea(CDC* pDC, CPrintInfo*)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);

	ASSERT (m_bIsPrinting);

	CRect rectArea = m_PrintParams.m_rectSelectAllArea;

	if (rectArea.IsRectEmpty ())
	{
		return;
	}
	
	// ----------------------
	// Print inside rectArea:
	// ----------------------
	CBrush brHeaderBG (m_clrPrintHeaderBG);

	CPen penHeader (PS_SOLID, m_PrintParams.m_pageInfo.m_szOne.cx, m_clrPrintHeader);
	CPen* pOldPen = pDC->SelectObject (&penHeader);
	COLORREF clrOld = pDC->GetTextColor ();

	//-----------------
	// Fill background:
	//-----------------
	pDC->FillRect (rectArea, &brHeaderBG);

	//-------------
	// Draw border:
	//-------------
	pDC->MoveTo (rectArea.TopLeft ());
	pDC->LineTo (rectArea.left, rectArea.bottom);
	pDC->MoveTo (rectArea.TopLeft ());
	pDC->LineTo (rectArea.right, rectArea.top);
	pDC->MoveTo (rectArea.left, rectArea.bottom);
	pDC->LineTo (rectArea.BottomRight ());
	pDC->MoveTo (rectArea.right, rectArea.top);
	pDC->LineTo (rectArea.BottomRight ());

	pDC->SelectObject (pOldPen);
	pDC->SetTextColor (clrOld);
}
//*******************************************************************************************
void CBCGPGridCtrl::OnPrintLineNumber (CDC* pDC, CPrintInfo*, CBCGPGridRow* pRow, CRect rect)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);

	ASSERT (m_bIsPrinting);

	const int CALCULATED_TEXT_MARGIN = TEXT_MARGIN * m_PrintParams.m_pageInfo.m_szOne.cx;
	const CRect& rectClip = m_PrintParams.m_rectRowHeader;

	//-----------
	// Draw text:
	//-----------
	CRect rectLabel = rect;
	rectLabel.DeflateRect (CALCULATED_TEXT_MARGIN, 0);

	CString strLabel;
	if (pRow->GetRowId () >= 0)
	{
		strLabel.Format (_T("%d"), pRow->GetRowId () + 1);
	}

	CRect rectClipLabel = rectLabel;
	rectClipLabel.NormalizeRect ();
	if (rectClipLabel.IntersectRect (&rectClipLabel, &rectClip))
	{
		ASSERT_VALID (m_pPrintDC);

		int nVCenterOffset = CALCULATED_TEXT_MARGIN;

		if (pRow->GetLinesNumber() == 1)
		{
			// Draw text vertically centered
			TEXTMETRIC tm;
			pDC->GetTextMetrics (&tm);
			int nDescent = tm.tmDescent;
			nVCenterOffset = (rectLabel.Height () - pDC->GetTextExtent (strLabel).cy + nDescent) / 2;
		}

		pDC->SetTextAlign (TA_LEFT | TA_TOP);
		pDC->ExtTextOut (rectLabel.left, rectLabel.top + nVCenterOffset, ETO_CLIPPED, &rectClipLabel, strLabel, NULL);
	}
}
//*******************************************************************************************
void CBCGPGridCtrl::OnPrintList(CDC* pDC, CPrintInfo* pInfo)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);
	ASSERT (pInfo != NULL);

	CPen penLine (PS_SOLID, m_PrintParams.m_pageInfo.m_szOne.cx, m_clrPrintLine);
	CPen* pOldPen = pDC->SelectObject (&penLine);

	CBCGPGridPageInfo* pPageInfo = (CBCGPGridPageInfo*) pInfo->m_lpUserData;
	ASSERT (pPageInfo != NULL);

	ASSERT (m_bIsPrinting);

	// calc rectItems
	int nItemsBottom = m_PrintParams.m_rectList.top + 
		pPageInfo->m_nGroupsInPage * m_PrintParams.m_nLargeRowHeight + 
		pPageInfo->m_nItemsInPage * m_PrintParams.m_nRowHeight;
	nItemsBottom = min (nItemsBottom, m_PrintParams.m_rectList.bottom);

	CRect rectItems = m_PrintParams.m_rectList;
	rectItems.bottom = nItemsBottom;

	// ----------------------------------
	// Print grid items inside rectItems:
	// ----------------------------------
	// map to printer metrics
	HDC hDCFrom = ::GetDC(NULL);
	int nXMul = pDC->GetDeviceCaps(LOGPIXELSX);			// pixels in print dc
	int nXDiv = ::GetDeviceCaps(hDCFrom, LOGPIXELSX);	// pixels in screen dc
	::ReleaseDC(NULL, hDCFrom);

	int nHorzScrollOffset = m_PrintParams.m_nHorzScrollOffset;
	CSize szOne = m_PrintParams.m_pageInfo.m_szOne;

	int nPageRight = m_PrintParams.m_rectList.left + m_PrintParams.m_pageInfo.m_nPageWidth;
	int nXLeft = m_PrintParams.m_rectList.left - nHorzScrollOffset;
	int nColumnCount = 0;	// count visible columns

	int nPos = GetColumnsInfo ().Begin ();
	while (nPos != GetColumnsInfo ().End ())
	{
		int iColumn = GetColumnsInfo ().Next (nPos);
		if (iColumn == -1)
		{
			break; // no more visible columns
		}

		ASSERT (iColumn >= 0);
		ASSERT (iColumn < GetColumnsInfo ().GetColumnCount ());

		int nWidth = GetColumnsInfo ().GetColumnWidth (iColumn);
		nWidth = ::MulDiv(nWidth, nXMul, nXDiv);

		BOOL bIsTreeColumn = (m_nTreeColumn == -1) ? (nColumnCount == 0):
			(m_nTreeColumn == iColumn);
		if (bIsTreeColumn)
		{
			nWidth += GetHierarchyOffset () * szOne.cx +
				GetExtraHierarchyOffset () * szOne.cx;
		}

		nXLeft += nWidth;
		nColumnCount ++;
	}

	CRect rectClipItems = rectItems;
	rectClipItems.left = m_PrintParams.m_rectList.left - nHorzScrollOffset;
	rectClipItems.right = min (nXLeft, nPageRight);

	rectClipItems.NormalizeRect ();
	pPageInfo->m_rectPageItems.IntersectRect (rectClipItems, m_PrintParams.m_rectList);

	int nFirst = pPageInfo->m_nFirstInPage;
	int nCount = pPageInfo->m_nTotalInPage;

	if (m_bVirtualMode)
	{
		for (int i = 0; i < pPageInfo->m_nTotalInPage; i++)
		{
			const int nIndex = pPageInfo->m_nFirstInPage + i;

			CBCGPGridRow* pItem = m_PrintParams.m_CachedPrintItems.GetCachedRow (nIndex);
			if (pItem == NULL)
			{
				pItem = CreateVirtualRow (nIndex);

				if (!m_PrintParams.m_CachedPrintItems.SetCachedRow (nIndex, pItem))
				{
					ASSERT (FALSE);
					break;
				}
			}

			if (pItem != NULL)
			{
				ASSERT_VALID (pItem);

				int y = m_PrintParams.m_rectList.top - 1 + i * m_PrintParams.m_nRowHeight;
				m_PrintParams.m_idCur.m_nRow = nIndex;
				
				pItem->Repos (y);
				OnPrintItem (pDC, pInfo, pItem);
				OnPrintRowHeaderItem (pDC, pInfo, pItem);
			}
		}
	}

	else
	{
		const CList<CBCGPGridRow*, CBCGPGridRow*>& lst =
			(!IsSortingMode () && !IsGrouping ()) ? m_lstItems : m_lstTerminalItems;
		
		POSITION pos = (nFirst < lst.GetCount ()) ? lst.FindIndex (nFirst) : NULL;
		for (int j = nFirst; pos != NULL && j < nFirst + nCount; j++)
		{
			CBCGPGridRow* pItem = lst.GetNext (pos);
			ASSERT_VALID (pItem);
			
			if (!OnPrintItem (pDC, pInfo, pItem))
			{
				break;
			}
			OnPrintRowHeaderItem (pDC, pInfo, pItem);
		}
	}

	RedrawMergedItems (pDC);

	pDC->SelectObject (pOldPen);
}
//*******************************************************************************************
int CBCGPGridCtrl::OnPrintItem (CDC* pDC, CPrintInfo* pInfo, CBCGPGridRow* pItem) const
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);
	ASSERT (pInfo != NULL);
	ASSERT_VALID (pItem);
	ASSERT (m_bIsPrinting);

	if (!pItem->m_Rect.IsRectEmpty ())
	{
		if (pItem->m_Rect.top >= m_PrintParams.m_rectList.bottom)
		{
			return FALSE;
		}

		if (pItem->m_Rect.bottom >= m_PrintParams.m_rectList.top)
		{
			CBCGPGridPageInfo* pPageInfo = (CBCGPGridPageInfo*) pInfo->m_lpUserData;
			ASSERT (pPageInfo != NULL);

			int dx = IsSortingMode () && !IsGrouping () ? 0 : pItem->GetHierarchyLevel () * GetHierarchyLevelOffset () * pPageInfo->m_szOne.cx;
			
			// --------------------------
			// draw left hierarchy offset
			// --------------------------
			CRect rectLeft = pItem->m_Rect;
			rectLeft.top += pPageInfo->m_szOne.cy;
			rectLeft.right = rectLeft.left + GetExtraHierarchyOffset () * pPageInfo->m_szOne.cx + dx;
			rectLeft.bottom += pPageInfo->m_szOne.cy;

			CRect rectClipLeft = rectLeft;

			int nCol0Right = pItem->m_Rect.right;
			if (GetColumnsInfo ().GetColumnCount (TRUE) > 0)
			{
				int nCol0Idx = GetColumnsInfo ().OrderToIndex (0);
				if (nCol0Idx != -1)
				{
					int nCol0Width = GetColumnsInfo ().GetColumnWidth (nCol0Idx) + GetHierarchyOffset () + GetExtraHierarchyOffset ();
					if (nCol0Width > 0)
					{
 						nCol0Right = pItem->m_Rect.left + nCol0Width * pPageInfo->m_szOne.cx;
 						rectClipLeft.right = min (rectClipLeft.right, nCol0Right);
					}
				}
			}

			rectClipLeft.NormalizeRect ();
			rectClipLeft.IntersectRect (&m_PrintParams.m_rectList, &rectClipLeft);

			if (rectClipLeft.Width () > 0)
			{
				CBrush br (m_clrPrintLeftOffset);
				pDC->FillRect (rectClipLeft, &br);
			}

			CRect rectName = pItem->m_Rect;

			// ---------------
			// fill background
			// ---------------
			if (!pItem->HasValueField ())
			{
				CRect rectFill = rectName;
				rectFill.top += pPageInfo->m_szOne.cy;
				rectFill.DeflateRect (dx, 0, 0, 0);

				CRect rectClipFill = rectFill;
				rectClipFill.NormalizeRect ();
				rectClipFill.IntersectRect (&rectClipFill, &pPageInfo->m_rectPageItems);

				if (rectClipFill.Width () > 0)
				{
					CBrush brBackground (m_clrPrintBG);
					pDC->FillRect (rectClipFill, &brBackground);

					// draw group underline
					if (pItem->IsGroup ())
					{
						CRect rectUnderline = rectClipFill;
						rectUnderline.top = rectUnderline.bottom;
						rectUnderline.InflateRect (0, pPageInfo->m_szOne.cy);

						COLORREF clrOld = pDC->GetBkColor ();
						pDC->FillSolidRect (rectUnderline, m_clrPrintGroup);
						pDC->SetBkColor (clrOld);
					}
				}
			}

			// ---------------
			// draw expandbox:
			// ---------------
			if (pItem->IsGroup () && (!IsSortingMode () || IsGrouping ()))
			{
				CRect rectExpand = rectName;
				rectName.left += m_PrintParams.m_nButtonWidth + dx;
				rectExpand.right = rectName.left;
				rectExpand.DeflateRect (dx, 0, 0, 0);

				CRect rectClipExpand = rectExpand;
				rectClipExpand.NormalizeRect ();
				rectClipExpand.IntersectRect (&rectClipExpand, &pPageInfo->m_rectPageItems);

				if (rectClipExpand.Width () > 0)
				{
					pItem->OnDrawExpandBox (pDC, rectClipExpand);
				}
			}

			// ----------
			// draw name:
			// ----------
			if (rectName.right > rectName.left)
			{
				HFONT hOldFont = NULL;
				if (pItem->IsGroup () && m_hPrinterBoldFont != NULL)
				{
					hOldFont = (HFONT) ::SelectObject (pDC->GetSafeHdc (), m_hPrinterBoldFont);
				}

				pItem->OnPrintName (pDC, rectName);

				if (hOldFont != NULL)
				{
					::SelectObject (pDC->GetSafeHdc (), hOldFont);
				}
			}

			// ------------
			// draw values:
			// ------------
			if (pItem->HasValueField ())
			{
				pItem->OnPrintItems (pDC, pItem->m_Rect);
			}

			// ---------------
			// draw focus rect
			// ---------------
			if (m_bDrawFocusRect &&
				IsFocused () && IsWholeRowSel () && GetCurSel () == pItem)
			{
			}
		}
	}

	return TRUE;
}
//*****************************************************************************
BOOL CBCGPGridCtrl::StartDragColumn (int nItem, CRect rect, 
									 BOOL bDragGroupItem, BOOL bDragFromChooser)
{
	int nLButton = GetSystemMetrics (SM_SWAPBUTTON) ? VK_RBUTTON : VK_LBUTTON;
	if ((GetAsyncKeyState (nLButton) & 0x8000) == 0)
	{
		return FALSE;
	}

	BOOL bHeaderSortClick = !bDragGroupItem && !bDragFromChooser &&
		(m_dwHeaderFlags & BCGP_GRID_HEADER_SORT) != 0;

	if ((m_dwHeaderFlags & BCGP_GRID_HEADER_MOVE_ITEMS) == 0 && !bHeaderSortClick)
	{
		return FALSE;
	}
	
	m_nDraggedColumn = nItem;

	m_bDragGroupItem = bDragGroupItem;
	m_bDragFromChooser = bDragFromChooser;

	RedrawWindow (rect);

	ClientToScreen (&rect);
	m_rectStartDrag = rect;

	SetCapture ();
	return TRUE;
}
//*****************************************************************************
BOOL CBCGPGridCtrl::DragColumn (CPoint ptScreen)
{
	if (m_nDraggedColumn < 0)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	CPoint point = ptScreen;
	ScreenToClient (&point);

	// Allow user limited mouse movement without starting a drag operation:
	BOOL bWasMoved = 
		(abs (m_ptStartDrag.x - point.x) > ::GetSystemMetrics (SM_CXDRAG) ||
		 abs (m_ptStartDrag.y - point.y) > ::GetSystemMetrics (SM_CYDRAG));

	if (!m_bDragHeaderItems ||
		!bWasMoved && m_pWndHeaderDrag == NULL)
	{
		SetCursor (::LoadCursor (NULL, IDC_ARROW));
		return FALSE;
	}

	BOOL bToGroupByBox = FALSE;
	BOOL bNoDropToHeader = FALSE;
	BOOL bDragOverChooser = FALSE;

	if (m_pWndHeaderDrag == NULL)
	{
		m_pWndHeaderDrag = new CBCGPHeaderItemDragWnd;
		ASSERT_VALID (m_pWndHeaderDrag);

		CRect rectItem = m_rectStartDrag;

		BOOL bCreated = m_pWndHeaderDrag->Create (this, m_nDraggedColumn);
		if (!bCreated)
		{
			TRACE (_T("CBCGPHeaderItemDragWnd::DragColumn: Unable to create m_pWndHeaderDrag\n"));

			if (::GetCapture () == GetSafeHwnd ())
			{
				ReleaseCapture ();
			}
			return FALSE;
		}
	}
	else
	{
		BOOL bDrop = m_pWndHeaderDrag->m_bDrop;

		m_pWndHeaderDrag->m_bDrop = FALSE;

		// Is over the column chooser?
		if (IsColumnsChooserVisible ())
		{
			CRect rectChooser;
			m_pColumnChooser->GetWindowRect (rectChooser);

			bDragOverChooser = rectChooser.PtInRect (ptScreen);
		}

		CRect rectHeader = m_rectHeader;
		ClientToScreen (&rectHeader);

		// Is over the grid header?
		BOOL bDragOverHeader = FALSE;
		int iDropIndex = -1;
		if (rectHeader.PtInRect (ptScreen))
		{
			bDragOverHeader = TRUE;
			iDropIndex = GetColumnsInfo ().HitTestColumn (point);

			if (iDropIndex == -1 && GetColumnsInfo ().GetColumnCount (TRUE) > 0)
			{
				bDragOverHeader = FALSE;
			}
		}

		if (bDragOverChooser)
		{
			// ---------------------------------
			// Dragging over the column chooser.
			// Hide the drop indicator:
			// ---------------------------------
			if (m_pWndHeaderDrop != NULL)
			{
				m_pWndHeaderDrop->Hide ();
			}

			m_pWndHeaderDrag->m_bDrop = TRUE;
		}
		else if (bDragOverHeader)
		{
			// -----------------------------------
			// Dragging over the grid header.
			// Set position of the drop indicator:
			// -----------------------------------
			int nPosDraggedColumn = GetColumnsInfo ().IndexToOrder(m_nDraggedColumn);
			int nPosDrop = iDropIndex >= 0 ? GetColumnsInfo ().IndexToOrder (iDropIndex) : -1;

			int x = 0;

			if (nPosDrop >= 0 && nPosDrop < GetColumnsInfo ().GetColumnCount (TRUE))
			{
				CRect rectItem;
				GetColumnsInfo ().GetColumnRect (iDropIndex, rectItem);

				if (nPosDrop == nPosDraggedColumn || nPosDraggedColumn == -1)
				{
					if (point.x > rectItem.CenterPoint ().x)
					{
						x = rectItem.right;
						nPosDrop ++;
					}
					else
					{
						x = rectItem.left;
					}
				}
				else if (nPosDrop > nPosDraggedColumn)
				{
					x = rectItem.right;
					nPosDrop ++;
				}
				else
				{
					x = rectItem.left;
				}
			}
			else
			{
				nPosDrop = 0; // all columns are hidden - insert first
			}

			if (nPosDrop == nPosDraggedColumn && !m_bDragGroupItem)
			{
				// position has not changed - hide drop indicator
				if (m_pWndHeaderDrop != NULL)
				{
					m_pWndHeaderDrop->Hide ();
				}
			}
			else if (!CanDropColumn (nPosDrop, nPosDraggedColumn))
			{
				// can't drop here - hide drop indicator
				if (m_pWndHeaderDrop != NULL)
				{
					m_pWndHeaderDrop->Hide ();
				}

				bNoDropToHeader = TRUE;
			}
			else
			{
				CPoint ptDrop (x, m_rectHeader.CenterPoint ().y);
				ClientToScreen (&ptDrop);

				ShowDropIndicator (ptDrop);
			}

			m_pWndHeaderDrag->m_bDrop = TRUE;
		}
		else 
		{
			CPoint point = ptScreen;
			ScreenToClient (&point);

			CRect rectClient;
			GetClientRect (rectClient);

			CRect rectGroupByBox = rectClient;
			rectGroupByBox.bottom = min (rectGroupByBox.top + m_nGroupByBoxHeight, rectClient.bottom);

			if (rectGroupByBox.PtInRect (point))
			{
				// -----------------------------------
				// Dragging over the group-by-box.
				// Set position of the drop indicator:
				// -----------------------------------
				CPoint ptDrop;

				int nPos = GetGroupByBoxDropIndex (point, &ptDrop);
				if (nPos >= 0)
				{
					BOOL bPosChanged = TRUE;

					int nPosDraggedColumn = GetColumnsInfo ().GetGroupColumnPos (m_nDraggedColumn);
					if (nPosDraggedColumn >= 0)
					{
						bPosChanged = (nPosDraggedColumn != nPos && 
							nPosDraggedColumn + 1 != nPos);
					}

					if (!m_bDragGroupItem && nPosDraggedColumn >= 0 ||
						!OnDropToGroupByBox (m_nDraggedColumn))
					{
						// column is already in the group-by-box - do nothing
						if (m_pWndHeaderDrop != NULL)
						{
							m_pWndHeaderDrop->Hide ();
						}
					}
					else if (bPosChanged)
					{
						ClientToScreen (&ptDrop);
						ShowDropIndicator (ptDrop);
					}
					else
					{
						// position has not changed - hide drop indicator
						if (m_pWndHeaderDrop != NULL)
						{
							m_pWndHeaderDrop->Hide ();
						}
					}
				}

				bToGroupByBox = TRUE;
				m_pWndHeaderDrag->m_bDrop = TRUE;
			}
			else
			{
				// --------------------
				// Dragging away.
				// Hide drop indicator:
				// --------------------
				if (m_pWndHeaderDrop != NULL)
				{
					m_pWndHeaderDrop->Hide ();
				}
			}
		}

		CRect rectWnd;
		m_pWndHeaderDrag->GetWindowRect (&rectWnd);

		m_pWndHeaderDrag->SetWindowPos
			(&wndTop,  
			ptScreen.x - rectWnd.Width () / 2, 
			ptScreen.y - rectWnd.Height () + 6,
			-1, -1, SWP_NOSIZE | SWP_SHOWWINDOW | SWP_NOACTIVATE);

		if (bDrop != m_pWndHeaderDrag->m_bDrop)
		{
			m_pWndHeaderDrag->RedrawWindow ();
		}
	}

	// -----------
	// Set cursor:
	// -----------
	BOOL bCursorSet = FALSE;

	if (m_bDragFromChooser)
	{
		if (m_pWndHeaderDrop == NULL ||
			!(m_pWndHeaderDrop->GetStyle () & WS_VISIBLE))
		{
			SetCursor (m_hcurNoDropColumn);
			bCursorSet = TRUE;
		}
	}
	else if (m_pWndHeaderDrag->m_bDrop &&
		(m_pWndHeaderDrop == NULL || !(m_pWndHeaderDrop->GetStyle () & WS_VISIBLE)) &&
		!m_bDragGroupItem && bToGroupByBox)
	{
		// column is already in the group-by-box
		SetCursor (m_hcurNoDropColumn);
		bCursorSet = TRUE;
	}
	else if (!m_pWndHeaderDrag->m_bDrop)
	{
		if (CanHideColumn (m_nDraggedColumn))
		{
			SetCursor (m_hcurDeleteColumn);
			bCursorSet = TRUE;
		}
		else
		{
			SetCursor (m_hcurNoDropColumn);
			bCursorSet = TRUE;
		}
	}

	if (!bCursorSet && bNoDropToHeader)
	{
		SetCursor (m_hcurNoDropColumn);
		bCursorSet = TRUE;
	}

	if (!bCursorSet && bDragOverChooser && 
		!CanDropColumnToColumnChooser (m_nDraggedColumn))
	{
		SetCursor (m_hcurNoDropColumn);
		bCursorSet = TRUE;
	}

	if (!bCursorSet)
	{
		SetCursor (::LoadCursor (NULL, IDC_ARROW));
	}

	return TRUE;
}
//*****************************************************************************
BOOL CBCGPGridCtrl::StopDragColumn (CPoint point, BOOL bUpdate)
{
	CWaitCursor wait;

	if (m_nDraggedColumn < 0)
	{
		ASSERT (m_pWndHeaderDrag == NULL);
		return FALSE;
	}

	BOOL bWasMoved = (m_pWndHeaderDrag != NULL);

	if (m_pWndHeaderDrag != NULL)
	{
		m_pWndHeaderDrag->DestroyWindow ();
		m_pWndHeaderDrag = NULL;
	}

	if (m_pWndHeaderDrop != NULL)
	{
		m_pWndHeaderDrop->DestroyWindow ();
		m_pWndHeaderDrop = NULL;
	}

	CRect rect;
	GetColumnsInfo ().GetColumnRect (m_nDraggedColumn, rect);

	int nDraggedColumn = m_nDraggedColumn;
	m_nDraggedColumn = -1;

	InvalidateRect (rect);
	UpdateWindow ();

	if (::GetCapture () == GetSafeHwnd ())
	{
		ReleaseCapture ();
	}

	if (!bUpdate)
	{
		return TRUE;
	}

	BOOL bHeaderSortClick = !m_bDragFromChooser && !m_bDragGroupItem &&
		(m_dwHeaderFlags & BCGP_GRID_HEADER_SORT) != 0 &&
		(m_dwHeaderFlags & BCGP_GRID_HEADER_MOVE_ITEMS) == 0;

	if (!m_bDragHeaderItems && !bHeaderSortClick)
	{
		OnHeaderColumnClick (nDraggedColumn);
		ToggleSortColumn (nDraggedColumn);
		return TRUE;
	}

	// ---------------------------
	// Drop to the column chooser:
	// ---------------------------
	if (IsColumnsChooserVisible () && m_bDragHeaderItems && 
		CanDropColumnToColumnChooser (nDraggedColumn))
	{
		CRect rectChooser;
		m_pColumnChooser->GetWindowRect (rectChooser);

		CPoint ptScreen = point;
		ClientToScreen (&ptScreen);

		if (rectChooser.PtInRect (ptScreen))
		{
			if (!m_bDragFromChooser)
			{
				if (m_bDragGroupItem)
				{
					// ------------------------
					// Stop group by the column
					// ------------------------
					RemoveGroupColumnByVal (nDraggedColumn);
				}

				else
				{
					// -----------
					// Hide column
					// -----------
					GetColumnsInfo ().SetColumnVisible (nDraggedColumn, FALSE);
					AdjustLayout ();
					UpdateColumnsChooser ();
				}
			}

			return TRUE;
		}
	}

	// -------------------------
	// Drop to the group-by-box:
	// -------------------------
	int nPos = GetGroupByBoxDropIndex (point);
	if (nPos >= 0 && m_bDragHeaderItems)
	{
		BOOL bPosChanged = TRUE;

		int nPosDraggedColumn = GetColumnsInfo ().GetGroupColumnPos (nDraggedColumn);
		if (nPosDraggedColumn >= 0)
		{
			bPosChanged = (nPosDraggedColumn != nPos && 
				nPosDraggedColumn + 1 != nPos);
		}

		if (!m_bDragGroupItem && nPosDraggedColumn >= 0 ||
			!OnDropToGroupByBox (nDraggedColumn))
		{
			// column is already in the group-by-box - do nothing
		}
		else if (!bPosChanged)
		{
			if (m_bDragGroupItem && !bWasMoved)
			{
				OnHeaderColumnClick (nDraggedColumn);
				ToggleSortColumn (nDraggedColumn);
			}
		}
		else
		{
			if (!m_bDragFromChooser && !m_bDragGroupItem &&
				OnHideInsertedGroupColumn (nDraggedColumn))
			{
				GetColumnsInfo ().SetColumnVisible (nDraggedColumn, FALSE);
				UpdateColumnsChooser ();
			}

			InsertGroupColumn (nPos, nDraggedColumn);
			m_pSetSelItem = m_bVirtualMode ? NULL : GetCurSelItem ();
			SetCurSel (m_idActive);
			m_pSetSelItem = NULL;
			AdjustLayout ();
		}
	}
	else
	{
		// Is over the grid header?
		BOOL bDragOverHeader = FALSE;
		int nColumn = -1;

		if (m_rectHeader.PtInRect (point))
		{
			bDragOverHeader = TRUE;
			nColumn = GetColumnsInfo ().HitTestColumn (point);

			if (nColumn == -1 && GetColumnsInfo ().GetColumnCount (TRUE) > 0)
			{
				bDragOverHeader = FALSE;
			}
		}

		// ------------------------
		// Drop to the grid header:
		// ------------------------
		if (bDragOverHeader)
		{
			BOOL bWasHidden = !GetColumnsInfo ().GetColumnVisible (nDraggedColumn);

			// ---------------------------------------
			// Get new and old positions of the column
			// ---------------------------------------
			int nPosDraggedColumn = GetColumnsInfo ().IndexToOrder(nDraggedColumn);
			int nPosDrop = nColumn >= 0 ? GetColumnsInfo ().IndexToOrder (nColumn) : -1;

			if (bWasHidden && nColumn == -1)
			{
				nPosDrop = 0;//GetColumnsInfo ().GetColumnCount (TRUE);
			}
			else if (nColumn >= 0 && (m_bDragGroupItem || m_bDragFromChooser))
			{
				CRect rectItem;
				GetColumnsInfo ().GetColumnRect (nColumn, rectItem);

				if (point.x > rectItem.CenterPoint ().x && nPosDraggedColumn == -1)
				{
					nPosDrop++; // insert after
				}
			}

			// ----------------------
			// Check if can drop here
			// ----------------------
			if (nPosDrop >= 0 && m_bDragHeaderItems)
			{
				if (!CanDropColumn (nPosDrop, nPosDraggedColumn))
				{
					// Can't drop here - do nothing
					return FALSE;
				}
			}

			BOOL bNeedUpdate = FALSE;

			if (m_bDragGroupItem)
			{
				// ------------------------
				// Stop group by the column
				// ------------------------
				RemoveGroupColumnByVal (nDraggedColumn);
				bNeedUpdate = FALSE;
			}

			if (nColumn == nDraggedColumn)	// position was not changed
			{
				if (!m_bDragGroupItem && !m_bDragFromChooser && !bWasMoved)
				{
					OnHeaderColumnClick (nDraggedColumn);
					if ((m_dwHeaderFlags & BCGP_GRID_HEADER_SORT) != 0)
					{
						ToggleSortColumn (nDraggedColumn);
					}
					else if ((m_dwHeaderFlags & BCGP_GRID_HEADER_SELECT) != 0)
					{
						SelectColumn (nDraggedColumn);
					}
				}
			}
			else if (nPosDrop >= 0 && m_bDragHeaderItems)
			{
				// --------------------------------------
				// Insert (show) previously hidden column
				// --------------------------------------
				if (bWasHidden)
				{
					GetColumnsInfo ().SetColumnVisible (nDraggedColumn, TRUE);
					UpdateColumnsChooser ();

					bNeedUpdate = TRUE;
				}

				// -----------------------------
				// Change position of the column
				// -----------------------------
				m_pSetSelItem = m_bVirtualMode ? NULL : GetCurSelItem ();
				SetCurSel (m_idActive);
				m_pSetSelItem = NULL;

				if (!GetColumnsInfo ().ChangeColumnOrder (nPosDrop, nDraggedColumn))
				{
					return FALSE;
				}
				bNeedUpdate = TRUE;
			}

			if (bNeedUpdate && GetSafeHwnd () != NULL)
			{
				SetRebuildTerminalItems ();
				ReposItems ();
				
				AdjustLayout ();
			}
		}
		
		// -----------------------
		// Drop away. Hide column:
		// -----------------------
		else
		{
			if (!m_bDragHeaderItems)
			{
				// Do nothing
				return FALSE;
			}

			if (m_bDragFromChooser)
			{
				// Do nothing
				return FALSE;
			}

			if (!CanHideColumn (nDraggedColumn))
			{
				// Do nothing
				return FALSE;
			}

			if (m_bDragGroupItem)
			{
				// Stop group by the column
				RemoveGroupColumnByVal (nDraggedColumn);
				return TRUE;
			}

			GetColumnsInfo ().SetColumnVisible (nDraggedColumn, FALSE);
			m_pSetSelItem = m_bVirtualMode ? NULL : GetCurSelItem ();
			SetCurSel (m_idActive);
			m_pSetSelItem = NULL;
			AdjustLayout ();
			UpdateColumnsChooser ();
		}
			
	}

	return TRUE;
}
//*******************************************************************************
void CBCGPGridCtrl::ShowDropIndicator (CPoint pt)
{
	if (m_pWndHeaderDrop == NULL)
	{
		m_pWndHeaderDrop = new CBCGPHeaderItemDropWnd;
		m_pWndHeaderDrop->Create (m_rectHeader.Height ());
	}

	m_pWndHeaderDrop->Show (pt);
}
//*******************************************************************************
CBCGPGridRow* CBCGPGridCtrl::CreateRowFromArchive (CArchive& archive, int nRow)
{
	CBCGPGridRow* pNewRow = NULL;
	archive >> pNewRow;	// Polymorphic reconstruction of persistent object

	if (pNewRow != NULL)
	{
		ASSERT_VALID (pNewRow);
		pNewRow->m_nIdRow = nRow;
	}
	
	return pNewRow;
}
//*******************************************************************************
CBCGPGridItem* CBCGPGridCtrl::CreateItemFromArchive (CArchive& archive, int nRow, int nColumn)
{
	CBCGPGridItem* pNewItem = NULL;
	archive >> pNewItem;	// Polymorphic reconstruction of persistent object

	if (pNewItem != NULL)
	{
		ASSERT_VALID (pNewItem);
		
		if (!pNewItem->OnCreate (nRow, nColumn))
		{
			delete pNewItem;
			return NULL;
		}

		pNewItem->m_bIsChanged = TRUE;
	}
	
	return pNewItem;
}
//*******************************************************************************
BOOL CBCGPGridCtrl::ReplaceItemFromArchive(CArchive& archive, CBCGPGridRow* pRow, int nColumn, BOOL bTestMode)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pRow);

	CBCGPGridItem* pNewItem = CreateItemFromArchive (archive, pRow->GetRowId (), nColumn);

	if (pNewItem == NULL)
	{
		return FALSE;
	}

	ASSERT_VALID (pNewItem);

	BOOL bCanReplace = pRow->CanReplaceItem (nColumn, pNewItem);

	if (bTestMode || !bCanReplace)
	{
		delete pNewItem;
		return bCanReplace;
	}

	if (!pRow->ReplaceItem (nColumn, pNewItem, FALSE, TRUE))
	{
		delete pNewItem;
		return FALSE;
	}

	m_pSelItem = NULL;
	m_pLastSelItem = NULL;

	pNewItem->SetItemChanged ();
	InvalidateRect (pNewItem->GetRect ());

	return TRUE;
}
//*******************************************************************************
BOOL CBCGPGridCtrl::AddItemFromArchive(CArchive& archive, CBCGPGridRow* pRow, int nColumn, BOOL /*bTestMode*/)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pRow);
	ASSERT (nColumn >= 0);
	ASSERT (nColumn < GetColumnCount ());

	CBCGPGridItem* pNewItem = CreateItemFromArchive (archive, pRow->GetRowId (), nColumn);

	if (pNewItem == NULL)
	{
		return FALSE;
	}

	ASSERT_VALID (pNewItem);

	if (pRow->GetItemCount () == nColumn)
	{
		pRow->AddItem (pNewItem);
	}
	else
	{
		ASSERT (FALSE);
		delete pNewItem;
		return FALSE;
	}

	return TRUE;
}
//*******************************************************************************
BOOL CBCGPGridCtrl::CreateSerializeManager ()
{
	ASSERT_VALID (this);
	m_pSerializeManager = new CBCGPGridSerializeManager (this);
	return (m_pSerializeManager != NULL);
}
//*******************************************************************************
BOOL CBCGPGridCtrl::StartDragItems (CPoint point)
{
	ASSERT_VALID (this);

	m_bDragDrop = FALSE;
	m_bDragEnter = FALSE;
	m_bDragRowHeader = FALSE;
	m_idDragFrom = CBCGPGridItemID (0 ,0);
	m_idDropTo = CBCGPGridItemID (0, 0);
	m_DropEffect = DROPEFFECT_NONE;
	m_DropArea = DropAt;

	m_bClickDrag = TRUE;
	m_ptClickOnce = point;

	return TRUE;
}
//*******************************************************************************
void CBCGPGridCtrl::StopDragItems ()
{
	m_bDragDrop = FALSE;
	m_bDragEnter = FALSE;
	m_bDragRowHeader = FALSE;
	m_idDragFrom = CBCGPGridItemID (0, 0);
	m_idDropTo = CBCGPGridItemID (0, 0);
	m_DropEffect = DROPEFFECT_NONE;
	m_DropArea = DropAt;

	m_bClickDrag = FALSE;
	m_ptClickOnce = CPoint (0, 0);

	HideDragFrame ();
	HideDragInsertMarker ();
}
//*******************************************************************************
BOOL CBCGPGridCtrl::DragItems (CPoint point)
{
	if (m_bDragDrop)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	point = m_ptClickOnce;

	//-----------------
	// Set m_idDragFrom
	//-----------------
	CRect rectRowHeader;
	CBCGPGridRow* pRow = HitTestRowHeader (point, rectRowHeader);
	
	if (pRow != NULL)
	{
		m_bDragRowHeader = TRUE;
		m_idDragFrom = CBCGPGridItemID (pRow->GetRowId (), 0);
	}
	else
	{
		CBCGPGridItem* pHitItem = NULL;
		HitTest (point, m_idDragFrom, pHitItem, NULL, TRUE);
	}

	if (m_idDragFrom.IsNull ())
	{
		return FALSE;
	}
	
	//-------
	// Notify
	//-------
	if (OnBeginDrag (point))
	{
		return TRUE;	// Custom Drag-and-Drop
	}

	if (m_pSerializeManager == NULL)
	{
		return FALSE;
	}

	ASSERT_VALID (m_pSerializeManager);

	//----------------------
	// Start OLE drag'n'drop
	//----------------------
	COleDataSource* pSrcItem = new COleDataSource();
	m_bDragDrop = TRUE;

	try
	{
		if (!m_pSerializeManager->PrepareDataFromSelection ())
		{
			m_bDragDrop = FALSE;
			return FALSE;
		}
		
		CSharedFile globFile;

		if (!m_pSerializeManager->SerializeTo (globFile))
		{
			m_bDragDrop = FALSE;

			if (pSrcItem != NULL)
			{
				pSrcItem->InternalRelease();
				pSrcItem = NULL;
			}
			
			return FALSE;
		}

		pSrcItem->CacheGlobalData (m_pSerializeManager->GetClipboardFormat (), globFile.Detach());
	}
	catch (COleException* pEx)
	{
		TRACE(_T("CBCGPGridCtrl::DragItems. OLE exception: %x\r\n"),
			pEx->m_sc);
		pEx->Delete ();
		
		m_bDragDrop = FALSE;

		if (pSrcItem != NULL)
		{
			pSrcItem->InternalRelease();
			pSrcItem = NULL;
		}

		return FALSE;
	}
	catch (CNotSupportedException *pEx)
	{
		TRACE(_T("CBCGPGridCtrl::DragItems. \"Not Supported\" exception\r\n"));
		pEx->Delete ();
		
		m_bDragDrop = FALSE;

		if (pSrcItem != NULL)
		{
			pSrcItem->InternalRelease();
			pSrcItem = NULL;
		}
		
		return FALSE;
	}

	if (!m_bDragDrop)
	{
		if (pSrcItem != NULL)
		{
			pSrcItem->InternalRelease();
			pSrcItem = NULL;
		}
		
		return FALSE;
	}

	DROPEFFECT dropEffect = pSrcItem->DoDragDrop (DROPEFFECT_COPY | DROPEFFECT_MOVE/*, NULL, &m_DropSource*/);

	//-----------------------
	// Finish OLE drag'n'drop
	//-----------------------
	CPoint ptEnd;
	GetCursorPos (&ptEnd);
	ScreenToClient (&ptEnd);

	BOOL bUpdate = FALSE;
	
	if (dropEffect != DROPEFFECT_NONE)
	{
		if (dropEffect == DROPEFFECT_MOVE)
		{
			// m_DropEffect is DROPEFFECT_NONE when content is already dropped to this grid
			// m_idDropTo is null when content is dropped to another application
			if (m_DropEffect != DROPEFFECT_NONE)
			{
				m_pSerializeManager->ClearPreviousSelection (FALSE);
				bUpdate = TRUE;
			}
		}
	}

	StopDragItems ();
	m_pSerializeManager->CleanUp ();

	if (bUpdate)
	{
		RedrawWindow ();
	}
	
	pSrcItem->InternalRelease();
	return TRUE;
}
//*******************************************************************************
void CBCGPGridCtrl::ShowDragFrame ()
{
	if (m_pSerializeManager == NULL)
	{
		return;
	}

	if (m_pSerializeManager->m_ClipboardFormatType != CBCGPGridSerializeManager::CF_Items)
	{
		return;
	}

	CBCGPGridRange range;
	if (!m_pSerializeManager->GetBoundingRange (range, m_pSerializeManager->GetDropOffset (m_idDragFrom, m_idDropTo)))
	{
		return;
	}

	CRect rect = GetRect (range);

	if (!m_rectDragFrame.EqualRect (&rect))
	{
		m_rectDragFrame = rect;
		
		InvalidateRect (m_rectDragFrame);
	}
}
//*******************************************************************************
void CBCGPGridCtrl::HideDragFrame ()
{
	InvalidateRect (m_rectDragFrame);
	m_rectDragFrame.SetRectEmpty ();
}
//*******************************************************************************
void CBCGPGridCtrl::ShowDragInsertMarker ()
{
	if (m_pSerializeManager == NULL)
	{
		return;
	}

	if (m_pSerializeManager->m_ClipboardFormatType != CBCGPGridSerializeManager::CF_Rows)
	{
		return;
	}

 	CBCGPGridRow* pRow = GetRow (m_idDropTo.m_nRow);
 	
	CRect rect = GetListRect (); // drop first
	rect.bottom = rect.top + 1;

	if (pRow != NULL && !pRow->GetRect ().IsRectEmpty ())
	{
		rect = pRow->GetRect ();

		if (m_DropArea == DropBefore)
		{
			rect.bottom = rect.top;
			rect.bottom = rect.top + 1;
		}
		else // DropAfter
		{
			rect.top = rect.bottom;
			rect.bottom = rect.top + 1;
		}
	}

	rect.InflateRect (0, 2);

	if (!m_rectDragMarker.EqualRect (&rect))
	{
		m_rectDragMarker = rect;
		
		InvalidateRect (m_rectDragMarker);
	}
}
//*******************************************************************************
void CBCGPGridCtrl::HideDragInsertMarker ()
{
	InvalidateRect (m_rectDragMarker);
	m_rectDragMarker.SetRectEmpty ();
}
//*******************************************************************************
DROPEFFECT CBCGPGridCtrl::OnDragEnter(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point)
{
	m_bDragEnter = TRUE;
	return OnDragOver (pDataObject, dwKeyState, point);
}
//*******************************************************************************
void CBCGPGridCtrl::OnDragLeave()
{
	m_bDragEnter = FALSE;
	HideDragFrame ();
	HideDragInsertMarker ();
}
//*******************************************************************************
DROPEFFECT CBCGPGridCtrl::OnDragOver(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point)
{
	if (!DoDragOver (pDataObject, dwKeyState, point))
	{
		m_DropEffect = DROPEFFECT_NONE;
	}

	if (m_DropEffect != DROPEFFECT_COPY && m_DropEffect != DROPEFFECT_MOVE)
	{
		//--------------------
		// Hide drag indicator
		//--------------------
		HideDragFrame ();
		HideDragInsertMarker ();
		RedrawWindow ();
	}
	else
	{
		//--------------------
		// Draw drag indicator
		//--------------------
		ShowDragFrame ();
		ShowDragInsertMarker ();
		RedrawWindow ();
	}

	return m_DropEffect;
}
//*******************************************************************************
BOOL CBCGPGridCtrl::OnDrop(COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point)
{
	OnDragLeave ();

	BOOL bResult = DoDrop(pDataObject, dropEffect, point);

	StopDragItems ();

	return bResult;
}
//*******************************************************************************
DROPEFFECT CBCGPGridCtrl::OnDragScroll(DWORD /*dwKeyState*/, CPoint point)
{
	CRect rectList = GetListRect ();
	BOOL bScroll = FALSE;
	if (rectList.bottom - point.y < m_nRowHeight)
	{
		// scroll down
		OnBeforeDragScroll ();
		OnVScroll (SB_LINEDOWN, 0, NULL);
		bScroll = TRUE;
	}
	else if (point.y - rectList.top < m_nRowHeight)
	{
		// scroll up
		OnBeforeDragScroll ();
		OnVScroll (SB_LINEUP, 0, NULL);
		bScroll = TRUE;
	}
	else if (rectList.right - point.x < m_nRowHeight && !m_bDragRowHeader) 
	{
		// scroll right
		OnBeforeDragScroll ();
		OnHScroll (SB_LINEDOWN, 0, NULL);
		bScroll = TRUE;
	}
	else if (point.x - (rectList.left + 1) < m_nRowHeight && !m_bDragRowHeader)
	{
		// scroll left
		OnBeforeDragScroll ();
		OnHScroll (SB_LINEUP, 0, NULL);
		bScroll = TRUE;
	}

	if (bScroll)
	{
		ShowDragFrame ();
		ShowDragInsertMarker ();
	}

	return DROPEFFECT_NONE;
}
//*******************************************************************************
void CBCGPGridCtrl::OnBeforeDragScroll ()
{
	m_rectDragFrame.SetRectEmpty ();
}
//*******************************************************************************
BOOL CBCGPGridCtrl::DoDragOver(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point)
{
	ASSERT_VALID (this);

	if (IsReadOnly () || m_pSerializeManager == NULL)
	{
		return FALSE;
	}
	
	ASSERT_VALID (m_pSerializeManager);
	
	if (!pDataObject->IsDataAvailable (m_pSerializeManager->GetClipboardFormat ()) || IsReadOnly ())
	{
		return FALSE;
	}
	
	CBCGPGridItemID idOldDropTo = m_idDropTo;
	
	//--------
	// HitTest
	//--------
	CBCGPGridRow* pHitRow = NULL;
	if (m_bDragRowHeader)
	{
		CRect rectRowHeader;
		pHitRow = HitTestRowHeader (point, rectRowHeader);
		if (pHitRow != NULL)
		{
			m_idDropTo = CBCGPGridItemID (pHitRow->GetRowId (), 0);
		}
	}
	if (pHitRow == NULL)
	{
		CBCGPGridItem* pHitItem = NULL;
		pHitRow = HitTest (point, m_idDropTo, pHitItem, NULL, TRUE);
	}
	
	if (m_bDragRowHeader || IsWholeRowSel ())
	{
		m_idDropTo.m_nColumn = 0;
	}
	
	if (pHitRow == NULL)
	{
		return FALSE;
	}

	DropArea dropArea = HitTestDropArea (point, m_idDropTo, pHitRow);
	
	DROPEFFECT dropEffect = (dwKeyState & MK_CONTROL) ? DROPEFFECT_COPY : DROPEFFECT_MOVE;

	if (m_idDropTo == idOldDropTo && dropArea == m_DropArea) // no changes - return saved drop effect
	{
		dropEffect = m_DropEffect;
		return TRUE;
	}
	
	//---------------
	// Get drop range
	//---------------
	CFile* pFile = pDataObject->GetFileData (m_pSerializeManager->GetClipboardFormat ());
	if (pFile == NULL)
	{
		return FALSE;
	}

	// m_bDragDrop is FALSE when content is dragged from another application
	if (!m_bDragDrop)
	{
		m_idDragFrom.SetNull ();
	}

	BOOL bCanDrop = 
		m_pSerializeManager->SerializeFrom (*pFile) &&
		m_pSerializeManager->CanDrop (m_idDropTo, m_idDragFrom, (dropEffect == DROPEFFECT_MOVE) && m_bDragDrop, dropArea == DropAfter);

	delete pFile;

	if (!bCanDrop)
	{
		return FALSE;
	}
	
	m_DropEffect = dropEffect;
	m_DropArea = dropArea;
	
	return TRUE;
}
//*******************************************************************************
BOOL CBCGPGridCtrl::DoDrop(COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint /*point*/)
{
	ASSERT_VALID (this);

	if (m_pSerializeManager == NULL)
	{
		return FALSE;
	}
	
	ASSERT_VALID (m_pSerializeManager);

	if (IsReadOnly () || !pDataObject->IsDataAvailable (m_pSerializeManager->GetClipboardFormat ()))
	{
		return FALSE;
	}
	
	if (dropEffect != DROPEFFECT_MOVE && dropEffect != DROPEFFECT_COPY)
	{
		return FALSE;
	}

	ASSERT (dropEffect == m_DropEffect);

	//--------------------------
	// Update previous selection
	//--------------------------
	InvalidateRect (OnGetSelectionRect ());
	for (int i= 0; i < GetSelectionCount (); i++)
	{
		CBCGPGridRange range;
		if (GetSelection (i, range))
		{
			CRect rect = GetRect (range);
			InvalidateRect (rect);
		}
	}

	//-----
	// Drop
	//-----
	// m_bDragDrop is FALSE when content is dragged from another application
	if (!m_bDragDrop)
	{
		m_idDragFrom.SetNull ();
	}

	BOOL bResult =
		m_pSerializeManager->Drop (m_idDropTo, m_idDragFrom,
			(dropEffect == DROPEFFECT_MOVE) && m_bDragDrop, m_DropArea == DropAfter);

	if (!m_bDragDrop)
	{
		m_pSerializeManager->CleanUp ();
	}

	return bResult;
}
//*******************************************************************************
CBCGPGridCtrl::DropArea CBCGPGridCtrl::HitTestDropArea (CPoint point, CBCGPGridItemID /*idDropTo*/,
														CBCGPGridRow* pHitRow)
{
	ASSERT_VALID (pHitRow);

	if (m_pSerializeManager != NULL && 
		m_pSerializeManager->m_ClipboardFormatType == CBCGPGridSerializeManager::CF_Rows)
	{
		CRect rect = pHitRow->GetRect ();
		return (point.y <= rect.top + rect.Height () / 2) ? DropBefore : DropAfter;
	}
	
	return DropAt;
}

#ifndef _BCGPGRID_STANDALONE

BOOL CBCGPGridCtrl::LoadState (LPCTSTR lpszProfileName, int nIndex)
{
	CString strProfileName;

#ifndef _BCGSUITE_
	if (g_pWorkspace == NULL ||
		AfxGetApp () == NULL || AfxGetApp ()->m_pszRegistryKey == NULL)
	{
		return FALSE;
	}

	strProfileName = g_pWorkspace->GetRegSectionPath (strGridsProfile);
#else
	CWinAppEx* pApp = DYNAMIC_DOWNCAST(CWinAppEx, AfxGetApp ());
	if (pApp == NULL || pApp->m_pszRegistryKey == NULL)
	{
		return FALSE;
	}

	strProfileName = pApp->GetRegSectionPath (strGridsProfile);
#endif

	if (lpszProfileName != NULL)
	{
		strProfileName += lpszProfileName;
	}

	CString strSection;
	strSection.Format (REG_SECTION_FMT, strProfileName, nIndex);

	CBCGPRegistrySP regSP;
	CBCGPRegistry& reg = regSP.Create (FALSE, TRUE);

	if (!reg.Open (strSection))
	{
		return FALSE;
	}

	//-----------------------------
	// Load column order and width:
	//-----------------------------
	CArray<int, int> arColumns;
	CArray<int, int> arColumnWidth;

	reg.Read (_T ("Columns"), arColumns);
	reg.Read (_T ("ColumnWidth"), arColumnWidth);

	if (arColumns.GetSize () == arColumnWidth.GetSize () &&
		arColumnWidth.GetSize () == GetColumnsInfo ().m_arrColumns.GetSize ())
	{
		for (int nColumn = 0; nColumn < GetColumnsInfo ().m_arrColumns.GetSize (); nColumn++)
		{
			CBCGPGridColumnsItem* pColumn = GetColumnsInfo ().m_arrColumns[nColumn];
			ASSERT_VALID (pColumn);

			pColumn->m_bVisible = (BOOL) arColumns [nColumn];
			pColumn->m_nDefaultSize = arColumnWidth [nColumn];
		}
	}
	else
	{
		return FALSE;
	}

	GetColumnsInfo ().m_arrColumnOrder.RemoveAll ();
	reg.Read (_T ("ColumnOrder"), GetColumnsInfo ().m_arrColumnOrder);

	//---------------------
	// Load groupping info:
	//---------------------
	GetColumnsInfo ().m_lstGroupingColumns.RemoveAll ();
	reg.Read (_T ("Groupping"), GetColumnsInfo ().m_lstGroupingColumns);

	//-------------------------
	// Load field chooser info:
	//-------------------------
	reg.Read (_T ("LastFieldChooserPos"), m_rectColumnChooser);
	reg.Read (_T ("IsFieldChooserVisible"), m_bColumnChooserVisible);
	reg.Read (_T ("IsFieldChooserThemed"), m_bFieldChooserThemed);

	if (m_bFieldChooserThemed && m_pColumnChooser != NULL)
	{
		m_pColumnChooser->EnableVisualManagerStyle (m_bFieldChooserThemed);
	}

	//-------------------
	// Load sorting info:
	//-------------------
	GetColumnsInfo ().m_mapSortColumn.RemoveAll ();
	reg.Read (_T ("SortColumns"), GetColumnsInfo ().m_mapSortColumn);
	GetColumnsInfo ().OnColumnsOrderChanged ();

	SetRebuildTerminalItems ();
	ReposItems ();

	AdjustLayout ();
	return TRUE;
}
//*************************************************************************************
BOOL CBCGPGridCtrl::SaveState (LPCTSTR lpszProfileName, int nIndex)
{
	CString strProfileName;

#ifndef _BCGSUITE_
	if (g_pWorkspace == NULL ||
		AfxGetApp () == NULL || AfxGetApp ()->m_pszRegistryKey == NULL)
	{
		return FALSE;
	}

	strProfileName = g_pWorkspace->GetRegSectionPath (strGridsProfile);
#else
	CWinAppEx* pApp = DYNAMIC_DOWNCAST(CWinAppEx, AfxGetApp ());
	if (pApp == NULL || pApp->m_pszRegistryKey == NULL)
	{
		return FALSE;
	}

	strProfileName = pApp->GetRegSectionPath (strGridsProfile);
#endif

	if (lpszProfileName != NULL)
	{
		strProfileName += lpszProfileName;
	}


	CString strSection;
	strSection.Format (REG_SECTION_FMT, strProfileName, nIndex);

	CBCGPRegistrySP regSP;
	CBCGPRegistry& reg = regSP.Create (FALSE, FALSE);

	if (!reg.CreateKey (strSection))
	{
		return FALSE;
	}

	//-----------------------------
	// Save column order and width:
	//-----------------------------
	CArray<int, int> arColumns;
	CArray<int, int> arColumnWidth;

	for (int nColumn = 0; nColumn < GetColumnsInfo ().m_arrColumns.GetSize (); nColumn++)
	{
		CBCGPGridColumnsItem* pColumn = GetColumnsInfo ().m_arrColumns[nColumn];
		ASSERT_VALID (pColumn);

		arColumns.Add ((int) pColumn->m_bVisible);
		arColumnWidth.Add (pColumn->m_nDefaultSize);
	}

	reg.Write (_T ("Columns"), arColumns);
	reg.Write (_T ("ColumnWidth"), arColumnWidth);
	reg.Write (_T ("ColumnOrder"), GetColumnsInfo ().m_arrColumnOrder);

	//---------------------
	// Save groupping info:
	//---------------------
	reg.Write (_T ("Groupping"), GetColumnsInfo ().m_lstGroupingColumns);

	//-------------------------
	// Save field chooser info:
	//-------------------------
	reg.Write (_T ("LastFieldChooserPos"), m_rectColumnChooser);
	reg.Write (_T ("IsFieldChooserVisible"), m_bColumnChooserVisible);
	reg.Write (_T ("IsFieldChooserThemed"), m_bFieldChooserThemed);

	//-------------------
	// Save sorting info:
	//-------------------
	reg.Write (_T ("SortColumns"), GetColumnsInfo ().m_mapSortColumn);

	return TRUE;
}
//*****************************************************************************
BOOL CBCGPGridCtrl::ResetState ()
{
	// TODO!
	return TRUE;
}

#endif
void CBCGPGridCtrl::OnTimer(UINT_PTR nIDEvent) 
{
	if (nIDEvent == GRID_CLICKVALUE_TIMER_ID)
	{
		// "Click and hold" event - do not translate click, just select or drag the item.
		KillTimer (GRID_CLICKVALUE_TIMER_ID);
		m_bClickTimer = FALSE;
		m_ptClickOnce = CPoint (0, 0);

		SetCursor (::LoadCursor (NULL, IDC_ARROW));
	}
	
	CBCGPWnd::OnTimer(nIDEvent);
}
//*****************************************************************************
BOOL CBCGPGridCtrl::DoClickValue (CBCGPGridItem* pItem, UINT uiMsg, CPoint point, 
								  BOOL bFirstClick, BOOL bButtonClick)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pItem);

	CBCGPGridRow* pCurSel = GetCurSel ();
	if (pCurSel == NULL)
	{
		return FALSE;
	}

	ASSERT_VALID (pCurSel);

	// Start inplace editing:
	if (AllowInPlaceEdit () && pItem->IsAllowEdit ()
		&& (!bFirstClick || IsEditFirstClick ()))
	{
		SetBeginEditReason (BeginEdit_MouseClick);
		EditItem (pCurSel, &point);

		DoInplaceEditSetSel (OnInplaceEditSetSel (pItem, BeginEdit_MouseClick));
	}

	// Translate click:
	if (bButtonClick)
	{
		if (pCurSel->m_bInPlaceEdit)
		{
			if (::GetCapture () == GetSafeHwnd ())
			{
				ReleaseCapture ();
			}
		}

		pItem->DoClickButton (point);
	}
	else
	{
		return pItem->OnClickValue (uiMsg, point);
	}

	return TRUE;
}
//*****************************************************************************
BOOL CBCGPGridCtrl::StartSelectItems ()
{
	int nLButton = GetSystemMetrics (SM_SWAPBUTTON) ? VK_RBUTTON : VK_LBUTTON;
	if ((GetAsyncKeyState (nLButton) & 0x8000) == 0)
	{
		return FALSE;
	}

	// Start selecting range of items:
	SetCapture ();
	m_bSelecting = TRUE;

    return TRUE;
}
//*****************************************************************************
void CBCGPGridCtrl::StopSelectItems ()
{
	m_bSelecting = FALSE;

	if (::GetCapture () == GetSafeHwnd ())
	{
		ReleaseCapture ();
	}
}
//*****************************************************************************
BOOL CBCGPGridCtrl::SelectItems (CPoint ptClient)
{
	// ------------------------
	// perform range selection:
	// ------------------------
	CBCGPGridItemID id;
	CBCGPGridItem* pHitItem = NULL;
	CBCGPGridRow* pHitRow = NULL;

	if (m_bHeaderRowSelecting)
	{
		pHitRow = HitTestRowHeader (ptClient);
		if (pHitRow != NULL)
		{
			id.m_nRow = pHitRow->GetRowId ();
			id.m_nColumn = -1;
		}
	}
	else if (m_bHeaderColSelecting)
	{
		int nColumnHit = GetColumnsInfo ().HitTestColumn (ptClient, FALSE, STRETCH_DELTA);
		if (nColumnHit >= 0)
		{
			id.m_nRow = -1;
			id.m_nColumn = nColumnHit;
		}
	}

	if (id.IsNull ())
	{
		pHitRow = HitTest (ptClient, id, pHitItem);
	}

	//---------------------------------
	// Update selection (second click):
	//---------------------------------
	BOOL bSelChanged = id != m_idLastSel;

	if (!id.IsNull () && bSelChanged)
	{
		DWORD dwSelMode = SM_SECOND_CLICK | SM_CONTINUE_SEL_GROUP;
		m_pSetSelItem = m_bVirtualMode ? NULL : pHitItem;

		SetCurSel (id, dwSelMode);

		m_pSetSelItem = NULL;

		if (pHitRow != NULL)
		{
			EnsureVisible (pHitRow);
		}
	}

    return TRUE;
}
//*****************************************************************************
CBCGPGridRow* CBCGPGridCtrl::GetVirtualRow (int nRow)
{
	if (nRow < 0)
	{
		return NULL;
	}

	CBCGPGridRow* pRow = m_CachedItems.GetCachedRow (nRow);
	if (pRow == NULL)
	{
		pRow = CreateVirtualRow (nRow);

		if (!m_CachedItems.SetCachedRow (nRow, pRow))
		{
			ASSERT (FALSE);
			return NULL;
		}

		//-------------------------------
		// Check whether row is selected:
		//-------------------------------
		for (POSITION pos = m_lstSel.GetHeadPosition (); pos != NULL; )
		{
			CBCGPGridRange* pRange = (CBCGPGridRange*) m_lstSel.GetNext (pos);
			ASSERT (pRange != NULL);

			// Is row in selected range?
			if (pRange->m_nTop <= nRow && pRange->m_nBottom >= nRow)
			{
				DoSelectRowInRange (pRow, *pRange, TRUE, FALSE);
			}
		}
	}

	return pRow;
}
//*****************************************************************************
BOOL CBCGPGridCtrl::SendDispInfoRequest (BCGPGRID_DISPINFO* pdi) const
{
    ASSERT (pdi != NULL);

	pdi->item.varValue.Clear ();
	pdi->item.dwData = 0;
	pdi->item.iImage = -1;
	pdi->item.clrBackground = m_ColorData.m_clrBackground;
	pdi->item.clrText = m_ColorData.m_clrText;
	memset (&pdi->item.lfFont, 0, sizeof (LOGFONT));

	if (m_pfnCallback != NULL)
	{
		return m_pfnCallback (pdi, m_lParamCallback);
	}
	else
	{
		// Send the notification message
		pdi->hdr.hwndFrom = m_hWnd;
		pdi->hdr.idFrom   = GetDlgCtrlID();
		pdi->hdr.code     = BCGPGN_GETDISPINFO;

		CWnd* pOwner = GetOwner ();

		if (pOwner != NULL && IsWindow(pOwner->m_hWnd))
		{
			return (BOOL) pOwner->SendMessage (	WM_NOTIFY, 
												pdi->hdr.idFrom, (LPARAM)pdi);
		}
	}

    return FALSE;
}
//*****************************************************************************
CBCGPGridRow* CBCGPGridCtrl::CreateVirtualRow (int nRowIndex)
{
	if (!m_bVirtualMode)
	{
		ASSERT (FALSE);
		return NULL;
	}

	//------------
	// Create row:
	//------------
	static BCGPGRID_DISPINFO di;
	di.item.nRow	= nRowIndex;	// Row index
	di.item.nCol	= -1;			// Request row info

	if (!SendDispInfoRequest (&di))
	{
		ASSERT (FALSE);
		return NULL;
	}

	CBCGPGridRow* pRow = OnCreateVirtualRow (&di);
	if (pRow == NULL)
	{
		ASSERT (FALSE);
		return NULL;
	}

	ASSERT_VALID (pRow);

	//--------------
	// Create items:
	//--------------
	for (int i = 0; i < GetColumnCount () ; i++)
	{
		di.item.nRow	= nRowIndex;
		di.item.nCol	= i;

		if (!SendDispInfoRequest (&di))
		{
			ASSERT (FALSE);
			break;
		}

		CBCGPGridItem* pItem = OnCreateVirtualItem (&di);
		if (pItem != NULL)
		{
			ASSERT_VALID (pItem);

			pItem->SetOwnerRow (pRow);
			int nColIndex = (int) pRow->m_arrRowItems.Add (pItem);
			pItem->m_nIdColumn = nColIndex;

			pItem->m_strLabel = pItem->FormatItem ();
		}
	}

	//-------------------
	// Calc row position:
	//-------------------
	if (m_bIsPrinting)
	{
		int y = m_PrintParams.m_rectList.top - 1 + nRowIndex * m_PrintParams.m_nRowHeight -
			m_PrintParams.m_nVertScrollOffset;
		m_PrintParams.m_idCur.m_nRow = nRowIndex;
		pRow->Repos (y);
	}
	else
	{
		int y = m_rectList.top - 1 + nRowIndex * m_nRowHeight - m_nVertScrollOffset;
		m_idCur.m_nRow = nRowIndex;
		pRow->Repos (y);
	}

	return pRow;
}
//*****************************************************************************
CBCGPGridRow* CBCGPGridCtrl::OnCreateVirtualRow (BCGPGRID_DISPINFO *pdi)
{
	ASSERT_VALID (this);
	ASSERT (pdi != NULL);

	int nRow = pdi->item.nRow;

	CBCGPGridRow* pRow = CreateRow ();
	ASSERT_VALID (pRow);

	pRow->m_pWndList = this;
	pRow->m_nIdRow = nRow;

	pRow->m_dwData = pdi->item.dwData;

	return pRow;
}
//*****************************************************************************
CBCGPGridItem* CBCGPGridCtrl::OnCreateVirtualItem (BCGPGRID_DISPINFO *pdi)
{
	ASSERT_VALID (this);
	ASSERT (pdi != NULL);

	int nRow = pdi->item.nRow;
	int nColumn = pdi->item.nCol;

	CBCGPGridItem* pItem = NULL;

	switch (pdi->item.varValue.vt)
	{
	case VT_BOOL:
	case VT_BSTR:
	case VT_R4:
	case VT_R8:
	case VT_UI1:
	case VT_CY:
	case VT_I2:
	case VT_I4:
	case VT_INT:
	case VT_UINT:
	case VT_UI2:
	case VT_UI4:
	case VT_I8:
	case VT_UI8:
	case VT_DATE:

		pItem = CreateItem (nRow, nColumn);
		ASSERT_VALID (pItem);

		pItem->OnEndEdit ();

		pItem->m_varValue = pdi->item.varValue;

		pItem->Init ();
		pItem->SetFlags ();
		break;

	default:
		// Empty item for unsupported types:
		pItem = CreateItem (nRow, nColumn);
		ASSERT_VALID (pItem);

		pItem->OnEndEdit ();
		pItem->m_varValue.Clear ();

		pItem->Init ();
		pItem->SetFlags ();
		break;
	}

	if (pItem != NULL)
	{
		pItem->m_nIdColumn = nColumn;

		pItem->m_dwData = pdi->item.dwData;
		pItem->m_clrBackground = pdi->item.clrBackground;
		pItem->m_clrText = pdi->item.clrText;
		pItem->m_iImage = pdi->item.iImage;
	}

	return pItem;
}
//*****************************************************************************
CRect CBCGPGridCtrl::GetVirtualRowRect (int nRow) const
{
    ASSERT_VALID (this);

	if (nRow < 0 || nRow >= m_nVirtualRows)
	{
		CRect rectEmpty;
		rectEmpty.SetRectEmpty ();
		return rectEmpty;
	}

    int nHorzScrollOffset = m_nHorzScrollOffset;

    int nRowTop = m_rectList.top - m_nVertScrollOffset - 1 + nRow * m_nRowHeight;
	int nRowLeft = m_rectList.left - nHorzScrollOffset;
	int nRowWidth = GetColumnsInfo ().GetTotalWidth ();

	BOOL bIsGroup = FALSE;//IsGroup ();
	CRect rect = CRect (nRowLeft, nRowTop,
		nRowLeft + nRowWidth, nRowTop + (bIsGroup ? m_nLargeRowHeight : m_nRowHeight));

    return rect;
}
//*****************************************************************************
CRect CBCGPGridCtrl::GetVirtualItemRect (int nRow, int nColumn) const
{
	ASSERT_VALID (this);
	ASSERT (nRow >= 0);
	ASSERT (nColumn >= 0);

	CRect rectRow = GetVirtualRowRect (nRow);
	CRect rect = rectRow;

	int nXMul = 1, nXDiv = 1;
	if (m_bIsPrinting)
	{
		// map to printer metrics
		ASSERT_VALID (m_pPrintDC);
		HDC hDCFrom = ::GetDC(NULL);

		nXMul = m_pPrintDC->GetDeviceCaps(LOGPIXELSX); // pixels in print dc
		nXDiv = ::GetDeviceCaps(hDCFrom, LOGPIXELSX); // pixels in screen dc

		::ReleaseDC(NULL, hDCFrom);
	}

	int nXLeft = rectRow.left;
	int nCount = 0;

	int nPos = GetColumnsInfo ().Begin ();
	while (nPos != GetColumnsInfo ().End ())
	{
		int iColumn = GetColumnsInfo ().Next (nPos);
		if (iColumn == -1)
		{
			break; // no more visible columns
		}

		ASSERT (iColumn >= 0);
		int nWidth = GetColumnsInfo ().GetColumnWidth (iColumn);

		if (m_bIsPrinting)
		{
			nWidth = ::MulDiv (nWidth, nXMul, nXDiv);
		}

        if (nColumn == iColumn)
		{
			rect.left = nXLeft;
			rect.right = rect.left + nWidth;
			return rect;
		}

		nXLeft += nWidth;
		nCount ++;
	}

	rect.SetRectEmpty ();
	return rect;
}
//*****************************************************************************
CBCGPGridItemID CBCGPGridCtrl::HitTestVirtual (CPoint pt, 
											   CBCGPGridRow::ClickArea* pnArea) const
{
	ASSERT (m_bIsPrinting == FALSE);

	//----------------
	// Calc row index:
	//----------------
	int nYTop = (pt.y - m_rectList.top + m_nVertScrollOffset + 1);
	int nRowIdx = (nYTop >= 0) ? (nYTop / m_nRowHeight) : -1;
	if (pnArea != NULL)
	{
		*pnArea = CBCGPGridRow::ClickName;
	}
	
	if (nRowIdx < 0 ||
		nRowIdx >= m_nVirtualRows ||
		!GetVirtualRowRect (nRowIdx).PtInRect (pt))
	{
		CBCGPGridItemID idNull;
		return idNull;
	}

	//-------------------
	// Calc column index:
	//-------------------
	int nColumnIdx = -1;

	int dx = 0;//GetHierarchyLevel () * m_nRowHeight;
	int nXLeft = m_rectList.left - m_nHorzScrollOffset;
	int nCount = 0;

	int nPos = GetColumnsInfo ().Begin ();
	while (nPos != GetColumnsInfo ().End ())
	{
		int iColumn = GetColumnsInfo ().Next (nPos);
		if (iColumn == -1)
		{
			break; // no more visible columns
		}

		ASSERT (iColumn >= 0);
		int nWidth = GetColumnsInfo ().GetColumnWidth (iColumn);

		BOOL bIsTreeColumn = (m_nTreeColumn == -1) ? (nCount == 0): (m_nTreeColumn == iColumn);
		if (bIsTreeColumn)
		{
			nWidth += GetExtraHierarchyOffset () + GetHierarchyOffset ();
		}

		if (pt.x < nXLeft + nWidth)
		{
			int nTreeOffset = bIsTreeColumn ? GetExtraHierarchyOffset () + dx : 0; 
			if (pt.x >= nXLeft + nTreeOffset)
			{
				nColumnIdx = iColumn;
				if (pnArea != NULL)
				{
					*pnArea = CBCGPGridRow::ClickValue;
				}
				break;
			}

			BOOL bIsGroup = FALSE;//IsGroup ();
			if (bIsGroup && pt.x >= nXLeft + dx)
			{
				if (pnArea != NULL)
				{
					*pnArea = CBCGPGridRow::ClickExpandBox;
				}
				break;
			}
		}

		nXLeft += nWidth;
		nCount++;
	}

	CBCGPGridItemID id (nRowIdx, nColumnIdx);
	return id;
}
//*******************************************************************************************
void CBCGPGridCtrl::OnBeginInplaceEdit (CBCGPGridItem* pItem)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pItem);

	CWnd* pOwner = GetOwner ();
	if (pOwner == NULL)
	{
		return;
	}

	CBCGPGridItemID id = pItem->GetGridItemID ();

	memset (&m_CurrentItemInfo, 0, sizeof (BCGPGRID_ITEM_INFO));

	m_CurrentItemInfo.pItem = pItem;
	m_CurrentItemInfo.nRow = id.m_nRow;
	m_CurrentItemInfo.nCol = id.m_nColumn;
	m_CurrentItemInfo.dwResultCode = (DWORD_PTR)m_dwBeginEditReason;

	pOwner->SendMessage (BCGM_GRID_ITEM_BEGININPLACEEDIT, GetDlgCtrlID (), LPARAM (&m_CurrentItemInfo));
}
//*******************************************************************************************
void CBCGPGridCtrl::OnEndInplaceEdit (CBCGPGridItem* pItem)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pItem);

	CWnd* pOwner = GetOwner ();
	if (pOwner == NULL)
	{
		return;
	}

	CBCGPGridItemID id = pItem->GetGridItemID ();

	memset (&m_CurrentItemInfo, 0, sizeof (BCGPGRID_ITEM_INFO));

	m_CurrentItemInfo.pItem = pItem;
	m_CurrentItemInfo.nRow = id.m_nRow;
	m_CurrentItemInfo.nCol = id.m_nColumn;
	m_CurrentItemInfo.dwResultCode = (DWORD_PTR)m_dwEndEditResult;
	
	ClearBeginEditReason ();
	ClearEndEditReason ();

	pOwner->PostMessage (BCGM_GRID_ITEM_ENDINPLACEEDIT, GetDlgCtrlID (), LPARAM (&m_CurrentItemInfo));
}
//*******************************************************************************************
BOOL CBCGPGridCtrl::IsAcceleratorKey(UINT nChar, UINT, UINT) const
{
	const BOOL bShift = (::GetAsyncKeyState (VK_SHIFT) & 0x8000) != 0 && !m_bIgnoreShiftBtn;
	const BOOL bCtrl = (::GetAsyncKeyState (VK_CONTROL) & 0x8000) != 0 && !m_bIgnoreCtrlBtn;

	// Ctrl + A
	if (nChar == VK_SELECT && bCtrl)
	{
		return TRUE;
	}

	// Ctrl + Space, Shift + Space
	if (nChar == VK_SPACE && (bShift || bCtrl))
	{
		return TRUE;
	}

	return FALSE;
}
//*******************************************************************************************
BOOL CBCGPGridCtrl::CanBeginInplaceEditOnChar (UINT nChar, UINT nRepCnt, UINT nFlags) const
{
	if (IsAcceleratorKey (nChar, nRepCnt, nFlags))
	{
		return FALSE;
	}

	if (nChar == VK_ESCAPE)
	{
		return FALSE;
	}

	if (nChar == VK_TAB)
	{
		return FALSE;
	}

	return TRUE;
}
//*******************************************************************************************
BOOL CBCGPGridCtrl::CanEndInplaceEditOnChar (UINT nChar, UINT /*nRepCnt*/, UINT /*nFlags*/) const
{
	switch (nChar)
	{
	case VK_ESCAPE:	// Enable these keys
	case VK_F4:
		
		return TRUE; // To close in-place edit

	case VK_RETURN:
		{
			const BOOL bShift = (::GetAsyncKeyState (VK_SHIFT) & 0x8000) != 0;
			const BOOL bCtrl = (::GetAsyncKeyState (VK_CONTROL) & 0x8000) != 0;

			return (!bShift && !bCtrl); // If SHIFT or CONTROL - continue edit
		}
	}

	return FALSE;
}
//*******************************************************************************************
BOOL CBCGPGridCtrl::OnInplaceEditKeyDown (CBCGPGridRow* pSel, MSG* pMsg)
{
	ASSERT_VALID (pSel);
	ASSERT (pMsg != NULL);

	const BOOL bShift = (::GetAsyncKeyState (VK_SHIFT) & 0x8000) != 0;
	const BOOL bCtrl = (::GetAsyncKeyState (VK_CONTROL) & 0x8000) != 0;

	const BOOL bCanEndEdit = CanEndInplaceEditOnChar ((UINT)pMsg->wParam, LOWORD(pMsg->lParam), HIWORD(pMsg->lParam));
	BOOL bDefaultCase = FALSE;

	switch (pMsg->wParam)
	{
	case VK_RETURN:
		if (bCanEndEdit)
		{
			CComboBox* pWndCombo = pSel->GetComboWnd ();
			if (pWndCombo != NULL && pWndCombo->GetDroppedState ())
			{
				pSel->OnSelectCombo ();

				DoInplaceEditSetSel (OnInplaceEditSetSel (GetCurSelItem (pSel), BeginEdit_ComboReturn));

				CWnd* pWndInPlace = pSel->GetInPlaceWnd ();
				ASSERT_VALID (pWndInPlace);
				pWndInPlace->SetFocus ();
				return TRUE;
			}

			SetEndEditReason (EndEdit_OK | EndEdit_Return, bShift, bCtrl);
			if (!EndEditItem ())
			{
				MessageBeep ((UINT)-1);
			}

			SetFocus ();
		}
		break;

 	case VK_TAB:
		if (bCanEndEdit)
		{
			SetEndEditReason (EndEdit_OK | EndEdit_Tab, bShift, bCtrl);
			if (!EndEditItem ())
			{
				MessageBeep ((UINT)-1);
			}
			
			SetFocus ();
			return TRUE;
		}

		bDefaultCase = TRUE;
 		break;

	case VK_ESCAPE:
		if (bCanEndEdit)
		{
			SetEndEditReason (EndEdit_Cancel | EndEdit_Escape, bShift, bCtrl);
			EndEditItem (FALSE);
			SetFocus ();
		}
		return TRUE;

	case VK_F4:
		if (bCanEndEdit && pSel != NULL && pSel->m_bEnabled)
		{
			CBCGPGridItem* pItem = GetCurSelItem (pSel);
			if (pItem != NULL && 
				((pItem->m_dwFlags) & (BCGP_GRID_ITEM_HAS_BUTTON | BCGP_GRID_ITEM_HAS_LIST)))
			{
				CComboBox* pWndCombo = pSel->GetComboWnd ();
				if (pWndCombo != NULL && pWndCombo->GetDroppedState ())
				{
					return FALSE;
				}

				pItem->DoClickButton (CPoint (-1, -1));
			}
			return TRUE;
		}

		bDefaultCase = TRUE;
		break;

	case VK_LEFT:
		if (bCanEndEdit)
		{
			SetEndEditReason (EndEdit_OK | EndEdit_Left);
			if (!EndEditItem ())
			{
				MessageBeep ((UINT)-1);
			}
			
			SetFocus ();
			return TRUE;
		}

		bDefaultCase = TRUE;
		break;

	case VK_RIGHT:
		if (bCanEndEdit)
		{
			SetEndEditReason (EndEdit_OK | EndEdit_Right);
			if (!EndEditItem ())
			{
				MessageBeep ((UINT)-1);
			}
			
			SetFocus ();
			return TRUE;
		}

		bDefaultCase = TRUE;
		break;

	case VK_UP:
		if (bCanEndEdit)
		{
			CComboBox* pWndCombo = pSel->GetComboWnd ();
			if (pWndCombo != NULL && pWndCombo->GetDroppedState ())
			{
				bDefaultCase = TRUE;
				break;
			}

			SetEndEditReason (EndEdit_OK | EndEdit_Up);
			if (!EndEditItem ())
			{
				MessageBeep ((UINT)-1);
			}
			
			SetFocus ();
			return TRUE;
		}
		
		bDefaultCase = TRUE;
		break;

	case VK_DOWN:
		if (bCanEndEdit)
		{
			CComboBox* pWndCombo = pSel->GetComboWnd ();
			if (pWndCombo != NULL && pWndCombo->GetDroppedState ())
			{
				bDefaultCase = TRUE;
				break;
			}

			SetEndEditReason (EndEdit_OK | EndEdit_Down);
			if (!EndEditItem ())
			{
				MessageBeep ((UINT)-1);
			}
			
			SetFocus ();
			return TRUE;
		}

		bDefaultCase = TRUE;
		break;

	case VK_HOME:
		if (bCanEndEdit)
		{
			SetEndEditReason (EndEdit_OK | EndEdit_Home);
			if (!EndEditItem ())
			{
				MessageBeep ((UINT)-1);
			}
			
			SetFocus ();
			return TRUE;
		}
		
		bDefaultCase = TRUE;
		break;
		
	case VK_END:
		if (bCanEndEdit)
		{
			SetEndEditReason (EndEdit_OK | EndEdit_End);
			if (!EndEditItem ())
			{
				MessageBeep ((UINT)-1);
			}
			
			SetFocus ();
			return TRUE;
		}
		
		bDefaultCase = TRUE;
		break;

	default:
		bDefaultCase = TRUE;
	}

	if (bDefaultCase)
	{
		if (!pSel->m_bAllowEdit)
		{
			pSel->PushChar ((UINT) pMsg->wParam);
			return TRUE;
		}

		if (ProcessClipboardAccelerators ((UINT) pMsg->wParam))
		{
			return TRUE;
		}
		
		return FALSE;
	}

	return bCanEndEdit; // should return FALSE to translate default message
}
//*****************************************************************************
UINT CBCGPGridCtrl::OnInplaceEditSetSel (CBCGPGridItem* /*pCurItem*/, UINT nReason) const
{
	if ((nReason & BeginEdit_Return) != 0)
	{
		return SetSel_SelectAll;
	}

	if ((nReason & BeginEdit_ComboReturn) != 0)
	{
		return SetSel_SelectAll;
	}

	return SetSel_CaretAtLeft;
}
//*****************************************************************************
void CBCGPGridCtrl::DoInplaceEditSetSel (UINT nFlags)
{
	CBCGPGridRow* pSel = GetCurSel ();
	if (pSel == NULL)
	{
		return;
	}

	if (nFlags == SetSel_CaretByCursor) // by default
	{
		// do nothing
		return;
	}

	CEdit* pWndInPlace = DYNAMIC_DOWNCAST (CEdit, pSel->GetInPlaceWnd ());
	if (pWndInPlace != NULL && pSel != NULL)
	{
		if ((nFlags & SetSel_SelectAll) != 0)
		{
			pWndInPlace->SendMessage (EM_SETSEL, (WPARAM)0, (LPARAM)-1); // select all inside inplace edit
		}
		else if ((nFlags & SetSel_CaretAtLeft) != 0) 
		{
			pWndInPlace->SendMessage (EM_SETSEL, (WPARAM)0, (LPARAM)0); 
		}
		else if ((nFlags & SetSel_CaretAtRight) != 0)
		{
			int nLen = pWndInPlace->GetWindowTextLength ();
			pWndInPlace->SendMessage (EM_SETSEL, (WPARAM)nLen, (LPARAM)nLen); 
		}
	}
}
//*****************************************************************************
UINT CBCGPGridCtrl::OnGridKeybordNavigation (CBCGPGridItem* /*pCurItem*/, UINT nReason)
{
	const BOOL bShift = (nReason & EndEdit_Shift) != 0;

	if ((nReason & EndEdit_Tab) != 0)
	{
		return !bShift ? 
			(NextColumn | Down): 
			(PrevColumn | Up);
	}
	else if ((nReason & EndEdit_Right) != 0)
	{
		return Right;
	}
	else if ((nReason & EndEdit_Left) != 0)
	{
		return Left;
	}
	else if ((nReason & EndEdit_Up) != 0)
	{
		return Up;
	}
	else if ((nReason & EndEdit_Down) != 0)
	{
		return Down;
	}
	else if ((nReason & EndEdit_Home) != 0)
	{
		return FirstRow;
	}
	else if ((nReason & EndEdit_End) != 0)
	{
		return LastRow;
	}
	else if ((nReason & EndEdit_Return) != 0)
	{
		return NoMove;
	}

	return NoMove;
}
//*****************************************************************************
BOOL CBCGPGridCtrl::GoToNextItem (UINT nDirectionFlags)
{
	CBCGPGridItem* pCurItemOld = GetCurSelItem ();
	if (pCurItemOld == NULL)
	{
		const UINT nRowFlags = Up | PrevRow | Down | NextRow | FirstRow | LastRow;

		if ((nDirectionFlags & nRowFlags) == 0)
		{
			return FALSE;
		}

		nDirectionFlags &= nRowFlags;
	}

	m_bIgnoreShiftBtn = TRUE;
	m_bIgnoreCtrlBtn = TRUE;

	if ((nDirectionFlags & (Left | PrevColumn)) != 0)
	{
		SendMessage (WM_KEYDOWN, VK_LEFT);
	}
	else if ((nDirectionFlags & (Right | NextColumn)) != 0)
	{
		SendMessage (WM_KEYDOWN, VK_RIGHT);
	}

	else if ((nDirectionFlags & (Up | PrevRow)) != 0)
	{
		SendMessage (WM_KEYDOWN, VK_UP);
	}
	else if ((nDirectionFlags & (Down | NextRow)) != 0)
	{
		SendMessage (WM_KEYDOWN, VK_DOWN);
	}

	if (GetCurSelItem () == pCurItemOld && pCurItemOld != NULL)
	{
		if ((nDirectionFlags & PrevColumn) != 0)
		{
			// Feed to next or previous row
			if ((nDirectionFlags & Up) != 0)
			{
				SendMessage (WM_KEYDOWN, VK_UP);
			}
			else if ((nDirectionFlags & Down) != 0)
			{
				SendMessage (WM_KEYDOWN, VK_DOWN);
			}

			if (GetCurSelItem () != pCurItemOld)
			{
				nDirectionFlags |= LastColumn;
			}
		}
		else if ((nDirectionFlags & NextColumn) != 0)
		{
			// Feed to next or previous row
			if ((nDirectionFlags & Up) != 0)
			{
				SendMessage (WM_KEYDOWN, VK_UP);
			}
			else if ((nDirectionFlags & Down) != 0)
			{
				SendMessage (WM_KEYDOWN, VK_DOWN);
			}

			if (GetCurSelItem () != pCurItemOld)
			{
				nDirectionFlags |= FirstColumn;
			}
		}
	}

	if ((nDirectionFlags & FirstColumn) != 0)
	{
		// Go to first column
		CBCGPGridItem* pItem = GetCurSelItem ();
		if (pItem != NULL)
		{
			CBCGPGridItemID id = pItem->GetGridItemID ();
			id.m_nColumn = GetColumnsInfo ().GetFirstVisibleColumn ();
			SetCurSel (id);
		}
	}
	else if ((nDirectionFlags & LastColumn) != 0)
	{
		// Go to last column
		CBCGPGridItem* pItem = GetCurSelItem ();
		if (pItem != NULL)
		{
			CBCGPGridItemID id = pItem->GetGridItemID ();
			id.m_nColumn = GetColumnsInfo ().GetLastVisibleColumn ();
			SetCurSel (id);
		}
	}

	else if ((nDirectionFlags & FirstRow) != 0)
	{
		SendMessage (WM_KEYDOWN, VK_HOME);
	}
	else if ((nDirectionFlags & LastRow) != 0)
	{
		SendMessage (WM_KEYDOWN, VK_END);
	}

	m_bIgnoreShiftBtn = FALSE;
	m_bIgnoreCtrlBtn = FALSE;

	if (GetCurSelItem () != pCurItemOld)
	{
		return TRUE;
	}

	return FALSE;
}
//*****************************************************************************
BOOL CBCGPGridCtrl::OnEditEmptyValue (int /*nRow*/, int /*nColumn*/, CBCGPGridItem* pItem)
{
	ASSERT_VALID (pItem);
	return pItem->ChangeType (_variant_t ((LPCTSTR) _T("")));
}
//*****************************************************************************
void CBCGPGridCtrl::OnSelChanging (const CBCGPGridRange &range, BOOL bSelect)
{
	BCGPGRID_NOTIFICATION gn;

    gn.nRow = range.m_nTop;
	gn.nCol = range.m_nLeft;
    gn.nRowTo = range.m_nBottom;
	gn.nColTo = range.m_nRight;
	gn.lParam = (LPARAM) bSelect;

	SendNotification (&gn, BCGPGN_SELCHANGING);
}
//*****************************************************************************
void CBCGPGridCtrl::OnSelChanged (const CBCGPGridRange &range, BOOL bSelect)
{
	BCGPGRID_NOTIFICATION gn;

    gn.nRow = range.m_nTop;
	gn.nCol = range.m_nLeft;
    gn.nRowTo = range.m_nBottom;
	gn.nColTo = range.m_nRight;
	gn.lParam = (LPARAM) bSelect;

	SendNotification (&gn, BCGPGN_SELCHANGED);

	InvalidateRect (m_rectHeader);
	InvalidateRect (m_rectRowHeader);
}
//*****************************************************************************
BOOL CBCGPGridCtrl::SendNotification (BCGPGRID_NOTIFICATION* pn, UINT uCode) const
{
    ASSERT (pn != NULL);

	// Send the notification message
	pn->hdr.hwndFrom = m_hWnd;
	pn->hdr.idFrom   = GetDlgCtrlID();
	pn->hdr.code     = uCode;

	CWnd *pOwner = GetOwner();
	if (pOwner && IsWindow(pOwner->m_hWnd))
	{
		return (BOOL) pOwner->SendMessage(WM_NOTIFY, pn->hdr.idFrom, (LPARAM)pn);
	}

    return FALSE;
}
//*****************************************************************************
void CBCGPGridCtrl::OnResizeColumns ()
{
}
//*****************************************************************************
void CBCGPGridCtrl::OnHeaderColumnClick (int nColumn)
{
	ASSERT_VALID (this);
	
	CWnd* pOwner = GetOwner ();
	if (pOwner != NULL)
	{
		pOwner->SendMessage (BCGM_GRID_COLUMN_CLICK, GetDlgCtrlID (), nColumn);
	}
}
//*****************************************************************************
void CBCGPGridCtrl::OnHideColumnChooser ()
{
	ASSERT_VALID (this);

	CWnd* pOwner = GetOwner ();
	if (pOwner != NULL)
	{
		pOwner->SendMessage (BCGM_GRID_ON_HIDE_COLUMNCHOOSER);
	}
}
//*****************************************************************************
int CBCGPGridCtrl::OnGetColumnMinWidth (int /*nColumn*/) const
{
	return GetBaseHeight ();
}
//*****************************************************************************
int CBCGPGridCtrl::OnGetColumnAutoSize (int /*nColumn*/) const
{
	return -1;
}
//*****************************************************************************
void CBCGPGridCtrl::UpdateColumnsChooser ()
{
	ASSERT_VALID (this);

	if (m_pColumnChooser != NULL)
	{
		m_pColumnChooser->UpdateList ();
	}
}
//*****************************************************************************
void CBCGPGridCtrl::MergeSelection (BOOL bRedraw)
{
	ASSERT_VALID (this);

	for (POSITION pos = m_lstSel.GetHeadPosition (); pos != NULL;)
	{
		CBCGPGridRange* pSelRange = m_lstSel.GetNext (pos);
		ASSERT (pSelRange != NULL);

		MergeRange (*pSelRange, bRedraw);
	}
}
//*****************************************************************************
void CBCGPGridCtrl::UnMergeSelection (BOOL bRedraw)
{
	ASSERT_VALID (this);

	for (POSITION pos = m_lstSel.GetHeadPosition (); pos != NULL;)
	{
		CBCGPGridRange* pSelRange = m_lstSel.GetNext (pos);
		ASSERT (pSelRange != NULL);

		UnMergeRange (*pSelRange, bRedraw);
	}
}
//*****************************************************************************
void CBCGPGridCtrl::MergeRange (const CBCGPGridRange& range, BOOL bRedraw)
{
	ASSERT_VALID (this);

	if (IsSortingMode ())
	{
		ASSERT(FALSE); // Sorting must be disabled
		return;
	}

	if (IsRowExtraHeightAllowed ())
	{
		ASSERT (FALSE); // The grid must have fixed row size
		return;
	}

	if (range.m_nLeft < 0 || range.m_nTop < 0 ||
		range.m_nRight < 0 || range.m_nBottom < 0)
	{
		return;
	}

	SetEndEditReason (EndEdit_AutoApply | EndEdit_Layout);
	EndEditItem ();

	if (!CanMergeRange (range, TRUE))
	{
		return;
	}

	CBCGPGridRange rangeExt = range;
	if (ExtendMergedRange (rangeExt))
	{
		// Range already contains some merged items. Unmerge them first.
		TRACE0 ("\nCBCGPGridCtrl::MergeRange: Can't merge.");
		return;
	}

	OnMergeCellsChanging (range, TRUE);

	CBCGPGridMergedCells* pMergedRange = new CBCGPGridMergedCells;
	pMergedRange->SetRange (range);

	CBCGPGridRange rangeNormalized (
			min (range.m_nLeft, range.m_nRight),
			min (range.m_nBottom, range.m_nTop),
			max (range.m_nLeft, range.m_nRight),
			max (range.m_nBottom, range.m_nTop));

	DoMergeInRange (rangeNormalized, pMergedRange);

	pMergedRange->Release ();
	pMergedRange = NULL; // pMergedRange can't be used now

	OnMergeCellsChanged (range, TRUE);

	if (bRedraw)
	{
		AdjustLayout ();
	}
}
//*****************************************************************************
void CBCGPGridCtrl::UnMergeRange (const CBCGPGridRange& range, BOOL bRedraw)
{
	ASSERT_VALID (this);

	if (IsRowExtraHeightAllowed ())
	{
		ASSERT (FALSE); // The grid must have fixed row size
		return;
	}

	if (range.m_nLeft < 0 || range.m_nTop < 0 ||
		range.m_nRight < 0 || range.m_nBottom < 0)
	{
		return;
	}

	SetEndEditReason (EndEdit_AutoApply | EndEdit_Layout);
	EndEditItem ();

	if (!CanMergeRange (range, FALSE))
	{
		return;
	}

	CBCGPGridRange rangeExt = range;
	if (ExtendMergedRange (rangeExt))
	{
		// Some merged items are partitially in range. Range should be extended.
		TRACE0 ("\nCBCGPGridCtrl::UnMergeRange: Can't unmerge.");
		return;
	}

	CBCGPGridRange rangeNormalized (
			min (range.m_nLeft, range.m_nRight),
			min (range.m_nBottom, range.m_nTop),
			max (range.m_nLeft, range.m_nRight),
			max (range.m_nBottom, range.m_nTop));

	OnMergeCellsChanging (rangeNormalized, FALSE);

	DoMergeInRange (rangeNormalized, NULL);

	OnMergeCellsChanged (rangeNormalized, FALSE);

	if (bRedraw)
	{
		AdjustLayout ();
	}
}
//*****************************************************************************
void CBCGPGridCtrl::DoMergeInRange (const CBCGPGridRange& rangeNormalized, CBCGPGridMergedCells* pMergedCells)
{	// if pMergedCells is NULL, then unmerge cells
	ASSERT_VALID (this);

	if (m_bVirtualMode)
	{
		// iterate through cached items
		for (POSITION pos = m_CachedItems.m_lstCache.GetHeadPosition (); pos != NULL; )
		{
			CBCGPGridCachePageInfo& cpi = m_CachedItems.m_lstCache.GetNext (pos);
			ASSERT (cpi.pArrCachePage->GetSize() == cpi.nSize);

			int nOffset = max (0, rangeNormalized.m_nTop - cpi.nFirst);
			while (nOffset >= 0 && nOffset < cpi.nSize && 
				nOffset <= rangeNormalized.m_nBottom - cpi.nFirst)
			{
				CBCGPGridRow* pRow = cpi.pArrCachePage->GetAt (nOffset);
				if (pRow != NULL)
				{
					ASSERT_VALID (pRow);
					DoMergeRowItemsInRange (pRow, rangeNormalized, pMergedCells);
				}

				nOffset++;
			}
		}
	}
	else
	//if (IsValidRange (range))
	{
		const CList<CBCGPGridRow*, CBCGPGridRow*>& lst =
			(!IsSortingMode () && !IsGrouping ()) ? m_lstItems : m_lstTerminalItems;

		POSITION pos = lst.FindIndex (rangeNormalized.m_nTop);
		for (int i = rangeNormalized.m_nTop; pos != NULL && i <= rangeNormalized.m_nBottom; i++)
		{
			CBCGPGridRow* pRow = lst.GetNext (pos);
			ASSERT_VALID (pRow);

			DoMergeRowItemsInRange (pRow, rangeNormalized, pMergedCells);
		}
	}
}
//*****************************************************************************
void CBCGPGridCtrl::DoMergeRowItemsInRange (CBCGPGridRow* pRow, const CBCGPGridRange& range,
											CBCGPGridMergedCells* pMergedCells)
{	// if pMergedCells is NULL, then unmerge cells
	ASSERT_VALID (pRow);

	BOOL bInRange = FALSE;

	int nPos = GetColumnsInfo ().Begin ();
	while (nPos != GetColumnsInfo ().End ())
	{
		int iColumn = GetColumnsInfo ().Next (nPos);
		if (iColumn == -1)
		{
			break; // no more visible columns
		}

		BOOL bIsRangeBound = (iColumn == range.m_nLeft || iColumn == range.m_nRight);

		if (bIsRangeBound || bInRange)
		{
			CBCGPGridItem* pItem = pRow->GetItem (iColumn);

			if (pItem != NULL)
			{
				ASSERT_VALID (pItem);
				pItem->Merge (pMergedCells);
			}
		}

		if (bIsRangeBound)
		{
			if (bInRange || range.m_nLeft == range.m_nRight)
			{
				break;	// last visible column in range
			}

			bInRange = TRUE;
		}
	}
}
//*****************************************************************************************************
BOOL CBCGPGridCtrl::ExtendMergedRange (CBCGPGridRange& range)
{
	ASSERT_VALID (this);

	if (range.IsEmpty ())
	{
		return FALSE;
	}

	// Extends the range to make it larger if has merged cells. 
	// The result range contains all existing merged items that are partitially fit in the source range.
	CList <CBCGPGridRange, CBCGPGridRange&> lstMergedRanges;

	CBCGPGridRange rangeCovered;
	CList <CBCGPGridRange, CBCGPGridRange&> lstRangeNonCovered;
	lstRangeNonCovered.AddTail (range);

	while (!lstRangeNonCovered.IsEmpty ())
	{
		CBCGPGridRange rangeNonCovered = lstRangeNonCovered.RemoveHead ();

		int nBefore = (int) lstMergedRanges.GetCount ();
		int nAdded = GetMergedItemsInRange (rangeNonCovered, lstMergedRanges) - nBefore;

		UnionRange (&rangeCovered, &rangeNonCovered);

		POSITION pos = lstMergedRanges.GetTailPosition ();
		for (int i = 0; i < nAdded && pos != NULL; i++)
		{
			const CBCGPGridRange& r = lstMergedRanges.GetPrev (pos);

			// Find out all parts which are out of covered range
			if (r.m_nLeft < rangeCovered.m_nLeft)
			{
				CBCGPGridRange rangeExtend = rangeCovered;
				rangeExtend.m_nLeft = r.m_nLeft;
				rangeExtend.m_nRight = rangeCovered.m_nLeft - 1;
				lstRangeNonCovered.AddTail (rangeExtend);
				
				rangeCovered.m_nLeft = r.m_nLeft;
			}
			
			if (r.m_nRight > rangeCovered.m_nRight)
			{
				CBCGPGridRange rangeExtend = rangeCovered;
				rangeExtend.m_nRight = r.m_nRight;
				rangeExtend.m_nLeft = rangeCovered.m_nRight + 1;
				lstRangeNonCovered.AddTail (rangeExtend);
				
				rangeCovered.m_nRight = r.m_nRight;
			}
			
			if (r.m_nTop < rangeCovered.m_nTop)
			{
				CBCGPGridRange rangeExtend = rangeCovered;
				rangeExtend.m_nTop = r.m_nTop;
				rangeExtend.m_nBottom = rangeCovered.m_nTop - 1;
				lstRangeNonCovered.AddTail (rangeExtend);
				
				rangeCovered.m_nTop = r.m_nTop;
			}
			
			if (r.m_nBottom > rangeCovered.m_nBottom)
			{
				CBCGPGridRange rangeExtend = rangeCovered;
				rangeExtend.m_nBottom = r.m_nBottom;
				rangeExtend.m_nTop = rangeCovered.m_nBottom + 1;
				lstRangeNonCovered.AddTail (rangeExtend);
				
				rangeCovered.m_nBottom = r.m_nBottom;
			}

			UnionRange (&rangeCovered, &r);
		}
	}

	if (rangeCovered != range)
	{
		range = rangeCovered;
		return TRUE;
	}

	return FALSE;
}
//*****************************************************************************************************
int CBCGPGridCtrl::GetMergedItemsInRange (const CBCGPGridRange& range, 
										  CList <CBCGPGridRange, CBCGPGridRange&> &lstRanges)
{
	ASSERT_VALID (this);

	CBCGPGridRange rangeNormalized (
			min (range.m_nLeft, range.m_nRight),
			min (range.m_nBottom, range.m_nTop),
			max (range.m_nLeft, range.m_nRight),
			max (range.m_nBottom, range.m_nTop));

	if (!m_bVirtualMode)
	{
		const CList<CBCGPGridRow*, CBCGPGridRow*>& lst =
			(!IsSortingMode () && !IsGrouping ()) ? m_lstItems : m_lstTerminalItems;

		POSITION pos = lst.FindIndex (rangeNormalized.m_nTop);
		for (int i = rangeNormalized.m_nTop; pos != NULL && i <= rangeNormalized.m_nBottom; i++)
		{
			CBCGPGridRow* pRow = lst.GetNext (pos);
			ASSERT_VALID (pRow);

			DoGetMergedItemsInRange (pRow, rangeNormalized, lstRanges);
		}
	}

	return (int) lstRanges.GetCount ();
}
//*****************************************************************************************************
void CBCGPGridCtrl::DoGetMergedItemsInRange (CBCGPGridRow* pRow, const CBCGPGridRange& range, 
											 CList <CBCGPGridRange, CBCGPGridRange&> &lstRanges)
{
	ASSERT_VALID (pRow);

	BOOL bInRange = FALSE;

	int nPos = GetColumnsInfo ().Begin ();
	while (nPos != GetColumnsInfo ().End ())
	{
		int iColumn = GetColumnsInfo ().Next (nPos);
		if (iColumn == -1)
		{
			break; // no more visible columns
		}

		BOOL bIsRangeBound = (iColumn == range.m_nLeft || iColumn == range.m_nRight);

		if (bIsRangeBound || bInRange)
		{
			CBCGPGridItem* pItem = pRow->GetItem (iColumn);

			if (pItem != NULL)
			{
				ASSERT_VALID (pItem);

				CBCGPGridRange range;
				if (pItem->GetMergedRange (range))
				{
					// add once:
					BOOL bAlreadyInList = FALSE;
					for (POSITION pos = lstRanges.GetTailPosition (); pos != NULL && !bAlreadyInList; )
					{
						CBCGPGridRange& rangeSaved = lstRanges.GetPrev (pos);
						if (rangeSaved == range)
						{
							bAlreadyInList = TRUE;
						}
					}

					if (!bAlreadyInList)
					{
						lstRanges.AddTail (range);
					}
				}
			}
		}

		if (bIsRangeBound)
		{
			if (bInRange || range.m_nLeft == range.m_nRight)
			{
				break;	// last visible column in range
			}

			bInRange = TRUE;
		}
	}
}
//*****************************************************************************
void CBCGPGridCtrl::MarkMergedItemChanged (const CRect& rectNew, CBCGPGridItem* pItem)
{
	ASSERT_VALID (this);

	if (pItem == NULL)
	{
		return;
	}

	ASSERT_VALID (pItem);

	const CRect& rectList = (m_bIsPrinting) ? m_PrintParams.m_rectList : m_rectList;

	if (pItem->m_Rect.top >= rectList.bottom ||
		pItem->m_Rect.bottom < rectList.top)
	{
		return; // item does not fit in the visible area
	}

	CBCGPGridMergedCells* pMerged = pItem->GetMergedCells ();
	if (pMerged != NULL)
	{
		ASSERT_VALID (pMerged);

		// save id of the first visible item in range
		CBCGPGridItemID id = pItem->GetGridItemID ();
		pMerged->MarkChanged (rectNew, id);

		// save merged range in list to update later
		CBCGPGridItemID idFirstVisible = pMerged->GetVisibleItemID ();
		if (id == idFirstVisible)
		{
			m_lstMergedItemsToUpdate.AddTail (pItem);
		}
	}
}
//*****************************************************************************
void CBCGPGridCtrl::UpdateMergedItems ()
{
	ASSERT_VALID (this);

	while (!m_lstMergedItemsToUpdate.IsEmpty ())
	{
		CBCGPGridItem* pMergedItem = m_lstMergedItemsToUpdate.RemoveHead ();
		ASSERT_VALID (pMergedItem);

		CRect rectItemOld = pMergedItem->GetRect ();

		CBCGPGridMergedCells* pMerged = pMergedItem->GetMergedCells ();
		ASSERT_VALID (pMerged);

		if (pMerged->IsChanged () && !pMerged->GetVisibleItemID ().IsNull ())
		{
			CBCGPGridRange& range = pMerged->GetRange ();

			//------------------------------------
			// Calc boundaries of the merged item:
			//------------------------------------
			CRect rectMerged = GetMergedRect (&range, pMerged->GetVisibleItemID ());
			pMerged->SetRect (rectMerged);

			pMerged->MarkUpdated ();

			// notify each item or main item
			pMergedItem->OnPosSizeChanged (rectItemOld);
		}
	}
}
//*****************************************************************************
void CBCGPGridCtrl::RedrawMergedItem (CBCGPGridItem* pItem)
{
	CBCGPGridMergedCells* pMerged = pItem->GetMergedCells ();
	ASSERT_VALID (pMerged);

	if (!m_lstMergedItemsToRedraw.IsEmpty ())
	{
		CBCGPGridItem* pLastItem = m_lstMergedItemsToRedraw.GetTail ();
		ASSERT_VALID (pLastItem);

		if (pItem->GetMergedCells () == pLastItem->GetMergedCells ())
		{
			return;
		}
	}

	pMerged->SetNeedRedraw (TRUE);
	m_lstMergedItemsToRedraw.AddTail (pItem);
}
//*****************************************************************************
void CBCGPGridCtrl::RedrawMergedItems (CDC* pDC)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);

	const int nFreezeOffset = m_rectList.left + GetColumnsInfo ().GetFreezeOffset ();
	
	while (!m_lstMergedItemsToRedraw.IsEmpty ())
	{
		CBCGPGridItem* pItem = m_lstMergedItemsToRedraw.RemoveHead ();
		ASSERT_VALID (pItem);
		
		CBCGPGridMergedCells* pMerged = pItem->GetMergedCells ();
		ASSERT_VALID (pMerged);

		if (pMerged->IsNeedRedraw ())
		{
			CBCGPGridItemID idMain = pMerged->GetMainItemID ();
			if (pItem->GetGridItemID () != idMain)
			{
				CBCGPGridRow* pRow = (idMain.m_nRow != -1) ? GetRow (idMain.m_nRow) : NULL;
				pItem = (pRow != NULL) ? pRow->GetItem (idMain.m_nColumn) : NULL;

				if (pItem == NULL)
				{
					continue;
				}

				ASSERT_VALID (pItem);
			}

			CRect rectValue = pMerged->GetRect ();
			rectValue.NormalizeRect ();

			CRect rectClipItem = rectValue;

			if (m_bIsPrinting)
			{
				const CRect rectClipPage = m_PrintParams.m_pageInfo.m_rectPageItems;
				if (rectClipItem.IntersectRect (&rectClipItem, &rectClipPage))
				{
					pItem->OnPrintValue (pDC, rectValue);
					pItem->OnPrintBorders (pDC, rectValue);
				}

				pMerged->SetNeedRedraw (FALSE);
				continue;
			}

			CRect rectBordersSize (0, 0, 0, 0);
			if (m_bGridItemBorders)
			{
				pItem->OnGetBorders (rectBordersSize);
				rectClipItem.InflateRect (rectBordersSize);
			}
			
			rectClipItem.IntersectRect (rectClipItem, m_rectClip);
			rectClipItem.IntersectRect (rectClipItem, m_rectList);

			// frozen columns:
			if (GetColumnsInfo ().IsFreezeColumnsEnabled ())
			{
				if (rectValue.right > nFreezeOffset)
				{
					// Do not allow unfrozen area to draw inside the frozen area
					rectClipItem.left = max (nFreezeOffset, rectClipItem.left);
				}

				if (pMerged->GetRange ().m_nLeft >= GetColumnsInfo ().GetFrozenColumnCount ())
				{
					if (rectValue.right <= nFreezeOffset)
					{
						rectClipItem.SetRectEmpty ();
					}
				}
 			}

			if (!rectClipItem.IsRectEmpty ())
			{
				m_rgnClipItem.CreateRectRgnIndirect (&rectClipItem);
				pDC->SelectClipRgn (&m_rgnClipItem);
				
				pItem->OnDrawValue (pDC, rectValue);
				
				if (m_bGridItemBorders)
				{
					pItem->OnDrawBorders (pDC, rectValue);
				}
				
				if (!pItem->m_rectButton.IsRectEmpty ())
				{
					pItem->OnDrawButton (pDC, pItem->m_rectButton);
				}
				
				pDC->SelectClipRgn (&m_rgnClip);
				m_rgnClipItem.DeleteObject ();
			}

			pMerged->SetNeedRedraw (FALSE);
		}
	}
}
//*****************************************************************************
CRect CBCGPGridCtrl::GetMergedRect (const CBCGPGridRange* pRange,
									 const CBCGPGridItemID& idVisible)
{	// TODO to speed up add parameter (const CBCGPGridItem* pItemVisible)
	ASSERT_VALID (this);
	ASSERT (pRange != NULL);
	ASSERT (pRange->IsValid ());
	ASSERT (pRange->IsInRange (idVisible));

	CRect rectResult (0, 0, 0, 0);

	CBCGPGridRow* pRowVisible = GetRow (idVisible.m_nRow);
	CBCGPGridItem* pItemVisible = (pRowVisible != NULL) ?
		pRowVisible->GetItem (idVisible.m_nColumn) : NULL;
	if (pItemVisible == NULL)
	{
		return rectResult;
	}

	ASSERT_VALID (pItemVisible);

	CRect rectVisible = pItemVisible->GetRect ();
	if (rectVisible.IsRectEmpty ())
	{
		return rectResult;
	}

	//-------------------------------------
	// Calculate top and bottom boundaries:
	//-------------------------------------
	rectResult = rectVisible;
	if (pRange->m_nTop < idVisible.m_nRow)
	{
		int nRowsBefore = idVisible.m_nRow - pRange->m_nTop;
		rectResult.top -= nRowsBefore * rectVisible.Height ();
	}
	if (pRange->m_nBottom > idVisible.m_nRow)
	{
		int nRowsAfter = pRange->m_nBottom - idVisible.m_nRow;
		rectResult.bottom += nRowsAfter * rectVisible.Height ();
	}

	//-------------------------------------
	// Calculate left and right boundaries:
	//-------------------------------------
	if (pRange->m_nLeft < idVisible.m_nColumn)
	{
		int nBefore = 0;

		for (int iColumn = pRange->m_nLeft;
			iColumn < idVisible.m_nColumn && iColumn < GetColumnsInfo ().GetColumnCount ();
			iColumn++)
		{
			int nWidth = GetColumnsInfo ().GetColumnWidth (iColumn);
			if (m_nTreeColumn == iColumn)
			{
				nWidth += GetHierarchyOffset () + GetExtraHierarchyOffset ();
			}
			nBefore += nWidth;
		}

		rectResult.left -= nBefore;
	}
	if (pRange->m_nRight > idVisible.m_nColumn)
	{
		int nXMul = 1, nXDiv = 1;
		if (m_bIsPrinting)
		{
			// map to printer metrics
			ASSERT_VALID (m_pPrintDC);
			HDC hDCFrom = ::GetDC(NULL);
			
			nXMul = m_pPrintDC->GetDeviceCaps(LOGPIXELSX); // pixels in print dc
			nXDiv = ::GetDeviceCaps(hDCFrom, LOGPIXELSX); // pixels in screen dc
			
			::ReleaseDC(NULL, hDCFrom);
		}

		int nAfter = 0;

		for (int iColumn = idVisible.m_nColumn + 1;
			iColumn <= pRange->m_nRight && iColumn < GetColumnsInfo ().GetColumnCount ();
			iColumn++)
		{
			int nWidth = GetColumnsInfo ().GetColumnWidth (iColumn);
			BOOL bIsTreeColumn = (m_nTreeColumn == -1) ? 
				(0 == iColumn) : (m_nTreeColumn == iColumn);
			if (bIsTreeColumn)
			{
				nWidth += GetHierarchyOffset () + GetExtraHierarchyOffset ();
			}
			if (m_bIsPrinting)
			{
				// map to printer metrics
				nWidth = ::MulDiv(nWidth, nXMul, nXDiv);
			}

			nAfter += nWidth;
		}

		rectResult.right += nAfter;
	}

	return rectResult;
}
//*****************************************************************************
CBCGPGridRange* CBCGPGridCtrl::GetMergedRange (CBCGPGridItemID id,
											   CBCGPGridItem* pItem, 
											   CBCGPGridRange& range)
{
	ASSERT_VALID (this);

	CBCGPGridItem* pMergedItem = pItem;
	if (m_bVirtualMode || pItem == NULL)
	{
		CBCGPGridRow* pRow = (id.m_nRow != -1) ? GetRow (id.m_nRow) : NULL;
		pMergedItem = (pRow != NULL) ? pRow->GetItem (id.m_nColumn) : NULL;
	}

	if (pMergedItem != NULL)
	{
		ASSERT_VALID (pMergedItem);

		if (pMergedItem->GetMergedRange (range))
		{
			return &range;
		}
	}

	return NULL;
}
//*****************************************************************************
void CBCGPGridCtrl::AllowRowExtraHeight (BOOL bAllow)
{
	m_bAllowRowExtraHeight = bAllow;
}
//*****************************************************************************
BOOL CBCGPGridCtrl::IsRowExtraHeightAllowed () const
{
	return m_bAllowRowExtraHeight;
}
//*****************************************************************************
BOOL CBCGPGridCtrl::IsPreviewRowEnabled () const
{
	return FALSE;
}
//*****************************************************************************
CRect CBCGPGridCtrl::OnGetPreviewRowMargins (CBCGPGridRow*) const
{
	CRect rect;
	rect.SetRectEmpty ();

	return rect;
}
//*****************************************************************************
void CBCGPGridCtrl::FreezeColumns (int nColumnCount, BOOL bRedraw)
//	if nColumnCount = -1, do not freeze columns
//	if nColumnCount > 0, nColumnCount is number of columns to freeze
{
	// TODO: if merged cells - ASSERT(FALSE)
	ASSERT_VALID (this);

	SetEndEditReason (EndEdit_Cancel | EndEdit_Layout);
	EndEditItem (FALSE);

	GetColumnsInfo ().EnableFreezeColumns (nColumnCount);

	if (bRedraw)
	{
		AdjustLayout ();
	}
}
//*****************************************************************************
void CBCGPGridCtrl::FreezeGroups (BOOL bFreeze, BOOL bRedraw)
{
	if (bFreeze == m_bFreezeGroups)
	{
		return;
	}

	m_bFreezeGroups = bFreeze;

	if (bRedraw)
	{
		AdjustLayout ();
	}
}
//*****************************************************************************
void CBCGPGridCtrl::IterateInRange (const CBCGPGridRange& range, 
   BCGPGRID_ITERATOR_ROW_CALLBACK pCallbackRowBegin, LPARAM lParamRowBegin,
   BCGPGRID_ITERATOR_ROW_CALLBACK pCallbackRowEnd, LPARAM lParamRowEnd,
   BCGPGRID_ITERATOR_ITEM_CALLBACK pCallbackItem, LPARAM lParamItem)
{
	ASSERT_VALID (this);

	CBCGPGridRange rangeNormalized = range;
	rangeNormalized.Normalize ();

	if (m_bVirtualMode)
	{
		// iterate through cached items
		for (POSITION pos = m_CachedItems.m_lstCache.GetHeadPosition (); pos != NULL; )
		{
			CBCGPGridCachePageInfo& cpi = m_CachedItems.m_lstCache.GetNext (pos);
			ASSERT (cpi.pArrCachePage->GetSize() == cpi.nSize);

			int nOffset = max (0, rangeNormalized.m_nTop - cpi.nFirst);
			while (nOffset >= 0 && nOffset < cpi.nSize && 
				nOffset <= rangeNormalized.m_nBottom - cpi.nFirst)
			{
				CBCGPGridRow* pRow = cpi.pArrCachePage->GetAt (nOffset);
				if (pRow != NULL)
				{
					ASSERT_VALID (pRow);

					if (!pRow->IsItemFiltered ()) // show item
					{
						DoIterateInRange (
							pRow, rangeNormalized,
							pCallbackRowBegin, lParamRowBegin,
							pCallbackRowEnd, lParamRowEnd,
							pCallbackItem, lParamItem);
					}
				}

				nOffset++;
			}
		}
	}
	else
	//if (IsValidRange (range))
	{
		BOOL bShowAllItems = (IsSortingMode () && !IsGrouping ());

		const CList<CBCGPGridRow*, CBCGPGridRow*>& lst =
			(!IsSortingMode () && !IsGrouping ()) ? m_lstItems : m_lstTerminalItems;

		POSITION pos = lst.FindIndex (rangeNormalized.m_nTop);
		for (int i = rangeNormalized.m_nTop; pos != NULL && i <= rangeNormalized.m_nBottom; i++)
		{
			CBCGPGridRow* pRow = lst.GetNext (pos);
			ASSERT_VALID (pRow);

			if (bShowAllItems ? 
				!pRow->IsItemFiltered () : pRow->IsItemVisible ()) // show item
			{
				DoIterateInRange (
					pRow, rangeNormalized,
					pCallbackRowBegin, lParamRowBegin,
					pCallbackRowEnd, lParamRowEnd,
					pCallbackItem, lParamItem);
			}
		}
	}
}
//*****************************************************************************
void CBCGPGridCtrl::DoIterateInRange (CBCGPGridRow* pRow, const CBCGPGridRange& range,
   BCGPGRID_ITERATOR_ROW_CALLBACK pCallbackRowBegin, LPARAM lParamRowBegin,
   BCGPGRID_ITERATOR_ROW_CALLBACK pCallbackRowEnd, LPARAM lParamRowEnd,
   BCGPGRID_ITERATOR_ITEM_CALLBACK pCallbackItem, LPARAM lParamItem)
{
	ASSERT_VALID (pRow);

	//-----------------------
	// Call pCallbackRowBegin
	//-----------------------
	if (pCallbackRowBegin != NULL)
	{
		pCallbackRowBegin (pRow, range, lParamRowBegin);
	}

	if (pCallbackItem != NULL)
	{
		BOOL bInRange = FALSE;

		int nPos = GetColumnsInfo ().Begin ();
		while (nPos != GetColumnsInfo ().End ())
		{
			int iColumn = GetColumnsInfo ().Next (nPos);
			if (iColumn == -1)
			{
				break; // no more visible columns
			}

			BOOL bIsRangeBound = (iColumn == range.m_nLeft || iColumn == range.m_nRight);

			if (bIsRangeBound || bInRange)
			{
				CBCGPGridItem* pItem = pRow->GetItem (iColumn);

				if (pItem != NULL)
				{
					ASSERT_VALID (pItem);

					//-------------------
					// Call pCallbackItem
					//-------------------
					pCallbackItem (pItem, range, lParamItem);
				}
			}

			if (bIsRangeBound)
			{
				if (bInRange || range.m_nLeft == range.m_nRight)
				{
					break;	// last visible column in range
				}

				bInRange = TRUE;
			}
		}
	}

	//---------------------
	// Call pCallbackRowEnd
	//---------------------
	if (pCallbackRowEnd != NULL)
	{
		pCallbackRowEnd (pRow, range, lParamRowEnd);
	}
}
//*****************************************************************************
void CBCGPGridCtrl::IterateColumnInRange (const CBCGPGridRange& range,
   BCGPGRID_ITERATOR_COLUMN_CALLBACK pCallbackColumn, LPARAM lParamColumn)
{
	if (pCallbackColumn != NULL)
	{
		BOOL bInRange = FALSE;

		int nPos = GetColumnsInfo ().Begin ();
		while (nPos != GetColumnsInfo ().End ())
		{
			int iColumn = GetColumnsInfo ().Next (nPos);
			if (iColumn == -1)
			{
				break; // no more visible columns
			}

			BOOL bIsRangeBound = (iColumn == range.m_nLeft || iColumn == range.m_nRight);

			if (bIsRangeBound || bInRange)
			{
				//---------------------
				// Call pCallbackColumn
				//---------------------
				pCallbackColumn (iColumn, range, lParamColumn);
			}

			if (bIsRangeBound)
			{
				if (bInRange || range.m_nLeft == range.m_nRight)
				{
					break;	// last visible column in range
				}

				bInRange = TRUE;
			}
		}
	}
}
//*****************************************************************************
// Implementation of ExportRangeToHTML:
class CallbackColumnParams
{
public:
	CallbackColumnParams (const CBCGPGridCtrl* p, CString& s)
		: pGrid (p), str (s)
	{
	}

	const CBCGPGridCtrl* pGrid;
	CString& str;
};
void CBCGPGridCtrl::pfnCallbackExportColumn (int nColumn, const CBCGPGridRange&, LPARAM lParam)
{
	CallbackColumnParams* params = (CallbackColumnParams*)lParam;
	if (params != NULL)
	{
		ASSERT_VALID (params->pGrid);

		CString& strHeaderLine = params->str;

		strHeaderLine += g_chSpace;
		strHeaderLine += g_chSpace;
		strHeaderLine += _T("<TH>");
		strHeaderLine += params->pGrid->GetColumnName (nColumn);
		strHeaderLine += _T("</TH>");
		strHeaderLine += g_strEOL;
	}
}
class CallbackRowParams
{
public:
	CallbackRowParams (const CBCGPGridCtrl* p, CString& s1, CString& s2)
		: pGrid (p), strHtml(s1), strLine(s2)
	{
	}

	const CBCGPGridCtrl* pGrid;
	CString& strHtml;
	CString& strLine;
};
void CBCGPGridCtrl::pfnCallbackExportRowEnd (CBCGPGridRow*, const CBCGPGridRange&, LPARAM lParam)
{
	CallbackRowParams* params = (CallbackRowParams*)lParam;
	if (params != NULL)
	{
		ASSERT_VALID (params->pGrid);

		CString& strHTML = params->strHtml;
		CString& strLine = params->strLine;

		if (!strLine.IsEmpty ())
		{
			strHTML += g_chSpace;
			strHTML += _T("<TR>");
			strHTML += g_strEOL;
			strHTML += strLine;
			strHTML += g_chSpace;
			strHTML += _T("</TR>");
			strHTML += g_strEOL;
		}

		strLine.Empty ();
	}
}
void CBCGPGridCtrl::pfnCallbackExportItem (CBCGPGridItem* pItem, const CBCGPGridRange&, LPARAM lParam)
{
	ASSERT_VALID (pItem);

	CallbackRowParams* params = (CallbackRowParams*)lParam;
	if (params != NULL)
	{
		ASSERT_VALID (params->pGrid);

		CString strItem = pItem->FormatItem ();
		if (strItem.IsEmpty ())
		{
			strItem = _T("&nbsp;");
		}
		params->pGrid->OnPrepareHTMLString (strItem);

		CString& strLine = params->strLine;
		strLine += g_chSpace;
		strLine += g_chSpace;
		strLine += _T("<TD>");
		strLine += strItem;
		strLine += _T("</TD>");
		strLine += g_strEOL;
	}
}
//*****************************************************************************
void CBCGPGridCtrl::ExportRangeToHTML (CString& strHTML, const CBCGPGridRange& range, DWORD /*dwFlags*/)
{
	strHTML += _T("<TABLE BORDER=1>");
	strHTML += g_strEOL;

	// ------------------------
	// Proceed header by items:
	// ------------------------
	CString strHeaderLine;

	CallbackColumnParams params1 (this, strHeaderLine);
	IterateColumnInRange (range, 
		&CBCGPGridCtrl::pfnCallbackExportColumn, (LPARAM )&params1);

	strHTML += g_chSpace;
	strHTML += _T("<TR>");
	strHTML += g_strEOL;
	strHTML += strHeaderLine;
	strHTML += _T("</TR>");
	strHTML += g_strEOL;

	CString strLine;

	CallbackRowParams params2 (this, strHTML, strLine);
	IterateInRange (range, 
	   NULL, 0,
	   &CBCGPGridCtrl::pfnCallbackExportRowEnd, (LPARAM )&params2,
	   &CBCGPGridCtrl::pfnCallbackExportItem, (LPARAM )&params2);

	strHTML += _T("</TABLE>");
	strHTML += g_strEOL;
}
//*****************************************************************************
void CBCGPGridCtrl::ExportToHTML (CString& strHTML, DWORD dwFlags)
{
	const int nFirstColumn = GetColumnsInfo ().GetFirstVisibleColumn ();
	const int nFirstRow = 0;
	const int nLastColumn = GetColumnsInfo ().GetLastVisibleColumn ();
	const int nLastRow = GetTotalItems () - 1;

	CBCGPGridRange range (nFirstColumn, nFirstRow, nLastColumn, nLastRow);

	ExportRangeToHTML (strHTML, range, dwFlags);
}
//*****************************************************************************
void CBCGPGridCtrl::ExportRowToHTML (CBCGPGridRow* pRow, CString& strHTML, DWORD /*dwFlags*/)
{
	ASSERT_VALID (pRow);

	// ----------------------
	// Proceed line by items:
	// ----------------------
	CString strLine;
	for (int i = 0; i < pRow->m_arrRowItems.GetSize (); i++)
	{
		CString strItem = pRow->m_arrRowItems [i]->FormatItem ();
		if (strItem.IsEmpty ())
		{
			strItem = _T("&nbsp;");
		}
		OnPrepareHTMLString (strItem);

		strLine += g_chSpace;
		strLine += g_chSpace;
		strLine += _T("<TD>");
		strLine += strItem;
		strLine += _T("</TD>");
		strLine += g_strEOL;
	}

	if (!strLine.IsEmpty ())
	{
		strHTML += g_chSpace;
		strHTML += _T("<TR>");
		strHTML += g_strEOL;
		strHTML += strLine;
		strHTML += g_chSpace;
		strHTML += _T("</TR>");
		strHTML += g_strEOL;
	}
}
//*****************************************************************************
void CBCGPGridCtrl::OnPrepareHTMLString (CString& str) const
{
	str.Replace (_T("<"), _T("&lt;"));
	str.Replace (_T(">"), _T("&gt;"));
}
//*****************************************************************************
// Implementation of ExportRangeToText:
class CallbackRowExportTextParams
{
public:
	CallbackRowExportTextParams (const CBCGPGridCtrl* p, CString& s1, CString& s2, DWORD dwF)
		: pGrid (p), strText(s1), strLine(s2), bNewLine (TRUE), dwFlags (dwF)
	{
	}
	
	const CBCGPGridCtrl* pGrid;
	CString& strText;
	CString& strLine;
	BOOL bNewLine;
	DWORD dwFlags;	// Format_CSV - Comma Separated Values, Format_TabSV - Tab Separated Values
};
void CBCGPGridCtrl::pfnCallbackExportTextRowEnd (CBCGPGridRow*, const CBCGPGridRange&, LPARAM lParam)
{
	CallbackRowExportTextParams* params = (CallbackRowExportTextParams*)lParam;
	if (params != NULL)
	{
		ASSERT_VALID (params->pGrid);
		
		CString& strText = params->strText;
		CString& strLine = params->strLine;
		
		if (!strLine.IsEmpty ())
		{
			if (!strText.IsEmpty ())
			{
				strText += g_strEOL;
			}
			strText += strLine;
		}
		
		strLine.Empty ();
		params->bNewLine = TRUE;
	}
}
void CBCGPGridCtrl::pfnCallbackExportTextItem (CBCGPGridItem* pItem, const CBCGPGridRange&, LPARAM lParam)
{
	ASSERT_VALID (pItem);
	
	CallbackRowExportTextParams* params = (CallbackRowExportTextParams*)lParam;
	if (params != NULL)
	{
		ASSERT_VALID (params->pGrid);
		
		CString strItem = pItem->FormatItem ();
		params->pGrid->OnPrepareTextString (strItem, params->dwFlags);
		
		CString& strLine = params->strLine;

		if (!params->bNewLine)
		{
			strLine += params->pGrid->GetExportTextDelimiter (params->dwFlags);
		}
		strLine += strItem;

		params->bNewLine = FALSE;
	}
}
//*****************************************************************************
void CBCGPGridCtrl::ExportRangeToText (CString& strText, const CBCGPGridRange& range, DWORD dwFlags)
{
	ASSERT_VALID (this);

	// Generate text in (Comma Separated Values or Tab Separated Values) format:
	CString strLine;

	CallbackRowExportTextParams params (this, strText, strLine, dwFlags);
	IterateInRange (range, 
	   NULL, 0,
	   &CBCGPGridCtrl::pfnCallbackExportTextRowEnd, (LPARAM )&params,
	   &CBCGPGridCtrl::pfnCallbackExportTextItem, (LPARAM )&params);
}
//*****************************************************************************
void CBCGPGridCtrl::OnPrepareTextString (CString& str, DWORD dwFlags) const
{
	//-------------------------------------
	// Comma Separated Values (CSV) Format:
	//-------------------------------------
	if ((dwFlags & Format_CSV) != 0)
	{
		// A quote within a text must be escaped with an additional quote immediately preceding the literal quote.
		str.Replace (_T("\""), _T("\"\""));

		// Text that contains commas, double-quotes, or line-breaks must be quoted.
		if (str.FindOneOf (_T(";,\"\n")) != -1)
		{
			CString strResult = _T("\"");
			strResult += str;
			strResult += _T("\"");
			str = strResult;
		}
	}

	//-----------------------------
	// Tab Separated Values format:
	//-----------------------------
	else if ((dwFlags & Format_TabSV) != 0)
	{
	}
}
//*****************************************************************************
CString CBCGPGridCtrl::GetExportTextDelimiter (DWORD dwFlags) const
{
	// Comma Separated Values (CSV) Format
	if ((dwFlags & Format_CSV) != 0)
	{
		return m_strExportCSVSeparator; // return ";" or ","
	}

	// Tab Separated Values Format
	else if ((dwFlags & Format_TabSV) != 0)
	{
		return _T("\t");
	}

	return _T("");
}
//*****************************************************************************
BOOL CBCGPGridCtrl::Cut (DWORD dwFlags)
{
	if (!Copy (dwFlags))
	{
		return FALSE;
	}

	if (m_pSerializeManager != NULL && 
		m_pSerializeManager->m_ClipboardFormatType == CBCGPGridSerializeManager::CF_Rows)
	{
		return Delete ();
	}

	return Clear (FALSE);
}
//*****************************************************************************
BOOL CBCGPGridCtrl::Copy (DWORD dwFlags)
{
	ASSERT_VALID (this);
	
	if (!IsCopyEnabled ())
	{
		ASSERT (FALSE);
		return FALSE;
	}

	if (dwFlags == (DWORD) -1) // by default
	{
		dwFlags = Format_TabSV | Format_Html;
	}

	//--------------------
	// Generate HTML Text:
	//--------------------
	CString strHtml;
	if ((dwFlags & Format_Html) != 0)
	{
		if (!m_lstSel.IsEmpty ()) // Copy selection to the clipboard:
		{
			CBCGPGridRange* pRangeLast = m_lstSel.GetTail ();
			ASSERT (pRangeLast != NULL);
			ExportRangeToHTML (strHtml, *pRangeLast, 0);
		}
		
		if (strHtml.IsEmpty ())
		{
			return FALSE;
		}
	}

	//-------------------------------
	// Generate Tab Separated Values:
	//-------------------------------
	CString strText;
	if ((dwFlags & (Format_TabSV | Format_CSV)) != 0)
	{
		if (!m_lstSel.IsEmpty ()) // Copy selection to the clipboard:
		{
			CBCGPGridRange* pRangeLast = m_lstSel.GetTail ();
			ASSERT (pRangeLast != NULL);
			ExportRangeToText (strText, *pRangeLast, dwFlags & (Format_TabSV | Format_CSV));
		}

		if (strText.IsEmpty ())
		{
			return FALSE;
		}
	}

	try
	{
		if (!OpenClipboard ())
		{
			TRACE0("Can't open clipboard\n");
			return FALSE;
		}
		
		if (!::EmptyClipboard ())
		{
			TRACE0("Can't empty clipboard\n");
			::CloseClipboard ();
			return FALSE;
		}
		
		//-----------------------------
		// Copy HTML Text to clipboard:
		//-----------------------------
		if ((dwFlags & Format_Html) != 0)
		{
			if (!CopyHtmlToClipboardInternal (strHtml, strHtml.GetLength ()))
			{
				::CloseClipboard ();
				return FALSE;
			}
		}
		
		//---------------------------
		// Copy CF_TEXT to clipboard:
		//---------------------------
		if ((dwFlags & (Format_TabSV | Format_CSV)) != 0)
		{
			if (!CopyTextToClipboardInternal (strText, strText.GetLength ()))
			{
				::CloseClipboard ();
				return FALSE;
			}
		}
		else if ((dwFlags & Format_Html) != 0)
		{
			if (!CopyTextToClipboardInternal (strHtml, strText.GetLength ()))
			{
				::CloseClipboard ();
				return FALSE;
			}
		}

		//----------------------------------------
		// Copy serialized selection to clipboard:
		//----------------------------------------
		if (!CopySelectionToClipboardInternal ())
		{
			::CloseClipboard ();
			return FALSE;
		}
		
		::CloseClipboard ();
	}
	catch (...)
	{
		TRACE0("CBCGPGridCtrl::Copy: out of memory\n");
	}
	
	return TRUE;
}
//*****************************************************************************
BOOL CBCGPGridCtrl::Paste ()
{
	ASSERT_VALID (this);

	if (IsReadOnly ())
	{
		return FALSE;
	}

	CBCGPGridRow* pSel = GetCurSel ();
	if (pSel != NULL && pSel->IsInPlaceEditing ())
	{
		return FALSE;
	}

	COleDataObject data;
	if (!data.AttachClipboard ())
	{
		TRACE0("Can't open clipboard\n");
		return FALSE;
	}

	CBCGPGridItemID idPasteTo = GetCurSelItemID ();
	if (idPasteTo.m_nRow == -1)
	{
		idPasteTo.m_nRow = 0; 
	}
	if (idPasteTo.m_nColumn == -1 || IsWholeRowSel ())
	{
		idPasteTo.m_nColumn = 0; 
	}

	return PasteFromDataObject (&data, idPasteTo, TRUE);
}
//*****************************************************************************
BOOL CBCGPGridCtrl::Clear (BOOL bQueryNonEmptyItems)
{
	if (IsReadOnly ())
	{
		return FALSE;
	}
	
	CBCGPGridRow* pSel = GetCurSel ();
	if (pSel != NULL && pSel->IsInPlaceEditing ())
	{
		return FALSE;
	}

	BOOL bResult = FALSE;
	BOOL bPrevQuery = FALSE;	// call OnQueryClearNonEmptyItem only once

	for (int i = 0; i < GetSelectionCount (); i++)
	{
		CBCGPGridRange range;
		if (!GetSelection (i, range))
		{
			bResult = FALSE;
			break;
		}

		if (!ClearRange (range, FALSE, bQueryNonEmptyItems, &bPrevQuery))
		{
			bResult = FALSE;
			break;
		}

		bResult = TRUE;
	}

	UpdateWindow ();

	return bResult;
}
//*****************************************************************************
BOOL CBCGPGridCtrl::Delete ()
{
	if (m_pSerializeManager == NULL || IsReadOnly ())
	{
		return FALSE;
	}

	CBCGPGridRow* pSel = GetCurSel ();
	if (pSel != NULL && pSel->IsInPlaceEditing ())
	{
		return FALSE;
	}
	
	return (RemoveSelectedRows () > 0);
}
//*****************************************************************************
BOOL CBCGPGridCtrl::IsCutEnabled () const
{
	if (m_pSerializeManager == NULL || IsReadOnly ())
	{
		return FALSE;
	}

	return !(GetCurSelItemID ().IsNull ());
}
//*****************************************************************************
BOOL CBCGPGridCtrl::IsCopyEnabled () const
{
	if (m_pSerializeManager == NULL)
	{
		return FALSE;
	}

	return !(GetCurSelItemID ().IsNull ());
}
//*****************************************************************************
BOOL CBCGPGridCtrl::IsPasteEnabled () const
{
	if (m_pSerializeManager == NULL || IsReadOnly ())
	{
		return FALSE;
	}
	
	ASSERT_VALID (m_pSerializeManager);
	
	COleDataObject data;
	if (data.AttachClipboard () && 
		data.IsDataAvailable (m_pSerializeManager->GetClipboardFormat ()))
	{
		return TRUE;
	}

	return FALSE;
}
//*****************************************************************************
BOOL CBCGPGridCtrl::IsClearEnabled () const
{
	if (IsReadOnly ())
	{
		return FALSE;
	}
	
	return !(GetCurSelItemID ().IsNull ());
}
//*****************************************************************************
BOOL CBCGPGridCtrl::IsDeleteEnabled () const
{
	if (IsReadOnly ())
	{
		return FALSE;
	}
	
	return !(GetCurSelItemID ().IsNull ());
}
//*****************************************************************************
BOOL CBCGPGridCtrl::OnQueryClearNonEmptyItem (const CBCGPGridItem* /*pItem*/) const
{
	CString strPrompt;
	strPrompt.LoadString (IDS_BCGBARRES_GRID_CLEAR_SELECTION);

	return (AfxMessageBox (strPrompt, MB_OKCANCEL) == IDOK);
}
//*****************************************************************************
BOOL CBCGPGridCtrl::OnQueryReplaceNonEmptyItem (const CBCGPGridItem* /*pItem*/) const
{
	CString strPrompt;
	strPrompt.LoadString (IDS_BCGBARRES_GRID_CLEAR_DEST);

	return (AfxMessageBox (strPrompt, MB_OKCANCEL) == IDOK);
}
//*****************************************************************************
BOOL CBCGPGridCtrl::CopyHTML ()
{
	ASSERT_VALID (this);

	CString strHtml;
	if (!m_lstSel.IsEmpty ()) // Copy selection to the clipboard:
	{
		CBCGPGridRange* pRangeLast = m_lstSel.GetTail ();
		ASSERT (pRangeLast != NULL);
		ExportRangeToHTML (strHtml, *pRangeLast, 0);
	}
	else // Copy all to the clipboard:
	{
		ExportToHTML (strHtml, 0);
	}

	if (strHtml.IsEmpty ())
	{
		return FALSE;
	}

	//--------------------------
	// Copy strHtml to clipboard
	//--------------------------
	try
	{
		if (!OpenClipboard ())
		{
			TRACE0("Can't open clipboard\n");
			return FALSE;
		}

		if (!::EmptyClipboard ())
		{
			TRACE0("Can't empty clipboard\n");
			::CloseClipboard ();
			return FALSE;
		}

		//-----------------------------
		// Copy HTML Text to clipboard:
		//-----------------------------
		if (!CopyHtmlToClipboardInternal (strHtml, strHtml.GetLength ()))
		{
			::CloseClipboard ();
			return FALSE;
		}

		//---------------------------
		// Copy CF_TEXT to clipboard:
		//---------------------------
		if (!CopyTextToClipboardInternal (strHtml, strHtml.GetLength ()))
		{
			::CloseClipboard ();
			return FALSE;
		}

		::CloseClipboard ();
	}
	catch (...)
	{
		TRACE0("CopyTextToClipboard: out of memory\n");
	}

	return TRUE;
}
//*****************************************************************************
static int StringToUTF8 (LPCTSTR lpSrc, LPSTR& lpDst, int nLength)
{
	LPWSTR lpWide = NULL;
	int count = 0;

#ifdef _UNICODE
	lpWide = (LPWSTR)lpSrc;
#else
	count = ::MultiByteToWideChar (::GetACP (), 0, lpSrc, nLength, NULL, 0);

	lpWide = new WCHAR[count + 1];
	memset (lpWide, 0, (count + 1) * sizeof(WCHAR));

	::MultiByteToWideChar (::GetACP (), 0, lpSrc, nLength, lpWide, count);
	nLength = count;
#endif

	count = ::WideCharToMultiByte (CP_UTF8, 0, lpWide, nLength, NULL, 0, NULL, 0);
	if (count > 1)
	{
		lpDst = new char[count + 1];
		memset (lpDst, 0, count + 1);

		::WideCharToMultiByte (CP_UTF8, 0, lpWide, nLength, lpDst, count, NULL, 0);
	}

#ifndef _UNICODE
	delete [] lpWide;
	lpWide = NULL;
#endif

	return count;
}
//*****************************************************************************
static int DigitCount (int nValue)
{
	ASSERT (nValue > 0);

	int nCount = 0;
	while (nValue > 0)
	{
		nValue /= 10;
		nCount ++;
	}

	if (nCount == 0)
	{
		nCount = 1;
	}

	return nCount;
}
//*****************************************************************************
HGLOBAL CBCGPGridCtrl::CopyHtmlToClipboardInternal ( LPCTSTR lpszText,
													 int nLen) const
{
	try
	{
		int cfid = ::RegisterClipboardFormat (_T("HTML Format"));

		if (cfid == 0)
		{
			TRACE0("CopyHtmlToClipboardInternal: can't register HTML format\n");
			return NULL;
		}

		//-----------
		// HTML text:
		//-----------
		CString strHtml;
		strHtml += _T("<HTML>\r\n");
		strHtml += _T("<BODY>\r\n");
		strHtml += _T("<!--StartFragment -->\r\n");

		LPSTR	lpHtml = NULL;
		int		nHtmlSize = StringToUTF8 (strHtml, lpHtml, strHtml.GetLength ());

		LPSTR	lpFragment = NULL;
		int		nFragmentSize = StringToUTF8 (lpszText, lpFragment, nLen);

		CString strEnd;
		strEnd += _T("<!--EndFragment -->\r\n");
		strEnd += _T("</BODY>\r\n");
		strEnd += _T("</HTML>\r\n");

		LPSTR	lpEnd = NULL;
		int		nEndSize = StringToUTF8 (strEnd, lpEnd, strEnd.GetLength ());

		//--------------------------
		// Clipboard context header:
		//--------------------------
		CString strStart;
		strStart += _T("Version:0.9\r\n");
		strStart += _T("StartHTML:00\r\n");
		strStart += _T("EndHTML:00\r\n");
		strStart += _T("StartFragment:00\r\n");
		strStart += _T("EndFragment:00\r\n");
		LPSTR	lpStart = NULL;
		int		nStartSize = StringToUTF8 (strStart, lpStart, strStart.GetLength ());

		int nExtraDigits = -8;
		nExtraDigits += DigitCount (nStartSize);
		nExtraDigits += DigitCount (nStartSize + nHtmlSize + nFragmentSize + nFragmentSize);
		nExtraDigits += DigitCount (nStartSize + nHtmlSize);
		nExtraDigits += DigitCount (nStartSize + nHtmlSize + nFragmentSize);
		nStartSize += nExtraDigits;

		CString strFormat;
		strStart.Empty ();
		strStart += _T("Version:0.9\r\n");
		strFormat.Format (_T("StartHTML:%d\r\n"), nStartSize);
		strStart += strFormat;
		strFormat.Format (_T("EndHTML:%d\r\n"), nStartSize + nHtmlSize + nFragmentSize + nFragmentSize);
		strStart += strFormat;
		strFormat.Format (_T("StartFragment:%d\r\n"), nStartSize + nHtmlSize);
		strStart += strFormat;
		strFormat.Format (_T("EndFragment:%d\r\n"), nStartSize + nHtmlSize + nFragmentSize);
		strStart += strFormat;

		delete [] lpStart;
		lpStart = NULL;
		nStartSize = StringToUTF8 (strStart, lpStart, strStart.GetLength ());

		//-----------------
		// Allocate memory:
		//-----------------
		int		nTextLen = nStartSize + nHtmlSize + nFragmentSize + nEndSize;
		SIZE_T	cbFinalSize = (nTextLen + 1) * sizeof(char);

		HGLOBAL hClipbuffer = ::GlobalAlloc (GMEM_DDESHARE, cbFinalSize);

		if (hClipbuffer == NULL)
		{
			TRACE0("CopyHtmlToClipboardInternal: out of memory\n");
			return NULL;
		}

		LPSTR lpBuffer = (LPSTR) GlobalLock (hClipbuffer);
		if (lpBuffer == NULL)
		{
			TRACE0("CopyHtmlToClipboardInternal: out of memory\n");
			GlobalFree (hClipbuffer);
			return NULL;
		}

		//---------------------
		// Build result string:
		//---------------------
		LPSTR lpDst = lpBuffer;
		memcpy (lpDst, lpStart, nStartSize * sizeof(char));
		lpDst += nStartSize;
		memcpy (lpDst, lpHtml, nHtmlSize * sizeof(char));
		lpDst += nHtmlSize;
		memcpy (lpDst, lpFragment, nFragmentSize * sizeof(char));
		lpDst += nFragmentSize;
		memcpy (lpDst, lpEnd, nEndSize * sizeof(char));
		lpDst += nEndSize;
 		lpBuffer [nTextLen] = '\0';

		delete [] lpStart;
		lpStart = NULL;
		delete [] lpHtml;
		lpHtml = NULL;
		delete [] lpFragment;
		lpFragment = NULL;
		delete [] lpEnd;
		lpEnd = NULL;

		if (hClipbuffer != NULL)
		{
			::GlobalUnlock (hClipbuffer);
			::SetClipboardData (cfid, hClipbuffer);
		}

		return hClipbuffer;

	}
	catch (...)
	{
		TRACE0("CopyHtmlToClipboardInternal: out of memory\n");
	}
	return NULL;	
}
//*****************************************************************************
HGLOBAL CBCGPGridCtrl::CopyTextToClipboardInternal ( LPCTSTR lpszText, 
													 int nLen) const
{
    HGLOBAL hClipbuffer = NULL;

	try
	{
		SIZE_T cbFinalSize = (nLen + 1) * sizeof(TCHAR);

		hClipbuffer = ::GlobalAlloc (GMEM_DDESHARE, cbFinalSize);

		if (hClipbuffer == NULL)
		{
			TRACE0("CopyTextToClipboardInternal: out of memory\n");
			return NULL;
		}

		LPTSTR lpBuffer = (LPTSTR) ::GlobalLock (hClipbuffer);
		if (lpBuffer == NULL)
		{
			TRACE0("CopyTextToClipboardInternal: out of memory\n");
			::GlobalFree (hClipbuffer);
			return NULL;
		}

		memcpy (lpBuffer, lpszText, nLen * sizeof(TCHAR));
		lpBuffer [nLen] = _T ('\0');

		::GlobalUnlock (hClipbuffer);
		::SetClipboardData (_TCF_TEXT, hClipbuffer);
	}
	catch (...)
	{
		TRACE0("CopyTextToClipboardInternal: out of memory\n");
	}

	return hClipbuffer;
}
//****************************************************************************
HGLOBAL CBCGPGridCtrl::CopySelectionToClipboardInternal (BOOL bCut)
{
	ASSERT_VALID (this);
	
	m_bCut = bCut;

	if (m_pSerializeManager == NULL)
	{
		return NULL;
	}
	
	ASSERT_VALID (m_pSerializeManager);
	
	try
	{
		if (!m_pSerializeManager->PrepareDataFromSelection ())
		{
			::CloseClipboard ();
			return FALSE;
		}
		
		CSharedFile globFile;
		
		if (!m_pSerializeManager->SerializeTo (globFile))
		{
			::CloseClipboard ();
			return FALSE;
		}
		
		HGLOBAL hBuffer = globFile.Detach ();
		::SetClipboardData (m_pSerializeManager->GetClipboardFormat (), hBuffer);

		m_pSerializeManager->CleanUp ();
		
		return hBuffer;
	}
	catch (COleException* pEx)
	{
		TRACE(_T("CopySelectionToClipboardInternal. OLE exception: %x\r\n"),
			pEx->m_sc);
		pEx->Delete ();
	}
	catch (CNotSupportedException *pEx)
	{
		TRACE(_T("CopySelectionToClipboardInternal. \"Not Supported\" exception\r\n"));
		pEx->Delete ();
	}
	catch (...)
	{
		TRACE0("CopySelectionToClipboardInternal: out of memory\n");
	}
	
	return NULL;	
}
//****************************************************************************
BOOL CBCGPGridCtrl::PasteFromDataObject (COleDataObject* pDataObject, 
										 CBCGPGridItemID idPasteTo, BOOL bRedraw)
{
	ASSERT_VALID (this);
	ASSERT (pDataObject != NULL);
	
	if (m_pSerializeManager == NULL || IsReadOnly ())
	{
		return FALSE;
	}
	
	ASSERT_VALID (m_pSerializeManager);
	
	if (!pDataObject->IsDataAvailable (m_pSerializeManager->GetClipboardFormat ()))
	{
		TRACE0("Incorrect clipboard format\n");
		return FALSE;
	}

	//------------------------
	// Save previous selection
	//------------------------
	CRect rectSaveSel = OnGetSelectionRect ();
	CArray<CBCGPGridRange, CBCGPGridRange> arrSaveSel;
	const int nSaveSelSize = GetSelectionCount ();

	arrSaveSel.SetSize (nSaveSelSize);
	int i;
	for (i = 0; i < nSaveSelSize; i++)
	{
		GetSelection (i, arrSaveSel [i]);
	}
	
	//---------------------------------------
	// Paste serialized data from DataObject:
	//---------------------------------------
	CFile* pFile = pDataObject->GetFileData (m_pSerializeManager->GetClipboardFormat ());
	if (pFile == NULL)
	{
		return FALSE;
	}
	
	BOOL bCanPaste = 
		m_pSerializeManager->SerializeFrom (*pFile) &&
		m_pSerializeManager->CanPaste (idPasteTo, m_bCut);
	
	delete pFile;
	
	if (!bCanPaste)
	{
		return FALSE;
	}
	
	BOOL bResult = m_pSerializeManager->Paste (idPasteTo, m_bCut);

	//--------------------------
	// Update previous selection
	//--------------------------
 	InvalidateRect (rectSaveSel);
	for (i = 0; i < arrSaveSel.GetSize (); i++)
	{
		if (!arrSaveSel [i].IsEmpty ())
		{
			CRect rect = GetRect (arrSaveSel [i]);
			InvalidateRect (rect);
		}
	}
	
	if (bRedraw)
	{
		UpdateWindow ();
	}
	
	return bResult;
}
//****************************************************************************
LRESULT CBCGPGridCtrl::OnPrintClient(WPARAM wp, LPARAM lp)
{
	DWORD dwFlags = (DWORD)lp;

	if (dwFlags & PRF_CLIENT)
	{
		CDC* pDC = CDC::FromHandle((HDC) wp);
		ASSERT_VALID(pDC);

		OnDraw(pDC);
	}

	return 0;
}
//****************************************************************************
LRESULT CBCGPGridCtrl::OnGridAdjustLayout(WPARAM /*wp*/, LPARAM /*lp*/)
{
	AdjustLayout ();
	m_bPostAdjustLayout = FALSE;
	return 0;
}
//****************************************************************************
LRESULT CBCGPGridCtrl::OnBCGUpdateToolTips (WPARAM wp, LPARAM)
{
	UINT nTypes = (UINT) wp;
	
	if (nTypes & BCGP_TOOLTIP_TYPE_GRID)
	{
#ifndef _BCGPGRID_STANDALONE
		CBCGPTooltipManager::CreateToolTip (m_pToolTip, this, BCGP_TOOLTIP_TYPE_GRID);
#else
		m_pToolTip = new CToolTipCtrl;
		m_pToolTip->Create (this);
#endif
		
		if (m_pToolTip->GetSafeHwnd () != NULL)
		{
			CRect rectDummy (0, 0, 0, 0);
			
			m_pToolTip->SetMaxTipWidth (640);
			
			m_pToolTip->AddTool (this, LPSTR_TEXTCALLBACK, &rectDummy, nIdToolTipHeader);
			m_pToolTip->SetToolRect (this, nIdToolTipHeader, m_rectHeader);
		}
	}
	
	return 0;
}
//****************************************************************************
BOOL CBCGPGridCtrl::OnNeedTipText(UINT /*id*/, NMHDR* pNMH, LRESULT* /*pResult*/)
{
	static CString strTipText;
	strTipText.Empty ();

	if (m_pToolTip->GetSafeHwnd () == NULL || 
		pNMH->hwndFrom != m_pToolTip->GetSafeHwnd ())
	{
		return FALSE;
	}

#ifndef _BCGPGRID_STANDALONE
	if (CBCGPPopupMenu::GetActiveMenu () != NULL)
	{
		return FALSE;
	}
#endif

	if (pNMH->idFrom == nIdToolTipHeader)
	{
		CPoint ptCursor;
		::GetCursorPos (&ptCursor);
		ScreenToClient (&ptCursor);
		
		CBCGPGridColumnsInfo::ClickArea clickAreaHeader;
		int nColumn = GetColumnsInfo ().HitTestColumn (ptCursor, FALSE, STRETCH_DELTA, &clickAreaHeader);
		
		if (nColumn >= 0 && 
			IsHeaderMenuButtonEnabled (nColumn) &&
			clickAreaHeader == CBCGPGridColumnsInfo::ClickHeaderButton)
		{
			GetHeaderMenuButtonTooltip (nColumn, strTipText);
		}
		else if (nColumn >= 0 && clickAreaHeader == CBCGPGridColumnsInfo::ClickHeader)
		{
			GetHeaderTooltip (nColumn, ptCursor, strTipText);
		}
	}

	if (strTipText.IsEmpty ())
	{
		return FALSE;
	}

	LPNMTTDISPINFO	pTTDispInfo	= (LPNMTTDISPINFO) pNMH;
	ASSERT((pTTDispInfo->uFlags & TTF_IDISHWND) == 0);

	pTTDispInfo->lpszText = const_cast<LPTSTR> ((LPCTSTR) strTipText);
 	return TRUE;
}
//****************************************************************************
BOOL CBCGPGridCtrl::OpenFindReplaceDlg (BOOL bFindDialogOnly)
{
	if (m_pFindDlg != NULL)
	{
		m_pFindDlg->SetFocus ();
		return TRUE;
	}

	ASSERT (bFindDialogOnly); // replace is not supported
	
	m_pFindDlg = new CBCGPGridFindDlg (this);
	
	OnPrepareFindString (m_strFindText);
	
	if (m_pFindDlg->Create (bFindDialogOnly, m_strFindText,
							NULL, 
							m_dwFindMask, this))
	{

	}
	else
	{
		delete m_pFindDlg;
		m_pFindDlg = NULL;

		return FALSE;
	}

	return TRUE;
}
//****************************************************************************
void CBCGPGridCtrl::CloseFindReplaceDlg ()
{
	if (m_pFindDlg->GetSafeHwnd () != NULL)
	{
		m_pFindDlg->PostMessage(WM_CLOSE);
	}
}
//****************************************************************************
LRESULT CBCGPGridCtrl::OnFindReplace(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	if (m_pFindDlg == NULL)
	{
		if (!Find(m_strFindText, m_dwFindMask, m_dwFindParamsEx))
		{
			OnTextNotFound(m_strFindText);
		}
		
		SetFocus();
	}
	else if (!m_pFindDlg->IsTerminating())
	{
		m_strFindText = m_pFindDlg->GetFindString();
		m_dwFindMask = m_pFindDlg->m_fr.Flags;
		m_dwFindParamsEx = BCGP_GRID_FINDREPLACE_PARAM::FR_LOOKIN_LABELS;

		if (IsPreviewRowEnabled ())
		{
			m_dwFindParamsEx |= BCGP_GRID_FINDREPLACE_PARAM::FR_LOOKIN_PREVIEW;
		}
		
		if (!Find(m_strFindText, m_dwFindMask, m_dwFindParamsEx))
		{
			OnTextNotFound(m_strFindText);
		}
		
		if (m_pFindDlg != NULL)
		{
			m_pFindDlg->SetFocus ();
		}
	}
	
	return 0;
}
//****************************************************************************
BOOL CBCGPGridCtrl::Find (const CString& strFind, DWORD dwFindMask, DWORD dwFindParamsEx)
{
	ASSERT_VALID (this);

	if (m_bVirtualMode)
	{
		return FALSE;
	}

	BCGP_GRID_FINDREPLACE_PARAM findParams (strFind);
	OnPrepareFindReplaceParams (findParams, dwFindMask, dwFindParamsEx);
	
	if ((dwFindMask & FR_REPLACE) != 0)
	{
		return FALSE;
	}
	else if ((dwFindMask & FR_REPLACEALL) != 0)
	{
		return FALSE;
	}
	else
	{
		CWaitCursor wait;
		CBCGPGridItemID id;
		
		if (!Search (id, id, findParams))
		{
			return FALSE;
		}
		
		ASSERT (!id.IsNull ());
		OnTextFound (findParams.lpszFind, id);
		
		return TRUE;
	}
}
//****************************************************************************
void CBCGPGridCtrl::OnPrepareFindReplaceParams (BCGP_GRID_FINDREPLACE_PARAM &params, 
												DWORD dwFindMask, DWORD dwFindParamsEx)
{
	params.bNext = (dwFindMask & FR_DOWN) != 0;
	params.bCase = (dwFindMask & FR_MATCHCASE) != 0;
	params.bWholeWord = (dwFindMask & FR_WHOLEWORD) != 0;
	
	params.bWholeCell = (dwFindParamsEx & BCGP_GRID_FINDREPLACE_PARAM::FR_WHOLECELL) != 0;
	params.scanOrder = ((dwFindParamsEx & BCGP_GRID_FINDREPLACE_PARAM::FR_SCANORDER_BYCOLS) != 0) 
		? BCGP_GRID_FINDREPLACE_PARAM::ByColumns 
		: BCGP_GRID_FINDREPLACE_PARAM::ByRows;
	params.nLookIn = dwFindParamsEx & BCGP_GRID_FINDREPLACE_PARAM::FR_LOOKIN_MASK;
	params.dwUserData = 0;
}
//****************************************************************************
void CBCGPGridCtrl::OnPrepareFindString (CString& /*strFind*/)
{

}
//****************************************************************************
void CBCGPGridCtrl::OnTextNotFound (LPCTSTR lpszFind)
{
	CWnd* pOwner = GetOwner ();
	if (pOwner == NULL)
	{
		return;
	}

	BCGPGRID_ITEM_INFO ii;
	memset (&ii, 0, sizeof (BCGPGRID_ITEM_INFO));
	ii.pItem = NULL;
	ii.nRow = -1;
	ii.nCol = -1;
	ii.dwResultCode = (DWORD_PTR)lpszFind;

	pOwner->SendMessage (BCGM_GRID_FIND_RESULT, GetDlgCtrlID (), LPARAM(&ii));
}
//****************************************************************************
void CBCGPGridCtrl::OnTextFound (LPCTSTR lpszFind, CBCGPGridItemID id)
{
	ASSERT_VALID (this);
	ASSERT (id.m_nRow >= 0);

	SetCurSel (id);

	CBCGPGridRow* pRow = GetCurSel ();
	if (pRow != NULL)
	{
		EnsureVisible (pRow);
	}

	if (id.m_nColumn >= 0)
	{
		EnsureVisibleColumn (id.m_nColumn);
	}

	CWnd* pOwner = GetOwner ();
	if (pOwner == NULL)
	{
		return;
	}
	
	BCGPGRID_ITEM_INFO ii;
	memset (&ii, 0, sizeof (BCGPGRID_ITEM_INFO));
	ii.pItem = NULL;
	ii.nRow = id.m_nRow;
	ii.nCol = id.m_nColumn;
	ii.dwResultCode = (DWORD_PTR)lpszFind;
	
	pOwner->SendMessage (BCGM_GRID_FIND_RESULT, GetDlgCtrlID (), LPARAM(&ii));
}
//****************************************************************************
CBCGPGridItemID CBCGPGridCtrl::GetNextItemID (CBCGPGridItemID id, 
											  BOOL bNext, 
											  BCGP_GRID_FINDREPLACE_PARAM::ScanOrder scanOrder) const
{
	ASSERT_VALID (this);

	if (id.IsNull ())
	{
		ASSERT (FALSE);
		return id;
	}

	int nColumnPos = (id.m_nColumn >= 0) ? GetColumnsInfo ().IndexToOrder (id.m_nColumn) : -1;

	const int nLastVisibleColumnPos = GetColumnsInfo ().GetColumnCount (TRUE) - 1;
	const int nFirstRow = 0;
	const int nLastRow = GetTotalItems () - 1;

	Direction dirVert = NoMove;
	Direction dirHorz = NoMove;

	if (scanOrder == BCGP_GRID_FINDREPLACE_PARAM::ByRows)
	{
		if (bNext)
		{
			if (nColumnPos >= 0 && nColumnPos < nLastVisibleColumnPos)
			{
				dirHorz = NextColumn;
			}
			else
			{
				dirHorz = FirstColumn;

				// next row:
				if (id.m_nRow >= 0 && id.m_nRow < nLastRow)
				{
					dirVert = NextRow;
				}
				else
				{
					dirVert = FirstRow;
				}
			}
		}
		else
		{
			if (nColumnPos > 0 && nColumnPos <= nLastVisibleColumnPos)
			{
				dirHorz = PrevColumn;
			}
			else
			{
				dirHorz = LastColumn;

				// prev row:
				if (id.m_nRow > 0 && id.m_nRow <= nLastRow)
				{
					dirVert = PrevRow;
				}
				else
				{
					dirVert = LastRow;
				}
			}
		}
	}
	else if (scanOrder == BCGP_GRID_FINDREPLACE_PARAM::ByColumns)
	{
		if (bNext)
		{
			if (id.m_nRow >= 0 && id.m_nRow < nLastRow)
			{
				dirVert = NextRow;
			}
			else
			{
				dirVert = FirstRow;

				// next column:
				if (nColumnPos >= 0 && nColumnPos < nLastVisibleColumnPos)
				{
					dirHorz = NextColumn;
				}
				else
				{
					dirHorz = FirstColumn;
				}
			}
		}
		else
		{
			if (id.m_nRow > 0 && id.m_nRow <= nLastRow)
			{
				dirVert = PrevRow;
			}
			else
			{
				dirVert = LastRow;

				// prev column
				if (nColumnPos > 0 && nColumnPos <= nLastVisibleColumnPos)
				{
					dirHorz = PrevColumn;
				}
				else
				{
					dirHorz = LastColumn;
				}
			}
		}
	}

	CBCGPGridItemID idNext = id;

	switch (dirVert)
	{
	case PrevRow:
		idNext.m_nRow --;
		break;
	case NextRow:
		idNext.m_nRow ++;
		break;
	case FirstRow:
		idNext.m_nRow = nFirstRow;
		break;
	case LastRow:
		idNext.m_nRow = nLastRow;
		break;
	}

	switch (dirHorz)
	{
	case PrevColumn:
		idNext.m_nColumn = GetColumnsInfo ().OrderToIndex (--nColumnPos);
		break;
	case NextColumn:
		idNext.m_nColumn = GetColumnsInfo ().OrderToIndex (++nColumnPos);
		break;
	case FirstColumn:
		idNext.m_nColumn = GetColumnsInfo ().GetFirstVisibleColumn ();
		break;
	case LastColumn:
		idNext.m_nColumn = GetColumnsInfo ().GetLastVisibleColumn ();
		break;
	}

	return idNext;
}
//****************************************************************************
BOOL CBCGPGridCtrl::Search (CBCGPGridItemID &idPos, CBCGPGridItemID idStart, const BCGP_GRID_FINDREPLACE_PARAM &params)
{
	ASSERT_VALID (this);

	if (params.lpszFind == NULL || params.lpszFind [0] == 0 || m_bVirtualMode)
	{
		return FALSE;
	}

	CBCGPGridItemID idDefault = (GetCurSelItemID ().IsNull ()) ? CBCGPGridItemID (0, 0) : GetCurSelItemID ();

	if (idStart.IsNull ())
	{
		// use current selection
		idStart = idDefault;
	}

	CBCGPGridItemID idStop = idStart;
	idPos = idStart = GetNextItemID (idStart, params.bNext, params.scanOrder);

	if (idStart.m_nRow < 0)
	{
		idPos.m_nRow = idStart.m_nRow = idStop.m_nRow = (params.bNext) ? 0 : GetTotalItems () - 1;
	}
	
	if (!IsValidID (idStart))
	{
		return FALSE;
	}

	//-------------
	// Scan by rows:
	//-------------
	if (params.scanOrder == BCGP_GRID_FINDREPLACE_PARAM::ByRows)
	{
		BOOL bShowAllItems = (IsSortingMode () && !IsGrouping ());
		
		const CList<CBCGPGridRow*, CBCGPGridRow*>& lst =
			(!IsSortingMode () && !IsGrouping ()) ? m_lstItems : m_lstTerminalItems;
		
		POSITION pos = lst.FindIndex (idStart.m_nRow);
		for (int i = idStart.m_nRow; pos != NULL; i++)
		{
			CBCGPGridRow* pRow = (params.bNext) ? lst.GetNext (pos) : lst.GetPrev (pos);
			ASSERT_VALID (pRow);
			
			if (bShowAllItems ? 
				!pRow->IsItemFiltered () : pRow->IsItemVisible ()) // show item
			{
				//------------
				// Search row:
				//------------
				if ((params.nLookIn & BCGP_GRID_FINDREPLACE_PARAM::FR_LOOKIN_LABELS) != 0)
				{
					idPos.m_nRow = pRow->GetRowId ();
					if (SearchRow (pRow, idPos.m_nColumn, idStart.m_nColumn, params) != NULL)
					{
						return TRUE;
					}
					
					idStart.m_nColumn = (params.bNext) ? 0 : GetColumnsInfo ().GetLastVisibleColumn ();
				}

				if ((params.nLookIn & BCGP_GRID_FINDREPLACE_PARAM::FR_LOOKIN_PREVIEW) != 0)
				{
					int nPosPreview = -1;
					if (SearchPreview (pRow, nPosPreview, 0, params))
					{
						idPos.m_nRow = pRow->GetRowId ();
						idPos.m_nColumn = -1;
						return TRUE;
					}
				}
			}
		}
	}

	//-----------------
	// Scan by columns:
	//-----------------
	else if (params.scanOrder == BCGP_GRID_FINDREPLACE_PARAM::ByColumns &&
			 (params.nLookIn & BCGP_GRID_FINDREPLACE_PARAM::FR_LOOKIN_LABELS) != 0)
	{
		BOOL bInRange = FALSE;
		
		int nPos = (params.bNext) ? GetColumnsInfo ().Begin () : GetColumnsInfo ().GetColumnCount (TRUE) - 1;
		while ((params.bNext) ? nPos != GetColumnsInfo ().End () : nPos >= 0)
		{
			int iColumn = (params.bNext) ? GetColumnsInfo ().Next (nPos) : GetColumnsInfo ().OrderToIndex (nPos--); 
			if (iColumn == -1)
			{
				break; // no more visible columns
			}
			
			BOOL bIsRangeBound = (iColumn == idStart.m_nColumn);
			
			if (bIsRangeBound || bInRange)
			{
				//---------------
				// Search column:
				//---------------
				idPos.m_nColumn = iColumn;
				if (SearchColumn (iColumn, idPos.m_nRow, idStart.m_nRow, params))
				{
					return TRUE;
				}

				idStart.m_nRow = (params.bNext) ? 0 : GetTotalRowCount () - 1;
			}
			
			if (bIsRangeBound)
			{
				bInRange = TRUE;
			}
		}
	}

	return FALSE;
}
//****************************************************************************
CBCGPGridItem* CBCGPGridCtrl::SearchRow (CBCGPGridRow* pRow, int &iPos, int iStart,
										 const BCGP_GRID_FINDREPLACE_PARAM &params)
{
	ASSERT_VALID (pRow);

	if (!pRow->HasValueField ())
	{
		return NULL;
	}

	BOOL bInRange = FALSE;

	int nPos = (params.bNext) ? GetColumnsInfo ().Begin () : GetColumnsInfo ().GetColumnCount (TRUE) - 1;
	while ((params.bNext) ? nPos != GetColumnsInfo ().End () : nPos >= 0)
	{
		int iColumn = (params.bNext) ? GetColumnsInfo ().Next (nPos) : GetColumnsInfo ().OrderToIndex (nPos--); 
		if (iColumn == -1)
		{
			break; // no more visible columns
		}
		
		BOOL bIsRangeBound = (iColumn == iStart);
		
		if (bIsRangeBound || bInRange)
		{
			//-------------
			// Search item:
			//-------------
			CBCGPGridItem* pItem = pRow->GetItem (iColumn);

			if (pItem != NULL)
			{
				iPos = iColumn;
				if (SearchItem (pItem, params))
				{
					return pItem;
				}
			}
		}
		
		if (bIsRangeBound)
		{
			bInRange = TRUE;
		}
	}

	return NULL;
}
//****************************************************************************
CBCGPGridItem* CBCGPGridCtrl::SearchColumn (int nColumn, int &nPos, int nStart,
											const BCGP_GRID_FINDREPLACE_PARAM &params)
{
	ASSERT_VALID (this);

	if (nColumn < 0 || nColumn >= GetColumnCount ())
	{
		return NULL;
	}

	BOOL bShowAllItems = (IsSortingMode () && !IsGrouping ());
	
	const CList<CBCGPGridRow*, CBCGPGridRow*>& lst =
		(!IsSortingMode () && !IsGrouping ()) ? m_lstItems : m_lstTerminalItems;
	
	POSITION pos = lst.FindIndex (nStart);
	for (int i = nStart; pos != NULL; i++)
	{
		CBCGPGridRow* pRow = (params.bNext) ? lst.GetNext (pos) : lst.GetPrev (pos);
		ASSERT_VALID (pRow);
		
		if (bShowAllItems ? 
			!pRow->IsItemFiltered () : pRow->IsItemVisible ()) // show item
		{
			//-------------
			// Search item:
			//-------------
			CBCGPGridItem* pItem = pRow->GetItem (nColumn);

			if (pItem != NULL)
			{
				nPos = pRow->GetRowId ();
				if (SearchItem (pItem, params))
				{
					return pItem;
				}
			}
		}
	}

	return NULL;
}
//****************************************************************************
BOOL CBCGPGridCtrl::SearchItem (CBCGPGridItem* pItem, const BCGP_GRID_FINDREPLACE_PARAM &params)
{
	ASSERT_VALID (pItem);
	ASSERT (params.lpszFind != NULL);

	if (params.bCase && !params.bWholeWord && !params.bWholeCell)
	{
		return (pItem->GetLabel ().Find (params.lpszFind) != -1);
	}

	CString strLabel = pItem->GetLabel ();
	CString strFind = params.lpszFind;
	
	if (!params.bCase)
	{
		strLabel.MakeUpper ();
		strFind.MakeUpper ();
	}

	if (params.bWholeCell)
	{
		return (strLabel.Compare (strFind) == 0);
	}
	
	if (!params.bWholeWord || params.bStartWith)
	{
		int nRes = strLabel.Find (strFind);
		return params.bStartWith ? (nRes == 0) : (nRes >= 0);
	}

	CString strWordDelimeters = _T(" \t\n,./?<>;:\"'{[}]~`%^&*()-+=!");
	const int nFindLen = strFind.GetLength();

	int nPos = -1;
	int nStartOffset = 0;
	do 
	{
		nPos = strLabel.Find (strFind, nStartOffset);
		
		if (nPos == -1)
		{
			return FALSE;
		}

		// skip all substrings which are not embraced with delimiters
 		int nPos1 = (nPos > 0)? strWordDelimeters.Find(strLabel.GetAt(nPos - 1)): 0;
 		int nPos2 = (nPos + nFindLen < strLabel.GetLength())? strWordDelimeters.Find(strLabel.GetAt(nPos + nFindLen)): 0;

		if (nPos1 != -1 && nPos2 != -1)
		{
			return TRUE;
		}

		nStartOffset = nPos + nFindLen;
	}
	while (nPos >= 0 && nFindLen > 0 && nStartOffset < strLabel.GetLength());

	return FALSE;
}
//****************************************************************************
BOOL CBCGPGridCtrl::SearchPreview (CBCGPGridRow* pRow, int &nPos, int nStart,
								   const BCGP_GRID_FINDREPLACE_PARAM &params)
{
	ASSERT_VALID (pRow);

	CString str;
	pRow->GetPreviewText (str);

	if (!params.bCase)
	{
		str.MakeUpper ();
		
		CString strFind = params.lpszFind;
		strFind.MakeUpper ();
		
		return (str.Find (strFind) != -1);
	}

	nPos = str.Find (params.lpszFind, nStart);
	return (nPos != -1);
}
//****************************************************************************
LRESULT CBCGPGridCtrl::OnBCGSetControlVMMode (WPARAM wp, LPARAM)
{
	m_bVisualManagerStyle = (BOOL) wp;
	return 0;
}
//****************************************************************************
BOOL CBCGPGridCtrl::SetExportTextSeparator (DWORD dwExportFlags, CString& strSeparator)
{
	if ((dwExportFlags & Format_CSV) != 0)
	{
		m_strExportCSVSeparator = strSeparator;
		return TRUE;
	}

	return FALSE;
}
//*****************************************************************************
BOOL CBCGPGridCtrl::OnSetAccData (long lVal)
{
	ASSERT_VALID (this);

	if (lVal == 0)
	{
		return FALSE;
	}

	CPoint pt(BCG_GET_X_LPARAM(lVal), BCG_GET_Y_LPARAM(lVal));
	ScreenToClient (&pt);

	CBCGPGridItemID id;
	CBCGPGridItem* pItem = NULL;
	CBCGPGridRow* pRow = HitTest(pt, id, pItem);
	if (pRow == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	m_AccData.Clear ();

	ASSERT_VALID (pRow);

	if (pItem != NULL)
	{
		ASSERT_VALID (pItem);
		pItem->SetACCData (this, m_AccData);
		return TRUE;
	}

	pRow->SetACCData (this, m_AccData);

	return TRUE;
}
//*****************************************************************************
LRESULT CBCGPGridCtrl::OnGetObject(WPARAM wParam, LPARAM lParam)
{
	return CBCGPWnd::OnGetObject (wParam, lParam);
}
//*****************************************************************************
HRESULT CBCGPGridCtrl::get_accChildCount(long *pcountChildren)
{
	if (!pcountChildren)
	{
		return E_INVALIDARG;
	}

	*pcountChildren = 0;
	return S_OK;
}
//*****************************************************************************
HRESULT CBCGPGridCtrl::get_accChild(VARIANT /*varChild*/, IDispatch **ppdispChild)
{
	if (!(*ppdispChild))
	{
		return E_INVALIDARG;
	}

	if (m_pStdObject != NULL)
	{
		*ppdispChild = m_pStdObject;
	}
	else
	{
		*ppdispChild = NULL;
	}
	return S_OK;
}
//*****************************************************************************
HRESULT CBCGPGridCtrl::get_accName(VARIANT varChild, BSTR *pszName)
{
	if ((varChild.vt == VT_I4) && (varChild.lVal == CHILDID_SELF))
	{
		CString strText;
		GetWindowText(strText);
		if (strText.GetLength() == 0)
		{
			*pszName  = SysAllocString(L"GridControl");
			return S_OK;
		}

		*pszName = strText.AllocSysString();
		return S_OK;
	}

	if (m_pAccItem != NULL)
	{
		ASSERT_VALID (m_pAccItem);

		CString strName = m_pAccItem->FormatItem ();
		*pszName = strName.AllocSysString();
		return S_OK;
	}

	if (m_pAccRow != NULL)
	{
		ASSERT_VALID(m_pAccRow);

		BOOL bHasItems = m_pAccRow->HasValueField () && m_pAccRow->GetItemCount () > 0;
		if (m_pAccRow->IsGroup() || !bHasItems)
		{
			CString strName = m_pAccRow->GetName();
			*pszName = strName.AllocSysString();
			return S_OK;
		}
		else
		{
			CString strName = m_pAccRow->FormatItem();
			*pszName = strName.AllocSysString();
			return S_OK;
		}
	}

	return S_FALSE;
}
//*****************************************************************************
HRESULT CBCGPGridCtrl::get_accValue(VARIANT varChild, BSTR *pszValue)
{
	if ((varChild.vt == VT_I4) && (varChild.lVal == CHILDID_SELF))
	{
		return S_FALSE;
	}

	if (m_pAccItem != NULL)
	{
		ASSERT_VALID (m_pAccItem);

		CString strName = m_pAccItem->FormatItem();
		*pszValue = strName.AllocSysString();
		return S_OK;
	}

	if (m_pAccRow != NULL)
	{
		BOOL bHasItems = m_pAccRow->HasValueField () && m_pAccRow->GetItemCount () > 0;
		if (m_pAccRow->IsGroup() || !bHasItems)
		{
			CString strName = m_pAccRow->GetName();
			*pszValue = strName.AllocSysString();
			return S_OK;
		}
		else
		{
			CString strValue = m_pAccRow->FormatItem();
			*pszValue = strValue.AllocSysString();
			return S_OK;
		}
	}

	return S_FALSE;
}
//*****************************************************************************
HRESULT CBCGPGridCtrl::get_accDescription(VARIANT varChild, BSTR *pszDescription)
{
	if (((varChild.vt != VT_I4) && (varChild.lVal != CHILDID_SELF)) || (NULL == pszDescription))
	{
		return E_INVALIDARG;
	}

	if ((varChild.vt == VT_I4) && (varChild.lVal == CHILDID_SELF))
	{
		*pszDescription = SysAllocString(L"GridControl");
		return S_OK;
	}

	if (m_pAccItem != NULL)
	{
		ASSERT_VALID (m_pAccItem);

		CBCGPGridItemID id = m_pAccItem->GetGridItemID ();
		CString strName;
		strName.Format (_T("Row %d, Column %d"), id.m_nRow + 1, id.m_nColumn + 1);
		*pszDescription = strName.AllocSysString();
		return S_OK;
	}

	if (m_pAccRow != NULL)
	{
		CBCGPReportRow* pReportRow = DYNAMIC_DOWNCAST (CBCGPReportRow, m_pAccRow);

		if (pReportRow != NULL)
		{
			CString strName = pReportRow->GetDescription();
			*pszDescription = strName.AllocSysString();
			return S_OK;
		}

		CString strName;
		strName.Format (_T("Row %d"), m_pAccRow->GetRowId () + 1);
		*pszDescription = strName.AllocSysString();
	}

	return S_OK;
}
//*****************************************************************************
HRESULT CBCGPGridCtrl::get_accRole(VARIANT varChild, VARIANT *pvarRole)
{
	if (!pvarRole || ((varChild.vt != VT_I4) && (varChild.lVal != CHILDID_SELF)))
	{
		return E_INVALIDARG;
	}

	if ((varChild.vt == VT_I4) && (varChild.lVal == CHILDID_SELF))
	{
		pvarRole->vt = VT_I4;
		pvarRole->lVal = ROLE_SYSTEM_TABLE;
		return S_OK;
	}

	if (m_pAccItem != NULL)
	{
		pvarRole->vt = VT_I4;
		pvarRole->lVal = ROLE_SYSTEM_CELL;
		return S_OK;
	}

	pvarRole->vt = VT_I4;
	pvarRole->lVal = ROLE_SYSTEM_ROW;

	return S_OK;
}
//*****************************************************************************
HRESULT CBCGPGridCtrl::get_accState(VARIANT varChild, VARIANT *pvarState)
{
	if ((varChild.vt == VT_I4) && (varChild.lVal == CHILDID_SELF))
	{
		pvarState->vt = VT_I4;
		pvarState->lVal = STATE_SYSTEM_NORMAL;
		return S_OK;
	}

	pvarState->vt = VT_I4;
	pvarState->lVal = STATE_SYSTEM_FOCUSABLE;
	pvarState->lVal |= STATE_SYSTEM_SELECTABLE;
	
	if (m_pAccItem != NULL)
	{
		if (m_pAccItem->IsSelected())
		{
			pvarState->lVal |= STATE_SYSTEM_FOCUSED;
			pvarState->lVal |= STATE_SYSTEM_SELECTED;
		}

		if (!m_pAccItem->IsEnabled() || m_pAccItem->IsReadOnly())
		{
			pvarState->lVal |= STATE_SYSTEM_READONLY;
		}

		return S_OK;
	}

	if (m_pAccRow != NULL)
	{
		if (m_pAccRow->IsSelected())
		{
			pvarState->lVal |= STATE_SYSTEM_FOCUSED;
			pvarState->lVal |= STATE_SYSTEM_SELECTED;
		}

		if (m_pAccRow->IsGroup())
		{
			if (m_pAccRow->IsExpanded ())
			{
				pvarState->lVal |= STATE_SYSTEM_EXPANDED;
			}
			else
			{
				pvarState->lVal |= STATE_SYSTEM_COLLAPSED;
			}
		}

		if (!m_pAccRow->IsEnabled() || (m_pAccRow->IsGroup() && !m_pAccRow->HasValueField ()))
		{
			pvarState->lVal |= STATE_SYSTEM_READONLY;
		}
	}

	return S_OK;
}
//*****************************************************************************
HRESULT CBCGPGridCtrl::get_accHelp(VARIANT /*varChild*/, BSTR * /*pszHelp*/)
{
	return S_FALSE;
}
//*****************************************************************************
HRESULT CBCGPGridCtrl::get_accHelpTopic(BSTR * /*pszHelpFile*/, VARIANT /*varChild*/, long * /*pidTopic*/)
{
	return S_FALSE;
}
//*****************************************************************************
HRESULT CBCGPGridCtrl::get_accKeyboardShortcut(VARIANT /*varChild*/, BSTR * /*pszKeyboardShortcut*/)
{
	return S_FALSE;
}
//*****************************************************************************
HRESULT CBCGPGridCtrl::get_accFocus(VARIANT *pvarChild)
{
	if (NULL == pvarChild)
	{
		return E_INVALIDARG;
	}

	return DISP_E_MEMBERNOTFOUND;
}
//*****************************************************************************
HRESULT CBCGPGridCtrl::get_accSelection(VARIANT *pvarChildren)
{
	if (NULL == pvarChildren)
	{
		return E_INVALIDARG;
	}
	return DISP_E_MEMBERNOTFOUND;
}
//*****************************************************************************
HRESULT CBCGPGridCtrl::get_accDefaultAction(VARIANT /*varChild*/, BSTR * /*pszDefaultAction*/)
{
	return DISP_E_MEMBERNOTFOUND; 
}
//*****************************************************************************
HRESULT CBCGPGridCtrl::accSelect(long flagsSelect, VARIANT varChild)
{
	if (m_pStdObject != NULL)
	{
		return m_pStdObject->accSelect(flagsSelect, varChild);
	}
	return E_INVALIDARG;
}
//*****************************************************************************
HRESULT CBCGPGridCtrl::accLocation(long *pxLeft, long *pyTop, long *pcxWidth, long *pcyHeight, VARIANT varChild)
{
	HRESULT hr = S_OK;

	if (!pxLeft || !pyTop || !pcxWidth || !pcyHeight)
	{
		return E_INVALIDARG;
	}

	if ((varChild.vt == VT_I4) && (varChild.lVal == CHILDID_SELF))
	{
		CRect rc;
		GetWindowRect(rc);

		*pxLeft = rc.left;
		*pyTop = rc.top;
		*pcxWidth = rc.Width();
		*pcyHeight = rc.Height();

		return S_OK;
	}
	else
	{
		if (m_pAccItem != NULL)
		{
			CRect rcItem = m_pAccItem->GetRect ();
			ClientToScreen(&rcItem);
			*pxLeft = rcItem.left;
			*pyTop = rcItem.top;
			*pcxWidth = rcItem.Width();
			*pcyHeight = rcItem.Height();
		}

		else if (m_pAccRow != NULL)
		{
			CRect rcRow = m_pAccRow->GetRect ();
			ClientToScreen(&rcRow);
			*pxLeft = rcRow.left;
			*pyTop = rcRow.top;
			*pcxWidth = rcRow.Width();
			*pcyHeight = rcRow.Height();
		}
	}

	return hr;
}
//*****************************************************************************
HRESULT CBCGPGridCtrl::accHitTest(long xLeft, long yTop, VARIANT *pvarChild)
{
	if (!pvarChild)
	{
		return E_INVALIDARG;
	}

	CPoint pt(xLeft, yTop);
	ScreenToClient(&pt);

	CBCGPGridItemID id;
	CBCGPGridItem* pItem = NULL;
	CBCGPGridRow* pRow = HitTest(pt, id, pItem);
	if (pRow != NULL)
	{
		LPARAM lParam = MAKELPARAM((WORD)xLeft, (WORD)yTop);
		pvarChild->vt = VT_I4;
		pvarChild->lVal = (LONG)lParam;
	}
	else
	{
		pvarChild->vt = VT_I4;
		pvarChild->lVal = CHILDID_SELF;
	}

	m_pAccRow = pRow;
	m_pAccItem = pItem;
	return S_OK;
}
//*****************************************************************************
void CBCGPGridCtrl::NotifyAccessibility (CBCGPGridRow* pRow, CBCGPGridItem* pItem)
{
	if (!globalData.IsAccessibilitySupport () || pRow == NULL)
	{
		return;
	}

	m_pAccRow = pRow;
	m_pAccItem = pItem;

	CPoint pt(pRow->m_Rect.left, pRow->m_Rect.top);
	if (pItem != NULL)
	{
		pt = CPoint(pItem->m_Rect.left, pItem->m_Rect.top);
	}
	ClientToScreen(&pt);
	LPARAM lParam = MAKELPARAM(pt.x, pt.y);

#ifndef _BCGSUITE_
	globalData.NotifyWinEvent (EVENT_OBJECT_FOCUS, GetSafeHwnd(), OBJID_CLIENT, (LONG)lParam);
#else
	::NotifyWinEvent(EVENT_OBJECT_FOCUS, GetSafeHwnd(), OBJID_CLIENT, (LONG)lParam);
#endif
}
//*****************************************************************************************
void CBCGPGridDataStateIcons::RemoveAll ()
{
	CBCGPGridDataStateIconSet* pIconset;
	int nColumn;

	for (POSITION pos = m_mapStateImages.GetStartPosition (); pos != NULL; )
	{
		m_mapStateImages.GetNextAssoc (pos, nColumn, pIconset);
		delete pIconset;
	}

	m_mapStateImages.RemoveAll();
}
//*****************************************************************************************
void CBCGPGridDataStateIcons::Set (int nColumn, CBCGPToolBarImages* pImages, BOOL bReverseOrder, CBCGPGridDataStateIconSet::ImagePlacementHorz placement)
{
	if (pImages == NULL && nColumn == -1)
	{
		RemoveAll ();
		return;
	}
	
	CBCGPGridDataStateIconSet* pIconset;
	if (m_mapStateImages.Lookup(nColumn, pIconset))
	{
		delete pIconset;
		m_mapStateImages.RemoveKey(nColumn);
	}
	
	if (pImages == NULL || !pImages->IsValid())
	{
		return;
	}
	
	ASSERT_VALID (pImages);

	pIconset = new CBCGPGridDataStateIconSet;

	if (!pIconset->SetImages (pImages, bReverseOrder, placement) )
	{
		ASSERT(FALSE);

		delete pIconset;
		return;
	}
	
	m_mapStateImages.SetAt (nColumn, pIconset);
}
//*****************************************************************************************
CBCGPToolBarImages* CBCGPGridDataStateIcons::Get (int nColumn, CBCGPGridDataStateIconSet** ppInfo) const
{
	CBCGPGridDataStateIconSet* pIconset = NULL;
	
	if (nColumn != -1 && m_mapStateImages.Lookup(nColumn, pIconset) && pIconset != NULL)
	{
		if (ppInfo != NULL)
		{
			*ppInfo = pIconset;
		}

		return &pIconset->m_StateImages;
	}
	
	if (m_mapStateImages.Lookup(-1, pIconset) && pIconset != NULL)
	{
		if (ppInfo != NULL)
		{
			*ppInfo = pIconset;
		}
		
		return &pIconset->m_StateImages;
	}
	
	return NULL;
}
//*****************************************************************************************
void CBCGPGridCtrl::SetCustomDataIconSet (CBCGPToolBarImages* pImages, int nColumn, BOOL bReverseOrder, CBCGPGridDataStateIconSet::ImagePlacementHorz placement)
{
	m_mapStateIcons.Set (nColumn, pImages, bReverseOrder, placement);
}
//*****************************************************************************************
CBCGPToolBarImages* CBCGPGridCtrl::GetCustomDataIconSet (int nColumn, CBCGPGridDataStateIconSet** ppInfo) const
{
	return m_mapStateIcons.Get (nColumn, ppInfo);
}
//*****************************************************************************************
void CBCGPGridDataBarColors::RemoveAll ()
{
	BCGP_GRID_COLOR_DATA::ColorData* pColors;
	int nColumn;
	
	for (POSITION pos = m_mapDataBarColors.GetStartPosition (); pos != NULL; )
	{
		m_mapDataBarColors.GetNextAssoc (pos, nColumn, pColors);
		delete pColors;
	}
	
	m_mapDataBarColors.RemoveAll();
}
//*****************************************************************************************
void CBCGPGridDataBarColors::Set (int nColumn, BCGP_GRID_COLOR_DATA::ColorData*	pNewColors)
{
	if (pNewColors == NULL && nColumn == -1)
	{
		RemoveAll ();
		return;
	}
	
	BCGP_GRID_COLOR_DATA::ColorData* pColors;
	if (m_mapDataBarColors.Lookup(nColumn, pColors))
	{
		delete pColors;
		m_mapDataBarColors.RemoveKey(nColumn);
	}
	
	if (pNewColors == NULL)
	{
		return;
	}
	
	m_mapDataBarColors.SetAt (nColumn, pNewColors);
}
//*****************************************************************************************
BCGP_GRID_COLOR_DATA::ColorData* CBCGPGridDataBarColors::Get (int nColumn) const
{
	BCGP_GRID_COLOR_DATA::ColorData* pColors = NULL;
	
	if (nColumn != -1 && m_mapDataBarColors.Lookup(nColumn, pColors) && pColors != NULL)
	{
		return pColors;
	}
	
	if (m_mapDataBarColors.Lookup(-1, pColors))
	{
		return pColors;
	}
	
	return NULL;
}
//*****************************************************************************************
void CBCGPGridCtrl::SetCustomDataBarColors (int nColumn, COLORREF clrBorder, COLORREF clrFill, COLORREF clrGradient, int nGradientAngle)
{
	BCGP_GRID_COLOR_DATA::ColorData* pNewColors = NULL;

	if (clrBorder != (COLORREF)-1 || clrFill != (COLORREF)-1)
	{
		pNewColors = new BCGP_GRID_COLOR_DATA::ColorData;
		pNewColors->InitColors ();

		pNewColors->m_clrBorder = clrBorder;;
		pNewColors->m_clrBackground = clrFill;
		pNewColors->m_clrGradient = clrGradient;
		pNewColors->m_nGradientAngle = nGradientAngle;
	}

	m_mapDataBarColors.Set(nColumn, pNewColors);
}
//*****************************************************************************************
BCGP_GRID_COLOR_DATA::ColorData* CBCGPGridCtrl::GetCustomDataBarColors (int nColumn) const
{
	return m_mapDataBarColors.Get (nColumn);
}


/////////////////////////////////////////////////////////////////////////////
// CBCGPHeaderItemDragWnd

CString	CBCGPHeaderItemDragWnd::m_strClassName;

CBCGPHeaderItemDragWnd::CBCGPHeaderItemDragWnd()
{
	m_pWndGrid = NULL;
	m_nItem = -1;
	m_bDrop = TRUE;

	if (m_strClassName.IsEmpty ())
	{
		m_strClassName = ::AfxRegisterWndClass (
			CS_SAVEBITS,
			::LoadCursor(NULL, IDC_ARROW),
			(HBRUSH)(COLOR_BTNFACE + 1), NULL);
	}
}

CBCGPHeaderItemDragWnd::~CBCGPHeaderItemDragWnd()
{
}

BEGIN_MESSAGE_MAP(CBCGPHeaderItemDragWnd, CWnd)
	//{{AFX_MSG_MAP(CBCGPHeaderItemDragWnd)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_SETCURSOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGPHeaderItemDragWnd message handlers

BOOL CBCGPHeaderItemDragWnd::Create (CBCGPGridCtrl* pGrid, int nItem)
{
	ASSERT_VALID(pGrid);
	ASSERT(nItem >= 0);

	m_pWndGrid = pGrid;
	m_nItem = nItem;

	CRect rect;
	pGrid->GetColumnsInfo ().GetColumnRect (nItem, rect);

	pGrid->ClientToScreen (&rect);

	if (m_pWndGrid->m_bDragGroupItem || m_pWndGrid->m_bDragFromChooser)
	{
		CString strLabel = m_pWndGrid->GetColumnsInfo ().GetColumnName (m_nItem);

		CClientDC dc (m_pWndGrid);
		HFONT hfontOld = m_pWndGrid->SetCurrFont (&dc);

		CSize sizeText = dc.GetTextExtent (strLabel);

		if (m_pWndGrid->m_bDragGroupItem)
		{
			rect.right = rect.left + sizeText.cx + 
				BCGPGRID_GROUPBYBOX_COLUMNWIDTH + TEXT_MARGIN;
		}
		else
		{
			int nWidth = (pGrid->m_pColumnChooser != NULL) ? 
				pGrid->m_pColumnChooser->GetColumnWidth () : 0;
			rect.right = rect.left + max (sizeText.cx + 20, nWidth);
		}
		rect.bottom = rect.top + sizeText.cy + 10;

		::SelectObject (dc.GetSafeHdc (), hfontOld);
	}

	return CreateEx (WS_EX_TOOLWINDOW | WS_EX_TOPMOST, 
		m_strClassName, NULL, WS_POPUP | WS_DISABLED,
		rect, NULL, 0);
}
//*******************************************************************************
void CBCGPHeaderItemDragWnd::OnPaint() 
{
	ASSERT_VALID(m_pWndGrid);

	CPaintDC dc(this); // device context for painting
	
	CRect rectClient;
	GetClientRect (rectClient);

	dc.FillRect (rectClient, &globalData.brBtnFace);

	HFONT hfontOld = m_pWndGrid->SetCurrFont (&dc);

	dc.SetTextColor (globalData.clrBtnText);
	dc.SetBkMode (TRANSPARENT);

	if (m_pWndGrid->m_bDragGroupItem || m_pWndGrid->m_bDragFromChooser)
	{
		CRect rectLabel = rectClient;
		rectLabel.DeflateRect (5, 0);

		CString strLabel = m_pWndGrid->GetColumnsInfo ().GetColumnName (m_nItem);
		UINT uiTextFlags = DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX;

		dc.DrawText (strLabel, rectLabel, uiTextFlags);

		dc.Draw3dRect (rectClient,
			globalData.clrBtnHilite,
			globalData.clrBtnShadow);
	}
	else
	{
		m_pWndGrid->GetColumnsInfo ().m_bDrawingDraggedColumn = TRUE;
		m_pWndGrid->GetColumnsInfo ().DrawColumn (&dc, m_nItem, rectClient, 5, 5, FALSE, TRUE, TRUE);
		m_pWndGrid->GetColumnsInfo ().m_bDrawingDraggedColumn = FALSE;
	}

	::SelectObject (dc.GetSafeHdc (), hfontOld);
}
//*******************************************************************************
BOOL CBCGPHeaderItemDragWnd::OnEraseBkgnd(CDC* /*pDC*/) 
{
	return TRUE;
}
//*******************************************************************************
void CBCGPHeaderItemDragWnd::PostNcDestroy() 
{
	CWnd::PostNcDestroy();
	delete this;
}
//*******************************************************************************
BOOL CBCGPHeaderItemDragWnd::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	if (!m_bDrop)
	{
		SetCursor (NULL);
		return TRUE;
	}
	
	return CWnd::OnSetCursor(pWnd, nHitTest, message);
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPHeaderItemDropWnd

CString	CBCGPHeaderItemDropWnd::m_strClassName;

CBCGPHeaderItemDropWnd::CBCGPHeaderItemDropWnd()
{
	if (m_strClassName.IsEmpty ())
	{
		m_strClassName = ::AfxRegisterWndClass (
			CS_SAVEBITS,
			::LoadCursor(NULL, IDC_ARROW),
			(HBRUSH)(COLOR_BTNFACE + 1), NULL);
	}
}

CBCGPHeaderItemDropWnd::~CBCGPHeaderItemDropWnd()
{
}


BEGIN_MESSAGE_MAP(CBCGPHeaderItemDropWnd, CWnd)
	//{{AFX_MSG_MAP(CBCGPHeaderItemDropWnd)
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CBCGPHeaderItemDropWnd message handlers


BOOL CBCGPHeaderItemDropWnd::Create (int nColumnHeight) 
{
	const int nArrowSize = 9;

	int nWndHeight = nColumnHeight + nArrowSize + nArrowSize;

	CRect rectWindow (0, 0, nArrowSize, nWndHeight);

	if (!CreateEx (WS_EX_TOOLWINDOW,
		m_strClassName, NULL, WS_POPUP | WS_DISABLED,
		rectWindow, NULL, 0))
	{
		return FALSE;
	}

	#define ARROW_PTS 8

	POINT pts1 [ARROW_PTS] =
	{
		{	3,	0	},
		{	3,	4	},
		{	0,	4	},
		{	5,	9	},
		{	5,	8	},
		{	9,	4	},
		{	6,	4	},
		{	6,	0	},
	};

	CRgn rgn1;
	rgn1.CreatePolygonRgn (pts1, ARROW_PTS, ALTERNATE);

	POINT pts2 [ARROW_PTS] =
	{
		{	3,	nColumnHeight + nArrowSize + 9	},
		{	3,	nColumnHeight + nArrowSize + 5	},
		{	-1,	nColumnHeight + nArrowSize + 5	},
		{	5,	nColumnHeight + nArrowSize - 1	},
		{	5,	nColumnHeight + nArrowSize + 0	},
		{	9,	nColumnHeight + nArrowSize + 5	},
		{	6,	nColumnHeight + nArrowSize + 5	},
		{	6,	nColumnHeight + nArrowSize + 9	},
	};


	CRgn rgn2;
	rgn2.CreatePolygonRgn (pts2, ARROW_PTS, ALTERNATE);

	CRgn rgn;
	rgn.CreateRectRgnIndirect (rectWindow);

	rgn.CombineRgn (&rgn1, &rgn2, RGN_OR);

	SetWindowRgn (rgn, FALSE);

	rgn1.DeleteObject();
	rgn2.DeleteObject();
	rgn.DeleteObject();

	return TRUE;
}
//*******************************************************************************
void CBCGPHeaderItemDropWnd::PostNcDestroy() 
{
	CWnd::PostNcDestroy();
	delete this;
}
//*******************************************************************************
BOOL CBCGPHeaderItemDropWnd::OnEraseBkgnd(CDC* pDC) 
{
	CRect rectClient;
	GetClientRect (rectClient);

	pDC->FillSolidRect (rectClient, RGB (255, 0, 0));
	return TRUE;
}
//*******************************************************************************
void CBCGPHeaderItemDropWnd::Show (CPoint point)
{
	CRect rectClient;
	GetClientRect (rectClient);

	int x = point.x - rectClient.Width () / 2;
	int y = point.y - rectClient.Height () / 2;

	SetWindowPos
			(&wndTop,  x, y,
			-1, -1, SWP_NOSIZE | SWP_SHOWWINDOW | SWP_NOACTIVATE);
}
//*******************************************************************************
void CBCGPHeaderItemDropWnd::Hide ()
{
	ShowWindow (SW_HIDE);
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPGridColumnChooser

IMPLEMENT_DYNAMIC(CBCGPGridColumnChooser, CMiniFrameWnd)

CBCGPGridColumnChooser::CBCGPGridColumnChooser(CBCGPGridColumnsInfo& columns)
	: m_wndList (columns),
	m_pOwnerGrid (NULL),
	m_bIsEmpty (TRUE)
{
}

CBCGPGridColumnChooser::~CBCGPGridColumnChooser()
{
	if (m_pOwnerGrid != NULL)
	{
		ASSERT_VALID (m_pOwnerGrid);
		m_pOwnerGrid->m_pColumnChooser = NULL;
	}
}


BEGIN_MESSAGE_MAP(CBCGPGridColumnChooser, CMiniFrameWnd)
	//{{AFX_MSG_MAP(CBCGPGridColumnChooser)
	ON_WM_SIZE()
	ON_WM_CREATE()
	ON_WM_CLOSE()
	ON_WM_WINDOWPOSCHANGED()
	ON_WM_GETMINMAXINFO()
	ON_WM_KEYDOWN()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGPGridColumnChooser message handlers

void CBCGPGridColumnChooser::OnSize(UINT nType, int cx, int cy) 
{
	CMiniFrameWnd::OnSize(nType, cx, cy);
	
	if (m_wndList.GetSafeHwnd () != NULL)
	{
		m_wndList.SetWindowPos (NULL, -1, -1, cx, cy,
			SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
	}
}
//*******************************************************************************
BOOL CBCGPGridColumnChooser::Create(CBCGPGridCtrl* pOwnerGrid,
									const RECT& rect, CWnd* pParentWnd)
{
	ASSERT_VALID (pParentWnd);
	ASSERT_VALID (pOwnerGrid);

	m_pOwnerGrid = pOwnerGrid;

	m_strNoFields = m_pOwnerGrid->GetFieldChooserEmptyContentLabel();

	CString strClassName = ::AfxRegisterWndClass (
			CS_SAVEBITS,
			::LoadCursor(NULL, IDC_ARROW),
			(HBRUSH)(COLOR_BTNFACE + 1), NULL);

	CBCGPLocalResource locaRes;

	CString strTitle;
	strTitle.LoadString (IDS_BCGBARRES_GRID_FIELD_CHOOSER);

	if (!CMiniFrameWnd::CreateEx (
				pParentWnd->GetExStyle() & WS_EX_LAYOUTRTL,
				strClassName, strTitle,
				WS_POPUP | WS_CAPTION | WS_SYSMENU |
				MFS_MOVEFRAME | MFS_SYNCACTIVE | MFS_BLOCKSYSMENU,
				rect,
				pParentWnd))
	{
		return FALSE;
	}

	CMenu* pSysMenu = GetSystemMenu (FALSE);

	if (pSysMenu->GetSafeHmenu () != NULL)
	{
		pSysMenu->DeleteMenu (SC_SIZE, MF_BYCOMMAND);
		pSysMenu->DeleteMenu (SC_MINIMIZE, MF_BYCOMMAND);
		pSysMenu->DeleteMenu (SC_MAXIMIZE, MF_BYCOMMAND);
		pSysMenu->DeleteMenu (SC_RESTORE, MF_BYCOMMAND);

		CString strHide;

		if (strHide.LoadString (AFX_IDS_HIDE))
		{
			pSysMenu->DeleteMenu (SC_CLOSE, MF_BYCOMMAND);
			pSysMenu->AppendMenu (MF_STRING|MF_ENABLED, SC_CLOSE, strHide);
		}
	}

	return TRUE;
}
//*******************************************************************************
int CBCGPGridColumnChooser::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CMiniFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rect;
	GetClientRect (&rect);
	
	m_wndList.Create (	WS_CHILD | LBS_OWNERDRAWFIXED | 
						LBS_NOINTEGRALHEIGHT | WS_VSCROLL, 
						rect, this, 1);
	CRect rectWindow;
	GetWindowRect (rectWindow);

	CPoint ptPos = rectWindow.TopLeft ();
	CPoint ptPosCurr = ptPos;

	CSize size = rectWindow.Size ();

	CRect rectScreen;

	MONITORINFO mi;
	mi.cbSize = sizeof (MONITORINFO);

	if (GetMonitorInfo (MonitorFromPoint (ptPos, MONITOR_DEFAULTTONEAREST), &mi))
	{
		rectScreen = mi.rcWork;
	}
	else
	{
		::SystemParametersInfo (SPI_GETWORKAREA, 0, &rectScreen, 0);
	}

	if (ptPos.x < rectScreen.left)
	{
		ptPos.x = rectScreen.left;
	}
	else if (ptPos.x + size.cx > rectScreen.right)
	{
		ptPos.x = rectScreen.right - size.cx;
	}

	if (ptPos.y < rectScreen.top)
	{
		ptPos.y = rectScreen.top;
	}
	else if (ptPos.y + size.cy > rectScreen.bottom)
	{
		ptPos.y = rectScreen.bottom - size.cy;
	}

	if (ptPos != ptPosCurr)
	{
		SetWindowPos (&wndTop, ptPos.x, ptPos.y, -1, -1,
					SWP_NOACTIVATE | SWP_NOSIZE);
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPGridColumnListBox

CBCGPGridColumnListBox::CBCGPGridColumnListBox(CBCGPGridColumnsInfo& columns)
	: m_Columns (columns)
{
	m_bVisualManagerStyle = FALSE;
	m_clrText = (COLORREF)-1;
}

CBCGPGridColumnListBox::~CBCGPGridColumnListBox()
{
}


BEGIN_MESSAGE_MAP(CBCGPGridColumnListBox, CListBox)
	//{{AFX_MSG_MAP(CBCGPGridColumnListBox)
	ON_WM_LBUTTONDOWN()
	ON_WM_CREATE()
	ON_WM_KEYDOWN()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGPGridColumnListBox message handlers

int CBCGPGridColumnListBox::CompareItem(LPCOMPAREITEMSTRUCT /*lpCompareItemStruct*/) 
{
	return 0;
}
//*******************************************************************************
BOOL CBCGPGridColumnListBox::OnEraseBkgnd(CDC* pDC)
{
	if (m_bVisualManagerStyle)
	{
		CRect rect;
		GetClientRect(&rect);

		m_clrText = visualManager->OnFillGridGroupByBoxBackground (pDC, rect);
		return TRUE;
	}

	m_clrText = (COLORREF)-1;
	return (BOOL)Default();
}
//*******************************************************************************
void CBCGPGridColumnListBox::DrawItem(LPDRAWITEMSTRUCT lpDIS) 
{
	ASSERT (lpDIS != NULL);
	ASSERT_VALID (m_Columns.m_pWndList);

	CDC* pDC = CDC::FromHandle (lpDIS->hDC);
	ASSERT_VALID (pDC);

	CRect rectItem = lpDIS->rcItem;
	int nIndex = lpDIS->itemID;

	if (nIndex < 0)
	{
		return;
	}

	CFont* pFontOld = pDC->SelectObject (m_Columns.m_pWndList->GetFont ());

	int nColumn = (int) GetItemData (nIndex);
	ASSERT (nColumn >= 0);
	ASSERT (nColumn < m_Columns.m_pWndList->GetColumnCount ());
	CString strName = m_Columns.GetColumnName (nColumn);

	pDC->SetBkMode (TRANSPARENT);

	COLORREF clrText = m_clrText == (COLORREF)-1 ? globalData.clrWindowText : m_clrText;

	if (m_bVisualManagerStyle)
	{
		COLORREF clrItemText = visualManager->OnDrawGridColumnChooserItem(m_Columns.m_pWndList, pDC, rectItem, (lpDIS->itemState & ODS_SELECTED) != 0);
		if (clrItemText != (COLORREF)-1)
		{
			clrText = clrItemText;
		}
	}
	else
	{
		pDC->FillRect (rectItem, &globalData.brBtnFace);

		pDC->Draw3dRect (rectItem,
			globalData.clrBtnLight,
			globalData.clrBtnDkShadow);

		rectItem.DeflateRect (1, 1);

		pDC->Draw3dRect (rectItem,
			globalData.clrBtnHilite,
			globalData.clrBtnShadow);
	}

	rectItem.DeflateRect (1, 1);

	CRect rectText = rectItem;
	rectText.DeflateRect (5, 0);

	pDC->SetTextColor (clrText);
	pDC->DrawText (strName, rectText, DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX | DT_END_ELLIPSIS);

	if (!m_bVisualManagerStyle && (lpDIS->itemState & ODS_SELECTED))
	{
		pDC->InvertRect (rectItem);
	}

	pDC->SelectObject (pFontOld);

}
//*******************************************************************************
void CBCGPGridColumnListBox::MeasureItem(LPMEASUREITEMSTRUCT lpMIS) 
{
	ASSERT (lpMIS != NULL);
	ASSERT_VALID (m_Columns.m_pWndList);

	CClientDC dc (this);
	CFont* pFontOld = dc.SelectObject (m_Columns.m_pWndList->GetFont ());

	TEXTMETRIC tm;
	dc.GetTextMetrics (&tm);
	lpMIS->itemHeight = tm.tmHeight + 6;

	dc.SelectObject (pFontOld);
}
//*******************************************************************************
void CBCGPGridColumnListBox::OnLButtonDown(UINT nFlags, CPoint point) 
{
	ASSERT_VALID (m_Columns.m_pWndList);

	int nColumn = -1;
	CRect rectItem;

	for (int i = 0; i < GetCount (); i++)
	{
		GetItemRect (i, rectItem);

		if (rectItem.PtInRect (point))
		{
			nColumn = (int) GetItemData (i);
			break;
		}
	}

	CListBox::OnLButtonDown(nFlags, point);

	if (nColumn >= 0)
	{
		ASSERT (nColumn < m_Columns.m_pWndList->GetColumnCount ());
		
		MapWindowPoints (m_Columns.m_pWndList, rectItem);
		m_Columns.m_pWndList->m_ptStartDrag = CPoint (0, 0);
		m_Columns.m_pWndList->StartDragColumn (nColumn, rectItem, FALSE, TRUE);
	}
}
//*******************************************************************************
int CBCGPGridColumnListBox::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	ASSERT_VALID (m_Columns.m_pWndList);
	ASSERT_VALID (GetParent ());

	if (CListBox::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	CClientDC dc (this);
	CFont* pFontOld = dc.SelectObject (m_Columns.m_pWndList->GetFont ());

	TEXTMETRIC tm;
	dc.GetTextMetrics (&tm);
	int nRowHeight = tm.tmHeight + 6;

	int nHeight = min (10, m_Columns.GetColumnCount ()) * nRowHeight;

	CString strParentCaption;
	GetParent ()->GetWindowText (strParentCaption);

	int nMaxColumnWidth = dc.GetTextExtent (strParentCaption).cx + 20;

	for (int i = 0; i < m_Columns.GetColumnCount (); i++)
	{
		CString strColumn = m_Columns.GetColumnName (i);

		nMaxColumnWidth = max (nMaxColumnWidth,
			dc.GetTextExtent (strColumn).cx);

		if (!m_Columns.GetColumnVisible (i))
		{
			int nItem = AddString (strColumn);
			SetItemData (nItem, (DWORD_PTR) i);
		}
	}

	int cxScroll = ::GetSystemMetrics (SM_CXHSCROLL);

	GetParent ()->SetWindowPos (NULL, -1, -1, nMaxColumnWidth + cxScroll + 20,
		nHeight + 20, SWP_NOZORDER | SWP_NOMOVE);

	if (GetCount () == 0)
	{
		ShowWindow (SW_HIDE);
	}
	else if ((GetStyle () & WS_VISIBLE) == 0)
	{
		ShowWindow (SW_SHOWNOACTIVATE);
	}

	dc.SelectObject (pFontOld);
	return 0;
}
//*******************************************************************************
void CBCGPGridColumnListBox::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	ASSERT_VALID (m_Columns.m_pWndList);
	
	if (nChar == VK_ESCAPE &&
		m_Columns.m_pWndList->m_nDraggedColumn >= 0)
	{
		// Cancel dragging a column:
		CPoint pt(-1, -1);
		m_Columns.m_pWndList->StopDragColumn (pt, FALSE);

		return;
	}

	CListBox::OnKeyDown(nChar, nRepCnt, nFlags);
}
//*******************************************************************************
void CBCGPGridColumnChooser::OnClose() 
{
	ASSERT_VALID (m_pOwnerGrid);

	ShowWindow (SW_HIDE);
	
	m_pOwnerGrid->m_bColumnChooserVisible = FALSE;
	m_pOwnerGrid->OnHideColumnChooser ();
}
//*******************************************************************************
void CBCGPGridColumnChooser::OnWindowPosChanged(WINDOWPOS FAR* lpwndpos) 
{
	ASSERT_VALID (m_pOwnerGrid);

	CMiniFrameWnd::OnWindowPosChanged(lpwndpos);
	GetWindowRect (m_pOwnerGrid->m_rectColumnChooser);
}
//*******************************************************************************
void CBCGPGridColumnChooser::UpdateList ()
{
    ASSERT_VALID (this);

	if (GetSafeHwnd () == NULL)
    {
        return;
    }

	m_wndList.ResetContent ();

	for (int i = 0; i < m_wndList.m_Columns.GetColumnCount (); i++)
	{
		if (!m_wndList.m_Columns.GetColumnVisible (i))
		{
			int nItem = m_wndList.AddString (m_wndList.m_Columns.GetColumnName (i));
            m_wndList.SetItemData (nItem, (DWORD_PTR) i);
		}
	}
	
	if (m_wndList.GetCount () == 0)
	{
		m_bIsEmpty = TRUE;
		m_wndList.ShowWindow (SW_HIDE);
		RedrawWindow();
	}
	else if ((m_wndList.GetStyle () & WS_VISIBLE) == 0)
	{
		m_bIsEmpty = FALSE;
		m_wndList.ShowWindow (SW_SHOWNOACTIVATE);
	}
}
//*******************************************************************************
int CBCGPGridColumnChooser::GetColumnWidth () const
{
    ASSERT_VALID (this);

	if (GetSafeHwnd () == NULL)
    {
        return 0;
    }

	int cxScroll = ::GetSystemMetrics (SM_CXHSCROLL);

	CRect rectLB;
	m_wndList.GetWindowRect (&rectLB);

	return (int) rectLB.Width () - cxScroll;
}
//*******************************************************************************
void CBCGPGridColumnChooser::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	if (!m_bIsEmpty)
	{
		return;
	}

	CRect rect;
	GetClientRect (rect);

	if (m_wndList.m_bVisualManagerStyle)
	{
		visualManager->OnFillGridGroupByBoxBackground (&dc, rect);
#ifndef _BCGSUITE_
		dc.SetTextColor(visualManager->GetToolbarDisabledTextColor());
#else
		dc.SetTextColor (globalData.clrGrayedText);
#endif
	}
	else
	{
		dc.FillRect (rect, &globalData.brBtnFace);
		dc.SetTextColor (globalData.clrGrayedText);
	}
	
	dc.SetBkMode (TRANSPARENT);

	CFont* pOldFont = dc.SelectObject (&globalData.fontRegular);
	ASSERT (pOldFont != NULL);

	CString str = m_strNoFields;
	if (str.IsEmpty())
	{
		CBCGPLocalResource locaRes;
		str.LoadString (IDS_BCGBARRES_GRID_NO_FIELDS_TITLE);
	}

	rect.top = max (rect.top, rect.CenterPoint ().y - globalData.GetTextHeight () / 2);
	dc.DrawText (str, rect, DT_CENTER | DT_WORDBREAK);

	dc.SelectObject (pOldFont);
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPGridCache object - page cache implementation for virtual mode

CBCGPGridCache::CBCGPGridCache ()
{
	m_nCachePageSize = 150;
	m_nCachePageCount = 5;
}
//*******************************************************************************
CBCGPGridCache::~CBCGPGridCache ()
{
	CleanUpCache ();
}
//*******************************************************************************
// Get item by ID
CBCGPGridRow* CBCGPGridCache::GetCachedRow (int nId)
{
	for (POSITION pos = m_lstCache.GetHeadPosition (); pos != NULL; )
	{
		CBCGPGridCachePageInfo& cpi = m_lstCache.GetNext (pos);
		int nOffset = nId - cpi.nFirst;
		if (nOffset >= 0 && nOffset < cpi.nSize)
		{
			ASSERT (cpi.nSize == cpi.pArrCachePage->GetSize ());
			cpi.bReferenced = TRUE;
			return cpi.pArrCachePage->GetAt (nOffset);
		}
	}

	return NULL;
}
//*******************************************************************************
// Save item in cache
BOOL CBCGPGridCache::SetCachedRow (int nId, CBCGPGridRow* pItem)
{
	if (nId < 0)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	int nNewPageSize = m_nCachePageSize;
	ASSERT (nNewPageSize > 0);
	int nNewPageFirst = max (0, nId - nNewPageSize);

	for (POSITION pos = m_lstCache.GetHeadPosition (); pos != NULL; )
	{
		CBCGPGridCachePageInfo& cpi = m_lstCache.GetNext (pos);
		int nOffset = nId - cpi.nFirst;
		if (nOffset >= 0 && nOffset < cpi.nSize)
		{
			ASSERT (cpi.nSize == cpi.pArrCachePage->GetSize ());
			cpi.bReferenced = TRUE;
			cpi.pArrCachePage->SetAt (nOffset, pItem);
			return TRUE;
		}
		else if (nOffset + nNewPageSize >= 0 && nOffset < cpi.nSize)
		{
			nNewPageSize = cpi.nFirst - nId;
			ASSERT (nNewPageSize > 0);
			ASSERT (nNewPageSize <= m_nCachePageSize);
		}
		else if (nOffset >= cpi.nSize && nNewPageFirst < cpi.nFirst + cpi.nSize)
		{
			nNewPageFirst = min (cpi.nFirst + cpi.nSize, nId);
			ASSERT (nNewPageFirst >= 0);
			ASSERT (nNewPageFirst <= nId);
		}
	}

	int nShiftUpperBound = 0;
	if (nNewPageSize < m_nCachePageSize && nNewPageFirst < nId)
	{
		nShiftUpperBound = min (
			m_nCachePageSize - nNewPageSize,	// try maximum page size
			nId - nNewPageFirst);				// as possible
		ASSERT (nId - nShiftUpperBound >= 0);
		ASSERT (nShiftUpperBound + nNewPageSize <= m_nCachePageSize);
	}

	CachePageArray* pArray = new CachePageArray;
	pArray->SetSize (nShiftUpperBound + nNewPageSize);
	DoAddCache (pArray, nId - nShiftUpperBound);

	CBCGPGridCachePageInfo& cpiNew = m_lstCache.GetHead ();
	cpiNew.bReferenced = TRUE;
	pArray->SetAt (nShiftUpperBound, pItem);

	DoSwapCache ();

	return TRUE;
}
//*******************************************************************************
// New cache page request
BOOL CBCGPGridCache::AddCache (int nId)
{
	return SetCachedRow (nId, NULL);
}
//*******************************************************************************
// Clean up cache
void CBCGPGridCache::CleanUpCache ()
{
	while (!m_lstCache.IsEmpty ())
	{
		CBCGPGridCachePageInfo cpi = m_lstCache.RemoveTail ();
		DoFreeCachePage (cpi);
	}
}
//*******************************************************************************
// Add cache page
void CBCGPGridCache::DoAddCache (CachePageArray* pArray, int nIdFirst)
{
	ASSERT (pArray != NULL);

	CBCGPGridCachePageInfo cpi;
	cpi.nFirst = nIdFirst;
	cpi.nSize = (int) pArray->GetSize ();
	cpi.pArrCachePage = pArray;
	cpi.bReferenced = FALSE;

	m_lstCache.AddHead (cpi);
}
//*******************************************************************************
// Free cache page
void CBCGPGridCache::DoFreeCachePage (CBCGPGridCachePageInfo& cpi)
{
	for (int i = (int) cpi.pArrCachePage->GetUpperBound (); i >= 0; i--)
	{
		delete cpi.pArrCachePage->GetAt (i);
		cpi.pArrCachePage->SetAt (i, NULL);
	}
	cpi.pArrCachePage->RemoveAll ();
	delete cpi.pArrCachePage;
}
//*******************************************************************************
// Free unreferenced cache page on exceed capacity (over cache)
void CBCGPGridCache::DoSwapCache ()
{
	ASSERT (m_nCachePageCount > 0);
	while (m_lstCache.GetCount () > m_nCachePageCount)
	{
		CBCGPGridCachePageInfo cpi = m_lstCache.RemoveTail ();
		if (cpi.bReferenced)
		{
			cpi.bReferenced = FALSE;
			m_lstCache.AddHead (cpi);
		}
		else
		{
			DoFreeCachePage (cpi);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// Grid Drop target

BOOL CBCGPGridCtrlDropTarget::Register (CBCGPGridCtrl* pOwner)
{
	m_pOwner = pOwner;
	return COleDropTarget::Register (pOwner);
}
//****************************************************************************************
DROPEFFECT CBCGPGridCtrlDropTarget::OnDragEnter(CWnd* /*pWnd*/, COleDataObject* pDataObject,
												DWORD dwKeyState, CPoint point) 
{
	ASSERT (m_pOwner != NULL);
	return m_pOwner->OnDragEnter(pDataObject, dwKeyState, point);
}
//****************************************************************************************
void CBCGPGridCtrlDropTarget::OnDragLeave(CWnd* /*pWnd*/) 
{
	ASSERT (m_pOwner != NULL);
	m_pOwner->OnDragLeave ();
}
//****************************************************************************************
DROPEFFECT CBCGPGridCtrlDropTarget::OnDragOver(CWnd* /*pWnd*/, COleDataObject* pDataObject,
											   DWORD dwKeyState, CPoint point) 
{
	ASSERT (m_pOwner != NULL);
	return m_pOwner->OnDragOver(pDataObject, dwKeyState, point);
}
//****************************************************************************************
DROPEFFECT CBCGPGridCtrlDropTarget::OnDropEx(CWnd* /*pWnd*/, 
											 COleDataObject* pDataObject, 
											 DROPEFFECT dropEffect, 
											 DROPEFFECT /*dropList*/, CPoint point) 
{
	ASSERT (m_pOwner != NULL);
	return m_pOwner->OnDrop(pDataObject, dropEffect, point) ? dropEffect : DROPEFFECT_NONE;
}
//****************************************************************************************
DROPEFFECT CBCGPGridCtrlDropTarget::OnDragScroll(CWnd* /*pWnd*/,
												 DWORD dwKeyState, CPoint point)
{
	ASSERT (m_pOwner != NULL);
	return m_pOwner->OnDragScroll(dwKeyState, point);
}

#endif // BCGP_EXCLUDE_GRID_CTRL
