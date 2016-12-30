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
// BCGPBrushButton.cpp : implementation file
//

#include "stdafx.h"
#include "bcgprores.h"
#include "BCGPBrushButton.h"
#include "BCGPDrawManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBCGPBrushButton

IMPLEMENT_DYNAMIC(CBCGPBrushButton, CBCGPButton)

CBCGPBrushButton::CBCGPBrushButton()
{
	m_bInCommand = FALSE;
	m_sizeImage = CSize(16, 16);
	m_bIsFirstDraw = TRUE;
	m_bDontAutoGrayImage = TRUE;

	UpdateBitmap(FALSE);
}

CBCGPBrushButton::~CBCGPBrushButton()
{
}

BEGIN_MESSAGE_MAP(CBCGPBrushButton, CBCGPButton)
	//{{AFX_MSG_MAP(CBCGPBrushButton)
	ON_CONTROL_REFLECT_EX(BN_CLICKED, OnClicked)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGPBrushButton message handlers

BOOL CBCGPBrushButton::OnClicked() 
{
	ASSERT_VALID (this);

	if (m_bInCommand)
	{
		return FALSE;
	}

	if (!IsWindowEnabled ())
	{
		return TRUE;
	}

	CBCGPEditBrushDlg dlg(m_Brush, GetParent(), &m_Options);
	dlg.EnableVisualManagerStyle(m_bVisualManagerStyle, m_bVisualManagerStyle);

	if (dlg.DoModal () == IDOK)
	{
		UpdateBitmap(TRUE);

		CWnd* pParent = GetParent ();
		if (pParent != NULL)
		{
			m_bInCommand = TRUE;
			pParent->SendMessage(WM_COMMAND, MAKEWPARAM (GetDlgCtrlID(), BN_CLICKED), (LPARAM)m_hWnd);
			m_bInCommand = FALSE;
		}
	}

	return TRUE;
}
//*******************************************************************************************************
void CBCGPBrushButton::SetBrush(const CBCGPBrush& brush)
{
	if (m_Brush == brush)
	{
		return;
	}

	m_Brush = brush;
	UpdateBitmap(TRUE);
}
//*******************************************************************************************************
void CBCGPBrushButton::UpdateBitmap(BOOL bRedraw)
{
	m_Image.Clear();

	if (GetSafeHwnd() != NULL)
	{
		CRect rectClient;
		GetClientRect(rectClient);

		int nImageSize = min(rectClient.Width(), rectClient.Height()) - 6;
		m_sizeImage = CSize(nImageSize, nImageSize);
	}

	m_Image.SetImageSize(m_sizeImage);

	CBCGPGraphicsManager* pGM = CBCGPGraphicsManager::CreateInstance();
	if (pGM != NULL)
	{
		pGM->EnableTransparentGradient();

		HBITMAP hbmp = CBCGPDrawManager::CreateBitmap_32(m_sizeImage, NULL);
		if (hbmp != NULL)
		{
			CBCGPRect rect(CBCGPPoint(), m_sizeImage);

			CDC dcMem;
			dcMem.CreateCompatibleDC(NULL);

			HBITMAP hbmpOld = (HBITMAP) dcMem.SelectObject(hbmp);

			pGM->BindDC(&dcMem, rect);
			pGM->BeginDraw();

			pGM->Clear();

			rect.DeflateRect(1, 1);

			pGM->FillRectangle(rect, CBCGPBrush(CBCGPColor::White));
			pGM->FillRectangle(rect, m_Brush);

			rect.right--;
			rect.bottom--;

			pGM->DrawRectangle(rect, CBCGPBrush(CBCGPColor::Gray));
			pGM->EndDraw();

			dcMem.SelectObject (hbmpOld);

			pGM->BindDC(NULL);

			m_Image.AddImage(hbmp, TRUE);

			::DeleteObject(hbmp);
		}

		delete pGM;
	}

	if (bRedraw && GetSafeHwnd() != NULL)
	{
		RedrawWindow();
	}
}
//*******************************************************************************************************
void CBCGPBrushButton::OnDrawFocusRect (CDC* pDC, const CRect& rectClient)
{
	CRect rectFocus = rectClient;

	CString strText;
	GetWindowText(strText);

	if (!strText.IsEmpty())
	{
		if (m_bTopImage)
		{
			rectFocus.top += m_sizeImage.cy + 3 * GetVertMargin () / 4;
		}
		else if (m_bRighImage)
		{
			rectFocus.right -= m_sizeImage.cx + 3 * GetImageHorzMargin() / 4;
		}
		else
		{
			rectFocus.left +=  m_sizeImage.cx + 3 * GetImageHorzMargin() / 4;
		}
	}

	CBCGPButton::OnDrawFocusRect(pDC, rectFocus);
}
//*******************************************************************************************************
void CBCGPBrushButton::OnDraw (CDC* pDC, const CRect& rect, UINT uiState)
{
	if (m_bIsFirstDraw)
	{
		UpdateBitmap(FALSE);
		m_bIsFirstDraw = FALSE;
	}

	CBCGPButton::OnDraw(pDC, rect, uiState);
}
