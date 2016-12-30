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
// BCGPGanttView.h: interface for the CBCGPGanttView class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPGANTTVIEW_H__44D5A182_40F7_45AD_8B2F_68703C10D182__INCLUDED_)
#define AFX_BCGPGANTTVIEW_H__44D5A182_40F7_45AD_8B2F_68703C10D182__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BCGPGanttView.h : header file
//

#include "BCGCBPro.h"

#if !defined (BCGP_EXCLUDE_GRID_CTRL) && !defined (BCGP_EXCLUDE_GANTT)

#include "BCGPGanttControl.h"

/////////////////////////////////////////////////////////////////////////////
// CBCGPGanttView view

class BCGCBPRODLLEXPORT CBCGPGanttView : public CView
{
	DECLARE_DYNCREATE(CBCGPGanttView)

protected:
	CBCGPGanttView();           // protected constructor used by dynamic creation

// Attributes
public:
	CBCGPGanttControl& GetControl ()
	{
		return m_wndGanttControl;
	}
	const CBCGPGanttControl& GetControl () const
	{
		return m_wndGanttControl;
	}

    CBCGPGanttGrid* GetGrid ()
	{
		return GetControl ().GetGrid ();
	}
    const CBCGPGanttGrid* GetGrid () const
	{
		return GetControl ().GetGrid ();
	}

    CBCGPGanttChart* GetChart ()
	{
		return GetControl ().GetChart ();
	}
    const CBCGPGanttChart* GetChart () const
	{
		return GetControl ().GetChart ();
	}

    CBCGPSplitterWnd* GetSplitter ()
	{
		return GetControl ().GetSplitter ();
	}
    const CBCGPSplitterWnd* GetSplitter () const
	{
		return GetControl ().GetSplitter ();
	}

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPGanttView)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnInitialUpdate(); // called first time after construct
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnPrint(CDC* pDC, CPrintInfo* pInfo);
	//}}AFX_VIRTUAL

// Implementation
protected:
    // User-defined grid data 
	virtual ~CBCGPGanttView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

private:
	virtual CBCGPGanttChart* OnCreateChart();
	virtual CBCGPGanttGrid* OnCreateGrid();

protected:
    virtual BOOL OnChartClick (UINT /*nFlags*/, CPoint /*point*/) {	return FALSE;	}
    virtual BOOL OnChartClickItem (UINT /*nFlags*/, CBCGPGanttItem* /*pItem*/) {	return FALSE;	}
    virtual BOOL OnChartClickHeader (UINT /*nFlags*/, CPoint /*point*/) {	return FALSE;	}
    virtual BOOL OnChartDoubleClick (UINT /*nFlags*/, CPoint /*point*/) {	return FALSE;	}
    virtual BOOL OnChartDoubleClickItem (UINT /*nFlags*/, CBCGPGanttItem* /*pItem*/) {	return FALSE;	}
    virtual BOOL OnChartDoubleClickHeader (UINT /*nFlags*/, CPoint /*point*/) {	return FALSE;	}
    virtual BOOL OnChartItemMoving (BCGP_GANTT_ITEM_DRAGDROP* /*pDragDrop*/) {	return TRUE;	}
    virtual void OnChartItemChanged (CBCGPGanttItem* /*pItem*/, DWORD /*dwAction*/) { }
    virtual BOOL OnChartScaleChanging (BOOL /*bIncrease*/) {	return FALSE;	}
    virtual void OnChartScaleChanged () {}

    virtual void OnStorageChanged (const BCGP_GANTT_STORAGE_UPDATE_INFO& /*updateInfo*/) { }
    virtual void OnStorageConnectionAdded (const CBCGPGanttConnection& /*connection*/)   { }
    virtual void OnStorageConnectionRemoved (const CBCGPGanttConnection& /*connection*/) { }

    virtual BOOL OnGridRowWriteData (const BCGP_GANTT_CONTROL_ROW& /*rowInfo*/) { return FALSE; }
    virtual BOOL OnGridRowReadData (const BCGP_GANTT_CONTROL_ROW& /*rowInfo*/) { return FALSE; }

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPGanttView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	//}}AFX_MSG
	afx_msg LRESULT OnNotifyCreateChart(WPARAM, LPARAM);
	afx_msg LRESULT OnNotifyCreateGrid(WPARAM, LPARAM);
	afx_msg LRESULT OnNotifyChartClick(WPARAM, LPARAM);
	afx_msg LRESULT OnNotifyChartClickItem(WPARAM, LPARAM);
	afx_msg LRESULT OnNotifyChartClickHeader(WPARAM, LPARAM);
    afx_msg LRESULT OnNotifyChartDblClick(WPARAM, LPARAM);
    afx_msg LRESULT OnNotifyChartDblClickItem(WPARAM, LPARAM);
    afx_msg LRESULT OnNotifyChartDblClickHeader(WPARAM, LPARAM);
	afx_msg LRESULT OnNotifyChartItemMoving(WPARAM, LPARAM);
	afx_msg LRESULT OnNotifyChartItemsChanged(WPARAM, LPARAM);
	afx_msg LRESULT OnNotifyChartScaleChanging(WPARAM, LPARAM);
	afx_msg LRESULT OnNotifyChartScaleChanged(WPARAM, LPARAM);
    afx_msg LRESULT OnNotifyStorageChanged (WPARAM, LPARAM);
    afx_msg LRESULT OnNotifyStorageConnectionAdded (WPARAM, LPARAM);
    afx_msg LRESULT OnNotifyStorageConnectionRemoved (WPARAM, LPARAM);
    afx_msg LRESULT OnNotifyReadDataFromGrid (WPARAM, LPARAM);
    afx_msg LRESULT OnNotifyWriteDataToGrid (WPARAM, LPARAM);
	afx_msg LRESULT OnChangeVisualManager (WPARAM wp, LPARAM lp);
	DECLARE_MESSAGE_MAP()

protected:
    CBCGPGanttControl       m_wndGanttControl;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined (BCGP_EXCLUDE_GRID_CTRL) && !defined (BCGP_EXCLUDE_GANTT)
#endif // !defined(AFX_BCGPGANTTVIEW_H__44D5A182_40F7_45AD_8B2F_68703C10D182__INCLUDED_)
