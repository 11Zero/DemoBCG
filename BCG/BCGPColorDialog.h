#if !defined(AFX_BCGPCOLORDIALOG_H__2C7F710C_9879_49AC_B4F5_3A6699211BE3__INCLUDED_)
#define AFX_BCGPCOLORDIALOG_H__2C7F710C_9879_49AC_B4F5_3A6699211BE3__INCLUDED_

// This is a part of the BCGControlBar Library
// Copyright (C) 1998-2014 BCGSoft Ltd.
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BCGColorDialog.h : header file
//

class CBCGPPropertySheetCtrl;

#include "BCGCBPro.h"
#include "BCGPDialog.h"
#include "bcgprores.h"
#include "BCGPPowerColorPicker.h"
#include "BCGPButton.h"
#include "ColorPage1.h"
#include "ColorPage2.h"

/////////////////////////////////////////////////////////////////////////////
// CBCGPColorDialog dialog

class BCGCBPRODLLEXPORT CBCGPColorDialog : public CBCGPDialog
{
	DECLARE_DYNAMIC(CBCGPColorDialog)

// Construction

public:
	CBCGPColorDialog (COLORREF clrInit = 0, DWORD dwFlags = 0 /* reserved */, 
					CWnd* pParentWnd = NULL,
					HPALETTE hPal = NULL);

	virtual ~CBCGPColorDialog ();

	void SetCurrentColor(COLORREF rgb);
	void SetNewColor(COLORREF rgb);

	COLORREF GetColor () const
	{
		return m_NewColor;
	}

	void SetPageTwo(BYTE R, BYTE G, BYTE B);
	void SetPageOne(BYTE R, BYTE G, BYTE B);

	CPalette* GetPalette () const
	{
		return m_pPalette;
	}

// Dialog Data
	//{{AFX_DATA(CBCGPColorDialog)
	enum { IDD = IDD_BCGBARRES_COLOR_DLG };
	CBCGPButton	m_btnColorSelect;
	CStatic	m_wndStaticPlaceHolder;
	CBCGPColorPickerCtrl m_wndColors;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPColorDialog)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CBCGPPropertySheetCtrl* m_pPropSheet;
	CBCGPColorPage1* m_pColourSheetOne;
	CBCGPColorPage2* m_pColourSheetTwo;

	COLORREF	m_CurrentColor;
	COLORREF	m_NewColor;
	CPalette*	m_pPalette;
	BOOL		m_bIsMyPalette;

	BOOL		m_bPickerMode;
	HCURSOR		m_hcurPicker;

	// Generated message map functions
	//{{AFX_MSG(CBCGPColorDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnSysColorChange();
	afx_msg void OnColorSelect();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG
#if (_MSC_VER < 1300)
	afx_msg void OnActivateApp(BOOL bActive, HTASK hTask);
#else
	afx_msg void OnActivateApp(BOOL bActive, DWORD dwThreadID);
#endif
	DECLARE_MESSAGE_MAP()

	void RebuildPalette ();
};

/////////////////////////////////////////////////////////////////////////////
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPCOLORDIALOG_H__2C7F710C_9879_49AC_B4F5_3A6699211BE3__INCLUDED_)
