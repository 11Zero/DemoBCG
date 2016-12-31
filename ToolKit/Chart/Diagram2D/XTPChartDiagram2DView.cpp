// XTPChartDiagram2D.cpp
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
#include "Common/XTPResourceManager.h"
#include "../Resource.h"

#include "XTPChartDiagram2D.h"
#include "XTPChartDiagram2DView.h"
#include "XTPChartAxis.h"
#include "../XTPChartTitle.h"
#include "XTPChartAxisView.h"
#include "XTPChartDiagramPane.h"
#include "XTPChartAxisGridLines.h"
#include "../XTPChartSeriesView.h"
#include "../XTPChartSeries.h"
#include "../XTPChartSeriesPoint.h"

#include "../Styles/Point/XTPChartDiagram2DSeriesStyle.h"
#include "../Styles/Point/XTPChartDiagram2DSeriesLabel.h"

#include "../XTPChartContent.h"
#include "../Appearance//XTPChartAppearance.h"
#include "../Appearance//XTPChartFillStyle.h"
#include "../Drawing/XTPChartTransformationDeviceCommand.h"

//////////////////////////////////////////////////////////////////////////
// CXTPChartDiagram2DView

CXTPChartDiagram2DView::CXTPChartDiagram2DView(CXTPChartDiagram* pDiagram, CXTPChartElementView* pParent)
	: CXTPChartDiagramView(pDiagram, pParent)
{
	m_pAxisViews = NULL;
	m_pPaneView = NULL;

	m_hcurNormalHand = XTPResourceManager()->LoadCursor(XTP_IDC_CHART_NORMALHAND);
	m_hcurDragHand = XTPResourceManager()->LoadCursor(XTP_IDC_CHART_DRAGHAND);

	m_rcLabelPadding.SetRectEmpty();
}

void CXTPChartDiagram2DView::AddAxisView(CXTPChartDeviceContext* pDC, CXTPChartElementView* pParentView, CXTPChartAxis* pAxis)
{
	CXTPChartAxisView* pAxisView = (CXTPChartAxisView*)pAxis->CreateView(pDC, pParentView);

	pAxisView->CreateView(pDC);
}

void CXTPChartDiagram2DView::UpdateRange(CXTPChartDeviceContext* pDC)
{
	int i;

	for (i = 0; i < GetSeriesView()->GetCount(); i++)
	{
		CXTPChartDiagram2DSeriesView* pSeriesView = (CXTPChartDiagram2DSeriesView*)GetSeriesView()->GetAt(i);

		pSeriesView->BeforeUpdateRange(pDC);
	}

	for (i = 0; i < m_pAxisViews->GetCount(); i++)
	{
		CXTPChartAxisView* pAxisView = (CXTPChartAxisView*)m_pAxisViews->GetAt(i);

		for (int j = 0; j < GetSeriesView()->GetCount(); j++)
		{
			CXTPChartSeriesView* pSeriesView = (CXTPChartSeriesView*)GetSeriesView()->GetAt(j);
			if (!pSeriesView->GetSeries()->IsVisible())
				continue;

			CXTPChartDiagram2DSeriesStyle* pStyle = DYNAMIC_DOWNCAST(CXTPChartDiagram2DSeriesStyle, pSeriesView->GetSeries()->GetStyle());

			ASSERT(pStyle);
			if (!pStyle)
				continue;


			BOOL bAxisX = !pAxisView->GetAxis()->IsValuesAxis();

			if (pAxisView->GetAxis()->IsSecondary() != (bAxisX ? pStyle->IsSecondaryAxisX() : pStyle->IsSecondaryAxisY()))
				continue;

			pAxisView->m_arrSeries.Add(pSeriesView);

			if (bAxisX)
				((CXTPChartDiagram2DSeriesView*)pSeriesView)->m_pAxisViewX = pAxisView;
			else
				((CXTPChartDiagram2DSeriesView*)pSeriesView)->m_pAxisViewY = pAxisView;
		}
	}

	for (i = 0; i < m_pAxisViews->GetCount(); i++)
	{
		CXTPChartAxisView* pAxisView = (CXTPChartAxisView*)m_pAxisViews->GetAt(i);

		pAxisView->GetAxis()->UpdateRange(pDC, pAxisView);

		pAxisView->AddLegendItems();
	}

	for (i = 0; i < GetSeriesView()->GetCount(); i++)
	{
		CXTPChartDiagram2DSeriesView* pSeriesView = (CXTPChartDiagram2DSeriesView*)GetSeriesView()->GetAt(i);

		pSeriesView->AfterUpdateRange(pDC);
	}
}

void CXTPChartDiagram2DView::CheckLabelBounds(const CXTPChartRectF& rcBounds)
{
	if (rcBounds.GetTop() < m_rcBounds.top)
	{
		m_rcLabelPadding.top = max(m_rcLabelPadding.top, m_rcBounds.top - (LONG)rcBounds.GetTop());
	}

	if (rcBounds.GetRight() > m_rcBounds.right)
	{
		m_rcLabelPadding.right = max(m_rcLabelPadding.right, (LONG)rcBounds.GetRight() - m_rcBounds.right);
	}

	if (rcBounds.GetLeft() < m_rcBounds.left)
	{
		m_rcLabelPadding.left = max(m_rcLabelPadding.left, m_rcBounds.left - (LONG)rcBounds.GetLeft());
	}

	if (rcBounds.GetBottom() > m_rcBounds.bottom)
	{
		m_rcLabelPadding.bottom = max(m_rcLabelPadding.bottom, (LONG)rcBounds.GetBottom() - m_rcBounds.bottom);
	}
}

void CXTPChartDiagram2DView::CalculateView(CXTPChartDeviceContext* pDC, CRect rcBounds)
{
	m_pDiagram->GetTitles()->CalculateView(pDC, rcBounds, m_pTitlesView);

	m_rcLabelPadding.SetRectEmpty();

	CRect rcDiagramBounds = rcBounds;

	for (int nUpdate = 0; nUpdate < 2; nUpdate++)
	{
		const int AXIS_GAP = 10;

		rcBounds = rcDiagramBounds;
		rcBounds.DeflateRect(m_rcLabelPadding);

		m_rcBounds = rcBounds;

		CRect rcInnerBounds(rcBounds);

		m_rcLabelPadding.SetRectEmpty();

		CRect rcPane(rcBounds);

		int i;

		((CXTPChartDiagram2D*)m_pDiagram)->UpdateLayout(pDC, this, rcBounds);


		CXTPChartDiagram2D* pDiagram = (CXTPChartDiagram2D*)GetDiagram();


		for (i = 0; i < m_pAxisViews->GetCount(); i++)
		{
			CXTPChartAxisView* pAxisView = (CXTPChartAxisView*)m_pAxisViews->GetAt(i);

			pAxisView->CalcSize(pDC, rcPane);

			if (!pAxisView->GetAxis()->IsVisible())
				continue;

			if (pAxisView->GetAxis()->IsVertical())
			{
				if (pAxisView->GetAxis()->GetAlignment() == xtpChartAlignNear)
				{
					if (rcPane.left != rcBounds.left) rcPane.left += AXIS_GAP;
					rcPane.left += pAxisView->GetSize();
				}
				else
				{
					if (rcPane.right != rcBounds.right) rcPane.right -= AXIS_GAP;
					rcPane.right -= pAxisView->GetSize();
				}
			}
			else
			{
				if (pAxisView->GetAxis()->GetAlignment() == xtpChartAlignNear)
				{
					if (rcPane.bottom != rcBounds.bottom) rcPane.bottom -= AXIS_GAP;
					rcPane.bottom -= pAxisView->GetSize();
				}
				else
				{
					if (rcPane.top != rcBounds.top) rcPane.top += AXIS_GAP;
					rcPane.top += pAxisView->GetSize();
				}
			}
		}



		for (i = m_pAxisViews->GetCount() - 1; i >= 0; i--)
		{
			CXTPChartAxisView* pAxisView = (CXTPChartAxisView*)m_pAxisViews->GetAt(i);
			int nSize = pAxisView->GetSize();


			if (pAxisView->GetAxis()->IsVertical())
			{
				if (pAxisView->GetAxis()->GetAlignment() == xtpChartAlignNear)
				{
					if (rcBounds.left != m_rcBounds.left) rcBounds.left += AXIS_GAP;
					pAxisView->SetBounds(pDC, CRect(rcBounds.left, rcPane.top, rcBounds.left + nSize, rcPane.bottom));
					rcBounds.left += nSize;
				}
				else
				{
					if (rcBounds.right != m_rcBounds.right) rcBounds.right -= AXIS_GAP;
					pAxisView->SetBounds(pDC, CRect(rcBounds.right - nSize, rcPane.top, rcBounds.right, rcPane.bottom));
					rcBounds.right -= nSize;
				}
			}
			else
			{
				if (pAxisView->GetAxis()->GetAlignment() == xtpChartAlignNear)
				{
					if (rcBounds.bottom != m_rcBounds.bottom) rcBounds.bottom -= AXIS_GAP;
					pAxisView->SetBounds(pDC, CRect(rcPane.left, rcBounds.bottom - nSize, rcPane.right, rcBounds.bottom));
					rcBounds.bottom -= nSize;
				}
				else
				{
					if (rcBounds.top != m_rcBounds.top) rcBounds.top += AXIS_GAP;
					pAxisView->SetBounds(pDC, CRect(rcPane.left, rcBounds.top, rcPane.right, rcBounds.top + nSize));
					rcBounds.top += nSize;
				}
			}
		}

		m_pPaneView->CalculateView(rcBounds);

		if (m_pPaneView->m_rcBounds.Width() < 1 || m_pPaneView->m_rcBounds.Height() < 1)
			break;

		pDiagram->CalculateSeriesLayout(pDC, this);


		for (i = 0; i < m_pLabelsView->GetCount(); i++)
		{
			CXTPChartDiagram2DSeriesLabelView* pLabelView = (CXTPChartDiagram2DSeriesLabelView*)m_pLabelsView->GetAt(i);

			pLabelView->CalculateLayout(pDC);
		}

		if( m_rcLabelPadding.IsRectNull())
			break;
	}
}

void CXTPChartDiagram2DView::CreateView(CXTPChartDeviceContext* pDC)
{
	CXTPChartDiagramView::CreateView(pDC);

	CXTPChartDiagram2D* pDiagram = (CXTPChartDiagram2D*)GetDiagram();

	CXTPChartElementView* pPaneView = new CXTPChartElementView(this);
	m_pAxisViews = new CXTPChartElementView(this);

	AddAxisView(pDC, m_pAxisViews, pDiagram->GetAxisX());
	AddAxisView(pDC, m_pAxisViews, pDiagram->GetSecondaryAxisX());

	AddAxisView(pDC, m_pAxisViews, pDiagram->GetAxisY());
	AddAxisView(pDC, m_pAxisViews, pDiagram->GetSecondaryAxisY());


	m_pPaneView = (CXTPChartDiagramPaneView*)pDiagram->GetPane()->CreateView(pDC, pPaneView);

	m_pSeriesView = m_pPaneView->CreateSeriesView();


	m_pLabelsView = new CXTPChartElementView(this);
}

CXTPChartDeviceCommand* CXTPChartDiagram2DView::CreateDeviceCommand(CXTPChartDeviceContext* pDC)
{
	if (m_pPaneView->m_rcBounds.Width() < 1 || m_pPaneView->m_rcBounds.Height() < 1)
		return NULL;
	return CXTPChartDiagramView::CreateDeviceCommand(pDC);
}

void CXTPChartDiagram2DView::OnMouseWheel(UINT /*nFlags*/, short zDelta, CPoint pt)
{
	CXTPChartDiagram2D* pDiagram = (CXTPChartDiagram2D*)GetDiagram();

	if (!pDiagram->IsAllowZoom())
		return;

	if (!m_pPaneView->GetBounds().PtInRect(pt))
		return;

	for (int i = 0; i < m_pAxisViews->GetCount(); i++)
	{
		CXTPChartAxisView* pAxisView = (CXTPChartAxisView*)m_pAxisViews->GetAt(i);

		pAxisView->PerformMouseWheel(zDelta, pt);
	}
}

BOOL CXTPChartDiagram2DView::OnSetCursor(CPoint point)
{
	if (!m_pPaneView->GetBounds().PtInRect(point))
	{
		return FALSE;
	}

	for (int i = 0; i < m_pAxisViews->GetCount(); i++)
	{
		CXTPChartAxisView* pAxisView = (CXTPChartAxisView*)m_pAxisViews->GetAt(i);

		if (pAxisView->IsScollBarVisible())
		{
			::SetCursor(m_hcurNormalHand);
			return TRUE;
		}
	}
	return FALSE;
}


void CXTPChartDiagram2DView::OnLButtonDown(UINT nFlags, CPoint point)
{

	CXTPChartDiagram2D* pDiagram = (CXTPChartDiagram2D*)GetDiagram();

	if (!pDiagram->IsAllowScroll())
		return;

	if (m_pPaneView->GetBounds().PtInRect(point))
	{
		m_pPaneView->OnLButtonDown(nFlags, point);
		return;
	}

	for (int i = 0; i < m_pAxisViews->GetCount(); i++)
	{
		CXTPChartAxisView* pAxisView = (CXTPChartAxisView*)m_pAxisViews->GetAt(i);

		if (pAxisView->GetBounds().PtInRect(point))
		{
			pAxisView->OnLButtonDown(nFlags, point);
			return;
		}
	}
}



//////////////////////////////////////////////////////////////////////////
// CXTPChartDiagram2DSeriesView

CXTPChartDiagram2DSeriesView::CXTPChartDiagram2DSeriesView(CXTPChartSeries* pSeries, CXTPChartDiagramView* pDiagramView, BOOL bSortPoints)
	: CXTPChartSeriesView(pSeries, pDiagramView)
{
	m_bSortPoints = bSortPoints;
	m_pAxisViewX = m_pAxisViewY = NULL;
}

CXTPChartPointF CXTPChartDiagram2DSeriesView::GetScreenPoint(double x, double y) const
{
	CXTPChartDiagram2DView* pDiagramView = (CXTPChartDiagram2DView*)GetDiagramView();

	CXTPChartDiagramPaneView* pPaneView = pDiagramView->GetPaneView();

	return pPaneView->GetScreenPoint(this, x, y);
}

int _cdecl CXTPChartDiagram2DSeriesView::ComparePoints(const CXTPChartSeriesPointView** ppPoint1, const CXTPChartSeriesPointView** ppPoint2)
{
	double d = (*ppPoint1)->GetPoint()->GetInternalArgumentValue() - (*ppPoint2)->GetPoint()->GetInternalArgumentValue();

	if (d > 0)
		return 1;
	if (d < 0)
		return -1;
	return 0;
}
void CXTPChartDiagram2DSeriesView::BeforeUpdateRange(CXTPChartDeviceContext* /*pDC*/)
{

}
void CXTPChartDiagram2DSeriesView::AfterUpdateRange(CXTPChartDeviceContext* /*pDC*/)
{
	if (m_bSortPoints)
	{
		typedef int (_cdecl *GENERICCOMPAREFUNC)(const void *, const void*);

		CXTPChartElementView** pChildren = m_pPointsView->GetChildren();

		qsort(pChildren, (size_t) m_pPointsView->GetCount(), sizeof(CXTPChartSeriesPointView*), (GENERICCOMPAREFUNC)ComparePoints);
	}
}
