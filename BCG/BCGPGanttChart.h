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
// BCGPGanttChart.h : header file
//

#if !defined(AFX_BCGPGANTTCHART_H__74E5EE7A_9BED_4A34_B359_95A187BA50CF__INCLUDED_)
#define AFX_BCGPGANTTCHART_H__74E5EE7A_9BED_4A34_B359_95A187BA50CF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"
#include "BCGPWnd.h"

#if !defined (BCGP_EXCLUDE_GRID_CTRL) && !defined (BCGP_EXCLUDE_GANTT)

#include "BCGPScrollBar.h"
#include "BCGPGanttItem.h"
#include "BCGPGanttItemStorage.h"
#include "bcgglobals.h"

class  CBCGPGanttRenderer;
class  CBCGPGanttDrawContext;
class  CBCGPGanttControl;
struct BCGP_GANTT_ITEM_DRAGDROP;

struct BCGP_GANTT_CHART_COLORS 
{
	COLORREF	clrBackground;
	COLORREF	clrRowBackground;
	COLORREF	clrRowDayOff;
	COLORREF	clrBarFill;
	COLORREF	clrGroupBarFill;
	COLORREF	clrBarComplete;
	COLORREF	clrGridLine0;
	COLORREF	clrGridLine1;
	COLORREF	clrConnectorLines;
	COLORREF	clrShadows;
	COLORREF	clrSelection;
	COLORREF	clrSelectionBorder;

	BCGP_GANTT_CHART_COLORS();
};


#define  TIMEFORMAT_NONE                0
#define  TIMEFORMAT_HOURS_ONLY          1
#define  TIMEFORMAT_MINUTES_ONLY        2 
#define  TIMEFORMAT_HOURS_AND_MINUTES   3

struct BCGP_GANTT_CHART_HEADER 
{
	BOOL			    bVisible;
	BOOL			    bAboveChart;
	DWORD			    dwAlignment;
	UINT			    nHeight;
	COleDateTimeSpan    dtCellTimeDelta;
	CString 		    sDateFormat;
	DWORD               dwTimeFormat;

	BCGP_GANTT_CHART_HEADER ();
};

struct BCGP_GANTT_CHART_HEADER_CELL_INFO
{
	CRect			  rectCell;			    // cell area (in header)
	CRect			  rectColumn;		    // column area. Defines visible chart area for this column.
	CRect			  rectClip;			    // clipping rectangle
	COleDateTime	  dtCellLeftMostTime;	// starting time for cell
	COleDateTimeSpan  dtCellTimeDelta;
	const BCGP_GANTT_CHART_HEADER*	  pHeaderInfo;

	BCGP_GANTT_CHART_HEADER_CELL_INFO ();

	bool operator == (const BCGP_GANTT_CHART_HEADER_CELL_INFO& rhs) const;
	bool operator != (const BCGP_GANTT_CHART_HEADER_CELL_INFO& rhs) const;

	bool Exists () const;
	void Reset ();
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPGanttChart window

class BCGCBPRODLLEXPORT CBCGPGanttChart : public CBCGPWnd
{
	friend class CBCGPGanttItemStorageBase;
	friend class CBCGPGanttControl;

	struct UPDATE_INFO;

	DECLARE_DYNCREATE (CBCGPGanttChart)

// Construction
public:
	CBCGPGanttChart();

// Attributes
public:
	// Provides access to the items storage.
	CBCGPGanttItemStorageBase* GetStorage ();
	const CBCGPGanttItemStorageBase* GetStorage () const;

	CBCGPGanttItem* GetItemByIndex (int index) const;

	int IndexOfItem (const CBCGPGanttItem* pItem) const;

	int GetItemCount () const;

	void AddItem (CBCGPGanttItem* pItem);

	void InsertItem (int pos, CBCGPGanttItem* pItem);

	void RemoveItem (CBCGPGanttItem* pItem);

	typedef CArray<CBCGPGanttConnection*, CBCGPGanttConnection*> CConnectionArray;

	void GetItemReferrers  (const CBCGPGanttItem* pItem, CConnectionArray& arrReferrers);

	void GetItemLinks (const CBCGPGanttItem* pItem, CConnectionArray& arrLinks);

	CBCGPGanttConnection* AddConnection (CBCGPGanttItem* pSourceItem, CBCGPGanttItem* pDestItem, int linkType);

	CBCGPGanttConnection* FindConnection (const CBCGPGanttItem* pSourceItem, const CBCGPGanttItem* pDestItem);

	BOOL RemoveConnection (CBCGPGanttItem* pSourceItem, CBCGPGanttItem* pDestItem);

	virtual     BOOL	IsWorkingTime(const CBCGPGanttItem* pItem, COleDateTime day) const;

	int  GetSelectedItemsCount () const;
	void GetSelectedItems (CArray <CBCGPGanttItem*, CBCGPGanttItem*>& arrSelected) const;

	// Selects an item pointed by pItem. nKeys is a combination of MK_CONTROL, MK_SHIFT etc.
	void SelectItem (CBCGPGanttItem* pItem, UINT nKeys);

	// Returns client coordinates of the top item (depends on vertical scroll position)
	virtual     int 	GetFirstItemOffset() const;
	// Returns the height of an item or 0 if the item is not visible
	virtual     UINT 	GetItemHeight(const CBCGPGanttItem* pItem) const;
	// Returns the height of an item bar
	virtual     int 	GetItemBarHeight(const CBCGPGanttItem* pItem) const;
	// Returns item row client coordinates
	virtual     CRect	GetItemRow(const CBCGPGanttItem* pItem) const;
	// Returns item bar client coordinates
	virtual     CRect	GetBarRect(const CBCGPGanttItem* pItem) const;

	void 	            SetDefaultItemHeight(UINT nHeight);
	UINT 	            GetDefaultItemHeight() const;

	// Returns visible chart area client coordinates
	virtual     CRect	GetVisibleChartArea() const;

	CBCGPGanttItem*     ItemFromPoint(CPoint pt) const;

	//////////////////////////////////////////////////////////////////////////
	//                       Scale and time functions
	//////////////////////////////////////////////////////////////////////////

	COleDateTimeSpan    GetTimeDelta () const
	{
		return m_dtPixelTimeDelta;
	}

	void                SetTimeDelta (COleDateTimeSpan dtPixelTimeDelta, BOOL bKeepCenterDate = TRUE);
	void                IncreaseScale ();
	void                DecreaseScale ();

	void                GetVisibleTimeRange (COleDateTime* ptmStart, COleDateTime* ptmEnd) const;
	void                SetVisibleTimeRange(COleDateTime tmStart, COleDateTime tmEnd);
	BOOL                IsTimeVisible (COleDateTime time) const;
	int                 TimeToClient (COleDateTime time) const;
	COleDateTime        ClientToTime (int x) const;

	void                GotoDateTime (COleDateTime time);

	//////////////////////////////////////////////////////////////////////////
	//                      Headers and grid-lines
	//////////////////////////////////////////////////////////////////////////

	COleDateTimeSpan    GetHeaderTimeDelta (UINT nHeader) const;
	COleDateTimeSpan    GetGridLinesSpan (BOOL bLarge = FALSE) const
	{
		return bLarge ? m_dtGridLinesLargeSpan : m_dtGridLinesSmallSpan;
	}

	COleDateTime        SnapDateToGrid (COleDateTime dateTime) const;

	void SetHeaderHeights (UINT nHeaderHeight, UINT nSubHeaderHeight);
	void GetHeaderHeights (UINT* pHeaderHeight, UINT* nSubHeaderHeight) const;

	BOOL HeaderCellFromPoint (CPoint pt, BCGP_GANTT_CHART_HEADER_CELL_INFO& colInfo) const;

	struct CRowLayout 
	{
		CBCGPGanttItem* pItem;
		int             nRowTopY;
		int             nRowBottomY;
		CRect			rectBar;

		CRowLayout ();
	};

	//////////////////////////////////////////////////////////////////////////
	//                      Appearance
	//////////////////////////////////////////////////////////////////////////

	virtual BCGP_GANTT_CHART_COLORS GetActualColors () const;

	COLORREF GetColorScheme () const;
	void SetColorScheme (COLORREF clrMain);

	static void PrepareColorScheme (COLORREF clrMain, BCGP_GANTT_CHART_COLORS& colors);

	void GetUserColors (BCGP_GANTT_CHART_COLORS& colors) const;
	void SetUserColors (const BCGP_GANTT_CHART_COLORS& colors);

	void SetHeaderVisible (UINT nHeader, BOOL bVisible);
	BOOL GetHeaderVisible (UINT nHeader) const;

	void SetHeaderAboveChart (UINT nHeader, BOOL bAboveChart);
	BOOL GetHeaderAboveChart (UINT nHeader) const;

	void  SetHeaderTextAlignment (UINT nHeader, DWORD dwAlignment);
	DWORD GetHeaderTextAlignment (UINT nHeader) const;

	void SetShowEmptyRows (BOOL bShow);
	BOOL GetShowEmptyRows () const
	{
		return m_bShowEmptyRows;
	}

	void SetShowToolTip (BOOL bEnable = TRUE)
	{
		m_bShowTooltips = bEnable;
	}

	BOOL IsShowToolTip () const
	{
		return m_bShowTooltips;
	}

	virtual BOOL	QueryToolTipText(const CBCGPGanttItem* pItem, CString& strText, CString& strDescription) const;

// Operations
public:
	void InvalidateItem (const CBCGPGanttItem* pItem);
	void InvalidateHeaders ();
	void InvalidateChart ();

	virtual int CalculateScrollHeight () const;
	virtual COleDateTimeSpan GetScrollTimeRange (COleDateTime* pdtStart, COleDateTime* pdtEnd) const;

	void SetScrollBarsStyle (CBCGPScrollBar::BCGPSB_STYLE style);

	int  GetVScrollPos () const;
	void SetVScrollPos (int nPixelOffset);

	void SetReadOnly (BOOL bReadOnly = TRUE);
	BOOL IsReadOnly () const
	{
		return m_bReadOnly;
	}

	virtual void AdjustLayout();

// Control rendering
protected:
	virtual void OnDraw(CDC* pDC);
	virtual void DrawHeaderCell (CDC& dc, const BCGP_GANTT_CHART_HEADER_CELL_INFO& cellInfo, BOOL bHilite) const;
	virtual void DrawHeaderText (CDC& dc, const BCGP_GANTT_CHART_HEADER_CELL_INFO& cellInfo, const CString& sCellText, BOOL bHilite) const;
	virtual void DrawChartBackground (CDC& dc, const CRect& rectChart) const;
	virtual void DrawItemBackground  (CDC& dc, const CRect& rectItem, const CBCGPGanttItem* pItem) const;
	virtual void DrawHeader (CDC& dc, const BCGP_GANTT_CHART_HEADER& header, const CRect& rcPaint) const;
	virtual void SetupItemDrawContext (CBCGPGanttDrawContext& ctx) const;

// Overrides
public:
	virtual BOOL Create (DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPGanttChart)
	public:
	virtual BOOL PreCreateWindow (CREATESTRUCT& cs);
	virtual BOOL PreTranslateMessage (MSG* pMsg);
	virtual CScrollBar* GetScrollBarCtrl (int nBar) const;
	//}}AFX_VIRTUAL

	virtual BOOL IsGanttChartNotificationMessage (UINT message, WPARAM wParam, LPARAM lParam) const;

protected:
	void SetUpdateInfo (const UPDATE_INFO& updateInfo);

	void UpdateItemsLayout ();
	void UpdateItemsTimeRange (CBCGPGanttItem* pItemChanged);
	void UpdateScrollTimeRange (COleDateTime dtTimeToShow);
	void RecalculateItemsTimeRange ();
	void GetHeaderCellTimeRange (const COleDateTime& time, COleDateTimeSpan tmSpan, COleDateTime& tmStart, COleDateTime& tmEnd) const;

	void SetLeftmostDateTime (const COleDateTime& time);

	// GanttControl parent window accepts various notifications from Gantt chart.
	virtual CBCGPGanttControl* GetGanttControl () const;

	virtual CWnd* GetNotificationWindow () const;

	virtual void UpdateHeaders();

	void UpdateScrollBars (BOOL bValuesOnly);

	void UpdateConnections(const CArray<CBCGPGanttConnection*, CBCGPGanttConnection*>& arrLinks);
	void UpdateItemConnections(const CBCGPGanttItem* pItem);
	virtual void CalculateConnectorLine (CBCGPGanttConnection& link, CRect rcSourceBar, CRect rcDestBar);

	// Returns an item storage. Called once during control initialization
	virtual CBCGPGanttItemStorageBase* QueryItemStorage ();

	// Releases current item storage. Called during control destruction
	virtual void ReleaseItemStorage (CBCGPGanttItemStorageBase* pStorage);

// Implementation
public:
	virtual ~CBCGPGanttChart();

	virtual void PerformAdjustLayout ();

protected:
	// Returns an item renderer. Called if m_pRenderer is NULL.
	virtual CBCGPGanttRenderer* QueryRenderer ();

	// Releases current item renderer. Called during control destruction
	virtual void ReleaseRenderer (CBCGPGanttRenderer* pRenderer);

	CBCGPGanttRenderer* GetRenderer();

	virtual void DoPaint (CDC& dc);

protected:
	virtual BOOL DoClick (UINT nFlags, CPoint point);
	virtual BOOL DoItemClick (UINT nFlags, CBCGPGanttItem* pItem);
	virtual BOOL DoHeaderClick (UINT nFlags, CPoint point);
	virtual BOOL DoChartClick (UINT nFlags, CPoint point);

	virtual BOOL DoDoubleClick (UINT nFlags, CPoint point);
	virtual BOOL DoItemDoubleClick (UINT nFlags, CBCGPGanttItem* pItem);
	virtual BOOL DoHeaderDoubleClick (UINT nFlags, CPoint point);
	virtual BOOL DoChartDoubleClick (UINT nFlags, CPoint point);

	virtual BOOL DoScaleChanging (BOOL bIncrease);
	virtual void DoScaleChanged ();

	virtual void CalculateMovePosition (BCGP_GANTT_ITEM_DRAGDROP& dragDropInfo, COleDateTimeSpan dtOffset, BOOL bSnapToGrid);
	virtual void MoveSelectedItems (COleDateTimeSpan dtOffset);
	virtual void CalculateItemsResize (BCGP_GANTT_ITEM_DRAGDROP& dragDropInfo, COleDateTimeSpan dtOffset, BOOL bSnapToGrid);
	virtual void ResizeSelectedItems (COleDateTimeSpan dtOffset);

	BOOL         IsPointInItemResizeArea (CPoint point, CBCGPGanttItem** ppItem = NULL) const;

	void         HiliteItem (const CBCGPGanttItem* pItem);
	void         HiliteHeaderCell (const BCGP_GANTT_CHART_HEADER_CELL_INFO* pHeaderCellInfo);

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPGanttChart)
	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnCancelMode();
	afx_msg void OnDestroy();
	afx_msg LRESULT OnMouseLeave(WPARAM, LPARAM);
	afx_msg LRESULT OnUpdateToolTips (WPARAM wParam, LPARAM);
	afx_msg BOOL OnToolTipText(UINT id, NMHDR* pNMH, LRESULT* pResult);
	afx_msg void OnSettingChange (UINT uFlags, LPCTSTR lpszSection);
	afx_msg LRESULT OnGetFont (WPARAM, LPARAM);
	afx_msg LRESULT OnSetFont (WPARAM, LPARAM);
	afx_msg LRESULT OnStorageChanged (WPARAM, LPARAM);
	afx_msg LRESULT OnStorageConnectionAdded (WPARAM, LPARAM);
	afx_msg LRESULT OnStorageConnectionRemoved (WPARAM, LPARAM);
	//}}AFX_MSG
	afx_msg LRESULT OnPrintClient(WPARAM wp, LPARAM lp);
	DECLARE_MESSAGE_MAP()

protected:
	CBCGPGanttItemStorageBase*  m_pItems;
	CBCGPGanttRenderer*         m_pRenderer;
	CFont*			            m_pFont;

	UINT				        m_nItemDefaultHeight;
	BCGP_GANTT_CHART_HEADER     m_hdrLargeHeader;
	BCGP_GANTT_CHART_HEADER     m_hdrSmallHeader;

	COleDateTimeSpan	        m_dtGridLinesLargeSpan;
	COleDateTimeSpan	        m_dtGridLinesSmallSpan;

	BOOL                        m_bReadOnly;
	BOOL				        m_bShowEmptyRows;

	BCGP_GANTT_CHART_COLORS     m_UserColors;  // Overrides current visual style colors
	COLORREF                    m_clrMainSchemeColor;

	// Scale
	COleDateTimeSpan	        m_dtPixelTimeDelta;

	// Scroll positions
	UINT		        		m_nVerticalOffset;
	COleDateTime        		m_dtLeftMostChartTime;

	COleDateTime	        	m_dtHScrollMin;
	COleDateTime	        	m_dtHScrollMax;

	COleDateTime	        	m_dtItemsMin;
	COleDateTime	        	m_dtItemsMax;

	// Controls
	CBCGPScrollBar		        m_wndScrollHorz;
	CBCGPScrollBar		        m_wndScrollVert;

	CToolTipCtrl*		        m_pToolTip;
	BOOL                        m_bShowTooltips;

	// Internal states
protected:
	enum EControlState
	{
		ecsNormal = 0,
		ecsSelectingItem,
		ecsTimeScrolling,
		ecsMoveItems,
		ecsResizeItems,
		ecsDestroying
	};

	EControlState		        m_nControlState;
	BOOL				        m_bInAdjustLayout;
	BOOL                        m_bTrackingScrollThumb;
	CPoint				        m_ptDragStart;
	CSize				        m_szDragOffset;
	CBCGPGanttItem* 	        m_pLastSelectedItem; // for shift-key selection

	UPDATE_INFO*                m_pUpdateInfo;

	// Control Layout

	CRect				        m_rectChart;
	CRect				        m_rectLargeHeader;
	CRect				        m_rectSmallHeader;
	int 				        m_nFirstVisibleRow;
	int 				        m_nLastVisibleRow;
	CArray<CRowLayout, CRowLayout&>	m_arrItemRects; // contains only visible and partially visible rows

	BCGP_GANTT_CHART_HEADER_CELL_INFO m_HilitedHeaderCell;
	const CBCGPGanttItem*       m_pHilitedItem;
};

//////////////////////////////////////////////////////////////////////////
//              Gantt chart registered window messages
//////////////////////////////////////////////////////////////////////////

// Mouse click notifications

// Sent when the user clicks an item on a chart.
//  wParam: UINT flags - combination of MK_CONTROL, MK_SHIFT, MK_LBUTTON, MK_RBUTTON, MK_MBUTTON etc.
//  lParam: pointer to CBCGPItem being clicked
//  return value: TRUE if message was handled or FALSE to allow default processing.
BCGCBPRODLLEXPORT extern const UINT BCGM_GANTT_CHART_CLICKITEM;

// Sent when the user clicks an chart header.
//  wParam: UINT flags - combination of MK_CONTROL, MK_SHIFT, MK_LBUTTON, MK_RBUTTON, MK_MBUTTON etc.
//  lParam: specifies coordinates of the cursor relative to the upper-left corner of the client area (see WM_LBUTTONDOWN message)
//  return value: TRUE if message was handled or FALSE to allow default processing.
BCGCBPRODLLEXPORT extern const UINT BCGM_GANTT_CHART_CLICKHEADER;

// Sent when the user clicks a chart.
//  wParam: UINT flags - combination of MK_CONTROL, MK_SHIFT, MK_LBUTTON, MK_RBUTTON, MK_MBUTTON etc.
//  lParam: specifies coordinates of the cursor relative to the upper-left corner of the client area (see WM_LBUTTONDOWN message)
//  return value: TRUE if message was handled or FALSE to allow default processing.
BCGCBPRODLLEXPORT extern const UINT BCGM_GANTT_CHART_CLICKCHART;

// Sent when the user double-clicks an item on a chart.
//  wParam: UINT flags - combination of MK_CONTROL, MK_SHIFT, MK_LBUTTON, MK_RBUTTON, MK_MBUTTON etc.
//  lParam: pointer to CBCGPItem being clicked
//  return value: TRUE if message was handled or FALSE to allow default processing.
BCGCBPRODLLEXPORT extern const UINT BCGM_GANTT_CHART_DBLCLICKITEM;

// Sent when the user double-clicks an chart header.
//  wParam: UINT flags - combination of MK_CONTROL, MK_SHIFT, MK_LBUTTON, MK_RBUTTON, MK_MBUTTON etc.
//  lParam: specifies coordinates of the cursor relative to the upper-left corner of the client area (see WM_LBUTTONDOWN message)
//  return value: TRUE if message was handled or FALSE to allow default processing.
BCGCBPRODLLEXPORT extern const UINT BCGM_GANTT_CHART_DBLCLICKHEADER;

// Sent when the user double-clicks a chart.
//  wParam: UINT flags - combination of MK_CONTROL, MK_SHIFT, MK_LBUTTON, MK_RBUTTON, MK_MBUTTON etc.
//  lParam: specifies coordinates of the cursor relative to the upper-left corner of the client area (see WM_LBUTTONDOWN message)
//  return value: TRUE if message was handled or FALSE to allow default processing.
BCGCBPRODLLEXPORT extern const UINT BCGM_GANTT_CHART_DBLCLICKCHART;

//////////////////////////////////////////////////////////////////////////
//                  Drag and drop notifications
//////////////////////////////////////////////////////////////////////////

struct BCGP_GANTT_ITEM_DRAGDROP
{
	CBCGPGanttItem* pItem; // The item being dragged
	COleDateTime    dtNewStartTime;
	COleDateTime    dtNewFinishTime;
};

// Allows user to confirm moving an item with mouse (drag and drop)
//  wParam: not used
//  lParam: pointer to BCGP_GANTT_ITEM_DRAGDROP struct
//  return value: 0 to confirm, -1 to cancel
BCGCBPRODLLEXPORT extern const UINT BCGM_GANTT_CHART_ITEM_MOVING;

// Sent when user is changing chart scale using Ctrl+mouse wheel
// Use SetScale() to get new scale value or SetVisibleTimeRange() to set time range.
//  wParam: < 0 if user attempts to decrease scale; or > 0 to increase.
//  lParam: not used
//  return value: TRUE if new scale has been set, FALSE to use internal scale table (default processing)
BCGCBPRODLLEXPORT extern const UINT BCGM_GANTT_CHART_SCALE_CHANGING;

// Sent when user has changed chart scale using Ctrl+mouse wheel
// Use GetScale() to get new scale value or GetVisibleTimeRange() to detect the time range.
//  wParam: not used
//  lParam: not used
//  return value: 0
BCGCBPRODLLEXPORT extern const UINT BCGM_GANTT_CHART_SCALE_CHANGED;

#endif // !defined (BCGP_EXCLUDE_GRID_CTRL) && !defined (BCGP_EXCLUDE_GANTT)

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPGANTTCHART_H__74E5EE7A_9BED_4A34_B359_95A187BA50CF__INCLUDED_)
