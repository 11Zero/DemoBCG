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

#if !defined(AFX_BCGPDATETIMECTRL_H__FEA34C6D_5D34_461C_9403_95C23C0B6F4A__INCLUDED_)
#define AFX_BCGPDATETIMECTRL_H__FEA34C6D_5D34_461C_9403_95C23C0B6F4A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BCGPDateTimeCtrl.h : header file
//

#include "BCGCBPro.h"
#include "BCGPSpinButtonCtrl.h"

class CBCGPCalendar;

#define MAX_PARTS	8

/////////////////////////////////////////////////////////////////////////////
// CBCGPDateTimeCtrl window

class BCGCBPRODLLEXPORT CBCGPDateTimeCtrl : public CButton
{
	friend class CBCGPCalendar;
	friend class CBCGPDateTimeProp;

	DECLARE_DYNAMIC(CBCGPDateTimeCtrl)

// Construction
public:
	CBCGPDateTimeCtrl();

// Attributes
public:
    void SetFirstDayOfWeek(int nDay);	// 0 - 6
	int GetFirstDayOfWeek () const
	{
		return m_weekStart - 1;
	}

	void SizeToContent();

	BOOL	m_bEnabledInCustomizeMode;
	BOOL	m_bAutoSetFocus;
	BOOL	m_bDrawDateTimeOnly;

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

	static const UINT		DTM_SPIN;
	static const UINT		DTM_DATE;
	static const UINT		DTM_TIME;
	static const UINT		DTM_CHECKBOX;
	static const UINT		DTM_DROPCALENDAR;
	static const UINT		DTM_TIME24H;
	static const UINT		DTM_CHECKED;
	static const UINT		DTM_TIME24HBYLOCALE;
	static const UINT		DTM_SECONDS;

	int m_monthFormat;	// 0 - short, 1 - long, 2 - numeric

	void SetDontChangeLocale(BOOL bSet = TRUE)
	{
		m_bDontChangeLocale = bSet;
	}
	
	BOOL IsDontChangeLocale() const
	{
		return m_bDontChangeLocale;
	}

	BOOL IsDroppedDown() const
	{
		return m_pPopup != NULL;
	}

// Operations
public:

	DATE GetDate();
	void SetDate(DATE newValue);
	DATE GetMinDate();
	void SetMinDate(DATE newValue);
	DATE GetMaxDate();
	void SetMaxDate(DATE newValue);

	UINT GetState () const;
	void SetState (UINT flags, UINT stateMask);
	BOOL GetAutoResize() const
	{
		return m_bAutoResize;
	}

	void SetAutoResize(BOOL bSet);

	BOOL IsPropListMode() const
	{
		return m_bPropListMode;
	}

	void AdjustControl (CRect rectClient);

	void EnableVisualManagerStyle(BOOL bEnable = TRUE);
	BOOL IsVisualManagerStyle() const
	{
		return m_bVisualManagerStyle;
	}
	
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPDateTimeCtrl)
	public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	protected:
	virtual void PreSubclassWindow();
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBCGPDateTimeCtrl();
	
	virtual CString ToString () const;

protected:
	virtual void OnHideCalendarPopup ();
	virtual void OnShowCalendarPopup ();

	virtual void OnDateChanged ();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPDateTimeCtrl)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg UINT OnGetDlgCode();
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp);
	afx_msg void OnNcPaint();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
	afx_msg void OnEnable(BOOL bEnable);
	afx_msg void OnCancelMode();
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	//}}AFX_MSG
	afx_msg LRESULT OnSetFont (WPARAM, LPARAM);
	afx_msg LRESULT OnGetFont (WPARAM, LPARAM);
	afx_msg LRESULT OnMouseLeave(WPARAM wp,LPARAM lp);
	afx_msg LRESULT OnBCGSetControlVMMode (WPARAM wp, LPARAM);
	afx_msg LRESULT OnPrint(WPARAM wp, LPARAM lp);
	DECLARE_MESSAGE_MAP()

// Attributes
public:
	BOOL m_type2DigitsInYear;
	long m_maxYear2Digits;

protected:
	BOOL m_checkButton;
	BOOL m_dropCalendar;
	BOOL m_showDate;
	BOOL m_showTime;
	BOOL m_spinButton;
	BOOL m_bAutoResize;
	BOOL m_bVisualManagerStyle;

	HFONT	m_hFont;

protected:
	enum PART_TYPE
	{
		NO,
		CHECK_BOX,
		DAY,
		MONTH,
		YEAR,
		HOUR,
		MIN,
		AMPM,
		SECONDS,
	};

	CRect					m_rectCheck;
	CRect					m_rectDropButton;
	CRect					m_rectText;
	COleDateTime			m_Date;
	COleDateTime			m_MinDate;
	COleDateTime			m_MaxDate;
	PART_TYPE				m_CurrPartType;
	int						m_iPartNum;
	BOOL					m_bCheckBoxIsAvailable;
	BOOL					m_bIsChecked;
	PART_TYPE				m_arPartsOrder [MAX_PARTS];
	CRect					m_arPartRects [MAX_PARTS];
	int						m_iPartsNumber;
	BOOL					m_b24HoursFormat;
	BOOL					m_b24HoursByLocale;
	BOOL					m_bSeconds;
	BOOL					m_bShowSelection;
	CString					m_strDateSeparator;
	CString					m_strTimeSeparator;
	int						m_iPrevDigit;
	CBCGPSpinButtonCtrl		m_wndSpin;
	BOOL					m_bIsDateComboDropped;
	CBCGPCalendar*			m_pPopup;
	BOOL					m_bDropButtonIsPressed;
	BOOL					m_bMouseOnDropButton;
	BOOL					m_bMouseOnCheckBox;
	int						m_iControlWidth;
	int						m_iControlHeight;
	CString					m_strAM;
	CString					m_strPM;
	COleDateTime			m_WidestDate;
	int						m_iYearPos;
	int						m_weekStart;
	BOOL					m_bIsInitialized;
	COLORREF				m_colorText;
	COLORREF				m_colorBackground;
	BOOL					m_bTrackPart;
	BOOL					m_bPropListMode;
	BOOL					m_bDontChangeLocale;

// Operations
protected:
	void DrawCheckBox (CDC* pDC, UINT uiState);
	void DrawDateDropButton (CDC* pDC, UINT uiState);
	void SelectNext ();
	void SelectPrev ();
	void SetPartsOrder ();
	LPCTSTR GetPartFormat (int iPart) const;
	int GetPartFromPoint (POINT point);
	void ScrollCurrPart (int iDir);
	void ScrollCurrPartToLimit (BOOL bTop);
	void PushDigit (int iDigit);
	void ChangeMonth (UINT uiMonth);
	void ChangeAmPm (UINT uiAmPm);
	void ToggleCheck ();

	BOOL IsDateValid (COleDateTime& date) const;

	BOOL IsDatePart (int iPart) const;
	BOOL IsTimePart (int iPart) const;

	void BuidWidestDate (CDC* pDC);
	void CalcPartRects (CDC* pDC);

	CString PartToString(const COleDateTime& date, int iPart) const;

	void ClosePopupCalendar (COleDateTime date = COleDateTime ());

	BOOL DoScroll(int nScrollSteps);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPDATETIMECTRL_H__FEA34C6D_5D34_461C_9403_95C23C0B6F4A__INCLUDED_)
