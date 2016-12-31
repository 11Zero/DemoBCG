// XTPChartTransformationDeviceCommand.h
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
#if !defined(__XTPCHARTTRANSFORMATIONDEVICECOMMAND_H__)
#define __XTPCHARTTRANSFORMATIONDEVICECOMMAND_H__
//}}AFX_CODEJOCK_PRIVATE

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "XTPChartDeviceCommand.h"
#include "../Types/XTPChartDiagramPoint.h"
#include "../Types/XTPChartMatrix.h"

//===========================================================================
// Summary:
//     This class helps to save the state of drawing.This object is a kind of
//     CXTPChartDeviceCommand.
// Remarks:
//===========================================================================
class _XTP_EXT_CLASS CXTPChartSaveStateDeviceCommand : public CXTPChartDeviceCommand
{
public:
	//-----------------------------------------------------------------------
	// Summary:
	//     Constructs a CXTPChartSaveStateDeviceCommand object.
	//-----------------------------------------------------------------------
	CXTPChartSaveStateDeviceCommand();

protected:
	//-----------------------------------------------------------------------
	// Summary:
	//     Override this function to do some  works if any, before the drawing.
	// Parameters:
	//     pDC - A pointer to the chart device context.
	// Remarks:
	//-----------------------------------------------------------------------
	virtual void BeforeExecute(CXTPChartDeviceContext* pDC);
	virtual void AfterExecute(CXTPChartDeviceContext* pDC);

	void BeforeExecute(CXTPChartOpenGLDeviceContext* pDC);
	void AfterExecute(CXTPChartOpenGLDeviceContext* pDC);

protected:
	UINT m_nState;
};

//===========================================================================
// Summary:
//     This class abstracts the clipping operation of the sceen.This object is
//     a kind of CXTPChartDeviceCommand.
// Remarks:
//===========================================================================
class _XTP_EXT_CLASS CXTPChartClipDeviceCommand : public CXTPChartDeviceCommand
{
public:
	//-----------------------------------------------------------------------
	// Summary:
	//     Constructs a CXTPChartClipDeviceCommand object.
	//-----------------------------------------------------------------------
	CXTPChartClipDeviceCommand(CRect rcClip);

protected:
	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to do the steps necessary to do the clipping
	//     operation.
	// Parameters:
	//     pDC - A pointer to the chart device context.
	// Remarks:
	//-----------------------------------------------------------------------
	virtual void BeforeExecute(CXTPChartDeviceContext* pDC);

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to do the steps after the clipping operation.
	// Parameters:
	//     pDC - A pointer to the chart device context.
	// Remarks:
	//-----------------------------------------------------------------------
	virtual void AfterExecute(CXTPChartDeviceContext* pDC);


protected:
	CRect m_rcClip;     //The clipping rectangle.
	CRect m_rcState;    //The default clipping rectangle.
};

//===========================================================================
// Summary:
//     This class abstracts a transformation device command, which does matrix
//     transformtions.This is a kind of CXTPChartDeviceCommand.
// Remarks:
//===========================================================================
class _XTP_EXT_CLASS CXTPChartTransformDeviceCommand : public CXTPChartDeviceCommand
{
public:
	//-----------------------------------------------------------------------
	// Summary:
	//     Constructs a CXTPChartTransformDeviceCommand object.
	// Parameters:
	//     matrix - The chart matrix object, which represents a transformation.
	//-----------------------------------------------------------------------
	CXTPChartTransformDeviceCommand(const CXTPChartMatrix& matrix);

protected:
	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to do the actual drawing.
	// Parameters:
	//     pDC - A pointer to the chart device context.
	// Remarks:
	//-----------------------------------------------------------------------
	void ExecuteOverride(CXTPChartOpenGLDeviceContext* pDC);

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to do a matrix transformation.
	// Parameters:
	//     matrix - A pointer to the transformation matrix.
	// Remarks:
	//-----------------------------------------------------------------------
	void TransformMatrix(CXTPChartMatrix* matrix);

protected:
	CXTPChartMatrix m_matrix;    //The matrix.
};

//===========================================================================
// Summary:
//     This class abstracts a rotation device command, which does rotation
//     transformtions.This is a kind of CXTPChartDeviceCommand.
// Remarks:
//===========================================================================
class _XTP_EXT_CLASS CXTPChartRotateDeviceCommand  : public CXTPChartDeviceCommand
{
public:
	//-----------------------------------------------------------------------
	// Summary:
	//     Constructs a CXTPChartRotateDeviceCommand object.
	// Parameters:
	//     fAngle - The angle of rotation.
	//-----------------------------------------------------------------------
	CXTPChartRotateDeviceCommand(float fAngle);
	//-----------------------------------------------------------------------
	// Summary:
	//     Constructs a CXTPChartRotateDeviceCommand object.
	// Parameters:
	//     fAngle        - The angle of rotation.
	//     rotateVector  - The vector to be rotated.
	//-----------------------------------------------------------------------
	CXTPChartRotateDeviceCommand(float fAngle, const CXTPChartDiagramPoint& rotateVector);

public:
	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to do a matrix transformation, here it is a rotation.
	// Parameters:
	//     matrix - A pointer to the transformation matrix.
	// Remarks:
	//-----------------------------------------------------------------------
	virtual void TransformMatrix(CXTPChartMatrix* matrix);

	CXTPChartElement* HitTest(CPoint point, CXTPChartElement* pParent) const;
protected:
	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to do the actual drawing.
	// Parameters:
	//     pDC - A pointer to the chart device context.
	// Remarks:
	//-----------------------------------------------------------------------
	virtual void ExecuteOverride(CXTPChartDeviceContext* pDC);

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to do the actual drawing.
	// Parameters:
	//     pDC - A pointer to the OpenGL device context.
	// Remarks:
	//-----------------------------------------------------------------------
	virtual void ExecuteOverride(CXTPChartOpenGLDeviceContext* pDC);

protected:

	float m_fAngle;                         //The angle of rotation.
	CXTPChartDiagramPoint m_ptRotateVector;  //The rotation vector.
};

//===========================================================================
// Summary:
//     This class abstracts a rotation device command, which does translation
//     on vertices.This is a kind of CXTPChartDeviceCommand.
// Remarks:
//===========================================================================
class _XTP_EXT_CLASS CXTPChartTranslateDeviceCommand : public CXTPChartDeviceCommand
{
public:
	//-----------------------------------------------------------------------
	// Summary:
	//     Constructs a CXTPChartTranslateDeviceCommand object.
	// Parameters:
	//     dx - The change in X direction
	//     dy - The change in Y direction
	//     dz - The change in Z direction
	//-----------------------------------------------------------------------
	CXTPChartTranslateDeviceCommand(double dx, double dy, double dz);

protected:
	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to do the actual drawing.
	// Parameters:
	//     pDC - A pointer to the chart device context.
	// Remarks:
	//-----------------------------------------------------------------------
	virtual void ExecuteOverride(CXTPChartDeviceContext* pDC);

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to do the actual drawing.
	// Parameters:
	//     pDC - A pointer to the OpenGL device context.
	// Remarks:
	//-----------------------------------------------------------------------
	virtual void ExecuteOverride(CXTPChartOpenGLDeviceContext* pDC);

public:
	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to do a matrix transformation, here it is a
	//     translation
	// Parameters:
	//     matrix - A pointer to the transformation matrix.
	// Remarks:
	//-----------------------------------------------------------------------
	virtual void TransformMatrix(CXTPChartMatrix* matrix);

	CXTPChartElement* HitTest(CPoint point, CXTPChartElement* pParent) const;


protected:

	double m_dx;    //The change in X direction.
	double m_dy;    //The change in Y direction.
	double m_dz;    //The change in Z direction.
};

//===========================================================================
// Summary:
//     This class abstracts a view port device command, which does view port
//     transformations.This is a kind of CXTPChartDeviceCommand.
// Remarks:
//===========================================================================
class _XTP_EXT_CLASS CXTPChartViewPortDeviceCommand : public CXTPChartDeviceCommand
{
public:
	//-----------------------------------------------------------------------
	// Summary:
	//     Constructs a CXTPChartViewPortDeviceCommand object.
	// Parameters:
	//     rcViewPort - The rectangle defining the view port.
	//-----------------------------------------------------------------------
	CXTPChartViewPortDeviceCommand(CRect rcViewPort);

protected:
	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to do some tasks necessary before the actual
	//     transformation.
	// Parameters:
	//     pDC - The pointer to the OpenGL device context.
	//-----------------------------------------------------------------------
	void BeforeExecute(CXTPChartOpenGLDeviceContext* pDC);

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to do the view port stransfromations.
	// Parameters:
	//     pDC - The pointer to the OpenGL device context.
	//-----------------------------------------------------------------------
	void AfterExecute(CXTPChartOpenGLDeviceContext* pDC);

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to do the necessary clean-ups after transformations.
	// Parameters:
	//     pDC - The pointer to the OpenGL device context.
	//-----------------------------------------------------------------------
	void ExecuteOverride(CXTPChartOpenGLDeviceContext* pDC);

protected:
	CRect m_rcViewPort;             //The view port rectangle.
	int m_rcDefaultViewPort[4];     //The integer array for storing the default view port of the OpenGL.
};
//===========================================================================
// Summary:
//     This class helps to do the depth test on pixels.
//     The depth test is used to control if a pixel should be drawn or not,
//     based on the information stored in depth buffer.This test compare the
//     depth of the pixel to be drawn to the existing depth in the buffer.
// Remarks:
//===========================================================================
class _XTP_EXT_CLASS CXTPChartDepthTestDeviceCommand : public CXTPChartDeviceCommand
{
public:
	//-----------------------------------------------------------------------
	// Summary:
	//     Constructs a CXTPChartDepthTestDeviceCommand object.
	// Parameters:
	//     pDC - The pointer to the OpenGL device context.
	//-----------------------------------------------------------------------
	CXTPChartDepthTestDeviceCommand();

protected:
	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to enable the depth test, before the real drawing.
	// Parameters:
	//     pDC - The pointer to the OpenGL device context.
	//-----------------------------------------------------------------------
	void BeforeExecute(CXTPChartOpenGLDeviceContext* pDC);

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to disable the depth test, after the real drawing.
	// Parameters:
	//     pDC - The pointer to the OpenGL device context.
	//-----------------------------------------------------------------------
	void AfterExecute(CXTPChartOpenGLDeviceContext* pDC);

protected:
};

//===========================================================================
// Summary:
//     This class helps to do load the identity matrix to the matrix stack
//     to isolate the effects of modeling transformations.
// Remarks:
//===========================================================================
class _XTP_EXT_CLASS CXTPChartIdentityTransformDeviceCommand : public CXTPChartDeviceCommand
{
public:

public:
	//-----------------------------------------------------------------------
	// Summary:
	//     Constructs a CXTPChartIdentityTransformDeviceCommand object.
	// Parameters:
	//     nMatrixType - The matrix type.
	// Remarks:
	//     The matrix type can be model view or projection.
	//-----------------------------------------------------------------------
	CXTPChartIdentityTransformDeviceCommand(XTPChartMatrixType nMatrixType);

protected:
	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to do the identity matrix loading operation.
	// Parameters:
	//     pDC - The pointer to the OpenGL device context.
	//-----------------------------------------------------------------------
	void ExecuteOverride(CXTPChartOpenGLDeviceContext* pDC);

protected:
	XTPChartMatrixType m_nMatrixType;    //The matrix type.
};
//===========================================================================
// Summary:
//     This class abstracts a frustum projection device command.This class helps
//     to do the perspective projection.
// Remarks:
//===========================================================================
class _XTP_EXT_CLASS CXTPChartFrustumProjectionDeviceCommand : public CXTPChartDeviceCommand
{
public:
	//-----------------------------------------------------------------------
	// Summary:
	//     Constructs a CXTPChartFrustumProjectionDeviceCommand object.
	// Parameters:
	//     p1 - The first point which defines the frustum.
	//     p2 - The first point which defines the frustum.
	// Remarks:
	//     The matrix type can be model view or projection.
	//-----------------------------------------------------------------------
	CXTPChartFrustumProjectionDeviceCommand(CXTPChartDiagramPoint p1, CXTPChartDiagramPoint p2);

protected:
	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to does the projection operation.
	// Parameters:
	//     pDC - The pointer to the OpenGL device context.
	//-----------------------------------------------------------------------
	void ExecuteOverride(CXTPChartOpenGLDeviceContext* pDC);

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to do a projection transformation.
	// Parameters:
	//     matrix - A pointer to the matrix to be transformed.
	// Remarks:
	//-----------------------------------------------------------------------
	void TransformMatrix(CXTPChartMatrix* matrix);

protected:
	CXTPChartDiagramPoint m_p1;  //Frustum point.
	CXTPChartDiagramPoint m_p2;  //Frustum point.
};


#endif //#if !defined(__XTPCHARTTRANSFORMATIONDEVICECOMMAND_H__)
