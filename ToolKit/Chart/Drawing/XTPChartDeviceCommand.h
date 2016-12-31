// XTPChartDeviceCommand.h
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
#if !defined(__XTPCHARTDEVICECOMMAND_H__)
#define __XTPCHARTDEVICECOMMAND_H__
//}}AFX_CODEJOCK_PRIVATE

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "../Types/XTPChartTypes.h"
#include "../Types/XTPChartDiagramPoint.h"

class CXTPChartDeviceCommand;
class CXTPChartDeviceContext;
class CXTPChartOpenGLDeviceContext;
class CXTPChartFont;
class CXTPChartMatrix;
class CXTPChartElement;

//===========================================================================
// Summary:
//     This class handles the rendering elements in a chart.
//     This class act as a base class for all the specialized device
//     command objects which do specific rendering jobs related to each
//     element in a chart.
// Remarks:
//===========================================================================
class _XTP_EXT_CLASS CXTPChartDeviceCommand
{
public:

	//-----------------------------------------------------------------------
	// Summary:
	//     Constructs a CXTPChartDeviceCommand object.
	// Remarks:
	//-----------------------------------------------------------------------
	CXTPChartDeviceCommand();

	//-------------------------------------------------------------------------
	// Summary:
	//     Destroys a CXTPChartDeviceCommand object, handles cleanup
	//-------------------------------------------------------------------------
	virtual ~CXTPChartDeviceCommand();

public:

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to add a child device command.
	// Parameters:
	//     pCommand - A pointer to the child device command.
	// Remarks:
	//     A device command object keeps an array of child device commands, an
	//     instruction to execute the drawing of the parent object trigger the
	//     drawing of children as well.
	//-----------------------------------------------------------------------
	CXTPChartDeviceCommand* AddChildCommand(CXTPChartDeviceCommand* pCommand);

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to execute the drawing.
	// Parameters:
	//     pDC - A pointer to the chart device context.
	// Remarks:
	//     An instruction to execute the drawing of the parent object trigger the
	//     drawing of children as well.
	//-----------------------------------------------------------------------
	virtual void Execute(CXTPChartDeviceContext* pDC);

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to execute the drawing in OpenGL mode.
	// Parameters:
	//     pDC - A pointer to the chart OpenGL device context.
	// Remarks:
	//     An instruction to execute the drawing of the parent object triggers the
	//     drawing of children as well.
	//-----------------------------------------------------------------------
	virtual void Execute(CXTPChartOpenGLDeviceContext* pDC);

	//-----------------------------------------------------------------------
	// Summary:
	//     Override this function to do some ground works if any, before the drawing.
	// Parameters:
	//     pDC - A pointer to the chart device context.
	// Remarks:
	//-----------------------------------------------------------------------
	virtual void BeforeExecute(CXTPChartDeviceContext* pDC);

	//-----------------------------------------------------------------------
	// Summary:
	//     Override this function to do some final cut if any, after the drawing.
	// Parameters:
	//     pDC - A pointer to the chart device context.
	// Remarks:
	//-----------------------------------------------------------------------
	virtual void AfterExecute(CXTPChartDeviceContext* pDC);

	//-----------------------------------------------------------------------
	// Summary:
	//     Override this function to write the drawing code for specific objects.
	// Parameters:
	//     pDC - A pointer to the chart device context.
	// Remarks:
	//-----------------------------------------------------------------------
	virtual void ExecuteOverride(CXTPChartDeviceContext* pDC);

	//-----------------------------------------------------------------------
	// Summary:
	//     Override this function to do some ground works if any, before the OpenGL
	//      mode drawing.
	// Parameters:
	//     pDC - A pointer to the chart OpenGL device context.
	// Remarks:
	//-----------------------------------------------------------------------
	virtual void BeforeExecute(CXTPChartOpenGLDeviceContext* pDC);

	//-----------------------------------------------------------------------
	// Summary:
	//     Override this function to do some final cut if any, after the OpenGL
	//     drawing.
	// Parameters:
	//     pDC - A pointer to the chart OpenGL device context.
	// Remarks:
	//-----------------------------------------------------------------------
	virtual void AfterExecute(CXTPChartOpenGLDeviceContext* pDC);

	//-----------------------------------------------------------------------
	// Summary:
	//     Override this function to write the drawing code in OpenGL for
	//     specific objects.
	// Parameters:
	//     pDC - A pointer to the chart OpenGL device context.
	// Remarks:
	//-----------------------------------------------------------------------
	virtual void ExecuteOverride(CXTPChartOpenGLDeviceContext* pDC);

	virtual CXTPChartElement* HitTest(CPoint point, CXTPChartElement* pParent) const;

public:
	//-----------------------------------------------------------------------
	// Summary:
	//     Override this function to write the code for the transformation of
	//     matrices.
	// Parameters:
	//     matrix - A pointer to the matrix.
	// Remarks:
	//-----------------------------------------------------------------------
	virtual void TransformMatrix(CXTPChartMatrix* matrix);

protected:
	CArray<CXTPChartDeviceCommand*, CXTPChartDeviceCommand*> m_arrChildren;   //The array of child device commands.
};

//===========================================================================
// Summary:
//     This class is a kind of CXTPChartDeviceCommand and it does handle some
//     house keeping works when the drawing mode is switched between native
//     and OpenGL.
// Remarks:
//===========================================================================
class _XTP_EXT_CLASS CXTPChartDrawingTypeDeviceCommand : public CXTPChartDeviceCommand
{
public:
	//-----------------------------------------------------------------------
	// Summary:
	//     Constructs a CXTPChartDrawingTypeDeviceCommand object.
	// Parameters:
	//      bNativeDrawing - TRUE if the drawing is native and FALSE if the drawing
	//      is OpenGL.
	// Remarks:
	//-----------------------------------------------------------------------
	CXTPChartDrawingTypeDeviceCommand(BOOL bNativeDrawing);

protected:
	//-----------------------------------------------------------------------
	// Summary:
	//     Override this function to do some ground works if any, before the OpenGL
	//      mode drawing.
	// Parameters:
	//     pDC - A pointer to the chart OpenGL device context.
	// Remarks:
	//-----------------------------------------------------------------------
	virtual void BeforeExecute(CXTPChartOpenGLDeviceContext* pDC);
	virtual void BeforeExecute(CXTPChartDeviceContext* pDC);

	//-----------------------------------------------------------------------
	// Summary:
	//     Override this function to do some final cut if any, after the OpenGL
	//     drawing.
	// Parameters:
	//     pDC - A pointer to the chart OpenGL device context.
	// Remarks:
	//-----------------------------------------------------------------------
	virtual void AfterExecute(CXTPChartOpenGLDeviceContext* pDC);
	virtual void AfterExecute(CXTPChartDeviceContext* pDC);

protected:
	BOOL m_bDrawingType;        //TRUE if the native drawing is used and FALSE if OpenGL is used.
	BOOL m_bOldDrawingType;     //Stores the old drawing mode.
};

//===========================================================================
// Summary:
//     This class is a kind of CXTPChartDeviceCommand and it draws the element and
//     does some additional task to smooth the drawings using antialiasing.
// Remarks:
//===========================================================================
class _XTP_EXT_CLASS CXTPChartPolygonAntialiasingDeviceCommand : public CXTPChartDeviceCommand
{
public:
	//-----------------------------------------------------------------------
	// Summary:
	//     Constructs a CXTPChartPolygonAntialiasingDeviceCommand object.
	// Parameters:
	//      bAntiAlias - TRUE if the antialiasing is enabled and FALSE if the
	//      antialiasing is disabled.
	// Remarks:
	//-----------------------------------------------------------------------
	CXTPChartPolygonAntialiasingDeviceCommand(BOOL bAntiAlias = TRUE);
	virtual ~CXTPChartPolygonAntialiasingDeviceCommand();

protected:
	//-----------------------------------------------------------------------
	// Summary:
	//      This override do some ground works if any, before the OpenGL mode
	//      antialiased drawing of polygons.
	// Parameters:
	//     pDC - A pointer to the chart OpenGL device context.
	// Remarks:
	//-----------------------------------------------------------------------
	void BeforeExecute(CXTPChartOpenGLDeviceContext* pDC);

	//-----------------------------------------------------------------------
	// Summary:
	//     Override this function to write the drawing code in OpenGL for
	//     specific objects.
	// Parameters:
	//     pDC - A pointer to the chart OpenGL device context.
	// Remarks:
	//     An instruction to execute the drawing of the object triggers the
	//     drawing of children as well.
	//-----------------------------------------------------------------------
	void Execute(CXTPChartOpenGLDeviceContext* pDC);

	//-----------------------------------------------------------------------
	// Summary:
	//     This override do some final cut if any, after the OpenGL antialiased
	//     drawing of polygons.
	// Parameters:
	//     pDC - A pointer to the chart OpenGL device context.
	// Remarks:
	//-----------------------------------------------------------------------
	void AfterExecute(CXTPChartOpenGLDeviceContext* pDC);

	void RestoreImage(CSize size);
	//-----------------------------------------------------------------------
	// Summary:
	//      This override do some ground works if any, before the native mode
	//      antialiased drawing of polygons.
	// Parameters:
	//     pDC - A pointer to the chart native device context.
	// Remarks:
	//-----------------------------------------------------------------------
	void BeforeExecute(CXTPChartDeviceContext* pDC);

	//-----------------------------------------------------------------------
	// Summary:
	//     This override do some final cut if any, after the native antialiased
	//     drawing of polygons.
	// Parameters:
	//     pDC - A pointer to the chart native device context.
	// Remarks:
	//-----------------------------------------------------------------------
	void AfterExecute(CXTPChartDeviceContext* pDC);

protected:
	int shiftCount;
	double shifts[4];
	int viewport[4];                //The view port.
	double projectionMatrix[16];    //The projection matrix.
	LPBYTE pixelData;               //The pixel data.

	BOOL m_bAntiAlias;              //TRUE if antialiasing enabled, FALSE if not.
	long m_bOldAntiAlias;           //The previous value of anti aliasing.
};

//===========================================================================
// Summary:
//     This class is a kind of CXTPChartDeviceCommand and it helps in drawing
//     lighting for OpenGL drawing.
// Remarks:
//===========================================================================
class _XTP_EXT_CLASS CXTPChartLightingDeviceCommand : public CXTPChartDeviceCommand
{
public:
	//-----------------------------------------------------------------------
	// Summary:
	//     Constructs a CXTPChartLightingDeviceCommand object.
	// Parameters:
	//      ambientColor            - The ambient color.
	//      materialSpecularColor   - The material specular reflection color.
	//      materialEmissionColor   - The material emission color.
	//      materialShininess       - The material shininess value.
	// Remarks:
	//-----------------------------------------------------------------------
	CXTPChartLightingDeviceCommand(CXTPChartColor ambientColor, CXTPChartColor materialSpecularColor,
		CXTPChartColor materialEmissionColor, float materialShininess);

protected:
	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to do some final cut if any, after the drawing.
	// Parameters:
	//     pDC - A pointer to the OpenGL device context.
	// Remarks:
	//-----------------------------------------------------------------------
	void AfterExecute(CXTPChartOpenGLDeviceContext* pDC);

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to write the drawing code in OpenGL for
	//     specific objects.
	// Parameters:
	//     pDC - A pointer to the chart OpenGL device context.
	// Remarks:
	//-----------------------------------------------------------------------
	void ExecuteOverride(CXTPChartOpenGLDeviceContext* pDC);

protected:
	CXTPChartColor m_ambientColor;           //This specifies the ambient RGBA reflectance of the material.
	CXTPChartColor m_materialSpecularColor;  //This specifies the specular RGBA reflectance of the material.
	CXTPChartColor m_materialEmissionColor;  //This specifies the RGBA emitted light intensity of the material.
	float m_materialShininess;              //This float value specifies the RGBA specular exponent of the material.

};
//===========================================================================
// Summary:
//     This class is a kind of CXTPChartDeviceCommand and it helps in providing
//     light source for  drawing lighting for OpenGL.
// Remarks:
//===========================================================================
class _XTP_EXT_CLASS CXTPChartLightDeviceCommand : public CXTPChartDeviceCommand
{
public:
	//-----------------------------------------------------------------------
	// Summary:
	//     Constructs a CXTPChartLightingDeviceCommand object.
	// Parameters:
	//      index                   - The light source number.
	//      ambientColor            - This specifies the ambient RGBA intensity of the light.
	//                                Ambient light is the average volume of light that is created
	//                                by emission of light from all of the light sources surrounding
	//                                (or located inside of) the lit area.
	//      diffuseColor            - This specifies the diffuse RGBA intensity of the light.Diffuse
	//                                light represents a directional light cast by a light source.
	//      specularColor           - This specifys the specular RGBA intensity of the light.The
	//                                specular light reflects off the surface in a sharp and uniform way.
	//      position                - The light source position.
	//      spotDirection           - The spot light direction.
	//      spotExponent            - The spot exponent, a floating-point value that specifies the
	//                                intensity distribution of the light.
	//      spotCutoff              - The spot cut off value.This floating-point value specifies
	//                                the maximum spread angle of a light source
	//      constantAttenuation     - The constant attennuation.Constant attenuation affects the
	//                                overall intensity of the light, regardless of the distance
	//                                of a surface from the light source.
	//      linearAttenuation       - The linear attenuation.In linear attenuation ss a light moves
	//                                away from a surface, the intensity of light striking the surface
	//                                is inversely proportional to the distance between the light and
	//                                the object.
	//      quadraticAttenuation    - The quadratic attenuation.In Quadratic attenuation, the light
	//                                intensity is attenuated by the square of the distance between
	//                                the surface and the light.
	// Remarks:
	//-----------------------------------------------------------------------
	CXTPChartLightDeviceCommand(int index, CXTPChartColor ambientColor, CXTPChartColor diffuseColor, CXTPChartColor specularColor,
		CXTPChartDiagramPoint position, CXTPChartDiagramPoint spotDirection, float spotExponent, float spotCutoff, float constantAttenuation, float linearAttenuation, float quadraticAttenuation);

protected:
	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to do some final cut if any, after the drawing.
	// Parameters:
	//     pDC - A pointer to the OpenGL device context.
	// Remarks:
	//-----------------------------------------------------------------------
	void AfterExecute(CXTPChartOpenGLDeviceContext* pDC);

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to write the drawing code in OpenGL for
	//     specific objects.
	// Parameters:
	//     pDC - A pointer to the chart OpenGL device context.
	// Remarks:
	//-----------------------------------------------------------------------
	void ExecuteOverride(CXTPChartOpenGLDeviceContext* pDC);

protected:
	int m_index;                            //The light source index.
	CXTPChartColor m_ambientColor;           //The ambient light color.
	CXTPChartColor m_diffuseColor;           //The diffuse light color.
	CXTPChartColor m_specularColor;          //The specular light color.
	CXTPChartDiagramPoint m_position;        //The position of the light in homogeneous object coordinates.
	CXTPChartDiagramPoint m_spotDirection;   //The direction of the light in homogeneous object coordinates.
	float m_spotExponent;                   //The intensity distribution of the light.
	float m_spotCutoff;                     //The maximum spread angle of a light source.
	float m_constantAttenuation;            //The constant attenuation.It affects the overall intensity of the light,
	                                        //regardless of the distance of a surface from the light source.
	float m_linearAttenuation;              //The linear attenuation.In linear attenuation a light moves
	                                        //away from a surface, the intensity of light striking the surface
	                                        //is inversely proportional to the distance between the light and
	                                        //the object.
	float m_quadraticAttenuation;           //The Quadratic attenuation, Here the light intensity is attenuated by
	                                        //the square of the distance between the surface and the light.
	bool m_directional;                     //TRUE if the light is directional, FALSE if not.
};


class CXTPChartHitTestElementCommand : public CXTPChartDeviceCommand
{
public:
	CXTPChartHitTestElementCommand(CXTPChartElement* pElement);
	CXTPChartHitTestElementCommand(CXTPChartElement* pElement, const CRect& rcBounds);
	CXTPChartHitTestElementCommand(CXTPChartElement* pElement, const CXTPChartRectF& rcBounds);

public:
	virtual CXTPChartElement* HitTest(CPoint point, CXTPChartElement* pParent) const;

protected:
	CXTPChartElement* m_pElement;
	CRect m_rcBounds;
};

#endif //#if !defined(__XTPCHARTDEVICECOMMAND_H__)
