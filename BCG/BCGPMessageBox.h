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
// BCGPMessageBox.h : header file
//

#if !defined(AFX_BCGPMESSAGEBOX_H__74E5EE7A_9BED_4A34_B359_95A187BA50CF__INCLUDED_)
#define AFX_BCGPMESSAGEBOX_H__74E5EE7A_9BED_4A34_B359_95A187BA50CF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"
#include "BCGPDialog.h"
#include "BCGPButton.h"

struct BCGCBPRODLLEXPORT BCGP_MSGBOXPARAMS : public MSGBOXPARAMS
{
	BOOL	bUseNativeControls;
	BOOL	bUseNativeCaption;
	BOOL	bDrawButtonsBanner;

	BOOL    bShowCheckBox;
	BOOL    bIsChecked;
	BOOL    bShowSeparator;  // Draw separator line above check box (only if check box present).
	LPCTSTR lpszCheckBoxText; 
	UINT*   puiAutoRespond;  // [in/out] points to variable that receives selected button ID when user checks "Don't ask me..." checkbox.

	BOOL    bIgnoreStandardButtons; // Show user-defined buttons only

	enum
	{
		UserButtonsCount = 4
	};

	LPCTSTR lpszUserButtonText[UserButtonsCount];
	UINT    uiUserButtonID[UserButtonsCount];

	inline BCGP_MSGBOXPARAMS ()
	{
		ZeroMemory (this, sizeof (BCGP_MSGBOXPARAMS));
		cbSize = sizeof (MSGBOXPARAMS);
		
		bDrawButtonsBanner = TRUE;
	}
};

class BCGCBPRODLLEXPORT CBCGPMessageBox : public CBCGPDialog
{
public:
	CBCGPMessageBox(const MSGBOXPARAMS* pParams);
	CBCGPMessageBox(const BCGP_MSGBOXPARAMS* pParams);
	~CBCGPMessageBox();

	enum { IDD = IDD_BCGBARRES_MSG_BOX };

	// Implementation
protected:
	void DoInitDialog();
	void Initialize ();
	void AddButton (UINT id, HINSTANCE hInst, LPCTSTR lpszCaption = NULL);

	CString GetString (LPCTSTR lpszText, LPCTSTR lpszDefault = NULL) const;
	// Generated message map functions
	//{{AFX_MSG(CBCGPMessageBox)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg void OnButton(UINT nID);
	afx_msg LRESULT OnHelp(WPARAM wp, LPARAM lp);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

	// Overridables
protected:
	virtual BOOL PreTranslateMessage (MSG* pMsg);
	virtual void DrawText (CDC* pDC);

	// Change these attributes to modify message box global appearance:
public:
	static BOOL	m_bUseNativeControls;
	static BOOL	m_bUseNativeCaption;
	static BOOL	m_bDontUseDefaultIcon;

protected:
	BCGP_MSGBOXPARAMS m_Params;
	HWND m_hwndOwner;

	HICON   m_hMessageIcon;
	CString m_strMessageCaption;
	CString m_strMessageText;
	CString m_strCheckBox;
	CFont   m_fntText;
	int     m_nDefaultButtonIndex;

	CArray <CWnd*, CWnd*> m_arrButtons;
	CBCGPButton m_wndDontAskCheckBox;
	bool    m_bHasCancelButton;
	DWORD   m_dwDrawTextFlags;
	CRect   m_rectButtons;
	CRect   m_rectIcon;
	CRect   m_rectText;
	int     m_cySeparatorLine;
	CPoint  m_ptNextButtonPos;
	bool    m_bRightAlignment;

	// Initial sizes
	CRect   m_rectClientMargins;
	int     m_cxIconSpacing;  // spacing between an icon and message text
	CSize   m_szButton;
	int     m_cyCheckBoxHeight;
	int     m_cxButtonSpacing;  // gap between buttons
	int     m_cyVerticalSpacing; // vertical spacing between text, optional check box and buttons
};

int BCGCBPRODLLEXPORT BCGPMessageBox (HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType, LPCTSTR lpszCheckBoxLabel = NULL, BOOL* pCheckBoxResult = NULL);
int BCGCBPRODLLEXPORT BCGPMessageBoxEx (HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType, WORD wLanguageId, LPCTSTR lpszCheckBoxLabel = NULL, BOOL* pCheckBoxResult = NULL);
int BCGCBPRODLLEXPORT BCGPMessageBoxIndirect (const MSGBOXPARAMS* pMsgBoxParams);
int BCGCBPRODLLEXPORT BCGPMessageBoxIndirect (const BCGP_MSGBOXPARAMS* pMsgBoxParams);

int BCGCBPRODLLEXPORT BCGPMessageBox (LPCTSTR lpszText, UINT nType = MB_OK, LPCTSTR lpszCheckBoxLabel = NULL, BOOL* pCheckBoxResult = NULL);
int BCGCBPRODLLEXPORT BCGPMessageBox (UINT nIDPrompt, UINT nType = MB_OK, UINT nIDCheckBoxPrompt = 0, BOOL* pCheckBoxResult = NULL);

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPMESSAGEBOX_H__74E5EE7A_9BED_4A34_B359_95A187BA50CF__INCLUDED_)
