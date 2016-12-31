// XTPChartOpenGLHelpers.h
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
#if !defined(__XTPCHARTOPENGLHELPERS_H__)
#define __XTPCHARTOPENGLHELPERS_H__
//}}AFX_CODEJOCK_PRIVATE

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


struct _XTP_EXT_CLASS CXTPChartPoint3f
{
	float x, y, z;

	void SetValue(float xx, float yy, float zz)
	{
		x = xx;
		y = yy;
		z = zz;
	}
};

struct _XTP_EXT_CLASS CXTPChartColor4f
{
	float r, g, b, a;

	void SetValue(float rr, float gg, float bb, float aa)
	{
		r = rr;
		g = gg;
		b = bb;
		a = aa;
	}
};

class _XTP_EXT_CLASS CXTPChartOpenGLHelpers
{
public:

	static void AFX_CDECL DrawQuad(double dWidth, double dHeight);
};


#endif //#if !defined(__XTPCHARTOPENGLHELPERS_H__)
