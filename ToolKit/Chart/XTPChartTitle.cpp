// XTPChartTitle.cpp
//
// This file is a part of the XTREME TOOLKIT PRO MFC class library.
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

#include "Common/XTPPropExchange.h"

#include "XTPChartTitle.h"
#include "Drawing/XTPChartDeviceContext.h"
#include "Drawing/XTPChartDeviceCommand.h"
#include "XTPChartElementView.h"
#include "XTPChartContent.h"
#include "Appearance/XTPChartAppearance.h"
#include "Appearance/XTPChartPalette.h"

//////////////////////////////////////////////////////////////////////////
// CXTPChartTitle

IMPLEMENT_DYNAMIC(CXTPChartTitle, CXTPChartTextElement)

CXTPChartTitle::CXTPChartTitle()
{
	m_clrTextColor = CXTPChartColor::Empty;
	m_bVisible = TRUE;

	m_bAntialiasing = TRUE;

	m_pFont = CXTPChartFont::GetTahoma18();

	m_nDocking = xtpChartDockTop;
	m_nAlignment = xtpChartAlignCenter;

	m_nIndent = 0;
	m_nInnerIndent = 5;

}

CXTPChartTitle::~CXTPChartTitle()
{
	SAFE_RELEASE(m_pFont);
}

void CXTPChartTitle::SetText(LPCTSTR lpszTitle)
{
	m_strText = lpszTitle;
	OnChartChanged();
}

void CXTPChartTitle::SetFont(CXTPChartFont* pFont)
{
	SAFE_RELEASE(m_pFont);
	m_pFont = pFont;
	OnChartChanged();
}

void CXTPChartTitle::SetTextColor(const CXTPChartColor& clrTextColor)
{
	m_clrTextColor = clrTextColor;
	OnChartChanged();
}

void CXTPChartTitle::SetDocking(XTPChartDocking nDocking)
{
	m_nDocking = nDocking;
	OnChartChanged();
}

void CXTPChartTitle::SetIndent(int nIndent)
{
	m_nIndent = nIndent;
	OnChartChanged();
}

void CXTPChartTitle::SetAlignment(XTPChartStringAlignment nAlignment)
{
	m_nAlignment = nAlignment;
	OnChartChanged();

}

int CXTPChartTitle::GetTextAngle() const
{
	if (m_nDocking == xtpChartDockTop || m_nDocking == xtpChartDockBottom)
		return 0;

	if (m_nDocking == xtpChartDockLeft)
		return 270;

	return 90;
}

CXTPChartColor CXTPChartTitle::GetTextColor() const
{
	return m_clrTextColor;
}

CXTPChartColor CXTPChartTitle::GetActualTextColor() const
{
	if (!m_clrTextColor.IsEmpty())
		return m_clrTextColor;

	return GetContent()->GetAppearance()->GetContentAppearance()->GetTitleAppearance()->TextColor;
}

/////////////////////////////////////////////////////////////////////////
// CXTPChartTitleView

class CXTPChartTitleView : public CXTPChartElementView
{
public:
	CXTPChartTitleView(CXTPChartTitle* pTitle, CXTPChartString& strText, CXTPChartElementView* pParentView);

	~CXTPChartTitleView();

public:
	CXTPChartDeviceCommand* CreateDeviceCommand(CXTPChartDeviceContext* pDC);

	void CalculateView(CXTPChartDeviceContext* pDC, CRect& rcBounds, CRect rcChart);

	CXTPChartString m_strText;
	CXTPChartTitle* m_pTitle;
	CPoint m_ptOrigin;

};

CPoint CXTPChartTitle::CalcOrigin(CRect bounds, CRect startBounds, CRect rect)
{
	if (m_nDocking == xtpChartDockLeft)
	{
		int x = bounds.left + m_nInnerIndent;

		if (m_nAlignment == xtpChartAlignCenter)
			return CPoint(x, startBounds.top + startBounds.Height() / 2);

		if (m_nAlignment == xtpChartAlignNear)
			return CPoint(x, bounds.bottom - rect.Height() / 2 - m_nInnerIndent);

		return CPoint(x, bounds.top + rect.Height() / 2 + m_nInnerIndent);

	}
	else if (m_nDocking == xtpChartDockRight)
	{
		int x = bounds.right - m_nInnerIndent;

		if (m_nAlignment == xtpChartAlignCenter)
			return CPoint(x, startBounds.top + startBounds.Height() / 2);

		if (m_nAlignment == xtpChartAlignNear)
			return CPoint(x, bounds.top + rect.Height() / 2 + m_nInnerIndent);

		return CPoint(x, bounds.bottom - rect.Height() / 2 - m_nInnerIndent);

	}
	else
	{
		int y = m_nDocking == xtpChartDockTop ? bounds.top + m_nInnerIndent : bounds.bottom - m_nInnerIndent;

		if (m_nAlignment == xtpChartAlignCenter)
			return CPoint(startBounds.left + startBounds.Width() / 2, y);

		if (m_nAlignment == xtpChartAlignNear)
			return CPoint(bounds.left + rect.Width() / 2 + m_nInnerIndent, y);

		return CPoint(bounds.right - rect.Width() / 2 - m_nInnerIndent, y);
	}
}

XTPChartNearTextPosition CXTPChartTitle::GetNearTextPosition()
{
	if (m_nDocking == xtpChartDockLeft) return xtpChartTextNearRight;
	else if (m_nDocking == xtpChartDockRight) return xtpChartTextNearLeft;
	else if (m_nDocking == xtpChartDockTop) return xtpChartTextNearBottom;
	return xtpChartTextNearTop;

}


void CXTPChartTitleView::CalculateView(CXTPChartDeviceContext* pDC, CRect& rcBounds, CRect rcChart)
{

	CXTPChartString strText = m_strText;

	CXTPChartRotatedTextPainterNearLine painter(pDC, strText, m_pTitle, CPoint(0, 0) , m_pTitle->GetNearTextPosition(), (float)m_pTitle->GetTextAngle());
	CXTPChartSizeF sizeString = painter.GetSize();

	CRect rect = painter.GetRoundedBounds();

	XTPChartDocking nDocking = m_pTitle->GetDocking();

	int size = (nDocking == xtpChartDockLeft || nDocking == xtpChartDockRight ? rect.Width() : rect.Height());

	CPoint origin = m_pTitle->CalcOrigin(rcBounds, rcChart, rect);
	rect.OffsetRect(origin);


	m_ptOrigin = origin;

	if (nDocking == xtpChartDockLeft) rcBounds.left += size + m_pTitle->GetIndent();
	else if (nDocking == xtpChartDockRight) rcBounds.right -= size + m_pTitle->GetIndent();
	else if (nDocking == xtpChartDockTop) rcBounds.top += size + m_pTitle->GetIndent();
	else if (nDocking == xtpChartDockBottom) rcBounds.bottom -= size + m_pTitle->GetIndent();
}

CXTPChartElementView* CXTPChartTitle::CreateView(CXTPChartDeviceContext* /*pDC*/, CXTPChartElementView* pParentView)
{
	if (!m_bVisible)
		return NULL;

	CXTPChartString strText = m_strText;

	CXTPChartTitleView* pView = new CXTPChartTitleView(this, strText, pParentView);

	return pView;
}

void CXTPChartTitle::DoPropExchange(CXTPPropExchange* pPX)
{
	PX_Bool(pPX, _T("Visible"), m_bVisible, FALSE);
	PX_Enum(pPX, _T("Docking"), m_nDocking, xtpChartDockTop);
	PX_Enum(pPX, _T("Alignment"), m_nAlignment, xtpChartAlignCenter);
	PX_Bool(pPX, _T("Antialiasing"), m_bAntialiasing, TRUE);
	PX_Int(pPX, _T("Indent"), m_nIndent, 0);
	PX_Color(pPX, _T("TextColor"), m_clrTextColor);
	PX_String(pPX, _T("Text"), m_strText);

	PX_Font(pPX, _T("Font"), m_pFont);
}



CXTPChartTitleView::CXTPChartTitleView(CXTPChartTitle* pTitle, CXTPChartString& strText, CXTPChartElementView* pParentView)
	: CXTPChartElementView(pParentView)
{
	m_strText = strText;
	m_pTitle = pTitle;
	m_ptOrigin = CPoint(0, 0);
}

CXTPChartTitleView::~CXTPChartTitleView()
{

}

CXTPChartDeviceCommand* CXTPChartTitleView::CreateDeviceCommand(CXTPChartDeviceContext* pDC)
{
	CXTPChartDeviceCommand* pCommand = new CXTPChartHitTestElementCommand(m_pTitle);

	CXTPChartRotatedTextPainterNearLine painter(pDC, m_strText, m_pTitle, m_ptOrigin, m_pTitle->GetNearTextPosition(), (float)m_pTitle->GetTextAngle());

	CXTPChartDeviceCommand* pTextCommand =  painter.CreateDeviceCommand(pDC, m_pTitle->GetActualTextColor());

	pCommand->AddChildCommand(pTextCommand);

	return pCommand;
}

//////////////////////////////////////////////////////////////////////////
// CXTPChartTitleCollection

CXTPChartTitleCollection::CXTPChartTitleCollection(CXTPChartElement* pOwner)
{
	m_pOwner = pOwner;

}

CXTPChartTitleCollection::~CXTPChartTitleCollection()
{
}



CXTPChartTitle* CXTPChartTitleCollection::Add(CXTPChartTitle* pTitle)
{
	InsertAt(GetCount(), pTitle);

	return pTitle;
}

void CXTPChartTitleCollection::DoPropExchange(CXTPPropExchange* pPX)
{
	if (pPX->IsLoading())
	{
		RemoveAll();

		CXTPPropExchangeEnumeratorPtr pEnumerator(pPX->GetEnumerator(_T("Title")));
		POSITION pos = pEnumerator->GetPosition(0);

		while (pos)
		{
			CXTPPropExchangeSection pxItem(pEnumerator->GetNext(pos));


			CXTPChartTitle* pTitle = new CXTPChartTitle();
			Add(pTitle);

			pTitle->DoPropExchange(&pxItem);
		}
	}
	else
	{
		CXTPPropExchangeEnumeratorPtr pEnumerator(pPX->GetEnumerator(_T("Title")));
		POSITION pos = pEnumerator->GetPosition((int)m_arrElements.GetSize());

		for (int i = 0; i < GetCount(); i++)
		{
			CXTPPropExchangeSection pxItem(pEnumerator->GetNext(pos));

			m_arrElements[i]->DoPropExchange(&pxItem);
		}

	}
}

void CXTPChartTitleCollection::CreateView(CXTPChartDeviceContext* pDC, CXTPChartElementView* pParentView)
{
	for (int i = 0; i < GetCount(); i++)
	{
		CXTPChartElementView* pTitleView = GetAt(i)->CreateView(pDC, pParentView);
		UNREFERENCED_PARAMETER(pTitleView);
	}
}

void CXTPChartTitleCollection::CalculateView(CXTPChartDeviceContext* pDC, CRect& rcChart, CXTPChartElementView* pParentView)
{
	CRect rc(rcChart);

	for (int i = 0; i < pParentView->GetCount(); i++)
	{
		CXTPChartTitleView* pTitleView = (CXTPChartTitleView*)pParentView->GetAt(i);
		pTitleView->CalculateView(pDC, rcChart, rc);
	}
}


