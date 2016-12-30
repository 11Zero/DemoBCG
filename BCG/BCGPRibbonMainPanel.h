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
// BCGPRibbonMainPanel.h: interface for the CBCGPRibbonMainPanel class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPRIBBONMAINPANEL_H__4FC2195A_9736_4B67_88CF_A5778C816CA0__INCLUDED_)
#define AFX_BCGPRIBBONMAINPANEL_H__4FC2195A_9736_4B67_88CF_A5778C816CA0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGPRibbonPanel.h"
#include "BCGPRibbonButton.h"
#include "BCGPRibbonButtonsGroup.h"

#ifndef BCGP_EXCLUDE_RIBBON

class CBCGPRibbonMainPanelButton;
class CBCGPRibbonSearchBox;

class BCGCBPRODLLEXPORT CBCGPRibbonRecentFilesList : public CBCGPRibbonButtonsGroup
{
	friend class CBCGPRibbonCollector;

	DECLARE_DYNCREATE(CBCGPRibbonRecentFilesList)

public:
	CBCGPRibbonRecentFilesList(LPCTSTR lpszLabel = NULL, BOOL bShowPins = FALSE)
	{
		SetText (lpszLabel == NULL ? _T("") : lpszLabel);
		m_bShowPins = bShowPins;
	}

	void FillList ();

protected:
	virtual void OnAfterChangeRect (CDC* pDC);
	virtual CSize GetRegularSize (CDC* pDC);
	virtual void OnDraw (CDC* pDC);
	virtual BOOL OnMenuKey (UINT nUpperChar);
	virtual void CopyFrom (const CBCGPBaseRibbonElement& src);

	BOOL m_bShowPins;
};

class BCGCBPRODLLEXPORT CBCGPRibbonMainPanel : public CBCGPRibbonPanel  
{
	friend class CBCGPRibbonMainButton;
	friend class CBCGPRibbonCollector;
	friend class CBCGPRibbonSearchBox;

	DECLARE_DYNCREATE (CBCGPRibbonMainPanel)

// Construction
public:
	CBCGPRibbonMainPanel();
	virtual ~CBCGPRibbonMainPanel();

// Operations
public:
	virtual void Add (CBCGPBaseRibbonElement* pElem);
	void AddToBottom (CBCGPRibbonMainPanelButton* pElem);

	void AddRecentFilesList (LPCTSTR lpszLabel, int nWidth = 300, BOOL bShowPins = FALSE);
	void AddToRight (CBCGPBaseRibbonElement* pElem, int nWidth = 300);

	void EnableCommandSearch (BOOL bEnable, LPCTSTR lpszLabel, LPCTSTR lpszKeyTip, int nWidth = 0, BOOL bHideDisabled = FALSE);

protected:
	virtual int GetMenuElements () const;
	void DrawMainButton (CDC* pDC, CWnd* pWnd);

// Overrides
protected:

	virtual void RecalcWidths (CDC* pDC, int nHeight);
	virtual void Repos (CDC* pDC, const CRect& rect);
	virtual void DoPaint (CDC* pDC);

	virtual BOOL IsFixedSize () const
	{
		return TRUE;
	}

	virtual void CopyFrom (CBCGPRibbonPanel& src);
	virtual BOOL GetPreferedMenuLocation (CRect& rect);

	virtual BOOL IsMainPanel () const
	{
		return TRUE;
	}

	virtual void OnDrawMenuBorder (CDC* pDC, CBCGPRibbonPanelMenuBar* pMenuBar);
	virtual CBCGPBaseRibbonElement* MouseButtonDown (CPoint point);

	void OnSearch (const CString& str);
	BOOL OnSearchNavigate (UINT nChar);

	BOOL IsBackstageRightPaneActive() const	{	return FALSE;	}

// Attributes:
public:
	CRect GetCommandsFrame () const;

protected:
	int						m_nBottomElementsNum;
	CRect					m_rectMenuElements;
	int						m_nTopMargin;
	CBCGPRibbonMainButton*	m_pMainButton;
	CBCGPBaseRibbonElement* m_pElemOnRight;
	CBCGPRibbonSearchBox*	m_pSearchBox;
	int						m_nSearchBoxWidth;
	BOOL					m_bHideDisabledInSearchResult;
	int						m_nRightPaneWidth;
	int						m_nSearchResults;
	BOOL					m_bSearchMode;
	BOOL					m_bReposAfterSearch;
	int						m_nTotalHeight;
	CRect					m_rectScrollCorner;
};

class BCGCBPRODLLEXPORT CBCGPRibbonMainPanelButton : public CBCGPRibbonButton
{
	DECLARE_DYNCREATE(CBCGPRibbonMainPanelButton)

// Construction
public:
	CBCGPRibbonMainPanelButton ();

	CBCGPRibbonMainPanelButton (
		UINT	nID, 
		LPCTSTR lpszText, 
		int		nSmallImageIndex);

	CBCGPRibbonMainPanelButton (
		UINT	nID, 
		LPCTSTR lpszText, 
		HICON	hIcon);

	virtual ~CBCGPRibbonMainPanelButton();

// Overrides
protected:
	virtual COLORREF OnFillBackground (CDC* pDC);
	virtual void OnDrawBorder (CDC* pDC);
};

#endif // BCGP_EXCLUDE_RIBBON

#endif // !defined(AFX_BCGPRIBBONMAINPANEL_H__4FC2195A_9736_4B67_88CF_A5778C816CA0__INCLUDED_)
