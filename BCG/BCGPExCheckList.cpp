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

// BCGPExCheckList.cpp : implementation file
//
#include "stdafx.h"
#include "afxpriv.h"
#include "BCGPExCheckList.h"
#include "BCGPVisualManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBCGPExCheckList

CBCGPExCheckList::CBCGPExCheckList()
{
}
//*******************************************************************************************
CBCGPExCheckList::~CBCGPExCheckList()
{
}

BEGIN_MESSAGE_MAP(CBCGPExCheckList, CCheckListBox)
	//{{AFX_MSG_MAP(CBCGPExCheckList)
	ON_WM_LBUTTONDOWN()
	ON_WM_KEYDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
	ON_MESSAGE(LB_ADDSTRING, OnLBAddString)
	ON_MESSAGE(LB_INSERTSTRING, OnLBInsertString)
	ON_MESSAGE(LB_RESETCONTENT, OnLBResetContent)
	ON_MESSAGE(LB_DELETESTRING, OnLBDeleteString)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGPExCheckList message handlers

void CBCGPExCheckList::OnLButtonDown(UINT nFlags, CPoint point)
{
	SetFocus();

	// determine where the click is
	BOOL bInCheck;
	int nIndex = CheckFromPoint(point, bInCheck);

	if (bInCheck && nIndex != LB_ERR && !IsCheckEnabled (nIndex))
	{
		MessageBeep ((UINT) -1);
		return;
	}
	
	CCheckListBox::OnLButtonDown(nFlags, point);
}
//*******************************************************************************************
void CBCGPExCheckList::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if (nChar == VK_SPACE)
	{
		int nIndex = GetCaretIndex();
		if (nIndex != LB_ERR && !IsCheckEnabled (nIndex))
		{
			MessageBeep ((UINT) -1);
			return;
		}
	}
	
	CCheckListBox::OnKeyDown(nChar, nRepCnt, nFlags);
}
//*******************************************************************************************
LRESULT CBCGPExCheckList::OnLBAddString(WPARAM wParam, LPARAM lParam)
{
	LRESULT lRes = CCheckListBox::OnLBAddString(wParam, lParam);
	OnNewString ((int) lRes);
	return lRes;
}
//*******************************************************************************************
LRESULT CBCGPExCheckList::OnLBInsertString(WPARAM wParam, LPARAM lParam)
{
	LRESULT lRes = CCheckListBox::OnLBInsertString(wParam, lParam);
	OnNewString ((int) lRes);
	return lRes;
}
//*******************************************************************************************
LRESULT CBCGPExCheckList::OnLBDeleteString(WPARAM wParam, LPARAM /*lParam*/)
{
	LRESULT lRes = Default ();
	if (lRes != LB_ERR)
	{
		m_arCheckData.RemoveAt ((int) wParam);
	}

	return lRes;
}
//*******************************************************************************************
LRESULT CBCGPExCheckList::OnLBResetContent(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	m_arCheckData.SetSize (0);
	return Default ();
}
//*******************************************************************************************
void CBCGPExCheckList::EnableCheck (int nIndex, BOOL bEnable)
{
	ASSERT (nIndex >= 0 && nIndex < m_arCheckData.GetSize ());
	m_arCheckData.SetAt (nIndex, bEnable);
}
//*******************************************************************************************
BOOL CBCGPExCheckList::IsCheckEnabled (int nIndex) const
{
	ASSERT (nIndex >= 0 && nIndex < m_arCheckData.GetSize ());
	return m_arCheckData.GetAt (nIndex);
}
//*******************************************************************************************
void CBCGPExCheckList::OnNewString (int iIndex)
{
	if (iIndex >= 0)
	{
		int iSize = GetCount ();
		m_arCheckData.SetSize (iSize);

		for (int i = iSize - 1; i > iIndex; i --)
		{
			m_arCheckData.SetAt (i, m_arCheckData.GetAt (i - 1));
		}
		
		m_arCheckData.SetAt (iIndex, TRUE);	// Enabled by default
	}
}
//****************************************************************************************
void CBCGPExCheckList::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	// determine where the click is
	BOOL bInCheck;
	int nIndex = CheckFromPoint(point, bInCheck);

	if (bInCheck && nIndex != LB_ERR && !IsCheckEnabled (nIndex))
	{
		MessageBeep ((UINT) -1);
		return;
	}
	
	CCheckListBox::OnLButtonDblClk(nFlags, point);
	GetParent()->SendMessage(WM_COMMAND,
                            MAKEWPARAM(GetDlgCtrlID(), CLBN_CHKCHANGE),
                            (LPARAM)m_hWnd);
}

void CBCGPExCheckList::PreSubclassWindow() 
{
	CCheckListBox::PreSubclassWindow();

#if _MSC_VER < 1300
	SetItemHeight(0, max(CalcMinimumItemHeight(), CBCGPVisualManager::GetInstance()->GetCheckRadioDefaultSize().cy + 2));
#endif
}

int CBCGPExCheckList::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CCheckListBox::OnCreate(lpCreateStruct) == -1)
		return -1;
	
#if _MSC_VER < 1300
	SetItemHeight(0, max(CalcMinimumItemHeight(), CBCGPVisualManager::GetInstance()->GetCheckRadioDefaultSize().cy));
#endif
	return 0;
}
