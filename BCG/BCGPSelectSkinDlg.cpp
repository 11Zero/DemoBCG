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
// BCGSelectSkinDlg.cpp : implementation file
//

#include "stdafx.h"

#if defined _AFXDLL && !defined _BCGCBPRO_STATIC_	// Skins manager can not be used in the static version

#include "BCGCBPro.h"
#include "BCGGlobals.h"
#include "bcgprores.h"
#include "BCGPSelectSkinDlg.h"
#include "BCGPSkinManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBCGPSelectSkinDlg dialog

CBCGPSelectSkinDlg::CBCGPSelectSkinDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CBCGPSelectSkinDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CBCGPSelectSkinDlg)
	m_strAuthor = _T("");
	//}}AFX_DATA_INIT

	m_iSelectedSkin = -1;
	m_strClose.LoadString (IDS_BCGBARRES_CLOSE);
	m_srDefault.LoadString (IDS_BCGBARRES_DEFAULT_VIEW);
}


void CBCGPSelectSkinDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBCGPSelectSkinDlg)
	DDX_Control(pDX, IDC_BCGBARRES_MAIL, m_btnMail);
	DDX_Control(pDX, IDC_BCGBARRES_URL, m_btnURL);
	DDX_Control(pDX, IDC_BCGBARRES_DOWNLOAD_SKINS, m_btnDownload);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_BCGBARRES_SKINS_LIST, m_wndSkinsList);
	DDX_Control(pDX, IDC_BCGBARRES_SKIN_PREVIEW, m_wndSkindPreviewArea);
	DDX_Text(pDX, IDC_BCGBARRES_AUTHOR, m_strAuthor);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CBCGPSelectSkinDlg, CDialog)
	//{{AFX_MSG_MAP(CBCGPSelectSkinDlg)
	ON_LBN_DBLCLK(IDC_BCGBARRES_SKINS_LIST, OnDblclkBcgbarresSkinsList)
	ON_WM_PAINT()
	ON_LBN_SELCHANGE(IDC_BCGBARRES_SKINS_LIST, OnSelchangeBcgbarresSkinsList)
	ON_BN_CLICKED(IDC_BCGBARRES_APPLY, OnApply)
	ON_BN_CLICKED(IDC_BCGBARRES_DOWNLOAD_SKINS, OnDownloadSkins)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BCGBARRES_MAIL, OnMail)
	ON_BN_CLICKED(IDC_BCGBARRES_URL, OnURL)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGPSelectSkinDlg message handlers

void CBCGPSelectSkinDlg::OnDblclkBcgbarresSkinsList() 
{
	OnOK();
}
//*************************************************************************************
void CBCGPSelectSkinDlg::OnOK() 
{
	g_pSkinManager->SetActiveSkin (m_iSelectedSkin);
	CDialog::OnOK();
}
//*************************************************************************************
BOOL CBCGPSelectSkinDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	if (g_pSkinManager == NULL)
	{
		ASSERT(FALSE);
		EndDialog (IDCANCEL);

		return TRUE;
	}

	if (AfxGetMainWnd () != NULL && 
		(AfxGetMainWnd ()->GetExStyle () & WS_EX_LAYOUTRTL))
	{
		ModifyStyleEx (0, WS_EX_LAYOUTRTL);
	}

	g_pSkinManager->LoadAllSkins ();

	if (!g_pSkinManager->IsDownloadAvailable ())
	{
		m_btnDownload.EnableWindow (FALSE);
		m_btnDownload.ShowWindow (SW_HIDE);
	}

	//--------------------
	// Setup user buttons:
	//--------------------
	m_btnMail.m_nFlatStyle = CBCGPButton::BUTTONSTYLE_FLAT;
	m_btnMail.SetImage (IDB_BCGBARRES_MAIL);
	m_btnMail.SetWindowText (_T(""));
	m_btnMail.SizeToContent ();
	
	m_btnURL.m_nFlatStyle = CBCGPButton::BUTTONSTYLE_FLAT;
	m_btnURL.SetImage (IDB_BCGBARRES_URL);
	m_btnURL.SetWindowText (_T(""));
	m_btnURL.SizeToContent ();

	//--------------------------
	// Define preview rectangle:
	//--------------------------
	m_wndSkindPreviewArea.GetWindowRect (&m_rectPreview);
	ScreenToClient (&m_rectPreview);

	RefreshSkinsList ();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
//*************************************************************************************
void CBCGPSelectSkinDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	if (m_iSelectedSkin != -1)
	{
		g_pSkinManager->PreviewSkin (&dc, m_iSelectedSkin, m_rectPreview);
	}

	dc.Draw3dRect (m_rectPreview, globalData.clrBtnShadow, globalData.clrBtnHilite);
}
//*************************************************************************************
void CBCGPSelectSkinDlg::OnSelchangeBcgbarresSkinsList() 
{
	ASSERT_VALID (g_pSkinManager);

	int iCurSel = m_wndSkinsList.GetCurSel ();
	m_iSelectedSkin = (int) m_wndSkinsList.GetItemData (iCurSel);

	LPCTSTR lpszAuthor = g_pSkinManager->GetSkinAuthor (m_iSelectedSkin);
	m_strAuthor = lpszAuthor == NULL ? _T("") : lpszAuthor;

	LPCTSTR lpszMail = g_pSkinManager->GetSkinAuthorMail (m_iSelectedSkin);
	m_btnMail.EnableWindow (lpszMail != NULL && lpszMail [0] != 0);
	m_btnMail.SetTooltip (lpszMail);

	LPCTSTR lpszURL = g_pSkinManager->GetSkinAuthorURL (m_iSelectedSkin);
	m_btnURL.EnableWindow (lpszURL != NULL && lpszURL [0] != 0);
	m_btnURL.SetTooltip (lpszURL);

	UpdateData (FALSE);

	InvalidateRect (m_rectPreview);
	UpdateWindow ();
}
//*************************************************************************************
void CBCGPSelectSkinDlg::OnApply() 
{
	g_pSkinManager->SetActiveSkin (m_iSelectedSkin);

	m_btnOK.EnableWindow (FALSE);
	m_btnOK.ShowWindow (SW_HIDE);

	m_btnCancel.SetWindowText (m_strClose);

	g_pSkinManager->LoadAllSkins ();
}
//**********************************************************************************
void CBCGPSelectSkinDlg::OnDownloadSkins() 
{
	ASSERT_VALID (g_pSkinManager);
	
	if (g_pSkinManager->DownloadSkins ())
	{
		g_pSkinManager->UnLoadAllSkins ();
		g_pSkinManager->ScanSkinsLocation ();
		g_pSkinManager->LoadAllSkins ();

		RefreshSkinsList ();
	}
}
//**********************************************************************************
void CBCGPSelectSkinDlg::RefreshSkinsList ()
{
	m_wndSkinsList.SetRedraw (FALSE);
	m_wndSkinsList.ResetContent ();

	int iIndex = m_wndSkinsList.AddString (m_srDefault);
	m_wndSkinsList.SetItemData (iIndex, (DWORD) -1);
	m_wndSkinsList.SetCurSel (iIndex);

	int cxExtent = 0;

	CClientDC dcList (&m_wndSkinsList);
	CFont* pOldFont = dcList.SelectObject (GetFont ());

	CRect rectList;
	m_wndSkinsList.GetClientRect (rectList);

	for (int i = 0; i < g_pSkinManager->GetSkinsCount (); i++)
	{
		LPCTSTR lpszName = g_pSkinManager->GetSkinName (i);
		if (lpszName == NULL)
		{
			ASSERT(FALSE);
			continue;
		}

		iIndex = m_wndSkinsList.AddString (lpszName);
		m_wndSkinsList.SetItemData (iIndex, (DWORD) i);

		if (i == g_pSkinManager->GetActiveSkin ())
		{
			m_iSelectedSkin = i;
			m_wndSkinsList.SetCurSel (iIndex);
		}

		int cxCurr = dcList.GetTextExtent (lpszName).cx;
		cxExtent = max (cxExtent, cxCurr);
	}

	m_wndSkinsList.SetHorizontalExtent (cxExtent + 10);
	dcList.SelectObject (pOldFont);

	OnSelchangeBcgbarresSkinsList ();
	m_wndSkinsList.SetRedraw ();
}
//***********************************************************************************
void CBCGPSelectSkinDlg::OnDestroy() 
{
	if (g_pSkinManager != NULL)
	{
		g_pSkinManager->UnLoadAllSkins ();
	}

	CDialog::OnDestroy();
}
//***********************************************************************************
void CBCGPSelectSkinDlg::OnMail() 
{
	ASSERT_VALID (g_pSkinManager);
	LPCTSTR lpszMail = g_pSkinManager->GetSkinAuthorMail (m_iSelectedSkin);

	if (lpszMail == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	CString str;
	str.Format (_T("mailto:%s"), lpszMail);

	if (::ShellExecute (NULL, NULL, str, NULL, NULL, NULL) < (HINSTANCE) 32)
	{
		TRACE(_T("Can't open URL: %s\n"), str);
	}
}
//***********************************************************************************
void CBCGPSelectSkinDlg::OnURL() 
{
	ASSERT_VALID (g_pSkinManager);
	LPCTSTR lpszURL = g_pSkinManager->GetSkinAuthorURL (m_iSelectedSkin);

	if (lpszURL == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	CString strURL = lpszURL;
	strURL.MakeLower ();

	const CString strPrefix = _T("http://");
	if (strURL.Find (strPrefix) != 0)
	{
		strURL = strPrefix + strURL;
	}

	if (::ShellExecute (NULL, NULL, strURL, NULL, NULL, NULL) < (HINSTANCE) 32)
	{
		TRACE(_T("Can't open URL: %s\n"), strURL);
	}
}

#endif
