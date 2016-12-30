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
// BCGPToolbarSpinEditBoxButton.h: interface for the CBCGPToolbarSpinEditBoxButton class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPTOOLBARSPINEDITBOXBUTTON_H__61779C5E_F035_4809_9C3C_AE8BD2204985__INCLUDED_)
#define AFX_BCGPTOOLBARSPINEDITBOXBUTTON_H__61779C5E_F035_4809_9C3C_AE8BD2204985__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGPToolbarEditBoxButton.h"
#include "BCGPSpinButtonCtrl.h"

class BCGCBPRODLLEXPORT CBCGPToolbarSpinEditBoxButton : public CBCGPToolbarEditBoxButton  
{
	DECLARE_SERIAL(CBCGPToolbarSpinEditBoxButton)

// Construction
public:
	CBCGPToolbarSpinEditBoxButton();
	CBCGPToolbarSpinEditBoxButton(UINT uiID, int iImage, DWORD dwStyle = ES_AUTOHSCROLL, int iWidth = 0);
	virtual ~CBCGPToolbarSpinEditBoxButton();

// Operations
public:
	void SetRange (int nMin, int nMax);
	void GetRange (int& nMin, int& nMax);

	CSpinButtonCtrl* GetSpinControl() {  return &m_wndSpin;  }

protected:
	void Init ();

// Attributes
protected:
   CBCGPSpinButtonCtrl	m_wndSpin;
   int					m_nMin;
   int					m_nMax;

// Overrides
protected:
	virtual CEdit* CreateEdit (CWnd* pWndParent, const CRect& rect);
	virtual void OnMove ();
	virtual void GetEditBorder (CRect& rectBorder);
	virtual void CopyFrom (const CBCGPToolbarButton& src);
	virtual void Serialize (CArchive& ar);
	virtual BOOL OnUpdateToolTip (CWnd* pWndParent, int iButtonIndex,
		CToolTipCtrl& wndToolTip, CString& str);
	virtual void OnShowEditbox (BOOL bShow);

public:
	virtual BOOL SetACCData (CWnd* pParent, CBCGPAccessibilityData& data);
};

#endif // !defined(AFX_BCGPTOOLBARSPINEDITBOXBUTTON_H__61779C5E_F035_4809_9C3C_AE8BD2204985__INCLUDED_)
