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

#if !defined(AFX_BCGPEDIT_H__A7B40464_1C05_4981_A5F2_E21D74058A09__INCLUDED_)
#define AFX_BCGPEDIT_H__A7B40464_1C05_4981_A5F2_E21D74058A09__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BCGPEdit.h : header file
//

#include "BCGCBPro.h"

#if (!defined _BCGSUITE_) && (!defined _BCGSUITE_INC_)
#include "BCGPToolBarImages.h"
#endif

class CBCGPCalculatorPopup;
class CBCGPCalculator;
class CBCGPEditDropDownList;

/////////////////////////////////////////////////////////////////////////////
// CBCGPEdit window

#if (!defined _BCGSUITE_) && (!defined _BCGSUITE_INC_)
extern BCGCBPRODLLEXPORT UINT BCGM_EDIT_ON_FILL_AUTOCOMPLETE_LIST;
#endif

class BCGCBPRODLLEXPORT CBCGPEdit : public CEdit
{
	DECLARE_DYNAMIC(CBCGPEdit)

	friend class CBCGPCalculator;
	friend class CBCGPEditDropDownList;

// Construction
public:
	CBCGPEdit();

// Attributes
public:
	enum BrowseMode
	{
		BrowseMode_None,
		BrowseMode_Default,
		BrowseMode_Calculator,
		BrowseMode_File,
		BrowseMode_Folder,
	};

	CBCGPEdit::BrowseMode GetMode () const
	{
		return m_Mode;
	}

	BOOL				m_bVisualManagerStyle;
	BOOL				m_bOnGlass;

	void SetPrompt(LPCTSTR lpszPrompt, COLORREF clrText = (COLORREF)-1, BOOL bRedraw = TRUE);
	void EnableSearchMode(BOOL bEnable, LPCTSTR lpszPrompt, COLORREF clrText = (COLORREF)-1, BOOL bRedraw = TRUE);
	
	BOOL IsSearchMode() const
	{
		return m_bSearchMode;
	}

	CString GetSearchPrompt () const
	{
		return GetPrompt();
	};

	CString GetPrompt() const
	{
		return m_strSearchPrompt;
	};

	void SetErrorMessage(LPCTSTR lpszMessage, COLORREF clrText = (COLORREF)-1, BOOL bRedraw = TRUE);
	CString GetErrorMessage() const
	{
		return m_strErrorMessage;
	}

protected:
	CRect				m_rectBtn;
	BOOL				m_bIsButtonPressed;
	BOOL				m_bIsButtonHighlighted;
	BOOL				m_bIsButtonCaptured;
	BrowseMode			m_Mode;
	CBCGPToolBarImages	m_ImageBrowse;
	BOOL				m_bDefaultImage;
	CSize				m_sizeImage;
	CString				m_strLabel;
	CString				m_strDefFileExt;
	CString				m_strFileFilter;
	CString				m_strFolderBrowseTitle;
	int					m_nBrowseButtonWidth;
	BOOL				m_bSearchMode;
	BOOL				m_bHasPrompt;
	CString				m_strSearchPrompt;
	COLORREF			m_clrPrompt;
	CString				m_strErrorMessage;
	COLORREF			m_clrErrorText;
	CBCGPToolBarImages	m_ImageSearch;
	CBCGPToolBarImages	m_ImageClear;
	BOOL				m_bTextIsEmpty;
	CToolTipCtrl*		m_pToolTip;
	BOOL				m_bShowToolTip;
	BOOL				m_bDefaultPrintClient;

	CBCGPCalculatorPopup*	m_pCalcPopup;
	CStringList				m_lstCalcAdditionalCommands;
	CList<UINT, UINT>		m_lstCalcExtCommands;
	CString					m_strCalcDisplayFormat;

#if (!defined _BCGSUITE_) && (!defined _BCGSUITE_INC_)
	CBCGPEditDropDownList*	m_pDropDownPopup;
	BOOL					m_bInAutoComplete;
#endif

// Operations
public:
	void EnableBrowseButton (BOOL bEnable = TRUE, LPCTSTR szLabel = _T("..."));
	void EnableFileBrowseButton (LPCTSTR lpszDefExt = NULL, LPCTSTR lpszFilter = NULL);
	void EnableFolderBrowseButton (LPCTSTR lpszTitle = NULL);
	void EnableCalculatorButton (const CStringList* plstAdditionalCommands = NULL,
		const CList<UINT, UINT>* plstExtCommands = NULL,
		LPCTSTR lpszDisplayFormat = NULL);

	void SetBrowseButtonImage (HICON hIcon, BOOL bAutoDestroy = TRUE, BOOL bAlphaBlend = FALSE);
	void SetBrowseButtonImage (HBITMAP hBitmap, BOOL bAutoDestroy = TRUE);
	void SetBrowseButtonImage (UINT uiBmpResId);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPEdit)
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

	virtual void OnBrowse ();
	virtual void OnDrawBrowseButton (CDC* pDC, CRect rect, BOOL bIsButtonPressed, BOOL bIsButtonHot);
	virtual void OnChangeLayout ();
	virtual void OnAfterUpdate ();
	virtual BOOL FilterCalcKey (int nChar);

	virtual void OnCalculatorUserCommand (CBCGPCalculator* pCalculator, UINT uiCmd);
	virtual BOOL OnIllegalFileName (CString& strFileName);

	virtual BOOL IsDrawPrompt();

	virtual void DoPaint(CDC* pDC, BOOL bDrawPrompt, BOOL bIsPrint);
	virtual void DoNcPaint(CDC* pDC, BOOL bIsPrint);

#if (!defined _BCGSUITE_) && (!defined _BCGSUITE_INC_)
	virtual BOOL OnGetAutoCompleteList(const CString& strEditText, CStringList& lstAutocomplete);
#endif

// Implementation
public:
	virtual ~CBCGPEdit();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPEdit)
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp);
	afx_msg void OnNcPaint();
	afx_msg void OnNcLButtonDblClk(UINT nHitTest, CPoint point);
	afx_msg void OnNcMouseMove(UINT nHitTest, CPoint point);
	afx_msg void OnCancelMode();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnPaint();
	afx_msg BOOL OnChange();
	afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
	afx_msg void OnDestroy();
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG
	afx_msg BCGNcHitTestType OnNcHitTest(CPoint point);
	afx_msg LRESULT OnBCGSetControlVMMode (WPARAM, LPARAM);
	afx_msg LRESULT OnBCGSetControlAero (WPARAM, LPARAM);
	afx_msg BOOL OnNeedTipText(UINT id, NMHDR* pNMH, LRESULT* pResult);
	afx_msg LRESULT OnPrintClient(WPARAM wParam, LPARAM lp);
	afx_msg LRESULT OnPrint(WPARAM wParam, LPARAM lp);
	DECLARE_MESSAGE_MAP()

	void CreateAutocompleteList(const CStringList& lstAutocomplete);
	void CloseAutocompleteList();
	void SetIntenalImage ();
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPEDIT_H__A7B40464_1C05_4981_A5F2_E21D74058A09__INCLUDED_)
