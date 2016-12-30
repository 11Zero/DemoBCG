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
// BCGPDlgImpl.h: interface for the CBCGPDlgImpl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPDLGIMPL_H__18772215_4E74_4900_82E4_288CA46AB7E0__INCLUDED_)
#define AFX_BCGPDLGIMPL_H__18772215_4E74_4900_82E4_288CA46AB7E0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"
#include "BCGGlobals.h"
#include "BCGPVisualManager.h"
#include "BCGPLayout.h"

#if (!defined _BCGSUITE_) && (!defined _BCGSUITE_INC_)
#include "BCGPShadowManager.h"
#endif

class CBCGPPopupMenu;
class CBCGPFrameCaptionButton;

class BCGCBPRODLLEXPORT CBCGPDlgImpl  
{
	friend class CBCGPDialog;
	friend class CBCGPPropertyPage;
	friend class CBCGPPropertySheet;
	friend class CBCGPDialogBar;
	friend class CBCGPFormView;
	friend class CBCGPShadowManager;
	friend class CBCGPProgressDlg;

protected:
	CBCGPDlgImpl(CWnd& dlg);
	virtual ~CBCGPDlgImpl();

	static LRESULT CALLBACK BCGDlgMouseProc (int nCode, WPARAM wParam, LPARAM lParam);

	void SetActiveMenu (CBCGPPopupMenu* pMenu);

	BOOL ProcessMouseClick (POINT pt);
	BOOL ProcessMouseMove (POINT pt);

	BOOL PreTranslateMessage(MSG* pMsg);
	BOOL OnCommand (WPARAM wParam, LPARAM lParam);
	void OnNcActivate (BOOL& bActive);
	void OnActivate(UINT nState, CWnd* pWndOther);

	void EnableVisualManagerStyle (BOOL bEnable, BOOL bNCArea = FALSE, const CList<UINT,UINT>* plstNonSubclassedItems = NULL);

	void OnDestroy ();
	void OnDWMCompositionChanged ();

	BOOL EnableAero (BCGPMARGINS& margins);
	void GetAeroMargins (BCGPMARGINS& margins) const;
	BOOL HasAeroMargins () const;
	void ClearAeroAreas (CDC* pDC);

	HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

	BOOL OnNcPaint();
	BOOL OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp);
	void UpdateCaption ();
	void UpdateCaptionButtons ();
	UINT OnNcHitTest (CPoint point);
	void OnNcMouseMove(UINT nHitTest, CPoint point);
	void OnLButtonDown(CPoint point);
	void OnLButtonUp(CPoint point);
	void OnMouseMove(CPoint point);
	CBCGPFrameCaptionButton* GetSysButton (UINT nHit);

	void SetHighlightedSysButton (UINT nHitTest);
	void OnTrackCaptionButtons (CPoint point);
	void StopCaptionButtonsTracking ();
	void RedrawCaptionButton (CBCGPFrameCaptionButton* pBtn);

	BOOL IsOwnerDrawCaption ()
	{
		if (m_Dlg.GetSafeHwnd() != NULL && m_Dlg.GetMenu() != NULL)
		{
			return FALSE;
		}

#if (!defined _BCGSUITE_) && (!defined _BCGSUITE_INC_)
		return m_bVisualManagerStyle && m_bVisualManagerNCArea && CBCGPVisualManager::GetInstance ()->IsOwnerDrawCaption ();
#else
		return m_bVisualManagerStyle && m_bVisualManagerNCArea && CMFCVisualManager::GetInstance ()->IsOwnerDrawCaption ();
#endif
	}

	CRect GetCaptionRect ();
	void OnChangeVisualManager ();
	void OnWindowPosChanged(WINDOWPOS FAR* lpwndpos);
	void OnGetMinMaxInfo (MINMAXINFO FAR* lpMMI);

	int GetCaptionHeight ();

	void EnableLayout(BOOL bEnable = TRUE, CRuntimeClass* pRTC = NULL, BOOL bResizeBox = TRUE);
	BOOL IsLayoutEnabled() const
	{
		return m_pLayout != NULL;
	}

	void EnableDragClientArea(BOOL bEnable = TRUE);
	BOOL IsDragClientAreaEnabled() const
	{
		return m_bDragClientArea;
	}

	int OnCreate();

	void DrawResizeBox(CDC* pDC);
	void AdjustControlsLayout();

	void EnableBackstageMode();

	BOOL LoadPlacement(LPCTSTR lpszProfileName = NULL);
	BOOL SavePlacement(LPCTSTR lpszProfileName = NULL);
	BOOL SetPlacement(WINDOWPLACEMENT& wp);

	BOOL GetPlacementSection(LPCTSTR lpszProfileName, CString& strSection);

	CWnd&					m_Dlg;
	static HHOOK			m_hookMouse;
	static CBCGPDlgImpl*	m_pMenuDlgImpl;
	BOOL					m_bVisualManagerStyle;
	BOOL					m_bVisualManagerNCArea;
	CArray<CWnd*, CWnd*>	m_arSubclassedCtrls;
	BCGPMARGINS				m_AeroMargins;
	BOOL					m_bTransparentStaticCtrls;
	CObList					m_lstCaptionSysButtons;
	UINT					m_nHotSysButton;
	UINT					m_nHitSysButton;
	CRect					m_rectRedraw;
	BOOL					m_bWindowPosChanging;
	BOOL					m_bIsWindowRgn;
	BOOL					m_bHasBorder;
	BOOL					m_bHasCaption;
	BOOL					m_bIsWhiteBackground;
	BOOL					m_bDragClientArea;

	CList<UINT,UINT>		m_lstNonSubclassedItems;
	CStringList				m_lstNonSubclassedWndClasses;

	CBCGPControlsLayout*	m_pLayout;
	MINMAXINFO				m_LayoutMMI;
	CRect					m_rectResizeBox;
	BOOL					m_bResizeBox;
	BOOL					m_bBackstageMode;
	BOOL					m_bLoadWindowPlacement;
	BOOL					m_bWindowPlacementIsSet;

	void AdjustShadow(BOOL bActive);
	CBCGPShadowManager*	m_pShadow;
};

extern BCGCBPRODLLEXPORT UINT BCGM_ONSETCONTROLAERO;
extern BCGCBPRODLLEXPORT UINT BCGM_ONSETCONTROLVMMODE;
extern BCGCBPRODLLEXPORT UINT BCGM_ONSETCONTROLBACKSTAGEMODE;

#endif // !defined(AFX_BCGPDLGIMPL_H__18772215_4E74_4900_82E4_288CA46AB7E0__INCLUDED_)
