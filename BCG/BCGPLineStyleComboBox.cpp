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
// BCGPLineStyleComboBox.cpp : implementation file
//

#include "stdafx.h"
#include "BCGPLineStyleComboBox.h"
#include "bcgglobals.h"
#include "BCGPLocalResource.h"
#include "BCGProRes.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define MARGIN_X		5
#define IMAGE_WIDTH		16
#define IMAGE_HEIGHT	4

/////////////////////////////////////////////////////////////////////////////
// CBCGPLineStyleComboBox

IMPLEMENT_DYNAMIC(CBCGPLineStyleComboBox, CBCGPComboBox)

CBCGPLineStyleComboBox::CBCGPLineStyleComboBox()
{
	m_bAutoFillItems = TRUE;
}

CBCGPLineStyleComboBox::~CBCGPLineStyleComboBox()
{
}


BEGIN_MESSAGE_MAP(CBCGPLineStyleComboBox, CBCGPComboBox)
	//{{AFX_MSG_MAP(CBCGPLineStyleComboBox)
	ON_WM_CREATE()
	ON_WM_SYSCOLORCHANGE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGPLineStyleComboBox message handlers

void CBCGPLineStyleComboBox::PreSubclassWindow() 
{
	CBCGPComboBox::PreSubclassWindow();

	_AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
	if (pThreadState->m_pWndInit == NULL)
	{
		Init();
	}
}
//****************************************************************************************
int CBCGPLineStyleComboBox::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CBCGPComboBox::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	Init();
	
	return 0;
}
//****************************************************************************************
void CBCGPLineStyleComboBox::Init()
{
	PrepareImages();

	if (!m_bAutoFillItems)
	{
		return;
	}

	CString strNames;

	{
		CBCGPLocalResource locaRes;
		strNames.LoadString(IDS_BCGBARRES_LINE_STYLES);
	}

	while (!strNames.IsEmpty())
	{
		CString strName;
		int nIndex = strNames.Find(_T('\n'));
		if (nIndex >= 0)
		{
			strName = strNames.Left(nIndex);
			strNames = strNames.Mid(nIndex + 1);
		}
		else
		{
			strName = strNames;
			strNames.Empty();
		}

		if (!strName.IsEmpty())
		{
			AddString(strName);
		}
	}
}
//****************************************************************************************
void CBCGPLineStyleComboBox::PrepareImages()
{
	CBCGPLocalResource locaRes;

	m_imageListStyles.Clear();
	m_imageListStylesSelected.Clear();
	m_imageListStylesDisabled.Clear();

	m_imageListStyles.SetImageSize (CSize(IMAGE_WIDTH, IMAGE_HEIGHT));
	m_imageListStyles.SetTransparentColor(RGB(255, 0, 255));
	m_imageListStyles.Load(IDB_BCGBARRES_LINE_STYLES);

	m_imageListStyles.CopyTo(m_imageListStylesSelected);
	m_imageListStyles.CopyTo(m_imageListStylesDisabled);

#ifndef _BCGSUITE_
	m_imageListStyles.AddaptColors(RGB(0, 0, 0), globalData.clrWindowText);
	m_imageListStylesSelected.AddaptColors(RGB(0, 0, 0), globalData.clrTextHilite);
	m_imageListStylesDisabled.AddaptColors(RGB(0, 0, 0), globalData.clrGrayedText);
#endif
}
//****************************************************************************************
void CBCGPLineStyleComboBox::DrawItem(LPDRAWITEMSTRUCT lpDIS) 
{
	CDC* pDC = CDC::FromHandle (lpDIS->hDC);
	ASSERT_VALID (pDC);

	int nIndexDC = pDC->SaveDC ();

	CRect rect = lpDIS->rcItem;
	int nItem = lpDIS->itemID;

	BOOL bIsSelected = (lpDIS->itemState & ODS_SELECTED);

	HBRUSH brBackground;

	if (!IsWindowEnabled())
	{
		brBackground = GetSysColorBrush (COLOR_WINDOW); 
	}
	else if (bIsSelected)
	{
		brBackground = GetSysColorBrush (COLOR_HIGHLIGHT);
	} 
	else 
	{
		brBackground = GetSysColorBrush (COLOR_WINDOW);
	} 

	pDC->SetBkMode(TRANSPARENT);

	if (lpDIS->itemAction & (ODA_DRAWENTIRE | ODA_SELECT))
	{
		::FillRect(pDC->GetSafeHdc(), &rect, brBackground);
	}

	if (nItem >= 0)
	{
		CBCGPToolBarImages& images = (!IsWindowEnabled()) ? m_imageListStylesDisabled :
			(bIsSelected) ? m_imageListStylesSelected : m_imageListStyles;

		rect.DeflateRect(MARGIN_X, 0);

		int y = max(rect.top, rect.CenterPoint().y - IMAGE_HEIGHT / 2);

		for (int x = rect.left; x < rect.right; x += IMAGE_WIDTH)
		{
			CRect rectImage(x, y, min(rect.right - 1, x + IMAGE_WIDTH), y + IMAGE_HEIGHT);

			images.DrawEx(pDC, rectImage, nItem, 
				CBCGPToolBarImages::ImageAlignHorzLeft, CBCGPToolBarImages::ImageAlignVertTop);
		}
	}

	pDC->RestoreDC (nIndexDC);
}
//****************************************************************************************
void CBCGPLineStyleComboBox::MeasureItem(LPMEASUREITEMSTRUCT lpMIS) 
{
	ASSERT(lpMIS->CtlType == ODT_COMBOBOX);

	CRect rc;
	GetWindowRect (&rc);

	lpMIS->itemWidth = rc.Width();
	lpMIS->itemHeight = globalData.GetTextHeight();
}
//****************************************************************************************
void CBCGPLineStyleComboBox::OnSysColorChange() 
{
	CBCGPComboBox::OnSysColorChange();
	PrepareImages();
}
