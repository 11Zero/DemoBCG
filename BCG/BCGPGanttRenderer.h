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
// BCGPGanttRenderer.h: interface for the CBCGPGanttRenderer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPGANTTRENDERER_H__1BA78DE8_318B_4539_8698_759862967D23__INCLUDED_)
#define AFX_BCGPGANTTRENDERER_H__1BA78DE8_318B_4539_8698_759862967D23__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"

#if !defined (BCGP_EXCLUDE_GRID_CTRL) && !defined (BCGP_EXCLUDE_GANTT)

class CBCGPGanttDrawContext;
struct CBCGPGanttConnection;

class BCGCBPRODLLEXPORT CBCGPGanttRenderer : public CObject  
{
	DECLARE_DYNAMIC(CBCGPGanttRenderer)

public:
	CBCGPGanttRenderer();
	virtual ~CBCGPGanttRenderer();

	virtual void DrawShadow (CBCGPGanttDrawContext& ctxDraw);
	virtual void DrawBar (CBCGPGanttDrawContext& ctxDraw);
	virtual void DrawSelection (CBCGPGanttDrawContext& ctxDraw);
	virtual void DrawConnection (CBCGPGanttDrawContext& ctxDraw, const CBCGPGanttConnection& link);

protected:
	// Draws an item. Used by DrawBar only.
	virtual void DrawSimpleItem (CBCGPGanttDrawContext& ctxDraw);

	// Draws a group item. Used by DrawBar only.
	virtual void DrawGroupItem (CBCGPGanttDrawContext& ctxDraw);

	// Draws a milestone item. Used by DrawBar only.
	virtual void DrawMileStoneItem (CBCGPGanttDrawContext& ctxDraw);

	// Draws a single line using current pen. Used by DrawConnection only.
	virtual void DrawConnectionLine (CBCGPGanttDrawContext& ctxDraw, CPoint ptBegin, CPoint ptEnd, BOOL bFirst, BOOL bLast);

	virtual void DrawGroupPath (CDC& dc, CRect rectBar);

	virtual void DrawMileStonePolygon (CDC& dc, CRect rectBar);
};

class CBCGPGanttItem;

class BCGCBPRODLLEXPORT CBCGPGanttDrawContext
{
public:
	CBCGPGanttDrawContext (const CBCGPGanttItem& item, CDC& dc);

	const CBCGPGanttItem&   m_Item;
	CDC&					m_DC;
	CRect					m_rectClip;
	CRect					m_rectBar;
	COLORREF				m_clrFill;
	COLORREF				m_clrFill2;
	COLORREF				m_clrBorder;
};

#endif // !defined (BCGP_EXCLUDE_GRID_CTRL) && !defined (BCGP_EXCLUDE_GANTT)

#endif // !defined(AFX_BCGPGANTTRENDERER_H__1BA78DE8_318B_4539_8698_759862967D23__INCLUDED_)
