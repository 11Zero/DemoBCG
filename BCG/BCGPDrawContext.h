#pragma once

#include "BCGPDrawManager.h"

class CBCGPVisualManager;

class BCGCBPRODLLEXPORT CBCGPDrawContext : public CBCGPDrawManager
{
public:
    CBCGPDrawContext (CDC* pDC, CRect rectPage);
    CBCGPDrawContext (CDC* pDC, CRect rectPage, CRect rectClip);
    CBCGPDrawContext (CBCGPVisualManager* pVisualManager, CDC* pDC, CRect rectContent);
    CBCGPDrawContext (CBCGPVisualManager* pVisualManager, CDC* pDC, CRect rectContent, CRect rectClip);

    // Copy constructor
    CBCGPDrawContext (const CBCGPDrawContext& context);

    CDC* GetDC () const { return &m_dc; }

    // Returns a pointer to the Visual Manager specified in this context.
    CBCGPVisualManager* GetVisualManager () const;

    void SetVisualManager (CBCGPVisualManager* pVisualManager);

    CPoint GetOrigin () const { return m_ptOrigin; }
    void SetOrigin (CPoint ptOrigin) { m_ptOrigin = ptOrigin; }

    // Returns a clipping rectangle.
    CRect GetClipRect () const { return m_rectClip; }

    // Returns bounds of the whole paint area.
    CRect GetContentRect () const { return m_rectContent; }

    void SetContentRect (const CRect& rect)
    {
        m_rectContent = rect;
    }

    double GetScale () const { return m_dScale; }
    void SetScale (double dScale) { m_dScale = dScale; }

    int MapX (int x);
    int MapY (int y);
    CPoint MapPoint (const CPoint& pt);
    CRect MapRect (const CRect& rect);

    DECLARE_DYNAMIC (CBCGPDrawContext)

private:
    CBCGPVisualManager* m_pVisualManager;
    CPoint  m_ptOrigin;
    double  m_dScale;
    CRect   m_rectContent;
    CRect   m_rectClip;
};


