// XTPOffice2007FrameHook.cpp : implementation file
//
// This file is a part of the XTREME RIBBON MFC class library.
// (c)1998-2011 Codejock Software, All Rights Reserved.
//
// THIS SOURCE FILE IS THE PROPERTY OF CODEJOCK SOFTWARE AND IS NOT TO BE
// RE-DISTRIBUTED BY ANY MEANS WHATSOEVER WITHOUT THE EXPRESSED WRITTEN
// CONSENT OF CODEJOCK SOFTWARE.
//
// THIS SOURCE CODE CAN ONLY BE USED UNDER THE TERMS AND CONDITIONS OUTLINED
// IN THE XTREME TOOLKIT PRO LICENSE AGREEMENT. CODEJOCK SOFTWARE GRANTS TO
// YOU (ONE SOFTWARE DEVELOPER) THE LIMITED RIGHT TO USE THIS SOFTWARE ON A
// SINGLE COMPUTER.
//
// CONTACT INFORMATION:
// support@codejock.com
// http://www.codejock.com
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "Common/XTPResourceImage.h"

#include "XTPCommandBars.h"
#include "XTPControlButton.h"

#include "XTPOffice2007Theme.h"
#include "XTPOffice2007FrameHook.h"

#ifdef _XTP_INCLUDE_RIBBON
#include "Ribbon/XTPRibbonBar.h"
#include "Ribbon/XTPRibbonPaintManager.h"
#endif

#define XTP_TID_MOUSELEAVE  0xACB1
#define XTP_TID_REFRESHFRAME 0xACD1

#ifndef WM_NCMOUSELEAVE
#define WM_NCMOUSELEAVE     0x02A2
#endif


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BOOL CXTPOffice2007FrameHook::m_bAllowDwm = TRUE;

class CXTPOffice2007FrameHook::CControlCaptionButton : public CXTPControl
{
public:
	CControlCaptionButton(CXTPOffice2007FrameHook* pFrame)
	{
		m_pFrame = pFrame;
	}

public:
	virtual int GetSelected() const
	{
		return m_pFrame->m_pSelected == (CXTPControl*)this;
	}
	virtual BOOL GetPressed() const
	{
		return m_pFrame->m_pSelected == (CXTPControl*)this && m_pFrame->m_bButtonPressed;
	}

protected:
	CXTPOffice2007FrameHook* m_pFrame;
};

void CXTPOffice2007FrameHook::AddCaptionButton(int nId, int nHit)
{
	CXTPControl* pButton = new CControlCaptionButton(this);
	pButton->SetID(nId);
	pButton->SetHelpId(nHit);
	m_pCaptionButtons->Add(pButton);
}


class CXTPOffice2007FrameHook::CWinEventHook
{
	DECLARE_HANDLE(HWINEVENTHOOK);

public:
	CWinEventHook();
	~CWinEventHook();

public:
	void SetEventHook();
	void RemoveEventHook();

protected:
	typedef void (CALLBACK* WINEVENTPROC) ( HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD idEventThread, DWORD dwmsEventTime);
	typedef HWINEVENTHOOK (WINAPI* LPFNSETWINEVENTHOOK)(UINT eventMin, UINT eventMax, HMODULE hmodWinEventProc,
		WINEVENTPROC lpfnWinEventProc, DWORD idProcess, DWORD idThread, UINT dwflags);
	typedef BOOL (WINAPI* LPFNUNHOOKWINEVENT)(HWINEVENTHOOK hWinEventHook);

	static void CALLBACK WinEventProc(HWINEVENTHOOK /*hWinEventHook*/,
		DWORD event, HWND hWnd, LONG idObject, LONG /*idChild*/, DWORD /*dwEventThread*/, DWORD /*dwmsEventTime*/);

protected:
	HWINEVENTHOOK m_hWinEventHook;      // HWINEVENTHOOK value that identifies event hook instance
	LPFNSETWINEVENTHOOK m_pSetWinEventHook;
	LPFNUNHOOKWINEVENT m_pUnhookWinEvent;

public:
	int m_nCount;
};

UINT CXTPOffice2007FrameHook::m_nMsgQueryFrameHook = 0;

CXTPOffice2007FrameHook::CWinEventHook::CWinEventHook()
{
	m_pSetWinEventHook = NULL;
	m_pUnhookWinEvent = NULL;
	m_hWinEventHook = NULL;
	m_nCount = 0;
}

CXTPOffice2007FrameHook::CWinEventHook::~CWinEventHook()
{

}

#define EVENT_OBJECT_REORDER 0x8004

void CXTPOffice2007FrameHook::CWinEventHook::SetEventHook()
{
	if (m_nCount > 0)
	{
		m_nCount++;
		return;
	}

	HMODULE hLib = GetModuleHandle(_T("USER32"));
	if (hLib)
	{
		m_pSetWinEventHook = (LPFNSETWINEVENTHOOK)GetProcAddress(hLib, "SetWinEventHook");
		m_pUnhookWinEvent = (LPFNUNHOOKWINEVENT)GetProcAddress(hLib, "UnhookWinEvent");

		if (m_pSetWinEventHook && m_pUnhookWinEvent)
		{
			m_hWinEventHook = m_pSetWinEventHook(EVENT_OBJECT_REORDER, EVENT_OBJECT_REORDER, NULL,
				&CXTPOffice2007FrameHook::CWinEventHook::WinEventProc, GetCurrentProcessId(), 0, 0);
		}
	}

	m_nCount++;
}

void CXTPOffice2007FrameHook::CWinEventHook::RemoveEventHook()
{
	m_nCount--;

	if (m_nCount > 0)
		return;

	HMODULE hLib = GetModuleHandle(_T("USER32"));
	if (m_hWinEventHook && hLib && m_pUnhookWinEvent)
	{
		m_pUnhookWinEvent(m_hWinEventHook);
	}
	m_hWinEventHook = NULL;
}

void CALLBACK CXTPOffice2007FrameHook::CWinEventHook::WinEventProc(HWINEVENTHOOK /*hWinEventHook*/,
	DWORD event, HWND hWnd, LONG idObject, LONG /*idChild*/, DWORD /*dwEventThread*/, DWORD /*dwmsEventTime*/)
{
	if (event == EVENT_OBJECT_REORDER && idObject == OBJID_WINDOW)
	{
		if (!XTPHookManager()->Lookup(hWnd))
			return;

		CXTPOffice2007FrameHook* pFrame = (CXTPOffice2007FrameHook*)::SendMessage(hWnd, CXTPOffice2007FrameHook::m_nMsgQueryFrameHook, 0, 0);

		if (pFrame && !pFrame->IsDwmEnabled())
		{
			DWORD dwStyle = pFrame->GetSiteStyle();
			DWORD dwStyleRemove = (WS_CAPTION | WS_VSCROLL | WS_HSCROLL);

			if (dwStyle & dwStyleRemove)
			{
				pFrame->GetSite()->SetTimer(XTP_TID_REFRESHFRAME, 100, NULL);
			}
		}
	}
}


CXTPOffice2007FrameHook::CXTPOffice2007FrameHook()
{
	m_pCommandBars = NULL;
	m_pRibbonBar = NULL;
	m_pPaintManager = NULL;

	m_bActive = TRUE;
	m_szFrameRegion = CSize(0, 0);
	m_hwndSite = NULL;
	m_pSelected = NULL;
	m_bDelayReclalcLayout = FALSE;
	m_bButtonPressed = FALSE;
	m_bOffice2007FrameEnabled = FALSE;
	m_nSkipNCPaint = 0;

	m_nFrameBorder = 0;
	m_bInUpdateFrame = FALSE;
	m_bLockNCPaint = FALSE;

	m_nMsgUpdateSkinState = RegisterWindowMessage(_T("WM_SKINFRAMEWORK_UPDATESTATE"));
	m_nMsgQuerySkinState = RegisterWindowMessage(_T("WM_SKINFRAMEWORK_QUERYSTATE"));
	if (m_nMsgQueryFrameHook == 0) m_nMsgQueryFrameHook = RegisterWindowMessage(_T("WM_GETFRAMEHOOK"));


	m_bDwmEnabled = FALSE;

	m_pCaptionButtons = new CXTPControls();
	AddCaptionButton(SC_CLOSE, HTCLOSE);
	AddCaptionButton(SC_MAXIMIZE, HTMAXBUTTON);
	AddCaptionButton(SC_RESTORE, HTMAXBUTTON);
	AddCaptionButton(SC_MINIMIZE, HTMINBUTTON);


	m_Sink.Advise(XTPResourceImages()->GetConnection(), XTP_NC_COMMON_RESOURCEIMAGES_CHANGED,
		&CXTPOffice2007FrameHook::OnResourceImagesChanged);
}

void CXTPOffice2007FrameHook::OnResourceImagesChanged(XTP_NOTIFY_CODE Event, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam); UNREFERENCED_PARAMETER(lParam);

	ASSERT(Event == XTP_NC_COMMON_RESOURCEIMAGES_CHANGED);
	if (Event == XTP_NC_COMMON_RESOURCEIMAGES_CHANGED)
	{
		if (m_hwndSite && IsWindow(m_hwndSite) && m_bOffice2007FrameEnabled && !m_bDwmEnabled)
		{
			GetPaintManager()->RefreshMetrics();
			RedrawFrame();
		}
	}
}


void CXTPOffice2007FrameHook::DisableOffice2007Frame()
{
	m_pCommandBars = NULL;
	m_pRibbonBar = NULL;
	m_pPaintManager = NULL;

	m_pSelected = NULL;
	m_bDelayReclalcLayout = FALSE;
	m_bButtonPressed = FALSE;
	m_bOffice2007FrameEnabled = FALSE;

	XTPHookManager()->RemoveAll(this);

	if (m_hwndSite)
	{
		if (m_bDwmEnabled)
		{
			CXTPWinDwmWrapper().ExtendFrameIntoClientArea(m_hwndSite, 0, 0, 0, 0);
		}

		SendMessage(m_hwndSite, m_nMsgUpdateSkinState, 0, 0);

		SetWindowRgn(m_hwndSite, (HRGN)NULL, TRUE);
		::SetWindowPos(m_hwndSite, 0, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
	}
	m_hwndSite = NULL;
	m_bDwmEnabled = FALSE;

	SetWinHook();
}

void CXTPOffice2007FrameHook::EnableOffice2007Frame(CWnd* pSite, CXTPPaintManager* pPaintManager)
{
	ASSERT(pSite);
	ASSERT(pPaintManager);

	if (!pSite || !pPaintManager)
		return;

	m_pCommandBars = NULL;
	m_pRibbonBar = NULL;
	m_pPaintManager = pPaintManager;
	m_hwndSite = pSite->GetSafeHwnd();

	m_bActive = TRUE;
	m_szFrameRegion = CSize(0, 0);

	m_pSelected = NULL;

	m_bDelayReclalcLayout = FALSE;
	m_bButtonPressed = FALSE;

	m_bOffice2007FrameEnabled = TRUE;

	m_bDwmEnabled = !m_bAllowDwm || pSite->GetStyle() & WS_CHILD ? FALSE : CXTPWinDwmWrapper().IsCompositionEnabled();

	XTPHookManager()->SetHook(m_hwndSite, this);
	SendMessage(m_hwndSite, m_nMsgUpdateSkinState, 0, 0);

	if (!m_bDwmEnabled)
	{
		RefreshFrameStyle();
	}

	::SetWindowPos(m_hwndSite, 0, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

	SetWinHook();
}

CXTPOffice2007FrameHook::~CXTPOffice2007FrameHook()
{
	CMDTARGET_RELEASE(m_pCaptionButtons);

	if (m_bOffice2007FrameEnabled)
	{
		EnableOffice2007Frame(NULL);
	}
}


DWORD CXTPOffice2007FrameHook::GetSiteStyle(BOOL bExStyle) const
{
	return m_hwndSite ? (DWORD)GetWindowLong(m_hwndSite, bExStyle ? GWL_EXSTYLE : GWL_STYLE) : 0;
}

void CXTPOffice2007FrameHook::EnableOffice2007Frame(CXTPCommandBars* pCommandBars)
{
	BOOL bEnabled = pCommandBars != NULL;
	if (m_bOffice2007FrameEnabled == bEnabled)
		return;

	m_pCommandBars = pCommandBars;
	m_pRibbonBar = NULL;
	m_pPaintManager = NULL;

	if (pCommandBars)
	{
#ifdef _XTP_INCLUDE_RIBBON
		m_pRibbonBar = DYNAMIC_DOWNCAST(CXTPRibbonBar, pCommandBars->GetMenuBar());
#endif
		m_hwndSite = pCommandBars->GetSite()->GetSafeHwnd();
	}

	m_bActive = TRUE;
	m_szFrameRegion = CSize(0, 0);

	m_pSelected = NULL;

	m_bDelayReclalcLayout = FALSE;
	m_bButtonPressed = FALSE;

	m_bOffice2007FrameEnabled = bEnabled;

	if (pCommandBars)
	{
		XTPHookManager()->SetHook(m_hwndSite, this);
		SendMessage(m_hwndSite, m_nMsgUpdateSkinState, 0, 0);

		m_bDwmEnabled = !m_bAllowDwm || GetSiteStyle() & WS_CHILD ? FALSE : CXTPWinDwmWrapper().IsCompositionEnabled();

		if (!m_bDwmEnabled)
		{
			RefreshFrameStyle();
		}
		else
		{
			SetWindowRgn(m_hwndSite, NULL, TRUE);
		}

		UpdateFrameRegion();
		::SetWindowPos(m_hwndSite, 0, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

		SetWinHook();
	}
	else
	{
		DisableOffice2007Frame();
	}
}

void CXTPOffice2007FrameHook::SetWinHook()
{
	static CWinEventHook hook;
	if (m_hwndSite)
	{
		hook.SetEventHook();
	}
	else
	{
		hook.RemoveEventHook();
	}

};


CXTPPaintManager* CXTPOffice2007FrameHook::GetPaintManager() const
{
	return m_pCommandBars ? (CXTPOffice2007Theme*)m_pCommandBars->GetPaintManager() : (CXTPOffice2007Theme*)m_pPaintManager;
}

CXTPFramePaintManager* CXTPOffice2007FrameHook::GetFramePaintManager() const
{
	return GetPaintManager()->GetFramePaintManager();
}


void CXTPOffice2007FrameHook::UpdateFrameRegion()
{
	if (!IsWindow(m_hwndSite))
		return;

	CXTPWindowRect rc(m_hwndSite);
	UpdateFrameRegion(rc.Size(), TRUE);
}

void CXTPOffice2007FrameHook::UpdateFrameRegion(CSize szFrameRegion, BOOL bUpdate)
{
	if (m_bInUpdateFrame)
		return;

	m_bInUpdateFrame = TRUE;

	if (m_szFrameRegion != szFrameRegion || bUpdate)
	{
		CRect rc(0, 0, szFrameRegion.cx, szFrameRegion.cy);
		AdjustWindowRectEx(rc, GetSiteStyle(FALSE), FALSE, GetSiteStyle(TRUE));

		m_nFrameBorder = -rc.left;

		if (!m_bDwmEnabled)
		{
			HRGN hRgn = 0;

			if (!IsMDIMaximized())
			{
				if (GetSiteStyle() & WS_MAXIMIZE)
				{
					int nFrameRegion = m_nFrameBorder - (GetSiteStyle(TRUE) & WS_EX_CLIENTEDGE ? 2 : 0);
					hRgn = CreateRectRgn(nFrameRegion, nFrameRegion, szFrameRegion.cx - nFrameRegion, szFrameRegion.cy - nFrameRegion);
				}
				else
					hRgn = GetFramePaintManager()->CalcFrameRegion(this, szFrameRegion);
			}

			::SetWindowRgn(m_hwndSite, hRgn, TRUE);
		}
		else
		{
			if (!(GetSiteStyle() & WS_MAXIMIZE))
			{
				m_nFrameBorder = 4;
			}
			int cyTopHeight = IsCaptionVisible() ? 0 : GetCaptionHeight();

#ifdef _XTP_INCLUDE_RIBBON
			if (GetFramePaintManager()->m_bFlatFrame && !IsCaptionVisible() && m_pRibbonBar)
				cyTopHeight += m_pRibbonBar->GetTabsHeight() + 1;
#endif
			CXTPWinDwmWrapper().ExtendFrameIntoClientArea(m_hwndSite, 0, cyTopHeight, 0, 0);
		}

		m_szFrameRegion = szFrameRegion;
	}

	m_bInUpdateFrame = FALSE;
}

BOOL CXTPOffice2007FrameHook::IsMDIMaximized() const
{
	if ((GetSiteStyle() & WS_MAXIMIZE) && (GetSiteStyle(TRUE) & WS_EX_MDICHILD))
		return TRUE;

	return FALSE;
}

BOOL CXTPOffice2007FrameHook::IsCaptionVisible() const
{
	if (!m_pRibbonBar)
		return TRUE;

#ifdef _XTP_INCLUDE_RIBBON
	if (!(m_pRibbonBar->IsRibbonBarVisible() && m_pRibbonBar->GetPosition() == xtpBarTop))
		return TRUE;
#endif

	if ((GetSiteStyle() & (WS_CHILD | WS_MINIMIZE)) == (WS_CHILD | WS_MINIMIZE))
		return TRUE;

	return FALSE;
}

BOOL CXTPOffice2007FrameHook::IsFrameHasStatusBar(int* pnStatusHeight /*= NULL*/) const
{
	if (!GetFramePaintManager()->m_bFrameStatusBar)
		return FALSE;

	CWnd* pStatusBar = CWnd::FromHandle(::GetDlgItem(m_hwndSite, AFX_IDW_STATUS_BAR));
	if (!pStatusBar)
		return FALSE;

	if ((GetWindowLong(pStatusBar->m_hWnd, GWL_STYLE) & WS_VISIBLE) == 0)
		return FALSE;

	if (!GetPaintManager()->m_bThemedStatusBar)
		return FALSE;

	if (pnStatusHeight)
	{
		*pnStatusHeight = CXTPWindowRect(pStatusBar).Height();
	}

	return TRUE;
}


void CXTPOffice2007FrameHook::RedrawFrame()
{
	if (!m_bDwmEnabled)
	{
		CWindowDC dc(GetSite());
		GetFramePaintManager()->DrawFrame(&dc, this);
	}
}

void CXTPOffice2007FrameHook::DrawRibbonFramePart(CDC* pDC)
{
#ifdef _XTP_INCLUDE_RIBBON
	if (m_pRibbonBar)
	{
		m_pRibbonBar->GetRibbonPaintManager()->DrawRibbonFramePart(pDC, m_pRibbonBar);
	}
#else
	UNREFERENCED_PARAMETER(pDC);
#endif
}

void CXTPOffice2007FrameHook::RedrawRibbonBar()
{
#ifdef _XTP_INCLUDE_RIBBON
	if (m_pRibbonBar)
	{
		m_pRibbonBar->Redraw();
		m_pRibbonBar->UpdateWindow();
	}
#endif
}

void CXTPOffice2007FrameHook::DelayRecalcFrameLayout()
{
	m_bDelayReclalcLayout = TRUE;
	::PostMessage(m_hwndSite, WM_IDLEUPDATECMDUI, 0, 0);
}

void CXTPOffice2007FrameHook::RecalcFrameLayout()
{
	m_bDelayReclalcLayout = FALSE;
	m_szFrameRegion = CSize(0);
	::SetWindowPos(m_hwndSite, 0, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

	CWnd* pSite = GetSite();

	CFrameWnd* pFrame = pSite->IsFrameWnd() ? (CFrameWnd*)pSite : NULL;
	if (pFrame)
	{
		pFrame->RecalcLayout(FALSE);
	}
	else
	{
		CXTPClientRect rc(pSite);
		pSite->SendMessage(WM_SIZE, 0, MAKELPARAM(rc.Width(), rc.Height()));
	}

#ifdef _XTP_INCLUDE_RIBBON
	if (m_pRibbonBar)
	{
		m_pRibbonBar->Redraw();
	}
#endif
}

AFX_INLINE void SetVisibleAndEnabled(CXTPControl* pControl, BOOL bVisible, BOOL bEnabled)
{
	pControl->SetHideFlag(xtpHideGeneric, !bVisible);
	pControl->SetEnabled(bEnabled);
}

void CXTPOffice2007FrameHook::RepositionCaptionButtons()
{
	CXTPWindowRect rc(m_hwndSite);
	rc.OffsetRect(-rc.TopLeft());

	DWORD dwStyle = GetSiteStyle(FALSE);
	DWORD dwExStyle = GetSiteStyle(TRUE);

	BOOL bToolWindow = (dwExStyle & WS_EX_TOOLWINDOW) == WS_EX_TOOLWINDOW;
	BOOL bMinimized = dwStyle & WS_MINIMIZE;
	BOOL bEnableClose = TRUE;
	BOOL bMaximized = (dwStyle & WS_MAXIMIZE) == WS_MAXIMIZE;
	BOOL bSysMenu = (dwStyle & WS_SYSMENU);
	BOOL bEnabledMaximize = ((dwStyle & WS_MAXIMIZEBOX) == WS_MAXIMIZEBOX);
	BOOL bEnabledMinimize = ((dwStyle & WS_MINIMIZEBOX) == WS_MINIMIZEBOX);
	BOOL bShowMinMaxButtons = !bToolWindow && bSysMenu && (bEnabledMaximize || bEnabledMinimize);
	if (bSysMenu && !bToolWindow)
	{
		CMenu* pMenu = CMenu::FromHandle(::GetSystemMenu(m_hwndSite, FALSE));
		if (pMenu && pMenu->GetMenuState(SC_CLOSE, MF_BYCOMMAND) & (MF_DISABLED | MF_GRAYED)) bEnableClose = FALSE;
	}


	CRect rcCaption(rc.left, rc.top + m_nFrameBorder, rc.right - 4, rc.top + GetCaptionHeight() - 3);

	for (int i = 0; i < m_pCaptionButtons->GetCount(); i++)
	{
		CXTPControl* pControl = m_pCaptionButtons->GetAt(i);

		if (pControl->GetID() == SC_CLOSE)
		{
			SetVisibleAndEnabled(pControl, bSysMenu, bEnableClose);
		}
		if (pControl->GetID() == SC_MAXIMIZE)
		{
			SetVisibleAndEnabled(pControl, !bMaximized && bShowMinMaxButtons, bEnabledMaximize);
		}
		if (pControl->GetID() == SC_RESTORE)
		{
			pControl->SetHelpId(bMinimized? HTMINBUTTON : HTMAXBUTTON);

			if (bMinimized)
				SetVisibleAndEnabled(pControl, bShowMinMaxButtons, bEnabledMinimize);
			else
				SetVisibleAndEnabled(pControl, bMaximized && bShowMinMaxButtons, bEnabledMaximize);
		}
		if (pControl->GetID() == SC_MINIMIZE)
		{
			SetVisibleAndEnabled(pControl, !bMinimized && bShowMinMaxButtons, bEnabledMinimize);
		}
		if (!pControl->IsVisible())
			continue;

		CRect rcButton(rcCaption.right - rcCaption.Height(), rcCaption.top, rcCaption.right, rcCaption.bottom);
		pControl->SetRect(rcButton);

		rcCaption.right -= rcButton.Width();
	}
}

void CXTPOffice2007FrameHook::ScreenToFrame(LPPOINT lpPoint)
{
	CXTPDrawHelpers::ScreenToWindow(GetSite(), lpPoint);
}


void CXTPOffice2007FrameHook::TrackCaptionButton(CXTPControl* pSelected)
{
	CWnd* pSite = GetSite();
	if (!pSite)
		return;

	pSite->SetCapture();
	BOOL bAccept = FALSE;
	m_bButtonPressed = TRUE;

	ASSERT(pSelected);
	if (!pSelected)
		return;

	RedrawFrame();

	while (::GetCapture() == m_hwndSite)
	{
		MSG msg;

		if (!::GetMessage(&msg, NULL, 0, 0))
		{
			AfxPostQuitMessage((int)msg.wParam);
			break;
		}

		if (msg.message == WM_LBUTTONUP)
		{
			bAccept = m_bButtonPressed;
			break;
		}
		else if (msg.message == WM_MOUSEMOVE)
		{
			POINT point = msg.pt;
			ScreenToFrame(&point);

			BOOL bButtonPressed = pSelected->GetRect().PtInRect(point);

			if (bButtonPressed != m_bButtonPressed)
			{
				m_bButtonPressed = bButtonPressed;
				RedrawFrame();
			}
		}
		else
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	m_bButtonPressed = NULL;
	m_pSelected = NULL;

	ReleaseCapture();
	RedrawFrame();

	if (bAccept)
	{
		pSite->SendMessage(WM_SYSCOMMAND, pSelected->GetID());
	}
}

int CXTPOffice2007FrameHook::GetCaptionHeight() const
{
#ifdef _XTP_INCLUDE_RIBBON
	if (IsCaptionVisible() || !m_pRibbonBar)
		return GetFramePaintManager()->GetFrameCaptionHeight() + m_nFrameBorder;

	return m_pRibbonBar->GetCaptionHeight() + m_nFrameBorder;
#else
	return GetFramePaintManager()->GetFrameCaptionHeight() + m_nFrameBorder;
#endif
}

#ifndef WM_DWMCOMPOSITIONCHANGED
#define WM_DWMCOMPOSITIONCHANGED  0x031E
#endif

CXTPControl* CXTPOffice2007FrameHook::HitTestCaptionButton(CPoint point)
{
	CXTPControl* pCaptionButton = m_pCaptionButtons->HitTest(point);
	if (pCaptionButton && !pCaptionButton->GetEnabled()) pCaptionButton = NULL;

	return pCaptionButton;
}


void CXTPOffice2007FrameHook::RefreshFrameStyle()
{
	if (m_bDwmEnabled)
		return;

	DWORD dwStyle = GetSiteStyle();
	DWORD dwStyleRemove = (WS_DLGFRAME | WS_VSCROLL | WS_HSCROLL);

	if (dwStyle & dwStyleRemove)
	{
		m_bLockNCPaint++;
		SetWindowLong(m_hwndSite, GWL_STYLE, dwStyle & ~dwStyleRemove);

		RECT rc = CXTPWindowRect(m_hwndSite);
		::SendMessage(m_hwndSite, WM_NCCALCSIZE, FALSE, (LPARAM)&rc);

		SetWindowLong(m_hwndSite, GWL_STYLE, dwStyle);

		m_bLockNCPaint--;
		RedrawFrame();
	}
}

BOOL CXTPOffice2007FrameHook::GetAutoHideBar() const
{
	APPBARDATA abd;
	ZeroMemory(&abd, sizeof(APPBARDATA));
	abd.cbSize = sizeof(APPBARDATA);

	return (SHAppBarMessage(ABM_GETSTATE, &abd) & ABS_AUTOHIDE) != 0;
}

int CXTPOffice2007FrameHook::OnHookMessage(HWND /*hWnd*/, UINT nMessage, WPARAM& wParam, LPARAM& lParam, LRESULT& lResult)
{
	if (!m_bOffice2007FrameEnabled)
	{
		return FALSE;
	}

	if (!m_bDwmEnabled && m_bLockNCPaint && (nMessage == WM_STYLECHANGING || nMessage == WM_STYLECHANGED
		|| nMessage == WM_WINDOWPOSCHANGED || nMessage == WM_WINDOWPOSCHANGING || nMessage == WM_NCPAINT))
	{
		if (nMessage == WM_WINDOWPOSCHANGING)
			((WINDOWPOS*)lParam)->flags &= ~SWP_FRAMECHANGED;

		return TRUE;
	}

	if (nMessage == WM_WINDOWPOSCHANGING)
	{
		WINDOWPOS* lpwndpos = (WINDOWPOS*)lParam;

		CSize szFrameRegion(lpwndpos->cx, lpwndpos->cy);

		if (((lpwndpos->flags & SWP_NOSIZE) ==  0) && (m_szFrameRegion != szFrameRegion) &&
			((GetSiteStyle() & WS_CHILD) == 0))
		{
			m_nSkipNCPaint = 1;
		}

		return FALSE;
	}

	if (nMessage == CXTPOffice2007FrameHook::m_nMsgQueryFrameHook)
	{
		lResult = (LRESULT)this;
		return TRUE;
	}

	if (nMessage == WM_WINDOWPOSCHANGED)
	{
		WINDOWPOS* lpwndpos = (WINDOWPOS*)lParam;
		if (lpwndpos->flags & SWP_FRAMECHANGED && !m_bInUpdateFrame)
		{
			UpdateFrameRegion();
		}
		return FALSE;
	}

	if (nMessage == WM_SIZE || nMessage == WM_STYLECHANGED)
	{
		CXTPWindowRect rc(m_hwndSite);

		CSize szFrameRegion(rc.Width(), rc.Height());

		if ((m_szFrameRegion != szFrameRegion) || (nMessage == WM_STYLECHANGED))
		{
			UpdateFrameRegion(szFrameRegion, (nMessage == WM_STYLECHANGED));
			RepositionCaptionButtons();
			RedrawFrame();
		}

		if (nMessage == WM_SIZE && wParam == SIZE_MINIMIZED) // Strange Vista bug with both WS_MINIMIZE and WS_MAXIMIZE styles
		{
			if (GetSiteStyle(FALSE) & WS_MAXIMIZE)
			{
				CWnd::ModifyStyle(m_hwndSite, WS_MAXIMIZE, 0, 0);
			}
		}

		return FALSE;
	}

	if (nMessage == WM_NCRBUTTONUP && IsCaptionVisible()&& !m_bDwmEnabled && wParam == HTCAPTION)
	{
		if (m_pSelected)
		{
			m_pSelected = NULL;
			RedrawFrame();
		}

		CPoint point((DWORD)lParam);
		::SendMessage(m_hwndSite, 0x0313, (WPARAM)m_hwndSite, MAKELPARAM(point.x, point.y));
		return TRUE;
	}

	if (nMessage == WM_NCLBUTTONDOWN && IsCaptionVisible() && !m_bDwmEnabled)
	{
		CPoint point((DWORD)lParam);
		ScreenToFrame(&point);

		CXTPControl* pCaptionButton = HitTestCaptionButton(point);
		if (pCaptionButton)
		{
			TrackCaptionButton(pCaptionButton);
			return TRUE;
		}
	}

	if (nMessage == WM_NCMOUSEMOVE && IsCaptionVisible() && !m_bDwmEnabled)
	{
		CPoint point((DWORD)lParam);
		ScreenToFrame(&point);

		CXTPControl* pCaptionButton = HitTestCaptionButton(point);
		if (pCaptionButton != m_pSelected)
		{
			m_pSelected = pCaptionButton;
			RedrawFrame();

			if (m_pSelected)
			{
				::SetTimer (m_hwndSite, XTP_TID_MOUSELEAVE, 50, NULL);
			}
			else
			{
				KillTimer(m_hwndSite, XTP_TID_MOUSELEAVE);
			}
		}
	}

	if (nMessage == WM_TIMER && wParam == XTP_TID_REFRESHFRAME)
	{
		RefreshFrameStyle();
		::KillTimer(m_hwndSite, XTP_TID_REFRESHFRAME);
		return  TRUE;
	}


	if (nMessage == WM_TIMER && wParam == XTP_TID_MOUSELEAVE && !m_bDwmEnabled)
	{
		if (::GetCapture() != NULL)
			return TRUE;

		CPoint pt;
		::GetCursorPos (&pt);
		ScreenToFrame(&pt);

		if (!HitTestCaptionButton(pt))
		{
			if (m_pSelected)
			{
				m_pSelected = NULL;
				RedrawFrame();
			}

			KillTimer(m_hwndSite, XTP_TID_MOUSELEAVE);
		}

		return TRUE;
	}

	if (nMessage == WM_NCHITTEST  && !IsCaptionVisible() && !m_bDwmEnabled)
	{
		CPoint point((DWORD)lParam);
		ScreenToFrame(&point);

		lResult = XTPHookManager()->Default();

		if ((GetSiteStyle(TRUE) & WS_EX_CLIENTEDGE) && (GetSiteStyle() & WS_SIZEBOX)
			&& !(GetSiteStyle() & WS_MINIMIZE))
		{
			if (point.y == m_szFrameRegion.cy - 1 && lResult == HTBORDER)
			{
				lResult = HTBOTTOM;
			}

		}

		return TRUE;
	}

	if (nMessage == WM_NCHITTEST  && !IsCaptionVisible() && m_bDwmEnabled)
	{
		CPoint point((DWORD)lParam);
		ScreenToFrame(&point);

		lResult = XTPHookManager()->Default();
		if (point.y < GetCaptionHeight())
		{
			CXTPWinDwmWrapper().DefWindowProc(m_hwndSite, nMessage, wParam, lParam, &lResult);
		}
		return TRUE;

	}
	if (nMessage == WM_NCHITTEST && IsCaptionVisible() && !m_bDwmEnabled)
	{
		CPoint point((DWORD)lParam);
		ScreenToFrame(&point);

		lResult = XTPHookManager()->Default();
		if (lResult == HTCLOSE || lResult == HTMINBUTTON || lResult == HTMAXBUTTON || lResult == HTHELP)
			lResult = HTCAPTION;

		CXTPControl* pCaptionButton = m_pCaptionButtons->HitTest(point);
		if (pCaptionButton)
		{
			lResult = pCaptionButton->GetHelpId();
			return TRUE;
		}

		if (lResult == HTNOWHERE &&  point.y < GetCaptionHeight())
		{
			if ((point.x < 7 + GetSystemMetrics(SM_CXSMICON)) && (GetSiteStyle() & WS_SYSMENU))
				lResult = HTSYSMENU;
			else
				lResult = HTCAPTION;
		}

		return TRUE;
	}

	if (nMessage == WM_SETTEXT)
	{
		lResult = XTPHookManager()->Default();

		if (IsCaptionVisible())
		{
			RedrawFrame();
		}
		else
		{
			if (m_pRibbonBar)
				RecalcFrameLayout();
		}
		return TRUE;
	}


	if (nMessage == WM_NCACTIVATE && !m_bDwmEnabled)
	{
		BOOL bActive = (BOOL)wParam;
		CWnd* pSite = GetSite();

		// stay active if WF_STAYACTIVE bit is on
		if (pSite->m_nFlags & WF_STAYACTIVE)
			bActive = TRUE;

		// but do not stay active if the window is disabled
		if (!pSite->IsWindowEnabled())
			bActive = FALSE;

		DWORD dwStyle = GetSiteStyle();

		if (dwStyle & WS_VISIBLE)
		{
			m_bLockNCPaint = TRUE;
			if (dwStyle & WS_SIZEBOX)
			{
				CWnd::ModifyStyle(m_hwndSite, WS_SIZEBOX, 0, 0);
			}

			if (pSite->IsWindowEnabled())
				lResult = XTPHookManager()->Default(bActive, 0);
			else
				lResult = XTPHookManager()->Default(wParam, lParam);

			if (dwStyle & WS_SIZEBOX)
			{
				CWnd::ModifyStyle(m_hwndSite, 0, WS_SIZEBOX, 0);
			}
			m_bLockNCPaint = FALSE;

			if (m_bActive != bActive)
			{
				m_bActive = bActive;

				RepositionCaptionButtons();
				RedrawFrame();
				RedrawRibbonBar();
			}
		}
		else
		{
			m_bActive = bActive;
			return FALSE;
		}

		return TRUE;
	}

	if (nMessage == WM_DWMCOMPOSITIONCHANGED)
	{
		BOOL bDwmEnabled = !m_bAllowDwm || GetSiteStyle() & WS_CHILD ? FALSE : CXTPWinDwmWrapper().IsCompositionEnabled();

		if (bDwmEnabled != m_bDwmEnabled)
		{
			m_bDwmEnabled = bDwmEnabled;

			::SetWindowRgn(m_hwndSite, 0, TRUE);

			if (!m_bDwmEnabled)
			{
				RefreshFrameStyle();
			}

			::SetWindowPos(m_hwndSite, 0, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
		}
		return FALSE;
	}

	if (nMessage == WM_NCCALCSIZE && !m_bLockNCPaint)
	{
		NCCALCSIZE_PARAMS FAR* lpncsp = (NCCALCSIZE_PARAMS FAR*)lParam;

		CRect rc(lpncsp->rgrc[0]);

		if (m_bDwmEnabled)
		{
			lResult = XTPHookManager()->Default();
		}
		else
		{
			lpncsp->rgrc[0].left += m_nFrameBorder;
			lpncsp->rgrc[0].top += m_nFrameBorder;
			lpncsp->rgrc[0].right -= m_nFrameBorder;
			lpncsp->rgrc[0].bottom -= m_nFrameBorder;
		}

		DWORD dwStyle = GetSiteStyle();

		if (m_bDwmEnabled)
		{
			if (!IsCaptionVisible())
					lpncsp->rgrc[0].top = rc.top;
		}
		else
		{
			lpncsp->rgrc[0].top += (IsCaptionVisible() ? GetCaptionHeight() - m_nFrameBorder :  0);
			if (IsFrameHasStatusBar() && ((dwStyle & WS_MAXIMIZE) == 0))
			{
				int nBorderSize = max(rc.bottom - lpncsp->rgrc[0].bottom - 3, 1);
				lpncsp->rgrc[0].bottom = rc.bottom - nBorderSize;
			}
		}

		if (((dwStyle & (WS_MAXIMIZE | WS_CHILD)) == WS_MAXIMIZE) && GetAutoHideBar())
		{
			lpncsp->rgrc[0].bottom -= 1;
		}

		return TRUE;
	}

	if (nMessage == m_nMsgQuerySkinState)
	{
		lResult = TRUE;
		return TRUE;
	}

	if (nMessage == WM_GETMINMAXINFO && !m_bDwmEnabled)
	{
		lResult = XTPHookManager()->Default();

		MINMAXINFO* lpMMI = (MINMAXINFO*)lParam;

		int yMin = m_nFrameBorder + GetCaptionHeight();
		int xMin = (int)3 * yMin;

		xMin += GetSystemMetrics(SM_CYSIZE) + 2 * GetSystemMetrics(SM_CXEDGE);

		lpMMI->ptMinTrackSize.x = max(lpMMI->ptMinTrackSize.x, xMin);
		lpMMI->ptMinTrackSize.y = max(lpMMI->ptMinTrackSize.y, yMin);

		return TRUE;
	}

	if (nMessage == WM_SYSCOMMAND && !m_bDwmEnabled && wParam == SC_MINIMIZE && GetSiteStyle() & WS_CHILD)
	{
		lResult = XTPHookManager()->Default();

		RedrawFrame();

		return TRUE;
	}

	if (nMessage == WM_NCPAINT && !m_bDwmEnabled)
	{
		if (m_nSkipNCPaint)
		{
			m_nSkipNCPaint--;
			return TRUE;
		}
		if (!IsMDIMaximized())
		{
			RepositionCaptionButtons();

			CWindowDC dc(GetSite());
			GetFramePaintManager()->DrawFrame(&dc, this);
		}
		return TRUE;
	}
	if (nMessage == WM_PRINT && !m_bDwmEnabled)
	{
		lResult = XTPHookManager()->Default();

		if (lParam & PRF_NONCLIENT)
		{
			CDC* pDC = CDC::FromHandle((HDC)wParam);
			GetFramePaintManager()->DrawFrame(pDC, this);
		}

	}

	if (nMessage == WM_IDLEUPDATECMDUI)
	{
		if (m_bDelayReclalcLayout)
		{
			RecalcFrameLayout();
		}
	}

	return FALSE;
}
