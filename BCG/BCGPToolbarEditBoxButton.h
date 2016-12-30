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

// BCGPToolbarEditBoxButton.h: interface for the CBCGPToolbarEditBoxButton class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPTOOLBAREDITBOXBUTTON_H__188F6EA4_E9D5_4B16_A58B_5B4D9B517AC6__INCLUDED_)
#define AFX_BCGPTOOLBAREDITBOXBUTTON_H__188F6EA4_E9D5_4B16_A58B_5B4D9B517AC6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"
#include "BCGPToolbarButton.h"
#include "BCGPEdit.h"

class BCGCBPRODLLEXPORT CBCGPToolbarEditBoxButton : public CBCGPToolbarButton  
{
	friend class CBCGPToolbarEditCtrl;

	DECLARE_SERIAL(CBCGPToolbarEditBoxButton)

public:
	CBCGPToolbarEditBoxButton();
	CBCGPToolbarEditBoxButton(UINT uiID, int iImage, DWORD dwStyle = ES_AUTOHSCROLL, int iWidth = 0);
	virtual ~CBCGPToolbarEditBoxButton();

// Operations:

// Overrides:
	virtual CEdit* CreateEdit (CWnd* pWndParent, const CRect& rect);

	virtual void OnDraw (CDC* pDC, const CRect& rect, CBCGPToolBarImages* pImages,
						BOOL bHorz = TRUE, BOOL bCustomizeMode = FALSE,
						BOOL bHighlight = FALSE,
						BOOL bDrawBorder = TRUE,
						BOOL bGrayDisabledButtons = TRUE);
	virtual void CopyFrom (const CBCGPToolbarButton& src);
	virtual void Serialize (CArchive& ar);
	virtual SIZE OnCalculateSize (CDC* pDC, const CSize& sizeDefault, BOOL bHorz);
	virtual BOOL OnClick (CWnd* pWnd, BOOL bDelay = TRUE);
	virtual void OnChangeParentWnd (CWnd* pWndParent);
	virtual void OnMove ();
	virtual void OnSize (int iSize);
	virtual HWND GetHwnd ()
	{	
		return m_pWndEdit->GetSafeHwnd ();
	}
	virtual BOOL NotifyCommand (int iNotifyCode);
	
	virtual BOOL CanBeStretched () const
	{	
		return TRUE;	
	}
	virtual void OnAddToCustomizePage ();
	virtual HBRUSH OnCtlColor(CDC* pDC, UINT nCtlColor);
	virtual BOOL HaveHotBorder () const
	{
		return m_pWndEdit->GetSafeHwnd () == NULL ||
			(m_pWndEdit->GetStyle () & WS_VISIBLE) == 0;
	}

	virtual int OnDrawOnCustomizeList (
			CDC* pDC, const CRect& rect, BOOL bSelected);

	virtual void OnShow (BOOL bShow);
	virtual void SetContents(const CString& sContents);

	virtual const CRect GetInvalidateRect () const;
	virtual void SetStyle (UINT nStyle);

	virtual void GetEditBorder (CRect& rectBorder);

	virtual BOOL OnUpdateToolTip (CWnd* pWndParent, int iButtonIndex,
		CToolTipCtrl& wndToolTip, CString& str);

	virtual void OnGlobalFontsChanged();

	virtual BOOL SetACCData (CWnd* pParent, CBCGPAccessibilityData& data);

	static void SetFlatMode (BOOL bFlat = TRUE)
	{
		m_bFlat = bFlat;
	}

	static BOOL IsFlatMode ()
	{
		return m_bFlat;
	}

	virtual BOOL IsAlwaysOpaque () const
	{
		return TRUE;
	}

protected:
	void Initialize ();
	void SetHotEdit (BOOL bHot);

	virtual void OnShowEditbox (BOOL /*bShow*/)	{}

// Attributes:
public:
	CEdit* GetEditBox () const
	{
		return m_pWndEdit;
	}

	void SetContextMenuID(UINT uiResID)
	{
		m_uiMenuResID = uiResID;
	}

	UINT GetContextMenuID()
	{
		return m_uiMenuResID;
	}

	void EnableCalculator (BOOL bEnable = TRUE)
	{
		m_bCalculator = bEnable;
	}

	BOOL IsCalculator () const
	{
		return m_bCalculator;
	}

	void SetPrompt(LPCTSTR lpszPrompt, COLORREF clrPrompt = (COLORREF)-1)
	{
		m_strPrompt = lpszPrompt == NULL ? _T("") : lpszPrompt;
		m_clrPrompt = clrPrompt;
	}

	const CString& GetPrompt() const
	{
		return m_strPrompt;
	}

	COLORREF GetPromptColor() const
	{
		return m_clrPrompt;
	}

	static CBCGPToolbarEditBoxButton* GetByCmd (UINT uiCmd);
	static BOOL SetContentsAll (UINT uiCmd, const CString& strContents);
	static CString GetContentsAll (UINT uiCmd);

protected:
	DWORD				m_dwStyle;
	CEdit*				m_pWndEdit;
	CString				m_strContents;
	BOOL				m_bChangingText;

	int					m_iWidth;

	BOOL				m_bHorz;
	BOOL				m_bIsHotEdit;

	static BOOL			m_bFlat;

	UINT				m_uiMenuResID;
	BOOL				m_bCalculator;
	CString				m_strPrompt;
	COLORREF			m_clrPrompt;
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPToolbarEditCtrl

class BCGCBPRODLLEXPORT CBCGPToolbarEditCtrl : public CBCGPEdit
{
// Construction
public:
	CBCGPToolbarEditCtrl(CBCGPToolbarEditBoxButton& edit);

// Attributes
protected:
	CBCGPToolbarEditBoxButton&	m_buttonEdit;
	BOOL						m_bTracked;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPToolbarEditCtrl)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBCGPToolbarEditCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPToolbarEditCtrl)
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	//}}AFX_MSG
	afx_msg LRESULT OnMouseLeave(WPARAM,LPARAM);
	DECLARE_MESSAGE_MAP()
};

#endif // !defined(AFX_BCGPTOOLBAREDITBOXBUTTON_H__188F6EA4_E9D5_4B16_A58B_5B4D9B517AC6__INCLUDED_)
