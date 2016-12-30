#if !defined(AFX_BCGPEDITVIEW_H__7C200AA0_5BE4_4C72_B0CF_2A5C51D18712__INCLUDED_)
#define AFX_BCGPEDITVIEW_H__7C200AA0_5BE4_4C72_B0CF_2A5C51D18712__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

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
// BCGPEditView.h : header file
//

#include "BCGCBPro.h"

#ifndef BCGP_EXCLUDE_EDIT_CTRL

class CBCGPEditCtrl;

class CBCGPEditFindDlg : public CFindReplaceDialog
{
friend class CBCGPEditView;

	virtual ~CBCGPEditFindDlg ();

protected:
	CBCGPEditView* m_pParent;
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPEditView view

class BCGCBPRODLLEXPORT CBCGPEditView : public CView
{
	friend class CBCGPEditFindDlg;

protected:
	DECLARE_DYNCREATE(CBCGPEditView)
	CBCGPEditView();           // protected constructor used by dynamic creation

// Attributes
public:
	CBCGPEditCtrl* GetEditCtrl () const
	{
		return m_pWndEditCtrl;
	}

	BOOL IsDisableMainframeForFindDlg () const
	{
		return m_bDisableMainframeForFindDlg;
	}

	void DisableMainframeForFindDlg (BOOL bDisable)
	{
		m_bDisableMainframeForFindDlg = bDisable;
	}

	static BOOL			m_bUpdateFindString;

protected:
	CBCGPEditCtrl*		m_pWndEditCtrl;
	CBCGPEditFindDlg*	m_pFindDlg;
	BOOL				m_bDisableMainframeForFindDlg;

	static CString		m_strFindText;
	static CString		m_strReplaceText;
	static DWORD		m_dwFindMask;

// Operations
public:
	void SerializeRaw(CArchive& ar);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPEditView)
	public:
	virtual void OnPrepareDC(CDC* pDC, CPrintInfo* pInfo = NULL);
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnPrint(CDC* pDC, CPrintInfo* pInfo);
	//}}AFX_VIRTUAL

	virtual BOOL FindText (const CString& strFind, DWORD dwFindMask);
	virtual void OnTextNotFound (LPCTSTR /*lpszFind*/) {}
	virtual void OnPrepareFindString (CString& strFind);

// Implementation
protected:
	virtual ~CBCGPEditView();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	virtual CBCGPEditCtrl* CreateEdit ();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPEditView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	afx_msg void OnFilePrintPreview();
	afx_msg void OnEditPreviousbookmark();
	afx_msg void OnEditCut();
	afx_msg void OnEditCopy();
	afx_msg void OnEditPaste();
	afx_msg void OnEditSelectAll();
	afx_msg void OnUpdateEditCut(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditCopy(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditPaste(CCmdUI* pCmdUI);
	afx_msg void OnEditRedo();
	afx_msg void OnUpdateEditRedo(CCmdUI* pCmdUI);
	afx_msg void OnEditUndo();
	afx_msg void OnUpdateEditUndo(CCmdUI* pCmdUI);
	afx_msg void OnEditFind();
	afx_msg void OnEditReplace();
	afx_msg void OnUpdateEditFindReplace(CCmdUI* pCmdUI);
	afx_msg LRESULT OnFindReplace(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnEditChange(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};

#endif// BCGP_EXCLUDE_EDIT_CTRL

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPEDITVIEW_H__7C200AA0_5BE4_4C72_B0CF_2A5C51D18712__INCLUDED_)
