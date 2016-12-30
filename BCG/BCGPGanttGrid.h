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
// BCGPGanttGrid.h: interface for the CBCGPGanttGrid class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPGANTTGRID_H__085103BE_34FD_47CC_9F72_363D71F1F974__INCLUDED_)
#define AFX_BCGPGANTTGRID_H__085103BE_34FD_47CC_9F72_363D71F1F974__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"
#include "BCGPGridCtrl.h"

#if !defined (BCGP_EXCLUDE_GRID_CTRL) && !defined (BCGP_EXCLUDE_GANTT)

class CBCGPGanttControl;

class BCGCBPRODLLEXPORT CBCGPGanttGrid : public CBCGPGridCtrl  
{
	friend class CBCGPGanttControl;
	friend class CBCGPGanttGridRow;
public:
	CBCGPGanttGrid();
	virtual ~CBCGPGanttGrid();

public:
	void SetVerticalSizes (UINT nHeaderHeight, UINT nRowHeight);
	void GetVerticalSizes (UINT* pHeaderHeight, UINT* pRowHeight) const;

protected:
	virtual CBCGPGanttControl* GetGanttControlNotify () const;
	virtual void  OnItemChanged (CBCGPGridItem* pItem, int nRow, int nColumn);

	virtual void  SetRowHeight ();

	virtual CRect OnGetHeaderRect (CDC* pDC, const CRect& rectDraw);
	virtual void  OnUpdateVScrollPos (int nVOffset, int nPrevVOffset);

	virtual void  OnExpandGroup (CBCGPGridRow* pRow, BOOL bExpand);

	int GetVScrollPos () const
	{
		return m_nVertScrollOffset;
	}

	void SetVScrollPos (int nNewVertOffset)
	{
		int nOld = m_nVertScrollOffset;
		CBCGPGridCtrl::SetScrollPos (SB_VERT, nNewVertOffset, FALSE);
		CBCGPGridCtrl::OnVScroll (SB_THUMBPOSITION, nNewVertOffset, NULL);
		OnUpdateVScrollPos (nNewVertOffset, nOld);
	}

	// Overrides
public:
	virtual CBCGPGridRow* CreateRow ();
	virtual CBCGPGridRow* CreateRow (CString strName);

	virtual CBCGPGridRow* CreateRow (int nColumns)
	{
		return CBCGPGridCtrl::CreateRow (nColumns);
	}

	virtual void InsertRowAfter (CBCGPGridRow* pParentRow, int nSubPos, CBCGPGridRow* pSubItem);

	virtual void AdjustLayout ();


	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPGanttGrid)
	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPaint();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

	DECLARE_DYNCREATE (CBCGPGanttGrid);

protected:
	UINT    m_nHeaderHeight; // desired header height
	BOOL    m_bNeedAdjustLayout; // allows to defer adjusting layout to next WM_PAINT
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPGridDateTimeItem object

class BCGCBPRODLLEXPORT CBCGPGridPercentItem : public CBCGPGridItem
{
	DECLARE_DYNCREATE(CBCGPGridPercentItem)

	// Construction
protected:
	CBCGPGridPercentItem ();

public:
	CBCGPGridPercentItem (double value, DWORD_PTR dwData = 0);

	virtual ~CBCGPGridPercentItem();

	void    SetMaxValue (double dValue);
	double  GetMaxValue () const
	{
		return max (0.0, m_dMaxValue);
	}

	void    SetPrecision (int nDigitsAfterFloatingPoint);
	int     GetPrecision () const
	{
		return max (0, min (8, m_nPrecision));
	}

	// Overrides
public:
	virtual BOOL OnUpdateValue ();
	virtual CString FormatItem ();

protected:
	double m_dMaxValue; // Maximal available value. Default is 1.0 (100%).
	int    m_nPrecision; // From 0 - "x %" (integer part only) to 8 "x.xxxxxxxx %"
};

#endif // !defined (BCGP_EXCLUDE_GRID_CTRL) && !defined (BCGP_EXCLUDE_GANTT)
#endif // !defined(AFX_BCGPGANTTGRID_H__085103BE_34FD_47CC_9F72_363D71F1F974__INCLUDED_)
