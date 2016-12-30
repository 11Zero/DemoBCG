//** *****************************************************************************
// COPYRIGHT NOTES
// ---------------
// This is a sample for BCGControlBar Library Professional Edition
// Copyright (C) 1998-2014 BCGSoft Ltd.
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.
//*******************************************************************************
//
// BCGPChartVisualObject.cpp : implementation file
//

#include "stdafx.h"
#include "bcgprores.h"
#include "BCGPChartVisualObject.h"
#include "BCGPChartImpl.h"
#include "BCGPChartLegend.h"
#include "BCGPChartObject.h"
#include "BCGPChartFormula.h"
#include "BCGPGraphicsManagerGDI.h"
#include "BCGPMath.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CBCGPChartVisualObject, CBCGPBaseVisualObject)

UINT BCGM_ON_CHART_MOUSE_TRACK			= ::RegisterWindowMessage (_T("BCGM_ON_CHART_MOUSE_TRACK"));
UINT BCGM_ON_CHART_MOUSE_DOWN			= ::RegisterWindowMessage (_T("BCGM_ON_CHART_MOUSE_DOWN"));
UINT BCGM_ON_CHART_MOUSE_UP				= ::RegisterWindowMessage (_T("BCGM_ON_CHART_MOUSE_UP"));
UINT BCGM_ON_CHART_AFTER_DRAW			= ::RegisterWindowMessage (_T("BCGM_ON_CHART_AFTER_DRAW"));
UINT BCGM_ON_CHART_AFTER_RECALC_LAYOUT	= ::RegisterWindowMessage (_T("BCGM_ON_CHART_AFTER_RECALC_LAYOUT"));
UINT BCGM_ON_CHART_AXIS_SCROLLED		= ::RegisterWindowMessage (_T("BCGM_ON_CHART_AXIS_SCROLLED"));
UINT BCGM_ON_CHART_AXIS_ZOOMED			= ::RegisterWindowMessage (_T("BCGM_ON_CHART_AXIS_ZOOMED"));

UINT BCGM_ON_CHART_AFTER_BEGIN_3DDRAW	= ::RegisterWindowMessage (_T("BCGM_ON_CHART_AFTER_BEGIN_3DDRAW"));
UINT BCGM_ON_CHART_BEFORE_END_3DDRAW	= ::RegisterWindowMessage (_T("BCGM_ON_CHART_BEFORE_END_3DDRAW"));

double CBCGPChartDiagram3D::BCGP_DEFAULT_X_ROTATION = 45.0;
double CBCGPChartDiagram3D::BCGP_DEFAULT_Y_ROTATION = 20.0;
double CBCGPChartDiagram3D::BCGP_DEFAULT_PERSPECTIVE = 0.1;

CList<CBCGPChartVisualObject*, CBCGPChartVisualObject*>	CBCGPChartVisualObject::m_lstCharts;

/////////////////////////////////////////////////////////////////////////////
// CBCGPChartVisualObject

CBCGPChartDiagram3D::~CBCGPChartDiagram3D()
{
	DestroyEngine3D();	
}

void CBCGPChartDiagram3D::DestroyEngine3D()
{
	if (m_pEngine3D != NULL && m_bAutoDestroy3DEngine)
	{
		delete m_pEngine3D;
		m_pEngine3D = NULL;
	}
}

void CBCGPChartDiagram3D::SetCalculateNormals(BOOL bSet)
{
	if (m_bCalculateNormals == bSet)
	{
		return;
	}

	m_bCalculateNormals = bSet;

	if (m_pChart != NULL)
	{
		m_pChart->SetDirty(TRUE, FALSE);
	}
}

void CBCGPChartDiagram3D::SetWallsIgnoreRotation(BOOL bSet)
{
	if (m_bWallsIgnoreRotation == bSet)
	{
		return;
	}

	m_bWallsIgnoreRotation = bSet;
	SetEdges(TRUE);
	m_pChart->SetDirty(TRUE, FALSE);
}

void CBCGPChartDiagram3D::SetRenderingType(CBCGPEngine3D::BCGP_3D_RENDERING_TYPE rt, BOOL bRenderToWindow)
{
#if _MSC_VER < 1700 || !defined(_BCGSUITE_)
	if (!globalData.bIsWindowsVista && bRenderToWindow)
	{
		bRenderToWindow = FALSE;
	}
#endif

	if (m_pEngine3D != NULL && m_pEngine3D->GetRenderingType() == rt && 
		m_pEngine3D->IsRenderToWindow() == bRenderToWindow)
	{
		return;
	}

	DestroyEngine3D();

	m_pEngine3D = CBCGPEngine3D::CreateInstance(rt, bRenderToWindow);
	m_bSortNeeded = TRUE;
	SetEdges(TRUE);
}

void CBCGPChartDiagram3D::SetEngine3D(CBCGPEngine3D* pEngine, BOOL bAutoDestroy)
{
	ASSERT_VALID(pEngine);
	ASSERT_KINDOF(CBCGPEngine3D, pEngine);

	if (pEngine != m_pEngine3D)
	{
		DestroyEngine3D();
	}

	m_bAutoDestroy3DEngine = bAutoDestroy;
	m_pEngine3D = pEngine;
	m_bSortNeeded = TRUE;
	SetEdges(TRUE);
}

void CBCGPChartDiagram3D::SetChart(CBCGPChartVisualObject* pChart) 
{
	ASSERT_VALID(pChart);

	m_pChart = pChart;

	for (int i = 0; i < m_arWalls.GetSize(); i++)
	{
		m_arWalls[i].SetChart(pChart);
	}
}

void CBCGPChartDiagram3D::InitWalls()
{
	m_vertexes.RemoveAll();
	m_vertexes.SetSize(8);

	// back plane
	m_vertexes[0] = CBCGPPoint(1, 1, 1);
	m_vertexes[1] = CBCGPPoint(1, -1, 1);
	m_vertexes[2] = CBCGPPoint(-1, -1, 1);
	m_vertexes[3] = CBCGPPoint(-1, 1, 1);

	// +left plane
	m_vertexes[4] = CBCGPPoint(-1, 1, -1);
	m_vertexes[5] = CBCGPPoint(-1, -1, -1);

	// right point
	m_vertexes[6] = CBCGPPoint(1, -1, -1);

	// top point
	m_vertexes[7] = CBCGPPoint(1, 1, -1);

	// floor indexes - 1, 2, 5, 6

	InitWallShapes();
}

void CBCGPChartDiagram3D::InitWallShapes()
{
	static const int nWallCount = 5; // 1 floor + 4 walls

	if (m_arWalls.GetSize() == 0)
	{
		m_arWalls.SetSize(nWallCount); 
	}

	double dblTickness = m_pChart == NULL ? 6. : m_pChart->OnGetWallThickness();

	CBCGPPoint pt(dblTickness, 0, 0);
	m_mScaleInverse.TransformPoint3D(pt, pt);

	m_dblWallThicknessNormal = pt.x;

	CheckWallVisibilty();

	for (int i = 0; i < nWallCount; i++)
	{
		m_arWalls[i].SetChart(m_pChart);
		m_arWalls[i].SetVertexes(i, pt.x);
	}
}

void CBCGPChartDiagram3D::CollectWallAndFloorSides(CList<CBCGPChartSide3D*, CBCGPChartSide3D*>& lstInvisibleSides, CList<CBCGPChartSide3D*, CBCGPChartSide3D*>& lstVisibleSides)
{
	for (int i = 0; i < m_arWalls.GetSize(); i++)
	{
		if (m_arWalls[i].m_bVisible)
		{
			m_arWalls[i].AddSidesToList(lstInvisibleSides, FALSE);
			m_arWalls[i].AddSidesToList(lstVisibleSides, TRUE);
		}
	}
}

BOOL CBCGPChartDiagram3D::IsWallVisible(int nWallPosition)
{
	if (nWallPosition < 0 || nWallPosition >= m_arWalls.GetSize())
	{
		return FALSE;
	}

	return m_arWalls[nWallPosition].m_bVisible;
}

void CBCGPChartDiagram3D::InitEdges()
{
	m_edges.Add(CBCGPChartEdge3D(1, 0));			// 0
 	m_edges.Add(CBCGPChartEdge3D(1, 2, 7, 9));	// 1
 	m_edges.Add(CBCGPChartEdge3D(2, 3, 5, 10));	// 2
	m_edges.Add(CBCGPChartEdge3D(3, 0));			// 3
 	m_edges.Add(CBCGPChartEdge3D(4, 3));			// 4
	m_edges.Add(CBCGPChartEdge3D(5, 4, 2, 0));	// 5
 	m_edges.Add(CBCGPChartEdge3D(5, 2, 8, 11));	// 6
	m_edges.Add(CBCGPChartEdge3D(5, 6, 1, 3));	// 7
 	m_edges.Add(CBCGPChartEdge3D(6, 1, 6, 4));	// 8
	m_edges.Add(CBCGPChartEdge3D(4, 7));			// 9
	m_edges.Add(CBCGPChartEdge3D(6, 7));			// 10
	m_edges.Add(CBCGPChartEdge3D(0, 7));			// 11
}

void CBCGPChartDiagram3D::SetEdges(BOOL bForce)
{
	double dblXRotation = GetXRotation();

	if (m_bWallsIgnoreRotation && m_pEngine3D != NULL && 
		!m_pEngine3D->IsSoftwareRendering() && !IsThickWallsAndFloor())
	{
		// some value between [0, 90] to adjust axes positions in this range 
		dblXRotation = 10.;
	}

	if (m_dblLastXRotation == dblXRotation && !bForce || m_edges.GetSize() == 0)
	{
		return;
	}

	m_dblLastXRotation = dblXRotation;

	// build list of edges
	// data (3rd parameter) represents ID of related axis

	// clear all IDs first
	for (int i = 0; i < m_edges.GetSize(); i++)
	{
		m_edges[i].m_dwData = (DWORD_PTR)-1;
	}

	if (dblXRotation >= 0 &&  dblXRotation < 90)
	{
		m_edges[5].m_dwData = m_bIsXHorizontal ? BCGP_CHART_Y_PRIMARY_AXIS : BCGP_CHART_X_PRIMARY_AXIS;
		m_edges[5].m_nOppositeEdgeIndex = 2;

		m_edges[7].m_dwData = m_bIsXHorizontal ? BCGP_CHART_X_PRIMARY_AXIS : BCGP_CHART_Y_PRIMARY_AXIS;
		m_edges[8].m_dwData = BCGP_CHART_Z_PRIMARY_AXIS;
	}
	else if (dblXRotation >= 90 && dblXRotation < 180)
	{
		m_edges[2].m_dwData = m_bIsXHorizontal ? BCGP_CHART_Y_PRIMARY_AXIS : BCGP_CHART_X_PRIMARY_AXIS;
		m_edges[2].m_nOppositeEdgeIndex = 5;	

		m_edges[1].m_dwData = m_bIsXHorizontal ? BCGP_CHART_X_PRIMARY_AXIS : BCGP_CHART_Y_PRIMARY_AXIS;
		m_edges[1].m_nIndex1 = 2;
		m_edges[1].m_nIndex2 = 1;

		m_edges[8].m_dwData = BCGP_CHART_Z_PRIMARY_AXIS;
	}
	else if (dblXRotation >= 180 && dblXRotation < 270)
	{
		m_edges[5].m_dwData = m_bIsXHorizontal ? BCGP_CHART_Y_PRIMARY_AXIS : BCGP_CHART_X_PRIMARY_AXIS;
		m_edges[5].m_nOppositeEdgeIndex = 10;

		m_edges[1].m_dwData = m_bIsXHorizontal ? BCGP_CHART_X_PRIMARY_AXIS : BCGP_CHART_Y_PRIMARY_AXIS;
		m_edges[1].m_nIndex1 = 2;
		m_edges[1].m_nIndex2 = 1;

		m_edges[6].m_dwData = BCGP_CHART_Z_PRIMARY_AXIS;
	}
	else 
	{
		m_edges[2].m_dwData = m_bIsXHorizontal ? BCGP_CHART_Y_PRIMARY_AXIS : BCGP_CHART_X_PRIMARY_AXIS;
		m_edges[2].m_nOppositeEdgeIndex = 0;	

		m_edges[7].m_dwData = m_bIsXHorizontal ? BCGP_CHART_X_PRIMARY_AXIS : BCGP_CHART_Y_PRIMARY_AXIS;
		m_edges[6].m_dwData = BCGP_CHART_Z_PRIMARY_AXIS;
	}
}

void CBCGPChartDiagram3D::CheckWallVisibilty()
{
	if (m_pChart == NULL)
	{
		return;
	}

	double dblXRotation = GetXRotation();

	int nHorzAxisID = m_bIsXHorizontal ? BCGP_CHART_X_PRIMARY_AXIS : BCGP_CHART_Y_PRIMARY_AXIS;
	int nVertAxisID = m_bIsXHorizontal ? BCGP_CHART_Y_PRIMARY_AXIS : BCGP_CHART_X_PRIMARY_AXIS;

	for (int i = 0; i < m_arWalls.GetSize(); i++)
	{
		m_arWalls[i].m_bVisible = TRUE;
		m_arWalls[i].ResetSideData();
	}

	CBCGPChartSide3DArray& sidesFloor = (CBCGPChartSide3DArray&) m_arWalls[CBCGPChartShape3DWall::WALL_POS_FLOOR].GetSides();
	CBCGPChartSide3DArray& sidesLeft = (CBCGPChartSide3DArray&) m_arWalls[CBCGPChartShape3DWall::WALL_POS_LEFT].GetSides();
	CBCGPChartSide3DArray& sidesBack = (CBCGPChartSide3DArray&) m_arWalls[CBCGPChartShape3DWall::WALL_POS_BACK].GetSides();
	CBCGPChartSide3DArray& sidesFront = (CBCGPChartSide3DArray&) m_arWalls[CBCGPChartShape3DWall::WALL_POS_FRONT].GetSides();
	CBCGPChartSide3DArray& sidesRight = (CBCGPChartSide3DArray&) m_arWalls[CBCGPChartShape3DWall::WALL_POS_RIGHT].GetSides();

	// only X and Z axes draw on the floor, do not depend on rotation
	sidesFloor[CBCGPChartShape3DCube::CUBE_SIDE_BOTTOM].m_nAxisIDGrid1 = nHorzAxisID;
	sidesFloor[CBCGPChartShape3DCube::CUBE_SIDE_BOTTOM].m_bIsAxisGrid1Near = TRUE;
	sidesFloor[CBCGPChartShape3DCube::CUBE_SIDE_BOTTOM].m_nAxisIDGrid2 = BCGP_CHART_Z_PRIMARY_AXIS;
	sidesFloor[CBCGPChartShape3DCube::CUBE_SIDE_BOTTOM].m_bIsAxisGrid2Near = TRUE;

	if (dblXRotation >= 0 &&  dblXRotation < 90)
	{
		m_arWalls[CBCGPChartShape3DWall::WALL_POS_FRONT].m_bVisible = FALSE;
		m_arWalls[CBCGPChartShape3DWall::WALL_POS_RIGHT].m_bVisible = FALSE;
		
		// set position of x and z axes on the floor
		sidesFloor[CBCGPChartShape3DCube::CUBE_SIDE_FRONT].m_nAxisID = nHorzAxisID;
		sidesFloor[CBCGPChartShape3DCube::CUBE_SIDE_RIGHT].m_nAxisID = BCGP_CHART_Z_PRIMARY_AXIS;

		// set position of vertical axis on the left wall
		sidesLeft[CBCGPChartShape3DCube::CUBE_SIDE_FRONT].m_nAxisID = nVertAxisID;

		m_nLeftVisibleWallPos = CBCGPChartShape3DWall::WALL_POS_LEFT;
		m_nRightVisibleWallPos = CBCGPChartShape3DWall::WALL_POS_BACK;

		// grid lines and interlace on the left wall
		sidesLeft[CBCGPChartShape3DCube::CUBE_SIDE_RIGHT].m_nAxisIDGrid1 = nVertAxisID;
		sidesLeft[CBCGPChartShape3DCube::CUBE_SIDE_RIGHT].m_bIsAxisGrid1Near = TRUE;
		sidesLeft[CBCGPChartShape3DCube::CUBE_SIDE_RIGHT].m_nAxisIDGrid2 = BCGP_CHART_Z_PRIMARY_AXIS;
		sidesLeft[CBCGPChartShape3DCube::CUBE_SIDE_RIGHT].m_bIsAxisGrid2Near = FALSE;

		// grid lines and interlace on back wall
		sidesBack[CBCGPChartShape3DCube::CUBE_SIDE_FRONT].m_nAxisIDGrid1 = nVertAxisID;
		sidesBack[CBCGPChartShape3DCube::CUBE_SIDE_FRONT].m_bIsAxisGrid1Near = FALSE;
		sidesBack[CBCGPChartShape3DCube::CUBE_SIDE_FRONT].m_nAxisIDGrid2 = nHorzAxisID;
		sidesBack[CBCGPChartShape3DCube::CUBE_SIDE_FRONT].m_bIsAxisGrid2Near = FALSE;
	}
	else if (dblXRotation >= 90 && dblXRotation < 180)
	{
		m_arWalls[CBCGPChartShape3DWall::WALL_POS_BACK].m_bVisible = FALSE;
		m_arWalls[CBCGPChartShape3DWall::WALL_POS_RIGHT].m_bVisible = FALSE;		

		// x and z axes on the floor
		sidesFloor[CBCGPChartShape3DCube::CUBE_SIDE_BACK].m_nAxisID = nHorzAxisID;
		sidesFloor[CBCGPChartShape3DCube::CUBE_SIDE_RIGHT].m_nAxisID = BCGP_CHART_Z_PRIMARY_AXIS;

		// vertical axis on left wall
		sidesLeft[CBCGPChartShape3DCube::CUBE_SIDE_BACK].m_nAxisID = nVertAxisID;

		m_nLeftVisibleWallPos = CBCGPChartShape3DWall::WALL_POS_FRONT;
		m_nRightVisibleWallPos = CBCGPChartShape3DWall::WALL_POS_LEFT;

		// grid lines and interlace on the left wall
		sidesLeft[CBCGPChartShape3DCube::CUBE_SIDE_RIGHT].m_nAxisIDGrid1 = nVertAxisID;
		sidesLeft[CBCGPChartShape3DCube::CUBE_SIDE_RIGHT].m_bIsAxisGrid1Near = TRUE;
		sidesLeft[CBCGPChartShape3DCube::CUBE_SIDE_RIGHT].m_nAxisIDGrid2 = BCGP_CHART_Z_PRIMARY_AXIS;
		sidesLeft[CBCGPChartShape3DCube::CUBE_SIDE_RIGHT].m_bIsAxisGrid2Near = FALSE;

		// grid lines and interlace on the front wall
		sidesFront[CBCGPChartShape3DCube::CUBE_SIDE_BACK].m_nAxisIDGrid1 = nVertAxisID;
		sidesFront[CBCGPChartShape3DCube::CUBE_SIDE_BACK].m_bIsAxisGrid1Near = FALSE;
		sidesFront[CBCGPChartShape3DCube::CUBE_SIDE_BACK].m_nAxisIDGrid2 = nHorzAxisID;
		sidesFront[CBCGPChartShape3DCube::CUBE_SIDE_BACK].m_bIsAxisGrid2Near = FALSE;
	}
	else if (dblXRotation >= 180 && dblXRotation < 270)
	{
		m_arWalls[CBCGPChartShape3DWall::WALL_POS_BACK].m_bVisible = FALSE;
		m_arWalls[CBCGPChartShape3DWall::WALL_POS_LEFT].m_bVisible = FALSE;		
		
		// x and z axes on the floor
		sidesFloor[CBCGPChartShape3DCube::CUBE_SIDE_BACK].m_nAxisID = nHorzAxisID;
		sidesFloor[CBCGPChartShape3DCube::CUBE_SIDE_LEFT].m_nAxisID = BCGP_CHART_Z_PRIMARY_AXIS;

		// vertical axis on the front wall
		sidesFront[CBCGPChartShape3DCube::CUBE_SIDE_LEFT].m_nAxisID = nVertAxisID;

		m_nLeftVisibleWallPos = CBCGPChartShape3DWall::WALL_POS_RIGHT;
		m_nRightVisibleWallPos = CBCGPChartShape3DWall::WALL_POS_FRONT;

		// grid lines and interlace on the front wall
		sidesFront[CBCGPChartShape3DCube::CUBE_SIDE_BACK].m_nAxisIDGrid1 = nVertAxisID;
		sidesFront[CBCGPChartShape3DCube::CUBE_SIDE_BACK].m_bIsAxisGrid1Near = TRUE;
		sidesFront[CBCGPChartShape3DCube::CUBE_SIDE_BACK].m_nAxisIDGrid2 = nHorzAxisID;
		sidesFront[CBCGPChartShape3DCube::CUBE_SIDE_BACK].m_bIsAxisGrid2Near = FALSE;

		// grid lines and interlace on the right wall
		sidesRight[CBCGPChartShape3DCube::CUBE_SIDE_LEFT].m_nAxisIDGrid1 = nVertAxisID;
		sidesRight[CBCGPChartShape3DCube::CUBE_SIDE_LEFT].m_bIsAxisGrid1Near = FALSE;
		sidesRight[CBCGPChartShape3DCube::CUBE_SIDE_LEFT].m_nAxisIDGrid2 = BCGP_CHART_Z_PRIMARY_AXIS;
		sidesRight[CBCGPChartShape3DCube::CUBE_SIDE_LEFT].m_bIsAxisGrid2Near = FALSE;
	}
	else
	{
		m_arWalls[CBCGPChartShape3DWall::WALL_POS_LEFT].m_bVisible = FALSE;	
		m_arWalls[CBCGPChartShape3DWall::WALL_POS_FRONT].m_bVisible = FALSE;

		// X and Z axes on the floor
		sidesFloor[CBCGPChartShape3DCube::CUBE_SIDE_FRONT].m_nAxisID = nHorzAxisID;
		sidesFloor[CBCGPChartShape3DCube::CUBE_SIDE_LEFT].m_nAxisID = BCGP_CHART_Z_PRIMARY_AXIS;

		// vertical axis on the back
		sidesBack[CBCGPChartShape3DCube::CUBE_SIDE_LEFT].m_nAxisID = nVertAxisID;

		m_nLeftVisibleWallPos = CBCGPChartShape3DWall::WALL_POS_BACK;
		m_nRightVisibleWallPos = CBCGPChartShape3DWall::WALL_POS_RIGHT;

		// grid lines and interlace on the right wall
		sidesBack[CBCGPChartShape3DCube::CUBE_SIDE_FRONT].m_nAxisIDGrid1 = nVertAxisID;
		sidesBack[CBCGPChartShape3DCube::CUBE_SIDE_FRONT].m_bIsAxisGrid1Near = TRUE;
		sidesBack[CBCGPChartShape3DCube::CUBE_SIDE_FRONT].m_nAxisIDGrid2 = nHorzAxisID;
		sidesBack[CBCGPChartShape3DCube::CUBE_SIDE_FRONT].m_bIsAxisGrid2Near = FALSE;

		// grid lines and interlace on the back wall
		sidesRight[CBCGPChartShape3DCube::CUBE_SIDE_LEFT].m_nAxisIDGrid1 = nVertAxisID;
		sidesRight[CBCGPChartShape3DCube::CUBE_SIDE_LEFT].m_bIsAxisGrid1Near = FALSE;
		sidesRight[CBCGPChartShape3DCube::CUBE_SIDE_LEFT].m_nAxisIDGrid2 = BCGP_CHART_Z_PRIMARY_AXIS;
		sidesRight[CBCGPChartShape3DCube::CUBE_SIDE_LEFT].m_bIsAxisGrid2Near = FALSE;
	}
}

CBCGPPoint CBCGPChartDiagram3D::GetAxisPoint(int nAxisID, AxisPointType apt) const
{
	int nEdgeIndex = FindEdgeIndexByAxisID(nAxisID);

	if (nEdgeIndex == -1)
	{
		return CBCGPPoint();
	}

	const CBCGPChartEdge3D& edge = m_edges[nEdgeIndex];

	switch(apt)
	{
	case CBCGPChartDiagram3D::APT_PLANE:
		return m_vertexes[edge.m_nIndex1];
		
	case CBCGPChartDiagram3D::APT_FLOOR:
		if (edge.m_nOppositeEdgeIndex != (DWORD_PTR)-1)
		{
			const CBCGPChartEdge3D& edgeOpposite = m_edges[edge.m_nOppositeEdgeIndex];
			return m_vertexes[edgeOpposite.m_nIndex1];
		}
		break;		

	case CBCGPChartDiagram3D::APT_WALL:
		if (edge.m_nDiagonalEdgeIndex != (DWORD_PTR)-1)
		{
			const CBCGPChartEdge3D& edgeDiagonal = m_edges[edge.m_nDiagonalEdgeIndex];
			return m_vertexes[edgeDiagonal.m_nIndex1];
		}
		break;
	}

	return CBCGPPoint();
}

void CBCGPChartDiagram3D::SetXHorizontal(BOOL bSet)
{
	if (m_bIsXHorizontal == bSet)
	{
		return;
	}

	m_bIsXHorizontal = bSet;
	SetEdges(TRUE);

	m_bLayoutChanged = TRUE;

	if (m_pChart != NULL)
	{
		m_pChart->SetDirty(TRUE, FALSE);
	}
}

BOOL CBCGPChartDiagram3D::GetNormalAxisCoordinates(int nAxisID, CBCGPPoint& ptStart, CBCGPPoint& ptEnd)
{
	if (IsThickWallsAndFloor())
	{
		ptStart.SetPoint(0., 0., 0.);
		ptEnd.SetPoint(0., 0., 0.);

		for (int i = 0; i < m_arWalls.GetSize(); i++)
		{
			CBCGPChartShape3DWall& wall = m_arWalls[i];
			if (wall.GetAxisCoordinates(nAxisID, ptStart, ptEnd))
			{
				return TRUE;
			}
		}


		return FALSE;
	}

	for (int i = 0; i < m_edges.GetSize(); i++)
	{
		const CBCGPChartEdge3D& edge = m_edges[i];

		if (edge.m_dwData == (DWORD_PTR)nAxisID)
		{
			ptStart = m_vertexes[edge.m_nIndex1];
			ptEnd = m_vertexes[edge.m_nIndex2];
			return TRUE;
		}
	}

	return FALSE;
}

int CBCGPChartDiagram3D::FindEdgeIndexByAxisID(int nAxisID) const
{
	for (int i = 0; i < m_edges.GetSize(); i++)
	{
		const CBCGPChartEdge3D& edge = m_edges[i];

		if (edge.m_dwData == (DWORD_PTR)nAxisID)
		{
			return i;
		}
	}

	return -1;
}

void CBCGPChartDiagram3D::Reset(BOOL bRedraw)
{
	SetPosition(BCGP_DEFAULT_X_ROTATION, BCGP_DEFAULT_Y_ROTATION, BCGP_DEFAULT_PERSPECTIVE);
	m_dblZRotation = 270.;

	m_dblDepthScalePercent = 0.;
	m_dblHeightScalePercent = 1.;

	if (bRedraw)
	{
		m_pChart->Redraw();
	}
}

void CBCGPChartDiagram3D::SetPosition(double dblXRotation, double dblYRotation, double dblPerspectivePercent)
{
	m_dblXRotation = bcg_normalize_deg(dblXRotation) + 90.;
	m_dblYRotation = bcg_clamp(dblYRotation, -90., 90.);

	m_dblPerspective = bcg_clamp(dblPerspectivePercent, 0., 1.);

	m_bLayoutChanged = TRUE;

	if (m_pChart != NULL)
	{
		m_pChart->SetDirty(TRUE, FALSE);
	}

	SetEdges();
}

void CBCGPChartDiagram3D::SetZRotation(double dblZRotation)
{
	m_dblZRotation = bcg_clamp(dblZRotation, 0., 360.);
	m_bLayoutChanged = TRUE;

	if (m_pChart != NULL)
	{
		m_pChart->SetDirty(TRUE, FALSE);
	}
}

void CBCGPChartDiagram3D::SetHeightScalePercent(double dblHeightPercent)
{
	m_dblHeightScalePercent = bcg_clamp(dblHeightPercent, .1, 10.);

	m_bLayoutChanged = TRUE;

	if (m_pChart != NULL)
	{
		m_pChart->SetDirty(TRUE, FALSE);
	}
}

void CBCGPChartDiagram3D::SetDepthScalePercent(double dblDepthPercent)
{
	m_dblDepthScalePercent = bcg_clamp(dblDepthPercent, .1, 10.);
	
	m_bLayoutChanged = TRUE;

	if (m_pChart != NULL)
	{
		m_pChart->SetDirty(TRUE, FALSE);
	}
}

double CBCGPChartDiagram3D::GetDepthScalePercent() const
{
	if (m_dblDepthScalePercent != 0.)
	{
		return m_dblDepthScalePercent;
	}

	if (m_pChart != NULL)
	{
		switch (m_pChart->GetChartCategory())
		{
		case BCGPChartArea3D:
			return 2.;
		}
	}

	return 1.;
}

void CBCGPChartDiagram3D::SetFrontDistancePercent(double dblDistancePercent)
{
	m_dblFrontDistancePercent = bcg_clamp(dblDistancePercent, 0., 0.9);
}

void CBCGPChartDiagram3D::SetZoomFactor(double dblZoom)
{
	m_bLayoutChanged = TRUE;
	m_dblZoomFactor = bcg_clamp(dblZoom, 0.1, 10.);
	CreateScaleMatrix();
}

void CBCGPChartDiagram3D::SetBaseDepthPercent(double dblBaseDepthPercent)
{
	dblBaseDepthPercent = bcg_clamp(dblBaseDepthPercent, .01, 1.);

	if (fabs(dblBaseDepthPercent - m_dblBaseDepthPercent) < 2 * DBL_EPSILON)
	{
		return;
	}

	InitWalls();

	m_dblBaseDepthPercent = dblBaseDepthPercent;

	int i = 0;
	for (i = 0; i < m_vertexes.GetSize(); i++)
	{
		CBCGPPoint pt = m_vertexes[i];
		pt.z *= m_dblBaseDepthPercent;
		m_vertexes[i] = pt;
	}

	m_bLayoutChanged = TRUE;

	if (m_pChart != NULL)
	{
		m_pChart->SetDirty(TRUE, FALSE);
	}
	
}

void CBCGPChartDiagram3D::SetGrouped(BOOL bSet, BOOL bRedraw)
{
	m_bGrouped = bSet;
	m_bLayoutChanged = TRUE;

	if (m_pChart != NULL)
	{
		m_pChart->SetDirty(TRUE, bRedraw);
	}
}

void CBCGPChartDiagram3D::SetExplicitGrouping(ExplicitGrouping grouping, BOOL bRedraw)
{
	m_explicitGrouping = grouping;

	if (m_pChart != NULL)
	{
		m_pChart->SetDirty(TRUE, bRedraw);
	}
}

void CBCGPChartDiagram3D::FillRotationMatrix(CBCGPMatrix4x4& m, double dblAngle, int nAxis)
{
	switch(nAxis)
	{
	// X axis
	case 0:
		m[1][1] = cos(dblAngle);
		m[1][2] = sin(dblAngle);
		m[2][1] = -sin(dblAngle);
		m[2][2] = cos(dblAngle);
		break;

	// Y axis
	case 1:
		m[0][0] = cos(dblAngle);
		m[0][2] = -sin(dblAngle);
		m[2][0] = sin(dblAngle);
		m[2][2] = cos(dblAngle);
		break;

	// Z axis
	case 2:
		m[0][0] = cos(dblAngle);
		m[0][1] = sin(dblAngle);
		m[1][0] = -sin(dblAngle);
		m[1][1] = cos(dblAngle);
		break;
	}
}

void CBCGPChartDiagram3D::CreateTransformationMatrix()
{
	double dblPhi = bcg_deg2rad(m_dblXRotation);
	double dblPsi = bcg_deg2rad(m_dblYRotation);
	double dblTheta = bcg_deg2rad(m_dblZRotation);

	m_mTransform.Identity();

	CBCGPMatrix4x4	mPhi;
	FillRotationMatrix(mPhi, dblPhi, 1);

	CBCGPMatrix4x4 mPsi;
	FillRotationMatrix(mPsi, dblPsi, 2);

	CBCGPMatrix4x4 mRes;
	mRes.MultiplyMatrixes4x4(mPhi, mPsi);

	CBCGPMatrix4x4 mTheta;
	FillRotationMatrix(mTheta, dblTheta, 1);

	CBCGPMatrix4x4 mResRotation;
	mResRotation.MultiplyMatrixes4x4(mRes, mTheta);

	CBCGPMatrix4x4 mPrj;
	m_mTransform.MultiplyMatrixes4x4(mPrj, mResRotation);

	double dblCoef = m_dblPerspective / 3.6 ;

	// perspective point X and Y
 	m_mTransform[0][3] = dblCoef * cos(bcg_deg2rad(m_dblXRotation));
 	m_mTransform[1][3] = -dblCoef * sin(bcg_deg2rad(m_dblYRotation));

	double dblDepthScalePercent = GetDepthScalePercent();

	if (dblDepthScalePercent > 1.)
	{
 		m_mTransform[2][3] = dblCoef * cos(bcg_deg2rad(m_dblXRotation - 90.)) * 
			cos(bcg_deg2rad(m_dblYRotation)) / dblDepthScalePercent;
	}
	else
	{
 		m_mTransform[2][3] = dblCoef * cos(bcg_deg2rad(m_dblXRotation - 90.)) * 
 			cos(bcg_deg2rad(m_dblYRotation));
	}

	m_mTransformInverse.CopyFrom(m_mTransform);
	m_mTransformInverse.Inverse();
}

void CBCGPChartDiagram3D::CreateScaleMatrix()
{
	m_mScale.Identity();

	double dblRangeX = m_dblXMax - m_dblXMin;
	double dblRangeY = m_dblYMax - m_dblYMin;

	double dblXScale = m_rectDiagram.Width() / dblRangeX;
	double dblYScale = m_rectDiagram.Height() / dblRangeY;
	double dblZScale = 2. / (m_dblZMax - m_dblZMin) * m_dblZoomFactor;

	double dblMinScale = min(dblXScale, dblYScale);

	m_mScale[2][2] = dblZScale;

	if (m_bProportionalScale || m_rectDiagram.Width() < m_rectDiagram.Height())
	{
		m_mScale[0][0] = dblMinScale * m_dblZoomFactor;
		m_mScale[1][1] = dblMinScale * m_dblZoomFactor;
	}
	else
	{
   		m_mScale[0][0] = min(dblXScale, dblMinScale * m_rectDiagram.Width() / m_rectDiagram.Height()) * m_dblZoomFactor;
   		m_mScale[1][1] = dblMinScale * m_dblZoomFactor;
	}

	m_mScaleInverse.CopyFrom(m_mScale);
	m_mScaleInverse.Inverse();
}

void CBCGPChartDiagram3D::ScalePoint(CBCGPPoint& ptInOut, const CBCGPPoint& ptDiagramCenter) const
{
	ptInOut.x -= m_ptZero.x;
	ptInOut.y -= m_ptZero.y;
	ptInOut.z -= m_ptZero.z;

	m_mScale.TransformPoint3D(ptInOut, ptInOut);

	ptInOut.x += ptDiagramCenter.x + m_ptScrollOffset.x;
	ptInOut.y = ptDiagramCenter.y  + m_ptScrollOffset.y - ptInOut.y;
}

void CBCGPChartDiagram3D::ScalePointInverse(CBCGPPoint& ptInOut, const CBCGPPoint& ptDiagramCenter)
{
	ptInOut.x -= ptDiagramCenter.x + m_ptScrollOffset.x;
	ptInOut.y = ptDiagramCenter.y  + m_ptScrollOffset.y - ptInOut.y;

	m_mScaleInverse.TransformPoint3D(ptInOut, ptInOut);

	ptInOut.x += m_ptZero.x;
	ptInOut.y += m_ptZero.y;
	ptInOut.z += m_ptZero.z;
}

CBCGPPoint	CBCGPChartDiagram3D::TransfromPointInverse(const CBCGPPoint& pt, const CBCGPPoint& ptDiagramCenter, BOOL bUnScale)
{
	CBCGPPoint ptCenter = ptDiagramCenter;

	if (ptDiagramCenter.x == -1 && ptDiagramCenter.y == -1)
	{
		ptCenter = m_rectDiagram.CenterPoint();
	}

	CBCGPPoint ptTransformed = pt;

	if (bUnScale)
	{
		ScalePointInverse(ptTransformed, ptCenter);
	}

	m_mTransformInverse.TransformPoint3D(ptTransformed, ptTransformed);

	ptTransformed.z /= GetDepthScalePercent();
	ptTransformed.y /= GetHeightScalePercent();

	return ptTransformed;
}

CBCGPPoint CBCGPChartDiagram3D::TransformPoint(const CBCGPPoint& pt, const CBCGPPoint& ptDiagramCenter, BOOL bScale) const
{
	CBCGPPoint ptCenter = ptDiagramCenter;

	if (ptDiagramCenter.x == -1 && ptDiagramCenter.y == -1)
	{
		ptCenter = m_rectDiagram.CenterPoint();
	}


	CBCGPPoint ptTransformed = pt;

	ptTransformed.z *= GetDepthScalePercent();
	ptTransformed.y *= m_dblHeightScalePercent;

	m_mTransform.TransformPoint3D(ptTransformed, ptTransformed);

	if (bScale)
	{
		ScalePoint(ptTransformed, ptCenter);
	}

	return ptTransformed;
}

void CBCGPChartDiagram3D::TransformPointOpt(CBCGPPoint& ptInOut, double dblDepthScalePercent, const CBCGPPoint& ptDiagramCenter, BOOL bScale) const
{
	ptInOut.z *= dblDepthScalePercent;
	ptInOut.y *= m_dblHeightScalePercent;

	m_mTransform.TransformPoint3D(ptInOut, ptInOut);

	if (bScale)
	{
		ScalePoint(ptInOut, ptDiagramCenter);
	}
}

void CBCGPChartDiagram3D::TransformVector4(const CBCGPVector4& vIn, CBCGPVector4& vOut, const CBCGPPoint& ptDiagramCenter, BOOL bScale) const
{
	CBCGPPoint pt;
	vIn.ToPoint(pt);
	pt = TransformPoint(pt, ptDiagramCenter, bScale);
	vOut.FromPoint3D(pt);
}

CBCGPPoint CBCGPChartDiagram3D::TranslateDistance(const CBCGPPoint& pt) // in pixels on the diagram
{
	CBCGPPoint ptRes;
	m_mScaleInverse.TransformPoint3D(pt, ptRes);

	return ptRes;
}

void CBCGPChartDiagram3D::TransformShape(CBCGPChartShape3D& shape, BOOL bCheckVisibility)
{
	const CBCGPPointsArray& arVertexes = shape.GetVertexes();
	CBCGPPointsArray transformedVertexes((int)arVertexes.GetSize());

	CBCGPPoint ptCenter = m_rectDiagram.CenterPoint();

	for (int i = 0; i < arVertexes.GetSize(); i++)
	{
		transformedVertexes[i] = TransformPoint(arVertexes[i], ptCenter);
	}

	shape.SetFinalVertexes(transformedVertexes);

	if (bCheckVisibility)
	{
		for (int i = 0; i < arVertexes.GetSize(); i++)
		{
			transformedVertexes[i] = TransformPoint(arVertexes[i], ptCenter, FALSE);
		}

		shape.SetRotatedVertexes(transformedVertexes);
	}
	
	shape.OnAfterTransform(this);
}

void CBCGPChartDiagram3D::SetDrawWallOptions(CBCGPChartDiagram3D::DrawWallOptions options, BOOL bRedraw)
{
	m_drawWallOptions = options;

	if (bRedraw && m_pChart != NULL)
	{
		m_pChart->Redraw();
	}
}

void CBCGPChartDiagram3D::StripWallsAndAxes(BOOL bStrip, BOOL bRedraw)
{
	if (bStrip)
	{
		m_drawWallOptions &= ~(CBCGPChartDiagram3D::DWO_DRAW_ALL_WALLS);
	}
	else
	{
		m_drawWallOptions |= (CBCGPChartDiagram3D::DWO_OUTLINE_ALL_WALLS);
	}

	for (int i = 0; i < m_edges.GetSize(); i++)
	{
		const CBCGPChartEdge3D& edge = m_edges[i];

		CBCGPChartAxis* pAxis = m_pChart->GetChartAxis((int)edge.m_dwData);

		if (pAxis == NULL)
		{
			continue;
		}

		if (pAxis->IsVertical())
		{
			pAxis->m_bVisible = !bStrip;
		}
	}

	if (bRedraw && m_pChart != NULL)
	{
		m_pChart->Redraw();
	}
}

void CBCGPChartDiagram3D::SetThickWallsAndFloor(BOOL bSet, BOOL bRedraw)
{
	if (m_bThickWallsAndFloor == bSet)
	{
		return;
	}

	m_bThickWallsAndFloor = bSet;
	m_bLayoutChanged = TRUE;

	if (m_pChart != NULL)
	{
		m_pChart->SetDirty(TRUE, bRedraw);
	}
}

void CBCGPChartDiagram3D::OnBegin3DDraw(CBCGPGraphicsManager* pGM)
{
	if (m_pEngine3D == NULL)
	{
		return;
	}

	ASSERT_VALID(m_pEngine3D);

	m_pEngine3D->BeginDraw(pGM);

	if (m_pChart != NULL)
	{
		ASSERT_VALID(m_pChart);
		m_pChart->OnAfterBegin3DDraw(pGM);
	}
}

void CBCGPChartDiagram3D::OnEnd3DDraw(const CBCGPRect& rectTarget, CBCGPGraphicsManager* pGMTarget)
{
	if (m_pEngine3D == NULL)
	{
		return;
	}

	ASSERT_VALID(m_pEngine3D);

	if (m_pChart != NULL)
	{
		ASSERT_VALID(m_pChart);
		m_pChart->OnBeforeEnd3DDraw(rectTarget, pGMTarget);
	}

	m_pEngine3D->EndDraw(rectTarget, pGMTarget);
}

void CBCGPChartDiagram3D::OnDraw(CBCGPGraphicsManager* pGM, BOOL bFill, BOOL bOutline)
{
	ASSERT_VALID(m_pEngine3D);

	if (IsThickWallsAndFloor())
	{
		return;
	}

	OnDrawFloor(pGM, bFill, bOutline);
	OnDrawWalls(pGM, bFill, bOutline);
}

void CBCGPChartDiagram3D::OnDrawThickWalls(CBCGPEngine3D* pEngine)
{
	if (!IsThickWallsAndFloor())
	{
		return;
	}

	for (int i = 0; i < m_arWalls.GetSize(); i++)
	{
		if (m_arWalls[i].m_bVisible)
		{
			m_arWalls[i].OnDraw(pEngine);
		}
	}
}

void CBCGPChartDiagram3D::OnDrawFloor(CBCGPGraphicsManager* /*pGM*/, BOOL bFill, BOOL bOutline)
{
	if (m_pChart == NULL || (m_drawWallOptions & CBCGPChartDiagram3D::DWO_DRAW_FLOOR) == 0)
	{
		return;
	}
 
 	ASSERT_VALID(m_pChart);

	const CBCGPChartTheme& theme = m_pChart->GetColors();
	const CBCGPBrush& brFloor = m_formatWalls.m_brBottomFillColor.IsEmpty() ? 
		theme.m_brFloorColor3D : m_formatWalls.m_brBottomFillColor;
	const CBCGPBrush& brFloorLine = m_formatWalls.m_outlineFormatFloor.m_brLineColor.IsEmpty() ? 
		theme.m_brAxisMajorGridLineColor : m_formatWalls.m_outlineFormatFloor.m_brLineColor;

	// floor indexes - 1, 2, 5, 6
	ASSERT_VALID(m_pEngine3D);

	if (m_bCalculateNormals)
	{
		CBCGPVector4 v;
		v.CalcNormal(m_arScreenPoints[1], m_arScreenPoints[2], m_arScreenPoints[5]);
		m_pEngine3D->SetPolygonNormal(v[0], v[1], v[2]);
	}

	CBCGPPointsArray arPoints;
	arPoints.Add(m_arScreenPoints[1]);
	arPoints.Add(m_arScreenPoints[2]);
	arPoints.Add(m_arScreenPoints[5]);
	arPoints.Add(m_arScreenPoints[6]);

	if (bFill && (m_drawWallOptions & CBCGPChartDiagram3D::DWO_FILL_FLOOR) != 0 && 
		(m_dblYRotation > 0 || !m_pEngine3D->IsSoftwareRendering()))
	{
		m_pEngine3D->DrawSide(m_arScreenPoints[1], m_arScreenPoints[2], m_arScreenPoints[5], m_arScreenPoints[6], 
			brFloor, CBCGPBrush());
	}

	if (bOutline && (m_drawWallOptions & CBCGPChartDiagram3D::DWO_OUTLINE_FLOOR) != 0)
	{
		DrawLinesCheckAxes(arPoints, brFloorLine, m_formatWalls.m_outlineFormat.m_dblWidth);
	}
}

void CBCGPChartDiagram3D::OnDrawWalls(CBCGPGraphicsManager* pGM, BOOL bFill, BOOL bOutline)
{
	ASSERT_VALID(pGM);

	if (m_pChart == NULL || (m_drawWallOptions & CBCGPChartDiagram3D::DWO_DRAW_ALL_WALLS) == 0)
	{
		return;
	}
 
 	ASSERT_VALID(m_pChart);

	const CBCGPChartTheme& theme = m_pChart->GetColors();
	const CBCGPBrush& brWallRight = m_formatWalls.m_brFillColor.IsEmpty() ? 
		theme.m_brRightWallColor3D : m_formatWalls.m_brFillColor;

	const CBCGPBrush& brWallLeft = m_formatWalls.m_brSideFillColor.IsEmpty() ? 
		theme.m_brLeftWallColor3DSide : m_formatWalls.m_brSideFillColor;

	const CBCGPBrush& brWallLineRight = m_formatWalls.m_outlineFormat.m_brLineColor.IsEmpty()  ? 
		theme.m_brAxisMajorGridLineColor : m_formatWalls.m_outlineFormat.m_brLineColor;
	const CBCGPBrush& brWallLineLeft = m_formatWalls.m_outlineFormatLeftWall.m_brLineColor.IsEmpty()  ? 
		theme.m_brAxisMajorGridLineColor : m_formatWalls.m_outlineFormatLeftWall.m_brLineColor;

	// draw walls
	
	int nHorzAxisIndex = m_bIsXHorizontal ? BCGP_CHART_X_PRIMARY_AXIS : BCGP_CHART_Y_PRIMARY_AXIS;

	OnDrawWallByAxisID(pGM, nHorzAxisIndex, brWallRight, brWallLineRight, m_formatWalls.m_outlineFormat.m_dblWidth, bFill, bOutline);
	OnDrawWallByAxisID(pGM, BCGP_CHART_Z_PRIMARY_AXIS, brWallLeft, brWallLineLeft, m_formatWalls.m_outlineFormat.m_dblWidth, bFill, bOutline);
}

void CBCGPChartDiagram3D::OnDrawWallByAxisID(CBCGPGraphicsManager* /*pGM*/, int nAxisIndex, 
							const CBCGPBrush& brWallColorFill, const CBCGPBrush& brWallColorLine, 
							double dblLineWidth, BOOL bFill, BOOL bOutline)
{
	int nEdgeIndexX = FindEdgeIndexByAxisID(nAxisIndex);

	if (nEdgeIndexX == -1)
	{
		return;
	}

	const CBCGPChartEdge3D& edge = m_edges[nEdgeIndexX];

	const CBCGPChartEdge3D& edgeOpposite = m_edges[edge.m_nOppositeEdgeIndex];
	const CBCGPChartEdge3D& edgeDiagonal = m_edges[edge.m_nDiagonalEdgeIndex];

	CBCGPPoint pt1 = m_arScreenPoints[edgeOpposite.m_nIndex1];
	CBCGPPoint pt2 = m_arScreenPoints[edgeOpposite.m_nIndex2];
	CBCGPPoint pt3 = m_arScreenPoints[edgeDiagonal.m_nIndex1];
	CBCGPPoint pt4 = m_arScreenPoints[edgeDiagonal.m_nIndex2];

	CBCGPPoint ptIntersect;
	if (BCGPIntersectPoints2D(pt1, pt4, pt2, pt3, ptIntersect))
	{
		pt4 = m_arScreenPoints[edgeDiagonal.m_nIndex1];
		pt3 = m_arScreenPoints[edgeDiagonal.m_nIndex2];
	}
	
	ASSERT_VALID(m_pEngine3D);

	if (m_bCalculateNormals)
	{
		CBCGPVector4 v;
		v.CalcNormal(pt1, pt2, pt3);
		m_pEngine3D->SetPolygonNormal(v[0], v[1], v[2]);
	}

	CBCGPPointsArray arPoints;
	arPoints.Add(pt1);
	arPoints.Add(pt2);
	arPoints.Add(pt3);
	arPoints.Add(pt4);

	BOOL bLeftWall = nAxisIndex == BCGP_CHART_Z_PRIMARY_AXIS;

	if (bFill)
	{
		if (bLeftWall && ((m_drawWallOptions & CBCGPChartDiagram3D::DWO_FILL_LEFT_WALL) != 0) || 
			!bLeftWall && ((m_drawWallOptions & CBCGPChartDiagram3D::DWO_FILL_RIGHT_WALL) != 0))
		{
			m_pEngine3D->DrawSide(pt1, pt2, pt3, pt4, brWallColorFill, CBCGPBrush());
		}
	}

	if (bOutline)
	{
		if (bLeftWall && ((m_drawWallOptions & CBCGPChartDiagram3D::DWO_OUTLINE_LEFT_WALL) != 0) || 
			!bLeftWall && ((m_drawWallOptions & CBCGPChartDiagram3D::DWO_OUTLINE_RIGHT_WALL) != 0))
		{
			DrawLinesCheckAxes(arPoints, brWallColorLine, dblLineWidth);
		}
	}
}

void CBCGPChartDiagram3D::DrawLinesCheckAxes(const CBCGPPointsArray& arPoints, const CBCGPBrush& brLine, double dblLineWidth)
{
	CBCGPChartAxis* axes[3];
	axes[0] = m_pChart->m_arAxes[BCGP_CHART_X_PRIMARY_AXIS];
	axes[1] = m_pChart->m_arAxes[BCGP_CHART_Y_PRIMARY_AXIS];
	axes[2] = m_pChart->m_arAxes[BCGP_CHART_Z_PRIMARY_AXIS];

	double dblLinePrecision = 0.5;
	int nSize = (int)arPoints.GetSize();

	for (int j = 0; j < nSize; j++)
	{
		BOOL bCanDrawLine = TRUE;
		const CBCGPPoint& ptStart = arPoints[j];
		const CBCGPPoint& ptEnd = arPoints[(j + 1) % nSize];

		for (int i = 0; i < 3; i++)
		{
			CBCGPChartAxis* pAxis = axes[i];

			if (pAxis == NULL || !pAxis->IsAxisVisible())
			{
				continue;
			}

			CBCGPPoint ptAxisStart;
			CBCGPPoint ptAxisEnd;

			pAxis->GetAxisPos(ptAxisStart, ptAxisEnd);

			if (bcg_pointInLine(ptAxisStart, ptAxisEnd, ptStart, dblLinePrecision) != 0 && 
				bcg_pointInLine(ptAxisStart, ptAxisEnd, ptEnd, dblLinePrecision) != 0)
			{
				bCanDrawLine = FALSE;
				break;
			}
		}

		if (bCanDrawLine)
		{
			m_pEngine3D->DrawLine(ptStart, ptEnd, brLine, dblLineWidth);
		}
	}
}

void CBCGPChartDiagram3D::AdjustLayout(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagram)
{
	CBCGPRect rectBounds = rectDiagram;

	rectBounds.DeflateRect(m_szMaxLabelSize.cx, m_szMaxLabelSize.cy); //TODO deflate axis label size

	m_rectDiagram = rectBounds;
	m_pChart->m_rectDiagramArea = m_rectDiagram;

	CBCGPPoint ptCenter = m_rectDiagram.CenterPoint();
	BOOL bLayoutWasChanged = m_bLayoutChanged;

	if (m_bLayoutChanged)
	{
		CreateTransformationMatrix();

		m_dblXMin = DBL_MAX;
		m_dblXMax = -DBL_MAX;
		m_dblYMin = DBL_MAX;
		m_dblYMax = -DBL_MAX;
		m_dblZMin = DBL_MAX;
		m_dblZMax = -DBL_MAX;

		for (int i = 0; i < m_vertexes.GetSize(); i++)
		{
			CBCGPPoint pt = m_vertexes[i];
			
			pt.z *= GetDepthScalePercent();
			pt.y *= m_dblHeightScalePercent;

			m_mTransform.TransformPoint3D(pt, pt);

			m_dblXMin = min(m_dblXMin, pt.x);
			m_dblXMax = max(m_dblXMax, pt.x);
			m_dblYMin = min(m_dblYMin, pt.y);
			m_dblYMax = max(m_dblYMax, pt.y);
			m_dblZMin = min(m_dblZMin, pt.z);
			m_dblZMax = max(m_dblZMax, pt.z);
		}

		m_ptZero.x = m_dblXMin + (m_dblXMax - m_dblXMin) / 2;
		m_ptZero.y = m_dblYMin + (m_dblYMax - m_dblYMin) / 2;
		m_ptZero.z = 0;

		m_bLayoutChanged = FALSE;
	}

	if (m_pEngine3D != NULL)
	{
		CBCGPRect rPlot;
		m_pChart->OnGetPlotAreaRect(rPlot);
		rPlot.left++;
		rPlot.top++;
		rPlot.right--;

		m_pEngine3D->SetSceneRectAndDepth(rPlot, m_dblZMin, m_dblZMax);
	}

	m_arScreenPoints.RemoveAll();

	CreateScaleMatrix();

	if (bLayoutWasChanged)
	{
		InitWallShapes();
	}

	int i = 0;
	for (i = 0; i < m_vertexes.GetSize(); i++)
	{
		CBCGPPoint pt = m_vertexes[i];
		m_arScreenPoints.Add(TransformPoint(pt, ptCenter));
	}

	for (i = 0; i < m_arWalls.GetSize(); i++)
	{
		TransformShape(m_arWalls[i], TRUE);
		m_arWalls[i].PrepareSidePoints();
	}

	CBCGPSize szMaxLabelSize;

	for (i = 0; i < m_edges.GetSize(); i++)
	{
		const CBCGPChartEdge3D& edge = m_edges[i];

		CBCGPChartAxis* pAxis = m_pChart->GetChartAxis((int)edge.m_dwData);

		if (pAxis == NULL)
		{
			continue;
		}

		pAxis->CalcAxisPos3D(rectDiagram, FALSE);
		pAxis->CalcMajorMinorUnits(pGM);
		pAxis->CalcMaxLabelSize(pGM);

		szMaxLabelSize.cx = max(szMaxLabelSize.cx, pAxis->m_szMaxLabelSize.cx + pAxis->m_dblLabelDistance + pAxis->GetMajorTickMarkLen(FALSE)); 
		szMaxLabelSize.cy = max(szMaxLabelSize.cy, pAxis->m_szMaxLabelSize.cy + pAxis->m_dblLabelDistance + pAxis->GetMajorTickMarkLen(FALSE)); 
	}

	if (szMaxLabelSize.cx > m_szMaxLabelSize.cx || szMaxLabelSize.cy > m_szMaxLabelSize.cy)
	{
		m_szMaxLabelSize = szMaxLabelSize;
		m_bLayoutChanged = TRUE;
		AdjustLayout(pGM, rectDiagram);
	}

	for (i = 0; i < m_edges.GetSize(); i++)
	{
		const CBCGPChartEdge3D& edge = m_edges[i];

		CBCGPChartAxis* pAxis = m_pChart->GetChartAxis((int)edge.m_dwData);

		if (pAxis == NULL)
		{
			continue;
		}

		pAxis->CalcAxisPos3D(rectDiagram, FALSE);
		pAxis->CalcMajorMinorUnits(pGM);
	}
}

CBCGPChartVisualObject::CBCGPChartVisualObject(CBCGPVisualContainer* pContainer) :
	CBCGPBaseVisualObject(pContainer), m_currentTheme(CBCGPChartTheme::CT_DEFAULT)
{
	m_lstCharts.AddTail(this);

	m_Category = BCGPChartLine;
	m_Type = BCGP_CT_SIMPLE;

	CBCGPChartAxis* pAxis = new CBCGPChartAxisX(BCGP_CHART_X_PRIMARY_AXIS, CBCGPChartAxis::ADP_BOTTOM, this);
	m_arAxes.SetAtGrow(BCGP_CHART_X_PRIMARY_AXIS, pAxis);

	pAxis = new CBCGPChartAxisX(BCGP_CHART_X_SECONDARY_AXIS, CBCGPChartAxis::ADP_TOP, this);
	m_arAxes.SetAtGrow(BCGP_CHART_X_SECONDARY_AXIS, pAxis);

	pAxis = new CBCGPChartAxisY(BCGP_CHART_Y_PRIMARY_AXIS, CBCGPChartAxis::ADP_LEFT, this);
	m_arAxes.SetAtGrow(BCGP_CHART_Y_PRIMARY_AXIS, pAxis);

	pAxis = new CBCGPChartAxisY(BCGP_CHART_Y_SECONDARY_AXIS, CBCGPChartAxis::ADP_RIGHT, this);
	m_arAxes.SetAtGrow(BCGP_CHART_Y_SECONDARY_AXIS, pAxis);

	pAxis = new CBCGPChartAxisZ(BCGP_CHART_Z_PRIMARY_AXIS, CBCGPChartAxis::ADP_DEPTH_BOTTOM, this);
	m_arAxes.SetAtGrow(BCGP_CHART_Z_PRIMARY_AXIS, pAxis);

	pAxis = new CBCGPChartAxisPolarY(BCGP_CHART_Y_POLAR_AXIS, CBCGPChartAxis::ADP_LEFT, this);
	m_arAxes.SetAtGrow(BCGP_CHART_Y_POLAR_AXIS, pAxis);

	pAxis = new CBCGPChartAxisPolarX(BCGP_CHART_X_POLAR_AXIS, CBCGPChartAxis::ADP_BOTTOM, this);
	m_arAxes.SetAtGrow(BCGP_CHART_X_POLAR_AXIS, pAxis);

	pAxis = new CBCGPChartTernaryAxis(BCGP_CHART_A_TERNARY_AXIS, CBCGPChartAxis::ADP_RIGHT, this);
	m_arAxes.SetAtGrow(BCGP_CHART_A_TERNARY_AXIS, pAxis);

	pAxis = new CBCGPChartTernaryAxis(BCGP_CHART_B_TERNARY_AXIS, CBCGPChartAxis::ADP_LEFT, this);
	m_arAxes.SetAtGrow(BCGP_CHART_B_TERNARY_AXIS, pAxis);

	pAxis = new CBCGPChartTernaryAxis(BCGP_CHART_C_TERNARY_AXIS, CBCGPChartAxis::ADP_BOTTOM, this);
	m_arAxes.SetAtGrow(BCGP_CHART_C_TERNARY_AXIS, pAxis);

	m_bIsAutoDestroyDiagram3D = TRUE;
	m_pDiagram3D = new CBCGPChartDiagram3D();
	m_pDiagram3D->SetChart(this);

	m_bEnableResizeAxes = FALSE;
	m_bResizeAxisMode = FALSE;
	m_pResizedAxis = NULL;
	m_pNextResizedAxis = NULL;
	m_bResizeAxisTop = FALSE;

	m_dblResizedAxisTopOffset = 0;
	m_dblResizedAxisBottomOffset = 0;
	m_dblNextResizedAxisTopOffset = 0;
	m_dblNextResizedAxisBottomOffset = 0;

	m_bShowSurfaceMapInLegend = TRUE;

	SetPlotAreaPadding(CBCGPRect (8, 4, 8, 4));
	SetTitleAreaPadding(CBCGPSize (5, 5));

	SetLegendAreaPadding(CBCGPSize (6, 6));
	SetLegendElementSpacing(CBCGPSize (6, 4));
	m_legendAreaFormat.SetContentPadding(CBCGPSize(10, 10));

	SetHitTestDataPointPrecision(CBCGPSize (2, 2));

	m_chartLayout.m_bShowChartTitle = TRUE;
	m_chartLayout.m_legendPosition = BCGPChartLayout::LP_RIGHT;
	m_chartLayout.m_bShowDataTable = FALSE;

	m_titleAreaFormat.m_textFormat.Create(BCGPChartFormatLabel::m_strDefaultFontFamily, 18);

	m_nDefaultVisualSettingIndex = 0;

	m_bEnableZoom = FALSE;
	m_bEnableMagnifier = FALSE;
	m_bEnableSelection = FALSE;
	m_bEnableScroll = FALSE;
	m_bSelectionMode = FALSE;
	m_bEnablePan = FALSE;
	m_bPanMode = FALSE;
	m_bThumbTrackMode = FALSE;
	m_pThumbTrackAxis = NULL;
	m_bThumbSizeMode = FALSE;
	m_pThumbSizeAxis = NULL;
	m_bThumbSizeLeft = FALSE;
	m_dblThumbHitOffset = 0;

	m_bAAEnabled = TRUE;

	m_bClipDiagramToAxes = TRUE;

	m_nLastColorIndex = 0;
	m_dblThemeOpacity = 1.;

	m_curveType = BCGPChartFormatSeries::CCT_LINE;

	m_hitInfoFlags = BCGPChartHitInfo::HIT_NONE;
	m_szHitTestDataPointPrecision = CBCGPSize(4, 4);

	m_bRecalcMinMaxForOptimizedSeries = TRUE;
	m_bSmartLabelsEnabled = TRUE;

	m_bIsThumbnailMode = FALSE;
	m_nThumbnailFlags = 0;

	m_dblDataTableHeaderColumnWidth = 0.0;
	m_dblDataTableRowHeight = 0.0;
	m_dblMaxSeriesKeyWidth = 0.0;
}
//****************************************************************************************
CBCGPChartVisualObject::~CBCGPChartVisualObject()
{
	POSITION posChart = m_lstCharts.Find(this);
	if (posChart != NULL)
	{
		m_lstCharts.RemoveAt(posChart);
	}

	if (!m_lstRelatedLegends.IsEmpty())
	{
		for (POSITION pos = m_lstRelatedLegends.GetHeadPosition(); pos != NULL;)
		{
			CBCGPChartLegendVisualObject* pLegend = DYNAMIC_DOWNCAST(CBCGPChartLegendVisualObject, m_lstRelatedLegends.GetNext(pos));
			if (pLegend != NULL)
			{
				ASSERT_VALID(pLegend);
				pLegend->RemoveRelatedChart(this, FALSE);
			}
		}
		
		m_lstRelatedLegends.RemoveAll();
	}

	CleanUpChartData();

	RemoveAllChartEffects();
	RemoveAllChartObjects();

	for (int i = 0; i < m_arAxes.GetSize(); i++)
	{
		CBCGPChartAxis* pAxis = m_arAxes[i];

		if (pAxis != NULL)
		{
			delete pAxis;
		}
	}

	m_arAxes.RemoveAll();

	DestroyDiagram3D();
}
//****************************************************************************************
void CBCGPChartVisualObject::SetChartType (BCGPChartCategory category, BCGPChartType type, 
								   BOOL bRedraw, BOOL bResetAxesDisplayRange)
{
	ASSERT_VALID (this);
	
	if (GetChartCategory() == category && GetChartType() == type)
	{
		return;
	}

	for (int i = 0; i < m_arData.GetSize(); i++)
	{
		CBCGPChartSeries* pSeries = GetSeries(i, FALSE);
		if (pSeries != NULL && !pSeries->CanBeConvertedToCategory(category))
		{
			return;
		}
	}

	UnZoom(FALSE);

	m_Category = category;
	m_Type = type;

	m_nLastColorIndex = 0;

	if (bResetAxesDisplayRange)
	{
		SetAutoDisplayRange(FALSE);
	}
	
	RemoveAllChartEffects();

	if (m_Category != BCGPChartLongData)
	{
		for (int i = 0; i < m_arData.GetSize(); i++)
		{
			CBCGPChartSeries* pSeries = GetSeries(i, FALSE);
			if (pSeries != NULL)
			{
				pSeries->SetChartType(category, type, FALSE, FALSE);
			}
		}
	}

	UpdateSeriesColorIndexes();

	if (m_Category != BCGPChartStock)
	{
		m_bRecalcMinMaxForOptimizedSeries = FALSE;
		RecalcMinMaxValues();
		m_bRecalcMinMaxForOptimizedSeries = TRUE;
	}

	if (bRedraw)
	{
		SetDirty();
		Redraw();
	}
}
//****************************************************************************************
void CBCGPChartVisualObject::SetChartFillColor(const CBCGPBrush& brColor)
{
	ASSERT_VALID(this);

	m_chartAreaFormat.m_brFillColor = brColor;
	UpdateRelatedLegendColors();
	SetDirty();
}
//****************************************************************************************
void CBCGPChartVisualObject::SetDiagramFillColor(const CBCGPBrush& brColor)
{
	ASSERT_VALID(this);

	m_plotAreaFormat.m_brFillColor = brColor;
	SetDirty();
}
//****************************************************************************************
const CBCGPBrush& CBCGPChartVisualObject::GetChartFillColor() const
{
	ASSERT_VALID(this);
	return m_chartAreaFormat.m_brFillColor;
}
//****************************************************************************************
const CBCGPBrush& CBCGPChartVisualObject::GetDiagramFillColor() const
{
	ASSERT_VALID(this);
	return m_plotAreaFormat.m_brFillColor;
}
//****************************************************************************************
void CBCGPChartVisualObject::SetLegendPosition(BCGPChartLayout::LegendPosition position, 
										 BOOL bLegendOverlapsChart, BOOL bRedraw)
{
	ASSERT_VALID(this);

	m_chartLayout.m_legendPosition = position;
	m_chartLayout.m_bLegendOverlapsChart = bLegendOverlapsChart;
	
	SetDirty(TRUE, bRedraw);
}
//****************************************************************************************
void CBCGPChartVisualObject::EnableDrawLegendShape(BOOL bEnable, BOOL bRedraw)
{
	ASSERT_VALID(this);

	m_chartLayout.m_bDrawLegendShape = bEnable;
	
	SetDirty(TRUE, bRedraw);
}
//****************************************************************************************
void CBCGPChartVisualObject::ShowChartTitle(BOOL bShow, BOOL bTitleOverlapsChart, BOOL bRedraw)
{
	ASSERT_VALID(this);

	m_chartLayout.m_bShowChartTitle = bShow;
	m_chartLayout.m_bTitleOverlapsChart = bTitleOverlapsChart;

	SetDirty(TRUE, bRedraw);
}
//****************************************************************************************
void CBCGPChartVisualObject::SetChartTitle(LPCTSTR lpcszText, BCGPChartFormatLabel* pTitleFormat, BOOL bAdjustLayout)
{
	m_strChartTitle = lpcszText;

	if (pTitleFormat != NULL)
	{
		m_titleAreaFormat = *pTitleFormat;
	}

	if (bAdjustLayout)
	{
		SetDirty();
	}
}
//****************************************************************************************
void CBCGPChartVisualObject::ShowDataTable(BOOL bShow/* = TRUE*/, BCGPChartFormatDataTable*	pDataTableAreaFormat/* = NULL*/)
{
	m_chartLayout.m_bShowDataTable = bShow;

	if (bShow && pDataTableAreaFormat != NULL)
	{
		m_dataTableAreaFormat = *pDataTableAreaFormat;
		m_dataTableAreaFormat.m_bCustomInterlaceColor = !m_dataTableAreaFormat.m_brInterlaceFill.IsEmpty();
	}

	SetDirty();
}
//****************************************************************************************
void CBCGPChartVisualObject::SetTernaryAxasLabelMode(CBCGPChartTernaryAxis::LabelMode lm, BOOL bRedraw)
{
	for (int i = BCGP_CHART_A_TERNARY_AXIS; i <= BCGP_CHART_C_TERNARY_AXIS; i++)
	{
		CBCGPChartTernaryAxis* pAxis = DYNAMIC_DOWNCAST(CBCGPChartTernaryAxis, GetChartAxis(i));

		if (pAxis != NULL)
		{
			ASSERT_VALID(pAxis);
			pAxis->SetLabelMode(lm);
		}
	}

	SetDirty(TRUE, bRedraw);
}
//****************************************************************************************
int CBCGPChartVisualObject::AddAxis(CBCGPChartAxis* pAxis)
{
	ASSERT_VALID(this);
	ASSERT_KINDOF(CBCGPChartAxis, pAxis);

	if (pAxis == NULL)
	{
		return -1;
	}

	if (pAxis->m_nAxisID == -1)
	{
		pAxis->m_nAxisID = GetNextCustomAxisID();
	}

	if (m_bIsThumbnailMode)
	{
		pAxis->SetThumbnailMode();
	}

	m_arAxes.SetAtGrow(pAxis->m_nAxisID, pAxis);

	return pAxis->m_nAxisID;
}
//****************************************************************************************
void CBCGPChartVisualObject::RemoveCustomAxis(CBCGPChartAxis* pAxis, BOOL bDeleteRelatedSeries)
{
	if (pAxis->m_nAxisID < BCGP_CHART_FIRST_CUSTOM_ID)
	{
		return;
	}

	pAxis->RemoveCustomAxis();

	if (m_arAxes.GetSize() > pAxis->m_nAxisID)
	{
		m_arAxes.SetAt(pAxis->m_nAxisID, NULL);
	}

	int i = 0;
	for (i = 0; i < GetSeriesCount(TRUE); i++)
	{
		CBCGPChartSeries* pSeries = GetSeries(i, FALSE);

		if (pSeries == NULL)
		{
			continue;
		}

		if (pSeries->GetRelatedAxisIndex(pAxis) != CBCGPChartSeries::AI_UNKNOWN)
		{
			if (bDeleteRelatedSeries)
			{
				CleanUpChartData(i, FALSE);
			}
			else
			{
				pSeries->ReplaceAxis(pAxis, NULL);
			}
		}
	}

	POSITION pos = NULL;
	for (pos = m_lstChartObjects.GetHeadPosition(); pos != NULL;)
	{
		POSITION posCurr = pos;
		CBCGPChartObject* pObj = m_lstChartObjects.GetNext(pos);

		if (pObj == NULL)
		{
			continue;
		}

		if (pObj->IsObjectShownOnAxis(pAxis))
		{
			m_lstChartObjects.RemoveAt(posCurr);
			delete pObj;
		}
	}

	for (pos = m_lstChartEffects.GetHeadPosition(); pos != NULL;)
	{
		POSITION posCurr = pos;
		CBCGPChartBaseEffect* pEffect = m_lstChartEffects.GetNext(pos);
		
		if (pEffect == NULL)
		{
			continue;
		}
		
		if (pEffect->IsEffectShownOnAxis(pAxis))
		{
			m_lstChartEffects.RemoveAt(posCurr);
			delete pEffect;
		}
	}

	delete pAxis;
}
//****************************************************************************************
CBCGPChartAxis* CBCGPChartVisualObject::ReplaceDefaultAxis(int nAxisID, CRuntimeClass* pCustomAxisRTC)
{
	ASSERT_VALID(this);
	ASSERT(pCustomAxisRTC != NULL);

	if (nAxisID >= BCGP_CHART_FIRST_CUSTOM_ID || nAxisID < 0)
	{
		TRACE0("CBCGPChartVisualObject::ReplaceDefaultAxis: Invalid default axis ID specified.");
		ASSERT(FALSE);
		return NULL;
	}

	CBCGPChartAxis* pAxis = DYNAMIC_DOWNCAST(CBCGPChartAxis, pCustomAxisRTC->CreateObject());

	if (pAxis == NULL)
	{
		TRACE0("CBCGPChartVisualObject::ReplaceDefaultAxis: Invalid runtime class for default axis is specified.");
		ASSERT(FALSE);
		return NULL;
	}

	CBCGPChartAxis* pDefaultAxis = GetChartAxis(nAxisID);

	if (pDefaultAxis == NULL)
	{
		TRACE0("CBCGPChartVisualObject::ReplaceDefaultAxis: Default axis with specified ID is not found.");
		ASSERT(FALSE);
		return NULL;
	}

	ASSERT_VALID(pDefaultAxis);

	pAxis->m_pChart = this;
	pAxis->SetAxisDefaultPosition(pDefaultAxis->m_axisDefaultPosition);
	pAxis->m_nAxisID = nAxisID;

	pAxis->CommonInit();

	m_arAxes.SetAt(nAxisID, pAxis);

	for (int i = 0; i < m_arData.GetSize(); i++)
	{
		CBCGPChartSeries* pSeries = GetSeries(i, FALSE);

		if (pSeries == NULL)
		{
			continue;
		}

		pSeries->ReplaceAxis(pDefaultAxis, pAxis);
	}

	delete pDefaultAxis;

	return pAxis;
}
//****************************************************************************************
int CBCGPChartVisualObject::GetNextCustomAxisID()
{
	int nID = 0;
	for (int i = 0; i < m_arAxes.GetSize(); i++)
	{
		CBCGPChartAxis* pAxis = m_arAxes[i];
		if (pAxis == NULL)
		{
			if (i >= BCGP_CHART_FIRST_CUSTOM_ID)
			{
				return i;
			}
			continue;
		}

		ASSERT_VALID(pAxis);
		nID = max(nID, pAxis->m_nAxisID);
	}

	if (nID < BCGP_CHART_FIRST_CUSTOM_ID)
	{
		return BCGP_CHART_FIRST_CUSTOM_ID;
	}

	return nID + 1;
}
//****************************************************************************************
void CBCGPChartVisualObject::ShowAxis(int nAxisID, BOOL bShow, BOOL bForceShow)
{
	ASSERT_VALID(this);
	CBCGPChartAxis* pAxis = GetChartAxis(nAxisID);

	if (pAxis == NULL)
	{
		return;
	}

	ASSERT_VALID(pAxis);

	pAxis->m_bVisible = bShow;
	pAxis->m_bAlwaysVisible = bForceShow;

	SetDirty();
}
//****************************************************************************************
void CBCGPChartVisualObject::ShowAxisIntervalInterlacing(int nAxisID, BOOL bShow, int nStep, int nFirstInterval)
{
	ASSERT_VALID(this);
	CBCGPChartAxis* pAxis = GetChartAxis(nAxisID);

	if (pAxis == NULL)
	{
		return;
	}

	ASSERT_VALID(pAxis);

	pAxis->EnableMajorUnitIntervalInterlacing(bShow);
	pAxis->SetInterlaceStep(nStep);
	pAxis->SetFirstInterlacedIntervalIndex(nFirstInterval);

	SetDirty();
}
//****************************************************************************************
void CBCGPChartVisualObject::SetAxisIntervalInterlaceColor(int nAxisID, const CBCGPBrush& brush)
{
	ASSERT_VALID(this);
	CBCGPChartAxis* pAxis = GetChartAxis(nAxisID);

	if (pAxis == NULL)
	{
		return;
	}

	ASSERT_VALID(pAxis);

	pAxis->m_brInterval = brush;

	SetDirty();
}
//****************************************************************************************
void CBCGPChartVisualObject::ShowAxisName(int nAxisID, BOOL bShow)
{
	ASSERT_VALID(this);
	CBCGPChartAxis* pAxis = GetChartAxis(nAxisID);

	if (pAxis == NULL)
	{
		return;
	}

	ASSERT_VALID(pAxis);

	pAxis->m_bDisplayAxisName = bShow;
	SetDirty();
}
//****************************************************************************************
void CBCGPChartVisualObject::ShowScrollBar(int nAxisID, BOOL bShow)
{
	ASSERT_VALID(this);
	CBCGPChartAxis* pAxis = GetChartAxis(nAxisID);

	if (pAxis == NULL)
	{
		return;
	}

	ASSERT_VALID(pAxis);

	pAxis->ShowScrollBar(bShow, TRUE);
}
//****************************************************************************************
void CBCGPChartVisualObject::ShowScrollBars(BOOL bShow, BOOL bRedraw)
{
	for (int i = 0; i < m_arAxes.GetSize(); i++)
	{
		CBCGPChartAxis* pAxis = m_arAxes[i];

		if (pAxis != NULL)
		{
			ASSERT_VALID(pAxis);
			pAxis->ShowScrollBar(bShow, FALSE);
		}
	}

	SetDirty(TRUE, bRedraw);
}
//****************************************************************************************
void CBCGPChartVisualObject::SetAxisName(int nAxisID, const CString& strName)
{
	ASSERT_VALID(this);
	CBCGPChartAxis* pAxis = GetChartAxis(nAxisID);

	if (pAxis == NULL)
	{
		return;
	}

	ASSERT_VALID(pAxis);
	
	pAxis->m_strAxisName = strName;
	SetDirty();
}
//****************************************************************************************
void CBCGPChartVisualObject::ShowAxisGridLines(int nAxisID, BOOL bShowMajorGridLines, BOOL bShowMinorGridLines)
{
	ASSERT_VALID(this);
	CBCGPChartAxis* pAxis = GetChartAxis(nAxisID);

	if (pAxis == NULL)
	{
		return;
	}

	ASSERT_VALID(pAxis);

	// both  methods call SetDirty in parent
	pAxis->ShowMajorGridLines(bShowMajorGridLines);
	pAxis->ShowMinorGridLines(bShowMinorGridLines);
}
//****************************************************************************************
CBCGPChartAxis*	CBCGPChartVisualObject::GetChartAxis(int nAxisID) const
{
	if (nAxisID < 0 || nAxisID >= m_arAxes.GetSize())
	{
		return NULL;
	}

	return m_arAxes.GetAt(nAxisID);
}
//****************************************************************************************
CBCGPChartAxis*	CBCGPChartVisualObject::GetPrimaryAxis(BOOL bHorizontal) const
{
	CBCGPChartAxis* pAxis = m_arAxes.GetAt(BCGP_CHART_X_PRIMARY_AXIS);

	if (bHorizontal && pAxis->IsVertical() || !bHorizontal && !pAxis->IsVertical())
	{
		return m_arAxes.GetAt(BCGP_CHART_Y_PRIMARY_AXIS);
	}

	return pAxis;
}
//****************************************************************************************
CBCGPChartAxis*	CBCGPChartVisualObject::GetSecondaryAxis(BOOL bHorizontal) const
{
	CBCGPChartAxis* pAxis = m_arAxes.GetAt(BCGP_CHART_X_SECONDARY_AXIS);

	if (bHorizontal && pAxis->IsVertical() || !bHorizontal && !pAxis->IsVertical())
	{
		return m_arAxes.GetAt(BCGP_CHART_Y_SECONDARY_AXIS);
	}

	return pAxis;

}
//****************************************************************************************
void CBCGPChartVisualObject::SwapAxesDirections(BOOL bAdjustGradientAngles)
{
	if (IsChart3D())
	{
		CBCGPChartAxis* pAxis = GetChartAxis(BCGP_CHART_Y_PRIMARY_AXIS);

		if (pAxis != NULL)
		{
			ASSERT_VALID(pAxis);
			pAxis->SwapDirection(bAdjustGradientAngles);
		}

		pAxis = GetChartAxis(BCGP_CHART_X_PRIMARY_AXIS);

		if (pAxis != NULL)
		{
			ASSERT_VALID(pAxis);
			pAxis->SwapDirection(bAdjustGradientAngles);
		}

		pAxis = GetChartAxis(BCGP_CHART_Z_PRIMARY_AXIS);

		if (pAxis != NULL)
		{
			ASSERT_VALID(pAxis);
			pAxis->SwapDirection(bAdjustGradientAngles);
		}
	}
	else
	{
		for (int i = 0; i < m_arAxes.GetSize(); i++)
		{
			CBCGPChartAxis* pAxis = m_arAxes[i];
			if (pAxis == NULL)
			{
				continue;
			}

			ASSERT_VALID(pAxis);
			pAxis->SwapDirection(bAdjustGradientAngles);
		}
	}

	if (bAdjustGradientAngles)
	{
		for (int i = 0; i < m_arData.GetSize(); i++)
		{
			CBCGPChartSeries* pSeries = GetSeries(i, FALSE);
			
			if (pSeries != NULL)
			{
				pSeries->AdjustGradientAngels();
			}
		}
	}
}
//****************************************************************************************
void CBCGPChartVisualObject::ResetAxes()
{
	for (int i = 0; i < m_arAxes.GetSize(); i++)
	{
		CBCGPChartAxis* pAxis = m_arAxes[i];
		if (pAxis == NULL)
		{
			continue;
		}

		ASSERT_VALID(pAxis);
		pAxis->Reset();
	}
}
//****************************************************************************************
void CBCGPChartVisualObject::ScrollAxis(float fUnitsToScroll, int nAxisID, BOOL bRedraw)
{
	ASSERT_VALID(this);

	if (!m_bEnableZoom)
	{
		return;
	}

	CBCGPChartAxis* pAxis = m_arAxes[nAxisID];

	if (pAxis == NULL)
	{
		return;
	}

	ASSERT_VALID(pAxis);

	pAxis->Scroll(fUnitsToScroll, FALSE);

	SetDirty();

	if (bRedraw)
	{
		RecalcMinMaxValues();
		Redraw();
	}
}
//****************************************************************************************
void CBCGPChartVisualObject::SetAutoDisplayRange(BOOL bRedraw, BOOL bByZoom)
{
	for (int i = 0; i < m_arAxes.GetSize(); i++)
	{
		CBCGPChartAxis* pAxis = m_arAxes[i];

		if (pAxis != NULL)
		{
			ASSERT_VALID(pAxis);
			if (bByZoom && pAxis->IsZoomEnabled() && pAxis->IsZoomed() || !bByZoom)
			{
				pAxis->SetAutoDisplayRange();
			}
		}
	}

	SetDirty();

	if (bRedraw)
	{
		m_bRecalcMinMaxForOptimizedSeries = FALSE;
		RecalcMinMaxValues();
		m_bRecalcMinMaxForOptimizedSeries = TRUE;
		Redraw();
	}
}
//****************************************************************************************
void CBCGPChartVisualObject::NormalizeDisplayRange()
{
	for (int i = 0; i < m_arAxes.GetSize(); i++)
	{
		CBCGPChartAxis* pAxis = m_arAxes[i];

		if (pAxis != NULL)
		{
			ASSERT_VALID(pAxis);
			pAxis->NormalizeDisplayRange();
		}
	}
}
//****************************************************************************************
void CBCGPChartVisualObject::SetFixedDisplayRange(CBCGPChartData& dataStart, CBCGPChartData& dataEnd, BOOL bAxesFromData, 
													BOOL bRedraw, BOOL bByZoom)
{
	if (bByZoom)
	{
		for (int i = 0; i < dataStart.GetComponentCount(); i++)
		{
			CBCGPChartValue valStart = dataStart.GetValue((CBCGPChartData::ComponentIndex) i);
			CBCGPChartValue valEnd = dataEnd.GetValue((CBCGPChartData::ComponentIndex) i);

			if (valStart.IsEmpty() || valEnd.IsEmpty())
			{
				continue;
			}

			double dblStart = valStart.GetValue();
			double dblEnd = valEnd.GetValue();

			if (dblStart > dblEnd)
			{
				dataStart.SetValue(CBCGPChartValue(dblEnd, valEnd.GetAxis()), (CBCGPChartData::ComponentIndex) i);
				dataEnd.SetValue(CBCGPChartValue(dblStart, valStart.GetAxis()), (CBCGPChartData::ComponentIndex) i);
			}

			// re-get value
			dblStart = dataStart.GetValue((CBCGPChartData::ComponentIndex) i);
			dblEnd = dataEnd.GetValue((CBCGPChartData::ComponentIndex) i);

			CBCGPChartAxis* pAxisStart = valStart.GetAxis();
			CBCGPChartAxis* pAxisEnd = valEnd.GetAxis();

			if (pAxisStart != NULL && pAxisStart == pAxisEnd)
			{
				// set empty values to prevent zoom
				if (dblEnd < pAxisStart->GetMinScrollValue() || 
					dblStart > pAxisStart->GetMaxScrollValue())
				{
					dataStart.SetValue(CBCGPChartValue(), (CBCGPChartData::ComponentIndex) i);
					dataEnd.SetValue(CBCGPChartValue(), (CBCGPChartData::ComponentIndex) i);
				}
			}
		}
	}

	SetFixedDisplayBound(dataStart, TRUE, bAxesFromData, bByZoom);
	SetFixedDisplayBound(dataEnd, FALSE, bAxesFromData, bByZoom);
	NormalizeDisplayRange();

	if (bRedraw)
	{
		m_bRecalcMinMaxForOptimizedSeries = FALSE;
		RecalcMinMaxValues();
		m_bRecalcMinMaxForOptimizedSeries = TRUE;
		Redraw();
	}
}
//****************************************************************************************
void CBCGPChartVisualObject::SetFixedDisplayBound(const CBCGPChartData& data, BOOL bIsMinimum, BOOL bAxesFromData, 
												  BOOL bByZoom)
{
	if (data.IsEmpty())
	{
		return;
	}

	for (int i = 0; i < data.GetComponentCount(); i++)
	{
		CBCGPChartValue val = data.GetValue((CBCGPChartData::ComponentIndex)i);
		if (val.IsEmpty())
		{
			continue;
		}

		CBCGPChartAxis* pAxis = val.GetAxis();
		if (pAxis == NULL && !bAxesFromData)
		{
			switch(i)
			{
			case CBCGPChartData::CI_Y:
				pAxis = GetChartAxis(BCGP_CHART_Y_PRIMARY_AXIS);
				break;
			case CBCGPChartData::CI_X:
				pAxis = GetChartAxis(BCGP_CHART_X_PRIMARY_AXIS);
				break;
			case CBCGPChartData::CI_Z:
				pAxis = GetChartAxis(BCGP_CHART_Z_PRIMARY_AXIS);
				break;
			}
		}

		if (pAxis == NULL || !pAxis->IsZoomEnabled() && bByZoom)
		{
			continue;
		}

		ASSERT_VALID(pAxis);
		
		bIsMinimum ? pAxis->SetFixedMinimumDisplayValue(val, bByZoom) : pAxis->SetFixedMaximumDisplayValue(val, bByZoom);
	}

	SetDirty();
}
//****************************************************************************************
void CBCGPChartVisualObject::RecalcMinMaxValues()
{
	for (int i = 0; i < m_arData.GetSize(); i++)
	{
		CBCGPChartSeries* pSeries = GetSeries(i, FALSE);

		if (pSeries != NULL && (!pSeries->IsOptimizedLongDataMode() || 
			pSeries->IsOptimizedLongDataMode() && m_bRecalcMinMaxForOptimizedSeries))
		{
			pSeries->RecalcMinMaxValues();
		}
	}
}
//****************************************************************************************
BOOL CBCGPChartVisualObject::IsChart3D() const
{
	return IsCategory3D(m_Category);
}
//****************************************************************************************
BOOL CBCGPChartVisualObject::IsCategory3D(BCGPChartCategory category)
{
	return (category == BCGPChartColumn3D || category == BCGPChartBar3D || category == BCGPChartLine3D || 
			category == BCGPChartArea3D || category == BCGPChartSurface3D);
}
//****************************************************************************************
void CBCGPChartVisualObject::SetDiagram3D(CBCGPChartDiagram3D* pDiagram3D, BOOL bAutoDestroy) 
{
	if (pDiagram3D == NULL)
	{
		return;
	}

	ASSERT_VALID(this);
	ASSERT(pDiagram3D != NULL);
	
	DestroyDiagram3D();

	m_bIsAutoDestroyDiagram3D = bAutoDestroy;
	m_pDiagram3D = pDiagram3D;
	m_pDiagram3D->SetChart(this);
}
//****************************************************************************************
void CBCGPChartVisualObject::DestroyDiagram3D()
{
	if (m_bIsAutoDestroyDiagram3D && m_pDiagram3D != NULL)
	{
		delete m_pDiagram3D;
		m_pDiagram3D = NULL;
	}	
}
//****************************************************************************************
void CBCGPChartVisualObject::SetBaseDepthPercent3D(double dblUnitSize)
{
	CBCGPChartDiagram3D* pDiagram3D = GetDiagram3D();

	if (pDiagram3D == NULL)
	{
		return;
	}

	if (!IsChart3DGrouped())
	{
		pDiagram3D->SetBaseDepthPercent(dblUnitSize * GetVisibleSeriesCount());
	}
	else
	{
		pDiagram3D->SetBaseDepthPercent(dblUnitSize);
	}
}
//****************************************************************************************
BOOL CBCGPChartVisualObject::IsChart3DGrouped() const
{
	if (!IsChart3D())
	{
		return TRUE;
	}

	CBCGPChartDiagram3D* pDiagram3D = GetDiagram3D();

	if (GetDiagram3D() != NULL)
	{
		CBCGPChartDiagram3D::ExplicitGrouping grouping = pDiagram3D->GetExplicitGrouping();

		if (grouping == CBCGPChartDiagram3D::EG_GROUPED)
		{
			return TRUE;
		}
		else if (grouping == CBCGPChartDiagram3D::EG_NOT_GROUPED)
		{
			return FALSE;
		}
	}

	for (int i = 0; i < (int)m_arData.GetSize(); i++)
	{
		CBCGPChartSeries* pSeries = (CBCGPChartSeries*) m_arData[i];

		if (pSeries == NULL || !pSeries->m_bVisible)
		{
			continue;
		}

		if (pSeries->GetChartCategory() == BCGPChartSurface3D)
		{
			return FALSE;
		}

		if ((pSeries->GetChartCategory() == BCGPChartArea3D ||
			 pSeries->GetChartCategory() == BCGPChartLine3D) && 
			!pSeries->IsStakedSeries())
		{
			return FALSE;
		}

		if (pSeries->IsStakedSeries())
		{
			return TRUE;
		}
	}
	
	if (pDiagram3D == NULL)
	{
		return TRUE;
	}

	return pDiagram3D->IsGrouped();
}
//****************************************************************************************
BOOL CBCGPChartVisualObject::IsDataTableSupported()
{
	if (m_bIsThumbnailMode)
	{
		return FALSE;
	}

	if (!NeedDisplayAxes())
	{
		return FALSE;
	}

	if (IsChart3D())
	{
		return FALSE;
	}

	switch (GetChartCategory())
	{
	case BCGPChartTernary:
	case BCGPChartPolar:
	case BCGPChartBubble:
		return FALSE;
	}

	CBCGPChartAxis* pAxis = m_arAxes.GetAt(BCGP_CHART_X_PRIMARY_AXIS);
	if (pAxis == NULL)
	{
		return FALSE;
	}

	if (!pAxis->m_bVisible || pAxis->m_bIsVertical)
	{
		return FALSE;
	}
	
	CBCGPChartAxisX* pAxisX = DYNAMIC_DOWNCAST(CBCGPChartAxisX, pAxis);
	if (pAxisX == NULL)
	{
		return FALSE;
	}

	pAxisX->CheckAxisComponent();

	return !pAxisX->IsComponentXSet();
}
//****************************************************************************************
void CBCGPChartVisualObject::AdjustLayout(CBCGPGraphicsManager* pGM)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pGM);

	if (pGM == NULL)
	{
		return;
	}

	CBCGPRect rectChart = m_rect;
	rectChart.DeflateRect(GetChartAreaFormat().GetContentPadding(TRUE));

	CBCGPRect rectLegend;
	BOOL bCustomLegendRect = OnGetLegendAreaRect(pGM, rectLegend);
	CBCGPSize szLegend = rectLegend.Size();

	if (!bCustomLegendRect)
	{
		szLegend = CalcLegendSize(pGM);
	}
	else
	{
		m_rectLegendArea = rectLegend;
	}

	CBCGPRect rectTitleArea;
	BOOL bCustomTitleRect = OnGetTitleAreaRect(rectTitleArea);
	CBCGPSize szTitle = m_rectTitleArea.Size();
	CBCGPSize szTitleAreaPadding = GetTitleAreaPadding(TRUE);

	if (!bCustomTitleRect)
	{
		szTitle = CalcTitleSize(pGM);
		rectTitleArea.SetRect(rectChart.left + rectChart.Width() / 2 - szTitle.cx / 2, 
			rectChart.top + szTitleAreaPadding.cx, 
			rectChart.left + rectChart.Width() / 2 + szTitle.cx / 2,
			rectChart.top + szTitleAreaPadding.cx + szTitle.cy);
		m_rectTitleArea = rectTitleArea;
	}
	else
	{
		m_rectTitleArea = rectTitleArea;
	}

	BOOL bHasDataTable = FALSE;
	double dblDataTableHeight = 0.0;
	
	m_rectDataTableArea.SetRectEmpty();
	m_dblDataTableHeaderColumnWidth = 0.0;
	m_dblDataTableRowHeight = 0.0;
	m_dblMaxSeriesKeyWidth = 0.0;

	if (m_chartLayout.m_bShowDataTable && IsDataTableSupported())
	{
		if (CalcDataTableSize(pGM, m_dblDataTableHeaderColumnWidth, dblDataTableHeight, m_dblDataTableRowHeight, m_dblMaxSeriesKeyWidth))
		{
			m_rectDataTableArea.SetRect(rectChart.left, 
				rectChart.bottom - dblDataTableHeight,
				rectChart.right,
				rectChart.bottom);

			CBCGPChartAxis* pAxis = m_arAxes.GetAt(BCGP_CHART_X_PRIMARY_AXIS);
			if (pAxis != NULL)
			{
				pAxis->m_bConnectedToDataTable = TRUE;
			}

			bHasDataTable = TRUE;
		}
	}
	
	CBCGPRect rectPlotArea = rectChart;
	BOOL bCustomPlotAreaRect = OnGetPlotAreaRect(rectPlotArea);

	int i = 0;
	for (i = 0; i < GetSeriesCount(); i++)
	{
		CBCGPChartSeries* pSeries = GetSeries(i, FALSE);

		if (pSeries != NULL && pSeries->m_bVisible && !pSeries->IsFullStackedMinMaxSet())
		{
			pSeries->SetFullStackedMinMax();
		}
	}

	BOOL bIsAxisWithFixedInterval = FALSE;

	for (i = 0; i < m_arAxes.GetSize(); i++)
	{
		CBCGPChartAxis* pAxis = m_arAxes[i];
		if (pAxis == NULL)
		{
			continue;
		}
		
		ASSERT_VALID(pAxis);
		if (pAxis->IsFixedIntervalWidth())
		{
			bIsAxisWithFixedInterval = TRUE;
		}
	}

	if (!bIsAxisWithFixedInterval)
	{
		for (i = 0; i < m_arAxes.GetSize(); i++)
		{
			CBCGPChartAxis* pAxis = m_arAxes[i];
			if (pAxis == NULL)
			{
				continue;
			}
			
			ASSERT_VALID(pAxis);
			pAxis->CalcMinMaxValues();
		}
	}

	CBCGPSize szLegendAreaPadding = GetLegendAreaPadding(TRUE);

	if (!bCustomPlotAreaRect)
	{
		CBCGPRect rectPlotAreaPadding = GetPlotAreaPadding(TRUE);

		m_rectPlotArea = rectChart;

		if (m_chartLayout.m_bShowChartTitle && !bCustomTitleRect && !m_chartLayout.m_bTitleOverlapsChart && !m_bIsThumbnailMode)
		{
			m_rectPlotArea.top = rectTitleArea.bottom + szTitleAreaPadding.cy;
		}
		
		m_rectPlotArea.top += rectPlotAreaPadding.top;
		
		if (!m_chartLayout.m_bLegendOverlapsChart && 
			m_chartLayout.m_legendPosition != BCGPChartLayout::LP_NONE && GetVisibleSeriesCount() > 0 && 
			!bCustomLegendRect)
		{
			switch (m_chartLayout.m_legendPosition)
			{
			case BCGPChartLayout::LP_TOP:
				m_rectPlotArea.top += szLegend.cy + szLegendAreaPadding.cy;
				m_rectPlotArea.DeflateRect(rectPlotAreaPadding.left, 0, 
					rectPlotAreaPadding.right, rectPlotAreaPadding.bottom);
				break;
			case BCGPChartLayout::LP_BOTTOM:
				m_rectPlotArea.bottom -= szLegend.cy + szLegendAreaPadding.cy * 2;
				m_rectPlotArea.DeflateRect(rectPlotAreaPadding.left, 0, 
					rectPlotAreaPadding.right, 0);
				break;
			case BCGPChartLayout::LP_LEFT:
				m_rectPlotArea.left += szLegend.cx + szLegendAreaPadding.cx * 2;
				m_rectPlotArea.DeflateRect(0, 0, 
					rectPlotAreaPadding.right, rectPlotAreaPadding.bottom);
				break;
			case BCGPChartLayout::LP_RIGHT:
				m_rectPlotArea.right -= szLegend.cx + szLegendAreaPadding.cx * 2;
				m_rectPlotArea.DeflateRect(rectPlotAreaPadding.left, 0, 
					0, rectPlotAreaPadding.bottom);
				break;
			case BCGPChartLayout::LP_TOPRIGHT:
				m_rectPlotArea.right -= szLegend.cx + szLegendAreaPadding.cx * 2;
				
				m_rectPlotArea.DeflateRect(rectPlotAreaPadding.left, 0, 
					0, rectPlotAreaPadding.bottom);
				break;
			}
		}
		else
		{
			m_rectPlotArea.DeflateRect(rectPlotAreaPadding.left, 0, 
				rectPlotAreaPadding.right, rectPlotAreaPadding.bottom);
		}

		rectPlotArea = m_rectPlotArea;
	}
	else
	{
		m_rectPlotArea = rectPlotArea;
	}

	if (!bCustomLegendRect)
	{
		if (m_bIsThumbnailMode)
		{
			m_rectLegendArea.SetRectEmpty();
		}
		else
		{
			switch (m_chartLayout.m_legendPosition)
			{
			case BCGPChartLayout::LP_TOP:
				if (m_chartLayout.m_bShowChartTitle && !bCustomTitleRect)
				{
					m_rectLegendArea = CBCGPRect(CBCGPPoint (rectChart.left + rectChart.Width () / 2 - szLegend.cx / 2, 
						rectTitleArea.bottom + szLegendAreaPadding.cy), szLegend);
				}
				else
				{
					m_rectLegendArea = CBCGPRect(CBCGPPoint (rectChart.left + rectChart.Width () / 2 - szLegend.cx / 2, 
						rectChart.top + szLegendAreaPadding.cy), szLegend);
				}
				break;
			case BCGPChartLayout::LP_BOTTOM:
				m_rectLegendArea = CBCGPRect(CBCGPPoint (rectChart.left + rectChart.Width () / 2 - szLegend.cx / 2, 
					rectChart.bottom - szLegendAreaPadding.cy - szLegend.cy), szLegend);
				break;
			case BCGPChartLayout::LP_LEFT:
				m_rectLegendArea = CBCGPRect(CBCGPPoint (rectChart.left + szLegendAreaPadding.cx, 
					rectPlotArea.top + rectPlotArea.Height() / 2 - szLegend.cy / 2), szLegend);
				break;
			case BCGPChartLayout::LP_RIGHT:
				m_rectLegendArea = CBCGPRect(CBCGPPoint (rectChart.right - szLegendAreaPadding.cx - szLegend.cx, 
					rectPlotArea.top + rectPlotArea.Height() / 2 - szLegend.cy / 2), szLegend);
				break;
			case BCGPChartLayout::LP_TOPRIGHT:
				m_rectLegendArea = CBCGPRect(CBCGPPoint (rectChart.right - szLegendAreaPadding.cx - szLegend.cx, 
					rectPlotArea.top), szLegend);
				break;
			}
		}
	}

	if (IsChart3D())
	{
		GetChartAxis(BCGP_CHART_X_PRIMARY_AXIS)->CalcNameRect(pGM, rectPlotArea, rectChart, FALSE);
		GetChartAxis(BCGP_CHART_Y_PRIMARY_AXIS)->CalcNameRect(pGM, rectPlotArea, rectChart, FALSE);
		GetChartAxis(BCGP_CHART_Z_PRIMARY_AXIS)->CalcNameRect(pGM, rectPlotArea, rectChart, FALSE);

		m_rectDiagramArea = rectPlotArea;

		CBCGPChartDiagram3D* pDiagram3D = GetDiagram3D();

		if (pDiagram3D == NULL)
		{
			return;
		}

		pDiagram3D->AdjustLayout(pGM, m_rectDiagramArea);
		OnAfterRecalcLayout(pGM);
		return;
	}

	if (GetChartCategory() == BCGPChartTernary)
	{
		int i;
		BOOL bAxisWasInit = FALSE;

		for (i = BCGP_CHART_A_TERNARY_AXIS; i <= BCGP_CHART_C_TERNARY_AXIS; i++)
		{
			CBCGPChartAxis* pAxis = m_arAxes[i];

			if (pAxis == NULL)
			{
				TRACE0("Missing standard ternary axis. AdjustLayout aborted.");
				return;
			}

			ASSERT_VALID(pAxis);
			pAxis->CalcNameRect(pGM, rectPlotArea, rectChart, FALSE);
			pAxis->CalcMaxLabelSize(pGM);

			bAxisWasInit = pAxis->m_bInitialized;
		}

		for (i = BCGP_CHART_A_TERNARY_AXIS; i <= BCGP_CHART_C_TERNARY_AXIS; i++)
		{
			CBCGPChartAxis* pAxis = m_arAxes[i];
			pAxis->CalcAxisPos(rectPlotArea);
			pAxis->CalcMajorMinorUnits(pGM);
		}

		if (!bAxisWasInit)
		{
			int i;
			for (i = BCGP_CHART_A_TERNARY_AXIS; i <= BCGP_CHART_C_TERNARY_AXIS; i++)
			{
				m_arAxes[i]->CalcMaxLabelSize(pGM);
			}

			for (i = BCGP_CHART_A_TERNARY_AXIS; i <= BCGP_CHART_C_TERNARY_AXIS; i++)
			{
				m_arAxes[i]->CalcAxisPos(rectPlotArea);
			}
		}

		for (i = BCGP_CHART_A_TERNARY_AXIS; i <= BCGP_CHART_C_TERNARY_AXIS; i++)
		{
			m_arAxes[i]->CalcNameRect(pGM, rectPlotArea, rectChart, TRUE);
		}

		m_rectDiagramArea = rectPlotArea;

		OnAfterRecalcLayout(pGM);
		return;
	}

	CBCGPRect rectPlotSaved = rectPlotArea;

	for (i = 0; i < m_arAxes.GetSize(); i++)
	{
		CBCGPChartAxis* pAxis = m_arAxes[i];

		if (pAxis == NULL)
		{
			continue;
		}

		ASSERT_VALID(pAxis);

		pAxis->UpdateAxisPos(rectPlotArea);
		if (!bIsAxisWithFixedInterval)
		{
			pAxis->CalcMajorMinorUnits(pGM);
		}
	}
	
	if (bIsAxisWithFixedInterval)
	{
		for (int k = 0; k < 2; k++)
		{
			for (i = 0; i < m_arAxes.GetSize(); i++)
			{
				CBCGPChartAxis* pAxis = m_arAxes[i];
				if (pAxis == NULL)
				{
					continue;
				}
				
				ASSERT_VALID(pAxis);
				if (pAxis->IsFixedIntervalWidth() && k == 0 || 
					!pAxis->IsFixedIntervalWidth() && k == 1)
				{
					pAxis->CalcMinMaxValues();
					pAxis->CalcMajorMinorUnits(pGM);
				}
			}
		}
	}

	if (!NeedDisplayAxes())
	{
		m_rectDiagramArea = rectPlotArea;
		OnAfterRecalcLayout(pGM);
		return;
	}

	for (i = 0; i < m_arAxes.GetSize(); i++)
	{
		CBCGPChartAxis* pAxis = m_arAxes[i];

		if (pAxis == NULL)
		{
			continue;
		}

		ASSERT_VALID(pAxis);

		pAxis->CalcNameRect(pGM, rectPlotArea, rectPlotSaved, FALSE);
		pAxis->CalcMaxLabelSize(pGM);
	}

	for (i = 0; i < m_arAxes.GetSize(); i++)
	{
		CBCGPChartAxis* pAxis = m_arAxes[i];
		
		if (pAxis == NULL)
		{
			continue;
		}
		
		ASSERT_VALID(pAxis);
		
		if (pAxis->IsScaleBreakEnabled())
		{
			pAxis->CalcMajorMinorUnitsForScaleParts(pGM);
			pAxis->CalcMaxLabelSizeForScaleParts(pGM);
			pAxis->CalcMajorMinorUnitsForScaleParts(pGM);
		}
	}


	m_rectDiagramArea = rectPlotArea;

	if (bHasDataTable)
	{
		m_rectDiagramArea.bottom -= dblDataTableHeight;
		m_rectDiagramArea.left += m_dblDataTableHeaderColumnWidth;

		CBCGPChartAxis* pAxis = m_arAxes.GetAt(BCGP_CHART_X_PRIMARY_AXIS);
		if (pAxis != NULL)
		{
			if (pAxis->m_szMaxLabelSize.IsEmpty())
			{
				pAxis->CalcMaxLabelSize(pGM);
			}

			m_rectDiagramArea.bottom -= pAxis->m_szMaxLabelSize.cy + 2.0 * pAxis->m_dblLabelDistance + m_rectPlotAreaPadding.bottom;
		}
	}
	else
	{
		CBCGPChartAxis* pAxis = m_arAxes.GetAt(BCGP_CHART_X_PRIMARY_AXIS);
		if (pAxis != NULL)
		{
			pAxis->m_bConnectedToDataTable = FALSE;
		}
	}

	for (i = 0; i < m_arAxes.GetSize(); i++)
	{
		CBCGPChartAxis* pAxis = m_arAxes[i];

		if (pAxis == NULL)
		{
			continue;
		}

		ASSERT_VALID(pAxis);

		pAxis->AdjustDiagramArea(m_rectDiagramArea, rectPlotArea);
	}

	for (i = 0; i < m_arAxes.GetSize(); i++)
	{
		CBCGPChartAxis* pAxis = m_arAxes[i];

		if (pAxis == NULL)
		{
			continue;
		}

		ASSERT_VALID(pAxis);
		pAxis->CalcMajorMinorUnits(pGM);
	}

	CBCGPChartAxis* pXAxis = GetChartAxis(BCGP_CHART_X_PRIMARY_AXIS);
	CBCGPChartAxis* pYAxis = GetChartAxis(BCGP_CHART_Y_PRIMARY_AXIS);
	CBCGPChartAxis* pXSecondaryAxis = GetChartAxis(BCGP_CHART_X_SECONDARY_AXIS);
	CBCGPChartAxis* pYSecondaryAxis = GetChartAxis(BCGP_CHART_Y_SECONDARY_AXIS);

	if (pXAxis->m_axisLabelType != CBCGPChartAxis::ALT_NO_LABELS)
	{
		if (pXAxis->IsVertical())
		{
			double dblMaxLabelSize = max(pXAxis->m_szMaxLabelSize.cy, pXSecondaryAxis->m_szMaxLabelSize.cy);

			m_rectDiagramArea.top += dblMaxLabelSize / 4;
			m_rectDiagramArea.bottom -= dblMaxLabelSize / 4;
		}
		else if (!pXAxis->IsVertical() && !pXAxis->IsDisplayDataBetweenTickMarks())
		{
			double dblMaxLabelSize = max(pXAxis->m_szMaxLabelSize.cx, pXSecondaryAxis->m_szMaxLabelSize.cx);

			m_rectDiagramArea.left += dblMaxLabelSize / 4;
			m_rectDiagramArea.right -= dblMaxLabelSize / 4;

			if (bHasDataTable && m_dblDataTableHeaderColumnWidth > dblMaxLabelSize)
			{
				m_dblDataTableHeaderColumnWidth -= dblMaxLabelSize;
			}
		}
	}

	if (pYAxis->m_axisLabelType != CBCGPChartAxis::ALT_NO_LABELS)
	{
		if (pYAxis->IsVertical())
		{
			double dblMaxLabelSize = max(pYAxis->m_szMaxLabelSize.cy, pYSecondaryAxis->m_szMaxLabelSize.cy);
			m_rectDiagramArea.top += dblMaxLabelSize / 4;
			m_rectDiagramArea.bottom -= dblMaxLabelSize / 4;
			
		}
		else if (!pYSecondaryAxis->IsVertical() && !pXAxis->IsDisplayDataBetweenTickMarks())
		{
			double dblMaxLabelSize = max(pYAxis->m_szMaxLabelSize.cx, pYSecondaryAxis->m_szMaxLabelSize.cx);

			m_rectDiagramArea.left += dblMaxLabelSize / 4;
			m_rectDiagramArea.right -= dblMaxLabelSize / 4;

			if (bHasDataTable && m_dblDataTableHeaderColumnWidth > dblMaxLabelSize)
			{
				m_dblDataTableHeaderColumnWidth -= dblMaxLabelSize;
			}
		}
	}

	if (m_dblDataTableHeaderColumnWidth > 0.0)
	{
		for (i = 0; i < m_arAxes.GetSize(); i++)
		{
			CBCGPChartAxis* pAxis = m_arAxes[i];
			if (pAxis == NULL)
			{
				continue;
			}
			
			ASSERT_VALID(pAxis);

			CBCGPRect rectAxisName;
			OnGetAxisNameAreaRect(pAxis, rectAxisName);

			if (pAxis->IsVertical() && !rectAxisName.IsRectEmpty() && !pAxis->m_bIsSecondaryAxis)
			{
				if (m_dblDataTableHeaderColumnWidth >= rectAxisName.Width())
				{
					m_dblDataTableHeaderColumnWidth -= rectAxisName.Width();
					m_rectDiagramArea.left -= rectAxisName.Width();

					break;
				}
			}
		}
	}

	for (i = 0; i < m_arAxes.GetSize(); i++)
	{
		CBCGPChartAxis* pAxis = m_arAxes[i];

		if (pAxis == NULL)
		{
			continue;
		}

		ASSERT_VALID(pAxis);

		pAxis->UpdateAxisPos(m_rectDiagramArea);
	}

	for (i = 0; i < m_arAxes.GetSize(); i++)
	{
		CBCGPChartAxis* pAxis = m_arAxes[i];
		
		if (pAxis == NULL)
		{
			continue;
		}
		
		ASSERT_VALID(pAxis);
		
		if (pAxis->IsScaleBreakEnabled())
		{
			pAxis->CalcMajorMinorUnitsForScaleParts(pGM);
			pAxis->CalcMaxLabelSizeForScaleParts(pGM);
			pAxis->CalcMajorMinorUnitsForScaleParts(pGM);
		}
	}

	for (i = 0; i < m_arAxes.GetSize(); i++)
	{
		CBCGPChartAxis* pAxis = m_arAxes[i];

		if (pAxis == NULL)
		{
			continue;
		}

		ASSERT_VALID(pAxis);

		pAxis->CalcAxisPos(m_rectDiagramArea, FALSE);
		if (pAxis->IsAxisVisible())
		{
			pAxis->CalcMajorMinorUnits(pGM);
		}
	}

	for (i = 0; i < m_arAxes.GetSize(); i++)
	{
		CBCGPChartAxis* pAxis = m_arAxes[i];

		if (pAxis == NULL)
		{
			continue;
		}

		ASSERT_VALID(pAxis);

		pAxis->CalcLabelsRect(m_rectDiagramArea);
	}

	for (i = 0; i < m_arAxes.GetSize(); i++)
	{
		CBCGPChartAxis* pAxis = m_arAxes[i];

		if (pAxis == NULL)
		{
			continue;
		}

		ASSERT_VALID(pAxis);

		pAxis->CalcNameRect(pGM, m_rectDiagramArea, m_rectDiagramArea, TRUE);
	}

	for (i = 0; i < m_arAxes.GetSize(); i++)
	{
		CBCGPChartAxis* pAxis = m_arAxes[i];
		
		if (pAxis == NULL)
		{
			continue;
		}
		
		ASSERT_VALID(pAxis);
		
		if (pAxis->IsScaleBreakEnabled())
		{
			pAxis->CalcMajorMinorUnitsForScaleParts(pGM);
		}
	}

	if (bHasDataTable)
	{
		if (m_dblDataTableHeaderColumnWidth > 0.0)
		{
			m_dblDataTableHeaderColumnWidth = m_rectDiagramArea.left - m_rectPlotArea.left + m_rectPlotAreaPadding.left;
		}
		else
		{
			m_rectDataTableArea.left = m_rectDiagramArea.left;
		}

		m_rectDataTableArea.right = m_rectDiagramArea.right;

		CBCGPChartAxis* pAxis = m_arAxes.GetAt(BCGP_CHART_X_PRIMARY_AXIS);
		if (pAxis != NULL)
		{
			m_rectDataTableArea.top = pAxis->GetAxisRect().bottom;
		}

		m_rectDataTableArea.bottom = m_rectDataTableArea.top + dblDataTableHeight;
	}

	OnAfterRecalcLayout(pGM);
}
//****************************************************************************************
void CBCGPChartVisualObject::OnAfterRecalcLayout(CBCGPGraphicsManager* pGM)
{
	OnCalcChartEffectsScreenPositions(pGM);

	for (POSITION pos = m_lstChartObjects.GetHeadPosition(); pos != NULL;)
	{
		CBCGPChartObject* pObject = m_lstChartObjects.GetNext(pos);

		if (pObject != NULL)
		{
			ASSERT_VALID(pObject);
			pObject->OnCalcScreenPoints(pGM);
		}
	}

	SetDirty(FALSE);

	if (m_pWndOwner != NULL && m_pWndOwner->GetOwner()->GetSafeHwnd() != NULL)
	{
		m_pWndOwner->GetOwner()->SendMessage(BCGM_ON_CHART_AFTER_RECALC_LAYOUT, (WPARAM)GetID());
	}
}
//****************************************************************************************
void CBCGPChartVisualObject::OnColorThemeChanged()
{
	UpdateSeriesColors();
	UpdateRelatedLegendColors();

	if (!m_dataTableAreaFormat.m_bCustomInterlaceColor)
	{
		m_dataTableAreaFormat.m_brInterlaceFill.Empty();
	}
}
//****************************************************************************************
void CBCGPChartVisualObject::OnScaleRatioChanged(const CBCGPSize& sizeScaleRatioOld)
{
	ASSERT_VALID(this);
	
	if (IsChart3D())
	{
		GetDiagram3D()->ResetLabelSize();
	}

	m_titleAreaFormat.OnScaleRatioChanged(m_sizeScaleRatio, sizeScaleRatioOld);
	m_legendAreaFormat.OnScaleRatioChanged(m_sizeScaleRatio, sizeScaleRatioOld);
	m_plotAreaFormat.OnScaleRatioChanged(m_sizeScaleRatio, sizeScaleRatioOld);
	m_chartAreaFormat.OnScaleRatioChanged(m_sizeScaleRatio, sizeScaleRatioOld);
	m_dataTableAreaFormat.OnScaleRatioChanged(m_sizeScaleRatio, sizeScaleRatioOld);

	int i = 0;
	for (; i < GetSeriesCount(); i++)
	{
		CBCGPChartSeries* pSeries = GetSeries(i, FALSE);

		if (pSeries == NULL)
		{
			continue;
		}

		ASSERT_VALID(pSeries);

		pSeries->OnScaleRatioChanged(m_sizeScaleRatio, sizeScaleRatioOld);
	}

	for (i = 0; i < m_arAxes.GetSize(); i++)
	{
		CBCGPChartAxis* pAxis = m_arAxes[i];

		if (pAxis == NULL)
		{
			continue;
		}

		ASSERT_VALID(pAxis);

		pAxis->OnScaleRatioChanged(m_sizeScaleRatio, sizeScaleRatioOld);
	}
	
	for (POSITION pos = m_lstChartObjects.GetHeadPosition(); pos != NULL;)
	{
		CBCGPChartObject* pObject = m_lstChartObjects.GetNext(pos);

		if (pObject != NULL)
		{
			ASSERT_VALID(pObject);

			pObject->OnScaleRatioChanged(m_sizeScaleRatio, sizeScaleRatioOld);
		}
	}

	SetDirty();
}
//****************************************************************************************
CBCGPRect CBCGPChartVisualObject::GetPlotAreaPadding(BOOL bScaled) const
{
	if (m_bIsThumbnailMode)
	{
		return CBCGPRect();
	}

	if (!bScaled)
	{
		return m_rectPlotAreaPadding;
	}

	return CBCGPRect(m_rectPlotAreaPadding.left * m_sizeScaleRatio.cx, m_rectPlotAreaPadding.top * m_sizeScaleRatio.cy, 
					 m_rectPlotAreaPadding.right * m_sizeScaleRatio.cx, m_rectPlotAreaPadding.bottom * m_sizeScaleRatio.cy);
}
//****************************************************************************************
CBCGPSize CBCGPChartVisualObject::GetTitleAreaPadding(BOOL bScaled) const
{
	if (m_bIsThumbnailMode)
	{
		return CBCGPSize();
	}

	if (!bScaled)
	{
		return m_szTitleAreaPadding;
	}

	return CBCGPSize(m_szTitleAreaPadding.cx * m_sizeScaleRatio.cx, m_szTitleAreaPadding.cy * m_sizeScaleRatio.cy);
}
//****************************************************************************************
CBCGPSize CBCGPChartVisualObject::GetLegendAreaPadding(BOOL bScaled) const
{
	if (m_bIsThumbnailMode)
	{
		return CBCGPSize();
	}
	
	if (!bScaled)
	{
		return m_szLegendAreaPadding;
	}

	return CBCGPSize(m_szLegendAreaPadding.cx * m_sizeScaleRatio.cx, m_szLegendAreaPadding.cy * m_sizeScaleRatio.cy);
}
//****************************************************************************************
CBCGPSize CBCGPChartVisualObject::GetLegendElementSpacing(BOOL bScaled) const
{
	if (!bScaled)
	{
		return m_szLegendElementSpacing;
	}

	return CBCGPSize(m_szLegendElementSpacing.cx * m_sizeScaleRatio.cx, m_szLegendElementSpacing.cy * m_sizeScaleRatio.cy);
}
//****************************************************************************************
CBCGPSize CBCGPChartVisualObject::GetHitTestDataPointPrecision(BOOL bScaled) const
{
	if (!bScaled)
	{
		return m_szHitTestDataPointPrecision;
	}

	return CBCGPSize(m_szHitTestDataPointPrecision.cx * m_sizeScaleRatio.cx, m_szHitTestDataPointPrecision.cy * m_sizeScaleRatio.cy);
}
//****************************************************************************************
BOOL CBCGPChartVisualObject::OnKeyboardDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (nChar == VK_ESCAPE && m_bResizeAxisMode && m_pResizedAxis != NULL)
	{
		EndResizeAxis(TRUE);
		SetCursor (::LoadCursor (NULL, IDC_ARROW));
		return TRUE;
	}

	return CBCGPBaseVisualObject::OnKeyboardDown(nChar, nRepCnt, nFlags);
}
//****************************************************************************************
void CBCGPChartVisualObject::EnableMouseTrackingMode(DWORD dwHitInfoFlags)
{
	m_hitInfoFlags = (BCGPChartHitInfo::HitInfoTest)dwHitInfoFlags;
}
//****************************************************************************************
void CBCGPChartVisualObject::OnAxisScrolled(CBCGPChartAxis* pAxis)
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

	pOwner->SendMessage(BCGM_ON_CHART_AXIS_SCROLLED, (WPARAM)GetID(), (LPARAM) (LPVOID)pAxis);
}
//****************************************************************************************
void CBCGPChartVisualObject::OnAxisZoomed(CBCGPChartAxis* pAxis)
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
	
	pOwner->SendMessage(BCGM_ON_CHART_AXIS_ZOOMED, (WPARAM)GetID(), (LPARAM) (LPVOID)pAxis);
}
//****************************************************************************************
BOOL CBCGPChartVisualObject::FireMouseMessage(UINT nMsg, int nButton, const CBCGPPoint& pt, 
															BOOL bUseLastHit)
{
	if (m_pWndOwner->GetSafeHwnd() == NULL)
	{
		return FALSE;
	}

	CWnd* pOwner = m_pWndOwner->GetOwner();

	if (pOwner->GetSafeHwnd() == NULL)
	{
		return FALSE;
	}

	if (!bUseLastHit)
	{
		HitTestInternal(pt, &m_lastHit);
	}

	m_lastHit.m_nMouseButton = nButton;
	m_lastHit.m_ptHit = pt;

	if (pOwner->SendMessage(nMsg, (WPARAM)GetID(), (LPARAM) (LPVOID)&m_lastHit))
	{
		return TRUE;
	}

	return FALSE;
}
//****************************************************************************************
BOOL CBCGPChartVisualObject::OnMouseDown(int nButton, const CBCGPPoint& pt)
{
	ASSERT_VALID(this);

	CBCGPBaseVisualObject::OnMouseDown(nButton, pt);

	if (m_pWndOwner->GetSafeHwnd() == NULL)
	{
		return FALSE;
	}

	if (FireMouseMessage(BCGM_ON_CHART_MOUSE_DOWN, nButton, pt))
	{
		return FALSE;
	}

	if (m_bIsThumbnailMode)
	{
		return FALSE;
	}

	if (IsScrollEnabled() && nButton == 0)
	{
		BCGPChartHitInfo hitInfo;
		HitTest(pt, &hitInfo);

		if ((hitInfo.m_hitInfo & BCGPChartHitInfo::HIT_AXIS_SCROLL_BAR) != 0)
		{
			CBCGPChartAxis* pAxis = GetChartAxis(hitInfo.m_nIndex1);
			if (pAxis != NULL)
			{
				ASSERT_VALID(pAxis);
				if (pAxis->ScrollTo(pAxis->ValueFromPointByScrollRange(pt)))
				{
					RecalcMinMaxValues();
					SetDirty(TRUE, TRUE);
				}
				
				return TRUE;
			}
		}
		else if ((hitInfo.m_hitInfo & BCGPChartHitInfo::HIT_AXIS_THUMB) != 0)
		{
			if (hitInfo.m_nIndex2 == -1 && BeginThumbTrack(hitInfo.m_nIndex1))
			{
				return TRUE;
			}

			BOOL bLeftGrip = hitInfo.m_nIndex2 == 0;
			if (BeginThumbSize(hitInfo.m_nIndex1, bLeftGrip, pt, hitInfo.m_dblVal1))
			{
				return TRUE;
			}

			return FALSE;
		}
	}

	if (IsResizeAxesEnabled() && nButton == 0)
	{
		BCGPChartHitInfo hitInfo;
		HitTest(pt, &hitInfo, BCGPChartHitInfo::HIT_AXIS_RESIZE_BAND);

		if ((hitInfo.m_hitInfo & BCGPChartHitInfo::HIT_AXIS_RESIZE_BAND) != 0)
		{
			if (BeginResizeAxis(hitInfo.m_nIndex1, hitInfo.m_nIndex2, pt))
			{
				return TRUE;
			}
		}
	}

	if (m_bSelectionMode || m_bEnableMagnifier && m_mouseConfig.m_nMagnifierInModifier == 0 ||
		!NeedDisplayAxes())
	{
		return FALSE;
	}

	if (nButton == m_mouseConfig.m_nZoomButton || nButton == m_mouseConfig.m_nPanButton || 
		nButton == m_mouseConfig.m_nSelectionButton)
	{
		BOOL bSelect = (GetAsyncKeyState(m_mouseConfig.m_nZoomInModifier) & 0x8000 || 
						GetAsyncKeyState(m_mouseConfig.m_nZoomOutModifier) & 0x8000 || 
						GetAsyncKeyState(m_mouseConfig.m_nSelectionModifier) & 0x8000 ||
						m_mouseConfig.m_nZoomInModifier == 0 && m_bEnableZoom || 
						m_mouseConfig.m_nSelectionModifier == 0 && m_bEnableSelection);
						
		BOOL bPan = (GetAsyncKeyState(m_mouseConfig.m_nPanModifier) & 0x8000 ||
					 m_mouseConfig.m_nPanModifier == 0) && m_bEnablePan;
						
		if (bSelect && (nButton == m_mouseConfig.m_nZoomButton || nButton == m_mouseConfig.m_nSelectionButton))
		{
			BeginSelection(pt);
			return TRUE;
		}
		else if (bPan && nButton == m_mouseConfig.m_nPanButton)
		{
			BeginPan(pt);
			return TRUE;
		}
	}

	return FALSE;
}
//****************************************************************************************
void CBCGPChartVisualObject::OnMouseUp(int nButton, const CBCGPPoint& pt)
{
	ASSERT_VALID(this);

	CBCGPBaseVisualObject::OnMouseUp(nButton, pt);

	if (m_pWndOwner->GetSafeHwnd() == NULL)
	{
		return;
	}

	if (FireMouseMessage(BCGM_ON_CHART_MOUSE_UP, nButton, pt))
	{
		return;
	}

	if (m_bIsThumbnailMode)
	{
		return;
	}

	BOOL bWasTracked = m_bThumbTrackMode;

	m_bPanMode = FALSE;
	EndThumbTrack();
	EndThumbSize();
	EndResizeAxis();

	if (!NeedDisplayAxes())
	{
		return;
	}

	if (nButton == m_mouseConfig.m_nZoomButton && m_bSelectionMode)
	{
		if (GetCapture() == m_pWndOwner->GetSafeHwnd())
		{
			ReleaseCapture();
			EndSelection(pt);
		}

		if (m_bEnableZoom)
		{
			BOOL bZoomIn = GetAsyncKeyState(m_mouseConfig.m_nZoomInModifier) & 0x8000 || 
							m_mouseConfig.m_nZoomInModifier == 0;
			BOOL bZoomOut = GetAsyncKeyState(m_mouseConfig.m_nZoomOutModifier) & 0x8000;

			if (bZoomIn || bZoomOut)
			{
				ZoomByCurrentSelection(bZoomOut);
				RecalcMinMaxValues();
			}
		}

		m_bSelectionMode = FALSE;

		m_ptSelStart.SetPoint(0, 0);
		m_ptSelEnd.SetPoint(0, 0);
		SetDirty(TRUE, TRUE);
	}
	else if (!m_bSelectionMode && m_bEnableMagnifier)
	{
		BCGPChartHitInfo hitInfo;
		HitTest(pt, &hitInfo, BCGPChartHitInfo::HIT_AXIS_SCROLL_BAR);

		BOOL bCanMagnify = (hitInfo.m_hitInfo & BCGPChartHitInfo::HIT_AXIS_SCROLL_BAR) == 0;

		if (!bCanMagnify || bWasTracked)
		{
			return;
		}

		BOOL bMagnifyIn = GetAsyncKeyState(m_mouseConfig.m_nMagnifierInModifier) & 0x8000 || 
							m_mouseConfig.m_nMagnifierInModifier == 0;
		BOOL bMagnifyOut = GetAsyncKeyState(m_mouseConfig.m_nMagnifierOutModifier) & 0x8000 ||
							m_mouseConfig.m_nMagnifierOutModifier == 0;

		if (nButton == m_mouseConfig.m_nMagnifierInButton && bMagnifyIn)
		{
			Zoom(m_mouseConfig.m_nMagnifyFactor, pt);
		}
		else if (nButton == m_mouseConfig.m_nMagnifierOutButton && bMagnifyOut)
		{
			Zoom(-m_mouseConfig.m_nMagnifyFactor, pt);
		}
	}
}
//****************************************************************************************
void CBCGPChartVisualObject::OnMouseMove(const CBCGPPoint& pt)
{
	ASSERT_VALID(this);

	CBCGPBaseVisualObject::OnMouseMove(pt);

	if (m_hitInfoFlags != BCGPChartHitInfo::HIT_NONE)
	{
		HitTestInternal(pt, &m_lastHit);

		if ((m_lastHit.m_hitInfo & m_hitInfoFlags) != 0)
		{
			if (FireMouseMessage(BCGM_ON_CHART_MOUSE_TRACK, -1, pt, TRUE))
			{
				return;
			}
		}
	}
	
	if (GetCapture() != m_pWndOwner->GetSafeHwnd())
	{
		return;
	}

	if (m_bSelectionMode)
	{
		UpdateSelection(pt);
	}
	else if (m_bResizeAxisMode && m_pResizedAxis != NULL)
	{
		if (UpdateAxisSize(pt))
		{
			m_ptStartResize = pt;
		}
	}
	else if (m_bPanMode && m_bEnableScroll)
	{
		CBCGPChartDiagram3D* pDiagram3D = GetDiagram3D();

		if (IsChart3D())
		{
			if (pDiagram3D == NULL)
			{
				return;
			}

			CBCGPPoint ptOffset = pt - m_ptPanStart;
			CBCGPPoint ptCurrOffset = pDiagram3D->GetScrollOffset();

			ptCurrOffset += ptOffset;

			pDiagram3D->SetScrollOffset(ptCurrOffset);
			m_ptPanStart = pt;

			SetDirty(TRUE, TRUE);
		}
		else if (!NeedDisplayAxes())
		{
			return;
		}
		else
		{
			BOOL bScrolled = FALSE;
			BOOL bXAxisScrolled = FALSE;

			for (int i = 0; i < m_arAxes.GetSize(); i++)
			{
				CBCGPChartAxis* pAxis = m_arAxes[i];

				if (pAxis == NULL)
				{
					continue;
				}

				CArray<CBCGPRect, CBCGPRect> arRects;
				pAxis->GetBoundingRects(arRects);

				BOOL bRectFound = FALSE;

				for (int j = 0; j < arRects.GetSize(); j++)
				{
					CBCGPRect rectAxis = arRects[j];

					if (rectAxis.PtInRect(m_ptPanOrigin))
					{
						bRectFound = TRUE;
						break;
					}
				}

				if (!bRectFound)
				{
					continue;
				}

				double dblStart = pAxis->ValueFromPoint(pAxis->m_ptPanStart);
				double dblEnd = pAxis->ValueFromPoint(pt);

				double dblValScroll = dblStart - dblEnd;

				if (dblValScroll == 0)
				{
					continue;
				}

				BOOL bIsInPixels = TRUE;

				if (pAxis->IsIndexedSeries() || pAxis->IsFixedIntervalWidth())
				{
					bIsInPixels = FALSE;

					if (fabs(dblValScroll) < 0.5)
					{
						continue;
					}
				}

				if (pAxis->HasSeries() && pAxis->Scroll(dblValScroll, bIsInPixels))
				{
					bScrolled = TRUE;

					if (pAxis->IsKindOf(RUNTIME_CLASS(CBCGPChartAxisX)))
					{
						bXAxisScrolled = TRUE;
					}

					if (pAxis->IsVertical())
					{
						pAxis->m_ptPanStart.y = pt.y;
					}
					else
					{
						pAxis->m_ptPanStart.x = pt.x;
					}
				}
			}

			if (bScrolled)
			{
				RecalcMinMaxValues();
				SetDirty(TRUE, TRUE);
			}
		}
	}
	else if (m_bThumbTrackMode && m_pThumbTrackAxis != NULL)
	{
		ASSERT_VALID(m_pThumbTrackAxis);

		if (m_pThumbTrackAxis->ScrollTo(m_pThumbTrackAxis->ValueFromPointByScrollRange(pt)))
		{
			RecalcMinMaxValues();
		}

		SetDirty(TRUE, TRUE);
	}
	else if (m_bThumbSizeMode && m_pThumbSizeAxis != NULL)
	{
		ASSERT_VALID(m_pThumbSizeAxis);

		if (m_pThumbSizeAxis->SetThumbRect(pt, m_bThumbSizeLeft, m_dblThumbHitOffset))
		{
			RecalcMinMaxValues();
			SetDirty(TRUE, TRUE);
			m_ptThumbSizeStart = pt;
		}
	}

	if (!IsChart3D())
	{
		for (int i = 0; i < m_arAxes.GetSize(); i++)
		{
			CBCGPChartAxis* pAxis = m_arAxes[i];
			
			if (pAxis == NULL)
			{
				continue;
			}

			if (pAxis->GetComponentIndex() == CBCGPChartData::CI_Y && pAxis->IsZoomed() && pAxis->m_bUnzoomOnRangeBreak)
			{
				for (int j = 0; j < GetSeriesCount(); j++)
				{
					CBCGPChartSeries* pSeries = GetSeries(j, FALSE);

					if (pSeries == NULL || !pSeries->IsShownOnAxis(pAxis))
					{
						return;
					}

					CBCGPChartValue valMin = pSeries->GetMinValue(CBCGPChartData::CI_Y);
					CBCGPChartValue valMax = pSeries->GetMaxValue(CBCGPChartData::CI_Y);

					if (pAxis->GetMinDisplayedValue() >= valMin || pAxis->GetMaxDisplayedValue() <= valMax)
					{
						pAxis->UnZoom();
					}
				}
			}
		}
	}
}
//****************************************************************************************
void CBCGPChartVisualObject::OnMouseLeave()
{
	CBCGPBaseVisualObject::OnMouseLeave();

	if (m_hitInfoFlags != BCGPChartHitInfo::HIT_NONE)
	{
		m_lastHit.m_hitInfo = BCGPChartHitInfo::HIT_NONE;
		FireMouseMessage(BCGM_ON_CHART_MOUSE_TRACK, -1, CBCGPPoint(-1, -1), TRUE);
	}
}
//****************************************************************************************
BOOL CBCGPChartVisualObject::OnMouseWheel(const CBCGPPoint& pt, short zDelta)
{
	ASSERT_VALID(this);

	CBCGPBaseVisualObject::OnMouseWheel(pt, zDelta);

	if (!NeedDisplayAxes() && !IsChart3D())
	{
		return FALSE;
	}

	if (m_pWndOwner->GetSafeHwnd() == NULL || !m_bEnableScroll || 
		m_mouseConfig.m_wheelOptions == BCGPChartMouseConfig::MWO_NONE)
	{
		return FALSE;
	}


	float fScaleOrScroll = ((float)zDelta / WHEEL_DELTA);

	if (m_mouseConfig.m_wheelOptions == BCGPChartMouseConfig::MWO_SCROLL)
	{
		const BOOL bScrollVert = ::GetAsyncKeyState (m_mouseConfig.m_nScrollByWheelModifier) & 0x8000;

		CBCGPChartDiagram3D* pDiagram3D = GetDiagram3D();

		if (IsChart3D())
		{
			if (m_pDiagram3D != NULL)
			{
				CBCGPPoint ptOffset = m_pDiagram3D->GetScrollOffset();
				 
				bScrollVert ? ptOffset.y += fScaleOrScroll * 10 : ptOffset.x += fScaleOrScroll * 10;
				pDiagram3D->SetScrollOffset(ptOffset);
				SetDirty(TRUE, TRUE);
			}
		}
		else
		{
			BOOL bScrolled = FALSE;
			BOOL bXAxisScrolled = FALSE;

			for (int i = 0; i < m_arAxes.GetSize(); i++)
			{
				CBCGPChartAxis* pAxis = m_arAxes[i];

				if (pAxis == NULL || pAxis->IsVertical() && !bScrollVert || 
					!pAxis->IsVertical() && bScrollVert)
				{
					continue;
				}

				CBCGPRect rectBounds = pAxis->GetBoundingRect();

				if (rectBounds.PtInRect(pt))
				{
					if (pAxis->Scroll(fScaleOrScroll, FALSE))
					{
						bScrolled = TRUE;

						if (pAxis->IsKindOf(RUNTIME_CLASS(CBCGPChartAxisX)))
						{
							bXAxisScrolled = TRUE;
						}
					}
				}
			}

			if (bScrolled)
			{
				if (bXAxisScrolled)
				{
					RecalcMinMaxValues();
				}
				
				SetDirty(TRUE, TRUE);
			}
		}
	}
	else if (m_mouseConfig.m_wheelOptions == BCGPChartMouseConfig::MWO_ZOOM)
	{
		Zoom((int)(fScaleOrScroll * m_mouseConfig.m_nMagnifyFactor), pt);
	}


	if (m_hitInfoFlags != 0)
	{
		HitTestInternal(pt, &m_lastHit);
	
		if ((m_lastHit.m_hitInfo & m_hitInfoFlags) != 0)
		{
			FireMouseMessage(BCGM_ON_CHART_MOUSE_TRACK, -1, pt, TRUE);
		}
	}

	return TRUE;
}
//****************************************************************************************
BOOL CBCGPChartVisualObject::OnSetMouseCursor(const CBCGPPoint& pt)
{
	CBCGPBaseVisualObject::OnSetMouseCursor(pt);

	if (!m_bEnableMagnifier && !m_bEnableZoom && !m_bEnablePan && !IsResizeAxesEnabled()|| m_pWndOwner->GetSafeHwnd() == NULL)
	{
		return FALSE;
	}

	BOOL bDiagram3D = IsChart3D();
	
	for (int i = 0; i < m_arAxes.GetSize(); i++)
	{
		CBCGPChartAxis* pAxis = m_arAxes[i];

		if ((pAxis == NULL || !pAxis->HasSeries()) && !bDiagram3D)
		{
			continue;
		}

		CBCGPRect rectBounds = pAxis->GetBoundingRect();

		if (pAxis->CanShowScrollBar())
		{
			rectBounds.UnionRect(pAxis->GetScrollBarRect());
		}

		if ((rectBounds.PtInRect(pt) || bDiagram3D) && (m_bEnableMagnifier || m_bEnableZoom || m_bEnablePan))
		{
			BCGPChartHitInfo hitInfo;
			HitTest(pt, &hitInfo, BCGPChartHitInfo::HIT_AXIS_SCROLL_BAR | BCGPChartHitInfo::HIT_AXIS_THUMB);

			if ((hitInfo.m_hitInfo & BCGPChartHitInfo::HIT_AXIS_THUMB) != 0 && !bDiagram3D)
			{
				if (hitInfo.m_nIndex2 != -1)
				{
					CBCGPChartAxis* pAxis = GetChartAxis(hitInfo.m_nIndex1);

					if (pAxis != NULL && pAxis->IsThumbGripEnabled())
					{
						UINT idCursor = pAxis->IsVertical() ? AFX_IDC_TRACKNS : AFX_IDC_TRACKWE;
						::SetCursor(AfxGetApp()->LoadCursor (idCursor));
						return TRUE;
					}
				}
			}

			BOOL bCanSetMagnifyCursor = (hitInfo.m_hitInfo & 
				(BCGPChartHitInfo::HIT_AXIS_SCROLL_BAR | BCGPChartHitInfo::HIT_AXIS_THUMB)) == 0;

			if (m_bEnableMagnifier && !bCanSetMagnifyCursor)
			{
				return FALSE;
			}

#if (!defined _BCGSUITE_) && (!defined _BCGPCHART_STANDALONE)
			if (globalData.m_hcurMagnify == NULL)
			{
				globalData.m_hcurMagnify = AfxGetApp()->LoadCursor (AFX_IDC_MAGNIFY);
			}
#endif

			if (m_bEnableMagnifier)
			{
#if (!defined _BCGSUITE_) && (!defined _BCGPCHART_STANDALONE)
				::SetCursor (globalData.m_hcurMagnify);
#else
				::SetCursor(AfxGetApp()->LoadCursor (AFX_IDC_MAGNIFY));
#endif
				return TRUE;
			}
#if !defined _BCGPCHART_STANDALONE
			else if (m_bEnablePan && (pAxis->CanBeScrolled() || bDiagram3D))
			{
				::SetCursor (globalData.GetHandCursor());
				return TRUE;
			}
#endif
		}
		else if (IsResizeAxesEnabled())
		{
			BCGPChartHitInfo hitInfo;
			HitTest(pt, &hitInfo, BCGPChartHitInfo::HIT_ALL_AXIS_ELEMENTS);

			if ((hitInfo.m_hitInfo & BCGPChartHitInfo::HIT_AXIS_RESIZE_BAND) != 0)
			{
				CBCGPChartAxis* pAxis = GetChartAxis(hitInfo.m_nIndex1);

				if (pAxis != NULL)
				{
					ASSERT_VALID(pAxis);

					UINT nID = pAxis->IsVertical() ? AFX_IDC_VSPLITBAR : AFX_IDC_HSPLITBAR;
					::SetCursor(AfxGetApp()->LoadCursor (nID));

					return TRUE;
				}
			}
		}

		if (bDiagram3D)
		{
			break;
		}
	}

	return FALSE;
}
//****************************************************************************************
BOOL CBCGPChartVisualObject::OnGetToolTip(const CBCGPPoint& pt, CString& strToolTip, CString& strDescr)
{
	BCGPChartHitInfo hi;
	BOOL bHitResult = HitTest(pt, &hi, (DWORD)(BCGPChartHitInfo::HIT_AXIS | BCGPChartHitInfo::HIT_DATA_POINT | 
		BCGPChartHitInfo::HIT_AXIS_SCALE_BREAK | BCGPChartHitInfo::HIT_DATA_TABLE));

	strToolTip.Empty();
	strDescr.Empty();

	if (!bHitResult)
	{
		return TRUE;
	}

	if (hi.m_hitInfo == BCGPChartHitInfo::HIT_AXIS)
	{
		CBCGPChartAxis* pAxis = GetChartAxis(hi.m_nIndex1);
		if (pAxis == NULL || pAxis->IsLogScale())
		{
			return TRUE;
		}

		CString strAxisNameFormat;
		strAxisNameFormat.LoadString(IDS_BCGBARRES_CHART_TOOLTIP_AXIS_NAME_FORMAT);
		strToolTip.Format(strAxisNameFormat, pAxis->m_strAxisName);

		double dblVal = pAxis->ValueFromPoint(pt);
		CString strLabel;
		pAxis->GetDisplayedLabel(dblVal, strLabel);
		CString strAxisDescrFormat;
		strAxisDescrFormat.LoadString(IDS_BCGBARRES_CHART_TOOLTIP_AXIS_VALUE_FORMAT);
		strDescr.Format(strAxisDescrFormat, strLabel);
		
	}
	else if (hi.m_hitInfo == BCGPChartHitInfo::HIT_AXIS_SCALE_BREAK)
	{
		CBCGPChartAxis* pAxis = GetChartAxis(hi.m_nIndex1);
		if (pAxis == NULL)
		{
			return TRUE;
		}

		strToolTip = _T("Axis Scale Break");
		CString strLowValue;
		CString strHighValue;

		pAxis->GetDisplayedLabel(hi.m_dblVal1, strLowValue);
		pAxis->GetDisplayedLabel(hi.m_dblVal2, strHighValue);
		strDescr.Format(_T("%s - %s"), strLowValue, strHighValue);
	}
	else if (hi.m_hitInfo == BCGPChartHitInfo::HIT_DATA_POINT || hi.m_hitInfo == BCGPChartHitInfo::HIT_DATA_TABLE)
	{
		CBCGPChartSeries* pSeries = GetSeries(hi.m_nIndex1);
		if (pSeries == NULL || hi.m_nIndex2 < 0)
		{
			return TRUE;
		}

		if (hi.m_hitInfo == BCGPChartHitInfo::HIT_DATA_TABLE)
		{
			if (!pSeries->IsDataTableEntryVisible())
			{
				return TRUE;
			}
		}
		else
		{
			if (!pSeries->m_bVisible)
			{
				return TRUE;
			}
		}

		BOOL bResult = pSeries->OnGetDataPointTooltip(hi.m_nIndex2, strToolTip, strDescr);

		if (bResult && strToolTip.GetLength() == 0 && hi.m_hitInfo != BCGPChartHitInfo::HIT_DATA_TABLE)
		{
			strToolTip = _T(" ");
		}

		return bResult;
	}
	else
	{
		return TRUE;
	}

	return TRUE;
}
//****************************************************************************************
BOOL CBCGPChartVisualObject::HitTest(const CBCGPPoint& pt, BCGPChartHitInfo* pHitInfo, DWORD dwHitInfoFlags)
{
	if (m_bIsThumbnailMode)
	{
		return FALSE;
	}

	DWORD dwInfoFlags = m_hitInfoFlags;
	m_hitInfoFlags = (BCGPChartHitInfo::HitInfoTest) dwHitInfoFlags;

	BOOL bResult = HitTestInternal(pt, pHitInfo);
	m_hitInfoFlags = (BCGPChartHitInfo::HitInfoTest)dwInfoFlags;

	return bResult;
}
//****************************************************************************************
BOOL CBCGPChartVisualObject::HitTestInternal(const CBCGPPoint& pt, BCGPChartHitInfo* pHitInfo)
{
	ASSERT_VALID(this);
	ASSERT(pHitInfo != NULL);

	if (IsDirty() || pHitInfo == NULL) 	
	{
		return FALSE;
	}

	pHitInfo->m_nIndex1 = -1;
	pHitInfo->m_nIndex2 = -1;

	if (!m_rect.PtInRect(pt))
	{
		pHitInfo->m_hitInfo = BCGPChartHitInfo::HIT_NONE;
		return TRUE;
	}

	CBCGPRect rectTitle;
	OnGetTitleAreaRect(rectTitle);

	if (rectTitle.PtInRect(pt) && (m_hitInfoFlags & BCGPChartHitInfo::HIT_TITLE) != 0)
	{
		pHitInfo->m_hitInfo = BCGPChartHitInfo::HIT_TITLE;
		return TRUE;
	}

	if (m_rectDataTableArea.PtInRect(pt) && (m_hitInfoFlags & BCGPChartHitInfo::HIT_DATA_TABLE) != 0)
	{
		pHitInfo->m_hitInfo = BCGPChartHitInfo::HIT_DATA_TABLE;

		if (m_dblDataTableRowHeight != 0.0)
		{
			int nRow = (int)((pt.y - m_rectDataTableArea.top) / m_dblDataTableRowHeight);
			int nCurrRow = 0;

			for (int i = 0; i < m_arData.GetSize(); i++)
			{
				CBCGPChartSeries* pSeries = GetSeries(i, FALSE);
				
				if (pSeries == NULL || !pSeries->IsDataTableEntryVisible())
				{
					continue;
				}

				if (nCurrRow == nRow)
				{
					pHitInfo->m_nIndex1 = i;

					CBCGPChartAxis* pAxis = m_arAxes.GetAt(BCGP_CHART_X_PRIMARY_AXIS);
					if (pAxis != NULL)
					{
						ASSERT_VALID(pAxis);

						CBCGPRect rectAxis(pAxis->GetAxisRect(FALSE, FALSE, FALSE));
						int nDPIndex = -1;
						int nFirstIndex = -1;
						CBCGPPoint ptFirst;
						
						int nStart = pAxis->m_bReverseOrder ? max(0, pSeries->GetDataPointCount() - 1) : 0;
						int nFinish = pAxis->m_bReverseOrder ? 0 : max(0, (int)pSeries->GetDataPointCount() - 1);
						int offsetIndex = pAxis->m_bReverseOrder ? -pAxis->m_nValuesPerInterval : pAxis->m_nValuesPerInterval;
						
						int offset = pAxis->m_bReverseOrder ? -1 : 1;
						
						int j = 0;
						
						for (j = nStart; j != nFinish; j += offset)
						{
							const CBCGPChartDataPoint* pDP = pSeries->GetDataPointAt(j);
							if (pDP != NULL)
							{
								BOOL bIsEmpty = FALSE;
								ptFirst = ScreenPointFromChartData(pDP->GetData(), j, bIsEmpty, pSeries);
								
								if (rectAxis.left <= ptFirst.x && ptFirst.x <= rectAxis.right)
								{
									nFirstIndex = j;
									break;
								}
							}
						}
						
						nStart = pAxis->m_bReverseOrder ? (int)pAxis->m_arMajorGridLines.GetSize() - 1 : 0;
						nFinish = pAxis->m_bReverseOrder ? 0 : (int)pAxis->m_arMajorGridLines.GetSize() - 1;
						
						for (j = nStart; j != nFinish; j += offset)
						{
							double x1 = pAxis->m_arMajorGridLines[j].CenterPoint().x;
							double x2 = pAxis->m_arMajorGridLines[j + offset].CenterPoint().x;
							
							if (nDPIndex == -1)
							{
								if (pAxis->m_arMajorGridLines[j].left < ptFirst.x && ptFirst.x < pAxis->m_arMajorGridLines[j + offset].right)
								{
									nDPIndex = nFirstIndex;
								}
							}
							
							if (nDPIndex >= 0 && nDPIndex < pSeries->GetDataPointCount())
							{
								if (pt.x >= x1 && pt.x <= x2)
								{
									pHitInfo->m_nIndex2 = nDPIndex;
									break;
								}

								nDPIndex += offsetIndex;
							}
						}
					}
					break;

				}

				nCurrRow++;
			}
		}

		
		return TRUE;
	}

	if (m_chartLayout.m_legendPosition != BCGPChartLayout::LP_NONE && 
		!m_bIsThumbnailMode &&
		m_rectLegendArea.PtInRect(pt) && (m_hitInfoFlags & BCGPChartHitInfo::HIT_LEGEND) != 0)
	{
		if ((m_hitInfoFlags & BCGPChartHitInfo::HIT_LEGEND_ENTRY) != 0)
		{
			for (int i = 0; i < m_arData.GetSize(); i++)
			{
				CBCGPChartSeries* pSeries = GetSeries(i, FALSE);

				if (pSeries == NULL || !pSeries->IsLegendEntryVisible())
				{
					continue;
				}

				CBCGPRect rectKey;
				CBCGPRect rectLabel;
				CBCGPRect rectUnion;

				CBCGPRect rectLegend = m_rectLegendArea;

				rectLegend.DeflateRect(m_legendAreaFormat.GetContentPadding(TRUE));

				if (pSeries->m_bIncludeDataPointLabelsToLegend)
				{
					for (int j = 0; j < pSeries->GetLegendElementCount(); j++)
					{
						CBCGPChartDataPoint* pDp = (CBCGPChartDataPoint*) pSeries->GetDataPointAt(j);
						if (pSeries->m_bIncludeDataPointLabelsToLegend && pSeries->CanIncludeDataPointToLegend(j) && 
							pDp != NULL)
						{
							rectKey = pDp->m_rectLegendKey;
							rectLabel = pDp->m_rectLegendLabel;		
							rectKey.OffsetRect(rectLegend.TopLeft());
							rectLabel.OffsetRect(rectLegend.TopLeft());

							rectUnion.UnionRect(rectKey, rectLabel);

							if (rectUnion.PtInRect(pt))
							{
								pHitInfo->m_hitInfo = BCGPChartHitInfo::HIT_LEGEND_ENTRY;
								pHitInfo->m_nIndex1 = i;
								pHitInfo->m_nIndex2 = j;
								return TRUE;
							}
						}
					}
				}
				else
				{
					rectKey = pSeries->m_rectLegendKey;
					rectLabel = pSeries->m_rectLegendLabel;
					rectKey.OffsetRect(rectLegend.TopLeft());
					rectLabel.OffsetRect(rectLegend.TopLeft());
					rectUnion.UnionRect(rectKey, rectLabel);

					if (rectUnion.PtInRect(pt))
					{
						pHitInfo->m_hitInfo = BCGPChartHitInfo::HIT_LEGEND_ENTRY;
						pHitInfo->m_nIndex1 = i;
						pHitInfo->m_nIndex2 = -1;
						return TRUE;
					}
				}

			}
		}

		pHitInfo->m_hitInfo = BCGPChartHitInfo::HIT_LEGEND;
		return TRUE;
	}

	int i = 0;

	if ((m_hitInfoFlags & BCGPChartHitInfo::HIT_ALL_AXIS_ELEMENTS) != 0)
	{
		for (i = 0; i < m_arAxes.GetSize(); i++)
		{
			CBCGPChartAxis* pAxis = m_arAxes[i];
			if (pAxis == NULL)
			{
				continue;
			}

			ASSERT_VALID(pAxis);

			if (pAxis->HitTest(pt, pHitInfo, m_hitInfoFlags))
			{
				return TRUE;
			}
		}
	}

	if (m_Category != BCGPChartHistoricalLine && m_Category != BCGPChartLongData && 
		((m_hitInfoFlags & BCGPChartHitInfo::HIT_DATA_POINT) != 0 || 
		 (m_hitInfoFlags & BCGPChartHitInfo::HIT_DATA_LABEL) != 0))
	{
		if (IsChart3D())
		{
			CBCGPChartSeries* pSeries = GetSeries(0, FALSE);
			if (pSeries != NULL)
			{
				CBCGPBaseChartImpl* pImpl = pSeries->GetChartImpl();
				CBCGPChartShape3D* pShape = NULL;

				if (pImpl != NULL)
				{
					const CList<CBCGPChartSide3D*, CBCGPChartSide3D*>& sides = pImpl->GetVisibleSides();

					for (POSITION pos = sides.GetHeadPosition(); pos != NULL;)
					{
						CBCGPChartSide3D* pSide = sides.GetNext(pos);

						if (pSide->m_bIsWallSide)
						{
							continue;
						}

						if (pSide->HitTest(pt, &pShape) && pShape != NULL)
						{
							pHitInfo->m_hitInfo = BCGPChartHitInfo::HIT_DATA_POINT;
							pHitInfo->m_nIndex1 = FindSeriesIndex(pShape->m_pSeries);
							pHitInfo->m_nIndex2 = pShape->m_nDataPointIndex;
							return TRUE;
						}
					}
				}
			}
		}

		for (i = (int)m_arData.GetSize() - 1; i >= 0 ; i--)
		{
			CBCGPChartSeries* pSeries = GetSeries(i, FALSE);

			if (pSeries == NULL || !pSeries->m_bVisible || pSeries->GetChartCategory() == BCGPChartSurface3D)
			{
				continue;
			}

			if ((m_hitInfoFlags & BCGPChartHitInfo::HIT_DATA_LABEL) != 0)
			{
				for (int j = pSeries->GetMaxDataPointIndex(); j >= pSeries->GetMinDataPointIndex(); j--)
				{
					const BCGPChartFormatSeries* pSeriesFormat = pSeries->GetDataPointFormat(j, FALSE);

					if (pSeriesFormat == NULL)
					{
						continue;
					}

					if (pSeriesFormat->m_dataLabelFormat.m_options.m_bShowDataLabel)
					{
						CBCGPRect rectDataLabel = pSeries->GetDataPointLabelRect(j);

						if (rectDataLabel.PtInRect(pt) && (m_hitInfoFlags & BCGPChartHitInfo::HIT_DATA_LABEL) != 0)
						{
							pHitInfo->m_hitInfo = BCGPChartHitInfo::HIT_DATA_LABEL;
							pHitInfo->m_nIndex1 = i;
							pHitInfo->m_nIndex2 = j;
							return TRUE;
						}
					}
				}
			}

			if ((m_hitInfoFlags & BCGPChartHitInfo::HIT_DATA_POINT) != 0)
			{
				if (!IsChart3D())
				{
					if (pSeries->HitTestDataPoint(pt, pHitInfo))
					{
						pHitInfo->m_nIndex1 = i;
						return TRUE;
					}
				}
			}
		}
	}

	if ((m_hitInfoFlags & BCGPChartHitInfo::HIT_OBJECT) != 0)
	{
		for (int i = 0; i < 2; i++)
		{
			// walking from the tail two times; first time check for foreground objects;
			// second time check for background objects
			for (POSITION pos = m_lstChartObjects.GetTailPosition(); pos != NULL;)
			{
				CBCGPChartObject* pObj = m_lstChartObjects.GetPrev(pos);
				if (pObj != NULL)
				{
					ASSERT_VALID(pObj);

					if (!pObj->IsVisible())
					{
						continue;
					}

					if (i == 0 && !pObj->IsForeground() || i == 1 && pObj->IsForeground())
					{
						continue;
					}

					if (pObj->HitTest(pt))
					{
						pHitInfo->m_hitInfo = BCGPChartHitInfo::HIT_OBJECT;
						pHitInfo->m_pChartObject = pObj;
						pHitInfo->m_nIndex1 = pObj->m_nObjectID;

						return TRUE;
					}
				}
			}
		}
	}

	if ((m_hitInfoFlags & BCGPChartHitInfo::HIT_AXIS_GRIDLINE) != 0)
	{
		for (i = 0; i < m_arAxes.GetSize(); i++)
		{
			CBCGPChartAxis* pAxis = m_arAxes[i];

			if (pAxis == NULL)
			{
				continue;
			}

			ASSERT_VALID(pAxis);

			if (pAxis->HitGridLinesTest(pt, pHitInfo))
			{
				return TRUE;
			}
		}
	}

	if (m_rectPlotArea.PtInRect(pt) && (m_hitInfoFlags & BCGPChartHitInfo::HIT_DIAGRAM) != 0)
	{
		pHitInfo->m_hitInfo = BCGPChartHitInfo::HIT_DIAGRAM;

		for (int i = 0; i < m_arAxes.GetSize(); i++)
		{
			CBCGPChartAxis* pAxis = m_arAxes[i];
			if (pAxis == NULL || !pAxis->HasSeries())
			{
				continue;
			}

			CBCGPRect rect = pAxis->GetBoundingRect();
			rect.Normalize();

			if (rect.PtInRect(pt))
			{
				CBCGPChartAxis* pPerpAxis = pAxis->GetPerpendecularAxis();

				if (pAxis->IsKindOf(RUNTIME_CLASS(CBCGPChartAxisX)))
				{
					pHitInfo->m_nIndex1 = pAxis->m_nAxisID;
					pHitInfo->m_nIndex2 = pPerpAxis == NULL ? pAxis->m_nAxisID : pPerpAxis->m_nAxisID;
				}
				else
				{
					pHitInfo->m_nIndex2 = pAxis->m_nAxisID;
					pHitInfo->m_nIndex1 = pPerpAxis == NULL ? pAxis->m_nAxisID : pPerpAxis->m_nAxisID;
				}	

				return TRUE;
			}
		}

		return TRUE;
	}

	pHitInfo->m_hitInfo = BCGPChartHitInfo::HIT_CHART_AREA;
	return TRUE;
}
//****************************************************************************************
BOOL CBCGPChartVisualObject::OnGetAxisNameAreaRect(CBCGPChartAxis* pAxis, CBCGPRect& rectAxisNameArea)
{
	ASSERT_VALID(this);

	rectAxisNameArea.SetRectEmpty();
	if (pAxis != NULL)
	{
		rectAxisNameArea = pAxis->m_rectAxisName;
	}

	return FALSE;
}
//****************************************************************************************
CBCGPSize CBCGPChartVisualObject::OnCalcAxisNameSize(CBCGPGraphicsManager* pGM, CBCGPChartAxis* pAxis)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pGM);
	ASSERT(pAxis != NULL);

	CBCGPSize sz(0, 0);

	if (pAxis == NULL || !pAxis->m_bDisplayAxisName || m_bIsThumbnailMode)
	{
		return sz;
	}

	if (!pAxis->IsVertical() && !m_rectDataTableArea.IsRectEmpty())
	{
		return sz;
	}

	CBCGPSize szPadding = pAxis->m_axisNameFormat.GetContentPadding(TRUE);

	sz = OnGetTextSize(pGM, pAxis->m_strAxisName, pAxis->m_axisNameFormat.m_textFormat);
	sz.cx += szPadding.cx * 2;
	sz.cy += szPadding.cy * 2;

	return sz;
}
//****************************************************************************************
double CBCGPChartVisualObject::OnGetWallThickness() const
{
	double dblThickness = 0.;
	CBCGPChartAxis* pAxis = GetChartAxis(BCGP_CHART_Y_PRIMARY_AXIS);

	if (pAxis != NULL)
	{
		dblThickness = max(dblThickness, pAxis->GetMajorTickMarkLen(TRUE));
	}

	pAxis = GetChartAxis(BCGP_CHART_X_PRIMARY_AXIS);

	if (pAxis != NULL)
	{
		dblThickness = max(dblThickness, pAxis->GetMajorTickMarkLen(TRUE));
	}

	pAxis = GetChartAxis(BCGP_CHART_Z_PRIMARY_AXIS);

	if (pAxis != NULL)
	{
		dblThickness = max(dblThickness, pAxis->GetMajorTickMarkLen(TRUE));
	}

	return dblThickness;
}
//****************************************************************************************
void CBCGPChartVisualObject::OnCalcMinMaxValues(CBCGPChartAxis* pAxis)
{
	ASSERT_VALID(this);

	if (pAxis != NULL)
	{
		pAxis->CalcMinMaxValues();
	}
}
//****************************************************************************************
BOOL CBCGPChartVisualObject::OnGetMinMaxValues(CBCGPChartAxis* pAxis, double& dblMin, double& dblMax)
{
	UNREFERENCED_PARAMETER(pAxis);
	UNREFERENCED_PARAMETER(dblMin);
	UNREFERENCED_PARAMETER(dblMax);

	return FALSE;
}
//****************************************************************************************
BOOL CBCGPChartVisualObject::IsLegendHorizontal() const
{
	ASSERT_VALID(this);

	return m_chartLayout.m_legendPosition == BCGPChartLayout::LP_TOP || m_chartLayout.m_legendPosition == BCGPChartLayout::LP_BOTTOM;
}
//****************************************************************************************
CBCGPSize CBCGPChartVisualObject::CalcLegendSize(CBCGPGraphicsManager* pGM)
{
	ASSERT_VALID(this);

	CBCGPSize szLegend(0, 0);

	if (m_chartLayout.m_legendPosition == BCGPChartLayout::LP_NONE || pGM == NULL || m_bIsThumbnailMode)
	{
		return szLegend;
	}

	ASSERT_VALID(pGM);

	CBCGPSize szLegendElementSpacing = GetLegendElementSpacing(TRUE);

	CBCGPSize szTextSize(0, 0);

	CBCGPSize szMaxLegendKeySize(0, 0);
	CBCGPSize szMaxTextLabelSize(0, 0);

	double nTopOffset = 0;
	double nLeftOffset = 0;

	BOOL bIsLegendHorz = IsLegendHorizontal();

	int i = 0;

	if (IsShowSurfaceMapInLegend())
	{
		CBCGPChartSurfaceSeries* pSurfaceSeries = NULL;

		for (i = 0; i < m_arData.GetSize(); i++)
		{
			pSurfaceSeries = DYNAMIC_DOWNCAST(CBCGPChartSurfaceSeries, GetSeries(i));
			if (pSurfaceSeries == NULL || !pSurfaceSeries->IsLegendEntryVisible())
			{
				continue;
			}

			break;
		}

		if (pSurfaceSeries == NULL)
		{
			return szLegend;
		}

		pSurfaceSeries->InitLegendElements();

		const BCGPChartFormatSeries& formatSeries = pSurfaceSeries->GetSeriesFormat();
		szMaxLegendKeySize = OnCalcLegendKeySize(pSurfaceSeries, -1);

		double dblKeyToLabelDist = pSurfaceSeries->m_nLegendKeyToLabelDistance * m_sizeScaleRatio.cx + 
			szMaxLegendKeySize.cx;

		int nElementCount = pSurfaceSeries->GetLevelCount();

		for (i = 0; i < nElementCount; i++)
		{
			CString strLabel;
			pSurfaceSeries->LevelIndexToString(i, strLabel);

			szTextSize = pGM->GetTextSize(strLabel, formatSeries.m_legendLabelFormat.m_textFormat);

			szMaxTextLabelSize.cx = max(szMaxTextLabelSize.cx, szTextSize.cx);
			szMaxTextLabelSize.cy = max(szMaxTextLabelSize.cy, szTextSize.cy);

			CBCGPRect rectKey(nLeftOffset, nTopOffset, nLeftOffset + szMaxLegendKeySize.cx, nTopOffset + szMaxLegendKeySize.cy);
			CBCGPRect rectLabel(nLeftOffset + dblKeyToLabelDist, nTopOffset, 
				nLeftOffset + dblKeyToLabelDist + szTextSize.cx, nTopOffset + szTextSize.cy);

			if (bIsLegendHorz)
			{
				szLegend.cx += szTextSize.cx + dblKeyToLabelDist;
				szLegend.cy = max(szLegend.cy, max (szMaxLegendKeySize.cy, szTextSize.cy));
				szLegend.cx += szLegendElementSpacing.cx;
				
				nLeftOffset += szTextSize.cx + szLegendElementSpacing.cx + dblKeyToLabelDist;
			}
			else
			{
				szLegend.cy += max (szMaxLegendKeySize.cy, szTextSize.cy);
				szLegend.cx = max(szLegend.cx, dblKeyToLabelDist + szTextSize.cx);
				szLegend.cy += szLegendElementSpacing.cy;

				if (pSurfaceSeries->IsContinuousLegendKey())
				{
					rectKey.top = rectLabel.top;
					rectKey.bottom = rectLabel.bottom;
				}
				

				nTopOffset += szLegendElementSpacing.cy + max (szMaxLegendKeySize.cy, szTextSize.cy);
			}

			pSurfaceSeries->AddLegendElement(rectKey, rectLabel, strLabel);
		}

		if (bIsLegendHorz)
		{
			szLegend.cx -= szLegendElementSpacing.cx;
		}
		else
		{
			szLegend.cy -= szLegendElementSpacing.cy;
		}

		/// center key and label
		for (i = 0; i < pSurfaceSeries->GetLegendElementCount(); i++)
		{
			CBCGPRect rectKey;
			CBCGPRect rectLabel;

			pSurfaceSeries->GetLegendElementRects(i, rectKey, rectLabel);

			CBCGPSize szKey = rectKey.Size();
			CBCGPSize szLabel = rectLabel.Size();

			if (bIsLegendHorz)
			{
				rectKey.top = szLegend.cy / 2 - rectKey.Height() / 2;
				rectKey.bottom = rectKey.top + szKey.cy;

				rectLabel.top = szLegend.cy / 2 - rectLabel.Height() / 2;
				rectLabel.bottom = rectLabel.top + szLabel.cy;
			}
			else
			{
				if (rectKey.Height() > rectLabel.Height())
				{
					rectLabel.top = rectKey.CenterPoint().y - rectLabel.Height() / 2;
					rectLabel.bottom = rectLabel.top + szLabel.cy;
				}
				else if (rectKey.Height() < rectLabel.Height())
				{
					rectKey.top = rectLabel.CenterPoint().y - rectKey.Height() / 2;
					rectKey.bottom = rectKey.top + szKey.cy;
				}

				rectKey.right = szMaxLegendKeySize.cx;
				rectKey.left =  szMaxLegendKeySize.cx - szKey.cx;

				if (pSurfaceSeries->IsContinuousLegendKey())
				{
					rectKey.top -= szLegendElementSpacing.cy / 2;
					rectKey.bottom += szLegendElementSpacing.cy / 2;
				}
				
				rectLabel.left = szMaxLegendKeySize.cx + dblKeyToLabelDist;
				rectLabel.right = rectLabel.left + szLabel.cx;
			}

			pSurfaceSeries->SetLegendElementRects(i, rectKey, rectLabel);
		}

		CBCGPSize szPadding = m_legendAreaFormat.GetContentPadding(TRUE);

		szLegend.cx += szPadding.cx * 2;
		szLegend.cy += szPadding.cy * 2;

		return szLegend;
	}

	for (i = 0; i < m_arData.GetSize(); i++)
	{
		CBCGPChartSeries* pSeries = GetSeries(i);
		if (pSeries == NULL || !pSeries->IsLegendEntryVisible())
		{
			continue;
		}

		int nElementCount = pSeries->GetLegendElementCount();

		for (int j = 0 ; j < nElementCount; j++)
		{
			if (pSeries->m_bIncludeDataPointLabelsToLegend && !pSeries->CanIncludeDataPointToLegend(j))
			{
				continue;
			}

			CBCGPSize szKeySize = OnCalcLegendKeySize(pSeries, j);
			szMaxLegendKeySize.cx = max(szMaxLegendKeySize.cx, szKeySize.cx);
			szMaxLegendKeySize.cy = max(szMaxLegendKeySize.cy, szKeySize.cy);
		}
	}

	for (i = 0; i < m_arData.GetSize(); i++)
	{
		CBCGPChartSeries* pSeries = GetSeries(i);
		if (pSeries == NULL || !pSeries->IsLegendEntryVisible())
		{
			continue;
		}

		int nElementCount = pSeries->GetLegendElementCount();

		for (int j = 0 ; j < nElementCount; j++)
		{
			szTextSize = OnCalcLegendLabelSize(pGM, pSeries, j);

			const CBCGPChartDataPoint* pDp = pSeries->GetDataPointAt(j);

			if (pSeries->m_bIncludeDataPointLabelsToLegend && !pSeries->CanIncludeDataPointToLegend(j))
			{
				continue;
			}

			CBCGPRect& rectKey = (CBCGPRect&)(pSeries->m_bIncludeDataPointLabelsToLegend && pDp != NULL ? 
				pDp->m_rectLegendKey : pSeries->m_rectLegendKey);
			CBCGPRect& rectLabel = (CBCGPRect&)(pSeries->m_bIncludeDataPointLabelsToLegend && pDp != NULL ? 
				pDp->m_rectLegendLabel : pSeries->m_rectLegendLabel);

			szMaxTextLabelSize.cx = max(szMaxTextLabelSize.cx, szTextSize.cx);
			szMaxTextLabelSize.cy = max(szMaxTextLabelSize.cy, szTextSize.cy);

			rectKey.SetRect(nLeftOffset, nTopOffset, 
				nLeftOffset + szMaxLegendKeySize.cx, nTopOffset + szMaxLegendKeySize.cy);

			rectLabel.left = nLeftOffset + szMaxLegendKeySize.cx + pSeries->m_nLegendKeyToLabelDistance * m_sizeScaleRatio.cx; 
			rectLabel.top = nTopOffset;
			rectLabel.right = rectLabel.left + szTextSize.cx;
			rectLabel.bottom = nTopOffset + szTextSize.cy;

			if (bIsLegendHorz)
			{
				szLegend.cx += szMaxLegendKeySize.cx + szTextSize.cx + pSeries->m_nLegendKeyToLabelDistance * m_sizeScaleRatio.cx;
				szLegend.cy = max(szLegend.cy, max (szMaxLegendKeySize.cy, szTextSize.cy));
				szLegend.cx += szLegendElementSpacing.cx;
				
				nLeftOffset += szMaxLegendKeySize.cx + szTextSize.cx + szLegendElementSpacing.cx + 
								pSeries->m_nLegendKeyToLabelDistance * m_sizeScaleRatio.cx;
			}
			else
			{
				szLegend.cy += max (szMaxLegendKeySize.cy, szTextSize.cy);
				szLegend.cx = max(szLegend.cx, szMaxLegendKeySize.cx + szTextSize.cx + pSeries->m_nLegendKeyToLabelDistance * m_sizeScaleRatio.cx);
				szLegend.cy += szLegendElementSpacing.cy;

				nTopOffset += szLegendElementSpacing.cy + max (szMaxLegendKeySize.cy, szTextSize.cy);
			}
		}
	}

	if (bIsLegendHorz)
	{
		szLegend.cx -= szLegendElementSpacing.cx;
	}
	else
	{
		szLegend.cy -= szLegendElementSpacing.cy;
	}
	

	// center legend key and label
	for (i = 0; i < m_arData.GetSize(); i++)
	{
		CBCGPChartSeries* pSeries = GetSeries(i);
		if (pSeries == NULL || !pSeries->IsLegendEntryVisible())
		{
			continue;
		}

		int nElementCount = pSeries->GetLegendElementCount();

		for (int j = 0 ; j < nElementCount; j++)
		{
			const CBCGPChartDataPoint* pDp = pSeries->GetDataPointAt(j);

			if (pSeries->m_bIncludeDataPointLabelsToLegend && !pSeries->CanIncludeDataPointToLegend(j))
			{
				continue;
			}

			CBCGPRect& rectKey = (CBCGPRect&)(pSeries->m_bIncludeDataPointLabelsToLegend && pDp != NULL ? 
				pDp->m_rectLegendKey : pSeries->m_rectLegendKey);
			CBCGPRect& rectLabel = (CBCGPRect&)(pSeries->m_bIncludeDataPointLabelsToLegend && pDp != NULL ? 
				pDp->m_rectLegendLabel : pSeries->m_rectLegendLabel);

			CBCGPSize szKey = rectKey.Size();
			CBCGPSize szLabel = rectLabel.Size();

			if (bIsLegendHorz)
			{
				rectKey.top = szLegend.cy / 2 - rectKey.Height() / 2;
				rectKey.bottom = rectKey.top + szKey.cy;

				rectLabel.top = szLegend.cy / 2 - rectLabel.Height() / 2;
				rectLabel.bottom = rectLabel.top + szLabel.cy;
			}
			else
			{
				if (rectKey.Height() > rectLabel.Height())
				{
					rectLabel.top = rectKey.CenterPoint().y - rectLabel.Height() / 2;
					rectLabel.bottom = rectLabel.top + szLabel.cy;
				}
				else if (rectKey.Height() < rectLabel.Height())
				{
					rectKey.top = rectLabel.CenterPoint().y - rectKey.Height() / 2;
					rectKey.bottom = rectKey.top + szKey.cy;
				}

				rectKey.right = szMaxLegendKeySize.cx;
				rectKey.left =  szMaxLegendKeySize.cx - szKey.cx;

				rectLabel.left = szMaxLegendKeySize.cx + pSeries->m_nLegendKeyToLabelDistance * m_sizeScaleRatio.cx;
				rectLabel.right = rectLabel.left + szLabel.cx;
			}
		}
	}

	CBCGPSize szPadding = m_legendAreaFormat.GetContentPadding(TRUE);

	szLegend.cx += szPadding.cx * 2;
	szLegend.cy += szPadding.cy * 2;

	return szLegend;
}
//****************************************************************************************
BOOL CBCGPChartVisualObject::CalcDataTableSize(CBCGPGraphicsManager* pGM, double& dblDataTableHeaderColumnWidth, double& dblDataTableHeight, double& dblDataTableRowHeight, double& dblMaxSeriesKeyWidth)
{
	ASSERT_VALID(this);

	dblDataTableHeaderColumnWidth = 0.0;
	dblDataTableHeight = 0.0;
	dblDataTableRowHeight = 0.0;
	dblMaxSeriesKeyWidth = 0.0;

	if (pGM == NULL)
	{
		return FALSE;
	}

	ASSERT_VALID(pGM);

	int nSeriesCount = 0;

	const CBCGPSize sizePadding = m_dataTableAreaFormat.GetContentPadding(TRUE);

	BOOL bHeaderColumnIsEmpty = !m_dataTableAreaFormat.m_bShowLegendKeys;

	for (int i = 0; i < GetSeriesCount(); i++)
	{
		CBCGPChartSeries* pSeries = GetSeries(i, FALSE);
		if (pSeries == NULL || !pSeries->IsDataTableEntryVisible())
		{
			continue;
		}
		
		const CBCGPTextFormat& tf = m_dataTableAreaFormat.m_labelFormat.m_textFormat.IsEmpty() ?
			pSeries->GetSeriesFormat().m_dataLabelFormat.m_textFormat : m_dataTableAreaFormat.m_labelFormat.m_textFormat;

		double dblLegendHeight = 0.0;
		double dblLegendWidth = 0.0;

		CString strLabel;
		pSeries->GetDataTableName(strLabel);

		if (!strLabel.IsEmpty())
		{
			bHeaderColumnIsEmpty = FALSE;

			CBCGPSize szLabel = pGM->GetTextSize(strLabel, tf);

			dblLegendHeight = szLabel.cy;
			dblLegendWidth = szLabel.cx + 2.0 * sizePadding.cx;
		}

		if (m_dataTableAreaFormat.m_bShowLegendKeys)
		{
			CBCGPSize szKeySize = OnCalcLegendKeySize(pSeries, -1);

			dblMaxSeriesKeyWidth = max(dblMaxSeriesKeyWidth, szKeySize.cx);

			dblLegendHeight = max(dblLegendHeight, szKeySize.cy);
			dblLegendWidth += szKeySize.cx + sizePadding.cx;
		}

		double cy = pGM->GetTextSize(_T("XYZ"), tf).cy + 2.0 * sizePadding.cy;
		double dblRowsHeight = max(dblLegendHeight, cy);

		dblDataTableRowHeight = max(m_dblDataTableRowHeight, dblRowsHeight);
		dblDataTableHeaderColumnWidth = max(dblDataTableHeaderColumnWidth, dblLegendWidth);

		nSeriesCount++;
	}

	if (nSeriesCount == 0)
	{
		dblDataTableHeaderColumnWidth = 0.0;
		dblDataTableHeight = 0.0;
		dblDataTableRowHeight = 0.0;

		return FALSE;
	}

	dblDataTableHeight = nSeriesCount * dblDataTableRowHeight;

	if (bHeaderColumnIsEmpty)
	{
		dblDataTableHeaderColumnWidth = 0.0;
	}
	else
	{
		dblDataTableHeaderColumnWidth = min(dblDataTableHeaderColumnWidth, m_rect.Width() / 4);
	}

	return TRUE;
}
//****************************************************************************************
CBCGPSize CBCGPChartVisualObject::OnCalcLegendKeySize(CBCGPChartSeries* pSeries, int nDataPointIndex)
{
	ASSERT_VALID(this);

	if (pSeries == NULL)
	{
		return CBCGPSize (0, 0);
	}

	ASSERT_VALID(pSeries);

	CBCGPBaseChartImpl* pChartImpl = pSeries->GetChartImpl();

	if (pChartImpl != NULL)
	{
		return pChartImpl->OnCalcLegendKeySize(pSeries, nDataPointIndex);
	}

	return CBCGPSize (0, 0);
}
//****************************************************************************************
CBCGPSize CBCGPChartVisualObject::OnCalcLegendKeySizeEx(const BCGPChartCellParams& params, CBCGPChartSeries* pSeries)
{
	ASSERT_VALID(this);
	
	if (pSeries == NULL)
	{
		return CBCGPSize (0, 0);
	}
	
	ASSERT_VALID(pSeries);
	
	CBCGPBaseChartImpl* pChartImpl = pSeries->GetChartImpl();
	
	if (pChartImpl != NULL)
	{
		return pChartImpl->OnCalcLegendKeySizeEx(params, pSeries);
	}
	
	return CBCGPSize (0, 0);
}
//****************************************************************************************
CBCGPSize CBCGPChartVisualObject::OnCalcLegendLabelSize(CBCGPGraphicsManager* pGM, CBCGPChartSeries* pSeries, 
												int nDataPointIndex)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pGM);

	if (pGM == NULL || pSeries == NULL || m_bIsThumbnailMode)
	{
		return CBCGPSize(0, 0);
	}

	CString strLabel = pSeries->m_strSeriesName;
	const BCGPChartFormatSeries* pFormat = &pSeries->GetSeriesFormat();

	if (nDataPointIndex >= 0 && nDataPointIndex < pSeries->GetLegendElementCount() && pSeries->m_bIncludeDataPointLabelsToLegend)
	{
		if (pSeries->m_bIncludeDataPointLabelsToLegend && pSeries->CanIncludeDataPointToLegend(nDataPointIndex))
		{
			pSeries->OnGetDataPointLegendLabel(nDataPointIndex, strLabel);
			pFormat = pSeries->GetDataPointFormat(nDataPointIndex, FALSE);
		}
	}

	return pGM->GetTextSize(strLabel, pFormat->m_legendLabelFormat.m_textFormat);
}
//****************************************************************************************
CBCGPSize CBCGPChartVisualObject::OnCalcLegendLabelSizeEx(CBCGPGraphicsManager* pGM, 
							const CString& strLabel, const BCGPChartCellParams& params)
{
	return pGM->GetTextSize(strLabel, params.m_textFormat);
}
//****************************************************************************************
CBCGPSize CBCGPChartVisualObject::CalcTitleSize(CBCGPGraphicsManager* pGM)
{
	ASSERT_VALID(this);

	CBCGPSize szTitle(0, 0);

	if (!m_chartLayout.m_bShowChartTitle || pGM == NULL || m_bIsThumbnailMode)
	{
		return szTitle;
	}

	ASSERT_VALID(pGM);

	CBCGPSize szPadding = m_titleAreaFormat.GetContentPadding(TRUE);

	szTitle = pGM->GetTextSize(m_strChartTitle, m_titleAreaFormat.m_textFormat);
	szTitle += szPadding;

	return szTitle;
}
//****************************************************************************************
void CBCGPChartVisualObject::OnDraw(CBCGPGraphicsManager* pGMSrc, const CBCGPRect& /*rectClip*/, DWORD dwFlags)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pGMSrc);

	if ((dwFlags & BCGP_DRAW_STATIC) == 0)
	{
		if (m_pWndOwner != NULL && m_pWndOwner->GetOwner()->GetSafeHwnd() != NULL)
		{
			WPARAM wParam = MAKEWPARAM(GetID(), dwFlags);
			m_pWndOwner->GetOwner()->SendMessage(BCGM_ON_CHART_AFTER_DRAW, wParam, (LPARAM)(LPVOID)pGMSrc);
		}
		return;
	}

	BOOL bCacheImage = m_bCacheImage;

	if (pGMSrc->IsOffscreen())
	{
		bCacheImage = FALSE;
	}

	CBCGPGraphicsManager* pGM = pGMSrc;
	CBCGPGraphicsManager* pGMMem = NULL;

	CBCGPRect rectSaved;

	if (bCacheImage)
	{	
		if (m_ImageCache.GetHandle() == NULL)
		{
			SetDirty();
		}

		if (m_ImageCache.GetHandle() != NULL && !IsDirty() && (dwFlags & BCGP_DRAW_STATIC))
		{
			pGMSrc->DrawImage(m_ImageCache, m_rect.TopLeft());
			dwFlags &= ~BCGP_DRAW_STATIC;

			if (dwFlags == 0)
			{
				return;
			}
		}
	}

	BOOL bPrevAA = pGM->IsAntialiasingEnabled();
	pGM->EnableAntialiasing(m_bAAEnabled);

	if (bCacheImage && (dwFlags & BCGP_DRAW_STATIC))
	{
		pGMMem = pGM->CreateOffScreenManager(m_rect, &m_ImageCache);

		if (pGMMem != NULL)
		{
			pGM = pGMMem;

			rectSaved = m_rect;
			m_rect = m_rect - m_rect.TopLeft();
		}
	}

	BOOL bWasDirty = IsDirty();
	if (bWasDirty)
	{
		ArrangeStackedSeries();
		InvalidateTrendFormulaSeries();
		AdjustLayout(pGM);
	}

	if (CalcScreenPositions(pGM, bWasDirty))
	{
		SetDirty(TRUE);
		AdjustLayout(pGM);
		CalcScreenPositions(pGM, TRUE);
	}

	CBCGPColor clrClear = m_plotAreaFormat.m_brFillColor.GetColor();

	if (clrClear.IsNull())
	{
		clrClear = m_currentTheme.m_brPlotFillColor.GetColor();
	}

	CBCGPChartDiagram3D* pDiagram3D = GetDiagram3D();

	if (pDiagram3D == NULL)
	{
		TRACE0("3D diagram manager is NULL.\n");
		return;
	}

	CBCGPEngine3D* pEngine3D = pDiagram3D->GetEngine3D();

	if (pEngine3D == NULL)
	{
		TRACE0("3D Graphics engine is NULL.\n");
		return;
	}

	ASSERT_VALID(pEngine3D);

	pEngine3D->SetClearColor(clrClear);
	pDiagram3D->OnBegin3DDraw(pGM);

	pEngine3D->EnableAntialiasing(m_bAAEnabled);

	OnFillBackground (pGM);

	CBCGPRect rectPlotArea;
	OnGetPlotAreaRect(rectPlotArea);

	OnDrawChartArea(pGM, m_rect);

	if (m_rectDiagramArea.Width() > 2 && m_rectDiagramArea.Height() > 2)
	{
		OnDrawPlotArea(pGM, rectPlotArea, m_rectDiagramArea);
		
		if (pGM->GetType() == CBCGPGraphicsManager::BCGP_GRAPHICS_MANAGER_GDI && !m_bIsThumbnailMode)
		{
			OnDrawSelection(pGM);
		}

		OnDrawPlotAreaItems(pGM, rectPlotArea, m_rectDiagramArea);	
	}

	if (!m_bIsThumbnailMode)
	{
		CBCGPRect rectChartTitle;
		OnGetTitleAreaRect(rectChartTitle);
		OnDrawChartTitle(pGM, m_strChartTitle, rectChartTitle, m_titleAreaFormat);

		CBCGPRect rectLegend = m_rectLegendArea;
		
		OnDrawChartLegend(pGM, rectLegend, m_legendAreaFormat);
		OnDrawChartObjects(pGM, TRUE);

		if (pGM->GetType() == CBCGPGraphicsManager::BCGP_GRAPHICS_MANAGER_D2D)
		{
			OnDrawSelection(pGM);
		}

		if (!m_rectDataTableArea.IsRectEmpty())
		{
			OnDrawChartDataTable(pGM, m_rectDataTableArea, m_dblDataTableHeaderColumnWidth);
		}
	}

	if (pGMMem != NULL)
	{
		delete pGMMem;
		pGM = pGMSrc;
		pGMMem = NULL;

		m_rect = rectSaved;

		pGMSrc->DrawImage(m_ImageCache, m_rect.TopLeft());
	}

	if ((dwFlags & BCGP_DRAW_DYNAMIC) == BCGP_DRAW_DYNAMIC)
	{
		if (m_pWndOwner != NULL && m_pWndOwner->GetOwner()->GetSafeHwnd() != NULL)
		{
			WPARAM wParam = MAKEWPARAM(GetID(), dwFlags);
			m_pWndOwner->GetOwner()->SendMessage(BCGM_ON_CHART_AFTER_DRAW, wParam, (LPARAM)(LPVOID)pGM);
		}
	}

	pGM->EnableAntialiasing(bPrevAA);
}
//****************************************************************************************
void CBCGPChartVisualObject::OnAfterBegin3DDraw(CBCGPGraphicsManager* pGM)
{
	if (m_pWndOwner->GetSafeHwnd() == NULL)
	{
		return;
	}

	ASSERT_VALID(m_pWndOwner);

	if (m_pWndOwner->GetOwner()->GetSafeHwnd() == NULL)
	{
		return;
	}

	ASSERT_VALID(m_pWndOwner->GetOwner());

	WPARAM wParam = MAKEWPARAM(GetID(), 0);
	m_pWndOwner->GetOwner()->SendMessage(BCGM_ON_CHART_AFTER_BEGIN_3DDRAW, wParam, (LPARAM)(LPVOID)pGM);
}
//****************************************************************************************
void CBCGPChartVisualObject::OnBeforeEnd3DDraw(const CBCGPRect& /*rectTarget*/, CBCGPGraphicsManager* pGMTarget)
{
	if (m_pWndOwner->GetSafeHwnd() == NULL)
	{
		return;
	}

	ASSERT_VALID(m_pWndOwner);

	if (m_pWndOwner->GetOwner()->GetSafeHwnd() == NULL)
	{
		return;
	}

	ASSERT_VALID(m_pWndOwner->GetOwner());

	WPARAM wParam = MAKEWPARAM(GetID(), 0);
	m_pWndOwner->GetOwner()->SendMessage(BCGM_ON_CHART_BEFORE_END_3DDRAW, wParam, (LPARAM)(LPVOID)pGMTarget);
}
//****************************************************************************************
void CBCGPChartVisualObject::ArrangeStackedSeries()
{
	CMap<int, int, int, int> mapSeriesByGroup;
	CMap<int, int, int, int> mapSeriesByAxis;
		
	int nDistinctSeriesIndex = 0;

	for (int i = 0; i < GetSeriesCount(); i++)
	{
		CBCGPChartSeries* pSeries = GetSeries(i);

		if (pSeries == NULL || !pSeries->m_bVisible)
		{
			continue;
		}

		CBCGPChartAxis* pAxis = pSeries->GetRelatedAxis(CBCGPChartSeries::AI_X);

		if (pAxis == NULL)
		{
			continue;
		}

		ASSERT_VALID(pAxis);

		CBCGPBaseChartImpl* pChartImpl = pSeries->GetChartImpl();

		// group stacked series by Group ID and non stacked by X axis
		if (pChartImpl != NULL && pChartImpl->IsKindOf(RUNTIME_CLASS(CBCGPBarChartImpl)))
		{
			if (pSeries->IsStakedSeries())
			{
				// Group series by Group ID. Series in the same group should take the same distinct index
				int nGroupID = pSeries->GetGroupID();

				int nIndex = 0;
				if (!mapSeriesByGroup.Lookup(nGroupID, nIndex))
				{
					mapSeriesByGroup.SetAt(nGroupID, nDistinctSeriesIndex);
					pSeries->SetOrderIndex(nDistinctSeriesIndex);
					nDistinctSeriesIndex++;
					continue;
				}

				pSeries->SetOrderIndex(nIndex);
			}
			else
			{
				// Group series by Axis ID
				int nAxisID = pAxis->m_nAxisID;

				int nIndex = 0;
				if (!mapSeriesByAxis.Lookup(nAxisID, nIndex))
				{
					mapSeriesByAxis.SetAt(nAxisID, nIndex);
					pSeries->SetOrderIndex(0);
					continue;
				}

				nIndex++;
				pSeries->SetOrderIndex(nIndex);
				mapSeriesByAxis.SetAt(nAxisID, nIndex);
			}
		}
	}
}
//****************************************************************************************
BOOL CBCGPChartVisualObject::CalcScreenPositions(CBCGPGraphicsManager* pGM, BOOL bDirty)
{
	BOOL bNeedRecalc = FALSE;

	for (int i = 0; i < m_arData.GetSize(); i++)
	{
		CBCGPChartSeries* pSeries = GetSeries(i, FALSE);
		if (pSeries == NULL || !pSeries->m_bVisible)
		{
			continue;
		}

		CBCGPChartBarSeries* pBarSeries = DYNAMIC_DOWNCAST(CBCGPChartBarSeries, pSeries);

		if (pBarSeries != NULL)
		{
			pBarSeries->CalcNumberOfSeriesOnAxis();
		}

		CBCGPBaseChartImpl* pChartImpl = pSeries->GetChartImpl();
		if (pChartImpl != NULL)
		{
			if (bDirty)
			{
				double dblDepthPercent = 0;

				CBCGPChartDiagram3D* pDiagram3D = GetDiagram3D();

				if (IsChart3D() && pDiagram3D != NULL)   
				{
					dblDepthPercent = pDiagram3D->GetBaseDepthPercent();
					// clear the depth percent; for 3D charts it will be set by OnCalcScreenPositions
					pDiagram3D->m_dblBaseDepthPercent = 0.;
				}

				pSeries->ClearBoundingRectMap();
				pChartImpl->OnCalcScreenPositions(pGM, m_rectDiagramArea, pSeries);

				if (IsChart3D() && pDiagram3D != NULL && fabs(dblDepthPercent - pDiagram3D->GetBaseDepthPercent()) > 2 * DBL_EPSILON)
				{
					bNeedRecalc = TRUE;
				}
			}
		}
	}
	
	if (IsSmartLabelsEnabled() && !IsChart3D() && bDirty)
	{
		ArrangeDataLabels(pGM);
	}
	
	return bNeedRecalc;
}
//****************************************************************************************
void CBCGPChartVisualObject::EnableSmartLabels(BOOL bEnable)
{
	m_bSmartLabelsEnabled = bEnable;
	SetDirty(TRUE, TRUE);
}
//****************************************************************************************
void CBCGPChartVisualObject::SetSmartLabelsParams(const BCGPChartSmartLabelParams& params, BOOL bRedraw)
{
	m_smartLabelsParams = params;
	SetDirty(TRUE, bRedraw);
}
//****************************************************************************************
static BOOL IsIntersected(const CArray<RECT, RECT&>& arRects, const CBCGPRect& rectIn)
{
	CRect rect = rectIn;

	for (int i = 0; i < arRects.GetSize(); i++)
	{
		CRect rectInter;
		CRect rectCurr = arRects[i];

		if (rectInter.IntersectRect(rect, rectCurr))
		{
			return TRUE;
		}
	}

	return FALSE;
}
//****************************************************************************************
static void CreateArrayOfNearest(const CArray<RECT, RECT&>& arRects, CArray<RECT, RECT&>& arRectsOut, const CRect& rectBounds)
{
	for (int i = 0; i < arRects.GetSize(); i++)
	{
		CRect rectInter;
		CRect rectCurr = arRects[i];

		if (rectInter.IntersectRect(rectBounds, rectCurr))
		{
			arRectsOut.Add(rectCurr);
		}
	}
}
//****************************************************************************************
void CBCGPChartVisualObject::ArrangeDataLabels(CBCGPGraphicsManager* pGM)
{
	if (IsChart3D())
	{
		return;
	}

	CArray<RECT, RECT&>	arRects;
	const int nSpacing = 4;
	
	BOOL bCanUseAngles = m_smartLabelsParams.m_bArrangeByAngle;
	BOOL bTreatAngleAsSide = FALSE;
	BOOL bOnlyColumns = TRUE;

	for (int iSeries = 0; iSeries < GetSeriesCount(); iSeries++)
	{
		CBCGPChartSeries* pSeries = GetSeries(iSeries, FALSE);
		if (pSeries == NULL || !pSeries->m_bVisible)
		{
			continue;
		}

		if (pSeries->IsOptimizedLongDataMode())
		{
			return;
		}

		BCGPChartCategory category = pSeries->GetChartCategory();

		if (category == BCGPChartPie || category == BCGPChartPie3D)
		{
			return;
		}

		if ((category != BCGPChartColumn && category != BCGPChartBar))
		{
			bOnlyColumns = FALSE;
			break;
		}
	}

	if (bOnlyColumns)
	{
		return;
	}

	if (!m_smartLabelsParams.m_bArrangeByDistance && !m_smartLabelsParams.m_bArrangeByAngle)
	{
		for (int iSeries = 0; iSeries < GetSeriesCount(); iSeries++)
		{
			CBCGPChartSeries* pSeries = GetSeries(iSeries, FALSE);
			if (pSeries == NULL)
			{
				continue;
			}

			for (int nDPIndex = pSeries->GetMinDataPointIndex(); nDPIndex <= pSeries->GetMaxDataPointIndex(); nDPIndex++)
			{
				CBCGPChartDataPoint* pDataPoint = (CBCGPChartDataPoint*)pSeries->GetDataPointAt(nDPIndex);
				pDataPoint->m_dblSmartLabelDistance = DBL_MAX;
				pDataPoint->m_nSmartLabelAngle = -1;
				pSeries->GetChartImpl()->OnCalcDataPointLabelRect(pGM, pDataPoint, m_rectPlotArea, pSeries, nDPIndex);
			}
		}

		return;
	}

	for (int nStep = 0; nStep < 2; nStep++)
	{
		for (int iSeries = 0; iSeries < GetSeriesCount(); iSeries++)
		{
			CBCGPChartSeries* pSeries = GetSeries(iSeries, FALSE);
			if (pSeries == NULL || !pSeries->m_bVisible)
			{
				continue;
			}

			BCGPChartCategory category = pSeries->GetChartCategory();

			if ((category == BCGPChartColumn || category == BCGPChartBar) && nStep == 1)
			{
				continue;
			}

			if (category == BCGPChartFunnel || category == BCGPChartFunnel3D ||
				category == BCGPChartPyramid || category == BCGPChartPyramid3D)
			{
				bCanUseAngles = FALSE;
				bTreatAngleAsSide = TRUE;

				CBCGPChartPyramidSeries* pPyramidSeries = DYNAMIC_DOWNCAST(CBCGPChartPyramidSeries, pSeries);

				if (pPyramidSeries != NULL && pPyramidSeries->m_bDataLabelsInColumns)
				{
					return;
				}
			}

			for (int nDPIndex = pSeries->GetMinDataPointIndex(); nDPIndex <= pSeries->GetMaxDataPointIndex(); nDPIndex++)
			{
				if (pSeries->IsDataPointScreenPointsEmpty(nDPIndex))
				{
					continue;
				}

				const BCGPChartFormatSeries* pFormat = pSeries->GetDataPointFormat(nDPIndex, FALSE);
				CPoint ptDataPoint = pSeries->GetDataPointScreenPoint(nDPIndex, 0);
				CBCGPSize szScaleRatio = pFormat->m_dataLabelFormat.m_options.GetScaleRatio();

				if (nStep == 0)
				{
					CRect rectMarker(ptDataPoint, ptDataPoint);
					CSize szMarkerSize = pFormat->m_markerFormat.GetMarkerSize();

					rectMarker.InflateRect(szMarkerSize.cx / 2 + nSpacing, szMarkerSize.cy / 2 + nSpacing);
					
					arRects.Add(rectMarker);
					
					if (category == BCGPChartColumn || category == BCGPChartBar)
					{
						CRect rectLabel = pSeries->GetDataPointLabelRect(nDPIndex);

						rectLabel.InflateRect(nSpacing, nSpacing);
						arRects.Add(rectLabel);
					}
				}
				else if (pFormat->m_dataLabelFormat.m_options.m_bShowDataLabel)
				{
					CBCGPChartDataPoint* pDataPoint = (CBCGPChartDataPoint*)pSeries->GetDataPointAt(nDPIndex);
					pDataPoint->m_dblSmartLabelDistance = DBL_MAX;

					const int nDefaultAngle = (int)pFormat->m_dataLabelFormat.m_options.m_dblAngle;
					const int nDefualtDistance = (int)pSeries->GetDataPointLabelDistance(nDPIndex);

					CSize sizeLabel = pSeries->GetDataPointLabelRect(nDPIndex).Size();

					int nDistance = nDefualtDistance;
					int nEndDistance = nDefualtDistance;
						
					if (m_smartLabelsParams.m_bArrangeByDistance)
					{
						int nMaxAllowedDistance = bcg_clamp(m_smartLabelsParams.m_nMaxAllowedDistancePercent, 100, 1000);
						nEndDistance = nDefualtDistance * nMaxAllowedDistance / 100;
					}

					int nDistanceStep = (nEndDistance - nDistance) / 10;

					if (nDistanceStep == 0 && nEndDistance != nDistance)
					{
						nDistanceStep = nEndDistance - nDistance;
					}

					int nFinalAngle = nDefaultAngle;

					if (bCanUseAngles)
					{
						int nAngleOffset = bcg_clamp(m_smartLabelsParams.m_nAngleOffset, 0, 360);
						nFinalAngle += nAngleOffset;
					}

					int nAngleStep = (nFinalAngle - nDefaultAngle) / 20;

					if (nAngleStep == 0 && nFinalAngle != nDefaultAngle)
					{
						nAngleStep = nFinalAngle - nDefaultAngle;
					}

					CArray<RECT, RECT&> arRectsCurr;
					
					CRect rectBounds(ptDataPoint, ptDataPoint);
					rectBounds.InflateRect(nEndDistance * 3 + sizeLabel.cx / 2, nEndDistance * 3 + sizeLabel.cy / 2);

					CreateArrayOfNearest(arRects, arRectsCurr, rectBounds);

					int nDPAngle = -1;
					BOOL bReady = FALSE;

					for (; !bReady && nDistance <= nEndDistance; nDistance += nDistanceStep)
					{
						for (int nAngle = nDefaultAngle; nAngle <= nFinalAngle; nAngle += nAngleStep)
						{
							CRect rectLabel = CalcSmartRect(ptDataPoint, sizeLabel, nAngle, nDistance, bTreatAngleAsSide);
							
							if (!IsIntersected(arRectsCurr, rectLabel))
							{
								rectLabel.InflateRect(nSpacing, nSpacing);
								arRects.Add(rectLabel);

								nDPAngle = nAngle;

								bReady = TRUE;
								break;
							}

							if (!bCanUseAngles || nAngleStep == 0)
							{
								break;
							}
						}

						if (bReady)
						{
							break;
						}

						if (!m_smartLabelsParams.m_bArrangeByDistance || nDistanceStep == 0)
						{
							break;
						}
					}

					if (bCanUseAngles)
					{
						pDataPoint->m_nSmartLabelAngle = nDPAngle;
					}

					pDataPoint->m_bShowSmartLabel = bReady;

					if (bReady)
					{
						if (m_smartLabelsParams.m_bArrangeByDistance)
						{
							pDataPoint->m_dblSmartLabelDistance = nDistance / max(szScaleRatio.cx, szScaleRatio.cy);
						}
						pSeries->GetChartImpl()->OnCalcDataPointLabelRect(pGM, pDataPoint, m_rectPlotArea, pSeries, nDPIndex);
					}
					else
					{
						pSeries->SetDataPointLabelRect(nDPIndex, CRect(0, 0, 0, 0));
						pDataPoint->m_dblSmartLabelDistance = DBL_MAX;
					}
				}
			}
		}
	}
}
//****************************************************************************************
CRect CBCGPChartVisualObject::CalcSmartRect(const CBCGPPoint& ptCenter, const CSize& size, int nAngle, int nDistance, 
											BOOL bTreatAngleAsSide)
{
	double dblLabelAngle = bcg_deg2rad(nAngle);
	double dblCos = cos(dblLabelAngle);
	double dblSin = sin(dblLabelAngle);
	
	CPoint ptOffset;
	CPoint ptLabelCenter;

	if (bTreatAngleAsSide)
	{
		ptLabelCenter.y = (int)ptCenter.y;
		if (nAngle > 0)
		{
			ptLabelCenter.x = (int)(ptCenter.x + nDistance);
		}
		else
		{
			ptLabelCenter.x = (int)(ptCenter.x - nDistance);
		}
	}
	else
	{
		ptOffset.x = (int)(size.cx / 2 * dblSin); 
		ptOffset.y = (int)(-size.cy / 2 * dblCos);
		ptLabelCenter.x = (int)(ptCenter.x + dblSin * nDistance);
		ptLabelCenter.y = (int)(ptCenter.y - dblCos * nDistance);
	}
	
	CRect rectLabel;

	rectLabel.SetRect(ptLabelCenter.x - size.cx / 2, 
		ptLabelCenter.y - size.cy / 2, 
		ptLabelCenter.x + size.cx / 2, 
		ptLabelCenter.y + size.cy / 2);

	rectLabel.OffsetRect(ptOffset);
	return rectLabel;
} 
//****************************************************************************************
void CBCGPChartVisualObject::OnDrawChartRect(CBCGPGraphicsManager* pGM, const CBCGPRect& rect, 
							const CBCGPBrush* pBrFill, const CBCGPBrush* pBrLine, double dblLineWidth, 
							const CBCGPStrokeStyle* pStrokeStyle, BOOL bNoFill, BOOL bNoLine)
{
	if (!bNoFill && pBrFill != NULL && !pBrFill->IsEmpty())
	{
		pGM->FillRectangle(rect, *pBrFill);

		if (pGM->IsDrawShadowMode())
		{
			return;
		}
	}

	if (!bNoLine && pBrLine != NULL && !pBrLine->IsEmpty())
	{
		if ((pStrokeStyle == NULL || pStrokeStyle->IsEmpty()) &&
			pBrLine->GetGradientType() == CBCGPBrush::BCGP_NO_GRADIENT &&
			dblLineWidth == 1.)
		{
			pGM->DrawLine(rect.left, rect.top, rect.right, rect.top, *pBrLine);
			pGM->DrawLine(rect.right, rect.top, rect.right, rect.bottom, *pBrLine);
			pGM->DrawLine(rect.right, rect.bottom, rect.left, rect.bottom, *pBrLine);
			pGM->DrawLine(rect.left, rect.bottom, rect.left, rect.top, *pBrLine);
		}
		else if (pBrLine != NULL)
		{
			pGM->DrawRectangle(rect, *pBrLine, dblLineWidth, pStrokeStyle);
		}
	}
}
//****************************************************************************************
void CBCGPChartVisualObject::OnDrawChartEllipse(CBCGPGraphicsManager* pGM, const CBCGPEllipse& ellipse, 
										 const CBCGPBrush* pBrFill, const CBCGPBrush* pBrLine,
										 double dblLineWidth, const CBCGPStrokeStyle& stroke, 
										 BOOL bNoFill, BOOL bNoLine)
{
	BOOL bFill = FALSE;
	
	if (!bNoFill && pBrFill != NULL && !pBrFill->IsEmpty())
	{
		pGM->FillEllipse(ellipse, *pBrFill);
		bFill = TRUE;

		if (pGM->IsDrawShadowMode())
		{
			return;
		}
	}
	
	if (!bNoLine && pBrLine != NULL && !pBrLine->IsEmpty())
	{
		pGM->DrawEllipse(ellipse, *pBrLine, dblLineWidth, &stroke);
		
		if (bFill && !pGM->IsSupported(BCGP_GRAPHICS_MANAGER_ANTIALIAS))
		{
			CBCGPEllipse ellipseInternal = ellipse;
			ellipseInternal.radiusX -= 1.;
			ellipseInternal.radiusY -= 1.;
			
			pGM->DrawEllipse(ellipseInternal, *pBrFill, dblLineWidth, &stroke);
		}
	}
}
//****************************************************************************************
void CBCGPChartVisualObject::OnDrawChartGeometry(CBCGPGraphicsManager* pGM, const CBCGPGeometry& geometry, 
												const CBCGPBrush* pBrFill, const CBCGPBrush* pBrLine,
												double dblLineWidth, const CBCGPStrokeStyle& stroke, 
												BOOL bNoFill, BOOL bNoLine)
{
	if (!bNoFill && pBrFill != NULL)
	{
		pGM->FillGeometry(geometry, *pBrFill);
	}
	
	if (!bNoLine && pBrLine != NULL && !pBrLine->IsEmpty() && (!pGM->IsDrawShadowMode() || bNoFill))
	{
		pGM->DrawGeometry(geometry, *pBrLine, dblLineWidth, &stroke);
	}
}
//****************************************************************************************
void CBCGPChartVisualObject::OnDrawChartElement(CBCGPGraphicsManager* pGM, const CBCGPRect& rect, 
									const BCGPChartFormatArea& format, 
									CBCGPChartVisualObject::ChartElement chartElement, 
									BOOL bNoFill, BOOL bNoLine)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pGM);

	CBCGPElementColors colors;
	GetElementColors(format, chartElement, colors);

	OnDrawChartRect(pGM, rect, colors.pBrFill, colors.pBrLine, format.m_outlineFormat.GetLineWidth(TRUE), 
		&format.m_outlineFormat.m_strokeStyle, bNoFill, bNoLine);
}
//****************************************************************************************
void CBCGPChartVisualObject::OnDrawChartElement(CBCGPGraphicsManager* pGM, const CBCGPEllipse& ellipse, 
									const BCGPChartFormatArea& format, CBCGPChartVisualObject::ChartElement chartElement, 
									BOOL bNoFill, BOOL bNoLine)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pGM);

	CBCGPElementColors colors;
	GetElementColors(format, chartElement, colors);

	OnDrawChartEllipse(pGM, ellipse, colors.pBrFill, colors.pBrLine, format.m_outlineFormat.GetLineWidth(TRUE), 
		format.m_outlineFormat.m_strokeStyle, bNoFill, bNoLine);
}
//****************************************************************************************
void CBCGPChartVisualObject::OnDrawChartElement(CBCGPGraphicsManager* pGM, const CBCGPGeometry& geometry, 
										const BCGPChartFormatArea& format, CBCGPChartVisualObject::ChartElement chartElement, 
										BOOL bNoFill, BOOL bNoLine)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pGM);

	CBCGPElementColors colors;
	GetElementColors(format, chartElement, colors);

	OnDrawChartGeometry(pGM, geometry, colors.pBrFill, colors.pBrLine, format.m_outlineFormat.GetLineWidth(TRUE), 
		format.m_outlineFormat.m_strokeStyle, bNoFill, bNoLine);
}
//****************************************************************************************
void CBCGPChartVisualObject::OnDrawChartSeriesItem(CBCGPGraphicsManager* pGM, const CBCGPRect& rect, 
						   CBCGPChartSeries* pSeries, int nDataPointIndex, CBCGPChartVisualObject::ChartElement chartElement, BOOL bNoFill, BOOL bNoLine)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pGM);

	BCGPSeriesColorsPtr colors;
	const BCGPChartFormatSeries* pFormatSeries = pSeries->GetColors(colors, nDataPointIndex);
	if (pFormatSeries == NULL)
	{
		return;
	}

	CBCGPBrush* pBrFill = (chartElement == CBCGPChartVisualObject::CE_MAIN_ELEMENT) ? 
		colors.m_pBrElementFillColor : colors.m_pBrMarkerFillColor;

	CBCGPBrush* pBrLine = (chartElement == CBCGPChartVisualObject::CE_MAIN_ELEMENT) ? 
		colors.m_pBrElementLineColor : colors.m_pBrMarkerLineColor;

	double dblLineWidth = (chartElement == CBCGPChartVisualObject::CE_MAIN_ELEMENT) ? 
		pFormatSeries->m_seriesElementFormat.m_outlineFormat.GetLineWidth(TRUE) :
		pFormatSeries->m_markerFormat.m_outlineFormat.GetLineWidth(TRUE);

	const CBCGPStrokeStyle& stroke = (chartElement == CBCGPChartVisualObject::CE_MAIN_ELEMENT) ?
		pFormatSeries->m_seriesElementFormat.m_outlineFormat.m_strokeStyle :
		pFormatSeries->m_markerFormat.m_outlineFormat.m_strokeStyle;

	OnDrawChartRect(pGM, rect, pBrFill, pBrLine, dblLineWidth, &stroke, bNoFill, bNoLine);
}
//****************************************************************************************
void CBCGPChartVisualObject::OnDrawChartSeriesItem(CBCGPGraphicsManager* pGM, const CBCGPEllipse& ellipse, 
												   CBCGPChartSeries* pSeries, int nDataPointIndex, 
												   CBCGPChartVisualObject::ChartElement chartElement,
												   BOOL bNoFill, BOOL bNoLine)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pGM);

	BCGPSeriesColorsPtr colors;
	const BCGPChartFormatSeries* pFormatSeries = pSeries->GetColors(colors, nDataPointIndex);
	if (pFormatSeries == NULL)
	{
		return;
	}

	CBCGPBrush* pBrFill = (chartElement == CBCGPChartVisualObject::CE_MAIN_ELEMENT) ? 
		colors.m_pBrElementFillColor : colors.m_pBrMarkerFillColor;

	CBCGPBrush* pBrLine = (chartElement == CBCGPChartVisualObject::CE_MAIN_ELEMENT) ? 
		colors.m_pBrElementLineColor : colors.m_pBrMarkerLineColor;

	double dblLineWidth = (chartElement == CBCGPChartVisualObject::CE_MAIN_ELEMENT) ? 
		pFormatSeries->m_seriesElementFormat.m_outlineFormat.GetLineWidth(TRUE) :
		pFormatSeries->m_markerFormat.m_outlineFormat.GetLineWidth(TRUE);

	const CBCGPStrokeStyle& stroke = (chartElement == CBCGPChartVisualObject::CE_MAIN_ELEMENT) ?
		pFormatSeries->m_seriesElementFormat.m_outlineFormat.m_strokeStyle :
		pFormatSeries->m_markerFormat.m_outlineFormat.m_strokeStyle;

	OnDrawChartEllipse(pGM, ellipse, pBrFill, pBrLine, dblLineWidth, stroke, bNoFill, bNoLine);
}
//****************************************************************************************
void CBCGPChartVisualObject::OnDrawChartSeriesItem(CBCGPGraphicsManager* pGM, const CBCGPGeometry& geometry, 
												   CBCGPChartSeries* pSeries, int nDataPointIndex, 
												   CBCGPChartVisualObject::ChartElement chartElement,
												   BOOL bNoFill, BOOL bNoLine)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pGM);

	BCGPSeriesColorsPtr colors;
	const BCGPChartFormatSeries* pFormatSeries = pSeries->GetColors(colors, nDataPointIndex);
	if (pFormatSeries == NULL)
	{
		return;
	}

	CBCGPBrush* pBrFill = (chartElement == CBCGPChartVisualObject::CE_MAIN_ELEMENT) ? 
		colors.m_pBrElementFillColor : colors.m_pBrMarkerFillColor;

	CBCGPBrush* pBrLine = (chartElement == CBCGPChartVisualObject::CE_MAIN_ELEMENT) ? 
		colors.m_pBrElementLineColor : colors.m_pBrMarkerLineColor;

	double dblLineWidth = (chartElement == CBCGPChartVisualObject::CE_MAIN_ELEMENT) ? 
		pFormatSeries->m_seriesElementFormat.m_outlineFormat.GetLineWidth(TRUE) :
		pFormatSeries->m_markerFormat.m_outlineFormat.GetLineWidth(TRUE);

	const CBCGPStrokeStyle& stroke = (chartElement == CBCGPChartVisualObject::CE_MAIN_ELEMENT) ?
		pFormatSeries->m_seriesElementFormat.m_outlineFormat.m_strokeStyle :
		pFormatSeries->m_markerFormat.m_outlineFormat.m_strokeStyle;

	OnDrawChartGeometry(pGM, geometry, pBrFill, pBrLine, dblLineWidth, stroke, bNoFill, bNoLine);
}
//****************************************************************************************
void CBCGPChartVisualObject::OnDrawChartSeriesDataLabel(CBCGPGraphicsManager* pGM, const CBCGPRect& rectText,
							const CString& strText, CBCGPChartSeries* pSeries, int nDataPointIndex)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pGM);

	if (m_bIsThumbnailMode)
	{
		return;
	}

	BCGPSeriesColorsPtr colors;
	const BCGPChartFormatSeries* pFormatSeries = pSeries->GetColors(colors, nDataPointIndex);
	if (pFormatSeries == NULL)
	{
		return;
	}

	const CBCGPChartDataPoint* pDP = pSeries->GetDataPointAt(nDataPointIndex);
	
	if (pDP == NULL)
	{
		return;
	}

	if (pDP->GetComponentValue().IsEmpty() && pSeries->GetTreatNulls() != CBCGPChartSeries::TN_VALUE)
	{
		return;
	}

	CBCGPPoint ptLabelLineStart;
	CBCGPPoint ptLabelLineEnd;
	CBCGPPoint ptLabelUnderline;

	BOOL bDropLineToMarker = pFormatSeries->m_dataLabelFormat.m_options.m_bDropLineToMarker;

	if (!IsChart3D())
	{
		if ((bDropLineToMarker || 
			pFormatSeries->m_dataLabelFormat.m_options.m_bUnderlineDataLabel) &&
			pSeries->GetDataPointLabelDropLines(nDataPointIndex, ptLabelLineStart, ptLabelLineEnd, ptLabelUnderline) && 
			colors.m_pBrDataLabelLineColor != NULL && ptLabelLineStart.x != 0 && ptLabelLineEnd.x != 0)
		{
			const BCGPChartFormatLine& lineFormat = pFormatSeries->m_dataLabelFormat.m_outlineFormat;

			if (pFormatSeries->m_dataLabelFormat.m_options.m_bUnderlineDataLabel && 
				!pFormatSeries->m_dataLabelFormat.m_options.m_bDrawDataLabelBorder || 
				bDropLineToMarker)
			{
				pGM->DrawLine(ptLabelLineStart, ptLabelLineEnd, *colors.m_pBrDataLabelLineColor,  
					lineFormat.GetLineWidth(TRUE), &lineFormat.m_strokeStyle);
			}
			
			if (pFormatSeries->m_dataLabelFormat.m_options.m_bUnderlineDataLabel && 
				!pFormatSeries->m_dataLabelFormat.m_options.m_bDrawDataLabelBorder)
			{
				pGM->DrawLine(ptLabelLineStart, ptLabelUnderline, *colors.m_pBrDataLabelLineColor,  
					lineFormat.GetLineWidth(TRUE), &lineFormat.m_strokeStyle);
			}
		}
	}

	if (colors.m_pBrDataLabelFillColor != NULL && colors.m_pBrDataLabelLineColor != NULL && 
		pFormatSeries->m_dataLabelFormat.m_options.m_bDrawDataLabelBorder)
	{
		OnDrawChartRect(pGM, rectText, colors.m_pBrDataLabelFillColor, colors.m_pBrDataLabelLineColor, 
			pFormatSeries->m_dataLabelFormat.m_outlineFormat.GetLineWidth(TRUE), 
			&pFormatSeries->m_dataLabelFormat.m_outlineFormat.m_strokeStyle, FALSE, 
			!pFormatSeries->m_dataLabelFormat.m_options.m_bDrawDataLabelBorder);
	}

	CBCGPRect rectLabelText = rectText;
	rectLabelText.DeflateRect(pFormatSeries->m_dataLabelFormat.GetContentPadding(TRUE));

	if (pFormatSeries->m_dataLabelFormat.m_options.m_bIncludeLegendKeyInLabel)
	{
		CBCGPSize szLegend = OnCalcLegendKeySize(pSeries, nDataPointIndex);
		CBCGPRect rectLegend = rectLabelText;
		
		rectLegend.right = rectLegend.left + szLegend.cx;

		rectLegend.top = rectText.top + rectText.Height() / 2 - szLegend.cy / 2;
		rectLegend.bottom = rectLegend.top + szLegend.cy;

		OnDrawChartLegendKey(pGM, rectLegend, pFormatSeries, pSeries, nDataPointIndex);

		rectLabelText.left = rectLegend.right + pFormatSeries->m_dataLabelFormat.m_options.GetKeyToLabelDistance();
	}

	if (colors.m_pBrDataLabelTextColor == NULL)
	{
		return;
	}

	if (pDP->IsUseWordWrapForDatalabels())
	{
		CBCGPTextFormat tf(pFormatSeries->m_dataLabelFormat.m_textFormat);
		tf.SetWordWrap(TRUE);

		pGM->DrawText(strText, rectLabelText, tf, *colors.m_pBrDataLabelTextColor);
	}
	else
	{
		pGM->DrawText(strText, rectLabelText, pFormatSeries->m_dataLabelFormat.m_textFormat, *colors.m_pBrDataLabelTextColor);
	}
}
//****************************************************************************************
void CBCGPChartVisualObject::OnDrawSelection(CBCGPGraphicsManager* pGM)
{
	if (m_bSelectionMode)
	{
		CBCGPRect rectSel(m_ptSelStart, m_ptSelEnd);
		rectSel.Normalize();
		OnDrawChartElement(pGM, rectSel, m_selectionFormat, CBCGPChartVisualObject::CE_SELECTION, FALSE, FALSE);
	}
}
//****************************************************************************************
void CBCGPChartVisualObject::OnFillAxisUnitInterval(CBCGPGraphicsManager* pGM, const CBCGPRect& rectInterval, 
													const CBCGPBrush& brFill)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pGM);

	pGM->FillRectangle(rectInterval, brFill);
}
//****************************************************************************************
void CBCGPChartVisualObject::OnFillAxisUnitInterval(CBCGPGraphicsManager* /*pGM*/, const CBCGPGeometry& gInterval, const CBCGPBrush& brFill)
{
	ASSERT_VALID(this);

	CBCGPEngine3D* pEngine3D = GetEngine3D();

	if (pEngine3D != NULL)
	{
		pEngine3D->FillGeometry(gInterval, brFill);
	}
}
//****************************************************************************************
CBCGPSize CBCGPChartVisualObject::OnGetTextSize(CBCGPGraphicsManager* pGM, const CString& strText, const CBCGPTextFormat& textFormat, double dblWidth)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pGM);

	return pGM->GetTextSize(strText, textFormat, dblWidth);
}
//****************************************************************************************
CBCGPSize CBCGPChartVisualObject::OnCalcAxisLabelSize(CBCGPGraphicsManager* pGM, CBCGPChartAxis* /*pAxis*/, double /*dblValue*/, CString& strLabel, 
		const CBCGPTextFormat& textFormat)
{
	return OnGetTextSize(pGM, strLabel, textFormat);
}
//****************************************************************************************
BOOL CBCGPChartVisualObject::OnCalcDataLabelSize(CBCGPGraphicsManager* /*pGM*/, const CString& /*strText*/, CBCGPChartSeries* /*pSeries*/, int /*nDataPointIndex*/, CBCGPSize& sz)
{
	sz.SetSizeEmpty();
	return FALSE;
}
//****************************************************************************************
void CBCGPChartVisualObject::OnDrawChartSeriesLine(CBCGPGraphicsManager* pGM, const CBCGPPoint& ptStart, const CBCGPPoint& ptEnd, 
										CBCGPChartSeries* pSeries, int nDataPointIndex)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pGM);

	BCGPSeriesColorsPtr colors;

	int nColorIndex = pSeries->IsColorEachLineEnabled() ? nDataPointIndex : -1;

	// take color from the first point, which is equal to series color
	pSeries->GetColors(colors, nColorIndex);
	const BCGPChartFormatSeries* pFormatSeries = pSeries->GetDataPointFormat(nDataPointIndex, FALSE);

	if (pFormatSeries == NULL)
	{
		return;
	}

	const CBCGPBrush& brLine = (!pFormatSeries->m_seriesElementFormat.m_outlineFormat.m_brLineColor.IsEmpty() || 
		colors.m_pBrElementLineColor == NULL) ? 
		pFormatSeries->m_seriesElementFormat.m_outlineFormat.m_brLineColor :
		*colors.m_pBrElementLineColor;
			

	if (pFormatSeries->m_curveType != BCGPChartFormatSeries::CCT_NO_LINE)
	{
		double dblLineWidth = IsChart3D() ? 
			pFormatSeries->m_dbl3DLineThickness : 
			pFormatSeries->m_seriesElementFormat.m_outlineFormat.GetLineWidth(TRUE);

		dblLineWidth = bcg_clamp(dblLineWidth, 1., 7. * max(m_sizeScaleRatio.cx, m_sizeScaleRatio.cy));

		pGM->DrawLine(ptStart, ptEnd, brLine, dblLineWidth, &pFormatSeries->m_seriesElementFormat.m_outlineFormat.m_strokeStyle);
	}
}
//****************************************************************************************
void CBCGPChartVisualObject::OnDrawChartSeriesMarker(CBCGPGraphicsManager* pGM, const CBCGPPoint& ptMarkerCenter, 
											   const BCGPChartFormatMarker& markerFormat, 
											   CBCGPChartSeries* pSeries, int nDataPointIndex, BOOL bAsMainItem)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pGM);

	int nMarkerSize = 0;
	CBCGPPoint ptCenter(0, 0);
	double dMarkerSize2 = 0;

	CalcMarkerDisplayParams(pGM, markerFormat.GetMarkerSize().cy, ptMarkerCenter, 
		markerFormat.m_outlineFormat.GetLineWidth(TRUE), nMarkerSize, dMarkerSize2, ptCenter);

	CBCGPChartVisualObject::ChartElement elementType = bAsMainItem ? CBCGPChartVisualObject::CE_MAIN_ELEMENT :
																		CBCGPChartVisualObject::CE_MARKER;

	switch(markerFormat.m_options.m_markerShape)
	{
	case BCGPChartMarkerOptions::MS_CIRCLE:
		{
			CBCGPEllipse ellipse(ptCenter, dMarkerSize2, dMarkerSize2);
			OnDrawChartSeriesItem(pGM, ellipse, pSeries, nDataPointIndex, elementType, FALSE, FALSE);
		}
		break;
	case BCGPChartMarkerOptions::MS_RECTANGLE:
		{
			CBCGPRect rect(CBCGPPoint(ptCenter.x - dMarkerSize2, ptCenter.y - dMarkerSize2), 
							CBCGPSize(nMarkerSize, nMarkerSize));
			OnDrawChartSeriesItem(pGM, rect, pSeries, nDataPointIndex, elementType, FALSE, FALSE);
		}
		break;
	case BCGPChartMarkerOptions::MS_RHOMBUS:
	case BCGPChartMarkerOptions::MS_TRIANGLE:
		{
			CBCGPPolygonGeometry geometry;
			CreateMarkerShape(ptCenter, nMarkerSize, dMarkerSize2, markerFormat.m_options.m_markerShape, geometry);
			OnDrawChartSeriesItem(pGM, geometry, pSeries, nDataPointIndex, elementType, FALSE, FALSE);
			break;
		}
	}
}
//****************************************************************************************
void CBCGPChartVisualObject::OnDrawChartSeriesMarkerEx(CBCGPGraphicsManager* pGM, const CBCGPPoint& ptMarkerCenter, 
								const CBCGPSize& szMarker, const CBCGPBrush& brFill, 
								const BCGPChartFormatLine& lineStyle, BCGPChartMarkerOptions::MarkerShape shape)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pGM);
	
	int nMarkerSize = 0;
	CBCGPPoint ptCenter(0, 0);
	double dMarkerSize2 = 0;
	
	CalcMarkerDisplayParams(pGM, szMarker.cy, ptMarkerCenter, 
		lineStyle.GetLineWidth(TRUE), nMarkerSize, dMarkerSize2, ptCenter);
	
	switch(shape)
	{
	case BCGPChartMarkerOptions::MS_CIRCLE:
		{
			CBCGPEllipse ellipse(ptCenter, dMarkerSize2, dMarkerSize2);
			OnDrawChartEllipse(pGM, ellipse, &brFill, &lineStyle.m_brLineColor, lineStyle.GetLineWidth(TRUE), 
				lineStyle.m_strokeStyle);
		}
		break;

	case BCGPChartMarkerOptions::MS_RECTANGLE:
		{
			CBCGPRect rect(CBCGPPoint(ptCenter.x - dMarkerSize2, ptCenter.y - dMarkerSize2), 
				CBCGPSize(nMarkerSize, nMarkerSize));
			OnDrawChartRect(pGM, rect, &brFill, &lineStyle.m_brLineColor, lineStyle.GetLineWidth(TRUE), 
				&lineStyle.m_strokeStyle);
		}
		break;

	case BCGPChartMarkerOptions::MS_RHOMBUS:
	case BCGPChartMarkerOptions::MS_TRIANGLE:
		{
			CBCGPPolygonGeometry geometry;
			CreateMarkerShape(ptCenter, nMarkerSize, dMarkerSize2, shape, geometry);
			OnDrawChartGeometry(pGM, geometry, &brFill, &lineStyle.m_brLineColor, lineStyle.GetLineWidth(TRUE), 
				lineStyle.m_strokeStyle);
			break;
		}
	}
}
//****************************************************************************************
void CBCGPChartVisualObject::CalcMarkerDisplayParams(CBCGPGraphicsManager* pGM, double dblFormattedSize, 
			const CBCGPPoint& ptMarkerCenter, double dblLineWidth, int& nMarkerSize, 
			double& dblMarkerSize2, CBCGPPoint& ptCenter)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pGM);
	
	nMarkerSize = (int)ceil(dblFormattedSize + dblLineWidth);
	
	if (nMarkerSize % 2 != 0)
	{
		nMarkerSize++;
	}
	
	ptCenter.SetPoint((long)ptMarkerCenter.x + 0.5, (long)ptMarkerCenter.y + 0.5);
	dblMarkerSize2 = bcg_round(nMarkerSize / 2.0);
	
	if (pGM->GetType() == CBCGPGraphicsManager::BCGP_GRAPHICS_MANAGER_GDI)
	{
		dblMarkerSize2 = nMarkerSize / 2;
		ptCenter = ptMarkerCenter;
	}
}
//****************************************************************************************
void CBCGPChartVisualObject::CreateMarkerShape(const CBCGPPoint& ptCenter, int nMarkerSize, 
					double dblMarkerSize2, BCGPChartMarkerOptions::MarkerShape shape, CBCGPPolygonGeometry& geometry)
{
	switch(shape)
	{
	case BCGPChartMarkerOptions::MS_RHOMBUS:
		{
			CBCGPPointsArray arPoints(4);
			
			arPoints[0] = CBCGPPoint(ptCenter.x, ptCenter.y - dblMarkerSize2);
			arPoints[1] = CBCGPPoint(ptCenter.x + dblMarkerSize2, ptCenter.y);
			arPoints[2] = CBCGPPoint(ptCenter.x, ptCenter.y + dblMarkerSize2);
			arPoints[3] = CBCGPPoint(ptCenter.x - dblMarkerSize2, ptCenter.y);
			
			geometry.SetPoints(arPoints);
		}
		break;

	case BCGPChartMarkerOptions::MS_TRIANGLE:
		{
			CBCGPPointsArray arPoints(3);
			
			double dblWidth = nMarkerSize * sin(bcg_deg2rad(30));
			CBCGPPoint ptTop(ptCenter.x, ptCenter.y - dblMarkerSize2);  
			CBCGPPoint ptLeft(ptCenter.x - dblWidth, ptTop.y + nMarkerSize);
			CBCGPPoint ptRight(ptLeft.x + dblWidth * 2.0, ptLeft.y);
			
			arPoints[0] = ptLeft;
			arPoints[1] = ptTop;
			arPoints[2] = ptRight;

			geometry.SetPoints(arPoints);
		}
		break;
	}
}
//****************************************************************************************
void CBCGPChartVisualObject::OnFillBackground (CBCGPGraphicsManager* /*pGM*/)
{
	ASSERT_VALID (this);
}
//******************************************************************************************
void CBCGPChartVisualObject::OnDrawChartArea (CBCGPGraphicsManager* pGM, const CBCGPRect& rectChartArea)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pGM);

	OnDrawChartElement(pGM, rectChartArea, m_chartAreaFormat, CBCGPChartVisualObject::CE_CHART_AREA, FALSE, FALSE);
}
//******************************************************************************************
void CBCGPChartVisualObject::OnDrawChartTitle(CBCGPGraphicsManager* pGM, CString& strChartTitle, CBCGPRect& rectChartTitle, 
									  BCGPChartFormatLabel& labelStyle)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pGM);

	if (!m_chartLayout.m_bShowChartTitle || m_bIsThumbnailMode)
	{
		return;
	}

	OnDrawChartElement(pGM, rectChartTitle, labelStyle, CBCGPChartVisualObject::CE_CHART_TITLE, FALSE, FALSE);

	const CBCGPBrush& brText = labelStyle.m_brTextColor.IsEmpty() ? m_currentTheme.m_brTitleTextColor : labelStyle.m_brTextColor;
	pGM->DrawText(strChartTitle, rectChartTitle, labelStyle.m_textFormat, brText);
}
//******************************************************************************************
void CBCGPChartVisualObject::OnDrawChartLegend(CBCGPGraphicsManager* pGM, CBCGPRect& rectLegend, BCGPChartFormatArea& legendStyle)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pGM);

	if (rectLegend.Width() < 2 || rectLegend.Height() < 2 || 
		m_chartLayout.m_legendPosition == BCGPChartLayout::LP_NONE || GetVisibleSeriesCount() == 0 ||
		m_bIsThumbnailMode)
	{
		return;
	}

	OnDrawChartElement(pGM, rectLegend, legendStyle, CBCGPChartVisualObject::CE_LEGEND, 
		!m_chartLayout.m_bDrawLegendShape, !m_chartLayout.m_bDrawLegendShape);

	rectLegend.DeflateRect(m_legendAreaFormat.GetContentPadding(TRUE));

	if (IsShowSurfaceMapInLegend())
	{
		for (int i = 0; i < m_arData.GetSize(); i++)
		{
			CBCGPChartSurfaceSeries* pSurfaceSeries = DYNAMIC_DOWNCAST(CBCGPChartSurfaceSeries, GetSeries(i, FALSE));

			if (pSurfaceSeries == NULL || !pSurfaceSeries->m_bVisible)
			{
				continue;
			}

			const BCGPChartFormatSeries* pFormat = &pSurfaceSeries->GetSeriesFormat();

			for (int j = 0; j < pSurfaceSeries->GetLegendElementCount(); j++) 
			{
				CBCGPRect rectKey; 
				CBCGPRect rectLabel;
				CString strLabel;

				pSurfaceSeries->GetLegendElementRects(j, rectKey, rectLabel);
				pSurfaceSeries->GetLegendElementLabel(j, strLabel);

				rectKey.OffsetRect(rectLegend.TopLeft());
				rectLabel.OffsetRect(rectLegend.TopLeft());

				OnDrawChartLegendKey(pGM, rectKey, pFormat, pSurfaceSeries, j);
				OnDrawChartLegendLabel(pGM, strLabel, rectLabel, pFormat->m_legendLabelFormat, pSurfaceSeries, -1);
			}

			break;
		}
	}
	else
	{
		for (int i = 0; i < m_arData.GetSize(); i++)
		{
			CBCGPChartSeries* pSeries = GetSeries(i, FALSE);

			if (pSeries == NULL || !pSeries->IsLegendEntryVisible())
			{
				continue;
			}

			if (pSeries->m_bIncludeDataPointLabelsToLegend)
			{
				for (int j = 0; j < pSeries->GetLegendElementCount(); j++)
				{
					CBCGPChartDataPoint* pDp = (CBCGPChartDataPoint*) pSeries->GetDataPointAt(j);
					if (pSeries->m_bIncludeDataPointLabelsToLegend && pSeries->CanIncludeDataPointToLegend(j))
					{
						OnDrawLegendEntry(pGM, rectLegend, pSeries, pDp, j);
					}
				}
			}
			else
			{
				OnDrawLegendEntry(pGM, rectLegend, pSeries, NULL, -1);
			}
		}
	}
}
//******************************************************************************************
void CBCGPChartVisualObject::OnDrawLegendEntry(CBCGPGraphicsManager* pGM, const CRect& rectLegend, 
									   CBCGPChartSeries* pSeries, CBCGPChartDataPoint* pDataPoint, int nDataPointIndex)
{
	CBCGPRect rectKey = pSeries->m_rectLegendKey;
	CBCGPRect rectLabel = pSeries->m_rectLegendLabel;
	CString strLabel = pSeries->m_strSeriesName;
	const BCGPChartFormatSeries* pFormat = &pSeries->GetSeriesFormat();

	if (pDataPoint != NULL && !pDataPoint->m_rectLegendKey.IsRectNull() && pDataPoint->m_bIncludeLabelToLegend)
	{
		rectKey = pDataPoint->m_rectLegendKey;
		rectLabel = pDataPoint->m_rectLegendLabel;

		pSeries->OnGetDataPointLegendLabel(nDataPointIndex, strLabel);
		pFormat = pSeries->GetDataPointFormat(nDataPointIndex, FALSE);
	}

	rectKey.OffsetRect(rectLegend.TopLeft());
	rectLabel.OffsetRect(rectLegend.TopLeft());

	OnDrawChartLegendKey(pGM, rectKey, pFormat, pSeries, nDataPointIndex);
	OnDrawChartLegendLabel(pGM, strLabel, rectLabel, pFormat->m_legendLabelFormat, pSeries, nDataPointIndex);
}
//******************************************************************************************
void CBCGPChartVisualObject::OnDrawPlotArea (CBCGPGraphicsManager* pGM, const CBCGPRect& rectPlotArea, const CBCGPRect& /*rectDiagramArea*/)
{
	ASSERT_VALID(this);

	if (rectPlotArea.Height() < 2 || rectPlotArea.Width() < 2)
	{
		return;
	}

	CBCGPRect rect;
	if (NeedDisplayAxes() && GetChartCategory() != BCGPChartTernary && !IsChart3D())
	{
		for (int i = 0; i < m_arAxes.GetSize(); i++)
		{
			CBCGPChartAxis* pAxis = m_arAxes[i];
			if (pAxis == NULL)
			{
				continue;
			}

			rect.UnionRect(pAxis->GetBoundingRect());
		}
	}
	else
	{
		rect = rectPlotArea;
	}

	OnDrawChartElement(pGM, rect, m_plotAreaFormat, CBCGPChartVisualObject::CE_PLOT_AREA, FALSE, TRUE);

	BOOL bNeedReleaseClipArea = FALSE;
	CBCGPRect rectNormal = m_rectPlotArea;
	rectNormal.Normalize();
	rectNormal.bottom++;

	BOOL bIsChart3D = IsChart3D();
	CBCGPChartDiagram3D* pDiagram3D = GetDiagram3D();

	if (bIsChart3D && pDiagram3D != NULL)
	{
		bNeedReleaseClipArea = TRUE;
		pGM->SetClipRect(rectNormal);

		pDiagram3D->OnDraw(pGM, TRUE, FALSE);
	}

	if (!bIsChart3D || bIsChart3D && !pDiagram3D->IsThickWallsAndFloor())
	{
		OnInterlaceAxesIntervals(pGM, m_rectDiagramArea);
	}

	if (bIsChart3D && pDiagram3D != NULL)
	{
		pDiagram3D->OnDraw(pGM, FALSE, TRUE);
	}

	if (bNeedReleaseClipArea)
	{
		pGM->ReleaseClipArea();
	}

	OnDrawChartElement(pGM, rect, m_plotAreaFormat, CBCGPChartVisualObject::CE_PLOT_AREA, TRUE, FALSE);
}
//******************************************************************************************
void CBCGPChartVisualObject::OnDrawPlotAreaItems (CBCGPGraphicsManager* pGM, const CBCGPRect& /*rectPlotArea*/, const CBCGPRect& rectDiagramArea)
{
	CBCGPChartDiagram3D* pDiagram3D = GetDiagram3D();

	if (IsChart3D() && pDiagram3D != NULL)
	{
		CBCGPRect rectNormal = m_rectPlotArea;
		rectNormal.Normalize();

		pGM->SetClipRect(rectNormal);

		BOOL bDispayAxes = NeedDisplayAxes();

		CBCGPChartAxis* pXAxis = GetChartAxis(BCGP_CHART_X_PRIMARY_AXIS);
		CBCGPChartAxis* pYAxis = GetChartAxis(BCGP_CHART_Y_PRIMARY_AXIS);
		CBCGPChartAxis* pZAxis = GetChartAxis(BCGP_CHART_Z_PRIMARY_AXIS);

		CBCGPChartAxis* pHorzAxis = pXAxis->IsVertical() ? pYAxis : pXAxis; 
		CBCGPChartAxis* pVertAxis = pXAxis->IsVertical() ? pXAxis : pYAxis; 

		BOOL bVertAxisBehind = FALSE;

		CBCGPPoint ptVertStart;
		CBCGPPoint ptVertEnd;

		CBCGPPoint ptHorzStart;
		CBCGPPoint ptHorzEnd;

		pDiagram3D->GetNormalAxisCoordinates(pVertAxis->m_nAxisID, ptVertStart, ptVertEnd);
		pDiagram3D->GetNormalAxisCoordinates(pHorzAxis->m_nAxisID, ptHorzStart, ptHorzEnd);

		if (ptVertStart.z != ptHorzStart.z)
		{
			bVertAxisBehind = TRUE;
		}

		OnDrawAxesGridLines(pGM, rectDiagramArea);
	
		if (bDispayAxes && !pDiagram3D->IsThickWallsAndFloor())
		{
			if (bVertAxisBehind)
			{
				pVertAxis->OnDraw(pGM, rectDiagramArea);
			}

			pZAxis->OnDraw(pGM, rectDiagramArea);
		}

		OnDrawDiagram(pGM, rectDiagramArea);
		
		if (bDispayAxes && !pDiagram3D->IsThickWallsAndFloor())
		{
			if (!bVertAxisBehind)
			{
				pVertAxis->OnDraw(pGM, rectDiagramArea);
			}

			pHorzAxis->OnDraw(pGM, rectDiagramArea);
		}
		
		CBCGPRect rectTarget = m_rectPlotArea;
		rectTarget.left++;
		rectTarget.top++;
		rectTarget.right--;

		pDiagram3D->OnEnd3DDraw(rectTarget);

		if (bDispayAxes)
		{
			pVertAxis->OnDrawAxisLabels(pGM, rectDiagramArea);
			pHorzAxis->OnDrawAxisLabels(pGM, rectDiagramArea);
			pVertAxis->OnDrawAxisLabels(pGM, rectDiagramArea);
			pZAxis->OnDrawAxisLabels(pGM, rectDiagramArea);
		}

		pGM->ReleaseClipArea();

		if (bDispayAxes)
		{
			OnDrawAxisName(pGM, pHorzAxis);
			OnDrawAxisName(pGM, pVertAxis);
			OnDrawAxisName(pGM, pZAxis);
		}
		
		OnDrawDiagramDataLabels(pGM, rectDiagramArea);
		
		return;
	}

	OnDrawAxesGridLines(pGM, rectDiagramArea);
	OnDrawAxes(pGM, rectDiagramArea);

	OnDrawChartObjects(pGM, FALSE);

	CBCGPRect rectNormal = rectDiagramArea;
	rectNormal.Normalize();
	rectNormal.bottom++;

	m_DrawOrder = CBCGPChartVisualObject::CDO_BACKGROUND;

	int i = 0;
	for (i = 0; i < 2; i++)
	{
		pGM->SetClipRect(rectNormal);
		OnDrawDiagram(pGM, rectDiagramArea);
		pGM->ReleaseClipArea();

		if (!m_bIsThumbnailMode || (m_nThumbnailFlags & BCGP_CHART_THUMBNAIL_DRAW_MARKERS) == BCGP_CHART_THUMBNAIL_DRAW_MARKERS)
		{
			OnDrawDiagramMarkers(pGM, rectDiagramArea);
		}

		if (m_DrawOrder == CBCGPChartVisualObject::CDO_IGNORE)
		{
			break;
		}

		m_DrawOrder = CBCGPChartVisualObject::CDO_NORMAL;
	}

	m_DrawOrder = CBCGPChartVisualObject::CDO_IGNORE;

	OnDrawAxisScaleBreaks(pGM, rectDiagramArea);

	if (!m_bIsThumbnailMode)
	{
		OnDrawDiagramDataLabels(pGM, rectDiagramArea);
	}
	
	OnDrawAxesLabels(pGM, rectDiagramArea);

	for (i = 0; i < m_arAxes.GetSize(); i++)
	{
		CBCGPChartAxis* pAxis = m_arAxes[i];

		if (pAxis == NULL)
		{
			continue;
		}

		ASSERT_VALID(pAxis);

		OnDrawAxisName(pGM, pAxis);
		pAxis->OnDrawScrollBar(pGM);
	}
}
//******************************************************************************************
void CBCGPChartVisualObject::OnDrawAxisScaleBreaks(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagramArea)
{
	if (!NeedDisplayAxes())
	{
		return;
	}

	for (int i = 0; i < m_arAxes.GetSize(); i++)
	{
		CBCGPChartAxis* pAxis = m_arAxes[i];
		
		if (pAxis == NULL)
		{
			continue;
		}
		
		ASSERT_VALID(pAxis);
		
		pAxis->OnDrawScaleBreaks(pGM, rectDiagramArea);
	}
}
//******************************************************************************************
void CBCGPChartVisualObject::OnDrawAxisScaleBreak(CBCGPGraphicsManager* pGM, CBCGPRect rectScaleBreak, 
													CBCGPChartAxisScaleBreak::AxisScaleBreakStyle scaleBreakStyle, 
													const CBCGPBrush& brFill, const CBCGPBrush& brLine, double dblLineWidth,
													const CBCGPStrokeStyle& strokeStyle, double /*dblScaleBreakStartValue*/, 
													double /*dblScaleBreakEndValue*/, CBCGPChartAxis* pAxis, 
													const CBCGPRect& rectDiagram)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pGM);
	ASSERT_VALID(pAxis);

	if (pAxis == NULL || pGM == NULL)
	{
		return;
	}

	if (rectScaleBreak.top < rectDiagram.top)
	{
		rectScaleBreak.top = rectDiagram.top;

		if (rectScaleBreak.bottom <= rectScaleBreak.top)
		{
			return;
		}
	}

	CBCGPRect rectChart;
	
	CBCGPRect rectDraw = rectScaleBreak;

	if (scaleBreakStyle == CBCGPChartAxisScaleBreak::ASBS_SAWTOOTH || 
		scaleBreakStyle == CBCGPChartAxisScaleBreak::ASBS_WAVE)
	{
		if (pAxis->IsVertical() && rectScaleBreak.Height() <= 3 || !pAxis->IsVertical() && rectScaleBreak.Width() <= 3)
		{
			scaleBreakStyle = CBCGPChartAxisScaleBreak::ASBS_LINE;
		}
	}

	switch (scaleBreakStyle)
	{
	case CBCGPChartAxisScaleBreak::ASBS_CONTINUOUS:
		return;

	case CBCGPChartAxisScaleBreak::ASBS_EMPTY:
		rectDraw.InflateRect(-1., 1.);
		if (!rectDraw.IsRectEmpty())
		{
			pGM->SetClipRect(rectDraw);
			pGM->FillRectangle(m_rect, brFill);
			pGM->ReleaseClipArea();
		}

		rectDraw = rectScaleBreak;
		rectDraw.InflateRect(1., 0.);

		if (!rectDraw.IsRectEmpty())
		{
			pGM->SetClipRect(rectDraw);
			pGM->FillRectangle(m_rect, brFill);
			pGM->ReleaseClipArea();
		}
		
		break;

	case CBCGPChartAxisScaleBreak::ASBS_LINE:
		if (pAxis->IsVertical())
		{
			rectDraw.InflateRect(2., 0.);

			if (!rectDraw.IsRectEmpty())
			{
				pGM->SetClipRect(rectDraw);
				pGM->FillRectangle(m_rect, brFill);
				pGM->ReleaseClipArea();
			}
			
			rectDraw = rectScaleBreak;
			rectDraw.InflateRect(0.5, 0.);
			pGM->DrawLine(rectDraw.TopLeft(), rectDraw.TopRight(), brLine, dblLineWidth, &strokeStyle);
			pGM->DrawLine(rectDraw.BottomLeft(), rectDraw.BottomRight(), brLine, dblLineWidth, &strokeStyle);
		}
		else
		{
			rectDraw.InflateRect(0, 2.);

			if (!rectDraw.IsRectEmpty())
			{
				pGM->SetClipRect(rectDraw);
				pGM->FillRectangle(m_rect, brFill);
				pGM->ReleaseClipArea();
			}

			rectDraw = rectScaleBreak;
			rectDraw.InflateRect(0., 0.5);
			pGM->DrawLine(rectDraw.TopLeft(), rectDraw.BottomLeft(), brLine, dblLineWidth, &strokeStyle);
			pGM->DrawLine(rectDraw.TopRight(), rectDraw.BottomRight(), brLine, dblLineWidth, &strokeStyle);
		}
		break;
	
	case CBCGPChartAxisScaleBreak::ASBS_BOX:
		if (!rectDraw.IsRectEmpty())
		{
			pGM->SetClipRect(rectDraw);
			pGM->FillRectangle(m_rect, brFill);
			pGM->ReleaseClipArea();
		}

		pGM->DrawRectangle(rectScaleBreak, brLine, dblLineWidth, &strokeStyle);
		break;

	case CBCGPChartAxisScaleBreak::ASBS_SAWTOOTH:
	case CBCGPChartAxisScaleBreak::ASBS_WAVE:
		{
			CBCGPComplexGeometry complex;
			CBCGPPointsArray arPointsTop;
			CBCGPPointsArray arPointsBottom;
			double dblStepMod = scaleBreakStyle == CBCGPChartAxisScaleBreak::ASBS_WAVE ? 8 : 2;

			CBCGPPoint ptAxistStart;
			CBCGPPoint ptAxisEnd;
			pAxis->GetAxisPos(ptAxistStart, ptAxisEnd);
			BOOL bStartOpposite = FALSE;

			if (pAxis->IsVertical())
			{
				bStartOpposite = ptAxistStart.x == rectScaleBreak.right;
				rectDraw.InflateRect(0, dblLineWidth);

				double dblWaveHeight = (rectScaleBreak.Height() / 2 - 1);
				double dblStep = dblStepMod * dblWaveHeight;  
				rectDraw.bottom += dblWaveHeight;
			
				double x;

				if (bStartOpposite)
				{
					for (x = rectScaleBreak.right + 1; x >= rectScaleBreak.left - dblStep; x -= dblStep)
					{
						arPointsTop.Add(CBCGPPoint(x, rectScaleBreak.top));
						arPointsTop.Add(CBCGPPoint(x - dblStep / 2, rectScaleBreak.top + dblWaveHeight));
					}
					
					for (x = rectScaleBreak.right + 1; x >= rectScaleBreak.left - dblStep; x -= dblStep)
					{
						arPointsBottom.InsertAt(0, CBCGPPoint(x, rectScaleBreak.bottom - 1));
						arPointsBottom.InsertAt(0, CBCGPPoint(x - dblStep / 2, rectScaleBreak.bottom + dblWaveHeight - 1));
					}
				}
				else
				{
					for (x = rectScaleBreak.left - 1; x <= rectScaleBreak.right + dblStep; x += dblStep)
					{
						arPointsTop.Add(CBCGPPoint(x, rectScaleBreak.top));
						arPointsTop.Add(CBCGPPoint(x + dblStep / 2, rectScaleBreak.top + dblWaveHeight));
					}
					
					for (x = rectScaleBreak.left - 1; x <= rectScaleBreak.right + dblStep; x += dblStep)
					{
						arPointsBottom.InsertAt(0, CBCGPPoint(x, rectScaleBreak.bottom - 1));
						arPointsBottom.InsertAt(0, CBCGPPoint(x + dblStep / 2, rectScaleBreak.bottom + dblWaveHeight - 1));
					}
				}
				
			}
			else
			{
				bStartOpposite = ptAxistStart.y == rectScaleBreak.bottom;

				rectDraw.InflateRect(dblLineWidth, 0);
				
				double dblWaveWidth = (rectScaleBreak.Width() / 2 - 1);
				double dblStep = dblStepMod * dblWaveWidth; 

				rectDraw.right += dblWaveWidth;
				
				double y;

				if (bStartOpposite)
				{
					for (y = rectScaleBreak.bottom + 1; y >= rectScaleBreak.top - dblStep; y -= dblStep)
					{
						arPointsTop.Add(CBCGPPoint(rectScaleBreak.left, y));
						arPointsTop.Add(CBCGPPoint(rectScaleBreak.left + dblWaveWidth, y - dblStep / 2));
					}
					
					for (y = rectScaleBreak.bottom + 1; y >= rectScaleBreak.top - dblStep; y -= dblStep)
					{
						arPointsBottom.InsertAt(0, CBCGPPoint(rectScaleBreak.right, y));
						arPointsBottom.InsertAt(0, CBCGPPoint(rectScaleBreak.right + dblWaveWidth - 1.5, y - dblStep / 2));
					}
				}
				else
				{
					for (y = rectScaleBreak.top - 1; y <= rectScaleBreak.bottom + dblStep; y += dblStep)
					{
						arPointsTop.Add(CBCGPPoint(rectScaleBreak.left, y));
						arPointsTop.Add(CBCGPPoint(rectScaleBreak.left + dblWaveWidth, y + dblStep / 2));
					}
					
					for (y = rectScaleBreak.top - 1; y <= rectScaleBreak.bottom + dblStep; y += dblStep)
					{
						arPointsBottom.InsertAt(0, CBCGPPoint(rectScaleBreak.right, y));
						arPointsBottom.InsertAt(0, CBCGPPoint(rectScaleBreak.right + dblWaveWidth - 1.5, y + dblStep / 2));
					}
				}
			}

			if (scaleBreakStyle == CBCGPChartAxisScaleBreak::ASBS_WAVE)
			{

				CBCGPSplineGeometry gTop(arPointsTop, CBCGPSplineGeometry::BCGP_SPLINE_TYPE_KB, FALSE);
				CBCGPSplineGeometry gBottom(arPointsBottom, CBCGPSplineGeometry::BCGP_SPLINE_TYPE_KB, FALSE);
				
				complex.AddPoints(gTop.GetPoints(), CBCGPPolygonGeometry::BCGP_CURVE_TYPE_BEZIER);
				complex.AddPoints(gBottom.GetPoints(), CBCGPPolygonGeometry::BCGP_CURVE_TYPE_BEZIER);
			}
			else
			{
				complex.AddPoints(arPointsTop, CBCGPPolygonGeometry::BCGP_CURVE_TYPE_LINE);
				complex.AddPoints(arPointsBottom, CBCGPPolygonGeometry::BCGP_CURVE_TYPE_LINE);
			}


			pGM->SetClipArea(complex);
			pGM->FillRectangle(m_rect, brFill);
			pGM->ReleaseClipArea();

			pGM->SetClipRect(rectDraw);
			pGM->DrawGeometry(complex, brLine, dblLineWidth, &strokeStyle);
			pGM->ReleaseClipArea();
		}
		break;
	}
}
//******************************************************************************************
void CBCGPChartVisualObject::OnDrawAxesLabels(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagramArea)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pGM);

	if (!NeedDisplayAxes())
	{
		return;
	}

	for (int i = 0; i < m_arAxes.GetSize(); i++)
	{
		CBCGPChartAxis* pAxis = m_arAxes[i];

		if (pAxis == NULL)
		{
			continue;
		}

		ASSERT_VALID(pAxis);

		pAxis->OnDrawAxisLabels(pGM, rectDiagramArea);
	}
}
//******************************************************************************************
void CBCGPChartVisualObject::OnInterlaceAxesIntervals(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagramArea)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pGM);

	if (!NeedDisplayAxes())
	{
		return;
	}

	for (int i = 0; i < m_arAxes.GetSize(); i++)
	{
		CBCGPChartAxis* pAxis = m_arAxes[i];

		if (pAxis == NULL)
		{
			continue;
		}

		ASSERT_VALID(pAxis);

		if (pAxis->IsMajorUnitIntervalInterlacingEnabled())
		{
			pAxis->OnFillUnitIntervals(pGM, rectDiagramArea);
		}	
	}
}
//******************************************************************************************
void CBCGPChartVisualObject::OnDrawAxes(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagramArea)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pGM);

	if (!NeedDisplayAxes())
	{
		return;
	}

	for (int i = 0; i < m_arAxes.GetSize(); i++)
	{
		CBCGPChartAxis* pAxis = m_arAxes[i];

		if (pAxis == NULL)
		{
			continue;
		}

		ASSERT_VALID(pAxis);

		pAxis->OnDraw(pGM, rectDiagramArea);
	}
}
//******************************************************************************************
void CBCGPChartVisualObject::OnDrawAxesGridLines(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagramArea)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pGM);

	if (!NeedDisplayAxes())
	{
		return;
	}

	int i = 0;	
	for (i = 0; i < m_arAxes.GetSize(); i++)
	{
		CBCGPChartAxis* pAxis = m_arAxes[i];

		if (pAxis == NULL)
		{
			continue;
		}

		ASSERT_VALID(pAxis);

		pAxis->OnDrawMinorGridLines(pGM, rectDiagramArea);
	}

	for (i = 0; i < m_arAxes.GetSize(); i++)
	{
		CBCGPChartAxis* pAxis = m_arAxes[i];

		if (pAxis == NULL)
		{
			continue;
		}

		ASSERT_VALID(pAxis);

		pAxis->OnDrawMajorGridLines(pGM, rectDiagramArea);
	}

	for (i = 0; i < m_arAxes.GetSize(); i++)
	{
		CBCGPChartAxis* pAxis = m_arAxes[i];

		if (pAxis == NULL || !pAxis->IsAxisVisible())
		{
			continue;
		}

		ASSERT_VALID(pAxis);

		double dblTopOffset = pAxis->GetTopOffset();
		double dblBottomOffset = pAxis->GetBottomOffset();

		if (dblTopOffset == 0 && dblBottomOffset == 0)
		{
			continue;
		}

		CBCGPRect rect = pAxis->GetBoundingRect();
		OnDrawAxisBounds(pGM, rect);
	}
}
//******************************************************************************************
void CBCGPChartVisualObject::OnDrawAxisBounds(CBCGPGraphicsManager* pGM, const CBCGPRect& rectBounds)
{
	OnDrawChartElement(pGM, rectBounds, m_plotAreaFormat, CBCGPChartVisualObject::CE_PLOT_AREA, TRUE, FALSE);
}
//******************************************************************************************
void CBCGPChartVisualObject::OnDrawAxisName(CBCGPGraphicsManager* pGM, CBCGPChartAxis* pAxis)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pGM);

	if (!NeedDisplayAxes() || !pAxis->IsAxisVisible() || !pAxis->m_bDisplayAxisName || m_bIsThumbnailMode)
	{
		return;
	}

	if (!pAxis->IsVertical() && !m_rectDataTableArea.IsRectEmpty())
	{
		return;
	}

	if (IsChart3D())
	{
		pAxis->OnDrawAxisName3D(pGM);
		return;
	}

	CBCGPRect rectAxisName;
	OnGetAxisNameAreaRect(pAxis, rectAxisName);

	if (!rectAxisName.IsRectNull())
	{
		OnDrawChartElement(pGM, rectAxisName, pAxis->m_axisNameFormat, CBCGPChartVisualObject::CE_AXIS_NAME, FALSE, FALSE);

		const CBCGPBrush& brText = pAxis->m_axisNameFormat.m_brTextColor.IsEmpty() ? m_currentTheme.m_brAxisNameTextColor :
														pAxis->m_axisNameFormat.m_brTextColor;
		pGM->DrawText(pAxis->m_strAxisName, rectAxisName, pAxis->m_axisNameFormat.m_textFormat, brText);
	}
}
//******************************************************************************************
void CBCGPChartVisualObject::OnDrawGridLine(CBCGPGraphicsManager* /*pGM*/, CBCGPPoint& ptGridLineStart, CBCGPPoint& ptGridLineEnd, 
						 CBCGPChartAxis* pAxis, double dblCurrValue, const BCGPChartFormatLine& gridLineStyle, 
						 BOOL bIsMajor)
{
	ASSERT_VALID(this);

	UNREFERENCED_PARAMETER(pAxis);
	UNREFERENCED_PARAMETER(dblCurrValue);

	CBCGPEngine3D* pEngine3D = GetEngine3D();

	if (pEngine3D != NULL)
	{
		const CBCGPBrush& brLine = gridLineStyle.m_brLineColor.IsEmpty() ? 
			(bIsMajor ? m_currentTheme.m_brAxisMajorGridLineColor : 
			m_currentTheme.m_brAxisMinorGridLineColor) : gridLineStyle.m_brLineColor;

		pEngine3D->DrawLine(ptGridLineStart, ptGridLineEnd, brLine, gridLineStyle.GetLineWidth(TRUE), 
							&gridLineStyle.m_strokeStyle);
	}
}
//******************************************************************************************
void CBCGPChartVisualObject::OnDrawGridEllipse(CBCGPGraphicsManager* pGM, const CBCGPEllipse& ellipse, 
							CBCGPChartAxis* /*pAxis*/, double /*dblCurrValue*/, const BCGPChartFormatLine& gridLineStyle, 
							BOOL bIsMajor)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pGM);

	const CBCGPBrush& brLine = gridLineStyle.m_brLineColor.IsEmpty() ? 
		(bIsMajor ? m_currentTheme.m_brAxisMajorGridLineColor : 
		m_currentTheme.m_brAxisMinorGridLineColor) : gridLineStyle.m_brLineColor;

	pGM->DrawEllipse(ellipse, brLine, gridLineStyle.GetLineWidth(TRUE), &gridLineStyle.m_strokeStyle);
}
//******************************************************************************************
void CBCGPChartVisualObject::OnDrawDiagram(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagramArea)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pGM);

	int i = 0;
	
	BOOL bWasTransparency = CBCGPGraphicsManagerGDI::IsTransparencyEnabled();
	CBCGPGraphicsManagerGDI::EnableTransparency();

	OnCalcChartEffectsScreenPositions(pGM);
	OnDrawChartEffects(pGM);

	if (IsChart3D())
	{
		for (i = 0; i < m_arData.GetSize(); i++)
		{
			CBCGPChartSeries* pSeries = GetSeries(i, FALSE);
			if (pSeries == NULL || !pSeries->m_bVisible)
			{
				continue;
			}

			CBCGPBaseChartImpl* pChartImpl = pSeries->GetChartImpl();

			if (pChartImpl == NULL)
			{
				continue;
			}

			// DRAWS all data points from all series according to Z order,
			// therefore it's called only once with pSeries = NULL
			pChartImpl->OnDrawDiagram(pGM, rectDiagramArea, NULL);
			break;
		}
	}
	else
	{
		ChartDrawOrder drawOrder = m_DrawOrder;
		if (drawOrder == CBCGPChartVisualObject::CDO_IGNORE)
		{
			drawOrder = CBCGPChartVisualObject::CDO_BACKGROUND;
		}

		for (int j = 0; j < 2; j++)
		{
			for (i = 0; i < m_arData.GetSize(); i++)
			{
				CBCGPChartSeries* pSeries = GetSeries(i, FALSE);
				if (pSeries == NULL || !pSeries->m_bVisible)
				{
					continue;
				}

				if ((drawOrder == CBCGPChartVisualObject::CDO_BACKGROUND && !pSeries->IsBackgroundOrder()) || 
					(drawOrder == CBCGPChartVisualObject::CDO_NORMAL && pSeries->IsBackgroundOrder()))
				{
					continue;
				}

				CBCGPBaseChartImpl* pChartImpl = pSeries->GetChartImpl();

				if (pChartImpl == NULL)
				{
					continue;
				}

				CBCGPRect rectClip;

				if (m_bClipDiagramToAxes && GetChartCategory() != BCGPChartSurface3D)
				{
					rectClip = pSeries->GetAxesBoundingRect();
					if (!rectClip.IsRectEmpty())
					{
						if (!pSeries->GetRelatedAxis(CBCGPChartSeries::AI_X)->IsZoomed() && 
							!pSeries->GetRelatedAxis(CBCGPChartSeries::AI_Y)->IsZoomed())
						{
							rectClip.InflateRect(1., 1., 1., 1.);
						}

						pGM->SetClipRect(rectClip);
					}
				}

				if (pSeries->IsDisplayShadow() && !m_bIsThumbnailMode && !IsChart3D())
				{
					pGM->SetDrawShadowMode(TRUE, 
						pSeries->GetSeriesFormat().m_shadowType.m_color,
						pSeries->GetShadowTransparencyPercent(),
						pSeries->GetShadowAngle(),
						(int)(GetScaleRatioMid() * pSeries->GetSeriesFormat().m_shadowType.m_nDistance));

					pChartImpl->OnDrawDiagram(pGM, rectDiagramArea, pSeries);

					pGM->SetDrawShadowMode(FALSE);
				}

				pChartImpl->OnDrawDiagram(pGM, rectDiagramArea, pSeries);

				if (m_bClipDiagramToAxes && !rectClip.IsRectEmpty())
				{
					pGM->ReleaseClipArea();
				}
			}

			if (m_DrawOrder != CBCGPChartVisualObject::CDO_IGNORE)
			{
				break;
			}

			drawOrder = CBCGPChartVisualObject::CDO_NORMAL;
		}
	}

	CBCGPGraphicsManagerGDI::EnableTransparency(bWasTransparency);
}
//******************************************************************************************
void CBCGPChartVisualObject::OnDrawDiagramMarkers(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagram)
{
	for (int i = 0; i < m_arData.GetSize(); i++)
	{
		CBCGPChartSeries* pSeries = GetSeries(i, FALSE);
		if (pSeries == NULL || !pSeries->m_bVisible)
		{
			continue;
		}

		if ((m_DrawOrder == CBCGPChartVisualObject::CDO_BACKGROUND && !pSeries->IsBackgroundOrder()) || 
			(m_DrawOrder == CBCGPChartVisualObject::CDO_NORMAL && pSeries->IsBackgroundOrder()))
		{
			continue;
		}

		CBCGPBaseChartImpl* pChartImpl = pSeries->GetChartImpl();
		if (pChartImpl != NULL)
		{
			if (pSeries->IsShownOnCustomOrResizedAxis() && !IsChart3D())
			{
				CBCGPRect rectBounds = pSeries->GetAxesBoundingRect();
				pGM->SetClipRect(rectBounds);
			}
			else if (IsChart3D())
			{
				CBCGPRect rectBounds = m_rectPlotArea;
				rectBounds.InflateRect(10, 10);
				pGM->SetClipRect(rectBounds);
			}

			if (pSeries->GetSeriesFormat().m_shadowType.m_bDisplayShadow && !m_bIsThumbnailMode)
			{
				pGM->SetDrawShadowMode(TRUE, 
					pSeries->GetSeriesFormat().m_shadowType.m_color,
					pSeries->GetShadowTransparencyPercent(),
					pSeries->GetShadowAngle(),
					(int)(GetScaleRatioMid() * pSeries->GetSeriesFormat().m_shadowType.m_nDistance));

				pChartImpl->OnDrawDiagramMarkers(pGM, pSeries, rectDiagram);

				pGM->SetDrawShadowMode(FALSE);
			}

			pChartImpl->OnDrawDiagramMarkers(pGM, pSeries, rectDiagram);

			if (pSeries->IsShownOnCustomOrResizedAxis() || IsChart3D())
			{
				pGM->ReleaseClipArea();
			}
		}
	}
}
//******************************************************************************************
void CBCGPChartVisualObject::OnDrawDiagramDataLabels(CBCGPGraphicsManager* pGM, const CBCGPRect& /*rectDiagram*/)
{
	for (int i = 0; i < m_arData.GetSize(); i++)
	{
		CBCGPChartSeries* pSeries = GetSeries(i, FALSE);
		if (pSeries == NULL || !pSeries->m_bVisible)
		{
			continue;
		}

		CBCGPBaseChartImpl* pChartImpl = pSeries->GetChartImpl();
		if (pChartImpl != NULL)
		{
			if (pSeries->IsShownOnCustomOrResizedAxis() && !IsChart3D())
			{
				CBCGPRect rectBounds = pSeries->GetAxesBoundingRect();
				pGM->SetClipRect(rectBounds);
			}
			else if (IsChart3D())
			{
				CBCGPRect rectBounds = m_rectPlotArea;
				rectBounds.InflateRect(10, 10);
				pGM->SetClipRect(rectBounds);
			}

			pChartImpl->OnDrawDiagramDataLabels(pGM, pSeries);

			if (pSeries->IsShownOnCustomOrResizedAxis() || IsChart3D())
			{
				pGM->ReleaseClipArea();
			}
		}
	}
}
//******************************************************************************************
void CBCGPChartVisualObject::OnDrawChartLegendKey(CBCGPGraphicsManager* pGM, const CBCGPRect& rectLegendKey, 
									  const BCGPChartFormatSeries* pSeriesStyle, CBCGPChartSeries* pSeries, int nDataPointIndex)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pGM);
	ASSERT_VALID(pSeries);

	CBCGPBaseChartImpl* pChartImpl = pSeries->GetChartImpl();

	if (pChartImpl != NULL)
	{
		pChartImpl->OnDrawChartLegendKey(pGM, rectLegendKey, pSeriesStyle, pSeries, nDataPointIndex);
	}
}
//******************************************************************************************
void CBCGPChartVisualObject::OnDrawChartLegendKeyEx(CBCGPGraphicsManager* pGM, const CBCGPRect& rectLegendKey, 
		CBCGPChartLegendCell* pCell)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pGM);
	ASSERT_VALID(pCell);

	if (pGM == NULL || pCell == NULL)
	{
		return;
	}

	CBCGPBaseChartImpl* pChartImpl = pCell->GetRelatedSeries()->GetChartImpl();
	
	if (pChartImpl != NULL)
	{
		pChartImpl->OnDrawChartLegendKeyEx(pGM, rectLegendKey, pCell->GetCellParams(), pCell->GetRelatedSeries());
	}
}
//******************************************************************************************
void CBCGPChartVisualObject::OnDrawChartLegendLabel(CBCGPGraphicsManager* pGM, CString& strLabel, CBCGPRect& rectLabel, 
					const BCGPChartFormatLabel& labelStyle, CBCGPChartSeries* pSeries, int nDataPointIndex)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pGM);

	UNREFERENCED_PARAMETER(nDataPointIndex);
	UNREFERENCED_PARAMETER(pSeries);

	OnDrawChartElement(pGM, rectLabel, labelStyle, CBCGPChartVisualObject::CE_LEGEND_ENTRY, FALSE, FALSE);

	const CBCGPBrush& brText = labelStyle.m_brTextColor.IsEmpty() ? m_currentTheme.m_brLegendEntryTextColor :
																	labelStyle.m_brTextColor;
	pGM->DrawText(strLabel, rectLabel, labelStyle.m_textFormat, brText);
}
//******************************************************************************************
void CBCGPChartVisualObject::OnDrawChartLegendLabelEx(CBCGPGraphicsManager* pGM, const CString& strLabel, 
							const CBCGPRect& rectLabel, CBCGPChartLegendCell* pCell)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pGM);
	ASSERT_VALID(pCell);

	pGM->DrawText(strLabel, rectLabel, pCell->GetCellParams().m_textFormat, pCell->GetCellParams().m_brTextColor);
}
//****************************************************************************************
void CBCGPChartVisualObject::OnDrawChartEffects(CBCGPGraphicsManager* pGM)
{
	for (POSITION pos = m_lstChartEffects.GetHeadPosition(); pos != NULL;)
	{
		CBCGPChartBaseEffect* pEffect = m_lstChartEffects.GetNext(pos);

		if (pEffect != NULL)
		{
			ASSERT_VALID(pEffect);

			//if (IsDirty())
			{
				pEffect->OnCalcScreenPoints(pGM);
			}

			pEffect->OnDraw(pGM);
		}
	}
}
//****************************************************************************************
void CBCGPChartVisualObject::OnDrawChartObjects(CBCGPGraphicsManager* pGM, BOOL bForeground)
{
	for (POSITION pos = m_lstChartObjects.GetHeadPosition(); pos != NULL;)
	{
		CBCGPChartObject* pObject = m_lstChartObjects.GetNext(pos);

		if (pObject != NULL)
		{
			ASSERT_VALID(pObject);
			if (pObject->IsForeground() && bForeground || 
				!pObject->IsForeground() && !bForeground)
			{
				CBCGPRect rectClip = pObject->GetBoundingRect();
				if (!rectClip.IsRectEmpty())
				{
					pGM->SetClipRect(rectClip);
					pObject->OnDraw(pGM, m_rectDiagramArea);
					pGM->ReleaseClipArea();
				}
			}
		}
	}	
}
//****************************************************************************************
// Series management
//****************************************************************************************
int CBCGPChartVisualObject::GetSeriesCount(BOOL bIncludeNulls)
{
	ASSERT_VALID (this);

	if (bIncludeNulls)
	{
		return (int)m_arData.GetSize();
	}

	int nCount = 0;
	
	for (int i = 0; i < m_arData.GetSize(); i++)
	{
		if (GetSeries(i, FALSE) != NULL)	
		{
			nCount++;
		}
	}

	return nCount;
}
//****************************************************************************************
CBCGPChartSeries* CBCGPChartVisualObject::CreateSeries(const CString& strName, const CBCGPColor& color, 
						   BCGPChartType type, BCGPChartCategory category, int nSeriesIndex)
{
	if (category == BCGPChartDefault)
	{
		category = GetChartCategory();
	}

	if (type == BCGP_CT_DEFAULT)
	{
		type = GetChartType();
	}

	if (!IsCategory3D(m_Category) && IsCategory3D(category) || 
		IsCategory3D(m_Category) && !IsCategory3D(category))
	{
		m_Category = category;
	}

	CBCGPChartSeries* pSeries = OnCreateChartSeries(category, type);
	if (pSeries != NULL)
	{
		pSeries->m_strSeriesName = strName;
		pSeries->SetDefaultSeriesColor(color);

		OnAddSeries(pSeries, nSeriesIndex);
	}
	return pSeries;
}
//****************************************************************************************
int CBCGPChartVisualObject::AddSeries(CBCGPChartSeries* pSeries, BCGPChartFormatSeries* pFormatSeries, 
									  int nIndex)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pSeries);
	if (pSeries == NULL)
	{
		return -1;
	}

	if (pFormatSeries != NULL)
	{
		pSeries->SetSeriesFormat(*pFormatSeries);
	}

	if (pSeries->GetRelatedAxis(CBCGPChartSeries::AI_X) == NULL)
	{
		// axis has not been set - set default primary axes
		pSeries->ShowOnPrimaryAxis(TRUE);
	}
		
	return OnAddSeries(pSeries, nIndex);
}
//****************************************************************************************
int CBCGPChartVisualObject::AddChartDataYX (double dblY, double dblX, int nSeries, DWORD_PTR dwUserData, BCGPChartFormatSeries* pFormatDataPoint)
{
	CBCGPChartSeries* pSeries = GetSeries (nSeries, TRUE);
	if (pSeries == NULL)
	{
		return -1;
	}

	CBCGPChartDataPoint dp(dblY, CBCGPChartData::CI_Y, dwUserData, pFormatDataPoint);
	dp.SetComponentValue(dblX, CBCGPChartData::CI_X);
	
	return pSeries->AddDataPoint(dp);
}
//****************************************************************************************
int	CBCGPChartVisualObject::AddChartDataYXZ (double dblY, double dblX, double dblZ, int nSeries, DWORD_PTR dwUserData, BCGPChartFormatSeries* pFormatDataPoint)
{
	CBCGPChartSeries* pSeries = GetSeries (nSeries, TRUE);
	if (pSeries == NULL)
	{
		return -1;
	}

	CBCGPChartDataPoint dp(dblY, CBCGPChartData::CI_Y, dwUserData, pFormatDataPoint);
	
	dp.SetComponentValue(dblX, CBCGPChartData::CI_X);
	dp.SetComponentValue(dblZ, CBCGPChartData::CI_Z);
	
	return pSeries->AddDataPoint(dp);
}
//****************************************************************************************
int CBCGPChartVisualObject::AddChartDataYXY1 (double dblY, double dblX, double dblY1, int nSeries, DWORD_PTR dwUserData, BCGPChartFormatSeries* pFormatDataPoint)
{
	CBCGPChartSeries* pSeries = GetSeries (nSeries, TRUE);
	if (pSeries == NULL)
	{
		return -1;
	}

	CBCGPChartDataPoint dp(_T(""), dblX, dblY, dblY1, dwUserData, pFormatDataPoint);
	return pSeries->AddDataPoint(dp);
}
//****************************************************************************************
int CBCGPChartVisualObject::AddChartDataYXY1 (const CString& strCategoryName, double dblY, double dblX, double dblY1, int nSeries, DWORD_PTR dwUserData, BCGPChartFormatSeries* pFormatDataPoint)
{
	CBCGPChartSeries* pSeries = GetSeries (nSeries, TRUE);
	if (pSeries == NULL)
	{
		return -1;
	}

	return pSeries->AddDataPoint(CBCGPChartDataPoint(strCategoryName, dblX, dblY, dblY1, dwUserData, pFormatDataPoint));
}
//****************************************************************************************
int CBCGPChartVisualObject::AddChartDataYY1 (const CString& strCategoryName, double dblY, double dblY1, 
											int nSeries, BCGPChartFormatSeries* pFormatDataPoint, DWORD_PTR dwUserData)
{
	CBCGPChartSeries* pSeries = GetSeries (nSeries, TRUE);
	if (pSeries == NULL)
	{
		return -1;
	}

	return pSeries->AddDataPoint(CBCGPChartDataPoint(strCategoryName, CBCGPChartValue(), dblY, dblY1, dwUserData, pFormatDataPoint));
}
//****************************************************************************************
int CBCGPChartVisualObject::AddChartDataYY1Y2(double dblY, double dblY1, double dblY2, int nSeries, BCGPChartFormatSeries* pFormatDataPoint, DWORD_PTR dwUserData)
{
	CBCGPChartSeries* pSeries = GetSeries (nSeries, TRUE);
	if (pSeries == NULL)
	{
		return -1;
	}

	return pSeries->AddDataPoint(CBCGPChartDataPoint(dblY, dblY1, dblY2, dwUserData, pFormatDataPoint));
}
//****************************************************************************************
int CBCGPChartVisualObject::AddChartData (CArray<double, double>& arData, int nSeries, DWORD_PTR dwUserData, BCGPChartFormatSeries* pFormatDataPoint)
{
	ASSERT_VALID (this);

	CBCGPChartSeries* pSeries = GetSeries (nSeries, TRUE);
	if (pSeries == NULL)
	{
		return -1;
	}

	ASSERT_VALID(pSeries);

	CBCGPChartDataPoint dp(arData, dwUserData, pFormatDataPoint);
	return pSeries->AddDataPoint(dp);
}
//****************************************************************************************
BOOL CBCGPChartVisualObject::SetChartData(double dblData, int nSeries, int nDataPointIndex, CBCGPChartData::ComponentIndex ci)
{
	ASSERT_VALID (this);

	CBCGPChartSeries* pSeries = GetSeries (nSeries, TRUE);
	if (pSeries == NULL || nDataPointIndex >= pSeries->GetDataPointCount() || nDataPointIndex < 0)
	{
		return FALSE;
	}

	ASSERT_VALID(pSeries);

	return pSeries->SetDataPointValue(nDataPointIndex, dblData, ci);
}
//****************************************************************************************
int CBCGPChartVisualObject::AddChartData (double dblData, int nSeries, CBCGPChartData::ComponentIndex ci, DWORD_PTR dwUserData, BCGPChartFormatSeries* pFormatDataPoint, 
				   BCGPChartFormatSeries* pFormatSeries)
{
	ASSERT_VALID (this);

	CBCGPChartSeries* pSeries = GetSeries (nSeries, TRUE);
	if (pSeries == NULL)
	{
		return -1;
	}

	ASSERT_VALID(pSeries);

	CBCGPChartDataPoint dataPoint (dblData, ci, dwUserData, pFormatDataPoint);
	int nDataPointIndex = pSeries->AddDataPoint(dataPoint);

	if (pSeries->m_strSeriesName.IsEmpty())
	{
		pSeries->m_strSeriesName.Format(_T("Series %d"), nSeries + 1);
	}

	if (pFormatSeries != NULL)
	{
		pSeries->SetSeriesFormat(*pFormatSeries);
	}

	return nDataPointIndex;
}
//****************************************************************************************
int CBCGPChartVisualObject::AddChartData (double dblY, BCGPChartFormatSeries* pFormatDataPoint, int nSeries, DWORD_PTR dwUserData)
{
	ASSERT_VALID (this);

	CBCGPChartSeries* pSeries = GetSeries (nSeries, TRUE);

	if (pSeries == NULL)
	{
		return -1;
	}

	ASSERT_VALID(pSeries);
	return pSeries->AddDataPoint(dblY, pFormatDataPoint, dwUserData);
}
//****************************************************************************************
int  CBCGPChartVisualObject::AddChartData (const CString& strCategoryName, double dblY, int nSeries, BCGPChartFormatSeries* pFormatDataPoint, DWORD_PTR dwUserData)
{
	ASSERT_VALID (this);

	CBCGPChartSeries* pSeries = GetSeries (nSeries, TRUE);
	if (pSeries == NULL)
	{
		return -1;
	}

	ASSERT_VALID(pSeries);
	return pSeries->AddDataPoint(strCategoryName, dblY, pFormatDataPoint, dwUserData);
}
//****************************************************************************************
void CBCGPChartVisualObject::AddDataPoints(const CBCGPDoubleArray& arYValues, int nSeries, 
										  CBCGPDoubleArray* parXValues, CBCGPDoubleArray* parY1Values, 
										  BOOL bRecalcMinMaxValues)
{
	ASSERT_VALID (this);
	
	CBCGPChartSeries* pSeries = GetSeries (nSeries, TRUE);
	if (pSeries == NULL)
	{
		return;
	}
	
	ASSERT_VALID(pSeries);
	pSeries->AddDataPoints(arYValues, parXValues, parY1Values, bRecalcMinMaxValues);
}
//****************************************************************************************
void CBCGPChartVisualObject::AddDataPointsOptimized(const CBCGPDoubleArray& arYValues, int nSeries, CBCGPDoubleArray* pXValues, 
		CBCGPDoubleArray* pY1Values, BOOL bRecalcMinMaxValues)
{
	ASSERT_VALID (this);
	
	CBCGPChartSeries* pSeries = GetSeries (nSeries, TRUE);
	if (pSeries == NULL)
	{
		return;
	}
	
	ASSERT_VALID(pSeries);
	pSeries->AddDataPointsOptimized(arYValues, pXValues, pY1Values, bRecalcMinMaxValues);
}
//****************************************************************************************
int CBCGPChartVisualObject::AddChartEmptyData(int nSeries, DWORD_PTR dwUserData, BCGPChartFormatSeries* pFormatDataPoint, 
									   BCGPChartFormatSeries* pFormatSeries)
{
	ASSERT_VALID (this);

	CBCGPChartSeries* pSeries = GetSeries (nSeries, TRUE);
	if (pSeries == NULL)
	{
		return -1;
	}

	if (pFormatSeries != NULL)
	{
		pSeries->SetSeriesFormat(*pFormatSeries);
	}

	return pSeries->AddEmptyDataPoint(dwUserData, pFormatDataPoint);
}
//****************************************************************************************
int CBCGPChartVisualObject::AddChartEmptyData(double dblX, int nSeries, DWORD_PTR dwUserData, BCGPChartFormatSeries* pFormatDataPoint)
{
	ASSERT_VALID (this);
	
	CBCGPChartSeries* pSeries = GetSeries (nSeries, TRUE);
	if (pSeries == NULL)
	{
		return -1;
	}

	return pSeries->AddEmptyDataPoint(dblX, dwUserData, pFormatDataPoint);
}
//****************************************************************************************
int CBCGPChartVisualObject::AddChartEmptyData(const CString& strCategoryName, int nSeries, DWORD_PTR dwUserData, BCGPChartFormatSeries* pFormatDataPoint)
{
	ASSERT_VALID (this);
	
	CBCGPChartSeries* pSeries = GetSeries (nSeries, TRUE);
	if (pSeries == NULL)
	{
		return -1;
	}

	return pSeries->AddEmptyDataPoint(strCategoryName, dwUserData, pFormatDataPoint);
}
//****************************************************************************************
BOOL CBCGPChartVisualObject::SetSeriesName (const CString& strName, int nSeries)
{
	ASSERT_VALID (this);

	CBCGPChartSeries* pSeries = GetSeries (nSeries, TRUE);
	if (pSeries == NULL)
	{
		return FALSE;
	}

	pSeries->m_strSeriesName = strName;
	return TRUE;
}
//****************************************************************************************
BOOL CBCGPChartVisualObject::ShowSeries (BOOL bVisible, int nSeries, BOOL bIsSecondaryAxis)
{
	ASSERT_VALID (this);

	CBCGPChartSeries* pSeries = GetSeries (nSeries, FALSE);
	if (pSeries == NULL || pSeries->m_bVisible == bVisible)
	{
		return FALSE;
	}

	pSeries->m_bVisible = bVisible;
	pSeries->ShowOnPrimaryAxis(!bIsSecondaryAxis);

	return TRUE;
}
//****************************************************************************************
BOOL CBCGPChartVisualObject::FormatSeries (BCGPChartFormatSeries* pFormatSeries, int nSeries)
{
	ASSERT_VALID (this);

	CBCGPChartSeries* pSeries = GetSeries (nSeries, TRUE);
	if (pSeries == NULL)
	{
		return FALSE;
	}

	pFormatSeries == NULL ? pSeries->ResetFormat() : pSeries->SetSeriesFormat(*pFormatSeries);
	return TRUE;
}
//****************************************************************************************
CBCGPChartSeries* CBCGPChartVisualObject::GetSeries (int nSeries, BOOL bAllocate)
{
	ASSERT_VALID (this);

	if (nSeries < 0)
	{
		return NULL;
	}

	CBCGPChartSeries* pSeries = NULL;
	BOOL bNeedAlloc = bAllocate || m_arData.GetSize() == 0 && nSeries == 0;

	if (nSeries >= m_arData.GetSize() || (pSeries = DYNAMIC_DOWNCAST (CBCGPChartSeries, m_arData [nSeries])) == NULL)
	{
		if (bNeedAlloc)
		{
			pSeries = OnCreateChartSeries(m_Category, m_Type);
			OnAddSeries(pSeries, nSeries);
		}
	}

	return pSeries;
}
//****************************************************************************************
void CBCGPChartVisualObject::GetAllNonFormulaSeries(CArray<CBCGPChartSeries*, CBCGPChartSeries*>& arSeries)
{
	ASSERT_VALID (this);

	arSeries.RemoveAll();

	for (int i = 0; i < GetSeriesCount(); i++)
	{
		CBCGPChartSeries* p = GetSeries(i, FALSE);
		if (p != NULL && !p->IsFormulaSeries() && !p->IsVirtualMode())
		{
			arSeries.Add(p);
		}
	}
}
//****************************************************************************************
void CBCGPChartVisualObject::InvalidateTrendFormulaSeries()
{
	for (int i = 0; i < m_arData.GetSize(); i++)
	{
		CBCGPChartSeries* pSeries = GetSeries (i, FALSE);

		if (pSeries == NULL)
		{
			continue;
		}

		ASSERT_VALID(pSeries);

		CBCGPChartTrendFormula* pFormula = DYNAMIC_DOWNCAST(CBCGPChartTrendFormula, pSeries->GetFormula());

		if (pFormula != NULL && pFormula->GetInputSeriesCount() > 0)
		{
			pFormula->RemoveAllCoefficients();
		}
	}
}
//****************************************************************************************
int CBCGPChartVisualObject::OnAddSeries(CBCGPChartSeries* pSeries, int nSeriesIndex)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pSeries);

	if (pSeries == NULL)
	{
		return -1;
	}

	if (pSeries->GetRelatedAxis(CBCGPChartSeries::AI_X) == NULL)
	{
		// axis has not been set - set default primary axes
		pSeries->ShowOnPrimaryAxis();
	}

	if (nSeriesIndex < 0)
	{
		BOOL bEmptyEntryFound = FALSE;

		for (int i = 0; i < m_arData.GetSize(); i++)
		{
			if (m_arData[i] == NULL)
			{
				nSeriesIndex = i;
				m_arData.SetAtGrow(nSeriesIndex, pSeries);
				bEmptyEntryFound = TRUE;
				break;
			}
		}

		if (!bEmptyEntryFound)
		{
			nSeriesIndex = (int)m_arData.Add(pSeries);
		}
	}
	else
	{
		m_arData.SetAtGrow(nSeriesIndex, pSeries);
	}

	// if the default chart type differs, a new series will be created and it will replace the 
	// current series; otherwise it will return "this"
	//pSeries = pSeries->SetChartType(m_Category, m_Type);

	if (pSeries->IsAutoColorDataPoints())
	{
		UpdateSeriesColorIndexes();
	}
	else
	{
		pSeries->SetColorIndex(m_nLastColorIndex++);
	}

	if (pSeries->GetSeriesFormat().m_curveType == (BCGPChartFormatSeries::ChartCurveType)-1)
	{
		pSeries->SetCurveType(m_curveType);
	}

	return nSeriesIndex;
}
//****************************************************************************************
int CBCGPChartVisualObject::FindSeriesIndex(CBCGPChartSeries* pSeries) const
{
	ASSERT_VALID(this);

	for (int i = 0; i < m_arData.GetSize(); i++)
	{
		CBCGPChartSeries* pS = DYNAMIC_DOWNCAST(CBCGPChartSeries, m_arData[i]);

		if (pS == pSeries)
		{
			return i;
		}
	}

	return -1;
}
//****************************************************************************************
void CBCGPChartVisualObject::UpdateSeriesColors()
{
	for (int i = 0; i < m_arData.GetSize(); i++)
	{
		CBCGPChartSeries* pSeries = DYNAMIC_DOWNCAST(CBCGPChartSeries, GetSeries (i, FALSE));

		if (pSeries == NULL)
		{
			continue;
		}

		pSeries->UpdateSeriesColors();
	}
}
//****************************************************************************************
CBCGPChartSeries* CBCGPChartVisualObject::OnCreateChartSeries(BCGPChartCategory category, BCGPChartType type)
{
	ASSERT_VALID(this);

	CBCGPChartSeries* pSeries = NULL;
	if (category == BCGPChartBubble)
	{
		pSeries = new CBCGPChartBubbleSeries(this);
	}
	else if (category == BCGPChartLine || category == BCGPChartLine3D)
	{
		pSeries = new CBCGPChartLineSeries(this, category, type);
	}
	else if (category == BCGPChartArea || category == BCGPChartArea3D)
	{
		pSeries = new CBCGPChartAreaSeries(this, category, type);
	}
	else if (category == BCGPChartSurface3D)
	{
		pSeries = new CBCGPChartSurfaceSeries(this);
	}
	else if (category == BCGPChartPie || category == BCGPChartPie3D || category == BCGPChartTorus3D)
	{
		pSeries = new CBCGPChartPieSeries(this, category);
	}
	else if (category == BCGPChartDoughnut || category == BCGPChartDoughnut3D)
	{
		pSeries = new CBCGPChartDoughnutSeries(this, category);
	}
	else if (category == BCGPChartPyramid || category == BCGPChartPyramid3D)
	{
		pSeries = new CBCGPChartPyramidSeries(this, category);
	}
	else if (category == BCGPChartFunnel || category == BCGPChartFunnel3D)
	{
		pSeries = new CBCGPChartFunnelSeries(this, category);
	}
	else if (category == BCGPChartBar || category == BCGPChartColumn || 
			category == BCGPChartColumn3D  || category == BCGPChartBar3D)
	{
		pSeries = new CBCGPChartBarSeries(this, category, type);
	}
	else if (category == BCGPChartHistogram)
	{
		pSeries = new CBCGPChartHistogramSeries(this, type);
	}
	else if (category == BCGPChartLongData)
	{
		pSeries = new CBCGPChartLongSeries(this);
	}
	else if (category == BCGPChartHistoricalLine)
	{
		CWnd* pWnd = CWnd::FromHandle(GetDesktopWindow());
		int nHistoryDepth = 1000;

		if (pWnd != NULL)
		{
			ASSERT_VALID(pWnd);

			CRect rect;
			pWnd->GetWindowRect(rect);

			nHistoryDepth = rect.Width();
		}

		pSeries = new CBCGPChartHistoricalLineSeries(this, nHistoryDepth);
	}
	else if (category == BCGPChartPolar)
	{
		pSeries = new CBCGPChartPolarSeries(this);
	}
	else if (category == BCGPChartTernary)
	{
		pSeries = new CBCGPChartTernarySeries(this);
	}
	else if (category == BCGPChartStock)
	{
		pSeries = new CBCGPChartStockSeries(this, CBCGPBaseChartStockSeries::SST_CANDLE);
	}
	else
	{
		pSeries = new CBCGPChartSeries(this, category, type);
	}

	pSeries->ShowDataLabel(m_dataLabelOptions.m_bShowDataLabel);
	if (m_dataLabelOptions.m_bShowDataLabel)
	{
		pSeries->SetDataLabelDrawBorder(m_dataLabelOptions.m_bDrawDataLabelBorder);
		pSeries->SetDataLabelDropLineToMarker(m_dataLabelOptions.m_bDropLineToMarker);
		if (m_dataLabelOptions.m_dblAngle != -1)
		{
			pSeries->SetDataLabelAngle(m_dataLabelOptions.m_dblAngle);
		}
	}	

	if (category != BCGPChartTernary)
	{
		pSeries->SetMarkerOptions(m_markerOptions);
	}

	return pSeries;
}
//****************************************************************************************
CBCGPChartValue CBCGPChartVisualObject::GetDataPointValue (int nSeries, int nDataPointIndex, CBCGPChartData::ComponentIndex ci)
{
	ASSERT_VALID (this);

	CBCGPChartSeries* pSeries = GetSeries (nSeries, FALSE);

	if (pSeries == NULL)
	{
		ASSERT(FALSE);
		TRACE0("Invalid series or datapoint index");
		return CBCGPChartValue();
	}
	
	return pSeries->GetDataPointValue(nDataPointIndex, ci);
}
//****************************************************************************************
const BCGPChartFormatSeries* CBCGPChartVisualObject::GetDataPointFormat(int nSeries, int nDataPointIndex) 
{
	ASSERT_VALID (this);

	CBCGPChartSeries* pSeries = GetSeries (nSeries, FALSE);

	if (pSeries == NULL || nDataPointIndex < 0 || 
		nDataPointIndex >= pSeries->GetDataPointCount())
	{
		ASSERT(FALSE);
		TRACE0("Invalid series or datapoint index");
		return NULL;
	}

	const BCGPChartFormatSeries* pFormat = pSeries->GetDataPointAt(nDataPointIndex)->GetFormat();

	if (pFormat != NULL)
	{
		return pFormat;
	}

	return &pSeries->GetSeriesFormat();
}
//****************************************************************************************
void CBCGPChartVisualObject::SetDataPointFormat(const BCGPChartFormatSeries& dataPointStyle, int nDataPointIndex, int nSeriesIndex)
{
	CBCGPChartSeries* pSeries = GetSeries (nSeriesIndex, FALSE);

	if (pSeries == NULL || nDataPointIndex < 0 || 
		nDataPointIndex >= pSeries->GetDataPointCount())
	{
		ASSERT(FALSE);
		TRACE0("Invalid series or datapoint index.");
		return;
	}

	CBCGPChartDataPoint* pDataPoint = (CBCGPChartDataPoint*) pSeries->GetDataPointAt(nDataPointIndex);

	ASSERT(pDataPoint != NULL);

	pDataPoint->SetFormat(dataPointStyle);
	SetDirty();
}
//****************************************************************************************
void CBCGPChartVisualObject::SetDataPointDataLabelText(const CString& strText, int nDataPointIndex, int nSeriesIndex)
{
	CBCGPChartSeries* pSeries = GetSeries (nSeriesIndex, FALSE);

	if (pSeries == NULL || nDataPointIndex < 0 || 
		nDataPointIndex >= pSeries->GetDataPointCount())
	{
		ASSERT(FALSE);
		TRACE0("Invalid series or datapoint index.");
		return;
	}

#ifdef _DEBUG
	CBCGPChartDataPoint* pDataPoint = (CBCGPChartDataPoint*) pSeries->GetDataPointAt(nDataPointIndex);
	ASSERT(pDataPoint != NULL);
#endif

	pSeries->OnSetDataPointDataLabelText(nDataPointIndex, strText);
	SetDirty();
}
//****************************************************************************************
void CBCGPChartVisualObject::SetDataPointLegendLabelText(const CString& strText, int nDataPointIndex, int nSeriesIndex)
{
	CBCGPChartSeries* pSeries = GetSeries (nSeriesIndex, FALSE);

	if (pSeries == NULL || nDataPointIndex < 0 || 
		nDataPointIndex >= pSeries->GetDataPointCount())
	{
		ASSERT(FALSE);
		TRACE0("Invalid series or datapoint index.");
		return;
	}

	CBCGPChartDataPoint* pDataPoint = (CBCGPChartDataPoint*) pSeries->GetDataPointAt(nDataPointIndex);

	ASSERT(pDataPoint != NULL);

	pDataPoint->m_strLegendLabel = strText;
	SetDirty();
}
//****************************************************************************************
void CBCGPChartVisualObject::SetDataPointCategoryName(const CString& strCategoryName, int nDataPointIndex, int nSeriesIndex)
{
	CBCGPChartSeries* pSeries = GetSeries (nSeriesIndex, FALSE);

	if (pSeries == NULL || nDataPointIndex < 0 || 
		nDataPointIndex >= pSeries->GetDataPointCount())
	{
		ASSERT(FALSE);
		TRACE0("Invalid series or datapoint index.");
		return;
	}

	pSeries->SetDataPointCategoryName(strCategoryName, nDataPointIndex);
	SetDirty();
}
//****************************************************************************************
void CBCGPChartVisualObject::SetDataLabelsOptions(const BCGPChartDataLabelOptions& options)
{
	m_dataLabelOptions = options;

	for (int i = 0; i < GetSeriesCount(); i++)
	{
		CBCGPChartSeries* pSeries = GetSeries (i, FALSE);

		if (pSeries == NULL)
		{
			continue;
		}

		ASSERT_VALID(pSeries);

		pSeries->SetDataLabelOptions(options);
	}

	SetDirty();
}
//****************************************************************************************
void CBCGPChartVisualObject::ShowAllDataLabels(BOOL bShow)
{
	m_dataLabelOptions.m_bShowDataLabel = bShow;

	for (int i = 0; i < GetSeriesCount(); i++)
	{
		CBCGPChartSeries* pSeries = GetSeries (i, FALSE);

		if (pSeries == NULL)
		{
			continue;
		}

		ASSERT_VALID(pSeries);

		pSeries->ShowDataLabel(bShow);
	}

	SetDirty();
}
//****************************************************************************************
void CBCGPChartVisualObject::ShowDataLabels(BOOL bShow, BOOL bDrawBorder, BOOL bDropLineToMarker, 
											double dblAngle)
{
	m_dataLabelOptions.m_bShowDataLabel = bShow;
	m_dataLabelOptions.m_bDrawDataLabelBorder = bDrawBorder;
	m_dataLabelOptions.m_bDropLineToMarker = bDropLineToMarker;
	m_dataLabelOptions.m_dblAngle = dblAngle;

	for (int i = 0; i < GetSeriesCount(); i++)
	{
		CBCGPChartSeries* pSeries = GetSeries (i, FALSE);

		if (pSeries == NULL)
		{
			continue;
		}

		ASSERT_VALID(pSeries);

		pSeries->ShowDataLabel(bShow);

		if (bShow)
		{
			pSeries->SetDataLabelDrawBorder(bDrawBorder);
			pSeries->SetDataLabelDropLineToMarker(bDropLineToMarker);
			if (dblAngle != -1)
			{
				pSeries->SetDataLabelAngle(dblAngle);
			}
		}
	}

	SetDirty();
}
//****************************************************************************************
void CBCGPChartVisualObject::ShowDataMarkers(BOOL bShow, int nSize,
			BCGPChartMarkerOptions::MarkerShape shape)
{
	m_markerOptions.m_bShowMarker = bShow;

	if (nSize != -1)
	{
		m_markerOptions.SetMarkerSize(nSize);
	}

	if (shape != (BCGPChartMarkerOptions::MarkerShape) -1)
	{
		m_markerOptions.m_markerShape = shape;
	}

	for (int i = 0; i < GetSeriesCount(); i++)
	{
		CBCGPChartSeries* pSeries = GetSeries (i, FALSE);

		if (pSeries == NULL)
		{
			continue;
		}

		ASSERT_VALID(pSeries);

		pSeries->ShowMarker(bShow);

		if (bShow && shape != (BCGPChartMarkerOptions::MarkerShape) -1)
		{
			pSeries->SetMarkerShape(shape);
		}

		if (bShow && nSize != -1)
		{
			pSeries->SetMarkerSize(nSize);
		}
	}

	SetDirty();
}
//****************************************************************************************
void CBCGPChartVisualObject::SetCurveType(BCGPChartFormatSeries::ChartCurveType curveType)
{
	m_curveType = curveType;

	for (int i = 0; i < GetSeriesCount(); i++)
	{
		CBCGPChartSeries* pSeries = GetSeries (i, FALSE);

		if (pSeries == NULL)
		{
			continue;
		}

		ASSERT_VALID(pSeries);

		pSeries->SetCurveType(curveType);
	}

	SetDirty();
}
//****************************************************************************************
DWORD_PTR CBCGPChartVisualObject::GetDataPointUserData (int nSeries, int nDataPointIndex)
{
	ASSERT_VALID (this);

	CBCGPChartSeries* pSeries = GetSeries (nSeries, FALSE);

	if (pSeries == NULL || nDataPointIndex < 0 || 
		nDataPointIndex >= pSeries->GetDataPointCount())
	{
		ASSERT(FALSE);
		TRACE0("Invalid series or datapoint index");
		return 0;
	}

	return pSeries->GetDataPointAt(nDataPointIndex)->m_dwUserData;
}
//****************************************************************************************
int CBCGPChartVisualObject::GetVisibleSeriesCount(CRuntimeClass* pImplType)
{
	ASSERT_VALID (this);

	int nCount = 0;

	for (int i = 0; i < m_arData.GetSize(); i++)
	{
		CBCGPChartSeries* pSeries = GetSeries(i, FALSE);

		if (pSeries == NULL || !pSeries->m_bVisible)
		{
			continue;
		}

		CBCGPBaseChartImpl* pChartImpl = pSeries->GetChartImpl();

		if (pChartImpl == NULL)
		{
			continue;
		}

		ASSERT_VALID(pChartImpl);

		if (pImplType == NULL || pImplType != NULL && pChartImpl->IsKindOf(pImplType)) 	
		{
			nCount++;
		}
	}

	return nCount;

}
//****************************************************************************************
BOOL CBCGPChartVisualObject::NeedDisplayAxes()
{
	for (int i = 0; i < m_arData.GetSize(); i++)
	{
		CBCGPChartSeries* pSeries = GetSeries(i, FALSE);
		if (pSeries == NULL || !pSeries->m_bVisible)
		{
			continue;
		}

		if (pSeries->GetChartImpl() != NULL && 
			pSeries->GetChartImpl()->GetAxisType() != CBCGPBaseChartImpl::AT_NO_AXIS)
		{
			return TRUE;
		}
	}

	return FALSE;
}
//****************************************************************************************
void CBCGPChartVisualObject::CleanUpChartData (int nSeries, BOOL bRemoveUnusedEntries)
{
	ASSERT_VALID (this);

	if (nSeries == -1)
	{
		for (int i = 0; i < m_arData.GetSize(); i++)
		{
			CBCGPChartSeries* pSeries = GetSeries (i, FALSE);
			if (pSeries != NULL)
			{
				delete pSeries;
				m_arData [i] = NULL;
			}
		}

		m_arData.RemoveAll();
		m_nLastColorIndex = 0;
	}
	else
	{
		CBCGPChartSeries* pSeries = GetSeries (nSeries, FALSE);
		if (pSeries != NULL)
		{
			delete pSeries;
			if (bRemoveUnusedEntries)
			{
				m_arData.RemoveAt(nSeries);
			}
			else
			{
				m_arData [nSeries] = NULL;
			}
		}

		//TODO Sort date values
	}

	ResetAxes();
}
//****************************************************************************************
// External Legend support
//****************************************************************************************
void CBCGPChartVisualObject::AddRelatedLegend(CBCGPChartLegendVisualObject* pLegend, BOOL bRedraw)
{
	ASSERT_VALID(this);

	if (m_lstRelatedLegends.Find(pLegend) == NULL && !m_bIsThumbnailMode)
	{
		m_lstRelatedLegends.AddTail(pLegend);
		pLegend->AddRelatedChart(this, FALSE);
		SetLegendPosition(BCGPChartLayout::LP_NONE);
	}

	SetDirty(TRUE, bRedraw);
}
//****************************************************************************************
void CBCGPChartVisualObject::RemoveRelatedLegend(CBCGPChartLegendVisualObject* pLegend, BOOL bRedraw)
{
	ASSERT_VALID(this);

	POSITION pos = m_lstRelatedLegends.Find(pLegend);
	if (pos != NULL)
	{
		m_lstRelatedLegends.RemoveAt(pos);
		pLegend->RemoveRelatedChart(this, FALSE);
	}
	
	SetDirty(TRUE, bRedraw);
}
//****************************************************************************************
void CBCGPChartVisualObject::UpdateRelatedLegendColors()
{
	for (POSITION pos = m_lstRelatedLegends.GetHeadPosition(); pos != NULL;)
	{
		CBCGPChartLegendVisualObject* pLegend = DYNAMIC_DOWNCAST(CBCGPChartLegendVisualObject, 
			m_lstRelatedLegends.GetNext(pos));

		if (pLegend != NULL)
		{
			pLegend->InvalidateLegendContent(FALSE);

			if (pLegend->GetRelatedChart(0) == this)
			{
				pLegend->UpdateLegendColors(TRUE);
				pLegend->SetDirty(TRUE, TRUE);
			}
		}
	}
}
//****************************************************************************************
CBCGPChartLegendVisualObject* CBCGPChartVisualObject::GetRelatedLegend(int nIndex) const
{
	int nCount = (int)m_lstRelatedLegends.GetCount();

	if (nCount == 0 || nIndex >= nCount)
	{
		return NULL;
	}

	POSITION pos = m_lstRelatedLegends.FindIndex(nIndex);

	if (pos == NULL)
	{
		return NULL;
	}

	return DYNAMIC_DOWNCAST(CBCGPChartLegendVisualObject, m_lstRelatedLegends.GetAt(pos));
}
//****************************************************************************************
// End External Legend support
//****************************************************************************************
void CBCGPChartVisualObject::OnDrawTickMark(CBCGPGraphicsManager* pGM, CBCGPPoint& ptStart, CBCGPPoint& ptEnd, 
									BCGPChartFormatLine& lineStyle, double dblVal, BOOL bIsMajor)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pGM);

	if (pGM == NULL)
	{
		return;
	}

	UNREFERENCED_PARAMETER(dblVal);
	UNREFERENCED_PARAMETER(bIsMajor);

	CBCGPEngine3D* pEngine3D = GetEngine3D();

	if (pEngine3D != NULL)
	{
		const CBCGPBrush& brLine = lineStyle.m_brLineColor.IsEmpty() ? m_currentTheme.m_brAxisLineColor :
				lineStyle.m_brLineColor;

		pEngine3D->DrawLine(ptStart, ptEnd, brLine, lineStyle.GetLineWidth(TRUE), &lineStyle.m_strokeStyle);
	}
}
//****************************************************************************************
void CBCGPChartVisualObject::OnDrawAxisLabel(CBCGPGraphicsManager* pGM, double dblCurrValue, CString& strLabel,
									  CBCGPChartAxis* pAxis, const CBCGPRect& rectLabel, const CBCGPRect& rectLabels, 
									  const CBCGPRect& rectDiagram)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pAxis);
	ASSERT(pGM != NULL);

	if (pGM == NULL || pAxis == NULL)
	{
		return;
	}

	UNREFERENCED_PARAMETER(dblCurrValue);
	UNREFERENCED_PARAMETER(rectDiagram);
	UNREFERENCED_PARAMETER(rectLabels);

	OnDrawChartElement(pGM, rectLabel, pAxis->m_axisLabelsFormat, CBCGPChartVisualObject::CE_AXIS_LABEL, FALSE, FALSE);

	const CBCGPBrush& brText = pAxis->m_axisLabelsFormat.m_brTextColor.IsEmpty() ? 
		m_currentTheme.m_brAxisLabelTextColor : pAxis->m_axisLabelsFormat.m_brTextColor;

	CBCGPRect rectText = rectLabel;
	CBCGPSize szPadding = pAxis->m_axisLabelsFormat.GetContentPadding(TRUE);

	if (pAxis->IsVertical())
	{
		rectText.DeflateRect(0, szPadding.cy);
	}
	else
	{
		rectText.DeflateRect(szPadding.cx, 0);
	}

	pGM->DrawText(strLabel, rectText, pAxis->m_axisLabelsFormat.m_textFormat, brText);
}
//****************************************************************************************
void CBCGPChartVisualObject::NormalizeValueInAxis(const CBCGPChartAxis* pAxis, CBCGPChartValue& val) const
{
	ASSERT_VALID(pAxis);

	if (!IsChart3D() || !pAxis->IsNormalizeValuesInAxis())
	{
		return;
	}

	if (pAxis->IsFixedMinimumDisplayValue() && val.GetValue() < pAxis->GetMinDisplayedValue())
	{
		val = pAxis->GetMinDisplayedValue();
	}
	
	if (pAxis->IsFixedMaximumDisplayValue() && val.GetValue() > pAxis->GetMaxDisplayedValue())
	{
		val = pAxis->GetMaxDisplayedValue();
	}
}
//****************************************************************************************
CBCGPPoint CBCGPChartVisualObject::ScreenPointFromChartData(const CBCGPChartData& chartData, int nDataPointIndex, BOOL& bIsEmpty, 
													CBCGPChartSeries* pSeries) const
{
	ASSERT_VALID(this);

	CBCGPPoint pt(0, 0);
	CBCGPChartAxis* pXAxis = pSeries->GetRelatedAxis(CBCGPChartSeries::AI_X);
	CBCGPChartAxis* pYAxis = pSeries->GetRelatedAxis(CBCGPChartSeries::AI_Y);

	if (pXAxis == NULL || pYAxis == NULL)
	{
		return pt;
	}
	
	bIsEmpty = FALSE;
	CBCGPChartValue valY = chartData.GetValue(CBCGPChartData::CI_Y);

	if (valY.IsEmpty())
	{
		if (pSeries->GetTreatNulls() != CBCGPChartSeries::TN_VALUE)
		{
			bIsEmpty = TRUE;
			return pt;
		}
	}

	CBCGPChartValue valX = chartData.GetValue(CBCGPChartData::CI_X);
	if (valX.IsEmpty())
	{
		valX = nDataPointIndex + 1;
	}

	if (pXAxis->IsIndexedSeries())
	{
		valX = nDataPointIndex;
	}

	NormalizeValueInAxis(pYAxis, valY);

	pt.y = pXAxis->IsVertical() ? pXAxis->PointFromValue(valX, FALSE) :
		pYAxis->PointFromValue(valY,  FALSE);
	pt.x = pXAxis->IsVertical() ? pYAxis->PointFromValue(valY, FALSE) :
		pXAxis->PointFromValue(valX, FALSE);

	if (IsChart3D())
	{
		CBCGPChartAxis* pZAxis = pSeries->GetRelatedAxis(CBCGPChartSeries::AI_Z);

		if (pZAxis != NULL)
		{
			CBCGPChartValue valZ = chartData.GetValue(CBCGPChartData::CI_Z);
			if (valZ.IsEmpty())
			{
				if (!IsChart3DGrouped() && pSeries->IsStakedSeries())
				{
					valZ = pSeries->GetGroupID() + 1;
				}
				else
				{
					valZ = FindSeriesIndex(pSeries) + 1;
				}
			}

			pt.z = pZAxis->PointFromValue(valZ, FALSE);
		}
	}
	
	return pt;
}
//****************************************************************************************
CBCGPChartData CBCGPChartVisualObject::ChartDataFromScreenPoint(const CBCGPPoint& pt, CBCGPChartAxis::RoundType roundType, 
																BOOL bUseSelectionTypeForReturn, BOOL bIsStart) const
{
	ASSERT_VALID(this);

	CBCGPChartData data;

	for (int i = 0; i < m_arAxes.GetSize(); i++)
	{
		CBCGPChartAxis* pAxis = m_arAxes[i];

		if (pAxis == NULL || !pAxis->m_bInitialized)
		{
			continue;
		}

		ASSERT_VALID(pAxis);

		if (bUseSelectionTypeForReturn)
		{
			CBCGPRect rectSel(m_ptSelStart, m_ptSelEnd); 
			rectSel.Normalize();

			CArray<CBCGPRect, CBCGPRect> arRects;
			CBCGPRect rectAxis = pAxis->GetBoundingRects(arRects);

			if (!rectAxis.IntersectRect(rectSel))
			{
				continue;
			}

			if (m_selectionFormat.m_selectionType == BCGPChartFormatSelection::ST_HORZ_AXIS_ONLY && pAxis->IsVertical())
			{
				continue;
			}

			if (m_selectionFormat.m_selectionType == BCGPChartFormatSelection::ST_VERT_AXIS_ONLY && !pAxis->IsVertical())
			{
				continue;
			}
		}

		if (pAxis->m_bReverseOrder && roundType != CBCGPChartAxis::RT_EXACT)
		{
			roundType = bIsStart ? CBCGPChartAxis::RT_CEIL : CBCGPChartAxis::RT_FLOOR;
		}

		if (roundType == CBCGPChartAxis::RT_EXACT && pAxis->m_bFormatAsDate)
		{
			roundType = bIsStart ? CBCGPChartAxis::RT_FLOOR : CBCGPChartAxis::RT_CEIL;
		}

		double dblVal = pAxis->ValueFromPoint(pt, roundType);

		// we set the value not at the real component index (X, Y, Z),
		// but rather use its internal value array to set values at Axis ID indexes
		data.SetValue(CBCGPChartValue(dblVal, pAxis), (CBCGPChartData::ComponentIndex) pAxis->m_nAxisID);
	}

	return data;
}
//****************************************************************************************
// Resize Axes
//****************************************************************************************
BOOL CBCGPChartVisualObject::BeginResizeAxis(int nAxisID, BOOL bTop, const CBCGPPoint& ptStart)
{
	ASSERT_VALID(this);

	m_pResizedAxis = GetChartAxis(nAxisID);

	if (m_pResizedAxis == NULL || !IsResizeAxesEnabled())
	{
		return FALSE;
	}

	ASSERT_VALID(m_pResizedAxis);

	m_bResizeAxisTop = bTop;
	m_bResizeAxisMode = TRUE;
	m_pNextResizedAxis = bTop ? m_pResizedAxis->GetSplitTop() : m_pResizedAxis->GetSplitBottom();
	m_ptStartResize = ptStart;

	m_dblResizedAxisTopOffset = m_pResizedAxis->GetTopOffset();
	m_dblResizedAxisBottomOffset = m_pResizedAxis->GetBottomOffset();
	m_dblNextResizedAxisTopOffset = m_pNextResizedAxis->GetTopOffset();
	m_dblNextResizedAxisBottomOffset = m_pNextResizedAxis->GetBottomOffset();

	return TRUE;
}
//****************************************************************************************
void CBCGPChartVisualObject::EndResizeAxis(BOOL bResizeCanceled)
{
	ASSERT_VALID(this);

	if (bResizeCanceled)
	{
		if (m_pResizedAxis != NULL)
		{
			m_pResizedAxis->SetAxisOffsets(m_dblResizedAxisBottomOffset, m_dblResizedAxisTopOffset, 
						m_pResizedAxis->GetAxisGap());
		}
		
		if (m_pNextResizedAxis != NULL)
		{
			m_pNextResizedAxis->SetAxisOffsets(m_dblNextResizedAxisBottomOffset, m_dblNextResizedAxisTopOffset, 
						m_pNextResizedAxis->GetAxisGap());
		}

		SetDirty(TRUE, TRUE);
	}

	m_pResizedAxis = NULL;
	m_bResizeAxisMode = FALSE;
	m_pNextResizedAxis = NULL;
}
//****************************************************************************************
BOOL CBCGPChartVisualObject::UpdateAxisSize(const CBCGPPoint& ptCurr)
{
	ASSERT_VALID(this);

	if (!IsResizeAxesEnabled() || m_pResizedAxis == NULL)
	{
		return FALSE;
	}

	ASSERT_VALID(m_pResizedAxis);

	CBCGPPoint ptOffset = ptCurr - m_ptStartResize;

	if (!m_pResizedAxis->CanUpdateAxisSizeByOffset(ptOffset, m_bResizeAxisTop) || 
		m_pNextResizedAxis != NULL && !m_pNextResizedAxis->CanUpdateAxisSizeByOffset(ptOffset, !m_bResizeAxisTop))
	{
		return FALSE;
	}
	
	m_pResizedAxis->UpdateAxisSizeByOffset(ptOffset, m_bResizeAxisTop, m_rectDiagramArea);

	if (m_pNextResizedAxis != NULL)
	{
		ASSERT_VALID(m_pNextResizedAxis);
		m_pNextResizedAxis->UpdateAxisSizeByOffset(ptOffset, !m_bResizeAxisTop, m_rectDiagramArea);
	}

	SetDirty(TRUE, TRUE);

	return TRUE;
}
//****************************************************************************************
// Selection / Zoom / Scroll
//****************************************************************************************
BOOL CBCGPChartVisualObject::BeginThumbTrack(int nAxisID)
{
	ASSERT_VALID(this);

	if (m_bThumbTrackMode)
	{
		return TRUE;
	}

	m_pThumbTrackAxis = GetChartAxis(nAxisID);

	if (m_pThumbTrackAxis == NULL)
	{
		return FALSE;
	}

	ASSERT_VALID(m_pThumbTrackAxis);

	if (!IsScrollEnabled() || !m_pThumbTrackAxis->CanShowScrollBar())
	{
		m_pThumbTrackAxis = NULL;
		return FALSE;
	}


	m_bThumbTrackMode = TRUE;
	return TRUE;
}
//****************************************************************************************
void CBCGPChartVisualObject::EndThumbTrack()
{
	ASSERT_VALID(this);

	m_bThumbTrackMode = FALSE;
	m_pThumbTrackAxis = NULL;
}
//****************************************************************************************
BOOL CBCGPChartVisualObject::BeginThumbSize(int nAxisID, BOOL bLeftGrip, const CBCGPPoint& ptHit, double dblOffset)
{
	ASSERT_VALID(this);

	if (m_bThumbSizeMode)
	{
		return TRUE;
	}

	m_pThumbSizeAxis = GetChartAxis(nAxisID);

	if (m_pThumbSizeAxis == NULL)
	{
		return FALSE;
	}

	ASSERT_VALID(m_pThumbSizeAxis);

	if (!IsZoomEnabled() || !m_pThumbSizeAxis->CanShowScrollBar() || !m_pThumbSizeAxis->IsThumbGripEnabled())
	{
		m_pThumbSizeAxis = NULL;
		return FALSE;
	}

	m_bThumbSizeMode = TRUE;
	m_bThumbSizeLeft = bLeftGrip;
	m_ptThumbSizeStart = ptHit;
	m_dblThumbHitOffset = dblOffset;

	return TRUE;
}
//****************************************************************************************
void CBCGPChartVisualObject::EndThumbSize()
{
	ASSERT_VALID(this);

	m_bThumbSizeMode = FALSE;
	m_pThumbSizeAxis = FALSE;
	m_dblThumbHitOffset = 0;

	SetDirty(TRUE, TRUE);
}
//****************************************************************************************
void CBCGPChartVisualObject::BeginSelection(const CBCGPPoint& ptStart)
{
	ASSERT_VALID(this);

	CBCGPRect rectAxes = m_rectDiagramArea;

	m_ptSelStart = ptStart;
	m_ptSelEnd = ptStart;

	if (m_selectionFormat.m_selectionType == BCGPChartFormatSelection::ST_HORZ_AXIS_ONLY)
	{
		m_ptSelStart.y = rectAxes.top;
		m_ptSelEnd.y = rectAxes.bottom;
	}

	if (m_selectionFormat.m_selectionType == BCGPChartFormatSelection::ST_VERT_AXIS_ONLY)
	{
		m_ptSelStart.x = rectAxes.left;
		m_ptSelEnd.x = rectAxes.right;
	}
	
	m_bSelectionMode = TRUE;
}
//****************************************************************************************
void CBCGPChartVisualObject::BeginPan(const CBCGPPoint& ptStart)
{
	ASSERT_VALID(this);

	if (!m_bEnablePan)
	{
		return;
	}

	for (int i = 0; i < m_arAxes.GetSize(); i++)
	{
		CBCGPChartAxis* pAxis = m_arAxes[i];
		
		if (pAxis == NULL)
		{
			continue;
		}
		
		ASSERT_VALID(pAxis);
		pAxis->m_ptPanStart = ptStart;
	}

	m_ptPanStart = ptStart;
	m_ptPanOrigin = ptStart;
	m_bPanMode = TRUE;
}
//****************************************************************************************
void CBCGPChartVisualObject::UpdateSelection(const CBCGPPoint& ptCurr, BOOL bRedraw)
{
	ASSERT_VALID(this);

	if (!m_bSelectionMode)
	{
		BeginSelection(ptCurr);
		return;
	}

	CBCGPRect rectAxes = m_rectDiagramArea;

	m_ptSelEnd = ptCurr;

	if (m_selectionFormat.m_selectionType == BCGPChartFormatSelection::ST_HORZ_AXIS_ONLY)
	{
		m_ptSelEnd.y = rectAxes.bottom;
	}

	if (m_selectionFormat.m_selectionType == BCGPChartFormatSelection::ST_VERT_AXIS_ONLY)
	{
		m_ptSelEnd.x = rectAxes.right;
	}

	if (bRedraw)
	{
		Redraw();
	}
}
//****************************************************************************************
void CBCGPChartVisualObject::EndSelection(const CBCGPPoint& ptEnd)
{
	m_ptSelEnd = ptEnd;
	m_bSelectionMode = FALSE;
}
//****************************************************************************************
void CBCGPChartVisualObject::ZoomByCurrentSelection(BOOL bZoomOut)
{
	ASSERT_VALID(this);

	if (!m_bEnableZoom)
	{
		return;
	}

	if (m_ptSelStart.x == m_ptSelEnd.x ||  m_ptSelStart.y == m_ptSelEnd.y)
	{
		return;
	}

	if (IsChart3D())
	{
		CBCGPChartDiagram3D* pDiagram3D = GetDiagram3D();

		if (pDiagram3D == NULL)
		{
			return;
		}

		CBCGPRect rectSel(m_ptSelStart, m_ptSelEnd);
		rectSel.Normalize();

		double dblZoomFactor = pDiagram3D->GetZoomFactor();

		bZoomOut ? dblZoomFactor /= m_mouseConfig.m_nZoomRatio3D : dblZoomFactor *= m_mouseConfig.m_nZoomRatio3D;

		if (!bZoomOut)
		{
			pDiagram3D->SetZoomFactor(dblZoomFactor);
		}

		CBCGPPoint ptSelCenter = rectSel.CenterPoint();
		CBCGPPoint ptDiagramCenter = pDiagram3D->GetDiagramRect().CenterPoint();
		CBCGPPoint ptDiagramCenterInv = ptDiagramCenter;

		pDiagram3D->ScalePointInverse(ptSelCenter, ptDiagramCenter);
		pDiagram3D->ScalePointInverse(ptDiagramCenterInv, ptDiagramCenter);

 		CBCGPPoint ptOffset = ptDiagramCenterInv - ptSelCenter;
 		pDiagram3D->ScalePoint(ptOffset, ptDiagramCenter);

		pDiagram3D->SetScrollOffset(ptOffset - ptDiagramCenter);

		if (bZoomOut)
		{
			pDiagram3D->SetZoomFactor(dblZoomFactor);
		}

		SetDirty(TRUE, TRUE);

		return;
	}

 	double tmp = m_ptSelStart.y;
 	m_ptSelStart.y = m_ptSelEnd.y;
 	m_ptSelEnd.y = tmp;

	CBCGPChartData dataStart = ChartDataFromScreenPoint(m_ptSelStart, CBCGPChartAxis::RT_EXACT, TRUE, TRUE);
	CBCGPChartData dataEnd = ChartDataFromScreenPoint(m_ptSelEnd, CBCGPChartAxis::RT_EXACT, TRUE, FALSE);

	for (int i = 0; i < dataStart.GetComponentCount(); i++)		
	{
		CBCGPChartValue valStart = dataStart.GetValue((CBCGPChartData::ComponentIndex)i);
		CBCGPChartValue valEnd = dataEnd.GetValue((CBCGPChartData::ComponentIndex)i);
		double dblRange = fabs((valEnd - valStart));

		CBCGPChartAxis* pAxis = valStart.GetAxis();

		if (pAxis == NULL)
		{
			continue;
		}

		if (bZoomOut)
		{
			double dblCurrRange = pAxis->GetMaxDisplayedValue() - pAxis->GetMinDisplayedValue();
			double dblRatio = m_mouseConfig.m_nMagnifyFactor + dblRange / dblCurrRange;
			double dblNewRange = dblCurrRange * dblRatio;

			valStart.SetValue(valStart + dblRange / 2 - dblNewRange / 2, pAxis);
			valEnd.SetValue(valStart + dblNewRange, pAxis);

			dataStart.SetValue(valStart, (CBCGPChartData::ComponentIndex)i);
			dataEnd.SetValue(valEnd, (CBCGPChartData::ComponentIndex)i);
		}
		else
		{
			double dblScrollRange = pAxis->GetMaxScrollValue() - pAxis->GetMinScrollValue();
			if (dblRange < dblScrollRange && dblScrollRange / dblRange > pAxis->GetMaxZoomInFactor())
			{
				dataStart.SetValue(pAxis->GetMinDisplayedValue(), (CBCGPChartData::ComponentIndex)i);
				dataEnd.SetValue(pAxis->GetMaxDisplayedValue(), (CBCGPChartData::ComponentIndex)i);
			}
		}
	}

	SetFixedDisplayRange(dataStart, dataEnd, TRUE, TRUE, TRUE);	
}
//****************************************************************************************
void CBCGPChartVisualObject::Zoom(int nMagnifier, const CBCGPPoint& ptZoomCenter)
{
	if (!m_bEnableZoom)
	{
		return;
	}

	if (IsChart3D())
	{
		CBCGPChartDiagram3D* pDiagram3D = GetDiagram3D();

		if (pDiagram3D == NULL)
		{
			return;
		}

		pDiagram3D->SetZoomFactor(pDiagram3D->GetZoomFactor() + nMagnifier / (m_mouseConfig.m_nZoomRatio3D * 5.));
	}
	else	
	{
		BOOL bHasFixedIntervalWidth = FALSE;
		for (int i = 0; i < m_arAxes.GetSize(); i++)
		{
			CBCGPChartAxis* pAxis = m_arAxes[i];

			if (pAxis == NULL || !pAxis->m_bInitialized)
			{
				continue;
			}

			ASSERT_VALID(pAxis);

			CBCGPRect rectBounds = pAxis->GetBoundingRect();

			if (rectBounds.PtInRect(ptZoomCenter) && !pAxis->IsIndependentZoomEnabled())
			{
				pAxis->Zoom(nMagnifier, ptZoomCenter);
			}
			else if (pAxis->IsIndependentZoomEnabled() && pAxis->GetAxisRect().NormalizedRect().PtInRect(ptZoomCenter))
			{
				pAxis->Zoom(nMagnifier, ptZoomCenter);
			}

			if (!bHasFixedIntervalWidth)
			{
				bHasFixedIntervalWidth = pAxis->IsFixedIntervalWidth();
			}
		}

		if (!bHasFixedIntervalWidth)
		{
			RecalcMinMaxValues();
		}
		else
		{
			SetDirty(TRUE, TRUE);
		}
	}
	
	SetDirty(TRUE, TRUE);
}
//****************************************************************************************
void CBCGPChartVisualObject::UnZoom(BOOL bRedraw)
{
	if (IsChart3D())
	{
		CBCGPChartDiagram3D* pDiagram3D = GetDiagram3D();

		if (pDiagram3D == NULL)
		{
			return;
		}

		pDiagram3D->SetZoomFactor(1.);
		pDiagram3D->SetScrollOffset(CBCGPPoint(0, 0));
	}
	else
	{
		if (!m_bEnableZoom)
		{
			return;
		}

		for (int i = 0; i < m_arAxes.GetSize(); i++)
		{
			CBCGPChartAxis* pAxis = m_arAxes[i];

			if (pAxis == NULL || !pAxis->m_bInitialized)
			{
				continue;
			}

			ASSERT_VALID(pAxis);

			pAxis->UnZoom();
		}

		m_bRecalcMinMaxForOptimizedSeries = FALSE;
		RecalcMinMaxValues();
		m_bRecalcMinMaxForOptimizedSeries = TRUE;
	}

	SetDirty(TRUE, bRedraw);
}
//****************************************************************************************
void CBCGPChartVisualObject::SetZoomScrollConfig(BCGPChartMouseConfig::ZoomScrollOptions options, 
												 BCGPChartFormatSelection::SelectionType selType)
{
	m_mouseConfig.SetConfig(options);
	SetSelectionType(selType);

	EnableMagnifier(FALSE);
	EnablePan(FALSE);

	EnableZoom(TRUE);

	switch(options)
	{
	case BCGPChartMouseConfig::ZSO_WHEEL_PAN:
		EnableScroll(TRUE);
		EnablePan();
		break;

	case BCGPChartMouseConfig::ZSO_MAGNIFY:
		EnableScroll(TRUE);
		EnableMagnifier();
		break;

	case BCGPChartMouseConfig::ZSO_SELECT:
		EnableScroll(TRUE);
		break;

	case BCGPChartMouseConfig::ZSO_NONE:
		EnableZoom(FALSE);
		break;
	}
}
//****************************************************************************************
// Visual settings
//****************************************************************************************
void CBCGPChartVisualObject::UpdateSeriesColorIndexes(BOOL bRedraw)
{
	int nColorIndex = 0;

	for (int i = 0; i < GetSeriesCount(); i++)
	{
		CBCGPChartSeries* pSeries = GetSeries(i);

		if (pSeries != NULL)
		{
			pSeries->SetColorIndex(nColorIndex);
			if (pSeries->IsAutoColorDataPoints())
			{
				int nDataPointCount = pSeries->GetDataPointCount();
				for (int j = 0; j < nDataPointCount; j++)
				{
					pSeries->SetDataPointColorIndex(j, nColorIndex++);
				}

				if (nDataPointCount == 0)
				{
					nColorIndex++;
				}
			}
			else
			{
				nColorIndex++;
			}
		}
	}

	m_nLastColorIndex = nColorIndex;

	if (bRedraw)
	{
		Redraw();
	}
}
//****************************************************************************************
void CBCGPChartVisualObject::SetThemeOpacity(int nOpacity)
{
	nOpacity = bcg_clamp(nOpacity, 0, 100);
	m_dblThemeOpacity = nOpacity / 100.;
	m_currentTheme.SetOpacity(m_dblThemeOpacity);
}
//****************************************************************************************
BOOL CBCGPChartVisualObject::GetElementColors(const BCGPChartFormatArea& format, 
								CBCGPChartVisualObject::ChartElement element, 
								CBCGPElementColors& colors) 
{
	BCGPChartFormatArea& fmt = (BCGPChartFormatArea&)format;

	switch(element)
	{
	case CBCGPChartVisualObject::CE_CHART_AREA:
		colors.pBrFill = fmt.m_brFillColor.IsEmpty() ? &m_currentTheme.m_brChartFillColor : &fmt.m_brFillColor;
		colors.pBrLine = fmt.m_outlineFormat.m_brLineColor.IsEmpty() ? &m_currentTheme.m_brChartLineColor : &fmt.m_outlineFormat.m_brLineColor;
		return TRUE;

	case CBCGPChartVisualObject::CE_PLOT_AREA:
		colors.pBrFill = fmt.m_brFillColor.IsEmpty() ? &m_currentTheme.m_brPlotFillColor : &fmt.m_brFillColor;
		colors.pBrLine = fmt.m_outlineFormat.m_brLineColor.IsEmpty() ? &m_currentTheme.m_brPlotLineColor : &fmt.m_outlineFormat.m_brLineColor;
		return TRUE;

	case CBCGPChartVisualObject::CE_CHART_TITLE:
		colors.pBrFill = fmt.m_brFillColor.IsEmpty() ? &m_currentTheme.m_brTitleFillColor : &fmt.m_brFillColor;
		colors.pBrLine = fmt.m_outlineFormat.m_brLineColor.IsEmpty() ? &m_currentTheme.m_brTitleLineColor : &fmt.m_outlineFormat.m_brLineColor;
		return TRUE;

	case CBCGPChartVisualObject::CE_LEGEND:
		colors.pBrFill = fmt.m_brFillColor.IsEmpty() ? &m_currentTheme.m_brLegendFillColor : &fmt.m_brFillColor;
		colors.pBrLine = fmt.m_outlineFormat.m_brLineColor.IsEmpty() ? &m_currentTheme.m_brLegendLineColor : &fmt.m_outlineFormat.m_brLineColor;
		return TRUE;

	case CBCGPChartVisualObject::CE_LEGEND_ENTRY:	
		colors.pBrFill = fmt.m_brFillColor.IsEmpty() ? &m_currentTheme.m_brLegendEntryFillColor : &fmt.m_brFillColor;
		colors.pBrLine = fmt.m_outlineFormat.m_brLineColor.IsEmpty() ? &m_currentTheme.m_brLegendEntryLineColor : &fmt.m_outlineFormat.m_brLineColor;
		return TRUE;

	case CBCGPChartVisualObject::CE_SELECTION:
		colors.pBrFill = fmt.m_brFillColor.IsEmpty() ? &m_currentTheme.m_brSelectionFillColor : &fmt.m_brFillColor;
		colors.pBrLine = fmt.m_outlineFormat.m_brLineColor.IsEmpty() ? &m_currentTheme.m_brSelectionLineColor : &fmt.m_outlineFormat.m_brLineColor;
		return TRUE;

	case CBCGPChartVisualObject::CE_AXIS_LABEL:
		colors.pBrFill = fmt.m_brFillColor.IsEmpty() ? &m_currentTheme.m_brAxisLabelFillColor : &fmt.m_brFillColor;
		colors.pBrLine = fmt.m_outlineFormat.m_brLineColor.IsEmpty() ? &m_currentTheme.m_brAxisLabelLineColor : &fmt.m_outlineFormat.m_brLineColor;
		return TRUE;

	case CBCGPChartVisualObject::CE_AXIS_NAME:
		colors.pBrFill = fmt.m_brFillColor.IsEmpty() ? &m_currentTheme.m_brAxisNameFillColor : &fmt.m_brFillColor;
		colors.pBrLine = fmt.m_outlineFormat.m_brLineColor.IsEmpty() ? &m_currentTheme.m_brAxisNameLineColor : &fmt.m_outlineFormat.m_brLineColor;
		return TRUE;

	}

	return FALSE;
}
//****************************************************************************************
void CBCGPChartVisualObject::SetSeriesShadow(BOOL bSet/* = TRUE*/)
{
	for (int i = 0; i < GetSeriesCount(); i++)
	{
		CBCGPChartSeries* pSeries = GetSeries(i);
		if (pSeries != NULL)
		{
			ASSERT_VALID(pSeries);
			
			BCGPChartFormatSeries style = pSeries->GetSeriesFormat();
			style.m_shadowType.m_bDisplayShadow = bSet;
			pSeries->SetSeriesFormat(style);
		}
	}
}

//****************************************************************************************
// Chart Objects
//****************************************************************************************
void CBCGPChartVisualObject::AddChartObject(CBCGPChartObject* pObj)
{
	m_lstChartObjects.AddTail(pObj);
	pObj->SetParentChart(this);
}
//****************************************************************************************
CBCGPChartObject* CBCGPChartVisualObject::AddChartObject(const CBCGPPoint& ptLeftTopOffset,
											CBCGPChartObject::CoordinateMode mode, BOOL bRelativeToDefaultAxes)
{
	return AddChartObject(CBCGPRect(ptLeftTopOffset, CBCGPChartObject::_EmptyPoint), mode, bRelativeToDefaultAxes);
}
//****************************************************************************************
CBCGPChartObject* CBCGPChartVisualObject::AddChartTextObject(const CBCGPRect& rcCoordinates, const CString& strText, 
														 CBCGPChartObject::CoordinateMode mode, BOOL bRelativeToDefaultAxes)
{
	ASSERT_VALID(this);

	CBCGPChartObject* pChartObject = AddChartObject(rcCoordinates, mode, bRelativeToDefaultAxes);
	pChartObject->m_strText = strText;
	
	return pChartObject;
}
//****************************************************************************************
CBCGPChartAxisMarkObject* CBCGPChartVisualObject::AddChartAxisMarkObject(double dblVal, const CString& strText, 
				BOOL bVertAxis, BOOL bOutside, const CBCGPBrush& brTextColor, const CBCGPBrush& brFill, 
				const CBCGPBrush& brOutline)
{
	ASSERT_VALID(this);

	CBCGPChartAxisMarkObject* pObj = new CBCGPChartAxisMarkObject(this, dblVal, strText, bVertAxis, bOutside, brTextColor, brFill, brOutline);
	AddChartObject(pObj);

	return pObj;
}
//****************************************************************************************
CBCGPChartLineObject* CBCGPChartVisualObject::AddChartLineObject(double dblX1, double dblY1, double dblX2, double dblY2, 
											const CBCGPBrush& brLine, double dblWidth, CBCGPStrokeStyle* pStrokeStyle)
{
	ASSERT_VALID(this);

	CBCGPChartLineObject* pChartObject = new CBCGPChartLineObject(this, dblX1, dblY1, dblX2, dblY2, brLine, dblWidth, pStrokeStyle);
	m_lstChartObjects.AddTail(pChartObject);

	return pChartObject;
}
//****************************************************************************************
CBCGPChartLineObject* CBCGPChartVisualObject::AddChartLineObject(double dblVal, BOOL bHorz, 
							const CBCGPBrush& brLine, double dblWidth, CBCGPStrokeStyle* pStrokeStyle)
{
	ASSERT_VALID(this);

	CBCGPChartLineObject* pChartObject = new CBCGPChartLineObject(this, dblVal, bHorz, brLine, dblWidth, pStrokeStyle);
	m_lstChartObjects.AddTail(pChartObject);

	return pChartObject;
}
//****************************************************************************************
CBCGPChartRangeObject* CBCGPChartVisualObject::AddChartRangeObject(double dblBottomVal, double dblTopVal, BOOL bHorz, const CBCGPBrush& brFill)
{
	ASSERT_VALID(this);

	CBCGPChartRangeObject* pChartObject = new CBCGPChartRangeObject(this, dblBottomVal, dblTopVal, bHorz, brFill);
	m_lstChartObjects.AddTail(pChartObject);

	return pChartObject;
}
//****************************************************************************************
CBCGPChartObject* CBCGPChartVisualObject::AddChartObject(const CBCGPRect& rcCoordinates, CBCGPChartObject::CoordinateMode mode, BOOL bRelativeToDefaultAxes)
{
	ASSERT_VALID(this);

	CBCGPChartObject* pChartObject = new CBCGPChartObject(this, rcCoordinates, mode);

	m_lstChartObjects.AddTail(pChartObject);

	if (bRelativeToDefaultAxes)
	{
		pChartObject->SetRelatedAxes(GetChartAxis(BCGP_CHART_X_PRIMARY_AXIS), GetChartAxis(BCGP_CHART_Y_PRIMARY_AXIS));
	}

	return pChartObject;
}
//****************************************************************************************
CBCGPChartObject* CBCGPChartVisualObject::FindChartObject(int nID) const
{
	for (POSITION pos = m_lstChartObjects.GetHeadPosition(); pos != NULL;)
	{
		CBCGPChartObject* pObj = m_lstChartObjects.GetNext(pos);

		if (pObj != NULL)
		{
			ASSERT_VALID(pObj);

			if (pObj->m_nObjectID == nID)
			{
				return pObj;
			}
		}
	}

	return NULL;
}
//****************************************************************************************
CBCGPChartObject* CBCGPChartVisualObject::FindChartObject(const CString& strName) const
{
	for (POSITION pos = m_lstChartObjects.GetHeadPosition(); pos != NULL;)
	{
		CBCGPChartObject* pObj = m_lstChartObjects.GetNext(pos);

		if (pObj != NULL)
		{
			ASSERT_VALID(pObj);

			if (pObj->m_strObjectName == strName)
			{
				return pObj;
			}
		}
	}

	return NULL;
}
//****************************************************************************************
void CBCGPChartVisualObject::RemoveChartObject(CBCGPChartObject* pObj)
{
	POSITION pos = m_lstChartObjects.Find(pObj);

	if (pos != NULL)
	{
		m_lstChartObjects.RemoveAt(pos);
	}
}
//****************************************************************************************
void CBCGPChartVisualObject::RemoveAllChartObjects()
{
	while(!m_lstChartObjects.IsEmpty())
	{
		delete m_lstChartObjects.RemoveTail();
	}
}
//****************************************************************************************
// Chart effects
//****************************************************************************************
void CBCGPChartVisualObject::OnCalcChartEffectsScreenPositions(CBCGPGraphicsManager* pGM)
{
	if (!IsDirty())
	{
		return;
	}

	for (POSITION pos = m_lstChartEffects.GetHeadPosition(); pos != NULL;)
	{
		CBCGPChartBaseEffect* pObject = m_lstChartEffects.GetNext(pos);

		if (pObject != NULL)
		{
			ASSERT_VALID(pObject);
			pObject->OnCalcScreenPoints(pGM);
		}
	}
}
//****************************************************************************************
void CBCGPChartVisualObject::AddChartEffect(CBCGPChartBaseEffect* pEffect)
{
	ASSERT_VALID(this);

	if (pEffect == NULL)
	{
		return;
	}

	ASSERT_VALID(pEffect);

	m_lstChartEffects.AddTail(pEffect);
}
//****************************************************************************************
void CBCGPChartVisualObject::RemoveAllChartEffects()
{
	ASSERT_VALID(this);

	while (!m_lstChartEffects.IsEmpty())
	{
		delete m_lstChartEffects.RemoveTail();
	}
}
//****************************************************************************************
void CBCGPChartVisualObject::SetThumbnailMode(BOOL bSet/* = TRUE*/, UINT nThumbnailFlags/* = 0*/)
{
	ASSERT_VALID(this);

	m_bIsThumbnailMode = bSet;
	m_nThumbnailFlags = nThumbnailFlags;

	for (int i = 0; i < m_arAxes.GetSize(); i++)
	{
		CBCGPChartAxis* pAxis = m_arAxes[i];
		
		if (pAxis == NULL)
		{
			continue;
		}
		
		ASSERT_VALID(pAxis);

		pAxis->SetThumbnailMode(bSet);
	}

	CBCGPChartDiagram3D* pDiagram3D = GetDiagram3D();
	if (pDiagram3D != NULL)
	{
		pDiagram3D->ResetLabelSize();
	}

	SetDirty();
}
//****************************************************************************************
BOOL CBCGPChartVisualObject::GetGestureConfig(CBCGPGestureConfig& gestureConfig)
{
	if (m_bEnableZoom || m_bEnableMagnifier)
	{
		gestureConfig.EnableZoom();
	}

	if (m_bEnablePan || m_bEnableScroll)
	{
		gestureConfig.EnablePan(TRUE, BCGP_GC_PAN_WITH_SINGLE_FINGER_VERTICALLY | BCGP_GC_PAN_WITH_SINGLE_FINGER_HORIZONTALLY | BCGP_GC_PAN_WITH_INERTIA | BCGP_GC_PAN_WITH_GUTTER);
	}

	return TRUE;
}
//****************************************************************************************
BOOL CBCGPChartVisualObject::OnGestureEventZoom(const CBCGPPoint& ptCenter, double dblZoomFactor)
{
	if (dblZoomFactor == 1.0)
	{
		return TRUE;
	}

	int nMagnifier = 0;

	if (dblZoomFactor < 1.0)
	{
		nMagnifier = (int)(-2.0 * (dblZoomFactor + 1.0));
	}
	else
	{
		nMagnifier = (int)(2.0 * (dblZoomFactor - 1.0));
	}

	Zoom(nMagnifier, ptCenter);
	return TRUE;
}
//****************************************************************************************
BOOL CBCGPChartVisualObject::OnGestureEventPan(const CBCGPPoint& ptFrom, const CBCGPPoint& ptTo, CBCGPSize& sizeOverPan)
{
	const BOOL bScrollVert = fabs(ptFrom.y - ptTo.y) > 1.0;
	const double dblDelta = bScrollVert ? ptFrom.y - ptTo.y : ptFrom.x - ptTo.x;
	BOOL bScrolled = FALSE;
	
	CBCGPChartDiagram3D* pDiagram3D = GetDiagram3D();
	
	if (IsChart3D())
	{
		if (m_pDiagram3D != NULL)
		{
			CBCGPPoint ptScrollOffset = m_pDiagram3D->GetScrollOffset() + ptTo - ptFrom;
			pDiagram3D->SetScrollOffset(ptScrollOffset);

			SetDirty(TRUE, TRUE);

			if (pDiagram3D->GetScrollOffset() != ptScrollOffset)
			{
				bScrolled = TRUE;
			}
		}
	}
	else
	{
		BOOL bXAxisScrolled = FALSE;
		
		for (int i = 0; i < m_arAxes.GetSize(); i++)
		{
			CBCGPChartAxis* pAxis = m_arAxes[i];
			
			if (pAxis == NULL || pAxis->IsVertical() && !bScrollVert || 
				!pAxis->IsVertical() && bScrollVert)
			{
				continue;
			}
			
			if (pAxis->Scroll(dblDelta, TRUE))
			{
				bScrolled = TRUE;
				
				if (pAxis->IsKindOf(RUNTIME_CLASS(CBCGPChartAxisX)))
				{
					bXAxisScrolled = TRUE;
				}
			}
		}
		
		if (bScrolled)
		{
			if (bXAxisScrolled)
			{
				RecalcMinMaxValues();
			}
			
			SetDirty(TRUE, TRUE);
		}
	}

	if (!bScrolled)
	{
		sizeOverPan.cx = bScrollVert ? 0.0 : -dblDelta;
		sizeOverPan.cy = bScrollVert ? -dblDelta : 0.0;
	}

	return TRUE;
}
//***********************************************************************************************************
static void ApplyDefaultFontFamily(CBCGPTextFormat& tf, const CString& strFontFamily, const CString& strOldFontFamily)
{
	if (!strOldFontFamily.IsEmpty() && tf.GetFontFamily() == strOldFontFamily)
	{
		tf.SetFontFamily(strFontFamily);
	}
}
//***********************************************************************************************************
void CBCGPChartVisualObject::UpdateAllChartsDefaultFont(const CString& strFontFamily, BOOL bApplyToExistingCharts)
{
	if (BCGPChartFormatLabel::m_strDefaultFontFamily == strFontFamily)
	{
		return;
	}
	
	CString strOldFontFamily = BCGPChartFormatLabel::m_strDefaultFontFamily;
	BCGPChartFormatLabel::m_strDefaultFontFamily = strFontFamily;

	if (bApplyToExistingCharts)
	{
		for (POSITION pos = m_lstCharts.GetHeadPosition(); pos != NULL;)
		{
			CBCGPChartVisualObject* pChart = m_lstCharts.GetNext(pos);
			ASSERT_VALID(pChart);

			pChart->UpdateDefaultFont(strFontFamily, strOldFontFamily);
		}
	}
}
//***********************************************************************************************************
void CBCGPChartVisualObject::UpdateDefaultFont(const CString& strFontFamily, const CString& strOldName)
{
	ASSERT_VALID(this);

	ApplyDefaultFontFamily(m_titleAreaFormat.m_textFormat, strFontFamily, strOldName);
	ApplyDefaultFontFamily(m_dataTableAreaFormat.m_labelFormat.m_textFormat, strFontFamily, strOldName);

	int i = 0;

	for (i = 0; i < (int)m_arAxes.GetSize(); i++)
	{
		CBCGPChartAxis* pAxis = m_arAxes[i];
		if (pAxis != NULL)
		{
			ASSERT_VALID(pAxis);

			ApplyDefaultFontFamily(pAxis->m_axisLabelsFormat.m_textFormat, strFontFamily, strOldName);
			ApplyDefaultFontFamily(pAxis->m_axisNameFormat.m_textFormat, strFontFamily, strOldName);
		}
	}

	for (i = 0; i < (int)m_arData.GetSize(); i++)
	{
		CBCGPChartSeries* pSeries = GetSeries(i, FALSE);
		if (pSeries != NULL)
		{
			ASSERT_VALID(pSeries);

			ApplyDefaultFontFamily(pSeries->m_formatSeries.m_dataLabelFormat.m_textFormat, strFontFamily, strOldName);
			ApplyDefaultFontFamily(pSeries->m_formatSeries.m_legendLabelFormat.m_textFormat, strFontFamily, strOldName);

			for (int iDP = 0; iDP < (int)pSeries->m_arDataPoints.GetSize(); iDP++)
			{
				CBCGPChartDataPoint* pDP = pSeries->m_arDataPoints[iDP];
				if (pDP != NULL && pDP->m_pFormatElement != NULL)
				{
					ApplyDefaultFontFamily(pDP->m_pFormatElement->m_dataLabelFormat.m_textFormat, strFontFamily, strOldName);
				}
			}
		}
	}

	for (POSITION posLegend = m_lstRelatedLegends.GetHeadPosition(); posLegend != NULL;)
	{
		CBCGPChartLegendVisualObject* pLegend = DYNAMIC_DOWNCAST(CBCGPChartLegendVisualObject, m_lstRelatedLegends.GetNext(posLegend));
		if (pLegend != NULL)
		{
			ASSERT_VALID(pLegend);

			ApplyDefaultFontFamily(pLegend->m_legendStyle.m_titleFormat.m_textFormat, strFontFamily, strOldName);
			pLegend->InvalidateLegendContent(TRUE);
		}
	}

	for (POSITION posObject = m_lstChartObjects.GetHeadPosition(); posObject != NULL;)
	{
		CBCGPChartObject* pObj = m_lstChartObjects.GetNext(posObject);
		
		if (pObj != NULL)
		{
			ASSERT_VALID(pObj);
			ApplyDefaultFontFamily(pObj->m_format.m_textFormat, strFontFamily, strOldName);
		}
	}

	SetDirty(TRUE, TRUE);
}
//***********************************************************************************************************
void CBCGPChartVisualObject::OnDrawChartDataTable(CBCGPGraphicsManager* pGM, CBCGPRect& rectDataTable, double dblDataTableHeaderColumnWidth)
{
	ASSERT_VALID(pGM);

	CBCGPChartAxis* pAxis = m_arAxes.GetAt(BCGP_CHART_X_PRIMARY_AXIS);
	if (pAxis == NULL)
	{
		return;
	}

	const CBCGPBrush& brGridLineHorz = m_dataTableAreaFormat.m_horizontalGridLinesFormat.m_brLineColor.IsEmpty() ?
		m_currentTheme.m_brAxisMajorGridLineColor : m_dataTableAreaFormat.m_horizontalGridLinesFormat.m_brLineColor;
	
	const CBCGPBrush& brGridLineVert = m_dataTableAreaFormat.m_verticalGridLinesFormat.m_brLineColor.IsEmpty() ?
		m_currentTheme.m_brAxisMajorGridLineColor : m_dataTableAreaFormat.m_verticalGridLinesFormat.m_brLineColor;

	const CBCGPBrush& brOutline = m_dataTableAreaFormat.m_outlineFormat.m_brLineColor.IsEmpty() ?
		m_currentTheme.m_brAxisLineColor : m_dataTableAreaFormat.m_outlineFormat.m_brLineColor;

	double dblOutlineLineWidth = m_dataTableAreaFormat.m_outlineFormat.GetLineWidth(TRUE);

	const CBCGPStrokeStyle& outlineStrokeStyle = m_dataTableAreaFormat.m_outlineFormat.m_strokeStyle;

	if (m_dataTableAreaFormat.m_bInterlaceRows && m_dataTableAreaFormat.m_brInterlaceFill.IsEmpty())
	{
		// Create from the chart color theme:
		m_dataTableAreaFormat.m_brInterlaceFill = m_dataTableAreaFormat.m_brFill.IsEmpty() ?
			m_currentTheme.m_brChartFillColor : m_dataTableAreaFormat.m_brFill;

		if (m_dataTableAreaFormat.m_brInterlaceFill.GetColor().IsPale())
		{
			m_dataTableAreaFormat.m_brInterlaceFill.MakeDarker(.02);
		}
		else
		{
			m_dataTableAreaFormat.m_brInterlaceFill.MakeLighter(.1);
		}
	}

	if (!m_dataTableAreaFormat.m_brFill.IsEmpty())
	{
		pGM->FillRectangle(rectDataTable, m_dataTableAreaFormat.m_brFill);
	}

	int i = 0;
	int nRow = 0;
	double y = rectDataTable.top;
	double yLineTop = m_rectDiagramArea.bottom;
	double xLineRight = rectDataTable.right;

	if (pAxis->m_arMajorGridLines.GetSize() > 0)
	{
		int nIndex = pAxis->m_bReverseOrder ? 0 : (int)pAxis->m_arMajorGridLines.GetSize() - 1;
		xLineRight = min(xLineRight, pAxis->m_arMajorGridLines[nIndex].CenterPoint().x);
	}

	if (!pAxis->m_rectScrollBar.IsRectEmpty())
	{
		yLineTop = pAxis->m_rectScrollBar.bottom;
	}

	//-----------------
	// Draw table rows:
	//-----------------
	for (i = 0; i < GetSeriesCount(); i++)
	{
		CBCGPChartSeries* pSeries = GetSeries(i, FALSE);
		if (pSeries == NULL || !pSeries->IsDataTableEntryVisible())
		{
			continue;
		}

		CBCGPRect rectRow(rectDataTable.left, y, xLineRight, y + m_dblDataTableRowHeight);

		if (m_dataTableAreaFormat.m_bInterlaceRows && (nRow % 2) == 0 && !m_dataTableAreaFormat.m_brInterlaceFill.IsEmpty())
		{
			pGM->FillRectangle(rectRow, m_dataTableAreaFormat.m_brInterlaceFill);
		}

		if (dblDataTableHeaderColumnWidth > 0.0)
		{
			CBCGPRect rectLegend = rectRow;
			rectLegend.right = rectLegend.left + dblDataTableHeaderColumnWidth;

			pGM->SetClipRect(rectLegend);

			OnDrawChartDataTableLegendItem(pGM, rectLegend, pSeries, nRow);

			pGM->ReleaseClipArea();
		}

		CBCGPRect rectAxis(pAxis->GetAxisRect(FALSE, FALSE, FALSE));
		int nDPIndex = -1;
		int nFirstIndex = -1;
		CBCGPPoint ptFirst;

		int nStart = pAxis->m_bReverseOrder ? max(0, pSeries->GetDataPointCount() - 1) : 0;
		int nFinish = pAxis->m_bReverseOrder ? 0 : max(0, (int)pSeries->GetDataPointCount() - 1);
		int offsetIndex = pAxis->m_bReverseOrder ? -pAxis->m_nValuesPerInterval : pAxis->m_nValuesPerInterval;

		int offset = pAxis->m_bReverseOrder ? -1 : 1;

		int j = 0;

		for (j = nStart; j != nFinish; j += offset)
		{
			const CBCGPChartDataPoint* pDP = pSeries->GetDataPointAt(j);
			if (pDP != NULL)
			{
				BOOL bIsEmpty = FALSE;
				ptFirst = ScreenPointFromChartData(pDP->GetData(), j, bIsEmpty, pSeries);
			
				if (rectAxis.left <= ptFirst.x && ptFirst.x <= rectAxis.right)
				{
					nFirstIndex = j;
					break;
				}
			}
		}

		nStart = pAxis->m_bReverseOrder ? (int)pAxis->m_arMajorGridLines.GetSize() - 1 : 0;
		nFinish = pAxis->m_bReverseOrder ? 0 : (int)pAxis->m_arMajorGridLines.GetSize() - 1;

		for (j = nStart; j != nFinish; j += offset)
		{
			double x1 = pAxis->m_arMajorGridLines[j].CenterPoint().x;
			double x2 = pAxis->m_arMajorGridLines[j + offset].CenterPoint().x;

			CBCGPRect rectCell(x1, y, x2, y + m_dblDataTableRowHeight);

			if (nDPIndex == -1)
			{
				if (pAxis->m_arMajorGridLines[j].left < ptFirst.x && ptFirst.x < pAxis->m_arMajorGridLines[j + offset].right)
				{
					nDPIndex = nFirstIndex;
				}
			}

			if (nDPIndex >= 0 && nDPIndex < pSeries->GetDataPointCount())
			{
				pGM->SetClipRect(rectCell);
				OnDrawChartDataTableEntry(pGM, rectCell, pSeries, nDPIndex, nRow);
				pGM->ReleaseClipArea();

				nDPIndex += offsetIndex;
			}
		}

		y += m_dblDataTableRowHeight;

		int iLastLineIndex = m_dataTableAreaFormat.m_bDrawOutline ? GetSeriesCount() - 2 : GetSeriesCount() - 1;

		if (m_dataTableAreaFormat.m_bDrawHorizontalGridLines && i <= iLastLineIndex)
		{
			pGM->DrawLine(rectDataTable.left, y, xLineRight, y, brGridLineHorz,
				m_dataTableAreaFormat.m_horizontalGridLinesFormat.GetLineWidth(TRUE),
				&m_dataTableAreaFormat.m_horizontalGridLinesFormat.m_strokeStyle);
		}

		nRow++;
	}

	//---------------------
	// Draw vertical lines:
	//---------------------
	for (i = 0; i < (int)pAxis->m_arMajorGridLines.GetSize(); i++)
	{
		double x = pAxis->m_arMajorGridLines[i].CenterPoint().x;

		if (i == 0 || i == (int)pAxis->m_arMajorGridLines.GetSize() - 1)
		{
			if (m_dataTableAreaFormat.m_bDrawOutline)
			{
				pGM->DrawLine(x, yLineTop, x, y, brOutline, dblOutlineLineWidth, &outlineStrokeStyle);
			}
		}
		else
		{
			if (m_dataTableAreaFormat.m_bDrawVerticalGridLines)
			{
				pGM->DrawLine(x, yLineTop, x, y, brGridLineVert,
					m_dataTableAreaFormat.m_verticalGridLinesFormat.GetLineWidth(TRUE),
					&m_dataTableAreaFormat.m_verticalGridLinesFormat.m_strokeStyle);
			}
		}
	}

	//---------------
	// Draw outlines:
	//---------------
	if (m_dataTableAreaFormat.m_bDrawOutline)
	{
		pGM->DrawLine(rectDataTable.left, rectDataTable.top, xLineRight, rectDataTable.top, brOutline, dblOutlineLineWidth, &outlineStrokeStyle);
		pGM->DrawLine(rectDataTable.left, rectDataTable.top, rectDataTable.left, y, brOutline, dblOutlineLineWidth, &outlineStrokeStyle);
		pGM->DrawLine(rectDataTable.left, y, xLineRight, y, brOutline, dblOutlineLineWidth, &outlineStrokeStyle);
	}
}
//***********************************************************************************************************
void CBCGPChartVisualObject::OnDrawChartDataTableLegendItem(CBCGPGraphicsManager* pGM, const CBCGPRect& rectLegend, 
											CBCGPChartSeries* pSeries, int nRow)
{
	ASSERT_VALID(pGM);
	ASSERT_VALID(pSeries);

	UNREFERENCED_PARAMETER(nRow);

	const BCGPChartFormatSeries& seriesFormat = pSeries->GetSeriesFormat();
	
	const CBCGPBrush& brText = m_dataTableAreaFormat.m_labelFormat.m_brTextColor.IsEmpty() ? 
		m_currentTheme.m_brAxisLabelTextColor : m_dataTableAreaFormat.m_labelFormat.m_brTextColor;

	const CBCGPTextFormat& tf = m_dataTableAreaFormat.m_labelFormat.m_textFormat.IsEmpty() ?
		seriesFormat.m_dataLabelFormat.m_textFormat : m_dataTableAreaFormat.m_labelFormat.m_textFormat;
	
	double xLabel = rectLegend.left;
	double dx = m_dataTableAreaFormat.GetContentPadding(TRUE).cx;
	double y = rectLegend.top;
	
	if (m_dataTableAreaFormat.m_bShowLegendKeys)
	{
		CBCGPSize sizeKey = OnCalcLegendKeySize(pSeries, -1);

		if (!sizeKey.IsEmpty())
		{
			sizeKey.cx = m_dblMaxSeriesKeyWidth;

			CBCGPRect rectKey(CBCGPPoint(xLabel + dx, y + max(0, (m_dblDataTableRowHeight - sizeKey.cy) / 2)), sizeKey);
			OnDrawChartLegendKey(pGM, rectKey, &seriesFormat, pSeries, -1);
		
			xLabel = rectKey.right;
		}
	}
	
	CBCGPRect rectLabel(xLabel + dx, y + max(0, (m_dblDataTableRowHeight - rectLegend.Height()) / 2), rectLegend.right - dx, rectLegend.bottom);

	CString strLabel;
	pSeries->GetDataTableName(strLabel);

	pGM->DrawText(strLabel, rectLabel, tf, brText);
}
//***********************************************************************************************************
void CBCGPChartVisualObject::OnDrawChartDataTableEntry(CBCGPGraphicsManager* pGM, const CBCGPRect& rectCell, CBCGPChartSeries* pSeries, int nDPIndex, int nRow)
{
	ASSERT_VALID(pGM);
	ASSERT_VALID(pSeries);

	UNREFERENCED_PARAMETER(nRow);

	const CBCGPBrush& brText = m_dataTableAreaFormat.m_labelFormat.m_brTextColor.IsEmpty() ? 
		m_currentTheme.m_brAxisLabelTextColor : m_dataTableAreaFormat.m_labelFormat.m_brTextColor;
	const CBCGPTextFormat& tf = m_dataTableAreaFormat.m_labelFormat.m_textFormat.IsEmpty() ?
		pSeries->GetSeriesFormat().m_dataLabelFormat.m_textFormat : m_dataTableAreaFormat.m_labelFormat.m_textFormat;
	
	CBCGPRect rectLabel = rectCell;
	rectLabel.DeflateRect(m_dataTableAreaFormat.GetContentPadding(TRUE).cx, m_dataTableAreaFormat.GetContentPadding(TRUE).cy);

	int nPrecision = -1;
	CString strDataLabel;
	BOOL bFitted = FALSE;

	while (TRUE)
	{
		strDataLabel.Empty();

		if (!pSeries->GetDataPointTableText(nDPIndex, strDataLabel, nPrecision))
		{
			return;
		}

		if (pGM->GetTextSize(strDataLabel, tf).cx <= rectLabel.Width())
		{
			bFitted = TRUE;
			break;
		}

		if (nPrecision == -1)
		{
			if (!m_dataTableAreaFormat.m_bRoundValues)
			{
				break;
			}

			nPrecision = 5;
		}
		else
		{
			nPrecision--;
			if (nPrecision < 0)
			{
				break;
			}
		}
	}
	
	BOOL bUseDefaultTextFormat = TRUE;

	if (tf.GetTextAlignment() != CBCGPTextFormat::BCGP_TEXT_ALIGNMENT_LEADING && !bFitted)
	{
		CBCGPTextFormat tfLeft = tf;
		tfLeft.SetTextAlignment(CBCGPTextFormat::BCGP_TEXT_ALIGNMENT_LEADING);

		if (m_tfDataTableLeft != tfLeft)
		{
			m_tfDataTableLeft = tfLeft;
		}

		pGM->DrawText(strDataLabel, rectLabel, m_tfDataTableLeft, brText);

		bUseDefaultTextFormat = FALSE;
	}

	if (bUseDefaultTextFormat)
	{
		pGM->DrawText(strDataLabel, rectLabel, tf, brText);
	}
}
