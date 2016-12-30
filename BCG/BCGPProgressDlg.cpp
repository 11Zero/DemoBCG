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
// BCGPProgressDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BCGPProgressDlg.h"
#include "bcgprores.h"
#include "BCGPLocalResource.h"
#include "BCGPDrawManager.h"
#include "BCGPVisualManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBCGPProgressDlg dialog

CBCGPProgressDlg::CBCGPProgressDlg()
{
	//{{AFX_DATA_INIT(CBCGPProgressDlg)
	//}}AFX_DATA_INIT

	m_bCancelled = FALSE;
    m_bParentDisabled = FALSE;
	m_bWaitForMessages = TRUE;
	m_nCurPos = 0;
	m_yLine = -1;
	m_bIsLocal = TRUE;
}
//**************************************************************************************
CBCGPProgressDlg::~CBCGPProgressDlg()
{
    if (m_hWnd != NULL)
	{
		DestroyWindow ();
	}
}
//**************************************************************************************
BOOL CBCGPProgressDlg::Create (const CBCGPProgressDlgParams& params, CWnd* pParent)
{
	ASSERT_VALID (this);
	
	m_Params = params;
	
	m_nCurPos = m_Params.m_nRangeMin;
	m_bWaitForMessages = m_Params.m_bWaitForMessages;
	
    // Get the true parent of the dialog
    m_pParentWnd = CWnd::GetSafeOwner(pParent);
	
    // m_bParentDisabled is used to re-enable the parent window
    // when the dialog is destroyed. So we don't want to set
    // it to TRUE unless the parent was already enabled.
	
    if (m_Params.m_bDisableParentWnd && m_pParentWnd->GetSafeHwnd () !=NULL && m_pParentWnd->IsWindowEnabled ())
    {
		m_pParentWnd->EnableWindow (FALSE);
		m_bParentDisabled = TRUE;
    }
	
	m_pLocaRes = new CBCGPLocalResource ();

#ifdef _BCGSUITE_
	if (m_Params.m_bShowInfiniteProgress)
	{
		m_Impl.m_lstNonSubclassedItems.AddTail(IDC_BCGBARRES_PROGRESS);
	}
#endif

    if (!CBCGPDialog::Create (CBCGPProgressDlg::IDD, pParent))
    {
		ReEnableParent();
		return FALSE;
    }
	
	CenterWindow ();
    return TRUE;
}
//**************************************************************************************
void CBCGPProgressDlg::DoDataExchange(CDataExchange* pDX)
{
	CBCGPDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBCGPProgressDlg)
	DDX_Control(pDX, IDC_BCGBARRES_DLGLINE, m_wndLine);
	DDX_Control(pDX, IDCANCEL, m_wndCancel);
	DDX_Control(pDX, IDC_BCGBARRES_PROGRESS_PERC, m_wndProgressPerc);
	DDX_Control(pDX, IDC_BCGBARRES_MESSAGE2, m_wndMessage2);
	DDX_Control(pDX, IDC_BCGBARRES_MESSAGE, m_wndMessage);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CBCGPProgressDlg, CBCGPDialog)
	//{{AFX_MSG_MAP(CBCGPProgressDlg)
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGPProgressDlg message handlers

void CBCGPProgressDlg::OffsetWnd (CWnd* pWnd, int cy)
{
	CRect rectWindow;
	pWnd->GetWindowRect (rectWindow);

	ScreenToClient (&rectWindow);

	pWnd->SetWindowPos (NULL, rectWindow.left, rectWindow.top + cy, -1, -1,
		SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}
//**************************************************************************************
BOOL CBCGPProgressDlg::OnInitDialog() 
{
	CBCGPDialog::OnInitDialog();

	CRect rectWindow;
	GetWindowRect (rectWindow);

	int cx = rectWindow.Width ();
	int cy = rectWindow.Height ();

	CProgressCtrl* pWndProgress = GetProgressCtrl ();
	ASSERT_VALID (pWndProgress);
	
    pWndProgress->SetRange32 (m_Params.m_nRangeMin, m_Params.m_nRangeMax);
    pWndProgress->SetStep (m_Params.m_nStep);
    pWndProgress->SetPos (m_Params.m_nRangeMin);

	if (m_Params.m_bShowInfiniteProgress)
	{
		pWndProgress->ModifyStyle (0, PBS_MARQUEE);
		pWndProgress->PostMessage (PBM_SETMARQUEE, 1, 0);
	}

	SetWindowText (m_Params.m_strCaption);

	m_wndMessage.SetWindowText (m_Params.m_strMessage);

	int cyOffset = 0;

	if (m_Params.m_nHeaderHeight > 0)
	{
		cy += m_Params.m_nHeaderHeight;
		cyOffset = m_Params.m_nHeaderHeight;
	}

	if ((m_Params.m_nAnimationResID != 0 || !m_Params.m_strAnimationPath.IsEmpty ()) && 
		m_Params.m_nAnimationHeight > 0)
	{
		CRect rectAnim;
		GetClientRect (rectAnim);

		rectAnim.top += cyOffset;
		rectAnim.bottom = rectAnim.top + m_Params.m_nAnimationHeight;

		m_wndAnimation.Create (WS_CHILD | WS_VISIBLE | ACS_CENTER | ACS_TRANSPARENT | ACS_AUTOPLAY, 
			rectAnim, this, 0);

		if (m_Params.m_nAnimationResID != 0)
		{
			m_wndAnimation.Open (m_Params.m_nAnimationResID);
		}
		else
		{
			m_wndAnimation.Open (m_Params.m_strAnimationPath);
		}

		cy += m_Params.m_nAnimationHeight;
		cyOffset += m_Params.m_nAnimationHeight;
	}

	if (cyOffset != 0)
	{
		OffsetWnd (&m_wndMessage, cyOffset);
		OffsetWnd (pWndProgress, cyOffset);
		OffsetWnd (&m_wndProgressPerc, cyOffset);
	}

	if (!m_Params.m_bShowProgress)
	{
		CRect rectProgress;
		pWndProgress->GetWindowRect (rectProgress);
		ScreenToClient (&rectProgress);

		pWndProgress->ShowWindow (SW_HIDE);
		m_wndProgressPerc.ShowWindow (SW_HIDE);

		cy -= 2 * rectProgress.Height ();
		cyOffset -= 2 * rectProgress.Height ();
	}
	else if (!m_Params.m_bShowPercentage || m_Params.m_bShowInfiniteProgress)
	{
		CRect rectPerc;
		m_wndProgressPerc.GetWindowRect (rectPerc);
		ScreenToClient (&rectPerc);

		m_wndProgressPerc.ShowWindow (SW_HIDE);

		CRect rectProgress;
		pWndProgress->GetWindowRect (rectProgress);
		ScreenToClient (&rectProgress);

		rectProgress.right = rectPerc.right;
		pWndProgress->SetWindowPos (NULL, -1, -1, rectProgress.Width (), rectProgress.Height (),
			SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
	}

	if (cyOffset != 0)
	{
		OffsetWnd (&m_wndMessage2, cyOffset);
		OffsetWnd (&m_wndLine, cyOffset);
		OffsetWnd (&m_wndCancel, cyOffset);
	}

	if (m_Params.m_strMessage2.IsEmpty ())
	{
		CRect rectMessage2;
		m_wndMessage2.GetWindowRect (rectMessage2);

		CRect rectLine;
		m_wndLine.GetWindowRect (rectLine);

		int nOffset = rectLine.top - rectMessage2.top;

		cy -= nOffset;

		m_wndMessage2.ShowWindow (SW_HIDE);

		if (m_Params.m_bShowCancel)
		{
			OffsetWnd (&m_wndLine, -nOffset);
			OffsetWnd (&m_wndCancel, -nOffset);
		}
	}
	else
	{
		m_wndMessage2.SetWindowText (m_Params.m_strMessage2);
	}

	if (!m_Params.m_bShowCancel)
	{
		CRect rectLine;
		m_wndLine.GetWindowRect (rectLine);

		CRect rectCancel;
		m_wndCancel.GetWindowRect (rectCancel);

		ModifyStyle (WS_SYSMENU, 0);
		
		if (IsVisualManagerStyle())
		{
			OnChangeVisualManager(0, 0);
		}

		m_wndCancel.EnableWindow (FALSE);
		m_wndCancel.ShowWindow (SW_HIDE);
		m_wndLine.ShowWindow (SW_HIDE);

		cy -= rectCancel.bottom - rectLine.top;
	}

	SetWindowPos (NULL, -1, -1, cx, cy,
		SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

	if (m_Params.m_bShowCancel)
	{
		CRect rectLine;
		m_wndLine.GetWindowRect (rectLine);
		ScreenToClient (&rectLine);

		m_yLine = rectLine.CenterPoint ().y;

		m_wndLine.ShowWindow (SW_HIDE);
	}

	UpdatePercent(m_Params.m_nRangeMin);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
//**************************************************************************************
void CBCGPProgressDlg::OnCancel() 
{
	m_bCancelled = TRUE;
}
//**************************************************************************************
long CBCGPProgressDlg::SetPos(long nPos)
{
    if (!PumpMessages ())
	{
		return 0;
	}

	CProgressCtrl* pWndProgress = GetProgressCtrl ();
	ASSERT_VALID (pWndProgress);

    long lResult = pWndProgress->SetPos(nPos);
    UpdatePercent(nPos);

    return lResult;
}
//**************************************************************************************
long CBCGPProgressDlg::OffsetPos (long nPos)
{
    if (!PumpMessages ())
	{
		return 0;
	}

	CProgressCtrl* pWndProgress = GetProgressCtrl ();
	ASSERT_VALID (pWndProgress);

    long lResult = pWndProgress->OffsetPos(nPos);
    UpdatePercent (lResult + nPos);

    return lResult;
}
//**************************************************************************************
void CBCGPProgressDlg::SetMessage (const CString& strMessage)
{
	m_Params.m_strMessage = strMessage;

	if (m_wndMessage.GetSafeHwnd () != NULL)
	{
		m_wndMessage.SetWindowText (strMessage);
	}
}
//**************************************************************************************
void CBCGPProgressDlg::SetMessage2 (const CString& strMessage)
{
	if (m_Params.m_strMessage2.IsEmpty ())
	{
		ASSERT (FALSE);
		return;
	}

	m_Params.m_strMessage2 = strMessage;

	if (m_wndMessage2.GetSafeHwnd () != NULL)
	{
		m_wndMessage2.SetWindowText (strMessage);
	}
}
//**************************************************************************************
long CBCGPProgressDlg::StepIt(BOOL bWaitForMessages)
{
	m_bWaitForMessages = bWaitForMessages;
    if (!PumpMessages ())
	{
		return 0;
	}

	CProgressCtrl* pWndProgress = GetProgressCtrl ();
	ASSERT_VALID (pWndProgress);

    long lResult = pWndProgress->StepIt();
    UpdatePercent (lResult + m_Params.m_nStep);

    return lResult;
}
//**************************************************************************************
BOOL CBCGPProgressDlg::PumpMessages()
{
    // Must call Create() before using the dialog
    ASSERT (GetSafeHwnd () != NULL);
	
	HWND hwndThis = GetSafeHwnd ();

    MSG msg;

    // Handle dialog messages
    while (::PeekMessage (&msg, NULL, 0, 0, PM_REMOVE))
    {
		if (msg.message == WM_QUIT)
		{
			PostThreadMessage (GetCurrentThreadId(), 
				msg.message, msg.wParam, msg.lParam);
			return FALSE;
		}

		if (!::IsWindow (hwndThis))
		{
			return FALSE;
		}

		if (!IsDialogMessage (&msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);  
		}

		if (!::IsWindow (hwndThis))
		{
			return FALSE;
		}

		if (m_bWaitForMessages)
		{
			WaitMessage ();
		}
    }

	return TRUE;
}
//**************************************************************************************
void CBCGPProgressDlg::UpdatePercent (long nNewPos)
{
	if (m_Params.m_bShowInfiniteProgress)
	{
		m_nCurPos += m_Params.m_nStep;
		return;
	}

	m_nCurPos = nNewPos;

	if (!m_Params.m_bShowPercentage || !m_Params.m_bShowProgress)
	{
		return;
	}

    int nDivisor = m_Params.m_nRangeMax - m_Params.m_nRangeMin;
    ASSERT (nDivisor > 0);  // m_nLower should be smaller than m_nUpper

    int nDividend = nNewPos - m_Params.m_nRangeMin;
    ASSERT (nDividend>=0);   // Current position should be greater than m_nLower

    int nPercent = nDividend * 100 / nDivisor;

    // Since the Progress Control wraps, we will wrap the percentage
    // along with it. However, don't reset 100% back to 0%
    if (nPercent != 100)
	{
		nPercent %= 100;
	}

    // Display the percentage
    CString strBuf;
    strBuf.Format(_T("%d %%"), nPercent);

	CString strCur; // get current percentage
    m_wndProgressPerc.GetWindowText(strCur);

	if (strCur != strBuf)
	{
		m_wndProgressPerc.SetWindowText(strBuf);
	}
}
    
//**************************************************************************************
BOOL CBCGPProgressDlg::DestroyWindow() 
{
    ReEnableParent();
	return CBCGPDialog::DestroyWindow();
}
//**************************************************************************************
void CBCGPProgressDlg::ReEnableParent()
{
    if (m_bParentDisabled && (m_pParentWnd->GetSafeHwnd () != NULL))
	{
		m_pParentWnd->EnableWindow ();
	}

    m_bParentDisabled = FALSE;
}
//**************************************************************************************
BOOL CBCGPProgressDlg::OnEraseBkgnd(CDC* pDC) 
{
	ASSERT_VALID (pDC);

	BOOL bRes = TRUE;

	CRect rectClient;
	GetClientRect (rectClient);

	CBCGPDrawManager dm(*pDC);

	if (!IsWindowBackground ())
	{
		bRes = CBCGPDialog::OnEraseBkgnd(pDC);

		if (m_yLine >= 0 && globalData.m_nBitsPerPixel > 8 && !globalData.IsHighContastMode ())
		{
			CRect rectButtons = rectClient;
			rectButtons.top = m_yLine;

			if (m_Params.m_bDialogLook)
			{
				rectButtons.DeflateRect(10, 0);
				dm.DrawLine (rectButtons.left, m_yLine, rectButtons.right, m_yLine, globalData.clrBtnShadow);
			}
			else
			{
				int nShadowSize = 4;

				rectButtons.top -= nShadowSize / 2;

				CRect rectShadow;
				GetClientRect(rectShadow);
				rectShadow.bottom = rectButtons.top;

				rectShadow.left -= 2 * nShadowSize;

				CBCGPVisualManager::GetInstance()->OnDrawButtonsArea(pDC, this, rectButtons);

				dm.DrawShadow(rectShadow, nShadowSize, 100, 70);
			}
		}
	}
	else
	{
		pDC->FillRect (rectClient, &globalData.brWindow);

		if (m_yLine >= 0)
		{
			CRect rectButtons = rectClient;
			rectButtons.top = m_yLine;

			pDC->FillRect (rectButtons, &globalData.brBtnFace);
			dm.DrawLine (rectButtons.left, m_yLine, rectButtons.right, m_yLine, globalData.clrBtnShadow);
		}
	}

	return bRes;
}
//**************************************************************************************
HBRUSH CBCGPProgressDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	if (pWnd->GetSafeHwnd () == m_wndAnimation.GetSafeHwnd ())
	{
		if (IsVisualManagerStyle ())
		{
			return (HBRUSH) CBCGPVisualManager::GetInstance ()->GetDlgBackBrush (this).GetSafeHandle ();
		}

		if (!m_Params.m_bDialogLook)
		{
			return (HBRUSH) globalData.brWindow.GetSafeHandle ();
		}
	}

	if (IsWindowBackground () && !globalData.IsHighContastMode())
	{
		#define MAX_CLASS_NAME	255
		#define STATIC_CLASS	_T("Static")

		if (nCtlColor == CTLCOLOR_STATIC)
		{
			TCHAR lpszClassName [MAX_CLASS_NAME + 1];

			::GetClassName (pWnd->GetSafeHwnd (), lpszClassName, MAX_CLASS_NAME);
			CString strClass = lpszClassName;

			if (strClass == STATIC_CLASS)
			{
				pDC->SetBkMode (TRANSPARENT);
				return (HBRUSH) ::GetStockObject (HOLLOW_BRUSH);
			}
		}
	}

	return CBCGPDialog::OnCtlColor(pDC, pWnd, nCtlColor);
}
//**************************************************************************************
void CBCGPProgressDlg::RedrawHeader ()
{
	if (GetSafeHwnd () == NULL || m_Params.m_nHeaderHeight <= 0)
	{
		return;
	}

	CRect rectHeader;
	GetClientRect (rectHeader);

	rectHeader.bottom = rectHeader.top + m_Params.m_nHeaderHeight;

	RedrawWindow (rectHeader, NULL, RDW_UPDATENOW | RDW_INVALIDATE);
}
//**************************************************************************************
void CBCGPProgressDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	if (m_Params.m_nHeaderHeight > 0)
	{
		CRect rectHeader;
		GetClientRect (rectHeader);

		rectHeader.bottom = rectHeader.top + m_Params.m_nHeaderHeight;

		CBCGPMemDC memDC (dc, rectHeader);
		CDC* pDC = &memDC.GetDC ();

		OnDrawHeader (pDC, rectHeader);
	}
}
