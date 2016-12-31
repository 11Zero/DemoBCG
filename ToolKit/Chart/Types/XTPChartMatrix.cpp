// XTPChartMatrix.cpp
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

#include "XTPChartMatrix.h"
#include "XTPChartDiagramPoint.h"
#include "../Utils/XTPChartMathUtils.h"


CXTPChartMatrix::CXTPChartMatrix(XTPChartMatrixType type /* = xtpChartMatrixModelView */)
{
	m_type = type;
	memset(m_matrix, 0, sizeof(m_matrix));

	m_matrix[0] = m_matrix[5] = m_matrix[10] = m_matrix[15] = 1.0;
}

CXTPChartMatrix CXTPChartMatrix::CalculateVMultVt(const CXTPChartDiagramPoint& p)
{
	CXTPChartMatrix result;
	result[0] = p.X * p.X;
	result[1] = p.X * p.Y;
	result[2] = p.X * p.Z;
	result[3] = 0.0;
	result[4] = p.Y * p.X;
	result[5] = p.Y * p.Y;
	result[6] = p.Y * p.Z;
	result[7] = 0.0;
	result[8] = p.Z * p.X;
	result[9] = p.Z * p.Y;
	result[10] = p.Z * p.Z;
	result[11] = 0.0;
	result[12] = 0.0;
	result[13] = 0.0;
	result[14] = 0.0;
	result[15] = 0.0;
	return result;
}

double& CXTPChartMatrix::operator[](int nIndex)
{
	return m_matrix[nIndex];
}

double CXTPChartMatrix::operator[](int nIndex) const
{
	return m_matrix[nIndex];
}

CXTPChartMatrix CXTPChartMatrix::operator+(const CXTPChartMatrix& m) const
{
	CXTPChartMatrix result(m_type);

	for (int i = 0; i < 16; i++)
		result[i] = m_matrix[i] + m[i];

	return result;
}


CXTPChartMatrix CXTPChartMatrix::operator-(const CXTPChartMatrix& m) const
{
	CXTPChartMatrix result(m_type);

	for (int i = 0; i < 16; i++)
		result[i] = m_matrix[i] - m[i];

	return result;
}

CXTPChartMatrix CXTPChartMatrix::operator*(double factor) const
{
	CXTPChartMatrix result(m_type);

	for (int i = 0; i < 16; i++)
		result[i] = m_matrix[i] * factor;

	return result;
}

CXTPChartMatrix CXTPChartMatrix::operator*(const CXTPChartMatrix& m2) const
{
	CXTPChartMatrix result(m_type);

	result.m_matrix[0] = m_matrix[0] * m2.m_matrix[0] + m_matrix[4] * m2.m_matrix[1] + m_matrix[8] * m2.m_matrix[2] + m_matrix[12] * m2.m_matrix[3];
	result.m_matrix[1] = m_matrix[1] * m2.m_matrix[0] + m_matrix[5] * m2.m_matrix[1] + m_matrix[9] * m2.m_matrix[2] + m_matrix[13] * m2.m_matrix[3];
	result.m_matrix[2] = m_matrix[2] * m2.m_matrix[0] + m_matrix[6] * m2.m_matrix[1] + m_matrix[10] * m2.m_matrix[2] + m_matrix[14] * m2.m_matrix[3];
	result.m_matrix[3] = m_matrix[3] * m2.m_matrix[0] + m_matrix[7] * m2.m_matrix[1] + m_matrix[11] * m2.m_matrix[2] + m_matrix[15] * m2.m_matrix[3];
	result.m_matrix[4] = m_matrix[0] * m2.m_matrix[4] + m_matrix[4] * m2.m_matrix[5] + m_matrix[8] * m2.m_matrix[6] + m_matrix[12] * m2.m_matrix[7];
	result.m_matrix[5] = m_matrix[1] * m2.m_matrix[4] + m_matrix[5] * m2.m_matrix[5] + m_matrix[9] * m2.m_matrix[6] + m_matrix[13] * m2.m_matrix[7];
	result.m_matrix[6] = m_matrix[2] * m2.m_matrix[4] + m_matrix[6] * m2.m_matrix[5] + m_matrix[10] * m2.m_matrix[6] + m_matrix[14] * m2.m_matrix[7];
	result.m_matrix[7] = m_matrix[3] * m2.m_matrix[4] + m_matrix[7] * m2.m_matrix[5] + m_matrix[11] * m2.m_matrix[6] + m_matrix[15] * m2.m_matrix[7];
	result.m_matrix[8] = m_matrix[0] * m2.m_matrix[8] + m_matrix[4] * m2.m_matrix[9] + m_matrix[8] * m2.m_matrix[10] + m_matrix[12] * m2.m_matrix[11];
	result.m_matrix[9] = m_matrix[1] * m2.m_matrix[8] + m_matrix[5] * m2.m_matrix[9] + m_matrix[9] * m2.m_matrix[10] + m_matrix[13] * m2.m_matrix[11];
	result.m_matrix[10] = m_matrix[2] * m2.m_matrix[8] + m_matrix[6] * m2.m_matrix[9] + m_matrix[10] * m2.m_matrix[10] + m_matrix[14] * m2.m_matrix[11];
	result.m_matrix[11] = m_matrix[3] * m2.m_matrix[8] + m_matrix[7] * m2.m_matrix[9] + m_matrix[11] * m2.m_matrix[10] + m_matrix[15] * m2.m_matrix[11];
	result.m_matrix[12] = m_matrix[0] * m2.m_matrix[12] + m_matrix[4] * m2.m_matrix[13] + m_matrix[8] * m2.m_matrix[14] + m_matrix[12] * m2.m_matrix[15];
	result.m_matrix[13] = m_matrix[1] * m2.m_matrix[12] + m_matrix[5] * m2.m_matrix[13] + m_matrix[9] * m2.m_matrix[14] + m_matrix[13] * m2.m_matrix[15];
	result.m_matrix[14] = m_matrix[2] * m2.m_matrix[12] + m_matrix[6] * m2.m_matrix[13] + m_matrix[10] * m2.m_matrix[14] + m_matrix[14] * m2.m_matrix[15];
	result.m_matrix[15] = m_matrix[3] * m2.m_matrix[12] + m_matrix[7] * m2.m_matrix[13] + m_matrix[11] * m2.m_matrix[14] + m_matrix[15] * m2.m_matrix[15];

	return result;
}

double* CXTPChartMatrix::GetMatrix() const
{
	return (double*)m_matrix;
}
