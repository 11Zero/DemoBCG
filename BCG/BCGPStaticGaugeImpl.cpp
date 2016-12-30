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
// BCGPStaticGaugeImpl.cpp: implementation of the CBCGPStaticGaugeImpl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "bcgcbpro.h"
#include "BCGPStaticGaugeImpl.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

UINT BCGM_ON_GAUGE_CLICK = ::RegisterWindowMessage (_T("BCGM_ON_GAUGE_CLICK"));

IMPLEMENT_DYNAMIC(CBCGPStaticGaugeImpl, CBCGPGaugeImpl)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGPStaticGaugeImpl::CBCGPStaticGaugeImpl(CBCGPVisualContainer* pContainer) :
	CBCGPGaugeImpl(pContainer)
{
	AddData(new CBCGPGaugeDataObject);
	m_nFlashTime = 0;
	m_bOff = FALSE;
	m_bIsPressed = FALSE;
	m_dblOpacity = 1.0;
	m_DefaultDrawFlags = BCGP_DRAW_STATIC;
}
//*******************************************************************************
CBCGPStaticGaugeImpl::~CBCGPStaticGaugeImpl()
{
}
//*******************************************************************************
void CBCGPStaticGaugeImpl::StartFlashing(UINT nTime)
{
	if (IsFlashing() || nTime == 0)
	{
		return;
	}

	m_arData[0]->SetAnimationID((UINT) ::SetTimer (NULL, 0, nTime, AnimTimerProc));

	g_cs.Lock ();
	m_mapAnimations.SetAt (m_arData[0]->GetAnimationID(), this);
	g_cs.Unlock ();

	m_nFlashTime = nTime;
}
//*******************************************************************************
void CBCGPStaticGaugeImpl::StopFlashing()
{
	if (m_arData[0]->GetAnimationID() > 0)
	{
		::KillTimer(NULL, m_arData[0]->GetAnimationID());
		m_arData[0]->SetAnimationID(0);

		m_bOff = FALSE;
		Redraw();
	}
}
//*******************************************************************************
CWnd* CBCGPStaticGaugeImpl::SetOwner(CWnd* pWndOwner, BOOL bRedraw)
{
	BOOL bIsFlashing = IsFlashing();

	CWnd* pWndRes = CBCGPGaugeImpl::SetOwner(pWndOwner, bRedraw);

	if (bIsFlashing)
	{
		StartFlashing(m_nFlashTime);
	}

	return pWndRes;
}
//*******************************************************************************
BOOL CBCGPStaticGaugeImpl::OnMouseDown(int nButton, const CBCGPPoint& pt)
{
	if (!m_bIsInteractiveMode || nButton != 0)
	{
		return CBCGPGaugeImpl::OnMouseDown(nButton, pt);
	}

	m_bIsPressed = TRUE;
	return TRUE;
}
//*******************************************************************************
void CBCGPStaticGaugeImpl::OnMouseUp(int nButton, const CBCGPPoint& pt)
{
	if (!m_bIsInteractiveMode || nButton != 0)
	{
		CBCGPGaugeImpl::OnMouseUp(nButton, pt);
		return;
	}

	if (!m_bIsPressed)
	{
		return;
	}

	m_bIsPressed = FALSE;
	FireClickEvent(pt);
}
//*******************************************************************************
void CBCGPStaticGaugeImpl::OnMouseMove(const CBCGPPoint& pt)
{
	if (!m_bIsPressed)
	{
		CBCGPGaugeImpl::OnMouseMove(pt);
	}
}
//*******************************************************************************
void CBCGPStaticGaugeImpl::OnCancelMode()
{
	if (!m_bIsPressed)
	{
		CBCGPGaugeImpl::OnCancelMode();
		return;
	}

	m_bIsPressed = FALSE;
}
//*******************************************************************************
void CBCGPStaticGaugeImpl::FireClickEvent(const CBCGPPoint& pt)
{
	if (m_pWndOwner->GetSafeHwnd() == NULL)
	{
		return;
	}

	if (globalData.IsAccessibilitySupport() && m_bIsInteractiveMode)
	{
#ifdef _BCGSUITE_
		::NotifyWinEvent (EVENT_OBJECT_STATECHANGE, m_pWndOwner->GetSafeHwnd (), OBJID_CLIENT , CHILDID_SELF);
		::NotifyWinEvent (EVENT_OBJECT_FOCUS, m_pWndOwner->GetSafeHwnd (), OBJID_CLIENT , CHILDID_SELF);
#else
		globalData.NotifyWinEvent (EVENT_OBJECT_STATECHANGE, m_pWndOwner->GetSafeHwnd (), OBJID_CLIENT , CHILDID_SELF);
		globalData.NotifyWinEvent (EVENT_OBJECT_FOCUS, m_pWndOwner->GetSafeHwnd (), OBJID_CLIENT , CHILDID_SELF);
#endif
	}
	
	m_pWndOwner->PostMessage(BCGM_ON_GAUGE_CLICK, (WPARAM)GetID(), MAKELPARAM(pt.x, pt.y));

	CWnd* pOwner = m_pWndOwner->GetOwner();
	if (pOwner->GetSafeHwnd() == NULL)
	{
		return;
	}

	m_pWndOwner->GetOwner()->PostMessage(BCGM_ON_GAUGE_CLICK, (WPARAM)GetID(), MAKELPARAM(pt.x, pt.y));
}
//*******************************************************************************
BOOL CBCGPStaticGaugeImpl::OnSetMouseCursor(const CBCGPPoint& pt)
{
	if (m_bIsInteractiveMode)
	{
		::SetCursor (globalData.GetHandCursor());
		return TRUE;
	}

	return CBCGPGaugeImpl::OnSetMouseCursor(pt);
}
//*******************************************************************************
void CBCGPStaticGaugeImpl::SetFillBrush(const CBCGPBrush& brush)
{
	m_brFill = brush;

	SetDirty();
}
//*******************************************************************************
void CBCGPStaticGaugeImpl::SetOutlineBrush(const CBCGPBrush& brush)
{
	m_brOutline = brush;

	SetDirty();
}
//*******************************************************************************
void CBCGPStaticGaugeImpl::SetOpacity(double opacity, BOOL bRedraw)
{
	m_dblOpacity = opacity;

	if (bRedraw)
	{
		Redraw();
	}
}
//*******************************************************************************
void CBCGPStaticGaugeImpl::SetDefaultDrawFlags(DWORD dwDrawFlags, BOOL bRedraw)
{
	m_DefaultDrawFlags = dwDrawFlags;

	if (bRedraw)
	{
		Redraw();
	}
}
//*******************************************************************************
void CBCGPStaticGaugeImpl::CopyFrom(const CBCGPBaseVisualObject& srcObj)
{
	CBCGPGaugeImpl::CopyFrom(srcObj);

	const CBCGPStaticGaugeImpl& src = (const CBCGPStaticGaugeImpl&)srcObj;

	m_nFlashTime = src.m_nFlashTime;
	m_dblOpacity = src.m_dblOpacity;
	m_brFill = src.m_brFill;
	m_brOutline = src.m_brOutline;
	m_DefaultDrawFlags = src.m_DefaultDrawFlags;
}

//----------------------------------------------------------------------------------
// CBCGPStaticFrameImpl

IMPLEMENT_DYNCREATE(CBCGPStaticFrameImpl, CBCGPStaticGaugeImpl)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGPStaticFrameImpl::CBCGPStaticFrameImpl(CBCGPVisualContainer* pContainer) :
	CBCGPStaticGaugeImpl(pContainer)
{
	m_dblFrameSize = 1.0;
	m_dblCornerRadius = 0.0;

	m_brOutline = CBCGPBrush(CBCGPColor::Black);
}
//*******************************************************************************
CBCGPStaticFrameImpl::~CBCGPStaticFrameImpl()
{
}
//*******************************************************************************
void CBCGPStaticFrameImpl::OnDraw(CBCGPGraphicsManager* pGM, const CBCGPRect& /*rectClip*/, DWORD dwFlags)
{
	ASSERT_VALID(pGM);

	if (m_rect.IsRectEmpty() || !m_bIsVisible)
	{
		return;
	}

	if ((dwFlags & m_DefaultDrawFlags) == 0)
	{
		return;
	}

	CBCGPRect rectDraw = m_rect;
	if (m_bIsSubGauge)
	{
		rectDraw.OffsetRect(-m_ptScrollOffset);
	}
	
	CBCGPRect rect = rectDraw;
	const double scaleRatio = GetScaleRatioMid();

	CBCGPStrokeStyle* pStrokeStyle = m_strokeStyle.IsEmpty() ? NULL : &m_strokeStyle;

	if (m_dblCornerRadius > 0.0)
	{
		CBCGPRoundedRect rectRounded(rect, m_dblCornerRadius * scaleRatio, m_dblCornerRadius * scaleRatio);

		pGM->FillRoundedRectangle(rectRounded, m_brFill);
		pGM->DrawRoundedRectangle(rectRounded, m_brOutline, m_dblFrameSize * scaleRatio, pStrokeStyle);
		
		if (pGM->GetType() == CBCGPGraphicsManager::BCGP_GRAPHICS_MANAGER_GDI)
		{
			rectRounded.rect.DeflateRect(m_sizeScaleRatio.cx, m_sizeScaleRatio.cy);
			pGM->DrawRoundedRectangle(rectRounded, m_brOutline, m_dblFrameSize * scaleRatio, pStrokeStyle);
		}
	}
	else
	{
		pGM->FillRectangle(rect, m_brFill);
		pGM->DrawRectangle(rect, m_brOutline, m_dblFrameSize * scaleRatio, pStrokeStyle);

		if (pGM->GetType() == CBCGPGraphicsManager::BCGP_GRAPHICS_MANAGER_GDI)
		{
			rect.DeflateRect(m_sizeScaleRatio.cx, m_sizeScaleRatio.cy);
			pGM->DrawRectangle(rect, m_brOutline, m_dblFrameSize * scaleRatio, pStrokeStyle);
		}
	}

	SetDirty (FALSE);
}
//*******************************************************************************
void CBCGPStaticFrameImpl::CopyFrom(const CBCGPBaseVisualObject& srcObj)
{
	CBCGPStaticGaugeImpl::CopyFrom(srcObj);

	const CBCGPStaticFrameImpl& src = (const CBCGPStaticFrameImpl&)srcObj;

	m_strokeStyle = src.m_strokeStyle;
	m_dblFrameSize = src.m_dblFrameSize;
	m_dblCornerRadius = src.m_dblCornerRadius;
}
//*******************************************************************************
void CBCGPStaticFrameImpl::SetFrameSize(double dblFrameSize, BOOL bRedraw)
{
	m_dblFrameSize = dblFrameSize;

	if (bRedraw)
	{
		Redraw();
	}
}
//*******************************************************************************
void CBCGPStaticFrameImpl::SetStrokeStyle(const CBCGPStrokeStyle& strokeStyle, BOOL bRedraw)
{
	m_strokeStyle = strokeStyle;

	if (bRedraw)
	{
		Redraw();
	}
}
//*******************************************************************************
void CBCGPStaticFrameImpl::SetCornerRadius(double dblCornerRadius, BOOL bRedraw)
{
	m_dblCornerRadius = dblCornerRadius;
	
	if (bRedraw)
	{
		Redraw();
	}
}

