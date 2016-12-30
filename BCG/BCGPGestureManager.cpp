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
// BCGPGestureManager.cpp: implementation of the CBCGPGestureManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "bcgcbpro.h"
#include "BCGPGestureManager.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#ifndef GF_BEGIN
#define GF_BEGIN	0x00000001
#endif

#ifndef GF_END
#define GF_END		0x00000004
#endif

#ifndef GF_INERTIA
#define GF_INERTIA	0x00000002
#endif

BCGCBPRODLLEXPORT CBCGPGestureManager bcgpGestureManager;

/////////////////////////////////////////////////////////////////////////////
// CBCGPGestureConfig functions

CBCGPGestureConfig::CBCGPGestureConfig()
{
	m_nConfigs = BCGP_GID_PRESSANDTAP - BCGP_GID_ZOOM + 1;
	m_pConfigs = new BCGP_GESTURECONFIG[m_nConfigs];

	// Enable all gesture features by default:
	for (int i = 0; i < m_nConfigs; i++)
	{
		m_pConfigs[i].dwID = BCGP_GID_ZOOM + i;
		m_pConfigs[i].dwWant = (m_pConfigs[i].dwID == BCGP_GID_PAN) ? 0 : BCGP_GC_ALLGESTURES;
		m_pConfigs[i].dwBlock = 0;
	}

	// Disable rotate:
	EnableRotate(FALSE);

	// By default Pan supports Gutter, Inertia only and Single Finger Vertically:
	EnablePan(TRUE, BCGP_GC_PAN_WITH_GUTTER | BCGP_GC_PAN_WITH_INERTIA | BCGP_GC_PAN_WITH_SINGLE_FINGER_VERTICALLY);
}
//************************************************************************************
CBCGPGestureConfig::~CBCGPGestureConfig()
{
	delete[] m_pConfigs;
}
//************************************************************************************
BOOL CBCGPGestureConfig::Modify(DWORD dwID, DWORD dwWant, DWORD dwBlock)
{
	ASSERT_VALID(this);

	ASSERT((dwWant & dwBlock) == 0);	// Should be exclusive!

	for (int i = 0; i < m_nConfigs; i++)
	{
		if (m_pConfigs[i].dwID == dwID)
		{
			m_pConfigs[i].dwWant |= dwWant;
			m_pConfigs[i].dwBlock |= dwBlock;

			// Clean-up dwWant from block and dwBlock from want:
			m_pConfigs[i].dwWant &= ((~dwBlock) & (DWORD)-1);
			m_pConfigs[i].dwBlock &= ((~dwWant) & (DWORD)-1);

			return TRUE;
		}
	}

	// Unknown or unsupported ID
	ASSERT(FALSE);
	return FALSE;
}
//************************************************************************************
DWORD CBCGPGestureConfig::Get(DWORD dwID, BOOL bWant) const
{
	ASSERT_VALID(this);

	for (int i = 0; i < m_nConfigs; i++)
	{
		if (m_pConfigs[i].dwID == dwID)
		{
			return bWant ? m_pConfigs[i].dwWant : m_pConfigs[i].dwBlock;
		}
	}

	// Unknown or unsupported ID
	ASSERT(FALSE);
	return (UINT)-1;
}
//************************************************************************************
void CBCGPGestureConfig::EnableZoom(BOOL bEnable)
{
	Modify(BCGP_GID_ZOOM, bEnable ? BCGP_GC_ZOOM : 0, bEnable ? 0 : BCGP_GC_ZOOM);
}
//************************************************************************************
void CBCGPGestureConfig::EnableRotate(BOOL bEnable)
{
	Modify(BCGP_GID_ROTATE, bEnable ? BCGP_GC_ROTATE : 0, bEnable ? 0 : BCGP_GC_ROTATE);
}
//************************************************************************************
void CBCGPGestureConfig::EnableTwoFingerTap(BOOL bEnable)
{
	Modify(BCGP_GID_TWOFINGERTAP, bEnable ? BCGP_GC_TWOFINGERTAP : 0, bEnable ? 0 : BCGP_GC_TWOFINGERTAP);
}
//************************************************************************************
void CBCGPGestureConfig::EnablePressAndTap(BOOL bEnable)
{
	Modify(BCGP_GID_PRESSANDTAP, bEnable ? BCGP_GC_PRESSANDTAP : 0, bEnable ? 0 : BCGP_GC_PRESSANDTAP);
}
//************************************************************************************
void CBCGPGestureConfig::EnablePan(BOOL bEnable, DWORD dwFlags)
{
	if (!bEnable)
	{
		Modify(BCGP_GID_PAN, 0, BCGP_GC_PAN_WITH_SINGLE_FINGER_VERTICALLY | BCGP_GC_PAN_WITH_SINGLE_FINGER_HORIZONTALLY | BCGP_GC_PAN_WITH_GUTTER | BCGP_GC_PAN_WITH_INERTIA);	// Disable all pan features
		return;
	}

	DWORD dwWant = 0;
	DWORD dwBlock = 0;

	DWORD dwAllFlags[] = { BCGP_GC_PAN_WITH_SINGLE_FINGER_VERTICALLY, BCGP_GC_PAN_WITH_SINGLE_FINGER_HORIZONTALLY, BCGP_GC_PAN_WITH_GUTTER, BCGP_GC_PAN_WITH_INERTIA };

	if ((dwFlags & BCGP_GC_PAN) == BCGP_GC_PAN)
	{
		dwFlags = BCGP_GC_PAN_WITH_SINGLE_FINGER_VERTICALLY | BCGP_GC_PAN_WITH_SINGLE_FINGER_HORIZONTALLY | BCGP_GC_PAN_WITH_GUTTER | BCGP_GC_PAN_WITH_INERTIA;
	}

	for (int i = 0; i < sizeof(dwAllFlags) / sizeof(DWORD); i++)
	{
		if ((dwFlags & dwAllFlags[i]) == dwAllFlags[i])
		{
			dwWant |= dwAllFlags[i];
		}
		else
		{
			dwBlock |= dwAllFlags[i];
		}
	}

	Modify(BCGP_GID_PAN, dwWant, dwBlock);
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGPGestureManager::CBCGPGestureManager()
{
	Resume();
}
//************************************************************************************
CBCGPGestureManager::~CBCGPGestureManager()
{
}
//************************************************************************************
BOOL CBCGPGestureManager::GetGestureInfo(HWND /*hwnd*/, BCGP_HGESTUREINFO hGestureInfo, PBCGP_GESTUREINFO pInfo)
{
	if (m_pfGetGestureInfo == NULL)
	{
		return FALSE;
	}
	
	return (*m_pfGetGestureInfo)(hGestureInfo, pInfo);
}
//************************************************************************************
BOOL CBCGPGestureManager::CloseGestureInfoHandle(BCGP_HGESTUREINFO hGestureInfo)
{
	if (m_pfCloseGestureInfoHandle == NULL)
	{
		return FALSE;
	}
	
	return (*m_pfCloseGestureInfoHandle)(hGestureInfo);
}
//************************************************************************************
BOOL CBCGPGestureManager::SetGestureConfig(HWND hwnd, const CBCGPGestureConfig& config)
{
	if (m_pfSetGestureConfig == NULL)
	{
		return FALSE;
	}
	
	BCGP_GESTURECONFIG* pConfigs = config.m_pConfigs;
	UINT cIDs = (UINT)config.m_nConfigs;
	
	return (*m_pfSetGestureConfig)(hwnd, 0, cIDs, pConfigs, sizeof(BCGP_GESTURECONFIG));
}
//************************************************************************************
BOOL CBCGPGestureManager::GetGestureConfig(HWND hwnd, CBCGPGestureConfig& options)
{
	if (m_pfGetGestureConfig == NULL)
	{
		return FALSE;
	}
	
	BCGP_GESTURECONFIG* pConfigs = options.m_pConfigs;
	UINT& cIDs = (UINT&)options.m_nConfigs;
	
	return (*m_pfGetGestureConfig)(hwnd, 0, 0, &cIDs, pConfigs, sizeof(BCGP_GESTURECONFIG));
}
//************************************************************************************
BOOL CBCGPGestureManager::BeginPanningFeedback(HWND hwnd)
{
	if (m_pfBeginPanningFeedback == NULL)
	{
		return FALSE;
	}
	
	return (*m_pfBeginPanningFeedback)(hwnd);
}
//************************************************************************************
BOOL CBCGPGestureManager::EndPanningFeedback(HWND hwnd, BOOL fAnimateBack)
{
	if (m_pfEndPanningFeedback == NULL)
	{
		return FALSE;
	}
	
	return (*m_pfEndPanningFeedback)(hwnd, fAnimateBack);
}
//************************************************************************************
BOOL CBCGPGestureManager::UpdatePanningFeedback(HWND hwnd, LONG lTotalOverpanOffsetX, LONG lTotalOverpanOffsetY, BOOL fInInertia)
{
	if (m_pfUpdatePanningFeedback == NULL)
	{
		return FALSE;
	}
	
	return (*m_pfUpdatePanningFeedback)(hwnd, lTotalOverpanOffsetX, lTotalOverpanOffsetY, fInInertia);
}
//************************************************************************************
void CBCGPGestureManager::UpdateUser32Wrappers()
{
	m_hinstUser32 = LoadLibrary (_T("USER32.DLL"));
	
	if (m_hinstUser32 != NULL)
	{
		m_pfGetGestureInfo = (BCGP_GETGESTUREINFO)GetProcAddress(m_hinstUser32, "GetGestureInfo");
		m_pfCloseGestureInfoHandle = (BCGP_CLOSEGESTUREINFOHANDLE)GetProcAddress(m_hinstUser32, "CloseGestureInfoHandle");
		m_pfSetGestureConfig = (BCGP_SETGESTURECONFIG)GetProcAddress(m_hinstUser32, "SetGestureConfig");
		m_pfGetGestureConfig = (BCGP_GETGESTURECONFIG)GetProcAddress(m_hinstUser32, "GetGestureConfig");
	}
	else
	{
		m_pfGetGestureInfo = NULL;
		m_pfCloseGestureInfoHandle = NULL;
		m_pfSetGestureConfig = NULL;
		m_pfGetGestureConfig = NULL;
	}
}
//************************************************************************************
void CBCGPGestureManager::UpdateUxThemeWrappers()
{
	m_hinstUXThemeDLL = LoadLibrary (_T("UxTheme.dll"));
	
	if (m_hinstUXThemeDLL != NULL)
	{
		m_pfBeginPanningFeedback = (BCGP_BEGINPANNINGFEEDBACK)::GetProcAddress (m_hinstUXThemeDLL, "BeginPanningFeedback");
		m_pfEndPanningFeedback = (BCGP_ENDPANNINGFEEDBACK)::GetProcAddress (m_hinstUXThemeDLL, "EndPanningFeedback");
		m_pfUpdatePanningFeedback = (BCGP_UPDATEPANNINGFEEDBACK)::GetProcAddress (m_hinstUXThemeDLL, "UpdatePanningFeedback");
	}
	else
	{
		m_pfBeginPanningFeedback = NULL;
		m_pfEndPanningFeedback = NULL;
		m_pfUpdatePanningFeedback = NULL;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPGestureBase

CBCGPGestureBase::CBCGPGestureBase()
{
	m_ptGestureFrom = CPoint(-1, -1);
	m_ulGestureArg = 0;
	m_pCurrentGestureInfo = NULL;
}
//************************************************************************************
CBCGPGestureBase::~CBCGPGestureBase()
{
	if (m_pCurrentGestureInfo != NULL)
	{
		delete m_pCurrentGestureInfo;
	}
}
//************************************************************************************
BOOL CBCGPGestureBase::ProcessGestureEvent(CWnd* pWndThis, WPARAM /*wp*/, LPARAM lp)
{
	if (m_pCurrentGestureInfo == NULL)
	{
		m_pCurrentGestureInfo = new BCGP_GESTUREINFO;
	}
	
	ZeroMemory(m_pCurrentGestureInfo, sizeof(BCGP_GESTUREINFO));
	m_pCurrentGestureInfo->cbSize = sizeof(BCGP_GESTUREINFO);
	
	if (!bcgpGestureManager.GetGestureInfo(pWndThis->GetSafeHwnd(), (BCGP_HGESTUREINFO)lp, m_pCurrentGestureInfo))
	{
		ZeroMemory(m_pCurrentGestureInfo, sizeof(BCGP_GESTUREINFO));
		return TRUE;
	}
	
	CPoint pt(m_pCurrentGestureInfo->ptsLocation.x, m_pCurrentGestureInfo->ptsLocation.y);
	pWndThis->ScreenToClient(&pt);
	
	BOOL bDefaultProcessing = TRUE;
	
	switch (m_pCurrentGestureInfo->dwID)
	{
	case BCGP_GID_BEGIN:
		m_ptGestureFrom = pt;
		m_ulGestureArg = m_pCurrentGestureInfo->ullArguments;
		bcgpGestureManager.CloseGestureInfoHandle((BCGP_HGESTUREINFO)lp);
		return TRUE;
		
	case BCGP_GID_END:
		m_ptGestureFrom = CPoint(-1, -1);
		m_ulGestureArg = 0;
		ZeroMemory(m_pCurrentGestureInfo, sizeof(BCGP_GESTUREINFO));
		return TRUE;
		
	case BCGP_GID_ZOOM:
		if (m_ulGestureArg != 0)
		{
			bDefaultProcessing = !OnGestureEventZoom(pt, (double)(long)m_pCurrentGestureInfo->ullArguments / (long)m_ulGestureArg);
		}
		break;
		
	case BCGP_GID_PAN:
		{
			if (m_pCurrentGestureInfo->dwFlags & GF_BEGIN)
			{
				bcgpGestureManager.BeginPanningFeedback(pWndThis->GetSafeHwnd());
			}
			else if (m_pCurrentGestureInfo->dwFlags & GF_END)
			{
				bcgpGestureManager.EndPanningFeedback(pWndThis->GetSafeHwnd(), TRUE);
			}
			
			CSize sizeOverPan(0, 0);
			bDefaultProcessing = !OnGestureEventPan(m_ptGestureFrom, pt, sizeOverPan);
			
			if (sizeOverPan.cx != 0 || sizeOverPan.cy != 0)
			{
				bcgpGestureManager.UpdatePanningFeedback(pWndThis->GetSafeHwnd(), (LONG)sizeOverPan.cx, (LONG)sizeOverPan.cy, m_pCurrentGestureInfo->dwFlags & GF_INERTIA);
			}
		}
		break;
		
	case BCGP_GID_ROTATE:
		if ((m_pCurrentGestureInfo->dwFlags & GF_BEGIN) == 0)
		{
			bDefaultProcessing = !OnGestureEventRotate(pt, GID_ROTATE_ANGLE_FROM_ARGUMENT(LOWORD(m_ulGestureArg - m_pCurrentGestureInfo->ullArguments)));
		}
		break;
		
	case BCGP_GID_TWOFINGERTAP:
		bDefaultProcessing = !OnGestureEventTwoFingerTap(pt);
		break;
		
	case BCGP_GID_PRESSANDTAP:
		bDefaultProcessing = !OnGestureEventPressAndTap(pt, (long)m_pCurrentGestureInfo->ullArguments);
		break;
	}
	
	m_ptGestureFrom = pt;
	m_ulGestureArg = m_pCurrentGestureInfo->ullArguments;
	
	if (!bDefaultProcessing)
	{
		bcgpGestureManager.CloseGestureInfoHandle((BCGP_HGESTUREINFO)lp);
		return FALSE;
	}
	
	return TRUE;
}
//******************************************************************************************************************
BOOL CBCGPGestureBase::OnGestureEventZoom(const CPoint& ptCenter, double dblZoomFactor)
{
	UNUSED_ALWAYS(ptCenter);
	UNUSED_ALWAYS(dblZoomFactor);

	return FALSE;
}
//******************************************************************************************************************
BOOL CBCGPGestureBase::OnGestureEventPan(const CPoint& ptFrom, const CPoint& ptTo, CSize& sizeOverPan)
{
	UNUSED_ALWAYS(ptFrom);
	UNUSED_ALWAYS(ptTo);
	UNUSED_ALWAYS(sizeOverPan);

	return FALSE;
}
//******************************************************************************************************************
BOOL CBCGPGestureBase::OnGestureEventRotate(const CPoint& ptCenter, double dblAngle)
{
	UNUSED_ALWAYS(ptCenter);
	UNUSED_ALWAYS(dblAngle);

	return FALSE;
}
//******************************************************************************************************************
BOOL CBCGPGestureBase::OnGestureEventTwoFingerTap(const CPoint& ptCenter)
{
	UNUSED_ALWAYS(ptCenter);
	
	return FALSE;
}
//******************************************************************************************************************
BOOL CBCGPGestureBase::OnGestureEventPressAndTap(const CPoint& ptPress, long lDelta)
{
	UNUSED_ALWAYS(ptPress);
	UNUSED_ALWAYS(lDelta);
	
	return FALSE;
}
