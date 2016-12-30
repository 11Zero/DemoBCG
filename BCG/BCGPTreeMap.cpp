//*******************************************************************************
// COPYRIGHT NOTES
// ---------------
// This is a part of BCGControlBar Library Professional Edition
// Copyright (C) 1998-2014 BCGSoft Ltd.
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.
//*******************************************************************************
//
// BCGPTreeMap.cpp: implementation of the CBCGPTreeMap class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "bcgcbpro.h"
#include "BCGPTreeMap.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

UINT BCGM_ON_CLICK_TREEMAP_NODE = ::RegisterWindowMessage (_T("BCGM_ON_CLICK_TREEMAP_NODE"));

/////////////////////////////////////////////////////////////////////////////
// CBCGPBaseTreeMapNode

IMPLEMENT_DYNAMIC(CBCGPBaseTreeMapNode, CObject)

void CBCGPBaseTreeMapNode::DrawTextWidthShadow(CBCGPGraphicsManager* pGM, const CString& str, const CBCGPRect& rect, const CBCGPBrush& br, const CBCGPTextFormat& tf)
{
	ASSERT_VALID(pGM);

	CBCGPSize sizeText = pGM->GetTextSize(str, tf);

	if (sizeText.cx > rect.Width() || sizeText.cy > rect.Height())
	{
		return;
	}

	CBCGPRect rectShadow = rect;
	rectShadow.OffsetRect(1, 1);

	pGM->DrawText(str, rectShadow, tf, CBCGPBrush(CBCGPColor::Black));
	pGM->DrawText(str, rect, tf, br);
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPTreeMapGroup

IMPLEMENT_DYNAMIC(CBCGPTreeMapGroup, CBCGPBaseTreeMapNode)

CBCGPTreeMapGroup::CBCGPTreeMapGroup(const CBCGPBrush& brush, LPCTSTR lpszLabel, const CBCGPColor& colorText, CBCGPTextFormat* pTF) :
	CBCGPBaseTreeMapNode(lpszLabel)
{
	m_bIsRoot = FALSE;

	m_brFill = brush;
	m_brText = colorText;

	m_brFillCaption = m_brFill.GetColor();
	m_brFillCaption.MakeDarker();

	InitTextFormat(pTF);
}
//********************************************************************************
void CBCGPTreeMapGroup::InitTextFormat(CBCGPTextFormat* pTF)
{
	if (pTF != NULL)
	{
		m_tf = *pTF;
	}
	else
	{
		LOGFONT lf;
		globalData.fontRegular.GetLogFont(&lf);
		m_tf.CreateFromLogFont(lf);

		m_tf.SetTextAlignment(CBCGPTextFormat::BCGP_TEXT_ALIGNMENT_CENTER);
		m_tf.SetTextVerticalAlignment(CBCGPTextFormat::BCGP_TEXT_ALIGNMENT_CENTER);
		m_tf.SetClipText();
	}
}
//********************************************************************************
CBCGPTreeMapGroup::~CBCGPTreeMapGroup()
{
	RemoveAll();
}
//********************************************************************************
CBCGPTreeMapGroup::CBCGPTreeMapGroup()
{
}
//********************************************************************************
void CBCGPTreeMapGroup::AddSubNode(CBCGPBaseTreeMapNode* pNode)
{
	ASSERT_VALID(pNode);

	BOOL bInserted = FALSE;

	int i = 0;
	int nSubNodes = (int)m_arSubNodes.GetSize();

	for (i = 0; !bInserted && i < nSubNodes; i++)
	{
		if (m_arSubNodes[i]->GetValue() <= pNode->GetValue())
		{
			m_arSubNodes.InsertAt(i, pNode);
			bInserted = TRUE;
		}
	}

	if (!bInserted)
	{
		m_arSubNodes.Add(pNode);
	}

	pNode->m_pParent = this;
	nSubNodes++;

	m_dblVal += max(0.0, pNode->GetValue());

	if (!m_bIsRoot)
	{
		// Recalc colors:
		for (i = 0; i < nSubNodes; i++)
		{
			m_arSubNodes[i]->m_brFill = m_brFill;
			m_arSubNodes[i]->m_brFill.MakeLighter(0.8 * i / nSubNodes);
		}
	}
}
//********************************************************************************
void CBCGPTreeMapGroup::RemoveAll()
{
	for (int i = 0; i < m_arSubNodes.GetSize(); i++)
	{
		delete m_arSubNodes[i];
	}

	m_arSubNodes.RemoveAll();
	m_dblVal = 0.;
}
//********************************************************************************
const CBCGPBaseTreeMapNode* CBCGPTreeMapGroup::HitTest(const CBCGPPoint& pt) const
{
	if (!m_rect.PtInRect(pt))
	{
		return NULL;
	}

	for (int i = 0; i < (int)m_arSubNodes.GetSize(); i++)
	{
		const CBCGPBaseTreeMapNode* pNode = m_arSubNodes[i]->HitTest(pt);
		if (pNode != NULL)
		{
			return pNode;
		}
	}

	return this;	// Caption
}
//********************************************************************************
void CBCGPTreeMapGroup::RecalcLayout(CBCGPGraphicsManager* pGM)
{
	ASSERT_VALID(pGM);

	int nSubNodes = (int)m_arSubNodes.GetSize();

	if (nSubNodes == 0 || m_rect.IsRectEmpty())
	{
		return;
	}

	CBCGPRect rect = m_rect;

	if (!m_strLabel.IsEmpty())
	{
		m_rectCaption = m_rect;
		m_rectCaption.bottom = m_rectCaption.top + pGM->GetTextSize(m_strLabel, m_tf).cy + 10.0;

		rect.top = m_rectCaption.bottom;
	}
	else
	{
		m_rectCaption.SetRectEmpty();
	}

	int nLevel = 0;
	CBCGPTreeMap::LayoutType layoutType = m_pOwner != NULL ? m_pOwner->GetLayoutType() : CBCGPTreeMap::Squarified;

	for (CBCGPBaseTreeMapNode* pParent = m_pParent; pParent != NULL; pParent = pParent->m_pParent)
	{
		if (pParent->m_pOwner != NULL)
		{
			ASSERT_VALID(pParent->m_pOwner);
			layoutType = pParent->m_pOwner->GetLayoutType();
		}

		nLevel++;
	}

	if (layoutType == CBCGPTreeMap::Slice)
	{
		const BOOL bIsVertLayout = (nLevel % 2) == 0;
		RecalcSliceLayout(pGM, 0, nSubNodes - 1, rect, bIsVertLayout);
	}
	else
	{
		RecalcSquarifiedLayout(pGM, 0, nSubNodes - 1, rect);
	}
}
//********************************************************************************
double CBCGPTreeMapGroup::GetChildrenTotal(int nStart, int nFinish)
{
	double dblTotal = 0.;

	for (int i = nStart; i <= nFinish; i++)
	{
		dblTotal += max(0.0, m_arSubNodes[i]->GetValue());
	}

	return dblTotal;
}
//********************************************************************************
void CBCGPTreeMapGroup::RecalcSliceLayout(CBCGPGraphicsManager* pGM, int nStart, int nFinish, const CBCGPRect& rect, BOOL bIsVertical)
{
	ASSERT(nStart >= 0);
	ASSERT(nStart < (int)m_arSubNodes.GetSize());
	ASSERT(nFinish >= 0);
	ASSERT(nFinish < (int)m_arSubNodes.GetSize());

	double dblTotal = GetChildrenTotal(nStart, nFinish);

	double x = rect.left;
	double y = rect.top;

	for (int i = nStart; i <= nFinish; i++)
	{
		CBCGPBaseTreeMapNode* pNode = m_arSubNodes[i];
		if (pNode->GetValue() <= 0.0)
		{
			pNode->SetRect(CBCGPRect());
			continue;
		}

		CBCGPRect rectSubNode = rect;

		if (bIsVertical)
		{
			double cx = rect.Width() / dblTotal * pNode->GetValue();

			rectSubNode.left = x;
			rectSubNode.right = x + cx;

			x += cx;
		}
		else
		{
			double cy = rect.Height() / dblTotal * pNode->GetValue();

			rectSubNode.top = y;
			rectSubNode.bottom = y + cy;

			y += cy;
		}

		m_arSubNodes[i]->SetRect(rectSubNode);
		m_arSubNodes[i]->RecalcLayout(pGM);
	}
}
//********************************************************************************
void CBCGPTreeMapGroup::RecalcSquarifiedLayout(CBCGPGraphicsManager* pGM, int nStart, int nFinish, const CBCGPRect& rect)
{
	if (nFinish - nStart < 2)
	{
		RecalcSliceLayout(pGM, nStart, nFinish, rect, rect.Width() > rect.Height());
		return;
	}

	ASSERT(nStart >= 0);
	ASSERT(nStart < (int)m_arSubNodes.GetSize());
	ASSERT(nFinish >= 0);
	ASSERT(nFinish < (int)m_arSubNodes.GetSize());

	int i = 0;
	double dblTotal = GetChildrenTotal(nStart, nFinish);

	double dblTotalLeft = 0.;

	for (i = nStart; i <= nFinish; i++)
	{
		dblTotalLeft += max(0.0, m_arSubNodes[i]->GetValue());
		
		if (dblTotalLeft >= 0.5 * dblTotal)
		{
			if (rect.Width() > rect.Height())
			{
				CBCGPRect rectLeft = rect;
				rectLeft.right = rectLeft.left + rectLeft.Width() * dblTotalLeft / dblTotal;

				RecalcSquarifiedLayout(pGM, nStart, i, rectLeft);

				CBCGPRect rectRight = rect;
				rectRight.left = rectLeft.right + 1;

				RecalcSquarifiedLayout(pGM, i + 1, nFinish, rectRight);
			}
			else
			{
				CBCGPRect rectTop = rect;
				rectTop.bottom = rectTop.top + rectTop.Height() * dblTotalLeft / dblTotal;

				RecalcSquarifiedLayout(pGM, nStart, i, rectTop);

				CBCGPRect rectBottom = rect;
				rectBottom.top = rectTop.bottom + 1;

				RecalcSquarifiedLayout(pGM, i + 1, nFinish, rectBottom);
			}

			return;
		}
	}

	ASSERT(FALSE);
}
//********************************************************************************
void CBCGPTreeMapGroup::OnDraw(CBCGPGraphicsManager* pGM, const CBCGPRect& rectClip, const CBCGPBrush& brBorder)
{
	ASSERT_VALID(pGM);

	if (m_rect.IsRectEmpty())
	{
		return;
	}

	CBCGPRect rectInter;
	if (!rectInter.IntersectRect(m_rect, rectClip))
	{
		return;
	}

	if (!m_rectCaption.IsRectEmpty())
	{
		pGM->FillRectangle(m_rectCaption, m_brFillCaption);
		DrawTextWidthShadow(pGM, m_strLabel, m_rectCaption, m_brText, m_tf);
	}

	int nSubNodes = (int)m_arSubNodes.GetSize();
	for (int i = 0; i < nSubNodes; i++)
	{
		m_arSubNodes[i]->OnDraw(pGM, rectClip, brBorder);
	}
}
//********************************************************************************
void CBCGPTreeMapGroup::SetFontScale(double dblScale)
{
	int nSubNodes = (int)m_arSubNodes.GetSize();
	for (int i = 0; i < nSubNodes; i++)
	{
		m_arSubNodes[i]->SetFontScale(dblScale);
	}

	m_tf.Scale(dblScale);
}
//********************************************************************************
void CBCGPTreeMapGroup::SetRect(const CBCGPRect& rectIn)
{
	CBCGPRect rect = rectIn;

	if (m_sizeMargin != CBCGPSize(-1., -1.))
	{
		rect.DeflateRect(m_sizeMargin);
	}
	else
	{
		for (CBCGPBaseTreeMapNode* pParent = m_pParent; pParent != NULL; pParent = pParent->m_pParent)
		{
			ASSERT_VALID(pParent);

			if (pParent->m_sizeMargin != CBCGPSize(-1., -1.))
			{
				rect.DeflateRect(pParent->m_sizeMargin);
				break;
			}
		}
	}

	m_rect = rect;
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPTreeMapNode

IMPLEMENT_DYNAMIC(CBCGPTreeMapNode, CBCGPBaseTreeMapNode)

CBCGPTreeMapNode::CBCGPTreeMapNode(double dblVal, LPCTSTR lpszLabel) :
	CBCGPBaseTreeMapNode(lpszLabel)
{
	m_dblVal = dblVal;
}
//****************************************************************************
CBCGPTreeMapNode::~CBCGPTreeMapNode()
{
}
//****************************************************************************
void CBCGPTreeMapNode::OnDraw(CBCGPGraphicsManager* pGM, const CBCGPRect& rectClip, const CBCGPBrush& brBorder)
{
	ASSERT_VALID(pGM);

	if (m_rect.IsRectEmpty())
	{
		return;
	}

	CBCGPRect rectInter;
	if (!rectInter.IntersectRect(m_rect, rectClip))
	{
		return;
	}

	pGM->FillRectangle(m_rect, m_brFill);

	if (!m_strLabel.IsEmpty())
	{
		CBCGPTreeMapGroup* pGroup = DYNAMIC_DOWNCAST(CBCGPTreeMapGroup, m_pParent);
		if (pGroup != NULL)
		{
			ASSERT_VALID(pGroup);
			DrawTextWidthShadow(pGM, m_strLabel, m_rect, pGroup->m_brText, pGroup->m_tf);
		}
	}

	if (m_pParent != NULL)
	{
		ASSERT_VALID(m_pParent);

		if (m_pParent->GetRect().top != m_rect.top)
		{
			pGM->DrawLine(m_rect.left, m_rect.top, m_rect.right, m_rect.top, brBorder);
		}

		if (m_pParent->GetRect().bottom != m_rect.bottom)
		{
			pGM->DrawLine(m_rect.left, m_rect.bottom + 1, m_rect.right, m_rect.bottom + 1, brBorder);
		}

		if (m_pParent->GetRect().left != m_rect.left)
		{
			pGM->DrawLine(m_rect.left, m_rect.top, m_rect.left, m_rect.bottom, brBorder);
		}

		if (m_pParent->GetRect().right != m_rect.right)
		{
			pGM->DrawLine(m_rect.right + 1, m_rect.top, m_rect.right + 1, m_rect.bottom, brBorder);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPTreeMap

IMPLEMENT_DYNAMIC(CBCGPTreeMap, CBCGPBaseVisualObject)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGPTreeMap::CBCGPTreeMap()
{
	m_LayoutType = Squarified;
	m_Root.m_pOwner = this;
	m_Root.m_bIsRoot = TRUE;
	m_Root.m_sizeMargin = CBCGPSize(1, 1);
	m_brFill = CBCGPBrush(CBCGPColor::DarkGray);
	
	m_pClicked = NULL;
}
//********************************************************************************
CBCGPTreeMap::~CBCGPTreeMap()
{
}
//********************************************************************************
void CBCGPTreeMap::SetLayoutType(LayoutType layoutType)
{
	m_LayoutType = layoutType;
	SetDirty();
	Redraw();
}
//*****************************************************************************************
void CBCGPTreeMap::SetFillBrush(const CBCGPBrush& br)
{
	m_brFill = br;
	Redraw();
}
//*****************************************************************************************
void CBCGPTreeMap::SetGroupMargin(const CBCGPSize& sizeMargin)
{
	m_Root.m_sizeMargin.cx = max(0., sizeMargin.cx);
	m_Root.m_sizeMargin.cy = max(0., sizeMargin.cy);

	SetDirty();
	Redraw();
}
//*****************************************************************************************
void CBCGPTreeMap::OnDraw(CBCGPGraphicsManager* pGM, const CBCGPRect& rectClip, DWORD dwFlags)
{
	if (IsDirty())
	{
		m_Root.RecalcLayout(pGM);
		SetDirty(FALSE);
	}

	if ((dwFlags & BCGP_DRAW_STATIC) == 0)
	{
		return;
	}

	if (m_Root.m_arSubNodes.GetSize() > 0)
	{
		pGM->FillRectangle(m_rect, m_brFill);
		m_Root.OnDraw(pGM, rectClip.IsRectEmpty() ? m_rect : rectClip, m_brFill);
	}
}
//*****************************************************************************************
void CBCGPTreeMap::AddGroup(CBCGPTreeMapGroup* pGroup)
{
	ASSERT_VALID(pGroup);

	m_Root.AddSubNode(pGroup);
	SetDirty();
}
//*****************************************************************************************
void CBCGPTreeMap::RemoveAll()
{
	m_Root.RemoveAll();
	m_pClicked = NULL;
	SetDirty();
}
//*****************************************************************************************
const CBCGPBaseTreeMapNode* CBCGPTreeMap::HitTestTreeMapNode(const CBCGPPoint& pt) const
{
	const CBCGPBaseTreeMapNode* pHit = m_Root.HitTest(pt);
	return pHit == &m_Root ? NULL : pHit;
}
//*****************************************************************************************
BOOL CBCGPTreeMap::OnMouseDown(int nButton, const CBCGPPoint& pt)
{
	if (nButton == 0)
	{
		m_pClicked = HitTestTreeMapNode(pt);
	}

	return CBCGPBaseVisualObject::OnMouseDown(nButton, pt);
}
//*****************************************************************************************
void CBCGPTreeMap::OnMouseUp(int nButton, const CBCGPPoint& pt)
{
	if (nButton == 0)
	{
		if (m_pClicked == HitTestTreeMapNode(pt))
		{
			const CBCGPBaseTreeMapNode* pClickedNode = m_pClicked;
			m_pClicked = NULL;
			
			if (pClickedNode != NULL)
			{
				OnClickTreeMapNode(pClickedNode);
			}
		}
	
		m_pClicked = NULL;
	}

	CBCGPBaseVisualObject::OnMouseUp(nButton, pt);
}
//*****************************************************************************************
void CBCGPTreeMap::OnMouseLeave()
{
	CBCGPBaseVisualObject::OnMouseLeave();
	m_pClicked = NULL;
}
//*****************************************************************************************
void CBCGPTreeMap::OnCancelMode()
{
	CBCGPBaseVisualObject::OnCancelMode();
	m_pClicked = NULL;
}
//*****************************************************************************************
BOOL CBCGPTreeMap::OnGetToolTip(const CBCGPPoint& pt, CString& strToolTip, CString& strDescr)
{
	const CBCGPBaseTreeMapNode* pNode = HitTestTreeMapNode(pt);
	if (pNode != NULL)
	{
		strToolTip.Format(_T("%s: %f"), pNode->GetLabel(), pNode->GetValue());
		return TRUE;
	}

	return CBCGPBaseVisualObject::OnGetToolTip(pt, strToolTip, strDescr);
}
//*****************************************************************************************
void CBCGPTreeMap::OnClickTreeMapNode(const CBCGPBaseTreeMapNode* pClickedNode)
{
	if (m_pWndOwner->GetSafeHwnd() == NULL)
	{
		return;
	}

	CWnd* pOwner = m_pWndOwner->GetOwner();
	if (pOwner->GetSafeHwnd() == NULL)
	{
		return;
	}

	m_pWndOwner->GetOwner()->PostMessage(BCGM_ON_CLICK_TREEMAP_NODE, (WPARAM)GetID(), (LPARAM)pClickedNode);
}
//*****************************************************************************************
void CBCGPTreeMap::OnScaleRatioChanged(const CBCGPSize& /*sizeScaleRatioOld*/)
{
	m_Root.SetFontScale(m_sizeScaleRatio.cy);
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPTreeMapCtrl window

IMPLEMENT_DYNAMIC(CBCGPTreeMapCtrl, CBCGPVisualCtrl)

CBCGPTreeMapCtrl::CBCGPTreeMapCtrl()
{
	m_pTreeMap = NULL;
}
//*****************************************************************************************
CBCGPTreeMapCtrl::~CBCGPTreeMapCtrl()
{
	if (m_pTreeMap != NULL)
	{
		delete m_pTreeMap;
	}
}

BEGIN_MESSAGE_MAP(CBCGPTreeMapCtrl, CBCGPVisualCtrl)
	//{{AFX_MSG_MAP(CBCGPTreeMapCtrl)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGPTreeMapCtrl message handlers
