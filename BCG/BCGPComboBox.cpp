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
// BCGPComboBox.cpp : implementation file
//

#include "stdafx.h"
#include "BCGPComboBox.h"
#include "BCGPDlgImpl.h"
#ifndef _BCGSUITE_
#include "BCGPToolBarImages.h"
#include "BCGPToolbarComboBoxButton.h"
#endif
#include "BCGPVisualManager.h"
#include "BCGPDrawManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBCGPComboBox

IMPLEMENT_DYNAMIC(CBCGPComboBox, CComboBox)

CBCGPComboBox::CBCGPComboBox()
{
	m_bVisualManagerStyle = FALSE;
	m_bOnGlass = FALSE;
	m_bIsDroppedDown = FALSE;
	m_bIsButtonHighlighted = FALSE;
	m_rectBtn.SetRectEmpty ();
	m_bTracked = FALSE;
	m_clrPrompt = (COLORREF)-1;
	m_clrErrorText = (COLORREF)-1;
	m_bDefaultPrintClient = FALSE;
}

CBCGPComboBox::~CBCGPComboBox()
{
}

BEGIN_MESSAGE_MAP(CBCGPComboBox, CComboBox)
	//{{AFX_MSG_MAP(CBCGPComboBox)
	ON_WM_NCPAINT()
	ON_WM_PAINT()
	ON_WM_MOUSEMOVE()
	ON_WM_CANCELMODE()
	ON_WM_LBUTTONDOWN()
	ON_WM_KILLFOCUS()
	ON_WM_CREATE()
	ON_WM_SETFOCUS()
	//}}AFX_MSG_MAP
	ON_CONTROL_REFLECT_EX(CBN_EDITUPDATE, OnEditupdate)
	ON_CONTROL_REFLECT_EX(CBN_SELCHANGE, OnSelchange)
	ON_CONTROL_REFLECT_EX(CBN_CLOSEUP, OnCloseup)
	ON_CONTROL_REFLECT_EX(CBN_DROPDOWN, OnDropdown)
	ON_REGISTERED_MESSAGE(BCGM_ONSETCONTROLVMMODE, OnBCGSetControlVMMode)
	ON_REGISTERED_MESSAGE(BCGM_ONSETCONTROLAERO, OnBCGSetControlAero)
	ON_MESSAGE(WM_SETTEXT, OnSetText)
	ON_MESSAGE(WM_PRINTCLIENT, OnPrintClient)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGPComboBox message handlers

LRESULT CBCGPComboBox::OnBCGSetControlVMMode (WPARAM wp, LPARAM)
{
	m_bVisualManagerStyle = (BOOL) wp;
	return 0;
}
//**************************************************************************
void CBCGPComboBox::SubclassEditBox()
{
	if (GetSafeHwnd() == NULL)
	{
		return;
	}

	if (m_wndEdit.GetSafeHwnd () == NULL && (GetStyle () & CBS_DROPDOWN))
	{
		CWnd* pWndChild = GetWindow (GW_CHILD);
		
		while (pWndChild != NULL)
		{
			ASSERT_VALID (pWndChild);
			
			if (CWnd::FromHandlePermanent (pWndChild->GetSafeHwnd ()) == NULL)
			{
				#define MAX_CLASS_NAME		255
				#define EDIT_CLASS			_T("Edit")
				
				TCHAR lpszClassName [MAX_CLASS_NAME + 1];
				
				::GetClassName (pWndChild->GetSafeHwnd (), lpszClassName, MAX_CLASS_NAME);
				CString strClass = lpszClassName;
				
				if (strClass == EDIT_CLASS)
				{
					m_wndEdit.SubclassWindow (pWndChild->GetSafeHwnd ());

					m_wndEdit.m_bOnGlass = m_bOnGlass;
					m_wndEdit.m_bVisualManagerStyle = m_bVisualManagerStyle;
					break;
				}
			}
			
			pWndChild = pWndChild->GetNextWindow ();
		}
	}
}
//**************************************************************************
LRESULT CBCGPComboBox::OnBCGSetControlAero (WPARAM wp, LPARAM)
{
	m_bOnGlass = (BOOL) wp;

	if (m_bOnGlass)
	{
		SubclassEditBox();
	}

	return 0;
}
//**************************************************************************
void CBCGPComboBox::OnNcPaint() 
{
#ifndef _BCGSUITE_
	if (globalData.bIsWindows9x)
	{
		Default();
	}
#endif
}
//**************************************************************************
void CBCGPComboBox::OnPaint() 
{
#ifndef _BCGSUITE_
	if (globalData.bIsWindows9x)
	{
		Default();
		return;
	}
#endif

	if ((GetStyle () & 0x0003L) == CBS_SIMPLE)
	{
		Default ();
		return;
	}

	BOOL bDrawPrompt = FALSE;

	if (!m_strPrompt.IsEmpty() || !m_strErrorMessage.IsEmpty())
	{
		BOOL bTextIsEmpty = GetWindowTextLength() == 0;

		if (m_wndEdit.GetSafeHwnd () != NULL)
		{
			if (!m_strErrorMessage.IsEmpty())
			{
				m_wndEdit.SetErrorMessage(m_strErrorMessage, m_clrErrorText);
			}
			else
			{
				m_wndEdit.SetPrompt(bTextIsEmpty ? m_strPrompt : _T(""), m_clrPrompt);
			}
		}
		else
		{
			bDrawPrompt = bTextIsEmpty || !m_strErrorMessage.IsEmpty();
		}
	}

	if (!m_bVisualManagerStyle && !m_bOnGlass && !bDrawPrompt)
	{
		Default ();
		return;
	}

	CPaintDC dc(this); // device context for painting
	OnDraw(&dc, bDrawPrompt);
}
//**************************************************************************
void CBCGPComboBox::OnDraw(CDC* pDCIn, BOOL bDrawPrompt)
{
	ASSERT_VALID(pDCIn);

	BYTE alpha = 0;
	if (m_bOnGlass)
	{
		alpha = 255;
	}

	CBCGPMemDC memDC(*pDCIn, this, alpha);
	CDC* pDC = &memDC.GetDC ();
	
	CRect rectClient;
	GetClientRect (rectClient);

	CBCGPDrawManager dm (*pDC);
	dm.DrawRect (rectClient, m_bVisualManagerStyle ? globalData.clrBarHilite : globalData.clrWindow, (COLORREF)-1);

	BOOL bDefaultDraw = TRUE;

	if (bDrawPrompt)
	{
		COLORREF clrText = !m_strErrorMessage.IsEmpty() ? m_clrErrorText : m_clrPrompt;
		
		if (clrText == (COLORREF)-1)
		{
#ifndef _BCGSUITE_
			clrText = m_bVisualManagerStyle ? CBCGPVisualManager::GetInstance ()->GetToolbarEditPromptColor() : globalData.clrPrompt;
#else
			clrText = globalData.clrGrayedText;
#endif
		}
		
		pDC->SetTextColor(clrText);
		pDC->SetBkMode(TRANSPARENT);
		
		CFont* pOldFont = pDC->SelectObject (GetFont());
		
		CRect rectText;
		GetClientRect(rectText);

		rectText.left += 4;
		
		if ((GetStyle () & WS_BORDER) != 0 || (GetExStyle () & WS_EX_CLIENTEDGE) != 0)
		{
			rectText.DeflateRect (1, 1);
		}
		
		UINT nFormat = DT_LEFT | DT_SINGLELINE | DT_VCENTER;
		const CString& str = !m_strErrorMessage.IsEmpty() ? m_strErrorMessage : m_strPrompt;
		
		if (m_bOnGlass)
		{
			CBCGPVisualManager::GetInstance ()->DrawTextOnGlass(pDC, str, rectText, nFormat, 0, clrText);
		}
		else
		{
			pDC->DrawText(str, rectText, nFormat);
		}
		
		pDC->SelectObject (pOldFont);
	}
	else
	{
	#ifndef _BCGSUITE_
		if ((GetStyle() & CBS_OWNERDRAWFIXED) == 0 && (GetStyle() & CBS_OWNERDRAWVARIABLE) == 0)
		{
			bDefaultDraw = !CBCGPVisualManager::GetInstance ()->OnDrawComboBoxText(pDC, this);

			if (m_bVisualManagerStyle && bDefaultDraw)
			{
				CString strText;
				GetWindowText(strText);
				
				CRect rect;
				GetClientRect(rect);
				
				BOOL bIsFocused = GetSafeHwnd() == CWnd::GetFocus()->GetSafeHwnd();
				COLORREF clrText = CBCGPVisualManager::GetInstance ()->OnFillComboBoxItem(pDC, this, GetCurSel(), rect, bIsFocused, bIsFocused);
				
				if (!IsWindowEnabled())
				{
					clrText = CBCGPVisualManager::GetInstance ()->GetToolbarDisabledTextColor();
				}
				
				rect.left += 4;
				
				CFont* pOldFont = pDC->SelectObject(GetFont());
				ASSERT_VALID(pOldFont);
				
				int nOldBkMode = pDC->SetBkMode(TRANSPARENT);
				COLORREF clrTextOld = pDC->SetTextColor(clrText);
				
				const int cxDropDown = ::GetSystemMetrics (SM_CXVSCROLL);
				rect.right -= cxDropDown;

				UINT nFormat = DT_SINGLELINE | DT_VCENTER | DT_LEFT;
				
				if (m_bOnGlass)
				{
					CBCGPVisualManager::GetInstance ()->DrawTextOnGlass(pDC, strText, rect, nFormat, 0, clrText);
				}
				else
				{
					pDC->DrawText(strText, rect, nFormat);
				}

				pDC->SelectObject(pOldFont);
				pDC->SetBkMode(nOldBkMode);
				pDC->SetTextColor(clrTextOld);

				bDefaultDraw = FALSE;
			}
		}

	#endif

		if (bDefaultDraw)
		{
			m_bDefaultPrintClient = TRUE;
			SendMessage (WM_PRINTCLIENT, (WPARAM) pDC->GetSafeHdc (), (LPARAM) PRF_CLIENT);
			m_bDefaultPrintClient = FALSE;
		}
	}

	if ((GetStyle() & CBS_OWNERDRAWFIXED) != 0 || (GetStyle() & CBS_OWNERDRAWVARIABLE) != 0)
	{
		pDC->SelectClipRgn (NULL);
	}

	const int cxDropDown = ::GetSystemMetrics (SM_CXVSCROLL) + 4;

	m_rectBtn = rectClient;
	m_rectBtn.left = m_rectBtn.right - cxDropDown;

	m_rectBtn.DeflateRect (2, 2);

	CBCGPDrawOnGlass dog (m_bOnGlass);

	CBCGPToolbarComboBoxButton buttonDummy;
#ifndef _BCGSUITE_
	buttonDummy.m_bIsCtrl = TRUE;

	CBCGPVisualManager::GetInstance ()->OnDrawComboDropButton (
		pDC, m_rectBtn, !IsWindowEnabled (), m_bIsDroppedDown,
		m_bIsButtonHighlighted,
		&buttonDummy);

	if (bDefaultDraw)
	{
		dm.DrawRect (rectClient, (COLORREF)-1, globalData.clrBarShadow);
	}
	else
	{
		CBCGPVisualManager::GetInstance ()->OnDrawControlBorder (pDC, rectClient, this, m_bOnGlass);
	}

#else
	CMFCVisualManager::GetInstance ()->OnDrawComboDropButton (
		pDC, m_rectBtn, !IsWindowEnabled (), m_bIsDroppedDown,
		m_bIsButtonHighlighted,
		&buttonDummy);

	dm.DrawRect (rectClient, (COLORREF)-1, globalData.clrBarShadow);

#endif

	rectClient.DeflateRect (1, 1);
	dm.DrawRect (rectClient, (COLORREF)-1, m_bVisualManagerStyle ? globalData.clrBarHilite : globalData.clrWindow);
}
//**************************************************************************
BOOL CBCGPComboBox::OnCloseup() 
{
	m_bIsDroppedDown = FALSE;
	m_bIsButtonHighlighted = FALSE;

	RedrawWindow (NULL, NULL, RDW_INVALIDATE | RDW_FRAME | RDW_UPDATENOW | RDW_ALLCHILDREN);

	return FALSE;
}
//**************************************************************************
BOOL CBCGPComboBox::OnDropdown() 
{
	if (m_bTracked)
	{
		ReleaseCapture ();
		m_bTracked = FALSE;
	}

	m_bIsDroppedDown = TRUE;
	m_bIsButtonHighlighted = FALSE;
	RedrawWindow (NULL, NULL, RDW_INVALIDATE | RDW_FRAME | RDW_UPDATENOW | RDW_ALLCHILDREN);

	return FALSE;
}
//**************************************************************************
void CBCGPComboBox::OnMouseMove(UINT nFlags, CPoint point) 
{
	if ((nFlags & MK_LBUTTON) == 0)
	{
		BOOL bIsButtonHighlighted = m_bIsButtonHighlighted;
		m_bIsButtonHighlighted = m_rectBtn.PtInRect (point);

		if (bIsButtonHighlighted != m_bIsButtonHighlighted)
		{
			if (!m_bTracked)
			{
				if (m_bIsButtonHighlighted)
				{
					SetCapture ();
					m_bTracked = TRUE;
				}
			}
			else
			{
				if (!m_bIsButtonHighlighted)
				{
					ReleaseCapture ();
					m_bTracked = FALSE;
				}
			}
		
			RedrawWindow(m_rectBtn, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
		}
	}
	
	CComboBox::OnMouseMove(nFlags, point);
}
//*****************************************************************************************
void CBCGPComboBox::OnCancelMode() 
{
	CComboBox::OnCancelMode();
	
	if (m_bTracked)
	{
		ReleaseCapture ();
		m_bIsButtonHighlighted = FALSE;
		m_bTracked = FALSE;

		RedrawWindow (NULL, NULL, RDW_INVALIDATE | RDW_FRAME | RDW_UPDATENOW | RDW_ALLCHILDREN);
	}
}
//**************************************************************************
void CBCGPComboBox::OnLButtonDown(UINT nFlags, CPoint point) 
{
	if (m_bTracked)
	{
		ReleaseCapture ();
		m_bIsButtonHighlighted = FALSE;
		m_bTracked = FALSE;

		RedrawWindow (NULL, NULL, RDW_INVALIDATE | RDW_FRAME | RDW_UPDATENOW | RDW_ALLCHILDREN);
	}
	
	CComboBox::OnLButtonDown(nFlags, point);
}
//**************************************************************************
void CBCGPComboBox::OnKillFocus(CWnd* pNewWnd) 
{
	CComboBox::OnKillFocus(pNewWnd);
	
	BOOL bDrawPrompt = (!m_strPrompt.IsEmpty() && GetWindowTextLength() == 0) || !m_strErrorMessage.IsEmpty();
	if (bDrawPrompt)
	{
		m_bIsButtonHighlighted = FALSE;
	}

	if (m_bVisualManagerStyle || bDrawPrompt)
	{
		RedrawWindow (NULL, NULL, RDW_INVALIDATE | RDW_FRAME | RDW_UPDATENOW | RDW_ALLCHILDREN);
	}
}
//**************************************************************************
void CBCGPComboBox::SetPrompt(LPCTSTR lpszPrompt, COLORREF clrText, BOOL bRedraw)
{
	ASSERT_VALID (this);

	CString strOldPrompt = m_strPrompt;
	BOOL bColorWasChanged = m_clrPrompt != clrText;

	m_strPrompt = (lpszPrompt == NULL) ? _T("") : lpszPrompt;
	m_clrPrompt = clrText;
	
	if (!m_strPrompt.IsEmpty())
	{
		SubclassEditBox();
	}

	if (m_wndEdit.GetSafeHwnd () != NULL)
	{
		m_wndEdit.SetPrompt(lpszPrompt, clrText, bRedraw);
	}

	if (bRedraw && GetSafeHwnd() != NULL && (bColorWasChanged || m_strPrompt != strOldPrompt))
	{
		RedrawWindow(NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
	}
}
//**************************************************************************
void CBCGPComboBox::SetErrorMessage(LPCTSTR lpszPrompt, COLORREF clrText, BOOL bRedraw)
{
	ASSERT_VALID (this);

	CString strOldPrompt = m_strErrorMessage;
	BOOL bColorWasChanged = m_clrErrorText != clrText;

	m_strErrorMessage = (lpszPrompt == NULL) ? _T("") : lpszPrompt;
	m_clrErrorText = clrText;
	
	if (!m_strErrorMessage.IsEmpty())
	{
		SubclassEditBox();
	}

	if (m_wndEdit.GetSafeHwnd () != NULL)
	{
		m_wndEdit.SetErrorMessage(lpszPrompt, clrText, bRedraw);
	}

	if (bRedraw && GetSafeHwnd() != NULL && (bColorWasChanged || m_strErrorMessage != strOldPrompt))
	{
		RedrawWindow(NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
	}
}
//**************************************************************************
void CBCGPComboBox::PreSubclassWindow() 
{
	CComboBox::PreSubclassWindow();

	if (!m_strPrompt.IsEmpty())
	{
		SubclassEditBox();
		
		if (m_wndEdit.GetSafeHwnd () != NULL)
		{
			m_wndEdit.SetPrompt(m_strPrompt, m_clrPrompt, FALSE);
		}
	}
}
//**************************************************************************
int CBCGPComboBox::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CComboBox::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_strPrompt.IsEmpty())
	{
		SubclassEditBox();
	
		if (m_wndEdit.GetSafeHwnd () != NULL)
		{
			m_wndEdit.SetPrompt(m_strPrompt, m_clrPrompt, FALSE);
		}
	}
	
	return 0;
}
//**************************************************************************
void CBCGPComboBox::OnSetFocus(CWnd* pOldWnd) 
{
	CComboBox::OnSetFocus(pOldWnd);
	
	BOOL bDrawPrompt = (!m_strPrompt.IsEmpty() && GetWindowTextLength() == 0) || !m_strErrorMessage.IsEmpty();
	if (bDrawPrompt)
	{
		m_bIsButtonHighlighted = TRUE;
	}
	
	if (bDrawPrompt)
	{
		RedrawWindow (NULL, NULL, RDW_INVALIDATE | RDW_FRAME | RDW_UPDATENOW | RDW_ALLCHILDREN);
	}
}
//**************************************************************************
BOOL CBCGPComboBox::OnEditupdate() 
{
	if (!m_strErrorMessage.IsEmpty())
	{
		SetErrorMessage(NULL, m_clrErrorText);
	}

	return FALSE;
}
//**************************************************************************
BOOL CBCGPComboBox::OnSelchange() 
{
	if (!m_strErrorMessage.IsEmpty())
	{
		SetErrorMessage(NULL, m_clrErrorText);
	}

	if (m_bVisualManagerStyle)
	{
		RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_FRAME | RDW_UPDATENOW | RDW_ALLCHILDREN);
	}

	return FALSE;
}
//**************************************************************************
LRESULT CBCGPComboBox::OnSetText (WPARAM, LPARAM)
{
	LRESULT lRes = Default();

	if (m_bVisualManagerStyle)
	{
		RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_FRAME | RDW_UPDATENOW | RDW_ALLCHILDREN);
	}

	return lRes;
}
//**************************************************************************
LRESULT CBCGPComboBox::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	LRESULT lRes = CComboBox::WindowProc(message, wParam, lParam);

	if (message == CB_SETCURSEL && m_bVisualManagerStyle)
	{
		RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_FRAME | RDW_UPDATENOW | RDW_ALLCHILDREN);
	}

	return lRes;
}
//*******************************************************************************
LRESULT CBCGPComboBox::OnPrintClient(WPARAM wp, LPARAM lp)
{
	if ((lp & PRF_CLIENT) == PRF_CLIENT)
	{
		if (m_bDefaultPrintClient)
		{
			return Default();
		}

		CDC* pDC = CDC::FromHandle((HDC) wp);
		ASSERT_VALID(pDC);

		if ((GetStyle () & 0x0003L) == CBS_SIMPLE)
		{
			return Default ();
		}
		
		BOOL bDrawPrompt = FALSE;
		
		if (!m_strPrompt.IsEmpty() || !m_strErrorMessage.IsEmpty())
		{
			BOOL bTextIsEmpty = GetWindowTextLength() == 0;
			
			if (m_wndEdit.GetSafeHwnd () == NULL)
			{
				bDrawPrompt = bTextIsEmpty || !m_strErrorMessage.IsEmpty();
			}
		}

		if (!m_bVisualManagerStyle && !bDrawPrompt)
		{
			return Default();
		}
		
		OnDraw(pDC, bDrawPrompt);
	}

	return 0;
}
