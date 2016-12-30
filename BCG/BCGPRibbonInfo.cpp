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
// BCGPRibbonInfo.cpp: implementation of the CBCGPBaseRibbonInfo class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BCGPRibbonInfo.h"
#include "BCGPTagManager.h"
#include "BCGPCalculator.h"
#include "BCGPMath.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#ifndef BCGP_EXCLUDE_RIBBON

static const DWORD c_dwVersion = MAKELONG (1, 0);

static LPCTSTR s_szTag_Text                   = _T("TEXT");
static LPCTSTR s_szTag_ToolTip                = _T("TOOLTIP");
static LPCTSTR s_szTag_Description            = _T("DESCRIPTION");
static LPCTSTR s_szTag_Keys                   = _T("KEYS");
static LPCTSTR s_szTag_MenuKeys               = _T("KEYS_MENU");
static LPCTSTR s_szTag_PaletteTop             = _T("PALETTE_TOP");
static LPCTSTR s_szTag_AlwaysLarge            = _T("ALWAYS_LARGE");
static LPCTSTR s_szTag_AlwaysShowDescription  = _T("ALWAYS_DESCRIPTION");
static LPCTSTR s_szTag_QATType                = _T("QAT_TYPE");

static LPCTSTR s_szTag_Index                  = _T("INDEX");
static LPCTSTR s_szTag_IndexSmall             = _T("INDEX_SMALL");
static LPCTSTR s_szTag_IndexLarge             = _T("INDEX_LARGE");
static LPCTSTR s_szTag_DefaultCommand         = _T("DEFAULT_COMMAND");
static LPCTSTR s_szTag_Link                   = _T("LINK");
static LPCTSTR s_szTag_Width                  = _T("WIDTH");
static LPCTSTR s_szTag_Height                 = _T("HEIGHT");
static LPCTSTR s_szTag_WidthFloaty            = _T("WIDTH_FLOATY");
static LPCTSTR s_szTag_SpinButtons            = _T("SPIN_BUTTONS");
static LPCTSTR s_szTag_Min                    = _T("MIN");
static LPCTSTR s_szTag_Max                    = _T("MAX");
static LPCTSTR s_szTag_SearchMode             = _T("SEARCH_MODE");
static LPCTSTR s_szTag_SearchPrompt           = _T("SEARCH_PROMPT");
static LPCTSTR s_szTag_CalculatorMode         = _T("CALCULATOR_MODE");
static LPCTSTR s_szTag_CalculatorCmdExt       = _T("CALCULATOR_CMD_EXT");

static LPCTSTR s_szTag_EditBox                = _T("EDIT_BOX");
static LPCTSTR s_szTag_DropDownList           = _T("DROPDOWN_LIST");
static LPCTSTR s_szTag_ResizeDropDownList     = _T("DROPDOWN_LIST_RESIZE");
static LPCTSTR s_szTag_AutoComplete           = _T("AUTO_COMPLETE");

static LPCTSTR s_szTag_FontType               = _T("FONT_TYPE");
static LPCTSTR s_szTag_CharSet                = _T("CHAR_SET");
static LPCTSTR s_szTag_PitchAndFamily         = _T("PITCH_AND_FAMILY");

static LPCTSTR s_szTag_ButtonMode             = _T("BUTTON_MODE");
static LPCTSTR s_szTag_MenuResize             = _T("MENU_RESIZE");
static LPCTSTR s_szTag_MenuResizeVertical     = _T("MENU_RESIZE_VERTICAL");
static LPCTSTR s_szTag_DrawDisabledItems      = _T("DRAW_DISABLED_ITEMS");
static LPCTSTR s_szTag_IconsInRow             = _T("ICONS_IN_ROW");
static LPCTSTR s_szTag_IconWidth              = _T("ICON_WIDTH");
static LPCTSTR s_szTag_SizeIcon               = _T("SIZE_ICON");

static LPCTSTR s_szTag_Color                  = _T("COLOR");
static LPCTSTR s_szTag_SizeBox                = _T("SIZE_BOX");
static LPCTSTR s_szTag_SimpleButtonLook       = _T("SIMPLE_LOOK");
static LPCTSTR s_szTag_AutomaticColorBtn      = _T("AUTOMATIC_BTN");
static LPCTSTR s_szTag_OtherColorBtn          = _T("OTHER_BTN");
static LPCTSTR s_szTag_Border                 = _T("BORDER");

static LPCTSTR s_szTag_Style                  = _T("STYLE");
static LPCTSTR s_szTag_Pos                    = _T("POS");
static LPCTSTR s_szTag_ZoomButtons            = _T("ZOOM_BUTTONS");

static LPCTSTR s_szTag_Horiz                  = _T("HORIZ");

static LPCTSTR s_szTag_AlmostLargeText        = _T("ALMOST_LARGE_TEXT");
static LPCTSTR s_szTag_Static                 = _T("STATIC");
static LPCTSTR s_szTag_TextAlign              = _T("TEXT_ALIGN");

static LPCTSTR s_szTag_QATTop                 = _T("QAT_TOP");
static LPCTSTR s_szTag_JustifyColumns         = _T("JUSTIFY_COLUMNS");
static LPCTSTR s_szTag_CenterColumnVert       = _T("CENTER_COLUMN_VERT");

static LPCTSTR s_szTag_Enable                 = _T("ENABLE");
static LPCTSTR s_szTag_EnableToolTips         = _T("ENABLE_TOOLTIPS");
static LPCTSTR s_szTag_EnableToolTipsDescr    = _T("ENABLE_TOOLTIPS_DESCRIPTION");
static LPCTSTR s_szTag_EnableKeys             = _T("ENABLE_KEYS");
static LPCTSTR s_szTag_EnablePrintPreview     = _T("ENABLE_PRINTPREVIEW");
static LPCTSTR s_szTag_DrawUsingFont          = _T("ENABLE_DRAWUSINGFONT");
static LPCTSTR s_szTag_EnableBackstageMode    = _T("ENABLE_BACKSTAGEMODE");

static LPCTSTR s_szTag_Label                  = _T("LABEL");
static LPCTSTR s_szTag_Visible                = _T("VISIBLE");
static LPCTSTR s_szTag_Infinite               = _T("INFINITE");
static LPCTSTR s_szTag_Vertical               = _T("VERTICAL");

static LPCTSTR s_szTag_RecentFileList         = _T("RECENT_FILE_LIST");
static LPCTSTR s_szTag_Search                 = _T("SEARCH_COMMAND");
static LPCTSTR s_szTag_ShowPins               = _T("SHOW_PINS");

static LPCTSTR s_szTag_MenuMode               = _T("MENU_MODE");

static LPCTSTR s_szTag_Element                = _T("ELEMENT");
static LPCTSTR s_szTag_Elements               = _T("ELEMENTS");
static LPCTSTR s_szTag_ElementsExtended       = _T("ELEMENTS_EXTENDED");
static LPCTSTR s_szTag_Item                   = _T("ITEM");
static LPCTSTR s_szTag_Items                  = _T("ITEMS");
static LPCTSTR s_szTag_Panel                  = _T("PANEL");
static LPCTSTR s_szTag_Panels                 = _T("PANELS");
static LPCTSTR s_szTag_Category               = _T("CATEGORY");
static LPCTSTR s_szTag_Categories             = _T("CATEGORIES");
static LPCTSTR s_szTag_Context                = _T("CONTEXT");
static LPCTSTR s_szTag_Contexts               = _T("CONTEXTS");
static LPCTSTR s_szTag_Group                  = _T("GROUP");
static LPCTSTR s_szTag_Groups                 = _T("GROUPS");
static LPCTSTR s_szTag_RibbonBar              = _T("RIBBON_BAR");
static LPCTSTR s_szTag_StatusBar              = _T("STATUS_BAR");
static LPCTSTR s_szTag_Button_Main            = _T("BUTTON_MAIN");
static LPCTSTR s_szTag_QAT_Elements           = _T("QAT_ELEMENTS");
static LPCTSTR s_szTag_Tab_Elements           = _T("TAB_ELEMENTS");
static LPCTSTR s_szTag_Button_Launch          = _T("BUTTON_LAUNCH");
static LPCTSTR s_szTag_CategoryMain           = _T("CATEGORY_MAIN");
static LPCTSTR s_szTag_CategoryBackstage      = _T("CATEGORY_BACKSTAGE");
static LPCTSTR s_szTag_CollapseOrder          = _T("COLLAPSE_ORDER");
static LPCTSTR s_szTag_SortOrder              = _T("SORT_ORDER");

static LPCTSTR s_szTag_Body                   = _T("BCGP_RIBBON");
static LPCTSTR s_szTag_Sizes                  = _T("SIZES");

static LPCTSTR s_szTag_Image                  = _T("IMAGE");
static LPCTSTR s_szTag_Image_Scenic           = _T("IMAGE_SCENIC");
//static LPCTSTR s_szTag_Image_Hot              = _T("IMAGE_HOT");
//static LPCTSTR s_szTag_Image_Disabled         = _T("IMAGE_DISABLED");
static LPCTSTR s_szTag_Image_Small            = _T("IMAGE_SMALL");
static LPCTSTR s_szTag_Image_Large            = _T("IMAGE_LARGE");
static LPCTSTR s_szTag_Image_SBGroup          = _T("IMAGE_SBGROUP");

static LPCTSTR s_szTag_Body_Customization     = _T("BCGP_RIBBON_CUSTOMIZATION");
static LPCTSTR s_szTag_Data_Category          = _T("DATA_CATEGORY");
static LPCTSTR s_szTag_Data_Panel             = _T("DATA_PANEL");
static LPCTSTR s_szTag_Hidden                 = _T("HIDDEN");
static LPCTSTR s_szTag_Names                  = _T("NAMES");
static LPCTSTR s_szTag_Key                    = _T("KEY");
static LPCTSTR s_szTag_KeyMap                 = _T("KEY_MAP");
static LPCTSTR s_szTag_Custom                 = _T("CUSTOM");
static LPCTSTR s_szTag_Original               = _T("ORIGINAL");


LPCTSTR CBCGPBaseRibbonInfo::s_szButton            = _T("Button");
LPCTSTR CBCGPBaseRibbonInfo::s_szButton_Check      = _T("Button_Check");
LPCTSTR CBCGPBaseRibbonInfo::s_szButton_Radio      = _T("Button_Radio");
LPCTSTR CBCGPBaseRibbonInfo::s_szButton_Color      = _T("Button_Color");
LPCTSTR CBCGPBaseRibbonInfo::s_szButton_Undo       = _T("Button_Undo");
LPCTSTR CBCGPBaseRibbonInfo::s_szButton_Palette    = _T("Button_Palette");
LPCTSTR CBCGPBaseRibbonInfo::s_szButton_Hyperlink  = _T("Button_Hyperlink");
LPCTSTR CBCGPBaseRibbonInfo::s_szButton_Main       = _T("Button_Main");
LPCTSTR CBCGPBaseRibbonInfo::s_szButton_MainPanel  = _T("Button_Main_Panel");
LPCTSTR CBCGPBaseRibbonInfo::s_szButton_Command    = _T("Button_Command");
LPCTSTR CBCGPBaseRibbonInfo::s_szButton_Launch     = _T("Button_Launch");
LPCTSTR CBCGPBaseRibbonInfo::s_szLabel             = _T("Label");
LPCTSTR CBCGPBaseRibbonInfo::s_szEdit              = _T("Edit");
LPCTSTR CBCGPBaseRibbonInfo::s_szComboBox          = _T("ComboBox");
LPCTSTR CBCGPBaseRibbonInfo::s_szComboBox_Font     = _T("ComboBox_Font");
LPCTSTR CBCGPBaseRibbonInfo::s_szSlider            = _T("Slider");
LPCTSTR CBCGPBaseRibbonInfo::s_szProgress          = _T("Progress");
LPCTSTR CBCGPBaseRibbonInfo::s_szSeparator         = _T("Separator");
LPCTSTR CBCGPBaseRibbonInfo::s_szGroup             = _T("Group");
LPCTSTR CBCGPBaseRibbonInfo::s_szStatusPane        = _T("StatusPane");
LPCTSTR CBCGPBaseRibbonInfo::s_szPanel             = _T("Panel");
LPCTSTR CBCGPBaseRibbonInfo::s_szCategory          = _T("Category");
LPCTSTR CBCGPBaseRibbonInfo::s_szContext           = _T("Context");
LPCTSTR CBCGPBaseRibbonInfo::s_szCategoryMain      = _T("Category_Main");
LPCTSTR CBCGPBaseRibbonInfo::s_szCategoryBackstage = _T("Category_Backstage");
LPCTSTR CBCGPBaseRibbonInfo::s_szQAT               = _T("QAT");
LPCTSTR CBCGPBaseRibbonInfo::s_szRibbonBar         = _T("RibbonBar");
LPCTSTR CBCGPBaseRibbonInfo::s_szStatusBar         = _T("StatusBar");

CBCGPBaseRibbonInfo::XElement::XElement(const CString& strElementName)
	: CBCGPBaseInfo::XBase (strElementName)
	, m_bIsOnPaletteTop    (FALSE)
	, m_bIsAlwaysLarge     (FALSE)
{
}

CBCGPBaseRibbonInfo::XElement::~XElement()
{
}

CBCGPBaseRibbonInfo::XElementSeparator::XElementSeparator()
	: CBCGPBaseRibbonInfo::XElement (CBCGPBaseRibbonInfo::s_szSeparator)
	, m_bIsHoriz                (FALSE)
{
}

CBCGPBaseRibbonInfo::XElementSeparator::~XElementSeparator()
{
}

CBCGPBaseRibbonInfo::XElementGroup::XElementGroup()
	: CBCGPBaseRibbonInfo::XElement (CBCGPBaseRibbonInfo::s_szGroup)
{
}

CBCGPBaseRibbonInfo::XElementGroup::~XElementGroup()
{
	int i;
	for (i = 0; i < (int)m_arButtons.GetSize (); i++)
	{
		if (m_arButtons[i] != NULL)
		{
			delete m_arButtons[i];
		}
	}
}

CBCGPBaseRibbonInfo::XElementButton::XElementButton(const CString& strElementName)
	: CBCGPBaseRibbonInfo::XElement (strElementName)
	, m_nSmallImageIndex        (-1)
	, m_nLargeImageIndex        (-1)
	, m_bIsDefaultCommand       (TRUE)
	, m_QATType                 (CBCGPRibbonButton::BCGPRibbonButton_Show_Default)
{
}

CBCGPBaseRibbonInfo::XElementButton::XElementButton()
	: CBCGPBaseRibbonInfo::XElement (CBCGPBaseRibbonInfo::s_szButton)
	, m_nSmallImageIndex        (-1)
	, m_nLargeImageIndex        (-1)
	, m_bIsDefaultCommand       (TRUE)
	, m_bIsAlwaysShowDescription(FALSE)
	, m_QATType                 (CBCGPRibbonButton::BCGPRibbonButton_Show_Default)
{
}

CBCGPBaseRibbonInfo::XElementButton::~XElementButton()
{
	int i;
	for (i = 0; i < (int)m_arSubItems.GetSize (); i++)
	{
		if (m_arSubItems[i] != NULL)
		{
			delete m_arSubItems[i];
		}
	}
}

CBCGPBaseRibbonInfo::XElementLabel::XElementLabel()
	: CBCGPBaseRibbonInfo::XElementButton (CBCGPBaseRibbonInfo::s_szLabel)
{
}

CBCGPBaseRibbonInfo::XElementLabel::~XElementLabel()
{
}

CBCGPBaseRibbonInfo::XElementButtonCheck::XElementButtonCheck()
	: CBCGPBaseRibbonInfo::XElementButton (CBCGPBaseRibbonInfo::s_szButton_Check)
{
}

CBCGPBaseRibbonInfo::XElementButtonCheck::~XElementButtonCheck()
{
}

CBCGPBaseRibbonInfo::XElementButtonRadio::XElementButtonRadio()
	: XElementButton (CBCGPBaseRibbonInfo::s_szButton_Radio)
{
}

CBCGPBaseRibbonInfo::XElementButtonRadio::~XElementButtonRadio()
{
}

CBCGPBaseRibbonInfo::XElementButtonHyperlink::XElementButtonHyperlink()
	: CBCGPBaseRibbonInfo::XElementButton (CBCGPBaseRibbonInfo::s_szButton_Hyperlink)
{
}

CBCGPBaseRibbonInfo::XElementButtonHyperlink::~XElementButtonHyperlink()
{
}

CBCGPBaseRibbonInfo::XElementEdit::XElementEdit(const CString& strElementName)
	: CBCGPBaseRibbonInfo::XElementButton (strElementName)
	, m_nWidth                        (0)
	, m_nWidthFloaty                  (0)
	, m_nTextAlign                    (ES_LEFT)
	, m_bHasSpinButtons               (FALSE)
	, m_nMin                          (INT_MAX)
	, m_nMax                          (INT_MAX)
	, m_bSearchMode                   (FALSE)
{
}

CBCGPBaseRibbonInfo::XElementEdit::XElementEdit()
	: CBCGPBaseRibbonInfo::XElementButton (CBCGPBaseRibbonInfo::s_szEdit)
	, m_nWidth                        (0)
	, m_nWidthFloaty                  (0)
	, m_nTextAlign                    (ES_LEFT)
	, m_bHasSpinButtons               (FALSE)
	, m_nMin                          (INT_MAX)
	, m_nMax                          (INT_MAX)
	, m_bSearchMode                   (FALSE)
{
}

CBCGPBaseRibbonInfo::XElementEdit::~XElementEdit()
{
}

CBCGPBaseRibbonInfo::XElementComboBox::XElementComboBox(const CString& strElementName)
	: CBCGPBaseRibbonInfo::XElementEdit (strElementName)
	, m_bHasEditBox                 (FALSE)
	, m_bHasDropDownList            (TRUE)
	, m_bResizeDropDownList         (TRUE)
	, m_bCalculatorMode             (FALSE)
	, m_bAutoComplete               (FALSE)
{
	m_nWidth = 108;
}

CBCGPBaseRibbonInfo::XElementComboBox::XElementComboBox()
	: CBCGPBaseRibbonInfo::XElementEdit (CBCGPBaseRibbonInfo::s_szComboBox)
	, m_bHasEditBox                 (FALSE)
	, m_bHasDropDownList            (TRUE)
	, m_bResizeDropDownList         (TRUE)
	, m_bCalculatorMode             (FALSE)
	, m_bAutoComplete               (FALSE)
	, m_sortOrder                   (CBCGPRibbonComboBox::BCGP_RIBBON_COMBO_SORT_ORDER_NO_SORT)
{
	m_nWidth = 108;
}

CBCGPBaseRibbonInfo::XElementComboBox::~XElementComboBox()
{
}

CBCGPBaseRibbonInfo::XElementFontComboBox::XElementFontComboBox()
	: CBCGPBaseRibbonInfo::XElementComboBox (CBCGPBaseRibbonInfo::s_szComboBox_Font)
	, m_nFontType                       (DEVICE_FONTTYPE | RASTER_FONTTYPE | TRUETYPE_FONTTYPE)
	, m_nCharSet                        (DEFAULT_CHARSET)
	, m_nPitchAndFamily                 (DEFAULT_PITCH)
{
	m_bHasEditBox = TRUE;
}

CBCGPBaseRibbonInfo::XElementFontComboBox::~XElementFontComboBox()
{
}

CBCGPBaseRibbonInfo::XElementButtonPalette::XPaletteGroup::XPaletteGroup()
	: m_nItems (0)
{
}

CBCGPBaseRibbonInfo::XElementButtonPalette::XPaletteGroup::~XPaletteGroup()
{
}

CBCGPBaseRibbonInfo::XElementButtonPalette::XElementButtonPalette(const CString& strElementName)
	: CBCGPBaseRibbonInfo::XElementButton (strElementName)
	, m_bIsButtonMode                 (TRUE)
	, m_bEnableMenuResize             (FALSE)
	, m_bMenuResizeVertical           (FALSE)
	, m_bDrawDisabledItems            (FALSE)
	, m_nIconsInRow                   (-1)
	, m_sizeIcon                      (0, 0)
{
}

CBCGPBaseRibbonInfo::XElementButtonPalette::XElementButtonPalette()
	: CBCGPBaseRibbonInfo::XElementButton (CBCGPBaseRibbonInfo::s_szButton_Palette)
	, m_bIsButtonMode                 (TRUE)
	, m_bEnableMenuResize             (FALSE)
	, m_bMenuResizeVertical           (FALSE)
	, m_bDrawDisabledItems            (FALSE)
	, m_nIconsInRow                   (-1)
	, m_sizeIcon                      (0, 0)
{
}

CBCGPBaseRibbonInfo::XElementButtonPalette::~XElementButtonPalette()
{
	int i = 0;
	for (i = 0; i < (int)m_arGroups.GetSize (); i++)
	{
		if (m_arGroups[i] != NULL)
		{
			delete m_arGroups[i];
		}
	}
}

CBCGPBaseRibbonInfo::XElementButtonColor::XElementButtonColor()
	: CBCGPBaseRibbonInfo::XElementButtonPalette (CBCGPBaseRibbonInfo::s_szButton_Color)
	, m_clrColor                      (RGB (0, 0, 0))
	, m_bSimpleButtonLook             (FALSE)
	, m_clrAutomaticBtnColor          (RGB (0, 0, 0))
	, m_bAutomaticBtnOnTop            (TRUE)
	, m_bAutomaticBtnBorder           (FALSE)
{
	m_sizeIcon    = CSize (22, 22);
	m_nIconsInRow = 5;
}

CBCGPBaseRibbonInfo::XElementButtonColor::~XElementButtonColor()
{
}

CBCGPBaseRibbonInfo::XElementButtonUndo::XElementButtonUndo()
	: CBCGPBaseRibbonInfo::XElementButtonPalette (CBCGPBaseRibbonInfo::s_szButton_Undo)
{
}

CBCGPBaseRibbonInfo::XElementButtonUndo::~XElementButtonUndo()
{
}

CBCGPBaseRibbonInfo::XElementButtonLaunch::XElementButtonLaunch()
	: CBCGPBaseRibbonInfo::XElementButton (CBCGPBaseRibbonInfo::s_szButton_Launch)
{
}

CBCGPBaseRibbonInfo::XElementButtonLaunch::~XElementButtonLaunch()
{
}

CBCGPBaseRibbonInfo::XElementButtonMain::XElementButtonMain()
	: CBCGPBaseRibbonInfo::XElementButton (CBCGPBaseRibbonInfo::s_szButton_Main)
{
}

CBCGPBaseRibbonInfo::XElementButtonMain::~XElementButtonMain()
{
}

CBCGPBaseRibbonInfo::XElementButtonMainPanel::XElementButtonMainPanel()
	: CBCGPBaseRibbonInfo::XElementButton (CBCGPBaseRibbonInfo::s_szButton_MainPanel)
{
}

CBCGPBaseRibbonInfo::XElementButtonMainPanel::~XElementButtonMainPanel()
{
}

CBCGPBaseRibbonInfo::XElementButtonCommand::XElementButtonCommand()
	: CBCGPBaseRibbonInfo::XElementButton (CBCGPBaseRibbonInfo::s_szButton_Command)
	, m_bIsMenu                       (FALSE)
{
}

CBCGPBaseRibbonInfo::XElementButtonCommand::~XElementButtonCommand()
{
}

CBCGPBaseRibbonInfo::XElementSlider::XElementSlider()
	: CBCGPBaseRibbonInfo::XElement (CBCGPBaseRibbonInfo::s_szSlider)
	, m_dwStyle                 (0)
	, m_nWidth                  (100)
	, m_nMin                    (0)
	, m_nMax                    (100)
	, m_nPos                    (0)
	, m_bZoomButtons            (FALSE)
{
}

CBCGPBaseRibbonInfo::XElementSlider::~XElementSlider()
{
}

CBCGPBaseRibbonInfo::XElementProgressBar::XElementProgressBar()
	: CBCGPBaseRibbonInfo::XElement (CBCGPBaseRibbonInfo::s_szProgress)
	, m_nWidth                  (100)
	, m_nHeight                 (22)
	, m_nMin                    (0)
	, m_nMax                    (100)
	, m_nPos                    (0)
	, m_bInfinite               (FALSE)
	, m_bVertical               (FALSE)
{
}

CBCGPBaseRibbonInfo::XElementProgressBar::~XElementProgressBar()
{
}

CBCGPBaseRibbonInfo::XElementStatusPane::XElementStatusPane()
	: CBCGPBaseRibbonInfo::XElementButton (CBCGPBaseRibbonInfo::s_szStatusPane)
	, m_bIsStatic                     (TRUE)
	, m_nTextAlign                    (TA_LEFT)
{
}

CBCGPBaseRibbonInfo::XElementStatusPane::~XElementStatusPane()
{
}

CBCGPBaseRibbonInfo::XPanel::XPanel()
	: CBCGPBaseInfo::XBase (CBCGPBaseRibbonInfo::s_szPanel)
	, m_nImageIndex        (-1)
	, m_bJustifyColumns    (FALSE)
	, m_bCenterColumnVert  (FALSE)
{
}

CBCGPBaseRibbonInfo::XPanel::~XPanel()
{
	int i;
	for (i = 0; i < (int)m_arElements.GetSize (); i++)
	{
		if (m_arElements[i] != NULL)
		{
			delete m_arElements[i];
		}
	}
}

CBCGPBaseRibbonInfo::XCategory::XCategory()
	: CBCGPBaseInfo::XBase (CBCGPBaseRibbonInfo::s_szCategory)
{
}

CBCGPBaseRibbonInfo::XCategory::~XCategory()
{
	int i;
	for (i = 0; i < (int)m_arPanels.GetSize (); i++)
	{
		if (m_arPanels[i] != NULL)
		{
			delete m_arPanels[i];
		}
	}

	for (i = 0; i < (int)m_arElements.GetSize (); i++)
	{
		if (m_arElements[i] != NULL)
		{
			delete m_arElements[i];
		}
	}
}

CBCGPBaseRibbonInfo::XContext::XContext()
	: CBCGPBaseInfo::XBase (CBCGPBaseRibbonInfo::s_szContext)
	, m_Color              (BCGPCategoryColor_None)
{
}

CBCGPBaseRibbonInfo::XContext::~XContext()
{
	int i;
	for (i = 0; i < (int)m_arCategories.GetSize (); i++)
	{
		if (m_arCategories[i] != NULL)
		{
			delete m_arCategories[i];
		}
	}
}

CBCGPBaseRibbonInfo::XCategoryMain::XCategoryMain()
	: CBCGPBaseInfo::XBase (CBCGPBaseRibbonInfo::s_szCategoryMain)
	, m_bRecentListEnable  (FALSE)
	, m_nRecentListWidth   (300)
	, m_bRecentListShowPins(FALSE)
	, m_bSearchEnable      (FALSE)
	, m_nSearchWidth       (0)
{
}

CBCGPBaseRibbonInfo::XCategoryMain::~XCategoryMain()
{
	int i = 0;
	for (i = 0; i < (int)m_arElements.GetSize (); i++)
	{
		if (m_arElements[i] != NULL)
		{
			delete m_arElements[i];
		}
	}
}

CBCGPBaseRibbonInfo::XCategoryBackstage::XCategoryBackstage()
	: CBCGPBaseInfo::XBase (CBCGPBaseRibbonInfo::s_szCategoryBackstage)
{
}

CBCGPBaseRibbonInfo::XCategoryBackstage::~XCategoryBackstage()
{
	int i = 0;
	for (i = 0; i < (int)m_arElements.GetSize (); i++)
	{
		if (m_arElements[i] != NULL)
		{
			delete m_arElements[i];
		}
	}
}

CBCGPBaseRibbonInfo::XQAT::XQATItem::XQATItem()
	: m_bVisible (TRUE)
{
}

CBCGPBaseRibbonInfo::XQAT::XQATItem::~XQATItem()
{
}

CBCGPBaseRibbonInfo::XQAT::XQAT()
	: CBCGPBaseInfo::XBase (CBCGPBaseRibbonInfo::s_szQAT)
	, m_bOnTop             (TRUE)
{
}

CBCGPBaseRibbonInfo::XQAT::~XQAT()
{
	m_arItems.RemoveAll ();
}

CBCGPBaseRibbonInfo::XRibbonBar::XRibbonBar()
	: CBCGPBaseInfo::XBase (CBCGPBaseRibbonInfo::s_szRibbonBar)
	, m_bToolTip           (TRUE)
	, m_bToolTipDescr      (TRUE)
	, m_bKeyTips           (TRUE)
	, m_bPrintPreview      (TRUE)
	, m_bDrawUsingFont     (FALSE)
	, m_btnMain            (NULL)
	, m_MainCategory       (NULL)
	, m_bBackstageMode     (FALSE)
	, m_BackstageCategory  (NULL)
{
}

CBCGPBaseRibbonInfo::XRibbonBar::~XRibbonBar()
{
	if (m_btnMain != NULL)
	{
		delete m_btnMain;
	}

	if (m_MainCategory != NULL)
	{
		delete m_MainCategory;
	}

	if (m_BackstageCategory != NULL)
	{
		delete m_BackstageCategory;
	}

	int i;
	for (i = 0; i < (int)m_arCategories.GetSize (); i++)
	{
		if (m_arCategories[i] != NULL)
		{
			delete m_arCategories[i];
		}
	}

	for (i = 0; i < (int)m_arContexts.GetSize (); i++)
	{
		if (m_arContexts[i] != NULL)
		{
			delete m_arContexts[i];
		}
	}
}

CBCGPBaseRibbonInfo::XStatusBar::XStatusElements::XStatusElements()
{
}

CBCGPBaseRibbonInfo::XStatusBar::XStatusElements::~XStatusElements()
{
	int i;
	for (i = 0; i < (int)m_arElements.GetSize (); i++)
	{
		if (m_arElements[i] != NULL)
		{
			delete m_arElements[i];
		}
	}
}

CBCGPBaseRibbonInfo::XStatusBar::XStatusBar()
	: CBCGPBaseInfo::XBase (CBCGPBaseRibbonInfo::s_szStatusBar)
{
}

CBCGPBaseRibbonInfo::XStatusBar::~XStatusBar()
{
}


CBCGPBaseInfo::XBase* CBCGPBaseRibbonInfo::CreateBaseFromName (const CString& name)
{
	CBCGPBaseInfo::XBase* base = NULL;

	if (name.Compare (CBCGPBaseRibbonInfo::s_szPanel) == 0)
	{
		base = new CBCGPBaseRibbonInfo::XPanel;
	}
	else if (name.Compare (CBCGPBaseRibbonInfo::s_szCategoryMain) == 0)
	{
		base = new CBCGPBaseRibbonInfo::XCategoryMain;
	}
	else if (name.Compare (CBCGPBaseRibbonInfo::s_szCategoryBackstage) == 0)
	{
		base = new CBCGPBaseRibbonInfo::XCategoryBackstage;
	}
	else if (name.Compare (CBCGPBaseRibbonInfo::s_szCategory) == 0)
	{
		base = new CBCGPBaseRibbonInfo::XCategory;
	}
	else if (name.Compare (CBCGPBaseRibbonInfo::s_szRibbonBar) == 0)
	{
		base = new CBCGPBaseRibbonInfo::XRibbonBar;
	}
	else if (name.Compare (CBCGPBaseRibbonInfo::s_szStatusBar) == 0)
	{
		base = new CBCGPBaseRibbonInfo::XStatusBar;
	}
	else if (name.Compare (CBCGPBaseRibbonInfo::s_szQAT) == 0)
	{
		base = new CBCGPBaseRibbonInfo::XQAT;
	}
	else if (name.Compare (CBCGPBaseRibbonInfo::s_szContext) == 0)
	{
		base = new CBCGPBaseRibbonInfo::XContext;
	}
	else
	{
		base = CBCGPBaseRibbonInfo::XElement::CreateFromName (name);
	}

	return base;
}

CBCGPBaseInfo::XBase* CBCGPBaseRibbonInfo::CreateBaseFromTag (const CString& tag)
{
	return CBCGPBaseInfo::CreateBaseFromTag (tag, &CBCGPBaseRibbonInfo::CreateBaseFromName);
}

CBCGPBaseRibbonInfo::XElement* CBCGPBaseRibbonInfo::XElement::CreateFromName (const CString& name)
{
	CBCGPBaseRibbonInfo::XElement* element = NULL;

	if (name.Compare (CBCGPBaseRibbonInfo::s_szButton) == 0)
	{
		element = new CBCGPBaseRibbonInfo::XElementButton;
	}
	else if (name.Compare (CBCGPBaseRibbonInfo::s_szButton_Check) == 0)
	{
		element = new CBCGPBaseRibbonInfo::XElementButtonCheck;
	}
	else if (name.Compare (CBCGPBaseRibbonInfo::s_szButton_Radio) == 0)
	{
		element = new CBCGPBaseRibbonInfo::XElementButtonRadio;
	}
	else if (name.Compare (CBCGPBaseRibbonInfo::s_szButton_Color) == 0)
	{
		element = new CBCGPBaseRibbonInfo::XElementButtonColor;
	}
	else if (name.Compare (CBCGPBaseRibbonInfo::s_szButton_Undo) == 0)
	{
		element = new CBCGPBaseRibbonInfo::XElementButtonUndo;
	}
	else if (name.Compare (CBCGPBaseRibbonInfo::s_szButton_Palette) == 0 ||
			name.Compare (_T("Button_Gallery")) == 0)
	{
		element = new CBCGPBaseRibbonInfo::XElementButtonPalette;
	}
	else if (name.Compare (CBCGPBaseRibbonInfo::s_szButton_Hyperlink) == 0 ||
			name.Compare (_T("Button_LinkCtrl")) == 0)
	{
		element = new CBCGPBaseRibbonInfo::XElementButtonHyperlink;
	}
	else if (name.Compare (CBCGPBaseRibbonInfo::s_szButton_Main) == 0)
	{
		element = new CBCGPBaseRibbonInfo::XElementButtonMain;
	}
	else if (name.Compare (CBCGPBaseRibbonInfo::s_szButton_MainPanel) == 0)
	{
		element = new CBCGPBaseRibbonInfo::XElementButtonMainPanel;
	}
	else if (name.Compare (CBCGPBaseRibbonInfo::s_szButton_Command) == 0)
	{
		element = new CBCGPBaseRibbonInfo::XElementButtonCommand;
	}
	else if (name.Compare (CBCGPBaseRibbonInfo::s_szButton_Launch) == 0)
	{
		element = new CBCGPBaseRibbonInfo::XElementButtonLaunch;
	}
	else if (name.Compare (CBCGPBaseRibbonInfo::s_szLabel) == 0)
	{
		element = new CBCGPBaseRibbonInfo::XElementLabel;
	}
	else if (name.Compare (CBCGPBaseRibbonInfo::s_szEdit) == 0)
	{
		element = new CBCGPBaseRibbonInfo::XElementEdit;
	}
	else if (name.Compare (CBCGPBaseRibbonInfo::s_szComboBox) == 0)
	{
		element = new CBCGPBaseRibbonInfo::XElementComboBox;
	}
	else if (name.Compare (CBCGPBaseRibbonInfo::s_szComboBox_Font) == 0)
	{
		element = new CBCGPBaseRibbonInfo::XElementFontComboBox;
	}
	else if (name.Compare (CBCGPBaseRibbonInfo::s_szSlider) == 0)
	{
		element = new CBCGPBaseRibbonInfo::XElementSlider;
	}
	else if (name.Compare (CBCGPBaseRibbonInfo::s_szProgress) == 0)
	{
		element = new CBCGPBaseRibbonInfo::XElementProgressBar;
	}
	else if (name.Compare (CBCGPBaseRibbonInfo::s_szSeparator) == 0)
	{
		element = new CBCGPBaseRibbonInfo::XElementSeparator;
	}
	else if (name.Compare (CBCGPBaseRibbonInfo::s_szGroup) == 0)
	{
		element = new CBCGPBaseRibbonInfo::XElementGroup;
	}
	else if (name.Compare (CBCGPBaseRibbonInfo::s_szStatusPane) == 0)
	{
		element = new CBCGPBaseRibbonInfo::XElementStatusPane;
	}

	return element;
}

BOOL CBCGPBaseRibbonInfo::XElement::FromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	CString strID;
	if (tm.ExcludeTag (s_szTag_ID, strID))
	{
		m_ID.FromTag (strID);
	}

	tm.ReadEntityString (s_szTag_Text, m_strText);
	tm.ReadEntityString (s_szTag_ToolTip, m_strToolTip);
	tm.ReadEntityString (s_szTag_Description, m_strDescription);
	tm.ReadEntityString (s_szTag_Keys, m_strKeys);
	tm.ReadEntityString (s_szTag_MenuKeys, m_strMenuKeys);
	tm.ReadBool (s_szTag_PaletteTop, m_bIsOnPaletteTop);
	tm.ReadBool (s_szTag_AlwaysLarge, m_bIsAlwaysLarge);

	return TRUE;
}

void CBCGPBaseRibbonInfo::XElement::ToTag (CString& strTag) const
{
	XBase::ToTag (strTag);

	CString strID;
	m_ID.ToTag (strID);
	CBCGPTagManager::WriteItem (strTag, s_szTag_ID, strID);

	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteEntityString (s_szTag_Text, m_strText));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteEntityString (s_szTag_ToolTip, m_strToolTip));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteEntityString (s_szTag_Description, m_strDescription));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteEntityString (s_szTag_Keys, m_strKeys));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteEntityString (s_szTag_MenuKeys, m_strMenuKeys));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBool (s_szTag_PaletteTop, m_bIsOnPaletteTop, FALSE));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBool (s_szTag_AlwaysLarge, m_bIsAlwaysLarge, FALSE));
}

BOOL CBCGPBaseRibbonInfo::XElementSeparator::FromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	tm.ReadBool (s_szTag_Horiz, m_bIsHoriz);

	return TRUE;
}

void CBCGPBaseRibbonInfo::XElementSeparator::ToTag (CString& strTag) const
{
	XBase::ToTag (strTag);

	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBool (s_szTag_Horiz, m_bIsHoriz, FALSE));
}

BOOL CBCGPBaseRibbonInfo::XElementGroup::FromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	CString strElements;
	if (tm.ExcludeTag (s_szTag_Elements, strElements))
	{
		CBCGPTagManager tmElements (strElements);

		CString strElement;
		while (tmElements.ExcludeTag (s_szTag_Element, strElement))
		{
			XElement* pElement = (XElement*)CBCGPBaseRibbonInfo::CreateBaseFromTag (strElement);
			if (pElement != NULL)
			{
				m_arButtons.Add (pElement);
			}
		}
	}

    CString strImage;
    if (tm.ExcludeTag(s_szTag_Image, strImage))
    {
        m_Images.FromTag(strImage);
    }

	return TRUE;
}

void CBCGPBaseRibbonInfo::XElementGroup::ToTag (CString& strTag) const
{
	XBase::ToTag (strTag);

	CString strImage;
	m_Images.ToTag (strImage);
	CBCGPTagManager::WriteItem (strTag, s_szTag_Image, strImage);
	
	CString strElements;

	int i = 0;
	for (i = 0; i < (int)m_arButtons.GetSize (); i++)
	{
		CString strElement;
		m_arButtons[i]->ToTag (strElement);
		CBCGPTagManager::WriteItem (strElements, s_szTag_Element, strElement);
	}

	CBCGPTagManager::WriteItem (strTag, s_szTag_Elements, strElements);
}

BOOL CBCGPBaseRibbonInfo::XElementButton::FromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	CString strElements;
	if (tm.ExcludeTag (s_szTag_Elements, strElements))
	{
		CBCGPTagManager tmElements (strElements);

		CString strElement;
		while (tmElements.ExcludeTag (s_szTag_Element, strElement))
		{
			XElement* pElement = (XElement*)CBCGPBaseRibbonInfo::CreateBaseFromTag (strElement);
			if (pElement != NULL)
			{
				m_arSubItems.Add (pElement);
			}
		}
	}

	tm.ReadInt (s_szTag_IndexSmall, m_nSmallImageIndex);
	tm.ReadInt (s_szTag_IndexLarge, m_nLargeImageIndex);
	tm.ReadBool (s_szTag_DefaultCommand, m_bIsDefaultCommand);
	tm.ReadBool (s_szTag_AlwaysShowDescription, m_bIsAlwaysShowDescription);

	int QATType = (int)CBCGPRibbonButton::BCGPRibbonButton_Show_Default;
	tm.ReadInt (s_szTag_QATType, QATType);
	m_QATType = (CBCGPRibbonButton::RibbonButtonOnQAT)bcg_clamp (QATType, (int)CBCGPRibbonButton::BCGPRibbonButton_Show_Default, (int)CBCGPRibbonButton::BCGPRibbonButton_Show_As_RadioButton);

	return XElement::FromTag (tm.GetBuffer ());
}

void CBCGPBaseRibbonInfo::XElementButton::ToTag (CString& strTag) const
{
	XElement::ToTag (strTag);

	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteInt (s_szTag_IndexSmall, m_nSmallImageIndex, -1));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteInt (s_szTag_IndexLarge, m_nLargeImageIndex, -1));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBool (s_szTag_DefaultCommand, m_bIsDefaultCommand, TRUE));

	if (GetElementName ().Compare (s_szButton) == 0)
	{
		CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBool (s_szTag_AlwaysShowDescription, m_bIsAlwaysShowDescription, FALSE));
	}

	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteInt (s_szTag_QATType, (int)m_QATType, (int)CBCGPRibbonButton::BCGPRibbonButton_Show_Default));

	CString strElements;

	int i = 0;
	for (i = 0; i < (int)m_arSubItems.GetSize (); i++)
	{
		CString strElement;
		m_arSubItems[i]->ToTag (strElement);
		CBCGPTagManager::WriteItem (strElements, s_szTag_Element, strElement);
	}

	CBCGPTagManager::WriteItem (strTag, s_szTag_Elements, strElements);
}

BOOL CBCGPBaseRibbonInfo::XElementLabel::FromTag (const CString& strTag)
{
	return XElementButton::FromTag (strTag);
}

void CBCGPBaseRibbonInfo::XElementLabel::ToTag (CString& strTag) const
{
	XElementButton::ToTag (strTag);
}

BOOL CBCGPBaseRibbonInfo::XElementButtonCheck::FromTag (const CString& strTag)
{
	return XElementButton::FromTag (strTag);
}

void CBCGPBaseRibbonInfo::XElementButtonCheck::ToTag (CString& strTag) const
{
	XElementButton::ToTag (strTag);
}

BOOL CBCGPBaseRibbonInfo::XElementButtonRadio::FromTag (const CString& strTag)
{
	return XElementButton::FromTag (strTag);
}

void CBCGPBaseRibbonInfo::XElementButtonRadio::ToTag (CString& strTag) const
{
	XElementButton::ToTag (strTag);
}

BOOL CBCGPBaseRibbonInfo::XElementButtonHyperlink::FromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	tm.ReadString (s_szTag_Link, m_strLink);

	return XElementButton::FromTag (tm.GetBuffer ());
}

void CBCGPBaseRibbonInfo::XElementButtonHyperlink::ToTag (CString& strTag) const
{
	XElementButton::ToTag (strTag);

	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteString (s_szTag_Link, m_strLink));
}

BOOL CBCGPBaseRibbonInfo::XElementEdit::FromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	tm.ReadInt (s_szTag_Width, m_nWidth);
	tm.ReadInt (s_szTag_WidthFloaty, m_nWidthFloaty);
	tm.ReadInt (s_szTag_TextAlign, m_nTextAlign);
	tm.ReadEntityString (s_szTag_SearchPrompt, m_strSearchPrompt);
	tm.ReadBool (s_szTag_SearchMode, m_bSearchMode);

	if (!m_bSearchMode)
	{
		tm.ReadBool (s_szTag_SpinButtons, m_bHasSpinButtons);
		if (m_bHasSpinButtons)
		{
			tm.ReadInt (s_szTag_Min, m_nMin);
			tm.ReadInt (s_szTag_Max, m_nMax);
		}
	}

	if (!XElementButton::FromTag (tm.GetBuffer ()))
	{
		return FALSE;
	}

	CString strID;
	tm.ExcludeTag (s_szTag_ID, strID);

	tm.ReadEntityString(s_szTag_Value, m_strValue);

	return TRUE;
}

void CBCGPBaseRibbonInfo::XElementEdit::ToTag (CString& strTag) const
{
	XElementButton::ToTag (strTag);

	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteInt (s_szTag_Width, m_nWidth, 0));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteInt (s_szTag_WidthFloaty, m_nWidthFloaty, 0));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteInt (s_szTag_TextAlign, m_nTextAlign, ES_LEFT));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteEntityString (s_szTag_SearchPrompt, m_strSearchPrompt));

	if (m_bSearchMode)
	{
		CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBool (s_szTag_SearchMode, m_bSearchMode, FALSE));
	}
	else
	{
		if (m_bHasSpinButtons)
		{
			CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBool (s_szTag_SpinButtons, m_bHasSpinButtons, FALSE));
			CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteInt (s_szTag_Min, m_nMin, INT_MAX));
			CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteInt (s_szTag_Max, m_nMax, INT_MAX));
		}
	}

	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteEntityString(s_szTag_Value, m_strValue));
}

BOOL CBCGPBaseRibbonInfo::XElementComboBox::FromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	tm.ReadBool (s_szTag_CalculatorMode, m_bCalculatorMode);
	if (m_bCalculatorMode)
	{
		CString strExt;
		if (tm.ReadEntityString (s_szTag_CalculatorCmdExt, strExt))
		{
			if (!strExt.IsEmpty ())
			{
				CStringArray sa;
				CBCGPTagManager::ParseString(strExt, _T(";"), sa, TRUE, FALSE);
				if (sa.GetSize () == 0)
				{
					sa.Add (strExt);
				}

				for (int i = 0; i < (int)sa.GetSize (); i++)
				{
					CBCGPCalculator::CalculatorCommands nCommand = 
						CBCGPCalculator::NameToCommand (sa[i]);
					if (nCommand != CBCGPCalculator::idCommandNone)
					{
						m_lstCalculatorExt.AddTail (nCommand);
					}
				}
			}
		}
	}
	else
	{
		CString strItems;
		if (tm.ExcludeTag (s_szTag_Items, strItems))
		{
			CBCGPTagManager tmItem (strItems);

			CString strItem;
			while (tmItem.ExcludeTag (s_szTag_Item, strItem))
			{
				CBCGPTagManager::Entity_FromTag (strItem);
				m_arItems.Add (strItem);
			}
		}

		tm.ReadBool (s_szTag_EditBox, m_bHasEditBox);
		tm.ReadBool (s_szTag_DropDownList, m_bHasDropDownList);
		tm.ReadBool (s_szTag_ResizeDropDownList, m_bResizeDropDownList);
		tm.ReadBool (s_szTag_AutoComplete, m_bAutoComplete);
		int sortOrder = (int)CBCGPRibbonComboBox::BCGP_RIBBON_COMBO_SORT_ORDER_NO_SORT;
		tm.ReadInt (s_szTag_SortOrder, sortOrder);
		m_sortOrder = (CBCGPRibbonComboBox::BCGP_RIBBON_COMBO_SORT_ORDER)bcg_clamp(sortOrder, (int)CBCGPRibbonComboBox::BCGP_RIBBON_COMBO_SORT_ORDER_NO_SORT, (int)CBCGPRibbonComboBox::BCGP_RIBBON_COMBO_SORT_ORDER_DESC);
	}

	return XElementEdit::FromTag (tm.GetBuffer ());
}

void CBCGPBaseRibbonInfo::XElementComboBox::ToTag (CString& strTag) const
{
	XElementEdit::ToTag (strTag);

	if (m_bCalculatorMode)
	{
		CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBool (s_szTag_CalculatorMode, m_bCalculatorMode, FALSE));

		CString strExt;
		POSITION pos = m_lstCalculatorExt.GetHeadPosition ();
		while (pos != NULL)
		{
			if (!strExt.IsEmpty ())
			{
				strExt += _T(";");
			}

			strExt += CBCGPCalculator::CommandToName ((CBCGPCalculator::CalculatorCommands)m_lstCalculatorExt.GetNext (pos));
		}

		CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteEntityString (s_szTag_CalculatorCmdExt, strExt));
	}
	else
	{
		CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBool (s_szTag_EditBox, m_bHasEditBox, FALSE));
		CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBool (s_szTag_DropDownList, m_bHasDropDownList, TRUE));
		CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBool (s_szTag_ResizeDropDownList, m_bResizeDropDownList, TRUE));
		CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBool (s_szTag_AutoComplete, m_bAutoComplete, FALSE));
		CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteInt (s_szTag_SortOrder, (int)m_sortOrder, (int)CBCGPRibbonComboBox::BCGP_RIBBON_COMBO_SORT_ORDER_NO_SORT));

		CString strItems;

		int i = 0;
		for (i = 0; i < (int)m_arItems.GetSize (); i++)
		{
			CBCGPTagManager::WriteTag (strItems, CBCGPTagManager::WriteEntityString (s_szTag_Item, m_arItems[i], m_arItems[i] + _T("_")));
		}

		CBCGPTagManager::WriteItem (strTag, s_szTag_Items, strItems);
	}
}

BOOL CBCGPBaseRibbonInfo::XElementFontComboBox::FromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	tm.ReadInt (s_szTag_FontType, m_nFontType);

	int nValue = m_nCharSet;
	tm.ReadInt (s_szTag_CharSet, nValue);
	m_nCharSet = (BYTE)nValue;

	nValue = m_nPitchAndFamily;
	tm.ReadInt (s_szTag_PitchAndFamily, nValue);
	m_nPitchAndFamily = (BYTE)nValue;

	return XElementComboBox::FromTag (tm.GetBuffer ());
}

void CBCGPBaseRibbonInfo::XElementFontComboBox::ToTag (CString& strTag) const
{
	XElementComboBox::ToTag (strTag);

	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteInt (s_szTag_FontType, m_nFontType, DEVICE_FONTTYPE | RASTER_FONTTYPE | TRUETYPE_FONTTYPE));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteInt (s_szTag_CharSet, m_nCharSet, DEFAULT_CHARSET));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteInt (s_szTag_PitchAndFamily, m_nPitchAndFamily, DEFAULT_PITCH));
}

BOOL CBCGPBaseRibbonInfo::XElementButtonPalette::XPaletteGroup::FromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	tm.ReadEntityString (s_szTag_Name, m_strName);
	tm.ReadInt (s_szTag_Items, m_nItems);
/*
	CString strToolTip;
	while (tm.ExcludeTag (s_szTag_ToolTip, strToolTip))
	{
		CBCGPTagManager::Entity_FromTag (strToolTip);
		m_arToolTips.Add (strToolTip);
	}
*/
	return TRUE;
}

void CBCGPBaseRibbonInfo::XElementButtonPalette::XPaletteGroup::ToTag (CString& strTag) const
{
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteEntityString (s_szTag_Name, m_strName));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteInt (s_szTag_Items, m_nItems, 0));
/*
	if (m_arToolTips.GetSize () > 0)
	{
		int i = 0;
		for (i = 0; i < m_arToolTips.GetSize (); i++)
		{
			CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteEntityString (s_szTag_ToolTip, m_arToolTips[i]), m_arToolTips[i] + _T("_"));
		}
	}
*/
}

BOOL CBCGPBaseRibbonInfo::XElementButtonPalette::FromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	CString strGroups;
	if (tm.ExcludeTag (s_szTag_Groups, strGroups))
	{
		CBCGPTagManager tmGroups (strGroups);

		CString strGroup;
		while (tmGroups.ExcludeTag (s_szTag_Group, strGroup))
		{
			XPaletteGroup* pGroup = new XPaletteGroup;
			if (pGroup->FromTag (strGroup))
			{
				m_arGroups.Add (pGroup);
			}
			else
			{
				delete pGroup;
			}
		}
	}

	tm.ReadBool (s_szTag_ButtonMode, m_bIsButtonMode);
	tm.ReadBool (s_szTag_MenuResize, m_bEnableMenuResize);
	tm.ReadBool (s_szTag_MenuResizeVertical, m_bMenuResizeVertical);
	tm.ReadBool (s_szTag_DrawDisabledItems, m_bDrawDisabledItems);
	tm.ReadInt (s_szTag_IconsInRow, m_nIconsInRow);
	if (!tm.ReadSize (s_szTag_SizeBox, m_sizeIcon))
	{
		tm.ReadSize (s_szTag_SizeIcon, m_sizeIcon);
	}

	CString strImage;
	if (tm.ExcludeTag(s_szTag_Image, strImage))
	{
		m_Images.FromTag(strImage);
		m_Images.m_Image.SetImageSize (m_sizeIcon);
	}

	return XElementButton::FromTag (tm.GetBuffer ());
}

void CBCGPBaseRibbonInfo::XElementButtonPalette::ToTag (CString& strTag) const
{
	XElementButton::ToTag (strTag);

	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBool (s_szTag_ButtonMode, m_bIsButtonMode, TRUE));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBool (s_szTag_MenuResize, m_bEnableMenuResize, FALSE));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBool (s_szTag_MenuResizeVertical, m_bMenuResizeVertical, FALSE));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBool (s_szTag_DrawDisabledItems, m_bDrawDisabledItems, FALSE));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteInt (s_szTag_IconsInRow, m_nIconsInRow, -1));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteSize (s_szTag_SizeIcon, m_sizeIcon, CSize (0, 0)));

	if (m_arGroups.GetSize () > 0)
	{
		CString strGroups;

		int i = 0;
		for (i = 0; i < (int)m_arGroups.GetSize (); i++)
		{
			CString strGroup;
			m_arGroups[i]->ToTag (strGroup);
			CBCGPTagManager::WriteItem (strGroups, s_szTag_Group, strGroup);
		}

		CBCGPTagManager::WriteItem (strTag, s_szTag_Groups, strGroups);
	}

	if (!m_Images.IsEmpty ())
	{
		CString strImage;
		m_Images.ToTag (strImage);
		CBCGPTagManager::WriteItem (strTag, s_szTag_Image, strImage);
	}
}

BOOL CBCGPBaseRibbonInfo::XElementButtonColor::FromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	CString strTagBtn;
	if (tm.ExcludeTag (s_szTag_AutomaticColorBtn, strTagBtn))
	{
		CBCGPTagManager tmBtn (strTagBtn);

		tmBtn.ReadEntityString (s_szTag_Label, m_strAutomaticBtnLabel);
		tmBtn.ReadEntityString (s_szTag_ToolTip, m_strAutomaticBtnToolTip);
		tmBtn.ReadColor (s_szTag_Color, m_clrAutomaticBtnColor);
		tmBtn.ReadBool (s_szTag_PaletteTop, m_bAutomaticBtnOnTop);
		tmBtn.ReadBool (s_szTag_Border, m_bAutomaticBtnBorder);
	}

	strTagBtn.Empty ();
	if (tm.ExcludeTag (s_szTag_OtherColorBtn, strTagBtn))
	{
		CBCGPTagManager tmBtn (strTagBtn);

		tmBtn.ReadEntityString (s_szTag_Label, m_strOtherBtnLabel);
		tmBtn.ReadEntityString (s_szTag_ToolTip, m_strOtherBtnToolTip);
	}

	tm.ReadColor (s_szTag_Color, m_clrColor);
	tm.ReadBool (s_szTag_SimpleButtonLook, m_bSimpleButtonLook);

	return XElementButtonPalette::FromTag (tm.GetBuffer ());
}

void CBCGPBaseRibbonInfo::XElementButtonColor::ToTag (CString& strTag) const
{
	XElementButtonPalette::ToTag (strTag);

	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteColor (s_szTag_Color, m_clrColor, RGB (0, 0, 0)));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBool (s_szTag_SimpleButtonLook, m_bSimpleButtonLook, FALSE));

	CString strTagBtn;
	CBCGPTagManager::WriteTag (strTagBtn, CBCGPTagManager::WriteEntityString (s_szTag_Label, m_strAutomaticBtnLabel));
	CBCGPTagManager::WriteTag (strTagBtn, CBCGPTagManager::WriteEntityString (s_szTag_ToolTip, m_strAutomaticBtnToolTip));
	CBCGPTagManager::WriteTag (strTagBtn, CBCGPTagManager::WriteColor (s_szTag_Color, m_clrAutomaticBtnColor, RGB (0, 0, 0)));
	CBCGPTagManager::WriteTag (strTagBtn, CBCGPTagManager::WriteBool (s_szTag_PaletteTop, m_bAutomaticBtnOnTop, TRUE));
	CBCGPTagManager::WriteTag (strTagBtn, CBCGPTagManager::WriteBool (s_szTag_Border, m_bAutomaticBtnBorder, FALSE));

	CBCGPTagManager::WriteItem (strTag, s_szTag_AutomaticColorBtn, strTagBtn);

	strTagBtn.Empty ();
	CBCGPTagManager::WriteTag (strTagBtn, CBCGPTagManager::WriteEntityString (s_szTag_Label, m_strOtherBtnLabel));
	CBCGPTagManager::WriteTag (strTagBtn, CBCGPTagManager::WriteEntityString (s_szTag_ToolTip, m_strOtherBtnToolTip));

	CBCGPTagManager::WriteItem (strTag, s_szTag_OtherColorBtn, strTagBtn);
}

BOOL CBCGPBaseRibbonInfo::XElementButtonUndo::FromTag (const CString& strTag)
{
	return XElementButtonPalette::FromTag (strTag);
}

void CBCGPBaseRibbonInfo::XElementButtonUndo::ToTag (CString& strTag) const
{
	XElementButtonPalette::ToTag (strTag);
}

BOOL CBCGPBaseRibbonInfo::XElementButtonLaunch::FromTag (const CString& strTag)
{
	return XElementButton::FromTag (strTag);
}

void CBCGPBaseRibbonInfo::XElementButtonLaunch::ToTag (CString& strTag) const
{
	if (m_ID.m_Value != 0)
	{
		XElementButton::ToTag (strTag);
	}
}

BOOL CBCGPBaseRibbonInfo::XElementButtonMain::FromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

    CString strImage;
    if (tm.ExcludeTag(s_szTag_Image, strImage))
    {
        m_Image.FromTag(strImage);
    }

    CString strImageScenic;
    if (tm.ExcludeTag(s_szTag_Image_Scenic, strImageScenic))
    {
        m_ImageScenic.FromTag(strImageScenic);
    }

	return XElementButton::FromTag (tm.GetBuffer ());
}

void CBCGPBaseRibbonInfo::XElementButtonMain::ToTag (CString& strTag) const
{
	XElementButton::ToTag (strTag);

	CString strImage;
	m_Image.ToTag (strImage);
	CBCGPTagManager::WriteItem (strTag, s_szTag_Image, strImage);

	CString strImageScenic;
	m_ImageScenic.ToTag (strImageScenic);
	CBCGPTagManager::WriteItem (strTag, s_szTag_Image_Scenic, strImageScenic);
}

BOOL CBCGPBaseRibbonInfo::XElementButtonMainPanel::FromTag (const CString& strTag)
{
	return XElementButton::FromTag (strTag);
}

void CBCGPBaseRibbonInfo::XElementButtonMainPanel::ToTag (CString& strTag) const
{
	XElementButton::ToTag (strTag);
}

BOOL CBCGPBaseRibbonInfo::XElementButtonCommand::FromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	tm.ReadBool (s_szTag_MenuMode, m_bIsMenu);

	return XElementButton::FromTag (strTag);
}

void CBCGPBaseRibbonInfo::XElementButtonCommand::ToTag (CString& strTag) const
{
	XElementButton::ToTag (strTag);

	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBool (s_szTag_MenuMode, m_bIsMenu, FALSE));
}

BOOL CBCGPBaseRibbonInfo::XElementSlider::FromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	tm.ReadDword (s_szTag_Style, m_dwStyle);
	tm.ReadInt (s_szTag_Width, m_nWidth);
	tm.ReadInt (s_szTag_Min, m_nMin);
	tm.ReadInt (s_szTag_Max, m_nMax);
	tm.ReadInt (s_szTag_Pos, m_nPos);
	m_nPos = min (max (m_nMin, m_nPos), m_nMax);
	tm.ReadBool (s_szTag_ZoomButtons, m_bZoomButtons);

	return XElement::FromTag (tm.GetBuffer ());
}

void CBCGPBaseRibbonInfo::XElementSlider::ToTag (CString& strTag) const
{
	XElement::ToTag (strTag);

	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteDword (s_szTag_Style, m_dwStyle, 0));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteInt (s_szTag_Width, m_nWidth, 100));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteInt (s_szTag_Min, m_nMin, 0));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteInt (s_szTag_Max, m_nMax, 100));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteInt (s_szTag_Pos, m_nPos, m_nMin));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBool (s_szTag_ZoomButtons, m_bZoomButtons, FALSE));
}

BOOL CBCGPBaseRibbonInfo::XElementProgressBar::FromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	tm.ReadInt (s_szTag_Width, m_nWidth);
	tm.ReadInt (s_szTag_Height, m_nHeight);
	tm.ReadInt (s_szTag_Min, m_nMin);
	tm.ReadInt (s_szTag_Max, m_nMax);
	tm.ReadInt (s_szTag_Pos, m_nPos);
	m_nPos = min (max (m_nMin, m_nPos), m_nMax);
	tm.ReadBool (s_szTag_Infinite, m_bInfinite);
	tm.ReadBool (s_szTag_Vertical, m_bVertical);

	return XElement::FromTag (tm.GetBuffer ());
}

void CBCGPBaseRibbonInfo::XElementProgressBar::ToTag (CString& strTag) const
{
	XElement::ToTag (strTag);

	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteInt (s_szTag_Width, m_nWidth, 100));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteInt (s_szTag_Height, m_nHeight, 22));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteInt (s_szTag_Min, m_nMin, 0));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteInt (s_szTag_Max, m_nMax, 100));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteInt (s_szTag_Pos, m_nPos, m_nMin));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBool (s_szTag_Infinite, m_bInfinite, FALSE));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBool (s_szTag_Vertical, m_bVertical, FALSE));
}

BOOL CBCGPBaseRibbonInfo::XElementStatusPane::FromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	tm.ReadEntityString (s_szTag_AlmostLargeText, m_strAlmostLargeText);
	tm.ReadBool (s_szTag_Static, m_bIsStatic);
	tm.ReadInt (s_szTag_TextAlign, m_nTextAlign);

	return XElementButton::FromTag (tm.GetBuffer ());
}

void CBCGPBaseRibbonInfo::XElementStatusPane::ToTag (CString& strTag) const
{
	XElementButton::ToTag (strTag);

	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteEntityString (s_szTag_AlmostLargeText, m_strAlmostLargeText));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBool (s_szTag_Static, m_bIsStatic, TRUE));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteInt (s_szTag_TextAlign, m_nTextAlign, TA_LEFT));
}

BOOL CBCGPBaseRibbonInfo::XPanel::FromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	CString strElements;
	if (tm.ExcludeTag (s_szTag_Elements, strElements))
	{
		CBCGPTagManager tmElements (strElements);

		CString strElement;
		while (tmElements.ExcludeTag (s_szTag_Element, strElement))
		{
			XElement* pElement = (XElement*)CBCGPBaseRibbonInfo::CreateBaseFromTag (strElement);
			if (pElement != NULL)
			{
				m_arElements.Add (pElement);
			}
		}
	}

	CString strButton;
	if (tm.ExcludeTag (s_szTag_Button_Launch, strButton))
	{
		m_btnLaunch.FromTag (strButton);
	}

	tm.ReadBool (s_szTag_JustifyColumns, m_bJustifyColumns);
	tm.ReadBool (s_szTag_CenterColumnVert, m_bCenterColumnVert);
	tm.ReadInt (s_szTag_Index, m_nImageIndex);
	tm.ReadEntityString (s_szTag_Name, m_strName);
	tm.ReadEntityString (s_szTag_Keys, m_strKeys);

	return TRUE;
}

void CBCGPBaseRibbonInfo::XPanel::ToTag (CString& strTag) const
{
	XBase::ToTag (strTag);

	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteEntityString (s_szTag_Name, m_strName));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteEntityString (s_szTag_Keys, m_strKeys));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteInt (s_szTag_Index, m_nImageIndex, -1));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBool (s_szTag_JustifyColumns, m_bJustifyColumns, FALSE));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBool (s_szTag_CenterColumnVert, m_bCenterColumnVert, FALSE));

	CString strButton;
	m_btnLaunch.ToTag (strButton);
	CBCGPTagManager::WriteItem (strTag, s_szTag_Button_Launch, strButton);

	CString strElements;

	int i = 0;
	for (i = 0; i < (int)m_arElements.GetSize (); i++)
	{
		CString strElement;
		m_arElements[i]->ToTag (strElement);
		CBCGPTagManager::WriteItem (strElements, s_szTag_Element, strElement);
	}

	CBCGPTagManager::WriteItem (strTag, s_szTag_Elements, strElements);
}

BOOL CBCGPBaseRibbonInfo::XCategory::FromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	CString strPanels;
	if (tm.ExcludeTag (s_szTag_Panels, strPanels))
	{
		CBCGPTagManager tmPanel (strPanels);

		CString strPanel;
		while (tmPanel.ExcludeTag (s_szTag_Panel, strPanel))
		{
			XPanel* pPanel = (XPanel*)CBCGPBaseRibbonInfo::CreateBaseFromTag (strPanel);
			if (pPanel != NULL)
			{
				m_arPanels.Add (pPanel);
			}
		}
	}

	CString strElements;
	if (tm.ExcludeTag (s_szTag_Elements, strElements))
	{
		CBCGPTagManager tmElements (strElements);

		CString strElement;
		while (tmElements.ExcludeTag (s_szTag_Element, strElement))
		{
			XElement* pElement = (XElement*)CBCGPBaseRibbonInfo::CreateBaseFromTag (strElement);
			if (pElement != NULL)
			{
				m_arElements.Add (pElement);
			}
		}
	}

	tm.ReadEntityString (s_szTag_Name, m_strName);
	tm.ReadEntityString (s_szTag_Keys, m_strKeys);

    CString strImage;
    if (tm.ExcludeTag(s_szTag_Image_Small, strImage))
    {
        m_SmallImages.FromTag(strImage);
    }
    if (tm.ExcludeTag(s_szTag_Image_Large, strImage))
    {
        m_LargeImages.FromTag(strImage);
    }

	CString strOrder;
	tm.ReadString (s_szTag_CollapseOrder, strOrder);
	if (!strOrder.IsEmpty ())
	{
		CStringArray sa;
		CBCGPTagManager::ParseString (strOrder, _T(";"), sa, TRUE, FALSE);

		for (int i = 0; i < (int)sa.GetSize (); i++)
		{
			if (!sa[i].IsEmpty ())
			{
				m_arCollapseOrder.Add (_ttoi(sa[i]));
			}
		}
	}

	return TRUE;
}

void CBCGPBaseRibbonInfo::XCategory::ToTag (CString& strTag) const
{
	XBase::ToTag (strTag);

	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteEntityString (s_szTag_Name, m_strName));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteEntityString (s_szTag_Keys, m_strKeys));

	CString strImage;
	m_SmallImages.ToTag (strImage);
	CBCGPTagManager::WriteItem (strTag, s_szTag_Image_Small, strImage);

	strImage.Empty ();
	m_LargeImages.ToTag (strImage);
	CBCGPTagManager::WriteItem (strTag, s_szTag_Image_Large, strImage);

	CString strPanels;

	int i = 0;
	for (i = 0; i < (int)m_arPanels.GetSize (); i++)
	{
		CString strPanel;
		m_arPanels[i]->ToTag (strPanel);
		CBCGPTagManager::WriteItem (strPanels, s_szTag_Panel, strPanel);
	}

	CBCGPTagManager::WriteItem (strTag, s_szTag_Panels, strPanels);

	CString strElements;

	for (i = 0; i < (int)m_arElements.GetSize (); i++)
	{
		CString strElement;
		m_arElements[i]->ToTag (strElement);
		CBCGPTagManager::WriteItem (strElements, s_szTag_Element, strElement);
	}

	CBCGPTagManager::WriteItem (strTag, s_szTag_Elements, strElements);

	int count = (int)m_arCollapseOrder.GetSize ();
	if (count > 0)
	{
		CString strOrder;
		for (i = 0; i < count; i++)
		{
			CString str;
			str.Format(_T("%d"), m_arCollapseOrder[i]);

			strOrder += str;
			if (i < (count - 1))
			{
				strOrder += _T(";");
			}
		}

		CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteString (s_szTag_CollapseOrder, strOrder));
	}
}

BOOL CBCGPBaseRibbonInfo::XContext::FromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	CString strCategories;
	if (tm.ExcludeTag (s_szTag_Categories, strCategories))
	{
		CBCGPTagManager tmElements (strCategories);

		CString strCategory;
		while (tmElements.ExcludeTag (s_szTag_Category, strCategory))
		{
			XCategory* pCategory = (XCategory*)CBCGPBaseRibbonInfo::CreateBaseFromTag (strCategory);
			if (pCategory != NULL)
			{
				m_arCategories.Add (pCategory);
			}
		}
	}

	CString strID;
	if (tm.ExcludeTag (s_szTag_ID, strID))
	{
		m_ID.FromTag (strID);
	}

	tm.ReadEntityString (s_szTag_Text, m_strText);

	int color = (int)BCGPCategoryColor_None;
	tm.ReadInt (s_szTag_Color, color);
	m_Color = (BCGPRibbonCategoryColor)color;

	return TRUE;
}

void CBCGPBaseRibbonInfo::XContext::ToTag (CString& strTag) const
{
	XBase::ToTag (strTag);

	CString strID;
	m_ID.ToTag (strID);
	CBCGPTagManager::WriteItem (strTag, s_szTag_ID, strID);

	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteEntityString (s_szTag_Text, m_strText));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteInt (s_szTag_Color, (int)m_Color, (int)BCGPCategoryColor_None));

	CString strCategories;
	for (int i = 0; i < (int)m_arCategories.GetSize (); i++)
	{
		CString strCategoryTag;
		m_arCategories[i]->ToTag (strCategoryTag);
		CBCGPTagManager::WriteItem (strCategories, s_szTag_Category, strCategoryTag);
	}
	CBCGPTagManager::WriteItem (strTag, s_szTag_Categories, strCategories);
}

BOOL CBCGPBaseRibbonInfo::XCategoryMain::FromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	CString strElements;
	if (tm.ExcludeTag (s_szTag_Elements, strElements))
	{
		CBCGPTagManager tmElements (strElements);

		CString strElement;
		while (tmElements.ExcludeTag (s_szTag_Element, strElement))
		{
			XElement* pElement = (XElement*)CBCGPBaseRibbonInfo::CreateBaseFromTag (strElement);
			if (pElement != NULL)
			{
				m_arElements.Add (pElement);
			}
		}
	}

	CString strRecent;
	if (tm.ExcludeTag (s_szTag_RecentFileList, strRecent))
	{
		CBCGPTagManager tmRecent (strRecent);

		tmRecent.ReadBool (s_szTag_Enable, m_bRecentListEnable);
		tmRecent.ReadEntityString (s_szTag_Label, m_strRecentListLabel);
		tmRecent.ReadInt (s_szTag_Width, m_nRecentListWidth);
		tmRecent.ReadBool (s_szTag_ShowPins, m_bRecentListShowPins);
	}

	CString strSearch;
	if (tm.ExcludeTag (s_szTag_Search, strSearch))
	{
		CBCGPTagManager tmSearch (strSearch);

		tmSearch.ReadBool (s_szTag_Enable, m_bSearchEnable);
		tmSearch.ReadEntityString (s_szTag_Label, m_strSearchLabel);
		tmSearch.ReadEntityString (s_szTag_Keys, m_strSearchKeys);
		tmSearch.ReadInt (s_szTag_Width, m_nSearchWidth);
	}

	tm.ReadEntityString (s_szTag_Name, m_strName);

    CString strImage;
    if (tm.ExcludeTag(s_szTag_Image_Small, strImage))
    {
        m_SmallImages.FromTag(strImage);
    }
    if (tm.ExcludeTag(s_szTag_Image_Large, strImage))
    {
        m_LargeImages.FromTag(strImage);
    }

	return TRUE;
}

void CBCGPBaseRibbonInfo::XCategoryMain::ToTag (CString& strTag) const
{
	if (m_arElements.GetSize () == 0 && !m_bRecentListEnable)
	{
		return;
	}

	XBase::ToTag (strTag);

	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteEntityString (s_szTag_Name, m_strName));

	CString strImage;
	m_SmallImages.ToTag (strImage);
	CBCGPTagManager::WriteItem (strTag, s_szTag_Image_Small, strImage);

	strImage.Empty ();
	m_LargeImages.ToTag (strImage);
	CBCGPTagManager::WriteItem (strTag, s_szTag_Image_Large, strImage);

	CString strElements;

	int i = 0;
	for (i = 0; i < (int)m_arElements.GetSize (); i++)
	{
		CString strElement;
		m_arElements[i]->ToTag (strElement);
		CBCGPTagManager::WriteItem (strElements, s_szTag_Element, strElement);
	}

	CBCGPTagManager::WriteItem (strTag, s_szTag_Elements, strElements);

	CString strRecent;
	CBCGPTagManager::WriteTag (strRecent, CBCGPTagManager::WriteBool (s_szTag_Enable, m_bRecentListEnable, FALSE));
	CBCGPTagManager::WriteTag (strRecent, CBCGPTagManager::WriteEntityString (s_szTag_Label, m_strRecentListLabel));
	CBCGPTagManager::WriteTag (strRecent, CBCGPTagManager::WriteInt (s_szTag_Width, m_nRecentListWidth, 300));
	CBCGPTagManager::WriteTag (strRecent, CBCGPTagManager::WriteBool (s_szTag_ShowPins, m_bRecentListShowPins, FALSE));

	CBCGPTagManager::WriteItem (strTag, s_szTag_RecentFileList, strRecent);

	CString strSearch;
	CBCGPTagManager::WriteTag (strSearch, CBCGPTagManager::WriteBool (s_szTag_Enable, m_bSearchEnable, FALSE));
	CBCGPTagManager::WriteTag (strSearch, CBCGPTagManager::WriteEntityString (s_szTag_Label, m_strSearchLabel));
	CBCGPTagManager::WriteTag (strSearch, CBCGPTagManager::WriteEntityString (s_szTag_Keys, m_strSearchKeys));
	CBCGPTagManager::WriteTag (strSearch, CBCGPTagManager::WriteInt (s_szTag_Width, m_nSearchWidth, 0));

	CBCGPTagManager::WriteItem (strTag, s_szTag_Search, strSearch);
}

BOOL CBCGPBaseRibbonInfo::XCategoryBackstage::FromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	CString strElements;
	if (tm.ExcludeTag (s_szTag_Elements, strElements))
	{
		CBCGPTagManager tmElements (strElements);

		CString strElement;
		while (tmElements.ExcludeTag (s_szTag_Element, strElement))
		{
			XElement* pElement = (XElement*)CBCGPBaseRibbonInfo::CreateBaseFromTag (strElement);
			if (pElement != NULL)
			{
				m_arElements.Add (pElement);
			}
		}
	}

	tm.ReadEntityString (s_szTag_Name, m_strName);

    CString strImage;
    if (tm.ExcludeTag(s_szTag_Image_Small, strImage))
    {
        m_SmallImages.FromTag(strImage);
    }

	return TRUE;
}

void CBCGPBaseRibbonInfo::XCategoryBackstage::ToTag (CString& strTag) const
{
	if (m_arElements.GetSize () == 0)
	{
		return;
	}

	XBase::ToTag (strTag);

	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteEntityString (s_szTag_Name, m_strName));

	CString strImage;
	m_SmallImages.ToTag (strImage);
	CBCGPTagManager::WriteItem (strTag, s_szTag_Image_Small, strImage);

	CString strElements;

	int i = 0;
	for (i = 0; i < (int)m_arElements.GetSize (); i++)
	{
		CString strElement;
		m_arElements[i]->ToTag (strElement);
		CBCGPTagManager::WriteItem (strElements, s_szTag_Element, strElement);
	}

	CBCGPTagManager::WriteItem (strTag, s_szTag_Elements, strElements);
}

BOOL CBCGPBaseRibbonInfo::XQAT::XQATItem::FromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	CString strID;
	if (tm.ExcludeTag (s_szTag_ID, strID))
	{
		if (m_ID.FromTag (strID))
		{
			tm.ReadBool (s_szTag_Visible, m_bVisible);
			return TRUE;
		}
	}

	return TRUE;
}

void CBCGPBaseRibbonInfo::XQAT::XQATItem::ToTag (CString& strTag) const
{
	CString strID;
	m_ID.ToTag (strID);
	CBCGPTagManager::WriteItem (strTag, s_szTag_ID, strID);

	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBool (s_szTag_Visible, m_bVisible, TRUE));
}

BOOL CBCGPBaseRibbonInfo::XQAT::FromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	CString strItem;
	while (tm.ExcludeTag (s_szTag_Item, strItem))
	{
		XQATItem item;
		if (item.FromTag (strItem))
		{
			m_arItems.Add (item);
		}
	}

	tm.ReadBool (s_szTag_QATTop, m_bOnTop);

	return TRUE;
}

void CBCGPBaseRibbonInfo::XQAT::ToTag (CString& strTag) const
{
	XBase::ToTag (strTag);

	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBool (s_szTag_QATTop, m_bOnTop, TRUE));

	CString strItems;
	for (int i = 0; i < (int)m_arItems.GetSize (); i++)
	{
		CString strItem;
		m_arItems[i].ToTag (strItem);

		CBCGPTagManager::WriteItem (strItems, s_szTag_Item, strItem);
	}

	CBCGPTagManager::WriteItem (strTag, s_szTag_Items, strItems);
}

BOOL CBCGPBaseRibbonInfo::XRibbonBar::FromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	CString strButton;
	if (tm.ExcludeTag (s_szTag_Button_Main, strButton))
	{
		m_btnMain = new XElementButtonMain;
		m_btnMain->FromTag (strButton);
	}

	CString strMainCategory;
	if (tm.ExcludeTag (s_szTag_CategoryMain, strMainCategory))
	{
		m_MainCategory = new XCategoryMain;
		m_MainCategory->FromTag (strMainCategory);
	}

	CString strBackstageCategory;
	if (tm.ExcludeTag (s_szTag_CategoryBackstage, strBackstageCategory))
	{
		m_BackstageCategory = new XCategoryBackstage;
		m_BackstageCategory->FromTag (strBackstageCategory);
	}

	CString strQATElements;
	if (tm.ExcludeTag (s_szTag_QAT_Elements, strQATElements))
	{
		m_QAT.FromTag (strQATElements);
	}

	CString strTabElements;
	if (tm.ExcludeTag (s_szTag_Tab_Elements, strTabElements))
	{
		m_TabElements.FromTag (strTabElements);
	}

	CString strContexts;
	if (tm.ExcludeTag (s_szTag_Contexts, strContexts))
	{
		CBCGPTagManager tmElements (strContexts);

		CString strContext;
		while (tmElements.ExcludeTag (s_szTag_Context, strContext))
		{
			XContext* pContext = (XContext*)CBCGPBaseRibbonInfo::CreateBaseFromTag (strContext);
			if (pContext != NULL)
			{
				m_arContexts.Add (pContext);
			}
		}
	}

	CString strCategories;
	if (tm.ExcludeTag (s_szTag_Categories, strCategories))
	{
		CBCGPTagManager tmElements (strCategories);

		CString strCategory;
		while (tmElements.ExcludeTag (s_szTag_Category, strCategory))
		{
			XCategory* pCategory = (XCategory*)CBCGPBaseRibbonInfo::CreateBaseFromTag (strCategory);
			if (pCategory != NULL)
			{
				m_arCategories.Add (pCategory);
			}
		}
	}

	tm.ReadBool (s_szTag_EnableToolTips, m_bToolTip);
	tm.ReadBool (s_szTag_EnableToolTipsDescr, m_bToolTipDescr);
	tm.ReadBool (s_szTag_EnableKeys, m_bKeyTips);
	tm.ReadBool (s_szTag_EnablePrintPreview, m_bPrintPreview);
	tm.ReadBool (s_szTag_DrawUsingFont, m_bDrawUsingFont);
	tm.ReadBool (s_szTag_EnableBackstageMode, m_bBackstageMode);

    CString strImage;
    if (tm.ExcludeTag(s_szTag_Image, strImage))
    {
        m_Images.FromTag(strImage);
    }

	return TRUE;
}

void CBCGPBaseRibbonInfo::XRibbonBar::ToTag (CString& strTag) const
{
	XBase::ToTag (strTag);

	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBool (s_szTag_EnableToolTips, m_bToolTip, TRUE));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBool (s_szTag_EnableToolTipsDescr, m_bToolTipDescr, TRUE));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBool (s_szTag_EnableKeys, m_bKeyTips, TRUE));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBool (s_szTag_EnablePrintPreview, m_bPrintPreview, TRUE));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBool (s_szTag_DrawUsingFont, m_bDrawUsingFont, FALSE));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBool (s_szTag_EnableBackstageMode, m_bBackstageMode, FALSE));

	CString strImage;
	m_Images.ToTag (strImage);
	CBCGPTagManager::WriteItem (strTag, s_szTag_Image, strImage);

	if (m_btnMain != NULL)
	{
		CString strButton;
		m_btnMain->ToTag (strButton);
		CBCGPTagManager::WriteItem (strTag, s_szTag_Button_Main, strButton);
	}

	if (m_MainCategory != NULL)
	{
		CString strMainCategory;
		m_MainCategory->ToTag (strMainCategory);
		CBCGPTagManager::WriteItem (strTag, s_szTag_CategoryMain, strMainCategory);
	}

	if (m_BackstageCategory != NULL)
	{
		CString strBackstageCategory;
		m_BackstageCategory->ToTag (strBackstageCategory);
		CBCGPTagManager::WriteItem (strTag, s_szTag_CategoryBackstage, strBackstageCategory);
	}

	int i = 0;

	CString strQATElements;
	m_QAT.ToTag (strQATElements);
	CBCGPTagManager::WriteItem (strTag, s_szTag_QAT_Elements, strQATElements);

	CString strTabElements;
	m_TabElements.ToTag (strTabElements);
	CBCGPTagManager::WriteItem (strTag, s_szTag_Tab_Elements, strTabElements);

	CString strCategories;
	for (i = 0; i < (int)m_arCategories.GetSize (); i++)
	{
		CString strCategory;
		m_arCategories[i]->ToTag (strCategory);
		CBCGPTagManager::WriteItem (strCategories, s_szTag_Category, strCategory);
	}
	CBCGPTagManager::WriteItem (strTag, s_szTag_Categories, strCategories);

	CString strContexts;
	for (i = 0; i < (int)m_arContexts.GetSize (); i++)
	{
		CString strContext;
		m_arContexts[i]->ToTag (strContext);
		CBCGPTagManager::WriteItem (strContexts, s_szTag_Context, strContext);
	}
	CBCGPTagManager::WriteItem (strTag, s_szTag_Contexts, strContexts);
}

BOOL CBCGPBaseRibbonInfo::XStatusBar::XStatusElements::FromTag (const CString& strTag)
{
	CBCGPTagManager tmElements (strTag);

	CString strElement;
	while (tmElements.ExcludeTag (s_szTag_Element, strElement))
	{
		XElement* pElement = (XElement*)CBCGPBaseRibbonInfo::CreateBaseFromTag (strElement);
		if (pElement != NULL)
		{
			m_arElements.Add (pElement);

			CBCGPTagManager tmElement (strElement);

			CString strLabel;
			tmElement.ReadEntityString (s_szTag_Label, strLabel);
			m_arLabels.Add (strLabel);

			BOOL bVisible = TRUE;
			tmElement.ReadBool (s_szTag_Visible, bVisible);
			m_arVisible.Add (bVisible);
		}
	}

	return TRUE;
}

void CBCGPBaseRibbonInfo::XStatusBar::XStatusElements::ToTag (CString& strTag) const
{
	int i = 0;
	for (i = 0; i < (int)m_arElements.GetSize (); i++)
	{
		CString strElement;
		m_arElements[i]->ToTag (strElement);

		CBCGPTagManager::WriteTag (strElement, CBCGPTagManager::WriteEntityString (s_szTag_Label, m_arLabels[i]));
		CBCGPTagManager::WriteTag (strElement, CBCGPTagManager::WriteBool (s_szTag_Visible, m_arVisible[i], TRUE));

		CBCGPTagManager::WriteItem (strTag, s_szTag_Element, strElement);
	}
}

BOOL CBCGPBaseRibbonInfo::XStatusBar::FromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	CString strExElements;
	if (tm.ExcludeTag (s_szTag_ElementsExtended, strExElements))
	{
		m_ExElements.FromTag (strExElements);
	}

	CString strElements;
	if (tm.ExcludeTag (s_szTag_Elements, strElements))
	{
		m_Elements.FromTag (strElements);
	}

    CString strImage;
    if (tm.ExcludeTag(s_szTag_Image, strImage))
    {
        m_Images.FromTag(strImage);
    }

	return TRUE;
}

void CBCGPBaseRibbonInfo::XStatusBar::ToTag (CString& strTag) const
{
	XBase::ToTag (strTag);

	CString strImage;
	m_Images.ToTag (strImage);
	CBCGPTagManager::WriteItem (strTag, s_szTag_Image, strImage);

	CString strElements;
	m_Elements.ToTag (strElements);
	CBCGPTagManager::WriteItem (strTag, s_szTag_Elements, strElements);

	CString strExElements;
	m_ExElements.ToTag (strExElements);
	CBCGPTagManager::WriteItem (strTag, s_szTag_ElementsExtended, strExElements);
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGPBaseRibbonInfo::CBCGPBaseRibbonInfo(DWORD dwVersion, DWORD dwFlags)
	: CBCGPBaseInfo (dwVersion, dwFlags)
{
}

CBCGPBaseRibbonInfo::~CBCGPBaseRibbonInfo()
{
}


CBCGPRibbonInfo::CBCGPRibbonInfo()
	: CBCGPBaseRibbonInfo(c_dwVersion, e_UseRibbon | e_UseStatus)
{
	m_sizeImage[e_ImagesSmall]   = CSize(16, 16);
	m_sizeImage[e_ImagesLarge]   = CSize(32, 32);
	m_sizeImage[e_ImagesSBGroup] = CSize(14, 14);
	
	m_RibbonBar.m_Images.m_Image.SetImageSize (m_sizeImage [e_ImagesSmall]);
	m_StatusBar.m_Images.m_Image.SetImageSize (m_sizeImage [e_ImagesSmall]);
}

CBCGPRibbonInfo::~CBCGPRibbonInfo()
{
}

BOOL CBCGPRibbonInfo::FromTag (const CString& strTag)
{
	DWORD dwFlags = GetFlags ();

	if ((dwFlags & (e_UseRibbon | e_UseStatus)) == 0)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	CString strXML;
	{
		CBCGPTagManager tmXML (strTag);

		if (!tmXML.ExcludeTag (s_szTag_Body, strXML))
		{
			if (!tmXML.ExcludeTag (_T("AFX_RIBBON"), strXML))
			{
				return FALSE;
			}
		}
	}

	CBCGPTagManager tm (strXML);

	CString strHeader;
	if (tm.ExcludeTag (CBCGPBaseInfo::s_szTag_Header, strHeader))
	{
		CBCGPTagManager tmHeader (strHeader);

		DWORD dwValue = GetVersion ();
		tmHeader.ReadDword (CBCGPBaseInfo::s_szTag_Version, dwValue);
		SetVersion (dwValue);

		dwValue = GetVersionStamp ();
		tmHeader.ReadDword (CBCGPBaseInfo::s_szTag_VersionStamp, dwValue);
		SetVersionStamp (dwValue);

		CString strSizes;
		if (tmHeader.ExcludeTag (s_szTag_Sizes, strSizes))
		{
			CBCGPTagManager tmSizes (strSizes);

			tmSizes.ReadSize (s_szTag_Image_Small, m_sizeImage[e_ImagesSmall]);
			tmSizes.ReadSize (s_szTag_Image_Large, m_sizeImage[e_ImagesLarge]);
			tmSizes.ReadSize (s_szTag_Image_SBGroup, m_sizeImage[e_ImagesSBGroup]);
		}
	}
	else
	{
		return FALSE;
	}

	BOOL bRibbon = FALSE;
	if ((dwFlags & e_UseRibbon) == e_UseRibbon)
	{
		CString strRibbonBar;
		if (tm.ExcludeTag (s_szTag_RibbonBar, strRibbonBar))
		{
			bRibbon = m_RibbonBar.FromTag (strRibbonBar);
		}
	}

	BOOL bStatus = FALSE;
	if ((dwFlags & e_UseStatus) == e_UseStatus)
	{
		CString strStatusBar;
		if (tm.ExcludeTag (s_szTag_StatusBar, strStatusBar))
		{
			bStatus = m_StatusBar.FromTag (strStatusBar);
		}
	}

	if (bRibbon)
	{
		m_RibbonBar.m_Images.m_Image.SetImageSize (m_sizeImage [e_ImagesSmall]);

		if (m_RibbonBar.m_MainCategory != NULL)
		{
			m_RibbonBar.m_MainCategory->m_SmallImages.m_Image.SetImageSize (m_sizeImage [e_ImagesSmall]);
			m_RibbonBar.m_MainCategory->m_LargeImages.m_Image.SetImageSize (m_sizeImage [e_ImagesLarge]);
		}

		if (m_RibbonBar.m_BackstageCategory != NULL)
		{
			m_RibbonBar.m_BackstageCategory->m_SmallImages.m_Image.SetImageSize (m_sizeImage [e_ImagesSmall]);
		}

		XArrayCategory arCategories;
		arCategories.Append (m_RibbonBar.m_arCategories);

		int i = 0;
		for (i = 0; i < (int)m_RibbonBar.m_arContexts.GetSize (); i++)
		{
			arCategories.Append (m_RibbonBar.m_arContexts [i]->m_arCategories);
		}

		for (i = 0; i < (int)arCategories.GetSize (); i++)
		{
			XCategory* pCategory = arCategories[i];

			pCategory->m_SmallImages.m_Image.SetImageSize (m_sizeImage [e_ImagesSmall]);
			pCategory->m_LargeImages.m_Image.SetImageSize (m_sizeImage [e_ImagesLarge]);

			for (int j = 0; j < (int)pCategory->m_arPanels.GetSize (); j++)
			{
				XPanel* pPanel = pCategory->m_arPanels[j];

				for (int k = 0; k < (int)pPanel->m_arElements.GetSize (); k++)
				{
					XElement* pElement = pPanel->m_arElements[k];

					if (pElement->GetElementName ().Compare (s_szGroup) == 0)
					{
						((XElementGroup*)pElement)->m_Images.m_Image.SetImageSize (m_sizeImage [e_ImagesSmall]);
					}
				}
			}
		}
	}

	if (bStatus)
	{
		m_StatusBar.m_Images.m_Image.SetImageSize (m_sizeImage [e_ImagesSmall]);

		int i = 0;
		for (i = 0; i < (int)m_StatusBar.m_Elements.m_arElements.GetSize (); i++)
		{
			if (m_StatusBar.m_Elements.m_arElements[i]->GetElementName ().Compare (s_szGroup) == 0)
			{
				((XElementGroup*)m_StatusBar.m_Elements.m_arElements[i])->m_Images.m_Image.SetImageSize (m_sizeImage [e_ImagesSBGroup]);
			}
		}
		for (i = 0; i < (int)m_StatusBar.m_ExElements.m_arElements.GetSize (); i++)
		{
			if (m_StatusBar.m_ExElements.m_arElements[i]->GetElementName ().Compare (s_szGroup) == 0)
			{
				((XElementGroup*)m_StatusBar.m_ExElements.m_arElements[i])->m_Images.m_Image.SetImageSize (m_sizeImage [e_ImagesSBGroup]);
			}
		}
	}

	return TRUE;
}

void CBCGPRibbonInfo::ToTag (CString& strTag) const
{
	DWORD dwFlags = GetFlags ();

	if ((dwFlags & (e_UseRibbon | e_UseStatus)) == 0)
	{
		ASSERT (FALSE);
		return;
	}

	CString strData;

	CString strSizes;
	CBCGPTagManager::WriteTag (strSizes, CBCGPTagManager::WriteSize (s_szTag_Image_Small, m_sizeImage[e_ImagesSmall], CSize (16, 16)));
	CBCGPTagManager::WriteTag (strSizes, CBCGPTagManager::WriteSize (s_szTag_Image_Large, m_sizeImage[e_ImagesLarge], CSize (32, 32)));
	CBCGPTagManager::WriteTag (strSizes, CBCGPTagManager::WriteSize (s_szTag_Image_SBGroup, m_sizeImage[e_ImagesSBGroup], CSize (14, 14)));

	CString strHeader;
	CBCGPTagManager::WriteTag (strHeader, CBCGPTagManager::WriteDword (CBCGPBaseInfo::s_szTag_Version, GetVersion (), 0));
	CBCGPTagManager::WriteTag (strHeader, CBCGPTagManager::WriteDword (CBCGPBaseInfo::s_szTag_VersionStamp, GetVersionStamp (), 0));
	CBCGPTagManager::WriteItem (strHeader, s_szTag_Sizes, strSizes);

	CBCGPTagManager::WriteItem (strData, CBCGPBaseInfo::s_szTag_Header, strHeader);

	if ((dwFlags & e_UseRibbon) == e_UseRibbon)
	{
		CString strRibbonBar;
		m_RibbonBar.ToTag (strRibbonBar);
		CBCGPTagManager::WriteItem (strData, s_szTag_RibbonBar, strRibbonBar);
	}

	if ((dwFlags & e_UseStatus) == e_UseStatus)
	{
		CString strStatusBar;
		m_StatusBar.ToTag (strStatusBar);
		CBCGPTagManager::WriteItem (strData, s_szTag_StatusBar, strStatusBar);
	}

	CBCGPTagManager::WriteItem (strTag, s_szTag_Body, strData);
}

void CBCGPRibbonInfo::AddElementImages (XElement& info, XArrayImages& images)
{
	CString strName (info.GetElementName ());
	BOOL bPalette = FALSE;

	if (strName.Compare (s_szButton_Palette) == 0)
	{
		images.Add(&((XElementButtonPalette&)info).m_Images);
		bPalette = TRUE;
	}
	else if (info.GetElementName ().Compare (s_szGroup) == 0)
	{
		XElementGroup& infoGroup = (XElementGroup&)info;
		images.Add(&(infoGroup.m_Images));

		for (int i = 0; i < (int)infoGroup.m_arButtons.GetSize (); i++)
		{
			AddElementImages (*(infoGroup.m_arButtons[i]), images);
		}
	}

	if (strName.Compare (s_szButton) == 0 || strName.Compare (s_szButton_Color) == 0 || bPalette)
	{
		XElementButton& infoBtn = (XElementButton&)info;

		for (int i = 0; i < (int)infoBtn.m_arSubItems.GetSize(); i++)
		{
			AddElementImages (*((XElement*)(infoBtn.m_arSubItems[i])), images);
		}
	}
}

void CBCGPRibbonInfo::GetArrayImages (XArrayImages& images)
{
	images.RemoveAll ();

	images.Add (&m_RibbonBar.m_Images);

	if (m_RibbonBar.m_btnMain != NULL)
	{
		images.Add (&m_RibbonBar.m_btnMain->m_Image);
		images.Add (&m_RibbonBar.m_btnMain->m_ImageScenic);
	}

	if (m_RibbonBar.m_MainCategory != NULL)
	{
		images.Add (&m_RibbonBar.m_MainCategory->m_SmallImages);
		images.Add (&m_RibbonBar.m_MainCategory->m_LargeImages);
	}

	if (m_RibbonBar.m_BackstageCategory != NULL)
	{
		images.Add (&m_RibbonBar.m_BackstageCategory->m_SmallImages);
	}

	int i = 0;
	int j = 0;
	int k = 0;

	for (i = 0; i < (int)m_RibbonBar.m_TabElements.m_arButtons.GetSize (); i++)
	{
		AddElementImages (*(m_RibbonBar.m_TabElements.m_arButtons[i]), images);
	}

	XArrayCategory arCategories;
	arCategories.Append(m_RibbonBar.m_arCategories);

	for (i = 0; i < (int)m_RibbonBar.m_arContexts.GetSize(); i++)
	{
		arCategories.Append(m_RibbonBar.m_arContexts[i]->m_arCategories);
	}
	
	for (i = 0; i < (int)arCategories.GetSize(); i++)
	{
		XCategory* pCategory = arCategories[i];

		images.Add(&pCategory->m_SmallImages);
		images.Add(&pCategory->m_LargeImages);

		for (j = 0; j < (int)pCategory->m_arPanels.GetSize(); j++)
		{
			XPanel* pPanel = pCategory->m_arPanels[j];

			for (k = 0; k < (int)pPanel->m_arElements.GetSize(); k++)
			{
				AddElementImages (*(pPanel->m_arElements[k]), images);
			}
		}
	}

	images.Add (&m_StatusBar.m_Images);

	for (i = 0; i < (int)m_StatusBar.m_Elements.m_arElements.GetSize (); i++)
	{
		if (m_StatusBar.m_Elements.m_arElements[i]->GetElementName ().Compare (s_szGroup) == 0)
		{
			images.Add (&((XElementGroup*)m_StatusBar.m_Elements.m_arElements[i])->m_Images);
		}
	}
	for (i = 0; i < (int)m_StatusBar.m_ExElements.m_arElements.GetSize (); i++)
	{
		if (m_StatusBar.m_ExElements.m_arElements[i]->GetElementName ().Compare (s_szGroup) == 0)
		{
			images.Add (&((XElementGroup*)m_StatusBar.m_ExElements.m_arElements[i])->m_Images);
		}
	}
}


CBCGPRibbonCustomizationInfo::CBCGPRibbonCustomizationInfo()
	: CBCGPBaseRibbonInfo(c_dwVersion, 0)
{
}

CBCGPRibbonCustomizationInfo::~CBCGPRibbonCustomizationInfo()
{
}

CBCGPRibbonCustomizationInfo::XCustomElement::XCustomElement()
	: m_nID        (0)
	, m_nCustomIcon(-1)
{
}

CBCGPRibbonCustomizationInfo::XCustomElement::~XCustomElement()
{
}

BOOL CBCGPRibbonCustomizationInfo::XCustomElement::FromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	tm.ReadUInt (s_szTag_ID, m_nID);
	tm.ReadEntityString (s_szTag_Text, m_strName);
	tm.ReadInt (s_szTag_Image, m_nCustomIcon);

	return TRUE;
}

void CBCGPRibbonCustomizationInfo::XCustomElement::ToTag (CString& strTag) const
{
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteUInt (s_szTag_ID, m_nID, 0));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteEntityString (s_szTag_Text, m_strName));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteInt (s_szTag_Image, m_nCustomIcon, -1));
}

CBCGPRibbonCustomizationInfo::XCustomPanel::XCustomPanel()
	: m_nKey      (0)
	, m_nID       (0)
	, m_nIndex    (-1)
	, m_dwOriginal((DWORD)-1)
{
}

CBCGPRibbonCustomizationInfo::XCustomPanel::~XCustomPanel()
{
	for (int i = 0; i < (int)m_arElements.GetSize (); i++)
	{
		if (m_arElements[i] != NULL)
		{
			delete m_arElements[i];
		}
	}
}

BOOL CBCGPRibbonCustomizationInfo::XCustomPanel::FromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	CString strElements;
	if (tm.ExcludeTag (s_szTag_Elements, strElements))
	{
		CBCGPTagManager tmElement (strElements);

		CString strElement;
		while (tmElement.ExcludeTag (s_szTag_Element, strElement))
		{
			XCustomElement* pElement = new XCustomElement;
			if (pElement->FromTag (strElement))
			{
				m_arElements.Add (pElement);
			}
			else
			{
				delete pElement;
			}
		}
	}

	tm.ReadUInt (s_szTag_ID, m_nID);
	tm.ReadInt (s_szTag_Key, m_nKey);
	tm.ReadEntityString (s_szTag_Text, m_strName);
	tm.ReadInt (s_szTag_Index, m_nIndex);
	tm.ReadDword (s_szTag_Original, m_dwOriginal);

	return TRUE;
}

void CBCGPRibbonCustomizationInfo::XCustomPanel::ToTag (CString& strTag) const
{
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteUInt (s_szTag_ID, m_nID, 0));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteInt (s_szTag_Key, m_nKey, 0));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteEntityString (s_szTag_Text, m_strName));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteInt (s_szTag_Index, m_nIndex, -1));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteDword (s_szTag_Original, m_dwOriginal, (DWORD)-1));

	CString strElements;
	for (int i = 0; i < (int)m_arElements.GetSize (); i++)
	{
		CString strElement;
		m_arElements[i]->ToTag (strElement);
		CBCGPTagManager::WriteItem (strElements, s_szTag_Element, strElement);
	}
	CBCGPTagManager::WriteItem (strTag, s_szTag_Elements, strElements);
}

CBCGPRibbonCustomizationInfo::XCustomCategory::XCustomCategory()
	: m_nKey       (0)
	, m_uiContextID(0)
{
}

CBCGPRibbonCustomizationInfo::XCustomCategory::~XCustomCategory()
{
	for (int i = 0; i < (int)m_arPanels.GetSize (); i++)
	{
		if (m_arPanels[i] != NULL)
		{
			delete m_arPanels[i];
		}
	}
}

BOOL CBCGPRibbonCustomizationInfo::XCustomCategory::FromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	CString strPanels;
	if (tm.ExcludeTag (s_szTag_Panels, strPanels))
	{
		CBCGPTagManager tmPanel (strPanels);

		CString strPanel;
		while (tmPanel.ExcludeTag (s_szTag_Panel, strPanel))
		{
			XCustomPanel* pPanel = new XCustomPanel;
			if (pPanel->FromTag (strPanel))
			{
				m_arPanels.Add (pPanel);
			}
			else
			{
				delete pPanel;
			}
		}
	}

	tm.ReadUInt (s_szTag_Context, m_uiContextID);
	tm.ReadInt (s_szTag_Key, m_nKey);
	tm.ReadEntityString (s_szTag_Text, m_strName);

	return TRUE;
}

void CBCGPRibbonCustomizationInfo::XCustomCategory::ToTag (CString& strTag) const
{
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteUInt (s_szTag_Context, m_uiContextID, 0));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteInt (s_szTag_Key, m_nKey, 0));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteEntityString (s_szTag_Text, m_strName));

	CString strPanels;
	for (int i = 0; i < (int)m_arPanels.GetSize (); i++)
	{
		CString strPanel;
		m_arPanels[i]->ToTag (strPanel);
		CBCGPTagManager::WriteItem (strPanels, s_szTag_Panel, strPanel);
	}
	CBCGPTagManager::WriteItem (strTag, s_szTag_Panels, strPanels);
}

CBCGPRibbonCustomizationInfo::XDataCategory::XDataCategory()
{
}

CBCGPRibbonCustomizationInfo::XDataCategory::~XDataCategory()
{
	for (int i = 0; i < (int)m_arCustom.GetSize (); i++)
	{
		if (m_arCustom[i] != NULL)
		{
			delete m_arCustom[i];
		}
	}
}

BOOL CBCGPRibbonCustomizationInfo::XDataCategory::FromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	CString strItems;

	if (tm.ExcludeTag (s_szTag_Custom, strItems))
	{
		CBCGPTagManager tmItems (strItems);

		CString strItem;
		while (tmItems.ExcludeTag (s_szTag_Category, strItem))
		{
			XCustomCategory* pCustom = new XCustomCategory;
			if (pCustom->FromTag (strItem))
			{
				m_arCustom.Add (pCustom);
			}
			else
			{
				delete pCustom;
			}
		}
	}

	strItems.Empty ();
	if (tm.ExcludeTag (s_szTag_Hidden, strItems))
	{
		CBCGPTagManager tmItems (strItems);

		CString strItem;
		while (tmItems.ExcludeTag (s_szTag_Item, strItem))
		{
			m_arHidden.Add(_ttoi(strItem));
		}
	}

	strItems.Empty ();
	if (tm.ExcludeTag (s_szTag_Index, strItems))
	{
		CBCGPTagManager tmItems (strItems);

		CString strItem;
		while (tmItems.ExcludeTag (s_szTag_Item, strItem))
		{
			CBCGPTagManager tmItem (strItem);

			int key = 0;
			tmItem.ReadInt (s_szTag_KeyMap, key);

			int value;
			tmItem.ReadInt (s_szTag_Value, value);

			m_mapIndexes[key] = value;
		}
	}

	strItems.Empty ();
	if (tm.ExcludeTag (s_szTag_Names, strItems))
	{
		CBCGPTagManager tmItems (strItems);

		CString strItem;
		while (tmItems.ExcludeTag (s_szTag_Item, strItem))
		{
			CBCGPTagManager tmItem (strItem);

			int key = 0;
			tmItem.ReadInt (s_szTag_KeyMap, key);

			CString value;
			tmItem.ReadEntityString (s_szTag_Value, value);

			m_mapNames[key] = value;
		}
	}

	return TRUE;
}

void CBCGPRibbonCustomizationInfo::XDataCategory::ToTag (CString& strTag) const
{
	int i = 0;
	POSITION pos = NULL;

	CString strItems;
	for (i = 0; i < (int)m_arHidden.GetSize (); i++)
	{
		CBCGPTagManager::WriteTag (strItems, CBCGPTagManager::WriteInt (s_szTag_Item, m_arHidden[i], -111111));
	}
	CBCGPTagManager::WriteItem (strTag, s_szTag_Hidden, strItems);

	strItems.Empty ();
	pos = m_mapIndexes.GetStartPosition ();
	while (pos != NULL)
	{
		int key = 0;
		int value = 0;

		m_mapIndexes.GetNextAssoc (pos, key, value);

		CString strItem;
		CBCGPTagManager::WriteTag (strItem, CBCGPTagManager::WriteInt (s_szTag_KeyMap, key, -111111));
		CBCGPTagManager::WriteTag (strItem, CBCGPTagManager::WriteInt (s_szTag_Value, value, -111111));
		CBCGPTagManager::WriteItem (strItems, s_szTag_Item, strItem);
	}
	CBCGPTagManager::WriteItem (strTag, s_szTag_Index, strItems);

	strItems.Empty ();
	pos = m_mapNames.GetStartPosition ();
	while (pos != NULL)
	{
		int key = 0;
		CString value;

		m_mapNames.GetNextAssoc (pos, key, value);

		CString strItem;
		CBCGPTagManager::WriteTag (strItem, CBCGPTagManager::WriteInt (s_szTag_KeyMap, key, -111111));
		CBCGPTagManager::WriteTag (strItem, CBCGPTagManager::WriteEntityString (s_szTag_Value, value));
		CBCGPTagManager::WriteItem (strItems, s_szTag_Item, strItem);
	}
	CBCGPTagManager::WriteItem (strTag, s_szTag_Names, strItems);

	strItems.Empty ();
	for (i = 0; i < (int)m_arCustom.GetSize (); i++)
	{
		CString strItem;
		m_arCustom[i]->ToTag (strItem);
		CBCGPTagManager::WriteItem (strItems, s_szTag_Category, strItem);
	}
	CBCGPTagManager::WriteItem (strTag, s_szTag_Custom, strItems);
}

CBCGPRibbonCustomizationInfo::XDataPanel::XDataPanel()
{
}

CBCGPRibbonCustomizationInfo::XDataPanel::~XDataPanel()
{
	POSITION pos = m_mapCustom.GetStartPosition ();
	while (pos != NULL)
	{
		DWORD key = 0;
		XCustomPanel* value = NULL;
		m_mapCustom.GetNextAssoc (pos, key, value);

		if (value != NULL)
		{
			delete value;
		}
	}
}

BOOL CBCGPRibbonCustomizationInfo::XDataPanel::FromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	CString strItems;

	if (tm.ExcludeTag (s_szTag_Custom, strItems))
	{
		CBCGPTagManager tmItems (strItems);

		CString strItem;
		while (tmItems.ExcludeTag (s_szTag_Item, strItem))
		{
			CBCGPTagManager tmItem (strItem);

			DWORD key = 0;
			tmItem.ReadDword (s_szTag_KeyMap, key);

			CString value;
			if (tmItem.ExcludeTag (s_szTag_Panel, value))
			{
				XCustomPanel* pCustom = new XCustomPanel;
				if (pCustom->FromTag (value))
				{
					m_mapCustom[key] = pCustom;
				}
				else
				{
					delete pCustom;
				}
			}
		}
	}

	strItems.Empty ();
	if (tm.ExcludeTag (s_szTag_Hidden, strItems))
	{
		CBCGPTagManager tmItems (strItems);

		CString strItem;
		while (tmItems.ExcludeTag (s_szTag_Item, strItem))
		{
			m_arHidden.Add((DWORD)_ttoi(strItem));
		}
	}

	strItems.Empty ();
	if (tm.ExcludeTag (s_szTag_Index, strItems))
	{
		CBCGPTagManager tmItems (strItems);

		CString strItem;
		while (tmItems.ExcludeTag (s_szTag_Item, strItem))
		{
			CBCGPTagManager tmItem (strItem);

			DWORD key = 0;
			tmItem.ReadDword (s_szTag_KeyMap, key);

			int value;
			tmItem.ReadInt (s_szTag_Value, value);

			m_mapIndexes[key] = value;
		}
	}

	strItems.Empty ();
	if (tm.ExcludeTag (s_szTag_Names, strItems))
	{
		CBCGPTagManager tmItems (strItems);

		CString strItem;
		while (tmItems.ExcludeTag (s_szTag_Item, strItem))
		{
			CBCGPTagManager tmItem (strItem);

			DWORD key = 0;
			tmItem.ReadDword (s_szTag_KeyMap, key);

			CString value;
			tmItem.ReadEntityString (s_szTag_Value, value);

			m_mapNames[key] = value;
		}
	}

	return TRUE;
}

void CBCGPRibbonCustomizationInfo::XDataPanel::ToTag (CString& strTag) const
{
	int i = 0;
	POSITION pos = NULL;

	CString strItems;
	for (i = 0; i < (int)m_arHidden.GetSize (); i++)
	{
		CBCGPTagManager::WriteTag (strItems, CBCGPTagManager::WriteDword (s_szTag_Item, m_arHidden[i], (DWORD)-1));
	}
	CBCGPTagManager::WriteItem (strTag, s_szTag_Hidden, strItems);

	strItems.Empty ();
	pos = m_mapIndexes.GetStartPosition ();
	while (pos != NULL)
	{
		DWORD key = 0;
		int value = 0;

		m_mapIndexes.GetNextAssoc (pos, key, value);

		CString strItem;
		CBCGPTagManager::WriteTag (strItem, CBCGPTagManager::WriteDword (s_szTag_KeyMap, key, (DWORD)-1));
		CBCGPTagManager::WriteTag (strItem, CBCGPTagManager::WriteInt (s_szTag_Value, value, -111111));
		CBCGPTagManager::WriteItem (strItems, s_szTag_Item, strItem);
	}
	CBCGPTagManager::WriteItem (strTag, s_szTag_Index, strItems);

	strItems.Empty ();
	pos = m_mapNames.GetStartPosition ();
	while (pos != NULL)
	{
		DWORD key = 0;
		CString value;

		m_mapNames.GetNextAssoc (pos, key, value);

		CString strItem;
		CBCGPTagManager::WriteTag (strItem, CBCGPTagManager::WriteDword (s_szTag_KeyMap, key, (DWORD)-1));
		CBCGPTagManager::WriteTag (strItem, CBCGPTagManager::WriteEntityString (s_szTag_Value, value));
		CBCGPTagManager::WriteItem (strItems, s_szTag_Item, strItem);
	}
	CBCGPTagManager::WriteItem (strTag, s_szTag_Names, strItems);

	strItems.Empty ();
	pos = m_mapCustom.GetStartPosition ();
	while (pos != NULL)
	{
		DWORD key = 0;
		XCustomPanel* value = NULL;

		m_mapCustom.GetNextAssoc (pos, key, value);

		CString strValue;
		value->ToTag (strValue);

		CString strItem;
		CBCGPTagManager::WriteTag (strItem, CBCGPTagManager::WriteDword (s_szTag_KeyMap, key, (DWORD)-1));
		CBCGPTagManager::WriteItem (strItem, s_szTag_Panel, strValue);
		CBCGPTagManager::WriteItem (strItems, s_szTag_Item, strItem);
	}
	CBCGPTagManager::WriteItem (strTag, s_szTag_Custom, strItems);
}

BOOL CBCGPRibbonCustomizationInfo::FromTag (const CString& strTag)
{
	CString strXML;
	{
		CBCGPTagManager tmXML (strTag);

		if (!tmXML.ExcludeTag (s_szTag_Body_Customization, strXML))
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
		tmHeader.ReadDword (CBCGPBaseInfo::s_szTag_Version, dwValue);
		SetVersion (dwValue);

		dwValue = GetVersionStamp ();
		tmHeader.ReadDword (CBCGPBaseInfo::s_szTag_VersionStamp, dwValue);
		SetVersionStamp (dwValue);
	}
	else
	{
		return FALSE;
	}

	CString strDataCategory;
	if (tm.ExcludeTag (s_szTag_Data_Category, strDataCategory))
	{
		m_DataCategory.FromTag (strDataCategory);
	}

	CString strDataPanel;
	if (tm.ExcludeTag (s_szTag_Data_Panel, strDataPanel))
	{
		m_DataPanel.FromTag (strDataPanel);
	}

	return TRUE;
}

void CBCGPRibbonCustomizationInfo::ToTag (CString& strTag) const
{
	CString strData;

	CString strHeader;
	CBCGPTagManager::WriteTag (strHeader, CBCGPTagManager::WriteDword (CBCGPBaseInfo::s_szTag_Version, GetVersion (), 0));
	CBCGPTagManager::WriteTag (strHeader, CBCGPTagManager::WriteDword (CBCGPBaseInfo::s_szTag_VersionStamp, GetVersionStamp (), 0));
	CBCGPTagManager::WriteItem (strData, CBCGPBaseInfo::s_szTag_Header, strHeader);

	CString strDataCategory;
	m_DataCategory.ToTag (strDataCategory);
	CBCGPTagManager::WriteItem (strData, s_szTag_Data_Category, strDataCategory);

	CString strDataPanel;
	m_DataPanel.ToTag (strDataPanel);
	CBCGPTagManager::WriteItem (strData, s_szTag_Data_Panel, strDataPanel);

	CBCGPTagManager::WriteItem (strTag, s_szTag_Body_Customization, strData);
}

#endif // BCGP_EXCLUDE_RIBBON
