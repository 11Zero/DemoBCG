// XTPChartDiagram2DView.h
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

//{{AFX_CODEJOCK_PRIVATE
#if !defined(__XTPCHARTDIAGRAM2DVIEW_H__)
#define __XTPCHARTDIAGRAM2DVIEW_H__
//}}AFX_CODEJOCK_PRIVATE

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CXTPChartAxis;
class CXTPChartDiagramPane;
class CXTPChartDiagramPaneView;
class CXTPChartDiagram2DAppearance;
class CXTPChartAxisView;

#include "../XTPChartDiagram.h"
#include "../XTPChartSeriesView.h"

//===========================================================================
// Summary:
//     This class represents the view of a chart 2D diagram, which is a kind of
//     CXTPChartDiagramView.
// Remarks:
//===========================================================================
class _XTP_EXT_CLASS CXTPChartDiagram2DView :public CXTPChartDiagramView
{
public:
	//-----------------------------------------------------------------------
	// Summary:
	//     Constructs a CXTPChartDiagram2DView object.
	// Parameters:
	//     pDiagram     - A pointer to the chart diagram object.
	//     pParent      - A pointer to the parent view object.
	// Remarks:
	//-----------------------------------------------------------------------
	CXTPChartDiagram2DView(CXTPChartDiagram* pDiagram, CXTPChartElementView* pParent);

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to get the diagram pane view object to which this
	//     CXTPChartDiagram2DView object is associated with.
	// Parameters:
	//     pDiagram     - A pointer to the chart diagram object.
	//     pParent      - A pointer to the parent view object.
	// Remarks:
	//-----------------------------------------------------------------------
	CXTPChartDiagramPaneView* GetPaneView() const;

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to create the view of the 2D diagram.
	// Parameters:
	//     pDC      - A pointer to the chart device context.
	// Remarks:
	//-----------------------------------------------------------------------
	void CreateView(CXTPChartDeviceContext* pDC);

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to calculate the layout based on a rectangular
	//     boundary.
	// Parameters:
	//     pDC      - A pointer to the chart device context.
	//     rcBounds - The diagram boundary.
	// Remarks:
	//-----------------------------------------------------------------------
	void CalculateView(CXTPChartDeviceContext* pDC, CRect rcBounds);

	void UpdateRange(CXTPChartDeviceContext* pDC);

	//-----------------------------------------------------------------------
	// Summary:
	//     Use this function to add an axis view to the diagram view.
	//     boundary.
	// Parameters:
	//     pDC          - A pointer to the chart device context.
	//     pParentView  - The parent view of the axis view.
	//     pAxis        - A pointer to the axis object, whose view is to be
	//                    added.
	// Remarks:
	//-----------------------------------------------------------------------
	void AddAxisView(CXTPChartDeviceContext* pDC, CXTPChartElementView* pParentView, CXTPChartAxis* pAxis);

	CXTPChartDeviceCommand* CreateDeviceCommand(CXTPChartDeviceContext* pDC);


	virtual void OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	virtual void OnLButtonDown(UINT nFlags, CPoint point);
	virtual BOOL OnSetCursor(CPoint point);

public:
	void CheckLabelBounds(const CXTPChartRectF& rcBounds);

public:

	CXTPChartElementView* m_pAxisViews;      //The axis view.
	CXTPChartDiagramPaneView* m_pPaneView;   //The pane view.

	HCURSOR m_hcurNormalHand;
	HCURSOR m_hcurDragHand;

	CRect m_rcLabelPadding;
};


class _XTP_EXT_CLASS CXTPChartDiagram2DSeriesView :public CXTPChartSeriesView
{
protected:
	CXTPChartDiagram2DSeriesView(CXTPChartSeries* pSeries, CXTPChartDiagramView* pDiagramView, BOOL bSortPoints = FALSE);

public:
	CXTPChartAxisView* GetAxisViewX() const;
	CXTPChartAxisView* GetAxisViewY() const;

	CXTPChartPointF GetScreenPoint(double x, double y) const;

	virtual void AfterUpdateRange(CXTPChartDeviceContext* pDC);
	virtual void BeforeUpdateRange(CXTPChartDeviceContext* pDC);

protected:
	static int _cdecl ComparePoints(const CXTPChartSeriesPointView** ppPoint1, const CXTPChartSeriesPointView** ppPoint2);

protected:
	CXTPChartAxisView* m_pAxisViewX;
	CXTPChartAxisView* m_pAxisViewY;

	BOOL m_bSortPoints;

	friend class CXTPChartDiagram2DView;
	friend class CXTPChartDiagramPaneView;
};

AFX_INLINE CXTPChartDiagramPaneView* CXTPChartDiagram2DView::GetPaneView() const {
	return m_pPaneView;
}

AFX_INLINE CXTPChartAxisView* CXTPChartDiagram2DSeriesView::GetAxisViewX() const {
	return m_pAxisViewX;
}

AFX_INLINE CXTPChartAxisView* CXTPChartDiagram2DSeriesView::GetAxisViewY() const {
	return m_pAxisViewY;
}


#endif //#if !defined(__XTPCHARTDIAGRAM2DVIEW_H__)
