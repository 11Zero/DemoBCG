#if !defined(AFX_COLORPOPUP_H__6ABB7DC6_CCA2_4B65_9269_3F41D3F21D3D__INCLUDED_)
#define AFX_COLORPOPUP_H__6ABB7DC6_CCA2_4B65_9269_3F41D3F21D3D__INCLUDED_

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
// ColorPopup.h : header file
//

#include "BCGPPopupMenu.h"
#include "BCGPColorBar.h"

class CBCGPControlBar;
class CBCGPPropList;
class CBCGPRibbonColorButton;

/////////////////////////////////////////////////////////////////////////////
// CColorPopup window

class BCGCBPRODLLEXPORT CColorPopup : public CBCGPPopupMenu
{
	friend class CBCGPColorMenuButton;
	friend class CBCGPColorButton;
	friend class CBCGPColorProp;
	friend class CBCGPGridColorItem;

	DECLARE_DYNAMIC(CColorPopup)

// Construction
public:
    CColorPopup (	const CArray<COLORREF, COLORREF>& colors,
					COLORREF color, 
					LPCTSTR lpszAutoColor,
					LPCTSTR lpszOtherColor,
					LPCTSTR lpszDocColors,
					CList<COLORREF,COLORREF>& lstDocColors,
					int nColumns,
					int nHorzDockRows,
					int nVertDockColumns,
					COLORREF colorAutomatic,
					UINT uiCommandID,
					BOOL bStdColorDlg = FALSE)
		: m_wndColorBar (colors, color, lpszAutoColor, lpszOtherColor,
						lpszDocColors, lstDocColors, nColumns, 
						nHorzDockRows, nVertDockColumns, colorAutomatic,
						uiCommandID, NULL)
	{
		m_bEnabledInCustomizeMode = FALSE;
		m_wndColorBar.m_bStdColorDlg = bStdColorDlg;
	}

    CColorPopup (	CBCGPColorButton* pParentBtn,
					const CArray<COLORREF, COLORREF>& colors,
					COLORREF color, 
					LPCTSTR lpszAutoColor,
					LPCTSTR lpszOtherColor,
					LPCTSTR lpszDocColors,
					CList<COLORREF,COLORREF>& lstDocColors,
					int nColumns,
					COLORREF colorAutomatic)
		: m_wndColorBar (colors, color, lpszAutoColor, lpszOtherColor,
						lpszDocColors, lstDocColors, nColumns, 
						-1, -1, colorAutomatic,
						(UINT)-1,
						pParentBtn)
	{
		m_bEnabledInCustomizeMode = FALSE;
	}

    CColorPopup (	CBCGPRibbonColorButton* pParentBtn,
					const CArray<COLORREF, COLORREF>& colors,
					COLORREF color, 
					LPCTSTR lpszAutoColor,
					LPCTSTR lpszOtherColor,
					LPCTSTR lpszDocColors,
					CList<COLORREF,COLORREF>& lstDocColors,
					int nColumns,
					COLORREF colorAutomatic,
					UINT nID)
		: m_wndColorBar (colors, color, lpszAutoColor, lpszOtherColor,
						lpszDocColors, lstDocColors, nColumns, 
						colorAutomatic,
						nID,
						pParentBtn)
	{
		m_bEnabledInCustomizeMode = FALSE;
	}

	virtual ~CColorPopup();

// Attributes
protected:
	CBCGPColorBar	m_wndColorBar;
	BOOL			m_bEnabledInCustomizeMode;

public:
	void SetPropList (CBCGPPropList* pWndList)
	{
		m_wndColorBar.SetPropList (pWndList);
	}

	void SetParentGrid (CBCGPGridCtrl* pWndGrid)
	{
		m_wndColorBar.SetParentGrid (pWndGrid);
	}

// Operations
public:

// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CColorPopup)
	//}}AFX_VIRTUAL

	virtual CBCGPPopupMenuBar* GetMenuBar ()
	{
		return &m_wndColorBar;
	}

	virtual CBCGPControlBar* CreateTearOffBar (CFrameWnd* pWndMain, UINT uiID, LPCTSTR lpszName);
	virtual CWnd* GetParentArea (CRect& rectParentBtn);

    // Generated message map functions
protected:
    //{{AFX_MSG(CColorPopup)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COLORPOPUP_H__6ABB7DC6_CCA2_4B65_9269_3F41D3F21D3D__INCLUDED_)
