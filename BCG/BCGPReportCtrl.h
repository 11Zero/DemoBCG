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
// BCGPReportCtrl.h: interface for the CBCGPReportCtrl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPREPORTCTRL_H__4B66884F_8973_472A_BCD1_4A3C27B62366__INCLUDED_)
#define AFX_BCGPREPORTCTRL_H__4B66884F_8973_472A_BCD1_4A3C27B62366__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"

#ifndef BCGP_EXCLUDE_GRID_CTRL

#include "BCGPGridCtrl.h"

/////////////////////////////////////////////////////////////////////////////
// CBCGPReportRow object

class BCGCBPRODLLEXPORT CBCGPReportRow : public CBCGPGridRow
{
	DECLARE_DYNAMIC(CBCGPReportRow)

	friend class CBCGPReportCtrl;

// Construction
protected:
	CBCGPReportRow ();
public:
	// Group constructor
	CBCGPReportRow (const CString& strGroupName, DWORD dwData = 0);

	virtual ~CBCGPReportRow();
	
// Operations:
public:
	const CString& GetDescription () const
	{
		return m_strDescr;
	}

	void SetDescription (const CString& strDescr)
	{
		m_strDescr = strDescr;
		m_nPreviewHeight = -1;
	}

// Overrides
public:
	virtual void OnDrawName (CDC* pDC, CRect rect);
	virtual void OnDrawPreview (CDC* pDC, CRect rect);
	virtual void OnDrawExpandBox (CDC* pDC, CRect rectExpand);

	virtual void OnPrintName (CDC* pDC, CRect rect);
	
	virtual int GetHierarchyLevel () const;

	virtual CString GetName ();

	virtual void SetName (const CString& strName)
	{
		m_strName = strName;
	}

	virtual CRect GetNameTooltipRect ();

protected:
	virtual BOOL HasValueField () const
	{
		return (m_arrRowItems.GetSize () > 0);
	}
	virtual void GetPreviewText (CString& str) const;
	virtual void OnMeasureGridRowRect (CRect& rect);
	
	int CalcPreview (const CRect& rec);
	
// Attributes
protected:
	CString			m_strName;
	CString			m_strDescr;			// Row description
	int				m_nPreviewHeight;
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPReportCtrl window

class BCGCBPRODLLEXPORT CBCGPReportCtrl : public CBCGPGridCtrl
{
	DECLARE_DYNAMIC(CBCGPReportCtrl)

	friend class CBCGPReportRow;

public:
	CBCGPReportCtrl();
	virtual ~CBCGPReportCtrl();

// Operations
	void EnablePreviewRow (BOOL bPreviewRow = TRUE, BOOL bRedraw = TRUE);
	virtual BOOL IsPreviewRowEnabled () const;

	void SetPreviewRowAutoLeftMargin (BOOL bAuto = TRUE);
	void RecalcMargins ();

	void SetPreviewRowCustomLeftMargin (int nMargin)
	{
		m_nPreviewRowLeftMargin = nMargin;
	}

	void SetPreviewRowCustomRightMargin (int nMargin)
	{
		m_nPreviewRowRightMargin = nMargin;
	}

// Overrides
public:
	virtual CBCGPGridRow* CreateRow ()
	{
		return new CBCGPReportRow;
	}

	virtual CBCGPGridRow* CreateRow (int nColumns)
	{
		return CBCGPGridCtrl::CreateRow (nColumns);
	}

	virtual CBCGPGridRow* CreateRow (CString strName)
	{
		return new CBCGPReportRow (strName);
	}

	virtual CRect OnGetPreviewRowMargins (CBCGPGridRow* pRow) const;

protected:
	virtual void SetRowHeight ();
	virtual int GetExtraHierarchyOffset () const { return 0;}
	virtual int GetHierarchyOffset () const;

	virtual BOOL AllowInPlaceEdit () const
	{
		return FALSE; // disable inplace editing
	}

	virtual BOOL IsSelectionBorderEnabled () const
	{
		return FALSE;
	}
	
	virtual BOOL OnTrackHeader ()
	{
		return FALSE; // do not resize columns inside client area by mouse
	}

	virtual void OnResizeColumns ();
	void SetRecalcPreview ();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPReportCtrl)
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
	// Preview feature to show description text in row:
	BOOL	m_bPreviewRow;	// If TRUE, extend the height of the row for description text
	int		m_nPreviewRowMaxLines;	// The maximum number of text lines for description
	int		m_nPreviewRowHeight;		// The maximum height for preview row
	int		m_nPrintPreviewRowHeight;	// 
	BOOL	m_bPreviewRowAutoLeftMargin;	// If true, calculates m_nPreviewRowLeftMargin automatically
	int		m_nPreviewRowLeftMargin;  // Custom Left margin for description text, if < 0 use default value
	int		m_nPreviewRowRightMargin; // Custom Right margin for description text, if < 0 use default value

	CDC*	m_pPreviewDC;	// used to calculate preview height
};

#endif // BCGP_EXCLUDE_GRID_CTRL

#endif // !defined(AFX_BCGPREPORTCTRL_H__4B66884F_8973_472A_BCD1_4A3C27B62366__INCLUDED_)
