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
// BCGPVisualInfo.cpp: implementation of the CBCGPVisualInfo class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BCGPVisualInfo.h"
#include "BCGPTagManager.h"
#include "BCGPMath.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

static const DWORD c_dwVersion = MAKELONG (1, 0);

static LPCTSTR s_szTag_Body                   = _T("BCGP_CONTAINER");

static LPCTSTR s_szTag_ResourceID             = _T("RESOURCE_ID");
static LPCTSTR s_szTag_ResourceType           = _T("RESOURCE_TYPE");
static LPCTSTR s_szTag_Path                   = _T("PATH");
static LPCTSTR s_szTag_IconSize               = _T("ICON_SIZE");
static LPCTSTR s_szTag_AlphaIcon              = _T("ALPHA_ICON");
static LPCTSTR s_szTag_IgnoreAlphaBitmap      = _T("IGNORE_ALPHA_BITMAP");
static LPCTSTR s_szTag_LightRatio             = _T("LIGHT_RATIO");
static LPCTSTR s_szTag_MapTo3d                = _T("MAPTO3D");
static LPCTSTR s_szTag_ColorTheme             = _T("COLOR_THEME");
static LPCTSTR s_szTag_SizeDest               = _T("SIZE_DEST");
static LPCTSTR s_szTag_Image                  = _T("IMAGE");
static LPCTSTR s_szTag_AlignHorz              = _T("ALIGN_HORZ");
static LPCTSTR s_szTag_AlignVert              = _T("ALIGN_VERT");
static LPCTSTR s_szTag_LockAspectRatio        = _T("ASPECT_RATIO");

static LPCTSTR s_szTag_ToolTip                = _T("TOOLTIP");
static LPCTSTR s_szTag_Description            = _T("DESCRIPTION");
static LPCTSTR s_szTag_Rect                   = _T("RECT");
static LPCTSTR s_szTag_FrameSize              = _T("FRAME_SIZE");
static LPCTSTR s_szTag_LabelFormat            = _T("LABEL_FORMAT");
static LPCTSTR s_szTag_Visible                = _T("VISIBLE");
static LPCTSTR s_szTag_AutoDestroy            = _T("AUTO_DESTROY");
static LPCTSTR s_szTag_InteractiveMode        = _T("INTERACTIVE");

static LPCTSTR s_szTag_Scales                 = _T("SCALES");
static LPCTSTR s_szTag_Scale                  = _T("SCALE");
static LPCTSTR s_szTag_Scale_Id               = _T("SCALE_ID");
static LPCTSTR s_szTag_Pointers               = _T("POINTERS");
static LPCTSTR s_szTag_Pointer                = _T("POINTER");
static LPCTSTR s_szTag_Ranges                 = _T("RANGES");
static LPCTSTR s_szTag_Range                  = _T("RANGE");
static LPCTSTR s_szTag_Start                  = _T("START");
static LPCTSTR s_szTag_Finish                 = _T("FINISH");
static LPCTSTR s_szTag_Start_Angle            = _T("START_ANGLE");
static LPCTSTR s_szTag_Finish_Angle           = _T("FINISH_ANGLE");
static LPCTSTR s_szTag_Start_Value            = _T("START_VALUE");
static LPCTSTR s_szTag_Finish_Value           = _T("FINISH_VALUE");
static LPCTSTR s_szTag_Start_Width            = _T("START_WIDTH");
static LPCTSTR s_szTag_Finish_Width           = _T("FINISH_WIDTH");
static LPCTSTR s_szTag_Start_Arrow            = _T("START_ARROW");
static LPCTSTR s_szTag_Finish_Arrow           = _T("FINISH_ARROW");
static LPCTSTR s_szTag_Step                   = _T("STEP");
static LPCTSTR s_szTag_DataName               = _T("DATA_NAME");
static LPCTSTR s_szTag_DataValue              = _T("DATA_VALUE");
static LPCTSTR s_szTag_TextValue              = _T("TEXT_VALUE");
static LPCTSTR s_szTag_Length                 = _T("LENGTH");
static LPCTSTR s_szTag_Size                   = _T("SIZE");
static LPCTSTR s_szTag_Width                  = _T("WIDTH");
static LPCTSTR s_szTag_ExtraLen               = _T("EXTRALEN");
static LPCTSTR s_szTag_PointerStyle           = _T("STYLE");
static LPCTSTR s_szTag_PointerPosition        = _T("POSITION");
static LPCTSTR s_szTag_Pos                    = _T("POS");
static LPCTSTR s_szTag_Offset                 = _T("OFFSET");
static LPCTSTR s_szTag_Offset_Center          = _T("OFFSET_CENTER");

static LPCTSTR s_szTag_Container              = _T("CONTAINER");
static LPCTSTR s_szTag_Element                = _T("ELEMENT");
static LPCTSTR s_szTag_Elements               = _T("ELEMENTS");

static LPCTSTR s_szTag_Colors                 = _T("COLORS");
static LPCTSTR s_szTag_Frame_Outline          = _T("FRAME_OUTLINE");
static LPCTSTR s_szTag_Frame_Brush            = _T("FRAME_BRUSH");
static LPCTSTR s_szTag_Frame_BrushInv         = _T("FRAME_BRUSH_INV");
static LPCTSTR s_szTag_Scale_Outline          = _T("SCALE_OUTLINE");
static LPCTSTR s_szTag_Scale_Brush            = _T("SCALE_BRUSH");
static LPCTSTR s_szTag_Fill_Outline           = _T("FILL_OUTLINE");
static LPCTSTR s_szTag_Fill_Brush             = _T("FILL_BRUSH");
static LPCTSTR s_szTag_Pointer_Outline        = _T("POINTER_OUTLINE");
static LPCTSTR s_szTag_Pointer_Brush          = _T("POINTER_BRUSH");
static LPCTSTR s_szTag_Cap_Outline            = _T("CAP_OUTLINE");
static LPCTSTR s_szTag_Cap_Brush              = _T("CAP_BRUSH");
static LPCTSTR s_szTag_Text_Brush             = _T("TEXT_BRUSH");
static LPCTSTR s_szTag_Text_Highlighted_Brush = _T("TEXT_HIGHLIGHTED_BRUSH");
static LPCTSTR s_szTag_TickMark_Outline       = _T("TICKMARK_OUTLINE");
static LPCTSTR s_szTag_TickMark_Brush         = _T("TICKMARK_BRUSH");
static LPCTSTR s_szTag_TickMarkMinor_Brush    = _T("TICKMARK_MINOR_BRUSH");
static LPCTSTR s_szTag_TickMarkMajor_Brush    = _T("TICKMARK_MAJOR_BRUSH");
static LPCTSTR s_szTag_TickMarkMinorOutline_Brush = _T("TICKMARK_MINOR_OUTLINE_BRUSH");
static LPCTSTR s_szTag_TickMarkMajorOutline_Brush = _T("TICKMARK_MAJOR_OUTLINE_BRUSH");
static LPCTSTR s_szTag_Separator_Brush        = _T("SEPARATOR_BRUSH");
static LPCTSTR s_szTag_Digit_Brush            = _T("DIGIT_BRUSH");
static LPCTSTR s_szTag_Decimal_Brush          = _T("DECIMAL_BRUSH");
static LPCTSTR s_szTag_Sign_Brush             = _T("SIGN_BRUSH");
static LPCTSTR s_szTag_Dot_Brush              = _T("DOT_BRUSH");
static LPCTSTR s_szTag_Shadow_Brush           = _T("SHADOW_BRUSH");
static LPCTSTR s_szTag_Caption_Brush          = _T("CAPTION_BRUSH");
static LPCTSTR s_szTag_Back_Outline           = _T("BACK_OUTLINE");
static LPCTSTR s_szTag_Back_Brush             = _T("BACK_BRUSH");
static LPCTSTR s_szTag_Thumb_Brush            = _T("THUMB_BRUSH");
static LPCTSTR s_szTag_Thumb_Outline          = _T("THUMB_OUTLINE");
static LPCTSTR s_szTag_FillOff_Brush          = _T("FILL_OFF_BRUSH");
static LPCTSTR s_szTag_FillOn_Brush           = _T("FILL_ON_BRUSH");
static LPCTSTR s_szTag_LabelOff_Brush         = _T("LABEL_OFF_BRUSH");
static LPCTSTR s_szTag_LabelOn_Brush          = _T("LABEL_ON_BRUSH");
static LPCTSTR s_szTag_Focus_Brush            = _T("FOCUS_BRUSH");

static LPCTSTR s_szTag_MajorTickMarkStep      = _T("MAJOR_TICKMARK_STEP");
static LPCTSTR s_szTag_MinorTickMarkSize      = _T("MINOR_TICKMARK_SIZE");
static LPCTSTR s_szTag_MajorTickMarkSize      = _T("MAJOR_TICKMARK_SIZE");
static LPCTSTR s_szTag_MinorTickMarkStyle     = _T("MINOR_TICKMARK_STYLE");
static LPCTSTR s_szTag_MajorTickMarkStyle     = _T("MAJOR_TICKMARK_STYLE");
static LPCTSTR s_szTag_MinorTickMarkPosition  = _T("MINOR_TICKMARK_POSITION");
static LPCTSTR s_szTag_CapSize                = _T("CAP_SIZE");
static LPCTSTR s_szTag_Shape_Area             = _T("SHAPE_AREA");
static LPCTSTR s_szTag_Offset_Frame           = _T("OFFSET_FRAME");
static LPCTSTR s_szTag_Closed                 = _T("CLOSED");
static LPCTSTR s_szTag_RotateLabels           = _T("ROTATE_LABELS");
static LPCTSTR s_szTag_DrawLastTickMark       = _T("DRAW_LAST_TICKMARK");
static LPCTSTR s_szTag_AnimationThroughStart  = _T("ANIMATION_THROUGH_START");
static LPCTSTR s_szTag_Vertical               = _T("VERTICAL");
static LPCTSTR s_szTag_DateIndex              = _T("DATE_INDEX");

static LPCTSTR s_szTag_Style                  = _T("STYLE");
static LPCTSTR s_szTag_Cells                  = _T("CELLS");
static LPCTSTR s_szTag_Decimals               = _T("DECIMALS");
static LPCTSTR s_szTag_SeparatorWidth         = _T("SEPARATOR_WIDTH");
static LPCTSTR s_szTag_DrawSign               = _T("SIGN");
static LPCTSTR s_szTag_DrawDecimalPoint       = _T("DECIMAL_POINT");
static LPCTSTR s_szTag_DrawLeadingZeros       = _T("LEADING_ZEROS");
static LPCTSTR s_szTag_DrawDynamicObjectsOnTop= _T("DYNAMIC_OBJECTS_ON_TOP");
static LPCTSTR s_szTag_DrawFlags              = _T("DRAW_FLAGS");
static LPCTSTR s_szTag_DrawLabel              = _T("DRAW_LABEL");

static LPCTSTR s_szTag_TextFormat             = _T("TEXTFORMAT");
static LPCTSTR s_szTag_Stretched              = _T("STRETCHED");
static LPCTSTR s_szTag_Opacity                = _T("OPACITY");

static LPCTSTR s_szTag_Label                  = _T("LABEL");
static LPCTSTR s_szTag_LabelOff               = _T("LABEL_OFF");
static LPCTSTR s_szTag_LabelOn                = _T("LABEL_ON");
static LPCTSTR s_szTag_SortOrder              = _T("SORT_ORDER");
static LPCTSTR s_szTag_SortDescending         = _T("SORT_DESCENDING");
static LPCTSTR s_szTag_MaxWeight              = _T("MAX_WEIGHT");
static LPCTSTR s_szTag_MarginHorz             = _T("MARGIN_HORZ");
static LPCTSTR s_szTag_MarginVert             = _T("MARGIN_VERT");
static LPCTSTR s_szTag_FontSizeStep           = _T("FONT_SIZE_STEP");
static LPCTSTR s_szTag_LayoutType             = _T("LAYOUT_TYPE");
static LPCTSTR s_szTag_Nodes                  = _T("NODES");
static LPCTSTR s_szTag_Node                   = _T("NODE");
static LPCTSTR s_szTag_Root                   = _T("ROOT");

static LPCTSTR s_szTag_Shape_Type             = _T("SHAPE_TYPE");
static LPCTSTR s_szTag_Arrow_Type             = _T("ARROW_TYPE");
static LPCTSTR s_szTag_Curve_Type             = _T("CURVE_TYPE");
static LPCTSTR s_szTag_Connector              = _T("CONNECTOR");
static LPCTSTR s_szTag_Anchor_Id              = _T("OBJECT_ID");
static LPCTSTR s_szTag_Item_Id                = _T("ITEM_ID");
static LPCTSTR s_szTag_Anchor_ConnectionID    = _T("CONNECTION_ID");
static LPCTSTR s_szTag_Point                  = _T("POINT");
static LPCTSTR s_szTag_Anchors                = _T("ANCHORS");
static LPCTSTR s_szTag_Anchor                 = _T("ANCHOR");
static LPCTSTR s_szTag_DataObjects            = _T("DATA_OBJECTS");
static LPCTSTR s_szTag_DataObject             = _T("DATA_OBJECT");
static LPCTSTR s_szTag_DataCaption            = _T("DATA_CAPTION");
static LPCTSTR s_szTag_Caption                = _T("CAPTION");
static LPCTSTR s_szTag_CaptionHeight          = _T("CAPTION_HEIGHT");

static LPCTSTR s_szTag_Custom_Name            = _T("CUSTOM_NAME");
static LPCTSTR s_szTag_Custom_Props           = _T("CUSTOM_PROPS");
static LPCTSTR s_szTag_Offset_Shelf           = _T("OFFSET_SHELF");
static LPCTSTR s_szTag_Orientation            = _T("ORIENTATION");
static LPCTSTR s_szTag_ResizeHandle           = _T("RESIZE_HANDLE");

static LPCTSTR s_szTag_Tiles                  = _T("TILES");
static LPCTSTR s_szTag_Tile                   = _T("TILE");
static LPCTSTR s_szTag_Text_Name              = _T("TEXT_NAME");
static LPCTSTR s_szTag_Text_Header            = _T("TEXT_HEADER");
static LPCTSTR s_szTag_Text_Description       = _T("TEXT_DESCRIPTION");
static LPCTSTR s_szTag_Text                   = _T("TEXT");
static LPCTSTR s_szTag_Badge_Number           = _T("BADGE_NUMBER");
static LPCTSTR s_szTag_Badge_Glyph            = _T("BADGE_GLYPH");
static LPCTSTR s_szTag_Badge_Custom_Number    = _T("BADGE_CUSTOM_NUMBER");
static LPCTSTR s_szTag_Group                  = _T("GROUP");
static LPCTSTR s_szTag_Importance             = _T("IMPORTANCE");
static LPCTSTR s_szTag_HorizontalLayout       = _T("HORIZONTAL_LAYOUT");
static LPCTSTR s_szTag_RoundedShapes          = _T("ROUNDED_SHAPES");
static LPCTSTR s_szTag_SquareWidth            = _T("SQUARE_WIDTH");
static LPCTSTR s_szTag_SquareHeight           = _T("SQUARE_HEIGHT");
static LPCTSTR s_szTag_BorderWidth            = _T("BORDER_WIDTH");
static LPCTSTR s_szTag_Thickness              = _T("THICKNESS");
static LPCTSTR s_szTag_StrokeStyle            = _T("STROKESTYLE");
static LPCTSTR s_szTag_Radius                 = _T("RADIUS");
static LPCTSTR s_szTag_GroupCaptions          = _T("GROUP_CAPTIONS");
static LPCTSTR s_szTag_GroupCaption           = _T("GROUP_CAPTION");
static LPCTSTR s_szTag_CaptionButtons         = _T("CAPTION_BUTTONS");
static LPCTSTR s_szTag_CaptionButton          = _T("CAPTION_BUTTON");
static LPCTSTR s_szTag_RTC_Name               = _T("RTC_NAME");
static LPCTSTR s_szTag_View                   = _T("VIEW");
static LPCTSTR s_szTag_Clickable              = _T("CLICKABLE");
static LPCTSTR s_szTag_ImageStretch           = _T("IMAGE_STRETCH");
static LPCTSTR s_szTag_Tile_Type              = _T("TYLE_TYPE");
static LPCTSTR s_szTag_Badge_Custom_Image     = _T("BADGE_CUSTOM_IMAGE");
static LPCTSTR s_szTag_Tiles_DragAndDrop      = _T("TILES_DND");
static LPCTSTR s_szTag_ID_View                = _T("ID_VIEW");
static LPCTSTR s_szTag_ID_Command             = _T("ID_CMD");
static LPCTSTR s_szTag_Text_View              = _T("TEXT_VIEW");

LPCTSTR CBCGPVisualInfo::s_szDataPointerCircular   = _T("PointerCircular");
LPCTSTR CBCGPVisualInfo::s_szDataPointerKnob       = _T("PointerKnob");
LPCTSTR CBCGPVisualInfo::s_szDataPointerLinear     = _T("PointerLinear");
LPCTSTR CBCGPVisualInfo::s_szDataDiagramText       = _T("DataDiagramText");
LPCTSTR CBCGPVisualInfo::s_szTreeMapNode           = _T("TreeMapNode");
LPCTSTR CBCGPVisualInfo::s_szTreeMapGroup          = _T("TreeMapGroup");

LPCTSTR CBCGPVisualInfo::s_szCircularGauge         = _T("CircularGauge");
LPCTSTR CBCGPVisualInfo::s_szKnob                  = _T("Knob");
LPCTSTR CBCGPVisualInfo::s_szAnalogClock           = _T("AnalogClock");
LPCTSTR CBCGPVisualInfo::s_szLinearGauge           = _T("LinearGauge");
LPCTSTR CBCGPVisualInfo::s_szNumericInd            = _T("NumericIndicator");
LPCTSTR CBCGPVisualInfo::s_szColorInd              = _T("ColorIndicator");
LPCTSTR CBCGPVisualInfo::s_szTextInd               = _T("TextIndicator");
LPCTSTR CBCGPVisualInfo::s_szImage                 = _T("Image");
LPCTSTR CBCGPVisualInfo::s_szSwitch                = _T("Switch");
LPCTSTR CBCGPVisualInfo::s_szFrame                 = _T("Frame");
LPCTSTR CBCGPVisualInfo::s_szTagCloud              = _T("TagCloud");
LPCTSTR CBCGPVisualInfo::s_szTreeMap               = _T("TreeMap");
LPCTSTR CBCGPVisualInfo::s_szWinUITiles			   = _T("WinUITiles");
LPCTSTR CBCGPVisualInfo::s_szDiagramConnector      = _T("DiagramConnector");
LPCTSTR CBCGPVisualInfo::s_szDiagramConnectorShelf = _T("DiagramConnectorShelf");
LPCTSTR CBCGPVisualInfo::s_szDiagramConnectorElbow = _T("DiagramConnectorElbow");
LPCTSTR CBCGPVisualInfo::s_szDiagramShape          = _T("DiagramShape");
LPCTSTR CBCGPVisualInfo::s_szDiagramTable          = _T("DiagramTable");
LPCTSTR CBCGPVisualInfo::s_szDiagramImage          = _T("DiagramImage");
LPCTSTR CBCGPVisualInfo::s_szDiagramCustom         = _T("DiagramCustom");
LPCTSTR CBCGPVisualInfo::s_szContainer             = _T("Container");

static LPCTSTR s_szConnector_Old = _T("Connector");
static LPCTSTR s_szShape_Old     = _T("Shape");
static LPCTSTR s_szTable_Old     = _T("Table");


CBCGPVisualInfo::XImage::XImage ()
	: m_sizeIcon            (0, 0)
	, m_bIsAlphaIcon        (FALSE)
	, m_bIsIgnoreAlphaBitmap(FALSE)
	, m_bMap3dColorsInGDI   (FALSE)
	, m_dblLightRatio       (1.0)
{
}

CBCGPVisualInfo::XImage::~XImage ()
{
}

BOOL CBCGPVisualInfo::XImage::FromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	CString strID;
	if (tm.ExcludeTag (s_szTag_ResourceID, strID))
	{
		m_ID.FromTag (strID);
	}

	tm.ReadString (s_szTag_ResourceType, m_strType);
	tm.ReadEntityString (s_szTag_Path, m_strPath);
	tm.ReadSize (s_szTag_IconSize, m_sizeIcon);
	tm.ReadBool (s_szTag_AlphaIcon, m_bIsAlphaIcon);
	tm.ReadBool (s_szTag_IgnoreAlphaBitmap, m_bIsIgnoreAlphaBitmap);
	tm.ReadBool (s_szTag_MapTo3d, m_bMap3dColorsInGDI);
	tm.ReadDouble (s_szTag_LightRatio, m_dblLightRatio);
	tm.ReadColor (s_szTag_ColorTheme, m_clrTheme);
	tm.ReadSize (s_szTag_SizeDest, m_sizeDest);

	return TRUE;
}

void CBCGPVisualInfo::XImage::ToTag (CString& strTag) const
{
	CString strID;
	m_ID.ToTag (strID);
	CBCGPTagManager::WriteItem (strTag, s_szTag_ResourceID, strID);

	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteString (s_szTag_ResourceType, m_strType));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteEntityString (s_szTag_Path, m_strPath));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteSize (s_szTag_IconSize, m_sizeIcon, CSize(0, 0)));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBool (s_szTag_AlphaIcon, m_bIsAlphaIcon, FALSE));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBool (s_szTag_IgnoreAlphaBitmap, m_bIsIgnoreAlphaBitmap, FALSE));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBool (s_szTag_MapTo3d, m_bMap3dColorsInGDI, FALSE));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteDouble (s_szTag_LightRatio, m_dblLightRatio, 1.0));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteColor (s_szTag_ColorTheme, m_clrTheme, CBCGPColor()));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteSize (s_szTag_SizeDest, m_sizeDest, CBCGPSize()));
}


CBCGPVisualInfo::XData::XData (const CString& strName)
	: m_strName (strName)
	, m_dblValue(0.0)
{
}

CBCGPVisualInfo::XData::~XData ()
{
}

CBCGPVisualInfo::XData* CBCGPVisualInfo::XData::CreateFromTag (const CString& tag)
{
	CBCGPVisualInfo::XData* pData = NULL;

	CString strDataName;
	{
		CBCGPTagManager tm (tag);
		tm.ReadString (s_szTag_DataName, strDataName);
	}

	if (!strDataName.IsEmpty ())
	{
		pData = CBCGPVisualInfo::XData::CreateFromName (strDataName);
		if (pData != NULL)
		{
			pData->FromTag (tag);
		}
	}

	return pData;
}

CBCGPVisualInfo::XData* CBCGPVisualInfo::XData::CreateFromName (const CString& name)
{
	CBCGPVisualInfo::XData* pData = NULL;

	if (name.Compare (CBCGPVisualInfo::s_szDataPointerCircular) == 0)
	{
		pData = new CBCGPVisualInfo::XCircularPointer;
	}
	else if (name.Compare (CBCGPVisualInfo::s_szDataPointerKnob) == 0)
	{
		pData = new CBCGPVisualInfo::XKnobPointer;
	}
	else if (name.Compare (CBCGPVisualInfo::s_szDataPointerLinear) == 0)
	{
		pData = new CBCGPVisualInfo::XLinearPointer;
	}
	else if (name.Compare (CBCGPVisualInfo::s_szDataDiagramText) == 0)
	{
		pData = new CBCGPVisualInfo::XDiagramTextData;
	}

	return pData;
}

BOOL CBCGPVisualInfo::XData::FromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

 	tm.ReadDouble (s_szTag_DataValue, m_dblValue);

	return TRUE;
}

void CBCGPVisualInfo::XData::ToTag (CString& strTag) const
{
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteString (s_szTag_DataName, m_strName));
 	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteDouble (s_szTag_DataValue, m_dblValue, 0.0));
}


CBCGPVisualInfo::XElement::XElement(const CString& strElementName)
	: CBCGPBaseInfo::XBase (strElementName)
	, m_Rect               (0, 0, 0, 0)
	, m_bIsVisible         (TRUE)
	, m_bIsAutoDestroy     (TRUE)
{
}

CBCGPVisualInfo::XElement::~XElement()
{
}

BOOL CBCGPVisualInfo::XElement::FromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	CString strID;
	if (tm.ExcludeTag (s_szTag_ID, strID))
	{
		m_ID.FromTag (strID);
	}

	tm.ReadRect (s_szTag_Rect, m_Rect);
	tm.ReadBool (s_szTag_Visible, m_bIsVisible);
	tm.ReadBool (s_szTag_AutoDestroy, m_bIsAutoDestroy);

	return TRUE;
}

void CBCGPVisualInfo::XElement::ToTag (CString& strTag) const
{
	XBase::ToTag (strTag);

	CString strID;
	m_ID.ToTag (strID);
	CBCGPTagManager::WriteItem (strTag, s_szTag_ID, strID);

	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteRect (s_szTag_Rect, m_Rect, CRect (0, 0, 0, 0)));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBool (s_szTag_Visible, m_bIsVisible, TRUE));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBool (s_szTag_AutoDestroy, m_bIsAutoDestroy, TRUE));
}

CBCGPVisualInfo::XContainer::XContainer()
	: CBCGPBaseInfo::XBase (CBCGPVisualInfo::s_szContainer)
	, m_Rect               (0, 0, 0, 0)
	, m_bDrawDynamicObjectsOnTop(TRUE)
{
}

CBCGPVisualInfo::XContainer::~XContainer()
{
	for (int i = 0; i < (int)m_arElements.GetSize (); i++)
	{
		if (m_arElements[i] != NULL)
		{
			delete m_arElements[i];
		}
	}
	m_arElements.RemoveAll ();
}

BOOL CBCGPVisualInfo::XContainer::FromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	CString strElements;
	if (tm.ExcludeTag (s_szTag_Elements, strElements))
	{
		CBCGPTagManager tmElements (strElements);

		CString strElement;
		while (tmElements.ExcludeTag (s_szTag_Element, strElement))
		{
			XElement* pElement = (XElement*)CBCGPVisualInfo::CreateBaseFromTag (strElement);
			if (pElement != NULL)
			{
				m_arElements.Add (pElement);
			}
		}
	}

	tm.ReadBool (s_szTag_DrawDynamicObjectsOnTop, m_bDrawDynamicObjectsOnTop);
	tm.ReadRect (s_szTag_Rect, m_Rect);
	tm.ReadBrush (s_szTag_Fill_Brush, m_brFill);
	tm.ReadBrush (s_szTag_Fill_Outline, m_brOutline);

	return TRUE;
}

void CBCGPVisualInfo::XContainer::ToTag (CString& strTag) const
{
	XBase::ToTag (strTag);

	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBool (s_szTag_DrawDynamicObjectsOnTop, m_bDrawDynamicObjectsOnTop, TRUE));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteRect (s_szTag_Rect, m_Rect, CRect (0, 0, 0, 0)));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Fill_Brush, m_brFill));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Fill_Outline, m_brOutline));

	CString strElements;
	for (int i = 0; i < (int)m_arElements.GetSize (); i++)
	{
		CString strElement;
		m_arElements[i]->ToTag (strElement);
		CBCGPTagManager::WriteItem (strElements, s_szTag_Element, strElement);
	}
	CBCGPTagManager::WriteItem (strTag, s_szTag_Elements, strElements);
}

CBCGPBaseInfo::XBase* CBCGPVisualInfo::CreateBaseFromName (const CString& name)
{
	CBCGPVisualInfo::XElement* element = NULL;

	if (name.Compare (CBCGPVisualInfo::s_szTagCloud) == 0)
	{
		element = new CBCGPVisualInfo::XElementTagCloud;
	}
	else if (name.Compare (CBCGPVisualInfo::s_szTreeMap) == 0)
	{
		element = new CBCGPVisualInfo::XElementTreeMap;
	}
	else if (name.Compare (CBCGPVisualInfo::s_szWinUITiles) == 0)
	{
		element = new CBCGPVisualInfo::XElementWinUITiles;
	}
	else
	{
		element = CBCGPVisualInfo::XGaugeElement::CreateFromName(name);

		if (element == NULL)
		{
			element = CBCGPVisualInfo::XDiagramElement::CreateFromName(name);
		}
	}

	return element;
}

CBCGPBaseInfo::XBase* CBCGPVisualInfo::CreateBaseFromTag (const CString& tag)
{
	return CBCGPBaseInfo::CreateBaseFromTag (tag, &CBCGPVisualInfo::CreateBaseFromName);
}

CBCGPVisualInfo::XGaugeElement* CBCGPVisualInfo::XGaugeElement::CreateFromName (const CString& name)
{
	CBCGPVisualInfo::XGaugeElement* element = NULL;

	if (name.Compare (CBCGPVisualInfo::s_szCircularGauge) == 0)
	{
		element = new CBCGPVisualInfo::XElementCircular;
	}
	else if (name.Compare (CBCGPVisualInfo::s_szKnob) == 0)
	{
		element = new CBCGPVisualInfo::XElementKnob;
	}
	else if (name.Compare (CBCGPVisualInfo::s_szAnalogClock) == 0)
	{
		element = new CBCGPVisualInfo::XElementAnalogClock;
	}
	else if (name.Compare (CBCGPVisualInfo::s_szLinearGauge) == 0)
	{
		element = new CBCGPVisualInfo::XElementLinear;
	}
	else if (name.Compare (CBCGPVisualInfo::s_szNumericInd) == 0)
	{
		element = new CBCGPVisualInfo::XElementNumeric;
	}
	else if (name.Compare (CBCGPVisualInfo::s_szColorInd) == 0)
	{
		element = new CBCGPVisualInfo::XElementColor;
	}
	else if (name.Compare (CBCGPVisualInfo::s_szTextInd) == 0)
	{
		element = new CBCGPVisualInfo::XElementText;
	}
	else if (name.Compare (CBCGPVisualInfo::s_szImage) == 0)
	{
		element = new CBCGPVisualInfo::XElementImage;
	}
	else if (name.Compare (CBCGPVisualInfo::s_szSwitch) == 0)
	{
		element = new CBCGPVisualInfo::XElementSwitch;
	}
	else if (name.Compare (CBCGPVisualInfo::s_szFrame) == 0)
	{
		element = new CBCGPVisualInfo::XElementFrame;
	}

	return element;
}

CBCGPVisualInfo::XDiagramElement* CBCGPVisualInfo::XDiagramElement::CreateFromName (const CString& name)
{
	CBCGPVisualInfo::XDiagramElement* element = NULL;

	if (name.Compare (CBCGPVisualInfo::s_szDiagramConnector) == 0 ||
		name.Compare (s_szConnector_Old) == 0)
	{
		element = new CBCGPVisualInfo::XElementDiagramConnector;
	}
	else if (name.Compare (CBCGPVisualInfo::s_szDiagramConnectorShelf) == 0)
	{
		element = new CBCGPVisualInfo::XElementDiagramConnectorShelf;
	}
	else if (name.Compare (CBCGPVisualInfo::s_szDiagramConnectorElbow) == 0)
	{
		element = new CBCGPVisualInfo::XElementDiagramConnectorElbow;
	}
	else if (name.Compare (CBCGPVisualInfo::s_szDiagramShape) == 0 ||
			name.Compare (s_szShape_Old) == 0)
	{
		element = new CBCGPVisualInfo::XElementDiagramShape;
	}
	else if (name.Compare (CBCGPVisualInfo::s_szDiagramTable) == 0 ||
			name.Compare (s_szTable_Old) == 0)
	{
		element = new CBCGPVisualInfo::XElementDiagramTable;
	}
	else if (name.Compare (CBCGPVisualInfo::s_szDiagramImage) == 0)
	{
		element = new CBCGPVisualInfo::XElementDiagramImage;
	}
	else if (name.Compare (CBCGPVisualInfo::s_szDiagramCustom) == 0)
	{
		element = new CBCGPVisualInfo::XElementDiagramCustom;
	}

	return element;
}

CBCGPVisualInfo::XGaugeScale::XGaugeScale ()
	: m_dblStart             (0.0)
	, m_dblFinish            (100.0)
	, m_dblStep              (5.0)
	, m_dblOffsetFromFrame   (5.0)
	, m_dblMajorTickMarkStep (2.0)
	, m_dblMinorTickMarkSize (5.0)
	, m_dblMajorTickMarkSize (10.0)
	, m_MinorTickMarkStyle   (CBCGPCircularGaugeScale::BCGP_TICKMARK_LINE)
	, m_MajorTickMarkStyle   (CBCGPCircularGaugeScale::BCGP_TICKMARK_LINE)
	, m_MinorTickMarkPosition(CBCGPCircularGaugeScale::BCGP_TICKMARK_POSITION_NEAR)
{
}

CBCGPVisualInfo::XGaugeScale::~XGaugeScale ()
{
}

BOOL CBCGPVisualInfo::XGaugeScale::FromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	tm.ReadDouble (s_szTag_Start, m_dblStart);
	tm.ReadDouble (s_szTag_Finish, m_dblFinish);
	tm.ReadDouble (s_szTag_Step, m_dblStep);
	tm.ReadDouble (s_szTag_Offset_Frame, m_dblOffsetFromFrame);
	tm.ReadEntityString (s_szTag_LabelFormat, m_strLabelFormat);
	tm.ReadDouble (s_szTag_MajorTickMarkStep, m_dblMajorTickMarkStep);
	tm.ReadDouble (s_szTag_MinorTickMarkSize, m_dblMinorTickMarkSize);
	tm.ReadDouble (s_szTag_MajorTickMarkSize, m_dblMajorTickMarkSize);

	int style = (int)CBCGPGaugeScaleObject::BCGP_TICKMARK_LINE;
	tm.ReadInt (s_szTag_MinorTickMarkStyle, style);
	m_MinorTickMarkStyle = (CBCGPGaugeScaleObject::BCGP_TICKMARK_STYLE)bcg_clamp(style, (int)CBCGPGaugeScaleObject::BCGP_TICKMARK_STYLE_FIRST, (int)CBCGPGaugeScaleObject::BCGP_TICKMARK_STYLE_LAST);

	style = (int)CBCGPGaugeScaleObject::BCGP_TICKMARK_LINE;
	tm.ReadInt (s_szTag_MajorTickMarkStyle, style);
	m_MajorTickMarkStyle = (CBCGPGaugeScaleObject::BCGP_TICKMARK_STYLE)bcg_clamp(style, (int)CBCGPGaugeScaleObject::BCGP_TICKMARK_STYLE_FIRST, (int)CBCGPGaugeScaleObject::BCGP_TICKMARK_STYLE_LAST);

	style = (int)CBCGPGaugeScaleObject::BCGP_TICKMARK_POSITION_NEAR;
	tm.ReadInt (s_szTag_MinorTickMarkPosition, style);
	m_MinorTickMarkPosition = (CBCGPGaugeScaleObject::BCGP_TICKMARK_POSITION)bcg_clamp(style, (int)CBCGPGaugeScaleObject::BCGP_TICKMARK_POSITION_FIRST, (int)CBCGPGaugeScaleObject::BCGP_TICKMARK_POSITION_LAST);

	tm.ReadBrush (s_szTag_Fill_Brush, m_brFill);
	tm.ReadBrush (s_szTag_Fill_Outline, m_brOutline);
	tm.ReadBrush (s_szTag_Text_Brush, m_brText);
	tm.ReadBrush (s_szTag_TickMarkMinor_Brush, m_brTickMarkMinor);
	tm.ReadBrush (s_szTag_TickMarkMinorOutline_Brush, m_brTickMarkMinorOutline);
	tm.ReadBrush (s_szTag_TickMarkMajor_Brush, m_brTickMarkMajor);
	tm.ReadBrush (s_szTag_TickMarkMajorOutline_Brush, m_brTickMarkMajorOutline);

	return TRUE;
}

void CBCGPVisualInfo::XGaugeScale::ToTag (CString& strTag) const
{
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteDouble (s_szTag_Start, m_dblStart, 0.0));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteDouble (s_szTag_Finish, m_dblFinish, 100.0));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteDouble (s_szTag_Step, m_dblStep, 5.0));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteDouble (s_szTag_Offset_Frame, m_dblOffsetFromFrame, 5.0));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteEntityString (s_szTag_LabelFormat, m_strLabelFormat));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteDouble (s_szTag_MajorTickMarkStep, m_dblMajorTickMarkStep, 2.0));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteDouble (s_szTag_MinorTickMarkSize, m_dblMinorTickMarkSize, 5.0));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteDouble (s_szTag_MajorTickMarkSize, m_dblMajorTickMarkSize, 10.0));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteInt (s_szTag_MinorTickMarkStyle, (int)m_MinorTickMarkStyle, (int)CBCGPGaugeScaleObject::BCGP_TICKMARK_LINE));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteInt (s_szTag_MajorTickMarkStyle, (int)m_MajorTickMarkStyle, (int)CBCGPGaugeScaleObject::BCGP_TICKMARK_LINE));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteInt (s_szTag_MinorTickMarkPosition, (int)m_MinorTickMarkPosition, (int)CBCGPGaugeScaleObject::BCGP_TICKMARK_POSITION_NEAR));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Fill_Brush, m_brFill));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Fill_Outline, m_brOutline));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Text_Brush, m_brText));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_TickMarkMinor_Brush, m_brTickMarkMinor));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_TickMarkMinorOutline_Brush, m_brTickMarkMinorOutline));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_TickMarkMajor_Brush, m_brTickMarkMajor));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_TickMarkMajorOutline_Brush, m_brTickMarkMajorOutline));	
}


CBCGPVisualInfo::XGaugeData::XGaugeData (const CString& strName)
	: CBCGPVisualInfo::XData(strName)
	, m_nScale              (0)
{
}

CBCGPVisualInfo::XGaugeData::~XGaugeData ()
{
}

BOOL CBCGPVisualInfo::XGaugeData::FromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	tm.ReadInt (s_szTag_Scale_Id, m_nScale);
 	tm.ReadBrush (s_szTag_Fill_Brush, m_brFill);
 	tm.ReadBrush (s_szTag_Fill_Outline, m_brOutline);

	return XData::FromTag(tm.GetBuffer ());
}

void CBCGPVisualInfo::XGaugeData::ToTag (CString& strTag) const
{
	XData::ToTag (strTag);

	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteInt (s_szTag_Scale_Id, m_nScale, m_nScale + 1));
 	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Fill_Brush, m_brFill));
 	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Fill_Outline, m_brOutline));
}


CBCGPVisualInfo::XGaugeColoredRange::XGaugeColoredRange()
	: m_nScale            (0)
	, m_dblStartValue     (0.0)
	, m_dblFinishValue    (0.0)
	, m_dblStartWidth     (0.0)
	, m_dblFinishWidth    (0.0)
	, m_dblOffsetFromFrame(0.0)
{
}

CBCGPVisualInfo::XGaugeColoredRange::~XGaugeColoredRange()
{
}

BOOL CBCGPVisualInfo::XGaugeColoredRange::FromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	tm.ReadInt (s_szTag_Scale_Id, m_nScale);
 	tm.ReadDouble (s_szTag_Start_Value, m_dblStartValue);
	tm.ReadDouble (s_szTag_Finish_Value, m_dblFinishValue);
 	tm.ReadDouble (s_szTag_Start_Width, m_dblStartWidth);
	tm.ReadDouble (s_szTag_Finish_Width, m_dblFinishWidth);
	tm.ReadDouble (s_szTag_Offset_Frame, m_dblOffsetFromFrame);
 	tm.ReadBrush (s_szTag_Fill_Brush, m_brFill);
 	tm.ReadBrush (s_szTag_Fill_Outline, m_brOutline);
	tm.ReadBrush (s_szTag_TickMark_Brush, m_brTickMarkFill);
	tm.ReadBrush (s_szTag_TickMark_Outline, m_brTickMarkOutline);
	tm.ReadBrush (s_szTag_Text_Brush, m_brTextLabel);

	return TRUE;
}

void CBCGPVisualInfo::XGaugeColoredRange::ToTag (CString& strTag) const
{
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteInt (s_szTag_Scale_Id, m_nScale, m_nScale + 1));
 	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteDouble (s_szTag_Start_Value, m_dblStartValue, 0.0));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteDouble (s_szTag_Finish_Value, m_dblFinishValue, 0.0));
 	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteDouble (s_szTag_Start_Width, m_dblStartWidth, 0.0));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteDouble (s_szTag_Finish_Width, m_dblFinishWidth, 0.0));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteDouble (s_szTag_Offset_Frame, m_dblOffsetFromFrame, 0.0));
 	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Fill_Brush, m_brFill));
 	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Fill_Outline, m_brOutline));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_TickMark_Brush, m_brTickMarkFill));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_TickMark_Outline, m_brTickMarkOutline));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Text_Brush, m_brTextLabel));
}


CBCGPVisualInfo::XDiagramID::XDiagramID()
	: m_nID       (CBCGPDiagramItemID::Undefined)
	, m_bConnector(FALSE)
{
}

CBCGPVisualInfo::XDiagramID::~XDiagramID()
{
}

BOOL CBCGPVisualInfo::XDiagramID::FromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	tm.ReadInt (s_szTag_Value, m_nID);
	tm.ReadBool (s_szTag_Connector, m_bConnector);

	return TRUE;
}

void CBCGPVisualInfo::XDiagramID::ToTag (CString& strTag) const
{
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteInt (s_szTag_Value, m_nID, CBCGPDiagramItemID::Undefined));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBool (s_szTag_Connector, m_bConnector, FALSE));
}


CBCGPVisualInfo::XDiagramAnchorPoint::XDiagramAnchorPoint ()
	: m_nConnectionPort(CBCGPDiagramVisualObject::CP_None)
	, m_ptNullAnchor    (0, 0)
{
}

CBCGPVisualInfo::XDiagramAnchorPoint::~XDiagramAnchorPoint ()
{
}

BOOL CBCGPVisualInfo::XDiagramAnchorPoint::FromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	CString strID;
	if (tm.ExcludeTag (s_szTag_Anchor_Id, strID))
	{
		m_idObject.FromTag (strID);
	}

 	tm.ReadUInt (s_szTag_Anchor_ConnectionID, m_nConnectionPort);
	tm.ReadPoint (s_szTag_Point, m_ptNullAnchor);

	return TRUE;
}
void CBCGPVisualInfo::XDiagramAnchorPoint::ToTag (CString& strTag) const
{
	CString strID;
	m_idObject.ToTag (strID);
	CBCGPTagManager::WriteItem (strTag, s_szTag_Anchor_Id, strID);

	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteUInt (s_szTag_Anchor_ConnectionID, m_nConnectionPort, CBCGPDiagramVisualObject::CP_None));
 	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WritePoint (s_szTag_Point, m_ptNullAnchor, CPoint (0, 0)));
}


CBCGPVisualInfo::XDiagramArrow::XDiagramArrow()
	: m_nShape (CBCGPDiagramConnector::BCGP_ARROW_NONE)
	, m_dLength(11.0)
	, m_dWidth (3.0)
{
}

CBCGPVisualInfo::XDiagramArrow::~XDiagramArrow()
{
}

BOOL CBCGPVisualInfo::XDiagramArrow::FromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	int shape = CBCGPDiagramConnector::BCGP_ARROW_NONE;
	tm.ReadInt (s_szTag_Arrow_Type, shape);
	m_nShape = (CBCGPDiagramConnector::BCGP_ARROW_SHAPE)bcg_clamp(shape, 
		CBCGPDiagramConnector::BCGP_ARROW_SHAPE_FIRST, CBCGPDiagramConnector::BCGP_ARROW_SHAPE_LAST);

	tm.ReadDouble (s_szTag_Length, m_dLength);
	tm.ReadDouble (s_szTag_Width, m_dWidth);
	tm.ReadBrush (s_szTag_Fill_Brush, m_brFill);
	tm.ReadBrush (s_szTag_Fill_Outline, m_brOutline);

	return TRUE;
}

void CBCGPVisualInfo::XDiagramArrow::ToTag (CString& strTag) const
{
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteInt (s_szTag_Arrow_Type, (int)m_nShape, CBCGPDiagramConnector::BCGP_ARROW_NONE));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteDouble (s_szTag_Length, m_dLength, 11.0));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteDouble (s_szTag_Width, m_dWidth, 3.0));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Fill_Brush, m_brFill, FALSE));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Fill_Outline, m_brOutline, FALSE));
}


CBCGPVisualInfo::XDiagramTextData::XDiagramTextData ()
	: CBCGPVisualInfo::XData(CBCGPVisualInfo::s_szDataDiagramText)
{
}

CBCGPVisualInfo::XDiagramTextData::~XDiagramTextData ()
{
}

BOOL CBCGPVisualInfo::XDiagramTextData::FromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	tm.ReadEntityString (s_szTag_TextValue, m_strText);
	tm.ReadBrush (s_szTag_Text_Brush, m_brText);
	tm.ReadTextFormat (s_szTag_TextFormat, m_fmtText);

	return XData::FromTag(tm.GetBuffer ());
}

void CBCGPVisualInfo::XDiagramTextData::ToTag (CString& strTag) const
{
	XData::ToTag (strTag);

	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteEntityString (s_szTag_TextValue, m_strText));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Text_Brush, m_brText));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteTextFormat (s_szTag_TextFormat, m_fmtText));
}

CBCGPVisualInfo::XGaugeElement::XGaugeElement(const CString& strElementName)
	: CBCGPVisualInfo::XElement(strElementName)
	, m_nFrameSize             (6)
	, m_Pos                    (CBCGPGaugeImpl::BCGP_SUB_GAUGE_NONE)
	, m_ptOffset               (0, 0)
	, m_bIsInteractiveMode     (FALSE)
{
}

CBCGPVisualInfo::XGaugeElement::~XGaugeElement()
{
	int i = 0;
	for (i = 0; i < (int)m_arScales.GetSize (); i++)
	{
		if (m_arScales[i] != NULL)
		{
			delete m_arScales[i];
		}
	}
	m_arScales.RemoveAll ();

	for (i = 0; i < (int)m_arRanges.GetSize (); i++)
	{
		if (m_arRanges[i] != NULL)
		{
			delete m_arRanges[i];
		}
	}
	m_arRanges.RemoveAll ();
}


CBCGPVisualInfo::XStaticGaugeElement::XStaticGaugeElement(const CString& strElementName)
	: CBCGPVisualInfo::XGaugeElement(strElementName)
	, m_dblOpacity                  (1.0)
	, m_DefaultDrawFlags            (BCGP_DRAW_STATIC)
{
}

CBCGPVisualInfo::XStaticGaugeElement::~XStaticGaugeElement()
{
}

CBCGPVisualInfo::XDiagramElement::XDiagramElement(const CString& strElementName)
	: CBCGPVisualInfo::XElement(strElementName)
	, m_Thickness              (1.0)
{
}

CBCGPVisualInfo::XDiagramElement::~XDiagramElement()
{
	for (int i = 0; i < (int)m_arDataObjects.GetSize (); i++)
	{
		if (m_arDataObjects[i] != NULL)
		{
			delete m_arDataObjects[i];
		}
	}
	m_arDataObjects.RemoveAll ();
}

CBCGPVisualInfo::XDiagramElementConnector::XDiagramElementConnector(const CString& strElementName)
	: CBCGPVisualInfo::XDiagramElement (strElementName)
	, m_curveType (CBCGPDiagramConnector::BCGP_CURVE_TYPE_SPLINE)
{
	m_brOutline = CBCGPBrush (CBCGPColor::Gray);
}

CBCGPVisualInfo::XDiagramElementConnector::~XDiagramElementConnector()
{
	for (int i = 0; i < (int)m_arPoints.GetSize (); i++)
	{
		if (m_arPoints[i] != NULL)
		{
			delete m_arPoints[i];
		}
	}
	m_arPoints.RemoveAll ();
}

CBCGPVisualInfo::XCircularScale::XCircularScale()
	: CBCGPVisualInfo::XGaugeScale()
	, m_dblStartAngle             (225.0)
	, m_dblFinishAngle            (-45.0)
	, m_bRotateLabels             (FALSE)
	, m_bIsClosed                 (FALSE)
	, m_bDrawLastTickMark         (TRUE)
	, m_bAnimationThroughStart    (TRUE)
{
}

CBCGPVisualInfo::XCircularScale::~XCircularScale()
{
}

BOOL CBCGPVisualInfo::XCircularScale::FromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	tm.ReadBool (s_szTag_Closed, m_bIsClosed);

	if (m_bIsClosed)
	{
		m_dblStartAngle = 90.0;
		m_dblFinishAngle = -360.0 + m_dblStartAngle;

		tm.ReadBool (s_szTag_DrawLastTickMark, m_bDrawLastTickMark);
		tm.ReadBool (s_szTag_AnimationThroughStart, m_bAnimationThroughStart);
	}

	tm.ReadDouble (s_szTag_Start_Angle, m_dblStartAngle);
	tm.ReadDouble (s_szTag_Finish_Angle, m_dblFinishAngle);
	tm.ReadBool (s_szTag_RotateLabels, m_bRotateLabels);

	return XGaugeScale::FromTag (tm.GetBuffer ());
}

void CBCGPVisualInfo::XCircularScale::ToTag (CString& strTag) const
{
	XGaugeScale::ToTag(strTag);

	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBool (s_szTag_Closed, m_bIsClosed, FALSE));

	if (m_bIsClosed)
	{
		CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteDouble (s_szTag_Start_Angle, m_dblStartAngle, 90.0));
		CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBool (s_szTag_DrawLastTickMark, m_bDrawLastTickMark, TRUE));
		CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBool (s_szTag_AnimationThroughStart, m_bAnimationThroughStart, TRUE));
	}
	else
	{
		CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteDouble (s_szTag_Start_Angle, m_dblStartAngle, 225.0));
		CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteDouble (s_szTag_Finish_Angle, m_dblFinishAngle, -45.0));
	}

	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBool (s_szTag_RotateLabels, m_bRotateLabels, FALSE));
}

CBCGPVisualInfo::XCircularPointer::XCircularPointer()
	: CBCGPVisualInfo::XGaugeData(CBCGPVisualInfo::s_szDataPointerCircular)
	, m_dblSize                  (0.0)
	, m_dblWidth                 (0.0)
	, m_bExtraLen                (FALSE)
	, m_Style                    (CBCGPCircularGaugePointer::BCGP_GAUGE_NEEDLE_TRIANGLE)
{
}

CBCGPVisualInfo::XCircularPointer::~XCircularPointer()
{
}

BOOL CBCGPVisualInfo::XCircularPointer::FromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	tm.ReadDouble (s_szTag_Size, m_dblSize);
	m_dblSize = bcg_clamp(m_dblSize, 0.0, 1.0);
	tm.ReadDouble (s_szTag_Width, m_dblWidth);
	tm.ReadBool (s_szTag_ExtraLen, m_bExtraLen);

	int style = CBCGPCircularGaugePointer::BCGP_GAUGE_NEEDLE_TRIANGLE;
	tm.ReadInt (s_szTag_PointerStyle, style);
	m_Style = (CBCGPCircularGaugePointer::BCGP_GAUGE_POINTER_STYLE)bcg_clamp(style, (int)CBCGPCircularGaugePointer::BCGP_GAUGE_POINTER_STYLE_FIRST, (int)CBCGPCircularGaugePointer::BCGP_GAUGE_POINTER_STYLE_LAST);

	return XGaugeData::FromTag (tm.GetBuffer ());
}

void CBCGPVisualInfo::XCircularPointer::ToTag (CString& strTag) const
{
	XGaugeData::ToTag(strTag);

 	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteDouble (s_szTag_Size, bcg_clamp(m_dblSize, 0.0, 1.0), 0.0));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteDouble (s_szTag_Width, m_dblWidth, 0.0));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBool (s_szTag_ExtraLen, m_bExtraLen, FALSE));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteInt (s_szTag_PointerStyle, (int)m_Style, (int)CBCGPCircularGaugePointer::BCGP_GAUGE_NEEDLE_TRIANGLE));
}

CBCGPVisualInfo::XCircularColoredRange::XCircularColoredRange()
	: CBCGPVisualInfo::XGaugeColoredRange ()
{
}

CBCGPVisualInfo::XCircularColoredRange::~XCircularColoredRange()
{
}

BOOL CBCGPVisualInfo::XCircularColoredRange::FromTag (const CString& strTag)
{
	return XGaugeColoredRange::FromTag (strTag);
}

void CBCGPVisualInfo::XCircularColoredRange::ToTag (CString& strTag) const
{
	XGaugeColoredRange::ToTag(strTag);
}

CBCGPVisualInfo::XElementCircular::XElementCircular(const CString& strElementName)
	: CBCGPVisualInfo::XGaugeElement (strElementName)
	, m_dblCapSize                   (15.0)
	, m_bShapeByTicksArea            (FALSE)
{
}

CBCGPVisualInfo::XElementCircular::XElementCircular()
	: CBCGPVisualInfo::XGaugeElement (CBCGPVisualInfo::s_szCircularGauge)
	, m_dblCapSize                   (15.0)
	, m_bShapeByTicksArea            (FALSE)
{
}

CBCGPVisualInfo::XElementCircular::~XElementCircular()
{
	int i = 0;
	for (i = 0; i < (int)m_arPointers.GetSize (); i++)
	{
		if (m_arPointers[i] != NULL)
		{
			delete m_arPointers[i];
		}
	}
	m_arPointers.RemoveAll ();

	for (i = 0; i < (int)m_arSubGauges.GetSize (); i++)
	{
		if (m_arSubGauges[i] != NULL)
		{
			delete m_arSubGauges[i];
		}
	}
	m_arSubGauges.RemoveAll ();
}

CBCGPVisualInfo::XKnobPointer::XKnobPointer()
	: CBCGPVisualInfo::XGaugeData(CBCGPVisualInfo::s_szDataPointerKnob)
	, m_dblOffsetFromCenter      (0.0)
	, m_dblWidth                 (0.0)
	, m_Style                    (CBCGPKnobPointer::BCGP_KNOB_POINTER_CIRCLE)
{
}

CBCGPVisualInfo::XKnobPointer::~XKnobPointer()
{
}

BOOL CBCGPVisualInfo::XKnobPointer::FromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	tm.ReadDouble (s_szTag_Offset_Center, m_dblOffsetFromCenter);
	tm.ReadDouble (s_szTag_Width, m_dblWidth);

	int style = CBCGPKnobPointer::BCGP_KNOB_POINTER_CIRCLE;
	tm.ReadInt (s_szTag_PointerStyle, style);
	m_Style = (CBCGPKnobPointer::BCGP_KNOB_POINTER_STYLE)bcg_clamp(style, (int)CBCGPKnobPointer::BCGP_KNOB_POINTER_STYLE_FIRST, (int)CBCGPKnobPointer::BCGP_KNOB_POINTER_STYLE_LAST);

	return XGaugeData::FromTag (tm.GetBuffer ());
}

void CBCGPVisualInfo::XKnobPointer::ToTag (CString& strTag) const
{
	XGaugeData::ToTag(strTag);

 	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteDouble (s_szTag_Offset_Center, m_dblOffsetFromCenter, 0.0));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteDouble (s_szTag_Width, m_dblWidth, 0.0));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteInt (s_szTag_PointerStyle, (int)m_Style, (int)CBCGPKnobPointer::BCGP_KNOB_POINTER_CIRCLE));
}

CBCGPVisualInfo::XElementKnob::XElementKnob()
	: CBCGPVisualInfo::XElementCircular (CBCGPVisualInfo::s_szKnob)
{
}

CBCGPVisualInfo::XElementKnob::~XElementKnob()
{
}

CBCGPVisualInfo::XElementAnalogClock::XElementAnalogClock()
	: CBCGPVisualInfo::XElementCircular (CBCGPVisualInfo::s_szAnalogClock)
	, m_nDateIndex(-1)
{
}

CBCGPVisualInfo::XElementAnalogClock::~XElementAnalogClock()
{
}

CBCGPVisualInfo::XLinearScale::XLinearScale()
	: CBCGPVisualInfo::XGaugeScale()
{
}

CBCGPVisualInfo::XLinearScale::~XLinearScale()
{
}

BOOL CBCGPVisualInfo::XLinearScale::FromTag (const CString& strTag)
{
	return XGaugeScale::FromTag (strTag);
}

void CBCGPVisualInfo::XLinearScale::ToTag (CString& strTag) const
{
	XGaugeScale::ToTag(strTag);
}

CBCGPVisualInfo::XLinearPointer::XLinearPointer()
	: CBCGPVisualInfo::XGaugeData(CBCGPVisualInfo::s_szDataPointerLinear)
	, m_dblSize                  (0.0)
	, m_dblWidth                 (0.0)
	, m_Style                    (CBCGPLinearGaugePointer::BCGP_GAUGE_NEEDLE_TRIANGLE)
	, m_Position                 (CBCGPLinearGaugePointer::BCGP_GAUGE_POSITION_NEAR)
{
}

CBCGPVisualInfo::XLinearPointer::~XLinearPointer()
{
}

BOOL CBCGPVisualInfo::XLinearPointer::FromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	tm.ReadDouble (s_szTag_Size, m_dblSize);
	m_dblSize = bcg_clamp(m_dblSize, 0.0, 1.0);
	tm.ReadDouble (s_szTag_Width, m_dblWidth);

	int style = CBCGPLinearGaugePointer::BCGP_GAUGE_NEEDLE_TRIANGLE;
	tm.ReadInt (s_szTag_PointerStyle, style);
	m_Style = (CBCGPLinearGaugePointer::BCGP_GAUGE_POINTER_STYLE)bcg_clamp(style, (int)CBCGPLinearGaugePointer::BCGP_GAUGE_POINTER_STYLE_FIRST, (int)CBCGPLinearGaugePointer::BCGP_GAUGE_POINTER_STYLE_LAST);

	style = CBCGPLinearGaugePointer::BCGP_GAUGE_POSITION_NEAR;
	tm.ReadInt (s_szTag_PointerPosition, style);
	m_Position = (CBCGPLinearGaugePointer::BCGP_GAUGE_POINTER_POSITION)bcg_clamp(style, (int)CBCGPLinearGaugePointer::BCGP_GAUGE_POINTER_POSITION_FIRST, (int)CBCGPLinearGaugePointer::BCGP_GAUGE_POINTER_POSITION_LAST);

	return XGaugeData::FromTag (tm.GetBuffer ());
}

void CBCGPVisualInfo::XLinearPointer::ToTag (CString& strTag) const
{
	XGaugeData::ToTag(strTag);

	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteDouble (s_szTag_Size, bcg_clamp(m_dblSize, 0.0, 1.0), 0.0));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteDouble (s_szTag_Width, m_dblWidth, 0.0));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteInt (s_szTag_PointerStyle, (int)m_Style, (int)CBCGPLinearGaugePointer::BCGP_GAUGE_NEEDLE_TRIANGLE));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteInt (s_szTag_PointerPosition, (int)m_Position, (int)CBCGPLinearGaugePointer::BCGP_GAUGE_POSITION_NEAR));
}

CBCGPVisualInfo::XLinearColoredRange::XLinearColoredRange()
	: CBCGPVisualInfo::XGaugeColoredRange ()
{
}

CBCGPVisualInfo::XLinearColoredRange::~XLinearColoredRange()
{
}

BOOL CBCGPVisualInfo::XLinearColoredRange::FromTag (const CString& strTag)
{
	return XGaugeColoredRange::FromTag (strTag);
}

void CBCGPVisualInfo::XLinearColoredRange::ToTag (CString& strTag) const
{
	XGaugeColoredRange::ToTag(strTag);
}

CBCGPVisualInfo::XElementLinear::XElementLinear()
	: CBCGPVisualInfo::XGaugeElement (CBCGPVisualInfo::s_szLinearGauge)
	, m_bIsVertical                  (FALSE)
{
}

CBCGPVisualInfo::XElementLinear::~XElementLinear()
{
	int i = 0;
	for (i = 0; i < (int)m_arPointers.GetSize (); i++)
	{
		if (m_arPointers[i] != NULL)
		{
			delete m_arPointers[i];
		}
	}
	m_arPointers.RemoveAll ();
}

CBCGPVisualInfo::XElementNumeric::XElementNumeric()
	: CBCGPVisualInfo::XGaugeElement (CBCGPVisualInfo::s_szNumericInd)
	, m_Style                  (CBCGPNumericIndicatorImpl::BCGP_NUMERIC_INDICATOR_CLASSIC)
	, m_nCells                 (7)
	, m_nDecimals              (1)
	, m_nSeparatorWidth        (1)
	, m_bDrawSign              (FALSE)
	, m_bDrawDecimalPoint      (FALSE)
	, m_bDrawLeadingZeros      (TRUE)
	, m_dblValue               (0.0)
{
	m_nFrameSize = 1;
}

CBCGPVisualInfo::XElementNumeric::~XElementNumeric()
{
}

CBCGPVisualInfo::XElementFrame::XElementFrame()
	: CBCGPVisualInfo::XStaticGaugeElement (CBCGPVisualInfo::s_szFrame)
	, m_dblFrameSize    (1.0)
	, m_dblCornerRadius (0.0)
{
}

CBCGPVisualInfo::XElementFrame::~XElementFrame()
{
}

CBCGPVisualInfo::XElementColor::XElementColor()
	: CBCGPVisualInfo::XStaticGaugeElement (CBCGPVisualInfo::s_szColorInd)
	, m_Style                  (CBCGPColorIndicatorImpl::BCGP_COLOR_INDICATOR_ELLIPSE)
	, m_bStretched             (FALSE)
{
}

CBCGPVisualInfo::XElementColor::~XElementColor()
{
}

CBCGPVisualInfo::XElementText::XElementText()
	: CBCGPVisualInfo::XStaticGaugeElement (CBCGPVisualInfo::s_szTextInd)
	, m_brText (CBCGPBrush (CBCGPColor::Black))
{
	m_DefaultDrawFlags = BCGP_DRAW_DYNAMIC;
}

CBCGPVisualInfo::XElementText::~XElementText()
{
}

CBCGPVisualInfo::XElementImage::XElementImage()
	: CBCGPVisualInfo::XStaticGaugeElement (CBCGPVisualInfo::s_szImage)
	, m_AlignHorz       (CBCGPImageGaugeImpl::HA_Left)
	, m_AlignVert       (CBCGPImageGaugeImpl::VA_Top)
	, m_bLockAspectRatio(TRUE)	
{
	m_DefaultDrawFlags = BCGP_DRAW_DYNAMIC;
}

CBCGPVisualInfo::XElementImage::~XElementImage()
{
}

CBCGPVisualInfo::XElementSwitch::XElementSwitch()
	: CBCGPVisualInfo::XStaticGaugeElement (CBCGPVisualInfo::s_szSwitch)
	, m_Style           (CBCGPSwitchImpl::BCGP_SWITCH_RECTANGLE)
	, m_strLabelOff     (_T("Off"))
	, m_strLabelOn      (_T("On"))
	, m_bDrawTextLabels (FALSE)
	, m_bValue          (FALSE)
{
}

CBCGPVisualInfo::XElementSwitch::~XElementSwitch()
{
}

CBCGPVisualInfo::XTagCloudData::XTagCloudData()
{
}

CBCGPVisualInfo::XTagCloudData::~XTagCloudData()
{
}

BOOL CBCGPVisualInfo::XTagCloudData::FromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

 	tm.ReadDouble (s_szTag_Value, m_dblValue);
	tm.ReadEntityString (s_szTag_Label, m_strLabel);

	tm.ReadBrush (s_szTag_Fill_Brush, m_brFill);
	tm.ReadColor (s_szTag_Text_Brush, m_clrText);
	tm.ReadColor (s_szTag_Text_Highlighted_Brush, m_clrTextHighlighted);
	tm.ReadColor (s_szTag_Fill_Outline, m_clrBorder);

	return TRUE;
}

void CBCGPVisualInfo::XTagCloudData::ToTag (CString& strTag) const
{
 	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteDouble (s_szTag_Value, m_dblValue, 0.0));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteEntityString (s_szTag_Label,m_strLabel));

	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Fill_Brush, m_brFill, FALSE));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteColor (s_szTag_Text_Brush, m_clrText, CBCGPColor()));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteColor (s_szTag_Text_Highlighted_Brush, m_clrTextHighlighted, CBCGPColor()));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteColor (s_szTag_Fill_Outline, m_clrBorder, CBCGPColor()));
}

CBCGPVisualInfo::XElementTagCloud::XElementTagCloud()
	: CBCGPVisualInfo::XElement (CBCGPVisualInfo::s_szTagCloud)
	, m_SortOrder      (CBCGPTagCloud::NoSort)
	, m_bDescending    (FALSE)
	, m_nMaxWeight     (5)
	, m_szMargin       (5.0, 5.0)
	, m_dblFontSizeStep(0.0)
	, m_brFill         (CBCGPColor::White)
	, m_clrText        (CBCGPColor::Black)
	, m_clrTextHighlighted(CBCGPColor::SteelBlue)
{
	LOGFONT lf;
	globalData.fontRegular.GetLogFont(&lf);

	lf.lfHeight = lf.lfHeight * 2;
	m_fmtBase = CBCGPTextFormat(lf);

	m_fmtBase.SetTextAlignment(CBCGPTextFormat::BCGP_TEXT_ALIGNMENT_CENTER);
	m_fmtBase.SetTextVerticalAlignment(CBCGPTextFormat::BCGP_TEXT_ALIGNMENT_CENTER);
}

CBCGPVisualInfo::XElementTagCloud::~XElementTagCloud()
{
	for (int i = 0; i < (int)m_arDataObjects.GetSize (); i++)
	{
		if (m_arDataObjects[i] != NULL)
		{
			delete m_arDataObjects[i];
		}
	}
	m_arDataObjects.RemoveAll ();
}

CBCGPVisualInfo::XTreeMapData::XTreeMapData (const CString& strName)
	: m_strName (strName)
	, m_dblValue(0.0)
	, m_szMargin(1.0, 1.0)
{
}

CBCGPVisualInfo::XTreeMapData::~XTreeMapData ()
{
}

CBCGPVisualInfo::XTreeMapData* CBCGPVisualInfo::XTreeMapData::CreateFromTag (const CString& tag)
{
	CBCGPVisualInfo::XTreeMapData* pData = NULL;

	CString strName;
	{
		CBCGPTagManager tm (tag);
		tm.ReadString (s_szTag_Name, strName);
	}

	if (!strName.IsEmpty ())
	{
		pData = CBCGPVisualInfo::XTreeMapData::CreateFromName (strName);
		if (pData != NULL)
		{
			pData->FromTag (tag);
		}
	}

	return pData;
}

CBCGPVisualInfo::XTreeMapData* CBCGPVisualInfo::XTreeMapData::CreateFromName (const CString& name)
{
	CBCGPVisualInfo::XTreeMapData* pData = NULL;

	if (name.Compare (CBCGPVisualInfo::s_szTreeMapNode) == 0)
	{
		pData = new CBCGPVisualInfo::XTreeMapNode;
	}
	else if (name.Compare (CBCGPVisualInfo::s_szTreeMapGroup) == 0)
	{
		pData = new CBCGPVisualInfo::XTreeMapGroup;
	}

	return pData;
}

BOOL CBCGPVisualInfo::XTreeMapData::FromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	return TRUE;
}

void CBCGPVisualInfo::XTreeMapData::ToTag (CString& strTag) const
{
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteString (s_szTag_Name, m_strName));
}

CBCGPVisualInfo::XTreeMapNode::XTreeMapNode()
	: CBCGPVisualInfo::XTreeMapData(s_szTreeMapNode)
{
}

CBCGPVisualInfo::XTreeMapNode::~XTreeMapNode()
{
}

BOOL CBCGPVisualInfo::XTreeMapNode::FromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

 	tm.ReadDouble (s_szTag_Value, m_dblValue);
	tm.ReadEntityString (s_szTag_Label, m_strLabel);

	return XTreeMapData::FromTag (tm.GetBuffer ());
}

void CBCGPVisualInfo::XTreeMapNode::ToTag (CString& strTag) const
{
	XTreeMapData::ToTag (strTag);

 	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteDouble (s_szTag_Value, m_dblValue, 0.0));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteEntityString (s_szTag_Label, m_strLabel));
}

CBCGPVisualInfo::XTreeMapGroup::XTreeMapGroup()
	: CBCGPVisualInfo::XTreeMapData(s_szTreeMapGroup)
{
	LOGFONT lf;
	globalData.fontRegular.GetLogFont(&lf);
	m_fmtText.CreateFromLogFont(lf);

	m_fmtText.SetTextAlignment(CBCGPTextFormat::BCGP_TEXT_ALIGNMENT_CENTER);
	m_fmtText.SetTextVerticalAlignment(CBCGPTextFormat::BCGP_TEXT_ALIGNMENT_CENTER);
	m_fmtText.SetClipText();
}

CBCGPVisualInfo::XTreeMapGroup::~XTreeMapGroup()
{
	for (int i = 0; i < (int)m_arNodes.GetSize (); i++)
	{
		if (m_arNodes[i] != NULL)
		{
			delete m_arNodes[i];
		}
	}
	m_arNodes.RemoveAll ();
}

BOOL CBCGPVisualInfo::XTreeMapGroup::FromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	CString strNodes;
	if (tm.ExcludeTag (s_szTag_Nodes, strNodes))
	{
		CBCGPTagManager tmNodes (strNodes);

		CString strNode;
		while (tmNodes.ExcludeTag (s_szTag_Node, strNode))
		{
			XTreeMapData* pNode = CBCGPVisualInfo::XTreeMapData::CreateFromTag (strNode);
			if (pNode == NULL)
			{
				continue;
			}

			m_arNodes.Add (pNode);
		}
	}

	tm.ReadEntityString (s_szTag_Label, m_strLabel);
	tm.ReadDouble (s_szTag_MarginHorz, m_szMargin.cx);
	tm.ReadDouble (s_szTag_MarginVert, m_szMargin.cy);

	tm.ReadTextFormat (s_szTag_TextFormat, m_fmtText);
	tm.ReadBrush (s_szTag_Fill_Brush, m_brFill);
	tm.ReadBrush (s_szTag_Text_Brush, m_brText);

	return XTreeMapData::FromTag (tm.GetBuffer ());
}

void CBCGPVisualInfo::XTreeMapGroup::ToTag (CString& strTag) const
{
	XTreeMapData::ToTag (strTag);

	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteEntityString (s_szTag_Label, m_strLabel));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteDouble (s_szTag_MarginHorz, m_szMargin.cx, 1.0));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteDouble (s_szTag_MarginVert, m_szMargin.cy, 1.0));

	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteTextFormat (s_szTag_TextFormat, m_fmtText));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Fill_Brush, m_brFill, FALSE));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Text_Brush, m_brText, FALSE));

	CString strNodes;
	for (int i = 0; i < (int)m_arNodes.GetSize (); i++)
	{
		CString strItem;
		m_arNodes[i]->ToTag (strItem);
		CBCGPTagManager::WriteItem (strNodes, s_szTag_Node, strItem);
	}
	CBCGPTagManager::WriteItem (strTag, s_szTag_Nodes, strNodes);
}

CBCGPVisualInfo::XElementTreeMap::XElementTreeMap()
	: CBCGPVisualInfo::XElement (CBCGPVisualInfo::s_szTreeMap)
	, m_LayoutType      (CBCGPTreeMap::Squarified)
{
}

CBCGPVisualInfo::XElementTreeMap::~XElementTreeMap()
{
}

CBCGPVisualInfo::XWinUIBaseElement::XWinUIBaseElement()
	: m_bIsVisible(TRUE)
	, m_nViewResID(0)
{
}

CBCGPVisualInfo::XWinUIBaseElement::~XWinUIBaseElement()
{
}

BOOL CBCGPVisualInfo::XWinUIBaseElement::FromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	tm.ReadEntityString (s_szTag_Text_Name, m_strName);
	tm.ReadEntityString (s_szTag_ToolTip, m_strToolTipText);
	tm.ReadEntityString (s_szTag_Description, m_strToolTipDescription);
	tm.ReadBool (s_szTag_Visible, m_bIsVisible);
	tm.ReadColor (s_szTag_Text_Brush, m_colorText);

	CString strView;
	if (tm.ExcludeTag(s_szTag_View, strView))
	{
		ViewFromTag(strView);
	}

	tm.ReadString (s_szTag_RTC_Name, m_strRTI);

	return TRUE;
}

void CBCGPVisualInfo::XWinUIBaseElement::ToTag (CString& strTag) const
{
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteString (s_szTag_RTC_Name, m_strRTI));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteEntityString (s_szTag_Text_Name, m_strName));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteEntityString (s_szTag_ToolTip, m_strToolTipText));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteEntityString (s_szTag_Description, m_strToolTipDescription));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBool (s_szTag_Visible, m_bIsVisible, TRUE));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteColor (s_szTag_Text_Brush, m_colorText, CBCGPColor()));

	ViewToTag(strTag);

	CBCGPTagManager::WriteItem (strTag, s_szTag_Custom_Props, m_strCustomProps);
}

BOOL CBCGPVisualInfo::XWinUIBaseElement::ViewFromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	tm.ReadString (s_szTag_RTC_Name, m_strRTIView);
	tm.ReadUInt(s_szTag_ID_View, m_nViewResID);
	tm.ReadEntityString (s_szTag_Text_View, m_strViewTitle);

	return TRUE;
}

void CBCGPVisualInfo::XWinUIBaseElement::ViewToTag (CString& strTag) const
{
	if (m_strRTIView.IsEmpty())
	{
		return;
	}

	CString strView;

	CBCGPTagManager::WriteTag (strView, CBCGPTagManager::WriteString (s_szTag_RTC_Name, m_strRTIView));
	CBCGPTagManager::WriteTag (strView, CBCGPTagManager::WriteUInt (s_szTag_ID_View, m_nViewResID, 0));
	CBCGPTagManager::WriteTag (strView, CBCGPTagManager::WriteEntityString (s_szTag_Text_View, m_strViewTitle));

	CBCGPTagManager::WriteItem (strTag, s_szTag_View, strView);
}

CBCGPVisualInfo::XWinUICaptionButton::XWinUICaptionButton()
	: XWinUIBaseElement()
	, m_nCommandID     (0)
{
}

CBCGPVisualInfo::XWinUICaptionButton::~XWinUICaptionButton()
{
}

BOOL CBCGPVisualInfo::XWinUICaptionButton::FromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	tm.ExcludeTag(s_szTag_Custom_Props, m_strCustomProps);

	CString strView;
	if (tm.ExcludeTag(s_szTag_View, strView))
	{
		ViewFromTag(strView);
	}

	tm.ReadUInt (s_szTag_ID_Command, m_nCommandID);
	tm.ReadEntityString (s_szTag_Text_Description, m_strDescription);

	CString strImage;
	if (tm.ExcludeTag (s_szTag_Image, strImage))
	{
		m_Image.FromTag (strImage);
	}

	return XWinUIBaseElement::FromTag(tm.GetBuffer ());
}

void CBCGPVisualInfo::XWinUICaptionButton::ToTag (CString& strTag) const
{
	XWinUIBaseElement::ToTag (strTag);

	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteUInt (s_szTag_ID_Command, m_nCommandID, 0));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteEntityString (s_szTag_Text_Description, m_strDescription));

	CString strImage;
	m_Image.ToTag (strImage);
	CBCGPTagManager::WriteItem (strTag, s_szTag_Image, strImage);
}

CBCGPVisualInfo::XWinUIGroupCaption::XWinUIGroupCaption()
	: XWinUIBaseElement()
	, m_nID         (-1)
	, m_bIsClickable(FALSE)
{
}

CBCGPVisualInfo::XWinUIGroupCaption::~XWinUIGroupCaption()
{
}

BOOL CBCGPVisualInfo::XWinUIGroupCaption::FromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	tm.ExcludeTag(s_szTag_Custom_Props, m_strCustomProps);

	tm.ReadInt (s_szTag_Group, m_nID);
	tm.ReadBool (s_szTag_Clickable, m_bIsClickable);
	tm.ReadBrush (s_szTag_Fill_Brush, m_brFillGroup);
	tm.ReadBrush (s_szTag_Fill_Outline, m_brOutlineGroup);

	return XWinUIBaseElement::FromTag(tm.GetBuffer ());
}

void CBCGPVisualInfo::XWinUIGroupCaption::ToTag (CString& strTag) const
{
	XWinUIBaseElement::ToTag (strTag);

	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteInt (s_szTag_Group, m_nID, -1));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBool (s_szTag_Clickable, m_bIsClickable, FALSE));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Fill_Brush, m_brFillGroup, FALSE));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Fill_Outline, m_brOutlineGroup, FALSE));
}

CBCGPVisualInfo::XWinUITile::XWinUITile()
	: XWinUIBaseElement        ()
	, m_nBadgeNumber        (-1)
	, m_BadgeGlyph          (CBCGPWinUITile::BCGP_NONE)
	, m_nCustomBadgeIndex   (-1)
	, m_nImportance         (0)
	, m_bStretchImage       (FALSE)
	, m_nGroup              (0)
	, m_Type                (CBCGPWinUITile::BCGP_TILE_REGULAR)
	, m_dblBorderWidth      (2.0)
{
}

CBCGPVisualInfo::XWinUITile::~XWinUITile()
{
}

BOOL CBCGPVisualInfo::XWinUITile::FromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	tm.ExcludeTag(s_szTag_Custom_Props, m_strCustomProps);

	tm.ReadEntityString (s_szTag_Text_Header, m_strHeader);
	tm.ReadEntityString (s_szTag_Text, m_strText);

	int glyph = (int)CBCGPWinUITile::BCGP_NONE;
	tm.ReadInt (s_szTag_Badge_Glyph, glyph);
	m_BadgeGlyph = (CBCGPWinUITile::BCGP_WINUI_BADGE_GLYPH)bcg_clamp(glyph, (int)CBCGPWinUITile::BCGP_WINUI_BADGE_GLYPH_FIRST, (int)CBCGPWinUITile::BCGP_WINUI_BADGE_GLYPH_LAST);

	tm.ReadInt (s_szTag_Badge_Number, m_nBadgeNumber);
	tm.ReadInt (s_szTag_Badge_Custom_Number, m_nCustomBadgeIndex);

	tm.ReadInt (s_szTag_Group, m_nGroup);
	tm.ReadInt (s_szTag_Importance, m_nImportance);

	int type = (int)CBCGPWinUITile::BCGP_TILE_REGULAR;
	tm.ReadInt (s_szTag_Tile_Type, type);
	m_Type = (CBCGPWinUITile::BCGP_WINUI_TILE_TYPE)bcg_clamp(type, (int)CBCGPWinUITile::BCGP_WINUI_TILE_TYPE_FIRST, (int)CBCGPWinUITile::BCGP_WINUI_TILE_TYPE_LAST);

	tm.ReadDouble (s_szTag_BorderWidth, m_dblBorderWidth);

	CString strImage;
	if (tm.ExcludeTag (s_szTag_Image, strImage))
	{
		m_Image.FromTag (strImage);
	}

	tm.ReadBool (s_szTag_ImageStretch, m_bStretchImage);

	tm.ReadBrush (s_szTag_Fill_Brush, m_brushBackground);
	tm.ReadColor (s_szTag_Fill_Outline, m_colorBorder);

	return XWinUIBaseElement::FromTag(tm.GetBuffer ());
}

void CBCGPVisualInfo::XWinUITile::ToTag (CString& strTag) const
{
	XWinUIBaseElement::ToTag (strTag);

	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteEntityString (s_szTag_Text_Header, m_strHeader));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteEntityString (s_szTag_Text, m_strText));

	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteInt (s_szTag_Badge_Glyph, (int)m_BadgeGlyph, (int)CBCGPWinUITile::BCGP_NONE));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteInt (s_szTag_Badge_Number, m_nBadgeNumber, -1));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteInt (s_szTag_Badge_Custom_Number, m_nCustomBadgeIndex, -1));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteInt (s_szTag_Group, m_nGroup, 0));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteInt (s_szTag_Importance, m_nImportance, 0));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteInt (s_szTag_Tile_Type, m_Type, (int)CBCGPWinUITile::BCGP_TILE_REGULAR));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteDouble (s_szTag_BorderWidth, m_dblBorderWidth, 2.0));

	CString strImage;
	m_Image.ToTag (strImage);
	CBCGPTagManager::WriteItem (strTag, s_szTag_Image, strImage);

	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBool (s_szTag_ImageStretch, m_bStretchImage, FALSE));

	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Fill_Brush, m_brushBackground, FALSE));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteColor (s_szTag_Fill_Outline, m_colorBorder, CBCGPColor()));
}

CBCGPVisualInfo::XElementWinUITiles::XElementWinUITiles()
	: CBCGPVisualInfo::XElement (CBCGPVisualInfo::s_szWinUITiles)
	, m_bIsHorizontalLayout     (TRUE)
	, m_bRoundedShapes          (FALSE)
	, m_bHasNavigationBackButton(FALSE)
	, m_pRTINavigationBackButton(NULL)
	, m_szMargin                (10.0, 10.0)
	, m_szSquare                (-1.0, -1.0)
	, m_dblCaptionExtraHeight   (0.0)
	, m_brFill                  (CBCGPColor::SteelBlue)
	, m_colorCaptionForeground  (CBCGPColor::White)
	, m_bTilesDragAndDrop       (FALSE)
{
	m_szSquare = GetDefaultSquareSize ();
}

CBCGPVisualInfo::XElementWinUITiles::~XElementWinUITiles()
{
	int i = 0;

	for (i = 0; i < (int)m_arGroupCaptions.GetSize (); i++)
	{
		if (m_arGroupCaptions[i] != NULL)
		{
			delete m_arGroupCaptions[i];
		}
	}
	m_arGroupCaptions.RemoveAll ();	

	for (i = 0; i < (int)m_arCaptionButtons.GetSize (); i++)
	{
		if (m_arCaptionButtons[i] != NULL)
		{
			delete m_arCaptionButtons[i];
		}
	}
	m_arCaptionButtons.RemoveAll ();	

	for (i = 0; i < (int)m_arTiles.GetSize (); i++)
	{
		if (m_arTiles[i] != NULL)
		{
			delete m_arTiles[i];
		}
	}
	m_arTiles.RemoveAll ();	
}

CBCGPSize CBCGPVisualInfo::XElementWinUITiles::GetDefaultSquareSize() const
{
	LOGFONT lf;
	globalData.fontRegular.GetLogFont(&lf);
	return CBCGPSize(fabs(12. * lf.lfHeight) + 10, fabs(10. * lf.lfHeight) + 10);
}

CBCGPVisualInfo::XElementDiagramConnector::XElementDiagramConnector()
	: CBCGPVisualInfo::XDiagramElementConnector(CBCGPVisualInfo::s_szDiagramConnector)
{
}

CBCGPVisualInfo::XElementDiagramConnector::~XElementDiagramConnector()
{
}

CBCGPVisualInfo::XElementDiagramConnectorShelf::XElementDiagramConnectorShelf()
	: CBCGPVisualInfo::XDiagramElementConnector(CBCGPVisualInfo::s_szDiagramConnectorShelf)
	, m_dShelfOffset (10.0)
{
}

CBCGPVisualInfo::XElementDiagramConnectorShelf::~XElementDiagramConnectorShelf()
{
}

CBCGPVisualInfo::XElementDiagramConnectorElbow::XElementDiagramConnectorElbow()
	: CBCGPVisualInfo::XDiagramElementConnector(CBCGPVisualInfo::s_szDiagramConnectorElbow)
	, m_Orientation (CBCGPDiagramElbowConnector::Auto)
{
}

CBCGPVisualInfo::XElementDiagramConnectorElbow::~XElementDiagramConnectorElbow()
{
}

CBCGPVisualInfo::XElementDiagramShape::XElementDiagramShape()
	: CBCGPVisualInfo::XDiagramElement (CBCGPVisualInfo::s_szDiagramShape)
	, m_shape (CBCGPDiagramShape::Box)
{
}

CBCGPVisualInfo::XElementDiagramShape::XElementDiagramShape(const CString& strElementName)
	: CBCGPVisualInfo::XDiagramElement (strElementName)
	, m_shape (CBCGPDiagramShape::Box)
{
}

CBCGPVisualInfo::XElementDiagramShape::~XElementDiagramShape()
{
}

CBCGPVisualInfo::XElementDiagramTable::XElementDiagramTable()
	: CBCGPVisualInfo::XElementDiagramShape (CBCGPVisualInfo::s_szDiagramTable)
	, m_bCaption (FALSE)
{
}

CBCGPVisualInfo::XElementDiagramTable::~XElementDiagramTable()
{
}

CBCGPVisualInfo::XElementDiagramImage::XElementDiagramImage()
	: CBCGPVisualInfo::XDiagramElement (CBCGPVisualInfo::s_szDiagramImage)
	, m_AlignHorz       (CBCGPDiagramImageObject::HA_Left)
	, m_AlignVert       (CBCGPDiagramImageObject::VA_Top)
	, m_bLockAspectRatio(TRUE)
{
}

CBCGPVisualInfo::XElementDiagramImage::~XElementDiagramImage()
{
}

CBCGPVisualInfo::XElementDiagramCustom::XElementDiagramCustom()
	: CBCGPVisualInfo::XDiagramElement (CBCGPVisualInfo::s_szDiagramCustom)
{
}

CBCGPVisualInfo::XElementDiagramCustom::~XElementDiagramCustom()
{
}

BOOL CBCGPVisualInfo::XGaugeElement::FromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	CString strColors;
	if (tm.ExcludeTag (s_szTag_Colors, strColors))
	{
		ColorsFromTag (strColors);
	}

	tm.ReadInt (s_szTag_FrameSize, m_nFrameSize);

	int pos = (int)CBCGPGaugeImpl::BCGP_SUB_GAUGE_NONE;
	tm.ReadInt (s_szTag_Pos, pos);
	m_Pos = (CBCGPGaugeImpl::BCGP_SUB_GAUGE_POS)bcg_clamp(pos, (int)CBCGPGaugeImpl::BCGP_SUB_GAUGE_POS_FIRST, (int)CBCGPGaugeImpl::BCGP_SUB_GAUGE_POS_LAST);

	tm.ReadPoint (s_szTag_Offset, m_ptOffset);
	tm.ReadBool (s_szTag_InteractiveMode, m_bIsInteractiveMode);
	tm.ReadEntityString (s_szTag_ToolTip, m_strToolTip);
	tm.ReadEntityString (s_szTag_Description, m_strDescription);

	return XElement::FromTag(tm.GetBuffer ());
}

void CBCGPVisualInfo::XGaugeElement::ToTag (CString& strTag) const
{
	XElement::ToTag (strTag);

	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteInt (s_szTag_FrameSize, m_nFrameSize, 6));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteInt (s_szTag_Pos, (int)m_Pos, (int)CBCGPGaugeImpl::BCGP_SUB_GAUGE_NONE));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WritePoint (s_szTag_Offset, m_ptOffset, CPoint(0, 0)));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBool (s_szTag_InteractiveMode, m_bIsInteractiveMode, FALSE));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteEntityString (s_szTag_ToolTip, m_strToolTip));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteEntityString (s_szTag_Description, m_strDescription));

	CString strColors;
	ColorsToTag (strColors);
	CBCGPTagManager::WriteItem (strTag, s_szTag_Colors, strColors);
}

BOOL CBCGPVisualInfo::XStaticGaugeElement::FromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	tm.ReadBrush (s_szTag_Back_Brush, m_brFill);
	tm.ReadBrush (s_szTag_Back_Outline, m_brOutline);
	tm.ReadDouble (s_szTag_Opacity, m_dblOpacity);
	m_dblOpacity = bcg_clamp(m_dblOpacity, 0.0, 1.0);
	tm.ReadDword (s_szTag_DrawFlags, m_DefaultDrawFlags);
	
	return XGaugeElement::FromTag(tm.GetBuffer ());
}

void CBCGPVisualInfo::XStaticGaugeElement::ToTag (CString& strTag) const
{
	XGaugeElement::ToTag (strTag);

	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Back_Brush, m_brFill));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Back_Outline, m_brOutline));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteDouble (s_szTag_Opacity, m_dblOpacity, 1.0));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteDword (s_szTag_DrawFlags, m_DefaultDrawFlags, 0));
}

BOOL CBCGPVisualInfo::XDiagramElement::FromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	tm.ReadString (s_szTag_Custom_Name, m_strCustomName);
	tm.ExcludeTag (s_szTag_Custom_Props, m_strCustomProps);

	CString strID;
	if (!tm.ExcludeTag (s_szTag_Item_Id, strID))
	{
		return FALSE;
	}
	m_idItem.FromTag (strID);

	CString strDataObjects;
	if (tm.ExcludeTag (s_szTag_DataObjects, strDataObjects))
	{
		CBCGPTagManager tmDataObjects (strDataObjects);

		CString strDataObject;
		while (tmDataObjects.ExcludeTag (s_szTag_DataObject, strDataObject))
		{
			XData* pDataObject = (XData*)CBCGPVisualInfo::XData::CreateFromTag (strDataObject);
			if (pDataObject == NULL)
			{
				continue;
			}

			if (pDataObject->GetName ().Compare (CBCGPVisualInfo::s_szDataDiagramText) == 0)
			{
				m_arDataObjects.Add (pDataObject);
			}
			else
			{
				delete pDataObject;
			}
		}
	}

	tm.ReadBrush (s_szTag_Fill_Brush, m_brFill);
	tm.ReadBrush (s_szTag_Fill_Outline, m_brOutline);
	tm.ReadBrush (s_szTag_Shadow_Brush, m_brShadow);
	tm.ReadDouble (s_szTag_Thickness, m_Thickness);
	tm.ReadStrokeStyle (s_szTag_StrokeStyle, m_StrokeStyle);

	return XElement::FromTag(tm.GetBuffer ());
}

void CBCGPVisualInfo::XDiagramElement::ToTag (CString& strTag) const
{
	XElement::ToTag (strTag);

	CString strID;
	m_idItem.ToTag (strID);
	CBCGPTagManager::WriteItem (strTag, s_szTag_Item_Id, strID);
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Fill_Brush, m_brFill));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Fill_Outline, m_brOutline));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Shadow_Brush, m_brShadow, FALSE));

	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteDouble (s_szTag_Thickness, m_Thickness, 1.0));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteStrokeStyle (s_szTag_StrokeStyle, m_StrokeStyle));

	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteString (s_szTag_Custom_Name, m_strCustomName));
	CBCGPTagManager::WriteItem (strTag, s_szTag_Custom_Props, m_strCustomProps);

	CString strDataObjects;
	for (int i = 0; i < (int)m_arDataObjects.GetSize (); i++)
	{
		CString strDataObject;
		m_arDataObjects[i]->ToTag (strDataObject);
		CBCGPTagManager::WriteItem (strDataObjects, s_szTag_DataObject, strDataObject, TRUE);
	}
	CBCGPTagManager::WriteItem (strTag, s_szTag_DataObjects, strDataObjects);
}

BOOL CBCGPVisualInfo::XDiagramElementConnector::FromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	CString strAnchors;
	if (tm.ExcludeTag (s_szTag_Anchors, strAnchors))
	{
		CBCGPTagManager tmAnchors (strAnchors);

		CString strAnchor;
		while (tmAnchors.ExcludeTag (s_szTag_Anchor, strAnchor))
		{
			XDiagramAnchorPoint* pAnchor = new XDiagramAnchorPoint;
			if (pAnchor->FromTag (strAnchor))
			{
				m_arPoints.Add (pAnchor);
			}
			else
			{
				delete pAnchor;
			}
		}
	}

	int type = CBCGPDiagramConnector::BCGP_CURVE_TYPE_SPLINE;
	tm.ReadInt (s_szTag_Curve_Type, type);
	m_curveType = (CBCGPDiagramConnector::BCGP_CURVE_TYPE)bcg_clamp(type, 
		CBCGPDiagramConnector::BCGP_CURVE_TYPE_FIRST, CBCGPDiagramConnector::BCGP_CURVE_TYPE_LAST);

	CString strArrow;
	if (tm.ExcludeTag (s_szTag_Start_Arrow, strArrow))
	{
		m_arrowBegin.FromTag (strArrow);
	}

	strArrow.Empty ();
	if (tm.ExcludeTag (s_szTag_Finish_Arrow, strArrow))
	{
		m_arrowEnd.FromTag (strArrow);
	}

	return XDiagramElement::FromTag (tm.GetBuffer ());
}

void CBCGPVisualInfo::XDiagramElementConnector::ToTag (CString& strTag) const
{
	XDiagramElement::ToTag (strTag);

	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteInt (s_szTag_Curve_Type, (int)m_curveType, CBCGPDiagramConnector::BCGP_CURVE_TYPE_SPLINE));

	CString strArrow;
	m_arrowBegin.ToTag (strArrow);
	CBCGPTagManager::WriteItem (strTag, s_szTag_Start_Arrow, strArrow);

	strArrow.Empty ();
	m_arrowEnd.ToTag (strArrow);
	CBCGPTagManager::WriteItem (strTag, s_szTag_Finish_Arrow, strArrow);

	CString strAnchors;
	for (int i = 0; i < (int)m_arPoints.GetSize (); i++)
	{
		CString strAnchor;
		m_arPoints[i]->ToTag (strAnchor);
		CBCGPTagManager::WriteItem (strAnchors, s_szTag_Anchor, strAnchor, TRUE);
	}
	CBCGPTagManager::WriteItem (strTag, s_szTag_Anchors, strAnchors);
}

BOOL CBCGPVisualInfo::XElementCircular::FromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	BOOL bKnob = GetElementName().Compare (CBCGPVisualInfo::s_szKnob) == 0;

	CString strElements;
	if (tm.ExcludeTag (s_szTag_Elements, strElements))
	{
		CBCGPTagManager tmElements (strElements);

		CString strElement;
		while (tmElements.ExcludeTag (s_szTag_Element, strElement))
		{
			CString strElementName;
			{
				CBCGPTagManager tm (strElement);
				tm.ReadString (CBCGPBaseInfo::s_szTag_ElementName, strElementName);
			}

			if (!strElementName.IsEmpty ())
			{
				XGaugeElement* pElement = (XGaugeElement*)CBCGPVisualInfo::XGaugeElement::CreateFromName (strElementName);
				if (pElement != NULL)
				{
					pElement->FromTag (strElement);
					m_arSubGauges.Add (pElement);
				}
			}
		}
	}

	CString strScales;
	if (tm.ExcludeTag (s_szTag_Scales, strScales))
	{
		CBCGPTagManager tmScales (strScales);

		CString strScale;
		while (tmScales.ExcludeTag (s_szTag_Scale, strScale))
		{
			XCircularScale* pScale = new XCircularScale;
			if (pScale->FromTag (strScale))
			{
				m_arScales.Add (pScale);
			}
			else
			{
				delete pScale;
			}
		}
	}

	CString strPointers;
	if (tm.ExcludeTag (s_szTag_Pointers, strPointers))
	{
		CBCGPTagManager tmPointers (strPointers);

		CString strPointer;
		while (tmPointers.ExcludeTag (s_szTag_Pointer, strPointer))
		{
			XData* pPointer = CBCGPVisualInfo::XData::CreateFromTag (strPointer);
			if (pPointer == NULL)
			{
				continue;
			}

			BOOL bAdd = FALSE;
			if (bKnob)
			{
				bAdd = pPointer->GetName ().Compare (CBCGPVisualInfo::s_szDataPointerKnob) == 0;
			}
			else
			{
				bAdd = pPointer->GetName ().Compare (CBCGPVisualInfo::s_szDataPointerCircular) == 0;
			}
						
			if (bAdd)
			{
				m_arPointers.Add (pPointer);
			}
			else
			{
				delete pPointer;
			}
		}
	}

	CString strRanges;
	if (tm.ExcludeTag (s_szTag_Ranges, strRanges))
	{
		CBCGPTagManager tmRanges (strRanges);

		CString strRange;
		while (tmRanges.ExcludeTag (s_szTag_Range, strRange))
		{
			XCircularColoredRange* pRange = new XCircularColoredRange;
			if (pRange->FromTag (strRange))
			{
				m_arRanges.Add (pRange);
			}
			else
			{
				delete pRange;
			}
		}
	}

	tm.ReadDouble (s_szTag_CapSize, m_dblCapSize);
	tm.ReadBool (s_szTag_Shape_Area, m_bShapeByTicksArea);
	tm.ReadTextFormat (s_szTag_TextFormat, m_fmtText);

	return XGaugeElement::FromTag (tm.GetBuffer ());
}

void CBCGPVisualInfo::XElementCircular::ToTag (CString& strTag) const
{
	XGaugeElement::ToTag (strTag);

	int i = 0;

	CString strScales;
	for (i = 0; i < (int)m_arScales.GetSize (); i++)
	{
		CString strScale;
		m_arScales[i]->ToTag (strScale);
		CBCGPTagManager::WriteItem (strScales, s_szTag_Scale, strScale, TRUE);
	}
	CBCGPTagManager::WriteItem (strTag, s_szTag_Scales, strScales);

	CString strPointers;
	for (i = 0; i < (int)m_arPointers.GetSize (); i++)
	{
		CString strPointer;
		m_arPointers[i]->ToTag (strPointer);
		CBCGPTagManager::WriteItem (strPointers, s_szTag_Pointer, strPointer, TRUE);
	}
	CBCGPTagManager::WriteItem (strTag, s_szTag_Pointers, strPointers);

	CString strRanges;
	for (i = 0; i < (int)m_arRanges.GetSize (); i++)
	{
		CString strRange;
		m_arRanges[i]->ToTag (strRange);
		CBCGPTagManager::WriteItem (strRanges, s_szTag_Range, strRange, TRUE);
	}
	CBCGPTagManager::WriteItem (strTag, s_szTag_Ranges, strRanges);

	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteDouble (s_szTag_CapSize, m_dblCapSize, 20.0));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBool (s_szTag_Shape_Area, m_bShapeByTicksArea, FALSE));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteTextFormat (s_szTag_TextFormat, m_fmtText));

	CString strElements;
	for (i = 0; i < (int)m_arSubGauges.GetSize (); i++)
	{
		CString strElement;
		m_arSubGauges[i]->ToTag (strElement);
		CBCGPTagManager::WriteItem (strElements, s_szTag_Element, strElement);
	}
	CBCGPTagManager::WriteItem (strTag, s_szTag_Elements, strElements);
}

BOOL CBCGPVisualInfo::XElementCircular::ColorsFromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	tm.ReadBrush (s_szTag_Pointer_Brush, m_Colors.m_brPointerFill);
	tm.ReadBrush (s_szTag_Pointer_Outline, m_Colors.m_brPointerOutline);
	tm.ReadBrush (s_szTag_Scale_Brush, m_Colors.m_brScaleFill);
	tm.ReadBrush (s_szTag_Scale_Outline, m_Colors.m_brScaleOutline);
	tm.ReadBrush (s_szTag_Text_Brush, m_Colors.m_brText);
	tm.ReadBrush (s_szTag_Frame_Brush, m_Colors.m_brFrameFill);
	tm.ReadBrush (s_szTag_Frame_BrushInv, m_Colors.m_brFrameFillInv);
	tm.ReadBrush (s_szTag_Frame_Outline, m_Colors.m_brFrameOutline);
	tm.ReadBrush (s_szTag_Fill_Brush, m_Colors.m_brFill);
	tm.ReadBrush (s_szTag_Cap_Brush, m_Colors.m_brCapFill);
	tm.ReadBrush (s_szTag_Cap_Outline, m_Colors.m_brCapOutline);
	tm.ReadBrush (s_szTag_TickMark_Brush, m_Colors.m_brTickMarkFill);
	tm.ReadBrush (s_szTag_TickMark_Outline, m_Colors.m_brTickMarkOutline);

	return TRUE;
}

void CBCGPVisualInfo::XElementCircular::ColorsToTag (CString& strTag) const
{
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Pointer_Brush, m_Colors.m_brPointerFill));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Pointer_Outline, m_Colors.m_brPointerOutline));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Scale_Brush, m_Colors.m_brScaleFill));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Scale_Outline, m_Colors.m_brScaleOutline));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Text_Brush, m_Colors.m_brText));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Frame_Brush, m_Colors.m_brFrameFill));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Frame_BrushInv, m_Colors.m_brFrameFillInv));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Frame_Outline, m_Colors.m_brFrameOutline));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Fill_Brush, m_Colors.m_brFill));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Cap_Brush, m_Colors.m_brCapFill));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Cap_Outline, m_Colors.m_brCapOutline));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_TickMark_Brush, m_Colors.m_brTickMarkFill));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_TickMark_Outline, m_Colors.m_brTickMarkOutline));
}

BOOL CBCGPVisualInfo::XElementAnalogClock::FromTag (const CString& strTag)
{
	if (!XElementCircular::FromTag (strTag))
	{
		return FALSE;
	}

	CBCGPTagManager tm (strTag);
	tm.ReadInt (s_szTag_DateIndex, m_nDateIndex);

	if (m_nDateIndex >= (int)m_arSubGauges.GetSize ())
	{
		m_nDateIndex = -1;
	}

	return TRUE;
}

void CBCGPVisualInfo::XElementAnalogClock::ToTag (CString& strTag) const
{
	XElementCircular::ToTag(strTag);

	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteInt (s_szTag_DateIndex, m_nDateIndex, -1));
}

BOOL CBCGPVisualInfo::XElementLinear::FromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	CString strScales;
	if (tm.ExcludeTag (s_szTag_Scales, strScales))
	{
		CBCGPTagManager tmScales (strScales);

		CString strScale;
		while (tmScales.ExcludeTag (s_szTag_Scale, strScale))
		{
			XLinearScale* pScale = new XLinearScale;
			if (pScale->FromTag (strScale))
			{
				m_arScales.Add (pScale);
			}
			else
			{
				delete pScale;
			}
		}
	}

	CString strPointers;
	if (tm.ExcludeTag (s_szTag_Pointers, strPointers))
	{
		CBCGPTagManager tmPointers (strPointers);

		CString strPointer;
		while (tmPointers.ExcludeTag (s_szTag_Pointer, strPointer))
		{
			XData* pPointer = (XData*)CBCGPVisualInfo::XData::CreateFromTag (strPointer);
			if (pPointer == NULL)
			{
				continue;
			}

			if (pPointer->GetName ().Compare (CBCGPVisualInfo::s_szDataPointerLinear) == 0)
			{
				m_arPointers.Add (pPointer);
			}
			else
			{
				delete pPointer;
			}
		}
	}

	CString strRanges;
	if (tm.ExcludeTag (s_szTag_Ranges, strRanges))
	{
		CBCGPTagManager tmRanges (strRanges);

		CString strRange;
		while (tmRanges.ExcludeTag (s_szTag_Range, strRange))
		{
			XLinearColoredRange* pRange = new XLinearColoredRange;
			if (pRange->FromTag (strRange))
			{
				m_arRanges.Add (pRange);
			}
			else
			{
				delete pRange;
			}
		}
	}

	tm.ReadBool (s_szTag_Vertical, m_bIsVertical);
	tm.ReadTextFormat (s_szTag_TextFormat, m_fmtText);

	return XGaugeElement::FromTag (tm.GetBuffer ());
}

void CBCGPVisualInfo::XElementLinear::ToTag (CString& strTag) const
{
	XGaugeElement::ToTag (strTag);

	int i = 0;

	CString strScales;
	for (i = 0; i < (int)m_arScales.GetSize (); i++)
	{
		CString strScale;
		m_arScales[i]->ToTag (strScale);
		CBCGPTagManager::WriteItem (strScales, s_szTag_Scale, strScale, TRUE);
	}
	CBCGPTagManager::WriteItem (strTag, s_szTag_Scales, strScales);

	CString strPointers;
	for (i = 0; i < (int)m_arPointers.GetSize (); i++)
	{
		CString strPointer;
		m_arPointers[i]->ToTag (strPointer);
		CBCGPTagManager::WriteItem (strPointers, s_szTag_Pointer, strPointer, TRUE);
	}
	CBCGPTagManager::WriteItem (strTag, s_szTag_Pointers, strPointers);

	CString strRanges;
	for (i = 0; i < (int)m_arRanges.GetSize (); i++)
	{
		CString strRange;
		m_arRanges[i]->ToTag (strRange);
		CBCGPTagManager::WriteItem (strRanges, s_szTag_Range, strRange, TRUE);
	}
	CBCGPTagManager::WriteItem (strTag, s_szTag_Ranges, strRanges);
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBool (s_szTag_Vertical, m_bIsVertical, FALSE));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteTextFormat (s_szTag_TextFormat, m_fmtText));
}

BOOL CBCGPVisualInfo::XElementLinear::ColorsFromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	tm.ReadBrush (s_szTag_Pointer_Brush, m_Colors.m_brPointerFill);
	tm.ReadBrush (s_szTag_Pointer_Outline, m_Colors.m_brPointerOutline);
	tm.ReadBrush (s_szTag_Scale_Brush, m_Colors.m_brScaleFill);
	tm.ReadBrush (s_szTag_Scale_Outline, m_Colors.m_brScaleOutline);
	tm.ReadBrush (s_szTag_Text_Brush, m_Colors.m_brText);
	tm.ReadBrush (s_szTag_Frame_Brush, m_Colors.m_brFrameFill);
	tm.ReadBrush (s_szTag_Frame_Outline, m_Colors.m_brFrameOutline);
	tm.ReadBrush (s_szTag_Fill_Brush, m_Colors.m_brFill);
	tm.ReadBrush (s_szTag_TickMark_Brush, m_Colors.m_brTickMarkFill);
	tm.ReadBrush (s_szTag_TickMark_Outline, m_Colors.m_brTickMarkOutline);

	return TRUE;
}

void CBCGPVisualInfo::XElementLinear::ColorsToTag (CString& strTag) const
{
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Pointer_Brush, m_Colors.m_brPointerFill));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Pointer_Outline, m_Colors.m_brPointerOutline));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Scale_Brush, m_Colors.m_brScaleFill));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Scale_Outline, m_Colors.m_brScaleOutline));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Text_Brush, m_Colors.m_brText));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Frame_Brush, m_Colors.m_brFrameFill));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Frame_Outline, m_Colors.m_brFrameOutline));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Fill_Brush, m_Colors.m_brFill));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_TickMark_Brush, m_Colors.m_brTickMarkFill));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_TickMark_Outline, m_Colors.m_brTickMarkOutline));
}

BOOL CBCGPVisualInfo::XElementNumeric::FromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	int style = (int)CBCGPNumericIndicatorImpl::BCGP_NUMERIC_INDICATOR_CLASSIC;
	tm.ReadInt (s_szTag_Style, style);
	m_Style = (CBCGPNumericIndicatorImpl::BCGPNumericIndicatorStyle)bcg_clamp(style, (int)CBCGPNumericIndicatorImpl::BCGP_NUMERIC_INDICATOR_STYLE_FIRST, (int)CBCGPNumericIndicatorImpl::BCGP_NUMERIC_INDICATOR_STYLE_LAST);

	tm.ReadInt (s_szTag_Cells, m_nCells);
	tm.ReadInt (s_szTag_Decimals, m_nDecimals);
	if (m_Style == CBCGPNumericIndicatorImpl::BCGP_NUMERIC_INDICATOR_CLASSIC)
	{
		tm.ReadInt (s_szTag_SeparatorWidth, m_nSeparatorWidth);
	}
	else
	{
		m_nSeparatorWidth = 0;
	}
	tm.ReadBool (s_szTag_DrawSign, m_bDrawSign);
	tm.ReadBool (s_szTag_DrawDecimalPoint, m_bDrawDecimalPoint);
	tm.ReadBool (s_szTag_DrawLeadingZeros, m_bDrawLeadingZeros);
	tm.ReadDouble (s_szTag_DataValue, m_dblValue);

	if (m_Style == CBCGPNumericIndicatorImpl::BCGP_NUMERIC_INDICATOR_CLASSIC)
	{
		tm.ReadTextFormat (s_szTag_TextFormat, m_fmtText);
	}

	return XGaugeElement::FromTag (tm.GetBuffer ());
}

void CBCGPVisualInfo::XElementNumeric::ToTag (CString& strTag) const
{
	XGaugeElement::ToTag (strTag);

	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteInt (s_szTag_Style, (int)m_Style, (int)CBCGPNumericIndicatorImpl::BCGP_NUMERIC_INDICATOR_CLASSIC));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteInt (s_szTag_Cells, m_nCells, 7));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteInt (s_szTag_Decimals, m_nDecimals, 1));
	if (m_Style == CBCGPNumericIndicatorImpl::BCGP_NUMERIC_INDICATOR_CLASSIC)
	{
		CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteInt (s_szTag_SeparatorWidth, m_nSeparatorWidth, 1));
	}
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBool (s_szTag_DrawSign, m_bDrawSign, FALSE));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBool (s_szTag_DrawDecimalPoint, m_bDrawDecimalPoint, FALSE));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBool (s_szTag_DrawLeadingZeros, m_bDrawLeadingZeros, TRUE));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteDouble (s_szTag_DataValue, m_dblValue, m_dblValue + 1.0));

	if (m_Style == CBCGPNumericIndicatorImpl::BCGP_NUMERIC_INDICATOR_CLASSIC)
	{
		CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteTextFormat (s_szTag_TextFormat, m_fmtText));
	}
}

BOOL CBCGPVisualInfo::XElementNumeric::ColorsFromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	tm.ReadBrush (s_szTag_Fill_Brush, m_Colors.m_brFill);
	tm.ReadBrush (s_szTag_Fill_Outline, m_Colors.m_brOutline);
	tm.ReadBrush (s_szTag_Digit_Brush, m_Colors.m_brDigit);
	tm.ReadBrush (s_szTag_Decimal_Brush, m_Colors.m_brDecimal);
	tm.ReadBrush (s_szTag_Sign_Brush, m_Colors.m_brSign);
	tm.ReadBrush (s_szTag_Dot_Brush, m_Colors.m_brDot);

	if (m_Style == CBCGPNumericIndicatorImpl::BCGP_NUMERIC_INDICATOR_CLASSIC)
	{
		tm.ReadBrush (s_szTag_Separator_Brush, m_Colors.m_brSeparator);
	}
	else
	{
		m_Colors.m_brSeparator = m_Colors.m_brDigit;
	}

	return TRUE;
}

void CBCGPVisualInfo::XElementNumeric::ColorsToTag (CString& strTag) const
{
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Fill_Brush, m_Colors.m_brFill));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Fill_Outline, m_Colors.m_brOutline));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Digit_Brush, m_Colors.m_brDigit));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Decimal_Brush, m_Colors.m_brDecimal));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Sign_Brush, m_Colors.m_brSign));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Dot_Brush, m_Colors.m_brDot));

	if (m_Style == CBCGPNumericIndicatorImpl::BCGP_NUMERIC_INDICATOR_CLASSIC)
	{
		CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Separator_Brush, m_Colors.m_brSeparator));
	}
}

BOOL CBCGPVisualInfo::XElementFrame::FromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	tm.ReadStrokeStyle (s_szTag_StrokeStyle, m_strokeStyle);
	tm.ReadDouble (s_szTag_FrameSize, m_dblFrameSize);
	tm.ReadDouble (s_szTag_Radius, m_dblCornerRadius);

	return XStaticGaugeElement::FromTag (tm.GetBuffer ());
}

void CBCGPVisualInfo::XElementFrame::ToTag (CString& strTag) const
{
	XStaticGaugeElement::ToTag (strTag);

	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteStrokeStyle (s_szTag_StrokeStyle, m_strokeStyle));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteDouble (s_szTag_FrameSize, m_dblFrameSize, 1.0));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteDouble (s_szTag_Radius, m_dblCornerRadius, 0.0));
}

BOOL CBCGPVisualInfo::XElementFrame::ColorsFromTag (const CString& /*strTag*/)
{
	return TRUE;
}

void CBCGPVisualInfo::XElementFrame::ColorsToTag (CString& /*strTag*/) const
{
}

BOOL CBCGPVisualInfo::XElementColor::FromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	int style = (int)CBCGPColorIndicatorImpl::BCGP_COLOR_INDICATOR_ELLIPSE;
	tm.ReadInt (s_szTag_Style, style);
	m_Style = (CBCGPColorIndicatorImpl::BCGPColorIndicatorStyle)bcg_clamp(style, (int)CBCGPColorIndicatorImpl::BCGP_COLOR_INDICATOR_STYLE_FIRST, (int)CBCGPColorIndicatorImpl::BCGP_COLOR_INDICATOR_STYLE_LAST);
	tm.ReadBool (s_szTag_Stretched, m_bStretched);

	return XStaticGaugeElement::FromTag (tm.GetBuffer ());
}

void CBCGPVisualInfo::XElementColor::ToTag (CString& strTag) const
{
	XStaticGaugeElement::ToTag (strTag);

	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteInt (s_szTag_Style, (int)m_Style, (int)CBCGPColorIndicatorImpl::BCGP_COLOR_INDICATOR_ELLIPSE));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBool (s_szTag_Stretched, m_bStretched, FALSE));
}

BOOL CBCGPVisualInfo::XElementColor::ColorsFromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	tm.ReadBrush (s_szTag_Fill_Brush, m_Colors.m_brFill);
	tm.ReadBrush (s_szTag_Fill_Outline, m_Colors.m_brOutline);

	return TRUE;
}

void CBCGPVisualInfo::XElementColor::ColorsToTag (CString& strTag) const
{
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Fill_Brush, m_Colors.m_brFill));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Fill_Outline, m_Colors.m_brOutline));
}

BOOL CBCGPVisualInfo::XElementText::FromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	tm.ReadEntityString (s_szTag_DataValue, m_strText);
	tm.ReadTextFormat (s_szTag_TextFormat, m_fmtText);

	return XStaticGaugeElement::FromTag (tm.GetBuffer ());
}

void CBCGPVisualInfo::XElementText::ToTag (CString& strTag) const
{
	XStaticGaugeElement::ToTag (strTag);

	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteEntityString (s_szTag_DataValue, m_strText));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteTextFormat (s_szTag_TextFormat, m_fmtText));
}

BOOL CBCGPVisualInfo::XElementText::ColorsFromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	tm.ReadBrush (s_szTag_Text_Brush, m_brText);

	return TRUE;
}

void CBCGPVisualInfo::XElementText::ColorsToTag (CString& strTag) const
{
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Text_Brush, m_brText));
}

BOOL CBCGPVisualInfo::XElementImage::FromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	CString strImage;
	if (tm.ExcludeTag (s_szTag_Image, strImage))
	{
		m_Image.FromTag (strImage);
	}

	int align = (int)CBCGPImageGaugeImpl::HA_Left;
	tm.ReadInt (s_szTag_AlignHorz, align);
	m_AlignHorz = (CBCGPImageGaugeImpl::HorizontalAlign)bcg_clamp(align, (int)CBCGPImageGaugeImpl::HA_First, (int)CBCGPImageGaugeImpl::HA_Last);

	align = (int)CBCGPImageGaugeImpl::VA_Top;
	tm.ReadInt (s_szTag_AlignVert, align);
	m_AlignVert = (CBCGPImageGaugeImpl::VerticalAlign)bcg_clamp(align, (int)CBCGPImageGaugeImpl::VA_First, (int)CBCGPImageGaugeImpl::VA_Last);

	tm.ReadBool (s_szTag_LockAspectRatio, m_bLockAspectRatio);

	return XStaticGaugeElement::FromTag (tm.GetBuffer ());
}

void CBCGPVisualInfo::XElementImage::ToTag (CString& strTag) const
{
	XStaticGaugeElement::ToTag (strTag);

	CString strImage;
	m_Image.ToTag (strImage);
	CBCGPTagManager::WriteItem (strTag, s_szTag_Image, strImage);

	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteInt (s_szTag_AlignHorz, (int)m_AlignHorz, (int)CBCGPImageGaugeImpl::HA_Left));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteInt (s_szTag_AlignVert, (int)m_AlignVert, (int)CBCGPImageGaugeImpl::VA_Top));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBool (s_szTag_LockAspectRatio, m_bLockAspectRatio, TRUE));
}

BOOL CBCGPVisualInfo::XElementImage::ColorsFromTag (const CString& /*strTag*/)
{
	return TRUE;
}

void CBCGPVisualInfo::XElementImage::ColorsToTag (CString& /*strTag*/) const
{
}

BOOL CBCGPVisualInfo::XElementSwitch::FromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	int style = (int)CBCGPSwitchImpl::BCGP_SWITCH_RECTANGLE;
	tm.ReadInt (s_szTag_Style, style);
	m_Style = (CBCGPSwitchImpl::BCGP_SWITCH_STYLE)bcg_clamp(style, (int)CBCGPSwitchImpl::BCGP_SWITCH_STYLE_FIRST, (int)CBCGPSwitchImpl::BCGP_SWITCH_STYLE_LAST);

	tm.ReadTextFormat (s_szTag_TextFormat, m_fmtText);
	tm.ReadEntityString (s_szTag_LabelOff, m_strLabelOff);
	tm.ReadEntityString (s_szTag_LabelOn, m_strLabelOn);
	tm.ReadBool (s_szTag_DrawLabel, m_bDrawTextLabels);
	tm.ReadBool (s_szTag_DataValue, m_bValue);

	return XStaticGaugeElement::FromTag (tm.GetBuffer ());
}

void CBCGPVisualInfo::XElementSwitch::ToTag (CString& strTag) const
{
	XStaticGaugeElement::ToTag (strTag);

	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteInt (s_szTag_Style, (int)m_Style, (int)CBCGPSwitchImpl::BCGP_SWITCH_RECTANGLE));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteTextFormat (s_szTag_TextFormat, m_fmtText));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteEntityString (s_szTag_LabelOff, m_strLabelOff, CString(_T("Off")), TRUE));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteEntityString (s_szTag_LabelOn, m_strLabelOn, CString(_T("On")), TRUE));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBool (s_szTag_DrawLabel, m_bDrawTextLabels, FALSE));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBool (s_szTag_DataValue, m_bValue, FALSE));
}

BOOL CBCGPVisualInfo::XElementSwitch::ColorsFromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	tm.ReadBrush (s_szTag_Fill_Brush, m_Colors.m_brFill);
	tm.ReadBrush (s_szTag_Fill_Outline, m_Colors.m_brOutline);
	tm.ReadBrush (s_szTag_Thumb_Brush, m_Colors.m_brFillThumb);
	tm.ReadBrush (s_szTag_Thumb_Outline, m_Colors.m_brOutlineThumb);
	tm.ReadBrush (s_szTag_FillOff_Brush, m_Colors.m_brFillOff);
	tm.ReadBrush (s_szTag_FillOn_Brush, m_Colors.m_brFillOn);
	tm.ReadBrush (s_szTag_LabelOff_Brush, m_Colors.m_brLabelOff);
	tm.ReadBrush (s_szTag_LabelOn_Brush, m_Colors.m_brLabelOn);
	tm.ReadBrush (s_szTag_Focus_Brush, m_Colors.m_brFocus);

	return TRUE;
}

void CBCGPVisualInfo::XElementSwitch::ColorsToTag (CString& strTag) const
{
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Fill_Brush, m_Colors.m_brFill));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Fill_Outline, m_Colors.m_brOutline));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Thumb_Brush, m_Colors.m_brFillThumb));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Thumb_Outline, m_Colors.m_brOutlineThumb));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_FillOff_Brush, m_Colors.m_brFillOff));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_FillOn_Brush, m_Colors.m_brFillOn));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_LabelOff_Brush, m_Colors.m_brLabelOff));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_LabelOn_Brush, m_Colors.m_brLabelOn));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Focus_Brush, m_Colors.m_brFocus));
}

BOOL CBCGPVisualInfo::XElementTagCloud::FromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	CString strDataObjects;
	if (tm.ExcludeTag (s_szTag_DataObjects, strDataObjects))
	{
		CBCGPTagManager tmDataObjects (strDataObjects);

		CString strDataObject;
		while (tmDataObjects.ExcludeTag (s_szTag_DataObject, strDataObject))
		{
			XTagCloudData* pDataObject = new XTagCloudData;
			if (pDataObject == NULL)
			{
				continue;
			}

			if (!pDataObject->FromTag (strDataObject))
			{
				delete pDataObject;
				continue;
			}

			m_arDataObjects.Add (pDataObject);
		}
	}

	int sortOrder = (int)CBCGPTagCloud::NoSort;
	tm.ReadInt (s_szTag_SortOrder, sortOrder);
	m_SortOrder = (CBCGPTagCloud::SortOrder)bcg_clamp(sortOrder, (int)CBCGPTagCloud::SortOrder_First, (int)CBCGPTagCloud::SortOrder_Last);

	tm.ReadBool (s_szTag_SortDescending, m_bDescending);
	tm.ReadInt (s_szTag_MaxWeight, m_nMaxWeight);
	tm.ReadDouble (s_szTag_FontSizeStep, m_dblFontSizeStep);
	tm.ReadDouble (s_szTag_MarginHorz, m_szMargin.cx);
	tm.ReadDouble (s_szTag_MarginVert, m_szMargin.cy);

	tm.ReadTextFormat (s_szTag_TextFormat, m_fmtBase);
	tm.ReadBrush (s_szTag_Fill_Brush, m_brFill);
	tm.ReadColor (s_szTag_Text_Brush, m_clrText);
	tm.ReadColor (s_szTag_Text_Highlighted_Brush, m_clrTextHighlighted);

	return XElement::FromTag (tm.GetBuffer ());
}

void CBCGPVisualInfo::XElementTagCloud::ToTag (CString& strTag) const
{
	XElement::ToTag (strTag);

	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteInt (s_szTag_SortOrder, (int)m_SortOrder, (int)CBCGPTagCloud::NoSort));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBool (s_szTag_SortDescending, m_bDescending, FALSE));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteInt (s_szTag_MaxWeight, m_nMaxWeight, 5));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteDouble (s_szTag_FontSizeStep, m_dblFontSizeStep, 0.0));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteDouble (s_szTag_MarginHorz, m_szMargin.cx, 5.0));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteDouble (s_szTag_MarginVert, m_szMargin.cy, 5.0));

	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteTextFormat (s_szTag_TextFormat, m_fmtBase));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Fill_Brush, m_brFill));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteColor (s_szTag_Text_Brush, m_clrText, CBCGPColor(CBCGPColor::Black)));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteColor (s_szTag_Text_Highlighted_Brush, m_clrTextHighlighted, CBCGPColor(CBCGPColor::SteelBlue)));

	CString strDataObjects;
	for (int i = 0; i < (int)m_arDataObjects.GetSize (); i++)
	{
		CString strItem;
		m_arDataObjects[i]->ToTag (strItem);
		CBCGPTagManager::WriteItem (strDataObjects, s_szTag_DataObject, strItem);
	}
	CBCGPTagManager::WriteItem (strTag, s_szTag_DataObjects, strDataObjects);
}

BOOL CBCGPVisualInfo::XElementTreeMap::FromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	CString strRoot;
	if (tm.ExcludeTag(s_szTag_Root, strRoot))
	{
		m_Root.FromTag (strRoot);
	}

	int layoutType = (int)CBCGPTreeMap::Squarified;
	tm.ReadInt (s_szTag_LayoutType, layoutType);
	m_LayoutType = (CBCGPTreeMap::LayoutType)bcg_clamp(layoutType, (int)CBCGPTreeMap::LayoutType_First, (int)CBCGPTreeMap::LayoutType_Last);

	return XElement::FromTag (tm.GetBuffer ());
}

void CBCGPVisualInfo::XElementTreeMap::ToTag (CString& strTag) const
{
	XElement::ToTag (strTag);

	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteInt (s_szTag_LayoutType, (int)m_LayoutType, (int)CBCGPTreeMap::Squarified));
	
	CString strRoot;
	m_Root.ToTag (strRoot);
	CBCGPTagManager::WriteItem (strTag, s_szTag_Root, strRoot);
}

BOOL CBCGPVisualInfo::XElementWinUITiles::FromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	CString strCaptionButtons;
	if (tm.ExcludeTag (s_szTag_CaptionButtons, strCaptionButtons))
	{
		CBCGPTagManager tmCaptionButtons (strCaptionButtons);

		CString strCaptionButton;
		while (tmCaptionButtons.ExcludeTag (s_szTag_CaptionButton, strCaptionButton))
		{
			XWinUICaptionButton* pCaptionButton = new XWinUICaptionButton;
			if (pCaptionButton == NULL)
			{
				continue;
			}

			if (!pCaptionButton->FromTag (strCaptionButton))
			{
				delete pCaptionButton;
				continue;
			}

			m_arCaptionButtons.Add (pCaptionButton);
		}
	}

	CString strGroupCaptions;
	if (tm.ExcludeTag (s_szTag_GroupCaptions, strGroupCaptions))
	{
		CBCGPTagManager tmGroupCaptions (strGroupCaptions);

		CString strGroupCaption;
		while (tmGroupCaptions.ExcludeTag (s_szTag_GroupCaption, strGroupCaption))
		{
			XWinUIGroupCaption* pGroupCaption = new XWinUIGroupCaption;
			if (pGroupCaption == NULL)
			{
				continue;
			}

			if (!pGroupCaption->FromTag (strGroupCaption))
			{
				delete pGroupCaption;
				continue;
			}

			m_arGroupCaptions.Add (pGroupCaption);
		}
	}

	CString strTiles;
	if (tm.ExcludeTag (s_szTag_Tiles, strTiles))
	{
		CBCGPTagManager tmTiles (strTiles);

		CString strTile;
		while (tmTiles.ExcludeTag (s_szTag_Tile, strTile))
		{
			XWinUITile* pTile = new XWinUITile;
			if (pTile == NULL)
			{
				continue;
			}

			if (!pTile->FromTag (strTile))
			{
				delete pTile;
				continue;
			}

			m_arTiles.Add (pTile);
		}
	}

	tm.ReadBool (s_szTag_HorizontalLayout, m_bIsHorizontalLayout);
	tm.ReadBool (s_szTag_RoundedShapes, m_bRoundedShapes);
	tm.ReadDouble (s_szTag_MarginHorz, m_szMargin.cx);
	tm.ReadDouble (s_szTag_MarginVert, m_szMargin.cy);
	tm.ReadDouble (s_szTag_SquareWidth, m_szSquare.cx);
	tm.ReadDouble (s_szTag_SquareHeight, m_szSquare.cy);
	tm.ReadEntityString (s_szTag_Caption, m_strCaption);
	tm.ReadDouble (s_szTag_CaptionHeight, m_dblCaptionExtraHeight);
	tm.ReadBrush (s_szTag_Fill_Brush, m_brFill);
	tm.ReadColor (s_szTag_Caption_Brush, m_colorCaptionForeground);
	tm.ReadBool (s_szTag_Tiles_DragAndDrop, m_bTilesDragAndDrop);

	CString strImage;
	if (tm.ExcludeTag (s_szTag_Badge_Custom_Image, strImage))
	{
		m_CustomBadgeGlyphs.FromTag (strImage);
	}

	return XElement::FromTag (tm.GetBuffer ());
}

void CBCGPVisualInfo::XElementWinUITiles::ToTag (CString& strTag) const
{
	XElement::ToTag (strTag);

	CBCGPSize szSquare(GetDefaultSquareSize ());

	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBool (s_szTag_HorizontalLayout, m_bIsHorizontalLayout, TRUE));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBool (s_szTag_RoundedShapes, m_bRoundedShapes, FALSE));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteDouble (s_szTag_MarginHorz, m_szMargin.cx, 10.0));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteDouble (s_szTag_MarginVert, m_szMargin.cy, 10.0));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteDouble (s_szTag_SquareWidth, m_szSquare.cx, szSquare.cx));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteDouble (s_szTag_SquareHeight, m_szSquare.cy, szSquare.cy));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteEntityString (s_szTag_Caption, m_strCaption));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteDouble (s_szTag_CaptionHeight, m_dblCaptionExtraHeight, 0.0));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Fill_Brush, m_brFill));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteColor (s_szTag_Caption_Brush, m_colorCaptionForeground, CBCGPColor(CBCGPColor::White)));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBool (s_szTag_Tiles_DragAndDrop, m_bTilesDragAndDrop, FALSE));

	CString strImage;
	m_CustomBadgeGlyphs.ToTag (strImage);
	CBCGPTagManager::WriteItem (strTag, s_szTag_Badge_Custom_Image, strImage);

	int i = 0;

	CString strCaptionButtons;
	for (i = 0; i < (int)m_arCaptionButtons.GetSize (); i++)
	{
		CString strItem;
		m_arCaptionButtons[i]->ToTag (strItem);
		CBCGPTagManager::WriteItem (strCaptionButtons, s_szTag_CaptionButton, strItem);
	}
	CBCGPTagManager::WriteItem (strTag, s_szTag_CaptionButtons, strCaptionButtons);

	CString strGroupCaptions;
	for (i = 0; i < (int)m_arGroupCaptions.GetSize (); i++)
	{
		CString strItem;
		m_arGroupCaptions[i]->ToTag (strItem);
		CBCGPTagManager::WriteItem (strGroupCaptions, s_szTag_GroupCaption, strItem);
	}
	CBCGPTagManager::WriteItem (strTag, s_szTag_GroupCaptions, strGroupCaptions);

	CString strTiles;
	for (i = 0; i < (int)m_arTiles.GetSize (); i++)
	{
		CString strItem;
		m_arTiles[i]->ToTag (strItem);
		CBCGPTagManager::WriteItem (strTiles, s_szTag_Tile, strItem);
	}
	CBCGPTagManager::WriteItem (strTag, s_szTag_Tiles, strTiles);
}

BOOL CBCGPVisualInfo::XElementDiagramConnectorShelf::FromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	tm.ReadDouble (s_szTag_Offset_Shelf, m_dShelfOffset);
	m_dShelfOffset = max(m_dShelfOffset, 0.0);

	return XDiagramElementConnector::FromTag (tm.GetBuffer ());
}

void CBCGPVisualInfo::XElementDiagramConnectorShelf::ToTag (CString& strTag) const
{
	XDiagramElementConnector::ToTag (strTag);

	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteDouble (s_szTag_Offset_Shelf, m_dShelfOffset, 10.0));
}

BOOL CBCGPVisualInfo::XElementDiagramConnectorElbow::FromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	int orient = CBCGPDiagramElbowConnector::Auto;
	tm.ReadInt (s_szTag_Orientation, orient);
	m_Orientation = (CBCGPDiagramElbowConnector::Orientation)bcg_clamp(orient, 
		(int)CBCGPDiagramElbowConnector::Orientation_First, (int)CBCGPDiagramElbowConnector::Orientation_Last);

	CPoint pt(0, 0);
	tm.ReadPoint (s_szTag_ResizeHandle, pt);
	m_ptResizeHandle = pt;

	return XDiagramElementConnector::FromTag (tm.GetBuffer ());
}

void CBCGPVisualInfo::XElementDiagramConnectorElbow::ToTag (CString& strTag) const
{
	XDiagramElementConnector::ToTag (strTag);

	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteInt (s_szTag_Orientation, (int)m_Orientation, CBCGPDiagramElbowConnector::Auto));

	CPoint pt(m_ptResizeHandle);
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WritePoint (s_szTag_ResizeHandle, pt, CPoint(0, 0)));
}

BOOL CBCGPVisualInfo::XElementDiagramShape::FromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	int shape = CBCGPDiagramShape::Box;
	tm.ReadInt (s_szTag_Shape_Type, shape);
	m_shape = (CBCGPDiagramShape::Shape)bcg_clamp(shape, 
		(int)CBCGPDiagramShape::FirstShape, (int)CBCGPDiagramShape::LastShape);

	return XDiagramElement::FromTag (tm.GetBuffer ());
}

void CBCGPVisualInfo::XElementDiagramShape::ToTag (CString& strTag) const
{
	XDiagramElement::ToTag (strTag);

	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteInt (s_szTag_Shape_Type, (int)m_shape, CBCGPDiagramShape::Box));
}

BOOL CBCGPVisualInfo::XElementDiagramTable::FromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	tm.ReadBool (s_szTag_Caption, m_bCaption);
	tm.ReadBrush (s_szTag_Caption_Brush, m_brCaptionFill);

	CString strDataCaption;
	while (tm.ExcludeTag (s_szTag_DataCaption, strDataCaption))
	{
		m_CaptionData.FromTag (strDataCaption);
	}

	return XElementDiagramShape::FromTag (tm.GetBuffer ());
}

void CBCGPVisualInfo::XElementDiagramTable::ToTag (CString& strTag) const
{
	XElementDiagramShape::ToTag (strTag);

	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBool (s_szTag_Caption, m_bCaption, FALSE));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Caption_Brush, m_brCaptionFill));

	CString strDataCaption;
	m_CaptionData.ToTag (strDataCaption);
	CBCGPTagManager::WriteItem (strTag, s_szTag_DataCaption, strDataCaption, TRUE);
}

BOOL CBCGPVisualInfo::XElementDiagramImage::FromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	CString strImage;
	if (tm.ExcludeTag (s_szTag_Image, strImage))
	{
		m_Image.FromTag (strImage);
	}

	int align = (int)CBCGPDiagramImageObject::HA_Left;
	tm.ReadInt (s_szTag_AlignHorz, align);
	m_AlignHorz = (CBCGPDiagramImageObject::HorizontalAlign)bcg_clamp(align, (int)CBCGPDiagramImageObject::HA_First, (int)CBCGPDiagramImageObject::HA_Last);

	align = (int)CBCGPDiagramImageObject::VA_Top;
	tm.ReadInt (s_szTag_AlignVert, align);
	m_AlignVert = (CBCGPDiagramImageObject::VerticalAlign)bcg_clamp(align, (int)CBCGPDiagramImageObject::VA_First, (int)CBCGPDiagramImageObject::VA_Last);

	tm.ReadBool (s_szTag_LockAspectRatio, m_bLockAspectRatio);

	return XDiagramElement::FromTag (tm.GetBuffer ());
}

void CBCGPVisualInfo::XElementDiagramImage::ToTag (CString& strTag) const
{
	XDiagramElement::ToTag (strTag);

	CString strImage;
	m_Image.ToTag (strImage);
	CBCGPTagManager::WriteItem (strTag, s_szTag_Image, strImage);

	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteInt (s_szTag_AlignHorz, (int)m_AlignHorz, (int)CBCGPDiagramImageObject::HA_Left));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteInt (s_szTag_AlignVert, (int)m_AlignVert, (int)CBCGPDiagramImageObject::VA_Top));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBool (s_szTag_LockAspectRatio, m_bLockAspectRatio, TRUE));
}


CBCGPVisualInfo::CBCGPVisualInfo()
	: CBCGPBaseInfo(c_dwVersion, 0)
{
}

CBCGPVisualInfo::~CBCGPVisualInfo()
{
}

BOOL CBCGPVisualInfo::FromTag (const CString& strTag)
{
	CString strXML;
	{
		CBCGPTagManager tmContainer (strTag);

		if (!tmContainer.ExcludeTag (s_szTag_Body, strXML))
		{
			return FALSE;
		}
	}

	CBCGPTagManager tm (strXML);

	CString strHeader;
	if (tm.ExcludeTag (CBCGPBaseInfo::s_szTag_Header, strHeader))
	{
		CBCGPTagManager tmHeader (strHeader);

		DWORD dwValue = GetVersion ();
		tmHeader.CBCGPTagManager::ReadDword (CBCGPBaseInfo::s_szTag_Version, dwValue);
		SetVersion (dwValue);
	}
	else
	{
		return FALSE;
	}

	CString strContainer;
	if (tm.ExcludeTag (s_szTag_Container, strContainer))
	{
		GetContainer().FromTag (strContainer);
	}

	return TRUE;
}

void CBCGPVisualInfo::ToTag (CString& strTag) const
{
	CString strData;

	CString strHeader;
	CBCGPTagManager::WriteTag (strHeader, CBCGPTagManager::WriteDword (CBCGPBaseInfo::s_szTag_Version, GetVersion (), 0));
	CBCGPTagManager::WriteItem (strData, CBCGPBaseInfo::s_szTag_Header, strHeader);

	CString strContainer;
	GetContainer().ToTag (strContainer);
	CBCGPTagManager::WriteItem (strData, s_szTag_Container, strContainer);

	CBCGPTagManager::WriteItem (strTag, s_szTag_Body, strData);
}


CBCGPVisualInfoLoader::CBCGPVisualInfoLoader (CBCGPVisualInfo& info)
	: CBCGPBaseInfoLoader(info, _T("BCGP_VISUAL_XML"), 0)
{
}

CBCGPVisualInfoLoader::~CBCGPVisualInfoLoader()
{
}

CBCGPVisualInfoWriter::CBCGPVisualInfoWriter(CBCGPVisualInfo& info)
	: CBCGPBaseInfoWriter(info)
{
}

CBCGPVisualInfoWriter::~CBCGPVisualInfoWriter()
{
}

BOOL CBCGPVisualInfoWriter::Save (const CString& strFileName)
{
	if (strFileName.IsEmpty ())
	{
		ASSERT(FALSE);
		return FALSE;
	}

	CStringArray sa;
	sa.Add (strFileName);

	if (!CheckFiles (sa))
	{
		return FALSE;
	}

	return SaveInfo (strFileName);
}
