// XTPSplitterWndTheme.h: interface for the CXTPSplitterWndTheme class.
//
// This file is a part of the XTREME CONTROLS MFC class library.
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
#if !defined(__XTPSPLITTERTHEME_H__)
#define __XTPSPLITTERTHEME_H__
//}}AFX_CODEJOCK_PRIVATE

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CXTPSplitterWnd;

//===========================================================================
// Summary:
//     Class CXTPSplitterWndTheme is derived from CXTPControlTheme.
//     This class is used to apply a Theme to splitter windows.
//===========================================================================
class _XTP_EXT_CLASS CXTPSplitterWndTheme : public CXTPControlTheme
{
public:
	//-----------------------------------------------------------------------
	// Summary:
	//     Construct a CXTPSplitterWndTheme object.
	//-----------------------------------------------------------------------
	CXTPSplitterWndTheme();

	//-------------------------------------------------------------------------
	// Summary:
	//     Call this function to set the splitter windows face and the
	//     borders to the system default colors.
	// Parameters:
	//     pSplitter - Points to the CXTPSplitterWnd object.
	//-------------------------------------------------------------------------
	virtual void RefreshMetrics(CXTPSplitterWnd* pSplitter);

public:
	CXTPPaintManagerColor m_clrSplitterFace;     // The color of the splitter.
	CXTPPaintManagerColor m_clrSplitterBorders;  // The color of the splitter borders.
};

//===========================================================================
// Summary:
//     Class CXTPSplitterWndThemeOfficeXP is derived from CXTPSplitterWndTheme.
//     This class is used to implement the Office 2003 theme for splitter
//     windows.
//===========================================================================
class _XTP_EXT_CLASS CXTPSplitterWndThemeOfficeXP : public CXTPSplitterWndTheme
{
public:
	//-----------------------------------------------------------------------
	// Summary:
	//     Construct a CXTPSplitterWndThemeOfficeXP object.
	//-----------------------------------------------------------------------
	CXTPSplitterWndThemeOfficeXP();

	//-------------------------------------------------------------------------
	// Summary:
	//     Call this function to set the splitter windows face and the
	//     borders to the system default colors.
	// Parameters:
	//     pSplitter - Points to the CXTPSplitterWnd object.
	//-------------------------------------------------------------------------
	virtual void RefreshMetrics(CXTPSplitterWnd* pSplitter);
};

//===========================================================================
// Summary:
//     Class CXTPSplitterWndThemeOffice2003 is derived from CXTPSplitterWndTheme.
//     This class is used to implement the Office 2003 theme for splitter
//     windows.
//===========================================================================
class _XTP_EXT_CLASS CXTPSplitterWndThemeOffice2003: public CXTPSplitterWndThemeOfficeXP
{
public:
	//-----------------------------------------------------------------------
	// Summary:
	//     Construct a CXTPSplitterWndThemeOffice2003 object.
	//-----------------------------------------------------------------------
	CXTPSplitterWndThemeOffice2003();

	//-------------------------------------------------------------------------
	// Summary:
	//     Call this function to set the splitter windows face and the
	//     borders to the system default colors.
	// Parameters:
	//     pSplitter - Points to the CXTPSplitterWnd object.
	//-------------------------------------------------------------------------
	virtual void RefreshMetrics(CXTPSplitterWnd* pSplitter);
};

//===========================================================================
// Summary:
//     Class CXTPSplitterWndThemeResource is derived from CXTPSplitterWndTheme.
//     This class is used to implement the Office 2007/2010 theme for splitter
//     windows.
//===========================================================================
class _XTP_EXT_CLASS CXTPSplitterWndThemeResource: public CXTPSplitterWndThemeOffice2003
{
public:
	//-----------------------------------------------------------------------
	// Summary:
	//     Construct a CXTPSplitterWndThemeResource object.
	//-----------------------------------------------------------------------
	CXTPSplitterWndThemeResource();

	//-------------------------------------------------------------------------
	// Summary:
	//     Call this function to set the splitter windows face and the
	//     borders to the system default colors.
	// Parameters:
	//     pSplitter - Points to the CXTPSplitterWnd object.
	//-------------------------------------------------------------------------
	virtual void RefreshMetrics(CXTPSplitterWnd* pSplitter);
};

/////////////////////////////////////////////////////////////////////////////

#endif // !defined(__XTPSPLITTERTHEME_H__)
