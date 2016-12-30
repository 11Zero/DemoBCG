
#include "stdafx.h"
#include "BCGPDrawContext.h"
#include "BCGPVisualManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CBCGPDrawContext, CObject)

CBCGPDrawContext::CBCGPDrawContext (CDC* pDC, CRect rectPage)
    : CBCGPDrawManager (*pDC)
//     : m_pDC (pDC)
//     , m_DrawManager(*pDC)
    , m_rectContent (rectPage)
    , m_dScale (1.0)
{
    m_pVisualManager = CBCGPVisualManager::GetInstance ();
    pDC->GetClipBox (&m_rectClip);
}

CBCGPDrawContext::CBCGPDrawContext (CDC* pDC, CRect rectPage, CRect rectClip)
//     : m_pDC (pDC)
    : CBCGPDrawManager (*pDC)
//    , m_DrawManager(*pDC)
    , m_rectContent (rectPage)
    , m_rectClip (rectClip)
    , m_dScale (1.0)
{
    m_pVisualManager = CBCGPVisualManager::GetInstance ();
}

CBCGPDrawContext::CBCGPDrawContext (CBCGPVisualManager* pVisualManager, CDC* pDC, CRect rectContent)
    : m_pVisualManager (pVisualManager)
    , CBCGPDrawManager (*pDC)
//     , m_pDC (pDC)
//     , m_DrawManager (*pDC)
    , m_rectContent (rectContent)
    , m_dScale (1.0)
{
    pDC->GetClipBox (&m_rectClip);
}

CBCGPDrawContext::CBCGPDrawContext (CBCGPVisualManager* pVisualManager, CDC* pDC, CRect rectContent, CRect rectClip)
    : m_pVisualManager (pVisualManager)
    , CBCGPDrawManager (*pDC)
//     , m_pDC (pDC)
//     , m_DrawManager (*pDC)
    , m_rectContent (rectContent)
    , m_rectClip (rectClip)
    , m_dScale (1.0)
{
}

CBCGPDrawContext::CBCGPDrawContext (const CBCGPDrawContext& ctx)
    : m_pVisualManager (ctx.m_pVisualManager)
    , CBCGPDrawManager (*ctx.GetDC ())
//     , m_pDC (ctx.m_pDC)
//     , m_DrawManager (*ctx.m_pDC)
    , m_rectContent (ctx.m_rectContent)
    , m_rectClip (ctx.m_rectClip)
    , m_dScale (ctx.m_dScale)
{
}

CBCGPVisualManager* CBCGPDrawContext::GetVisualManager () const
{
    return m_pVisualManager;
}

// CBCGPDrawManager* CBCGPDrawContext::GetDrawManager ()
// {
//     return &m_DrawManager;
// }

int CBCGPDrawContext::MapX (int x)
{
    return (int)(x * m_dScale - m_ptOrigin.x + 0.5);
}

int CBCGPDrawContext::MapY (int y)
{
    return (int)(y * m_dScale - m_ptOrigin.y + 0.5);
}

CPoint CBCGPDrawContext::MapPoint (const CPoint& pt)
{
    return CPoint (MapX (pt.x), MapY (pt.y));
}

CRect CBCGPDrawContext::MapRect (const CRect& rect)
{
    return CRect (MapX (rect.left), MapY (rect.top), MapX (rect.right), MapY (rect.bottom));
}
