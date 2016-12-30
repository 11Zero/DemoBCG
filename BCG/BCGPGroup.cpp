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
// BCGPGroup.cpp : implementation file
//

#include "stdafx.h"
#include "BCGPGroup.h"
#include "Bcgglobals.h"
#include "BCGPVisualManager.h"
#include "BCGPDlgImpl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBCGPGroup

IMPLEMENT_DYNAMIC(CBCGPGroup, CButton)

CBCGPGroup::CBCGPGroup()
{
	m_bVisualManagerStyle = FALSE;
	m_bOnGlass = FALSE;
	m_hFont	= NULL;
}

CBCGPGroup::~CBCGPGroup()
{
}

#ifndef WM_UPDATEUISTATE
#define	WM_UPDATEUISTATE 0x0128
#endif

BEGIN_MESSAGE_MAP(CBCGPGroup, CButton)
	//{{AFX_MSG_MAP(CBCGPGroup)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_ENABLE()
	//}}AFX_MSG_MAP
	ON_REGISTERED_MESSAGE(BCGM_ONSETCONTROLVMMODE, OnBCGSetControlVMMode)
	ON_REGISTERED_MESSAGE(BCGM_ONSETCONTROLAERO, OnBCGSetControlAero)
	ON_MESSAGE(WM_UPDATEUISTATE, OnUpdateUIState)
	ON_MESSAGE(WM_SETTEXT, OnSetText)
	ON_MESSAGE(WM_PRINTCLIENT, OnPrintClient)
	ON_MESSAGE(WM_SETFONT, OnSetFont)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGPGroup message handlers

BOOL CBCGPGroup::OnEraseBkgnd(CDC* /*pDC*/) 
{
#ifndef _BCGSUITE_
	if (globalData.bIsWindows9x)
	{
		return (BOOL)Default();
	}
#endif
	return TRUE;
}
//**************************************************************************
void CBCGPGroup::OnPaint() 
{
#ifndef _BCGSUITE_
	if (globalData.bIsWindows9x)
	{
		Default();
		return;
	}
#endif

	if (!m_bVisualManagerStyle && !m_bOnGlass)
	{
		Default();
		return;
	}

	CPaintDC dc(this); // device context for painting
	OnDraw(&dc);
}
//*******************************************************************************
void CBCGPGroup::OnDraw(CDC* pDCIn)
{
	ASSERT_VALID(pDCIn);

	CBCGPMemDC* pMemDC = NULL;
	CDC* pDC = pDCIn;
	
	if (m_bOnGlass)
	{
		pMemDC = new CBCGPMemDC (*pDCIn, this);
		pDC = &pMemDC->GetDC ();
		globalData.DrawParentBackground (this, pDC);
	}
	
	CRect rectClient;
	GetClientRect (rectClient);
	
	CString strName;
	GetWindowText (strName);
	
	if (m_hFont != NULL && ::GetObjectType (m_hFont) != OBJ_FONT)
	{
		m_hFont = NULL;
	}

	CFont* pOldFont = m_hFont == NULL ?
		(CFont*) pDC->SelectStockObject (DEFAULT_GUI_FONT) :
		pDC->SelectObject (CFont::FromHandle (m_hFont));
	ASSERT(pOldFont != NULL);
	
	pDC->SetBkMode (TRANSPARENT);
	pDC->SetTextColor (globalData.clrBarText);
	
	CBCGPVisualManager::GetInstance ()->OnDrawGroup (pDC, this, rectClient, strName);
	
	pDC->SelectObject (pOldFont);
	
	if (pMemDC != NULL)
	{
		delete pMemDC;
	}
}
//*******************************************************************************
LRESULT CBCGPGroup::OnPrintClient(WPARAM wp, LPARAM lp)
{
	if (lp & PRF_CLIENT)
	{
		CDC* pDC = CDC::FromHandle((HDC) wp);
		ASSERT_VALID(pDC);

		if (!m_bVisualManagerStyle && !m_bOnGlass)
		{
			return Default();
		}
		
		OnDraw(pDC);
	}

	return 0;
}
//**************************************************************************
LRESULT CBCGPGroup::OnBCGSetControlVMMode (WPARAM wp, LPARAM)
{
	m_bVisualManagerStyle = (BOOL) wp;
	return 0;
}
//**************************************************************************
LRESULT CBCGPGroup::OnBCGSetControlAero (WPARAM wp, LPARAM)
{
	m_bOnGlass = (BOOL) wp;
	return 0;
}
//**************************************************************************
void CBCGPGroup::OnEnable(BOOL bEnable) 
{
	CButton::OnEnable(bEnable);

	if (GetParent () != NULL)
	{
		CRect rect;
		GetWindowRect (rect);

		GetParent ()->ScreenToClient (&rect);
		GetParent ()->RedrawWindow (rect);
	}
}
//**************************************************************************
LRESULT CBCGPGroup::OnUpdateUIState (WPARAM, LPARAM)
{
	return 0;
}
//*****************************************************************************
LRESULT CBCGPGroup::OnSetText (WPARAM, LPARAM)
{
	LRESULT lr = Default ();

	if (GetParent () != NULL)
	{
		CRect rect;
		GetWindowRect (rect);

		GetParent ()->ScreenToClient (&rect);
		GetParent ()->RedrawWindow (rect);
	}

	return lr;
}
//*****************************************************************************
LRESULT CBCGPGroup::OnSetFont (WPARAM wParam, LPARAM)
{
	m_hFont = (HFONT) wParam;
	return Default();
}
