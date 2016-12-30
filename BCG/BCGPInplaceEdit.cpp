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
// BCGPInplaceEdit.cpp : implementation file
//

#include "stdafx.h"
#include "BCGPInplaceEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBCGPInplaceEdit

CBCGPInplaceEdit::CBCGPInplaceEdit()
{
	m_bResizeEditor = FALSE;// if TRUE, editor changes its height and width on content changing
	m_bWrapText = FALSE;	// if FALSE, editor enlarge its width to fit text

	m_rectInital.SetRectEmpty ();	// Initial bounding rect of the editor
	m_nTopMargin = 0;				// Top margin for vertical alignment
	m_rectEditMax.SetRectEmpty ();	// Maximal size of the editor

	m_nVStep = 18;
	m_nHLeftStep = 30;
	m_nHRightStep = 30;
	m_VertAlign = VA_Top;
}

CBCGPInplaceEdit::~CBCGPInplaceEdit()
{
}


BEGIN_MESSAGE_MAP(CBCGPInplaceEdit, CEdit)
	//{{AFX_MSG_MAP(CBCGPInplaceEdit)
	ON_CONTROL_REFLECT(EN_UPDATE, OnUpdateR)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_SETTEXT, OnSetText)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGPInplaceEdit message handlers

void CBCGPInplaceEdit::OnUpdateR() 
{
	ResizeEditor ();
}
//*******************************************************************************
int CBCGPInplaceEdit::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CEdit::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	CRect rect;
	GetWindowRect (rect);
	GetParent ()->ScreenToClient (rect);
	m_rectInital = rect;

	DWORD dwStyle = GetStyle ();
	BOOL bAutoHScroll = (dwStyle & ES_AUTOHSCROLL) != 0;
	BOOL bMultiLine = (dwStyle & ES_MULTILINE) != 0;
	m_bWrapText = bMultiLine && !bAutoHScroll;

	ResizeEditor ();

	return 0;
}
//*******************************************************************************
LRESULT CBCGPInplaceEdit::OnSetText (WPARAM, LPARAM)
{
	LRESULT lRes = Default ();

	ResizeEditor ();

	return lRes;
}
//*******************************************************************************
void CBCGPInplaceEdit::ResizeEditor ()
{
	ASSERT_VALID (this);

	CString str;
	CWnd::GetWindowText(str);

	CRect rect;
	GetWindowRect (rect);
	GetParent ()->ScreenToClient (rect);

	CDC* pDC = GetWindowDC ();

	//-------------
	// Select font:
	//-------------
	CFont* pFont = GetFont ();
	if (pFont != NULL)
	{
		ASSERT_VALID (pFont);
		pDC->SelectObject (pFont);
	}

	UINT uiTextFlags = DT_LEFT | DT_NOPREFIX | DT_CALCRECT;

	//---------------
	// Word wrapping:
	//---------------
	BOOL bMultiLine = (GetStyle () & ES_MULTILINE) != 0;
	if (!bMultiLine)
	{
		uiTextFlags |= DT_SINGLELINE | DT_VCENTER;
	}
	else if (bMultiLine && m_bWrapText)
	{
		uiTextFlags |= DT_WORDBREAK;
	}
	else // multiline && no wraptext
	{
	}


	//-----------
	// Alignment:
	//-----------
	DWORD dwStyle = GetStyle ();
	if ((dwStyle & ES_CENTER) != 0)
	{
		uiTextFlags |= DT_CENTER;
	}
	else if ((dwStyle & ES_RIGHT) != 0)
	{
		uiTextFlags |= DT_RIGHT;
	}

	//-----------------------
	// Calculate text extent:
	//-----------------------
	CRect rectNew = rect;

	if (str.IsEmpty ())
	{
		str = _T("W");
	}
	pDC->DrawText (str, rectNew, uiTextFlags);

	const int nMinWidth = pDC->GetTextExtent (_T("W")).cx;
	rectNew.right = max (rectNew.left + nMinWidth, rectNew.right);

	//---------
	// Padding:
	//---------
	if (m_VertAlign == VA_Top) // GRID_VTOP
	{
		int nTopMargin = 0;
		m_nTopMargin = max (0, nTopMargin);
	}
	if (m_VertAlign == VA_Center) // GRID_VCENTER
	{
		CPoint ptCenter = m_rectInital.CenterPoint ();
		int nTop = ptCenter.y - rectNew.Height () / 2;
		int nTopMargin = nTop - rectNew.top;
		m_nTopMargin = max (0, nTopMargin);
	}
	if (m_VertAlign == VA_Bottom) // GRID_VBOTTOM
	{
		int nTop = m_rectInital.bottom - rectNew.Height ();
		int nTopMargin = nTop - rectNew.top;
		m_nTopMargin = max (0, nTopMargin);
	}
	ASSERT (m_nTopMargin >= 0);

	//---------------------------
	// Inflate editor if allowed:
	//---------------------------
	BOOL bResize = FALSE;
	if (CanEnlargeHeight ())
	{
		bResize = (rectNew.Height () != m_rectInital.Height ());
		rectNew.bottom = max (m_rectInital.bottom, rectNew.bottom);
	}
	else
	{
		rectNew.bottom = m_rectInital.bottom;
	}
	if (CanEnlargeWidth ())
	{
		if (rectNew.Width () > m_rectInital.Width ())
		{
			bResize = TRUE;

			DWORD dwStyle = GetStyle ();
			if ((dwStyle & ES_CENTER) != 0)
			{
				m_rectInital.left -= m_nHLeftStep;
				m_rectInital.right += m_nHRightStep;
			}
			else if ((dwStyle & ES_RIGHT) != 0)
			{
				rectNew.left -= (rectNew.Width () > m_rectInital.Width ());
				rectNew.right = m_rectInital.right;
				m_rectInital.left -= m_nHLeftStep;
			}
			else // (dwStyle & ES_LEFT) != 0
			{
				m_rectInital.right += m_nHRightStep;
			}
		}

		rectNew.left = min (rectNew.left, m_rectInital.left);
		rectNew.right = max (rectNew.right, m_rectInital.right);

	}
	else
	{
		rectNew.left = m_rectInital.left;
		rectNew.right = m_rectInital.right;
	}

	//----------------------------
	// Vertically align the editor:
	//----------------------------
	if (bResize)
	{
		SetWindowPos (NULL, rectNew.left, rectNew.top, rectNew.Width (), rectNew.Height (), SWP_NOACTIVATE | SWP_NOZORDER);
	}

	CRect rectFormating (0, 0, rectNew.Width (), rectNew.Height ());
	rectFormating.top += m_nTopMargin;
	SendMessage (EM_SETRECT, 0, (LPARAM)(LPRECT)&rectFormating);
}
