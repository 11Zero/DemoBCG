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
// BCGPGanttGrid.cpp: implementation of the CBCGPGanttGrid class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BCGPGanttGrid.h"
#include "BCGPGanttControl.h"
#include <float.h>

#if !defined (BCGP_EXCLUDE_GRID_CTRL) && !defined (BCGP_EXCLUDE_GANTT)

class CBCGPGanttGridRow : public CBCGPGridRow
{
	DECLARE_DYNAMIC(CBCGPGanttGridRow)
public:
	// Simple item
	CBCGPGanttGridRow (int nColumns = 0)
		: CBCGPGridRow (nColumns)
	{
	}

	// Group constructor
	CBCGPGanttGridRow (const CString& strGroupName)
		: CBCGPGridRow (strGroupName)
	{
	}

	void SwapSubItems (CBCGPGridRow* pRow1, CBCGPGridRow* pRow2)
	{
		ASSERT_VALID (pRow1);
		ASSERT_VALID (pRow2);

		POSITION pos1 = m_lstSubItems.Find (pRow1);
		POSITION pos2 = m_lstSubItems.Find (pRow2);

		if (pos1 == NULL || pos2 == NULL || pos1 == pos2)
		{
			return;
		}

		int id1 = pRow1->GetRowId ();
		int id2 = pRow2->GetRowId ();
		CBCGPGanttGridRow* pGanttRow1 = (CBCGPGanttGridRow*)pRow1;
		CBCGPGanttGridRow* pGanttRow2 = (CBCGPGanttGridRow*)pRow2;
		pGanttRow1->m_nIdRow = id2;
		pGanttRow2->m_nIdRow = id1;

		m_lstSubItems.SetAt (pos1, pRow2);
		m_lstSubItems.SetAt (pos2, pRow1);

		pRow1->Redraw ();
		pRow2->Redraw ();
	}

protected:
	virtual void OnExpand (BOOL bExpand)
	{
		CBCGPGridCtrl* pGridCtrl = GetOwnerList (); // parent grid

		CBCGPGanttGrid* pGanttGrid = DYNAMIC_DOWNCAST (CBCGPGanttGrid, pGridCtrl);
		if (pGanttGrid != NULL)
		{
			pGanttGrid->OnExpandGroup (DYNAMIC_DOWNCAST(CBCGPGridRow, this), bExpand);
		}
	}

public:
	virtual ~CBCGPGanttGridRow ()
	{
	}
};

IMPLEMENT_DYNAMIC(CBCGPGanttGridRow, CBCGPGridRow)


//////////////////////////////////////////////////////////////////////
//       Implementation of the CBCGPGanttGrid class
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE (CBCGPGanttGrid, CBCGPGridCtrl);

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGPGanttGrid::CBCGPGanttGrid()
	: m_nHeaderHeight (0)
	, m_bNeedAdjustLayout (TRUE)
{

}

CBCGPGanttGrid::~CBCGPGanttGrid()
{

}

CBCGPGridRow* CBCGPGanttGrid::CreateRow ()
{
	return new CBCGPGanttGridRow;
}

CBCGPGridRow* CBCGPGanttGrid::CreateRow (CString strName)
{
	return new CBCGPGanttGridRow (strName);
}

void CBCGPGanttGrid::InsertRowAfter (CBCGPGridRow* pParentRow, int nSubPos, CBCGPGridRow* pSubItem)
{
	if (pParentRow == NULL) // top-level item
	{
		CBCGPGridCtrl::InsertRowAfter (nSubPos, pSubItem);
	}
	else
	{
		CBCGPGanttGridRow* pGridParentRow = DYNAMIC_DOWNCAST (CBCGPGanttGridRow, pParentRow);
		pGridParentRow->AllowSubItems (TRUE);
		pGridParentRow->AddSubItem (pSubItem);

		if (nSubPos < pGridParentRow->GetSubItemsCount(FALSE))
		{
			pGridParentRow->SwapSubItems (pGridParentRow->GetSubItem (nSubPos), pSubItem);
		}
	}
}

void CBCGPGanttGrid::OnItemChanged (CBCGPGridItem* pGridItem, int nRow, int nColumn)
{
	CBCGPGridCtrl::OnItemChanged (pGridItem, nRow, nColumn);

	CBCGPGanttControl* pNotify = GetGanttControlNotify ();

	if (pNotify != NULL)
	{
		ASSERT_VALID (pNotify);

		pNotify->OnGridItemChanged (pGridItem, nRow, nColumn);
	}
}

void CBCGPGanttGrid::SetVerticalSizes (UINT nHeaderHeight, UINT nRowHeight)
{
	m_nHeaderHeight = nHeaderHeight;
	m_nBaseHeight = nRowHeight; 
	m_nRowHeight = nRowHeight;
	m_nLargeRowHeight = nRowHeight;
	m_nButtonWidth = nRowHeight;
	m_bAllowRowExtraHeight = FALSE;

	AdjustLayout ();
}

void CBCGPGanttGrid::GetVerticalSizes (UINT* pHeaderHeight, UINT* pRowHeight) const
{
	if (pHeaderHeight != NULL)
	{
		*pHeaderHeight = GetRowHeight () + abs(GetGridHeaderRect ().Height ());
	}
	if (pRowHeight != NULL)
	{
		*pRowHeight = GetRowHeight ();
	}
}

void CBCGPGanttGrid::OnUpdateVScrollPos (int nVOffset, int)
{
	CBCGPGanttControl* pNotify = GetGanttControlNotify ();
	if (pNotify != NULL)
	{
		ASSERT_VALID (pNotify);

		pNotify->DoVerticalScroll (this, nVOffset);
	}
}

void CBCGPGanttGrid::SetRowHeight ()
{
	if (m_bIsPrinting || m_nRowHeight <= 0)
	{
		CBCGPGridCtrl::SetRowHeight ();
	}
}

CRect CBCGPGanttGrid::OnGetHeaderRect (CDC* pDC, const CRect& rectDraw)
{
	CRect rect = CBCGPGridCtrl::OnGetHeaderRect (pDC, rectDraw);
	if (m_nHeaderHeight > 0 && !m_bIsPrinting)
	{
		rect.bottom = rect.top + m_nHeaderHeight;
	}
	return rect;
}

void CBCGPGanttGrid::AdjustLayout ()
{
	m_bNeedAdjustLayout = TRUE;
	Invalidate ();
	return;
}

void CBCGPGanttGrid::OnPaint() 
{
	if (m_bNeedAdjustLayout)
	{
		// Adjusting layout

		UINT nOldHeader, nOldRows;
		GetVerticalSizes (&nOldHeader, &nOldRows);

		CBCGPGridCtrl::AdjustLayout ();

		UINT nHeader, nRows;
		GetVerticalSizes (&nHeader, &nRows);

		if (nHeader != nOldHeader || nRows != nOldRows)
		{
			CBCGPGanttControl* pGanttNotify = GetGanttControlNotify ();
			if (pGanttNotify != NULL)
			{
				ASSERT_VALID (pGanttNotify);
				pGanttNotify->DoVerticalResize (this);
			}
		}

		m_bNeedAdjustLayout = FALSE;
	}

	CBCGPGridCtrl::OnPaint();
}

void CBCGPGanttGrid::OnExpandGroup (CBCGPGridRow* pRow, BOOL bExpand)
{
	CBCGPGanttControl* pGanttCtrl = GetGanttControlNotify ();
	if (pGanttCtrl != NULL)
	{
		pGanttCtrl->DoExpandGroup (this, pRow, bExpand);
	}
}

CBCGPGanttControl* CBCGPGanttGrid::GetGanttControlNotify () const
{
	CWnd* pParent = GetParent ();

	if (pParent == NULL)
	{
		return NULL;
	}

	CWnd* pParent2 = pParent->GetParent ();

	if (pParent2 == NULL)
	{
		pParent2 = pParent;
	}

	CBCGPGanttControl* pGanttCtrl =  DYNAMIC_DOWNCAST (CBCGPGanttControl, pParent2);

	if (pGanttCtrl == NULL)
	{
		return NULL;
	}

	return static_cast<CBCGPGanttControl*>(pGanttCtrl);
}

BEGIN_MESSAGE_MAP(CBCGPGanttGrid, CBCGPGridCtrl)
	//{{AFX_MSG_MAP(CBCGPGanttGrid)
	ON_WM_CREATE()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

int  CBCGPGanttGrid::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	int iResult = CBCGPGridCtrl::OnCreate (lpCreateStruct);
	if (iResult != 0)
	{
		return iResult;
	}

	ShowVertScrollBar (FALSE);
	ShowHorzScrollBar (TRUE);
	m_bScrollHorzShowAlways = TRUE;

	EnableHeader (TRUE, BCGP_GRID_HEADER_HIDE_ITEMS);

	return 0;
}

//////////////////////////////////////////////////////////////////////////
//       CBCGPGridPercentItem  implementation
//////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE (CBCGPGridPercentItem, CBCGPGridItem)

CBCGPGridPercentItem::CBCGPGridPercentItem ()
	: m_dMaxValue (1.0)
	, m_nPrecision (0)
{
}

CBCGPGridPercentItem::CBCGPGridPercentItem (double value, DWORD_PTR dwData)
	: CBCGPGridItem (value, dwData)
	, m_dMaxValue (1.0)
	, m_nPrecision (0)
{
}

CBCGPGridPercentItem::~CBCGPGridPercentItem()
{
}

void CBCGPGridPercentItem::SetMaxValue (double dValue)
{
	m_dMaxValue = max (0.0, dValue);
}

void CBCGPGridPercentItem::SetPrecision (int nDigits)
{
	m_nPrecision = max (0, min (8, nDigits));
}


CString CBCGPGridPercentItem::FormatItem ()
{
	TCHAR pcszFormat[] = _T("%.1f %%");
	ASSERT (m_nPrecision >= 0);
	ASSERT (m_nPrecision < 10);
	pcszFormat[2] = TCHAR ('0' + GetPrecision ());

	double dValue = 100.0 * (double)m_varValue;

	CString sText;
	sText.Format (pcszFormat, dValue);
	return sText;
}

static BOOL ConvertToDouble (LPCTSTR pszText, double& dValue) // user-friendly string to number conversion
{
	BOOL minus = FALSE;
	BOOL sign_awaited = TRUE;

	int fpt = -1;       // A number of digits after floating point or (-1).
	int exp = 0;        // Decimal exponent
	int dig = 0;        // A number of decimal digits
	double x = 0;
	TCHAR c;             // A current character

	while (*pszText != 0)
	{
		c = *pszText++;

		if (c == ' ')
		{
			continue;
		}

		if (sign_awaited && (c == '+' || c == '-'))
		{
			minus = (c == '-');
			sign_awaited = FALSE;
			continue;
		}

		if (fpt < 0 && (c == '.' || c == ','))
		{
			fpt = 0;
			sign_awaited = FALSE;
			continue;
		}

		if (c < '0' || c > '9')
		{
			return FALSE;
		}

		sign_awaited = FALSE;
		if (x != 0 || dig == 0)
		{
			++dig;
		}

		if (dig >= DBL_DIG)
		{
			if (fpt < 0) ++exp;
		}
		else
		{
			x = x * 10 + (c - '0');
			if (fpt >= 0) ++fpt;
		}
	}

	if (dig == 0)
	{
		return FALSE;
	}

	if (fpt >= 0) exp = exp - fpt;

	while (exp > 0)
	{
		if (x > DBL_MAX / 10)
		{
			return FALSE;
		}

		x *= 10;
		--exp;
	}

	while (exp < 0)
	{
		x /= 10;
		++exp;
	}

	dValue = minus ? -x : x;
	return TRUE;
}

BOOL CBCGPGridPercentItem::OnUpdateValue ()
{
	ASSERT_VALID (m_pWndInPlace);
	ASSERT (::IsWindow (m_pWndInPlace->GetSafeHwnd ()));
	ASSERT_VALID (m_pGridRow);

	ASSERT_VALID (m_pWndInPlace);

	double dOldValue = (double)GetValue ();

	CString sTextValue;
	m_pWndInPlace->GetWindowText (sTextValue);
	sTextValue.TrimRight (_T(" %"));

	double dValue;
	if (!ConvertToDouble (sTextValue, dValue))
	{
		return FALSE; // continue editing on syntax error
	}

	dValue = min (GetMaxValue (), dValue / 100.0);

	if (dValue < 0.0)
	{
		dValue = dOldValue;
	}

	if (CanUpdateData ())
	{
		m_varValue = _variant_t (dValue);
	}

	if (dOldValue != dValue)
	{
		CBCGPGridItemID id = GetGridItemID ();
		m_pGridRow->OnItemChanged (this, id.m_nRow, id.m_nColumn);
		m_bIsChanged = TRUE;
	}

	return TRUE;
}

#endif // !defined (BCGP_EXCLUDE_GRID_CTRL) && !defined (BCGP_EXCLUDE_GANTT)
