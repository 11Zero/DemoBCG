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

#if !defined(AFX_BUTTONSLIST_H__6656D214_C47F_11D1_A644_00A0C93A70EC__INCLUDED_)
#define AFX_BUTTONSLIST_H__6656D214_C47F_11D1_A644_00A0C93A70EC__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// ButtonsList.h : header file
//

class CBCGPToolbarButton;
class CCustToolbarDlg;
class CBCGPToolBarImages;

/////////////////////////////////////////////////////////////////////////////
// CButtonsList window

class CButtonsList : public CButton
{
// Construction
public:
	CButtonsList();

// Operations
public:
	void SetImages (CBCGPToolBarImages* pImages);
	void AddButton (CBCGPToolbarButton* pButton);
	void RemoveButtons ();

	CBCGPToolbarButton* GetSelectedButton () const
	{
		return m_pSelButton;
	}

	BOOL SelectButton (int iImage);
	void EnableDragFromList (BOOL bEnable = TRUE)
	{
		m_bEnableDragFromList = bEnable;
	}

protected:
	CBCGPToolbarButton* HitTest (POINT point) const;
	void SelectButton (CBCGPToolbarButton* pButton);
	void RebuildLocations ();
	void RedrawSelection ();

// Attributes
protected:
	CObList				m_Buttons;			// CBCGPToolbarButton list
	CBCGPToolBarImages*	m_pImages;
	CSize				m_sizeButton;

	BOOL				m_bInited;

	CBCGPToolbarButton*	m_pSelButton;

	int					m_iScrollOffset;
	int					m_iScrollTotal;
	int					m_iScrollPage;
	CScrollBar			m_wndScrollBar;
	BOOL				m_bEnableDragFromList;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CButtonsList)
	public:
	virtual CScrollBar* GetScrollBarCtrl(int nBar) const;
	//}}AFX_VIRTUAL

	virtual void DrawItem (LPDRAWITEMSTRUCT lpDIS);

// Implementation
public:
	virtual ~CButtonsList();

	// Generated message map functions
protected:
	//{{AFX_MSG(CButtonsList)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnEnable(BOOL bEnable);
	afx_msg void OnSysColorChange();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg UINT OnGetDlgCode();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BUTTONSLIST_H__6656D214_C47F_11D1_A644_00A0C93A70EC__INCLUDED_)
