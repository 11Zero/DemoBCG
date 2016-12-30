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

#if !defined(AFX_BCGPPOPUPMENUBAR_H__A6E1AD76_D511_11D1_A64D_00A0C93A70EC__INCLUDED_)
#define AFX_BCGPPOPUPMENUBAR_H__A6E1AD76_D511_11D1_A64D_00A0C93A70EC__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// BCGPPopupMenuBar.h : header file
//

#include "BCGCBPro.h"
#include "BCGPToolBar.h"
#include "BCGPMenuBar.h"

class CBCGPToolbarMenuButton;

/////////////////////////////////////////////////////////////////////////////
// CBCGPPopupMenuBar command target

class BCGCBPRODLLEXPORT CBCGPPopupMenuBar : public CBCGPToolBar
{
	friend class CBCGPPopupMenu;
	friend class CBCGPDropDownList;
	friend class CBCGPCustomizeMenuButton;

	DECLARE_SERIAL(CBCGPPopupMenuBar)

public:
	CBCGPPopupMenuBar();
	virtual ~CBCGPPopupMenuBar();

// Attributes
public:
	UINT GetDefaultMenuId () const
	{
		return m_uiDefaultMenuCmdId;
	}

	int GetOffset () const
	{
		return m_iOffset;
	}

	void SetOffset (int iOffset)
	{
		if (m_iOffset != iOffset)
		{
			m_iOffset = iOffset;
			AdjustLocations ();
		}
	}

	virtual CSize GetCurrentMenuImageSize () const
	{
		if (m_pRelatedToolbar == NULL || !m_pRelatedToolbar->IsLocked ())
		{
			return CBCGPToolBar::GetMenuImageSize ();
		}

		return m_pRelatedToolbar->GetLockedImageSize ();
	}

	BOOL			m_bDisableSideBarInXPMode;
	CBCGPToolBar*	m_pRelatedToolbar;

	BOOL IsDropDownListMode () const
	{
		return m_bDropDownListMode;
	}

protected:
	UINT						m_uiDefaultMenuCmdId;
	CBCGPToolbarMenuButton*		m_pDelayedPopupMenuButton;
	CBCGPToolbarMenuButton*		m_pDelayedClosePopupMenuButton;
	BOOL						m_bFirstClick;
	BOOL						m_bFirstMove;
	int							m_iOffset;	// rows
	int							m_xSeparatorOffsetLeft;
	int							m_xSeparatorOffsetRight;
	BOOL						m_bAreAllCommandsShown;
	int							m_iMaxWidth;
	int							m_iMinWidth;
	CUIntArray					m_arColumns;
	CMap<UINT, UINT,UINT,UINT>	m_HiddenItemsAccel;	// Accell. <==> Command
	BOOL						m_bInCommand;
	BOOL						m_bTrackMode;
	BOOL						m_bIsClickOutsideItem;
	BOOL						m_bPaletteMode;
	BOOL						m_bPaletteRows;
	BOOL						m_bDropDownListMode;
	BOOL						m_nDropDownPageSize;
	BOOL						m_bInScrollMode;
	BOOL						m_bResizeTracking;
	CPoint						m_ptCursorInit;
	BOOL						m_bGutterLogo;

	static UINT					m_uiPopupTimerDelay;
	static int					m_nLastCommandIndex;

// Operations
public:
	virtual BOOL ImportFromMenu (HMENU hMenu, BOOL bShowAllCommands = FALSE);
	virtual HMENU ExportToMenu () const;

	void StartPopupMenuTimer (CBCGPToolbarMenuButton* pMenuButton, int nDelayFactor = 1);
	CBCGPToolBar* FindDestBar (CPoint point);

	virtual void CloseDelayedSubMenu ();
	virtual void RestoreDelayedSubMenu ();

	BOOL LoadFromHash (HMENU hMenu);

	static int GetLastCommandIndex ()
	{
		return m_nLastCommandIndex;
	}

	BOOL IsPaletteMode () const
	{
		return m_bPaletteMode;
	}

	virtual BOOL IsRibbonPanel () const
	{
		return FALSE;
	}

	virtual BOOL IsRibbonPanelInRegularMode () const
	{
		return FALSE;
	}

	virtual void AdjustSizeImmediate (BOOL /*bRecalcLayout = TRUE*/) {}
	
	BOOL BuildOrigItems (UINT uiMenuResID);

	int GetGutterWidth ();
	BOOL HasGutterLogo() const
	{
		return m_bGutterLogo;
	}

	virtual BOOL IsDrawGutter() const
	{
		return !m_bDisableSideBarInXPMode;
	}

public:
// Overrides
	virtual void SetButtonStyle(int nIndex, UINT nStyle);

	virtual HRESULT get_accRole(VARIANT varChild, VARIANT *pvarRole);
	virtual HRESULT get_accState(VARIANT varChild, VARIANT *pvarState);
	virtual HRESULT get_accParent(IDispatch **ppdispParent);
	virtual HRESULT get_accName(VARIANT varChild, BSTR *pszName);

protected:
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPPopupMenuBar)
	//}}AFX_VIRTUAL

	virtual BOOL OnSendCommand (const CBCGPToolbarButton* pButton);
	virtual void AdjustLocations ();
	virtual void DrawSeparator (CDC* pDC, const CRect& rect, BOOL bHorz);
	virtual CSize CalcSize (BOOL bVertDock);
	virtual BOOL AllowSelectDisabled () const	{	return CBCGPMenuBar::IsHighlightDisabledItems ();	}
	virtual BOOL AllowShowOnList () const		{	return FALSE;	}
	virtual int FindDropIndex (const CPoint p, CRect& rectDrag) const;
	virtual void DrawDragMarker (CDC* pDC);
	virtual CBCGPToolbarButton* CreateDroppedButton (COleDataObject* pDataObject);
	virtual void OnChangeHot (int iHot);
	virtual BOOL OnKey (UINT nChar);
	virtual BOOL OnSetDefaultButtonText (CBCGPToolbarButton* pButton);
	virtual BOOL EnableContextMenuItems (CBCGPToolbarButton* pButton, CMenu* pPopup);
	virtual void OnFillBackground (CDC* pDC);
	virtual void ShowCommandMessageString (UINT uiCmdId);

	virtual DROPEFFECT OnDragOver(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	virtual INT_PTR OnToolHitTest(CPoint point, TOOLINFO* pTI) const;

	virtual BOOL OnUserToolTip (CBCGPToolbarButton* /*pButton*/, CString& /*strTTText*/) const
	{
		return FALSE;
	}

	virtual CWnd* GetCommandTarget () const;
	void InvokeMenuCommand (UINT uiCmdId, const CBCGPToolbarButton* pMenuItem);

	void SetInCommand (BOOL bInCommand = TRUE);

	virtual BOOL TranslateChar (UINT /*nChar*/)
	{
		return FALSE;
	}

	virtual BOOL IsButtonExtraSizeAvailable () const
	{
		return FALSE;
	}

	virtual void AdjustLayout ();

	virtual void OnCalcSeparatorRect (CBCGPToolbarButton* pButton, 
										CRect& rectSeparator, 
										BOOL bHorz);
	virtual BOOL AllowAltCustomization () const	{	return FALSE;	}
	virtual void OnAfterButtonDelete ();

	virtual BOOL AllowShowOnControlMenu () const			{	return FALSE;	}
	virtual BOOL AlwaysSaveSelection () const				{	return FALSE;	}

	virtual CSize GetEmptyMenuSize() const;

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CBCGPPopupMenuBar)
	afx_msg void OnNcPaint();
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp);
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg LRESULT OnIdleUpdateCmdUI(WPARAM wParam, LPARAM lParam);
	afx_msg void OnToolbarImageAndText();
	afx_msg void OnToolbarText();
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPPOPUPMENUBAR_H__A6E1AD76_D511_11D1_A64D_00A0C93A70EC__INCLUDED_)
