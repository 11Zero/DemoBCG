//*******************************************************************************
// COPYRIGHT NOTES
// ---------------
// This is a part of BCGControlBar Library Professional Edition
// Copyright (C) 1998-2014 BCGSoft Ltd.
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.
//*******************************************************************************
// BCGPRibbonBar.h : header file
//

#if !defined(AFX_BCGPRIBBONBAR_H__B3C69A4D_9CDE_406F_9837_2347D306920A__INCLUDED_)
#define AFX_BCGPRIBBONBAR_H__B3C69A4D_9CDE_406F_9837_2347D306920A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define AFX_IDW_RIBBON_BAR		0xE806  // CBCGPRibbonBar
#define RIBBON_CAPTION_BUTTONS	3	

#include "BCGCBPro.h"

#ifndef BCGP_EXCLUDE_RIBBON

#include "BCGPControlBar.h"
#include "BCGPRibbonButton.h"
#include "BCGPRibbonQuickAccessToolbar.h"
#include "BCGPRibbonCustomizationData.h"
#include "BCGPPageTransitionManager.h"

class CBCGPRibbonCategory;
class CBCGPRibbonBar;
class CBCGPRibbonMainButton;
class CBCGPToolbarMenuButton;
class CBCGPRibbonMainPanel;
class CBCGPRibbonBackstageViewPanel;
class CBCGPRibbonEditCtrl;
class CBCGPRibbonKeyTip;
class CBCGPRibbonPanel;
class CBCGPRibbonStatusBar;
class CBCGPRibbonDefaultPanelButton;

/////////////////////////////////////////////////////////////////////////////
// BCGPRibbonCategoryColor

enum BCGPRibbonCategoryColor
{
	BCGPCategoryColor_None,
	BCGPCategoryColor_Red,
	BCGPCategoryColor_Orange,
	BCGPCategoryColor_Yellow,
	BCGPCategoryColor_Green,
	BCGPCategoryColor_Blue,
	BCGPCategoryColor_Indigo,
	BCGPCategoryColor_Violet
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPRibbonCaptionButton

class BCGCBPRODLLEXPORT CBCGPRibbonCaptionButton : public CBCGPRibbonButton
{
	DECLARE_DYNCREATE (CBCGPRibbonCaptionButton)

	friend class CBCGPRibbonBar;

public:
	BOOL IsMDIChildButton () const
	{
		return m_hwndMDIChild != NULL;
	}

protected:
	CBCGPRibbonCaptionButton (UINT uiCmd = 0, HWND hwndMDIChild = NULL);

	virtual void OnDraw (CDC* pDC);
	virtual void OnLButtonUp (CPoint point);

	virtual CSize GetRegularSize (CDC* pDC);

	virtual CSize GetCompactSize (CDC* pDC)
	{
		return GetRegularSize (pDC);
	}

	virtual BOOL IsShowTooltipOnBottom () const	{	return FALSE;	}

	virtual int AddToListBox (CBCGPRibbonCommandsListBox* /*pWndListBox*/, BOOL /*bDeep*/)
	{
		return -1;
	}

	virtual CBCGPGridRow* AddToTree (CBCGPGridCtrl* /*pGrid*/, CBCGPGridRow* /*pParent*/)
	{
		return NULL;
	}

	virtual BOOL CanBeAddedToQAT () const
	{
		return FALSE;
	}
	
	virtual BOOL IsCaptionButton () const { return TRUE; }

	HWND	m_hwndMDIChild;
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPRibbonCaptionCustomButton

class BCGCBPRODLLEXPORT CBCGPRibbonCaptionCustomButton : public CBCGPRibbonCaptionButton
{
	friend class CBCGPRibbonBar;

	DECLARE_DYNCREATE (CBCGPRibbonCaptionCustomButton)

public:
	CBCGPRibbonCaptionCustomButton(UINT uiCmd, HICON hIcon) :
		CBCGPRibbonCaptionButton(uiCmd)
	{
		m_hIconSmall = hIcon;
		m_bSmallIconMode = TRUE;

		UpdateTooltipInfo();
	}
		
protected:
	CBCGPRibbonCaptionCustomButton()
	{
	}
	
	virtual void OnDraw (CDC* pDC);
	virtual void OnLButtonUp (CPoint point);
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPRibbonBackstageCloseButton

class CBCGPRibbonBackstageCloseButton : public CBCGPRibbonButton
{
	DECLARE_DYNAMIC(CBCGPRibbonBackstageCloseButton)

	friend class CBCGPRibbonBar;

	CBCGPRibbonBackstageCloseButton(CBCGPToolBarImages& icon) :
		m_icon(icon)
	{
		m_rectBackstageTopArea.SetRectEmpty();
	}
	
	virtual void OnDraw (CDC* pDC);
	
	virtual int AddToListBox (CBCGPRibbonCommandsListBox* /*pWndListBox*/, BOOL /*bDeep*/)
	{
		return -1;
	}
	
	virtual CBCGPGridRow* AddToTree (CBCGPGridCtrl* /*pGrid*/, CBCGPGridRow* /*pParent*/)
	{
		return NULL;
	}

	void PrepareInvertedIcon();
	void SetCommandsRect(CRect rectBackstageTopArea)
	{
		m_rectBackstageTopArea = rectBackstageTopArea;
	}

	CBCGPToolBarImages&	m_icon;
	CBCGPToolBarImages	m_iconInverted;
	CRect				m_rectBackstageTopArea;
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPRibbonContextCaption

/// <summary>
/// The CBCGPRibbonContextCaption class implements the functionality of colored caption that appears on the top of ribbon context tabs.</summary>
class BCGCBPRODLLEXPORT CBCGPRibbonContextCaption : public CBCGPRibbonButton
{
	DECLARE_DYNCREATE (CBCGPRibbonContextCaption)

	friend class CBCGPRibbonBar;

public:
	/// <summary>
	/// The returned value can be one of the following enumerated values:
	/// BCGPCategoryColor_None
	/// BCGPCategoryColor_Red
	/// BCGPCategoryColor_Orange
	/// BCGPCategoryColor_Yellow
	/// BCGPCategoryColor_Green
	/// BCGPCategoryColor_Blue
	/// BCGPCategoryColor_Indigo
	/// BCGPCategoryColor_Violet
	/// The color can be set using CBCGPRibbonCategory::SetTabColor or CBCGPRibbonBar::AddContextCategory.</summary>
	BCGPRibbonCategoryColor	GetColor () const
	{
		return m_Color;
	}

	int GetRightTabX () const
	{
		return m_nRightTabX;
	}

	UINT GetContextID () const
	{
		return m_uiID;
	}

   int GetContextCategoryCount();

   /// <summary></summary>
   /// <param name="arCategories"></param>
   void GetContextCategories(CArray<CBCGPRibbonCategory*,CBCGPRibbonCategory*>& arCategories);

   /// <summary></summary>
   /// <param name="pContextCaption"></param>
   int GetContextCaptionIndex(CBCGPRibbonContextCaption* pContextCaption);

protected:
	CBCGPRibbonContextCaption (LPCTSTR lpszName, UINT uiID, BCGPRibbonCategoryColor clrContext);
	CBCGPRibbonContextCaption ();

	virtual void OnDraw (CDC* pDC);
	virtual void OnLButtonUp (CPoint point);

	// Accessibility:
	virtual HRESULT get_accParent(IDispatch **ppdispParent);
	virtual HRESULT get_accChildCount(long *pcountChildren);
	virtual HRESULT accDoDefaultAction(VARIANT varChild);
	virtual HRESULT accNavigate(long navDir, VARIANT varStart, VARIANT *pvarEndUpAt);
	virtual BOOL OnSetAccData (long lVal);
	virtual BOOL SetACCData(CWnd* pParent, CBCGPAccessibilityData& data);

	UINT					m_uiID;
	BCGPRibbonCategoryColor	m_Color;
	int						m_nRightTabX;
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPRibbonCustomizationOptions

struct BCGCBPRODLLEXPORT CBCGPRibbonCustomizationOptions
{
	CBCGPRibbonCustomizationOptions()
	{
		m_strNewCategoryLabel = _T("New Tab");
		m_strNewPanelLabel = _T("New Group");
		m_strCustomLabel = _T(" (Custom)");

		m_strNewCategoryKeyTipPrefix = _T("Y");
		m_strNewPanelKeyTipPrefix = _T("Z");
	}

	CString	m_strNewCategoryLabel;
	CString	m_strNewPanelLabel;
	CString	m_strCustomLabel;

	CString	m_strNewCategoryKeyTipPrefix;
	CString	m_strNewPanelKeyTipPrefix;

	CString	m_strCustomImagesPath;
	CString	m_strCustomizationFilePath;		// If empty - save customization data in registry
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPRibbonBar window

#define BCGPRIBBONBAR_HIDE_ELEMENTS	0x0001
#define BCGPRIBBONBAR_HIDE_ALL		0x0002

/// <summary>
/// The CBCGPRibbonBar class implements functionality of "Ribbon" control.</summary>
/// <remarks>
/// "Ribbon" control was introduced by Microsoft in Office 2007. It's not just a new control - it's a new user interface ideology. Ribbon control replaces traditional toolbars and menus with tabbed groups (Categories). Each group is logically split into Panels and each panel may contain various controls and command buttons. In addition, Ribbon control provides smart layout maximally utilizing the available space.  For example, if a Panel has been stretched and has no place to display all available controls, it becomes a menu button which can display sub-items on a popup menu. 
/// Ribbon control behaves as a "static" (non-floating) control bar and can be docked at the top of frame. In fact, the same Ribbon Bar can be used to implement Office 2007-style status bar or any other control that hosts Ribbon Categories (CBCGPRibbonCategory). A Ribbon Category is a logical entity. The visual representation of Category is a Ribbon Tab (CBCGPRibbonTab). A Category contains (and the Tab displays) a group of Ribbon Panels. Each Ribbon Panel contains one or more Ribbon Elements (CBCBPBaseRibbonElement-derived objects).</remarks>
/// <example> This sample shows how to How to add ribbon control to your application.
/// <code>
/// // Open MainFrm.h, remove CBCGPMenuBar m_wndMenuBar and CBCGPToolBar m_wndToolBar.
/// // Add definitions for the Ribbon Bar and Ribbon Main Button:
/// CBCGPRibbonBar m_wndRibbonBar;
/// CBCGPRibbonMainButton m_MainButton; 
///
/// // Add definition for the panels image list:
/// CBCGPToolBarImages m_PanelIcons; 
///
/// // Open MainFrm.cpp and remove everything related to m_wndMenuBar and m_wndToolBar.
///	// Add to the resources a bitmap for the Ribbon Main Button (IDB_MAIN). Use a bitmap 26x26 pixels. Add a bitmap for image list of small icons (16 pixels height) and a bitmap for image list of large icons (32 pixels height). Name them IDB_SMALL_ICONS and IDB_LARGE_ICONS respectively.
///	// Create Ribbon Bar in CMainFrame::OnCreate:
/// m_wndRibbonBar.Create(this); 
///
/// // Initialize and set Main Ribbon Button:
/// m_MainButton.SetMenu (IDR_FILE_MENU);
/// m_MainButton.SetImage (IDB_MAIN);
/// m_MainButton.SetToolTipText (_T("File"));
/// m_wndRibbonBar.SetMainButton (&m_MainButton, CSize (45, 45)); 
///
/// // Initialize and load image list of panel icons:
/// m_PanelIcons.SetImageSize (CSize (16, 16));
/// m_PanelIcons.Load (IDB_PANEL_ICONS); 
///
/// // Add the first category:
/// CBCGPRibbonCategory* pCategory = m_wndRibbonBar.AddCategory
/// 	(_T("&Write"),           // Category name
/// 	IDB_WRITE,              // Category small images (16 x 16)
/// 	IDB_WRITE_LARGE);   // Category large images (32 x 32) 
/// 
/// // Add the first Panel to category:
/// CBCGPRibbonPanel* pPanel = pCategory->AddPanel (
/// 	_T("Clipboard"),                       // Panel name
/// 	m_PanelIcons.ExtractIcon (0));  // Panel icon 
/// 
/// // Now, we need to add ribbon elements (buttons) to the panel:
/// // Create the first button to Panel ("Paste"):
/// CBCGPRibbonButton* pPasteButton = new CBCGPRibbonButton (ID_EDIT_PASTE, _T("Paste"), -1, 0);
///
/// // The third parameter (-1) tells that this button does not have a small icon.
// // Therefore the "Paste" button will be always displayed with large icon.
///
/// // Associate a popup menu with the "Paste" button:
/// pPasteButton->SetMenu (IDR_CONTEXT_MENU); 
///
/// // Add other buttons to the panel. These buttons have small icons only:
/// pPanel->Add (new CBCGPRibbonButton (ID_EDIT_CUT, _T("Cut"), 1));
/// pPanel->Add (new CBCGPRibbonButton (ID_EDIT_COPY, _T("Copy"), 2));
/// pPanel->Add (new CBCGPRibbonButton (ID_EDIT_PAINT, _T("Paint"), 9));
/// </code>
/// </example>
class BCGCBPRODLLEXPORT CBCGPRibbonBar : public CBCGPControlBar
{
	friend class CBCGPRibbonCategory;
	friend class CBCGPRibbonCmdUI;
	friend class CBCGPBaseRibbonElement;
	friend class CBCGPRibbonCustomizeRibbonPage;
	friend class CBCGPRibbonCustomizeQATPage;
	friend class CBCGPPrintPreviewView;
	friend class CBCGPRibbonMainButton;
	friend class CBCGPRibbonPanelMenuBar;
	friend class CBCGPFrameImpl;
	friend class CBCGPRibbonEdit;
	friend class CBCGPRibbonPanel;
	friend class CBCGPRibbonConstructor;
	friend class CBCGPRibbonCollector;
	friend class CBCGPRibbonBackstageView;
	friend class CBCGPRibbonTreeCtrl;
	friend class CBCGPRibbonCustomizationData;
	friend class CBCGPRibbonKeyboardCustomizeDlg;
	friend class CBCGPRibbonCustomCategory;
	friend class CBCGPRibbonCustomPanel;
	friend class CBCGPRibbonTabsGroup;
	friend class CBCGPRibbonTab;

	DECLARE_DYNAMIC(CBCGPRibbonBar)

public:
	/// <summary></summary>
	/// <param name="rb"></param>
	/// <param name="sb"></param>
	/// <param name="strFolder"></param>
	/// <param name="strResourceName"></param>
	/// <param name="dwFlags"></param>
	/// <param name="strResourceFolder"></param>
	/// <param name="strResourcePrefix"></param>
	static BOOL SaveRibbon (const CBCGPRibbonBar* rb, const CBCGPRibbonStatusBar* sb,
					const CString& strFolder, 
					const CString& strResourceName,
					DWORD dwFlags = 0x0F,
					const CString& strResourceFolder = _T("res\\BCGSoft_ribbon"), 
					const CString& strResourcePrefix = _T("IDR_BCGP"));

// Construction
public:
	/// <summary>CBCGPRibbonBar default cosntructor</summary>
	/// <param name="bReplaceFrameCaption">TRUE if the ribbon bar replaces the application frame caption.</param>
	CBCGPRibbonBar(BOOL bReplaceFrameCaption = TRUE);

	/// <summary>
	/// Creates the Windows control and attaches it to CBCGPRibbonBar.</summary>
	/// <returns> 
	/// TRUE if the function succeeds; otherwise FALSE.</returns>
	/// <param name="pParentWnd">Pointer to parent window (usually main frame).</param>
	/// <param name="dwStyle">A combination of window styles to be applied to the control.</param>
	/// <param name="nID">Specifies control ID.</param>
	BOOL Create(CWnd* pParentWnd,
			DWORD dwStyle = WS_CHILD | WS_VISIBLE | CBRS_TOP,
			UINT nID = AFX_IDW_RIBBON_BAR);

	/// <summary>
	/// Creates the Windows control and attaches it to CBCGPRibbonBar.</summary>
	/// <returns> 
	/// TRUE if the function succeeds; otherwise FALSE.</returns>
	/// <param name="pParentWnd">Pointer to parent window (usually main frame).</param>
	/// <param name="dwCtrlStyle">Control extended style</param>
	/// <param name="dwStyle">A combination of window styles to be applied to the control.</param>
	/// <param name="nID">Specifies control ID.</param>
	BOOL CreateEx(CWnd* pParentWnd, DWORD dwCtrlStyle = 0,
		DWORD dwStyle = WS_CHILD | WS_VISIBLE | CBRS_TOP,
		UINT nID = AFX_IDW_RIBBON_BAR);

	/// <summary>
	/// Use this method to load a Ribbon Bar schema from XML. All ribbon elements (categories, panels, buttons etc) will be recreated according definitions written in the XML.</summary>
	/// <param name="uiXMLResID">Specifies Resource ID for the XML.</param>
	virtual BOOL LoadFromXML (UINT uiXMLResID);

	/// <summary>
	/// Use this method to load a Ribbon Bar schema from XML. All ribbon elements (categories, panels, buttons etc) will be recreated according definitions written in the XML.</summary>
	/// <param name="lpszXMLResID">Specifies resource ID for the XML.</param>
	virtual BOOL LoadFromXML (LPCTSTR lpszXMLResID);

	/// <summary>
	/// Use this method to import a Ribbon Bar schema from VS 2010/2012 resource file. All ribbon elements (categories, panels, buttons etc) will be recreated according definitions written in the VS resources.</summary>
	/// <param name="uiXMLResID">Specifies Resource ID for the XML.</param>
	virtual BOOL LoadFromVSRibbon(UINT uiXMLResID);

	/// <summary>
	/// Use this method to import a Ribbon Bar schema from VS 2010/2012 resource file. All ribbon elements (categories, panels, buttons etc) will be recreated according definitions written in the VS resources.</summary>
	/// <param name="lpszXMLResID">Specifies resource ID for the XML.</param>
	virtual BOOL LoadFromVSRibbon(LPCTSTR lpszXMLResID);

	/// <summary>
	/// Use this method to load a Ribbon Bar from a memory buffer containing XML definition of the Ribbon Bar schema. All ribbon elements (categories, panels, buttons etc) will be recreated according definitions written in the XML.</summary>
	/// <param name="lpszXMLBuffer">A pointer to a memory buffer with XML.</param>
	virtual BOOL LoadFromBuffer (LPCTSTR lpszXMLBuffer);

	/// <summary>
	/// Call this function with bScenicLook set to TRUE to display the Ribbon Bar in scenic look.</summary>
	/// <param name="bScenicLook">Specifies the look to be set.</param>
	/// <param name="bRecalc">Specifies whether the Ribbon Bar should recalculate layout.</param>
	void SetScenicLook (BOOL bScenicLook = TRUE, BOOL bRecalc = TRUE);

	/// <summary>
	/// Check for the scenic (Windows 7/8-like) look.</summary>
	/// <returns> 
	/// TRUE if the current look of Ribbon is scenic; otherwise FALSE.</returns>
	BOOL IsScenicLook () const;

	/// <summary>
	/// This function is called to turn backstage mode on or off./// </summary>
	/// <param name="bSet">TRUE - turn backstage mode on; FALSE - turn backstage mode off.</param>
	void SetBackstageMode(BOOL bSet = TRUE)
	{
		ASSERT_VALID(this);
		m_bBackstageMode = bSet;
	}

	/// <summary>
	/// Backstage mode checking.</summary>
	/// <returns> 
	/// TRUE if backstage mode is on; otherwise FALSE.</returns>
	BOOL IsBackstageMode() const
	{
		return m_bBackstageMode;
	}

	/// <summary>
	/// Call this method to activate a backstage page. If nPage is -1, the first page is activated.</summary>
	/// <param name="nPage">Specifies a page to activate.</param>
	BOOL ShowBackstageView(int nPage = -1);

	/// <summary>
	/// Call this method to activate a backstage page with a specified runtime class.</summary>
	/// <param name="pViewClass">Specifies a page to activate.</param>
	BOOL ShowBackstageView(CRuntimeClass* pViewClass);

	/// <summary>
	/// Call this method to activate a backstage "Print" page.</summary>
	BOOL ShowBackstagePrintView();

	/// <summary>
	/// Call this method to activate a backstage "Recent Files/Folders" page.</summary>
	BOOL ShowBackstageRecentView();

	/// <summary>
	/// Call this method to specify backstage view pages transition effect.</summary>
	void SetBackstagePageTransitionEffect(CBCGPPageTransitionManager::BCGPPageTransitionEffect effect, int nAnimationTime = 300 /* ms */);

	CBCGPPageTransitionManager::BCGPPageTransitionEffect GetBackstagePageTransitionEffect() const
	{
		return m_BackStagePageTransitionEffect;
	}

	int	GetBackstagePageTransitionTime() const
	{
		return m_BackStagePageTransitionTime;
	}

	/// <summary>Enable/disable the Ribbon customization</summary>
	/// <param name="bEnable">TRUE - enable customization; FALSE - otherwise</param>
	/// <param name="pOptions">The Ribbon customization options</param>
	void EnableCustomization(BOOL bEnable = TRUE, CBCGPRibbonCustomizationOptions* pOptions = NULL);

	/// <summary>
	/// </summary>
	/// <returns> 
	/// TRUE if Ribbon customization is enabled; otherwise FALSE.</returns>
	BOOL IsCustomizationEnabled() const
	{
		return m_bCustomizationEnabled;
	}

	/// <summary>
	/// </summary>
	/// <returns> 
	/// Ribbon customization options.</returns>
	CBCGPRibbonCustomizationOptions& GetCustomizationOptions()
	{ 
		return m_CustomizationOptions;
	}

// Operations
public:
	/// <summary></summary>
	virtual void RecalcLayout ();

	/// <summary>
	/// Use this function to assign main ribbon button to Ribbon control. Main ribbon button is a button located at the top left corner of Ribbon control.</summary>
	/// <param name="pButton">A pointer to main ribbon button.</param>
	/// <param name="sizeButton">Specifies the size of main ribbon button.</param>
	void SetMainButton (CBCGPRibbonMainButton* pButton, CSize sizeButton);

	//--------------------------
	// Ribbon categories (tabs):
	//--------------------------

	/// <summary>
	/// Use this function to add main ribbon panel. Main ribbon panel is a special panel which appears when user clicks main ribbon button.</summary>
	/// <returns> 
	/// A pointer to the newly created main ribbon panel.</returns>
	/// <param name="lpszName">Category (panel) name.</param>
	/// <param name="uiSmallImagesResID">Resource ID of small images.</param>
	/// <param name="uiLargeImagesResID">Resource ID of large images.</param>
	/// <param name="sizeSmallImage">Specifies size of small images.</param>
	/// <param name="sizeLargeImage">Specifies size of large images.</param>
	/// <param name="pRTI">Ribbon main category runtime class. Should be either NULL or CBCGPRibbonMainPanel-derived</param>
	CBCGPRibbonMainPanel* AddMainCategory (
		LPCTSTR		lpszName,
		UINT		uiSmallImagesResID,
		UINT		uiLargeImagesResID,
		CSize		sizeSmallImage = CSize (16, 16),
		CSize		sizeLargeImage = CSize (32, 32),
		CRuntimeClass*	pRTI = NULL);

	/// <summary>
	/// Use this method to add a backstage category to Ribbon Control. It can be only one backstage category in a Ribbon Control, therefore you should call this method only once at the initialization stage. 
	/// Use the returned pointer to backstage panel to initialize the backstage view.</summary>
	/// <returns> 
	/// A valid pointer to CBCGPRibbonBackstageViewPanel object.</returns>
	/// <param name="lpszName">Specifies category name.</param>
	/// <param name="uiSmallImagesResID">Resource ID of category's small image.</param>
	/// <param name="sizeSmallImage">Specifies the size of small image.</param>
	/// <param name="pRTI">A pointer to runtime class information of a CBCGPRibbonCategory-derived class.</param>
	CBCGPRibbonBackstageViewPanel* AddBackstageCategory (
		LPCTSTR		lpszName,
		UINT		uiSmallImagesResID,
		CSize		sizeSmallImage = CSize (16, 16),
		CRuntimeClass*	pRTI = NULL);

	/// <summary>
	/// Use this function to add a new ribbon category.  The category can be set up using the returned pointer.</summary>
	/// <returns> 
	/// A pointer to newly created category. This pointer can be used to setup the category.</returns>
	/// <param name="lpszName">Category name.</param>
	/// <param name="uiSmallImagesResID"></param>
	/// <param name="uiLargeImagesResID">Resource ID of large images.</param>
	/// <param name="sizeSmallImage">Specifies size of small images.</param>
	/// <param name="sizeLargeImage">Specifies size of large images.</param>
	/// <param name="nInsertAt">Zero based index to insert the category at.</param>
	/// <param name="pRTI">A pointer to runtime class information of a CBCGPRibbonCategory-derived class</param>
	CBCGPRibbonCategory* AddCategory (
		LPCTSTR			lpszName,
		UINT			uiSmallImagesResID,
		UINT			uiLargeImagesResID,
		CSize			sizeSmallImage = CSize (16, 16),
		CSize			sizeLargeImage = CSize (32, 32),
		int				nInsertAt = -1,
		CRuntimeClass*	pRTI = NULL);

	/// <summary>
	/// Use this function to add a context category. Context categories are a special type of categories, which can be shown or hidden at runtime depending on the current application context. For example, when the user selects an object, you can display special tabs with context categories intended to manipulate the specific selected object.
	///  The context category color should be one of the following values:
	/// BCGPCategoryColor_None
	/// BCGPCategoryColor_Red
	/// BCGPCategoryColor_Orange
	/// BCGPCategoryColor_Yellow
	/// BCGPCategoryColor_Green
	/// BCGPCategoryColor_Blue
	/// BCGPCategoryColor_Indigo
	/// BCGPCategoryColor_Violet</summary>
	/// <returns> 
	/// A pointer to the newly created category.</returns>
	/// <param name="lpszName">Specifies the category name.</param>
	/// <param name="lpszContextName">Specifies the category context name.</param>
	/// <param name="uiContextID">Context ID.</param>
	/// <param name="clrContext">Context category color.</param>
	/// <param name="uiSmallImagesResID">Resource ID of category small image.</param>
	/// <param name="uiLargeImagesResID">Resource ID of category large image.</param>
	/// <param name="sizeSmallImage">Size of small image.</param>
	/// <param name="sizeLargeImage">Size of large image.</param>
	/// <param name="pRTI"></param>
	CBCGPRibbonCategory* AddContextCategory (
		LPCTSTR					lpszName,
		LPCTSTR					lpszContextName,
		UINT					uiContextID,
		BCGPRibbonCategoryColor	clrContext,
		UINT					uiSmallImagesResID,
		UINT					uiLargeImagesResID,
		CSize					sizeSmallImage = CSize (16, 16),
		CSize					sizeLargeImage = CSize (32, 32),
		CRuntimeClass*			pRTI = NULL);

	CBCGPRibbonCategory* AddQATOnlyCategory (
		LPCTSTR					lpszName,
		UINT					uiSmallImagesResID,
		CSize					sizeSmallImage = CSize (16, 16));

	virtual CBCGPRibbonCategory* AddPrintPreviewCategory ();

	/// <summary>
	/// Use this function to enable the "Print Preview" tab. If print preview is enabled, the library automatically displays the "Print Preview" tab for print preview view.
	/// By the default "Print Preview" tab is enabled.</summary>
	/// <param name="bEnable">If TRUE, "Print Preview" tab will be displayed; if FALSE, "Print Preview" tab won't be diplayed automatically.</param>
	void EnablePrintPreview (BOOL bEnable = TRUE);

	BOOL IsPrintPreviewEnabled () const
	{
		return m_bIsPrintPreview;
	}

	void EnableMinimizeButton(BOOL bEnable = TRUE, BOOL bRecalc = TRUE);
	BOOL HasMinimizeButton() const
	{
		return m_bHasMinimizeButton;
	}

	int GetCategoryCount () const;
	int GetVisibleCategoryCount () const;
	CBCGPRibbonCategory* GetCategory (int nIndex) const;
	int GetCategoryIndex (CBCGPRibbonCategory* pCategory) const;

	void ShowCategory (int nIndex, BOOL bShow = TRUE);
	void ShowContextCategories (UINT uiContextID, BOOL bShow = TRUE);
	BOOL HideAllContextCategories ();
	BOOL ActivateContextCategory (UINT uiContextID);

	BCGPRibbonCategoryColor GetContextColor(UINT nContextID) const;
	int GetContextCategoriesCount() const;

	BOOL RemoveCategory (int nIndex);
	void RemoveAllCategories ();

	virtual BOOL SetActiveCategory (CBCGPRibbonCategory* pCategory, BOOL bForceRestore = FALSE);
	CBCGPRibbonCategory* GetActiveCategory () const
	{
		return m_pActiveCategory;
	}

	int FindCategoryIndexByData (DWORD dwData) const;

	BOOL GetContextName (UINT uiContextID, CString& strName) const;

	void SetBackgroundImage(UINT nResID);	// 0 - No image
	void SetBackgroundImage(CBCGPToolBarImages& image);

	//-------------------------------
	// Ribbon elements direct access:
	//-------------------------------
	CBCGPBaseRibbonElement* FindByID (UINT uiCmdID, BOOL bVisibleOnly = TRUE, BOOL bExcludeQAT = FALSE, BOOL bNonCustomOnly = TRUE) const;
	CBCGPBaseRibbonElement* FindByData (DWORD_PTR dwData, BOOL bVisibleOnly = TRUE) const;

	BOOL SetElementKeys (UINT uiCmdID, LPCTSTR lpszKeys, LPCTSTR lpszMenuKeys = NULL);

	void GetElementsByID (UINT uiCmdID, 
		CArray<CBCGPBaseRibbonElement*, CBCGPBaseRibbonElement*>& arButtons, BOOL bIncludeFloaty = FALSE);

	virtual void GetVisibleElements (
		CArray<CBCGPBaseRibbonElement*, CBCGPBaseRibbonElement*>& arButtons);

	void SetQuickAccessDefaultState (const CBCGPRibbonQATDefaultState& state);
	void SetQuickAccessCommands (const CList<UINT,UINT>& lstCommands, BOOL bRecalcLayout = TRUE);
	void GetQuickAccessCommands (CList<UINT,UINT>& lstCommands);

	void GetElementsByName (LPCTSTR lpszName, 
		CArray<CBCGPBaseRibbonElement*, CBCGPBaseRibbonElement*>& arButtons, DWORD dwFlags = 0);

	BOOL ReplaceRibbonElementByID(UINT uiCmdID, CBCGPBaseRibbonElement& newElement, BOOL bCopyContent = TRUE) const;

	CBCGPRibbonPanel* FindPanel(int nCategoryKey, int nPanelKey) const;

	//----------------------------------
	// Main panel/Backstage view access:
	//----------------------------------
	CBCGPRibbonMainPanel* GetMainPanel() const;
	CBCGPRibbonBackstageViewPanel* GetBackstageViewPanel() const;

	//--------------------------------------------------
	// Additional elements located on the right of tabs:
	//--------------------------------------------------
	void AddToTabs (CBCGPBaseRibbonElement* pElement);
	void RemoveAllFromTabs ();

	//------------------
	// Tooltips support:
	//------------------
	void EnableToolTips (BOOL bEnable = TRUE, BOOL bEnableDescr = TRUE);
	
	BOOL IsToolTipEnabled () const
	{
		return m_bToolTip;
	}

	BOOL IsToolTipDescrEnabled () const
	{
		return m_bToolTipDescr;
	}

	int GetTooltipFixedWidthRegular () const
	{
		return m_nTooltipWidthRegular;
	}

	int GetTooltipFixedWidthLargeImage () const
	{
		return m_nTooltipWidthLargeImage;
	}

	void SetTooltipFixedWidth (int nWidthRegular, int nWidthLargeImage);	// 0 - set variable size

	//-----------------
	// Key tip support:
	//-----------------
	void EnableKeyTips (BOOL bEnable = TRUE);

	BOOL IsKeyTipEnabled () const
	{
		return m_bKeyTips;
	}

	void GetItemIDsList (CList<UINT,UINT>& lstItems, BOOL bHiddenOnly = FALSE) const;

	void ToggleMimimizeState ();

	BOOL OnSysKeyDown (CFrameWnd* pFrameWnd, WPARAM wParam, LPARAM lParam);
	BOOL OnSysKeyUp (CFrameWnd* pFrameWnd, WPARAM wParam, LPARAM lParam);

	void EnableSingleLevelAccessibilityMode(BOOL bEnable = TRUE)
	{
		m_bSingleLevelAccessibilityMode = bEnable;
	}

	BOOL IsSingleLevelAccessibilityMode() const;

	//------------
	// Input mode:
	//------------
	virtual void SetInputMode(BCGP_INPUT_MODE mode);

	//--------------------------------
	// Custom caption buttons support:
	//--------------------------------
	int AddCaptionCustomButton(CBCGPRibbonCaptionCustomButton* pButton);

	int GetCaptionCustomButtonsCount() const;
	CBCGPRibbonCaptionCustomButton* GetCaptionCustomButton(int nIndex) const;

	void RemoveAllCaptionCustomButtons();

	//-------------------
	// Application state:
	//-------------------
	void SetAppFrameActiveState(BOOL bActive);
	BOOL GetAppFrameActiveState() const
	{
		return m_bAppFrameIsActive;
	}

// Attributes
public:
	int GetCaptionHeight () const
	{
		return m_nCaptionHeight;
	}

	int GetCategoryHeight () const
	{
		return m_nCategoryHeight;
	}

	BOOL IsReplaceFrameCaption () const
	{
		return m_bReplaceFrameCaption;
	}

	CBCGPRibbonMainButton* GetMainButton () const
	{
		return m_pMainButton;
	}

	CBCGPRibbonQuickAccessToolbar* GetQuickAccessToolbar ()
	{
		return &m_QAToolbar;
	}

	CBCGPRibbonCategory* GetMainCategory () const
	{
		return m_pMainCategory;
	}

	CBCGPRibbonCategory* GetBackstageCategory () const
	{
		return m_pBackstageCategory;
	}

	virtual CBCGPBaseRibbonElement* HitTest (CPoint point, 
		BOOL bCheckActiveCategory = FALSE,
		BOOL bCheckPanelCaption = FALSE);

	//---------------------------------
	// Quick access toolbar attributes:
	//---------------------------------
	void SetQuickAccessToolbarOnTop (BOOL bOnTop);
	BOOL IsQuickAccessToolbarOnTop () const
	{
		return m_bQuickAccessToolbarOnTop && m_bReplaceFrameCaption;
	}

	CRect GetQuickAccessToolbarLocation () const
	{
		return m_QAToolbar.GetRect ();
	}

	CRect GetQATCommandsLocation () const
	{
		return m_QAToolbar.GetCommandsRect ();
	}

	BOOL IsQATEmpty () const
	{
		return (int) m_QAToolbar.m_arButtons.GetSize () <= 1;
	}

	CBCGPBaseRibbonElement* GetQATDroppedDown ()
	{
		return m_QAToolbar.GetDroppedDown ();
	}

	DWORD GetHideFlags () const
	{
		return m_dwHideFlags;
	}

	int GetTabTrancateRatio () const
	{
		return m_nTabTrancateRatio;
	}

	void SetMaximizeMode (BOOL bMax, CWnd* pWnd = NULL);
	void SetActiveMDIChild (CWnd* pWnd);

	virtual CBCGPBaseRibbonElement* GetDroppedDown ();
	virtual CBCGPBaseRibbonElement* GetFocused ();

	BOOL IsTransparentCaption () const
	{
		return m_bIsTransparentCaption;
	}

	BOOL AreTransparentTabs () const
	{
		return m_bIsTransparentCaption && m_bTransparentTabs;
	}

	int GetKeyboardNavigationLevel () const
	{
		return m_nKeyboardNavLevel;
	}

	void SetKeyboardNavigationLevel (CObject* pLevel, BOOL bSetFocus = TRUE);

	CObject* GetKeyboardNavLevelParent () const
	{
		return m_pKeyboardNavLevelParent;
	}

	CObject* GetKeyboardNavLevelCurrent () const
	{
		return m_pKeyboardNavLevelCurrent;
	}

	BOOL IsDlgBarMode () const
	{
		return m_bDlgBarMode;
	}

	BOOL IsBackstageViewActive() const
	{
		return m_bBackstageViewActive;
	}

	int GetInitialBackstagePage() const
	{
		return m_nInitialBackstagePage;
	}

	void SetDefaultBackstagePage(int nPage)
	{
		m_nDefaultBackstagePage = nPage;
	}

	int GetDefaultBackstagePage() const
	{
		return m_nDefaultBackstagePage;
	}

	CBCGPRibbonButton* FindBackstageFormParent (CRuntimeClass* pClass, BOOL bDerived = TRUE);

	BOOL AreAllTabsAreInvisible() const { return m_bAllTabsAreInvisible; }

	int	GetTabsHeight() const { return m_nTabsHeight; }

	CBCGPRibbonTabsGroup* GetTabs() { return &m_Tabs;}

	LPDISPATCH GetAccessibleDispatch();

	BOOL IsCaptionButtons ();

	virtual CBCGPBaseAccessibleObject* AccessibleObjectByIndex(long lVal);

	virtual CBCGPBaseAccessibleObject* AccessibleObjectFromPoint(CPoint point);

	int GetAccObjectCount();

	const CSize	GetControlsSpacing() const { return m_sizePadding; }

	virtual BOOL GetSeparatorCustomLabel(CString& /*strSeparatorLabel*/) const { return FALSE; }

protected:
	BOOL					m_bDlgBarMode;
	int						m_nTabsHeight;
	int						m_nCategoryHeight;
	int						m_nCategoryMinWidth;
	BOOL					m_bRecalcCategoryHeight;
	BOOL					m_bRecalcCategoryWidth;
	HFONT					m_hFont;
	CBCGPRibbonCategory*	m_pActiveCategory;
	CBCGPRibbonCategory*	m_pActiveCategorySaved;
	int						m_nHighlightedTab;
	
	CArray<CBCGPRibbonCategory*,CBCGPRibbonCategory*> m_arCategories;
	CArray<int,int>			m_arVisibleCategoriesSaved;
	int						m_nNextCategoryKey;
	int						m_nNextPanelKey;

	int						m_nCaptionHeight;
	CRect					m_rectCaption;
	CRect					m_rectCaptionText;
	CBCGPRibbonMainButton*	m_pMainButton;
	BOOL					m_bAutoDestroyMainButton;
	CBCGPRibbonCategory*	m_pMainCategory;
	CBCGPRibbonCategory*	m_pBackstageCategory;
	CBCGPRibbonCategory*	m_pPrintPreviewCategory;
	int						m_nInitialBackstagePage;
	int						m_nDefaultBackstagePage;
	BOOL					m_bIsPrintPreview;
	CSize					m_sizeMainButton;
	CBCGPBaseRibbonElement*	m_pHighlighted;
	CBCGPBaseRibbonElement*	m_pPressed;
	BOOL					m_bTracked;
	const BOOL				m_bReplaceFrameCaption;
	BOOL					m_bQuickAccessToolbarOnTop;
	DWORD					m_dwHideFlags;
	int						m_nTabTrancateRatio;
	CBCGPRibbonButtonsGroup	m_TabElements;
	int						m_nBackstageViewTop;
	CToolTipCtrl*			m_pToolTip;
	BOOL					m_bForceRedraw;
	int						m_nSystemButtonsNum;
	BOOL					m_bMaximizeMode;
	BOOL					m_bAutoCommandTimer;
	BOOL					m_bPrintPreviewMode;
	BOOL					m_bBackstageViewActive;
	BOOL					m_bIsTransparentCaption;
	BOOL					m_bTransparentTabs;
	CRect					m_rectSysButtons;
	BOOL					m_bIsMaximized;
	BOOL					m_bToolTip;
	BOOL					m_bToolTipDescr;
	int						m_nTooltipWidthRegular;
	int						m_nTooltipWidthLargeImage;
	BOOL					m_bKeyTips;
	BOOL					m_bIsCustomizeMenu;
	int						m_nKeyboardNavLevel;
	CObject*				m_pKeyboardNavLevelParent;
	CObject*				m_pKeyboardNavLevelCurrent;
	int						m_nCurrKeyChar;
	BOOL					m_bDontSetKeyTips;
	BOOL					m_bHasMinimizeButton;
	BOOL					m_bAllTabsAreInvisible;
	UINT					m_uiActiveContext;
	CRect					m_rectBackstageTopArea;
	CBCGPRibbonBackstageCloseButton	m_BackStageCloseBtn;
	CBCGPToolBarImages		m_imageBackstageClose;
	CBCGPToolBarImages		m_imageBackground;
	
	CBCGPRibbonCaptionButton m_CaptionButtons [RIBBON_CAPTION_BUTTONS];
	CArray<CBCGPRibbonCaptionCustomButton*, CBCGPRibbonCaptionCustomButton*> m_arCaptionCustomButtons;

	CArray<CBCGPRibbonContextCaption*, CBCGPRibbonContextCaption*>
							m_arContextCaptions;

	CBCGPRibbonQuickAccessToolbar	m_QAToolbar;

	CArray<CBCGPRibbonKeyTip*,CBCGPRibbonKeyTip*>
							m_arKeyElements;
	BOOL					m_bScenicLook;
	BOOL					m_bIsScenicSet;
	BOOL					m_bBackstageMode;
	int						m_nBackstageHorzOffset;

	CBCGPPageTransitionManager::BCGPPageTransitionEffect	m_BackStagePageTransitionEffect;
	int														m_BackStagePageTransitionTime;

	BOOL							m_bCustomizationEnabled;
	CBCGPRibbonCustomizationData	m_CustomizationData;
	CBCGPRibbonCustomizationOptions	m_CustomizationOptions;
	CBCGPToolBarImages				m_CustomImages;
	DWORD							m_VersionStamp;

	CBCGPToolBarImages				m_PanelIcons;

	BOOL							m_bSingleLevelAccessibilityMode;
	CBCGPRibbonTabsGroup			m_Tabs;
	LONG							m_nAccLastIndex;
	CSize							m_sizePadding;
	BOOL							m_bAppFrameIsActive;

// Overrides
public:
	virtual void OnClickButton (CBCGPRibbonButton* pButton, CPoint point);
	virtual BOOL IsMainRibbonBar () const	{	return TRUE;	}
	virtual BOOL IsShowGroupBorder (CBCGPRibbonButtonsGroup* /*pGroup*/) const	{	return FALSE;	}
	virtual void OnEditContextMenu (CBCGPRibbonEditCtrl* pEdit, CPoint point);

	virtual void DWMCompositionChanged ();

	virtual BOOL OnShowRibbonQATMenu (CWnd* pWnd, int x, int y, CBCGPBaseRibbonElement* pHit);

	virtual BOOL TranslateChar (UINT nChar);

	virtual void OnRTLChanged (BOOL bIsRTL);

	virtual BOOL OnFilterSearchResult(const CBCGPBaseRibbonElement* /*pHit*/) { return TRUE; }

	virtual void DrawCustomElementImage(CDC* pDC, const CBCGPBaseRibbonElement* pElem, CBCGPBaseRibbonElement::RibbonImageType type, CRect& rectImage);
	virtual CSize GetCustomElementImageSize(const CBCGPBaseRibbonElement* pElem, CBCGPBaseRibbonElement::RibbonImageType type) const;

	virtual CWnd* GetTopLevelOwner() const { return CWnd::GetSafeOwner(); } 

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPRibbonBar)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

	virtual void OnUpdateCmdUI (CFrameWnd* pTarget, BOOL bDisableIfNoHndler);
	virtual void OnControlBarContextMenu (CWnd* pParentFrame, CPoint point);

	virtual BOOL OnBeforeShowPaletteContextMenu (const CBCGPBaseRibbonElement* pHit, CMenu& menu);
	virtual void OnBeforeShowContextMenu(CMenu& /*menu*/) {}
	virtual BOOL OnBeforeShowBackstageView();

	// Accessibility:
	virtual BOOL OnSetAccData(long lVal);
	virtual HRESULT get_accChildCount(long *pcountChildren);
	virtual HRESULT get_accChild(VARIANT varChild, IDispatch **ppdispChild);
	virtual HRESULT accHitTest(long xLeft, long yTop, VARIANT *pvarChild);
	virtual HRESULT accNavigate(long navDir, VARIANT varStart, VARIANT *pvarEndUpAt);
	virtual HRESULT accDoDefaultAction(VARIANT varChild);
    virtual HRESULT accLocation(long *pxLeft, long *pyTop, long *pcxWidth, long *pcyHeight, VARIANT varChild);
	virtual HRESULT get_accName(VARIANT varChild, BSTR *pszName);

protected:
	virtual CSize CalcFixedLayout(BOOL bStretch, BOOL bHorz);
	virtual BOOL AllowShowOnControlMenu () const			
	{	
		return FALSE;	
	}

	virtual void OnFillBackground (CDC* pDC, CRect rectClient);

	virtual BOOL LoadState (LPCTSTR lpszProfileName = NULL, int nIndex = -1, UINT uiID = (UINT) -1);
	virtual BOOL SaveState (LPCTSTR lpszProfileName = NULL, int nIndex = -1, UINT uiID = (UINT) -1);

	virtual void OnSetPrintPreviewKeys (
		CBCGPRibbonPanel* pPrintPanel,
		CBCGPRibbonPanel* pZoomPanel,
		CBCGPRibbonPanel* pPreviewPanel);

	virtual BOOL HideInPrintPreviewMode () const {	return FALSE;	}

	virtual void OnBeforeProcessKey (int& nChar);

	virtual void OnChangeVisualManager();

	virtual void SetBackstageViewActve(BOOL bOn);

	virtual int GetFrameHeight() const;

// Implementation
public:
	virtual ~CBCGPRibbonBar();

	void PopTooltip ();
	BOOL DrawMenuImage (CDC* pDC, const CBCGPToolbarMenuButton* pMenuItem, 
						const CRect& rectImage);
	HICON ExportImageToIcon(UINT uiCmdID, BOOL bIsLargeIcon);

	virtual BOOL OnShowRibbonContextMenu (CWnd* pWnd, int x, int y, CBCGPBaseRibbonElement* pHit);

	void ForceRecalcLayout ();
	void DeactivateKeyboardFocus (BOOL bSetFocus = TRUE);

	void ShowKeyTips (BOOL bRepos = FALSE);
	void HideKeyTips ();

	int GetVisibleContextCaptionCount(); 
	void GetVisibleContextCaptions(CArray<int, int>* arCaptions);
	void GetVisibleContextCaptions(CArray<CBCGPRibbonContextCaption*, CBCGPRibbonContextCaption*>& arCaptions);

	void SetBackstageHorzOffset(int nScrollOffsetHorz);

protected:
	virtual void OnDraw(CDC* pDC);
	void ShowSysMenu (const CPoint& point);

	void SetPrintPreviewMode (BOOL bSet = TRUE);
	void SetVisibilityInFrame (int cxFrame, int cyFrame);

	CBCGPRibbonContextCaption* FindContextCaption (UINT uiID) const;

	void UpdateToolTipsRect ();
	BOOL ProcessKey (int nChar);
	BOOL NavigateRibbon (int nChar);

	static CBCGPBaseRibbonElement* FindNextFocusedElement (
		int nChar, const CArray<CBCGPBaseRibbonElement*, CBCGPBaseRibbonElement*>& arElems,
		CRect rectElems, CBCGPBaseRibbonElement* pFocused,
		BOOL bIsScrollLeftAvailable, BOOL bIsScrollRightAvailable, int& nScroll);

	static CBCGPBaseRibbonElement* FindNearest (CPoint pt, const CArray<CBCGPBaseRibbonElement*, CBCGPBaseRibbonElement*>& arButtons);

	void RemoveAllKeys ();

	void AddCustomCategory(CBCGPRibbonCategory* pCategory, BOOL bIsNew);
	void OnCloseCustomizePage(BOOL bIsOK);

	void ApplyCustomizationData(const CBCGPRibbonCustomizationData& data);

	BOOL IsPanelValid(CBCGPRibbonDefaultPanelButton* pDefaultButton);

// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPRibbonBar)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnCancelMode();
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnDestroy();
	afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnSysColorChange();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	//}}AFX_MSG
	afx_msg LRESULT OnPrint(WPARAM wp, LPARAM lp);
	afx_msg LRESULT OnSetFont (WPARAM, LPARAM);
	afx_msg LRESULT OnGetFont (WPARAM, LPARAM);
	afx_msg LRESULT OnMouseLeave (WPARAM, LPARAM);
	afx_msg LRESULT OnBCGUpdateToolTips (WPARAM, LPARAM);
	afx_msg BOOL OnNeedTipText(UINT id, NMHDR* pNMH, LRESULT* pResult);
	afx_msg LRESULT OnPostRecalcLayout (WPARAM,LPARAM);
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPRibbonBar idle update through CBCGPRibbonCmdUI class

class CBCGPRibbonCmdUI : public CCmdUI
{
public:
	CBCGPRibbonCmdUI ();

	virtual void Enable(BOOL bOn);
	virtual void SetCheck(int nCheck);
	virtual void SetText(LPCTSTR lpszText);
	virtual void SetRadio(BOOL bOn = TRUE);

	CBCGPBaseRibbonElement*	m_pUpdated;
};

//////////////////////////////////////////////////////////////////////
// CBCGPRibbonMainButton

class BCGCBPRODLLEXPORT CBCGPRibbonMainButton : public CBCGPRibbonButton
{
	friend class CBCGPRibbonCollector;
	friend class CBCGPRibbonConstructor;
	friend class CBCGPRibbonBar;
	friend class CBCGPRibbonBackstageView;

	DECLARE_DYNCREATE(CBCGPRibbonMainButton);

public:
	CBCGPRibbonMainButton ()
	{
	}

	CBCGPRibbonMainButton (UINT uiBmpResID)
	{
		SetImage (uiBmpResID);
	}

	CBCGPRibbonMainButton (HBITMAP hBmp)
	{
		SetImage (hBmp);
	}

	void SetImage (UINT uiBmpResID);
	void SetImage (HBITMAP hBmp);
	void SetScenicImage (UINT uiBmpResID);
	void SetScenicImage (HBITMAP hBmp);
	void SetScenicText (LPCTSTR lpszText);

	CBCGPToolBarImages& GetImage ()			{	return m_Image;			}
	CBCGPToolBarImages& GetScenicImage ()	{	return m_ImageScenic;	}
	CString GetScenicText ()				{	return m_strTextScenic;	}

protected:
	virtual BOOL IsShowTooltipOnBottom () const	{	return FALSE;	}

	virtual BOOL IsMainRibbonButton () const
	{
		return TRUE;
	}

	virtual BOOL CanBeAddedToQAT () const
	{
		return FALSE;
	}

	virtual int AddToListBox (CBCGPRibbonCommandsListBox* /*pWndListBox*/, BOOL /*bDeep*/)
	{
		return -1;
	}

	virtual CBCGPGridRow* AddToTree (CBCGPGridCtrl* /*pGrid*/, CBCGPGridRow* /*pParent*/)
	{
		return NULL;
	}

	virtual void OnCalcTextSize (CDC* pDC);
	virtual CSize GetImageSize (RibbonImageType /*type*/) const
	{
		ASSERT_VALID (this);

		if (m_sizeTextRight.cx > 0)
		{
			return m_sizeTextRight;
		}

		if (m_pRibbonBar != NULL && m_pRibbonBar->IsScenicLook () && 
			m_ImageScenic.IsValid ())
		{
			return m_ImageScenic.GetImageSize ();
		}

		return m_Image.GetImageSize ();
	}

	virtual void DrawImage (CDC* pDC, RibbonImageType type, CRect rectImage);

	virtual void OnLButtonDown (CPoint point);
	virtual void OnLButtonDblClk (CPoint point);
	virtual BOOL OnKey (BOOL bIsMenuKey);
	virtual void OnAccDefaultAction()			{	ShowMainMenu();	}
	virtual BOOL IsDrawTooltipImage () const	{	return FALSE;	}

	// Accessibility:
	virtual HRESULT get_accParent(IDispatch **ppdispParent);
	virtual HRESULT accNavigate(long navDir, VARIANT varStart, VARIANT *pvarEndUpAt);
	virtual BOOL SetACCData(CWnd* pParent, CBCGPAccessibilityData& data);

	BOOL ShowMainMenu ();

	CBCGPToolBarImages		m_Image;
	CBCGPToolBarImages		m_ImageScenic;
	CString					m_strTextScenic;
};

//////////////////////////////////////////////////////////////////////
// CBCGPRibbonMinimizeButton

class BCGCBPRODLLEXPORT CBCGPRibbonMinimizeButton : public CBCGPRibbonButton
{
	DECLARE_DYNAMIC (CBCGPRibbonMinimizeButton)

	friend class CBCGPRibbonBar;

protected:
	CBCGPRibbonMinimizeButton (CBCGPRibbonBar& parentRibbon);

	virtual void DrawImage (CDC* pDC, RibbonImageType /*type*/, CRect /*rectImage*/);
	
	virtual CSize GetImageSize (RibbonImageType /*type*/) const;

	virtual void OnClick (CPoint /*point*/)
	{
		m_parentRibbon.ToggleMimimizeState ();
	}

	virtual int AddToListBox (CBCGPRibbonCommandsListBox* /*pWndListBox*/, BOOL /*bDeep*/)
	{
		return -1;
	}

	virtual CBCGPGridRow* AddToTree (CBCGPGridCtrl* /*pGrid*/, CBCGPGridRow* /*pParent*/)
	{
		return NULL;
	}

	CBCGPRibbonBar& m_parentRibbon;
};

extern BCGCBPRODLLEXPORT UINT BCGM_ON_CHANGE_RIBBON_CATEGORY;
extern BCGCBPRODLLEXPORT UINT BCGM_ON_RIBBON_CUSTOMIZE;
extern BCGCBPRODLLEXPORT UINT BCGM_ON_HIGHLIGHT_RIBBON_LIST_ITEM;
extern BCGCBPRODLLEXPORT UINT BCGM_ON_BEFORE_SHOW_RIBBON_ITEM_MENU;
extern BCGCBPRODLLEXPORT UINT BCGM_ON_BEFORE_SHOW_PALETTE_CONTEXTMENU;
extern BCGCBPRODLLEXPORT UINT BCGM_ON_BEFORE_RIBBON_BACKSTAGE_VIEW;

#define ForceRelalcLayout	ForceRecalcLayout

#endif // BCGP_EXCLUDE_RIBBON

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPRIBBONBAR_H__B3C69A4D_9CDE_406F_9837_2347D306920A__INCLUDED_)
