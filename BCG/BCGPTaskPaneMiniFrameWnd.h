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

#if !defined(AFX_BCGPTASKPANEMINIFRAMEWND_H__07742A0D_50C0_474A_9E8F_8A3A4B047FD9__INCLUDED_)
#define AFX_BCGPTASKPANEMINIFRAMEWND_H__07742A0D_50C0_474A_9E8F_8A3A4B047FD9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BCGPTaskPaneMiniFrameWnd.h: header file
//

#include "BCGCBPro.h"

#ifndef BCGP_EXCLUDE_TASK_PANE

#include "BCGPMiniFrameWnd.h"

#define BCGP_CAPTION_BTN_LEFT		0x10
#define BCGP_CAPTION_BTN_RIGHT		0x20
#define BCGP_CAPTION_BTN_TPMENU		0x40

class BCGCBPRODLLEXPORT CBCGPTaskPaneMiniFrameWnd : public CBCGPMiniFrameWnd  
{
	DECLARE_SERIAL(CBCGPTaskPaneMiniFrameWnd)
public:
	CBCGPTaskPaneMiniFrameWnd();
	virtual ~CBCGPTaskPaneMiniFrameWnd();

// Operations
public:
	virtual void SetCaptionButtons (DWORD dwButtons);
	void SetTaskPaneCaptionButtons ();

	virtual void CalcBorderSize (CRect& rectBorderSize) const;

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPTaskPaneMiniFrameWnd)
	afx_msg void OnNcPaint();
	//}}AFX_MSG
	afx_msg BOOL OnNeedTipText(UINT id, NMHDR* pNMH, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()
	
protected:
	virtual void AddButton (UINT nHit);
	virtual void OnTrackCaptionButtons (CPoint point);
	virtual void StopCaptionButtonsTracking ();

	virtual void OnDrawBorder (CDC* pDC);
	virtual void OnDrawCaptionButtons (CDC* pDC);
	virtual void OnPressButtons (UINT nHit);
	
	BOOL	m_bMenuBtnPressed;
};

#endif BCGP_EXCLUDE_TASK_PANE

#endif // !defined(AFX_BCGPTASKPANEMINIFRAMEWND_H__07742A0D_50C0_474A_9E8F_8A3A4B047FD9__INCLUDED_)
