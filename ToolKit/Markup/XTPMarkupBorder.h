// XTPMarkupBorder.h: interface for the CXTPMarkupBorder class.
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
#if !defined(__XTPMARKUPBORDER_H__)
#define __XTPMARKUPBORDER_H__
//}}AFX_CODEJOCK_PRIVATE

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "XTPMarkupDecorator.h"

class CXTPMarkupBrush;
class CXTPMarkupBuilder;

//===========================================================================
// Summary: CXTPMarkupBorder is CXTPMarkupDecorator derived class. It implements Border XAML Tag.
//===========================================================================
class _XTP_EXT_CLASS CXTPMarkupBorder : public CXTPMarkupDecorator
{
	DECLARE_MARKUPCLASS(CXTPMarkupBorder);

protected:
	//-----------------------------------------------------------------------
	// Summary:
	//     Constructs a CXTPMarkupBorder object
	//-----------------------------------------------------------------------
	CXTPMarkupBorder();

	//-----------------------------------------------------------------------
	// Summary:
	//     Destroys a CXTPMarkupBorder object, handles cleanup and deallocation
	//-----------------------------------------------------------------------
	virtual ~CXTPMarkupBorder();

public:
	// ---------------------------------------------------------------------
	// Summary:
	//     Call this method to set a value that indicates the thickness of padding space between the boundaries of the content area,
	//     and the content displayed by a Border
	// Parameters
	//     nLeft - Left space
	//     nTop - Top space
	//     nRight - Right space
	//     nBottom - Bottom space
	//     nPadding - All borders
	//-----------------------------------------------------------------------
	void SetPadding(int nLeft, int nTop, int nRight, int nBottom);
	void SetPadding(int nPadding); // <Combine CXTPMarkupBorder::SetPadding@int@int@int@int>

	//-----------------------------------------------------------------------
	// Summary:
	//     Gets a value that indicates the thickness of padding space between the boundaries of the content area,
	//     and the content displayed by a Border
	// Returns:
	//     CXTPMarkupThickness object contains padding space.
	// See Also: SetPadding
	//-----------------------------------------------------------------------
	CXTPMarkupThickness* GetPadding() const;

	// ----------------------------------------------------
	// Summary:
	//     Call this method to set thickness of the Border
	// Parameters
	//     nLeft - Left border width
	//     nTop - Top border width
	//     nRight - Right border width
	//     nBottom - Bottom border width
	//     nBorderThickness - Thickness for all borders
	//-----------------------------------------------------------------------
	void SetBorderThickness(int nLeft, int nTop, int nRight, int nBottom);
	void SetBorderThickness(int nBorderThickness); // <Combine CXTPMarkupBorder::SetBorderThickness@int@int@int@int>


	//-----------------------------------------------------------------------
	// Summary:
	//     Gets a value that indicates the thickness of the Border
	// Returns:
	//     CXTPMarkupThickness object contains border widths
	// See Also: SetBorderThickness
	//-----------------------------------------------------------------------
	CXTPMarkupThickness* GetBorderThickness() const;

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this method to set background of the Border object
	// Parameters: brush - New Brush to be set as background brush
	//-----------------------------------------------------------------------
	void SetBackground(CXTPMarkupBrush* brush);

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this method to get background brush of the Border object
	// Returns: Pointer to CXTPMarkupBrush object contained current brush of Border's object
	//-----------------------------------------------------------------------
	CXTPMarkupBrush* GetBackground() const;

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this method to set border brush for the Border object
	// Parameters: brush - New Brush to be set as border brush
	//-----------------------------------------------------------------------
	void SetBorderBrush(CXTPMarkupBrush* brush);

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this method to get border brush of the Border object
	// Returns: Pointer to CXTPMarkupBrush object contained current brush of border
	//-----------------------------------------------------------------------
	CXTPMarkupBrush* GetBorderBrush() const;

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this method to set corner radisu of the Border
	// Parameters
	//     nCornerRadius - Radius of the corners
	//-----------------------------------------------------------------------
	void SetCornerRadius(int nCornerRadius);

	//-----------------------------------------------------------------------
	// Summary:
	//     Gets a value that indicates the radius of the Border
	// Returns:
	//     CXTPMarkupThickness object contains radius
	// See Also: SetCornerRadius
	//-----------------------------------------------------------------------
	CXTPMarkupThickness* GetCornerRadius() const;

protected:
//{{AFX_CODEJOCK_PRIVATE
	// Implementation
	virtual CSize MeasureOverride(CXTPMarkupDrawingContext* pDC, CSize szAvailableSize);
	virtual CSize ArrangeOverride(CSize szFinalSize);
	virtual void OnRender(CXTPMarkupDrawingContext* drawingContext);
	virtual CXTPMarkupInputElement* InputHitTestOverride(CPoint point) const;

public:
	static CXTPMarkupDependencyProperty* m_pBackgroundProperty;
	static CXTPMarkupDependencyProperty* m_pBorderThicknessProperty;
	static CXTPMarkupDependencyProperty* m_pPaddingProperty;
	static CXTPMarkupDependencyProperty* m_pBorderBrushProperty;
	static CXTPMarkupDependencyProperty* m_pCornerRadiusProperty;
//}}AFX_CODEJOCK_PRIVATE




};

AFX_INLINE void CXTPMarkupBorder::SetPadding(int nLeft, int nTop, int nRight, int nBottom) {
	SetValue(m_pPaddingProperty, new CXTPMarkupThickness(nLeft, nTop, nRight, nBottom));
}
AFX_INLINE void CXTPMarkupBorder::SetPadding(int padding) {
	SetValue(m_pPaddingProperty, new CXTPMarkupThickness(padding));
}
AFX_INLINE CXTPMarkupThickness* CXTPMarkupBorder::GetPadding() const {
	return  MARKUP_STATICCAST(CXTPMarkupThickness, GetValue(m_pPaddingProperty));
}
AFX_INLINE void CXTPMarkupBorder::SetBorderThickness(int nLeft, int nTop, int nRight, int nBottom) {
	SetValue(m_pBorderThicknessProperty, new CXTPMarkupThickness(nLeft, nTop, nRight, nBottom));
}
AFX_INLINE void CXTPMarkupBorder::SetBorderThickness(int padding) {
	SetValue(m_pBorderThicknessProperty, new CXTPMarkupThickness(padding));
}
AFX_INLINE CXTPMarkupThickness* CXTPMarkupBorder::GetBorderThickness() const {
	return  MARKUP_STATICCAST(CXTPMarkupThickness, GetValue(m_pBorderThicknessProperty));
}
AFX_INLINE CXTPMarkupThickness* CXTPMarkupBorder::GetCornerRadius() const {
	return  MARKUP_STATICCAST(CXTPMarkupThickness, GetValue(m_pCornerRadiusProperty));
}
AFX_INLINE void CXTPMarkupBorder::SetCornerRadius(int nCornerRadius) {
	SetValue(m_pCornerRadiusProperty, new CXTPMarkupThickness(nCornerRadius));

}


#endif // !defined(__XTPMARKUPBORDER_H__)
