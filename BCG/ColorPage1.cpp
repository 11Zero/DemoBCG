// ColorPage1.cpp : implementation file
//
// This is a part of the BCGControlBar Library
// Copyright (C) 1998-2014 BCGSoft Ltd.
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.

#include "stdafx.h"

#include "bcgprores.h"
#include "BCGCBPro.h"
#include "BCGPColorDialog.h"
#include "BCGPDrawManager.h"
#include "ColorPage1.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBCGPColorPage1 property page

IMPLEMENT_DYNCREATE(CBCGPColorPage1, CBCGPPropertyPage)

CBCGPColorPage1::CBCGPColorPage1() : CBCGPPropertyPage(CBCGPColorPage1::IDD)
{
	//{{AFX_DATA_INIT(CBCGPColorPage1)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	EnableVisualManagerStyle (globalData.m_bUseVisualManagerInBuiltInDialogs);
}

void CBCGPColorPage1::DoDataExchange(CDataExchange* pDX)
{
	CBCGPPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBCGPColorPage1)
	DDX_Control(pDX, IDC_BCGBARRES_HEXPLACEHOLDER, m_hexpicker);
	DDX_Control(pDX, IDC_BCGBARRES_GREYSCALEPLACEHOLDER, m_hexpicker_greyscale);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CBCGPColorPage1, CBCGPPropertyPage)
	//{{AFX_MSG_MAP(CBCGPColorPage1)
	ON_BN_CLICKED(IDC_BCGBARRES_GREYSCALEPLACEHOLDER, OnGreyscale)
	ON_BN_CLICKED(IDC_BCGBARRES_HEXPLACEHOLDER, OnHexColor)
	ON_BN_DOUBLECLICKED(IDC_BCGBARRES_GREYSCALEPLACEHOLDER, OnDoubleClickedColor)
	ON_BN_DOUBLECLICKED(IDC_BCGBARRES_HEXPLACEHOLDER, OnDoubleClickedColor)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGPColorPage1 message handlers

BOOL CBCGPColorPage1::OnInitDialog() 
{
	CBCGPPropertyPage::OnInitDialog();
	
	m_hexpicker.SetPalette (m_pDialog->GetPalette ());
	m_hexpicker.SetType(CBCGPColorPickerCtrl::HEX);

	m_hexpicker_greyscale.SetPalette (m_pDialog->GetPalette ());
	m_hexpicker_greyscale.SetType(CBCGPColorPickerCtrl::HEX_GREYSCALE);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CBCGPColorPage1::OnGreyscale() 
{
	double H,L,S;
	m_hexpicker_greyscale.GetHLS(&H,&L,&S);

	COLORREF color = CBCGPDrawManager::HLStoRGB_TWO(H, L, S);

	m_pDialog->SetNewColor (color);

	BYTE R = GetRValue (color);
	BYTE G = GetGValue (color);
	BYTE B = GetBValue (color);

	m_pDialog->SetPageTwo (R, G, B);

	m_hexpicker.SelectCellHexagon (R, G, B);
	m_hexpicker.Invalidate ();
}

void CBCGPColorPage1::OnHexColor() 
{
	COLORREF color = m_hexpicker.GetRGB ();

	BYTE R = GetRValue (color);
	BYTE G = GetGValue (color);
	BYTE B = GetBValue (color);

	double H,L,S;
	m_hexpicker.GetHLS (&H,&L,&S);

	// Set actual color.
	m_pDialog->SetNewColor (color);

	m_pDialog->SetPageTwo(R, G, B);

	m_hexpicker_greyscale.SelectCellHexagon (R, G, B);
	m_hexpicker_greyscale.Invalidate ();
}

void CBCGPColorPage1::OnDoubleClickedColor() 
{
	m_pDialog->EndDialog (IDOK);
}
