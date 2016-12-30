#if !defined(AFX_BCGPMASKEDIT_H__F333B383_1E7B_4C5B_A133_24CC735EAE84__INCLUDED_)
#define AFX_BCGPMASKEDIT_H__F333B383_1E7B_4C5B_A133_24CC735EAE84__INCLUDED_

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
// BCGPMaskEdit.h : header file
//

#include "BCGCBPro.h"
#include "BCGPEdit.h"

/////////////////////////////////////////////////////////////////////////////
// CBCGPMaskEdit window

class BCGCBPRODLLEXPORT CBCGPMaskEdit : public CBCGPEdit
{
	DECLARE_DYNAMIC(CBCGPMaskEdit)

// Construction
public:
	CBCGPMaskEdit();
	~CBCGPMaskEdit();

// Implementation
public:
	void EnableMask(LPCTSTR lpszMask, LPCTSTR lpszInputTemplate, 
					TCHAR chMaskInputTemplate = _T('_'), LPCTSTR lpszValid = NULL);
	void DisableMask();

	void SetValidChars(LPCTSTR lpszValid = NULL);
	void EnableGetMaskedCharsOnly(BOOL bEnable = TRUE) { m_bGetMaskedCharsOnly = bEnable; }
	void EnableSetMaskedCharsOnly(BOOL bEnable = TRUE) { m_bSetMaskedCharsOnly = bEnable; }
	void EnableSelectByGroup(BOOL bEnable = TRUE) { m_bSelectByGroup = bEnable; }
	
	// Replace standard operations
	// CWnd:	SetWindowText, GetWindowText

protected:
	virtual BOOL IsMaskedChar(TCHAR chChar, TCHAR chMaskChar) const;

	const CString GetValue() const { return m_str;}
	const CString GetMaskedValue(BOOL bWithSpaces = TRUE) const;
	BOOL SetValue(LPCTSTR lpszString, BOOL bWithDelimiters = TRUE);

protected:
	virtual BOOL CheckChar(TCHAR chChar, int nPos);

	void OnCharPrintchar(UINT nChar, UINT nRepCnt, UINT nFlags);
	void OnCharBackspace(UINT nChar, UINT nRepCnt, UINT nFlags);
	void OnCharDelete(UINT nChar, UINT nRepCnt, UINT nFlags);
	void GetGroupBounds(int &nBegin, int &nEnd, int nStartPos=0, BOOL bForward=TRUE);

	BOOL DoUpdate (BOOL bRestoreLastGood = TRUE, int nBeginOld = -1, int nEndOld = -1);
	
	virtual BOOL IsDrawPrompt();

// Attributes
protected:
	CString		m_str;					// Initial value
	CString		m_strMask;				// The mask string
	CString		m_strInputTemplate;		// "_" char = character entry
	TCHAR		m_chMaskInputTemplate;	// Default char
	CString		m_strValid;             // Valid string characters
	BOOL		m_bGetMaskedCharsOnly;
	BOOL		m_bSetMaskedCharsOnly;
	BOOL		m_bSelectByGroup;
	BOOL		m_bMaskKeyInProgress;		
	BOOL		m_bPasteProcessing;
	BOOL		m_bSetTextProcessing;

protected:
	// Generated message map functions
	//{{AFX_MSG(CBCGPMaskEdit)
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSetFocusR();
	//}}AFX_MSG
	LRESULT OnPaste(WPARAM, LPARAM);
	afx_msg LRESULT OnSetText (WPARAM, LPARAM);
	afx_msg LRESULT OnGetText(WPARAM, LPARAM);
	afx_msg LRESULT OnGetTextLength(WPARAM, LPARAM);
	afx_msg LRESULT OnCut(WPARAM, LPARAM);
	afx_msg LRESULT OnClear(WPARAM, LPARAM);
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPMASKEDIT_H__F333B383_1E7B_4C5B_A133_24CC735EAE84__INCLUDED_)
