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
//
// BCGPRadialMenu.h : header file
//

#if !defined(AFX_BCGPRADIALMENU_H__C391C485_663A_4CDA_8310_00EE1B7A5261__INCLUDED_)
#define AFX_BCGPRADIALMENU_H__C391C485_663A_4CDA_8310_00EE1B7A5261__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"
#include "BCGPVisualCtrl.h"

class CBCGPRadialMenuCmdUI;

class BCGCBPRODLLEXPORT CBCGPRadialMenuItem : public CObject
{
	friend class CBCGPRadialMenuObject;
	friend class CBCGPRadialMenuCmdUI;
	friend class CBCGPRadialMenu;
	friend class CBCGPRotationObject;

public:
	CBCGPRadialMenuItem();
	CBCGPRadialMenuItem(UINT nID);

	virtual ~CBCGPRadialMenuItem();

protected:
	CBCGPRadialMenuItem(const CBCGPRadialMenuItem& src);

	void CommonInit();
	void UpdateToolTip();

protected:
	virtual void OnDrawIcon(CBCGPGraphicsManager* pGM, BOOL bIsCtrlDisabled, CBCGPImage& icons, CBCGPSize sizeIcon);
	virtual void OnUpdateCmdUI(CBCGPRadialMenuCmdUI* pCmdUI, CFrameWnd* pTarget, BOOL bDisableIfNoHndler);

protected:
	UINT					m_nID;		// Command ID
	HICON					m_hIcon;
	BOOL					m_bDestroyIcon;
	CString					m_strText;
	BOOL					m_bIsLargeIcon;
	int						m_nImageIndex;
	CString					m_strToolTip;
	CString					m_strDescription;
	CBCGPPoint				m_ptCenter;
	CBCGPComplexGeometry	m_Shape;
	BOOL					m_bIsDisabled;
	BOOL					m_bIsChecked;
	BOOL					m_bIsCenter;
};

class BCGCBPRODLLEXPORT CBCGPRadialMenuObject : public CBCGPBaseVisualObject  
{
	friend class CBCGPRadialMenu;

public:
	enum ColorTheme
	{
		BCGP_COLOR_THEME_FIRST          = 0,
		BCGP_COLOR_THEME_VISUAL_MANAGER = BCGP_COLOR_THEME_FIRST,
		BCGP_COLOR_THEME_BLUE           = 1,
		BCGP_COLOR_THEME_GREEN          = 2,
		BCGP_COLOR_THEME_SILVER         = 3,
		BCGP_COLOR_THEME_BLACK          = 4,
		BCGP_COLOR_THEME_LAST           = BCGP_COLOR_THEME_BLACK
	};

	CBCGPRadialMenuObject();
	virtual ~CBCGPRadialMenuObject();

	void SetImageList(UINT uiResID, int cx = 16);

	void AddCommand(UINT nCmdID);	// Icon will be automatically obtained from application toolbar/ribbon
	void AddCommand(UINT nCmdID, HICON hIcon);
	void AddCommand(UINT nCmdID, int nImageIndex);

	int GetClicked() const
	{
		return m_nLastClicked;
	}

	void EnableCenterButton(BOOL bEnable = TRUE);
	BOOL HasCenterButton() const
	{
		return m_bHasCenterButton;
	}

	void SetAutorepeatMode (int nTimeDelay = 500 /* ms */);
	BOOL IsAutorepeatCommandMode () const
	{
		return m_nAutoRepeatTimeDelay > 0;
	}

	void SetColorTheme(ColorTheme theme);

	void SetColors(
		const CBCGPColor& clrFill,
		const CBCGPColor& clrBorder,
		const CBCGPColor& clrHighlighted,
		const CBCGPColor& clrPressed,
		BOOL bIsFlat = FALSE);

	BOOL IsItemDisabled(int nIndex) const;

	void SetCloseOnInvoke(BOOL bIsCloseOnInvoke = TRUE)
	{
		m_bIsCloseOnInvoke = bIsCloseOnInvoke;
	}

protected:
	virtual int HitTestShape(const CBCGPPoint& pt);

	virtual void OnDraw(CBCGPGraphicsManager* pGM, const CBCGPRect& rectClip, DWORD dwFlags = BCGP_DRAW_STATIC | BCGP_DRAW_DYNAMIC);
	virtual void OnMouseMove(const CBCGPPoint& pt);
	virtual void OnMouseLeave();
	virtual BOOL OnMouseDown(int nButton, const CBCGPPoint& pt);
	virtual void OnMouseUp(int nButton, const CBCGPPoint& pt);
	virtual void OnCancelMode();
	virtual BOOL OnGetToolTip(const CBCGPPoint& pt, CString& strToolTip, CString& strDescr);
	virtual BOOL OnKeyboardDown(UINT nChar, UINT nRepCnt, UINT nFlags);

	static VOID CALLBACK AutoRepeatTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

	void StopAutorepeatTimer();
	virtual BOOL NotifyCommand();

	int GetShadowDepth() const;

	virtual HRESULT get_accRole(VARIANT varChild, VARIANT *pvarRole)
	{
		HRESULT hr = CBCGPBaseVisualObject::get_accRole(varChild, pvarRole);
		if (SUCCEEDED(hr))
		{
			pvarRole->lVal = ROLE_SYSTEM_MENUPOPUP;
		}
		
		return hr;
	}

protected:
	CArray<CBCGPRadialMenuItem*, CBCGPRadialMenuItem*> m_arItems;

	int						m_nHighlighted;
	int						m_nPressed;
	CBCGPImage				m_Icons;
	int						m_cxIcon;
	int						m_nLastClicked;
	CBCGPRadialMenu*		m_pCtrl;
	int						m_nAutoRepeatTimeDelay;
	int						m_nAutoRepeatTimerID;
	CBCGPBrush				m_brFill;
	CBCGPBrush				m_brBorder;
	CBCGPBrush				m_brFocusedBorder;
	CBCGPBrush				m_brPressed;
	CBCGPBrush				m_brHighlighted;
	CBCGPBrush				m_brShadow;
	BOOL					m_bIsFirstClick;
	BOOL					m_bHasCenterButton;
	BOOL					m_bIsCloseOnInvoke;
	int						m_nShadowDepth;
	BOOL					m_bIsVisualManagerTheme;

	static CMap<UINT,UINT,CBCGPRadialMenuObject*,CBCGPRadialMenuObject*>
		m_mapAutorepeat;
	static CCriticalSection g_cs;			// For multi-thread applications
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPRadialMenu window

class BCGCBPRODLLEXPORT CBCGPRadialMenu : public CBCGPVisualCtrl
{
	DECLARE_DYNCREATE(CBCGPRadialMenu)

	friend class CBCGPRadialMenuObject;

// Construction
public:
	CBCGPRadialMenu();

	BOOL CreatePopup(const CPoint& ptCenter,
		BYTE nTransparency = 255 /* from 0 to 255 */, 
		BOOL bIsTrackingMode = FALSE,
		CWnd* pWndOwner = NULL);

// Attributes
public:
	virtual CBCGPRadialMenuObject* GetRadialMenuObject()
	{
		if (m_pRadialMenuObject == NULL)
		{
			m_pRadialMenuObject = new CBCGPRadialMenuObject();
			m_pRadialMenuObject->m_pCtrl = this;
		}

		return m_pRadialMenuObject;
	}

protected:
	virtual CBCGPBaseVisualObject* GetVisualObject()
	{
		return GetRadialMenuObject();
	}

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPRadialMenu)
	//}}AFX_VIRTUAL

	virtual void OnUpdateCmdUI (CFrameWnd* pTarget, BOOL bDisableIfNoHndler);
	virtual BOOL IsCloseOnClick(CPoint ptScreen);

	virtual int GetIconSize();

// Implementation
public:
	virtual ~CBCGPRadialMenu();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPRadialMenu)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	afx_msg LRESULT OnIdleUpdateCmdUI(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnChangeVisualManager(WPARAM, LPARAM);
	DECLARE_MESSAGE_MAP()

	void SetRgn();

protected:
	CBCGPRadialMenuObject*	m_pRadialMenuObject;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPRADIALMENU_H__C391C485_663A_4CDA_8310_00EE1B7A5261__INCLUDED_)
