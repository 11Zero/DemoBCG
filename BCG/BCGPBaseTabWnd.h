#if !defined(AFX_BCGPBASETABWND_H__742253C1_CFB3_11D5_A305_444553540000__INCLUDED_)
#define AFX_BCGPBASETABWND_H__742253C1_CFB3_11D5_A305_444553540000__INCLUDED_

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
// BCGPBaseTabWnd.h : header file
//

#ifndef __AFXTEMPL_H__
	#include "afxtempl.h"
#endif

#include "BCGCBPro.h"
#include "BCGPWnd.h"
#include "BCGPBaseControlBar.h"

extern BCGCBPRODLLEXPORT UINT BCGM_ON_RENAME_TAB;
extern BCGCBPRODLLEXPORT UINT BCGM_ON_MOVE_TAB;
extern BCGCBPRODLLEXPORT UINT BCGM_CHANGE_ACTIVE_TAB;
extern BCGCBPRODLLEXPORT UINT BCGM_CHANGING_ACTIVE_TAB;
extern BCGCBPRODLLEXPORT UINT BCGM_ON_GET_TAB_TOOLTIP;
extern BCGCBPRODLLEXPORT UINT BCGM_NEW_TAB;

/////////////////////////////////////////////////////////////////////////////
// CBCGPTabDropTarget command target

class CBCGPBaseTabWnd;

class BCGCBPRODLLEXPORT CBCGPTabDropTarget : public COleDropTarget
{
public:
	CBCGPTabDropTarget()
	{
		m_pOwner = NULL;
	}

// Attributes
protected:
	CBCGPBaseTabWnd* m_pOwner;

// Operations
public:
	BOOL Register (CBCGPBaseTabWnd *pOwner);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPTabDropTarget)
	public:
	virtual DROPEFFECT OnDragEnter(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	virtual void OnDragLeave(CWnd* pWnd);
	virtual DROPEFFECT OnDragOver(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	virtual DROPEFFECT OnDropEx(CWnd* pWnd, COleDataObject* pDataObject, DROPEFFECT dropEffect, DROPEFFECT dropList, CPoint point);
	//}}AFX_VIRTUAL
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPTabInfo

class BCGCBPRODLLEXPORT CBCGPTabInfo : public CObject
{
	friend class CBCGPTabWnd;
	friend class CBCGPBaseTabWnd;
	friend class CBCGPOutlookWnd;

protected:

	CBCGPTabInfo (	const CString&	strText,
					const UINT		uiIcon,
					CWnd*			pWnd,
					const int		iTabID, 
					BOOL			bDetachable) :
		m_pWnd (pWnd),
		m_uiIcon (uiIcon),
		m_iTabID (iTabID),
		m_bIsDetachable (bDetachable)
	{
		m_strText = strText;
		m_rect.SetRectEmpty ();
		m_bVisible = TRUE;
		m_nFullWidth = 0;
		m_hIcon = NULL;
		m_hGrayIcon = NULL;
		m_bIconOnly = FALSE;
		m_bAlwaysShowToolTip = FALSE;
		m_bIsNewTab = FALSE;

		m_clrText = (COLORREF) -1;
		m_clrBack = (COLORREF) -1;

		m_rectClose.SetRectEmpty();
		m_bIsCloseHighlighted = FALSE;
		m_bIsClosePressed = FALSE;

		if (m_pWnd != NULL)
		{
			TCHAR szClass [256];
			::GetClassName (m_pWnd->GetSafeHwnd (), szClass, 255);

			CString strClass = szClass;
			m_bIsListView = (strClass == _T("SysListView32"));
		}
		else
		{
			m_bIsListView = FALSE;
		}
	}

	~CBCGPTabInfo()
	{
		if (m_hIcon != NULL)
		{
			::DestroyIcon (m_hIcon);
		}

		if (m_hGrayIcon != NULL)
		{
			::DestroyIcon(m_hGrayIcon);
		}
	}

	CString		m_strText;
	UINT		m_uiIcon;
	HICON		m_hIcon;
	HICON		m_hGrayIcon;
	CRect		m_rect;
	BOOL		m_bVisible;
	CWnd*		m_pWnd;
	BOOL		m_bIsListView;
	int			m_nFullWidth;
	const int	m_iTabID;

	COLORREF	m_clrText;
	COLORREF	m_clrBack;

	BOOL		m_bIconOnly;
	BOOL		m_bAlwaysShowToolTip;

	BOOL		m_bIsDetachable;

	BOOL		m_bIsNewTab;

	CRect		m_rectClose;
	BOOL		m_bIsCloseHighlighted;
	BOOL		m_bIsClosePressed;

public:
	CRect GetRect () const		{	return m_rect;	}
	void SetRect (CRect rect)	{	m_rect = rect;	} 
};

struct CBCGPRestoredTabInfo
{
	CString		m_strText;
	BOOL		m_bVisible;
	int			m_nControlBarID;
	BOOL		m_bDetachable;
	COLORREF	m_clrText;
	COLORREF	m_clrBack;
};

struct BCGCBPRODLLEXPORT CBCGPTabToolTipInfo
{
	CString				m_strText;
	CBCGPBaseTabWnd*	m_pTabWnd;
	int					m_nTabIndex;
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPBaseTabWnd window

class BCGCBPRODLLEXPORT CBCGPBaseTabWnd : public CBCGPWnd
{
	friend class CBCGPOutlookBar;
	friend class CBCGPToolBox;

	DECLARE_DYNAMIC(CBCGPBaseTabWnd)

// Construction
public:
	CBCGPBaseTabWnd();

	enum Location
	{
		LOCATION_BOTTOM = 0,
		LOCATION_TOP = 1
	};

// Attributes
public:
	virtual BOOL IsPtInTabArea (CPoint point) const = 0;
	virtual void GetTabArea (CRect& rectTabAreaTop, CRect& rectTabAreaBottom) const = 0;

	static int TAB_TEXT_MARGIN;
	static int TAB_IMAGE_MARGIN;

// Operations
public:
	virtual BOOL SetImageList (UINT uiID, int cx = 15, 
								COLORREF clrTransp = RGB (255, 0, 255));
	virtual BOOL SetImageList (HIMAGELIST hImageList);
	virtual void ClearImageList ();
	
	virtual void SetTabsHeight ();
	virtual int GetTabsHeight () const
	{
		return m_nTabsHeight;
	}

	virtual void GetTabsRect (CRect& rect) const
	{
		rect.SetRectEmpty ();
	}

	virtual void RecalcLayout () = 0;
	
	// Tab access:
	virtual void AddTab (CWnd* pTabWnd, LPCTSTR lpszTabLabel, UINT uiImageId = (UINT)-1, BOOL bDetachable = TRUE);
	virtual void AddTab (CWnd* pTabWnd, UINT uiResTabLabel, UINT uiImageId = (UINT)-1, BOOL bDetachable = TRUE);

	virtual void InsertTab (CWnd* pNewWnd, LPCTSTR lpszTabLabel, int nInsertAt, UINT uiImageId = (UINT)-1, BOOL bDetachable = TRUE);
	virtual void InsertTab (CWnd* pNewWnd, UINT uiResTabLabel, int nInsertAt, UINT uiImageId = (UINT)-1, BOOL bDetachable = TRUE);

	virtual BOOL RemoveTab (int iTab, BOOL bRecalcLayout = TRUE);
	virtual void RemoveAllTabs ();

	virtual BOOL DetachTab (BCGP_DOCK_METHOD dockMethod, int nTabNum = -1, 
							BOOL bHide = FALSE);

	virtual int GetVisibleTabsNum () const;
	virtual BOOL ShowTab (int iTab, BOOL bShow = TRUE, BOOL bRecalcLayout = TRUE, BOOL bActivate = FALSE);

	virtual int GetTabsNum () const
	{
		return m_iTabsNum;
	}

	// Tab properties:
	virtual CWnd* GetTabWnd (int iTab) const;
	virtual CWnd* GetTabWndNoWrapper (int iTab) const;
	virtual BOOL GetTabRect(int iTab, CRect& rect) const;
	virtual BOOL GetTabCloseButtonRect(int iTab, CRect& rect, BOOL* pbIsHighlighted = NULL, BOOL* pbIsPressed = NULL) const;
	int GetTabID (int iTab) const;

	BOOL IsTabNew(int iTab) const;
	BOOL IsFirstTab(int iTab) const;

	virtual BOOL GetTabLabel (int iTab, CString& strLabel) const;
	virtual BOOL SetTabLabel (int iTab, const CString& strLabel);

	virtual UINT GetTabIcon (int iTab) const;
	virtual BOOL SetTabIcon (int iTab, UINT uiIcon);

	virtual BOOL IsTabIconOnly(int iTab) const;
	virtual BOOL SetTabIconOnly(int iTab, BOOL bIconOnly = TRUE, BOOL bShowTooltipAlways = FALSE);

	virtual BOOL SetTabHicon (int iTab, HICON hIcon);
	virtual HICON GetTabHicon (int iTab) const;

	virtual COLORREF GetTabBkColor (int iTab) const;
	virtual BOOL SetTabBkColor (int iTab, COLORREF color = (COLORREF)-1);

	virtual COLORREF GetTabTextColor (int iTab) const;
	virtual BOOL SetTabTextColor (int iTab, COLORREF color = (COLORREF)-1);

	virtual int GetTabFullWidth (int iTab) const;

	void EnableAutoColor (BOOL bEnable = TRUE);
	BOOL IsAutoColor () const
	{
		return m_bIsAutoColor;
	}

	virtual void EnableNewTab(BOOL /*bEnable*/ = TRUE) {}
	virtual BOOL IsNewTabEnabled() const
	{
		return FALSE;
	}

	void SetAutoColors (const CArray<COLORREF, COLORREF>& arColors);
	const CArray<COLORREF, COLORREF>& GetAutoColors () const
	{
		return m_arAutoColors;
	}

	virtual BOOL IsTabDetachable (int iTab) const;
	virtual BOOL EnableTabDetach (int iTab, BOOL bEnable);

	virtual BOOL StartRenameTab (int iTab);
	virtual BOOL RenameTab ();
	virtual BOOL OnRenameTab (int /*iTab*/, CString& /*strName*/)
	{
		return TRUE;
	}

	virtual void EnableInPlaceEdit (BOOL bEnable) = 0;
	virtual void CalcRectEdit (CRect& rectEdit) = 0;
 
	void SetDrawNoPrefix (BOOL bNoPrefix, BOOL bRedraw = TRUE);
	BOOL IsDrawNoPrefix () const
	{
		return m_bLabelNoPrefix;
	}

	void SetDrawNameInUpperCase(BOOL bSet = TRUE);
	BOOL IsDrawNameInUpperCase() const
	{
		return m_bDrawNameInUpperCase;
	}

	// Tab activation:
	virtual int GetActiveTab () const
	{
		return m_iActiveTab;
	}

	virtual CWnd* GetActiveWnd () const;
	virtual BOOL SetActiveTab (int iTab) = 0;

	virtual int GetTabFromPoint (CPoint& pt) const;
	virtual int GetTabFromHwnd (HWND hwnd) const;

	virtual int GetTabByID (int id) const;

	virtual BOOL EnsureVisible (int /*iTab*/)	{	return FALSE;	}

	// Active tab color operations:
	virtual void SetActiveTabColor (COLORREF clr);
	virtual void SetActiveTabTextColor (COLORREF clr);

	virtual COLORREF GetActiveTabColor () const
	{
		return m_clrActiveTabBk == (COLORREF) -1 ?
			globalData.clrWindow : m_clrActiveTabBk;
	}

	virtual COLORREF GetActiveTabTextColor () const
	{
		return m_clrActiveTabFg == (COLORREF) -1 ?
			globalData.clrWindowText : m_clrActiveTabFg;
	}

	virtual	CSize GetImageSize () const
	{
		return m_sizeImage;
	}

	virtual const CImageList* GetImageList () const;

	virtual BOOL HasImage (int iTab) const;

	virtual void SetTabBorderSize (int nTabBorderSize, BOOL bRepaint = TRUE);

	virtual int GetTabBorderSize () const
	{
		return GetTabsHeight () > 0 ? m_nTabBorderSize : 0;
	}

	virtual CWnd* FindTargetWnd (const CPoint& pt) = 0;

	void InvalidateTab (int iTab);

	virtual void Serialize (CArchive& ar);
	virtual void ApplyRestoredTabInfo (BOOL bUseTabIndexes = FALSE);
	virtual CWnd* GetFirstVisibleTab (int& iTabNum);
	virtual CWnd* GetFirstVisibleTab (int iStartFrom, int& iTabNum);
	virtual CWnd* GetLastVisibleTab (int& iTabNum);

	void EnableTabSwap (BOOL bEnable) {m_bEnableTabSwap = bEnable;}
	BOOL IsTabSwapEnabled	() const {return m_bEnableTabSwap;}

	void SetDontActivateView (BOOL bSet)	{	m_bDontActivateView = bSet;	}
	BOOL IsActivateView	()					{	return m_bDontActivateView;	}

	virtual void SwapTabs	(int nFisrtTabID, int nSecondTabID);
	virtual void MoveTab	(int nSource, int nDest);
	

	BOOL SetTabsOrder (const CArray<int,int>& arOrder);

	void EnterDragMode ();

	virtual CSize GetMaxWindowSize () const;

	void AutoDestroyWindow (BOOL bAutoDestroy = TRUE)
	{
		m_bAutoDestoyWindow = bAutoDestroy;
	}

	BOOL IsAutoDestroyWindow () const {return m_bAutoDestoyWindow;}

	void EnableActivateLastActive(BOOL bLastActive = TRUE)
	{
		m_bActivateLastActiveTab = bLastActive;
	}

	virtual void OnChangeTabs () {}
	virtual void FireChangeActiveTab (int nNewTab);
	virtual BOOL FireChangingActiveTab (int nNewTab);
	virtual BOOL FireNewTab ();

	virtual void OnClickCloseButton (CWnd* pWndActive);

	void ResetImageList ();
	BOOL IsIconAdded (HICON hIcon, int& iIcon) 
	{
		return (m_mapAddedIcons.Lookup (hIcon, iIcon));
	}

	void AddIcon (HICON hIcon, int iIcon) {m_mapAddedIcons.SetAt (hIcon, iIcon);}
	
	BOOL EnableCustomToolTips (BOOL bEnable = TRUE);

	virtual BOOL IsActiveInMDITabGroup () const { return FALSE; }
	virtual BOOL IsMDIFocused() const { return FALSE; }

	void SetParentFocused(BOOL bSet) 
	{ 
		m_bIsParentFocused = bSet;
	}

	BOOL IsParentFocused() const { return m_bIsParentFocused; }

// Attributes
public:
	Location GetLocation () const
	{
		return m_location;
	}

	virtual void SetLocation (Location location);

	virtual BOOL IsTabVisible (int iTab) const;	
	virtual BOOL IsFlatTab () const		{return FALSE;}
	virtual BOOL IsActiveTabCloseButton () const	{	return FALSE;	}
	virtual BOOL IsTabCloseButton() const { return FALSE; }
	virtual BOOL IsOneNoteStyle () const{return FALSE;}
	virtual BOOL Is3DStyle () const{return FALSE;}
	virtual BOOL IsVS2005Style () const	{return FALSE;}
	virtual BOOL IsLeftRightRounded () const	{return FALSE;}
	virtual BOOL IsPointerStyle() const	{return FALSE;}
	virtual BOOL IsFlatFrame () const	{return FALSE;}
	virtual BOOL IsInPlaceEdit () const {return m_bIsInPlaceEdit;}
	virtual int	GetFirstVisibleTabNum () const	{	return -1;	}
	virtual BOOL IsRestoreTabAttributes() const {	return TRUE;	}

	virtual BOOL IsHideSingleTab () const	{return m_bHideSingleTab;}
	virtual void HideSingleTab (BOOL bHide = TRUE);

	BOOL IsSingleTabStatic() const { return m_bSingleTabIsStatic; }
	void SetSingleTabStatic(BOOL bSet = TRUE);

	int GetHighlightedTab () const
	{
		return m_iHighlighted;
	}

	BOOL IsDialogControl () const
	{
		return m_bIsDlgControl;
	}

	virtual BOOL IsColored () const;

	BOOL IsMDITab () const { return m_bIsMDITab; }
	BOOL IsPropertySheetTab() const { return m_bIsPropertySheetTab; }

	CToolTipCtrl& GetToolTipCtrl () const
	{
		if (m_pToolTip == NULL)
		{
			static CToolTipCtrl ttEmpty;
			return ttEmpty;
		}

		ASSERT_VALID (m_pToolTip);
		return (CToolTipCtrl&)*m_pToolTip;
	}

	virtual void OnChangeHighlightedTab(int /* iPrevHighlighted */) {}

	BOOL IsCaptionFont() const	{return m_bCaptionFont;}
	void SetCaptionFont(BOOL bSet = TRUE);

	// detachable tabs support
	void SetDockingBarWrapperRTC (CRuntimeClass* pRTC) {m_pDockingBarWrapperRTC = pRTC;}
	BOOL			m_bEnableWrapping;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPBaseTabWnd)
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

	virtual BOOL OnDrop(COleDataObject* /*pDataObject*/, DROPEFFECT /*dropEffect*/, CPoint /*point*/)
	{
		return FALSE;
	}

	virtual DROPEFFECT OnDragEnter(COleDataObject* /*pDataObject*/, DWORD /*dwKeyState*/, CPoint /*point*/)
	{
		return DROPEFFECT_NONE;
	}

	virtual void OnDragLeave()	{}
	virtual DROPEFFECT OnDragOver(COleDataObject* /*pDataObject*/, DWORD /*dwKeyState*/, CPoint /*point*/)
	{
		return DROPEFFECT_NONE;
	}

	BOOL	m_bActivateLastVisibleTab;
	BOOL	m_bHighLightTabs;
	BOOL	m_bActivateTabOnRightClick;

// Implementation
public:
	virtual ~CBCGPBaseTabWnd();
	virtual void CleanUp ();
	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPBaseTabWnd)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnCancelMode();
	afx_msg void OnDestroy();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSysColorChange();
	afx_msg void OnWindowPosChanged(WINDOWPOS FAR* lpwndpos);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
	//}}AFX_MSG
	afx_msg BOOL OnNeedTipText(UINT id, NMHDR* pNMH, LRESULT* pResult);
	afx_msg LRESULT OnSetFont(WPARAM, LPARAM);
	afx_msg LRESULT OnGetFont(WPARAM, LPARAM);
	DECLARE_MESSAGE_MAP()

	virtual int GetTabNumberToDetach (int nTabNum = -1) const;
	virtual CWnd* CreateWrapper (CWnd* pWndToWrap, LPCTSTR lpszTabLabel, BOOL bDetachable);

	virtual void InitAutoColors ();

	virtual BOOL ActivateOnBtnUp () const
	{
		return m_bActivateOnBtnUp;
	}

	virtual BOOL OnSetAccData (long lVal);
	virtual HRESULT get_accChildCount(long *pcountChildren);
	virtual HRESULT get_accChild(VARIANT varChild, IDispatch **ppdispChild);
	virtual HRESULT accHitTest(long xLeft, long yTop, VARIANT *pvarChild);
	virtual HRESULT accNavigate(long navDir, VARIANT varStart, VARIANT *pvarEndUpAt);
	virtual HRESULT accDoDefaultAction(VARIANT varChild);
    virtual HRESULT get_accRole(VARIANT varChild, VARIANT *pvarRole);
	virtual HRESULT get_accValue(VARIANT varChild, BSTR *pszValue);
	virtual HRESULT get_accState(VARIANT varChild, VARIANT *pvarState);

	virtual BOOL SetACCData (CBCGPTabInfo* pTab, CBCGPAccessibilityData& data, BOOL bIsActive);

	virtual CFont* GetTabFont();
	virtual int GetTextHeight(int& nTextMargin);

	virtual void OnCustomFontChanged() {}

protected:
	int CBCGPBaseTabWnd::FindTabInfo (int nBarID, CBCGPTabInfo** ppTabInfo);

	Location		m_location;			// Tabs location

	CPtrArray		m_arTabs;			// Array of CBCGTabInfo objects
	CArray<int,int>	m_arTabIndexs;		// Array of tab indexes
	int				m_iTabsNum;			// m_arTabs size
	int				m_iActiveTab;		// Active tab number
	int             m_iTabBeforeDrag;   // tab before drag

	CImageList		m_Images;			// Tab images
	HIMAGELIST		m_hImageList;		// External tab images

	CImageList		m_ImagesGray;		// Tab images in grayscale

	CSize			m_sizeImage;		// Tab image size

	CMap<HICON,HICON,int,int> m_mapAddedIcons;	// Icons already loaded into the image list

	CToolTipCtrl*	m_pToolTip;
	CToolTipCtrl*	m_pToolTipClose;			// Tooltip for active tab "close" button
	BOOL			m_bCustomToolTips;

	int				m_iCurTab;
	int				m_nNextTabID;

	int				m_nTabBorderSize;
	int				m_nTabsHeight;		// Tabs area height

	BOOL			m_bAutoDestoyWindow;// Auto-destroy tab windows

	BOOL			m_bHideInactiveWnd;	// Is inactive window should be invisible?

	int				m_iHighlighted;		// Highlighted tab number
	int				m_iPressed;			// Pressed tab number

	BOOL			m_bActivateOnBtnUp;	// Active tab changed on WM_LBUTTONUP

	BOOL			m_bEnableTabSwap;
	BOOL			m_bDontActivateView;

	BOOL			m_bHideSingleTab;	// Hide tabs when only one tab is available
	BOOL			m_bSingleTabIsStatic;// When only one tab is available, make it "static"

	COLORREF		m_clrActiveTabBk;	// Active tab backgound color
	COLORREF		m_clrActiveTabFg;	// Active tab foreground color

	CBrush			m_brActiveTab;		// Active tab background brush

	CFont			m_fntTabs;			// Tab font (flat tabs only)
	CFont			m_fntTabsBold;		// Active tab font (flat tabs only)
	HFONT			m_hFontCustom;		// Custom tab font
	BOOL			m_bCaptionFont;		// Use large (caption) font

	BOOL			m_bLabelNoPrefix;	// Draw tab label with DT_NOPREFIX flag

	BOOL			m_bDrawNameInUpperCase;

	BOOL			m_bIsDlgControl;	// Tab is created on the dialog

	BOOL			m_bIsMDITab;		// Tab is created for switching MDI windows
	BOOL			m_bIsPropertySheetTab;	// Tab is a property sheet tab
	BOOL			m_bIsParentFocused;

	// in-place editing
	int				m_iEditedTab;
	CEdit*			m_pInPlaceEdit;
	BOOL			m_bIsInPlaceEdit;

	///////////////////////////
	/// detachable bars support
	///////////////////////////
	BOOL			m_bReadyToDetach;
	CPoint			m_ptHot;
	int				m_nOffsetFromTabLeft;

	// save/restore state support
	CList<CBCGPRestoredTabInfo, CBCGPRestoredTabInfo>	m_lstRestoredTabInfo;
	int													m_nRestoredActiveTabID;
	Location											m_locationRestored;

	CRuntimeClass*	m_pDockingBarWrapperRTC;

	// Drag/drop toolbar buttons support:
	CBCGPTabDropTarget	m_DropTarget;

	// Auto-colors support
	CArray<COLORREF, COLORREF>	m_arAutoColors;
	BOOL						m_bIsAutoColor;
	BOOL						m_bIsDefaultAutoColor;

	// VS.NET Activation support
	int							m_iLastActiveTab;
	BOOL						m_bActivateLastActiveTab;

	BOOL						m_bSetActiveTabFired;
	BOOL						m_bSetActiveTabByMouseClick;

	// needed to prevent unnesessary capturing during LButtonDown
	// in case tab activation leads to adjust layout and moving of tab window.
	BOOL						m_bWindowPosChanged;

public:
	BOOL						m_bUserSelectedTab;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPBASETABWND_H__742253C1_CFB3_11D5_A305_444553540000__INCLUDED_)
