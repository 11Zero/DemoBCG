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
// BCGPDockingCBWrapper.cpp : implementation file
//

#include "stdafx.h"
#include "bcgcbpro.h"
#include "BCGPDockingCBWrapper.h"

#include "RegPath.h"
#include "BCGPRegistry.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static const CString strControlBarProfile = _T ("BCGControlBars");

#define REG_SECTION_FMT					_T("%sBCGPDockingCBWrapper-%d")
#define REG_SECTION_FMT_EX				_T("%sBCGPDockingCBWrapper-%d%x")


IMPLEMENT_SERIAL(CBCGPDockingCBWrapper, CBCGPDockingControlBar, VERSIONABLE_SCHEMA | 2)

/////////////////////////////////////////////////////////////////////////////
// CBCGPDockingCBWrapper

CBCGPDockingCBWrapper::CBCGPDockingCBWrapper()
{
	m_pWnd = NULL;
	m_dwEnabledAlignmentInitial = CBRS_ALIGN_ANY;
	m_rectInitial.SetRect (30, 30, 180, 180);
}


CBCGPDockingCBWrapper::~CBCGPDockingCBWrapper()
{
}


BEGIN_MESSAGE_MAP(CBCGPDockingCBWrapper, CBCGPDockingControlBar)
	//{{AFX_MSG_MAP(CBCGPDockingCBWrapper)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CBCGPDockingCBWrapper message handlers
void CBCGPDockingCBWrapper::OnSize(UINT nType, int cx, int cy) 
{
	CBCGPDockingControlBar::OnSize(nType, cx, cy);
	
	if (m_pWnd != NULL)
	{
		m_pWnd->SetWindowPos (NULL, 0, 0, cx, cy, SWP_NOACTIVATE | SWP_NOZORDER/* | SWP_NOREDRAW*/);
	}
}
//------------------------------------------------------------------------------------
BOOL CBCGPDockingCBWrapper::SetWrappedWnd (CWnd* pWnd)
{
	ASSERT_VALID (pWnd);
	ASSERT(IsWindow (m_hWnd));

	pWnd->SetParent (this);

	m_pWnd = pWnd;

	if (pWnd->IsKindOf (RUNTIME_CLASS (CBCGPBaseControlBar)))
	{
		CBCGPBaseControlBar* pBar = (CBCGPBaseControlBar*) pWnd;
		EnableDocking (pBar->GetEnabledAlignment ());
		m_bRecentVisibleState = pBar->GetRecentVisibleState ();
		SetRestoredFromRegistry (pBar->IsRestoredFromRegistry ());
		if (pWnd->IsKindOf (RUNTIME_CLASS (CBCGPControlBar)))
		{
			m_rectSavedDockedRect = ((CBCGPControlBar*) pBar)->m_rectSavedDockedRect;
		}
	}
	else
	{
		EnableDocking (m_dwEnabledAlignmentInitial);
	}

	return TRUE;
}
//------------------------------------------------------------------------------------
BOOL CBCGPDockingCBWrapper::SaveState (LPCTSTR lpszProfileName, int nIndex, UINT uiID)
{
	CString strProfileName = ::BCGPGetRegPath (strControlBarProfile, lpszProfileName);

	if (nIndex == -1)
	{
		nIndex = GetDlgCtrlID ();
	}

	CString strSection;
	if (uiID == (UINT) -1)
	{
		strSection.Format (REG_SECTION_FMT, strProfileName, nIndex);
	}
	else
	{
		strSection.Format (REG_SECTION_FMT_EX, strProfileName, nIndex, uiID);
	}

	CBCGPRegistrySP regSP;
	CBCGPRegistry& reg = regSP.Create (FALSE, FALSE);

	if (reg.CreateKey (strSection))
	{
		CString strName;
		GetWindowText (strName);
		reg.Write (_T ("BarName"), strName);
		
	}
	return CBCGPDockingControlBar::SaveState (lpszProfileName, nIndex, uiID);	
}
//------------------------------------------------------------------------------------
BOOL CBCGPDockingCBWrapper::LoadState (LPCTSTR lpszProfileName, int nIndex, UINT uiID)
{
	CString strProfileName = ::BCGPGetRegPath (strControlBarProfile, lpszProfileName);

	if (nIndex == -1)
	{
		nIndex = GetDlgCtrlID ();
	}

	CString strSection;
	if (uiID == (UINT) -1)
	{
		strSection.Format (REG_SECTION_FMT, strProfileName, nIndex);
	}
	else
	{
		strSection.Format (REG_SECTION_FMT_EX, strProfileName, nIndex, uiID);
	}

	CBCGPRegistrySP regSP;
	CBCGPRegistry& reg = regSP.Create (FALSE, FALSE);

	if (!reg.Open (strSection))
	{
		return FALSE;
	}

	CString strName;
	reg.Read (_T ("BarName"), strName);
	if (!strName.IsEmpty ())
	{
		SetWindowText (strName);
	}

	return CBCGPDockingControlBar::LoadState (lpszProfileName, nIndex, uiID);	
}
