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
// BCGPWinUITiles.cpp : implementation file
//

#include "stdafx.h"
#include "BCGPWinUITiles.h"
#include "BCGPDrawManager.h"
#include "BCGPMath.h"
#include "BCGPLocalResource.h"
#include "bcgprores.h"

#ifndef _BCGSUITE_
#include "BCGPPopupMenu.h"
#include "RegPath.h"
#include "BCGPRegistry.h"
#include "BCGPTagManager.h"
#else
	CString AFXGetRegPath(LPCTSTR lpszPostFix, LPCTSTR lpszProfileName = NULL);
#endif

#include "BCGPVisualCollector.h"
#include "BCGPVisualConstructor.h"
#include "BCGPTagManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static const CString strWinUITilesProfile = _T("BCGPWinUITiles");

#define REG_SECTION_FMT						_T("%sBCGPWinUITiles-%d")
#define REG_SECTION_FMT_EX					_T("%sBCGPWinUITiles-%d%x")
#define REG_ENTRY_TILES_CUSTOMIZATION_DATA	_T("CustomizationData")

UINT BCGM_ON_CLICK_WINUI_UI_TILE = ::RegisterWindowMessage (_T("BCGM_ON_CLICK_WINUI_UI_TILE"));
UINT BCGM_ON_CLICK_WINUI_GROUP_CAPTION = ::RegisterWindowMessage (_T("BCGM_ON_CLICK_WINUI_GROUP_CAPTION"));
UINT BCGM_ON_CLICK_WINUI_CAPTION_BUTTON = ::RegisterWindowMessage (_T("BCGM_ON_CLICK_WINUI_CAPTION_BUTTON"));
UINT BCGM_ON_CLICK_WINUI_NAV_BACK_BUTTON = ::RegisterWindowMessage (_T("BCGM_ON_CLICK_WINUI_NAV_BACK_BUTTON"));
UINT BCGM_ON_CREATE_WINUI_VIEW = ::RegisterWindowMessage (_T("BCGM_ON_CREATE_WINUI_VIEW"));
UINT BCGM_ON_CLOSE_WINUI_VIEW = ::RegisterWindowMessage (_T("BCGM_ON_CLOSE_WINUI_VIEW"));

CMap<UINT,UINT,CBCGPWinUITile*,CBCGPWinUITile*> CBCGPWinUITile::m_mapAnimations;
CCriticalSection CBCGPWinUITile::g_cs;

#define BADGE_GLYPH_SIZE	15
#define ANIMATION_STEP_TIME	50

IMPLEMENT_DYNAMIC(CBCGPWinUIBaseObject, CObject)
IMPLEMENT_DYNCREATE(CBCGPWinUITile, CBCGPWinUIBaseObject)
IMPLEMENT_DYNCREATE(CBCGPWinUITilesGroupCaption, CBCGPWinUIBaseObject)
IMPLEMENT_DYNCREATE(CBCGPWinUITilesCaptionButton, CBCGPWinUIBaseObject)
IMPLEMENT_DYNCREATE(CBCGPWinUITilesNavigationButton, CBCGPWinUITilesCaptionButton)

/////////////////////////////////////////////////////////////////////////////
// CBCGPWinUIBaseObject

CBCGPWinUIBaseObject::CBCGPWinUIBaseObject()
{
	m_pOwner = NULL;
	m_rect.SetRectEmpty();
	m_dwUserData = 0;
	m_bIsVisible = TRUE;
	m_pRTIView = NULL;
	m_nViewResID = 0;
}
//*****************************************************************************************
LPCTSTR CBCGPWinUIBaseObject::GetAccName() const 
{ 
	CString strName = m_strName;
	if (strName.IsEmpty())
	{
		strName = m_strToolTipText;
	}

	return strName; 
}
//*****************************************************************************************
UINT CBCGPWinUIBaseObject::GetAccState() const
{ 
	ASSERT_VALID(this);

	UINT nState = STATE_SYSTEM_FOCUSABLE | STATE_SYSTEM_SELECTABLE;
	if (m_pOwner != NULL && m_pOwner->m_pSelected == this)
	{
		nState |= STATE_SYSTEM_FOCUSED;
		nState |= STATE_SYSTEM_SELECTED;	
	}

	return nState;
}
//*****************************************************************************************
void CBCGPWinUIBaseObject::SetUserData(DWORD_PTR dwUserData)
{ 
	ASSERT_VALID(this);
	m_dwUserData = dwUserData;
}
//*****************************************************************************************
void CBCGPWinUIBaseObject::SetVisible(BOOL bIsVisible)
{ 
	ASSERT_VALID(this);

	m_bIsVisible = bIsVisible;
	m_rect.SetRectEmpty();

	if (m_pOwner != NULL)
	{
		m_pOwner->SetDirty();
	}
}
//*****************************************************************************************
void CBCGPWinUIBaseObject::SetToolTipText(const CString& strToolTipText)
{
	ASSERT_VALID(this);
	m_strToolTipText = strToolTipText;
}
//*****************************************************************************************
void CBCGPWinUIBaseObject::SetToolTipDescription(const CString& strToolTipDescription)
{
	ASSERT_VALID(this);
	m_strToolTipDescription = strToolTipDescription;
}
//*****************************************************************************************
void CBCGPWinUIBaseObject::Redraw()
{
	ASSERT_VALID(this);

	if (m_pOwner != NULL)
	{
		m_pOwner->Redraw();
	}
}
//*****************************************************************************************
void CBCGPWinUIBaseObject::SetName(const CString& strName) 
{ 
	ASSERT_VALID(this);
	m_strName = strName;
}
//*****************************************************************************************
void CBCGPWinUIBaseObject::SetView(CRuntimeClass* pRTI, UINT nViewResID, LPCTSTR lpszViewTitle)
{
	ASSERT_VALID(this);

	m_pRTIView = pRTI;
	m_nViewResID = nViewResID;
	m_strViewTitle = lpszViewTitle == NULL ? _T("") : lpszViewTitle;
}
//*****************************************************************************************
CWnd* CBCGPWinUIBaseObject::CreateView()
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pOwner);

	if (m_pRTIView == NULL)
	{
		ASSERT(FALSE);
		return NULL;
	}

	CObject* pObjView = m_pRTIView->CreateObject();
	if (pObjView == NULL)
	{
		ASSERT(FALSE);
		return NULL;
	}

	CWnd* pWnd = DYNAMIC_DOWNCAST(CWnd, pObjView);
	if (pWnd == NULL)
	{
		ASSERT(FALSE);
		delete pObjView;
		return NULL;
	}

	CBCGPRect rectView = m_pOwner->GetTilesArea();

	CBCGPVisualContainerCtrl* pContainerCtrl = DYNAMIC_DOWNCAST(CBCGPVisualContainerCtrl, pWnd);
	if (pContainerCtrl != NULL)
	{
		ASSERT_VALID(pContainerCtrl);
		ASSERT_VALID(pContainerCtrl->GetVisualContainer());

		if (m_nViewResID != 0)
		{
			pContainerCtrl->GetVisualContainer()->LoadFromXML(m_nViewResID);
		}

		if (!pContainerCtrl->Create(rectView, m_pOwner->GetOwner(), 1, WS_CHILD))
		{
			ASSERT(FALSE);
			delete pContainerCtrl;
			return NULL;
		}
	}
	else
	{
		CBCGPVisualCtrl* pCtrl = DYNAMIC_DOWNCAST(CBCGPVisualCtrl, pWnd);
		if (pCtrl != NULL)
		{
			if (!pCtrl->Create(rectView, m_pOwner->GetOwner(), 1, WS_CHILD))
			{
				ASSERT(FALSE);
				delete pCtrl;
				return NULL;
			}
		}
		else
		{
			m_pOwner->OnCreateViewWnd(pWnd, this);
		}
	}

	return pWnd;
}
//*****************************************************************************************
BOOL CBCGPWinUIBaseObject::IsDragged() const
{
	ASSERT_VALID(this);

	if (m_pOwner != NULL)
	{
		ASSERT_VALID(m_pOwner);

		return m_pOwner->IsDraggingTile() && m_pOwner->m_pPressed == this;
	}
	
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPWinUITile

CBCGPWinUITile::CBCGPWinUITile()
{
	CommonInit();
}
//*****************************************************************************************
CBCGPWinUITile::CBCGPWinUITile(const CString& strName,
										   BCGP_WINUI_TILE_TYPE type,
										   const CBCGPColor& colorText,
										   const CBCGPBrush& brushBackground,
										   const CBCGPColor& colorBorder)
{
	CommonInit();

	m_strName = strName;
	m_Type = type;
	m_bIsWide = (type == BCGP_TILE_WIDE);
	m_colorText = colorText;
	SetBorderColor(colorBorder);
	m_brushBackground = brushBackground;
}
//*****************************************************************************************
CBCGPWinUITile::CBCGPWinUITile(const CString& strName,
										   BOOL bIsWide,
										   const CBCGPColor& colorText,
										   const CBCGPBrush& brushBackground,
										   const CBCGPColor& colorBorder)
{
	CommonInit();

	m_strName = strName;
	m_Type = bIsWide ? BCGP_TILE_WIDE : BCGP_TILE_REGULAR;
	m_bIsWide = bIsWide;
	m_colorText = colorText;
	SetBorderColor(colorBorder);
	m_brushBackground = brushBackground;
}
//*****************************************************************************************
void CBCGPWinUITile::CommonInit()
{
	m_nAnimationID = 0;
	m_sizePadding = CBCGPSize(15.0, 5.0);
	m_nBadgeNumber = -1;
	m_BadgeGlyph = BCGP_NONE;
	m_nCustomBadgeIndex = -1;
	m_nImportance = 0;
	m_nGroup = 0;
	m_Type = BCGP_TILE_REGULAR;
	m_bIsWide = FALSE;
	m_dblBorderWidth = 2.0;
	m_dblImageOpacity = 1.0;
	m_dblImageOpacityDelta = 0.0;
	m_bStretchImage = FALSE;
}
//*****************************************************************************************
CBCGPWinUITile::~CBCGPWinUITile()
{
	if (m_nAnimationID != 0)
	{
		StopAnimation();
	}
}
//*****************************************************************************************
void CBCGPWinUITile::SetBorderColor(const CBCGPColor& color) 
{ 
	m_colorBorderSel = m_colorBorder = color; 

	if (m_colorBorderSel.IsLight())
	{
		m_colorBorderSel.MakeDarker(.5);
	}
	else
	{
		m_colorBorderSel.MakePale();
	}
}
//*****************************************************************************************
CBCGPSize CBCGPWinUITile::GetSize() const
{
	CBCGPSize size;

	if (m_pOwner != NULL)
	{
		ASSERT_VALID(m_pOwner);

		size = m_pOwner->GetSquareSize();
		
		if (m_Type == BCGP_TILE_WIDE || m_Type == BCGP_TILE_DOUBLE_SIZE)
		{
			size.cx *= 2.0;
		}

		if (m_Type == BCGP_TILE_DOUBLE_SIZE && m_pOwner->IsHorizontalLayout())
		{
			size.cy *= 2.0;
		}
	}

	return size;
}
//*****************************************************************************************
void CBCGPWinUITile::DoDraw(CBCGPWinUITiles* pWinUITiles, CBCGPGraphicsManager* pGM, BOOL bIsSelected, BOOL bIsHighlighted)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pGM);
	ASSERT_VALID(pWinUITiles);

	if (m_rect.IsRectEmpty())
	{
		return;
	}

	CBCGPRect rect = m_rect;
	CBCGPRect rectShape = m_rect;

	CBCGPRoundedRect roundedRect(rect, pWinUITiles->m_dblCornerRadius, pWinUITiles->m_dblCornerRadius);

	if (!m_brushBackground.IsEmpty() && m_brushBackgroundDark.IsEmpty())
	{
		m_brushBackgroundDark = m_brushBackground;
		m_brushBackgroundDark.MakeDarker();
	}

	if (bIsHighlighted || bIsSelected)
	{
		const CBCGPBrush& br = !m_brushBackgroundDark.IsEmpty() ? m_brushBackgroundDark : pWinUITiles->GetTileBrushDark();

		if (pWinUITiles->IsRoundedShapes())
		{
			pGM->FillRoundedRectangle(roundedRect, br);
		}
		else
		{
			pGM->FillRectangle(rectShape, br);
		}
	}
	else
	{
		const CBCGPBrush& br = !m_brushBackground.IsEmpty() ? m_brushBackground : pWinUITiles->GetTileBrush();

		if (pWinUITiles->IsRoundedShapes())
		{
			pGM->FillRoundedRectangle(roundedRect, br);
		}
		else
		{
			pGM->FillRectangle(rectShape, br);
		}
	}

	rect.DeflateRect(m_sizePadding.cx, m_sizePadding.cy);

	CBCGPColor clrText = m_colorText;

	if (clrText.IsNull())
	{
		clrText = pWinUITiles->GetTextColor();
	}

	CBCGPSize sizeName = GetNameSize(pGM, pWinUITiles->m_textFormatName, rect.Width());
	CBCGPSize sizeBadge = GetBadgeSize(pGM, pWinUITiles->m_textFormatBadge);
	CBCGPSize sizeHeader = GetHeaderSize(pGM, pWinUITiles->m_textFormatHeader, rect.Width());
	CBCGPSize sizeText = GetTextSize(pGM, pWinUITiles->m_textFormatText, rect.Width());
	
	CBCGPRect rectName = rect;
	rectName.top = rectName.bottom - sizeName.cy;

	double dblBorderWidth = m_dblBorderWidth;
	double dblScale = globalData.GetRibbonImageScale();
	if (dblScale != 1.0)
	{
		dblBorderWidth *= dblScale;
	}

	CBCGPRect rectBadge = rect;
	rectBadge.DeflateRect(0.5 * dblBorderWidth, 0.5 * dblBorderWidth);

	rectBadge.top = rectBadge.bottom - sizeBadge.cy;
	rectBadge.left = rectBadge.right - sizeBadge.cx;

	rect.bottom = min(rectBadge.top, rectName.top);

	CBCGPRect rectHeader = rect;
	rectHeader.bottom = rectHeader.top + sizeHeader.cy;

	CBCGPRect rectText = rect;

	if (sizeHeader.cy == 0)
	{
		rectText.top = rect.top + 2 * m_sizePadding.cy;
	}
	else
	{
		rectText.top = rectHeader.bottom;
	}

	// Draw image:
	
	BOOL bIsFullSizeImage = FALSE;

	CBCGPSize sizeImage = GetImageSize(pGM);
	if (!sizeImage.IsEmpty())
	{
		CBCGPRect rectImage;

		if (sizeImage.cx < rect.Width() && sizeImage.cy < rect.Height())
		{
			rectImage = rect;
			rectImage.bottom = rectName.top;

			if (!m_strHeader.IsEmpty() || !m_strText.IsEmpty())
			{
				rectImage.right = rectImage.left + sizeImage.cx;

				rectHeader.left = rectImage.right + m_sizePadding.cx;
				rectText.left = rectImage.right + m_sizePadding.cx;
			}
		}
		else
		{
			rectImage = m_rect;
			bIsFullSizeImage = TRUE;
		}

		BOOL bClipRoundedRect = FALSE;
		
		if (bIsFullSizeImage && pWinUITiles->IsRoundedShapes())
		{
			pGM->SetClipRoundedRect(roundedRect);
			bClipRoundedRect = TRUE;
		}

		OnDrawImage(pGM, rectImage);

		if (bClipRoundedRect)
		{
			pGM->ReleaseClipArea();
		}
	}

	if ((sizeImage.IsEmpty() || bIsFullSizeImage) && sizeText.IsEmpty())
	{
		rectHeader = rect;
	}

	// Draw texts:
	OnDrawName(pGM, rectName, pWinUITiles->m_textFormatName, clrText);
	OnDrawHeader(pGM, rectHeader, pWinUITiles->m_textFormatHeader, clrText);
	OnDrawText(pGM, rectText, pWinUITiles->m_textFormatText, clrText);

	// Draw badge:
	if (bIsFullSizeImage && !sizeBadge.IsEmpty())
	{
		CBCGPRect rectFillBadge = rectBadge;
		rectFillBadge.InflateRect(4, 3);

		const CBCGPBrush& br = !m_brushBackground.IsEmpty() ? m_brushBackground : pWinUITiles->GetTileBrush();
		pGM->FillRectangle(rectFillBadge, br);
	}

	OnDrawBadge(pGM, rectBadge, pWinUITiles->m_textFormatBadge, clrText);

	if (!m_colorBorder.IsNull() && dblBorderWidth > 0.0)
	{
		if (!bIsFullSizeImage)
		{
			if (pWinUITiles->IsRoundedShapes())
			{
				roundedRect.rect.DeflateRect(dblBorderWidth / 2, dblBorderWidth / 2);
				pGM->DrawRoundedRectangle(roundedRect, CBCGPBrush(m_colorBorder), dblBorderWidth);
			}
			else
			{
				rectShape.DeflateRect(dblBorderWidth / 2, dblBorderWidth / 2);
				pGM->DrawRectangle(rectShape, CBCGPBrush(m_colorBorder), dblBorderWidth);
			}
		}

		if (bIsSelected)
		{
			if (pWinUITiles->IsRoundedShapes())
			{
				roundedRect.rect.DeflateRect(dblBorderWidth / 2, dblBorderWidth / 2);
				pGM->DrawRoundedRectangle(roundedRect, CBCGPBrush(m_colorBorderSel), dblBorderWidth);
			}
			else
			{
				rectShape.DeflateRect(dblBorderWidth / 2, dblBorderWidth / 2);
				pGM->DrawRectangle(rectShape, CBCGPBrush(m_colorBorderSel), dblBorderWidth);
			}
		}
	}
}
//*****************************************************************************************
CBCGPSize CBCGPWinUITile::GetNameSize(CBCGPGraphicsManager* pGM, const CBCGPTextFormat& tf, double dblWidth)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pGM);

	if (m_strName.IsEmpty())
	{
		return CBCGPSize(0., 0.);
	}

	return pGM->GetTextSize(m_strName, tf, dblWidth);
}
//*****************************************************************************************
void CBCGPWinUITile::OnDrawName(CBCGPGraphicsManager* pGM, const CBCGPRect& rectText, const CBCGPTextFormat& tf, const CBCGPColor& clrText)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pGM);

	pGM->DrawText(m_strName, rectText, tf, CBCGPBrush(clrText));
}
//*****************************************************************************************
CBCGPSize CBCGPWinUITile::GetHeaderSize(CBCGPGraphicsManager* pGM, const CBCGPTextFormat& tf, double dblWidth)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pGM);

	if (m_strHeader.IsEmpty())
	{
		return CBCGPSize(0., 0.);
	}
	
	return pGM->GetTextSize(m_strHeader, tf, dblWidth);
}
//*****************************************************************************************
void CBCGPWinUITile::OnDrawHeader(CBCGPGraphicsManager* pGM, const CBCGPRect& rectText, const CBCGPTextFormat& tf, const CBCGPColor& clrText)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pGM);

	pGM->DrawText(m_strHeader, rectText, tf, CBCGPBrush(clrText));
}
//*****************************************************************************************
CBCGPSize CBCGPWinUITile::GetTextSize(CBCGPGraphicsManager* pGM, const CBCGPTextFormat& tf, double dblWidth)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pGM);

	if (m_strText.IsEmpty())
	{
		return CBCGPSize(0., 0.);
	}
	
	return pGM->GetTextSize(m_strText, tf, dblWidth);
}
//*****************************************************************************************
void CBCGPWinUITile::OnDrawText(CBCGPGraphicsManager* pGM, const CBCGPRect& rectText, const CBCGPTextFormat& tf, const CBCGPColor& clrText)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pGM);

	pGM->DrawText(m_strText, rectText, tf, CBCGPBrush(clrText));
}
//*****************************************************************************************
CBCGPSize CBCGPWinUITile::GetBadgeSize(CBCGPGraphicsManager* pGM, const CBCGPTextFormat& tf)
{
	if (m_nBadgeNumber >= 0)
	{
		CString strNumber;
		strNumber.Format(_T("%d"), m_nBadgeNumber);
		
		return pGM->GetTextSize(strNumber, tf);
	}
	
	if (m_BadgeGlyph == BCGP_NONE && m_nCustomBadgeIndex < 0)
	{
		return CBCGPSize(0, 0);
	}
	
	CBCGPSize sizeBadge(BADGE_GLYPH_SIZE, BADGE_GLYPH_SIZE);
	
	double dblScale = globalData.GetRibbonImageScale();
	if (dblScale != 1.0)
	{
		sizeBadge.cx *= dblScale;
		sizeBadge.cy *= dblScale;
	}

	return sizeBadge;
}
//*****************************************************************************************
void CBCGPWinUITile::OnDrawBadge(CBCGPGraphicsManager* pGM, const CBCGPRect& rectBadge, const CBCGPTextFormat& tf, const CBCGPColor& clrText)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pGM);
	ASSERT_VALID(m_pOwner);

	if (m_nBadgeNumber >= 0)
	{
		CString strNumber;
		strNumber.Format(_T("%d"), m_nBadgeNumber);

		pGM->DrawText(strNumber, rectBadge, tf, CBCGPBrush(clrText));
		return;
	}

	if (m_BadgeGlyph == BCGP_NONE && m_nCustomBadgeIndex < 0)
	{
		return;
	}

	CBCGPSize sizeIconSrc(BADGE_GLYPH_SIZE, BADGE_GLYPH_SIZE);
	CBCGPSize sizeIcon = sizeIconSrc;
	
	double dblScale = globalData.GetRibbonImageScale();
	if (dblScale != 1.0)
	{
		sizeIcon.cx *= dblScale;
		sizeIcon.cy *= dblScale;
	}
	
	int nImageIndex = m_nCustomBadgeIndex >= 0 ? m_nCustomBadgeIndex : (int)m_BadgeGlyph;
	const CBCGPImage& imageList = m_nCustomBadgeIndex >= 0 ? m_pOwner->m_CustomBadgeGlyphs : m_pOwner->m_BadgeGlyphs;

	pGM->DrawImage(imageList, rectBadge.TopLeft(), sizeIcon, 1.0, CBCGPImage::BCGP_IMAGE_INTERPOLATION_MODE_LINEAR,
		CBCGPRect(CBCGPPoint(sizeIconSrc.cx * nImageIndex, 0), sizeIconSrc));
}
//*****************************************************************************************
CBCGPSize CBCGPWinUITile::GetImageSize(CBCGPGraphicsManager* pGM)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pGM);

	if (m_bStretchImage)
	{
		CBCGPSize sizeTile = GetSize();
		if (!sizeTile.IsEmpty())
		{
			m_Image.Resize(sizeTile);
			return sizeTile;
		}
	}

	return pGM->GetImageSize(m_Image);
}
//*****************************************************************************************
void CBCGPWinUITile::OnDrawImage(CBCGPGraphicsManager* pGM, const CBCGPRect& rectImage)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pGM);

	CBCGPSize sizeImage = GetImageSize(pGM);
	if (!sizeImage.IsEmpty())
	{
		double xOffset = max(0, 0.5 * (rectImage.Width() - sizeImage.cx));
		double yOffset = max(0, 0.5 * (rectImage.Height() - sizeImage.cy));
		
		pGM->DrawImage(m_Image, rectImage.TopLeft() + CBCGPPoint(xOffset, yOffset), CBCGPSize(), m_dblImageOpacity);
	}
}
//*****************************************************************************************
void CBCGPWinUITile::SetGroupID(int nID, BOOL bRecalcLayout)
{
	ASSERT_VALID(this);

	if (m_nGroup == nID)
	{
		return;
	}

	m_nGroup = nID;

	if (bRecalcLayout && m_pOwner != NULL)
	{
		ASSERT_VALID(m_pOwner);
		m_pOwner->SetDirty(TRUE, TRUE);
	}
}
//*****************************************************************************************
void CBCGPWinUITile::SetHeader(const CString& strHeader) 
{ 
	ASSERT_VALID(this);
	m_strHeader = strHeader;
}
//*****************************************************************************************
void CBCGPWinUITile::SetText(const CString& strText) 
{ 
	ASSERT_VALID(this);
	m_strText = strText;
}
//*****************************************************************************************
void CBCGPWinUITile::SetBadgeNumber(int nBadgeNumber)
{
	ASSERT_VALID(this);

	m_nBadgeNumber = nBadgeNumber;

	if (m_nBadgeNumber >= 0)
	{
		m_BadgeGlyph = BCGP_NONE;
		m_nCustomBadgeIndex = -1;
	}
}
//*****************************************************************************************
void CBCGPWinUITile::SetBadgeGlyph(BCGP_WINUI_BADGE_GLYPH glyph)
{
	ASSERT_VALID(this);

	m_BadgeGlyph = glyph;

	if (m_BadgeGlyph != BCGP_NONE)
	{
		m_nBadgeNumber = -1;
		m_nCustomBadgeIndex = -1;
	}
}
//*****************************************************************************************
void CBCGPWinUITile::SetCustomBadgeIndex(int nCustomBadgeIndex)
{
	ASSERT_VALID(this);
	
	m_nCustomBadgeIndex = nCustomBadgeIndex;
	
	if (m_nCustomBadgeIndex >= 0)
	{
		m_nBadgeNumber = -1;
		m_BadgeGlyph = BCGP_NONE;
	}
}
//*****************************************************************************************
void CBCGPWinUITile::ClearBadge()
{
	ASSERT_VALID(this);

	m_nBadgeNumber = -1;
	m_BadgeGlyph = BCGP_NONE;
	m_nCustomBadgeIndex = -1;
}
//*****************************************************************************************
void CBCGPWinUITile::SetImage(const CBCGPImage& image, BCGP_WINUI_IMAGE_EFFECT effect, int nAnimationTime, BOOL bStretch) 
{ 
	ASSERT_VALID(this);

	if (m_nAnimationID != 0)
	{
		StopAnimation();
		Redraw();
	}

	nAnimationTime = max(nAnimationTime, ANIMATION_STEP_TIME);

	m_Image = image;
	m_bStretchImage = bStretch;

	if (effect == BCGP_ANIMATION_NONE || nAnimationTime <= 0 || m_pOwner == NULL)
	{
		return;
	}

	m_dblImageOpacity = 0.0;
	m_dblImageOpacityDelta = 1.0 / ((double) nAnimationTime / ANIMATION_STEP_TIME);

	m_nAnimationID = ((UINT) ::SetTimer (NULL, 0, ANIMATION_STEP_TIME, AnimTimerProc));
	
	g_cs.Lock ();
	m_mapAnimations.SetAt(m_nAnimationID, this);
	g_cs.Unlock ();
}
//*******************************************************************************
BOOL CBCGPWinUITile::OnAnimation()
{
	ASSERT_VALID(this);

	m_dblImageOpacity += m_dblImageOpacityDelta;

	if (m_dblImageOpacity >= 1.0)
	{
		m_dblImageOpacity = 1.0;
		return TRUE;
	}

	return FALSE;
}
//*******************************************************************************
void CBCGPWinUITile::StopAnimation()
{
	ASSERT_VALID(this);

	::KillTimer (NULL, m_nAnimationID);
	
	g_cs.Lock ();
	m_mapAnimations.RemoveKey(m_nAnimationID);
	g_cs.Unlock ();

	m_nAnimationID = 0;
	m_dblImageOpacity = 1.0;
}
//*******************************************************************************
VOID CALLBACK CBCGPWinUITile::AnimTimerProc (HWND /*hwnd*/, UINT /*uMsg*/,
													   UINT_PTR idEvent, DWORD /*dwTime*/)
{
	CBCGPWinUITile* pTile = NULL;

	g_cs.Lock ();
	BOOL bFound = m_mapAnimations.Lookup ((UINT) idEvent, pTile);
	g_cs.Unlock ();

	if (!bFound)
	{
		return;
	}

	ASSERT_VALID(pTile);

	if (pTile->OnAnimation())
	{
		pTile->StopAnimation();
	}

	pTile->Redraw();
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPWinUITiles

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CBCGPWinUITiles, CBCGPBaseVisualObject)

CBCGPWinUITiles::CBCGPWinUITiles()
{
	CBCGPLocalResource locaRes;
	m_BadgeGlyphs.Load(IDR_BCGBARRES_GLYPH_BADGES);

#ifndef _BCGSUITE_
	m_NavigationBack.Load(IDB_BCGBARRES_NAV_BUTTONS_120_W8);
#else
	m_NavigationBack.Load(IDB_BCGBARRES_NAV_BUTTONS_120);
#endif

	m_bIsHorizontalLayout = TRUE;
	m_bRoundedShapes = FALSE;

	m_bTilesDragAndDrop = FALSE;
	m_ptDragTile = CBCGPPoint(-1., -1.);
	m_bDraggedOut = FALSE;

	m_bHasNavigationBackButton = FALSE;

	m_nHorzMargin = 10.;
	m_nVertMargin = 10.;

	m_dblCornerRadius = m_dblCornerRadiusOriginal = 20.0;

	m_dblTotalSize = 0.;

	m_nHorzMarginOriginal = m_nHorzMargin;
	m_nVertMarginOriginal = m_nVertMargin;

	SetFillBrush(CBCGPBrush(CBCGPColor::SteelBlue), FALSE);
	m_strokeFocus.SetDashStyle(CBCGPStrokeStyle::BCGP_DASH_STYLE_DASH);

	m_brCaptionForeground = CBCGPBrush(CBCGPColor::White);

	m_brTileFill = CBCGPBrush(CBCGPColor::Green);
	
	m_brTileFillDark = m_brTileFill;
	m_brTileFillDark.MakeDarker();

	m_colorTileText = CBCGPColor::White;

	m_pHighlighted = NULL;
	m_pPressed = NULL;
	m_pSelected = NULL;

	LOGFONT lf;
	globalData.fontRegular.GetLogFont(&lf);
	
	m_sizeSquareOriginal = m_sizeSquare = CBCGPSize(fabs(12. * lf.lfHeight) + 10, fabs(10. * lf.lfHeight) + 10);

	m_textFormatText = CBCGPTextFormat(lf);
	
	m_textFormatText.SetTextAlignment(CBCGPTextFormat::BCGP_TEXT_ALIGNMENT_LEADING);
	m_textFormatText.SetTextVerticalAlignment(CBCGPTextFormat::BCGP_TEXT_ALIGNMENT_LEADING);
	m_textFormatText.SetWordWrap();
	m_textFormatText.SetClipText();

	m_textFormatName = CBCGPTextFormat(lf);
	
	m_textFormatName.SetTextAlignment(CBCGPTextFormat::BCGP_TEXT_ALIGNMENT_LEADING);
	m_textFormatName.SetTextVerticalAlignment(CBCGPTextFormat::BCGP_TEXT_ALIGNMENT_LEADING);
	m_textFormatName.SetClipText();

	int lfHeightSaved = lf.lfHeight;
	int lfWeightSaved = lf.lfWeight;

	lf.lfHeight = 4 * lf.lfHeight / 3;
	lf.lfWeight = FW_BOLD;

	m_textFormatBadge = CBCGPTextFormat(lf);

	lf.lfWeight = lfWeightSaved;

	m_textFormatBadge.SetTextAlignment(CBCGPTextFormat::BCGP_TEXT_ALIGNMENT_CENTER);
	m_textFormatBadge.SetTextVerticalAlignment(CBCGPTextFormat::BCGP_TEXT_ALIGNMENT_CENTER);
	m_textFormatBadge.SetClipText();

	lf.lfHeight = 2 * lfHeightSaved;

	m_textFormatHeader = CBCGPTextFormat(lf);
	
	m_textFormatHeader.SetTextAlignment(CBCGPTextFormat::BCGP_TEXT_ALIGNMENT_LEADING);
	m_textFormatHeader.SetTextVerticalAlignment(CBCGPTextFormat::BCGP_TEXT_ALIGNMENT_LEADING);
	m_textFormatHeader.SetClipText();

	lf.lfHeight = 3 * lfHeightSaved;
	m_textFormatCaption = CBCGPTextFormat(lf);
	m_textFormatCaption.SetClipText();
	m_textFormatCaption.SetTextVerticalAlignment(CBCGPTextFormat::BCGP_TEXT_ALIGNMENT_CENTER);

	lf.lfHeight = 2 * lfHeightSaved;
	m_textFormatGroupCaption = CBCGPTextFormat(lf);
	m_textFormatGroupCaption.SetClipText();
	m_textFormatGroupCaption.SetTextVerticalAlignment(CBCGPTextFormat::BCGP_TEXT_ALIGNMENT_CENTER);

	lf.lfHeight = 5 * lfHeightSaved / 4;
	m_textFormatCaptionButton = CBCGPTextFormat(lf);
	m_textFormatCaptionButton.SetClipText();
	m_textFormatCaptionButton.SetTextVerticalAlignment(CBCGPTextFormat::BCGP_TEXT_ALIGNMENT_CENTER);

	m_dblCaptionExtraHeight = 0.0;
	m_xLeftCaptionButtons = INT_MIN;
	m_xRightCaptionButtons = 0.0;

	m_pWndCurrView = NULL;
	m_pLastClicked = NULL;
	m_bRestoreSel = FALSE;
}
//*****************************************************************************************
CBCGPWinUITiles::~CBCGPWinUITiles()
{
	RemoveAll();
}
//*****************************************************************************************
void CBCGPWinUITiles::SetCaption(const CString& strCaption, double dblCaptionExtraHeight)
{
	m_strTopLevelCaption = m_strCaption = strCaption;
	m_dblCaptionExtraHeight = dblCaptionExtraHeight;

	SetDirty();
}
//*****************************************************************************************
CBCGPWinUITilesGroupCaption* CBCGPWinUITiles::SetGroupCaption(int nGroupID, LPCTSTR lpszName, 
	const CBCGPColor& color, BOOL bIsClickable, const CBCGPBrush& brGroupFill, const CBCGPBrush& brGroupOutline)
{
	SetDirty();

	CBCGPWinUITilesGroupCaption* pCaption = NULL;

	for (POSITION pos = m_lstCaptions.GetHeadPosition(); pos != NULL;)
	{
		POSITION posSaved = pos;
		
		CBCGPWinUITilesGroupCaption* pListCaption = m_lstCaptions.GetNext(pos);
		ASSERT_VALID(pListCaption);

		if (pListCaption->m_nID == nGroupID)
		{
			if (lpszName == NULL)
			{
				m_lstCaptions.RemoveAt(posSaved);
				delete pListCaption;
				return NULL;
			}
			else
			{
				pCaption = pListCaption;
				break;
			}
		}
	}

	if (lpszName == NULL)
	{
		return NULL;
	}

	if (pCaption == NULL)
	{
		pCaption = OnCreateGroupCaption(nGroupID);
		ASSERT_VALID(pCaption);

		m_lstCaptions.AddTail(pCaption);
	}

	pCaption->m_pOwner = this;
	pCaption->SetName(lpszName);
	pCaption->SetTextColor(color);
	pCaption->m_bIsClickable = bIsClickable;
	pCaption->m_brFillGroup = brGroupFill;
	pCaption->m_brOutlineGroup = brGroupOutline;

	return pCaption;
}
//*****************************************************************************************
CBCGPWinUITilesGroupCaption* CBCGPWinUITiles::OnCreateGroupCaption(int nGroupID)
{
	ASSERT_VALID(this);
	return new CBCGPWinUITilesGroupCaption(nGroupID);
}
//*****************************************************************************************
CBCGPWinUITilesGroupCaption* CBCGPWinUITiles::GetGroupCaption(int nGroupID)
{
	for (POSITION pos = m_lstCaptions.GetHeadPosition(); pos != NULL;)
	{
		CBCGPWinUITilesGroupCaption* pCaption = m_lstCaptions.GetNext(pos);
		ASSERT_VALID(pCaption);
		
		if (pCaption->m_nID == nGroupID)
		{
			return pCaption;
		}
	}

	return NULL;
}
//*****************************************************************************************
void CBCGPWinUITiles::SetHorizontalLayout(BOOL bIsHorizontalLayout)
{
	if (m_bIsHorizontalLayout == bIsHorizontalLayout)
	{
		return;
	}

	m_bIsHorizontalLayout = bIsHorizontalLayout;
	SetDirty();
}
//*****************************************************************************************
void CBCGPWinUITiles::SetRoundedShapes(BOOL bSet)
{
	m_bRoundedShapes = bSet;
}
//*****************************************************************************************
void CBCGPWinUITiles::SetHorzMargin(double nMargin)
{
	m_nHorzMargin = nMargin;
	m_nHorzMarginOriginal = m_nHorzMargin;

	SetDirty();
}
//*****************************************************************************************
void CBCGPWinUITiles::SetVertMargin(double nMargin)
{
	m_nVertMargin = nMargin;
	m_nVertMarginOriginal = m_nVertMargin;

	SetDirty();
}
//*****************************************************************************************
void CBCGPWinUITiles::SetSquareSize(const CBCGPSize& size)
{
	m_sizeSquareOriginal = m_sizeSquare = size;

	SetDirty();
}
//*****************************************************************************************
void CBCGPWinUITiles::Add(CBCGPWinUITile* pTile, int nGroup)
{
	ASSERT_VALID(pTile);

	pTile->m_pOwner = this;
	pTile->m_nGroup = nGroup;

	m_lstTiles.AddTail(pTile);

	AddSorted(pTile);
	SetDirty();
}
//*****************************************************************************************
void CBCGPWinUITiles::AddCaptionButton(CBCGPWinUITilesCaptionButton* pButton)
{
	ASSERT_VALID(pButton);

	pButton->m_pOwner = this;

	m_lstCaptionButtons.AddTail(pButton);
	SetDirty();
}
//*****************************************************************************************
void CBCGPWinUITiles::Remove(CBCGPWinUIBaseObject* pObject, BOOL bDelete)
{
	ASSERT_VALID(pObject);

	if (pObject == m_pLastClicked)
	{
		m_pLastClicked = NULL;
	}

	if (pObject == m_pHighlighted)
	{
		m_pHighlighted = NULL;
	}

	if (pObject == m_pPressed)
	{
		m_pPressed = NULL;
	}

	if (pObject == m_pSelected)
	{
		m_pSelected = NULL;
	}

	CBCGPWinUITile* pTile = DYNAMIC_DOWNCAST(CBCGPWinUITile, pObject);
	if (pTile != NULL)
	{
		POSITION pos = m_lstTiles.Find(pTile);
		if (pos == NULL)
		{
			ASSERT(FALSE);
		}
		else
		{
			m_lstTiles.RemoveAt(pos);
		}

		pos = m_lstTilesSorted.Find(pTile);
		if (pos == NULL)
		{
			ASSERT(FALSE);
		}
		else
		{
			m_lstTilesSorted.RemoveAt(pos);
		}

		// If it was a last tile in group, remove the group caption:
		const int nGroupID = pTile->GetGroupID();
		BOOL bRemoveCaption = TRUE;

		for (pos = m_lstTiles.GetHeadPosition(); pos != NULL;)
		{
			CBCGPWinUITile* pListTile = m_lstTiles.GetNext(pos);
			ASSERT_VALID(pListTile);

			if (pListTile->GetGroupID() == nGroupID)
			{
				bRemoveCaption = FALSE;
				break;
			}
		}

		if (bRemoveCaption)
		{
			SetGroupCaption(nGroupID, NULL);
		}
	}
	else
	{
		CBCGPWinUITilesGroupCaption* pGroupCaption = DYNAMIC_DOWNCAST(CBCGPWinUITilesGroupCaption, pObject);
		if (pGroupCaption != NULL)
		{
			POSITION pos = m_lstCaptions.Find(pGroupCaption);
			if (pos == NULL)
			{
				ASSERT(FALSE);
			}
			else
			{
				m_lstCaptions.RemoveAt(pos);
			}
		}
		else
		{
			CBCGPWinUITilesCaptionButton* pCaptionButton = DYNAMIC_DOWNCAST(CBCGPWinUITilesCaptionButton, pObject);
			if (pCaptionButton != NULL)
			{
				POSITION pos = m_lstCaptionButtons.Find(pCaptionButton);
				if (pos == NULL)
				{
					ASSERT(FALSE);
				}
				else
				{
					m_lstCaptionButtons.RemoveAt(pos);
				}
			}
		}
	}

	if (bDelete)
	{
		delete pObject;
	}

	SetDirty();
}
//*****************************************************************************************
void CBCGPWinUITiles::RemoveTiles()
{
	CBCGPWinUITile* pSelTile = DYNAMIC_DOWNCAST(CBCGPWinUITile, m_pLastClicked);
	if (pSelTile != NULL && m_lstTiles.Find(pSelTile) != NULL)
	{
		m_pLastClicked = NULL;
	}

	while (!m_lstTiles.IsEmpty())
	{
		delete m_lstTiles.RemoveHead();
	}
	
	m_lstTilesSorted.RemoveAll();
	
	while (!m_lstCaptions.IsEmpty())
	{
		delete m_lstCaptions.RemoveHead();
	}

	m_pHighlighted = m_pPressed = m_pSelected = NULL;
	SetDirty();
}
//*****************************************************************************************
void CBCGPWinUITiles::RemoveCaptionButtons()
{
	CBCGPWinUITilesCaptionButton* pSelButton = DYNAMIC_DOWNCAST(CBCGPWinUITilesCaptionButton, m_pLastClicked);
	if (pSelButton != NULL && m_lstCaptionButtons.Find(pSelButton) != NULL)
	{
		m_pLastClicked = NULL;
	}

	while (!m_lstCaptionButtons.IsEmpty())
	{
		delete m_lstCaptionButtons.RemoveHead();
	}
	
	m_pHighlighted = m_pPressed = m_pSelected = NULL;
	SetDirty();
}
//*****************************************************************************************
void CBCGPWinUITiles::RemoveAll()
{
	if (m_pWndCurrView != NULL)
	{
		if (m_pWndCurrView->GetSafeHwnd() != NULL)
		{
			m_pWndCurrView->DestroyWindow();
		}

		delete m_pWndCurrView;
		m_pWndCurrView = NULL;
	}

	while (!m_lstTiles.IsEmpty())
	{
		delete m_lstTiles.RemoveHead();
	}

	m_lstTilesSorted.RemoveAll();

	while (!m_lstCaptions.IsEmpty())
	{
		delete m_lstCaptions.RemoveHead();
	}

	while (!m_lstCaptionButtons.IsEmpty())
	{
		delete m_lstCaptionButtons.RemoveHead();
	}

	m_pLastClicked = m_pHighlighted = m_pPressed = m_pSelected = NULL;
	SetDirty();
}
//*****************************************************************************************
void CBCGPWinUITiles::EnableNavigationBackButton(BOOL bEnable, CRuntimeClass* pRTC)
{
	m_bHasNavigationBackButton = bEnable;

	if (!bEnable)
	{
		for (POSITION posCaptionButton = m_lstCaptionButtons.GetHeadPosition(); posCaptionButton != NULL;)
		{
			POSITION posSaved = posCaptionButton;

			CBCGPWinUITilesCaptionButton* pCaptionButton = m_lstCaptionButtons.GetNext(posCaptionButton);
			ASSERT_VALID(pCaptionButton);

			if (DYNAMIC_DOWNCAST(CBCGPWinUITilesNavigationButton, pCaptionButton) != NULL)
			{
				delete pCaptionButton;
				m_lstCaptionButtons.RemoveAt(posSaved);
			}
		}
	}
	else
	{
		for (POSITION posCaptionButton = m_lstCaptionButtons.GetHeadPosition(); posCaptionButton != NULL;)
		{
			CBCGPWinUITilesCaptionButton* pCaptionButton = m_lstCaptionButtons.GetNext(posCaptionButton);
			ASSERT_VALID(pCaptionButton);

			if (DYNAMIC_DOWNCAST(CBCGPWinUITilesNavigationButton, pCaptionButton) != NULL)
			{
				// Already exist
				return;
			}
		}

		CBCGPWinUITilesNavigationButton* pButton = NULL;
		if (pRTC == NULL)
		{
			pButton = new CBCGPWinUITilesNavigationButton;
		}
		else
		{
			pButton = DYNAMIC_DOWNCAST(CBCGPWinUITilesNavigationButton, pRTC->CreateObject());
		}

		ASSERT_VALID(pButton);
		
		pButton->m_pOwner = this;

		CBCGPLocalResource locaRes;
		
		CString strTooltip;
		strTooltip.LoadString(ID_BCGBARRES_TASKPANE_BACK);

		pButton->SetToolTipText(strTooltip);

		m_lstCaptionButtons.AddHead(pButton);
	}

	SetDirty();
}
//*****************************************************************************************
void CBCGPWinUITiles::RecalcLayout(CBCGPGraphicsManager* pGM)
{
	m_ScrollBar.SetParentVisualObject(this);
	
	m_lstVisibleObjects.RemoveAll();

	for (POSITION posGroup = m_lstCaptions.GetHeadPosition(); posGroup != NULL;)
	{
		CBCGPWinUITilesGroupCaption* pCaption = m_lstCaptions.GetNext(posGroup);
		ASSERT_VALID(pCaption);

		pCaption->m_rect.SetRectEmpty();
		pCaption->m_rectGroup.SetRectEmpty();
	}

	m_dblTotalSize = 0.;

	m_ScrollBar.Reset();

	double dblCaptionHeight = GetCaptionHeight(pGM);

	m_xLeftCaptionButtons = INT_MIN;
	m_xRightCaptionButtons = 0.0;

	if (dblCaptionHeight > 0.)
	{
		m_rectCaption = m_rect;
		m_rectCaption.bottom = m_rectCaption.top + dblCaptionHeight;

		double xLeft = m_rectCaption.left + 2 * m_nHorzMargin;
		double xRight = m_rectCaption.right - 1.0;

		for (POSITION posCaptionButton = m_lstCaptionButtons.GetHeadPosition(); posCaptionButton != NULL;)
		{
			CBCGPWinUITilesCaptionButton* pCaptionButton = m_lstCaptionButtons.GetNext(posCaptionButton);
			ASSERT_VALID(pCaptionButton);

			CBCGPSize sizeButton = pCaptionButton->GetSize(pGM);
			if (sizeButton.IsEmpty())
			{
				pCaptionButton->m_rect.SetRectEmpty();
				continue;
			}

			if (pCaptionButton->IsRightAligned())
			{
				double xButton = xRight - sizeButton.cx;

				if (xButton < m_xRightCaptionButtons)
				{
					pCaptionButton->m_rect.SetRectEmpty();
				}
				else
				{
					pCaptionButton->m_rect = CBCGPRect(
						CBCGPPoint(xButton, m_rectCaption.CenterPoint().y - sizeButton.cy / 2),
						sizeButton);

					xRight -= sizeButton.cx + 1.0;
					m_xLeftCaptionButtons = xRight;
				}
			}
			else
			{
				pCaptionButton->m_rect = CBCGPRect(
					CBCGPPoint(xLeft, m_rectCaption.CenterPoint().y - sizeButton.cy / 2),
					sizeButton);
				
				xLeft += sizeButton.cx + 1.0;
				m_xRightCaptionButtons = xLeft;
			}
		}
	}
	else
	{
		m_rectCaption.SetRectEmpty();

		for (POSITION posCaptionButton = m_lstCaptionButtons.GetHeadPosition(); posCaptionButton != NULL;)
		{
			CBCGPWinUITilesCaptionButton* pCaptionButton = m_lstCaptionButtons.GetNext(posCaptionButton);
			ASSERT_VALID(pCaptionButton);

			pCaptionButton->m_rect.SetRectEmpty();
		}
	}

	m_rectTiles = m_rect;
	m_rectTiles.top += dblCaptionHeight;

	m_rectTiles.DeflateRect(2 * m_nHorzMargin, m_nVertMargin);

	if (m_lstTiles.IsEmpty())
	{
		return;
	}
	
	POSITION pos = NULL;
	int nCurrGroup = 0;
	
	CBCGPWinUITilesGroupCaption* pCurrCaption = GetGroupCaption(nCurrGroup);
	CBCGPWinUITilesGroupCaption* pFirstCaption = pCurrCaption;
	BOOL bIsFirstGroupEmpty = TRUE;

	CBCGPSize sizeGroupCaption = GetGroupCaptionSize(pGM, nCurrGroup);

	double x = m_rectTiles.left;
	double y = m_rectTiles.top + sizeGroupCaption.cy;

	if (!sizeGroupCaption.IsEmpty())
	{
		y += m_nVertMargin;
	}

	double xColumn = x;
	double yRow = y;

	if (pCurrCaption != NULL)
	{
		pCurrCaption->m_rect = CBCGPRect(x, m_rectTiles.top, x, m_rectTiles.top + sizeGroupCaption.cy);
	}

	BOOL bIsOneRow = m_bIsHorizontalLayout && (m_sizeSquare.cy * 2.0 > m_rectTiles.Height() - sizeGroupCaption.cy - m_nVertMargin);

	CBCGPRect rectGroup(x, m_rectTiles.top, x, m_rectTiles.top);

	for (pos = m_lstTilesSorted.GetHeadPosition(); pos != NULL;)
	{
		CBCGPWinUITile* pTile = m_lstTilesSorted.GetNext(pos);
		ASSERT_VALID(pTile);

		if (!pTile->IsVisible())
		{
			pTile->m_rect.SetRectEmpty();
			continue;
		}

		if (pTile->GetGroupID() == 0)
		{
			bIsFirstGroupEmpty = FALSE;
		}

		CBCGPSize sizeTile = pTile->GetSize();

		if (pTile->m_nGroup != nCurrGroup)
		{
			// Start new group
			if (pCurrCaption != NULL)
			{
				pCurrCaption->m_rectGroup = rectGroup;
			}

			nCurrGroup = pTile->m_nGroup;

			pCurrCaption = GetGroupCaption(nCurrGroup);
			sizeGroupCaption = GetGroupCaptionSize(pGM, nCurrGroup);

			if (m_bIsHorizontalLayout)
			{
				if (m_dblTotalSize == 0.)
				{
					x = xColumn + 2 * m_sizeSquare.cx + 4 * m_nHorzMargin;
				}
				else
				{
					x = m_dblTotalSize + 4 * m_nHorzMargin;
				}

				y = m_rectTiles.top;

				rectGroup = CBCGPRect(x, y, x, y);

				if (pCurrCaption != NULL)
				{
					pCurrCaption->m_rect = CBCGPRect(x, m_rectTiles.top, x, m_rectTiles.top + sizeGroupCaption.cy);
					y += sizeGroupCaption.cy;

					if (!sizeGroupCaption.IsEmpty())
					{
						y += m_nVertMargin;
					}
				}

				xColumn = x;
			}
			else
			{
				if (m_dblTotalSize == 0.)
				{
					y = yRow + 2 * m_sizeSquare.cy + 4 * m_nVertMargin;
				}
				else
				{
					y = m_dblTotalSize + 2 * m_nVertMargin;
				}

				x = m_rectTiles.left;

				rectGroup = CBCGPRect(x, y, x, y);

				if (pCurrCaption != NULL)
				{
					pCurrCaption->m_rect = CBCGPRect(CBCGPPoint(x, y), sizeGroupCaption);

					y += sizeGroupCaption.cy;
					
					if (!sizeGroupCaption.IsEmpty())
					{
						y += m_nVertMargin;
					}
				}

				yRow = y;
			}
		}

		if (pTile->IsWide() && m_bIsHorizontalLayout)
		{
			if (x > xColumn && !bIsOneRow)
			{
				x = xColumn;
				y += m_sizeSquare.cy;
			}
		}

		if (m_bIsHorizontalLayout)
		{
			if (!bIsOneRow && y + sizeTile.cy > m_rectTiles.bottom)
			{
				x += 2 * m_sizeSquare.cx;

				y = m_rectTiles.top + sizeGroupCaption.cy;
				if (!sizeGroupCaption.IsEmpty())
				{
					y += m_nVertMargin;
				}

				xColumn = x;
			}
		}
		else
		{
			if (x + sizeTile.cx > m_rectTiles.right && x > m_rectTiles.left)
			{
				y += sizeTile.cy;
				x = m_rectTiles.left;

				yRow = y - m_sizeSquare.cy;
			}
		}

		CBCGPRect rectOld = pTile->m_rect;

		pTile->m_rect = CBCGPRect(CBCGPPoint(x, y), sizeTile);

		if (m_bIsHorizontalLayout)
		{
			m_dblTotalSize = max(m_dblTotalSize, pTile->m_rect.right);
		}
		else
		{
			m_dblTotalSize = max(m_dblTotalSize, pTile->m_rect.bottom);
		}

		pTile->m_rect.right -= m_nHorzMargin;
		pTile->m_rect.bottom -= m_nVertMargin;

		if (pCurrCaption != NULL)
		{
			pCurrCaption->m_rect.right = max(pCurrCaption->m_rect.right, pTile->m_rect.right);
		}

		if (m_bIsHorizontalLayout)
		{
			if ((pTile->IsWide() || x > xColumn) && !bIsOneRow)
			{
				x = xColumn;
				y += sizeTile.cy;
			}
			else
			{
				x += sizeTile.cx;
			}
		}
		else
		{
			x += sizeTile.cx;
		}

		if (rectOld != pTile->m_rect)
		{
			pTile->OnChangeRect(rectOld);
		}

		rectGroup.right = max(rectGroup.right, pTile->m_rect.right);
		rectGroup.bottom = max(rectGroup.bottom, pTile->m_rect.bottom);
	}

	if (bIsFirstGroupEmpty && pFirstCaption != NULL)
	{
		pFirstCaption->m_rect.SetRectEmpty();
	}

	if (pCurrCaption != NULL)
	{
		pCurrCaption->m_rectGroup = rectGroup;
	}

	if (m_dblTotalSize != 0.)
	{
		m_ScrollBar.SetTotal(m_dblTotalSize);
		m_ScrollBar.SetStep(m_bIsHorizontalLayout ? m_sizeSquare.cx : m_sizeSquare.cy);

		ReposScrollBar(pGM);
	}

	if (m_pWndCurrView->GetSafeHwnd() != NULL)
	{
		CRect rectTiles = m_rectTiles;

		m_pWndCurrView->SetWindowPos(NULL, rectTiles.left, rectTiles.top,
			rectTiles.Width(), rectTiles.Height(), SWP_NOZORDER | SWP_NOACTIVATE);
		m_pWndCurrView->RedrawWindow();
	}

	//----------------------------
	// Build visible objects list:
	//----------------------------
	for (pos = m_lstCaptionButtons.GetHeadPosition(); pos != NULL;)
	{
		CBCGPWinUIBaseObject* pListTile = m_lstCaptionButtons.GetNext(pos);
		ASSERT_VALID(pListTile);
		
		if (pListTile->IsVisible() && !pListTile->GetRect().IsRectEmpty())
		{
			m_lstVisibleObjects.AddHead(pListTile);
		}
	}
	
	for (pos = m_lstCaptions.GetHeadPosition(); pos != NULL;)
	{
		CBCGPWinUIBaseObject* pListTile = m_lstCaptions.GetNext(pos);
		ASSERT_VALID(pListTile);
		
		if (pListTile->IsVisible() && !pListTile->GetRect().IsRectEmpty())
		{
			m_lstVisibleObjects.AddHead(pListTile);
		}
	}
	
	for (pos = m_lstTilesSorted.GetHeadPosition(); pos != NULL;)
	{
		CBCGPWinUIBaseObject* pListTile = m_lstTilesSorted.GetNext(pos);
		ASSERT_VALID(pListTile);
		
		if (pListTile->IsVisible() && !pListTile->GetRect().IsRectEmpty())
		{
			m_lstVisibleObjects.AddHead(pListTile);
		}
	}
}
//*****************************************************************************************
void CBCGPWinUITiles::ReposScrollBar(CBCGPGraphicsManager* pGM)
{
	CBCGPRect rectScroll;

	double dblViewSize = m_bIsHorizontalLayout ? m_rect.Width() : m_rect.Height();

	if (m_dblTotalSize > dblViewSize)
	{
		rectScroll = m_rect;

		if (m_bIsHorizontalLayout)
		{
			rectScroll.top = rectScroll.bottom - ::GetSystemMetrics(SM_CYHSCROLL);
		}
		else
		{
			rectScroll.top += GetCaptionHeight(pGM);
			rectScroll.left = rectScroll.right - ::GetSystemMetrics(SM_CXVSCROLL);
		}
	}

	m_ScrollBar.SetHorizontal(m_bIsHorizontalLayout);
	m_ScrollBar.SetRect(rectScroll);
}
//*****************************************************************************************
double CBCGPWinUITiles::GetCaptionHeight(CBCGPGraphicsManager* pGM) const
{
	ASSERT_VALID(pGM);

	double dblHeight = 0.0;

	if (!m_strCaption.IsEmpty())
	{
		CBCGPSize sizeText = pGM->GetTextSize(m_strCaption, m_textFormatCaption);
		dblHeight = sizeText.cy + m_nVertMargin + m_dblCaptionExtraHeight;
	}

	for (POSITION posCaptionButton = m_lstCaptionButtons.GetHeadPosition(); posCaptionButton != NULL;)
	{
		CBCGPWinUITilesCaptionButton* pCaptionButton = m_lstCaptionButtons.GetNext(posCaptionButton);
		ASSERT_VALID(pCaptionButton);

		dblHeight = max(dblHeight, pCaptionButton->GetSize(pGM).cy + m_nVertMargin);
	}

	return dblHeight;
}
//*****************************************************************************************
CBCGPSize CBCGPWinUITiles::GetGroupCaptionSize(CBCGPGraphicsManager* pGM, int nGroup)
{
	CString strGroupName;
	
	CBCGPWinUITilesGroupCaption* pCaption = GetGroupCaption(nGroup);
	if (pCaption != NULL)
	{
		ASSERT_VALID(pCaption);
		strGroupName = pCaption->GetName();
	}
	
	CBCGPSize sizeGroupCaption;
	
	if (!strGroupName.IsEmpty())
	{
		sizeGroupCaption = pGM->GetTextSize(strGroupName, m_textFormatGroupCaption);
	}

	return sizeGroupCaption;
}
//*****************************************************************************************
void CBCGPWinUITiles::OnScroll(CBCGPVisualScrollBar* /*pScrollBar*/, double dblDelta)
{
	if (dblDelta == 0.)
	{
		return;
	}

	for (POSITION pos = m_lstTilesSorted.GetHeadPosition(); pos != NULL;)
	{
		CBCGPWinUITile* pTile = m_lstTilesSorted.GetNext(pos);
		ASSERT_VALID(pTile);

		if (pTile->IsVisible() && !pTile->GetRect().IsRectEmpty())
		{
			if (m_bIsHorizontalLayout)
			{
				pTile->m_rect.OffsetRect(-dblDelta, 0);
			}
			else
			{
				pTile->m_rect.OffsetRect(0, -dblDelta);
			}
		}
	}

	for (POSITION posGroup = m_lstCaptions.GetHeadPosition(); posGroup != NULL;)
	{
		CBCGPWinUITilesGroupCaption* pCaption = m_lstCaptions.GetNext(posGroup);
		ASSERT_VALID(pCaption);

		if (m_bIsHorizontalLayout)
		{
			pCaption->m_rect.OffsetRect(-dblDelta, 0);
			pCaption->m_rectGroup.OffsetRect(-dblDelta, 0);
		}
		else
		{
			pCaption->m_rect.OffsetRect(0, -dblDelta);
			pCaption->m_rectGroup.OffsetRect(0, -dblDelta);
		}
	}
		
	Redraw();
}
//*****************************************************************************************
void CBCGPWinUITiles::OnDraw(CBCGPGraphicsManager* pGM, const CBCGPRect& rectClip, DWORD dwFlags)
{
	ASSERT_VALID(pGM);

	if (IsDirty())
	{
		RecalcLayout(pGM);
		SetDirty(FALSE);
	}

	if ((dwFlags & BCGP_DRAW_DYNAMIC) == 0)
	{
		return;
	}

	pGM->FillRectangle(m_rect, m_brFill);

	POSITION posGroup = NULL;

	// Draw groups:
	for (posGroup = m_lstCaptions.GetHeadPosition(); posGroup != NULL;)
	{
		CBCGPWinUITilesGroupCaption* pCaption = m_lstCaptions.GetNext(posGroup);
		ASSERT_VALID(pCaption);

		CBCGPRect rectGroup = pCaption->m_rectGroup;

		if (!rectGroup.IsRectEmpty())
		{
			rectGroup.InflateRect(m_nHorzMargin, m_nVertMargin);
			OnDrawTilesGroup(pGM, rectGroup, pCaption);
		}
	}

	// Draw tiles:
	for (POSITION pos = m_lstTilesSorted.GetHeadPosition(); pos != NULL;)
	{
		CBCGPWinUITile* pTile = m_lstTilesSorted.GetNext(pos);
		ASSERT_VALID(pTile);

		if (IsDraggingTile() && pTile == m_pPressed)
		{
			continue;
		}

		CBCGPRect rectInter;
		if (pTile->IsVisible() && rectInter.IntersectRect(pTile->m_rect, rectClip))
		{
			pGM->SetClipRect(pTile->m_rect);
			pTile->DoDraw(this, pGM, pTile == m_pSelected, pTile == m_pHighlighted);
			pGM->ReleaseClipArea();
		}
	}

	// Draw group captions:
	for (posGroup = m_lstCaptions.GetHeadPosition(); posGroup != NULL;)
	{
		CBCGPWinUITilesGroupCaption* pCaption = m_lstCaptions.GetNext(posGroup);
		ASSERT_VALID(pCaption);

		CBCGPRect rectInter;
		if (!pCaption->m_rect.IsRectEmpty() && rectInter.IntersectRect(pCaption->m_rect, rectClip))
		{
			pGM->SetClipRect(pCaption->m_rect);
			OnDrawGroupCaption(pGM, pCaption, pCaption == m_pHighlighted, pCaption == m_pPressed);
			pGM->ReleaseClipArea();
		}
	}

	// Draw caption:
	if (!m_rectCaption.IsRectEmpty())
	{
		if (!m_bIsHorizontalLayout && m_ScrollBar.GetOffset() != 0.)
		{
			pGM->SetGradientRectangle(m_rect);
			pGM->FillRectangle(m_rectCaption, m_brFill);
			pGM->SetGradientRectangle(CBCGPRect());
		}

		CBCGPRect rectCaption = m_rectCaption;
		if (m_xLeftCaptionButtons > INT_MIN)
		{
			rectCaption.right = m_xLeftCaptionButtons;
		}

		if (m_xRightCaptionButtons > 0.0)
		{
			rectCaption.left = m_xRightCaptionButtons;
		}
		
		if (rectCaption.right > rectCaption.left)
		{
			OnDrawCaption(pGM, rectCaption);
		}
		
		// Draw caption buttons:
		for (POSITION posCaptionButton = m_lstCaptionButtons.GetHeadPosition(); posCaptionButton != NULL;)
		{
			CBCGPWinUITilesCaptionButton* pCaptionButton = m_lstCaptionButtons.GetNext(posCaptionButton);
			ASSERT_VALID(pCaptionButton);
			
			CBCGPRect rectInter;
			if (!pCaptionButton->m_rect.IsRectEmpty() && rectInter.IntersectRect(pCaptionButton->m_rect, rectClip))
			{
				OnDrawCaptionButton(pGM, pCaptionButton, pCaptionButton == m_pHighlighted, pCaptionButton == m_pPressed);
			}
		}
	}
	
	// Draw scroll bar:
	m_ScrollBar.DoDraw(pGM);

	// Draw dragged tile:
	if (IsDraggingTile())
	{
		CBCGPRect rectTileSaved = m_pPressed->GetRect();
		
		m_pPressed->m_rect = CBCGPRect(m_ptDragTile, rectTileSaved.Size());

		pGM->SetClipRect(m_pPressed->m_rect);
		m_pPressed->DoDraw(this, pGM, TRUE, TRUE);
		pGM->ReleaseClipArea();

		m_pPressed->m_rect = rectTileSaved;
	}
}
//*****************************************************************************************
void CBCGPWinUITiles::OnDrawTilesGroup(CBCGPGraphicsManager* pGM, const CBCGPRect& rectGroup, CBCGPWinUITilesGroupCaption* pCaption)
{
	ASSERT_VALID(pGM);
	ASSERT_VALID(pCaption);

	double dblBorderWidth = globalData.GetRibbonImageScale();

	if (IsRoundedShapes())
	{
		CBCGPRoundedRect roundedRect(rectGroup, m_dblCornerRadius, m_dblCornerRadius);

		if (!pCaption->m_brFillGroup.IsEmpty())
		{
			pGM->FillRoundedRectangle(roundedRect, pCaption->m_brFillGroup);
		}

		if (!pCaption->m_brOutlineGroup.IsEmpty())
		{
			pGM->DrawRoundedRectangle(roundedRect, pCaption->m_brOutlineGroup, dblBorderWidth);
		}
	}
	else
	{
		if (!pCaption->m_brFillGroup.IsEmpty())
		{
			pGM->FillRectangle(rectGroup, pCaption->m_brFillGroup);
		}

		if (!pCaption->m_brOutlineGroup.IsEmpty())
		{
			pGM->DrawRectangle(rectGroup, pCaption->m_brOutlineGroup, dblBorderWidth);
		}
	}
}
//*****************************************************************************************
void CBCGPWinUITiles::OnDrawCaption(CBCGPGraphicsManager* pGM, const CBCGPRect& rectCaption)
{
	ASSERT_VALID(pGM);

	CBCGPRect rectText = rectCaption;
	rectText.DeflateRect(m_nHorzMargin, 0);

	pGM->DrawText(m_strCaption, rectText, m_textFormatCaption, m_brCaptionForeground);
}
//*****************************************************************************************
void CBCGPWinUITiles::OnDrawGroupCaption(CBCGPGraphicsManager* pGM, CBCGPWinUITilesGroupCaption* pCaption,
										 BOOL bIsHighlighted, BOOL bIsPressed)
{
	ASSERT_VALID(pCaption);
	pCaption->DoDraw(this, pGM, bIsPressed, bIsHighlighted);
}
//*****************************************************************************************
void CBCGPWinUITiles::OnDrawCaptionButton(CBCGPGraphicsManager* pGM, CBCGPWinUITilesCaptionButton* pCaptionButton, BOOL bIsHighlighted, BOOL bIsPressed)
{
	ASSERT_VALID(pCaptionButton);
	pCaptionButton->DoDraw(this, pGM, bIsPressed, bIsHighlighted);
}
//*****************************************************************************************
void CBCGPWinUITiles::OnClickAndHoldEvent(UINT nID, const CBCGPPoint& point)
{
	CBCGPBaseVisualObject::OnClickAndHoldEvent(nID, point);

	m_ScrollBar.OnScrollStep(m_ScrollBar.GetNextButton().PtInRect(point));
}
//*****************************************************************************************
BOOL CBCGPWinUITiles::OnKeyboardDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (IsDraggingTile() && nChar != VK_ESCAPE)
	{
		return CBCGPBaseVisualObject::OnKeyboardDown(nChar, nRepCnt, nFlags);
	}

	CBCGPWinUIBaseObject* pNewSelected = NULL;

	switch (nChar)
	{
	case VK_ESCAPE:
		if (IsDraggingTile())
		{
			OnCancelMode();
			Redraw();
		}
		break;

	case VK_LEFT:
		if (m_pSelected == NULL)
		{
			pNewSelected = GetFirstVisibleTile();
		}
		else
		{
			if (IsHorizontalLayout() && m_pSelected->GetRect().left <= m_rectTiles.left + m_sizeSquare.cx)
			{
				m_ScrollBar.OnScrollStep(FALSE);
				m_ScrollBar.OnScrollStep(FALSE);
			}
			
			double x = m_pSelected->GetRect().left - 1;
			double y = m_pSelected->GetRect().CenterPoint().y;

			if (m_pSelected->IsHigh() && m_bIsHorizontalLayout)
			{
				y -= m_sizeSquare.cy / 2;
			}

			while (x > m_rect.left)
			{
				CBCGPWinUITile* pHit = HitTestTile(CBCGPPoint(x, y));
				if (pHit != NULL && pHit != m_pSelected)
				{
					pNewSelected = pHit;
					break;
				}

				x -= m_sizeSquare.cx / 4;
			}
		}
		break;

	case VK_RIGHT:
		if (m_pSelected == NULL)
		{
			pNewSelected = GetFirstVisibleTile();
		}
		else
		{
			if (IsHorizontalLayout() && m_pSelected->GetRect().right >= m_rectTiles.right - m_sizeSquare.cx)
			{
				m_ScrollBar.OnScrollStep(TRUE);
				m_ScrollBar.OnScrollStep(TRUE);
			}

			double x = m_pSelected->GetRect().right + 1;
			double y = m_pSelected->GetRect().CenterPoint().y;
			
			if (m_pSelected->IsHigh() && m_bIsHorizontalLayout)
			{
				y -= m_sizeSquare.cy / 2;
			}

			while (x < m_rect.right)
			{
				CBCGPWinUITile* pHit = HitTestTile(CBCGPPoint(x, y));
				if (pHit != NULL && pHit != m_pSelected)
				{
					pNewSelected = pHit;
					break;
				}

				x += m_sizeSquare.cx / 4;
			}
		}
		break;

	case VK_UP:
		if (m_pSelected == NULL)
		{
			pNewSelected = GetFirstVisibleTile();
		}
		else
		{
			if (!IsHorizontalLayout() && m_pSelected->GetRect().top <= m_rectTiles.top + m_sizeSquare.cy)
			{
				m_ScrollBar.OnScrollStep(FALSE);
				m_ScrollBar.OnScrollStep(FALSE);
			}

			double x = m_pSelected->GetRect().left + m_sizeSquare.cx / 2;
			double y = m_pSelected->GetRect().top - 1;
			
			while (y > m_rect.top)
			{
				CBCGPWinUITile* pHit = HitTestTile(CBCGPPoint(x, y));
				if (pHit != NULL && pHit != m_pSelected)
				{
					pNewSelected = pHit;
					break;
				}
				
				y -= m_sizeSquare.cy / 4;
			}
		}
		break;
		
	case VK_DOWN:
		if (m_pSelected == NULL)
		{
			pNewSelected = GetFirstVisibleTile();
		}
		else
		{
			if (!IsHorizontalLayout() && m_pSelected->GetRect().bottom >= m_rectTiles.bottom - m_sizeSquare.cy)
			{
				m_ScrollBar.OnScrollStep(TRUE);
				m_ScrollBar.OnScrollStep(TRUE);
			}
			
			double x = m_pSelected->GetRect().left + m_sizeSquare.cx / 2;
			double y = m_pSelected->GetRect().bottom + 1;
			
			while (y < m_rect.bottom)
			{
				CBCGPWinUITile* pHit = HitTestTile(CBCGPPoint(x, y));
				if (pHit != NULL && pHit != m_pSelected)
				{
					pNewSelected = pHit;
					break;
				}
				
				y += m_sizeSquare.cy / 4;
			}
		}
		break;

	case VK_RETURN:
	case VK_SPACE:
		if (m_pSelected != NULL && m_pSelected->IsVisible() && !m_pSelected->GetRect().IsRectEmpty())
		{
			if (OnClick(m_pSelected))
			{
				return TRUE;
			}
		}
		break;

	case VK_HOME:
		pNewSelected = GetFirstVisibleTile();
		if (pNewSelected != NULL)
		{
			EnsureVisible(pNewSelected);
		}
		break;

	case VK_END:
		pNewSelected = GetLastVisibleTile();
		if (pNewSelected != NULL)
		{
			EnsureVisible(pNewSelected);
		}
		break;

	case VK_TAB:
		if (m_pSelected == NULL)
		{
			pNewSelected = GetFirstVisibleTile();
		}
		else
		{
			const BOOL bShift = ::GetAsyncKeyState (VK_SHIFT) & 0x8000;

			CBCGPWinUITile* pTile = DYNAMIC_DOWNCAST(CBCGPWinUITile, m_pSelected);
			if (pTile != NULL)
			{
				int nGroupID = bShift ? pTile->GetGroupID() : pTile->GetGroupID() + 1;

				CBCGPWinUITilesGroupCaption* pGroupCaption = GetGroupCaption(nGroupID);
				if (pGroupCaption != NULL && pGroupCaption->IsClickable())
				{
					pNewSelected = pGroupCaption;
					EnsureVisible(pNewSelected);
				}
				else
				{
					if (!m_lstCaptionButtons.IsEmpty())
					{
						pNewSelected = bShift ? m_lstCaptionButtons.GetHead() : m_lstCaptionButtons.GetTail();
					}
				}
			}
			else
			{
				CBCGPWinUITilesGroupCaption* pGroupCaption = DYNAMIC_DOWNCAST(CBCGPWinUITilesGroupCaption, m_pSelected);
				if (pGroupCaption != NULL)
				{
					int nID = bShift ? pGroupCaption->GetID() - 1 : pGroupCaption->GetID();

					for (POSITION pos = m_lstTilesSorted.GetHeadPosition(); pos != NULL;)
					{
						CBCGPWinUITile* pTile = m_lstTilesSorted.GetNext(pos);
						ASSERT_VALID(pTile);
						
						if (pTile->IsVisible() && !pTile->GetRect().IsRectEmpty() && pTile->GetGroupID() == nID)
						{
							pNewSelected = pTile;
							EnsureVisible(pNewSelected);
							break;
						}
					}
				}
				else
				{
					CBCGPWinUITilesCaptionButton* pCaptionButton = DYNAMIC_DOWNCAST(CBCGPWinUITilesCaptionButton, m_pSelected);
					if (pCaptionButton != NULL)
					{
						BOOL bSet = FALSE;

						POSITION pos = m_lstCaptionButtons.Find(pCaptionButton);
						if (pos != NULL)
						{
							if (bShift)
							{
								m_lstCaptionButtons.GetNext(pos);
							}
							else
							{
								m_lstCaptionButtons.GetPrev(pos);
							}

							if (pos != NULL)
							{
								pNewSelected = m_lstCaptionButtons.GetAt(pos);
								bSet = TRUE;
							}
						}

						if (!bSet)
						{
							if (!m_lstCaptions.IsEmpty() && m_lstCaptions.GetHead()->IsClickable())
							{
								pNewSelected = m_lstCaptions.GetHead();
							}
							else
							{
								pNewSelected = GetFirstVisibleTile();
							}

							EnsureVisible(pNewSelected);
						}
					}
				}
			}
		}
		break;
	}

	if (pNewSelected != m_pSelected && pNewSelected != NULL)
	{
		SetCurSelObject(pNewSelected);
		return TRUE;
	}

	return CBCGPBaseVisualObject::OnKeyboardDown(nChar, nRepCnt, nFlags);
}
//*****************************************************************************************
BOOL CBCGPWinUITiles::IsObjectOnScreen(CBCGPWinUIBaseObject* pObject) const
{
	ASSERT_VALID(pObject);

	CBCGPRect rectTile = pObject->GetRect();
	if (rectTile.IsRectEmpty())
	{
		return FALSE;
	}
	
	if (IsHorizontalLayout())
	{
		if (m_rectTiles.Width() >= rectTile.Width())
		{
			return rectTile.left >= m_rectTiles.left && rectTile.right <= m_rectTiles.right;
		}
		else
		{
			return rectTile.left >= m_rectTiles.left && rectTile.left <= m_rectTiles.right;
		}
	}
	else
	{
		if (m_rectTiles.Height() >= rectTile.Height())
		{
			return rectTile.top >= m_rectTiles.top && rectTile.bottom <= m_rectTiles.bottom;
		}
		else
		{
			return rectTile.top >= m_rectTiles.top && rectTile.top <= m_rectTiles.bottom;
		}
	}
}
//*****************************************************************************************
void CBCGPWinUITiles::EnsureVisible(CBCGPWinUIBaseObject* pObject)
{
	ASSERT_VALID(pObject);

	if (IsObjectOnScreen(pObject))
	{
		return;
	}

	if (m_pWndOwner->GetSafeHwnd() == NULL)
	{
		return;
	}

	m_pWndOwner->LockWindowUpdate();

	SetDirty(TRUE, TRUE);

	while (!IsObjectOnScreen(pObject)) 
	{
		double nPrevOffset = m_ScrollBar.GetOffset();
		m_ScrollBar.OnScrollStep(TRUE);

		if (nPrevOffset == m_ScrollBar.GetOffset())
		{
			break;
		}
	}

	m_pWndOwner->UnlockWindowUpdate();
	Redraw();
}
//*****************************************************************************************
void CBCGPWinUITiles::StopTileDragging(BOOL bRepos)
{
	if (!IsDraggingTile())
	{
		return;
	}

	if (bRepos && m_bDraggedOut)
	{
		CBCGPWinUITile* pDraggedTile = DYNAMIC_DOWNCAST(CBCGPWinUITile, m_pPressed);
		ASSERT_VALID(pDraggedTile);

		CBCGPSize sizeDragged = pDraggedTile->GetRect().Size(); 

		POSITION posOld = m_lstTilesSorted.Find(pDraggedTile);
		ASSERT(posOld != NULL);

		m_lstTilesSorted.RemoveAt(posOld);

		POSITION posNearest = NULL;
		double dblDistMin = 0.0;
		int nNewGroup = -1;
		CBCGPRect rectNearest;

		for (POSITION pos = m_lstTilesSorted.GetHeadPosition(); pos != NULL;)
		{
			POSITION posSaved = pos;
			
			CBCGPWinUITile* pTile = m_lstTilesSorted.GetNext(pos);
			ASSERT_VALID(pTile);

			double dblDist = bcg_distance(pTile->GetRect().TopLeft(), m_ptDragTile);
			if (dblDistMin == 0.0 || dblDistMin > dblDist)
			{
				dblDistMin = dblDist;
				posNearest = posSaved;
				nNewGroup = pTile->GetGroupID();
				rectNearest = pTile->GetRect();
			}
		}

		if (posNearest != NULL)
		{
			pDraggedTile->SetGroupID(nNewGroup);

			BOOL bInsertAfter = TRUE;

			if (IsHorizontalLayout())
			{
				bInsertAfter = m_ptDragTile.x + sizeDragged.cx / 2 > rectNearest.CenterPoint().x;
			}
			else
			{
				bInsertAfter = m_ptDragTile.y + sizeDragged.cy / 2 > rectNearest.CenterPoint().y;
			}

			if (bInsertAfter)
			{
				m_lstTilesSorted.InsertAfter(posNearest, pDraggedTile);
			}
			else
			{
				m_lstTilesSorted.InsertBefore(posNearest, pDraggedTile);
			}
		}
		else
		{
			m_lstTilesSorted.AddTail(pDraggedTile);
		}

		m_ptDragTile = CBCGPPoint(-1., -1.);
		m_bDraggedOut = FALSE;
		
		m_pHighlighted = m_pPressed = NULL;

		SetDirty(TRUE, TRUE);

		EnsureVisible(pDraggedTile);
	}
	else
	{
		CBCGPRect rectDrag;

		if (m_pPressed != NULL)
		{
			rectDrag = CBCGPRect(m_ptDragTile, m_pPressed->GetRect().Size());
		}

		m_ptDragTile = CBCGPPoint(-1., -1.);
		m_bDraggedOut = FALSE;

		GetOwner()->RedrawWindow((CRect)rectDrag);
		RedrawObject(m_pPressed);
	}
}
//*****************************************************************************************
BOOL CBCGPWinUITiles::OnMouseDown(int nButton, const CBCGPPoint& pt)
{
	if (CBCGPPopupMenu::GetActiveMenu() == NULL && nButton == 0)
	{
		if (m_ScrollBar.OnMouseDown(pt))
		{
			m_pPressed = NULL;
			return TRUE;
		}

		m_pPressed = HitTestObject(pt);
		m_bDraggedOut = FALSE;

		if (m_bTilesDragAndDrop && m_pPressed != NULL && DYNAMIC_DOWNCAST(CBCGPWinUITile, m_pPressed) != NULL)
		{
			m_ptDragTile = m_pPressed->GetRect().TopLeft();
			m_ptDragTileOffset = pt - m_ptDragTile;

			::SetCursor (AfxGetApp ()->LoadStandardCursor (IDC_ARROW));
			return TRUE;	// Capture mouse input
		}
	}

	return CBCGPBaseVisualObject::OnMouseDown(nButton, pt);
}
//*****************************************************************************************
void CBCGPWinUITiles::OnMouseUp(int nButton, const CBCGPPoint& pt)
{
	CBCGPWinUIBaseObject* pClicked = NULL;

	if (nButton == 0)
	{
		BOOL bDraggedOut = m_bDraggedOut;

		if (IsDraggingTile())
		{
			StopTileDragging(bDraggedOut);
		}
		
		if (m_pPressed == m_pHighlighted && m_pHighlighted != NULL && !bDraggedOut)
		{
			pClicked = m_pHighlighted;
		}
		
		m_pPressed = m_pHighlighted = NULL;
		m_ScrollBar.OnCancelMode();
	}

	if (pClicked != NULL)
	{
		RedrawObject(pClicked);
		OnClick(pClicked);
	}

	CBCGPBaseVisualObject::OnMouseUp(nButton, pt);
}
//*****************************************************************************************
void CBCGPWinUITiles::OnMouseMove(const CBCGPPoint& pt)
{
	CBCGPBaseVisualObject::OnMouseMove(pt);

	if (CBCGPPopupMenu::GetActiveMenu() != NULL)
	{
		return;
	}

	if (m_ScrollBar.OnDragThumb(pt))
	{
		Redraw();
		return;
	}

	if (IsDraggingTile())
	{
		if (!m_ScrollBar.GetRect().IsRectEmpty() && !m_rect.PtInRect(pt))
		{
			if (IsHorizontalLayout())
			{
				m_ScrollBar.OnScrollStep(pt.x > m_rect.right);
			}
			else
			{
				m_ScrollBar.OnScrollStep(pt.y > m_rect.bottom);
			}
		}

		CBCGPRect rectDragOld(m_ptDragTile, m_pPressed->GetRect().Size());
		rectDragOld.InflateRect(5, 5);
		
		m_ptDragTile = pt - m_ptDragTileOffset;

		CBCGPRect rectDragNew(m_ptDragTile, m_pPressed->GetRect().Size());
		rectDragNew.InflateRect(5, 5);
			
		GetOwner()->RedrawWindow((CRect)rectDragOld);
		GetOwner()->RedrawWindow((CRect)rectDragNew);

		if (!m_pPressed->GetRect().PtInRect(pt))
		{
			m_bDraggedOut = TRUE;
		}

		GetOwner()->UpdateWindow();

		return;
	}

	CBCGPWinUIBaseObject* pOldHighlighted = m_pHighlighted;
	m_pHighlighted = HitTestObject(pt);

	if (m_pHighlighted != pOldHighlighted)
	{
		RedrawObject(pOldHighlighted);
		RedrawObject(m_pHighlighted);
	}
}
//*****************************************************************************************
void CBCGPWinUITiles::OnMouseLeave()
{
	CBCGPBaseVisualObject::OnMouseLeave();

	if (IsDraggingTile())
	{
		return;
	}

	CBCGPWinUIBaseObject* pOldHighlighted = m_pHighlighted;
	CBCGPWinUIBaseObject* pOldPressed = m_pPressed;

	m_pHighlighted = m_pPressed = NULL;

	RedrawObject(pOldHighlighted);

	if (pOldPressed != pOldHighlighted)
	{
		RedrawObject(pOldPressed);
	}
}
//*****************************************************************************************
BOOL CBCGPWinUITiles::OnSetMouseCursor(const CBCGPPoint& pt)
{
	if (CBCGPPopupMenu::GetActiveMenu() == NULL && HitTestObject(pt))
	{
		::SetCursor (globalData.GetHandCursor ());
		return TRUE;
	}

	return CBCGPBaseVisualObject::OnSetMouseCursor(pt);
}
//*****************************************************************************************
void CBCGPWinUITiles::OnCancelMode()
{
	CBCGPBaseVisualObject::OnCancelMode();

	StopTileDragging(FALSE);

	CBCGPWinUIBaseObject* pOldHighlighted = m_pHighlighted;
	CBCGPWinUIBaseObject* pOldPressed = m_pPressed;
	
	m_pHighlighted = m_pPressed = NULL;

	m_ScrollBar.OnCancelMode();

	RedrawObject(pOldHighlighted);
	
	if (pOldPressed != pOldHighlighted)
	{
		RedrawObject(pOldPressed);
	}
}
//*****************************************************************************************
BOOL CBCGPWinUITiles::OnGetToolTip(const CBCGPPoint& pt, CString& strToolTip, CString& strDescr)
{
	strToolTip.Empty();
	strDescr.Empty();

	if (CBCGPPopupMenu::GetActiveMenu() == NULL && !IsDraggingTile())
	{
		CBCGPWinUIBaseObject* pTile = HitTestObject(pt);
		if (pTile != NULL)
		{
			ASSERT_VALID(pTile);

			if (!pTile->GetToolTipText().IsEmpty())
			{
				strToolTip = pTile->GetToolTipText();
				strDescr = pTile->GetToolTipDescription();
				return TRUE;
			}
			else if (!pTile->GetToolTipDescription().IsEmpty())
			{
				strToolTip = pTile->GetName();
				strDescr = pTile->GetToolTipDescription();
				return TRUE;
			}
		}
	}

	return CBCGPBaseVisualObject::OnGetToolTip(pt, strToolTip, strDescr);
}
//*****************************************************************************************
void CBCGPWinUITiles::SetCurSel(CBCGPWinUITile* pSelTile)
{
	SetCurSelObject(pSelTile);
}
//*****************************************************************************************
void CBCGPWinUITiles::SetCurSelObject(CBCGPWinUIBaseObject* pObject)
{
	if (m_pSelected != pObject)
	{
		CBCGPWinUIBaseObject* pOldSel = m_pSelected;
		m_pSelected = pObject;
		
		RedrawObject(pOldSel);
		RedrawObject(m_pSelected);

		NotifyAccessibility(m_pSelected);
	}
}
//************************************************************************************
void CBCGPWinUITiles::NotifyAccessibility(CBCGPWinUIBaseObject* pTile)
{
	if (!globalData.IsAccessibilitySupport () || pTile == NULL)
	{
		return;
	}

#ifndef _BCGSUITE_
	globalData.NotifyWinEvent(EVENT_OBJECT_FOCUS, GetOwner()->GetSafeHwnd(), OBJID_CLIENT, GetAccChildIndex(pTile));
#else
	::NotifyWinEvent(EVENT_OBJECT_FOCUS, GetOwner()->GetSafeHwnd(), OBJID_CLIENT, GetAccChildIndex(pTile));
#endif
}
//*****************************************************************************************
CBCGPWinUITile* CBCGPWinUITiles::HitTestTile(const CBCGPPoint& point) const
{
	for (POSITION pos = m_lstTilesSorted.GetHeadPosition(); pos != NULL;)
	{
		CBCGPWinUITile* pTile = m_lstTilesSorted.GetNext(pos);
		ASSERT_VALID(pTile);

		if (pTile->IsVisible() && pTile->m_rect.PtInRect(point))
		{
			return pTile;
		}
	}

	return NULL;
}
//*****************************************************************************************
CBCGPWinUITile* CBCGPWinUITiles::FindTileByName(const CString& strName) const
{
	for (POSITION pos = m_lstTilesSorted.GetHeadPosition(); pos != NULL;)
	{
		CBCGPWinUITile* pTile = m_lstTilesSorted.GetNext(pos);
		ASSERT_VALID(pTile);

		if (pTile->GetName() == strName)
		{
			return pTile;
		}
	}

	return NULL;
}
//*****************************************************************************************
CBCGPWinUIBaseObject* CBCGPWinUITiles::HitTestObject(const CBCGPPoint& point) const
{
	if (((CBCGPWinUITiles*)this)->m_ScrollBar.GetRect().PtInRect(point))
	{
		return NULL;
	}

	for (POSITION posCaptionButton = m_lstCaptionButtons.GetHeadPosition(); posCaptionButton != NULL;)
	{
		CBCGPWinUITilesCaptionButton* pCaptionButton = m_lstCaptionButtons.GetNext(posCaptionButton);
		ASSERT_VALID(pCaptionButton);

		if (pCaptionButton->m_rect.PtInRect(point))
		{
			return pCaptionButton;
		}
	}

	if (m_rectCaption.PtInRect(point))
	{
		return NULL;
	}
	
	for (POSITION pos = m_lstCaptions.GetHeadPosition(); pos != NULL;)
	{
		CBCGPWinUITilesGroupCaption* pCaption = m_lstCaptions.GetNext(pos);
		ASSERT_VALID(pCaption);
		
		if (pCaption->m_rect.PtInRect(point))
		{
			return pCaption->IsClickable() ? pCaption : NULL;
		}
	}

	return HitTestTile(point);
}
//*****************************************************************************************
CBCGPWinUITile* CBCGPWinUITiles::GetFirstVisibleTile() const
{
	for (POSITION pos = m_lstTilesSorted.GetHeadPosition(); pos != NULL;)
	{
		CBCGPWinUITile* pTile = m_lstTilesSorted.GetNext(pos);
		ASSERT_VALID(pTile);
		
		if (pTile->IsVisible() && !pTile->GetRect().IsRectEmpty())
		{
			return pTile;
		}
	}
	
	return NULL;
}
//*****************************************************************************************
CBCGPWinUITile* CBCGPWinUITiles::GetLastVisibleTile() const
{
	for (POSITION pos = m_lstTilesSorted.GetTailPosition(); pos != NULL;)
	{
		CBCGPWinUITile* pTile = m_lstTilesSorted.GetPrev(pos);
		ASSERT_VALID(pTile);
		
		if (pTile->IsVisible() && !pTile->GetRect().IsRectEmpty())
		{
			return pTile;
		}
	}
	
	return NULL;
}
//*****************************************************************************************
void CBCGPWinUITiles::RedrawObject(CBCGPWinUIBaseObject* pTile)
{
	if (pTile != NULL)
	{
		GetOwner()->RedrawWindow((CRect)pTile->m_rect);
	}
}
//*****************************************************************************************
void CBCGPWinUITiles::SetFillBrush(const CBCGPBrush& brFill, BOOL bRedraw)
{
	m_brFill = brFill;

	COLORREF clrFill = (COLORREF)m_brFill.GetColor();

	double H, S, V;
	CBCGPDrawManager::RGBtoHSV(clrFill, &H, &S, &V);
	
	double Hc = (H < 180.0) ? H + 180.0 : H - 180.0;
	
    double Sc = S < 0.2 ? 0.2 : bcg_clamp(V * S / (V * (S - 1.0) + 1.0), 0.0, 1.0);
    double Vc = bcg_clamp((V * (S - 1.0) + 1.0), 0.0, 1.0);

	COLORREF clrFocus = CBCGPDrawManager::HSVtoRGB(Hc, Sc, Vc);

	m_brFocus = CBCGPBrush(CBCGPColor(clrFocus));

	if (bRedraw)
	{
		Redraw();
	}
}
//*****************************************************************************************
void CBCGPWinUITiles::SetCaptionForegroundColor(const CBCGPColor& colorCaptionForeground, BOOL bRedraw)
{
	m_brCaptionForeground = colorCaptionForeground;

	if (bRedraw)
	{
		Redraw();
	}
}
//*****************************************************************************************
void CBCGPWinUITiles::SetCustomBadgeGlyphs(UINT nImageResID)
{
	if (nImageResID == 0)
	{
		m_CustomBadgeGlyphs.Clear();
	}
	else
	{
		m_CustomBadgeGlyphs.Load(nImageResID);
	}
}
//*****************************************************************************************
void CBCGPWinUITiles::SetCustomBadgeGlyphs(const CBCGPImage& customBadgeGlyphs)
{
	m_CustomBadgeGlyphs = customBadgeGlyphs;
}
//*****************************************************************************************
void CBCGPWinUITiles::AddSorted(CBCGPWinUITile* pTileNew)
{
	for (POSITION pos = m_lstTilesSorted.GetHeadPosition(); pos != NULL;)
	{
		POSITION posSaved = pos;

		CBCGPWinUITile* pTile = m_lstTilesSorted.GetNext(pos);
		ASSERT_VALID(pTile);

		if (pTile->m_nGroup > pTileNew->m_nGroup || 
			pTile->m_nImportance > pTileNew->m_nImportance)
		{
			m_lstTilesSorted.InsertBefore(posSaved, pTileNew);
			return;
		}
	}

	m_lstTilesSorted.AddTail(pTileNew);
}
//*****************************************************************************************
void CBCGPWinUITiles::OnClickTile(CBCGPWinUITile* pClickedTile)
{
	ASSERT_VALID(pClickedTile);

	if (m_pWndOwner->GetSafeHwnd() == NULL)
	{
		return;
	}

	CWnd* pOwner = m_pWndOwner->GetOwner();
	if (pOwner->GetSafeHwnd() == NULL)
	{
		return;
	}

	if (m_pSelected != NULL && m_pSelected != pClickedTile)
	{
		SetCurSel(pClickedTile);
	}

	if (pClickedTile->GetViewRuntimeClass() != NULL)
	{
		OnCreateView(pClickedTile);
	}
	else
	{
		m_pWndOwner->GetOwner()->PostMessage(BCGM_ON_CLICK_WINUI_UI_TILE, (WPARAM)GetID(), (LPARAM)pClickedTile);
	}
}
//*****************************************************************************************
void CBCGPWinUITiles::OnClickGroupCaption(CBCGPWinUITilesGroupCaption* pCaption)
{
	if (m_pWndOwner->GetSafeHwnd() == NULL)
	{
		return;
	}

	CWnd* pOwner = m_pWndOwner->GetOwner();
	if (pOwner->GetSafeHwnd() == NULL)
	{
		return;
	}

	m_pWndOwner->GetOwner()->PostMessage(BCGM_ON_CLICK_WINUI_GROUP_CAPTION, (WPARAM)GetID(), (LPARAM)pCaption);
}
//*****************************************************************************************
void CBCGPWinUITiles::OnClickCaptionButton(CBCGPWinUITilesCaptionButton* pCaptionButton)
{
	if (m_pWndOwner->GetSafeHwnd() == NULL)
	{
		return;
	}
	
	CWnd* pOwner = m_pWndOwner->GetOwner();
	if (pOwner->GetSafeHwnd() == NULL)
	{
		return;
	}

	if (DYNAMIC_DOWNCAST(CBCGPWinUITilesNavigationButton, pCaptionButton) != NULL)
	{
		if (m_pWndCurrView->GetSafeHwnd() != NULL)
		{
			OnCloseView();
		}
		else
		{
			m_pWndOwner->GetOwner()->PostMessage(BCGM_ON_CLICK_WINUI_NAV_BACK_BUTTON, (WPARAM)GetID());
		}
	}
	else
	{
		if (pCaptionButton->GetViewRuntimeClass() != NULL)
		{
			OnCreateView(pCaptionButton);
		}
		else
		{
			m_pWndOwner->GetOwner()->PostMessage(BCGM_ON_CLICK_WINUI_CAPTION_BUTTON, (WPARAM)GetID(), (LPARAM)pCaptionButton);
		}
	}
}
//*****************************************************************************************
void CBCGPWinUITiles::OnScaleRatioChanged(const CBCGPSize& /*sizeScaleRatioOld*/)
{
	m_textFormatName.Scale(m_sizeScaleRatio.cy);
	m_textFormatText.Scale(m_sizeScaleRatio.cy);
	m_textFormatHeader.Scale(m_sizeScaleRatio.cy);
	m_textFormatBadge.Scale(m_sizeScaleRatio.cy);

	m_nHorzMargin = m_nHorzMarginOriginal * m_sizeScaleRatio.cx;
	m_nVertMargin = m_nVertMarginOriginal * m_sizeScaleRatio.cy;

	m_sizeSquare.cx = m_sizeSquareOriginal.cx * m_sizeScaleRatio.cx;
	m_sizeSquare.cy = m_sizeSquareOriginal.cy * m_sizeScaleRatio.cy;

	m_dblCornerRadius = m_dblCornerRadiusOriginal * m_sizeScaleRatio.cx;

	SetDirty();
}
//*******************************************************************************
BOOL CBCGPWinUITiles::OnMouseWheel(const CBCGPPoint& pt, short zDelta)
{
	if (!m_ScrollBar.GetRect().IsRectEmpty())
	{
		const int nSteps = abs(zDelta) / WHEEL_DELTA;
		for (int i = 0; i < nSteps; i++)
		{
			m_ScrollBar.OnScrollStep(zDelta < 0);
		}

		return TRUE;
	}

	return CBCGPBaseVisualObject::OnMouseWheel(pt, zDelta);
}
//*******************************************************************************
BOOL CBCGPWinUITiles::GetGestureConfig(CBCGPGestureConfig& gestureConfig)
{
	gestureConfig.EnablePan(TRUE, BCGP_GC_PAN_WITH_SINGLE_FINGER_VERTICALLY | BCGP_GC_PAN_WITH_SINGLE_FINGER_HORIZONTALLY | BCGP_GC_PAN_WITH_INERTIA | BCGP_GC_PAN_WITH_GUTTER);
	return TRUE;
}
//*******************************************************************************
BOOL CBCGPWinUITiles::OnGestureEventPan(const CBCGPPoint& ptFrom, const CBCGPPoint& ptTo, CBCGPSize& sizeOverPan)
{
	if (m_ScrollBar.GetRect().IsRectEmpty() || m_ScrollBar.GetStep() == 0.)
	{
		return FALSE;
	}

	double dblDelta = 0.;

	double dblOffsetOld = m_ScrollBar.GetOffset();

	if (m_bIsHorizontalLayout)
	{
		dblDelta = ptTo.x - ptFrom.x;
	}
	else
	{
		dblDelta = ptTo.y - ptFrom.y;
	}

	double dblOffsetNew = dblOffsetOld - dblDelta;
	double dblOffsetMax = m_ScrollBar.GetTotal() - m_ScrollBar.GetScrollSize();
	double dblOverPan = 0.;

	if (dblOffsetNew < 0.)
	{
		dblOverPan = -dblOffsetNew;
		dblOffsetNew = 0.;
		dblDelta = dblOffsetOld;
	}
	else if (dblOffsetNew > dblOffsetMax)
	{
		dblOverPan = dblOffsetMax - dblOffsetNew;
		dblOffsetNew = dblOffsetMax;
		dblDelta = dblOffsetOld - dblOffsetNew;
	}

	if (dblOffsetNew != m_ScrollBar.GetOffset())
	{
		m_ScrollBar.SetOffset(dblOffsetNew);
		m_ScrollBar.ReposThumb();
		OnScroll(&m_ScrollBar, -dblDelta);

		return TRUE;
	}

	if (dblOverPan != 0.)
	{
		if (m_bIsHorizontalLayout)
		{
			sizeOverPan.cx = dblOverPan;
		}
		else
		{
			sizeOverPan.cy = dblOverPan;
		}
	}

	return TRUE;
}
//******************************************************************************************
void CBCGPWinUITiles::SetScrollBarColorTheme(CBCGPVisualScrollBarColorTheme& theme)
{
	m_ScrollBar.SetColorTheme(theme);
}
//******************************************************************************************
void CBCGPWinUITiles::OnCreateView(CBCGPWinUIBaseObject* pParentObject)
{
	ASSERT_VALID(pParentObject);

	if (m_pWndCurrView->GetSafeHwnd() != NULL)
	{
		if (m_pLastClicked == pParentObject)
		{
			return;
		}

		m_pWndCurrView->DestroyWindow();
		delete m_pWndCurrView;
		m_pWndCurrView = NULL;
	}

	m_pLastClicked = NULL;

	m_pWndCurrView = pParentObject->CreateView();
	if (m_pWndCurrView->GetSafeHwnd() == NULL)
	{
		return;
	}

	m_pLastClicked = pParentObject;
	m_bRestoreSel = (pParentObject == m_pSelected);

	// Hide tiles:
	for (POSITION pos = m_lstTiles.GetHeadPosition(); pos != NULL;)
	{
		CBCGPWinUITile* pTile = m_lstTiles.GetNext(pos);
		ASSERT_VALID(pTile);
		
		pTile->SetVisible(FALSE);
	}
	
	if (!m_strCaption.IsEmpty())
	{
		// Set view caption:
		m_strCaption = pParentObject->GetViewTitle();
	}
	
	// Enable navigation "Back" button:
	EnableNavigationBackButton();

	m_pWndCurrView->SetFocus();

	if (m_pWndOwner->GetSafeHwnd() != NULL && m_pWndOwner->GetOwner()->GetSafeHwnd() != NULL)
	{
		m_pWndOwner->GetOwner()->SendMessage(BCGM_ON_CREATE_WINUI_VIEW, 0, (LPARAM)m_pWndCurrView);
	}

	m_pWndCurrView->ShowWindow(SW_SHOWNOACTIVATE);
	SetDirty(TRUE, TRUE);
}
//******************************************************************************************
void CBCGPWinUITiles::OnCloseView()
{
	// Destroy current view:
	if (m_pWndCurrView->GetSafeHwnd() != NULL)
	{
		m_pWndCurrView->DestroyWindow();
		delete m_pWndCurrView;
		m_pWndCurrView = NULL;
	}
	
	// Show tiles:
	for (POSITION pos = m_lstTiles.GetHeadPosition(); pos != NULL;)
	{
		CBCGPWinUITile* pTile = m_lstTiles.GetNext(pos);
		ASSERT_VALID(pTile);
		
		pTile->SetVisible();
	}
	
	// Restore default caption:
	m_strCaption = m_strTopLevelCaption;
	
	// Hide navigation "Back" button:
	EnableNavigationBackButton(FALSE);

	if (m_pWndOwner->GetSafeHwnd() != NULL && m_pWndOwner->GetOwner()->GetSafeHwnd() != NULL)
	{
		m_pWndOwner->GetOwner()->SendMessage(BCGM_ON_CLOSE_WINUI_VIEW);
	}

	if (m_bRestoreSel)
	{
		SetCurSelObject(m_pLastClicked);
	}

	m_pLastClicked = NULL;
	SetDirty(TRUE, TRUE);

	if (m_bRestoreSel)
	{
		if (m_pSelected != NULL)
		{
			EnsureVisible(m_pSelected);
		}

		m_bRestoreSel = FALSE;
	}
}
//*****************************************************************************
CBCGPWinUIBaseObject* CBCGPWinUITiles::GetAccChild(int nIndex)
{
	ASSERT_VALID(this);

	if (m_pWndCurrView->GetSafeHwnd() == NULL)
	{
		nIndex--;
	}
	else
	{
		nIndex -= 2;
	}

	if (nIndex < 0 || nIndex >= (int)m_lstVisibleObjects.GetCount())
	{
		return NULL;
	}

	POSITION pos = m_lstVisibleObjects.FindIndex(nIndex);
	if (pos == NULL)
	{
		return NULL;
	}

	return m_lstVisibleObjects.GetAt(pos);
}
//*****************************************************************************
long CBCGPWinUITiles::GetAccChildIndex(CBCGPWinUIBaseObject* pTile)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pTile);

	if (!pTile->IsVisible())
	{
		return 0;
	}

	long nIndex = m_pWndCurrView->GetSafeHwnd() == NULL ? 1 : 2;

	for (POSITION pos = m_lstVisibleObjects.GetHeadPosition(); pos != NULL; nIndex++)
	{
		CBCGPWinUIBaseObject* pListTile = m_lstVisibleObjects.GetNext(pos);
		ASSERT_VALID(pListTile);

		if (pListTile == pTile)
		{
			return nIndex;
		}
	}

	return 0;
}
//****************************************************************************
HRESULT CBCGPWinUITiles::get_accChildCount(long *pcountChildren)
{
	if (!pcountChildren)
	{
		return E_INVALIDARG;
	}

	long nCount = (long)m_lstVisibleObjects.GetCount();

	if (m_pWndCurrView->GetSafeHwnd() != NULL)
	{
		nCount++;
	}

	*pcountChildren = nCount;
	return S_OK;
}
//****************************************************************************
HRESULT CBCGPWinUITiles::get_accChild(VARIANT varChild, IDispatch** ppdispChild)
{
	if ((m_pWndCurrView->GetSafeHwnd() != NULL) && (varChild.vt == VT_I4) && (varChild.lVal == 1))
	{
		if (ppdispChild == NULL)
		{
			return E_INVALIDARG;
		}

		*ppdispChild = NULL;

		AccessibleObjectFromWindow(m_pWndCurrView->GetSafeHwnd(), (DWORD)OBJID_CLIENT, IID_IAccessible, (void**)ppdispChild);
		
		if (*ppdispChild != NULL)
		{
			return S_OK;
		}
	}

	return S_FALSE;
}
//****************************************************************************
HRESULT CBCGPWinUITiles::get_accName(VARIANT varChild, BSTR *pszName)
{
	if ((varChild.vt == VT_I4) && (varChild.lVal == CHILDID_SELF))
	{
		CString strText = m_strCaption;

		if (strText.IsEmpty() && m_pWndOwner->GetSafeHwnd() != NULL)
		{
			m_pWndOwner->GetWindowText(strText);

			if (strText.IsEmpty())
			{
				strText = _T("WinUITiles");
			}
		}

		*pszName = strText.AllocSysString();
		return S_OK;
	}

	if (varChild.vt == VT_I4)
	{
		CBCGPWinUIBaseObject* pAccTile = GetAccChild(varChild.lVal);
		if (pAccTile != NULL)
		{
			CString strName = pAccTile->GetAccName();
			*pszName = strName.AllocSysString();

			return S_OK;
		}
	}

	return S_FALSE;
}
//****************************************************************************
HRESULT CBCGPWinUITiles::get_accValue(VARIANT varChild, BSTR *pszValue)
{
	if ((varChild.vt == VT_I4) && (varChild.lVal == CHILDID_SELF))
	{
		return S_FALSE;
	}

	if (varChild.vt == VT_I4)
	{
		CBCGPWinUIBaseObject* pAccTile = GetAccChild(varChild.lVal);
		if (pAccTile != NULL)
		{
			CString strValue;

			LPCTSTR lpszValue = pAccTile->GetAccValue();
			if (lpszValue != NULL)
			{
				strValue = lpszValue;
			}
			else
			{
				CBCGPWinUITile* pTile = DYNAMIC_DOWNCAST(CBCGPWinUITile, pAccTile);
				if (pTile != NULL)
				{
					int nNumber = pTile->GetBadgeNumber();
					if (nNumber >= 0)
					{
						strValue.Format(_T("%d"), nNumber);
					}
				}
			}

			if (!strValue.IsEmpty())
			{
				*pszValue = strValue.AllocSysString();
				return S_OK;
			}
		}
	}

	return S_FALSE;
}
//****************************************************************************
HRESULT CBCGPWinUITiles::get_accDescription(VARIANT varChild, BSTR *pszDescription)
{
	if (((varChild.vt != VT_I4) && (varChild.lVal != CHILDID_SELF)) || (NULL == pszDescription))
	{
		return E_INVALIDARG;
	}

	if ((varChild.vt == VT_I4) && (varChild.lVal == CHILDID_SELF))
	{
		*pszDescription = SysAllocString(L"WinUITiles Table");
		return S_OK;
	}

	CBCGPWinUIBaseObject* pAccTile = GetAccChild(varChild.lVal);
	if (pAccTile != NULL)
	{
		CString strDescr;

		LPCTSTR lpszDescr = pAccTile->GetAccDescription();
		if (lpszDescr != NULL)
		{
			strDescr = lpszDescr;
		}
		else
		{
			CBCGPWinUITile* pTile = DYNAMIC_DOWNCAST(CBCGPWinUITile, pAccTile);
			if (pTile != NULL)
			{
				strDescr = pTile->GetText();
			}
		}

		if (!strDescr.IsEmpty())
		{
			*pszDescription = strDescr.AllocSysString();
			return S_OK;
		}
	}

	return S_FALSE;
}
//****************************************************************************
HRESULT CBCGPWinUITiles::get_accRole(VARIANT varChild, VARIANT *pvarRole)
{
	if (!pvarRole || ((varChild.vt != VT_I4) && (varChild.lVal != CHILDID_SELF)))
	{
		return E_INVALIDARG;
	}

	if ((varChild.vt == VT_I4) && (varChild.lVal == CHILDID_SELF))
	{
		pvarRole->vt = VT_I4;
		pvarRole->lVal = ROLE_SYSTEM_TABLE;

		return S_OK;
	}

	CBCGPWinUIBaseObject* pAccTile = GetAccChild(varChild.lVal);
	if (pAccTile != NULL)
	{
		pvarRole->vt = VT_I4;
		pvarRole->lVal = pAccTile->GetAccRole();

		return S_OK;
	}

	return S_FALSE;
}
//****************************************************************************
HRESULT CBCGPWinUITiles::get_accState(VARIANT varChild, VARIANT *pvarState)
{
	if ((varChild.vt == VT_I4) && (varChild.lVal == CHILDID_SELF))
	{
		pvarState->vt = VT_I4;
		pvarState->lVal = STATE_SYSTEM_NORMAL;
		return S_OK;
	}

	pvarState->vt = VT_I4;
	pvarState->lVal = STATE_SYSTEM_FOCUSABLE;
	pvarState->lVal |= STATE_SYSTEM_SELECTABLE;
	
	if (varChild.vt == VT_I4)
	{
		CBCGPWinUIBaseObject* pAccTile = GetAccChild(varChild.lVal);
		if (pAccTile != NULL && pAccTile == m_pSelected)
		{
			pvarState->lVal |= STATE_SYSTEM_FOCUSED;
			pvarState->lVal |= STATE_SYSTEM_SELECTED;
		}
	}

	return S_OK;
}
//****************************************************************************
HRESULT CBCGPWinUITiles::get_accDefaultAction(VARIANT varChild, BSTR* pszDefaultAction)
{
	if (varChild.vt == VT_I4 && varChild.lVal > 0)
	{
		CBCGPWinUIBaseObject* pAccTile = GetAccChild(varChild.lVal);
		if (pAccTile != NULL)
		{
			CString strDefAction = pAccTile->GetAccDefaultAction();
			*pszDefaultAction = strDefAction.AllocSysString();

			return S_OK;
		}
	}
	
	return S_FALSE;
}
//****************************************************************************
HRESULT CBCGPWinUITiles::accSelect(long flagsSelect, VARIANT varChild)
{
	if (varChild.vt != VT_I4)
	{
		return E_INVALIDARG;
	}

	if (flagsSelect == SELFLAG_NONE || varChild.lVal == CHILDID_SELF)
	{
		return S_FALSE;
	}

	if (flagsSelect == SELFLAG_TAKEFOCUS || flagsSelect == SELFLAG_TAKESELECTION)
	{
		CBCGPWinUIBaseObject* pAccTile = GetAccChild(varChild.lVal);
		if (pAccTile != NULL)
		{
			SetCurSelObject(pAccTile);
			return S_OK;
		}
	}

	return S_FALSE;
}
//****************************************************************************
HRESULT CBCGPWinUITiles::accLocation(long *pxLeft, long *pyTop, long *pcxWidth, long *pcyHeight, VARIANT varChild)
{
	if (!pxLeft || !pyTop || !pcxWidth || !pcyHeight)
	{
		return E_INVALIDARG;
	}

	if (m_pWndOwner->GetSafeHwnd() == NULL)
	{
		return S_FALSE;
	}

	if ((varChild.vt == VT_I4) && (varChild.lVal == CHILDID_SELF))
	{
		CRect rc;
		m_pWndOwner->GetWindowRect(rc);

		*pxLeft = rc.left;
		*pyTop = rc.top;
		*pcxWidth = rc.Width();
		*pcyHeight = rc.Height();

		return S_OK;
	}
	else if (varChild.vt == VT_I4)
	{
		CBCGPWinUIBaseObject* pAccTile = GetAccChild(varChild.lVal);
		if (pAccTile != NULL)
		{
			CRect rcProp = pAccTile->GetRect();
			m_pWndOwner->ClientToScreen(&rcProp);

			*pxLeft = rcProp.left;
			*pyTop = rcProp.top;
			*pcxWidth = rcProp.Width();
			*pcyHeight = rcProp.Height();
		}
	}

	return S_OK;
}
//****************************************************************************
HRESULT CBCGPWinUITiles::accHitTest(long  xLeft, long yTop, VARIANT *pvarChild)
{
	if (!pvarChild)
	{
		return E_INVALIDARG;
	}

	if (m_pWndOwner->GetSafeHwnd() == NULL)
	{
        pvarChild->vt = VT_EMPTY;
		return S_FALSE;
	}

	CPoint pt(xLeft, yTop);
	HWND hwdFromPoint = WindowFromPoint(pt);

	if (hwdFromPoint != m_pWndOwner->GetSafeHwnd() && hwdFromPoint != NULL)
    {
		if (hwdFromPoint == m_pWndCurrView->GetSafeHwnd())
		{
			pvarChild->pdispVal = NULL;
			pvarChild->vt = VT_DISPATCH;

			AccessibleObjectFromWindow(hwdFromPoint, (DWORD)OBJID_CLIENT, IID_IAccessible, (void**)&pvarChild->pdispVal);

			if (pvarChild->pdispVal != NULL)
			{
				return S_OK;
			}
		}

        return S_FALSE;
    }

	m_pWndOwner->ScreenToClient(&pt);

	pvarChild->vt = VT_I4;

	CBCGPWinUIBaseObject* pTile = HitTestObject(pt);
	if (pTile != NULL)
	{
		pvarChild->lVal = GetAccChildIndex(pTile);
	}
	else
	{
		pvarChild->lVal = CHILDID_SELF;
	}

	return S_OK;
}
//******************************************************************************
HRESULT CBCGPWinUITiles::accDoDefaultAction(VARIANT varChild)
{
    if (varChild.vt != VT_I4)
    {
        return E_INVALIDARG;
    }

	CBCGPWinUIBaseObject* pAccTile = GetAccChild(varChild.lVal);
	if (pAccTile != NULL)
	{
		if (OnClick(pAccTile))
		{
			return S_OK;
		}
    }

    return S_FALSE;
}
//******************************************************************************
BOOL CBCGPWinUITiles::OnClick(CBCGPWinUIBaseObject* pObject)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pObject);

	CBCGPWinUITile* pTile = DYNAMIC_DOWNCAST(CBCGPWinUITile, pObject);
	if (pTile != NULL)
	{
		OnClickTile(pTile);
		return TRUE;
	}
	else
	{
		CBCGPWinUITilesGroupCaption* pGroupCaption = DYNAMIC_DOWNCAST(CBCGPWinUITilesGroupCaption, pObject);
		if (pGroupCaption != NULL)
		{
			OnClickGroupCaption(pGroupCaption);
			return TRUE;
		}
		else
		{
			CBCGPWinUITilesCaptionButton* pCaptionButton = DYNAMIC_DOWNCAST(CBCGPWinUITilesCaptionButton, pObject);
			if (pCaptionButton != NULL)
			{
				OnClickCaptionButton(pCaptionButton);
				return TRUE;
			}
		}
	}

	return FALSE;
}
//******************************************************************************
BOOL CBCGPWinUITiles::SaveState (LPCTSTR lpszProfileName/* = NULL*/, int nIndex/* = -1*/, UINT uiID/* = (UINT) -1*/)
{
#ifndef _BCGSUITE_
	CString strProfileName = ::BCGPGetRegPath (strWinUITilesProfile, lpszProfileName);
#else
	CString strProfileName = AFXGetRegPath (strWinUITilesProfile, lpszProfileName);
#endif
	
	BOOL bResult = FALSE;
	
	if (nIndex == -1)
	{
		nIndex = m_nID;
	}
	
	CString strSection;
	if (uiID == (UINT) -1)
	{
		strSection.Format (REG_SECTION_FMT, strProfileName, nIndex);
	}
	else
	{
		strSection.Format (REG_SECTION_FMT_EX, strProfileName, nIndex, uiID);
	}

	CBCGPVisualInfo temp;
	CBCGPVisualCollector collector(temp);

	m_lstTiles.RemoveAll();
	m_lstTiles.AddTail(&m_lstTilesSorted);

	CBCGPVisualInfo::XElementWinUITiles* pInfo = (CBCGPVisualInfo::XElementWinUITiles*)collector.CollectElement(*this);
	if (pInfo == NULL)
	{
		return FALSE;
	}

	CBCGPRegistrySP regSP;
	CBCGPRegistry& reg = regSP.Create (FALSE, FALSE);
	
	if (reg.CreateKey (strSection))
	{
		CString strCustomization;

		BOOL bFormatTags = CBCGPTagManager::s_bFormatTags;
		CBCGPTagManager::s_bFormatTags = FALSE;

		pInfo->ToTag(strCustomization);

		CBCGPTagManager::s_bFormatTags = bFormatTags;

		reg.Write (REG_ENTRY_TILES_CUSTOMIZATION_DATA, strCustomization);
	}

	if (pInfo != NULL)
	{
		delete pInfo;
	}
	
	return bResult;
}
//******************************************************************************
BOOL CBCGPWinUITiles::LoadState(LPCTSTR lpszProfileName/* = NULL*/, int nIndex/* = -1*/, UINT uiID/* = (UINT) -1*/)
{
#ifndef _BCGSUITE_
	CString strProfileName = ::BCGPGetRegPath (strWinUITilesProfile, lpszProfileName);
#else
	CString strProfileName = AFXGetRegPath (strWinUITilesProfile, lpszProfileName);
#endif
	
	if (nIndex == -1)
	{
		nIndex = m_nID;
	}
	
	CString strSection;
	if (uiID == (UINT) -1)
	{
		strSection.Format (REG_SECTION_FMT, strProfileName, nIndex);
	}
	else
	{
		strSection.Format (REG_SECTION_FMT_EX, strProfileName, nIndex, uiID);
	}
	
	CBCGPRegistrySP regSP;
	CBCGPRegistry& reg = regSP.Create (FALSE, TRUE);
	
	if (!reg.Open (strSection))
	{
		return FALSE;
	}
	
	CString strCustomization;
	reg.Read (REG_ENTRY_TILES_CUSTOMIZATION_DATA, strCustomization);
	
	if (strCustomization.IsEmpty())
	{
		return FALSE;
	}

	CBCGPVisualInfo::XElementWinUITiles info;
	if (!info.FromTag(strCustomization))
	{
		return FALSE;
	}

	RemoveAll();

	CBCGPVisualInfo temp;
	CBCGPVisualConstructor constructor(temp);

	constructor.ConstructElement(*this, info);

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPWinUITilesCtrl

CBCGPWinUITilesCtrl::CBCGPWinUITilesCtrl()
{
	m_pWinUITiles = NULL;

#if _MSC_VER >= 1300
	EnableActiveAccessibility();
#endif
}
//*****************************************************************************
CBCGPWinUITilesCtrl::~CBCGPWinUITilesCtrl()
{
	if (m_pWinUITiles != NULL)
	{
		delete m_pWinUITiles;
	}
}

BEGIN_MESSAGE_MAP(CBCGPWinUITilesCtrl, CBCGPVisualCtrl)
	//{{AFX_MSG_MAP(CBCGPWinUITilesCtrl)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGPWinUITilesGroupCaption

void CBCGPWinUITilesGroupCaption::DoDraw(CBCGPWinUITiles* pOwner, CBCGPGraphicsManager* pGM, BOOL bIsPressed, BOOL bIsHighlighted)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pGM);
	ASSERT_VALID(pOwner);

	CBCGPRect rect = m_rect;
	CBCGPColor clrText = m_colorText;
	
	if (clrText.IsNull())
	{
		clrText = pOwner->GetCaptionForegroundColor();
	}

	if (bIsPressed || bIsHighlighted)
	{
		clrText.MakeLighter();
	}

	pGM->DrawText(m_strName, rect, pOwner->GetGroupCaptionTextFormat(), CBCGPBrush(clrText));

	if (pOwner->IsObjectSelected(this))
	{
		rect.DeflateRect(1.0, 1.0);
		pGM->DrawRectangle(rect, pOwner->m_brFocus, 1.0, &pOwner->m_strokeFocus);
	}
}

/////////////////////////////////////////////////////////////////////////////
// / CBCGPWinUITilesCaptionButton

void CBCGPWinUITilesCaptionButton::DoDraw(CBCGPWinUITiles* pOwner, CBCGPGraphicsManager* pGM, BOOL bIsPressed, BOOL bIsHighlighted)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pGM);
	ASSERT_VALID(pOwner);
	
	CBCGPRect rect = m_rect;
	rect.DeflateRect(0.5 * pOwner->GetHorzMargin(), 0.5 * pOwner->GetVertMargin());

	CBCGPRect rectBounds = rect;
	
	CBCGPSize sizeImage = pGM->GetImageSize(m_Image);
	if (!sizeImage.IsEmpty())
	{
		pGM->DrawImage(m_Image, CBCGPPoint(rect.left + 0.5 * pOwner->GetHorzMargin(), rect.top + 0.5 * pOwner->GetVertMargin()));
		rect.left += sizeImage.cx + pOwner->GetHorzMargin();
	}

	if (!m_strName.IsEmpty())
	{
		CBCGPColor clrText = m_colorText;
		
		if (clrText.IsNull())
		{
			clrText = pOwner->GetCaptionForegroundColor();
		}
		
		if (bIsPressed || bIsHighlighted)
		{
			clrText.MakeLighter();
		}
		
		CBCGPRect rectName = rect;
		rectName.top = rectName.CenterPoint().y - 0.5 * m_dblTextHeight;
		rectName.bottom = rectName.top + m_dblTextHeight;

		CBCGPRect rectDescr = rect;

		if (!m_strDescription.IsEmpty())
		{
			rectName.bottom = rectName.top + m_dblNameHeight + 1.0;
			rectDescr.top = rectName.bottom;
		}

		pGM->DrawText(m_strName, rectName, pOwner->GetCaptionButtonTextFormat(), CBCGPBrush(clrText));

		if (!m_strDescription.IsEmpty())
		{
			pGM->DrawText(m_strDescription, rectDescr, pOwner->GetNameTextFormat(), CBCGPBrush(clrText));
		}
	}

	if (pOwner->IsObjectSelected(this))
	{
		rectBounds.DeflateRect(1.0, 1.0);
		pGM->DrawRectangle(rectBounds, pOwner->m_brFocus, 1.0, &pOwner->m_strokeFocus);
	}
}
//*******************************************************************************
CBCGPSize CBCGPWinUITilesCaptionButton::GetSize(CBCGPGraphicsManager* pGM)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pGM);
	ASSERT_VALID(m_pOwner);

	CBCGPSize sizeImage = pGM->GetImageSize(m_Image);
	CBCGPSize sizeText = pGM->GetTextSize(m_strName, m_pOwner->GetCaptionButtonTextFormat());
	CBCGPSize sizeDescription = pGM->GetTextSize(m_strDescription, m_pOwner->GetNameTextFormat());

	if (!sizeDescription.IsEmpty() && !sizeText.IsEmpty())
	{
		sizeDescription.cy += m_pOwner->GetVertMargin() / 4;
		sizeText.cx = max(sizeText.cx, sizeDescription.cx);
	}

	m_dblNameHeight = sizeText.cy;
	m_dblTextHeight = sizeText.cy + sizeDescription.cy;

	if (sizeImage.IsEmpty())
	{
		if (sizeText.IsEmpty())
		{
			return CBCGPSize(0., 0.);
		}

		sizeText.cy = m_dblTextHeight;

		return sizeText + CBCGPSize(m_pOwner->GetHorzMargin(), m_pOwner->GetVertMargin());
	}

	if (sizeText.IsEmpty())
	{
		return sizeImage + CBCGPSize(1.5 * m_pOwner->GetHorzMargin(), 1.5 * m_pOwner->GetVertMargin());
	}

	return CBCGPSize(2.5 * m_pOwner->GetHorzMargin() + sizeImage.cx + sizeText.cx, max(m_dblTextHeight, sizeImage.cy) + 2.0 * m_pOwner->GetVertMargin());
}
//*****************************************************************************************
void CBCGPWinUITilesCaptionButton::SetImage(const CBCGPImage& image) 
{ 
	ASSERT_VALID(this);
	m_Image = image;
}
//*****************************************************************************************
void CBCGPWinUITilesCaptionButton::SetDescription(const CString& strDescription)
{
	ASSERT_VALID(this);
	m_strDescription = strDescription;
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPWinUITilesNavigationButton

void CBCGPWinUITilesNavigationButton::DoDraw(CBCGPWinUITiles* /*pOwner*/, CBCGPGraphicsManager* pGM, BOOL bIsPressed, BOOL bIsHighlighted)
{
	ASSERT_VALID(pGM);
	ASSERT_VALID(m_pOwner);
	
	if (m_rect.IsRectEmpty())
	{
		return;
	}
	
	CBCGPRect rect = m_rect;
	double dblImageOpacity = bIsPressed ? 0.5 : bIsHighlighted ? 0.8 : 1.0;
	int nImageIndex = 1;
	
	CBCGPSize sizeIcon(32, 32);
	CBCGPPoint ptOffset(0.5 * (rect.Width() - sizeIcon.cx), 0.5 * (rect.Height() - sizeIcon.cy));
	
	pGM->DrawImage(m_pOwner->m_NavigationBack, rect.TopLeft() + ptOffset, sizeIcon, dblImageOpacity, CBCGPImage::BCGP_IMAGE_INTERPOLATION_MODE_LINEAR,
		CBCGPRect(CBCGPPoint(sizeIcon.cx * nImageIndex, 0), sizeIcon));

	if (m_pOwner->IsObjectSelected(this))
	{
		rect.DeflateRect(0.5, 0.5);
		pGM->DrawRectangle(rect, m_pOwner->m_brFocus, 1.0, &m_pOwner->m_strokeFocus);
	}
}
//*****************************************************************************************
CBCGPSize CBCGPWinUITilesNavigationButton::GetSize(CBCGPGraphicsManager* /*pGM*/)
{
	CBCGPSize sizeIcon(34, 34);
	return sizeIcon;
}
