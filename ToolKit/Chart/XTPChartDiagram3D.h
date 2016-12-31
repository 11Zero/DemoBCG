// XTPChartDiagram3D.h
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
#if !defined(__XTPCHART3DDIAGRAM_H__)
#define __XTPCHART3DDIAGRAM_H__
//}}AFX_CODEJOCK_PRIVATE

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "XTPChartDiagram.h"
#include "Types/XTPChartMatrix.h"

class CXTPChartDiagram3D;


class _XTP_EXT_CLASS CXTPChartDiagram3DDomain : public CXTPChartDiagramDomain
{
public:
	//-----------------------------------------------------------------------
	// Summary:
	//     Constructs a CXTPChartDiagram3DDomain object.
	// Parameters:
	//     pDiagram      - A pointer to chart diagram object.
	//     rcBounds      - The bounding rectangle of the diagram.
	//     rcLabelBounds - The bounding rectangle of the diagram label.
	//     rcInnerBounds - The inner bounds of the diagram.
	//     nDepthPercent - The depth percentage.
	// Remarks:
	//-----------------------------------------------------------------------
	CXTPChartDiagram3DDomain(CXTPChartDiagram3D* pDiagram, const CRect& rcBounds, const CRect& rcLabelBounds, const CXTPChartRectF& rcInnerBounds, int nDepthPercent);

public:
	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to get the inner bounds of the chart diagram.
	// Returns:
	//     A chart rect object specifying the inner bounds of the diagram.
	// Remarks:
	//-----------------------------------------------------------------------
	CXTPChartRectF GetInnerBounds() const;

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to get the bounds of the chart diagram.
	// Returns:
	//     A CRect object specifying the bounds of the chart.
	// Remarks:
	//-----------------------------------------------------------------------
	CRect GetBounds() const;

public:
	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to create a view port graphics device command.
	// Returns:
	//     A pointer to CXTPChartViewPortDeviceCommand object which is a kind
	//     of CXTPChartDeviceCommand.
	// Remarks:
	//-----------------------------------------------------------------------
	CXTPChartDeviceCommand* CreateViewPortGraphicsCommand();

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to calculate the bounds after zooming and scrolling.
	// Parameters:
	//     distance - The zooming depth.
	//     p1       - An out parameter gives the new point described by diagram
	//                point object.
	//     p2       - An out parameter gives the new point described by diagram
	//                point object.
	// Remarks:
	//-----------------------------------------------------------------------
	void CalcBoundsAccordingZoomingAndScrolling(double distance, CXTPChartDiagramPoint& p1, CXTPChartDiagramPoint& p2);

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to create a graphics device command.
	// Parameters:
	//     pInnerCommand - A pointer to inner device command.
	// Remarks:
	// This is a pure virtual function , which makes it mandatory to override
	// this function in the derived classes.
	//-----------------------------------------------------------------------
	virtual CXTPChartDeviceCommand* CreateGraphicsCommand(CXTPChartDeviceCommand*& pInnerCommand) = 0;

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to create an additional model view device command
	//     for 3D drawing.
	// Returns:
	//     A pointer to CXTPChartDeviceCommand object.
	// Remarks:
	// This is a pure virtual function , which makes it mandatory to override
	// this function in the derived classes.
	//-----------------------------------------------------------------------
	virtual CXTPChartDeviceCommand* CreateAdditionalModelViewCommand() = 0;

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to create a lighting device command for drawing
	//     the lighting effects in 3D drawing.
	// Parameters:
	//     innerLightingCommand - A reference to the pointer to chart device
	//                            command.Which is an out param.
	// Returns:
	//     A pointer to CXTPChartDeviceCommand object.
	// Remarks:
	// This is a pure virtual function , which makes it mandatory to override
	// this function in the derived classes.
	//-----------------------------------------------------------------------
	virtual CXTPChartDeviceCommand* CreateLightingCommand(CXTPChartDeviceCommand*& innerLightingCommand) = 0;

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to project a 3D point in 2D window.
	// Returns:
	//     A pointer to CXTPChartDeviceCommand object.
	// Remarks:
	//-----------------------------------------------------------------------
	CXTPChartDiagramPoint Project(const CXTPChartDiagramPoint& point);

protected:
	CXTPChartDiagram3D* m_pDiagram;      //A pointer to 3D diagram object.
	CRect m_rcBounds;                   //The rectangular diagram boundary.
	CRect m_rcLabelBounds;              //The rectangular label boundary.
	CXTPChartRectF m_rcInnerBounds;      //The rectangular inner boundary.
	double m_dDepthFactor;              //The depth factor.

public:
	double m_dWidth;                    //The width of the diagram.
	double m_dHeight;                   //The height of the diagram.
	double m_dDepth;                    //The depth of the diagram.

	double m_dViewRadius;               //The view radius.
	double m_dZoomFactor;               //The zoom factor.
	double m_dViewOffsetX;              //The view offset in X axis.
	double m_dViewOffsetY;              //The view offset in Y axis.
};

//===========================================================================
// Summary:
//     This class abstracts a 3D diagram.
// Remarks:
//===========================================================================
class _XTP_EXT_CLASS CXTPChartDiagram3D : public CXTPChartDiagram
{
	DECLARE_DYNAMIC(CXTPChartDiagram3D)

public:
	//-----------------------------------------------------------------------
	// Summary:
	//     Constructs a CXTPChartDiagram3D object.
	// Remarks:
	//-----------------------------------------------------------------------
	CXTPChartDiagram3D();

public:
	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to get the perspective angle.
	// Returns:
	//     An integer specifying the perspective angle.
	// Remarks:
	//-----------------------------------------------------------------------
	int GetPerspectiveAngle() const;

public:
	void SetAllowRotate(BOOL bAllowRotate);
	BOOL IsAllowRotate() const;


public:

	//-----------------------------------------------------------------------
	// Summary:
	//     Use this function to create a rotation device command, this device
	//     command will help to do rotation transformations.
	// Returns:
	//     A pointer to the CXTPChartTransformDeviceCommand object, which is a
	//     kind of CXTPChartDeviceCommand.
	// Remarks:
	//-----------------------------------------------------------------------
	CXTPChartDeviceCommand* CreateRotationGraphicsCommand();

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to create a perspective graphics device command, this device
	//     command will help to do perspective projection.
	// Returns:
	//     A pointer to the CXTPChartTranslateDeviceCommand object, which is a
	//     kind of CXTPChartDeviceCommand.
	// Remarks:
	//-----------------------------------------------------------------------
	CXTPChartDeviceCommand* CreatePerspectiveGraphicsCommand(CXTPChartDiagram3DDomain* pDomain);

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to create a projection graphics device command, this device
	//     command will help to do projections.
	// Returns:
	//     A pointer to the CXTPChartTranslateDeviceCommand object, which is a
	//     kind of CXTPChartDeviceCommand.
	// Remarks:
	//-----------------------------------------------------------------------
	CXTPChartDeviceCommand* CreateProjectionGraphicsCommand(CXTPChartDiagram3DDomain* pDomain);

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to create a model view device command, this device
	//     command will help to do the model view transformations.
	// Returns:
	//     A pointer to the CXTPChartDeviceCommand object.
	// Remarks:
	//-----------------------------------------------------------------------
	CXTPChartDeviceCommand* CreateModelViewGraphicsCommand(CXTPChartDiagram3DDomain* pDomain);

public:
	void DoPropExchange(CXTPPropExchange* pPX);

public:
	//-----------------------------------------------------------------------
	// Summary:
	//     Use this function to check whether the diagram is a 3D type.
	// Returns:
	//     TRUE if the diagram is 3D and FALSE if not.
	// Remarks:
	//-----------------------------------------------------------------------
	BOOL Is3DDiagram() const;

public:
	CXTPChartMatrix GetRotationMatrix() const;
	void SetRotationMatrix(const CXTPChartMatrix& matrix);

public:

	int m_nPerspectiveAngle;            //The perspective angle.
	BOOL m_bAllowRotate;

	CXTPChartMatrix m_rotationMatrix;    //The rotation matrix.
};

AFX_INLINE int CXTPChartDiagram3D::GetPerspectiveAngle() const {
	return m_nPerspectiveAngle;
}

AFX_INLINE  CXTPChartRectF CXTPChartDiagram3DDomain::GetInnerBounds() const {
	return m_rcInnerBounds;
}
AFX_INLINE CRect CXTPChartDiagram3DDomain::GetBounds() const {
	return m_rcBounds;
}
AFX_INLINE BOOL CXTPChartDiagram3D::Is3DDiagram() const {
	return TRUE;
}
AFX_INLINE void CXTPChartDiagram3D::SetAllowRotate(BOOL bAllowRotate) {
	m_bAllowRotate = bAllowRotate;
	OnChartChanged();
}
AFX_INLINE BOOL CXTPChartDiagram3D::IsAllowRotate() const {
	return m_bAllowRotate;

}
AFX_INLINE CXTPChartMatrix CXTPChartDiagram3D::GetRotationMatrix() const {
	return m_rotationMatrix;
}
AFX_INLINE  void CXTPChartDiagram3D::SetRotationMatrix(const CXTPChartMatrix& matrix) {
	m_rotationMatrix = matrix;
	OnChartChanged(xtpChartUpdateLayout);
}


#endif //#if !defined(__XTPCHART3DDIAGRAM_H__)
