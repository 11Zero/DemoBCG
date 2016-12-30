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

#if !defined(AFX_BCGPMAINCLIENTAREAWND_H__08B9EC05_DCE3_11D1_A64F_00A0C93A70EC__INCLUDED_)
#define AFX_BCGPMAINCLIENTAREAWND_H__08B9EC05_DCE3_11D1_A64F_00A0C93A70EC__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// BCGMainClientAreaWnd.h : header file
//

#ifndef __AFXTEMPL_H__
	#include "afxtempl.h"
#endif

#include "BCGCBPro.h"
#include "BCGPTabWnd.h"

class CBCGPMDIFrameWnd;

extern BCGCBPRODLLEXPORT UINT BCGM_ON_MOVETOTABGROUP;

/////////////////////////////////////////////////////////////////////////////
// CBCGPMDITabParams

class BCGCBPRODLLEXPORT CBCGPMDITabParams
{
public:
	CBCGPMDITabParams ();
	void Serialize (CArchive& ar);

	CBCGPTabWnd::Location			m_tabLocation;
	CBCGPTabWnd::Style				m_style;
	CBCGPTabWnd::TabCloseButtonMode	m_closeButtonMode;

	BOOL							m_bTabIcons;
	BOOL							m_bTabCloseButton;
	BOOL							m_bTabCustomTooltips;
	BOOL							m_bAutoColor;
	BOOL							m_bDocumentMenu;
	BOOL							m_bEnableTabSwap;
	int								m_nTabBorderSize;
	BOOL							m_bFlatFrame;
	BOOL							m_bActiveTabCloseButton;	// Use m_closeButtonMode member instead
	BOOL							m_bReuseRemovedTabGroups;
	BOOL							m_bActiveTabBoldFont;
	BOOL							m_bTabsCaptionFont;
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPMainClientAreaWnd window

class BCGCBPRODLLEXPORT CBCGPMainClientAreaWnd : public CWnd
{
	DECLARE_DYNAMIC(CBCGPMainClientAreaWnd)
	friend class CBCGPMDIFrameWnd;

// Construction
public:
	CBCGPMainClientAreaWnd();

// Attributes
public:
	CBCGPTabWnd& GetMDITabs ()
	{
		return m_wndTab;
	}

protected:
	CBCGPTabWnd					m_wndTab;
	BOOL						m_bTabIsVisible;
	BOOL						m_bTabIsEnabled;
	BOOL						m_bLastActiveTab;

	CImageList					m_TabIcons;
	CMap<HICON,HICON,int,int>	m_mapIcons;	// Icons already loaded into the image list

	// ---- MDITabGroup+
	enum GROUP_ALIGNMENT
	{
		GROUP_NO_ALIGN,
		GROUP_VERT_ALIGN,
		GROUP_HORZ_ALIGN
	};

	CBCGPMDITabParams			m_mdiTabParams;
	CObList						m_lstTabbedGroups;
	
	CMap<CWnd*, CWnd*, CImageList*, CImageList*> m_mapTabIcons;
	BOOL						m_bIsMDITabbedGroup;
	CObList						m_lstRemovedTabbedGroups;
	
	GROUP_ALIGNMENT				m_groupAlignment;
	int							m_nResizeMargin;
	CRect						m_rectNewTabGroup;
	BOOL						m_bNewVericalGroup;
	int							m_nNewGroupMargin;
	BOOL						m_bDisableUpdateTabs;
	int							m_nTotalResizeRest;
	CStringList					m_lstLoadedTabDocuments;

	BOOL						m_bInsideDragComplete;
	// ---- MDITabGroup-

	BOOL						m_bActive;


// Operations
public:
	void EnableMDITabs (BOOL bEnable, const CBCGPMDITabParams& params);

	BOOL DoesMDITabExist () const
	{
		return m_bTabIsEnabled;
	}

	void SetActiveTab (HWND hwnd);
	void UpdateTabs (BOOL bSetActiveTabVisible = FALSE);

	void EnableMDITabsLastActiveActivation(BOOL bLastActiveTab = TRUE);

	// ---- MDITabGroup+
	void EnableMDITabbedGroups (BOOL bEnable, const CBCGPMDITabParams& mdiTabParams);
	BOOL IsMDITabbedGroup () const {return m_bIsMDITabbedGroup;}

	virtual CBCGPTabWnd* CreateTabGroup		(CBCGPTabWnd* pWndTab);
	void UpdateMDITabbedGroups				(BOOL bSetActiveTabVisible);
	void CalcWindowRectForMDITabbedGroups	(LPRECT lpClientRect, UINT nAdjustType);
	
	DWORD GetMDITabsContextMenuAllowedItems ();

	BOOL			IsMemberOfMDITabGroup (CWnd* pWnd);
	CBCGPTabWnd*	FindActiveTabWndByActiveChild ();
	CBCGPTabWnd*	FindActiveTabWnd ();
	CBCGPTabWnd*	GetFirstTabWnd ();
	CBCGPTabWnd*	FindTabWndByChild(HWND hWndChild, int& iIndex);
	const CObList&	GetMDITabGroups () const {return m_lstTabbedGroups;}

	void MDITabMoveToNextGroup (BOOL bNext = TRUE);
	void MDITabNewGroup (BOOL bVert = TRUE);
	BOOL MoveWindowToTabGroup (CBCGPTabWnd* pTabWndFrom, CBCGPTabWnd* pTabWndTo, int nIdxFrom = -1);

	void RemoveTabGroup (CBCGPTabWnd* pTabWnd, BOOL bRecalcLayout = TRUE);
	void CloseAllWindows (CBCGPTabWnd* pTabWnd);

	BOOL SaveState (LPCTSTR lpszProfileName, UINT nFrameID);
	BOOL LoadState (LPCTSTR lpszProfileName, UINT nFrameID);
	void Serialize (CArchive& ar);
	// ---- MDITabGroup-

	// Win7 taskbar interaction
	void SetTaskbarTabOrder();
	CBCGPMDIChildWnd* FindNextRegisteredWithTaskbarMDIChild(CBCGPTabWnd* pTabCtrl, int iStartFrom = 0);
	CBCGPMDIChildWnd* FindNextRegisteredWithTaskbarMDIChild(CBCGPMDIChildWnd* pOrgWnd);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPMainClientAreaWnd)
	protected:
	virtual void CalcWindowRect(LPRECT lpClientRect, UINT nAdjustType = adjustBorder);
	virtual void PreSubclassWindow();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBCGPMainClientAreaWnd();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPMainClientAreaWnd)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnStyleChanging (int nStyleType, LPSTYLESTRUCT lpStyleStruct);
	//}}AFX_MSG
	afx_msg LRESULT OnSetMenu (WPARAM wp, LPARAM);
	afx_msg LRESULT OnUpdateTabs (WPARAM, LPARAM);
	afx_msg LRESULT OnMDIRefreshMenu (WPARAM wp, LPARAM);
	afx_msg LRESULT OnMDIDestroy(WPARAM wp, LPARAM);
	afx_msg LRESULT OnMDINext(WPARAM wp, LPARAM);
	afx_msg LRESULT OnGetDragBounds (WPARAM wp, LPARAM lp);
	afx_msg LRESULT OnDragComplete (WPARAM wp, LPARAM lp);
	afx_msg LRESULT OnTabGroupMouseMove (WPARAM wp, LPARAM lp);
	afx_msg LRESULT OnCancelTabMove(WPARAM wp, LPARAM lp);
	afx_msg LRESULT OnMoveTabComplete(WPARAM wp, LPARAM lp);
	afx_msg LRESULT OnActiveTabChanged(WPARAM wp, LPARAM lp);
	afx_msg LRESULT OnIdleUpdateCmdUI(WPARAM wParam, LPARAM);
	DECLARE_MESSAGE_MAP()

private:
	CBCGPTabWnd* GetNextTabWnd (CBCGPTabWnd* pOrgTabWnd, BOOL bWithoutAsserts = FALSE);
	void AdjustMDIChildren (CBCGPTabWnd* pTabWnd);
	void DrawNewGroupRect (LPCRECT rectNew, LPCRECT rectOld);
	CBCGPTabWnd* TabWndFromPoint (CPoint ptScreen);
	CBCGPTabWnd* CreateNewTabGroup (CBCGPTabWnd* pTabWndAfter, CRect rectGroup, BOOL bVertical);
	void ApplyParams (CBCGPTabWnd* pTabWnd);
	void SerializeTabGroup (CArchive& ar, CBCGPTabWnd* pTabWnd, BOOL bSetRelation = FALSE);
	void SerializeOpenChildren (CArchive& ar);

	BOOL IsKeepClientEdge ();
};



/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPMAINCLIENTAREAWND_H__08B9EC05_DCE3_11D1_A64F_00A0C93A70EC__INCLUDED_)
