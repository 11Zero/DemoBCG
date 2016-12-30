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

#if !defined(AFX_BCGPWORKSPACE_H__C82F6ED0_8263_11D3_A9EF_005056800000__INCLUDED_)
#define AFX_BCGPWORKSPACE_H__C82F6ED0_8263_11D3_A9EF_005056800000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"
#include "BCGPUserTool.h"
#include "BCGPToolBarImages.h"
#include "BCGPTabWnd.h"
#include "BCGPToolbarCustomize.h"

class CBCGPMouseManager;
class CBCGPContextMenuManager;
class CBCGPKeyboardManager;
class CBCGPUserToolsManager;
class CBCGPFrameImpl;
class CBCGPMDIFrameWnd;
class CBCGPFrameWnd;
class CBCGPOleIPFrameWnd;
class CBCGPShellManager;
class CBCGPWorkspace;
class CBCGPTooltipManager;

#if defined _AFXDLL && !defined _BCGCBPRO_STATIC_	// Skins manager can not be used in the static version
class CBCGPSkinManager;
#endif

// Some accessor functions
BCGCBPRODLLEXPORT CBCGPWorkspace* GetWorkspace();
#define WORKSPACE (*GetWorkspace())

#define BCG_DEFAULT_SKINS_DIR	(LPCTSTR)-1

//---------------------------------------------------------------------------------
//  *
//  * This class is useful to store all the customizations at once
//  *
//  *
//  * you may use this as a mixin class for your CWinApp object !
//  *
//  * i.g. "class CMyApp : public CWinApp, public CBCGPWorkspace {...};"
//  *
//  * but you may also use it standalone. Just call once UseWorkspaceManager
//  *
//---------------------------------------------------------------------------------
class BCGCBPRODLLEXPORT CBCGPWorkspace
{
public:
	CBCGPWorkspace (BOOL bResourceSmartUpdate = FALSE);
	virtual ~CBCGPWorkspace();

	static BOOL UseWorkspaceManager(LPCTSTR lpszSectionName = NULL, BOOL bResourceSmartUpdate = FALSE);
	LPCTSTR SetRegistryBase(LPCTSTR lpszSectionName = NULL);
	LPCTSTR	GetRegistryBase() {	  return m_strRegSection;	}

	// Saved data version:
	int		GetDataVersionMajor () const	{	return m_iSavedVersionMajor;	}
	int		GetDataVersionMinor () const	{	return m_iSavedVersionMinor;	}
	int		GetDataVersion () const;

	BOOL	InitMouseManager ();
	BOOL	InitContextMenuManager ();
	BOOL	InitKeyboardManager ();
	BOOL	InitShellManager ();
	BOOL	InitTooltipManager ();

#if defined _AFXDLL && !defined _BCGCBPRO_STATIC_	// Skins manager can not be used in the static version
	BOOL	InitSkinManager (LPCTSTR lpszSkinsDirectory = BCG_DEFAULT_SKINS_DIR);
				// BCG_DEFAULT_SKINS_DIR is "<program files>\Common Files\BCGSoft\Skins
#endif

	virtual void OnAfterDownloadSkins (const CString& /*strSkinsDirectory*/) {}

	BOOL	EnableUserTools (const UINT uiCmdToolsDummy,
							const UINT uiCmdFirst, const UINT uiCmdLast,
							CRuntimeClass* pToolRTC = RUNTIME_CLASS (CBCGPUserTool),
							UINT uArgMenuID = 0, UINT uInitDirMenuID = 0);

	BOOL	EnableTearOffMenus (LPCTSTR lpszRegEntry,
								const UINT uiCmdFirst, const UINT uiCmdLast);


	BOOL IsResourceSmartUpdate () const
	{
		return m_bResourceSmartUpdate;
	}

	void EnableLoadWindowPlacement (BOOL bEnable = TRUE)
	{
		m_bLoadWindowPlacement = bEnable;
	}

	CBCGPMouseManager*			GetMouseManager();
	CBCGPContextMenuManager*	GetContextMenuManager();
	CBCGPKeyboardManager*		GetKeyboardManager();
	CBCGPUserToolsManager*		GetUserToolsManager();
	CBCGPShellManager*			GetShellManager ();
	CBCGPTooltipManager*		GetTooltipManager ();

#if defined _AFXDLL && !defined _BCGCBPRO_STATIC_	// Skins manager can not be used in the static version
	CBCGPSkinManager*		GetSkinManager();
#endif

	// Call on of those in CMyApp::InitInstance just after ProcessShellCommand()
	// and before pMainFrame->ShowWindow(). See BCGDevStudioExample
	BOOL LoadState (CBCGPMDIFrameWnd* pFrame, LPCTSTR lpszSectionName = NULL);
	BOOL LoadState (CBCGPFrameWnd* pFrame, LPCTSTR lpszSectionName = NULL);
	BOOL LoadState (CBCGPOleIPFrameWnd* pFrame, LPCTSTR lpszSectionName = NULL);
	virtual BOOL LoadState (LPCTSTR lpszSectionName = NULL, CBCGPFrameImpl* pFrameImpl = NULL);

	virtual BOOL CleanState (LPCTSTR lpszSectionName = NULL);
	virtual BOOL SaveState (LPCTSTR lpszSectionName = NULL, CBCGPFrameImpl* pFrameImpl = NULL);

	BOOL SaveState (CBCGPMDIFrameWnd* pFrame, LPCTSTR lpszSectionName = NULL);
	BOOL SaveState (CBCGPFrameWnd* pFrame, LPCTSTR lpszSectionName = NULL);
	BOOL SaveState (CBCGPOleIPFrameWnd* pFrame, LPCTSTR lpszSectionName = NULL);

	BOOL IsStateExists(LPCTSTR lpszSectionName /*=NULL*/);

	virtual BOOL OnViewDoubleClick (CWnd* pWnd, int iViewId);
	virtual BOOL ShowPopupMenu (UINT uiMenuResId, const CPoint& point, CWnd* pWnd);

	virtual CString	GetRegSectionPath (LPCTSTR szSectionAdd = _T(""));

	// These functions load and store values from the "Custom" subkey
	// To use subkeys of the "Custom" subkey use GetSectionInt() etc.
	// instead
	int		GetInt(LPCTSTR lpszEntry, int nDefault = 0);
	double	GetDouble(LPCTSTR lpszEntry, double dblDefault = 0.0);
	CString	GetString(LPCTSTR lpszEntry, LPCTSTR lpzDefault = _T(""));
	BOOL	GetBinary(LPCTSTR lpszEntry, LPBYTE* ppData, UINT* pBytes);
	BOOL	GetObject(LPCTSTR lpszEntry, CObject& obj);

	BOOL	WriteInt(LPCTSTR lpszEntry, int nValue );
	BOOL	WriteDouble(LPCTSTR lpszEntry, double dblValue );
	BOOL	WriteString(LPCTSTR lpszEntry, LPCTSTR lpszValue );
	BOOL	WriteBinary(LPCTSTR lpszEntry, LPBYTE pData, UINT nBytes);
	BOOL	WriteObject(LPCTSTR lpszEntry, CObject& obj);

	
	// These functions load and store values from a given subkey
	// of the "Custom" subkey. For simpler access you may use
	// GetInt() etc.
	int		GetSectionInt( LPCTSTR lpszSubSection, LPCTSTR lpszEntry, int nDefault = 0);
	double	GetSectionDouble( LPCTSTR lpszSubSection, LPCTSTR lpszEntry, double dblDefault = 0.0);
	CString	GetSectionString( LPCTSTR lpszSubSection, LPCTSTR lpszEntry, LPCTSTR lpszDefault = _T(""));
	BOOL	GetSectionBinary(LPCTSTR lpszSubSection, LPCTSTR lpszEntry, LPBYTE* ppData, UINT* pBytes);
	BOOL	GetSectionObject(LPCTSTR lpszSubSection, LPCTSTR lpszEntry, CObject& obj);

	BOOL	WriteSectionInt( LPCTSTR lpszSubSection, LPCTSTR lpszEntry, int nValue );
	BOOL	WriteSectionDouble( LPCTSTR lpszSubSection, LPCTSTR lpszEntry, double dblValue );
	BOOL	WriteSectionString( LPCTSTR lpszSubSection, LPCTSTR lpszEntry, LPCTSTR lpszValue );
	BOOL	WriteSectionBinary(LPCTSTR lpszSubSection, LPCTSTR lpszEntry, LPBYTE pData, UINT nBytes);
	BOOL	WriteSectionObject(LPCTSTR lpszSubSection, LPCTSTR lpszEntry, CObject& obj);

	// WinHelp override:
	virtual void OnAppContextHelp (CWnd* pWndControl, const DWORD dwHelpIDArray []);

	// Idle processing override:
	virtual BOOL OnBCGPIdle (CWnd* /*pWnd*/)	{	return FALSE;	}

	// Task bar interaction:
	void EnableTaskBarInteraction(BOOL bEnable = TRUE);
	BOOL IsTaskBarInteractionEnabled()
	{
		return m_bTaskBarInteraction;
	}

	// Mouse wheel behavior:
	void EnableMouseWheelInInactiveWindow(BOOL bEnable = TRUE)
	{
		m_bMouseWheelInInactiveWindow = bEnable;
	}

	BOOL IsMouseWheelInInactiveWindowEnabled()
	{
		return m_bMouseWheelInInactiveWindow;
	}

	// Pinned files and folders:
	const CStringArray& GetPinnedPaths(BOOL bFile);
	BOOL IsPinned(BOOL bFile, LPCTSTR lpszPath);
	void PinPath(BOOL bFile, LPCTSTR lpszPath, BOOL bPin);

	// Create screenshot:
	BOOL CreateScreenshot(CBitmap& bmpScreenshot, CWnd* pWnd = NULL /* default - main app window */);

#if defined _AFXDLL && !defined _BCGCBPRO_STATIC_	// Skins manager can not be used in the static version
	virtual void OnSelectSkin ();
#endif

public:
	BOOL	m_bLoadUserToolbars;

protected:
	friend class CBCGPFrameImpl;

	// Overidables for customization
	virtual	void OnClosingMainFrame (CBCGPFrameImpl* pFrameImpl);
	
	// called before anything is loaded
	virtual void PreLoadState() {}

	// called after everything is loaded
	virtual void LoadCustomState() {}

	// called before anything is saved
	virtual void PreSaveState() {}

	// called after everything is saved
	virtual void SaveCustomState() {}

	virtual BOOL LoadWindowPlacement (
					CRect& rectNormalPosition, int& nFflags, int& nShowCmd);
	virtual BOOL StoreWindowPlacement (
				const CRect& rectNormalPosition, int nFflags, int nShowCmd);

	virtual BOOL ReloadWindowPlacement (CFrameWnd* pFrame);

protected:
	CString	m_strRegSection;

	BOOL	m_bKeyboardManagerAutocreated;
	BOOL	m_bContextMenuManagerAutocreated;
	BOOL	m_bMouseManagerAutocreated;
	BOOL	m_bUserToolsManagerAutoCreated;
	BOOL	m_bTearOffManagerAutoCreated;
	BOOL	m_bSkinManagerAutocreated;
	BOOL	m_bShellManagerAutocreated;
	BOOL	m_bTooltipManagerAutocreated;

	int		m_iSavedVersionMajor;
	int		m_iSavedVersionMinor;

	BOOL	m_bForceDockStateLoad;	// Load dock bars state even it's not valid
	BOOL	m_bLoadSaveFrameBarsOnly;

	BOOL	m_bSaveState;			// Automatically save state when the main frame is closed.
	BOOL	m_bAfxStoreDockSate;

	const BOOL	m_bResourceSmartUpdate;	// Automatic toolbars/menu resource update
	BOOL	m_bForceImageReset;			// Force image reset every time when the frame is loaded
	BOOL	m_bLoadWindowPlacement;

	BOOL	m_bTaskBarInteraction;
	BOOL	m_bMouseWheelInInactiveWindow;

	CStringArray	m_arPinnedFiles;
	CStringArray	m_arPinnedFolders;

	BOOL			m_bAutoMenuBar;
};

struct BCGCBPRODLLEXPORT CBCGPToolbarOptions
{
public:
	CBCGPToolbarOptions()
	{
		m_bInitialized = FALSE;
		m_nViewToolbarsMenuEntryID = 0;
		m_nCustomizeCommandID = 0;
		m_bContextMenuShowsToolbarsOnly = FALSE;
		m_bViewMenuShowsToolbarsOnly = FALSE;
		m_strCustomizeCommandLabel = _T("Customize...");
		m_bShowCustomizeButton = TRUE;
		m_nUserToolbarCommandIDFirst = 0;
		m_nUserToolbarCommandIDLast = 0;
		m_nToolbarCustomizeFlags = BCGCUSTOMIZE_TEXT_LABELS | BCGCUSTOMIZE_MENU_ANIMATIONS;
	}

	BOOL	m_bInitialized;
	UINT	m_nViewToolbarsMenuEntryID;	// Usually ID_VIEW_TOOLBAR
	UINT	m_nCustomizeCommandID;		// Command ID of menu item associated with toolbar customization dialog
	CString	m_strCustomizeCommandLabel;	// Name of menu item associated with toolbar customization dialog
	BOOL	m_bShowCustomizeButton;		// Display toolbar chevron

	BOOL	m_bContextMenuShowsToolbarsOnly;
	BOOL	m_bViewMenuShowsToolbarsOnly;

	UINT	m_nUserToolbarCommandIDFirst;
	UINT	m_nUserToolbarCommandIDLast;

	CString	m_strToolbarCustomIconsPath;

	UINT	m_nToolbarCustomizeFlags;
};

// General, Visual Manager-specific application options:
struct BCGCBPRODLLEXPORT CBCGPAppOptions
{
public:
	CBCGPAppOptions()
	{
		Reset();
	}

	void Reset()
	{
		m_bScenicRibbon = FALSE;
		m_bRibbonMinimizeButton = FALSE;
		m_strScenicRibbonLabel.Empty();
		m_MDITabsStyle = CBCGPTabWnd::STYLE_3D_SCROLLED;
		m_MDITabsCloseButtonMode = CBCGPTabWnd::TAB_CLOSE_BUTTON_NONE;
		m_bMDITabsAutoColor = FALSE;
		m_bMDITabsLargeFont = FALSE;
		m_bMDIActiveTabBold = TRUE;
	}

	// Ribbon-specific options:
	BOOL							m_bScenicRibbon;
	BOOL							m_bRibbonMinimizeButton;
	CString							m_strScenicRibbonLabel;

	// MDI Tabs/Tabbed Groups-specific options:
	CBCGPTabWnd::Style				m_MDITabsStyle;
	CBCGPTabWnd::TabCloseButtonMode	m_MDITabsCloseButtonMode;
	BOOL							m_bMDITabsAutoColor;
	BOOL							m_bMDITabsLargeFont;
	BOOL							m_bMDIActiveTabBold;
};

class BCGCBPRODLLEXPORT CBCGPWinApp :	public CWinApp,
										public CBCGPWorkspace
{
	DECLARE_DYNAMIC(CBCGPWinApp)

public:
	enum BCGP_VISUAL_THEME
	{
		BCGP_VISUAL_THEME_DEFAULT = 0,	// Windows native theme
		BCGP_VISUAL_THEME_OFFICE_2000,
		BCGP_VISUAL_THEME_OFFICE_XP,
		BCGP_VISUAL_THEME_OFFICE_2003,
		BCGP_VISUAL_THEME_VS_2005,
		BCGP_VISUAL_THEME_OFFICE_2007_BLUE,
		BCGP_VISUAL_THEME_OFFICE_2007_BLACK,
		BCGP_VISUAL_THEME_OFFICE_2007_SILVER,
		BCGP_VISUAL_THEME_OFFICE_2007_AQUA,
		BCGP_VISUAL_THEME_CARBON,
		BCGP_VISUAL_THEME_VS_2008,
		BCGP_VISUAL_THEME_VS_2010,
		BCGP_VISUAL_THEME_SCENIC,
		BCGP_VISUAL_THEME_OFFICE_2010_BLUE,
		BCGP_VISUAL_THEME_OFFICE_2010_BLACK,
		BCGP_VISUAL_THEME_OFFICE_2010_SILVER,
		BCGP_VISUAL_THEME_VS_2012_LIGHT,
		BCGP_VISUAL_THEME_VS_2012_DARK,
		BCGP_VISUAL_THEME_VS_2012_BLUE,
		BCGP_VISUAL_THEME_OFFICE_2013_WHITE,
		BCGP_VISUAL_THEME_OFFICE_2013_GRAY,
		BCGP_VISUAL_THEME_OFFICE_2013_DARK_GRAY,
		BCGP_VISUAL_THEME_VS_2013_LIGHT,
		BCGP_VISUAL_THEME_VS_2013_DARK,
		BCGP_VISUAL_THEME_VS_2013_BLUE,
		BCGP_VISUAL_THEME_CUSTOM = 9999,
	};

public:
	CBCGPWinApp();

	virtual BOOL InitInstance();
	virtual int ExitInstance();

	void SetVisualTheme(BCGP_VISUAL_THEME theme);
	BCGP_VISUAL_THEME GetVisualTheme() const
	{
		return m_ActiveTheme;
	}

	UINT GetVisualThemeCommandID(BCGP_VISUAL_THEME theme) const;

	void AddVisualTheme(BCGP_VISUAL_THEME theme, UINT nCommandID, BOOL bActive = FALSE);

	void SetToolbarOptions(const CBCGPToolbarOptions& options)
	{
		m_ToolbarOptions = options;
		m_ToolbarOptions.m_bInitialized = TRUE;
	}

	const CBCGPToolbarOptions& GetToolbarOptions() const
	{
		return m_ToolbarOptions;
	}

// Overrides:
public:
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	virtual BOOL LoadState (LPCTSTR lpszSectionName = NULL, CBCGPFrameImpl* pFrameImpl = NULL);

	BOOL LoadState (CBCGPMDIFrameWnd* pFrame, LPCTSTR lpszSectionName = NULL) { return CBCGPWorkspace::LoadState(pFrame, lpszSectionName); }
	BOOL LoadState (CBCGPFrameWnd* pFrame, LPCTSTR lpszSectionName = NULL) { return CBCGPWorkspace::LoadState(pFrame, lpszSectionName); }
	BOOL LoadState (CBCGPOleIPFrameWnd* pFrame, LPCTSTR lpszSectionName = NULL) { return CBCGPWorkspace::LoadState(pFrame, lpszSectionName); }

protected:
	virtual void OnBeforeChangeVisualTheme(CBCGPAppOptions& /*appOptions*/, CWnd* /*pMainWnd*/) {}
	virtual void OnAfterChangeVisualTheme(CWnd* /*pMainWnd*/) {}
	virtual BOOL OnCustomizeToolBars();
	virtual void OnBeforeCreateCustomizationDlg(CBCGPToolbarCustomize* /*pDlgCust*/) {}

	virtual void PreSaveState();

protected:
	BOOL				m_bDPIAware;
	BOOL				m_bMSAASupport;
	BOOL				m_bThemedTooltips;
	BOOL				m_bKeyboardManager;
	BOOL				m_bContextMenuManager;
	BOOL				m_bShellManager;
	BCGP_VISUAL_THEME	m_ActiveTheme;
	CWnd*				m_pMainWndTemp;		// Temporary
	
	BOOL				m_bReloadRecentTheme;
	CString				m_strRecentThemeRegistry;

	CMap<UINT, UINT, BCGP_VISUAL_THEME, BCGP_VISUAL_THEME>	m_mapVisualThemeCmds;

	CBCGPToolBarImages	m_ToolbarCustomIcons;
	CBCGPAppOptions		m_AppOptions;

private:
	CBCGPToolbarOptions	m_ToolbarOptions;
};

#endif // !defined(AFX_BCGPWORKSPACE_H__C82F6ED0_8263_11D3_A9EF_005056800000__INCLUDED_)
