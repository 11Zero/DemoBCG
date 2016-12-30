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
// BCGPRibbonPaletteButton.h: interface for the CBCGPRibbonPaletteButton class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPRIBBONPALETTEBUTTON_H__EE6BE1A6_CA76_4764_9A27_C5128583BEB6__INCLUDED_)
#define AFX_BCGPRIBBONPALETTEBUTTON_H__EE6BE1A6_CA76_4764_9A27_C5128583BEB6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"

#ifndef BCGP_EXCLUDE_RIBBON

#include "BCGPRibbonButton.h"
#include "BCGPToolBarImages.h"
#include "BCGPToolbarMenuButton.h"
#include "BCGPMenuButton.h"

class CBCGPRibbonPaletteIcon;
class CBCGPPopupMenu;
class CBCGPRibbonGalleryCtrl;

class BCGCBPRODLLEXPORT CBCGPRibbonPaletteButton : public CBCGPRibbonButton
{
	friend class CBCGPRibbonPaletteIcon;
	friend class CBCGPRibbonPanelMenuBar;
	friend class CBCGPRibbonPanelMenu;
	friend class CBCGPRibbonPanel;
	friend class CBCGPRibbonPaletteMenuButton;
	friend class CBCGPRibbonCollector;
	friend class CBCGPRibbonConstructor;
	friend class CBCGPRibbonGalleryCtrl;
	friend class CBCGPRibbonComboGalleryCtrl;

	DECLARE_DYNCREATE(CBCGPRibbonPaletteButton)

// Construction
public:
	CBCGPRibbonPaletteButton();

	CBCGPRibbonPaletteButton (
		UINT				nID,
		LPCTSTR				lpszText, 
		int					nSmallImageIndex,
		int					nLargeImageIndex,
		CBCGPToolBarImages& imagesPalette);

	CBCGPRibbonPaletteButton (
		UINT				nID,
		LPCTSTR				lpszText, 
		int					nSmallImageIndex,
		int					nLargeImageIndex,
		UINT				uiImagesPaletteResID = 0,
		int					cxPaletteImage = 0);

	// Owner-draw icons constructor:
	CBCGPRibbonPaletteButton (
		UINT				nID,
		LPCTSTR				lpszText, 
		int					nSmallImageIndex,
		int					nLargeImageIndex,
		CSize				sizeIcon,
		int					nIconsNum,					// If 0 - you should call AddGroup later
		BOOL				bDefaultButtonStyle = TRUE);// If FALSE - you need to implement your own border/content drawing

	virtual ~CBCGPRibbonPaletteButton();

// Attributes
public:
	void SetPaletteID (UINT nID)
	{
		m_nPaletteID = nID;
	}

	int GetPaletteID () const
	{
		return m_nPaletteID;
	}

	void SetButtonMode (BOOL bSet = TRUE)
	{
		m_bIsButtonMode = bSet;
	}

	BOOL IsButtonMode () const
	{
		return m_bIsButtonMode || IsCustom();
	}

	void SelectItem (int nItemIndex);
	int GetSelectedItem () const
	{
		return m_nSelected;
	}

	void EnableMenuResize (BOOL bEnable = TRUE, BOOL bVertcalOnly = FALSE)
	{
		m_bEnableMenuResize = bEnable;
		m_bMenuResizeVertical = bVertcalOnly;
	}

	BOOL IsMenuResizeEnabled () const
	{
		return m_bEnableMenuResize;
	}

	BOOL IsMenuResizeVertical () const
	{
		return m_bMenuResizeVertical;
	}

	void SetIconsInRow (int nIconsInRow)
	{
		m_nIconsInRow = nIconsInRow;
	}

	int GetIconsInRow () const
	{
		return m_nIconsInRow;
	}

	void SetItemToolTip (int nItemIndex, LPCTSTR lpszToolTip);
	LPCTSTR GetItemToolTip (int nItemIndex) const;
	void RemoveItemToolTips ();

	static int GetLastSelectedItem (UINT uiCmdID);
	static void ClearLastSelectedItem(UINT uiCmdID);

	BOOL IsMenuSideBar () const
	{
		return m_bMenuSideBar;
	}

	void EnablMenuSideBar (BOOL bEnable = TRUE)
	{
		m_bMenuSideBar = bEnable;
	}

	CBCGPToolBarImages& GetImages ()
	{
		return m_imagesPalette;
	}

	virtual CSize GetIconSize () const;
	virtual CSize GetItemSize () const { return GetIconSize(); }

	void SetDrawDisabledItems(BOOL bSet = TRUE)
	{
		m_bDrawDisabledItems = bSet;
	}
	
	BOOL IsDrawDisabledItems() const
	{
		return m_bDrawDisabledItems;
	}

	void EnableItemCheckBoxes(BOOL bEnable = TRUE)
	{
		m_bItemCheckBoxes = bEnable;
	}

	BOOL IsItemCheckBoxesEnabled() const
	{
		return m_bItemCheckBoxes;
	}

// Operations:
public:
	void AddGroup (
		LPCTSTR lpszGroupName,
		UINT	uiImagesPaletteResID,
		int		cxPaletteImage);

	void AddGroup (
		LPCTSTR lpszGroupName,
		CBCGPToolBarImages& imagesGroup);

	void AddGroup (			// For user-defined icons only!
		LPCTSTR lpszGroupName,
		int		nIconsNum);

	void SetGroupName (int nGroupIndex, LPCTSTR lpszGroupName);
	LPCTSTR GetGroupName (int nGroupIndex) const;

	void SetPalette (CBCGPToolBarImages& imagesPalette);
	void SetPalette (UINT uiImagesPaletteResID, int cxPaletteImage);

	virtual void Clear ();

	void RedrawIcons ();

	void AddSubItem (CBCGPBaseRibbonElement* pSubItem, int nIndex = -1, BOOL bOnTop = FALSE);
	virtual void GetVisibleElements (CArray<CBCGPBaseRibbonElement*, CBCGPBaseRibbonElement*>& arElements);

protected:
	void CommonInit ();
	void CreateIcons ();
	void AddScrollButtons();
	void RemoveAll ();
	void RebuildIconLocations ();
	
	virtual BOOL IsButtonLook () const
	{
		return	m_bQuickAccessMode || m_bFloatyMode || m_bIsButtonMode || m_bIsCollapsed || m_bSearchResultMode ||
				m_pParentGroup != NULL || m_nIcons == 0 || IsCustom()								;
	}

	BOOL IsOwnerDraw () const
	{
		return m_bIsOwnerDraw;
	}

	virtual void OnClickPaletteIcon (CBCGPRibbonPaletteIcon* pIcon);
	void GetMenuItems (CArray<CBCGPBaseRibbonElement*, CBCGPBaseRibbonElement*>& arButtons);

	virtual CBCGPBaseRibbonElement* HitTest (CPoint point);
	virtual CBCGPBaseRibbonElement* GetPressed ();
	virtual CBCGPBaseRibbonElement* GetHighlighted ();

	virtual void CopyFrom (const CBCGPBaseRibbonElement& src);
	virtual void SetParentCategory (CBCGPRibbonCategory* pParent);

	virtual void OnShowPopupMenu ();

	int GetMenuRowHeight () const;

	virtual UINT GetNotifyID ()
	{
		if (m_bNotifyPaletteID)
		{
			SetNotifyParentID (FALSE);
			return m_nPaletteID;
		}
		else
		{
			return m_nID;
		}
	}

	virtual BOOL OnKey (BOOL bIsMenuKey);
	virtual CRect GetKeyTipRect (CDC* pDC, BOOL bIsMenu);

	virtual CString GetIconToolTip (const CBCGPRibbonPaletteIcon* pIcon) const;
	virtual CString GetIconDescription (const CBCGPRibbonPaletteIcon* pIcon) const;

	virtual BOOL CanBeStretchedHorizontally ()
	{
		return !IsButtonMode () && !m_bIsCollapsed;
	}

	virtual void SetInitialMode (BOOL bOneRow = FALSE)
	{
		CBCGPRibbonButton::SetInitialMode (bOneRow);

		m_bResetColumns = TRUE;
		m_bIsCollapsed = FALSE;
	}

	virtual BOOL HasLargeMode () const
	{
		if (IsCustom())
		{
			return FALSE;
		}

		return !m_bIsButtonMode || CBCGPRibbonButton::HasLargeMode ();
	}

	virtual void StretcheHorizontally ()
	{
		if (m_bSmallIcons || m_nPanelColumns <= 3)
		{
			m_bIsCollapsed = TRUE;
		}
		else
		{
			m_nPanelColumns--;
		}
	}

	virtual BOOL OnClickPaletteSubItem (CBCGPRibbonButton* /*pButton*/, CBCGPRibbonPanelMenuBar* /*pMenuBar*/)
	{
		return FALSE;
	}

	virtual BOOL IsItemMenuLook () const
	{
		return FALSE;
	}

	virtual BOOL IsWholeRowHeight () const
	{
		return m_bOnDialogBar;
	}
	
// Overrides
public:
	virtual void OnDraw (CDC* pDC);

	virtual CSize GetRegularSize (CDC* pDC);
	virtual CSize GetCompactSize (CDC* pDC);

	virtual void OnAfterChangeRect (CDC* pDC);

	virtual BOOL HasMenu () const
	{
		return m_nIcons > 0 && !IsBackstageViewMode();
	}

	virtual BOOL HasSubitems () const
	{
		return m_nIcons > 0;
	}

	virtual CBCGPBaseRibbonElement* GetDroppedDown ();
	virtual void OnEnable (BOOL bEnable);

	virtual void OnRTLChanged (BOOL bIsRTL);

	virtual int GetGroupOffset () const
	{
		return 0;
	}

	virtual void OnSetFocus (BOOL bSet);

	virtual BOOL SetACCData (CWnd* pParent, CBCGPAccessibilityData& data);

protected:
	virtual void OnDrawPaletteIcon (CDC* pDC, CRect rectIcon, int nIconIndex, CBCGPRibbonPaletteIcon* pIcon, COLORREF clrText);

	virtual CWnd* GetParentWnd () const
	{
		ASSERT_VALID (this);

		if (m_pParentControl != NULL)
		{
			ASSERT_VALID (m_pParentControl);
			return m_pParentControl;
		}

		return CBCGPRibbonButton::GetParentWnd ();
	}

// Attributes
protected:
	CArray<CBCGPBaseRibbonElement*, CBCGPBaseRibbonElement*>	m_arIcons;
	CStringArray		m_arGroupNames;
	CArray<int,int>		m_arGroupLen;

	UINT				m_nPaletteID;
	BOOL				m_bIsButtonMode;
	BOOL				m_bNotifyPaletteID;
	CBCGPToolBarImages	m_imagesPalette;
	int					m_nImagesInRow;
	int					m_nPanelColumns;
	int					m_nImagesInColumn;
	BOOL				m_bSmallIcons;
	int					m_nScrollOffset;
	int					m_nScrollTotal;
	int					m_nSelected;
	BOOL				m_bEnableMenuResize;
	BOOL				m_bMenuResizeVertical;
	int					m_nIconsInRow;
	CStringArray		m_arToolTips;
	BOOL				m_bIsOwnerDraw;
	BOOL				m_bDefaultButtonStyle;
	int					m_nIcons;
	BOOL				m_bMenuSideBar;
	BOOL				m_bIsCollapsed;
	BOOL				m_bResetColumns;
	CWnd*				m_pParentControl;
	BOOL				m_bIsComboMode;
	BOOL				m_bDrawDisabledItems;
	BOOL				m_bItemCheckBoxes;

	static CMap<UINT,UINT,int,int>	m_mapSelectedItems;

	void SetNotifyParentID (BOOL bSet);
};

////////////////////////////////////////////
// CBCGPRibbonPaletteIcon

class BCGCBPRODLLEXPORT CBCGPRibbonPaletteIcon : public CBCGPRibbonButton
{
	friend class CBCGPRibbonPaletteButton;
	friend class CBCGPRibbonPanel;
	friend class CBCGPRibbonUndoButton;
	friend class CBCGPRibbonComboGalleryCtrl;

	DECLARE_DYNCREATE(CBCGPRibbonPaletteIcon)

public:
	CBCGPRibbonPaletteIcon (CBCGPRibbonPaletteButton* pOwner = NULL,
		int nIndex = 0);

public:
	BOOL IsFirst () const;
	BOOL IsLast () const;

	int GetIndex () const
	{
		return m_nIndex;
	}

	BOOL IsFirstInRow () const
	{
		return m_bIsFirstInRow;
	}

	BOOL IsLastInRow () const
	{
		return m_bIsLastInRow;
	}

	BOOL IsFirstInColumn () const
	{
		return m_bIsFirstInColumn;
	}

	BOOL IsLastInColumn () const
	{
		return m_bIsLastInColumn;
	}

	CBCGPRibbonPaletteButton* GetOwner ()
	{
		return m_pOwner;
	}

	virtual UINT GetQATID () const
	{
		if (m_pOwner != NULL)
		{
			ASSERT_VALID (m_pOwner);
			return m_pOwner->GetID ();
		}

		return m_nID;
	}

protected:
	virtual void OnDraw (CDC* pDC);
	virtual void OnClick (CPoint point);
	virtual void OnLButtonDown (CPoint point);
	virtual void CopyFrom (const CBCGPBaseRibbonElement& src);
	virtual CSize GetRegularSize (CDC* pDC);
	virtual CSize GetCompactSize (CDC* pDC);
	virtual CSize GetIntermediateSize (CDC* pDC);
	virtual BOOL IsAutoRepeatMode (int& nDelay /* ms */) const;
	virtual BOOL OnAutoRepeat ();
	virtual BOOL CanBeAddedToQAT () const
	{
		return TRUE;
	}
	virtual BOOL OnAddToQAToolbar (CBCGPRibbonQuickAccessToolbar& qat);

	virtual CString GetToolTipText () const;
	virtual CString GetDescription () const;
	virtual void OnHighlight (BOOL bHighlight);

	virtual CWnd* GetParentWnd () const
	{
		ASSERT_VALID (this);

		if (m_pOwner != NULL)
		{
			ASSERT_VALID (m_pOwner);
			return m_pOwner->GetParentWnd ();
		}

		return CBCGPRibbonButton::GetParentWnd ();
	}

	virtual BOOL IsPaletteIcon () const
	{
		ASSERT_VALID (this);

		if (m_pOwner != NULL)
		{
			ASSERT_VALID (m_pOwner);
			return !m_pOwner->IsItemMenuLook ();
		}

		return TRUE;
	}

	virtual BOOL SetACCData (CWnd* pParent, CBCGPAccessibilityData& data);

	virtual void OnAfterChangeRect (CDC* pDC)
	{
		CBCGPRibbonButton::OnAfterChangeRect(pDC);
		SetCheckRect();
	}

	void SetCheckRect();

	int							m_nIndex;
	int							m_nImageIndex;
	CBCGPRibbonPaletteButton*	m_pOwner;
	BOOL						m_bIsFirstInRow;
	BOOL						m_bIsLastInRow;
	BOOL						m_bIsFirstInColumn;
	BOOL						m_bIsLastInColumn;
	int							m_nCheckState;
	CRect						m_rectCheckBox;
};

////////////////////////////////////////////////
// CBCGPRibbonPaletteMenuButton

class BCGCBPRODLLEXPORT CBCGPRibbonPaletteMenuButton : public CBCGPToolbarMenuButton
{
	DECLARE_DYNCREATE(CBCGPRibbonPaletteMenuButton)

// Construction
protected:
	CBCGPRibbonPaletteMenuButton();

public:
	CBCGPRibbonPaletteMenuButton (UINT uiID, int iImage, LPCTSTR lpszText,
		CBCGPToolBarImages& imagesPalette);

	CBCGPRibbonPaletteMenuButton (UINT uiID, int iImage, LPCTSTR lpszText,
		UINT uiImagesPaletteResID = 0, int cxPaletteImage = 0);

	virtual ~CBCGPRibbonPaletteMenuButton ();

protected:
	void CommonInit ();

// Attributes:
public:
	CBCGPRibbonPaletteButton& GetPalette ()
	{
		return m_paletteButton;
	}

protected:
	CBCGPRibbonPaletteButton m_paletteButton;

// Overrides:
public:
	virtual void CopyFrom (const CBCGPToolbarButton& src);
	virtual CBCGPPopupMenu* CreatePopupMenu ();
	virtual BOOL IsEmptyMenuAllowed () const
	{
		return TRUE;
	}
	virtual BOOL HasButton () const
	{
		return TRUE;
	}

// Operations:
public:
};

////////////////////////////////////////////////
// CBCGPRibbonComboGalleryCtrl

class BCGCBPRODLLEXPORT CBCGPRibbonComboGalleryCtrl : public CBCGPMenuButton
{
	DECLARE_DYNCREATE(CBCGPRibbonComboGalleryCtrl)

// Construction
public:
	CBCGPRibbonComboGalleryCtrl ();
	virtual ~CBCGPRibbonComboGalleryCtrl ();

// Attributes:
public:
	void SetIcons(UINT uiImagesResID, int cxImage);

	CBCGPRibbonPaletteButton& GetPalette ()
	{
		return m_paletteButton;
	}

	int GetCount() const
	{
		return m_paletteButton.m_nIcons;
	}

	int GetCurSel() const
	{
		return m_paletteButton.GetSelectedItem();
	}

	int SetCurSel(int nSelect)
	{
		if (m_paletteButton.m_nPaletteID == 0 && ::IsWindow (GetSafeHwnd ()))
		{
			m_paletteButton.m_nPaletteID = GetDlgCtrlID();
		}

		int nOldSel = m_paletteButton.GetSelectedItem();
		m_paletteButton.SelectItem(nSelect);

		if (GetSafeHwnd() != NULL)
		{
			RedrawWindow();
		}

		return nOldSel;
	}

// Operations:
	int AddString(LPCTSTR lpszString, int nIconIndex = -1);
	void AddLabel(LPCTSTR lpszString);
	void AddCommand(UINT uiCmdID, LPCTSTR lpszString, BOOL bOnTop = FALSE);
	void ResetContent();

// Overrides:
public:
	virtual void OnShowMenu ();
	virtual void OnDraw (CDC* pDC, const CRect& rect, UINT uiState);

protected:
	CBCGPRibbonPaletteButton	m_paletteButton;
	BOOL						m_bHasDescriptions;

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPRibbonComboGalleryCtrl)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif // BCGP_EXCLUDE_RIBBON

#endif // !defined(AFX_BCGPRIBBONPALETTEBUTTON_H__EE6BE1A6_CA76_4764_9A27_C5128583BEB6__INCLUDED_)
