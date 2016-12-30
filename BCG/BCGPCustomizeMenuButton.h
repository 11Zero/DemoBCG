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
// BCGPCustomizeMenuButton.h: interface for the CBCGPCustomizeMenuButton class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPCUSTOMIZEMENUBUTTON_H__F9AC7C5A_0566_45CB_8C66_8E53E787412D__INCLUDED_)
#define AFX_BCGPCUSTOMIZEMENUBUTTON_H__F9AC7C5A_0566_45CB_8C66_8E53E787412D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"
#include "BCGPToolbarMenuButton.h"

#define BCGPCUSTOMIZE_INTERNAL_ID ((UINT)-20)

class BCGCBPRODLLEXPORT  CBCGPCustomizeMenuButton : public CBCGPToolbarMenuButton  
{
	friend class CBCGPPopupMenuBar;

	DECLARE_DYNCREATE(CBCGPCustomizeMenuButton)

public:
	CBCGPCustomizeMenuButton(UINT uiID,HMENU hMenu,int iImage,LPCTSTR lpszText=NULL,BOOL bUserButton=FALSE);
	CBCGPCustomizeMenuButton();
	virtual ~CBCGPCustomizeMenuButton();

	static BOOL SetParentToolbar(CBCGPToolBar* pToolBar)
	{
		m_pWndToolBar = pToolBar;

		return TRUE;
	}

	static CBCGPToolBar* GetParentToolbar()
	{
		return m_pWndToolBar;
	}

	void SetItemIndex(UINT uiIndex, BOOL bExist = TRUE, BOOL bAddSp = FALSE);


//Overridables
protected:
	virtual void OnDraw (CDC* pDC, const CRect& rect, CBCGPToolBarImages* pImages,
						BOOL bHorz = TRUE, BOOL bCustomizeMode = FALSE,
						BOOL bHighlight = FALSE,
						BOOL bDrawBorder = TRUE,
						BOOL bGrayDisabledButtons = TRUE);

	virtual void CopyFrom (const CBCGPToolbarButton& src);
	virtual SIZE OnCalculateSize (CDC* pDC, const CSize& sizeDefault, BOOL bHorz);
	virtual BOOL OnClickMenuItem();
	virtual void DrawCheckBox(CDC* pDC, const CRect& rect, BOOL bHighlight);

public:

	void SetSeparator()
	{
		bSeparator = TRUE;
		SetStyle(TBBS_DISABLED);
	}

	void RestoreRecentlyUsedState()
	{
		CBCGPMenuBar::SetRecentlyUsedMenus(m_bRecentlyUsedOld);	
	}

	//Save Resently Used State
	static BOOL m_bRecentlyUsedOld;

	void EnableCustomization(BOOL bEnable = TRUE)
	{
		m_bIsEnabled = bEnable;
	}

	void SetBrothersButton()
	{
		m_bBrothersBtn = TRUE;
	}

	static CMap<UINT, UINT, int, int>	m_mapPresentIDs;

	static BOOL IsCommandExist(UINT uiCmdId);

protected:
	static CBCGPToolBar* m_pWndToolBar;  
	UINT m_uiIndex;
	BOOL m_bShow;
	BOOL bSeparator;
	BOOL m_bExist;
	BOOL m_bAddSpr;
	BOOL m_bIsEnabled;
	BOOL m_bBrothersBtn;

protected:
		CString SearchCommandText(CMenu* pMenu, UINT in_uiCmd);
		void UpdateCustomizeButton();
};

#endif // !defined(AFX_BCGPCUSTOMIZEMENUBUTTON_H__F9AC7C5A_0566_45CB_8C66_8E53E787412D__INCLUDED_)
