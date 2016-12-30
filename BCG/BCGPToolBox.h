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
//
// BCGPToolBox.h: interface for the CBCGPToolBox and CBCGPToolBox classes.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPTOOLBOX_H__58F892DC_E5CC_4A7F_84A5_DCE55D58509B__INCLUDED_)
#define AFX_BCGPTOOLBOX_H__58F892DC_E5CC_4A7F_84A5_DCE55D58509B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BCGPToolBox.h : header file
//

#include "BCGCBPro.h"

#ifndef BCGP_EXCLUDE_TOOLBOX

#if (!defined _BCGSUITE_) && !defined (_BCGSUITE_INC_)

#include "BCGPDockingControlBar.h"
#include "BCGPOutlookBar.h"
#include "BCGPButton.h"
#include "BCGPTasksPane.h"
#include "BCGPToolBarImages.h"

#endif

/////////////////////////////////////////////////////////////////////////////
// CBCGPToolBoxButton

class CBCGPToolBox;
class CBCGPToolBoxEx;
class CBCGPToolBoxPage;

class BCGCBPRODLLEXPORT CBCGPToolBoxButton : public CObject
{
	DECLARE_DYNCREATE(CBCGPToolBoxButton)

	CBCGPToolBoxButton ();

	virtual void OnFillBackground (CDC* pDC, const CRect& rectClient);
	virtual void OnDrawBorder (CDC* pDC, CRect& rectClient, UINT uiState);

	virtual void OnDraw (CDC* pDC);

	CString				m_strLabel;
	int					m_iImageIndex;
	BOOL				m_bIsHighlighted;
	BOOL				m_bIsChecked;
	CRect				m_Rect;
	CBCGPToolBarImages*	m_pImages;
	CBCGPToolBoxPage*	m_pPage;
	int					m_nID;

public:
	void Highlight (BOOL bSet = TRUE)
	{
		m_bIsHighlighted = bSet;
	}

	BOOL IsHighlighted () const
	{
		return m_bIsHighlighted;
	}

	void SetCheck (BOOL bCheck = TRUE)
	{
		m_bIsChecked = bCheck;
	}

	BOOL GetCheck () const
	{
		return m_bIsChecked;
	}

	void SetLabel (LPCTSTR lpszLabel)
	{
		m_strLabel = lpszLabel == NULL ? _T("") : lpszLabel;
	}

	LPCTSTR GetLabel () const
	{
		return m_strLabel;
	}

	int GetImageIndex () const
	{
		return m_iImageIndex;
	}

	void SetImageIndex (int iImage)
	{
		m_iImageIndex = iImage;
	}

	void SetImageList (CBCGPToolBarImages* pImages)
	{
		m_pImages = pImages;
	}

	CBCGPToolBarImages* GetImageList () const
	{
		return m_pImages;
	}

	const CRect& GetRect () const
	{
		return m_Rect;
	}

	void SetRect (CRect rect)
	{
		m_Rect = rect;
	}

	void Hide ()
	{
		m_Rect.SetRectEmpty ();
	}

	void SetParentPage (CBCGPToolBoxPage* pPage)
	{
		m_pPage = pPage;
	}

	int GetID () const
	{
		return m_nID;
	}

	void SetID (int nID)
	{
		m_nID = nID;
	}
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPToolBoxPage window

class BCGCBPRODLLEXPORT CBCGPToolBoxPage : public CBCGPControlBar  
{
	friend class CBCGPToolBox;
	friend class CBCGPToolBoxEx;

	DECLARE_DYNCREATE(CBCGPToolBoxPage)

// Construction
public:
	CBCGPToolBoxPage();

// Attributes
public:
	enum ToolBoxPageMode
	{
		ToolBoxPageMode_Default,
		ToolBoxPageMode_Images,
	};

	BOOL SetSelected (int nItem);
	int GetSelected () const
	{
		return m_nCheckedButton;
	}

	ToolBoxPageMode GetMode () const
	{
		return m_Mode;
	}

	void SetMode (ToolBoxPageMode mode);

	CBCGPToolBarImages& GetImageList ()
	{
		return m_Images;
	}

// Operations:
public:
	int AddButton (LPCTSTR lpszText, HICON hIcon);
	BOOL DeleteButton (int nItem);
	void DeleteAllButons ();

	int GetButtonCount () const
	{
		return (int) m_arButtons.GetSize ();
	}

	CBCGPToolBoxButton* GetButton (int nItem);
	CBCGPToolBoxButton* GetButtonByID (int nID);

	BOOL UpdateImageList(UINT nResID);

protected:
	CArray<CBCGPToolBoxButton*, CBCGPToolBoxButton*>	m_arButtons;
	CSize								m_sizeImage;
	int									m_nCheckedButton;
	int									m_nHighlightedButton;
	CBCGPToolBox*						m_pToolBox;
	CBCGPToolBoxEx*						m_pToolBoxEx;
	int									m_nVertScrollOffset;
	int									m_nVertScrollSize;
	int									m_nScrollValue;
	BOOL								m_bCheckFirstButton;
	CBCGPToolBarImages					m_Images;
	UINT								m_uiBmpResID;
	ToolBoxPageMode						m_Mode;
	CToolTipCtrl*						m_pToolTip;
	CRuntimeClass*						m_pButtonClass;

protected:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPToolBoxPage)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

	virtual BOOL IsAccessibilityCompatible () { return TRUE; }

	virtual HRESULT get_accParent(IDispatch **ppdispParent);
	virtual HRESULT get_accChild(VARIANT varChild, IDispatch **ppdispChild);
	virtual HRESULT get_accChildCount(long *pcountChildren);
	virtual HRESULT get_accName(VARIANT varChild, BSTR *pszName);
	virtual HRESULT get_accDescription(VARIANT varChild, BSTR *pszDescription);
	virtual HRESULT get_accRole(VARIANT varChild, VARIANT *pvarRole);
	virtual HRESULT get_accState(VARIANT varChild, VARIANT *pvarState);
	virtual HRESULT get_accDefaultAction(VARIANT varChild, BSTR *pszDefaultAction);
	virtual HRESULT accSelect(long flagsSelect, VARIANT varChild);
	virtual HRESULT accLocation(long *pxLeft, long *pyTop, long *pcxWidth, long *pcyHeight, VARIANT varChild);
	virtual HRESULT accHitTest(long xLeft, long yTop, VARIANT *pvarChild);
	virtual HRESULT accDoDefaultAction(VARIANT varChild);

// Implementation
public:
	virtual ~CBCGPToolBoxPage();
	void SetScrollValue (int nScrollValue);

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPToolBoxPage)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnNcDestroy();
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnDestroy();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnSysColorChange();
	//}}AFX_MSG
	afx_msg BOOL OnNeedTipText(UINT id, NMHDR* pNMH, LRESULT* pResult);
	afx_msg LRESULT OnBCGUpdateToolTips (WPARAM, LPARAM);
	DECLARE_MESSAGE_MAP()

	virtual void DoPaint(CDC* pDCPaint);
	virtual int HitTest(CPoint point);
	virtual BOOL OnShowControlBarMenu (CPoint /*point*/)	{	return FALSE;	}
	virtual BOOL AllowShowOnControlMenu () const			{	return FALSE;	}
	virtual void AdjustLayout ();
	virtual BOOL OnGestureEventPan(const CPoint& ptFrom, const CPoint& ptTo, CSize& sizeOverPan);
	virtual BOOL OnClickButton(int nButtonIndex);

	void ReposButtons ();

	void RedrawButton (int nButton);
	void HighlightButton (int nButton);

	int GetMaxHeight () const;

	BOOL InitPage (UINT uiBmpResID, int nImageWidth, const CStringList& lstLabels, CRuntimeClass* pButtonClass);

	CBCGPToolBoxButton* GetAccChild(int nIndex);
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPToolBoxBar window

class CBCGPToolBoxBar : public CBCGPOutlookBar
{
	virtual void OnActivateTab (int iTabNum);
	virtual void OnScroll (BOOL bDown);

	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPToolBox window

class BCGCBPRODLLEXPORT CBCGPToolBox : public CBCGPDockingControlBar
{
	DECLARE_DYNCREATE(CBCGPToolBox)

// Construction
public:
	CBCGPToolBox();

// Attributes
public:
	int GetActivePage () const;
	int GetLastClickedTool (int nPage) const;
	int GetPageNumber (CBCGPToolBoxPage* pPage) const;

	CBCGPToolBoxPage* GetPage (int nPage) const;

	CBCGPBaseTabWnd* GetTabWnd () const;

protected:
	CBCGPToolBoxBar m_wndOutlook;

// Operations
public:
	BOOL AddToolsPage (LPCTSTR lpszPageName, UINT uiBmpResID, int nImageWidth,
		const CStringList& lstLabels,
		CRuntimeClass* pPageClass = RUNTIME_CLASS (CBCGPToolBoxPage),
		CRuntimeClass* pButtonClass = RUNTIME_CLASS (CBCGPToolBoxButton));

	BOOL AddToolsPage (LPCTSTR lpszPageName, UINT uiBmpResID, int nImageWidth,
		LPCTSTR lpszLabels,	// Devided by '\n'
		CRuntimeClass* pPageClass = RUNTIME_CLASS (CBCGPToolBoxPage),
		CRuntimeClass* pButtonClass = RUNTIME_CLASS (CBCGPToolBoxButton));

// Overrides
public:
	virtual void OnClickTool (int nPage, int nIndex);
	virtual void OnActivatePage (int nPage);
	virtual BOOL OnShowToolboxMenu (CPoint point, CBCGPToolBoxPage* pPage, int nHit);

	virtual void OnScroll (BOOL bDown);

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPToolBox)
	//}}AFX_VIRTUAL
	virtual void AdjustLayout ();

// Implementation
public:
	virtual ~CBCGPToolBox();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPToolBox)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	virtual BOOL IsAccessibilityCompatible () { return TRUE; }
	virtual HRESULT get_accChildCount(long *pcountChildren);
	virtual HRESULT get_accChild(VARIANT varChild, IDispatch **ppdispChild);
	virtual HRESULT accHitTest(long xLeft, long yTop, VARIANT *pvarChild);
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPToolBox window

#ifndef BCGP_EXCLUDE_TASK_PANE

class BCGCBPRODLLEXPORT CBCGPToolBoxEx : public CBCGPTasksPane
{
	DECLARE_DYNCREATE(CBCGPToolBoxEx)

// Construction
public:
	CBCGPToolBoxEx();

// Attributes
public:
	int GetLastClickedTool (int& nClickedPage) const;
	int GetPageNumber (CBCGPToolBoxPage* pPage) const;

	int GetPageCount () const;
	CBCGPToolBoxPage* GetPage (int nPage) const;

// Operations
public:
	BOOL AddToolsPage (LPCTSTR lpszPageName, UINT uiBmpResID, int nImageWidth,
		const CStringList& lstLabels,
		CRuntimeClass* pPageClass = RUNTIME_CLASS (CBCGPToolBoxPage),
		CRuntimeClass* pButtonClass = RUNTIME_CLASS (CBCGPToolBoxButton));

	BOOL AddToolsPage (LPCTSTR lpszPageName, UINT uiBmpResID, int nImageWidth,
		LPCTSTR lpszLabels,	// Devided by '\n'
		CRuntimeClass* pPageClass = RUNTIME_CLASS (CBCGPToolBoxPage),
		CRuntimeClass* pButtonClass = RUNTIME_CLASS (CBCGPToolBoxButton));

	BOOL RemoveToolsPage (int nPage);
	void RemoveAllToolsPages ();

// Overrides
public:
	virtual void OnClickTool (int nPage, int nIndex);
	virtual BOOL OnShowToolboxMenu (CPoint point, CBCGPToolBoxPage* pPage, int nHit);

#if (!defined _BCGSUITE_) && !defined (_BCGSUITE_INC_)
	virtual BCGP_DOCK_TYPE GetDockMode () const 
	{
		return CBCGPBaseControlBar::GetDockMode ();
	}
#endif

	virtual void OnFillBackground (CDC* pDC, CRect rectFill);

	virtual BOOL IsToolBox () const		{	return TRUE;	}

	virtual void AdjustLayout ();

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPToolBoxEx)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBCGPToolBoxEx();

protected:
	virtual void ScrollChild (HWND hwndChild, int nScrollValue);

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPToolBoxEx)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	virtual void NotifyAccessibility (int /*nGroupNumber*/, int /*nTaskNumber*/) {}
	virtual void NotifyAccessibilityFocusEvent (BOOL /*bUseCursor*/) {}
	virtual HRESULT get_accChildCount(long *pcountChildren);
	virtual HRESULT get_accChild(VARIANT varChild, IDispatch **ppdispChild);
	virtual HRESULT accHitTest(long xLeft, long yTop, VARIANT *pvarChild);
};

#endif // BCGP_EXCLUDE_TASK_PANE
#endif // BCGP_EXCLUDE_TOOLBOX

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPTOOLBOX_H__58F892DC_E5CC_4A7F_84A5_DCE55D58509B__INCLUDED_)
