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

// BCGToolbarComboBoxButton.h: interface for the CBCGPToolbarComboBoxButton class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPTOOLBARCOMBOBOXBUTTON_H__D5B381B4_CC65_11D1_A648_00A0C93A70EC__INCLUDED_)
#define AFX_BCGPTOOLBARCOMBOBOXBUTTON_H__D5B381B4_CC65_11D1_A648_00A0C93A70EC__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXTEMPL_H__
	#include "afxtempl.h"
#endif

#include "BCGCBPro.h"
#include "BCGPToolbarButton.h"
#include "BCGPEdit.h"

class CBCGPToolbarMenuButton;
class CBCGPComboEdit;

class BCGCBPRODLLEXPORT CBCGPToolbarComboBoxButton : public CBCGPToolbarButton  
{
	friend class CBCGPComboEdit;
	friend class CBCGPRibbonComboBox;
	friend class CBCGPRibbonEdit;
	friend class CBCGPComboBox;
	friend class CBCGPColorButton;

	DECLARE_SERIAL(CBCGPToolbarComboBoxButton)

public:
	CBCGPToolbarComboBoxButton();
	CBCGPToolbarComboBoxButton(UINT uiID, int iImage, DWORD dwStyle = CBS_DROPDOWNLIST, int iWidth = 0);
	virtual ~CBCGPToolbarComboBoxButton();

// Operations:
	virtual INT_PTR AddItem (LPCTSTR lpszItem, DWORD_PTR dwData = 0);
	virtual INT_PTR AddSortedItem(LPCTSTR lpszItem, DWORD_PTR dwData = 0);

	INT_PTR GetCount () const;
	LPCTSTR GetItem (int iIndex = -1) const;
	DWORD_PTR GetItemData (int iIndex = -1) const;
	int GetCurSel () const
	{
		return m_iSelIndex;
	}
	void RemoveAllItems ();
	BOOL SelectItem (int iIndex, BOOL bNotify = TRUE);
	BOOL SelectItem (DWORD_PTR dwData);
	BOOL SelectItem (LPCTSTR lpszText);

	BOOL DeleteItem (int iIndex);
	BOOL DeleteItem (DWORD_PTR dwData);
	BOOL DeleteItem (LPCTSTR lpszText);

	int FindItem (LPCTSTR lpszText) const;

	void SetDropDownHeight (int nHeight);

	void SetPrompt(LPCTSTR lpszPrompt, COLORREF clrPrompt = (COLORREF)-1)
	{
		m_strPrompt = lpszPrompt == NULL ? _T("") : lpszPrompt;
		m_clrPrompt = clrPrompt;
	}
	
	COLORREF GetPromptColor() const
	{
		return m_clrPrompt;
	}
	
// Overrides:
	virtual CComboBox* CreateCombo (CWnd* pWndParent, const CRect& rect);
	virtual CBCGPComboEdit* CreateEdit (CWnd* pWndParent, const CRect& rect, DWORD dwEditStyle);

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
		return m_pWndCombo->GetSafeHwnd ();
	}

	virtual CEdit* GetEditCtrl () 
	{
		return m_pWndEdit;
	}

	virtual void EnableWindow (BOOL bEnable = TRUE)
	{
		if (m_pWndCombo->GetSafeHwnd () != NULL)
		{
			m_pWndCombo->EnableWindow (bEnable);
		}

		if (m_pWndEdit->GetSafeHwnd () != NULL)
		{
			m_pWndEdit->EnableWindow (bEnable);
		}
	}

	virtual BOOL IsWindowVisible ()
	{
		return ((m_pWndCombo->GetSafeHwnd () != NULL &&
				m_pWndCombo->GetStyle () & WS_VISIBLE) ||
				(m_pWndEdit->GetSafeHwnd () != NULL &&
				m_pWndEdit->GetStyle () & WS_VISIBLE));
	}

	virtual BOOL IsOwnerOf (HWND hwnd)
	{
		if (m_pWndCombo->GetSafeHwnd () != NULL &&
			(m_pWndCombo->GetSafeHwnd () == hwnd || 
			::IsChild (m_pWndCombo->GetSafeHwnd (), hwnd)))
		{
			return TRUE;
		}

		if (m_pWndEdit->GetSafeHwnd () != NULL &&
			(m_pWndEdit->GetSafeHwnd () == hwnd || 
			::IsChild (m_pWndEdit->GetSafeHwnd (), hwnd)))
		{
			return TRUE;
		}

		return TRUE;
	}

	virtual BOOL NotifyCommand (int iNotifyCode);
	
	virtual BOOL CanBeStretched () const
	{	
		return TRUE;	
	}
	virtual void OnAddToCustomizePage ();
	virtual HBRUSH OnCtlColor(CDC* pDC, UINT nCtlColor);
	virtual int OnDrawOnCustomizeList (
			CDC* pDC, const CRect& rect, BOOL bSelected);

	virtual void DuplicateData () {}
	virtual void ClearData () {}

	virtual void OnShow (BOOL bShow);
	virtual BOOL ExportToMenuButton (CBCGPToolbarMenuButton& menuButton) const;

	virtual void SetStyle (UINT nStyle);

	virtual int Compare(LPCTSTR lpszItem1, LPCTSTR lpszItem2);
	virtual void OnGlobalFontsChanged();

	virtual BOOL OnUpdateToolTip (CWnd* pWndParent, int iButtonIndex,
		CToolTipCtrl& wndToolTip, CString& str);

	virtual CString GetPrompt () const	{	return m_strPrompt;	}

	virtual BOOL SetACCData (CWnd* pParent, CBCGPAccessibilityData& data);

	virtual BOOL IsAlwaysOpaque () const
	{
		return TRUE;
	}

	virtual void OnCancelMode ();

protected:
	void Initialize ();
	void AdjustRect ();
	void SetHotEdit (BOOL bHot = TRUE);

// Attributes:
public:
	static void SetFlatMode (BOOL bFlat = TRUE)
	{
		m_bFlat = bFlat;
	}

	static BOOL IsFlatMode ()
	{
		return m_bFlat;
	}

	static void SetCenterVert (BOOL bCenterVert = TRUE)
	{
		m_bCenterVert = bCenterVert;
	}

	static BOOL IsCenterVert ()
	{
		return m_bCenterVert;
	}

	CComboBox* GetComboBox () const
	{
		return m_pWndCombo;
	}

	LPCTSTR GetText () const
	{
		return m_strEdit;
	}

	void SetText (LPCTSTR lpszText);

	void SetContextMenuID(UINT uiResID)
	{
		m_uiMenuResID = uiResID;
	}

	UINT GetContextMenuID()
	{
		return m_uiMenuResID;
	}

	static CBCGPToolbarComboBoxButton* GetByCmd (UINT uiCmd, BOOL bIsFocus = FALSE);
	static BOOL SelectItemAll (UINT uiCmd, int iIndex);
	static BOOL SelectItemAll (UINT uiCmd, DWORD_PTR dwData);
	static BOOL SelectItemAll (UINT uiCmd, LPCTSTR lpszText);
	static int GetCountAll (UINT uiCmd);
	static int GetCurSelAll (UINT uiCmd);
	static LPCTSTR GetItemAll (UINT uiCmd, int iIndex = -1);
	static DWORD_PTR GetItemDataAll (UINT uiCmd, int iIndex = -1);
	static void* GetItemDataPtrAll (UINT uiCmd, int iIndex = -1);
	static LPCTSTR GetTextAll (UINT uiCmd);

	virtual BOOL HasFocus () const;

	BOOL IsRibbonButton () const
	{
		return m_bIsRibbon;
	}
	BOOL IsRibbonFloaty () const
	{
		return m_bIsRibbonFloaty;
	}

	BOOL IsCtrlButton () const
	{
		return m_bIsCtrl;
	}

	virtual void SetOnGlass (BOOL bOnGlass);

protected:
	DWORD				m_dwStyle;
	CComboBox*			m_pWndCombo;
	CEdit*				m_pWndEdit;

	CStringList			m_lstItems;
	CList<DWORD_PTR, DWORD_PTR>	m_lstItemData;
	int					m_iWidth;
	int					m_iSelIndex;

	BOOL				m_bHorz;
	CString				m_strEdit;
	CRect				m_rectCombo;
	CRect				m_rectButton;

	int					m_nDropDownHeight;
	BOOL				m_bIsHotEdit;

	static BOOL			m_bFlat;
	static BOOL			m_bCenterVert;

	UINT				m_uiMenuResID;

	BOOL				m_bIsRibbon;
	BOOL				m_bIsRibbonFloaty;
	BOOL				m_bIsCtrl;

	CString				m_strPrompt;
	COLORREF			m_clrPrompt;
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPComboEdit

class BCGCBPRODLLEXPORT CBCGPComboEdit : public CBCGPEdit
{
// Construction
public:
	CBCGPComboEdit(CBCGPToolbarComboBoxButton& combo);

// Attributes
protected:
	CBCGPToolbarComboBoxButton&	m_combo;
	BOOL						m_bTracked;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPComboEdit)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBCGPComboEdit();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPComboEdit)
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnChange();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnPaint();
	//}}AFX_MSG
	afx_msg LRESULT OnMouseLeave(WPARAM,LPARAM);
	DECLARE_MESSAGE_MAP()
};

#endif // !defined(AFX_BCGPTOOLBARCOMBOBOXBUTTON_H__D5B381B4_CC65_11D1_A648_00A0C93A70EC__INCLUDED_)

/////////////////////////////////////////////////////////////////////////////
