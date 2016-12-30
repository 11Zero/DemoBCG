#if !defined(AFX_BCGPBASETABBEDBAR_H__63755164_ADEE_4730_8955_4BC65BCCEC79__INCLUDED_)
#define AFX_BCGPBASETABBEDBAR_H__63755164_ADEE_4730_8955_4BC65BCCEC79__INCLUDED_

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
// BCGPBaseTabbedBar.h : header file
//

#ifndef __AFXTEMPL_H__
	#include "afxtempl.h"
#endif

#include "BCGCBPro.h"
#include "BCGPDockingControlBar.h"
#include "BCGPBaseTabWnd.h"

/////////////////////////////////////////////////////////////////////////////
// CBCGPBaseTabbedBar window

class BCGCBPRODLLEXPORT CBCGPBaseTabbedBar : public CBCGPDockingControlBar
{
	DECLARE_DYNAMIC(CBCGPBaseTabbedBar)
	
// Construction
public:
	CBCGPBaseTabbedBar(BOOL bAutoDestroy = FALSE);

// Attributes
public:
	virtual CBCGPBaseTabWnd* GetUnderlinedWindow ()
	{
		return m_pTabWnd;
	}

	virtual void GetTabArea (CRect& rectTabAreaTop, CRect& rectTabAreaBottom) const = 0;
	virtual HICON GetBarIcon (BOOL bBigIcon);

	void SetAutoDestroy (BOOL bAutoDestroy = TRUE)
	{
		// should be called after dynamic creation (by CreateObject)
		m_bAutoDestroy = bAutoDestroy;
	}

	virtual BOOL HasAutoHideMode () const
	{
		return FALSE;
	}

	virtual int GetTabsNum () const
	{
		if (m_pTabWnd != NULL)
		{
			return m_pTabWnd->GetTabsNum ();
		}

		return 0;
	}

	virtual int GetVisibleTabsNum () const
	{
		if (m_pTabWnd != NULL)
		{
			return m_pTabWnd->GetVisibleTabsNum ();
		}

		return 0;
	}

	virtual BOOL IsHideSingleTab () const
	{
		if (m_pTabWnd != NULL)
		{
			return m_pTabWnd->IsHideSingleTab ();
		}

		return FALSE;
	}

	virtual BOOL CanFloat () const;
	virtual BOOL AllowDestroyEmptyTabbedBar () const {return TRUE;}

	const CArray<int,int>& GetDefaultTabsOrder ()
	{
		if (m_arDefaultTabsOrder.GetSize () == 0)
		{
			FillDefaultTabsOrderArray ();
		}

		return m_arDefaultTabsOrder;
	}

	virtual void GetMinSize (CSize& size) const;

	virtual BOOL IsRestoreTabsState() const
	{
		return TRUE;
	}
	
// Operations
public:
	virtual BOOL AddTab (CWnd* pNewBar, BOOL bVisible = TRUE, BOOL bSetActive = TRUE, 
						 BOOL bDetachable = TRUE);
	
	virtual BOOL FloatTab (CWnd* pBar, int nTabID, BCGP_DOCK_METHOD dockMethod, 
						   BOOL bHide = FALSE);
	virtual BOOL DetachControlBar (CWnd* pBar, BOOL bHide = FALSE);
	virtual BOOL RemoveControlBar (CWnd* pBar);
	virtual BOOL ShowTab (CWnd* pBar, BOOL bShow, BOOL bDelay, BOOL bActivate);

	virtual CWnd* FindBarByID (UINT uBarID);
	virtual CWnd* FindBarByTabNumber (int nTabNum, BOOL bGetWrappedBar = FALSE);

	virtual CBCGPAutoHideToolBar* SetAutoHideMode (BOOL bMode, DWORD dwAlignment, 
										CBCGPAutoHideToolBar* pCurrAutoHideBar = NULL,
										BOOL bUseTimer = TRUE);


	virtual CWnd* GetFirstVisibleTab (int& iTabNum);
	virtual void RecalcLayout ();

	virtual void ApplyRestoredTabInfo (BOOL bUseTabIndexes = FALSE);
	virtual void Serialize (CArchive& ar);
	virtual void SerializeTabWindow (CArchive& ar);
	virtual void SaveSiblingBarIDs (CArchive& ar);

	virtual void EnableSetCaptionTextToTabName (BOOL bEnable)
	{
		m_bSetCaptionTextToTabName = bEnable;
	}

	virtual BOOL CanSetCaptionTextToTabName () const
	{
		return m_bSetCaptionTextToTabName;
	}

	virtual void GetControlBarList (CObList& lst, CRuntimeClass* pRTCFilter = NULL);
	virtual void ConvertToTabbedDocument (BOOL bActiveTabOnly = TRUE);

	// it should be static because it may be called before the tab bar has been created
	static  void LoadSiblingBarIDs (CArchive& ar, CList<UINT, UINT>& lstBarIDs);
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPBaseTabbedBar)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBCGPBaseTabbedBar();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPBaseTabbedBar)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnNcDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	//}}AFX_MSG
	afx_msg LRESULT OnChangeActiveTab(WPARAM,LPARAM);
	DECLARE_MESSAGE_MAP()

protected:
	virtual BOOL FloatControlBar	(CRect rectFloat, BCGP_DOCK_METHOD dockMethod = BCGP_DM_UNKNOWN, 
									 bool bShow = true);

	virtual BOOL SaveState (LPCTSTR lpszProfileName, int nIndex = -1, UINT uiID = (UINT) -1);
	virtual BOOL LoadState (LPCTSTR lpszProfileName = NULL, int nIndex = -1, UINT uiID = (UINT) -1);

	virtual void StoreRecentDockInfo ();

	virtual BOOL Dock (CBCGPBaseControlBar* pTargetBar, LPCRECT lpRect, BCGP_DOCK_METHOD dockMethod);

	virtual void OnActivateTab (int /*iTabNum*/) {}
	virtual void OnChangeActiveState();

	void FillDefaultTabsOrderArray ();

protected:
	BOOL					m_bAutoDestroy;
	CBCGPBaseTabWnd*		m_pTabWnd;
	BOOL					m_bSetCaptionTextToTabName;
	CArray<int,int>			m_arDefaultTabsOrder;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPBASETABBEDBAR_H__63755164_ADEE_4730_8955_4BC65BCCEC79__INCLUDED_)
