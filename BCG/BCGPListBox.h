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
// BCGPListBox.h : header file
//

#if !defined(AFX_BCGPLISTBOX_H__7E778AEF_FFBB_486D_A7B3_D25D7868F5D9__INCLUDED_)
#define AFX_BCGPLISTBOX_H__7E778AEF_FFBB_486D_A7B3_D25D7868F5D9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "bcgcbpro.h"

#if (!defined _BCGSUITE_) && (!defined _BCGSUITE_INC_)
#include "BCGPToolBarImages.h"
#endif

struct BCGP_LB_ITEM_DATA;

extern BCGCBPRODLLEXPORT UINT BCGM_ON_CLICK_LISTBOX_PIN;

/////////////////////////////////////////////////////////////////////////////
// CBCGPListBox window

class BCGCBPRODLLEXPORT CBCGPListBox : public CListBox
{
	friend class CBCGPPropertySheet;

	DECLARE_DYNAMIC(CBCGPListBox)

// Construction
public:
	CBCGPListBox();

// Attributes
public:
	BOOL	m_bOnGlass;
	BOOL	m_bVisualManagerStyle;

	BOOL IsBackstageMode() const
	{
		return m_bBackstageMode;
	}

	BOOL IsPropertySheetNavigator() const
	{
		return m_bPropertySheetNavigator;
	}

	void EnablePins(BOOL bEnable = TRUE);
	BOOL HasPins() const
	{
		return m_bPins;
	}

	void EnableItemDescription(BOOL bEnable = TRUE, int nRows = 2);
	BOOL HasItemDescriptions() const
	{
		return m_nDescrRows > 0;
	}

	void EnableItemHighlighting(BOOL bEnable = TRUE);
	BOOL IsItemHighlightingEnabled() const
	{
		return m_bItemHighlighting;
	}

// Operations
public:
	BOOL SetImageList (HIMAGELIST hImageList, int nVertMargin = 2);
	BOOL SetImageList (UINT nImageListResID, int cxIcon = 16, int nVertMargin = 2);

	void SetItemImage(int nIndex, int nImageIndex);
	int GetItemImage(int nIndex) const;

	void SetItemDescription(int nIndex, const CString& strDescription);
	LPCTSTR GetItemDescription(int nIndex) const;

	void SetItemPinned(int nIndex, BOOL bSet = TRUE, BOOL bRedraw = FALSE);
	BOOL IsItemPinned(int nIndex);
	void ResetPins();

	void AddCaption(LPCTSTR lpszCaption);
	void AddSeparator();

	BOOL IsCaptionItem(int nIndex) const;
	BOOL IsSeparatorItem(int nIndex) const;

	void Enable(int nIndex, BOOL bEnabled = TRUE, BOOL bRedraw = TRUE);
	BOOL IsEnabled(int nIndex) const;

	void CleanUp();

	virtual int GetCheck(int /*nIndex*/) const
	{
		return 0;
	}

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPListBox)
	public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void DeleteItem(LPDELETEITEMSTRUCT lpDeleteItemStruct);
	protected:
	virtual void PreSubclassWindow();
	//}}AFX_VIRTUAL

	virtual void OnClickPin(int nClickedItem);
	virtual void OnClickItem(int /*nClickedItem*/)	{}
	virtual BOOL OnReturnKey() { return FALSE; }

	virtual int GetItemMinHeight();
	virtual void OnDrawItemContent(CDC* pDC, CRect rect, int nIndex);

	virtual int OnDrawItemName(CDC* pDC, int xStart, CRect rect, int nIndex, const CString& strName, UINT nDrawFlags);

// Implementation
public:
	virtual ~CBCGPListBox();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPListBox)
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnCancelMode();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnNcPaint();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg LRESULT OnLBAddString(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnLBGetItemData(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnLBInsertString(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnLBSetItemData(WPARAM wParam, LPARAM lParam);
	//}}AFX_MSG
	afx_msg BOOL OnSelchange();
	afx_msg LRESULT OnBCGSetControlVMMode (WPARAM, LPARAM);
	afx_msg LRESULT OnBCGSetControlAero (WPARAM, LPARAM);
	afx_msg LRESULT OnMouseLeave(WPARAM,LPARAM);
	afx_msg LRESULT OnBCGSetControlBackStageMode (WPARAM, LPARAM);
	afx_msg LRESULT OnPrintClient(WPARAM wp, LPARAM lp);
	afx_msg LRESULT OnSetFont(WPARAM wp, LPARAM lp);
	afx_msg LRESULT OnLBSetTabstops(WPARAM wp, LPARAM lp);
	DECLARE_MESSAGE_MAP()

	void OnDraw(CDC* pDC);
	int HitTest(CPoint pt, BOOL* pbPin = NULL, BOOL* pbCheck = NULL);

	const BCGP_LB_ITEM_DATA* _GetItemData(int nItem) const;
	BCGP_LB_ITEM_DATA* _GetAllocItemData(int nItem);

	int DU2DP(int nDU);

	int					m_nHighlightedItem;
	BOOL				m_bItemHighlighting;
	int					m_nClickedItem;
	BOOL				m_bTracked;
	HIMAGELIST			m_hImageList;		// External images
	CBCGPToolBarImages	m_ImageList;
	CSize				m_sizeImage;		// Image size
	BOOL				m_bBackstageMode;
	BOOL				m_bPropertySheetNavigator;
	BOOL				m_bIsPinHighlighted;
	BOOL				m_bPins;
	BOOL				m_bIsCheckHighlighted;
	BOOL				m_bHasCheckBoxes;
	BOOL				m_bHasDescriptions;
	int					m_nDescrRows;
	HFONT				m_hFont;
	int					m_nTextHeight;
	BOOL				m_bInAddingCaption;

	CList<int, int>		m_lstCaptionIndexes;

	CArray<int, int>	m_arTabStops;
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPCheckListBox window

class BCGCBPRODLLEXPORT CBCGPCheckListBox : public CBCGPListBox
{
	DECLARE_DYNAMIC(CBCGPCheckListBox)

// Construction
public:
	CBCGPCheckListBox();

// Attributes
public:
	void SetCheckStyle(UINT nCheckStyle = BS_AUTOCHECKBOX);
	UINT GetCheckStyle() const
	{
		return m_nCheckStyle;
	}

	int GetCheckCount() const;

// Operations
public:
	void SetCheck(int nIndex, int nCheck, BOOL bRedraw = TRUE);
	virtual int GetCheck(int nIndex) const;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPCheckListBox)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBCGPCheckListBox();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPCheckListBox)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	void SetSelectionCheck(int nCheck);

	UINT m_nCheckStyle;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPLISTBOX_H__7E778AEF_FFBB_486D_A7B3_D25D7868F5D9__INCLUDED_)
