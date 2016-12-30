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
// BCGPRibbonProgressBar.cpp: implementation of the CBCGPRibbonProgressBar class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BCGPRibbonProgressBar.h"
#include "BCGPVisualManager.h"
#include "BCGPPopupMenu.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#ifndef BCGP_EXCLUDE_RIBBON

IMPLEMENT_DYNCREATE(CBCGPRibbonProgressBar, CBCGPBaseRibbonElement)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGPRibbonProgressBar::CBCGPRibbonProgressBar()
{
	CommonInit ();
}
//*****************************************************************************************************
CBCGPRibbonProgressBar::CBCGPRibbonProgressBar(
		UINT	nID, 
		int		nWidth,
		int		nHeight,
		BOOL	bIsVertical)
{
	CommonInit ();

	m_nID = nID;
	m_nWidth = nWidth;
	m_nHeight = nHeight;
	m_bIsVertical = bIsVertical;
}
//*****************************************************************************************************
CBCGPRibbonProgressBar::~CBCGPRibbonProgressBar()
{
}
//*****************************************************************************************************
void CBCGPRibbonProgressBar::CommonInit ()
{
	m_nMin = 0;
	m_nMax = 100;
	m_nPos = 0;
	m_nWidth = 100;
	m_nHeight = 22;
	m_bInfiniteMode = FALSE;
	m_bIsVertical = FALSE;
}
//********************************************************************************
void CBCGPRibbonProgressBar::OnDraw (CDC* pDC)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);

	if (m_rect.IsRectEmpty () || m_nMax - m_nMin <= 0)
	{
		return;
	}

	CRect rectProgress = m_rect;
	rectProgress.DeflateRect (5, 5);

	CRect rectChunk(0, 0, 0, 0);
	CalculateChunkRect(rectProgress, rectChunk);

	CBCGPVisualManager::GetInstance ()->OnDrawRibbonProgressBar (
		pDC, this, rectProgress, rectChunk, m_bInfiniteMode);
}
//*****************************************************************************
void CBCGPRibbonProgressBar::CopyFrom (const CBCGPBaseRibbonElement& s)
{
	ASSERT_VALID (this);

	CBCGPBaseRibbonElement::CopyFrom (s);

	CBCGPRibbonProgressBar& src = (CBCGPRibbonProgressBar&) s;

	m_nMin = src.m_nMin;
	m_nMax = src.m_nMax;
	m_nPos = src.m_nPos;
	m_nWidth = src.m_nWidth;
	m_nHeight = src.m_nHeight;
	m_bInfiniteMode = src.m_bInfiniteMode;
	m_bIsVertical = src.m_bIsVertical;
}
//*****************************************************************************
CSize CBCGPRibbonProgressBar::GetRegularSize (CDC* /*pDC*/)
{
	ASSERT_VALID (this);

	int nWidth = m_bIsVertical && IsQATMode() ? m_nHeight : m_nWidth;
	int nHeight = m_bIsVertical && IsQATMode() ? m_nWidth : m_nHeight;

	if (globalData.GetRibbonImageScale () != 1.)
	{
		if (IsVertical())
		{
			nWidth = (int)(.5 + globalData.GetRibbonImageScale () * nWidth);
			nWidth -= (nWidth - m_nWidth) / 2;
		}
		else
		{
			nHeight = (int)(.5 + globalData.GetRibbonImageScale () * nHeight);
			nHeight -= (nHeight - m_nHeight) / 2;
		}
	}

	return CSize (nWidth, nHeight);
}
//*****************************************************************************
void CBCGPRibbonProgressBar::SetRange (int nMin, int nMax)
{
	ASSERT_VALID (this);

	m_nMin = nMin;
	m_nMax = nMax;
}
//*****************************************************************************
void CBCGPRibbonProgressBar::SetPos (int nPos, BOOL bRedraw)
{
	ASSERT_VALID (this);

	m_nPos = min (max (m_nMin, nPos), m_nMax);

	if (bRedraw)
	{
		Redraw ();

		CWnd* pMenu = CBCGPPopupMenu::GetActiveMenu ();

		if (pMenu != NULL && CWnd::FromHandlePermanent (pMenu->GetSafeHwnd ()) != NULL &&
			GetParentWnd () != NULL)
		{
			CRect rectScreen = m_rect;
			GetParentWnd ()->ClientToScreen (&rectScreen);

			CBCGPPopupMenu::UpdateAllShadows (rectScreen);
		}
	}
}
//*****************************************************************************
void CBCGPRibbonProgressBar::SetInfiniteMode (BOOL bSet)
{
	ASSERT_VALID (this);
	m_bInfiniteMode = bSet;
}
//********************************************************************************
void CBCGPRibbonProgressBar::OnDrawOnList (CDC* pDC, CString strText,
									  int nTextOffset, CRect rect,
									  BOOL /*bIsSelected*/,
									  BOOL /*bHighlighted*/)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);

	const int nProgressWidth = rect.Height () * 2;

	BOOL bIsDisabled = m_bIsDisabled;
	m_bIsDisabled = FALSE;

	CRect rectText = rect;

	rectText.left += nTextOffset;
	rectText.right -= nProgressWidth;

	const int xMargin = 3;
	rectText.DeflateRect (xMargin, 0);

	pDC->DrawText (strText, rectText, DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

	// Draw progress:
	CRect rectProgress = rect;
	rectProgress.left = rectProgress.right - nProgressWidth;

	if (IsVertical())
	{
		rectProgress.DeflateRect (rectProgress.Width() / 3, 1);
	}
	else
	{
		rectProgress.DeflateRect (1, rectProgress.Height () / 4);
	}

	CRect rectChunk = rectProgress;
	if (IsVertical())
	{
		rectChunk.top = rectChunk.CenterPoint().y;
	}
	else
	{
		rectChunk.right = rectChunk.CenterPoint().x;
	}

	rectChunk.DeflateRect(1, 1);

	int nPos = m_nPos;
	m_nPos = m_nMin + (m_nMax - m_nMin) / 2;

	CBCGPVisualManager::GetInstance ()->OnDrawRibbonProgressBar (
		pDC, this, rectProgress, rectChunk, FALSE);

	m_bIsDisabled = bIsDisabled;
	m_nPos = nPos;
}
//********************************************************************************
void CBCGPRibbonProgressBar::CalculateChunkRect(const CRect& rectProgress, CRect& rectChunk)
{
	ASSERT_VALID (this);

	rectChunk.SetRectEmpty();
	
	if (m_nMax > m_nMin && rectProgress.Width () > 0 && rectProgress.Height () > 0)
	{
		rectChunk = rectProgress;
		
		if (m_bInfiniteMode)
		{
			if (IsVertical())
			{
				int yTop = rectChunk.top;
				int yBottom = rectChunk.bottom;

				const int nChunkHeight = min(rectProgress.Width() * 3, rectProgress.Height() / 3);
				
				rectChunk.InflateRect(0, nChunkHeight);
				
				int y = rectChunk.bottom - m_nPos * rectChunk.Height () / (m_nMax - m_nMin);

				rectChunk.bottom = min(yBottom, y);
				rectChunk.top = max(yTop, y - nChunkHeight);
			}
			else
			{
				int xLeft = rectChunk.left;
				int xRight = rectChunk.right;

				const int nChunkWidth = min(rectProgress.Height() * 3, rectProgress.Width() / 6);
				
				rectChunk.InflateRect (nChunkWidth, 0);
				
				int x = rectChunk.left + m_nPos * rectChunk.Width () / (m_nMax - m_nMin);
				rectChunk.left = max(xLeft, x);
				rectChunk.right = min(xRight, x + nChunkWidth);
			}
		}
		else
		{
			int nTotalSize = IsVertical() ? rectChunk.Height () : rectChunk.Width();
			int ptPos = (m_nPos - m_nMin) * nTotalSize / (m_nMax - m_nMin);

			if (IsVertical())
			{
				rectChunk.top = max(rectChunk.bottom - ptPos, rectProgress.top);
			}
			else
			{
				rectChunk.right = min(rectChunk.left + ptPos, rectProgress.right);
			}
		}
	}
}
//********************************************************************************
BOOL CBCGPRibbonProgressBar::SetACCData(CWnd* pParent, CBCGPAccessibilityData& data)
{
	ASSERT_VALID(this);

	if (!CBCGPBaseRibbonElement::SetACCData(pParent, data))
	{
		return FALSE;
	}

	data.m_nAccRole = ROLE_SYSTEM_PROGRESSBAR;
	data.m_strAccDefAction.Empty();
	data.m_bAccState = STATE_SYSTEM_NORMAL;
	data.m_strAccValue.Format (_T("%d"), m_nPos);
	return TRUE;
}

#endif // BCGP_EXCLUDE_RIBBON

