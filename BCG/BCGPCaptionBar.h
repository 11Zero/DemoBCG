// BCGPCaptionBar.h: interface for the CBCGPCaptionBar class.
//
// This is a part of BCGControlBar Library Professional Edition
// Copyright (C) 1998-2014 BCGSoft Ltd.
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPCAPTIONBAR_H__AA2360F2_0462_411A_821D_BB78BCA54E49__INCLUDED_)
#define AFX_BCGPCAPTIONBAR_H__AA2360F2_0462_411A_821D_BB78BCA54E49__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"
						    
#include "BCGPButton.h"
#include "BCGPToolBarImages.h"
#include "BCGPControlBar.h"

class BCGCBPRODLLEXPORT CBCGPCaptionBar : public CBCGPControlBar  
{
	DECLARE_DYNCREATE(CBCGPCaptionBar)

public:
	CBCGPCaptionBar();
	virtual ~CBCGPCaptionBar();

	BOOL Create (DWORD dwStyle, CWnd* pParentWnd, UINT uID, int nHeight = -1,
		BOOL bIsMessageBarMode = FALSE);

protected:
	//{{AFX_MSG(CBCGPCaptionBar)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSysColorChange();
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp);
	afx_msg void OnNcPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnDestroy();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	//}}AFX_MSG
	afx_msg LRESULT OnSetFont (WPARAM, LPARAM);
	afx_msg LRESULT OnGetFont (WPARAM, LPARAM);
	afx_msg LRESULT OnMouseLeave(WPARAM,LPARAM);
	afx_msg LRESULT OnBCGUpdateToolTips (WPARAM, LPARAM);
	afx_msg BOOL OnNeedTipText(UINT id, NMHDR* pNMH, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()

//Attributes
public:
	enum BarElement
	{
		ELEM_BUTTON,
		ELEM_TEXT,
		ELEM_ICON
	};
	enum BarElementAlignment
	{
		ALIGN_INVALID,
		ALIGN_LEFT,
		ALIGN_RIGHT,
		ALIGN_CENTER
	};

	void SetMessageBarMode(BOOL bIsMessageBarMode = TRUE);

	BOOL IsMessageBarMode () const
	{
		return m_bIsMessageBarMode;
	}

	void SetFlatBorder (BOOL bFlat = TRUE)
	{
		m_bFlatBorder = bFlat;
	}

	void SetButton		(LPCTSTR lpszLabel, UINT uiCmdUI, 
						BarElementAlignment btnAlignmnet = ALIGN_LEFT,
						BOOL bHasDropDownArrow = TRUE);
	void SetButtonToolTip (LPCTSTR lpszToolTip, LPCTSTR lpszDescription = NULL);
	void RemoveButton	();
	void EnableButton	(BOOL bEnable = TRUE);
	void SetButtonPressed (BOOL bPresed = TRUE);

	void SetIcon		(HICON hIcon, BarElementAlignment iconAlignment = ALIGN_RIGHT);
	void RemoveIcon		();

	void SetBitmap		(HBITMAP hBitmap, COLORREF clrTransparent, 
							BOOL bStretch = FALSE,
							BarElementAlignment bmpAlignment = ALIGN_RIGHT);
	void SetBitmap		(UINT uiBmpResID, COLORREF clrTransparent, 
							BOOL bStretch = FALSE,
							BarElementAlignment bmpAlignment = ALIGN_RIGHT);
	void RemoveBitmap	();
	void SetImageToolTip (LPCTSTR lpszToolTip, LPCTSTR lpszDescription = NULL);

	CString GetText		() const
	{
		return m_strText;
	}
	void SetText		(const CString& strText, BarElementAlignment textAlignment = ALIGN_RIGHT);
	void RemoveText		();

	void SetBorderSize (int nSize) {m_nBorderSize = nSize;}
	int  GetBorderSize () const {return m_nBorderSize;}

	void SetMargin (int nMargin) {m_nMargin = nMargin;}
	int  GetMargin () const {return m_nMargin;}

	BarElementAlignment GetAlignment (BarElement elem);

	COLORREF m_clrBarText;
	COLORREF m_clrBarBackground;
	COLORREF m_clrBarBorder;

	CRect GetButtonRect () const
	{
		return m_rectButton;
	}

	virtual BOOL DoesAllowDynInsertBefore () const {return FALSE;}

protected:
// Overridables
    virtual void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler);
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	virtual void DoPaint(CDC* pDC);
	virtual void DoNcPaint(CDC* pDC);

	virtual void OnDrawBackground (CDC* pDC, CRect rect);
	virtual void OnDrawBorder	  (CDC* pDC, CRect rect);
	virtual void OnDrawText		  (CDC* pDC, CRect rect, const CString& strText);
	virtual void OnDrawImage	  (CDC* pDC, CRect rect);
	virtual void OnDrawButton	  (CDC* pDC, CRect rect, const CString& strButton, BOOL bEnabled);

	virtual CSize CalcFixedLayout(BOOL bStretch, BOOL bHorz);

	virtual CSize GetTextSize	  (CDC* pDC, const CString& strText);

	virtual void AdjustLayout ();
	virtual void RecalcLayout ();

	virtual BOOL OnShowControlBarMenu (CPoint /*point*/)	{	return FALSE;	}

	virtual BOOL OnClickCloseButton()	{	return TRUE;	}

protected:

	BOOL				m_bIsMessageBarMode;
	CToolTipCtrl*		m_pToolTip;

	//------------------
	// Image attributes:
	//------------------
	HICON				m_hIcon;
	CBCGPToolBarImages	m_Bitmap;
	BOOL				m_bStretchImage;
	BarElementAlignment	m_iconAlignment;
	CRect				m_rectImage;
	CString				m_strImageToolTip;
	CString				m_strImageDescription;

	//-----------------------
	// Text label attributes:
	//-----------------------
	HFONT				m_hFont;
	CString				m_strText;
	CStringArray		m_arTextParts;
	CArray<int,int>		m_arTextPartTypes;
	CArray<RECT,RECT>	m_arHyperlinksRects;
	BarElementAlignment	m_textAlignment;
	CRect				m_rectText;
	CRect				m_rectDrawText;
	BOOL				m_bTextIsTruncated;
	int					m_nHyperlinkHighlighted;
	int					m_nHyperlinkPressed;

	//-------------------
	// Button attributes:
	//-------------------
	CString				m_strBtnText;
	CString				m_strButtonToolTip;
	CString				m_strButtonDescription;
	UINT				m_uiBtnID;
	BarElementAlignment	m_btnAlignnment;
	CRect				m_rectButton;
	BOOL				m_bIsBtnPressed;
	BOOL				m_bIsBtnHighlighted;
	BOOL				m_bIsBtnForcePressed;
	BOOL				m_bTracked;
	BOOL				m_bBtnEnabled;
	BOOL				m_bBtnHasDropDownArrow;

	int					m_nBorderSize;
	int					m_nMargin;
	int					m_nHorzElementOffset;

	int					m_nDefaultHeight;
	int					m_nCurrentHeight;

	BOOL				m_bFlatBorder;

	//-------------------------
	// Close Button attributes:
	//-------------------------
	BOOL				m_bIsCloseBtnPressed;
	BOOL				m_bIsCloseBtnHighlighted;
	BOOL				m_bCloseTracked;
	CRect				m_rectClose;

protected:
	CSize GetImageSize () const;

	BOOL CheckRectangle		(CRect& rectSrc, const CRect& rectOther, BOOL bLeftOf);
	void AdjustRectToMargin (CRect& rectSrc, const CRect& rectClient, int nMargin, 
							 BOOL bRetainSize = FALSE);

	void UpdateTooltips ();

	bool IsImageSet () const
	{
		return (m_hIcon != NULL || m_Bitmap.GetCount () > 0);
	}

	int HitTestHyperlink(CPoint point);
};

extern BCGCBPRODLLEXPORT UINT BCGM_ON_CLICK_CAPTIONBAR_HYPERLINK;

#endif // !defined(AFX_BCGPCAPTIONBAR_H__AA2360F2_0462_411A_821D_BB78BCA54E49__INCLUDED_)
