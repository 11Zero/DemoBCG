// XTPChartOpenGLHelpers.cpp
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

#include "GraphicLibrary/OpenGL/Gl.h"

#include "XTPChartOpenGLHelpers.h"

void CXTPChartOpenGLHelpers::DrawQuad(double dWidth, double dHeight)
{
	glDisable(GL_DEPTH_TEST);

	glBegin(GL_QUADS);
	glTexCoord2d(0, 0);
	glVertex3d(0, 0, 0.0);
	glTexCoord2d(0, 1);
	glVertex3d(0, 0 + dHeight, 0.0);
	glTexCoord2d(1, 1);
	glVertex3d(dWidth, dHeight, 0.0);
	glTexCoord2d(1, 0);
	glVertex3d(dWidth, 0, 0.0);
	glEnd();

	glEnable(GL_DEPTH_TEST);
}
