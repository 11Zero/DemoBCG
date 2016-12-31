// XTPChartTypes.h
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
#if !defined(__XTPCHARTTYPES_H__)
#define __XTPCHARTTYPES_H__
//}}AFX_CODEJOCK_PRIVATE

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#define float_EPSILON 1.192092896e-07F

class CXTPPropExchange;

//===========================================================================
// Summary:
//     This class abstracts a size entity and its operations.This object composed
//     of two float values width and height.
// Remarks:
//===========================================================================
class _XTP_EXT_CLASS CXTPChartSizeF
{
public:
	//-----------------------------------------------------------------------
	// Summary:
	//     Default constructor, creates a CXTPChartSizeF object and initializes
	//     the values.
	// Remarks:
	//-----------------------------------------------------------------------
	CXTPChartSizeF()
	{
		Width = Height = 0.0f;
	}

	//-----------------------------------------------------------------------
	// Summary:
	//     Copy constructor, creates a CXTPChartSizeF object and initializes
	//     the values.
	// Parameters:
	//     size - A reference to a CXTPChartSizeF object.
	// Remarks:
	//-----------------------------------------------------------------------
	CXTPChartSizeF(const CXTPChartSizeF& size)
	{
		Width = size.Width;
		Height = size.Height;
	}

	//-----------------------------------------------------------------------
	// Summary:
	//     Overloaded constructor, creates a CXTPChartSizeF object and initializes
	//     the values.
	// Parameters:
	//     width - A float(float) value for width.
	//     height - A float value for height.
	// Remarks:
	//-----------------------------------------------------------------------
	CXTPChartSizeF(float width, float height)
	{
		Width = width;
		Height = height;
	}

	//-----------------------------------------------------------------------
	// Summary:
	//     Overloaded + operator, adds two CXTPChartSizeF objects.
	//     the values.
	// Parameters:
	//     sz - A reference to a CXTPChartSizeF object.
	// Returns:
	//     A CXTPChartPointF object which contain the result of operation.
	// Remarks:
	//     It is a constant member function.
	//-----------------------------------------------------------------------
	CXTPChartSizeF operator+(const CXTPChartSizeF& sz) const
	{
		return CXTPChartSizeF(Width + sz.Width,
			Height + sz.Height);
	}

	//-----------------------------------------------------------------------
	// Summary:
	//     Overloaded - operator, subtracts two CXTPChartSizeF objects.
	//     the values.
	// Parameters:
	//     sz - A reference to a CXTPChartSizeF object.
	// Returns:
	//     A CXTPChartPointF object which contain the result of operation.
	// Remarks:
	//     It is a constant member function.
	//-----------------------------------------------------------------------
	CXTPChartSizeF operator-(const CXTPChartSizeF& sz) const
	{
		return CXTPChartSizeF(Width - sz.Width,
			Height - sz.Height);
	}
	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to know whether the two CXTPChartSizeF objects are
	//     equal.
	// Parameters:
	//     sz - A reference to a CXTPChartSizeF object.
	// Returns:
	//     TRUE if the objects are equal and FALSE if they are unequal.
	// Remarks:
	//     It is a constant member function.
	//-----------------------------------------------------------------------
	BOOL Equals(const CXTPChartSizeF& sz) const
	{
		return (Width == sz.Width) && (Height == sz.Height);
	}

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to check whether the object is empty or its
	//     height and width are zero.
	// Returns:
	//     TRUE if the objects is empty and FALSE if it contains some value other
	//     than zero.
	// Remarks:
	//     It is a constant member function.
	//-----------------------------------------------------------------------
	BOOL Empty() const
	{
		return (Width == 0.0f && Height == 0.0f);
	}

public:

	float Width;     //The width.
	float Height;    //The height.
};

//===========================================================================
// Summary:
//     This class abstracts a Point entity and its operations.This object composed
//     of two float values for x and y coordinates.
// Remarks:
//===========================================================================
class _XTP_EXT_CLASS CXTPChartPointF
{
public:
	//-----------------------------------------------------------------------
	// Summary:
	//     Default constructor, creates a CXTPChartPointF object and initializes
	//     the values.
	// Remarks:
	//-----------------------------------------------------------------------
	CXTPChartPointF()
	{
		X = Y = 0.0f;
	}

	//-----------------------------------------------------------------------
	// Summary:
	//     Copy constructor, creates a CXTPChartPointF object and initializes
	//     the values.
	// Parameters:
	//     size - A reference to a CXTPChartPointF object.
	// Remarks:
	//-----------------------------------------------------------------------
	CXTPChartPointF(const CXTPChartPointF &point)
	{
		X = point.X;
		Y = point.Y;
	}

	//-----------------------------------------------------------------------
	// Summary:
	//     Overloaded constructor, creates a CXTPChartPointF object from a chart
	//     size type object and initializes the values.
	// Parameters:
	//     size - A reference to a CXTPChartSizeF.
	// Remarks:
	//-----------------------------------------------------------------------
	CXTPChartPointF(const CXTPChartSizeF &size)
	{
		X = size.Width;
		Y = size.Height;
	}

	//-----------------------------------------------------------------------
	// Summary:
	//     Overloaded constructor, creates a CXTPChartPointF object.
	// Parameters:
	//     x - A float(float) value denoting the x coordinate.
	//     y - A float(float) value denoting the y coordinate.
	// Remarks:
	//-----------------------------------------------------------------------
	CXTPChartPointF(double x, double y)
	{
		X = (float)x;
		Y = (float)y;
	}

	//-----------------------------------------------------------------------
	// Summary:
	//     Overloaded constructor, creates a CXTPChartPointF object.
	// Parameters:
	//     p - A POINT structure contains values for x and y coordinates.
	// Remarks:
	//-----------------------------------------------------------------------
	CXTPChartPointF(const POINT& p)
	{
		X = (float)p.x;
		Y = (float)p.y;
	}

	//-----------------------------------------------------------------------
	// Summary:
	//     Overloaded + operator, adds two CXTPChartPointF objects.
	//     the values.
	// Parameters:
	//     point - A reference to a CXTPChartPointF object.
	// Returns:
	//     A CXTPChartPointF object which contain the result of operation.
	// Remarks:
	//     It is a constant member function.
	//-----------------------------------------------------------------------
	CXTPChartPointF operator+(const CXTPChartPointF& point) const
	{
		return CXTPChartPointF(X + point.X,
			Y + point.Y);
	}

	void operator+=(const CXTPChartPointF& point)
	{
		X += point.X;
		Y += point.Y;
	}

	//-----------------------------------------------------------------------
	// Summary:
	//     Overloaded - operator, subtracts two CXTPChartPointF objects.
	//     the values.
	// Parameters:
	//     sz - A reference to a CXTPChartPointF object.
	// Returns:
	//     A CXTPChartPointF object which contain the result of operation.
	// Remarks:
	//     It is a constant member function.
	//-----------------------------------------------------------------------
	CXTPChartPointF operator-(const CXTPChartPointF& point) const
	{
		return CXTPChartPointF(X - point.X,
			Y - point.Y);
	}
	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to know whether the two CXTPChartPointF objects are
	//     equal.
	// Parameters:
	//     point - A reference to a CXTPChartPointF object.
	// Returns:
	//     TRUE if the objects are equal and FALSE if they are unequal.
	// Remarks:
	//     It is a constant member function.
	//-----------------------------------------------------------------------
	BOOL Equals(const CXTPChartPointF& point)
	{
		return (X == point.X) && (Y == point.Y);
	}

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to round the point values to nearest integer.
	// Returns:
	//     A CPoint object which contain integer values for x and y coordinates.
	// Remarks:
	//     It is a constant member function.
	//-----------------------------------------------------------------------

	CPoint Round() const
	{
		return CPoint((int)X, (int)Y);
	}

public:

	float X;     //The x coordinate.
	float Y;     //The y coordinate.
};

typedef CArray<CXTPChartPointF, const CXTPChartPointF&> CXTPChartPoints;
//===========================================================================
// Summary:
//     This class abstracts a rectangular entity and its operations.This object composed
//     of four float values for x and y coordinates and width and height.
// Remarks:
//===========================================================================
class _XTP_EXT_CLASS CXTPChartRectF
{
public:
	//-----------------------------------------------------------------------
	// Summary:
	//     Default constructor, creates a CXTPChartRectF object and initializes
	//     the values.
	// Remarks:
	//-----------------------------------------------------------------------
	CXTPChartRectF()
	{
		X = Y = Width = Height = 0.0f;
	}

	//-----------------------------------------------------------------------
	// Summary:
	//     Overloaded constructor, creates a CXTPChartRectF object from
	//     CRect type object and initializes the values.
	// Parameters:
	//     rc - A reference to a CRect.
	// Remarks:
	//-----------------------------------------------------------------------
	CXTPChartRectF(const CRect& rc)
	{
		X = (float)rc.left;
		Y = (float)rc.top;
		Width = (float)rc.Width();
		Height = (float)rc.Height();
	}

	//-----------------------------------------------------------------------
	// Summary:
	//     Overloaded constructor, creates a CXTPChartRectF object.
	// Parameters:
	//     x      - The x coordinate.
	//     y      - The y coordinate.
	//     width  - The width of the rectangle.
	//     height - The height of the rectangle.
	// Remarks:
	//-----------------------------------------------------------------------
	CXTPChartRectF(float x, float y, float width, float height)
	{
		X = x;
		Y = y;
		Width = width;
		Height = height;
	}

	//-----------------------------------------------------------------------
	// Summary:
	//     Overloaded constructor, creates a CXTPChartRectF object from chart point
	//     and chart size objects.
	// Parameters:
	//     location - The rectangle location.
	//     size     - The size of the rectangle.
	// Remarks:
	//-----------------------------------------------------------------------
	CXTPChartRectF(const CXTPChartPointF& location,
		  const CXTPChartSizeF& size)
	{
		X = location.X;
		Y = location.Y;
		Width = size.Width;
		Height = size.Height;
	}

	CXTPChartRectF(const CXTPChartPointF& ptTopLeft, const CXTPChartPointF& ptBottomRight)
	{
		X = ptTopLeft.X;
		Y = ptTopLeft.Y;
		Width = ptBottomRight.X - ptTopLeft.X;
		Height = ptBottomRight.Y - ptTopLeft.Y;

		Normalize();
	}

	void Normalize()
	{
		if (Width < 0)
		{
			X += Width;
			Width = -Width;
		}

		if (Height < 0)
		{
			Y += Height;
			Height = -Height;
		}
	}

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to get the coordinate location of the rectangle.
	// Returns:
	//     A CXTPChartPointF object denoting the current location of the rectangle
	//     in a plane.
	// Remarks:
	//-----------------------------------------------------------------------
	CXTPChartPointF GetLocation() const
	{
		return CXTPChartPointF(X, Y);
	}

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to get the size of the rectangle.
	// Returns:
	//     A CXTPChartSizeF object denoting size of the rectangle.
	// Remarks:
	//-----------------------------------------------------------------------
	CXTPChartSizeF GetSize() const
	{
		return CXTPChartSizeF(Width, Height);
	}

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to get the starting x coordinate of the rectangle.
	// Returns:
	//     A float value denoting the left of the rectangle.
	// Remarks:
	//-----------------------------------------------------------------------
	float GetLeft() const
	{
		return X;
	}

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to get the starting y coordinate of the rectangle.
	// Returns:
	//     A float value denoting the top of the rectangle.
	// Remarks:
	//-----------------------------------------------------------------------
	float GetTop() const
	{
		return Y;
	}

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to get the ending x coordinate of the rectangle.
	// Returns:
	//     A float value denoting the right of the rectangle.
	// Remarks:
	//-----------------------------------------------------------------------
	float GetRight() const
	{
		return X+Width;
	}

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to get the ending y coordinate of the rectangle.
	// Returns:
	//     A float value denoting the bottom of the rectangle.
	// Remarks:
	//-----------------------------------------------------------------------
	float GetBottom() const
	{
		return Y+Height;
	}

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to check whether the rectangular area is epty.
	// Returns:
	//     TRUE if the rectangle is of finite area and FALSE if it is empty.
	// Remarks:
	//-----------------------------------------------------------------------
	BOOL IsEmptyArea() const
	{
		return (Width <= float_EPSILON) || (Height <= float_EPSILON);
	}

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to check whether the current rectangle is equal
	//     to another.
	// Parameters:
	//     rect - The rectangle which is to be compared with the current object
	//     for equality.
	// Returns:
	//     TRUE if the second rectangle is equal to the current object and FALSE
	//     they are unequal.
	// Remarks:
	//-----------------------------------------------------------------------
	BOOL Equals(const CXTPChartRectF & rect) const
	{
		return X == rect.X &&
			   Y == rect.Y &&
			   Width == rect.Width &&
			   Height == rect.Height;
	}

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to check whether a point is inside the rectangle
	//     or not.
	// Parameters:
	//     x - The x coordinate of the point.
	//     y - The y coordinate of the point.
	// Returns:
	//     TRUE if the point is inside the rectangle and FALSE if the point is
	//     outside the rectangle.
	// Remarks:
	//-----------------------------------------------------------------------
	BOOL Contains(float x,
				  float y) const
	{
		return x >= X && x < X+Width &&
			   y >= Y && y < Y+Height;
	}

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to check whether a point is inside the rectangle
	//     or not.
	// Parameters:
	//     pt - The point.
	// Returns:
	//     TRUE if the point is inside the rectangle and FALSE if the point is
	//     outside the rectangle.
	// Remarks:
	//-----------------------------------------------------------------------
	BOOL Contains(const CXTPChartPointF& pt) const
	{
		return Contains(pt.X, pt.Y);
	}

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to check whether a rectangle is inside the rectangle
	//     or not.
	// Parameters:
	//     pt - The rectangle.
	// Returns:
	//     TRUE if the second rectangle is inside the current rectangle and FALSE if
	//     the not.
	//
	// Remarks:
	//-----------------------------------------------------------------------
	BOOL Contains(const CXTPChartRectF& rect) const
	{
		return (X <= rect.X) && (rect.GetRight() <= GetRight()) &&
			   (Y <= rect.Y) && (rect.GetBottom() <= GetBottom());
	}

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to inflate the rectangle by an arbitrary value.
	// Parameters:
	//     dx - The change in x value.
	//     dy - The change in y value.
	// Remarks:
	//-----------------------------------------------------------------------
	VOID Inflate(float dx,
				 float dy)
	{
		X -= dx;
		Y -= dy;
		Width += 2*dx;
		Height += 2*dy;
	}

	void DeflateRect(float l, float t, float r, float b)
	{
		X += l;
		Y += t;
		Width -= l + r;
		Height -= t + b;
	}

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to inflate the rectangle by an arbitrary value
	//     denoted by a CXTPChartPointF object.
	// Parameters:
	//     point - The change value.
	// Remarks:
	//-----------------------------------------------------------------------
	VOID Inflate(const CXTPChartPointF& point)
	{
		Inflate(point.X, point.Y);
	}

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function calculates the intersection, the current object
	//     and another rectangle and places the coordinates of the intersection
	//     rectangle into the current object.
	// Parameters:
	//     rect - The other rectangle.
	// Returns:
	//     TRUE if the operation succeed and FALSE if operation failed.
	// Remarks:
	//-----------------------------------------------------------------------
	BOOL Intersect(const CXTPChartRectF& rect)
	{
		return Intersect(*this, *this, rect);
	}

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this static function to calculates the intersection of two
	//     source rectangles and places the coordinates of the intersection
	//     rectangle into the destination rectangle.
	// Parameters:
	//     c - The reference to destination rectangle.
	//     a - The first source rectangle.
	//     b - The second source rectangle.
	// Returns:
	//     TRUE if the operation succeed and FALSE if operation failed.
	// Remarks:
	//-----------------------------------------------------------------------
	static BOOL Intersect(OUT CXTPChartRectF& c,
						  const CXTPChartRectF& a,
						  const CXTPChartRectF& b)
	{
		float right = min(a.GetRight(), b.GetRight());
		float bottom = min(a.GetBottom(), b.GetBottom());
		float left = max(a.GetLeft(), b.GetLeft());
		float top = max(a.GetTop(), b.GetTop());

		c.X = left;
		c.Y = top;
		c.Width = right - left;
		c.Height = bottom - top;
		return !c.IsEmptyArea();
	}

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to check whether this object intersect with a given
	//     rectangle.
	// Parameters:
	//     rect - The reference to the other rectangle.
	// Returns:
	//     TRUE if the rectangles intersect and FALSE if rectangles do not intersect.
	// Remarks:
	//-----------------------------------------------------------------------
	BOOL IntersectsWith(const CXTPChartRectF& rect) const
	{
		return (GetLeft() < rect.GetRight() &&
				GetTop() < rect.GetBottom() &&
				GetRight() > rect.GetLeft() &&
				GetBottom() > rect.GetTop());
	}

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this static function to calculates the union of two
	//     source rectangles and places the coordinates of the union
	//     rectangle into the destination rectangle.
	// Parameters:
	//     c - The reference to destination rectangle.
	//     a - The first source rectangle.
	//     b - The second source rectangle.
	// Returns:
	//     TRUE if the operation succeed and FALSE if operation failed.
	// Remarks:
	//     The union is the smallest rectangle that contains both source rectangles.
	//-----------------------------------------------------------------------
	static BOOL Union(OUT CXTPChartRectF& c, const CXTPChartRectF& a, const CXTPChartRectF& b)
	{
		float right = max(a.GetRight(), b.GetRight());
		float bottom = max(a.GetBottom(), b.GetBottom());
		float left = min(a.GetLeft(), b.GetLeft());
		float top = min(a.GetTop(), b.GetTop());

		c.X = left;
		c.Y = top;
		c.Width = right - left;
		c.Height = bottom - top;
		return !c.IsEmptyArea();
	}

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to move the rectangle by an offset.
	// Parameters:
	//     point - The offset as a point.
	// Remarks:
	//-----------------------------------------------------------------------
	VOID Offset(const CXTPChartPointF& point)
	{
		Offset(point.X, point.Y);
	}
	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to move the rectangle by an offset.
	// Parameters:
	//     dx - The offset x value.
	//     dy - The offset y value.
	// Remarks:
	//-----------------------------------------------------------------------
	VOID Offset(float dx,
				float dy)
	{
		X += dx;
		Y += dy;
	}

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to get the center point of the rectangle.
	// Returns:
	//     A CXTPChartPointF object representing the center of the rectangle.
	// Remarks:
	//-----------------------------------------------------------------------
	CXTPChartPointF GetCenter()
	{
		return CXTPChartPointF(X + Width / 2.0f, Y + Height / 2.0f);
	}

	void Round();

public:

	float X;         //The x coordinate(left).
	float Y;         //The y coordinate(top).
	float Width;     //The width of the rectangle.
	float Height;    //The height of the rectangle.
};


typedef CString CXTPChartString;

//===========================================================================
// Summary:
//     This class abstracts a color entity.This object consists of an ARGB
//     value which represents a color.
// Remarks:
//===========================================================================
class _XTP_EXT_CLASS CXTPChartColor
{
public:
	typedef DWORD ARGB;

public:

	//-----------------------------------------------------------------------
	// Summary:
	//     Default constructor, creates a CXTPChartColor object and initializes
	//     the values.
	// Remarks:
	//-----------------------------------------------------------------------
	CXTPChartColor()
	{
		Argb = CXTPChartColor::Empty;
	}

	//-----------------------------------------------------------------------
	// Summary:
	//     Overloaded constructor, creates an opaque color object from the
	//     specified Red, Green, Blue values.
	// Parameters:
	//     r - Red value.
	//     g - Green value.
	//     b - Blue value.
	// Remarks:
	//     The alpha channel value is set to 255 hence the color is 100% opaque
	//     Color values are not premultiplied.
	//-----------------------------------------------------------------------
	CXTPChartColor(BYTE r, BYTE g, BYTE b)
	{
		Argb = MakeARGB(255, r, g, b);
	}

	//-----------------------------------------------------------------------
	// Summary:
	//     Overloaded constructor, creates a color object from the specified
	//     Red, Green, Blue values.
	// Parameters:
	//     r - Red value.
	//     g - Green value.
	//     b - Blue value.
	// Remarks:
	//     Color values are not premultiplied.
	//-----------------------------------------------------------------------
	CXTPChartColor(BYTE a, BYTE r, BYTE g, BYTE b)
	{
		Argb = MakeARGB(a, r, g, b);
	}

	//-----------------------------------------------------------------------
	// Summary:
	//     Overloaded constructor, creates a color object from the specified
	//     premultiplied ARGB values.
	// Parameters:
	//     argb - The ARGB value.
	// Remarks:
	//-----------------------------------------------------------------------
	CXTPChartColor(ARGB argb)
	{
		Argb = argb;
	}

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to get the alpha channel value of the color.
	// Returns:
	//     A BYTE value specifying the alpha value, the range is from
	//     0 to 255.
	// Remarks:
	//-----------------------------------------------------------------------
	BYTE GetAlpha() const
	{
		return (BYTE) (Argb >> AlphaShift);
	}

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to get the alpha channel value of the color.
	// Returns:
	//     A BYTE value specifying the alpha value, the range is from
	//     0 to 255.
	// Remarks:
	//-----------------------------------------------------------------------
	BYTE GetA() const
	{
		return GetAlpha();
	}

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to get the RED channel value of the color.
	// Returns:
	//     A BYTE value specifying the RED value, the range is from
	//     0 to 255.
	// Remarks:
	//-----------------------------------------------------------------------
	BYTE GetRed() const
	{
		return (BYTE) (Argb >> RedShift);
	}

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to get the RED channel value of the color.
	// Returns:
	//     A BYTE value specifying the RED value, the range is from
	//     0 to 255.
	// Remarks:
	//-----------------------------------------------------------------------
	BYTE GetR() const
	{
		return GetRed();
	}

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to get the GREEN channel value of the color.
	// Returns:
	//     A BYTE value specifying the GREEN value, the range is from
	//     0 to 255.
	// Remarks:
	//-----------------------------------------------------------------------
	BYTE GetGreen() const
	{
		return (BYTE) (Argb >> GreenShift);
	}

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to get the GREEN channel value of the color.
	// Returns:
	//     A BYTE value specifying the GREEN value, the range is from
	//     0 to 255.
	// Remarks:
	//-----------------------------------------------------------------------
	BYTE GetG() const
	{
		return GetGreen();
	}

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to get the BLUE channel value of the color.
	// Returns:
	//     A BYTE value specifying the BLUE value, the range is from
	//     0 to 255.
	// Remarks:
	//-----------------------------------------------------------------------
	BYTE GetBlue() const
	{
		return (BYTE) (Argb >> BlueShift);
	}

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to get the BLUE channel value of the color.
	// Returns:
	//     A BYTE value specifying the BLUE value, the range is from
	//     0 to 255.
	// Remarks:
	//-----------------------------------------------------------------------
	BYTE GetB() const
	{
		return GetBlue();
	}

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to get the ARGB value of the color object.
	// Returns:
	//     A ARGB value specifying the color.
	// Remarks:
	//-----------------------------------------------------------------------
	ARGB GetValue() const
	{
		return Argb;
	}

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to set the ARGB value of the color object.
	// Parameters:
	//     argb - A ARGB value specifying the color.
	// Remarks:
	//-----------------------------------------------------------------------
	VOID SetValue(ARGB argb)
	{
		Argb = argb;
	}

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to set the opaque color value from a COLORREF value.
	// Parameters:
	//     rgb - A RGB value specifying the color.
	//     The alpha channel value is set to 255 hence the color is 100% opaque
	// Remarks:
	//-----------------------------------------------------------------------
	VOID SetFromCOLORREF(COLORREF rgb)
	{
		Argb = MakeARGB(255, GetRValue(rgb), GetGValue(rgb), GetBValue(rgb));
	}

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to set the color value from a string value.
	// Parameters:
	//     lpsz - The string contains the color values in a specified format.
	// Remarks:
	//     The format should be "100,100,100,100" for ARGB values
	//     or "100,100,100" for opaque color values
	//-----------------------------------------------------------------------
	void SetFromString(LPCTSTR lpsz);

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to convert the object to a COLORREF value.
	// Returns:
	//     A ARGB value specifying the color.
	// Remarks:
	//-----------------------------------------------------------------------
	COLORREF ToCOLORREF() const
	{
		return RGB(GetRed(), GetGreen(), GetBlue());
	}

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to check whether the color object is empty.
	// Returns:
	//     A BOOL value of TRUE if the object is empty and FALSE if the object
	//     is not empty.
	// Remarks:
	//-----------------------------------------------------------------------
	BOOL IsEmpty() const
	{
		return Argb == Empty;
	}

	CXTPChartColor GetDarkColor() const;

public:

	// Common color constants

	const static ARGB AliceBlue;
	const static ARGB AntiqueWhite;
	const static ARGB Aqua;
	const static ARGB Aquamarine;
	const static ARGB Azure;
	const static ARGB Beige;
	const static ARGB Bisque;
	const static ARGB Black;
	const static ARGB BlanchedAlmond;
	const static ARGB Blue;
	const static ARGB BlueViolet;
	const static ARGB Brown;
	const static ARGB BurlyWood;
	const static ARGB CadetBlue;
	const static ARGB Chartreuse;
	const static ARGB Chocolate;
	const static ARGB Coral;
	const static ARGB CornflowerBlue;
	const static ARGB Cornsilk;
	const static ARGB Crimson;
	const static ARGB Cyan;
	const static ARGB DarkBlue;
	const static ARGB DarkCyan;
	const static ARGB DarkGoldenrod;
	const static ARGB DarkGray;
	const static ARGB DarkGreen;
	const static ARGB DarkKhaki;
	const static ARGB DarkMagenta;
	const static ARGB DarkOliveGreen;
	const static ARGB DarkOrange;
	const static ARGB DarkOrchid;
	const static ARGB DarkRed;
	const static ARGB DarkSalmon;
	const static ARGB DarkSeaGreen;
	const static ARGB DarkSlateBlue;
	const static ARGB DarkSlateGray;
	const static ARGB DarkTurquoise;
	const static ARGB DarkViolet;
	const static ARGB DeepPink;
	const static ARGB DeepSkyBlue;
	const static ARGB DimGray;
	const static ARGB DodgerBlue;
	const static ARGB Firebrick;
	const static ARGB FloralWhite;
	const static ARGB ForestGreen;
	const static ARGB Fuchsia;
	const static ARGB Gainsboro;
	const static ARGB GhostWhite;
	const static ARGB Gold;
	const static ARGB Goldenrod;
	const static ARGB Gray;
	const static ARGB Green;
	const static ARGB GreenYellow;
	const static ARGB Honeydew;
	const static ARGB HotPink;
	const static ARGB IndianRed;
	const static ARGB Indigo;
	const static ARGB Ivory;
	const static ARGB Khaki;
	const static ARGB Lavender;
	const static ARGB LavenderBlush;
	const static ARGB LawnGreen;
	const static ARGB LemonChiffon;
	const static ARGB LightBlue;
	const static ARGB LightCoral;
	const static ARGB LightCyan;
	const static ARGB LightGoldenrodYellow;
	const static ARGB LightGray;
	const static ARGB LightGreen;
	const static ARGB LightPink;
	const static ARGB LightSalmon;
	const static ARGB LightSeaGreen;
	const static ARGB LightSkyBlue;
	const static ARGB LightSlateGray;
	const static ARGB LightSteelBlue;
	const static ARGB LightYellow;
	const static ARGB Lime;
	const static ARGB LimeGreen;
	const static ARGB Linen;
	const static ARGB Magenta;
	const static ARGB Maroon;
	const static ARGB MediumAquamarine;
	const static ARGB MediumBlue;
	const static ARGB MediumOrchid;
	const static ARGB MediumPurple;
	const static ARGB MediumSeaGreen;
	const static ARGB MediumSlateBlue;
	const static ARGB MediumSpringGreen;
	const static ARGB MediumTurquoise;
	const static ARGB MediumVioletRed;
	const static ARGB MidnightBlue;
	const static ARGB MintCream;
	const static ARGB MistyRose;
	const static ARGB Moccas;
	const static ARGB NavajoWhite;
	const static ARGB Navy;
	const static ARGB OldLace;
	const static ARGB Olive;
	const static ARGB OliveDrab;
	const static ARGB Orange;
	const static ARGB OrangeRed;
	const static ARGB Orchid;
	const static ARGB PaleGoldenrod;
	const static ARGB PaleGreen;
	const static ARGB PaleTurquoise;
	const static ARGB PaleVioletRed;
	const static ARGB PapayaWhip;
	const static ARGB PeachPuff;
	const static ARGB Peru;
	const static ARGB Pink;
	const static ARGB Plum;
	const static ARGB PowderBlue;
	const static ARGB Purple;
	const static ARGB Red;
	const static ARGB RosyBrown;
	const static ARGB RoyalBlue;
	const static ARGB SaddleBrown;
	const static ARGB Salmon;
	const static ARGB SandyBrown;
	const static ARGB SeaGreen;
	const static ARGB SeaShell;
	const static ARGB Sienna;
	const static ARGB Silver;
	const static ARGB SkyBlue;
	const static ARGB SlateBlue;
	const static ARGB SlateGray;
	const static ARGB Snow;
	const static ARGB SpringGreen;
	const static ARGB SteelBlue;
	const static ARGB Tan;
	const static ARGB Teal;
	const static ARGB Thistle;
	const static ARGB Tomato;
	const static ARGB Transparent;
	const static ARGB Turquoise;
	const static ARGB Violet;
	const static ARGB Wheat;
	const static ARGB White;
	const static ARGB WhiteSmoke;
	const static ARGB Yellow;
	const static ARGB YellowGreen;
	const static ARGB Empty;

	//===========================================================================
	// Summary:
	//     Enumeration for shift count of A, R, G, B components
	// Remarks:
	//===========================================================================
	enum
	{
		AlphaShift  = 24,
		RedShift    = 16,
		GreenShift  = 8,
		BlueShift   = 0
	};
	//===========================================================================
	// Summary:
	//     Enumeration for bit mask of A, R, G, B components
	// Remarks:
	//===========================================================================
	enum
	{
		AlphaMask   = 0xff000000,
		RedMask     = 0x00ff0000,
		GreenMask   = 0x0000ff00,
		BlueMask    = 0x000000ff
	};

	// Assemble A, R, G, B values into a 32-bit integer
	//===========================================================================
	// Summary:
	//     Call this static function to assemble A, R, G, B values into a 32-bit
	//     integer ARGB value.
	// Parameters:
	//     a - The Alpha value.
	//     r - The Red value
	//     g - The Green value
	//     b - The Blue value
	// Returns:
	//     A 32 bit ARGB value.
	// Remarks:
	//===========================================================================
	static ARGB MakeARGB(BYTE a,
						 BYTE r,
						 BYTE g,
						 BYTE b)
	{
		return (((ARGB) (b) <<  BlueShift) |
				((ARGB) (g) << GreenShift) |
				((ARGB) (r) <<   RedShift) |
				((ARGB) (a) << AlphaShift));
	}

protected:

	ARGB Argb;  //The 32 bit color value.

#ifdef _XTP_ACTIVEX
//{{AFX_CODEJOCK_PRIVATE
public:
	OLE_COLOR ToOleColor();
	static CXTPChartColor AFX_CDECL FromOleColor(OLE_COLOR);
//}}AFX_CODEJOCK_PRIVATE
#endif
};

//===========================================================================
// Summary:
//     This class abstracts a font entity.
// Remarks:
//===========================================================================
class _XTP_EXT_CLASS CXTPChartFont : public CXTPCmdTarget
{
public:
	//-----------------------------------------------------------------------
	// Summary:
	//     Default constructor, creates a CXTPChartFont object and initializes
	//     the values.
	// Remarks:
	//-----------------------------------------------------------------------
	CXTPChartFont();

	//-----------------------------------------------------------------------
	// Summary:
	//     Overloaded constructor, creates a font object from the LOGFONT structure.
	// Parameters:
	//     pLogFont - A pointer to the LOGFONT structure.
	// Remarks:
	//-----------------------------------------------------------------------
	CXTPChartFont(LOGFONT* pLogFont);

	//-----------------------------------------------------------------------
	// Summary:
	//     Destructor, does the cleaning.
	// Remarks:
	//-----------------------------------------------------------------------
	virtual ~CXTPChartFont();

public:
	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to set the LOGFONT for this object.
	// Parameters:
	//     lf - A pointer to the LOGFONT structure.
	// Remarks:
	//-----------------------------------------------------------------------
	void SetLogFont(LOGFONT* lf);

public:
	//-----------------------------------------------------------------------
	// Summary:
	//     Call this static function to get Tahoma font with size 18.
	// Returns:
	//     A pointer to the CXTPChartFont object.
	// Remarks:
	//     This function create a new CXTPChartFont and set the underlying font to
	//     Tahoma with size 18.
	//-----------------------------------------------------------------------
	static CXTPChartFont* AFX_CDECL GetTahoma18();

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this static function to get Tahoma font with size 12.
	// Returns:
	//     A pointer to the CXTPChartFont object.
	// Remarks:
	//     This function create a new CXTPChartFont and set the underlying font to
	//     Tahoma with size 12.
	//-----------------------------------------------------------------------
	static CXTPChartFont* AFX_CDECL GetTahoma12();

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this static function to get Tahoma font with size 8.
	// Returns:
	//     A pointer to the CXTPChartFont object.
	// Remarks:
	//     This function create a new CXTPChartFont and set the underlying font to
	//     Tahoma with size 8.
	//-----------------------------------------------------------------------
	static CXTPChartFont* AFX_CDECL GetTahoma8();

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to do the clean up.
	//-----------------------------------------------------------------------
	void Release();


#ifdef _XTP_ACTIVEX
public:
//{{AFX_CODEJOCK_PRIVATE
	DECLARE_DISPATCH_MAP()
	DECLARE_INTERFACE_MAP()
	DECLARE_OLETYPELIB_EX(CXTPChartFont);
	afx_msg void OleSetFont(LPFONTDISP pFontDisp);
	afx_msg LPFONTDISP OleGetFont();
//}}AFX_CODEJOCK_PRIVATE
#endif

public:
	LOGFONT m_lf;   //The LOGFONT variable.
};

BOOL AFX_CDECL PX_Color(CXTPPropExchange* pPX, LPCTSTR pszPropName, CXTPChartColor& clrValue);
BOOL AFX_CDECL PX_Font(CXTPPropExchange* pPX, LPCTSTR pszPropName, CXTPChartFont* pFont);

#endif //#if !defined(__XTPCHARTTYPES_H__)
