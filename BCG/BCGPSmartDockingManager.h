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
// BCGPSmartDockingManager.h: interface for the CBCGPSmartDockingManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPSMARTDOCKINGMANAGER_H__49C67626_ECA8_4BCB_9466_CA2B01DD6345__INCLUDED_)
#define AFX_BCGPSMARTDOCKINGMANAGER_H__49C67626_ECA8_4BCB_9466_CA2B01DD6345__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGPSmartDockingMarker.h"
#include "BCGPSDPlaceMarkerWnd.h"

class CBCGPSmartDockingManager : public CObject  
{
public:
	CBCGPSmartDockingManager();
	virtual ~CBCGPSmartDockingManager();

	// create/destroy the manager
	void Create (CWnd* pwndOwner,
		CRuntimeClass* prtMarker = NULL,
		CRuntimeClass* prtCentralGroup = NULL);

	void Destroy ();

	// start/stop smart-docking action
	void Start (CWnd* pDockingWnd);
	void Stop ();

    // show/hide smart-docking markers (typically while Ctrl-key is held)
	void Show (BOOL bShow = TRUE);

    // get currently highlighted marker number
    CBCGPSmartDockingMarker::SDMarkerPlace GetHilitedMarkerNo () const
    {
        return m_nHiliteSideNo;
    }

    BOOL IsStarted ()
    {
        return m_bCreated && m_bStarted;
    }

	// react to mouse move
	void OnMouseMove (CPoint point);

	// react to mainframe's position change
	void OnPosChange ();

    // set owner's outer docking rectangle (in screen coords)
    void SetOuterRect (CRect rcOuter);

    // move central group to new position
    // middle marker: nMiddleIsOn == -1 no change, 0 off, 1 on
    void MoveCentralGroup (CRect rect, int nMiddleIsOn = -1, DWORD dwEnabledAlignment = CBRS_ALIGN_ANY);
    void ShowCentralGroup (BOOL bShow = TRUE, DWORD dwEnabledAlignment = CBRS_ALIGN_ANY);

    // marker place
    void ShowPlaceAt (CRect rect);
    void HidePlace ();
    void ShowTabbedPlaceAt (CRect rect, int nTabXOffset,
                            int nTabWidth, int nTabHeight);
	CWnd* GetOwner () const
    {
        return m_pwndOwner;
    };

	void CauseCancelMode ();

protected:
	// state flags
	BOOL        m_bStarted;
	BOOL        m_bCreated;
    BOOL        m_bShown;
    BOOL        m_bCentralGroupShown;

    // currently highlighted marker
    CBCGPSmartDockingMarker::SDMarkerPlace    m_nHiliteSideNo;

	// mainframe
	CWnd*       m_pwndOwner;

	// miniframe being docked
	CWnd*		m_pDockingWnd;
	// enabled alignment of embedded docking bar
	DWORD		m_dwEnabledAlignment;

    // outer rectangle
    CRect		m_rcOuter;

    // place marker
    CBCGPSDPlaceMarkerWnd		m_wndPlaceMarker;

	CBCGPSmartDockingMarker*	m_arMarkers [9];
	CBCGPSDCentralGroup*		m_pCentralGroup;
};

#endif // !defined(AFX_BCGPSMARTDOCKINGMANAGER_H__49C67626_ECA8_4BCB_9466_CA2B01DD6345__INCLUDED_)
