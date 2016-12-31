// XTPChartDiagramPane.cpp
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

#include "XTPChartDiagram2D.h"
#include "XTPChartDiagram2DView.h"
#include "XTPChartDiagramPane.h"
#include "XTPChartAxis.h"
#include "XTPChartAxisGridLines.h"
#include "XTPChartAxisView.h"

#include "../Drawing/XTPChartDeviceCommand.h"
#include "../Drawing/XTPChartDeviceContext.h"
#include "../Drawing/XTPChartRectangleDeviceCommand.h"
#include "../Drawing/XTPChartLineDeviceCommand.h"
#include "../Drawing/XTPChartTransformationDeviceCommand.h"
#include "../Utils/XTPChartTextPainter.h"
#include "../Appearance/XTPChartAppearance.h"
#include "../Appearance/XTPChartFillStyle.h"

using namespace Gdiplus;

IMPLEMENT_DYNAMIC(CXTPChartDiagramPane, CXTPChartElement);

CXTPChartDiagramPane::CXTPChartDiagramPane(CXTPChartDiagram* pDiagram)
{
	m_pOwner = pDiagram;

	m_pBackgroundFillStyle = new CXTPChartFillStyle(this);
	m_pBackgroundFillStyle->SetFillMode(xtpChartFillEmpty);
}

CXTPChartDiagramPane::~CXTPChartDiagramPane()
{
	SAFE_RELEASE(m_pBackgroundFillStyle);
}

CXTPChartElementView* CXTPChartDiagramPane::CreateView(CXTPChartDeviceContext* pDC, CXTPChartElementView* pParent)
{
	UNREFERENCED_PARAMETER(pDC);

	CXTPChartDiagramPaneView* pView = new CXTPChartDiagramPaneView(this, pParent);

	return pView;
}

CXTPChartDiagram2DAppearance* CXTPChartDiagramPane::GetAppearance() const
{
	return CXTPChartAppearance::GetAppearance(this)->GetDiagram2DAppearance();
}

CXTPChartColor CXTPChartDiagramPane::GetActualBackgroundColor() const
{
	if (!m_clrBackgroundColor.IsEmpty())
		return m_clrBackgroundColor;

	return GetAppearance()->BackgroundColor;
}

CXTPChartColor CXTPChartDiagramPane::GetActualBackgroundColor2() const
{
	if (!m_clrBackgroundColor2.IsEmpty())
		return m_clrBackgroundColor2;

	return GetAppearance()->BackgroundColor2;
}

CXTPChartColor CXTPChartDiagramPane::GetActualBorderColor() const
{
	if (!m_clrBorderColor.IsEmpty())
		return m_clrBorderColor;

	return GetAppearance()->BorderColor;
}

CXTPChartColor CXTPChartDiagramPane::GetBackgroundColor() const
{
	return m_clrBackgroundColor;
}

CXTPChartColor CXTPChartDiagramPane::GetBackgroundColor2() const
{
	return m_clrBackgroundColor2;
}

CXTPChartColor CXTPChartDiagramPane::GetBorderColor() const
{
	return m_clrBorderColor;
}



CXTPChartFillStyle* CXTPChartDiagramPane::GetActualFillStyle() const
{
	if (m_pBackgroundFillStyle->GetFillMode() != xtpChartFillEmpty)
		return m_pBackgroundFillStyle;

	return GetAppearance()->BackgroundFillStyle;
}

void CXTPChartDiagramPane::DoPropExchange(CXTPPropExchange* pPX)
{
	PX_Color(pPX, _T("BackgroundColor"), m_clrBackgroundColor);
	PX_Color(pPX, _T("BackgroundColor2"), m_clrBackgroundColor2);
	PX_Color(pPX, _T("BorderColor"), m_clrBorderColor);

	CXTPPropExchangeSection secFillStyle(pPX->GetSection(_T("BackgroundFillStyle")));
	m_pBackgroundFillStyle->DoPropExchange(&secFillStyle);

}


//////////////////////////////////////////////////////////////////////////
// CXTPChartDiagramPaneView

CXTPChartDiagramPaneView::CXTPChartDiagramPaneView(CXTPChartDiagramPane* pPane, CXTPChartElementView* pParentView)
	: CXTPChartElementView(pParentView)
{
	m_pPane = pPane;

	m_ptOldPosition = CPoint(-1, -1);
}

CXTPChartDiagramPaneView::~CXTPChartDiagramPaneView()
{

}

void CXTPChartDiagramPaneView::CalculateView(CRect rcBounds)
{
	m_rcBounds = rcBounds;
}

CXTPChartAxisView* CXTPChartDiagramPaneView::GetAxisView(CXTPChartAxis* pAxis) const
{
	CXTPChartDiagram2DView* pDiagramView = (CXTPChartDiagram2DView*)m_pParentView->GetParentView();

	CXTPChartElementView* pAxisViews =  pDiagramView->m_pAxisViews;

	for (int i = 0; i < pAxisViews->GetCount(); i++)
	{
		CXTPChartAxisView* pAxisView = (CXTPChartAxisView*)pAxisViews->GetAt(i);
		if (pAxisView->GetAxis() == pAxis)
			return pAxisView;
	}
	return NULL;
}

CXTPChartDeviceCommand* CXTPChartDiagramPaneView::CreateGridLinesDeviceCommand(CXTPChartDeviceContext* pDC, CXTPChartAxisView* pAxisView)
{
	return pAxisView->CreateGridLinesDeviceCommand(pDC, m_rcBounds);
}

CXTPChartDeviceCommand* CXTPChartDiagramPaneView::CreateConstantLinesDeviceCommand(CXTPChartDeviceContext* pDC, CXTPChartAxisView* pAxisView, BOOL bBehind)
{
	return pAxisView->CreateConstantLinesDeviceCommand(pDC, m_rcBounds, bBehind);
}

CXTPChartDeviceCommand* CXTPChartDiagramPaneView::CreateStripsDeviceCommand(CXTPChartDeviceContext* pDC, CXTPChartAxisView* pAxisView)
{
	return pAxisView->CreateStripsDeviceCommand(pDC, m_rcBounds);
}


CXTPChartDeviceCommand* CXTPChartDiagramPaneView::CreateInterlacedDeviceCommand(CXTPChartDeviceContext* pDC, CXTPChartAxisView* pAxisView)
{
	return pAxisView->CreateInterlacedDeviceCommand(pDC, m_rcBounds);
}

CXTPChartDeviceCommand* CXTPChartDiagramPaneView::CreateDeviceCommand(CXTPChartDeviceContext* pDC)
{
	CXTPChartDeviceCommand* pCommand = new CXTPChartHitTestElementCommand(m_pPane, m_rcBounds);

	pCommand->AddChildCommand(m_pPane->GetActualFillStyle()->CreateDeviceCommand(m_rcBounds,
		m_pPane->GetActualBackgroundColor(), m_pPane->GetActualBackgroundColor2()));

	CXTPChartDiagram2DView* pDiagramView = (CXTPChartDiagram2DView*)m_pParentView->GetParentView();

	CXTPChartElementView* pAxisViews =  pDiagramView->m_pAxisViews;

	pCommand->AddChildCommand(new CXTPChartBoundedRectangleDeviceCommand(m_rcBounds, m_pPane->GetActualBorderColor(), 1));

	CRect rcBounds(m_rcBounds);
	rcBounds.DeflateRect(1, 1, 0, 0);

	CXTPChartDeviceCommand* pClipDeviceCommand = new CXTPChartClipDeviceCommand(rcBounds);
	pCommand->AddChildCommand(pClipDeviceCommand);


	int i;

	for (i = 0; i < pAxisViews->GetCount(); i++)
	{
		CXTPChartAxisView* pAxisView = (CXTPChartAxisView*)pAxisViews->GetAt(i);
		pClipDeviceCommand->AddChildCommand(CreateInterlacedDeviceCommand(pDC,pAxisView));
	}

	for (i = 0; i < pAxisViews->GetCount(); i++)
	{
		CXTPChartAxisView* pAxisView = (CXTPChartAxisView*)pAxisViews->GetAt(i);
		pClipDeviceCommand->AddChildCommand(CreateStripsDeviceCommand(pDC,pAxisView));
	}

	for (i = 0; i < pAxisViews->GetCount(); i++)
	{
		CXTPChartAxisView* pAxisView = (CXTPChartAxisView*)pAxisViews->GetAt(i);
		pClipDeviceCommand->AddChildCommand(CreateGridLinesDeviceCommand(pDC,pAxisView));
	}

	for (i = 0; i < pAxisViews->GetCount(); i++)
	{
		CXTPChartAxisView* pAxisView = (CXTPChartAxisView*)pAxisViews->GetAt(i);
		pClipDeviceCommand->AddChildCommand(CreateConstantLinesDeviceCommand(pDC,pAxisView, TRUE));
	}

	pClipDeviceCommand->AddChildCommand(m_pSeriesView->CreateDeviceCommand(pDC));

	for (i = 0; i < pAxisViews->GetCount(); i++)
	{
		CXTPChartAxisView* pAxisView = (CXTPChartAxisView*)pAxisViews->GetAt(i);
		pClipDeviceCommand->AddChildCommand(CreateConstantLinesDeviceCommand(pDC,pAxisView, FALSE));
	}


	return pCommand;

}

CXTPChartPointF CXTPChartDiagramPaneView::GetScreenPoint(const CXTPChartSeriesView* pView, double x, double y) const
{
	CRect rcBounds = GetBounds();

	CXTPChartPointF res;

	CXTPChartAxisView* pAxisViewX = ((CXTPChartDiagram2DSeriesView*)pView)->m_pAxisViewX;
	CXTPChartAxisView* pAxisViewY = ((CXTPChartDiagram2DSeriesView*)pView)->m_pAxisViewY;

	res.X = (float)pAxisViewX->ValueToPoint(x);
	res.Y = (float)pAxisViewY->ValueToPoint(y);

	CXTPChartDiagram2DView* pDiagramView = (CXTPChartDiagram2DView*)m_pParentView->GetParentView();
	CXTPChartDiagram2D* pDiagram = (CXTPChartDiagram2D*)pDiagramView->GetDiagram();

	if (pDiagram->IsRotated())
	{
		res = CXTPChartPointF(res.Y, res.X);
	}

	return res;
}


CXTPChartElementView* CXTPChartDiagramPaneView::CreateSeriesView()
{
	m_pSeriesView = new CXTPChartElementView(this);

	return m_pSeriesView;
}

void CXTPChartDiagramPaneView::OnLButtonDown(UINT /*nFlags*/, CPoint point)
{
	CXTPChartDiagram2DView* pDiagramView = (CXTPChartDiagram2DView*)m_pParentView->GetParentView();

	BOOL bScrollBarFound = FALSE;
	for (int i = 0; i < pDiagramView->m_pAxisViews->GetCount(); i++)
	{
		CXTPChartAxisView* pAxisView = (CXTPChartAxisView*)pDiagramView->m_pAxisViews->GetAt(i);

		if (pAxisView->IsScollBarVisible())
		{
			bScrollBarFound = TRUE;
		}
	}
	if (!bScrollBarFound)
		return;


	CXTPChartContainer* pContainer = m_pContainer;
	ASSERT (pContainer);

	m_pContainer->SetCapture(this);
	m_ptOldPosition = point;

	::SetCursor(pDiagramView->m_hcurDragHand);
}

void CXTPChartDiagramPaneView::OnMouseMove(UINT /*nFlags*/, CPoint point)
{
	int dx = point.x - m_ptOldPosition.x;
	int dy = point.y - m_ptOldPosition.y;
	m_ptOldPosition = point;

	CXTPChartDiagram2DView* pDiagramView = (CXTPChartDiagram2DView*)m_pParentView->GetParentView();

	for (int i = 0; i < pDiagramView->m_pAxisViews->GetCount(); i++)
	{
		CXTPChartAxisView* pAxisView = (CXTPChartAxisView*)pDiagramView->m_pAxisViews->GetAt(i);

		pAxisView->PerformPaneDragging(dx, dy);
	}

}

BOOL CXTPChartDiagramPaneView::OnSetCursor(CPoint /*point*/)
{
	CXTPChartDiagram2DView* pDiagramView = (CXTPChartDiagram2DView*)m_pParentView->GetParentView();

	::SetCursor(pDiagramView->m_hcurDragHand);
	return TRUE;
}
