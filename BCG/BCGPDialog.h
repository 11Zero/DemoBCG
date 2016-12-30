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


#if !defined(AFX_BCGPDIALOG_H__09800B11_4F60_11D4_AB0C_000000000000__INCLUDED_)
#define AFX_BCGPDIALOG_H__09800B11_4F60_11D4_AB0C_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"
#include "bcgpdlgimpl.h"

class CBCGPLocalResource;
class CBCGPToolBarImages;

/////////////////////////////////////////////////////////////////////////////
// CBCGPDialog dialog

/// <summary>
/// The CBCGPDialog class extends the functionality of the CDialog class. It allows to define a background image or color for the dialog box window and handles all the palette an transparency issues.
/// To take advantage of CBCGPDialog features you just need to derive your dialog class from the CBCGPDialog class instead of CDialog class.
/// When you call SetBackgroundImage method and pass HBITMAP as the image handle, the CBCGPDialog object will take ownership over the image and delete it only if the m_bAutoDestroyBmp flag is TRUE. The current background image will be deleted if you call the SetBackgroundImage again or by OnDestroy message handler. Images loaded from the resource file  are always destroyed automatically.</summary>
/// <remarks></remarks>

class BCGCBPRODLLEXPORT CBCGPDialog : public CDialog
{
	friend class CBCGPPopupMenu;
	friend class CBCGPDropDownList;
	friend class CBCGPContextMenuManager;
	friend class CBCGPRibbonBackstageViewItemForm;
	friend class CBCGPDlgImpl;

	DECLARE_DYNAMIC(CBCGPDialog)

// Construction
public:
	/// <summary>
	/// Constructs the object</summary>
	CBCGPDialog();

	/// <summary>
	/// Constructs the object</summary>
	/// <param name="nIDTemplate">Specifies the dialog template resource ID.</param>
	/// <param name="pParentWnd">A pointer to the parent window</param>
	CBCGPDialog(UINT nIDTemplate, CWnd* pParentWnd = NULL);

	/// <summary>
	/// Constructs the object</summary>
	/// <param name="lpszTemplateName">Specifies a dialog template resource name .</param>
	/// <param name="pParentWnd">A pointer to the parent window</param>
	CBCGPDialog(LPCTSTR lpszTemplateName, CWnd* pParentWnd = NULL);

protected:
	void CommonConstruct ();

// Attributes:
public:
	// Visualization:

	/// <summary>
	/// Automatically subclasses all hosted controls in order to take advantage of Visual Manager themes and the "Aero Glass" technology.</summary>
	/// <param name="bEnable">TRUE - enables Visual Manager theme support for hosted controls. FALSE - disables Visual Manager theme support for hosted controls.</param>
	/// <param name="bNCArea">If TRUE, the non-client area of dialog will be displayed using the current Visual Style</param>
	/// <param name="plstNonSubclassedItems">List of controls that shouldn't be subclassed.</param>
	/// <remarks>
	/// Call this function with bEnable TRUE to automatically subclass all hosted controls in order to take advantage of Visual Manager themes and the "Aero Glass" technology.</remarks>
	void EnableVisualManagerStyle (BOOL bEnable = TRUE, BOOL bNCArea = FALSE, const CList<UINT,UINT>* plstNonSubclassedItems = NULL);

	BOOL IsVisualManagerStyle () const
	{
		return m_Impl.m_bVisualManagerStyle;
	}

	BOOL IsVisualManagerNCArea () const
	{
		return m_Impl.m_bVisualManagerNCArea;
	}

	BOOL IsBackstageMode() const
	{
		return m_Impl.m_bBackstageMode;
	}

	enum BackgroundLocation
	{
		BACKGR_TILE,
		BACKGR_TOPLEFT,
		BACKGR_TOPRIGHT,
		BACKGR_BOTTOMLEFT,
		BACKGR_BOTTOMRIGHT,
	};

	/// <summary>
	/// This method has effect only under Windows Vista with the Aero theme enabled.
	/// Call EnableAero to display "Aero Glass" either on the whole dialog, or on some edges. 
	/// To enable glass for the whole dialog, just set _BCGPMARGINS::cyTopHeight to the height of the client area.See the SkinnedDialog sample for more info.</summary>
	/// <param name="margins">Specifies the margins for the "glass" areas.</param>
	BOOL EnableAero (BCGPMARGINS& margins);

	/// <summary></summary>
	/// <param name="margins"></param>
	void GetAeroMargins (BCGPMARGINS& margins) const;

	/// <summary></summary>
	BOOL HasAeroMargins () const
	{
		return m_Impl.HasAeroMargins ();
	}

	// Allow to drag the dialog box by clicking inside the client area
	void EnableDragClientArea(BOOL bEnable = TRUE);
	BOOL IsDragClientAreaEnabled() const
	{
		return m_Impl.IsDragClientAreaEnabled();
	}

	// Layout:
	
	/// <summary>
	/// Call this method to enable or disable layout management for a dialog box.
	/// If you specify a custom layout manager (pRTC), it must be derived from CBCGPControlsLayout or one of its descendants.</summary>
	/// <param name="bEnable">TRUE - enable layout management, FALSE - disable layout management.</param>
	/// <param name="pRTC">A pointer to a runtime class information of custom layout manager.</param>
	/// <param name="bResizeBox">TRUE - display resize box; FALSE - don't display resize box.</param>
	void EnableLayout(BOOL bEnable = TRUE, CRuntimeClass* pRTC = NULL, BOOL bResizeBox = TRUE);
	BOOL IsLayoutEnabled() const
	{
		return m_Impl.IsLayoutEnabled();
	}

	CBCGPControlsLayout* GetLayout()
	{
		return m_Impl.m_pLayout;
	}

	virtual void AdjustControlsLayout()
	{
		m_Impl.AdjustControlsLayout();
	}

	void UpdateCaptionButtons ()
	{
		m_Impl.UpdateCaptionButtons();
	}

	void EnableLoadWindowPlacement(BOOL bEnable = TRUE)
	{
		m_Impl.m_bLoadWindowPlacement = bEnable;
	}

	BOOL IsWindowPlacementEnabled() const
	{
		return m_Impl.m_bLoadWindowPlacement;
	}

	void SetWhiteBackground(BOOL bSet = TRUE);
	BOOL IsWhiteBackground() const
	{
		return m_Impl.m_bIsWhiteBackground;
	}

protected:
	HBITMAP				m_hBkgrBitmap;
	CSize				m_sizeBkgrBitmap;
	CBrush				m_brBkgr;
	BackgroundLocation	m_BkgrLocation;
	BOOL				m_bAutoDestroyBmp;
	BOOL				m_bWasMaximized;
	CBCGPDlgImpl		m_Impl;
	BOOL				m_bIsLocal;
	CBCGPLocalResource*	m_pLocaRes;
	CRect				m_rectBackstageWatermark;
	CBCGPToolBarImages*	m_pBackstageWatermark;

// Operations:
public:
	void SetBackgroundColor (COLORREF color, BOOL bRepaint = TRUE);
	void SetBackgroundImage (HBITMAP hBitmap, 
							BackgroundLocation location = BACKGR_TILE,
							BOOL bAutoDestroy = TRUE,
							BOOL bRepaint = TRUE);
	BOOL SetBackgroundImage (UINT uiBmpResId,
							BackgroundLocation location = BACKGR_TILE,
							BOOL bRepaint = TRUE);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPDialog)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual INT_PTR DoModal();
	protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL
	virtual void PreInitDialog();

public:
	BOOL Create(UINT nIDTemplate, CWnd* pParentWnd = NULL);
	BOOL Create(LPCTSTR lpszTemplateName, CWnd* pParentWnd = NULL);

	virtual void OnOK()
	{
		if (!IsBackstageMode())
		{
			CDialog::OnOK();
		}
	}

	virtual void OnCancel()
	{
		if (!IsBackstageMode())
		{
			CDialog::OnCancel();
		}
		else
		{
			CWnd* pParent = GetParent();
			if (pParent->GetSafeHwnd() != NULL)
			{
				pParent->SendMessage(WM_CLOSE);
			}
		}
	}

	virtual BOOL OnSetPlacement(WINDOWPLACEMENT& wp);

public:
	virtual void OnDrawBackstageWatermark(CDC* /*pDC*/, CRect /*rect*/)	{}

// Implementation
protected:

	//{{AFX_MSG(CBCGPDialog)
	afx_msg void OnActivate(UINT nState, CWnd *pWndOther, BOOL bMinimized);
	afx_msg BOOL OnNcActivate(BOOL bActive);
	afx_msg void OnEnable(BOOL bEnable);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnDestroy();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnSysColorChange();
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnNcPaint();
	afx_msg void OnNcMouseMove(UINT nHitTest, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg BCGNcHitTestType OnNcHitTest(CPoint point);
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnWindowPosChanged(WINDOWPOS FAR* lpwndpos);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	//}}AFX_MSG
	afx_msg LRESULT OnDWMCompositionChanged(WPARAM,LPARAM);
	afx_msg LRESULT OnChangeVisualManager (WPARAM, LPARAM);
	afx_msg LRESULT OnSetText(WPARAM, LPARAM);
	afx_msg LRESULT OnPowerBroadcast(WPARAM wp, LPARAM);
	DECLARE_MESSAGE_MAP()

	virtual void SetActiveMenu (CBCGPPopupMenu* pMenu);
	
	void EnableBackstageMode()
	{
		m_Impl.EnableBackstageMode();
	}
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPDIALOG_H__09800B11_4F60_11D4_AB0C_000000000000__INCLUDED_)
