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
// BCGPScrollBar.h : header file
//

#if !defined(AFX_BCGPSCROLLBAR_H__A96CA752_C9F3_4FEB_8D6F_9057D17337A0__INCLUDED_)
#define AFX_BCGPSCROLLBAR_H__A96CA752_C9F3_4FEB_8D6F_9057D17337A0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"
#include "BCGPVisualManager.h"

BCGCBPRODLLEXPORT extern UINT BCGM_ON_DRAW_SCROLLBAR;	// wParam - scrollbar ID, lParam - pointer to BCGPSCROLLBAR_DRAW_INFO

/////////////////////////////////////////////////////////////////////////////
// BCGPSCROLLBAR_DRAW_INFO struct, used in BCGM_ON_DRAW_SCROLLBAR message

class CBCGPScrollBar;

enum BCGPSB_PART
{
	BCGP_SCROLLBAR_BACKGROUND	= 0,
	BCGP_SCROLLBAR_BUTTON		= 1,
	BCGP_SCROLLBAR_THUMB		= 2,
};

struct BCGCBPRODLLEXPORT BCGPSCROLLBAR_DRAW_INFO
{
	CDC*			m_pDC;
	CBCGPScrollBar*	m_pScrollBar;
	BCGPSB_PART		m_part;
	CRect			m_rect;
	BOOL			m_bFirst;
	BOOL			m_bHorz;
	BOOL			m_bHighlighted;
	BOOL			m_bPressed;
	BOOL			m_bDisabled;

	BCGPSCROLLBAR_DRAW_INFO (
		CDC* pDC, CBCGPScrollBar* pScrollBar, BCGPSB_PART part, 
		CRect rect, BOOL bHorz, BOOL bHighlighted, BOOL bPressed, 
		BOOL bFirst, BOOL bDisabled)
	{
		m_pDC = pDC;
		m_pScrollBar = pScrollBar;
		m_part = part;
		m_rect = rect;
		m_bHorz = bHorz;
		m_bHighlighted = bHighlighted;
		m_bPressed = bPressed;
		m_bFirst = bFirst;
		m_bDisabled = bDisabled;
	}
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPScrollBar window

class BCGCBPRODLLEXPORT CBCGPScrollBar : public CScrollBar
{
	DECLARE_DYNAMIC(CBCGPScrollBar)

// Construction
public:
	CBCGPScrollBar();

	enum BCGPSB_STYLE
	{
		BCGP_SBSTYLE_FIRST                  = 0,
		BCGP_SBSTYLE_DEFAULT				= BCGP_SBSTYLE_FIRST,	// Windows default style,
		BCGP_SBSTYLE_VISUAL_MANAGER			= 1,	// Defined by currently activated Visual Manager
		BCGP_SBSTYLE_VISUAL_MANAGER_FRAME	= 2,	// Used in frame window,
		BCGP_SBSTYLE_OWNER_DRAW				= 3,	// Developer should handle BCGM_ON_DRAW_SCROLLBAR message,
		BCGP_SBSTYLE_LAST                   = BCGP_SBSTYLE_OWNER_DRAW
	};

// Attributes
public:
	void SetVisualStyle (BCGPSB_STYLE style, BOOL bIsRedaw = TRUE)
	{
		ASSERT (BCGP_SBSTYLE_FIRST <= style && style <= BCGP_SBSTYLE_LAST);
		m_Style = style;

		if (bIsRedaw && GetSafeHwnd () != NULL)
		{
			RedrawWindow ();
		}
	}

	BCGPSB_STYLE GetVisualStyle () const
	{
		return m_Style;
	}

	BOOL IsActive () const
	{
		return m_HitTest != BCGP_HT_NOWHERE;
	}

	BOOL	m_bAutoDestoyWindow;
	BOOL	m_bOnGlass;

protected:
	enum BCGPSB_HITTEST
	{
		BCGP_HT_NOWHERE       = 0,
		BCGP_HT_BUTTON_LEFT   = 1,
		BCGP_HT_BUTTON_UP     = BCGP_HT_BUTTON_LEFT,
		BCGP_HT_BUTTON_LTUP   = BCGP_HT_BUTTON_LEFT,
		BCGP_HT_BUTTON_RIGHT  = 2,
		BCGP_HT_BUTTON_DOWN   = BCGP_HT_BUTTON_RIGHT,
		BCGP_HT_BUTTON_RTDN   = BCGP_HT_BUTTON_RIGHT,
		BCGP_HT_THUMB         = 3,
		BCGP_HT_CLIENT_LEFT   = 4,
		BCGP_HT_CLIENT_UP     = BCGP_HT_CLIENT_LEFT,
		BCGP_HT_CLIENT_LTUP   = BCGP_HT_CLIENT_LEFT,
		BCGP_HT_CLIENT_RIGHT  = 5,
		BCGP_HT_CLIENT_DOWN   = BCGP_HT_CLIENT_RIGHT,
		BCGP_HT_CLIENT_RTDN   = BCGP_HT_CLIENT_RIGHT,
		BCGP_HT_CLIENT        = 6
	};

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPScrollBar)
	protected:
	virtual void PreSubclassWindow();
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	virtual BOOL PreTranslateMessage (MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBCGPScrollBar();

	// Generated message map functions
protected:
	virtual BCGPSB_HITTEST HitTest (const CPoint& point) const;

	//{{AFX_MSG(CBCGPScrollBar)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPaint();
	afx_msg void OnCancelMode();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg LRESULT OnMouseLeave(WPARAM,LPARAM);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnNcDestroy();
	//}}AFX_MSG
	afx_msg LRESULT OnBCGSetControlAero (WPARAM, LPARAM);
	afx_msg LRESULT OnPrintClient(WPARAM wp, LPARAM lp);
	DECLARE_MESSAGE_MAP()

	virtual void DoPaint (CDC* pDC);

	virtual void DoPaintThumb (CDC* pDC, CRect rect, BOOL bHorz,
								   BOOL bHighlighted, BOOL bPressed, BOOL bDisabled)
	{
		if (m_Style == BCGP_SBSTYLE_OWNER_DRAW)
		{
			BCGPSCROLLBAR_DRAW_INFO info (pDC, this, BCGP_SCROLLBAR_THUMB, rect, 
				bHorz, bHighlighted, bPressed, FALSE, bDisabled);

			ASSERT_VALID (GetOwner ());

			GetOwner ()->SendMessage (BCGM_ON_DRAW_SCROLLBAR, (WPARAM) GetDlgCtrlID (), (LPARAM) &info);
			return;
		}

		CBCGPVisualManager::GetInstance ()->OnScrollBarDrawThumb (pDC, this, rect, 
			bHorz, bHighlighted, bPressed, bDisabled);
	}

	virtual void DoPaintButton (CDC* pDC, CRect rect, BOOL bHorz,
								BOOL bHighlighted, BOOL bPressed, BOOL bFirst, BOOL bDisabled)
	{
		if (m_Style == BCGP_SBSTYLE_OWNER_DRAW)
		{
			BCGPSCROLLBAR_DRAW_INFO info (pDC, this, BCGP_SCROLLBAR_BUTTON, rect, 
				bHorz, bHighlighted, bPressed, bFirst, bDisabled);

			ASSERT_VALID (GetOwner ());

			GetOwner ()->SendMessage (BCGM_ON_DRAW_SCROLLBAR, (WPARAM) GetDlgCtrlID (), (LPARAM) &info);
			return;
		}

		if (bHorz && (GetExStyle() & WS_EX_LAYOUTRTL))
		{
			bFirst = !bFirst;
		}

		CBCGPVisualManager::GetInstance ()->OnScrollBarDrawButton (pDC, this, rect, bHorz, 
								bHighlighted, bPressed, bFirst, bDisabled);
	}

	virtual void DoPaintBackground (CDC* pDC, CRect rect, BOOL bHorz, 
									BOOL bHighlighted, BOOL bPressed, BOOL bFirst, BOOL bDisabled)
	{
		if (m_Style == BCGP_SBSTYLE_OWNER_DRAW)
		{
			BCGPSCROLLBAR_DRAW_INFO info (pDC, this, BCGP_SCROLLBAR_BACKGROUND, rect, 
				bHorz, bHighlighted, bPressed, bFirst, bDisabled);

			ASSERT_VALID (GetOwner ());

			GetOwner ()->SendMessage (BCGM_ON_DRAW_SCROLLBAR, (WPARAM) GetDlgCtrlID (), (LPARAM) &info);
			return;
		}

		CBCGPVisualManager::GetInstance ()->OnScrollBarFillBackground (pDC, this, rect, bHorz, 
								bHighlighted, bPressed, bFirst, bDisabled);
	}

	virtual void Process ();

	CRect GetThumbRect (const CRect& rect, BOOL bHorz) const;
	int   PosFromThumb (int thumb, const CSize& szClamp);

protected:
	BCGPSB_STYLE	m_Style;
	BOOL			m_bTracked;
	BCGPSB_HITTEST	m_HitTest;
	BCGPSB_HITTEST	m_HitPressed;
	CWnd*			m_pLastCapture;
	
	DWORD			m_dwDisabledArrows;

	UINT_PTR		m_TimerDown;
	UINT_PTR		m_Timer;

	CPoint			m_ptTrackStart;
	CRect			m_rectTrackThumb;
	CRect			m_rectTrackThumbStart;
	int				m_TrackPos;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPSCROLLBAR_H__A96CA752_C9F3_4FEB_8D6F_9057D17337A0__INCLUDED_)
