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
// BCGPEditBrushDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BCGPMath.h"
#include "bcgcbpro.h"
#include "bcgprores.h"
#include "BCGPEditBrushDlg.h"
#include "BCGPLocalResource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBCGBrushPreviewCtrl

BEGIN_MESSAGE_MAP(CBCGBrushPreviewCtrl, CBCGPStatic)
	//{{AFX_MSG_MAP(CBCGBrushPreviewCtrl)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CBCGBrushPreviewCtrl::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	CBCGPMemDC memDC (dc, this);
	CDC* pDC = &memDC.GetDC ();

	CRect rect;
	GetClientRect(rect);

	pDC->SetBkColor(RGB(255, 255, 255));
	pDC->SetTextColor (RGB (192, 192, 192));

	pDC->FillRect (rect, &m_brBack);

	if (m_pGM == NULL)
	{
		m_pGM = CBCGPGraphicsManager::CreateInstance();
		if (m_pGM != NULL)
		{
			m_pGM->EnableTransparentGradient();
		}
	}

	if (m_pGM == NULL || m_pBrush == NULL)
	{
		return;
	}

	m_pGM->BindDC(pDC, rect);

	if (!m_pGM->BeginDraw())
	{
		return;
	}

	m_pGM->FillRectangle (rect, *m_pBrush);

	m_pGM->EndDraw();
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPEditBrushDlg dialog

CBCGPEditBrushDlg::CBCGPEditBrushDlg(CBCGPBrush& brush, CWnd* pParent, CBCGPEditBrushOptions* pOptions)
	: CBCGPDialog(CBCGPEditBrushDlg::IDD, pParent),
	m_brushRes(brush)
{
	//{{AFX_DATA_INIT(CBCGPEditBrushDlg)
	m_nType = 0;
	//}}AFX_DATA_INIT

	m_bIsLocal = TRUE;
	m_brush = brush;
	m_bOriginalIsEmpty = brush.IsEmpty();

	if (m_bOriginalIsEmpty)
	{
		m_nType = 0;
	}
	else
	{
		m_nType = (int)m_brush.GetGradientType() + 1;
	}

	if (pOptions != NULL)
	{
		m_Options = *pOptions;
	}

#ifndef _BCGSUITE_
	EnableVisualManagerStyle (globalData.m_bUseVisualManagerInBuiltInDialogs, TRUE);
#endif
}

void CBCGPEditBrushDlg::DoDataExchange(CDataExchange* pDX)
{
	CBCGPDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBCGPEditBrushDlg)
	DDX_Control(pDX, IDC_BCGBARRES_SWAP_COLORS, m_wndButtonSwap);
	DDX_Control (pDX, IDC_BCGBARRES_BRUSH_COLOR1, m_wndColor1);
	DDX_Control (pDX, IDC_BCGBARRES_BRUSH_COLOR2, m_wndColor2);
	DDX_Control (pDX, IDC_BCGBARRES_BRUSH_OPACITY, m_wndOpacity);
	DDX_Control(pDX, IDC_BCGBARRES_IMAGE, m_wndImage);
	DDX_CBIndex(pDX, IDC_BCGBARRES_BRUSH_TYPE, m_nType);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CBCGPEditBrushDlg, CBCGPDialog)
	//{{AFX_MSG_MAP(CBCGPEditBrushDlg)
	ON_BN_CLICKED(IDC_BCGBARRES_BRUSH_COLOR1, OnUpdateBrush)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_BCGBARRES_BRUSH_COLOR2, OnUpdateBrush)
	ON_CBN_SELENDOK(IDC_BCGBARRES_BRUSH_TYPE, OnUpdateBrush)
	ON_BN_CLICKED(IDC_BCGBARRES_SWAP_COLORS, OnSwapColors)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGPEditBrushDlg message handlers

void CBCGPEditBrushDlg::OnUpdateBrush()
{
	UpdateData();

	BOOL bIsEmpty = (m_nType == 0);

	CBCGPBrush::BCGP_GRADIENT_TYPE type = (CBCGPBrush::BCGP_GRADIENT_TYPE)(max(0, m_nType - 1));

	BOOL bTwoColors = type != CBCGPBrush::BCGP_NO_GRADIENT && type != CBCGPBrush::BCGP_GRADIENT_BEVEL;

	if (bIsEmpty)
	{
		m_brush.Empty();
	}
	else
	{
		COLORREF clr1 = m_wndColor1.GetColor();
		COLORREF clr2 = m_wndColor2.GetColor();

		if (m_bOriginalIsEmpty && clr1 == (COLORREF)-1 && clr2 == (COLORREF)-1)
		{
			clr1 = m_Options.m_clrDefault;
			clr2 = m_Options.m_clrGradientDefault;

			m_wndColor1.SetColor(clr1);
			m_wndColor2.SetColor(clr2);

			m_bOriginalIsEmpty = FALSE;
		}

		CBCGPColor c1(clr1, clr1 == (COLORREF)-1 ? 0 : 1);
		CBCGPColor c2(clr2, clr2 == (COLORREF)-1 ? 0 : 1);

		m_brush.SetColors(c1, c2, type, (double)m_wndOpacity.GetPos () / 100.0);
	}

	m_wndColor1.EnableWindow(!bIsEmpty);
	m_wndColor2.EnableWindow(bTwoColors && !bIsEmpty);
	m_wndButtonSwap.EnableWindow(bTwoColors && !bIsEmpty);
	m_wndOpacity.EnableWindow(!bIsEmpty);

	m_wndImage.RedrawWindow();
}

BOOL CBCGPEditBrushDlg::OnInitDialog() 
{
	CBCGPLocalResource locaRes;

	CBCGPDialog::OnInitDialog();

	if (m_brush.HasTextureImage())
	{
		// You cannot use this dialog for texture brushes
		TRACE0("CBCGPEditBrushDlg cannot be used with textured brushes\n");
		ASSERT(FALSE);
		EndDialog(IDCANCEL);
		return TRUE;
	}

#ifdef _BCGSUITE_
	CPalette palette;
	CBCGPColor::CreatePalette(palette);
#endif

	m_wndOpacity.ModifyStyle(TBS_NOTICKS, 0);
	m_wndOpacity.SetRange(0, 100);
	m_wndOpacity.SetTic(0);
	m_wndOpacity.SetTic(50);
	m_wndOpacity.SetTic(100);
	m_wndOpacity.SetPos ((int)(m_brush.GetOpacity() * 100.0));

	m_wndColor1.EnableOtherButton(m_Options.m_strOtherColorLabel);
#ifdef _BCGSUITE_
	m_wndColor1.SetPalette(&palette);
#else
	m_wndColor1.SetColors (CBCGPColor::GetRGBArray());
#endif
	m_wndColor1.SetColumnsNumber (14);
	m_wndColor1.EnableAutomaticButton(m_Options.m_strEmptyColorLabel, 0xFFFFFFFF);
	
	CBCGPColor c1 = m_brush.GetColor();
	m_wndColor1.SetColor(c1.a == 0. ? -1 : c1);

	m_wndColor2.EnableOtherButton(m_Options.m_strOtherColorLabel);
#ifdef _BCGSUITE_
	m_wndColor2.SetPalette(&palette);
#else
	m_wndColor2.SetColors (CBCGPColor::GetRGBArray());
#endif
	m_wndColor2.SetColumnsNumber (14);
	m_wndColor2.EnableAutomaticButton(m_Options.m_strEmptyColorLabel, 0xFFFFFFFF);

	CBCGPColor c2 = m_brush.GetGradientColor();
	m_wndColor2.SetColor(c2.a == 0. ? -1 : c2);

	m_wndButtonSwap.SetImage(IDB_BCGBARRES_SWAP);

	m_wndImage.m_pBrush = &m_brush;

	OnUpdateBrush();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CBCGPEditBrushDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	CBCGPDialog::OnHScroll(nSBCode, nPos, pScrollBar);
	if (pScrollBar == (CScrollBar*)&m_wndOpacity)
	{
		OnUpdateBrush();
	}
}

void CBCGPEditBrushDlg::OnOK() 
{
	m_brushRes = m_brush;
	CBCGPDialog::OnOK();
}


void CBCGPEditBrushDlg::OnSwapColors() 
{
	COLORREF color1 = m_wndColor1.GetColor ();
	COLORREF color2 = m_wndColor2.GetColor ();
	
	m_wndColor1.SetColor(color2);
	m_wndColor2.SetColor(color1);

	OnUpdateBrush();
}

CBCGPEditBrushOptions::CBCGPEditBrushOptions()
{
#ifndef _BCGSUITE_
	CBCGPLocalResource locaRes;

	m_strLabel.LoadString(IDS_BCGBARRES_BRUSH_LABELBRUSH);
	m_strEmptyColorLabel.LoadString(IDS_BCGBARRES_BRUSH_LABELEMPTYCOLOR);
	m_strOtherColorLabel.LoadString(IDS_BCGBARRES_BRUSH_LABELOTHERCOLOR);
#else
	m_strLabel = _T("Brush");
	m_strEmptyColorLabel = _T("No color");
	m_strOtherColorLabel = _T("Other");
#endif

	m_clrDefault = RGB(192, 192, 192);
	m_clrGradientDefault = RGB(255, 255, 255);
}
