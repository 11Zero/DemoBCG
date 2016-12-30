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
//
// BCGPRibbonItemDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BCGPRibbonItemDlg.h"
#include "BCGPToolbarButton.h"
#include "BCGPToolBarImages.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBCGPRibbonItemDlg dialog


CBCGPRibbonItemDlg::CBCGPRibbonItemDlg(CBCGPToolBarImages& images, CWnd* pParent /*=NULL*/)
	: CBCGPDialog(CBCGPRibbonItemDlg::IDD, pParent),
	m_images(images)
{
	//{{AFX_DATA_INIT(CBCGPRibbonItemDlg)
	m_strName = _T("");
	//}}AFX_DATA_INIT

	m_iSelImage = -1;
	m_bIsLocal = TRUE;

	EnableLayout();
	EnableVisualManagerStyle (globalData.m_bUseVisualManagerInBuiltInDialogs, TRUE);
}

CBCGPRibbonItemDlg::~CBCGPRibbonItemDlg()
{
	while (!m_Buttons.IsEmpty())
	{
		delete m_Buttons.RemoveHead();
	}	
}

void CBCGPRibbonItemDlg::DoDataExchange(CDataExchange* pDX)
{
	CBCGPDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBCGPRibbonItemDlg)
	DDX_Control(pDX, IDC_BCGBARRES_IMAGE_LIST, m_wndImageList);
	DDX_Text(pDX, IDC_BCGBARRES_NAME, m_strName);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CBCGPRibbonItemDlg, CBCGPDialog)
	//{{AFX_MSG_MAP(CBCGPRibbonItemDlg)
	ON_BN_CLICKED(IDC_BCGBARRES_IMAGE_LIST, OnImageList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGPRibbonItemDlg message handlers

BOOL CBCGPRibbonItemDlg::OnInitDialog() 
{
	CBCGPDialog::OnInitDialog();

	CBCGPStaticLayout* pLayout = (CBCGPStaticLayout*)GetLayout ();
	if (pLayout != NULL)
	{
		pLayout->AddAnchor (IDC_BCGBARRES_IMAGE_LIST, CBCGPStaticLayout::e_MoveTypeNone, CBCGPStaticLayout::e_SizeTypeBoth, CSize(0, 0), CSize(100, 100));
		pLayout->AddAnchor (IDD_BCGBAR_RES_LABEL1, CBCGPStaticLayout::e_MoveTypeVert, CBCGPStaticLayout::e_SizeTypeNone, CSize(50, 100));
		pLayout->AddAnchor (IDC_BCGBARRES_NAME, CBCGPStaticLayout::e_MoveTypeVert, CBCGPStaticLayout::e_SizeTypeHorz, CSize(50, 100), CSize(100, 100));
		pLayout->AddAnchor (IDOK, CBCGPStaticLayout::e_MoveTypeBoth, CBCGPStaticLayout::e_SizeTypeNone, CSize(100, 100));
		pLayout->AddAnchor (IDCANCEL, CBCGPStaticLayout::e_MoveTypeBoth, CBCGPStaticLayout::e_SizeTypeNone, CSize(100, 100));
	}
	
	m_wndImageList.SetImages(&m_images);
	
	int nCount = m_images.GetCount ();

	for (int iImage = 0; iImage < nCount; iImage++)
	{
		CBCGPToolbarButton* pButton = new CBCGPToolbarButton;

		pButton->SetImage (iImage);

		m_wndImageList.AddButton (pButton);
		m_Buttons.AddTail (pButton);
	}

	m_wndImageList.SelectButton (m_iSelImage);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
//***************************************************************************************************************
void CBCGPRibbonItemDlg::OnImageList() 
{
	CBCGPToolbarButton* pSelButton = m_wndImageList.GetSelectedButton ();
	m_iSelImage = (pSelButton == NULL) ? -1 : pSelButton->GetImage ();
}
