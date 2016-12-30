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
// bcgtoolbar.h : definition of CBCGPToolBar
//
// This code is based on the Microsoft Visual C++ sample file
// TOOLBAR.C from the OLDBARS example
//

#ifndef _TOOLBAR_H_
#define _TOOLBAR_H_

#ifndef __AFXTEMPL_H__
	#include "afxtempl.h"
#endif

#include "BCGCBPro.h"
#include "BCGPToolbarDropTarget.h"
#include "BCGPToolbarDropSource.h"
#include "BCGPToolBarImages.h"
#include "BCGPToolbarButton.h"
#include "BCGPControlBarImpl.h"
#include "CmdUsageCount.h"
#include "BCGPCommandManager.h"
#include "BCGPVisualManager.h"
#include "BCGGlobals.h"

#include "BCGPBaseToolBar.h"

class CBCGPToolbarMenuButton;
class CCustomizeButton;
class CBCGPRegistry;

//-----------------------------------------------
// CBCGPToolbar LoadBitmap/LoadToolbar parameters:
//-----------------------------------------------
class BCGCBPRODLLEXPORT CBCGPToolBarParams
{
public:
	CBCGPToolBarParams();

	UINT	m_uiColdResID;			// Toolbar regular images
	UINT	m_uiHotResID;			// Toolbar "hot" images
	UINT	m_uiDisabledResID;		// Toolbar disabled images
	UINT	m_uiLargeColdResID;		// Toolbar large regular images
	UINT	m_uiLargeHotResID;		// Toolbar large "hot" images
	UINT	m_uiLargeDisabledResID;	// Toolbar large disabled images
	UINT	m_uiMenuResID;			// Menu images
	UINT	m_uiMenuDisabledResID;	// Menu disabled images
};

//----------------------------------
// BCGPToolbar notification messages:
//----------------------------------
BCGCBPRODLLEXPORT extern UINT BCGM_TOOLBARMENU;
BCGCBPRODLLEXPORT extern UINT BCGM_CUSTOMIZETOOLBAR;
BCGCBPRODLLEXPORT extern UINT BCGM_CREATETOOLBAR;
BCGCBPRODLLEXPORT extern UINT BCGM_DELETETOOLBAR;
BCGCBPRODLLEXPORT extern UINT BCGM_CUSTOMIZEHELP;
BCGCBPRODLLEXPORT extern UINT BCGM_RESETTOOLBAR;
BCGCBPRODLLEXPORT extern UINT BCGM_RESETMENU;
BCGCBPRODLLEXPORT extern UINT BCGM_SHOWREGULARMENU;
BCGCBPRODLLEXPORT extern UINT BCGM_RESETCONTEXTMENU;
BCGCBPRODLLEXPORT extern UINT BCGM_RESETKEYBOARD;
BCGCBPRODLLEXPORT extern UINT BCGM_RESETRPROMPT;

extern const UINT uiAccTimerDelay;   
extern const UINT uiAccNotifyEvent;

#define LINE_OFFSET			5

class BCGCBPRODLLEXPORT CBCGPToolBar : public CBCGPBaseToolBar
{
	friend class CBCGPToolbarDropTarget;
	friend class CBCGPToolbarsPage;
	friend class CBCGPOptionsPage;
	friend class CButtonsTextList;
	friend class CBCGPCommandManager;
	friend class CCustomizeButton;
	friend class CBCGPCustomizeMenuButton;
	friend class CBCGPToolTipCtrl;

	DECLARE_SERIAL(CBCGPToolBar)

	//--------------
	// Construction:
	//--------------
public:
	CBCGPToolBar();
	virtual BOOL Create(CWnd* pParentWnd,
			DWORD dwStyle = dwDefaultToolbarStyle,
			UINT nID = AFX_IDW_TOOLBAR);
	virtual BOOL CreateEx(CWnd* pParentWnd, DWORD dwCtrlStyle = TBSTYLE_FLAT,
		DWORD dwStyle = dwDefaultToolbarStyle,
		CRect rcBorders = CRect(1, 1, 1, 1),
		UINT nID = AFX_IDW_TOOLBAR);

	//----------------------------------------------------------------
	// Set toolbar buttons image resources.
	// You should use SINGLE CBCGPToolBarImages for ALL your toolbars!
	//----------------------------------------------------------------
	static BOOL SetUserImages (CBCGPToolBarImages* pUserImages, BOOL bAdjustSize = FALSE);

	virtual void ResetImages ();	// Reset all toolbar images exept user-derfined to the default

	//------------------------------
	// Clear all images except user:
	//------------------------------
	static void ResetAllImages();

	//--------------------------------
	// Dimension manipulation methods:
	//--------------------------------
	static void SetSizes (SIZE sizeButton, SIZE sizeImage);
	static void SetMenuSizes (SIZE sizeButton, SIZE sizeImage);
	static CSize GetMenuImageSize ();
	static CSize GetMenuButtonSize ();

	void SetLockedSizes (SIZE sizeButton, SIZE sizeImage, BOOL bDontScale = FALSE);
	void SetHeight (int cyHeight);

	//-----------------
	// Toolbar context:
	//-----------------
	virtual BOOL LoadBitmap (UINT uiResID, UINT uiColdResID = 0, 
					UINT uiMenuResID = 0, BOOL bLocked = FALSE,
					UINT uiDisabledResID = 0, UINT uiMenuDisabledResID = 0);
	virtual BOOL LoadToolBar (UINT uiResID, UINT uiColdResID = 0, 
					UINT uiMenuResID = 0, BOOL bLocked = FALSE,
					UINT uiDisabledResID = 0, UINT uiMenuDisabledResID = 0,
  				    UINT uiHotResID = 0);
	virtual BOOL LoadBitmapEx (CBCGPToolBarParams& params, BOOL bLocked = FALSE);
	virtual BOOL LoadToolBarEx (UINT uiToolbarResID, CBCGPToolBarParams& params, 
								BOOL bLocked = FALSE);

	//----------------------------
	// Toolbar buttons add/remove:
	//----------------------------
	virtual BOOL SetButtons(const UINT* lpIDArray, int nIDCount, BOOL bRemapImages = TRUE);

	virtual int InsertButton (const CBCGPToolbarButton& button, INT_PTR iInsertAt = -1);
	virtual int InsertSeparator (INT_PTR iInsertAt = -1);

	int ReplaceButton (UINT uiCmd, const CBCGPToolbarButton& button, BOOL bAll = FALSE);
	
	virtual BOOL RemoveButton (int iIndex);
	virtual void RemoveAllButtons ();

	static BOOL IsLastCommandFromButton (CBCGPToolbarButton* pButton);
	static BOOL AddToolBarForImageCollection (UINT uiResID, UINT uiBmpResID = 0,
									UINT uiColdResID = 0, UINT uiMenuResID = 0,
									UINT uiDisabledResID = 0, UINT uiMenuDisabledResID = 0);

	static void SetNonPermittedCommands (CList<UINT, UINT>& lstCommands);
	static BOOL IsCommandPermitted (UINT uiCmd)
	{
		return m_lstUnpermittedCommands.Find (uiCmd) == NULL;
	}

	static void SetBasicCommands (CList<UINT, UINT>& lstCommands);
	static void AddBasicCommand (UINT uiCmd);
	
	static BOOL IsBasicCommand (UINT uiCmd)
	{
		return m_lstBasicCommands.Find (uiCmd) != NULL;
	}

	static const CList<UINT, UINT>& GetBasicCommands ()
	{
		return m_lstBasicCommands;
	}

	static BOOL IsCommandRarelyUsed (UINT uiCmd);

	static void AddCommandUsage (UINT uiCommand);
	static BOOL SetCommandUsageOptions (UINT nStartCount, UINT nMinUsagePercentage = 5);

	virtual int GetRowHeight () const
	{
		if (m_bDrawTextLabels)
		{
			ASSERT (m_nMaxBtnHeight > 0);
			return m_nMaxBtnHeight;
		}

		return max (globalData.GetTextHeight (m_dwStyle & CBRS_ORIENT_HORZ),
			(m_bMenuMode ?
			(m_sizeMenuButton.cy > 0 ?
				m_sizeMenuButton.cy : m_sizeButton.cy) :
			GetButtonSize ().cy));
	}

	virtual int GetColumnWidth () const
	{
		return m_bMenuMode ?
			m_sizeMenuButton.cx > 0 ?
				m_sizeMenuButton.cx : m_sizeButton.cx :
			GetButtonSize ().cx;
	}

	virtual BOOL IsButtonExtraSizeAvailable () const
	{
		return TRUE;
	}

	static void SetHelpMode (BOOL bOn = TRUE);
	virtual void Deactivate ();
	virtual void RestoreFocus ();

	void SetToolBarBtnText (UINT nBtnIndex,
							LPCTSTR szText = NULL,
							BOOL bShowText = TRUE,
							BOOL bShowImage = TRUE);

	virtual BOOL CanFocus () const {return FALSE;}

	void EnableLargeIcons (BOOL bEnable);

	static void SetLargeIcons (BOOL bLargeIcons = TRUE);
	static BOOL IsLargeIcons ()
	{
		return m_bLargeIcons;
	}

	virtual void OnLargeIconsModeChanged() {}

	static void AutoGrayInactiveImages (BOOL bEnable = TRUE, 
										int nGrayPercentage = 0,
										BOOL bRedrawAllToolbars = TRUE);
	static BOOL IsAutoGrayInactiveImages ()
	{
		return m_bAutoGrayInactiveImages;
	}

	CSize GetButtonSize () const
	{
		CSize size = m_bLocked ?
			(m_bLargeIconsAreEnbaled ? m_sizeCurButtonLocked : m_sizeButtonLocked) :
			(m_bLargeIconsAreEnbaled ? m_sizeCurButton : m_sizeButton);
		
		if (IsButtonExtraSizeAvailable ())
		{
			size += CBCGPVisualManager::GetInstance ()->GetButtonExtraBorder ();
		}

		return size;
	}

	CSize GetImageSize () const
	{
		return m_bLocked ?
			m_bLargeIconsAreEnbaled ? m_sizeCurImageLocked : m_sizeImageLocked :
			m_bLargeIconsAreEnbaled ? m_sizeCurImage : m_sizeImage;
	}

	CSize GetLockedImageSize () const
	{
		if (!m_bLocked)
		{
			ASSERT (FALSE);
			return CSize (0, 0);
		}

		return m_sizeImageLocked;
	}

	BOOL IsButtonHighlighted (int iButton) const;

	//------------
	// Attributes:
	//------------
public: 
	BOOL IsLocked () const
	{
		return m_bLocked;
	}

	UINT GetResourceID () const
	{
		return m_uiOriginalResID;
	}

	virtual BOOL IsResourceChanged () const
	{
		return m_bResourceWasChanged;
	}

	virtual BOOL IsFloating () const {return m_bFloating;}

	static const CObList& GetAllToolbars ();

	// standard control bar things
	int CommandToIndex(UINT nIDFind, int iIndexFirst = 0) const;
	UINT GetItemID(int nIndex) const;
	
	virtual void GetItemRect(int nIndex, LPRECT lpRect) const;
	virtual void GetInvalidateItemRect(int nIndex, LPRECT lpRect) const;

	UINT GetButtonStyle(int nIndex) const;
	virtual void SetButtonStyle(int nIndex, UINT nStyle);

	int GetCount () const;
	int ButtonToIndex(const CBCGPToolbarButton* pButton) const;
	CBCGPToolbarButton* GetButton (int iIndex) const;

	const CObList& GetAllButtons() const { return m_Buttons; }
	const CObList& GetOrigButtons() const { return m_OrigButtons; }
	const CObList& GetOrigResetButtons() const { return m_OrigResetButtons; }

	void SetOrigButtons (const CObList& lstOrigButtons);

	// Find all buttons specified by the given command ID from the all
	// toolbars:
	static int GetCommandButtons (UINT uiCmd, CObList& listButtons);
	
	static int GetCommandRangeButtons (UINT uiFirstID, UINT uiLastID, CObList& listButtons);

	static BOOL SetCustomizeMode (BOOL bSet = TRUE);
	static BOOL IsCustomizeMode ()
	{
		return m_bCustomizeMode;
	}

	static BOOL IsAltCustomizeMode ()
	{
		return m_bAltCustomizeMode;
	}

	BOOL IsUserDefined () const;

	static CBCGPToolBar* FromHandlePermanent (HWND hwnd);
	static CBCGPToolBarImages* GetImages ()
	{
		return &m_Images;
	}
	static CBCGPToolBarImages* GetColdImages ()
	{
		return &m_ColdImages;
	}
	static CBCGPToolBarImages* GetDisabledImages ()
	{
		return &m_DisabledImages;
	}
	static CBCGPToolBarImages* GetLargeImages ()
	{
		return &m_LargeImages;
	}
	static CBCGPToolBarImages* GetLargeColdImages ()
	{
		return &m_LargeColdImages;
	}
	static CBCGPToolBarImages* GetLargeDisabledImages ()
	{
		return &m_LargeDisabledImages;
	}
	static CBCGPToolBarImages* GetMenuImages ()
	{
		return &m_MenuImages;
	}
	static CBCGPToolBarImages* GetDisabledMenuImages ()
	{
		return &m_DisabledMenuImages;
	}
	static CBCGPToolBarImages* GetUserImages ()
	{
		return m_pUserImages;
	}

	CBCGPToolBarImages*	GetLockedImages ()
	{
		if (!m_bLocked)
		{
			ASSERT (FALSE);
			return NULL;
		}

		return &m_ImagesLocked;
	}

	CBCGPToolBarImages*	GetLockedColdImages ()
	{
		if (!m_bLocked)
		{
			ASSERT (FALSE);
			return NULL;
		}

		return &m_ColdImagesLocked;
	}

	CBCGPToolBarImages*	GetLockedDisabledImages ()
	{
		if (!m_bLocked)
		{
			ASSERT (FALSE);
			return NULL;
		}

		return &m_DisabledImagesLocked;
	}
	
	CBCGPToolBarImages* GetLockedMenuImages ()
	{
		if (!m_bLocked)
		{
			ASSERT (FALSE);
			return NULL;
		}

		if (m_MenuImagesLocked.GetCount () > 0)
		{
			return &m_MenuImagesLocked;
		}

		if (m_ImagesLocked.GetCount () > 0)
		{
			return &m_ImagesLocked;
		}

		return NULL;
	}

	static int GetDefaultImage (UINT uiID)
	{
		int iImage;
		if (m_DefaultImages.Lookup (uiID, iImage))
		{
			return iImage;
		}

		return -1;
	}

	int GetImagesOffset () const
	{
		return m_iImagesOffset;
	}

	CBCGPToolbarButton* GetHighlightedButton () const;

	static void SetHotTextColor (COLORREF clrText);
	static COLORREF GetHotTextColor ();

	void SetHotBorder (BOOL bShowHotBorder)
	{
		m_bShowHotBorder = bShowHotBorder;
	}

	BOOL GetHotBorder () const
	{
		return m_bShowHotBorder;
	}

	void SetGrayDisabledButtons (BOOL bGrayDisabledButtons)
	{
		m_bGrayDisabledButtons = bGrayDisabledButtons;
	}

	BOOL GetGrayDisabledButtons () const
	{
		return m_bGrayDisabledButtons;
	}

	//------------------------------------------------------
	// Enable/disable quick customization mode ("Alt+drag"):
	//------------------------------------------------------
	static void EnableQuickCustomization (BOOL bEnable = TRUE)
	{
		m_bAltCustomization = bEnable;
	}

	static void SetLook2000 (BOOL bLook2000 = TRUE);
	static BOOL IsLook2000 ();

	virtual void EnableDocking (DWORD dwAlignment); 
	
	void EnableCustomizeButton (BOOL bEnable, UINT uiCustomizeCmd, const CString& strCustomizeText, BOOL bQuickCustomize = TRUE);
	void EnableCustomizeButton (BOOL bEnable, UINT uiCustomizeCmd, UINT uiCustomizeTextResId, BOOL bQuickCustomize = TRUE);

	BOOL IsExistCustomizeButton()
	{
		if(m_pCustomizeBtn == NULL)
		{
			return FALSE;
		}
		return TRUE;
	}

	CCustomizeButton* GetCustomizeButton()
	{
		return m_pCustomizeBtn;
	}

	void EnableTextLabels (BOOL bEnable = TRUE);

	virtual BOOL AllowChangeTextLabels () const
	{
		return TRUE;
	}

	BOOL AreTextLabels () const
	{
		return m_bTextLabels;
	}

	virtual BOOL OnBeforeRemoveButton (CBCGPToolbarButton* /*pButton*/, DROPEFFECT /*dropEffect*/)
	{
		return TRUE;
	}

	void SetMaskMode (BOOL bMasked)
	{
		m_bMasked = bMasked;
	}

	void SetPermament (BOOL bPermament = TRUE)
	{
		m_bPermament = bPermament;
	}

	BOOL GetIgnoreSetText () const
	{
		return m_bIgnoreSetText;
	}

	void SetIgnoreSetText (BOOL bValue)
	{
		m_bIgnoreSetText = bValue;
	}

	BOOL GetRouteCommandsViaFrame () 
	{
		return m_bRouteCommandsViaFrame;
	}

	void SetRouteCommandsViaFrame (BOOL bValue)
	{
		m_bRouteCommandsViaFrame = bValue;
	}

	BOOL IsAddRemoveQuickCustomize()
	{
		return m_bQuickCustomize;
	}

	BOOL IsBrother()
	{
		return m_bHasBrother;
	}

	CBCGPToolBar* GetBrotherToolbar()
	{
		return m_pBrotherToolBar;
	}

	void SetBrotherToolbar(CBCGPToolBar* pBrotherToolbar);
	BOOL IsOneRowWithBrother();
	void SetOneRowWithBrother();
	void SetTwoRowsWithBrother();
	BOOL CanHandleBrothers();

	void EnableReflections(BOOL bEnable = TRUE)
	{
		m_bAllowReflections = bEnable;
	}

	static BOOL GetShowTooltips()				{return m_bShowTooltips;}
	static void SetShowTooltips(BOOL bValue)	{m_bShowTooltips = bValue;}

	HWND GetHwndLastFocus() const				{return m_hwndLastFocus;}; 

	static BOOL m_bDisableLabelsEdit;

public:
	// for changing button info
	void GetButtonInfo(int nIndex, UINT& nID, UINT& nStyle, int& iImage) const;
	void SetButtonInfo(int nIndex, UINT nID, UINT nStyle, int iImage);

	BOOL SetButtonText(int nIndex, LPCTSTR lpszText);
	CString GetButtonText( int nIndex ) const;
	void GetButtonText( int nIndex, CString& rString ) const;

	// Save/load toolbar state + buttons:
	void Serialize (CArchive& ar);
	virtual BOOL LoadState (LPCTSTR lpszProfileName = NULL, int nIndex = -1, UINT uiID = (UINT) -1);
	virtual BOOL SaveState (LPCTSTR lpszProfileName = NULL, int nIndex = -1, UINT uiID = (UINT) -1);
	virtual BOOL RemoveStateFromRegistry (LPCTSTR lpszProfileName = NULL, int nIndex = -1, UINT uiID = (UINT) -1);
	static BOOL LoadParameters (LPCTSTR lpszProfileName = NULL);
	static BOOL SaveParameters (LPCTSTR lpszProfileName = NULL);
	static BOOL LoadLargeIconsState (LPCTSTR lpszProfileName = NULL);

	virtual BOOL CanBeRestored () const;
	virtual BOOL CanBeClosed () const
	{
		return !m_bPermament;
	}

	virtual BOOL RestoreOriginalstate ();
	virtual void OnReset () {}

	static void ResetAll ();
	static void RedrawUnderlines ();

	virtual void AdjustLayout ();
	virtual int HitTest(CPoint point);
	virtual BOOL TranslateChar (UINT nChar);
	virtual void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler);

	virtual BOOL PrevMenu ();
	virtual BOOL NextMenu ();
	BOOL ProcessCommand (CBCGPToolbarButton* pButton);
	CBCGPToolbarMenuButton* GetDroppedDownMenu (int* pIndex = NULL) const;

	BOOL SetHot (CBCGPToolbarButton *pMenuButton);

	virtual BOOL OnSetDefaultButtonText (CBCGPToolbarButton* pButton);

	BOOL IsDragButton (const CBCGPToolbarButton* pButton) const
	{
		return pButton == m_pDragButton;
	}

	virtual void  OnFillBackground (CDC* /*pDC*/) {}
	virtual void OnGlobalFontsChanged ();

	static BOOL		m_bExtCharTranslation;
	static double	m_dblLargeImageRatio;	// large image stretch ratio (default: * 2)

	static BOOL		m_bDontScaleImages;		// don't scale shared images in high DPI mode

	static void CBCGPToolBar::CleanUpImages ();
	void CleanUpLockedImages ();

	void AdjustSize ();

	virtual BOOL OnUserToolTip (CBCGPToolbarButton* pButton, CString& strTTText) const;

	virtual BOOL IsPopupMode() const
	{
		return FALSE;
	}

// Implementation
public:
	virtual ~CBCGPToolBar();

	virtual CSize CalcFixedLayout(BOOL bStretch, BOOL bHorz);

	virtual CSize CalcSize (BOOL bVertDock);
	int WrapToolBar (int nWidth, int nHeight = 32767,
		CDC* pDC = NULL,
		int nColumnWidth = -1, int nRowHeight = -1);

	virtual void OnChangeHot (int iHot);

	virtual CSize StretchControlBar (int nLength, BOOL bVert);


	CBCGPToolbarButton* InvalidateButton(int nIndex);
	void UpdateButton(int nIndex);

	virtual void OnChangeVisualManager ();

protected:
	virtual CSize CalcLayout (DWORD dwMode, int nLength = -1);
	void  SizeToolBar (int nLength, BOOL bVert = FALSE);

public:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	virtual int InsertButton (CBCGPToolbarButton* pButton, int iInsertAt = -1);

	virtual BOOL DrawButton (CDC* pDC, CBCGPToolbarButton* pButton, 
		CBCGPToolBarImages* pImages, BOOL bHighlighted,
		BOOL bDrawDisabledImages);
	virtual void CheckForButtonImages(CBCGPToolbarButton* /*pButton*/, CBCGPToolBarImages** /*pNewImages*/) {}

	virtual void DrawDragMarker (CDC* pDC);
	CBCGPToolBarImages* GetImageList (CBCGPToolBarImages& images, CBCGPToolBarImages& imagesLocked, 
							 CBCGPToolBarImages& largeImages, CBCGPToolBarImages& largeImagesLocked) const;

	virtual void RebuildAccelerationKeys ();
	virtual CWnd* GetCommandTarget () const
	{
		// determine target of command update
		CFrameWnd* pTarget = (CFrameWnd*) GetOwner();
		if (pTarget == NULL || (m_bRouteCommandsViaFrame && !pTarget->IsFrameWnd ()))
		{
			pTarget = BCGPGetParentFrame (this);
		}

		return pTarget;
	}

	void UpdateTooltips ();

	virtual void OnAfterFloat ();
	virtual void OnAfterDock  (CBCGPBaseControlBar* /*pBar*/, LPCRECT /*lpRect*/, BCGP_DOCK_METHOD /*dockMethod*/);
	virtual void OnBeforeChangeParent (CWnd* pWndNewParent, BOOL bDelay = FALSE);

protected:
	friend class CBCGPWorkspace;

	static CBCGPToolBar*		m_pSelToolbar;			// "Selected" toolbar in the customization mode

	static CBCGPToolBarImages	m_Images;				// Shared toolbar images
	static CBCGPToolBarImages	m_ColdImages;			// Shared toolbar "cold" images
	static CBCGPToolBarImages	m_DisabledImages;		// Shared disabled images

	static CBCGPToolBarImages	m_LargeImages;			// Shared toolbar large "hot" images
	static CBCGPToolBarImages	m_LargeColdImages;		// Shared toolbar large "cold" images
	static CBCGPToolBarImages	m_LargeDisabledImages;	// Shared disabled large images

	static CBCGPToolBarImages	m_MenuImages;
	static CBCGPToolBarImages	m_DisabledMenuImages;

	static BOOL	m_bAutoGrayInactiveImages;
	static int	m_nGrayImagePercentage;

	static CBCGPToolBarImages*	m_pUserImages;			// Shared user-defined images

	CBCGPToolBarImages	m_ImagesLocked;					// "Locked" toolbar images
	CBCGPToolBarImages	m_ColdImagesLocked;				// "Locked" toolbar "cold" images
	CBCGPToolBarImages	m_DisabledImagesLocked;			// "Locked" toolbar disabled images
	CBCGPToolBarImages	m_LargeImagesLocked;			// "Locked" toolbar large images
	CBCGPToolBarImages	m_LargeColdImagesLocked;		// "Locked" toolbar large "cold" images
	CBCGPToolBarImages	m_LargeDisabledImagesLocked;	// "Locked" toolbar large disabled images
	CBCGPToolBarImages	m_MenuImagesLocked;				// "Locked" toolbar menu images
	CBCGPToolBarImages	m_DisabledMenuImagesLocked;		// "Locked" toolbar menu disabled images

	BOOL				m_bLocked;
	BOOL				m_bLargeIconsAreEnbaled;

	CBCGPControlBarImpl	m_Impl;

	BOOL				m_bMasked;
	BOOL				m_bPermament;	// Can't be closed

	BOOL				m_bTextLabels;	// Text labels below the image are available
	BOOL				m_bDrawTextLabels;
	int					m_nMaxBtnHeight;// Actual only if m_bTextLabels is TRUE

	static CMap<UINT, UINT, int, int>	m_DefaultImages;

	static CSize m_sizeButton;			// original size of button
	static CSize m_sizeImage;			// original size of glyph
	static CSize m_sizeMenuButton;		// size of button on the menu
	static CSize m_sizeMenuImage;		// size of image on the menu
	static CSize m_sizeCurButton;		// size of button
	static CSize m_sizeCurImage;		// size of glyph

	CSize m_sizeButtonLocked;			// original size of button of the locked toolbars
	CSize m_sizeImageLocked;			// original size of glyph of the locked toolbars
	CSize m_sizeCurButtonLocked;		// size of button
	CSize m_sizeCurImageLocked;			// size of glyph
	BOOL m_bDontScaleLocked;			// don't scale locked images in high DPI mode

	int m_iButtonCapture;				// index of button with capture (-1 => none)
	int m_iHighlighted;					// highlighted button index
	int m_iSelected;					// selected button index
	int	m_iHot;

	CObList		m_Buttons;
	CObList		m_OrigButtons;			// Original (not customized) items
	CObList		m_OrigResetButtons;		// Original (not customized) items after reset

	BOOL		m_bResourceWasChanged;	// Resource was changed since last session

	BOOL		m_bLeaveFocus;			// Don't remove selection in the focused bars.

	BOOL		m_bFloating;

	CBCGPToolbarDropTarget	m_DropTarget;
	static CBCGPToolbarDropSource m_DropSource;

	BOOL		m_bNoDropTarget;

	static BOOL m_bCustomizeMode;
	static BOOL m_bAltCustomizeMode;

	CToolTipCtrl*			m_pToolTip;
	int						m_nTooltipsCount;

	int			m_iDragIndex;
	CRect		m_rectDrag;
	CPen		m_penDrag;
	CBCGPToolbarButton* m_pDragButton;
	CPoint		m_ptStartDrag;
	BOOL		m_bIsDragCopy;

	BOOL		m_bStretchButton;
	CRect		m_rectTrack;

	int			m_iImagesOffset;
	UINT		m_uiOriginalResID;	// Toolbar resource ID

	BOOL		m_bTracked;
	CPoint		m_ptLastMouse;

	BOOL		m_bMenuMode;

	CWnd*		m_pWndLastCapture;
	HWND		m_hwndLastFocus;

	BOOL		m_bDisableControlsIfNoHandler;
	BOOL		m_bRouteCommandsViaFrame;
	BOOL		m_bDisableCustomize;

	CSize		m_sizeLast;

	static COLORREF	m_clrTextHot;
	
	static HHOOK m_hookMouseHelp;	// Mouse hook for the help mode
	static CBCGPToolBar* m_pLastHookedToolbar;

	CMap<UINT, UINT&, CBCGPToolbarButton*, CBCGPToolbarButton*&>	m_AcellKeys;	// Keyborad acceleration keys

	static BOOL m_bShowTooltips;
	static BOOL m_bShowShortcutKeys;
	static BOOL m_bLargeIcons;

	static CList<UINT, UINT>	m_lstUnpermittedCommands;
	static CList<UINT, UINT>	m_lstBasicCommands;

	static CCmdUsageCount	m_UsageCount;

	BOOL		m_bShowHotBorder;
	BOOL		m_bGrayDisabledButtons;
	BOOL		m_bIgnoreSetText;

	int			m_nMaxLen;

	static BOOL	m_bAltCustomization;

	CCustomizeButton*	m_pCustomizeBtn;

	BOOL				m_bQuickCustomize;


	BOOL				m_bHasBrother;
	BOOL				m_bElderBrother;
	CBCGPToolBar*		m_pBrotherToolBar;

	BOOL				m_bAllowReflections;

	int					m_iAccHotItem;

	BOOL				m_bRoundShape;
	BOOL				m_bInUpdateShadow;

	virtual BOOL OnDrop(COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point);
	virtual DROPEFFECT OnDragEnter(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	virtual void OnDragLeave();
	virtual DROPEFFECT OnDragOver(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);

	virtual void DoPaint(CDC* pDC);
	virtual INT_PTR OnToolHitTest(CPoint point, TOOLINFO* pTI) const;
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	virtual BOOL NotifyControlCommand (CBCGPToolbarButton* pButton,
										 BOOL bAccelerator,
										 int nNotifyCode,
										 WPARAM wParam,
										 LPARAM lParam);

	virtual int FindDropIndex (const CPoint point, CRect& rectDrag) const;
	virtual void AdjustLocations ();

	virtual BOOL OnSendCommand (const CBCGPToolbarButton* /*pButton*/)	{	return FALSE;	}

	virtual BOOL AllowSelectDisabled () const	{	return FALSE;	}
	virtual BOOL AllowShowOnList () const		{	return TRUE;	}
	virtual BOOL AlwaysSaveSelection () const	{	return FALSE;	}

	virtual void DrawSeparator (CDC* pDC, const CRect& rect, BOOL bHorz);
	virtual CBCGPToolbarButton* CreateDroppedButton (COleDataObject* pDataObject);
	virtual BOOL OnKey (UINT /*nChar*/)				{	return FALSE;	}
	virtual void OnCustomizeMode (BOOL bSet);

	virtual BOOL EnableContextMenuItems (CBCGPToolbarButton* pButton, CMenu* pPopup);
	virtual BOOL IsPureMenuButton (CBCGPToolbarButton* /*pButton*/) const
	{
		return m_bMenuMode;
	}

	virtual void OnCalcSeparatorRect (CBCGPToolbarButton* pButton, 
										CRect& rectSeparator, 
										BOOL bHorz);

	virtual void AddRemoveSeparator (const CBCGPToolbarButton* pButton,
						const CPoint& ptStart, const CPoint& ptDrop);
	virtual void ShowCommandMessageString (UINT uiCmdId);

	static LRESULT CALLBACK BCGToolBarMouseProc (int nCode, WPARAM wParam, LPARAM lParam);

	BOOL DropDownMenu (CBCGPToolbarButton* pButton);

	virtual int CalcMaxButtonHeight ();

	virtual BOOL AllowAltCustomization () const	{	return TRUE;	}
	virtual void OnAfterButtonDelete ();

	void SetRoundedRgn ();
	void RedrawCustomizeButton ();

	void UpdateImagesColor ();

	virtual BOOL OnSetAccData (long lVal);
	void AccNotifyObjectFocusEvent (int iButtton);
	virtual HRESULT get_accChildCount(long *pcountChildren);
	virtual HRESULT get_accChild(VARIANT varChild, IDispatch **ppdispChild);
	virtual HRESULT get_accRole(VARIANT varChild, VARIANT *pvarRole);
	virtual HRESULT get_accState(VARIANT varChild, VARIANT *pvarState);
	virtual HRESULT accHitTest(long xLeft, long yTop, VARIANT *pvarChild);
	virtual HRESULT accNavigate(long navDir, VARIANT varStart, VARIANT *pvarEndUpAt);
	virtual HRESULT accDoDefaultAction(VARIANT varChild);
	virtual int AccGetButtonsCount();
	CBCGPToolbarButton* AccGetButtonByChildId(long lVal);
	int AccGetChildIdByButtonIndex(int nButtonIndex);

	// SmartUpdate methods:
	virtual void SaveOriginalState (CBCGPRegistry& reg);
	virtual BOOL LoadLastOriginalState (CBCGPRegistry& reg);
	virtual BOOL SmartUpdate (const CObList& lstPrevButtons);

	void SaveResetOriginalState (CBCGPRegistry& reg);
	BOOL LoadResetOriginalState (CBCGPRegistry& reg);

	BOOL RemoveResetStateButton (UINT uiCmdId);
	int  InsertResetStateButton (const CBCGPToolbarButton& button, int iInsertAt);

	//{{AFX_MSG(CBCGPToolBar)
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnCancelMode();
	afx_msg void OnSysColorChange();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnToolbarAppearance();
	afx_msg void OnToolbarDelete();
	afx_msg void OnToolbarImage();
	afx_msg void OnToolbarImageAndText();
	afx_msg void OnToolbarStartGroup();
	afx_msg void OnToolbarText();
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnWindowPosChanged(WINDOWPOS FAR* lpwndpos);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnBcgbarresToolbarReset();
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp);
	afx_msg void OnNcPaint();
	afx_msg void OnBcgbarresCopyImage();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnBcgbarresToolbarNewMenu();
	afx_msg void OnWindowPosChanging(WINDOWPOS FAR* lpwndpos);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG
	afx_msg void OnContextMenu(CWnd*, CPoint point);
	afx_msg LRESULT OnMouseLeave(WPARAM,LPARAM);
	afx_msg LRESULT OnHelpHitTest(WPARAM,LPARAM);
	afx_msg LRESULT OnGetButtonCount(WPARAM,LPARAM);
	afx_msg LRESULT OnGetItemRect(WPARAM,LPARAM);
	afx_msg LRESULT OnGetButton(WPARAM,LPARAM);
	afx_msg LRESULT OnGetButtonText(WPARAM,LPARAM);
	afx_msg BOOL OnNeedTipText(UINT id, NMHDR* pNMH, LRESULT* pResult);
	afx_msg LRESULT OnPromptReset(WPARAM, LPARAM);
	afx_msg BCGNcHitTestType OnNcHitTest(CPoint point);
	afx_msg LRESULT OnBCGUpdateToolTips(WPARAM, LPARAM);
	afx_msg LRESULT OnBCGSetControlAero (WPARAM, LPARAM);
	DECLARE_MESSAGE_MAP()
};

#endif //!_TOOLBAR_H_
