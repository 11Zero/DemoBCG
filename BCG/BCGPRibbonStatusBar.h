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
// BCGPRibbonStatusBar.h : header file
//

#if !defined(AFX_BCGPRIBBONSTATUSBAR_H__D5142815_892D_48EA_807E_6259C79A1210__INCLUDED_)
#define AFX_BCGPRIBBONSTATUSBAR_H__D5142815_892D_48EA_807E_6259C79A1210__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"

#ifndef BCGP_EXCLUDE_RIBBON

#include "BCGPRibbonBar.h"

class CBCGPBaseRibbonElement;

/////////////////////////////////////////////////////////////////////////////
// CBCGPRibbonStatusBar window

class BCGCBPRODLLEXPORT CBCGPRibbonStatusBar : public CBCGPRibbonBar
{
	friend class CBCGPFrameImpl;
	friend class CBCGPRibbonBackstageView;

	DECLARE_DYNAMIC(CBCGPRibbonStatusBar)

// Construction
public:
	CBCGPRibbonStatusBar();

	BOOL Create(CWnd* pParentWnd,
			DWORD dwStyle = WS_CHILD | WS_VISIBLE | CBRS_BOTTOM,
			UINT nID = AFX_IDW_STATUS_BAR);
	BOOL CreateEx(CWnd* pParentWnd, DWORD dwCtrlStyle = 0,
		DWORD dwStyle = WS_CHILD | WS_VISIBLE | CBRS_BOTTOM,
		UINT nID = AFX_IDW_STATUS_BAR);

	virtual BOOL LoadFromXML (LPCTSTR lpszXMLResID);
	virtual BOOL LoadFromBuffer (LPCTSTR lpszXMLBuffer);

// Attributes
public:
	BOOL IsShowEmptyExtendedArea () const
	{
		return m_bShowEmptyExtArea;
	}
	void SetShowEmptyExtendedArea (BOOL bShowEmptyExtArea = TRUE);

	virtual BOOL GetExtendedArea (CRect& rect) const;

	int GetSpace () const
	{
		// Returns a free space for dynamic panes
		return m_cxFree;
	}

	BOOL IsBottomFrame () const
	{
		return m_bBottomFrame;
	}

protected:
	int		m_cxSizeBox;
	CRect	m_rectSizeBox;
	CRect	m_rectResizeBottom;
	int		m_cxFree;
	BOOL	m_bBottomFrame;
	CString	m_strInfo;
	CRect	m_rectInfo;
	BOOL	m_bShowEmptyExtArea;
	BOOL	m_bTemporaryHidden;

	CArray<CBCGPBaseRibbonElement*, CBCGPBaseRibbonElement*>	m_arElements;
	CArray<CBCGPBaseRibbonElement*, CBCGPBaseRibbonElement*>	m_arExElements;
	
	CList<CBCGPBaseRibbonElement*, CBCGPBaseRibbonElement*>		m_lstDynElements;

	CStringArray	m_arElementLabels;
	CStringArray	m_arExElementLabels;

	CArray<CBCGPBaseRibbonElement*, CBCGPBaseRibbonElement*>	m_arCustomizeItems;

// Operations
public:
	void AddElement (CBCGPBaseRibbonElement* pElement, LPCTSTR lpszLabel, BOOL bIsVisible = TRUE);
	void AddExtendedElement (CBCGPBaseRibbonElement* pElement, LPCTSTR lpszLabel, BOOL bIsVisible = TRUE);
	void AddSeparator ();

	BOOL ReplaceElementByID(UINT uiCmdID, CBCGPBaseRibbonElement& newElement, LPCTSTR lpszLabel = NULL, BOOL bCopyContent = TRUE);

	void SetInformation (LPCTSTR lpszInfo);	// lpszInfo == NULL - remove information
	BOOL IsInformationMode () const
	{
		return !m_strInfo.IsEmpty ();
	}

	void AddDynamicElement (CBCGPBaseRibbonElement* pElement);

	BOOL RemoveElement (UINT uiID);
	void RemoveAll ();

	int GetCount () const;
	int GetExCount () const;

	CBCGPBaseRibbonElement* GetElement (int nIndex);
	CBCGPBaseRibbonElement* GetExElement (int nIndex);

	CBCGPBaseRibbonElement* FindByID (UINT uiCmdID, BOOL /*bVisibleOnly*/ = TRUE)
	{
		return FindElement (uiCmdID);
	}

	CBCGPBaseRibbonElement* FindElement (UINT uiID);

	BOOL IsExtendedElement (CBCGPBaseRibbonElement* pElement) const;

	CString GetLabel (const CBCGPBaseRibbonElement* pElement) const;

// Overrides
public:
	virtual void SetInputMode(BCGP_INPUT_MODE mode);
	virtual void RecalcLayout ();

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPRibbonStatusBar)
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

	virtual CSize CalcFixedLayout(BOOL bStretch, BOOL bHorz);

	virtual BOOL IsMainRibbonBar () const					{	return FALSE;	}
	virtual BOOL IsShowGroupBorder (CBCGPRibbonButtonsGroup* /*pGroup*/) const	{	return TRUE;	}

	virtual CBCGPBaseRibbonElement* HitTest (CPoint point, 
		BOOL bCheckActiveCategory = FALSE,
		BOOL bCheckPanelCaption = FALSE);

	virtual CBCGPBaseRibbonElement* GetDroppedDown ();
	virtual void OnUpdateCmdUI (CFrameWnd* pTarget, BOOL bDisableIfNoHndler);
	virtual void OnControlBarContextMenu (CWnd* pParentFrame, CPoint point);

	virtual BOOL LoadState (LPCTSTR lpszProfileName = NULL, int nIndex = -1, UINT uiID = (UINT) -1);
	virtual BOOL SaveState (LPCTSTR lpszProfileName = NULL, int nIndex = -1, UINT uiID = (UINT) -1);

	virtual void OnRTLChanged (BOOL bIsRTL);

	virtual void OnDrawInformation (CDC* pDC, CString& strInfo, CRect rectInfo);

	virtual void GetVisibleElements (CArray<CBCGPBaseRibbonElement*, CBCGPBaseRibbonElement*>& arButtons);
	
	virtual CBCGPBaseAccessibleObject* AccessibleObjectFromPoint(CPoint point);
	virtual CBCGPBaseAccessibleObject* AccessibleObjectByIndex(long lVal);
protected:
	virtual void OnDraw(CDC* pDC);

// Implementation
public:
	virtual ~CBCGPRibbonStatusBar();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPRibbonStatusBar)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	//}}AFX_MSG
	afx_msg BCGNcHitTestType OnNcHitTest(CPoint point);
	afx_msg LRESULT OnUpdateShadows(WPARAM,LPARAM);
	DECLARE_MESSAGE_MAP()

	void CleanUpCustomizeItems ();
};

#endif // BCGP_EXCLUDE_RIBBON

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPRIBBONSTATUSBAR_H__D5142815_892D_48EA_807E_6259C79A1210__INCLUDED_)
