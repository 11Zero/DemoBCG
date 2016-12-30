//*******************************************************************************
// COPYRIGHT NOTES
// ---------------
// This is a part of the BCGControlBar Library
// Copyright (C) 1998-2014 BCGSoft Ltd.
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.
//*******************************************************************************
//
// BCGPSplitterWnd.cpp : implementation file
//

#include "stdafx.h"
#include "bcgcbpro.h"
#include "BCGPVisualManager.h"
#include "BCGPSplitterWnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CBCGPSplitterWnd, CSplitterWnd)

/////////////////////////////////////////////////////////////////////////////
// CBCGPSplitterWnd

CBCGPSplitterWnd::CBCGPSplitterWnd()
{
}

CBCGPSplitterWnd::~CBCGPSplitterWnd()
{
}


BEGIN_MESSAGE_MAP(CBCGPSplitterWnd, CSplitterWnd)
	//{{AFX_MSG_MAP(CBCGPSplitterWnd)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CBCGPSplitterWnd message handlers

void CBCGPSplitterWnd::OnDrawSplitter (CDC* pDC, ESplitType nType, 
									   const CRect& rectArg)
{
	// if pDC == NULL, then just invalidate
	if (pDC == NULL)
	{
		RedrawWindow(rectArg, NULL, RDW_INVALIDATE|RDW_NOCHILDREN);
		return;
	}

	CRect rect = rectArg;

	switch (nType)
	{
	case splitBorder:
		CBCGPVisualManager::GetInstance ()->OnDrawSplitterBorder (pDC, this, rect);
		return;

	case splitBox:
		CBCGPVisualManager::GetInstance ()->OnDrawSplitterBox (pDC, this, rect);
		break;

	case splitIntersection:
	case splitBar:
		break;

	default:
		ASSERT(FALSE);  // unknown splitter type
	}

	// fill the middle
	CBCGPVisualManager::GetInstance ()->OnFillSplitterBackground (pDC, this, rect);
}

void CBCGPSplitterWnd::RecalcLayout()
{
	for (int col = 0; col < m_nCols; col++)
	{
		for (int row = 0; row < m_nRows; row++)
		{
			if (GetDlgItem(IdFromRowCol(row, col)) == NULL)
			{
				// Not created yet, do nothing
				return;
			}
		}
	}

	CSplitterWnd::RecalcLayout();
}

