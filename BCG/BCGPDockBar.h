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

#if !defined(AFX_BCGPDOCKBAR_H__E6383A87_73D1_4CB4_8C45_EF46C6537FCA__INCLUDED_)
#define AFX_BCGPDOCKBAR_H__E6383A87_73D1_4CB4_8C45_EF46C6537FCA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BCGPDockBar.h : header file
//

#include "BCGCBPro.h"
#include "BCGPControlBar.h"

class CBCGPDockBarRow;
class CBCGPDockManager;

typedef enum BCG_DS_TYPE
{
	DS_TYPE_UNDEFINED = 0x0,
	DS_STATIC = 0x1,
	DS_RESIZABLE = 0x2
};

#pragma warning( disable : 4100 34 )

class BCGCBPRODLLEXPORT CBCGPDockBar : public CBCGPBaseControlBar
{
	friend class CBCGPDockManager;

	DECLARE_DYNCREATE(CBCGPDockBar)
// Construction
public:
	CBCGPDockBar();

// Attributes
public:

	bool IsLastRow (CBCGPDockBarRow* pRow) const; 
	BOOL IsRectWithinDockBar (CRect rect, CPoint& ptDelta);
	virtual BOOL IsResizable () const {return FALSE;}
	virtual BOOL CanAcceptBar (const CBCGPBaseControlBar* pBar) const;
	virtual BOOL IsDragMode () const;

	virtual BOOL DoesAllowDynInsertBefore () const {return FALSE;}

	const CObList& GetControlBarList () const {return m_lstControlBars;}

	const CObList& GetDockBarRowsList () const {return m_lstDockBarRows;}
	
// Operations
public:
	virtual CBCGPDockBarRow* CreateRow (CBCGPDockBar* pParentDockBar, int nOffset, int nRowHeight);

	virtual void DockControlBar			(CBCGPControlBar* pWnd, BCGP_DOCK_METHOD dockMethod, LPCRECT lpRect = NULL);
	virtual BOOL DockControlBarLeftOf	(CBCGPControlBar* pBarToDock, CBCGPControlBar* pTargetBar);

	virtual void RemoveControlBar	(CBCGPControlBar* pWnd, BCGP_DOCK_METHOD dockMethod);

	virtual BOOL MoveControlBar		(CBCGPControlBar* pWnd, UINT nFlags, CPoint ptOffset);

	virtual void OnInsertRow (POSITION pos);
	virtual void OnRemoveRow (POSITION pos, BOOL bByShow = FALSE);	
	virtual int  OnResizeRow (CBCGPDockBarRow* pRowToResize, int nOffset);
	virtual void OnShowRow (POSITION pos, BOOL bShow);

	virtual void OnSizeParent (CRect& rectAvailable, UINT nSide, BOOL bExpand, int nOffset) {}
	virtual void RepositionBars	(CRect& rectNewClientArea);
	virtual CSize CalcFixedLayout (BOOL bStretch, BOOL bHorz);

	CBCGPDockBarRow* AddRow (POSITION pos, int nHeight);
	void RemoveRow (CBCGPDockBarRow* pRow);
	void ShowRow (CBCGPDockBarRow* pRow, BOOL bShow, BOOL bAdjustLayout);

	void ResizeDockBar (int nNewWidth, int nNewHeight);
	int ResizeRow  (CBCGPDockBarRow* pRow, int nNewSize, BOOL bAdjustLayout = TRUE);

	void SwapRows (CBCGPDockBarRow* pFirstRow, CBCGPDockBarRow* pSecondRow);

	virtual void FixupVirtualRects ();

	virtual BOOL ShowControlBar (CBCGPBaseControlBar* pBar, BOOL bShow, BOOL bDelay, BOOL bActivate);
	

	virtual CBCGPControlBar* ControlBarFromPoint (CPoint pt);

	BOOL ReplaceControlBar (CBCGPControlBar* pOldBar, CBCGPControlBar* pNewBar);
	void AlignDockBar (const CRect& rectToAlignBy, CRect& rectResult, BOOL bMoveImmediately);

	int FindRowIndex (CBCGPDockBarRow* pRow);

	virtual void AdjustLayout ();
	virtual void AdjustDockingLayout ();
	virtual UINT GetDockBarID () const {return m_nDockBarID;}

	CBCGPControlBar* FindBarByID (UINT nID);

public:
	static int RectSideFromPoint (const CRect& rect, const CPoint& point);
		
	virtual BOOL OnSetWindowPos (const CWnd* pWndInsertAfter, const CRect& rectWnd, UINT nFlags);

	virtual BOOL IsAccessibilityCompatible () { return FALSE; }

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPDockBar)
	public:
	virtual BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, DWORD dwBCGStyle, CCreateContext* pContext = NULL);
	virtual BOOL CreateEx(DWORD dwStyleEx, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, DWORD dwBCGStyle, CCreateContext* pContext = NULL);
	protected:
	virtual void CalcWindowRect(LPRECT lpClientRect, UINT nAdjustType = adjustBorder);
	//}}AFX_VIRTUAL

	

// Implementation
public:
	virtual ~CBCGPDockBar();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPDockBar)
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnNcDestroy();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CBCGPDockBarRow* RowFromPoint (CPoint pt, bool& bOuterRow) const;
	CBCGPDockBarRow* RowFromControlBar (CBCGPBaseControlBar* pBar) const;
	CBCGPDockBarRow* FindNextVisibleRow (POSITION& pos, BOOL bForward = TRUE);
	void ResizeDockBarByOffset (int nOffset, BOOL bAdjustLayout = TRUE);
	CBCGPDockBarRow* FindRowByRect (CRect rectRow);

protected:
	CObList		m_lstControlBars;
	CObList		m_lstDockBarRows;

	UINT		m_nDockBarID; // for compatibility with MFC
};

#pragma warning( default : 4100 34 )

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPDOCKBAR_H__E6383A87_73D1_4CB4_8C45_EF46C6537FCA__INCLUDED_)
