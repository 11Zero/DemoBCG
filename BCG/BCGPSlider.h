#if !defined(AFX_BCGPSLIDER_H__8F3BF585_F0C0_4FFF_8558_1BE7286D08A2__INCLUDED_)
#define AFX_BCGPSLIDER_H__8F3BF585_F0C0_4FFF_8558_1BE7286D08A2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

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
// BCGPSlider.h : header file
//

#include "BCGCBPro.h"
#include "BCGPBaseControlBar.h"

class CBCGPBarContainerManager;
class CBCGPControlBar;
class CBCGPDockingControlBar;
class CBCGPBarContainer;
class CBCGPTrackingWnd;

/////////////////////////////////////////////////////////////////////////////
// CBCGPSlider window

class BCGCBPRODLLEXPORT CBCGPSlider : public CBCGPBaseControlBar
{
	friend class CBCGPDockingControlBar;

	DECLARE_DYNCREATE(CBCGPSlider)
// Construction
public:
	CBCGPSlider ();
	CBCGPSlider (BOOL bDefaultSlider, CWnd* pParent = NULL);

	void Init (BOOL bDefaultSlider = FALSE, CWnd* pParent = NULL);

	enum SS_STYLE 
	{
		SS_HORZ = 0x1,
		SS_VERT = 0x2
	};

// Attributes
public:
	BOOL		IsHorizontal	() const {return m_dwSliderStyle & SS_HORZ;}
	int			GetWidth		() const  {return m_nWidth;}
	static int	GetDefaultWidth () {return m_nDefaultWidth;}
	BOOL		IsDefault		() const {return m_bDefaultSlider;}
	BOOL		IsAutoHideMode	() const {return m_bAutoHideMode;}

	DWORD		GetSliderStyle  () const {return m_dwSliderStyle;}

	virtual BOOL DoesAllowDynInsertBefore () const;

	static int m_nDefaultWidth;
	static CRuntimeClass* m_pContainerManagerRTC;
	static CRuntimeClass* m_pSliderRTC;
	
// Operations
public:

	void ShowWindow (int nCmdShow);

	virtual void MoveSlider			 (CPoint& ptOffset, BOOL bAdjustLayout = TRUE);
	virtual void AddControlBar		 (CBCGPDockingControlBar* pBar);

	virtual CBCGPDockingControlBar* AddRecentControlBar (CBCGPDockingControlBar* pBar);

	virtual BOOL InsertControlBar	 (CBCGPDockingControlBar* pBarToInsert, 
									  CBCGPDockingControlBar* pTargetBar, 
									  DWORD dwAlignment, 
									  LPCRECT lpRect = NULL);
	virtual BOOL ReplaceControlBar	 (CBCGPDockingControlBar* pBarToReplace, 
									  CBCGPDockingControlBar* pBarToReplaceWith);
	virtual void RemoveControlBar	 (CBCGPDockingControlBar* pBar);
	virtual void RepositionBars		 (CRect& rectNew, HDWP& hdwp);
	virtual BOOL AddContainer		 (CBCGPBarContainerManager& barContainerManager, BOOL bOuterEdge);
	virtual BOOL AddContainer		 (CBCGPDockingControlBar* pTargetBar, 
									  CBCGPBarContainerManager& barContainerManager, 
									  DWORD dwAlignment);

	virtual BOOL CheckVisibility ();

	virtual void OnShowControlBar (CBCGPDockingControlBar* pBar, BOOL bShow);

	virtual BOOL DoesContainFloatingBar ();
	virtual void CalcExpectedDockedRect (CWnd* pWndToDock, CPoint ptMouse, 
					CRect& rectResult, BOOL& bDrawTab, CBCGPDockingControlBar** ppTargetBar);

	void StoreRecentDockInfo (CBCGPDockingControlBar* pBar);
	void StoreRecentTabRelatedInfo (CBCGPDockingControlBar* pDockingBar, 
									CBCGPDockingControlBar* pTabbedBar);

	void SetContainerManager (CBCGPBarContainerManager* p) {m_pContainerManager = p;}
	void ReleaseEmptyContainers ();
	

	void SetAutoHideMode (BOOL bMode) {m_bAutoHideMode = bMode;}

	const CBCGPBaseControlBar* GetFirstBar () const;

	void Serialize (CArchive& ar);
	CBCGPDockingControlBar* FindTabbedBar (UINT nID);
	CBCGPBarContainer* FindContainer (CBCGPDockingControlBar* pBar, BOOL& bLeftBar);
	CRect GetRootContainerRect (); 

	virtual void NotifyAboutRelease ();

	void GetControlBars (CObList& lstBars);
	void GetSliders (CObList& lstSliders);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPSlider)
	public:
	virtual BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	virtual BOOL CreateEx(DWORD dwStyleEx, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBCGPSlider();

	virtual CSize CalcFixedLayout(BOOL bStretch, BOOL bHorz);
	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPSlider)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnDestroy();
	afx_msg void OnNcDestroy();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnCancelMode ();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
	void SetDefaultMode (BOOL bMode) {m_bDefaultSlider = bMode;}
	virtual void StopTracking (BOOL bMoveSlider);

protected:

	UINT	m_nID;
	DWORD	m_dwSliderStyle;
	int		m_nWidth;

	bool	m_bCaptured;
	CPoint  m_ptHotTotal;
	CPoint  m_ptHotDrag;

	BOOL	m_bDefaultSlider; // true if this slider is created on the "edge"
								   // of one or several containers	

	BOOL	m_bAutoHideMode;

	int		m_nMinOffset; 
	int		m_nMaxOffset; 
	int		m_nStep;

	CRect	m_rectLastDragRect;
	CRect	m_rectDragBounds;
	CBCGPTrackingWnd*			m_pWndTrack;

	CBCGPBarContainerManager*	m_pContainerManager;

	CWnd*						m_pParentWndForSerialize;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPSLIDER_H__8F3BF585_F0C0_4FFF_8558_1BE7286D08A2__INCLUDED_)
