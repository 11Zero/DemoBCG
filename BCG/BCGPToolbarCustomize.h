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

// CBCGPToolbarCustomize.h : header file
//
// CBCGPToolbarCustomize is a modeless property sheet that is 
// created once and not destroyed until the application
// closes.  It is initialized and controlled from
// CPropertyFrame.
 
#ifndef __BCGPTOOLBARCUSTOMIZE_H__
#define __BCGPTOOLBARCUSTOMIZE_H__

#include "BCGCBPro.h"
#include "BCGPToolbarCustomizePages.h"
#include "BCGPToolsPage.h"
#include "BCGPMenuPage.h"
#include "BCGPMousePage.h"
#include "BCGPKeyboardPage.h"
#include "BCGPOptionsPage.h"
#include "BCGPButton.h"

class CBCGPToolBarImages;
class CBCGPToolbarButton;
class CBCGPToolBar;
class CBCGPUserTool;

//---------------------
// Customization flags:
//---------------------
#define BCGCUSTOMIZE_MENU_SHADOWS		0x0001	// Allow chnage menu shadow appearance
#define BCGCUSTOMIZE_TEXT_LABELS		0x0002	// Allow chnage toolbar text lables below the image
#define BCGCUSTOMIZE_LOOK_2000			0x0004	// Allow chnage look 2000
#define BCGCUSTOMIZE_MENU_ANIMATIONS	0x0008	// Allow chnage menu animations
#define BCGCUSTOMIZE_NOHELP				0x0010	// Remove help button from the customization dialog
#define BCGCUSTOMIZE_CONTEXT_HELP		0x0020	// Add '?' to caption and "What's This" context menu
#define BCGCUSTOMIZE_SELECT_SKINS		0x0040	// Obsolete flag
#define BCGCUSTOMIZE_NOTOOLS			0x0080	// Remove tools page from the customization dialog
#define BCGCUSTOMIZE_MENUAMPERS			0x0100	// Allow buttons text contain '&'
#define BCGCUSTOMIZE_NO_LARGE_ICONS		0x0200	// Remove "Large Icons" option from the customization dialog

/////////////////////////////////////////////////////////////////////////////
// CBCGPToolbarCustomize

class BCGCBPRODLLEXPORT CBCGPToolbarCustomize : public CPropertySheet
{
	friend class CBCGPToolBar;
	friend class CBCGPToolsPage;
	friend class CBCGPKeyboardPage;
	friend class CToolsList;

	DECLARE_DYNAMIC(CBCGPToolbarCustomize)

// Construction
public:
	CBCGPToolbarCustomize (CFrameWnd* pWndParentFrame, 
						BOOL bAutoSetFromMenus = FALSE,
						UINT uiFlags = (BCGCUSTOMIZE_MENU_SHADOWS | BCGCUSTOMIZE_TEXT_LABELS | BCGCUSTOMIZE_LOOK_2000 | BCGCUSTOMIZE_MENU_ANIMATIONS),
						CList <CRuntimeClass*, CRuntimeClass*>* plistCustomPages = NULL);
// Attributes
public:
	UINT GetFlags () const
	{
		return m_uiFlags;
	}

protected:
	//------------------------------------------
	// Toolbar+menu items divided by categories:
	//------------------------------------------
	CMap<CString, LPCTSTR, CObList*, CObList*>	m_ButtonsByCategory;
	CStringList									m_strCategoriesList;
													// Need for order!
	//----------------
	// Property pages:
	//----------------	
	CBCGPCustomizePage*	m_pCustomizePage;
	CBCGPToolbarsPage*	m_pToolbarsPage;
	CBCGPKeyboardPage*	m_pKeyboardPage;
	CBCGPMenuPage*		m_pMenuPage;
	CBCGPMousePage*		m_pMousePage;
	CBCGPOptionsPage*	m_pOptionsPage;
	CBCGPToolsPage*		m_pToolsPage;
	
	//-------------------------------------
	// Support for additional custom pages:
	//-------------------------------------
	CList<CPropertyPage*,CPropertyPage*>	m_listCustomPages;

	CString				m_strAllCommands;

	CFrameWnd*			m_pParentFrame;
	BOOL				m_bAutoSetFromMenus;
	UINT				m_uiFlags;
	BOOL				m_bSaveMenuAmps;

	CBCGPButton			m_btnHelp;

	UINT				m_uiControlbarsMenuEntryID;

// Operations
public:

	void AddButton (UINT uiCategoryId, const CBCGPToolbarButton& button,
					int iInsertAfter = -1);
	void AddButton (LPCTSTR lpszCategory, const CBCGPToolbarButton& button,
					int iInsertAfter = -1);

	int RemoveButton (UINT uiCategoryId, UINT uiCmdId);
	int RemoveButton (LPCTSTR lpszCategory, UINT uiCmdId);

	void ReplaceButton (UINT uiCmd, const CBCGPToolbarButton& button);

	BOOL AddToolBar (UINT uiCategoryId, UINT uiToolbarResId);
	BOOL AddToolBar (LPCTSTR lpszCategory, UINT uiToolbarResId);
	BOOL AddMenu (UINT uiMenuResId);
	void AddMenuCommands (const CMenu* pMenu, BOOL bPopup, LPCTSTR lpszCategory = NULL,
						LPCTSTR lpszMenuPath = NULL /* e.g. "ViewToolbars"*/);

	BOOL RenameCategory(LPCTSTR lpszCategoryOld, LPCTSTR lpszCategoryNew);

	BOOL SetUserCategory (LPCTSTR lpszCategory);

	void EnableUserDefinedToolbars (BOOL bEnable = TRUE);
	void EnableTools (CObList* pToolsList);	// List of CBCGPUserTool-derived objects

	void FillCategoriesComboBox (CComboBox& wndCategory, BOOL bAddEmpty = TRUE) const;
	void FillCategoriesListBox (CListBox& wndCategory, BOOL bAddEmpty = TRUE) const;
	
	virtual void FillAllCommandsList (CListBox& wndListOfCommands) const;

	LPCTSTR GetCommandName (UINT uiCmd) const;

	int GetCountInCategory (LPCTSTR lpszItemName, const CObList& lstCommands) const;
		// Retrun number of items with the same name

protected:
	void SetFrameCustMode (BOOL bCustMode);
	void ShowToolBar (CBCGPToolBar* pToolBar, BOOL bShow);
	void SetupFromMenus ();
	void AddUserTools (LPCTSTR lpszCategory);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPToolbarCustomize)
	public:
	virtual BOOL OnInitDialog();
	protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

	//------------------
	// Tools page hooks:
	//------------------
	virtual void OnInitToolsPage () {}
	virtual void OnBeforeChangeTool (CBCGPUserTool* /*pSelTool*/) {}
	virtual void OnAfterChangeTool (CBCGPUserTool* /*pSelTool*/) {}
	virtual BOOL CheckToolsValidity (const CObList& /*lstTools*/)
	{
		return TRUE;
	}

	//---------------------
	// Keyboard page hooks:
	//---------------------
	virtual BOOL OnAssignKey (ACCEL* /*pAccel*/)	{	return TRUE;	}

public:
	virtual BOOL Create();

	virtual BOOL OnEditToolbarMenuImage (CWnd* pWndParent, CBitmap& bitmap, int nBitsPerPixel);

// Implementation
public:
	 virtual ~CBCGPToolbarCustomize();
	 virtual void PostNcDestroy();

// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPToolbarCustomize)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif	// __BCGPTOOLBARCUSTOMIZE_H__
