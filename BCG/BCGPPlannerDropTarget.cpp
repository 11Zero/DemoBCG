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
// BCGPPlannerDropTarget.cpp : implementation file
//

#include "stdafx.h"
#include "BCGPPlannerDropTarget.h"

#ifndef BCGP_EXCLUDE_PLANNER

#include "BCGPPlannerManagerCtrl.h"
#include "BCGPCalendarBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBCGPPlannerDropTarget


CBCGPPlannerDropTarget::CBCGPPlannerDropTarget()
	: m_pPlanner (NULL)
	, m_pCalendar (NULL)
{
}

CBCGPPlannerDropTarget::~CBCGPPlannerDropTarget()
{
}

BOOL CBCGPPlannerDropTarget::Register (CBCGPPlannerManagerCtrl* pPlanner)
{
	ASSERT (m_pCalendar == NULL);

	m_pPlanner = pPlanner;
	return COleDropTarget::Register (pPlanner);
}

BOOL CBCGPPlannerDropTarget::Register (CBCGPCalendar* pCalendar)
{
	ASSERT (m_pPlanner == NULL);

	if (m_pCalendar != NULL)
	{
		return TRUE;
	}

	m_pCalendar = pCalendar;
	return COleDropTarget::Register (pCalendar);
}

DROPEFFECT CBCGPPlannerDropTarget::OnDragEnter(CWnd* /*pWnd*/, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point) 
{
	ASSERT (m_pPlanner != NULL || m_pCalendar != NULL);

	if (!pDataObject->IsDataAvailable (CBCGPPlannerManagerCtrl::GetClipboardFormat ()))
	{
		return DROPEFFECT_NONE;
	}

	if (m_pPlanner != NULL)
	{
		ASSERT_VALID (m_pPlanner);
		return m_pPlanner->OnDragEnter(pDataObject, dwKeyState, point);
	}
	else if (m_pCalendar != NULL)
	{
		ASSERT_VALID (m_pCalendar);
		return m_pCalendar->OnDragEnter(pDataObject, dwKeyState, point);
	}

	return DROPEFFECT_NONE;
}

void CBCGPPlannerDropTarget::OnDragLeave(CWnd* /*pWnd*/) 
{
	ASSERT (m_pPlanner != NULL || m_pCalendar != NULL);

	if (m_pPlanner != NULL)
	{
		ASSERT_VALID (m_pPlanner);
		m_pPlanner->OnDragLeave ();
	}
	else if (m_pCalendar != NULL)
	{
		ASSERT_VALID (m_pCalendar);
		m_pCalendar->OnDragLeave ();
	}
}

DROPEFFECT CBCGPPlannerDropTarget::OnDragOver(CWnd* /*pWnd*/, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point) 
{
	ASSERT (m_pPlanner != NULL || m_pCalendar != NULL);

	if (!pDataObject->IsDataAvailable (CBCGPPlannerManagerCtrl::GetClipboardFormat ()))
	{
		return DROPEFFECT_NONE;
	}

	if (m_pPlanner != NULL)
	{
		ASSERT_VALID (m_pPlanner);
		return m_pPlanner->OnDragOver(pDataObject, dwKeyState, point);
	}
	else if (m_pCalendar != NULL)
	{
		ASSERT_VALID (m_pCalendar);
		return m_pCalendar->OnDragOver(pDataObject, dwKeyState, point);
	}

	return DROPEFFECT_NONE;
}

DROPEFFECT CBCGPPlannerDropTarget::OnDragScroll(CWnd* pWnd, DWORD dwKeyState, CPoint point)
{
	if (m_pPlanner == NULL)
	{
		return DROPEFFECT_NONE;
	}

	ASSERT_VALID (m_pPlanner);

	DROPEFFECT dropEffect = m_pPlanner->OnDragScroll(dwKeyState, point);

	// DROPEFFECT_SCROLL means do the default
	if (dropEffect != DROPEFFECT_SCROLL)
		return dropEffect;

	// get client rectangle of destination window
	CRect rectClient;
	m_pPlanner->GetDragScrollRect(rectClient);
	CRect rect = rectClient;

	// hit-test against inset region
	UINT nTimerID = MAKEWORD(-1, -1);
	rect.InflateRect(-nScrollInset, -nScrollInset);

	if (rectClient.PtInRect(point) && !rect.PtInRect(point))
	{
		// determine which way to scroll along both X & Y axis
		if (point.x < rect.left)
		{
			nTimerID = MAKEWORD(SB_LINEUP, HIBYTE(nTimerID));
		}
		else if (point.x >= rect.right)
		{
			nTimerID = MAKEWORD(SB_LINEDOWN, HIBYTE(nTimerID));
		}

		if (point.y < rect.top)
		{
			nTimerID = MAKEWORD(LOBYTE(nTimerID), SB_LINEUP);
		}
		else if (point.y >= rect.bottom)
		{
			nTimerID = MAKEWORD(LOBYTE(nTimerID), SB_LINEDOWN);
		}

		ASSERT(nTimerID != MAKEWORD(-1, -1));

		BOOL bEnableScroll = m_pPlanner->OnScroll(nTimerID, 0, FALSE);

		if (!bEnableScroll)
		{
			nTimerID = MAKEWORD(-1, -1);
		}
	}

	if (m_nTimerID == MAKEWORD(-1, -1) && nTimerID != MAKEWORD(-1, -1))
	{
		// send fake OnDragEnter when transition from scroll->normal
		COleDataObject dataObject;
		dataObject.Attach(m_lpDataObject, FALSE);
		OnDragEnter(pWnd, &dataObject, dwKeyState, point);
	}

	if (nTimerID == MAKEWORD(-1, -1))
	{
		if (m_nTimerID != MAKEWORD(-1, -1))
		{
			// send fake OnDragEnter when transition from scroll->normal
			//COleDataObject dataObject;
			//dataObject.Attach(m_lpDataObject, FALSE);
			//OnDragEnter(pWnd, &dataObject, dwKeyState, point);
			m_nTimerID = MAKEWORD(-1, -1);
		}
		
		return DROPEFFECT_NONE;
	}

	// save tick count when timer ID changes
	DWORD dwTick = GetTickCount();
	if (nTimerID != m_nTimerID)
	{
		m_dwLastTick = dwTick;
		m_nScrollDelay = nScrollDelay;
	}

	// scroll if necessary
	if (dwTick - m_dwLastTick > m_nScrollDelay)
	{
		m_pPlanner->OnScroll(nTimerID, 0, TRUE);

		m_dwLastTick = dwTick;
		m_nScrollDelay = nScrollInterval;
	}
	
	if (m_nTimerID == MAKEWORD(-1, -1))
	{
		// send fake OnDragLeave when transitioning from normal->scroll
		//OnDragLeave(pWnd);
	}

	m_nTimerID = nTimerID;
/*
	// check for force link
	if ((dwKeyState & (MK_CONTROL | MK_SHIFT)) == (MK_CONTROL | MK_SHIFT))
		dropEffect = DROPEFFECT_SCROLL|DROPEFFECT_LINK;
	// check for force copy
	else */if ((dwKeyState & MK_CONTROL) == MK_CONTROL)
		dropEffect = DROPEFFECT_SCROLL | DROPEFFECT_COPY;
	// check for force move
	else if ((dwKeyState & MK_ALT) == MK_ALT ||
		(dwKeyState & MK_SHIFT) == MK_SHIFT)
		dropEffect = DROPEFFECT_SCROLL | DROPEFFECT_MOVE;
	// default -- recommended action is move
	else
		dropEffect = DROPEFFECT_SCROLL | DROPEFFECT_MOVE;
	
	return dropEffect;
}

DROPEFFECT CBCGPPlannerDropTarget::OnDropEx(CWnd* /*pWnd*/, 
							COleDataObject* pDataObject, 
							DROPEFFECT dropEffect, 
							DROPEFFECT /*dropList*/, CPoint point) 
{
	ASSERT (m_pPlanner != NULL || m_pCalendar != NULL);

	if (!pDataObject->IsDataAvailable (CBCGPPlannerManagerCtrl::GetClipboardFormat ()))
	{
		return DROPEFFECT_NONE;
	}

	if (m_pPlanner != NULL)
	{
		ASSERT_VALID (m_pPlanner);
		return m_pPlanner->OnDrop(pDataObject, dropEffect, point) ?
				dropEffect : DROPEFFECT_NONE;
	}

	if (m_pCalendar != NULL)
	{
		ASSERT_VALID (m_pCalendar);
		return m_pCalendar->OnDrop(pDataObject, dropEffect, point) ?
				dropEffect : DROPEFFECT_NONE;
	}

	return DROPEFFECT_NONE;
}

#endif // BCGP_EXCLUDE_PLANNER
