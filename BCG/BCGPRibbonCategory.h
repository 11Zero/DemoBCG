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

#if !defined(AFX_BCGPRIBBONCATEGORY_H__DADE0EF9_F888_4CDC_A1B7_8F972E57A7C9__INCLUDED_)
#define AFX_BCGPRIBBONCATEGORY_H__DADE0EF9_F888_4CDC_A1B7_8F972E57A7C9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BCGPRibbonCategory.h : header file
//

#include "BCGCBPro.h"

#ifndef BCGP_EXCLUDE_RIBBON

#include "BCGPBaseRibbonElement.h"
#include "BCGPToolBarImages.h"
#include "BCGPRibbonBar.h"

/////////////////////////////////////////////////////////////////////////////
// CBCGPRibbonTab

class CBCGPRibbonCategory;

class BCGCBPRODLLEXPORT CBCGPRibbonTab : public CBCGPBaseRibbonElement
{
	DECLARE_DYNAMIC(CBCGPRibbonTab)

	friend class CBCGPRibbonCategory;
	friend class CBCGPRibbonBar;

	CBCGPRibbonTab ();
	virtual void OnDraw (CDC* pDC);
	virtual CSize GetRegularSize (CDC* pDC);
	virtual void OnLButtonDown (CPoint point);
	virtual void OnLButtonDblClk (CPoint point);
	virtual CString GetToolTipText () const;
	virtual void CopyFrom (const CBCGPBaseRibbonElement& src);
	virtual CRect GetKeyTipRect (CDC* pDC, BOOL bIsMenu);
	virtual BOOL OnKey (BOOL bIsMenuKey);
	virtual BOOL IsShowTooltipOnBottom () const	{	return FALSE;	}
	virtual void Redraw ();
	virtual BOOL SetACCData (CWnd* pParent, CBCGPAccessibilityData& data);
	virtual void OnAccDefaultAction();

	BOOL					m_bIsTrancated;
	BCGPRibbonCategoryColor m_Color;
	int						m_nFullWidth;

	// Accessibility:
	virtual HRESULT get_accParent(IDispatch **ppdispParent);
	virtual HRESULT get_accDefaultAction(VARIANT varChild, BSTR *pszDefaultAction);
	virtual HRESULT accLocation(long *pxLeft, long *pyTop, long *pcxWidth, long *pcyHeight, VARIANT varChild);
	virtual HRESULT accNavigate(long navDir, VARIANT varStart, VARIANT *pvarEndUpAt);
	virtual HRESULT accHitTest(long xLeft, long yTop, VARIANT *pvarChild);

public:
	BOOL IsSelected () const;
};

/////////////////////////////////////////////////////////////////////////////////
// CBCGPRibbonCategoryScroll

class BCGCBPRODLLEXPORT CBCGPRibbonCategoryScroll : public CBCGPRibbonButton
{
	DECLARE_DYNAMIC(CBCGPRibbonCategoryScroll)

	friend class CBCGPRibbonCategory;
	friend class CBCGPRibbonBar;

	CBCGPRibbonCategoryScroll ();

	virtual void OnMouseMove (CPoint point);
	virtual void OnDraw (CDC* pDC);
	virtual BOOL OnAutoRepeat ();

	virtual BOOL IsAutoRepeatMode (int& /*nDelay*/) const
	{
		return TRUE;
	}

	virtual void CopyFrom (const CBCGPBaseRibbonElement& src);

	virtual void OnClick (CPoint /*point*/)
	{
		OnAutoRepeat ();
	}

	BOOL	m_bIsLeft;

public:
	BOOL IsLeftScroll () const
	{
		return m_bIsLeft;
	}
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPRibbonCategory

class CBCGPRibbonPanel;
class CBCGPRibbonBar;

class BCGCBPRODLLEXPORT CBCGPRibbonCategory :  public CBCGPBaseAccessibleObject
{
	friend class CBCGPRibbonBar;
	friend class CBCGPRibbonTab;
	friend class CBCGPRibbonPanel;
	friend class CBCGPRibbonPanelMenuBar;
	friend class CBCGPBaseRibbonElement;
	friend class CBCGPRibbonDialogBar;
	friend class CBCGPRibbonCollector;
	friend class CBCGPRibbonTreeCtrl;
	friend class CBCGPRibbonCustomizationData;
	friend class CBCGPRibbonCustomizeRibbonPage;
	friend class CBCGPRibbonCustomCategory;
	friend class CBCGPRibbonTabsGroup;

	DECLARE_DYNCREATE(CBCGPRibbonCategory)

// Construction
protected:
	CBCGPRibbonCategory();

	CBCGPRibbonCategory (
		CBCGPRibbonBar*	pParenrRibbonBar, 
		LPCTSTR			lpszName,
		UINT			uiSmallImagesResID,
		UINT			uiLargeImagesResID,
		CSize			sizeSmallImage = CSize (16, 16),
		CSize			sizeLargeImage = CSize (32, 32));

	// Custom category:
	CBCGPRibbonCategory (CBCGPRibbonBar* pParenrRibbonBar);
	CBCGPRibbonCategory* CreateCustomCopy(CBCGPRibbonBar* pParenrRibbonBar);

	void CommonInit (CBCGPRibbonBar*	pParenrRibbonBar = NULL,
					LPCTSTR				lpszName = NULL,
					UINT				uiSmallImagesResID = 0,
					UINT				uiLargeImagesResID = 0,
					CSize				sizeSmallImage = CSize (0, 0),
					CSize				sizeLargeImage = CSize (0, 0));

// Attributes
public:
	CBCGPRibbonBar* GetParentRibbonBar () const
	{
		return m_pParenrRibbonBar;
	}

	CBCGPRibbonPanelMenuBar* GetParentMenuBar () const
	{
		return m_pParentMenuBar;
	}

	CBCGPBaseRibbonElement* HitTest (CPoint point, BOOL bCheckPanelCaption = FALSE) const;
	CBCGPBaseRibbonElement* HitTestScrollButtons (CPoint point) const;
	int HitTestEx (CPoint point) const;
	CBCGPRibbonPanel* GetPanelFromPoint (CPoint point) const;

	CBCGPBaseRibbonElement* GetFirstVisibleElement () const;
	CBCGPBaseRibbonElement* GetLastVisibleElement () const;

	CRect GetTabRect () const
	{
		return m_Tab.m_rect;
	}

	CRect GetRect () const
	{
		return m_rect;
	}

	CRect GetClientRect() const;	// Rectangle without scroll buttons

	BOOL IsActive () const
	{
		return m_bIsActive;
	}

	CSize GetImageSize (BOOL bIsLargeImage) const;

	int GetImageCount (BOOL bIsLargeImage) const
	{
		return bIsLargeImage ? m_LargeImages.GetCount () : m_SmallImages.GetCount ();
	}

	LPCTSTR GetName () const
	{
		return m_strName;
	}

	CString GetDisplayName (BOOL bCheckForStyle = FALSE) const;

	void SetName (LPCTSTR lpszName);

	DWORD_PTR GetData () const
	{
		return m_dwData;
	}

	void SetData (DWORD_PTR dwData)
	{
		m_dwData = dwData;
	}

	UINT GetContextID () const
	{
		return m_uiContextID;
	}

	void SetTabColor (BCGPRibbonCategoryColor color)
	{
		m_Tab.m_Color = color;
	}

	BCGPRibbonCategoryColor GetTabColor () const
	{
		return m_Tab.m_Color;
	}

	BOOL IsVisible () const
	{
		return m_bIsVisible;
	}

	CBCGPBaseRibbonElement* GetDroppedDown ();
	CBCGPBaseRibbonElement* GetFocused ();
	CBCGPBaseRibbonElement* GetHighlighted ();

	CBCGPBaseRibbonElement* GetParentButton () const;

	CBCGPToolBarImages& GetSmallImages ()	{	return m_SmallImages;	}
	CBCGPToolBarImages& GetLargeImages ()	{	return m_LargeImages;	}

	void SetKeys (LPCTSTR lpszKeys);

	BOOL IsOnDialogBar () const
	{
		return m_bOnDialogBar;
	}

	BOOL IsScenicLook () const;

	int GetKey() const { return m_nKey; }

	BOOL IsCustom() const { return m_bIsCustom; }

	BOOL IsPrintPreview() const { return m_bIsPrintPreview; }

	CBCGPRibbonTab* GetTab()
	{
		return &m_Tab;
	}

// Operations
public:
	CBCGPRibbonPanel* AddPanel (LPCTSTR lpszPanelName, HICON hIcon = 0,
		CRuntimeClass* pRTI = NULL, BOOL bAutoDestroyIcon = TRUE,
		int nInsertAt = -1);

	CBCGPRibbonPanel* AddPanel (LPCTSTR lpszPanelName, const CBCGPToolBarImages& icons, int nIconIndex,
		CRuntimeClass* pRTI = NULL, int nInsertAt = -1);

	void SetCollapseOrder (const CArray<int, int>& arCollapseOrder);
	
	int GetPanelCount () const;
	CBCGPRibbonPanel* GetPanel (int nIndex);

	BOOL RemovePanel (int nIndex, BOOL bDelete = TRUE);

	int GetPanelIndex (const CBCGPRibbonPanel* pPanel) const;

	int GetMaxHeight (CDC* pDC);
	void SetPadding(const CSize& size);

	CBCGPBaseRibbonElement* FindByID (UINT uiCmdID, BOOL bVisibleOnly = TRUE, BOOL bNonCustomOnly = TRUE) const;
	CBCGPBaseRibbonElement* FindByData (DWORD_PTR dwData, BOOL bVisibleOnly = TRUE) const;

	CBCGPRibbonPanel* HighlightPanel (CBCGPRibbonPanel* pHLPanel, CPoint point);
	CBCGPRibbonPanel* FindPanelWithElem (const CBCGPBaseRibbonElement* pElement);

	BOOL HasPanel(CBCGPRibbonPanel* pPanel);
	CBCGPRibbonPanel* FindPanelByOriginal(CBCGPRibbonPanel* pOriginal);

	void AddHidden (CBCGPBaseRibbonElement* pElem);

	void GetElements (
		CArray <CBCGPBaseRibbonElement*, CBCGPBaseRibbonElement*>& arElements);

	void GetElementsByID (UINT uiCmdID, 
		CArray <CBCGPBaseRibbonElement*, CBCGPBaseRibbonElement*>& arElements);

	void GetElementsByName (LPCTSTR, 
		CArray <CBCGPBaseRibbonElement*, CBCGPBaseRibbonElement*>& arElements, DWORD dwFlags = 0);

	void GetVisibleElements (
		CArray <CBCGPBaseRibbonElement*, CBCGPBaseRibbonElement*>& arElements);

	void GetItemIDsList (CList<UINT,UINT>& lstItems, BOOL bHiddenOnly = FALSE) const;

	int GetTextTopLine () const;

	void OnChangeVisualManager();

// Overrides
	virtual void RecalcLayout (CDC* pDC);
	virtual void OnDraw (CDC* pDC);
	virtual void OnCancelMode();
	virtual CBCGPBaseRibbonElement* OnLButtonDown (CPoint point);
	virtual void OnLButtonUp (CPoint point);
	virtual void OnMouseMove (CPoint point);
	virtual void OnUpdateCmdUI (CBCGPRibbonCmdUI* pCmdUI, CFrameWnd* pTarget, BOOL bDisableIfNoHndler);
	virtual BOOL NotifyControlCommand (BOOL bAccelerator, int nNotifyCode, WPARAM wParam, LPARAM lParam);

	virtual BOOL OnDrawImage (CDC* pDC, CRect rect, 
		CBCGPBaseRibbonElement* pElement, BOOL bIsLargeImage, 
		BOOL nImageIndex, BOOL bCenter);

	virtual void CopyFrom (CBCGPRibbonCategory& src);

	virtual void OnDrawMenuBorder (CDC* /*pDC*/, CBCGPRibbonPanelMenuBar* /*pMenuBar*/)	{}
	
	virtual void OnRTLChanged (BOOL bIsRTL);
	virtual BOOL OnScrollHorz (BOOL bScrollLeft, int nScrollOffset = 0);

	virtual void ReposPanels (CDC* pDC);
	virtual void ReposPanelsVert (CDC* pDC);

	virtual BOOL OnKey (UINT nChar);

	virtual CBCGPRibbonMainPanel* GetMainPanel();

	//Accessibility
	virtual HRESULT get_accParent(IDispatch **ppdispParent);
	virtual HRESULT get_accChildCount(long *pcountChildren);
	virtual HRESULT get_accChild(VARIANT varChild, IDispatch **ppdispChild);
	virtual HRESULT accLocation(long *pxLeft, long *pyTop, long *pcxWidth, long *pcyHeight, VARIANT varChild);
	virtual HRESULT accNavigate(long navDir, VARIANT varStart, VARIANT *pvarEndUpAt);
	virtual HRESULT accHitTest(long xLeft, long yTop, VARIANT *pvarChild);
	virtual CWnd* GetParentWnd () const { return m_pParenrRibbonBar; }
	virtual BOOL OnSetAccData (long lVal);
	virtual BOOL SetACCData (CWnd* pParent, CBCGPAccessibilityData& data);

// Implementation
public:
	virtual ~CBCGPRibbonCategory();

protected:
	void AddPanelInternal(CBCGPRibbonPanel* pPanel, int nInsertAt);

	void SetActive (BOOL bIsActive = TRUE);
	void ShowElements (BOOL bShow = TRUE);

	void ShowFloating (CRect rectFloating);
	void NormalizeFloatingRect (CBCGPRibbonBar* pRibbonBar, CRect& rectCategory);

	void RecalcPanelWidths (CDC* pDC);
	void CleanUpSizes ();
	int GetMinWidth (CDC* pDC);

	BOOL SetPanelsLayout (int nWidth);
	void ResetPanelsLayout ();

	void UpdateScrollButtons ();
	void EnsureVisible (CBCGPRibbonButton* pButton);

	void OnCloseCustomizePage(BOOL bIsOK);

	void SetCustom();
	void AddCustomPanel(CBCGPRibbonPanel* pPanel, int nIndex = -1);
	void ApplyCustomizationData(CBCGPRibbonCustomCategory& customCategory);

	CString										m_strName;
	int											m_nKey;
	BOOL										m_bIsPrintPreview;
	BOOL										m_bIsActive;
	BOOL										m_bIsVisible;
	BOOL										m_bIsCustom;
	BOOL										m_bIsQATOnly;
	BOOL										m_bOnDialogBar;
	CArray<CBCGPRibbonPanel*,CBCGPRibbonPanel*>	m_arPanels;
	CBCGPRibbonBar*								m_pParenrRibbonBar;
	CBCGPRibbonPanelMenuBar*					m_pParentMenuBar;
	BOOL										m_bMouseIsPressed;
	CRect										m_rect;
	CBCGPRibbonTab								m_Tab;
	CBCGPRibbonCategoryScroll					m_ScrollLeft;
	CBCGPRibbonCategoryScroll					m_ScrollRight;
	int											m_nScrollOffset;
	DWORD_PTR									m_dwData;
	UINT										m_uiContextID;
	int											m_nLastCategoryWidth;
	int											m_nLastCategoryOffsetY;
	int											m_nMinWidth;
	CArray<int, int>							m_arCollapseOrder;
	clock_t										m_ActiveTime;
	BOOL										m_bIsNew;
	BOOL										m_bToBeDeleted;

	//----------------------
	// Category image lists:
	//----------------------
	CBCGPToolBarImages		m_SmallImages;
	CBCGPToolBarImages		m_LargeImages;

	//---------------------------------
	// Category elements (non-visible):
	//---------------------------------
	CArray<CBCGPBaseRibbonElement*, CBCGPBaseRibbonElement*>	m_arElements;
};

#endif // BCGP_EXCLUDE_RIBBON

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPRIBBONCATEGORY_H__DADE0EF9_F888_4CDC_A1B7_8F972E57A7C9__INCLUDED_)
