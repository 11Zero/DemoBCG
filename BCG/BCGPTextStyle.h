#if !defined(__BCGPTEXTSTYLE_H)
#define __BCGPTEXTSTYLE_H

//*******************************************************************************
// COPYRIGHT NOTES
// ---------------
// This is a part of BCGControlBar Library Professional Edition
// Copyright (C) 1998-2010 BCGSoft Ltd.
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.
//*******************************************************************************

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGPPropertyBag.h"
#include "BCGPDrawContext.h"

class BCGCBPRODLLEXPORT CBCGPTextStyleRef
{
public:
    CBCGPTextStyleRef ();
    CBCGPTextStyleRef (CBCGPPropertyBag* pPropertyBag);
    virtual ~CBCGPTextStyleRef ();

// Font Attributes
public:
    void SetFontName (const CString& fontName);
    CString GetFontName () const;

    void SetFontSize (double fontPointSize);
    double GetFontSize () const;

    void SetBold (bool bBold);
    bool IsBold () const;

    void SetItalic (bool bItalic);
    bool IsItalic () const;

    void SetTextColor (COLORREF color);
    COLORREF GetTextColor () const;

    void SetBackColor (COLORREF color);
    COLORREF GetBackColor () const;

// Paragraph Attributes
public:
    enum HorzAlign { HA_Left, HA_Center, HA_Right };
    enum VertAlign { VA_Top, VA_Center, VA_Bottom };

    void SetHorizontalAlignment (HorzAlign align);
    HorzAlign GetHorizontalAlignment () const;

    void SetVerticalAlignment (VertAlign align);
    VertAlign GetVerticalAlignment () const;

    void SetMargins (int nLeft, int nTop, int nRight, int nBottom);
    void SetMargins (const CRect& rectMargins);
    CRect GetMargins () const;

    void SetMultiLine (bool bMultiLine);
    bool IsMultiLine () const;

// Other properties
public:
    CBCGPPropertyBag* GetPropertyBag ()
    {
        return m_pBag;
    }

protected:
    CBCGPPropertyBag* m_pBag;
};

BCGCBPRODLLEXPORT void TextStyleCreateFont (const CBCGPTextStyleRef& textStyle, CBCGPDrawContext& drawCtx, CFont& font);
BCGCBPRODLLEXPORT void TextStyleRenderSingleLine (const CBCGPTextStyleRef& textStyle, CBCGPDrawContext& drawCtx, const CString& text);
BCGCBPRODLLEXPORT CSize TextStyleGetSingleLineExtent (const CBCGPTextStyleRef& textStyle, CBCGPDrawContext& drawCtx, const CString& text);


#endif  // __BCGPTEXTSTYLE_H