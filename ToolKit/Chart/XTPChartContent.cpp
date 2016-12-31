// XTPChartContent.cpp
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
#include "Common/XTPMarkupRender.h"

#include "XTPChartContent.h"
#include "XTPChartDiagram.h"
#include "XTPChartDiagram3D.h"
#include "XTPChartTitle.h"
#include "XTPChartSeries.h"
#include "XTPChartSeriesView.h"
#include "XTPChartSeriesStyle.h"
#include "XTPChartSeriesPoint.h"

#include "XTPChartLegend.h"
#include "XTPChartPanel.h"

#include "Drawing/XTPChartDeviceContext.h"
#include "Drawing/XTPChartOpenGLDeviceContext.h"
#include "Drawing/XTPChartDeviceCommand.h"
#include "Drawing/XTPChartRectangleDeviceCommand.h"
#include "Appearance/XTPChartAppearance.h"
#include "Appearance/XTPChartBorder.h"



//////////////////////////////////////////////////////////////////////////
// CXTPChartContent
IMPLEMENT_DYNAMIC(CXTPChartContent, CXTPChartElement)

CXTPChartContent::CXTPChartContent()
{
	m_pTitles = new CXTPChartTitleCollection(this);

	m_pSeries = new CXTPChartSeriesCollection(this);

	m_pLegend = new CXTPChartLegend(this);

	m_pPanels = new CXTPChartPanelCollection(this);

	m_pAppearance = new CXTPChartAppearance(this);

	m_pBorder = new CXTPChartBorder(this);

	m_pMarkupContext = NULL;


	m_nPanelDistance = 10;
	m_nPanelDirection = xtpChartPanelHorizontal;


	CXTPChartDeviceContext::Register(TRUE);

}

CXTPChartContent::~CXTPChartContent()
{

	SAFE_RELEASE(m_pTitles);
	SAFE_RELEASE(m_pSeries);
	SAFE_RELEASE(m_pPanels);

	SAFE_RELEASE(m_pLegend);
	SAFE_RELEASE(m_pAppearance);
	SAFE_RELEASE(m_pBorder);

	XTPMarkupReleaseContext(m_pMarkupContext);

	CXTPChartDeviceContext::Register(FALSE);
}


CXTPChartContentView* CXTPChartContent::CreateView(CXTPChartDeviceContext* pDC, CRect rcBounds)
{
	CXTPChartContentView* pContentView = new CXTPChartContentView(pDC->GetContainer(), this);

	pContentView->CreateView(pDC);
	pContentView->CalculateView(pDC, rcBounds);


	return pContentView;
}


CXTPChartDeviceCommand* CXTPChartContent::CreateDeviceComand(CXTPChartDeviceContext* pDC, CRect rcBounds)
{
	CXTPChartElementView* pView = CreateView(pDC, rcBounds);

	CXTPChartDeviceCommand* pCommand = NULL;

	if (pView)
	{
		pCommand = pView->CreateDeviceCommand(pDC);
	}

	SAFE_RELEASE(pView);

	return pCommand;
}

void CXTPChartContent::DrawContent(CXTPChartDeviceContext* pDC, CRect rcBounds)
{
	CXTPChartDeviceCommand* pCommand = CreateDeviceComand(pDC, rcBounds);

	if (pCommand)
	{
		pDC->Execute(pCommand);

		delete pCommand;
	}
}

void CXTPChartContent::OnSeriesStyleChanged(CXTPChartSeries* pSeries)
{
	int i;

	ASSERT(pSeries->GetStyle());

	CXTPChartDiagram* pDiagram = pSeries->GetDiagram();

	if (pDiagram)
	{
		if (pSeries->GetStyle()->IsStyleDiagram(pSeries->GetDiagram()))
			return;

		pSeries->SetDiagram(NULL);
	}


	for (i = 0; i < m_pPanels->GetCount(); i++)
	{
		CXTPChartDiagram* pDiagram = DYNAMIC_DOWNCAST(CXTPChartDiagram, m_pPanels->GetAt(i));
		if (!pDiagram)
			continue;

		if (pSeries->SetDiagram(pDiagram))
		{
			return;
		}
	}

	pDiagram = pSeries->GetStyle()->CreateDiagram();

	VERIFY(pSeries->SetDiagram(pDiagram));

	m_pPanels->Add(pDiagram);
}

void CXTPChartContent::UpdateDiagram()
{
	int i;
	int nIndex = 0;

	for (i = 0; i < GetSeries()->GetCount(); i++)
	{
		CXTPChartSeries* pSeries = GetSeries()->GetAt(i);
		if (!pSeries->IsVisible())
			continue;

		if (!pSeries->GetStyle())
			continue;

		if (pSeries->GetStyle()->IsColorEach())
		{
			for (int j = 0; j < pSeries->GetPoints()->GetCount(); j++)
			{
				pSeries->GetPoints()->GetAt(j)->m_nPaletteIndex = nIndex++;
			}
		}
		else
		{
			pSeries->m_nPaletteIndex = nIndex++;
		}
	}
}


BOOL CXTPChartContent::Has3DDiagram() const
{
	for (int i = 0; i < m_pPanels->GetCount(); i++)
	{
		CXTPChartDiagram3D* pDiagram = DYNAMIC_DOWNCAST(CXTPChartDiagram3D, m_pPanels->GetAt(i));

		if (pDiagram)
		{
			return TRUE;
		}
	}
	return FALSE;
}

CXTPChartDeviceContext* CXTPChartContent::CreateDeviceContext(CXTPChartContainer* pContainer, HDC hDC, CRect rcBounds, BOOL bWindowDC)
{
	UpdateDiagram();

	if (Has3DDiagram())
	{
		return new CXTPChartOpenGLDeviceContext(pContainer, hDC, rcBounds.Size(), bWindowDC);
	}
	else
	{
		return new CXTPChartDeviceContext(pContainer, hDC);
	}
}


CXTPChartDiagram* CXTPChartContent::GetPrimaryDiagram() const
{
	if (m_pPanels->GetCount() == 0)
		return NULL;
	return DYNAMIC_DOWNCAST(CXTPChartDiagram, m_pPanels->GetAt(0));
}

CXTPChartColor CXTPChartContent::GetActualBackgroundColor() const
{
	if (m_clrBackground.IsEmpty())
		return m_pAppearance->GetContentAppearance()->BackgroundColor;

	return m_clrBackground;
}


CXTPChartColor CXTPChartContent::GetActualBorderColor() const
{
	if (m_pBorder->GetColor().IsEmpty())
		return m_pAppearance->GetContentAppearance()->BorderColor;

	return m_pBorder->GetColor();
}

CXTPChartColor CXTPChartContent::GetBackgroundColor() const
{
	return m_clrBackground;
}

void CXTPChartContent::OnChartChanged(XTPChartUpdateOptions updateOptions /* = xtpChartUpdateView */)
{
	for (int i = 0; i < m_arrContainers.GetSize(); i++)
	{
		m_arrContainers[i]->OnChartChanged(updateOptions);
	}
}

void CXTPChartContent::DoPropExchange(CXTPPropExchange* pPX)
{
	PX_Int(pPX, _T("PanelDistance"), m_nPanelDistance, 10);
	PX_Enum(pPX, _T("PanelDirection"), m_nPanelDirection, xtpChartPanelHorizontal);
	PX_Color(pPX, _T("Background"), m_clrBackground);

	CXTPPropExchangeSection secBorder(pPX->GetSection(_T("Border")));
	m_pBorder->DoPropExchange(&secBorder);

	CXTPPropExchangeSection secLegend(pPX->GetSection(_T("Legend")));
	m_pLegend->DoPropExchange(&secLegend);

	CXTPPropExchangeSection secTitles(pPX->GetSection(_T("Titles")));
	m_pTitles->DoPropExchange(&secTitles);

	CXTPPropExchangeSection secPanels(pPX->GetSection(_T("Panels")));
	m_pPanels->DoPropExchange(&secPanels);

	CXTPPropExchangeSection secSeries(pPX->GetSection(_T("Series")));
	m_pSeries->DoPropExchange(&secSeries);
}

void CXTPChartContent::EnableMarkup(BOOL bEnable)
{
	XTPMarkupReleaseContext(m_pMarkupContext);

	if (bEnable)
	{
		m_pMarkupContext = XTPMarkupCreateContext();
	}
}

void CXTPChartContent::AddContainer(CXTPChartContainer* pContainer)
{
	m_arrContainers.Add(pContainer);
}

void CXTPChartContent::RemoveContainer(CXTPChartContainer* pContainer)
{
	for (int i = 0; i < m_arrContainers.GetSize(); i++)
	{
		if (m_arrContainers.GetAt(i) == pContainer)
		{
			m_arrContainers.RemoveAt(i);
			return;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
// CXTPChartContentView


CXTPChartContentView::CXTPChartContentView(CXTPChartContainer* pContainer, CXTPChartContent* pContent)
	: CXTPChartElementView(pContainer)
{
	m_pContent = pContent;
	m_rcBounds.SetRectEmpty();

	m_pLegendView = NULL;
	m_pTitlesView = NULL;
	m_pDiagramView = NULL;
}

CXTPChartDeviceCommand* CXTPChartContentView::CreateDeviceCommand(CXTPChartDeviceContext* pDC)
{
	CXTPChartDeviceCommand* pCommand = new CXTPChartHitTestElementCommand(m_pContent, m_rcBounds);

	pCommand->AddChildCommand(new CXTPChartContentBackgroundDeviceCommand(m_rcBounds, m_pContent->GetActualBackgroundColor()));

	CXTPChartDeviceCommand* pDrawingType = pCommand->AddChildCommand(new CXTPChartDrawingTypeDeviceCommand(TRUE));

	if (m_pContent->GetBorder()->IsVisible())
	{
		pDrawingType->AddChildCommand(m_pContent->GetBorder()->CreateInnerBorderDeviceCommand(m_rcBounds, m_pContent->GetActualBorderColor()));
	}

	if (m_pDiagramView)
	{
		pDrawingType->AddChildCommand(m_pDiagramView->CreateDeviceCommand(pDC));
	}

	if (m_pLegendView)
	{
		pDrawingType->AddChildCommand(m_pLegendView->CreateDeviceCommand(pDC));
	}

	if (m_pTitlesView)
	{
		pDrawingType->AddChildCommand(m_pTitlesView->CreateDeviceCommand(pDC));
	}

	return pCommand;
}


void CXTPChartContentView::CreateView(CXTPChartDeviceContext* pDC)
{

	m_pTitlesView = new CXTPChartElementView(this);

	m_pLegendView = m_pContent->GetLegend()->CreateView(pDC, this);

	m_pDiagramView = new CXTPChartElementView(this);

	for (int nPanel = 0; nPanel < m_pContent->GetPanels()->GetCount(); nPanel++)
	{
		CXTPChartPanel* pPanel = m_pContent->GetPanels()->GetAt(nPanel);

		CXTPChartDiagram* pDiagram = DYNAMIC_DOWNCAST(CXTPChartDiagram, pPanel);
		if (pDiagram == NULL)
			continue;

		CXTPChartDiagramView* pDiagramView = pDiagram->CreateView(pDC, m_pDiagramView);

		if (pDiagramView)
		{
			pDiagramView->CreateView(pDC);

			for (int i = 0; i < m_pContent->GetSeries()->GetCount(); i++)
			{
				CXTPChartSeries* pSeries = m_pContent->GetSeries()->GetAt(i);
				if (!pSeries->IsVisible())
					continue;

				if (pSeries->GetDiagram() != pDiagram)
					continue;


				CXTPChartSeriesView* pSeriesView = pSeries->GetStyle()->CreateView(pSeries, pDiagramView);

				if (pSeriesView)
				{
					pSeriesView->CreatePointsView(pDC);

					if (m_pLegendView)
					{
						pSeriesView->AddToLegend(m_pLegendView);
					}
				}
			}

			pDiagramView->UpdateRange(pDC);
		}
	}

	m_pContent->GetTitles()->CreateView(pDC, m_pTitlesView);

}

CXTPChartDiagramView* CXTPChartContentView::HitTestDiagramView(CPoint pt) const
{
	for (int i = 0; i < m_pDiagramView->GetCount(); i++)
	{
		CXTPChartDiagramView* pDiagramView = (CXTPChartDiagramView*)m_pDiagramView->GetAt(i);

		if (pDiagramView->GetBounds().PtInRect(pt))
			return pDiagramView;
	}
	return NULL;
}


void CXTPChartContentView::CalculateView(CXTPChartDeviceContext* pDC, CRect rcBounds)
{
	m_rcBounds = rcBounds;

	if (m_pContent->GetBorder()->IsVisible())
		rcBounds.DeflateRect(m_pContent->GetBorder()->GetThickness(), m_pContent->GetBorder()->GetThickness());

	m_pContent->GetTitles()->CalculateView(pDC, rcBounds, m_pTitlesView);

	rcBounds.DeflateRect(10, 10, 10, 10);


	if (m_pLegendView)
	{
		m_pLegendView->CalculateView(pDC, rcBounds);
	}

	int nCount = m_pDiagramView->GetCount();
	if (nCount == 0)
		return;

	int nGap = m_pContent->GetPanelDistance();

	int nSize = m_pContent->GetPanelDirection() == xtpChartPanelHorizontal ?
		(rcBounds.Width() - nGap * (nCount - 1)) / nCount :
		(rcBounds.Height() - nGap * (nCount - 1)) / nCount;


	for (int i = 0; i < nCount; i++)
	{
		CXTPChartDiagramView* pDiagramView = (CXTPChartDiagramView*)m_pDiagramView->GetAt(i);

		CRect rcDiagram =
			m_pContent->GetPanelDirection() == xtpChartPanelHorizontal ?
				CRect(rcBounds.left + i * (nSize + nGap), rcBounds.top, rcBounds.left + i * (nSize + nGap) + nSize, rcBounds.bottom) :
				CRect(rcBounds.left, rcBounds.top + i * (nSize + nGap), rcBounds.right, rcBounds.top + i * (nSize + nGap) + nSize);

		pDiagramView->CalculateView(pDC, rcDiagram);
	}
}

CXTPChartLegendView* CXTPChartContentView::GetLegendView() const
{
	return m_pLegendView;
}



