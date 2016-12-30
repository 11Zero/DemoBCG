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
// BCGPRibbonQuickAccessToolbar.h: interface for the CBCGPRibbonQuickAccessToolbar class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPRIBBONQUICKACCESSTOOLBAR_H__07D110EE_3743_4DE1_A7BC_53DA14E7089B__INCLUDED_)
#define AFX_BCGPRIBBONQUICKACCESSTOOLBAR_H__07D110EE_3743_4DE1_A7BC_53DA14E7089B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"

#ifndef BCGP_EXCLUDE_RIBBON

#include "BCGPRibbonButtonsGroup.h"
#include "BCGPRibbonButton.h"

////////////////////////////////////////////////////////
// CBCGPRibbonQuickAccessToolbar class

class CBCGPRibbonQuickAccessCustomizeButton;

class BCGCBPRODLLEXPORT CBCGPRibbonQATDefaultState
{
	friend class CBCGPRibbonQuickAccessToolbar;
	friend class CBCGPRibbonBar;
	friend class CBCGPRibbonCollector;

public:
	CBCGPRibbonQATDefaultState();

	void AddCommand (UINT uiCmd, BOOL bIsVisible = TRUE);
	void AddSeparator (BOOL bIsVisible = TRUE);
	void RemoveAll ();

	void CopyFrom (const CBCGPRibbonQATDefaultState& src);

protected:
	CArray<UINT,UINT>	m_arCommands;
	CArray<BOOL,BOOL>	m_arVisibleState;
};

class BCGCBPRODLLEXPORT CBCGPRibbonQuickAccessToolbar : public CBCGPRibbonButtonsGroup
{
	DECLARE_DYNCREATE(CBCGPRibbonQuickAccessToolbar)

	friend class CBCGPRibbonBar;
	friend class CBCGPRibbonQuickAccessCustomizeButton;
	friend class CBCGPBaseRibbonElement;
	friend class CBCGPRibbonCustomizeQATPage;
	friend class CBCGPRibbonCollector;

public:
	CBCGPRibbonQuickAccessToolbar();
	virtual ~CBCGPRibbonQuickAccessToolbar();

protected:
	void SetCommands (	CBCGPRibbonBar* pRibbonBar,
						const CList<UINT,UINT>& lstCommands,
						LPCTSTR lpszToolTip);

	void SetCommands (	CBCGPRibbonBar* pRibbonBar,
						const CList<UINT,UINT>& lstCommands,
						CBCGPRibbonQuickAccessCustomizeButton* pCustButton);

	void GetCommands (CList<UINT,UINT>& lstCommands);

	void GetDefaultCommands (CList<UINT,UINT>& lstCommands);

	void ReplaceCommands (const CList<UINT,UINT>& lstCommands);
	void ResetCommands ();

	int GetActualWidth () const;

	virtual CSize GetRegularSize (CDC* pDC);
	virtual void OnAfterChangeRect (CDC* pDC);

	virtual BOOL IsQAT () const
	{
		return TRUE;
	}

	// Accessibility:
	virtual HRESULT get_accParent(IDispatch **ppdispParent);
	virtual HRESULT get_accChildCount(long *pcountChildren);
	virtual HRESULT accLocation(long *pxLeft, long *pyTop, long *pcxWidth, long *pcyHeight, VARIANT varChild);
	virtual HRESULT accNavigate(long navDir, VARIANT varStart, VARIANT *pvarEndUpAt);
	virtual HRESULT accHitTest(long xLeft, long yTop, VARIANT *pvarChild);
	virtual HRESULT accDoDefaultAction(VARIANT varChild);
	virtual BOOL OnSetAccData(long lVal);
	virtual BOOL SetACCData(CWnd* pParent, CBCGPAccessibilityData& data);

	void Add (CBCGPBaseRibbonElement* pElem);
	void Remove (CBCGPBaseRibbonElement* pElem);

	void RebuildHiddenItems ();

	CRect GetCommandsRect () const
	{
		return m_rectCommands;
	}

	void RebuildKeys ();

protected:
	CBCGPRibbonQATDefaultState	m_DefaultState;
	CRect						m_rectCommands;
};

#endif // BCGP_EXCLUDE_RIBBON

#endif // !defined(AFX_BCGPRIBBONQUICKACCESSTOOLBAR_H__07D110EE_3743_4DE1_A7BC_53DA14E7089B__INCLUDED_)
