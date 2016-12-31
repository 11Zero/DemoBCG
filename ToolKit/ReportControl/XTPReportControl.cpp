// XTPReportControl.cpp : implementation of the CXTPReportControl class.
//
// This file is a part of the XTREME REPORTCONTROL MFC class library.
// (c)1998-2011 Codejock Software, All Rights Reserved.
//
// THIS SOURCE FILE IS THE PROPERTY OF CODEJOCK SOFTWARE AND IS NOT TO BE
// RE-DISTRIBUTED BY ANY MEANS WHATSOEVER WITHOUT THE EXPRESSED WRITTEN
// CONSENT OF CODEJOCK SOFTWARE.
//
// THIS SOURCE CODE CAN ONLY BE USED UNDER THE TERMS AND CONDITIONS OUTLINED
// IN THE XTREME TOOLKIT PRO LICENSE AGREEMENT. CODEJOCK SOFTWARE GRANTS TO
// YOU (ONE SOFTWARE DEVELOPER) THE LIMITED RIGHT TO USE THIS SOFTWARE ON A
// SINGLE COMPUTER.
//
// CONTACT INFORMATION:
// support@codejock.com
// http://www.codejock.com
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Resource.h"

#include "Common/XTPDrawHelpers.h"
#include "Common/XTPSystemHelpers.h"
#include "Common/XTPImageManager.h"
#include "Common/XTPVC80Helpers.h"

#include "Common/XTPPropExchange.h"
#include "Common/XTPToolTipContext.h"
#include "Common/XTPResourceManager.h"
#include "Common/XTPMarkupRender.h"
#include "Common/Resource.h"

#include "XTPReportRecordItem.h"
#include "XTPReportRecordItemText.h"
#include "XTPReportRecord.h"
#include "XTPReportRecords.h"
#include "XTPReportHeader.h"
#include "XTPReportColumn.h"
#include "XTPReportColumns.h"
#include "XTPReportRow.h"
#include "XTPReportRows.h"
#include "XTPReportControl.h"
#include "XTPReportPaintManager.h"
#include "XTPReportNavigator.h"
#include "XTPReportFilterEditControl.h"
#include "XTPReportSubListControl.h"
#include "XTPReportGroupRow.h"
#include "XTPReportInplaceControls.h"
#include "XTPReportRecordItemControls.h"
#include "XTPReportDataManager.h"

#include <locale.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define XTP_REPORT_HSCROLL_STEP 7

#define XTP_REPORT_AUTO_SCROLL_TIMER_ID 7
#define XTP_REPORT_AUTO_SCROLL_TIMER_RESOLUTION_MS  200

#define XTP_REPORT_DELAY_CLICK_TIMER_ID 10
#define XTP_REPORT_DELAYEDIT_CLICK_TIMER_ID 20
#define XTP_REPORT_CB_RECORDS_DATA_VER 1

#define ASSERT_DBG_REMOVE_RECORD_EX
//#define XTP_DBG_REMOVE_RECORD_EX_ASSERT ASSERT

#ifndef _UNICODE
	#define XTP_CF_TEXT_T CF_TEXT
#else
	#define XTP_CF_TEXT_T CF_UNICODETEXT
#endif

//////////////////////////////////////////////////////////////////////////
XTP_IMPLEMENT_HEAP_ALLOCATOR(CXTPReportDataAllocator, FALSE)
XTP_IMPLEMENT_HEAP_ALLOCATOR(CXTPReportRowAllocator, FALSE)

// to allocate in app default heap
XTP_IMPLEMENT_HEAP_ALLOCATOR(CXTPReportAllocatorDefault, FALSE)

class CXTPReportRow_Batch : public CXTPBatchAllocObjT<CXTPReportRow, CXTPReportRow_BatchData> {};
class CXTPReportGroupRow_Batch : public CXTPBatchAllocObjT<CXTPReportGroupRow, CXTPReportGroupRow_BatchData> {};


XTP_IMPLEMENT_BATCH_ALLOC_OBJ_DATA(CXTPReportRow_BatchData, CXTPReportRow_Batch, FALSE)
XTP_IMPLEMENT_BATCH_ALLOC_OBJ_DATA(CXTPReportGroupRow_BatchData, CXTPReportGroupRow_Batch, FALSE)

//===========================================================================
HGLOBAL XTPAllocStrInGlobalMem(const CString& strText)
{
	int nLen = (strText.GetLength() + 1) * sizeof(TCHAR);

	UINT nAllocFlags = GMEM_MOVEABLE | GMEM_DDESHARE | GMEM_ZEROINIT;
	HGLOBAL hText = ::GlobalAlloc(nAllocFlags, nLen);

	if (hText != NULL)
	{
		LPTSTR lptstrCopy = (TCHAR*)GlobalLock(hText);
		STRCPY_S(lptstrCopy, strText.GetLength() + 1, (LPCTSTR)strText);
		GlobalUnlock(hText);
	}

	return hText;
}

//===========================================================================
BOOL CXTPReportControl::UseReportCustomHeap()
{
	ASSERT(CXTPReportDataAllocator::ms_dwRefs == 0 || CXTPReportDataAllocator::ms_bUseCustomHeap);
	ASSERT(CXTPReportRowAllocator::ms_dwRefs == 0 || CXTPReportRowAllocator::ms_bUseCustomHeap);

	if (CXTPReportDataAllocator::ms_dwRefs == 0)
		CXTPReportDataAllocator::ms_bUseCustomHeap = TRUE;

	if (CXTPReportRowAllocator::ms_dwRefs == 0)
		CXTPReportRowAllocator::ms_bUseCustomHeap = TRUE;

	return CXTPReportDataAllocator::ms_bUseCustomHeap &&
			CXTPReportRowAllocator::ms_bUseCustomHeap;
}

BOOL CXTPReportControl::UseRowBatchAllocation()
{
	ASSERT(CXTPReportRow_BatchData::IsDataEmpty() || CXTPReportRow_BatchData::m_bBatchAllocationEnabled);
	ASSERT(CXTPReportGroupRow_BatchData::IsDataEmpty() || CXTPReportGroupRow_BatchData::m_bBatchAllocationEnabled);

	if (CXTPReportRow_BatchData::IsDataEmpty())
		CXTPReportRow_BatchData::m_bBatchAllocationEnabled = TRUE;

	if (CXTPReportGroupRow_BatchData::IsDataEmpty())
		CXTPReportGroupRow_BatchData::m_bBatchAllocationEnabled = TRUE;

	return CXTPReportRow_BatchData::m_bBatchAllocationEnabled &&
			CXTPReportGroupRow_BatchData::m_bBatchAllocationEnabled;
}

void CXTPReportControl::FreeRowBatchExtraData()
{
	CXTPReportRow_Batch::FreeExtraData();
	CXTPReportGroupRow_Batch::FreeExtraData();
}

//////////////////////////////////////////////////////////////////////////
BOOL CXTPReportControlLocale::s_bUseResourceFileLocale = FALSE;

CArray<CXTPReportControlLocale::XTP_TIMESPEC, CXTPReportControlLocale::XTP_TIMESPEC&>
	CXTPReportControlLocale::s_arMappedSpecs;

//===========================================================================
BOOL CXTPReportControlLocale::IsUseResourceFileLocale()
{
	return s_bUseResourceFileLocale;
}

void CXTPReportControlLocale::SetUseResourceFileLocale(BOOL bUseResourceFileLocale)
{
	s_bUseResourceFileLocale = bUseResourceFileLocale;
}

LCID CXTPReportControlLocale::GetActiveLCID()
{
	LCID lcidCurr = LOCALE_USER_DEFAULT;

	if (s_bUseResourceFileLocale)
		lcidCurr = MAKELCID(XTPResourceManager()->GetResourcesLangID(), SORT_DEFAULT);

	return lcidCurr;
}

BOOL AFX_CDECL CXTPReportControlLocale::VariantChangeTypeEx(VARIANT& rVarValue, VARTYPE vartype, BOOL bThrowError)
{
	if (vartype != rVarValue.vt)
	{
		//if (rVarValue.vt == VT_BSTR)
		//{
		//  vartype = VT_BSTR;
		//  return TRUE;
		//}

		LCID lcID = GetActiveLCID();
		HRESULT hr = ::VariantChangeTypeEx(&rVarValue, &rVarValue, lcID, 0, vartype);

		if (bThrowError && FAILED(hr))
		{
			if (hr == E_OUTOFMEMORY)
				AfxThrowMemoryException();
			else
				AfxThrowOleException(hr);
		}

		return SUCCEEDED(hr);
	}
	return TRUE;
}

CString AFX_CDECL CXTPReportControlLocale::FormatDateTime(const COleDateTime& dt, LPCTSTR lpcszFormatString)
{
	return _FormatDateTime(dt, lpcszFormatString, GetActiveLCID());
}

CString CXTPReportControlLocale::_FormatDateTime(const COleDateTime& dt, LPCTSTR lpcszFormatString, LCID lcLocaleID)
{
	if (dt.GetStatus() != COleDateTime::valid)
	{
		ASSERT(dt.GetStatus() == COleDateTime::null);
		return _T("");
	}

	CString strDT = lpcszFormatString;

	SYSTEMTIME sysTime;
	ZeroMemory(&sysTime, sizeof(sysTime));

	if (!dt.GetAsSystemTime(sysTime))
	{
		ASSERT(FALSE);
		return _T("");
	}

	//  %% Percent sign
	strDT.Replace(_T("%%"), _T("\0x1"));

	_ProcessMappedSpecs(strDT, &sysTime, lcLocaleID);
	_ProcessDateTimeSpecs(strDT, &sysTime, lcLocaleID);

	// All locale dependent specifiers already processed
	_ProcessOtherSpecs(strDT, dt);

	strDT.Replace(_T("\0x1"), _T("%"));

	return strDT;
}

void CXTPReportControlLocale::_ProcessMappedSpecs(CString& rstrFormat, const SYSTEMTIME* pST, LCID lcLocaleID)
{
	_InitMappedSpecs();

	const int cnBufferSize = 96;
	TCHAR szBuffer[cnBufferSize];

	int nCount = (int)s_arMappedSpecs.GetSize();
	for (int i = 0; i < nCount; i++)
	{
		const XTP_TIMESPEC& specI = s_arMappedSpecs.ElementAt(i);
		if (rstrFormat.Find(specI.pcszSpec, 0) < 0)
			continue;

		::ZeroMemory(szBuffer, sizeof(szBuffer));

		int nResult;
		if (specI.bTime)
		{
			nResult = ::GetTimeFormat(lcLocaleID, 0, pST, specI.pcszFormat, szBuffer, cnBufferSize);
		}
		else
		{
			nResult = ::GetDateFormat(lcLocaleID, 0, pST, specI.pcszFormat, szBuffer, cnBufferSize);
		}
		ASSERT(nResult);
		if (nResult)
		{
			rstrFormat.Replace(specI.pcszSpec, szBuffer);
		}
	}
}

void CXTPReportControlLocale::_ProcessDateTimeSpecs(CString& rstrFormat, const SYSTEMTIME* pST, LCID lcLocaleID)
{
//  %c  Date and time representation appropriate for locale
//  %#c Long date and time representation

//  %x Date representation for current locale
//  %#x Long date representation for current locale

//  %X Time representation for current locale

	rstrFormat.Replace(_T("%c"), _T("%x %X"));
	rstrFormat.Replace(_T("%#c"), _T("%#x %X"));

	__ProcessDate_x(rstrFormat, pST, lcLocaleID);
	__ProcessTime_X(rstrFormat, pST, lcLocaleID);
}

void CXTPReportControlLocale::__ProcessDate_x(CString& rstrFormat, const SYSTEMTIME* pST, LCID lcLocaleID)
{
	const int cnBufferSize = 96;
	TCHAR szBuffer[cnBufferSize];

	//  %x Date representation for current locale
	//  %#x Long date representation for current locale

	TCHAR* arSpec2[2] = {_T("%x"), _T("%#x")};

	for (int i = 0; i < 2; i++)
	{
		if (rstrFormat.Find(arSpec2[i], 0) >= 0)
		{
			::ZeroMemory(szBuffer, sizeof(szBuffer));

			DWORD dwFlags = (i == 0) ? DATE_SHORTDATE : DATE_LONGDATE;

			int nRes = ::GetDateFormat(lcLocaleID, dwFlags, pST, NULL, szBuffer, cnBufferSize);

			ASSERT(nRes);
			rstrFormat.Replace(arSpec2[i], szBuffer);
		}
	}
}

void CXTPReportControlLocale::__ProcessTime_X(CString& rstrFormat, const SYSTEMTIME* pST, LCID lcLocaleID)
{
	if (rstrFormat.Find(_T("%X"), 0) >= 0)
	{
		const int cnBufferSize = 96;
		TCHAR szBuffer[cnBufferSize];

		::ZeroMemory(szBuffer, sizeof(szBuffer));
		int nRes = ::GetTimeFormat(lcLocaleID, 0, pST, NULL, szBuffer, cnBufferSize);

		ASSERT(nRes);
		rstrFormat.Replace(_T("%X"), szBuffer);
	}
}

void CXTPReportControlLocale::_ProcessOtherSpecs(CString& rstrFormat, const COleDateTime& dt)
{
	//  %j Day of year as decimal number (001 - 366)
	//  %w Weekday as decimal number (0 - 6; Sunday is 0)
	//  %U Week of year as decimal number, with Sunday as first day of week (00 - 53)
	//  %W Week of year as decimal number, with Monday as first day of week (00 - 53)
	//  %z, %Z  Either the time-zone name or time zone abbreviation, depending on registry settings; no characters if time zone is unknown

	static LPCTSTR arszSpecs[] = {  _T("%j"), _T("%#j"),
									_T("%w"), _T("%#w"),
									_T("%U"), _T("%#U"),
									_T("%W"), _T("%#W"),
									_T("%z"), _T("%Z") };

	int nCount = _countof(arszSpecs);
	for (int i = 0; i < nCount; i++)
	{
		if (rstrFormat.Find(arszSpecs[i], 0) < 0)
			continue;

		CString str = dt.Format(arszSpecs[i]);

		rstrFormat.Replace(arszSpecs[i], str);
	}
}

void CXTPReportControlLocale::_InitMappedSpecs()
{
	if (s_arMappedSpecs.GetSize())
		return;

	// date
	_AddsMappedSpec(_T("%a"), _T("ddd"),    FALSE);
	_AddsMappedSpec(_T("%A"), _T("dddd"),   FALSE);

	_AddsMappedSpec(_T("%b"), _T("MMM"),    FALSE);
	_AddsMappedSpec(_T("%B"), _T("MMMM"),   FALSE);

	_AddsMappedSpec(_T("%d"),  _T("dd"),    FALSE);
	_AddsMappedSpec(_T("%#d"), _T("d"),     FALSE);

	_AddsMappedSpec(_T("%m"),  _T("MM"),    FALSE);
	_AddsMappedSpec(_T("%#m"), _T("M"),     FALSE);

	_AddsMappedSpec(_T("%y"),  _T("yy"),    FALSE);
	_AddsMappedSpec(_T("%#y"), _T("y"),     FALSE);
	_AddsMappedSpec(_T("%Y"), _T("yyyy"),   FALSE);

	// time
	_AddsMappedSpec(_T("%H"),  _T("HH"),    TRUE);
	_AddsMappedSpec(_T("%#H"), _T("H"),     TRUE);

	_AddsMappedSpec(_T("%I"),  _T("hh"),    TRUE);
	_AddsMappedSpec(_T("%#I"), _T("h"),     TRUE);

	_AddsMappedSpec(_T("%M"),  _T("mm"),    TRUE);
	_AddsMappedSpec(_T("%#M"), _T("m"),     TRUE);

	_AddsMappedSpec(_T("%S"),  _T("ss"),    TRUE);
	_AddsMappedSpec(_T("%#S"), _T("s"),     TRUE);

	_AddsMappedSpec(_T("%p"), _T("tt"),     TRUE);
}

void CXTPReportControlLocale::_AddsMappedSpec(LPCTSTR pcszSpec, LPCTSTR pcszFormat, BOOL bTime)
{
	XTP_TIMESPEC tmpSpec = {pcszSpec, pcszFormat, bTime};
	s_arMappedSpecs.Add(tmpSpec);
}

//////////////////////////////////////////////////////////////////////////
// CReportDropTarget
class CXTPReportControl::CReportDropTarget : public COleDropTarget
{
public:
	virtual DROPEFFECT OnDragEnter(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point)
	{
		CXTPReportControl* pReport = DYNAMIC_DOWNCAST(CXTPReportControl, pWnd);
		if (!pReport)
			return DROPEFFECT_NONE;

		return pReport->OnDragOver(pDataObject, dwKeyState, point, 0);
	}

	virtual DROPEFFECT OnDragOver(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point)
	{
		CXTPReportControl* pReport = DYNAMIC_DOWNCAST(CXTPReportControl, pWnd);
		if (!pReport)
			return DROPEFFECT_NONE;

		return pReport->OnDragOver(pDataObject, dwKeyState, point, 2);
	}

	virtual void OnDragLeave(CWnd* pWnd)
	{
		CXTPReportControl* pReport = DYNAMIC_DOWNCAST(CXTPReportControl, pWnd);

		if (pReport)
			pReport->OnDragOver(NULL, 0, CPoint(-1, -1), 1);
	}

	virtual BOOL OnDrop(CWnd* pWnd, COleDataObject* pDataObject,
		DROPEFFECT dropEffect, CPoint point)
	{
		CXTPReportControl* pReport = DYNAMIC_DOWNCAST(CXTPReportControl, pWnd);

		if (pReport)
			return pReport->OnDrop(pDataObject, dropEffect, point);

		return FALSE;
	}
};

/////////////////////////////////////////////////////////////////////////////
void XTPStrSplit(LPCTSTR pcszString, LPCTSTR pcszSeparator, CStringArray& rarStrings)
{
	rarStrings.RemoveAll();

	int nSeparatorLen = (int)_tcslen(pcszSeparator);
	CString strString(pcszString);
	CString strItem;
	int nIndex_start = 0;

	// parse data
	BOOL bBreak = FALSE;
	do
	{
		int nIndex = strString.Find(pcszSeparator, nIndex_start);
		if (nIndex >= 0)
		{
			strItem = strString.Mid(nIndex_start, nIndex - nIndex_start);
		}
		else
		{
			strItem = strString.Mid(nIndex_start);
			bBreak = TRUE;
		}

		//---------------------------------------
		rarStrings.Add(strItem);

		nIndex_start = nIndex + nSeparatorLen;
	}
	while (!bBreak);
}

CString XTPStrMake(const CStringArray& arStrings, LPCTSTR pcszSeparator)
{
	CString strString;

	int nCount = (int)arStrings.GetSize();
	for (int i = 0; i < nCount; i++)
	{
		if (i > 0)
			strString += pcszSeparator;

		strString += arStrings[i];
	}

	return strString;
}

// CXTPReportControl
IMPLEMENT_DYNCREATE(CXTPReportControl, CWnd)

BEGIN_MESSAGE_MAP(CXTPReportControl, CWnd)
//{{AFX_MSG_MAP(CXTPReportControl)
	ON_WM_PAINT()
	ON_MESSAGE(WM_PRINTCLIENT, OnPrintClient)
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_LBUTTONDOWN()
	ON_WM_NCHITTEST_EX()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_CONTEXTMENU()
	ON_WM_MOUSEMOVE()
	ON_WM_SETCURSOR()
	ON_WM_KEYDOWN()
	ON_WM_VSCROLL()
	ON_WM_HSCROLL()
	ON_WM_MOUSEWHEEL()
	ON_WM_SYSKEYDOWN()
	ON_WM_CAPTURECHANGED()
	ON_WM_SYSCOLORCHANGE()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_KEYUP()
	ON_WM_SYSKEYUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_GETDLGCODE()
	ON_WM_CHAR()
	ON_MESSAGE_VOID(WM_MOUSELEAVE, OnMouseLeave)
	ON_WM_STYLECHANGED()
	ON_WM_ENABLE()
	ON_WM_TIMER()
	ON_MESSAGE(WM_GETOBJECT, OnGetObject)
	ON_MESSAGE_VOID(XTP_WM_MARKUPREDRAW, RedrawControl)
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CXTPReportControl::CXTPReportControl()
{
	RegisterWindowClass();

	m_nLockUpdateCount = 0;
	m_nRowsPerWheel = GetMouseScrollLines();

	m_pRows = new CXTPHeapObjectT<CXTPReportRows, CXTPReportAllocatorDefault>;
	m_pPlainTree = new CXTPHeapObjectT<CXTPReportRows, CXTPReportAllocatorDefault>;
	m_pRecords = new CXTPHeapObjectT<CXTPReportRecords, CXTPReportAllocatorDefault>;

	m_pColumns = new CXTPReportColumns(this);
	m_pReportHeader = new CXTPReportHeader(this, m_pColumns);
//ICON_VIEW_MODE RELATED <<
	m_bIconView = FALSE;
	m_iIconWidth = 0;
	m_iIconHeight = 0;
	m_iIconWidthSpacing = 0;
	m_iIconHeightSpacing = 0;
	m_iIconViewColumn = -1;
	m_iIconViewRowsPerLine = 0;
	m_bUseIconColumnForNum = FALSE;

	m_iColumnForNumPrev = -1;
	m_iColumnForNum = -1;
	m_bPrevFocusSubItems = FALSE;
	m_bPrevHeaderAutoSize = FALSE;
	m_iIconPropNum = 0;
	m_iIconNum = 0;
	m_pPrevVisible = NULL;
	m_pPrevGroupsOrder = NULL;
	m_nPrevTreeIndent = 0;

	m_bIconColumnIndexNotValid = FALSE;

	m_bPrevHeaderRows = FALSE;
	m_bPrevFooterRows = FALSE;
	m_bPrevHeaderShow = FALSE;
	m_bPrevFooterShow = FALSE;

	m_bRClickDrag = FALSE;

	m_PrevVertStyle = xtpReportGridSolid;
	m_PrevHorStyle = xtpReportGridSolid;

//ICON_VIEW_MODE RELATED >>
	m_pPaintManager = new CXTPReportPaintManager();
	m_pPaintManager->SetReportControl(this);

	m_pNavigator = new CXTPReportNavigator(this);
	m_nTopRow = 0;

	m_nFocusedRow = -1;
	m_nFocusedHeaderRow = -1;
	m_nFocusedFooterRow = -1;
	//m_nSelectionLastBlockStartRow = -1;
	m_nTopRowIdx = -1;
	//m_ptMouseDown = CPoint(-1, -1);
	m_bKeepSelectionAfterSort = TRUE;
	m_bUnrestrictedDragDrop = FALSE;

	m_bFreeHeightMode = FALSE;
	m_nDefaultRowFreeHeight = 20;

	m_uiDelayEditTimer = NULL;
	m_uiDelayEditMaxTime = GetDoubleClickTime() + 50;

	m_iLastRqstEditRow = -1;
	m_iLastRqstEditCol = -1;
	m_uRqstEditTimer = 0;
	m_mouseMode = xtpReportMouseNothing;
	m_bWasShiftKey = FALSE;
	m_bSortedDragDrop = FALSE;
	m_bNoNeedSortedDragDrop = TRUE;
	m_bInternalMove = FALSE;
	m_bTrapTabKey = FALSE;

	m_pSelectedRows = new CXTPReportSelectedRows(this);

	m_bStrictBestFit = FALSE;
	m_bChanged = TRUE;
	m_bRefreshIndexes = FALSE;

	m_bGroupByEnabled = FALSE;
	m_bHeaderVisible = TRUE;
	m_bFooterVisible = FALSE;
	m_bHeaderRecordsVisible = FALSE;
	m_bFooterRecordsVisible = FALSE;

	m_bBlockSelection = TRUE;
	m_bControlKeyAlwaysOn = FALSE;

	m_bShowTooltips = TRUE;

	m_bSkipGroupsFocus = TRUE;
	m_bSelectionExcludeGroupRows = TRUE;

	m_pImageManager = new CXTPImageManager();

	m_pFocusedColumn = NULL;
	m_pActiveItem = NULL;

	m_bFocusSubItems = FALSE;
	m_bEditOnClick = TRUE;
	m_bEditOnDelayClick = FALSE;
	m_bAllowEdit = FALSE;
	m_bHeaderAllowEdit = FALSE;
	m_bFooterAllowEdit = FALSE;
	m_bPreviewAllowEdit = FALSE;
	m_bSelectionEnable = TRUE;
	m_bInitialSelectionEnable = TRUE;
	m_bRowFocusVisible = TRUE;

	m_bAutoCheckItems = TRUE;

	m_bShowIconWhenEditing = FALSE;

//TEST CASE ONLY <<
	m_iCheckWithRightButton = FALSE;
	m_iCheckRightButtonExtended = FALSE;
//TEST CASE ONLY >>

	m_pInplaceEdit = new CXTPReportInplaceEdit();
	m_pInplaceButtons = new CXTPReportInplaceButtons();
	m_pInplaceList = new CXTPReportInplaceList();

	m_rcGroupByArea.SetRectEmpty();
	m_rcHeaderArea.SetRectEmpty();
	m_rcReportArea.SetRectEmpty();
	m_rcFooterArea.SetRectEmpty();

	m_bFreezeColumnsAbs = FALSE;
	m_bMovePivot = FALSE;
	m_bForcePagination = FALSE;
	m_nFreezeColumnsCount = 0;

	m_nDisableReorderColumnsCount = 0;
	m_nLeftOffset = 0;
	m_pHotRow = NULL;
	m_pHotExpandButtonRow = NULL;
	m_nOLEDropMode = 0;
	m_nOLEDropAbove = FALSE;
	m_bFullColumnScrolling = FALSE;
	m_nHScrollStep = XTP_REPORT_HSCROLL_STEP;

	m_bVScrollBarVisible = FALSE;
	m_bHScrollBarVisible = FALSE;

	m_bPrepareDrag = FALSE;
	m_pointDrag = CPoint(0, 0);

	m_pToolTipContext = new CXTPToolTipContext;
	m_pCachedToolTipInfo = new XTP_NM_REPORTTOOLTIPINFO;

	m_nPopulatedRecordsCount = 0;

	m_nClickRow = -1;
	m_cfReport = NULL;
	m_pDropTarget = new CReportDropTarget;
	m_bDragMode = FALSE;
	m_bInternalDrag = FALSE;
	m_dwDragDropFlags = 0;
	m_dwDropMarkerFlags = xtpReportDropBetween;   // Default to how it has always been.
	m_pSelectedRowsBeforeDrag = NULL;
	m_nDropPos = -1;
	m_pDropRecords = NULL;

	m_bFastDeselectMode = FALSE;
	m_bOnSizeRunning = FALSE;
	m_bAdjustLayoutRunning = FALSE;

	m_uAutoScrollTimerID = 0;

	m_bSortRecordChilds = FALSE;

	m_pRowsCompareFunc = NULL;

	m_ptrVirtualEditingRow = NULL;

	m_bFilterHiddenColumns = FALSE;
	m_nRecordsTreeFilterMode = xtpReportFilterTreeSimple;

	m_pHeaderRecords = new CXTPHeapObjectT<CXTPReportRecords, CXTPReportAllocatorDefault>;
	m_pFooterRecords = new CXTPHeapObjectT<CXTPReportRecords, CXTPReportAllocatorDefault>;
	m_pHeaderRows = new CXTPHeapObjectT<CXTPReportRows, CXTPReportAllocatorDefault>;
	m_pFooterRows = new CXTPHeapObjectT<CXTPReportRows, CXTPReportAllocatorDefault>;

	m_hbmpWatermark = NULL;
	m_WatermarkTransparency = 100;
	m_WatermarkAlignment = xtpReportWatermarkStretch;

	m_bHeaderRowsAllowAccess = TRUE;
	m_bFooterRowsAllowAccess = TRUE;

	m_bHeaderRowsSelectionEnable = TRUE;
	m_bFooterRowsSelectionEnable = TRUE;

	m_nEnsureVisibleRowIdx = m_nEnsureVisibleColumnIdx = -1;

	m_pDataManager = NULL;

	m_nAutoVScrollTimerResolution = XTP_REPORT_AUTO_SCROLL_TIMER_RESOLUTION_MS;

	m_bAdjustScrollBars = FALSE;

	m_bMarkupEnabled = FALSE;

	m_bDesktopTrackerMode = FALSE;

	m_bStrictFiltering = FALSE;

	m_wndTip.m_pReportControl = this;

	EnableAutomation();
}

BOOL CXTPReportControl::RegisterWindowClass(HINSTANCE hInstance /*= NULL*/)
{
	return XTPDrawHelpers()->RegisterWndClass(hInstance, XTPREPORTCTRL_CLASSNAME, CS_DBLCLKS);
}

CXTPReportControl::~CXTPReportControl()
{
	if (::IsWindow(m_wndTip.GetSafeHwnd()))
		m_wndTip.DestroyWindow();

	EditItem(NULL);

	//m_arrScreenRows.Clear();
	ResetContent(FALSE);

	m_UaSelected.RemoveAll();

	CMDTARGET_RELEASE(m_pRows);
	CMDTARGET_RELEASE(m_pPlainTree);
	CMDTARGET_RELEASE(m_pSelectedRows);
	CMDTARGET_RELEASE(m_pRecords);
	CMDTARGET_RELEASE(m_pColumns);
	CMDTARGET_RELEASE(m_pPaintManager);
	CMDTARGET_RELEASE(m_pNavigator);

	CMDTARGET_RELEASE(m_pImageManager);
	CMDTARGET_RELEASE(m_pReportHeader);

	CMDTARGET_RELEASE(m_pToolTipContext);
	SAFE_DELETE(m_pCachedToolTipInfo);

	SAFE_DELETE(m_pInplaceEdit);
	SAFE_DELETE(m_pInplaceButtons);
	SAFE_DELETE(m_pInplaceList);

	CMDTARGET_RELEASE(m_pDropTarget);
	CMDTARGET_RELEASE(m_pSelectedRowsBeforeDrag);
	CMDTARGET_RELEASE(m_ptrVirtualEditingRow);

	m_pHeaderRecords->m_pMarkupContext = NULL;
	m_pFooterRecords->m_pMarkupContext = NULL;

	CMDTARGET_RELEASE(m_pHeaderRecords);
	CMDTARGET_RELEASE(m_pFooterRecords);
	CMDTARGET_RELEASE(m_pHeaderRows);
	CMDTARGET_RELEASE(m_pFooterRows);

	CMDTARGET_RELEASE(m_pDropRecords);

	if (m_hbmpWatermark)
		::DeleteObject(m_hbmpWatermark);
//ICON_VIEW_MODE RELATED <<
	CMDTARGET_RELEASE(m_pPrevVisible);
	CMDTARGET_RELEASE(m_pPrevGroupsOrder);
//ICON_VIEW_MODE RELATED >>
	CMDTARGET_RELEASE(m_pDataManager);
}

void CXTPReportControl::ResetContent(BOOL bUpdateControl)
{
	EditItem(NULL);

	m_arrScreenRows.Clear();

	if (m_pRows)
		m_pRows->Clear();
	if (m_pPlainTree)
		m_pPlainTree->Clear();
	if (m_pSelectedRows)
		m_pSelectedRows->Clear();

//  m_pHeaderRows->Clear();
//  m_pFooterRows->Clear();

//  m_pHeaderRecords->RemoveAll();
//  m_pFooterRecords->RemoveAll();

	if (m_pRecords)
		m_pRecords->RemoveAll();

	if (bUpdateControl)
	{
		AdjustIndentation();
		AdjustScrollBars();
		RedrawControl();
	}
}


void CXTPReportControl::SetReportHeader(CXTPReportHeader* pReportHeader)
{
	if (pReportHeader)
	{
		m_pReportHeader->InternalRelease();
		m_pReportHeader = pReportHeader;
		AdjustLayout();
	}
}

void CXTPReportControl::SetImageManager(CXTPImageManager* pImageManager)
{
	if (pImageManager)
	{
		m_pImageManager->InternalRelease();
		m_pImageManager = pImageManager;
	}

}

void CXTPReportControl::SetImageList(CImageList* pImageList)
{
	for (int i = 0; i < pImageList->GetImageCount(); i++)
	{
		HICON hIcon = pImageList->ExtractIcon(i);
		m_pImageManager->SetIcon(hIcon, i);

		DestroyIcon(hIcon);
	}
}

void CXTPReportControl::SetPaintManager(CXTPReportPaintManager* pPaintManager)
{
	if (pPaintManager)
	{
		m_pPaintManager->SetReportControl(NULL);
		m_pPaintManager->InternalRelease();

		m_pPaintManager = pPaintManager;
		m_pPaintManager->SetReportControl(this);

		AdjustLayout();
		AdjustScrollBars();
	}
}


BOOL CXTPReportControl::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext)
{
	if (!CWnd::Create(XTPREPORTCTRL_CLASSNAME, NULL, dwStyle, rect, pParentWnd, nID, pContext))
		return FALSE;

	return TRUE;
}

int CXTPReportControl::GetIndent(int nLevel) const
{
	return max(0, (nLevel - 1) * m_pPaintManager->m_nTreeIndent);
}

void CXTPReportControl::SetTreeIndent(int nIndent)
{
	m_pPaintManager->m_nTreeIndent = nIndent;
}

void CXTPReportControl::BeginUpdate()
{
	m_nLockUpdateCount++;
	m_bRefreshIndexes = FALSE;
}

void CXTPReportControl::EndUpdate()
{
	SetChanged();
	m_nLockUpdateCount--;

	if (m_nLockUpdateCount == 0)
	{
		if (m_bRefreshIndexes)
		{
			m_bRefreshIndexes = FALSE;
			_RefreshIndexes();
		}

		RedrawControl();
	}
}

void CXTPReportControl::RedrawControl()
{
	SetChanged();
	if (m_nLockUpdateCount == 0 && GetSafeHwnd())
		Invalidate(FALSE);
}

void CXTPReportControl::UpdateSubList()
{
	CXTPReportHeader* pHeader = GetReportHeader();
	if (pHeader && pHeader->m_pSubList &&
		(pHeader->m_pSubList->GetReportCtrl() == this))
	{
		pHeader->m_pSubList->UpdateList();
	}
}

void CXTPReportControl::_DoCollapse(CXTPReportRow* pRow)
{
	int nIndex = pRow->GetIndex() + 1;
	int nCount = 0;

	while (nIndex < m_pRows->GetCount())
	{
		CXTPReportRow* pRowChild = m_pRows->GetAt(nIndex);
		if (pRowChild)
		{
			if (!pRowChild->HasParent(pRow))
				break;

			pRowChild->m_bVisible = FALSE;
			pRowChild->m_nIndex = -1;
			m_pRows->RemoveAt(nIndex);
			nCount++;
		}
	}
	if (nCount > 0)
	{
		m_pSelectedRows->_OnCollapsed(nIndex - 1, nCount);

		if (m_nFocusedRow >= nIndex)
			m_nFocusedRow = max(nIndex - 1, m_nFocusedRow - nCount);
	}
}

void CXTPReportControl::_DoExpand(CXTPReportRow* pRow)
{
	int nIndex = pRow->m_nIndex;
	int nCount = _DoExpand(nIndex, pRow);

	if (nCount > 0)
	{
		m_pSelectedRows->_OnExpanded(nIndex, nCount);

		if (m_nFocusedRow > nIndex)
			m_nFocusedRow += nCount;
	}
}

int CXTPReportControl::_DoExpand(int nIndex, CXTPReportRow* pRow)
{
	if (!pRow->HasChildren())
		return 0;

	int nStartIndex = nIndex;
	for (int i = 0; i < pRow->GetChilds()->GetCount(); i++)
	{
		CXTPReportRow* pRowChild = pRow->GetChilds()->GetAt(i);
		if (pRowChild)
		{
			pRowChild->m_nRowLevel = pRow->m_nRowLevel + 1;
			pRowChild->m_nGroupLevel = pRow->m_nGroupLevel + (pRow->IsGroupRow() ? 1 : 0);

			nIndex += InsertRow(nIndex + 1, pRowChild);
		}
	}
	return nIndex - nStartIndex;
}

void CXTPReportControl::RefreshIndexes(BOOL bAdjustLayout, BOOL bReverseOrder)
{
	int nRowCount = m_pRows->GetCount();
	int nStartIdx = 0, nEndIdx = nRowCount, nStep = 1;
	if (bReverseOrder)
	{
		nStartIdx = nRowCount - 1;
		nEndIdx = nStep = -1;
	}
	for (int nIndex = nStartIdx; nIndex != nEndIdx; nIndex += nStep)
	{
		CXTPReportRow* pRow = m_pRows->GetAt(nIndex);
		if (pRow)
		{
			BOOL bSelected = m_pSelectedRows->Contains(pRow);
			if (bSelected)
				m_pSelectedRows->Remove(pRow);

			pRow->m_nIndex = nIndex;
			ASSERT(pRow->m_bVisible);

			if (bSelected)
				m_pSelectedRows->Add(pRow);
		}
	}

	if (bAdjustLayout)
		AdjustScrollBars();
}

void CXTPReportControl::_RefreshIndexes(BOOL bAdjustLayout, BOOL bReverseOrder)
{
	if (m_nLockUpdateCount != 0)
	{
		m_bRefreshIndexes = TRUE;
		return;
	}

	RefreshIndexes(bAdjustLayout, bReverseOrder);
}

int CXTPReportControl::InsertRow(int nIndex, CXTPReportRow* pRow)
{
	m_pRows->InsertAt(nIndex, pRow);
	pRow->InternalAddRef();
	pRow->m_bVisible = TRUE;


	int nRowsInserted = 1;

	if (pRow->IsExpanded() && pRow->HasChildren())
		nRowsInserted += _DoExpand(nIndex, pRow);

	return nRowsInserted;
}

void CXTPReportControl::BuildTree(CXTPReportRows* pTree, CXTPReportRow* pParentRow, CXTPReportRecords* pRecords)
{
	ASSERT(pTree->GetCount() == 0);

	pTree->ReserveSize(pRecords->GetCount());

	for (int i = 0; i < pRecords->GetCount(); i++)
	{
		CXTPReportRecord* pRecord = pRecords->GetAt(i);
		if (pRecord == NULL) continue;

		// add record if all conditions are met
		if (pRecord->IsLocked() ||
			(pRecord->IsVisible() && !ApplyFilter(pRecord, GetFilterText(), IsPreviewMode())))
		{
			CXTPReportRow* pRow = CreateRow();
			pRow->InitRow(this, pRecord);

			pRow->m_pParentRow = pParentRow;
			pTree->Add(pRow);

			if (pRecord->HasChildren())
			{
				BuildTree(pRow->GetChilds(), pRow, pRecord->GetChilds());

				if (pRow->GetChilds() && IsSortRecordChilds())
				{
					SortRows(pRow->GetChilds());

					pRow->GetChilds()->RefreshChildIndices(FALSE);
				}
			}
		}
	}
}

void CXTPReportControl::SortTree(CXTPReportRows* pTree)
{
	ASSERT(pTree);
	if (!pTree)
		return;

	SortRows(pTree);

	for (int i = 0; i < pTree->GetCount(); i++)
	{
		CXTPReportRow* pRow = pTree->GetAt(i);
		ASSERT(pRow);
		if (!pRow)
			continue;


		BOOL bRecordHasChildren = pRow->GetRecord() && pRow->GetRecord()->HasChildren();

		if (pRow->HasChildren() && pRow->GetChilds() &&
			(bRecordHasChildren && m_bSortRecordChilds || !bRecordHasChildren))
		{
			SortTree(pRow->GetChilds());
		}
	}

	pTree->RefreshChildIndices(FALSE);
}

void CXTPReportControl::ReSortRows()
{
	if (IsVirtualMode())
	{
		Populate();
		return;
	}

// are there any selected rows?
	BOOL bRowsSelected = m_bInitialSelectionEnable;
	if (GetSelectedRows()!= NULL)
	{
		if (GetSelectedRows()->GetCount() > 0)
			bRowsSelected = TRUE;
	}

	SortTree(m_pPlainTree);

	CXTPReportRecord* pFocusedRecord = GetFocusedRow() ? GetFocusedRow()->GetRecord() : NULL;

	m_pRows->Clear();
	m_arrScreenRows.Clear(FALSE);

	for (int nGroupRow = 0; nGroupRow < m_pPlainTree->GetCount(); nGroupRow++)
	{
		CXTPReportRow* pRow = m_pPlainTree->GetAt(nGroupRow);
		InsertRow(m_pRows->GetCount(), pRow);
		pRow->m_nChildIndex = nGroupRow;
		ASSERT(pRow->m_pParentRows == m_pPlainTree);
	}

	//-----------------------------------------------------------------------
	m_nFocusedRow = -1;
	// Update indexes on virtual rows
	int nRowCount = m_pRows->GetCount();
	for (int nRowIndex = 0; nRowIndex < nRowCount; nRowIndex++)
	{
		CXTPReportRow* pRow = m_pRows->GetAt(nRowIndex);
		if (pRow)
		{
			pRow->SetIndex(nRowIndex);
			pRow->m_bVisible = TRUE;
			CXTPReportRecord* pRec = pRow->GetRecord();
			if (pRec)
			{
				if (pFocusedRecord && pRec == pFocusedRecord)
				{
					m_nFocusedRow = pRow->GetIndex();
					if (IsSelectionEnabled() && bRowsSelected)
						m_pSelectedRows->Select(pRow);
				}
				BOOL b1 = pRow->IsSelected();
				BOOL b2 = pRec->m_bSelectedAsChildFlag;
				if (b1 != b2)
					pRow->SetSelected(b2);
			}
		}
	}

	AdjustIndentation();
	AdjustLayout();

	RedrawControl();
}

CLIPFORMAT CXTPReportControl::EnableDragDrop(LPCTSTR lpszClipboardFormat, DWORD dwFlags, DWORD dwDropMarkerFlags)
{
	if (m_dwDragDropFlags != 0)
	{
		if (m_pDropTarget)
			m_pDropTarget->Revoke();
	}

	m_dwDragDropFlags = dwFlags;
	m_dwDropMarkerFlags = dwDropMarkerFlags;
	m_cfReport = NULL;

	if (m_dwDragDropFlags != 0)
	{
		if (m_pDropTarget)
		{
			m_cfReport = (CLIPFORMAT)::RegisterClipboardFormat(lpszClipboardFormat);

			m_pDropTarget->Revoke(); // to ensure kill previous registration.

			m_pDropTarget->Register(this);
		}
	}

	return m_cfReport;
}

BOOL CXTPReportControl::ApplyFilter(CXTPReportRecord* pRecord, CString strFilterText, BOOL bIncludePreview)
{
	// not filtered if filter text is empty
	if (!pRecord)
		return FALSE;

	if (pRecord->IsFiltered())
		return TRUE;

	if (strFilterText.IsEmpty())
		return FALSE;

	BOOL bFilterSimple = GetRecordsTreeFilterMode() == xtpReportFilterTreeSimple;

	if (!bFilterSimple && pRecord->HasChildren() && pRecord->GetChilds())
	{
		CXTPReportRecords* pChildren = pRecord->GetChilds();
		int nCount = pChildren->GetCount();
		for (int i = 0; i < nCount; i++)
		{
			CXTPReportRecord* pR = pChildren->GetAt(i);
			if (!ApplyFilter(pR, strFilterText, bIncludePreview))
				return FALSE;
		}

	}

	if (GetRecordsTreeFilterMode() == xtpReportFilterTreeByEndChildrenOnly
		&& pRecord->HasChildren())
		return TRUE;

	return _ApplyFilter(pRecord, strFilterText, bIncludePreview);

}

BOOL CXTPReportControl::_ApplyFilter(CXTPReportRecord* pRecord, CString strFilterText, BOOL bIncludePreview)
{
	// not filtered if filter text is empty
	if (!pRecord)
		return FALSE;

	if (pRecord->IsFiltered())
		return TRUE;

	if (strFilterText.IsEmpty())
		return FALSE;

	BOOL bFilterHidden = IsFilterHiddenColumns();

	// process each token in the filter string
	TCHAR szSeps[] = _T(" \t");
	TCHAR szSepsStrict[] = _T("\t");

	TCHAR *szToken, *lpszContext = 0;
	//int nCurPos = 0;

	if (m_bStrictFiltering)
		szToken = STRTOK_S(strFilterText.GetBuffer(strFilterText.GetLength()), szSepsStrict, &lpszContext);
	else
		szToken = STRTOK_S(strFilterText.GetBuffer(strFilterText.GetLength()), szSeps, &lpszContext);
	while (szToken != NULL)
	{
		CString strToken(szToken);
		strToken.MakeLower();
		BOOL bTokenFound = FALSE;
		// enumerate all visible columns
		int nColumnsCount = m_pColumns->GetCount();
		for (int nCol = 0; nCol < nColumnsCount; nCol++)
		{
			CXTPReportColumn* pCol = m_pColumns->GetAt(nCol);
			if (pCol && (pCol->IsVisible() || bFilterHidden) && pCol->IsFiltrable())
			{
				CXTPReportRecordItem* pItem = pRecord->GetItem(pCol);
				if (pItem)
				{
					CString sItemText = pItem->GetCaption(pCol);
					// case-insensitive search
					sItemText.MakeLower();
					bTokenFound = sItemText.Find(strToken) != -1;

					if (bTokenFound)
					{
						// stop search current token - passed
						break;
					}
				}
			}
		}
		// also check preview text
		if (bIncludePreview && !bTokenFound && pRecord->GetItemPreview())
		{
			CString sItemText = pRecord->GetItemPreview()->GetCaption(NULL);
			// case-insensitive search
			sItemText.MakeLower();
			bTokenFound = sItemText.Find(strToken) != -1;
		}

		// Token not found - filter this record
		if (!bTokenFound)
		{
			return TRUE;
		}

		// get next token
		if (m_bStrictFiltering)
			szToken = STRTOK_S(NULL, szSepsStrict, &lpszContext);
		else
			szToken = STRTOK_S(NULL, szSeps, &lpszContext);
	}

	return FALSE;
}

void CXTPReportControl::Populate()
{
	EditItem(NULL);

	BeginUpdate();

	m_nPopulatedRecordsCount = 0;

	// save focused items
	CXTPReportRecord* pFocusedRecord = GetFocusedRow() ? GetFocusedRow()->GetRecord() : NULL;
	m_pSelectedRows->Clear();
	m_nFocusedRow = -1;
	m_nFocusedHeaderRow = -1;
	m_nFocusedFooterRow = -1;

	m_pRows->Clear();
	m_pPlainTree->Clear();
	m_arrScreenRows.Clear(FALSE);
	m_pHeaderRows->Clear();
	m_pFooterRows->Clear();

	if (IsVirtualMode())
	{
		CXTPReportRow* pRow = CreateRow();
		pRow->InitRow(this, GetRecords()->m_pVirtualRecord);
		pRow->m_bVisible = TRUE;

		m_pRows->SetVirtualMode(pRow, GetRecords()->GetCount());
		m_nPopulatedRecordsCount = GetRecords()->GetCount();
	}
	else
	{
		BuildTree(m_pPlainTree, NULL, m_pRecords);
		m_nPopulatedRecordsCount = m_pPlainTree->GetCount();

		SortRows(m_pPlainTree);

		int nGroupRowsCount = 0;

		if (m_pColumns->GetGroupsOrder()->GetCount() > 0)
		{
			CXTPReportRows* pGroupTree = new CXTPHeapObjectT<CXTPReportRows, CXTPReportAllocatorDefault>;

			int nReserve = m_pRecords->GetCount() / (m_pColumns->GetGroupsOrder()->GetCount() + 1);
			nReserve = max(nReserve, 300);
			pGroupTree->ReserveSize(nReserve);

			CXTPReportGroupRow* pLastGroup = NULL;

			for (int nPlainRow = 0; nPlainRow < m_pPlainTree->GetCount(); nPlainRow++)
			{
				CXTPReportRow* pRow = m_pPlainTree->GetAt(nPlainRow);
				CXTPReportGroupRow* pGroupToAdd = NULL;

				for (int nColumn = 0; nColumn < m_pColumns->GetGroupsOrder()->GetCount(); nColumn++)
				{
					CXTPReportColumn* pColumn = m_pColumns->GetGroupsOrder()->GetAt(nColumn);
					CXTPReportRecordItem* pItem = pRow->GetRecord()->GetItem(pColumn);
					CString strGroup = pItem ? pItem->GetGroupCaption(pColumn) : _T("");

					if (pLastGroup && GetRecords()->Compare(pLastGroup->GetCaption(), strGroup) == 0)
					{
						pGroupToAdd = pLastGroup;

						if (pGroupToAdd->HasChildren())
							pLastGroup = (CXTPReportGroupRow*)pGroupToAdd->GetChilds()->GetAt(pGroupToAdd->GetChilds()->GetCount() - 1);
					}
					else
					{
						CXTPReportGroupRow* pGroup = CreateGroupRow();
						nGroupRowsCount++;

						pGroup->InitRow(this, NULL);
						pGroup->SetCaption(strGroup);

						if (pGroupToAdd)
							pGroupToAdd->AddChild(pGroup);
						else
							pGroupTree->Add(pGroup);

						pGroupToAdd = pGroup;
						pLastGroup = NULL;
					}

				}
				if (pGroupToAdd)
					pGroupToAdd->AddChild(pRow);
				pRow->InternalAddRef();
				pLastGroup = (CXTPReportGroupRow*)pGroupTree->GetAt(pGroupTree->GetCount() - 1);
			}

			m_pPlainTree->InternalRelease();
			m_pPlainTree = pGroupTree;
		}

		m_pRows->ReserveSize(m_pRecords->GetCount() + nGroupRowsCount + 10);

		for (int nGroupRow = 0; nGroupRow < m_pPlainTree->GetCount(); nGroupRow++)
		{
			CXTPReportRow* pRow = m_pPlainTree->GetAt(nGroupRow);
			InsertRow(m_pRows->GetCount(), pRow);
			pRow->m_nChildIndex = nGroupRow;
			ASSERT(pRow->m_pParentRows == m_pPlainTree);
		}

		m_nFocusedRow = -1;
		m_nFocusedHeaderRow = -1;
		m_nFocusedFooterRow = -1;

		// Update indexes on virtual rows
		int nRowCount = m_pRows->GetCount();
		for (int nRowIndex = 0; nRowIndex < nRowCount; nRowIndex++)
		{
			CXTPReportRow* pRow = m_pRows->GetAt(nRowIndex);
			if (pRow)
			{
				pRow->SetIndex(nRowIndex);
				pRow->m_bVisible = TRUE;

				if (pFocusedRecord && pRow->GetRecord() == pFocusedRecord)
				{
					m_nFocusedRow = pRow->GetIndex();
					if (IsSelectionEnabled() && IsInitialSelectionEnabled())
					{
						//m_pSelectedRows->Select(pRow);
						CXTPReportRow* pFirst = pRow;
						if (pFirst)
						{
							if (!IsSkipGroupsFocusEnabled() || !pFirst->IsGroupRow())
								m_pSelectedRows->Select(pFirst);
							else
							{
								while (pFirst->IsGroupRow())
								{
									if (pFirst->HasChildren())
										pFirst = pFirst->GetChilds()->GetAt(0);
								}
								m_pSelectedRows->Select(pFirst);
							}
						}
					}
				}
			}
		}

		int i;
		// header record rows
		for (i = 0; i < m_pHeaderRecords->GetCount(); i++)
		{
			CXTPReportRecord* pRecord = m_pHeaderRecords->GetAt(i);
			CXTPReportRow* pRow = CreateHeaderFooterRow();

			pRow->InitRow(this, pRecord);
			pRow->SetIndex(i);
			pRow->m_nRowType = xtpRowTypeHeader;
			pRow->m_pParentRow = NULL;
			m_pHeaderRows->Add(pRow);
		}

		// footer record rows
		for (i = 0; i < m_pFooterRecords->GetCount(); i++)
		{
			CXTPReportRecord* pRecord = m_pFooterRecords->GetAt(i);
			CXTPReportRow* pRow = CreateHeaderFooterRow();

			pRow->InitRow(this, pRecord);
			pRow->SetIndex(i);
			pRow->m_nRowType = xtpRowTypeFooter;
			pRow->m_pParentRow = NULL;
			m_pFooterRows->Add(pRow);
		}
	}

	AdjustIndentation();
	AdjustLayout();
	AdjustScrollBars();

	UpdateSubList();

	if (m_nFocusedRow == -1)
	{
		SetTopRow(0);

		if (GetRows()->GetCount() > 0)
		{
			m_nFocusedRow = 0;
			if (IsSelectionEnabled() && IsInitialSelectionEnabled())
			{
				CXTPReportRow* pFirst = m_pRows->GetAt(0);
				if (pFirst)
				{
					if (!IsSkipGroupsFocusEnabled() || !pFirst->IsGroupRow())
						m_pSelectedRows->Select(pFirst);
					else
					{
						while (pFirst->IsGroupRow())
						{
							if (pFirst->HasChildren())
								pFirst = pFirst->GetChilds()->GetAt(0);
						}
						m_pSelectedRows->Select(pFirst);
					}
				}
			}
		}

	}
	else
	{
		EnsureVisible(GetFocusedRow());
	}

	if (m_pFocusedColumn == NULL && m_bFocusSubItems)
	{
		m_pFocusedColumn = m_pColumns->GetFirstVisibleColumn();
//------------ Forum proposal 2009 June 19 ---------------------------- <<
		if (GetRows())
		{
			CXTPReportRow* pRow = GetRows()->GetAt(GetTopRowIndex());
			if (pRow)
				m_pFocusedColumn = GetNextFocusableColumn(pRow, 0, 1);
		}
//------------ Forum proposal 2009 June 19 ---------------------------- >>
	}
	EndUpdate();
}

void CXTPReportControl::PopulateHeaderRows()
{
	if (IsVirtualMode())
		return;

	EditItem(NULL);
	BeginUpdate();

	m_pHeaderRows->Clear();
	for (int i = 0; i < m_pHeaderRecords->GetCount(); i++)
	{
		CXTPReportRecord* pRecord = m_pHeaderRecords->GetAt(i);
		CXTPReportRow* pRow = CreateHeaderFooterRow();

		pRow->InitRow(this, pRecord);
		pRow->SetIndex(i);
		pRow->m_nRowType = xtpRowTypeHeader;
		pRow->m_pParentRow = NULL;
		m_pHeaderRows->Add(pRow);
	}
	AdjustIndentation();
	AdjustLayout();
	AdjustScrollBars();

	UpdateSubList();

	EndUpdate();
}

void CXTPReportControl::PopulateFooterRows()
{
	if (IsVirtualMode())
		return;

	EditItem(NULL);
	BeginUpdate();

	m_pFooterRows->Clear();
	for (int i = 0; i < m_pFooterRecords->GetCount(); i++)
	{
		CXTPReportRecord* pRecord = m_pFooterRecords->GetAt(i);
		CXTPReportRow* pRow = CreateHeaderFooterRow();

		pRow->InitRow(this, pRecord);
		pRow->SetIndex(i);
		pRow->m_nRowType = xtpRowTypeFooter;
		pRow->m_pParentRow = NULL;
		m_pFooterRows->Add(pRow);
	}
	AdjustIndentation();
	AdjustLayout();
	AdjustScrollBars();

	UpdateSubList();

	EndUpdate();
}

void CXTPReportControl::SortRows(CXTPReportRows* pRows)
{
	if (pRows->GetCount() == 0)
		return;

	if (m_pColumns->GetSortOrder()->GetCount() == 0
		&& m_pColumns->GetGroupsOrder()->GetCount() == 0)
		return;

	m_UaPreSorted.RemoveAll();
	CXTPReportRow* pRow;
	CXTPReportRecord* pRec;
	for (int i = 0; i < pRows->GetCount(); i++)
	{
		pRow = pRows->GetAt(i);
		if (pRow)
		{
			pRec = pRow->GetRecord();
			if (pRec)
				m_UaPreSorted.Add(pRec->GetIndex());
		}
	}

	if (m_pRowsCompareFunc && m_pRowsCompareFunc != CXTPReportRows::CompareRows)
		pRows->SortEx(m_pRowsCompareFunc);
	else
		pRows->Sort();

	m_UaSorted.RemoveAll();
	for (int ii = 0; ii < pRows->GetCount(); ii++)
	{
		pRow = pRows->GetAt(ii);
		if (pRow)
		{
			pRec = pRow->GetRecord();
			if (pRec)
				m_UaSorted.Add(pRec->GetIndex());
		}
	}
}

void CXTPReportControl::AdjustIndentation()
{
	GetReportHeader()->m_nIndentLevel = m_pColumns->GetGroupsOrder()->GetCount();
}

CXTPReportColumn* CXTPReportControl::AddColumn(CXTPReportColumn* pColumn)
{
	ASSERT(pColumn);
	m_pColumns->Add(pColumn);

	return pColumn;
}

CXTPReportRecord* CXTPReportControl::AddRecord(CXTPReportRecord* pRecord)
{
	ASSERT(pRecord);
	if (!pRecord)
		return NULL;

	m_pRecords->Add(pRecord);

	return pRecord;
}

void CXTPReportControl::AddRecordEx(CXTPReportRecord* pRecord, CXTPReportRecord* pParentRecord,
									int nRowChildIndex, int nRecordChildIndex)
{
	ASSERT(pRecord);
	if (!pRecord)
		return;

	// add record
	BOOL bAddRecord = TRUE;
	CXTPReportRecords* pParentRecords = pParentRecord ? pParentRecord->HasChildren() ? pParentRecord->GetChilds() : NULL : m_pRecords;
	if (pParentRecords)
	{
		for (int nChild = 0; nChild < pParentRecords->GetCount(); nChild++)
		{
			if (pRecord == pParentRecords->GetAt(nChild))
			{
				bAddRecord = FALSE;
				break;
			}
		}
	}
	if (bAddRecord)
	{
		CXTPReportRecords* pRecords = pParentRecord ? pParentRecord->GetChilds() : m_pRecords;

		if (nRecordChildIndex >= 0 && nRecordChildIndex < pRecords->GetCount())
			pRecords->InsertAt(nRecordChildIndex, pRecord);
		else
			pRecords->Add(pRecord);
	}

	if (!pRecord->IsVisible())
		return;

	// find parent record rows
	CXTPReportRows* pParentRows = m_pPlainTree;
	CXTPReportRow* pParentRow = NULL;
	if (pParentRecord)
	{
		pParentRow = m_pPlainTree->FindInTree(pParentRecord);
		if (pParentRow)
			pParentRows = pParentRow->GetChilds();
	}
	BOOL bRoot = pParentRows == m_pPlainTree;

	// create new row
	CXTPReportRow* pNewRow = CreateRow();
	pNewRow->InitRow(this, pRecord);
	pNewRow->m_pParentRow = NULL;

	// add row to pParentRows
	BOOL bInsertAfter = FALSE;
	CXTPReportRow* pPlainTreeRow = NULL;
	CXTPReportRow* pInsertRowPos = NULL;
	int nNextSiblingIndex = m_pRows->GetCount();
	if (bRoot)
	{
		pInsertRowPos = pParentRows->FindInsertionPos(pNewRow, bInsertAfter);
		CXTPReportRow* pRow = pInsertRowPos;
		while (pRow)
		{
			if (pRow->GetNextSiblingRow())
			{
				nNextSiblingIndex = pRow->GetNextSiblingRow()->GetIndex();
				break;
			}
			else
				pRow = pRow->GetParentRow();
		}
		if (m_pColumns->GetGroupsOrder()->GetCount() > 0)
		{
			CXTPReportGroupRow* pGroupToAdd = pInsertRowPos ? (CXTPReportGroupRow*)pInsertRowPos->GetParentRow() : NULL;
			if (!pInsertRowPos || (pInsertRowPos && pInsertRowPos->IsGroupRow()))
			{
				for (int nColumn = pInsertRowPos ? pInsertRowPos->GetGroupLevel() : 0; nColumn < m_pColumns->GetGroupsOrder()->GetCount(); nColumn++)
				{
					CXTPReportColumn* pColumn = m_pColumns->GetGroupsOrder()->GetAt(nColumn);
					CXTPReportRecordItem* pItem = pRecord->GetItem(pColumn);
					CString strGroup = pItem ? pItem->GetGroupCaption(pColumn) : _T("");

					CXTPReportGroupRow* pGroup = CreateGroupRow();
					pGroup->InitRow(this, NULL);
					pGroup->SetCaption(strGroup);
					pGroup->m_nGroupLevel = pGroupToAdd ? pGroupToAdd->m_nRowLevel + 1 : 0;
					pGroup->m_nRowLevel = nColumn;
					if (pGroupToAdd)
					{
						if (!pPlainTreeRow)
						{
							pPlainTreeRow = pGroup;
							pGroupToAdd->GetChilds()->InsertAt(bInsertAfter ? pInsertRowPos ? pInsertRowPos->m_nChildIndex + 1 : 0 : pInsertRowPos ? pInsertRowPos->m_nChildIndex : 0, pGroup);
						}
						else
							pGroupToAdd->AddChild(pGroup);
						pGroup->m_pParentRows = pGroupToAdd->GetChilds();
						pGroup->m_pParentRow = pGroupToAdd;
						pGroup->m_bVisible = pGroupToAdd->IsExpanded();
					}
					else
					{
						pPlainTreeRow = pGroup;
						pParentRows->InsertAt(pInsertRowPos ? pInsertRowPos->m_nChildIndex : pParentRows->GetCount(), pGroup);
						pGroup->m_pParentRows = pParentRows;
						pGroup->m_pParentRow = NULL;
						pGroup->m_bVisible = TRUE;
					}
					pGroupToAdd = pGroup;
				}
				// insert row
				if (pGroupToAdd)
				{
					pGroupToAdd->AddChild(pNewRow);
					pNewRow->m_pParentRows = pGroupToAdd->GetChilds();
	//              pNewRow->m_pParentRow = pGroupToAdd;
					pNewRow->m_bVisible = pGroupToAdd->IsExpanded();
				}
			}
			else
			{
				// insert row
				if (pGroupToAdd)
				{
					pGroupToAdd->GetChilds()->InsertAt(bInsertAfter ? pInsertRowPos ? pInsertRowPos->m_nChildIndex + 1 : 0 : pInsertRowPos ? pInsertRowPos->m_nChildIndex : 0, pNewRow);
					pNewRow->m_pParentRows = pGroupToAdd->GetChilds();
					pNewRow->m_pParentRow = pGroupToAdd;
					pNewRow->m_bVisible = pGroupToAdd->IsExpanded();
				}
			}
		}
		else
		{
			pParentRows->InsertAt(pInsertRowPos ? pInsertRowPos->m_nChildIndex : pParentRows->GetCount(), pNewRow);
			pNewRow->m_pParentRows = pParentRows;
			pNewRow->m_pParentRow = NULL;
			pNewRow->m_bVisible = TRUE;
		}
	}
	else
	{
		if (nRowChildIndex >= 0 && nRowChildIndex < pParentRows->GetCount())
			pParentRows->InsertAt(nRowChildIndex, pNewRow);
		else
			pParentRows->Add(pNewRow);
		pNewRow->m_pParentRows = pParentRows;
		pNewRow->m_pParentRow = pParentRow;
		pNewRow->m_bVisible = TRUE;
		pNewRow->m_nRowLevel = pParentRow->m_nRowLevel + 1;
		pNewRow->m_nGroupLevel = pParentRow->m_nGroupLevel;
		pNewRow->m_nChildIndex = nRowChildIndex >= 0 ? nRowChildIndex : pParentRows->GetCount() - 1;

		CXTPReportRow* pRow = pNewRow;
		while (pRow)
		{
			if (pRow->GetNextSiblingRow())
			{
				nNextSiblingIndex = pRow->GetNextSiblingRow()->GetIndex();
				break;
			}
			else
				pRow = pRow->GetParentRow();
		}
	}
	// refresh child indices
	if (pInsertRowPos && pInsertRowPos->GetParentRows())
		pInsertRowPos->GetParentRows()->RefreshChildIndices();
	else
		pParentRows->RefreshChildIndices();

	// add row to m_pRows
	if (bRoot)
	{
		if (pInsertRowPos)
		{
			CXTPReportRow* pRow = pInsertRowPos->GetParentRow();
			BOOL bIsExpanded = TRUE;
			while (pRow && bIsExpanded)
			{
				bIsExpanded = bIsExpanded && pRow->IsExpanded();
				pRow = pRow->GetParentRow();
			}

			if (bIsExpanded)
			{
				if (bInsertAfter)
					InsertRow(nNextSiblingIndex, pPlainTreeRow ? pPlainTreeRow : pNewRow);
				else
					InsertRow(pInsertRowPos->GetIndex(), pPlainTreeRow ? pPlainTreeRow : pNewRow);
			}
		}
		else
		{
			InsertRow(m_pRows->GetCount(), pPlainTreeRow ? pPlainTreeRow : pNewRow);
		}
	}
	else
	{
		CXTPReportRow* pRow = pNewRow->GetParentRow();
		BOOL bIsExpanded = TRUE;
		while (pRow && bIsExpanded)
		{
			bIsExpanded = bIsExpanded && pRow->IsExpanded();
			pRow = pRow->GetParentRow();
		}

		if (bIsExpanded)
			InsertRow(nNextSiblingIndex, pNewRow);
	}

	RefreshIndexes(FALSE, TRUE);

	// add children
	if (pRecord && pRecord->HasChildren())
	{
		for (int nChild = 0; nChild < pRecord->GetChilds()->GetCount(); nChild++)
		{
			AddRecordEx(pRecord->GetChilds()->GetAt(nChild), pRecord);
		}
	}
}

BOOL CXTPReportControl::RemoveRowEx(CXTPReportRow* pRow, BOOL bAdjustLayout)
{
	ASSERT(pRow);
	if (!pRow)
		return FALSE;

	//------------------------------------------------
	if (!pRow->IsGroupRow())
	{
		ASSERT(pRow->GetRecord());
		return RemoveRecordEx(pRow->GetRecord(), bAdjustLayout);
	}

	//------------------------------------------------
	CWaitCursor _WC;

	int nCount = pRow->GetChilds()->GetCount();

	for (int i = nCount - 1; i >= 0 ; i--)
	{
		CXTPReportRow* pRowI = pRow->GetChilds()->GetAt(i);
		RemoveRowEx(pRowI, bAdjustLayout);
	}

	return TRUE;
}

BOOL CXTPReportControl::RemoveRecordEx(CXTPReportRecord* pRecord, BOOL bAdjustLayout, BOOL bRemoveFromParent)
{
	ASSERT(pRecord && m_pRecords);

	if (!pRecord || !m_pRecords)
		return FALSE;

	if (pRecord->HasChildren())
	{
		for (int i = pRecord->GetChilds()->GetCount() - 1; i >= 0 ; i--)
		{
			RemoveRecordEx(pRecord->GetChilds()->GetAt(i), bAdjustLayout, FALSE);
		}

//      return RemoveRecordEx(pRecord, bAdjustLayout);
	}

	BOOL bResult = FALSE;

	//--------------------------------------------------------
	CXTPReportRow* pRow0 = m_pPlainTree->FindInTree(pRecord);

	ASSERT_DBG_REMOVE_RECORD_EX(pRow0);

	if (pRow0)
	{
		CXTPReportRow* pRow = pRow0;
		pRow0 = NULL;

		do
		{
			CXTPReportRow* pRow_parent = pRow->GetParentRow();

			ASSERT(pRow->GetParentRows());

			// 1. Remove from selected rows
			if (m_pSelectedRows->Contains(pRow))
				m_pSelectedRows->Remove(pRow);

			// 2. remove from Rows Tree
			if (pRow->GetParentRows())
				VERIFY(pRow->GetParentRows()->RemoveRow(pRow) >= 0);

			// 3. remove from Display Rows array
			m_pRows->RemoveRow(pRow);

			pRow = pRow_parent;

		} while (pRow && pRow->IsGroupRow() && pRow->GetChilds()->GetCount() == 0);

		// refresh child indices
		if (pRow && pRow->HasChildren())
			pRow->GetChilds()->RefreshChildIndices();
		else
			m_pPlainTree->RefreshChildIndices();

		bResult = TRUE;
	}

	//-------------------------------------------------------
	//pRecord->Delete(); // the code below is more safe when record already removed!

	ASSERT(pRecord->GetRecords());

	if (bRemoveFromParent && pRecord->GetRecords())
	{
		BOOL bRecordRem = pRecord->GetRecords()->RemoveRecord(pRecord) >= 0;
		pRecord = NULL;
		ASSERT_DBG_REMOVE_RECORD_EX(bRecordRem);

		bResult |= bRecordRem;
	}

	if (m_nFocusedRow >= 0 && m_nFocusedRow >= m_pRows->GetCount())
		m_nFocusedRow = m_pRows->GetCount() - 1;

	//-------------------------------------------------------
	if (bResult)
		RefreshIndexes(bAdjustLayout);

	return bResult;
}

void CXTPReportControl::UpdateRecord(CXTPReportRecord* pRecord, BOOL bUpdateChildren)
{
	// get parent record
	CXTPReportRecord* pParentRecord = NULL;
	if (pRecord->GetRecords())
		pParentRecord = pRecord->GetRecords()->GetOwnerRecord();
	// update record
	if (!pParentRecord || bUpdateChildren)
	{
		// internal addref
		pRecord->TreeAddRef();

		// get record row child index
		CXTPReportRow* pRow = m_pPlainTree->FindInTree(pRecord);
		int nRowChildIndex = pRow->m_nChildIndex;
		int nRecordChildIndex = pRecord->GetIndex();
/*
		if (nRowChildIndex != nRecordChildIndex)
		{
			TRACE(_T("nRowChildIndex=%d nRecordChildIndex=%d\n"), nRowChildIndex, nRecordChildIndex);
		}
*/
		// remove record
		RemoveRecordEx(pRecord, FALSE);

		// add record
		pRecord->InternalAddRef(); //AddRecordEx expects that pRecord has +1 ref count.

		AddRecordEx(pRecord, pParentRecord, nRowChildIndex, nRecordChildIndex);

		// internal release
		pRecord->TreeRelease();
	}

	RedrawControl();
}

void CXTPReportControl::DrawNoItems(CDC* pDC, const CRect& rcClient)
{
	pDC->SetTextColor(GetPaintManager()->m_clrWindowText);
	CString strNoItems = GetPaintManager()->m_strNoItems;

	if (!strNoItems.IsEmpty())
	{
		CRect rcText(rcClient);
		rcText.DeflateRect(5, 5, 5, 5);
		CXTPFontDC font(pDC, &GetPaintManager()->m_fontText);
		UINT uFlags = DT_CENTER | DT_TOP | DT_NOPREFIX | DT_WORDBREAK |
			DT_WORD_ELLIPSIS | DT_END_ELLIPSIS | DT_EDITCONTROL;

		XTPMarkupDrawText(GetMarkupContext(), pDC->GetSafeHdc(), strNoItems, rcText, uFlags);
	}
}

void CXTPReportControl::DrawRows(CDC* pDC, CRect& rcClient)
{
	m_arrScreenRows.Clear(FALSE);

	pDC->SetBkMode(TRANSPARENT);

	int y = rcClient.top;

	int nRowCount = m_pRows->GetCount();

	if (0 == nRowCount)
	{
		DrawNoItems(pDC, rcClient);
	}

	if (GetReportHeader()->GetNextVisibleColumn(-1, 1) == NULL)
		return;

	int nRowHeight = 0;
	int nHeaderWidth = GetReportHeader()->GetWidth();

	m_arrScreenRows.SetSize(0, 200);

	if (!m_bIconView)
	{
		for (int i = m_nTopRow; i < nRowCount; i++)
		{
			CXTPReportRow* pRow = m_pRows->GetAt(i);
			ASSERT(pRow);
			if (!pRow)
				continue;

			if (y > rcClient.bottom)
				break;

			nRowHeight = pRow->GetHeight(pDC, nHeaderWidth);
			CRect rcRow(rcClient.left, y,
						rcClient.left + nHeaderWidth, y + nRowHeight);

			pRow->Draw(pDC, rcRow, m_nLeftOffset);
//GRID <<
			if (GetPaintManager()->m_bAllowMergeCells
				&& (GetColumns()->GetGroupsOrder()->GetCount() == 0)
				&& !GetReportHeader()->IsAllowColumnReorder()
				&& !GetReportHeader()->IsAllowColumnSort()
				&& !GetReportHeader()->IsAllowColumnRemove())
				y -= pRow->m_VertCorrection;
//GRID >>
			y += rcRow.Height();

			CXTPReportRow* pScreenRow = NULL;
			if (IsVirtualMode())
			{
				pScreenRow = CreateRow();
				pScreenRow->InitRow(pRow);
				pScreenRow->m_bExpanded = pRow->m_bExpanded;
				pScreenRow->m_rcCollapse = pRow->m_rcCollapse;
			}
			else
			{
				pScreenRow = pRow;
				pScreenRow->InternalAddRef();

			}
			m_arrScreenRows.InsertAt(m_arrScreenRows.GetCount(), pScreenRow);
		}

		// fill the rest of space with the "fake" rows
		if (GetPaintManager()->IsDrawGridForEmptySpace() && y < rcClient.bottom)
		{
			CRect rcEmpty(rcClient);
			rcEmpty.top = y;
			//DrawDefaultGrid(pDC, rcEmpty, nRowHeight, m_nLeftOffset); //can produce group-row height in some cases
			DrawDefaultGrid(pDC, rcEmpty, GetPaintManager()->m_nRowHeight, m_nLeftOffset); //always use standard row height
		}

	}
	else
	{
		m_iIconViewRowsPerLine = GetNumRowsOnLine(nHeaderWidth);
		int x = rcClient.left;
		int iLastMainRow = m_nTopRow;

		while(iLastMainRow % m_iIconViewRowsPerLine)
		{
			iLastMainRow++;
		}

		for (int i = iLastMainRow; i < nRowCount; i++)
		{
			CXTPReportRow* pRow = m_pRows->GetAt(i);
			ASSERT(pRow);
			if (!pRow)
				continue;

			if (y > rcClient.bottom)
				break;

			if (pRow->GetIndex() % m_iIconViewRowsPerLine)
			{
				x += m_iIconWidthSpacing;

				CRect rcRow(x, y, x + m_iIconWidthSpacing,
							y + m_iIconHeightSpacing);

				pRow->Draw(pDC, rcRow, 0);
			}
			else
			{
				x = rcClient.left;

				if (iLastMainRow != pRow->GetIndex())
				{
					y += m_iIconHeightSpacing;
					iLastMainRow = pRow->GetIndex();
				}

				CRect rcRow(x, y, x + m_iIconWidthSpacing,
							y + m_iIconHeightSpacing);

				pRow->Draw(pDC, rcRow, 0);
			}

			CXTPReportRow* pScreenRow = NULL;
			if (IsVirtualMode())
			{
				pScreenRow = CreateRow();
				pScreenRow->InitRow(pRow);
				pScreenRow->SetExpanded(pRow->IsExpanded());
				pScreenRow->m_rcCollapse = pRow->m_rcCollapse;
			}
			else
			{
				pScreenRow = pRow;
				pScreenRow->InternalAddRef();
			}
			m_arrScreenRows.InsertAt(m_arrScreenRows.GetCount(), pScreenRow);
		}
	}
}

void CXTPReportControl::DrawFixedRows(CDC* pDC, CRect& rcClient, CXTPReportRows* pRows)
{
	pDC->SetBkMode(TRANSPARENT);

	int y = rcClient.top;

	int nRowCount = pRows->GetCount();

	if (nRowCount > 0)
	{
		if (GetReportHeader()->GetNextVisibleColumn(-1, 1) == NULL)
			return;

		int nHeaderWidth = GetReportHeader()->GetWidth();

		for (int i = 0; i < nRowCount; i++)
		{
			CXTPReportRow* pRow = pRows->GetAt(i);

			if (y > rcClient.bottom)
				break;

			int nHeight = GetPaintManager()->GetRowHeight(pDC, pRow, nHeaderWidth);
			CRect rcRow(rcClient.left, y, rcClient.left + nHeaderWidth, y + nHeight);
						//y + pRow->GetHeight(pDC, nHeaderWidth)); //Fix for PreviewMode

			pRow->DrawFixed(pDC, rcRow, m_nLeftOffset, rcClient);

			y += rcRow.Height();
		}
	}
}

void CXTPReportControl::DrawFixedRecordsDivider(CDC* pDC, CRect& rcClient, BOOL bHeaderRows)
{
	GetPaintManager()->DrawFixedRowsDivider(pDC, rcClient, this, bHeaderRows, m_bVScrollBarVisible);
}

void CXTPReportControl::OnSelectionChanged()
{
	SetLastRqstEdit(-1, -1);

	//m_iLastRqstEditCol = -1;
	//m_iLastRqstEditRow = -1;

	SendNotifyMessage(XTP_NM_REPORT_SELCHANGED);

	if (m_pSelectedRows)
		m_pSelectedRows->SetChanged(FALSE);
}

BOOL CXTPReportControl::OnFocusChanging(CXTPReportRow* pNewRow, CXTPReportColumn* pNewCol)
{
	XTP_NM_REPORTREQUESTEDIT nm;
	::ZeroMemory(&nm, sizeof(nm));

	nm.bCancel = FALSE;
	nm.pRow = pNewRow ? pNewRow : GetFocusedRow();
	nm.pColumn = pNewCol ? pNewCol : GetFocusedColumn();
	nm.pItem = nm.pRow && nm.pColumn ? (nm.pRow->GetRecord() ? nm.pRow->GetRecord()->GetItem(nm.pColumn) : NULL) : NULL;

	SendNotifyMessage(XTP_NM_REPORT_FOCUS_CHANGING, (NMHDR*)&nm);

	return !nm.bCancel;
}

void CXTPReportControl::GetItemMetrics(XTP_REPORTRECORDITEM_DRAWARGS* pDrawArgs,
	XTP_REPORTRECORDITEM_METRICS* pMetrics)
{
	XTP_NM_REPORTITEMMETRICS nmData;
	nmData.pDrawArgs = pDrawArgs;
	nmData.pItemMetrics = pMetrics;

	SendNotifyMessage(XTP_NM_REPORT_GETITEMMETRICS, (NMHDR*) &nmData);
}

BOOL CXTPReportControl::SetFocusedColumn(CXTPReportColumn* pColumn)
{
	if (m_pFocusedColumn != pColumn)
	{
		if (m_bFocusSubItems && pColumn)
		{
			if (!OnFocusChanging(NULL, pColumn))
				return FALSE;
		}

		m_pFocusedColumn = pColumn;

		if (m_pFocusedColumn && m_bFocusSubItems)
		{
			CRect rc(m_pFocusedColumn->GetRect());

			if (rc.right >= m_rcReportArea.Width())
			{
				SetLeftOffset(m_nLeftOffset + min(rc.left, rc.right - m_rcReportArea.Width()));
			}
			else
			{
				int nFreezeColumnWidth = 0;
				if (m_nFreezeColumnsCount > 0)
				{
					int nFreezeColumnsCount = m_nFreezeColumnsCount;
					for (int i = 0; (i < m_pColumns->GetCount()) && (nFreezeColumnsCount > 0); i++)
					{
						CXTPReportColumn* pColumnCheck = m_pColumns->GetAt(i);

						if (pColumnCheck == m_pFocusedColumn)
						{
							nFreezeColumnWidth = 0;
							break;
						}

						if (pColumnCheck && pColumnCheck->IsVisible())
						{
							nFreezeColumnsCount--;
							nFreezeColumnWidth = pColumnCheck->GetRect().right;
						}
					}
				}

				if (rc.left - nFreezeColumnWidth <= 0 && m_nLeftOffset != 0)
				{
					SetLeftOffset(m_nLeftOffset + rc.left - nFreezeColumnWidth);
				}
			}
		}

		if (m_bFocusSubItems)
			OnSelectionChanged();
	}

	return TRUE;
}

BOOL CXTPReportControl::SetFocusedRow(CXTPReportRow* pRow, BOOL bControlKey)
{
	if (pRow)
	{
		int nFocusedRow = m_nFocusedRow != -1 ?
			m_nFocusedRow :
			m_nFocusedHeaderRow != -1 ? m_nFocusedHeaderRow : m_nFocusedFooterRow;
		int nFocusedRowType = m_nFocusedRow != -1 ?
			xtpRowTypeBody : m_nFocusedHeaderRow != -1 ? xtpRowTypeHeader : xtpRowTypeFooter;

		if (nFocusedRow != pRow->GetIndex() || pRow->GetType() != nFocusedRowType)
		{
			CMDTARGET_ADDREF(pRow);

			BOOL bApply = OnFocusChanging(pRow, NULL);

			if (pRow->m_dwRef == 1) // Removed
				bApply = FALSE;

			CMDTARGET_RELEASE(pRow);

			if (!bApply)
				return FALSE;
		}
	}

	EditItem(NULL);
	BeginUpdate();

	BOOL bEnableSelection = (!bControlKey || !m_bBlockSelection) && IsSelectionEnabled();
	if (pRow && pRow->GetType() == xtpRowTypeHeader)
		bEnableSelection &= IsHeaderRowsSelectionEnabled();
	if (pRow && pRow->GetType() == xtpRowTypeFooter)
		bEnableSelection &= IsFooterRowsSelectionEnabled();

	if (pRow)
	{
		if (bEnableSelection)
			m_pSelectedRows->Select(pRow);

		m_nFocusedRow = m_nFocusedHeaderRow = m_nFocusedFooterRow = -1;

		switch (pRow->GetType())
		{
			case xtpRowTypeBody: m_nFocusedRow = pRow->GetIndex(); break;
			case xtpRowTypeHeader: m_nFocusedHeaderRow = pRow->GetIndex(); break;
			case xtpRowTypeFooter: m_nFocusedFooterRow = pRow->GetIndex(); break;
		}

		GetNavigator()->SetMovePosition(pRow->GetType());

		EnsureVisible(pRow);
	}

	EndUpdate();

	if (m_pSelectedRows->IsChanged())
		OnSelectionChanged();

	return TRUE;
}

BOOL CXTPReportControl::SetFocusedRow(CXTPReportRow* pRow, BOOL bShiftKey, BOOL bControlKey)
{
	if (!pRow)
		return FALSE;

	int nFocusedRow = m_nFocusedRow != -1 ?
			m_nFocusedRow :
			m_nFocusedHeaderRow != -1 ? m_nFocusedHeaderRow : m_nFocusedFooterRow;
	int nFocusedRowType = m_nFocusedRow != -1 ?
			xtpRowTypeBody :
			m_nFocusedHeaderRow != -1 ? xtpRowTypeHeader : xtpRowTypeFooter;

	if (nFocusedRow != pRow->GetIndex() || pRow->GetType() != nFocusedRowType)
	{
		if (!OnFocusChanging(pRow, NULL))
			return FALSE;
	}

	EditItem(NULL);
	BeginUpdate();

	BOOL bEnableSelection = IsSelectionEnabled();
	if (pRow && pRow->GetType() == xtpRowTypeHeader)
		bEnableSelection &= IsHeaderRowsSelectionEnabled();
	if (pRow && pRow->GetType() == xtpRowTypeFooter)
		bEnableSelection &= IsFooterRowsSelectionEnabled();

	if (bEnableSelection)
	{
		if (m_bBlockSelection)
		{
			if (bShiftKey && nFocusedRow != -1)
			{
				int nSelRow = pRow->GetIndex();
				m_pSelectedRows->SelectBlock(nFocusedRow, nSelRow, bControlKey);
			}
			else if (!bControlKey)
			{
				m_pSelectedRows->Select(pRow);
			}
		}
		else
		{
			m_pSelectedRows->Select(pRow);
		}
	}

	m_nFocusedRow = m_nFocusedHeaderRow = m_nFocusedFooterRow = -1;
	switch (pRow->GetType())
	{
		case xtpRowTypeBody:    m_nFocusedRow       = pRow->GetIndex(); break;
		case xtpRowTypeHeader:  m_nFocusedHeaderRow = pRow->GetIndex(); break;
		case xtpRowTypeFooter:  m_nFocusedFooterRow = pRow->GetIndex(); break;
	}

	GetNavigator()->SetMovePosition(pRow->GetType());

	EnsureVisible(pRow);
	EndUpdate();

	if (m_pSelectedRows->IsChanged())
		OnSelectionChanged();

	return TRUE;
}

void CXTPReportControl::SetLeftOffset(int nOffset)
{
	if (nOffset < 0)
		nOffset = 0;

	if (nOffset == m_nLeftOffset)
		return;

	m_nLeftOffset = nOffset;

	if (!IsFullColumnScrolling())
		SetScrollPos(SB_HORZ, nOffset);

	AdjustScrollBars();
}

void CXTPReportControl::SetTopRow(int nIndex)
{
	if (nIndex == m_nTopRow)
		return;

	ASSERT(nIndex >= 0);

	if (IsIconView())
	{
		// Must get first row on the same line.
		while(nIndex % GetRowsPerLine())
		{
			nIndex--;
		}
	}

	if (nIndex < 0)
		nIndex = 0;

	m_nTopRow = nIndex;
	if (m_nLockUpdateCount == 0 && GetSafeHwnd())
	{
		SetScrollPos(SB_VERT, nIndex);
		AdjustScrollBars();
		m_nTopRowIdx = -1;
	}
	else
	{
		m_nTopRowIdx = nIndex;
	}
}

void CXTPReportControl::EnsureVisible(CXTPReportRow* pCheckRow)
{
	int nCheckIndex = pCheckRow ? pCheckRow->GetIndex() : -1;

	if (nCheckIndex == -1 || !pCheckRow->m_bVisible || nCheckIndex >= m_pRows->GetCount())
		return;

	if (m_rcReportArea.Height() <= 0)
	{
		m_nEnsureVisibleRowIdx = nCheckIndex;
		return;
	}

	if (nCheckIndex < m_nTopRow)
	{
		SetTopRow(nCheckIndex);
		return;
	}

	CClientDC dc (this);

	int top = m_rcReportArea.top;
	int nHeaderWidth = GetReportHeader()->GetWidth();

	for (int i = m_nTopRow; i < m_pRows->GetCount(); i++)
	{
		CXTPReportRow* pRow = m_pRows->GetAt(i);
		ASSERT(pRow);
		if (!pRow)
			continue;

		int nRowHeight = pRow->GetHeight(&dc, nHeaderWidth);

		if (top + nRowHeight > m_rcReportArea.bottom)
			break;

		if (i == nCheckIndex)
			return;

		top += nRowHeight;
	}

	int nHeight = m_rcReportArea.Height();
	for (top = nCheckIndex; top >= 0; top--)
	{
		CXTPReportRow* pRow = m_pRows->GetAt(top);
		int nRowHeight = pRow->GetHeight(&dc, nHeaderWidth);

		if (nHeight - nRowHeight < 0)
		{
			if (top != nCheckIndex)
			{
// Must go to the next equivalency class, to truly get the next row in icon view.  SetTopRow will correctly set the top row to the first in this row.
				top += IsIconView() ? GetRowsPerLine() : 1;
			}
			break;
		}

		nHeight -= nRowHeight;
	}
	SetTopRow(top);
	RedrawControl();
}

void CXTPReportControl::EnsureVisible(CXTPReportColumn* pCheckColumn)
{
	int nCheckIndex = pCheckColumn ? pCheckColumn->GetIndex() : -1;

	if (nCheckIndex == -1 || !pCheckColumn->m_bVisible || nCheckIndex >= m_pColumns->GetCount())
		return;

	if (m_rcReportArea.Height() <= 0)
	{
		m_nEnsureVisibleRowIdx = nCheckIndex;
		return;
	}

	CRect rc(pCheckColumn->GetRect());

	if (rc.right >= m_rcReportArea.Width())
	{
		SetLeftOffset(m_nLeftOffset + min(rc.left, rc.right - m_rcReportArea.Width()));
	}
	else
	{
		int nFreezeColumnWidth = 0;
		if (m_nFreezeColumnsCount > 0)
		{
			int nFreezeColumnsCount = m_nFreezeColumnsCount;
			for (int i = 0; (i < m_pColumns->GetCount()) && (nFreezeColumnsCount > 0); i++)
			{
				CXTPReportColumn* pColumn = m_pColumns->GetAt(i);

				if (pColumn == pCheckColumn)
				{
					nFreezeColumnWidth = 0;
					break;
				}

				if (pColumn && pColumn->IsVisible())
				{
					nFreezeColumnsCount--;
					nFreezeColumnWidth = pColumn->GetRect().right;
				}
			}
		}

		if (rc.left - nFreezeColumnWidth <= 0 && m_nLeftOffset != 0)
		{
			SetLeftOffset(m_nLeftOffset + rc.left - nFreezeColumnWidth);
		}
	}
}

CXTPReportRow* CXTPReportControl::HitTest(CPoint pt, BOOL bConsiderFastDeselectMode) const
{
	if (m_rcReportArea.PtInRect(pt))
	{
		for (int i = 0; i < m_arrScreenRows.GetCount(); i++)
		{
			CXTPReportRow* pRow = m_arrScreenRows.GetAt(i);
			CRect rc = pRow->GetRect();
			if (rc.PtInRect(pt))
			{
				if (bConsiderFastDeselectMode && m_bFastDeselectMode)
				{
					if (m_bIconView)
					{
						rc.DeflateRect(5, 5);
						if (!rc.PtInRect(pt))
							pRow = NULL;
					}
				}
				return pRow;
			}
		}
	}

	// header records
	if (m_rcHeaderRecordsArea.PtInRect(pt))
	{
		for (int i = 0; i < m_pHeaderRows->GetCount(); i++)
		{
			CXTPReportRow* pRow = m_pHeaderRows->GetAt(i);

			CRect rc = pRow->GetRect();

			if (rc.PtInRect(pt))
				return pRow;
		}
	}

	// footer records
	if (m_rcFooterRecordsArea.PtInRect(pt))
	{
		for (int i = 0; i < m_pFooterRows->GetCount(); i++)
		{
			CXTPReportRow* pRow = m_pFooterRows->GetAt(i);

			CRect rc = pRow->GetRect();

			if (rc.PtInRect(pt))
				return pRow;
		}
	}

	return NULL;
}

int CXTPReportControl::GetReportAreaRows(int nStartRow, BOOL bMoveDown)
{
	int nDirection = bMoveDown ? +1 : -1;

	int top = m_rcReportArea.top;

	CClientDC dc(this);
	int nHeaderWidth = GetReportHeader()->GetWidth();

	int i = nStartRow;

	if (IsIconView())
	{
	// Find the first row, to account for the height of the row we are in.
		while(i % GetRowsPerLine() != 0)
		{
			i--;
		}
	}

	for (; (i < m_pRows->GetCount() && i >= 0); i += nDirection)
	{
		CXTPReportRow* pRow = m_pRows->GetAt(i);
		ASSERT(pRow);
		if (!pRow)
			continue;

		int rowHeight = pRow->GetHeight(&dc, nHeaderWidth);

		if (top + rowHeight > m_rcReportArea.bottom)
			return bMoveDown ? i - nStartRow - 1 : nStartRow - i - 1;

		top += rowHeight;
	}

	//return bMoveDown ? m_pRows->GetCount() - nStartRow : nStartRow;
	return bMoveDown ? m_pRows->GetCount() - nStartRow - 1: nStartRow; //-this is better way!
}

void CXTPReportControl::AdjustLayout()
{
	if (GetSafeHwnd() == 0)
		return;

	if (m_bAdjustLayoutRunning) //guard to prevent the recursion similar to OnSize function
		return;

	m_bAdjustLayoutRunning = TRUE;

	CXTPClientRect rc(this);

	int nHeaderWidth = m_rcHeaderArea.Width();

	CXTPReportHeader* pHeader = GetReportHeader();
	int nGroupByHeight = (m_bGroupByEnabled && pHeader) ? pHeader->GetGroupByHeight() : 0;
	m_rcGroupByArea.SetRect(0, 0, rc.Width(), nGroupByHeight);

	int nHeaderHeight = 0;
	int nFooterHeight = 0;
	int nHeaderRecordsHeight = 0;
	int nFooterRecordsHeight = 0;

	if (m_bHeaderVisible)
	{
		CWindowDC dc (this);
		nHeaderHeight = GetPaintManager()->GetHeaderHeight(this, &dc);
	}

	if (m_bFooterVisible)
	{
		CWindowDC dc (this);
		nFooterHeight = GetPaintManager()->GetFooterHeight(this, &dc);
	}

	// header records divider
	int nHeaderDividerHeight = GetHeaderRowsDividerHeight();
	int nFooterDividerHeight = GetFooterRowsDividerHeight();

	// header records height
	if (m_bHeaderRecordsVisible)
		nHeaderRecordsHeight = GetRowsHeight(m_pHeaderRows, rc.Width());
	else
		nHeaderDividerHeight = 0;

	// footer records height
	if (m_bFooterRecordsVisible)
		nFooterRecordsHeight = GetRowsHeight(m_pFooterRows, rc.Width());

	m_rcHeaderArea.SetRect(0, m_rcGroupByArea.bottom, rc.Width(), m_rcGroupByArea.bottom + nHeaderHeight);

	m_rcHeaderRecordsArea.SetRect(0, m_rcHeaderArea.bottom, rc.Width(), m_rcHeaderArea.bottom + nHeaderRecordsHeight);
	m_rcHeaderRecordsDividerArea.SetRect(0, m_rcHeaderRecordsArea.bottom, rc.Width(), m_rcHeaderRecordsArea.bottom + nHeaderDividerHeight);

	int nFreeHeight = rc.Height() - nGroupByHeight - nHeaderHeight - nFooterHeight - nHeaderRecordsHeight - nFooterRecordsHeight - nHeaderDividerHeight - nFooterDividerHeight;
	BOOL bPinned = FALSE;

	if (m_bFooterRecordsVisible && m_bPinFooterRecords)
	{
		int nBodyRowsHeight = GetRowsHeight(m_pRows, rc.Width(), nFreeHeight);

		if (nBodyRowsHeight < nFreeHeight)
		{
			// footer records immediately after body records
			m_rcFooterRecordsArea.SetRect(0, m_rcHeaderRecordsArea.bottom + nHeaderDividerHeight + nFooterDividerHeight + nBodyRowsHeight, rc.Width(),
				m_rcHeaderRecordsArea.bottom + nHeaderDividerHeight + nFooterDividerHeight + nBodyRowsHeight + nFooterRecordsHeight);

			m_rcFooterRecordsDividerArea.SetRect(0,m_rcFooterRecordsArea.top - nFooterDividerHeight,rc.Width(), m_rcFooterRecordsArea.top);
			bPinned = TRUE;
		}
	}

	if (!bPinned)
	{
		if (rc.Height() - nFooterHeight - nFooterRecordsHeight > m_rcHeaderRecordsArea.bottom + nHeaderDividerHeight)
		{
			// there can be empty space between body rows and footer records rows
			m_rcFooterRecordsArea.SetRect(0, rc.Height() - nFooterHeight - nFooterRecordsHeight, rc.Width(), rc.Height() - nFooterHeight);
		}
		else
		{
			// no place for body records (between header and footer records)
			m_rcFooterRecordsArea.SetRect(0, m_rcHeaderRecordsArea.bottom + nHeaderDividerHeight, rc.Width(), m_rcHeaderRecordsArea.bottom + nFooterRecordsHeight + nHeaderDividerHeight);
		}

		m_rcFooterRecordsDividerArea.SetRect(0,m_rcFooterRecordsArea.top - nFooterDividerHeight -1,rc.Width(), m_rcFooterRecordsArea.top);
	}

	m_rcFooterArea.SetRect(0, rc.Height() - nFooterHeight, rc.Width(), rc.Height());
	m_rcReportArea.SetRect(0, m_rcHeaderRecordsArea.bottom + nHeaderDividerHeight, rc.Width(), m_rcFooterRecordsArea.top - nFooterDividerHeight);

	if (nHeaderWidth != m_rcHeaderArea.Width() && pHeader)
		pHeader->AdjustColumnsWidth(m_rcHeaderArea.Width());

	m_bAdjustLayoutRunning = FALSE;
}

CScrollBar* CXTPReportControl::GetScrollBarCtrl(int nBar) const
{
	if (DYNAMIC_DOWNCAST(CView, GetParent()))
		return GetParent()->GetScrollBarCtrl(nBar);
	//else if (DYNAMIC_DOWNCAST(CDialog, GetParent()))  //<<>>
	//  return GetParent()->GetScrollBarCtrl(nBar);     //<<>>

	return 0;
}

void CXTPReportControl::AdjustScrollBars()
{
	if (GetSafeHwnd() == 0)
		return;

	EditItem(NULL);

	int nHeight = m_rcReportArea.Height();

	if (nHeight <= 0)
		return;

	if (m_bAdjustScrollBars)
		return;

	m_bAdjustScrollBars = TRUE;

	BeginUpdate();

	int nCount = m_pRows->GetCount() - 1;
	int nLinesCount = nCount;
	int nHeaderWidth = GetReportHeader()->GetWidth();

	{
		CClientDC dc(this);

		for (; nLinesCount >= 0; nLinesCount--)
		{

			nHeight -= m_pRows->GetAt(nLinesCount)->GetHeight(&dc, nHeaderWidth);

			if (nHeight < 0)
			{
				if (nLinesCount != nCount)
					nLinesCount += IsIconView() ? GetRowsPerLine() : 1; // Want the next row, in icon view, that is an addition of an entire row.

				break;
			}
		}
	}

	nLinesCount = max(nLinesCount, 0);

	if (m_nTopRow > nLinesCount)
	{
		if (IsIconView())
		{
			while(nLinesCount % GetRowsPerLine())
			{
			// Get beginning of line
				nLinesCount--;
			}
		}
		m_nTopRow = nLinesCount;
		SetScrollPos(SB_VERT, m_nTopRow);
	}

	BOOL bEnabled = nLinesCount > 0;

	if (bEnabled)
	{
		SCROLLINFO  si;
		si.cbSize = sizeof(SCROLLINFO);
		si.nPage = nCount - nLinesCount + 1;
		si.nMax = nCount;
		si.nMin = 0;
		si.fMask = SIF_PAGE | SIF_RANGE;
		SetScrollInfo(SB_VERT, &si);
	}

	EnableScrollBarCtrl(SB_VERT, bEnabled);

	::EnableScrollBar(m_hWnd, SB_VERT, (bEnabled && IsWindowEnabled()) ? ESB_ENABLE_BOTH : ESB_DISABLE_BOTH);

	m_bVScrollBarVisible = bEnabled;

	if (!GetReportHeader()->m_bAutoColumnSizing)
	{
		if (IsFullColumnScrolling())
		{
			CXTPReportColumn *pPrev = NULL, *pCurr = NULL, *pNext = NULL;
			int nScrollPos, nScrollMax;
			GetReportHeader()->GetFulColScrollInfo(pPrev, pCurr, pNext, nScrollPos, nScrollMax);

			bEnabled = nScrollMax > 1;

			if (bEnabled)
			{
				SCROLLINFO  si ;
				si.cbSize = sizeof(SCROLLINFO);
				si.nPage = 1;
				si.nMax = nScrollMax - 1;
				si.nMin = 0;
				si.nPos = nScrollPos;
				si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
				SetScrollInfo(SB_HORZ, &si);
			}
			else
			{
				m_nLeftOffset = 0;
				SetScrollPos(SB_HORZ, m_nLeftOffset);
			}
		}
		else
		{
			int nScroll = 0;
			if (m_bVScrollBarVisible)
				nScroll = GetSystemMetrics(SM_CXVSCROLL);

			nHeaderWidth = max(GetReportHeader()->GetWidth() - m_rcReportArea.Width(), 0);

			if (m_nLeftOffset > nHeaderWidth)
			{
				m_nLeftOffset = nHeaderWidth;
				SetScrollPos(SB_HORZ, m_nLeftOffset);
			}

			bEnabled = nHeaderWidth > 0;

			if (bEnabled)
			{
				SCROLLINFO  si;
				si.cbSize = sizeof(SCROLLINFO);
				si.nPage = m_rcReportArea.Width();
				si.nMax = si.nPage + nHeaderWidth;
				si.nMin = 0;
				si.fMask = SIF_PAGE | SIF_RANGE;
				SetScrollInfo(SB_HORZ, &si);
			}
		}

		EnableScrollBarCtrl(SB_HORZ, bEnabled);
		m_bHScrollBarVisible = bEnabled;
		::EnableScrollBar(m_hWnd, SB_HORZ, (bEnabled && IsWindowEnabled()) ? ESB_ENABLE_BOTH : ESB_DISABLE_BOTH);
	}
	else
	{
		nHeaderWidth = GetReportHeader()->ForceLayoutAdjustment();

		int nScroll = 0;
		if (m_bVScrollBarVisible)
			nScroll = GetSystemMetrics(SM_CXVSCROLL);

		if (m_nLeftOffset > nHeaderWidth)
		{
			m_nLeftOffset = nHeaderWidth;
			SetScrollPos(SB_HORZ, m_nLeftOffset);
		}

		bEnabled = nHeaderWidth > 0;

		if (bEnabled)
		{
			SCROLLINFO  si;
			si.cbSize = sizeof(SCROLLINFO);
			si.nPage = m_rcReportArea.Width();
			si.nMax = si.nPage + nHeaderWidth;
			si.nMin = 0;
			si.fMask = SIF_PAGE | SIF_RANGE;
			SetScrollInfo(SB_HORZ, &si);
		}

		EnableScrollBarCtrl(SB_HORZ, bEnabled);
		m_bHScrollBarVisible = bEnabled;
		::EnableScrollBar(m_hWnd, SB_HORZ, (bEnabled && IsWindowEnabled()) ? ESB_ENABLE_BOTH : ESB_DISABLE_BOTH);
	}

	AdjustLayout();

	EndUpdate();

	m_bAdjustScrollBars = FALSE;

}

void CXTPReportControl::SetFullColumnScrolling(BOOL bSet)
{
	m_bFullColumnScrolling = bSet;

	if (m_hWnd)
	{
		CXTPReportControl::OnHScroll(SB_TOP, 0, NULL);

		RedrawControl();
		UpdateWindow();

		AdjustScrollBars();
	}
}

BOOL CXTPReportControl::SetWatermarkBitmap(HBITMAP hBitmap, BYTE Transparency)
{
	m_WatermarkTransparency = Transparency;

	// remove old watermark bitmap if one is
	if (m_hbmpWatermark)
	{
		::DeleteObject(m_hbmpWatermark);
		m_hbmpWatermark = NULL;
	}

	if (hBitmap)
	{
		// add new watermark bitmap

		BITMAP bmSrc;
		if (!CBitmap::FromHandle(hBitmap)->GetBitmap(&bmSrc))
			return FALSE;

		BOOL bAlphaBitmap = bmSrc.bmBitsPixel == 32;

		// create an alpha bitmap if the bitmap supplied isn't an alpha one
		if (!bAlphaBitmap)
		{
			m_hbmpWatermark = ::CreateBitmap(bmSrc.bmWidth, bmSrc.bmHeight, bmSrc.bmPlanes, 32, NULL);

			ASSERT(m_hbmpWatermark);
			if (!m_hbmpWatermark)
				return FALSE;

			CXTPCompatibleDC memWatermarkDC(NULL, m_hbmpWatermark);
			CXTPCompatibleDC memScrDC(NULL, hBitmap);

			memWatermarkDC.BitBlt(0, 0, bmSrc.bmWidth, bmSrc.bmHeight, &memScrDC, 0, 0, SRCCOPY);
		}
		else
		{
			m_hbmpWatermark = CXTPImageManagerIcon::CopyAlphaBitmap(hBitmap);
		}

		if (!m_hbmpWatermark)
			return FALSE;

		if (!CBitmap::FromHandle(m_hbmpWatermark)->GetBitmap(&m_bmWatermark))
		{
			::DeleteObject(m_hbmpWatermark);
			m_hbmpWatermark = NULL;
			return FALSE;
		}
	}

	return TRUE;
}

BOOL CXTPReportControl::SetWatermarkBitmap(LPCTSTR szPath, BYTE Transparency)
{
	m_WatermarkTransparency = Transparency;

	// if path is empty, remove watermark bitmap
	if (_tcslen(szPath) == 0)
	{
		::DeleteObject(m_hbmpWatermark);
		m_hbmpWatermark = NULL;
	}
	else
	{
		// remove old watermark bitmap if one is
		if (m_hbmpWatermark)
		{
			::DeleteObject(m_hbmpWatermark);
			m_hbmpWatermark = NULL;
		}
		// add new watermark bitmap
		BOOL bAlphaBitmap = FALSE;
		m_hbmpWatermark = CXTPImageManagerIcon::LoadBitmapFromFile(szPath, &bAlphaBitmap);
		if (!m_hbmpWatermark)
			return FALSE;
		if (!CBitmap::FromHandle(m_hbmpWatermark)->GetBitmap(&m_bmWatermark))
		{
			::DeleteObject(m_hbmpWatermark);
			m_hbmpWatermark = NULL;
			return FALSE;
		}
		// create an alpha bitmap if the bitmap supplied isn't an alpha one
		if (!bAlphaBitmap)
		{
			CXTPCompatibleDC memWatermarkDC(NULL, m_hbmpWatermark);
			CXTPImageManagerIcon::DrawAlphaBitmap(&memWatermarkDC, m_hbmpWatermark, CPoint(0, 0), CSize(m_bmWatermark.bmWidth, m_bmWatermark.bmHeight));
		}
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
// CXTPReportControl message handlers

void CXTPReportControl::OnPaint()
{
	CPaintDC dc(this);      // device context for painting
	CXTPClientRect rc(this);

	// ensure visible row and column if first started
	if (m_nEnsureVisibleRowIdx >= 0)
	{
		EnsureVisible(GetRows()->GetAt(m_nEnsureVisibleRowIdx));
		m_nEnsureVisibleRowIdx = -1;
	}

	if (m_nEnsureVisibleColumnIdx >= 0)
	{
		EnsureVisible(GetColumns()->GetAt(m_nEnsureVisibleColumnIdx));
		m_nEnsureVisibleColumnIdx = -1;
	}

	if (m_nTopRowIdx >= 0)
	{
		SetScrollPos(SB_VERT, m_nTopRowIdx);
		AdjustScrollBars();
		m_nTopRowIdx = -1;
	}
	// start counting drawing time
#ifdef XTP_DEBUG
	LARGE_INTEGER iStartCount;
	QueryPerformanceCounter(&iStartCount);
#endif

	if (m_nLockUpdateCount == 0 && (IsChanged() || m_bmpCache.GetSafeHandle() == 0))
	{
		CDC memDC;
		memDC.CreateCompatibleDC(&dc);

		m_bmpCache.DeleteObject();

		if (!m_bmpCache.CreateCompatibleBitmap(&dc, rc.Width(), rc.Height()))
		{
			// Failed to create Cache bitmap

			OnDraw(&dc);

			SetChanged(FALSE);
			return;
		}

		CBitmap* pOldBitmap = memDC.SelectObject(&m_bmpCache);

		OnDraw(&memDC);

		// update flag
		SetChanged(FALSE);

		if (!IsWindowEnabled() && GetPaintManager()->m_bGrayIfDisable)
		{
			XTPImageManager()->DisableBitmap(memDC, rc, RGB(250, 250, 250), RGB(128, 128, 128));
		}

		if (m_hbmpWatermark)
		{
			CRect rcDst(GetReportRectangle());
			if (IsHeaderRowsVisible())
				rcDst.top = m_rcHeaderArea.top;
			if (IsFooterRowsVisible())
				rcDst.bottom = m_rcFooterArea.bottom;
			CRect rcAll(rcDst);
			CRect rcSrc(0, 0, m_bmWatermark.bmWidth, m_bmWatermark.bmHeight);
			// no stretch
			if (!(GetWatermarkAlignment() & xtpReportWatermarkStretch))
			{
				if (rcSrc.Width() > rcDst.Width())
					rcSrc.right = rcSrc.left + rcDst.Width();
				else
					rcDst.right = rcDst.left + rcSrc.Width();
				if (rcSrc.Height() > rcDst.Height())
					rcSrc.bottom = rcSrc.top + rcDst.Height();
				else
					rcDst.bottom = rcDst.top + rcSrc.Height();
			}
			// enlarge only
			if ((GetWatermarkAlignment() & xtpReportWatermarkStretch) && (GetWatermarkAlignment() & xtpReportWatermarkEnlargeOnly))
			{
				if (rcSrc.Width() > rcDst.Width())
					rcSrc.right = rcSrc.left + rcDst.Width();
				if (rcSrc.Height() > rcDst.Height())
					rcSrc.bottom = rcSrc.top + rcDst.Height();
			}
			// shrink only
			if ((GetWatermarkAlignment() & xtpReportWatermarkStretch) && (GetWatermarkAlignment() & xtpReportWatermarkShrinkOnly))
			{
				if (rcSrc.Width() < rcDst.Width())
					rcDst.right = rcDst.left + rcSrc.Width();
				if (rcSrc.Height() < rcDst.Height())
					rcDst.bottom = rcDst.top + rcSrc.Height();
			}
			// preserve aspect ratio
			if ((GetWatermarkAlignment() & xtpReportWatermarkStretch) && (GetWatermarkAlignment() & xtpReportWatermarkPreserveRatio))
			{
				if (rcDst.Width() > (rcDst.Height() * rcSrc.Width() / rcSrc.Height()))
					rcDst.right = rcDst.left + rcDst.Height() * rcSrc.Width() / rcSrc.Height();
				if (rcDst.Height() > (rcDst.Width() * rcSrc.Height() / rcSrc.Width()))
					rcDst.bottom = rcDst.top + rcDst.Width() * rcSrc.Height() / rcSrc.Width();
			}

			// horizontal
			switch (GetWatermarkAlignment() & xtpReportWatermarkHmask)
			{
				// center
				case xtpReportWatermarkCenter :
					rcDst.OffsetRect((rcAll.Width() - rcDst.Width()) / 2, 0);
					break;
				// right
				case xtpReportWatermarkRight :
					rcDst.OffsetRect(rcAll.Width() - rcDst.Width(), 0);
					break;
				// left
				default :
					break;
			}
			// vertical
			switch (GetWatermarkAlignment() & xtpReportWatermarkVmask)
			{
				// center
				case xtpReportWatermarkVCenter :
					rcDst.OffsetRect(0, (rcAll.Height() - rcDst.Height()) / 2);
					break;
				// bottom
				case xtpReportWatermarkBottom:
					rcDst.OffsetRect(0, rcAll.Height() - rcDst.Height());
					break;
				// top
				default :
					break;
			}

			CXTPCompatibleDC memWatermarkDC(&dc, m_hbmpWatermark);

			COLORREF clrBk = GetPaintManager()->m_clrControlBack;
			CRect rcReport00(0, 0, rc.Width(), rc.Height());

			CBitmap bmpTmp;
			bmpTmp.CreateCompatibleBitmap(&dc, rc.Width(), rc.Height());

			CXTPCompatibleDC memDCtmp(&dc, &bmpTmp);

			memDCtmp.FillSolidRect(&rcReport00, clrBk);

			if (GetWatermarkAlignment() & xtpReportWatermarkStretch)
			{
				CBitmap bmpWatermarkTmp;
				bmpWatermarkTmp.CreateCompatibleBitmap(&dc, rc.Width(), rc.Height());
				CXTPCompatibleDC memWatermarkDCtmp(&dc, &bmpWatermarkTmp);
				memWatermarkDCtmp.FillSolidRect(&rc, RGB(255, 255, 255));
				memWatermarkDCtmp.SetStretchBltMode(HALFTONE);
				memWatermarkDCtmp.StretchBlt(rcDst.left, rcDst.top, rcDst.Width(), rcDst.Height(), &memWatermarkDC, 0, 0, rcSrc.Width(), rcSrc.Height(), SRCCOPY);

				XTPImageManager()->AlphaBlend2(memDCtmp, rc, memWatermarkDCtmp, rc, m_WatermarkTransparency);
			}
			else
			{
				XTPImageManager()->AlphaBlend2(memDCtmp, rcDst, memWatermarkDC, rcSrc, m_WatermarkTransparency);
			}

			XTPImageManager()->TransparentBlt(memDCtmp, rcReport00, memDC, rcReport00, clrBk);

			memDC.BitBlt(0, 0, rc.right, rc.bottom, &memDCtmp, 0, 0, SRCCOPY);
		}

		dc.BitBlt(0, 0, rc.right, rc.bottom, &memDC, 0, 0, SRCCOPY);

		memDC.SelectObject(pOldBitmap);
	}
	else
	{
		CXTPCompatibleDC memDC(&dc, &m_bmpCache);
		dc.BitBlt(0, 0, rc.right, rc.bottom, &memDC, 0, 0, SRCCOPY);
	}

	// count drawing time
#ifdef XTP_DEBUG
	LARGE_INTEGER iEndCount;
	QueryPerformanceCounter(&iEndCount);
	XTP_TRACE(_T("Draw counter ticks: %d\n"), iEndCount.LowPart-iStartCount.LowPart);
#endif

}

void CXTPReportControl::DrawWatermark(CDC* pDC, CRect RC)
{
	if (m_hbmpWatermark)
	{
		CRect rc(0, 0, RC.Width(), RC.Height());
		CDC memDC;
		memDC.CreateCompatibleDC(pDC);

		CBitmap* pOldBitmap = memDC.SelectObject(&m_bmpCache);

		CRect rcDst(rc);
		if (IsHeaderRowsVisible())
			rcDst.top = m_rcHeaderArea.top;
		if (IsFooterRowsVisible())
			rcDst.bottom = m_rcFooterArea.bottom;
		CRect rcAll(rcDst);
		CRect rcSrc(0, 0, m_bmWatermark.bmWidth, m_bmWatermark.bmHeight);
		// no stretch
		if (!(GetWatermarkAlignment() & xtpReportWatermarkStretch))
		{
			if (rcSrc.Width() > rcDst.Width())
				rcSrc.right = rcSrc.left + rcDst.Width();
			else
				rcDst.right = rcDst.left + rcSrc.Width();
			if (rcSrc.Height() > rcDst.Height())
				rcSrc.bottom = rcSrc.top + rcDst.Height();
			else
				rcDst.bottom = rcDst.top + rcSrc.Height();
		}
		// enlarge only
		if ((GetWatermarkAlignment() & xtpReportWatermarkStretch) && (GetWatermarkAlignment() & xtpReportWatermarkEnlargeOnly))
		{
			if (rcSrc.Width() > rcDst.Width())
				rcSrc.right = rcSrc.left + rcDst.Width();
			if (rcSrc.Height() > rcDst.Height())
				rcSrc.bottom = rcSrc.top + rcDst.Height();
		}
		// shrink only
		if ((GetWatermarkAlignment() & xtpReportWatermarkStretch) && (GetWatermarkAlignment() & xtpReportWatermarkShrinkOnly))
		{
			if (rcSrc.Width() < rcDst.Width())
				rcDst.right = rcDst.left + rcSrc.Width();
			if (rcSrc.Height() < rcDst.Height())
				rcDst.bottom = rcDst.top + rcSrc.Height();
		}
		// preserve aspect ratio
		if ((GetWatermarkAlignment() & xtpReportWatermarkStretch) && (GetWatermarkAlignment() & xtpReportWatermarkPreserveRatio))
		{
			if (rcDst.Width() > (rcDst.Height() * rcSrc.Width() / rcSrc.Height()))
				rcDst.right = rcDst.left + rcDst.Height() * rcSrc.Width() / rcSrc.Height();
			if (rcDst.Height() > (rcDst.Width() * rcSrc.Height() / rcSrc.Width()))
				rcDst.bottom = rcDst.top + rcDst.Width() * rcSrc.Height() / rcSrc.Width();
		}
		// horizontal
		switch (GetWatermarkAlignment() & xtpReportWatermarkHmask)
		{
			// center
			case xtpReportWatermarkCenter :
				rcDst.OffsetRect((rcAll.Width() - rcDst.Width()) / 2, 0);
				break;
			// right
			case xtpReportWatermarkRight :
				rcDst.OffsetRect(rcAll.Width() - rcDst.Width(), 0);
				break;
			// left
			default :
				break;
		}
		// vertical
		switch (GetWatermarkAlignment() & xtpReportWatermarkVmask)
		{
			// center
			case xtpReportWatermarkVCenter :
				rcDst.OffsetRect(0, (rcAll.Height() - rcDst.Height()) / 2);
				break;
			// bottom
			case xtpReportWatermarkBottom:
				rcDst.OffsetRect(0, rcAll.Height() - rcDst.Height());
				break;
			// top
			default :
				break;
		}

		CXTPCompatibleDC memWatermarkDC(pDC, m_hbmpWatermark);
		COLORREF clrBk = GetPaintManager()->m_clrControlBack;
		CRect rcReport00(0, 0, rc.Width(), rc.Height());
		CBitmap bmpTmp;
		bmpTmp.CreateCompatibleBitmap(pDC, rc.Width(), rc.Height());
		CXTPCompatibleDC memDCtmp(pDC, &bmpTmp);
		memDCtmp.FillSolidRect(&rcReport00, clrBk);

		if (GetWatermarkAlignment() & xtpReportWatermarkStretch)
		{
			CBitmap bmpWatermarkTmp;
			bmpWatermarkTmp.CreateCompatibleBitmap(pDC, rc.Width(), rc.Height());
			CXTPCompatibleDC memWatermarkDCtmp(pDC, &bmpWatermarkTmp);
			memWatermarkDCtmp.FillSolidRect(&rc, RGB(255, 255, 255));
			memWatermarkDCtmp.SetStretchBltMode(HALFTONE);
			memWatermarkDCtmp.StretchBlt(rcDst.left, rcDst.top, rcDst.Width(), rcDst.Height(), &memWatermarkDC, 0, 0, rcSrc.Width(), rcSrc.Height(), SRCCOPY);

			XTPImageManager()->AlphaBlend2(memDCtmp, rc, memWatermarkDCtmp, rc, m_WatermarkTransparency);
		}
		else
		{
			XTPImageManager()->AlphaBlend2(memDCtmp, rcDst, memWatermarkDC, rcSrc, m_WatermarkTransparency);
		}

		XTPImageManager()->TransparentBlt(memDCtmp, rcReport00, memDC, rcReport00, clrBk);

		memDC.BitBlt(0, 0, rc.Width(), rc.Height(), &memDCtmp, 0, 0, SRCCOPY);
		pDC->BitBlt(RC.left, RC.top - m_rcHeaderArea.Height(), rc.Width(), rc.Height(), &memDC, 0, 0, SRCCOPY);

		memDC.SelectObject(pOldBitmap);
	}
}

LRESULT CXTPReportControl::OnPrintClient(WPARAM wParam, LPARAM /*lParam*/)
{
	CDC* pDC = CDC::FromHandle((HDC)wParam);
	if (pDC)
	{
		if (m_bmpCache.GetSafeHandle() == 0)
			OnDraw(pDC);
		else
		{
			CXTPCompatibleDC memDC(pDC, &m_bmpCache);
			CXTPClientRect rc(this);
			pDC->BitBlt(0, 0, rc.right, rc.bottom, &memDC, 0, 0, SRCCOPY);
		}
	}

	return TRUE;
}

BOOL CXTPReportControl::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;    // Don't erase the background.
}

void CXTPReportControl::OnDraw(CDC* pDC)
{
	if (GetExStyle() & WS_EX_RTLREADING)
		pDC->SetTextAlign(TA_RTLREADING);

	// draw new image
	pDC->FillSolidRect(CXTPClientRect(this), GetPaintManager()->GetControlBackColor(this));

	CXTPReportHeader* pHeader = GetReportHeader();

	CRect rcHeader = m_rcHeaderArea;
	rcHeader.right = rcHeader.left + pHeader->GetWidth();

	if (pHeader)
	{
		pHeader->Draw(pDC, rcHeader, m_nLeftOffset); // draw header
		pHeader->DrawGroupByControl(pDC, m_rcGroupByArea); // draw group by box
	}

	DrawRows(pDC, m_rcReportArea);

	if (m_bHeaderRecordsVisible && m_pHeaderRows->GetCount() > 0)
	{
		DrawFixedRows(pDC, m_rcHeaderRecordsArea, m_pHeaderRows);
		DrawFixedRecordsDivider(pDC, m_rcHeaderRecordsDividerArea, TRUE);
	}

	if (m_bFooterRecordsVisible && m_pFooterRows->GetCount() > 0)
	{
		DrawFixedRows(pDC, m_rcFooterRecordsArea, m_pFooterRows);
		DrawFixedRecordsDivider(pDC, m_rcFooterRecordsDividerArea, FALSE);
	}

	CRect rcFooter = m_rcFooterArea;
	if (rcFooter.Height() > 0)
		pHeader->DrawFooter(pDC, rcFooter, m_nLeftOffset);

	BOOL bForce(FALSE);
	BOOL bDrawBetween = FALSE;
	int yForce(-1);
	if (m_bDragMode && GetPaintManager() && !IsIconView())
	{
		bForce = GetPaintManager()->m_bForceShowDropMarker;
		CPoint ptMouse;
		if (::GetCursorPos(&ptMouse))
		{
			ScreenToClient(&ptMouse);

			EnsureStartAutoVertScroll();
//          DoAutoVertScrollIfNeed(m_pointDrag, ptMouse);

			if (m_dwDropMarkerFlags & xtpReportDropBetween)
			{
				CXTPReportRow* pHitRow = HitTest(ptMouse);
				if (pHitRow && pHitRow->GetRecord())
				{
					if(bForce
						||  !(m_dwDropMarkerFlags & xtpReportDropSelect)
						||  (m_pSelectedRows->GetCount() == 0
						&& (ptMouse.y <= (pHitRow->GetRect().top + 4)
						|| ptMouse.y >= (pHitRow->GetRect().bottom - 4))))
					{
						bDrawBetween = TRUE;
						BOOL bAbove = TRUE;
						if (pHitRow->GetRect().CenterPoint().y < ptMouse.y)
							bAbove = FALSE;
						if (bAbove)
							yForce = pHitRow->GetRect().top - 1;
						else
							yForce = pHitRow->GetRect().bottom - 1;
					}
				}
			}
		}
	}

	if (bDrawBetween)
	{
		if (m_nDropPos != -1  && GetPaintManager()->m_bUseDropMarker)
			DrawDropMarker(pDC);
		else if (bForce && yForce > 0 && GetPaintManager()->m_bUseDropMarker)
			DrawExtDropMarker(pDC, yForce);
	}

	// update flag
	SetChanged(FALSE);
}

void CXTPReportControl::DrawDropMarker(CDC* pDC)
{
	if (m_nDropPos == 0 && GetRows()->GetCount() == 0)
		m_nDropPos = m_rcReportArea.top;
	else if (m_nDropPos < m_rcReportArea.top - 1
			|| m_nDropPos > m_rcReportArea.bottom)
		return;

	CRect rc(m_rcReportArea.left, m_nDropPos, m_rcReportArea.right, m_nDropPos + 1);

	pDC->FillSolidRect(rc, GetPaintManager()->m_clrHotDivider);
	CXTPPenDC pen(*pDC, GetPaintManager()->m_clrHotDivider);
	CXTPBrushDC brush(*pDC, GetPaintManager()->m_clrHotDivider);

	int x = rc.left;
	int y = m_nDropPos;

	POINT ptsLeftArrow[] =
	{
		{x, y - 2},
		{x + 2, y - 2},
		{x + 2, y - 5},
		{x + 7, y},
		{x + 2, y + 5},
		{x + 2, y + 2},
		{x, y + 2}
	};
	pDC->Polygon(ptsLeftArrow, 7);

	x = rc.right - 1;

	POINT ptsRightArrow[] =
	{
		{x, y - 2},
		{x - 2, y - 2},
		{x - 2, y - 5},
		{x - 7, y},
		{x - 2, y + 5},
		{x - 2, y + 2},
		{x, y + 2}
	};
	pDC->Polygon(ptsRightArrow, 7);
}

void CXTPReportControl::DrawExtDropMarker(CDC* pDC, int y)
{
	CRect rc(m_rcReportArea.left, y, m_rcReportArea.right, y + 1);

#ifdef _DEBUG
	pDC->FillSolidRect(rc, GetPaintManager()->m_clrHyper);
	CXTPPenDC pen(*pDC, GetPaintManager()->m_clrHyper);
	CXTPBrushDC brush(*pDC, GetPaintManager()->m_clrHyper);
#else
	pDC->FillSolidRect(rc, GetPaintManager()->m_clrHotDivider);
	CXTPPenDC pen(*pDC, GetPaintManager()->m_clrHotDivider);
	CXTPBrushDC brush(*pDC, GetPaintManager()->m_clrHotDivider);
#endif

	int x = rc.left;

	POINT ptsLeftArrow[] =
	{
		{x, y - 2},
		{x + 2, y - 2},
		{x + 2, y - 5},
		{x + 7, y},
		{x + 2, y + 5},
		{x + 2, y + 2},
		{x, y + 2}
	};
	pDC->Polygon(ptsLeftArrow, 7);

	x = rc.right - 1;

	POINT ptsRightArrow[] =
	{
		{x, y - 2},
		{x - 2, y - 2},
		{x - 2, y - 5},
		{x - 7, y},
		{x - 2, y + 5},
		{x - 2, y + 2},
		{x, y + 2}
	};
	pDC->Polygon(ptsRightArrow, 7);
}

void CXTPReportControl::OnSize(UINT nType, int cx, int cy)
{
	if (m_bOnSizeRunning)
		return;

	m_bOnSizeRunning = TRUE;

	CWnd::OnSize(nType, cx, cy);

	AdjustLayout();

	GetReportHeader()->AdjustColumnsWidth(cx);

	m_bOnSizeRunning = FALSE;
}

void CXTPReportControl::OnRButtonDown(UINT nFlags, CPoint point)
{
	CWnd::OnRButtonDown(nFlags, point);

	m_ptMouseDown = point;


	CXTPReportRow* pRow = HitTest(point, TRUE);
	if (pRow)
	{
		SetFocusedRow(pRow, pRow->IsSelected());
		m_pointDrag = point;
		if (m_bRClickDrag) m_bPrepareDrag = TRUE;
	}
	else if (m_bFastDeselectMode)
	{
		BOOL bControlKey = (GetKeyState(VK_CONTROL) < 0) || m_bControlKeyAlwaysOn;
		BOOL bShiftKey = (GetKeyState(VK_SHIFT) < 0);
		if (!bControlKey && !bShiftKey)
		{
			m_pSelectedRows->Clear();
			if (m_pSelectedRows->IsChanged())
				OnSelectionChanged();
		}
	}

}

void CXTPReportControl::OnRButtonUp(UINT nFlags, CPoint point)
{
	HWND hWnd = m_hWnd;
	CWnd::OnRButtonUp(nFlags, point);
//R-DD <<
	m_bPrepareDrag = FALSE;
//R-DD >>
	if (!IsWindow(hWnd)) // Can be destroyed in WM_CONTEXTMENU
		return;

	if (m_iCheckWithRightButton)
	{
		CXTPReportRow* pRow = HitTest(point, TRUE);
		if (pRow)
		{
			XTP_REPORTRECORDITEM_CLICKARGS clickArgs;
			clickArgs.pControl = this;
			clickArgs.pRow = pRow;
			clickArgs.ptClient = point;
			clickArgs.pColumn = NULL;

			// find clicked item
			clickArgs.pItem = pRow->HitTest(point, &clickArgs.rcItem, &clickArgs.pColumn);

			if (clickArgs.pItem && !clickArgs.pItem->IsKindOf(RUNTIME_CLASS(CXTPReportRecordItemPreview)))
			{
				clickArgs.pItem->OnClick(&clickArgs);
				if (m_iCheckRightButtonExtended)
					SetSelectionState(clickArgs.pItem->GetIndex(), clickArgs.pItem->GetCheckedState());
			}
		}
		else if (m_bFastDeselectMode)
		{
			BOOL bControlKey = (GetKeyState(VK_CONTROL) < 0) || m_bControlKeyAlwaysOn;
			BOOL bShiftKey = (GetKeyState(VK_SHIFT) < 0);
			if (!bControlKey && !bShiftKey)
			{
				m_pSelectedRows->Clear();
				if (m_pSelectedRows->IsChanged())
					OnSelectionChanged();
			}
		}
	}

}

LRESULT CXTPReportControl::OnNcHitTest(CPoint point)
{
	LRESULT ht = CWnd::OnNcHitTest(point);

	if (ht != HTCLIENT)
		return ht;

	DWORD dwStyle = GetStyle();

	if ((dwStyle & (WS_VSCROLL | WS_HSCROLL)) == 0)
		return ht;

	CXTPWindowRect rcWindow(this);

	if (dwStyle & WS_VSCROLL)
	{
		if (IsLayoutRTL() || !!(GetWindowLong(m_hWnd, GWL_EXSTYLE) & WS_EX_LEFTSCROLLBAR))
		{
			CRect rc(rcWindow.left, rcWindow.top, GetSystemMetrics(SM_CXVSCROLL), rcWindow.bottom - (dwStyle & WS_HSCROLL ? GetSystemMetrics(SM_CYHSCROLL) : 0));

			if (rc.PtInRect(point))
				return HTVSCROLL;
		}
		else
		{
			CRect rc(rcWindow.right- GetSystemMetrics(SM_CXVSCROLL), rcWindow.top,
				rcWindow.right, rcWindow.bottom - (dwStyle & WS_HSCROLL ? GetSystemMetrics(SM_CYHSCROLL) : 0));

			if (rc.PtInRect(point))
				return HTVSCROLL;
		}
	}

	if (dwStyle & WS_HSCROLL)
	{
		CRect rc(rcWindow.left, rcWindow.bottom - GetSystemMetrics(SM_CYHSCROLL),
			rcWindow.right - (dwStyle & WS_VSCROLL ? GetSystemMetrics(SM_CXVSCROLL) : 0), rcWindow.bottom);

		if (rc.PtInRect(point))
			return HTHSCROLL;
	}

	return ht;
}

void CXTPReportControl::OnLButtonDown(UINT nFlags, CPoint point)
{
	CWnd::OnLButtonDown(nFlags, point);
	SetFocus();
	EditItem(NULL);

	m_ptMouseDown = point;

	// columns processing
	CXTPReportHeader* pHeader = GetReportHeader();
	if (pHeader)
		pHeader->OnLButtonDown(point);

	// rows selection
	BOOL bControlKey = (GetKeyState(VK_CONTROL) < 0) || m_bControlKeyAlwaysOn;
	BOOL bShiftKey = (GetKeyState(VK_SHIFT) < 0);
	CXTPReportRow* pRow = HitTest(point);
	if (pRow)
	{
		XTP_REPORTRECORDITEM_CLICKARGS clickArgs;
		clickArgs.pControl = this;
		clickArgs.pRow = pRow;
		clickArgs.ptClient = point;
		clickArgs.pColumn = NULL;

		// find clicked item
		clickArgs.pItem = pRow->HitTest(point, &clickArgs.rcItem, &clickArgs.pColumn);

		if (pRow->OnLButtonDown(&clickArgs))
			return;

		// some rows may be unaccessible for end user
		if (pRow->GetType() == xtpRowTypeHeader
			&& !m_bHeaderRowsAllowAccess)
			return;

		if (pRow->GetType() == xtpRowTypeFooter
			&& !m_bFooterRowsAllowAccess)
			return;

		CXTPReportColumn* pFocusedColumn = clickArgs.pColumn;

		CUpdateContext updateContext(this);

		if (m_bFocusSubItems
			&& pFocusedColumn
			&& pRow->GetRecord()
			&& pRow->GetRecord()->GetItem(pFocusedColumn)
			&& pRow->GetRecord()->GetItem(pFocusedColumn)->IsFocusable())
			SetFocusedColumn(pFocusedColumn);

		BOOL bFocusChanged = TRUE;

		if (bShiftKey || !bControlKey)
		{
//Need to press Control to keep selection under collapse - expand actions
			  if (!pRow->IsSelected()
				  || bControlKey
				  || bShiftKey)
				  bFocusChanged = SetFocusedRow(pRow, bShiftKey, bControlKey);

			  BOOL bSelectionChanged = m_pSelectedRows->IsChanged();

			  if (bFocusChanged
				  && bControlKey
				  && IsSelectionEnabled()
				  && !bShiftKey)
			  {
				  m_pSelectedRows->Invert(pRow);
				  if (!bSelectionChanged && m_pSelectedRows->IsChanged())
					  OnSelectionChanged();
			  }
		}
		else
		{
// Setup for selection change in OnLButtonUp
			m_nClickRow = pRow->GetIndex();
		}

		m_pointDrag = point;
		m_bPrepareDrag = TRUE;
	}
	else if (m_bFastDeselectMode)
	{
		if (!bControlKey && !bShiftKey)
		{
			m_pSelectedRows->Clear();
			if (m_pSelectedRows->IsChanged())
				OnSelectionChanged();
		}
	}


}

void CXTPReportControl::OnLButtonUp(UINT nFlags, CPoint point)
{
	CWnd::OnLButtonUp(nFlags, point);

	m_bPrepareDrag = FALSE;

	EnsureStopAutoVertScroll();

	// columns processing
	CXTPReportHeader* pHeader = GetReportHeader();
	if (pHeader)
		pHeader->OnLButtonUp(nFlags, point);

	CXTPReportRow* pRow = HitTest(point, TRUE);
	if (pRow)
	{
		XTP_REPORTRECORDITEM_CLICKARGS clickArgs;
		clickArgs.pControl = this;
		clickArgs.pRow = pRow;
		clickArgs.ptClient = point;
		clickArgs.pColumn = NULL;

		// find clicked item
		clickArgs.pItem = pRow->HitTest(point, &clickArgs.rcItem, &clickArgs.pColumn);

		pRow->OnLButtonUp(&clickArgs);
	}
	BOOL bControlKey = (GetKeyState(VK_CONTROL) < 0) || m_bControlKeyAlwaysOn;
	BOOL bShiftKey = (GetKeyState(VK_SHIFT) < 0);

	if (pRow)
	{
		if (pRow->IsGroupRow())
		{
			pRow->OnClick(point);
			return;
		}

		if (m_nClickRow != -1 && m_nClickRow == pRow->GetIndex())
		{
			  BOOL bFocusChanged = TRUE;
			  //Need to press Control to keep selection under collapse - expand actions
			  if (!pRow->IsSelected()
				  || bControlKey
				  || bShiftKey)
				  bFocusChanged = SetFocusedRow(pRow, bShiftKey, bControlKey);

			  BOOL bSelectionChanged = m_pSelectedRows->IsChanged();

			  if (bFocusChanged
				  && bControlKey
				  && IsSelectionEnabled()
				  && !bShiftKey)
			  {
				  m_pSelectedRows->Invert(pRow);

				  if (!bSelectionChanged && m_pSelectedRows->IsChanged())
					  OnSelectionChanged();
			  }
		}
		m_nClickRow = -1;

		if (pRow->IsSelected()
			&& !bShiftKey
			&& !bControlKey)
			SetFocusedRow(pRow, bShiftKey, bControlKey);

		CMDTARGET_ADDREF(pRow);

		if (!m_bBlockSelection
			|| (!bShiftKey && !bControlKey && !m_bControlKeyAlwaysOn))
		{
			// row processing
			if (pRow->IsFocused())
				pRow->OnClick(point);
		}

//<<special case - allow to select with Ctrl + Click in single selection mode
		if (pRow->IsFocused()
			&& !m_bBlockSelection
			&& m_bControlKeyAlwaysOn)
			SetFocusedRow(pRow, bShiftKey, bControlKey);

		CMDTARGET_RELEASE(pRow);
//>>
	}
	else if (m_bFastDeselectMode)
	{
		if (!bControlKey && !bShiftKey)
		{
			m_pSelectedRows->Clear();
			if (m_pSelectedRows->IsChanged())
				OnSelectionChanged();
		}
	}

}

void CXTPReportControl::OnLButtonDblClk(UINT nFlags, CPoint ptDblClick)
{
	CWnd::OnLButtonDblClk(nFlags, ptDblClick);

	EditItem(NULL);

	// Information for delay editing
	EnsureStopDelayEditTimer();
	SetLastRqstEdit(-1, -1);

	// columns processing
	CXTPReportHeader* pHeader = GetReportHeader();
	if (pHeader)
		pHeader->OnLButtonDblClk(ptDblClick);

	// rows processing
	CXTPReportRow* pRow = HitTest(ptDblClick, TRUE);
	if (pRow)
		pRow->OnDblClick(ptDblClick);
	else // just notify parent
		SendMessageToParent(NULL, NULL, NULL, NM_DBLCLK, &ptDblClick);
}

void CXTPReportControl::OnContextMenu(CWnd* /*pWnd*/, CPoint pos)
{
	if (GetMouseMode() != xtpReportMouseNothing)
		return;

	CPoint ptClient = pos;
	ScreenToClient(&ptClient);

	// call context menu handler for report header if clicked inside
	if (m_rcHeaderArea.PtInRect(ptClient) ||
		m_rcGroupByArea.PtInRect(ptClient))
	{
		CXTPReportHeader* pHeader = GetReportHeader();
		if (pHeader)
			pHeader->OnContextMenu(ptClient);
		return;
	}

	// call context menu handler for report area if clicked inside
	if (m_rcReportArea.PtInRect(ptClient)           ||
		m_rcHeaderRecordsArea.PtInRect(ptClient)    ||
		m_rcFooterRecordsArea.PtInRect(ptClient))
	{
		// rows processing
		CXTPReportRow* pRow = HitTest(ptClient);
		if (pRow)
		{
			SetFocusedRow(pRow, pRow->IsSelected());
			pRow->OnContextMenu(ptClient);
		}
		else
		{
			if (m_bFastDeselectMode)
			{
// Must deselect before sending message to parent to show context menu.
				BOOL bControlKey = (GetKeyState(VK_CONTROL) < 0) || m_bControlKeyAlwaysOn;
				BOOL bShiftKey = (GetKeyState(VK_SHIFT) < 0);
				if (!bControlKey && !bShiftKey)
				{
					m_pSelectedRows->Clear();
					if (m_pSelectedRows->IsChanged())
						OnSelectionChanged();
				}
			}
			SendMessageToParent(NULL, NULL, NULL, NM_RCLICK, &pos);
		}

		return;
	}

	if (pos == CPoint(-1, -1))
	{
		CXTPReportRow* pFocusedRow = GetFocusedRow();
		if (pFocusedRow)
		{
			ptClient = CPoint(pFocusedRow->GetRect().left, pFocusedRow->GetRect().bottom);
			pFocusedRow->OnContextMenu(ptClient);
		}
		else
		{
			pos = m_rcReportArea.TopLeft();
			ClientToScreen(&pos);
			SendMessageToParent(NULL, NULL, NULL, NM_RCLICK, &pos);
		}
	}
}

CXTPReportColumn* CXTPReportControl::GetNextFocusableColumn(CXTPReportRow* pRow, int nColumnIndex, int nDirection)
{
	if (!pRow->GetRecord())
		return NULL;

	for (;;)
	{
		CXTPReportColumn* pColumn = GetReportHeader()->GetNextVisibleColumn(nColumnIndex, nDirection);
		if (!pColumn)
			return NULL;

		CXTPReportRecordItem* pItem = pRow->GetRecord()->GetItem(pColumn);

		if (pItem && pItem->IsFocusable())
			return pColumn;

		nColumnIndex = pColumn->GetIndex();
	}
}

BOOL CXTPReportControl::OnPreviewKeyDown(UINT& rnChar, UINT nRepCnt, UINT nFlags)
{
	XTP_NM_REPORTPREVIEWKEYDOWN nmParams;
	::ZeroMemory(&nmParams, sizeof(nmParams));

	nmParams.nChar = rnChar;
	nmParams.nRepCnt = nRepCnt;
	nmParams.nFlags = nFlags;
	nmParams.bCancel = FALSE;

	SendNotifyMessage(XTP_NM_REPORT_PREVIEWKEYDOWN, (NMHDR*)&nmParams);
	rnChar = nmParams.nChar;

	return !nmParams.bCancel;
}

void CXTPReportControl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);

	BOOL bControlKeyOnly = (GetKeyState(VK_CONTROL) < 0);
	BOOL bControlKey = (GetKeyState(VK_CONTROL) < 0) || m_bControlKeyAlwaysOn;
	BOOL bShiftKey = (GetKeyState(VK_SHIFT) < 0);

	CXTPReportRow* pFocusedRow = GetFocusedRow();

	CXTPDrawHelpers::KeyToLayout(this, nChar);
	int nRows = m_pRows->GetCount();
	int nHeadRows = m_pHeaderRows->GetCount();
	int nFootRows = m_pFooterRows->GetCount();

	if (nRows > 0
		|| nHeadRows > 0
		|| nFootRows > 0)
	switch (nChar)
	{
		case VK_F5:
			Recalc(bShiftKey);
			break;

		case VK_ADD:
			if (bControlKeyOnly)
				ExpandAll();
			else if (pFocusedRow && pFocusedRow->HasChildren() && !pFocusedRow->IsExpanded())
				pFocusedRow->SetExpanded(TRUE);
			break;

		case VK_SUBTRACT:
			if (bControlKeyOnly)
				CollapseAll();
			else if (pFocusedRow && pFocusedRow->HasChildren() && pFocusedRow->IsExpanded())
				pFocusedRow->SetExpanded(FALSE);
			break;

		case  VK_LEFT:
			if (m_bIconView)
			{
				GetNavigator()->MoveLeft(bShiftKey, bControlKey);
				break;
			}
			else
			{
				CMDTARGET_ADDREF(pFocusedRow);

				if (pFocusedRow && pFocusedRow->HasChildren() && pFocusedRow->IsExpanded())
				{
					pFocusedRow->SetExpanded(FALSE);
					if (pFocusedRow->IsGroupRow() && IsSkipGroupsFocusEnabled())
						pFocusedRow->SetSelected(FALSE);
					break;
				}
				if (pFocusedRow && !pFocusedRow->HasChildren() && !bShiftKey)
				{
					pFocusedRow->SetExpanded(FALSE);
					CXTPReportRow* pParRow = pFocusedRow;
					pParRow = pParRow->GetParentRow();
					while (pParRow)
					{
						if (!SetFocusedRow(pParRow))
							break;
						pParRow->SetExpanded(FALSE);
						if (pParRow->IsGroupRow() && IsSkipGroupsFocusEnabled())
							pParRow->SetSelected(FALSE);
						pParRow = pParRow->GetParentRow();
						if (bControlKeyOnly)
							break;
					}
					//break; - comment to give a chance for next condition without SHIFT pressed
				}
				if (pFocusedRow && m_bFocusSubItems && m_pFocusedColumn)
				{
					CXTPReportColumn* pColumn = GetNextFocusableColumn(pFocusedRow, m_pFocusedColumn->GetIndex(), -1);
					if (pColumn)
					{
						SetFocusedColumn(pColumn);
						SetFocusedRow(GetFocusedRow());
					}
				}

				CMDTARGET_RELEASE(pFocusedRow);
				break;
			}

		case VK_RIGHT:
			if (m_bIconView)
			{
				GetNavigator()->MoveRight(bShiftKey, bControlKey);
				break;
			}
			else
			{
				if (pFocusedRow && pFocusedRow->HasChildren() && !pFocusedRow->IsExpanded())
				{
					pFocusedRow->SetExpanded(TRUE, bControlKeyOnly);
					if (pFocusedRow->IsGroupRow() && IsSkipGroupsFocusEnabled())
						pFocusedRow->SetSelected(FALSE);
					break;
				}
				if (pFocusedRow && m_bFocusSubItems && m_pFocusedColumn)
				{
					CXTPReportColumn* pColumn = GetNextFocusableColumn(pFocusedRow, m_pFocusedColumn->GetIndex(), +1);
					if (pColumn)
					{
						SetFocusedColumn(pColumn);
						SetFocusedRow(GetFocusedRow());
					}
				}
				break;
			}
		case VK_DOWN:
			GetNavigator()->MoveDown(bShiftKey, bControlKey);
			break;

		case VK_UP:
			GetNavigator()->MoveUp(bShiftKey, bControlKey);
			break;

		case VK_HOME:
			GetNavigator()->MoveFirstRow(bShiftKey, FALSE);
			break;

		case VK_END:
			GetNavigator()->MoveLastRow(bShiftKey, FALSE);
			break;

		case VK_NEXT:
			GetNavigator()->MovePageDown(bShiftKey, FALSE);
			break;

		case VK_PRIOR:
			GetNavigator()->MovePageUp(bShiftKey, FALSE);
			break;

		case VK_RETURN:
			if (pFocusedRow && pFocusedRow->HasChildren())
				pFocusedRow->SetExpanded(!pFocusedRow->IsExpanded());
			break;

		case VK_ESCAPE:
			if (m_mouseMode != xtpReportMouseNothing)
				GetReportHeader()->CancelMouseMode();
			break;

		case VK_F2:
			GetNavigator()->BeginEdit();
			break;

		case VK_SPACE:
			if (IsSelectionEnabled() && bControlKey && pFocusedRow)
			{
				if (pFocusedRow->IsSelected())
					m_pSelectedRows->Remove(pFocusedRow);
				else
					m_pSelectedRows->Add(pFocusedRow);

				OnSelectionChanged();
				//OnSelectionChanged set Selection flag m_bChanged = FALSE;
			}
			break;

		case 'A':
			if (IsSelectionEnabled() && !bShiftKey && bControlKeyOnly && IsMultipleSelection())
			{
				if (IsVirtualMode())
				{
					GetNavigator()->MoveToRow(0);
					GetNavigator()->MoveToRow(GetRecords()->GetCount() - 1, TRUE);
				}
				else
				{
					_SelectRows(GetRecords());
				}
				RedrawControl();
			}
			break;
	}

	NMKEY nmgv;
	nmgv.nVKey = nChar;
	nmgv.uFlags = nFlags;
	SendNotifyMessage(NM_KEYDOWN, (NMHDR*)&nmgv);

}

//ICON_VIEW_MODE RELATED <<
void CXTPReportControl::SetNavigator(CXTPReportNavigator* pNavigator)
{
	if (pNavigator)
	{
		CMDTARGET_RELEASE(m_pNavigator);
		m_pNavigator = pNavigator;
	}
}

void CXTPReportControl::SetIconViewToDefaults()
{
	m_bIconView = FALSE;
	m_iIconWidth = 0;
	m_iIconHeight = 0;
	m_iIconWidthSpacing = 0;
	m_iIconHeightSpacing = 0;
	m_iIconViewColumn = -1;
	m_iIconViewRowsPerLine = 0;
	m_bUseIconColumnForNum = FALSE;
	m_iColumnForNum = -1;
	m_bPrevFocusSubItems = FALSE;
	m_bPrevHeaderAutoSize = FALSE;
	m_iIconPropNum = 0;
	m_iIconNum = 0;
	m_pPrevVisible = NULL;
	m_pPrevGroupsOrder = NULL;
	m_nPrevTreeIndent = 0;

	m_bIconColumnIndexNotValid = FALSE;

	m_bPrevHeaderRows = FALSE;
	m_bPrevFooterRows = FALSE;
	m_bPrevHeaderShow = FALSE;
	m_bPrevFooterShow = FALSE;

	m_bRClickDrag = FALSE;

	m_PrevVertStyle = xtpReportGridSolid;
	m_PrevHorStyle = xtpReportGridSolid;
}

void CXTPReportControl::SetIconColumn(CXTPReportColumn* pColumn)
{
	ASSERT(pColumn);
	m_iIconViewColumn = pColumn ? pColumn->GetItemIndex() : -1;
}

void CXTPReportControl::CreateIconColumn(BOOL bUseColumnForNum, int nWidth)
{
	if (GetColumns())
	{
		if (!m_bUseIconColumnForNum)
			m_bUseIconColumnForNum = bUseColumnForNum;

		if (m_iIconViewColumn == -1)
		{
			int iCnt = GetColumns()->GetCount();
			if (GetRecords())
			{
				int iCnt0 = iCnt;
				if (GetRecords()->GetAt(0))
						iCnt = max(iCnt, GetRecords()->GetAt(0)->GetItemCount());
				if (iCnt > iCnt0)
					for (int iIc = 0; iIc < iCnt - iCnt0; iIc++)
						AddColumn(new CXTPReportColumn(iIc,_T(""), 0, FALSE,XTP_REPORT_NOICON,FALSE,FALSE));
			}

			CXTPReportColumn* pIconCol = AddColumn(new CXTPReportColumn(iCnt, _T(""), 0));
			pIconCol->SetVisible(FALSE);

			if (m_bUseIconColumnForNum)
			{
				pIconCol->SetCaption(_T("#"));
				pIconCol->SetVisible(TRUE);
				pIconCol->SetHeaderAlignment(DT_CENTER);
				pIconCol->SetWidth(nWidth);

				if (GetColumns()->GetGroupsOrder()->GetCount() > 0 && nWidth < 30)
					nWidth = 30;

				GetPaintManager()->m_RecordNumberWidth = nWidth;

				pIconCol->AllowRemove(FALSE);
				pIconCol->EnableResize(FALSE);
				//pIconCol->SetAutoSize(FALSE);
				pIconCol->SetAllowDrag(FALSE);
				pIconCol->SetEditable(FALSE);
				pIconCol->SetGroupable(FALSE);
				pIconCol->SetSortable(FALSE);
			}
			m_iIconViewColumn = pIconCol->GetIndex();
			m_iColumnForNum = m_iIconViewColumn;
		}
	}
}

void CXTPReportControl::AssignIconViewPropNumAndIconNum(int nCol, int nIcon,
														BOOL bUseColumnForNum, int nWidth)
{
	BOOL bAlreadyUsed = m_bUseIconColumnForNum;

	m_bIconColumnIndexNotValid = TRUE;

	CreateIconColumn(bUseColumnForNum, nWidth);

	m_bIconColumnIndexNotValid = FALSE;

	if (!bAlreadyUsed && bUseColumnForNum)
	{
		m_bUseIconColumnForNum = bUseColumnForNum;

		if (m_iIconViewColumn > -1)
		{
			CXTPReportColumn* pIconCol = GetColumns()->GetAt(m_iIconViewColumn);
			if (m_bUseIconColumnForNum)
			{
				pIconCol->SetCaption(_T("#"));
				pIconCol->SetVisible(TRUE);
				pIconCol->SetHeaderAlignment(DT_CENTER);
				pIconCol->SetWidth(nWidth);

				if (GetColumns()->GetGroupsOrder()->GetCount() > 0 && nWidth < 30)
					nWidth = 30;

				GetPaintManager()->m_RecordNumberWidth = nWidth;

				pIconCol->AllowRemove(FALSE);
				pIconCol->EnableResize(FALSE);
				pIconCol->SetEditable(FALSE);
				pIconCol->SetGroupable(FALSE);
				pIconCol->SetSortable(FALSE);
			}
		}
	}
	if (nCol > -1)
		m_iIconPropNum = nCol;
	if (nIcon > -1)
		m_iIconNum = nIcon;

	if (!bAlreadyUsed && m_iColumnForNum > -1 && bUseColumnForNum)
		GetColumns()->ChangeColumnOrder(0, m_iColumnForNum);
}
//ICON_VIEW_MODE RELATED >>

void CXTPReportControl::OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	CWnd::OnSysKeyDown(nChar, nRepCnt, nFlags);
}

void CXTPReportControl::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	m_bWasShiftKey = (GetKeyState(VK_SHIFT) < 0);
	CWnd::OnKeyUp(nChar, nRepCnt, nFlags);
}

void CXTPReportControl::OnSysKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	CWnd::OnSysKeyUp(nChar, nRepCnt, nFlags);
}

void CXTPReportControl::OnChar(UINT nChar, UINT nRepCntr, UINT nFlags)
{
	NMCHAR nmgv;
	ZeroMemory(&nmgv, sizeof(NMCHAR));
	nmgv.ch = nChar;
	SendNotifyMessage(NM_CHAR, (NMHDR*)&nmgv);

	CXTPReportRow* pFocusedRow = GetFocusedRow();

	if (pFocusedRow && (nChar == VK_TAB))
	{
		if (m_bFocusSubItems)
		{
			EditItem(NULL);

			BOOL bBack = (GetKeyState(VK_SHIFT) < 0);
			GetNavigator()->MoveLeftRight(bBack);

			if (m_bAllowEdit && m_bEditOnClick && !m_bTrapTabKey)
				GetNavigator()->BeginEdit();
		}
		return;
	}

	if (m_pFocusedColumn && pFocusedRow && pFocusedRow->GetRecord()
		&& (nChar != VK_RETURN) && (nChar != VK_ESCAPE))
	{
		XTP_REPORTRECORDITEM_ARGS itemArgs(this, pFocusedRow, m_pFocusedColumn);

		if (itemArgs.pItem && itemArgs.pItem->OnChar(&itemArgs, nChar))
			return;
	}

	CWnd::OnChar(nChar, nRepCntr, nFlags);
}

void CXTPReportControl::OnCaptureChanged(CWnd* pWnd)
{
	if (m_mouseMode != xtpReportMouseNothing)
		GetReportHeader()->CancelMouseMode();

	CWnd::OnCaptureChanged(pWnd);
}

void CXTPReportControl::OnEnable(BOOL bEnable)
{
	UNREFERENCED_PARAMETER(bEnable);

	AdjustScrollBars();
}

void CXTPReportControl::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (pScrollBar != NULL)
	{
		CWnd::OnVScroll(nSBCode, nPos, pScrollBar);
		return;
	}
	if (!m_bIconView)
	{
		int nCurPos = m_nTopRow;

		// decide what to do for each diffrent scroll event
		switch (nSBCode)
		{
			case SB_TOP:
				nCurPos = 0;
				break;
			case SB_BOTTOM:
				nCurPos = GetScrollLimit(SB_VERT);
				break;
			case SB_LINEUP:
				nCurPos = max(nCurPos - 1, 0);
				break;
			case SB_PAGEUP:
				//nCurPos = max(nCurPos - GetReportAreaRows(nCurPos, FALSE), 0);
				nCurPos = max(nCurPos - max(1, GetReportAreaRows(nCurPos, FALSE)), 0);
				break;
			case SB_LINEDOWN:
				nCurPos = min(nCurPos + 1, GetScrollLimit(SB_VERT));
				break;
			case SB_PAGEDOWN:
				//nCurPos = min(nCurPos + GetReportAreaRows(nCurPos, TRUE), GetScrollLimit(SB_VERT));
				nCurPos = min(nCurPos + max(1, GetReportAreaRows(nCurPos, TRUE)), GetScrollLimit(SB_VERT));
				break;
			case SB_THUMBTRACK:
			case SB_THUMBPOSITION:
			{
				SCROLLINFO si;
				ZeroMemory(&si, sizeof(SCROLLINFO));
				si.cbSize = sizeof(SCROLLINFO);
				si.fMask = SIF_TRACKPOS;

				if (!GetScrollInfo(SB_VERT, &si))
					return;
				nCurPos = si.nTrackPos;
			}
			break;
		}

		SetTopRow(nCurPos);
	}
	else
	{
		int nCurPos = m_nTopRow;

		CXTPReportColumn* pColumn = GetColumns()->Find(m_iIconViewColumn);
		if (!pColumn)
		{
			CXTPReportControl::OnVScroll(nSBCode, nPos, pScrollBar);
			return;
		}

		// decide what to do for each diffrent scroll event
		switch (nSBCode)
		{
			case SB_TOP:
				nCurPos = 0; break;
			case SB_BOTTOM:
				nCurPos = GetScrollLimit(SB_VERT); break;
			case SB_LINEUP:
				nCurPos = max(nCurPos - m_iIconViewRowsPerLine, 0); break;
			case SB_LINEDOWN:
				nCurPos = min(nCurPos + m_iIconViewRowsPerLine, GetScrollLimit(SB_VERT)); break;
			case SB_PAGEUP:
				nCurPos = max(nCurPos - GetReportAreaRows(nCurPos, FALSE), 0); break;
			case SB_PAGEDOWN:
				nCurPos = min(nCurPos + GetReportAreaRows(nCurPos, TRUE), GetScrollLimit(SB_VERT)); break;
			case SB_THUMBTRACK:
			case SB_THUMBPOSITION:
			{
				SCROLLINFO si;
				ZeroMemory(&si, sizeof(SCROLLINFO));
				si.cbSize = sizeof(SCROLLINFO);
				si.fMask = SIF_TRACKPOS;

				if (!GetScrollInfo(SB_VERT, &si))
					return;
				nCurPos = si.nTrackPos;
			}
			break;
		}

		SetTopRow(nCurPos);
	}
}

void CXTPReportControl::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (pScrollBar != NULL)
	{
		CWnd::OnHScroll(nSBCode, nPos, pScrollBar);
		return;
	}
	int nCurPos = m_nLeftOffset;

	// FCS - full column scrolling
	int nLeftOffset_new = m_nLeftOffset;

	CXTPReportColumn *pPrev = NULL, *pCurr = NULL, *pNext = NULL;
	int nScrollPos = 0, nScrollMax = 0;
	int nFreezOffset = 0;
	if (IsFullColumnScrolling())
		nFreezOffset = GetReportHeader()->GetFulColScrollInfo(pPrev, pCurr, pNext, nScrollPos, nScrollMax);

	// decide what to do for each different scroll event
	switch (nSBCode)
	{
		case SB_TOP:
			nCurPos = 0;
			nLeftOffset_new = 0;
			nScrollPos = 0;
			break;

		case SB_BOTTOM:
			nCurPos = GetScrollLimit(SB_HORZ);
			if (IsFullColumnScrolling())
			{
				int nVisColCount = GetColumns()->GetVisibleColumnsCount();
				CXTPReportColumn *pLast = GetColumns()->GetVisibleAt(max(nVisColCount-1, 0));
				ASSERT(pLast);
				if (!pLast)
					return;

				nLeftOffset_new = labs(GetReportHeader()->m_rcHeader.left - pLast->GetRect().left) - nFreezOffset;
				nScrollPos = max(nScrollMax - 1, 0);
			}
			break;

		case SB_LINEUP:
			nCurPos = max(nCurPos - m_nHScrollStep, 0);
			if (IsFullColumnScrolling() && pPrev)
			{
				nLeftOffset_new = labs(GetReportHeader()->m_rcHeader.left - pPrev->GetRect().left) - nFreezOffset;
				nScrollPos = max(nScrollPos - 1, 0);
			}
			break;

		case SB_LINEDOWN:
			nCurPos = min(nCurPos + m_nHScrollStep, GetScrollLimit(SB_HORZ));
			if (IsFullColumnScrolling() && pNext)
			{
				nLeftOffset_new = labs(GetReportHeader()->m_rcHeader.left - pNext->GetRect().left) - nFreezOffset;
				nScrollPos = nScrollPos + 1;
			}
			break;

		case SB_PAGEUP:
			nCurPos = max(nCurPos - m_rcReportArea.Width(), 0);
			if (IsFullColumnScrolling() && pPrev)
			{
				nLeftOffset_new = labs(GetReportHeader()->m_rcHeader.left - pPrev->GetRect().left) - nFreezOffset;
				nScrollPos = max(nScrollPos - 1, 0);
			}
			break;

		case SB_PAGEDOWN:
			nCurPos = min(nCurPos + m_rcReportArea.Width(), GetScrollLimit(SB_HORZ));
			if (IsFullColumnScrolling() && pNext)
			{
				nLeftOffset_new = labs(GetReportHeader()->m_rcHeader.left - pNext->GetRect().left) - nFreezOffset;
				nScrollPos = nScrollPos + 1;
			}
			break;

		case SB_THUMBTRACK:
		case SB_THUMBPOSITION:
			SCROLLINFO si;
			ZeroMemory(&si, sizeof(SCROLLINFO));
			si.cbSize = sizeof(SCROLLINFO);
			si.fMask = SIF_TRACKPOS;

			if (!GetScrollInfo(SB_HORZ, &si))
				return;
			nCurPos = si.nTrackPos;

			if (IsFullColumnScrolling())
			{
				int nVisColCount = GetColumns()->GetVisibleColumnsCount();

				int nCurrCol = nVisColCount - nScrollMax + nCurPos;
				CXTPReportColumn *pCurrScroll = GetColumns()->GetVisibleAt(nCurrCol);

				if (!pCurrScroll)
					return;

				nLeftOffset_new = labs(GetReportHeader()->m_rcHeader.left - pCurrScroll->GetRect().left) - nFreezOffset;
				nScrollPos = nCurPos;
			}
			break;
	}

	if (IsFullColumnScrolling())
	{
		SetLeftOffset(nLeftOffset_new);
		SetScrollPos(SB_HORZ, nScrollPos);
	}
	else
	{
		SetLeftOffset(nCurPos);
	}
}

UINT CXTPReportControl::GetMouseScrollLines()
{
	int nScrollLines = 3; // default value

	if (XTPSystemVersion()->IsWin95())
	{
		HKEY hKey;
		if (ERROR_SUCCESS == RegOpenKeyEx(
			HKEY_CURRENT_USER, _T("Control Panel\\Desktop"), 0, KEY_QUERY_VALUE, &hKey))
		{
			TCHAR szData[128];
			DWORD dwKeyDataType;
			DWORD dwDataBufSize = sizeof(szData);

			if (ERROR_SUCCESS == RegQueryValueEx(
				hKey, _T("WheelScrollLines"), NULL, &dwKeyDataType, (LPBYTE) &szData, &dwDataBufSize))
			{
				nScrollLines = _tcstoul(szData, NULL, 10);
			}
			RegCloseKey(hKey);
		}
	}
	// win98 or greater
	else
		SystemParametersInfo (SPI_GETWHEELSCROLLLINES, 0, &nScrollLines, 0);

	return nScrollLines;
}

BOOL CXTPReportControl::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	if (!m_bVScrollBarVisible)
		return CWnd::OnMouseWheel(nFlags, zDelta, pt);

	UINT uiMsg;
	int nScrollsCount = 0;
	// calculate what should be sent
	if (m_nRowsPerWheel == -1)
	{
		// A m_nRowsPerWheel value less than 0 indicates that the mouse wheel scrolls whole pages, not just lines.
		int nPagesScrolled = zDelta / 120;
		uiMsg = nPagesScrolled > 0 ? SB_PAGEUP : SB_PAGEDOWN;
		nScrollsCount = nPagesScrolled > 0 ? nPagesScrolled : -nPagesScrolled;
	}
	else
	{
		int nRowsScrolled = m_nRowsPerWheel * zDelta / 120;
		uiMsg = nRowsScrolled > 0 ? SB_LINEUP : SB_LINEDOWN;
		nScrollsCount = nRowsScrolled > 0 ? nRowsScrolled : -nRowsScrolled;
	}

	if ((uiMsg == SB_LINEUP || uiMsg == SB_PAGEUP) && m_nTopRow == 0)
		return CWnd::OnMouseWheel(nFlags, 0, pt);

	if (uiMsg == SB_LINEDOWN || uiMsg == SB_PAGEDOWN)
	{
		int nDelta = GetReportAreaRows(m_nTopRow, TRUE);
		if (GetRows()->GetCount() - 1 - m_nTopRow - nDelta == 0)
			return CWnd::OnMouseWheel(nFlags, 0, pt);
	}

	BeginUpdate();

	// send scroll messages
	for (int i = 0; i < nScrollsCount; i++)
		OnVScroll(uiMsg, 0, NULL);

	EndUpdate();

	UpdateWindow();

	return CWnd::OnMouseWheel(nFlags, zDelta, pt);
}

void CXTPReportControl::OnMouseLeave()
{
	OnMouseMove(0, CPoint(-1, -1));
}

void CXTPReportControl::OnMouseMove(UINT nFlags, CPoint point)
{
	CWnd::OnMouseMove(nFlags, point);

	CXTPReportHeader* pHeader = GetReportHeader();
	if (pHeader)
		pHeader->OnMouseMove(nFlags, point);

	CXTPReportRow* pHotExpandButtonRow = NULL;

	if (GetMouseMode() == xtpReportMouseNothing)
	{
		CXTPReportRow* pRow = HitTest(point, TRUE);
		if (pRow)
		{
			pRow->OnMouseMove(nFlags, point);

			if (m_bShowTooltips && nFlags == 0)
				pRow->ShowToolTip(point, &m_wndTip);
		}

		if (pRow && pRow->IsGroupRow() && m_pPaintManager->m_columnStyle == xtpReportColumnResource)
		{
			if (pRow->m_rcCollapse.PtInRect(point))
			{
				pHotExpandButtonRow = pRow;
			}
		}

		if (m_pHotRow != pRow)
		{
			TRACKMOUSEEVENT tme =
			{
				sizeof(TRACKMOUSEEVENT), TME_LEAVE, GetSafeHwnd(), 0
			};
			_TrackMouseEvent (&tme);

			m_pHotRow = pRow;
		}

		// If mouse moved some since down...
		if (m_bPrepareDrag && (labs (point.x - m_pointDrag.x) > 3 ||
			labs (point.y - m_pointDrag.y) > 3))
		{
			// Prevent duplicate
			m_bPrepareDrag = FALSE;

			// Begin a drag operation
			OnBeginDrag(m_pointDrag, nFlags);
		}
	}

	if (m_pHotExpandButtonRow != pHotExpandButtonRow)
	{
		m_pHotExpandButtonRow = pHotExpandButtonRow;
		RedrawControl();
	}


}

void CXTPReportControl::OnBeginDrag(CPoint point)
{
	OnBeginDrag(point, 0);
}

void CXTPReportControl::OnBeginDrag(CPoint point, UINT nFlags)
{
	m_nClickRow = -1;
	if (SendMessageToParent(NULL, NULL, NULL, LVN_BEGINDRAG, &point))
		return;
//R-DD <<
	if ((nFlags & MK_RBUTTON) && SendMessageToParent(NULL, NULL, NULL, LVN_BEGINRDRAG, &point))
		return;
//R-DD >>
	if (m_cfReport ==  NULL)
		return;

	CXTPReportSelectedRows* pSelectedRows = GetSelectedRows();
	if (!pSelectedRows)
		return;

	if ((m_dwDragDropFlags & xtpReportAllowDrag) == 0)
		return;

	if (pSelectedRows->m_nRowType == xtpRowTypeHeader ||
		pSelectedRows->m_nRowType == xtpRowTypeFooter)
		return;

	int nCount = pSelectedRows->GetCount();
	for (int i = nCount - 1; i >= 0; i--)
	{
		CXTPReportRow* pRow = pSelectedRows->GetAt(i);
		if (pRow->IsGroupRow())
		{
			pRow->SetExpanded(TRUE);
			pRow->SelectChilds();
		}
	}

	int nRowsCount = pSelectedRows->GetCount();
	if (nRowsCount < 1)
		return;

	// minimize memory reallocs to improve performance
	UINT nAveRecordsSize = 1500; // bytes
	UINT nGrowBytes = ((nRowsCount * nAveRecordsSize) / 4096 + 1) * 4096;

	UINT nAllocFlags = GMEM_MOVEABLE | GMEM_DDESHARE | GMEM_ZEROINIT;

	CSharedFile fileRecords(nAllocFlags, nGrowBytes);

	BOOL bSucceed = FALSE;

	//------------------------------------------------------------------------
	const int cErrTextSize = 1024;
	TCHAR szErrText[cErrTextSize + 1];

	CXTPReportRecords* pDragRecords = new CXTPReportRecords(TRUE);

	if (!_GetSelectedRows(pDragRecords))
	{
		CMDTARGET_RELEASE(pDragRecords);
		return;
	}

	try
	{
		CArchive ar(&fileRecords, CArchive::store);
		CXTPPropExchangeArchive px(ar);

		bSucceed = _WriteRecordsData(&px, pDragRecords);

		ar.Close(); // perform Flush() and detach from file
	}
	catch(CArchiveException* pE)
	{
		if (pE->GetErrorMessage(szErrText, cErrTextSize))
		{
			TRACE(_T("EXCEPTION: CXTPReportControl::OnBeginDrag() - %s\n"), szErrText);
		}
		pE->Delete();
	}
	catch(CFileException* pE)
	{
		if (pE->GetErrorMessage(szErrText, cErrTextSize))
		{
			TRACE(_T("EXCEPTION: CXTPReportControl::OnBeginDrag() - %s\n"), szErrText);
		}
		pE->Delete();
	}
	catch(...)
	{
		TRACE(_T("EXCEPTION: CXTPReportControl::OnBeginDrag() - Unhandled Exception!\n"));
	}

	if (!bSucceed)
	{
		CMDTARGET_RELEASE(pDragRecords);
		return;
	}

	HGLOBAL hGlobal = fileRecords.Detach();

	m_bDragMode = TRUE;
	m_bInternalDrag = TRUE;

	DROPEFFECT dropEffectMask =
		((m_dwDragDropFlags & xtpReportAllowDragCopy) ? DROPEFFECT_COPY : 0) +
		((m_dwDragDropFlags & xtpReportAllowDragMove) ? DROPEFFECT_MOVE : 0);

	COleDataSource* pds = new COleDataSource();
	XTP_NM_REPORTDRAGDROP nmData;
	ZeroMemory(&nmData, sizeof(nmData));
	nmData.pRecords = pDragRecords;
	nmData.pDataSource = pds;       // Data Source - issue 22675

	if (SendNotifyMessage(XTP_NM_REPORT_BEGINDRAG, (NMHDR*) &nmData) == -1)
	{
		CMDTARGET_RELEASE(pDragRecords);
		CMDTARGET_RELEASE(m_pSelectedRowsBeforeDrag)
		m_bDragMode = FALSE;
		m_bInternalDrag = FALSE;

		pds->InternalRelease();
		return;
	}

	pds->CacheGlobalData(m_cfReport, hGlobal);

	if (!(m_dwDragDropFlags & xtpReportDontDropAsText))
	{
		// Get As Text
		CString strDropText = _GetSelectedRowsVisibleColsText();
		HGLOBAL hDropText = XTPAllocStrInGlobalMem(strDropText);
		if (hDropText)
			pds->CacheGlobalData(XTP_CF_TEXT_T, hDropText);
	}

	DROPEFFECT dropEffect = pds->DoDragDrop(dropEffectMask);
	m_bDragMode = FALSE;
	m_bInternalDrag = FALSE;

	if ((m_dwDropMarkerFlags & xtpReportDropSelect)
		&& m_pSelectedRowsBeforeDrag)
	{
		// Must remember the selected items before drag.
		m_pSelectedRows->Clear();
		for (int i = 0; i < m_pSelectedRowsBeforeDrag->m_arrSelectedBlocks.GetSize(); i++)
			m_pSelectedRows->m_arrSelectedBlocks.Add(m_pSelectedRowsBeforeDrag->m_arrSelectedBlocks[i]);

		CMDTARGET_RELEASE(m_pSelectedRowsBeforeDrag);
	}

	nmData.dropEffect = dropEffect;
	SendNotifyMessage(XTP_NM_REPORT_DRAGDROP_COMPLETED, (NMHDR*) &nmData);

	if ((dropEffect == DROPEFFECT_MOVE) && (dropEffectMask & DROPEFFECT_MOVE))
		Cut();

	CMDTARGET_RELEASE(pDragRecords);

	EnsureStopAutoVertScroll();

	pds->InternalRelease();
}

DROPEFFECT CXTPReportControl::OnDragOver(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point, int nState)
{
	DROPEFFECT dropEffect = DROPEFFECT_MOVE;
	BOOL bAbove = TRUE;
	CXTPReportRecord* pTargetRecord = NULL;
	int nTargetRow = -1;

	if (nState == 0)
	{
		// Must get the selected rows before the selection changes.
		if(m_dwDropMarkerFlags & xtpReportDropSelect)
		{
			// Must remember the selected items before drag.
			m_pSelectedRowsBeforeDrag = new CXTPReportSelectedRows(this);
			for(int i = 0; i < m_pSelectedRows->m_arrSelectedBlocks.GetSize(); i++)
				m_pSelectedRowsBeforeDrag->m_arrSelectedBlocks.Add(m_pSelectedRows->m_arrSelectedBlocks[i]);
		}
	}

	if ((dwKeyState & MK_CONTROL) == MK_CONTROL)
		dropEffect = DROPEFFECT_COPY;

	if (!m_nOLEDropMode)
	{
		if (m_cfReport == NULL)
			dropEffect = DROPEFFECT_NONE;

		if ((m_dwDragDropFlags & xtpReportAllowDrop) == 0)
			dropEffect = DROPEFFECT_NONE;

		XTP_NM_REPORTDRAGDROP nmData;
		nmData.dropEffect = dropEffect;
		nmData.pDataObject = pDataObject;
		nmData.pt = point;
//R-DD <<
		nmData.dwKeyState = dwKeyState;
//R-DD >>

		int iValid = (int) SendNotifyMessage(XTP_NM_REPORT_HASVALIDDROPTYPE, (NMHDR*) &nmData);

		m_nOLEDropAbove = nmData.bAbove;

		if ((dropEffect != DROPEFFECT_NONE)
			&& (!pDataObject || (!pDataObject->IsDataAvailable(m_cfReport) && iValid != 1)))
			dropEffect = DROPEFFECT_NONE;
	}

	int nDropPos = m_nDropPos;
	m_nDropPos = -1;

	if (dropEffect != DROPEFFECT_NONE)
	{
		EnsureStartAutoVertScroll();
//      DoAutoVertScrollIfNeed(m_pointDrag, point);

		m_bNoNeedSortedDragDrop = (GetColumns()->GetSortOrder()->GetCount() == 0
			&& GetColumns()->GetGroupsOrder()->GetCount() == 0);

		if (m_bNoNeedSortedDragDrop || m_bSortedDragDrop)
		{
			CXTPReportRow* pRow = HitTest(point);
			if (pRow && pRow->GetRecord())
			{
				if (pRow->GetRect().CenterPoint().y < point.y)
					bAbove = FALSE;
				if (bAbove)
					m_nDropPos = pRow->GetRect().top == 0 ? 0 : pRow->GetRect().top - 1;
					//m_nDropPos = pRow->GetRect().top - 1;
				else
					m_nDropPos = pRow->GetRect().bottom - 1;
				pTargetRecord = pRow->GetRecord();
				nTargetRow = pRow->GetIndex();

				if (((m_dwDropMarkerFlags & xtpReportDropSelect)
						&& (m_dwDropMarkerFlags & xtpReportDropBetween)
						&& point.y > (pRow->GetRect().top + 3)
						&& point.y < (pRow->GetRect().bottom - 3))
					|| ((m_dwDropMarkerFlags & xtpReportDropSelect)
						&& point.y > pRow->GetRect().top
						&& point.y < pRow->GetRect().bottom))
				{
					XTP_NM_REPORTDRAGDROP nmData;
					nmData.dropEffect = dropEffect;
					nmData.pDataObject = pDataObject;
					nmData.pTargetRecord = pTargetRecord;
					nmData.nTargetRow = nTargetRow;
					nmData.pt = point;
//R-DD <<
					nmData.dwKeyState = dwKeyState;
//R-DD >>

					int iValid = (int) SendNotifyMessage(XTP_NM_REPORT_VALIDDROPTARGET, (NMHDR*) &nmData);
					if (iValid == 1)
						m_pSelectedRows->Select(pRow);
					else
						m_pSelectedRows->Clear();

					m_nOLEDropAbove = nmData.bAbove;
				}
				else if(m_dwDropMarkerFlags & xtpReportDropSelect)
				{
					m_pSelectedRows->Clear();
				}
			}
			else
			{
				if ((m_nDropPos > m_rcReportArea.top - 1 || m_nDropPos < m_rcReportArea.bottom - 1))
					m_nDropPos = point.y;

				if (GetRows()->GetCount() == 0)
					m_nDropPos = 0;

				if (m_dwDropMarkerFlags & xtpReportDropSelect)
					m_pSelectedRows->Clear();
			}

			if (m_nDropPos == 0 && GetRows()->GetCount() == 0)
				m_nDropPos = m_rcReportArea.top;
			else if ((m_nDropPos < m_rcReportArea.top - 1 || m_nDropPos > m_rcReportArea.bottom - 1))
			//else if ((m_nDropPos < (m_bHeaderRecordsVisible ? m_rcReportArea.top : m_rcHeaderRecordsArea.bottom) - 1 || m_nDropPos > (!m_bFooterRecordsVisible ? m_rcReportArea.bottom : m_rcFooterRecordsArea.bottom) - 1))
				dropEffect = DROPEFFECT_NONE;
		}
	}


	if (nState == 0)
	{
		// entering, get report records
		CMDTARGET_RELEASE(m_pDropRecords);

		CFile* pFile = m_cfReport ? pDataObject->GetFileData(m_cfReport) : NULL;
		if (pFile)
		{
			m_pDropRecords = new CXTPReportRecords();

			const int cErrTextSize = 1024;
			TCHAR szErrText[cErrTextSize + 1];

			try
			{
				CArchive ar(pFile, CArchive::load);
				CXTPPropExchangeArchive px(ar);

				if (!_ReadRecordsFromData(&px, *m_pDropRecords))
				{
					m_pDropRecords->RemoveAll();
				}
				ar.Close(); // detach from file
			}
			catch(CArchiveException* pE)
			{
				if (pE->GetErrorMessage(szErrText, cErrTextSize))
				{
					TRACE(_T("EXCEPTION: CXTPReportControl::OnDragOver() - %s\n"), szErrText);
				}
				pE->Delete();
			}
			catch(CFileException* pE)
			{
				if (pE->GetErrorMessage(szErrText, cErrTextSize))
				{
					TRACE(_T("EXCEPTION: CXTPReportControl::OnDragOver() - %s\n"), szErrText);
				}
				pE->Delete();
			}
			catch(...)
			{
				TRACE(_T("EXCEPTION: CXTPReportControl::OnDragOver() - Unhandled Exception!\n"));
			}

			delete pFile;
		}

		m_bDragMode = TRUE;
	}
	else if (nState == 1)
	{
		// leaving, release drop records
		CMDTARGET_RELEASE(m_pDropRecords);

		if ((m_dwDropMarkerFlags & xtpReportDropSelect)
			&& m_pSelectedRowsBeforeDrag)
		{
			// Must remember the selected items before drag.
			m_pSelectedRows->Clear();
			for (int i = 0; i < m_pSelectedRowsBeforeDrag->m_arrSelectedBlocks.GetSize(); i++)
				m_pSelectedRows->m_arrSelectedBlocks.Add(m_pSelectedRowsBeforeDrag->m_arrSelectedBlocks[i]);

			CMDTARGET_RELEASE(m_pSelectedRowsBeforeDrag);
		}

		m_bDragMode = FALSE;
	}

	COleDataSource* pds = new COleDataSource();

	XTP_NM_REPORTDRAGDROP nmData;
	nmData.pRecords = m_pDropRecords;
	nmData.pTargetRecord = pTargetRecord;
	nmData.nTargetRow = nTargetRow;
	nmData.bAbove = bAbove;
	nmData.dropEffect = dropEffect;
	nmData.pt = point;
	nmData.nState = nState;

	nmData.pDataSource = pds;       // Data Source - issue 22675
	nmData.pDataObject = pDataObject;
//R-DD <<
	nmData.dwKeyState = dwKeyState;
//R-DD >>

	m_nOLEDropAbove = nmData.bAbove;

	SendNotifyMessage(XTP_NM_REPORT_DRAGOVER, (NMHDR*) &nmData);

	m_nOLEDropAbove = nmData.bAbove;

	if (nState != 1
		&& nmData.dropEffect == DROPEFFECT_NONE
		&& (m_dwDropMarkerFlags & xtpReportDropSelect))
		m_pSelectedRows->Clear();

	if (m_nDropPos != nDropPos)
		RedrawControl();

	pds->InternalRelease();

	return nmData.dropEffect;
}

BOOL CXTPReportControl::OnDrop(COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point)
{
	CMDTARGET_RELEASE(m_pDropRecords);

	EnsureStopAutoVertScroll();

	m_bDragMode = FALSE;

	if (m_nDropPos == -1)
		return FALSE;

	CUpdateContext updateContext(this);
	int nDropPos = m_nDropPos;
	m_nDropPos = -1;

	if (dropEffect != DROPEFFECT_COPY
		&& dropEffect != DROPEFFECT_MOVE)
		return FALSE;


	//if (IsVirtualMode()) return FALSE;
	int nTargetRow = -1;
	if ((m_dwDropMarkerFlags & xtpReportDropSelect)
		&& m_pSelectedRows->GetCount() == 1)
		nTargetRow = m_pSelectedRows->GetAt(0)->GetIndex();

	XTP_NM_REPORTDRAGDROP nmData;
	nmData.dropEffect = dropEffect;
	nmData.pDataObject = pDataObject;
	nmData.pt = point;

	int iValid = (int) SendNotifyMessage(XTP_NM_REPORT_HASVALIDDROPTYPE, (NMHDR*) &nmData);
	m_nOLEDropAbove = nmData.bAbove;
	if (iValid != -1 && nmData.dropEffect == DROPEFFECT_NONE)
	{
		// Drop handled or not allowed.
		if ((m_dwDropMarkerFlags & xtpReportDropSelect)
			&& m_pSelectedRowsBeforeDrag)
		{
			// Must remember the selected items before drag.
			m_pSelectedRows->Clear();
			for (int i = 0; i < m_pSelectedRowsBeforeDrag->m_arrSelectedBlocks.GetSize(); i++)
				m_pSelectedRows->m_arrSelectedBlocks.Add(m_pSelectedRowsBeforeDrag->m_arrSelectedBlocks[i]);

			CMDTARGET_RELEASE(m_pSelectedRowsBeforeDrag);
		}
		return FALSE;
	}

	if ((m_dwDropMarkerFlags & xtpReportDropSelect)
		&& m_pSelectedRowsBeforeDrag)
	{
		// Must remember the selected items before drag.
		m_pSelectedRows->Clear();
		for (int i = 0; i < m_pSelectedRowsBeforeDrag->m_arrSelectedBlocks.GetSize(); i++)
			m_pSelectedRows->m_arrSelectedBlocks.Add(m_pSelectedRowsBeforeDrag->m_arrSelectedBlocks[i]);

		CMDTARGET_RELEASE(m_pSelectedRowsBeforeDrag);
	}

	if (!pDataObject || (!pDataObject->IsDataAvailable(m_cfReport) && iValid != 1))
		return FALSE;

	if ((m_dwDragDropFlags & xtpReportAllowDrop) == 0)
		return FALSE;

	int nInsert = GetRecords()->GetCount();

	BOOL bAbove = TRUE;
	CXTPReportRecord* pTargetRecord = NULL;
	CXTPReportRecord* pOriginalTargetRecord = NULL;
	CXTPReportRow* pRow = HitTest(point);
	if (pRow)
	{
		if (pRow->GetRect().CenterPoint().y < point.y)
			bAbove = FALSE;
		pTargetRecord = pRow->GetRecord();
		if (pTargetRecord)
		{
			if (m_bUnrestrictedDragDrop)
				pOriginalTargetRecord = pTargetRecord;

			while (pTargetRecord->GetParentRecord() != NULL)
				pTargetRecord = pTargetRecord->GetParentRecord();

			nInsert = pTargetRecord->GetIndex();
			if (!m_bNoNeedSortedDragDrop
				&& m_bSortedDragDrop && nInsert != -1
				&& m_UaSorted.GetSize() > nInsert)
					nInsert = m_UaSorted.GetAt(nInsert);

			if (!bAbove)
				nInsert++;
		}
	}

	m_bInternalMove = FALSE;

	if (m_bInternalDrag && dropEffect == DROPEFFECT_MOVE)
	{
		m_bInternalMove = TRUE;

		if (nDropPos == -1)
			return FALSE;

		BOOL bRedraw = FALSE;
		if (!m_bNoNeedSortedDragDrop && m_bSortedDragDrop && GetColumns()->GetGroupsOrder()->GetCount() == 0)
		{
			ReleaseSorted();
			bRedraw = TRUE;
		}

		CXTPReportRecords* pDropRecords = new CXTPReportRecords(TRUE);
		if (!iValid && (!_GetSelectedRows(pDropRecords)
			|| pDropRecords->GetCount() == GetRows()->GetCount()))
		{
			CMDTARGET_RELEASE(pDropRecords);
			return FALSE;
		}

		if (bRedraw)
		{
			Populate();
			RedrawControl();
		}

		nmData.pRecords = pDropRecords;
		nmData.pTargetRecord = pTargetRecord;
		nmData.nTargetRow = nTargetRow;
		if (m_bUnrestrictedDragDrop)
			nmData.pTargetRecord = pOriginalTargetRecord;

		nmData.bAbove = bAbove;
		nmData.dropEffect = dropEffect;
		nmData.pDataObject = pDataObject;
		//nmData.bReturnValue = TRUE; //James proposal (wrong!)
		nmData.bReturnValue = FALSE;

		m_nOLEDropAbove = nmData.bAbove;

		CMDTARGET_ADDREF(pTargetRecord);


		if (SendNotifyMessage(XTP_NM_REPORT_DROP, (NMHDR*) &nmData) == -1)
		{
			CMDTARGET_RELEASE(pDropRecords);
			CMDTARGET_RELEASE(pTargetRecord);
			return nmData.bReturnValue;
		}

		if (!IsVirtualMode())
		{
			if (!pTargetRecord == NULL)
				nInsert = pTargetRecord->GetIndex();
			else
				nInsert = m_pRows->GetCount();

			if (!bAbove)
			  nInsert++;

			GetRecords()->Move(nInsert, pDropRecords);
			Populate();
			_SelectRows(pDropRecords);

			RedrawControl();
		}

		m_nOLEDropAbove = nmData.bAbove;

		SendNotifyMessage(XTP_NM_REPORT_RECORDS_DROPPED, (NMHDR*) &nmData);

		CMDTARGET_RELEASE(pDropRecords);

		CMDTARGET_RELEASE(pTargetRecord);

		return nmData.bReturnValue;
	}

	CXTPReportRecords* pDropRecords = NULL;

	if (pDataObject->IsDataAvailable(m_cfReport))
	{
		CFile* pFile = pDataObject->GetFileData(m_cfReport);
		if (pFile)
		{
			pDropRecords = new CXTPReportRecords();

			const int cErrTextSize = 1024;
			TCHAR szErrText[cErrTextSize + 1];

			try
			{
				CArchive ar(pFile, CArchive::load);
				CXTPPropExchangeArchive px(ar);

				if (!_ReadRecordsFromData(&px, *pDropRecords))
				{
					pDropRecords->RemoveAll();
				}
				ar.Close(); // detach from file
			}
			catch(CArchiveException* pE)
			{
				if (pE->GetErrorMessage(szErrText, cErrTextSize))
				{
					TRACE(_T("EXCEPTION: CXTPReportControl::OnDrop() - %s\n"), szErrText);
				}
				pE->Delete();
			}
			catch(CFileException* pE)
			{
				if (pE->GetErrorMessage(szErrText, cErrTextSize))
				{
					TRACE(_T("EXCEPTION: CXTPReportControl::OnDrop() - %s\n"), szErrText);
				}
				pE->Delete();
			}
			catch(...)
			{
				TRACE(_T("EXCEPTION: CXTPReportControl::OnDrop() - Unhandled Exception!\n"));
			}

			delete pFile;
		}
	}

	nmData.pRecords = pDropRecords;
	nmData.pTargetRecord = pTargetRecord;
	if (m_bUnrestrictedDragDrop)
		nmData.pTargetRecord = pOriginalTargetRecord;
	nmData.nTargetRow = nTargetRow;
	nmData.bAbove = bAbove;
	nmData.dropEffect = dropEffect;
	nmData.pDataObject = pDataObject;
	nmData.bReturnValue = TRUE;

	m_nOLEDropAbove = nmData.bAbove;

	if (SendNotifyMessage(XTP_NM_REPORT_DROP, (NMHDR*) &nmData) == -1)
	{
		CMDTARGET_RELEASE(pDropRecords);
		return nmData.bReturnValue;
	}

	m_nOLEDropAbove = nmData.bAbove;

	if (!IsVirtualMode() && pDropRecords)
	{
		// Add and Populate records
		int nRecordsCount = pDropRecords->GetCount();
		if (nRecordsCount > 0)
		{
			// Add
			for (int i = 0; i < nRecordsCount; i++)
			{
				CXTPReportRecord* pRecord = pDropRecords->GetAt(i);
				if (pRecord)
				{
					CMDTARGET_ADDREF(pRecord);
					m_pRecords->InsertAt(nInsert, pRecord);
					nInsert++;
				}
			}

			Populate();
			_SelectRows(pDropRecords);

			RedrawControl();

			SendNotifyMessage(XTP_NM_REPORT_RECORDS_DROPPED, (NMHDR*) &nmData);
		}
	}
	else
	{
		SendNotifyMessage(XTP_NM_REPORT_RECORDS_DROPPED, (NMHDR*) &nmData);
	}

	CMDTARGET_RELEASE(pDropRecords);

	//return TRUE;
	return nmData.bReturnValue;
}

void CXTPReportControl::ReleaseSorted()
{
	int nSortCnt = GetColumns()->GetSortOrder()->GetCount();
	if (nSortCnt > 0)
	{
		GetColumns()->GetSortOrder()->Clear();

		CXTPReportRecord* pDropRec;
		int N = (int) m_UaSorted.GetSize();
		for (int J = 0; J < N; J++)
		{
			pDropRec = GetRows()->GetAt(J)->GetRecord();
			if (pDropRec)
			{
				CXTPReportRecords* pDropRecs = new CXTPReportRecords(TRUE);
				pDropRecs->Add(pDropRec);
				GetRecords()->Move(J, pDropRecs);
				delete pDropRecs;
			}
		}
	}
	if (GetColumns()->GetGroupsOrder()->GetCount() > 0)
		GetColumns()->GetGroupsOrder()->Clear();

	m_bNoNeedSortedDragDrop = TRUE;
	m_bSortedDragDrop = TRUE;
}

void CXTPReportControl::OnTimer(UINT_PTR uTimerID)
{
	if (m_uAutoScrollTimerID == uTimerID)
	{
		CPoint ptMouse;
		if (::GetCursorPos(&ptMouse))
		{
			ScreenToClient(&ptMouse);
			DoAutoVertScrollIfNeed(m_pointDrag, ptMouse);
		}
	}

	if (m_uRqstEditTimer == uTimerID)
	{
		m_iLastRqstEditRow = -1;
		m_iLastRqstEditCol = -1;
	}

	// Information for delay editing.
	if (m_uiDelayEditTimer == uTimerID)
	{
		EnsureStopDelayEditTimer();

		ASSERT(!IsEditMode());
		if (m_iLastRqstEditRow != -1
			&& m_iLastRqstEditCol != -1
			&& m_iLastRqstEditRow < GetRows()->GetCount()
			&& IsEditOnDelayClick())
		{
			XTP_REPORTRECORDITEM_ARGS args(this, GetRows()->GetAt(m_iLastRqstEditRow), GetColumns()->Find(m_iLastRqstEditCol));

			EditItem(&args);

			StartLastRqstTimer();
		}
	}

	CWnd::OnTimer(uTimerID);
}

void CXTPReportControl::EnsureStopAutoVertScroll()
{
	if (m_uAutoScrollTimerID)
	{
		KillTimer(m_uAutoScrollTimerID);
		m_uAutoScrollTimerID = 0;
	}
}

void CXTPReportControl::EnsureStartAutoVertScroll()
{
	if (!m_uAutoScrollTimerID)
		m_uAutoScrollTimerID = SetTimer(XTP_REPORT_AUTO_SCROLL_TIMER_ID, m_nAutoVScrollTimerResolution, NULL);
}

void CXTPReportControl::SetAutoVScrollTimerResolution(UINT nNewTimerResolution)
{
	if (nNewTimerResolution == m_nAutoVScrollTimerResolution)
		return;

	BOOL bStartTimer = (m_uAutoScrollTimerID != 0);

	EnsureStopAutoVertScroll();
	m_nAutoVScrollTimerResolution = nNewTimerResolution;

	if (bStartTimer)
		EnsureStartAutoVertScroll();
}

void CXTPReportControl::DoAutoVertScrollIfNeed(CPoint ptClick, CPoint pt)
{
	if (!m_arrScreenRows.GetCount())
	{
		EnsureStopAutoVertScroll();
		return;
	}

	CXTPReportRow* pRow0 = m_arrScreenRows.GetAt(0);
	CXTPReportRow* pRow1 = m_arrScreenRows.GetAt(m_arrScreenRows.GetCount() - 1);

	if (!pRow0 || !pRow1)
	{
		EnsureStopAutoVertScroll();
		return;
	}

	int nDirection = 0;

	CRect rc0 = m_rcReportArea;
	CRect rc1 = m_rcReportArea;
	rc0.bottom = m_rcReportArea.top + 20; //while dragging item
	rc1.top = m_rcReportArea.bottom - 20;

	if (rc0.PtInRect(pt))
		nDirection = -1;
	else if (rc1.PtInRect(pt))
		nDirection = 1;

	if (!nDirection ||
		m_bDragMode &&
			(nDirection == 1 && pt.y - ptClick.y < 3 ||
			nDirection == -1 && ptClick.y - pt.y < 3))
	{
		EnsureStopAutoVertScroll();
		return;
	}

	int nTopRow = GetTopRowIndex();
	int nRowsCount = m_pRows->GetCount();

	if (nDirection == -1 && nTopRow > 0 ||
		nDirection == 1 &&
			(pRow1->GetIndex() < nRowsCount ||
			pRow1->GetRect().bottom > m_rcReportArea.bottom))
	{
		SetTopRow(nTopRow + nDirection);

		EnsureStartAutoVertScroll();
		return;
	}

	EnsureStopAutoVertScroll();
}

BOOL CXTPReportControl::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if (nHitTest == HTCLIENT)
	{
		switch (m_mouseMode)
		{
			case xtpReportMouseOverColumnDivide:
				SetCursor(GetReportHeader()->m_hResizeCursor);
			return TRUE;
		}
	}

	return CWnd::OnSetCursor(pWnd, nHitTest, message);
}


LRESULT CXTPReportControl::SendNotifyMessage(UINT nMessage, NMHDR* pNMHDR) const
{
	if (!IsWindow(m_hWnd))
		return 0;

	NMHDR nmhdr;
	if (pNMHDR == NULL)
		pNMHDR = &nmhdr;

	pNMHDR->hwndFrom = GetSafeHwnd();
	pNMHDR->idFrom = GetDlgCtrlID();
	pNMHDR->code = nMessage;

	CWnd *pOwner = GetOwner();
	if (pOwner && IsWindow(pOwner->m_hWnd))
		return pOwner->SendMessage(WM_NOTIFY, pNMHDR->idFrom, (LPARAM)pNMHDR);
	else
		return 0;
}


LRESULT CXTPReportControl::SendMessageToParent(CXTPReportRow* pRow, CXTPReportRecordItem* pItem, CXTPReportColumn* pColumn, UINT nMessage, CPoint* pPoint, int nHyperlink) const
{
	if (!IsWindow(m_hWnd))
		return 0;

	XTP_NM_REPORTRECORDITEM nmgv;
	nmgv.pItem = pItem;
	nmgv.pColumn = pColumn;
	nmgv.pRow = pRow;
	nmgv.nHyperlink = nHyperlink;
	nmgv.pt.x = 0;
	nmgv.pt.y = 0;
	if (pPoint)
		nmgv.pt = *pPoint;

	return SendNotifyMessage(nMessage, (NMHDR*)&nmgv);
}

void CXTPReportControl::DoPropExchange(CXTPPropExchange* pPX)
{
//ICON_VIEW_MODE RELATED <<
	BeginUpdate();

	BOOL bCurIconView = IsIconView();
	BOOL bSetBackToIconView = bCurIconView;
	if (bSetBackToIconView)
		SetIconView(FALSE);

//ICON_VIEW_MODE RELATED >>
	TRY
	{
		pPX->ExchangeSchemaSafe();

		CXTPPropExchangeSection secColumns(pPX->GetSection(_T("Columns")));
		m_pColumns->DoPropExchange(&secColumns);

		PX_Bool(pPX, _T("ShowGroupBox"), m_bGroupByEnabled, FALSE);

		if (pPX->GetSchema() >= _XTP_SCHEMA_110)
		{
			PX_Bool(pPX, _T("FullColumnScrolling"), m_bFullColumnScrolling, FALSE);
			PX_Int(pPX, _T("HScrollStep"), m_nHScrollStep, XTP_REPORT_HSCROLL_STEP);
		}

		PX_Int(pPX, _T("FreezeColumnsCount"), m_nFreezeColumnsCount, 0);
		if (pPX->IsLoading())
			SetFreezeColumnsCount(m_nFreezeColumnsCount);

		if (pPX->GetSchema() >= _XTP_SCHEMA_1310)
		{
			PX_Bool(pPX, _T("FreezeColumnsAbs"), m_bFreezeColumnsAbs, FALSE);
			PX_Bool(pPX, _T("MovePivot"), m_bMovePivot, FALSE);
			PX_Bool(pPX, _T("ForcePagination"), m_bForcePagination, FALSE);
			PX_Bool(pPX, _T("StrictBestFit"), m_bStrictBestFit, FALSE);
			PX_Bool(pPX, _T("SelectionExcludeGroupRows"), m_bSelectionExcludeGroupRows, TRUE);
			PX_Bool(pPX, _T("DesktopTrackerMode"), m_bDesktopTrackerMode, FALSE);
		}

		CXTPPropExchangeSection secHeader(pPX->GetSection(_T("Header")));
		GetReportHeader()->DoPropExchange(&secHeader);

		if (pPX->IsLoading())
		{
			GetReportHeader()->OnColumnsChanged(xtpReportColumnOrderChanged | xtpReportColumnPropExchange, NULL);
			Populate();
		}
	}
	CATCH(CArchiveException, e)
	{
	}
	END_CATCH
//ICON_VIEW_MODE RELATED <<
	TRY
	{
		pPX->ExchangeSchemaSafe();

		PX_Bool(pPX, _T("ShowIconView"), bCurIconView);

		if (pPX->IsLoading() && bCurIconView)
			SetIconView(bCurIconView);
	}
	CATCH(CArchiveException, e)
	{
	}
	END_CATCH

	if (pPX->IsStoring() && bSetBackToIconView)
		SetIconView(TRUE);

	EndUpdate();
//ICON_VIEW_MODE RELATED >>
}

//ICON_VIEW_MODE RELATED <<
void CXTPReportControl::SetIconView(BOOL bIconView /* = TRUE */)
{
	if (m_iIconViewColumn == -1) //No Icon Column Set
	{
		m_bIconView = FALSE;
		return;
	}

	if (m_bIconView == bIconView)
		return;

	CXTPReportColumn* pColumn = NULL;

	m_UaSelected.RemoveAll();
	CXTPReportSelectedRows* pSel = GetSelectedRows();
	for (int i = 0; i < pSel->GetCount(); i++)
	{
		if (pSel->GetAt(i))
			m_UaSelected.Add(pSel->GetAt(i)->GetIndex());
	}

	if (bIconView)
	{
		if (!m_pPrevGroupsOrder)
			m_pPrevGroupsOrder = new CXTPReportColumnOrder(GetColumns());

		if (!m_pPrevVisible)
			m_pPrevVisible = new CXTPReportColumnOrder(GetColumns());

		if (m_pPrevGroupsOrder)
		{
			m_pPrevGroupsOrder->Clear();

			CXTPReportColumnOrder* pCurrentColumnOrder = GetColumns()->GetGroupsOrder();
			if (pCurrentColumnOrder->GetCount())
			{
				for (int i = 0; i < GetColumns()->GetCount(); i++)
				{
					pColumn = pCurrentColumnOrder->GetAt(i);
					if (pColumn)
					{
						m_pPrevGroupsOrder->Add(pColumn);
						pColumn->SetVisible(FALSE);
					}
				}
				pCurrentColumnOrder->Clear();
				Populate();
			}
		}
		if (m_pPrevVisible)
		{
			m_pPrevVisible->Clear();

			for (int i = 0; i < GetColumns()->GetCount(); i++)
			{
				pColumn = GetColumns()->GetAt(i);
				if (pColumn && pColumn->IsVisible())
				{
					m_pPrevVisible->Add(pColumn);
					pColumn->SetVisible(FALSE);
				}
			}
		}

		m_iIconWidth = GetSystemMetrics(SM_CXICON);
		m_iIconHeight = GetSystemMetrics(SM_CYICON);
		m_iIconWidthSpacing = GetSystemMetrics(SM_CXICONSPACING) + 5;
		m_iIconHeightSpacing = GetSystemMetrics(SM_CYICONSPACING);

		m_iIconHeightSpacing = max(m_iIconHeightSpacing, (((CXTPReportPaintManager* ) GetPaintManager())->m_nRowHeight << 1) + m_iIconHeight);

		pColumn = GetColumns()->Find(m_iIconViewColumn);
		if (pColumn)
		{
			pColumn->SetVisible(TRUE);

			if (pColumn->IsResizable())
				GetPaintManager()->m_RecordNumberWidth = pColumn->GetWidth();
			pColumn->SetWidth(m_iIconWidth);
			pColumn->SetEditable(TRUE);
		}

		m_bPrevHeaderShow = IsHeaderVisible();
		m_bPrevFooterShow = IsFooterVisible();

		ShowHeader(FALSE);
		ShowFooter(FALSE);

		m_bPrevFocusSubItems = IsFocusSubItems();
		m_bPrevHeaderAutoSize = GetReportHeader()->IsAutoColumnSizing();
		// This is not needed since we only have one column and it screws up the selection.
		FocusSubItems(FALSE);
		GetReportHeader()->SetAutoColumnSizing(TRUE);

		m_PrevVertStyle = m_pPaintManager->GetGridStyle(TRUE);
		m_PrevHorStyle = m_pPaintManager->GetGridStyle(FALSE);

		SetGridStyle(TRUE, xtpReportGridNoLines);
		SetGridStyle(FALSE, xtpReportGridNoLines);

		m_bPrevHeaderRows = IsHeaderRowsVisible();
		m_bPrevFooterRows = IsFooterRowsVisible();

		ShowHeaderRows(FALSE);
		ShowFooterRows(FALSE);

		// Need this so the scrollbar is correct at the beginning.
		m_iIconViewRowsPerLine = GetNumRowsOnLine(GetReportHeader()->GetWidth());
		m_nPrevTreeIndent = GetPaintManager()->m_nTreeIndent;
		GetPaintManager()->m_nTreeIndent = 0;
		m_bPrevPreviewMode = IsPreviewMode();
		GetPaintManager()->EnablePreviewMode(FALSE);
	}
	else
	{
		GetPaintManager()->m_nTreeIndent = m_nPrevTreeIndent;
		GetReportHeader()->SetAutoColumnSizing(m_bPrevHeaderAutoSize);

		pColumn = GetColumns()->Find(m_iIconViewColumn);
		if (pColumn)
		{
			if (m_bUseIconColumnForNum)
			{
				if (m_iColumnForNum > -1)
				{
					if (m_pPrevGroupsOrder
						&& m_pPrevGroupsOrder->GetCount() > 0
						&& GetPaintManager()->m_RecordNumberWidth < 30)
						GetPaintManager()->m_RecordNumberWidth = 30;

					pColumn->SetWidth(GetPaintManager()->m_RecordNumberWidth);
					pColumn->SetMinWidth(GetPaintManager()->m_RecordNumberWidth);
					pColumn->SetVisible(TRUE);
				}
				else
					pColumn->SetVisible(FALSE);

				pColumn->SetHeaderAlignment(DT_CENTER);
				pColumn->SetEditable(FALSE);
			}
			else
			{
				pColumn->SetVisible(FALSE);
				pColumn->SetWidth(0);
			}
		}
		if (m_pPrevVisible)
		{
			for (int i = 0; i < m_pPrevVisible->GetCount(); i++)
			{
				pColumn = m_pPrevVisible->GetAt(i);
				if (pColumn)
				{
					pColumn->SetVisible(TRUE);
					if (pColumn->GetWidth() == 0)
						pColumn->SetWidth(pColumn->GetMinWidth());
				}
			}
		}

		if (m_pPrevGroupsOrder)
		{
			for (int i = 0; i < m_pPrevGroupsOrder->GetCount(); i++)
			{
				pColumn = m_pPrevGroupsOrder->GetAt(i);
				if (pColumn)
				{
					GetColumns()->GetGroupsOrder()->Add(pColumn);
					pColumn->SetVisible(TRUE);
				}
			}
			Populate();
		}

		ShowHeader(m_bPrevHeaderShow);
		ShowFooter(m_bPrevFooterShow);

		if (m_bPrevFocusSubItems)
			FocusSubItems(m_bPrevFocusSubItems);

		SetGridStyle(TRUE, m_PrevVertStyle);
		SetGridStyle(FALSE, m_PrevHorStyle);

		ShowHeaderRows(m_bPrevHeaderRows);
		ShowFooterRows(m_bPrevFooterRows);

		GetPaintManager()->EnablePreviewMode(m_bPrevPreviewMode);
	}

	BOOL bOldFirst(TRUE);
	for (int ii = 0; ii < m_UaSelected.GetSize(); ii++)
	{
		int j = (int) m_UaSelected.GetAt(ii);
		if (j == 0)
			bOldFirst = FALSE;
		if (j > -1)
		{
			if (GetRows()->GetAt(j))
				GetRows()->GetAt(j)->SetSelected(TRUE);
		}
	}
	if (bOldFirst)
	{
		if (GetRows()->GetAt(0))
			GetRows()->GetAt(0)->SetSelected(FALSE);
	}

	m_bIconView = bIconView;

	AdjustScrollBars();
}

int CXTPReportControl::GetNumRowsOnLine(int iTotalWidth)
{
	return max(iTotalWidth / m_iIconWidthSpacing, 1);
}

//ICON_VIEW_MODE RELATED >>

void CXTPReportControl::SerializeState(CArchive& ar)
{
	CXTPPropExchangeArchive px(ar);
	DoPropExchange(&px);
}

void CXTPReportControl::CollapseAll()
{
	BeginUpdate();
	BOOL bSignal(TRUE);
	m_UaSelected.RemoveAll();
	CXTPReportSelectedRows* pSel = GetSelectedRows();
	for (int i = 0; i < pSel->GetCount(); i++)
	{
		if (pSel->GetAt(i))
			m_UaSelected.Add(pSel->GetAt(i)->GetIndex());
	}
	if (pSel->GetCount() == 1 && pSel->GetAt(0)->GetIndex() == 0)
		bSignal = FALSE;

	for (int j = m_pRows->GetCount() - 1; j >= 0; j--)
		m_pRows->GetAt(j)->SetExpanded(FALSE);

	EndUpdate();
	EnsureVisible(GetFocusedRow());

	if (m_bSelectionExcludeGroupRows)
		UnselectGroupRows();

	if (bSignal)
		OnSelectionChanged();
}

void CXTPReportControl::ExpandAll(BOOL bRecursive)
{
	if (m_pRows->GetCount() == 0)
		return;

	BeginUpdate();

	for (int i = m_pRows->GetCount() - 1; i >= 0; i--)
	{
		CXTPReportRow* pRow = m_pRows->GetAt(i);
		if (pRow)
			pRow->SetExpanded(TRUE, bRecursive);
	}

	BOOL bOldFirst(TRUE);
	for (int j = 0; j < m_UaSelected.GetSize(); j++)
	{
		int jj = (int) m_UaSelected.GetAt(j);
		if (jj == 0)
			bOldFirst = FALSE;
		CXTPReportRow* pRow = m_pRows->GetAt(jj);
		if (pRow)
			pRow->SetSelected(TRUE);
	}

	int nTopRowIndex = GetTopRowIndex();

	if (bOldFirst && nTopRowIndex >= 0 && nTopRowIndex < GetRows()->GetCount())
		GetRows()->GetAt(nTopRowIndex)->SetSelected(FALSE);

	EndUpdate();
	EnsureVisible(GetFocusedRow());
}

void CXTPReportControl::SetMouseMode(XTPReportMouseMode nMode)
{
	XTP_TRACE(_T("SetMouseMode: Switching from %d to %d\n"), m_mouseMode, nMode);
	m_mouseMode = nMode;
}

void CXTPReportControl::RelayToolTipEvent(UINT message)
{
	if (m_wndTip.GetSafeHwnd() && m_wndTip.IsWindowVisible())
	{
		CPoint pt;
		GetCursorPos(&pt);

		if (!m_wndTip.GetHoverRect().PtInRect(pt))
		{
			m_wndTip.SetTooltipText(NULL);
			m_wndTip.Activate(FALSE, FALSE);
		}

		switch (message)
		{
			case WM_MOUSEWHEEL:
			case WM_KEYDOWN:
			case WM_SYSKEYDOWN:
			case WM_LBUTTONDOWN:
			case WM_RBUTTONDOWN:
			case WM_MBUTTONDOWN:
			case WM_LBUTTONUP:
			case WM_RBUTTONUP:
			case WM_MBUTTONUP:
			case WM_MOUSELEAVE:
				m_wndTip.Activate(FALSE, FALSE);
		}
	}

}

BOOL CXTPReportControl::OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	static BOOL bRelay = FALSE;
	if (m_wndTip.GetSafeHwnd() && m_wndTip.IsWindowVisible() && !bRelay)
	{
		bRelay = TRUE;
		RelayToolTipEvent(message);
		bRelay = FALSE;
	}

	if (m_pToolTipContext && m_bShowTooltips)
	{
		m_pToolTipContext->FilterToolTipMessage(this, message, wParam, lParam);
	}

	if (GetMarkupContext())
	{
		CPoint ptMouse(0);
		GetCursorPos(&ptMouse);
		ScreenToClient(&ptMouse);

		BOOL bRelay = FALSE;

		CXTPReportRow* pRow = HitTest(ptMouse);
		if (pRow)
		{
			CXTPReportRecordItem* pItem = pRow->HitTest(ptMouse);
			if (pItem && pItem->GetMarkupUIElement())
			{
				bRelay = TRUE;
				if (XTPMarkupRelayMessage(pItem->GetMarkupUIElement(), message, wParam, lParam, pResult))
					return TRUE;
			}
//13.2.2 <<
//enable markup mouse move for groups
			else if (pRow->IsGroupRow())
			{
				bRelay = TRUE;
				if (XTPMarkupRelayMessage(((CXTPReportGroupRow*) pRow)->m_pMarkupUIElement, message, wParam, lParam, pResult))
					return TRUE;
			}
//13.2.2 >>
		}

		if (!bRelay)
		{
			if (XTPMarkupRelayMessage(GetMarkupContext(), message, wParam, lParam, pResult))
				return TRUE;
		}

	}
	return CWnd::OnWndMsg(message, wParam, lParam, pResult);
}

void CXTPReportControl::OnSysColorChange()
{
	CWnd::OnSysColorChange();
	m_pPaintManager->RefreshMetrics();

	RedrawControl();
}

UINT CXTPReportControl::OnGetDlgCode()
{
	return (m_bFocusSubItems ? DLGC_WANTTAB : 0) | DLGC_WANTARROWS | DLGC_WANTCHARS;
}

void CXTPReportControl::OnSetFocus(CWnd* pOldWnd)
{
	CWnd::OnSetFocus(pOldWnd);
	RedrawControl();

}

void CXTPReportControl::OnKillFocus (CWnd* pNewWnd)
{
	CWnd::OnKillFocus(pNewWnd);

	EnsureStopAutoVertScroll();
	RedrawControl();

}

int CXTPReportControl::GetHeaderIndent() const
{
	return GetIndent(GetReportHeader()->m_nIndentLevel);
}

CXTPReportRow* CXTPReportControl::CreateRow()
{
	return new CXTPReportRow_Batch();
}

CXTPReportGroupRow* CXTPReportControl::CreateGroupRow()
{
	return new CXTPReportGroupRow_Batch();
}

CXTPReportRow* CXTPReportControl::CreateHeaderFooterRow()
{
	return new CXTPHeapObjectT<CXTPReportRow, CXTPReportAllocatorDefault>;
}

void CXTPReportControl::ShowIconWhenEditing(BOOL bShow)
{
	m_bShowIconWhenEditing = bShow;
}

BOOL CXTPReportControl::IsShowIconWhenEditing()
{
	return m_bShowIconWhenEditing;
}

void CXTPReportControl::EditItem(XTP_REPORTRECORDITEM_ARGS* pItemArgs)
{
	CXTPReportRecordItem* pItem = pItemArgs ? pItemArgs->pItem : NULL;

	if (m_pActiveItem != NULL)
	{
		m_pActiveItem->OnCancelEdit(this, TRUE);
		m_pActiveItem = NULL;

		if (!m_bFocusSubItems)
			SetFocusedColumn(NULL);
	}
	CMDTARGET_RELEASE(m_ptrVirtualEditingRow);

	if (pItem && pItemArgs && pItemArgs->pRow)
	{
		if (!HasFocus())
			SetFocus();

		if (!IsVirtualMode())
		{
			AdjustScrollBars();
			RedrawControl();
			UpdateWindow();
		}

		if (IsVirtualMode())
		{
			int nRowIndex = pItemArgs->pRow->GetIndex();

			EnsureVisible(pItemArgs->pRow);

			RedrawControl();
			UpdateWindow();
			pItemArgs->pRow = NULL; // RedrawControl delete and re-create new screen rows

			BOOL bMapped = FALSE;
			int nScrCount = m_arrScreenRows.GetCount();
			for (int i = 0; i < nScrCount; i++)
			{
				CXTPReportRow* pRow = m_arrScreenRows.GetAt(i);
				if (pRow->GetIndex() == nRowIndex)
				{
					pItemArgs->pRow = pRow;
					bMapped = TRUE;

					ASSERT(m_ptrVirtualEditingRow == NULL);

					m_ptrVirtualEditingRow = pRow;
					CMDTARGET_ADDREF(m_ptrVirtualEditingRow);
					break;
				}
			}
			ASSERT(bMapped);
			if (!bMapped)
				return;
		}
		else if (GetFocusedRow() != pItemArgs->pRow)
		{
			SetFocusedRow(pItemArgs->pRow);
			UpdateWindow();
		}

		pItemArgs->rcItem = pItemArgs->pRow->GetItemRect(pItem);
		pItemArgs->rcItem.left += (pItemArgs->rcItem.left == 0 ? GetHeaderIndent() : 0);

		if (m_bShowIconWhenEditing)
		{
			XTP_REPORTRECORDITEM_METRICS* pMetrics = new XTP_REPORTRECORDITEM_METRICS();
			XTP_REPORTRECORDITEM_DRAWARGS drawArgs;
			drawArgs.pControl = this;
			drawArgs.pColumn = pItemArgs->pColumn;
			drawArgs.rcItem = pItemArgs->pColumn->GetRect();

			drawArgs.pRow = pItemArgs->pRow;
			drawArgs.pItem = pItemArgs->pItem;

			pItemArgs->pRow->GetItemMetrics(&drawArgs, pMetrics);

			int nIcon = pMetrics->nItemIcon;
			nIcon = (nIcon != XTP_REPORT_NOICON) ? nIcon : pItem->GetIconIndex();
			if (nIcon != XTP_REPORT_NOICON)
			{
				CXTPImageManagerIcon* pIcon = GetImageManager()->GetImage(nIcon, 0);
				if (pIcon)
					pItemArgs->rcItem.left += pIcon->GetWidth();
			}

			CMDTARGET_RELEASE(pMetrics);
		}

		if (pItem->GetHasCheckbox() && pItem->GetCanEditCheckboxItem())
			pItemArgs->rcItem.left += 15;

		if (SetFocusedColumn(pItemArgs->pColumn))
		{
			m_pActiveItem = pItem;
			pItem->OnBeginEdit(pItemArgs);

			if (!IsEditMode()) // Need to actually cancel the edit - old version (10) did not have this call
				EditItem(NULL);
			if (pItem->IsKindOf(RUNTIME_CLASS(CXTPReportRecordItemPreview)) && !m_bPreviewAllowEdit)
				EditItem(NULL);
		}

			// Fix selection
		if (GetInplaceEdit()->GetSafeHwnd()
			&& GetInplaceEdit()->GetItem() == pItem)
		{
			CXTPReportRecordItemEditOptions* pEditOptions = pItem->GetEditOptions(pItemArgs->pColumn);
			if (pEditOptions && pEditOptions->m_bSelectTextOnEdit)
			{
				GetInplaceEdit()->SetSel(0, -1);
			}
			else
			{
				CString str;
				GetInplaceEdit()->GetWindowText(str);
				GetInplaceEdit()->SetSel(str.GetLength(), str.GetLength());
			}
		}
	}

	RedrawControl();
}

BOOL CXTPReportControl::HasFocus() const
{
	const CWnd* pFocusWnd = GetFocus();
	if (!pFocusWnd)
		return FALSE;

	return (pFocusWnd->GetSafeHwnd() == m_hWnd)
		|| (pFocusWnd->GetParent()->GetSafeHwnd() == m_hWnd)
		|| (pFocusWnd->GetOwner()->GetSafeHwnd() == m_hWnd);
}

void CXTPReportControl::ReleaseItem(int nIndex)
{
	int i(0);
	if (IsVirtualMode() && m_pRecords->GetCount())
	{
		CXTPReportRecord* pRecord = m_pRecords->GetAt(0);

		pRecord->m_arrItems[nIndex]->InternalRelease();
		pRecord->m_arrItems.RemoveAt(nIndex);
	}
	else
	{
		for (i = 0; i < m_pRecords->GetCount(); i++)
		{
			CXTPReportRecord* pRecord = m_pRecords->GetAt(i);

			pRecord->m_arrItems[nIndex]->InternalRelease();
			pRecord->m_arrItems.RemoveAt(nIndex);
		}
	}

	CXTPReportColumns* pColumns = GetColumns();
	CXTPReportColumn* pColumnToRemove = NULL;
	int nColumnsCount = pColumns->GetCount();
	for (i = 0; i < nColumnsCount; i++)
	{
		CXTPReportColumn* pColumn = pColumns->GetAt(i);

		if (pColumn->m_nItemIndex > nIndex)
			pColumn->m_nItemIndex--;
		else if (pColumn->m_nItemIndex == nIndex)
			pColumnToRemove = pColumn;
	}

	if (pColumnToRemove)
		pColumns->Remove(pColumnToRemove);
}

void CXTPReportControl::SetVirtualMode(CXTPReportRecord* pVirtualRecord, int nCount, int nFields)
{
	if (m_pRecords)
	{
		if (pVirtualRecord)
		{
			int nItems = pVirtualRecord->GetItemCount();
			if (nItems == 0 && nFields > 0)
				for (int i = 0; i < nFields; i++)
					pVirtualRecord->AddItem(new CXTPReportRecordItem());
		}
		else
		{
			pVirtualRecord = new CXTPReportRecord();
			if (nFields > 0)
				for (int i = 0; i < nFields; i++)
					pVirtualRecord->AddItem(new CXTPReportRecordItem());
		}
		if (m_iIconViewColumn > -1)
		{
			int nItems = pVirtualRecord->GetItemCount();
			CXTPReportRecordItem* pLastItem = pVirtualRecord->GetItem(nItems - 1);
			if (!pLastItem->IsKindOf(RUNTIME_CLASS(CXTPReportRecordItemIcon)))
				pVirtualRecord->AddItem(new CXTPReportRecordItemIcon());
		}

		m_pRecords->SetVirtualMode(pVirtualRecord, nCount);
	}
}

BOOL CXTPReportControl::IsVirtualMode() const
{
	return m_pRecords ? m_pRecords->IsVirtualMode() : FALSE;
}

CXTPReportRow* CXTPReportControl::GetFocusedRow() const
{
	if (m_nFocusedRow != -1)
		return m_pRows ? m_pRows->GetAt(m_nFocusedRow) : NULL;

	if (m_nFocusedHeaderRow != -1)
		return m_pHeaderRows ? m_pHeaderRows->GetAt(m_nFocusedHeaderRow) : NULL;

	if (m_nFocusedFooterRow != -1)
		return m_pFooterRows ? m_pFooterRows->GetAt(m_nFocusedFooterRow) : NULL;

	return NULL;
}

void CXTPReportControl::OnStyleChanged(int nStyleType, LPSTYLESTRUCT lpStyleStruct)
{
	CWnd::OnStyleChanged(nStyleType, lpStyleStruct);

	RedrawControl();
}

CXTPToolTipContext* CXTPReportControl::GetToolTipContext() const
{
	return m_pToolTipContext;
}

INT_PTR CXTPReportControl::OnToolHitTest(CPoint point, TOOLINFO* pTI) const
{
	ASSERT_VALID(this);
	ASSERT(::IsWindow(m_hWnd));

	// check child windows first by calling CControlBar
	INT_PTR nHit = CWnd::OnToolHitTest(point, pTI);
	if (nHit != -1)
		return nHit;

	nHit = GetReportHeader()->OnToolHitTest(point, pTI);
	if (nHit != -1)
		return nHit;

	CXTPReportRow* pRow = HitTest(point, TRUE);
	if (pRow)
		nHit = pRow->OnToolHitTest(point, pTI);

	return nHit;
}

void CXTPReportControl::SetLayoutRTL(BOOL bRightToLeft)
{
	if (!XTPSystemVersion()->IsLayoutRTLSupported())
		return;

	if (!m_hWnd)
		return;

	if (bRightToLeft)
		ModifyStyleEx(NULL, WS_EX_LAYOUTRTL);
	else
		ModifyStyleEx(WS_EX_LAYOUTRTL, NULL);

	//ModifyStyleEx(bRightToLeft ? 0 : WS_EX_LAYOUTRTL, !bRightToLeft ? 0 : WS_EX_LAYOUTRTL);
	GetImageManager()->DrawReverted(bRightToLeft);

	GetInplaceEdit()->DestroyWindow();
	AdjustLayout();

	if (GetParent())
	{
		CRect rc;
		GetParent()->GetClientRect(&rc);
		GetParent()->SetWindowPos(NULL, 0, 0, rc.Width() + 1, rc.Height(), SWP_NOMOVE | SWP_NOZORDER);
	}
}

BOOL CXTPReportControl::IsLayoutRTL()
{
	if (!XTPSystemVersion()->IsLayoutRTLSupported())
		return FALSE;

	if (!m_hWnd)
		return FALSE;

	return !!(GetWindowLong(m_hWnd, GWL_EXSTYLE) & WS_EX_LAYOUTRTL);
}

//////////////////////////////////////////////////////////////////////////
// Clipboard operations

BOOL CXTPReportControl::CanCopy()
{
	// Check whether are there any selected rows to be copied
	CXTPReportSelectedRows* pSelRows = GetSelectedRows();

	if ((pSelRows != NULL) && (pSelRows->GetCount() > 0))
		return TRUE;

	return FALSE;
}

BOOL CXTPReportControl::CanCut()
{
//Old VirtualMode
	if (IsVirtualMode())
		return FALSE;

	return CanCopy();
}

BOOL CXTPReportControl::CanPaste()
{
	CLIPFORMAT uCF_Records = (CLIPFORMAT)::RegisterClipboardFormat(XTPREPORTCTRL_CF_RECORDS);

	BOOL bCan = FALSE;
	COleDataObject odj;
	if (odj.AttachClipboard())
	{
		bCan = odj.IsDataAvailable(CF_TEXT)
			|| odj.IsDataAvailable(CF_UNICODETEXT)
			|| odj.IsDataAvailable(uCF_Records) ;
	}
	return bCan;
}

void CXTPReportControl::Cut()
{
	CWaitCursor _WC;

	Copy();

	if (IsVirtualMode())
		return;

// Delete selected rows
	CXTPInternalCollectionT<CXTPReportRow> arSelectedRows;
	_GetSelectedRows(NULL, &arSelectedRows);
	int nSelRowsCount = (int) arSelectedRows.GetSize();
	int nFirstSelRow = INT_MAX;
	for (int i = nSelRowsCount - 1; i >= 0 ; i--)
	{
		CXTPReportRow* pRow = arSelectedRows.GetAt(i, FALSE);
		if (pRow)
		{
			nFirstSelRow = min(nFirstSelRow, pRow->GetIndex());
			VERIFY(RemoveRowEx(pRow, FALSE));
		}

		arSelectedRows.RemoveAt(i);
	}

	if (GetSelectedRows())
	{
		GetSelectedRows()->Clear();

		if (nFirstSelRow != INT_MAX)
		{
			m_nFocusedRow = min(nFirstSelRow, GetRows()->GetCount() - 1);

			if (GetFocusedRow())
			{
				SetFocusedRow(GetFocusedRow());
				GetSelectedRows()->Add(GetFocusedRow());
			}
		}
	}

	AdjustScrollBars();
	RedrawControl();
}

// We support text format with \t dividers for record items and
// \r\n dividers for records (simple tab-separated text).
// Such format is also supported by Excel and some other applications.
void CXTPReportControl::Copy()
{
	CWaitCursor _WC;

	int nRowsCount = 1;
	if (GetSelectedRows())
		nRowsCount = GetSelectedRows()->GetCount();

	// minimize memory reallocs to improve performance
	UINT nAveRecordsSize = 1500; // bytes
	UINT nGrowBytes = ((nRowsCount * nAveRecordsSize) / 4096 + 1) * 4096;

	UINT nAllocFlags = GMEM_MOVEABLE | GMEM_DDESHARE | GMEM_ZEROINIT;

	CSharedFile fileRecords(nAllocFlags, nGrowBytes);

	BOOL bIsRecordsData = FALSE;

	//------------------------------------------------------------------------
	const int cErrTextSize = 1024;
	TCHAR szErrText[cErrTextSize + 1];

	try
	{
		CArchive ar(&fileRecords, CArchive::store);
		CXTPPropExchangeArchive px(ar);

		bIsRecordsData = _WriteSelectedRowsData(&px);

		ar.Close(); // perform Flush() and detach from file
	}
	catch(CArchiveException* pE)
	{
		if (pE->GetErrorMessage(szErrText, cErrTextSize))
		{
			TRACE(_T("EXCEPTION: CXTPReportControl::Copy() - %s\n"), szErrText);
		}
		pE->Delete();
	}
	catch(CFileException* pE)
	{
		if (pE->GetErrorMessage(szErrText, cErrTextSize))
		{
			TRACE(_T("EXCEPTION: CXTPReportControl::Copy() - %s\n"), szErrText);
		}
		pE->Delete();
	}
	catch(...)
	{
		TRACE(_T("EXCEPTION: CXTPReportControl::Copy() - Unhandled Exception!\n"));
	}

	CString strClipText = _GetSelectedRowsVisibleColsText();

	CLIPFORMAT uCF_Records = (CLIPFORMAT)::RegisterClipboardFormat(XTPREPORTCTRL_CF_RECORDS);

	// Put prepared text into the clipboard
	if (OpenClipboard())
	{
		::EmptyClipboard();

		// 1 - Text
		HGLOBAL hText = XTPAllocStrInGlobalMem(strClipText);
		if (hText != NULL)
		{
			::SetClipboardData(XTP_CF_TEXT_T, hText);
		}

		// 2 - Blob data
		if (bIsRecordsData)
		{
			HGLOBAL hData = fileRecords.Detach();
			::GlobalUnlock(hData); // unlock data
			::SetClipboardData(uCF_Records, hData);
		}

		::CloseClipboard();
	}
}

void CXTPReportControl::Paste()
{
	CWaitCursor _WC;

	CLIPFORMAT uCF_Records = (CLIPFORMAT)::RegisterClipboardFormat(XTPREPORTCTRL_CF_RECORDS);

	CXTPReportRecords arRecords;
	BOOL bTryPasteFromText = TRUE;

	// Retrieve text from the clipboard
	if (!OpenClipboard())
		return;

	if (::IsClipboardFormatAvailable(uCF_Records))
	{
		HGLOBAL hPasteData = ::GetClipboardData(uCF_Records);

		if (hPasteData)
		{
			bTryPasteFromText = FALSE;

			const int cErrTextSize = 1024;
			TCHAR szErrText[cErrTextSize + 1];

			CSharedFile fileSahred;
			fileSahred.SetHandle(hPasteData, FALSE);
			CArchive ar(&fileSahred, CArchive::load);

			try
			{
				CXTPPropExchangeArchive px(ar);

				if (!_ReadRecordsFromData(&px, arRecords))
				{
					arRecords.RemoveAll();
				}
			}
			catch(CArchiveException* pE)
			{
				if (pE->GetErrorMessage(szErrText, cErrTextSize))
				{
					TRACE(_T("EXCEPTION: CXTPReportControl::Paste() - %s\n"), szErrText);
				}
				pE->Delete();
			}
			catch(CFileException* pE)
			{
				if (pE->GetErrorMessage(szErrText, cErrTextSize))
				{
					TRACE(_T("EXCEPTION: CXTPReportControl::Paste() - %s\n"), szErrText);
				}
				pE->Delete();
			}
			catch(...)
			{
				TRACE(_T("EXCEPTION: CXTPReportControl::Paste() - Unhandled Exception!\n"));
			}

			//*********
			ar.Close(); // detach from file
			fileSahred.Detach(); //detach from data
			::GlobalUnlock(hPasteData); // unlock data
		}
	}

	UINT uCF_TText = sizeof(TCHAR) == 2 ? CF_UNICODETEXT : CF_TEXT;

	if (bTryPasteFromText && ::IsClipboardFormatAvailable(uCF_TText))
	{
		// Try to get text data from the clipboard
		HGLOBAL hglbPaste = ::GetClipboardData(uCF_TText);

		// Import Text data into the control
		if (hglbPaste != NULL)
		{
			TCHAR* lpszClipboard = (TCHAR*)GlobalLock(hglbPaste);

			if (!_ReadRecordsFromText(lpszClipboard, arRecords))
			{
				arRecords.RemoveAll();
			}
			::GlobalUnlock(hglbPaste);
		}
	}

	::CloseClipboard();

	if (IsVirtualMode())
		return;
	CUpdateContext updateContext(this);

	// Add and Populate records
	int nRecordsCount = arRecords.GetCount();
	if (nRecordsCount > 0)
	{
		// Add
		for (int i = 0; i < nRecordsCount; i++)
		{
			CXTPReportRecord* pRecord = arRecords.GetAt(i);
			if (pRecord)
			{
				CMDTARGET_ADDREF(pRecord);
				AddRecord(pRecord);
			}
		}

		Populate();

		// Select added records
		_SelectRows(&arRecords);
	}
}

// We support text format with \t dividers for record items and
// \r\n dividers for records (simple tab-separated text).
// Such format is also supported by Excel and some other applications.
CString CXTPReportControl::_GetSelectedRowsVisibleColsText()
{
	CString strSelText;

	CXTPReportColumns* pColumns = GetColumns();
	if (NULL == pColumns)
		return _T("");

	const int nColumnCount = pColumns->GetVisibleColumnsCount();

	// Iterate over the selected rows and prepare corresponding records text
	CXTPReportSelectedRows* pSelectedRows = GetSelectedRows();
	if ((pSelectedRows != NULL) && (pSelectedRows->GetCount() > 0))
	{
		POSITION pos = pSelectedRows->GetFirstSelectedRowPosition();
		while (pos)
		{
			CXTPReportRow* pRow = pSelectedRows->GetNextSelectedRow(pos);
			if (NULL == pRow)
				break;

			CXTPReportRecord* pRecord = pRow->GetRecord();
			if (NULL == pRecord)
				continue;

			CStringArray arStrings;
			for (int nCol = 0; nCol < nColumnCount; nCol++)
			{
				CXTPReportColumn* pColumn = pColumns->GetVisibleAt(nCol);
				CXTPReportRecordItem* pItem = pRecord->GetItem(pColumn);
				if (NULL == pItem)
					continue;

				arStrings.Add(pItem->GetCaption(pColumn));
			}

			BOOL bCanceled = OnBeforeCopyToText(pRecord, arStrings);
			if (bCanceled)
				continue;

			strSelText += XTPStrMake(arStrings, _T("\t"));
			strSelText += _T("\r\n");
		}
	}

	return strSelText;
}

BOOL CXTPReportControl::_ReadRecordsFromText(LPCTSTR pcszText,
											CXTPReportRecords& rarRecords)
{
	//////////////////////////////////////////////////////////////////////////
	// Insert retrieved text into the control

	CStringArray arRecordsStrings;
	XTPStrSplit(pcszText, _T("\r\n"), arRecordsStrings);

	int nCount = (int) arRecordsStrings.GetSize();
	for (int i = 0; i < nCount; i++)
	{
		CString strRecord = arRecordsStrings[i];
		if (strRecord.IsEmpty())
			continue;

		CXTPReportRecord* pNewRec = _CreateRecodFromText(strRecord);
		// Case: pNewRec = NULL - paste was handled by the user

		if (pNewRec)
			rarRecords.Add(pNewRec);
	}

	return TRUE;
}

CXTPReportRecord* CXTPReportControl::_CreateRecodFromText(LPCTSTR pcszRecord)
{
	if (!GetColumns())
	{
		ASSERT(FALSE);
		return NULL;
	}

	CStringArray arStrings;

	// Read each field from the initial string and set visible field one by one
	XTPStrSplit(pcszRecord, _T("\t"), arStrings);

	CXTPReportRecord* pRecord = NULL;
	BOOL bHandled = OnBeforePasteFromText(arStrings, &pRecord);
	if (bHandled)
		return pRecord;

#if _MFC_VER >= 0x0600 // Not supported by Visual Studio 5.0

	if (!pRecord)
	{
		pRecord = new CXTPReportRecord();
		if (!pRecord)
		{
			return NULL;
		}

		int nDataCount = (int) arStrings.GetSize();

		// Fill record with all items
		int nCount = GetColumns()->GetCount();
		for (int i = 0; i < nCount; i++)
		{
			COleVariant varItem(_T(""));

			CXTPReportRecordItem* pItem = new CXTPReportRecordItemVariant(varItem);
			if (!pItem)
			{
				CMDTARGET_RELEASE(pRecord);
				return NULL;
			}
			pRecord->AddItem(pItem);
		}

		// Iterate all visible columns and set text for each next
		const int nColumnCount = GetColumns()->GetVisibleColumnsCount();
		for (int nCol = 0; nCol < nColumnCount; nCol++)
		{
			COleVariant varItem(nCol < nDataCount ? (LPCTSTR)arStrings[nCol] : _T(""));

			CXTPReportColumn* pColumn = GetColumns()->GetVisibleAt(nCol);
			CXTPReportRecordItem* pItem = pRecord->GetItem(pColumn);
			ASSERT(pItem);
			if (NULL == pItem)
				continue;

			CXTPReportRecordItemVariant* pItemVar = DYNAMIC_DOWNCAST(CXTPReportRecordItemVariant, pItem);
			ASSERT(pItemVar);

			if (pItemVar)
			{
				pItemVar->m_oleValue = varItem;
			}
		}
	}
#endif

	return pRecord;
}

BOOL CXTPReportControl::_WriteRecordsData(CXTPPropExchange* pPX, CXTPReportRecords* pRecords)
{
	if (!pRecords)
		return FALSE;

	long nSchema = XTP_REPORT_CB_RECORDS_DATA_VER;

	PX_Long(pPX, _T("Version"), (long&)nSchema);
	pPX->ExchangeLocale();


	CXTPPropExchangeSection secRecords(pPX->GetSection(_T("ReportRecords")));

	int nRecordsCount = (int)pRecords->GetCount();
	CXTPPropExchangeEnumeratorPtr pEnumRecords(secRecords->GetEnumerator(_T("Record")));
	POSITION posRecord = pEnumRecords->GetPosition((DWORD)nRecordsCount);

	for (int i = 0; i < nRecordsCount; i++)
	{
		CXTPPropExchangeSection secRecord(pEnumRecords->GetNext(posRecord));

		CXTPReportRecord* pRecord = pRecords->GetAt(i);
		PX_Object(&secRecord, pRecord, RUNTIME_CLASS(CXTPReportRecord));
	}

	return TRUE;
}

void CXTPReportControl::_SelectRows(CXTPReportRecords* pRecords)
{
	CXTPReportRows* pRows = GetRows();
	CXTPReportSelectedRows* pSelRows = GetSelectedRows();
	if (!pRows || !pSelRows)
		return;

	pSelRows->Clear();
	int nRecordsCount = pRecords->GetCount();

	for (int nNewRecNr = 0; nNewRecNr < nRecordsCount; nNewRecNr++)
	{
		CXTPReportRecord* pRec = pRecords->GetAt(nNewRecNr);
		CXTPReportRow* pRow = pRows->Find(pRec);
		if (pRow)
		{
			pSelRows->Add(pRow);

			if (nNewRecNr == nRecordsCount - 1)
				SetFocusedRow(pRow, TRUE);
		}
	}
}

BOOL CXTPReportControl::_GetSelectedRows(CXTPReportRecords* pRecords,
		CXTPInternalCollectionT<CXTPReportRow> *pRows)
{
	ASSERT(!pRecords || pRecords->m_bArray == TRUE);

	CXTPReportSelectedRows* pSelectedRows = GetSelectedRows();
	if (!pSelectedRows
		|| !pSelectedRows->GetCount()
		|| (!pRecords && !pRows))
		return FALSE;

	POSITION pos = pSelectedRows->GetFirstSelectedRowPosition();
	while (pos)
	{
		CXTPReportRow* pRow = pSelectedRows->GetNextSelectedRow(pos);
		CXTPReportRecord* pRecord = pRow ? pRow->GetRecord() : NULL;
		if (NULL == pRow)
			break;

		if (pRecord && pRecords)
			pRecords->Add(pRecord);

		if (pRow && pRows)
			pRows->AddPtr(pRow, TRUE);
	}

	return TRUE;
}

BOOL CXTPReportControl::_WriteSelectedRowsData(CXTPPropExchange* pPX)
{
	if (!pPX || !pPX->IsStoring())
	{
		ASSERT(FALSE);
		return FALSE;
	}

	CXTPReportRecords* pSelectedRecords= new CXTPReportRecords(TRUE);

	if (!_GetSelectedRows(pSelectedRecords) || !_WriteRecordsData(pPX, pSelectedRecords))
	{
		CMDTARGET_RELEASE(pSelectedRecords);
		return FALSE;
	}

	CMDTARGET_RELEASE(pSelectedRecords);
	return TRUE;
}

BOOL CXTPReportControl::_ReadRecordsFromData(CXTPPropExchange* pPX,
											CXTPReportRecords& rarRecords)
{
	rarRecords.RemoveAll();

	if (!pPX || !pPX->IsLoading())
	{
		ASSERT(FALSE);
		return FALSE;
	}

	CXTPPropExchangeSection secRecords(pPX->GetSection(_T("ReportRecords")));

	long nSchema = 0;

	PX_Long(pPX, _T("Version"), (long&)nSchema);
	pPX->ExchangeLocale();

	if (nSchema != XTP_REPORT_CB_RECORDS_DATA_VER)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	CXTPPropExchangeEnumeratorPtr pEnumRecords(secRecords->GetEnumerator(_T("Record")));
	POSITION posRecord = pEnumRecords->GetPosition();

	while (posRecord)
	{
		CXTPPropExchangeSection secRecord(pEnumRecords->GetNext(posRecord));

		CXTPReportRecord* pRecord = NULL;
		PX_Object(&secRecord, pRecord, RUNTIME_CLASS(CXTPReportRecord));

		if (!pRecord)
			AfxThrowArchiveException(CArchiveException::badClass);

		CXTPReportRecord* pRecord2 = pRecord;
		BOOL bCanceled = OnBeforePaste(&pRecord2);

		if (bCanceled || pRecord2 != pRecord)
			CMDTARGET_RELEASE(pRecord);

		if (bCanceled)
			continue;

		rarRecords.Add(pRecord2);
	}

	return TRUE;
}

BOOL CXTPReportControl::OnBeforeCopyToText(CXTPReportRecord* pRecord, CStringArray& rarStrings)
{
	XTP_NM_REPORT_BEFORE_COPYPASTE nmParams;
	::ZeroMemory(&nmParams, sizeof(nmParams));

	CXTPReportRecord* pRecordTmp = pRecord;
	nmParams.ppRecord = &pRecordTmp;
	nmParams.parStrings = &rarStrings;

	LRESULT lResult = SendNotifyMessage(XTP_NM_REPORT_BEFORE_COPY_TOTEXT, (NMHDR*)&nmParams);

	return lResult != 0;
}

BOOL CXTPReportControl::OnBeforePasteFromText(CStringArray& arStrings,
	CXTPReportRecord** ppRecord)
{
	XTP_NM_REPORT_BEFORE_COPYPASTE nmParams;
	::ZeroMemory(&nmParams, sizeof(nmParams));

	nmParams.parStrings = &arStrings;
	nmParams.ppRecord = ppRecord;

	LRESULT lResult = SendNotifyMessage(XTP_NM_REPORT_BEFORE_PASTE_FROMTEXT, (NMHDR*)&nmParams);

	return lResult != 0;
}

BOOL CXTPReportControl::OnBeforePaste(CXTPReportRecord** ppRecord)
{
	XTP_NM_REPORT_BEFORE_COPYPASTE nmParams;
	::ZeroMemory(&nmParams, sizeof(nmParams));

	nmParams.ppRecord = ppRecord;

	LRESULT lResult = SendNotifyMessage(XTP_NM_REPORT_BEFORE_PASTE, (NMHDR*)&nmParams);

	return lResult != 0;
}

int CXTPReportControl::OnGetColumnDataBestFitWidth(CXTPReportColumn* pColumn)
{
	CXTPReportPaintManager* pPaintManager = GetPaintManager();
	if (!pColumn || !pPaintManager)
	{
		ASSERT(FALSE);
		return 0;
	}
	int nDataWidth = 0;
	int nDataWidth0 = 0;
	int nDataWidth1 = 0;

	CXTPClientRect rcClient(this);
	CBitmap bmp;
	{
		CClientDC dcClient(this);
		bmp.CreateCompatibleBitmap(&dcClient, rcClient.Width(), rcClient.Height());
	}

	CXTPCompatibleDC dc(NULL, &bmp);

	CXTPFontDC autoFont(&dc, pPaintManager->GetTextFont()); // to reset selected font on exit

	if (pColumn->GetBestFitMode() == xtpColumnBestFitModeVisibleData
		|| (pColumn->GetBestFitMode() == xtpColumnBestFitModeAllData && IsVirtualMode())
		/* && pColumn->m_nMaxItemWidth == 0*/)
	{
		// calculate width for visible rows only
		int nVisibleRows = GetReportAreaRows(GetTopRowIndex(), TRUE);

		nDataWidth = OnGetItemsCaptionMaxWidth(&dc, GetRows(), pColumn,
			GetTopRowIndex(), nVisibleRows);
	}
	else if (pColumn->GetBestFitMode() == xtpColumnBestFitModeAllData && !IsVirtualMode())
		nDataWidth = OnGetItemsCaptionMaxWidth(&dc, GetRows(), pColumn);
	else
		return 0;

	if (m_bHeaderRecordsVisible && m_pHeaderRows->GetCount() > 0)
		nDataWidth0 = OnGetItemsCaptionMaxWidth(&dc, m_pHeaderRows, pColumn);

	if (m_bFooterRecordsVisible && m_pFooterRows->GetCount() > 0)
		nDataWidth1 = OnGetItemsCaptionMaxWidth(&dc, m_pFooterRows, pColumn);

	nDataWidth = max(nDataWidth, max(nDataWidth0, nDataWidth1));

	return nDataWidth;
}

int CXTPReportControl::OnGetItemsCaptionMaxWidth(CDC* pDC, CXTPReportRows* pRows,
													CXTPReportColumn* pColumn, int nStartRow, int nRowsCount)
{
	CXTPReportPaintManager* pPaintManager = GetPaintManager();
	if (!pDC || !pRows || !pColumn || !pPaintManager)
	{
		ASSERT(FALSE);
		return 0;
	}

	XTP_REPORTRECORDITEM_DRAWARGS drawArgs;
	drawArgs.pDC = pDC;
	drawArgs.pControl = this;
	drawArgs.pColumn = pColumn;
	drawArgs.rcItem = pColumn->GetRect();


	XTP_REPORTRECORDITEM_METRICS* pMetrics = new XTP_REPORTRECORDITEM_METRICS();

	int nGroupIndent = 0;
	if (GetColumns()->GetVisibleAt(0) == pColumn)
		nGroupIndent = GetHeaderIndent();

	CSize sizeBitmap(0, 0);
	int nMaxWidth = 0;
	int nItemIndex = pColumn->GetItemIndex();

	int nEndRow = pRows->GetCount();
	if (nRowsCount > 0)
		nEndRow = min(pRows->GetCount(), nStartRow + nRowsCount);

	for (int i = max(0, nStartRow); i < nEndRow; i++)
	{
		CXTPReportRow* pRow = pRows->GetAt(i);
		if (!pRow)
			continue;

		if (pRow && pRow->IsGroupRow())
			continue;

		CXTPReportRecord* pRec = pRow ? pRow->GetRecord() : NULL;
		CXTPReportRecordItem* pItem = pRec ? pRec->GetItem(nItemIndex) : NULL;

		if (!pItem)
			continue;

		// 1. Calculate Text
		drawArgs.pRow = pRow;
		drawArgs.pItem = pItem;

		pMetrics->strText = pItem->GetCaption(pColumn);

		pRow->GetItemMetrics(&drawArgs, pMetrics);

		pDC->SelectObject(pMetrics->pFont);

		int nWidth = 0;

		if (pItem->GetMarkupUIElement())
		{
			nWidth = XTPMarkupMeasureElement(pItem->GetMarkupUIElement(), INT_MAX, 0).cx + 4;
		}
		else
		{
			CString sFirst(pMetrics->strText);
			CString sLast;
			int jPos = sFirst.Find(_T('\n'), 0);
			int wd(0);
			while (jPos > -1)
			{
				sLast = sFirst.Mid(jPos + 1);
				sFirst = sFirst.Left(jPos - 1);
				wd = max(wd, pDC->GetTextExtent(sFirst).cx + 7);
				sFirst = sLast;
				jPos = sFirst.Find(_T('\n'), 0);
			}
			if (!sFirst.IsEmpty())
				nWidth = max(wd, pDC->GetTextExtent(sFirst).cx + 7);

//          nWidth = pDC->GetTextExtent(pMetrics->strText).cx + 7;
		}

		// 2. Calculate Tree Indent
		if (pColumn->IsTreeColumn())
		{
			int nTreeDepth = pRow->GetTreeDepth() - pRow->GetGroupLevel();
			if (nTreeDepth > 0)
				nTreeDepth++;
			nWidth += GetIndent(nTreeDepth);

			if (sizeBitmap.cx == 0 && sizeBitmap.cy == 0)
			{
				CRect rcBmp(0, 0, 100, 100);
				sizeBitmap = pPaintManager->DrawCollapsedBitmap(NULL, pRow, rcBmp);
			}

			nWidth += sizeBitmap.cx + 2;
		}

		// 2.1 Calculate Group Indent
		nWidth += nGroupIndent;

		// 3. Calculate item Icon
		int nIcon = pMetrics->nItemIcon;
		nIcon = (nIcon != XTP_REPORT_NOICON) ? nIcon : pItem->GetIconIndex();
		if (nIcon != XTP_REPORT_NOICON)
		{
			CXTPImageManagerIcon* pIcon = GetImageManager()->GetImage(nIcon, 0);
			if (pIcon)
				nWidth += pIcon->GetWidth() + 4;
		}

		if (pItem->GetHasCheckbox())
		{
			nWidth += 16;
		}

		// 4. Calculate item Controls
		if (pItem->m_pItemControls)
		{
			int nControlsCount = pItem->m_pItemControls->GetSize();
			for (int k = 0; k < nControlsCount; k++)
			{
				CXTPReportRecordItemControl* pCtrl = pItem->m_pItemControls->GetAt(k);
				int nControlWidth = 0;
				if (pCtrl)
					nControlWidth = pCtrl->GetSize().cx;
				if (nControlWidth < 0)
					nControlWidth = 20;

				nWidth += nControlWidth;
			}
		}

		nMaxWidth = max(nMaxWidth, nWidth);
	}

	CMDTARGET_RELEASE(pMetrics);

	return nMaxWidth;
}

BOOL CXTPReportControl::IsEditMode()
{
	CXTPReportInplaceEdit* pEdit = GetInplaceEdit();

	BOOL bEditMode = pEdit && pEdit->GetSafeHwnd() && pEdit->IsWindowVisible();
	bEditMode |= GetInplaceList() && GetInplaceList()->GetSafeHwnd() && GetInplaceList()->IsWindowVisible();

	return bEditMode;
}


int CXTPReportControl::GetRowsHeight(CXTPReportRows* pRows, int nTotalWidth, int nMaxHeight)
{
	int nRowsHeight = 0;

	CWindowDC dc (this);

	for (int i = 0; i < pRows->GetCount(); ++i)
	{
		nRowsHeight += GetPaintManager()->GetRowHeight(&dc, pRows->GetAt(i), nTotalWidth);

		if (nMaxHeight >= 0 && nRowsHeight > nMaxHeight)
			return nRowsHeight;
	}

	return nRowsHeight;
}

int CXTPReportControl::GetHeaderRowsDividerHeight()
{
	return GetPaintManager()->GetHeaderRowsDividerHeight();
}

int CXTPReportControl::GetFooterRowsDividerHeight()
{
	return GetPaintManager()->GetFooterRowsDividerHeight();
}

void CXTPReportControl::DrawDefaultGrid(CDC* pDC, CRect rcClient, int nRowHeight, int nLeftOffset)
{
	if (nRowHeight <= 0)
		return;

	int nFreezeCols = m_nFreezeColumnsCount;
	CRect rcClipBox = GetReportRectangle();

	CRect rcRow;
	rcRow = rcClient;
	rcRow.left -= nLeftOffset;
	rcRow.right -= nLeftOffset;
	rcRow.bottom = rcClient.top + nRowHeight;

	int nIndentWidth = GetHeaderIndent();
	CXTPReportPaintManager* pPaintManager = GetPaintManager();

	CXTPReportColumns arrVisibleColumns(this);
	GetColumns()->GetVisibleColumns(arrVisibleColumns);
	int nVisColCount = arrVisibleColumns.GetCount();
	nFreezeCols = min(nFreezeCols, nVisColCount);

	// fill the empty space
	while (rcRow.top < rcClient.bottom)
	{
		CRect rcItem(rcRow.left, rcRow.top, rcRow.right, rcRow.bottom);

		CRect rcIndent(nFreezeCols ? rcRow : rcRow); /////////////////////
		rcIndent.right = rcIndent.left + nIndentWidth;

		int xMinCol_0 = rcRow.left + nIndentWidth;

		for (int nColumn = nVisColCount-1; nColumn >= 0; nColumn--)
		{
			BOOL bFreezeCol = nColumn < nFreezeCols;
			int nColIdx = bFreezeCol ? nFreezeCols - 1 - nColumn : nColumn;

			CXTPReportColumn* pColumn = arrVisibleColumns.GetAt(nColIdx);
			ASSERT(pColumn && pColumn->IsVisible());

			if (pColumn)
			{
				rcItem.left = pColumn->GetRect().left;
				if (nColIdx == 0)
					rcItem.left = max(xMinCol_0, rcItem.left);

				rcItem.right = pColumn->GetRect().right;

				if (!CRect().IntersectRect(rcClipBox, rcItem))
					continue;

				if (bFreezeCol)
				{
					CRect rcFreeze(rcItem);
					rcFreeze.top +=1;
					pDC->FillSolidRect(rcFreeze, pPaintManager->GetControlBackColor(this));
				}

				CRect rcGridItem(rcItem);
				rcGridItem.left--;

				pPaintManager->DrawGrid(pDC, FALSE, rcGridItem);
				pPaintManager->DrawGrid(pDC, TRUE, rcGridItem);

				if (nColIdx == nFreezeCols - 1)
					pPaintManager->DrawFreezeColsDivider(pDC, rcGridItem, this);
			}
		}

		if (nIndentWidth > 0) // draw indent column
			pPaintManager->FillIndent(pDC, rcIndent);

		rcRow.top += nRowHeight;
		rcRow.bottom += nRowHeight;
	}
}


BOOL CXTPReportControl::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		if (!OnPreviewKeyDown((UINT&)pMsg->wParam, LOWORD(pMsg->lParam), HIWORD(pMsg->lParam)) )
		{
			return TRUE;
		}
	}

	return CWnd::PreTranslateMessage(pMsg);
}

BOOL CXTPReportControl::OnConstraintSelecting(CXTPReportRow* pRow, CXTPReportRecordItem* pItem, CXTPReportColumn* pColumn,
		CXTPReportRecordItemConstraint* pConstraint)
{
	XTP_NM_REPORTCONSTRAINTSELECTING    nmConstraint;
	::ZeroMemory(&nmConstraint, sizeof(nmConstraint));

	nmConstraint.pRow = pRow;
	nmConstraint.pColumn = pColumn;
	nmConstraint.pItem = pItem;
	nmConstraint.pConstraint = pConstraint;

	LRESULT lResult = SendNotifyMessage(XTP_NM_REPORT_CONSTRAINT_SELECTING, (NMHDR*)&nmConstraint);

	return lResult != 0;
}

const XTP_NM_REPORTTOOLTIPINFO& CXTPReportControl::OnGetToolTipInfo(CXTPReportRow* pRow, CXTPReportRecordItem* pItem, CString& rstrToolTipText)
{
	::ZeroMemory(m_pCachedToolTipInfo, sizeof(XTP_NM_REPORTTOOLTIPINFO));

	m_pCachedToolTipInfo->pRow = pRow;
	m_pCachedToolTipInfo->pItem = pItem;
	m_pCachedToolTipInfo->pstrText = &rstrToolTipText;

	SendNotifyMessage(XTP_NM_REPORT_GETTOOLTIPINFO, (NMHDR*)m_pCachedToolTipInfo);

	return *m_pCachedToolTipInfo;
}

CXTPReportDataManager* CXTPReportControl::GetDataManager()
{
	if (!m_pDataManager)
		m_pDataManager = new CXTPReportDataManager(this);

	return m_pDataManager;
}

CRect CXTPReportControl::GetElementRect(int nElement) const
{
	switch (nElement)
	{
		case xtpReportElementRectGroupByArea : return m_rcGroupByArea; break;
		case xtpReportElementRectHeaderArea : return m_rcHeaderArea; break;
		case xtpReportElementRectFooterArea : return m_rcFooterArea; break;
		case xtpReportElementRectHeaderRecordsArea : return m_rcHeaderRecordsArea; break;
		case xtpReportElementRectFooterRecordsArea : return m_rcFooterRecordsArea; break;
		case xtpReportElementRectHeaderRecordsDividerArea : return m_rcHeaderRecordsDividerArea; break;
		case xtpReportElementRectFooterRecordsDividerArea : return m_rcFooterRecordsDividerArea; break;
		default : return m_rcReportArea; break;
	}
}

CXTPMarkupContext* CXTPReportControl::GetMarkupContext() const
{
	return m_pRecords->GetMarkupContext();
}

void CXTPReportControl::EnableMarkup(BOOL bEnable)
{
	m_bMarkupEnabled = bEnable;

	BOOL bOldEnable = GetMarkupContext() != NULL;
	if (bOldEnable == bEnable)
		return;

	XTPMarkupReleaseContext(m_pRecords->m_pMarkupContext);
	m_pHeaderRecords->m_pMarkupContext = NULL;
	m_pFooterRecords->m_pMarkupContext = NULL;

	if (bEnable)
	{
		m_pRecords->m_pMarkupContext = XTPMarkupCreateContext(m_hWnd);
		m_pHeaderRecords->m_pMarkupContext = m_pRecords->m_pMarkupContext;
		m_pFooterRecords->m_pMarkupContext = m_pRecords->m_pMarkupContext;
	}
}

LRESULT CXTPReportControl::OnGetObject(WPARAM wParam, LPARAM lParam)
{
	if (lParam != OBJID_CLIENT)
		return (LRESULT)Default();

	LPUNKNOWN lpUnknown = GetInterface(&IID_IAccessible);
	if (!lpUnknown)
		return E_FAIL;

	return LresultFromObject(IID_IAccessible, wParam, lpUnknown);
}

BEGIN_INTERFACE_MAP(CXTPReportControl, CCmdTarget)
	INTERFACE_PART(CXTPReportControl, IID_IAccessible, ExternalAccessible)
END_INTERFACE_MAP()

CCmdTarget* CXTPReportControl::GetAccessible()
{
	return this;
}

HRESULT CXTPReportControl::GetAccessibleParent(IDispatch* FAR* ppdispParent)
{
	*ppdispParent = NULL;

	if (GetSafeHwnd())
	{
		return AccessibleObjectFromWindow(GetSafeHwnd(), OBJID_WINDOW, IID_IDispatch, (void**)ppdispParent);
	}
	return E_FAIL;
}

HRESULT CXTPReportControl::GetAccessibleChildCount(long FAR* pChildCount)
{
	if (pChildCount == 0)
	{
		return E_INVALIDARG;
	}

	*pChildCount = 1 + GetRows()->GetCount();
	return S_OK;
}

HRESULT CXTPReportControl::GetAccessibleChild(VARIANT varChild, IDispatch* FAR* ppdispChild)
{
	*ppdispChild = NULL;
	int nChild = GetChildIndex(&varChild);

	if (nChild <= 0)
	{
		return E_INVALIDARG;
	}

	if (nChild == 1)
	{
		*ppdispChild = GetReportHeader()->GetIDispatch(TRUE);
		return S_OK;
	}

	CXTPReportRow* pRow = GetRows()->GetAt(nChild - 2);
	if (!pRow)
	{
		return E_INVALIDARG;
	}

	*ppdispChild = pRow->GetIDispatch(TRUE);
	return S_OK;
}

HRESULT CXTPReportControl::GetAccessibleName(VARIANT varChild, BSTR* pszName)
{
	int nChild = GetChildIndex(&varChild);

	if (nChild == CHILDID_SELF || nChild == -1)
	{
		CString strCaption;
		GetWindowText(strCaption);
		if (strCaption.IsEmpty()) strCaption = _T("Report");
		*pszName = strCaption.AllocSysString();
		return S_OK;
	}

	return E_INVALIDARG;
}

HRESULT CXTPReportControl::GetAccessibleRole(VARIANT varChild, VARIANT* pvarRole)
{
	pvarRole->vt = VT_EMPTY;
	int nChild = GetChildIndex(&varChild);

	if (nChild == CHILDID_SELF)
	{
		pvarRole->vt = VT_I4;
		pvarRole->lVal = ROLE_SYSTEM_TABLE;
		return S_OK;
	}

	return E_INVALIDARG;
}

HRESULT CXTPReportControl::GetAccessibleState(VARIANT varChild, VARIANT* pvarState)
{
	pvarState->vt = VT_I4;
	pvarState->lVal = 0;
	int nChild = GetChildIndex(&varChild);

	if (nChild == CHILDID_SELF)
	{
		pvarState->lVal = STATE_SYSTEM_FOCUSABLE;
	}

	return S_OK;
}


HRESULT CXTPReportControl::AccessibleLocation(long* pxLeft, long* pyTop, long* pcxWidth, long* pcyHeight, VARIANT varChild)
{
	*pxLeft = *pyTop = *pcxWidth = *pcyHeight = 0;

	if (!GetSafeHwnd())
		return S_OK;

	CRect rc;
	GetWindowRect(&rc);

	int nChild = GetChildIndex(&varChild);

	if (nChild == 1)
	{
		rc = m_rcHeaderArea;
		ClientToScreen(&rc);
	}

	if (nChild > 1)
	{
		CXTPReportRow* pRow = GetRows()->GetAt(nChild - 2);
		if (pRow)
		{
			rc = pRow->GetRect();
			ClientToScreen(&rc);
		}

	}

	*pxLeft = rc.left;
	*pyTop = rc.top;
	*pcxWidth = rc.Width();
	*pcyHeight = rc.Height();

	return S_OK;
}

HRESULT CXTPReportControl::AccessibleHitTest(long xLeft, long yTop, VARIANT* pvarID)
{
	if (pvarID == NULL)
		return E_INVALIDARG;

	pvarID->vt = VT_EMPTY;

	if (!GetSafeHwnd())
		return S_FALSE;

	if (!CXTPWindowRect(this).PtInRect(CPoint(xLeft, yTop)))
		return S_FALSE;

	pvarID->vt = VT_I4;
	pvarID->lVal = 0;

	CPoint pt(xLeft, yTop);
	ScreenToClient(&pt);

	if (m_rcHeaderArea.PtInRect(pt))
	{
		pvarID->vt = VT_DISPATCH;
		pvarID->pdispVal = GetReportHeader()->GetIDispatch(TRUE);
		return S_OK;
	}

	CXTPReportRow* pRow = HitTest(pt);
	if (pRow)
	{
		pvarID->vt = VT_DISPATCH;
		pvarID->pdispVal = pRow->GetIDispatch(TRUE);
		return S_OK;
	}

	return S_FALSE;
}

void CXTPReportControl::EnsureStopLastRqstTimer()
{
	if (m_uRqstEditTimer)
	{
		KillTimer(m_uRqstEditTimer);

		m_uRqstEditTimer = 0;
		m_iLastRqstEditRow = -1;
		m_iLastRqstEditCol = -1;
	}
}

// Information for delay editing.
void CXTPReportControl::EnsureStopDelayEditTimer()
{
	if (m_uiDelayEditTimer)
	{
		KillTimer(m_uiDelayEditTimer);

		m_uiDelayEditTimer = NULL;
	}
}

void CXTPReportControl::StartLastRqstTimer()
{
	EnsureStopLastRqstTimer();

	m_uRqstEditTimer = SetTimer(XTP_REPORT_DELAY_CLICK_TIMER_ID, 3000, NULL);   // After 3 seconds, ignore this click.
}

// Information for delay editing.
void CXTPReportControl::StartDelayEditTimer()
{
	EnsureStopDelayEditTimer();

	m_uiDelayEditTimer = SetTimer(XTP_REPORT_DELAYEDIT_CLICK_TIMER_ID, m_uiDelayEditMaxTime, NULL);  // Begin edit after a full double click is possible.
}

void CXTPReportControl::SetSelectionState(int index, int state)
{
	CXTPReportSelectedRows* pSelectedRows = GetSelectedRows();
	if (!pSelectedRows)
		return;
	CXTPReportRecordItem* pItem = NULL;
	CXTPReportRecord* pRec = NULL;
	for (int i = 0; i < pSelectedRows->GetCount(); i++)
	{
		if (pSelectedRows->GetAt(i))
		{
			pRec = pSelectedRows->GetAt(i)->GetRecord();
			if (pRec)
			{
				if (index < pRec->GetItemCount())
				{
					pItem = pRec->GetItem(index);
					if (pItem)
					{
						pItem->SetChecked(state == 1);
						pItem->SetCheckedState(state);
					}
				}
			}
		}
	}
	RedrawControl();
}

void CXTPReportControl::Recalc(BOOL bAll)
{
	EditItem(NULL);

	if (!bAll && GetFocusedRow())
	{
		CXTPReportRecord* pRec = GetFocusedRow()->GetRecord();
		if (pRec)
		{
			int N = pRec->GetItemCount();
			CXTPReportRecordItem* pItem = NULL;
			for (int i = 0; i < N; i++)
			{
				pItem = pRec->GetItem(i);
				if (pItem)
				{
					if (!pItem->GetFormula().IsEmpty())
						pItem->SetCaption(_T("x")); //it will call recalc for item
				}
			}
			RedrawControl();
		}
	}
	else if (GetRecords())
	{
		int n = GetRecords()->GetCount();
		for (int j = 0; j < n; j++)
		{
			CXTPReportRecord* pRec = GetRecords()->GetAt(j);
			if (pRec)
			{
				int N = pRec->GetItemCount();
				CXTPReportRecordItem* pItem = NULL;
				for (int i = 0; i < N; i++)
				{
					pItem = pRec->GetItem(i);
					if (pItem)
					{
						if (!pItem->GetFormula().IsEmpty())
							pItem->SetCaption(_T("x"));
					}
				}
				RedrawControl();
			}
		}
		int N = GetRows()->GetCount();
		for (int I = 0; I < N; I++)
		{
			CXTPReportRow* pTRow = GetRows()->GetAt(I);
			if (pTRow && pTRow->IsGroupRow())
			{
				CXTPReportGroupRow* pTgRow = (CXTPReportGroupRow*) pTRow;
				if (!pTgRow->GetFormula().IsEmpty())
					pTgRow->SetCaption(_T("x"));
			}
		}
		RedrawControl();
	}
}

BOOL CXTPReportControl::IsShowRowNumber()
{
	if (m_iColumnForNum > -1)
	{
		if (m_bUseIconColumnForNum)
		{
			CXTPReportColumn* pIconCol = GetColumns()->Find(m_iColumnForNum);
			if (pIconCol)
				return pIconCol->IsVisible();
		}
	}
	return FALSE;
}

void CXTPReportControl::ShowRowNumber(BOOL bSet)
{
	if (bSet)
	{
		if (m_iColumnForNumPrev > -1)
			m_iColumnForNum = m_iColumnForNumPrev;
		if (m_iColumnForNum > -1)
		{
			if (m_bUseIconColumnForNum)
			{
				CXTPReportColumn* pIconCol = GetColumns()->Find(m_iColumnForNum);
				if (pIconCol)
					pIconCol->SetVisible(TRUE);
			}
		}
	}
	else
	{
		m_iColumnForNumPrev = m_iColumnForNum;
		if (m_iColumnForNum > -1)
		{
			if (m_bUseIconColumnForNum && !m_bIconView)
			{
				CXTPReportColumn* pIconCol = GetColumns()->Find(m_iColumnForNum);
				if (pIconCol)
					pIconCol->SetVisible(FALSE);
			}
		}
		m_iColumnForNum = -1;
	}
	RedrawControl();
}

void CXTPReportControl::SetFreezeColumnsCount(int nCount, BOOL bAdjust /*FALSE*/)
{
	if (nCount >= 0 && nCount < m_pColumns->GetCount())
	{
		BOOL bCallView = FALSE;
		if (GetPaintManager() != NULL && GetPaintManager()->m_bMoveScrollbarOnFixedColumnsIndent)
			bCallView = TRUE;

		m_nFreezeColumnsCount = nCount;
		if (IsFullColumnScrolling())
			SetFullColumnScrolling(IsFullColumnScrolling());
		else if (bCallView && m_hWnd != NULL)
		{
			CXTPReportControl::OnHScroll(SB_TOP, 0, NULL);

			RedrawControl();
			UpdateWindow();

			AdjustScrollBars();
		}

		if (bCallView)
		{
			CWnd* pParent = GetParent();
			if (pParent)
			{
				CRect rc;
				pParent->GetWindowRect(&rc);
				int w = rc.Width();
				int h = rc.Height();
				if (bAdjust)
					h -= 4;
				if (DYNAMIC_DOWNCAST(CView, pParent))
					((CView*) pParent)->SendMessage(WM_SIZE, 0, MAKELPARAM(w, h));
					//pParent->SetWindowPos(NULL, 0, 0, rc.Width(), rc.Height(), SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED);
			}
		}
	}
	else
		m_nFreezeColumnsCount = 0;
}

void CXTPReportControl::SetDisableReorderColumnsCount(int nCount)
{
	if (nCount >= 0 && nCount < m_pColumns->GetCount())
		m_nDisableReorderColumnsCount = nCount;
	else
		m_nDisableReorderColumnsCount = 0;
}

CXTPReportRecordItem* CXTPReportControl::GetFocusedRecordItem() const
{
	CXTPReportColumn* pCol = GetFocusedColumn();
	CXTPReportRow* pRow = GetFocusedRow();

	if (pCol && pRow && pRow->GetRecord())
		return pRow->GetRecord()->GetItem(pCol);

	return NULL;
}

BOOL CXTPReportControl::SetCellText(int row, int col, CString sText)
{
	CXTPReportRow* pRow = GetRows()->GetAt(row);
	if (pRow && pRow->GetRecord())
	{
		CXTPReportRecordItem* pItem = pRow->GetRecord()->GetItem(col);
		if (pItem)
		{
			if (pItem->IsKindOf(RUNTIME_CLASS(CXTPReportRecordItemText)))
				((CXTPReportRecordItemText*) pItem)->SetValue(sText);
			pItem->SetCaption(sText);
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CXTPReportControl::SetCellFormula(int row, int col, CString sFormula)
{
	CXTPReportRow* pRow = GetRows()->GetAt(row);
	if (pRow && pRow->GetRecord())
	{
		CXTPReportRecordItem* pItem = pRow->GetRecord()->GetItem(col);
		if (pItem)
		{
			pItem->SetFormula(sFormula); //e.g. "SUM(R0*C2:R1*C5)"
			pItem->SetFormatString(_T("%d"));
			pItem->SetCaption(_T("x"));
			pItem->SetEditable(FALSE);
			return TRUE;
		}
	}
	return FALSE;
}

void CXTPReportControl::UnselectGroupRows()
{
	if (IsSkipGroupsFocusEnabled() && !IsVirtualMode())
	{
		for (int II = 0; II < GetRows()->GetCount(); II++)
		{
			CXTPReportRow* pRow = GetRows()->GetAt(II);
			if (pRow)
			{
				if (pRow->IsGroupRow())
					pRow->SetSelected(FALSE);
			}
		}
	}
}

BOOL CXTPReportControl::IsFullColumnScrolling() const
{
//<<Sep 04>> we can keep passed flag but use it only if no autosizing
	if (!GetReportHeader())
		return FALSE;
	else if (GetReportHeader()->IsAutoColumnSizing())
		return FALSE;
	return m_bFullColumnScrolling;

	//else if (!GetReportHeader()->IsAutoColumnSizing())
	//  return (m_bFullColumnScrolling > 0);
	//else
	//  return FALSE;
}


//----------------------------------------------------------------------
IMPLEMENT_DYNAMIC(CXTPPromptDlg, CDialog)

CXTPPromptDlg::CXTPPromptDlg(CWnd* pParent) : CDialog()
{
	UNREFERENCED_PARAMETER(pParent);
}

CXTPPromptDlg::~CXTPPromptDlg()
{
}

INT_PTR CXTPPromptDlg::DoModal()
{
	BYTE DlgTempl[]=
	{
		0X01, 0X00, 0XFF, 0XFF, 0X00, 0X00, 0X00, 0X00, 0X80, 0X00, 0X00, 0X00, 0X48, 0X00, 0XC8, 0X80,
		0X02, 0X00, 0X00, 0X00, 0X00, 0X00, 0X06, 0X01, 0X19, 0X00, 0X00, 0X00, 0X00, 0X00, 0X41, 0X00,
		0X73, 0X00, 0X73, 0X00, 0X69, 0X00, 0X67, 0X00, 0X6E, 0X00, 0X20, 0X00, 0X4D, 0X00, 0X61, 0X00,
		0X72, 0X00, 0X6B, 0X00, 0X65, 0X00, 0X72, 0X00, 0X20, 0X00, 0X4E, 0X00, 0X61, 0X00, 0X6D, 0X00,
		0X65, 0X00, 0X00, 0X00, 0X08, 0X00, 0X90, 0X01, 0X00, 0X01, 0X4D, 0X00, 0X53, 0X00, 0X20, 0X00,
		0X53, 0X00, 0X68, 0X00, 0X65, 0X00, 0X6C, 0X00, 0X6C, 0X00, 0X20, 0X00, 0X44, 0X00, 0X6C, 0X00,
		0X67, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X80, 0X10, 0X81, 0X50,
		0X02, 0X00, 0X02, 0X00, 0XCA, 0X00, 0X15, 0X00, 0XF1, 0X03, 0X00, 0X00, 0XFF, 0XFF, 0X81, 0X00,
		0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X01, 0X00, 0X01, 0X50,
		0XD0, 0X00, 0X04, 0X00, 0X32, 0X00, 0X0E, 0X00, 0X01, 0X00, 0X00, 0X00, 0XFF, 0XFF, 0X80, 0X00,
		0X4F, 0X00, 0X4B, 0X00, 0X00, 0X00, 0X00, 0X00
	};
	InitModalIndirect((LPCDLGTEMPLATE)&DlgTempl, NULL);
	return CDialog::DoModal();
}

void CXTPPromptDlg::OnOK()
{
	if (GetDlgItem(1009))
		GetDlgItem(1009)->GetWindowText(m_sName);
	CDialog::OnOK();
}

BOOL CXTPPromptDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	if (GetDlgItem(1009))
		GetDlgItem(1009)->SetWindowText(m_sName);
	return TRUE;
}
