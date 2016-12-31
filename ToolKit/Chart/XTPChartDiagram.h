// XTPChartDiagram.h
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
#if !defined(__XTPCHARTDIAGRAM_H__)
#define __XTPCHARTDIAGRAM_H__
//}}AFX_CODEJOCK_PRIVATE

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "XTPChartPanel.h"
#include "XTPChartElementView.h"
#include "Types/XTPChartTypes.h"

class CXTPChartSeriesView;
class CXTPChartElementView;
class CXTPChartDiagramView;
class CXTPChartSeries;
class CXTPChartPanel;

typedef CArray<CXTPChartSeries*, CXTPChartSeries*> CXTPChartSeriesArray;



//===========================================================================
// Summary:
//     This class represents a chart diagram, which is a kind of CXTPChartElement.
//     This class act as a base class for 2D and 3D diagrams.
// Remarks:
//===========================================================================
class _XTP_EXT_CLASS CXTPChartDiagram : public CXTPChartPanel
{
	DECLARE_DYNAMIC(CXTPChartDiagram)

protected:
	//-----------------------------------------------------------------------
	// Summary:
	//     Constructs a CXTPChartDiagram object.
	// Remarks:
	//-----------------------------------------------------------------------
	CXTPChartDiagram();

public:
	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to create view of the diagram.
	// Parameters:
	//     pDC      - The chart device context.
	//     pParent  - A pointer to the parent view object.
	// Returns:
	//     A pointer to CXTPChartDiagramView object.
	// Remarks:
	//     This is a virtual function, so the sub classes can give their type
	//     specific implementation for this function.
	//-----------------------------------------------------------------------
	virtual CXTPChartDiagramView* CreateView(CXTPChartDeviceContext* pDC, CXTPChartElementView* pParent);

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to calculate series layout of the diagram.
	// Parameters:
	//     pDC      - The chart device context.
	//     pView  - A pointer to the diagram view object.
	// Remarks:
	//     This is a virtual function, so the sub classes can give their type
	//     specific implementation for this function.
	//-----------------------------------------------------------------------
	virtual void CalculateSeriesLayout(CXTPChartDeviceContext* pDC, CXTPChartDiagramView* pView);

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to check whether the diagram is a 3D style.
	// Remarks:
	//     This is a virtual function, so the sub classes can give their type
	//     specific implementation for this function.
	//-----------------------------------------------------------------------
	virtual BOOL Is3DDiagram() const;

public:

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to get the series collection object.
	// Returns:
	//     A reference to CXTPChartSeriesArray object.
	// Remarks:
	//-----------------------------------------------------------------------
	const CXTPChartSeriesArray& GetSeries() const;

protected:
	//-----------------------------------------------------------------------
	// Summary:
	//     This function called when a series is added to the series collection
	//     in the chart content object which has a collection of series.
	// Parameters:
	//     pSeries - A pointer to chart series object.
	// Remarks:
	//-----------------------------------------------------------------------
	virtual BOOL OnSeriesAdded(CXTPChartSeries* pSeries);

	//-----------------------------------------------------------------------
	// Summary:
	//     This function called when a series is removed from the series collection
	//     in the chart content object which has a collection of series.
	// Parameters:
	//     pSeries - A pointer to chart series object.
	// Remarks:
	//-----------------------------------------------------------------------
	void OnSeriesRemoved(CXTPChartSeries* pSeries);

protected:
	//-------------------------------------------------------------------------
	// Summary:
	//     Call this function to decrease the usage count of the object.
	//-------------------------------------------------------------------------
	void Release();

public:
	//-----------------------------------------------------------------------
	// Summary:
	//     Reads or writes this object from or to an archive.
	// Parameters:
	//     pPX - A CXTPPropExchange object to serialize to or from.
	//----------------------------------------------------------------------
	virtual void DoPropExchange(CXTPPropExchange* pPX);

public:

	friend class CXTPChartContent;
	friend class CXTPChartSeries;



protected:
	CXTPChartSeriesArray m_arrSeries;        //The series collection.
};

class _XTP_EXT_CLASS CXTPChartDiagramDomain
{
public:
	virtual CXTPChartRectF GetInnerBounds() const = 0;
};
//===========================================================================
// Summary:
//     This class represents the view of a chart diagram, which is a kind of
//     CXTPChartElementView.
// Remarks:
//===========================================================================
class _XTP_EXT_CLASS CXTPChartDiagramView : public CXTPChartElementView
{
public:
	//-----------------------------------------------------------------------
	// Summary:
	//     Constructs a CXTPChartDiagram object.
	// Parameters:
	//     pDiagram    - A pointer to chart diagram object.
	//     pParentView - A pointer to the parent view.
	// Remarks:
	//-----------------------------------------------------------------------
	CXTPChartDiagramView(CXTPChartDiagram* pDiagram, CXTPChartElementView* pParentView);

public:

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to get the bounds of the diagram.
	// Returns:
	//     A CRect object contains the bounds of the diagram.
	// Remarks:
	//-----------------------------------------------------------------------
	CRect GetBounds() const;

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to get the diagram object associated with the
	//     view.
	// Returns:
	//     A CXTPChartDiagram pointer, the actual type could be CXTPChartDiagram2D
	//     or CXTPChartDiagram3D.
	// Remarks:
	//-----------------------------------------------------------------------
	CXTPChartDiagram* GetDiagram() const;

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to get the label view object associated with
	//     this diagram view.
	// Returns:
	//     A CXTPChartElementView pointer, representing the label view.
	// Remarks:
	//-----------------------------------------------------------------------
	CXTPChartElementView* GetLabelsView() const;

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to get the series view object associated with
	//     this diagram view.
	// Returns:
	//     A CXTPChartElementView pointer, representing the series view.
	// Remarks:
	//-----------------------------------------------------------------------
	CXTPChartElementView* GetSeriesView() const;

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to calculate the view of the diagram.
	// Parameters:
	//     pDC      - Pointer to the chart device context object.
	//     rcBounds - The bounding rectangle.
	// Remarks:
	//     This is a virtual function, so the sub classes can give their type
	//     specific implementation for this function.
	//-----------------------------------------------------------------------
	virtual void CalculateView(CXTPChartDeviceContext* pDC, CRect rcBounds);

	virtual void UpdateRange(CXTPChartDeviceContext* pDC);

	virtual void OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);

	virtual void CreateView(CXTPChartDeviceContext* pDC);


protected:
	virtual CXTPChartDeviceCommand* CreateDeviceCommand(CXTPChartDeviceContext* pDC);

protected:
	CXTPChartDiagram* m_pDiagram;        //Pointer to the associated diagram object.
	CRect m_rcBounds;                   //The diagram bounds.
	CXTPChartElementView* m_pLabelsView; //The label view.
	CXTPChartElementView* m_pSeriesView; //The series view.
	CXTPChartElementView* m_pTitlesView; // Titles View.
};


AFX_INLINE CRect CXTPChartDiagramView::GetBounds() const {
	return m_rcBounds;
}
AFX_INLINE BOOL CXTPChartDiagram::Is3DDiagram() const {
	return FALSE;
}
AFX_INLINE CXTPChartDiagram* CXTPChartDiagramView::GetDiagram() const {
	return m_pDiagram;
}
AFX_INLINE const CXTPChartSeriesArray& CXTPChartDiagram::GetSeries() const {
	return m_arrSeries;
}
AFX_INLINE CXTPChartElementView* CXTPChartDiagramView::GetLabelsView() const {
	return m_pLabelsView;
}
AFX_INLINE CXTPChartElementView* CXTPChartDiagramView::GetSeriesView() const {
	return m_pSeriesView;
}


#endif //#if !defined(__XTPCHARTDIAGRAM_H__)
