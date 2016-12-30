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

// RebarManager.cpp: implementation of the CBCGPRebarState class.
// By Nick Hodapp
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BCGPRegistry.h"
#include "BCGPReBar.h"
#include "BCGPRebarState.h"
#include "BCGPDockingControlBar.h"

static const CString strRebarKeyFmt = _T("BCGRebar-%ld");
static const CString strRebarKey	= _T("RBI");
static const CString strRebarId		= _T("IDs");
static const CString strRebarLocked	= _T("Locked");

BOOL CBCGPRebarState::LoadRebarStateProc(HWND hwnd, LPARAM lParam)
{
	// determine if this is a rebar:
	CWnd* pWnd = CWnd::FromHandle(hwnd);
	if (!pWnd->IsKindOf(RUNTIME_CLASS(CBCGPReBar)))
	{
		return TRUE;
	}

	CReBarCtrl& rc = reinterpret_cast<CBCGPReBar*>(pWnd)->GetReBarCtrl();

	// retrieve our registry section:
	CString strRegSection = reinterpret_cast<LPCTSTR>(lParam);

	CString strRebar;
	strRebar.Format(strRebarKeyFmt, GetWindowLong(rc.GetSafeHwnd(), GWL_ID));

	strRegSection += strRebar;

	CBCGPRegistrySP regSP;
	CBCGPRegistry& reg = regSP.Create (FALSE, TRUE);

	if (!reg.Open (strRegSection))
	{
		return FALSE;
	}

	UINT ulBands = 0;

	// attempt to load this rebar:

	REBARBANDINFO* aBandInfo = NULL;
	if (!reg.Read (strRebarKey, reinterpret_cast<BYTE**>(&aBandInfo), &ulBands))
	{
		delete [] aBandInfo;
		return TRUE;
	}

	LONG* aBandIds = NULL;
	if (!reg.Read (strRebarId, reinterpret_cast<BYTE**>(&aBandIds),  &ulBands))
	{
		delete [] aBandInfo;
		delete [] aBandIds;
		return TRUE;
	}

	// band count should be identical
	ulBands /= sizeof(LONG);

	if (ulBands != rc.GetBandCount())
	{
		delete [] aBandInfo;
		delete [] aBandIds;
		return TRUE;
	}

	// reorder the bands:
	REBARBANDINFO rbi;
	for (UINT i = 0 ; i < ulBands ; i++)
	{
		// check all bands (in a release build the assert above won't fire if there's a mixup
		// and we'll happily do our best)
		for (UINT j = i; j < rc.GetBandCount (); j++)
		{
			memset(&rbi, 0, sizeof(rbi));				 
			rbi.cbSize = sizeof(rbi);
			rbi.fMask = RBBIM_CHILD;
			rc.GetBandInfo(j, &rbi);
			if (aBandIds[i] != GetWindowLong(rbi.hwndChild, GWL_ID))
				continue;

			CBCGPDockingControlBar* pBar = DYNAMIC_DOWNCAST(CBCGPDockingControlBar, CWnd::FromHandlePermanent(rbi.hwndChild));
			if (pBar != NULL && ((aBandInfo[i].fStyle) & RBBS_HIDDEN) == 0)
			{
				ASSERT_VALID(pBar);
				pBar->m_bRecentVisibleState = TRUE;
			}

			if (i != j)
				rc.MoveBand(j, i);

			rc.SetBandInfo(i, &aBandInfo[i]);

			break;
		}
	}

	delete [] aBandInfo;
	delete [] aBandIds;

	reg.Read (strRebarLocked, reinterpret_cast<CBCGPReBar*>(pWnd)->m_bLocked);

	return TRUE;
}
//**********************************************************************************
BOOL CBCGPRebarState::SaveRebarStateProc(HWND hwnd, LPARAM lParam)
{
	// determine if this is a rebar:
	CWnd* pWnd = CWnd::FromHandle(hwnd);
	if (!pWnd->IsKindOf(RUNTIME_CLASS(CBCGPReBar)))
	{
		return TRUE;
	}

	CReBarCtrl& rc = reinterpret_cast<CBCGPReBar*>(pWnd)->GetReBarCtrl();

	//-------------------------------
	// retrieve our registry section:
	//-------------------------------
	CString strRegSection = reinterpret_cast<LPCTSTR>(lParam);

	CString strRebar;
	strRebar.Format(strRebarKeyFmt, GetWindowLong(rc.GetSafeHwnd(), GWL_ID));

	strRegSection += strRebar;

	CBCGPRegistrySP regSP;
	CBCGPRegistry& reg = regSP.Create (FALSE, FALSE);

	if (!reg.CreateKey (strRegSection))
	{
		return FALSE;
	}

	UINT ulBands = rc.GetBandCount ();
	if (ulBands == 0)
	{
		return TRUE;
	}

	REBARBANDINFO* aBandInfo = new REBARBANDINFO[ulBands];
	LONG*          aBandIds  = new LONG[ulBands];
	if (NULL == aBandInfo || NULL == aBandIds)
	{
		delete [] aBandInfo;
		delete [] aBandIds;
		return TRUE;
	}

	memset(aBandInfo, 0, ulBands * globalData.GetRebarBandInfoSize ());
	for (UINT i = 0 ; i < rc.GetBandCount() ; i++)
	{
		REBARBANDINFO& rbi = aBandInfo [i];
		rbi.cbSize = globalData.GetRebarBandInfoSize ();
		rbi.fMask = RBBIM_CHILD | RBBIM_ID | RBBIM_CHILDSIZE | RBBIM_IDEALSIZE | RBBIM_SIZE | RBBIM_STYLE | RBBIM_HEADERSIZE;
		rc.GetBandInfo(i, &aBandInfo[i]);

		// apparently fixed size bands mis-report their cxMinChildSize:
		rbi.cxMinChild += rbi.fStyle & RBBS_FIXEDSIZE ? 4 : 0;

		aBandIds[i] = GetWindowLong(rbi.hwndChild, GWL_ID);
		rbi.hwndChild = 0;
		rbi.fMask ^= RBBIM_CHILD;
	}

	reg.Write (strRebarKey, reinterpret_cast<BYTE*>(aBandInfo), ulBands * globalData.GetRebarBandInfoSize ());
	reg.Write (strRebarId, reinterpret_cast<BYTE*>(aBandIds),  ulBands * sizeof(LONG));
	reg.Write (strRebarLocked, reinterpret_cast<CBCGPReBar*>(pWnd)->m_bLocked);

	delete [] aBandInfo;
	delete [] aBandIds;

	return TRUE;
}
//**********************************************************************************
void CBCGPRebarState::LoadState (CString& strRegKey, CFrameWnd* pFrrame)
{
	ASSERT_VALID (pFrrame);
	EnumChildWindows(pFrrame->GetSafeHwnd(), LoadRebarStateProc, (LPARAM)(LPCTSTR)strRegKey);


}
//**********************************************************************************
void CBCGPRebarState::SaveState(CString& strRegKey, CFrameWnd* pFrrame)
{
	ASSERT_VALID (pFrrame);
	EnumChildWindows(pFrrame->GetSafeHwnd(), SaveRebarStateProc, (LPARAM)(LPCTSTR)strRegKey);
}
