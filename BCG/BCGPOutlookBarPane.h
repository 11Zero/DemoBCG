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

#if !defined(AFX_BCGPOUTLOOKBARPANE_H__CEC069F8_8AE2_4EC3_BB74_03CF55EACA94__INCLUDED_)
#define AFX_BCGPOUTLOOKBARPANE_H__CEC069F8_8AE2_4EC3_BB74_03CF55EACA94__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BCGPOutlookBarPane.h : header file
//

#include "BCGCBPro.h"
#include "BCGPToolbar.h"
#include "BCGPButton.h"
#include "BCGPToolbarImages.h"

/////////////////////////////////////////////////////////////////////////////
// CBCGPOutlookBarPane window

class BCGCBPRODLLEXPORT CBCGPOutlookBarPane : public CBCGPToolBar
{
	friend class CBCGPOutlookButton;
	friend class CBCGPVisualManager;
	friend class CBCGPOutlookWnd;

	DECLARE_SERIAL(CBCGPOutlookBarPane)

// Construction
public:
	CBCGPOutlookBarPane();

// Attributes
public:

// Operations
public:
	//--------------------
	// Add/remove buttons:
	//--------------------

	BOOL AddButton (UINT uiImage, LPCTSTR lpszLabel, UINT iIdCommand, int iInsertAt = -1);
	BOOL AddButton (UINT uiImage, UINT uiLabel, UINT iIdCommand, int iInsertAt = -1);
	BOOL AddButton (LPCTSTR szBmpFileName, LPCTSTR szLabel, UINT iIdCommand, int iInsertAt = -1);

	BOOL AddButton (HBITMAP hBmp, LPCTSTR lpszLabel, UINT iIdCommand, int iInsertAt = -1);
	BOOL AddButton (HICON hIcon, LPCTSTR lpszLabel, UINT iIdCommand, int iInsertAt = -1, BOOL bAlphaBlend = FALSE);

	BOOL RemoveButton (UINT iIdCommand);
	BOOL RemoveButtonByIndex (int nIndex);

	void ClearAll ();
	void SetDefaultState ();

	//--------------
	// General look:
	//--------------
	void SetTextColor (COLORREF clrRegText, COLORREF clrSelText = 0/* Obsolete*/);
	void SetTransparentColor (COLORREF color);
	void SetBackImage (UINT uiImageID);
	void SetBackColor (COLORREF color);

	void SetExtraSpace (int nSpace)	// Set extra space betwen buttons
	{
		m_nExtraSpace = nSpace;
	}
	
	virtual BOOL CanBeAttached () const
	{
		return TRUE;
	}

	virtual BCGP_CS_STATUS IsChangeState (int nOffset, CBCGPBaseControlBar** ppTargetBar) const;
	virtual BOOL Dock (CBCGPBaseControlBar* pDockBar, LPCRECT lpRect, 
						BCGP_DOCK_METHOD dockMethod);
	virtual BOOL OnBeforeFloat (CRect& rectFloat, BCGP_DOCK_METHOD dockMethod);

	virtual BOOL RestoreOriginalstate ();
	virtual BOOL SmartUpdate (const CObList& lstPrevButtons);

	virtual BOOL CanBeRestored () const
	{
		return !m_OrigButtons.IsEmpty ();
	}

	virtual void RemoveAllButtons ();

protected:
	BOOL InternalAddButton (int iImageIndex, LPCTSTR szLabel, UINT iIdCommand, int iInsertAt = -1);
	int	 AddBitmapImage (HBITMAP hBitmap);

	void ScrollUp ();
	void ScrollDown ();
	void ScrollPageUp ();
	void ScrollPageDown ();

	void CopyButtonsList (const CObList& lstSrc, CObList& lstDst);

// Overrides
	virtual CSize CalcFixedLayout (BOOL bStretch, BOOL bHorz);

	virtual void AdjustLocations ();
	virtual void DoPaint(CDC* pDC);
	virtual DROPEFFECT OnDragOver(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	virtual CBCGPToolbarButton* CreateDroppedButton (COleDataObject* pDataObject);

	virtual BOOL EnableContextMenuItems (CBCGPToolbarButton* pButton, CMenu* pPopup);

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void OnEraseWorkArea (CDC* /*pDC*/, CRect /*rectWorkArea*/) {} /* Obsolete*/

	virtual DWORD GetCurrentAlignment () const
	{
		return (m_dwStyle & CBRS_ALIGN_ANY) & ~CBRS_ORIENT_HORZ;
	}

	virtual BOOL AllowShowOnList () const		{	return FALSE;	}
	virtual BOOL CanFloat () const				{	return FALSE;	}

	virtual void AddRemoveSeparator (const CBCGPToolbarButton* /*pButton*/,
				const CPoint& /*ptStart*/, const CPoint& /*ptDrop*/)	{}

	virtual BOOL AllowShowOnControlMenu () const	{	return FALSE;	}

	virtual BOOL OnGestureEventPan(const CPoint& ptFrom, const CPoint& ptTo, CSize& sizeOverPan);
	virtual BOOL InternalScroll(int nDelta, CSize& sizeOverPan);

public:
	virtual BOOL Create(CWnd* pParentWnd,
			DWORD dwStyle = dwDefaultToolbarStyle,
			UINT uiID = (UINT)-1,
			DWORD dwBCGStyle = 0);

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPOutlookBarPane)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBCGPOutlookBarPane();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPOutlookBarPane)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp);
	afx_msg void OnSysColorChange();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnNcDestroy();
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	COLORREF GetRegularColor () const
	{
		return m_clrRegText;
	}

	BOOL IsDrawShadedHighlight () const
	{
		return m_bDrawShadedHighlight;
	}

	BOOL IsBackgroundTexture () const
	{
		return m_bmpBack.GetCount () != 0;
	}

	void EnablePageScrollMode(BOOL bPageScroll = TRUE)
	{
		m_bPageScrollMode = bPageScroll;
	}

// Attributes:
protected:
	COLORREF			m_clrRegText;
	COLORREF			m_clrTransparentColor;
	COLORREF			m_clrBackColor;

	int					m_nSize;	// Width or Height, orientation dependable

	CBCGPToolBarImages	m_bmpBack;
	UINT				m_uiBackImageId;

	CBCGPButton			m_btnUp;
	CBCGPButton			m_btnDown;
	int					m_iScrollOffset;
	int					m_iFirstVisibleButton;
	BOOL				m_bScrollDown;

	static CSize				m_csImage;
	static CBCGPToolBarImages	m_Images;

	BOOL				m_bDrawShadedHighlight;

	int					m_nExtraSpace;

	HWND				m_hRecentOutlookWnd;
	BOOL				m_bDontAdjustLayout;
	BOOL				m_bPageScrollMode;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPOUTLOOKBARPANE_H__CEC069F8_8AE2_4EC3_BB74_03CF55EACA94__INCLUDED_)
