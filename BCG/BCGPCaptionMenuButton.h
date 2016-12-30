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
// BCGPCaptionMenuButton.h: interface for the CBCGPCaptionMenuButton class.
//
///////////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPCAPTIONMENUBUTTON_H__2F9D4E4A_7724_4ED7_A8A9_8008D0C9252F__INCLUDED_)
#define AFX_BCGPCAPTIONMENUBUTTON_H__2F9D4E4A_7724_4ED7_A8A9_8008D0C9252F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"
#include "BCGPCaptionButton.h"
#include "MenuImages.h"
#include "BCGPVisualManager.h"

class BCGCBPRODLLEXPORT CBCGPCaptionMenuButton : public CBCGPCaptionButton
{
public:
	CBCGPCaptionMenuButton();
	CBCGPCaptionMenuButton(UINT nHit, BOOL bLeftAlign = FALSE);
	virtual ~CBCGPCaptionMenuButton();

// Attributes
public:
	int		m_nMenuResult;
	BOOL	m_bOSMenu;			// User standard Windows menu either than library
	BOOL	m_bMenuIsActive;

// Operations
public:
	void ShowMenu (HMENU hMenu, CWnd* pWnd);
	
	virtual void OnDraw (CDC* pDC, BOOL bActive, 
		BOOL bHorz = TRUE, BOOL bMaximized = TRUE, BOOL bDisabled = FALSE);
	

};

#endif // !defined(AFX_BCGPCAPTIONMENUBUTTON_H__2F9D4E4A_7724_4ED7_A8A9_8008D0C9252F__INCLUDED_)
