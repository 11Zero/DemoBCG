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

#ifndef __BCGPMATH_H
#define __BCGPMATH_H

#ifndef _USE_MATH_DEFINES
    #define _USE_MATH_DEFINES
#endif

#include <math.h>

#ifndef M_PI   // pi
    #define M_PI       3.14159265358979323846
#endif

#ifndef M_PI_2 // pi/2
	#define M_PI_2     1.57079632679489661923
#endif

#ifndef M_LN2  // ln(2)
	#define M_LN2      0.693147180559945309417
#endif


#include "BCGPGraphicsManager.h"

inline int bcg_sqr(int value)
{
	return value * value;
};

inline double bcg_sqr(double value)
{
	return value * value;
};

inline int bcg_sign(int value)
{
	if (value == 0)
	{
		return 0;
	}

	return value > 0 ? 1 : -1;
}

inline double bcg_sign(double value)
{
	if (value == 0.0)
	{
		return 0.0;
	}

	return value > 0.0 ? 1.0 : -1.0;
}

inline int bcg_round(double val)
{
    return int((val - int(val - 0.5)) >= 0.5 ? val + 0.5 : val - 0.5);
}

inline double bcg_frac(double value)
{
	return value - (int)value;
}

inline double bcg_clamp(double value, double low, double high)
{
	return value < low ? low : (value > high ? high : value);
}

inline int bcg_clamp(int value, int low, int high)
{
	return value < low ? low : (value > high ? high : value);
}

inline double bcg_clamp_to_byte(double value)
{
	return bcg_clamp(value, 0.0, 255.0);
}

inline double bcg_distance(double dx, double dy)
{
	return sqrt(dx * dx + dy * dy);
}

inline double bcg_distance(const CBCGPPoint& pt1, const CBCGPPoint& pt2)
{
	return bcg_distance(pt2.x - pt1.x, pt2.y - pt1.y);
}

inline CBCGPPoint bcg_lineCenter (const CBCGPPoint& pt1, const CBCGPPoint& pt2)
{
    return CBCGPPoint ((pt1.x + pt2.x) / 2.0, (pt1.y + pt2.y) / 2.0);
}

inline double bcg_angle (double dx, double dy)
{
	if (dy == 0)
	{
		return (dx < 0.0) ? -M_PI : 0.0;
	}

	return atan2 (dy, dx);
}

inline double bcg_angle (const CBCGPPoint& pt1, const CBCGPPoint& pt2, BOOL invY = FALSE)
{
	return bcg_angle (pt2.x - pt1.x, invY ? pt1.y - pt2.y : pt2.y - pt1.y);
}

inline double bcg_deg2rad(double value)
{
	return value * M_PI / 180.0;
}

inline double bcg_rad2deg(double value)
{
	return value * 180.0 / M_PI;
}

inline double bcgp_normal_of_3D_vector(const double vect[3])
{
	return sqrt(vect[0] * vect[0] + vect[1] * vect[1] + vect[2] * vect[2]);
}

inline double bcgp_normalize_3D_vector(double vect[3])
{
	double dblVal = 0.; 

	if ((dblVal = bcgp_normal_of_3D_vector(vect)) != 0.)
	{
		for (int i = 0; i < 3; i++)
		{
			vect[i] /= dblVal;
		}
	}

	return dblVal;
}

inline void bcgp_cross_3D_vectors(const double vect1[3], const double vect2[3], double vect3[3])
{
	vect3[0] = vect1[1] * vect2[2] - vect1[2] * vect2[1]; 
	vect3[1] = vect1[2] * vect2[0] - vect1[0] * vect2[2];
	vect3[2] = vect1[0] * vect2[1] - vect1[1] * vect2[0];
}

BCGCBPRODLLEXPORT double bcg_double_precision(double value, int precision);

// returns positive value if p2 is located on the left side from the line defined by p0 and p1
// negative - on the right side
// 0 - on the line
// the direction is defined by vector p1 - p0
inline int bcgp_classify_point2D(const CBCGPPoint& p0, const CBCGPPoint& p1, const CBCGPPoint& p2) 
{
	return (int)bcg_sign((p1.x - p0.x) * (p2.y - p0.y) - (p2.x - p0.x) * (p1.y - p0.y));
}

void PrintPoint3D(const CBCGPPoint& pt);


BCGCBPRODLLEXPORT double bcg_normalize_rad(double value);
BCGCBPRODLLEXPORT double bcg_normalize_deg(double value);

BCGCBPRODLLEXPORT int bcg_pointInLine (const CBCGPPoint& pt1, const CBCGPPoint& pt2,
                                        const CBCGPPoint& ptTest, double precision);

BCGCBPRODLLEXPORT bool bcg_linesIntersection (
                            const CBCGPPoint& pt1, const CBCGPPoint& pt2,
                            const CBCGPPoint& pt3, const CBCGPPoint& pt4,
                            double& outX, double& outY);

BCGCBPRODLLEXPORT bool bcg_pointInPolygon (CBCGPPoint* pPoints, int nPoints, const CBCGPPoint& ptTest);

BCGCBPRODLLEXPORT bool bcg_pointInPie (const CBCGPRect& rect, double dblAngleStart, double dblAngleFinish, const CBCGPPoint& ptTest, double dblDoughnutPercent = 0.);

BCGCBPRODLLEXPORT bool bcg_clockwise (const CBCGPPoint& pt1, const CBCGPPoint& pt2, const CBCGPPoint& pt3);

BCGCBPRODLLEXPORT void bcg_setLineLength (const CBCGPPoint& pt1, CBCGPPoint& pt2, double dNewLength);

BCGCBPRODLLEXPORT BOOL bcg_CS_clip(const CBCGPRect& rect, CBCGPPoint& point1, CBCGPPoint& point2);
BCGCBPRODLLEXPORT int bcg_CS_clip_inv(const CBCGPRect& rect, CBCGPPoint& pt1_1, CBCGPPoint& pt1_2,
									CBCGPPoint* pt2_1 = NULL, CBCGPPoint* pt2_2 = NULL);
BCGCBPRODLLEXPORT BOOL bcg_CS_intersect(const CBCGPRect& rect, const CBCGPPoint& point1, const CBCGPPoint& point2,
									CBCGPPoint& point);

BCGCBPRODLLEXPORT BOOL bcg_IsFullEllipse(double dblStartAngle, double dblFinishAngle, BOOL bIsClockwise, double dblPrecision);

class BCGCBPRODLLEXPORT CBCGPVector
{
public:
	CBCGPVector()
		: m_Size (0)
		, m_pData(NULL)
	{
	}
	CBCGPVector(int size)
		: m_Size (0)
		, m_pData(NULL)
	{
		Create(size);
	}
	CBCGPVector(int size, double value)
		: m_Size (0)
		, m_pData(NULL)
	{
		Create(size, value);
	}

	~CBCGPVector()
	{
		Destroy();
	}

	BOOL IsEmpty() const
	{
		return m_pData == NULL;
	}

	int GetSize() const
	{
		return m_Size;
	}

	const double* GetData() const
	{
		return m_pData;
	}

	void Create(int size)
	{
		if (size <= 0)
		{
			ASSERT(FALSE);
			return;
		}

		Destroy();

		m_Size = size;
		m_pData = new double[m_Size];
	}

	void Create(int size, double value)
	{
		Create(size);
		if (!IsEmpty ())
		{
			Fill(value);
		}
	}

	void Destroy()
	{
		if (!IsEmpty())
		{
			delete [] m_pData;
			m_pData = NULL;
		}
	}

	void Fill(double value)
	{
		if (IsEmpty())
		{
			ASSERT(FALSE);
			return;
		}

		for (int x = 0; x < m_Size; x++)
		{
			m_pData[x] = value;
		}
	}

	double Sum() const
	{
		if (IsEmpty())
		{
			ASSERT(FALSE);
			return 0.0;
		}

		double sum = 0.0;
		for (int i = 0; i < m_Size; i++)
		{
			sum += m_pData[i];
		}

		return sum;
	}

	void Swap()
	{
		if (IsEmpty())
		{
			ASSERT(FALSE);
			return;
		}

		for (int i = 0; i < m_Size / 2; i++)
		{
			double temp = m_pData[i];
			m_pData[i] = m_pData[m_Size - i - 1];
			m_pData[m_Size - i - 1] = temp;
		}
	}

	double& operator[](int index)
	{
		ASSERT(0 <= index && index < m_Size);
		return m_pData[index];
	}
	const double& operator[](int index) const
	{
		ASSERT(0 <= index && index < m_Size);
		return m_pData[index];
	}

protected:
	int		m_Size;
	double*	m_pData;
};

class BCGCBPRODLLEXPORT CBCGPMatrix
{
public:
	CBCGPMatrix()
		: m_Rows (0)
		, m_Cols (0)
		, m_pData(NULL)
	{
	}
	CBCGPMatrix(int rows, int cols)
		: m_Rows (0)
		, m_Cols (0)
		, m_pData(NULL)
	{
		Create(rows, cols);
	}
	CBCGPMatrix(int rows, int cols, double value)
		: m_Rows (0)
		, m_Cols (0)
		, m_pData(NULL)
	{
		Create(rows, cols, value);
	}

	void LoadIdentity()
	{
		if (IsEmpty() || m_Rows != m_Cols)
		{
			return;
		}

		for (int y = 0; y < m_Rows; y++)
		{
			double* pRow = m_pData[y];

			for (int x = 0; x < m_Cols; x++)
			{
				if (x != y)
				{
					*pRow++ = 0.;
				}
				else
				{
					*pRow++ = 1.;
				}
			}
		}
	}

	virtual ~CBCGPMatrix()
	{
		Destroy();
	}

	BOOL IsEmpty() const
	{
		return m_pData == NULL;
	}

	int GetRows() const
	{
		return m_Rows;
	}

	int GetCols() const
	{
		return m_Cols;
	}

	double** const GetData() const
	{
		return m_pData;
	}

	void Create(int rows, int cols)
	{
		if (rows <= 0 || cols <= 0)
		{
			ASSERT(FALSE);
			return;
		}

		Destroy();

		m_Rows = rows;
		m_Cols = cols;

		m_pData = (double**)(new LPVOID[m_Rows]);
		for (int y = 0; y < m_Rows; y++)
		{
			m_pData[y] = new double[m_Cols];
		}
	}

	void Create(int rows, int cols, double value)
	{
		Create(rows, cols);
		if (!IsEmpty ())
		{
			Fill(value);
		}
	}

	void Destroy()
	{
		if (!IsEmpty())
		{
			for (int y = 0; y < m_Rows; y++)
			{
				delete [] m_pData[y];
			}

			delete [] m_pData;
			m_pData = NULL;
		}
	}

	void Fill(double value)
	{
		if (IsEmpty())
		{
			ASSERT(FALSE);
			return;
		}

		for (int y = 0; y < m_Rows; y++)
		{
			double* pRow = m_pData[y];

			for (int x = 0; x < m_Cols; x++)
			{
				*pRow++ = value;
			}
		}
	}

	void SetMatrix(const CBCGPMatrix& src)
	{
		Create(src.GetRows(), src.GetCols(), 0);

		for (int i = 0; i < src.GetRows(); i++)
		{
			const double* pRow = src[i];
			memcpy(m_pData[i], pRow, sizeof(double) * m_Cols);
		}
	}

	void GetRow(CBCGPVector& vector, int index) const
	{
		if (IsEmpty())
		{
			ASSERT(FALSE);
			return;
		}

		if (index < 0 || m_Rows <= index)
		{
			ASSERT(FALSE);
			return;
		}

		vector.Create(m_Cols);
		memcpy((double*)vector.GetData(), m_pData[index], sizeof(double) * m_Cols);
	}

	void SetRow(const CBCGPVector& vector, int index)
	{
		if (IsEmpty())
		{
			ASSERT(FALSE);
			return;
		}

		if (index < 0 || m_Rows <= index)
		{
			ASSERT(FALSE);
			return;
		}

		if (vector.GetSize() != m_Cols)
		{
			ASSERT(FALSE);
			return;
		}

		memcpy(m_pData[index], vector.GetData(), sizeof(double) * m_Cols);
	}

	void GetColumn(CBCGPVector& vector, int index) const
	{
		if (IsEmpty())
		{
			ASSERT(FALSE);
			return;
		}

		if (index < 0 || m_Cols <= index)
		{
			ASSERT(FALSE);
			return;
		}

		vector.Create(m_Rows);

		for (int i = 0; i < m_Rows; i++)
		{
			vector[i] = m_pData[i][index];
		}
	}

	void SetColumn(const CBCGPVector& vector, int index)
	{
		if (IsEmpty())
		{
			ASSERT(FALSE);
			return;
		}

		if (index < 0 || m_Cols <= index)
		{
			ASSERT(FALSE);
			return;
		}

		if (vector.GetSize() != m_Rows)
		{
			ASSERT(FALSE);
			return;
		}

		for (int i = 0; i < m_Rows; i++)
		{
			m_pData[i][index] = vector[i];
		}
	}

	double Sum() const
	{
		if (IsEmpty())
		{
			ASSERT(FALSE);
			return 0.0;
		}

		double sum = 0.0;
		for (int y = 0; y < m_Rows; y++)
		{
			double* pRow = m_pData[y];
			for (int x = 0; x < m_Cols; x++)
			{
				sum += pRow[x];
			}
		}

		return sum;
	}

	double SumRow(int index) const
	{
		if (IsEmpty())
		{
			ASSERT(FALSE);
			return 0.0;
		}

		if (index < 0 || m_Rows <= index)
		{
			ASSERT(FALSE);
			return 0.0;
		}

		double sum = 0.0;
		double* pRow = m_pData[index];
		for (int i = 0; i < m_Cols; i++)
		{
			sum += pRow[i];
		}

		return sum;
	}

	double SumColumn(int index) const
	{
		if (IsEmpty())
		{
			ASSERT(FALSE);
			return 0.0;
		}

		if (index < 0 || m_Cols <= index)
		{
			ASSERT(FALSE);
			return 0.0;
		}

		double sum = 0.0;
		for (int i = 0; i < m_Rows; i++)
		{
			sum += m_pData[i][index];
		}

		return sum;
	}

	double* operator[](int index)
	{
		ASSERT(0 <= index && index < m_Rows);
		return m_pData[index];
	}
	const double* operator[](int index) const
	{
		ASSERT(0 <= index && index < m_Rows);
		return m_pData[index];
	}

protected:
	int			m_Rows;
	int			m_Cols;
	double**	m_pData;
};

class BCGCBPRODLLEXPORT CBCGPVector4 
{
public:
	CBCGPVector4(double dblVal = 1)
	{
		Fill(dblVal);
	}

	CBCGPVector4(const CBCGPPoint& pt)
	{
		FromPoint3D(pt);
	}

	void ToPoint(CBCGPPoint& pt) const
	{
		pt.x = m_data[0];
		pt.y = m_data[1];
		pt.z = m_data[2];
	}

	void FromPoint3D(const CBCGPPoint& pt)
	{
		m_data[0] = pt.x;
		m_data[1] = pt.y;
		m_data[2] = pt.z;
		m_data[3] = 1;
	}

	void Fill(double dblVal)
	{
		for (int i = 0; i < 4; i++)
		{
			m_data[i] = dblVal;
		}
	}

	void Normalize()
	{
		double dblLen = 0;

		int i = 0;
		for (i = 0; i < 4; i++)
		{
			dblLen += m_data[i] * m_data[i];
		}

		if (dblLen == 0.)
		{
			dblLen = 1.;
		}

		dblLen = sqrt(dblLen);

		for (i = 0; i < 4; i++)
		{
			m_data[i] = m_data[i] / dblLen;
		}
	}

	void SetOpenGLDefaultNormal()
	{
		m_data[0] = 0.;
		m_data[1] = 0.;
		m_data[2] = 1.;
		m_data[3] = 0.;
	}

	BOOL IsOpenGLDefaultNormal() const
	{
		return m_data[0] == 0. && m_data[1] == 0. && m_data[2] == 1. && m_data[3] == 0.;
	}

	BOOL IsNull() const 
	{
		for (int i = 0; i < 4; i++)
		{
			if (m_data[i] != 0.)
			{
				return FALSE;
			}
		}

		return TRUE;
	}

	CBCGPVector4& operator=(const CBCGPVector4& src)
	{
		for (int i = 0; i < 4; i++)
		{
			m_data[i] = src[i];
		}

		return *this;
	}

	double& operator[](int index)
	{
		ASSERT(0 <= index && index < 4);
		return m_data[index];
	}
	
	const double& operator[](int index) const
	{
		ASSERT(0 <= index && index < 4);
		return m_data[index];
	}

	void MultiplyVectors4x4(const CBCGPVector4& v1, const CBCGPVector4& v2);

	double MultiplyVectorsScalar(const CBCGPVector4& v1);

	void CalcNormal(const CBCGPPoint& ptA, const CBCGPPoint& ptB, const CBCGPPoint& ptC);
	void CalcPlane(const CBCGPPointsArray& arPoints);
	void CalcPlane(CBCGPPoint arPoints[]);

	double* GetData() {return m_data;}

protected:
	double m_data[4];
};


class BCGCBPRODLLEXPORT CBCGPMatrix4x4  
{
public:
	CBCGPMatrix4x4(BOOL bMakeIdentity = TRUE)
	{
		if (bMakeIdentity)
		{
			Identity();
		}
		else
		{
			Fill(0.);
		}
	}

	CBCGPMatrix4x4(const CBCGPMatrix4x4& mSrc)
	{
		CopyFrom(mSrc);
	}

	void Fill(double dblValue)
	{
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				m_data[i][j] = dblValue;
			}
		}
	}

	void Identity()
	{
		Fill(0.);
		for (int i = 0; i < 4; i++)
		{
			m_data[i][i] = 1.;
		}
	}

	void Inverse();

	double Det(int nOrder)
	{
		if (nOrder < 1 || nOrder > 4)
		{
			return 0;
		}

		if (nOrder == 1)
		{
			return m_data[0][0];
		}

		if (nOrder == 2)
		{
			return m_data[0][0] * m_data[1][1] - m_data[0][1] * m_data[1][0];
		}

		if (nOrder == 3)
		{
			return m_data[0][0] * m_data[1][1] * m_data[2][2] + m_data[0][1] * m_data[1][2] * m_data[2][0] + 
				m_data[0][2] * m_data[1][0] * m_data[2][1] - m_data[0][2] * m_data[1][1] * m_data[2][0] - 
				m_data[0][1] * m_data[1][0] * m_data[2][2] - m_data[0][0] * m_data[1][2] * m_data[2][1];
		}

		if (m_data[0][0] == 0 || m_data[1][1] == 0 || m_data[2][2] == 0)
		{
			return 0;
		}
		
		double mod[4][4];

		// step 1 - subtract form row 1 row 0 multiplied by m10/m00
		memcpy(mod, m_data, sizeof(double) * 16);

		double dblDiv = mod[1][0] / mod[0][0];

		int i = 0;
		for (i = 0; i < 4; i++)
		{
			mod[1][i] -= mod[0][i] * dblDiv;
		}

		// step 2 - subtract from row 2 row 0 multiplied by m20/m00
		dblDiv = mod[2][0] / mod[0][0];

		for (i = 0; i < 4; i++)
		{
			mod[2][i] -= mod[0][i] * dblDiv;
		}

		// step 3 - subtract from row 3 row 0 multiplied by m30/m00
		dblDiv = mod[3][0] / mod[0][0];

		for (i = 0; i < 4; i++)
		{
			mod[3][i] -= mod[0][i] * dblDiv;
		}

		// step 4 - subtract from row 2 row 1 multiplied by m21/m11
		dblDiv = mod[2][1] / mod[1][1];

		for (i = 0; i < 4; i++)
		{
			mod[2][i] -= mod[1][i] * dblDiv;
		}

		//step 5 - subtract from row 3 row 1 multiplied by m31/m11
		dblDiv = mod[3][1] / mod[1][1];

		for (i = 0; i < 4; i++)
		{
			mod[3][i] -= mod[1][i] * dblDiv;
		}

		// step 6 - subtract from row 3 row 2 multiplied by m32/m22
		dblDiv = mod[3][2] / mod[2][2];

		for (i = 0; i < 4; i++)
		{
			mod[3][i] -= mod[2][i] * dblDiv;
		}

		return mod[0][0] * mod[1][1] * mod[2][2] * mod[3][3];
		
	}

	double* operator[](int index)
	{
		ASSERT(0 <= index && index < 4);
		return m_data[index];
	}
	const double* operator[](int index) const
	{
		ASSERT(0 <= index && index < 4);
		return m_data[index];
	}

	double* GetData() const {return (double*)m_data;}

	void CopyFrom(const CBCGPMatrix4x4& mSrc)
	{
		memcpy(m_data, mSrc.GetData(), sizeof(double) * 16);
	}

	void MultiplyMatrixes4x4(const CBCGPMatrix4x4& m1, const CBCGPMatrix4x4& m2);
	void TransformPoint3D(const CBCGPPoint& ptIn, CBCGPPoint& ptOut) const;

protected:
	double m_data[4][4];
};

// finds intersection of two lines defined by 4 points
BOOL BCGPIntersectPoints2D(const CBCGPPoint& ptStart1, const CBCGPPoint& ptEnd1, 
					   const CBCGPPoint& ptStart2, const CBCGPPoint& ptEnd2,
					   CBCGPPoint& ptIntersect);
BOOL BCGPCalculateIntersectPoint(const CBCGPPointsArray& arPoly1, const CBCGPPointsArray& arPoly2, 
						CBCGPPoint& ptIntersect);
BOOL BCGPIntersectTriangle2D(CBCGPPoint arPoly1[], CBCGPPoint arPoly2[], CBCGPPoint arPolyRes[], int& nResCount);

#endif // !defined(__BCGPMATH_H)
