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
// BCGPRibbonBackstagePageRecent.cpp : implementation file
//

#include "stdafx.h"
#include "bcgprores.h"
#include "BCGPLocalResource.h"
#include "BCGPRibbonBackstagePageRecent.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifndef BCGP_EXCLUDE_RIBBON

/////////////////////////////////////////////////////////////////////////////
// CBCGPRibbonBackstagePageRecent dialog

IMPLEMENT_DYNCREATE(CBCGPRibbonBackstagePageRecent, CBCGPDialog)

CBCGPRibbonBackstagePageRecent::CBCGPRibbonBackstagePageRecent(CWnd* pParent /*=NULL*/)
	: CBCGPDialog(CBCGPRibbonBackstagePageRecent::IDD, pParent)
{
	//{{AFX_DATA_INIT(CBCGPRibbonBackstagePageRecent)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	EnableLayout();
	m_wndRecentFolders.SetFoldersMode();

	m_bIsLocal = TRUE;
	m_nFlags = 0xFFFF;
}

void CBCGPRibbonBackstagePageRecent::SetFlags(UINT nFlags)
{
	ASSERT(GetSafeHwnd() == NULL);
	m_nFlags = nFlags;
}

void CBCGPRibbonBackstagePageRecent::DoDataExchange(CDataExchange* pDX)
{
	CBCGPDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBCGPRibbonBackstagePageRecent)
	DDX_Control(pDX, IDC_BCGBARRES_RECENT_FILES, m_wndRecentFiles);
	DDX_Control(pDX, IDC_BCGBARRES_RECENT_FOLDERS, m_wndRecentFolders);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CBCGPRibbonBackstagePageRecent, CBCGPDialog)
	//{{AFX_MSG_MAP(CBCGPRibbonBackstagePageRecent)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_PRINTCLIENT, OnPrintClient)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGPRibbonBackstagePageRecent message handlers

void CBCGPRibbonBackstagePageRecent::HideAndDisable(UINT nID)
{
	CWnd* pCtrl = GetDlgItem(nID);
	if (pCtrl != NULL)
	{
		pCtrl->EnableWindow(FALSE);
		pCtrl->ShowWindow(SW_HIDE);
	}
}

BOOL CBCGPRibbonBackstagePageRecent::OnInitDialog() 
{
	CBCGPDialog::OnInitDialog();
	
	CBCGPStaticLayout* pLayout = (CBCGPStaticLayout*)GetLayout();
	ASSERT_VALID(pLayout);

	BOOL bShowFolders = (m_nFlags & BCGP_SHOW_FOLDERS_LIST) != 0;
	BOOL bShowPins = (m_nFlags & BCGP_SHOW_PINS) != 0;

	m_wndRecentFiles.EnablePins(bShowPins);
	m_wndRecentFiles.FillList();

	if (bShowFolders)
	{
		pLayout->AddAnchor(IDC_BCGBARRES_RECENT_FILES, CBCGPStaticLayout::e_MoveTypeNone, CBCGPStaticLayout::e_SizeTypeBoth, CSize(0, 0), CSize(50, 100));
		pLayout->AddAnchor(IDC_BCGBARRES_SEPARATOR1, CBCGPStaticLayout::e_MoveTypeNone, CBCGPStaticLayout::e_SizeTypeHorz, CSize(0, 0), CSize(50, 100));

		m_wndRecentFolders.EnablePins(bShowPins);
		m_wndRecentFolders.FillList();
	}
	else
	{
		CRect rectRight;
		m_wndRecentFolders.GetWindowRect(&rectRight);

		CRect rectLeft;
		m_wndRecentFiles.GetWindowRect(&rectLeft);

		rectLeft.right = rectRight.right;

		m_wndRecentFiles.SetWindowPos(NULL, -1, -1, rectLeft.Width(), rectLeft.Height(),
			SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE);

		CWnd* pWndSep = GetDlgItem(IDC_BCGBARRES_SEPARATOR1);
		if (pWndSep->GetSafeHwnd() != NULL)
		{
			CRect rectSep;
			pWndSep->GetWindowRect(&rectSep);

			rectSep.right = rectRight.right;

			pWndSep->SetWindowPos(NULL, -1, -1, rectSep.Width(), rectSep.Height(),
				SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE);
		}

		pLayout->AddAnchor(IDC_BCGBARRES_RECENT_FILES, CBCGPStaticLayout::e_MoveTypeNone, CBCGPStaticLayout::e_SizeTypeBoth);
		pLayout->AddAnchor(IDC_BCGBARRES_SEPARATOR1, CBCGPStaticLayout::e_MoveTypeNone, CBCGPStaticLayout::e_SizeTypeHorz);
	}

	if (bShowFolders)
	{
		pLayout->AddAnchor(IDC_BCGBARRES_LABEL2, CBCGPStaticLayout::e_MoveTypeHorz, CBCGPStaticLayout::e_SizeTypeNone, CSize(50, 0), CSize(50, 100));
		pLayout->AddAnchor(IDC_BCGBARRES_SEPARATOR2, CBCGPStaticLayout::e_MoveTypeHorz, CBCGPStaticLayout::e_SizeTypeHorz, CSize(50, 0), CSize(50, 100));
		pLayout->AddAnchor(IDC_BCGBARRES_RECENT_FOLDERS, CBCGPStaticLayout::e_MoveTypeHorz, CBCGPStaticLayout::e_SizeTypeBoth, CSize(50, 0), CSize(50, 100));
		
		pLayout->AddAnchor(IDC_BCGBARRES_SEPARATOR3, CBCGPStaticLayout::e_MoveTypeHorz, CBCGPStaticLayout::e_SizeTypeVert, CSize(50, 100), CSize(50, 100));
	}
	else
	{
		HideAndDisable(IDC_BCGBARRES_LABEL2);
		HideAndDisable(IDC_BCGBARRES_SEPARATOR2);
		HideAndDisable(IDC_BCGBARRES_RECENT_FOLDERS);
		HideAndDisable(IDC_BCGBARRES_SEPARATOR3);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CBCGPRibbonBackstagePageRecent::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	OnDraw(&dc);
}

void CBCGPRibbonBackstagePageRecent::OnDraw(CDC* pDC)
{
	ASSERT_VALID(pDC);

	pDC->SetBkMode(TRANSPARENT);
	
	CFont* pOldFont = pDC->SelectObject(&globalData.fontCaption);
	pDC->SetTextColor(CBCGPVisualManager::GetInstance ()->GetRibbonBackstageTextColor());

	BOOL bShowFolders = (m_nFlags & BCGP_SHOW_FOLDERS_LIST) != 0;
	int nSteps = bShowFolders ? 2 : 1;

	for (int i = 0; i < nSteps; i++)
	{
		UINT nID = (i == 0) ? IDC_BCGBARRES_LABEL1 : IDC_BCGBARRES_LABEL2;

		CWnd* pCtrl = GetDlgItem(nID);
		if (pCtrl != NULL)
		{
			CRect rectLabel;

			GetDlgItem(nID)->GetWindowRect(rectLabel);
			ScreenToClient(rectLabel);

			CString strLabel;
			GetDlgItem(nID)->GetWindowText(strLabel);

			pDC->DrawText(strLabel, rectLabel, DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOCLIP);
		}
	}

	pDC->SelectObject(pOldFont);
}

LRESULT CBCGPRibbonBackstagePageRecent::OnPrintClient(WPARAM wp, LPARAM lp)
{
	if (lp & PRF_CLIENT)
	{
		CDC* pDC = CDC::FromHandle((HDC) wp);
		ASSERT_VALID(pDC);
		
		OnDraw(pDC);
	}
	
	return 0;
}

#endif // BCGP_EXCLUDE_RIBBON
