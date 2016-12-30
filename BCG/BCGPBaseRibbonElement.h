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
// BCGPBaseRibbonElement.h: interface for the CBCGPBaseRibbonElement class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPBASERIBBONELEMENT_H__920C1FDC_61E3_4AC0_995B_AF8F333276E4__INCLUDED_)
#define AFX_BCGPBASERIBBONELEMENT_H__920C1FDC_61E3_4AC0_995B_AF8F333276E4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"

#ifndef BCGP_EXCLUDE_RIBBON

class CBCGPRibbonCategory;
class CBCGPRibbonPanel;
class CBCGPRibbonCmdUI;
class CBCGPPopupMenu;
class CBCGPRibbonButtonsGroup;
class CBCGPRibbonPanelMenuBar;
class CBCGPRibbonCommandsListBox;
class CBCGPPopupMenu;
class CBCGPRibbonQuickAccessToolbar;
class CBCGPRibbonKeyTip;
class CBCGPGridRow;
class CBCGPRibbonCustomElement;

#include "bcgpaccessibility.h"
#include "BCGPWnd.h"
#include "BCGPToolBar.h"

class BCGCBPRODLLEXPORT CBCGPBaseRibbonElement :  public CBCGPBaseAccessibleObject
{
	friend class CBCGPRibbonPanel;
	friend class CBCGPRibbonBar;
	friend class CBCGPRibbonCmdUI;
	friend class CBCGPRibbonButtonsGroup;
	friend class CBCGPPopupMenu;
	friend class CBCGPRibbonPanelMenuBar;
	friend class CBCGPRibbonPanelMenu;
	friend class CBCGPRibbonFloaty;
	friend class CBCGPRibbonCategory;
	friend class CBCGPPopupMenuBar;
	friend class CBCGPRibbonQuickAccessToolbar;
	friend class CBCGPRibbonPaletteButton;
	friend class CBCGPRibbonStatusBar;
	friend class CBCGPRibbonCommandsListBox;
	friend class CBCGPRibbonTreeCtrl;
	friend class CBCGPRibbonMainPanel;
	friend class CBCGPRibbonBackstageViewPanel;
	friend class CBCGPRibbonButton;
	friend class CBCGPRibbonComboGalleryCtrl;
	friend class CBCGPRibbonCustomPanel;
	friend class CBCGPRibbonCustomElement;
	friend class CBCGPRibbonCustomizeRibbonPage;
	friend class CBCGPRibbonMainButton;
	friend class CBCGPRibbonTabsGroup;

	DECLARE_DYNAMIC(CBCGPBaseRibbonElement)

// Construction
public:
	CBCGPBaseRibbonElement ();
	virtual ~CBCGPBaseRibbonElement ();

// Attributes
public:
	enum RibbonElementLocation
	{
		RibbonElementNotInGroup,
		RibbonElementSingleInGroup,
		RibbonElementFirstInGroup,
		RibbonElementLastInGroup,
		RibbonElementMiddleInGroup,
	};

	enum RibbonImageType
	{
		RibbonImageLarge,
		RibbonImageSmall
	};

	RibbonElementLocation GetLocationInGroup () const
	{
		return m_Location;
	}

	virtual void SetID (UINT nID);
	UINT GetID () const
	{
		return m_nID;
	}

	virtual void SetText (LPCTSTR lpszText);
	LPCTSTR GetText () const
	{
		return m_strText;
	}

	virtual void SetKeys (LPCTSTR lpszKeys, LPCTSTR lpszMenuKeys = NULL);
	LPCTSTR GetKeys () const
	{
		return m_strKeys;
	}

	LPCTSTR GetMenuKeys () const
	{
		return m_strMenuKeys;
	}

	virtual void SetTextAlwaysOnRight (BOOL bSet = TRUE);
	BOOL IsTextAlwaysOnRight () const
	{
		return m_bTextAlwaysOnRight;
	}

	CBCGPPopupMenu*	GetPopupMenu()
	{
		return m_pPopupMenu;
	}

	void SetRect (CRect rect)
	{
		m_rect = rect;
	}

	CRect GetRect () const
	{
		return m_rect;
	}

	DWORD_PTR GetData () const
	{
		return m_dwData;
	}

	void SetData (DWORD_PTR dwData)
	{
		m_dwData = dwData;
	}

	BOOL IsDefaultMenuLook () const
	{
		return m_bIsDefaultMenuLook;
	}

	void SetDefaultMenuLook (BOOL bIsDefaultMenuLook = TRUE)
	{
		m_bIsDefaultMenuLook = bIsDefaultMenuLook;
	}

	virtual CString GetToolTipText () const;
	virtual CString GetDescription () const;
	CString GetToolTip () const
	{
		return m_strToolTip;
	}

	virtual void SetToolTipText (LPCTSTR lpszText);
	virtual void SetDescription (LPCTSTR lpszText);

	virtual BOOL IsHighlighted () const
	{
		return m_bIsHighlighted;
	}

	virtual BOOL IsFocused () const
	{
		return m_bIsFocused;
	}

	virtual BOOL IsPressed () const
	{
		return m_bIsPressed;
	}

	virtual BOOL IsDisabled () const
	{
		return m_bIsDisabled;
	}

	virtual BOOL IsChecked () const
	{
		return m_bIsChecked;
	}

	virtual BOOL IsDroppedDown () const
	{
		return 	m_bIsDroppedDown;
	}

	BOOL IsLargeMode () const
	{
		return !m_bCompactMode && !m_bIntermediateMode;
	}

	BOOL IsCompactMode () const
	{
		return m_bCompactMode;
	}

	BOOL IsIntermediateMode () const
	{
		return m_bIntermediateMode;
	}

	BOOL IsVisible () const
	{
		return m_bIsVisible;
	}

	void SetVisible (BOOL bIsVisible)
	{
		m_bIsVisible = bIsVisible;
	}

	BOOL IsCustom() const { return m_bIsCustom; }

	virtual BOOL IsAlwaysLargeImage () const
	{
		return m_bIsAlwaysLarge;
	}

	virtual void SetCompactMode (BOOL bCompactMode = TRUE)
	{
		if (!bCompactMode)
		{
			if (m_bCompactMode)
			{
				if (HasIntermediateMode ())
				{
					m_bIntermediateMode = TRUE;
					m_bCompactMode = FALSE;
				}
				else
				{
					m_bIntermediateMode = FALSE;
					m_bCompactMode = FALSE;
				}
			}
		}
		else
		{
			if (m_bCompactMode)
			{
				return;
			}

			if (m_bIntermediateMode)
			{
				if (HasCompactMode ())
				{
					m_bCompactMode = TRUE;
					m_bIntermediateMode = FALSE;
				}
			}
			else
			{
				if (HasIntermediateMode ())
				{
					m_bCompactMode = FALSE;
					m_bIntermediateMode = TRUE;
				}
			}
		}
	}

	virtual BOOL CanBeCompacted () const
	{
		if (IsAlwaysLargeImage () || m_bCompactMode)
		{
			return FALSE;
		}

		return m_bIntermediateMode ? HasCompactMode () : HasIntermediateMode ();
	}

	const CSize& GetPadding() const { return m_sizePadding; }
	virtual void SetPadding(const CSize& sizePadding) { m_sizePadding = sizePadding; }

	virtual CSize GetSize (CDC* pDC)
	{
		if (!CBCGPToolBar::IsCommandPermitted (m_nID))
		{
			return CSize (0, 0);
		}

		if (g_pUserToolsManager != NULL && g_pUserToolsManager->GetToolsEntryCmd() == m_nID)
		{
			return CSize (0, 0);
		}

		return (m_bIntermediateMode ? GetIntermediateSize (pDC) : m_bCompactMode ? GetCompactSize (pDC) : GetRegularSize (pDC));
	}

	virtual CSize GetRegularSize (CDC* pDC) = 0;
	virtual CSize GetCompactSize (CDC* pDC)
	{
		return GetRegularSize (pDC);
	}
	virtual CSize GetIntermediateSize (CDC* pDC)
	{
		return GetRegularSize (pDC);
	}

	virtual BOOL HasLargeMode () const
	{
		return FALSE;
	}

	virtual BOOL HasIntermediateMode () const
	{
		return TRUE;
	}

	virtual BOOL HasCompactMode () const
	{
		return FALSE;
	}

	virtual void SetInitialMode (BOOL bOneRow = FALSE)
	{
		m_bIntermediateMode = FALSE;
		m_bCompactMode = FALSE;

		if (m_pParentGroup != NULL || bOneRow)
		{
			if (HasCompactMode ())
			{
				m_bCompactMode = TRUE;
			}
			else if (HasIntermediateMode ())
			{
				m_bIntermediateMode = TRUE;
			}
		}
		else
		{
			if (HasLargeMode ())
			{
			}
			else if (HasIntermediateMode ())
			{
				m_bIntermediateMode = TRUE;
			}
			else
			{
				m_bIntermediateMode = FALSE;
			}
		}
	}

	virtual CSize GetImageSize (RibbonImageType /*type*/) const
	{
		return CSize (0, 0);
	}

	virtual BOOL IsSeparator () const
	{
		return FALSE;
	}

	virtual BOOL IsTabStop () const
	{
		return TRUE;
	}

	BOOL IsShowGroupBorder () const
	{
		return m_bShowGroupBorder;
	}

	virtual void DrawImage (CDC* /*pDC*/, RibbonImageType /*type*/, CRect /*rectImage*/) {}
	virtual HICON ExportImageToIcon (BOOL /*bIsLargeIcon*/) { return NULL; }

	virtual void OnAfterChangeRect (CDC* pDC);
	virtual void OnShow (BOOL /*bShow*/) {}

	virtual CBCGPBaseRibbonElement* HitTest (CPoint point);

	CBCGPRibbonCategory* GetParentCategory () const
	{
		return m_pParent;
	}

	virtual CBCGPRibbonPanel* GetParentPanel () const;

	virtual CWnd* GetParentWnd () const;

	CBCGPRibbonBar*	GetParentRibbonBar () const
	{
		return m_pRibbonBar;
	}

	virtual void SetParentRibbonBar (CBCGPRibbonBar* pRibbonBar);
	
	CBCGPRibbonButtonsGroup* GetParentGroup () const
	{
		return m_pParentGroup;
	}

	CBCGPRibbonBar* GetTopLevelRibbonBar () const;

	BOOL IsMenuMode () const;
	
	virtual BOOL HasMenu () const
	{
		return FALSE;
	}

	virtual BOOL HasPin () const
	{
		return FALSE;
	}

	virtual BOOL HasSubitems () const
	{
		return FALSE;
	}

	virtual void OnShowPopupMenu ();

	virtual BOOL CanBeAddedToQAT () const;
	
	BOOL IsQATMode () const
	{
		return m_bQuickAccessMode;
	}

	BOOL IsStatusBarMode () const
	{
		return m_bStatusBarMode;
	}

	BOOL IsSearchResultMode () const
	{
		return m_bSearchResultMode;
	}

	virtual BOOL IsWholeRowHeight () const
	{
		return FALSE;
	}

	virtual UINT GetNotifyID ()
	{
		return m_nID;
	}

	virtual BOOL IsAutoRepeatMode (int& /*nDelay*/ /* ms */) const
	{
		return FALSE;
	}

	virtual BOOL OnAutoRepeat ()
	{
		return FALSE;
	}

	virtual BOOL StretchToWholeRow (CDC* pDC, int nHeight);

	virtual BOOL OnDrawMenuImage (CDC* pDC, CRect rect)
	{
		DrawImage (pDC, RibbonImageSmall, rect);
		return TRUE;
	}

	CBCGPBaseRibbonElement*	GetOriginal () const
	{
		return m_pOriginal;
	}

	void SetStretchedOnDialogBar (BOOL bSet = TRUE)
	{
		m_bCanBeStretchedOnDialogBar = bSet;
	}

	BOOL IsOnPaletteTop () const
	{
		return m_bIsOnPaletteTop;
	}
	
	BOOL IsTabElement () const
	{
		return m_bIsTabElement;
	}
	
	BOOL IsFloatyMode () const
	{
		return m_bFloatyMode;
	}

	void SetForceDrawDisabledOnList(BOOL bSet) { m_bForceDrawDisabledOnList = bSet; }

// Overrides
public:
	virtual void SetParentCategory (CBCGPRibbonCategory* pParent);
	virtual void CopyFrom (const CBCGPBaseRibbonElement& src);
	virtual void CopyBaseFrom (const CBCGPBaseRibbonElement& src);
	virtual void SetParentMenu (CBCGPRibbonPanelMenuBar* pMenuBar);
	virtual void SetOriginal (CBCGPBaseRibbonElement* pOriginal);

	virtual int AddToListBox (CBCGPRibbonCommandsListBox* pWndListBox, BOOL bDeep);
	virtual CBCGPGridRow* AddToTree (CBCGPGridCtrl* pGrid, CBCGPGridRow* pParent);
	virtual void OnDrawOnList (CDC* pDC, CString strText, int nTextOffset, CRect rect, BOOL bIsSelected, BOOL bHighlighted);

	virtual void OnDraw (CDC* pDC) = 0;
	virtual void OnCalcTextSize (CDC* /*pDC*/)	{}
	virtual void CleanUpSizes ()	{}

	virtual BOOL CanBeStretched ()
	{
		return TRUE;
	}

	virtual BOOL CanBeStretchedOnDialogBar ()
	{
		return m_bCanBeStretchedOnDialogBar;
	}

	virtual BOOL CanBeStretchedHorizontally ()
	{
		return FALSE;
	}

	virtual void StretcheHorizontally ()
	{
		ASSERT (FALSE);
	}

	virtual BOOL IsAlignByColumn () const
	{
		return TRUE;
	}

	virtual void ClosePopupMenu ();
	virtual CBCGPBaseRibbonElement* Find (const CBCGPBaseRibbonElement* pElement);
	virtual CBCGPBaseRibbonElement* FindByID (UINT uiCmdID);
	virtual CBCGPBaseRibbonElement* FindByIDNonCustom (UINT uiCmdID);
	virtual CBCGPBaseRibbonElement* FindByData (DWORD_PTR dwData);
	virtual CBCGPBaseRibbonElement* FindByOriginal (CBCGPBaseRibbonElement* pOriginal);
	virtual void UpdateTooltipInfo ();

	virtual BOOL OnAddToQAToolbar (CBCGPRibbonQuickAccessToolbar& qat);
	virtual UINT GetQATID () const
	{
		return m_nID;
	}

	virtual CBCGPBaseRibbonElement* CreateQATCopy();
	virtual CBCGPBaseRibbonElement* CreateCustomCopy();

	virtual void Redraw ();
	virtual void DestroyCtrl () {}

	virtual CBCGPBaseRibbonElement* GetPressed ();
	virtual CBCGPBaseRibbonElement* GetDroppedDown ();
	virtual CBCGPBaseRibbonElement* GetHighlighted ();
	virtual CBCGPBaseRibbonElement* GetFocused ();

	virtual void OnDrawKeyTip (CDC* pDC, const CRect& rect, BOOL bIsMenu);
	virtual CSize GetKeyTipSize (CDC* pDC);
	virtual CRect GetKeyTipRect (CDC* /*pDC*/, BOOL /*bIsMenu*/)	{	return CRect (0, 0, 0, 0);	}
	virtual BOOL OnKey (BOOL bIsMenuKey);
	virtual void AddToKeyList (CArray<CBCGPRibbonKeyTip*,CBCGPRibbonKeyTip*>& arElems);
	virtual BOOL OnMenuKey (UINT /*nUpperChar*/)	{	return FALSE;	}

	virtual void GetElementsByID (UINT uiCmdID, 
		CArray<CBCGPBaseRibbonElement*, CBCGPBaseRibbonElement*>& arElements);

	virtual void GetVisibleElements (
		CArray<CBCGPBaseRibbonElement*, CBCGPBaseRibbonElement*>& arElements);

	virtual void GetElementsByName (LPCTSTR lpszName, 
		CArray<CBCGPBaseRibbonElement*, CBCGPBaseRibbonElement*>& arButtons, DWORD dwFlags = 0);

	virtual void GetElements (
		CArray <CBCGPBaseRibbonElement*, CBCGPBaseRibbonElement*>& arElements)
	{
		arElements.Add (this);
	}

	virtual void OnRTLChanged (BOOL /*bIsRTL*/)	{}
	virtual void OnAfterAddToParent (CBCGPBaseRibbonElement* /*pParentElem*/)	{}

	virtual BOOL IsShowTooltipOnBottom () const
	{
		return m_pRibbonBar == NULL;
	}

	virtual void OnChangeMenuHighlight (CBCGPRibbonPanelMenuBar* /*pPanelMenuBar*/, CBCGPBaseRibbonElement* /*pHot*/)	{}
	virtual void NotifyHighlightListItem (int nIndex);

	virtual BOOL IsPaletteIcon () const
	{
		return FALSE;
	}

	virtual BOOL IsScrolledOut () const;

	virtual CBCGPBaseRibbonElement* GetFirstTabStop ()
	{
		if (IsTabStop () && !m_rect.IsRectEmpty ())
		{
			return this;
		}

		return NULL;
	}

	virtual CBCGPBaseRibbonElement* GetLastTabStop ()
	{
		if (IsTabStop () && !m_rect.IsRectEmpty ())
		{
			return this;
		}

		return NULL;
	}

	virtual void GetItemIDsList (CList<UINT,UINT>& lstItems) const;

	virtual BOOL IsBackstageViewMode() const
	{
		return m_bIsBackstageViewMode;
	}

	virtual void SetBackstageViewMode()
	{
		m_bIsBackstageViewMode = TRUE;
	}

	virtual CBCGPBaseRibbonElement* GetBackstageAttachedView()
	{
		return NULL;
	}

	virtual CWnd* GetTargetCmdWnd();

	virtual BOOL CanBePlacedOnNonCollapsiblePanel() const
	{
		return TRUE;
	}

// Accessibility:
	virtual HRESULT get_accParent(IDispatch **ppdispParent);
	virtual HRESULT accLocation(long* pxLeft, long* pyTop, long* pcxWidth, long* pcyHeight, VARIANT varChild);
	virtual HRESULT accNavigate(long navDir, VARIANT varStart, VARIANT *pvarEndUpAt);
	virtual HRESULT accHitTest(long xLeft, long yTop, VARIANT *pvarChild);
	virtual HRESULT accDoDefaultAction(VARIANT varChild);
	virtual BOOL SetACCData (CWnd* pParent, CBCGPAccessibilityData& data);

protected:
	virtual void OnLButtonDown (CPoint point);
	virtual void OnLButtonUp (CPoint /*point*/)		{}
	virtual void OnMouseMove (CPoint /*point*/)		{}
	virtual void OnLButtonDblClk (CPoint /*point*/)	{}
	virtual void OnHighlight (BOOL /*bHighlight*/)	{}
	virtual void OnSetFocus (BOOL /*bSet*/)			{}

	virtual void OnUpdateCmdUI (CBCGPRibbonCmdUI* pCmdUI, CFrameWnd* pTarget, BOOL bDisableIfNoHndler);
	virtual BOOL NotifyControlCommand (BOOL bAccelerator, int nNotifyCode, WPARAM wParam, LPARAM lParam);

	virtual BOOL ReplaceByID (UINT uiCmdID, CBCGPBaseRibbonElement* pElem);
	virtual BOOL ReplaceSubitemByID (UINT uiCmdID, CBCGPBaseRibbonElement* pElem, BOOL bCopyContent);

	virtual void OnEnable (BOOL /*bEnable*/)	{}
	virtual void OnCheck (BOOL /*bCheck*/)	{}

	virtual int GetDropDownImageWidth () const;

	virtual BOOL OnProcessKey (UINT /*nChar*/)	{	return FALSE;	}

	virtual BOOL HasFocus () const				{	return FALSE;	}

	virtual void OnAccDefaultAction()			{	NotifyCommand(TRUE);	}

	virtual int DoDrawText (CDC* pDC, const CString& strText, CRect rectText, UINT uiDTFlags,
							COLORREF clrText = (COLORREF)-1);

	virtual BOOL CanBeSeparated() const			{	return TRUE;	}
	virtual BOOL IsCustomIconAllowed() const	{	return TRUE;	}

	virtual void OnChangeVisualManager() {}

	virtual BOOL IsCaptionButton () const { return FALSE; }

// Operations
public:
	virtual BOOL NotifyCommand (BOOL bWithDelay = FALSE);
	void PostMenuCommand (UINT uiCmdId);
	void EnableUpdateTooltipInfo (BOOL bEnable = TRUE);
	void EnableTooltipInfoShortcut (BOOL bEnable = TRUE);

protected:
	void SetDroppedDown (CBCGPPopupMenu* pPopupMenu);
	
	virtual void SetCustom()
	{
		m_bIsCustom = TRUE;
	}

	virtual void ApplyCustomizationData(CBCGPRibbonCustomElement& customElement);

// Attributes
protected:
	RibbonElementLocation		m_Location;
	UINT						m_nID;
	DWORD_PTR					m_dwData;
	CString						m_strText;
	CString						m_strKeys;
	CString						m_strMenuKeys;
	BOOL						m_bTextAlwaysOnRight;
	CRect						m_rect;
	CBCGPRibbonCategory*		m_pParent;
	CBCGPRibbonButtonsGroup*	m_pParentGroup;
	CBCGPRibbonPanelMenuBar*	m_pParentMenu;
	CBCGPRibbonBar*				m_pRibbonBar;
	BOOL						m_bCompactMode;
	BOOL						m_bIntermediateMode;
	BOOL						m_bFloatyMode;
	BOOL						m_bQuickAccessMode;
	BOOL						m_bStatusBarMode;
	BOOL						m_bSearchResultMode;
	BOOL						m_bIsHighlighted;
	BOOL						m_bIsFocused;
	BOOL						m_bIsPressed;
	BOOL						m_bIsDisabled;
	BOOL						m_bIsChecked;
	BOOL						m_bIsRadio;
	BOOL						m_bIsDroppedDown;
	CBCGPBaseRibbonElement*		m_pOriginal;
	CString						m_strToolTip;
	CString						m_strDescription;
	int							m_nRow;
	BOOL						m_bDontNotify;
	CBCGPPopupMenu*				m_pPopupMenu;
	int							m_nImageOffset;
	BOOL						m_bShowGroupBorder;
	BOOL						m_bIsVisible;
	BOOL						m_bIsDefaultMenuLook;
	BOOL						m_bIsAlwaysLarge;
	BOOL						m_bDrawDefaultIcon;
	BOOL						m_bIsOnPaletteTop;
	BOOL						m_bOnBeforeShowItemMenuIsSent;
	BOOL						m_bIsTabElement;
	BOOL						m_bEnableUpdateTooltipInfo;
	BOOL						m_bEnableTooltipInfoShortcut;
	BOOL						m_bOnDialogBar;
	BOOL						m_bCanBeStretchedOnDialogBar;
	int							m_nTextGlassGlowSize;
	BOOL						m_bIsBackstageViewMode;
	int							m_nExtraMaginX;
	CWnd*						m_pWndTargetCmd;
	BOOL						m_bIsCustom;
	int							m_nCustomImageIndex;
	BOOL						m_bIsNew;
	BOOL						m_bForceDrawDisabledOnList;
	CSize						m_sizePadding;
};

class BCGCBPRODLLEXPORT CBCGPRibbonSeparator : public CBCGPBaseRibbonElement
{
	DECLARE_DYNCREATE(CBCGPRibbonSeparator)

public:
	CBCGPRibbonSeparator (BOOL bIsHoriz = FALSE);

protected:

	virtual BOOL IsSeparator () const
	{
		return TRUE;
	}

	virtual BOOL IsTabStop () const
	{
		return FALSE;
	}

public:
	virtual int AddToListBox (CBCGPRibbonCommandsListBox* pWndListBox, BOOL bDeep);

	virtual CBCGPGridRow* AddToTree (CBCGPGridCtrl* /*pGrid*/, CBCGPGridRow* /*pParent*/)
	{
		return NULL;
	}

	BOOL IsHorizontal () const
	{
		return m_bIsHoriz;
	}

protected:
	virtual void OnDraw (CDC* pDC);
	virtual CSize GetRegularSize (CDC* pDC);
	virtual void CopyFrom (const CBCGPBaseRibbonElement& src);
	virtual void OnDrawOnList (CDC* pDC, CString strText, int nTextOffset, CRect rect, BOOL bIsSelected, BOOL bHighlighted);
	virtual void OnAfterAddToParent (CBCGPBaseRibbonElement* /*pParentElem*/)
	{
		m_bIsHoriz = TRUE;
	}

	BOOL	m_bIsHoriz;
};

#endif // BCGP_EXCLUDE_RIBBON

#endif // !defined(AFX_BCGPBASERIBBONELEMENT_H__920C1FDC_61E3_4AC0_995B_AF8F333276E4__INCLUDED_)
