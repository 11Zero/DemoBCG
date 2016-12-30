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
// BCGPDurationCtrl.h: interface for the CBCGPDurationCtrl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPDURATIONCTRL_H__4AD6FDCB_64F2_4906_B871_7A882DEDB32E__INCLUDED_)
#define AFX_BCGPDURATIONCTRL_H__4AD6FDCB_64F2_4906_B871_7A882DEDB32E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGPSpinButtonCtrl.h"

#ifndef _BCGPCALENDAR_STANDALONE
#if (!defined _BCGSUITE_) && (!defined _BCGSUITE_INC_)
	#include "BCGPButton.h"
#endif
#endif

#define PARTS_NUM	8

class BCGCBPRODLLEXPORT CBCGPDurationCtrl : public CButton
{
public:
	UINT GetState () const;
	void SetState (UINT flags, UINT mask);

	COleDateTimeSpan	GetDuration() const;
	void SetDuration(const COleDateTimeSpan& timeSpan);

	CBCGPDurationCtrl();
	virtual ~CBCGPDurationCtrl();

	static const UINT DRTN_DAYS;
	static const UINT DRTN_HOURS_MINS;
	static const UINT DRTN_SECONDS;
	static const UINT DRTN_SPIN;

	void SetAutoResize (BOOL bAutoResize = TRUE)
	{
		m_bAutoResize = bAutoResize;

		if (GetSafeHwnd () != NULL)
		{
			SizeToContent ();
			RedrawWindow ();
		}
	}

	BOOL GetAutoResize () const
	{
		return m_bAutoResize;
	}

	void AdjustControl (CRect rectClient, BOOL bRedraw = TRUE);
	void SizeToContent(BOOL bRedraw = TRUE);

	void SetTextColor (COLORREF color, BOOL bRedraw = TRUE);
	COLORREF GetTextColor () const
	{
		return m_colorText;
	}

	void SetBackgroundColor (COLORREF color, BOOL bRedraw = TRUE);
	COLORREF GetBackgroundColor () const
	{
		return m_colorBackground;
	}

	CString	GetDaysLabel () const
	{
		return m_strDaysLabel;
	}

	CString	GetHoursLabel () const
	{
		return m_strHoursLabel;
	}

	CString	GetMinutesLabel () const
	{
		return m_strMinutesLabel;
	}

	CString	GetSecondsLabel () const
	{
		return m_strSecondsLabel;
	}

	void SetDaysLabel (CString strLabel, BOOL bRedraw = TRUE);
	void SetHoursLabel (CString strLabel, BOOL bRedraw = TRUE);
	void SetMinutesLabel (CString strLabel, BOOL bRedraw = TRUE);
	void SetSecondsLabel (CString strLabel, BOOL bRedraw = TRUE);

	void EnableVisualManagerStyle(BOOL bEnable = TRUE);
	BOOL IsVisualManagerStyle() const
	{
		return m_bVisualManagerStyle;
	}
	
protected:
	enum PART_TYPE
	{
		UNDEFINED_PART = -1,
		DAYS,
		DAYS_LABEL,
		HOURS,
		HOURS_LABEL,
		MINUTES,
		MINUTES_LABEL,
		SECONDS,
		SECONDS_LABEL,
	};

// Operations:
protected:
	void GetPartSize (const CString& strText, CSize& size);
	void SelectPart (int iNewSel);
	void SelectNext ();
	void SelectPrev ();
	void PushDigit (int iDigit);
	void ScrollCurrPart (int iDelta);
	BOOL DoScroll(int nScrollSteps);

protected:
	BOOL				m_spinButton;
	BOOL				m_bShowDays;
	BOOL				m_bShowHoursMinutes;
	BOOL				m_bShowSeconds;

	CString				m_strDaysLabel;
	CString				m_strHoursLabel;
	CString				m_strMinutesLabel;
	CString				m_strSecondsLabel;

	CBCGPSpinButtonCtrl	m_wndSpin;

	PART_TYPE			m_CurrPartType;
	CString				m_strTimeSeparator;
	int					m_iPrevDigit;
	int					m_iSelectedPart;

	CRect				m_rectParts [PARTS_NUM];

	int					m_lDays;
	int					m_lHours;
	int					m_lMinutes;
	int					m_lSeconds;

	int					m_iControlWidth;
	int					m_iControlHeight;

	BOOL				m_bAutoResize;
	BOOL				m_bIsInitialized;

	COLORREF			m_colorText;
	COLORREF			m_colorBackground;

	PART_TYPE	GetPartType(const CPoint& pt);

	HFONT				m_hFont;
	BOOL				m_bVisualManagerStyle;

// Overrides
protected:
	virtual void OnDurationChanged ();

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPDurationCtrl)
	public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	protected:
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void PreSubclassWindow();
	//}}AFX_VIRTUAL

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPDurationCtrl)
	afx_msg UINT OnGetDlgCode();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp);
	afx_msg void OnNcPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnEnable(BOOL bEnable);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	afx_msg LRESULT OnPrint(WPARAM wp, LPARAM lp);
	afx_msg LRESULT OnSetFont (WPARAM, LPARAM);
	afx_msg LRESULT OnGetFont (WPARAM, LPARAM);
	afx_msg LRESULT OnBCGSetControlVMMode (WPARAM wp, LPARAM);
	DECLARE_MESSAGE_MAP()
};

#endif // !defined(AFX_BCGPDURATIONCTRL_H__4AD6FDCB_64F2_4906_B871_7A882DEDB32E__INCLUDED_)
