// XTPChartRectangleDeviceCommand.h
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
#if !defined(__XTPCHARTRECTANGLEDEVICECOMMAND_H__)
#define __XTPCHARTRECTANGLEDEVICECOMMAND_H__
//}}AFX_CODEJOCK_PRIVATE

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "XTPChartDeviceCommand.h"
#include "../XTPChartDefines.h"

//===========================================================================
// Summary:
//     Enumerates the linear gradient mode.
// Remarks:
//===========================================================================
enum XTPChartLinearGradientMode
{
	xtpChartLinearGradientModeHorizontal,        //The gradient mode horizontal
	xtpChartLinearGradientModeVertical,          //The gradient mode vertical
	xtpChartLinearGradientModeForwardDiagonal,   //The gradient mode forward dialoganl
	xtpChartLinearGradientModeBackwardDiagonal,  //The gradient mode backward dialoganl
	xtpChartLinearGradientModeCenterHorizontal,  //The gradient mode horizontal
	xtpChartLinearGradientModeCenterVertical,    //The gradient mode vertical
};

namespace Gdiplus
{
	class GpPath;
};

//===========================================================================
// Summary:
//     This class represents a bounded rectangle device command,which is a kind of
//     CXTPChartDeviceCommand.It specifically handles the rendering of bounded rectangular
//     shapes in a chart.
// Remarks:
//===========================================================================
class _XTP_EXT_CLASS CXTPChartBoundedRectangleDeviceCommand : public CXTPChartDeviceCommand
{
public:
	//-----------------------------------------------------------------------
	// Summary:
	//     Constructs a CXTPChartBoundedRectangleDeviceCommand object.
	// Parameters:
	//     rect      - The rectangular area to be rendered by the device command.
	//     color     - The color of the rectangle border.
	//     thickness - The thickness of the rectangular border.
	// Remarks:
	//-----------------------------------------------------------------------
	CXTPChartBoundedRectangleDeviceCommand(const CXTPChartRectF& rect, const CXTPChartColor& color, int thickness);

protected:
	//-----------------------------------------------------------------------
	// Summary:
	//     This is a virtual function override of base class CXTPChartDeviceContext,
	//     act polymorphically to do the actual drawing of the chart element, to which
	//     this device command is associated with.
	// Parameters:
	//     pDC      - The device context of the chart.
	// Remarks:
	//-----------------------------------------------------------------------
	void ExecuteOverride(CXTPChartDeviceContext* pDC);

protected:
	CXTPChartColor m_color;      //The color of the rectangle border.
	int m_thickness;            //The thickness of the rectangle border.
	CXTPChartRectF m_rect;       //The bounds of the rectangle.
};

class _XTP_EXT_CLASS CXTPChartInnerBorderDeviceCommand : public CXTPChartBoundedRectangleDeviceCommand
{
public:
	//-----------------------------------------------------------------------
	// Summary:
	//     Constructs a CXTPChartInsideBorderDeviceCommand object.
	// Parameters:
	//     rect      - The rectangular area to be rendered by the device command.
	//     color     - The color of the rectangle border.
	//     thickness - The thickness of the rectangular border.
	// Remarks:
	//-----------------------------------------------------------------------
	CXTPChartInnerBorderDeviceCommand(const CXTPChartRectF& rect, const CXTPChartColor& color, int thickness);
};


//===========================================================================
// Summary:
//     This class represents a solid rectangle device command,which is a kind of CXTPChartDeviceCommand.
//     It specifically handles the rendering of solid rectangular shapes in a chart like label.
// Remarks:
//===========================================================================
class _XTP_EXT_CLASS CXTPChartSolidRectangleDeviceCommand : public CXTPChartDeviceCommand
{
public:
	//-----------------------------------------------------------------------
	// Summary:
	//     Constructs a CXTPChartSolidRectangleDeviceCommand object.
	// Parameters:
	//     rect      - The rectangular area to be rendered by the device command.
	//     color     - The color of the rectangle.
	// Remarks:
	//-----------------------------------------------------------------------
	CXTPChartSolidRectangleDeviceCommand(const CXTPChartRectF& rect, const CXTPChartColor& color);

protected:
	//-----------------------------------------------------------------------
	// Summary:
	//     This is a virtual function override of base class CXTPChartDeviceContext,
	//     act polymorphically to do the actual drawing of the chart element, to which
	//     this device command is associated with.
	// Parameters:
	//     pDC      - The device context of the chart.
	// Remarks:
	//-----------------------------------------------------------------------
	void ExecuteOverride(CXTPChartDeviceContext* pDC);

	CXTPChartElement* HitTest(CPoint point, CXTPChartElement* pParent) const;

protected:
	CXTPChartColor m_color;      //The color of the rectangle.
	CXTPChartRectF m_rect;       //The bounds of the rectangle.
};

#ifdef _XTP_DEMOMODE

class _XTP_EXT_CLASS CXTPChartWatermarkBackgroundDeviceCommand : public CXTPChartDeviceCommand
{
public:
	CXTPChartWatermarkBackgroundDeviceCommand(const CXTPChartRectF& rect);

public:
	void ExecuteOverride(CXTPChartDeviceContext* pDC);

protected:
	CXTPChartRectF m_rect;       //The bounds of the rectangle.
};

#endif

class _XTP_EXT_CLASS CXTPChartContentBackgroundDeviceCommand : public CXTPChartSolidRectangleDeviceCommand
{
public:
	//-----------------------------------------------------------------------
	// Summary:
	//     Constructs a CXTPChartContentBackgroundDeviceCommand object.
	// Parameters:
	//     rect      - The rectangular area to be rendered by the device command.
	//     color     - The color of the rectangle.
	// Remarks:
	//-----------------------------------------------------------------------
	CXTPChartContentBackgroundDeviceCommand(const CXTPChartRectF& rect, const CXTPChartColor& color);

protected:
	void ExecuteOverride(CXTPChartOpenGLDeviceContext* pDC);
	void ExecuteOverride(CXTPChartDeviceContext* pDC);
};

//===========================================================================
// Summary:
//     This class represents a gradient rectangle device command,which is a kind of CXTPChartDeviceCommand.
//     It specifically handles the rendering of gradient rectangular shapes in a chart.
// Remarks:
//===========================================================================
class _XTP_EXT_CLASS CXTPChartGradientRectangleDeviceCommand : public CXTPChartDeviceCommand
{
public:

	//-----------------------------------------------------------------------
	// Summary:
	//     Constructs a CXTPChartGradientRectangleDeviceCommand object.
	// Parameters:
	//     rect      - The rectangular area to be rendered by the device command.
	//     color     - The start color of the gradient.
	//     color2    - The end color of the gradient.
	//     nMode     - The linear gradient mode, it can be horizontal, vertical,
	//                 forward diagonal or backward diagonal.
	// Remarks:
	//-----------------------------------------------------------------------
	CXTPChartGradientRectangleDeviceCommand(const CXTPChartRectF& rect, const CXTPChartColor& color, const CXTPChartColor& color2, XTPChartLinearGradientMode nMode);

protected:

	//-----------------------------------------------------------------------
	// Summary:
	//     This is a virtual function override of base class CXTPChartDeviceContext,
	//     act polymorphically to do the actual drawing of the chart element, to which
	//     this device command is associated with.
	// Parameters:
	//     pDC      - The device context of the chart.
	// Remarks:
	//-----------------------------------------------------------------------
	void ExecuteOverride(CXTPChartDeviceContext* pDC);

	CXTPChartElement* HitTest(CPoint point, CXTPChartElement* pParent) const;

protected:
	CXTPChartColor m_color;              //The start color of the gradient.
	CXTPChartColor m_color2;             //The end color of the gradient.
	CXTPChartRectF m_rect;               //The bounding rect of the shape.
	XTPChartLinearGradientMode m_nMode;  //The gradient mode.
};

//===========================================================================
// Summary:
//     This class represents a hatch rectangle device command,which is a kind of
//     CXTPChartDeviceCommand.It specifically handles the rendering of hatch
//     rectangular shapes in a chart.
// Remarks:
//===========================================================================
class _XTP_EXT_CLASS CXTPChartHatchRectangleDeviceCommand : public CXTPChartDeviceCommand
{
public:
	//-----------------------------------------------------------------------
	// Summary:
	//     Constructs a CXTPChartHatchRectangleDeviceCommand object.
	// Parameters:
	//     rect      - The rectangular area to be rendered by the device command.
	//     color     - The first color of the pattern.
	//     color2    - The second color of the pattern.
	//     nStyle    - chart hatch style.
	// Remarks:
	//-----------------------------------------------------------------------
	CXTPChartHatchRectangleDeviceCommand(const CXTPChartRectF& rect, const CXTPChartColor& color, const CXTPChartColor& color2, XTPChartHatchStyle nStyle);

protected:

	//-----------------------------------------------------------------------
	// Summary:
	//     This is a virtual function override of base class CXTPChartDeviceContext,
	//     act polymorphically to do the actual drawing of the chart element, to which
	//     this device command is associated with.
	// Parameters:
	//     pDC      - The device context of the chart.
	// Remarks:
	//-----------------------------------------------------------------------
	void ExecuteOverride(CXTPChartDeviceContext* pDC);

	CXTPChartElement* HitTest(CPoint point, CXTPChartElement* pParent) const;

protected:
	CXTPChartColor m_color;      //The first color.
	CXTPChartColor m_color2;     //The second color.
	CXTPChartRectF m_rect;       //The bounds of the shape.
	XTPChartHatchStyle m_nStyle; //The hatch style selected.
};

class _XTP_EXT_CLASS CXTPChartCircleDeviceCommand : public CXTPChartDeviceCommand
{
protected:
	CXTPChartCircleDeviceCommand(const CXTPChartPointF& center, double radius);

public:
	CXTPChartElement* HitTest(CPoint point, CXTPChartElement* pParent) const;

protected:
	CXTPChartPointF m_center;            //The center of the circle.
	double m_radius;                    //The radius of the circle.
};

//===========================================================================
// Summary:
//     This class represents a gradient circle device command,which is a kind of CXTPChartDeviceCommand.
//     It specifically handles the rendering of gradient circular shapes in a chart.
// Remarks:
//===========================================================================
class _XTP_EXT_CLASS CXTPChartSolidCircleDeviceCommand : public CXTPChartCircleDeviceCommand
{
public:
	//-----------------------------------------------------------------------
	// Summary:
	//     Constructs a CXTPChartSolidCircleDeviceCommand object.
	// Parameters:
	//     center    - The center point of the circle.
	//     radius    - The radius of the circle.
	//     color     - The color of the circle.
	// Remarks:
	//-----------------------------------------------------------------------
	CXTPChartSolidCircleDeviceCommand(const CXTPChartPointF& center, double radius, const CXTPChartColor& color);

protected:
	//-----------------------------------------------------------------------
	// Summary:
	//     This is a virtual function override of base class CXTPChartDeviceContext,
	//     act polymorphically to do the actual drawing of the chart element, to which
	//     this device command is associated with.
	// Parameters:
	//     pDC      - The device context of the chart.
	// Remarks:
	//-----------------------------------------------------------------------
	void ExecuteOverride(CXTPChartDeviceContext* pDC);

protected:
	CXTPChartColor m_color;              //The start color.
};

//===========================================================================
// Summary:
//     This class represents a gradient circle device command,which is a kind of CXTPChartDeviceCommand.
//     It specifically handles the rendering of gradient circular shapes in a chart.
// Remarks:
//===========================================================================
class _XTP_EXT_CLASS CXTPChartGradientCircleDeviceCommand : public CXTPChartCircleDeviceCommand
{
public:
	//-----------------------------------------------------------------------
	// Summary:
	//     Constructs a CXTPChartGradientCircleDeviceCommand object.
	// Parameters:
	//     center    - The center point of the circle.
	//     radius    - The radius of the circle.
	//     color     - The start color of the gradient.
	//     color2    - The end color of the gradient.
	//     nMode     - The linear gradient mode, it can be horizontal, vertical,
	//                 forward diagonal or backward diagonal.
	// Remarks:
	//-----------------------------------------------------------------------
	CXTPChartGradientCircleDeviceCommand(const CXTPChartPointF& center, double radius, const CXTPChartColor& color, const CXTPChartColor& color2, XTPChartLinearGradientMode nMode);

protected:
	//-----------------------------------------------------------------------
	// Summary:
	//     This is a virtual function override of base class CXTPChartDeviceContext,
	//     act polymorphically to do the actual drawing of the chart element, to which
	//     this device command is associated with.
	// Parameters:
	//     pDC      - The device context of the chart.
	// Remarks:
	//-----------------------------------------------------------------------
	void ExecuteOverride(CXTPChartDeviceContext* pDC);

protected:
	CXTPChartColor m_color;              //The start color.
	CXTPChartColor m_color2;             //The end color.
	XTPChartLinearGradientMode m_nMode;  //The gradient mode used.
};


//===========================================================================
// Summary:
//     This class represents a gradient hatch device command,which is a kind of CXTPChartCircleDeviceCommand.
//     It specifically handles the rendering of hatch circular shapes in a chart.
// Remarks:
//===========================================================================
class _XTP_EXT_CLASS CXTPChartHatchCircleDeviceCommand : public CXTPChartCircleDeviceCommand
{
public:
	//-----------------------------------------------------------------------
	// Summary:
	//     Constructs a CXTPChartHatchCircleDeviceCommand object.
	// Parameters:
	//     center    - The center point of the circle.
	//     radius    - The radius of the circle.
	//     color     - The start color of the gradient.
	//     color2    - The end color of the gradient.
	//     nStyle    - Chart hatch style.
	// Remarks:
	//-----------------------------------------------------------------------
	CXTPChartHatchCircleDeviceCommand(const CXTPChartPointF& center, double radius, const CXTPChartColor& color, const CXTPChartColor& color2, XTPChartHatchStyle nStyle);

protected:
	//-----------------------------------------------------------------------
	// Summary:
	//     This is a virtual function override of base class CXTPChartDeviceContext,
	//     act polymorphically to do the actual drawing of the chart element, to which
	//     this device command is associated with.
	// Parameters:
	//     pDC      - The device context of the chart.
	// Remarks:
	//-----------------------------------------------------------------------
	void ExecuteOverride(CXTPChartDeviceContext* pDC);

protected:
	CXTPChartColor m_color;              //The start color.
	CXTPChartColor m_color2;             //The end color.
	XTPChartHatchStyle m_nStyle;         //The hatch mode used.
};


//===========================================================================
// Summary:
//     This class represents a bounded circle device command,which is a kind of CXTPChartDeviceCommand.
//     It specifically handles the rendering of circular shapes in a chart.
// Remarks:
//===========================================================================
class _XTP_EXT_CLASS CXTPChartBoundedCircleDeviceCommand : public CXTPChartDeviceCommand
{
public:
	//-----------------------------------------------------------------------
	// Summary:
	//     Constructs a CXTPChartBoundedCircleDeviceCommand object.
	// Parameters:
	//     center    - The center point of the circle.
	//     radius    - The radius of the circle.
	//     color     - The color of boundary.
	//     thickness - The thickness of the boundary.
	// Remarks:
	//-----------------------------------------------------------------------
	CXTPChartBoundedCircleDeviceCommand(const CXTPChartPointF& center, double radius, const CXTPChartColor& color, int thickness);

protected:
	//-----------------------------------------------------------------------
	// Summary:
	//     This is a virtual function override of base class CXTPChartDeviceContext,
	//     act polymorphically to do the actual drawing of the chart element, to which
	//     this device command is associated with.
	// Parameters:
	//     pDC      - The device context of the chart.
	// Remarks:
	//-----------------------------------------------------------------------
	void ExecuteOverride(CXTPChartDeviceContext* pDC);

protected:
	CXTPChartColor m_color;          //The color of the cirular boundary.
	int m_thickness;                //The thickness of the circular boundary.
	CXTPChartPointF m_center;        //The center point of the circle.
	double m_radius;                //The radius of the circle.
};

class _XTP_EXT_CLASS CXTPChartPolygonDeviceCommand : public CXTPChartDeviceCommand
{
protected:
	CXTPChartPolygonDeviceCommand(const CXTPChartPoints& points);

protected:
	CXTPChartElement* HitTest(CPoint point, CXTPChartElement* pParent) const;

protected:
	CXTPChartPoints m_points;    //The points which form the polygon.
	CXTPChartRectF m_bounds;             //The rectanglur broundary of the polygon.

};

//===========================================================================
// Summary:
//     This class represents a polygon device command,which is a kind of CXTPChartDeviceCommand.
//     It specifically handles the rendering of polygonal shapes in a chart.
// Remarks:
//===========================================================================
class _XTP_EXT_CLASS CXTPChartSolidPolygonDeviceCommand : public CXTPChartPolygonDeviceCommand
{
public:
	//-----------------------------------------------------------------------
	// Summary:
	//     Constructs a CXTPChartSolidPolygonDeviceCommand object.
	// Parameters:
	//     points    - The collection of points which make the polygon.
	//     color     - The color of boundary.
	// Remarks:
	//-----------------------------------------------------------------------
	CXTPChartSolidPolygonDeviceCommand(const CXTPChartPoints& points, const CXTPChartColor& color);

protected:
	//-----------------------------------------------------------------------
	// Summary:
	//     This is a virtual function override of base class CXTPChartDeviceContext,
	//     act polymorphically to do the actual drawing of the chart element, to which
	//     this device command is associated with.
	// Parameters:
	//     pDC      - The device context of the chart.
	// Remarks:
	//-----------------------------------------------------------------------
	void ExecuteOverride(CXTPChartDeviceContext* pDC);

protected:
	CXTPChartColor m_color;      //The color of the boundary.
};

//===========================================================================
// Summary:
//     This class represents a solid spline polygon device command,which is a kind
//     of CXTPChartDeviceCommand.It specifically handles the rendering of solid spline
//     polygonal shapes in a chart.
//
// Remarks:
//===========================================================================
class _XTP_EXT_CLASS CXTPChartSolidSplinePolygonDeviceCommand : public CXTPChartDeviceCommand
{
public:
	//-----------------------------------------------------------------------
	// Summary:
	//     Constructs a CXTPChartSolidSplinePolygonDeviceCommand object.
	// Parameters:
	//     points    - The collection of points which make the polygon spline.
	//     color     - The fill color of the solid spline.
	// Remarks:
	//-----------------------------------------------------------------------
	CXTPChartSolidSplinePolygonDeviceCommand(const CXTPChartPoints& points, const CXTPChartColor& color, BOOL bTwoSides = FALSE);
	~CXTPChartSolidSplinePolygonDeviceCommand();

protected:
	//-----------------------------------------------------------------------
	// Summary:
	//     This is a virtual function override of base class CXTPChartDeviceContext,
	//     act polymorphically to do the actual drawing of the chart element, to which
	//     this device command is associated with.
	// Parameters:
	//     pDC      - The device context of the chart.
	// Remarks:
	//-----------------------------------------------------------------------
	void ExecuteOverride(CXTPChartDeviceContext* pDC);

	CXTPChartElement* HitTest(CPoint point, CXTPChartElement* pParent) const;

protected:
	CXTPChartColor m_color;      //The fill color of the spline polygon.

	Gdiplus::GpPath* m_pGpPath;
};

//===========================================================================
// Summary:
//     This class represents a gradient polygon device command,which is a kind
//     of CXTPChartDeviceCommand.It specifically handles the rendering of gradient
//     polygonal shapes in a chart.
//
// Remarks:
//===========================================================================
class _XTP_EXT_CLASS CXTPChartGradientPolygonDeviceCommand : public CXTPChartPolygonDeviceCommand
{
public:
	//-----------------------------------------------------------------------
	// Summary:
	//     Constructs a CXTPChartGradientPolygonDeviceCommand object.
	// Parameters:
	//     points    - The collection of points which make the polygon spline.
	//     color     - The start color of the gradient.
	//     color2    - The end color of the gradient.
	//     nMode     - The linear gradient mode, it can be horizontal, vertical,
	//                 forward diagonal or backward diagonal.
	// Remarks:
	//-----------------------------------------------------------------------
	CXTPChartGradientPolygonDeviceCommand(const CXTPChartPoints& points, const CXTPChartColor& color, const CXTPChartColor& color2, XTPChartLinearGradientMode nMode);

protected:
	//-----------------------------------------------------------------------
	// Summary:
	//     This is a virtual function override of base class CXTPChartDeviceContext,
	//     act polymorphically to do the actual drawing of the chart element, to which
	//     this device command is associated with.
	// Parameters:
	//     pDC      - The device context of the chart.
	// Remarks:
	//-----------------------------------------------------------------------
	void ExecuteOverride(CXTPChartDeviceContext* pDC);

protected:
	CXTPChartColor m_color;              //The start color of the gradient.
	CXTPChartColor m_color2;             //The end color of the gradiednt.
	XTPChartLinearGradientMode m_nMode;  //The gradient mode.
};
//===========================================================================
// Summary:
//     This class represents a hatch polygon device command,which is a kind
//     of CXTPChartDeviceCommand.It specifically handles the rendering of hatch
//     polygonal shapes in a chart.
//
// Remarks:
//===========================================================================
class _XTP_EXT_CLASS CXTPChartHatchPolygonDeviceCommand : public CXTPChartPolygonDeviceCommand
{
public:
	//-----------------------------------------------------------------------
	// Summary:
	//     Constructs a CXTPChartHatchPolygonDeviceCommand object.
	// Parameters:
	//     points    - The collection of points which make the polygon spline.
	//     color     - The first color of the pattern.
	//     color2    - The second color of the pattern.
	//     nStyle    - The hatch style.
	// Remarks:
	//-----------------------------------------------------------------------
	CXTPChartHatchPolygonDeviceCommand(const CXTPChartPoints& points, const CXTPChartColor& color, const CXTPChartColor& color2, XTPChartHatchStyle nStyle);

protected:
	//-----------------------------------------------------------------------
	// Summary:
	//     This is a virtual function override of base class CXTPChartDeviceContext,
	//     act polymorphically to do the actual drawing of the chart element, to which
	//     this device command is associated with.
	// Parameters:
	//     pDC      - The device context of the chart.
	// Remarks:
	//-----------------------------------------------------------------------
	void ExecuteOverride(CXTPChartDeviceContext* pDC);

protected:
	CXTPChartColor m_color;          //The first color of the pattern.
	CXTPChartColor m_color2;         //The second color of the pattern.
	XTPChartHatchStyle m_nStyle;     //The hatch style.
};

//===========================================================================
// Summary:
//     This class represents a bounded rectangle device command,which is a kind of
//     CXTPChartDeviceCommand.It specifically handles the rendering of bounded rectangular
//     shapes in a chart.
// Remarks:
//===========================================================================
class _XTP_EXT_CLASS CXTPChartBoundedPolygonDeviceCommand : public CXTPChartDeviceCommand
{
public:
	//-----------------------------------------------------------------------
	// Summary:
	//     Constructs a CXTPChartBoundedPolygonDeviceCommand object.
	// Parameters:
	//     rect      - The rectangular area to be rendered by the device command.
	//     color     - The color of the rectangle border.
	//     thickness - The thickness of the rectangular border.
	// Remarks:
	//-----------------------------------------------------------------------
	CXTPChartBoundedPolygonDeviceCommand(const CXTPChartPoints& points, const CXTPChartColor& color, int thickness);

protected:
	//-----------------------------------------------------------------------
	// Summary:
	//     This is a virtual function override of base class CXTPChartDeviceContext,
	//     act polymorphically to do the actual drawing of the chart element, to which
	//     this device command is associated with.
	// Parameters:
	//     pDC      - The device context of the chart.
	// Remarks:
	//-----------------------------------------------------------------------
	void ExecuteOverride(CXTPChartDeviceContext* pDC);

protected:
	CXTPChartPoints m_points;    //The points which form the spline polygon.
	CXTPChartColor m_color;      //The color of the rectangle border.
	int m_thickness;            //The thickness of the rectangle border.
};

#endif //#if !defined(__XTPCHARTRECTANGLEDEVICECOMMAND_H__)
