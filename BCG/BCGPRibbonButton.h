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
// BCGPRibbonButton.h: interface for the CBCGPRibbonButton class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPRIBBONBUTTON_H__3698E9F7_DEB7_43C4_90C0_7973B7FF0A8F__INCLUDED_)
#define AFX_BCGPRIBBONBUTTON_H__3698E9F7_DEB7_43C4_90C0_7973B7FF0A8F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"

#ifndef BCGP_EXCLUDE_RIBBON

#include "BCGPBaseRibbonElement.h"

#define BCGPRIBBON_IMAGE_AUTO	9999

class BCGCBPRODLLEXPORT CBCGPRibbonButton : public CBCGPBaseRibbonElement  
{
	friend class CBCGPRibbonPanel;
	friend class CBCGPRibbonBar;
	friend class CBCGPRibbonPanelMenuBar;
	friend class CBCGPToolTipCtrl;
	friend class CBCGPRibbonConstructor;
	friend class CBCGPRibbonTreeCtrl;
	friend class CBCGPRibbonLaunchButton;

	DECLARE_DYNCREATE(CBCGPRibbonButton)

public:
	enum RibbonButtonOnQAT
	{
		BCGPRibbonButton_Show_Default,
		BCGPRibbonButton_Show_As_CheckBox,
		BCGPRibbonButton_Show_As_RadioButton,
	};

// Construction
public:
	CBCGPRibbonButton ();

	CBCGPRibbonButton (
		UINT	nID, 
		LPCTSTR lpszText, 
		int		nSmallImageIndex = -1,
		int		nLargeImageIndex = -1,
		BOOL	bAlwaysShowDescription = FALSE);

	CBCGPRibbonButton (
		UINT	nID, 
		LPCTSTR lpszText, 
		HICON	hIcon,
		BOOL	bAlwaysShowDescription = FALSE,
		HICON	hIconSmall = NULL,
		BOOL	bAutoDestroyIcon = FALSE,
		BOOL	bAlphaBlendIcon = FALSE);

	CBCGPRibbonButton (
		UINT	nID, 
		HICON	hIconSmall,
		LPCTSTR lpszText, 
		BOOL	bAutoDestroyIcon = FALSE,
		BOOL	bAlphaBlendIcon = FALSE);

	virtual ~CBCGPRibbonButton();

// Attributes
public:
	virtual void SetText (LPCTSTR lpszText);
	virtual void SetDescription (LPCTSTR lpszText);

	virtual BOOL IsDefaultPanelButton () const
	{
		return FALSE;
	}

	virtual BOOL IsMainRibbonButton () const
	{
		return FALSE;
	}

	void SetMenu (	HMENU hMenu, 
					BOOL bIsDefaultCommand = FALSE, 
					BOOL bRightAlign = FALSE);

	void SetMenu (	UINT uiMenuResID, 
					BOOL bIsDefaultCommand = FALSE,
					BOOL bRightAlign = FALSE);

	HMENU GetMenu () const
	{
		return m_hMenu;
	}

	void SetDefaultCommand (BOOL bSet = TRUE)
	{
		m_bIsDefaultCommand = bSet;
	}

	BOOL IsDefaultCommand () const
	{
		return m_bIsDefaultCommand;
	}

	void SetRightAlignMenu (BOOL bSet = TRUE)
	{
		m_bRightAlignMenu = bSet;
	}

	BOOL IsRightAlignMenu () const
	{
		return m_bRightAlignMenu;
	}

	BOOL IsAlwaysShowDescription () const
	{
		return m_bAlwaysShowDescription;
	}

	virtual BOOL HasMenu () const
	{
		return (m_hMenu != NULL || m_arSubItems.GetSize () > 0) && !IsBackstageViewMode();
	}

	virtual BOOL HasSubitems () const
	{
		return m_arSubItems.GetSize () > 0;
	}

	virtual BOOL IsCommandAreaHighlighted () const
	{
		return m_bIsCommandHighlighted && IsHighlighted ();
	}

	virtual BOOL IsMenuAreaHighlighted () const
	{
		return m_bIsMenuHighlighted && IsHighlighted ();
	}

	CRect GetMenuRect () const
	{
		return m_rectMenu;
	}

	CRect GetCommandRect () const
	{
		return m_rectCommand;
	}

	BOOL IsMenuOnBottom () const
	{
		return m_bMenuOnBottom;
	}

	virtual BOOL IsAlwaysDrawBorder () const
	{
		return m_bForceDrawBorder;
	}

	virtual BOOL CanBeStretched ();

	BOOL IsSingleLineText () const
	{
		return m_sizeTextBottom.cy == m_sizeTextRight.cy;
	}

	int GetTextRowHeight () const
	{
		return m_sizeTextRight.cy;
	}

	const CArray<CBCGPBaseRibbonElement*, CBCGPBaseRibbonElement*>&
		GetSubItems () const
	{
		return m_arSubItems;
	}

	virtual CBCGPBaseRibbonElement* GetBackstageAttachedView()
	{
		if (!IsBackstageViewMode() || m_arSubItems.GetSize() == 0)
		{
			return NULL;
		}

		return m_arSubItems[0];
	}

	int	GetImageIndex (BOOL bLargeImage) const
	{
		return bLargeImage ? m_nLargeImageIndex : m_nSmallImageIndex;
	}

	void SetImageIndex (int nIndex, BOOL bLargeImage)
	{
		if (bLargeImage)
		{
			m_nLargeImageIndex = nIndex;
		}
		else
		{
			m_nSmallImageIndex = nIndex;
		}
	}

	BOOL IsLargeImage () const
	{
		return m_bIsLargeImage;
	}

	HICON GetIcon (BOOL bLargeIcon = TRUE) const
	{
		return bLargeIcon ? m_hIcon : m_hIconSmall;
	}

	void SetIcon (HICON hIcon, BOOL bLargeIcon = TRUE, BOOL	bAutoDestroyIcon = FALSE, BOOL bAlphaBlendIcon = FALSE);

	virtual BOOL IsAlwaysLargeImage () const
	{
		return m_bIsAlwaysLarge || (m_nLargeImageIndex >= 0 && m_nSmallImageIndex < 0);
	}

	void SetAlwaysLargeImage (BOOL bSet = TRUE)
	{
		m_bIsAlwaysLarge = bSet;
	}

	static BOOL m_bUseMenuHandle;

	RibbonButtonOnQAT GetQATType() const
	{
		return m_QATType;
	}

	void SetQATType(RibbonButtonOnQAT type)
	{
		m_QATType = type;
	}

// Operations
public:
	void AddSubItem (CBCGPBaseRibbonElement* pSubItem, int nIndex = -1);
	int FindSubItemIndexByID (UINT uiID) const;
	BOOL RemoveSubItem (int nIndex);
	void RemoveAllSubItems ();

	CSize DrawBottomText (CDC* pDC, BOOL bCalcOnly);

protected:
	void CommonInit ();
	void FillWindowList ();
	int GetGroupButtonExtraWidth ();

// Overrides
public:
	virtual void OnDraw (CDC* pDC);

	virtual CSize GetRegularSize (CDC* pDC);
	virtual CSize GetCompactSize (CDC* pDC);
	virtual CSize GetIntermediateSize (CDC* pDC);

	virtual BOOL HasLargeMode () const
	{
		return !m_bIsCustom && GetImageSize (RibbonImageLarge) != CSize (0, 0);
	}

	virtual BOOL HasIntermediateMode () const
	{
		return !m_strText.IsEmpty ();
	}

	virtual BOOL HasCompactMode () const
	{
		return GetImageSize (RibbonImageSmall) != CSize (0, 0);
	}

	virtual void DrawImage (CDC* pDC, RibbonImageType type, CRect rectImage);
	virtual CSize GetImageSize (RibbonImageType type) const;
	virtual HICON ExportImageToIcon(BOOL bIsLargeIcon);

	virtual void OnCalcTextSize (CDC* pDC);
	virtual void CleanUpSizes ()
	{
		m_sizeTextRight = m_sizeTextBottom = CSize (0, 0);
	}

	virtual void SetParentCategory (CBCGPRibbonCategory* pParent);
	virtual void SetParentMenu (CBCGPRibbonPanelMenuBar* pMenuBar);

	virtual void ClosePopupMenu ();
	virtual CString GetToolTipText () const;

	virtual void OnClick (CPoint point);
	virtual void OnAccDefaultAction();

	virtual BOOL IsDrawTooltipImage () const			{	return TRUE;	}
	virtual CSize GetToolTipImageSize(int& nRibbonImageType) const;
	virtual void OnDrawTooltipImage(CDC* pDC, RibbonImageType type, const CRect& rectImage);

	virtual BOOL IsPopupDefaultMenuLook () const		{	return TRUE;	}

	virtual COLORREF OnFillBackground (CDC* pDC);
	virtual void OnDrawBorder (CDC* pDC);

	virtual BOOL HasImage (RibbonImageType type) const;

	virtual BOOL ReplaceSubitemByID (UINT uiCmdID, CBCGPBaseRibbonElement* pElem, BOOL bCopyContent);

protected:
	virtual void OnDrawMenuArrow(CDC* pDC, const CRect& rectMenuArrow);
	virtual void OnDrawOnList (CDC* pDC, CString strText, int nTextOffset, CRect rect, BOOL bIsSelected, BOOL bHighlighted);
	virtual int AddToListBox (CBCGPRibbonCommandsListBox* pWndListBox, BOOL bDeep);
	virtual CBCGPGridRow* AddToTree (CBCGPGridCtrl* pGrid, CBCGPGridRow* pParent);

	virtual void OnLButtonDown (CPoint point);
	virtual void OnLButtonUp (CPoint point);
	virtual void OnMouseMove (CPoint point);

	virtual void OnShowPopupMenu ();

	virtual void CopyFrom (const CBCGPBaseRibbonElement& src);
	virtual void CopyBaseFrom (const CBCGPBaseRibbonElement& src);
	virtual void SetOriginal (CBCGPBaseRibbonElement* pOriginal);

	virtual CBCGPBaseRibbonElement* FindByID (UINT uiCmdID);
	virtual CBCGPBaseRibbonElement* FindByIDNonCustom (UINT uiCmdID);
	virtual CBCGPBaseRibbonElement* FindByData (DWORD_PTR dwData);

	virtual void GetElementsByID (UINT uiCmdID, 
		CArray<CBCGPBaseRibbonElement*, CBCGPBaseRibbonElement*>& arElements);

	virtual void GetElements (
		CArray<CBCGPBaseRibbonElement*, CBCGPBaseRibbonElement*>& arElements);

	virtual void GetElementsByName (LPCTSTR lpszName, 
		CArray<CBCGPBaseRibbonElement*, CBCGPBaseRibbonElement*>& arButtons, DWORD dwFlags = 0);

	virtual void GetItemIDsList (CList<UINT,UINT>& lstItems) const;

	virtual int GetTextOffset () const	{	return IsBackstageViewMode() ? m_szMargin.cx / 3 : m_szMargin.cx;	}

	virtual void SetParentRibbonBar (CBCGPRibbonBar* pRibbonBar);
	virtual CRect GetKeyTipRect (CDC* pDC, BOOL bIsMenu);
	virtual BOOL OnKey (BOOL bIsMenuKey);

	virtual void OnAfterChangeRect (CDC* pDC);

	virtual void SetMargin (const CSize& sizeMargiin)
	{
		m_szMargin = sizeMargiin;
	}

	virtual BOOL SetACCData (CWnd* pParent, CBCGPAccessibilityData& data);

	virtual void OnChangeVisualManager();

	virtual CBCGPBaseRibbonElement* CreateQATCopy();

	virtual void SetCustom();
	virtual void SetPadding(const CSize& sizePadding);

// Attributes
protected:
	CSize	m_sizeTextRight;
	CSize	m_sizeTextBottom;

	int		m_nSmallImageIndex;
	int		m_nLargeImageIndex;

	HMENU	m_hMenu;
	BOOL	m_bRightAlignMenu;
	BOOL	m_bIsDefaultCommand;
	int		m_nMenuArrowMargin;
	BOOL	m_bAutodestroyMenu;

	CSize	m_szMargin;

	CRect	m_rectMenu;
	CRect	m_rectCommand;

	BOOL	m_bMenuOnBottom;
	BOOL	m_bIsMenuHighlighted;
	BOOL	m_bIsCommandHighlighted;

	HICON	m_hIcon;
	HICON	m_hIconSmall;
	BOOL	m_bAutoDestroyIcon;
	BOOL	m_bAlphaBlendIcon;
	BOOL	m_bSmallIconMode;
	BOOL	m_bForceDrawBorder;

	BOOL	m_bToBeClosed;
	BOOL	m_bAlwaysShowDescription;

	BOOL	m_bIsLargeImage;

	BOOL	m_bCreatedFromMenu;
	BOOL	m_bIsWindowsMenu;
	int		m_nWindowsMenuItems;

	CArray<int,int>	m_arWordIndexes;
	int		m_nWrapIndex;

	CArray<CBCGPBaseRibbonElement*, CBCGPBaseRibbonElement*>	m_arSubItems;

	RibbonButtonOnQAT	m_QATType;
};

#endif // BCGP_EXCLUDE_RIBBON

#endif // !defined(AFX_BCGPRIBBONBUTTON_H__3698E9F7_DEB7_43C4_90C0_7973B7FF0A8F__INCLUDED_)
