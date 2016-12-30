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
// KeyAssign.cpp : implementation file
//

#include "stdafx.h"
#include "KeyAssign.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CKeyAssign

CKeyAssign::CKeyAssign() :
	m_Helper (&m_Accel)
{
    m_bIsDefined = FALSE;
	m_bIsFocused = FALSE;

	ResetKey ();
}
//***************************************************************************
CKeyAssign::~CKeyAssign()
{
}

BEGIN_MESSAGE_MAP(CKeyAssign, CEdit)
	//{{AFX_MSG_MAP(CKeyAssign)
	ON_WM_KILLFOCUS()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CKeyAssign message handlers

BOOL CKeyAssign::PreTranslateMessage(MSG* pMsg) 
{
    BOOL bIsKeyPressed = FALSE;
	BOOL bIsFirstPress = FALSE;

	switch (pMsg->message)
	{
	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		m_bIsFocused = TRUE;
		SetFocus ();
		return TRUE;

	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		bIsKeyPressed = TRUE;
		bIsFirstPress = (pMsg->lParam & (1 << 30)) != 0;
		// To the key processing....

	case WM_KEYUP:
	case WM_SYSKEYUP:
		{
			if (bIsKeyPressed && m_bIsDefined && !bIsFirstPress)
			{
				ResetKey ();
			}

			if (!m_bIsDefined)
			{
				switch (pMsg->wParam)
				{
				case VK_SHIFT:
					SetAccelFlag (FSHIFT, bIsKeyPressed);
					break;

				case VK_CONTROL:
					SetAccelFlag (FCONTROL, bIsKeyPressed);
					break;

				case VK_MENU:
					SetAccelFlag (FALT, bIsKeyPressed);
					break;

				default:
					if (!m_bIsFocused)
					{
						m_bIsFocused = TRUE;
						return TRUE;
					}

					m_Accel.key = (WORD) pMsg->wParam;

					if (bIsKeyPressed)
					{
						m_bIsDefined = TRUE;
						SetAccelFlag (FVIRTKEY, TRUE);
					}
				}
			}

			BOOL bDefaultProcess = FALSE;

			if ((m_Accel.fVirt & FCONTROL) == 0 &&
				(m_Accel.fVirt & FSHIFT) == 0 &&
				(m_Accel.fVirt & FALT) == 0 &&
				(m_Accel.fVirt & FVIRTKEY))
			{
				switch (m_Accel.key)
				{
				case VK_ESCAPE:
					ResetKey ();
					return TRUE;

				case VK_TAB:
					bDefaultProcess = TRUE;
				}
			}

			if (!bDefaultProcess)
			{
				CString strKbd;
				m_Helper.Format (strKbd);

				SetWindowText (strKbd);
				return TRUE;
			}

			ResetKey ();
		}
	}

    return CEdit::PreTranslateMessage(pMsg);
}
//******************************************************************
void CKeyAssign::ResetKey ()
{
	memset (&m_Accel, 0, sizeof (ACCEL));
    m_bIsDefined = FALSE;

	if (m_hWnd != NULL)
	{
		SetWindowText (_T(""));
	}
}
//******************************************************************
void CKeyAssign::SetAccelFlag (BYTE bFlag, BOOL bOn)
{
	if (bOn)
	{
		m_Accel.fVirt |= bFlag;
	}
	else
	{
		m_Accel.fVirt &= ~bFlag;
	}
}
//******************************************************************
void CKeyAssign::OnKillFocus(CWnd* pNewWnd) 
{
	m_bIsFocused = FALSE;
	CEdit::OnKillFocus(pNewWnd);
}
