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
// BCGPShadowManager.cpp: implementation of the CBCGPShadowManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BCGPMath.h"
#include "BCGPShadowManager.h"
#include "Bcgglobals.h"
#include "BCGPGlobalUtils.h"
#include "BCGPDrawManager.h"

#include "BCGPFrameWnd.h"
#include "BCGPMDIFrameWnd.h"
#include "BCGPRibbonBar.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

class CBCGPShadowSideWnd : public CWnd
{
public:	
	enum XPos
	{
		e_PosLeft,
		e_PosTop,
		e_PosRight,
		e_PosBottom
	};

public:
	CBCGPShadowSideWnd(CBCGPShadowManager& rManager, XPos pos)
		: m_rManager(rManager)
		, m_Pos     (pos)
	{
	}

	~CBCGPShadowSideWnd()
	{
	}
	
	void Repos(HDWP hDWP);
	void CreateShadow();
	void DrawShadow(BOOL bUpdateOnly = FALSE);

	UINT HitTest(CPoint point);
	
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPShadowWnd)
public:
	virtual BOOL Create();

protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL
	
	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPShadowWnd)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnWindowPosChanged(WINDOWPOS FAR* lpwndpos);
	afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
	afx_msg BOOL OnNcActivate(BOOL bActive);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	
	CBCGPShadowManager&	m_rManager;
	XPos				m_Pos;
	CSize				m_Size;
	CBitmap				m_Bitmap;
};

BEGIN_MESSAGE_MAP(CBCGPShadowSideWnd, CWnd)
	//{{AFX_MSG_MAP(CBCGPShadowPartWnd)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_WINDOWPOSCHANGED()
	ON_WM_MOUSEACTIVATE()
	ON_WM_NCACTIVATE()
	ON_WM_SETCURSOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CBCGPShadowSideWnd::Create ()
{
	ASSERT_VALID (m_rManager.m_pOwner);

	if (!globalData.IsWindowsLayerSupportAvailable () || globalData.m_nBitsPerPixel <= 8)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	CString strClassName = ::AfxRegisterWndClass (
			CS_SAVEBITS,
			::LoadCursor(NULL, IDC_ARROW),
			(HBRUSH)(COLOR_BTNFACE + 1), NULL);

	CRect rectDummy (0, 0, 0, 0);

	if (!CWnd::CreateEx (WS_EX_TOOLWINDOW | WS_EX_LAYERED, strClassName, _T(""), WS_POPUP, rectDummy, m_rManager.m_pOwner, 0))
	{
		return FALSE;
	}

	SetOwner(m_rManager.m_pOwner);

	m_Size = CSize(0, 0);

	return TRUE;
}

BOOL CBCGPShadowSideWnd::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CBCGPShadowSideWnd::OnWindowPosChanged(WINDOWPOS FAR* lpwndpos) 
{
	CWnd::OnWindowPosChanged (lpwndpos);

	DrawShadow((lpwndpos->flags & SWP_NOSIZE) == SWP_NOSIZE);
}

int CBCGPShadowSideWnd::OnMouseActivate(CWnd* /*pDesktopWnd*/, UINT /*nHitTest*/, UINT /*message*/) 
{
	return MA_NOACTIVATE;
}

BOOL CBCGPShadowSideWnd::OnNcActivate(BOOL /*bActive*/)
{
	return (BOOL)DefWindowProc(WM_NCACTIVATE, FALSE, 0L);
}

BOOL CBCGPShadowSideWnd::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	CPoint point;
	::GetCursorPos(&point);

	UINT nHit = HitTest(point);

	if (nHit == HTNOWHERE || nHit == HTBORDER)
	{
		return CWnd::OnSetCursor(pWnd, nHitTest, message);
	}

	LPCTSTR nID = NULL;

	switch(nHit)
	{
	case HTLEFT:
	case HTRIGHT:
		nID = IDC_SIZEWE;
		break;

	case HTTOP:
	case HTBOTTOM:
		nID = IDC_SIZENS;
		break;

	case HTTOPLEFT:
	case HTBOTTOMRIGHT:
		nID = IDC_SIZENWSE;
		break;

	case HTTOPRIGHT:
	case HTBOTTOMLEFT:
		nID = IDC_SIZENESW;
		break;
	}

	::SetCursor(::LoadCursor(NULL, nID));

	return TRUE;
}

LRESULT CBCGPShadowSideWnd::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_LBUTTONDOWN)
	{
		CPoint point(BCG_GET_X_LPARAM(lParam), BCG_GET_Y_LPARAM(lParam));
		ClientToScreen(&point);

		BCG_GET_X_LPARAM(lParam) = (short)point.x;
		BCG_GET_Y_LPARAM(lParam) = (short)point.y;

		m_rManager.m_pOwner->PostMessage(WM_NCLBUTTONDOWN, (WPARAM)HitTest(point), lParam);
		return 0L;
	}

	return CWnd::WindowProc(message, wParam, lParam);
}

UINT CBCGPShadowSideWnd::HitTest(CPoint point)
{
	if (!IsWindowVisible())
	{
		return HTNOWHERE;
	}

	CRect rect;
	GetWindowRect(&rect);

	if (!rect.PtInRect(point))
	{
		return HTNOWHERE;
	}

	if (!m_rManager.m_bInteracion)
	{
		return HTBORDER;
	}

	UINT nHit = HTNOWHERE;

	CSize szBorders(m_rManager.GetBorderSize());

	switch(m_Pos)
	{
	case e_PosLeft:
	case e_PosRight:
		nHit = m_Pos == e_PosLeft ? HTLEFT : HTRIGHT;

		szBorders.cx *= 4;
		szBorders.cy *= 4;

		if (rect.top <= point.y && point.y <= (rect.top + szBorders.cy))
		{
			nHit = m_Pos == e_PosLeft ? HTTOPLEFT : HTTOPRIGHT;
		}
		else if ((rect.bottom - szBorders.cy) <= point.y && point.y <= rect.bottom)
		{
			nHit = m_Pos == e_PosLeft ? HTBOTTOMLEFT : HTBOTTOMRIGHT;
		}
		break;

	case e_PosTop:
	case e_PosBottom:
		nHit = m_Pos == e_PosTop ? HTTOP : HTBOTTOM;

		szBorders.cx *= 5;
		szBorders.cy *= 5;

		if (rect.left <= point.x && point.x <= (rect.left + szBorders.cx))
		{
			nHit = m_Pos == e_PosTop ? HTTOPLEFT : HTBOTTOMLEFT;
		}
		else if ((rect.right - szBorders.cx) <= point.x && point.x <= rect.right)
		{
			nHit = m_Pos == e_PosTop ? HTTOPRIGHT : HTBOTTOMRIGHT;
		}
		break;
	}

	return nHit;
}

void CBCGPShadowSideWnd::Repos(HDWP hDWP)
{
	ASSERT_VALID (m_rManager.m_pOwner);

	CRect rectWindow;
	m_rManager.m_pOwner->GetWindowRect (rectWindow);

	CSize szBorders(m_rManager.GetBorderSize());

	switch(m_Pos)
	{
	case e_PosLeft:
		rectWindow.right = rectWindow.left;
		rectWindow.left -= szBorders.cx;
		break;

	case e_PosTop:
		rectWindow.bottom = rectWindow.top;
		rectWindow.top -= szBorders.cy;
		rectWindow.left -= szBorders.cx;
		rectWindow.right += szBorders.cx;
		break;

	case e_PosRight:
		rectWindow.left = rectWindow.right;
		rectWindow.right += szBorders.cx;
		break;

	case e_PosBottom:
		rectWindow.top = rectWindow.bottom;
		rectWindow.bottom += szBorders.cy;
		rectWindow.left -= szBorders.cx;
		rectWindow.right += szBorders.cx;
		break;
	}

	DWORD dwFlags = SWP_NOACTIVATE | SWP_NOOWNERZORDER;

	if (!IsWindowVisible ())
	{
		dwFlags |= SWP_SHOWWINDOW;
	}

	if (hDWP == NULL)
	{
		SetWindowPos (m_rManager.m_pOwner, rectWindow.left, rectWindow.top, rectWindow.Width (), rectWindow.Height (),
				dwFlags);
	}
	else
	{
		::DeferWindowPos (hDWP, GetSafeHwnd(), m_rManager.m_pOwner->GetSafeHwnd(), rectWindow.left, rectWindow.top, rectWindow.Width (), rectWindow.Height (),
				dwFlags);

	}
}

void CBCGPShadowSideWnd::CreateShadow()
{
	if (m_Size.cx == 0 || m_Size.cy == 0)
	{
		return;
	}

	BITMAP bmp = {0};
	if (m_Bitmap.GetSafeHandle() != NULL)
	{
		m_Bitmap.GetBitmap(&bmp);
	}

	if (bmp.bmWidth < m_Size.cx || bmp.bmHeight < m_Size.cy)
	{
		LPBYTE pBits = NULL;
		HBITMAP hBitmap = CBCGPDrawManager::CreateBitmap_32 (m_Size, (void**)&pBits);
		if (hBitmap == NULL)
		{
			return;
		}

		m_Bitmap.DeleteObject();
		m_Bitmap.Attach (hBitmap);
	}

	CClientDC clientDC(this);
	CDC dc;
	dc.CreateCompatibleDC (&clientDC);

	CBitmap* pBitmapOld = (CBitmap*)dc.SelectObject (&m_Bitmap);

	CRect rect(CPoint(0, 0), m_Size);
	CSize szBorders(m_rManager.GetBorderSize());

	::FillRect(dc.GetSafeHdc(), rect, (HBRUSH)::GetStockObject(BLACK_BRUSH));

	switch(m_Pos)
	{
	case e_PosLeft:
		rect.right += szBorders.cx * 2 + 1;
		rect.top -= szBorders.cy;
		rect.bottom += szBorders.cy;
		break;

	case e_PosTop:
		rect.bottom += szBorders.cy * 2 + 1;
		break;

	case e_PosRight:
		rect.left -= szBorders.cx * 2 + 1;
		rect.top -= szBorders.cy;
		rect.bottom += szBorders.cy;
		break;

	case e_PosBottom:
		rect.top -= szBorders.cy * 2 + 1;
		break;
	}

	m_rManager.m_Shadow.CBCGPControlRenderer::DrawFrame (&dc, rect, 0, 255);

	dc.SelectObject (pBitmapOld);
}

void CBCGPShadowSideWnd::DrawShadow(BOOL bUpdateOnly/* = FALSE*/)
{
	CRect rect;
	GetWindowRect (rect);

	CSize size (rect.Size ());
	BOOL bSizeChanged = size != m_Size && size.cx != 0 && size.cy != 0;

	if (!bSizeChanged && !bUpdateOnly)
	{
		return;
	}

	if (bSizeChanged)
	{
		if (size.cx != 0 && size.cy != 0)
		{
			m_Size = size;
		}

		CreateShadow();
	}

	if (m_Size.cx == 0 || m_Size.cy == 0)
	{
		return;
	}

	CClientDC clientDC(this);
	CDC dc;
	dc.CreateCompatibleDC (&clientDC);

	CBitmap* pBitmapOld = (CBitmap*)dc.SelectObject (&m_Bitmap);

	BLENDFUNCTION bf;
	bf.BlendOp             = AC_SRC_OVER;
	bf.BlendFlags          = 0;
	bf.SourceConstantAlpha = m_rManager.m_nTransparency;
	bf.AlphaFormat         = LWA_COLORKEY;

	POINT point = {0, 0};
	globalData.UpdateLayeredWindow (GetSafeHwnd (), NULL, 0, &m_Size, dc.GetSafeHdc (), 
		&point, 0, &bf, 0x02);

	dc.SelectObject (pBitmapOld);
}


CBCGPShadowManager::CBCGPShadowManager(CWnd* pOwner, BOOL bInteraction)
	: m_pOwner			(pOwner)
	, m_bInteracion		(bInteraction)
	, m_bVisible		(FALSE)
	, m_Size			(0, 0)
	, m_nMinBrightness	(0)
	, m_nMaxBrightness	(100)
	, m_dblSmooth		(2.0)
	, m_dblDarkRatio	(0.25)
{
	for(int i = 0; i < 4; i++)
	{
		m_arWnd[i] = NULL;
	}
}

CBCGPShadowManager::~CBCGPShadowManager()
{
	for(int i = 0; i < 4; i++)
	{
		if (m_arWnd[i] != NULL)
		{
			if (m_arWnd[i]->GetSafeHwnd() != NULL && ::IsWindow(m_arWnd[i]->GetSafeHwnd()))
			{
				m_arWnd[i]->DestroyWindow();
			}

			delete m_arWnd[i];
		}
	}
}

BOOL CBCGPShadowManager::Create(const CSize& size, int nMinBrightness, int nMaxBrightness, double dblSmooth, double dblDarkRatio)
{
	BOOL bRes = TRUE;

	m_Size = CSize(0, 0);
	if (size.cx > 0 && size.cy > 0)
	{
		m_Size = size;
	}

	m_nMinBrightness = nMinBrightness >= 0 ? bcg_clamp(nMinBrightness, 0, 100) : 0;
	m_nMaxBrightness = nMaxBrightness >= 0 ? bcg_clamp(nMaxBrightness, 0, 100) : 100;
	m_dblSmooth = dblSmooth != 0.0 ? dblSmooth : 2.0;
	m_dblDarkRatio = dblDarkRatio != 0.0 ? dblDarkRatio : 0.65;

	UpdateBaseColor();

	for(int i = 0; i < 4; i++)
	{
		m_arWnd[i] = new CBCGPShadowSideWnd(*this, (CBCGPShadowSideWnd::XPos)i);
		if (!m_arWnd[i]->Create())
		{
			bRes = FALSE;
			break;
		}
	}

	m_bVisible = TRUE;
	m_nTransparency = 255;

	return bRes;
}

void CBCGPShadowManager::Repos() 
{
	ASSERT_VALID (m_pOwner);

	if (!::IsWindow(m_pOwner->GetSafeHwnd()) || !m_bVisible)
	{
		return;
	}

	if (m_pOwner->IsZoomed() || m_pOwner->IsIconic() || !m_pOwner->IsWindowVisible())
	{
		Show(FALSE);
		return;
	}

	HDWP hDWP = ::BeginDeferWindowPos(4);

	int i = 0;
	for (i = 0; i < 4; i++)
	{
		if (!::IsWindow(m_arWnd[i]->GetSafeHwnd()))
		{
			continue;
		}

		m_arWnd[i]->Repos(hDWP);
	}

	::EndDeferWindowPos(hDWP);
}

void CBCGPShadowManager::UpdateTransparency (BYTE nTransparency)
{
	if (m_nTransparency == nTransparency)
	{
		return;
	}

	m_nTransparency = nTransparency;

	if (!m_bVisible)
	{
		return;
	}

	for(int i = 0; i < 4; i++)
	{
		if (m_arWnd[i]->GetSafeHwnd() != NULL && m_arWnd[i]->IsWindowVisible())
		{
			m_arWnd[i]->DrawShadow(TRUE);
		}
	}
}

void CBCGPShadowManager::UpdateBaseColor(COLORREF clr)
{
	if (clr == (COLORREF)-1)
	{
		clr = CBCGPVisualManager::GetInstance ()->GetFrameShadowColor(TRUE);
	}

	CSize szBorders(GetBorderSize());
	int nDepth = max(szBorders.cx, szBorders.cy) * 2;

	m_Shadow.Create (nDepth, clr, m_nMinBrightness, m_nMaxBrightness, FALSE, m_dblSmooth, m_dblDarkRatio);

	for (int i = 0; i < 4; i++)
	{
		if (m_arWnd[i]->GetSafeHwnd() != NULL)
		{
			m_arWnd[i]->CreateShadow();

			if (m_arWnd[i]->IsWindowVisible())
			{
				m_arWnd[i]->DrawShadow(TRUE);
			}
		}
	}
}

void CBCGPShadowManager::SetVisible(BOOL bVisible)
{
	if (m_bVisible == bVisible)
	{
		return;
	}

	m_bVisible = bVisible;
	Show(m_bVisible);
}

void CBCGPShadowManager::Show(BOOL bShow)
{
	if (bShow)
	{
		Repos();
		return;
	}

	for(int i = 0; i < 4; i++)
	{
		if (m_arWnd[i]->GetSafeHwnd() != NULL && m_arWnd[i]->IsWindowVisible())
		{
			m_arWnd[i]->ShowWindow(SW_HIDE);
		}
	}
}

UINT CBCGPShadowManager::HitTest(CPoint point)
{
	if (!m_bVisible)
	{
		return HTNOWHERE;
	}

	UINT nRet = HTNOWHERE;

	for(int i = 0; i < 4; i++)
	{
		if (m_arWnd[i]->GetSafeHwnd() == NULL)
		{
			continue;
		}

		UINT nHit = m_arWnd[i]->HitTest(point);
		if (nHit != HTNOWHERE)
		{
			nRet = nHit;
			break;
		}
	}

	return nRet;
}


CSize CBCGPShadowManager::GetBorderSize() const
{
	if (m_Size != CSize(0, 0))
	{
		return m_Size;
	}

	CSize size(0, 0);

	if (::IsWindow(m_pOwner->GetSafeHwnd()))
	{
		DWORD dwStyle = m_pOwner->GetStyle();

		BOOL bRibbonCaption = FALSE;
		CBCGPFrameImpl* pImpl = NULL;

		if (DYNAMIC_DOWNCAST(CBCGPFrameWnd, m_pOwner))
		{
#ifndef BCGP_EXCLUDE_RIBBON
			CBCGPRibbonBar* pRibbon = ((CBCGPFrameWnd*)m_pOwner)->GetRibbonBar();
			bRibbonCaption = pRibbon != NULL && pRibbon->IsWindowVisible() && pRibbon->IsReplaceFrameCaption();
#endif
			pImpl = &((CBCGPFrameWnd*)m_pOwner)->m_Impl;
		}
		else if (DYNAMIC_DOWNCAST(CBCGPMDIFrameWnd, m_pOwner))
		{
#ifndef BCGP_EXCLUDE_RIBBON
			CBCGPRibbonBar* pRibbon = ((CBCGPMDIFrameWnd*)m_pOwner)->GetRibbonBar();
			bRibbonCaption = pRibbon != NULL && pRibbon->IsWindowVisible() && pRibbon->IsReplaceFrameCaption();
#endif
			pImpl = &((CBCGPMDIFrameWnd*)m_pOwner)->m_Impl;
		}

		if (bRibbonCaption || (pImpl != NULL && pImpl->IsOwnerDrawCaption()))
		{
			dwStyle |= WS_CAPTION;
		}

		size = globalUtils.GetSystemBorders(dwStyle);
	}

	return size;
}