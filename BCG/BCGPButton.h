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

#if !defined(AFX_BCGPBUTTON_H__C159C4C6_B79B_11D3_A712_009027900694__INCLUDED_)
#define AFX_BCGPBUTTON_H__C159C4C6_B79B_11D3_A712_009027900694__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BCGPButton.h : header file
//

#include "BCGCBPro.h"
#include "MenuImages.h"

/////////////////////////////////////////////////////////////////////////////
// CBCGPButton window

class BCGCBPRODLLEXPORT CBCGPButton : public CButton
{
	friend class CBCGPVisualManager;

	DECLARE_DYNCREATE(CBCGPButton)

// Construction
public:
	CBCGPButton();

// Attributes
public:
	enum FlatStyle
	{
		BUTTONSTYLE_3D,
		BUTTONSTYLE_FLAT,
		BUTTONSTYLE_SEMIFLAT,
		BUTTONSTYLE_NOBORDERS
	};

	enum AlignStyle
	{
		ALIGN_LEFT,
		ALIGN_RIGHT,
		ALIGN_CENTER
	};
	
	FlatStyle		m_nFlatStyle;
	AlignStyle		m_nAlignStyle;
	BOOL			m_bRighImage;	// Is image on the right side?
	BOOL			m_bTopImage;	// Is image on top?
	BOOL			m_bTransparent;
	BOOL			m_bDrawFocus;
	BOOL			m_bHighlightChecked;
	BOOL			m_bGrayDisabled;
	BOOL			m_bResponseOnButtonDown;
	BOOL			m_bDontUseWinXPTheme;
	BOOL			m_bVisualManagerStyle;
	BOOL			m_bOnGlass;
	BOOL			m_bBackstageMode;
	BOOL			m_bDontAutoGrayImage;

	BOOL			m_bWasDblClk;

	CToolTipCtrl& GetToolTipCtrl ()
	{
		if (m_pToolTip == NULL)
		{
			static CToolTipCtrl emptyTooltip;
			return emptyTooltip;
		}

		return (*m_pToolTip);
	}

	BOOL IsPressed () const
	{
		return m_bPushed && m_bHighlighted;
	}

	BOOL IsPushed () const
	{
		return m_bPushed;
	}

	BOOL IsHighlighted () const
	{
		return m_bHighlighted;
	}

	BOOL IsCheckBox () const 
	{ 
		return m_bCheckButton;
	}

	BOOL IsRadioButton () const 
	{ 
		return m_bRadioButton; 
	}

	BOOL IsAutoCheck () const 
	{ 
		return m_bAutoCheck; 
	}

	void SetAutoCheck(BOOL bSet = TRUE);

	BOOL IsChecked () const
	{
		return m_bChecked;
	}

	BOOL IsIndeterminate () const
	{
		return m_bIndeterminate;
	}

	BOOL IsDefaultButton () const
	{
		return m_bDefaultButton;
	}

	static BOOL		m_bDontSkin;

protected:
	BOOL			m_bPushed;
	BOOL			m_bClickiedInside;
	BOOL			m_bHighlighted;
	BOOL			m_bCaptured;
	BOOL			m_bHover;
	BOOL			m_bChecked;
	BOOL			m_bIndeterminate;
	BOOL			m_bCheckButton;
	BOOL			m_b3State;
	BOOL			m_bRadioButton;
	BOOL			m_bAutoCheck;
	BOOL			m_bDefaultButton;
	BOOL			m_bIsLeftText;
	BOOL			m_bDrawText;
	CString			m_strDescription;

	CSize						m_sizeImage;
	CBCGPToolBarImages			m_Image;
	CBCGPToolBarImages			m_ImageHot;
	CBCGPToolBarImages			m_ImageDisabled;
	CBCGPToolBarImages			m_ImageChecked;
	CBCGPToolBarImages			m_ImageCheckedHot;
	CBCGPToolBarImages			m_ImageCheckedDisabled;
	BOOL						m_bAutoDestroyImage;
	BOOL						m_bFullTextTooltip;
	BOOL						m_bDelayFullTextTooltipSet;

	CBCGPMenuImages::IMAGES_IDS		m_nStdImageId;
	CBCGPMenuImages::IMAGES_IDS		m_nStdImageDisabledId;
	CBCGPMenuImages::IMAGE_STATE	m_StdImageState;
	CBCGPMenuImages::IMAGE_STATE	m_StdImageStateDisabled;

	CToolTipCtrl*	m_pToolTip;

	HCURSOR			m_hCursor;

	CSize			m_sizePushOffset;

	COLORREF		m_clrRegular;
	COLORREF		m_clrHover;
	COLORREF		m_clrFace;
	COLORREF		m_clrText;

	HFONT			m_hFont;
	int				m_nAutoRepeatTimeDelay;

	static BOOL		m_bWinXPTheme;
	static BOOL		m_bWinXPThemeWasChecked;

// Operations
public:
	void SetImage (HICON hIcon, BOOL bAutoDestroy = TRUE, HICON hIconHot = NULL, HICON hIconDisabled = NULL, BOOL bAlphaBlend = FALSE);
	void SetImage (HBITMAP hBitmap, BOOL bAutoDestroy = TRUE, HBITMAP hBitmapHot = NULL, BOOL bMap3dColors = TRUE, HBITMAP hBitmapDisabled = NULL);
	void SetImage (UINT uiBmpResId, UINT uiBmpHotResId = 0, UINT uiBmpDsblResID = 0);

	void SetCheckedImage (HICON hIcon, BOOL bAutoDestroy = TRUE, HICON hIconHot = NULL, HICON hIconDisabled = NULL, BOOL bAlphaBlend = FALSE);
	void SetCheckedImage (HBITMAP hBitmap, BOOL bAutoDestroy = TRUE, HBITMAP hBitmapHot = NULL, BOOL bMap3dColors = TRUE, HBITMAP hBitmapDisabled = NULL);
	void SetCheckedImage (UINT uiBmpResId, UINT uiBmpHotResId = 0, UINT uiBmpDsblResID = 0);

	void SetStdImage (CBCGPMenuImages::IMAGES_IDS id, CBCGPMenuImages::IMAGE_STATE state = CBCGPMenuImages::ImageBlack,
		CBCGPMenuImages::IMAGES_IDS idDisabled = (CBCGPMenuImages::IMAGES_IDS) -1,
		CBCGPMenuImages::IMAGE_STATE stateDisabled = (CBCGPMenuImages::IMAGE_STATE)-1);

	void SetTooltip (LPCTSTR lpszToolTipText);	// lpszToolTip == NULL: disable tooltip
	void GetTooltip (CString& strTooltip);

	void SetDescription(LPCTSTR lpszDescr);
	void GetDescription (CString& strDescr) const;

	void SetMouseCursor (HCURSOR hcursor);
	void SetMouseCursorHand ();

	void SetTextColor (COLORREF clrText)
	{
		m_clrRegular = clrText;
	}

	void SetTextHotColor (COLORREF clrTextHot)
	{
		m_clrHover = clrTextHot;
	}

	void SetFaceColor (COLORREF crFace, BOOL bRedraw = TRUE);

	void EnableMenuFont (BOOL bOn = TRUE, BOOL bRedraw = TRUE);
	void EnableFullTextTooltip (BOOL bOn = TRUE);

	void SetAutorepeatMode (int nTimeDelay = 500 /* ms */);
	BOOL IsAutorepeatCommandMode () const
	{
		return m_nAutoRepeatTimeDelay > 0;
	}

	void SetDrawText(BOOL bSet = TRUE, BOOL bRedraw = TRUE);
	BOOL IsDrawText() const
	{
		return m_bDrawText;
	}

	// Allow to draw button borders using current WinXP theme.
	// CBCGWinXPVisualManager should be active visual manager:
	static void EnableWinXPTheme (BOOL bEnable = TRUE);
	static BOOL IsWinXPThemeEnabled ()
	{
		return m_bWinXPTheme;
	}

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPButton)
	public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void PreSubclassWindow();
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBCGPButton();
	virtual void CleanUp ();

	virtual CSize SizeToContent (BOOL bCalcOnly = FALSE);
	virtual void OnDrawParentBackground (CDC* pDC, CRect rectClient);

protected:
	virtual void DoDrawItem (CDC* pDCPaint, CRect rectClient, UINT itemState);
	virtual void OnFillBackground (CDC* pDC, const CRect& rectClient);
	virtual void OnDrawBorder (CDC* pDC, CRect& rectClient, UINT uiState);
	virtual void OnDrawFocusRect (CDC* pDC, const CRect& rectClient);
	virtual void OnDraw (CDC* pDC, const CRect& rect, UINT uiState);

	virtual void OnDrawText (CDC* pDC, const CRect& rect, const CString& strText,
							UINT uiDTFlags, UINT uiState);

	virtual CFont* SelectFont (CDC* pDC);

	virtual int GetImageHorzMargin() const;
	virtual int GetVertMargin() const;

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPButton)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnCancelMode();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnEnable(BOOL bEnable);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnDestroy();
	afx_msg UINT OnGetDlgCode();
	afx_msg void OnPaint();
	//}}AFX_MSG
	afx_msg LRESULT OnPrintClient(WPARAM, LPARAM);
	afx_msg LRESULT OnSetFont (WPARAM, LPARAM);
	afx_msg LRESULT OnGetFont (WPARAM, LPARAM);
	afx_msg LRESULT OnGetCheck (WPARAM, LPARAM);
	afx_msg LRESULT OnSetCheck (WPARAM, LPARAM);
	afx_msg LRESULT OnSetImage (WPARAM, LPARAM);
	afx_msg LRESULT OnGetImage (WPARAM, LPARAM);
	afx_msg LRESULT OnBCGUpdateToolTips (WPARAM, LPARAM);
	afx_msg LRESULT OnBCGSetControlVMMode (WPARAM, LPARAM);
	afx_msg LRESULT OnBCGSetControlAero (WPARAM, LPARAM);
	afx_msg LRESULT OnBCGSetControlBackStageMode (WPARAM, LPARAM);
	DECLARE_MESSAGE_MAP()

	void InitStyle (DWORD dwStyle);

	BOOL CheckNextPrevRadioButton (BOOL bNext);
	void UncheckRadioButtonsInGroup ();

	void SetImageInternal (HICON hIcon, BOOL bAutoDestroy, HICON hIconHot, BOOL bChecked, HICON hIconDisabled = NULL,
		BOOL bAlphaBlend = FALSE);
	void SetImageInternal (HBITMAP hBitmap, BOOL bAutoDestroy, HBITMAP hBitmapHot, BOOL bMap3dColors, BOOL bChecked, HBITMAP hBitmapDisabled = NULL);
	void SetImageInternal (UINT uiBmpResId, UINT uiBmpHotResId, BOOL bChecked, UINT uiBmpDsblResId = 0);

	void ClearImages (BOOL bChecked);

	void DrawBorder (CDC* pDC, CRect& rectClient, UINT uiState);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPBUTTON_H__C159C4C6_B79B_11D3_A712_009027900694__INCLUDED_)
