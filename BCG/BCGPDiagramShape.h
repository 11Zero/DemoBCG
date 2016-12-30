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
// BCGPDiagramShape.h: interface for the CBCGPDiagramShape class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPDIAGRAMSHAPE_H__D398297F_D298_496B_B7CF_25A1BE568972__INCLUDED_)
#define AFX_BCGPDIAGRAMSHAPE_H__D398297F_D298_496B_B7CF_25A1BE568972__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGPDiagramVisualObject.h"

//////////////////////////////////////////////////////////////////////
// CBCGPDiagramShape

class BCGCBPRODLLEXPORT CBCGPDiagramShape : public CBCGPDiagramVisualObject
{
	friend class CBCGPVisualCollector;
	friend class CBCGPVisualConstructor;

	DECLARE_DYNCREATE(CBCGPDiagramShape)
		
public:
	enum Shape
	{
		FirstShape    = 0,
		Box           = FirstShape,
		Ellipse       = 1,
		Star          = 2,
		RoundBox      = 3,
		Rhombus       = 4,
		Rectangle     = 5,
		Parallelogram = 6,
		Trapezoid     = 7,
		Cloud         = 8,
		Custom        = 9,
		LastShape     = Custom
	};

	virtual ~CBCGPDiagramShape ();
	CBCGPDiagramShape(const CBCGPRect& rect, const CBCGPBrush& brFill, const CBCGPBrush& brOutline, const Shape shape, const CBCGPBrush& brShadow = CBCGPBrush(CBCGPColor(.1, .1, .1, .2)));
	CBCGPDiagramShape(const CBCGPDiagramShape& src);

 	virtual void CopyFrom(const CBCGPBaseVisualObject& srcObj);

	Shape GetShape () const { return m_shape; }
	void SetShape (Shape shape);

	virtual void OnDraw(CBCGPGraphicsManager* pGM, const CBCGPRect& rectClip, DWORD dwFlags = BCGP_DRAW_STATIC | BCGP_DRAW_DYNAMIC);
	virtual void DrawTextData(CBCGPGraphicsManager* pGM, const CBCGPRect& rectClip);
	virtual CBCGPGeometry* CreateGeometry (CBCGPGraphicsManager* pGM, BOOL bShadow = FALSE);

// Overrides:
public:
	virtual void SetConnectionPorts();
	
// Implementation:
protected:
	CBCGPDiagramShape();

	void CleanUp ();
	
protected:
	Shape					m_shape;
	CBCGPGeometry*			m_pGeometry;
	CBCGPGeometry*			m_pGeometryShadow;
};

//////////////////////////////////////////////////////////////////////
// CBCGPDiagramCustomShape

class BCGCBPRODLLEXPORT CBCGPDiagramCustomShape : public CBCGPDiagramShape
{
	friend class CBCGPVisualCollector;
	friend class CBCGPVisualConstructor;

	DECLARE_DYNCREATE(CBCGPDiagramCustomShape)
		
public:
	virtual ~CBCGPDiagramCustomShape ();
	CBCGPDiagramCustomShape(const CBCGPRect& rect, const CBCGPBrush& brFill, const CBCGPBrush& brOutline, const CBCGPBrush& brShadow = CBCGPBrush(CBCGPColor(.1, .1, .1, .2)));
	CBCGPDiagramCustomShape(const CBCGPDiagramCustomShape& src);

	virtual void CopyFrom(const CBCGPBaseVisualObject& srcObj);

// Operations:
	void StartPart (BOOL bShadow = FALSE, const CBCGPPoint& ptShadowOffset = CBCGPPoint(0., 0.), const CBCGPPoint& ptStart = CBCGPPoint(-1., -1.), BOOL bIsClosed = TRUE);

	void SetClosed(BOOL bIsClosed = TRUE);
	void SetStart(const CBCGPPoint& ptStart);

	void AddLine(const CBCGPPoint& pt);
	void AddBezier(const CBCGPPoint& pt1, const CBCGPPoint& pt2, const CBCGPPoint& pt3);
	void AddArc(const CBCGPPoint& pt, const CBCGPSize szRadius, 
		BOOL bIsClockwise = TRUE, BOOL bIsLargeArc = FALSE, double dblRotationAngle = 0.);
	void AddPoints(const CBCGPPointsArray& arPoints, CBCGPPolygonGeometry::BCGP_CURVE_TYPE curveType);

	void SetCanvasRect (const CBCGPRect& rect);

// Overrides:
public:
	virtual void OnDraw(CBCGPGraphicsManager* pGM, const CBCGPRect& rectClip, DWORD dwFlags = BCGP_DRAW_STATIC | BCGP_DRAW_DYNAMIC);
	virtual CBCGPGeometry* CreateGeometry (CBCGPGraphicsManager* pGM, BOOL bShadow = FALSE);

// Implementation:
protected:
	CBCGPDiagramCustomShape();

	void CleanUp (BOOL bAll = TRUE);

protected:
	class BaseSegment : public CBCGPPointsArray
	{
	public:
		BaseSegment() {}
		BaseSegment(const BaseSegment& src);
		virtual ~BaseSegment() {}
		virtual BaseSegment* CreateCopy () { return new BaseSegment(*this); }
		virtual BOOL AddToGeometry (CBCGPComplexGeometry* /*pGeometry*/, const CBCGPPoint& /*ptOffset*/, const CBCGPSize& /*szScale*/) { return FALSE; }
	};

	class LineSegment : public BaseSegment
	{
	public:
		LineSegment() {}
		LineSegment(const CBCGPPoint& pt);
		LineSegment(const LineSegment& src);
		virtual BaseSegment* CreateCopy () { return new LineSegment(*this); }
		virtual BOOL AddToGeometry (CBCGPComplexGeometry* pGeometry, const CBCGPPoint& ptOffset, const CBCGPSize& szScale);
	};

	class BezierSegment : public BaseSegment
	{
	public:
		BezierSegment() {}
		BezierSegment(const CBCGPPoint& pt1, const CBCGPPoint& pt2, const CBCGPPoint& pt3);
		BezierSegment(const BezierSegment& src);
		virtual BaseSegment* CreateCopy () { return new BezierSegment(*this); }
		virtual BOOL AddToGeometry (CBCGPComplexGeometry* pGeometry, const CBCGPPoint& ptOffset, const CBCGPSize& szScale);
	};

	class ArcSegment : public BaseSegment
	{
	public:
		ArcSegment();
		ArcSegment(const CBCGPPoint& pt, const CBCGPSize szRadius, 
			BOOL bIsClockwise = TRUE, BOOL bIsLargeArc = FALSE, double dblRotationAngle = 0.);
		ArcSegment(const ArcSegment& src);
		virtual BaseSegment* CreateCopy () { return new ArcSegment(*this); }
		virtual BOOL AddToGeometry (CBCGPComplexGeometry* pGeometry, const CBCGPPoint& ptOffset, const CBCGPSize& szScale);

		CBCGPSize	m_szRadius;
		BOOL		m_bIsClockwise;
		BOOL		m_bIsLargeArc;
		double		m_dblRotationAngle;
	};

	class PointsSegment : public BaseSegment
	{
	public:
		PointsSegment();
		PointsSegment(const CBCGPPointsArray& arPoints, CBCGPPolygonGeometry::BCGP_CURVE_TYPE curveType);
		PointsSegment(const PointsSegment& src);
		virtual BaseSegment* CreateCopy () { return new PointsSegment(*this); }
		virtual BOOL AddToGeometry (CBCGPComplexGeometry* pGeometry, const CBCGPPoint& ptOffset, const CBCGPSize& szScale);

		CBCGPPolygonGeometry::BCGP_CURVE_TYPE m_curveType;
	};

	class CustomShapePart
	{
	public:
		CustomShapePart(BOOL bShadow = FALSE, const CBCGPPoint& ptShadowOffset = CBCGPPoint(0., 0.), const CBCGPPoint& ptStart = CBCGPPoint(-1., -1.), BOOL bIsClosed = TRUE);
		CustomShapePart(const CustomShapePart& src);
		~CustomShapePart();

		CBCGPComplexGeometry* CreateGeometry (CBCGPGraphicsManager* pGM, const CBCGPPoint& ptOffset, const CBCGPSize& szScale);
		void Offset (const CBCGPPoint& pt);

	public:
		typedef CArray<BaseSegment*, BaseSegment*> SegmentsArray;

		BOOL		m_bIsClosed;
		CBCGPPoint	m_ptStart;

		SegmentsArray m_arrSegments;

		BOOL		m_bShadow;
		CBCGPPoint	m_ptShadowOffset;
	};

protected:
	CArray <CustomShapePart*, CustomShapePart*> m_arrParts;
	CBCGPRect	m_rectCanvas;

	CArray <CBCGPComplexGeometry*,CBCGPComplexGeometry*> m_arrGeometries;
	CArray <CBCGPComplexGeometry*,CBCGPComplexGeometry*> m_arrGeometriesShadow;
};

//////////////////////////////////////////////////////////////////////
// CBCGPDiagramImageObject

class BCGCBPRODLLEXPORT CBCGPDiagramImageObject : public CBCGPDiagramVisualObject
{
	DECLARE_DYNCREATE(CBCGPDiagramImageObject)

	friend class CBCGPVisualCollector;
	friend class CBCGPVisualConstructor;

public:
	enum VerticalAlign
	{
		VA_First   = 0,
		VA_Top     = VA_First,
		VA_Center  = 1,
		VA_Bottom  = 2,
		VA_Stretch = 3,
		VA_Last    = VA_Stretch
	};

	enum HorizontalAlign
	{
		HA_First   = 0,
		HA_Left    = HA_First,
		HA_Center  = 1,
		HA_Right   = 2,
		HA_Stretch = 3,
		HA_Last    = HA_Stretch
	};

public:
	CBCGPDiagramImageObject(UINT uiImageResID);
	CBCGPDiagramImageObject(const CBCGPImage& image);
	CBCGPDiagramImageObject(const CBCGPDiagramImageObject& src);
	CBCGPDiagramImageObject(const CBCGPRect& rectBound);
	virtual ~CBCGPDiagramImageObject ();

	virtual void CopyFrom(const CBCGPBaseVisualObject& srcObj);

	void SetImage (UINT uiImageResID, BOOL bRedraw = FALSE);
	void SetImage (const CBCGPImage& image, BOOL bRedraw = FALSE);

 	void SetImageAlign (HorizontalAlign horzAlign, VerticalAlign vertAlign, BOOL bLockAspectRatio = TRUE, BOOL bRedraw = FALSE);
	
	HorizontalAlign GetHorizontalAlign() const
	{
		return m_HorizontalAlign;
	}

	VerticalAlign GetVerticalAlign() const
	{
		return m_VerticalAlign;
	}

	BOOL IsAspectRatioLocked() const
	{
		return m_bLockAspectRatio;
	}

	CBCGPSize GetImageSize(CBCGPGraphicsManager* pGM);

// Overrides:
public:
	virtual void OnDraw(CBCGPGraphicsManager* pGM, const CBCGPRect& rectClip, DWORD dwFlags = BCGP_DRAW_STATIC | BCGP_DRAW_DYNAMIC);

// Implementation:
protected:
	CBCGPDiagramImageObject();

	void Init ();

protected:
	CBCGPImage		m_Image;
 	VerticalAlign	m_VerticalAlign;
 	HorizontalAlign m_HorizontalAlign;
	BOOL			m_bLockAspectRatio;
};

//////////////////////////////////////////////////////////////////////
// CBCGPDiagramTableShape

class BCGCBPRODLLEXPORT CBCGPDiagramTableShape : public CBCGPDiagramShape
{
	friend class CBCGPVisualCollector;
	friend class CBCGPVisualConstructor;

	DECLARE_DYNCREATE(CBCGPDiagramTableShape)

public:
	CBCGPDiagramTableShape(const CBCGPRect& rect, const CBCGPBrush& brFill, const CBCGPBrush& brOutline, const CBCGPBrush& brShadow = CBCGPBrush(CBCGPColor(.1, .1, .1, .2)));
	CBCGPDiagramTableShape(const CBCGPDiagramTableShape& rect);

	virtual void CopyFrom(const CBCGPBaseVisualObject& srcObj);

	void SetCaption (const CString& strText, const CBCGPColor& clrText = CBCGPColor::Black);
	void SetCaption (const CString& strText, const CBCGPColor& clrText, const CBCGPBrush& brCaptionFill, BOOL bShowCaption = TRUE);
	CString GetCaptionText() const;
	CBCGPColor GetCaptionTextColor() const;

	const CBCGPColor& GetCaptionFillColor() const;
	void SetCaptionFillBrush(const CBCGPBrush& brush);
	const CBCGPBrush& GetCaptionFillBrush() const;

	void EnableCaption (BOOL bEnable = TRUE);
	BOOL IsCaptionEnabled () const;

	UINT ConnectionPortID (int nRowIndex, BOOL bLeft) const;

// Overrides:
public:
	virtual void OnDraw(CBCGPGraphicsManager* pGM, const CBCGPRect& rectClip, DWORD dwFlags = BCGP_DRAW_STATIC | BCGP_DRAW_DYNAMIC);
	virtual void DrawTextData(CBCGPGraphicsManager* pGM, const CBCGPRect& rectClip);
	virtual void OnScaleRatioChanged(const CBCGPSize& sizeScaleRatioOld);
	virtual void SetConnectionPorts();
	virtual UINT HitTestConnectionPort (const CBCGPPoint& pt) const;

// Implementation:
protected:
	CBCGPDiagramTableShape();

	void Repos ();

protected:
	BOOL		m_bRepos;

	BOOL		m_bCaption;
	CBCGPDiagramTextDataObject	m_CaptionData;
	CBCGPBrush	m_brCaptionFill;
};

#endif // !defined(AFX_BCGPDIAGRAMSHAPE_H__D398297F_D298_496B_B7CF_25A1BE568972__INCLUDED_)
