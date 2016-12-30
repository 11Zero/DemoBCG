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

// BCGPKeyMapDlg.cpp : implementation file
//

#include "stdafx.h"

#include "BCGCBPro.h"
#include "BCGPKeyMapDlg.h"
#include "BCGPToolbarCustomize.h"
#include "BCGPMultiDocTemplate.h"
#include "BCGPLocalResource.h"
#include "BCGPToolbarButton.h"
#include "BCGPWorkspace.h"
#include "BCGPRegistry.h"
#include "bcgglobals.h"
#include "BCGPFrameWnd.h"
#include "BCGPMDIFrameWnd.h"

#ifndef BCGP_EXCLUDE_RIBBON
#include "BCGPRibbonBar.h"
#include "BCGPRibbonCategory.h"
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef _UNICODE
	#define _TCF_TEXT	CF_UNICODETEXT
#else
	#define _TCF_TEXT	CF_TEXT
#endif

static const int iColumnCommand = 0;
static const int iColumnKeys = 1;
static const int iColumnDescr = 2;

static const CString strDummyAmpSeq					= _T("\001\001");
static const CString strWindowPlacementRegSection	= _T("KeyMapWindowPlacement");
static const CString strRectKey						= _T("KeyMapWindowRect");

static int CALLBACK listCompareFunc (
	LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	// lParamSort contains a pointer to the dialog.
	// The lParam of an item is just its index.

	CBCGPKeyMapDlg* pDlg = (CBCGPKeyMapDlg*) lParamSort;
	ASSERT_VALID (pDlg);

	LVFINDINFO info;
	info.flags = LVFI_PARAM;
	
	info.lParam = lParam1;
	int iIndex1 = pDlg->m_KeymapList.FindItem (&info);
	ASSERT (iIndex1 >= 0);

	info.lParam = lParam2;
	int iIndex2 = pDlg->m_KeymapList.FindItem (&info);
	ASSERT (iIndex2 >= 0);

	CString strItem1 = pDlg->m_KeymapList.GetItemText (iIndex1, pDlg->m_nSortedCol);
	CString strItem2 = pDlg->m_KeymapList.GetItemText (iIndex2, pDlg->m_nSortedCol);

	return pDlg->m_bSortAscending ?
		strItem1.Compare (strItem2) : 
		strItem2.Compare (strItem1);
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPKeyMapDlg dialog

CBCGPKeyMapDlg::CBCGPKeyMapDlg(CFrameWnd* pWndParentFrame,
							 BOOL bEnablePrint /* = FALSE */,
							 BOOL bShowCommandsWithKeyOnly /* = FALSE */)
	:	CBCGPDialog(CBCGPKeyMapDlg::IDD, pWndParentFrame),
		m_bEnablePrint (bEnablePrint),
		m_bShowCommandsWithKeyOnly(bShowCommandsWithKeyOnly)
{
	m_pWndParentFrame = pWndParentFrame;

	//{{AFX_DATA_INIT(CBCGPKeyMapDlg)
	//}}AFX_DATA_INIT

	m_hAccelTable	= NULL;
	m_lpAccel		= NULL;
	m_nAccelSize	= 0;

	m_pDlgCust		= NULL;

	m_nSortedCol = 0;
	m_bSortAscending = TRUE;
	m_hInstDefault = NULL;

	m_bIsLocal = TRUE;

	m_pWndRibbonBar = NULL;
	m_bItemWasAdded = FALSE;

#ifndef BCGP_EXCLUDE_RIBBON
	CBCGPFrameWnd* pFrameWnd = DYNAMIC_DOWNCAST(CBCGPFrameWnd, m_pWndParentFrame);
	if (pFrameWnd != NULL)
	{
		if (pFrameWnd->GetRibbonBar()->GetSafeHwnd() != NULL && pFrameWnd->GetRibbonBar()->IsWindowVisible())
		{
			m_pWndRibbonBar = pFrameWnd->GetRibbonBar();
		}
	}
	else
	{
		CBCGPMDIFrameWnd* pMDIFrameWnd = DYNAMIC_DOWNCAST(CBCGPMDIFrameWnd, m_pWndParentFrame);
		if (pMDIFrameWnd != NULL)
		{
			if (pMDIFrameWnd->GetRibbonBar()->GetSafeHwnd() != NULL && pMDIFrameWnd->GetRibbonBar()->IsWindowVisible())
			{
				m_pWndRibbonBar = pMDIFrameWnd->GetRibbonBar();
			}
		}
	}
#endif

	EnableVisualManagerStyle (globalData.m_bUseVisualManagerInBuiltInDialogs, TRUE);
}

CBCGPKeyMapDlg::~CBCGPKeyMapDlg()
{
	if (m_pDlgCust != NULL)
	{
		delete m_pDlgCust;
	}

	if (m_lpAccel != NULL)
	{
		delete [] m_lpAccel;
	}
}

void CBCGPKeyMapDlg::DoDataExchange(CDataExchange* pDX)
{
	CBCGPDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBCGPKeyMapDlg)
	DDX_Control(pDX, IDC_BCGBARRES_ACCEL_LABEL, m_wndAccelLabel);
	DDX_Control(pDX, IDC_BCGBARRES_KEYLIST, m_KeymapList);
	DDX_Control(pDX, IDC_BCGBARRES_CATEGORY, m_wndCategoryList);
	DDX_Control(pDX, IDC_BCGBARRES_VIEW_ICON, m_wndViewIcon);
	DDX_Control(pDX, IDC_BCGBARRES_VIEW_TYPE, m_wndViewTypeList);
	DDX_Control(pDX, IDC_BCGBARRES_PRINT_KEYMAP, m_ButtonPrint);
	DDX_Control(pDX, IDC_BCGBARRES_COPY_KEYMAP, m_ButtonCopy);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CBCGPKeyMapDlg, CBCGPDialog)
	//{{AFX_MSG_MAP(CBCGPKeyMapDlg)
	ON_CBN_SELCHANGE(IDC_BCGBARRES_VIEW_TYPE, OnSelchangeViewType)
	ON_CBN_SELCHANGE(IDC_BCGBARRES_CATEGORY, OnSelchangeCategory)
	ON_BN_CLICKED(IDC_BCGBARRES_COPY_KEYMAP, OnCopy)
	ON_BN_CLICKED(IDC_BCGBARRES_PRINT_KEYMAP, OnPrint)
	ON_WM_SIZE()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGPKeyMapDlg message handlers

BOOL CBCGPKeyMapDlg::OnInitDialog() 
{
	CBCGPDialog::OnInitDialog();

	if (AfxGetMainWnd () != NULL && 
		(AfxGetMainWnd ()->GetExStyle () & WS_EX_LAYOUTRTL))
	{
		ModifyStyleEx (0, WS_EX_LAYOUTRTL);
	}

	{
		CBCGPLocalResource locaRes;

		//-----------------
		// Set dialog icon:
		//-----------------
		SetIcon ((HICON) ::LoadImage (
			AfxGetResourceHandle (),
			MAKEINTRESOURCE (IDI_BCGBARRES_HELP),
			IMAGE_ICON,
			::GetSystemMetrics (SM_CXSMICON),
			::GetSystemMetrics (SM_CYSMICON),
			LR_SHARED), FALSE);

		//---------------
		// Setup buttons:
		//---------------
		m_ButtonPrint.m_nFlatStyle = CBCGPButton::BUTTONSTYLE_FLAT;
		m_ButtonCopy.m_nFlatStyle = CBCGPButton::BUTTONSTYLE_FLAT;

		CString strTT;

		if (m_bEnablePrint)
		{
			m_ButtonPrint.SetImage (globalData.Is32BitIcons () ? 
				IDB_BCGBARRES_PRINT32 : IDB_BCGBARRES_PRINT, NULL);
			m_ButtonPrint.GetWindowText (strTT);
			m_ButtonPrint.SetWindowText (_T(""));
			m_ButtonPrint.SetTooltip (strTT);
			m_ButtonPrint.SizeToContent ();
			m_ButtonPrint.m_bDrawFocus = FALSE;
		}
		else
		{
			m_ButtonPrint.ShowWindow (SW_HIDE);
		}

		m_ButtonCopy.SetImage (globalData.Is32BitIcons () ?
			IDB_BCGBARRES_COPY32 : IDB_BCGBARRES_COPY, NULL);
		m_ButtonCopy.GetWindowText (strTT);
		m_ButtonCopy.SetWindowText (_T(""));
		m_ButtonCopy.SetTooltip (strTT);
		m_ButtonCopy.SizeToContent ();
		m_ButtonCopy.m_bDrawFocus = FALSE;

		//-------------
		// Add columns:
		//-------------
		OnSetColumns ();
		SetColumnsWidth ();
	}

	//-------------------------------------------------------------
	// Find all application document templates and fill accelerator
	// tables  combobox by document template data:
	//-------------------------------------------------------------
	CDocManager* pDocManager = AfxGetApp ()->m_pDocManager;
	if (pDocManager != NULL)
	{
		//---------------------------------------
		// Walk all templates in the application:
		//---------------------------------------
		for (POSITION pos = pDocManager->GetFirstDocTemplatePosition (); pos != NULL;)
		{
			CBCGPMultiDocTemplate* pTemplate = 
				(CBCGPMultiDocTemplate*) pDocManager->GetNextDocTemplate (pos);
			ASSERT_VALID (pTemplate);
			ASSERT_KINDOF (CDocTemplate, pTemplate);

			//-----------------------------------------------------
			// We are interessing CBCGPMultiDocTemplate objects with
			// the shared menu only....
			//-----------------------------------------------------
			if (!pTemplate->IsKindOf (RUNTIME_CLASS (CMultiDocTemplate)) ||
				pTemplate->m_hAccelTable == NULL)
			{
				continue;
			}

			CString strName;
			pTemplate->GetDocString (strName, CDocTemplate::fileNewName);

			int iIndex = m_wndViewTypeList.AddString (strName);
			m_wndViewTypeList.SetItemData (iIndex, (DWORD_PTR) pTemplate);
		}
	}

	//--------------------------
	// Add a default application:
	//--------------------------
	CFrameWnd* pWndMain = DYNAMIC_DOWNCAST (CFrameWnd, m_pWndParentFrame);
	if (pWndMain != NULL && pWndMain->m_hAccelTable != NULL)
	{
		CBCGPLocalResource locaRes;

		CString strName;
		strName.LoadString (IDS_BCGBARRES_DEFAULT_VIEW);

		int iIndex = m_wndViewTypeList.AddString (strName);
		m_wndViewTypeList.SetItemData (iIndex, (DWORD_PTR) NULL);

		m_wndViewTypeList.SetCurSel (iIndex);
		OnSelchangeViewType();
	}

	m_KeymapList.SetExtendedStyle (LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	OnSelchangeViewType ();

	//---------------------------------
	// Initialize commands by category:
	//---------------------------------	
	if (m_pWndRibbonBar == NULL)
	{
		m_pDlgCust = new CBCGPToolbarCustomize(m_pWndParentFrame, TRUE);
		m_pDlgCust->EnableUserDefinedToolbars();
		m_pDlgCust->FillCategoriesComboBox (m_wndCategoryList);
	}
#ifndef BCGP_EXCLUDE_RIBBON
	else
	{
		CBCGPRibbonCategory* pMainCategory = m_pWndRibbonBar->GetMainCategory ();
		if (pMainCategory != NULL)
		{
			ASSERT_VALID (pMainCategory);
			m_wndCategoryList.AddString (pMainCategory->GetName ());
		}
		
		for (int i = 0; i < m_pWndRibbonBar->GetCategoryCount (); i++)
		{
			m_wndCategoryList.AddString(m_pWndRibbonBar->GetCategory (i)->GetName ());
		}
	}
#endif

	m_wndCategoryList.SetCurSel (0);
	OnSelchangeCategory ();

	//----------------------------------
	// Restore window position and size:
	//----------------------------------
	if (GetWorkspace () != NULL)
	{
		CBCGPRegistrySP regSP;
		CBCGPRegistry& reg = regSP.Create (FALSE, TRUE);

		CRect rectPosition;

		if (reg.Open (GetWorkspace ()->GetRegSectionPath (strWindowPlacementRegSection)) &&
			reg.Read (strRectKey, rectPosition))
		{
			MoveWindow (rectPosition);
		}
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
//*************************************************************************************
void CBCGPKeyMapDlg::OnSelchangeViewType() 
{
	m_hAccelTable = NULL;

	if (m_lpAccel != NULL)
	{
		delete [] m_lpAccel;
		m_lpAccel = NULL;
	}

	int iIndex = m_wndViewTypeList.GetCurSel ();
	if (iIndex == CB_ERR)
	{
		m_wndViewIcon.SetIcon (NULL);
		return;
	}

	HICON hicon = NULL;

	CBCGPMultiDocTemplate* pTemplate = 
			(CBCGPMultiDocTemplate*) m_wndViewTypeList.GetItemData (iIndex);
	if (pTemplate != NULL)
	{
		ASSERT_VALID (pTemplate);

		hicon = ::LoadIcon (m_hInstDefault, MAKEINTRESOURCE (pTemplate->GetResId ()));
		m_hAccelTable = pTemplate->m_hAccelTable;
	}
	else
	{
		CFrameWnd* pWndMain = DYNAMIC_DOWNCAST (CFrameWnd, m_pWndParentFrame);
		if (pWndMain != NULL)
		{
			hicon = (HICON)(LONG_PTR) GetClassLongPtr (*pWndMain, GCLP_HICON);
			m_hAccelTable = pWndMain->m_hAccelTable;
		}
	}

	if (hicon == NULL)
	{
		hicon = ::LoadIcon(NULL, IDI_APPLICATION);
	}

	m_wndViewIcon.SetIcon (hicon);

	ASSERT (m_hAccelTable != NULL);

	m_nAccelSize = ::CopyAcceleratorTable (m_hAccelTable, NULL, 0);

	m_lpAccel = new ACCEL [m_nAccelSize];
	ASSERT (m_lpAccel != NULL);

	::CopyAcceleratorTable (m_hAccelTable, m_lpAccel, m_nAccelSize);
	OnSelchangeCategory ();
}
//*************************************************************************************
void CBCGPKeyMapDlg::OnSelchangeCategory() 
{
	UpdateData ();

	ASSERT (m_lpAccel != NULL);

	int iIndex = m_wndCategoryList.GetCurSel ();
	if (iIndex == LB_ERR)
	{
		return;
	}

	HINSTANCE hInstRes = AfxGetResourceHandle ();

#ifndef BCGP_EXCLUDE_RIBBON
	if (m_pWndRibbonBar != NULL)
	{
		CBCGPRibbonCategory* pCategory = NULL;

		if (m_pWndRibbonBar->GetMainCategory () != NULL)
		{
			iIndex--;

			if (iIndex < 0)
			{
				pCategory = m_pWndRibbonBar->GetMainCategory ();
			}
		}

		if (pCategory == NULL)
		{
			pCategory = m_pWndRibbonBar->GetCategory (iIndex);
		}

		ASSERT_VALID(pCategory);

		CArray<CBCGPBaseRibbonElement*, CBCGPBaseRibbonElement*> arElements;
		pCategory->GetElements (arElements);

		AfxSetResourceHandle (m_hInstDefault);

		int nItem = 0;
		m_KeymapList.DeleteAllItems();

		for (int i = 0; i < (int)arElements.GetSize (); i++)
		{
			CBCGPBaseRibbonElement* pElem = arElements [i];
			ASSERT_VALID (pElem);

			if (!pElem->IsKindOf (RUNTIME_CLASS (CBCGPRibbonSeparator)) && pElem->GetID() > 0 && pElem->GetID() != (UINT) -1)
			{
				CString strLabel = pElem->GetToolTip();
				if (strLabel.IsEmpty ())
				{
					strLabel = pElem->GetText();
				}

				if (!strLabel.IsEmpty())
				{
					OnInsertItem(pElem, nItem);

					if (m_bItemWasAdded)
					{
						nItem++;
					}
				}
			}
		}
	}
	else
#endif
	{
		CObList* pCategoryButtonsList = (CObList*) m_wndCategoryList.GetItemData (iIndex);
		ASSERT_VALID (pCategoryButtonsList);

		AfxSetResourceHandle (m_hInstDefault);

		int nItem = 0;
		m_KeymapList.DeleteAllItems();

		for (POSITION pos = pCategoryButtonsList->GetHeadPosition (); pos != NULL;)
		{
			CBCGPToolbarButton* pButton = (CBCGPToolbarButton*) pCategoryButtonsList->GetNext (pos);
			ASSERT (pButton != NULL);

			if (pButton->m_nID > 0 && pButton->m_nID != (UINT) -1)
			{
				OnInsertItem (pButton, nItem);

				if (m_bItemWasAdded)
				{
					nItem++;
				}
			}
		}
	}

	m_KeymapList.SortItems (listCompareFunc, (LPARAM) this);
	AfxSetResourceHandle (hInstRes);
}
//*************************************************************************************
void CBCGPKeyMapDlg::OnCopy() 
{
	m_KeymapList.SetFocus ();
	CopyKeyMap ();
}
//*************************************************************************************
void CBCGPKeyMapDlg::OnPrint() 
{
	m_KeymapList.SetFocus ();
	PrintKeyMap ();
}
//*************************************************************************************
void CBCGPKeyMapDlg::OnSize(UINT nType, int cx, int cy) 
{
	CBCGPDialog::OnSize(nType, cx, cy);

	if (m_KeymapList.GetSafeHwnd () == NULL)
	{
		return;
	}

	//---------------------------------------------------------------
	// List of keys should cover the whole bottom part of the dialog:
	//---------------------------------------------------------------
	CRect rectList;
	m_KeymapList.GetClientRect (rectList);
	m_KeymapList.MapWindowPoints (this, &rectList);

	CRect rectClient;
	GetClientRect (rectClient);
	
	rectList.right = rectClient.right;
	rectList.bottom = rectClient.bottom;

	m_KeymapList.SetWindowPos (NULL, -1, -1,
		rectList.Width (), rectList.Height (),
		SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);

	//--------------------------
	// Adjust the columns width:
	//--------------------------
	SetColumnsWidth ();
}
//*************************************************************************************
BOOL CBCGPKeyMapDlg::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	NMHDR* pNMHDR = (NMHDR*) lParam;
	if (pNMHDR != NULL && pNMHDR->code == HDN_ITEMCLICK)
	{
		HD_NOTIFY *phdn = (HD_NOTIFY *) pNMHDR;

		if (phdn->iButton == 0)	// Left button
		{
			if (phdn->iItem == m_nSortedCol)
				m_bSortAscending = !m_bSortAscending;
			else
				m_bSortAscending = TRUE;

			m_nSortedCol = phdn->iItem;
			m_KeymapList.SortItems (listCompareFunc, (LPARAM) this);
			m_KeymapList.SetSortColumn (m_nSortedCol, m_bSortAscending);
		}
	}
	
	return CBCGPDialog::OnNotify(wParam, lParam, pResult);
}
//*************************************************************************************
void CBCGPKeyMapDlg::CopyKeyMap ()
{
	int i = m_KeymapList.GetSelectedCount();
	if (i <= 0)
	{
		MessageBeep ((UINT)-1);
		return;
	}

	CString strText;
	int nItem = -1;
	int nFlag = (m_KeymapList.GetSelectedCount () > 0)  ?  LVNI_SELECTED : LVNI_ALL;

	while ((nItem = m_KeymapList.GetNextItem (nItem, nFlag)) >= 0)
	{
		strText += FormatItem (nItem) + _T("\r\n");
	}

	if (OpenClipboard ())
	{
		EmptyClipboard ();

		HGLOBAL hClipbuffer = ::GlobalAlloc (GMEM_DDESHARE, (strText.GetLength () + 1) * sizeof (TCHAR));
		if (hClipbuffer != NULL)
		{
			LPTSTR lpszBuffer = (LPTSTR) GlobalLock (hClipbuffer);

			lstrcpy (lpszBuffer, (LPCTSTR) strText);

			::GlobalUnlock (hClipbuffer);
			::SetClipboardData (_TCF_TEXT, hClipbuffer);
		}

		CloseClipboard();
	}
}
//*************************************************************************************
void CBCGPKeyMapDlg::PrintKeyMap ()
{
	CWaitCursor WaitCursor;
	
	int nItem = -1;
	int nFlag = (m_KeymapList.GetSelectedCount () > 0)  ?  LVNI_SELECTED : LVNI_ALL;
	
	CPrintDialog dlgPrint (FALSE, PD_ALLPAGES | PD_RETURNDC | PD_NOSELECTION, NULL);
	if (dlgPrint.DoModal() != IDOK)
	{
		return;
	}
	
	// Obtain a handle to the device context.
	HDC hdcPrn = dlgPrint.GetPrinterDC ();
	if (hdcPrn == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	CDC dc;
	dc.Attach (hdcPrn);

	CSize szPage (dc.GetDeviceCaps (HORZRES), dc.GetDeviceCaps (VERTRES));

	dc.StartDoc(_T("BCGKeyMapDlg"));  // begin a new print job
	dc.StartPage ();
	
	int nPage = 1;
	int y = OnPrintHeader (dc, nPage, szPage.cx);

	while ((nItem = m_KeymapList.GetNextItem (nItem, nFlag)) >= 0)
	{
		int nItemHeight = OnPrintItem (dc, nItem, y, szPage.cx, TRUE /* Calc height */);
		if (y + nItemHeight > szPage.cy)
		{
			dc.EndPage();

			dc.StartPage ();
			y = OnPrintHeader (dc, ++nPage, szPage.cx);
		}

		y += OnPrintItem (dc, nItem, y, szPage.cx, FALSE /* Draw */);
	}
	
	dc.EndPage();
	dc.EndDoc();
}
//*************************************************************************************
int CBCGPKeyMapDlg::OnPrintHeader (CDC& dc, int nPage, int cx) const
{
	TEXTMETRIC tm;
	dc.GetTextMetrics (&tm);

	int yMargin = tm.tmHeight * 2;

	CString strAppName = (AfxGetApp ()->m_pszAppName == NULL) ? _T("") :
						AfxGetApp ()->m_pszAppName;

	CString strTitle;
	GetWindowText (strTitle);

	CString strCaption;
	strCaption.Format (_T("- %d -\r\n%s: %s"), nPage, strAppName, strTitle);

	CRect rectText (0, yMargin, cx, 32767);
	return dc.DrawText (strCaption, rectText, DT_WORDBREAK | DT_CENTER) + 2 * yMargin;
}
//*************************************************************************************
int CBCGPKeyMapDlg::OnPrintItem (CDC& dc, int nItem, int y, int cx, BOOL bCalcHeight) const
{
	ASSERT_VALID (this);
	ASSERT (nItem >= 0);

	TEXTMETRIC tm;
	dc.GetTextMetrics (&tm);

	int xMargin = tm.tmMaxCharWidth * 2;

	CString strCommand = m_KeymapList.GetItemText (nItem, iColumnCommand);
	CString strKeys = m_KeymapList.GetItemText (nItem, iColumnKeys);
	CString strDescr = m_KeymapList.GetItemText (nItem, iColumnDescr);

	// Define column width:
	int nKeyColumWidth = dc.GetTextExtent (CString (_T("Ctrl+Shift+W"))).cx + xMargin;
	int nRestOfWidth = cx - nKeyColumWidth - 2 * xMargin;

	int nHeight = 1;

	for (int iStep = 0; iStep < (bCalcHeight ? 1 : 2); iStep ++)
	{
		UINT uiFormat = iStep == 0 ? 
			(DT_CALCRECT | DT_WORDBREAK) : 
			(DT_WORDBREAK | DT_END_ELLIPSIS);

		CRect rectCmd (CPoint (xMargin, y), CSize (nRestOfWidth / 3, 32676));
		int nCmdHeight = dc.DrawText (strCommand, rectCmd, uiFormat);

		CRect rectKey (CPoint (rectCmd.right + xMargin, y), CSize (nKeyColumWidth, 32676));
		int nKeyHeight = dc.DrawText (strKeys, rectKey, uiFormat);

		CRect rectDescr (rectKey.right + xMargin, y, cx, 32676);
		int nDescrHeight = dc.DrawText (strDescr, rectDescr, uiFormat);

		nHeight = max (nCmdHeight, max (nKeyHeight, nDescrHeight));
	}

	return nHeight;
}
//*************************************************************************************
void CBCGPKeyMapDlg::SetColumnsWidth ()
{
	CRect rectList;
	m_KeymapList.GetClientRect (rectList);

	CClientDC dc (this);
	CFont* pOldFont = dc.SelectObject (m_KeymapList.GetFont ());
	ASSERT_VALID (pOldFont);

	int nKeyColumWidth = dc.GetTextExtent (CString (_T("Ctrl+Shift+W"))).cx + 10;

	dc.SelectObject (pOldFont);

	int nRestOfWidth = rectList.Width () - nKeyColumWidth - ::GetSystemMetrics (SM_CXHSCROLL);

	m_KeymapList.SetColumnWidth (iColumnCommand, nRestOfWidth / 3);
	m_KeymapList.SetColumnWidth (iColumnKeys, nKeyColumWidth);
	m_KeymapList.SetColumnWidth (iColumnDescr, nRestOfWidth * 2 / 3);
}
//*************************************************************************************
INT_PTR CBCGPKeyMapDlg::DoModal() 
{
	m_hInstDefault = AfxGetResourceHandle ();
	return CBCGPDialog::DoModal();
}
//*************************************************************************************
BOOL CBCGPKeyMapDlg::Create(CWnd* pParentWnd) 
{
	m_hInstDefault = AfxGetResourceHandle ();

	CBCGPLocalResource locaRes;
	return CBCGPDialog::Create(IDD, pParentWnd);
}
//*************************************************************************************
void CBCGPKeyMapDlg::OnDestroy() 
{
	//----------------------------------
	// Save window position and size:
	//----------------------------------
	if (GetWorkspace () != NULL)
	{
		CRect rectPosition;
		GetWindowRect (rectPosition);

		CBCGPRegistrySP regSP;
		CBCGPRegistry& reg = regSP.Create (FALSE, FALSE);

		if (reg.CreateKey (GetWorkspace ()->GetRegSectionPath (strWindowPlacementRegSection)))
		{
			reg.Write (strRectKey, rectPosition);
		}
	}

	CBCGPDialog::OnDestroy();
}
//*************************************************************************************
CString CBCGPKeyMapDlg::FormatItem (int nItem) const
{	
	ASSERT_VALID (this);

	CString strKeys = m_KeymapList.GetItemText (nItem, iColumnKeys);
	if (strKeys.IsEmpty ())
	{
		strKeys = _T("-");
	}

	CString strItem;
	strItem.Format (_T("%-30s\t%-20s\t%s"),
				m_KeymapList.GetItemText (nItem, iColumnCommand),
				strKeys,
				m_KeymapList.GetItemText (nItem, iColumnDescr));

	return strItem;
}
//***************************************************************************************
void CBCGPKeyMapDlg::OnSetColumns ()
{
	CString strCaption;

	strCaption.LoadString (IDS_BCGBARRES_COMMAND);
	m_KeymapList.InsertColumn (iColumnCommand, strCaption);

	strCaption.LoadString (IDS_BCGBARRES_KEYS);
	m_KeymapList.InsertColumn (iColumnKeys, strCaption);

	strCaption.LoadString (IDS_BCGBARRES_DESCRIPTION);
	m_KeymapList.InsertColumn (iColumnDescr, strCaption);
}
//***************************************************************************************
void CBCGPKeyMapDlg::OnInsertItem (CBCGPToolbarButton* pButton, int nItem)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pButton);

	m_bItemWasAdded = FALSE;

	CString strKeys = GetCommandKeys (pButton->m_nID);
	if (m_bShowCommandsWithKeyOnly && strKeys.IsEmpty())
	{
		return;
	}

	//------------------
	// Set command name:
	//------------------
	CString strText = pButton->m_strTextCustom.IsEmpty () ? pButton->m_strText : pButton->m_strTextCustom;

	int iIndex = m_KeymapList.InsertItem (nItem, strText, -1);
	m_KeymapList.SetItemText (iIndex, iColumnKeys, strKeys);
	m_KeymapList.SetItemData (iIndex, (DWORD_PTR) pButton);

	//-------------------------
	// Set command description:
	//-------------------------
	CString strDescr;
	CFrameWnd* pParent = GetParentFrame ();

	if (pParent != NULL && pParent->GetSafeHwnd () != NULL && pButton->m_nID != 0)
	{
		pParent->GetMessageString (pButton->m_nID, strDescr);
	}

	m_KeymapList.SetItemText (iIndex, iColumnDescr, strDescr);
	m_bItemWasAdded = TRUE;
}

#ifndef BCGP_EXCLUDE_RIBBON

void CBCGPKeyMapDlg::OnInsertItem (CBCGPBaseRibbonElement* pElem, int nItem)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pElem);

	m_bItemWasAdded = FALSE;

	CString strKeys = GetCommandKeys(pElem->GetID());
	if (m_bShowCommandsWithKeyOnly && strKeys.IsEmpty())
	{
		return;
	}

	//------------------
	// Set command name:
	//------------------
	CString strLabel = pElem->GetToolTip();
	if (strLabel.IsEmpty ())
	{
		strLabel = pElem->GetText();
	}

	strLabel.Replace (_T("&&"), strDummyAmpSeq);
	strLabel.Remove (_T('&'));
	strLabel.Replace (strDummyAmpSeq, _T("&"));

	int iIndex = m_KeymapList.InsertItem (nItem, strLabel, -1);
	m_KeymapList.SetItemText (iIndex, iColumnKeys, strKeys);
	m_KeymapList.SetItemData (iIndex, (DWORD_PTR) pElem);

	//-------------------------
	// Set command description:
	//-------------------------
	CString strDescr;
	CFrameWnd* pParent = GetParentFrame ();

	if (pParent != NULL && pParent->GetSafeHwnd () != NULL && pElem->GetID() != 0)
	{
		pParent->GetMessageString (pElem->GetID(), strDescr);
	}

	m_KeymapList.SetItemText (iIndex, iColumnDescr, strDescr);
	m_bItemWasAdded = TRUE;
}

#endif

CString CBCGPKeyMapDlg::GetCommandKeys (UINT uiCmdID) const
{
	//--------------------------------------------
	// Fill keys associated with selected command:
	//--------------------------------------------
	CString strKey;

	for (int i = 0; i < m_nAccelSize; i ++)
	{
		if (uiCmdID == m_lpAccel [i].cmd)
		{
			ASSERT (&m_lpAccel [i] != NULL);

			CBCGPKeyHelper helper (&m_lpAccel [i]);
			CString sNewKey;
			helper.Format (sNewKey);

			if (!strKey.IsEmpty())
			{
				strKey += _T("; ");
			}

			strKey += sNewKey;
		}
	}

	return strKey;
}
