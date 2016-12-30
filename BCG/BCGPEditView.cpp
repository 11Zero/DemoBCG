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
// BCGPEditView.cpp : implementation file
//

#include "stdafx.h"
#include "BCGPEditCtrl.h"

#ifndef BCGP_EXCLUDE_EDIT_CTRL

#ifndef _BCGPEDIT_STANDALONE
#ifndef _BCGSUITE_
#include "BCGPPrintPreviewView.h"
#endif
#endif

#include "BCGPEditView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static UINT WM_FINDREPLACE = ::RegisterWindowMessage(FINDMSGSTRING);

CBCGPEditFindDlg::~CBCGPEditFindDlg ()
{
	if (m_pParent != NULL)
	{
		ASSERT_VALID (m_pParent);

		m_pParent->m_pFindDlg = NULL;
		if (m_pParent->IsDisableMainframeForFindDlg ())
		{
			AfxGetMainWnd()->ModifyStyle(WS_DISABLED,0);
			m_pParent->SetFocus();
		}
	}
}


/////////////////////////////////////////////////////////////////////////////
// CBCGPEditView

#define ID_EDITCTRL 1

IMPLEMENT_DYNCREATE(CBCGPEditView, CView)

DWORD CBCGPEditView::m_dwFindMask = FR_DOWN;
CString CBCGPEditView::m_strFindText;
CString CBCGPEditView::m_strReplaceText;
BOOL	CBCGPEditView::m_bUpdateFindString = TRUE;

CBCGPEditView::CBCGPEditView()
{
	m_pWndEditCtrl = NULL;
	m_pFindDlg = NULL;
	m_bDisableMainframeForFindDlg = TRUE;
}
//*******************************************************************************
CBCGPEditView::~CBCGPEditView()
{
}

BEGIN_MESSAGE_MAP(CBCGPEditView, CView)
	//{{AFX_MSG_MAP(CBCGPEditView)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_SETFOCUS()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CUT, OnUpdateEditCut)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdateEditPaste)
	ON_COMMAND(ID_EDIT_SELECT_ALL, OnEditSelectAll)
	ON_COMMAND(ID_EDIT_FIND, OnEditFind)
	ON_COMMAND(ID_EDIT_REPLACE, OnEditReplace)
	ON_UPDATE_COMMAND_UI(ID_EDIT_FIND, OnUpdateEditFindReplace)
	ON_UPDATE_COMMAND_UI(ID_EDIT_REPLACE, OnUpdateEditFindReplace)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, OnFilePrintPreview)
	ON_COMMAND(ID_FILE_PRINT, CBCGPEditView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CBCGPEditView::OnFilePrint)
	// Undo/redo
	ON_COMMAND(ID_EDIT_REDO, OnEditRedo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_REDO, OnUpdateEditRedo)
	ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, OnUpdateEditUndo)
	ON_REGISTERED_MESSAGE(WM_FINDREPLACE, OnFindReplace)
	ON_REGISTERED_MESSAGE(BCGM_ON_EDITCHANGE, OnEditChange)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGPEditView drawing

void CBCGPEditView::OnDraw(CDC* /*pDC*/)
{
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPEditView diagnostics

#ifdef _DEBUG
void CBCGPEditView::AssertValid() const
{
	CView::AssertValid();
}

void CBCGPEditView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

void CBCGPEditView::SerializeRaw(CArchive& ar)
{
	ASSERT_VALID(this);

	if (m_pWndEditCtrl != NULL)
	{
		ASSERT_VALID (m_pWndEditCtrl);
		m_pWndEditCtrl->Serialize(ar);
	}

	ASSERT_VALID(this);
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPEditView message handlers

CBCGPEditCtrl* CBCGPEditView::CreateEdit ()
{
	return new CBCGPEditCtrl;
}
//*******************************************************************************
int CBCGPEditView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	m_pWndEditCtrl = CreateEdit ();
	if (m_pWndEditCtrl == NULL)
	{
		TRACE0("CBCGPEditView::OnCreate: edit control is not created\n");
		return -1;
	}

	ASSERT_VALID (m_pWndEditCtrl);
	ASSERT (m_pWndEditCtrl->IsKindOf (RUNTIME_CLASS (CBCGPEditCtrl)));

	if (!m_pWndEditCtrl->Create (WS_CHILD | WS_VISIBLE, 
		CRect (0, 0, 0, 0), this, ID_EDITCTRL))
	{
		TRACE0("CBCGPEditView::OnCreate: cannot create edit control\n");
		return -1;
	}

	return 0;
}
//*******************************************************************************
void CBCGPEditView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);
	
	if (m_pWndEditCtrl->GetSafeHwnd () != NULL)
	{
		m_pWndEditCtrl->SetWindowPos (NULL, -1, -1, cx, cy,
			SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
	}
}
//*******************************************************************************
BOOL CBCGPEditView::OnEraseBkgnd(CDC* /*pDC*/) 
{
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPEditView printing

BOOL CBCGPEditView::OnPreparePrinting(CPrintInfo* pInfo)
{
	return DoPreparePrinting(pInfo);
}
//*******************************************************************************
void CBCGPEditView::OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo)
{
	if (m_pWndEditCtrl != NULL)
	{
		ASSERT_VALID (m_pWndEditCtrl);
		m_pWndEditCtrl->OnBeginPrinting (pDC, pInfo);
	}
}
//*******************************************************************************
void CBCGPEditView::OnEndPrinting(CDC* pDC, CPrintInfo* pInfo)
{
	if (m_pWndEditCtrl != NULL)
	{
		ASSERT_VALID (m_pWndEditCtrl);
		m_pWndEditCtrl->OnEndPrinting (pDC, pInfo);
	}
}
//*******************************************************************************
void CBCGPEditView::OnFilePrintPreview() 
{
#ifndef _BCGPEDIT_STANDALONE
	BCGPPrintPreview (this);
#else
	CView::OnFilePrintPreview();
#endif
}
//*******************************************************************************
void CBCGPEditView::OnEditCut() 
{
	if (m_pWndEditCtrl != NULL)
	{
		ASSERT_VALID (m_pWndEditCtrl);
		m_pWndEditCtrl->Cut();
	}
}
//*******************************************************************************
void CBCGPEditView::OnEditCopy() 
{
	if (m_pWndEditCtrl != NULL)
	{
		ASSERT_VALID (m_pWndEditCtrl);
		m_pWndEditCtrl->Copy();
	}
}
//*******************************************************************************
void CBCGPEditView::OnEditPaste() 
{
	if (m_pWndEditCtrl != NULL)
	{
		ASSERT_VALID (m_pWndEditCtrl);
		m_pWndEditCtrl->Paste();
	}
}
//*******************************************************************************
void CBCGPEditView::OnEditSelectAll() 
{
	if (m_pWndEditCtrl != NULL)
	{
		ASSERT_VALID (m_pWndEditCtrl);
		m_pWndEditCtrl->MakeSelection (CBCGPEditCtrl::ST_ALL_TEXT);
	}
}
//*******************************************************************************
void CBCGPEditView::OnUpdateEditCut(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable (m_pWndEditCtrl != NULL && m_pWndEditCtrl->IsCutEnabled());
}
//*******************************************************************************
void CBCGPEditView::OnUpdateEditCopy(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable (m_pWndEditCtrl != NULL && m_pWndEditCtrl->IsCopyEnabled());
}
//*******************************************************************************
void CBCGPEditView::OnUpdateEditPaste(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable (m_pWndEditCtrl != NULL && m_pWndEditCtrl->IsPasteEnabled());
}
//*******************************************************************************
void CBCGPEditView::OnPrint(CDC* pDC, CPrintInfo* pInfo) 
{
	if (m_pWndEditCtrl != NULL)
	{
		ASSERT_VALID (m_pWndEditCtrl);
		m_pWndEditCtrl->OnPrint (pDC, pInfo);
	}
}
//*******************************************************************************
void CBCGPEditView::OnPrepareDC(CDC* pDC, CPrintInfo* pInfo) 
{
	if (!pDC->IsPrinting ())
	{
		CView::OnPrepareDC(pDC, pInfo);
	}
}
//*******************************************************************************
void CBCGPEditView::OnSetFocus(CWnd* pOldWnd) 
{
	CView::OnSetFocus(pOldWnd);

	if (m_pWndEditCtrl != NULL)
	{
		ASSERT_VALID (m_pWndEditCtrl);
		m_pWndEditCtrl->SetFocus();
	}
}
//*******************************************************************************
void CBCGPEditView::OnDestroy() 
{
	if (m_pWndEditCtrl != NULL)
	{
		ASSERT_VALID (m_pWndEditCtrl);

		m_pWndEditCtrl->DestroyWindow ();
		delete m_pWndEditCtrl;
		m_pWndEditCtrl = NULL;
	}

	if (m_pFindDlg != NULL)
	{
		m_pFindDlg->PostMessage(WM_CLOSE);
		m_pFindDlg->m_pParent = NULL;
	}

	CView::OnDestroy();
}
//*******************************************************************************
void CBCGPEditView::OnEditRedo() 
{
	if (m_pWndEditCtrl != NULL)
	{
		ASSERT_VALID (m_pWndEditCtrl);
		m_pWndEditCtrl->OnRedo();
	}
}
//*******************************************************************************
void CBCGPEditView::OnUpdateEditRedo(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable (m_pWndEditCtrl != NULL && m_pWndEditCtrl->CanRedo());
}
//*******************************************************************************
void CBCGPEditView::OnEditUndo() 
{
	if (m_pWndEditCtrl != NULL)
	{
		ASSERT_VALID (m_pWndEditCtrl);
		m_pWndEditCtrl->OnUndo();
	}
}
//*******************************************************************************
void CBCGPEditView::OnUpdateEditUndo(CCmdUI* pCmdUI) 
{
	ASSERT_VALID (m_pWndEditCtrl);

	pCmdUI->Enable (m_pWndEditCtrl->CanUndo());
}
//*******************************************************************************
void CBCGPEditView::OnEditFind()
{
	if (m_pFindDlg != NULL)
	{
		m_pFindDlg->SetFocus ();
		return;
	}

	m_pFindDlg = new CBCGPEditFindDlg;
	m_pFindDlg->m_pParent = this;
	
	OnPrepareFindString (m_strFindText);

	if (m_pFindDlg->Create (TRUE, m_strFindText, NULL, m_dwFindMask, this))
	{
		if (IsDisableMainframeForFindDlg ())
		{
			AfxGetMainWnd()->ModifyStyle(0,WS_DISABLED);
		}
	}
	else
	{
		delete m_pFindDlg;
	}
}
//*******************************************************************************
void CBCGPEditView::OnUpdateEditFindReplace(CCmdUI* pCmdUI)
{
	ASSERT_VALID (m_pWndEditCtrl);

	pCmdUI->Enable (!m_pWndEditCtrl->GetText ().IsEmpty ());
}
//*******************************************************************************
void CBCGPEditView::OnEditReplace()
{
	if (m_pFindDlg != NULL)
	{
		m_pFindDlg->SetFocus ();
		return;
	}

	m_pFindDlg = new CBCGPEditFindDlg;
	m_pFindDlg->m_pParent = this;
	
	OnPrepareFindString (m_strFindText);

	if (m_pFindDlg->Create (FALSE, m_strFindText, m_strReplaceText, m_dwFindMask, this))
	{
		if (IsDisableMainframeForFindDlg ())
		{
			AfxGetMainWnd()->ModifyStyle(0,WS_DISABLED);
		}
	}
	else
	{
		delete m_pFindDlg;
	}
}
//*******************************************************************************
LRESULT CBCGPEditView::OnFindReplace(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	if (m_pFindDlg == NULL)
	{
		if (!FindText(m_strFindText, m_dwFindMask))
		{
			OnTextNotFound(m_strFindText);
		}

		SetFocus();
	}
	else if (!m_pFindDlg->IsTerminating())
	{
		m_strFindText = m_pFindDlg->GetFindString();
		m_strReplaceText = m_pFindDlg->GetReplaceString();
		m_dwFindMask = m_pFindDlg->m_fr.Flags;

		if (!FindText(m_strFindText,m_dwFindMask))
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
//*******************************************************************************
BOOL CBCGPEditView::FindText (const CString& strFind, DWORD dwFindMask)
{
	if (m_pWndEditCtrl == NULL)
	{
		return FALSE;
	}

	ASSERT_VALID (m_pWndEditCtrl);

	if ((dwFindMask & FR_REPLACE) != 0)
	{
		return m_pWndEditCtrl->ReplaceText(strFind, m_strReplaceText,
							(dwFindMask & FR_DOWN) != 0,
							(dwFindMask & FR_MATCHCASE) != 0,
							(dwFindMask & FR_WHOLEWORD) != 0);
	}
	else if ((dwFindMask & FR_REPLACEALL) != 0)
	{
		return m_pWndEditCtrl->ReplaceAll(strFind, m_strReplaceText,
							(dwFindMask & FR_DOWN) != 0,
							(dwFindMask & FR_MATCHCASE) != 0,
							(dwFindMask & FR_WHOLEWORD) != 0) > 0;
	}
	else
	{
		return m_pWndEditCtrl->FindText(strFind,
							(dwFindMask & FR_DOWN) != 0,
							(dwFindMask & FR_MATCHCASE) != 0,
							(dwFindMask & FR_WHOLEWORD) != 0);
	}
}
//*******************************************************************************
LRESULT CBCGPEditView::OnEditChange(WPARAM, LPARAM)
{
	ASSERT_VALID(this);

	if (GetDocument() != NULL)
	{
		GetDocument()->SetModifiedFlag();
		ASSERT_VALID(this);
	}

	return 0;
}
//*******************************************************************************
void CBCGPEditView::OnPrepareFindString (CString& strFind)
{
	if (m_pWndEditCtrl != NULL && m_bUpdateFindString)
	{
		ASSERT_VALID (m_pWndEditCtrl);

		int i1, i2;
		m_pWndEditCtrl->GetSel (i1, i2);
		int iStartSel = min (i1, i2);
		int iEndSel = max (i1, i2);

		if (iStartSel < 0 || iEndSel <= iStartSel)
		{
			int iCurOffset = m_pWndEditCtrl->GetCurOffset ();
			m_pWndEditCtrl->GetWordFromOffset(iCurOffset, strFind);
		}
		else
		{
			const CString& strBuffer = m_pWndEditCtrl->GetText ();
			CString strSel = strBuffer.Mid (iStartSel, iEndSel - iStartSel);
			if (strSel.Find (_T('\n')) == -1)
			{
				strSel.Replace (_T('\t'), _T(' '));
				strFind = strSel;
			}
		}
	}
}

#endif // BCGP_EXCLUDE_EDIT_CTRL

