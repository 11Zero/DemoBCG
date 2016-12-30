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
// BCGPDiagramConnector.cpp: implementation of the CBCGPDiagramConnector class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "bcgcbpro.h"
#include "BCGPDiagramConnector.h"
#include "BCGPDiagramVisualContainer.h"
#include <math.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CBCGPDiagramShelfConnector, CBCGPDiagramConnector)
IMPLEMENT_DYNCREATE(CBCGPDiagramElbowConnector, CBCGPDiagramConnector)

//////////////////////////////////////////////////////////////////////
// CBCGPDiagramShelfConnector

CBCGPDiagramShelfConnector::CBCGPDiagramShelfConnector(double dShelfOffset)
{
	m_dShelfOffset = dShelfOffset;
	m_curveType = BCGP_CURVE_TYPE_LINE;
}

CBCGPDiagramShelfConnector::CBCGPDiagramShelfConnector(const CBCGPPoint& pt1, const CBCGPPoint& pt2, double dShelfOffset)
{
	m_dShelfOffset = dShelfOffset;
	m_curveType = BCGP_CURVE_TYPE_LINE;

	InsertPoint (0, pt1);
	InsertPoint (1, pt1);
	InsertPoint (2, pt2);
	InsertPoint (3, pt2);

	RecalcPoints();
}

CBCGPDiagramShelfConnector::CBCGPDiagramShelfConnector(const CBCGPDiagramShelfConnector& src)
{
	CopyFrom(src);
}

CBCGPDiagramShelfConnector::~CBCGPDiagramShelfConnector ()
{
}
//*******************************************************************************
void CBCGPDiagramShelfConnector::SetShelfSize (double dSize)
{
	ASSERT(dSize > 0.0);
	m_dShelfOffset = dSize;

	RecalcPoints();
	SetDirty ();
}
//*******************************************************************************
double CBCGPDiagramShelfConnector::GetShelfSize () const
{
	return m_dShelfOffset;
}
//*******************************************************************************
void CBCGPDiagramShelfConnector::SetTrackingRect(const CBCGPRect& rectTrack)
{
	const CBCGPSize sizeMin = GetMinSize();
	
	if (!sizeMin.IsEmpty())
	{
		if (rectTrack.Width() < sizeMin.cx || rectTrack.Height() < sizeMin.cy)
		{
			return;
		}
	}

	CBCGPDiagramConnector::SetTrackingRect (rectTrack);

	CBCGPRect rectTracker;
	UINT nTrackerID;
	for (POSITION pos = m_mapTrackRects.GetStartPosition (); pos != NULL; )
	{
		m_mapTrackRects.GetNextAssoc (pos, nTrackerID, rectTracker);

		if (nTrackerID > CP_CustomFirst)
		{
			m_mapTrackRects.RemoveKey (nTrackerID);
		}
	}
}
//*******************************************************************************
void CBCGPDiagramShelfConnector::RecalcPoints()
{
	CBCGPDiagramConnector::RecalcPoints ();

	AnchorPointsArray& arPoints = m_bIsSelected ? m_arTrackPoints : m_arPoints;

	if (arPoints.GetSize () < 4)
	{
		return;
	}

	CBCGPPoint pt1 = arPoints[0].m_ptNullAnchor;
	CBCGPPoint pt2 = arPoints[3].m_ptNullAnchor;

	const CBCGPDiagramVisualContainer* pContainer = GetParentDiagram ();
	if (pContainer != NULL)
	{
		pt1 = pContainer->CalculatePoint (arPoints[0]);
		pt2 = pContainer->CalculatePoint (arPoints[3]);
	}

	double dOffset = (pt2.x - pt1.x >= 0) ? m_dShelfOffset : -m_dShelfOffset;
	dOffset *= m_sizeScaleRatio.cx;

	if (arPoints[1].IsNull ())
	{
		arPoints[1].m_ptNullAnchor.SetPoint (pt1.x + dOffset, pt1.y);
	}

	if (arPoints[2].IsNull ())
	{
		arPoints[2].m_ptNullAnchor.SetPoint (pt2.x - dOffset, pt2.y);
	}
}
//*******************************************************************************
UINT CBCGPDiagramShelfConnector::HitTestAnchorPoint (const CBCGPPoint& pt, int& nIndex) const
{
	UINT nRes = CBCGPDiagramConnector::HitTestAnchorPoint (pt, nIndex);

	if (nRes > CP_CustomFirst)
	{
		nIndex = -1;
		return HTNOWHERE;
	}

	return nRes;
}
//*******************************************************************************
void CBCGPDiagramShelfConnector::CopyFrom(const CBCGPBaseVisualObject& srcObj)
{
	CBCGPDiagramConnector::CopyFrom(srcObj);

	const CBCGPDiagramShelfConnector& src = (const CBCGPDiagramShelfConnector&)srcObj;

	m_dShelfOffset = src.m_dShelfOffset;
}

//////////////////////////////////////////////////////////////////////
// CBCGPDiagramElbowConnector

CBCGPDiagramElbowConnector::CBCGPDiagramElbowConnector(Orientation orientation)
{
	m_Orientation = orientation;
}
//*******************************************************************************
CBCGPDiagramElbowConnector::CBCGPDiagramElbowConnector(const CBCGPRect& rect, Orientation orientation)
{
	m_Orientation = orientation;
	m_ptResizeHandle = rect.CenterPoint ();

	InsertPoint (0, rect.TopLeft ());
	InsertPoint (1, rect.TopLeft ());
	InsertPoint (2, rect.BottomRight ());
	InsertPoint (3, rect.BottomRight ());

	RecalcPoints();
}
//*******************************************************************************
CBCGPDiagramElbowConnector::CBCGPDiagramElbowConnector(const CBCGPPoint& pt1, const CBCGPPoint& pt2, const CBCGPPoint& ptResizeHandle, Orientation orientation)
{
	m_Orientation = orientation;
	m_ptResizeHandle = (ptResizeHandle != CBCGPPoint()) ? ptResizeHandle : (pt1 + pt2) / 2.0;

	InsertPoint (0, pt1);
	InsertPoint (1, pt1);
	InsertPoint (2, pt2);
	InsertPoint (3, pt2);

	RecalcPoints();
}
//*******************************************************************************
CBCGPDiagramElbowConnector::CBCGPDiagramElbowConnector(const CBCGPDiagramElbowConnector& src)
{
	CopyFrom(src);
}
//*******************************************************************************
void CBCGPDiagramElbowConnector::SetResizeHandlePoint (const CBCGPPoint& ptResizeHandle)
{
	m_ptResizeHandle = ptResizeHandle;

	RecalcPoints();
	SetDirty ();
}
//*******************************************************************************
const CBCGPPoint& CBCGPDiagramElbowConnector::GetResizeHandlePoint () const
{
	return m_ptResizeHandle;
}
//*******************************************************************************
void CBCGPDiagramElbowConnector::SetOrientation (Orientation orientation)
{
	m_Orientation = orientation;

	RecalcPoints ();
	SetDirty ();
}
//*******************************************************************************
CBCGPDiagramElbowConnector::Orientation CBCGPDiagramElbowConnector::GetOrientation () const
{
	return m_Orientation;
}
//*******************************************************************************
void CBCGPDiagramElbowConnector::SetRect(const CBCGPRect& rect, BOOL bRedraw)
{
	CBCGPDiagramConnector::SetRect (rect, bRedraw);

	//m_ptResizeHandle = rect.CenterPoint ();
}
//*******************************************************************************
void CBCGPDiagramElbowConnector::SetTrackingRect(const CBCGPRect& rectTrack)
{
	const CBCGPSize sizeMin = GetMinSize();
	
	if (!sizeMin.IsEmpty())
	{
		if (rectTrack.Width() < sizeMin.cx || rectTrack.Height() < sizeMin.cy)
		{
			return;
		}
	}

	CBCGPDiagramConnector::SetTrackingRect (rectTrack);

	m_ptTrackResizeHandle = m_rectTrack.CenterPoint ();

	CBCGPRect rectTracker;
	UINT nTrackerID;
	for (POSITION pos = m_mapTrackRects.GetStartPosition (); pos != NULL; )
	{
		m_mapTrackRects.GetNextAssoc (pos, nTrackerID, rectTracker);

		if (nTrackerID > CP_CustomFirst)
		{
			m_mapTrackRects.RemoveKey (nTrackerID);
		}
	}

	m_mapTrackRects[CP_ResizeHandle] = MakeTrackMarker (m_ptTrackResizeHandle);
}
//*******************************************************************************
void CBCGPDiagramElbowConnector::RecalcPoints()
{
	const CBCGPRect rectOld = GetBoundsRect ();

	CBCGPDiagramConnector::RecalcPoints ();

	const CBCGPRect& rect = m_bIsSelected ? m_rectTrack : m_rect;
	AnchorPointsArray& arPoints = m_bIsSelected ? m_arTrackPoints : m_arPoints;

	if (m_bIsSelected)
	{
		m_ptTrackResizeHandle = rect.CenterPoint ();
	}

	CBCGPPoint& ptResizeHandle = m_bIsSelected ? m_ptTrackResizeHandle : m_ptResizeHandle;

	if (rectOld != rect)
	{
		const CBCGPPoint ptOffset = rect.TopLeft () - rectOld.TopLeft ();
		const double dblAspectRatioX = (rectOld.Width () == 0.0) ? 1.0 : rect.Width () / rectOld.Width ();
		const double dblAspectRatioY = (rectOld.Height () == 0.0) ? 1.0 : rect.Height () / rectOld.Height ();

		ptResizeHandle.Scale(CBCGPPoint (dblAspectRatioX, dblAspectRatioY), rectOld.TopLeft ());
		ptResizeHandle.Offset (ptOffset);
	}

	if (arPoints.GetSize () < 4)
	{
		return;
	}

	CBCGPPoint pt1 = arPoints[0].m_ptNullAnchor;
	CBCGPPoint pt2 = arPoints[3].m_ptNullAnchor;

	const CBCGPDiagramVisualContainer* pContainer = GetParentDiagram ();
	if (pContainer != NULL)
	{
		pt1 = pContainer->CalculatePoint (arPoints[0]);
		pt2 = pContainer->CalculatePoint (arPoints[3]);
	}

	Orientation orientation = m_Orientation;
	if (m_Orientation == Auto)
	{
		orientation = (fabs (pt1.x - pt2.x) >= fabs (pt1.y - pt2.y)) ? Horizontal : Vertical;
	}

	if (orientation == Horizontal)
	{
		if (arPoints[1].IsNull ())
		{
			arPoints[1].m_ptNullAnchor.SetPoint (ptResizeHandle.x, pt1.y);
		}

		if (arPoints[2].IsNull ())
		{
			arPoints[2].m_ptNullAnchor.SetPoint (ptResizeHandle.x, pt2.y);
		}
	}
	else
	{
		if (arPoints[1].IsNull ())
		{
			arPoints[1].m_ptNullAnchor.SetPoint (pt1.x, ptResizeHandle.y);
		}

		if (arPoints[2].IsNull ())
		{
			arPoints[2].m_ptNullAnchor.SetPoint (pt2.x, ptResizeHandle.y);
		}
	}
}
//*******************************************************************************
void CBCGPDiagramElbowConnector::OnEndTrackingPoints (BOOL bSaveChanges)
{
	CBCGPDiagramConnector::OnEndTrackingPoints (bSaveChanges);

	if (bSaveChanges)
	{
		m_ptResizeHandle = m_ptTrackResizeHandle;
	}
}
//*******************************************************************************
UINT CBCGPDiagramElbowConnector::HitTestAnchorPoint (const CBCGPPoint& pt, int& nIndex) const
{
	UINT nRes = CBCGPDiagramConnector::HitTestAnchorPoint (pt, nIndex);

	if (nRes > CP_CustomFirst)
	{
		nIndex = -1;
		return HTNOWHERE;
	}

	return nRes;
}
//*******************************************************************************
void CBCGPDiagramElbowConnector::CopyFrom(const CBCGPBaseVisualObject& srcObj)
{
	CBCGPDiagramConnector::CopyFrom(srcObj);

	const CBCGPDiagramElbowConnector& src = (const CBCGPDiagramElbowConnector&)srcObj;

	m_Orientation = src.m_Orientation;
	m_ptResizeHandle = src.m_ptResizeHandle;
}
