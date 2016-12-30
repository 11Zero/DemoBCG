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
// BCGPDiagramVisualObject.h: interface for the CBCGPDiagramVisualObject class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPDIAGRAMVISUALOBJECT_H__546EB43C_856D_461D_8FF0_908206E295A4__INCLUDED_)
#define AFX_BCGPDIAGRAMVISUALOBJECT_H__546EB43C_856D_461D_8FF0_908206E295A4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGPVisualContainer.h"

#define CONNECTIONPORT_MARKER_SIZE 4
#define CONNECTIONPORT_SHELF_OFFSET 10

//////////////////////////////////////////////////////////////////////
// CBCGPDiagramItemID

class BCGCBPRODLLEXPORT CBCGPDiagramItemID
{
public:
	CBCGPDiagramItemID (int nId = Undefined, BOOL bConnector = FALSE)
		: m_nId (nId)
		, m_bConnector (bConnector)
	{
	}
	CBCGPDiagramItemID (const CBCGPDiagramItemID& src) { m_nId = src.m_nId; m_bConnector = src.m_bConnector; }

	BOOL IsNull () const { return m_nId == Undefined; }

	bool operator==(const CBCGPDiagramItemID& src) const
	{
		return (m_nId == src.m_nId && m_bConnector == src.m_bConnector);
	}
	
	bool operator!=(const CBCGPDiagramItemID& src) const
	{
		return (m_nId != src.m_nId || m_bConnector != src.m_bConnector);
	}

	enum { Undefined = -1 };
	
public:
	int 	m_nId;
	BOOL	m_bConnector;
};

typedef CList<CBCGPDiagramItemID, CBCGPDiagramItemID&> CItemIDList;

//////////////////////////////////////////////////////////////////////
// CBCGPDiagramAnchorPoint

class BCGCBPRODLLEXPORT CBCGPDiagramAnchorPoint
{
public:
	CBCGPDiagramAnchorPoint ();
	CBCGPDiagramAnchorPoint (CBCGPDiagramItemID	idObject, UINT nConnectionPort);
	CBCGPDiagramAnchorPoint (const CBCGPDiagramAnchorPoint& src);

	const CBCGPDiagramAnchorPoint& operator=(const CBCGPDiagramAnchorPoint& src);

	BOOL IsNull () const;
	BOOL IsEqual (const CBCGPDiagramAnchorPoint& src) const;

	static CBCGPDiagramAnchorPoint NullAnchor (CBCGPPoint pt);

public:
	CBCGPDiagramItemID	m_idObject;
	UINT				m_nConnectionPort;

	CBCGPPoint			m_ptNullAnchor;
};

//////////////////////////////////////////////////////////////////////
// CBCGPDiagramVisualObject

class CBCGPDiagramTextDataObject;

class BCGCBPRODLLEXPORT CBCGPDiagramVisualObject : public CBCGPBaseVisualObject
{
	friend class CBCGPDiagramVisualContainer;
	friend class CBCGPVisualConstructor;

	DECLARE_DYNCREATE(CBCGPDiagramVisualObject)

// Construction
public:
	CBCGPDiagramVisualObject(CBCGPVisualContainer* pContainer = NULL);
	CBCGPDiagramVisualObject(const CBCGPDiagramVisualObject& src);
	virtual ~CBCGPDiagramVisualObject();

	virtual CBCGPBaseVisualObject* CreateCopy () const;
	virtual void CopyFrom(const CBCGPBaseVisualObject& srcObj);

// Operations:
public:
	// General attributes:
	void SetItemID (const CBCGPDiagramItemID& id);
	const CBCGPDiagramItemID& GetItemID () const { return m_ItemId; }

	const CBCGPDiagramVisualContainer* GetParentDiagram () const;

	void SetParentContainer (CBCGPVisualContainer* pContainer)
	{
		if (pContainer != NULL)
		{
			ASSERT_VALID (pContainer);
		}
		
		m_pParentContainer = pContainer;
	}

	void SetAutoDestroy (BOOL bAutoDestroy);

	BOOL IsReadOnly () const
	{
		return FALSE;
	}

	// Colors:
	void SetOutlineColor(const CBCGPColor& color);

	const CBCGPColor& GetOutlineColor() const
	{
		return m_brOutline.GetColor ();
	}

	void SetFillColor(const CBCGPColor& color);

	const CBCGPColor& GetFillColor() const
	{
		return m_brFill.GetColor ();
	}

	void SetShadowColor(const CBCGPColor& color);

	const CBCGPColor& GetShadowColor() const
	{
		return m_brShadow.GetColor ();
	}

	void SetOutlineBrush(const CBCGPBrush& brush);

	const CBCGPBrush& GetOutlineBrush() const
	{
		return m_brOutline;
	}

	void SetFillBrush(const CBCGPBrush& brush);

	const CBCGPBrush& GetFillBrush() const
	{
		return m_brFill;
	}

	void SetShadowBrush(const CBCGPBrush& brush);

	const CBCGPBrush& GetShadowBrush() const
	{
		return m_brShadow;
	}

	// Borders:
	void SetThickness (double dThickness);
	double GetThickness () const {	return m_Thickness;	}

	void SetStrokeStyle (const CBCGPStrokeStyle& strokeStyle);
	const CBCGPStrokeStyle& GetStrokeStyle () const {	return m_StrokeStyle;	}

	// Connection points:
	enum ConnectionPorts
	{
		CP_None			= HTNOWHERE,		//0
			
		CP_Center		= HTCLIENT,			//1
		CP_Left			= HTLEFT,			//10
		CP_Right		= HTRIGHT,			//11
		CP_Top			= HTTOP,			//12
		CP_TopLeft		= HTTOPLEFT,		//13
		CP_TopRight		= HTTOPRIGHT,		//14
		CP_Bottom		= HTBOTTOM,			//15
		CP_BottomLeft	= HTBOTTOMLEFT,		//16
		CP_BottomRight	= HTBOTTOMRIGHT,	//17
		
		CP_Begin		= CP_TopLeft,
		CP_End			= CP_BottomRight,

		CP_Caption		= 22,// = HTCAPTION, //2
		CP_ResizeHandle = 23,
		CP_RotateHandle = 24,
		
		CP_CustomFirst	= 32
	};

	virtual void SetConnectionPorts ();
	virtual UINT HitTestConnectionPort (const CBCGPPoint& pt) const;
	BOOL GetConnectionPort (UINT nConnectionPortID, CBCGPPoint& pt) const;
	CBCGPDiagramAnchorPoint UseConnectionPort (UINT nConnectionPortID, CBCGPPoint* pDefaultPoint = NULL) const;

	virtual void DrawConnectionPorts(CBCGPGraphicsManager* pGM, const CBCGPBrush& brOutline, const CBCGPBrush& brFill);

	// Connections:
	struct IncidentEdge
	{
		IncidentEdge () : m_nConnectionPortID (CP_None), m_idIncidentConnector () {}
		IncidentEdge (const IncidentEdge& src) 
		{
			m_nConnectionPortID = src.m_nConnectionPortID;
			m_idIncidentConnector = src.m_idIncidentConnector;
		}

		UINT				m_nConnectionPortID;
		CBCGPDiagramItemID	m_idIncidentConnector;
	};

	BOOL HasConnectors (UINT nConnectionPortID) const;
	BOOL GetConnectors (UINT nConnectionPortID, CItemIDList& lstConnectors) const;
	void GetAllConnectors (CItemIDList& lstConnectors) const;

	virtual void OnConnectionAdded (UINT nConnectionPortID, CBCGPDiagramItemID idIncidentConnector, CBCGPDiagramItemID idAdjacentObject);
	virtual void OnConnectionRemoved (UINT nConnectionPortID, CBCGPDiagramItemID idIncidentConnector, CBCGPDiagramItemID idAdjacentObject);
	virtual void UpdateConnections (BOOL bRedraw);

	virtual void OnAdd (CBCGPDiagramVisualContainer* pContainer);
	virtual void OnRemove (CBCGPDiagramVisualContainer* pContainer);

	// Text:
	int AddTextData (const CString& str, const CBCGPColor& clrText = CBCGPColor::Black);
	CBCGPDiagramTextDataObject* GetTextData (int nIndex);
	BOOL SetTextData (const CString& str, int nIndex, BOOL bRedraw);
	void RemoveTextData (int nIndex, BOOL bRedraw);

	// In-place edit:
	virtual BOOL OnEdit (const CBCGPPoint* pt);
	virtual BOOL OnEndEdit ();
	virtual CWnd* CreateInPlaceEdit (CRect rectEdit, CBCGPDiagramTextDataObject* pTextData, BOOL& bDefaultFormat);
	virtual void OnDestroyWindow ();
	virtual void AdjustInPlaceEditRect (CBCGPRect& rectEdit);
	virtual void SetInPlaceEditFont ();

	BOOL IsInPlaceEdit () const
	{
		return (m_pWndInPlace != NULL && m_nInPlaceEditIndex >= 0);
	}

	CWnd* GetInPlaceEditWnd ()
	{
		return m_pWndInPlace;
	}

	virtual BOOL ValidateInPlaceEdit ();
	virtual BOOL UpdateData ();

	virtual BOOL FromTag(const CString& /*strCustomProps*/) {	return TRUE;	}
	virtual void ToTag(CString& /*strCustomName*/, CString& /*strCustomProps*/) const {}

// Implementation:
protected:

// Overrides:
public:
	virtual void OnDraw (CBCGPGraphicsManager* pGM, const CBCGPRect& rectClip, DWORD dwFlags = BCGP_DRAW_STATIC | BCGP_DRAW_DYNAMIC);
	virtual void SetRect(const CBCGPRect& rect, BOOL bRedraw = FALSE);
	virtual void OnScaleRatioChanged(const CBCGPSize& sizeScaleRatioOld);

// Attributes:
protected:
	CBCGPDiagramItemID		m_ItemId;
	CMap<UINT, UINT, CBCGPPoint, const CBCGPPoint&>	m_mapConnectionPorts;
	CList<IncidentEdge, IncidentEdge&>				m_lstIncidentConnectors;

	int		m_nInPlaceEditIndex;
	CWnd*	m_pWndInPlace;

	CBCGPBrush			m_brOutline;
	CBCGPBrush			m_brFill;
	CBCGPBrush			m_brShadow;

 	double				m_Thickness;
 	CBCGPStrokeStyle	m_StrokeStyle;
};

//////////////////////////////////////////////////////////////////////
// CBCGPDiagramAnchorPointObject

class BCGCBPRODLLEXPORT CBCGPDiagramAnchorPointObject : public CBCGPDiagramVisualObject
{
	DECLARE_DYNCREATE(CBCGPDiagramAnchorPointObject)

// Construction
protected:
	CBCGPDiagramAnchorPointObject();

public:
	CBCGPDiagramAnchorPointObject(CBCGPPoint pt, CBCGPVisualContainer* pContainer = NULL);
	CBCGPDiagramAnchorPointObject(const CBCGPDiagramAnchorPointObject& src);

// Overrides:
	virtual void SetConnectionPorts ();
};

//////////////////////////////////////////////////////////////////////
// CBCGPDiagramConnector

class CBCGPDiagramVisualContainer;

class BCGCBPRODLLEXPORT CBCGPDiagramConnector : public CBCGPDiagramVisualObject
{
	friend class CBCGPVisualConstructor;
	friend class CBCGPDiagramVisualContainer;

	DECLARE_DYNCREATE(CBCGPDiagramConnector)

public:
	enum BCGP_CURVE_TYPE
	{
		BCGP_CURVE_TYPE_FIRST  = 0,
		BCGP_CURVE_TYPE_LINE   = BCGP_CURVE_TYPE_FIRST,
		BCGP_CURVE_TYPE_SPLINE = 1,
		BCGP_CURVE_TYPE_LAST   = BCGP_CURVE_TYPE_SPLINE
	};

    enum BCGP_ARROW_SHAPE
    {
		BCGP_ARROW_SHAPE_FIRST    = 0,
		BCGP_ARROW_NONE           = BCGP_ARROW_SHAPE_FIRST,
		BCGP_ARROW_OPEN           = 1,
		BCGP_ARROW_STEALTH        = 2,
		BCGP_ARROW_TRIANGLE       = 3,
		BCGP_ARROW_FILLEDTRIANGLE = 4,
		BCGP_ARROW_CIRCLE         = 5,
		BCGP_ARROW_FILLEDCIRCLE   = 6,
		BCGP_ARROW_DIAMOND        = 7,
		BCGP_ARROW_FILLEDDIAMOND  = 8,
		BCGP_ARROW_SHAPE_LAST     = BCGP_ARROW_FILLEDDIAMOND
    };

	struct Arrow
	{
		Arrow () : m_nShape (CBCGPDiagramConnector::BCGP_ARROW_NONE), m_dLength (11.0), m_dWidth (3.0), m_ptOffset(0.0, 0.0) {}
		Arrow (const Arrow& src) 
		{
			CopyFrom (src);
		}

		void CopyFrom (const Arrow& src)
		{
			m_nShape = src.m_nShape;
			m_dLength = src.m_dLength;
			m_dWidth = src.m_dWidth;
			m_brFill = src.m_brFill;
			m_brOutline = src.m_brOutline;

			m_ptOffset.SetPoint (0.0, 0.0);
		}

		BOOL IsNull () const
		{
			return (m_nShape == BCGP_ARROW_NONE || m_dLength <= 0 || m_dWidth <= 0);
		}
	
		BCGP_ARROW_SHAPE m_nShape;
		double			 m_dLength;
		double			 m_dWidth;
		CBCGPBrush		 m_brFill;
		CBCGPBrush		 m_brOutline;

		CBCGPPoint		 m_ptOffset;
	};

	typedef CArray<CBCGPDiagramAnchorPoint, CBCGPDiagramAnchorPoint&> AnchorPointsArray;

// Construction
public:
	CBCGPDiagramConnector(CBCGPVisualContainer* pContainer = NULL);
	CBCGPDiagramConnector(const CBCGPDiagramConnector& src);
	CBCGPDiagramConnector(const CBCGPPoint& pt1, const CBCGPPoint& pt2);
	CBCGPDiagramConnector(const CBCGPPoint& pt1, const CBCGPPoint& pt2, const CBCGPPoint& pt3, BCGP_CURVE_TYPE curveType = BCGP_CURVE_TYPE_LINE);
	CBCGPDiagramConnector(const CBCGPPoint& pt1, const CBCGPPoint& pt2, const CBCGPPoint& pt3, const CBCGPPoint& pt4, BCGP_CURVE_TYPE curveType = BCGP_CURVE_TYPE_LINE);
	virtual ~CBCGPDiagramConnector();

	virtual void CopyFrom(const CBCGPBaseVisualObject& srcObj);

// Operations:
public:
	BOOL Connect (CBCGPDiagramVisualObject* pObject1, CBCGPDiagramVisualObject* pObject2,
		UINT ConnectionPortId1 = CP_Center,
		UINT ConnectionPortId2 = CP_Center);
	BOOL Connect (const CBCGPPoint& start, const CBCGPPoint& end);
	BOOL Connect (CBCGPDiagramAnchorPoint start, CBCGPDiagramAnchorPoint end);
	BOOL IsConnected () const;

	BOOL InsertPoint (int nIndex, const CBCGPPoint& pt);
	int GetPoints(CBCGPPointsArray& arPoints) const;
	int GetPointCount () const	{ return (int) m_arPoints.GetSize (); }

	void SetCurveType (BCGP_CURVE_TYPE type);
	BCGP_CURVE_TYPE GetCurveType () const { return m_curveType; }
	
    void SetBeginArrow (BCGP_ARROW_SHAPE shape);
    void SetBeginArrow (BCGP_ARROW_SHAPE shape, double dLength, double dWidth);
    BCGP_ARROW_SHAPE GetBeginArrow (double* pLength = NULL, double* pWidth = NULL) const;
	void SetBeginArrowOutlineBrush (const CBCGPBrush& brush);
	const CBCGPBrush& GetBeginArrowOutlineBrush () const;
	void SetBeginArrowFillBrush (const CBCGPBrush& brush);
	const CBCGPBrush& GetBeginArrowFillBrush () const;
	
    void SetEndArrow (BCGP_ARROW_SHAPE shape);
    void SetEndArrow (BCGP_ARROW_SHAPE shape, double dLength, double dWidth);
    BCGP_ARROW_SHAPE GetEndArrow (double* pLength = NULL, double* pWidth = NULL) const;
	void SetEndArrowOutlineBrush (const CBCGPBrush& brush);
	const CBCGPBrush& GetEndArrowOutlineBrush () const;
	void SetEndArrowFillBrush (const CBCGPBrush& brush);
	const CBCGPBrush& GetEndArrowFillBrush () const;

	CBCGPDiagramAnchorPoint& AnchorPoint (int nIndex);
	int FindAnchorPointIndex (CBCGPDiagramAnchorPoint& anchor) const;
	virtual UINT HitTestAnchorPoint (const CBCGPPoint& pt, int& nIndex) const;

public:
	void BeginTrackAnchorPoint (CBCGPDiagramAnchorPoint anchor);
	void EndTrackAnchorPoint (BOOL bSaveChanges = TRUE);
	BOOL IsTrackingAnchorPoints () const { return m_nTrackPointIndex != -1; }
	const CBCGPDiagramAnchorPoint GetTrackedAnchorPoint () const;
	BOOL SetTrackedAnchorPoint (const CBCGPDiagramAnchorPoint& anchor);
	int GetTrackedPoints(CBCGPPointsArray& arPoints) const;

	CBCGPRect GetBoundsRect () const;

	virtual void OnAdd (CBCGPDiagramVisualContainer* pContainer);
	virtual void OnRemove (CBCGPDiagramVisualContainer* pContainer);
	virtual void DrawConnection(CBCGPGraphicsManager* pGM, const CBCGPBrush& brOutline);
	virtual void DrawBeginArrow(CBCGPGraphicsManager* pGM, const CBCGPBrush& brOutline, const CBCGPBrush& brFill);
	virtual void DrawEndArrow(CBCGPGraphicsManager* pGM, const CBCGPBrush& brOutline, const CBCGPBrush& brFill);

// Implementation:
protected:
	BOOL GetArrowPoints (CBCGPPoint ptStart, CBCGPPoint ptEnd, CBCGPDiagramConnector::Arrow arrow,
		CBCGPPointsArray& arPoints);
	CBCGPGeometry* CreateArrowGeometry (const CBCGPDiagramConnector::Arrow& arrow, const CBCGPPointsArray& arPoints,
		CBCGPPoint& ptLineOffset);
	
	void Init ();
	virtual void RecalcPoints();
	virtual void OnEndTrackingPoints (BOOL bSaveChanges = TRUE);

// Overrides:
public:
	virtual int HitTest(const CBCGPPoint& pt) const;
	virtual BOOL OnSetMouseCursor(const CBCGPPoint& pt);
	virtual void OnDraw(CBCGPGraphicsManager* pGM, const CBCGPRect& rectClip, DWORD dwFlags = BCGP_DRAW_STATIC | BCGP_DRAW_DYNAMIC);
	virtual void DrawTrackingRect(CBCGPGraphicsManager* pGM,
		const CBCGPBrush& brOutline, const CBCGPBrush& brFill);
	virtual void SetRect(const CBCGPRect& rect, BOOL bRedraw = FALSE);
	virtual void SetSelected(BOOL bSet = TRUE);
	virtual void SetConnectionPorts();

protected:
	virtual void SetTrackingRect(const CBCGPRect& rect);
	
// Attributes:
protected:
	AnchorPointsArray	m_arPoints;
	int					m_nTrackPointIndex;
	AnchorPointsArray	m_arTrackPoints;
	BCGP_CURVE_TYPE		m_curveType;

	Arrow	m_arrowBegin;
	Arrow	m_arrowEnd;
};

//////////////////////////////////////////////////////////////////////
// CBCGPDiagramGroupObject

class BCGCBPRODLLEXPORT CBCGPDiagramGroupObject : public CBCGPDiagramVisualObject
{
	DECLARE_DYNCREATE(CBCGPDiagramGroupObject)

// Construction
public:
	CBCGPDiagramGroupObject(CBCGPVisualContainer* pContainer = NULL);
	CBCGPDiagramGroupObject(const CBCGPDiagramConnector& src);
	virtual ~CBCGPDiagramGroupObject();

// Operations:
public:
	CBCGPDiagramItemID AddItem (CBCGPBaseVisualObject* pObject, BOOL bAutoDestroy = TRUE);
	void RemoveItem (CBCGPBaseVisualObject* pObject);
	void RemoveItem (CBCGPDiagramItemID id);
	void RemoveAll ();

	void GetItemList (CList<CBCGPBaseVisualObject*, CBCGPBaseVisualObject*> lstItems);
	void GetItemList (CItemIDList& lstItems);

// Attributes:
protected:
	CMap<int, int, CBCGPBaseVisualObject*, CBCGPBaseVisualObject*> m_mapItems;		// nodes
	CMap<int, int, CBCGPDiagramConnector*, CBCGPDiagramConnector*> m_mapConnectors;	// edges

	CBCGPDiagramGroupObject*	m_pParentGroup;
	CBCGPRect					m_rectBounds;
};

//////////////////////////////////////////////////////////////////////
// CBCGPDiagramTextDataObject

class CBCGPGraphicsManager;

class BCGCBPRODLLEXPORT CBCGPDiagramTextDataObject : public CBCGPVisualDataObject
{
	DECLARE_DYNCREATE(CBCGPDiagramTextDataObject)

// Construction
public:
	CBCGPDiagramTextDataObject(LPCTSTR lpszText = NULL, const CBCGPColor& color = CBCGPColor::Black);
	CBCGPDiagramTextDataObject(LPCTSTR lpszText, const CBCGPBrush& brush);

	CBCGPDiagramTextDataObject & operator = (const CBCGPDiagramTextDataObject & src)
	{
		CopyFrom(src);
		return *this;
	}

// Operations:
public:
	void SetText(LPCTSTR lpszText, BOOL bRedraw = FALSE);
	void SetText(const CString& str, const CBCGPColor& clrText, BOOL bRedraw = FALSE);
	void SetText(const CString& str, const CBCGPBrush& brText, BOOL bRedraw = FALSE);

	const CString& GetText() const
	{
		return m_strText;
	}

	void SetTextColor(const CBCGPColor& color);

	const CBCGPColor& GetTextColor() const
	{
		return m_brText.GetColor ();
	}

	void SetTextBrush(const CBCGPBrush& brush);

	const CBCGPBrush& GetTextBrush() const
	{
		return m_brText;
	}

	const CBCGPTextFormat& GetTextFormat() const
	{
		return m_textFormat;
	}
	
	void SetTextFormat(const CBCGPTextFormat& textFormat);

	const CBCGPSize& GetSize () const
	{
		return m_size;
	}

	void SetSize (const CBCGPSize& size);

	void SetFontScale (double dScaleRatio);

public:
	virtual CBCGPSize GetDefaultSize(CBCGPGraphicsManager* pGM, const CBCGPDiagramVisualObject* pParentObject);
	virtual void Draw(CBCGPGraphicsManager* pGM, const CBCGPRect& rect, const CBCGPRect& rectClip);

	virtual CString ToString () const;
	virtual BOOL ParseString (const CString& str);
	virtual BOOL ValidateString (const CString& str);
	
	virtual CBCGPVisualDataObject* CreateCopy () const
	{
		return new CBCGPDiagramTextDataObject (*this);
	}
	virtual void CopyFrom(const CBCGPVisualDataObject& srcIn)
	{
		CBCGPVisualDataObject::CopyFrom(srcIn);

		const CBCGPDiagramTextDataObject& src = (const CBCGPDiagramTextDataObject&)srcIn;

		m_strText = src.m_strText;
		m_brText = src.m_brText;
		m_textFormat = src.m_textFormat;
		m_size = src.m_size;
	}

	static void SetDefaultTextFormat(CBCGPTextFormat& tf);

// Implementation
protected:
	void CreateResources();

// Attributes:
protected:
	CString			m_strText;
	CBCGPBrush		m_brText;
	CBCGPTextFormat	m_textFormat;
	CBCGPSize		m_size;
};

#endif // !defined(AFX_BCGPDIAGRAMVISUALOBJECT_H__546EB43C_856D_461D_8FF0_908206E295A4__INCLUDED_)
