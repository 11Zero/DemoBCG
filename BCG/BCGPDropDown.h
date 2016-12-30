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

#if !defined(AFX_DROPDOWN_TOOLBAR)
#define AFX_DROPDOWN_TOOLBAR

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// BCGPDropDown.h : header file
//

#include "BCGCBPro.h"
#include "BCGPPopupMenuBar.h"

class CBCGPDropDownToolbarButton;
class CBCGPMenuBar;
class CBCGPToolbarMenuButton;

class BCGCBPRODLLEXPORT CBCGPDropDownToolBar : public CBCGPToolBar
{
	friend class CBCGPDropDownToolbarButton;
	friend class CBCGPDropDownFrame;

	DECLARE_SERIAL(CBCGPDropDownToolBar)
	
public:
	CBCGPDropDownToolBar()
	{
		m_bLocked = TRUE;
	}

	virtual ~CBCGPDropDownToolBar() {}
   
	virtual BOOL OnSendCommand (const CBCGPToolbarButton* pButton);

	virtual BOOL LoadBitmap (UINT uiResID, UINT uiColdResID = 0, 
					UINT uiMenuResID = 0, BOOL /*bLocked*/ = FALSE,
					UINT uiDisabledResID = 0, UINT uiMenuDisabledResID = 0)
	{
		return CBCGPToolBar::LoadBitmap (uiResID, uiColdResID, 
					uiMenuResID, TRUE /* Locked */,
					uiDisabledResID, uiMenuDisabledResID);
	}

	virtual BOOL LoadToolBar (UINT uiResID, UINT uiColdResID = 0, 
					UINT uiMenuResID = 0, BOOL /*bLocked*/ = FALSE,
					UINT uiDisabledResID = 0, UINT uiMenuDisabledResID = 0,
					UINT uiHotResID = 0)
	{
		return CBCGPToolBar::LoadToolBar (uiResID, uiColdResID, 
					uiMenuResID, /*bLocked*/ TRUE,
					uiDisabledResID, uiMenuDisabledResID, uiHotResID);
	}

	virtual void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler);
	virtual BOOL AllowShowOnControlMenu () const			{	return FALSE;	}

	// Generated message map functions
	//{{AFX_MSG(CBCGPDropDownToolBar)
   afx_msg void OnMouseMove(UINT nFlags, CPoint point);
   afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPDropDownFrame frame

class CBCGPDropDownFrame : public CMiniFrameWnd
{
	friend CBCGPDropDownToolbarButton;
	friend CBCGPDropDownToolBar;

	DECLARE_SERIAL(CBCGPDropDownFrame)

public:
	CBCGPDropDownFrame();
	virtual ~CBCGPDropDownFrame();

// Attributes
public:
	BOOL					m_bAutoDestroyParent;
	void SetAutoDestroy (BOOL bAutoDestroy = TRUE)
	{
		m_bAutoDestroy = bAutoDestroy;
	}

protected:
	CBCGPDropDownToolbarButton*	m_pParentBtn;
	CBCGPDropDownToolBar			m_wndToolBar;
	CBCGPDropDownToolBar*		m_pWndOriginToolbar;

	int							m_x;
	int							m_y;
	BOOL						m_bAutoDestroy;

	CString						m_strCaption;

protected:
	static CString	m_strClassName;

// Operations
public:
	CBCGPDropDownFrame* GetParentPopupMenu () const;
	CBCGPMenuBar* GetParentMenuBar () const;

protected:
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPDropDownFrame)
	public:
	virtual BOOL Create(CWnd* pWndParent, int x, int y, CBCGPDropDownToolBar* pWndOriginToolbar);
	virtual void RecalcLayout(BOOL bNotify = TRUE);
	protected:
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CBCGPDropDownFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
#if _MSC_VER >= 1300
	afx_msg void OnActivateApp(BOOL bActive, DWORD dwThreadID);
#else
	afx_msg void OnActivateApp(BOOL bActive, HTASK hTask);
#endif
	DECLARE_MESSAGE_MAP()
};

class BCGCBPRODLLEXPORT CBCGPDropDownToolbarButton : public CBCGPToolbarButton  
{
	friend class CBCGPDropDownFrame;

	DECLARE_SERIAL(CBCGPDropDownToolbarButton)

public:
	CBCGPDropDownToolbarButton();
	CBCGPDropDownToolbarButton (LPCTSTR lpszName, CBCGPDropDownToolBar* pToolBar);
	virtual ~CBCGPDropDownToolbarButton();

// Overrides:
public:
	virtual void CopyFrom (const CBCGPToolbarButton& src);
	virtual void Serialize (CArchive& ar);
	virtual void OnDraw (CDC* pDC, const CRect& rect, CBCGPToolBarImages* pImages,
						BOOL bHorz = TRUE, BOOL bCustomizeMode = FALSE,
						BOOL bHighlight = FALSE,
						BOOL bDrawBorder = TRUE,
						BOOL bGrayDisabledButtons = TRUE);
	virtual BOOL OnClick (CWnd* pWnd, BOOL bDelay = TRUE);
	virtual BOOL OnClickUp();
	virtual void OnChangeParentWnd (CWnd* pWndParent);
	virtual void OnCancelMode ();
	virtual BOOL OnContextHelp (CWnd* pWnd)
	{
		return OnClick (pWnd, FALSE);
	}

	virtual int OnDrawOnCustomizeList (CDC* pDC, const CRect& rect, 
										BOOL bSelected);
	virtual SIZE OnCalculateSize (CDC* pDC, const CSize& sizeDefault, BOOL bHorz);
	virtual BOOL ExportToMenuButton (CBCGPToolbarMenuButton& menuButton) const;

	void SetDefaultCommand (UINT uiCmd);
	BOOL DropDownToolbar (CWnd* pWnd);

	virtual BOOL IsExtraSize () const
	{
		return m_bExtraSize && !m_bInternalDraw;
	}

	virtual BOOL OnCustomizeMenu (CMenu* pMenu);

// Attributes:
public:
	BOOL IsDropDown () const
	{
		return m_pPopupMenu != NULL;
	}

	CBCGPToolBar* GetDropDownToolBar () const
	{
		return m_pToolBar;
	}

	static UINT m_uiShowBarDelay;

protected:
	CBCGPDropDownToolBar*	m_pToolBar;
	CString					m_strName;
	CBCGPDropDownFrame*		m_pPopupMenu;

	UINT					m_uiTimer;
	int						m_iSelectedImage;
	CString					m_strSelectedText;
	BOOL					m_bInternalDraw;
	BOOL					m_bLocalUserButton;


// diagnostics:

public:

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DROPDOWN_TOOLBAR)
