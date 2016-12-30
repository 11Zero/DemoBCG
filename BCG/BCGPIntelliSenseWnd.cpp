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
// BCGPIntelliSenseWnd.cpp : implementation file
//

#include "stdafx.h"

#include "BCGPIntelliSenseWnd.h"
#include "BCGPEditCtrl.h"

#ifndef _BCGPEDIT_STANDALONE
	#include "BCGPGlobalUtils.h"
#endif

#ifndef BCGP_EXCLUDE_EDIT_CTRL

#pragma warning (disable : 4706)

#ifdef _BCGPEDIT_STANDALONE
 #ifdef _AFXDLL
  #define COMPILE_MULTIMON_STUBS
 #endif // _AFXDLL
#endif // _BCGPEDIT_STANDALONE
#include "multimon.h"

#pragma warning (default : 4706)


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBCGPIntelliSenseWnd

IMPLEMENT_DYNCREATE(CBCGPIntelliSenseWnd, CMiniFrameWnd)

CBCGPIntelliSenseWnd::CBCGPIntelliSenseWnd()
{
	m_pParentEditCtrl = NULL;
	m_pLstBoxData = NULL;
}

CBCGPIntelliSenseWnd::~CBCGPIntelliSenseWnd()
{
}


BEGIN_MESSAGE_MAP(CBCGPIntelliSenseWnd, CMiniFrameWnd)
	//{{AFX_MSG_MAP(CBCGPIntelliSenseWnd)
	ON_WM_DESTROY()
	ON_WM_NCHITTEST()
	//}}AFX_MSG_MAP
	ON_MESSAGE (WM_DELETEITEM, DeleteItem)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGPIntelliSenseWnd message handlers
BOOL CBCGPIntelliSenseWnd::Create (CObList& lstData, DWORD dwStyle, CPoint pt, 
								   CWnd* pParentWnd, CFont* pLBFont, CImageList* pImageList, 
								   CRuntimeClass* pLBDataRTC)
{
	ASSERT_VALID (this);
	ASSERT (m_pLstBoxData == NULL);

	m_pParentEditCtrl = DYNAMIC_DOWNCAST (CBCGPEditCtrl, pParentWnd);

	if (m_pParentEditCtrl == NULL)
	{
		return FALSE;
	}

	DWORD dwFinalStyle = dwStyle & ~WS_VISIBLE;
	if (!CMiniFrameWnd::Create (NULL, NULL, dwFinalStyle, 
							CRect (pt.x, pt.y, pt.x + 100, pt.y + 100), pParentWnd))
	{
		return FALSE;
	}

	if (pLBDataRTC != NULL)
	{
		m_pLstBoxData = DYNAMIC_DOWNCAST (CBCGPBaseIntelliSenseLB, 
											pLBDataRTC->CreateObject ());
	}
	else
	{	
		m_pLstBoxData = new CBCGPIntelliSenseLB;
	}

	ASSERT_VALID (m_pLstBoxData);

	m_pLstBoxData->SetImageList (pImageList);

	if (!m_pLstBoxData->Create (WS_CHILD | WS_VSCROLL | LBS_OWNERDRAWVARIABLE | LBS_NOTIFY | LBS_SORT, 
							CRect (0, 0, 100, 100), this, 1))
	{
		return FALSE;
	}

	m_pParentEditCtrl->m_bIntelliSenseMode = TRUE;	
	m_pParentEditCtrl->SetIntelliSenseWnd (this);

	LOGFONT logfont;
	memset (&logfont, 0, sizeof (LOGFONT));
	if (pLBFont == NULL)
	{
		logfont.lfHeight = 8;
		lstrcpy (logfont.lfFaceName, _T ("MS Sans Serif"));
	}
	else
	{
		pLBFont->GetLogFont (&logfont);
	}

	m_lbFont.CreateFontIndirect (&logfont);
	m_pLstBoxData->m_pFont = &m_lbFont; 


	for (POSITION pos = lstData.GetHeadPosition (); pos != NULL;)
	{
		CBCGPIntelliSenseData* pData = 
			(CBCGPIntelliSenseData*) lstData.GetNext (pos);

		ASSERT_VALID (pData);	
		int nIdx = m_pLstBoxData->AddString ((LPCTSTR)pData);
		m_pLstBoxData->SetItemDataPtr (nIdx, pData);
	}

	int nLBWidth = m_pLstBoxData->m_sizeMaxItem.cx;
	if (lstData.GetCount () > CBCGPIntelliSenseLB::m_nNumVisibleItems)
	{
		nLBWidth += GetSystemMetrics (SM_CXVSCROLL);
	}

	m_pLstBoxData->SetWindowPos (NULL, 0, 0, nLBWidth, m_pLstBoxData->m_nLBHeight, 
							 SWP_NOZORDER  | SWP_NOACTIVATE); 

	CSize sizeImage = m_pLstBoxData->GetImageSize (0);
	int nLBOffset = sizeImage.cx + GetSystemMetrics (SM_CXBORDER) + 
					CBCGPIntelliSenseLB::m_nImageToFocusRectSpacing + 
					CBCGPIntelliSenseLB::m_nFocusRectToTextSpacing + 1;

#ifndef _BCGPEDIT_STANDALONE
	CSize szSystemBorder (globalUtils.GetSystemBorders (this));
#else
	CSize szSystemBorder (GetSystemMetrics (SM_CXSIZEFRAME), GetSystemMetrics (SM_CYSIZEFRAME));
#endif
	CRect rectLB (pt.x - nLBOffset, pt.y + m_pParentEditCtrl->GetLineHeight (), 
				pt.x - nLBOffset + nLBWidth + szSystemBorder.cx * 2, 
				pt.y + m_pParentEditCtrl->GetLineHeight () + 
				m_pLstBoxData->m_nLBHeight + szSystemBorder.cy * 2);
	
	CRect rectScreen;

	MONITORINFO mi;
	mi.cbSize = sizeof (MONITORINFO);
	if (GetMonitorInfo (MonitorFromPoint (pt, MONITOR_DEFAULTTONEAREST),
		&mi))
	{
		rectScreen = mi.rcWork;
	}
	else
	{
		::SystemParametersInfo (SPI_GETWORKAREA, 0, &rectScreen, 0);
	}

	int nRectLBWidth = rectLB.Width ();
	int nRectLBHeight = rectLB.Height ();

	if (rectLB.bottom > rectScreen.bottom)
	{
		rectLB.bottom = rectLB.top - m_pParentEditCtrl->GetLineHeight ();
		rectLB.top = rectLB.bottom - nRectLBHeight;
	}

	if (rectLB.right > rectScreen.right)
	{
		rectLB.right = pt.x;
		rectLB.left = rectLB.right - nRectLBWidth;
	}

	SetWindowPos (NULL, rectLB.left, rectLB.top, nRectLBWidth, nRectLBHeight,
					SWP_NOZORDER  | SWP_NOACTIVATE); 
	
	m_pLstBoxData->SetFocus ();

	m_pLstBoxData->ShowWindow (SW_SHOW);
	ShowWindow (SW_SHOW);
	m_pLstBoxData->RedrawWindow ();
	m_pLstBoxData->SelectCurrentWord ();

	return TRUE;
}

void CBCGPIntelliSenseWnd::OnDestroy() 
{
	CWnd* pParentWnd = GetParentEditCtrl ();
	if(pParentWnd->GetSafeHwnd () != NULL)
	{
		pParentWnd->SetFocus ();
	}

	CMiniFrameWnd::OnDestroy ();
}

void CBCGPIntelliSenseWnd::PostNcDestroy() 
{
	if (m_pParentEditCtrl != NULL)
	{
		m_pParentEditCtrl->SetIntelliSenseWnd (NULL);
		m_pParentEditCtrl->m_bIntelliSenseMode = FALSE;	
	}
	
	CMiniFrameWnd::PostNcDestroy();
}

BCGNcHitTestType CBCGPIntelliSenseWnd::OnNcHitTest(CPoint /*point*/) 
{
	return HTCLIENT;
}

LRESULT CBCGPIntelliSenseWnd::DeleteItem (WPARAM /*wParam*/, LPARAM lParam)
{
	LPDELETEITEMSTRUCT lpDeleteItemStruct = (LPDELETEITEMSTRUCT ) lParam;	// item information

	m_pLstBoxData->DeleteItem (lpDeleteItemStruct);
	return TRUE;
}

#endif	// BCGP_EXCLUDE_EDIT_CTRL
