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
//

#include "stdafx.h"
#include "bcgprores.h"
#include "BCGPRibbonBackstagePagePrint.h"
#include "BCGPRibbonBackstageView.h"
#include "BCGPLocalResource.h"
#include "MenuImages.h"
#include "BCGPMath.h"

#include <winspool.h>
#include <io.h>

#ifndef BCGP_EXCLUDE_RIBBON

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#ifndef PRINTER_ATTRIBUTE_FAX
	#define PRINTER_ATTRIBUTE_FAX 0x00004000
#endif

#ifndef GetDefaultPrinter
	typedef BOOL (WINAPI *LPFNGetDefaultPrinterA)(LPSTR pszBuffer, LPDWORD pcchBuffer);
	typedef BOOL (WINAPI *LPFNGetDefaultPrinterW)(LPWSTR pszBuffer, LPDWORD pcchBuffer);
	static LPFNGetDefaultPrinterA	m_lpfnGetDefaultPrinterA = NULL;
	static LPFNGetDefaultPrinterW	m_lpfnGetDefaultPrinterW = NULL;
	#ifdef UNICODE
		#define _GetDefaultPrinter  m_lpfnGetDefaultPrinterW
	#else
		#define _GetDefaultPrinter  m_lpfnGetDefaultPrinterA
	#endif // !UNICODE
#else 
	#define _GetDefaultPrinter  GetDefaultPrinter
#endif

static LPCTSTR s_szShell32_DLL  = _T("shell32.dll");
static LPCTSTR s_szImageres_DLL = _T("imageres.dll");
static const DWORD s_dwFlagsMask = PD_ALLPAGES | PD_PAGENUMS | PD_SELECTION | PD_COLLATE | PD_PRINTTOFILE;


CString CBCGPRibbonBackstagePagePrint::XPrinterInfo::GetDisplayName() const
{
	CString str(strPrinterName);
	if (!strServerName.IsEmpty ())
	{
		CBCGPLocalResource localRes;

		CString strLine;
		strLine.LoadString (IDP_BCGBARRES_PRINT_NAME);

		CString strFmt;
		strFmt.Format(strLine, (LPCTSTR)strPrinterName + strServerName.GetLength () + 1, (LPCTSTR)strServerName + 2);

		str = strFmt;
	}

	str += _T("\n") + GetStatusName();

	return str;
}

CString CBCGPRibbonBackstagePagePrint::XPrinterInfo::GetStatusName() const
{
	CBCGPLocalResource localRes;

	CString strLine;
	UINT nID = IDS_BCGBARRES_PRINT_STATUS_READY;
	if (dwStatus == 0)
	{
		if (dwAttributes & PRINTER_ATTRIBUTE_WORK_OFFLINE)
		{
			nID = IDS_BCGBARRES_PRINT_STATUS_OFFLINE;
		}
	}
	else
	{
		UINT nIDS[][2] = {
			{PRINTER_STATUS_BUSY, IDS_BCGBARRES_PRINT_STATUS_BUSY},
			{PRINTER_STATUS_DOOR_OPEN, IDS_BCGBARRES_PRINT_STATUS_DOOR_OPEN},
			{PRINTER_STATUS_ERROR, IDS_BCGBARRES_PRINT_STATUS_ERROR},
			{PRINTER_STATUS_INITIALIZING, IDS_BCGBARRES_PRINT_STATUS_INITIALIZING},
			{PRINTER_STATUS_IO_ACTIVE, IDS_BCGBARRES_PRINT_STATUS_IO_ACTIVE},
			{PRINTER_STATUS_MANUAL_FEED, IDS_BCGBARRES_PRINT_STATUS_MANUAL_FEED},
			{PRINTER_STATUS_NO_TONER, IDS_BCGBARRES_PRINT_STATUS_NO_TONER},
			{PRINTER_STATUS_NOT_AVAILABLE, IDS_BCGBARRES_PRINT_STATUS_NOT_AVAILABLE},
			{PRINTER_STATUS_OFFLINE, IDS_BCGBARRES_PRINT_STATUS_OFFLINE},
			{PRINTER_STATUS_OUT_OF_MEMORY, IDS_BCGBARRES_PRINT_STATUS_OUT_OF_MEMORY},
			{PRINTER_STATUS_OUTPUT_BIN_FULL, IDS_BCGBARRES_PRINT_STATUS_OUTPUT_BIN_FULL},
			{PRINTER_STATUS_PAGE_PUNT, IDS_BCGBARRES_PRINT_STATUS_PAGE_PUNT},
			{PRINTER_STATUS_PAPER_JAM, IDS_BCGBARRES_PRINT_STATUS_PAPER_JAM},
			{PRINTER_STATUS_PAPER_OUT, IDS_BCGBARRES_PRINT_STATUS_PAPER_OUT},
			{PRINTER_STATUS_PAPER_PROBLEM, IDS_BCGBARRES_PRINT_STATUS_PAPER_PROBLEM},
			{PRINTER_STATUS_PAUSED, IDS_BCGBARRES_PRINT_STATUS_PAUSED},
			{PRINTER_STATUS_PENDING_DELETION, IDS_BCGBARRES_PRINT_STATUS_PENDING_DELETION},
			{PRINTER_STATUS_PRINTING, IDS_BCGBARRES_PRINT_STATUS_PRINTING},
			{PRINTER_STATUS_PROCESSING, IDS_BCGBARRES_PRINT_STATUS_PROCESSING},
			{PRINTER_STATUS_TONER_LOW, IDS_BCGBARRES_PRINT_STATUS_TONER_LOW},
			{PRINTER_STATUS_USER_INTERVENTION, IDS_BCGBARRES_PRINT_STATUS_USER_INTERVENTION},
			{PRINTER_STATUS_WAITING, IDS_BCGBARRES_PRINT_STATUS_WAITING},
			{PRINTER_STATUS_WARMING_UP, IDS_BCGBARRES_PRINT_STATUS_WARMING_UP},
			{PRINTER_STATUS_SERVER_UNKNOWN, IDS_BCGBARRES_PRINT_STATUS_SERVER_UNKNOWN},
			{PRINTER_STATUS_POWER_SAVE, IDS_BCGBARRES_PRINT_STATUS_POWER_SAVE}
		};

		for (int i = 0; i < sizeof(nIDS) / (2 * sizeof(UINT)); i++)
		{
			if (dwStatus & nIDS[i][0])
			{
				CString str;
				str.LoadString (nIDS[i][1]);

				if (!str.IsEmpty ())
				{
					if (!strLine.IsEmpty ())
					{
						strLine += _T("; ");
					}

					strLine += str;
				}
			}
		}

		if (strLine.IsEmpty ())
		{
			nID = IDS_BCGBARRES_PRINT_STATUS_UNKNOWN;
		}
	}

	if (strLine.IsEmpty ())
	{
		strLine.LoadString (nID);
	}

	return strLine;
}

CString CBCGPRibbonBackstagePagePrint::XPaperInfo::GetDisplayName() const
{
	CString str(strName);
	if (ptSize.x != 0 && ptSize.y != 0)
	{
		CBCGPLocalResource localRes;

		CString strLine;
		strLine.LoadString (IDP_BCGBARRES_PRINT_SIZE);

		CString strFmt;
		strFmt.Format(strLine, ptSize.x / 100.0, ptSize.y / 100.0);
		str = strName + _T("\n") + strFmt;
	}

	return str;
}

BOOL BCGPPreparePrinting(CView* pView, CPrintInfo* pInfo)
{
	ASSERT_VALID(pView);

	BOOL bPreview = pInfo->m_bPreview;
	if (CBCGPRibbonBackstagePagePrint::s_BCGPrintInfo.bPrinting && !pInfo->m_bPreview && !pInfo->m_bDirect)
	{
		pInfo->m_bPreview = TRUE;
	}

	BOOL bRes = pView->DoPreparePrinting(pInfo);

	pInfo->m_bPreview = bPreview;

	if (CBCGPRibbonBackstagePagePrint::s_BCGPrintInfo.bPrinting)
	{
		if (bRes && !pInfo->m_bPreview)
		{
			pInfo->m_pPD->m_pd.Flags &= ~s_dwFlagsMask;
			pInfo->m_pPD->m_pd.Flags |= (CBCGPRibbonBackstagePagePrint::s_BCGPrintInfo.Flags & s_dwFlagsMask);
			pInfo->m_pPD->m_pd.nFromPage = CBCGPRibbonBackstagePagePrint::s_BCGPrintInfo.nFromPage;
			pInfo->m_pPD->m_pd.nToPage = CBCGPRibbonBackstagePagePrint::s_BCGPrintInfo.nToPage;
			pInfo->m_pPD->m_pd.nCopies = CBCGPRibbonBackstagePagePrint::s_BCGPrintInfo.nCopies;
		}

		CBCGPRibbonBackstagePagePrint::s_BCGPrintInfo.bPrinting = FALSE;
	}

	return bRes;
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPRibbonBackstagePagePrint dialog

CBCGPPrintInfo CBCGPRibbonBackstagePagePrint::s_BCGPrintInfo;

IMPLEMENT_DYNCREATE(CBCGPRibbonBackstagePagePrint, CBCGPDialog)

CBCGPRibbonBackstagePagePrint::CBCGPRibbonBackstagePagePrint(UINT nIDTemplate/* = 0*/, CWnd* pParent /*=NULL*/)
	: CBCGPDialog    (nIDTemplate == 0 ? CBCGPRibbonBackstagePagePrint::IDD : nIDTemplate, pParent)
	, m_wndPreview   (NULL)
	, m_pPrintView   (NULL)
{
	m_bIsLocal = TRUE;

	//{{AFX_DATA_INIT(CBCGPRibbonBackstagePagePrint)
	m_nCopies = 1;
	m_nPageFrom = 1;
	m_nPageTo = 0xFFFF;
	//}}AFX_DATA_INIT

	EnableLayout();

#ifndef GetDefaultPrinter
    m_lpfnGetDefaultPrinterA = NULL;
    m_lpfnGetDefaultPrinterW = NULL;

	HMODULE hModule = ::GetModuleHandle(_T("winspool.drv"));
	if (hModule != NULL)
	{
		m_lpfnGetDefaultPrinterA = (LPFNGetDefaultPrinterA)::GetProcAddress(hModule, "GetDefaultPrinterA");
		m_lpfnGetDefaultPrinterW = (LPFNGetDefaultPrinterW)::GetProcAddress(hModule, "GetDefaultPrinterW");
	}
#endif
}

CBCGPRibbonBackstagePagePrint::~CBCGPRibbonBackstagePagePrint()
{
	if (m_wndPreview != NULL)
	{
		delete m_wndPreview;
	}

	if (m_pPrintView != NULL)
	{
		m_pPrintView->PostMessage(WM_COMMAND, ID_FILE_PRINT);
	}
}

void CBCGPRibbonBackstagePagePrint::DoDataExchange(CDataExchange* pDX)
{
	CBCGPDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBCGPRibbonBackstagePagePrint)
	DDX_Control(pDX, IDC_BCGBARRES_PRINT_LABEL3, m_wndLabel3);
	DDX_Control(pDX, IDC_BCGBARRES_PRINT_LABEL2, m_wndLabel2);
	DDX_Control(pDX, IDC_BCGBARRES_PRINT_LABEL1, m_wndLabel1);
	DDX_Control(pDX, AFX_ID_PREVIEW_PRINT, m_btnPrint);
	DDX_Control(pDX, IDC_BCGBARRES_PRINT_COPIES_EDIT, m_wndCopies);
	DDX_Control(pDX, IDC_BCGBARRES_PRINT_COPIES_SPIN, m_btnCopies);
	DDX_Control(pDX, IDC_BCGBARRES_PRINT_PRINTER, m_wndPrinter);
	DDX_Control(pDX, IDC_BCGBARRES_PRINT_PROPERTIES, m_wndPrinterProperties);
	DDX_Control(pDX, IDC_BCGBARRES_PRINT_PAGE, m_wndPage);
	DDX_Control(pDX, IDC_BCGBARRES_PRINT_PAGE_FROM, m_wndPageFrom);
	DDX_Control(pDX, IDC_BCGBARRES_PRINT_PAGE_TO, m_wndPageTo);
	DDX_Control(pDX, IDC_BCGBARRES_PRINT_COLLATE, m_wndCollate);
	DDX_Control(pDX, IDC_BCGBARRES_PRINT_ORIENT, m_wndOrientation);
	DDX_Control(pDX, IDC_BCGBARRES_PRINT_PAPER, m_wndPaper);
	DDX_Control(pDX, AFX_ID_PREVIEW_PREV, m_btnPrev);
	DDX_Control(pDX, IDC_BCGBARRES_PRINT_PAGE_NUM, m_wndPageNum);
	DDX_Control(pDX, AFX_ID_PREVIEW_NEXT, m_btnNext);
	DDX_Control(pDX, IDC_BCGBARRES_PRINT_ZOOM_NUM, m_wndZoomNum);
	DDX_Control(pDX, IDC_BCGBARRES_PRINT_ZOOM_SLIDER, m_wndZoomSlider);
	DDX_Text(pDX, IDC_BCGBARRES_PRINT_COPIES_EDIT, m_nCopies);
	DDV_MinMaxShort(pDX, m_nCopies, 1, 9999);
	//}}AFX_DATA_MAP

	PRINTDLG* dlgPrint = GetPrintDlg();
	if (dlgPrint != NULL)
	{
		DDX_Text(pDX, IDC_BCGBARRES_PRINT_PAGE_FROM, m_nPageFrom);
		DDV_MinMaxUInt(pDX, m_nPageFrom, dlgPrint->nMinPage, min(dlgPrint->nMaxPage, 32767U));

		if (pDX->m_bSaveAndValidate)
		{
			CString strText;
			m_wndPageTo.GetWindowText (strText);
			if (strText.IsEmpty ())
			{
				m_nPageTo = 0xFFFF;
			}
			else
			{
				DDX_Text(pDX, IDC_BCGBARRES_PRINT_PAGE_TO, m_nPageTo);
				DDV_MinMaxUInt(pDX, m_nPageTo, dlgPrint->nMinPage, min(dlgPrint->nMaxPage, 32767U));
			}
		}
		else
		{
			if (m_nPageTo == 0xFFFF)
			{
				m_wndPageTo.SetWindowText (_T(""));
			}
			else
			{
				DDX_Text(pDX, IDC_BCGBARRES_PRINT_PAGE_TO, m_nPageTo);
			}
		}
	}
}


BEGIN_MESSAGE_MAP(CBCGPRibbonBackstagePagePrint, CBCGPDialog)
	//{{AFX_MSG_MAP(CBCGPRibbonBackstagePagePrint)
	ON_COMMAND(IDC_BCGBARRES_PRINT_PRINTER, OnSelectPrinter)
	ON_COMMAND(IDC_BCGBARRES_PRINT_FILE, OnPrintFile)
	ON_UPDATE_COMMAND_UI(IDC_BCGBARRES_PRINT_FILE, OnUpdatePrintFile)
	ON_COMMAND(IDC_BCGBARRES_PRINT_PROPERTIES, OnPrinterProperties)
	ON_COMMAND(IDC_BCGBARRES_PRINT_COLLATE, OnSelectCollate)
	ON_COMMAND(IDC_BCGBARRES_PRINT_PAGE, OnSelectPage)
	ON_COMMAND(IDC_BCGBARRES_PRINT_ORIENT, OnSelectPaper)
	ON_COMMAND(AFX_ID_PREVIEW_PRINT, OnPreviewPrint)
	ON_COMMAND(AFX_ID_PREVIEW_PREV, OnPrevPage)
	ON_COMMAND(IDC_BCGBARRES_PRINT_PAGE_NUM, OnPageChanged)
	ON_COMMAND(AFX_ID_PREVIEW_NEXT, OnNextPage)
	ON_COMMAND(IDC_BCGBARRES_PRINT_ZOOM_NUM, OnZoomChanged)
	ON_WM_HSCROLL()
	ON_COMMAND(IDC_BCGBARRES_PRINT_PAPER, OnSelectPaper)
	ON_WM_SETTINGCHANGE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGPRibbonBackstagePagePrint message handlers

CBCGPPrintPreviewCtrl* CBCGPRibbonBackstagePagePrint::CreatePreviewWnd() const
{
	return (CBCGPPrintPreviewCtrl*)(RUNTIME_CLASS(CBCGPPrintPreviewCtrl)->CreateObject ());
}

BOOL CBCGPRibbonBackstagePagePrint::OnInitDialog() 
{
	CBCGPDialog::OnInitDialog();

	UpdateLabels();

	if (CBCGPVisualManager::GetInstance()->IsDWMCaptionSupported())
	{
		m_wndZoomNum.m_bOnGlass = TRUE;
	}

	BOOL bInitialized = FALSE;

	CView* pView = NULL;
	CFrameWnd* pFrame = DYNAMIC_DOWNCAST(CFrameWnd, AfxGetApp ()->GetMainWnd ());
	if (pFrame != NULL)
	{
		if (pFrame->IsKindOf (RUNTIME_CLASS(CMDIFrameWnd)))
		{
			pFrame = ((CMDIFrameWnd*)pFrame)->GetActiveFrame ();
		}

		pView = pFrame->GetActiveView ();
		bInitialized = pView != NULL;
	}

	CRect rectWnd;

	CWnd* pWndLocation = GetDlgItem (IDC_BCGBARRES_PRINT_PREVIEW);
	ASSERT(pWndLocation);
	pWndLocation->GetWindowRect (rectWnd);
	ScreenToClient (rectWnd);
	pWndLocation->ShowWindow (SW_HIDE);
	pWndLocation->DestroyWindow ();

	if (bInitialized && pView->GetSafeHwnd () != NULL)
	{
		// Create the preview view object
		m_wndPreview = CreatePreviewWnd();
		if (m_wndPreview == NULL)
		{
			TRACE0("Error: Failed to create preview control.\n");
		}
		else if (!m_wndPreview->Create(WS_CHILD | WS_TABSTOP, CRect(0, 0, 0, 0), this, IDC_BCGBARRES_PRINT_PREVIEW))
		{
			TRACE0("Error: Failed to create preview control.\n");
			delete m_wndPreview;
			m_wndPreview = NULL;
		}
		else
		{
			m_wndPreview->m_bVisualManagerStyle = IsVisualManagerStyle();
			m_wndPreview->m_bBackstageMode = IsBackstageMode();

			m_wndPreview->SetNotifyPage (IDC_BCGBARRES_PRINT_PAGE_NUM);
			m_wndPreview->SetNotifyZoom (IDC_BCGBARRES_PRINT_ZOOM_NUM);

			InitializePrintInfo();

			if (!m_wndPreview->SetPrintView (pView, NULL))
			{
				TRACE0("Error: Failed to initialize preview control.\n");

				m_wndPreview->DestroyWindow ();
				delete m_wndPreview;
				m_wndPreview = NULL;
				bInitialized = FALSE;
			}
			else
			{
				m_wndPreview->ShowWindow (SW_SHOWNA);
				m_wndPreview->MoveWindow (rectWnd);
			}
		}
	}

	OnInitPrintControls();

	CBCGPStaticLayout* pLayout = (CBCGPStaticLayout*)GetLayout();
	ASSERT_VALID(pLayout);

	pLayout->AddAnchor(IDC_BCGBARRES_PRINT_SEPARATOR, CBCGPStaticLayout::e_MoveTypeNone, CBCGPStaticLayout::e_SizeTypeVert);
	if (m_wndPreview->GetSafeHwnd () != NULL)
	{
		pLayout->AddAnchor(m_wndPreview->GetSafeHwnd (), CBCGPStaticLayout::e_MoveTypeNone, CBCGPStaticLayout::e_SizeTypeBoth);
	}
	pLayout->AddAnchor(AFX_ID_PREVIEW_PREV, CBCGPStaticLayout::e_MoveTypeVert, CBCGPStaticLayout::e_SizeTypeNone);
	pLayout->AddAnchor(IDC_BCGBARRES_PRINT_PAGE_NUM, CBCGPStaticLayout::e_MoveTypeVert, CBCGPStaticLayout::e_SizeTypeNone);
	pLayout->AddAnchor(AFX_ID_PREVIEW_NEXT, CBCGPStaticLayout::e_MoveTypeVert, CBCGPStaticLayout::e_SizeTypeNone);
	pLayout->AddAnchor(IDC_BCGBARRES_PRINT_ZOOM_NUM, CBCGPStaticLayout::e_MoveTypeBoth, CBCGPStaticLayout::e_SizeTypeNone);
	pLayout->AddAnchor(IDC_BCGBARRES_PRINT_ZOOM_SLIDER, CBCGPStaticLayout::e_MoveTypeBoth, CBCGPStaticLayout::e_SizeTypeNone);

	PRINTDLG* dlgPrint = GetPrintDlg();
	if (!bInitialized || dlgPrint == NULL || dlgPrint->hDevMode == NULL || dlgPrint->hDevNames == NULL)
	{
		m_btnPrint.EnableWindow (FALSE);
		SetCopies(m_nCopies, FALSE);
	}

	OnPageChanged ();
	OnZoomChanged ();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CBCGPRibbonBackstagePagePrint::InitializePrintInfo()
{
	CBCGPRibbonBackstagePagePrint::s_BCGPrintInfo.bPrinting = FALSE;
}

void CBCGPRibbonBackstagePagePrint::OnInitPrintControls()
{
	CBCGPLocalResource localRes;

	double dblScale = globalData.GetRibbonImageScale ();
	if (dblScale != 1.0)
	{
		CBCGPToolBarImages image;
		image.Load (IDB_BCGBARRES_PRINT_BUTTON);
		image.SetSingleImage ();
		image.SmoothResize (dblScale);

		m_btnPrint.SetImage (image.GetImageWell (), 0, 0);
	}
	else
	{
		m_btnPrint.SetImage(IDB_BCGBARRES_PRINT_BUTTON);
	}

	m_btnPrint.m_bTopImage = TRUE;
	m_btnPrint.m_bDrawFocus = FALSE;

	m_btnCopies.SetRange (1, 9999);

	m_wndPrinterProperties.m_bDefaultClickProcess = TRUE;
	m_wndPrinterProperties.m_nAlignStyle = CBCGPButton::ALIGN_CENTER;

	m_btnPrev.SetWindowText (_T(""));
	m_btnPrev.SetStdImage (CBCGPMenuImages::IdArowLeftTab3d);

	m_btnNext.SetWindowText (_T(""));
	m_btnNext.SetStdImage (CBCGPMenuImages::IdArowRightTab3d);

	m_wndZoomSlider.SetRange (10, 100);
	m_wndZoomSlider.SetPos (10);

	PRINTDLG* dlgPrint = GetPrintDlg();
	if (dlgPrint == NULL || dlgPrint->hDevMode == NULL || dlgPrint->hDevNames == NULL)
	{
		m_wndPrinter.EnableWindow (FALSE);
		m_wndPrinterProperties.EnableWindow (FALSE);
		m_wndPage.EnableWindow (FALSE);
		m_wndPageFrom.EnableWindow (FALSE);
		m_wndPageTo.EnableWindow (FALSE);
		m_wndCollate.EnableWindow (FALSE);
		m_wndOrientation.EnableWindow (FALSE);
		m_wndPaper.EnableWindow (FALSE);

		return;
	}

	CString strLine;

	int nPages = 0;
	m_wndPage.SetIcons (IDB_BCGBARRES_PRINT_PAGE, 32);
	strLine.LoadString (IDS_BCGBARRES_PRINT_PAGE_A);
	m_wndPage.AddString(strLine, 0);
	if ((dlgPrint->Flags & PD_NOSELECTION) == 0)
	{
		strLine.LoadString (IDS_BCGBARRES_PRINT_PAGE_S);
		m_wndPage.AddString(strLine, 1);
		if ((dlgPrint->Flags & PD_SELECTION) == PD_SELECTION)
		{
			nPages = 1;
		}
	}
	if ((dlgPrint->Flags & PD_NOPAGENUMS) == 0 && dlgPrint->nMinPage != dlgPrint->nMaxPage)
	{
		strLine.LoadString (IDS_BCGBARRES_PRINT_PAGE_C);
		m_wndPage.AddString(strLine, 2);
		strLine.LoadString (IDS_BCGBARRES_PRINT_PAGE_R);
		m_wndPage.AddString(strLine, 3);
		if ((dlgPrint->Flags & PD_PAGENUMS) == PD_PAGENUMS)
		{
			nPages = m_wndPage.GetCount () - 1;
		}
	}
	else
	{
		m_wndPageFrom.EnableWindow (FALSE);
		m_wndPageTo.EnableWindow (FALSE);
	}

	m_wndPage.SetCurSel (nPages);

	m_wndCollate.SetIcons (IDB_BCGBARRES_PRINT_COLLATE, 32);
	strLine.LoadString (IDS_BCGBARRES_PRINT_COLLATE_C);
	m_wndCollate.AddString (strLine, 0);
	strLine.LoadString (IDS_BCGBARRES_PRINT_COLLATE_U);
	m_wndCollate.AddString (strLine, 1);

	m_wndOrientation.SetIcons (IDB_BCGBARRES_PRINT_ORIENT, 32);
	strLine.LoadString (IDS_BCGBARRES_PRINT_ORIENT_P);
	m_wndOrientation.AddString (strLine, 0);
	strLine.LoadString (IDS_BCGBARRES_PRINT_ORIENT_L);
	m_wndOrientation.AddString (strLine, 1);

	m_nPageFrom = dlgPrint->nFromPage;
	m_nPageTo = dlgPrint->nToPage;
	UpdateData(FALSE);

	LoadPrinterImages ();

	UpdatePrinters ();
}

short CBCGPRibbonBackstagePagePrint::GetCopies()
{
	UpdateData (TRUE);
	return m_nCopies;
}

void CBCGPRibbonBackstagePagePrint::SetCopies(short nCopies, BOOL bEnable)
{
	m_nCopies = (short)max((int)nCopies, 1);

	PRINTDLG* pDlg = GetPrintDlg ();
	if (pDlg != NULL)
	{
		bEnable = bEnable || ((pDlg->Flags & PD_USEDEVMODECOPIESANDCOLLATE) == 0);
	}
	else
	{
		bEnable = FALSE;
	}

	m_wndCopies.EnableWindow (bEnable);
	m_btnCopies.EnableWindow (bEnable);

	UpdateData (FALSE);
}

void CBCGPRibbonBackstagePagePrint::UpdatePrinters ()
{
	CBCGPLocalResource localRes;

	m_wndPrinter.ResetContent ();
	m_Printers.RemoveAll ();

	PRINTDLG* dlgPrint = GetPrintDlg();
	ASSERT(dlgPrint != NULL);

	if (dlgPrint->hDevNames == NULL)
	{
		return;
	}

	int nSelection = -1;

	DWORD dwNeeded = 0;
	DWORD dwReturned = 0;
	DWORD i = 0;
	LPPRINTER_INFO_2 pPrinters = NULL;
	::EnumPrinters(PRINTER_ENUM_FAVORITE | PRINTER_ENUM_LOCAL | PRINTER_ENUM_NETWORK, NULL, 2, NULL, 0, &dwNeeded, &dwReturned);
	if (dwNeeded > 0)
	{
		LPBYTE pPrinterEnum = new BYTE[dwNeeded];

		if (!::EnumPrinters(PRINTER_ENUM_FAVORITE | PRINTER_ENUM_LOCAL | PRINTER_ENUM_NETWORK, NULL, 2, pPrinterEnum, dwNeeded, &dwNeeded, &dwReturned))
		{
			delete [] pPrinterEnum;
			pPrinterEnum = NULL;
		}

		pPrinters = (LPPRINTER_INFO_2)pPrinterEnum;
	}

	CString strPrinterDef;
	dwNeeded = 0;
#ifndef GetDefaultPrinter
	if (_GetDefaultPrinter != NULL)
#endif
	{
		if (_GetDefaultPrinter(NULL, &dwNeeded) != ERROR_FILE_NOT_FOUND)
		{
			if (dwNeeded > 0)
			{
				_GetDefaultPrinter(strPrinterDef.GetBuffer (dwNeeded + 1), &dwNeeded);
				strPrinterDef.ReleaseBuffer ();
			}
		}
	}

	if (pPrinters == NULL)
	{
		return;
	}

	m_PrinterImages.CopyTo (m_wndPrinter.GetPalette ().GetImages ());

	LPDEVNAMES lpDevNames = (LPDEVNAMES)::GlobalLock(dlgPrint->hDevNames);
	ASSERT(lpDevNames != NULL);

	for (i = 0; i < dwReturned; i++)
	{
		PRINTER_INFO_2& printerInfo = pPrinters[i];
		XPrinterInfo info(printerInfo.pServerName, printerInfo.pPrinterName, 
						printerInfo.pPortName, printerInfo.pDriverName, 
						printerInfo.Attributes, printerInfo.Status);
		m_Printers.Add (info);

		int nImageIndex = 0;
		if ((info.dwAttributes & PRINTER_ATTRIBUTE_FAX) == PRINTER_ATTRIBUTE_FAX)
		{
			nImageIndex += 4;
		}
		if ((info.dwAttributes & PRINTER_ATTRIBUTE_NETWORK) == PRINTER_ATTRIBUTE_NETWORK)
		{
			nImageIndex += 2;
		}
		if ((info.dwAttributes & PRINTER_ATTRIBUTE_DEFAULT) == PRINTER_ATTRIBUTE_DEFAULT ||
			info.strPrinterName.Compare(strPrinterDef) == 0)
		{
			nImageIndex += 1;
		}

		m_wndPrinter.AddString (info.GetDisplayName (), nImageIndex);
		//m_wndPrinter.SetItemData(m_wndPrinter.AddString (strName), i);

		if (nSelection == -1)
		{
			if (//lstrcmp((LPCTSTR)lpDevNames + lpDevNames->wDriverOffset, info.strDriverName) == 0 ||
				lstrcmp((LPCTSTR)lpDevNames + lpDevNames->wDeviceOffset, info.strPrinterName) == 0 &&
				lstrcmp((LPCTSTR)lpDevNames + lpDevNames->wOutputOffset, info.strPortName) == 0)
			{
				nSelection = i;
			}
		}
	}

	delete [] pPrinters;

	::GlobalUnlock(dlgPrint->hDevNames);

	CString strLine;
	strLine.LoadString (IDS_BCGBARRES_PRINT_FILE);
	m_wndPrinter.AddCommand (IDC_BCGBARRES_PRINT_FILE, strLine);

	m_wndPrinter.SetCurSel (max(nSelection, 0));

	if (nSelection == -1)
	{
		OnSelectPrinter ();
	}
	else if (dlgPrint->hDevMode != NULL)
	{
		UpdatePapers ();
		UpdatePrinterProperties (FALSE);
	}
}

void CBCGPRibbonBackstagePagePrint::UpdatePapers (short nPaper)
{
	CBCGPLocalResource localRes;

	m_wndPaper.ResetContent ();
	m_Papers.RemoveAll ();

	PRINTDLG* dlgPrint = GetPrintDlg();
	ASSERT(dlgPrint != NULL);

	if (dlgPrint->hDevNames == NULL || dlgPrint->hDevMode == NULL)
	{
		return;
	}

	m_wndPaper.SetIcons (IDB_BCGBARRES_PRINT_PAPER, 32);

	LPDEVNAMES lpDevNames = (LPDEVNAMES)::GlobalLock(dlgPrint->hDevNames);
	ASSERT(lpDevNames != NULL);

	CString strDeviceName ((LPTSTR)lpDevNames + lpDevNames->wDeviceOffset);
	CString strPortName ((LPTSTR)lpDevNames + lpDevNames->wOutputOffset);

	::GlobalUnlock (dlgPrint->hDevNames);

	LPDEVMODE lpDevMode = (LPDEVMODE)::GlobalLock(dlgPrint->hDevMode);
	ASSERT(lpDevMode != NULL);

	int nSelection = -1;
	short nPaperID[] = {DMPAPER_LETTER, DMPAPER_TABLOID, DMPAPER_LEGAL, DMPAPER_EXECUTIVE, DMPAPER_A3, DMPAPER_A4, DMPAPER_A5};

	DWORD i = 0;
	DWORD dwCount = ::DeviceCapabilities (strDeviceName, strPortName, DC_PAPERS, NULL, lpDevMode);
	if (dwCount > 0)
	{
		WORD* lpPapers = new WORD[dwCount];
		POINT* lpSizes = new POINT[dwCount];
		LPTSTR lpNames = new TCHAR[dwCount * 64];
		::DeviceCapabilities (strDeviceName, strPortName, DC_PAPERS, (LPTSTR)lpPapers, lpDevMode);
		DWORD dwCountS = ::DeviceCapabilities (strDeviceName, strPortName, DC_PAPERSIZE, (LPTSTR)lpSizes, lpDevMode);
		DWORD dwCountN = ::DeviceCapabilities (strDeviceName, strPortName, DC_PAPERNAMES, (LPTSTR)lpNames, lpDevMode);

		for (i = 0; i < dwCount; i++)
		{
			if (dwCountN <= i)
			{
				continue;
			}

			CString strName(lpNames + i * 64);
			if (strName.IsEmpty ())
			{
				continue;
			}

			POINT ptSize = {0, 0};
			if (i < dwCountS)
			{
				ptSize = lpSizes[i];
			}

			XPaperInfo info(lpPapers[i], (LPCTSTR)strName, ptSize);
			m_Papers.Add (info);

			int nImageIndex = 0;
			for (int j = 0; j < sizeof(nPaperID) / sizeof(nPaperID[0]); j++)
			{
				if (info.nPaper == nPaperID[j])
				{
					nImageIndex = j + 1;
					break;
				}
			}

			m_wndPaper.AddString (info.GetDisplayName (), nImageIndex);
//			m_wndPaper.SetItemData(m_wndPaper.AddString (info.strName), i);

			if (nSelection == -1)
			{
				if (info.nPaper == lpDevMode->dmPaperSize)
				{
					nSelection = i;
				}
			}
		}

		delete [] lpPapers;
		delete [] lpSizes;
		delete [] lpNames;
	}

	::GlobalUnlock (dlgPrint->hDevMode);

	if (nSelection == -1)
	{
		lpDevMode->dmPaperSize = nPaper;
		for (i = 0; i < dwCount; i++)
		{
			if (m_Papers[i].nPaper == nPaper)
			{
				nSelection = i;
				break;
			}
		}
	}
/*
	if (nSelection != -1)
	{
		for (i = 0; i < cbNeeded; i++)
		{
			if (m_wndPaper.GetItemData (i) == (DWORD)nSelection)
			{
				nSelection = i;
				break;
			}
		}
	}
*/
	m_wndPaper.SetCurSel (max(nSelection, 0));
}

int CBCGPRibbonBackstagePagePrint::GetPrinterSelection () const
{
	int nSelection = m_wndPrinter.GetCurSel ();
	if (nSelection < 0)
	{
		ASSERT(FALSE);
		return -1;
	}

	//nSelection = m_wndPrinter.GetItemData (nSelection);
	return nSelection;
}

int CBCGPRibbonBackstagePagePrint::GetPaperSelection () const
{
/*
	int nSelection = m_wndPaper.GetCurSel ();
	if (nSelection < 0)
	{
		ASSERT(FALSE);
		return -1;
	}

	return m_wndPaper.GetItemData (nSelection);
*/
	return m_wndPaper.GetCurSel ();
}

CString CBCGPRibbonBackstagePagePrint::GetDeviceName ()
{
	CString strDeviceName;

	PRINTDLG* dlgPrint = GetPrintDlg();
	ASSERT(dlgPrint != NULL);

	LPDEVNAMES lpDevNames = (LPDEVNAMES)::GlobalLock(dlgPrint->hDevNames);
	if (lpDevNames != NULL)
	{
		strDeviceName = (LPTSTR)lpDevNames + lpDevNames->wDeviceOffset;
	}
	::GlobalUnlock (dlgPrint->hDevNames);

	return strDeviceName;
}

HANDLE CBCGPRibbonBackstagePagePrint::OpenPrinterByName (LPCTSTR lpDeviceName)
{
	CString strDeviceName (lpDeviceName);
	if (strDeviceName.IsEmpty ())
	{
		strDeviceName = GetDeviceName ();
	}

	HANDLE hPrinter = NULL;
	if (!strDeviceName.IsEmpty ())
	{
		if (!::OpenPrinter ((LPTSTR)(LPCTSTR)strDeviceName, &hPrinter, NULL))
		{
			hPrinter = NULL;
		}
	}

	return hPrinter;
}

void CBCGPRibbonBackstagePagePrint::OnSelectPrinter() 
{
	CWinApp* pApp = AfxGetApp();
	if (pApp == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	int nSelection = GetPrinterSelection ();
	if (nSelection < 0)
	{
		ASSERT(FALSE);
		return;
	}

	PRINTDLG* dlgPrint = GetPrintDlg();
	ASSERT(dlgPrint != NULL);

	LPDEVNAMES lpDevNames = NULL;
	if (dlgPrint->hDevNames != NULL)
	{
		lpDevNames = (LPDEVNAMES)::GlobalLock(dlgPrint->hDevNames);
		ASSERT(lpDevNames != NULL);
	}

	XPrinterInfo& info = m_Printers[nSelection];

	if (lstrcmp((LPCTSTR)lpDevNames + lpDevNames->wDriverOffset, info.strDriverName) != 0 ||
		lstrcmp((LPCTSTR)lpDevNames + lpDevNames->wDeviceOffset, info.strPrinterName) != 0 ||
		lstrcmp((LPCTSTR)lpDevNames + lpDevNames->wOutputOffset, info.strPortName) != 0)
	{
		::GlobalUnlock(dlgPrint->hDevNames);

		// Compute size of DEVNAMES structure from PRINTER_INFO_2's data
		int drvNameLen = info.strDriverName.GetLength () + 1;  // driver name
		int ptrNameLen = info.strPrinterName.GetLength () + 1; // printer name
		int porNameLen = info.strPortName.GetLength () + 1;    // port name

		// Allocate a global handle big enough to hold DEVNAMES.
		HGLOBAL hDevNames = ::GlobalAlloc(GHND, sizeof(DEVNAMES) + (drvNameLen + ptrNameLen + porNameLen) * sizeof(TCHAR));
		LPDEVNAMES lpDevNames = (LPDEVNAMES)::GlobalLock(hDevNames);
		ASSERT(lpDevNames != NULL);

		// Copy the DEVNAMES information from PRINTER_INFO_2
		// tcOffset = TCHAR Offset into structure
		int tcOffset = sizeof(DEVNAMES) / sizeof(TCHAR);
		ASSERT(sizeof(DEVNAMES) == tcOffset * sizeof(TCHAR));

		lpDevNames->wDriverOffset = (WORD)tcOffset;
		memcpy((LPTSTR)lpDevNames + tcOffset, (LPCTSTR)info.strDriverName, drvNameLen * sizeof(TCHAR));
		tcOffset += drvNameLen;

		lpDevNames->wDeviceOffset = (WORD)tcOffset;
		memcpy((LPTSTR)lpDevNames + tcOffset, (LPCTSTR)info.strPrinterName, ptrNameLen * sizeof(TCHAR));
		tcOffset += ptrNameLen;

		lpDevNames->wOutputOffset = (WORD)tcOffset;
		memcpy((LPTSTR)lpDevNames + tcOffset, (LPCTSTR)info.strPortName, porNameLen * sizeof(TCHAR));
		lpDevNames->wDefault = 0;

		::GlobalUnlock(hDevNames);

		dlgPrint->hDevNames = hDevNames;

		pApp->SelectPrinter(dlgPrint->hDevNames, NULL, TRUE);
		pApp->DevModeChange((LPTSTR)(LPCTSTR)info.strPrinterName);
		pApp->GetPrinterDeviceDefaults (dlgPrint);

		short dmPaperSize = 0;
		if (dlgPrint->hDevMode != NULL)
		{
			LPDEVMODE lpDevMode = (LPDEVMODE)::GlobalLock (dlgPrint->hDevMode);
			dmPaperSize = lpDevMode->dmPaperSize;
			::GlobalUnlock (dlgPrint->hDevMode);
		}

		BOOL bNotify = m_wndPaper.GetCount () > 0;

		UpdatePrinterProperties(TRUE);
		UpdatePapers (dmPaperSize);
		UpdatePrinterProperties(FALSE, bNotify);
	}
}

void CBCGPRibbonBackstagePagePrint::OnPrintFile()
{
	PRINTDLG* dlgPrint = GetPrintDlg();
	if (dlgPrint == NULL)
	{
		return;
	}

	if ((dlgPrint->Flags & PD_PRINTTOFILE) == PD_PRINTTOFILE)
	{
		dlgPrint->Flags &= ~PD_PRINTTOFILE;
	}
	else
	{
		dlgPrint->Flags |= PD_PRINTTOFILE;
	}
}

void CBCGPRibbonBackstagePagePrint::OnUpdatePrintFile(CCmdUI* pCmdUI)
{
	PRINTDLG* dlgPrint = GetPrintDlg();
	if (dlgPrint == NULL)
	{
		pCmdUI->Enable (FALSE);
		return;
	}

	pCmdUI->Enable (TRUE);
	pCmdUI->SetCheck ((dlgPrint->Flags & PD_PRINTTOFILE) == PD_PRINTTOFILE);
}

void CBCGPRibbonBackstagePagePrint::OnPrinterProperties() 
{
	CWaitCursor wait;

	CWinApp* pApp = AfxGetApp();
	if (pApp == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	int nSelection = GetPrinterSelection ();
	if (nSelection < 0)
	{
		ASSERT(FALSE);
		return;
	}

	PRINTDLG* dlgPrint = GetPrintDlg();
	ASSERT(dlgPrint != NULL);

	if (dlgPrint->hDevMode == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	CString strDeviceName (GetDeviceName ());
	HANDLE hPrinter =  OpenPrinterByName(strDeviceName);
	if (hPrinter == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	UpdatePrinterProperties (TRUE);

	LPDEVMODE lpDevMode = (LPDEVMODE)::GlobalLock (dlgPrint->hDevMode);
	BOOL bRes = ::DocumentProperties(GetParent()->GetSafeHwnd (), hPrinter, (LPTSTR)(LPCTSTR)strDeviceName, lpDevMode, lpDevMode, DM_IN_PROMPT | DM_IN_BUFFER | DM_OUT_BUFFER) == IDOK;
	::GlobalUnlock (dlgPrint->hDevMode);

	::ClosePrinter (hPrinter);

	if (bRes)
	{
		UpdatePrinterProperties(FALSE, TRUE);
	}
}

void CBCGPRibbonBackstagePagePrint::OnSelectCollate()
{
	UpdatePrinterProperties (TRUE, FALSE);
}

void CBCGPRibbonBackstagePagePrint::OnSelectPage()
{
}

void CBCGPRibbonBackstagePagePrint::OnSelectPaper()
{
	UpdatePrinterProperties (TRUE, TRUE);
}

void CBCGPRibbonBackstagePagePrint::UpdatePrinterProperties(BOOL bSaveAndValidate, BOOL bNotify/* = FALSE*/)
{
	PRINTDLG* dlgPrint = GetPrintDlg();
	ASSERT(dlgPrint != NULL);

	if (dlgPrint->hDevMode == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	LPDEVMODE lpDevMode = (LPDEVMODE)::GlobalLock (dlgPrint->hDevMode);

	if (bSaveAndValidate)
	{
		if ((dlgPrint->Flags & PD_USEDEVMODECOPIESANDCOLLATE) == PD_USEDEVMODECOPIESANDCOLLATE)
		{
			if ((lpDevMode->dmFields & DM_COPIES) == DM_COPIES)
			{
				lpDevMode->dmCopies = GetCopies ();
			}
			if ((lpDevMode->dmFields & DM_COLLATE) == DM_COLLATE)
			{
				lpDevMode->dmCollate = (short)(m_wndCollate.GetCurSel () == 0 ? DMCOLLATE_TRUE : DMCOLLATE_FALSE);
			}
		}
		else
		{
			dlgPrint->nCopies = GetCopies ();
			if ((lpDevMode->dmFields & DM_COPIES) == DM_COPIES)
			{
				lpDevMode->dmCopies = 1;
			}

			if (m_wndCollate.GetCurSel () == 0)
			{
				dlgPrint->Flags |= PD_COLLATE;
			}
			else
			{
				dlgPrint->Flags &= ~PD_COLLATE;
			}
		}

		if ((lpDevMode->dmFields & DM_ORIENTATION) == DM_ORIENTATION)
		{
			lpDevMode->dmOrientation = (short)(m_wndOrientation.GetCurSel () == 0 ? DMORIENT_PORTRAIT : DMORIENT_LANDSCAPE);
		}

		if (m_wndPaper.GetCount () > 0)
		{
			//lpDevMode->dmPaperSize = m_Papers[m_wndPaper.GetItemData (m_wndPaper.GetCurSel ())].nPaper;
			lpDevMode->dmPaperSize = m_Papers[m_wndPaper.GetCurSel ()].nPaper;
		}

		CString strDeviceName (GetDeviceName ());
		HANDLE hPrinter =  OpenPrinterByName(strDeviceName);
		if (hPrinter != NULL)
		{
			::DocumentProperties(NULL, hPrinter, (LPTSTR)(LPCTSTR)strDeviceName, lpDevMode, lpDevMode, DM_IN_BUFFER | DM_OUT_BUFFER);
			::ClosePrinter (hPrinter);
		}

		if ((dlgPrint->Flags & PD_USEDEVMODECOPIESANDCOLLATE) == PD_USEDEVMODECOPIESANDCOLLATE)
		{
			dlgPrint->nCopies = 1;
			dlgPrint->Flags &= ~PD_COLLATE;
		}
	}
	else
	{
		if ((dlgPrint->Flags & PD_USEDEVMODECOPIESANDCOLLATE) == PD_USEDEVMODECOPIESANDCOLLATE)
		{
			SetCopies (lpDevMode->dmCopies, (lpDevMode->dmFields & DM_COPIES) == DM_COPIES);
			m_wndCollate.SetCurSel (lpDevMode->dmCollate == DMCOLLATE_TRUE ? 0 : 1);
			m_wndCollate.EnableWindow ((lpDevMode->dmFields & DM_COLLATE) == DM_COLLATE);
		}
		else
		{
			SetCopies (dlgPrint->nCopies, TRUE);
			m_wndCollate.SetCurSel ((dlgPrint->Flags & PD_COLLATE) == PD_COLLATE ? 0 : 1);
			m_wndCollate.EnableWindow (TRUE);
		}

		m_wndOrientation.SetCurSel (lpDevMode->dmOrientation == DMORIENT_PORTRAIT ? 0 : 1);
		m_wndOrientation.EnableWindow ((lpDevMode->dmFields & DM_ORIENTATION) == DM_ORIENTATION);

		int count = m_wndPaper.GetCount ();
		for (int i = 0; i < count; i++)
		{
			//if (lpDevMode->dmPaperSize == m_Papers[m_wndPaper.GetItemData (i)].nPaper)
			if (lpDevMode->dmPaperSize == m_Papers[i].nPaper)
			{
				m_wndPaper.SetCurSel (i);
				break;
			}
		}
	}

	::GlobalUnlock (dlgPrint->hDevMode);

	if (bNotify)
	{
		if (m_wndPreview->GetSafeHwnd () != NULL)
		{
			m_wndPreview->ChangeSettings ();
		}
	}
}

void CBCGPRibbonBackstagePagePrint::LoadPrinterImages()
{
	m_PrinterImages.Clear ();

	CString strPath;
    ::GetSystemDirectory(strPath.GetBuffer (_MAX_PATH + 1), _MAX_PATH);
	strPath.ReleaseBuffer ();

	if (strPath.IsEmpty ())
	{
		return;
	}

	if (strPath[strPath.GetLength () - 1] != TCHAR('\\'))
	{
		strPath += _T("\\");
	}

	// shell32.dll
	int uiImageShell32[] = {17, 168, 140, 169, 196, 197, 199, 198};
	int uiImageImageRes[] = {0, 0, 0, 0, 0, 0, 0, 0};
	if (_taccess(strPath + s_szImageres_DLL, 0) != -1)
	{
		// imageres.dll
		uiImageImageRes[0] = 51;
		uiImageImageRes[1] = 49;
		uiImageImageRes[2] = 53;
		uiImageImageRes[3] = 50;
		uiImageImageRes[4] = 76;
	}

	m_PrinterImages.SetImageSize (CSize(::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON)), FALSE);

	for (int i = 0; i < 8; i++)
	{
		CString strFile;
		int nIconIndex = -1;
		if (uiImageImageRes[i] != 0)
		{
			nIconIndex = uiImageImageRes[i];
			strFile = strPath + s_szImageres_DLL;
		}
		else
		{
			nIconIndex = uiImageShell32[i];
			strFile = strPath + s_szShell32_DLL;
		}

		HICON hIcon = NULL;
		::ExtractIconEx(strFile, -nIconIndex, &hIcon, NULL, 1);
		if (hIcon != NULL)
		{
			m_PrinterImages.AddIcon (hIcon, TRUE);
			::DestroyIcon (hIcon);
		}
	}
}


CBCGPRibbonBackstageView* CBCGPRibbonBackstagePagePrint::GetBackStageView()
{
	CWnd* pWnd = GetParent ();
	if (pWnd != NULL)
	{
		return DYNAMIC_DOWNCAST(CBCGPRibbonBackstageView, pWnd);
	}

	return NULL;
}


BOOL CBCGPRibbonBackstagePagePrint::PreparePrintInfo()
{
	if (m_wndPreview->GetSafeHwnd () == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	CPrintInfo* pPrintInfo = m_wndPreview->GetPrintInfo ();
	if (pPrintInfo == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	PRINTDLG* dlgPrint = GetPrintDlg ();
	if (dlgPrint == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	if (!UpdateData ())
	{
		return FALSE;
	}

	UpdatePrinterProperties (TRUE, FALSE);

	int nPages = m_wndPage.GetCurSel ();
	if (nPages == 0)
	{
		dlgPrint->Flags &= ~(PD_PAGENUMS | PD_SELECTION);
		dlgPrint->Flags |= PD_ALLPAGES;
	}
	else
	{
		int nPageCur = 1;
		int nPageNum = 2;

		if ((dlgPrint->Flags & PD_NOSELECTION) == 0)
		{
			nPageCur++;
			nPageNum++;
		}

		if (nPages == nPageCur || nPages == nPageNum)
		{
			if (nPages == nPageCur)
			{
				dlgPrint->nFromPage = (WORD)m_wndPreview->GetCurrentPage ();
				dlgPrint->nToPage = dlgPrint->nFromPage;
			}
			else
			{
				dlgPrint->nFromPage = (WORD)m_nPageFrom;
				dlgPrint->nToPage = (WORD)(m_nPageTo == 0xFFFF ? m_nPageFrom : m_nPageTo);
			}

			dlgPrint->Flags &= ~(PD_ALLPAGES | PD_SELECTION);
			dlgPrint->Flags |= PD_PAGENUMS;
		}
		else
		{
			dlgPrint->Flags &= ~(PD_ALLPAGES | PD_PAGENUMS);
			dlgPrint->Flags |= PD_SELECTION;
		}
	}

	m_pPrintView = m_wndPreview->GetPrintView ();
	
	s_BCGPrintInfo.Flags = (dlgPrint->Flags & s_dwFlagsMask);
	s_BCGPrintInfo.nFromPage = dlgPrint->nFromPage;
	s_BCGPrintInfo.nToPage = dlgPrint->nToPage;
	s_BCGPrintInfo.nCopies = dlgPrint->nCopies;
	s_BCGPrintInfo.bPrinting = TRUE;

	return TRUE;
}

void CBCGPRibbonBackstagePagePrint::OnPreviewPrint()
{
	if (!PreparePrintInfo ())
	{
		return;
	}

	CBCGPRibbonBackstageView* pWnd = GetBackStageView ();
	if (pWnd != NULL)
	{
		pWnd->PostMessage (WM_CLOSE);
	}
}

void CBCGPRibbonBackstagePagePrint::OnPrevPage()
{
	if (m_wndPreview->GetSafeHwnd () != NULL)
	{
		m_wndPreview->SetCurrentPage (m_wndPreview->GetCurrentPage () - 1);
	}
}

void CBCGPRibbonBackstagePagePrint::OnPageChanged() 
{
	if (m_wndPreview->GetSafeHwnd () != NULL)
	{
		CPrintInfo* pPrintInfo = m_wndPreview->GetPrintInfo ();

		CString strPage;
		CString strFmt;
		if (!AfxExtractSubString (strFmt, pPrintInfo->m_strPageDesc, 0))
		{
			strFmt = _T("%d");
		}

		strPage.Format(strFmt, m_wndPreview->GetCurrentPage ());

		m_wndPageNum.SetWindowText (strPage);

		m_btnPrev.EnableWindow (m_wndPreview->GetCurrentPage () > pPrintInfo->GetMinPage ());
		m_btnNext.EnableWindow (m_wndPreview->GetCurrentPage () < pPrintInfo->GetMaxPage ());
	}
	else
	{
		m_btnPrev.EnableWindow (FALSE);
		m_btnNext.EnableWindow (FALSE);

		m_wndPageNum.SetWindowText (_T(""));
		m_wndPageNum.EnableWindow (FALSE);
	}
}

void CBCGPRibbonBackstagePagePrint::OnNextPage()
{
	if (m_wndPreview->GetSafeHwnd () != NULL)
	{
		m_wndPreview->SetCurrentPage (m_wndPreview->GetCurrentPage () + 1);
	}
}

void CBCGPRibbonBackstagePagePrint::OnZoomChanged() 
{
	if (m_wndPreview->GetSafeHwnd () != NULL)
	{
		int nZoom = (int)(m_wndPreview->GetZoom () * 100.0);
		m_wndZoomSlider.SetPos (nZoom);

		CString str;
		str.Format(_T("%d %%"), nZoom);

		m_wndZoomNum.SetWindowText (str);
	}
	else
	{
		m_wndZoomNum.SetWindowText (_T(""));
		m_wndZoomNum.EnableWindow (FALSE);

		m_wndZoomSlider.EnableWindow (FALSE);
	}
}

void CBCGPRibbonBackstagePagePrint::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	if (pScrollBar == NULL || pScrollBar != (CScrollBar*)&m_wndZoomSlider || m_wndPreview->GetSafeHwnd () == NULL)
	{
		CBCGPDialog::OnVScroll(nSBCode, nPos, pScrollBar);
		return;
	}

	if (m_wndPreview->GetSafeHwnd () != NULL)
	{
		int nZoom = m_wndZoomSlider.GetPos ();
		m_wndPreview->SetZoomType (nZoom);

		CString str;
		str.Format(_T("%d %%"), nZoom);

		m_wndZoomNum.SetWindowText (str);
	}
}

void CBCGPRibbonBackstagePagePrint::UpdateLabels()
{
	static UINT nLabelIDs[] = {IDC_BCGBARRES_PRINT_LABEL1, IDC_BCGBARRES_PRINT_LABEL2, IDC_BCGBARRES_PRINT_LABEL3};

	for (int i = 0; i < sizeof(nLabelIDs) / sizeof(UINT); i++)
	{
		CWnd* pWnd = GetDlgItem (nLabelIDs[i]);
		if (pWnd->GetSafeHwnd () != NULL)
		{
			pWnd->SetFont (&globalData.fontCaption);
		}
	}
}

void CBCGPRibbonBackstagePagePrint::OnSettingChange(UINT uFlags, LPCTSTR lpszSection) 
{
	UpdateLabels();
	CBCGPDialog::OnSettingChange(uFlags, lpszSection);
}

#endif // BCGP_EXCLUDE_RIBBON
