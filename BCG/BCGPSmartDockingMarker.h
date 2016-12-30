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
// BCGPSmartDockingMarker.h: interface for the CBCGPSmartDockingMarker class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPSMARTDOCKINGMARKER_H__320EA862_186F_4D9B_94C7_FF9D72E53492__INCLUDED_)
#define AFX_BCGPSMARTDOCKINGMARKER_H__320EA862_186F_4D9B_94C7_FF9D72E53492__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGPToolBarImages.h"

class CBCGPSmartDockingParams;

class CBCGPMarkerBmpWnd : public CWnd
{
// Construction
public:
	CBCGPMarkerBmpWnd ();
	virtual ~CBCGPMarkerBmpWnd ();

	BOOL Create (	LPRECT pWndRect, HBITMAP hbmpFace, HRGN hrgnShape, 
					CWnd* pwndOwner, BOOL bIsDefaultImage,
					BOOL bIsVert);

	BOOL Assign (HBITMAP hbmpFace, BOOL bRedraw);
	void Highlight (BOOL bSet);
	void UpdateLayeredWindow ();

// Attributes
protected:
	HBITMAP			m_hbmpFace;
	BOOL			m_bIsDefaultImage;
	BOOL			m_bIsHighlighted;
	COLORREF		m_clrFrame;
	BOOL			m_bIsVert;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPMarkerBmpWnd)
	//}}AFX_VIRTUAL

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPMarkerBmpWnd)
	afx_msg void OnPaint();
	afx_msg void OnClose();
	afx_msg BOOL OnEraseBkgnd (CDC* pDC);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

class CBCGPSmartDockingMarker : public CObject
{
public:
	enum SDMarkerPlace
	{
		sdNONE = -1,

		sdLEFT,
		sdRIGHT,
		sdTOP,
		sdBOTTOM,

		sdCLEFT,
		sdCRIGHT,
		sdCTOP,
		sdCBOTTOM,
		sdCMIDDLE,
	};

	CBCGPSmartDockingMarker ();
	virtual ~CBCGPSmartDockingMarker ();

	// create
	virtual void Create (SDMarkerPlace nSideNo, CWnd* pwndOwner);
	// destroy
	virtual void Destroy ();

	// show or hide marker
	virtual void Show (BOOL bShow = TRUE);

	// adjust marker position to the host rectangle (in screen coords)
	virtual void AdjustPos (CRect rcHost);

	// highlight the marker
	virtual void Highlight (BOOL bHiLite = TRUE);

	// discover if a point (in screen coords) lies within marker
	virtual BOOL IsPtIn(CPoint point) const;

protected:
	virtual void InitImages (CBCGPSmartDockingParams& params);

	SDMarkerPlace		m_nSideNo;
	CBCGPMarkerBmpWnd	m_wndBmp;

	CBCGPToolBarImages	m_Image;
	CBCGPToolBarImages	m_ImageHot;
	CRgn				m_Rgn;
	int					m_cx;
	int					m_cy;
	int					m_nPadding;

	BOOL				m_bHiLited;
	BOOL				m_bLayered;

	BOOL				m_bIsDefaultImage;

	static const int	m_nLeftOffsetX;
	static const int	m_nRightOffsetX;
	static const int	m_nTopOffsetY;
	static const int	m_nBottomOffsetY;

	DECLARE_DYNCREATE(CBCGPSmartDockingMarker)
};

class CBCGPSDCentralGroup;

class CBCGPCentralGroupWnd : public CWnd
{
	friend class CBCGPSDCentralGroup;

protected:
	CBCGPCentralGroupWnd ();

public:
	void Update ();

	afx_msg void OnPaint ();
	afx_msg void OnClose ();
	afx_msg BOOL OnEraseBkgnd (CDC* pDC);
	DECLARE_MESSAGE_MAP()

protected:
	CBCGPSDCentralGroup*	m_pCentralGroup;
	CBrush					m_brBaseBackground;
	CBrush					m_brBaseBorder;
};

class CBCGPSDCentralGroupMarker : public CBCGPSmartDockingMarker
{
public:
	CBCGPSDCentralGroupMarker ();
	virtual ~CBCGPSDCentralGroupMarker ();

	// create
	virtual void Create (SDMarkerPlace nSideNo, CWnd* pwndOwner);
	// destroy
	virtual void Destroy ();

	// show or hide marker
	virtual void Show (BOOL bShow = TRUE);

	// adjust marker position to the host rectangle (in screen coords)
	virtual void AdjustPos (CRect rcHost);

	// highlight the marker
	virtual void Highlight (BOOL bHiLite = TRUE);

	// discover if a point (in screen coords) lies within marker
	virtual BOOL IsPtIn(CPoint point) const;

	void SetVisible (BOOL bVisible = TRUE, BOOL bRedraw = TRUE);
	BOOL IsVisible () const
	{
		return m_bVisible;
	}

protected:
	virtual void Create (SDMarkerPlace nSideNo, CBCGPSDCentralGroup* pCentralGroup);
	virtual void DestroyImages ();

	virtual void Draw (CDC& dc, BOOL bAlpha = FALSE);

	CBCGPSDCentralGroup*	m_pCentralGroup;
	CPen					m_penFrame;
	CPen					m_penHighlight;
	COLORREF				m_clrFrame;

	int						m_nOffsetX;
	int						m_nOffsetY;
	BOOL					m_bVisible;

	friend class CBCGPSDCentralGroup;
};

class CBCGPSDCentralGroup : public CObject
{
public:
	CBCGPSDCentralGroup ();
	virtual ~CBCGPSDCentralGroup ();

	// create
	virtual void Create (CWnd* pwndOwner);
	// destroy
	virtual void Destroy ();

	// show or hide group
	virtual void Show (BOOL bShow = TRUE);

	// adjust group position to the host rectangle (in screen coords)
    // middle marker: nMiddleIsOn == -1 no change, 0 off, 1 on
	virtual BOOL AdjustPos (CRect rcHost, int nMiddleIsOn = -1);

	void ShowMarker (CBCGPSmartDockingMarker::SDMarkerPlace nMarkerNo, 
					BOOL bShow = TRUE, BOOL bRedraw = TRUE);

	// fetch markers' interfaces (0 - left, 1 - right, 2 - top, 3 - bottom, 4 - middle)
	virtual CBCGPSDCentralGroupMarker* GetMarker (CBCGPSmartDockingMarker::SDMarkerPlace nMarkerNo)
	{
		if (nMarkerNo < CBCGPSmartDockingMarker::sdCLEFT ||
			nMarkerNo > CBCGPSmartDockingMarker::sdCMIDDLE)
		{
			ASSERT (FALSE);
			return NULL;
		}

		return &m_arMarkers [nMarkerNo - CBCGPSmartDockingMarker::sdCLEFT];
	}

	void GetWindowRect (CRect& rect);

	DECLARE_DYNCREATE(CBCGPSDCentralGroup)

protected:
	CBCGPCentralGroupWnd m_Wnd;

	CBCGPSDCentralGroupMarker m_arMarkers [5];

	CBCGPToolBarImages	m_Image;
	CRgn				m_rgnBase;
	BOOL				m_bCreated;
    BOOL				m_bMiddleIsOn;
	BOOL				m_bLayered;

	friend class CBCGPCentralGroupWnd;
	friend class CBCGPSDCentralGroupMarker;
    friend class CBCGPSmartDockingManager;

	void DrawCentralGroup (CDC& dc, CBrush& brBaseBackground, CBrush& brBaseBorder, CRect rectClient);
};

// provide window class name for specified class style
template <DWORD WndStyle>
LPCTSTR GetSDWndClassName ()
{
	static CString strSDWndClassName;
	if (strSDWndClassName.IsEmpty ())
	{
		strSDWndClassName = AfxRegisterWndClass (WndStyle);
	}

	return strSDWndClassName;
}

#endif // !defined(AFX_BCGPSMARTDOCKINGMARKER_H__320EA862_186F_4D9B_94C7_FF9D72E53492__INCLUDED_)
