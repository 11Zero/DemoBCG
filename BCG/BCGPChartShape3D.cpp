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
// BCGPChartShape3D.cpp : implements functionality of shapes used for 3D charts
//

#include "stdafx.h"
#include "BCGPChartShape3D.h"
#include "BCGPChartSeries.h"
#include "BCGPChartVisualObject.h"
#include "BCGPChartAxis.h"

#include "float.h"
#include "BCGPMath.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CBCGPChartShape3D, CObject)
IMPLEMENT_DYNCREATE(CBCGPChartShape3DRect, CBCGPChartShape3D)
IMPLEMENT_DYNCREATE(CBCGPChartShape3DCube, CBCGPChartShape3D)
IMPLEMENT_DYNCREATE(CBCGPChartShape3DWall, CBCGPChartShape3DCube)

CBCGPChartEdge3DList	CBCGPChartShape3DRect::m_lst3DRectEdges;
BOOL			CBCGPChartShape3DRect::m_b3DRectShapeInitilaized = FALSE;

CBCGPChartEdge3DList CBCGPChartShape3DCube::m_lst3DCubeEdges;
BOOL			CBCGPChartShape3DCube::m_b3DCubeShapeInitilaized;

CBCGPMatrix			CBCGPChartShape3DRect::m_m3DRectVertexIndexes;
CBCGPMatrix			CBCGPChartShape3DCube::m_m3DCubeVertexIndexes;

const int CBCGPChartShape3DCube::CUBE_SIDE_BACK = 0;
const int CBCGPChartShape3DCube::CUBE_SIDE_TOP = 1;
const int CBCGPChartShape3DCube::CUBE_SIDE_LEFT = 2;
const int CBCGPChartShape3DCube::CUBE_SIDE_RIGHT = 3;
const int CBCGPChartShape3DCube::CUBE_SIDE_BOTTOM = 4;
const int CBCGPChartShape3DCube::CUBE_SIDE_FRONT = 5;

const int CBCGPChartShape3DWall::WALL_POS_FLOOR = 0;
const int CBCGPChartShape3DWall::WALL_POS_LEFT = 1;
const int CBCGPChartShape3DWall::WALL_POS_BACK = 2;
const int CBCGPChartShape3DWall::WALL_POS_RIGHT = 3;
const int CBCGPChartShape3DWall::WALL_POS_FRONT = 4;

const double CBCGPChartSide3D::COLLISION_COEF = 100.;


//*******************************************************************************
// A Side (facet) of 3D shape
//*******************************************************************************
void CBCGPChartSide3D::ResetDrawingResources()
{
	m_pBrFill = NULL;
	m_pBrLine = NULL;
	m_pOutlineFormat = NULL;
}
//*******************************************************************************
void CBCGPChartSide3D::CopyFrom(const CBCGPChartSide3D& src)
{
	m_nSideIndex = src.m_nSideIndex;
	m_pShape = src.m_pShape;
	m_bVisible = src.m_bVisible;
	m_bIgnored = src.m_bIgnored;
	m_dblAverageZOrder = src.m_dblAverageZOrder;
	m_rectMinMaxPoints = src.m_rectMinMaxPoints;
	m_dblMinZOrder = src.m_dblMinZOrder;
	m_dblMaxZOrder = src.m_dblMaxZOrder;
	m_vPlaneCoef = src.m_vPlaneCoef;
	m_vNormal = src.m_vNormal;

	m_nAxisID = src.m_nAxisID;
	m_nAxisIDGrid1 = src.m_nAxisIDGrid1;
	m_nAxisIDGrid2 = src.m_nAxisIDGrid2;
	m_bIsAxisGrid1Near = src.m_bIsAxisGrid1Near;
	m_bIsAxisGrid2Near = src.m_bIsAxisGrid2Near;
	m_bIsWallSide = src.m_bIsWallSide;

	m_pBrFill = src.m_pBrFill;
	m_pBrLine = src.m_pBrLine;
	m_pOutlineFormat = src.m_pOutlineFormat;
}
//*******************************************************************************
double CBCGPChartSide3D::CalcZ(const CBCGPPoint& pt) const
{
	if (fabs(m_vPlaneCoef[2]) < DBL_EPSILON)
	{
		return m_dblAverageZOrder;
	}
	return -(m_vPlaneCoef[0] * pt.x + m_vPlaneCoef[1] * pt.y + m_vPlaneCoef[3]) / m_vPlaneCoef[2];
}
//*******************************************************************************
BOOL CBCGPChartSide3D::HasCollision(const CBCGPChartSide3D& side, CBCGPPoint& ptCenterCollision) const
{
	if (m_pShape == NULL || side.m_pShape == NULL || 
		m_pShape == side.m_pShape && m_bVisible == side.m_bVisible || m_bIgnored || side.m_bIgnored)
	{
		return FALSE;
	}

	ptCenterCollision.SetPoint(0, 0);

 	const CBCGPPtArrayArray& arPtPreparedThis = m_pShape->GetPreparedSidePoints();
 	const CBCGPPtArrayArray& arPtPreparedOther = side.m_pShape->GetPreparedSidePoints();

#ifdef _DEBUG
	if (arPtPreparedThis.GetSize() == 0 || arPtPreparedOther.GetSize() == 0)
	{
		return FALSE;
	}
#endif

 	const CBCGPPointsArray* pArThis = &(arPtPreparedThis.GetData()[m_nSideIndex]);
 	const CBCGPPointsArray* pArOther = &(arPtPreparedOther.GetData()[side.m_nSideIndex]);

	if (!BCGPCalculateIntersectPoint(*pArThis, *pArOther, ptCenterCollision))
	{
 		return FALSE;
	}

	ptCenterCollision /= COLLISION_COEF;

 	return TRUE;
}
//*******************************************************************************
void CBCGPChartSide3D::OnDraw(CBCGPEngine3D* pGM, BOOL bFill, BOOL bDrawLine)
{
	if (pGM == NULL)
	{
		return;
	}

	ASSERT_VALID(pGM);

	if (m_pShape == NULL || m_bIgnored || m_bDrawn)
	{
		return;
	}

	m_bDrawn = TRUE;

	for (POSITION pos = m_lstSidesBefore.GetHeadPosition(); pos != NULL;)
	{
		CBCGPChartSide3D* pSide = m_lstSidesBefore.GetNext(pos);
		pSide->OnDraw(pGM, bFill, bDrawLine);
	}

	m_lstSidesBefore.RemoveAll();

	if (bFill && !m_bVisible)
	{
		bFill = FALSE;
	}

	if (m_pBrFill == NULL || m_pBrLine == NULL || m_pOutlineFormat == NULL)
	{
		m_pShape->GetDrawingResources(m_nSideIndex, &m_pBrFill, &m_pBrLine, &m_pOutlineFormat);
	}

	if (m_pBrFill == NULL || m_pBrLine == NULL || m_pOutlineFormat == NULL)
	{
		return;
	}

	CBCGPPointsArray arVertexes;
	m_pShape->GetSidePoints(arVertexes, m_nSideIndex, CBCGPChartShape3D::SVT_FINAL);

	if (m_bIsWallSide)
	{
		CBCGPChartVisualObject* pChart = m_pShape->GetParentChart();
		CBCGPChartShape3DWall* pWall = DYNAMIC_DOWNCAST(CBCGPChartShape3DWall, m_pShape);

		if (pChart != NULL && pWall != NULL)
		{
			CBCGPChartDiagram3D* pDiagram3D = pChart->GetDiagram3D();

			if (pDiagram3D != NULL)
			{
				int nWallPos = pWall->GetWallPosition();
				CBCGPChartDiagram3D::DrawWallOptions dwo = pDiagram3D->GetDrawWallOptions();

				if (nWallPos == CBCGPChartShape3DWall::WALL_POS_FLOOR)
				{
					if (((dwo & CBCGPChartDiagram3D::DWO_FILL_FLOOR) == 0))
					{
						bFill = FALSE;
					}

					if (((dwo & CBCGPChartDiagram3D::DWO_OUTLINE_FLOOR) == 0))
					{
						bDrawLine = FALSE;
					}
				}
				else if (nWallPos == pDiagram3D->GetLeftVisibleWallPos())
				{
					if (((dwo & CBCGPChartDiagram3D::DWO_FILL_LEFT_WALL) == 0))
					{
						bFill = FALSE;
					}

					if (((dwo & CBCGPChartDiagram3D::DWO_OUTLINE_LEFT_WALL) == 0))
					{
						bDrawLine = FALSE;
					}
				}
				else if (nWallPos == pDiagram3D->GetRightVisibleWallPos())
				{
					if (((dwo & CBCGPChartDiagram3D::DWO_FILL_RIGHT_WALL) == 0))
					{
						bFill = FALSE;
					}

					if (((dwo & CBCGPChartDiagram3D::DWO_OUTLINE_RIGHT_WALL) == 0))
					{
						bDrawLine = FALSE;
					}
				}
			}
		}
	}

	CBCGPBrush brEmpty;
	CBCGPBrush& brFill = bFill ? *m_pBrFill : brEmpty;
	CBCGPBrush& brLine = bDrawLine ? *m_pBrLine : brEmpty;
	
	if (arVertexes.GetSize() == 4)
	{
		pGM->SetPolygonNormal(m_vNormal[0], m_vNormal[1], m_vNormal[2]);
		if (!m_bIsWallSide)
		{
			pGM->DrawSide(arVertexes[0], arVertexes[1], arVertexes[2], arVertexes[3], 
					brFill, brLine, m_pOutlineFormat->m_dblWidth, bFill, bDrawLine);
		}
		else
		{
			CBCGPChartShape3DWall* pWall = DYNAMIC_DOWNCAST(CBCGPChartShape3DWall, m_pShape);
			pGM->m_bForceDisableDepthTest = pWall->GetWallPosition() != CBCGPChartShape3DWall::WALL_POS_FLOOR;
			pGM->DrawSide(arVertexes[0], arVertexes[1], arVertexes[2], arVertexes[3], 
					brFill, brEmpty, m_pOutlineFormat->m_dblWidth, bFill, FALSE);
			pGM->m_bForceDisableDepthTest = FALSE;
		}
	}

	if (m_bIsWallSide)
	{
		CBCGPChartVisualObject* pChart = m_pShape->GetParentChart();

		if (pChart == NULL)
		{
			return;
		}

		CBCGPChartAxis* pAxis = NULL; // axis to draw
		CBCGPChartAxis* pAxis1 = NULL; // axis for grid line
		CBCGPChartAxis* pAxis2 = NULL; // second axis for grid line

		if (m_nAxisID != -1)
		{
			pAxis = m_pShape->GetParentChart()->GetChartAxis(m_nAxisID);
		}
		
		if (bDrawLine)
		{
			CBCGPChartDiagram3D* pDiagram3D = pChart->GetDiagram3D();		

			if (pDiagram3D != NULL)
			{
				pDiagram3D->DrawLinesCheckAxes(arVertexes, brLine, m_pOutlineFormat->m_dblWidth);
			}
		}
		
		if (pAxis != NULL)
		{
			pAxis->OnDrawAxisOnThickWall(pGM, this);
		}

		if (m_nAxisIDGrid1 != -1)
		{
			pAxis1 = m_pShape->GetParentChart()->GetChartAxis(m_nAxisIDGrid1);
		}

		if (m_nAxisIDGrid2 != -1)
		{
			pAxis2 = m_pShape->GetParentChart()->GetChartAxis(m_nAxisIDGrid2);
		}

		if (pAxis1 != NULL)
		{
			pAxis1->OnDrawInterlaceOnThickWall(pGM, this, m_bIsAxisGrid1Near);
		}

		if (pAxis2 != NULL)
		{
			pAxis2->OnDrawInterlaceOnThickWall(pGM, this, m_bIsAxisGrid2Near);
		}

		if (pAxis1 != NULL)
		{
			pAxis1->OnDrawGridLinesOnThickWall(pGM, this, m_bIsAxisGrid1Near);
		}

		if (pAxis2 != NULL)
		{
			pAxis2->OnDrawGridLinesOnThickWall(pGM, this, m_bIsAxisGrid2Near);
		}
	}
}
//*******************************************************************************
BOOL CBCGPChartSide3D::HitTest(const CBCGPPoint& ptHit, CBCGPChartShape3D** ppShape)
{
	if (ppShape == NULL)
	{
		return FALSE;
	}

	*ppShape = NULL;

	CBCGPPointsArray arVertexes;
	m_pShape->GetSidePoints(arVertexes, m_nSideIndex, CBCGPChartShape3D::SVT_FINAL);

	CBCGPRect rect = arVertexes.GetBoundsRect().NormalizedRect();

	if (rect.PtInRect(ptHit))
	{
		*ppShape = m_pShape;
		return TRUE;
	}

	for (POSITION pos = m_lstSidesBefore.GetTailPosition(); pos != NULL;)
	{
		CBCGPChartSide3D* pSide = m_lstSidesBefore.GetPrev(pos);

		if (pSide != NULL)
		{
			if (pSide->HitTest(ptHit, ppShape) && *ppShape != NULL)
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}
//*******************************************************************************
void CBCGPChartSide3D::ResetState(BOOL bDrawFlagOnly)
{
	m_bDrawn = FALSE;

	if (!bDrawFlagOnly)
	{
		m_lstSidesBefore.RemoveAll();
	}
}

//*******************************************************************************
// Base shape implementation
//*******************************************************************************
CBCGPChartShape3D::CBCGPChartShape3D()
{
	m_pSeries = NULL;
	m_nDataPointIndex = -1;
	m_dblMaxZOrder = -DBL_MAX;
	m_dblMinZOrder = DBL_MAX;
	m_bVisible = TRUE;
	m_pParentChart = NULL;
}
//*******************************************************************************
void CBCGPChartShape3D::CopyFrom(const CBCGPChartShape3D& src)
{
	m_arOriginalVertexes.RemoveAll();
	m_arOriginalVertexes.Append(src.GetVertexes());

	SetFinalVertexes(src.GetFinalVertexes());
	SetRotatedVertexes(src.GetRotatedVertexes());

	m_pSeries = src.m_pSeries;
	m_nDataPointIndex = src.m_nDataPointIndex;

	m_sides.RemoveAll();
	m_sides.Append(src.GetSides());

	m_bVisible = src.m_bVisible;

	for (int i = 0; i < m_sides.GetSize(); i++)
	{
		m_sides[i].m_pShape = this;
	}

	m_dblMinZOrder = src.m_dblMinZOrder;
	m_dblMaxZOrder = src.m_dblMaxZOrder;

	m_pParentChart = src.m_pParentChart;
}
//*******************************************************************************
CBCGPPoint CBCGPChartShape3D::GetVertex(int nIndex, ShapeVertexType svt) const
{
	if (nIndex >= m_arOriginalVertexes.GetSize() || nIndex < 0)
	{
		ASSERT(FALSE);
		return CBCGPPoint();
	}

	switch(svt)
	{
	case CBCGPChartShape3D::SVT_ORIGINAL:
		return m_arOriginalVertexes[nIndex];

	case CBCGPChartShape3D::SVT_ROTATED:
		return m_arRotatedVertexes[nIndex];

	case CBCGPChartShape3D::SVT_FINAL:
		return m_arFinalVertexes[nIndex];
	}

	return CBCGPPoint();
}
//*******************************************************************************
void CBCGPChartShape3D::GetSidePoints(CBCGPPointsArray& arSidePoints, int nSideIndex, 
								ShapeVertexType svt) const
{
	arSidePoints.RemoveAll();

	if (m_arOriginalVertexes.GetSize() == 0)
	{
		return;
	}

	
	// matrix row corresponds to side index
	// matrix column corresponds to vertex index on that side
	const CBCGPMatrix& mVertexIndexes = GetVertexIndexes();

	for (int i = 0; i < mVertexIndexes.GetCols(); i++)
	{
		arSidePoints.Add(GetVertex((int)mVertexIndexes[nSideIndex][i], svt));
	}
}
//*******************************************************************************
CBCGPChartVisualObject* CBCGPChartShape3D::GetParentChart() const
{
	if (m_pParentChart != NULL)
	{
		return m_pParentChart;
	}

	return NULL;
}
//*******************************************************************************
void CBCGPChartShape3D::SetSideIgnored(int nSideIndex, BOOL bIgnored)
{
	CBCGPChartSide3D& side = m_sides[nSideIndex];
	side.m_bIgnored = bIgnored;
	m_sides[nSideIndex] = side;
}
//*******************************************************************************
void CBCGPChartShape3D::OnAfterTransform(CBCGPChartDiagram3D* pDiagram)
{
	m_dblMinZOrder = DBL_MAX;
	m_dblMaxZOrder = -DBL_MAX;

	if (pDiagram == NULL)
	{
		return;
	}

	double dblYRotation = pDiagram->GetYRotation();
	CBCGPPoint ptCenter = pDiagram->GetDiagramRect().CenterPoint();

	for (int i = 0; i < m_sides.GetSize(); i++)
	{
		CBCGPPointsArray arPoints;
		GetSidePoints(arPoints, i, CBCGPChartShape3D::SVT_FINAL);

		CBCGPPointsArray arPointsRotated;
		GetSidePoints(arPointsRotated, i, CBCGPChartShape3D::SVT_ROTATED);

		m_sides[i].m_vPlaneCoef.CalcPlane(arPointsRotated);

		if (arPoints.GetSize() > 2)
		{
			m_sides[i].m_vNormal.CalcNormal(arPoints[0], arPoints[1], arPoints[2]);
		}

		m_dblMinZOrder = arPointsRotated[0].z;
		

 		double dblAverageZOrder = 0;
 		double dblMinZOrder = DBL_MAX;
 		double dblMaxZOrder = -DBL_MAX;

		for (int j = 0; j < arPointsRotated.GetSize(); j++)
		{
			const CBCGPPoint& ptNext = arPointsRotated[j];
			
			dblAverageZOrder += ptNext.z;
			dblMinZOrder = min(dblMinZOrder, ptNext.z);
			dblMaxZOrder = max(dblMaxZOrder, ptNext.z);
		}
 		
		m_sides[i].m_rectMinMaxPoints = arPoints.GetBoundsRect();
 		m_sides[i].m_dblAverageZOrder = dblAverageZOrder / arPoints.GetSize();	
 		m_sides[i].m_dblMinZOrder = dblMinZOrder;
 		m_sides[i].m_dblMaxZOrder = dblMaxZOrder;

		m_sides[i].m_bVisible = dblYRotation >= -90 && dblYRotation <= 90 ? 
			m_sides[i].m_vPlaneCoef[2] > 0 : 
			m_sides[i].m_vPlaneCoef[2] < 0;

 		m_dblMinZOrder = min(dblMinZOrder, m_dblMinZOrder);
 		m_dblMaxZOrder = max(m_dblMaxZOrder, dblMaxZOrder);
	}
}
//*******************************************************************************
void CBCGPChartShape3D::ResetDrawingResources()
{
	for (int i = 0; i < m_sides.GetSize(); i++)
	{
		m_sides[i].ResetDrawingResources();
	}
}
//*******************************************************************************
void CBCGPChartShape3D::ResetSideData()
{
	for (int i = 0; i < m_sides.GetSize(); i++)
	{
		m_sides[i].m_nAxisID = -1;
		m_sides[i].m_nAxisIDGrid1 = -1;
		m_sides[i].m_nAxisIDGrid2 = -1;
		m_sides[i].m_bIsAxisGrid1Near = FALSE;
		m_sides[i].m_bIsAxisGrid2Near = FALSE;
	}
}
//*******************************************************************************
void CBCGPChartShape3D::GetDrawingResources(int /*nSideIndex*/, CBCGPBrush** ppBrFill, CBCGPBrush** ppBrLine, BCGPChartFormatLine** ppOutlineFormat) const
{
	if (ppBrFill == NULL || ppBrLine == NULL || ppOutlineFormat == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	BCGPSeriesColorsPtr colors;
	const BCGPChartFormatSeries* pFormatSeries = m_pSeries->GetColors(colors, m_nDataPointIndex);
	if (pFormatSeries == NULL)
	{
		return;
	}
	
	*ppBrFill = colors.m_pBrElementFillColor;
	*ppBrLine = colors.m_pBrElementLineColor;
	*ppOutlineFormat = (BCGPChartFormatLine*) &pFormatSeries->m_seriesElementFormat.m_outlineFormat;
}
//*******************************************************************************
void CBCGPChartShape3D::AddSidesToList(CList<CBCGPChartSide3D*, CBCGPChartSide3D*>& lst, BOOL bVisible)
{
	if (!m_bVisible)
	{
		return;
	}

	for (int i = 0; i < m_sides.GetSize(); i++)
	{
		CBCGPChartSide3D& side = m_sides[i];

		if (side.m_bVisible && bVisible || !side.m_bVisible && !bVisible)
		{
			lst.AddTail(&side);
		}
	}
}
//*******************************************************************************
void CBCGPChartShape3D::PrepareSidePoints(ShapeVertexType svt)
{
	if (m_arPreparedSidePoints.GetSize() == 0)
	{
		m_arPreparedSidePoints.SetSize(GetSideCount());
	}

	for (int i = 0; i < GetSideCount(); i++)
	{
		CBCGPPointsArray* pArThis = &(m_arPreparedSidePoints.GetData()[i]);
		pArThis->RemoveAll();

		GetSidePoints(*pArThis, i, svt);

		for (int i = 0; i < pArThis->GetSize(); i++)
		{
			float x = (float)((*pArThis)[i].x * CBCGPChartSide3D::COLLISION_COEF);
			float y = (float)((*pArThis)[i].y * CBCGPChartSide3D::COLLISION_COEF);
			(*pArThis)[i] = CBCGPPoint(x, y);
		}

		pArThis->StoreBoundsRect(pArThis->GetBoundsRect().NormalizedRect());
	}
}
//*******************************************************************************
// A rectangle in 3D space
//*******************************************************************************
CBCGPChartShape3DRect::CBCGPChartShape3DRect()
{
	InitEdgesAndSides();
}
//*******************************************************************************
CBCGPChartShape3DRect::CBCGPChartShape3DRect(const CBCGPRect& rect, double dblZ)
{
	InitEdgesAndSides();

	m_arOriginalVertexes.SetSize(4);

	// rt, lt, lb, rb
	CBCGPPoint pt(rect.right, rect.top, dblZ);
	m_arOriginalVertexes[0] = pt;

	pt.y = rect.left;
	m_arOriginalVertexes[1] = pt;

	pt.x = rect.bottom;
	m_arOriginalVertexes[2] = pt;

	pt.y = rect.right;
	m_arOriginalVertexes[3] = pt;
}
//*******************************************************************************
// bSetFinalvertexes = TRUE used for surface chart, to save on costly operation
// of additional transformation. Original vertexes in this case are empty - do not refer them.
//*******************************************************************************
CBCGPChartShape3DRect::CBCGPChartShape3DRect(const CBCGPPoint& ptLeftTop, const CBCGPPoint& ptRightTop, 
		const CBCGPPoint& ptLeftBottom, const CBCGPPoint& ptRightBottom, BOOL bSetFinalVertexes)
{
	InitEdgesAndSides();
	SetVertexes(ptLeftTop, ptRightTop, ptLeftBottom, ptRightBottom, bSetFinalVertexes);
}
CBCGPChartShape3DRect::~CBCGPChartShape3DRect()
{
}
//*******************************************************************************
void CBCGPChartShape3DRect::SetVertexes(const CBCGPPoint& ptLeftTop, const CBCGPPoint& ptRightTop, 
				 const CBCGPPoint& ptLeftBottom, const CBCGPPoint& ptRightBottom, BOOL bSetFinalVertexes)
{
	CBCGPPointsArray& arVertexes = bSetFinalVertexes ? m_arFinalVertexes : m_arOriginalVertexes;

	arVertexes.SetSize(4);

	arVertexes[0] = ptRightTop;
	arVertexes[1] = ptLeftTop;
	arVertexes[2] = ptLeftBottom;
	arVertexes[3] = ptRightBottom;
}
//*******************************************************************************
void CBCGPChartShape3DRect::InitEdgesAndSides()
{
	if (!m_b3DRectShapeInitilaized)
	{
		m_lst3DRectEdges.AddTail(CBCGPChartEdge3D(0, 1));
		m_lst3DRectEdges.AddTail(CBCGPChartEdge3D(1, 2));
		m_lst3DRectEdges.AddTail(CBCGPChartEdge3D(2, 3));
		m_lst3DRectEdges.AddTail(CBCGPChartEdge3D(3, 0));

		m_m3DRectVertexIndexes.Create(1, 4, 0);
		m_m3DRectVertexIndexes[0][0] = 0;
		m_m3DRectVertexIndexes[0][1] = 1;
		m_m3DRectVertexIndexes[0][2] = 2;
		m_m3DRectVertexIndexes[0][3] = 3;

		m_b3DRectShapeInitilaized = TRUE;
	}

	m_sides.RemoveAll();
	m_sides.Add(CBCGPChartSide3D(0, this));
}
//*******************************************************************************
void CBCGPChartShape3DRect::OnDraw(CBCGPEngine3D* pGM)
{
	if (m_pSeries == NULL || !m_bVisible)
	{
		return;
	}

	CBCGPBrush*				pBrFill = NULL; 
	CBCGPBrush*				pBrLine = NULL; 
	BCGPChartFormatLine*	pOutlineFormat = NULL;

	GetDrawingResources(0, &pBrFill, &pBrLine, &pOutlineFormat);

	CBCGPPolygonGeometry g(m_arFinalVertexes);
	g.SetTemporary();

	if (pBrFill != NULL)
	{
		pGM->FillGeometry(g, *pBrFill);
	}

	if (pBrLine != NULL && pOutlineFormat != NULL)
	{
		for (POSITION pos = m_lst3DRectEdges.GetHeadPosition(); pos != NULL;)
		{
			const CBCGPChartEdge3D& edge = m_lst3DRectEdges.GetNext(pos);
			pGM->DrawLine(m_arFinalVertexes[edge.m_nIndex1], m_arFinalVertexes[edge.m_nIndex2], *pBrLine, 
				pOutlineFormat->m_dblWidth);
		}
	}
}
//*******************************************************************************
// 3D Cube (parallelepiped) implementation
//*******************************************************************************
CBCGPChartShape3DCube::CBCGPChartShape3DCube()
{
	InitEdgesAndSides();
}
//*******************************************************************************
CBCGPChartShape3DCube::CBCGPChartShape3DCube(const CBCGPRect& rect, double dblZ, double dblDepth)
{
	InitEdgesAndSides();
	SetOriginalVertexes(rect, dblZ, dblDepth);
}
//*******************************************************************************
CBCGPChartShape3DCube::CBCGPChartShape3DCube(const CBCGPPoint& ptLeft, const CBCGPPoint& ptRight, 
											 double dblHeight, double dblZ, double dblDepth)
{
	InitEdgesAndSides();
	SetOriginalVertexes(ptLeft, ptRight, dblHeight, dblZ, dblDepth);
}
//*******************************************************************************
CBCGPChartShape3DCube::CBCGPChartShape3DCube(const CBCGPPoint& ptLeftTop, const CBCGPPoint& ptRightTop, 
							const CBCGPPoint& ptLeftBottom, const CBCGPPoint& ptRightBottom, 
							double dblZ, double dblDepth)
{
	InitEdgesAndSides();
	SetOriginalVertexes(ptLeftTop, ptRightTop, ptLeftBottom, ptRightBottom, dblZ, dblDepth);
}
//*******************************************************************************
void CBCGPChartShape3DCube::SetOriginalVertexes(const CBCGPRect& rect, double dblZ, double dblDepth)
{
	if (dblDepth < 0)
	{
		dblDepth = rect.Width();
	}

	m_arOriginalVertexes.SetSize(8);

	double dblFrontPlaneZ = dblZ - dblDepth / 2;
	double dblBackPlaneZ = dblZ + dblDepth / 2;

	// front plane - lt, lb, rb, rt
	CBCGPPoint pt(rect.left, rect.top, dblFrontPlaneZ);
	m_arOriginalVertexes[0] = pt;

	pt.y = rect.bottom;
	m_arOriginalVertexes[1] = pt;

	pt.x = rect.right;
	m_arOriginalVertexes[2] = pt;

	pt.y = rect.top;
	m_arOriginalVertexes[3] = pt;

	// back plane - lt, lb, rb, rt
	pt.SetPoint(rect.left, rect.top, dblBackPlaneZ);

	m_arOriginalVertexes[4] = pt;

	pt.y = rect.bottom;
	m_arOriginalVertexes[5] = pt;

	pt.x = rect.right;
	m_arOriginalVertexes[6] = pt;

	pt.y = rect.top;
	m_arOriginalVertexes[7] = pt;
}	
//*******************************************************************************
void CBCGPChartShape3DCube::SetOriginalVertexes(const CBCGPPoint& ptLeft, const CBCGPPoint& ptRight, double dblHeight, 
							double dblZ, double dblDepth)
{
	if (dblDepth < 0)
	{
		dblDepth = ptRight.x - ptLeft.x;
	}

	m_arOriginalVertexes.SetSize(8);

	double dblFrontPlaneZ = dblZ - dblDepth / 2;
	double dblBackPlaneZ = dblZ + dblDepth / 2;

 	// front plane - lt, lb, rb, rt
 	CBCGPPoint pt(ptLeft.x, ptLeft.y + dblHeight / 2, dblFrontPlaneZ);
 	m_arOriginalVertexes[0] = pt;

 	pt.y = ptLeft.y - dblHeight / 2;
 	m_arOriginalVertexes[1] = pt;

 	pt.x = ptRight.x;
 	pt.y = ptRight.y - dblHeight / 2;
 	m_arOriginalVertexes[2] = pt;

 	pt.y = ptRight.y + dblHeight / 2;
 	m_arOriginalVertexes[3] = pt;

 	// back plane - lt, lb, rb, rt
 	pt.SetPoint(ptLeft.x, ptLeft.y + dblHeight / 2, dblBackPlaneZ);

 	m_arOriginalVertexes[4] = pt;

 	pt.y = ptLeft.y - dblHeight / 2;
 	m_arOriginalVertexes[5] = pt;

 	pt.x = ptRight.x;
 	pt.y = ptRight.y - dblHeight / 2;
 	m_arOriginalVertexes[6] = pt;

 	pt.y = ptRight.y + dblHeight / 2;
 	m_arOriginalVertexes[7] = pt;
}
//*******************************************************************************
void CBCGPChartShape3DCube::SetOriginalVertexes(const CBCGPPoint& ptLeftTop, const CBCGPPoint& ptRightTop, 
							const CBCGPPoint& ptLeftBottom, const CBCGPPoint& ptRightBottom, 
							double dblZ, double dblDepth)
{
	if (dblDepth < 0)
	{
		dblDepth = ptRightTop.x - ptLeftTop.x;
	}

	m_arOriginalVertexes.SetSize(8);

	double dblFrontPlaneZ = dblZ - dblDepth / 2;
	double dblBackPlaneZ = dblZ + dblDepth / 2;

	// front plane - lt, lb, rb, rt
	CBCGPPoint pt = ptLeftTop;
	pt.z = dblFrontPlaneZ;
	m_arOriginalVertexes[0] = pt;

	pt = ptLeftBottom;
	pt.z = dblFrontPlaneZ;
	m_arOriginalVertexes[1] = pt;

	pt = ptRightBottom;
	pt.z = dblFrontPlaneZ;
	m_arOriginalVertexes[2] = pt;

	pt = ptRightTop;
	pt.z = dblFrontPlaneZ;
	m_arOriginalVertexes[3] = pt;

	// back plane - lt, lb, rb, rt
	pt = ptLeftTop;
	pt.z = dblBackPlaneZ;
	m_arOriginalVertexes[4] = pt;

	pt = ptLeftBottom;
	pt.z = dblBackPlaneZ;
	m_arOriginalVertexes[5] = pt;

	pt = ptRightBottom;
	pt.z = dblBackPlaneZ;
	m_arOriginalVertexes[6] = pt;

	pt = ptRightTop;
	pt.z = dblBackPlaneZ;
	m_arOriginalVertexes[7] = pt;
}
//*******************************************************************************
void CBCGPChartShape3DCube::CopyFrom(const CBCGPChartShape3D& src)
{
	CBCGPChartShape3D::CopyFrom(src);
	m_bFillLeftAsTop = ((CBCGPChartShape3DCube&)src).IsFillLeftAsTop();
	m_bFillLeftAsBottom = ((CBCGPChartShape3DCube&)src).IsFillLeftAsBottom();
}
//*******************************************************************************
void CBCGPChartShape3DCube::Flip(BOOL bTopBottom)
{
	CBCGPPoint ptTmp;

	if (bTopBottom)
	{
		ptTmp = m_arOriginalVertexes[0];
		m_arOriginalVertexes[0] = m_arOriginalVertexes[1];
		m_arOriginalVertexes[1] = ptTmp;

		ptTmp = m_arOriginalVertexes[2];
		m_arOriginalVertexes[2] = m_arOriginalVertexes[3];
		m_arOriginalVertexes[3] = ptTmp;

		ptTmp = m_arOriginalVertexes[4];
		m_arOriginalVertexes[4] = m_arOriginalVertexes[5];
		m_arOriginalVertexes[5] = ptTmp;

		ptTmp = m_arOriginalVertexes[6];
		m_arOriginalVertexes[6] = m_arOriginalVertexes[7];
		m_arOriginalVertexes[7] = ptTmp;

	}
	else
	{
		ptTmp = m_arOriginalVertexes[0];
		m_arOriginalVertexes[0] = m_arOriginalVertexes[3];
		m_arOriginalVertexes[3] = ptTmp;

		ptTmp = m_arOriginalVertexes[1];
		m_arOriginalVertexes[1] = m_arOriginalVertexes[2];
		m_arOriginalVertexes[2] = ptTmp;

		ptTmp = m_arOriginalVertexes[5];
		m_arOriginalVertexes[5] = m_arOriginalVertexes[6];
		m_arOriginalVertexes[6] = ptTmp;

		ptTmp = m_arOriginalVertexes[4];
		m_arOriginalVertexes[4] = m_arOriginalVertexes[7];
		m_arOriginalVertexes[7] = ptTmp;
	}
}
//*******************************************************************************
void CBCGPChartShape3DCube::InitEdgesAndSides()
{
	if (!m_b3DCubeShapeInitilaized)
	{
		// front plane
		m_lst3DCubeEdges.AddTail(CBCGPChartEdge3D(0, 1));
		m_lst3DCubeEdges.AddTail(CBCGPChartEdge3D(1, 2));
		m_lst3DCubeEdges.AddTail(CBCGPChartEdge3D(2, 3));
		m_lst3DCubeEdges.AddTail(CBCGPChartEdge3D(3, 0));

		// back plane
		m_lst3DCubeEdges.AddTail(CBCGPChartEdge3D(4, 5));
		m_lst3DCubeEdges.AddTail(CBCGPChartEdge3D(5, 6));
		m_lst3DCubeEdges.AddTail(CBCGPChartEdge3D(6, 7));
		m_lst3DCubeEdges.AddTail(CBCGPChartEdge3D(7, 4));

		// left plane
		m_lst3DCubeEdges.AddTail(CBCGPChartEdge3D(0, 4));
		m_lst3DCubeEdges.AddTail(CBCGPChartEdge3D(1, 5));

		// right plane
		m_lst3DCubeEdges.AddTail(CBCGPChartEdge3D(3, 7));
		m_lst3DCubeEdges.AddTail(CBCGPChartEdge3D(2, 6));

		// The cube is flipped up down after conversion! Therefore top non-converted vertex
		// corresponds to bottom of converted vertex

		m_m3DCubeVertexIndexes.Create(6, 4);

		m_m3DCubeVertexIndexes[CUBE_SIDE_TOP][0] = 0;
		m_m3DCubeVertexIndexes[CUBE_SIDE_TOP][1] = 4;
		m_m3DCubeVertexIndexes[CUBE_SIDE_TOP][2] = 7;
		m_m3DCubeVertexIndexes[CUBE_SIDE_TOP][3] = 3;

		m_m3DCubeVertexIndexes[CUBE_SIDE_BOTTOM][0] = 2;
		m_m3DCubeVertexIndexes[CUBE_SIDE_BOTTOM][1] = 6;
		m_m3DCubeVertexIndexes[CUBE_SIDE_BOTTOM][2] = 5;
		m_m3DCubeVertexIndexes[CUBE_SIDE_BOTTOM][3] = 1;

		// lb, lt, rt, rb
		m_m3DCubeVertexIndexes[CUBE_SIDE_FRONT][0] = 1;
		m_m3DCubeVertexIndexes[CUBE_SIDE_FRONT][1] = 0;
		m_m3DCubeVertexIndexes[CUBE_SIDE_FRONT][2] = 3;
		m_m3DCubeVertexIndexes[CUBE_SIDE_FRONT][3] = 2;

		// lt, lb, rb, rt
		m_m3DCubeVertexIndexes[CUBE_SIDE_BACK][0] = 4;
		m_m3DCubeVertexIndexes[CUBE_SIDE_BACK][1] = 5;
		m_m3DCubeVertexIndexes[CUBE_SIDE_BACK][2] = 6;
		m_m3DCubeVertexIndexes[CUBE_SIDE_BACK][3] = 7;

		// lb, rb, rt, lt
		m_m3DCubeVertexIndexes[CUBE_SIDE_LEFT][0] = 1;
		m_m3DCubeVertexIndexes[CUBE_SIDE_LEFT][1] = 5;
		m_m3DCubeVertexIndexes[CUBE_SIDE_LEFT][2] = 4;
		m_m3DCubeVertexIndexes[CUBE_SIDE_LEFT][3] = 0;

		// lt, rt, rb, lb
		m_m3DCubeVertexIndexes[CUBE_SIDE_RIGHT][0] = 3;
		m_m3DCubeVertexIndexes[CUBE_SIDE_RIGHT][1] = 7;
		m_m3DCubeVertexIndexes[CUBE_SIDE_RIGHT][2] = 6;
		m_m3DCubeVertexIndexes[CUBE_SIDE_RIGHT][3] = 2;

		m_b3DCubeShapeInitilaized = TRUE;
	}

	m_sides.RemoveAll();
	m_sides.SetSize(6);
	
	m_sides[CUBE_SIDE_TOP] = CBCGPChartSide3D(CUBE_SIDE_TOP, this);
	m_sides[CUBE_SIDE_BOTTOM] = CBCGPChartSide3D(CUBE_SIDE_BOTTOM, this);
	m_sides[CUBE_SIDE_FRONT] = CBCGPChartSide3D(CUBE_SIDE_FRONT, this);
	m_sides[CUBE_SIDE_BACK] = CBCGPChartSide3D(CUBE_SIDE_BACK, this);
	m_sides[CUBE_SIDE_LEFT] = CBCGPChartSide3D(CUBE_SIDE_LEFT, this);
	m_sides[CUBE_SIDE_RIGHT] = CBCGPChartSide3D(CUBE_SIDE_RIGHT, this);

	m_bFillLeftAsTop = FALSE;
	m_bFillLeftAsBottom = FALSE;
}
//*******************************************************************************
void CBCGPChartShape3DCube::GetDrawingResources(int nSideIndex, CBCGPBrush** ppBrFill, CBCGPBrush** ppBrLine, BCGPChartFormatLine** ppOutlineFormat) const
{
	ASSERT_VALID(this);

	if (ppBrFill == NULL || ppBrLine == NULL || ppOutlineFormat == NULL)
	{
		return;
	}

	BCGPSeriesColorsPtr colors;
	const BCGPChartFormatSeries* pFormatSeries = m_pSeries->GetColors(colors, m_nDataPointIndex);
	if (pFormatSeries == NULL)
	{
		return;
	}
	
	*ppBrFill = colors.m_pBrElementFillColor;
	*ppBrLine = colors.m_pBrElementLineColor;
	*ppOutlineFormat = (BCGPChartFormatLine*) &pFormatSeries->m_seriesElementFormat.m_outlineFormat;

	switch (nSideIndex)
	{
	case CUBE_SIDE_TOP:
		*ppBrFill = colors.m_pBrElementBottomFillColor;
		break;

	case CUBE_SIDE_BOTTOM:
		*ppBrFill = colors.m_pBrElementTopFillColor;
		break;

	case CUBE_SIDE_LEFT:
		if (m_bFillLeftAsTop)
		{
			*ppBrFill = colors.m_pBrElementTopFillColor;
		}
		else if (m_bFillLeftAsBottom)
		{
			*ppBrFill = colors.m_pBrElementBottomFillColor;
		}
		else
		{
			*ppBrFill = colors.m_pBrElementSideFillColor;
		}
		break;

	case CUBE_SIDE_RIGHT:
		*ppBrFill = colors.m_pBrElementSideFillColor;
		break;

	case CUBE_SIDE_FRONT:
	case CUBE_SIDE_BACK:
		// already set
		break;
	}
}
//*******************************************************************************
void CBCGPChartShape3DCube::OnDraw(CBCGPEngine3D* pGM)
{
	ASSERT_VALID(this);

	if (!m_bVisible)
	{
		return;
	}

	for (int j = 0; j < 2; j++)
	{
		for (int i = 0; i < m_sides. GetSize(); i++)
		{
			CBCGPChartSide3D& side = m_sides[i];
			
			if (j == 0 && side.m_bVisible || j == 1 && !side.m_bVisible)
			{
				continue;
			}

			side.OnDraw(pGM, TRUE, TRUE);
		}
	}
}
//*******************************************************************************
// Special 3D diagram wall implementation
//*******************************************************************************
CBCGPChartShape3DWall::CBCGPChartShape3DWall()
{
	m_nWallPosition = -1;

	InitEdgesAndSides();
}
//*******************************************************************************
void CBCGPChartShape3DWall::SetVertexes(int nWallPosition, double dblWallThickness)
{
	if (m_pParentChart == NULL)
	{
		return;
	}

	CBCGPChartDiagram3D* pDiagram3D = m_pParentChart->GetDiagram3D();

	if (pDiagram3D == NULL)
	{
		return;
	}

	CBCGPPoint ptLeftTop;
	CBCGPPoint ptRightTop;
	CBCGPPoint ptLeftBottom;
	CBCGPPoint ptRightBottom;
	double dblZ = 0.;
	double dblDepth = 2.;

	static BOOL bNoGaps = TRUE;

	switch (nWallPosition)
	{
	case WALL_POS_FLOOR:
		ptLeftTop.SetPoint(-1, -1);
		ptRightTop.SetPoint(1, -1);
		ptLeftBottom.SetPoint(-1, -1 - dblWallThickness);
		ptRightBottom.SetPoint(1, -1 - dblWallThickness);
		break;
		
	case WALL_POS_LEFT:
		ptLeftTop.SetPoint(-1 - dblWallThickness, 1);
		ptRightTop.SetPoint(-1, 1);
		if (bNoGaps)
		{
			ptLeftBottom.SetPoint(-1 - dblWallThickness, -1 - dblWallThickness);
			ptRightBottom.SetPoint(-1, -1 - dblWallThickness);
		}
		else
		{
			ptLeftBottom.SetPoint(-1 - dblWallThickness, -1);
			ptRightBottom.SetPoint(-1, -1);
		}
		break;

	case WALL_POS_RIGHT:
		ptLeftTop.SetPoint(1, 1);
		ptRightTop.SetPoint(1 + dblWallThickness, 1);
		if (bNoGaps)
		{
			ptLeftBottom.SetPoint(1, -1 - dblWallThickness);
			ptRightBottom.SetPoint(1 + dblWallThickness, -1 - dblWallThickness);
		}
		else
		{
			ptLeftBottom.SetPoint(1, -1);
			ptRightBottom.SetPoint(1 + dblWallThickness, -1);
		}
		break;

	case WALL_POS_BACK:
	case WALL_POS_FRONT:
		if (bNoGaps)
		{
			if (pDiagram3D->IsWallVisible(WALL_POS_LEFT))
			{
				ptLeftTop.SetPoint(-1 - dblWallThickness, 1);
				ptLeftBottom.SetPoint(-1 - dblWallThickness, -1 - dblWallThickness);
			}
			else
			{
				ptLeftTop.SetPoint(-1, 1);
				ptLeftBottom.SetPoint(-1, -1 - dblWallThickness);
			}

			if (pDiagram3D->IsWallVisible(WALL_POS_RIGHT))
			{
				ptRightTop.SetPoint(1 + dblWallThickness, 1);
				ptRightBottom.SetPoint(1 + dblWallThickness, -1 - dblWallThickness);
			}
			else
			{
				ptRightTop.SetPoint(1, 1);
				ptRightBottom.SetPoint(1, -1 - dblWallThickness);
			}
		}
		else
		{
			ptLeftTop.SetPoint(-1, 1);
			ptLeftBottom.SetPoint(-1, -1);
			ptRightTop.SetPoint(1, 1);
			ptRightBottom.SetPoint(1, -1);
		}

		if (nWallPosition == WALL_POS_BACK)
		{
			dblZ = 1. + dblWallThickness / 2;
			dblDepth = dblWallThickness;
		}
		else
		{
			dblZ = -1. - dblWallThickness / 2;
			dblDepth = dblWallThickness;
		}
		break;
	}

	SetOriginalVertexes(ptLeftTop, ptRightTop, ptLeftBottom, ptRightBottom, dblZ, dblDepth);

	for (int i = 0; i < m_sides.GetSize(); i++)
	{
		m_sides[i].m_bIsWallSide = TRUE;
	}

	Flip(TRUE);

	m_nWallPosition = nWallPosition;

	double dblBaseDepthPercent = pDiagram3D->GetBaseDepthPercent();
	OnSetBaseDepthPercent(dblBaseDepthPercent);
}
//*******************************************************************************
void CBCGPChartShape3DWall::OnSetBaseDepthPercent(double dblBaseDepthPercent)
{
	for (int i = 0; i < m_arOriginalVertexes.GetSize(); i++)
	{
		CBCGPPoint pt = m_arOriginalVertexes[i];
		pt.z *= dblBaseDepthPercent;
		m_arOriginalVertexes[i] = pt;
	}
}
//*******************************************************************************
void CBCGPChartShape3DWall::GetDrawingResources(int nSideIndex, CBCGPBrush** ppBrFill, CBCGPBrush** ppBrLine, BCGPChartFormatLine** ppOutlineFormat) const
{
	if (ppBrFill == NULL || ppBrLine == NULL || ppOutlineFormat == NULL || m_pParentChart == NULL)
	{
		return;
	}

	CBCGPChartTheme& theme = (CBCGPChartTheme&)m_pParentChart->GetColors();
	CBCGPChartDiagram3D* pDiagram3D = m_pParentChart->GetDiagram3D();

	if (pDiagram3D == NULL)
	{
		return;
	}

	switch (m_nWallPosition)
	{
	case WALL_POS_FLOOR:
		*ppOutlineFormat = &pDiagram3D->m_formatWalls.m_outlineFormatFloor;
		*ppBrLine = (CBCGPBrush*)(pDiagram3D->m_formatWalls.m_outlineFormat.m_brLineColor.IsEmpty() ? 
					&theme.m_brAxisMajorGridLineColor : &pDiagram3D->m_formatWalls.m_outlineFormatFloor.m_brLineColor);
		break;
	case WALL_POS_LEFT:
	case WALL_POS_RIGHT:
		*ppOutlineFormat = &pDiagram3D->m_formatWalls.m_outlineFormatLeftWall;
		*ppBrLine = (CBCGPBrush*)(pDiagram3D->m_formatWalls.m_outlineFormat.m_brLineColor.IsEmpty() ? 
					&theme.m_brAxisMajorGridLineColor : &pDiagram3D->m_formatWalls.m_outlineFormatLeftWall.m_brLineColor);
		break;
	case WALL_POS_FRONT:
	case WALL_POS_BACK:
		*ppOutlineFormat = &pDiagram3D->m_formatWalls.m_outlineFormat;
		*ppBrLine = (CBCGPBrush*)(pDiagram3D->m_formatWalls.m_outlineFormat.m_brLineColor.IsEmpty() ? 
					&theme.m_brAxisMajorGridLineColor : &pDiagram3D->m_formatWalls.m_outlineFormat.m_brLineColor);
		break;

	}

	switch (nSideIndex)
	{
	case CUBE_SIDE_TOP:
		if (m_nWallPosition == WALL_POS_FLOOR)
		{
			*ppBrFill = pDiagram3D->m_formatWalls.m_brFloorFillColorBottom3D.IsEmpty() ? 
				&theme.m_brFloorColor3DBottom : &pDiagram3D->m_formatWalls.m_brFloorFillColorBottom3D;
		}
		else if (m_nWallPosition == WALL_POS_LEFT || m_nWallPosition == WALL_POS_RIGHT)
		{
			*ppBrFill = pDiagram3D->m_formatWalls.m_brLeftWallFillColorBottom3D.IsEmpty() ? 
				&theme.m_brLeftWallColor3DBottom : &pDiagram3D->m_formatWalls.m_brLeftWallFillColorBottom3D;
		}
		else if (m_nWallPosition == WALL_POS_FRONT || m_nWallPosition == WALL_POS_BACK)
		{
			*ppBrFill = pDiagram3D->m_formatWalls.m_brRightWallFillColorBottom3D.IsEmpty() ?
				&theme.m_brRightWallColor3DBottom : &pDiagram3D->m_formatWalls.m_brRightWallFillColorBottom3D;
		}
		break;

	case CUBE_SIDE_BOTTOM:
		if (m_nWallPosition == WALL_POS_FLOOR)
		{
			*ppBrFill = pDiagram3D->m_formatWalls.m_brFloorFillColorTop3D.IsEmpty() ? 
				&theme.m_brFloorColor3DTop : &pDiagram3D->m_formatWalls.m_brFloorFillColorTop3D;
		}
		else if (m_nWallPosition == WALL_POS_LEFT || m_nWallPosition == WALL_POS_RIGHT)
		{
			*ppBrFill = pDiagram3D->m_formatWalls.m_brLeftWallFillColorTop3D.IsEmpty() ? 
				&theme.m_brLeftWallColor3DTop : &pDiagram3D->m_formatWalls.m_brLeftWallFillColorTop3D;
		}
		else if (m_nWallPosition == WALL_POS_FRONT || m_nWallPosition == WALL_POS_BACK)
		{
			*ppBrFill = pDiagram3D->m_formatWalls.m_brRightWallFillColorTop3D.IsEmpty() ? 
				&theme.m_brRightWallColor3DTop : &pDiagram3D->m_formatWalls.m_brRightWallFillColorTop3D;
		}
		break;

	case CUBE_SIDE_LEFT:
	case CUBE_SIDE_RIGHT:
		if (m_nWallPosition == WALL_POS_FLOOR)
		{
			*ppBrFill = pDiagram3D->m_formatWalls.m_brFloorFillColorSide3D.IsEmpty() ? 
				&theme.m_brFloorColor3DSide : &pDiagram3D->m_formatWalls.m_brFloorFillColorSide3D;
		}
		else if (m_nWallPosition == WALL_POS_LEFT || m_nWallPosition == WALL_POS_RIGHT)
		{
			*ppBrFill = pDiagram3D->m_formatWalls.m_brLeftWallFillColorSide3D.IsEmpty() ?
				&theme.m_brLeftWallColor3DSide : &pDiagram3D->m_formatWalls.m_brLeftWallFillColorSide3D;
		}
		else if (m_nWallPosition == WALL_POS_FRONT || m_nWallPosition == WALL_POS_BACK)
		{
			*ppBrFill = pDiagram3D->m_formatWalls.m_brRightWallFillColorSide3D.IsEmpty() ?
				&theme.m_brRightWallColor3DSide : &pDiagram3D->m_formatWalls.m_brRightWallFillColorSide3D;
		}
		break;

	case CUBE_SIDE_BACK:
	case CUBE_SIDE_FRONT:
		if (m_nWallPosition == WALL_POS_FLOOR)
		{
			*ppBrFill = pDiagram3D->m_formatWalls.m_brBottomFillColor.IsEmpty() ? 
				&theme.m_brFloorColor3D : &pDiagram3D->m_formatWalls.m_brBottomFillColor;
		}
		else if (m_nWallPosition == WALL_POS_LEFT || m_nWallPosition == WALL_POS_RIGHT)
		{
			*ppBrFill = pDiagram3D->m_formatWalls.m_brFillColor.IsEmpty() ? 
				&theme.m_brLeftWallColor3D : &pDiagram3D->m_formatWalls.m_brSideFillColor;
		}
		else if (m_nWallPosition == WALL_POS_FRONT || m_nWallPosition == WALL_POS_BACK)
		{
			*ppBrFill = pDiagram3D->m_formatWalls.m_brSideFillColor.IsEmpty() ? 
				&theme.m_brRightWallColor3D : &pDiagram3D->m_formatWalls.m_brFillColor;
		}
		break;
	}
}
//*******************************************************************************
BOOL CBCGPChartShape3DWall::GetAxisCoordinates(int nAxisID, CBCGPPoint& ptStart, CBCGPPoint& ptEnd)
{
	if (m_pParentChart == NULL)
	{
		return FALSE;
	}

	CBCGPChartAxis* pAxis = NULL;
	int nSideIndex = -1;

	for (int i = 0; i < m_sides.GetSize(); i++)
	{
		CBCGPChartSide3D& side = m_sides[i];

		if (side.m_nAxisID != -1)
		{
			pAxis = m_pParentChart->GetChartAxis(side.m_nAxisID);

			if (pAxis != NULL && pAxis->m_nAxisID == nAxisID)
			{
				nSideIndex = side.GetSideIndex();
				break;
			}
		}
	}

	if (pAxis == NULL || nSideIndex == -1)
	{
		return FALSE;
	}

	CBCGPPointsArray arSidePoints; 
	GetSidePoints(arSidePoints, nSideIndex, CBCGPChartShape3D::SVT_ORIGINAL);

	// transformed cube is flipped top-down, therefore for bottom edge take top points
	// e.g. if an axis is drawn on the bottom of floor (from left bottom to right bottom of side)
	// we need to take left top and right top points
	switch (m_nWallPosition)
	{
	case WALL_POS_FLOOR:
		if (nSideIndex == CBCGPChartShape3DCube::CUBE_SIDE_FRONT)
		{
			// side points are in order lb, lt, rt, rb
			ptStart = arSidePoints[1];
			ptEnd   = arSidePoints[2];
		}
		else if (nSideIndex == CBCGPChartShape3DCube::CUBE_SIDE_RIGHT)
		{
			// side points are in order lt, rt, rb, lb
			ptStart = arSidePoints[0];
			ptEnd   = arSidePoints[1];
		}
		else if (nSideIndex == CBCGPChartShape3DCube::CUBE_SIDE_BACK)
		{
			// side points are in order lt, lb, rb, rt
			ptStart = arSidePoints[0];
			ptEnd   = arSidePoints[3];
		}
		else if (nSideIndex == CBCGPChartShape3DCube::CUBE_SIDE_LEFT)
		{
			// side points are in order lb, rb, rt, lt
			ptStart = arSidePoints[3];
			ptEnd   = arSidePoints[2];
		}
		break;

	case WALL_POS_LEFT:
		if (nSideIndex == CBCGPChartShape3DCube::CUBE_SIDE_FRONT)
		{
			ptStart = arSidePoints[1];
			ptEnd   = arSidePoints[0];
		}
		else if (nSideIndex == CBCGPChartShape3DCube::CUBE_SIDE_BACK)
		{
			ptStart = arSidePoints[0];
			ptEnd   = arSidePoints[1];
		}
		break;

	case WALL_POS_FRONT:
		if (nSideIndex == CBCGPChartShape3DCube::CUBE_SIDE_LEFT)
		{
			ptStart = arSidePoints[3];
			ptEnd   = arSidePoints[0];
		}
		break;

	case WALL_POS_BACK:
		if (nSideIndex == CBCGPChartShape3DCube::CUBE_SIDE_LEFT)
		{
			ptStart = arSidePoints[2];
			ptEnd   = arSidePoints[1];
		}
		break;

	case WALL_POS_RIGHT:
		if (nSideIndex == CBCGPChartShape3DCube::CUBE_SIDE_FRONT)
		{
			ptStart = arSidePoints[2];
			ptEnd   = arSidePoints[3];
		}
		break;
	default:
		ptStart = arSidePoints[1];
		ptEnd   = arSidePoints[0];
	}

	if (pAxis->IsVertical() && ptStart.y < -1)
	{
		ptStart.y = -1;
	}
	
	return TRUE;	
}