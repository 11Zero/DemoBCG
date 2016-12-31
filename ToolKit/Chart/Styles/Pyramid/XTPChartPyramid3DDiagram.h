// XTPChartPyramid3DDiagram.h
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
#if !defined(__XTPCHARTPYRAMID3DDIAGRAM_H__)
#define __XTPCHARTPYRAMID3DDIAGRAM_H__
//}}AFX_CODEJOCK_PRIVATE

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "../../Types/XTPChartTypes.h"
#include "../../XTPChartDiagram3D.h"

#include "../../XTPChartSeries.h"
#include "../../XTPChartSeriesPoint.h"
#include "../../Types/XTPChartDiagramPoint.h"


#include "XTPChartPyramidDiagram.h"
#include "XTPChartPyramidSeriesStyle.h"

class CXTPChartPyramid;
class CXTPChartPyramid3DDiagramDomain;
class CXTPChartDiagram3D;

//===========================================================================
// Summary:
//     This class abstracts a 3D pie diagram.This class is a kind of
//     CXTPChartDiagram3D.
// Remarks:
//===========================================================================
class _XTP_EXT_CLASS CXTPChartPyramid3DDiagram : public CXTPChartDiagram3D
{
	DECLARE_SERIAL(CXTPChartPyramid3DDiagram)

public:
	//-----------------------------------------------------------------------
	// Summary:
	//     Constructs a CXTPChartPyramid3DDiagram object.
	// Remarks:
	//-----------------------------------------------------------------------
	CXTPChartPyramid3DDiagram();

public:
	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to create the view of the 3D pie diagram.
	// Parameters:
	//     pDC      - A pointer to the chart device context object.
	//     pParent  - A pointer to the parent view.
	// Returns:
	//     A pointer to the CXTPChartPyramidDiagramView object, which is a kind
	//     of CXTPChartDiagramView.
	// Remarks:
	//-----------------------------------------------------------------------
	virtual CXTPChartDiagramView* CreateView(CXTPChartDeviceContext* pDC, CXTPChartElementView* pParent);
	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to calculate the layout of the 3D pie diagram.
	// Parameters:
	//     pDC      - A pointer to the chart device context object.
	//     pParent  - A pointer to the parent view.
	// Remarks:
	//-----------------------------------------------------------------------
	virtual void CalculateSeriesLayout(CXTPChartDeviceContext* pDC, CXTPChartDiagramView* pView);
};

//===========================================================================
// Summary:
//     This class represents the view of a 3D pie series ,which is a kind of
//     CXTPChartPyramidSeriesView.
// Remarks:
//===========================================================================
class _XTP_EXT_CLASS CXTPChartPyramid3DSeriesView : public CXTPChartPyramidSeriesView
{
public:
	//-----------------------------------------------------------------------
	// Summary:
	//     Constructs a CXTPChartPyramid3DSeriesView object.
	// Parameters:
	//     pSeries       - A pointer to the chart series object.
	//     pDiagramView  - A pointer to the chart diagram view.
	// Remarks:
	//-----------------------------------------------------------------------
	CXTPChartPyramid3DSeriesView(CXTPChartSeries* pSeries, CXTPChartDiagramView* pDiagramView);

	//-------------------------------------------------------------------------
	// Summary:
	//     Destroys a CXTPChartPyramid3DSeriesView object, handle cleanup.
	//-------------------------------------------------------------------------
	virtual ~CXTPChartPyramid3DSeriesView();

public:
	//-------------------------------------------------------------------------
	// Summary:
	//     Call this function to calculate the layout of the label.
	// Parameters:
	//     pDC      - A pointer to the chart device context object.
	//
	//-------------------------------------------------------------------------
	void CalculateLabelLayout(CXTPChartDeviceContext* pDC);

	void CalculatePointLayout(CXTPChartDeviceContext* pDC, CRect rcBounds);

	virtual CXTPChartRectF GetInnerBounds() const;

protected:
	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to create view of the 3d pie series point.
	// Parameters:
	//     pDC    - A pointer to chart device context.
	//     pPoint - A pointer to the chart series point object.
	// Returns:
	//     A pointer to CXTPChartSeriesPointView,which refers to a newly created
	//     CXTPChartPyramid3DSeriesPointView object.
	// Remarks:
	//-----------------------------------------------------------------------
	virtual CXTPChartSeriesPointView* CreateSeriesPointView(CXTPChartDeviceContext* pDC, CXTPChartSeriesPoint* pPoint, CXTPChartElementView* pParentView);

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to create the diagram domain object for the 3D pie
	//     diagram.
	// Parameters:
	//     rcBounds - A CRect object specifying the rectangular boundary of the
	//                diagram.
	// Returns:
	//     A pointer to CXTPChartDiagramDomain,which refers to a newly created
	//     CXTPChartPyramid3DDiagramDomain object.
	// Remarks:
	//-----------------------------------------------------------------------
	CXTPChartDiagramDomain* CreateDiagramDomain(CXTPChartDeviceContext* pDC, CRect rcBounds);

	//-------------------------------------------------------------------------
	// Summary:
	//     This function create a CXTPChartDeviceCommand object, this object
	//     represents the rendering of a 3D pie series.
	// Parameters:
	//     pDC     - Pointer to a CXTPChartDeviceContext object.
	// Returns:
	//     Returns CXTPChartDeviceCommand object, this object handles the
	//     rendering of the 3D pie series.
	// Remarks:
	// See Also:
	//-------------------------------------------------------------------------
	CXTPChartDeviceCommand* CreateDeviceCommand(CXTPChartDeviceContext* pDC);

	//{{AFX_CODEJOCK_PRIVATE
	void CalculateValues();
	//}}AFX_CODEJOCK_PRIVATE

public:
	CXTPChartDiagramDomain* m_pDomain;
};
//===========================================================================
// Summary:
//     This class represents the view of a 3D pie series point, which is a kind of
//     CXTPChartPyramidSeriesPointView.
// Remarks:
//===========================================================================
class _XTP_EXT_CLASS CXTPChartPyramid3DSeriesPointView : public CXTPChartPyramidSeriesPointView
{
public:
	//-----------------------------------------------------------------------
	// Summary:
	//     Constructs a CXTPChartPyramid3DSeriesPointView object.
	// Parameters:
	//     pPoint - A pointer to the chart series point object.
	// Remarks:
	//-----------------------------------------------------------------------
	CXTPChartPyramid3DSeriesPointView(CXTPChartSeriesPoint* pPoint, CXTPChartElementView* pParentView);

	//-------------------------------------------------------------------------
	// Summary:
	//     Destroys a CXTPChartPyramid3DSeriesPointView object, handle cleanup.
	//-------------------------------------------------------------------------
	virtual ~CXTPChartPyramid3DSeriesPointView();

public:

protected:
	//-------------------------------------------------------------------------
	// Summary:
	//     This function create a CXTPChartDeviceCommand object, this object
	//     represents the rendering of a 3D pie series point.
	// Parameters:
	//     pDC     - Pointer to a CXTPChartDeviceContext object.
	// Returns:
	//     Returns CXTPChartDeviceCommand object, this object handles the
	//     rendering of the 3D pie series point.
	// Remarks:
	// See Also:
	//-------------------------------------------------------------------------
	virtual CXTPChartDeviceCommand* CreateDeviceCommand(CXTPChartDeviceContext* pDC);

protected:

	friend class CXTPChartPyramid3DSeriesView;
};
//===========================================================================
// Summary:
//     This class abstracts a 3D pie diagram domain.This class is a kind of
//     CXTPChartDiagram3DDomain.
// Remarks:
//===========================================================================
class _XTP_EXT_CLASS CXTPChartPyramid3DDiagramDomain : public CXTPChartDiagram3DDomain
{
public:
	//-----------------------------------------------------------------------
	// Summary:
	//     Constructs a CXTPChartPyramid3DDiagramDomain object.
	// Parameters:
	//     pDiagram      - A pointer to chart diagram object.
	//     rcBounds      - The bounding rectangle of the diagram.
	//     rcLabelBounds - The bounding rectangle of the diagram label.
	//     rcInnerBounds - The inner bounds of the diagram.
	//     nDepthPercent - The depth percentage.
	// Remarks:
	//-----------------------------------------------------------------------
	CXTPChartPyramid3DDiagramDomain(CXTPChartDiagram3D* pDiagram, CRect& rcBounds, CRect& rcLabelBounds, CXTPChartRectF& rcInnerBounds, int nDepthPercent);

public:
	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to create a graphics device command.
	// Parameters:
	//     pInnerCommand - A pointer to inner device command.
	// Remarks:
	//-----------------------------------------------------------------------
	CXTPChartDeviceCommand* CreateGraphicsCommand(CXTPChartDeviceCommand*& pInnerCommand);

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to create an additional model view device command
	//     for 3D drawing.
	// Returns:
	//     A pointer to CXTPChartDeviceCommand object.
	// Remarks:
	//-----------------------------------------------------------------------
	CXTPChartDeviceCommand* CreateAdditionalModelViewCommand();

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to create a lighting device command for drawing
	//     the lighting effects in 3D drawing.
	// Parameters:
	//     innerLightingCommand - A reference to the pointer to chart device
	//                            command.Which is an out param, to pass the
	//                            CXTPChartLightingDeviceCommand object.
	// Returns:
	//     A pointer to CXTPChartDeviceCommand object.
	// Remarks:
	//-----------------------------------------------------------------------
	CXTPChartDeviceCommand* CreateLightingCommand(CXTPChartDeviceCommand*& innerLightingCommand);
};


class CXTPChartPyramid3DDiagramView : public CXTPChartPyramidDiagramView
{
public:
	CXTPChartPyramid3DDiagramView(CXTPChartDiagram* pDiagram, CXTPChartElementView* pParent);

public:
	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to do rotation.
	// parameters:
	//     dx - The angle to rotate about X coordinate.
	//     dy - The angle to rotate about Y coordinate.
	// Remarks:
	//-----------------------------------------------------------------------
	BOOL PerformRotation(int dx, int dy);

public:
	void OnLButtonDown(UINT nFlags, CPoint point);
	void OnMouseMove(UINT nFlags, CPoint point);

protected:
	CPoint m_ptOldPosition;
};


#endif //#if !defined(__XTPCHARTPYRAMID3DDIAGRAM_H__)
