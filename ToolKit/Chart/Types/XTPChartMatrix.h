// XTPChartMatrix.h
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
#if !defined(__XTPCHARTMATRIX_H__)
#define __XTPCHARTMATRIX_H__
//}}AFX_CODEJOCK_PRIVATE

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "XTPChartTypes.h"

//===========================================================================
// Summary:
//     This enumeration specifies the type of the matrix.
//===========================================================================
enum XTPChartMatrixType
{
	xtpChartMatrixModelView,     //The model view matrix.
	xtpChartMatrixProjection     //The projection matrix.
};

//===========================================================================
// Summary:
//     Forward declaration
//===========================================================================
class CXTPChartDiagramPoint;

//===========================================================================
// Summary:
//     This class abstracts a matrix.
//===========================================================================
class _XTP_EXT_CLASS CXTPChartMatrix
{
public:
	//-----------------------------------------------------------------------
	// Summary:
	//     Constructs a CXTPChartMatrix object.
	// Parameters:
	//     type - The type of matrix object.
	// Remarks:
	//     3D chart uses mainly the model view and projection matrices.
	//-----------------------------------------------------------------------
	CXTPChartMatrix(XTPChartMatrixType type = xtpChartMatrixModelView);

public:
	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to get the type of the matrix.
	// Returns:
	//     An enumerated type, XTPChartMatrixType which represents the matrix
	//     type.
	// Remarks:
	//     3D chart uses mainly the model view and projection matrices.
	//-----------------------------------------------------------------------
	XTPChartMatrixType GetType() const;

public:
	//-----------------------------------------------------------------------
	// Summary:
	//     Call this operator function to get the reference of the double value
	//     at an arbitrary index in the matrix.
	// Parameters:
	//     nIndex - The zero based index of matrix element.
	//     type.
	// Remarks:
	//     The matrix is a 1D array of 16 floats.You can use this function to
	//     manipulate the value of the element like matrix[10] = 1.0f.
	//-----------------------------------------------------------------------
	double& operator[](int nIndex);

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this operator function to get the the double value at an
	//     arbitrary index in the matrix.
	// Parameters:
	//     nIndex - The zero based index of matrix element.
	//     type.
	// Remarks:
	//     The matrix is a 1D array of 16 doubles.
	//-----------------------------------------------------------------------
	double operator[](int nIndex) const;

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this operator function to get the the pointer to the 1D double
	//     array which essentially represents a matrix.
	// Returns:
	//     A pointer to double, which is the matrix.
	//     type.
	// Remarks:
	//     The matrix is a 1D array of 16 doubles.
	//-----------------------------------------------------------------------
	double* GetMatrix() const;

	//-----------------------------------------------------------------------
	// Summary:
	//     This operator function adds two matrices.
	// Parameters:
	//     m - The second matrix.
	// Returns:
	//     CXTPChartMatrix which contains the output of the operation.
	// Remarks:
	//-----------------------------------------------------------------------
	CXTPChartMatrix operator+(const CXTPChartMatrix& m) const;

	//-----------------------------------------------------------------------
	// Summary:
	//     This operator function subtracts a matrix from another.
	// Parameters:
	//     m - The second matrix.
	// Returns:
	//     CXTPChartMatrix which contains the output of the operation.
	// Remarks:
	//-----------------------------------------------------------------------
	CXTPChartMatrix operator-(const CXTPChartMatrix& m) const;

	//-----------------------------------------------------------------------
	// Summary:
	//     This operator function multiplies two matrices.
	// Parameters:
	//     m - The second matrix.
	// Returns:
	//     CXTPChartMatrix which contains the output of the operation.
	// Remarks:
	//-----------------------------------------------------------------------
	CXTPChartMatrix operator*(const CXTPChartMatrix& m) const;
	//-----------------------------------------------------------------------
	// Summary:
	//     This operator function multiplies a matrix with a constant.
	// Parameters:
	//     factor - The double value use to multiply the matrix.
	// Returns:
	//     CXTPChartMatrix which contains the output of the operation.
	// Remarks:
	//-----------------------------------------------------------------------
	CXTPChartMatrix operator*(double factor) const;

public:
	static CXTPChartMatrix CalculateVMultVt(const CXTPChartDiagramPoint& p);

public:

	double m_matrix[16];        //The double array which forms the matrix.
	XTPChartMatrixType m_type;   //The type of the matrix.
};

AFX_INLINE XTPChartMatrixType CXTPChartMatrix::GetType() const {
	return m_type;
}

#endif //#if !defined(__XTPCHARTMATRIX_H__)
