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
// BCGPRadialMenu.cpp : implementation file
//

#include "stdafx.h"
#include "BCGPRadialMenu.h"
#include "bcgpmath.h"

#ifndef _BCGSUITE_
#include "BCGPVisualManager.h"
#include "BCGPToolBar.h"
#include "BCGPFrameWnd.h"
#include "BCGPMDIFrameWnd.h"
#include "BCGPRibbonBar.h"
#else
#define BCGPGetParentFrame AFXGetParentFrame
#endif

#include "BCGPDrawManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static inline BOOL IsSystemCommand (UINT uiCmd)
{
	return (uiCmd >= 0xF000 && uiCmd < 0xF1F0);
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPRadialMenu idle update through CBCGPRadialMenuCmdUI class

class CBCGPRadialMenuCmdUI : public CCmdUI
{
public:
	CBCGPRadialMenuCmdUI()
	{
		m_pUpdated = NULL;
	}

	virtual void Enable(BOOL bOn)
	{
		m_bEnableChanged = TRUE;

		ASSERT_VALID (m_pOther);
		ASSERT_VALID (m_pUpdated);

		const BOOL bIsDisabled = !bOn;

		if (m_pUpdated->m_bIsDisabled != bIsDisabled)
		{
			m_pUpdated->m_bIsDisabled = bIsDisabled;
			m_pOther->RedrawWindow();
		}
	}

	virtual void SetCheck(int nCheck)
	{
		ASSERT_VALID (m_pOther);
		ASSERT_VALID (m_pUpdated);

		if (m_pUpdated->m_bIsChecked != (BOOL)nCheck)
		{
			m_pUpdated->m_bIsChecked = (BOOL)nCheck;
			m_pOther->RedrawWindow();
		}
	}

	virtual void SetText(LPCTSTR lpszText)
	{
		ASSERT (lpszText != NULL);
		ASSERT_VALID (m_pOther);
		ASSERT_VALID (m_pUpdated);

		if (lstrcmp (m_pUpdated->m_strText, lpszText) != 0)
		{
			m_pUpdated->m_strText = lpszText;
			m_pOther->RedrawWindow();
		}
	}

	virtual void SetRadio(BOOL bOn = TRUE)
	{
		ASSERT_VALID (m_pUpdated);
		SetCheck (bOn ? 1 : 0);
	}

	CBCGPRadialMenuItem* m_pUpdated;
};

////////////////////////////////////////////////////////////////////////////////
// CBCGPRadialMenuItem

CBCGPRadialMenuItem::CBCGPRadialMenuItem(UINT nID)
{
	CommonInit();

	m_nID = nID;

	UpdateToolTip();
}
//******************************************************************************
CBCGPRadialMenuItem::CBCGPRadialMenuItem()
{
	CommonInit();
}
//******************************************************************************
CBCGPRadialMenuItem::CBCGPRadialMenuItem(const CBCGPRadialMenuItem& src)
{
	CommonInit();

	m_nID = src.m_nID;
	m_hIcon = src.m_hIcon;
	m_bIsLargeIcon = src.m_bIsLargeIcon;
	m_nImageIndex = src.m_nImageIndex;
	m_ptCenter = src.m_ptCenter;
	m_bIsDisabled = src.m_bIsDisabled;
	m_bIsChecked = src.m_bIsChecked;
	m_bDestroyIcon = src.m_bDestroyIcon;
	m_bIsCenter = src.m_bIsCenter;
	m_strText = src.m_strText;
	m_strToolTip = src.m_strToolTip;
	m_strDescription = src.m_strDescription;
}
//******************************************************************************
void CBCGPRadialMenuItem::CommonInit()
{
	m_nID = 0;
	m_hIcon = NULL;
	m_bIsLargeIcon = FALSE;
	m_nImageIndex = -1;
	m_ptCenter = CBCGPPoint(-1, -1);
	m_bIsDisabled = FALSE;
	m_bIsChecked = FALSE;
	m_bDestroyIcon = FALSE;
	m_bIsCenter = FALSE;
}
//******************************************************************************
CBCGPRadialMenuItem::~CBCGPRadialMenuItem()
{
	if (/*m_bDestroyIcon && */m_hIcon != NULL)
	{
		::DestroyIcon(m_hIcon);
	}
}
//******************************************************************************
void CBCGPRadialMenuItem::UpdateToolTip()
{
	CString strText;
	if (!strText.LoadString (m_nID))
	{
		return;
	}

	m_strToolTip.Empty ();
	m_strDescription.Empty ();

	if (strText.IsEmpty ())
	{
		return;
	}

	AfxExtractSubString (m_strDescription, strText, 0);
	AfxExtractSubString (m_strToolTip, strText, 1, '\n');

	const CString strDummyAmpSeq = _T("\001\001");

	m_strToolTip.Replace (_T("&&"), strDummyAmpSeq);
	m_strToolTip.Remove (_T('&'));
	m_strToolTip.Replace (strDummyAmpSeq, _T("&"));
}
//******************************************************************************
void CBCGPRadialMenuItem::OnDrawIcon(CBCGPGraphicsManager* pGM, BOOL bIsCtrlDisabled, CBCGPImage& icons, CBCGPSize sizeIcon)
{
	ASSERT_VALID(pGM);

	if (m_ptCenter == CBCGPPoint(-1, -1))
	{
		return;
	}

	BOOL bIsDisabled = bIsCtrlDisabled || m_bIsDisabled;
	HICON hIcon = NULL;

	if (m_hIcon != NULL)
	{
		hIcon = m_hIcon;
	}
	else if (m_nImageIndex >= 0)
	{
		CBCGPPoint ptImage((double)bcg_round(m_ptCenter.x - .5 * sizeIcon.cx),
							(double)bcg_round(m_ptCenter.y - .5 * sizeIcon.cy));

		pGM->DrawImage(icons, ptImage, sizeIcon, bIsDisabled ? .4 : 1., CBCGPImage::BCGP_IMAGE_INTERPOLATION_MODE_LINEAR,
			CBCGPRect(CBCGPPoint(sizeIcon.cx * m_nImageIndex, 0), sizeIcon));
	}
	else
	{
		// Try to obtain icon from ribbon/toolbars:
#ifndef _BCGSUITE_
#ifndef BCGP_EXCLUDE_RIBBON
		CFrameWnd* pParentFrame = DYNAMIC_DOWNCAST (CFrameWnd, AfxGetMainWnd());
		CBCGPRibbonBar* pRibbonBar = NULL;

		CBCGPMDIFrameWnd* pMainFrame = DYNAMIC_DOWNCAST (CBCGPMDIFrameWnd, pParentFrame);
		if (pMainFrame != NULL)
		{
			pRibbonBar = pMainFrame->GetRibbonBar();
		}
		else	// Maybe, SDI frame...
		{
			CBCGPFrameWnd* pFrame = DYNAMIC_DOWNCAST (CBCGPFrameWnd, pParentFrame);
			if (pFrame != NULL)
			{
				pRibbonBar = pFrame->GetRibbonBar();
			}
		}

		if (pRibbonBar != NULL)
		{
			ASSERT_VALID(pRibbonBar);

			hIcon = pRibbonBar->ExportImageToIcon(m_nID, FALSE);
		}
		else
#endif
#endif
		{
#ifndef _BCGSUITE_
			int nImage = CImageHash::GetImageOfCommand(m_nID, FALSE);
#else
			int nImage = GetCmdMgr ()->GetCmdImage(m_nID, FALSE);
#endif
			CBCGPToolBarImages* pImages = CBCGPToolBar::GetImages();
			if (pImages != NULL && nImage >= 0)
			{
				hIcon = pImages->ExtractIcon(nImage);
			}
		}

		if (hIcon != NULL)
		{
			m_hIcon = hIcon;
			m_bDestroyIcon = TRUE;
		}
	}

	if (hIcon != NULL)
	{
		CBCGPImage image(hIcon);

		sizeIcon = pGM->GetImageSize(image);
		
		CBCGPPoint ptImage = m_ptCenter;
		ptImage.x -= .5 * sizeIcon.cx;
		ptImage.y -= .5 * sizeIcon.cy;

		pGM->DrawImage(image, ptImage, CBCGPSize(), bIsDisabled ? .4 : 1);
	}
}
//******************************************************************************
void CBCGPRadialMenuItem::OnUpdateCmdUI(CBCGPRadialMenuCmdUI* pCmdUI,
											CFrameWnd* pTarget,
											BOOL bDisableIfNoHndler)
{
	ASSERT_VALID (this);
	ASSERT (pCmdUI != NULL);

	if (m_nID == 0 || IsSystemCommand(m_nID) ||
		m_nID >= AFX_IDM_FIRST_MDICHILD)
	{
		return;
	}

	pCmdUI->m_pUpdated = this;

	pCmdUI->m_nID = m_nID;
	pCmdUI->DoUpdate(pTarget, bDisableIfNoHndler);

	pCmdUI->m_pUpdated = NULL;
}

#define INTERNAL_PART	0.2

////////////////////////////////////////////////////////////////////////////////
// CBCGPRadialMenuObject

CMap<UINT,UINT,CBCGPRadialMenuObject*,CBCGPRadialMenuObject*> CBCGPRadialMenuObject::m_mapAutorepeat;
CCriticalSection CBCGPRadialMenuObject::g_cs;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGPRadialMenuObject::CBCGPRadialMenuObject()
{
	m_nHighlighted = -1;
	m_bIsInteractiveMode = TRUE;
	m_nPressed = -1;
	m_nLastClicked = -1;
	m_pCtrl = NULL;
	m_nAutoRepeatTimeDelay = 0;
	m_nAutoRepeatTimerID = 0;
	m_bIsFirstClick = TRUE;
	m_bHasCenterButton = FALSE;
	m_bIsCloseOnInvoke = TRUE;
	m_cxIcon = 16;
	m_nShadowDepth = 0;
	m_bIsVisualManagerTheme = FALSE;

	SetColorTheme(BCGP_COLOR_THEME_SILVER);

	m_brShadow.SetColors(CBCGPColor(.1, .1, .1, .1), CBCGPColor(1, 1, 1, .1), CBCGPBrush::BCGP_GRADIENT_DIAGONAL_LEFT);
}
//***************************************************************************************
CBCGPRadialMenuObject::~CBCGPRadialMenuObject()
{
	for (int i = 0; i < m_arItems.GetSize(); i++)
	{
		delete m_arItems[i];
	}
}
//***************************************************************************************
void CBCGPRadialMenuObject::SetImageList(UINT uiResID, int cx)
{
	m_Icons.Load(uiResID);

	m_cxIcon = cx;
}
//***************************************************************************************
void CBCGPRadialMenuObject::AddCommand(UINT nCmdID)
{
	CBCGPRadialMenuItem* pItem = new CBCGPRadialMenuItem(nCmdID);
	m_arItems.Add(pItem);

	SetDirty();
}
//***************************************************************************************
void CBCGPRadialMenuObject::AddCommand(UINT nCmdID, int nImageIndex)
{
	CBCGPRadialMenuItem* pItem = new CBCGPRadialMenuItem(nCmdID);
	pItem->m_nImageIndex = nImageIndex;

	m_arItems.Add(pItem);

	SetDirty();
}
//***************************************************************************************
void CBCGPRadialMenuObject::AddCommand(UINT nCmdID, HICON hIcon)
{
	CBCGPRadialMenuItem* pItem = new CBCGPRadialMenuItem(nCmdID);
	pItem->m_hIcon = ::CopyIcon(hIcon);

	m_arItems.Add(pItem);
	SetDirty();
}
//***************************************************************************************
void CBCGPRadialMenuObject::OnDraw(CBCGPGraphicsManager* pGM, const CBCGPRect& /*rectClip*/, DWORD dwFlags)
{
	if (dwFlags == BCGP_DRAW_STATIC)
	{
		return;
	}

	m_nShadowDepth = pGM->IsSupported(BCGP_GRAPHICS_MANAGER_COLOR_OPACITY) ? GetShadowDepth() : 0;

	CBCGPRect rect = m_rect;
	if (rect.Width() < rect.Height())
	{
		rect.top += (rect.Height() - rect.Width()) / 2;
		rect.bottom = rect.top + rect.Width();
	}
	else if (rect.Height() < rect.Width())
	{
		rect.left += (rect.Width() - rect.Height()) / 2;
		rect.right = rect.left + rect.Height();
	}

	rect.DeflateRect(2., 2.);

	rect.right -= m_nShadowDepth;
	rect.bottom -= m_nShadowDepth;

	const double radius = rect.Width() / 2;
	const double radiusSmall = INTERNAL_PART * rect.Width() + 1.0;
	const CBCGPPoint center = rect.CenterPoint();

	CBCGPSize sizeIcon((double)m_cxIcon, 16);

	if (!m_Icons.IsNull())
	{
		sizeIcon.cy = pGM->GetImageSize(m_Icons).cy;
	}

	const int nItems = (int)m_arItems.GetSize();

	if (IsDirty())
	{
		int nCircleItems = m_bHasCenterButton ? nItems - 1 : nItems;

		double dblDeltaAngle = nCircleItems == 0 ? 0. : 360. / nCircleItems;
		double dblStartAngle = 90. - dblDeltaAngle / 2;

		for (int i = 0; i < nItems; i++)
		{
			CBCGPRadialMenuItem* pItem = m_arItems[i];
			ASSERT_VALID(pItem);

			pItem->m_bIsCenter = i == nItems -1 && m_bHasCenterButton;

			pItem->m_Shape.Destroy();
			pItem->m_Shape.Clear();

			if (!pItem->m_bIsCenter)
			{
				double dblFinishAngle = dblStartAngle + dblDeltaAngle;

				const double dblStartAngleRad = bcg_deg2rad(dblStartAngle);
				const double dblFinishAngleRad = bcg_deg2rad(dblFinishAngle);
				const double dblMiddleAngleRad = bcg_deg2rad(dblStartAngle + dblDeltaAngle / 2);

				double angleStartCos = cos(dblStartAngleRad);
				double angleStartSin = sin(dblStartAngleRad);
				double angleFinishCos = cos(dblFinishAngleRad);
				double angleFinishSin = sin(dblFinishAngleRad);

				pItem->m_Shape.SetStart(
					CBCGPPoint(center.x + angleStartCos * radius, center.y - angleStartSin * radius));
				pItem->m_Shape.AddArc(
					CBCGPPoint(center.x + angleFinishCos * radius, center.y - angleFinishSin * radius),
					CBCGPSize(radius, radius), dblStartAngle > dblFinishAngle, FALSE);
				pItem->m_Shape.AddLine(
					CBCGPPoint(center.x + angleFinishCos * radiusSmall, center.y - angleFinishSin * radiusSmall));
				pItem->m_Shape.AddArc(
					CBCGPPoint(center.x + angleStartCos * radiusSmall, center.y - angleStartSin * radiusSmall),
					CBCGPSize(radiusSmall, radiusSmall), dblStartAngle < dblFinishAngle, FALSE);

				pItem->m_ptCenter = CBCGPPoint(
					center.x + cos(dblMiddleAngleRad) * 2 * radius / 3,
					center.y - sin(dblMiddleAngleRad) * 2 * radius / 3);

				dblStartAngle = dblFinishAngle;
			}
			else
			{
				pItem->m_Shape.SetStart(center);
				pItem->m_Shape.AddLine(center);
				pGM->CombineGeometry(pItem->m_Shape, pItem->m_Shape, CBCGPEllipseGeometry(CBCGPEllipse(center, radiusSmall, radiusSmall)), RGN_OR);

				pItem->m_ptCenter = center;
			}
		}
	}

	CBCGPEllipse ellipseInt(center, radiusSmall, radiusSmall);

	CBCGPRect rectShadow = rect;
	rectShadow.OffsetRect(m_nShadowDepth, m_nShadowDepth);

	if (!m_bHasCenterButton && m_pCtrl->GetSafeHwnd() != NULL && (m_pCtrl->GetExStyle() & WS_EX_LAYERED))
	{
		if (m_nShadowDepth > 0)
		{
			CBCGPEllipseGeometry egShadow(rectShadow);

			CBCGPPoint centerShadow = center;
			centerShadow.x += m_nShadowDepth;
			centerShadow.y += m_nShadowDepth;

			CBCGPEllipse ellipseIntShadow(centerShadow, radiusSmall, radiusSmall);
			CBCGPEllipseGeometry egInternalShadow(ellipseIntShadow);

			CBCGPComplexGeometry shapeShadow;
			pGM->CombineGeometry(shapeShadow, egShadow, egInternalShadow, RGN_DIFF);

			pGM->FillGeometry(shapeShadow, m_brShadow);
		}

		CBCGPEllipseGeometry eg(rect);
		CBCGPEllipseGeometry egInternal(ellipseInt);

		CBCGPComplexGeometry shape;
		pGM->CombineGeometry(shape, eg, egInternal, RGN_DIFF);

		pGM->FillGeometry(shape, m_brFill);

	}
	else
	{
		if (m_nShadowDepth > 0)
		{
			pGM->FillEllipse(rectShadow, m_brShadow);
		}

		pGM->FillEllipse(rect, m_brFill);
	}

	pGM->DrawEllipse(rect, m_brBorder);

	if (!pGM->IsSupported(BCGP_GRAPHICS_MANAGER_COLOR_OPACITY))
	{
		CBCGPRect rect1 = rect;
		rect1.DeflateRect(1, 1);

		pGM->DrawEllipse(rect1, m_brFill);
	}

	BOOL bIsCtrlDisabled = m_pCtrl->GetSafeHwnd() != NULL && !m_pCtrl->IsWindowEnabled();

	for (int i = 0; i < nItems; i++)
	{
		CBCGPRadialMenuItem* pItem = m_arItems[i];
		ASSERT_VALID(pItem);

		if (i == m_nHighlighted)
		{
			pGM->FillGeometry(pItem->m_Shape, m_nHighlighted == m_nPressed ? m_brPressed : 
				m_brHighlighted.IsEmpty() ? m_brFill : m_brHighlighted);
		}

		pItem->OnDrawIcon(pGM, bIsCtrlDisabled, m_Icons, sizeIcon);

		pGM->DrawGeometry(pItem->m_Shape, m_brBorder);
	}

	pGM->DrawEllipse(ellipseInt, m_brBorder);

	if (!pGM->IsSupported(BCGP_GRAPHICS_MANAGER_ANTIALIAS))
	{
		rect.InflateRect(1, 1);
		pGM->DrawEllipse(rect, m_brBorder);
	}

	if (m_pCtrl->GetSafeHwnd() != NULL && m_pCtrl->IsFocused() && !m_pCtrl->IsPopup())
	{
		rect.InflateRect(1, 1);
		pGM->DrawEllipse(rect, m_brFocusedBorder);
	}
}
//***************************************************************************************
int CBCGPRadialMenuObject::HitTestShape(const CBCGPPoint& pt)
{
	CBCGPRect rect = m_rect;
	if (rect.Width() < rect.Height())
	{
		rect.top += (rect.Height() - rect.Width()) / 2;
		rect.bottom = rect.top + rect.Width();
	}
	else if (rect.Height() < rect.Width())
	{
		rect.left += (rect.Width() - rect.Height()) / 2;
		rect.right = rect.left + rect.Height();
	}

	rect.DeflateRect(2., 2.);

	rect.right -= m_nShadowDepth;
	rect.bottom -= m_nShadowDepth;

	const double radius = rect.Width() / 2;
	const double radiusSmall = INTERNAL_PART * rect.Width() + 1.0;
	const CBCGPPoint center = rect.CenterPoint();

	double dblDistanceToCenter = bcg_distance(pt, center);

	if (dblDistanceToCenter > radius)
	{
		return -1;
	}
	
	const int nItems = (int)m_arItems.GetSize();
	if (dblDistanceToCenter <= radiusSmall)
	{
		return m_bHasCenterButton ? nItems - 1 : -1;
	}

	const int nCircleItems = m_bHasCenterButton ? nItems - 1 : nItems;
	if (nCircleItems == 0)
	{
		return -1;
	}

	double dblDeltaAngle = 360. / nCircleItems;
	double dblStartAngle = 90. - dblDeltaAngle / 2;
	double dblAngle = bcg_normalize_deg(bcg_rad2deg(acos((pt.x - center.x) / dblDistanceToCenter)));
	if (pt.y > center.y)
	{
		dblAngle = 360 - dblAngle;
	}

	int nHit = (int)((dblAngle - dblStartAngle) / dblDeltaAngle);
	if (dblAngle < dblStartAngle)
	{
		nHit = nCircleItems - 1 - (int)((dblStartAngle - dblAngle) / dblDeltaAngle);
	}

	return nHit;
}
//***************************************************************************************
BOOL CBCGPRadialMenuObject::IsItemDisabled(int nIndex) const
{
	if (nIndex >= 0 && nIndex < m_arItems.GetSize())
	{
		return m_arItems[nIndex]->m_bIsDisabled;
	}

	return FALSE;
}
//***************************************************************************************
void CBCGPRadialMenuObject::OnMouseMove(const CBCGPPoint& pt)
{
	int nHighlightedOld = m_nHighlighted;
	m_nHighlighted = HitTestShape(pt);

	if (IsItemDisabled(m_nHighlighted))
	{
		m_nHighlighted = -1;
	}
	else if (m_nPressed >= 0 && m_nHighlighted != m_nPressed)
	{
		m_nHighlighted = -1;
	}

	if (nHighlightedOld != m_nHighlighted)
	{
		Redraw();
	}
}
//***************************************************************************************
void CBCGPRadialMenuObject::OnMouseLeave()
{
	if (m_nHighlighted >= 0)
	{
		m_nHighlighted = -1;
		Redraw();
	}
}
//***************************************************************************************
BOOL CBCGPRadialMenuObject::OnMouseDown(int nButton, const CBCGPPoint& pt)
{
	if (nButton != 0)
	{
		return FALSE;
	}

	m_bIsFirstClick = FALSE;

	m_nPressed = HitTestShape(pt);
	if (IsItemDisabled(m_nPressed))
	{
		m_nPressed = -1;
	}

	if (m_nPressed >= 0)
	{
		Redraw();

		if (m_nAutoRepeatTimeDelay > 0)
		{
			m_nAutoRepeatTimerID = (UINT) ::SetTimer (NULL, 0, m_nAutoRepeatTimeDelay, AutoRepeatTimerProc);

			g_cs.Lock ();
			m_mapAutorepeat.SetAt (m_nAutoRepeatTimerID, this);
			g_cs.Unlock ();
		}

		return TRUE;
	}

	return FALSE;
}
//***************************************************************************************
void CBCGPRadialMenuObject::OnMouseUp(int /*nButton*/, const CBCGPPoint& /*pt*/)
{
	StopAutorepeatTimer();

	BOOL bSendCommand = (m_nHighlighted == m_nPressed && m_nPressed >= 0 && m_nPressed < m_arItems.GetSize() &&
		m_pCtrl->GetSafeHwnd() != NULL && m_pCtrl->GetOwner() != NULL);

	if (!m_bIsFirstClick && m_bIsCloseOnInvoke && bSendCommand && m_pCtrl != NULL && m_pCtrl->IsPopup() && m_pCtrl->GetSafeHwnd() != NULL)
	{
		m_pCtrl->PostMessage(WM_CLOSE);
	}

	NotifyCommand();

	m_nHighlighted = -1;
	m_nPressed = -1;
	m_nLastClicked = -1;

	Redraw();
}
//***************************************************************************************
BOOL CBCGPRadialMenuObject::NotifyCommand()
{
	if (m_nHighlighted == m_nPressed && m_nPressed >= 0 && m_pCtrl->GetSafeHwnd() != NULL)
	{
		CWnd* pOwner = m_pCtrl->GetOwner();
		if (pOwner != NULL)
		{
			m_nLastClicked = m_nPressed;
			pOwner->PostMessage(WM_COMMAND, m_arItems[m_nPressed]->m_nID);
			return TRUE;
		}
	}

	return FALSE;
}
//***************************************************************************************
void CBCGPRadialMenuObject::OnCancelMode()
{
	if (m_nHighlighted >= 0 || m_nPressed >= 0)
	{
		m_nHighlighted = -1;
		m_nPressed = -1;
		Redraw();
	}

	StopAutorepeatTimer();

	if (m_pCtrl != NULL && m_pCtrl->IsPopup() && m_pCtrl->GetSafeHwnd() != NULL)
	{
		m_pCtrl->PostMessage(WM_CLOSE);
	}
}
//***************************************************************************************
void CBCGPRadialMenuObject::StopAutorepeatTimer()
{
	if (m_nAutoRepeatTimerID != 0)
	{
		::KillTimer(NULL, m_nAutoRepeatTimerID);
		m_nAutoRepeatTimerID = 0;

		g_cs.Lock ();
		m_mapAutorepeat.RemoveKey (m_nAutoRepeatTimerID);
		g_cs.Unlock ();
	}
}
//***************************************************************************************
void CBCGPRadialMenuObject::SetAutorepeatMode (int nTimeDelay)
{
	ASSERT (nTimeDelay >= 0);
	m_nAutoRepeatTimeDelay = nTimeDelay;
}
//***************************************************************************************
VOID CALLBACK CBCGPRadialMenuObject::AutoRepeatTimerProc (HWND /*hwnd*/, UINT /*uMsg*/,
													   UINT_PTR idEvent, DWORD /*dwTime*/)
{
	CBCGPRadialMenuObject* pThis = NULL;

	g_cs.Lock ();

	if (!m_mapAutorepeat.Lookup ((UINT) idEvent, pThis))
	{
		g_cs.Unlock ();
		return;
	}

	ASSERT_VALID (pThis);

	g_cs.Unlock ();

	pThis->NotifyCommand();
}
//***************************************************************************************
BOOL CBCGPRadialMenuObject::OnGetToolTip(const CBCGPPoint& pt, CString& strToolTip, CString& strDescr)
{
	int nHit = HitTestShape(pt);
	if (nHit < 0 || nHit >= m_arItems.GetSize())
	{
		return FALSE;
	}

	strToolTip = m_arItems[nHit]->m_strToolTip;
	strDescr = m_arItems[nHit]->m_strDescription;

	return TRUE;
}
//***************************************************************************************
void CBCGPRadialMenuObject::SetColors(
		const CBCGPColor& clrFill,
		const CBCGPColor& clrBorder,
		const CBCGPColor& clrHighlighted,
		const CBCGPColor& clrPressed,
		BOOL bIsFlat)
{
	if (globalData.IsHighContastMode())
	{
		m_brBorder.SetColor(globalData.clrBarHilite);
		m_brFill.SetColor(globalData.clrBarFace);
		m_brPressed.SetColor(globalData.clrHilite);
		m_brHighlighted.SetColor(globalData.clrHilite);
	}
	else
	{
		m_brBorder.SetColor(clrBorder);

		if (bIsFlat)
		{
			m_brFill.SetColor(clrFill);
			m_brPressed.SetColor(clrPressed);
			m_brHighlighted.SetColor(clrHighlighted);
		}
		else
		{
			m_brFill.SetColors(clrFill, CBCGPColor::White, CBCGPBrush::BCGP_GRADIENT_RADIAL_TOP);
			m_brPressed.SetColors(CBCGPColor::White, clrPressed, CBCGPBrush::BCGP_GRADIENT_RADIAL_CENTER);
			m_brHighlighted.SetColors(clrHighlighted, CBCGPColor::White, CBCGPBrush::BCGP_GRADIENT_RADIAL_TOP);
		}
	}

	m_brFocusedBorder = m_brHighlighted;
}
//***************************************************************************************
void CBCGPRadialMenuObject::SetColorTheme(ColorTheme theme)
{
	m_bIsVisualManagerTheme = FALSE;

	switch (theme)
	{
	case BCGP_COLOR_THEME_VISUAL_MANAGER:
		SetColors(
				globalData.clrBarFace, 
#ifndef _BCGSUITE_
				CBCGPVisualManager::GetInstance()->GetSeparatorColor(), 
				CBCGPVisualManager::GetInstance()->GetHighlightedColor(0), 
				CBCGPVisualManager::GetInstance()->GetHighlightedColor(1),
				CBCGPVisualManager::GetInstance()->IsFlatTheme());
#else
				globalData.clrBarShadow, 
				globalData.clrHilite, 
				globalData.clrHilite);
#endif
		m_bIsVisualManagerTheme = TRUE;
		break;

	case BCGP_COLOR_THEME_BLUE:
		SetColors(
				CBCGPColor::LightSteelBlue, 
				CBCGPColor::LightSteelBlue, 
				CBCGPColor::LightSteelBlue, 
				CBCGPColor::LightSteelBlue);
		break;

	case BCGP_COLOR_THEME_GREEN:
		SetColors(
				CBCGPColor::DarkSeaGreen,
				CBCGPColor::DarkSeaGreen, 
				CBCGPColor::DarkSeaGreen, 
				CBCGPColor::DarkSeaGreen);
		break;

	case BCGP_COLOR_THEME_SILVER:
		SetColors(
				CBCGPColor::Silver, 
				CBCGPColor::Silver, 
				CBCGPColor::Orange, 
				CBCGPColor::DarkOrange);
		break;

	case BCGP_COLOR_THEME_BLACK:
		SetColors(
				CBCGPColor::DarkSlateGray, 
				CBCGPColor::DarkSlateGray, 
				CBCGPColor::DarkSlateGray, 
				CBCGPColor::DarkSlateGray);
		break;
	}
}
//***************************************************************************************
BOOL CBCGPRadialMenuObject::OnKeyboardDown(UINT nChar, UINT /*nRepCnt*/, UINT /*nFlags*/)
{
	int elem = -1;

	switch (nChar)
	{
	case VK_LEFT:
	case VK_NUMPAD4:
		elem = 2;
		break;

	case VK_HOME:
	case VK_NUMPAD7:
		elem = 1;
		break;

	case VK_UP:
	case VK_NUMPAD8:
		elem = 0;
		break;

	case VK_PRIOR:
	case VK_NUMPAD9:
		elem = 3;
		break;

	case VK_RIGHT:
	case VK_NUMPAD6:
		elem = 6;
		break;

	case VK_NEXT:
	case VK_NUMPAD3:
		elem = 5;
		break;

	case VK_DOWN:
	case VK_NUMPAD2:
		elem = 4;
		break;

	case VK_END:
	case VK_NUMPAD1:
		elem = 7;
		break;

	case VK_CLEAR:
	case VK_NUMPAD5:
		elem = 8;
		break;

	case VK_ESCAPE:
		if (m_pCtrl != NULL && m_pCtrl->IsPopup() && m_pCtrl->GetSafeHwnd() != NULL)
		{
			m_pCtrl->PostMessage(WM_CLOSE);
		}
		break;

	default:
		return FALSE;
	}

	if (elem < 0 || elem >= m_arItems.GetSize())
	{
		return FALSE;
	}

	if (IsItemDisabled(elem))
	{
		return TRUE;
	}

	m_nHighlighted = m_nPressed = elem;
	m_nLastClicked = elem;

	Redraw();

	NotifyCommand();

	m_nHighlighted = -1;
	m_nPressed = -1;
	m_nLastClicked = -1;

	Redraw();

	return TRUE;
}
//***************************************************************************************
void CBCGPRadialMenuObject::EnableCenterButton(BOOL bEnable)
{
	m_bHasCenterButton = bEnable;

	if (m_pCtrl->GetSafeHwnd() != NULL)
	{
		if (m_pCtrl->GetExStyle() & WS_EX_LAYERED)
		{
			m_pCtrl->OnDrawLayeredPopup();
		}
		else
		{
			m_pCtrl->SetRgn();
		}
	}

	SetDirty(TRUE, TRUE);
}
//***************************************************************************************
int CBCGPRadialMenuObject::GetShadowDepth() const
{
	if (m_pCtrl->GetSafeHwnd() != NULL && (m_pCtrl->GetExStyle() & WS_EX_LAYERED))
	{
		return 5;
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPRadialMenu

IMPLEMENT_DYNCREATE(CBCGPRadialMenu, CBCGPVisualCtrl)

CBCGPRadialMenu::CBCGPRadialMenu()
{
	m_pRadialMenuObject = NULL;
	m_nDlgCode = DLGC_WANTARROWS | DLGC_WANTCHARS;
	EnableTooltip();
}
//************************************************************************************
CBCGPRadialMenu::~CBCGPRadialMenu()
{
	if (m_pRadialMenuObject != NULL)
	{
		delete m_pRadialMenuObject;
	}
}
//************************************************************************************
BOOL CBCGPRadialMenu::CreatePopup(const CPoint& ptCenter, BYTE nTransparency, BOOL /*bIsTrackingMode*/, CWnd* pWndOwner)
{
	int radius = 4 * GetIconSize();

	CRect rect(CPoint(ptCenter.x - radius, ptCenter.y - radius), CSize(2 * radius, 2 * radius));

	return CBCGPVisualCtrl::CreatePopup(rect, nTransparency, pWndOwner);
}

BEGIN_MESSAGE_MAP(CBCGPRadialMenu, CBCGPVisualCtrl)
	//{{AFX_MSG_MAP(CBCGPRadialMenu)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_IDLEUPDATECMDUI, OnIdleUpdateCmdUI)
	ON_REGISTERED_MESSAGE(BCGM_CHANGEVISUALMANAGER, OnChangeVisualManager)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGPRadialMenu message handlers

void CBCGPRadialMenu::OnSize(UINT nType, int cx, int cy) 
{
	CBCGPVisualCtrl::OnSize(nType, cx, cy);

	if ((GetExStyle() & WS_EX_LAYERED) == 0)
	{
		SetRgn();
	}
}
//************************************************************************************
void CBCGPRadialMenu::SetRgn()
{
	if (m_pRadialMenuObject != NULL)
	{
		CRect rect;
		GetClientRect(rect);

		int cx = rect.Width();
		int cy = rect.Height();

		CRgn rgn;
		rgn.CreateEllipticRgn(0, 0, cx, cy);

		if (!m_pRadialMenuObject->m_bHasCenterButton)
		{
			double rx = INTERNAL_PART * cx;
			double ry = INTERNAL_PART * cy;

			int x1 = (int)(.5 + .5 * cx - rx);
			int x2 = (int)(.5 + .5 * cx + rx);
			int y1 = (int)(.5 +.5 * cy - ry);
			int y2 = (int)(.5 + .5 * cy + ry);

			CRgn rgnCenter;
			rgnCenter.CreateEllipticRgn(x1, y1, x2, y2);

			rgn.CombineRgn(&rgn, &rgnCenter, RGN_XOR);
		}

		SetWindowRgn(rgn, FALSE);
	}
	else
	{
		SetWindowRgn(NULL, FALSE);
	}
}
//************************************************************************************
BOOL CBCGPRadialMenu::IsCloseOnClick(CPoint ptScreen)
{
	if (m_pRadialMenuObject == NULL)
	{
		return CBCGPVisualCtrl::IsCloseOnClick(ptScreen);
	}

	if (m_pRadialMenuObject->m_nPressed >= 0)
	{
		return FALSE;
	}

	CPoint pt = ptScreen;
	ScreenToClient(&pt);

	return m_pRadialMenuObject->HitTestShape(pt) < 0;
}
//************************************************************************************
void CBCGPRadialMenu::OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler)
{
	ASSERT_VALID (this);

	if (m_pRadialMenuObject != NULL)
	{
		CBCGPRadialMenuCmdUI state;
		state.m_pOther = this;

		for (int i = 0; i < (int)m_pRadialMenuObject->m_arItems.GetSize(); i++)
		{
			m_pRadialMenuObject->m_arItems[i]->OnUpdateCmdUI(&state, pTarget, bDisableIfNoHndler);
		}
	}

	// update the dialog controls added to the ribbon
	UpdateDialogControls(pTarget, bDisableIfNoHndler);
}
//************************************************************************************
LRESULT CBCGPRadialMenu::OnIdleUpdateCmdUI(WPARAM wParam, LPARAM)
{
	CFrameWnd* pTarget = (CFrameWnd*)GetOwner();
	if (pTarget == NULL || !pTarget->IsFrameWnd())
		pTarget = BCGPGetParentFrame(this);
	if (pTarget != NULL)
		OnUpdateCmdUI(pTarget, (BOOL)wParam);

	return 0L;
}
//************************************************************************************
int CBCGPRadialMenu::GetIconSize()
{
	if (m_pRadialMenuObject == NULL)
	{
		ASSERT(FALSE);
		return 0;
	}

	return m_pRadialMenuObject->m_cxIcon;
}
//**************************************************************************
LRESULT CBCGPRadialMenu::OnChangeVisualManager (WPARAM, LPARAM)
{
	if (m_pRadialMenuObject != NULL)
	{
		ASSERT_VALID(m_pRadialMenuObject);

		if (m_pRadialMenuObject->m_bIsVisualManagerTheme)
		{
			m_pRadialMenuObject->SetColorTheme(CBCGPRadialMenuObject::BCGP_COLOR_THEME_VISUAL_MANAGER);
			m_pRadialMenuObject->Redraw();
		}
	}

	return 0L;
}
