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
// BCGPDiagramView.h : header file
//

#if !defined(AFX_BCGPDIAGRAMVIEW_H__7620EB4D_7760_4FB8_AF93_988DB65BBA9D__INCLUDED_)
#define AFX_BCGPDIAGRAMVIEW_H__7620EB4D_7760_4FB8_AF93_988DB65BBA9D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"

class CBCGPDiagramVisualContainerCtrl;
class CBCGPDiagramVisualContainer;

/////////////////////////////////////////////////////////////////////////////
// CBCGPDiagramView view

class BCGCBPRODLLEXPORT CBCGPDiagramView : public CView
{
protected:
	CBCGPDiagramView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CBCGPDiagramView)

// Attributes
public:
	CBCGPDiagramVisualContainerCtrl* GetDiagramCtrl () const
	{
		return m_pWndDiagramCtrl;
	}

	CBCGPDiagramVisualContainer* GetDiagram() const;

protected:
	CBCGPDiagramVisualContainerCtrl*	m_pWndDiagramCtrl;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPDiagramView)
	protected:
	virtual void OnPrepareDC(CDC* pDC, CPrintInfo* pInfo = NULL);
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnPrint(CDC* pDC, CPrintInfo* pInfo);
	//}}AFX_VIRTUAL

	virtual CBCGPDiagramVisualContainerCtrl* CreateDiagram ();

// Implementation
protected:
	virtual ~CBCGPDiagramView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPDiagramView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	afx_msg void OnFilePrintPreview();
	afx_msg LRESULT OnPrepareTaskBarPreview(WPARAM wp, LPARAM lp);
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPDIAGRAMVIEW_H__7620EB4D_7760_4FB8_AF93_988DB65BBA9D__INCLUDED_)
