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

// BCGPToolbarButton.h: interface for the CBCGPToolbarButton class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPTOOLBARBUTTON_H__00F1EBC3_61BF_11D5_A304_F156F535EA11__INCLUDED_)
#define AFX_BCGPTOOLBARBUTTON_H__00F1EBC3_61BF_11D5_A304_F156F535EA11__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXOLE_H__
	#include <afxole.h>
#endif

#ifndef __AFXTEMPL_H__
	#include "afxtempl.h"
#endif

#include "BCGCBPro.h"

#define TBBS_BREAK	0x20000000

class CBCGPToolBar;
class CBCGPToolBarImages;
class CBCGPToolbarMenuButton;
struct CBCGPAccessibilityData;

class BCGCBPRODLLEXPORT CBCGPToolbarButton : public CObject  
{
	friend class CButtonsList;

	DECLARE_SERIAL(CBCGPToolbarButton)

public:
	CBCGPToolbarButton ();
	CBCGPToolbarButton (UINT uiID, int iImage, LPCTSTR lpszText = NULL, 
						BOOL bUserButton = FALSE, BOOL bLocked = FALSE);
	virtual ~CBCGPToolbarButton ();
// Operations:
public:

	//--------------------
	// Drag and drop stuf:
	//--------------------
	static CLIPFORMAT GetClipboardFormat ();
	static CBCGPToolbarButton* CreateFromOleData (COleDataObject* pDataObject);
	virtual BOOL PrepareDrag (COleDataSource& srcItem);

	//-----------------------------------------------------
	// Optional: if you want, that user may drag buttons 
	// between different applications, set your own format:
	//-----------------------------------------------------
	static void SetClipboardFormatName (LPCTSTR lpszName);

	virtual BOOL CanBeDropped (CBCGPToolBar* /*pToolbar*/)	{	return TRUE; }

	//-----------------------------------------------------------------
	//	Protected commands support. 
	//	Protected buttons will disabled in customization mode, so user 
	//	will be unable to drag/drop/change them.
	//-----------------------------------------------------------------
	static void SetProtectedCommands (const CList<UINT, UINT>& lstCmds);
	static const CList<UINT, UINT>& GetProtectedCommands ()	{	return m_lstProtectedCommands;	}

// Overrides:
	virtual void CopyFrom (const CBCGPToolbarButton& src);
	virtual void Serialize (CArchive& ar);
	virtual void OnDraw (CDC* pDC, const CRect& rect, CBCGPToolBarImages* pImages,
						BOOL bHorz = TRUE, BOOL bCustomizeMode = FALSE,
						BOOL bHighlight = FALSE,
						BOOL bDrawBorder = TRUE,
						BOOL bGrayDisabledButtons = TRUE);
	virtual SIZE OnCalculateSize (CDC* pDC, const CSize& sizeDefault, BOOL bHorz);
	virtual BOOL OnClick (CWnd* pWnd, BOOL bDelay = TRUE)		
	{	
		pWnd;
		bDelay;
		return FALSE;
	}
	
	virtual BOOL OnClickUp() {return FALSE;}
	virtual void OnClickUpOutside() {}
	virtual void OnChangeParentWnd (CWnd* pWndParent);
	virtual BOOL ExportToMenuButton (CBCGPToolbarMenuButton& menuButton) const;
	virtual void OnMove ()								{}
	virtual void OnSize (int /*iSize*/)					{}
	virtual HWND GetHwnd ()								{	return NULL;	}
	virtual BOOL CanBeStretched () const				{	return FALSE;	}
	virtual BOOL NotifyCommand (int /*iNotifyCode*/)	{	return FALSE;	}
	virtual void OnAddToCustomizePage ()		{}
	virtual HBRUSH OnCtlColor(CDC* /*pDC*/, UINT /*nCtlColor*/)	{	return NULL;	}
	virtual void OnDblClick (CWnd* /*pWnd*/)			{}
	virtual BOOL CanBeStored () const					{	return TRUE;	}
	virtual BOOL HaveHotBorder () const					{	return TRUE;	}
	virtual void OnCancelMode ()						{}
	virtual void OnGlobalFontsChanged() {}
	virtual BOOL IsEditable () const
	{
		return !IsBCGPStandardCommand (m_nID) && 
				(m_lstProtectedCommands.Find (m_nID) == NULL);
	}
	
	virtual BOOL OnContextHelp (CWnd* /* pWnd */)		{	return FALSE;	}
	virtual BOOL OnCustomizeMenu (CMenu* /*pMenu*/)		{	return FALSE;	}

	virtual int OnDrawOnCustomizeList (CDC* pDC, const CRect& rect, 
										BOOL bSelected);

	virtual BOOL IsDroppedDown () const
	{
		return FALSE;
	}

	virtual BOOL OnBeforeDrag () const
	{
		return TRUE;	// Drag is possible
	}

	virtual BOOL OnBeforeDrop (CBCGPToolBar* /*pTarget*/)
	{
		return TRUE;	// Drop is possible
	}

	virtual BOOL OnToolHitTest(const CWnd* pWnd, TOOLINFO* pTI);
	virtual void SaveBarState () {}

	virtual void OnShow (BOOL /*bShow*/)	{}

	virtual const CRect GetInvalidateRect () const
	{
		return m_rect;
	}

	virtual void SetStyle (UINT nStyle)
	{
		m_nStyle = nStyle;
	}

	virtual void ResetImageToDefault ();
	virtual BOOL CompareWith (const CBCGPToolbarButton& other) const;

	virtual void EnableWindow (BOOL bEnable = TRUE)
	{
		if (GetHwnd () != NULL)
		{
			::EnableWindow (GetHwnd (), bEnable);
		}
	}

	virtual BOOL IsWindowVisible ()
	{
		CWnd* pWnd = GetHwnd () != NULL ? CWnd::FromHandle (GetHwnd ()) : NULL;
		return (pWnd != NULL && (pWnd->GetStyle () & WS_VISIBLE));
	}

	virtual BOOL IsOwnerOf (HWND hwnd)
	{
		return	GetHwnd () != NULL &&
				(GetHwnd () == hwnd || ::IsChild (GetHwnd (), hwnd));
	}

	virtual BOOL HasFocus () const
	{
		HWND hwndBtn = ((CBCGPToolbarButton*)this)->GetHwnd ();

		return hwndBtn != NULL && 
			(hwndBtn == ::GetFocus () || ::IsChild (hwndBtn, ::GetFocus ()));
	}

	virtual BOOL OnGetCustomToolTipText (CString& /*strToolTip*/)
	{
		return FALSE;
	}

	virtual BOOL OnUpdateToolTip (CWnd* /*pWndParent*/, int /*iButtonIndex*/, 
		CToolTipCtrl& /*wndToolTip*/, CString& /*str*/)
	{
		return FALSE;
	}

	virtual BOOL IsFirstInGroup () const;
	virtual BOOL IsLastInGroup () const;

	virtual BOOL IsAlwaysOpaque () const
	{
		return FALSE;
	}

	BOOL IsRibbonImage() const
	{
		return m_bRibbonImage;
	}

	virtual BOOL GetKeyboardAccelerator(CString& strAccel) const;

	//Accessibility support
	virtual BOOL SetACCData (CWnd* pParent, CBCGPAccessibilityData& data);
	virtual int GetAccCount();

	virtual void DrawButtonText (CDC* pDC, const CString& strText, CRect rectText, UINT uiDTFlags,
							COLORREF clrText, int state);
	virtual CString GetDisplayText() const;

protected:
	void Initialize ();
	void FillInterior (CDC* pDC, const CRect& rect, BOOL bHighlight, BOOL bMenuImage = FALSE);

// Attributes:
public:
	BOOL IsDrawText () const
	{
		return m_bText && !m_strText.IsEmpty ();
	}

	BOOL IsDrawImage () const
	{
		return m_bImage && GetImage () >= 0;
	}

	int GetImage () const
	{
		return m_bUserButton ? m_iUserImage : m_iImage;
	}

	virtual void SetImage (int iImage);
	virtual void SetRadio () {}

	BOOL IsLocked () const
	{
		return m_bLocked;
	}

	void SetRect (const CRect rect)
	{
		if (m_rect != rect)
		{
			m_rect = rect;
			OnMove ();
		}
	}

	const CRect& Rect () const
	{
		return m_rect;
	}

	void Show (BOOL bShow)
	{
		if (m_bIsHidden != !bShow)
		{
			m_bIsHidden = !bShow;
			OnShow (bShow);
		}
	}

	BOOL IsHidden () const
	{
		return m_bIsHidden;
	}

	BOOL IsVisible () const
	{
		return m_bVisible;
	}

	void SetVisible (BOOL bShow = TRUE)
	{
		m_bVisible = bShow;
	}

	virtual BOOL IsExtraSize () const
	{
		return m_bExtraSize;
	}

	SIZE GetTextSize () const
	{
		// Actual only if m_bTextBelow is TRUE
		return m_sizeText;
	}

	BOOL IsHorizontal () const
	{
		return m_bHorz;
	}

	CWnd* GetParentWnd () const
	{
		return m_pWndParent;
	}

	BOOL IsOnGlass () const
	{
		return m_bOnGlass;
	}

	virtual void SetOnGlass (BOOL bOnGlass);

	BOOL	m_bUserButton;	// Is user-defined tool button?
	UINT	m_nID;			// Command ID that this button sends
	UINT	m_nStyle;		// TBBS_ styles
	CString	m_strText;		// Button text (for user-defined buttons only!)
	BOOL	m_bText;		// Draw text label
	BOOL	m_bImage;		// Draw image
	BOOL	m_bWrap;		// Wrap toolbar
	BOOL	m_bWholeText;	// Is whole text printed?
	BOOL	m_bTextBelow;	// Is text below image?
	CString	m_strTextCustom;// Text appear on the customization list

	// Run-time properties:
	BOOL	m_bDragFromCollection;	// Button is dragged from collection
	static CLIPFORMAT	m_cFormat;
	static CString		m_strClipboardFormatName;

	static BOOL	m_bWrapText;	// Is toolbar text may be multi-lined?
	static BOOL	m_bUpdateImages;

	DWORD	m_dwdItemData;	// User-defined data

protected:
	CRect	m_rect;			// Button location
	int		m_iImage;		// index into bitmap of this button's picture
	int		m_iUserImage;	// index into user's bitmap of this button's picture

	BOOL	m_bLocked;		// Is buttons placed on the "locked" toolbar
	BOOL	m_bIsHidden;	// Button rectangle is out of bar

	CSize	m_sizeText;		// Actual only if m_bTextBelow is TRUE

	static CList<UINT, UINT>	m_lstProtectedCommands;
									// Buttons are disabled in customization mode
	BOOL	m_bDisableFill;	// Disable interior fill
	BOOL	m_bExtraSize;	// Is Visual Manager's extra size used?
	BOOL	m_bHorz;		// Is located on the horizontal toolbar?

	BOOL    m_bVisible;     // Is button  visible 

	CWnd*	m_pWndParent;	// Parent window
	BOOL	m_bOnGlass;		// Located on "aero" toolbar
	BOOL	m_bRibbonImage;	// Button image is provided by application Ribbon

// diagnostics:
public:

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
};

#endif // !defined(AFX_BCGPTOOLBARBUTTON_H__00F1EBC3_61BF_11D5_A304_F156F535EA11__INCLUDED_)
