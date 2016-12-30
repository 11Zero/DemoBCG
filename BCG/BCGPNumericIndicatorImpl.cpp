//*******************************************************************************
// COPYRIGHT NOTES
// ---------------
// This is a part of the BCGControlBar Library
// Copyright (C) 1998-2014 BCGSoft Ltd.
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.
 //*******************************************************************************
//
// BCGPNumericIndicatorImpl.cpp: implementation of the CBCGPNumericIndicatorImpl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "bcgprores.h"
#include "BCGPMath.h"
#include "BCGPVisualManager.h"
#include "BCGPNumericIndicatorImpl.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

void CBCGPDigit::XSegment::CreateDrawPoints(double scale)
{
	m_DrawPoints.RemoveAll ();
	m_DrawPoints.Copy (m_Points);

	m_DrawPoints *= scale;
}

void CBCGPDigit::XSegment::Draw(CBCGPGraphicsManager& gm, const CBCGPBrush& br, const CBCGPRect& rect)
{
	CBCGPPoint ptOffset (rect.TopLeft ());

	CBCGPPointsArray points;
	points.Copy (m_DrawPoints);

	points += ptOffset;

	if (points.GetSize () > 2)
	{
		CBCGPPolygonGeometry geometry;
		geometry.SetPoints (points);
		gm.FillGeometry (geometry, br);
	}
	else
	{
		gm.DrawLine (points[0], points[1], br);
	}
}

CBCGPRect CBCGPDigit::XSegment::GetBounds() const
{
	CBCGPRect bounds(m_Points.GetBoundsRect ());
	if (bounds.Width() < 1.0)
	{
		bounds.SetWidth (1.0);
	}
	if (bounds.Height() < 1.0)
	{
		bounds.SetHeight (1.0);
	}

	return bounds;
}

void CBCGPDigit::XDigit::CreateDrawPoints(double scale)
{
	if (m_Scale == scale)
	{
		return;
	}

	m_Scale = scale;
	for (int i = 0; i < m_Segments.GetSize (); i++)
	{
		m_Segments[i].CreateDrawPoints (m_Scale);
	}
}

void CBCGPDigit::XDigit::DrawSegments(CBCGPGraphicsManager& gm, const CBCGPBrush& br, const CString& strSegments, const CBCGPRect& rect, BOOL bInvert)
{
	for (int i = 0; i < m_Segments.GetSize (); i++)
	{
		TCHAR ch = strSegments[i];

		if (bInvert)
		{
			ch = ch == TCHAR('0') ? TCHAR('1') : TCHAR('0');
		}

		if (ch == TCHAR('0'))
		{
			continue;
		}

		m_Segments[i].Draw(gm, br, rect);
	}
}

void CBCGPDigit::XDigit::DrawDot(CBCGPGraphicsManager& gm, const CBCGPBrush& br, const CBCGPRect& rect)
{
	double size = m_Dot.Width() * rect.Width () / m_Dot.right;
	if (size < 1.0)
	{
		size = 1.0;
	}

	gm.FillRectangle (CBCGPRect (rect.right - size, rect.bottom - size, rect.right, rect.bottom), br);
}

void CBCGPDigit::XDigit::Initialize(double skewAngle)
{
	const int count = (int)m_Segments.GetSize ();
	int i = 0;

	if (skewAngle != 0.0 && m_Segments[0].m_Points.GetSize() > 2)
	{
		const double skew = tan(bcg_deg2rad(bcg_clamp(skewAngle, -75.0, 75.0)));
		for (i = 0; i < count; i++)
		{
			CBCGPPointsArray& points = m_Segments[i].m_Points;
			for (int j = 0; j < points.GetSize (); j++)
			{
				points[j] = CBCGPPoint (points[j].x + skew * points[j].y, points[j].y);
			}
		}
	}

	m_Bounds = m_Segments[0].GetBounds ();

	for (i = 1; i < count; i++)
	{
		m_Bounds |= m_Segments[i].GetBounds ();
	}

	CBCGPPoint ptOffset(-m_Bounds.TopLeft ().x, 0.0);
	m_Bounds.OffsetRect (ptOffset);

	for (i = 0; i < count; i++)
	{
		m_Segments[i].m_Points += ptOffset;
	}
}

CBCGPRect CBCGPDigit::XDigit::GetBounds(BOOL bDot) const
{
	CBCGPRect rect(m_Bounds);

	if (bDot)
	{
		rect.right = m_Dot.right;
	}

	return rect;
}

CBCGPDigit::CBCGPDigit()
	: m_Digit(0)
{
}

CBCGPDigit::~CBCGPDigit()
{
}

CBCGPRect CBCGPDigit::CreateDigit(CBCGPGraphicsManager& gm, const CBCGPRect& rect, BOOL bDot)
{
	BOOL bFixed = gm.GetType () == CBCGPGraphicsManager::BCGP_GRAPHICS_MANAGER_GDI;

	CBCGPRect rectCell(rect);
	CBCGPSize size(rectCell.Size ());

	const double c_dblScale = bFixed ? 1.0 : 0.75;

	CArray<XDigit, XDigit>& digits = bFixed ? m_DigitsFix : m_Digits;

	m_Digit = 0;
	for (int i = (int)digits.GetSize () - 1; i >= 0; i--)
	{
		CBCGPSize sizeSrc (digits[i].GetBounds (bDot).Size ());
		double scale = min(size.cx / sizeSrc.cx, size.cy / sizeSrc.cy);
		if (scale >= c_dblScale)
		{
			m_Digit = i;
			break;
		}
	}

	CBCGPSize sizeSrc (digits[m_Digit].GetBounds (bDot).Size ());

	bFixed = bFixed && m_Digit > 0 && m_Digit < (digits.GetSize() - 1);

	double scale = bFixed ? 1.0 : min(size.cx / sizeSrc.cx, size.cy / sizeSrc.cy);
	rectCell.SetSize (sizeSrc * scale);
	rectCell.OffsetRect ((size - rectCell.Size ()) / 2.0);

	digits[m_Digit].CreateDrawPoints (scale);

	return rectCell;
}

void CBCGPDigit::DrawDigit(CBCGPGraphicsManager& gm, const CBCGPBrush& br, TCHAR ch, const CBCGPRect& rect, double opacityInv)
{
	CString strSegments;

	switch(ch)
	{
	case TCHAR('-'):
		strSegments = _T("0000001");
		break;

	case TCHAR('0'):
		strSegments = _T("1111110");
		break;

	case TCHAR('1'):
		strSegments = _T("0011000");
		break;

	case TCHAR('2'):
		strSegments = _T("0110111");
		break;

	case TCHAR('3'):
		strSegments = _T("0111101");
		break;

	case TCHAR('4'):
		strSegments = _T("1011001");
		break;

	case TCHAR('5'):
		strSegments = _T("1101101");
		break;

	case TCHAR('6'):
		strSegments = _T("1101111");
		break;

	case TCHAR('7'):
		strSegments = _T("0111000");
		break;

	case TCHAR('8'):
		strSegments = _T("1111111");
		break;

	case TCHAR('9'):
		strSegments = _T("1111101");
		break;

	case TCHAR('A'):
	case TCHAR('a'):
		strSegments = _T("1111011");
		break;

	case TCHAR('B'):
	case TCHAR('b'):
		strSegments = _T("1001111");
		break;

	case TCHAR('C'):
	case TCHAR('c'):
		strSegments = _T("1100110");
		break;

	case TCHAR('D'):
	case TCHAR('d'):
		strSegments = _T("0011111");
		break;

	case TCHAR('E'):
	case TCHAR('e'):
		strSegments = _T("1100111");
		break;

	case TCHAR('F'):
	case TCHAR('f'):
		strSegments = _T("1100011");
		break;

	case TCHAR('O'):
	case TCHAR('o'):
		strSegments = _T("0001111");
		break;

	case TCHAR('R'):
	case TCHAR('r'):
		strSegments = _T("0000011");
		break;
	}

	if (strSegments.IsEmpty())
	{
		return;
	}

	opacityInv = br.GetOpacity() * bcg_clamp(opacityInv, 0.0, 1.0);

	if (gm.GetType() == CBCGPGraphicsManager::BCGP_GRAPHICS_MANAGER_GDI)
	{
		m_DigitsFix[m_Digit].DrawSegments(gm, br, strSegments, rect);
	}
	else
	{
		if (opacityInv != 0.0)
		{
			CBCGPBrush brInv(br);
			brInv.SetOpacity(opacityInv);

			m_Digits[m_Digit].DrawSegments(gm, brInv, strSegments, rect, TRUE);
		}

		m_Digits[m_Digit].DrawSegments(gm, br, strSegments, rect, FALSE);
	}
}

void CBCGPDigit::DrawDot(CBCGPGraphicsManager& gm, const CBCGPBrush& br, const CBCGPRect& rect)
{
	if (gm.GetType() == CBCGPGraphicsManager::BCGP_GRAPHICS_MANAGER_GDI)
	{
		m_DigitsFix[m_Digit].DrawDot(gm, br, rect);
	}
	else
	{
		m_Digits[m_Digit].DrawDot(gm, br, rect);
	}
}

void CBCGPDigit::Initialize(double skewAngle)
{
	int i = 0;
	for (i = 0; i < m_DigitsFix.GetSize(); i++)
	{
		m_DigitsFix[i].Initialize(0.0);
	}

	for (i = 0; i < m_Digits.GetSize(); i++)
	{
		m_Digits[i].Initialize(skewAngle);
	}
}

CBCGPSize CBCGPDigit::GetDefaultSize(CBCGPGraphicsManager& gm, BOOL bDot) const
{
	CBCGPSize size(0.0, 0.0);

	if (m_Digits.GetSize () > 0)
	{
		double dblRatio = gm.GetType() == CBCGPGraphicsManager::BCGP_GRAPHICS_MANAGER_GDI ? 2.0 : 1.75;
		size = m_Digits[0].GetBounds (bDot).Size () * dblRatio;
	}

	return size;
}

class CBCGPDigit8: public CBCGPDigit
{
public:
	CBCGPDigit8() {}
	virtual ~CBCGPDigit8() {}

	virtual void Create (double skewAngle = 0.0)
	{
		const int count_s = 7;
		const int count_p = 6;

//GDI points
		const int count_fix_d = 5;
		// width 1
		const double pts_fix_1[count_s][count_p][2] = {
			{{0.0,  4.0}, {0.0,  0.0}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}},
			{{1.0,  0.0}, {5.0,  0.0}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}},
			{{5.0,  1.0}, {5.0,  5.0}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}},
			{{5.0,  6.0}, {5.0, 10.0}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}},
			{{4.0, 10.0}, {0.0, 10.0}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}},
			{{0.0,  9.0}, {0.0,  5.0}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}},
			{{1.0,  5.0}, {5.0,  5.0}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}}};

		// width 2
		const double pts_fix_2[count_s][count_p][2] = {
			{{ 0.0,  0.0}, { 2.0,  2.0}, { 2.0,  6.0}, { 0.0,  8.0}, {-1.0, -1.0}, {-1.0, -1.0}},
			{{ 1.0,  0.0}, { 9.0,  0.0}, { 7.0,  2.0}, { 3.0,  2.0}, {-1.0, -1.0}, {-1.0, -1.0}},
			{{ 8.0,  2.0}, {10.0,  0.0}, {10.0,  8.0}, { 8.0,  6.0}, {-1.0, -1.0}, {-1.0, -1.0}},
			{{ 8.0, 11.0}, {10.0,  9.0}, {10.0, 17.0}, { 8.0, 15.0}, {-1.0, -1.0}, {-1.0, -1.0}},
			{{ 0.0, 18.0}, { 2.0, 16.0}, { 8.0, 16.0}, {10.0, 18.0}, {-1.0, -1.0}, {-1.0, -1.0}},
			{{ 0.0,  9.0}, { 2.0, 11.0}, { 2.0, 15.0}, { 0.0, 17.0}, {-1.0, -1.0}, {-1.0, -1.0}},
			{{ 2.0,  8.0}, { 8.0,  8.0}, { 8.0, 10.0}, { 2.0, 10.0}, {-1.0, -1.0}, {-1.0, -1.0}}};

		// width 3
		const double pts_fix_3[count_s][count_p][2] = {
			{{ 0.0,  2.0}, { 2.0,  2.0}, { 3.0,  3.0}, { 3.0, 10.0}, { 1.0, 12.0}, { 0.0, 12.0}},
			{{ 2.0,  1.0}, { 2.0,  0.0}, {13.0,  0.0}, {13.0,  1.0}, {11.0,  3.0}, { 3.0,  3.0}},
			{{12.0,  3.0}, {13.0,  2.0}, {15.0,  2.0}, {15.0, 12.0}, {13.0, 12.0}, {12.0, 10.0}},
			{{12.0, 16.0}, {13.0, 15.0}, {15.0, 15.0}, {15.0, 25.0}, {13.0, 25.0}, {12.0, 23.0}},
			{{ 2.0, 27.0}, { 2.0, 25.0}, { 3.0, 24.0}, {12.0, 24.0}, {13.0, 25.0}, {13.0, 27.0}},
			{{ 0.0, 15.0}, { 2.0, 15.0}, { 3.0, 16.0}, { 3.0, 23.0}, { 1.0, 25.0}, { 0.0, 25.0}},
			{{ 1.0, 13.0}, { 2.0, 12.0}, {13.0, 12.0}, {14.0, 13.0}, {12.0, 15.0}, { 2.0, 15.0}}};

		// width 5
		const double pts_fix_5[count_s][count_p][2] = {
			{{ 0.0,  3.0}, { 1.0,  1.0}, { 5.0,  5.0}, { 5.0, 18.0}, { 1.0, 22.0}, { 0.0, 20.0}},
			{{ 2.0,  1.0}, { 3.0,  0.0}, {22.0,  0.0}, {23.0,  1.0}, {19.0,  5.0}, { 5.0,  5.0}},
			{{20.0,  5.0}, {23.0,  1.0}, {25.0,  3.0}, {25.0, 20.0}, {23.0, 22.0}, {20.0, 18.0}},
			{{20.0, 26.0}, {23.0, 22.0}, {25.0, 24.0}, {25.0, 41.0}, {23.0, 43.0}, {20.0, 39.0}},
			{{ 2.0, 43.0}, { 5.0, 40.0}, {20.0, 40.0}, {23.0, 43.0}, {21.0, 45.0}, { 4.0, 45.0}},
			{{ 0.0, 24.0}, { 1.0, 22.0}, { 5.0, 26.0}, { 5.0, 39.0}, { 1.0, 43.0}, { 0.0, 41.0}},
			{{ 2.0, 22.0}, { 4.0, 20.0}, {21.0, 20.0}, {23.0, 22.0}, {20.0, 25.0}, { 4.0, 25.0}}};

		// width 7
		const double pts_fix_7[count_s][count_p][2] = {
			{{ 0.0,  6.0}, { 2.0,  3.0}, { 7.0,  8.0}, { 7.0, 25.0}, { 2.0, 30.0}, { 0.0, 27.0}},
			{{ 4.0,  2.0}, { 6.0,  0.0}, {29.0,  0.0}, {31.0,  2.0}, {26.0,  7.0}, { 8.0,  7.0}},
			{{28.0,  8.0}, {32.0,  3.0}, {35.0,  6.0}, {35.0, 27.0}, {32.0, 30.0}, {28.0, 25.0}},
			{{28.0, 37.0}, {32.0, 32.0}, {35.0, 35.0}, {35.0, 56.0}, {32.0, 59.0}, {28.0, 54.0}},
			{{ 4.0, 60.0}, { 8.0, 56.0}, {27.0, 56.0}, {31.0, 60.0}, {28.0, 63.0}, { 6.0, 63.0}},
			{{ 0.0, 35.0}, { 2.0, 32.0}, { 7.0, 37.0}, { 7.0, 54.0}, { 2.0, 59.0}, { 0.0, 56.0}},
			{{ 4.0, 31.0}, { 7.0, 28.0}, {28.0, 28.0}, {31.0, 31.0}, {27.0, 35.0}, { 7.0, 35.0}}};


		const int count_d = 5;
		// width 1
		const double pts_1[count_s][count_p][2] = {
			{{0.0,  4.0}, {0.0,  1.0}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}},
			{{1.0,  0.0}, {4.0,  0.0}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}},
			{{5.0,  1.0}, {5.0,  4.0}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}},
			{{5.0,  6.0}, {5.0,  9.0}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}},
			{{4.0, 10.0}, {1.0, 10.0}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}},
			{{0.0,  9.0}, {0.0,  6.0}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}},
			{{1.0,  5.0}, {4.0,  5.0}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}}};

		// width 2
		const double pts_2[count_s][count_p][2] = {
			{{ 0.0,  2.5}, { 1.0,  1.5}, { 2.0,  2.5}, { 2.0,  7.5}, { 1.0,  8.5}, { 0.0,  7.5}},
			{{ 1.5,  1.0}, { 2.5,  0.0}, { 7.5,  0.0}, { 8.5,  1.0}, { 7.5,  2.0}, { 2.5,  2.0}},
			{{ 8.5,  2.5}, { 9.0,  1.5}, {10.0,  2.5}, {10.0,  7.5}, { 9.0,  8.5}, { 8.0,  7.5}},
			{{ 8.0, 10.5}, { 9.0,  9.5}, {10.0, 10.5}, {10.0, 15.5}, { 9.0, 16.5}, { 8.0, 15.5}},
			{{ 1.5, 17.0}, { 2.5, 16.0}, { 7.5, 16.0}, { 8.5, 17.0}, { 7.5, 18.0}, { 2.5, 18.0}},
			{{ 0.0, 10.5}, { 1.0,  9.5}, { 2.0, 10.5}, { 2.0, 15.5}, { 1.0, 16.5}, { 0.0, 15.5}},
			{{ 1.5,  9.0}, { 2.5,  8.0}, { 7.5,  8.0}, { 8.5,  9.0}, { 7.5, 10.0}, { 2.5, 10.0}}};

		// width 3
		const double pts_3[count_s][count_p][2] = {
			{{ 0.0,  3.0}, { 1.0,  2.0}, { 3.0,  4.0}, { 3.0, 11.5}, { 1.0, 13.0}, { 0.0, 12.0}},
			{{ 2.0,  1.0}, { 3.0,  0.0}, {12.0,  0.0}, {13.0,  1.0}, {11.0,  3.0}, { 4.0,  3.0}},
			{{12.0,  4.0}, {14.0,  2.0}, {15.0,  3.0}, {15.0, 12.0}, {14.0, 13.0}, {12.0, 11.5}},
			{{12.0, 15.5}, {14.0, 14.0}, {15.0, 15.0}, {15.0, 24.0}, {14.0, 25.0}, {12.0, 23.0}},
			{{ 2.0, 26.0}, { 4.0, 24.0}, {11.0, 24.0}, {13.0, 26.0}, {12.0, 27.0}, { 3.0, 27.0}},
			{{ 0.0, 15.0}, { 1.0, 14.0}, { 3.0, 15.5}, { 3.0, 23.0}, { 1.0, 25.0}, { 0.0, 24.0}},
			{{ 2.0, 13.5}, { 4.0, 12.0}, {11.0, 12.0}, {13.0, 13.5}, {11.0, 15.0}, { 4.0, 15.0}}};

		// width 5
		const double pts_5[count_s][count_p][2] = {
			{{ 0.0,  3.0}, { 1.0,  2.0}, { 5.0,  6.0}, { 5.0, 18.0}, { 1.0, 21.5}, { 0.0, 20.5}},
			{{ 2.0,  1.0}, { 3.0,  0.0}, {22.0,  0.0}, {23.0,  1.0}, {19.0,  5.0}, { 6.0,  5.0}},
			{{20.0,  6.0}, {24.0,  2.0}, {25.0,  3.0}, {25.0, 20.5}, {24.0, 21.5}, {20.0, 18.0}},
			{{20.0, 27.0}, {24.0, 23.5}, {25.0, 24.5}, {25.0, 42.0}, {24.0, 43.0}, {20.0, 39.0}},
			{{ 2.0, 44.0}, { 6.0, 40.0}, {19.0, 40.0}, {23.0, 44.0}, {22.0, 45.0}, { 3.0, 45.0}},
			{{ 0.0, 24.5}, { 1.0, 23.5}, { 5.0, 27.0}, { 5.0, 39.0}, { 1.0, 43.0}, { 0.0, 42.0}},
			{{ 2.0, 22.5}, { 5.0, 20.0}, {20.0, 20.0}, {23.0, 22.5}, {20.0, 25.0}, { 5.0, 25.0}}};

		// width 7
		const double pts_7[count_s][count_p][2] = {
			{{ 0.0,  5.0}, { 2.0,  3.0}, { 7.0,  8.0}, { 7.0, 26.5}, { 2.0, 31.0}, { 0.0, 29.0}},
			{{ 3.0,  2.0}, { 5.0,  0.0}, {30.0,  0.0}, {32.0,  2.0}, {27.0,  7.0}, { 8.0,  7.0}},
			{{28.0,  8.0}, {33.0,  3.0}, {35.0,  5.0}, {35.0, 29.0}, {33.0, 31.0}, {28.0, 26.5}},
			{{28.0, 36.5}, {33.0, 32.0}, {35.0, 34.0}, {35.0, 58.0}, {33.0, 60.0}, {28.0, 55.0}},
			{{ 3.0, 61.0}, { 8.0, 56.0}, {27.0, 56.0}, {32.0, 61.0}, {30.0, 63.0}, { 5.0, 63.0}},
			{{ 0.0, 34.0}, { 2.0, 32.0}, { 7.0, 36.5}, { 7.0, 55.0}, { 2.0, 60.0}, { 0.0, 58.0}},
			{{ 3.5, 31.5}, { 7.5, 28.0}, {27.5, 28.0}, {31.5, 31.5}, {27.5, 35.0}, { 7.5, 35.0}}};

		const double* pts_fix[count_fix_d] = {(const double*)pts_fix_1, (const double*)pts_fix_2, (const double*)pts_fix_3, (const double*)pts_fix_5, (const double*)pts_fix_7};
		const double* pts[count_d] = {(const double*)pts_1, (const double*)pts_2, (const double*)pts_3, (const double*)pts_5, (const double*)pts_7};

		m_Digit = count_d - 1;

		int k = 0;

		m_DigitsFix.SetSize(count_fix_d, 0);
		for (k = 0; k < count_fix_d; k++)
		{
			CArray<XSegment, XSegment>& segments = m_DigitsFix[k].m_Segments;
			segments.SetSize(count_s, 0);

			const double* p = pts_fix[k];

			for (int i = 0; i < count_s; i++)
			{
				CBCGPPointsArray& points = segments[i].m_Points;

				for (int j = 0; j < count_p; j++)
				{
					if (p[0] != -1 && p[1] != -1)
					{
						points.Add(CBCGPPoint (p[0], p[1]));
					}

					p += 2;
				}
			}
		}

 		m_DigitsFix[0].m_Dot = CBCGPRect( 7,  9,  8, 10);
 		m_DigitsFix[1].m_Dot = CBCGPRect(11, 16, 13, 18);
 		m_DigitsFix[2].m_Dot = CBCGPRect(17, 24, 20, 27);
 		m_DigitsFix[3].m_Dot = CBCGPRect(28, 40, 33, 45);
 		m_DigitsFix[4].m_Dot = CBCGPRect(39, 56, 46, 63);

		m_Digits.SetSize(count_d, 0);
		for (k = 0; k < count_d; k++)
		{
			CArray<XSegment, XSegment>& segments = m_Digits[k].m_Segments;
			segments.SetSize(count_s, 0);

			const double* p = pts[k];

			for (int i = 0; i < count_s; i++)
			{
				CBCGPPointsArray& points = segments[i].m_Points;

				for (int j = 0; j < count_p; j++)
				{
					if (p[0] != -1 && p[1] != -1)
					{
						points.Add(CBCGPPoint (p[0], p[1]));
					}

					p += 2;
				}
			}
		}

 		m_Digits[0].m_Dot = CBCGPRect( 8,  9,  9, 10);
 		m_Digits[1].m_Dot = CBCGPRect(11, 16, 13, 18);
		m_Digits[2].m_Dot = CBCGPRect(17, 24, 20, 27);
 		m_Digits[3].m_Dot = CBCGPRect(28, 44, 32, 48);
 		m_Digits[4].m_Dot = CBCGPRect(43, 68, 49, 74);

		Initialize(skewAngle);
	}
};


CBCGPNumericIndicatorColors::CBCGPNumericIndicatorColors(int nTheme)
{
	switch (nTheme)
	{
	case 0:
	default:
		m_brOutline = CBCGPBrush(CBCGPColor::DarkGray);
		m_brFill = CBCGPBrush(CBCGPColor::White, CBCGPColor::LightGray, CBCGPBrush::BCGP_GRADIENT_CENTER_HORIZONTAL);
		m_brSeparator = CBCGPBrush(CBCGPColor::Silver);
		m_brDigit = CBCGPBrush(CBCGPColor::SteelBlue);
		m_brDecimal = CBCGPBrush(CBCGPColor::DarkRed);
		m_brSign = CBCGPBrush(CBCGPColor::SteelBlue);
		m_brDot = CBCGPBrush(CBCGPColor::SteelBlue);
		break;

	case 1:
		m_brOutline = CBCGPBrush(CBCGPColor::DarkGray);
		m_brFill = CBCGPBrush(CBCGPColor(CBCGPColor::DarkKhaki, .8), CBCGPColor::PaleGoldenrod, CBCGPBrush::BCGP_GRADIENT_DIAGONAL_LEFT);
		m_brSeparator = m_brDot = m_brSign = m_brDecimal = m_brDigit = CBCGPBrush(CBCGPColor::DarkSlateGray);
		break;
	}
}

IMPLEMENT_DYNCREATE(CBCGPNumericIndicatorImpl, CBCGPGaugeImpl)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGPNumericIndicatorImpl::CBCGPNumericIndicatorImpl(BCGPNumericIndicatorStyle style, CBCGPVisualContainer* pContainer)
	: CBCGPGaugeImpl(pContainer)
	, m_Digit       (NULL)
{
	m_nCells            = 7;
	m_nDecimals         = 1;
	m_nSeparatorWidth   = 1;
	m_bDrawSign         = FALSE;
	m_bDrawDecimalPoint = FALSE;
	m_bDrawLeadingZeros = TRUE;
	m_Style             = BCGP_NUMERIC_INDICATOR_CLASSIC;
	m_nFrameSize        = 1;

	AddData(new CBCGPGaugeDataObject);

	SetStyle(style);

	CreateResources();
}

CBCGPNumericIndicatorImpl::~CBCGPNumericIndicatorImpl()
{
	if (m_Digit != NULL)
	{
		delete m_Digit;
	}
}

void CBCGPNumericIndicatorImpl::CreateResources()
{
	LOGFONT lf;
	globalData.fontBold.GetLogFont(&lf);

	m_textFormat.CreateFromLogFont(lf);

	m_textFormat.SetTextAlignment(CBCGPTextFormat::BCGP_TEXT_ALIGNMENT_CENTER);
	m_textFormat.SetTextVerticalAlignment(CBCGPTextFormat::BCGP_TEXT_ALIGNMENT_CENTER);

	if (m_Digit != NULL)
	{
		delete m_Digit;
		m_Digit = NULL;
	}

	m_Digit = new CBCGPDigit8;
	m_Digit->Create (0.0);
}

void CBCGPNumericIndicatorImpl::OnDraw(CBCGPGraphicsManager* pGM, const CBCGPRect& rectClip, DWORD dwFlags)
{
	ASSERT_VALID(pGM);

	if (m_rect.IsRectEmpty() || !m_bIsVisible)
	{
		return;
	}

	if ((dwFlags & BCGP_DRAW_DYNAMIC) == 0)
	{
		return;
	}

	CBCGPRect rect = m_rect;
	if (m_bIsSubGauge)
	{
		rect.OffsetRect(-m_ptScrollOffset);
	}

	pGM->FillRectangle(rect, m_Colors.m_brFill);

	double dblValue = m_arData[0]->GetValue();
	if (!m_bDrawSign)
	{
		dblValue = fabs(dblValue);
	}

	CString strLabel;

	if (m_nDecimals == 0)
	{
		int nDigits = m_nCells - (m_bDrawSign ? 1 : 0);
		CString strFormat;
		strFormat.Format(_T("%%%s%dd"), m_bDrawLeadingZeros ? _T("0") : _T(""), nDigits);

		strLabel.Format(strFormat, (int)dblValue);
	}
	else
	{
		int nDigits = m_nCells - (m_bDrawSign ? 1 : 0) - (((m_Style != BCGP_NUMERIC_INDICATOR_DIGITAL) && m_bDrawDecimalPoint) ? 1 : 0) + 1;
		CString strFormat;
		strFormat.Format(_T("%%%s%d.%df"), m_bDrawLeadingZeros ? _T("0") : _T(""), nDigits, m_nDecimals);

		strLabel.Format(strFormat, dblValue);
	}

	BOOL bDot = m_nDecimals > 0 && m_bDrawDecimalPoint;
	if (!bDot)
	{
		strLabel.Remove(_T('.'));
		strLabel.Remove(_T(','));
	}

	strLabel.TrimLeft();
	strLabel.TrimRight();

	CBCGPSize sizeMargin(m_sizeMargin.cx * m_sizeScaleRatio.cx, m_sizeMargin.cy * m_sizeScaleRatio.cy);
	double nSeparatorWidth = m_nSeparatorWidth * m_sizeScaleRatio.cx;

	double x = rect.right - sizeMargin.cx;
	double dx = (rect.Width() - 2 * sizeMargin.cx) / m_nCells;

	CBCGPRect rectCell(x - dx, rect.top, x, rect.bottom);
	CBCGPRect rectSeparator;

	if (nSeparatorWidth > 0)
	{
		rectSeparator = CBCGPRect(
			CBCGPPoint (rectCell.left - .5 * nSeparatorWidth, rect.top + sizeMargin.cy), 
			CBCGPSize((double)nSeparatorWidth, rect.Height() - 2 * sizeMargin.cy));
	}

	rectCell.left += nSeparatorWidth;

	if (m_Style == BCGP_NUMERIC_INDICATOR_DIGITAL && m_Digit != NULL)
	{
		rectCell.DeflateRect(sizeMargin.cx, sizeMargin.cy);
		rectCell = m_Digit->CreateDigit (*pGM, rectCell, bDot);
	}

	BOOL bIsError = (strLabel.GetLength () - (((m_Style == BCGP_NUMERIC_INDICATOR_DIGITAL) && bDot) ? 1 : 0)) > m_nCells;
	if (bIsError)
	{
		strLabel.Empty ();

		if (m_nCells >= 5)
		{
			strLabel = _T("Error");
		}
		else
		{
			LPTSTR lpLabel = strLabel.GetBuffer (m_nCells + 1);

			for (int i = 0; i < m_nCells; i++)
			{
				*lpLabel++ = TCHAR('-');
			}
			*lpLabel++ = 0;

			strLabel.ReleaseBuffer ();
		}
	}

	int nIndex = strLabel.GetLength () - 1;
	for (int i = 0; i < m_nCells; i++)
	{
		BCGPNumericIndicatorCellType typeCell = i < m_nDecimals
			? BCGP_NUMERIC_INDICATOR_CELL_DECIMAL : BCGP_NUMERIC_INDICATOR_CELL_DIGIT;
		
		TCHAR ch = 0;

		if (nIndex >= 0)
		{
			ch = strLabel[nIndex];
			nIndex--;
		}

		BOOL bSeparator = m_nSeparatorWidth > 0 && i < (m_nCells - 1);

		if (ch != 0)
		{
			if (bIsError)
			{
				typeCell = BCGP_NUMERIC_INDICATOR_CELL_ERROR;
			}
			else
			{
				switch (ch)
				{
				case TCHAR('-'):
				case TCHAR('+'):
					typeCell = BCGP_NUMERIC_INDICATOR_CELL_SIGN;
					break;

				case TCHAR('.'):
				case TCHAR(','):
					typeCell = BCGP_NUMERIC_INDICATOR_CELL_DOT;
				}
			}

			CBCGPRect rectInter;
			if (rectInter.IntersectRect (rectCell, rectClip))
			{
				OnDrawChar(pGM, m_textFormat, rectCell, ch, i, typeCell);
			}

			if (m_Style == BCGP_NUMERIC_INDICATOR_DIGITAL && typeCell == BCGP_NUMERIC_INDICATOR_CELL_DOT)
			{
				rectCell.OffsetRect (dx, 0.0);
				if (bSeparator)
				{
					rectSeparator.OffsetRect(dx, 0);
				}

				i--;
			}
		}

		if (bSeparator)
		{
			CBCGPRect rectInter;
			if (rectInter.IntersectRect (rectSeparator, rectClip))
			{
				OnDrawSeparator(pGM, rectSeparator);
			}

			rectSeparator.OffsetRect(-dx, 0);
		}

		rectCell.OffsetRect (-dx, 0.0);
	}

	OnDrawFrame(pGM, rect);
	SetDirty(FALSE);
}
//*******************************************************************************
void CBCGPNumericIndicatorImpl::OnDrawSeparator(CBCGPGraphicsManager* pGM, CBCGPRect rectSeparator)
{
	ASSERT_VALID(pGM);
	pGM->FillRectangle(rectSeparator, m_Colors.m_brSeparator);
}
//*******************************************************************************
void CBCGPNumericIndicatorImpl::OnDrawFrame(CBCGPGraphicsManager* pGM, CBCGPRect rectFrame)
{
	ASSERT_VALID(pGM);
	pGM->DrawRectangle(rectFrame, m_Colors.m_brOutline, GetScaleRatioMid());
}
//*******************************************************************************
void CBCGPNumericIndicatorImpl::OnDrawChar(
	CBCGPGraphicsManager* pGM, const CBCGPTextFormat& tf, CBCGPRect rect, TCHAR ch, 
	int /*nCellIndex*/, BCGPNumericIndicatorCellType type)
{
	const CBCGPBrush& br = type == BCGP_NUMERIC_INDICATOR_CELL_DIGIT ? m_Colors.m_brDigit :
			type == BCGP_NUMERIC_INDICATOR_CELL_DECIMAL ? m_Colors.m_brDecimal :
			type == BCGP_NUMERIC_INDICATOR_CELL_SIGN ? m_Colors.m_brSign :
			type == BCGP_NUMERIC_INDICATOR_CELL_DOT ? m_Colors.m_brDot :
			m_Colors.m_brDigit;

	if (m_Style == BCGP_NUMERIC_INDICATOR_CLASSIC)
	{
		pGM->DrawText(CString (ch), rect, tf, br);
		return;
	}

	if (m_Digit == NULL)
	{
		return;
	}

	if (type == BCGP_NUMERIC_INDICATOR_CELL_DOT)
	{
		m_Digit->DrawDot (*pGM, br, rect);
	}
	else
	{
		m_Digit->DrawDigit (*pGM, br, ch, rect);
	}
}
//*******************************************************************************
void CBCGPNumericIndicatorImpl::SetTextFormat(const CBCGPTextFormat& textFormat)
{
	m_textFormat.Destroy();
	m_textFormat.CopyFrom(textFormat);
	SetDirty();
}
//*******************************************************************************
void CBCGPNumericIndicatorImpl::SetStyle(BCGPNumericIndicatorStyle style)
{
	if (m_Style != style)
	{
		m_Style = style;
		SetColors(CBCGPNumericIndicatorColors(m_Style == BCGP_NUMERIC_INDICATOR_CLASSIC ? 0 : 1));

		switch (style)
		{
		case BCGP_NUMERIC_INDICATOR_CLASSIC:
		default:
			m_sizeMargin = CBCGPSize(2., 2.);
			break;

		case BCGP_NUMERIC_INDICATOR_DIGITAL:
			m_sizeMargin = CBCGPSize(1., 2.);
			break;
		}

		SetDirty();
	}
}
//*******************************************************************************
BOOL CBCGPNumericIndicatorImpl::SetValue(double dblVal, BOOL bRedraw)
{
	CBCGPGaugeDataObject* pData = DYNAMIC_DOWNCAST(CBCGPGaugeDataObject, m_arData[0]);
	ASSERT_VALID(pData);

	if (pData->GetValue() == dblVal)
	{
		return TRUE;
	}

	pData->SetValue(dblVal);

	if (bRedraw)
	{
		Redraw();
	}

	return TRUE;
}
//*******************************************************************************
CBCGPSize CBCGPNumericIndicatorImpl::GetDefaultSize(CBCGPGraphicsManager* pGM, const CBCGPBaseVisualObject* /*pParentGauge*/)
{
	ASSERT_VALID(pGM);

	CBCGPSize size(0, 0);

	CBCGPSize sizeMargin(m_sizeMargin.cx * m_sizeScaleRatio.cx, m_sizeMargin.cy * m_sizeScaleRatio.cy);
	double nSeparatorWidth = m_nSeparatorWidth * m_sizeScaleRatio.cx;

	if (m_Style == BCGP_NUMERIC_INDICATOR_CLASSIC)
	{
		size = pGM->GetTextSize(_T("W"), m_textFormat);
	}
	else if (m_Digit != NULL)
	{
		size = m_Digit->GetDefaultSize(*pGM, m_nDecimals > 0 && m_bDrawDecimalPoint);
		size.cx += 2 * sizeMargin.cx;
	}

	if (size.cx > 0.0 && nSeparatorWidth > 0)
	{
		size.cx = (size.cx + nSeparatorWidth) * m_nCells - nSeparatorWidth;
	}

	size.cx += 2 * sizeMargin.cx;
	size.cy += 2 * sizeMargin.cy;

	return size;
}
//*******************************************************************************
void CBCGPNumericIndicatorImpl::OnScaleRatioChanged(const CBCGPSize& sizeScaleRatioOld)
{
	CBCGPGaugeImpl::OnScaleRatioChanged(sizeScaleRatioOld);

	m_textFormat.Scale(GetScaleRatioMid());
	SetDirty();
}
//*******************************************************************************
void CBCGPNumericIndicatorImpl::CopyFrom(const CBCGPBaseVisualObject& srcObj)
{
	CBCGPGaugeImpl::CopyFrom(srcObj);

	const CBCGPNumericIndicatorImpl& src = (const CBCGPNumericIndicatorImpl&)srcObj;

	m_nCells = src.m_nCells;
	m_nDecimals = src.m_nDecimals;
	m_nSeparatorWidth = src.m_nSeparatorWidth;
	m_bDrawSign = src.m_bDrawSign;
	m_bDrawDecimalPoint = src.m_bDrawDecimalPoint;
	m_bDrawLeadingZeros = src.m_bDrawLeadingZeros;
	m_Colors.CopyFrom(src.m_Colors);
	m_Style = src.m_Style;
	m_textFormat = src.m_textFormat;
	m_sizeMargin = src.m_sizeMargin;
}
