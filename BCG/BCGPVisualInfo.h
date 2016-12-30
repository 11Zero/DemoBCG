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
// BCGPVisualInfo.h: interface for the CBCGPCircularGaugeImpl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPVISUALINFO_H__38DF9C07_6F92_433D_B7D0_2CBB09EDD9D6__INCLUDED_)
#define AFX_BCGPVISUALINFO_H__38DF9C07_6F92_433D_B7D0_2CBB09EDD9D6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "bcgcbpro.h"
#include "BCGPBaseInfo.h"

#include "BCGPCircularGaugeImpl.h"
#include "BCGPKnob.h"
#include "BCGPLinearGaugeImpl.h"
#include "BCGPNumericIndicatorImpl.h"
#include "BCGPColorIndicatorImpl.h"
#include "BCGPImageGaugeImpl.h"
#include "BCGPSwitchImpl.h"
#include "BCGPTagCloud.h"
#include "BCGPTreeMap.h"
#include "BCGPWinUITiles.h"
#include "BCGPDiagramVisualObject.h"
#include "BCGPDiagramConnector.h"
#include "BCGPDiagramShape.h"

class CBCGPVisualInfo: public CBCGPBaseInfo  
{
public:
	static CBCGPBaseInfo::XBase* CreateBaseFromName (const CString& name);
	static CBCGPBaseInfo::XBase* CreateBaseFromTag (const CString& tag);

	class XImage
	{
	public:
		XImage ();
		virtual ~XImage ();

		virtual BOOL FromTag (const CString& strTag);
		virtual void ToTag (CString& strTag) const;

	public:
		XID			m_ID;
		CString		m_strType;
		CString		m_strPath;
		CSize		m_sizeIcon;
		BOOL		m_bIsAlphaIcon;
		BOOL		m_bIsIgnoreAlphaBitmap;
		BOOL		m_bMap3dColorsInGDI;
		double		m_dblLightRatio;
		CBCGPColor	m_clrTheme;
		CBCGPSize	m_sizeDest;
	};

	class XData
	{
	public:
		virtual ~XData ();

		static XData* CreateFromName (const CString& name);
		static XData* CreateFromTag (const CString& tag);

		virtual BOOL FromTag (const CString& strTag);
		virtual void ToTag (CString& strTag) const;

		const CString& GetName() const
		{
			return m_strName;
		}

	protected:
		XData (const CString& strName);
		CString		m_strName;

	public:
		double		m_dblValue;
	};
	typedef CArray<XData*, XData*> XArrayData;

	class XElement: public CBCGPBaseInfo::XBase
	{
	protected:
		XElement(const CString& strElementName);

	public:
		virtual ~XElement ();

		virtual BOOL FromTag (const CString& strTag);
		virtual void ToTag (CString& strTag) const;

	public:
		XID			m_ID;
		CRect		m_Rect;
		BOOL		m_bIsVisible;
		BOOL		m_bIsAutoDestroy;
	};
	typedef CArray<XElement*, XElement*> XArrayElement;

	class XContainer: public CBCGPBaseInfo::XBase
	{
	public:
		XContainer();
		virtual ~XContainer();

		virtual BOOL FromTag (const CString& strTag);
		virtual void ToTag (CString& strTag) const;

	public:
		BOOL			m_bDrawDynamicObjectsOnTop;
		CRect			m_Rect;
		CBCGPBrush		m_brFill;
		CBCGPBrush		m_brOutline;

		XArrayElement	m_arElements;
	};

	// Gauge
	class XGaugeScale
	{
	public:
		XGaugeScale ();
		virtual ~XGaugeScale ();

		virtual BOOL FromTag (const CString& strTag);
		virtual void ToTag (CString& strTag) const;

	public:
		double			m_dblStart;
		double			m_dblFinish;
		double			m_dblStep;
		double			m_dblOffsetFromFrame;
		CString			m_strLabelFormat;
		double			m_dblMajorTickMarkStep;
		double			m_dblMinorTickMarkSize;
		double			m_dblMajorTickMarkSize;
		CBCGPGaugeScaleObject::BCGP_TICKMARK_STYLE
						m_MinorTickMarkStyle;
		CBCGPGaugeScaleObject::BCGP_TICKMARK_STYLE
						m_MajorTickMarkStyle;
		CBCGPGaugeScaleObject::BCGP_TICKMARK_POSITION
						m_MinorTickMarkPosition;
		CBCGPBrush		m_brFill;
		CBCGPBrush		m_brOutline;
		CBCGPBrush		m_brText;
		CBCGPBrush		m_brTickMarkMinor;
		CBCGPBrush		m_brTickMarkMinorOutline;
		CBCGPBrush		m_brTickMarkMajor;
		CBCGPBrush		m_brTickMarkMajorOutline;
	};
	typedef CArray<XGaugeScale*, XGaugeScale*> XArrayGaugeScale;

	class XGaugeData : public XData
	{
	public:
		virtual ~XGaugeData ();

		virtual BOOL FromTag (const CString& strTag);
		virtual void ToTag (CString& strTag) const;

	protected:
		XGaugeData (const CString& strName);

	public:
		int			m_nScale;
		CBCGPBrush	m_brFill;
		CBCGPBrush	m_brOutline;
	};

	class XGaugeColoredRange
	{
	public:
		XGaugeColoredRange ();
		virtual ~XGaugeColoredRange ();

		virtual BOOL FromTag (const CString& strTag);
		virtual void ToTag (CString& strTag) const;

	public:
		int			m_nScale;
		double		m_dblStartValue;
		double		m_dblFinishValue;
		double		m_dblStartWidth;
		double		m_dblFinishWidth;
		double		m_dblOffsetFromFrame;
		CBCGPBrush	m_brFill;
		CBCGPBrush	m_brOutline;
		CBCGPBrush	m_brTickMarkFill;
		CBCGPBrush	m_brTickMarkOutline;
		CBCGPBrush	m_brTextLabel;
	};
	typedef CArray<XGaugeColoredRange*, XGaugeColoredRange*> XArrayGaugeColoredRange;

	class XGaugeElement: public XElement
	{
	protected:
		XGaugeElement(const CString& strElementName);

	public:
		virtual ~XGaugeElement();

		static XGaugeElement* CreateFromName (const CString& name);

		virtual BOOL FromTag (const CString& strTag);
		virtual void ToTag (CString& strTag) const;

	protected:
		virtual BOOL ColorsFromTag (const CString& strTag) = 0;
		virtual void ColorsToTag (CString& strTag) const = 0;

	public:
		int						m_nFrameSize;
		CBCGPGaugeImpl::BCGP_SUB_GAUGE_POS
								m_Pos;
		CPoint					m_ptOffset;
		XArrayGaugeScale		m_arScales;
		XArrayGaugeColoredRange	m_arRanges;

		BOOL					m_bIsInteractiveMode;
		CString					m_strToolTip;
		CString					m_strDescription;
	};
	typedef CArray<XGaugeElement*, XGaugeElement*> XArrayGaugeElement;

	class XCircularScale: public XGaugeScale
	{
	public:
		XCircularScale ();
		virtual ~XCircularScale ();

		virtual BOOL FromTag (const CString& strTag);
		virtual void ToTag (CString& strTag) const;

	public:
		double			m_dblStartAngle;
		double			m_dblFinishAngle;
		BOOL			m_bRotateLabels;
		BOOL			m_bIsClosed;
		BOOL			m_bDrawLastTickMark;
		BOOL			m_bAnimationThroughStart;
	};

	class XCircularPointer: public XGaugeData
	{
	public:
		XCircularPointer ();
		virtual ~XCircularPointer ();

		virtual BOOL FromTag (const CString& strTag);
		virtual void ToTag (CString& strTag) const;

	public:
		double		m_dblSize;
		double		m_dblWidth;
		BOOL		m_bExtraLen;
		CBCGPCircularGaugePointer::BCGP_GAUGE_POINTER_STYLE
					m_Style;
	};

	class XCircularColoredRange: public XGaugeColoredRange
	{
	public:
		XCircularColoredRange ();
		virtual ~XCircularColoredRange ();

		virtual BOOL FromTag (const CString& strTag);
		virtual void ToTag (CString& strTag) const;
	};

	class XElementCircular: public XGaugeElement
	{
	protected:
		XElementCircular(const CString& strElementName);

	public:
		XElementCircular();
		virtual ~XElementCircular();

		virtual BOOL FromTag (const CString& strTag);
		virtual void ToTag (CString& strTag) const;

	protected:
		virtual BOOL ColorsFromTag (const CString& strTag);
		virtual void ColorsToTag (CString& strTag) const;

	public:
		CBCGPCircularGaugeColors	m_Colors;
		CBCGPTextFormat				m_fmtText;
		double						m_dblCapSize;
		BOOL						m_bShapeByTicksArea;

		XArrayData					m_arPointers;
		XArrayGaugeElement			m_arSubGauges;
	};

	class XKnobPointer: public XGaugeData
	{
	public:
		XKnobPointer ();
		virtual ~XKnobPointer ();

		virtual BOOL FromTag (const CString& strTag);
		virtual void ToTag (CString& strTag) const;

	public:
		double		m_dblOffsetFromCenter;
		double		m_dblWidth;
		CBCGPKnobPointer::BCGP_KNOB_POINTER_STYLE
					m_Style;
	};

	class XElementKnob: public XElementCircular
	{
	public:
		XElementKnob ();
		virtual ~XElementKnob ();
	};

	class XElementAnalogClock: public XElementCircular
	{
	public:
		XElementAnalogClock ();
		virtual ~XElementAnalogClock ();

		virtual BOOL FromTag (const CString& strTag);
		virtual void ToTag (CString& strTag) const;

	public:
		int	m_nDateIndex;
	};

	class XLinearScale: public XGaugeScale
	{
	public:
		XLinearScale ();
		virtual ~XLinearScale ();

		virtual BOOL FromTag (const CString& strTag);
		virtual void ToTag (CString& strTag) const;
	};

	class XLinearPointer: public XGaugeData
	{
	public:
		XLinearPointer ();
		virtual ~XLinearPointer ();

		virtual BOOL FromTag (const CString& strTag);
		virtual void ToTag (CString& strTag) const;

	public:
		double		m_dblSize;
		double		m_dblWidth;
		CBCGPLinearGaugePointer::BCGP_GAUGE_POINTER_STYLE
					m_Style;
		CBCGPLinearGaugePointer::BCGP_GAUGE_POINTER_POSITION
					m_Position;
	};

	class XLinearColoredRange: public XGaugeColoredRange
	{
	public:
		XLinearColoredRange ();
		virtual ~XLinearColoredRange ();

		virtual BOOL FromTag (const CString& strTag);
		virtual void ToTag (CString& strTag) const;
	};

	class XElementLinear: public XGaugeElement
	{
	public:
		XElementLinear();
		virtual ~XElementLinear();

		virtual BOOL FromTag (const CString& strTag);
		virtual void ToTag (CString& strTag) const;

	protected:
		virtual BOOL ColorsFromTag (const CString& strTag);
		virtual void ColorsToTag (CString& strTag) const;

	public:
		CBCGPLinearGaugeColors		m_Colors;
		CBCGPTextFormat				m_fmtText;
		BOOL						m_bIsVertical;

		XArrayData					m_arPointers;
	};

	class XElementNumeric: public XGaugeElement
	{
	public:
		XElementNumeric();
		virtual ~XElementNumeric();

		virtual BOOL FromTag (const CString& strTag);
		virtual void ToTag (CString& strTag) const;

	protected:
		virtual BOOL ColorsFromTag (const CString& strTag);
		virtual void ColorsToTag (CString& strTag) const;

	public:
		CBCGPNumericIndicatorColors	m_Colors;
		CBCGPTextFormat				m_fmtText;
		CBCGPNumericIndicatorImpl::BCGPNumericIndicatorStyle
									m_Style;
		int							m_nCells;
		int							m_nDecimals;
		int							m_nSeparatorWidth;
		BOOL						m_bDrawSign;
		BOOL						m_bDrawDecimalPoint;
		BOOL						m_bDrawLeadingZeros;
		double						m_dblValue;
	};

	class XStaticGaugeElement: public XGaugeElement
	{
	protected:
		XStaticGaugeElement(const CString& strElementName);

	public:
		virtual ~XStaticGaugeElement();

		virtual BOOL FromTag (const CString& strTag);
		virtual void ToTag (CString& strTag) const;

	public:
		CBCGPBrush		m_brFill;
		CBCGPBrush		m_brOutline;
		double			m_dblOpacity;
		DWORD			m_DefaultDrawFlags;
	};	

	class XElementFrame: public XStaticGaugeElement
	{
	public:
		XElementFrame();
		virtual ~XElementFrame();

		virtual BOOL FromTag (const CString& strTag);
		virtual void ToTag (CString& strTag) const;

	protected:
		virtual BOOL ColorsFromTag (const CString& strTag);
		virtual void ColorsToTag (CString& strTag) const;

	public:
		CBCGPStrokeStyle	m_strokeStyle;
		double				m_dblFrameSize;
		double				m_dblCornerRadius;
	};

	class XElementColor: public XStaticGaugeElement
	{
	public:
		XElementColor();
		virtual ~XElementColor();

		virtual BOOL FromTag (const CString& strTag);
		virtual void ToTag (CString& strTag) const;

	protected:
		virtual BOOL ColorsFromTag (const CString& strTag);
		virtual void ColorsToTag (CString& strTag) const;

	public:
		CBCGPColorIndicatorColors	m_Colors;
		CBCGPColorIndicatorImpl::BCGPColorIndicatorStyle
									m_Style;
		BOOL						m_bStretched;
	};

	class XElementText: public XStaticGaugeElement
	{
	public:
		XElementText();
		virtual ~XElementText();

		virtual BOOL FromTag (const CString& strTag);
		virtual void ToTag (CString& strTag) const;

	protected:
		virtual BOOL ColorsFromTag (const CString& strTag);
		virtual void ColorsToTag (CString& strTag) const;

	public:
		CBCGPBrush		m_brText;
		CBCGPTextFormat	m_fmtText;
		CString			m_strText;
	};

	class XElementImage: public XStaticGaugeElement
	{
	public:
		XElementImage();
		virtual ~XElementImage();

		virtual BOOL FromTag (const CString& strTag);
		virtual void ToTag (CString& strTag) const;

	protected:
		virtual BOOL ColorsFromTag (const CString& strTag);
		virtual void ColorsToTag (CString& strTag) const;

	public:
		XImage	m_Image;
		
		CBCGPImageGaugeImpl::HorizontalAlign m_AlignHorz;
		CBCGPImageGaugeImpl::VerticalAlign m_AlignVert;
		BOOL	m_bLockAspectRatio;
	};

	class XElementSwitch: public XStaticGaugeElement
	{
	public:
		XElementSwitch();
		virtual ~XElementSwitch();

		virtual BOOL FromTag (const CString& strTag);
		virtual void ToTag (CString& strTag) const;

	protected:
		virtual BOOL ColorsFromTag (const CString& strTag);
		virtual void ColorsToTag (CString& strTag) const;

	public:
		CBCGPSwitchColors	m_Colors;
		CBCGPSwitchImpl::BCGP_SWITCH_STYLE
							m_Style;
		CBCGPTextFormat		m_fmtText;
		CString				m_strLabelOff;
		CString				m_strLabelOn;
		BOOL				m_bDrawTextLabels;
		BOOL				m_bValue;
	};

	class XTagCloudData
	{
	public:
		XTagCloudData ();
		~XTagCloudData ();

		BOOL FromTag (const CString& strTag);
		void ToTag (CString& strTag) const;

	public:
		double		m_dblValue;
		CString		m_strLabel;
		CBCGPBrush	m_brFill;
		CBCGPColor	m_clrText;
		CBCGPColor	m_clrTextHighlighted;
		CBCGPColor	m_clrBorder;
	};
	typedef CArray<XTagCloudData*, XTagCloudData*> XArrayTagCloudData;

	class XElementTagCloud: public XElement
	{
	public:
		XElementTagCloud();
		virtual ~XElementTagCloud();

		virtual BOOL FromTag (const CString& strTag);
		virtual void ToTag (CString& strTag) const;

	public:
		CBCGPTagCloud::SortOrder	m_SortOrder;
		BOOL						m_bDescending;
		int							m_nMaxWeight;
		double						m_dblFontSizeStep;
		CBCGPSize					m_szMargin;
		CBCGPTextFormat				m_fmtBase;
		CBCGPBrush					m_brFill;
		CBCGPColor					m_clrText;
		CBCGPColor					m_clrTextHighlighted;

		XArrayTagCloudData			m_arDataObjects;
	};

	class XTreeMapData
	{
	public:
		virtual ~XTreeMapData ();

		static XTreeMapData* CreateFromName (const CString& name);
		static XTreeMapData* CreateFromTag (const CString& tag);

		virtual BOOL FromTag (const CString& strTag);
		virtual void ToTag (CString& strTag) const;

		const CString& GetName() const
		{
			return m_strName;
		}

	protected:
		XTreeMapData (const CString& strName);
		CString		m_strName;

	public:
		double		m_dblValue;
		CString		m_strLabel;
		CBCGPBrush	m_brFill;
		CBCGPSize	m_szMargin;
	};
	typedef CArray<XTreeMapData*, XTreeMapData*> XArrayTreeMapData;

	class XTreeMapNode: public XTreeMapData
	{
	public:
		XTreeMapNode ();
		virtual ~XTreeMapNode ();

		virtual BOOL FromTag (const CString& strTag);
		virtual void ToTag (CString& strTag) const;
	};

	class XTreeMapGroup: public XTreeMapData
	{
	public:
		XTreeMapGroup ();
		virtual ~XTreeMapGroup ();

		virtual BOOL FromTag (const CString& strTag);
		virtual void ToTag (CString& strTag) const;

	public:
		CBCGPTextFormat		m_fmtText;
		CBCGPBrush			m_brText;

		XArrayTreeMapData	m_arNodes;
	};

	class XElementTreeMap: public XElement
	{
	public:
		XElementTreeMap();
		virtual ~XElementTreeMap();

		virtual BOOL FromTag (const CString& strTag);
		virtual void ToTag (CString& strTag) const;

	public:
		CBCGPTreeMap::LayoutType	m_LayoutType;
		XTreeMapGroup				m_Root;
	};

	class XWinUIBaseElement
	{
	protected:
		XWinUIBaseElement();

	public:
		virtual ~XWinUIBaseElement ();

		virtual BOOL FromTag (const CString& strTag);
		virtual void ToTag (CString& strTag) const;

	protected:
		BOOL ViewFromTag (const CString& strTag);
		void ViewToTag (CString& strTag) const;

	public:
		CString					m_strName;
		CString					m_strToolTipText;
		CString					m_strToolTipDescription;
		BOOL					m_bIsVisible;
		CBCGPColor				m_colorText;

		CString					m_strRTI;

		CString					m_strRTIView;
		UINT					m_nViewResID;
		CString					m_strViewTitle;

		CString					m_strCustomProps;
	};

	class XWinUICaptionButton : public XWinUIBaseElement
	{
	public:
		XWinUICaptionButton ();
		~XWinUICaptionButton ();

		virtual BOOL FromTag (const CString& strTag);
		virtual void ToTag (CString& strTag) const;

	public:
		UINT					m_nCommandID;
		XImage					m_Image;
		CString					m_strDescription;
	};
	typedef CArray<XWinUICaptionButton*, XWinUICaptionButton*> XArrayWinUITilesCaptionButton;

	class XWinUIGroupCaption : public XWinUIBaseElement
	{
	public:
		XWinUIGroupCaption ();
		~XWinUIGroupCaption ();

		virtual BOOL FromTag (const CString& strTag);
		virtual void ToTag (CString& strTag) const;

	public:
		int						m_nID;
		BOOL					m_bIsClickable;
		CBCGPBrush				m_brFillGroup;
		CBCGPBrush				m_brOutlineGroup;
	};
	typedef CArray<XWinUIGroupCaption*, XWinUIGroupCaption*> XArrayWinUITilesGroupCaption;

	class XWinUITile : public XWinUIBaseElement
	{
	public:
		XWinUITile ();
		~XWinUITile ();

		BOOL FromTag (const CString& strTag);
		void ToTag (CString& strTag) const;

	public:
		CString					m_strHeader;
		CString					m_strText;

		int						m_nBadgeNumber;
		CBCGPWinUITile::BCGP_WINUI_BADGE_GLYPH
								m_BadgeGlyph;
		int						m_nCustomBadgeIndex;
		XImage					m_Image;
		BOOL					m_bStretchImage;
		int						m_nGroup;
		int						m_nImportance;
		CBCGPWinUITile::BCGP_WINUI_TILE_TYPE
								m_Type;
		CBCGPBrush				m_brushBackground;
		CBCGPColor				m_colorBorder;
		double					m_dblBorderWidth;
	};
	typedef CArray<XWinUITile*, XWinUITile*> XArrayWinUITile;

	class XElementWinUITiles: public XElement
	{
	public:
		XElementWinUITiles();
		virtual ~XElementWinUITiles();

		virtual BOOL FromTag (const CString& strTag);
		virtual void ToTag (CString& strTag) const;

		CBCGPSize GetDefaultSquareSize() const;

	public:
		BOOL				m_bIsHorizontalLayout;
		BOOL				m_bRoundedShapes;
		BOOL				m_bHasNavigationBackButton;
		CRuntimeClass*		m_pRTINavigationBackButton;
		CBCGPSize			m_szMargin;
		CBCGPSize			m_szSquare;
		CString				m_strCaption;
		double				m_dblCaptionExtraHeight;
		CBCGPBrush			m_brFill;
		CBCGPColor			m_colorCaptionForeground;
		XImage				m_CustomBadgeGlyphs;
		BOOL				m_bTilesDragAndDrop;

		XArrayWinUITilesCaptionButton
							m_arCaptionButtons;
		XArrayWinUITilesGroupCaption
							m_arGroupCaptions;
		XArrayWinUITile		m_arTiles;
	};

	// Diagram
	class XDiagramID
	{
	public:
		XDiagramID ();
		virtual ~XDiagramID ();

		virtual BOOL FromTag (const CString& strTag);
		virtual void ToTag (CString& strTag) const;

	public:
		int		m_nID;
		BOOL	m_bConnector;
	};

	class XDiagramAnchorPoint
	{
	public:
		XDiagramAnchorPoint ();
		virtual ~XDiagramAnchorPoint ();

		virtual BOOL FromTag (const CString& strTag);
		virtual void ToTag (CString& strTag) const;

	public:
		XDiagramID	m_idObject;
		UINT		m_nConnectionPort;
		CPoint		m_ptNullAnchor;
	};
	typedef CArray<XDiagramAnchorPoint*, XDiagramAnchorPoint*> XArrayDiagramAnchorPoint;

	class XDiagramArrow
	{
	public:
		XDiagramArrow ();
		virtual ~XDiagramArrow ();

		virtual BOOL FromTag (const CString& strTag);
		virtual void ToTag (CString& strTag) const;

	public:
		CBCGPDiagramConnector::BCGP_ARROW_SHAPE	m_nShape;
		double				m_dLength;
		double				m_dWidth;
		CBCGPBrush			m_brFill;
		CBCGPBrush			m_brOutline;
	};

	class XDiagramTextData : public XData
	{
	public:
		XDiagramTextData ();
		virtual ~XDiagramTextData ();

		virtual BOOL FromTag (const CString& strTag);
		virtual void ToTag (CString& strTag) const;

	public:
		CString			m_strText;
		CBCGPBrush		m_brText;
		CBCGPTextFormat	m_fmtText;
	};

	class XDiagramElement: public XElement
	{
	protected:
		XDiagramElement(const CString& strElementName);

	public:
		virtual ~XDiagramElement();

		static XDiagramElement* CreateFromName (const CString& name);

		virtual BOOL FromTag (const CString& strTag);
		virtual void ToTag (CString& strTag) const;

	public:
		XDiagramID			m_idItem;
		XArrayData			m_arDataObjects;
		CBCGPBrush			m_brFill;
		CBCGPBrush			m_brOutline;
		CBCGPBrush			m_brShadow;
		double				m_Thickness;
		CBCGPStrokeStyle	m_StrokeStyle;

		CString		m_strCustomName;
		CString		m_strCustomProps;
	};

	class XDiagramElementConnector: public XDiagramElement
	{
	protected:
		XDiagramElementConnector(const CString& strElementName);

	public:
		virtual ~XDiagramElementConnector();

		virtual BOOL FromTag (const CString& strTag);
		virtual void ToTag (CString& strTag) const;

	public:
		XArrayDiagramAnchorPoint m_arPoints;
		CBCGPDiagramConnector::BCGP_CURVE_TYPE	m_curveType;
		XDiagramArrow	m_arrowBegin;
		XDiagramArrow	m_arrowEnd;
	};

	class XElementDiagramConnector: public XDiagramElementConnector
	{
	public:
		XElementDiagramConnector();
		virtual ~XElementDiagramConnector();
	};

	class XElementDiagramConnectorShelf: public XDiagramElementConnector
	{
	public:
		XElementDiagramConnectorShelf();
		virtual ~XElementDiagramConnectorShelf();

		virtual BOOL FromTag (const CString& strTag);
		virtual void ToTag (CString& strTag) const;

	public:
		double	m_dShelfOffset;
	};

	class XElementDiagramConnectorElbow: public XDiagramElementConnector
	{
	public:
		XElementDiagramConnectorElbow();
		virtual ~XElementDiagramConnectorElbow();

		virtual BOOL FromTag (const CString& strTag);
		virtual void ToTag (CString& strTag) const;

	public:
		CBCGPDiagramElbowConnector::Orientation	m_Orientation;
		CBCGPPoint								m_ptResizeHandle;
	};

	class XElementDiagramShape: public XDiagramElement
	{
	public:
		XElementDiagramShape();
		virtual ~XElementDiagramShape();

		virtual BOOL FromTag (const CString& strTag);
		virtual void ToTag (CString& strTag) const;

	protected:
		XElementDiagramShape(const CString& strElementName);

	public:
		CBCGPDiagramShape::Shape	m_shape;
	};

	class XElementDiagramTable: public XElementDiagramShape
	{
	public:
		XElementDiagramTable();
		virtual ~XElementDiagramTable();

		virtual BOOL FromTag (const CString& strTag);
		virtual void ToTag (CString& strTag) const;

	public:
		XDiagramTextData	m_CaptionData;
		CBCGPBrush			m_brCaptionFill;
		BOOL				m_bCaption;
	};
	
	class XElementDiagramImage: public XDiagramElement
	{
	public:
		XElementDiagramImage();
		virtual ~XElementDiagramImage();

		virtual BOOL FromTag (const CString& strTag);
		virtual void ToTag (CString& strTag) const;

	public:
		XImage	m_Image;
		
		CBCGPDiagramImageObject::HorizontalAlign m_AlignHorz;
		CBCGPDiagramImageObject::VerticalAlign m_AlignVert;
		BOOL	m_bLockAspectRatio;
	};

	class XElementDiagramCustom: public XDiagramElement
	{
	public:
		XElementDiagramCustom();
		virtual ~XElementDiagramCustom();
	};

public:
	CBCGPVisualInfo();
	virtual ~CBCGPVisualInfo();

	virtual BOOL FromTag (const CString& strTag);
	virtual void ToTag (CString& strTag) const;

	inline XContainer& GetContainer ()
	{
		return m_Container;
	}
	inline const XContainer& GetContainer () const
	{
		return m_Container;
	}

public:
	static LPCTSTR s_szDataPointerCircular;
	static LPCTSTR s_szDataPointerKnob;
	static LPCTSTR s_szDataPointerLinear;
	static LPCTSTR s_szDataDiagramText;
	static LPCTSTR s_szTreeMapNode;
	static LPCTSTR s_szTreeMapGroup;

	static LPCTSTR s_szCircularGauge;
	static LPCTSTR s_szKnob;
	static LPCTSTR s_szAnalogClock;
	static LPCTSTR s_szLinearGauge;
	static LPCTSTR s_szNumericInd;
	static LPCTSTR s_szColorInd;
	static LPCTSTR s_szTextInd;
	static LPCTSTR s_szImage;
	static LPCTSTR s_szSwitch;
	static LPCTSTR s_szFrame;

	static LPCTSTR s_szTagCloud;
	static LPCTSTR s_szTreeMap;
	static LPCTSTR s_szWinUITiles;

	static LPCTSTR s_szDiagramConnector;
	static LPCTSTR s_szDiagramConnectorShelf;
	static LPCTSTR s_szDiagramConnectorElbow;
	static LPCTSTR s_szDiagramShape;
	static LPCTSTR s_szDiagramTable;
	static LPCTSTR s_szDiagramImage;
	static LPCTSTR s_szDiagramCustom;

	static LPCTSTR s_szContainer;

private:
	XContainer	m_Container;
};

class CBCGPVisualInfoLoader: public CBCGPBaseInfoLoader
{
public:
	CBCGPVisualInfoLoader (CBCGPVisualInfo& info);
	virtual ~CBCGPVisualInfoLoader();

protected:
	CBCGPVisualInfo& GetVisualInfo () const
	{
		return (CBCGPVisualInfo&)GetInfo ();
	}
};

class CBCGPVisualInfoWriter: public CBCGPBaseInfoWriter
{
public:
	CBCGPVisualInfoWriter(CBCGPVisualInfo& info);
	virtual ~CBCGPVisualInfoWriter();

	virtual BOOL Save (const CString& strFileName);

protected:
	CBCGPVisualInfo& GetVisualInfo ()
	{
		return (CBCGPVisualInfo&)GetInfo ();
	}
};

#endif // !defined(AFX_BCGPVISUALINFO_H__38DF9C07_6F92_433D_B7D0_2CBB09EDD9D6__INCLUDED_)
