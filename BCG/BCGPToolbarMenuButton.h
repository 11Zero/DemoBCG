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

// BCGPToolbarMenuButton.h: interface for the CBCGToolbarMenuButton class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPTOOLBARMENUBUTTON_H__B715CF84_CA00_11D1_A647_00A0C93A70EC__INCLUDED_)
#define AFX_BCGPTOOLBARMENUBUTTON_H__B715CF84_CA00_11D1_A647_00A0C93A70EC__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "BCGCBPro.h"
#include "BCGPToolbarButton.h"
#include "BCGPPopupMenu.h"

static const int TEXT_MARGIN = 3;
static const int MENU_IMAGE_MARGIN = 2;

static const int nBCGPMenuGroupID = ((UINT)-1) - 1;

class BCGCBPRODLLEXPORT CBCGPToolbarMenuButton : public CBCGPToolbarButton  
{
	friend class CBCGPPopupMenu;
	friend class CBCGPPopupMenuBar;
	friend class CBCGPToolBar;
	friend class CBCGPMenuBar;
	friend class CBCGPRibbonStatusBarCustomizeButton;
	friend class CBCGPRibbonButton;

	DECLARE_SERIAL(CBCGPToolbarMenuButton)

public:
	CBCGPToolbarMenuButton();
	CBCGPToolbarMenuButton(const CBCGPToolbarMenuButton& src);
	CBCGPToolbarMenuButton (UINT uiID, HMENU hMenu, int iImage, LPCTSTR lpszText = NULL,
								BOOL bUserButton = FALSE);
	virtual ~CBCGPToolbarMenuButton();

// Operations:
protected:
	void Initialize ();
	void Initialize (UINT uiID, HMENU hMenu, int iImage, LPCTSTR lpszText = NULL,
								BOOL bUserButton = FALSE);
	void DrawMenuItem (CDC* pDC, const CRect& rect, CBCGPToolBarImages* pImages, 
					BOOL bCustomizeMode, BOOL bHighlight, BOOL bGrayDisabledButtons,
					BOOL bContentOnly = FALSE);
	void DrawDocumentIcon (CDC* pDC, const CRect& rectImage, HICON hIcon);

	static void GetTextHorzOffsets (int& xOffsetLeft, int& xOffsetRight);

public:
	virtual BOOL OpenPopupMenu (CWnd* pWnd = NULL);
	virtual void CopyFrom (const CBCGPToolbarButton& src);
	virtual void Serialize (CArchive& ar);
	virtual void OnDraw (CDC* pDC, const CRect& rect, CBCGPToolBarImages* pImages,
						BOOL bHorz = TRUE, BOOL bCustomizeMode = FALSE,
						BOOL bHighlight = FALSE,
						BOOL bDrawBorder = TRUE,
						BOOL bGrayDisabledButtons = TRUE);

	virtual SIZE OnCalculateSize (CDC* pDC, const CSize& sizeDefault, BOOL bHorz);
	virtual BOOL OnClick (CWnd* pWnd, BOOL bDelay = TRUE);
	virtual void OnChangeParentWnd (CWnd* pWndParent);
	virtual void CreateFromMenu (HMENU hMenu);
	virtual HMENU CreateMenu () const;
	virtual BOOL HaveHotBorder () const			{	return FALSE;	}
	virtual void OnCancelMode ();
	virtual BOOL OnContextHelp (CWnd* pWnd)
	{
		return OnClick (pWnd, FALSE);
	}

	virtual int OnDrawOnCustomizeList (CDC* pDC, const CRect& rect, BOOL bSelected);

	virtual BOOL IsDroppedDown () const
	{
		return m_pPopupMenu != NULL;
	}

	virtual CBCGPPopupMenu* CreatePopupMenu ()
	{
		return new CBCGPPopupMenu;
	}

	virtual void OnAfterCreatePopupMenu ()	{}

	virtual BOOL IsEmptyMenuAllowed () const
	{
		return FALSE;
	}

	virtual BOOL OnBeforeDrag () const;
	virtual void SaveBarState ();

	void GetImageRect (CRect& rectImage);
	
	virtual void SetRadio ();
	virtual void ResetImageToDefault ();
	virtual BOOL CompareWith (const CBCGPToolbarButton& other) const;

	virtual BOOL IsBorder () const
	{
		return TRUE;
	}

	virtual BOOL OnClickMenuItem ()
	{
		return FALSE;	// Return TRUE for the custom process
	}

	virtual BOOL IsExclusive () const
	{
		return FALSE;
	}

	virtual BOOL HasButton () const
	{
		return FALSE;
	}

	virtual BOOL SetACCData (CWnd* pParent, CBCGPAccessibilityData& data);

// Attributes:
public:
	const CObList& GetCommands () const
	{
		return m_listCommands;
	}

	CBCGPPopupMenu* GetPopupMenu () const
	{
		return m_pPopupMenu;
	}

	void SetMenuOnly (BOOL bMenuOnly)
	{ 
		m_bMenuOnly	= bMenuOnly;
	}

	virtual BOOL IsTearOffMenu () const
	{
		return m_uiTearOffBarID != 0;
	}

	virtual void SetTearOff (UINT uiBarID);

	static BOOL		m_bAlwaysCallOwnerDraw;
						// CMainFrame::OnDrawMenuImage will be called 
						// even after default image was drawn

	void SetMessageWnd (CWnd* pWndMessage)
	{
		m_pWndMessage = pWndMessage;
	}

	void SetMenuPaletteMode (BOOL bMenuPaletteMode = TRUE, int nPaletteRows = 1);
	BOOL IsMenuPaletteMode () const
	{
		return m_bMenuPaletteMode;
	}

	int GetPaletteRows () const
	{
		return m_nPaletteRows;
	}

	void EnableQuickCustomize()
	{
		m_bQuickCustomMode = TRUE;
	}

	BOOL IsQuickMode()
	{
		return m_bQuickCustomMode;
	}

	BOOL IsClickedOnMenu () const
	{
		return m_bClickedOnMenu;
	}

protected:
	CObList			m_listCommands;	// List of CBCGPToolbarButton
	BOOL			m_bDrawDownArrow;
	BOOL			m_bMenuMode;

	CBCGPPopupMenu*	m_pPopupMenu;
	BOOL			m_bDefault;

	BOOL			m_bClickedOnMenu;
	BOOL			m_bHorz;
	BOOL			m_bToBeClosed;

	BOOL			m_bMenuOnly;
	UINT			m_uiTearOffBarID;

	BOOL			m_bIsRadio;
	CWnd*			m_pWndMessage;

	BOOL			m_bMenuPaletteMode;
	int				m_nPaletteRows;
	BOOL            m_bQuickCustomMode;
	BOOL			m_bShowAtRightSide;

	CRect			m_rectArrow;
	CRect			m_rectButton;

public:

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
};

#endif // !defined(AFX_BCGPTOOLBARMENUBUTTON_H__B715CF84_CA00_11D1_A647_00A0C93A70EC__INCLUDED_)
