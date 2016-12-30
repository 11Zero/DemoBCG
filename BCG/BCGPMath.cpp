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

#include "stdafx.h"
#include "BCGPMath.h"
#include <float.h>

void PrintVector(const CBCGPVector4& v)
{
	for (int i = 0; i < 4; i++)
	{
		CString str;
		str.Format(_T("%.18f"), v[i]);
		TRACE1("%s", str);

		if (i < 3)
		{
			TRACE0(", ");
		}
	}

	TRACE0("\n");
}

void PrintPoint3D(const CBCGPPoint& pt)
{
	CBCGPVector4 v;
	v.FromPoint3D(pt);
	PrintVector(v);
}

double bcg_normalize_rad(double value)
{
	const double _PI2 = M_PI * 2.0;

    if(fabs(value) >= _PI2)
    {
        value -= ((long)(value / _PI2)) * _PI2;
    }

    if(value < 0.0)
    {
        value += _PI2;
    }

    return value;
}

double bcg_normalize_deg(double value)
{
	if(fabs(value) >= 360.0)
    {
        value -= ((long)(value / 360)) * 360;
    }

    if(value < 0.0)
    {
        value += 360.0;
    }

    return value;
}

int bcg_pointInLine (const CBCGPPoint& pt1, const CBCGPPoint& pt2, const CBCGPPoint& pt, double precision)
{
    double ptx  = pt.x;
    double pty  = pt.y;
    double pt1x = pt1.x;
    double pt1y = pt1.y;
    double pt2x = pt2.x;
    double pt2y = pt2.y;

    double ptDx = pt2x - pt1x;
    double ptDy = pt2y - pt1y;

    if(ptDx != 0 || ptDy != 0)
    {
        double ptPx = ptx - pt1x;
        double ptPy = pty - pt1y;

        if((fabs(ptDx) <= (precision * 2.0) && fabs(ptPx) <= precision && ((pt1y <= pty && pty <= pt2y) || (pt2y <= pty && pty <= pt1y))) ||
            (fabs(ptDy) <= (precision * 2.0) && fabs(ptPy) <= precision && ((pt1x <= ptx && ptx <= pt2x) || (pt2x <= ptx && ptx <= pt1x))) ||
            (ptDx != 0.0 && fabs((ptDy * ptPx / ptDx) - ptPy) <= precision))
        {
            // on open ray _pt1
            if(
                ((pt2x < pt1x && pt1x < ptx) || (pt2y < pt1y && pt1y < pty)) ||
                ((ptx < pt1x && pt1x < pt2x) || (pty < pt1y && pt1y < pt2y))
              )
            {
                return 2;
            }

            // on open ray _pt2
            if(
                ((pt1x < pt2x && pt2x < ptx) || (pt1y < pt2y && pt2y < pty)) ||
                ((ptx < pt2x && pt2x < pt1x) || (pty < pt2y && pt2y < pt1y))
              )
            {
                return 3;
            }

            return 1;
        }
    }
    else
    {
        if ((bcg_sqr(pt1x - ptx) + bcg_sqr(pt1y - pty)) <= (precision * precision))
        {
            return 1;
        }
    }

    return 0;
}

bool bcg_linesIntersection (const CBCGPPoint& pt1, const CBCGPPoint& pt2,
                            const CBCGPPoint& pt3, const CBCGPPoint& pt4,
                            double& outX, double& outY)
{
    double x1 = pt1.x, y1 = pt1.y, x2 = pt2.x, y2 = pt2.y;
    double x3 = pt3.x, y3 = pt3.y, x4 = pt4.x, y4 = pt4.y;

    if (x2 != x3 || y2 != y3)
    {
        double dx1 = x2 - x1;
        double dy1 = y2 - y1;
        double dx2 = x4 - x3;
        double dy2 = y4 - y3;
        double div = dx1 * dy2 - dx2 * dy1;

        if (div == 0)
        {
            if (!((dx1 == 0 && dy1 == 0) || (dx2 == 0 && dy2 == 0)))
            {
                if (dy1 == 0 && dy2 != 0)
                {
                    double L = (y1 - y3) / dy2;
                    outX = x3 + L * dx2;
                    outY = y1;
                    return L >= 0 && L <= 1;
                }
                else if (dy2 == 0 && dy1 != 0)
                {
                    double L = (y3 - y1) / dy1;
                    outX = x1 + L * dx1;
                    outY = y3;
                    return L >= 0 && L <= 1;
                }
                else if (dx1 == 0 && dx2 != 0)
                {
                    double L = (x1 - x3) / dx2;
                    outX = x1;
                    outY = y3 + L * dy2;
                    return L >= 0 && L <= 1;
                }
                else if (dx2 == 0 && dx1 != 0)
                {
                    double L = (x3 - x1) / dx1;
                    outX = x3;
                    outY = y1 + L * dy1;
                    return L >= 0 && L <= 1;
                }
            }

            outX = (x2 + x3) / 2;
            outY = (y2 + y3) / 2;
            return false;
        }

        double L1 = ((y1 - y3) * dx2 - (x1 - x3) * dy2) / div;
        outX = x1 + L1 * dx1;
        outY = y1 + L1 * dy1;

        if (L1 < 0 || L1 > 1)
        {
            return false;
        }

        double L2 = ((y1 - y3) * dx1 - (x1 - x3) * dy1) / div;
        return L2 >= 0 && L2 <= 1;
    }

    outX = x2;
    outY = y2;

    return true;
}

bool bcg_pointInPolygon (CBCGPPoint* pts, int nPoints, const CBCGPPoint& pt)
{
    if (nPoints < 3)
    {
        return false;
    }

    bool bResult = true;
    for (int i = 0, j = nPoints - 1; i < nPoints; j = i++)
    {
        if ((((pts[i].y <= pt.y) && (pt.y < pts[j].y)) ||
            ((pts[j].y <= pt.y) && (pt.y < pts[i].y))) &&
            ((pt.x - pts[i].x) * (pts[j].y - pts[i].y) < (pts[j].x - pts[i].x) * (pt.y - pts[i].y)))
        {
            bResult = !bResult;
        }
    }

    return bResult;
}

bool bcg_pointInPie (const CBCGPRect& rect, double dblAngleStart, double dblAngleFinish, const CBCGPPoint& ptTestIn, double dblDoughnutPercent)
{
    if (!rect.PtInRect(ptTestIn))
    {
        return false;
    }

	CBCGPPoint ptTest = ptTestIn;
	CBCGPPoint ptCenter = rect.CenterPoint();
	double dblRadiusX = 0.5 * rect.Width();
	double dblRadiusY = 0.5 * rect.Height();

	if (dblRadiusX > dblRadiusY && dblRadiusX != 0.)
	{
		ptTest.Scale(CBCGPPoint(dblRadiusY / dblRadiusX, 1.0), ptCenter);
	}
	else if (dblRadiusY > dblRadiusX && dblRadiusY != 0.)
	{
		ptTest.Scale(CBCGPPoint(1.0, dblRadiusX / dblRadiusY, 1.0), ptCenter);
	}
	
	double dblAngle = bcg_normalize_rad(bcg_angle(ptCenter, ptTest));

	dblAngleStart = bcg_normalize_rad(dblAngleStart);
	dblAngleFinish = bcg_normalize_rad(dblAngleFinish);

	BOOL bIn = FALSE;
	const BOOL bIsFullEllipse = bcg_IsFullEllipse(bcg_rad2deg(dblAngleStart), bcg_rad2deg(dblAngleFinish), TRUE, 0.1f);

	if (bIsFullEllipse)
	{
		bIn = TRUE;
	}
	else
	{
		if (dblAngleStart > dblAngleFinish)
		{
			bIn = (dblAngle <= dblAngleFinish) || (dblAngleStart <= dblAngle);
		}
		else
		{
			bIn = (dblAngle >= dblAngleStart) && (dblAngle <= dblAngleFinish);
		}
	}

	if (bIn)
	{
		double angleCos = cos(dblAngle);
		double angleSin = sin(dblAngle);

		CBCGPPoint ptEdge(ptCenter.x + angleCos * .5 * rect.Width(), ptCenter.y + angleSin * .5 * rect.Height());
		double r = bcg_distance(ptEdge, ptCenter);
		double distToCenter = bcg_distance(ptTestIn, ptCenter);

		return (distToCenter <= r && distToCenter > r * dblDoughnutPercent);
	}

	return false;
}

bool bcg_clockwise (const CBCGPPoint& pt1, const CBCGPPoint& pt2, const CBCGPPoint& pt3)
{
    return (pt2.x * pt3.y - pt2.y * pt3.x - pt1.x * pt3.y + pt1.y * pt3.x + pt1.x * pt2.y - pt1.y * pt2.x) > 0;
}

void bcg_setLineLength (const CBCGPPoint& pt1, CBCGPPoint& pt2, double dNewLength)
{
    double dLen = bcg_distance (pt1, pt2);
    double cosT = (dLen > DBL_EPSILON) ? bcg_clamp ((pt2.x - pt1.x) / dLen, -1.0, 1.0) : 1.0;
    double sinT = (dLen > DBL_EPSILON) ? bcg_clamp ((pt2.y - pt1.y) / dLen, -1.0, 1.0) : 0.0;

    pt2.x = pt1.x + dNewLength * cosT;
    pt2.y = pt1.y + dNewLength * sinT;
}

static int bcg_CS_code(const CBCGPRect& rect, const CBCGPPoint& point)
{
	int i = 0;

	if (point.x < rect.left)
	{
		i |= 0x01;
	}
	else if (point.x > rect.right)
	{
		i |= 0x02;
	}

	if (point.y < rect.top)
	{
		i |= 0x04;
	}
	else if (point.y > rect.bottom)
	{
		i |= 0x08;
	}

	return i;
}

BOOL bcg_CS_clip(const CBCGPRect& rect, CBCGPPoint& point1, CBCGPPoint& point2)
{
	if (rect.IsRectEmpty ())
	{
		return TRUE;
	}

	if (point1 == point2)
	{
		return rect.PtInRect(point1);
	}

	int code1 = bcg_CS_code(rect, point1);
	int code2 = bcg_CS_code(rect, point2);

	double kx = 0.0;
	double ky = 0.0;
	CBCGPSize d(point2.x - point1.x, point2.y - point1.y);
	if (d.cx != 0.0)
	{
		ky = d.cy / d.cx;
	}
	else if (d.cy == 0.0)
	{
		if (code1 == 0 && code2 == 0)
		{
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}

	if (d.cy != 0.0)
	{
		kx = d.cx / d.cy;
	}

	BOOL visible = FALSE;
	CBCGPPoint pt1(point1);
	CBCGPPoint pt2(point2);

	int count_inv = 0;

	for(int i = 0; i < 4; i++)
	{
		if (code1 & code2)
		{
			break;
		}
		else if (code1 == 0 && code2 == 0)
		{
			visible = TRUE;
			break;
		}

		if (code1 == 0)
		{
			int c = code1;
			code1 = code2;
			code2 = c;

			CBCGPPoint p(pt1);
			pt1 = pt2;
			pt2 = p;

			count_inv++;
		}

		if (code1 & 0x01)
		{
			pt1.y += ky * (rect.left - pt1.x);
			pt1.x = rect.left;
		}
		else if (code1 & 0x02)
		{
			pt1.y += ky * (rect.right - pt1.x);
			pt1.x = rect.right;
		}
		else if (code1 & 0x04)
		{
			pt1.x += kx * (rect.top - pt1.y);
			pt1.y = rect.top;
		}
		else if (code1 & 0x08)
		{
			pt1.x += kx * (rect.bottom - pt1.y);
			pt1.y = rect.bottom;
		}

		code1 = bcg_CS_code(rect, pt1);
	}

	if (visible)
	{
		if ((count_inv % 2) != 0)
		{
			point1 = pt1;
			point2 = pt2;
		}
		else
		{
			point1 = pt2;
			point2 = pt1;
		}
	}

	return visible;
}

int bcg_CS_clip_inv(const CBCGPRect& rect, CBCGPPoint& pt1_1, CBCGPPoint& pt1_2,
					   CBCGPPoint* pt2_1, CBCGPPoint* pt2_2)
{
	if (rect.IsRectEmpty ())
	{
		return 1;
	}

	BOOL bInRect1 = rect.PtInRect (pt1_1);
	BOOL bInRect2 = rect.PtInRect (pt1_2);
	if (bInRect1 && bInRect2)
	{
		return 0;
	}

	if (pt1_1 == pt1_2)
	{
		return bInRect1 ? 0 : 1;
	}

	CBCGPPoint pt1(pt1_1);
	CBCGPPoint pt2(pt1_2);

	if (!bcg_CS_clip(rect, pt1, pt2))
	{
		return 1;
	}

	int count = 0;

 	if (bInRect1)
 	{
		pt1_1 = pt1;
		count = 1;
	}
	else if (bInRect2)
	{
		pt1_2 = pt2;
		count = 1;
	}
	else
	{
		count = 1;

		if (pt2_1 != NULL && pt2_2 != NULL)
		{
 			*pt2_1 = pt1;
 			*pt2_2 = pt1_2;

			count = 2;
		}

		pt1_2 = pt2;
	}

	return count;
}

BOOL bcg_CS_intersect(const CBCGPRect& rect, const CBCGPPoint& point1, const CBCGPPoint& point2,
					CBCGPPoint& point)
{
	CBCGPPoint pt1(point1);
	CBCGPPoint pt2(point2);

	if (bcg_CS_clip_inv(rect, pt1, pt2) == 1)
	{
		point = pt2;
		return TRUE;
	}

	return FALSE;
}

void CBCGPMatrix4x4::MultiplyMatrixes4x4(const CBCGPMatrix4x4& m1, const CBCGPMatrix4x4& m2)
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			m_data[i][j] = 0;

			for (int k = 0; k < 4; k++)
			{
				m_data[i][j] += m1[i][k] * m2[k][j];
			}
		}
	}
}

void CBCGPVector4::MultiplyVectors4x4(const CBCGPVector4& v1, const CBCGPVector4& v2)
{
	// for left coordinate system [a,  b] = [az * by - ay * bz, ax * bz - az * bx, ay * bx - ax * by]
	m_data[0] = v1[2] * v2[1] - v1[1] * v1[2];
	m_data[1] = v1[0] * v2[2] - v1[2] * v2[0];
	m_data[2] = v1[1] * v2[0] - v1[0] * v2[1];
	m_data[3] = 1.;
}

double CBCGPVector4::MultiplyVectorsScalar(const CBCGPVector4& v1)
{
	double dblSum = 0;

	for (int i = 0; i < 4; i++)
	{
		dblSum += v1[i] * m_data[i];
	}

	return dblSum;
}

void CBCGPVector4::CalcNormal(const CBCGPPoint& ptA, const CBCGPPoint& ptB, const CBCGPPoint& ptC)
{
	Fill(0);

	CBCGPPoint pt0 = ptA - ptB;
	CBCGPPoint pt1 = ptB - ptC;

	m_data[0] = (pt0.y * pt1.z) - (pt0.z * pt1.y);
	m_data[1] = (pt0.z * pt1.x) - (pt0.x * pt1.z);
	m_data[2] = (pt0.x * pt1.y) - (pt0.y * pt1.x);

	Normalize();
}

void CBCGPVector4::CalcPlane(const CBCGPPointsArray& arPoints)
{
	int i;
	const CBCGPPoint* pData = arPoints.GetData();

	if (pData == NULL)
	{
		return;
	}

	CBCGPMatrix4x4 mA;
	for (i = 0; i < 3; i++)
	{
		mA[i][0] = 1.;
		mA[i][1] = pData[i].y;
		mA[i][2] = pData[i].z;
	}

	CBCGPMatrix4x4 mB;
	for (i = 0; i < 3; i++)
	{
		mB[i][0] = pData[i].x;
		mB[i][1] = 1.;
		mB[i][2] = pData[i].z;
	}

	CBCGPMatrix4x4 mC;
	for (i = 0; i < 3; i++)
	{
		mC[i][0] = pData[i].x;
		mC[i][1] = pData[i].y;
		mC[i][2] = 1.;
	}

	CBCGPMatrix4x4 mD;
	for (i = 0; i < 3; i++)
	{
		mD[i][0] = pData[i].x;
		mD[i][1] = pData[i].y;
		mD[i][2] = pData[i].z;
	}


	m_data[0] = mA.Det(3);
	m_data[1] = mB.Det(3);
	m_data[2] = mC.Det(3);
	m_data[3] = -mD.Det(3);
}

void CBCGPVector4::CalcPlane(CBCGPPoint arPoints[])
{
	int i;

	CBCGPMatrix4x4 mA;
	for (i = 0; i < 3; i++)
	{
		mA[i][0] = 1.;
		mA[i][1] = arPoints[i].y;
		mA[i][2] = arPoints[i].z;
	}

	CBCGPMatrix4x4 mB;
	for (i = 0; i < 3; i++)
	{
		mB[i][0] = arPoints[i].x;
		mB[i][1] = 1.;
		mB[i][2] = arPoints[i].z;
	}

	CBCGPMatrix4x4 mC;
	for (i = 0; i < 3; i++)
	{
		mC[i][0] = arPoints[i].x;
		mC[i][1] = arPoints[i].y;
		mC[i][2] = 1.;
	}

	CBCGPMatrix4x4 mD;
	for (i = 0; i < 3; i++)
	{
		mD[i][0] = arPoints[i].x;
		mD[i][1] = arPoints[i].y;
		mD[i][2] = arPoints[i].z;
	}


	m_data[0] = mA.Det(3);
	m_data[1] = mB.Det(3);
	m_data[2] = mC.Det(3);
	m_data[3] = -mD.Det(3);
}

void CBCGPMatrix4x4::TransformPoint3D(const CBCGPPoint& ptIn, CBCGPPoint& ptOut) const
{
	double vResult[4];
	
	register double dblInX = ptIn.x;
	register double dblInY = ptIn.y;
	register double dblInZ = ptIn.z;

	register double dblTmp = dblInX * m_data[0][0];
	dblTmp += dblInY * m_data[1][0];
	dblTmp += dblInZ * m_data[2][0];
	dblTmp += m_data[3][0];
	vResult[0] = dblTmp;

	dblTmp = dblInX * m_data[0][1];
	dblTmp += dblInY * m_data[1][1];
	dblTmp += dblInZ * m_data[2][1];
	dblTmp += m_data[3][1];
	vResult[1] = dblTmp;

	dblTmp = dblInX * m_data[0][2];
	dblTmp += dblInY * m_data[1][2];
	dblTmp += dblInZ * m_data[2][2];
	dblTmp += m_data[3][2];
	vResult[2] = dblTmp;
	
	dblTmp = dblInX * m_data[0][3];
	dblTmp += dblInY * m_data[1][3];
	dblTmp += dblInZ * m_data[2][3];
	dblTmp += m_data[3][3];
	vResult[3] = dblTmp;

	ptOut.x = vResult[0] / vResult[3];
	ptOut.y = vResult[1] / vResult[3];
	ptOut.z = vResult[2] / vResult[3];
}

void CBCGPMatrix4x4::Inverse()
{
	double dblDet = Det(4);

	if (dblDet == 0)
	{
		return;
	}

	dblDet = 1./ dblDet;

	double d[16];

	d[0] = m_data[1][1]*m_data[2][2]*m_data[3][3] - m_data[1][1]*m_data[2][3]*m_data[3][2] - 
			m_data[2][1]*m_data[1][2]*m_data[3][3] + m_data[2][1]*m_data[1][3]*m_data[3][2] + 
			m_data[3][1]*m_data[1][2]*m_data[2][3] - m_data[3][1]*m_data[1][3]*m_data[2][2];

	d[1] = -m_data[0][1]*m_data[2][2]*m_data[3][3] + m_data[0][1]*m_data[2][3]*m_data[3][2] + 
			m_data[2][1]*m_data[0][2]*m_data[3][3] - m_data[2][1]*m_data[0][3]*m_data[3][2] - 
			m_data[3][1]*m_data[0][2]*m_data[2][3] + m_data[3][1]*m_data[0][3]*m_data[2][2];

	d[2] =  m_data[0][1]*m_data[1][2]*m_data[3][3] - m_data[0][1]*m_data[1][3]*m_data[3][2] - 
			m_data[1][1]*m_data[0][2]*m_data[3][3] + m_data[1][1]*m_data[0][3]*m_data[3][2] + 
			m_data[3][1]*m_data[0][2]*m_data[1][3] - m_data[3][1]*m_data[0][3]*m_data[1][2];

	d[3] = -m_data[0][1]*m_data[1][2]*m_data[2][3] + m_data[0][1]*m_data[1][3]*m_data[2][2] + 
			m_data[1][1]*m_data[0][2]*m_data[2][3] - m_data[1][1]*m_data[0][3]*m_data[2][2] - 
			m_data[2][1]*m_data[0][2]*m_data[1][3] + m_data[2][1]*m_data[0][3]*m_data[1][2];

	d[4] = -m_data[1][0]*m_data[2][2]*m_data[3][3] + m_data[1][0]*m_data[2][3]*m_data[3][2] + 
			m_data[2][0]*m_data[1][2]*m_data[3][3] - m_data[2][0]*m_data[1][3]*m_data[3][2] - 
			m_data[3][0]*m_data[1][2]*m_data[2][3] + m_data[3][0]*m_data[1][3]*m_data[2][2];

	d[5] =  m_data[0][0]*m_data[2][2]*m_data[3][3] - m_data[0][0]*m_data[2][3]*m_data[3][2] - 
			m_data[2][0]*m_data[0][2]*m_data[3][3] + m_data[2][0]*m_data[0][3]*m_data[3][2] + 
			m_data[3][0]*m_data[0][2]*m_data[2][3] - m_data[3][0]*m_data[0][3]*m_data[2][2];

	d[6] = -m_data[0][0]*m_data[1][2]*m_data[3][3] + m_data[0][0]*m_data[1][3]*m_data[3][2] + 
			m_data[1][0]*m_data[0][2]*m_data[3][3] - m_data[1][0]*m_data[0][3]*m_data[3][2] - 
			m_data[3][0]*m_data[0][2]*m_data[1][3] + m_data[3][0]*m_data[0][3]*m_data[1][2];

	d[7] =  m_data[0][0]*m_data[1][2]*m_data[2][3] - m_data[0][0]*m_data[1][3]*m_data[2][2] - 
			m_data[1][0]*m_data[0][2]*m_data[2][3] + m_data[1][0]*m_data[0][3]*m_data[2][2] + 
			m_data[2][0]*m_data[0][2]*m_data[1][3] - m_data[2][0]*m_data[0][3]*m_data[1][2];

	d[8] =  m_data[1][0]*m_data[2][1]*m_data[3][3] - m_data[1][0]*m_data[2][3]*m_data[3][1] - 
			m_data[2][0]*m_data[1][1]*m_data[3][3] + m_data[2][0]*m_data[1][3]*m_data[3][1] + 
			m_data[3][0]*m_data[1][1]*m_data[2][3] - m_data[3][0]*m_data[1][3]*m_data[2][1];

	d[9] = -m_data[0][0]*m_data[2][1]*m_data[3][3] + m_data[0][0]*m_data[2][3]*m_data[3][1] + 
			m_data[2][0]*m_data[0][1]*m_data[3][3] - m_data[2][0]*m_data[0][3]*m_data[3][1] - 
			m_data[3][0]*m_data[0][1]*m_data[2][3] + m_data[3][0]*m_data[0][3]*m_data[2][1];

	d[10]=  m_data[0][0]*m_data[1][1]*m_data[3][3] - m_data[0][0]*m_data[1][3]*m_data[3][1] - 
			m_data[1][0]*m_data[0][1]*m_data[3][3] + m_data[1][0]*m_data[0][3]*m_data[3][1] + 
			m_data[3][0]*m_data[0][1]*m_data[1][3] - m_data[3][0]*m_data[0][3]*m_data[1][1];

	d[11]= -m_data[0][0]*m_data[1][1]*m_data[2][3] + m_data[0][0]*m_data[1][3]*m_data[2][1] + 
			m_data[1][0]*m_data[0][1]*m_data[2][3] - m_data[1][0]*m_data[0][3]*m_data[2][1] - 
			m_data[2][0]*m_data[0][1]*m_data[1][3] + m_data[2][0]*m_data[0][3]*m_data[1][1];

	d[12]= -m_data[1][0]*m_data[2][1]*m_data[3][2] + m_data[1][0]*m_data[2][2]*m_data[3][1] + 
			m_data[2][0]*m_data[1][1]*m_data[3][2] - m_data[2][0]*m_data[1][2]*m_data[3][1] - 
			m_data[3][0]*m_data[1][1]*m_data[2][2] + m_data[3][0]*m_data[1][2]*m_data[2][1];

	d[13]=  m_data[0][0]*m_data[2][1]*m_data[3][2] - m_data[0][0]*m_data[2][2]*m_data[3][1] - 
			m_data[2][0]*m_data[0][1]*m_data[3][2] + m_data[2][0]*m_data[0][2]*m_data[3][1] + 
			m_data[3][0]*m_data[0][1]*m_data[2][2] - m_data[3][0]*m_data[0][2]*m_data[2][1];

	d[14]= -m_data[0][0]*m_data[1][1]*m_data[3][2] + m_data[0][0]*m_data[1][2]*m_data[3][1] + 
			m_data[1][0]*m_data[0][1]*m_data[3][2] - m_data[1][0]*m_data[0][2]*m_data[3][1] - 
			m_data[3][0]*m_data[0][1]*m_data[1][2] + m_data[3][0]*m_data[0][2]*m_data[1][1];

	d[15]=  m_data[0][0]*m_data[1][1]*m_data[2][2] - m_data[0][0]*m_data[1][2]*m_data[2][1] - 
			m_data[1][0]*m_data[0][1]*m_data[2][2] + m_data[1][0]*m_data[0][2]*m_data[2][1] + 
			m_data[2][0]*m_data[0][1]*m_data[1][2] - m_data[2][0]*m_data[0][2]*m_data[1][1];

	for (int i = 0; i < 16; i++)
	{
		d[i] *= dblDet;
	}

	memcpy(m_data, d, sizeof(double) * 16);
}

BOOL BCGPIntersectPoints2D(const CBCGPPoint& ptStart1, const CBCGPPoint& ptEnd1, 
					   const CBCGPPoint& ptStart2, const CBCGPPoint& ptEnd2,
					   CBCGPPoint& ptIntersect)
{
	CBCGPPoint ptES(ptEnd1.x - ptStart1.x, ptEnd1.y - ptStart1.y);
	CBCGPPoint ptSE(ptStart2.x - ptEnd2.x, ptStart2.y - ptEnd2.y);
	CBCGPPoint ptSS(ptStart2.x - ptStart1.x, ptStart2.y - ptStart1.y);

	double d = ptES.y * ptSE.x - ptSE.y * ptES.x;

	if (d == 0)
	{
		// parallel
		// if ((d1 == 0.0) && (d2 == 0.0)) - same line (see below)
		return FALSE;
	}

	double d1 = (ptSS.y * ptSE.x - ptSE.y * ptSS.x) / d;
	double d2 = (ptES.y * ptSS.x - ptSS.y * ptES.x) / d;

	if (0.0 <= d1 && d1 <= 1.0 && 0.0 <= d2 && d2 <= 1.0)
	{
		ptIntersect.x = ptStart1.x + ptES.x * d1;
		ptIntersect.y = ptStart1.y + ptES.y * d1;

		return TRUE;
	}

	return FALSE;
}

int FindPointInPolygon(const CBCGPPointsArray& arPoly, const CBCGPPoint& pt)
{
	int nSize = (int)arPoly.GetSize ();
	if (nSize == 0)
	{
		return -1;
	}

	int index = -1;
	const CBCGPPoint* pData = arPoly.GetData();

	for (int i = 0; i < nSize; i++)
	{
		if (pData[i].x == pt.x && pData[i].y == pt.y)
		{
			index = i;
			break;
		}
	}

	return index;
}

int FindPointInPolygon(const CBCGPPoint arPoly[], const CBCGPPoint& pt, int nSize)
{
	if (nSize == 0)
	{
		return -1;
	}

	int index = -1;

	for (int i = 0; i < nSize; i++)
	{
		if (arPoly[i].x == pt.x && arPoly[i].y == pt.y)
		{
			index = i;
			break;
		}
	}

	return index;
}

BOOL AddPointInPolygon(CBCGPPointsArray& arPoly, const CBCGPPoint& pt)
{
	if (FindPointInPolygon (arPoly, pt) != -1)
	{
		return FALSE;
	}

	arPoly.Add (pt);

	return TRUE;
}

BOOL AddPointInPolygon(CBCGPPoint arPoly[], const CBCGPPoint& pt, int& nPos)
{
	if (FindPointInPolygon (arPoly, pt, nPos) != -1)
	{
		return FALSE;
	}

	arPoly[nPos] = pt;
	nPos++;

	return TRUE;
}

BOOL PointInPolygon(const CBCGPPointsArray& arPoly, const CBCGPPoint& pt)
{
	int nSize = (int)arPoly.GetSize ();
    if (nSize < 3)
    {
        return FALSE;
    }

	const CBCGPPoint* pData = arPoly.GetData();

    int count   = 0;
    int current = 0;
    int next    = 1;

    while(next < nSize)
    {
        BOOL b = (pData[next].y <= pt.y);

        if (pData[current].y <= pt.y)
        {
            b = !b;
        }

        if (b)
        {
            if(((pData[next].x - pData[current].x) * (pt.y - pData[current].y) / 
                (pData[next].y - pData[current].y)) <= (pt.x - pData[current].x))
            {
                count++;
            }
        }

        current++;
        next++;
    }

    if (pData[0] != pData[nSize - 1])
    {
        current = nSize - 1;
        next    = 0;
        BOOL b  = (pData[next].y <= pt.y);

        if (pData[current].y <= pt.y)
        {
            b = !b;
        }

        if (b)
        {
            if(((pData[next].x - pData[current].x) * (pt.y - pData[current].y) / 
                (pData[next].y - pData[current].y)) <= (pt.x - pData[current].x))
            {
                count++;
            }
        }
    }

    return (count & 1) ? TRUE : FALSE;
}

BOOL PointInPolygon(CBCGPPoint arPoly[], const CBCGPPoint& pt)
{
    int count   = 0;
    int current = 0;
    int next    = 1;

    while(next < 3)
    {
        BOOL b = (arPoly[next].y <= pt.y);

        if (arPoly[current].y <= pt.y)
        {
            b = !b;
        }

        if (b)
        {
            if(((arPoly[next].x - arPoly[current].x) * (pt.y - arPoly[current].y) / 
                (arPoly[next].y - arPoly[current].y)) <= (pt.x - arPoly[current].x))
            {
                count++;
            }
        }

        current++;
        next++;
    }

    if (arPoly[0].x != arPoly[2].x || arPoly[0].y != arPoly[2].y)
    {
        current = 2;
        next    = 0;
        BOOL b  = (arPoly[next].y <= pt.y);

        if (arPoly[current].y <= pt.y)
        {
            b = !b;
        }

        if (b)
        {
            if(((arPoly[next].x - arPoly[current].x) * (pt.y - arPoly[current].y) / 
                (arPoly[next].y - arPoly[current].y)) <= (pt.x - arPoly[current].x))
            {
                count++;
            }
        }
    }

    return (count & 1) ? TRUE : FALSE;
}


BOOL BCGPCalculateIntersectPoint(const CBCGPPointsArray& arPoly1, const CBCGPPointsArray& arPoly2, 
						CBCGPPoint& ptIntersect)
{
	int nPoly1Size = (int)arPoly1.GetSize();
	int nPoly2Size = (int)arPoly2.GetSize();

	if (nPoly1Size == 0 || nPoly2Size == 0)
	{
		return FALSE;
	}

	const CBCGPPoint* pData1 = arPoly1.GetData ();
	const CBCGPPoint* pData2 = arPoly2.GetData ();

	CBCGPPointsArray arPolyRes;
	arPolyRes.SetSize (0, nPoly1Size + nPoly2Size);

	int i = 0;
	int j = 0;

	// 1. find all points from arPoly1F in arPoly2F
	int nSize1 = 0;
	for (i = 0; i < nPoly1Size; i++)
	{
		const CBCGPPoint& pt = pData1[i];
		if (PointInPolygon (arPoly2, pt))
		{
			if (AddPointInPolygon(arPolyRes, pt))
			{
				nSize1++;
			}
		}
	}

	// all points in arPoly2F
	if (nSize1 != nPoly1Size)
	{
		// 2. find all points from arPoly2F in arPoly1F
		int nSize2 = 0;
		for (i = 0; i < nPoly2Size; i++)
		{
			const CBCGPPoint& pt = pData2[i];
			if (PointInPolygon (arPoly1, pt))
			{
				if (AddPointInPolygon(arPolyRes, pt))
				{
					nSize2++;
				}
			}
		}

		// all points in arPoly1F
		if (nSize1 != 0 || nSize2 != nPoly2Size)
		{
			// 3. find all intersection points arPoly1F and arPoly2F
			for (i = 0; i < nPoly1Size; i++)
			{
				const CBCGPPoint& ptStart1 = pData1[i];
				const CBCGPPoint& ptEnd1   = pData1[(i + 1) % nPoly1Size];

				for (j = 0; j < nPoly2Size; j++)
				{
					const CBCGPPoint& ptStart2 = pData2[j];
					const CBCGPPoint& ptEnd2   = pData2[(j + 1) % nPoly2Size];

 					if (bcgp_classify_point2D(ptStart1, ptEnd1, ptStart2) != 
 						bcgp_classify_point2D(ptStart1, ptEnd1, ptEnd2)) 
 					{
						CBCGPPoint pt;
						if (BCGPIntersectPoints2D (ptStart1, ptEnd1, ptStart2, ptEnd2, pt))
						{
							AddPointInPolygon(arPolyRes, pt);
						}
					}
				}
			}
		}
	}

	int nSize = (int)arPolyRes.GetSize();
	if (nSize < 2)
	{
		return FALSE;
	}

	ptIntersect.x = 0.0;
	ptIntersect.y = 0.0;

	for (i = 0; i < nSize; i++)
	{
		ptIntersect += arPolyRes[i];
	}

	ptIntersect /= nSize;

	return TRUE;
}

BOOL BCGPIntersectTriangle2D(CBCGPPoint arPoly1[], CBCGPPoint arPoly2[], CBCGPPoint arPolyRes[], int& nResCount)
{
	int nPoly1Size = 3;
	int nPoly2Size = 3;
	nResCount = 0;

	int i = 0;
	int j = 0;

	// 1. find all points from arPoly1F in arPoly2F
	int nSize1 = 0;
	for (i = 0; i < nPoly1Size; i++)
	{
		const CBCGPPoint& pt = arPoly1[i];
		if (PointInPolygon (arPoly2, pt))
		{
			if (AddPointInPolygon(arPolyRes, pt, nResCount))
			{
				nSize1++;
			}
		}
	}

	// all points in arPoly2F
	if (nSize1 == nPoly1Size)
	{
		return TRUE;
	}

	// 2. find all points from arPoly2F in arPoly1F
	int nSize2 = 0;
	for (i = 0; i < nPoly2Size; i++)
	{
		const CBCGPPoint& pt = arPoly2[i];
		if (PointInPolygon (arPoly1, pt))
		{
			if (AddPointInPolygon(arPolyRes, pt, nResCount))
			{
				nSize2++;
			}
		}
	}

	// all points in arPoly1F
	if (nSize1 == 0 && nSize2 == nPoly2Size)
	{
		return TRUE;
	}

	// 3. find all intersection points arPoly1F and arPoly2F
	for (i = 0; i < nPoly1Size; i++)
	{
		const CBCGPPoint& ptStart1 = arPoly1[i];
		const CBCGPPoint& ptEnd1   = arPoly1[(i + 1) % nPoly1Size];

		for (j = 0; j < nPoly2Size; j++)
		{
			const CBCGPPoint& ptStart2 = arPoly2[j];
			const CBCGPPoint& ptEnd2   = arPoly2[(j + 1) % nPoly2Size];

 			if (bcgp_classify_point2D(ptStart1, ptEnd1, ptStart2) != 
 				bcgp_classify_point2D(ptStart1, ptEnd1, ptEnd2)) 
 			{
				CBCGPPoint pt;
				if (BCGPIntersectPoints2D (ptStart1, ptEnd1, ptStart2, ptEnd2, pt))
				{
					AddPointInPolygon(arPolyRes, pt, nResCount);
				}
			}
		}
	}

	if (nResCount < 3)
	{
		nResCount = 0;
		return FALSE;
	}

	return TRUE;
}

BOOL bcg_IsFullEllipse(double dblStartAngle, double dblFinishAngle, BOOL bIsClockwise, double dblPrecision)
{
	double dblAngle1 = bcg_normalize_deg (dblStartAngle);
	double dblAngle2 = bcg_normalize_deg (dblFinishAngle);

	if (fabs(dblAngle1 - dblAngle2) < FLT_EPSILON && fabs(dblStartAngle - dblFinishAngle) > FLT_EPSILON)
	{
		return TRUE;
	}

	if (bIsClockwise)
	{
		if (dblAngle1 < dblAngle2)
		{
			dblAngle1 += 360.0;
		}
	}
	else
	{
		if (dblAngle2 < dblAngle1)
		{
			dblAngle2 += 360.0;
		}
	}

	if (dblAngle2 < dblAngle1)
	{
		double t = dblAngle1;
		dblAngle1 = dblAngle2;
		dblAngle2 = t;
	}

	return (dblAngle2 - dblAngle1) <= dblPrecision;
}

double bcg_double_precision(double value, int precision)
{
	if (precision < 0)
	{
		return value;
	}

    if (value == 0.0)
    {
        return 0.0;
    }
	
    if (precision > DBL_DIG)
    {
        precision = DBL_DIG;
    }
	
    double fract = modf(value, &value);
    double prec = pow((double)10.0, (int)precision);
	
    if (fabs(fract) > DBL_EPSILON / 2.0)
    {
        double f = bcg_sign(fract) * DBL_EPSILON / 2.0;
        fract = bcg_round(fract * prec) / prec + f;
    }
    else
    {
        fract = 0.0;
    }
	
    return value + fract;
}
