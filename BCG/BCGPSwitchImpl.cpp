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
// BCGPSwitchImpl.cpp: implementation of the CBCGPSwitchImpl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BCGPSwitchImpl.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CBCGPSwitchColors::CBCGPSwitchColors()
{
	m_brOutline = m_brOutlineThumb = CBCGPBrush(CBCGPColor::Gray);
	m_brFillThumb = CBCGPBrush(CBCGPColor::DarkGray);

	m_brFillOn = CBCGPBrush(CBCGPColor::DodgerBlue);
	m_brFillOff = CBCGPBrush(CBCGPColor::Silver);
	
	m_brLabelOn = CBCGPBrush(CBCGPColor::White);
	m_brLabelOff = CBCGPBrush(CBCGPColor::DimGray);

	m_brFill = CBCGPBrush(CBCGPColor::WhiteSmoke);
	m_brFocus = CBCGPBrush(CBCGPColor::LightGray);
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(CBCGPSwitchImpl, CBCGPStaticGaugeImpl)
IMPLEMENT_DYNAMIC(CBCGPSwitchCtrl, CBCGPVisualCtrl)

CBCGPSwitchImpl::CBCGPSwitchImpl(CBCGPVisualContainer* pContainer)
	: CBCGPStaticGaugeImpl(pContainer)
{
	m_bIsInteractiveMode = TRUE;
	m_bValue = FALSE;
	m_bDrawTextLabels = FALSE;
	m_bImageScaling = TRUE;
	m_bDontDrawThumbOn = FALSE;
	m_bDontDrawThumbOff = FALSE;
	m_Style = BCGP_SWITCH_RECTANGLE;
	m_DefaultDrawFlags = BCGP_DRAW_DYNAMIC;
	
	m_strLabelOn = _T("On");
	m_strLabelOff = _T("Off");

	LOGFONT lf;
	globalData.fontBold.GetLogFont(&lf);
	
	m_textFormat.CreateFromLogFont(lf);
	
	m_textFormat.SetTextAlignment(CBCGPTextFormat::BCGP_TEXT_ALIGNMENT_CENTER);
	m_textFormat.SetTextVerticalAlignment(CBCGPTextFormat::BCGP_TEXT_ALIGNMENT_CENTER);

	m_strokeFocus.SetDashStyle(CBCGPStrokeStyle::BCGP_DASH_STYLE_DASH);
	m_bDrawFocus = FALSE;
}
//*******************************************************************************
CBCGPSwitchImpl::~CBCGPSwitchImpl()
{
}
//*******************************************************************************
void CBCGPSwitchImpl::OnDraw(CBCGPGraphicsManager* pGM, const CBCGPRect& /*rectClip*/, DWORD dwFlags)
{
	if (m_rect.IsRectEmpty() || !m_bIsVisible)
	{
		return;
	}
	
	if ((dwFlags & m_DefaultDrawFlags) == 0)
	{
		return;
	}
	
	ASSERT_VALID(pGM);

	CBCGPRect rect = m_rect;

	BOOL bIsDrawFocusFrame = FALSE;
	if (m_bDrawFocus)
	{
		CBCGPSwitchCtrl* pCtrl = DYNAMIC_DOWNCAST(CBCGPSwitchCtrl, m_pWndOwner);
		if (pCtrl->GetSafeHwnd() != NULL && pCtrl->IsFocused())
		{
			bIsDrawFocusFrame = TRUE;
		}
	}

	double dblCtrlPad = m_bDrawFocus ? 2.0 : 0.5;
	rect.DeflateRect(dblCtrlPad, dblCtrlPad);

	if (!m_bImageScaling)
	{
		CBCGPImage& imageGutter = IsOn() ? m_imageGutterOn : m_imageGutterOff;
		if (!imageGutter.IsEmpty())
		{
			CBCGPSize sizeImageGutter = pGM->GetImageSize(imageGutter);

			if (rect.Width() > sizeImageGutter.cx)
			{
				rect.left = rect.CenterPoint().x - sizeImageGutter.cx / 2;
				rect.right = rect.left + sizeImageGutter.cx;
			}

			if (rect.Height() > sizeImageGutter.cy)
			{
				rect.top = rect.CenterPoint().y - sizeImageGutter.cy / 2;
				rect.bottom = rect.top + sizeImageGutter.cy;
			}
		}
	}


	CBCGPRect rectThumb = rect;
	double dblPad = 0.0;
	double dblThumbsSize = 0;

	CBCGPRect rectLabel = rect;
	BOOL bIsVertical = FALSE;

	if (rect.Width() >= rect.Height())
	{
		dblThumbsSize = (m_Style != BCGP_SWITCH_CIRCLE && m_Style != BCGP_SWITCH_CIRCLE_NARROW_GUTTER) ? rectThumb.Height() * 2.0 / 3.0 : rectThumb.Height();
		dblPad = max(2.0, dblThumbsSize / 6);

		rectLabel.DeflateRect(dblPad, dblPad);

		if (IsOn())
		{
			rectThumb.left = rectThumb.right - dblThumbsSize;
			rectLabel.right = rectThumb.left - 1;
		}
		else
		{
			rectThumb.right = rectThumb.left + dblThumbsSize;
			rectLabel.left = rectThumb.right + 1;
		}
	}
	else
	{
		bIsVertical = TRUE;

		dblThumbsSize = (m_Style != BCGP_SWITCH_CIRCLE && m_Style != BCGP_SWITCH_CIRCLE_NARROW_GUTTER) ? rectThumb.Width() * 2.0 / 3.0 : rectThumb.Width();
		dblPad = max(2.0, dblThumbsSize / 6);

		rectLabel.DeflateRect(dblPad, dblPad);

		if (IsOn())
		{
			rectThumb.top = rectThumb.bottom - dblThumbsSize;
			rectLabel.bottom = rectThumb.top - 1;
		}
		else
		{
			rectThumb.bottom = rectThumb.top + dblThumbsSize;
			rectLabel.top = rectThumb.bottom + 1;
		}
	}

	OnDrawGutter(pGM, rect, dblPad, dblThumbsSize);

	if (m_bDrawTextLabels)
	{
		m_textFormat.SetDrawingAngle(bIsVertical ? 90.0 : 0.0);
		rectLabel.DeflateRect(1.0, 1.0);

		if (bIsVertical)
		{
			rectLabel.right--;
		}
		else
		{
			rectLabel.bottom--;
		}

		OnDrawLabel(pGM, rectLabel);
	}

	BOOL bDontDrawThum = IsOn() ? m_bDontDrawThumbOn : m_bDontDrawThumbOff;
	
	if (!bDontDrawThum)
	{
		OnDrawThumb(pGM, rectThumb, dblPad * 1.5);
	}

	if (bIsDrawFocusFrame)
	{
		CBCGPRect rectFrame = m_rect;
		rectFrame.DeflateRect(0.1, 0.1);

		pGM->DrawRectangle(rectFrame, m_Colors.m_brFocus, 1.0, &m_strokeFocus);
	}
}
//*******************************************************************************
void CBCGPSwitchImpl::OnDrawGutter(CBCGPGraphicsManager* pGM, const CBCGPRect& rect, double dblPad, double dblThumbsSize)
{
	ASSERT_VALID(pGM);

	CBCGPRect rectFill = rect;

	const CBCGPImage& image = IsOn() ? m_imageGutterOn : m_imageGutterOff;

	if (m_Style == BCGP_SWITCH_RECTANGLE || m_Style == BCGP_SWITCH_RECTANGLE_NARROW_GUTTER)
	{
		if (m_Style == BCGP_SWITCH_RECTANGLE_NARROW_GUTTER)
		{
			double delta = dblPad * 1.5;
			rectFill.DeflateRect(delta, delta);
			
			if (!image.IsEmpty())
			{
				DoDrawImage(pGM, image, rectFill);
			}
			else
			{
				pGM->FillRectangle(rectFill, IsOn() ? m_Colors.m_brFillOn : m_Colors.m_brFillOff);
				pGM->DrawRectangle(rectFill, m_Colors.m_brOutline);
			}
		}
		else
		{
			if (!image.IsEmpty())
			{
				DoDrawImage(pGM, image, rectFill);
			}
			else
			{
				pGM->FillRectangle(rect, m_Colors.m_brFill);
				pGM->DrawRectangle(rect, m_Colors.m_brOutline);
				
				rectFill.DeflateRect(dblPad, dblPad);
				
				pGM->FillRectangle(rectFill, IsOn() ? m_Colors.m_brFillOn : m_Colors.m_brFillOff);
			}
		}
	}
	else
	{
		double radius = 0.0;
		
		switch (m_Style)
		{
		case BCGP_SWITCH_CIRCLE:
			radius = .5 * dblThumbsSize;
			break;
			
		case BCGP_SWITCH_CIRCLE_NARROW_GUTTER:
			radius = .25 * dblThumbsSize;
			rectFill.DeflateRect(dblPad, dblPad);
			break;
			
		case BCGP_SWITCH_ROUNDED_RECTANGLE:
			radius = dblPad * 2.0;
			break;
			
		case BCGP_SWITCH_ROUNDED_RECTANGLE_NARROW_GUTTER:
			radius = dblPad * 1.5;
			rectFill.DeflateRect(radius, radius);
			break;
		}
		
		if (!image.IsEmpty())
		{
			DoDrawImage(pGM, image, rectFill);
		}
		else
		{
			CBCGPRoundedRect rectFillRounded(rectFill, radius, radius);
			
			pGM->FillRoundedRectangle(rectFillRounded, IsOn() ? m_Colors.m_brFillOn : m_Colors.m_brFillOff);
			pGM->DrawRoundedRectangle(rectFillRounded, m_Colors.m_brOutline);
		}
	}
}
//*******************************************************************************
void CBCGPSwitchImpl::OnDrawLabel(CBCGPGraphicsManager* pGM, const CBCGPRect& rectLabel)
{
	ASSERT_VALID(pGM);
	pGM->DrawText(GetLabel(IsOn()), rectLabel, m_textFormat, IsOn() ? m_Colors.m_brLabelOn : m_Colors.m_brLabelOff);
}
//*******************************************************************************
void CBCGPSwitchImpl::OnDrawThumb(CBCGPGraphicsManager* pGM, const CBCGPRect& rectThumb, double dblRoundedRectCornerRadius)
{
	ASSERT_VALID(pGM);

	const CBCGPImage& image = m_imageThumb;
	
	if (!image.IsEmpty())
	{
		DoDrawImage(pGM, image, rectThumb);
		return;
	}

	switch (m_Style)
	{
	case BCGP_SWITCH_CIRCLE:
	case BCGP_SWITCH_CIRCLE_NARROW_GUTTER:
		pGM->FillEllipse(rectThumb, m_Colors.m_brFillThumb);
		pGM->DrawEllipse(rectThumb, m_Colors.m_brOutlineThumb);
		break;

	case BCGP_SWITCH_ROUNDED_RECTANGLE:
	case BCGP_SWITCH_ROUNDED_RECTANGLE_NARROW_GUTTER:
		{
			CBCGPRoundedRect rectThumbRounded(rectThumb, dblRoundedRectCornerRadius, dblRoundedRectCornerRadius);
			
			pGM->FillRoundedRectangle(rectThumbRounded, m_Colors.m_brFillThumb);
			pGM->DrawRoundedRectangle(rectThumbRounded, m_Colors.m_brOutlineThumb);
		}
		break;
	
	default:
		pGM->FillRectangle(rectThumb, m_Colors.m_brFillThumb);
		pGM->DrawRectangle(rectThumb, m_Colors.m_brOutlineThumb);
		break;
	}
}
//*******************************************************************************
void CBCGPSwitchImpl::CopyFrom(const CBCGPBaseVisualObject& srcObj)
{
	CBCGPStaticGaugeImpl::CopyFrom(srcObj);

	const CBCGPSwitchImpl& src = (const CBCGPSwitchImpl&)srcObj;

	m_Colors.CopyFrom(src.m_Colors);

	m_bValue = src.m_bValue;
	m_Style = src.m_Style;
	m_bDrawTextLabels = src.m_bDrawTextLabels;
	m_strLabelOn = src.m_strLabelOn;
	m_strLabelOff = src.m_strLabelOff;
	m_textFormat = src.m_textFormat;
	m_bDrawFocus = src.m_bDrawFocus;
	m_imageGutterOff = src.m_imageGutterOff;
	m_imageGutterOn = src.m_imageGutterOn;
	m_imageThumb = src.m_imageThumb;
	m_bImageScaling = src.m_bImageScaling;
	m_bDontDrawThumbOn = src.m_bDontDrawThumbOn;
	m_bDontDrawThumbOff = src.m_bDontDrawThumbOff;
}
//*******************************************************************************
BOOL CBCGPSwitchImpl::OnKeyboardDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (m_bIsInteractiveMode)
	{
		BOOL bOldValue = m_bValue;

		switch (nChar)
		{
		case VK_SPACE:
			m_bValue = !m_bValue;
			break;

		case VK_LEFT:
			if (m_rect.Width() > m_rect.Height() && m_bValue)
			{
				m_bValue = FALSE;
			}
			break;

		case VK_RIGHT:
			if (m_rect.Width() > m_rect.Height() && !m_bValue)
			{
				m_bValue = TRUE;
			}
			break;

		case VK_UP:
			if (m_rect.Width() < m_rect.Height() && m_bValue)
			{
				m_bValue = FALSE;
			}
			break;
			
		case VK_DOWN:
			if (m_rect.Width() < m_rect.Height() && !m_bValue)
			{
				m_bValue = TRUE;
			}
			break;
		}

		if (m_bValue != bOldValue)
		{
			SetDirty(TRUE, TRUE);
			CBCGPStaticGaugeImpl::FireClickEvent(CBCGPPoint(-1.0, -1.0));
			return TRUE;
		}
	}

	return CBCGPStaticGaugeImpl::OnKeyboardDown(nChar, nRepCnt, nFlags);
}
//*******************************************************************************
CBCGPSize CBCGPSwitchImpl::GetDefaultSize(CBCGPGraphicsManager* pGM, const CBCGPBaseVisualObject* /*pParentGauge*/)
{
	ASSERT_VALID(pGM);

	if (!m_bDrawTextLabels)
	{
		return CBCGPSize(50, 20);
	}

	CBCGPSize sizeLabelOn = pGM->GetTextSize(m_strLabelOn, m_textFormat);
	CBCGPSize sizeLabelOff = pGM->GetTextSize(m_strLabelOn, m_textFormat);

	double cx = max(sizeLabelOn.cx, sizeLabelOff.cx);
	double cy = max(sizeLabelOn.cy, sizeLabelOff.cy);

	return CBCGPSize(1.5 * cx, 1.25 * cy);
}
//*******************************************************************************
void CBCGPSwitchImpl::OnScaleRatioChanged(const CBCGPSize& sizeScaleRatioOld)
{
	CBCGPStaticGaugeImpl::OnScaleRatioChanged(sizeScaleRatioOld);
	
	m_textFormat.Scale(GetScaleRatioMid());
	SetDirty();
}
//*******************************************************************************
void CBCGPSwitchImpl::DoDrawImage(CBCGPGraphicsManager* pGM, const CBCGPImage& image, const CBCGPRect& rect)
{
	ASSERT_VALID(pGM);

	if (m_bImageScaling)
	{
		pGM->DrawImage(image, rect.TopLeft(), rect.Size());
		return;
	}

	CBCGPSize sizeImage = pGM->GetImageSize((CBCGPImage&)image);

	double x = rect.left + max(0.0, 0.5 * (rect.Width() - sizeImage.cx));
	double y = rect.top + max(0.0, 0.5 * (rect.Height() - sizeImage.cy));

	pGM->DrawImage(image, CBCGPPoint(x, y));
}
//*******************************************************************************
void AFXAPI DDX_Switch(CDataExchange* pDX, int nIDC, int& value)
{
	ASSERT(pDX != NULL);
	ASSERT(pDX->m_pDlgWnd->GetSafeHwnd() != NULL);

	pDX->PrepareCtrl(nIDC);

	HWND hWndCtrl = NULL;
	pDX->m_pDlgWnd->GetDlgItem(nIDC, &hWndCtrl);

	CBCGPSwitchCtrl* pSwitchCtrl = DYNAMIC_DOWNCAST(CBCGPSwitchCtrl, CWnd::FromHandle(hWndCtrl));
	if (pSwitchCtrl != NULL)
	{
		ASSERT(pSwitchCtrl->GetSwitch() != NULL);

		if (pDX->m_bSaveAndValidate)
		{
			value = (int)pSwitchCtrl->GetSwitch()->IsOn();
			ASSERT(value >= 0 && value <= 1);
		}
		else
		{
			if (value < 0 || value > 1)
			{
				TRACE1("Warning: dialog data checkbox value (%d) out of range.\n", value);
				value = 0; // default to off
			}

			pSwitchCtrl->GetSwitch()->SetOn(value, TRUE);
		}
	}
	else
	{
		TRACE1("Warning: dialog control ID (%d) isn't a CBCGPSwitchCtrl.\n", nIDC);
	}
}
//************************************************************************************
HRESULT CBCGPSwitchImpl::get_accName(VARIANT varChild, BSTR *pszName)
{
	if (pszName == NULL)
	{
		return E_INVALIDARG;
	}

	if ((varChild.vt == VT_I4) && (varChild.lVal == CHILDID_SELF))
	{
		CString strText = GetName();
		if (strText.IsEmpty())
		{
			strText = GetToolTip();
			if (strText.IsEmpty())
			{
				strText = _T("Switch");
			}
		}

		*pszName = strText.AllocSysString();
		return S_OK;
	}

	return S_FALSE;
}
//****************************************************************************
HRESULT CBCGPSwitchImpl::get_accValue(VARIANT varChild, BSTR *pszValue)
{
	if (pszValue == NULL)
	{
		return E_INVALIDARG;
	}

	if ((varChild.vt == VT_I4) && (varChild.lVal == CHILDID_SELF))
	{
		CString strValue = m_bValue ? m_strLabelOn : m_strLabelOff;

		if (!strValue.IsEmpty())
		{
			*pszValue = strValue.AllocSysString();
			return S_OK;
		}
	}

	return S_FALSE;
}
//****************************************************************************
HRESULT CBCGPSwitchImpl::get_accRole(VARIANT varChild, VARIANT *pvarRole)
{
	if (!pvarRole || ((varChild.vt != VT_I4) && (varChild.lVal != CHILDID_SELF)))
	{
		return E_INVALIDARG;
	}

	if ((varChild.vt == VT_I4) && (varChild.lVal == CHILDID_SELF))
	{
		pvarRole->vt = VT_I4;
		pvarRole->lVal = ROLE_SYSTEM_CHECKBUTTON;

		return S_OK;
	}

	return S_FALSE;
}
//****************************************************************************
HRESULT CBCGPSwitchImpl::get_accState(VARIANT varChild, VARIANT *pvarState)
{
	if ((varChild.vt == VT_I4) && (varChild.lVal == CHILDID_SELF))
	{
		pvarState->vt = VT_I4;
		pvarState->lVal = STATE_SYSTEM_FOCUSABLE;
		pvarState->lVal |= STATE_SYSTEM_SELECTABLE;

		if (m_bValue)
		{
			pvarState->lVal |= STATE_SYSTEM_CHECKED;
		}

		CBCGPSwitchCtrl* pCtrl = DYNAMIC_DOWNCAST(CBCGPSwitchCtrl, m_pWndOwner);
		if (pCtrl->GetSafeHwnd() != NULL && pCtrl->IsFocused())
		{
			pvarState->lVal |= STATE_SYSTEM_FOCUSED;
			pvarState->lVal |= STATE_SYSTEM_SELECTED;
		}

		return S_OK;
	}

	return S_FALSE;
}
//****************************************************************************
HRESULT CBCGPSwitchImpl::get_accDefaultAction(VARIANT varChild, BSTR* pszDefaultAction)
{
	if (varChild.vt == VT_I4 && varChild.lVal == CHILDID_SELF)
	{
		CString str = _T("Toggle");
		*pszDefaultAction = str.AllocSysString();
		
		return S_OK;
	}
	
	return S_FALSE;
}
//******************************************************************************
HRESULT CBCGPSwitchImpl::accDoDefaultAction(VARIANT varChild)
{
    if (varChild.vt != VT_I4)
    {
        return E_INVALIDARG;
    }

	if (m_bIsInteractiveMode)
	{
		m_bValue = !m_bValue;

		SetDirty(TRUE, TRUE);
		CBCGPStaticGaugeImpl::FireClickEvent(CBCGPPoint(-1.0, -1.0));
	}

    return S_OK;
}
