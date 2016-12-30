#include "stdafx.h"
#include "BCGPTextStyle.h"

#error "This file has been excluded from project. Please get the latest version from SourceSafe."

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBCGPTextStyleRef

CBCGPTextStyleRef::CBCGPTextStyleRef ()
{
}

CBCGPTextStyleRef::CBCGPTextStyleRef (CBCGPPropertyBag* pPropertyBag)
    : m_pBag (pPropertyBag)
{
    ASSERT_VALID (m_pBag);
}

CBCGPTextStyleRef::~CBCGPTextStyleRef ()
{
}

void TextStyleCreateFont (const CBCGPTextStyleRef& textStyle, CBCGPDrawContext& drawCtx, CFont& font)
{
    LOGFONT lf;
    ZeroMemory (&lf, sizeof (lf));

    CString strFontName = textStyle.GetFontName ();
    double dPointSize = textStyle.GetFontSize ();

    lstrcpyn (lf.lfFaceName, strFontName, LF_FACESIZE);
    lf.lfHeight = (long)(dPointSize * 10 * drawCtx.GetScale ());
    lf.lfWeight = textStyle.IsBold () ? FW_BOLD : FW_NORMAL;
    lf.lfItalic = textStyle.IsItalic ();
    font.CreatePointFontIndirect (&lf, drawCtx.GetDC ());
}

void TextStyleRenderSingleLine (const CBCGPTextStyleRef& textStyle, CBCGPDrawContext& drawCtx, const CString& text)
{
    CFont font;
    TextStyleCreateFont (textStyle, drawCtx, font);

    CDC* pDC = drawCtx.GetDC ();
    CBCGPFontSelector fontSelector (*pDC, &font);

    CRect rectOutput = drawCtx.GetContentRect ();

    COLORREF clrBack = textStyle.GetBackColor ();
    if (clrBack != CLR_INVALID)
    {
        pDC->SetBkColor (clrBack);
        pDC->SetBkMode (OPAQUE);
    }
    else
    {
        pDC->SetBkMode (TRANSPARENT);
    }
    COLORREF clrText = textStyle.GetTextColor ();
    pDC->SetTextColor (clrText);

    pDC->DrawText (text, rectOutput, DT_LEFT | DT_SINGLELINE);
}

CSize TextStyleGetSingleLineExtent (const CBCGPTextStyleRef& textStyle, CBCGPDrawContext& drawCtx, const CString& text)
{
    CFont font;
    TextStyleCreateFont (textStyle, drawCtx, font);

    CDC* pDC = drawCtx.GetDC ();
    CBCGPFontSelector fontSelector (*pDC, &font);

    CSize szExtent;
    szExtent = pDC->GetTextExtent (text);
    return szExtent;
}

void CBCGPTextStyleRef::SetFontName (const CString& fontName)
{
    m_pBag->SetProperty ("FontName", fontName);
}

CString CBCGPTextStyleRef::GetFontName () const
{
    return m_pBag->GetStringDefault ("FontName", CString ());
}

void CBCGPTextStyleRef::SetFontSize (double fontPointSize)
{
    m_pBag->SetProperty ("FontSize", fontPointSize);
}

double CBCGPTextStyleRef::GetFontSize () const
{
    return m_pBag->GetDoubleDefault ("FontSize", 8.25);
}

void CBCGPTextStyleRef::SetBold (bool bBold)
{
    m_pBag->SetProperty ("Bold", bBold);
}

bool CBCGPTextStyleRef::IsBold () const
{
    return m_pBag->GetBoolDefault ("Bold", false);
}

void CBCGPTextStyleRef::SetItalic (bool bItalic)
{
    m_pBag->SetProperty ("Italic", bItalic);
}

bool CBCGPTextStyleRef::IsItalic () const
{
    return m_pBag->GetBoolDefault ("Italic", false);
}

void CBCGPTextStyleRef::SetTextColor (COLORREF color)
{
    m_pBag->SetProperty ("TextColor", color);
}

COLORREF CBCGPTextStyleRef::GetTextColor () const
{
    return m_pBag->GetUlongDefault ("TextColor", CLR_INVALID);
}

void CBCGPTextStyleRef::SetBackColor (COLORREF color)
{
    m_pBag->SetProperty ("TextBackColor", color);
}

COLORREF CBCGPTextStyleRef::GetBackColor () const
{
    return m_pBag->GetUlongDefault ("TextBackColor", CLR_INVALID);
}

void CBCGPTextStyleRef::SetHorizontalAlignment (CBCGPTextStyleRef::HorzAlign align)
{
    const wchar_t* pstr = NULL;

    switch (align)
    {
    case HA_Left:
        pstr = L"Left";
        break;
    case HA_Center:
        pstr = L"Center";
        break;
    case HA_Right:
        pstr = L"Right";
        break;
    default:
        m_pBag->ResetProperty ("TextAlignment");
        return;
    }

    m_pBag->SetProperty ("TextAlignment", pstr);
}

CBCGPTextStyleRef::HorzAlign CBCGPTextStyleRef::GetHorizontalAlignment () const
{
    CString value;
    m_pBag->GetProperty ("TextAlignment", value);

    if (value == _T("Center"))
    {
        return HA_Center;
    }

    if (value == _T("Right"))
    {
        return HA_Right;
    }

    return HA_Left; // default
}

void CBCGPTextStyleRef::SetVerticalAlignment (CBCGPTextStyleRef::VertAlign align)
{
    const wchar_t* pstr = NULL;

    switch (align)
    {
    case VA_Top:
        pstr = L"Top";
        break;
    case VA_Center:
        pstr = L"Center";
        break;
    case VA_Bottom:
        pstr = L"Bottom";
        break;
    default:
        m_pBag->ResetProperty ("VerticalAlignment");
        return;
    }

    m_pBag->SetProperty ("VerticalAlignment", pstr);
}

CBCGPTextStyleRef::VertAlign CBCGPTextStyleRef::GetVerticalAlignment () const
{
    CString value;
    m_pBag->GetProperty ("VerticalAlignment", value);

    if (value == _T("Center"))
    {
        return VA_Center;
    }

    if (value == _T("Bottom"))
    {
        return VA_Bottom;
    }

    return VA_Top; // default
}

void CBCGPTextStyleRef::SetMargins (int nLeft, int nTop, int nRight, int nBottom)
{
    m_pBag->SetProperty ("LeftMargin", nLeft);
    m_pBag->SetProperty ("TopMargin", nTop);
    m_pBag->SetProperty ("RightMargin", nRight);
    m_pBag->SetProperty ("BottomMargin", nBottom);
}

void CBCGPTextStyleRef::SetMargins (const CRect& rect)
{
    SetMargins (rect.left, rect.top, rect.right, rect.bottom);
}

CRect CBCGPTextStyleRef::GetMargins () const
{
    CRect margins (0, 0, 0, 0);
    m_pBag->GetProperty ("LeftMargin", margins.left);
    m_pBag->GetProperty ("TopMargin", margins.top);
    m_pBag->GetProperty ("RightMargin", margins.right);
    m_pBag->GetProperty ("BottomMargin", margins.bottom);
    return margins;
}

void CBCGPTextStyleRef::SetMultiLine (bool bMultiLine)
{
    m_pBag->SetProperty ("MultiLine", bMultiLine);
}

bool CBCGPTextStyleRef::IsMultiLine () const
{
    return m_pBag->GetBoolDefault ("MultiLine", false);
}
