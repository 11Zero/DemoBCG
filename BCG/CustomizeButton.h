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

// CustomizeButton.h: interface for the CCustomizeButton class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CUSTOMIZEBUTTON_H__7DC72143_689E_11D3_95C6_00A0C9289F1B__INCLUDED_)
#define AFX_CUSTOMIZEBUTTON_H__7DC72143_689E_11D3_95C6_00A0C9289F1B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGPToolbarMenuButton.h"

class CBCGPToolBar;

class BCGCBPRODLLEXPORT CCustomizeButton : public  CBCGPToolbarMenuButton
{
	friend class CBCGPToolBar;

	DECLARE_SERIAL(CCustomizeButton)

public:
	CCustomizeButton();
	CCustomizeButton(UINT uiCustomizeCmdId, const CString& strCustomizeText);
	virtual ~CCustomizeButton();

	virtual void OnDraw (CDC* pDC, const CRect& rect, CBCGPToolBarImages* pImages,
						BOOL bHorz = TRUE, BOOL bCustomizeMode = FALSE,
						BOOL bHighlight = FALSE,
						BOOL bDrawBorder = TRUE,
						BOOL bGrayDisabledButtons = TRUE);
	virtual CBCGPPopupMenu* CreatePopupMenu ();

	virtual void CopyFrom (const CBCGPToolbarButton& src);
	virtual BOOL IsEmptyMenuAllowed () const
	{
		return TRUE;
	}

	virtual void OnCancelMode ();
	virtual void OnChangeParentWnd (CWnd* pWndParent);
	virtual SIZE OnCalculateSize (CDC* pDC, const CSize& sizeDefault, BOOL bHorz);

	virtual BOOL IsEditable () const
	{
		return FALSE;
	}

	virtual BOOL CanBeStored () const			{	return FALSE;	}

	UINT  GetCustomizeCmdId() const
	{
		return m_uiCustomizeCmdId;
	}

	CString	GetCustomizeText() const
	{
		return 	m_strCustomizeText;
	}

	const CObList& GetInvisibleButtons () const
	{
		return m_lstInvisibleButtons;
	}

	CBCGPToolBar* GetParentToolbar()
	{
		return m_pWndParentToolbar;
	}

	void SetDefaultDraw (BOOL bDefaultDraw = TRUE)
	{
		m_bDefaultDraw = bDefaultDraw;
	}

	BOOL IsDefaultDraw () const
	{
		return m_bDefaultDraw;
	}

	void SetExtraSize (int cx, int cy)
	{
		m_sizeExtra = CSize (cx, cy);
	}

	CSize GetExtraSize () const
	{
		return m_bIsPipeStyle ? m_sizeExtra : CSize (0, 0);
	}

	void SetPipeStyle (BOOL bOn = TRUE)
	{
		m_bIsPipeStyle = bOn;
	}

	BOOL IsPipeStyle () const
	{
		return m_bIsPipeStyle && !m_bOnRebar;
	}

	void AddInvisibleButton (CBCGPToolbarButton* pButton)
	{
		m_lstInvisibleButtons.AddTail (pButton);
	}

	virtual BOOL InvokeCommand (CBCGPPopupMenuBar* pMenuBar, 
		const CBCGPToolbarButton* pButton);

	void SetMenuRightAlign (BOOL bMenuRightAlign)
	{
		m_bMenuRightAlign = bMenuRightAlign;
	}

	BOOL IsMenuRightAlign () const
	{
		return m_bMenuRightAlign;
	}

public:
	static BOOL		m_bIgnoreLargeIconsMode;

protected:
	void CommonInit ();

	UINT			m_uiCustomizeCmdId;
	CString			m_strCustomizeText;
	CBCGPToolBar*	m_pWndParentToolbar;
	BOOL			m_bIsEmpty;
	BOOL			m_bDefaultDraw;
	CSize			m_sizeExtra;
	BOOL			m_bIsPipeStyle;		// Used in 2003 style only
	BOOL			m_bOnRebar;
	BOOL			m_bMenuRightAlign;

	CObList			m_lstInvisibleButtons;	// List of invisible butons on 
											// the parent toolbar.
};

#endif // !defined(AFX_CUSTOMIZEBUTTON_H__7DC72143_689E_11D3_95C6_00A0C9289F1B__INCLUDED_)
