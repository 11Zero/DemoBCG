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
// BCGPChartVisualObject.h : header file
//

#if !defined(AFX_BCGPCHARTVISUALOBJECT_H__927A0486_C998_41ED_91EA_82593504FBB8__INCLUDED_)
#define AFX_BCGPCHARTVISUALOBJECT_H__927A0486_C998_41ED_91EA_82593504FBB8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"
#include "BCGPChartFormat.h"
#include "BCGPChartAxis.h"
#include "BCGPChartSeries.h"
#include "BCGPChartObject.h"
#include "BCGPMath.h"

#include "BCGPEngine3D.h"

class CBCGPChartAxisX;
class CBCGPChartAxisY;
class CBCGPChartLegendVisualObject;

// CBCGPChartVisualObject window

#define BCGP_CHART_Y_PRIMARY_AXIS			0
#define BCGP_CHART_Y_SECONDARY_AXIS			1
#define BCGP_CHART_X_PRIMARY_AXIS			2
#define BCGP_CHART_X_SECONDARY_AXIS			3
#define BCGP_CHART_Z_PRIMARY_AXIS			4
#define BCGP_CHART_Z_SECONDARY_AXIS			5
#define BCGP_CHART_Y_POLAR_AXIS				6
#define BCGP_CHART_X_POLAR_AXIS				7
#define BCGP_CHART_A_TERNARY_AXIS			8
#define BCGP_CHART_B_TERNARY_AXIS			9
#define BCGP_CHART_C_TERNARY_AXIS			10
#define BCGP_CHART_FIRST_CUSTOM_ID			11

// Thumbnail Flags
#define BCGP_CHART_THUMBNAIL_DRAW_MARKERS	0x0001

// Notifications
extern BCGCBPRODLLEXPORT UINT BCGM_ON_CHART_MOUSE_TRACK;
extern BCGCBPRODLLEXPORT UINT BCGM_ON_CHART_MOUSE_DOWN;
extern BCGCBPRODLLEXPORT UINT BCGM_ON_CHART_MOUSE_UP;
extern BCGCBPRODLLEXPORT UINT BCGM_ON_CHART_AFTER_DRAW;
extern BCGCBPRODLLEXPORT UINT BCGM_ON_CHART_AFTER_RECALC_LAYOUT;
extern BCGCBPRODLLEXPORT UINT BCGM_ON_CHART_AXIS_SCROLLED;
extern BCGCBPRODLLEXPORT UINT BCGM_ON_CHART_AXIS_ZOOMED;

extern BCGCBPRODLLEXPORT UINT BCGM_ON_CHART_AFTER_BEGIN_3DDRAW;
extern BCGCBPRODLLEXPORT UINT BCGM_ON_CHART_BEFORE_END_3DDRAW;


class CBCGPChartVisualObject;
class CBCGPChartSeries;
class CBCGPChartDataPoint;
struct BCGPChartFormatSeries;

struct BCGCBPRODLLEXPORT BCGPChartHitInfo
{
	enum HitInfoTest
	{
		HIT_NONE = 0x0,
		HIT_CHART_AREA = 0x2,		// index1, index2 are set to -1
		HIT_AXIS = 0x4,				// index1 - axis ID, index2 set to -1
		HIT_AXIS_GRIDLINE = 0x8,	// index1 - axis ID, index2 set to 0 for minor gridlines, 1 - for major gridlines
		HIT_LEGEND = 0x10,			// index1, index2 are set to -1 (hit border)
		HIT_LEGEND_ENTRY = 0x20,	// index1 - series index, index2 - data point index if DP are added to legend
		HIT_TITLE = 0x40,			// index1, index2 are set to -1
		HIT_AXIS_NAME = 0x80,		// index1 - axis ID
		HIT_DIAGRAM = 0x100,		// index1 - horz axis ID, index2 - vert axis ID around this area
		HIT_DATA_POINT = 0x200,		// index1 - series index, index2 - data point index
		HIT_DATA_LABEL = 0x400,		// index1 - series index, index2 - data point index
		HIT_AXIS_SCROLL_BAR = 0x800,// index1 - axis ID, index2 set to -1
		HIT_AXIS_THUMB = 0x1000,	// index1 - axis ID, index2 set to -1 if thumb size grip is not hit, 0 - left grip, 1 - right grip; 
									// m_dblVal1 - offset of hit point from left or right bound of size grip
		HIT_AXIS_RESIZE_BAND = 0x2000,	// index1 - axis ID, index2 - 1: resize top; 0: resize bottom
		HIT_OBJECT = 0x4000,		// index1 - object ID
		HIT_AXIS_SCALE_BREAK = 0x8000,	// index1 - axis ID, index2 - sb index, m_dblVal1 - low value, m_dblVal2 - high value
		HIT_DATA_TABLE = 0x10000,		// index1 - series index, index2 - data point index (-1 - legend)
		HIT_ALL_AXIS_ELEMENTS = HIT_AXIS | HIT_AXIS_SCROLL_BAR | HIT_AXIS_THUMB | HIT_AXIS_RESIZE_BAND | HIT_AXIS_NAME | HIT_AXIS_SCALE_BREAK,
		HIT_ALL_ELEMENTS = 0xFFFFF,
	};

	BCGPChartHitInfo()
	{
		m_nIndex1 = -1;
		m_nIndex2 = -1;
		m_nIndex3 = -1;
		m_dblVal1 = 0;
		m_dblVal2 = 0;
		m_hitInfo = BCGPChartHitInfo::HIT_NONE;

		m_pChartObject = NULL;

		m_nMouseButton = -1;
	}

	HitInfoTest	m_hitInfo;
	int			m_nIndex1;
	int			m_nIndex2;
	int			m_nIndex3;

	double		m_dblVal1;
	double		m_dblVal2;

	CBCGPChartObject* m_pChartObject;

	CBCGPPoint	m_ptHit;
	int			m_nMouseButton;
};

struct BCGCBPRODLLEXPORT BCGPChartMouseConfig
{
	enum MouseWheelOptions
	{
		MWO_NONE,
		MWO_ZOOM,
		MWO_SCROLL
	};
	
	enum ZoomScrollOptions
	{
		ZSO_NONE,
		ZSO_WHEEL_PAN,
		ZSO_MAGNIFY,
		ZSO_SELECT
	};

	BCGPChartMouseConfig()
	{
		SetConfig(BCGPChartMouseConfig::ZSO_NONE);
	}

	void SetConfig(BCGPChartMouseConfig::ZoomScrollOptions options)
	{
		Clear();

		switch(options)
		{
		case BCGPChartMouseConfig::ZSO_WHEEL_PAN:
			m_wheelOptions = BCGPChartMouseConfig::MWO_ZOOM;
			m_nPanButton = 0;
			m_nPanModifier = 0;
			break;

		case BCGPChartMouseConfig::ZSO_MAGNIFY:
			m_nMagnifierInButton = 0;
			m_nMagnifierInModifier = 0;
			m_nMagnifierOutButton = 1;
			m_nMagnifierOutModifier = 0;
			m_wheelOptions = BCGPChartMouseConfig::MWO_SCROLL;
			m_nScrollByWheelModifier = VK_CONTROL;
			break;

		case BCGPChartMouseConfig::ZSO_SELECT:
			m_nZoomButton = 0;
			m_nZoomInModifier = 0;
			m_nZoomOutModifier = VK_MENU;
			m_wheelOptions = BCGPChartMouseConfig::MWO_SCROLL;
			m_nScrollByWheelModifier = VK_CONTROL;
			break;
		}

		m_zoomScrollOption = options;
	}

	BCGPChartMouseConfig::ZoomScrollOptions GetZoomScrollOptions() const {return m_zoomScrollOption;}

	void Reset()
	{
		m_nSelectionButton = 0;
		m_nSelectionModifier = 0;

		m_nZoomButton = 0;
		m_nZoomInModifier = VK_SHIFT;
		m_nZoomOutModifier = VK_MENU;

		m_nPanButton = 0;
		m_nPanModifier = 0;
		
		m_nMagnifierInButton = 0;
		m_nMagnifierInModifier = 0;
		m_nMagnifierOutButton = 1;
		m_nMagnifierOutModifier = 0;
		m_nMagnifyFactor = 2;

		m_wheelOptions = BCGPChartMouseConfig::MWO_ZOOM;
		m_nScrollByWheelModifier = VK_SHIFT;
	}

	void Clear()
	{
		m_nSelectionButton = -1;
		m_nSelectionModifier = -1;

		m_nZoomButton = -1;
		m_nZoomInModifier = -1;
		m_nZoomOutModifier = -1;

		m_nPanButton = -1;
		m_nPanModifier = -1;
		
		m_nMagnifierInButton = -1;
		m_nMagnifierInModifier = -1;
		m_nMagnifierOutButton = -1;
		m_nMagnifierOutModifier = -1;
		m_nMagnifyFactor = 2;

		m_nZoomRatio3D = 2; 

		m_wheelOptions = BCGPChartMouseConfig::MWO_NONE;
		m_nScrollByWheelModifier = -1;
	}

	int		m_nSelectionButton;
	int		m_nSelectionModifier;

	int		m_nZoomButton; // zoom button for zoom by selection
	int		m_nZoomInModifier;
	int		m_nZoomOutModifier;

	int		m_nMagnifierInButton;
	int		m_nMagnifierOutButton;
	int		m_nMagnifierInModifier;
	int		m_nMagnifierOutModifier;
	int		m_nMagnifyFactor;

	int		m_nZoomRatio3D;

	int		m_nPanButton;
	int		m_nPanModifier;
	

	BCGPChartMouseConfig::MouseWheelOptions	m_wheelOptions;	
	int										m_nScrollByWheelModifier;

protected:
	BCGPChartMouseConfig::ZoomScrollOptions m_zoomScrollOption; 
}; 

struct BCGPChartSmartLabelParams
{
	BCGPChartSmartLabelParams()
	{
		m_nAngleOffset = 360;
		m_nMaxAllowedDistancePercent = 300;
		m_bArrangeByAngle = TRUE;
		m_bArrangeByDistance = TRUE;
	}

	// the final angle will be calculated as default angle + m_nAngleOffset 
	int		m_nAngleOffset;
	// the final distance is defaultDistance *  m_nMaxAllowedDistancePercent / 100
	int		m_nMaxAllowedDistancePercent; 

	BOOL	m_bArrangeByAngle;
	BOOL	m_bArrangeByDistance;
};

class BCGCBPRODLLEXPORT CBCGPChartDiagram3D
{
	friend class CBCGPChartVisualObject;
	friend class CBCGPChartSide3D;
	friend class CBCGPChartAxis;
	
public:
	enum AxisPointType
	{
		APT_PLANE,
		APT_FLOOR,
		APT_WALL
	};

	enum ExplicitGrouping
	{
		EG_DEFAULT,
		EG_GROUPED,
		EG_NOT_GROUPED
	};

	enum DrawWallOptions
	{
		DWO_NONE = 0,
		DWO_FILL_LEFT_WALL = 0x0001,
		DWO_OUTLINE_LEFT_WALL = 0x0002,
		DWO_FILL_RIGHT_WALL = 0x0004,
		DWO_OUTLINE_RIGHT_WALL = 0x0008,
		DWO_FILL_FLOOR = 0x0010,
		DWO_OUTLINE_FLOOR = 0x0020,
		DWO_DRAW_ALL = 0xFFFF,
		DWO_DRAW_LEFT_WALL = DWO_FILL_LEFT_WALL | DWO_OUTLINE_LEFT_WALL,
		DWO_DRAW_RIGHT_WALL = DWO_FILL_RIGHT_WALL | DWO_OUTLINE_RIGHT_WALL,
		DWO_DRAW_FLOOR = DWO_FILL_FLOOR | DWO_OUTLINE_FLOOR,
		DWO_DRAW_ALL_WALLS = DWO_DRAW_LEFT_WALL | DWO_DRAW_RIGHT_WALL,
		DWO_OUTLINE_ALL_WALLS = DWO_OUTLINE_LEFT_WALL | DWO_OUTLINE_RIGHT_WALL,
		DWO_OUTLINE_ALL = DWO_OUTLINE_ALL_WALLS | DWO_OUTLINE_FLOOR,
		DWO_FILL_ALL_WALLS = DWO_FILL_LEFT_WALL | DWO_FILL_RIGHT_WALL,
		DWO_FILL_ALL = DWO_FILL_ALL_WALLS | DWO_FILL_FLOOR,
		DWO_DEFAULT = DWO_OUTLINE_ALL
	};

	CBCGPChartDiagram3D()
	{
		m_pChart = NULL;
		m_pEngine3D = NULL;

		Init();
	}

	virtual ~CBCGPChartDiagram3D();

	void Init()
	{
		m_nLeftVisibleWallPos = -1;
		m_nRightVisibleWallPos = -1;

		m_bThickWallsAndFloor = TRUE;
		m_dblWallThicknessNormal = 0.;
		m_bSortNeeded = FALSE;
		m_bAutoDestroy3DEngine = TRUE;
		SetRenderingType(CBCGPEngine3D::BCGP_RT_SOFTWARE);

		m_dblLastXRotation = 0.;
		m_dblXRotation = 90.;
		m_dblYRotation = 0.;
		m_dblZRotation = 270.;
		m_dblPerspective = 0.;
		m_bLayoutChanged = TRUE;

		m_bWallsIgnoreRotation = FALSE;

		m_dblHeightScalePercent = 1.;
		m_dblDepthScalePercent	= 0.;
		m_dblBaseDepthPercent	= 1.;
		m_dblFrontDistancePercent = 0.5;
		m_bProportionalScale = FALSE;
		m_bIsXHorizontal = TRUE;

		m_dblZoomFactor = 1.;

		m_bGrouped = FALSE;
		m_explicitGrouping = EG_DEFAULT;
		m_drawWallOptions = DWO_OUTLINE_ALL | DWO_DRAW_ALL_WALLS | DWO_DRAW_FLOOR;

		m_bShowBackfaces = TRUE;
		m_bCalculateNormals = FALSE;
		
		InitEdges();
		SetEdges();

		// it calls InitWalls
		SetBaseDepthPercent(0.1);
		SetPosition(BCGP_DEFAULT_X_ROTATION, BCGP_DEFAULT_Y_ROTATION, BCGP_DEFAULT_PERSPECTIVE);
	}

	void SetRenderingType(CBCGPEngine3D::BCGP_3D_RENDERING_TYPE rt, BOOL bRenderToWindow = TRUE);
	void SetEngine3D(CBCGPEngine3D* pEngine, BOOL bAutoDestroy = TRUE);
	virtual CBCGPEngine3D* GetEngine3D() const {return m_pEngine3D;}

	void SetSortNeeded(BOOL bSet) {m_bSortNeeded = bSet;}
	BOOL IsSortNeeded() const {return m_bSortNeeded;}

	void SetCalculateNormals(BOOL bSet);
	BOOL IsCalculateNormals() const {return m_bCalculateNormals;}

	void SetWallsIgnoreRotation(BOOL bSet);
	BOOL IsWallIgnoreRotation() const {return m_bWallsIgnoreRotation;}

	virtual void OnBegin3DDraw(CBCGPGraphicsManager* pGM);
	virtual void OnEnd3DDraw(const CBCGPRect& rectTarget, CBCGPGraphicsManager* pGMTarget = NULL);

	virtual void OnDraw(CBCGPGraphicsManager* pGM, BOOL bFill, BOOL bOutline);
	virtual void OnDrawWalls(CBCGPGraphicsManager* pGM, BOOL bFill, BOOL bOutline);
	virtual void OnDrawFloor(CBCGPGraphicsManager* pGM, BOOL bFill, BOOL bOutline);
	virtual void AdjustLayout(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagram);
	virtual void OnDrawThickWalls(CBCGPEngine3D* pEngine);

	void SetPosition(double dblXRotation, double dblYRotation, double dblPerspectivePercent);
	void SetZRotation(double dblZRotation);
	void Reset(BOOL bRedraw = TRUE);

	double GetXRotation() const {return m_dblXRotation - 90.;}
	double GetYRotation() const {return m_dblYRotation;}
	double GetZRotation() const {return m_dblZRotation;}

	void SetHeightScalePercent(double dblHeightPercent);
	void SetDepthScalePercent(double dblDepthPercent);
	void SetBaseDepthPercent(double dblBaseDepthPercent);
	void SetFrontDistancePercent(double dblDistancePercent);
	void SetZoomFactor(double dblZoom);

	void SetScrollOffset(const CBCGPPoint& ptOffset) {m_ptScrollOffset = ptOffset;}
	CBCGPPoint GetScrollOffset() const {return m_ptScrollOffset;}

	double GetBaseDepthPercent() const {return m_dblBaseDepthPercent;}
	double GetHeightScalePercent() const {return m_dblHeightScalePercent;}
	double GetDepthScalePercent() const;
	double GetFrontDistancePercent() const {return m_dblFrontDistancePercent;}
	double GetPerspectivePercent() const {return m_dblPerspective;}
	double GetZoomFactor()  const {return m_dblZoomFactor;}
	
	void SetProportionalScale(BOOL bSet = TRUE) {m_bProportionalScale = bSet;}

	void		ScalePoint(CBCGPPoint& ptInOut, const CBCGPPoint& ptDiagramCenter) const;
	CBCGPPoint	TransformPoint(const CBCGPPoint& pt, const CBCGPPoint& ptDiagramCenter = CBCGPPoint(-1, -1), BOOL bScale = TRUE) const;
	void		TransformPointOpt(CBCGPPoint& ptInOut, double dblDepthScalePercent, const CBCGPPoint& ptDiagramCenter, BOOL bScale = TRUE) const;
	void		TransformShape(CBCGPChartShape3D& shape, BOOL bCheckVisibility = TRUE);
	CBCGPPoint	TranslateDistance(const CBCGPPoint& pt); // in pixels on the diagram
	void		TransformVector4(const CBCGPVector4& vIn, CBCGPVector4& vOut, const CBCGPPoint& ptDiagramCenter, BOOL bScale) const;

	void		ScalePointInverse(CBCGPPoint& ptInOut, const CBCGPPoint& ptDiagramCenter);
	CBCGPPoint	TransfromPointInverse(const CBCGPPoint& pt, const CBCGPPoint& ptDiagramCenter = CBCGPPoint(-1, -1), BOOL bUnScale = TRUE);

	void SetChart(CBCGPChartVisualObject* pChart);

	BOOL GetNormalAxisCoordinates(int nAxisID, CBCGPPoint& ptStart, CBCGPPoint& ptEnd);

	BOOL IsGrouped() const {return m_bGrouped;}
	void SetGrouped(BOOL bSet = TRUE, BOOL bRedraw = TRUE);

	ExplicitGrouping GetExplicitGrouping() const {return m_explicitGrouping;}
	void SetExplicitGrouping(ExplicitGrouping grouping, BOOL bRedraw = TRUE);

	const CBCGPRect& GetDiagramRect() const {return	m_rectDiagram;}

	BOOL IsXHorizontal() const {return m_bIsXHorizontal;}
	void SetXHorizontal(BOOL bSet);

	void SetDrawWallOptions(CBCGPChartDiagram3D::DrawWallOptions options, BOOL bRedraw = FALSE);
	DrawWallOptions GetDrawWallOptions() const {return (DrawWallOptions) m_drawWallOptions;}
	void StripWallsAndAxes(BOOL bStrip = TRUE, BOOL bRedraw = FALSE);
	void SetThickWallsAndFloor(BOOL bSet = TRUE, BOOL bRedraw = FALSE);
	BOOL IsThickWallsAndFloor() const {return m_bThickWallsAndFloor;}
	void CollectWallAndFloorSides(CList<CBCGPChartSide3D*, CBCGPChartSide3D*>& lstInvisibleSides, CList<CBCGPChartSide3D*, CBCGPChartSide3D*>& lstVisibleSides);
	BOOL IsWallVisible(int nWallPosition);
	int  GetLeftVisibleWallPos() const {return m_nLeftVisibleWallPos;}
	int	 GetRightVisibleWallPos() const {return m_nRightVisibleWallPos;}
	double GetWallThicknessNormal() const {return m_dblWallThicknessNormal;}

	void SetShowBackfaces(BOOL bSet)  {m_bShowBackfaces = bSet;}
	BOOL IsShowBackfaces() const {return m_bShowBackfaces;}

	CBCGPPoint GetAxisPoint(int nAxisID, AxisPointType apt) const;

	BCGPChartFormatArea m_formatWalls;

	double GetMinZ() const {return m_dblZMin;}
	double GetMaxZ() const {return m_dblZMax;}

	void ResetLabelSize() {m_szMaxLabelSize.SetSize(0, 0);}

protected:
	void FillRotationMatrix(CBCGPMatrix4x4& m, double dblAngle, int nAxis);
	void CreateTransformationMatrix();
	void CreateScaleMatrix();
	void InitWalls();
	void InitEdges();
	void InitWallShapes();
	void SetEdges(BOOL bForce = FALSE);
	void CheckWallVisibilty();

	int FindEdgeIndexByAxisID(int nAxisID) const;

	virtual void OnDrawWallByAxisID(CBCGPGraphicsManager* pGM, int nAxisIndex, const CBCGPBrush& brWallColorFill, 
							const CBCGPBrush& brWallColorLine, double dblLineWidth = 1., BOOL bFill = TRUE, BOOL bOutline = TRUE);
	virtual void DrawLinesCheckAxes(const CBCGPPointsArray& arPoints, const CBCGPBrush& brLine, double dblLineWidth = 1.);

	virtual void DestroyEngine3D();

protected:
	double			m_dblXRotation;
	double			m_dblYRotation;
	double			m_dblZRotation;
	double			m_dblPerspective;

	double			m_dblLastXRotation;
	
	BOOL			m_bIsXHorizontal;

	double			m_dblHeightScalePercent;
	double			m_dblDepthScalePercent;
	double			m_dblBaseDepthPercent;
	double			m_dblFrontDistancePercent;

	double			m_dblZoomFactor;
	CBCGPPoint		m_ptScrollOffset;

	BOOL			m_bProportionalScale;

	BOOL			m_bWallsIgnoreRotation;
	DWORD			m_drawWallOptions;
	BOOL			m_bThickWallsAndFloor;

	BOOL			m_bShowBackfaces;

	BOOL			m_bGrouped;
	ExplicitGrouping m_explicitGrouping;

	BOOL			m_bLayoutChanged;
	BOOL			m_bSortNeeded;

	BOOL			m_bCalculateNormals;

	CBCGPMatrix4x4	m_mTransform;
	CBCGPMatrix4x4	m_mScale;
	
	CBCGPMatrix4x4	m_mTransformInverse;
	CBCGPMatrix4x4	m_mScaleInverse;

	CBCGPRect		m_rectDiagram;
	CBCGPSize		m_szMaxLabelSize;

	CArray<CBCGPPoint, CBCGPPoint>						m_vertexes;
	CArray<CBCGPChartEdge3D, const CBCGPChartEdge3D&>	m_edges;

	CArray<CBCGPChartShape3DWall, const CBCGPChartShape3DWall&> m_arWalls;
	int	m_nLeftVisibleWallPos;
	int	m_nRightVisibleWallPos;

	CBCGPPointsArray					m_arScreenPoints;

	BOOL					m_bAutoDestroy3DEngine;
	CBCGPChartVisualObject* m_pChart;
	CBCGPEngine3D*			m_pEngine3D;

	double					m_dblWallThicknessNormal;

	static double BCGP_DEFAULT_X_ROTATION;
	static double BCGP_DEFAULT_Y_ROTATION;
	static double BCGP_DEFAULT_PERSPECTIVE;

protected:
	double m_dblXMin;
	double m_dblXMax;
	double m_dblYMin;
	double m_dblYMax;
	double m_dblZMin;
	double m_dblZMax;

	CBCGPPoint m_ptZero;
};

class BCGCBPRODLLEXPORT CBCGPChartVisualObject : public CBCGPBaseVisualObject
{
	friend class CBCGPBaseChartImpl;
	friend class CBCGPChartSeries;
	friend class CBCGPChartDiagram3D;
	friend class CBCGPChartLegendVisualObject;

	DECLARE_DYNCREATE(CBCGPChartVisualObject);

public:
	enum ChartElement
	{
		CE_CHART_AREA,
		CE_PLOT_AREA,
		CE_CHART_TITLE,
		CE_LEGEND,
		CE_SELECTION,
		CE_AXIS_NAME,
		CE_AXIS_LABEL,
		CE_LEGEND_ENTRY,
		CE_MAIN_ELEMENT,
		CE_MARKER,
		CE_DATA_TABLE,
	};

// Construction
public:
	CBCGPChartVisualObject(CBCGPVisualContainer* pContainer = NULL);

// Attributes
public:
	void SetChartType(BCGPChartCategory category, BCGPChartType type = BCGP_CT_SIMPLE,   
		BOOL bRedraw = TRUE, BOOL bResetAxesDisplayRange = TRUE);

	static BOOL IsCategory3D(BCGPChartCategory category);

	BOOL IsChart3D() const;
	void SetDiagram3D(CBCGPChartDiagram3D* pDiagram3D, BOOL bAutoDestroy = TRUE);
	virtual CBCGPChartDiagram3D* GetDiagram3D() const {return m_pDiagram3D;}

	CBCGPEngine3D* GetEngine3D() const
	{
		if (m_pDiagram3D == NULL)
		{
			return NULL;
		}

		return m_pDiagram3D->GetEngine3D();
	}

	void SetColors(const CBCGPChartTheme& theme)
	{
		m_currentTheme = theme;
		m_currentTheme.SetOpacity(m_dblThemeOpacity);

		OnColorThemeChanged();
	}

	void SetColors(CBCGPChartTheme::ChartTheme themeType)
	{
		m_currentTheme.SetTheme(themeType);
		m_currentTheme.SetOpacity(m_dblThemeOpacity);

		OnColorThemeChanged();
	}

	const CBCGPChartTheme& GetColors() const
	{
		return m_currentTheme;
	}

	void SetThemeOpacity(int nOpacity = 100);

	BCGPChartCategory GetChartCategory() const {return m_Category;}
	BCGPChartType GetChartType() const {return m_Type;}
	
	void SetThumbnailMode(BOOL bSet = TRUE, UINT nThumbnailFlags = 0);
	
	BOOL IsThumbnailMode() const
	{
		return m_bIsThumbnailMode;
	}

	UINT GetThumbnailFlags() const
	{
		return m_nThumbnailFlags;
	}

	virtual void UpdateSeriesColorIndexes(BOOL bRedraw = FALSE);

	void SetSeriesShadow(BOOL bSet = TRUE);

	// General Layout
	BCGPChartLayout			m_chartLayout;

	void SetPlotAreaPadding(const CBCGPRect& rectPadding) {m_rectPlotAreaPadding = rectPadding;}
	CBCGPRect GetPlotAreaPadding(BOOL bScaled = FALSE) const;

	void SetTitleAreaPadding(const CBCGPSize& szPadding) {m_szTitleAreaPadding = szPadding;}
	CBCGPSize GetTitleAreaPadding(BOOL bScaled = FALSE) const;

	void SetLegendAreaPadding(const CBCGPSize& szPadding) {m_szLegendAreaPadding = szPadding;}
	CBCGPSize GetLegendAreaPadding(BOOL bScaled = FALSE) const;

	void SetLegendElementSpacing(const CBCGPSize& szSpacing) {m_szLegendElementSpacing = szSpacing;}
	CBCGPSize GetLegendElementSpacing(BOOL bScaled = FALSE) const;

	void SetHitTestDataPointPrecision(const CBCGPSize& szPrecision) {m_szHitTestDataPointPrecision = szPrecision;}
	CBCGPSize GetHitTestDataPointPrecision(BOOL bScaled = FALSE) const;

	const CBCGPRect& GetDiagramArea() const {return m_rectDiagramArea;}
	const CBCGPRect& GetLegendArea() const {return m_rectLegendArea;}
	const CBCGPRect& GetDataTableArea() const {return m_rectDataTableArea;}
	double GetDataTableHeaderColumnWidth() const { return m_dblDataTableHeaderColumnWidth; }
	double GetDataTableRowHeight() const { return m_dblDataTableRowHeight; }

	void SetSmartLabelsParams(const BCGPChartSmartLabelParams& params, BOOL bRedraw = TRUE);
	const BCGPChartSmartLabelParams& GetSmartLabelsParams() const {return m_smartLabelsParams;}

public:
	BCGPChartFormatArea& GetChartAreaFormat() {return m_chartAreaFormat;}
	BCGPChartFormatArea& GetPlotAreaFormat() {return m_plotAreaFormat;}
	BCGPChartFormatLabel& GetTitleLabelFormat() {return m_titleAreaFormat;}
	BCGPChartFormatArea& GetLegendFormat() {return m_legendAreaFormat;}
	BCGPChartFormatDataTable& GetDataTableFormat() {return m_dataTableAreaFormat;}
	BCGPChartMouseConfig& GetMouseConfig() {return m_mouseConfig;};

	const CString& GetChartTitle() const {return m_strChartTitle;}

// Helpers for chart layout and some visual settings
	void SetChartFillColor(const CBCGPBrush& brColor);
	void SetDiagramFillColor(const CBCGPBrush& brColor);

	const CBCGPBrush& GetChartFillColor() const;
	const CBCGPBrush& GetDiagramFillColor() const;

	void SetLegendPosition (BCGPChartLayout::LegendPosition position = BCGPChartLayout::LP_RIGHT, 
		BOOL bLegendOverlapsChart = FALSE, BOOL bRedraw = FALSE);
	void EnableDrawLegendShape(BOOL bEnable = TRUE, BOOL bRedraw = FALSE);
	void ShowChartTitle(BOOL bShow = TRUE, BOOL bTitleOverlapsChart = FALSE, BOOL bRedraw = FALSE);
	void ShowDataTable(BOOL bShow = TRUE, BCGPChartFormatDataTable*	pDataTableAreaFormat = NULL);
	void ShowAxis(int nAxisID, BOOL bShow = TRUE, BOOL bForceShow = FALSE);
	void ShowAxisIntervalInterlacing(int nAxisID, BOOL bShow = TRUE, int nStep = 2, int nFirstInterval = 0);
	void SetAxisIntervalInterlaceColor(int nAxisID, const CBCGPBrush& brush);
	void ShowAxisName(int nAxisID, BOOL bShow = TRUE);
	void ShowScrollBar(int nAxisID, BOOL bShow = TRUE);
	void ShowScrollBars(BOOL bShow = TRUE, BOOL bRedraw = TRUE);
	void SetAxisName(int nAxisID, const CString& strName);
	void ShowAxisGridLines(int nAxisID, BOOL bShowMajorGridLines = TRUE, BOOL bShowMinorGridLines = FALSE);
	void SetChartTitle(LPCTSTR lpcszText, BCGPChartFormatLabel* pTitleFormat = NULL, BOOL bAdjustLayout = FALSE);
	void SetTernaryAxasLabelMode(CBCGPChartTernaryAxis::LabelMode lm, BOOL bRedraw = FALSE);

	void EnableAntialiasing(BOOL bEnable = TRUE) {m_bAAEnabled = bEnable;}
	BOOL IsAntialiasingEnabled() const {return m_bAAEnabled;}

// Notifications
	void EnableMouseTrackingMode(DWORD dwHitInfoFlags = BCGPChartHitInfo::HIT_ALL_ELEMENTS);

// Smart labels
	BOOL IsSmartLabelsEnabled() const {return m_bSmartLabelsEnabled;}
	void EnableSmartLabels(BOOL bEnable);

protected:
// Chart Theme
	CBCGPChartTheme			m_currentTheme;
	double					m_dblThemeOpacity;

// Define chart style
	BCGPChartCategory	m_Category;
	BCGPChartType		m_Type;

// Styles located in m_arSeriesColorThemes will be applied automatically to 
// each newly created series. Set it to -1 to manage styles manually.
	int					m_nDefaultVisualSettingIndex;

// Zoom/Selection/Scroll support
	BOOL						m_bEnableSelection;
	BOOL						m_bEnableZoom;
	BOOL						m_bEnableMagnifier;
	BOOL						m_bEnableScroll;
	BOOL						m_bEnablePan;

	CBCGPPoint					m_ptSelStart;
	CBCGPPoint					m_ptSelEnd;
	BOOL						m_bSelectionMode;

	CBCGPPoint					m_ptPanStart;
	CBCGPPoint					m_ptPanOrigin;
	BOOL						m_bPanMode;

	BOOL						m_bThumbTrackMode;
	CBCGPChartAxis*				m_pThumbTrackAxis;

	BOOL						m_bThumbSizeMode;
	CBCGPChartAxis*				m_pThumbSizeAxis;
	CBCGPPoint					m_ptThumbSizeStart;
	BOOL						m_bThumbSizeLeft;
	double						m_dblThumbHitOffset;

	BCGPChartMouseConfig		m_mouseConfig;

	BOOL						m_bClipDiagramToAxes;

// Mouse tracking
	BCGPChartHitInfo::HitInfoTest	m_hitInfoFlags;
	BCGPChartHitInfo				m_lastHit;

// Layout
 	CBCGPRect				m_rectPlotAreaPadding;
 	CBCGPSize				m_szTitleAreaPadding; // cx - distance from top, cy - distance from next vertical chart element
 	CBCGPSize				m_szLegendAreaPadding; // cx - horizontal distance from chart elements to legend; cy - vertical disnace from chart elements to legend
 	CBCGPSize				m_szLegendElementSpacing;
 	CBCGPSize				m_szHitTestDataPointPrecision;

// Format Areas
	// Format chart area
	BCGPChartFormatArea			m_chartAreaFormat;
	// Format plot area
	BCGPChartFormatArea			m_plotAreaFormat;
	// Format legend
	BCGPChartFormatArea			m_legendAreaFormat;
	// Format data table
	BCGPChartFormatDataTable	m_dataTableAreaFormat;
	// Format chart title
	BCGPChartFormatLabel		m_titleAreaFormat;
	// Selection
	BCGPChartFormatSelection	m_selectionFormat;

// Data Label And Marker Options
	BCGPChartDataLabelOptions	m_dataLabelOptions;
	BCGPChartMarkerOptions		m_markerOptions;

// Cached curve type
	BCGPChartFormatSeries::ChartCurveType	m_curveType;
	
// Area locations
	CBCGPRect	m_rectPlotArea;
	CBCGPRect	m_rectDiagramArea;
	CBCGPRect	m_rectLegendArea;
	CBCGPRect	m_rectDataTableArea;
	double		m_dblDataTableHeaderColumnWidth;
	double		m_dblDataTableRowHeight;
	double		m_dblMaxSeriesKeyWidth;
	CBCGPRect	m_rectTitleArea;

// Data
	CString	m_strChartTitle;

// Legend
	CObList m_lstRelatedLegends;
	BOOL	m_bShowSurfaceMapInLegend;

// Axes
	CArray<CBCGPChartAxis*, CBCGPChartAxis*>	m_arAxes;
	BOOL										m_bEnableResizeAxes;

	BOOL										m_bResizeAxisMode;
	CBCGPChartAxis*								m_pResizedAxis;
	CBCGPChartAxis*								m_pNextResizedAxis;
	BOOL										m_bResizeAxisTop;
	CBCGPPoint									m_ptStartResize;
	double										m_dblResizedAxisTopOffset;
	double										m_dblResizedAxisBottomOffset;
	double										m_dblNextResizedAxisTopOffset;
	double										m_dblNextResizedAxisBottomOffset;

// Chart Objects
	CList<CBCGPChartObject*, CBCGPChartObject*> m_lstChartObjects;

// Chart Effects
	CList<CBCGPChartBaseEffect*, CBCGPChartBaseEffect*> m_lstChartEffects;

// AutoColors
	int m_nLastColorIndex;

	CBCGPImage						m_ImageCache;

// 3D Diagram
	CBCGPChartDiagram3D*			m_pDiagram3D;
	BOOL							m_bIsAutoDestroyDiagram3D;

// Antialiasing
	BOOL							m_bAAEnabled;

// Compatibility
	BOOL							m_bRecalcMinMaxForOptimizedSeries;

// Smart Labels
	BOOL							m_bSmartLabelsEnabled;
	BCGPChartSmartLabelParams		m_smartLabelsParams;

// Thumbnail Mode
	BOOL							m_bIsThumbnailMode;
	UINT							m_nThumbnailFlags;

// Data table
	CBCGPTextFormat					m_tfDataTableLeft;

// Current draw order
	enum ChartDrawOrder
	{
		CDO_IGNORE,
		CDO_BACKGROUND,
		CDO_NORMAL
	};
	ChartDrawOrder					m_DrawOrder;

	static CList<CBCGPChartVisualObject*, CBCGPChartVisualObject*>	m_lstCharts;

// Operations
public:
	static void UpdateAllChartsDefaultFont(const CString& strFontFamily, BOOL bApplyToExistingCharts = TRUE);
	void UpdateDefaultFont(const CString& strFontFamily, const CString& strOldName = _T(""));

// Layout 
	virtual void AdjustLayout(CBCGPGraphicsManager* pGM);
	virtual void OnScaleRatioChanged(const CBCGPSize& sizeScaleRatioOld);

// 3D operations
	void SetBaseDepthPercent3D(double dblUnitSize);
	BOOL IsChart3DGrouped() const;

// Gesture processing:
	virtual BOOL GetGestureConfig(CBCGPGestureConfig& gestureConfig);
	virtual BOOL OnGestureEventZoom(const CBCGPPoint& ptCenter, double dblZoomFactor);
	virtual BOOL OnGestureEventPan(const CBCGPPoint& ptFrom, const CBCGPPoint& ptTo, CBCGPSize& sizeOverPan);

// Mouse processing
	virtual BOOL OnMouseDown(int nButton, const CBCGPPoint& pt);
	virtual void OnMouseUp(int nButton, const CBCGPPoint& pt);
	virtual void OnMouseMove(const CBCGPPoint& pt);
	virtual void OnMouseLeave();
	virtual BOOL OnMouseWheel(const CBCGPPoint& pt, short zDelta);
	virtual BOOL OnSetMouseCursor(const CBCGPPoint& pt);

	virtual BOOL HitTest(const CBCGPPoint& pt, BCGPChartHitInfo* pHitInfo, DWORD dwHitInfoFlags = 0xFFFF);

	virtual BOOL OnKeyboardDown(UINT nChar, UINT nRepCnt, UINT nFlags);

// Axes
	virtual CBCGPChartAxis*	GetChartAxis(int nAxisID) const;
	virtual CBCGPChartAxis*	GetPrimaryAxis(BOOL bHorizontal) const;
	virtual CBCGPChartAxis*	GetSecondaryAxis(BOOL bHorizontal) const;
	void ResetAxes();
	int AddAxis(CBCGPChartAxis* pAxis);
	void RemoveCustomAxis(CBCGPChartAxis* pAxis, BOOL bDeleteRelatedSeries);
	int GetAxisCount() const {return (int)m_arAxes.GetSize();}
	int GetNextCustomAxisID();
	CBCGPChartAxis* ReplaceDefaultAxis(int nAxisID, CRuntimeClass* pCustomAxisRTC);
	virtual BOOL OnGetMinMaxValues(CBCGPChartAxis* pAxis, double& dblMin, double& dblMax);
	virtual void OnCalcMinMaxValues(CBCGPChartAxis* pAxis);
	void ScrollAxis(float fUnitsToScroll, int nAxisID, BOOL bRedraw = TRUE);
	void SetAutoDisplayRange(BOOL bRedraw = TRUE, BOOL bByZoom = FALSE);
	void SetFixedDisplayBound(const CBCGPChartData& data, BOOL bIsMinimum, BOOL bAxesFromData = FALSE, BOOL bByZoom = FALSE);
	void SetFixedDisplayRange(CBCGPChartData& dataStart, CBCGPChartData& dataEnd, BOOL bAxesFromData = FALSE, 
		BOOL bRedraw = TRUE, BOOL bByZoom = FALSE);
	virtual void SwapAxesDirections(BOOL bAdjustGradientAngles = TRUE);

	void EnableResizeAxes(BOOL bEnable = TRUE) {m_bEnableResizeAxes = bEnable;}
	BOOL IsResizeAxesEnabled() const {return m_bEnableResizeAxes;}

	BOOL BeginResizeAxis(int nAxisID, BOOL bTop, const CBCGPPoint& ptStart);
	void EndResizeAxis(BOOL bResizeCanceled = FALSE);
	virtual BOOL UpdateAxisSize(const CBCGPPoint& ptCurr);

// Chart objects (like lines, texts etc)
	void AddChartObject(CBCGPChartObject* pObj);
	CBCGPChartObject* AddChartObject(const CBCGPPoint& ptLeftTopOffset, CBCGPChartObject::CoordinateMode mode = CBCGPChartObject::CM_PIXELS, BOOL bRelativeToDefaultAxes = TRUE);
	CBCGPChartObject* AddChartObject(const CBCGPRect& rcCoordinates, CBCGPChartObject::CoordinateMode mode = CBCGPChartObject::CM_PIXELS, BOOL bRelativeToDefaultAxes = TRUE);
	CBCGPChartObject* AddChartTextObject(const CBCGPRect& rcCoordinates, const CString& strText, CBCGPChartObject::CoordinateMode mode = CBCGPChartObject::CM_PIXELS, BOOL bRelativeToDefaultAxes = TRUE);
	CBCGPChartAxisMarkObject* AddChartAxisMarkObject(double dblVal, const CString& strText, BOOL bVertAxis = TRUE, BOOL bOutside = TRUE, 
		const CBCGPBrush& brTextColor = CBCGPBrush(), const CBCGPBrush& brFill = CBCGPBrush(), const CBCGPBrush& brOutline = CBCGPBrush());
	CBCGPChartLineObject* AddChartLineObject(double dblX1, double dblY1, double dblX2, double dblY2, 
							const CBCGPBrush& brLine = CBCGPBrush(), double dblWidth = 1., CBCGPStrokeStyle* pStrokeStyle = NULL);
	CBCGPChartLineObject* AddChartLineObject(double dblVal, BOOL bHorz = TRUE, 
							const CBCGPBrush& brLine = CBCGPBrush(), double dblWidth = 1., CBCGPStrokeStyle* pStrokeStyle = NULL);
	CBCGPChartRangeObject* AddChartRangeObject(double dblBottomVal, double dblTopVal, BOOL bHorz, const CBCGPBrush& brFill);
	
	void RemoveAllChartObjects();
	void RemoveChartObject(CBCGPChartObject* pObj);
	CBCGPChartObject* FindChartObject(int nID) const;
	CBCGPChartObject* FindChartObject(const CString& strName) const;

// Chart Effects
	virtual void OnCalcChartEffectsScreenPositions(CBCGPGraphicsManager* pGM);
	void AddChartEffect(CBCGPChartBaseEffect* pEffect);
	void RemoveAllChartEffects();

// Layout overrides and calculations
	virtual CBCGPSize CalcLegendSize(CBCGPGraphicsManager* pGM);
	virtual BOOL CalcDataTableSize(CBCGPGraphicsManager* pGM, double& dblDataTableHeaderColumnWidth, double& dblDataTableHeight, double& dblDataTableRowHeight, double& dblMaxSeriesKeyWidth);
	virtual CBCGPSize CalcTitleSize(CBCGPGraphicsManager* pGM);
	virtual CBCGPSize OnCalcLegendKeySize(CBCGPChartSeries* pSeries, int nDataPointIndex);
	virtual CBCGPSize OnCalcLegendKeySizeEx(const BCGPChartCellParams& params, CBCGPChartSeries* pSeries);
	virtual CBCGPSize OnCalcLegendLabelSize(CBCGPGraphicsManager* pGM, CBCGPChartSeries* pSeries, int nDataPointIndex);
	virtual CBCGPSize OnCalcLegendLabelSizeEx(CBCGPGraphicsManager* pGM, const CString& strLabel, 
		const BCGPChartCellParams& params);
	virtual CBCGPSize OnCalcAxisLabelSize(CBCGPGraphicsManager* pGM, CBCGPChartAxis* pAxis, double dblValue, CString& strLabel, 
		const CBCGPTextFormat& textFormat);
	virtual BOOL OnCalcDataLabelSize(CBCGPGraphicsManager* pGM, const CString& strText, CBCGPChartSeries* pSeries, int nDataPointIndex, CBCGPSize& sz); 
	virtual BOOL IsLegendHorizontal() const;

	void SetShowSurfaceMapInLegend(BOOL bSet = TRUE) {m_bShowSurfaceMapInLegend = bSet;}
	BOOL IsShowSurfaceMapInLegend() const {return m_bShowSurfaceMapInLegend && (GetChartCategory() == BCGPChartSurface3D);}

// Series Management
	virtual void RecalcMinMaxValues();
	CBCGPChartSeries* OnCreateChartSeries(BCGPChartCategory category, BCGPChartType type);
	int FindSeriesIndex(CBCGPChartSeries* pSeries) const;

	CBCGPChartSeries* CreateSeries(const CString& strName, const CBCGPColor& color = CBCGPColor(), 
		BCGPChartType type = BCGP_CT_DEFAULT,
		BCGPChartCategory category = BCGPChartDefault, int nSeriesIndex = -1);

	int AddSeries(CBCGPChartSeries* pSeries, BCGPChartFormatSeries* pFormatSeries = NULL, int nIndex = -1);

	int  AddChartData (const CString& strCategoryName, double dblY, int nSeries = 0, BCGPChartFormatSeries* pFormatDataPoint = NULL, DWORD_PTR dwUserData = 0);
	int  AddChartData (double dblData, int nSeries = 0, CBCGPChartData::ComponentIndex ci = CBCGPChartData::CI_Y, DWORD_PTR dwUserData = 0, BCGPChartFormatSeries* pFormatDataPoint = NULL, BCGPChartFormatSeries* pFormatSeries = NULL);
	int  AddChartData (double dblY, BCGPChartFormatSeries* pFormatDataPoint, int nSeries = 0, DWORD_PTR dwUserData = 0);
	int  AddChartDataYX (double dblY, double dblX, int nSeries = 0, DWORD_PTR dwUserData = 0, BCGPChartFormatSeries* pFormatDataPoint = NULL);
	int	 AddChartDataYXZ (double dblY, double dblX, double dblZ, int nSeries = 0, DWORD_PTR dwUserData = 0, BCGPChartFormatSeries* pFormatDataPoint = NULL);
	int  AddChartDataYXY1 (double dblY, double dblX, double dblY1, int nSeries = 0, DWORD_PTR dwUserData = 0, BCGPChartFormatSeries* pFormatDataPoint = NULL);
	int  AddChartDataYXY1 (const CString& strCategoryName, double dblY, double dblX, double dblY1, int nSeries = 0, DWORD_PTR dwUserData = 0, BCGPChartFormatSeries* pFormatDataPoint = NULL);
	int  AddChartDataYY1 (const CString& strCategoryName, double dblY, double dblY1, int nSeries = 0, BCGPChartFormatSeries* pFormatDataPoint = NULL, DWORD_PTR dwUserData = 0);
	int  AddChartDataYY1Y2(double dblY, double dblY1, double dblY2, int nSeries = 0, BCGPChartFormatSeries* pFormatDataPoint = NULL, DWORD_PTR dwUserData = 0);
	int  AddChartData (CArray<double, double>& arData, int nSeries = 0, DWORD_PTR dwUserData = 0, BCGPChartFormatSeries* pFormatDataPoint = NULL);
	void AddDataPoints(const CBCGPDoubleArray& arYValues, int nSeries = 0, 
					  CBCGPDoubleArray* parXValues = NULL, CBCGPDoubleArray* parY1Values = NULL, BOOL bRecalcMinMaxValues = TRUE);
	void AddDataPointsOptimized(const CBCGPDoubleArray& arYValues, int nSeries = 0, CBCGPDoubleArray* pXValues = NULL, 
		CBCGPDoubleArray* pY1Values = NULL, BOOL bRecalcMinMaxValues = FALSE);

	BOOL SetChartData(double dblData, int nSeries, int nDataPointIndex, CBCGPChartData::ComponentIndex ci);
	int AddChartEmptyData (int nSeries = 0, DWORD_PTR dwUserData = 0, BCGPChartFormatSeries* pFormatDataPoint = NULL, BCGPChartFormatSeries* pFormatSeries = NULL);
	int AddChartEmptyData(double dblX, int nSeries = 0, DWORD_PTR dwUserData = 0, BCGPChartFormatSeries* pFormatDataPoint = NULL);
	int AddChartEmptyData(const CString& strCategoryName, int nSeries = 0, DWORD_PTR dwUserData = 0, BCGPChartFormatSeries* pFormatDataPoint = NULL);
	

	BOOL SetSeriesName (const CString& strName, int nSeries = 0);
	BOOL ShowSeries (BOOL bVisible = TRUE, int nSeries = 0, BOOL bIsSecondaryAxis = FALSE);
	BOOL FormatSeries (BCGPChartFormatSeries* pFormatSeries, int nSeries = 0);
	int  GetSeriesCount(BOOL bIncludeNulls = TRUE);
	void GetAllNonFormulaSeries(CArray<CBCGPChartSeries*, CBCGPChartSeries*>& arSeries);
	virtual void InvalidateTrendFormulaSeries();
	CBCGPChartSeries* GetSeries (int nSeries, BOOL bAllocate = FALSE);
	const BCGPChartFormatSeries* GetDataPointFormat(int nSeries, int nDataPointIndex);
	CBCGPChartValue GetDataPointValue (int nSeries, int nDataPointIndex, CBCGPChartData::ComponentIndex ci = CBCGPChartData::CI_Y);
	DWORD_PTR GetDataPointUserData (int nSeries, int nDataPointIndex);
	void CleanUpChartData (int nSeries = -1, BOOL bRemoveUnusedEntries = FALSE);
	int GetVisibleSeriesCount(CRuntimeClass* pImplType = NULL);
	void SetDataPointFormat(const BCGPChartFormatSeries& dataPointStyle, int nDataPointIndex, int nSeriesIndex = 0);
	void SetDataPointDataLabelText(const CString& strText, int nDataPointIndex, int nSeriesIndex = 0);
	void SetDataPointLegendLabelText(const CString& strText, int nDataPointIndex, int nSeriesIndex = 0);
	void SetDataPointCategoryName(const CString& strCategoryName, int nDataPointIndex, int nSeriesIndex = 0);
	void ShowAllDataLabels(BOOL bShow = TRUE);
	void ShowDataLabels(BOOL bShow = TRUE, BOOL bDrawBorder = TRUE, BOOL bDropLineToMarker = TRUE, 
		double dblAngle = -1);
	void ShowDataMarkers(BOOL bShow = TRUE, int nSize = -1,
		BCGPChartMarkerOptions::MarkerShape shape = (BCGPChartMarkerOptions::MarkerShape) -1);
	void SetCurveType(BCGPChartFormatSeries::ChartCurveType curveType);

	void SetDataLabelsOptions(const BCGPChartDataLabelOptions& options);
	const BCGPChartDataLabelOptions& GetDataLabelOptions() const {return m_dataLabelOptions;}

// Selection and Zoom
	BCGPChartFormatSelection& GetSelectionFormat() {return m_selectionFormat;}	
	void BeginSelection(const CBCGPPoint& ptStart);
	void BeginPan(const CBCGPPoint& ptStart);
	virtual BOOL BeginThumbTrack(int nAxisID);
	virtual void EndThumbTrack();
	virtual BOOL BeginThumbSize(int nAxisID, BOOL bLeftGrip, const CBCGPPoint& ptHit, double dblOffset);
	virtual void EndThumbSize();
	CBCGPChartAxis* GetThumbSizeAxis() const {return m_pThumbSizeAxis;}

	BOOL IsPanMode() const {return m_bPanMode;}
	BOOL IsThumbTrackMode() const {return m_bThumbTrackMode;}
	BOOL IsThumbSizeMode() const {return m_bThumbSizeMode;}

	void UpdateSelection(const CBCGPPoint& ptCurr, BOOL bRedraw = TRUE);
	void EndSelection(const CBCGPPoint& ptEnd);
	virtual void ZoomByCurrentSelection(BOOL bZoomOut);
	virtual void Zoom(int nMagnifier, const CBCGPPoint& ptZoomCenter = CBCGPPoint(0, 0));
	virtual void UnZoom(BOOL bRedraw = TRUE);

	void EnableZoom(BOOL bEnable = TRUE) {m_bEnableZoom = bEnable; ResetGestureOptions(); }
	void EnableMagnifier(BOOL bEnable = TRUE) {m_bEnableMagnifier = bEnable;}
	void EnableScroll(BOOL bEnable = TRUE) {m_bEnableScroll = bEnable; ResetGestureOptions(); }
	void EnablePan(BOOL bEnable = TRUE) {m_bEnablePan = bEnable; ResetGestureOptions(); }
	BOOL IsMagnifierEnabled() const {return m_bEnableMagnifier;}
	BOOL IsZoomEnabled() const {return m_bEnableZoom;}
	BOOL IsScrollEnabled() const {return m_bEnableScroll;}
	BOOL IsPanEnabled() const {return m_bEnablePan;}
	BOOL IsClipDiagramToAxes() const {return m_bClipDiagramToAxes;}
	void EnableClipDiagramToAxes(BOOL bEnable = TRUE) {m_bClipDiagramToAxes = bEnable;}
	BOOL IsSelectionEnabled() const {return m_bEnableSelection;}
	void EnableSelection(BOOL bEnable = TRUE, BCGPChartFormatSelection::SelectionType selType = BCGPChartFormatSelection::ST_VERT_AND_HORZ_AXES) 
	{
		m_bEnableSelection = bEnable;
		m_selectionFormat.m_selectionType = selType;
	}
	void SetSelectionType(BCGPChartFormatSelection::SelectionType selType = BCGPChartFormatSelection::ST_VERT_AND_HORZ_AXES)
	{
		m_selectionFormat.m_selectionType = selType;
	}
	void SetZoomScrollConfig(BCGPChartMouseConfig::ZoomScrollOptions options, 
							 BCGPChartFormatSelection::SelectionType selType = BCGPChartFormatSelection::ST_VERT_AND_HORZ_AXES);
	BCGPChartMouseConfig::ZoomScrollOptions GetZoomScrollConfig() const {return m_mouseConfig.GetZoomScrollOptions();}

	void NormalizeDisplayRange();

// Data Point to Screen coordinates and screen coordinates to data point conversions
	CBCGPPoint ScreenPointFromChartData(const CBCGPChartData& chartData, int nDataPointIndex, BOOL& bIsEmpty, CBCGPChartSeries* pSeries) const;
	CBCGPChartData ChartDataFromScreenPoint(const CBCGPPoint& pt, CBCGPChartAxis::RoundType roundType, BOOL bUseSelectionTypeForReturn = FALSE, BOOL bIsStart = TRUE) const;

// Legend
	void AddRelatedLegend(CBCGPChartLegendVisualObject* pLegend, BOOL bRedraw = TRUE);
	void RemoveRelatedLegend(CBCGPChartLegendVisualObject* pLegend, BOOL bRedraw = TRUE);
	void UpdateRelatedLegendColors();
	const CObList& GetRelatedLegends() const {return m_lstRelatedLegends;}
	CBCGPChartLegendVisualObject* GetRelatedLegend(int nIndex = 0) const;

// Drawing
	virtual void OnDraw(CBCGPGraphicsManager* pGM, const CBCGPRect& rectClip, DWORD dwFlags = BCGP_DRAW_STATIC | BCGP_DRAW_DYNAMIC);
	virtual void OnFillBackground (CBCGPGraphicsManager* pGM);
	virtual void OnDrawChartArea (CBCGPGraphicsManager* pGM, const CBCGPRect& rectChartArea);
	virtual void OnDrawChartTitle(CBCGPGraphicsManager* pGM, CString& strChartTitle, 
		CBCGPRect& rectChartTitle, BCGPChartFormatLabel& labelStyle);
	virtual void OnDrawChartLegend(CBCGPGraphicsManager* pGM, CBCGPRect& rectLegend, BCGPChartFormatArea& legendStyle);
	virtual void OnDrawLegendEntry(CBCGPGraphicsManager* pGM, const CRect& rectLegend, 
		CBCGPChartSeries* pSeries, CBCGPChartDataPoint* pDataPoint, int nDataPointIndex);
	virtual void OnDrawChartDataTable(CBCGPGraphicsManager* pGM, CBCGPRect& rectDataTable, double nDataTableHeaderColumnWidth);
	virtual void OnDrawChartDataTableLegendItem(CBCGPGraphicsManager* pGM, const CBCGPRect& rectLegend, 
		CBCGPChartSeries* pSeries, int nRow);
	virtual void OnDrawChartDataTableEntry(CBCGPGraphicsManager* pGM, const CBCGPRect& rectCell, 
		CBCGPChartSeries* pSeries, int nDPIndex, int nRow);
	virtual void OnDrawPlotArea (CBCGPGraphicsManager* pGM, const CBCGPRect& rectPlotArea, const CBCGPRect& rectDiagramArea);
	virtual void OnDrawPlotAreaItems (CBCGPGraphicsManager* pGM, const CBCGPRect& rectPlotArea, const CBCGPRect& rectDiagramArea);
	virtual void OnInterlaceAxesIntervals(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagramArea);
	virtual void OnDrawAxes(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagramArea);
	virtual void OnDrawAxesGridLines(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagramArea);
	virtual void OnDrawAxisBounds(CBCGPGraphicsManager* pGM, const CBCGPRect& rectBounds);
	virtual void OnDrawAxisName(CBCGPGraphicsManager* pGM, CBCGPChartAxis* pAxis);
	virtual void OnDrawAxisScaleBreaks(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagramArea);
	virtual void OnDrawAxesLabels(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagramArea);
	virtual void OnDrawGridLine(CBCGPGraphicsManager* pGM, CBCGPPoint& ptGridLineStart, CBCGPPoint& ptGridLineEnd, 
		CBCGPChartAxis* pAxis, double dblCurrValue, const BCGPChartFormatLine& gridLineStyle, BOOL bIsMajor);
	virtual void OnDrawGridEllipse(CBCGPGraphicsManager* pGM, const CBCGPEllipse& ellipse,
		CBCGPChartAxis* pAxis, double dblCurrValue, const BCGPChartFormatLine& gridLineStyle, BOOL bIsMajor);
	virtual void OnDrawDiagram(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagramArea);
	virtual void OnDrawDiagramMarkers(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagram);
	virtual void OnDrawDiagramDataLabels(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagram);
	virtual void OnDrawChartEffects(CBCGPGraphicsManager* pGM);
	virtual void OnDrawChartObjects(CBCGPGraphicsManager* pGM, BOOL bForeground);
	virtual void OnDrawSelection(CBCGPGraphicsManager* pGM);
	virtual void OnDrawTickMark(CBCGPGraphicsManager* pGM, CBCGPPoint& ptStart, CBCGPPoint& ptEnd, BCGPChartFormatLine& lineStyle, double dblVal, BOOL bIsMajor);
	virtual void OnDrawAxisLabel(CBCGPGraphicsManager* pGM, double dblCurrValue, CString& strLabel, CBCGPChartAxis* pAxis, 
		const CBCGPRect& rectLabel, const CBCGPRect& rectLabels, const CBCGPRect& rectDiagram);
	virtual void OnFillAxisUnitInterval(CBCGPGraphicsManager* pGM, const CBCGPRect& rectInterval, const CBCGPBrush& brFill);
	virtual void OnFillAxisUnitInterval(CBCGPGraphicsManager* pGM, const CBCGPGeometry& gInterval, const CBCGPBrush& brFill);

	virtual void OnDrawChartLegendKey(CBCGPGraphicsManager* pGM, const CBCGPRect& rectLegendKey, 
		const BCGPChartFormatSeries* pSeriesStyle, CBCGPChartSeries* pSeries, int nDataPointIndex);
	virtual void OnDrawChartLegendLabel(CBCGPGraphicsManager* pGM, CString& strLabel, 
		CBCGPRect& rectLabel, const BCGPChartFormatLabel& labelStyle, CBCGPChartSeries* pSeries, int nDataPointIndex);
	virtual void OnDrawChartLegendKeyEx(CBCGPGraphicsManager* pGM, const CBCGPRect& rectLegendKey, 
		CBCGPChartLegendCell* pCell);
	virtual void OnDrawChartLegendLabelEx(CBCGPGraphicsManager* pGM, const CString& strLabel, 
							const CBCGPRect& rectLabel, CBCGPChartLegendCell* pCell);

	virtual void OnDrawChartElement(CBCGPGraphicsManager* pGM, const CBCGPRect& rect, const BCGPChartFormatArea& format, CBCGPChartVisualObject::ChartElement chartElement, 
		BOOL bNoFill, BOOL bNoLine);
	virtual void OnDrawChartElement(CBCGPGraphicsManager* pGM, const CBCGPEllipse& ellipse, const BCGPChartFormatArea& format, CBCGPChartVisualObject::ChartElement chartElement, 
		BOOL bNoFill, BOOL bNoLine);
	virtual void OnDrawChartElement(CBCGPGraphicsManager* pGM, const CBCGPGeometry& geometry, const BCGPChartFormatArea& format, CBCGPChartVisualObject::ChartElement chartElement, 
		BOOL bNoFill, BOOL bNoLine);
	
	virtual CBCGPSize OnGetTextSize(CBCGPGraphicsManager* pGM, const CString& strText, const CBCGPTextFormat& textFormat, double dblWidth = 0.);

	virtual void OnDrawChartSeriesLine(CBCGPGraphicsManager* pGM, const CBCGPPoint& ptStart, const CBCGPPoint& ptEnd, 
		CBCGPChartSeries* pSeries, int nDataPointIndex);
	virtual void OnDrawChartSeriesItem(CBCGPGraphicsManager* pGM, const CBCGPRect& rect, 
		CBCGPChartSeries* pSeries, int nDataPointIndex, CBCGPChartVisualObject::ChartElement chartElement, BOOL bNoFill, BOOL bNoLine);
	virtual void OnDrawChartSeriesItem(CBCGPGraphicsManager* pGM, const CBCGPEllipse& ellipse, 
		CBCGPChartSeries* pSeries, int nDataPointIndex, CBCGPChartVisualObject::ChartElement chartElement, BOOL bNoFill, BOOL bNoLine);
	virtual void OnDrawChartSeriesItem(CBCGPGraphicsManager* pGM, const CBCGPGeometry& geometry, 
		CBCGPChartSeries* pSeries, int nDataPointIndex, CBCGPChartVisualObject::ChartElement chartElement, BOOL bNoFill, BOOL bNoLine);
	virtual void OnDrawChartSeriesDataLabel(CBCGPGraphicsManager* pGM, const CBCGPRect& rectText,
		const CString& strText, CBCGPChartSeries* pSeries, int nDataPointIndex);
	virtual void OnDrawChartSeriesMarker(CBCGPGraphicsManager* pGM, const CBCGPPoint& ptMarkerCenter, 
		const BCGPChartFormatMarker& markerFormat, CBCGPChartSeries* pSeries, int nDataPointIndex,
		BOOL bAsMainElement = FALSE);
	virtual void OnDrawChartSeriesMarkerEx(CBCGPGraphicsManager* pGM, const CBCGPPoint& ptMarkerCenter, 
		const CBCGPSize& szMarker, const CBCGPBrush& brFill, const BCGPChartFormatLine& lineStyle, 
		BCGPChartMarkerOptions::MarkerShape shape);

	virtual void OnDrawAxisScaleBreak(CBCGPGraphicsManager* pGM, CBCGPRect rectScaleBreak, 
		CBCGPChartAxisScaleBreak::AxisScaleBreakStyle scaleBreakStyle, const CBCGPBrush& brFill, const CBCGPBrush& brLine, 
		double dblLineWidth, const CBCGPStrokeStyle& strokeStyle, double dblScaleBreakStartValue, double dblScaleBreakEndValue, 
		CBCGPChartAxis* pAxis, const CBCGPRect& rectDiagram);

	virtual void OnDrawChartRect(CBCGPGraphicsManager* pGM, const CBCGPRect& rect, 
		const CBCGPBrush* pBrFill, const CBCGPBrush* pBrLine, double dblLineWidth, 
		const CBCGPStrokeStyle* pStrokeStyle, BOOL bNoFill = FALSE, BOOL bNoLine = FALSE);
	virtual void OnDrawChartEllipse(CBCGPGraphicsManager* pGM, const CBCGPEllipse& ellipse, const CBCGPBrush* pBrFill, 
		const CBCGPBrush* pBrLine, double dblLineWidth, const CBCGPStrokeStyle& stroke, 
		BOOL bNoFill = FALSE, BOOL bNoLine = FALSE);
	virtual void OnDrawChartGeometry(CBCGPGraphicsManager* pGM, const CBCGPGeometry& geometry, 
		const CBCGPBrush* pBrFill, const CBCGPBrush* pBrLine,
		double dblLineWidth, const CBCGPStrokeStyle& stroke, 
		BOOL bNoFill = FALSE, BOOL bNoLine = FALSE);

	virtual void OnAfterBegin3DDraw(CBCGPGraphicsManager* pGM);
	virtual void OnBeforeEnd3DDraw(const CBCGPRect& rectTarget, CBCGPGraphicsManager* pGMTarget);

// Implementation
public:
	virtual ~CBCGPChartVisualObject();

protected:
	BOOL NeedDisplayAxes();
	BOOL GetElementColors(const BCGPChartFormatArea& format, CBCGPChartVisualObject::ChartElement element, 
		CBCGPElementColors& colors);
	virtual int OnAddSeries(CBCGPChartSeries* pSeries, int nSeriesIndex);
	virtual BOOL HitTestInternal(const CBCGPPoint& pt, BCGPChartHitInfo* pHitInfo);

	virtual void OnAfterRecalcLayout(CBCGPGraphicsManager* pGM);
	virtual void OnColorThemeChanged();

	void UpdateSeriesColors();

	void ArrangeStackedSeries();
	BOOL CalcScreenPositions(CBCGPGraphicsManager* pGM, BOOL bDirty);

	virtual void DestroyDiagram3D();

	void ArrangeDataLabels(CBCGPGraphicsManager* pGM);
	CRect CalcSmartRect(const CBCGPPoint& ptCenter, const CSize& size, int nAngle, int nDistance, BOOL bTreatAngleAsSide);

	void CalcMarkerDisplayParams(CBCGPGraphicsManager* pGM, double dblFormattedSize, 
		const CBCGPPoint& ptMarkerCenter, double dblLineWidth, int& nMarkerSize, 
		double& dblMarkerSize2, CBCGPPoint& ptCenter);
	void CreateMarkerShape(const CBCGPPoint& ptCenter, int nMarkerSize, 
		double dblMarkerSize2, BCGPChartMarkerOptions::MarkerShape shape, CBCGPPolygonGeometry& geometry);

	void NormalizeValueInAxis(const CBCGPChartAxis* pAxis, CBCGPChartValue& val) const;

	virtual BOOL IsDataTableSupported();

//Some overrides
public:
	BOOL FireMouseMessage(UINT nMsg, int nButton, const CBCGPPoint& pt, BOOL bUseLastHit = FALSE);

	virtual void OnAxisScrolled(CBCGPChartAxis* pAxis);
	virtual void OnAxisZoomed(CBCGPChartAxis* pAxis);

	virtual BOOL  OnGetMaxLabelSize(CBCGPChartAxis* pAxis, CBCGPSize& szLabel)
	{
		UNREFERENCED_PARAMETER(pAxis);
		UNREFERENCED_PARAMETER(szLabel);
		return FALSE;
	}

	virtual BOOL OnGetDisplayedLabel(CBCGPChartAxis* pAxis, double dblValue, CString& strLabel)
	{
		UNREFERENCED_PARAMETER(pAxis);
		UNREFERENCED_PARAMETER(dblValue);
		UNREFERENCED_PARAMETER(strLabel);
		return FALSE;
	}

	virtual BOOL OnGetPlotAreaRect(CBCGPRect& rectPlotArea) const
	{
		rectPlotArea = m_rectPlotArea;
		return FALSE;
	}

	virtual BOOL OnGetLegendAreaRect(CBCGPGraphicsManager* pGM, CBCGPRect& rectLegendArea)
	{
		UNREFERENCED_PARAMETER(pGM);
		rectLegendArea = m_rectLegendArea; // for compatibility; after 17.0 use GetLegendArea to obtain the rect
		return FALSE;
	}

	virtual BOOL OnGetTitleAreaRect(CBCGPRect& rectTitleArea) const
	{
		rectTitleArea = m_rectTitleArea;
		return FALSE;
	}

	virtual BOOL OnDrawChartObjectShape(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagram, CBCGPChartObject* pChartObject) const
	{
		UNREFERENCED_PARAMETER(pGM);
		UNREFERENCED_PARAMETER(pChartObject);
		UNREFERENCED_PARAMETER(rectDiagram);
		return FALSE;
	}

	virtual BOOL OnDrawChartObjectText(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagram, CBCGPChartObject* pChartObject) const
	{
		UNREFERENCED_PARAMETER(pGM);
		UNREFERENCED_PARAMETER(pChartObject);
		UNREFERENCED_PARAMETER(rectDiagram);
		return FALSE;
	}

	virtual BOOL OnGetAxisNameAreaRect(CBCGPChartAxis* pAxis, CBCGPRect& rectAxisNameArea);
	virtual CBCGPSize OnCalcAxisNameSize(CBCGPGraphicsManager* pGM, CBCGPChartAxis* pAxis);

	virtual double OnGetWallThickness() const;

	virtual BOOL OnGetToolTip(const CBCGPPoint& pt, CString& strToolTip, CString& strDescr);

	virtual HRESULT get_accName(VARIANT varChild, BSTR *pszName)
	{
		if ((varChild.vt == VT_I4) && (varChild.lVal == CHILDID_SELF))
		{
			CString strName = GetChartTitle();
			if (strName.IsEmpty())
			{
				strName = GetName();
			}

			if (!strName.IsEmpty())
			{
				*pszName = strName.AllocSysString();
				return S_OK;
			}
		}
		
		return S_FALSE;
	}

	virtual HRESULT get_accRole(VARIANT varChild, VARIANT *pvarRole)
	{
		HRESULT hr = CBCGPBaseVisualObject::get_accRole(varChild, pvarRole);
		if (SUCCEEDED(hr))
		{
			pvarRole->lVal = ROLE_SYSTEM_CHART;
		}
		
		return hr;
	}
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPCHARTVISUALOBJECT_H__927A0486_C998_41ED_91EA_82593504FBB8__INCLUDED_)

