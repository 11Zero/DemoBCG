#if !defined(AFX_BCGPTASKSPANE_H__77AE018B_A87F_4AD8_B3CA_9E6A578556F4__INCLUDED_)
#define AFX_BCGPTASKSPANE_H__77AE018B_A87F_4AD8_B3CA_9E6A578556F4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

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
// BCGPTasksPane.h : header file
//

#include "BCGCBPro.h"

#ifndef BCGP_EXCLUDE_TASK_PANE

#include "BCGPDockingControlBar.h"
#include "BCGPCaptionMenuButton.h"
#include "BCGPToolBar.h"
#include "BCGPTaskPaneMiniFrameWnd.h"
#include "BCGPScrollBar.h"

/////////////////////////////////////////////////////////////////////////////
// CBCGPTasksPane additional classes

class CBCGPTasksPane;

class BCGCBPRODLLEXPORT CBCGPTasksPanePage : public CObject
{
public:
	CBCGPTasksPanePage(LPCTSTR lpszName, CBCGPTasksPane *pTaskPane)
	{
		m_strName = lpszName;
		m_pTaskPane = pTaskPane;
	}

	virtual ~CBCGPTasksPanePage ()
	{
		m_pTaskPane = NULL;
	}

	virtual BOOL SetACCData (CWnd* pParent, CBCGPAccessibilityData& data);

	CString			m_strName;
	CBCGPTasksPane*	m_pTaskPane;
};

class BCGCBPRODLLEXPORT CBCGPTasksGroup : public CObject
{
public:
	CBCGPTasksGroup(LPCTSTR lpszName, BOOL bIsBottom, BOOL bIsSpecial = FALSE, 
		BOOL bIsCollapsed = FALSE, CBCGPTasksPanePage* pPage = NULL, HICON hIcon = NULL)
	{
		m_pPage = pPage;
		m_strName =  lpszName;
		m_bIsBottom = bIsBottom;
		m_bIsSpecial = bIsSpecial;
		m_rect.SetRectEmpty ();
		m_rectGroup.SetRectEmpty ();
		m_bIsCollapsed = bIsCollapsed;
		m_hIcon = hIcon;
		m_sizeIcon = CSize(0, 0);

		m_clrText		= (COLORREF)-1;
		m_clrTextHot	= (COLORREF)-1;

		ICONINFO iconInfo;
		::ZeroMemory(&iconInfo, sizeof(iconInfo));
		::GetIconInfo(m_hIcon, &iconInfo);
		
		BITMAP bm;
		::ZeroMemory(&bm, sizeof(bm));
		::GetObject(iconInfo.hbmColor, sizeof(BITMAP), &bm);

		m_sizeIcon = CSize(bm.bmWidth, bm.bmHeight);

		::DeleteObject (iconInfo.hbmColor);
		::DeleteObject (iconInfo.hbmMask);
	}

	virtual ~CBCGPTasksGroup()
	{
		while (!m_lstTasks.IsEmpty ())
		{
			delete m_lstTasks.RemoveHead ();
		}
		m_pPage = NULL;
	}

	virtual BOOL SetACCData (CWnd* pParent, CBCGPAccessibilityData& data);

	CBCGPTasksPanePage*	m_pPage;
	CString	m_strName;
	CObList	m_lstTasks;
	BOOL	m_bIsBottom;
	BOOL	m_bIsSpecial;
	CRect	m_rect;
	CRect	m_rectGroup;
	BOOL	m_bIsCollapsed;
	HICON	m_hIcon;
	CSize	m_sizeIcon;
	COLORREF m_clrText;
	COLORREF m_clrTextHot;
};

class BCGCBPRODLLEXPORT CBCGPTask : public CObject
{
public:
	enum WindowAlign
	{
		BCGP_TASKPANE_WND_STRETCH = 0,
		BCGP_TASKPANE_WND_LEFT,
		BCGP_TASKPANE_WND_RIGHT,
		BCGP_TASKPANE_WND_CENTER,
	};

public:
	CBCGPTask(CBCGPTasksGroup* pGroup, LPCTSTR lpszName, int nIcon, 
			UINT uiCommandID, DWORD_PTR dwUserData = 0,
			HWND hwndTask = NULL, BOOL bAutoDestroyWindow = FALSE, int nWindowHeight = 0,
			WindowAlign align = BCGP_TASKPANE_WND_STRETCH, int nWindowWidth = 0)
	{
		m_pGroup		= pGroup;
		m_strName		= lpszName == NULL ? _T("") : lpszName;
		m_nIcon			= nIcon;
		m_uiCommandID	= uiCommandID;
		m_dwUserData	= dwUserData;
		m_hwndTask		= hwndTask;
		m_bAutoDestroyWindow = bAutoDestroyWindow;
		m_nWindowHeight	= nWindowHeight;
		m_nWindowWidth	= nWindowWidth;
		m_WindowAlign	= align;
		m_bVisible		= TRUE;
		m_bEnabled		= TRUE;
		m_bIsSeparator	= lpszName == NULL;
		m_clrText		= (COLORREF)-1;
		m_clrTextHot	= (COLORREF)-1;
		m_bIsBold		= FALSE;

		m_rect.SetRectEmpty ();
	}

	virtual ~CBCGPTask()
	{
		if (m_hwndTask != NULL && m_bAutoDestroyWindow)
		{
			CWnd* pWnd = CWnd::FromHandlePermanent (m_hwndTask);
			if (pWnd != NULL)
			{
				pWnd->DestroyWindow ();
				delete pWnd;
			}
			else
			{
				::DestroyWindow (m_hwndTask);
			}
		}
		m_pGroup = NULL;
	}

	virtual BOOL SetACCData (CWnd* pParent, CBCGPAccessibilityData& data);

	CBCGPTasksGroup*	m_pGroup;
	CString			m_strName;
	int				m_nIcon;
	UINT			m_uiCommandID;
	DWORD_PTR		m_dwUserData;
	HWND			m_hwndTask;
	BOOL			m_bAutoDestroyWindow;
	CRect			m_rect;
	BOOL			m_bVisible;
	BOOL			m_bEnabled;
	int				m_nWindowHeight;
	int				m_nWindowWidth;
	WindowAlign		m_WindowAlign;
	BOOL			m_bIsSeparator;
	COLORREF		m_clrText;
	COLORREF		m_clrTextHot;
	BOOL			m_bIsBold;		// For labels only
};

class CBCGPTasksPaneToolBar : public CBCGPToolBar
{
	friend class CBCGPTasksPane;

	DECLARE_SERIAL(CBCGPTasksPaneToolBar)

// Overrides
public:
	CBCGPTasksPaneToolBar()
	{
		m_pBtnBack = NULL;
		m_pBtnForward = NULL;
	}

	virtual BOOL AllowShowOnList () const			{	return FALSE;	}
	virtual BOOL AllowShowOnControlMenu () const	{	return FALSE;	}

	virtual BOOL IsButtonExtraSizeAvailable () const
	{
		return FALSE;
	}

	void UpdateMenuButtonText (const CString& str);
	void UpdateButtons ();

protected:
	virtual void AdjustLayout ();
	virtual void AdjustLocations ();
	virtual BOOL OnUserToolTip (CBCGPToolbarButton* pButton, CString& strTTText) const;

	// Generated message map functions
	//{{AFX_MSG(CBCGPTasksPaneToolBar)
	//}}AFX_MSG
	afx_msg LRESULT OnIdleUpdateCmdUI(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

	CBCGPToolbarMenuButton* m_pBtnBack;
	CBCGPToolbarMenuButton* m_pBtnForward;
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPTasksPane window

class BCGCBPRODLLEXPORT CBCGPTasksPane : public CBCGPDockingControlBar
{
	friend CBCGPTasksPaneToolBar;
	DECLARE_SERIAL(CBCGPTasksPane);

// Construction
public:
	CBCGPTasksPane();

// Attributes
protected:
	CObList			m_lstTasksPanes;
	CArray<int, int> m_arrHistoryStack;
	int				m_iActivePage;
	const int		m_nMaxHistory;
	
	CObList			m_lstTaskGroups;
	CBCGPTask*		m_pHotTask;
	CBCGPTask*		m_pClickedTask;
	CBCGPTask*		m_pFocusedTask;
	CBCGPTasksGroup*	m_pHotGroupCaption;
	CBCGPTasksGroup*	m_pClickedGroupCaption;
	CBCGPTasksGroup*	m_pFocusedGroupCaption;

	CString			m_strCaption;

	HFONT			m_hFont;
	CBCGPScrollBar	m_wndScrollVert;		// Vertical scroll bar
	CFont			m_fontBold;
	CFont			m_fontBoldUnderline;
	CFont			m_fontUnderline;
	CImageList		m_lstIcons;
	CSize			m_sizeIcon;
	BOOL			m_bCanCollapse;

	CRect			m_rectTasks;

	BOOL				m_bUseNavigationToolbar;
	BOOL				m_bHistoryMenuButtons;
	CBCGPTasksPaneToolBar	m_wndToolBar;
	UINT				m_uiToolbarBmpRes;
	CSize				m_sizeToolbarImage;
	CSize				m_sizeToolbarButton;
	CRect				m_rectToolbar;

	BOOL			m_bUseScrollButtons;
	CRect			m_rectScrollUp;
	CRect			m_rectScrollDn;
	int				m_iScrollMode;	// -1 - Up, 0 - None, 1 - Down
	int				m_iScrollBtnHeight;

	int				m_nVertScrollOffset;
	int				m_nVertScrollTotal;
	int				m_nVertScrollPage;
	int				m_nRowHeight;
	
	BOOL			m_bAnimationEnabled;
	CBCGPTasksGroup* m_pAnimatedGroup;
	CSize			m_sizeAnim;
	int				m_nAnimGroupExtraHeight;

	int m_nVertMargin;
	int m_nHorzMargin;
	int m_nGroupVertOffset;
	int m_nGroupCaptionHeight;
	int m_nGroupCaptionHorzOffset;
	int m_nGroupCaptionVertOffset;
	int m_nTaskMinHeight;
	int m_nTasksVertMargin;
	int m_nTasksHorzOffset;
	int m_nTasksIconHorzOffset;
	int m_nTasksIconVertOffset;
	BOOL m_bOffsetCustomControls;

	CMenu			m_menuOther;
	BOOL			m_bMenuBtnPressed;

	BOOL			m_bWrapTasks;
	BOOL			m_bWrapLabels;

	BOOL			m_bPrecessAccFocusEvent;

	CBCGPScrollBar::BCGPSB_STYLE m_ScrollBarStyle;			// Scroll bars style

	static clock_t m_nLastAnimTime;
	static const int m_iAnimTimerDuration;
	static const int m_iScrollTimerDuration;
	static const LONG m_lAccUseCursorPosValue;
	static const LONG m_lAccUseFocus;

// Operations
public:
	BOOL SetIconsList (UINT uiImageListResID, int cx, COLORREF clrTransparent = RGB (255, 0, 255));
	void SetIconsList (HIMAGELIST hIcons);

	void RecalcLayout (BOOL bRedraw = TRUE);

	// --------------
	// Pages support:
	// --------------
	int AddPage (LPCTSTR lpszPageLabel);
	void RemovePage (int nPageIdx);
	void RemoveAllPages ();
	
	int GetPagesCount() const 
	{ 
		return (int) m_lstTasksPanes.GetCount();
	}

	void SetActivePage (int nPageIdx);

	int GetActivePage () const
	{
		return m_arrHistoryStack[m_iActivePage];
	}

	void GetPreviousPages (CStringList& lstPrevPages) const;
	void GetNextPages (CStringList&  lstNextPages) const;

	void SetCaption (LPCTSTR lpszName);
	void SetPageCaption (int nPageIdx, LPCTSTR lpszName);

	BOOL GetPageByGroup (int nGroup, int &nPage) const;

	virtual void OnPressBackButton ();
	virtual void OnPressForwardButton ();
	virtual void OnPressHomeButton ();
	virtual void OnPressOtherButton (CBCGPCaptionMenuButton* pbtn, CWnd* pWndOwner);
	BOOL IsBackButtonEnabled () const		{ return m_iActivePage > 0; }
	BOOL IsForwardButtonEnabled () const	{ return m_iActivePage < m_arrHistoryStack.GetUpperBound (); }

	// --------------
	// Group support:
	// --------------
	int AddGroup (int nPageIdx, LPCTSTR lpszGroupName, BOOL bBottomLocation = FALSE, 
		BOOL bSpecial = FALSE, HICON hIcon = NULL);
	int AddGroup (LPCTSTR lpszGroupName, BOOL bBottomLocation = FALSE, 
		BOOL bSpecial = FALSE, HICON hIcon = NULL)
	{
		return AddGroup (0, lpszGroupName, bBottomLocation, bSpecial, hIcon);
	}
	void RemoveGroup (int nGroup);
	void RemoveAllGroups (int nPageIdx = 0);

	BOOL SetGroupName (int nGroup, LPCTSTR lpszGroupName);
	BOOL SetGroupTextColor (int nGroup, COLORREF color, COLORREF colorHot = (COLORREF)-1);
	BOOL CollapseGroup (CBCGPTasksGroup* pGroup, BOOL bCollapse = TRUE);

	BOOL CollapseGroup (int nGroup, BOOL bCollapse = TRUE)
	{
		return CollapseGroup (GetTaskGroup (nGroup), bCollapse);
	}

	void CollapseAllGroups (BOOL bCollapse = TRUE);
	void CollapseAllGroups (int nPageIdx, BOOL bCollapse);

	void EnableGroupCollapse (BOOL bEnable) 
	{ 
		if (!bEnable)
		{
			CollapseAllGroups (FALSE);
		}

		m_bCanCollapse = bEnable; 
	}

	BOOL IsGroupCollapseEnabled () const
	{
		return m_bCanCollapse;
	}

	CBCGPTasksGroup* GetTaskGroup (int nGroup) const;
	BOOL GetGroupLocation (CBCGPTasksGroup* pGroup, int &nGroup) const;
	int GetGroupCount () const
	{
		return (int) m_lstTaskGroups.GetCount ();
	}

	// -------------
	// Task support:
	// -------------
	int AddTask (int nGroup, LPCTSTR lpszTaskName, int nTaskIcon = -1,
		UINT uiCommandID = 0, DWORD_PTR dwUserData = 0);
	int AddSeparator (int nGroup)
	{
		return AddTask (nGroup, NULL); 
	}

	BOOL SetTaskName (int nGroup, int nTask, LPCTSTR lpszTaskName);
	BOOL SetTaskTextColor (int nGroup, int nTask, COLORREF color, COLORREF colorHot = (COLORREF)-1);
	BOOL ShowTask (int nGroup, int nTask, BOOL bShow = TRUE, BOOL bRedraw = TRUE);
	BOOL ShowTaskByCmdId (UINT uiCommandID, BOOL bShow = TRUE, BOOL bRedraw = TRUE);
	BOOL RemoveTask (int nGroup, int nTask, BOOL bRedraw = TRUE);
	void RemoveAllTasks (int nGroup);
	BOOL GetTaskLocation (UINT uiCommandID, int& nGroup, int& nTask) const;
	BOOL GetTaskLocation (HWND hwndTask, int& nGroup, int& nTask) const;
	CBCGPTask* GetTask (int nGroup, int nTask) const;
	BOOL GetTaskLocation (CBCGPTask* pTask, int &nGroup, int& nTask) const;

	int GetTaskCount (int nGroup) const
	{
		ASSERT(nGroup >= 0);
		ASSERT(nGroup < m_lstTaskGroups.GetCount ());

		CBCGPTasksGroup* pGroup = GetTaskGroup (nGroup);
		ASSERT_VALID (pGroup);

		return (int) pGroup->m_lstTasks.GetCount ();
	}
	
	int AddWindow (int nGroup, HWND hwndTask, int nWndHeight, 
		BOOL bAutoDestroyWindow = FALSE, DWORD_PTR dwUserData = 0,
		CBCGPTask::WindowAlign align = CBCGPTask::BCGP_TASKPANE_WND_STRETCH,
		int nWndWidth = 0);

	BOOL SetWindowHeight (int nGroup, HWND hwndTask, int nWndHeight);
	BOOL SetWindowHeight (HWND hwndTask, int nWndHeight);

	int AddLabel (	int nGroup, LPCTSTR lpszLabelName, int nTaskIcon = -1, 
					BOOL bIsBold = FALSE)
	{ 
		int nTask = AddTask(nGroup, lpszLabelName, nTaskIcon); 
		if (nTask == -1 || !bIsBold)
		{
			return nTask;
		}

		POSITION pos = m_lstTaskGroups.FindIndex (nGroup);
		if (pos == NULL)
		{
			ASSERT (FALSE);
			return -1;
		}

		CBCGPTasksGroup* pGroup = (CBCGPTasksGroup*) m_lstTaskGroups.GetAt (pos);
		ASSERT_VALID (pGroup);

		CBCGPTask* pTask = (CBCGPTask*)pGroup->m_lstTasks.GetTail ();
		if (pTask != NULL)
		{
			pTask->m_bIsBold = bIsBold;
		}

		return nTask;
	}

	int AddMRUFilesList (int nGroup, int nMaxFiles = 4);

	// --------
	// Margins:
	// --------
	int GetVertMargin () const
	{
		return m_nVertMargin;
	}

	void SetVertMargin (int n = -1)
	{
		ASSERT(n >= -1);
		m_nVertMargin = n;
	}

	int GetHorzMargin () const
	{
		return m_nHorzMargin;
	}

	void SetHorzMargin (int n = -1)
	{
		ASSERT(n >= -1);
		m_nHorzMargin = n;
	}

	int GetGroupVertOffset () const
	{
		return m_nGroupVertOffset;
	}

	void SetGroupVertOffset (int n = -1)
	{
		ASSERT(n >= -1);
		m_nGroupVertOffset = n;
	}

	int GetGroupCaptionHeight () const
	{
		return m_nGroupCaptionHeight;
	}

	void SetGroupCaptionHeight (int n = -1)
	{
		ASSERT(n >= -1);
		m_nGroupCaptionHeight = n;
	}

	int GetGroupCaptionHorzOffset () const
	{
		return m_nGroupCaptionHorzOffset;
	}

	void SetGroupCaptionHorzOffset (int n = -1)
	{
		ASSERT(n >= -1);
		m_nGroupCaptionHorzOffset = n;
	}

	int GetGroupCaptionVertOffset () const
	{
		return m_nGroupCaptionVertOffset;
	}

	void SetGroupCaptionVertOffset (int n = -1)
	{
		ASSERT(n >= -1);
		m_nGroupCaptionVertOffset = n;
	}

	int GetTaskMinHeight () const
	{
		return m_nTaskMinHeight;
	}

	void SetTaskMinHeight (int n = -1)
	{
		ASSERT(n >= -1);
		m_nTaskMinHeight = n;
	}

	int GetTasksVertMargin () const
	{
		return m_nTasksVertMargin;
	}

	void SetTasksVertMargin (int n = -1)
	{
		ASSERT(n >= -1);
		m_nTasksVertMargin = n;
	}

	int GetTasksHorzOffset () const
	{
		return m_nTasksHorzOffset;
	}

	void SetTasksHorzOffset (int n = -1)
	{
		ASSERT(n >= -1);
		m_nTasksHorzOffset = n;
	}

	int GetTasksIconHorzOffset () const
	{
		return m_nTasksIconHorzOffset;
	}

	void SetTasksIconHorzOffset (int n = -1)
	{
		ASSERT(n >= -1);
		m_nTasksIconHorzOffset = n;
	}

	int GetTasksIconVertOffset () const
	{
		return m_nTasksIconVertOffset;
	}

	void SetTasksIconVertOffset (int n = -1)
	{
		ASSERT(n >= -1);
		m_nTasksIconVertOffset = n;
	}

	void EnableOffsetCustomControls (BOOL bEnable)
	{
		m_bOffsetCustomControls = bEnable;
	}

	// ---------
	// Behavior:
	// ---------
	void EnableScrollButtons (BOOL bEnable = TRUE);

	void EnableNavigationToolbar (BOOL bEnable = TRUE,
		UINT uiToolbarBmpRes = 0, CSize sizeToolbarImage = CSize (0, 0),
		CSize sizeToolbarButton = CSize (0, 0));

	BOOL IsNavigationToolbarEnabled () const
	{
		return m_bUseNavigationToolbar;
	}

	void EnableAnimation (BOOL bEnable = TRUE)
	{
		m_bAnimationEnabled = bEnable;
	}

	BOOL IsAnimationEnabled () const
	{
		return m_bAnimationEnabled;
	}

	void EnableHistoryMenuButtons (BOOL bEnable = TRUE);

	BOOL IsHistoryMenuButtonsEnabled () const
	{
		return m_bHistoryMenuButtons;
	}

	void EnableWrapTasks  (BOOL bEnable = TRUE)
	{
		m_bWrapTasks = bEnable;
	}

	void EnableWrapLabels  (BOOL bEnable = TRUE)
	{
		m_bWrapLabels = bEnable;
	}

	BOOL IsWrapTasksEnabled () const
	{
		return m_bWrapTasks;
	}

	BOOL IsWrapLabelsEnabled () const
	{
		return m_bWrapLabels;
	}

	// ----------------
	// Scrollbar style:
	// ----------------
	void SetScrollBarsStyle (CBCGPScrollBar::BCGPSB_STYLE style)
	{
		m_ScrollBarStyle = style;

		m_wndScrollVert.SetVisualStyle (style);
	}

	CBCGPScrollBar::BCGPSB_STYLE GetScrollBarsStyle () const
	{
		return m_ScrollBarStyle;
	}

// Overrides
public:
	virtual void OnClickTask (int nGroupNumber, int nTaskNumber, 
							UINT uiCommandID, DWORD_PTR dwUserData);
	virtual void OnFirst();
	virtual void OnNext();
	virtual void OnPrev();
	virtual BOOL OnEnter();

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPTasksPane)
	public:
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL
	virtual CScrollBar* GetScrollBarCtrl(int nBar) const;
	virtual void Serialize (CArchive& ar);
	virtual BOOL LoadState (LPCTSTR lpszProfileName = NULL, int nIndex = -1, UINT uiID = (UINT) -1);
	virtual BOOL SaveState (LPCTSTR lpszProfileName = NULL, int nIndex = -1, UINT uiID = (UINT) -1);
	virtual CBCGPMiniFrameWnd* CreateDefaultMiniframe (CRect rectInitial);

	virtual BCGP_DOCK_TYPE GetDockMode () const 
	{
		return BCGP_DT_IMMEDIATE;
	}

	virtual void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler);
	virtual void ShowCommandMessageString (UINT uiCmdId);
	
	virtual void Update ();
	virtual void UpdateFocusedItems ();
	
	virtual BOOL IsToolBox () const		{	return FALSE;	}

	virtual BOOL IsAccessibilityCompatible () { return TRUE; }
	virtual BOOL CBCGPTasksPane::OnSetAccData (long /*lVal*/);

	virtual BOOL OnGestureEventPan(const CPoint& ptFrom, const CPoint& ptTo, CSize& sizeOverPan);

protected:
	virtual void OnFillBackground (CDC* pDC, CRect rectFill);
	virtual void OnDrawTasks (CDC* pDC, CRect rectWorkArea);
	virtual void OnDrawTaskFocusRect (CDC* pDC, CBCGPTask* pTask);
	virtual void OnDrawGroupFocusRect (CDC* pDC, CBCGPTasksGroup* pGroup);
	virtual void OnFillGroup(CDC* pDC, CBCGPTasksGroup* pGroup);
	virtual void OnActivateTasksPanePage () {}
	virtual void SetCaptionButtons ();
	virtual void DrawCaption (CDC* pDC, CRect rectCaption);
	virtual void OnPressButtons (UINT nHit);

	virtual void OnTrackCaptionButtons (CPoint point);
	virtual void StopCaptionButtonsTracking ();
	
	virtual BOOL GetMRUFileName (CRecentFileList* pRecentFileList, int nIndex, CString &strName);

	virtual CSize GetTasksGroupBorders () const;
	
	virtual void ScrollChild (HWND /*hwndTask*/, int /*nScrollValue*/) {}

	virtual void NotifyAccessibility (int nGroupNumber, int nTaskNumber);
	virtual void NotifyAccessibilityFocusEvent (BOOL bUseCursor);
	virtual BOOL ProcessKeyboard (UINT nKey);
	virtual void OnChangeFocus (CBCGPTask* pTask, CBCGPTasksGroup* pGroup);
	virtual void OnHotTask (CBCGPTask* pTask);
	virtual void OnHotGroupCaption (CBCGPTasksGroup* pGroup);

	virtual void OnEndAnimation (CBCGPTasksGroup* /*pGroup*/) {}

// Implementation
public:
	virtual ~CBCGPTasksPane();
	
	HMENU CreateMenu () const;

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPTasksPane)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnCancelMode();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	//}}AFX_MSG
	afx_msg LRESULT OnSetFont (WPARAM, LPARAM);
	afx_msg LRESULT OnGetFont (WPARAM, LPARAM);
	afx_msg BOOL OnNeedTipText(UINT id, NMHDR* pNMH, LRESULT* pResult);
	afx_msg void OnBack ();
	afx_msg void OnForward ();
	afx_msg void OnHome ();
	afx_msg void OnClose ();
	afx_msg void OnOther ();
	afx_msg void OnUpdateBack (CCmdUI* pCmdUI);
	afx_msg void OnUpdateForward (CCmdUI* pCmdUI);
	afx_msg void OnUpdateClose (CCmdUI* pCmdUI);
	afx_msg LRESULT OnSetText(WPARAM, LPARAM);
	DECLARE_MESSAGE_MAP()


	virtual void DoPaint(CDC* pDC);
	virtual int ReposTasks (BOOL bCalcHeightOnly = FALSE);
	void CreateFonts ();
	HFONT SetFont (CDC* pDC);
	void SetScrollSizes ();
	void AdjustScroll ();
	void RebuildMenu ();
	void ChangeActivePage (int nNewPageHistoryIdx, int nOldPageHistoryIdx);
	void SaveHistory (int nPageIdx);
	BOOL CreateNavigationToolbar ();
	void UpdateCaption ();

	BOOL IsScrollUpAvailable ()
	{
		return m_nVertScrollOffset > 0;
	}

	BOOL IsScrollDnAvailable ()
	{
		return m_nVertScrollOffset <= m_nVertScrollTotal - m_nVertScrollPage && m_nVertScrollTotal > 0;
	}

	BOOL ForceShowNavToolbar () const
	{
		BOOL bIsAttached = GetParentTabbedBar () != NULL;
		CBCGPMiniFrameWnd* pMiniFrame = GetParentMiniFrame (TRUE);
		BOOL bNonTasksPaneMiniFrame = pMiniFrame != NULL && 
			!pMiniFrame->IsKindOf( RUNTIME_CLASS (CBCGPTaskPaneMiniFrameWnd));
		return bIsAttached || bNonTasksPaneMiniFrame;
	}

	CBCGPTask* TaskHitTest (CPoint pt) const;
	virtual CBCGPTasksGroup* GroupCaptionHitTest (CPoint pt) const;

	void EnsureVisible ();
	void DoTaskClick (CBCGPTask* pClickTask);
	void DoGroupCaptionClick (CBCGPTasksGroup* pClickGroupCaption, BOOL bDisableAnimation);
 	CBCGPTask* GetNextTask (CBCGPTasksGroup* pGroup, CBCGPTask* pStartTask, BOOL bForward) const;
 	CBCGPTasksGroup* GetNextGroup (CBCGPTasksGroup* pStartGroup, BOOL bForward) const;
};

#endif // BCGP_EXCLUDE_TASK_PANE

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPTASKSPANE_H__77AE018B_A87F_4AD8_B3CA_9E6A578556F4__INCLUDED_)
