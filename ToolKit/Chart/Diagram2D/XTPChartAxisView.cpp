// XTPChartAxisView.cpp
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
#include <math.h>
#include "XTPChartDiagram2D.h"
#include "XTPChartAxis.h"
#include "XTPChartAxisView.h"
#include "XTPChartAxisRange.h"
#include "XTPChartAxisLabel.h"
#include "XTPChartAxisGridLines.h"
#include "XTPChartAxisTickMarks.h"
#include "XTPChartAxisTitle.h"
#include "XTPChartAxisConstantLines.h"
#include "XTPChartAxisStrips.h"
#include "XTPChartAxisCustomLabels.h"
#include "XTPChartScaleTypeMap.h"

#include "../XTPChartContent.h"
#include "../XTPChartLegend.h"

#include "../Drawing/XTPChartDeviceContext.h"
#include "../Drawing/XTPChartLineDeviceCommand.h"
#include "../Drawing/XTPChartRectangleDeviceCommand.h"

#include "../Utils/XTPChartTextPainter.h"
#include "../Utils/XTPChartMathUtils.h"
#include "../Appearance/XTPChartLineStyle.h"
#include "../Appearance/XTPChartFillStyle.h"

#define  SCROLLBARSIZE 10

//////////////////////////////////////////////////////////////////////////

// CXTPChartAxisViewTickEnumerator
//===========================================================================
// Summary:
//     This class helps to enumerates the axis tick marks.
// Remarks:
// See Also:
//===========================================================================
class CXTPChartAxisViewTickEnumerator
{
public:

	//-----------------------------------------------------------------------
	// Summary:
	//     Constructs a CXTPChartAxisViewTickEnumerator object.
	// Parameters:
	//     pView   - Pointer to an axis view object.
	//     bMinor  - The tick mark is minor or major.
	// Remarks:
	// See Also:
	//-----------------------------------------------------------------------
	CXTPChartAxisViewTickEnumerator(CXTPChartAxisView* pView, BOOL bMinor);

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to get the next tick mark.
	// Parameters:
	//     dMark   - Reference to a double to pass the next tick mark value out.
	// Remarks:
	// See Also:
	//-----------------------------------------------------------------------
	BOOL GetNext(double& dMark);

protected:
	CXTPChartAxisView* m_pView;      //Pointer to axis view object.
	BOOL m_bMinor;                  //TRUE if the tick mark is major and FALSE if it is minor.
	int m_nMinorCount;              //The number of minors present.
	double m_dMinValue;             //The minimum value of the axis.
	double m_dMaxValue;             //The maximum value of the axis.
	double m_dGridSpacing;          //The spcing between the grid lines.
	double m_dMinordGridSpacing;    //The spacing between the minor tick marks.
	int m_nCurMinor;                //The current minor.

	double m_dMark;                 //The tick current mark value.
	int m_nCurCustomLabel;
};

CXTPChartAxisViewTickEnumerator::CXTPChartAxisViewTickEnumerator(CXTPChartAxisView* pView, BOOL bMinor)
{
	m_pView = pView;
	m_bMinor = bMinor;
	m_nMinorCount = pView->GetAxis()->GetMinorCount();

	m_dMinValue = pView->GetViewRangeMinValue();
	m_dMaxValue = pView->GetViewRangeMaxValue();
	m_dGridSpacing = pView->GetGridSpacing();

	m_dMinordGridSpacing = m_dGridSpacing / (m_nMinorCount + 1);

	m_dMark = ((int)(m_dMinValue / m_dGridSpacing)) * m_dGridSpacing;
	if (m_dMark < m_dMinValue)
		m_dMark += m_dGridSpacing;

	m_nCurMinor = 0;

	if (m_bMinor)
	{
		m_dMark -= m_dGridSpacing;
		while (m_dMark < m_dMinValue)
		{
			m_dMark += m_dMinordGridSpacing;
			m_nCurMinor++;
		}
	}

	m_nCurCustomLabel = -1;
	if (pView->GetAxis()->GetCustomLabels()->GetCount())
		m_nCurCustomLabel = 0;

}

BOOL CXTPChartAxisViewTickEnumerator::GetNext(double& dMark)
{
	if (m_bMinor && m_nCurCustomLabel != -1)
		return FALSE;

	if (m_nCurCustomLabel != -1)
	{
		if (m_nCurCustomLabel >= m_pView->GetAxis()->GetCustomLabels()->GetCount())
			return FALSE;

		CXTPChartAxisCustomLabel* pLabel = m_pView->GetAxis()->GetCustomLabels()->GetAt(m_nCurCustomLabel);

		dMark = !pLabel->GetAxisValue().IsEmpty() ? m_pView->GetAxis()->GetScaleTypeMap()->ValueToInternal(pLabel->GetAxisValue()) :
				pLabel->GetAxisValueInternal();

		m_nCurCustomLabel++;
		return TRUE;
	}


	if (m_dMark > m_dMaxValue)
		return FALSE;

	dMark = m_dMark;

	if (m_bMinor)
	{
		m_dMark += m_dMinordGridSpacing;
		m_nCurMinor += 1;

		if (m_nCurMinor == m_nMinorCount + 1)
		{
			m_dMark += m_dMinordGridSpacing;
			m_nCurMinor = 1;
		}
	}
	else
	{
		m_dMark += m_dGridSpacing;
	}

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
// CXTPChartAxisView

CXTPChartAxisView::CXTPChartAxisView(CXTPChartAxis* pAxis, CXTPChartElementView* pParentView)
	: CXTPChartElementView(pParentView)
{
	m_pAxis = pAxis;

	m_dGridSpacing = 1;


	ASSERT(m_pContainer);

	m_nSize = 0;
}

CXTPChartAxisView::~CXTPChartAxisView()
{
}

double CXTPChartAxisView::GetRangeMinValue() const
{
	return m_pAxis->GetRange()->GetMinValue();
}
double CXTPChartAxisView::GetRangeMaxValue() const
{
	return m_pAxis->GetRange()->GetMaxValue();
}
double CXTPChartAxisView::GetViewRangeMinValue() const
{
	return max(m_pAxis->GetRange()->GetViewMinValue(), GetRangeMinValue());
}
double CXTPChartAxisView::GetViewRangeMaxValue() const
{
	return min(m_pAxis->GetRange()->GetViewMaxValue(), GetRangeMaxValue());
}

CXTPChartDeviceCommand* CXTPChartAxisView::CreateInterlacedDeviceCommand(CXTPChartDeviceContext* pDC, CRect rcPane)
{
	UNREFERENCED_PARAMETER(pDC);

	if (!m_pAxis->IsInterlaced())
		return NULL;

	if (m_arrTicks.GetSize() < 1)
		return NULL;

	CXTPChartDeviceCommand* pCommands = new CXTPChartDeviceCommand();

	BOOL bVertical = IsVertical();

	BOOL bReversed = m_pAxis->IsReversed();

	int nLeft = bVertical ?  (bReversed ? m_rcBounds.top : m_rcBounds.bottom) : (bReversed ? m_rcBounds.right : m_rcBounds.left);

	int i = 0;
	double dMark;

	double dGridSpacing = GetGridSpacing();

	int nCount = int((m_arrTicks[0].m_dValue - GetRangeMinValue()) / dGridSpacing);

	if ((nCount & 1) == 1)
	{
		dMark = m_arrTicks[0].m_dValue;
		nLeft = (int)ValueToPoint(dMark);

		i = 1;
	}

	while (i <= m_arrTicks.GetSize())
	{

		int nRight;

		if (i >= m_arrTicks.GetSize())
		{
			nRight = bVertical ?  (!bReversed ? m_rcBounds.top : m_rcBounds.bottom) : (!bReversed ? m_rcBounds.right : m_rcBounds.left);
		}
		else
		{
			dMark = m_arrTicks[i].m_dValue;
			nRight = (int)ValueToPoint(dMark);

		}

		i++;

		CXTPChartRectF rc = bVertical ? CXTPChartRectF((float)rcPane.left, (float)nRight, (float)rcPane.Width(), (float)nLeft - nRight) :
			CXTPChartRectF((float)nLeft, (float)rcPane.top, (float)nRight - nLeft, (float)rcPane.Height());

		if (rc.Width < 0) rc.X += rc.Width, rc.Width = -rc.Width;
		if (rc.Height < 0) rc.Y += rc.Height, rc.Height = -rc.Height;


		pCommands->AddChildCommand(m_pAxis->GetInterlacedFillStyle()->CreateDeviceCommand(rc,
			m_pAxis->GetActualInterlacedColor(), m_pAxis->GetActualInterlacedColor2()));

		if (i >= m_arrTicks.GetSize())
			break;

		dMark = m_arrTicks[i].m_dValue;
		nLeft = (int)ValueToPoint(dMark);

		i++;
	}


	return pCommands;
}

CXTPChartDeviceCommand* CXTPChartAxisView::CreateStripsDeviceCommand(CXTPChartDeviceContext* pDC, CRect rcPane)
{
	UNREFERENCED_PARAMETER(pDC);

	CXTPChartAxisStrips* pStrips = m_pAxis->GetStrips();
	if (pStrips->GetCount() == 0)
		return NULL;

	CXTPChartDeviceCommand* pCommands = new CXTPChartDeviceCommand();
	BOOL bVertical = IsVertical();

	for (int i = 0; i < pStrips->GetCount(); i++)
	{
		CXTPChartAxisStrip* pStrip = pStrips->GetAt(i);
		if (!pStrip->IsVisible())
			continue;

		double dMarkLeft = !pStrip->GetAxisMinValue().IsEmpty() ? m_pAxis->GetScaleTypeMap()->ValueToInternal(pStrip->GetAxisMinValue()) :
			pStrip->GetAxisMinValueInternal();

		double dMarkRight = !pStrip->GetAxisMaxValue().IsEmpty() ? m_pAxis->GetScaleTypeMap()->ValueToInternal(pStrip->GetAxisMaxValue()) :
			pStrip->GetAxisMaxValueInternal();

		int nLeft = (int)ValueToPoint(dMarkLeft);
		int nRight = (int)ValueToPoint(dMarkRight);

		CXTPChartRectF rc;
		if (bVertical)
		{
			rc = CXTPChartRectF((float)rcPane.left, (float)nRight, (float)rcPane.Width(), (float)nLeft - nRight);
		}
		else
		{
			rc = CXTPChartRectF((float)nLeft, (float)rcPane.top, (float)nRight - nLeft, (float)rcPane.Height());
		}

		rc.Normalize();

		pCommands->AddChildCommand(pStrip->GetFillStyle()->CreateDeviceCommand(rc, pStrip->GetActualColor(), pStrip->GetActualColor2()));

	}

	return pCommands;
}

CXTPChartDeviceCommand* CXTPChartAxisView::CreateConstantLinesDeviceCommand(CXTPChartDeviceContext* pDC, CRect rcPane, BOOL bBehind)
{
	CXTPChartAxisConstantLines* pConstantLines = m_pAxis->GetConstantLines();
	if (pConstantLines->GetCount() == 0)
		return NULL;

	CXTPChartDeviceCommand* pCommands = new CXTPChartDeviceCommand();
	BOOL bVertical = IsVertical();

	for (int i = 0; i < pConstantLines->GetCount(); i++)
	{
		CXTPChartAxisConstantLine* pConstantLine = pConstantLines->GetAt(i);
		if (!pConstantLine->IsVisible())
			continue;

		if (pConstantLine->IsShowBehind() != bBehind)
			continue;

		double dMark = !pConstantLine->GetAxisValue().IsEmpty() ? m_pAxis->GetScaleTypeMap()->ValueToInternal(pConstantLine->GetAxisValue()) :
			pConstantLine->GetAxisValueInternal();

		int nLeft = (int)ValueToPoint(dMark);

		if (bVertical)
		{
			pCommands->AddChildCommand(pConstantLine->GetLineStyle()->CreateDeviceCommand(CXTPChartDiagramPoint(rcPane.left, nLeft),
				CXTPChartDiagramPoint(rcPane.right, nLeft), pConstantLine->GetActualColor()));
		}
		else
		{
			pCommands->AddChildCommand(pConstantLine->GetLineStyle()->CreateDeviceCommand(CXTPChartDiagramPoint(nLeft, rcPane.top),
				CXTPChartDiagramPoint(nLeft, rcPane.bottom), pConstantLine->GetActualColor()));
		}

		CXTPChartString strText = pConstantLine->GetText();

		if (bVertical)
		{
			CXTPChartTextPainter painter(pDC, strText, pConstantLine);
			int x = pConstantLine->GetAlignment() == xtpChartAlignNear ? rcPane.left + 2 :
				pConstantLine->GetAlignment() == xtpChartAlignFar ? rcPane.right - 2 - (int)painter.GetSize().Width :
				(rcPane.left + rcPane.right - (int)painter.GetSize().Width) / 2;

			painter.SetLocation(CPoint(x, pConstantLine->IsTextBelow() ? nLeft + 2 : nLeft - (int)painter.GetSize().Height));
			pCommands->AddChildCommand(painter.CreateDeviceCommand(pDC, pConstantLine->GetActualTextColor()));
		}
		else
		{
			CXTPChartRotatedTextPainterNearLine painter(pDC, strText, pConstantLine, CPoint(0, 0), TRUE ? xtpChartTextNearRight : xtpChartTextNearTop, 90);

			int y = pConstantLine->GetAlignment() == xtpChartAlignNear ? rcPane.bottom - (int)painter.GetSize().Width / 2 :
				pConstantLine->GetAlignment() == xtpChartAlignFar ? rcPane.top + (int)painter.GetSize().Width / 2 : (rcPane.top + rcPane.bottom) / 2;

			painter.SetBasePoint(CPoint(pConstantLine->IsTextBelow() ? nLeft + 2 - (int)painter.GetSize().Height : nLeft + 2, y));

			pCommands->AddChildCommand(painter.CreateDeviceCommand(pDC, pConstantLine->GetActualTextColor()));

		}
	}

	return pCommands;
}

CXTPChartDeviceCommand* CXTPChartAxisView::CreateGridLinesDeviceCommand(CXTPChartDeviceContext* pDC, CRect rcPane)
{
	UNREFERENCED_PARAMETER(pDC);

	CXTPChartAxisGridLines* pGridLines = m_pAxis->GetGridLines();

	if (!pGridLines->IsVisible())
		return NULL;

	CXTPChartDeviceCommand* pCommands = new CXTPChartDeviceCommand();

	double dMark;
	CXTPChartColor clrGridLines = pGridLines->GetColor();
	CXTPChartColor clrMinorGridLines = pGridLines->GetMinorColor();
	BOOL bMinorVisible = pGridLines->IsMinorVisible();

	BOOL bVertical = IsVertical();

	for (int i = 0; i < m_arrTicks.GetSize(); i++)
	{
		int nLeft = (int)ValueToPoint(m_arrTicks[i].m_dValue);

		if (bVertical)
		{
			pCommands->AddChildCommand(pGridLines->GetLineStyle()->CreateDeviceCommand(CXTPChartDiagramPoint(rcPane.left, nLeft),
				CXTPChartDiagramPoint(rcPane.right, nLeft), clrGridLines));
		}
		else
		{
			pCommands->AddChildCommand(pGridLines->GetLineStyle()->CreateDeviceCommand(CXTPChartDiagramPoint(nLeft, rcPane.top),
				CXTPChartDiagramPoint(nLeft, rcPane.bottom), clrGridLines));
		}
	}

	if (bMinorVisible)
	{
		CXTPChartAxisViewTickEnumerator* pEmumerator = CreateTickMarksEnumerator(TRUE);

		while (pEmumerator->GetNext(dMark))
		{
			int nLeft = (int)ValueToPoint(dMark);

			if (bVertical)
			{
				pCommands->AddChildCommand(pGridLines->GetMinorLineStyle()->CreateDeviceCommand(CXTPChartDiagramPoint(rcPane.left, nLeft),
					CXTPChartDiagramPoint(rcPane.right, nLeft), clrMinorGridLines));
			}
			else
			{
				pCommands->AddChildCommand(pGridLines->GetMinorLineStyle()->CreateDeviceCommand(CXTPChartDiagramPoint(nLeft, rcPane.top),
					CXTPChartDiagramPoint(nLeft, rcPane.bottom), clrMinorGridLines));
			}
		}
		delete pEmumerator;
	}

	return pCommands;
}

double CXTPChartAxisView::GetScale() const
{
	double dMinValue = GetViewRangeMinValue();
	double dMaxValue = GetViewRangeMaxValue();

	if (IsVertical())
	{
		return (m_rcBounds.Height()) / (dMaxValue - dMinValue);
	}
	else
	{
		return (m_rcBounds.Width()) / (dMaxValue - dMinValue);
	}
}

double CXTPChartAxisView::DistanceToPoint(double x) const
{
	double dMinValue = GetViewRangeMinValue();
	double dMaxValue = GetViewRangeMaxValue();

	if (IsVertical())
	{
		return x / (dMaxValue - dMinValue) * (m_rcBounds.Height());
	}
	else
	{
		return x / (dMaxValue - dMinValue) * (m_rcBounds.Width());
	}
}

double CXTPChartAxisView::ValueToPoint(double x) const
{
	double dMinValue = GetViewRangeMinValue();
	double dMaxValue = GetViewRangeMaxValue();

	BOOL bRevered = m_pAxis->IsReversed();

	double dValue;

	if (bRevered)
	{
		if (IsVertical())
		{
			dValue = m_rcBounds.top + (x - dMinValue) / (dMaxValue - dMinValue) * (m_rcBounds.Height());
		}
		else
		{
			dValue = m_rcBounds.right - (x - dMinValue) / (dMaxValue - dMinValue) * (m_rcBounds.Width());
		}
	}
	else
	{
		if (IsVertical())
		{
			dValue = m_rcBounds.bottom - (x - dMinValue) / (dMaxValue - dMinValue) * (m_rcBounds.Height());
		}
		else
		{
			dValue = m_rcBounds.left + (x - dMinValue) / (dMaxValue - dMinValue) * (m_rcBounds.Width());
		}
	}

	return CXTPChartMathUtils::Round(dValue);
}

CXTPChartAxisViewTickEnumerator* CXTPChartAxisView::CreateTickMarksEnumerator(BOOL bMinor)
{
	return new CXTPChartAxisViewTickEnumerator(this, bMinor);
}

CXTPChartDeviceCommand* CXTPChartAxisView::CreateTickMarksDeviceCommand(CXTPChartDeviceContext* pDC)
{
	UNREFERENCED_PARAMETER(pDC);

	CXTPChartDeviceCommand* pCommands = new CXTPChartHitTestElementCommand(m_pAxis, m_rcBounds);

	CXTPChartAxisTickMarks* pTickMarks = m_pAxis->GetTickMarks();

	if (!pTickMarks->IsVisible())
		return pCommands;

	int nScrollBarSize = IsScollBarVisible() ? SCROLLBARSIZE : 0;

	CXTPChartColor clrAxis = m_pAxis->GetActualColor();
	int nLength = pTickMarks->GetLength();
//  int nMinorCount = m_pAxis->GetMinorCount();
	int nMinorThickness = pTickMarks->GetMinorThickness();
	int nThickness = pTickMarks->GetThickness();
	BOOL bMinorVisible = pTickMarks->IsMinorVisible();
	BOOL bVertical = IsVertical();
	BOOL bCross = pTickMarks->IsCrossAxis();
	int nAxisThickness = m_pAxis->GetThickness() - 1 + nScrollBarSize;
	int nExtraLength = bCross ? nLength + nAxisThickness : 0;

	double dMark;

	for (int i = 0; i < m_arrTicks.GetSize(); i++)
	{
		int nLeft = (int)ValueToPoint(m_arrTicks[i].m_dValue);

		if (bVertical)
		{
			if (m_pAxis->GetAlignment() == xtpChartAxisNear)
				pCommands->AddChildCommand(new CXTPChartSolidLineDeviceCommand(CXTPChartDiagramPoint(m_rcBounds.right - nAxisThickness - nLength, nLeft),
					CXTPChartDiagramPoint(m_rcBounds.right - nAxisThickness + nExtraLength, nLeft), clrAxis, nThickness));
			else
				pCommands->AddChildCommand(new CXTPChartSolidLineDeviceCommand(CXTPChartDiagramPoint(m_rcBounds.left + nAxisThickness - nExtraLength, nLeft),
					CXTPChartDiagramPoint(m_rcBounds.left + nAxisThickness + nLength, nLeft), clrAxis, nThickness));
		}
		else
		{
			if (m_pAxis->GetAlignment() == xtpChartAxisNear)
				pCommands->AddChildCommand(new CXTPChartSolidLineDeviceCommand(CXTPChartDiagramPoint(nLeft, m_rcBounds.top + nAxisThickness - nExtraLength),
					CXTPChartDiagramPoint(nLeft, m_rcBounds.top + nAxisThickness + nLength), clrAxis, nThickness));
			else
				pCommands->AddChildCommand(new CXTPChartSolidLineDeviceCommand(CXTPChartDiagramPoint(nLeft, m_rcBounds.bottom - nAxisThickness - nLength),
					CXTPChartDiagramPoint(nLeft, m_rcBounds.bottom - nAxisThickness + nExtraLength), clrAxis, nThickness));
		}
	}


	if (bMinorVisible)
	{
		nLength = pTickMarks->GetMinorLength();
		int nExtraLength = bCross ? nLength + nAxisThickness : 0;

		CXTPChartAxisViewTickEnumerator* pEmumerator = CreateTickMarksEnumerator(TRUE);

		while (pEmumerator->GetNext(dMark))
		{
			int nLeft = (int)ValueToPoint(dMark);

			if (bVertical)
			{
				if (m_pAxis->GetAlignment() == xtpChartAxisNear)
					pCommands->AddChildCommand(new CXTPChartSolidLineDeviceCommand(CXTPChartDiagramPoint(m_rcBounds.right - nAxisThickness  - nLength, nLeft),
					CXTPChartDiagramPoint(m_rcBounds.right - nAxisThickness + nExtraLength, nLeft), clrAxis, nMinorThickness));
				else
					pCommands->AddChildCommand(new CXTPChartSolidLineDeviceCommand(CXTPChartDiagramPoint(m_rcBounds.left + nAxisThickness  - nExtraLength, nLeft),
					CXTPChartDiagramPoint(m_rcBounds.left + nAxisThickness + nLength, nLeft), clrAxis, nMinorThickness));
			}
			else
			{
				if (m_pAxis->GetAlignment() == xtpChartAxisNear)
					pCommands->AddChildCommand(new CXTPChartSolidLineDeviceCommand(CXTPChartDiagramPoint(nLeft, m_rcBounds.top + nAxisThickness  - nExtraLength),
					CXTPChartDiagramPoint(nLeft, m_rcBounds.top + nAxisThickness + nLength), clrAxis, nMinorThickness));
				else
					pCommands->AddChildCommand(new CXTPChartSolidLineDeviceCommand(CXTPChartDiagramPoint(nLeft, m_rcBounds.bottom - nAxisThickness - nLength),
					CXTPChartDiagramPoint(nLeft, m_rcBounds.bottom - nAxisThickness + nExtraLength), clrAxis, nMinorThickness));
			}
		}

		delete pEmumerator;
	}


	return pCommands;
}

CXTPChartDeviceCommand* CXTPChartAxisView::CreateLabelsDeviceCommand(CXTPChartDeviceContext* pDC)
{
	if (!m_pAxis->GetLabel()->IsVisible())
		return NULL;

	CXTPChartDeviceCommand* pLabelsCommand = new CXTPChartHitTestElementCommand(m_pAxis->GetLabel(), m_rcBounds);
	CXTPChartColor clrAxis = m_pAxis->GetActualColor();

	BOOL bVertical = IsVertical();
	int nAngle = m_pAxis->GetLabel()->GetAngle();
	BOOL bNear = m_pAxis->GetAlignment() == xtpChartAxisNear;

	int nOffset = m_pAxis->GetThickness() + (m_pAxis->GetTickMarks()->IsVisible() ? m_pAxis->GetTickMarks()->GetLength() : 0)
		+ (IsScollBarVisible() ? SCROLLBARSIZE : 0);

	for (int i = 0; i < m_arrTicks.GetSize(); i++)
	{
		int nLeft = (int)ValueToPoint(m_arrTicks[i].m_dValue);
		CXTPChartString s = m_arrTicks[i].m_strLabel;

		if (bVertical)
		{
			CXTPChartRotatedTextPainterNearLine painter(pDC, s, m_pAxis->GetLabel(), CPoint(bNear ? m_rcBounds.right - nOffset : m_rcBounds.left + nOffset, nLeft), bNear ? xtpChartTextNearLeft : xtpChartTextNearRight, (float)nAngle);
			pLabelsCommand->AddChildCommand(painter.CreateDeviceCommand(pDC, m_pAxis->GetLabel()->GetActualTextColor()));

		}
		else
		{
			CXTPChartRotatedTextPainterNearLine painter(pDC, s, m_pAxis->GetLabel(), CPoint(nLeft, bNear ? m_rcBounds.top + nOffset : m_rcBounds.bottom - nOffset), bNear ? xtpChartTextNearBottom : xtpChartTextNearTop, (float)nAngle);
			pLabelsCommand->AddChildCommand(painter.CreateDeviceCommand(pDC, m_pAxis->GetLabel()->GetActualTextColor()));
		}
	}

	return pLabelsCommand;
}


void CXTPChartAxisView::OnLButtonDown(UINT /*nFlags*/, CPoint point)
{

	CXTPChartContainer* pContainer = m_pContainer;
	ASSERT (pContainer);

	if (m_rcScrollBar.GetLeft() > point.x || m_rcScrollBar.GetRight() < point.x)
		return;
	if (m_rcScrollBar.GetTop() > point.y || m_rcScrollBar.GetBottom() < point.y)
		return;

	m_pContainer->SetCapture(this);
	m_ptOldPosition = point;


	if (IsVertical())
	{
		double top = m_rcThumb.GetTop();
		double bottom = m_rcThumb.GetBottom();

		if (point.y < top || point.y > bottom)
		{
			double dy = !m_pAxis->IsReversed() ? (m_rcScrollBar.GetBottom() - point.y) : (point.y - m_rcScrollBar.GetTop());

			double pos = dy * (GetRangeMaxValue() - GetRangeMinValue())
				/ m_rcScrollBar.Height  + GetRangeMinValue();

			double delta = -(GetViewRangeMinValue() - (pos - (GetViewRangeMaxValue() - GetViewRangeMinValue()) / 2));


			if (GetViewRangeMaxValue() + delta > GetRangeMaxValue())
				delta = GetRangeMaxValue() - GetViewRangeMaxValue();

			if (GetViewRangeMinValue() + delta < GetRangeMinValue())
				delta = GetRangeMinValue() - GetViewRangeMinValue();

			m_pAxis->GetRange()->SetViewMaxValue(GetViewRangeMaxValue() + delta);
			m_pAxis->GetRange()->SetViewMinValue(GetViewRangeMinValue() + delta);
		}

	}
	else
	{
		double left = m_rcThumb.GetLeft();
		double right = m_rcThumb.GetRight();


		if (point.x < left || point.x > right)
		{
			double dx = !m_pAxis->IsReversed() ? (point.x - m_rcScrollBar.GetLeft()) : (m_rcScrollBar.GetRight() - point.x);

			double pos = dx * (GetRangeMaxValue() - GetRangeMinValue())
				/ m_rcScrollBar.Width  + GetRangeMinValue();

			double delta = -(GetViewRangeMinValue() - (pos - (GetViewRangeMaxValue() - GetViewRangeMinValue()) / 2));


			if (GetViewRangeMaxValue() + delta > GetRangeMaxValue())
				delta = GetRangeMaxValue() - GetViewRangeMaxValue();

			if (GetViewRangeMinValue() + delta < GetRangeMinValue())
				delta = GetRangeMinValue() - GetViewRangeMinValue();

			m_pAxis->GetRange()->SetViewMaxValue(GetViewRangeMaxValue() + delta);
			m_pAxis->GetRange()->SetViewMinValue(GetViewRangeMinValue() + delta);
		}
	}
}

void CXTPChartAxisView::OnMouseMove(UINT /*nFlags*/, CPoint point)
{
	int dx = point.x - m_ptOldPosition.x;
	int dy = point.y - m_ptOldPosition.y;
	m_ptOldPosition = point;


	CXTPChartContainer* pContainer = m_pContainer;
	ASSERT (pContainer);

	m_pAxis->GetRange()->SetViewAutoRange(FALSE);

	double dMinValue = GetRangeMinValue();
	double dMaxValue = GetRangeMaxValue();
	double dScale = 0;

	if (IsVertical())
	{
		dScale = (m_rcBounds.Height()) / (dMaxValue - dMinValue);
	}
	else
	{
		dScale = (m_rcBounds.Width()) / (dMaxValue - dMinValue);
	}

	double delta = 1.0 / dScale;
	if (IsVertical())
		delta *= -dy;
	else
		delta *= dx;


	if (m_pAxis->IsReversed())
		delta *= -1;

	if (GetViewRangeMaxValue() + delta > GetRangeMaxValue())
		delta = GetRangeMaxValue() - GetViewRangeMaxValue();

	if (GetViewRangeMinValue() + delta < GetRangeMinValue())
		delta = GetRangeMinValue() - GetViewRangeMinValue();

	m_pAxis->GetRange()->SetViewMaxValue(GetViewRangeMaxValue() + delta);
	m_pAxis->GetRange()->SetViewMinValue(GetViewRangeMinValue() + delta);
}


void CXTPChartAxisView::PerformPaneDragging(int dx, int dy)
{
	CXTPChartContainer* pContainer = m_pContainer;
	ASSERT (pContainer);

	m_pAxis->GetRange()->SetViewAutoRange(FALSE);

	double delta = 1.0 / GetScale();
	if (IsVertical())
		delta *= dy;
	else
		delta *= -dx;

	if (m_pAxis->IsReversed())
		delta *= -1;

	if (GetViewRangeMaxValue() + delta > GetRangeMaxValue())
		delta = GetRangeMaxValue() - GetViewRangeMaxValue();

	if (GetViewRangeMinValue() + delta < GetRangeMinValue())
		delta = GetRangeMinValue() - GetViewRangeMinValue();

	m_pAxis->GetRange()->SetViewMaxValue(GetViewRangeMaxValue() + delta);
	m_pAxis->GetRange()->SetViewMinValue(GetViewRangeMinValue() + delta);
}

void CXTPChartAxisView::PerformMouseWheel(short zDelta, CPoint /*pt*/)
{
	CXTPChartContainer* pContainer = m_pContainer;
	ASSERT (pContainer);

	if (!m_pAxis->IsAllowZoom())
		return;

	CXTPChartAxisRange* pRange = m_pAxis->GetRange();

	pRange->SetViewAutoRange(FALSE);

	double dRange = (GetViewRangeMaxValue() - GetViewRangeMinValue());
	double delta = dRange / 10;

	if (zDelta >= 0)
		delta = -delta;

	if (dRange - 2 * delta < pRange->GetZoomLimit())
	{
		delta = (dRange - pRange->GetZoomLimit()) / 2;
	}

	pRange->SetViewMaxValue(min(GetRangeMaxValue(), GetViewRangeMaxValue() - delta));
	pRange->SetViewMinValue(max(GetRangeMinValue(), GetViewRangeMinValue() + delta));

}

CXTPChartDeviceCommand* CXTPChartAxisView::CreateScrollBarDeviceCommand(CXTPChartDeviceContext* /*pDC*/)
{
	m_rcScrollBar = CXTPChartRectF(0, 0, 0, 0);

	if (!IsScollBarVisible())
		return NULL;

	CXTPChartDeviceCommand* pCommand = new CXTPChartDeviceCommand();

	CRect rcBounds = m_rcBounds;
	int nSize = SCROLLBARSIZE;

	if (IsVertical())
	{
		if (m_pAxis->GetAlignment() == xtpChartAxisNear)
		{
			rcBounds.left = rcBounds.right - nSize;
		}
		else
		{
			rcBounds.right = rcBounds.left + nSize;
		}
	}
	else
	{
		if (m_pAxis->GetAlignment() == xtpChartAxisNear)
		{
			rcBounds.bottom = rcBounds.top + nSize;
		}
		else
		{
			rcBounds.top = rcBounds.bottom - nSize;
		}
	}

	CXTPChartColor clrAxis = m_pAxis->GetActualColor();

	pCommand->AddChildCommand(new CXTPChartSolidRectangleDeviceCommand(rcBounds, CXTPChartColor::White));

	pCommand->AddChildCommand(new CXTPChartBoundedRectangleDeviceCommand(rcBounds, clrAxis, 1));

	CXTPChartRectF rcScrollBar(rcBounds);

	rcScrollBar.DeflateRect(1.5f, 1.5f, 1.5f, 1.5f);


	double minValue = (GetViewRangeMinValue() - GetRangeMinValue()) / (
		GetRangeMaxValue() - GetRangeMinValue());

	double maxValue = (GetViewRangeMaxValue() - GetRangeMinValue()) / (
		GetRangeMaxValue() - GetRangeMinValue());

	const int MIN_BUTTON_SIZE = 15;

	CXTPChartRectF rcThumb(rcScrollBar);
	if (IsVertical())
	{
		double top, bottom;
		if (!m_pAxis->IsReversed())
		{
			bottom = rcThumb.GetBottom() - rcThumb.Height * minValue;
			top = rcThumb.GetBottom() - rcThumb.Height * maxValue;
		}
		else
		{
			top = rcThumb.GetTop() + rcThumb.Height * minValue;
			bottom = rcThumb.GetTop() + rcThumb.Height * maxValue;

		}

		if (bottom - top < MIN_BUTTON_SIZE)
		{
			bottom = top + MIN_BUTTON_SIZE;
			if (bottom > rcScrollBar.GetBottom())
			{
				bottom = rcScrollBar.GetBottom();
				top = bottom - MIN_BUTTON_SIZE;
			}
			if (top < rcScrollBar.GetTop())
				top = rcScrollBar.GetTop();
		}

		rcThumb.Y = (float)top;
		rcThumb.Height = (float)(bottom - top);
	}
	else
	{
		double left, right;
		if (!m_pAxis->IsReversed())
		{
			left = rcThumb.GetLeft() + rcThumb.Width * minValue;
			right = rcThumb.GetLeft() + rcThumb.Width * maxValue;
		}
		else
		{
			right = rcThumb.GetRight() - rcThumb.Width * minValue;
			left = rcThumb.GetRight() - rcThumb.Width * maxValue;

		}

		if (right - left < MIN_BUTTON_SIZE)
		{
			right = left + MIN_BUTTON_SIZE;
			if (right > rcScrollBar.GetRight())
			{
				right = rcScrollBar.GetRight();
				left = right - MIN_BUTTON_SIZE;
			}
			if (left < rcScrollBar.GetLeft())
				left = rcScrollBar.GetLeft();
		}

		rcThumb.X = (float)left;
		rcThumb.Width = (float)(right - left);
	}

	m_rcScrollBar = rcScrollBar;
	m_rcThumb = rcThumb;

	pCommand->AddChildCommand(new CXTPChartSolidRectangleDeviceCommand(rcThumb, clrAxis));

	if (IsVertical())
	{
		if (rcThumb.Height > 10)
		{
			CXTPChartPointF pt = rcThumb.GetCenter();

			pCommand->AddChildCommand(new CXTPChartSolidLineDeviceCommand(
				CXTPChartDiagramPoint(pt.X - 1, pt.Y), CXTPChartDiagramPoint(pt.X + 1, pt.Y), CXTPChartColor::White, 1));
			pCommand->AddChildCommand(new CXTPChartSolidLineDeviceCommand(
				CXTPChartDiagramPoint(pt.X - 1, pt.Y - 2), CXTPChartDiagramPoint(pt.X + 1, pt.Y - 2), CXTPChartColor::White, 1));
			pCommand->AddChildCommand(new CXTPChartSolidLineDeviceCommand(
				CXTPChartDiagramPoint(pt.X - 1, pt.Y + 2), CXTPChartDiagramPoint(pt.X + 1, pt.Y + 2), CXTPChartColor::White, 1));

		}
	}
	else
	{
		if (rcThumb.Width > 10)
		{
			CXTPChartPointF pt = rcThumb.GetCenter();

			pCommand->AddChildCommand(new CXTPChartSolidLineDeviceCommand(
				CXTPChartDiagramPoint(pt.X, pt.Y - 1), CXTPChartDiagramPoint(pt.X, pt.Y + 1), CXTPChartColor::White, 1));
			pCommand->AddChildCommand(new CXTPChartSolidLineDeviceCommand(
				CXTPChartDiagramPoint(pt.X - 2, pt.Y - 1), CXTPChartDiagramPoint(pt.X - 2, pt.Y + 1), CXTPChartColor::White, 1));
			pCommand->AddChildCommand(new CXTPChartSolidLineDeviceCommand(
				CXTPChartDiagramPoint(pt.X + 2, pt.Y - 1), CXTPChartDiagramPoint(pt.X + 2, pt.Y + 1), CXTPChartColor::White, 1));

		}

	}


	return pCommand;
}

CXTPChartDeviceCommand* CXTPChartAxisView::CreateTitleDeviceCommand(CXTPChartDeviceContext* pDC)
{
	CXTPChartAxisTitle* pTitle = GetAxis()->GetTitle();
	if (!pTitle->IsVisible())
		return NULL;

	CXTPChartString strText = pTitle->GetText();

	CXTPChartDeviceCommand* pCommand = new CXTPChartHitTestElementCommand(pTitle, m_rcBounds);

	if (IsVertical())
	{
		if (m_pAxis->GetAlignment() == xtpChartAxisNear)
		{
			CXTPChartRotatedTextPainterNearLine painter(pDC, pTitle->GetText(), pTitle,
				CPoint(m_rcBounds.left, m_rcBounds.CenterPoint().y), xtpChartTextNearRight, 270);
			pCommand->AddChildCommand(painter.CreateDeviceCommand(pDC,  pTitle->GetActualTextColor()));
		}
		else
		{
			CXTPChartRotatedTextPainterNearLine painter(pDC, pTitle->GetText(), pTitle,
				CPoint(m_rcBounds.right, m_rcBounds.CenterPoint().y), xtpChartTextNearLeft, 90);
			pCommand->AddChildCommand(painter.CreateDeviceCommand(pDC, pTitle->GetActualTextColor()));

		}
	}
	else
	{
		if (m_pAxis->GetAlignment() == xtpChartAxisNear)
		{
			if (pTitle->GetAlignment() == xtpChartAlignCenter)
			{
				CXTPChartRotatedTextPainterNearLine painter(pDC, pTitle->GetText(), pTitle,
					CPoint(m_rcBounds.CenterPoint().x, m_rcBounds.bottom + 3), xtpChartTextNearTop, 0);
				pCommand->AddChildCommand(painter.CreateDeviceCommand(pDC, pTitle->GetActualTextColor()));
			}
			else
			{
				CXTPChartTextPainter painter(pDC, pTitle->GetText(), pTitle);
				painter.SetLocation(CXTPChartPointF(pTitle->GetAlignment() == xtpChartAlignNear ? m_rcBounds.left :
					m_rcBounds.right - painter.GetSize().Width, m_rcBounds.bottom + 3 - painter.GetSize().Height));

				pCommand->AddChildCommand(painter.CreateDeviceCommand(pDC, pTitle->GetActualTextColor()));

			}
		}
		else
		{
			CXTPChartRotatedTextPainterNearLine painter(pDC, pTitle->GetText(), pTitle,
				CPoint(m_rcBounds.CenterPoint().x, m_rcBounds.top), xtpChartTextNearBottom, 0);
			pCommand->AddChildCommand(painter.CreateDeviceCommand(pDC, pTitle->GetActualTextColor()));

		}
	}


	return pCommand;
}

BOOL CXTPChartAxisView::IsScollBarVisible() const
{
	if (m_pAxis->GetRange()->IsViewAutoRange())
		return FALSE;

	if (fabs(GetViewRangeMinValue() - GetRangeMinValue()) < 1e-6 &&
		fabs(GetViewRangeMaxValue() - GetRangeMaxValue()) < 1e-6)
		return FALSE;

	if (!m_pAxis->GetDiagram()->IsAllowScroll())
		return FALSE;

	return TRUE;
}

CXTPChartDeviceCommand* CXTPChartAxisView::CreateDeviceCommand(CXTPChartDeviceContext* pDC)
{
	CXTPChartDeviceCommand* pAxisCommand = new CXTPChartDeviceCommand();

	CXTPChartColor clrAxis = m_pAxis->GetActualColor();

	int nScrollBarSize = IsScollBarVisible() ? SCROLLBARSIZE : 0;
	int nThickness = m_pAxis->GetThickness();

	if (m_pAxis->IsVisible())
	{
		if (IsVertical())
		{
			float fPosition = m_pAxis->GetAlignment() == xtpChartAxisNear ? m_rcBounds.right - nScrollBarSize - nThickness / 2.0f + 0.5f : m_rcBounds.left + nScrollBarSize +  nThickness / 2.0f - 0.5f;

			pAxisCommand->AddChildCommand(new CXTPChartSolidLineDeviceCommand(CXTPChartDiagramPoint(fPosition, (float)m_rcBounds.top),
				CXTPChartDiagramPoint(fPosition, m_rcBounds.bottom + 0.5f), clrAxis, m_pAxis->GetThickness()));

		}
		else
		{
			float fPosition = m_pAxis->GetAlignment() == xtpChartAxisNear ? m_rcBounds.top + nScrollBarSize + nThickness / 2.0f - 0.5f : m_rcBounds.bottom - nThickness / 2.0f + 0.5f - nScrollBarSize;

			pAxisCommand->AddChildCommand(new CXTPChartSolidLineDeviceCommand(CXTPChartDiagramPoint(m_rcBounds.left, fPosition),
				CXTPChartDiagramPoint(m_rcBounds.right + 0.5f, fPosition), clrAxis, nThickness));
		}

		pAxisCommand->AddChildCommand(CreateTickMarksDeviceCommand(pDC));
		pAxisCommand->AddChildCommand(CreateLabelsDeviceCommand(pDC));
		pAxisCommand->AddChildCommand(CreateTitleDeviceCommand(pDC));

		pAxisCommand->AddChildCommand(CreateScrollBarDeviceCommand(pDC));
	}


	return pAxisCommand;
}



double CXTPChartAxisView::CalculateGridSpacing(double nAxisRangeDelta, double nScreenDelta, double nGridSpacingFactor)
{
	if (m_pAxis->GetScaleType() == xtpChartScaleQualitative)
		return 1.0;

	double multipliers[]  = { 1, 2, 3, 5 };

	if (nScreenDelta <= 0)
		return 1;

	double deltaCoef = nGridSpacingFactor * nAxisRangeDelta / nScreenDelta;
	if (deltaCoef > 1.0)
	{
		for (double factor = 1;; factor *= 10)
		{
			for (int i = 0; i < 4; i++) {
				double result = multipliers[i] * factor;
				if (deltaCoef <= result)
					return result;
			}
		}
	}
	else
	{
		double result = 1;
		for (double factor = 0.1;; factor /= 10)
		{
			for (int i = 4 - 1; i >= 0; i--) {
				double newResult = multipliers[i] * factor;
				if (deltaCoef > newResult)
					return result;
				result = newResult;
			}
		}
	}
}

void CXTPChartAxisView::SetBounds(CXTPChartDeviceContext* pDC, CRect rcBounds)
{
	UNREFERENCED_PARAMETER(pDC);
	m_rcBounds = rcBounds;
}

void CXTPChartAxisView::CreateTickMarks(CXTPChartDeviceContext* pDC, CRect rcDiagram)
{
	UNREFERENCED_PARAMETER(pDC);
	UNREFERENCED_PARAMETER(rcDiagram);

	m_arrTicks.RemoveAll();

	double dMinValue = GetViewRangeMinValue();
	double dMaxValue = GetViewRangeMaxValue();
	double dGridSpacing = GetGridSpacing();


	BOOL bVertical = IsVertical();
	BOOL bNear = m_pAxis->GetAlignment() == xtpChartAxisNear;
	int nAngle = m_pAxis->GetLabel()->GetAngle();

	if (m_pAxis->GetCustomLabels()->GetCount() > 0)
	{
		CXTPChartAxisCustomLabels* pCustomLabels = m_pAxis->GetCustomLabels();
		int nCount = pCustomLabels->GetCount();

		for (int i = 0; i < nCount; i++)
		{
			CXTPChartAxisCustomLabel* pLabel = pCustomLabels->GetAt(i);

			CXTPChartAxisViewTick tick;
			tick.m_dValue = !pLabel->GetAxisValue().IsEmpty() ? m_pAxis->GetScaleTypeMap()->ValueToInternal(pLabel->GetAxisValue()) :
				pLabel->GetAxisValueInternal();

			tick.m_strLabel = pLabel->GetText();

			CXTPChartRotatedTextPainterNearLine painter(pDC, tick.m_strLabel, m_pAxis->GetLabel(),
				CPoint(0, 0), bVertical ? (bNear ? xtpChartTextNearLeft : xtpChartTextNearRight) :
			(bNear ? xtpChartTextNearBottom : xtpChartTextNearTop), (float)nAngle);

			tick.m_szLabel = painter.GetSize();

			tick.m_szBounds = painter.GetRoundedBounds().Size();

			m_arrTicks.Add(tick);
		}

	}
	else
	{
		double dMark = ((int)(dMinValue / dGridSpacing)) * dGridSpacing;
		if (dMark < dMinValue)
			dMark += dGridSpacing;
		while (dMark < dMaxValue + 1e-6)
		{
			CXTPChartAxisViewTick tick;
			tick.m_dValue = dMark;

			tick.m_strLabel = m_pAxis->GetScaleTypeMap()->InternalToValue(m_pAxis->GetLabel()->GetFormat(), dMark);

			CXTPChartRotatedTextPainterNearLine painter(pDC, tick.m_strLabel, m_pAxis->GetLabel(),
				CPoint(0, 0), bVertical ? (bNear ? xtpChartTextNearLeft : xtpChartTextNearRight) :
			(bNear ? xtpChartTextNearBottom : xtpChartTextNearTop), (float)nAngle);

			tick.m_szLabel = painter.GetSize();

			tick.m_szBounds = painter.GetRoundedBounds().Size();

			m_arrTicks.Add(tick);

			dMark += dGridSpacing;
		}
	}
}

void CXTPChartAxisView::CalcSize(CXTPChartDeviceContext* pDC, CRect rcDiagram)
{
	CXTPChartAxis* pAxis = GetAxis();

	double dScreenLength = IsVertical() ? rcDiagram.Height() : rcDiagram.Width();

	if (dScreenLength < 1)
	{
		m_nSize = 0;
		return;
	}

	double dRangeDelta = GetViewRangeMaxValue() - GetViewRangeMinValue();

	m_dGridSpacing = pAxis->GetGridSpacingAuto() ? CalculateGridSpacing(dRangeDelta, dScreenLength, IsVertical() ? 30 : 50) : pAxis->GetGridSpacing();
	pAxis->m_dGridSpacing = m_dGridSpacing;

	CreateTickMarks(pDC, rcDiagram);

	if (!pAxis->IsVisible())
	{
		m_nSize = 0;
		return;
	}

	BOOL bVertical = IsVertical();


	m_nSize = pAxis->GetThickness();

	if (pAxis->GetTickMarks()->IsVisible())
	{
		m_nSize += pAxis->GetTickMarks()->GetLength();
	}

	if (IsScollBarVisible())
	{
		m_nSize += SCROLLBARSIZE;

	}

	if (pAxis->GetLabel()->IsVisible())
	{
		int nLabelSize = 0;

		for (int i = 0; i < m_arrTicks.GetSize(); i++)
		{
			CSize szBounds = m_arrTicks[i].m_szBounds;

			nLabelSize = max(nLabelSize, bVertical ? szBounds.cx : szBounds.cy);
		}

		m_nSize += nLabelSize;

	}

	if (pAxis->GetTitle()->IsVisible())
	{
		CXTPChartAxisTitle* pTitle = GetAxis()->GetTitle();

		CXTPChartString strText = pTitle->GetText();

		BOOL bNear = m_pAxis->GetAlignment() == xtpChartAxisNear;

		CXTPChartRotatedTextPainterNearLine painter(pDC, pTitle->GetText(), pTitle,
			CPoint(0, 0), IsVertical() ? (bNear ? xtpChartTextNearRight : xtpChartTextNearLeft) :
			(bNear ? xtpChartTextNearTop : xtpChartTextNearBottom), (float)(IsVertical() ? (bNear ? 270 : 90) : 0));

		CSize szTitle = painter.GetRoundedBounds().Size();

		m_nSize += bVertical ? szTitle.cx : szTitle.cy;

	}
}

BOOL CXTPChartAxisView::IsVertical() const
{
	return m_pAxis->IsVertical();
}

void CXTPChartAxisView::CreateView(CXTPChartDeviceContext* pDC)
{
	UNREFERENCED_PARAMETER(pDC);
}

void CXTPChartAxisView::AddLegendItems()
{

	CXTPChartElementView* pParentView = m_pParentView;
	while (pParentView->GetParentView() != NULL)
		pParentView = pParentView->GetParentView();

	CXTPChartContentView* pContentView = (CXTPChartContentView*)pParentView;

	if (pContentView->GetLegendView())
	{
		CXTPChartAxisConstantLines* pConstantLines = m_pAxis->GetConstantLines();

		int i;

		for (i = 0; i < pConstantLines->GetCount(); i++)
		{
			CXTPChartAxisConstantLine* pConstantLine = pConstantLines->GetAt(i);
			if (!pConstantLine->IsVisible() || !pConstantLine->IsLegendVisible())
				continue;

			pContentView->GetLegendView()->AddItem(pConstantLine);

		}

		CXTPChartAxisStrips* pStrips = m_pAxis->GetStrips();

		for (i = 0; i < pStrips->GetCount(); i++)
		{
			CXTPChartAxisStrip* pStrip = pStrips->GetAt(i);
			if (!pStrip->IsVisible() || !pStrip->IsLegendVisible())
				continue;

			pContentView->GetLegendView()->AddItem(pStrip);

		}
	}

}
