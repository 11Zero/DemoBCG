//*******************************************************************************
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
// BCGPChartShape3D.h : declarations of shape classes used for 3D charts
//

#if !defined(AFX_BCGPCHARTSHAPE3D_H__95D25FEE_77AA_4D59_83EC_65B8A5471694__INCLUDED_)
#define AFX_BCGPCHARTSHAPE3D_H__95D25FEE_77AA_4D59_83EC_65B8A5471694__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"
#include "BCGPGraphicsManager.h"
#include "BCGPMath.h"

class CBCGPChartShape3D;
class CBCGPChartSeries;
class CBCGPChartDiagram3D;
struct BCGPChartFormatLine;
class CBCGPChartVisualObject;

class CBCGPEngine3D;

//*******************************************************************************
class BCGCBPRODLLEXPORT CBCGPChartEdge3D
{
public:
	CBCGPChartEdge3D()
	{
		m_nIndex1 = 0;
		m_nIndex2 = 1;
		m_dwData = (DWORD_PTR)-1;

		m_nOppositeEdgeIndex = -1;
		m_nDiagonalEdgeIndex = -1;
	}

	CBCGPChartEdge3D(int nIdx1, int nIdx2, int nOppositeIdx = -1, int nDiagonalIdx = -1,  DWORD_PTR dwData = (DWORD_PTR)-1)
	{
		m_nIndex1 = nIdx1;
		m_nIndex2 = nIdx2;
		m_dwData = dwData;

		m_nOppositeEdgeIndex = nOppositeIdx;
		m_nDiagonalEdgeIndex = nDiagonalIdx;
	}

	int			m_nIndex1;
	int			m_nIndex2;
	DWORD_PTR	m_dwData;

	int			m_nOppositeEdgeIndex;
	int			m_nDiagonalEdgeIndex;
};

typedef CList<CBCGPChartEdge3D, const CBCGPChartEdge3D&> CBCGPChartEdge3DList;
typedef CArray<CBCGPChartEdge3D, const CBCGPChartEdge3D&> CBCGPChartEdge3DArray;

//*******************************************************************************
class BCGCBPRODLLEXPORT CBCGPChartSide3D
{
	friend class CBCGPChartShape3D;

public:
	CBCGPChartSide3D() : m_nSideIndex(-1), m_pShape(NULL)
	{
		CommonInit();
	}

	CBCGPChartSide3D(int nSideIndex, CBCGPChartShape3D* pShape) : m_nSideIndex(nSideIndex), m_pShape(pShape) 
	{
		CommonInit();	
	}

	void CommonInit()
	{
		m_bIgnored = FALSE;
		m_bVisible = FALSE;
		m_dblAverageZOrder = 1.;
		m_dblMinZOrder = 0.;
		m_dblMaxZOrder = 0;

		m_nAxisID = -1;
		m_nAxisIDGrid1 = -1;
		m_nAxisIDGrid2 = -1;
		m_bIsAxisGrid1Near = FALSE;
		m_bIsAxisGrid2Near = FALSE;
		m_bIsWallSide = FALSE;	

		m_bDrawn = FALSE;
		
		m_vNormal.SetOpenGLDefaultNormal();
		ResetDrawingResources();
	}

	CBCGPChartSide3D(const CBCGPChartSide3D& src)
	{
		CopyFrom(src);
	}

	const CBCGPChartSide3D& operator=(const CBCGPChartSide3D& src)
	{
		CopyFrom(src);
		return *this;
	}

	void ResetDrawingResources();
	void ResetState(BOOL bDrawFlagOnly = FALSE);

	BOOL HasCollision(const CBCGPChartSide3D& side, CBCGPPoint& ptCenterCollision) const;
	double CalcZ(const CBCGPPoint& pt) const;

	void CopyFrom(const CBCGPChartSide3D& src);

	int GetSideIndex() const {return m_nSideIndex;}
	CBCGPChartShape3D* GetShape() const {return m_pShape;}

	virtual void OnDraw(CBCGPEngine3D* pGM, BOOL bFill, BOOL bDrawLine);
	virtual BOOL HitTest(const CBCGPPoint& ptHit, CBCGPChartShape3D** ppShape);

public:
	BOOL				m_bIgnored;
	BOOL				m_bVisible;
	double				m_dblAverageZOrder;
	double				m_dblMinZOrder;
	double				m_dblMaxZOrder;

	CBCGPVector4		m_vPlaneCoef;
	CBCGPVector4		m_vNormal;

	CBCGPRect			m_rectMinMaxPoints;

	BOOL				m_bIsWallSide;
	int					m_nAxisID;
	int					m_nAxisIDGrid1;
	int					m_nAxisIDGrid2;
	BOOL				m_bIsAxisGrid1Near;
	BOOL				m_bIsAxisGrid2Near;

	// this list is valid only during paint
	CList<CBCGPChartSide3D*, CBCGPChartSide3D*>		m_lstSidesBefore;
	BOOL											m_bDrawn;

	static const double		COLLISION_COEF;

protected:
	int						m_nSideIndex;
	CBCGPChartShape3D*		m_pShape;

	CBCGPBrush*				m_pBrFill; 
	CBCGPBrush*				m_pBrLine; 
	BCGPChartFormatLine*	m_pOutlineFormat;
};

typedef CArray<CBCGPChartSide3D, const CBCGPChartSide3D&> CBCGPChartSide3DArray;

typedef CArray<CBCGPPointsArray, const CBCGPPointsArray&> CBCGPPtArrayArray;

//*******************************************************************************
class BCGCBPRODLLEXPORT CBCGPChartShape3D : public CObject
{
	DECLARE_DYNAMIC(CBCGPChartShape3D)

protected:
	CBCGPChartShape3D();

public:
	enum ShapeVertexType
	{
		SVT_ORIGINAL,
		SVT_ROTATED,
		SVT_FINAL
	};

	virtual ~CBCGPChartShape3D()
	{
	}

	const CBCGPChartShape3D& operator=(const CBCGPChartShape3D& src)
	{
		CopyFrom(src);
		return *this;	
	}

	virtual void CopyFrom(const CBCGPChartShape3D& src);

	void SetOriginalVertexes(const CBCGPPointsArray& arPoints)
	{
		m_arOriginalVertexes.RemoveAll();
		m_arOriginalVertexes.Append(arPoints);
	}

	virtual void SetFinalVertexes(const CBCGPPointsArray&	arPoints)
	{
		m_arFinalVertexes.RemoveAll();
		m_arFinalVertexes.Append(arPoints);
	}

	virtual void SetRotatedVertexes(const CBCGPPointsArray&	arPoints)
	{
		m_arRotatedVertexes.RemoveAll();
		m_arRotatedVertexes.Append(arPoints);
	}

	const CBCGPPointsArray& GetVertexes() const {return m_arOriginalVertexes;}
	const CBCGPPointsArray&	GetFinalVertexes() const {return m_arFinalVertexes;}
	const CBCGPPointsArray&	GetRotatedVertexes() const {return m_arRotatedVertexes;}
	const CBCGPChartSide3DArray& GetSides() const {return m_sides;}

	CBCGPPoint GetVertex(int nIndex, ShapeVertexType svt = CBCGPChartShape3D::SVT_ORIGINAL) const;
	virtual void GetSidePoints(CBCGPPointsArray& arSidePoints, int nSideIndex, 
								ShapeVertexType svt = CBCGPChartShape3D::SVT_ORIGINAL) const;

	void SetSideIgnored(int nSideIndex, BOOL bIgnored);

	BOOL IsSideVisible(int nSideIndex) const
	{
		return m_sides[nSideIndex].m_bVisible;
	}

	void SetChart(CBCGPChartVisualObject* pChart) {m_pParentChart = pChart;}
	CBCGPChartVisualObject* GetParentChart() const;

	virtual const CBCGPChartEdge3DList&	GetEdges() const = 0;
	virtual const CBCGPMatrix& GetVertexIndexes() const = 0;

	virtual int GetSideCount() const = 0;
	virtual void OnDraw(CBCGPEngine3D* pGM) = 0;
	virtual void OnAfterTransform(CBCGPChartDiagram3D* pDiagram);

	virtual void ResetDrawingResources();
	virtual void ResetSideData();
	virtual void GetDrawingResources(int nSideIndex, CBCGPBrush** ppBrFill, CBCGPBrush** ppBrLine, BCGPChartFormatLine** ppOutlineFormat) const;

	virtual void AddSidesToList(CList<CBCGPChartSide3D*, CBCGPChartSide3D*>& lst, BOOL bVisible = TRUE);

	virtual void PrepareSidePoints(ShapeVertexType svt = CBCGPChartShape3D::SVT_ROTATED);
	const CBCGPPtArrayArray& GetPreparedSidePoints() const {return m_arPreparedSidePoints;}	


	CBCGPChartSeries*		m_pSeries;
	int						m_nDataPointIndex;
	
	double					m_dblMinZOrder;
	double					m_dblMaxZOrder;

	BOOL					m_bVisible;

protected:
	CBCGPPointsArray	m_arOriginalVertexes;
	CBCGPPointsArray	m_arRotatedVertexes;
	CBCGPPointsArray	m_arFinalVertexes;
	CBCGPChartSide3DArray	m_sides;

	CBCGPPtArrayArray	m_arPreparedSidePoints;
	CBCGPChartVisualObject* m_pParentChart;
};

//*******************************************************************************
class BCGCBPRODLLEXPORT CBCGPChartShape3DRect : public CBCGPChartShape3D
{
	DECLARE_DYNCREATE(CBCGPChartShape3DRect)
public:
	CBCGPChartShape3DRect();
	CBCGPChartShape3DRect(const CBCGPRect& rect, double dblZ = 0);
	CBCGPChartShape3DRect(const CBCGPPoint& ptLeftTop, const CBCGPPoint& ptRightTop, 
		const CBCGPPoint& ptLeftBottom, const CBCGPPoint& ptRightBottom, BOOL bSetFinalVertexes = FALSE);

	virtual ~CBCGPChartShape3DRect();
	
	void InitEdgesAndSides();
	virtual int GetSideCount() const {return 1;}
	virtual const CBCGPChartEdge3DList&	GetEdges() const {return CBCGPChartShape3DRect::m_lst3DRectEdges;}
	virtual const CBCGPMatrix& GetVertexIndexes() const {return CBCGPChartShape3DRect::m_m3DRectVertexIndexes;}

	virtual void OnDraw(CBCGPEngine3D* pGM);

	void SetVertexes(const CBCGPPoint& ptLeftTop, const CBCGPPoint& ptRightTop, 
		const CBCGPPoint& ptLeftBottom, const CBCGPPoint& ptRightBottom, BOOL bSetFinalVertexes = FALSE);

protected:
	static CBCGPChartEdge3DList	m_lst3DRectEdges;
	static BOOL				m_b3DRectShapeInitilaized;

	static CBCGPMatrix			m_m3DRectVertexIndexes;
};
//*******************************************************************************
class BCGCBPRODLLEXPORT CBCGPChartShape3DCube : public CBCGPChartShape3D
{
	DECLARE_DYNCREATE(CBCGPChartShape3DCube)
public:
	static const int CUBE_SIDE_TOP;
	static const int CUBE_SIDE_BOTTOM;
	static const int CUBE_SIDE_FRONT;
	static const int CUBE_SIDE_BACK;
	static const int CUBE_SIDE_LEFT;
	static const int CUBE_SIDE_RIGHT;

public:
	CBCGPChartShape3DCube();

	void InitEdgesAndSides();

	CBCGPChartShape3DCube(const CBCGPRect& rect, double dblZ = 0, double dblDepth = -1);
	CBCGPChartShape3DCube(const CBCGPPoint& ptLeft, const CBCGPPoint& ptRight, double dblHeight, 
							double dblZ = 0, double dblDepth = -1);
	CBCGPChartShape3DCube(const CBCGPPoint& ptLeftTop, const CBCGPPoint& ptRightTop, 
							const CBCGPPoint& ptLeftBottom, const CBCGPPoint& ptRightBottom, 
							double dblZ = 0, double dblDepth = -1);

	void SetOriginalVertexes(const CBCGPRect& rect, double dblZ = 0, double dblDepth = -1);	
	void SetOriginalVertexes(const CBCGPPoint& ptLeft, const CBCGPPoint& ptRight, double dblHeight, 
							double dblZ = 0, double dblDepth = -1);
	void SetOriginalVertexes(const CBCGPPoint& ptLeftTop, const CBCGPPoint& ptRightTop, 
							const CBCGPPoint& ptLeftBottom, const CBCGPPoint& ptRightBottom, 
							double dblZ = 0, double dblDepth = -1);

	virtual int GetSideCount() const {return 6;}
	virtual void GetDrawingResources(int nSideIndex, CBCGPBrush** ppBrFill, CBCGPBrush** ppBrLine, BCGPChartFormatLine** ppOutlineFormat) const;

	void Flip(BOOL bTopBottom);

	virtual const CBCGPChartEdge3DList&	GetEdges() const {return CBCGPChartShape3DCube::m_lst3DCubeEdges;}
	virtual const CBCGPMatrix& GetVertexIndexes() const {return CBCGPChartShape3DCube::m_m3DCubeVertexIndexes;}

	virtual void OnDraw(CBCGPEngine3D* pGM);
	virtual void CopyFrom(const CBCGPChartShape3D& src);

	void SetFillLeftAsTop(BOOL bSet) {m_bFillLeftAsTop = bSet;}
	BOOL IsFillLeftAsTop() const {return m_bFillLeftAsTop;}

	void SetFillLeftAsBottom(BOOL bSet) {m_bFillLeftAsBottom = bSet;}
	BOOL IsFillLeftAsBottom() const {return m_bFillLeftAsBottom;}

protected:
	static CBCGPChartEdge3DList	m_lst3DCubeEdges;
	static BOOL				m_b3DCubeShapeInitilaized;
	static CBCGPMatrix			m_m3DCubeVertexIndexes;

	BOOL m_bFillLeftAsTop;
	BOOL m_bFillLeftAsBottom;
};
//*******************************************************************************
class BCGCBPRODLLEXPORT CBCGPChartShape3DWall : public CBCGPChartShape3DCube
{
	DECLARE_DYNCREATE(CBCGPChartShape3DWall)

public:
	static const int WALL_POS_FLOOR;
	static const int WALL_POS_LEFT;
	static const int WALL_POS_BACK;
	static const int WALL_POS_RIGHT;
	static const int WALL_POS_FRONT;
public:
	CBCGPChartShape3DWall();
	void SetVertexes(int nWallPosition, double dblWallThickness);
	void OnSetBaseDepthPercent(double dblBaseDepthPercent);

	BOOL GetAxisCoordinates(int nAxisID, CBCGPPoint& ptStart, CBCGPPoint& ptEnd);
	int GetWallPosition() const {return m_nWallPosition;}

	virtual void GetDrawingResources(int nSideIndex, CBCGPBrush** ppBrFill, CBCGPBrush** ppBrLine, BCGPChartFormatLine** ppOutlineFormat) const;

protected:
	
	int	m_nWallPosition;
};
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPCHARTSHAPE3D_H__95D25FEE_77AA_4D59_83EC_65B8A5471694__INCLUDED_)
