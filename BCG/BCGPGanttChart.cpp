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
// BCGPGanttCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "BCGPGanttChart.h"
#include "BCGPGanttRenderer.h"
#include "BCGPGanttItemStorage.h"
#include "BCGPGanttControl.h"
#include "TrackMouse.h"
#include "BCGPDrawManager.h"

#ifndef _BCGSUITE_
#include "BCGPPopupMenu.h"
#include "BCGPToolTipCtrl.h"
#include "BCGPTooltipManager.h"
#endif

#include "BCGPMath.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#if !defined (BCGP_EXCLUDE_GRID_CTRL) && !defined (BCGP_EXCLUDE_GANTT)

#define DATETIMERANGE_YEAR		1
#define DATETIMERANGE_QUARTER	2
#define DATETIMERANGE_MONTH		3
#define DATETIMERANGE_WEEK		4
#define DATETIMERANGE_DAY		5
#define DATETIMERANGE_12HOURS	6
#define DATETIMERANGE_6HOURS	7
#define DATETIMERANGE_4HOURS	8
#define DATETIMERANGE_2HOURS	9
#define DATETIMERANGE_1HOUR		10
#define DATETIMERANGE_30MINUTES 11
#define DATETIMERANGE_20MINUTES 12
#define DATETIMERANGE_15MINUTES 13
#define DATETIMERANGE_10MINUTES 14
#define DATETIMERANGE_5MINUTES  15
#define DATETIMERANGE_1MINUTE	16
#define DATETIMERANGE_30SECONDS 17
#define DATETIMERANGE_1SECOND	18

//////////////////////////////////////////////////////////////////////////
//              Gantt chart registered window messages
//////////////////////////////////////////////////////////////////////////

const UINT BCGM_GANTT_CHART_CLICKITEM      = ::RegisterWindowMessage (_T("BCGM_GANTT_ITEM_CLICK"));
const UINT BCGM_GANTT_CHART_CLICKHEADER    = ::RegisterWindowMessage (_T("BCGM_GANTT_HEADER_CLICK"));
const UINT BCGM_GANTT_CHART_CLICKCHART     = ::RegisterWindowMessage (_T("BCGM_GANTT_CHART_CLICK"));
const UINT BCGM_GANTT_CHART_DBLCLICKITEM   = ::RegisterWindowMessage (_T("BCGM_GANTT_ITEM_DBLCLICK"));
const UINT BCGM_GANTT_CHART_DBLCLICKHEADER = ::RegisterWindowMessage (_T("BCGM_GANTT_HEADER_DBLCLICK"));
const UINT BCGM_GANTT_CHART_DBLCLICKCHART  = ::RegisterWindowMessage (_T("BCGM_GANTT_CHART_DBLCLICK"));

const UINT BCGM_GANTT_CHART_ITEM_MOVING    = ::RegisterWindowMessage (_T("BCGM_GANTT_ITEM_MOVING"));

const UINT BCGM_GANTT_CHART_SCALE_CHANGING = ::RegisterWindowMessage (_T("BCGM_GANTT_SCALE_CHANGING"));
const UINT BCGM_GANTT_CHART_SCALE_CHANGED  = ::RegisterWindowMessage (_T("BCGM_GANTT_SCALE_CHANGED"));

//////////////////////////////////////////////////////////////////////////
//                          Helper structures
//////////////////////////////////////////////////////////////////////////

BCGP_GANTT_CHART_COLORS::BCGP_GANTT_CHART_COLORS ()
	: clrBackground     (CLR_DEFAULT)
	, clrRowBackground  (CLR_DEFAULT)
	, clrBarFill        (CLR_DEFAULT)
	, clrBarComplete    (CLR_DEFAULT)
	, clrGroupBarFill   (CLR_DEFAULT)
	, clrGridLine0      (CLR_DEFAULT)
	, clrGridLine1      (CLR_DEFAULT)
	, clrConnectorLines (CLR_DEFAULT)
	, clrShadows        (CLR_DEFAULT)
	, clrSelection      (CLR_DEFAULT)
	, clrSelectionBorder(CLR_DEFAULT)
	, clrRowDayOff      (CLR_DEFAULT)
{
}

BCGP_GANTT_CHART_HEADER::BCGP_GANTT_CHART_HEADER ()
	: bVisible      (TRUE)
	, bAboveChart   (TRUE)
	, dwAlignment   (DT_LEFT)
	, nHeight       (21)
	, dtCellTimeDelta(1, 0, 0, 0)
	, dwTimeFormat  (TIMEFORMAT_NONE)
{
}

BCGP_GANTT_CHART_HEADER_CELL_INFO::BCGP_GANTT_CHART_HEADER_CELL_INFO ()
	: rectCell      (0, 0, 0, 0)
	, rectColumn    (0, 0, 0, 0)
	, rectClip      (0, 0, 0, 0)
	, pHeaderInfo   (NULL)
{}

bool BCGP_GANTT_CHART_HEADER_CELL_INFO::operator == (const BCGP_GANTT_CHART_HEADER_CELL_INFO& rhs) const
{
	return (pHeaderInfo == rhs.pHeaderInfo && dtCellLeftMostTime == rhs.dtCellLeftMostTime);
}

bool BCGP_GANTT_CHART_HEADER_CELL_INFO::operator != (const BCGP_GANTT_CHART_HEADER_CELL_INFO& rhs) const
{
	return !(*this == rhs);
}

bool BCGP_GANTT_CHART_HEADER_CELL_INFO::Exists () const
{
	return pHeaderInfo != NULL;
}

void BCGP_GANTT_CHART_HEADER_CELL_INFO::Reset ()
{
	*this = BCGP_GANTT_CHART_HEADER_CELL_INFO ();
}

CBCGPGanttChart::CRowLayout::CRowLayout ()
	: pItem (NULL)
	, nRowTopY (0)
	, nRowBottomY (0)
	, rectBar (0, 0, 0, 0)
{
}

//////////////////////////////////////////////////////////////////////////
//                          Helper functions
//////////////////////////////////////////////////////////////////////////

CFont* SelectFont (CDC* pDC, CFont* pFont)
{
	ASSERT_VALID (pDC);

	return pDC->SelectObject((pFont == NULL) ? &globalData.fontRegular : pFont);
}

UINT GetFontHeight (CFont* pFont, CDC* pDC = NULL)
{
	if (pFont == NULL)
	{
		return globalData.GetTextHeight ();
	}

	TEXTMETRIC tm;

	if (pDC == NULL)
	{
		CWindowDC dc (NULL);
		CFont* pOldFont = dc.SelectObject (pFont);
		dc.GetTextMetrics (&tm);
		dc.SelectObject (pOldFont);
	}
	else
	{
		CFont* pOldFont = pDC->SelectObject (pFont);
		pDC->GetTextMetrics (&tm);
		pDC->SelectObject (pOldFont);       
	}

	return tm.tmHeight + (tm.tmHeight < 15 ? 2 : 5); 
		// see also  BCGPGLOBAL_DATA::UpdateTextMetrics() in BCGGlobals.cpp
}


// Extracts date by setting time to 0:00:00
static COleDateTime ExtractDate (const COleDateTime& date)
{
	return COleDateTime (date.GetYear (), date.GetMonth (), date.GetDay (), 0, 0, 0);
}

static BOOL IsTimeNull (const COleDateTime& date)
{
	SYSTEMTIME st;
	date.GetAsSystemTime (st);
	return st.wHour == 0 && st.wMinute == 0 && st.wSecond == 0;
}

static DWORD DetectSpanType(const COleDateTimeSpan& tmSpan)
{
	int nDays = (int)tmSpan.GetTotalDays ();
	int nMonths = nDays / 29;
	int nYears = nDays / 365;
	int nSeconds = (int)tmSpan.GetTotalSeconds ();
	int nMinutes = nSeconds / 60;
	int nHours = nMinutes / 60;

	if (nYears >= 1)
	{
		return DATETIMERANGE_YEAR;
	}
	if (nMonths == 3)
	{
		return DATETIMERANGE_QUARTER;
	}
	if (nMonths >= 1)
	{
		return DATETIMERANGE_MONTH;
	}
	if (nDays == 7)
	{
		return DATETIMERANGE_WEEK;
	}
	if (nDays >= 1)
	{
		return DATETIMERANGE_DAY;
	}

	if (nHours == 12)
	{
		return DATETIMERANGE_12HOURS;
	}
	if (nHours == 6)
	{
		return DATETIMERANGE_6HOURS;
	}
	if (nHours == 4)
	{
		return DATETIMERANGE_4HOURS;
	}
	if (nHours == 2)
	{
		return DATETIMERANGE_2HOURS;
	}
	if (nHours >= 1)
	{
		return DATETIMERANGE_1HOUR;
	}

	if (nMinutes == 30)
	{
		return DATETIMERANGE_30MINUTES;
	}
	if (nMinutes == 20)
	{
		return DATETIMERANGE_20MINUTES;
	}
	if (nMinutes == 15)
	{
		return DATETIMERANGE_15MINUTES;
	}
	if (nMinutes == 10)
	{
		return DATETIMERANGE_10MINUTES;
	}
	if (nMinutes == 5)
	{
		return DATETIMERANGE_5MINUTES;
	}
	if (nMinutes >= 1)
	{
		return DATETIMERANGE_1MINUTE;
	}

	if (nSeconds == 30)
	{
		return DATETIMERANGE_30SECONDS;
	}
	
	return DATETIMERANGE_1SECOND;
}

static void GetDateTimeRange (const COleDateTime& date, DWORD dwRangeType, COleDateTime& tmStart, COleDateTime& tmEnd)
{
	SYSTEMTIME st;
	date.GetAsSystemTime (st);

	switch (dwRangeType)
	{
	case DATETIMERANGE_YEAR:
		tmStart.SetDateTime (st.wYear, 1, 1, 0, 0, 0);
		tmEnd.SetDateTime (st.wYear + 1, 1, 1, 0, 0, 0);
		return;
	case DATETIMERANGE_QUARTER:
		tmStart.SetDateTime (st.wYear, (st.wMonth / 3) * 3, 1, 0, 0, 0);
		if (st.wMonth < 10)
		{
			tmEnd.SetDateTime (st.wYear, (st.wMonth / 3) * 3 + 3, 1, 0, 0, 0);
		}
		else
		{
			tmEnd.SetDateTime (st.wYear + 1, 1, 1, 0, 0, 0);
		}
		return;
	case DATETIMERANGE_MONTH:
		tmStart.SetDateTime (st.wYear, st.wMonth, 1, 0, 0, 0);
		if (st.wMonth < 12)
		{
			tmEnd.SetDateTime (st.wYear, st.wMonth + 1, 1, 0, 0, 0);
		}
		else
		{
			tmEnd.SetDateTime (st.wYear + 1, 1, 1, 0, 0, 0);
		}
		return;
	case DATETIMERANGE_WEEK:
		tmStart = date - COleDateTimeSpan (st.wDayOfWeek, 0, 0, 0);
		tmStart = ExtractDate (tmStart);
		tmEnd = tmStart + COleDateTimeSpan (7, 0, 0, 0);
		return;
	case DATETIMERANGE_DAY:
		tmStart = ExtractDate (date);
		tmEnd = tmStart + COleDateTimeSpan (1, 0, 0, 0);
		return;
	case DATETIMERANGE_4HOURS:
		tmStart.SetDateTime(st.wYear, st.wMonth, st.wDay, (st.wHour / 4) * 4, 0, 0);
		tmEnd = tmStart + COleDateTimeSpan (0, 4, 0, 0);
		return;
	case DATETIMERANGE_1HOUR:
		tmStart.SetDateTime(st.wYear, st.wMonth, st.wDay, st.wHour, 0, 0);
		tmEnd = tmStart + COleDateTimeSpan (0, 1, 0, 0);
		return;
	case DATETIMERANGE_15MINUTES:
		tmStart.SetDateTime(st.wYear, st.wMonth, st.wDay, st.wHour, (st.wMinute / 15) * 15, 0);
		tmEnd = tmStart + COleDateTimeSpan (0, 0, 15, 0);
		return;
	case DATETIMERANGE_5MINUTES:
		tmStart.SetDateTime(st.wYear, st.wMonth, st.wDay, st.wHour, (st.wMinute / 5) * 5, 0);
		tmEnd = tmStart + COleDateTimeSpan (0, 0, 5, 0);
		return;
	case DATETIMERANGE_1MINUTE:
		tmStart.SetDateTime(st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, 0);
		tmEnd = tmStart + COleDateTimeSpan (0, 0, 1, 0);
		return;
	case DATETIMERANGE_1SECOND:
		tmStart.SetDateTime(st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
		tmEnd = tmStart + COleDateTimeSpan (0, 0, 0, 1);
		return;
	default:
		TRACE0("GetDateTimeRange() : Invalid range type argument\n");
		ASSERT(FALSE);
		break;
	}
}
/////////////////////////////////////////////////////////////////////////////
// CBCGPGanttChart::UPDATE_INFO

struct CBCGPGanttChart::UPDATE_INFO
{
	int                 nHScrollDelta;   // Differs from 0 if horizontal scrolling occurred
	int                 nVScrollDelta;   // Differs from 0 if horizontal scrolling occurred

	BOOL                bItemsLayoutChanged; // If TRUE, scroll deltas are ignored.

	BOOL                bLayoutChanged; // Occurs when control has been resized, header positions changed etc.

	BOOL                bAppearanceChanged; // Fonts, Colors, Grid lines, Visual style etc.

	BOOL                bConnectionsChanged;

	UPDATE_INFO()
		: nHScrollDelta(0)
		, nVScrollDelta(0)
		, bItemsLayoutChanged(TRUE)
		, bLayoutChanged(TRUE)
		, bAppearanceChanged(TRUE)
		, bConnectionsChanged(TRUE)
	{
	}

	bool NeedsUpdate() const
	{
		return  nHScrollDelta != 0 ||
				nVScrollDelta != 0 ||
				bItemsLayoutChanged ||
				bLayoutChanged ||
				bAppearanceChanged ||
				bConnectionsChanged;
	}

	void Overwrite (const UPDATE_INFO& info)
	{
		nHScrollDelta += info.nHScrollDelta;
		nVScrollDelta += info.nVScrollDelta;
		bItemsLayoutChanged = bItemsLayoutChanged || info.bItemsLayoutChanged;
		bLayoutChanged = bLayoutChanged || info.bLayoutChanged;
		bAppearanceChanged = bAppearanceChanged || info.bAppearanceChanged;
		bConnectionsChanged = bConnectionsChanged || info.bConnectionsChanged;
	}

	void Reset()
	{
		nHScrollDelta = 0;
		nVScrollDelta = 0;
		bItemsLayoutChanged = FALSE;
		bLayoutChanged = FALSE;
		bAppearanceChanged = FALSE;
		bConnectionsChanged = FALSE;
	}
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPGanttChart

IMPLEMENT_DYNCREATE (CBCGPGanttChart, CBCGPWnd)

CBCGPGanttChart::CBCGPGanttChart()
	: m_nItemDefaultHeight (23)
	, m_nVerticalOffset (0)
	, m_pRenderer (NULL)
	, m_pItems (NULL)
	, m_pFont (NULL)  // Use regular font from globalData
	, m_dtPixelTimeDelta (0, 0, 15, 0)
	, m_bInAdjustLayout (FALSE)
	, m_bTrackingScrollThumb (FALSE)
	, m_nControlState (ecsNormal)
	, m_pUpdateInfo (new UPDATE_INFO)
	, m_dtGridLinesLargeSpan (0, 0, 0, 0)
	, m_dtGridLinesSmallSpan (0, 0, 0, 0)
	, m_bReadOnly (FALSE)
	, m_bShowEmptyRows (FALSE)
	, m_pLastSelectedItem (NULL)
	, m_pHilitedItem (NULL)
	, m_pToolTip (NULL)
	, m_nFirstVisibleRow (0)
	, m_nLastVisibleRow (0)
	, m_clrMainSchemeColor (CLR_DEFAULT)
	, m_bShowTooltips (TRUE)
{
	m_dtHScrollMin = COleDateTime::GetCurrentTime ();
	m_dtHScrollMax = m_dtHScrollMin;
	m_dtItemsMin = m_dtHScrollMin;
	m_dtItemsMax = m_dtHScrollMin;

	m_dtLeftMostChartTime = m_dtHScrollMin;

	m_hdrSmallHeader.dwAlignment = DT_CENTER;

	SetScrollBarsStyle (CBCGPScrollBar::BCGP_SBSTYLE_DEFAULT);
}

CBCGPGanttChart::~CBCGPGanttChart()
{
	delete m_pUpdateInfo;
}

BEGIN_MESSAGE_MAP(CBCGPGanttChart, CBCGPWnd)
	//{{AFX_MSG_MAP(CBCGPGanttChart)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_VSCROLL()
	ON_WM_HSCROLL()
	ON_WM_MOUSEWHEEL()
	ON_WM_KEYDOWN()
	ON_WM_CHAR()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONDBLCLK()
	ON_WM_CONTEXTMENU()
	ON_WM_SETCURSOR()
	ON_WM_SETTINGCHANGE()
	ON_WM_CANCELMODE()
	ON_WM_DESTROY()
	ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXT, 0, 0xFFFF, OnToolTipText)
	ON_REGISTERED_MESSAGE(BCGM_UPDATETOOLTIPS, OnUpdateToolTips)
	ON_REGISTERED_MESSAGE(BCGM_GANTT_STORAGE_CHANGED, OnStorageChanged)
	ON_REGISTERED_MESSAGE(BCGM_GANTT_CONNECTION_ADDED, OnStorageConnectionAdded)
	ON_REGISTERED_MESSAGE(BCGM_GANTT_CONNECTION_REMOVED, OnStorageConnectionRemoved)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_PRINTCLIENT, OnPrintClient)
	ON_MESSAGE(WM_SETFONT, OnSetFont)
	ON_MESSAGE(WM_GETFONT, OnGetFont)
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////
//                          Chart Colors
//////////////////////////////////////////////////////////////////////////

BCGP_GANTT_CHART_COLORS CBCGPGanttChart::GetActualColors () const
{
	BCGP_GANTT_CHART_COLORS colors;
	CBCGPVisualManager::GetInstance ()->GetGanttColors (this, colors, m_clrMainSchemeColor);

	if (m_UserColors.clrBackground != CLR_DEFAULT)
	{
		colors.clrBackground = m_UserColors.clrBackground;
	}
	if (m_UserColors.clrRowBackground != CLR_DEFAULT)
	{
		colors.clrRowBackground = m_UserColors.clrRowBackground;
	}
	if (m_UserColors.clrRowDayOff != CLR_DEFAULT)
	{
		colors.clrRowDayOff = m_UserColors.clrRowDayOff;
	}
	if (m_UserColors.clrBarFill != CLR_DEFAULT)
	{
		colors.clrBarFill = m_UserColors.clrBarFill;
	}
	if (m_UserColors.clrGroupBarFill != CLR_DEFAULT)
	{
		colors.clrGroupBarFill = m_UserColors.clrGroupBarFill;
	}
	if (m_UserColors.clrBarComplete != CLR_DEFAULT)
	{
		colors.clrBarComplete = m_UserColors.clrBarComplete;
	}
	if (m_UserColors.clrGridLine0 != CLR_DEFAULT)
	{
		colors.clrGridLine0 = m_UserColors.clrGridLine0;
	}
	if (m_UserColors.clrGridLine1 != CLR_DEFAULT)
	{
		colors.clrGridLine1 = m_UserColors.clrGridLine1;
	}
	if (m_UserColors.clrConnectorLines != CLR_DEFAULT)
	{
		colors.clrConnectorLines = m_UserColors.clrConnectorLines;
	}
	if (m_UserColors.clrShadows != CLR_DEFAULT)
	{
		colors.clrShadows = m_UserColors.clrShadows;
	}
	if (m_UserColors.clrSelection != CLR_DEFAULT)
	{
		colors.clrSelection = m_UserColors.clrSelection;
	}
	if (m_UserColors.clrSelectionBorder != CLR_DEFAULT)
	{
		colors.clrSelectionBorder = m_UserColors.clrSelectionBorder;
	}

	// Calculate some color values if not defined

	if (colors.clrGroupBarFill == CLR_DEFAULT)
	{
		colors.clrGroupBarFill = CBCGPDrawManager::MixColors (colors.clrBarFill, RGB(48, 48, 48), 0.5f);
	}
	if (colors.clrSelectionBorder == CLR_DEFAULT)
	{
		colors.clrSelectionBorder = CBCGPDrawManager::MixColors (colors.clrSelection, RGB(0, 0, 0), 0.5f);
	}

	return colors;
}

void CBCGPGanttChart::GetUserColors (BCGP_GANTT_CHART_COLORS& colors) const
{
	colors = m_UserColors;
}

void CBCGPGanttChart::SetUserColors (const BCGP_GANTT_CHART_COLORS& colors)
{
	m_UserColors = colors;

	if (GetSafeHwnd () != NULL)
	{
		Invalidate ();
	}
}

COLORREF CBCGPGanttChart::GetColorScheme () const
{
	return m_clrMainSchemeColor;
}

void CBCGPGanttChart::SetColorScheme (COLORREF clrMain)
{
	if (clrMain != m_clrMainSchemeColor)
	{
		m_clrMainSchemeColor = clrMain;

		if (GetSafeHwnd () != NULL)
		{
			Invalidate ();
		}
	}
}

void CBCGPGanttChart::PrepareColorScheme (COLORREF clrMain, BCGP_GANTT_CHART_COLORS& colors) // static
{
	double H, S, L;
	CBCGPDrawManager::RGBtoHSL (clrMain, &H, &S, &L);
	double L1 = max (L, 0.85);
	clrMain = CBCGPDrawManager::HLStoRGB_ONE (H, L1, S);

	colors.clrRowBackground  = clrMain;
	colors.clrRowDayOff      = CBCGPDrawManager::HLStoRGB_ONE (H, L1 - 0.05, S);
	colors.clrBarFill        = CBCGPDrawManager::HLStoRGB_ONE (H, L1 - 0.50, S);
	colors.clrBarComplete    = CBCGPDrawManager::HLStoRGB_ONE (H, L1 - 0.20, S);
	colors.clrConnectorLines = CBCGPDrawManager::MixColors (clrMain, RGB(0, 0, 0),       0.85f);
	colors.clrGridLine0      = CBCGPDrawManager::MixColors (clrMain, RGB(192, 192, 192), 0.75f);
	colors.clrGridLine1      = CBCGPDrawManager::MixColors (clrMain, RGB(64, 64, 64),    0.75f);
	colors.clrShadows        = CBCGPDrawManager::MixColors (clrMain, RGB(32, 32, 32),    0.8f);
}

//////////////////////////////////////////////////////////////////////////
//                          Header options
//////////////////////////////////////////////////////////////////////////

void CBCGPGanttChart::SetHeaderVisible(UINT nHeader, BOOL bVisible)
{
	switch (nHeader)
	{
	case 0: 
		m_hdrLargeHeader.bVisible = bVisible;
		break;
	case 1:
		m_hdrSmallHeader.bVisible = bVisible;
		break;
	default:
		ASSERT (FALSE);
	}

	UPDATE_INFO updateLayout;
	updateLayout.Reset ();
	updateLayout.bLayoutChanged = TRUE;
	SetUpdateInfo (updateLayout);
}

BOOL CBCGPGanttChart::GetHeaderVisible(UINT nHeader) const
{
	switch (nHeader)
	{
	case 0:
		return m_hdrLargeHeader.bVisible;
	case 1:
		return m_hdrSmallHeader.bVisible;
	}
	return FALSE;
}


void CBCGPGanttChart::SetHeaderAboveChart(UINT nHeader, BOOL bAboveChart)
{
	switch (nHeader)
	{
	case 0: 
		m_hdrLargeHeader.bAboveChart = bAboveChart;
		break;
	case 1:
		m_hdrSmallHeader.bAboveChart = bAboveChart;
		break;
	default:
		ASSERT (FALSE);
	}

	UPDATE_INFO updateLayout;
	updateLayout.Reset ();
	updateLayout.bLayoutChanged = TRUE;
	SetUpdateInfo (updateLayout);
}

BOOL CBCGPGanttChart::GetHeaderAboveChart(UINT nHeader) const
{
	switch (nHeader)
	{
	case 0:
		return m_hdrLargeHeader.bAboveChart;
	case 1:
		return m_hdrSmallHeader.bAboveChart;
	}
	return FALSE;
}

void CBCGPGanttChart::SetHeaderTextAlignment(UINT nHeader, DWORD dwAlignment)
{
	switch (nHeader)
	{
	case 0: 
		m_hdrLargeHeader.dwAlignment = dwAlignment;
		break;
	case 1:
		m_hdrSmallHeader.dwAlignment = dwAlignment;
		break;
	default:
		ASSERT (FALSE);
	}

	UPDATE_INFO updateLayout;
	updateLayout.Reset ();
	updateLayout.bLayoutChanged = TRUE;
	updateLayout.bAppearanceChanged = TRUE;
	SetUpdateInfo (updateLayout);
}

DWORD CBCGPGanttChart::GetHeaderTextAlignment(UINT nHeader) const
{
	switch (nHeader)
	{
	case 0:
		return m_hdrLargeHeader.dwAlignment;
	case 1:
		return m_hdrSmallHeader.dwAlignment;
	}
	return FALSE;
}


COleDateTimeSpan CBCGPGanttChart::GetHeaderTimeDelta (UINT nHeader) const
{
	switch (nHeader)
	{
	case 0:
		return m_hdrLargeHeader.dtCellTimeDelta;
	case 1:
		return m_hdrSmallHeader.dtCellTimeDelta;
	default:
		ASSERT (FALSE);
	}
	return COleDateTimeSpan (0, 0, 0, 0);
}

struct CSpanFormat 
{
	double      m_tmMinSpan;
	DWORD       m_dwSpanType;
	LPCTSTR     m_pszDateFormat;
	DWORD       m_dwTimeFormat;
};

static const CSpanFormat s_arrSpanFormats[] =
{
	COleDateTimeSpan (0, 0, 15, 0),  DATETIMERANGE_1MINUTE, _T(""),         TIMEFORMAT_HOURS_AND_MINUTES,
	COleDateTimeSpan (0, 0, 15, 0),  DATETIMERANGE_1MINUTE, _T(""),         TIMEFORMAT_MINUTES_ONLY,
	COleDateTimeSpan (0, 0, 30, 0),  DATETIMERANGE_1MINUTE, _T(""),         TIMEFORMAT_HOURS_AND_MINUTES,
	COleDateTimeSpan (0, 0, 30, 0),  DATETIMERANGE_1MINUTE, _T(""),         TIMEFORMAT_MINUTES_ONLY,
	COleDateTimeSpan (0, 1, 0, 0),   DATETIMERANGE_1HOUR,   _T(""),         TIMEFORMAT_HOURS_AND_MINUTES,
	COleDateTimeSpan (0, 1, 0, 0),   DATETIMERANGE_1HOUR,   _T(""),         TIMEFORMAT_HOURS_ONLY,
	COleDateTimeSpan (0, 2, 0, 0),   DATETIMERANGE_1HOUR,   _T(""),         TIMEFORMAT_HOURS_AND_MINUTES,
	COleDateTimeSpan (0, 2, 0, 0),   DATETIMERANGE_1HOUR,   _T(""),         TIMEFORMAT_HOURS_ONLY,
	COleDateTimeSpan (0, 4, 0, 0),   DATETIMERANGE_1HOUR,   _T(""),         TIMEFORMAT_HOURS_AND_MINUTES,
	COleDateTimeSpan (0, 4, 0, 0),   DATETIMERANGE_1HOUR,   _T(""),         TIMEFORMAT_HOURS_ONLY,
	COleDateTimeSpan (1, 0, 0, 0),   DATETIMERANGE_DAY,     _T("Long"),     TIMEFORMAT_NONE,    // long date representation (full date)
	COleDateTimeSpan (1, 0, 0, 0),   DATETIMERANGE_DAY,     _T("Short"),    TIMEFORMAT_NONE,    // short date representation (full date)
	COleDateTimeSpan (1, 0, 0, 0),   DATETIMERANGE_DAY,     _T("d MMM"),    TIMEFORMAT_NONE,    // day and month abbreviation
	COleDateTimeSpan (1, 0, 0, 0),   DATETIMERANGE_DAY,     _T("dd"),       TIMEFORMAT_NONE,    // day only
	COleDateTimeSpan (28, 0, 0, 0),  DATETIMERANGE_MONTH,   _T("MMMM yyyy"),TIMEFORMAT_NONE,    // month and year
	COleDateTimeSpan (28, 0, 0, 0),  DATETIMERANGE_MONTH,   _T("MMMM"),     TIMEFORMAT_NONE,    // month (full)
	COleDateTimeSpan (28, 0, 0, 0),  DATETIMERANGE_MONTH,   _T("MMM"),      TIMEFORMAT_NONE,    // month (abbr)
	COleDateTimeSpan (28, 0, 0, 0),  DATETIMERANGE_MONTH,   _T("MM"),       TIMEFORMAT_NONE,    // month (numeric)
	COleDateTimeSpan (365, 0, 0, 0), DATETIMERANGE_YEAR,    _T("yyyy"),     TIMEFORMAT_NONE,    // year only
};

static CString FormatTime (COleDateTime datetime, DWORD dwTimeFormat, LCID locale = LOCALE_USER_DEFAULT, DWORD dwFlags = 0)
{
	TCHAR buf[256] = {0};

	SYSTEMTIME st;
	datetime.GetAsSystemTime (st);

	switch (dwTimeFormat)
	{
	case TIMEFORMAT_NONE:
		return CString ();
	case TIMEFORMAT_MINUTES_ONLY:
		::GetTimeFormat (locale, dwFlags, &st, _T("mm"), buf, 256);
		return CString(buf);
		break;
	case TIMEFORMAT_HOURS_AND_MINUTES:
		dwFlags |= TIME_NOSECONDS;
		break;
	case TIMEFORMAT_HOURS_ONLY:
		dwFlags |= TIME_NOMINUTESORSECONDS;
		break;
	default:
		TRACE0 ("Invalid flags\n");
		ASSERT (0);
		return CString ();
	}

	::GetTimeFormat (locale, dwFlags, &st, NULL, buf, 256);
	return CString(buf);
}

static CString FormatDate (COleDateTime datetime, const CString& sFormat, LCID locale = LOCALE_USER_DEFAULT, DWORD dwFlags = 0)
{
	TCHAR buf[256] = {0};

	SYSTEMTIME st;
	datetime.GetAsSystemTime (st);

	LPCTSTR pszFormat = sFormat;
	if (sFormat.CompareNoCase (_T("Long")) == 0)
	{
		pszFormat = NULL;
		dwFlags |= DATE_LONGDATE;
	}
	else if (sFormat.CompareNoCase (_T("Short")) == 0)
	{
		pszFormat = NULL;
		dwFlags |= DATE_SHORTDATE;
	}

	::GetDateFormat (locale, dwFlags, &st, pszFormat, buf, 256);
	return CString(buf);
}

void CBCGPGanttChart::UpdateHeaders()
{
	// m_dtPixelTimeDelta has changed. Recalculate headers.

	ASSERT ((double)m_dtPixelTimeDelta > 0.0);
	if ((double)m_dtPixelTimeDelta <= 0.0)
	{
		return;
	}

	// Detecting headers font metrics
	TEXTMETRIC tmHeaderFont;
	{
		CWindowDC dc (NULL);
		CFont* pOldFont = SelectFont (&dc, m_pFont);

		dc.GetTextMetrics (&tmHeaderFont);
		dc.SelectObject (pOldFont);
	}

	UINT nAverageCharWidth = (tmHeaderFont.tmAveCharWidth * 2 + tmHeaderFont.tmMaxCharWidth) / 3;

	// Expect array is ordered.
	
	const COleDateTime tmTest (1980, 11, 30, 23, 59, 59);

	// header 1 

	int i1, n = sizeof(s_arrSpanFormats) / sizeof(*s_arrSpanFormats);

	for (i1 = 0; i1 < n; ++i1)
	{
		CString sFormat = FormatDate (tmTest, s_arrSpanFormats[i1].m_pszDateFormat) + FormatTime (tmTest, s_arrSpanFormats[i1].m_dwTimeFormat);
		if (!sFormat.IsEmpty ())
		{
			int nFmtMaxWidth = (nAverageCharWidth * sFormat.GetLength ()) + tmHeaderFont.tmOverhang; // max width of text in pixels for this span
			int nSpanMinWidth = (int)(s_arrSpanFormats[i1].m_tmMinSpan / (double)m_dtPixelTimeDelta); // min width of this span (for current zoom)

			m_hdrSmallHeader.dtCellTimeDelta = s_arrSpanFormats[i1].m_tmMinSpan;
			m_hdrSmallHeader.sDateFormat     = s_arrSpanFormats[i1].m_pszDateFormat;
			m_hdrSmallHeader.dwTimeFormat    = s_arrSpanFormats[i1].m_dwTimeFormat;
			if (nSpanMinWidth >= nFmtMaxWidth + 3)
			{
				break;
			}
		}
	}

	// header 0 (Large)

	m_hdrLargeHeader.dtCellTimeDelta = s_arrSpanFormats[i1].m_tmMinSpan;
	m_hdrLargeHeader.sDateFormat     = s_arrSpanFormats[i1].m_pszDateFormat;
	m_hdrLargeHeader.dwTimeFormat    = s_arrSpanFormats[i1].m_dwTimeFormat;
	int i0;
	for (i0 = i1; i0 < n; ++i0)
	{
		if (s_arrSpanFormats[i0].m_dwSpanType == s_arrSpanFormats[i1].m_dwSpanType)
		{
			continue;
		}
		CString sFormat = FormatDate (tmTest, s_arrSpanFormats[i0].m_pszDateFormat) + FormatTime (tmTest, s_arrSpanFormats[i0].m_dwTimeFormat);
		if (!sFormat.IsEmpty ())
		{
			int nFmtMaxWidth = (tmHeaderFont.tmMaxCharWidth * sFormat.GetLength ()) + tmHeaderFont.tmOverhang; // max width of text in pixels for this span
			int nSpanMinWidth = (int)(s_arrSpanFormats[i0].m_tmMinSpan / (double)m_dtPixelTimeDelta); // min width of this span (for current zoom)
			m_hdrLargeHeader.dtCellTimeDelta = s_arrSpanFormats[i0].m_tmMinSpan;
			m_hdrLargeHeader.sDateFormat     = s_arrSpanFormats[i0].m_pszDateFormat;
			m_hdrLargeHeader.dwTimeFormat    = s_arrSpanFormats[i0].m_dwTimeFormat;

			if (nSpanMinWidth >= nFmtMaxWidth + 3) // 3 is an indent for cell borders
			{
				break;
			}
		}
	}

	// Set grid lines span

	m_dtGridLinesSmallSpan = m_hdrSmallHeader.dtCellTimeDelta;
	COleDateTimeSpan tmDay (1, 0, 0, 0), tmWeek (7, 0, 0, 0);
	
	int nDaySpanWidth = (int)((double)tmDay / (double)m_dtPixelTimeDelta); // in pixels

	if (nDaySpanWidth > 240)
	{
		m_dtGridLinesLargeSpan.SetDateTimeSpan (0, 1, 0, 0); //hour
	}
	else if (nDaySpanWidth > 8)
	{
		m_dtGridLinesLargeSpan.SetDateTimeSpan (1, 0, 0, 0); //1 day
	}
	else if (nDaySpanWidth >= 4)
	{
		m_dtGridLinesLargeSpan.SetDateTimeSpan (7, 0, 0, 0); //week
	}
	else
	{
		m_dtGridLinesLargeSpan.SetDateTimeSpan (28, 0, 0, 0); //month
		m_dtGridLinesSmallSpan.SetDateTimeSpan (365, 0, 0, 0); //year
	}

	if (m_dtGridLinesLargeSpan == m_dtGridLinesSmallSpan)
	{
		m_dtGridLinesLargeSpan = m_hdrLargeHeader.dtCellTimeDelta;
	}

	if (m_dtGridLinesLargeSpan < m_dtGridLinesSmallSpan)
	{
		COleDateTimeSpan t = m_dtGridLinesSmallSpan;
		m_dtGridLinesSmallSpan = m_dtGridLinesLargeSpan;
		m_dtGridLinesLargeSpan = t;
	}

	InvalidateHeaders ();
}

void CBCGPGanttChart::SetShowEmptyRows(BOOL bShow)
{
	if (m_bShowEmptyRows != bShow)
	{
		m_bShowEmptyRows = bShow;
		InvalidateChart ();
	}
}

//////////////////////////////////////////////////////////////////////////
//                  Aggregate objects control
//////////////////////////////////////////////////////////////////////////

CBCGPGanttRenderer* CBCGPGanttChart::QueryRenderer ()
{
	return new CBCGPGanttRenderer ();
}

void CBCGPGanttChart::ReleaseRenderer (CBCGPGanttRenderer* pRenderer)
{
	if (pRenderer != NULL)
	{
		delete pRenderer;
		pRenderer = NULL;
	}
}

CBCGPGanttRenderer* CBCGPGanttChart::GetRenderer ()
{
	if (m_pRenderer == NULL)
	{
		m_pRenderer = QueryRenderer ();
	}
	return m_pRenderer;
}


CBCGPGanttItemStorageBase* CBCGPGanttChart::QueryItemStorage ()
{
	return new CBCGPGanttItemStorage (this);
}

void CBCGPGanttChart::ReleaseItemStorage (CBCGPGanttItemStorageBase* pStorage)
{
	if (pStorage != NULL)
	{
		ASSERT_VALID (pStorage);
		delete pStorage;
		pStorage = NULL;
	}
}

//////////////////////////////////////////////////////////////////////////
//                      Rendering chart
//////////////////////////////////////////////////////////////////////////

void CBCGPGanttChart::DoPaint (CDC& dc)
{
	CRect rcClip;
	dc.GetClipBox(&rcClip);

	CRect rcChart = GetVisibleChartArea();
	rcChart.IntersectRect(&rcChart, &rcClip);
	DrawChartBackground(dc, rcChart);

	// Calculate rows that are really need update

	int nFirstVisibleRow = -1; // actual first visible row (depends on clipping rectangle)
	int nLastVisibleRow = -1; // actual last visible row (depends on clipping rectangle)
	int nItemRects = (int)m_arrItemRects.GetSize ();
	int i;
	for (i = m_nFirstVisibleRow; m_nFirstVisibleRow >= 0 && i <= m_nLastVisibleRow && i < nItemRects; ++i)
	{
		int nRowTop = m_arrItemRects[i].nRowTopY;
		int nRowBottom = m_arrItemRects[i].nRowBottomY;
		if (nRowTop <= rcChart.bottom && nRowBottom >= rcChart.top)
		{
			if (nFirstVisibleRow < 0)
			{
				nFirstVisibleRow = i;
			}
		}
		else if (nRowTop > rcChart.bottom)
		{
			nLastVisibleRow = i - 1;
			break;
		}
	}
	
	if (nLastVisibleRow < 0)
	{
		nLastVisibleRow = m_nLastVisibleRow;
	}
	if (nFirstVisibleRow < 0)
	{
		nFirstVisibleRow = m_nFirstVisibleRow;
	}

	// Drawing visible items background

	for (i = nFirstVisibleRow; (i <= nLastVisibleRow) && (i >= 0) && (i < nItemRects); ++i)
	{
		CRect rectRow (m_rectChart.left, m_arrItemRects[i].nRowTopY, m_rectChart.right, m_arrItemRects[i].nRowBottomY);
		DrawItemBackground (dc, rectRow, m_arrItemRects[i].pItem);
	}

	if (m_bShowEmptyRows)
	{
		CRect rectRow = rcChart;

		if (m_nFirstVisibleRow < 0 || m_nLastVisibleRow < 0 || m_nLastVisibleRow < m_nFirstVisibleRow) // no visible items
		{
			rectRow.bottom = rectRow.top;
		}
		else
		{
			rectRow.top = m_arrItemRects[m_nLastVisibleRow].nRowTopY;
			rectRow.bottom = m_arrItemRects[m_nLastVisibleRow].nRowBottomY;
		}
		while (rectRow.bottom < rcChart.bottom)
		{
			rectRow.top = rectRow.bottom;
			rectRow.bottom = rectRow.top + GetDefaultItemHeight ();
			DrawItemBackground (dc, rectRow, NULL);
		}
	}

	// Drawing vertical grid lines

	BCGP_GANTT_CHART_COLORS colors = GetActualColors ();

	int yBottom = rcChart.bottom;

	if ((double)m_dtPixelTimeDelta > 0.0 && (double)m_dtGridLinesSmallSpan / (double)m_dtPixelTimeDelta > 3.0)
	{
		CBCGPPenSelector pen (dc, colors.clrGridLine1, 1);

		COleDateTime t = m_dtLeftMostChartTime;
		while (IsTimeVisible (t))
		{
			COleDateTime tmStart, tmEnd;
			GetHeaderCellTimeRange (t, m_dtGridLinesSmallSpan, tmStart, tmEnd);

			int x = TimeToClient (tmStart) - 1;
			dc.MoveTo (x, rcChart.top);
			dc.LineTo (x, yBottom);

			t = tmEnd;
		}
	}

	if ((double)m_dtPixelTimeDelta > 0.0 && (double)m_dtGridLinesLargeSpan / (double)m_dtPixelTimeDelta > 3.0)
	{
		CBCGPPenSelector pen (dc, colors.clrGridLine0, 1, PS_DOT);

		COleDateTime t = m_dtLeftMostChartTime;
		while (IsTimeVisible (t))
		{
			COleDateTime tmStart, tmEnd;
			GetHeaderCellTimeRange (t, m_dtGridLinesLargeSpan, tmStart, tmEnd);

			int x = TimeToClient (tmStart) - 1;
			dc.MoveTo (x, rcChart.top);
			dc.LineTo (x, yBottom);

			t = tmEnd;
		}
	}

	// Drawing headers

	DrawHeader (dc, m_hdrLargeHeader, m_rectLargeHeader);
	DrawHeader (dc, m_hdrSmallHeader, m_rectSmallHeader);

	CRgn rgnChart;
	rgnChart.CreateRectRgnIndirect (rcChart);

	// Drawing item shadows

	CBCGPGanttRenderer* pRenderer = GetRenderer ();
	ASSERT_VALID (pRenderer);

	COLORREF clrShadow = GetActualColors ().clrShadows;
	for (i = nFirstVisibleRow; (i <= nLastVisibleRow) && (i >= 0) && (i < nItemRects); ++i)
	{
		const CBCGPGanttItem* pItem = m_arrItemRects[i].pItem;

		dc.SelectClipRgn (&rgnChart);
		CBCGPGanttDrawContext ctxDraw(*pItem, dc);
		ctxDraw.m_clrFill = clrShadow;
		ctxDraw.m_rectClip = rcChart; // An intersection of actual chart and the clip rect. Does not include headers area.
		ctxDraw.m_rectBar = m_arrItemRects[i].rectBar;

		pRenderer->DrawShadow (ctxDraw);
	}

	// Drawing connectors

	CArray <CBCGPGanttConnection*, CBCGPGanttConnection*> arrLinks;
	m_pItems->GetAllConnections (arrLinks);

	dc.SelectClipRgn (&rgnChart);

	COLORREF clrLines = GetActualColors().clrConnectorLines;

	for (i = 0; i < arrLinks.GetSize (); ++i)
	{
		CBCGPGanttConnection* link = arrLinks[i];
		if (link->m_pSourceItem != NULL)
		{
			CBCGPGanttDrawContext ctxDraw (*(link->m_pSourceItem), dc);
			ctxDraw.m_clrFill = clrLines;
			ctxDraw.m_rectClip = rcChart;
			ctxDraw.m_rectBar = GetBarRect (link->m_pSourceItem);
			pRenderer->DrawConnection (ctxDraw, *link);
		}
	}

	// Drawing item bars

	COLORREF clrSelection = GetActualColors().clrSelection;
	COLORREF clrSelectionBorder = GetActualColors().clrSelectionBorder;

	BOOL bDraggingItems = m_nControlState == ecsMoveItems || m_nControlState == ecsResizeItems;

	for (i = nFirstVisibleRow; (i <= nLastVisibleRow) && (i >= 0) && (i < nItemRects); ++i)
	{
		const CBCGPGanttItem* pItem = m_arrItemRects[i].pItem;
		BOOL  bSelected = pItem->IsSelected ();

		dc.SelectClipRgn (&rgnChart);
		CBCGPGanttDrawContext ctxDraw(*pItem, dc);
		SetupItemDrawContext (ctxDraw);
		ctxDraw.m_rectClip = rcChart;
		ctxDraw.m_rectBar = m_arrItemRects[i].rectBar;

		pRenderer->DrawBar (ctxDraw);

		if (bSelected)
		{
			ctxDraw.m_clrFill = clrSelection;
			ctxDraw.m_clrBorder = clrSelectionBorder;
			pRenderer->DrawSelection (ctxDraw);
		}
	}

	// Drawing items being dragged
	if (bDraggingItems)
	{
		for (i = nFirstVisibleRow; (i <= nLastVisibleRow) && (i >= 0) && (i < nItemRects); ++i)
		{
			const CBCGPGanttItem* pItem = m_arrItemRects[i].pItem;
			if (pItem->IsSelected ())
			{
				CRect rcBar = m_arrItemRects[i].rectBar;
				if (m_nControlState == ecsMoveItems)
				{
					rcBar.OffsetRect (m_szDragOffset);
				}
				else if (m_nControlState == ecsResizeItems)
				{
					rcBar.right += m_szDragOffset.cx;
				}
				CRect rcClip;
				rcClip.IntersectRect(&rcBar, &rcChart);
				if (!rcClip.IsRectEmpty () && rcBar.right > rcBar.left)
				{
					CBCGPAlphaDC alphaDC (dc, rcClip, 0.32f);
					CBCGPGanttDrawContext ctxDraw (*pItem, alphaDC);
					SetupItemDrawContext (ctxDraw);
					ctxDraw.m_rectClip = rcClip;
					ctxDraw.m_rectBar = rcBar;
					pRenderer->DrawBar (ctxDraw);
				}
			}
		}
	}

	dc.SelectClipRgn (NULL);
}

void CBCGPGanttChart::SetupItemDrawContext(CBCGPGanttDrawContext& ctx) const
{
	ctx.m_clrFill = ctx.m_Item.GetPrimaryColor ();
	ctx.m_clrFill2 = ctx.m_Item.GetCompleteColor ();

	BCGP_GANTT_CHART_COLORS colors = GetActualColors ();

	if (ctx.m_clrFill == CLR_DEFAULT)
	{
		ctx.m_clrFill  = ctx.m_Item.IsGroupItem () ? colors.clrGroupBarFill : colors.clrBarFill;
	}
	if (ctx.m_clrFill2 == CLR_DEFAULT)
	{
		ctx.m_clrFill2 = colors.clrBarComplete;
	}

	ctx.m_clrBorder = CBCGPDrawManager::MixColors (ctx.m_clrFill, RGB(48, 48, 48), 0.75);
}

void CBCGPGanttChart::GetHeaderCellTimeRange (const COleDateTime& time, COleDateTimeSpan minSpan, COleDateTime& tmStart, COleDateTime& tmEnd) const
{
	// Note: tmEnd is not included in column range
	// [tmStart; tmEnd)

	DWORD dwRangeType = DATETIMERANGE_1SECOND;

	if (minSpan.GetDays() > 28)
	{
		dwRangeType = DATETIMERANGE_YEAR;
	}
	else if (minSpan.GetDays() > 7)
	{
		dwRangeType = DATETIMERANGE_MONTH;
	}
	else if (minSpan.GetDays() > 1)
	{
		dwRangeType = DATETIMERANGE_WEEK;
	}
	else if (minSpan.GetDays() == 1 || minSpan.GetHours() > 4)
	{
		dwRangeType = DATETIMERANGE_DAY;
	}
	else if (minSpan.GetHours() > 1)
	{
		dwRangeType = DATETIMERANGE_4HOURS;
	}
	else if (minSpan.GetHours() == 1)
	{
		dwRangeType = DATETIMERANGE_1HOUR;
	}
	else
	{
		dwRangeType = DATETIMERANGE_15MINUTES;
	}

	GetDateTimeRange (time, dwRangeType, tmStart, tmEnd);
}

BOOL CBCGPGanttChart::IsWorkingTime (const CBCGPGanttItem* /*pItem*/, COleDateTime day) const
{
	int w = day.GetDayOfWeek ();
	return (w != 1 && w != 7); // Sunday or Saturday
}

COleDateTime CBCGPGanttChart::SnapDateToGrid (COleDateTime dateTime) const
{
	COleDateTime dtLargeStart, dtLargeEnd;
	GetHeaderCellTimeRange (dateTime, GetGridLinesSpan (TRUE), dtLargeStart, dtLargeEnd);

	COleDateTime dtSmallStart, dtSmallEnd;
	GetHeaderCellTimeRange (dateTime, GetGridLinesSpan (FALSE), dtSmallStart, dtSmallEnd);

	double t  = (double)dateTime;

	double d[4] = 
	{
		fabs((double)dtLargeStart - t),
		fabs((double)dtLargeEnd - t),
		fabs((double)dtSmallStart - t),
		fabs((double)dtSmallEnd - t)
	};

	double dMin = d[0];
	int i = 0, iMin = 0;

	for (i = 1; i < 4; ++i)
	{
		if (d[i] < dMin)
		{
			iMin = i;
			dMin = d[i];
		}
	}

	switch (iMin)
	{
	case 0:
		return dtLargeStart;
	case 1:
		return dtLargeEnd;
	case 2:
		return dtSmallStart;
	case 3:
		return dtSmallEnd;
	}

	return dateTime;
}


BOOL CBCGPGanttChart::HeaderCellFromPoint (CPoint pt, BCGP_GANTT_CHART_HEADER_CELL_INFO& cellInfo) const
{
	if (m_rectLargeHeader.PtInRect (pt))
	{
		cellInfo.pHeaderInfo = &m_hdrLargeHeader;
		cellInfo.rectCell = m_rectLargeHeader;
	}
	else if (m_rectSmallHeader.PtInRect (pt))
	{
		cellInfo.pHeaderInfo = &m_hdrSmallHeader;
		cellInfo.rectCell = m_rectSmallHeader;
	}
	else
	{
		return FALSE;
	}

	COleDateTime t = m_dtLeftMostChartTime;
	while (IsTimeVisible (t))
	{
		COleDateTime tmStart, tmFinish;
		GetHeaderCellTimeRange (t, cellInfo.pHeaderInfo->dtCellTimeDelta, tmStart, tmFinish);
		int xLeft  =  TimeToClient (tmStart);
		int xRight =  TimeToClient (tmFinish);
		if (pt.x >= xLeft && pt.x < xRight)
		{
			CRect rcChart = GetVisibleChartArea ();

			cellInfo.rectCell.left = xLeft;
			cellInfo.rectCell.right = xRight;
			cellInfo.rectColumn = cellInfo.rectCell;
			cellInfo.rectColumn.top = rcChart.top;
			cellInfo.rectColumn.bottom = rcChart.bottom;
			cellInfo.dtCellLeftMostTime = tmStart;
			cellInfo.dtCellTimeDelta = (tmFinish - tmStart);
			cellInfo.rectClip.SetRect (0, 0, 0, 0);
			return TRUE;
		}

		t = tmFinish;
	}

	return TRUE;
}

void CBCGPGanttChart::DrawHeader (CDC& dc, const BCGP_GANTT_CHART_HEADER& header, const CRect& rcPaint) const
{
	CRect rcChart = GetVisibleChartArea ();
	COleDateTime t = m_dtLeftMostChartTime;

	while (IsTimeVisible (t))
	{
		COleDateTime tmStart, tmEnd;
		GetHeaderCellTimeRange (t, header.dtCellTimeDelta, tmStart, tmEnd);
		BCGP_GANTT_CHART_HEADER_CELL_INFO ci;
		ci.pHeaderInfo = &header;
		ci.rectCell = rcPaint;
		ci.rectCell.left = TimeToClient (tmStart);
		ci.rectCell.right = TimeToClient (tmEnd);
		ci.rectColumn = ci.rectCell;
		ci.rectColumn.top = rcChart.top;
		ci.rectColumn.bottom = rcChart.bottom;
		ci.dtCellLeftMostTime = tmStart;
		ci.dtCellTimeDelta = (tmEnd - tmStart);
		ci.rectClip = rcPaint;

		if (header.bVisible && header.nHeight > 0)
		{
			CString sText = FormatDate (t, header.sDateFormat) + FormatTime (t, header.dwTimeFormat);
			DrawHeaderCell (dc, ci, FALSE);
			DrawHeaderText (dc, ci, sText, FALSE);
		}

		t = tmEnd;
	}
}

void CBCGPGanttChart::DrawHeaderCell (CDC& dc, const BCGP_GANTT_CHART_HEADER_CELL_INFO& cellInfo, BOOL bHilite) const
{
	CBCGPVisualManager::GetInstance ()->DrawGanttHeaderCell (this, dc, cellInfo, bHilite);
}

void CBCGPGanttChart::DrawHeaderText (CDC& dc, const BCGP_GANTT_CHART_HEADER_CELL_INFO& cellInfo, const CString& sCellText, BOOL bHilite) const
{
	CBCGPVisualManager::GetInstance ()->DrawGanttHeaderText (this, dc, cellInfo, sCellText, bHilite);
}

void CBCGPGanttChart::DrawChartBackground (CDC& dc, const CRect& rectChart) const
{
	CBCGPVisualManager::GetInstance ()->DrawGanttChartBackground (this, dc, rectChart, GetActualColors ().clrBackground);
}

void CBCGPGanttChart::DrawItemBackground (CDC& dc, const CRect& rcItem, const CBCGPGanttItem* pItem) const
{
	CRect r = rcItem;
	r.bottom --;

	CBCGPPenSelector pen (dc, GetActualColors ().clrGridLine1, 1);
	dc.MoveTo (r.left, r.bottom);
	dc.LineTo (r.right, r.bottom);

	CRect rcClip;
	dc.GetClipBox (&rcClip);
	rcClip.IntersectRect (&rcClip, &r);
	if (rcClip.IsRectEmpty ())
	{
		return;
	}
	
	BOOL bShowDaysOff = TRUE;
	COleDateTimeSpan tmDay (1, 0, 0, 0);   // Check if scale is large enough so days-off can be shown
	double dPixelsPerDay =  (double)tmDay / (double)m_dtPixelTimeDelta;
	if (dPixelsPerDay < 4.0) // one day must be at least 4 pixels wide to show days-off
	{
		bShowDaysOff = FALSE;
	}

	COleDateTime t = ClientToTime (rcClip.left);
	COleDateTime tEnd = ClientToTime (rcClip.right);
	DWORD dwRangeType = DATETIMERANGE_DAY;

	COleDateTime tmRangeStart = t;
	BOOL bPrevIsWT = IsWorkingTime (pItem, t) || !bShowDaysOff;

	CBCGPVisualManager* pVisualManager = CBCGPVisualManager::GetInstance ();
	ASSERT_VALID (pVisualManager);

	BCGP_GANTT_CHART_COLORS colors = GetActualColors ();
	for (;;)
	{
		COleDateTime tmStart;
		COleDateTime tmEnd;
		GetDateTimeRange (t, dwRangeType, tmStart, tmEnd);

		BOOL bIsWT = IsWorkingTime (pItem, tmStart) || !bShowDaysOff;

		if (bIsWT != bPrevIsWT || tmStart >= tEnd)
		{
			CRect rcFill = rcClip;
			rcFill.left = TimeToClient (tmRangeStart);
			rcFill.right = TimeToClient (tmStart); // start of current range

			pVisualManager->DrawGanttItemBackgroundCell (this, dc, rcItem, rcFill, colors, !bPrevIsWT);

			tmRangeStart = tmStart;
			bPrevIsWT = bIsWT;
		}

		if (tmStart > tEnd)
		{
			break;
		}

		t = tmEnd;
	}
}

//////////////////////////////////////////////////////////////////////////
//							Size calculations
//////////////////////////////////////////////////////////////////////////

int CBCGPGanttChart::CalculateScrollHeight () const
{
	int height = 0;
	POSITION pos = m_pItems->GetHeadPosition ();
	while (pos != NULL)
	{
		height += GetItemHeight (m_pItems->GetNext (pos));
	}
	return height;
}

void CBCGPGanttChart::SetDefaultItemHeight (UINT nHeight)
{
	if (m_nItemDefaultHeight != nHeight)
	{
		m_nItemDefaultHeight = nHeight;

		UPDATE_INFO updateLayout;
		updateLayout.Reset ();
		updateLayout.bItemsLayoutChanged = TRUE;
		SetUpdateInfo (updateLayout);
	}
}

UINT CBCGPGanttChart::GetDefaultItemHeight () const
{
	return m_nItemDefaultHeight;
}

UINT CBCGPGanttChart::GetItemHeight (const CBCGPGanttItem* pItem) const
{
	return (pItem != NULL && pItem->IsVisible ()) ? GetDefaultItemHeight () : 0;
}

int CBCGPGanttChart::GetItemBarHeight (const CBCGPGanttItem* /*pItem*/) const
{
	return max (12, m_nItemDefaultHeight - 12); // the bar height is constant for all items
}

void CBCGPGanttChart::SetHeaderHeights (UINT nHeaderHeight, UINT nSubHeaderHeight)
{
	UINT hMin = GetFontHeight (m_pFont);
	nHeaderHeight = max (nHeaderHeight, hMin);
	nSubHeaderHeight = max (nSubHeaderHeight, hMin);

	// Applying changes

	if (nHeaderHeight == m_hdrLargeHeader.nHeight && nSubHeaderHeight == m_hdrSmallHeader.nHeight)
	{
		return;
	}

	m_hdrLargeHeader.nHeight = nHeaderHeight;
	m_hdrSmallHeader.nHeight = nSubHeaderHeight;

	PerformAdjustLayout ();
}

void CBCGPGanttChart::GetHeaderHeights (UINT* pHeaderHeight, UINT* pSubHeaderHeight) const
{
	if (pHeaderHeight != NULL)
	{
		*pHeaderHeight = m_hdrLargeHeader.nHeight;
	}
	if (pSubHeaderHeight != NULL)
	{
		*pSubHeaderHeight = m_hdrSmallHeader.nHeight;
	}
}

void CBCGPGanttChart::SetUpdateInfo (const UPDATE_INFO& updateInfo)
{
	ASSERT (m_pUpdateInfo != NULL);

	m_pUpdateInfo->Overwrite(updateInfo);
	if (m_pUpdateInfo->NeedsUpdate ())
	{
		Invalidate ();
	}
}

void CBCGPGanttChart::AdjustLayout()
{
	UPDATE_INFO updateAll;
	SetUpdateInfo (updateAll);
	Invalidate ();
}

void CBCGPGanttChart::UpdateItemsLayout ()
{
	if (m_pUpdateInfo->bItemsLayoutChanged)
	{
		// Update all items layout

		m_arrItemRects.RemoveAll ();

		int y = GetFirstItemOffset ();

		POSITION pos = m_pItems->GetHeadPosition ();

		while (pos != NULL)
		{
			CBCGPGanttItem* pCurrentItem = m_pItems->GetNext (pos);
			int h = GetItemHeight (pCurrentItem);
			int nBarHeight = GetItemBarHeight (pCurrentItem); // virtual call
			nBarHeight = min (nBarHeight, h - 4);
			if (h > 0)
			{
				CRowLayout r;
				r.pItem = pCurrentItem;

				r.nRowTopY = y;
				r.nRowBottomY = y + h;

				r.rectBar.left = TimeToClient (pCurrentItem->GetStartTime ());
				r.rectBar.right = TimeToClient (pCurrentItem->GetFinishTime ());
				r.rectBar.top = y + (h - nBarHeight) / 2;
				r.rectBar.bottom = r.rectBar.top + nBarHeight;

				if (pCurrentItem->IsMileStone ())
				{
					r.rectBar.left -= (nBarHeight / 2) + 1;
					r.rectBar.right = r.rectBar.left + nBarHeight + 1;
					r.rectBar.bottom += 1;
				}

				m_arrItemRects.Add (r);
			}
			y += h;
		}

		CArray<CBCGPGanttConnection*, CBCGPGanttConnection*> arrLinks;
		m_pItems->GetAllConnections (arrLinks);
		UpdateConnections (arrLinks);

		m_pUpdateInfo->bConnectionsChanged = FALSE;
		m_pUpdateInfo->bItemsLayoutChanged = FALSE;
	}
	else if (m_pUpdateInfo->nHScrollDelta != 0 || m_pUpdateInfo->nVScrollDelta != 0)
	{
		// Just scroll items

		for (int n = (int)m_arrItemRects.GetSize (); n > 0; --n)
		{
			m_arrItemRects[n - 1].nRowTopY -= m_pUpdateInfo->nVScrollDelta;
			m_arrItemRects[n - 1].nRowBottomY -= m_pUpdateInfo->nVScrollDelta;
			m_arrItemRects[n - 1].rectBar.OffsetRect (- m_pUpdateInfo->nHScrollDelta, - m_pUpdateInfo->nVScrollDelta);
		}

		m_pUpdateInfo->nHScrollDelta = 0;
		m_pUpdateInfo->nVScrollDelta = 0;
	}

}

void CBCGPGanttChart::PerformAdjustLayout()
{
	if (GetSafeHwnd () == NULL || m_bInAdjustLayout || m_nControlState == ecsDestroying)
	{
		return;
	}

	m_bInAdjustLayout = TRUE;
	{
		UpdateScrollBars (FALSE);

		HiliteHeaderCell (NULL);

		CRect rcClient;
		GetClientRect (&rcClient);

		UINT nToolTipId = 1;

		BOOL bHorzBarVisible = (m_wndScrollHorz.GetSafeHwnd () != NULL) && (m_wndScrollHorz.IsWindowVisible ());
		BOOL bVertBarVisible = (m_wndScrollVert.GetSafeHwnd () != NULL) && (m_wndScrollVert.IsWindowVisible ());
		if (bHorzBarVisible)
		{
			rcClient.bottom -= ::GetSystemMetrics (SM_CXHSCROLL);
		}
		if (bVertBarVisible)
		{
			rcClient.right -= ::GetSystemMetrics (SM_CXVSCROLL);
		}

		// Calculate headers and chart areas

		m_rectChart = rcClient;
		m_rectLargeHeader.SetRectEmpty ();
		m_rectSmallHeader.SetRectEmpty ();

		if (m_hdrLargeHeader.bVisible)
		{
			m_rectLargeHeader = m_rectChart;
			if (m_hdrLargeHeader.bAboveChart)
			{
				m_rectLargeHeader.top = m_rectChart.top;
				m_rectLargeHeader.bottom = m_rectLargeHeader.top + m_hdrLargeHeader.nHeight;
				m_rectChart.top = m_rectLargeHeader.bottom;
			}
			else
			{
				m_rectLargeHeader.bottom = m_rectChart.bottom;
				m_rectLargeHeader.top = m_rectLargeHeader.bottom - m_hdrLargeHeader.nHeight;
				m_rectChart.bottom = m_rectLargeHeader.top;
			}
			if (m_pToolTip != NULL && m_bShowTooltips)
			{
				m_pToolTip->AddTool (this, LPSTR_TEXTCALLBACK, &m_rectLargeHeader, nToolTipId ++);
			}
		}

		if (m_hdrSmallHeader.bVisible)
		{
			m_rectSmallHeader = m_rectChart;
			if (m_hdrSmallHeader.bAboveChart)
			{
				m_rectSmallHeader.top = m_rectChart.top;
				m_rectSmallHeader.bottom = m_rectSmallHeader.top + m_hdrSmallHeader.nHeight;
				m_rectChart.top = m_rectSmallHeader.bottom;
			}
			else
			{
				m_rectSmallHeader.bottom = m_rectChart.bottom;
				m_rectSmallHeader.top = m_rectSmallHeader.bottom - m_hdrSmallHeader.nHeight;
				m_rectChart.bottom = m_rectSmallHeader.top;
			}
			if (m_pToolTip != NULL && m_bShowTooltips)
			{
				m_pToolTip->AddTool (this, LPSTR_TEXTCALLBACK, &m_rectSmallHeader, nToolTipId ++);
			}
		}

		if (m_pUpdateInfo->bLayoutChanged)
		{
			CBCGPGanttControl* pGanttNotify = GetGanttControl ();
			if (pGanttNotify != NULL)
			{
				pGanttNotify->DoVerticalResize (this);
			}
		}

		UpdateItemsLayout ();

		int n = (int)m_arrItemRects.GetSize ();
		m_nFirstVisibleRow = -1;
		m_nLastVisibleRow = -1;
		for (int i = 0; i < n; ++i)
		{
			const CRowLayout& r = m_arrItemRects[i];
			if (r.nRowTopY <= m_rectChart.bottom && r.nRowBottomY >= m_rectChart.top)
			{
				if (m_nFirstVisibleRow < 0)
				{
					m_nFirstVisibleRow = i;
				}
			}
			else if (r.nRowTopY > m_rectChart.bottom)
			{
				m_nLastVisibleRow = i - 1;
			}
		}

		if (m_nLastVisibleRow < 0)
		{
			m_nLastVisibleRow = n - 1;
		}

		// Dynamic item tooltip (for whole chart area)
		if (m_pToolTip != NULL && m_bShowTooltips)
		{
			m_pToolTip->AddTool (this, LPSTR_TEXTCALLBACK, m_rectChart, nToolTipId ++);
		}

		UpdateScrollBars (TRUE); // Update scrollbar values

	} // end of Adjust layout code block


	m_pUpdateInfo->Reset ();
	m_bInAdjustLayout = FALSE;
}

CBCGPGanttControl* CBCGPGanttChart::GetGanttControl () const
{
	CWnd* pParent = GetParent ();
	if (pParent == NULL)
	{
		return NULL;
	}
	CWnd* pParent2 = pParent->GetParent ();
	if (pParent2 == NULL)
	{
		pParent2 = pParent;
	}
	CBCGPGanttControl* pGanttCtrl =  DYNAMIC_DOWNCAST (CBCGPGanttControl, pParent2);
	if (pGanttCtrl == NULL)
	{
		return NULL;
	}
	return static_cast<CBCGPGanttControl*>(pGanttCtrl);
}

CWnd* CBCGPGanttChart::GetNotificationWindow () const
{
	CBCGPGanttControl* pControl = GetGanttControl ();

	if (pControl != NULL)
	{
		return pControl;
	}

	return GetParent ();
}

CRect CBCGPGanttChart::GetVisibleChartArea() const
{
	return m_rectChart;
}

int CBCGPGanttChart::GetFirstItemOffset() const
{
	return GetVisibleChartArea ().top - m_nVerticalOffset;
}

CRect CBCGPGanttChart::GetItemRow(const CBCGPGanttItem* pItem) const
{
	int n = (int)m_arrItemRects.GetSize ();

	for (int i = 0; i < n; ++i)
	{
		if (m_arrItemRects[i].pItem == pItem)
		{
			CRect rectRow (m_rectChart.left, m_arrItemRects[i].nRowTopY, m_rectChart.right, m_arrItemRects[i].nRowBottomY);
			return rectRow;
		}
	}

	return CRect (0, 0, 0, 0); // specified row is not visible
}

CRect CBCGPGanttChart::GetBarRect(const CBCGPGanttItem* pItem) const
{
	int n = (int)m_arrItemRects.GetSize ();
	for (int i = 0; i < n; ++i)
	{
		if (m_arrItemRects[i].pItem == pItem)
		{
			return m_arrItemRects[i].rectBar;
		}
	}
	return CRect (0, 0, 0, 0); // specified bar was not found.
}

//////////////////////////////////////////////////////////////////////////
//                       Scale and time functions
//////////////////////////////////////////////////////////////////////////

void  CBCGPGanttChart::SetTimeDelta (COleDateTimeSpan dtPixelTimeDelta, BOOL bKeepCenterDate)
{
	if (dtPixelTimeDelta != m_dtPixelTimeDelta && dtPixelTimeDelta.GetTotalSeconds () > 0)
	{
		int nChartWidth = GetVisibleChartArea ().Width ();
		COleDateTime dtCenter = m_dtLeftMostChartTime + COleDateTimeSpan (m_dtPixelTimeDelta * nChartWidth / 2);
		m_dtPixelTimeDelta = dtPixelTimeDelta;

		m_dtHScrollMin = m_dtItemsMin - m_hdrSmallHeader.dtCellTimeDelta;
		m_dtHScrollMax = m_dtItemsMax + m_hdrSmallHeader.dtCellTimeDelta;

		if (bKeepCenterDate)
		{
			COleDateTime dtLeft = dtCenter - COleDateTimeSpan (dtPixelTimeDelta * nChartWidth / 2);
			UpdateScrollTimeRange (dtLeft);
			SetLeftmostDateTime (dtLeft);
		}

		UpdateHeaders ();
		DoScaleChanged ();
		UpdateScrollBars (TRUE);

		UPDATE_INFO updateLayout;
		updateLayout.Reset ();
		updateLayout.bItemsLayoutChanged = TRUE;
		SetUpdateInfo (updateLayout);
	}
}

static COleDateTimeSpan s_arrAvailableScales[] =
{
	COleDateTimeSpan(0, 0, 0, 30),
	COleDateTimeSpan(0, 0, 0, 45),
	COleDateTimeSpan(0, 0, 2, 0),
	COleDateTimeSpan(0, 0, 4, 0),
	COleDateTimeSpan(0, 0, 8, 0),
	COleDateTimeSpan(0, 0, 12, 0),
	COleDateTimeSpan(0, 0, 15, 0),
	COleDateTimeSpan(0, 0, 20, 0),
	COleDateTimeSpan(0, 0, 30, 0),
	COleDateTimeSpan(0, 0, 40, 0),
	COleDateTimeSpan(0, 1, 0, 0),
	COleDateTimeSpan(0, 1, 30, 0),
	COleDateTimeSpan(0, 2, 0, 0),
	COleDateTimeSpan(0, 3, 0, 0),
	COleDateTimeSpan(0, 4, 0, 0),
	COleDateTimeSpan(0, 6, 0, 0),
	COleDateTimeSpan(0, 8, 0, 0),
	COleDateTimeSpan(0, 12, 0, 0),
};

void  CBCGPGanttChart::IncreaseScale ()
{
	if (!DoScaleChanging (TRUE)) // this allows user to use his own algorithm
	{
		// use internal table
		const int nScales = sizeof(s_arrAvailableScales) / sizeof(*s_arrAvailableScales);
		COleDateTimeSpan tmPrev = s_arrAvailableScales[0];
		for (int i = 0; i < nScales; ++i)
		{
			if (s_arrAvailableScales[i] >= m_dtPixelTimeDelta)
			{
				SetTimeDelta (tmPrev, TRUE);
				break;
			}
			tmPrev = s_arrAvailableScales[i];
		}
	}
}

void  CBCGPGanttChart::DecreaseScale ()
{
	if (!DoScaleChanging (FALSE)) // this allows user to use his own algorithm
	{
		// use internal table
		const int nScales = sizeof(s_arrAvailableScales) / sizeof(*s_arrAvailableScales);
		COleDateTimeSpan tmPrev = s_arrAvailableScales[0];
		for (int i = 0; i < nScales; ++i)
		{
			if (s_arrAvailableScales[i] > m_dtPixelTimeDelta)
			{
				SetTimeDelta (s_arrAvailableScales[i], TRUE);
				break;
			}
			tmPrev = s_arrAvailableScales[i];
		}
	}
}

BOOL CBCGPGanttChart::IsTimeVisible(COleDateTime time) const
{
	COleDateTime tmStart, tmEnd;
	GetVisibleTimeRange (&tmStart, &tmEnd);
	return (time >= tmStart && time < tmEnd);
}

int CBCGPGanttChart::TimeToClient(COleDateTime time) const
{
	if ((double)m_dtPixelTimeDelta <= 0.0)
	{
		TRACE0 ("Invalid m_dtPixelTimeDelta value!\n");
		ASSERT (FALSE);
		return -1;
	}

	COleDateTimeSpan t = time - m_dtLeftMostChartTime;
	return GetVisibleChartArea ().left + (int)(0.5 + (double)t / (double)m_dtPixelTimeDelta);
}

COleDateTime CBCGPGanttChart::ClientToTime(int x) const
{
	int chartX = x - GetVisibleChartArea ().left;
	return m_dtLeftMostChartTime + (COleDateTimeSpan)(m_dtPixelTimeDelta * chartX);
}

void CBCGPGanttChart::GetVisibleTimeRange(COleDateTime* ptmStart, COleDateTime* ptmEnd) const
{
	if (ptmStart != NULL)
	{
		*ptmStart = ClientToTime (GetVisibleChartArea ().left);
	}
	if (ptmEnd != NULL)
	{
		*ptmEnd = ClientToTime (GetVisibleChartArea ().right);
	}
}

void CBCGPGanttChart::SetVisibleTimeRange (COleDateTime tmStart, COleDateTime tmEnd)
{
	ASSERT (tmEnd > tmStart);
	if (tmEnd <= tmStart)
	{
		tmEnd = tmStart + COleDateTimeSpan (1, 0, 0, 0);
	}

	SetLeftmostDateTime (tmStart);

	int nPixels = GetVisibleChartArea ().Width ();
	if (nPixels < 8)
	{
		return;
	}

	SetTimeDelta ((tmEnd - tmStart) / nPixels, FALSE);
}

void CBCGPGanttChart::GotoDateTime (COleDateTime time)
{
	SetLeftmostDateTime (time - COleDateTimeSpan (m_dtPixelTimeDelta * GetVisibleChartArea ().Width () / 2));
}

void CBCGPGanttChart::SetLeftmostDateTime (const COleDateTime& time)
{
	if (m_dtLeftMostChartTime != time)
	{
		UPDATE_INFO updateTime;
		updateTime.Reset ();
		updateTime.bItemsLayoutChanged = TRUE;
			//updateTime.nHScrollDelta = TimeToClient(time) - TimeToClient(m_dtLeftMostChartTime);
		SetUpdateInfo (updateTime);

		m_dtLeftMostChartTime = time;
	}
}

//////////////////////////////////////////////////////////////////////////
//              Horizontal scroll range functions
//////////////////////////////////////////////////////////////////////////

COleDateTimeSpan CBCGPGanttChart::GetScrollTimeRange (COleDateTime* pdtStart, COleDateTime* pdtEnd) const
{
	COleDateTimeSpan dtGridSpan = GetGridLinesSpan ();
	COleDateTimeSpan dtVisibleSpan = m_dtPixelTimeDelta * GetVisibleChartArea ().Width ();
	COleDateTime dtLeft = min (m_dtLeftMostChartTime, m_dtItemsMin - dtGridSpan);
	COleDateTime dtRight = max (m_dtLeftMostChartTime + dtVisibleSpan, m_dtItemsMax + dtGridSpan);

	ASSERT (dtLeft < dtRight);

	if (pdtStart != NULL)
	{
		*pdtStart = dtLeft;
	}

	if (pdtEnd != NULL)
	{
		*pdtEnd = dtRight;
	}

	return (dtRight - dtLeft);
}

void CBCGPGanttChart::UpdateItemsTimeRange (CBCGPGanttItem* pItemChanged)
{
	if (pItemChanged == NULL)
	{
		return;
	}

	COleDateTime dtTime = pItemChanged->GetStartTime ();

	if (dtTime < m_dtItemsMin)
	{
		m_dtItemsMin = dtTime;
	}
	else if (dtTime > m_dtItemsMax)
	{
		m_dtItemsMax = dtTime;
	}

	dtTime = pItemChanged->GetFinishTime ();

	if (dtTime < m_dtItemsMin)
	{
		m_dtItemsMin = dtTime;
	}
	else if (dtTime > m_dtItemsMax)
	{
		m_dtItemsMax = dtTime;
	}
}

void CBCGPGanttChart::UpdateScrollTimeRange (COleDateTime dtTimeToShow)
{
	BOOL bUpdate = FALSE;
	if (dtTimeToShow < m_dtHScrollMin)
	{
		m_dtHScrollMin = dtTimeToShow;
		bUpdate = TRUE;
	}
	else if (dtTimeToShow > m_dtHScrollMax)
	{
		m_dtHScrollMax = dtTimeToShow;
		bUpdate = TRUE;
	}

	if (m_dtItemsMin < m_dtHScrollMin)
	{
		m_dtHScrollMin = m_dtItemsMin;
		bUpdate = TRUE;
	}
	else if (m_dtItemsMax > m_dtHScrollMax)
	{
		m_dtHScrollMax = m_dtItemsMax;
		bUpdate = TRUE;
	}

	if (bUpdate)
	{
		UpdateScrollBars (TRUE);
	}
}

void CBCGPGanttChart::RecalculateItemsTimeRange ()
{
	m_dtItemsMin = COleDateTime::GetCurrentTime ();
	m_dtItemsMax = m_dtItemsMin;

	POSITION pos = m_pItems->GetHeadPosition ();

	if (pos != NULL)
	{
		const CBCGPGanttItem* pItem = m_pItems->GetNext (pos);
		m_dtItemsMin = pItem->GetStartTime ();
		m_dtItemsMax = pItem->GetFinishTime ();
	}

	while (pos != NULL)
	{
		const CBCGPGanttItem* pItem = m_pItems->GetNext (pos);
		COleDateTime dt = pItem->GetStartTime ();
		if (dt < m_dtItemsMin)
		{
			m_dtItemsMin = dt;
		}
		dt = pItem->GetFinishTime ();
		if (dt > m_dtItemsMax)
		{
			m_dtItemsMax = dt;
		}
	}

	m_dtHScrollMin = m_dtItemsMin;
	m_dtHScrollMax = m_dtItemsMax;

	UPDATE_INFO updateLayout;
	updateLayout.Reset ();
	updateLayout.bItemsLayoutChanged = TRUE;
	SetUpdateInfo (updateLayout);
}

//////////////////////////////////////////////////////////////////////////
//                          Update functions
//////////////////////////////////////////////////////////////////////////

void CBCGPGanttChart::InvalidateItem (const CBCGPGanttItem* pItem)
{
	if (GetSafeHwnd () != NULL)
	{
		InvalidateRect (GetBarRect (pItem));
	}
}

void CBCGPGanttChart::InvalidateHeaders ()
{
	if (GetSafeHwnd () != NULL)
	{
		InvalidateRect (m_rectLargeHeader);
		InvalidateRect (m_rectSmallHeader);
	}
}

void CBCGPGanttChart::InvalidateChart ()
{
	if (GetSafeHwnd () != NULL)
	{
		InvalidateRect (m_rectChart);
	}
}

//////////////////////////////////////////////////////////////////////////
//                      Storage notifications
//////////////////////////////////////////////////////////////////////////

LRESULT CBCGPGanttChart::OnStorageChanged (WPARAM, LPARAM lParam)
{
	BCGP_GANTT_STORAGE_UPDATE_INFO* pUpdate = (BCGP_GANTT_STORAGE_UPDATE_INFO*)lParam;
	if (pUpdate == NULL)
	{
		return 0L;
	}

	bool bNeedAdjustLayout = TRUE;

	switch (pUpdate->uiAction)
	{
	case BCGP_GANTT_STORAGE_INSERT_ITEM:
		if (pUpdate->pItem != NULL)
		{
			m_nFirstVisibleRow = -1;
			m_nLastVisibleRow = -1;
			UpdateItemsTimeRange (pUpdate->pItem);
		}
		HiliteItem (NULL);
		break;

	case BCGP_GANTT_STORAGE_UPDATE_ITEM:
		if (pUpdate->pItem != NULL) // single item
		{
			if (pUpdate->dwFlags & (BCGP_GANTT_ITEM_PROP_START | BCGP_GANTT_ITEM_PROP_FINISH))
			{
				UpdateItemsTimeRange (pUpdate->pItem);
			}

			if (pUpdate->pItem == m_pLastSelectedItem && !pUpdate->pItem->IsSelected ())
			{
				m_pLastSelectedItem = NULL;
			}

			if (pUpdate->pItem == m_pHilitedItem)
			{
				HiliteItem (NULL);
			}
		}
		else // multiple items
		{
			RecalculateItemsTimeRange ();
		}
		break;

	case BCGP_GANTT_STORAGE_BEFORE_REMOVE_ITEM:
		bNeedAdjustLayout = FALSE;
		break;

	case BCGP_GANTT_STORAGE_REMOVED_ITEM:
		m_pLastSelectedItem = NULL;
		HiliteItem (NULL);
		break;
	}

	CWnd* pWnd = GetNotificationWindow ();
	if (pWnd->GetSafeHwnd () != NULL)
	{
		pWnd->SendMessage (BCGM_GANTT_STORAGE_CHANGED, (WPARAM)0, lParam);       
	}

	if (GetSafeHwnd() != NULL && bNeedAdjustLayout)
	{
		UPDATE_INFO updateLayout;
		updateLayout.Reset ();
		updateLayout.bItemsLayoutChanged = TRUE;
		SetUpdateInfo (updateLayout);
	}

	return 0L;
}

//////////////////////////////////////////////////////////////////////////
//                         Item Connections
//////////////////////////////////////////////////////////////////////////

LRESULT CBCGPGanttChart::OnStorageConnectionAdded (WPARAM, LPARAM lParam)
{
	CBCGPGanttConnection* pConnection = (CBCGPGanttConnection*)lParam;
	if (pConnection == NULL)
	{
		return 0L;
	}

	UpdateItemConnections (pConnection->m_pDestItem);
	UpdateItemConnections (pConnection->m_pSourceItem);

	CWnd* pWnd = GetNotificationWindow ();
	if (pWnd->GetSafeHwnd () != NULL)
	{
		pWnd->SendMessage (BCGM_GANTT_CONNECTION_ADDED, (WPARAM)0, lParam);       
	}

	return 0L;
}

LRESULT CBCGPGanttChart::OnStorageConnectionRemoved (WPARAM, LPARAM lParam)
{
	CBCGPGanttConnection* pConnection = (CBCGPGanttConnection*)lParam;
	if (pConnection == NULL)
	{
		return 0L;
	}

	InvalidateChart();

	CWnd* pWnd = GetNotificationWindow ();
	if (pWnd->GetSafeHwnd () != NULL)
	{
		pWnd->SendMessage (BCGM_GANTT_CONNECTION_REMOVED, (WPARAM)0, lParam);       
	}

	return 0L;
}

void CBCGPGanttChart::UpdateConnections (const CArray<CBCGPGanttConnection*, CBCGPGanttConnection*>& arrLinks)
{
	int n = (int)arrLinks.GetSize ();

	for (int i = 0; i < n; ++i)
	{
		CBCGPGanttConnection* pLink = arrLinks[i];
		if (pLink != NULL && pLink->m_pSourceItem != NULL && pLink->m_pDestItem != NULL)
		{
			if (pLink->m_pSourceItem->IsVisible () && pLink->m_pDestItem->IsVisible ())
			{
				CalculateConnectorLine (*pLink, GetBarRect (pLink->m_pSourceItem), GetBarRect (pLink->m_pDestItem));
			}
			else
			{
				pLink->m_Points.RemoveAll ();
			}
		}
	}

	UPDATE_INFO updateLayout;
	updateLayout.Reset ();
	updateLayout.bConnectionsChanged = TRUE;
	SetUpdateInfo (updateLayout);
}

void CBCGPGanttChart::UpdateItemConnections (const CBCGPGanttItem* pItem)
{
	CArray<CBCGPGanttConnection*, CBCGPGanttConnection*> arrLinks;
	m_pItems->GetItemLinks (pItem, arrLinks);
	UpdateConnections (arrLinks);

	arrLinks.RemoveAll ();
	m_pItems->GetItemReferrers (pItem, arrLinks);
	UpdateConnections (arrLinks);
}

void CBCGPGanttChart::CalculateConnectorLine (CBCGPGanttConnection& link, CRect rcSourceBar, CRect rcDestBar)
{
	link.m_Points.RemoveAll ();

	if (link.m_pDestItem == NULL || link.m_pSourceItem == NULL)
	{
		return;
	}

	// Note that all connection points are relative to source bar left-top edge.
	CPoint ptOrigin = rcSourceBar.TopLeft ();

	bool bFromSrcStart = (link.m_LinkType == BCGPGANTTLINK_START_TO_FINISH || link.m_LinkType == BCGPGANTTLINK_START_TO_START);
	bool bSrcAboveDest = (rcSourceBar.bottom < rcDestBar.top);

	// Calculate bars connection points
	CPoint ptSource;
	ptSource.y = rcSourceBar.CenterPoint ().y;
	ptSource.x = (bFromSrcStart) ? rcSourceBar.left : rcSourceBar.right - 1;

	CPoint ptDest;
	ptDest.y = (bSrcAboveDest) ? rcDestBar.top - 1 : rcDestBar.bottom;

	if (link.m_LinkType == BCGPGANTTLINK_START_TO_START || link.m_LinkType == BCGPGANTTLINK_FINISH_TO_START)
	{
		ptDest.x = rcDestBar.left + 5;
	}
	else
	{
		ptDest.x = rcDestBar.right - 6;
	}

	if (ptDest.x < rcDestBar.left + 2 || ptDest.x > rcDestBar.right - 3 || link.m_pDestItem->IsMileStone ())
	{
		ptDest.x = rcDestBar.CenterPoint ().x;
	}

	CPoint ptStart (ptSource.x - ptOrigin.x, ptSource.y - ptOrigin.y);
	link.m_Points.AddTail (ptStart); // Starting point

	const int xMinSpan = 5;
	const int yMinSpan = (GetItemHeight (link.m_pSourceItem) - 5) / 2;

	int x2 = ptDest.x;
	if (!bFromSrcStart)
	{
		if (ptDest.x < ptSource.x + xMinSpan)
		{
			if (ptDest.x < ptSource.x - xMinSpan)
			{
				x2 = ptSource.x + xMinSpan;
			}
			else
			{
				x2 = ptDest.x + xMinSpan * 2;
			}
		}
	}
	else
	{
		if (ptDest.x > ptSource.x - xMinSpan)
		{
			if (ptDest.x > ptSource.x + xMinSpan)
			{
				x2 = ptSource.x - xMinSpan;
			}
			else
			{
				x2 = ptDest.x - xMinSpan * 2;
			}
		}
	}

	if (x2 != ptDest.x) // complex connector
	{
		int y2 = (bSrcAboveDest) ? ptSource.y + yMinSpan : ptSource.y - yMinSpan;
		CPoint pt1 (x2 - ptOrigin.x, ptSource.y - ptOrigin.y);
		CPoint pt2 (x2 - ptOrigin.x, y2 - ptOrigin.y);
		CPoint pt3 (ptDest.x - ptOrigin.x, y2 - ptOrigin.y);
		link.m_Points.AddTail (pt1);
		link.m_Points.AddTail (pt2);
		link.m_Points.AddTail (pt3);
	}
	else //simple connector
	{
		CPoint pt1 (ptDest.x - ptOrigin.x, ptSource.y - ptOrigin.y);
		link.m_Points.AddTail (pt1);
	}

	CPoint ptEnd (ptDest.x - ptOrigin.x, ptDest.y - ptOrigin.y);
	link.m_Points.AddTail (ptEnd);
}


//////////////////////////////////////////////////////////////////////////
//              CBCGPGanttChart message handlers
//////////////////////////////////////////////////////////////////////////

LRESULT CBCGPGanttChart::OnUpdateToolTips (WPARAM wParam, LPARAM)
{
	UINT nTypes = (UINT) wParam;

	if (m_pToolTip->GetSafeHwnd () == NULL)
	{
		return 0;
	}

	if (nTypes & BCGP_TOOLTIP_TYPE_GANTT)
	{
		CBCGPTooltipManager::CreateToolTip (m_pToolTip, this, BCGP_TOOLTIP_TYPE_GANTT);

		if (m_pToolTip->GetSafeHwnd () != NULL)
		{
			UPDATE_INFO updateLayout;
			updateLayout.Reset ();
			updateLayout.bLayoutChanged = TRUE;
			updateLayout.bItemsLayoutChanged = TRUE;
			SetUpdateInfo (updateLayout);
		}
	}

	return 0;
}

BOOL CBCGPGanttChart::OnToolTipText(UINT /*uiToolID*/, NMHDR* pNMH, LRESULT* /*pResult*/)
{
	static CString strTipText;
	CString strDescription;

	if (m_pToolTip->GetSafeHwnd () == NULL || pNMH->hwndFrom != m_pToolTip->GetSafeHwnd ())
	{
		return FALSE;
	}

	if (CBCGPPopupMenu::GetActiveMenu () != NULL)
	{
		return FALSE;
	}

	LPNMTTDISPINFO pTTDispInfo = (LPNMTTDISPINFO) pNMH;
	ASSERT((pTTDispInfo->uFlags & TTF_IDISHWND) == 0);

	CPoint point;
	::GetCursorPos (&point);
	ScreenToClient (&point);

	BCGP_GANTT_CHART_HEADER_CELL_INFO cellInfo;
	if (HeaderCellFromPoint (point, cellInfo))
	{
		COleDateTime dateTime = cellInfo.dtCellLeftMostTime;

		switch (DetectSpanType (cellInfo.dtCellTimeDelta))
		{
		case DATETIMERANGE_YEAR:
			strTipText = FormatDate (dateTime, _T("yyyy"));
			break;
		case DATETIMERANGE_QUARTER:
		case DATETIMERANGE_MONTH:
			strTipText = FormatDate (dateTime, _T("MMMM yyyy"));
			break;
		case DATETIMERANGE_WEEK:
			strTipText = FormatDate (dateTime, _T("Long")) + _T(" - ") + FormatDate (dateTime + cellInfo.dtCellTimeDelta, _T("Long"));
			break;
		case DATETIMERANGE_DAY:
			strTipText = FormatDate (dateTime, _T("Long"));
			break;
		case DATETIMERANGE_12HOURS:
		case DATETIMERANGE_6HOURS:
		case DATETIMERANGE_4HOURS:
		case DATETIMERANGE_2HOURS:
		case DATETIMERANGE_1HOUR:
		case DATETIMERANGE_30MINUTES:
		case DATETIMERANGE_20MINUTES:
		case DATETIMERANGE_15MINUTES:
		case DATETIMERANGE_10MINUTES:
		case DATETIMERANGE_5MINUTES:
		case DATETIMERANGE_1MINUTE:
			strTipText = FormatDate (dateTime, _T("Long")) + _T(" ") + FormatTime (dateTime, TIMEFORMAT_HOURS_AND_MINUTES);
			break;
		case DATETIMERANGE_30SECONDS:
		case DATETIMERANGE_1SECOND:
			strTipText = FormatTime (dateTime, TIMEFORMAT_HOURS_AND_MINUTES);
			break;
		default:
			ASSERT (FALSE);
			break;
		}
	}
	else
	{
		CBCGPGanttItem* pItem = ItemFromPoint (point);
		if (pItem == NULL)
		{
			return TRUE;
		}

		if (!QueryToolTipText (pItem, strTipText, strDescription))
		{
			return TRUE;
		}

		if (strTipText.IsEmpty ())
		{
			strTipText = pItem->GetName ();
		}
	}

	CBCGPToolTipCtrl* pToolTip = DYNAMIC_DOWNCAST (CBCGPToolTipCtrl, m_pToolTip);

	if (pToolTip != NULL)
	{
		ASSERT_VALID (pToolTip);
		pToolTip->SetDescription (strDescription);
	}

	pTTDispInfo->lpszText = const_cast<LPTSTR> ((LPCTSTR)strTipText);
	return TRUE;
}

BOOL CBCGPGanttChart::QueryToolTipText(const CBCGPGanttItem* pItem, CString& strText, CString& strDescription) const
{
	if (pItem == NULL)
	{
		return FALSE;
	}

	CString strFormat;

	if (!pItem->GetName ().IsEmpty ())
	{
		strFormat = _T(" (%d%%)");
	}
	else
	{
		strFormat = _T("%d%%");
	}

	strText.Format (strFormat, (int)(pItem->GetProgress () * 100.0f + 0.5f));
	strText = pItem->GetName () + strText;

	COleDateTime tmStart = pItem->GetStartTime ();
	COleDateTime tmFinish = pItem->GetFinishTime ();

	CString strStart, strFinish;
	if (IsTimeNull (tmStart) && IsTimeNull (tmFinish))
	{
		strStart = FormatDate (tmStart, _T("Short"));
		strFinish = FormatDate (tmFinish, _T("Short"));
	}
	else
	{
		strStart = FormatDate (tmStart, _T("Short")) + _T(" ") + FormatTime (tmStart, TIMEFORMAT_HOURS_AND_MINUTES);
		strFinish = FormatDate (tmFinish, _T("Short")) + _T(" ") + FormatTime (tmFinish, TIMEFORMAT_HOURS_AND_MINUTES);
	}

	if (pItem->IsMileStone ())
	{
		strText = strStart;
	}
	else
	{
		strDescription.Format (
			_T("%s - %s"),
			strStart,
			strFinish);
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//						Control window creation
//////////////////////////////////////////////////////////////////////////

BOOL CBCGPGanttChart::Create (DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	return CBCGPWnd::Create (globalData.RegisterWindowClass (_T("BCGPGanttChart")), _T(""), dwStyle, rect, pParentWnd, nID);
}

BOOL CBCGPGanttChart::PreCreateWindow (CREATESTRUCT& cs)
{
	cs.style = (cs.style & ~(WS_VSCROLL | WS_HSCROLL)) | WS_CLIPCHILDREN;
	return CBCGPWnd::PreCreateWindow (cs);
}

int CBCGPGanttChart::OnCreate (LPCREATESTRUCT lpCreateStruct)
{
	int iResult = CBCGPWnd::OnCreate (lpCreateStruct);
	if (iResult != 0)
	{
		return iResult;
	}

	ASSERT (m_pItems == NULL);
	m_pItems = QueryItemStorage ();

	m_dtPixelTimeDelta = COleDateTimeSpan (0, 0, 20, 0); // Initial scale: 20 minutes per pixel
	m_dtLeftMostChartTime = COleDateTime::GetCurrentTime () - COleDateTimeSpan (1, 0, 0, 0);

	CBCGPTooltipManager::CreateToolTip (m_pToolTip, this, BCGP_TOOLTIP_TYPE_GANTT);

	UpdateHeaders ();
	UpdateScrollBars (FALSE);

	CBCGPGestureConfig gestureConfig;
	gestureConfig.EnablePan(TRUE, BCGP_GC_PAN_WITH_SINGLE_FINGER_VERTICALLY | BCGP_GC_PAN_WITH_SINGLE_FINGER_HORIZONTALLY | BCGP_GC_PAN_WITH_GUTTER | BCGP_GC_PAN_WITH_INERTIA);
	
	bcgpGestureManager.SetGestureConfig(GetSafeHwnd(), gestureConfig);
	return 0;
}

void CBCGPGanttChart::OnDestroy ()
{
	m_nControlState = ecsDestroying;

	if (m_pItems != NULL)
	{
		ReleaseItemStorage (m_pItems);
		m_pItems = NULL;
	}

	if (m_pRenderer != NULL)
	{
		ReleaseRenderer (m_pRenderer);
		m_pRenderer = NULL;
	}

	CBCGPTooltipManager::DeleteToolTip (m_pToolTip);

	CBCGPWnd::OnDestroy ();
}

//////////////////////////////////////////////////////////////////////////
//								Scrolling
//////////////////////////////////////////////////////////////////////////

CScrollBar* CBCGPGanttChart::GetScrollBarCtrl (int nBar) const
{
	if (nBar == SB_HORZ)
	{
		if (m_wndScrollHorz.GetSafeHwnd () != NULL)
		{
			return (CScrollBar*)&m_wndScrollHorz;
		}
	}

	if (nBar == SB_VERT)
	{
		if (m_wndScrollVert.GetSafeHwnd () != NULL)
		{
			return (CScrollBar*)&m_wndScrollVert;
		}
	}

	return NULL;
}

void CBCGPGanttChart::UpdateScrollBars (BOOL bValuesOnly)
{
	if (GetSafeHwnd () == NULL)
	{
		return;
	}

	CRect rcClient;
	GetClientRect (rcClient);

	BOOL bShowHorz = TRUE;
	BOOL bShowVert = TRUE;
	BOOL bHorzBarExists = m_wndScrollHorz.GetSafeHwnd () != NULL;
	BOOL bVertBarExists = m_wndScrollVert.GetSafeHwnd () != NULL;

	int cxScrollWidth  = ::GetSystemMetrics (SM_CXVSCROLL);
	int cxScrollHeight = ::GetSystemMetrics (SM_CXHSCROLL);

	if (!bValuesOnly)
	{
		if (rcClient.Width () <= cxScrollWidth)
		{
			bShowVert = FALSE;
		}
		if (rcClient.Height () <= cxScrollHeight)
		{
			bShowHorz = FALSE;
		}

		CRect rcScrollH = rcClient;
		rcScrollH.top = rcScrollH.bottom - cxScrollHeight;
		if (bShowVert)
		{
			rcScrollH.right -= cxScrollWidth;
		}

		CRect rcScrollV = rcClient;
		rcScrollV.left = rcScrollV.right - cxScrollWidth;
		if (bShowHorz)
		{
			rcScrollV.bottom -= cxScrollHeight;
		}

		if (bShowHorz && !bHorzBarExists)
		{
			m_wndScrollHorz.Create (WS_CHILD | WS_VISIBLE | SBS_HORZ, rcScrollH, this, 1);
			bHorzBarExists = m_wndScrollHorz.GetSafeHwnd () != NULL;
		}

		if (bShowVert && !bVertBarExists)
		{
			m_wndScrollVert.Create (WS_CHILD | WS_VISIBLE | WS_DISABLED | SBS_VERT, rcScrollV, this, 2);
			bVertBarExists = m_wndScrollVert.GetSafeHwnd () != NULL;
		}

		if (bHorzBarExists)
		{
			m_wndScrollHorz.SetWindowPos (NULL, rcScrollH.left, rcScrollH.top, rcScrollH.Width (), rcScrollH.Height (), SWP_NOZORDER | SWP_NOACTIVATE);
			m_wndScrollHorz.RedrawWindow ();
		}

		if (bVertBarExists)
		{
			m_wndScrollVert.SetWindowPos (NULL, rcScrollV.left, rcScrollV.top, rcScrollV.Width (), rcScrollV.Height (), SWP_NOZORDER | SWP_NOACTIVATE);
			m_wndScrollVert.RedrawWindow ();
		}
	}

	// updating scroll positions

	// vertical:
	int nTotalHeight = CalculateScrollHeight ();
	CRect rcVisible = GetVisibleChartArea ();

	if (bVertBarExists)
	{
		if (nTotalHeight < rcVisible.Height ())
		{
			m_wndScrollVert.SetScrollRange (0, INT_MAX, FALSE);
			m_wndScrollVert.EnableWindow (FALSE);
			m_nVerticalOffset = 0;
		}
		else
		{
			m_wndScrollVert.EnableWindow (TRUE);
			SCROLLINFO sci;
			sci.cbSize = sizeof (SCROLLINFO);
			m_wndScrollVert.GetScrollInfo (&sci);
			sci.nPage = rcVisible.Height ();
			sci.nPos = m_nVerticalOffset;
			sci.nMin = 0;
			sci.nMax = nTotalHeight;
			m_wndScrollVert.SetScrollInfo (&sci);
		}
	}

	// horizontal:
	if (bHorzBarExists)
	{
		COleDateTime dtMin, dtMax;
		COleDateTimeSpan dtSpan = GetScrollTimeRange (&dtMin, &dtMax);

		COleDateTimeSpan dtPage = m_dtPixelTimeDelta * rcVisible.Width ();

		const int xLeft = 0;
		const int xRight = 0x4000;

		SCROLLINFO sci;
		sci.cbSize = sizeof (SCROLLINFO);
		m_wndScrollHorz.GetScrollInfo (&sci);

		double k = (dtMax > dtMin) ? (xRight - xLeft) / (dtMax - dtMin) : 1.0;
		sci.nMin = xLeft;
		sci.nMax = xRight;
		sci.nPos = (int)(xLeft + (m_dtLeftMostChartTime - dtMin) * k);

		if (!m_bTrackingScrollThumb)
		{
			sci.nPage = (UINT)(dtPage * k);
		}

		sci.fMask = SIF_POS | SIF_RANGE | SIF_PAGE;
		m_wndScrollHorz.SetScrollInfo (&sci);
	}

	m_bTrackingScrollThumb = FALSE;
}

void CBCGPGanttChart::SetScrollBarsStyle (CBCGPScrollBar::BCGPSB_STYLE style)
{
	ASSERT (CBCGPScrollBar::BCGP_SBSTYLE_FIRST <= style && style <= CBCGPScrollBar::BCGP_SBSTYLE_LAST);

	m_wndScrollHorz.SetVisualStyle (style);
	m_wndScrollVert.SetVisualStyle (style);
	UpdateScrollBars (FALSE);
}

int  CBCGPGanttChart::GetVScrollPos () const
{
	return m_nVerticalOffset;
}

void CBCGPGanttChart::SetVScrollPos (int nPixelOffset)
{
	ASSERT (nPixelOffset >= 0);

	int nOldVerticalOffset = m_nVerticalOffset;

	m_nVerticalOffset = nPixelOffset;
	int nTotalHeight = CalculateScrollHeight ();
	if (m_wndScrollVert.GetSafeHwnd () != NULL && nTotalHeight > 0)
	{
		int minY, maxY;
		m_wndScrollVert.GetScrollRange (&minY, &maxY);
		m_wndScrollVert.SetScrollPos ((maxY - minY) * nPixelOffset / nTotalHeight);
	}

	UPDATE_INFO updateVPos;
	updateVPos.Reset ();
	updateVPos.nVScrollDelta = m_nVerticalOffset - nOldVerticalOffset;
	SetUpdateInfo (updateVPos);

	CBCGPGanttControl* pNotify = GetGanttControl ();
	if (pNotify != NULL)
	{
		pNotify->DoVerticalScroll (this, nPixelOffset);
	}
}

void CBCGPGanttChart::OnVScroll (UINT nSBCode, UINT /*nPos*/, CScrollBar* /*pScrollBar*/)
{
	if (m_wndScrollVert.GetSafeHwnd () == NULL || !m_wndScrollVert.IsWindowEnabled ())
	{
		return;
	}

	SCROLLINFO sci;
	sci.cbSize = sizeof (SCROLLINFO);
	m_wndScrollVert.GetScrollInfo (&sci);
	int minY = sci.nMin;
	int maxY = sci.nMax;
	int y = sci.nPos;
	int yOld = y;

	switch (nSBCode)
	{
	case SB_TOP:
		y = minY;
		break;
	case SB_BOTTOM:
		y = maxY;
		break;
	case SB_LINEUP:
		y -= 20;
		break;
	case SB_LINEDOWN:
		y += 20;
		break;
	case SB_PAGEUP:
		y -= 120;
		break;
	case SB_PAGEDOWN:
		y += 120;
		break;
	case SB_THUMBTRACK:
		y = sci.nTrackPos;
		break;
	}
	if (y < minY) y = minY;
	if (y > maxY) y = maxY;
	
	if (y != yOld && maxY > minY)
	{
		SetVScrollPos (MulDiv(y, CalculateScrollHeight (), maxY - minY));
	}
}

void CBCGPGanttChart::OnHScroll (UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (m_wndScrollHorz.GetSafeHwnd () == NULL)
	{
		return;
	}

	COleDateTime dtNewPos = m_dtLeftMostChartTime;

	COleDateTimeSpan dtLineStep = m_dtGridLinesSmallSpan;
	COleDateTimeSpan dtPageStep = m_dtGridLinesLargeSpan / 2;

	COleDateTime dtMin, dtMax;
	GetScrollTimeRange (&dtMin, &dtMax);

	BOOL bCheckRange = TRUE;
	m_bTrackingScrollThumb = (nSBCode == SB_THUMBTRACK);

	switch (nSBCode)
	{
	case SB_LEFT:
		dtNewPos = dtMin;
		break;

	case SB_RIGHT:
		dtNewPos = dtMax;
		break;

	case SB_LINELEFT:
		dtNewPos -= dtLineStep;
		bCheckRange = FALSE;
		break;

	case SB_LINERIGHT:
		dtNewPos += dtLineStep;
		bCheckRange = FALSE;
		break;

	case SB_PAGELEFT:
		dtNewPos -= dtPageStep;
		break;

	case SB_PAGERIGHT:
		dtNewPos += dtPageStep;
		break;

	case SB_THUMBTRACK:
		{
			m_bTrackingScrollThumb = TRUE;
			int minX, maxX;
			m_wndScrollHorz.GetScrollRange (&minX, &maxX);
			if (minX < maxX)
			{
				dtNewPos = dtMin + COleDateTimeSpan((dtMax - dtMin) * (int(nPos) - minX) / (maxX - minX));
			}
		}
		break;
	}

	if (bCheckRange)
	{
		if (dtNewPos < dtMin)
		{
			dtNewPos = dtMin;
		}

		if (dtNewPos > dtMax)
		{
			dtNewPos = dtMax;
		}
	}

	SetLeftmostDateTime (dtNewPos);

	CBCGPWnd::OnHScroll (nSBCode, nPos, pScrollBar);
}

BOOL CBCGPGanttChart::OnMouseWheel (UINT nFlags, short zDelta, CPoint pt)
{
	if (m_nControlState != ecsNormal)
	{
		return FALSE;
	}

	if (CBCGPPopupMenu::GetActiveMenu () != NULL)
	{
		return TRUE;
	}

	if ((nFlags & MK_CONTROL) == MK_CONTROL) // scaling
	{
		if (zDelta > 0)
		{
			IncreaseScale ();
		}
		else
		{
			DecreaseScale ();
		}
		return TRUE;
	}
	else // scrolling
	{
		int nSteps = abs(zDelta) / WHEEL_DELTA;

		for (int i = 0; i < nSteps; i++)
		{
			if ((nFlags & MK_SHIFT) == MK_SHIFT) // horizontal scrolling
			{
				OnHScroll (zDelta < 0 ? SB_LINELEFT : SB_LINERIGHT, 0, NULL);
			}
			else
			{
				if (m_wndScrollVert.GetSafeHwnd () == NULL || !m_wndScrollVert.IsWindowEnabled ())
				{
					OnHScroll (zDelta < 0 ? SB_LINELEFT : SB_LINERIGHT, 0, NULL);
				}
				else
				{
					OnVScroll (zDelta < 0 ? SB_LINEDOWN : SB_LINEUP, 0, NULL);
				}
			}
		}
	}

	return CBCGPWnd::OnMouseWheel (nFlags, zDelta, pt);
}


//////////////////////////////////////////////////////////////////////////
//								Drawing
//////////////////////////////////////////////////////////////////////////

BOOL CBCGPGanttChart::OnEraseBkgnd (CDC* /*pDC*/)
{
	return TRUE;
}

void CBCGPGanttChart::OnPaint ()
{
	CPaintDC dc(this);
	OnDraw(&dc);
}

void CBCGPGanttChart::OnDraw(CDC* pDC)
{
	ASSERT_VALID(pDC);

	if (m_pUpdateInfo->NeedsUpdate())
	{
		PerformAdjustLayout ();
	}

	CRect rcClient;
	GetClientRect (&rcClient);
	BOOL bHorzBarVisible = (m_wndScrollHorz.GetSafeHwnd () != NULL) && (m_wndScrollHorz.IsWindowVisible ());
	BOOL bVertBarVisible = (m_wndScrollVert.GetSafeHwnd () != NULL) && (m_wndScrollVert.IsWindowVisible ());
	int nScrollWidth = ::GetSystemMetrics (SM_CXVSCROLL);
	int nScrollHeight = ::GetSystemMetrics (SM_CXHSCROLL);
	if (bHorzBarVisible && bVertBarVisible)
	{
		CRect rcUnused = rcClient;
		rcUnused.top = rcUnused.bottom - nScrollHeight;
		rcUnused.left = rcUnused.right - nScrollWidth;
		pDC->FillSolidRect(rcUnused, GetActualColors ().clrBackground);
	}
	if (bHorzBarVisible)
	{
		rcClient.bottom -= nScrollHeight;
	}
	if (bVertBarVisible)
	{
		rcClient.right -= nScrollWidth;
	}

	if (!rcClient.IsRectEmpty())
	{
		CBCGPMemDC memDc (*pDC, rcClient);
		DoPaint (memDc.GetDC());
	}
}

//////////////////////////////////////////////////////////////////////////
//          Other message handlers
//////////////////////////////////////////////////////////////////////////

void CBCGPGanttChart::OnSize (UINT nType, int cx, int cy)
{
	UpdateScrollBars (FALSE);

	UPDATE_INFO updateLayout;
	updateLayout.Reset ();
	updateLayout.bLayoutChanged = TRUE;
	SetUpdateInfo (updateLayout);

	CBCGPWnd::OnSize (nType, cx, cy);
}

LRESULT CBCGPGanttChart::OnSetFont (WPARAM wParam, LPARAM)
{
	CFont* pFont = CFont::FromHandle ((HFONT)wParam);

	if (pFont != m_pFont)
	{
		m_pFont = pFont;

		UPDATE_INFO updateFonts;
		updateFonts.Reset ();
		updateFonts.bAppearanceChanged = TRUE;
		SetUpdateInfo (updateFonts);
	}

	return 0;
}

LRESULT CBCGPGanttChart::OnGetFont (WPARAM, LPARAM)
{
	return (LRESULT)(HFONT)m_pFont->GetSafeHandle ();
}

void CBCGPGanttChart::OnSettingChange (UINT /*uFlags*/, LPCTSTR /* lpszSection */)
{
	SetHeaderHeights (0, 0);
}

CBCGPGanttItem* CBCGPGanttChart::ItemFromPoint (CPoint pt) const
{
	for (int i = (int)m_arrItemRects.GetSize () - 1; i >= 0; --i)
	{
		if (m_arrItemRects[i].rectBar.PtInRect(pt))
		{
			return m_arrItemRects[i].pItem;
		}
	}

	return NULL;  // no item at that point
}

CBCGPGanttItemStorageBase* CBCGPGanttChart::GetStorage ()
{
	return m_pItems;
}

const CBCGPGanttItemStorageBase* CBCGPGanttChart::GetStorage () const
{
	return m_pItems;
}

CBCGPGanttItem* CBCGPGanttChart::GetItemByIndex (int index) const
{
	return m_pItems->GetItem (index);
}

int CBCGPGanttChart::IndexOfItem (const CBCGPGanttItem* pItem) const
{
	return m_pItems->IndexOf (pItem);
}

int CBCGPGanttChart::GetItemCount () const
{
	return m_pItems->GetCount ();
}

void CBCGPGanttChart::AddItem (CBCGPGanttItem* pItem)
{
	m_pItems->Add (pItem);
}
void CBCGPGanttChart::InsertItem (int pos, CBCGPGanttItem* pItem)
{
	m_pItems->Insert (pos, pItem);
}

void CBCGPGanttChart::RemoveItem (CBCGPGanttItem* pItem)
{
	m_pItems->Remove (pItem);
}

void CBCGPGanttChart::GetItemReferrers  (const CBCGPGanttItem* pItem, CConnectionArray& arrReferrers)
{
	m_pItems->GetItemReferrers (pItem, arrReferrers);
}

void CBCGPGanttChart::GetItemLinks (const CBCGPGanttItem* pItem, CConnectionArray& arrLinks)
{
	m_pItems->GetItemLinks (pItem, arrLinks);
}

CBCGPGanttConnection* CBCGPGanttChart::AddConnection (CBCGPGanttItem* pSourceItem, CBCGPGanttItem* pDestItem, int linkType)
{
	return m_pItems->AddConnection (pSourceItem, pDestItem, linkType);
}

CBCGPGanttConnection* CBCGPGanttChart::FindConnection (const CBCGPGanttItem* pSourceItem, const CBCGPGanttItem* pDestItem)
{
	return m_pItems->FindConnection (pSourceItem, pDestItem);
}

BOOL CBCGPGanttChart::RemoveConnection (CBCGPGanttItem* pSourceItem, CBCGPGanttItem* pDestItem)
{
	return m_pItems->RemoveConnection (pSourceItem, pDestItem);
}

void CBCGPGanttChart::GetSelectedItems (CArray <CBCGPGanttItem*, CBCGPGanttItem*>& arrSelected) const
{
	arrSelected.RemoveAll ();

	ASSERT_VALID (m_pItems);

	POSITION pos = m_pItems->GetHeadPosition ();
	while (pos != NULL)
	{
		CBCGPGanttItem* pItem = m_pItems->GetNext (pos);
		if (pItem != NULL && pItem->IsSelected ())
		{
			arrSelected.Add (pItem);
		}
	}
}

int CBCGPGanttChart::GetSelectedItemsCount () const
{
	int n = 0;
	POSITION pos = m_pItems->GetHeadPosition ();

	while (pos != NULL)
	{
		CBCGPGanttItem* pItem = m_pItems->GetNext (pos);
		if (pItem != NULL && pItem->IsSelected ())
		{
			n ++;
		}
	}

	return n;
}

void CBCGPGanttChart::SelectItem (CBCGPGanttItem* pItem, UINT nKeys)
{
	bool bCtrl  = ((nKeys & MK_CONTROL) == MK_CONTROL);
	bool bShift = ((nKeys & MK_SHIFT) == MK_SHIFT);

	if (m_pLastSelectedItem == NULL)
	{
		bShift = false;  // if there were no items selected, shift key doesn't matters
	}

	if (!bCtrl && !bShift)
	{
		if (pItem != NULL && pItem->IsSelected ()) // if an item is already selected...
		{
			m_nControlState = ecsSelectingItem; // then if mouse move occurs, change it to ecsMoveItems or ecsResizeItems
		}
		else
		{
			for (int i = m_pItems->GetCount () - 1; i >= 0; --i)
			{
				CBCGPGanttItem* pCurrent = m_pItems->GetItem(i);
				pCurrent->Select (pCurrent == pItem);
			}
			m_pLastSelectedItem = pItem;
			m_nControlState = ecsSelectingItem; // prepare to drag it...
		}
	}

	if (pItem != NULL)
	{
		if (bCtrl)
		{
			pItem->Select (!pItem->IsSelected ()); // invert selection
			m_pLastSelectedItem = pItem;
		}
		else if (bShift)
		{
			int j = m_pItems->IndexOf (pItem), k = m_pItems->IndexOf (m_pLastSelectedItem);
			ASSERT (j >= 0);
			ASSERT (k >= 0);

			if (k < j)
			{
				int t = k;
				k = j;
				j = t;
			}

			for (int i = m_pItems->GetCount () - 1; i >= 0; --i)
			{
				m_pItems->GetItem(i)->Select (j <= i && i <= k);
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
//							Mouse events
//////////////////////////////////////////////////////////////////////////

BOOL CBCGPGanttChart::DoClick (UINT nFlags, CPoint point)
{
	SetFocus ();
	if (m_rectLargeHeader.PtInRect (point) || m_rectSmallHeader.PtInRect (point))
	{
		return DoHeaderClick (nFlags, point);
	}

	if (m_rectChart.PtInRect (point))
	{
		CBCGPGanttItem* pItem = ItemFromPoint (point);
		if (pItem == NULL)
		{
			return DoChartClick (nFlags, point);
		}
		else
		{
			return DoItemClick (nFlags, pItem);
		}
	}

	return FALSE; // default processing
}

BOOL CBCGPGanttChart::DoItemClick (UINT nFlags, CBCGPGanttItem* pItem)
{
	CWnd* pWnd = GetNotificationWindow ();
	if (pWnd != NULL)
	{
		return (BOOL)pWnd->SendMessage (BCGM_GANTT_CHART_CLICKITEM, (WPARAM)nFlags, (LPARAM)pItem);       
	}
	return FALSE;
}

BOOL CBCGPGanttChart::DoHeaderClick (UINT nFlags, CPoint point)
{
	CWnd* pWnd = GetNotificationWindow ();
	if (pWnd != NULL)
	{
		return (BOOL)pWnd->SendMessage (BCGM_GANTT_CHART_CLICKHEADER, (WPARAM)nFlags, MAKELPARAM(point.x, point.y));
	}
	return FALSE;
}

BOOL CBCGPGanttChart::DoChartClick (UINT nFlags, CPoint point)
{
	CWnd* pWnd = GetNotificationWindow ();
	if (pWnd != NULL)
	{
		return (BOOL)pWnd->SendMessage (BCGM_GANTT_CHART_CLICKCHART, (WPARAM)nFlags, MAKELPARAM(point.x, point.y));
	}
	return FALSE;
}

BOOL CBCGPGanttChart::DoDoubleClick (UINT nFlags, CPoint point)
{
	SetFocus ();
	if (m_rectLargeHeader.PtInRect (point) || m_rectSmallHeader.PtInRect (point))
	{
		return DoHeaderDoubleClick (nFlags, point);
	}

	if (m_rectChart.PtInRect (point))
	{
		CBCGPGanttItem* pItem = ItemFromPoint (point);
		if (pItem == NULL)
		{
			return DoChartDoubleClick (nFlags, point);
		}
		else
		{
			return DoItemDoubleClick (nFlags, pItem);
		}
	}

	return FALSE; // default processing
}
BOOL CBCGPGanttChart::DoItemDoubleClick (UINT nFlags, CBCGPGanttItem* pItem)
{
	CWnd* pWnd = GetNotificationWindow ();
	if (pWnd != NULL)
	{
		return (BOOL)pWnd->SendMessage (BCGM_GANTT_CHART_DBLCLICKITEM, (WPARAM)nFlags, (LPARAM)pItem);       
	}
	return FALSE;
}

BOOL CBCGPGanttChart::DoHeaderDoubleClick (UINT nFlags, CPoint point)
{
	CWnd* pWnd = GetNotificationWindow ();
	if (pWnd != NULL)
	{
		return (BOOL)pWnd->SendMessage (BCGM_GANTT_CHART_DBLCLICKHEADER, (WPARAM)nFlags, MAKELPARAM(point.x, point.y));
	}
	return FALSE;
}

BOOL CBCGPGanttChart::DoChartDoubleClick (UINT nFlags, CPoint point)
{
	CWnd* pWnd = GetNotificationWindow ();
	if (pWnd != NULL)
	{
		return (BOOL)pWnd->SendMessage (BCGM_GANTT_CHART_DBLCLICKCHART, (WPARAM)nFlags, MAKELPARAM(point.x, point.y));
	}
	return FALSE;
}

BOOL CBCGPGanttChart::IsPointInItemResizeArea (CPoint point, CBCGPGanttItem** ppItem) const
{
	for (int i = (int)m_arrItemRects.GetSize () - 1; i >= 0; --i)
	{
		CRect rect = m_arrItemRects[i].rectBar;

		if (rect.PtInRect (point))
		{
			if (ppItem != NULL)
			{
				*ppItem = m_arrItemRects[i].pItem;
			}

			if (m_arrItemRects[i].pItem->IsMileStone ())
			{
				return FALSE;
			}

			return (rect.Width () >= 12 && point.x >= rect.right - 4);
		}
	}

	if (ppItem != NULL)
	{
		*ppItem = NULL;
	}

	return FALSE;
}

BOOL CBCGPGanttChart::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	CPoint ptCursor;
	::GetCursorPos (&ptCursor);
	ScreenToClient (&ptCursor);

	CBCGPGanttItem* pItem = NULL;
	BOOL bItemResize = !m_bReadOnly && IsPointInItemResizeArea (ptCursor, &pItem);

	if (pItem != NULL || bItemResize)
	{
		::SetCursor (AfxGetApp ()->LoadStandardCursor (bItemResize ? IDC_SIZEWE : IDC_SIZEALL));
		return TRUE;
	}

	return CBCGPWnd::OnSetCursor (pWnd, nHitTest, message);
}

void CBCGPGanttChart::CalculateMovePosition (BCGP_GANTT_ITEM_DRAGDROP& dragDropInfo, COleDateTimeSpan dtOffset, BOOL bSnapToGrid)
{
	ASSERT_VALID (dragDropInfo.pItem);

	COleDateTime dtStart  = dragDropInfo.pItem->GetStartTime () + dtOffset;
	COleDateTime dtFinish = dragDropInfo.pItem->GetFinishTime () + dtOffset;
	COleDateTimeSpan dtDuration = dtFinish - dtStart;

	if (bSnapToGrid)
	{
		COleDateTime dtS = SnapDateToGrid (dtStart);
		COleDateTime dtF = SnapDateToGrid (dtFinish);
		double dS = fabs ((double)dtS - (double)dtStart);
		double dF = fabs ((double)dtF - (double)dtFinish);
		if (dF < dS)
		{
			dtFinish = dtF;
			dtStart = dtF - dtDuration;
		}
		else
		{
			dtStart = dtS;
			dtFinish = dtS + dtDuration;
		}
	}

	dragDropInfo.dtNewStartTime = dtStart;
	dragDropInfo.dtNewFinishTime = dtFinish;   
}

void CBCGPGanttChart::MoveSelectedItems(COleDateTimeSpan dtOffset)
{
	BOOL bAltKeyPressed = ::GetAsyncKeyState (VK_MENU) & 0x8000;
	for (int i = m_pItems->GetCount () - 1; i >= 0; --i)
	{
		CBCGPGanttItem* pItem = m_pItems->GetItem(i);
		if (pItem->IsSelected ())
		{
			COleDateTime dtStart  = pItem->GetStartTime ();
			COleDateTime dtFinish = pItem->GetFinishTime ();
			BCGP_GANTT_ITEM_DRAGDROP dragdrop;
			dragdrop.pItem = pItem;
			CalculateMovePosition (dragdrop, dtOffset, !bAltKeyPressed);

			BOOL bCanMove = TRUE;
			CWnd* pWnd = GetNotificationWindow ();
			if (pWnd != NULL)
			{
				bCanMove = (0 == pWnd->SendMessage (BCGM_GANTT_CHART_ITEM_MOVING, (WPARAM)0, (LPARAM)&dragdrop));
			}

			if (bCanMove)
			{
				dragdrop.pItem->SetInterval (dragdrop.dtNewStartTime, dragdrop.dtNewFinishTime);
			}
		}
	}
	
	InvalidateChart ();   
}

void CBCGPGanttChart::CalculateItemsResize (BCGP_GANTT_ITEM_DRAGDROP& dragDropInfo, COleDateTimeSpan dtOffset, BOOL bSnapToGrid)
{
	ASSERT_VALID (dragDropInfo.pItem);

	COleDateTime dtStart  = dragDropInfo.pItem->GetStartTime ();
	COleDateTime dtFinish = dragDropInfo.pItem->GetFinishTime ();
	dragDropInfo.dtNewStartTime = dtStart;
	dragDropInfo.dtNewFinishTime = dtFinish;   

	dtFinish += dtOffset;

	if (bSnapToGrid)
	{
		dtFinish = SnapDateToGrid (dtFinish);
	}

	if (dtFinish > dtStart)
	{
		dragDropInfo.dtNewFinishTime = dtFinish;   
	}
}

void CBCGPGanttChart::ResizeSelectedItems (COleDateTimeSpan dtOffset)
{
	BOOL bAltKeyPressed = ::GetAsyncKeyState (VK_MENU) & 0x8000;
	for (int i = m_pItems->GetCount () - 1; i >= 0; --i)
	{
		CBCGPGanttItem* pItem = m_pItems->GetItem(i);
		if (pItem->IsSelected ())
		{
			COleDateTime dtStart  = pItem->GetStartTime ();
			COleDateTime dtFinish = pItem->GetFinishTime ();
			BCGP_GANTT_ITEM_DRAGDROP dragdrop;
			dragdrop.pItem = pItem;
			CalculateItemsResize (dragdrop, dtOffset, !bAltKeyPressed);
			dragdrop.pItem->SetInterval (dragdrop.dtNewStartTime, dragdrop.dtNewFinishTime);
		}
	}
	
	InvalidateChart ();   
}

void CBCGPGanttChart::OnLButtonDblClk (UINT nFlags, CPoint point)
{
	DoDoubleClick (nFlags, point);
}

void CBCGPGanttChart::OnRButtonDblClk (UINT nFlags, CPoint point)
{
	DoDoubleClick (nFlags, point);
}

void CBCGPGanttChart::OnLButtonDown (UINT nFlags, CPoint point)
{
	if (DoClick (nFlags, point))
	{
		return;
	}

	if (m_nControlState == ecsNormal)
	{
		if (m_rectLargeHeader.PtInRect (point) || m_rectSmallHeader.PtInRect (point))
		{
			m_nControlState = ecsTimeScrolling;
			m_ptDragStart = point;
			SetCapture ();
			return;
		}

		if (m_rectChart.PtInRect (point))
		{
			SelectItem (ItemFromPoint (point), nFlags);
			if (m_nControlState != ecsNormal)
			{
				m_ptDragStart = point;
			}
			return;
		}
	}

	CBCGPWnd::OnLButtonDown (nFlags, point);
}

void CBCGPGanttChart::OnMouseMove (UINT nFlags, CPoint point)
{
	BOOL bHandled = FALSE;

	BOOL bMouseButtonsPressed = (nFlags & MK_LBUTTON) != 0 || (nFlags & MK_RBUTTON) != 0 || (nFlags & MK_MBUTTON) != 0;

	switch (m_nControlState)
	{
	case ecsTimeScrolling:
		{
			int pixelDelta = m_ptDragStart.x - point.x;
			m_dtLeftMostChartTime += m_dtPixelTimeDelta * (m_ptDragStart.x - point.x);
			m_ptDragStart.x = point.x;
			UpdateScrollBars (TRUE);	// update values only

			UPDATE_INFO updateTime;
			updateTime.Reset ();
			updateTime.nHScrollDelta = pixelDelta;
			SetUpdateInfo (updateTime);

			RedrawWindow (NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
			bHandled = TRUE;
		}
		break;

	case ecsSelectingItem:
		if (!m_bReadOnly && (abs (m_ptDragStart.x - point.x) > 2 || abs (m_ptDragStart.y - point.y) > 2))
		{
			BOOL bItemResize = IsPointInItemResizeArea (m_ptDragStart, NULL);
			m_nControlState = bItemResize ? ecsResizeItems : ecsMoveItems; // initialize items moving or resizing           
			m_szDragOffset = CSize (0, 0);
			::SetCursor (AfxGetApp ()->LoadStandardCursor (bItemResize ? IDC_SIZEWE : IDC_SIZEALL));
			SetCapture ();
			bHandled = TRUE;
		}
		break;

	case ecsResizeItems:
	case ecsMoveItems:
		if (bMouseButtonsPressed)
		{
			CSize szOffset = m_szDragOffset;
			m_szDragOffset.cx = point.x - m_ptDragStart.x;
			if (szOffset != m_szDragOffset)
			{
				for (int i = m_nFirstVisibleRow; i >= 0 && i <= m_nLastVisibleRow; ++i)
				{
					if (m_arrItemRects[i].pItem->IsSelected ())
					{
						CRect rectRow (m_rectChart.left, m_arrItemRects[i].nRowTopY, m_rectChart.right, m_arrItemRects[i].nRowBottomY);
						InvalidateRect (rectRow);
					}
				}
				bHandled = TRUE;
			}
		}
		break;
	}

	BOOL bTrackMouse = FALSE;

	CBCGPGanttItem* pItem = ItemFromPoint (point);
	if (pItem != NULL)
	{
		HiliteItem (pItem);
		bTrackMouse = TRUE;
	}
	else if (m_pHilitedItem != NULL)
	{
		HiliteItem (NULL);
	}

	BCGP_GANTT_CHART_HEADER_CELL_INFO ci;
	if (HeaderCellFromPoint (point, ci))
	{
		HiliteHeaderCell (&ci);
		bTrackMouse = TRUE;
	}
	else if (m_HilitedHeaderCell.Exists ())
	{
		HiliteHeaderCell (NULL); // The mouse has been moved out from the hilited header cell.
	}

	if (bTrackMouse)
	{
		TRACKMOUSEEVENT trackmouseevent;
		trackmouseevent.cbSize = sizeof(trackmouseevent);
		trackmouseevent.dwFlags = TME_LEAVE;
		trackmouseevent.hwndTrack = GetSafeHwnd();
		trackmouseevent.dwHoverTime = HOVER_DEFAULT;

		BCGPTrackMouse (&trackmouseevent);
	}

	if (!bHandled)
	{
		CBCGPWnd::OnMouseMove (nFlags, point);
	}
}

void CBCGPGanttChart::SetReadOnly (BOOL bReadOnly)
{
	m_bReadOnly = bReadOnly;
	if (m_bReadOnly && m_nControlState != ecsNormal)
	{
		PostMessage (WM_CANCELMODE);
	}
}

void CBCGPGanttChart::HiliteHeaderCell (const BCGP_GANTT_CHART_HEADER_CELL_INFO* pHeaderCellInfo)
{
	if (pHeaderCellInfo == NULL)
	{
		if (m_bShowTooltips && m_pToolTip->GetSafeHwnd () != NULL)
		{
			m_pToolTip->Pop ();
		}
		InvalidateRect (m_HilitedHeaderCell.rectCell);
		m_HilitedHeaderCell.Reset ();
	}
	else if (*pHeaderCellInfo != m_HilitedHeaderCell)
	{
		if (m_bShowTooltips && m_pToolTip->GetSafeHwnd () != NULL)
		{
			m_pToolTip->Pop ();
		}
		InvalidateRect (m_HilitedHeaderCell.rectCell);
		InvalidateRect (pHeaderCellInfo->rectCell);
		m_HilitedHeaderCell = *pHeaderCellInfo;
	}
}

void CBCGPGanttChart::HiliteItem (const CBCGPGanttItem* pItem)
{
	if (pItem == NULL)
	{
		if (m_bShowTooltips && m_pToolTip->GetSafeHwnd () != NULL)
		{
			m_pToolTip->Pop ();
		}

		m_pHilitedItem = NULL;
	}
	else if (pItem != m_pHilitedItem)
	{
		if (m_bShowTooltips && m_pToolTip->GetSafeHwnd () != NULL)
		{
			m_pToolTip->Pop ();
		}

		m_pHilitedItem = pItem;
	}
}

LRESULT CBCGPGanttChart::OnMouseLeave (WPARAM, LPARAM)
{
	HiliteHeaderCell (NULL);
	HiliteItem (NULL);

	if (m_bShowTooltips && m_pToolTip->GetSafeHwnd () != NULL)
	{
		m_pToolTip->Pop ();
	}
	return 0;
}

void CBCGPGanttChart::OnLButtonUp (UINT nFlags, CPoint point)
{
	switch (m_nControlState)
	{
	case ecsTimeScrolling:
		ReleaseCapture ();
		break;
	case ecsSelectingItem:
		SelectItem (ItemFromPoint (point), nFlags);
		break;
	case ecsMoveItems:
		if (m_szDragOffset.cx != 0)
		{
			COleDateTimeSpan dtDelta = (m_dtPixelTimeDelta * m_szDragOffset.cx);
			MoveSelectedItems (dtDelta);
		}
		ReleaseCapture ();
		break;
	case ecsResizeItems:
		if (m_szDragOffset.cx != 0)
		{
			COleDateTimeSpan dtDelta = (m_dtPixelTimeDelta * m_szDragOffset.cx);
			ResizeSelectedItems (dtDelta);
		}
		ReleaseCapture ();
		break;
	}

	m_nControlState = ecsNormal;
	CBCGPWnd::OnLButtonUp (nFlags, point);
}

void CBCGPGanttChart::OnRButtonDown (UINT nFlags, CPoint point)
{
	if (DoClick (nFlags, point))
	{
		return;
	}

	CBCGPWnd::OnRButtonDown (nFlags, point);
}

void CBCGPGanttChart::OnRButtonUp (UINT nFlags, CPoint point)
{
	CBCGPWnd::OnRButtonUp (nFlags, point);
}

void CBCGPGanttChart::OnContextMenu(CWnd* pWnd, CPoint point)
{
	if (m_nControlState != ecsNormal)
	{
		return;
	}

	if (CBCGPPopupMenu::GetActiveMenu () != NULL)
	{
		return;
	}

	CBCGPWnd::OnContextMenu (pWnd, point);
}

void CBCGPGanttChart::OnMButtonDown(UINT nFlags, CPoint point)
{
	if (DoClick (nFlags, point))
	{
		return;
	}

	if (!DoChartClick (nFlags, point))
	{
		CBCGPWnd::OnMButtonDown (nFlags, point);
	}
}

void CBCGPGanttChart::OnMButtonUp (UINT nFlags, CPoint point)
{
	CBCGPWnd::OnMButtonUp (nFlags, point);
}

BOOL CBCGPGanttChart::DoScaleChanging (BOOL bIncrease)
{
	COleDateTimeSpan dtPixelTimeDelta = m_dtPixelTimeDelta;
	CWnd* pWnd = GetNotificationWindow ();
	if (pWnd != NULL)
	{
		return (BOOL)pWnd->SendMessage (BCGM_GANTT_CHART_SCALE_CHANGING, (WPARAM)(bIncrease ? 1 : -1), (LPARAM)0);
	}
	return FALSE;
}

void CBCGPGanttChart::DoScaleChanged ()
{
	CWnd* pWnd = GetNotificationWindow ();
	if (pWnd != NULL)
	{
		pWnd->SendMessage (BCGM_GANTT_CHART_SCALE_CHANGED, (WPARAM)0, (LPARAM)0);
	}
}

//////////////////////////////////////////////////////////////////////////
//                      Keyboard events
//////////////////////////////////////////////////////////////////////////

void CBCGPGanttChart::OnKeyDown (UINT nChar, UINT nRepCnt, UINT nFlags)
{
	CBCGPWnd::OnKeyDown (nChar, nRepCnt, nFlags);
}

void CBCGPGanttChart::OnChar (UINT nChar, UINT nRepCnt, UINT nFlags)
{
	CBCGPWnd::OnChar (nChar, nRepCnt, nFlags);
}

void CBCGPGanttChart::OnCancelMode ()
{
	switch (m_nControlState)
	{
	case ecsTimeScrolling:
		ReleaseCapture ();
		break;
	case ecsMoveItems:
	case ecsResizeItems:
		ReleaseCapture ();
		m_szDragOffset = CSize (0, 0);
		InvalidateChart ();
	}

	m_nControlState = ecsNormal;
}

BOOL CBCGPGanttChart::PreTranslateMessage (MSG* pMsg) 
{
	switch (pMsg->message)
	{
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
	case WM_MOUSEMOVE:
		if (m_bShowTooltips && m_pToolTip->GetSafeHwnd () != NULL)
		{
			m_pToolTip->RelayEvent(pMsg);
		}
		break;
	}

	if (pMsg->message == WM_SYSKEYDOWN || pMsg->message == WM_SYSKEYUP)
	{
		// Suppress system keys (such as ALT key) while moving or resizing items
		if (m_nControlState == ecsMoveItems || m_nControlState == ecsResizeItems)
		{
			return TRUE;
		}
	}

	if (pMsg->message == WM_KEYDOWN)
	{
		int nChar = (int) pMsg->wParam;

		switch (nChar)
		{
		case VK_ESCAPE:
			if (m_nControlState != ecsNormal)
			{
				SendMessage (WM_CANCELMODE);
			}
			break;
		default:
			break;
		}
	}

	return CBCGPWnd::PreTranslateMessage (pMsg);
}

BOOL CBCGPGanttChart::IsGanttChartNotificationMessage (UINT message, WPARAM, LPARAM) const
{
	return (
		message == BCGM_GANTT_CHART_CLICKITEM      ||
		message == BCGM_GANTT_CHART_CLICKHEADER    ||
		message == BCGM_GANTT_CHART_CLICKCHART     ||
		message == BCGM_GANTT_CHART_DBLCLICKITEM   ||
		message == BCGM_GANTT_CHART_DBLCLICKHEADER ||
		message == BCGM_GANTT_CHART_DBLCLICKCHART  ||
		message == BCGM_GANTT_CHART_ITEM_MOVING    ||
		message == BCGM_GANTT_CHART_SCALE_CHANGING ||
		message == BCGM_GANTT_CHART_SCALE_CHANGED  ||
		message == BCGM_GANTT_STORAGE_CHANGED      ||
		message == BCGM_GANTT_CONNECTION_ADDED     ||
		message == BCGM_GANTT_CONNECTION_REMOVED
		);
}

LRESULT CBCGPGanttChart::OnPrintClient(WPARAM wp, LPARAM lp)
{
	DWORD dwFlags = (DWORD)lp;

	if (dwFlags & PRF_CLIENT)
	{
		CDC* pDC = CDC::FromHandle((HDC) wp);
		ASSERT_VALID(pDC);

		OnDraw(pDC);
	}

	return 0;
}
#endif // !defined (BCGP_EXCLUDE_GRID_CTRL) && !defined (BCGP_EXCLUDE_GANTT)

