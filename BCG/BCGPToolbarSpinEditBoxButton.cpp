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
// BCGPToolbarSpinEditBoxButton.cpp: implementation of the CBCGPToolbarSpinEditBoxButton class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "bcgcbpro.h"
#include "BCGPToolbarSpinEditBoxButton.h"
#include "BCGPAccessibility.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_SERIAL(CBCGPToolbarSpinEditBoxButton, CBCGPToolbarEditBoxButton, 1)

CBCGPToolbarSpinEditBoxButton::CBCGPToolbarSpinEditBoxButton()
{
	Init ();
}
//***************************************************************************************
CBCGPToolbarSpinEditBoxButton::CBCGPToolbarSpinEditBoxButton (UINT uiId,
			int iImage,
			DWORD dwStyle,
			int iWidth) :
			CBCGPToolbarEditBoxButton (uiId, iImage, dwStyle, iWidth)
{
	Init ();
}
//***************************************************************************************
void CBCGPToolbarSpinEditBoxButton::Init ()
{
	m_nMin = INT_MIN;
	m_nMax = INT_MAX;
}
//****************************************************************************************
CBCGPToolbarSpinEditBoxButton::~CBCGPToolbarSpinEditBoxButton()
{
	if (m_wndSpin.GetSafeHwnd () != NULL)
	{
		m_wndSpin.DestroyWindow ();
	}
}
//***************************************************************************************
CEdit* CBCGPToolbarSpinEditBoxButton::CreateEdit(CWnd* pWndParent, const CRect& rect)
{
   CEdit *pEdit = CBCGPToolbarEditBoxButton::CreateEdit(pWndParent,rect);
   if (pEdit == NULL)
   {
	   return NULL;
   }

	if (!m_wndSpin.Create(
		WS_CHILD | WS_VISIBLE | UDS_ALIGNRIGHT | UDS_ARROWKEYS | UDS_SETBUDDYINT,
		  rect, pWndParent, m_nID))
		  return NULL;

   m_wndSpin.SetBuddy (pEdit);
   m_wndSpin.SetRange32 (m_nMin, m_nMax);

   return pEdit;
}
//**************************************************************************************
void CBCGPToolbarSpinEditBoxButton::OnMove ()
{
	CBCGPToolbarEditBoxButton::OnMove ();

	if (m_pWndEdit->GetSafeHwnd () != NULL)
	{
		m_wndSpin.SetBuddy (m_pWndEdit);
	}	
}
//**************************************************************************************
void CBCGPToolbarSpinEditBoxButton::GetEditBorder (CRect& rectBorder)
{
	ASSERT (m_pWndEdit->GetSafeHwnd () != NULL);

	m_pWndEdit->GetWindowRect (rectBorder);
	m_pWndEdit->GetParent ()->ScreenToClient (rectBorder);

	CRect rectSpin;
	m_wndSpin.GetWindowRect (rectSpin);
	m_wndSpin.GetParent ()->ScreenToClient (rectSpin);

	rectBorder.right = rectSpin.right;

	rectBorder.InflateRect (1, 1);
}
//**************************************************************************************
void CBCGPToolbarSpinEditBoxButton::CopyFrom (const CBCGPToolbarButton& s)
{
	CBCGPToolbarEditBoxButton::CopyFrom (s);

	const CBCGPToolbarSpinEditBoxButton& src = (const CBCGPToolbarSpinEditBoxButton&) s;

	m_nMin = src.m_nMin;
	m_nMax = src.m_nMax;
}
//**************************************************************************************
void CBCGPToolbarSpinEditBoxButton::Serialize (CArchive& ar)
{
	CBCGPToolbarEditBoxButton::Serialize (ar);

	if (ar.IsLoading ())
	{
		ar >> m_nMin;
		ar >> m_nMax;
	}
	else
	{
		ar << m_nMin;
		ar << m_nMax;
	}
}
//***************************************************************************************
void CBCGPToolbarSpinEditBoxButton::SetRange (int nMin, int nMax)
{
	ASSERT_VALID (this);

	m_nMin = nMin;
	m_nMax = nMax;

	if (m_wndSpin.GetSafeHwnd () != NULL)
	{
		m_wndSpin.SetRange32 (nMin, nMax);
	}
}
//***************************************************************************************
void CBCGPToolbarSpinEditBoxButton::GetRange (int& nMin, int& nMax)
{
	ASSERT_VALID (this);

	nMin = m_nMin;
	nMax = m_nMax;
}
//********************************************************************************
BOOL CBCGPToolbarSpinEditBoxButton::OnUpdateToolTip (CWnd* /*pWndParent*/, int /*iButtonIndex*/,
													 CToolTipCtrl& wndToolTip,
													 CString& strTipText)
{
	CEdit* pEdit = GetEditBox ();
	CSpinButtonCtrl* pSpin = GetSpinControl ();

	if ((pEdit != NULL) && (::IsWindow (pEdit->GetSafeHwnd ())))
	{
		CString strTips;

		if (OnGetCustomToolTipText (strTips))
		{
			wndToolTip.AddTool (pEdit, strTips, NULL, 0);
			wndToolTip.AddTool (pSpin, strTips, NULL, 0);
		}
		else
		{
			wndToolTip.AddTool (pEdit, strTipText, NULL, 0);
			wndToolTip.AddTool (pSpin, strTipText, NULL, 0);
		}

		return TRUE;
	}

	return FALSE;
}
//********************************************************************************
void CBCGPToolbarSpinEditBoxButton::OnShowEditbox (BOOL bShow)
{
	if (m_wndSpin.GetSafeHwnd () != NULL)
	{
		m_wndSpin.ShowWindow (bShow ? SW_SHOWNOACTIVATE : SW_HIDE);
	}
}
//**********************************************************************************
BOOL CBCGPToolbarSpinEditBoxButton::SetACCData (CWnd* pParent, CBCGPAccessibilityData& data)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pParent);

	if (!CBCGPToolbarEditBoxButton::SetACCData (pParent, data))
	{
		return FALSE;
	}

	data.m_strAccValue = m_strText;
	return TRUE;
}