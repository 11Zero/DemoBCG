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
// BCGPTooltipManager.h: interface for the CBCGPTooltipManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPTOOLTIPMANAGER_H__7070822E_9DAB_47DA_80BF_F76338D007E3__INCLUDED_)
#define AFX_BCGPTOOLTIPMANAGER_H__7070822E_9DAB_47DA_80BF_F76338D007E3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXTEMPL_H__
	#include "afxtempl.h"
#endif

#include "BCGCBPro.h"
#include "BCGPToolTipCtrl.h"

#define BCGP_TOOLTIP_TYPE_ALL			0xFFFF

#define BCGP_TOOLTIP_TYPE_DEFAULT		0x0001
#define BCGP_TOOLTIP_TYPE_TOOLBAR		0x0002
#define BCGP_TOOLTIP_TYPE_TAB			0x0004
#define BCGP_TOOLTIP_TYPE_MINIFRAME		0x0008
#define BCGP_TOOLTIP_TYPE_DOCKBAR		0x0010
#define BCGP_TOOLTIP_TYPE_EDIT			0x0020
#define	BCGP_TOOLTIP_TYPE_BUTTON		0x0040
#define	BCGP_TOOLTIP_TYPE_TOOLBOX		0x0080
#define BCGP_TOOLTIP_TYPE_PLANNER		0x0100
#define BCGP_TOOLTIP_TYPE_RIBBON		0x0200
#define	BCGP_TOOLTIP_TYPE_CAPTIONBAR	0x0400
#define	BCGP_TOOLTIP_TYPE_STATUSBAR		0x0800
#define BCGP_TOOLTIP_TYPE_GANTT		    0x1000
#define BCGP_TOOLTIP_TYPE_GRID		    0x2000

#define BCGP_TOOLTIP_TYPES				14

class BCGCBPRODLLEXPORT CBCGPTooltipManager : public CObject  
{
public:

	static BOOL CreateToolTip (
		CToolTipCtrl*& pToolTip, CWnd* pWndParent, UINT nType);

	static void DeleteToolTip (CToolTipCtrl*& pToolTip);

	static void SetTooltipText (TOOLINFO* pTI,
				CToolTipCtrl* pToolTip, UINT nType, const CString strText,
				LPCTSTR lpszDescr = NULL);

	CBCGPTooltipManager();
	virtual ~CBCGPTooltipManager();

	void SetTooltipParams (	UINT nTypes,
							CRuntimeClass* pRTC = RUNTIME_CLASS (CBCGPToolTipCtrl),
							CBCGPToolTipParams* pParams = NULL);

	void UpdateTooltips ();

protected:
	BOOL CreateToolTipObject (CToolTipCtrl*& pToolTip, UINT nType);

	CBCGPToolTipParams	m_Params [BCGP_TOOLTIP_TYPES];
	CRuntimeClass*		m_pRTC [BCGP_TOOLTIP_TYPES];
	CList<HWND, HWND>	m_lstOwners;
};

extern BCGCBPRODLLEXPORT CBCGPTooltipManager*	g_pTooltipManager;
BCGCBPRODLLEXPORT extern UINT BCGM_UPDATETOOLTIPS;

#endif // !defined(AFX_BCGPTOOLTIPMANAGER_H__7070822E_9DAB_47DA_80BF_F76338D007E3__INCLUDED_)
