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
// BCGPRibbonPanelMenu.h : header file
//

#if !defined(AFX_BCGPRIBBONPANELMENU_H__A1D87F0A_7939_4469_B7F3_ACA148EE801B__INCLUDED_)
#define AFX_BCGPRIBBONPANELMENU_H__A1D87F0A_7939_4469_B7F3_ACA148EE801B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "bcgcbpro.h"

#ifndef BCGP_EXCLUDE_RIBBON

#include "BCGPPopupMenu.h"
#include "BCGPPopupMenuBar.h"
#include "BCGPRibbonPanel.h"
#include "BCGPScrollBar.h"

class CBCGPRibbonButton;
class CBCGPRibbonPaletteButton;
class CBCGPRibbonCategory;

/////////////////////////////////////////////////////////////////////////////
// CBCGPRibbonPanelMenuBar window

class BCGCBPRODLLEXPORT CBCGPRibbonPanelMenuBar : public CBCGPPopupMenuBar
{
	DECLARE_DYNAMIC(CBCGPRibbonPanelMenuBar)

	friend class CBCGPBaseRibbonElement;
	friend class CBCGPRibbonPanelMenu;
	friend class CBCGPRibbonFloaty;
	friend class CBCGPRibbonQuickAccessCustomizeButton;
	friend class CBCGPRibbonBackstageViewPanel;
	friend class CBCGPRibbonCategory;

// Construction
public:
	CBCGPRibbonPanelMenuBar(CBCGPRibbonPanel* pPanel);

	CBCGPRibbonPanelMenuBar(CBCGPRibbonBar* pRibbonBar,
		const CArray<CBCGPBaseRibbonElement*, CBCGPBaseRibbonElement*>&	arButtons, BOOL bIsFloatyMode = FALSE);

	CBCGPRibbonPanelMenuBar(CBCGPRibbonPaletteButton* pPaletteButton);

	CBCGPRibbonPanelMenuBar (CBCGPRibbonCategory* pCategory, CSize size);

protected:
	CBCGPRibbonPanelMenuBar();
	void CommonInit ();

// Attributes
public:
	CBCGPRibbonBar*	GetTopLevelRibbonBar () const;
	CBCGPRibbonPanel* GetPanel ()
	{
		return m_pPanel;
	}

	CBCGPRibbonCategory* GetCategory ()
	{
		return m_pCategory;
	}

	void SetMenuMode (BOOL bIsMenuMode = TRUE)
	{
		m_bIsMenuMode = bIsMenuMode;
	}

	BOOL IsMenuMode () const
	{
		return m_bIsMenuMode;
	}

	void SetDefaultMenuLook (BOOL bIsDefaultMenuLook = TRUE)
	{
		m_bIsDefaultMenuLook = bIsDefaultMenuLook;
	}

	BOOL IsDefaultMenuLook () const
	{
		return m_bIsDefaultMenuLook;
	}

	BOOL IsMainPanel () const
	{
		return m_pPanel != NULL && m_pPanel->IsMainPanel ();
	}

	BOOL IsQATPopup () const
	{
		return m_bIsQATPopup;
	}

	BOOL IsFloaty () const
	{
		return m_bIsFloaty;
	}

	BOOL IsCategoryPopup () const
	{
		return m_pCategory != NULL;
	}

	CBCGPRibbonButton* GetDroppedDown () const;

	CBCGPBaseRibbonElement* HitTest (CPoint point) const;
	CBCGPBaseRibbonElement* GetFocused () const;
	int HitTestEx (CPoint point) const;

	virtual BOOL IsBackStageView() const
	{
		return FALSE;
	}

protected:
	CBCGPRibbonPanel*		m_pPanel;
	CBCGPRibbonCategory*	m_pCategory;
	BOOL					m_bTracked;
	CBCGPRibbonPanel*		m_pPanelOrigin;
	CBCGPRibbonBar*			m_pRibbonBar;
	BOOL					m_bSimpleMode;
	BOOL					m_bIsMenuMode;
	BOOL					m_bIsDefaultMenuLook;
	CSize					m_sizePrefered;
	CBCGPRibbonButton*		m_pDelayedCloseButton;
	CBCGPRibbonButton*		m_pDelayedButton;
	CBCGPBaseRibbonElement*	m_pPressed;
	CBCGPScrollBar			m_wndScrollBarVert;
	CBCGPScrollBar			m_wndScrollBarHorz;
	CSize					m_sizeCategory;
	BOOL					m_bIsQATPopup;
	BOOL					m_bCustomizeMenu;
	BOOL					m_bIsFloaty;
	BOOL					m_bHasKeyTips;
	BOOL					m_bSetKeyTips;
	BOOL					m_bAutoCommandTimer;
	CRect					m_rectAutoCommand;
	CPoint					m_ptStartMenu;
	BOOL					m_bIsOneRowFloaty;
	BOOL					m_bIsCtrlMode;

// Operations
public:
	void PopTooltip ();
	void SetActive (BOOL bIsActive);

	CBCGPBaseRibbonElement* FindByOrigin (CBCGPBaseRibbonElement* pOrigin) const;

// Overrides
	virtual void OnClickButton (CBCGPRibbonButton* pButton, CPoint point);
	virtual void OnChangeHighlighted (CBCGPBaseRibbonElement* pHighlighted);

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPRibbonPanelMenuBar)
	//}}AFX_VIRTUAL

	virtual void AdjustLocations ();
	virtual CSize CalcSize (BOOL bVertDock);
	virtual void DoPaint(CDC* pDC);

	virtual void OnUpdateCmdUI (CFrameWnd* pTarget, BOOL bDisableIfNoHndler);
	virtual void CloseDelayedSubMenu ();
	virtual void RestoreDelayedSubMenu ();

	virtual BOOL OnKey (UINT nChar);

	virtual BOOL IsRibbonPanel () const
	{
		return TRUE;
	}

	virtual BOOL IsRibbonPanelInRegularMode () const
	{
		return !m_bIsMenuMode && !IsMainPanel ();
	}

	virtual BOOL OnSetAccData (long lVal);
	virtual HRESULT get_accChildCount(long *pcountChildren);
	virtual HRESULT accHitTest(long xLeft, long yTop, VARIANT *pvarChild);

	void StopWaitForPopup();

protected:
	virtual void OnDrawMenuBorder (CDC* pDC);
	virtual BOOL PreTranslateMessage(MSG* pMsg);

// Implementation
public:
	virtual ~CBCGPRibbonPanelMenuBar();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPRibbonPanelMenuBar)
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	//}}AFX_MSG
	afx_msg LRESULT OnMouseLeave(WPARAM,LPARAM);
	afx_msg LRESULT OnBCGUpdateToolTips (WPARAM, LPARAM);
	afx_msg BOOL OnNeedTipText(UINT id, NMHDR* pNMH, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()

	void AddButtons (CBCGPRibbonBar* pRibbonBar,
		const CArray<CBCGPBaseRibbonElement*, CBCGPBaseRibbonElement*>&	arButtons,
		BOOL bFloatyMode);

	void SetPreferedSize (CSize size);
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPRibbonPanelMenu window

class BCGCBPRODLLEXPORT CBCGPRibbonPanelMenu : public CBCGPPopupMenu
{
	friend class CBCGPRibbonPanelMenuBar;
	friend class CBCGPRibbonPanel;

	DECLARE_DYNAMIC(CBCGPRibbonPanelMenu)

// Construction
public:
	CBCGPRibbonPanelMenu (CBCGPRibbonPanel* pPanel);
	
	CBCGPRibbonPanelMenu (
		CBCGPRibbonBar* pRibbonBar,
		const CArray<CBCGPBaseRibbonElement*, CBCGPBaseRibbonElement*>&	arButtons,
		BOOL bIsFloatyMode = FALSE);

	CBCGPRibbonPanelMenu (
		CBCGPRibbonPaletteButton* pPaletteButton);

	CBCGPRibbonPanelMenu (
		CBCGPRibbonCategory* pCategory, CSize size);

protected:
	CBCGPRibbonPanelMenu ();

// Attributes
public:
	void SetMenuMode (BOOL bIsMenuMode = TRUE)
	{
		m_wndRibbonBar.SetMenuMode (bIsMenuMode);
	}

	BOOL IsMenuMode () const
	{
		return m_wndRibbonBar.IsMenuMode ();
	}

	void SetDefaultMenuLook (BOOL bIsDefaultMenuLook = TRUE)
	{
		m_wndRibbonBar.SetDefaultMenuLook (bIsDefaultMenuLook);
	}

	BOOL IsDefaultMenuLook () const
	{
		return m_wndRibbonBar.IsDefaultMenuLook ();
	}

	CBCGPRibbonPanel* GetPanel () const
	{
		return m_wndRibbonBar.m_pPanel;
	}

	CBCGPRibbonCategory* GetCategory () const
	{
		return m_wndRibbonBar.m_pCategory;
	}

	void SetPreferedSize (CSize size)
	{
		m_wndRibbonBar.SetPreferedSize (size);
	}

	CBCGPRibbonPanelMenuBar* GetParentRibbonMenuBar () const;

	void EnableCustomizeMenu (BOOL bEnable = TRUE)
	{
		m_wndRibbonBar.m_bCustomizeMenu = bEnable;
	}

	BOOL IsCustomizeMenu () const
	{
		return m_wndRibbonBar.m_bCustomizeMenu;
	}

protected:
	CBCGPRibbonPanelMenuBar	m_wndRibbonBar;
	BOOL					m_bForceClose;
	BOOL					m_bInScroll;

// Operations
public:
	virtual void SetActive (BOOL /*bIsActive*/)	{}

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPRibbonPanelMenu)
	//}}AFX_VIRTUAL

	virtual CBCGPPopupMenuBar* GetMenuBar ()
	{
		return &m_wndRibbonBar;
	}

	virtual BOOL IsAlwaysClose () const;

	CBCGPBaseRibbonElement* FindByOrigin (CBCGPBaseRibbonElement* pOrigin) const
	{
		return m_wndRibbonBar.FindByOrigin (pOrigin);
	}

	virtual BOOL IsScrollUpAvailable ();
	virtual BOOL IsScrollDnAvailable ();

	virtual BOOL DefaultMouseClickOnClose () const
	{
		return m_wndRibbonBar.IsFloaty () && !m_wndRibbonBar.IsQATPopup ();
	}

protected:
	virtual void DoPaint (CDC* pPaintDC);
	virtual int GetBorderSize () const;

// Implementation
public:
	virtual ~CBCGPRibbonPanelMenu();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPRibbonPanelMenu)
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif // BCGP_EXCLUDE_RIBBON

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPRIBBONPANELMENU_H__A1D87F0A_7939_4469_B7F3_ACA148EE801B__INCLUDED_)
