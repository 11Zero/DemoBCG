// XTPChartOpenGLDeviceContext.h
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
#if !defined(__XTPCHARTOPENGLDEVICECONTEXT_H__)
#define __XTPCHARTOPENGLDEVICECONTEXT_H__
//}}AFX_CODEJOCK_PRIVATE

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "XTPChartDeviceContext.h"

namespace Gdiplus
{
	class GpBitmap;
};

//===========================================================================
// Summary:
//     This class abstracts an OpenGL device context, A device context stores,
//     retrieves, and modifies the attributes of graphic objects and
//     specifies graphic modes.This class is a kind of CXTPChartDeviceContext
//     to enhance the MFC command routing.
//
// Remarks:
//===========================================================================
class _XTP_EXT_CLASS CXTPChartOpenGLDeviceContext : public CXTPChartDeviceContext
{
public:
	//-----------------------------------------------------------------------
	// Summary:
	//     Constructs a CXTPChartOpenGLDeviceContext object.
	// Parameters:
	//     hDC      - Handle to the windows device context.
	//     szBounds - The bounds for the device context.
	// Remarks:
	//-----------------------------------------------------------------------
	CXTPChartOpenGLDeviceContext(CXTPChartContainer* pContainer, HDC hDC, CSize szBounds, BOOL bWindowDC);

	//-------------------------------------------------------------------------
	// Summary:
	//     Destroys a CXTPChartOpenGLDeviceContext  object, handles cleanup
	//-------------------------------------------------------------------------
	virtual ~CXTPChartOpenGLDeviceContext();

public:

	//-------------------------------------------------------------------------
	// Summary:
	//     Call this function to get the bounds of the device context.
	// Returns:
	//     A CSize object specifying the bounds.
	//-------------------------------------------------------------------------
	CSize GetSize() const;

public:
	//-------------------------------------------------------------------------
	// Summary:
	//     Call this function to calculate the color components from a chart
	//     color object.
	// Parameters:
	//     color - A reference to chart color object denoting an ARGB value.
	//     res   - An array of float for the individual color values and
	//             alpha.
	//-------------------------------------------------------------------------
	void CalculateColorComponents(CXTPChartColor& color, float res[4]);

	//-------------------------------------------------------------------------
	// Summary:
	//     Call this function to enable/disable native drawing.
	// Parameters:
	//     bNativeDrawing - TRUE to enable the native and FALSE to disable.
	//-------------------------------------------------------------------------
	void SetNativeDrawing(BOOL bNativeDrawing);

	//-------------------------------------------------------------------------
	// Summary:
	//     Call this function to get whether native drawing is enabled or not.
	// Returns:
	//     TRUE if the native drawing is enabled and FALSE if not.
	//-------------------------------------------------------------------------
	BOOL GetNativeDrawing() const;

protected:
	//-------------------------------------------------------------------------
	// Summary:
	//     Call this function to create an OpenGL rendering context.
	// Parameters:
	//     hDC         - The windows device context.
	//     pixelFormat - The GDI pixel format.
	//     pfd         - This structure that contains the logical pixel format
	//                   specification.
	// Returns:
	//     TRUE if the operation is success and FALSE if not.
	//-------------------------------------------------------------------------
	BOOL CreateContext(HDC hDC, int pixelFormat, PIXELFORMATDESCRIPTOR& pfd);

	//-------------------------------------------------------------------------
	// Summary:
	//     Call this function to create the GDI+ native objects.
	//-------------------------------------------------------------------------
	void CreateNativeObjects();

	//-------------------------------------------------------------------------
	// Summary:
	//     Call this function to execute the drawing code of OpenGL.
	// Parameters:
	//     pCommand - A pointer to chart device command object.
	//-------------------------------------------------------------------------
	virtual void Execute(CXTPChartDeviceCommand* pCommand);

	//-------------------------------------------------------------------------
	// Summary:
	//     Call this function to execute the drawing code of OpenGL.
	// Parameters:
	//     pCommand - A pointer to chart device command object.
	//-------------------------------------------------------------------------

	//{{AFX_CODEJOCK_PRIVATE
	void DrawBitmap();
	//}}AFX_CODEJOCK_PRIVATE
public:
	CSize m_szBounds;       //The bounds of the device context.
	CSize m_szBitmap;
	BOOL m_bWindowDC;       //TRUE if window device context is used, FALSE if not.
	HGLRC m_hglrc;          //
	BOOL m_bDoubleBuffered; //TRUE if double buffered drawing is used, FALSE if not.
	BOOL m_bNativeDrawing;  //TRUE if native drawing is used, FALSE if not.

	Gdiplus::GpBitmap* m_pGpBitmap;  //The GDI+ bitmap object.
};


AFX_INLINE CSize CXTPChartOpenGLDeviceContext::GetSize() const {
	return m_szBounds;
}
AFX_INLINE BOOL CXTPChartOpenGLDeviceContext::GetNativeDrawing() const {
	return m_bNativeDrawing;
}


#endif //#if !defined(__XTPCHARTOPENGLDEVICECONTEXT_H__)
