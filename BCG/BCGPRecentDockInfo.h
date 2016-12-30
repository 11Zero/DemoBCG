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
// BCGPRecentDockInfo.h: interface for the CBCGPRecentDockInfo class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPRECENTDOCKINFO_H__1FB34813_798E_48C1_AAA2_5C7F9DF0B751__INCLUDED_)
#define AFX_BCGPRECENTDOCKINFO_H__1FB34813_798E_48C1_AAA2_5C7F9DF0B751__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CBCGPBarContainer;
class CBCGPDockBar;
class CBCGPDockBarRow;
class CBCGPSlider;
class CBCGPMiniFrameWnd;
class CBCGPControlBar;
class CBCGPDockingControlBar;

class CBCGPRecentContainerInfo : public CObject  
{
public:
	CBCGPRecentContainerInfo ();
	virtual ~CBCGPRecentContainerInfo ();

	CBCGPRecentContainerInfo& operator= (CBCGPRecentContainerInfo& src);

	void Init ();

	virtual void StoreDockInfo (CBCGPBarContainer* pRecentContainer, 
								CBCGPDockingControlBar* pBar, 
								CBCGPDockingControlBar* pTabbedBar = NULL);

	virtual void SetInfo (CBCGPRecentContainerInfo& srcInfo);
	

public:
	CBCGPBarContainer*		m_pRecentBarContainer;	// recent container of CB
	
	CRect					m_rectDockedRect;		// in client coordinates
	int						m_nRecentPercent;		// what percent CB took in container
	BOOL					m_bIsRecentLeftBar;		// is CB was left in its container
	CList<HWND,HWND>		m_lstRecentListOfBars; // need to remember a list of siblings bar
												   // in container manager to place the CB
												   // back when docked again
	CBCGPBarContainer*		m_pRecentContanerOfTabWnd;
};
//-------------------------------------------------//
class CBCGPRecentDockInfo : public CObject  
{
public:
	CBCGPRecentDockInfo(CBCGPControlBar* pBar);
	virtual ~CBCGPRecentDockInfo();

	CBCGPRecentDockInfo& operator= (CBCGPRecentDockInfo& src);

	void Init ();
	void CleanUp ();

	virtual void StoreDockInfo (CBCGPBarContainer* pRecentContainer, 
									CBCGPDockingControlBar* pTabbedBar = NULL);
	virtual void SetInfo (BOOL bForSlider, CBCGPRecentDockInfo& srcInfo);

	CBCGPBarContainer* GetRecentContainer (BOOL bForSlider);
	CBCGPBarContainer* GetRecentTabContainer (BOOL bForSlider);
	CRect& GetRecentDockedRect (BOOL bForSlider);
	int GetRecentDockedPercent (BOOL bForSlider);
	BOOL IsRecentLeftBar (BOOL bForSlider);
	void SaveListOfRecentBars (CList<HWND,HWND>& lstOrg, BOOL bForSlider);
	CList<HWND, HWND>& GetRecentListOfBars (BOOL bForSlider);
	CBCGPSlider* GetRecentDefaultSlider ();

public:
	CRect		m_rectRecentFloatingRect; // in screen coordinates
	DWORD		m_dwRecentAlignmentToFrame; // to what frame side control bar was docked

	int			m_nRecentRowIndex;  // if CB was resided on a dock bar 
									// what row it occupied (zero-based)
									// it will be used instead of m_pRecentDockBarRow
									// if this pointer is invalid (the row was deleted)

	CBCGPDockBar*		m_pRecentDockBar;    // what dock bar CB was docked on
	CBCGPDockBarRow*	m_pRecentDockBarRow; // a pointer to dock bar row

	int					m_nRecentTabNumber; // recent position on tab window


	HWND		m_hRecentDefaultSlider; // recent default slider of CB
	HWND		m_hRecentMiniFrame;

	CBCGPRecentContainerInfo m_recentSliderInfo;
	CBCGPRecentContainerInfo m_recentMiniFrameInfo;

protected:
	CBCGPControlBar*	m_pBar;
};

#endif // !defined(AFX_BCGPRECENTDOCKINFO_H__1FB34813_798E_48C1_AAA2_5C7F9DF0B751__INCLUDED_)
