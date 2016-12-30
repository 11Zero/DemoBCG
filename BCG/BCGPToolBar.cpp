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

// bcgtoolbar.cpp : definition of CBCGPToolBar
//
// This code is based on the Microsoft Visual C++ sample file
// TOOLBAR.C from the OLDBARS example
//

#include "stdafx.h"

#include "bcgprores.h"
#include "BCGPToolBar.h"
#include "BCGPMenuBar.h"
#include "BCGPToolbarButton.h"
#include "BCGPToolbarDropSource.h"
#include "ButtonAppearanceDlg.h"
#include "BCGPToolbarCustomize.h"
#include "BCGPLocalResource.h"
#include "BCGPRegistry.h"
#include "BCGPMDIFrameWnd.h"
#include "BCGPMDIChildWnd.h"
#include "BCGPFrameWnd.h"
#include "BCGPKeyboardManager.h"
#include "BCGPToolbarMenuButton.h"
#include "BCGPToolbarSystemMenuButton.h"
#include "BCGPPopupMenu.h"
#include "CustomizeButton.h"
#include "BCGPCommandManager.h"
#include "RegPath.h"
#include "trackmouse.h"
#include "BCGPOleIPFrameWnd.h"
#include "BCGPUserToolsManager.h"
#include "bcgpsound.h"
#include "BCGPVisualManager.h"
#include "BCGPDropDown.h"
#include "BCGPWorkspace.h"

#include "BCGPDockBar.h"
#include "BCGPDockBarRow.h"
#include "BCGPMiniFrameWnd.h"
#include "BCGPReBar.h"

#include "BCGPCustomizeMenuButton.h"
#include "BCGPToolbarMenuButtonsButton.h"
#include "BCGPTooltipManager.h"
#include "BCGPDlgImpl.h"

extern CBCGPWorkspace*	g_pWorkspace;

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

#define TEXT_MARGIN			3
#define STRETCH_DELTA		6
#define BUTTON_MIN_WIDTH	5


#define REG_SECTION_FMT					_T("%sBCGToolBar-%d")
#define REG_SECTION_FMT_EX				_T("%sBCGToolBar-%d%x")
#define REG_PARAMS_FMT					_T("%sBCGToolbarParameters")
#define REG_ENTRY_NAME					_T("Name")
#define REG_ENTRY_BUTTONS				_T("Buttons")
#define REG_ENTRY_ORIG_ITEMS			_T("OriginalItems")
#define REG_ENTRY_TOOLTIPS				_T("Tooltips")
#define REG_ENTRY_KEYS					_T("ShortcutKeys")
#define REG_ENTRY_LARGE_ICONS			_T("LargeIcons")
#define REG_ENTRY_ANIMATION				_T("MenuAnimation")
#define REG_ENTRY_RU_MENUS				_T("RecentlyUsedMenus")
#define REG_ENTRY_MENU_SHADOWS			_T("MenuShadows")
#define REG_ENTRY_SHOW_ALL_MENUS_DELAY	_T("ShowAllMenusAfterDelay")
#define REG_ENTRY_CMD_USAGE_COUNT		_T("CommandsUsage")
#define REG_ENTRY_LOOK2000				_T("Look2000")
#define REG_ENTRY_RESET_ITEMS			_T("OrigResetItems")

static const CString strToolbarProfile	= _T("BCGToolBars");

#ifdef AFX_INIT_SEG
#pragma code_seg(AFX_INIT_SEG)
#endif

/////////////////////////////////////////////////////////////////////////////
// CBCGPToolBar notification messages:

UINT BCGM_TOOLBARMENU		= ::RegisterWindowMessage (_T("BCGTOOLBAR_POPUPMENU"));
UINT BCGM_CUSTOMIZETOOLBAR	= ::RegisterWindowMessage (_T("BCGTOOLBAR_CUSTOMIZE"));
UINT BCGM_CREATETOOLBAR		= ::RegisterWindowMessage (_T("BCGTOOLBAR_CREATE"));
UINT BCGM_DELETETOOLBAR		= ::RegisterWindowMessage (_T("BCGTOOLBAR_DELETE"));
UINT BCGM_CUSTOMIZEHELP		= ::RegisterWindowMessage (_T("BCGTOOLBAR_CUSTOMIZEHELP"));
UINT BCGM_RESETTOOLBAR		= ::RegisterWindowMessage (_T("BCGTOOLBAR_RESETTOOLBAR"));
UINT BCGM_RESETMENU			= ::RegisterWindowMessage (_T("BCGTOOLBAR_RESETMENU"));
UINT BCGM_SHOWREGULARMENU	= ::RegisterWindowMessage (_T("BCGTOOLBAR_SHOWREGULARMENU"));
UINT BCGM_RESETCONTEXTMENU	= ::RegisterWindowMessage (_T("BCGTOOLBAR_RESETCONTEXTMENU"));
UINT BCGM_RESETKEYBOARD		= ::RegisterWindowMessage (_T("BCGTOOLBAR_RESETKEYBAORD"));
UINT BCGM_RESETRPROMPT		= ::RegisterWindowMessage (_T("BCGTOOLBAR_RESETRPROMPT"));

/////////////////////////////////////////////////////////////////////////////
// All CBCGPToolBar collection:
CObList	gAllToolbars;

const UINT uiAccPopupTimerDelay  = 1300; 
const UINT uiAccTimerDelay  = 500;   
const UINT uiAccNotifyEvent = 20;


BOOL CBCGPToolBar::m_bCustomizeMode = FALSE;
BOOL CBCGPToolBar::m_bAltCustomizeMode = FALSE;
BOOL CBCGPToolBar::m_bShowTooltips = TRUE;
BOOL CBCGPToolBar::m_bShowShortcutKeys = TRUE;
BOOL CBCGPToolBar::m_bLargeIcons = FALSE;
BOOL CBCGPToolBar::m_bAutoGrayInactiveImages = FALSE;
int  CBCGPToolBar::m_nGrayImagePercentage = 0;

BOOL CBCGPToolBar::m_bDisableLabelsEdit = FALSE;
CBCGPToolbarDropSource CBCGPToolBar::m_DropSource;

CBCGPToolBarImages	CBCGPToolBar::m_Images;
CBCGPToolBarImages	CBCGPToolBar::m_ColdImages;
CBCGPToolBarImages	CBCGPToolBar::m_MenuImages;
CBCGPToolBarImages	CBCGPToolBar::m_DisabledImages;
CBCGPToolBarImages	CBCGPToolBar::m_DisabledMenuImages;
CBCGPToolBarImages	CBCGPToolBar::m_LargeImages;
CBCGPToolBarImages	CBCGPToolBar::m_LargeColdImages;
CBCGPToolBarImages	CBCGPToolBar::m_LargeDisabledImages;

CBCGPToolBarImages*	CBCGPToolBar::m_pUserImages = NULL;

BOOL CBCGPToolBar::m_bDontScaleImages = FALSE;

CSize CBCGPToolBar::m_sizeButton = CSize (23, 22);
CSize CBCGPToolBar::m_sizeImage	= CSize (16, 15);
CSize CBCGPToolBar::m_sizeCurButton = CSize (23, 22);
CSize CBCGPToolBar::m_sizeCurImage	= CSize (16, 15);
CSize CBCGPToolBar::m_sizeMenuImage	= CSize (-1, -1);
CSize CBCGPToolBar::m_sizeMenuButton	= CSize (-1, -1);

double CBCGPToolBar::m_dblLargeImageRatio = 2.;

BOOL CBCGPToolBar::m_bExtCharTranslation = FALSE;

CMap<UINT, UINT, int, int> CBCGPToolBar::m_DefaultImages;

COLORREF CBCGPToolBar::m_clrTextHot = (COLORREF) -1;
extern CBCGPToolbarCustomize* g_pWndCustomize;

HHOOK CBCGPToolBar::m_hookMouseHelp = NULL;
CBCGPToolBar* CBCGPToolBar::m_pLastHookedToolbar = NULL;

CList<UINT, UINT> CBCGPToolBar::m_lstUnpermittedCommands;
CList<UINT, UINT> CBCGPToolBar::m_lstBasicCommands;

CCmdUsageCount	CBCGPToolBar::m_UsageCount;

BOOL CBCGPToolBar::m_bAltCustomization = FALSE;
CBCGPToolBar* CBCGPToolBar::m_pSelToolbar = NULL;

static inline BOOL IsSystemCommand (UINT uiCmd)
{
	return (uiCmd >= 0xF000 && uiCmd < 0xF1F0);
}

CBCGPToolBarParams::CBCGPToolBarParams()
{
	m_uiColdResID = 0;
	m_uiHotResID = 0;
	m_uiDisabledResID = 0;
	m_uiLargeColdResID = 0;
	m_uiLargeHotResID = 0;
	m_uiLargeDisabledResID = 0;
	m_uiMenuResID = 0;
	m_uiMenuDisabledResID = 0;
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPToolBar

IMPLEMENT_SERIAL(CBCGPToolBar, CBCGPBaseToolBar, VERSIONABLE_SCHEMA | 1)

#pragma warning (disable : 4355)

CBCGPToolBar::CBCGPToolBar() :
	m_bMenuMode (FALSE),
	m_Impl (this),
	m_bIgnoreSetText (FALSE)
{
#if _MSC_VER >= 1300
	if (globalData.IsAccessibilitySupport())
	{
		EnableActiveAccessibility();
	}
#endif

	m_iButtonCapture = -1;      // nothing captured
	m_iHighlighted = -1;
	m_iSelected = -1;
	m_iHot = -1;

	m_iDragIndex = -1;
	m_rectDrag.SetRectEmpty ();
	m_pDragButton = NULL;
	m_ptStartDrag = CPoint (-1, -1);
	m_bIsDragCopy = FALSE;

	m_bMasked = FALSE;
	m_bPermament = FALSE;

	m_pCustomizeBtn = NULL;

	//---------------------
	// UISG standard sizes:
	//---------------------
	m_cyTopBorder = m_cyBottomBorder = 1;   // 1 pixel for top/bottom gaps

	m_sizeCurButtonLocked = CSize (23, 22);
	m_sizeCurImageLocked = CSize (16, 15);
	m_sizeButtonLocked = CSize (23, 22);
	m_sizeImageLocked = CSize (16, 15);
	
	m_bDontScaleLocked = FALSE;

	m_bStretchButton = FALSE;
	m_rectTrack.SetRectEmpty ();

	m_iImagesOffset = 0;
	m_uiOriginalResID = 0;

	m_bTracked = FALSE;
	m_ptLastMouse = CPoint (-1, -1);
	m_pWndLastCapture = NULL;
	m_hwndLastFocus = NULL;

	m_bLocked = FALSE;
	m_bShowHotBorder = TRUE;
	m_bGrayDisabledButtons = TRUE;
	m_bLargeIconsAreEnbaled = TRUE;

	m_bTextLabels = FALSE;
	m_bDrawTextLabels = FALSE;
	m_nMaxBtnHeight = 0;

	m_bDisableControlsIfNoHandler = TRUE;
	m_bRouteCommandsViaFrame = TRUE;

	m_bResourceWasChanged = FALSE;

	m_nTooltipsCount = 0;

	m_nMaxLen = 0;

	m_sizeLast = CSize (0, 0);
	m_bLeaveFocus = TRUE;
	m_bDisableCustomize   = FALSE;

	m_bHasBrother         = FALSE;
	m_bElderBrother		  = FALSE;
	m_pBrotherToolBar     = NULL;

	m_bAllowReflections	  = FALSE;

	m_bQuickCustomize	  = FALSE;

	m_iAccHotItem		  = -10;

	m_bNoDropTarget		  = FALSE;
	m_bFloating			  = FALSE;

	m_bRoundShape		  = FALSE;
	m_bInUpdateShadow	  = FALSE;

	m_pToolTip			  = NULL;
}

#pragma warning (default : 4355)

//******************************************************************************************
CBCGPToolBar::~CBCGPToolBar()
{
	while (!m_OrigButtons.IsEmpty ())
	{
		delete m_OrigButtons.RemoveHead ();
	}

	while(!m_OrigResetButtons.IsEmpty())
	{
		delete m_OrigResetButtons.RemoveHead ();
	}

	RemoveAllButtons ();
}
//******************************************************************************************
BOOL CBCGPToolBar::Create(CWnd* pParentWnd, DWORD dwStyle, UINT nID)
{
	return CBCGPToolBar::CreateEx (pParentWnd, TBSTYLE_FLAT,
							dwStyle,
							CRect(1, 1, 1, 1),
							nID);
}
//******************************************************************************************
BOOL CBCGPToolBar::CreateEx (CWnd* pParentWnd, 
							DWORD dwCtrlStyle,
							DWORD dwStyle,
							CRect rcBorders,
							UINT nID)
{
	dwStyle |= CBRS_GRIPPER; 

	if (pParentWnd != NULL)
	{
		ASSERT_VALID(pParentWnd);   // must have a parent
	}

	if (rcBorders.left < 1)
	{
		rcBorders.left = 1;	// Otherwise, I have a problem with a "double" grippers
	}

	if (rcBorders.top < 1)
	{
		rcBorders.top = 1;	// Otherwise, I have a problem with a "double" grippers
	}

	SetBorders (rcBorders);

	//----------------
	// Save the style:
	//----------------
	m_dwStyle = (dwStyle & CBRS_ALL);

	SetBarAlignment (m_dwStyle);
	if (nID == AFX_IDW_TOOLBAR)
	{
		m_dwStyle |= CBRS_HIDE_INPLACE;
	}

	BOOL bFixed = FALSE;

	dwStyle &= ~CBRS_ALL;

	if (dwStyle & CBRS_SIZE_FIXED)
	{
		bFixed = TRUE;
	}

	dwStyle |= CCS_NOPARENTALIGN|CCS_NOMOVEY|CCS_NODIVIDER|CCS_NORESIZE;
	dwStyle |= dwCtrlStyle;

	if (!bFixed)
	{
		dwStyle &= ~CBRS_SIZE_FIXED;
	}

	//----------------------------
	// Initialize common controls:
	//----------------------------
	VERIFY (AfxDeferRegisterClass (AFX_WNDCOMMCTLS_REG));

	//-----------------
	// Create the HWND:
	//-----------------
	CRect rect;
	rect.SetRectEmpty();

	if (!CBCGPBaseToolBar::Create (
		globalData.RegisterWindowClass (_T("BCGPToolBar")), dwStyle, rect, pParentWnd, nID, 0))
	{
		return FALSE;
	}

	return TRUE;
}
//******************************************************************************************
void CBCGPToolBar::SetSizes (SIZE sizeButton, SIZE sizeImage)
{
	ASSERT(sizeButton.cx > 0 && sizeButton.cy > 0);

	m_sizeButton = sizeButton;
	m_sizeImage = sizeImage;

	m_sizeCurButton = sizeButton;
	m_sizeCurImage = sizeImage;

	m_Images.SetImageSize (m_sizeImage);
	m_ColdImages.SetImageSize (m_sizeImage);
	m_DisabledImages.SetImageSize (m_sizeImage);

	CSize sizeImageLarge (	(int) (.5 + m_dblLargeImageRatio * m_sizeImage.cx), 
							(int) (.5 + m_dblLargeImageRatio * m_sizeImage.cy));

	m_LargeImages.SetImageSize (sizeImageLarge);
	m_LargeColdImages.SetImageSize (sizeImageLarge);
	m_LargeDisabledImages.SetImageSize (sizeImageLarge);

	if (m_bLargeIcons)
	{
		m_sizeCurButton.cx = (int) (.5 + m_dblLargeImageRatio * m_sizeCurButton.cx);
		m_sizeCurButton.cy = (int) (.5 + m_dblLargeImageRatio * m_sizeCurButton.cy);

		m_sizeCurImage.cx = (int) (.5 + m_dblLargeImageRatio * m_sizeCurImage.cx);
		m_sizeCurImage.cy = (int) (.5 + m_dblLargeImageRatio * m_sizeCurImage.cy);
	}

	if (m_pUserImages != NULL && m_pUserImages->GetScale () == 1.)
	{
		if (globalData.GetRibbonImageScale () != 1.)
		{
			m_pUserImages->SetTransparentColor (globalData.clrBtnFace);
			m_pUserImages->SmoothResize (globalData.GetRibbonImageScale ());
		}
		else
		{
			m_pUserImages->SetImageSize (m_sizeImage);
		}
	}
}
//******************************************************************************************
void CBCGPToolBar::SetLockedSizes (SIZE sizeButton, SIZE sizeImage, BOOL bDontScale)
{
	ASSERT(sizeButton.cx > 0 && sizeButton.cy > 0);

	m_bDontScaleLocked = bDontScale;

	m_sizeButtonLocked = sizeButton;
	m_sizeImageLocked = sizeImage;

	m_sizeCurButtonLocked = sizeButton;
	m_sizeCurImageLocked = sizeImage;

	m_ImagesLocked.SetImageSize (m_sizeImageLocked);
	m_MenuImagesLocked.SetImageSize (m_sizeImageLocked);
	m_ColdImagesLocked.SetImageSize (m_sizeImageLocked);
	m_DisabledImagesLocked.SetImageSize (m_sizeImageLocked);
	m_DisabledMenuImagesLocked.SetImageSize (m_sizeImageLocked);

	CSize sizeImageLarge (
		(int) (.5 + m_dblLargeImageRatio * m_sizeImageLocked.cx), 
		(int) (.5 + m_dblLargeImageRatio * m_sizeImageLocked.cy));

	m_LargeImagesLocked.SetImageSize (sizeImageLarge);
	m_LargeColdImagesLocked.SetImageSize (sizeImageLarge);
	m_LargeDisabledImagesLocked.SetImageSize (sizeImageLarge);

	if (m_bLargeIcons)
	{
		m_sizeCurButtonLocked.cx = (int) (.5 + m_dblLargeImageRatio * m_sizeCurButtonLocked.cx);
		m_sizeCurButtonLocked.cy = (int) (.5 + m_dblLargeImageRatio * m_sizeCurButtonLocked.cy);

		m_sizeCurImageLocked.cx = (int) (.5 + m_dblLargeImageRatio * m_sizeCurImageLocked.cx);
		m_sizeCurImageLocked.cy = (int) (.5 + m_dblLargeImageRatio * m_sizeCurImageLocked.cy);
	}
}
//******************************************************************************************
void CBCGPToolBar::SetHeight(int cyHeight)
{
	ASSERT_VALID (this);

	int nHeight = cyHeight;
	
	if (m_dwStyle & CBRS_BORDER_TOP)
	{
		nHeight -= afxData.cyBorder2;
	}

	if (m_dwStyle & CBRS_BORDER_BOTTOM)
	{
		nHeight -= afxData.cyBorder2;
	}

	m_cyBottomBorder = (nHeight - GetRowHeight ()) / 2;
	
	//-------------------------------------------------------
	// If there is an extra pixel, m_cyTopBorder will get it:
	//-------------------------------------------------------
	m_cyTopBorder = nHeight - GetRowHeight () - m_cyBottomBorder;
	
	if (m_cyTopBorder < 0)
	{
		TRACE(_T("Warning: CBCGPToolBar::SetHeight(%d) is smaller than button.\n"), cyHeight);
		m_cyBottomBorder += m_cyTopBorder;
		m_cyTopBorder = 0;  // will clip at bottom
	}

	if (GetSafeHwnd () != NULL)
	{
		RedrawWindow (NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
	}
}
//******************************************************************************************
BOOL CBCGPToolBar::SetUserImages (CBCGPToolBarImages* pUserImages, BOOL bAdjustSize)
{
	ASSERT (pUserImages != NULL);
	if (!pUserImages->IsValid ())
	{
		ASSERT (FALSE);
		return FALSE;
	}

	if (m_sizeImage != pUserImages->GetImageSize ())
	{
		if (!bAdjustSize)
		{
			ASSERT (FALSE);
			return FALSE;
		}

		pUserImages->SetImageSize(m_sizeImage);
	}

	m_pUserImages = pUserImages;
	return TRUE;
}
//******************************************************************************************
BOOL CBCGPToolBar::SetButtons(const UINT* lpIDArray, int nIDCount, BOOL bRemapImages)
{
	ASSERT_VALID(this);
	ASSERT(nIDCount >= 1);  // must be at least one of them
	ASSERT(lpIDArray == NULL ||
		AfxIsValidAddress(lpIDArray, sizeof(UINT) * nIDCount, FALSE));

	//-----------------------
	// Save customize button:
	//-----------------------
	CCustomizeButton* pCustomizeBtn = NULL;
	if (m_pCustomizeBtn != NULL)
	{
		ASSERT_VALID (m_pCustomizeBtn);
		ASSERT (m_pCustomizeBtn == m_Buttons.GetTail ());	// Should be last

		CRuntimeClass* pRTC = m_pCustomizeBtn->GetRuntimeClass ();
		pCustomizeBtn = DYNAMIC_DOWNCAST (CCustomizeButton, 
			pRTC->CreateObject ());

		ASSERT_VALID (pCustomizeBtn);
		pCustomizeBtn->CopyFrom (*m_pCustomizeBtn);
	}

	RemoveAllButtons ();

	while (!m_OrigButtons.IsEmpty ())
	{
		delete m_OrigButtons.RemoveHead ();
	}

	if (lpIDArray == NULL)
	{
		while (nIDCount-- > 0)
		{
			InsertSeparator ();
		}
		
		return TRUE;
	}

	int iImage = m_iImagesOffset;

	//--------------------------------
	// Go through them adding buttons:
	//--------------------------------
	for (int i = 0; i < nIDCount; i ++)
	{
		int iCmd = *lpIDArray ++;

		m_OrigButtons.AddTail (new CBCGPToolbarButton (iCmd, -1));

		if (iCmd == 0)	// Separator
		{
			InsertSeparator ();
		}
		else if (bRemapImages)
		{
			if (InsertButton (CBCGPToolbarButton (iCmd, iImage, NULL, FALSE, 
				m_bLocked)) >= 0 && !m_bLocked)
			{
				m_DefaultImages.SetAt (iCmd, iImage);
			}

			iImage ++;
		}
		else
		{	
			if (m_DefaultImages.Lookup (iCmd, iImage)) 
			{
				InsertButton (CBCGPToolbarButton (iCmd, iImage, NULL, FALSE, m_bLocked));
			}
		}
	}

	//--------------------------
	// Restore customize button:
	//--------------------------
	if (pCustomizeBtn != NULL)
	{
		InsertButton (pCustomizeBtn);
		m_pCustomizeBtn = pCustomizeBtn;
	}

	if (GetSafeHwnd () != NULL)
	{
		//------------------------------------
		// Allow to produce some user actions:
		//------------------------------------

		OnReset ();

		CWnd* pParentFrame = (m_pDockSite == NULL) ?
			GetParent () : m_pDockSite;
		if (pParentFrame != NULL)
		{
			pParentFrame->SendMessage (BCGM_RESETTOOLBAR, (WPARAM) m_uiOriginalResID);

			while (!m_OrigResetButtons.IsEmpty ())
			{
				delete m_OrigResetButtons.RemoveHead ();
			}

			//-------------------------------------------
			//	Store Buttons state after OnToolbarReset
			//-------------------------------------------
			int i = 0;
			for (POSITION pos = m_Buttons.GetHeadPosition (); pos != NULL; i++)
			{
				CBCGPToolbarButton* pButton = (CBCGPToolbarButton*) m_Buttons.GetNext (pos);

				if(pButton != NULL && pButton->IsKindOf(RUNTIME_CLASS(CBCGPToolbarButton)))
				{
					CRuntimeClass* pRTC = pButton->GetRuntimeClass();
					CBCGPToolbarButton* pBtn = (CBCGPToolbarButton*)pRTC->CreateObject();
					pBtn->CopyFrom(*pButton);
					m_OrigResetButtons.AddTail(pBtn); 

				}
			}
		}
	}

	return TRUE;
}
//******************************************************************************************
BOOL CBCGPToolBar::LoadBitmap (UINT uiResID, UINT uiColdResID, UINT uiMenuResID, 
							  BOOL bLocked, 
							  UINT uiDisabledResID, UINT uiMenuDisabledResID)
{
	CBCGPToolBarParams params;

	params.m_uiColdResID		= uiColdResID;
	params.m_uiHotResID			= uiResID;
	params.m_uiDisabledResID	= uiDisabledResID;
	params.m_uiMenuResID		= uiMenuResID;
	params.m_uiMenuDisabledResID= uiMenuDisabledResID;

	return LoadBitmapEx (params, bLocked);
}
//******************************************************************************************
BOOL CBCGPToolBar::LoadToolBar(UINT uiResID, UINT uiColdResID, UINT uiMenuResID, 
							  BOOL bLocked,
							  UINT uiDisabledResID, UINT uiMenuDisabledResID,
							  UINT uiHotResID)
{
	CBCGPToolBarParams params;

	params.m_uiColdResID		= uiColdResID;
	params.m_uiHotResID			= uiHotResID;
	params.m_uiDisabledResID	= uiDisabledResID;
	params.m_uiMenuResID		= uiMenuResID;
	params.m_uiMenuDisabledResID= uiMenuDisabledResID;

	return LoadToolBarEx (uiResID, params, bLocked);
}
//*****************************************************************************************
BOOL CBCGPToolBar::LoadBitmapEx (CBCGPToolBarParams& params, BOOL bLocked)
{
	m_bLocked = bLocked;

	if (m_bLocked)
	{
		//------------------------------------------
		// Don't add bitmap to the shared resources!
		//------------------------------------------
		if (!m_ImagesLocked.Load (params.m_uiHotResID, AfxGetResourceHandle (), TRUE))
		{
			return FALSE;
		}

		if (params.m_uiColdResID != 0)
		{
			if (!m_ColdImagesLocked.Load (params.m_uiColdResID, AfxGetResourceHandle (), TRUE))
			{
				return FALSE;
			}

			ASSERT (m_ImagesLocked.GetCount () == m_ColdImagesLocked.GetCount ());
		}
		else if (m_bAutoGrayInactiveImages)
		{
			m_ImagesLocked.CopyTo (m_ColdImagesLocked);
			m_ColdImagesLocked.GrayImages (m_nGrayImagePercentage);
		}
		
		if (params.m_uiDisabledResID != 0)
		{
			if (!m_DisabledImagesLocked.Load (params.m_uiDisabledResID, AfxGetResourceHandle (), TRUE))
			{
				return FALSE;
			}
			
			ASSERT (m_ImagesLocked.GetCount () == m_DisabledImagesLocked.GetCount ());
		}

		//------------------
		// Load large images:
		//-------------------
		if (params.m_uiLargeHotResID != 0)
		{	
			if (!m_LargeImagesLocked.Load (params.m_uiLargeHotResID, AfxGetResourceHandle (), TRUE))
			{
				return FALSE;
			}

			ASSERT (m_ImagesLocked.GetCount () == m_LargeImagesLocked.GetCount ());
		}

		if (params.m_uiLargeColdResID != 0)
		{
			ASSERT (params.m_uiColdResID != 0);

			if (!m_LargeColdImagesLocked.Load (params.m_uiLargeColdResID, AfxGetResourceHandle (), TRUE))
			{
				return FALSE;
			}

			ASSERT (m_ImagesLocked.GetCount () == m_LargeColdImagesLocked.GetCount ());
		}
		
		if (params.m_uiLargeDisabledResID != 0)
		{
			ASSERT (params.m_uiDisabledResID != 0);

			if (!m_LargeDisabledImagesLocked.Load (params.m_uiLargeDisabledResID, AfxGetResourceHandle (), TRUE))
			{
				return FALSE;
			}
			
			ASSERT (m_ImagesLocked.GetCount () == m_LargeDisabledImagesLocked.GetCount ());
		}
	
		if (params.m_uiMenuResID != 0)
		{
			if (!m_MenuImagesLocked.Load (params.m_uiMenuResID, AfxGetResourceHandle (), TRUE))
			{
				return FALSE;
			}

			ASSERT (m_ImagesLocked.GetCount () == m_MenuImagesLocked.GetCount ());
		}

		if (params.m_uiMenuDisabledResID != 0)
		{
			if (!m_MenuImagesLocked.Load (params.m_uiMenuResID, AfxGetResourceHandle (), TRUE))
			{
				return FALSE;
			}

			ASSERT (m_ImagesLocked.GetCount () == m_MenuImagesLocked.GetCount ());
		}
		
		return TRUE;
	}

	if (!m_Images.Load (params.m_uiHotResID, AfxGetResourceHandle (), TRUE))
	{
		return FALSE;
	}

	m_iImagesOffset = m_Images.GetResourceOffset (params.m_uiHotResID);
	ASSERT (m_iImagesOffset >= 0);

	if (params.m_uiColdResID != 0)
	{
		if (!m_ColdImages.Load (params.m_uiColdResID, AfxGetResourceHandle (), TRUE))
		{
			return FALSE;
		}

		ASSERT (m_Images.GetCount () == m_ColdImages.GetCount ());
		ASSERT (m_Images.GetImageSize ().cy == m_ColdImages.GetImageSize ().cy);
	}
	else if (m_bAutoGrayInactiveImages)
	{
		m_Images.CopyTo (m_ColdImages);
		m_ColdImages.GrayImages (m_nGrayImagePercentage);
	}

	if (params.m_uiMenuResID != 0)
	{
		if (!m_MenuImages.Load (params.m_uiMenuResID, AfxGetResourceHandle (), TRUE))
		{
			return FALSE;
		}

		ASSERT (m_Images.GetCount () == m_MenuImages.GetCount ());
		ASSERT (m_MenuImages.GetImageSize ().cy == m_sizeMenuImage.cy);
	}

	if (params.m_uiDisabledResID != 0)
	{
		if (!m_DisabledImages.Load (params.m_uiDisabledResID, AfxGetResourceHandle (), TRUE))
		{
			return FALSE;
		}
		
		ASSERT (m_Images.GetCount () == m_DisabledImages.GetCount ());
	}
	
	if (params.m_uiMenuDisabledResID != 0)
	{
		if (!m_DisabledMenuImages.Load (params.m_uiMenuDisabledResID, AfxGetResourceHandle (), TRUE))
		{
			return FALSE;
		}
		
		ASSERT (m_Images.GetCount () == m_DisabledMenuImages.GetCount ());
	}

	//------------------
	// Load large images:
	//-------------------
	if (params.m_uiLargeHotResID != 0)
	{	
		if (!m_LargeImages.Load (params.m_uiLargeHotResID, AfxGetResourceHandle (), TRUE))
		{
			return FALSE;
		}

		ASSERT (m_Images.GetCount () == m_LargeImages.GetCount ());
	}

	if (params.m_uiLargeColdResID != 0)
	{
		ASSERT (params.m_uiColdResID != 0);

		if (!m_LargeColdImages.Load (params.m_uiLargeColdResID, AfxGetResourceHandle (), TRUE))
		{
			return FALSE;
		}

		ASSERT (m_Images.GetCount () == m_LargeColdImages.GetCount ());
	}
	
	if (params.m_uiLargeDisabledResID != 0)
	{
		ASSERT (params.m_uiDisabledResID != 0);

		if (!m_LargeDisabledImages.Load (params.m_uiLargeDisabledResID, AfxGetResourceHandle (), TRUE))
		{
			return FALSE;
		}
		
		ASSERT (m_Images.GetCount () == m_LargeDisabledImages.GetCount ());
	}
	
	return TRUE;
}
//******************************************************************************************
BOOL CBCGPToolBar::LoadToolBarEx(UINT uiToolbarResID, CBCGPToolBarParams& params, 
								BOOL bLocked)
{
	struct CToolBarData
	{
		WORD wVersion;
		WORD wWidth;
		WORD wHeight;
		WORD wItemCount;

		WORD* items()
			{ return (WORD*)(this+1); }
	};

	ASSERT_VALID(this);

	LPCTSTR lpszResourceName = MAKEINTRESOURCE (uiToolbarResID);
	ASSERT(lpszResourceName != NULL);

	//---------------------------------------------------
	// determine location of the bitmap in resource fork:
	//---------------------------------------------------
	HINSTANCE hInst = AfxFindResourceHandle(lpszResourceName, RT_TOOLBAR);
	HRSRC hRsrc = ::FindResource(hInst, lpszResourceName, RT_TOOLBAR);
	if (hRsrc == NULL)
		return FALSE;

	HGLOBAL hGlobal = LoadResource(hInst, hRsrc);
	if (hGlobal == NULL)
		return FALSE;

	CToolBarData* pData = (CToolBarData*)LockResource(hGlobal);
	if (pData == NULL)
		return FALSE;
	ASSERT(pData->wVersion == 1);

	UINT* pItems = new UINT[pData->wItemCount];
	ASSERT (pItems != NULL);

	CSize sizeImage (pData->wWidth, pData->wHeight);
	CSize sizeButton (pData->wWidth + 6, pData->wHeight + 6);

	BOOL bDontScaleImages = bLocked ? m_bDontScaleLocked : m_bDontScaleImages;

	if (!bDontScaleImages && globalData.GetRibbonImageScale () != 1.)
	{
		double dblImageScale = globalData.GetRibbonImageScale ();

		sizeButton = CSize ((int)(.5 + sizeButton.cx * dblImageScale), (int)(.5 + sizeButton.cy * dblImageScale));
	}

	if (bLocked)
	{
		SetLockedSizes (sizeButton, sizeImage);
	}
	else if (!m_Images.IsScaled ())
	{
		SetSizes (sizeButton, sizeImage);
	}

	BOOL bResult = TRUE;

	if (params.m_uiHotResID == 0)	// Use toolbar resource as hot image
	{
		params.m_uiHotResID = uiToolbarResID;
	}

	if (m_uiOriginalResID != 0 || // Buttons are already created
		LoadBitmapEx (params, bLocked))

	{
		int iImageIndex = m_iImagesOffset;
		for (int i = 0; i < pData->wItemCount; i++)
		{
			pItems[i] = pData->items()[i];

			if (!bLocked && pItems [i] > 0)
			{
				m_DefaultImages.SetAt (pItems[i], iImageIndex ++);
			}
		}

		m_uiOriginalResID = uiToolbarResID;
		bResult = SetButtons(pItems, pData->wItemCount);

		if (!bResult)
		{
			m_uiOriginalResID = 0;
		}
	}

	delete[] pItems;

	UnlockResource(hGlobal);
	FreeResource(hGlobal);

	return bResult;
}
//*****************************************************************************************
int CBCGPToolBar::InsertButton (const CBCGPToolbarButton& button, INT_PTR iInsertAt)
{
	CRuntimeClass* pClass = button.GetRuntimeClass ();
	ASSERT (pClass != NULL);

	CBCGPToolbarButton* pButton = (CBCGPToolbarButton*) pClass->CreateObject ();
	ASSERT_VALID(pButton);

	pButton->CopyFrom (button);

	INT_PTR iIndex = InsertButton (pButton, (int) iInsertAt);
	if (iIndex < 0)
	{
		delete pButton;
	}
	
	return (int) iIndex;
}
//******************************************************************************************
int CBCGPToolBar::ReplaceButton (UINT uiCmd, const CBCGPToolbarButton& button,
								 BOOL bAll/* = FALSE*/)
{
	ASSERT_VALID (this);

	int nButtonsCount = 0;

	for (int iStartIndex = 0;;)
	{
		int iIndex = CommandToIndex (uiCmd, iStartIndex);
		if (iIndex < 0)
		{
			break;
		}

		POSITION pos = m_Buttons.FindIndex (iIndex);
		if (pos == NULL)
		{
			ASSERT (FALSE);
			break;
		}

		CBCGPToolbarButton* pOldButton = (CBCGPToolbarButton*) m_Buttons.GetAt (pos);
		ASSERT_VALID (pOldButton);

		m_Buttons.RemoveAt (pos);
		pOldButton->OnCancelMode ();

		delete pOldButton;

		if (InsertButton (button, iIndex) < 0)
		{
			ASSERT (FALSE);
			return FALSE;
		}

		nButtonsCount++;

		if (bAll)
		{
			iStartIndex = iIndex + 1;
		}
		else
		{
			break;
		}
	}

	if (nButtonsCount == 0)
	{
		TRACE(_T("ReplaceButton: Can't find command %d\n"), uiCmd);
	}

	return nButtonsCount;
}
//******************************************************************************************
int CBCGPToolBar::InsertButton (CBCGPToolbarButton* pButton, int iInsertAt)
{
	ASSERT (pButton != NULL);

	if (!IsCommandPermitted (pButton->m_nID))
	{
		return -1;
	}

	if (iInsertAt != -1 &&
		(iInsertAt < 0 || iInsertAt > m_Buttons.GetCount ()))
	{
		return -1;
	}

	if (iInsertAt == -1 || iInsertAt == m_Buttons.GetCount ())
	{
		if (m_pCustomizeBtn != NULL)
		{
			ASSERT_VALID (m_pCustomizeBtn);
			ASSERT (m_pCustomizeBtn == m_Buttons.GetTail ());	// Should be last!

			iInsertAt = (int) m_Buttons.GetCount () - 1;
		}
		else
		{
			//-------------------------
			// Add to the toolbar tail:
			//-------------------------
			m_Buttons.AddTail (pButton);
			pButton->OnChangeParentWnd (this);

			return (int) m_Buttons.GetCount () - 1;
		}
	}

	POSITION pos = m_Buttons.FindIndex (iInsertAt);
	ASSERT (pos != NULL);

	m_Buttons.InsertBefore (pos, pButton);
	pButton->OnChangeParentWnd (this);

	return iInsertAt;
}
//******************************************************************************************
int CBCGPToolBar::InsertSeparator (INT_PTR iInsertAt)
{
	// Don't allow add a separtor first:
	if (m_Buttons.IsEmpty () || iInsertAt == 0)
	{
		return -1;
	}

	CBCGPToolbarButton* pButton = new CBCGPToolbarButton;
	ASSERT (pButton != NULL);

	pButton->m_nStyle = TBBS_SEPARATOR;

	int iNewButtonIndex = (int) InsertButton (pButton, (int) iInsertAt);
	if (iNewButtonIndex == -1)
	{
		delete pButton;
	}

	return iNewButtonIndex;
}
//******************************************************************************************
void CBCGPToolBar::RemoveAllButtons ()
{
	m_iButtonCapture = -1;      // nothing captured
	m_iHighlighted = -1;
	m_iSelected = -1;

	while (!m_Buttons.IsEmpty ())
	{
		CBCGPToolbarButton* pButton = (CBCGPToolbarButton*) m_Buttons.RemoveHead ();
		ASSERT_VALID (pButton);

		if (pButton != NULL)
		{
			pButton->OnCancelMode ();
			delete pButton;
		}
	}

	m_pCustomizeBtn = NULL;
}
//******************************************************************************************
BOOL CBCGPToolBar::RemoveButton (int iIndex)
{
	POSITION pos = m_Buttons.FindIndex (iIndex);
	if (pos == NULL)
	{
		return FALSE;
	}

	if (iIndex == m_Buttons.GetCount () - 1 && m_pCustomizeBtn != NULL)
	{
		//-------------------------------------
		// Unable to remove "Customize" button:
		//-------------------------------------
		ASSERT_VALID (m_pCustomizeBtn);
		ASSERT (m_pCustomizeBtn == m_Buttons.GetTail ());	// Should be last!
		ASSERT (FALSE);

		return FALSE;
	}

	CBCGPToolbarButton* pButton = (CBCGPToolbarButton*) m_Buttons.GetAt (pos);
	ASSERT_VALID (pButton);

	m_Buttons.RemoveAt (pos);
	pButton->OnCancelMode ();

	delete pButton;

	if (iIndex == m_iSelected)
	{
		m_iSelected = -1;
	}
	else if (iIndex < m_iSelected && m_iSelected >= 0)
	{
		m_iSelected --;
	}

	if (iIndex == m_iButtonCapture)
	{
		m_iButtonCapture = -1;
	}
	else if (iIndex < m_iButtonCapture && m_iButtonCapture >= 0)
	{
		m_iButtonCapture --;
	}

	if (iIndex == m_iHighlighted)
	{
		m_iHighlighted = -1;
		OnChangeHot (m_iHighlighted);
	}
	else if (iIndex < m_iHighlighted && m_iHighlighted >= 0)
	{
		m_iHighlighted --;
		OnChangeHot (m_iHighlighted);
	}

	//-----------------------------------------
	// If last button is separator - remove it:
	//-----------------------------------------
	pos = m_Buttons.GetTailPosition();
	if (pos != NULL && m_pCustomizeBtn == m_Buttons.GetTail ())
	{
		m_Buttons.GetPrev (pos);
	}
	while (pos != NULL)
	{
		POSITION posSave = pos;
		CBCGPToolbarButton* pLastButton = (CBCGPToolbarButton*) m_Buttons.GetPrev(pos);
		if (pos != NULL)
		{
			if (pLastButton->m_nStyle & TBBS_SEPARATOR)
			{
				m_Buttons.RemoveAt (posSave);

				delete pLastButton;

			}
			else
			{
				//----------------------
				// Regular button, stop!
				//----------------------
				break;
			}
		}
	}

	//----------------------------
	// Don't leave two separators:
	//----------------------------
	if (iIndex > 0 && iIndex < m_Buttons.GetCount ())
	{
		CBCGPToolbarButton* pPrevButton = GetButton (iIndex - 1);
		ASSERT_VALID (pPrevButton);

		CBCGPToolbarButton* pNextButton = GetButton (iIndex);
		ASSERT_VALID (pNextButton);

		if ((pPrevButton->m_nStyle & TBBS_SEPARATOR) &&
			(pNextButton->m_nStyle & TBBS_SEPARATOR))
		{
			RemoveButton (iIndex);
		}
	}

	RebuildAccelerationKeys ();

	return TRUE;
}

#ifdef AFX_CORE3_SEG
#pragma code_seg(AFX_CORE3_SEG)
#endif

/////////////////////////////////////////////////////////////////////////////
// CBCGPToolBar attribute access

int CBCGPToolBar::CommandToIndex (UINT nIDFind, int iIndexFirst/* = 0*/) const
{
	ASSERT_VALID(this);

	int i = 0;
	for (POSITION pos = m_Buttons.GetHeadPosition (); pos != NULL; i ++)
	{
		CBCGPToolbarButton* pButton = (CBCGPToolbarButton*) m_Buttons.GetNext (pos);
		ASSERT (pButton != NULL);

		if (i >= iIndexFirst && pButton->m_nID == nIDFind)
		{
			return i;
		}
	}

	return -1;
}
//*****************************************************************
UINT CBCGPToolBar::GetItemID(int nIndex) const
{
	ASSERT_VALID(this);

	CBCGPToolbarButton* pButton = GetButton (nIndex);
	if (pButton == NULL)
	{
		ASSERT (FALSE);
		return 0;
	}

	return pButton->m_nID;
}
//*****************************************************************
void CBCGPToolBar::GetItemRect(int nIndex, LPRECT lpRect) const
{
	ASSERT_VALID(this);

	ASSERT(nIndex >= 0 && nIndex < m_Buttons.GetCount ());
	ASSERT(AfxIsValidAddress(lpRect, sizeof(RECT)));

	CBCGPToolbarButton* pButton = GetButton (nIndex);
	if (pButton == NULL)
	{
		ASSERT (FALSE);
		*lpRect = CRect (0, 0, 0, 0);
	}
	else
	{
		*lpRect = pButton->Rect ();
	}
}
//*****************************************************************
void CBCGPToolBar::GetInvalidateItemRect(int nIndex, LPRECT lpRect) const
{
	ASSERT_VALID(this);
	
	ASSERT(nIndex >= 0 && nIndex < m_Buttons.GetCount ());
	ASSERT(AfxIsValidAddress(lpRect, sizeof(RECT)));
	
	CBCGPToolbarButton* pButton = GetButton (nIndex);
	if (pButton == NULL)
	{
		ASSERT (FALSE);
		*lpRect = CRect (0, 0, 0, 0);
	}
	else
	{
		*lpRect = pButton->GetInvalidateRect ();
	}
}
//***************************************************************************
UINT CBCGPToolBar::GetButtonStyle(int nIndex) const
{
	CBCGPToolbarButton* pButton = GetButton (nIndex);
	if (pButton == NULL)
	{
		ASSERT (FALSE);
		return 0;
	}

	return pButton->m_nStyle;
}
//*****************************************************************
int CBCGPToolBar::ButtonToIndex (const CBCGPToolbarButton* pButton) const
{
	ASSERT_VALID (this);
	ASSERT_VALID (pButton);

	int i = 0;
	for (POSITION pos = m_Buttons.GetHeadPosition (); pos != NULL; i ++)
	{
		CBCGPToolbarButton* pListButton = (CBCGPToolbarButton*) m_Buttons.GetNext (pos);
		ASSERT (pListButton != NULL);

		if (pListButton == pButton)
		{
			return i;
		}
	}

	return -1;
}
//*****************************************************************
void CBCGPToolBar::SetButtonStyle(int nIndex, UINT nStyle)
{
	CBCGPToolbarButton* pButton = GetButton (nIndex);
	if (pButton == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	UINT nOldStyle = pButton->m_nStyle;
	if (nOldStyle != nStyle)
	{
		if (nStyle & TBBS_DISABLED)
		{
			// Disabled button shouldn't be pressed
			nStyle &= ~TBBS_PRESSED;
		}

		// update the style and invalidate
		pButton->SetStyle (nStyle);

		// invalidate the button only if both styles not "pressed"
		if (!(nOldStyle & nStyle & TBBS_PRESSED))
		{
			InvalidateButton(nIndex);
		}
	}
}
//****************************************************************
CSize CBCGPToolBar::CalcFixedLayout(BOOL bStretch, BOOL bHorz)
{
	DWORD dwMode = bStretch ? LM_STRETCH : 0;
	dwMode |= bHorz ? LM_HORZ : 0;

	return CalcLayout (dwMode);
}
//*************************************************************************************
void CBCGPToolBar::GetButtonInfo(int nIndex, UINT& nID, UINT& nStyle, int& iImage) const
{
	ASSERT_VALID(this);

	CBCGPToolbarButton* pButton = GetButton(nIndex);
	if (pButton == NULL)
	{
		ASSERT (FALSE);

		nID = 0;
		nStyle = 0;
		iImage = -1;

		return;
	}

	nID = pButton->m_nID;
	nStyle = pButton->m_nStyle;
	iImage = pButton->GetImage ();
}
//*************************************************************************************
void CBCGPToolBar::SetButtonInfo(int nIndex, UINT nID, UINT nStyle, int iImage)
{
	ASSERT_VALID(this);

	CBCGPToolbarButton* pButton = GetButton(nIndex);
	if (pButton == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	ASSERT_VALID (pButton);

	pButton->m_nStyle = nStyle;
	pButton->m_nID = nID;
	pButton->SetImage (iImage);

	if ((nStyle & TBBS_SEPARATOR) && iImage > 0) // iImage parameter is a button width!
	{
		AdjustLayout ();
	}

	InvalidateButton(nIndex);
}
//*************************************************************************************
BOOL CBCGPToolBar::SetButtonText(int nIndex, LPCTSTR lpszText)
{
	ASSERT_VALID(this);
	ASSERT(lpszText != NULL);

	CBCGPToolbarButton* pButton = GetButton(nIndex);
	if (pButton == NULL)
	{
		return FALSE;
	}

	pButton->m_strText = lpszText;
	return TRUE;
}
//*************************************************************************************
CString CBCGPToolBar::GetButtonText( int nIndex ) const
{
	ASSERT_VALID(this);

	CBCGPToolbarButton* pButton = GetButton(nIndex);
	if (pButton == NULL)
	{
		ASSERT (FALSE);
		return _T("");
	}

	ASSERT_VALID (pButton);

	return pButton->m_strText;
}
//*************************************************************************************
void CBCGPToolBar::GetButtonText( int nIndex, CString& rString ) const
{
	ASSERT_VALID(this);

	CBCGPToolbarButton* pButton = GetButton(nIndex);
	if (pButton == NULL)
	{
		ASSERT (FALSE);
		rString.Empty ();
		return;
	}

	ASSERT_VALID (pButton);

	rString = pButton->m_strText;
}
//*************************************************************************************
void CBCGPToolBar::DoPaint(CDC* pDCPaint)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDCPaint);

	CBCGPDrawOnGlass dog (m_bOnGlass);

	CRect rectClip;
	pDCPaint->GetClipBox (rectClip);

	BOOL bHorz = GetCurrentAlignment () & CBRS_ORIENT_HORZ ? TRUE : FALSE;

	CRect rectClient;
	GetClientRect (rectClient);

	CBCGPMemDC memDC (*pDCPaint, this);
	CDC* pDC = &memDC.GetDC ();

	if ((GetStyle () & TBSTYLE_TRANSPARENT) == 0)
	{
		CBCGPVisualManager::GetInstance ()->OnFillBarBackground (pDC, this,
			rectClient, rectClip);
	}
	else
	{
		m_Impl.GetBackgroundFromParent (pDC);
	}

	OnFillBackground (pDC);

	pDC->SetTextColor (globalData.clrBtnText);
	pDC->SetBkMode (TRANSPARENT);

	CRect rect;
	GetClientRect(rect);

	//-----------------------------------
	// Force the full size of the button:
	//-----------------------------------
	if (bHorz)
	{
		rect.bottom = rect.top + GetRowHeight ();
	}
	else
	{
		rect.right = rect.left + GetColumnWidth ();
	}

	BOOL bDontScaleImages = m_bLocked ? m_bDontScaleLocked : m_bDontScaleImages;
	const double dblImageScale = bDontScaleImages ? 1.0 : globalData.GetRibbonImageScale ();
		
	CBCGPToolBarImages* pImages = 
		GetImageList (m_Images, m_ImagesLocked, m_LargeImages, m_LargeImagesLocked);
	CBCGPToolBarImages* pHotImages = pImages;
	CBCGPToolBarImages* pColdImages = 
		GetImageList (m_ColdImages, m_ColdImagesLocked, m_LargeColdImages, m_LargeColdImagesLocked);
	CBCGPToolBarImages* pDisabledImages = 
		GetImageList (m_DisabledImages, m_DisabledImagesLocked, m_LargeDisabledImages, m_LargeDisabledImagesLocked);
	CBCGPToolBarImages* pMenuImages = !m_bLocked ? 
		&m_MenuImages : &m_MenuImagesLocked;

	CBCGPToolBarImages* pDisabledMenuImages = !m_bLocked ?
		&m_DisabledMenuImages : &m_DisabledMenuImagesLocked;

	BOOL bDrawImages = pImages->IsValid ();

	pHotImages->SetTransparentColor (globalData.clrBtnFace);

	BOOL bFadeInactiveImages = CBCGPVisualManager::GetInstance ()->IsFadeInactiveImage ();

	CSize sizeImageDest = m_bMenuMode ? m_sizeMenuImage : GetImageSize ();
	if (dblImageScale != 1.)
	{
		if (m_bMenuMode && sizeImageDest == CSize (-1, -1))
		{
			sizeImageDest = GetImageSize ();

			if (dblImageScale > 1. && m_bLargeIconsAreEnbaled)
			{
				sizeImageDest = m_sizeImage;
			}
		}

		sizeImageDest = CSize ((int)(.5 + sizeImageDest.cx * dblImageScale), (int)(.5 + sizeImageDest.cy * dblImageScale));
	}

	BOOL bImageWasPrepared = bDrawImages;

	CBCGPDrawState ds(CBCGPVisualManager::GetInstance()->IsAutoGrayscaleImages());
	if (bDrawImages)
	{
		if (dblImageScale != 1.0 && pHotImages->GetScale () == 1.0)
		{
			pHotImages->SmoothResize (dblImageScale);
		}

		if (!pHotImages->PrepareDrawImage (ds, sizeImageDest, bFadeInactiveImages))
		{
			return;     // something went wrong
		}
	}

	CFont* pOldFont;
	if (bHorz)
	{
		pOldFont = SelectDefaultFont (pDC);
	}
	else
	{
		pOldFont = (CFont*) pDC->SelectObject (&globalData.fontVert);
	}

	if (pColdImages->GetCount () > 0)
	{
		//------------------------------------------
		// Disable fade effect for inactive buttons:
		//------------------------------------------
		CBCGPVisualManager::GetInstance ()->SetFadeInactiveImage (FALSE);
	}

	//--------------
	// Draw buttons:
	//--------------
	int iButton = 0;
	for (POSITION pos = m_Buttons.GetHeadPosition (); pos != NULL; iButton ++)
	{
		CBCGPToolbarButton* pButton = (CBCGPToolbarButton*) m_Buttons.GetNext (pos);
		if (pButton == NULL)
		{
			break;
		}

		ASSERT_VALID (pButton);

		rect = pButton->Rect ();
		CRect rectInter;

		if (pButton->m_nStyle & TBBS_SEPARATOR)
		{
			BOOL bHorzSeparator = bHorz;
			CRect rectSeparator; rectSeparator.SetRectEmpty ();

			OnCalcSeparatorRect (pButton, rectSeparator, bHorz);

			if (pButton->m_bWrap && bHorz)
			{
				bHorzSeparator = FALSE;
			}

			if (rectInter.IntersectRect (rectSeparator, rectClip) && !pButton->IsHidden())
			{
				DrawSeparator (pDC, rectSeparator, bHorzSeparator);
			}

			continue;
		}

		if (!rectInter.IntersectRect (rect, rectClip))
		{
			continue;
		}

		BOOL bHighlighted = IsButtonHighlighted (iButton);
		BOOL bDisabled = (pButton->m_nStyle & TBBS_DISABLED) && !IsCustomizeMode ();

		if (pDC->RectVisible(&rect))
		{
			BOOL bDrawDisabledImages = FALSE;

			if (bDrawImages)
			{
				CBCGPToolBarImages* pNewImages = NULL;

				if (pButton->m_bUserButton)
				{
					if (pButton->GetImage () >= 0)
					{
						pNewImages = m_pUserImages;
					}
				}
				else
				{
					if (m_bMenuMode)
					{
						if (bDisabled && pDisabledMenuImages->GetCount () > 0)
						{
							bDrawDisabledImages = TRUE;
							pNewImages = pDisabledMenuImages;
						}
						else if (pMenuImages->GetCount () > 0)
						{
							pNewImages = pMenuImages;
						}
						else
						{
							bDrawDisabledImages = 
								(bDisabled && pDisabledImages->GetCount () > 0);

							pNewImages =  bDrawDisabledImages ? 
											pDisabledImages : pHotImages;
						}
					}
					else	// Toolbar mode
					{
						bDrawDisabledImages = 
							(bDisabled && pDisabledImages->GetCount () > 0);

						pNewImages =  bDrawDisabledImages ? 
										pDisabledImages : pHotImages;

						if (!bHighlighted && !bDrawDisabledImages &&
							(pButton->m_nStyle & TBBS_PRESSED) == 0 &&
							pColdImages->GetCount () > 0 &&
							!pButton->IsDroppedDown ())
						{
							pNewImages = pColdImages;
						}
					}
				}

				CheckForButtonImages(pButton, &pNewImages);

				if (bDrawImages && pNewImages != pImages && pNewImages != NULL)
				{
					if (bImageWasPrepared)
					{
						pImages->EndDrawImage (ds);
					}
					
					pNewImages->SetTransparentColor (globalData.clrBtnFace);

					if (dblImageScale != 1.0 && pNewImages->GetScale () == 1.0)
					{
						pNewImages->SmoothResize (dblImageScale);
					}

					bImageWasPrepared = pNewImages->PrepareDrawImage (ds, sizeImageDest, bFadeInactiveImages);

					pImages = pNewImages;
				}
			}

			DrawButton (pDC, pButton, bDrawImages ? pImages : NULL, 
						bHighlighted, bDrawDisabledImages);
		}
	}

	//-------------------------------------------------------------
	// Highlight selected button in the toolbar customization mode:
	//-------------------------------------------------------------
	if (m_iSelected >= m_Buttons.GetCount ())
	{
		m_iSelected = -1;
	}

	if ((IsCustomizeMode () || m_bAltCustomizeMode) && m_iSelected >= 0 && !m_bLocked && 
		m_pSelToolbar == this)
	{
		CBCGPToolbarButton* pSelButton = GetButton (m_iSelected);
		ASSERT (pSelButton != NULL);

		if (pSelButton != NULL && pSelButton->CanBeStored ())
		{
			CRect rectDrag1 = pSelButton->Rect ();

			pDC->Draw3dRect(&rectDrag1, globalData.clrBarText, globalData.clrBarText);
			rectDrag1.DeflateRect (1, 1);
			pDC->Draw3dRect(&rectDrag1, globalData.clrBarText, globalData.clrBarText);
		}
	}

	if (IsCustomizeMode () && m_iDragIndex >= 0 && !m_bLocked)
	{
		DrawDragMarker (pDC);
	}

	pDC->SelectObject (pOldFont);

	if (bDrawImages && bImageWasPrepared)
	{
		pImages->EndDrawImage (ds);
	}

	CBCGPVisualManager::GetInstance ()->SetFadeInactiveImage (bFadeInactiveImages);
}
//*************************************************************************************
BOOL CBCGPToolBar::IsButtonHighlighted (int iButton) const
{
	BOOL bHighlighted = FALSE;

	if (IsCustomizeMode () && !m_bLocked)
	{
		bHighlighted = FALSE;
	}
	else
	{
		if (m_bMenuMode)
		{
			bHighlighted = (iButton == m_iHighlighted);
		}
		else
		{
			bHighlighted = ((iButton == m_iHighlighted ||
							iButton == m_iButtonCapture) &&
							(m_iButtonCapture == -1 ||
							iButton == m_iButtonCapture));
		}
	}

	return bHighlighted;
}
//*************************************************************************************
BOOL CBCGPToolBar::DrawButton(CDC* pDC, CBCGPToolbarButton* pButton,
							CBCGPToolBarImages* pImages,
							BOOL bHighlighted, BOOL bDrawDisabledImages)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pButton);

	if (!pButton->IsVisible() || pButton->IsHidden () || !pDC->RectVisible (pButton->Rect ()))
	{
		return TRUE;
	}

	BOOL bHorz = GetCurrentAlignment () & CBRS_ORIENT_HORZ ? TRUE : FALSE;

	//---------------------
	// Draw button context:
	//---------------------
	pButton->OnDraw (pDC, pButton->Rect (), pImages, bHorz, 
		IsCustomizeMode () && !m_bAltCustomizeMode && !m_bLocked, 
		bHighlighted, m_bShowHotBorder,
		m_bGrayDisabledButtons && !bDrawDisabledImages);
	return TRUE;
}
//*************************************************************************************
CBCGPToolbarButton* CBCGPToolBar::GetButton(int nIndex) const
{
	POSITION pos = m_Buttons.FindIndex (nIndex);
	if (pos == NULL)
	{
		return NULL;
	}

	CBCGPToolbarButton* pButton = (CBCGPToolbarButton*) m_Buttons.GetAt (pos);
	ASSERT (pButton != NULL);

	return pButton;
}
//*************************************************************************************
CBCGPToolbarButton* CBCGPToolBar::InvalidateButton (int nIndex)
{
	ASSERT_VALID(this);

	if (nIndex < 0 || nIndex >= m_Buttons.GetCount ())
	{
		return NULL;
	}

	CRect rect;
	GetInvalidateItemRect (nIndex, &rect);

	CBCGPToolbarButton* pButton = GetButton (nIndex);
	if (pButton != NULL && pButton == m_pCustomizeBtn)
	{
		rect.right += 10;
		rect.bottom += 10;
	}

	InvalidateRect (rect);

	if (pButton != NULL && pButton == m_pCustomizeBtn &&
		m_pCustomizeBtn->GetExtraSize () != CSize (0, 0))
	{
		rect.InflateRect (m_pCustomizeBtn->GetExtraSize ());
		RedrawWindow (rect, NULL, RDW_FRAME | RDW_INVALIDATE);
	}

	return pButton;
}
//*************************************************************************************
INT_PTR CBCGPToolBar::OnToolHitTest(CPoint point, TOOLINFO* pTI) const
{
	ASSERT_VALID(this);

	if (!m_bShowTooltips)
	{
		return -1;
	}

	// check child windows first by calling CBCGPBaseToolBar
	INT_PTR nHit = (INT_PTR) CBCGPBaseToolBar::OnToolHitTest(point, pTI);
	if (nHit != -1)
		return nHit;

	// now hit test against CBCGPToolBar buttons
	nHit = ((CBCGPToolBar*)this)->HitTest(point);
	if (nHit != -1)
	{
		CBCGPToolbarButton* pButton = GetButton((int)nHit);
		if (pButton == NULL)
		{
			return -1;
		}

		if (pButton->IsDroppedDown ())
		{
			return -1;
		}

		if (pTI != NULL)
		{
			CString strTipText;
			if (!OnUserToolTip (pButton, strTipText))
			{
				if ((pButton->m_nID == 0 || pButton->m_nID == (UINT) -1 ||
					pButton->m_bUserButton) &&
					!pButton->m_strText.IsEmpty ())
				{
					// Use button text as tooltip!
					strTipText = pButton->m_strText;

					strTipText.Remove (_T('&'));
				}
				else
				{
					if (g_pUserToolsManager != NULL &&
						g_pUserToolsManager->IsUserToolCmd (pButton->m_nID))
					{
						strTipText = pButton->m_strText;
					}
					else
					{
						TCHAR szFullText [256];

						AfxLoadString (pButton->m_nID, szFullText);
						AfxExtractSubString(strTipText, szFullText, 1, '\n');

						strTipText.Remove (_T('&'));
					}
				}
			}

			if (strTipText.IsEmpty ())
			{
				return -1;
			}

			if (pButton->m_nID != 0 && pButton->m_nID != (UINT) -1 && 
				m_bShowShortcutKeys)
			{
				//--------------------
				// Add shortcut label:
				//--------------------
				CString strLabel;
				if (pButton->GetKeyboardAccelerator(strLabel))
				{
					strTipText += _T(" (");
					strTipText += strLabel;
					strTipText += _T(')');
				}
			}

			CString strDescr;
			
			CFrameWnd* pParent = GetParentFrame ();
			if (pParent->GetSafeHwnd () != NULL && pButton->m_nID != 0)
			{
				pParent->GetMessageString (pButton->m_nID, strDescr);
			}

			CBCGPTooltipManager::SetTooltipText (pTI,
				m_pToolTip, BCGP_TOOLTIP_TYPE_TOOLBAR, strTipText, strDescr);

			GetItemRect((int)nHit, &pTI->rect);
			pTI->uId = (pButton->m_nID == (UINT) -1) ? 0 : pButton->m_nID;
			pTI->hwnd = m_hWnd;
		}

		nHit = (pButton->m_nID == (UINT) -1) ? 0 : pButton->m_nID;
	}

	return nHit;
}
//*************************************************************************************
int CBCGPToolBar::HitTest(CPoint point) // in window relative coords
{
	int iButton = 0;
	for (POSITION pos = m_Buttons.GetHeadPosition (); pos != NULL; iButton ++)
	{
		CBCGPToolbarButton* pButton = (CBCGPToolbarButton*) m_Buttons.GetNext (pos);
		ASSERT (pButton != NULL);

		if (pButton->Rect ().PtInRect (point) && !pButton->IsHidden ())
		{
			return (pButton->m_nStyle & TBBS_SEPARATOR) ? -1 : iButton;
		}
	}

	return -1;      // nothing hit
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPToolBar message handlers

BEGIN_MESSAGE_MAP(CBCGPToolBar, CBCGPBaseToolBar)
	ON_WM_CONTEXTMENU()
	//{{AFX_MSG_MAP(CBCGPToolBar)
	ON_WM_MOUSEMOVE()
	ON_WM_CANCELMODE()
	ON_WM_SYSCOLORCHANGE()
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_LBUTTONDOWN()
	ON_COMMAND(ID_BCGBARRES_TOOLBAR_APPEARANCE, OnToolbarAppearance)
	ON_COMMAND(ID_BCGBARRES_TOOLBAR_DELETE, OnToolbarDelete)
	ON_COMMAND(ID_BCGBARRES_TOOLBAR_IMAGE, OnToolbarImage)
	ON_COMMAND(ID_BCGBARRES_TOOLBAR_IMAGE_AND_TEXT, OnToolbarImageAndText)
	ON_COMMAND(ID_BCGBARRES_TOOLBAR_START_GROUP, OnToolbarStartGroup)
	ON_COMMAND(ID_BCGBARRES_TOOLBAR_TEXT, OnToolbarText)
	ON_WM_LBUTTONUP()
	ON_WM_WINDOWPOSCHANGED()
	ON_WM_CTLCOLOR()
	ON_WM_SETCURSOR()
	ON_WM_SIZE()
	ON_WM_LBUTTONDBLCLK()
	ON_COMMAND(ID_BCGBARRES_TOOLBAR_RESET, OnBcgbarresToolbarReset)
	ON_WM_NCCALCSIZE()
	ON_WM_NCPAINT()
	ON_COMMAND(ID_BCGBARRES_COPY_IMAGE, OnBcgbarresCopyImage)
	ON_WM_SETFOCUS()
	ON_COMMAND(ID_BCGBARRES_TOOLBAR_NEW_MENU, OnBcgbarresToolbarNewMenu)
	ON_WM_WINDOWPOSCHANGING()
	ON_WM_ERASEBKGND()
	ON_WM_KILLFOCUS()
	ON_WM_SETTINGCHANGE()
	ON_WM_SHOWWINDOW()
	ON_WM_NCHITTEST()
	ON_WM_RBUTTONDOWN()
	//}}AFX_MSG_MAP
	ON_REGISTERED_MESSAGE(BCGM_RESETRPROMPT, OnPromptReset)
	ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
	ON_MESSAGE(WM_HELPHITTEST, OnHelpHitTest)
	ON_MESSAGE(TB_BUTTONCOUNT, OnGetButtonCount)
	ON_MESSAGE(TB_GETITEMRECT, OnGetItemRect)
	ON_MESSAGE(TB_GETBUTTON, OnGetButton)
	ON_MESSAGE(TB_GETBUTTONTEXT, OnGetButtonText)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXT, 0, 0xFFFF, OnNeedTipText)
	ON_REGISTERED_MESSAGE(BCGM_UPDATETOOLTIPS, OnBCGUpdateToolTips)
	ON_REGISTERED_MESSAGE(BCGM_ONSETCONTROLAERO, OnBCGSetControlAero)
END_MESSAGE_MAP()

void CBCGPToolBar::OnLButtonDown(UINT nFlags, CPoint point)
{
	int iButton = HitTest(point);

	if (m_pSelToolbar != this && IsCustomizeMode ())
	{
		CBCGPToolBar* pSelToolbar = m_pSelToolbar;
		m_pSelToolbar = this;

		if (pSelToolbar != NULL)
		{
			ASSERT_VALID (pSelToolbar);

			int iOldSelected = pSelToolbar->m_iSelected;
			pSelToolbar->m_iSelected = -1;
			pSelToolbar->InvalidateButton (iOldSelected);
		}
	}

	if (!IsCustomizeMode () &&
		DYNAMIC_DOWNCAST (CBCGPControlBar, GetParent ()) != NULL)
	{
		GetParent ()->SetFocus ();
	}

	if (iButton < 0) // nothing hit
	{
		m_iButtonCapture = -1;

		if (IsCustomizeMode () && !m_bLocked)
		{
			int iSelected = m_iSelected;
			m_iSelected = -1;

			if (iSelected != -1)
			{
				InvalidateButton (iSelected);
				UpdateWindow ();
			}

			OnChangeHot (-1);
		}

		if (CanFloat ())
		{
			SetCursor (globalData.m_hcurSizeAll);
		}

		CBCGPBaseToolBar::OnLButtonDown(nFlags, point);
		return;
	}

	CBCGPToolbarButton* pButton = GetButton(iButton);
	if (pButton == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	ASSERT(!(pButton->m_nStyle & TBBS_SEPARATOR));

	//-----------------------------------------------------------------
	// Check for "Alt-customizible mode" (when ALT key is holded down):
	//-----------------------------------------------------------------
	m_bAltCustomizeMode = FALSE;
	if (m_bAltCustomization &&
		AllowAltCustomization () &&
		!m_bCustomizeMode &&
		GetAsyncKeyState (VK_MENU) & 0x8000)	// ALT is pressed
	{
		m_bAltCustomizeMode = TRUE;
		m_iSelected = iButton;
		m_iHighlighted = -1;
		m_pSelToolbar = this;
	}

	if ((!IsCustomizeMode () && !m_bAltCustomizeMode) || m_bLocked || m_bDisableCustomize)
	{
		m_iButtonCapture = iButton;

		// update the button before checking for disabled status
		UpdateButton(m_iButtonCapture);
		if ((pButton->m_nStyle & TBBS_DISABLED) &&
			!pButton->IsKindOf(RUNTIME_CLASS(CBCGPDropDownToolbarButton)))
		{
			m_iButtonCapture = -1;
			return;     // don't press it
		}

		pButton->m_nStyle |= TBBS_PRESSED;

		InvalidateButton (iButton);
		UpdateWindow(); // immediate feedback

		ShowCommandMessageString (pButton->m_nID);

		BOOL bWasDroppedDown = pButton->IsDroppedDown();

		if (pButton->OnClick (this, FALSE /* No delay*/))
		{
			BOOL bIsRemoved = FALSE;

			if (m_Buttons.Find (pButton) != NULL)
			{
				pButton->m_nStyle &= ~TBBS_PRESSED;
			}
			else
			{
				bIsRemoved = TRUE;
			}

			m_iButtonCapture = -1;
			m_iHighlighted = -1;

			OnChangeHot (m_iHighlighted);

			InvalidateButton (iButton);
			UpdateWindow(); // immediate feedback

			if (!bIsRemoved && bWasDroppedDown != pButton->IsDroppedDown())
			{
				int nChildID = AccGetChildIdByButtonIndex(iButton);
				if (nChildID > 0)
				{
					globalData.NotifyWinEvent (EVENT_OBJECT_STATECHANGE, GetSafeHwnd (), OBJID_CLIENT , nChildID);
					globalData.NotifyWinEvent (EVENT_OBJECT_FOCUS, GetSafeHwnd (), OBJID_CLIENT , nChildID);
				}
			}
		}
		else
		{
			m_pWndLastCapture = SetCapture ();
		}
	}
	else
	{
		int iSelected = m_iSelected;
		m_iSelected = iButton;

		CRect rect;
		GetItemRect (iButton, &rect);

		if (iSelected != -1)
		{
			InvalidateButton (iSelected);
		}

		m_pDragButton = GetButton (m_iSelected);
		ASSERT (m_pDragButton != NULL);

		m_bIsDragCopy = (nFlags & MK_CONTROL);

		if (!m_pDragButton->IsEditable ())
		{
			m_iSelected = -1;
			m_pDragButton = NULL;

			if (iSelected != -1)
			{
				InvalidateButton (iSelected);
			}
			return;
		}

		InvalidateButton (iButton);
		UpdateWindow(); // immediate feedback

		if (m_pDragButton->CanBeStretched () &&
			abs (point.x - rect.right) <= STRETCH_DELTA &&
			!m_bAltCustomizeMode)
		{
			m_bStretchButton = TRUE;

			m_rectTrack = m_pDragButton->Rect ();

			if (m_pDragButton->GetHwnd () != NULL)
			{
				m_rectTrack.InflateRect (2, 2);
			}

			m_pWndLastCapture = SetCapture ();
			::SetCursor (globalData.m_hcurStretch);
		}
		else if (m_pDragButton->CanBeStored () &&
			m_pDragButton->OnBeforeDrag ())
		{
			COleDataSource* pSrcItem = new COleDataSource();

			m_pDragButton->PrepareDrag(*pSrcItem);

			ShowCommandMessageString (pButton->m_nID);

			m_DropSource.m_bDragStarted = FALSE;
			m_ptStartDrag = point;

			HWND hwndSaved = m_hWnd;

			if (m_bAltCustomizeMode)
			{
				m_bCustomizeMode = TRUE;
			}

			DROPEFFECT dropEffect = pSrcItem->DoDragDrop(DROPEFFECT_COPY|DROPEFFECT_MOVE, &rect, &m_DropSource);

			if (!::IsWindow (hwndSaved))
			{
				if (m_bAltCustomizeMode)
				{
					m_bCustomizeMode = FALSE;
					m_bAltCustomizeMode = FALSE;
					m_pSelToolbar = NULL;
				}

				return;
			}

			CPoint ptDrop;
			::GetCursorPos (&ptDrop);
			ScreenToClient (&ptDrop);

			if (m_DropSource.m_bDragStarted &&
				!rect.PtInRect (ptDrop))
			{
				if (dropEffect != DROPEFFECT_COPY && 
					m_pDragButton != NULL &&
					!m_DropSource.m_bEscapePressed &&
					OnBeforeRemoveButton (m_pDragButton, dropEffect))
				{
					//---------------------
					// Remove source button:
					//---------------------
					RemoveButton (ButtonToIndex (m_pDragButton));
					AdjustLocations ();
					RedrawWindow (NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
					AdjustSizeImmediate ();

					if (GetParent ()->GetSafeHwnd () != NULL)
					{
						GetParent ()->RedrawWindow (NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
					}

					if (!m_bAltCustomizeMode)
					{
						AdjustLayout ();
						RedrawWindow (NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
					}
					
					if (GetParent ()->IsKindOf (RUNTIME_CLASS (CBCGPTabWnd)))
					{
						CBCGPToolBar* pParentTabbedBar = 
							DYNAMIC_DOWNCAST (CBCGPToolBar, GetParent ()->GetParent ());
						if (pParentTabbedBar != NULL)
						{
							pParentTabbedBar->RecalcLayout ();
						}
					}
				}
				else if (m_pDragButton != NULL)
				{
					InvalidateRect (m_pDragButton->Rect ());
				}
			}
			else
			{
				m_iHighlighted = iButton;
				OnChangeHot (m_iHighlighted);
			}

			m_pDragButton = NULL;
			m_ptStartDrag = CPoint (-1, -1);

			pSrcItem->InternalRelease();
		}
		else
		{
			m_pDragButton = NULL;
		}
	}

	if (m_bAltCustomizeMode)
	{
		m_bAltCustomizeMode = FALSE;
		m_pSelToolbar = NULL;
		SetCustomizeMode (FALSE);

		RedrawWindow (NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
	}
}
//**************************************************************************************
void CBCGPToolBar::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_bDragMode)
	{
		CBCGPBaseToolBar::OnMouseMove (nFlags, point);
		return;
	}

	if (IsCustomizeMode () && !m_bLocked)
	{
		if (m_bStretchButton)
		{
			ASSERT_VALID (m_pDragButton);

			if (point.x - m_pDragButton->Rect ().left >= BUTTON_MIN_WIDTH)
			{
				CClientDC dc (this);

				CRect rectTrackOld = m_rectTrack;
				m_rectTrack.right = point.x;
				dc.DrawDragRect (&m_rectTrack, CSize (2, 2), &rectTrackOld, CSize (2, 2));
			}

			::SetCursor (globalData.m_hcurStretch);
		}

		return;
	}

	if (m_ptLastMouse != CPoint (-1, -1) &&
		abs (m_ptLastMouse.x - point.x) < 1 &&
		abs (m_ptLastMouse.y - point.y) < 1)
	{
		m_ptLastMouse = point;
		return;
	}

	m_ptLastMouse = point;

	int iPrevHighlighted = m_iHighlighted;
	m_iHighlighted = HitTest (point);

	int nTracked = m_iHighlighted;

	if (m_bLeaveFocus && m_iHighlighted == -1 && GetFocus () == this)
	{
		m_iHighlighted = iPrevHighlighted;
		return;
	}

	CBCGPToolbarMenuButton* pCurrPopupMenu = GetDroppedDownMenu ();
	if (pCurrPopupMenu != NULL && pCurrPopupMenu->IsExclusive ())
	{
		m_iHighlighted = iPrevHighlighted;
		return;
	}

	CBCGPToolbarButton* pButton = m_iHighlighted == -1 ?
							NULL : GetButton (m_iHighlighted);
	if (pButton != NULL &&
		(pButton->m_nStyle & TBBS_SEPARATOR || 
		(pButton->m_nStyle & TBBS_DISABLED && 
		!AllowSelectDisabled ())))
	{
		m_iHighlighted = -1;
	}


	if (m_bMenuMode && m_iHighlighted == -1)
	{
		if (globalData.IsAccessibilitySupport ())
		{
			int nIndex = HitTest (point);
			if (nIndex != -1)
			{
				if (nIndex != m_iAccHotItem)
				{
					m_iAccHotItem = nIndex;
					SetTimer (uiAccNotifyEvent, uiAccTimerDelay, NULL);
				}
			}
		}
	}

	if (!m_bTracked)
	{
		m_bTracked = TRUE;
		
		TRACKMOUSEEVENT trackmouseevent;
		trackmouseevent.cbSize = sizeof(trackmouseevent);
		trackmouseevent.dwFlags = TME_LEAVE;
		trackmouseevent.hwndTrack = GetSafeHwnd();
		trackmouseevent.dwHoverTime = HOVER_DEFAULT;
		::BCGPTrackMouse (&trackmouseevent);	
	}
	
	if (iPrevHighlighted != nTracked)
	{
		if (nTracked != -1 && 
			(m_bMenuMode || nTracked == m_iButtonCapture || m_iButtonCapture == -1))
		{
			if (pButton != NULL)
			{
				ShowCommandMessageString (pButton->m_nID);
			}
		}
		else if ((m_iButtonCapture == -1 || (m_bMenuMode && nTracked == -1))
			&& m_hookMouseHelp == NULL)
		{
			GetOwner()->SendMessage (WM_SETMESSAGESTRING, AFX_IDS_IDLEMESSAGE);
		}
	}

	if (iPrevHighlighted != m_iHighlighted)
	{
		BOOL bNeedUpdate = FALSE;

		if (m_iButtonCapture != -1)
		{
			CBCGPToolbarButton* pTBBCapt = GetButton (m_iButtonCapture);
			ASSERT (pTBBCapt != NULL);
			ASSERT (!(pTBBCapt->m_nStyle & TBBS_SEPARATOR));

			UINT nNewStyle = (pTBBCapt->m_nStyle & ~TBBS_PRESSED);
			if (m_iHighlighted == m_iButtonCapture)
			{
				nNewStyle |= TBBS_PRESSED;
			}

			if (nNewStyle != pTBBCapt->m_nStyle)
			{
				SetButtonStyle (m_iButtonCapture, nNewStyle);
				bNeedUpdate = TRUE;
			}
		}

		if ((m_bMenuMode || m_iButtonCapture == -1 || 
			iPrevHighlighted == m_iButtonCapture) &&

			iPrevHighlighted != -1)
		{
			InvalidateButton (iPrevHighlighted);
			bNeedUpdate = TRUE;
		}

		if ((m_bMenuMode || m_iButtonCapture == -1 || 
			m_iHighlighted == m_iButtonCapture) &&
			m_iHighlighted != -1) 
		{
			InvalidateButton (m_iHighlighted);
			bNeedUpdate = TRUE;

			if (globalData.IsAccessibilitySupport () && m_bMenuMode)
			{
				BOOL bDropDown = FALSE;
				CBCGPToolbarMenuButton* pMenuButton = 
					DYNAMIC_DOWNCAST (CBCGPToolbarMenuButton, GetButton (m_iHighlighted));
				if (pMenuButton != NULL && pMenuButton->m_bDrawDownArrow)
				{
					bDropDown = TRUE;		
				}

				int nIndex = HitTest (point);
				if (nIndex != m_iAccHotItem)
				{
					m_iAccHotItem = nIndex;
					
					KillTimer (uiAccNotifyEvent);

					if (bDropDown)
					{
						SetTimer (uiAccNotifyEvent, uiAccPopupTimerDelay, NULL);
					}
					else
					{
						SetTimer (uiAccNotifyEvent, uiAccTimerDelay, NULL);
					}
				}	
			}
		}

		OnChangeHot (m_iHighlighted);
		if (bNeedUpdate)
		{
			UpdateWindow ();
		}
	}

	CBCGPBaseToolBar::OnMouseMove (nFlags, point);
}
//*************************************************************************************
void CBCGPToolBar::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (IsCustomizeMode () && !m_bLocked)
	{
		if (m_bStretchButton)
		{
			ASSERT_VALID (m_pDragButton);

			CRect rect = m_pDragButton->Rect ();
			rect.right = point.x;

			if (rect.Width () >= BUTTON_MIN_WIDTH &&
				abs (m_pDragButton->Rect ().right - point.x) > STRETCH_DELTA)
			{
				m_pDragButton->OnSize (rect.Width ());
				AdjustLayout ();
			}

			m_rectTrack.SetRectEmpty ();

			m_pDragButton = NULL;
			m_bStretchButton = FALSE;

			RedrawWindow (NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);

			::ReleaseCapture ();

			if (m_pWndLastCapture != NULL)
			{
				m_pWndLastCapture->SetCapture ();
				m_pWndLastCapture = NULL;
			}

			AdjustSizeImmediate ();
		}
		CBCGPBaseToolBar::OnLButtonUp(nFlags, point);
		return;
	}

	if (m_bDragMode)
	{
		CBCGPBaseToolBar::OnLButtonUp(nFlags, point);
		return;
	}

	if (m_iButtonCapture == -1)
	{
		if (HitTest (point) == -1)
		{
			CBCGPBaseToolBar::OnLButtonUp(nFlags, point);

			m_ptLastMouse = CPoint (-1, -1);
			OnMouseMove (0, point);
		}

		return;     // not captured
	}

	::ReleaseCapture();
	if (m_pWndLastCapture != NULL)
	{
		m_pWndLastCapture->SetCapture ();
		m_pWndLastCapture = NULL;
	}

	m_iHighlighted = HitTest (point);

	CBCGPToolbarButton* pButton = GetButton(m_iButtonCapture);
	if (pButton == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	const BOOL bIsSystemMenuButton = pButton->IsKindOf (
		RUNTIME_CLASS (CBCGPToolbarMenuButtonsButton));

	ASSERT(!(pButton->m_nStyle & TBBS_SEPARATOR));
	UINT nIDCmd = 0;

	UINT nNewStyle = (pButton->m_nStyle & ~TBBS_PRESSED);
	if (m_iButtonCapture == m_iHighlighted)
	{
		// we did not lose the capture
		if (HitTest(point) == m_iButtonCapture)
		{
			// give button a chance to update
			UpdateButton(m_iButtonCapture);

			// then check for disabled state
			if (!(pButton->m_nStyle & TBBS_DISABLED))
			{
				// pressed, will send command notification
				nIDCmd = pButton->m_nID;

				if (pButton->m_nStyle & TBBS_CHECKBOX)
				{
					// auto check: three state => down
					if (nNewStyle & TBBS_INDETERMINATE)
						nNewStyle &= ~TBBS_INDETERMINATE;

					nNewStyle ^= TBBS_CHECKED;
				}
			}
		}
	}

	if (m_hookMouseHelp == NULL)
	{
		GetOwner()->SendMessage(WM_SETMESSAGESTRING, AFX_IDS_IDLEMESSAGE);
	}

	int iButtonCapture = m_iButtonCapture;
	m_iButtonCapture = -1;
	m_iHighlighted = -1;

	HWND hwndSaved = m_hWnd;
	RestoreFocus ();

	if (HitTest(point) == iButtonCapture && 
		!OnSendCommand (pButton) &&
		nIDCmd != 0 && nIDCmd != (UINT) -1)
	{
		InvalidateButton (iButtonCapture);
		UpdateWindow(); // immediate feedback

		AddCommandUsage (nIDCmd);

		if (!pButton->OnClickUp () && 
			(g_pUserToolsManager == NULL ||
			!g_pUserToolsManager->InvokeTool (nIDCmd)))
		{
			GetOwner()->SendMessage (WM_COMMAND, nIDCmd);    // send command
		}
	}
	else if (::IsWindow (hwndSaved) && iButtonCapture < m_Buttons.GetCount () && GetButton(iButtonCapture) == pButton)
	{
		pButton->OnClickUpOutside();
	}

	if (::IsWindow (hwndSaved) &&				// "This" may be destoyed now!
		iButtonCapture < m_Buttons.GetCount ())	// Button may disappear now!
	{
		if (bIsSystemMenuButton)
		{
			CBCGPToolbarButton* pButton = GetButton (iButtonCapture);
			if (pButton != NULL)
			{
				pButton->m_nStyle &= ~TBBS_PRESSED;
			}
		}
		else
		{
			SetButtonStyle (iButtonCapture, nNewStyle);
		}

		UpdateButton(iButtonCapture);
		InvalidateButton (iButtonCapture);
		UpdateWindow(); // immediate feedback

		m_ptLastMouse = CPoint (-1, -1);
		OnMouseMove (0, point);
	}
}
//*************************************************************************************
void CBCGPToolBar::OnCancelMode()
{
	CBCGPBaseToolBar::OnCancelMode();

	if (m_bStretchButton)
	{
		m_pDragButton = NULL;
		m_bStretchButton = FALSE;

		m_rectTrack.SetRectEmpty ();

		::ReleaseCapture ();
		if (m_pWndLastCapture != NULL)
		{
			m_pWndLastCapture->SetCapture ();
			m_pWndLastCapture = NULL;
		}
	}

	if (m_iButtonCapture >= 0 ||
		m_iHighlighted >= 0)
	{
		if (m_iButtonCapture >= 0)
		{
			CBCGPToolbarButton* pButton = GetButton(m_iButtonCapture);
			if (pButton == NULL)
			{
				ASSERT (FALSE);
			}
			else
			{
				ASSERT(!(pButton->m_nStyle & TBBS_SEPARATOR));
				UINT nNewStyle = (pButton->m_nStyle & ~TBBS_PRESSED);
				if (GetCapture() == this)
				{
					::ReleaseCapture();

					if (m_pWndLastCapture != NULL)
					{
						m_pWndLastCapture->SetCapture ();
						m_pWndLastCapture = NULL;
					}
				}

				SetButtonStyle(m_iButtonCapture, nNewStyle);
			}
		}

		m_iButtonCapture = -1;
		m_iHighlighted = -1;

		OnChangeHot (m_iHighlighted);
	}

	for (POSITION pos = m_Buttons.GetHeadPosition (); pos != NULL;)
	{
		CBCGPToolbarButton* pButton = (CBCGPToolbarButton*) m_Buttons.GetNext (pos);
		ASSERT (pButton != NULL);
	
		pButton->OnCancelMode ();
	}

	RedrawWindow (NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
}
//*************************************************************************************
void CBCGPToolBar::OnSysColorChange()
{
	globalData.UpdateSysColors ();

	CBCGPVisualManager::GetInstance ()->OnUpdateSystemColors ();

	UpdateImagesColor ();

	RedrawWindow (NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
}
//*************************************************************************************
void CBCGPToolBar::UpdateImagesColor ()
{
	m_Images.OnSysColorChange ();
	m_ColdImages.OnSysColorChange ();
	m_DisabledImages.OnSysColorChange ();
	m_ImagesLocked.OnSysColorChange ();
	m_ColdImagesLocked.OnSysColorChange ();
	m_MenuImages.OnSysColorChange ();
	m_DisabledMenuImages.OnSysColorChange ();
	m_MenuImagesLocked.OnSysColorChange ();
	m_DisabledImagesLocked.OnSysColorChange ();
	m_DisabledMenuImagesLocked.OnSysColorChange ();

	m_LargeImages.OnSysColorChange ();
	m_LargeColdImages.OnSysColorChange ();
	m_LargeDisabledImages.OnSysColorChange ();
	m_LargeImagesLocked.OnSysColorChange ();
	m_LargeColdImagesLocked.OnSysColorChange ();
	m_LargeDisabledImagesLocked.OnSysColorChange ();
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPToolBar idle update through CToolCmdUI class

#define CToolCmdUI COldToolCmdUI

class CToolCmdUI : public CCmdUI        // class private to this file !
{
public: // re-implementations only
	virtual void Enable(BOOL bOn);
	virtual void SetCheck(int nCheck);
	virtual void SetText(LPCTSTR lpszText);
	virtual void SetRadio(BOOL bOn = TRUE);
};

void CToolCmdUI::Enable(BOOL bOn)
{
	m_bEnableChanged = TRUE;
	CBCGPToolBar* pToolBar = (CBCGPToolBar*)m_pOther;
	ASSERT(pToolBar != NULL);
	ASSERT_KINDOF(CBCGPToolBar, pToolBar);
	ASSERT(m_nIndex < m_nIndexMax);

	UINT nNewStyle = pToolBar->GetButtonStyle(m_nIndex) & ~TBBS_DISABLED;

	if (!bOn)
		nNewStyle |= TBBS_DISABLED;
	ASSERT(!(nNewStyle & TBBS_SEPARATOR));
	pToolBar->SetButtonStyle(m_nIndex, nNewStyle);
}
//*************************************************************************************
void CToolCmdUI::SetCheck(int nCheck)
{
	ASSERT (nCheck >= 0);
	if (nCheck > 2)
	{
		nCheck = 1;
	}

	CBCGPToolBar* pToolBar = (CBCGPToolBar*)m_pOther;
	ASSERT(pToolBar != NULL);
	ASSERT_KINDOF(CBCGPToolBar, pToolBar);
	ASSERT(m_nIndex < m_nIndexMax);

	UINT nNewStyle = pToolBar->GetButtonStyle(m_nIndex) &
				~(TBBS_CHECKED | TBBS_INDETERMINATE);
	if (nCheck == 1)
		nNewStyle |= TBBS_CHECKED;
	else if (nCheck == 2)
		nNewStyle |= TBBS_INDETERMINATE;
	ASSERT(!(nNewStyle & TBBS_SEPARATOR));
	pToolBar->SetButtonStyle(m_nIndex, nNewStyle | TBBS_CHECKBOX);
}
//*************************************************************************************
void CToolCmdUI::SetRadio(BOOL bOn)
{
	SetCheck(bOn ? 1 : 0); // this default works for most things as well

	CBCGPToolBar* pToolBar = (CBCGPToolBar*)m_pOther;
	ASSERT(pToolBar != NULL);
	ASSERT_KINDOF(CBCGPToolBar, pToolBar);
	ASSERT(m_nIndex < m_nIndexMax);

	CBCGPToolbarButton* pButton = pToolBar->GetButton (m_nIndex);
	ASSERT_VALID (pButton);

	pButton->SetRadio ();
}
//*************************************************************************************
void CToolCmdUI::SetText (LPCTSTR lpszText)
{
	ASSERT (lpszText != NULL);

	CBCGPToolBar* pToolBar = (CBCGPToolBar*)m_pOther;
	ASSERT(pToolBar != NULL);
	ASSERT_KINDOF(CBCGPToolBar, pToolBar);
	ASSERT(m_nIndex < m_nIndexMax);

	if (pToolBar->GetIgnoreSetText ())	
	{
		return;
	}

	CBCGPToolbarButton* pButton = pToolBar->GetButton (m_nIndex);
	if (pButton == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	ASSERT_VALID (pButton);

	//Remove any amperstands and trailing label (ex.:"\tCtrl+S")
	CString strNewText(lpszText);

	int iOffset = strNewText.Find (_T('\t'));
	if (iOffset != -1)
	{
		strNewText = strNewText.Left (iOffset);
	}

	CString strOldText = pButton->m_strText.SpanExcluding(_T("\t"));
	if (strOldText == strNewText)		
	{
		return;
	}

	pButton->m_strText = strNewText;	
	pToolBar->AdjustLayout ();
}
//*************************************************************************************
void CBCGPToolBar::OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler)
{
	CToolCmdUI state;
	state.m_pOther = this;

	state.m_nIndexMax = (UINT)m_Buttons.GetCount ();
	for (state.m_nIndex = 0; state.m_nIndex < state.m_nIndexMax;
	  state.m_nIndex++)
	{
		CBCGPToolbarButton* pButton = GetButton(state.m_nIndex);
		if (pButton == NULL)
		{
			return;
		}

		if (g_pUserToolsManager != NULL &&
			g_pUserToolsManager->IsUserToolCmd (pButton->m_nID))
		{
			bDisableIfNoHndler = FALSE;
		}

		state.m_nID = pButton->m_nID;

		// ignore separators and system commands
		if (!(pButton->m_nStyle & TBBS_SEPARATOR) &&
			pButton->m_nID != 0 &&
			!IsSystemCommand (pButton->m_nID) &&
			pButton->m_nID < AFX_IDM_FIRST_MDICHILD)
		{
			state.DoUpdate(pTarget, bDisableIfNoHndler);
		}
	}

	// update the dialog controls added to the toolbar
	UpdateDialogControls(pTarget, 
		bDisableIfNoHndler && m_bDisableControlsIfNoHandler);
}
//*************************************************************************************
void CBCGPToolBar::UpdateButton(int nIndex)
{
	CWnd* pTarget = GetCommandTarget ();

	// send the update notification
	if (pTarget != NULL)
	{
		CToolCmdUI state;
		state.m_pOther = this;
		state.m_nIndex = nIndex;
		state.m_nIndexMax = (UINT)m_Buttons.GetCount ();
		CBCGPToolbarButton* pButton = GetButton(nIndex);

		if (pButton == NULL)
		{
			ASSERT (FALSE);
			return;
		}

		if (pButton->m_nID != 0 &&
			!IsSystemCommand (pButton->m_nID) &&
			pButton->m_nID < AFX_IDM_FIRST_MDICHILD)
		{
			BOOL bAutoMenuEnable = FALSE;
			if (pTarget->IsFrameWnd ())
			{
				bAutoMenuEnable = ((CFrameWnd*) pTarget)->m_bAutoMenuEnable;
			}

			state.m_nID = pButton->m_nID;
			state.DoUpdate(pTarget, bAutoMenuEnable &&
						(g_pUserToolsManager == NULL ||
						!g_pUserToolsManager->IsUserToolCmd (pButton->m_nID)));
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPToolBar diagnostics

#ifdef _DEBUG
void CBCGPToolBar::AssertValid() const
{
	CBCGPBaseToolBar::AssertValid();
}

void CBCGPToolBar::Dump(CDumpContext& dc) const
{
	CBCGPBaseToolBar::Dump (dc);

	CString strName;

	if (::IsWindow (m_hWnd))
	{
		GetWindowText (strName);
	}

	dc << "\n**** Toolbar ***" << strName;
	dc << "\nButtons: " << m_Buttons.GetCount () << "\n";

	dc.SetDepth (dc.GetDepth () + 1);

	for (POSITION pos = m_Buttons.GetHeadPosition (); pos != NULL;)
	{
		CBCGPToolbarButton* pButton = (CBCGPToolbarButton*) m_Buttons.GetNext (pos);
		ASSERT_VALID (pButton);

		pButton->Dump (dc);
		dc << "\n";
	}

	dc.SetDepth (dc.GetDepth () - 1);
	dc << "\n";
}
#endif

#undef new
#ifdef AFX_INIT_SEG
#pragma code_seg(AFX_INIT_SEG)
#endif

int CBCGPToolBar::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CBCGPBaseToolBar::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (globalData.m_hcurStretch == NULL)
	{
		globalData.m_hcurStretch = AfxGetApp ()->LoadCursor (AFX_IDC_HSPLITBAR);
	}

	if (globalData.m_hcurStretchVert == NULL)
	{
		globalData.m_hcurStretchVert = AfxGetApp ()->LoadCursor (AFX_IDC_VSPLITBAR);
	}

	if (globalData.m_hcurSizeAll == NULL)
	{
		globalData.m_hcurSizeAll = AfxGetApp ()->LoadStandardCursor (IDC_SIZEALL);
	}

	CFrameWnd* pParent = BCGPGetParentFrame (this) == NULL ?
		NULL : BCGCBProGetTopLevelFrame (BCGPGetParentFrame (this));

	if (pParent != NULL)
	{
		CBCGPToolBarImages::EnableRTL (pParent->GetExStyle () & WS_EX_LAYOUTRTL);
	}

	if (!m_bNoDropTarget)
	{
		_AFX_THREAD_STATE* pState = AfxGetThreadState();
		if (pState->m_bNeedTerm)	// AfxOleInit was called
		{
			m_DropTarget.Register (this);
		}
	}

	m_penDrag.CreatePen (PS_SOLID, 1, globalData.clrBarText);

	CBCGPTooltipManager::CreateToolTip (m_pToolTip, this,
		BCGP_TOOLTIP_TYPE_TOOLBAR);

	m_bRoundShape = 
		CBCGPVisualManager::GetInstance ()->IsToolbarRoundShape (this);

	if (m_bRoundShape)
	{
		SetRoundedRgn ();
	}
	else
	{
		SetWindowRgn (NULL, FALSE);
	}

	gAllToolbars.AddTail(this);

	return 0;
}
//****************************************************************************************
DROPEFFECT CBCGPToolBar::OnDragEnter(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point) 
{
	if (!m_bDisableCustomize)
	{
		return DROPEFFECT_NONE;
	}

	m_iDragIndex = -1;
	m_DropSource.m_bDeleteOnDrop = FALSE;

	return OnDragOver(pDataObject, dwKeyState, point);
}
//****************************************************************************************
void CBCGPToolBar::OnDragLeave() 
{
	if (m_bDisableCustomize)
	{
		return;
	}

	m_iDragIndex = -1;
	
	CRect rect = m_rectDrag;
	rect.InflateRect (2, 2);
	InvalidateRect (&rect);

	UpdateWindow ();

	m_rectDrag.SetRectEmpty ();
	m_iDragIndex = -1;

	m_DropSource.m_bDeleteOnDrop = TRUE;
}
//****************************************************************************************
DROPEFFECT CBCGPToolBar::OnDragOver(COleDataObject* pDataObject, DWORD dwKeyState, 
								   CPoint point) 
{
	if (m_bLocked || m_bDisableCustomize)
	{
		return DROPEFFECT_NONE;
	}

	CBCGPToolbarButton* pButton = CBCGPToolbarButton::CreateFromOleData (pDataObject);
	if (pButton == NULL)
	{
		return DROPEFFECT_NONE;
	}

	BOOL bAllowDrop = pButton->CanBeDropped (this);
	delete pButton;

	if (!bAllowDrop)
	{
		return DROPEFFECT_NONE;
	}

	BOOL bCopy = (dwKeyState & MK_CONTROL);

	m_bIsDragCopy = bCopy;

	if (m_pDragButton == NULL)	// Drag from the other toolbar
	{
		//------------------
		// Remove selection:
		//------------------
		int iSelected = m_iSelected;
		m_iSelected = -1;

		if (iSelected != -1)
		{
			InvalidateButton (iSelected);
			UpdateWindow ();
		}
	}

	//---------------------
	// Find the drop place:
	//---------------------
	CRect rect = m_rectDrag;
	int iIndex = FindDropIndex (point, m_rectDrag);

	if (rect != m_rectDrag)
	{
		//--------------------
		// Redraw drop marker:
		//--------------------
		m_iDragIndex = iIndex;

		rect.InflateRect (2, 2);
		InvalidateRect (&rect);

		rect = m_rectDrag;
		rect.InflateRect (2, 2);
		InvalidateRect (&m_rectDrag);

		UpdateWindow ();
	}

	int iPrevHighlighted = m_iHighlighted;
	m_iHighlighted = HitTest (point);

	if (iPrevHighlighted != m_iHighlighted)
	{
		OnChangeHot (m_iHighlighted);
	}

	if (iIndex == -1)
	{
		return DROPEFFECT_NONE;
	}

	return (bCopy) ? DROPEFFECT_COPY : DROPEFFECT_MOVE;
}
//****************************************************************************************
BOOL CBCGPToolBar::OnDrop(COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point) 
{
	ASSERT_VALID(this);

	if (m_bDisableCustomize)
	{
		return FALSE;
	}

	int iDragIndex = m_iDragIndex;
	if (iDragIndex < 0)
	{
		return FALSE;
	}

	CBCGPToolbarButton* pDragButton = m_pDragButton;
	m_pDragButton = NULL;

	OnDragLeave();

	int iHit = HitTest (point);
	if (iHit >= 0 && pDragButton == GetButton (iHit))
	{
		return FALSE;
	}

	//----------------------------------------------------
	// Create a button object from the OLE clipboard data:
	//----------------------------------------------------
	CBCGPToolbarButton* pButton = CreateDroppedButton (pDataObject);
	if (pButton == NULL)
	{
		return FALSE;
	}

	if (!pButton->OnBeforeDrop (this))
	{
		delete pButton;
		return TRUE;
	}

	pButton->m_bDragFromCollection = FALSE;

	if (pDragButton != NULL && dropEffect != DROPEFFECT_COPY)
	{
		int iOldIndex = ButtonToIndex (pDragButton);
		if (iDragIndex == iOldIndex || iDragIndex == iOldIndex + 1)
		{
			AddRemoveSeparator (pDragButton, m_ptStartDrag, point);
			delete pButton;
			return TRUE;
		}
		
		RemoveButton (iOldIndex);
		if (iDragIndex > iOldIndex)
		{
			iDragIndex --;
		}

		iDragIndex = min (iDragIndex, (int) m_Buttons.GetCount ());
	}

	if (InsertButton (pButton, iDragIndex) == -1)
	{
		ASSERT (FALSE);
		delete pButton;
		return FALSE;
	}

	AdjustLayout ();

	if (GetParent ()->IsKindOf (RUNTIME_CLASS (CBCGPTabWnd)))
	{
		CBCGPToolBar* pParentTabbedBar = 
			DYNAMIC_DOWNCAST (CBCGPToolBar, GetParent ()->GetParent ());
		if (pParentTabbedBar != NULL)
		{
			pParentTabbedBar->RecalcLayout ();
		}
	}

	if (m_bAltCustomizeMode)
	{
		//------------------------------
		// Immideatly save button state:
		//------------------------------
		pButton->SaveBarState ();
	}

	m_iSelected = -1;
	RedrawWindow (NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);

	CBCGPPopupMenu* pPopupMenu = DYNAMIC_DOWNCAST (CBCGPPopupMenu, GetParent ());
	if (pPopupMenu != NULL)
	{
		ASSERT_VALID (pPopupMenu);
		pPopupMenu->RedrawWindow (NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
	}

	return TRUE;
}
//****************************************************************************************
BOOL CBCGPToolBar::SetCustomizeMode (BOOL bSet)
{
	if (m_bCustomizeMode == bSet)
	{
		return FALSE;
	}

	//---------------------------------------------------------------------
	// First step - inform all toolbars about start/end customization mode:
	//---------------------------------------------------------------------
	for (BOOL bToolbarsListWasChanged = TRUE;
		bToolbarsListWasChanged;)
	{
		INT_PTR iOrigCount = gAllToolbars.GetCount ();
		bToolbarsListWasChanged = FALSE;

		for (POSITION posTlb = gAllToolbars.GetHeadPosition (); 
			posTlb != NULL && !bToolbarsListWasChanged;)
		{
			CBCGPToolBar* pToolBar = (CBCGPToolBar*) gAllToolbars.GetNext (posTlb);
			ASSERT (pToolBar != NULL);

			if (CWnd::FromHandlePermanent (pToolBar->m_hWnd) != NULL)
			{
				ASSERT_VALID(pToolBar);

				pToolBar->OnCustomizeMode (bSet);

				//-------------------------------------------------
				// CBCGPToolBar::OnCustomizeMode can add/remove some
				// "sub-toolbars". So, let's start loop again!
				//-------------------------------------------------
				if (gAllToolbars.GetCount () != iOrigCount)
				{
					bToolbarsListWasChanged = TRUE;
				}
			}
			}
	}

	m_bCustomizeMode = bSet;

	//----------------------------------------------
	// Second step - adjust layout for all toolbars:
	//----------------------------------------------
	POSITION posTlb = NULL;
	for (posTlb = gAllToolbars.GetHeadPosition (); posTlb != NULL;)
	{
		CBCGPToolBar* pToolBar = (CBCGPToolBar*) gAllToolbars.GetNext (posTlb);
		ASSERT (pToolBar != NULL);

		if (CWnd::FromHandlePermanent (pToolBar->m_hWnd) != NULL && 
			!pToolBar->IsLocked())
		{
			ASSERT_VALID(pToolBar);
			pToolBar->AdjustLayout ();
		}
	}

	//-----------------------------------
	// Third step - redraw all toolbars:
	//-----------------------------------
	BOOL bLayoutRecalculated = FALSE;
	for (posTlb = gAllToolbars.GetHeadPosition (); posTlb != NULL;)
	{
		CBCGPToolBar* pToolBar = (CBCGPToolBar*) gAllToolbars.GetNext (posTlb);
		ASSERT (pToolBar != NULL);

		if (!pToolBar->IsBarVisible ())
		{
			continue;
		}

		CFrameWnd* pWnd = DYNAMIC_DOWNCAST (CFrameWnd, pToolBar->GetDockSite ());

		if (pWnd != NULL && !bLayoutRecalculated)
		{
			pWnd->RecalcLayout ();
			bLayoutRecalculated = TRUE;
		}

		if (CWnd::FromHandlePermanent (pToolBar->m_hWnd) != NULL)
		{
			ASSERT_VALID(pToolBar);
			pToolBar->RedrawWindow (NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_FRAME | RDW_ERASE | RDW_ALLCHILDREN);
		}
	}
	

	if (!bSet)
	{
		m_pSelToolbar = NULL;
	}

	return TRUE;
}
//********************************************************************************
int CBCGPToolBar::GetCommandButtons (UINT uiCmd, CObList& listButtons)
{
	listButtons.RemoveAll ();
	if (uiCmd == 0)
	{
		return 0;
	}

	for (POSITION posTlb = gAllToolbars.GetHeadPosition (); posTlb != NULL;)
	{
		CBCGPToolBar* pToolBar = (CBCGPToolBar*) gAllToolbars.GetNext (posTlb);
		ASSERT (pToolBar != NULL);

		if (CWnd::FromHandlePermanent (pToolBar->m_hWnd) != NULL)
		{
			ASSERT_VALID(pToolBar);

			for (POSITION pos = pToolBar->m_Buttons.GetHeadPosition (); pos != NULL;)
			{
				CBCGPToolbarButton* pButton = (CBCGPToolbarButton*) pToolBar->m_Buttons.GetNext (pos);
				if (pButton == NULL)
				{
					break;
				}

				ASSERT_VALID (pButton);

				if (pButton->m_nID == uiCmd)
				{
					listButtons.AddTail (pButton);
				}
			}
		}
	}

	return (int) listButtons.GetCount ();
}
//********************************************************************************
int CBCGPToolBar::GetCommandRangeButtons (UINT uiFirstID, UINT uiLastID, CObList& listButtons)
{
	ASSERT(uiFirstID <= uiLastID);

	listButtons.RemoveAll ();

	for (POSITION posTlb = gAllToolbars.GetHeadPosition (); posTlb != NULL;)
	{
		CBCGPToolBar* pToolBar = (CBCGPToolBar*) gAllToolbars.GetNext (posTlb);
		ASSERT (pToolBar != NULL);

		if (CWnd::FromHandlePermanent (pToolBar->m_hWnd) != NULL)
		{
			ASSERT_VALID(pToolBar);

			for (POSITION pos = pToolBar->m_Buttons.GetHeadPosition (); pos != NULL;)
			{
				CBCGPToolbarButton* pButton = (CBCGPToolbarButton*) pToolBar->m_Buttons.GetNext (pos);
				if (pButton == NULL)
				{
					break;
				}

				ASSERT_VALID (pButton);

				if (pButton->m_nID >= uiFirstID && pButton->m_nID <= uiLastID)
				{
					listButtons.AddTail (pButton);
				}
			}
		}
	}

	return (int) listButtons.GetCount ();
}
//********************************************************************************
int CBCGPToolBar::FindDropIndex (const CPoint p, CRect& rectDrag) const
{
	int iDragButton = -1;
	rectDrag.SetRectEmpty ();

	BOOL bHorz = (GetCurrentAlignment () & CBRS_ORIENT_HORZ) ? TRUE : FALSE;

	CPoint point = p;
	if (point.y < 0)
	{
		point.y = 0;
	}

	if (m_Buttons.IsEmpty () || 
		(m_Buttons.GetCount () == 1 && m_pCustomizeBtn != NULL))
	{
		GetClientRect (&rectDrag);
		iDragButton = 0;
	}
	else
	{
		if (bHorz)
		{
			int iOffset = GetRowHeight ();
			int iButton = 0;
			CRect rectPrev;
			rectPrev.SetRectEmpty ();

			POSITION pos;
			for (pos = m_Buttons.GetHeadPosition (); pos != NULL; iButton ++)
			{
				CBCGPToolbarButton* pButton = (CBCGPToolbarButton*) m_Buttons.GetNext (pos);
				ASSERT (pButton != NULL);

				if (!pButton->IsHidden () && pButton->IsVisible () && pButton != m_pCustomizeBtn)
				{
					CRect rect = pButton->Rect ();

					if (iButton > 0 && rect.top > rectPrev.bottom)
					{
						iOffset	= rect.top - rectPrev.bottom;
						break;
					}

					rectPrev = rect;
				}
			}

			int iCursorRow = point.y / (GetRowHeight () + iOffset);
			int iRow = 0;
			iButton = 0;

			for (pos = m_Buttons.GetHeadPosition (); pos != NULL; iButton ++)
			{
				CBCGPToolbarButton* pButton = (CBCGPToolbarButton*) m_Buttons.GetNext (pos);
				ASSERT (pButton != NULL);

				if (!pButton->IsHidden () && pButton->IsVisible () && pButton != m_pCustomizeBtn)
				{
					CRect rect = pButton->Rect ();
					
					if (iButton > 0 && rect.top >= rectPrev.bottom)
					{
						iRow ++;
					}

					if (iRow > iCursorRow)
					{
						rectDrag = rectPrev;
						rectDrag.left = rectDrag.right;
						iDragButton = iButton - 1;
						break;
					}

					if (iRow == iCursorRow)
					{
						if (point.x < rect.left)
						{
							iDragButton = iButton;
							rectDrag = rect;
							rectDrag.right = rectDrag.left;
							break;
						}
						else if (point.x <= rect.right)
						{
							rectDrag = rect;
							if (point.x - rect.left > rect.right - point.x)
							{
								iDragButton = iButton + 1;
								rectDrag.left = rectDrag.right;
							}
							else
							{
								iDragButton = iButton;
								rectDrag.right = rectDrag.left;
							}
							break;
						}
					}

					rectPrev = rect;
				}
			}

			if (iDragButton == -1 && iRow == iCursorRow)
			{
				rectDrag = rectPrev;
				rectDrag.left = rectDrag.right;
				iDragButton = iButton;
			}
		}
		else
		{
			int iButton = 0;
			for (POSITION pos = m_Buttons.GetHeadPosition (); pos != NULL; iButton ++)
			{
				CBCGPToolbarButton* pButton = (CBCGPToolbarButton*) m_Buttons.GetNext (pos);
				ASSERT (pButton != NULL);

				if (pButton != m_pCustomizeBtn)
				{
					CRect rect = pButton->Rect ();

					if (point.y < rect.top)
					{
						iDragButton = iButton;
						rectDrag = rect;
						rectDrag.bottom = rectDrag.top;
						break;
					}
					else if (point.y <= rect.bottom)
					{
						rectDrag = rect;
						if (point.y - rect.top > rect.bottom - point.y)
						{
							iDragButton = iButton + 1;
							rectDrag.top = rectDrag.bottom;
						}
						else
						{
							iDragButton = iButton;
							rectDrag.bottom = rectDrag.top;
						}
						break;
					}
				}
			}
		}
	}

	if (iDragButton >= 0)
	{
		const int iCursorSize = 6;

		CRect rectClient;	// Client area rectangle
		GetClientRect (&rectClient);

		if (m_pCustomizeBtn != NULL && iDragButton == m_Buttons.GetCount ())
		{
			iDragButton = max (0, (int) m_Buttons.GetCount () - 1);
		}

		if (bHorz)
		{
			rectDrag.left = 
				max (rectClient.left, rectDrag.left - iCursorSize / 2);

			rectDrag.right = rectDrag.left + iCursorSize;
			if (rectDrag.right > rectClient.right)
			{
				rectDrag.right = rectClient.right;
				rectDrag.left = rectDrag.right - iCursorSize;
			}
		}
		else
		{
			rectDrag.top = 
				max (rectClient.top, rectDrag.top - iCursorSize / 2);

			rectDrag.bottom = rectDrag.top + iCursorSize;
			if (rectDrag.bottom > rectClient.bottom)
			{
				rectDrag.bottom = rectClient.bottom;
				rectDrag.top = rectDrag.bottom - iCursorSize;
			}
		}
	}

	if (m_pCustomizeBtn != NULL && iDragButton == m_Buttons.GetCount ())
	{
		iDragButton = -1;
		rectDrag.SetRectEmpty ();
	}

	return iDragButton;
}
//***********************************************************************
void CBCGPToolBar::DrawDragMarker (CDC* pDC)
{
	BOOL bHorz = (GetCurrentAlignment () & CBRS_ORIENT_HORZ) ? TRUE : FALSE;

	CPen* pOldPen = (CPen*) pDC->SelectObject (&m_penDrag);

	for (int i = 0; i < 2; i ++)
	{
		if (bHorz)
		{
			pDC->MoveTo (m_rectDrag.left + m_rectDrag.Width () / 2 + i - 1, m_rectDrag.top);
			pDC->LineTo (m_rectDrag.left + m_rectDrag.Width () / 2 + i - 1, m_rectDrag.bottom);

			pDC->MoveTo (m_rectDrag.left + i, m_rectDrag.top + i);
			pDC->LineTo (m_rectDrag.right - i, m_rectDrag.top + i);

			pDC->MoveTo (m_rectDrag.left + i, m_rectDrag.bottom - i - 1);
			pDC->LineTo (m_rectDrag.right - i, m_rectDrag.bottom - i - 1);
		}
		else
		{
			pDC->MoveTo (m_rectDrag.left, m_rectDrag.top + m_rectDrag.Height () / 2 + i - 1);
			pDC->LineTo (m_rectDrag.right, m_rectDrag.top + m_rectDrag.Height () / 2 + i - 1);

			pDC->MoveTo (m_rectDrag.left + i, m_rectDrag.top + i);
			pDC->LineTo (m_rectDrag.left + i, m_rectDrag.bottom - i);

			pDC->MoveTo (m_rectDrag.right - i - 1, m_rectDrag.top + i);
			pDC->LineTo (m_rectDrag.right - i - 1, m_rectDrag.bottom - i);
		}
	}

	pDC->SelectObject (pOldPen);
}
//********************************************************************
void CBCGPToolBar::OnDestroy() 
{
	m_penDrag.DeleteObject ();

	CBCGPTooltipManager::DeleteToolTip (m_pToolTip);
	CBCGPBaseToolBar::OnDestroy();

	if (m_pSelToolbar == this)
	{
		m_pSelToolbar = NULL;
	}

	for (POSITION pos = gAllToolbars.GetHeadPosition (); pos != NULL;)
	{
		POSITION posSave = pos;

		CBCGPToolBar* pToolBar = (CBCGPToolBar*) gAllToolbars.GetNext (pos);
		ASSERT (pToolBar != NULL);

		if (CWnd::FromHandlePermanent (pToolBar->m_hWnd) != NULL)
		{
			ASSERT_VALID(pToolBar);

			if (pToolBar == this)
			{
				gAllToolbars.RemoveAt (posSave);
				break;
			}
		}
	}
}
//********************************************************************
void CBCGPToolBar::Serialize (CArchive& ar)
{
	CBCGPBaseToolBar::Serialize (ar);

	if (m_bLocked) 
	{
		return;
	}

	POSITION pos;
	CString strName;

	try
	{
		
		if (ar.IsLoading ())
		{
			//-----------------------
			// Save customize button:
			//-----------------------
			CCustomizeButton* pCustomizeBtn = NULL;
			if (m_pCustomizeBtn != NULL)
			{
				ASSERT_VALID (m_pCustomizeBtn);
				ASSERT (m_pCustomizeBtn == m_Buttons.GetTail ());	// Should be last

				CRuntimeClass* pRTC = m_pCustomizeBtn->GetRuntimeClass ();
				pCustomizeBtn = DYNAMIC_DOWNCAST (CCustomizeButton, 
					pRTC->CreateObject ());

				ASSERT_VALID (pCustomizeBtn);
				pCustomizeBtn->CopyFrom (*m_pCustomizeBtn);
			}

			RemoveAllButtons ();
			m_Buttons.Serialize (ar);

			for (pos = m_Buttons.GetHeadPosition (); pos != NULL;)
			{
				CBCGPToolbarButton* pButton = (CBCGPToolbarButton*) m_Buttons.GetNext (pos);
				if (pButton == NULL)
				{
					// Corrupted data!
					ASSERT (FALSE);
					m_Buttons.RemoveAll ();	// Memory leak! Don't delete wrong objects.

					if (CanBeRestored ())
					{
						RestoreOriginalstate ();
					}
					AdjustLocations ();
					return;
				}

				pButton->m_nStyle &= ~(TBBS_PRESSED | TBBS_CHECKED);	// Fix for the "stuck" buttons.
				pButton->OnChangeParentWnd (this);
			}
			
			BOOL bTextLabels;
			ar >> bTextLabels;
			if (AllowChangeTextLabels ())
			{
				m_bTextLabels = bTextLabels;
			}

			//--------------------------
			// Restore customize button:
			//--------------------------
			if (pCustomizeBtn != NULL)
			{
				InsertButton (pCustomizeBtn);
				m_pCustomizeBtn = pCustomizeBtn;
			}

			AdjustLocations ();

			ar >> strName;

			if (::IsWindow (m_hWnd))
			{
				SetWindowText (strName);
			}

			//--------------------------
			// Remove all "bad" buttons:
			//--------------------------
			for (pos = m_lstUnpermittedCommands.GetHeadPosition (); pos != NULL;)
			{
				UINT uiCmd = m_lstUnpermittedCommands.GetNext (pos);

				int iIndex = CommandToIndex (uiCmd);
				if (iIndex >= 0)
				{
					RemoveButton (iIndex);
				}
			}

			if (g_pWorkspace != NULL && g_pWorkspace->GetDataVersion () >= 0x60200)
			{
				ar >> m_nMRUWidth;		
			}			
		}
		else
		{
			//-----------------------------------
			// Serialize just "Storable" buttons:
			//-----------------------------------
			CObList buttons;

			for (pos = m_Buttons.GetHeadPosition (); pos != NULL;)
			{
				CBCGPToolbarButton* pButton = (CBCGPToolbarButton*) m_Buttons.GetNext (pos);
				ASSERT_VALID (pButton);

				if (pButton->CanBeStored ())
				{
					buttons.AddTail (pButton);
				}
			}

			buttons.Serialize (ar);
			ar << m_bTextLabels;

			if (::IsWindow (m_hWnd))
			{
				GetWindowText (strName);
			}

			ar << strName;
			ar << m_nMRUWidth;
		}
	}
	catch (CArchiveException* pEx)
	{
		TRACE(_T("Archive exception in CBCGPToolBar::Serialize!\n"));
		pEx->Delete ();

	}
	catch (...)
	{
		
	}
}
//*********************************************************************
BOOL CBCGPToolBar::SaveState (LPCTSTR lpszProfileName, int nIndex, UINT uiID)
{
	CString strProfileName = ::BCGPGetRegPath (strToolbarProfile, lpszProfileName);

	BOOL bResult = FALSE;

	if (nIndex == -1)
	{
		nIndex = GetDlgCtrlID ();
	}

	CString strSection;
	if (uiID == (UINT) -1)
	{
		strSection.Format (REG_SECTION_FMT, strProfileName, nIndex);
	}
	else
	{
		strSection.Format (REG_SECTION_FMT_EX, strProfileName, nIndex, uiID);
	}

	try
	{
		CMemFile file;

		{
			CArchive ar (&file, CArchive::store);

			Serialize (ar);
			ar.Flush ();
		}

		UINT uiDataSize = (UINT) file.GetLength ();
		LPBYTE lpbData = file.Detach ();

		if (lpbData != NULL)
		{
			CBCGPRegistrySP regSP;
			CBCGPRegistry& reg = regSP.Create (FALSE, FALSE);

			if (reg.CreateKey (strSection))
			{
				if (::IsWindow (m_hWnd))
				{
					CString strToolbarName;
					GetWindowText (strToolbarName);

					reg.Write (REG_ENTRY_NAME, strToolbarName);
				}

				bResult = reg.Write (REG_ENTRY_BUTTONS, lpbData, uiDataSize);

				// Save orinal (before customization) state:
				if (bResult && 
					GetWorkspace () != NULL &&
					GetWorkspace ()->IsResourceSmartUpdate ())
				{
					// Save orginal (before customization) state:
					SaveOriginalState (reg);
				}
					// Save orginal after reset state
					SaveResetOriginalState(reg);
			}

			free (lpbData);
		}
	}
	catch (CMemoryException* pEx)
	{
		pEx->Delete ();
		TRACE(_T("Memory exception in CBCGPToolBar::SaveState ()!\n"));
	}

	bResult = CBCGPControlBar::SaveState (lpszProfileName, nIndex, uiID);

	return bResult;
}
//*********************************************************************
BOOL CBCGPToolBar::RemoveStateFromRegistry (LPCTSTR lpszProfileName, int nIndex, UINT uiID)
{
	CString strProfileName = ::BCGPGetRegPath (strToolbarProfile, lpszProfileName);

	if (nIndex == -1)
	{
		nIndex = GetDlgCtrlID ();
	}

	CString strSection;
	if (uiID == (UINT) -1)
	{
		strSection.Format (REG_SECTION_FMT, strProfileName, nIndex);
	}
	else
	{
		strSection.Format (REG_SECTION_FMT_EX, strProfileName, nIndex, uiID);
	}

	CBCGPRegistrySP regSP;
	CBCGPRegistry& reg = regSP.Create (FALSE, FALSE);

	return reg.DeleteKey (strSection);
}
//*********************************************************************
BOOL CBCGPToolBar::LoadState (LPCTSTR lpszProfileName, int nIndex, UINT uiID)
{
	CString strProfileName = ::BCGPGetRegPath (strToolbarProfile, lpszProfileName);

	BOOL bResult = FALSE;

	if (nIndex == -1)
	{
		nIndex = GetDlgCtrlID ();
	}

	CString strSection;
	if (uiID == (UINT) -1)
	{
		strSection.Format (REG_SECTION_FMT, strProfileName, nIndex);
	}
	else
	{
		strSection.Format (REG_SECTION_FMT_EX, strProfileName, nIndex, uiID);
	}

	LPBYTE	lpbData = NULL;
	UINT	uiDataSize;

	CBCGPRegistrySP regSP;
	CBCGPRegistry& reg = regSP.Create (FALSE, TRUE);

	if (!reg.Open (strSection))
	{
		return FALSE;
	}

	if (!reg.Read (REG_ENTRY_BUTTONS, &lpbData, &uiDataSize))
	{
		return FALSE;
	}

	try
	{
		CMemFile file (lpbData, uiDataSize);
		CArchive ar (&file, CArchive::load);

		Serialize (ar);
		bResult = TRUE;
	}
	catch (CMemoryException* pEx)
	{
		pEx->Delete ();
		TRACE(_T("Memory exception in CBCGPToolBar::LoadState ()!\n"));
	}
	catch (CArchiveException* pEx)
	{
		pEx->Delete ();
		TRACE(_T("CArchiveException exception in CBCGPToolBar::LoadState ()!\n"));

		m_Buttons.RemoveAll ();	// Memory leak! Don't delete wrong objects.
		if (CanBeRestored ())
		{
			RestoreOriginalstate ();
		}

	}

	if (lpbData != NULL)
	{
		delete [] lpbData;
	}

	LoadResetOriginalState(reg);

	if (bResult &&
		GetWorkspace () != NULL &&
		GetWorkspace ()->IsResourceSmartUpdate ())
	{
		LoadLastOriginalState (reg);
	}

	bResult = CBCGPControlBar::LoadState (lpszProfileName, nIndex, uiID);

	AdjustLayout ();

	if (m_pParentDockBar != NULL && m_pDockBarRow != NULL)
	{
		ASSERT_VALID (m_pParentDockBar);
		ASSERT_VALID (m_pDockBarRow);
		
		CSize sizeCurr = CalcFixedLayout (TRUE, IsHorizontal ());
		m_pParentDockBar->ResizeRow (m_pDockBarRow, 
			 IsHorizontal () ? sizeCurr.cy : sizeCurr.cx);
	}

	return bResult;
}
//*******************************************************************************************
void CBCGPToolBar::OnContextMenu(CWnd* pWnd, CPoint point)
{
	if (m_bLocked && IsCustomizeMode ())
	{
		MessageBeep ((UINT) -1);
		return;
	}

	if (IsDragMode () || m_bStretchButton)
	{
		return;
	}

	OnChangeHot (-1);

	if (!IsCustomizeMode ())
	{
		if (m_iHighlighted >= 0 && point == CPoint(-1, -1))
		{
			Deactivate ();
			RestoreFocus ();
		}
		
		CBCGPBaseToolBar::OnContextMenu (pWnd, point);
		return;
	}

	SetFocus ();

	CPoint ptClient = point;
	ScreenToClient (&ptClient);

	int iButton = HitTest(ptClient);

	int iSelected = m_iSelected;
	m_iSelected = iButton;

	if (iSelected != -1)
	{
		InvalidateButton (iSelected);
	}

	if (m_iSelected != -1)
	{
		InvalidateButton (m_iSelected);
	}

	if (m_pSelToolbar != this)
	{
		CBCGPToolBar* pSelToolbar = m_pSelToolbar;
		m_pSelToolbar = this;

		if (pSelToolbar != NULL)
		{
			ASSERT_VALID (pSelToolbar);

			int iOldSelected = pSelToolbar->m_iSelected;
			pSelToolbar->m_iSelected = -1;
			pSelToolbar->InvalidateButton (iOldSelected);
		}
	}

	UpdateWindow ();

	if (iButton < 0) // nothing hit
	{
		return;
	}

	CBCGPToolbarButton* pButton = GetButton(m_iSelected);
	if (pButton == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	ASSERT(!(pButton->m_nStyle & TBBS_SEPARATOR));

	if (!pButton->IsEditable ())
	{
		m_iSelected = -1;
		InvalidateButton (iButton);
		UpdateWindow ();

		return;
	}

	if (pButton->CanBeStored ())
	{
		if (point.x == -1 && point.y == -1){
			//keystroke invocation
			CRect rect;
			GetClientRect(rect);
			ClientToScreen(rect);

			point = rect.TopLeft();
			point.Offset(5, 5);
		}

		CBCGPLocalResource locaRes;

		CMenu menu;
		VERIFY(menu.LoadMenu(IDR_BCGBARRES_POPUP_BCGTOOL_BAR));

		CMenu* pPopup = menu.GetSubMenu(0);
		ASSERT(pPopup != NULL);

		if (pButton->IsLocked ())
		{
			pPopup->EnableMenuItem (ID_BCGBARRES_TOOLBAR_RESET, MF_BYCOMMAND | MF_GRAYED);
		}

		if (!EnableContextMenuItems (pButton, pPopup))
		{
			return;
		}

		//Disable StartGroup Item if left button is not visible
		int nPrevIndex = m_iSelected-1;
		if (nPrevIndex >= 0)
		{
			CBCGPToolbarButton* pPrevButton = GetButton (nPrevIndex);
			if (pPrevButton != NULL && !pPrevButton->IsVisible ())
			{
				pPopup->EnableMenuItem (ID_BCGBARRES_TOOLBAR_START_GROUP, MF_BYCOMMAND | MF_GRAYED);
			}
		}

		pPopup->TrackPopupMenu (TPM_LEFTALIGN | TPM_RIGHTBUTTON,
								point.x, point.y, this);
	}
}
//*******************************************************************************************
void CBCGPToolBar::OnToolbarAppearance() 
{
	ASSERT (IsCustomizeMode () && !m_bLocked);
	ASSERT (m_iSelected >= 0);

	CBCGPToolbarButton* pButton = GetButton(m_iSelected);
	if (pButton == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	ASSERT(!(pButton->m_nStyle & TBBS_SEPARATOR));

	if (pButton->m_strText.IsEmpty ())
	{
		OnSetDefaultButtonText (pButton);
	}

	CBCGPLocalResource locaRes;

	CButtonAppearanceDlg dlg (pButton, m_pUserImages, this, 0, IsPureMenuButton (pButton));
	if (dlg.DoModal () == IDOK)
	{
		AdjustLayout ();
		AdjustSizeImmediate();
		RedrawWindow (NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
	}
}
//*******************************************************************************************
void CBCGPToolBar::OnToolbarDelete() 
{
	ASSERT (m_iSelected >= 0);
	RemoveButton (m_iSelected);
	
	m_iSelected = -1;

	OnAfterButtonDelete ();
}
//*******************************************************************************************
void CBCGPToolBar::OnAfterButtonDelete ()
{
	if (IsFloating ())
	{
		AdjustLayout ();
	}
	else
	{
		AdjustSizeImmediate ();
	}
}
//*********************************************************************************
void CBCGPToolBar::OnToolbarImage() 
{
	CBCGPLocalResource locaRes;

	ASSERT (IsCustomizeMode () && !m_bLocked);
	ASSERT (m_iSelected >= 0);

	CBCGPToolbarButton* pButton = GetButton(m_iSelected);
	if (pButton == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	ASSERT(!(pButton->m_nStyle & TBBS_SEPARATOR));

	BOOL bSaveText = pButton->m_bText;
	BOOL bSaveImage = pButton->m_bImage;

	pButton->m_bText = FALSE;
	pButton->m_bImage = TRUE;

	if (pButton->GetImage () < 0)
	{
		CButtonAppearanceDlg dlg (pButton, m_pUserImages, this, 0, IsPureMenuButton (pButton));
		if (dlg.DoModal () != IDOK)
		{
			pButton->m_bText = bSaveText;
			pButton->m_bImage = bSaveImage;
			return;
		}
	}

	AdjustLayout ();
	AdjustSizeImmediate();
}
//*******************************************************************************************
void CBCGPToolBar::OnToolbarImageAndText() 
{
	ASSERT (IsCustomizeMode () && !m_bLocked);
	ASSERT (m_iSelected >= 0);

	CBCGPToolbarButton* pButton = GetButton(m_iSelected);
	if (pButton == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	ASSERT(!(pButton->m_nStyle & TBBS_SEPARATOR));

	BOOL bSaveText = pButton->m_bText;
	BOOL bSaveImage = pButton->m_bImage;

	pButton->m_bText = TRUE;
	pButton->m_bImage = TRUE;

	if (pButton->GetImage () < 0)
	{
		CBCGPLocalResource locaRes;

		CButtonAppearanceDlg dlg (pButton, m_pUserImages, this, 0, IsPureMenuButton (pButton));
		if (dlg.DoModal () != IDOK)
		{
			pButton->m_bText = bSaveText;
			pButton->m_bImage = bSaveImage;
			return;
		}
	}

	if (pButton->m_strText.IsEmpty ())
	{
		OnSetDefaultButtonText (pButton);
	}

	if (pButton->m_strText.IsEmpty ())
	{
		MessageBeep ((UINT) -1);

		pButton->m_bText = FALSE;
		pButton->m_bImage = TRUE;
	}

	AdjustLayout ();
}
//*******************************************************************************************
void CBCGPToolBar::OnToolbarStartGroup() 
{
	ASSERT (m_iSelected > 0);

	CBCGPToolbarButton* pPrevButton = NULL;
	int i = 0;
	for (POSITION pos = m_Buttons.GetHeadPosition (); pos != NULL; i ++)
	{
		CBCGPToolbarButton* pCurrButton = (CBCGPToolbarButton*) m_Buttons.GetNext (pos);
		ASSERT_VALID (pCurrButton);

		if (i == m_iSelected)
		{
			ASSERT (pPrevButton != NULL);	// m_iSelected > 0!

			if (pPrevButton->m_nStyle & TBBS_SEPARATOR)
			{
				if (pPrevButton->IsVisible ())
				{
					VERIFY (RemoveButton (m_iSelected - 1));
				}
			}
			else
			{
				InsertSeparator (m_iSelected ++);
			}

			break;
		}

		pPrevButton = pCurrButton;
	}

	AdjustLayout ();
}
//*******************************************************************************************
void CBCGPToolBar::OnToolbarText() 
{
	ASSERT (IsCustomizeMode () && !m_bLocked);
	ASSERT (m_iSelected >= 0);

	CBCGPToolbarButton* pButton = GetButton(m_iSelected);
	if (pButton == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	ASSERT(!(pButton->m_nStyle & TBBS_SEPARATOR));

	pButton->m_bText = TRUE;
	pButton->m_bImage = FALSE;

	if (pButton->m_strText.IsEmpty ())
	{
		OnSetDefaultButtonText (pButton);
	}

	if (pButton->m_strText.IsEmpty ())
	{
		MessageBeep ((UINT) -1);

		pButton->m_bText = FALSE;
		pButton->m_bImage = TRUE;
	}

	AdjustLayout ();
	AdjustSizeImmediate();
}
//************************************************************************************
void CBCGPToolBar::OnWindowPosChanged(WINDOWPOS FAR* lpwndpos) 
{
	BOOL bExpanded = (m_sizeLast.cx < lpwndpos->cx ||
						 m_sizeLast.cy < lpwndpos->cy);

	m_sizeLast = CSize (lpwndpos->cx, lpwndpos->cy);


	CBCGPBaseToolBar::OnWindowPosChanged(lpwndpos);
	CWnd* pParent = GetParent ();
	
	if (bExpanded || (pParent != NULL && pParent->IsKindOf (RUNTIME_CLASS (CBCGPReBar))))
	{
		RedrawWindow (NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_FRAME | RDW_ALLCHILDREN);
	}

	RedrawCustomizeButton ();
}
//**************************************************************************************
HBRUSH CBCGPToolBar::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CBCGPBaseToolBar::OnCtlColor (pDC, pWnd, nCtlColor);
	if (!CBCGPVisualManager::GetInstance ()->AreCustomToolbarCtrlColors() && (!IsCustomizeMode () || m_bLocked))
	{
		return hbr;
	}

	for (POSITION pos = m_Buttons.GetHeadPosition (); pos != NULL;)
	{
		CBCGPToolbarButton* pButton = (CBCGPToolbarButton*) m_Buttons.GetNext (pos);
		ASSERT_VALID (pButton);

		if (pButton->IsOwnerOf (pWnd->GetSafeHwnd ()))
		{
			HBRUSH hbrButton = pButton->OnCtlColor (pDC, nCtlColor);
			return (hbrButton == NULL) ? hbr : hbrButton;
		}
	}

	return hbr;
}
//**************************************************************************************
int CBCGPToolBar::GetCount () const
{
	return (int) m_Buttons.GetCount ();
}
//*************************************************************************************
BOOL CBCGPToolBar::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	BOOL bStretch = m_bStretchButton;

	CPoint ptCursor;
	::GetCursorPos (&ptCursor);

	if (!bStretch && IsCustomizeMode () && m_iSelected != -1 && !m_bLocked)
	{
		ScreenToClient (&ptCursor);

		if (HitTest (ptCursor) == m_iSelected)
		{
			CBCGPToolbarButton* pButton = GetButton (m_iSelected);
			ASSERT_VALID (pButton);

			if (pButton->CanBeStretched () &&
				abs (ptCursor.x - pButton->Rect ().right) <= STRETCH_DELTA)
			{
				bStretch = TRUE;
			}
		}
	}

	if (bStretch)
	{
		::SetCursor (globalData.m_hcurStretch);
		return TRUE;
	}

	CPoint ptCursorClient = ptCursor;
	ScreenToClient (&ptCursorClient);

	CRect rectGripper;
	m_Impl.GetGripperRect (rectGripper, TRUE);

	if (rectGripper.PtInRect (ptCursorClient) &&
		CBCGPPopupMenu::GetActiveMenu () == NULL)
	{
		SetCursor (globalData.m_hcurSizeAll);
		return TRUE;
	}

	return CBCGPBaseToolBar::OnSetCursor(pWnd, nHitTest, message);
}
//****************************************************************************************
BOOL CBCGPToolBar::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message == WM_KEYDOWN &&
		pMsg->wParam == VK_ESCAPE)
	{
		if (m_bStretchButton)
		{
			OnCancelMode ();
		}
		else
		{
			CBCGPToolbarMenuButton* pMenuButon = GetDroppedDownMenu ();
			if (pMenuButon != NULL)
			{
				return CBCGPBaseToolBar::PreTranslateMessage(pMsg);
			}

			Deactivate ();
			RestoreFocus ();
		}

		return TRUE;
	}

	if(pMsg->message == BCGM_RESETRPROMPT)
	{
		OnPromptReset(0,0);

		return TRUE;
	}


   	switch (pMsg->message)
	{
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
	case WM_NCLBUTTONDOWN:
	case WM_NCRBUTTONDOWN:
	case WM_NCMBUTTONDOWN:
	case WM_NCLBUTTONUP:
	case WM_NCRBUTTONUP:
	case WM_NCMBUTTONUP:
	case WM_MOUSEMOVE:
		if (m_pToolTip->GetSafeHwnd () != NULL)
		{
			m_pToolTip->RelayEvent(pMsg);
		}
	}

	return CBCGPBaseToolBar::PreTranslateMessage(pMsg);
}
//**************************************************************************************
BOOL CBCGPToolBar::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (IsCustomizeMode () && !m_bLocked)
	{
		return CBCGPBaseToolBar::OnCommand (wParam, lParam);
	}

	BOOL bAccelerator = FALSE;
	int nNotifyCode = HIWORD (wParam);

	// Find the control send the message:
	HWND hWndCtrl = (HWND)lParam;
	if (hWndCtrl == NULL)
	{
		if (wParam == IDCANCEL)	// ESC was pressed
		{
			RestoreFocus ();
			return TRUE;
		}

		if (wParam != IDOK ||
			(hWndCtrl = ::GetFocus ()) == NULL)
		{
			return FALSE;
		}

		bAccelerator = TRUE;
		nNotifyCode = 0;
	}

	for (POSITION pos = m_Buttons.GetHeadPosition (); pos != NULL;)
	{
		CBCGPToolbarButton* pButton = (CBCGPToolbarButton*) m_Buttons.GetNext (pos);
		ASSERT_VALID (pButton);

		HWND hwdList = pButton->GetHwnd ();
		if (hwdList == NULL)	// No control
		{
			continue;
		}

		if (hwdList == hWndCtrl || ::IsChild (hwdList, hWndCtrl))
		{
			if (!NotifyControlCommand (pButton, bAccelerator, nNotifyCode, wParam, lParam))
			{
				if (m_bAllowReflections)
				{
					return FALSE;
				}
				break;
			}

			return TRUE;
		}
	}

	return FALSE;
}
//*************************************************************************************
BOOL CBCGPToolBar::NotifyControlCommand (CBCGPToolbarButton* pButton,
										 BOOL bAccelerator,
										 int nNotifyCode,
										 WPARAM wParam,
										 LPARAM lParam)
{
	UNUSED_ALWAYS(wParam);

	ASSERT_VALID (this);
	ASSERT_VALID (pButton);

	if (!bAccelerator)
	{
		ASSERT (LOWORD (wParam) == pButton->m_nID);
		if (!pButton->NotifyCommand (nNotifyCode))
		{
			return FALSE;
		}
	}

	GetOwner()->PostMessage (WM_COMMAND, 
							MAKEWPARAM (pButton->m_nID, nNotifyCode), lParam);
	return TRUE;
}
//*************************************************************************************
CBCGPToolBar* CBCGPToolBar::FromHandlePermanent (HWND hwnd)
{
	for (POSITION posTlb = gAllToolbars.GetHeadPosition (); posTlb != NULL;)
	{
		CBCGPToolBar* pToolBar = (CBCGPToolBar*) gAllToolbars.GetNext (posTlb);
		if (pToolBar->GetSafeHwnd () == hwnd)
		{
			return pToolBar;
		}
	}

	return NULL;
}
//**********************************************************************************
CSize CBCGPToolBar::StretchControlBar (int nLength, BOOL bVert)
{
	ASSERT_VALID(this);
	ASSERT(::IsWindow(m_hWnd));

	if (m_pCustomizeBtn != NULL)
	{
		ASSERT_VALID (m_pCustomizeBtn);
		m_pCustomizeBtn->m_bIsEmpty = FALSE;
	}

	m_nMaxBtnHeight = CalcMaxButtonHeight ();

	CSize sizeResult (0,0);

	CRect rect;
	rect.SetRectEmpty ();

	int nLen = nLength + (IsHorizontal () ? rect.Height() : rect.Width());

	SizeToolBar (nLen, bVert);

	sizeResult = CalcSize (!IsHorizontal ());
	if (m_pParentDockBar == NULL)
	{
		m_nMRUWidth = IsHorizontal () ? sizeResult.cx : sizeResult.cy;
	}

	return sizeResult;
}
//**********************************************************************************
CSize CBCGPToolBar::CalcLayout(DWORD dwMode, int nLength)
{
	ASSERT_VALID(this);
	ASSERT(::IsWindow(m_hWnd));
	
	if (m_pCustomizeBtn != NULL)
	{
		ASSERT_VALID (m_pCustomizeBtn);
		m_pCustomizeBtn->m_bIsEmpty = FALSE;
	}

	if (dwMode & LM_HORZDOCK)
	{
		ASSERT(dwMode & LM_HORZ);
	}

	m_nMaxBtnHeight = CalcMaxButtonHeight ();

	CSize sizeResult(0,0);

	if (!(m_dwStyle & CBRS_SIZE_FIXED))
	{
		BOOL bDynamic = m_dwStyle & CBRS_SIZE_DYNAMIC;

		if (bDynamic && (dwMode & LM_MRUWIDTH))
			SizeToolBar(m_nMRUWidth);
		else if (bDynamic && (dwMode & LM_HORZDOCK))
			SizeToolBar(32767);
		else if (bDynamic && (dwMode & LM_VERTDOCK))
		{
			SizeToolBar(0);
		}
		else if (bDynamic && (nLength != -1))
		{
			CRect rect; rect.SetRectEmpty();
			CalcInsideRect(rect, (dwMode & LM_HORZ));
			BOOL bVert = (dwMode & LM_LENGTHY);

			int nLen = nLength + (bVert ? rect.Height() : rect.Width());

			SizeToolBar(nLen, bVert);
		}
		else if (bDynamic && (m_dwStyle & CBRS_FLOATING))
			SizeToolBar(m_nMRUWidth);
		else
			SizeToolBar((dwMode & LM_HORZ) ? 32767 : 0);
	}

	sizeResult = CalcSize ((dwMode & LM_HORZ) == 0);
	
	if (m_pCustomizeBtn != NULL &&
		(int) m_pCustomizeBtn->m_uiCustomizeCmdId <= 0 &&
		m_pCustomizeBtn->m_lstInvisibleButtons.IsEmpty ())
	{
		ASSERT_VALID (m_pCustomizeBtn);

		// Hide "Customize button and calc. size again:
		m_pCustomizeBtn->m_bIsEmpty = TRUE;
		sizeResult = CalcSize ((dwMode & LM_HORZ) == 0);
	}

	if (dwMode & LM_COMMIT)
	{

		if ((m_dwStyle & CBRS_FLOATING) && (m_dwStyle & CBRS_SIZE_DYNAMIC) &&
			(dwMode & LM_HORZ))
		{
			m_nMRUWidth = sizeResult.cx;
		}

	}

	//BLOCK: Adjust Margins

	{
		CRect rect; rect.SetRectEmpty();
		CalcInsideRect(rect, (dwMode & LM_HORZ));
		sizeResult.cy -= rect.Height();
		sizeResult.cx -= rect.Width();

		CSize size = CBCGPBaseToolBar::CalcFixedLayout((dwMode & LM_STRETCH), (dwMode & LM_HORZ));
		sizeResult.cx = max(sizeResult.cx, size.cx);
		sizeResult.cy = max(sizeResult.cy, size.cy);

	}

	RebuildAccelerationKeys ();
	return sizeResult;
}
//**********************************************************************************
CSize CBCGPToolBar::CalcSize (BOOL bVertDock)
{
	if (m_Buttons.IsEmpty ())
	{
		return GetButtonSize ();
	}

	CClientDC dc (this);
	CFont* pOldFont = NULL;
	
	if (!bVertDock)
	{
		pOldFont = SelectDefaultFont (&dc);
	}
	else
	{
		pOldFont = dc.SelectObject (&globalData.fontVert);
	}

	ASSERT (pOldFont != NULL);

	CSize sizeGrid (GetColumnWidth (), GetRowHeight ());
	CSize sizeResult = sizeGrid;

	CRect rect; rect.SetRectEmpty();
	CalcInsideRect (rect, !bVertDock);

	int iStartX = bVertDock ? 0 : 1;
	int iStartY = bVertDock ? 1 : 0;

	CPoint cur (iStartX, iStartY);

	for (POSITION pos = m_Buttons.GetHeadPosition (); pos != NULL;)
	{
		CBCGPToolbarButton* pButton = (CBCGPToolbarButton*) m_Buttons.GetNext (pos);
		if (pButton == NULL)
		{
			break;
		}

		ASSERT_VALID (pButton);

		if (pos == NULL && m_pCustomizeBtn != NULL && IsFloating ())
		{
			ASSERT_VALID (m_pCustomizeBtn);
			ASSERT (m_pCustomizeBtn == pButton);	// Should be last
			break;
		}

		CSize sizeDefault (sizeGrid.cx, 
				m_bDrawTextLabels ? GetButtonSize ().cy : sizeGrid.cy);
		CSize sizeButton = pButton->OnCalculateSize (&dc, sizeDefault, !bVertDock);

		if (m_bDrawTextLabels)
		{
			sizeButton.cy = m_nMaxBtnHeight;
		}

		if (!bVertDock)
		{
			if ((cur.x == iStartX || pButton->m_bWrap)
				&& pButton->m_nStyle & TBBS_SEPARATOR)
			{
				sizeButton = CSize (0, 0);
			}

			sizeResult.cx = max (cur.x + sizeButton.cx, sizeResult.cx);
			sizeResult.cy = max (cur.y + sizeButton.cy, sizeResult.cy);

			cur.x += sizeButton.cx;

			if (pButton->m_bWrap)
			{
				cur.x = iStartX;
				cur.y += sizeGrid.cy + LINE_OFFSET;
			}
		}
		else
		{
			sizeResult.cx = max (cur.x + sizeButton.cx, sizeResult.cx);
			sizeResult.cy = max (cur.y + sizeButton.cy, sizeResult.cy);

			cur.x = iStartX;
			cur.y += sizeButton.cy;
		}
	}

	dc.SelectObject (pOldFont);
	return sizeResult;
}
//**********************************************************************************
int CBCGPToolBar::WrapToolBar (int nWidth, int nHeight /*= 32767*/,
							   CDC* pDC /* = NULL*/,
							   int nColumnWidth/* = -1*/, int nRowHeight/* = -1*/)
{
	int nResult = 0;
	
	CFont* pOldFont = NULL;

	BOOL bVertDock = (GetCurrentAlignment () & CBRS_ORIENT_HORZ) == 0;
	BOOL bIsClientDC = FALSE;

	if (pDC == NULL)
	{
		pDC = new CClientDC (this);
		bIsClientDC = TRUE;

		if (!bVertDock)
		{
			pOldFont = SelectDefaultFont (pDC);
		}
		else
		{
			pOldFont = pDC->SelectObject (&globalData.fontVert);
		}

		ASSERT (pOldFont != NULL);
	}

	CBCGPToolbarButton* pPrevButton = NULL;

	CRect rect;
	GetClientRect(rect);

	int x = 0;
	int y = rect.top;

    if (IsFloating())
    {
        nHeight = 32767;
    }

	CSize sizeGrid (nColumnWidth, nRowHeight);
	if (sizeGrid.cx < 0 || sizeGrid.cy < 0)
	{
		sizeGrid = CSize (GetColumnWidth (), GetRowHeight ());
	}

    if (!IsFloating() && !bVertDock && m_pCustomizeBtn != NULL)
    {
		CSize sizeButton = m_pCustomizeBtn->OnCalculateSize (pDC,
			sizeGrid, !bVertDock);
        nWidth -= sizeButton.cx;
    }

	for (POSITION pos = m_Buttons.GetHeadPosition (); pos != NULL;)
	{
		CBCGPToolbarButton* pButton = (CBCGPToolbarButton*) m_Buttons.GetNext (pos);
		if (pButton == NULL)
		{
			break;
		}

		ASSERT_VALID (pButton);

		pButton->m_bWrap = FALSE;

		if (pos == NULL && m_pCustomizeBtn != NULL)
		{
			ASSERT_VALID (m_pCustomizeBtn);
			ASSERT (m_pCustomizeBtn == pButton);	// Should be last
			break;
		}

		// Don't process invisivle buttons
		if (!pButton->IsVisible ())  
		{
			continue;
		}

		CSize sizeButton = pButton->OnCalculateSize (pDC, sizeGrid, !bVertDock);
		
		if (x == 0 && (pButton->m_nStyle & TBBS_SEPARATOR))
		{
			// Don't show separator on the first column!
			sizeButton = CSize (0, 0);
		}

		if (x + sizeButton.cx > nWidth &&
            y + sizeButton.cy < nHeight &&
			!(pButton->m_nStyle & TBBS_SEPARATOR))
		{
			if (pPrevButton != NULL)
			{
				pPrevButton->m_bWrap = TRUE;
				x = 0;
				y += sizeButton.cy + LINE_OFFSET;
				nResult ++;
			}
		}

		pPrevButton = pButton;
		x += sizeButton.cx;
	}

	if (bIsClientDC)
	{
		pDC->SelectObject (pOldFont);
		delete pDC;
	}

	return nResult + 1;
}
//**********************************************************************************
void CBCGPToolBar::SizeToolBar (int nLength, BOOL bVert)
{
	CSize size;
	const int nDefaultHeight = 32767;

	const int nColumnWidth = GetColumnWidth ();
	const int nRowHeight = GetRowHeight ();

	CClientDC dc (this);
	CFont* pOldFont = NULL;

	if ((GetCurrentAlignment () & CBRS_ORIENT_HORZ))
	{
		pOldFont = SelectDefaultFont (&dc);
	}
	else
	{
		pOldFont = (CFont*) dc.SelectObject (&globalData.fontVert);
	}
	
	ASSERT (pOldFont != NULL);

	if (!bVert)
	{
		int nMin, nMax, nTarget, nCurrent, nMid;

		// Wrap ToolBar vertically
		nMin = 0;
		nCurrent = WrapToolBar (nMin, nDefaultHeight, &dc, nColumnWidth, nRowHeight);

		// Wrap ToolBar as specified
		nMax = nLength;
		nTarget = WrapToolBar (nMax, nDefaultHeight, &dc, nColumnWidth, nRowHeight);

		if (nCurrent != nTarget)
		{
			while (nMin < nMax)
			{
				nMid = (nMin + nMax) / 2;
				nCurrent = WrapToolBar (nMid, nDefaultHeight, &dc, nColumnWidth, nRowHeight);

				if (nCurrent == nTarget)
					nMax = nMid;
				else
				{
					if (nMin == nMid)
					{
						WrapToolBar(nMax, nDefaultHeight, &dc, nColumnWidth, nRowHeight);
						break;
					}

					nMin = nMid;
				}
			}
		}

		size = CalcSize (bVert);
		WrapToolBar (size.cx, nDefaultHeight, &dc, nColumnWidth, nRowHeight);
	}
	else
	{
		int iWidth = 32767;
		WrapToolBar (iWidth, nDefaultHeight, &dc, nColumnWidth, nRowHeight);

		size = CalcSize (FALSE);
		if (nLength > size.cy)
		{
			iWidth = 0;

			do
			{
				iWidth += GetButtonSize ().cx;
				WrapToolBar (iWidth, nDefaultHeight, &dc, nColumnWidth, nRowHeight);
				size = CalcSize (FALSE);
			}
			while (nLength < size.cy);
		}

		WrapToolBar (size.cx, nDefaultHeight, &dc, nColumnWidth, nRowHeight);
	}

	dc.SelectObject (pOldFont);
}
//**********************************************************************************
void CBCGPToolBar::OnSize(UINT nType, int cx, int cy) 
{
	SetRoundedRgn ();

	CBCGPBaseToolBar::OnSize(nType, cx, cy);
	
	if (IsCustomizeMode () && !m_bLocked)
	{
		OnCancelMode ();
	}

	if (!m_bInUpdateShadow)
	{
		AdjustLocations ();
	}

	//------------------------------------------------------
	// Adjust system menu of the floating toolbar miniframe:
	//------------------------------------------------------
	if (IsFloating ())
	{
		CMiniFrameWnd* pMiniFrame = 
			DYNAMIC_DOWNCAST (CMiniFrameWnd, BCGPGetParentFrame (this));
		if (pMiniFrame != NULL)
		{
			CMenu* pSysMenu = pMiniFrame->GetSystemMenu(FALSE);
			if (pSysMenu != NULL)
			{
				pSysMenu->DeleteMenu (SC_RESTORE, MF_BYCOMMAND);
				pSysMenu->DeleteMenu (SC_MINIMIZE, MF_BYCOMMAND);
				pSysMenu->DeleteMenu (SC_MAXIMIZE, MF_BYCOMMAND);

				if (!CanBeClosed ())
				{
					pSysMenu->EnableMenuItem (SC_CLOSE, MF_BYCOMMAND | MF_GRAYED);
				}
			}
		}
	}
}
//**********************************************************************************
void CBCGPToolBar::AdjustLocations ()
{
	ASSERT_VALID(this);

	if (m_Buttons.IsEmpty () || GetSafeHwnd () == NULL)
	{
		return;
	}

	BOOL bHorz = GetCurrentAlignment () & CBRS_ORIENT_HORZ ? TRUE : FALSE;

	CRect rectClient;
	GetClientRect (rectClient);

	int xRight = rectClient.right;

	CClientDC dc (this);
	CFont* pOldFont;
	if (bHorz)
	{
		pOldFont = SelectDefaultFont (&dc);
	}
	else
	{
		pOldFont = (CFont*) dc.SelectObject (&globalData.fontVert);
	}
	
	ASSERT (pOldFont != NULL);

	int iStartOffset;
	if (bHorz)
	{
		iStartOffset = rectClient.left + 1;
	}
	else
	{
		iStartOffset = rectClient.top + 1;
	}

	int iOffset = iStartOffset;
	int y = rectClient.top;

	CSize sizeGrid (GetColumnWidth (), GetRowHeight ());

	CSize sizeCustButton (0, 0);

	BOOL bHideChevronInCustomizeMode = IsCustomizeMode() && DYNAMIC_DOWNCAST(CBCGPReBar, GetParent()) != NULL;

	if (m_pCustomizeBtn != NULL && !IsFloating () && !bHideChevronInCustomizeMode)
	{
		ASSERT_VALID (m_pCustomizeBtn);
		ASSERT (m_pCustomizeBtn == m_Buttons.GetTail ());	// Should be last

		m_pCustomizeBtn->m_lstInvisibleButtons.RemoveAll ();

		BOOL bIsEmpty = m_pCustomizeBtn->m_bIsEmpty;
		m_pCustomizeBtn->m_bIsEmpty = FALSE;

		sizeCustButton = m_pCustomizeBtn->OnCalculateSize (&dc,
			CSize (	bHorz ? sizeGrid.cx : rectClient.Width (), 
			bHorz ? rectClient.Height () : sizeGrid.cy), bHorz);

		m_pCustomizeBtn->m_bIsEmpty = bIsEmpty;
	}

	BOOL bPrevWasSeparator = FALSE;
	int nRowActualWidth = 0;
	for (POSITION pos = m_Buttons.GetHeadPosition (); pos != NULL;)
	{
		POSITION posSave = pos;

		CBCGPToolbarButton* pButton = (CBCGPToolbarButton*) m_Buttons.GetNext (pos);
		if (pButton == NULL)
		{
			break;
		}

		ASSERT_VALID (pButton);

		BOOL bVisible = TRUE;

		CSize sizeButton = pButton->OnCalculateSize (&dc, sizeGrid, bHorz);
		if (pButton->m_bTextBelow && bHorz)
		{
			sizeButton.cy =  sizeGrid.cy;
		}

		if (pButton->m_nStyle & TBBS_SEPARATOR)
		{
			if (iOffset == iStartOffset || bPrevWasSeparator)
			{
				sizeButton = CSize (0, 0);
				bVisible = FALSE;
			}
			else
			{
				bPrevWasSeparator = TRUE;
			}
		}

		int iOffsetPrev = iOffset;

		CRect rectButton;
		if (bHorz)
		{
			rectButton.left = iOffset;
			rectButton.right = rectButton.left + sizeButton.cx;
			rectButton.top = y;
			rectButton.bottom = rectButton.top + sizeButton.cy;
			
			iOffset += sizeButton.cx;
			nRowActualWidth += sizeButton.cx;
		}
		else
		{
			rectButton.left = rectClient.left;
			rectButton.right = rectClient.left + sizeButton.cx;
			rectButton.top = iOffset;
			rectButton.bottom = iOffset + sizeButton.cy;

			iOffset += sizeButton.cy;
		}

		if (m_pCustomizeBtn != NULL && pButton != m_pCustomizeBtn &&
			!IsFloating () && !IsCustomizeMode ())
		{
			CSize fakeSizeCustButton (sizeCustButton);

			//-------------------------------------------------------------------------
			// I assume, that the customize button is at the tail position at any time.
			//-------------------------------------------------------------------------
			if ((int) m_pCustomizeBtn->m_uiCustomizeCmdId <= 0 &&
				(pos != NULL && m_Buttons.GetAt (pos) == m_pCustomizeBtn) && 
				m_pCustomizeBtn->m_lstInvisibleButtons.IsEmpty ())
			{
				fakeSizeCustButton = CSize (0,0);
			}

			if ((bHorz && rectButton.right > xRight - fakeSizeCustButton.cx) ||
				(!bHorz && rectButton.bottom > rectClient.bottom - fakeSizeCustButton.cy))
			{
				bVisible = FALSE;
				iOffset = iOffsetPrev;
				
				m_pCustomizeBtn->m_lstInvisibleButtons.AddTail (pButton);
			}
		}

		pButton->Show (bVisible);
		pButton->SetRect (rectButton);

		if (bVisible)
		{
			bPrevWasSeparator = (pButton->m_nStyle & TBBS_SEPARATOR);
		}

		if ((pButton->m_bWrap || pos == NULL) && bHorz)
		{
			//-----------------------
			// Center buttons in row:
			//-----------------------
			int nShift = (xRight - nRowActualWidth - iStartOffset) / 2;
			if (IsFloating () && nShift > 0 && m_bTextLabels)
			{
				for (POSITION posRow = posSave; posRow != NULL;)
				{
					BOOL bThis = (posRow == posSave);

					CBCGPToolbarButton* pButtonRow = (CBCGPToolbarButton*) m_Buttons.GetPrev (posRow);
					ASSERT (pButtonRow != NULL);

					if (pButtonRow->m_bWrap && !bThis)
					{
						break;
					}

					CRect rect = pButtonRow->Rect ();
					rect.OffsetRect (nShift, 0);
					pButtonRow->SetRect (rect);
				}
			}

			iOffset = iStartOffset;
			nRowActualWidth = 0;
			y += sizeGrid.cy + LINE_OFFSET;
		}
	}

	if (m_pCustomizeBtn != NULL)
	{
		CRect rectButton = rectClient;

		if ((int) m_pCustomizeBtn->m_uiCustomizeCmdId <= 0 &&
			m_pCustomizeBtn->m_lstInvisibleButtons.IsEmpty () || IsFloating () || bHideChevronInCustomizeMode)
		{
			// Hide customize button:
			m_pCustomizeBtn->SetRect (CRect (0, 0, 0, 0));
			m_pCustomizeBtn->Show (FALSE);
		}
		else
		{
			if (bHorz)
			{
				rectButton.right = xRight - 1;
				rectButton.left = rectButton.right - sizeCustButton.cx + 1;
			}
			else
			{
				rectButton.bottom --;
				rectButton.top = rectButton.bottom - sizeCustButton.cy;
			}

			m_pCustomizeBtn->SetRect (rectButton);
			m_pCustomizeBtn->Show (TRUE);
		}
	}

	dc.SelectObject (pOldFont);
	UpdateTooltips ();
	RedrawCustomizeButton ();
}
//**********************************************************************************
void CBCGPToolBar::AddRemoveSeparator (const CBCGPToolbarButton* pButton,
						const CPoint& ptStart, const CPoint& ptDrop)
{
	ASSERT_VALID (pButton);
	
	int iIndex = ButtonToIndex (pButton);
	if (iIndex <= 0)
	{
		return;
	}

	BOOL bHorz = (GetCurrentAlignment () & CBRS_ORIENT_HORZ) != 0;
	int iDelta = (bHorz) ? ptDrop.x - ptStart.x : ptDrop.y - ptStart.y;

	if (abs (iDelta) < STRETCH_DELTA)
	{
		// Ignore small move....
		return;
	}

	if (iDelta > 0)	// Add a separator left of button
	{
		const CBCGPToolbarButton* pLeftButton = GetButton (iIndex - 1);
		ASSERT_VALID (pLeftButton);

		if (pLeftButton->m_nStyle & TBBS_SEPARATOR)
		{
			// Already have separator, do nothing...
			return;
		}

		InsertSeparator (iIndex);
	}
	else	// Remove a separator in the left side
	{
		const CBCGPToolbarButton* pLeftButton = GetButton (iIndex - 1);
		ASSERT_VALID (pLeftButton);

		if ((pLeftButton->m_nStyle & TBBS_SEPARATOR) == 0)
		{
			// Not a separator, do nothing...
			return;
		}

		if (pLeftButton->IsVisible ())
		{
			RemoveButton (iIndex - 1);
		}
	}

	AdjustLayout ();

	m_iSelected = -1;

	RedrawWindow (NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
}
//***************************************************************************************
void CBCGPToolBar::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	int iButton = HitTest(point);
	if (iButton >= 0)
	{
		CBCGPToolbarButton* pButton = GetButton (iButton);
		if (pButton == NULL)
		{
			ASSERT (FALSE);
			return;
		}

		BOOL bIsSysMenu = pButton->IsKindOf (RUNTIME_CLASS (CBCGPToolbarSystemMenuButton));
		pButton->OnDblClick (this);

		if (!bIsSysMenu)
		{
			OnLButtonDown (nFlags, point);
		}

		//----------------------------------------------------------
		// Don't permit dock/undock when user double clicks on item!
		//----------------------------------------------------------
	}
	else
	{
		if (IsDocked())
		{
			CBCGPBaseToolBar::OnLButtonDblClk(nFlags, point);
		}
		else
		{
			CBCGPMiniFrameWnd* pMiniFrame = GetParentMiniFrame();
			if (pMiniFrame != NULL)
			{
				ASSERT_VALID(pMiniFrame);
				pMiniFrame->OnDockToRecentPos();
			}
		}
	}
}
//***************************************************************************************
void CBCGPToolBar::DrawSeparator (CDC* pDC, const CRect& rect, BOOL bHorz)
{
	CBCGPVisualManager::GetInstance ()->OnDrawSeparator (pDC, this, rect, bHorz);
}
//***************************************************************************************
CBCGPToolbarButton* CBCGPToolBar::CreateDroppedButton (COleDataObject* pDataObject)
{
	CBCGPToolbarButton* pButton = CBCGPToolbarButton::CreateFromOleData (pDataObject);
	ASSERT (pButton != NULL);

	//---------------------------
	// Remove accelerator string:
	//---------------------------
	int iOffset = pButton->m_strText.Find (_T('\t'));
	if (iOffset >= 0)
	{
		pButton->m_strText = pButton->m_strText.Left (iOffset);
	}

	if (pButton->m_bDragFromCollection)
	{
		pButton->m_bText = FALSE;
		pButton->m_bImage = TRUE;
	}

	if (pButton->m_bDragFromCollection && 
		pButton->GetImage () == -1 &&
		pButton->m_strText.IsEmpty ())
	{
		//----------------------------------------------
		// User-defined button by default have no image
		// and text and empty. To avoid the empty button
		// appearance, ask user about it's properties:
		//----------------------------------------------
		CBCGPLocalResource locaRes;
		CButtonAppearanceDlg dlg (pButton, m_pUserImages, this, 0, IsPureMenuButton (pButton));

		if (dlg.DoModal () != IDOK)
		{
			delete pButton;
			return NULL;
		}
	}

	if (pButton->GetImage () < 0)
	{
		pButton->m_bText = TRUE;
		pButton->m_bImage = FALSE;
	}

	return pButton;
}
//****************************************************************************************
CBCGPToolbarButton* CBCGPToolBar::GetHighlightedButton () const
{
	if (m_iHighlighted < 0)
	{
		return NULL;
	}
	else
	{
		return GetButton (m_iHighlighted);
	}
}
//****************************************************************************************
void CBCGPToolBar::RebuildAccelerationKeys ()
{
	m_AcellKeys.RemoveAll ();

	for (POSITION pos = m_Buttons.GetHeadPosition (); pos != NULL;)
	{
		CBCGPToolbarButton* pButton = (CBCGPToolbarButton*) m_Buttons.GetNext (pos);
		if (pButton == NULL)
		{
			break;
		}

		ASSERT_VALID (pButton);

		if ((pButton->m_nStyle & TBBS_SEPARATOR) ||
			!pButton->m_bText)
		{
			continue;
		}

		int iAmpOffset = pButton->m_strText.Find (_T('&'));
		if (iAmpOffset >= 0 && iAmpOffset < pButton->m_strText.GetLength () - 1)
		{
			TCHAR szChar [2] = { pButton->m_strText.GetAt (iAmpOffset + 1), '\0' };
			CharUpper (szChar);
			UINT uiHotKey = (UINT) (szChar [0]);
			m_AcellKeys.SetAt (uiHotKey, pButton);
		}
	}
}
//****************************************************************************************
void CBCGPToolBar::OnCustomizeMode (BOOL bSet)
{
	m_iButtonCapture = -1;
	m_iHighlighted = -1;
	m_iSelected = -1;

	for (POSITION pos = m_Buttons.GetHeadPosition (); pos != NULL;)
	{
		CBCGPToolbarButton* pButton = (CBCGPToolbarButton*) m_Buttons.GetNext (pos);
		ASSERT (pButton != NULL);

		pButton->OnCancelMode ();

		if ((pButton->m_nStyle & TBBS_DISABLED) == 0)
		{
			pButton->EnableWindow (!bSet);
		}
	}
}
//****************************************************************************************
BOOL CBCGPToolBar::RestoreOriginalstate ()
{
	if (m_uiOriginalResID == 0)
	{
		return FALSE;
	}

	BOOL bRes = LoadToolBar (m_uiOriginalResID);

	AdjustLayout ();

	if (IsFloating ())
	{
		RecalcLayout ();
	}
	else if (m_pParentDockBar != NULL)
	{
		CSize sizeCurr = CalcFixedLayout (FALSE, IsHorizontal ());
		CRect rect;
		GetWindowRect (rect);

		if (rect.Size () != sizeCurr)
		{
			SetWindowPos (NULL, 0, 0, sizeCurr.cx, sizeCurr.cy, SWP_NOMOVE  | SWP_NOACTIVATE | SWP_NOZORDER);
			UpdateVirtualRect ();
		}
		m_pDockBarRow->ArrangeBars(this);
		BCGPGetParentFrame (this)->RecalcLayout ();
	}

	RedrawWindow (NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);

	return bRes;
}
//*****************************************************************************************
void CBCGPToolBar::ShowCommandMessageString (UINT uiCmdId)
{
	if (m_hookMouseHelp != NULL)
	{
		return;
	}

	if (uiCmdId == 0 || uiCmdId == (UINT) -1 || uiCmdId == BCGPCUSTOMIZE_INTERNAL_ID)
	{
		GetOwner()->SendMessage(WM_SETMESSAGESTRING, AFX_IDS_IDLEMESSAGE);
		return;
	}

	UINT uiTrackId = uiCmdId;
	if (IsSystemCommand (uiCmdId))
	{
		uiTrackId = ID_COMMAND_FROM_SC (uiCmdId);
		ASSERT (uiTrackId >= AFX_IDS_SCFIRST &&
				uiTrackId < AFX_IDS_SCFIRST + 31);
	}
	else if (uiCmdId >= AFX_IDM_FIRST_MDICHILD)
	{
		// all MDI Child windows map to the same help id
		uiTrackId = AFX_IDS_MDICHILD;
	}

	GetOwner()->SendMessage (WM_SETMESSAGESTRING, (WPARAM) uiTrackId);
}
//*****************************************************************************************
afx_msg LRESULT CBCGPToolBar::OnMouseLeave(WPARAM,LPARAM)
{
	if (m_hookMouseHelp != NULL || 
		(m_bMenuMode && !IsCustomizeMode () && GetDroppedDownMenu () != NULL))
	{
		return 0;
	}

	m_bTracked = FALSE;
	m_ptLastMouse = CPoint (-1, -1);

	CWnd* pFocusWnd = GetFocus ();

	BOOL bFocusHere = (pFocusWnd == this);

	CWnd* pImmediateParent = GetParent ();
	if (pImmediateParent != NULL && 
		pImmediateParent->IsKindOf (RUNTIME_CLASS (CBCGPTabWnd)))
	{
		bFocusHere = (pImmediateParent == pFocusWnd);

		if (!bFocusHere)
		{
			bFocusHere = (pImmediateParent->GetParent () == pFocusWnd);
		}
	}

	if (m_iHighlighted < 0)
	{
		ShowCommandMessageString ((UINT) -1);
	}
	else if (!bFocusHere && !AlwaysSaveSelection ())
	{
		int iButton = m_iHighlighted;
		m_iHighlighted = -1;

		OnChangeHot (m_iHighlighted);

		CBCGPToolbarButton* pButton = InvalidateButton (iButton);
		UpdateWindow(); // immediate feedback

		if (pButton == NULL || !pButton->IsDroppedDown ())
		{
			GetOwner()->SendMessage(WM_SETMESSAGESTRING, AFX_IDS_IDLEMESSAGE);
		}
	}

	return 0;
}
//*****************************************************************************************
BOOL CBCGPToolBar::CanBeRestored () const
{
	return (m_uiOriginalResID != 0);
}
//*****************************************************************************************
BOOL CBCGPToolBar::IsLastCommandFromButton (CBCGPToolbarButton* pButton)
{
	ASSERT_VALID (pButton);

	HWND hwnd = pButton->GetHwnd ();

	if (!::IsWindow(hwnd))
	{
		return FALSE;
	}

	const MSG* pMsg = CWnd::GetCurrentMessage();
	if (pMsg == NULL)
	{
		return FALSE;
	}

	return (hwnd == (HWND) pMsg->lParam || hwnd == pMsg->hwnd);
}
//*****************************************************************************************
BOOL CBCGPToolBar::AddToolBarForImageCollection (UINT uiResID, UINT uiBmpResID/*= 0*/,
												UINT uiColdResID/*= 0*/, UINT uiMenuResID/*= 0*/,
												UINT uiDisabledResID/*= 0*/, UINT uiMenuDisabledResID/*= 0*/)
{
	CBCGPToolBar tlbTmp;
	return tlbTmp.LoadToolBar (uiResID, uiColdResID, uiMenuResID, FALSE, 
								uiDisabledResID, uiMenuDisabledResID, uiBmpResID);
}
//*****************************************************************************************
void CBCGPToolBar::SetHotTextColor (COLORREF clrText)
{
	m_clrTextHot = clrText;
}
//*****************************************************************************************
COLORREF CBCGPToolBar::GetHotTextColor ()
{
	return m_clrTextHot == (COLORREF) -1 ?
		globalData.clrBtnText : m_clrTextHot;
}
//*****************************************************************************************
void CBCGPToolBar::OnBcgbarresToolbarReset() 
{
	ASSERT (IsCustomizeMode () && !m_bLocked);
	ASSERT (m_iSelected >= 0);

	CBCGPToolbarButton* pButton = GetButton(m_iSelected);
	if (pButton == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	ASSERT(!(pButton->m_nStyle & TBBS_SEPARATOR));

	if (g_pUserToolsManager == NULL ||
		g_pUserToolsManager->FindTool (pButton->m_nID) == NULL)
	{
		int iImage;
		if (m_DefaultImages.Lookup (pButton->m_nID, iImage))
		{
			pButton->m_bUserButton = FALSE;
			pButton->SetImage (iImage);
			pButton->m_bImage = TRUE;
		}
		else
		{
			pButton->m_bImage = FALSE;
		}
	}

	pButton->m_bText = m_bMenuMode || !pButton->m_bImage;

	//----------------------
	// Restore default text:
	//----------------------
	OnSetDefaultButtonText (pButton);

	AdjustLayout ();
	BCGPCMD_MGR.ClearCmdImage (pButton->m_nID);

	RedrawWindow (NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
}
//***************************************************************************************
afx_msg LRESULT CBCGPToolBar::OnHelpHitTest(WPARAM wParam, LPARAM lParam)
{
	OnCancelMode ();

	int nIndex = HitTest ((DWORD) lParam);
	if (nIndex < 0)	// Click into the empty space or separator,
	{				// don't show HELP
		MessageBeep ((UINT) -1);
		return -1;
	}

	CBCGPToolbarButton* pButton = GetButton (nIndex);
	if (pButton == NULL)
	{
		ASSERT (FALSE);
		return -1;
	}

	if (pButton->OnContextHelp (this))
	{
		return -1;	// Continue help mode
	}

	LRESULT lres = CBCGPBaseControlBar::OnHelpHitTest (wParam, lParam);

	if (IsSystemCommand (pButton->m_nID))
	{
		lres = HID_BASE_COMMAND+ID_COMMAND_FROM_SC (pButton->m_nID);
	}

	CBCGPToolbarMenuButtonsButton* pSysButton =
		DYNAMIC_DOWNCAST (CBCGPToolbarMenuButtonsButton, pButton);
	if (pSysButton != NULL)
	{
		lres = HID_BASE_COMMAND+ID_COMMAND_FROM_SC (pSysButton->m_uiSystemCommand);
	}

	if (lres > 0)
	{
		SetHelpMode (FALSE);
	}

	return lres;
}
//****************************************************************************************
LRESULT CALLBACK CBCGPToolBar::BCGToolBarMouseProc (int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode != HC_ACTION)
	{
		return CallNextHookEx (m_hookMouseHelp, nCode, wParam, lParam);
	}

	MOUSEHOOKSTRUCT* lpMS = (MOUSEHOOKSTRUCT*) lParam;
	ASSERT (lpMS != NULL);

	if (wParam == WM_MOUSEMOVE)
	{
		//------------------------------------------
		// Find a toolbar matched to the mouse hook:
		//------------------------------------------
		CBCGPToolBar* pWndToolBar = 
			DYNAMIC_DOWNCAST (CBCGPToolBar, CWnd::WindowFromPoint (lpMS->pt));
		if (pWndToolBar != NULL)
		{
			CPoint ptClient = lpMS->pt;
			pWndToolBar->ScreenToClient (&ptClient);
			pWndToolBar->OnMouseMove (0, ptClient);
		}

		if (m_pLastHookedToolbar != NULL &&
			m_pLastHookedToolbar != pWndToolBar)
		{
			m_pLastHookedToolbar->m_bTracked = FALSE;
			m_pLastHookedToolbar->m_ptLastMouse = CPoint (-1, -1);

			if (m_pLastHookedToolbar->m_iHighlighted >= 0)
			{
				int iButton = m_pLastHookedToolbar->m_iHighlighted;
				m_pLastHookedToolbar->m_iHighlighted = -1;

				CBCGPPopupMenu* pPopupMenu = pWndToolBar == NULL ? 
					NULL : 
					DYNAMIC_DOWNCAST (CBCGPPopupMenu, pWndToolBar->GetParent ());

				if (pPopupMenu == NULL || 
					pPopupMenu->GetParentToolBar () != m_pLastHookedToolbar)
				{
					m_pLastHookedToolbar->OnChangeHot (m_pLastHookedToolbar->m_iHighlighted);

					m_pLastHookedToolbar->InvalidateButton (iButton);
					m_pLastHookedToolbar->UpdateWindow(); // immediate feedback
				}
			}
		}

		m_pLastHookedToolbar = pWndToolBar;
	}

	return 0;
}
//***************************************************************************************
void CBCGPToolBar::SetHelpMode (BOOL bOn)
{
	if (bOn)
	{
		if (m_hookMouseHelp == NULL)	// Not installed yet, set it now!
		{
			m_hookMouseHelp = ::SetWindowsHookEx (WH_MOUSE, BCGToolBarMouseProc, 
				0, GetCurrentThreadId ());
			if (m_hookMouseHelp == NULL)
			{
				TRACE (_T("CBCGPToolBar: Can't set mouse hook!\n"));
			}
		}
	}
	else if (m_hookMouseHelp != NULL)
	{
		::UnhookWindowsHookEx (m_hookMouseHelp);
		m_hookMouseHelp = NULL;

		m_pLastHookedToolbar = NULL;

		for (POSITION posTlb = gAllToolbars.GetHeadPosition (); posTlb != NULL;)
		{
			POSITION posSave = posTlb;

			CBCGPToolBar* pToolBar = (CBCGPToolBar*) gAllToolbars.GetNext (posTlb);
			ASSERT (pToolBar != NULL);

			if (CWnd::FromHandlePermanent (pToolBar->m_hWnd) != NULL)
			{
				ASSERT_VALID (pToolBar);
				pToolBar->OnCancelMode ();
			}

			posTlb = posSave;
			gAllToolbars.GetNext (posTlb);
		}
	}
}
//***************************************************************************************
void CBCGPToolBar::SetNonPermittedCommands (CList<UINT, UINT>& lstCommands)
{
	m_lstUnpermittedCommands.RemoveAll ();
	m_lstUnpermittedCommands.AddTail (&lstCommands);
}
//***************************************************************************************
void CBCGPToolBar::SetBasicCommands (CList<UINT, UINT>& lstCommands)
{
	m_lstBasicCommands.RemoveAll ();
	m_lstBasicCommands.AddTail (&lstCommands);
}
//***************************************************************************************
void CBCGPToolBar::AddBasicCommand (UINT uiCmd)
{
	if (m_lstBasicCommands.Find (uiCmd) == NULL)
	{
		m_lstBasicCommands.AddTail (uiCmd);
	}
}
//***************************************************************************************
void CBCGPToolBar::OnNcCalcSize(BOOL /*bCalcValidRects*/, NCCALCSIZE_PARAMS FAR* lpncsp) 
{
	m_Impl.CalcNcSize (lpncsp);
}
//***************************************************************************************
void CBCGPToolBar::OnNcPaint() 
{
	m_Impl.DrawNcArea ();
}
//****************************************************************************************
BCGNcHitTestType CBCGPToolBar::OnNcHitTest(CPoint /*point*/) 
{
	return HTCLIENT;
}
//***************************************************************************************
void CBCGPToolBar::AdjustLayout ()
{
	if (GetSafeHwnd () == NULL)
	{
		return;
	}

	BOOL bHorz = GetCurrentAlignment () & CBRS_ORIENT_HORZ ? TRUE : FALSE;

	for (POSITION pos = m_Buttons.GetHeadPosition (); pos != NULL;)
	{
		CBCGPToolbarButton* pButton = (CBCGPToolbarButton*) m_Buttons.GetNext (pos);
		if (pButton == NULL)
		{
			break;
		}

		ASSERT_VALID (pButton);

		pButton->m_bTextBelow = ((pButton->m_nStyle & TBBS_SEPARATOR) == 0) &&
								m_bTextLabels && bHorz;
	}	

	CBCGPReBar* pBar = DYNAMIC_DOWNCAST (CBCGPReBar, GetParent ());
	if (pBar != NULL)
	{
		CReBarCtrl& wndReBar = pBar->GetReBarCtrl ();
		UINT uiReBarsCount = wndReBar.GetBandCount ();

		REBARBANDINFO bandInfo;
		bandInfo.cbSize = globalData.GetRebarBandInfoSize ();
		bandInfo.fMask = (RBBIM_CHILDSIZE | RBBIM_CHILD | RBBIM_IDEALSIZE);

		UINT uiBand = 0;
		for (uiBand = 0; uiBand < uiReBarsCount; uiBand ++)
		{
			wndReBar.GetBandInfo (uiBand, &bandInfo);
			if (bandInfo.hwndChild == GetSafeHwnd ())
			{
				break;
			}
		}

		bandInfo.fMask ^= RBBIM_CHILD;

		if (uiBand >= uiReBarsCount)
		{
			ASSERT (FALSE);
		}
		else
		{
			CSize size = CBCGPBaseToolBar::CalcFixedLayout (FALSE, TRUE);

			m_nMaxBtnHeight = CalcMaxButtonHeight ();
			CSize sizeMin = CalcSize (FALSE);

			CRect rect; rect.SetRectEmpty();
			CalcInsideRect (rect, TRUE);
			sizeMin.cy -= rect.Height();
			sizeMin.cx -= rect.Width();

			sizeMin.cx = max(sizeMin.cx, size.cx);
			sizeMin.cy = max(sizeMin.cy, size.cy);

			bandInfo.cxMinChild = m_sizeButton.cx;
			bandInfo.cyMinChild = sizeMin.cy;

			bandInfo.cxIdeal = sizeMin.cx;

			wndReBar.SetBandInfo (uiBand, &bandInfo);
		}
	}
	else
	{
		AdjustSize ();
	}

	AdjustLocations ();

	RedrawWindow (NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
}
//****************************************************************************************
void CBCGPToolBar::OnBcgbarresCopyImage() 
{
	ASSERT (m_iSelected >= 0);

	CBCGPToolbarButton* pButton = GetButton(m_iSelected);
	if (pButton == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	ASSERT(!(pButton->m_nStyle & TBBS_SEPARATOR));
	ASSERT (pButton->GetImage () >= 0);

	//-----------------------------
	// Is this button "user tool"?
	//-----------------------------
	CBCGPUserTool* pUserTool = NULL;
	if (g_pUserToolsManager != NULL && !pButton->m_bUserButton)
	{
		pUserTool = g_pUserToolsManager->FindTool (pButton->m_nID);
		if (pUserTool != NULL)
		{
			pUserTool->CopyIconToClipboard ();
			return;
		}
	}

	CBCGPToolBarImages* pImages = (pButton->m_bUserButton) ? 
			m_pUserImages : &m_Images;
	ASSERT (pImages != NULL);

	CWaitCursor wait;
	pImages->CopyImageToClipboard (pButton->GetImage ());
}
//****************************************************************************************
BOOL CBCGPToolBar::OnSetDefaultButtonText (CBCGPToolbarButton* pButton)
{
	ASSERT_VALID (pButton);

	if (pButton->m_nID == 0 || pButton->m_nID == (UINT) -1)
	{
		return FALSE;
	}

	TCHAR szFullText [256];
	CString strTipText;

	if (AfxLoadString (pButton->m_nID, szFullText) &&
		AfxExtractSubString (strTipText, szFullText, 1, '\n'))
	{
		pButton->m_strText = strTipText;
		return TRUE;
	}

	return FALSE;
}
//****************************************************************************************
void CBCGPToolBar::SetMenuSizes (SIZE sizeButton, SIZE sizeImage)
{
	ASSERT(sizeButton.cx > 0 && sizeButton.cy > 0);

	//-----------------------------------------------------------------
	// Button must be big enough to hold image + 3 pixels on each side:
	//-----------------------------------------------------------------
	ASSERT(sizeButton.cx >= sizeImage.cx + 6);
	ASSERT(sizeButton.cy >= sizeImage.cy + 6);

	m_sizeMenuButton = sizeButton;
	m_sizeMenuImage = sizeImage;

	m_MenuImages.SetImageSize (m_sizeMenuImage);
	m_DisabledMenuImages.SetImageSize (m_sizeMenuImage);
}
//****************************************************************************************
CSize CBCGPToolBar::GetMenuImageSize ()
{
	CSize size = (m_sizeMenuImage.cx == -1) ? m_sizeImage : m_sizeMenuImage;

	if (globalData.GetRibbonImageScale () != 1.)
	{
		size = CSize (
			(int)(.5 + size.cx * globalData.GetRibbonImageScale ()),
			(int)(.5 + size.cy * globalData.GetRibbonImageScale ()));
	}

	return size;
}
//****************************************************************************************
CSize CBCGPToolBar::GetMenuButtonSize ()
{
	if (m_sizeMenuButton.cx == -1)
	{
		return m_sizeButton;
	}
	else
	{
		return m_sizeMenuButton;
	}
}
//****************************************************************************************
BOOL CBCGPToolBar::EnableContextMenuItems (CBCGPToolbarButton* pButton, CMenu* pPopup)
{
	ASSERT_VALID (pButton);
	ASSERT_VALID (pPopup);

	BOOL bHorz = m_dwStyle & CBRS_ORIENT_HORZ ? TRUE : FALSE;

	if (!pButton->OnCustomizeMenu (pPopup))
	{
		if (!pButton->m_bImage || pButton->GetImage () < 0)
		{
			pPopup->EnableMenuItem (ID_BCGBARRES_COPY_IMAGE, MF_BYCOMMAND | MF_GRAYED);
		}

		if (pButton->m_nID == (UINT) -1 || pButton->m_nID == 0)
		{
			pPopup->EnableMenuItem (ID_BCGBARRES_TOOLBAR_RESET, MF_BYCOMMAND | MF_GRAYED);
		}

		if (pButton->m_bText || (pButton->m_bTextBelow && bHorz))
		{
			if (pButton->m_bImage)
			{
				pPopup->CheckMenuItem (ID_BCGBARRES_TOOLBAR_IMAGE_AND_TEXT, MF_CHECKED  | MF_BYCOMMAND);
			}
			else
			{
				pPopup->CheckMenuItem (ID_BCGBARRES_TOOLBAR_TEXT, MF_CHECKED  | MF_BYCOMMAND);
			}
		}
		else
		{
			ASSERT (pButton->m_bImage);
			pPopup->CheckMenuItem (ID_BCGBARRES_TOOLBAR_IMAGE, MF_CHECKED | MF_BYCOMMAND);
		}

		if (pButton->m_bTextBelow && bHorz)
		{
			//------------------------
			// Text is always visible!
			//------------------------
			pPopup->EnableMenuItem (ID_BCGBARRES_TOOLBAR_IMAGE, MF_BYCOMMAND | MF_GRAYED);
		}

		if (IsPureMenuButton (pButton))
		{
			//--------------------------
			// Disable text/image items:
			//--------------------------
			pPopup->EnableMenuItem (ID_BCGBARRES_TOOLBAR_IMAGE, MF_GRAYED | MF_BYCOMMAND);
			pPopup->EnableMenuItem (ID_BCGBARRES_TOOLBAR_TEXT, MF_GRAYED | MF_BYCOMMAND);
			pPopup->EnableMenuItem (ID_BCGBARRES_TOOLBAR_IMAGE_AND_TEXT, MF_GRAYED | MF_BYCOMMAND);

			pButton->m_bText = TRUE;
		}
	}

	//---------------------------
	// Adjust "Start group" item:
	//---------------------------
	CBCGPToolbarButton* pPrevButton = NULL;
	for (POSITION pos = m_Buttons.GetHeadPosition (); pos != NULL;)
	{
		CBCGPToolbarButton* pCurrButton = (CBCGPToolbarButton*) m_Buttons.GetNext (pos);
		ASSERT_VALID (pCurrButton);

		if (pCurrButton == pButton)
		{
			if (pPrevButton == NULL)	// First button
			{
				pPopup->EnableMenuItem (ID_BCGBARRES_TOOLBAR_START_GROUP, MF_BYCOMMAND | MF_GRAYED);
			}
			else if (pPrevButton->m_nStyle & TBBS_SEPARATOR)
			{
				pPopup->CheckMenuItem (ID_BCGBARRES_TOOLBAR_START_GROUP, MF_CHECKED  | MF_BYCOMMAND);
			}

			break;
		}

		pPrevButton = pCurrButton;
	}

	return TRUE;
}
//***************************************************************************************
void CBCGPToolBar::OnChangeHot (int iHot)
{
	if (m_iHot == iHot && m_iHot >= 0)
	{
		iHot = -1;
	}

	m_iHot = iHot;
		
	CBCGPToolbarMenuButton* pCurrPopupMenu = GetDroppedDownMenu ();
	if (pCurrPopupMenu == NULL && !CBCGPToolBar::IsCustomizeMode ())
	{
		CBCGPMenuBar* pMenuBar = DYNAMIC_DOWNCAST (CBCGPMenuBar, this);
		if (pMenuBar != NULL && globalData.IsAccessibilitySupport ())
		{
			if (GetFocus () == this)
            {
				int nChildID = AccGetChildIdByButtonIndex(m_iHot);
				if (nChildID > 0)
				{
					globalData.NotifyWinEvent (EVENT_OBJECT_FOCUS, GetSafeHwnd(), OBJID_CLIENT , nChildID);
				}
            }

		}
		return;
	}

	if (pCurrPopupMenu != NULL && pCurrPopupMenu->IsExclusive ())
	{
		return;
	}

	if (iHot < 0 || iHot >= m_Buttons.GetCount())
	{
		m_iHot = -1;
		if (pCurrPopupMenu != NULL && CBCGPToolBar::IsCustomizeMode () &&
			!m_bAltCustomizeMode)
		{
			pCurrPopupMenu->OnCancelMode ();
		}

		return;
	}

	CBCGPToolbarMenuButton* pMenuButton =
		DYNAMIC_DOWNCAST (CBCGPToolbarMenuButton, GetButton (iHot));

	if (pMenuButton != pCurrPopupMenu)
	{
		BOOL bDisableMenuAnimation = FALSE;
		CBCGPPopupMenu::ANIMATION_TYPE animType = CBCGPPopupMenu::GetAnimationType ();

		if (pCurrPopupMenu != NULL)
		{
			int iHighlighted = m_iHighlighted;

			if (!CBCGPToolBar::IsCustomizeMode ())
			{
				m_iHighlighted = -1;
			}

			pCurrPopupMenu->OnCancelMode ();

			m_iHighlighted = iHighlighted;
			bDisableMenuAnimation = TRUE;
		}

		if (pMenuButton != NULL &&
			(!CBCGPToolBar::IsCustomizeMode () || 
			!pMenuButton->IsKindOf (RUNTIME_CLASS (CBCGPToolbarSystemMenuButton))))
		{
			if (bDisableMenuAnimation)
			{
				CBCGPPopupMenu::SetAnimationType (
					CBCGPPopupMenu::NO_ANIMATION);
			}

			pMenuButton->OnClick (this);
			CBCGPPopupMenu::SetAnimationType (animType);
		}
	}
	else
	{
		if (CBCGPToolBar::IsCustomizeMode () &&
			pCurrPopupMenu != NULL && pCurrPopupMenu->IsDroppedDown ())
		{
			pCurrPopupMenu->OnCancelMode ();
		}
	}

	if (IsCustomizeMode () && m_iDragIndex < 0)
	{
		int nSelected = m_iHighlighted;
		m_iSelected = m_iHot;

		if (nSelected != -1)
		{
			InvalidateButton (nSelected);
		}

		CBCGPToolbarButton* pSelButton = GetButton (m_iSelected);
		if (pSelButton == NULL)
		{
			ASSERT (FALSE);
			return;
		}

		if (pSelButton->m_nStyle & TBBS_SEPARATOR)
		{
			m_iSelected = -1;
		}
		else
		{
			InvalidateButton (m_iSelected);
		}
	}

	if (m_iHot >= 0 && m_iHot != m_iHighlighted)
	{

		CBCGPMenuBar* pMenuBar = DYNAMIC_DOWNCAST (CBCGPMenuBar, this);
		if (pMenuBar != NULL && globalData.IsAccessibilitySupport ())
		{
			if (m_iHighlighted == -1)
			{
				int nChildID = AccGetChildIdByButtonIndex(m_iHot);
				if (nChildID > 0)
				{
					globalData.NotifyWinEvent(EVENT_OBJECT_FOCUS, GetSafeHwnd(), OBJID_CLIENT, nChildID);
				}
			}
		}

		int iCurrHighlighted = m_iHighlighted;
		if (iCurrHighlighted >= 0)
		{
			InvalidateButton (iCurrHighlighted);
		}

		m_iHighlighted = m_iHot;

		InvalidateButton (m_iHighlighted);
		UpdateWindow ();
	}
}
//*****************************************************************************************
BOOL CBCGPToolBar::PrevMenu ()
{
	int iHot;
	CBCGPToolbarMenuButton* pCurrPopupMenu = GetDroppedDownMenu (&iHot);
	if (pCurrPopupMenu == NULL || pCurrPopupMenu->IsExclusive ())
	{
		return FALSE;
	}

	int iHotOriginal = iHot;
	int iTotalItems = GetCount ();

	while (--iHot != iHotOriginal)
	{
		if (iHot < 0)
		{
			iHot = iTotalItems - 1;
		}

		CBCGPToolbarButton* pButton = GetButton (iHot);
		if (DYNAMIC_DOWNCAST (CBCGPToolbarMenuButton, pButton) != NULL &&
			(pButton->m_nStyle & TBBS_DISABLED) == 0 &&
			!pButton->Rect ().IsRectEmpty ())
		{
			break;
		}
	}

	if (iHot == iHotOriginal)	// Only one menu item on the toolbar,
	{							// do nothing
		return TRUE;
	}

	//-------------------------------------------
	// Save animation type and disable animation:
	//-------------------------------------------
	CBCGPPopupMenu::ANIMATION_TYPE animType = CBCGPPopupMenu::GetAnimationType ();
	CBCGPPopupMenu::SetAnimationType (CBCGPPopupMenu::NO_ANIMATION);

	OnChangeHot (iHot);

	//-----------------------
	// Select the first item:
	//-----------------------
	if (m_iHot >= 0)
	{
		CBCGPToolbarMenuButton* pMenuButton =
			DYNAMIC_DOWNCAST (CBCGPToolbarMenuButton, GetButton (m_iHot));
		if (pMenuButton != NULL && pMenuButton->IsDroppedDown ())
		{
			pMenuButton->m_pPopupMenu->SendMessage (WM_KEYDOWN, VK_HOME);
		}
	}

	//-------------------
	// Restore animation:
	//-------------------
	CBCGPPopupMenu::SetAnimationType (animType);
	return TRUE;
}
//*****************************************************************************************
BOOL CBCGPToolBar::NextMenu ()
{
	int iHot;
	CBCGPToolbarMenuButton* pCurrPopupMenu = GetDroppedDownMenu (&iHot);
	if (pCurrPopupMenu == NULL || pCurrPopupMenu->IsExclusive ())
	{
		return FALSE;
	}

	int iHotOriginal = iHot;
	int iTotalItems = GetCount ();

	while (++iHot != iHotOriginal)
	{
		if (iHot >= iTotalItems)
		{
			iHot = 0;
		}

		CBCGPToolbarButton* pButton = GetButton (iHot);
		if (DYNAMIC_DOWNCAST (CBCGPToolbarMenuButton, pButton) != NULL &&
			(pButton->m_nStyle & TBBS_DISABLED) == 0 &&
			!pButton->Rect ().IsRectEmpty ())
		{
			break;
		}
	}

	if (iHot == iHotOriginal)	// Only one menu item on the toolbar,
	{							// do nothing
		return TRUE;
	}

	//-------------------------------------------
	// Save animation type and disable animation:
	//-------------------------------------------
	CBCGPPopupMenu::ANIMATION_TYPE animType = CBCGPPopupMenu::GetAnimationType ();
	CBCGPPopupMenu::SetAnimationType (CBCGPPopupMenu::NO_ANIMATION);

	OnChangeHot (iHot);

	//-----------------------
	// Select the first item:
	//-----------------------
	if (m_iHot >= 0)
	{
		CBCGPToolbarMenuButton* pMenuButton =
			DYNAMIC_DOWNCAST (CBCGPToolbarMenuButton, GetButton (m_iHot));
		if (pMenuButton != NULL && pMenuButton->IsDroppedDown ())
		{
			pMenuButton->m_pPopupMenu->SendMessage (WM_KEYDOWN, VK_HOME);
		}
	}

	//-------------------
	// Restore animation:
	//-------------------
	CBCGPPopupMenu::SetAnimationType (animType);
	return TRUE;
}
//*****************************************************************************************
BOOL CBCGPToolBar::SetHot (CBCGPToolbarButton *pMenuButton)
{
	CBCGPToolbarMenuButton* pCurrPopupMenu = GetDroppedDownMenu ();
	if (pCurrPopupMenu != NULL && pCurrPopupMenu->IsExclusive ())
	{
		return TRUE;
	}

	if (pMenuButton == NULL)
	{
		m_iHot = -1;
		return TRUE;
	}

	int i = 0;
	for (POSITION pos = m_Buttons.GetHeadPosition (); pos != NULL; i ++)
	{
		CBCGPToolbarButton* pButton = (CBCGPToolbarButton*) m_Buttons.GetNext (pos);
		ASSERT_VALID (pButton);

		if (pMenuButton == pButton)
		{
			if (m_iHot != i)
			{
				OnChangeHot (i);
			}
			return TRUE;
		}
	}

	return FALSE;
}
//**************************************************************************************
BOOL CBCGPToolBar::DropDownMenu (CBCGPToolbarButton* pButton)
{
	if (pButton == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	ASSERT_VALID (pButton);

	//----------------------------
	// Simulate menu button click:
	//----------------------------
	CBCGPToolbarMenuButton* pMenuButton =
		DYNAMIC_DOWNCAST (CBCGPToolbarMenuButton, pButton);
	if (pMenuButton == NULL || !pMenuButton->OnClick (this))
	{
		return FALSE;
	}

	//----------------------------
	// Select the first menu item:
	//----------------------------
	if (pMenuButton->IsDroppedDown ())
	{
		pMenuButton->m_pPopupMenu->SendMessage (WM_KEYDOWN, VK_HOME);
	}

	SetHot (pMenuButton);
	return TRUE;
}
//********************************************************************************************
BOOL CBCGPToolBar::ProcessCommand (CBCGPToolbarButton* pButton)
{
	ASSERT_VALID (pButton);

	if (pButton->m_nID == 0 ||
		pButton->m_nID == (UINT) -1)
	{
		return FALSE;
	}

	BCGPlaySystemSound (BCGSOUND_MENU_COMMAND);

	//-----------------------
	// Send command to owner:
	//-----------------------
	AddCommandUsage (pButton->m_nID);
	GetOwner()->PostMessage (WM_COMMAND, pButton->m_nID);

	return TRUE;
}
//********************************************************************************************
CBCGPToolbarMenuButton* CBCGPToolBar::GetDroppedDownMenu (int* pIndex) const
{
	if (m_Buttons.IsEmpty ())
	{
		return NULL;
	}

	int iIndex = 0;
	for (POSITION pos = m_Buttons.GetHeadPosition (); pos != NULL; iIndex ++)
	{
		CBCGPToolbarButton* pButton = (CBCGPToolbarButton*) m_Buttons.GetNext (pos);
		ASSERT_VALID (pButton);

		CBCGPToolbarMenuButton* pMenuButton =
			DYNAMIC_DOWNCAST (CBCGPToolbarMenuButton, pButton);

		if (pMenuButton != NULL && pMenuButton->IsDroppedDown ())
		{
			if (pIndex != NULL)
			{
				*pIndex = iIndex;
			}

			return pMenuButton;
		}
	}

	if (pIndex != NULL)
	{
		*pIndex = -1;
	}

	return NULL;
}
//******************************************************************
void CBCGPToolBar::Deactivate ()
{
	if (m_iHighlighted >= 0 && m_iHighlighted < m_Buttons.GetCount ())
	{
		int iButton = m_iHighlighted;
		m_iHighlighted = m_iHot = -1;

		InvalidateButton (iButton);
		UpdateWindow ();

		GetOwner()->SendMessage (WM_SETMESSAGESTRING, AFX_IDS_IDLEMESSAGE);
	}

	RestoreFocus ();
}
//*********************************************************************
BOOL CBCGPToolBar::SaveParameters (LPCTSTR lpszProfileName)
{
	CString strProfileName = ::BCGPGetRegPath (strToolbarProfile, lpszProfileName);

	BOOL bResult = FALSE;

	CString strSection;
	strSection.Format (REG_PARAMS_FMT, strProfileName);

	CBCGPRegistrySP regSP;
	CBCGPRegistry& reg = regSP.Create (FALSE, FALSE);

	if (reg.CreateKey (strSection))
	{
		bResult =	reg.Write (REG_ENTRY_TOOLTIPS, m_bShowTooltips) &&
					reg.Write (REG_ENTRY_KEYS, m_bShowShortcutKeys) &&
					reg.Write (REG_ENTRY_LARGE_ICONS, m_bLargeIcons) &&
					reg.Write (REG_ENTRY_ANIMATION, (int) CBCGPPopupMenu::GetAnimationType (TRUE)) &&
					reg.Write (REG_ENTRY_RU_MENUS, CBCGPMenuBar::m_bRecentlyUsedMenus) &&
					reg.Write (REG_ENTRY_MENU_SHADOWS, CBCGPMenuBar::m_bMenuShadows) &&
					reg.Write (REG_ENTRY_SHOW_ALL_MENUS_DELAY, CBCGPMenuBar::m_bShowAllMenusDelay) &&
					reg.Write (REG_ENTRY_LOOK2000, CBCGPVisualManager::GetInstance ()->IsLook2000 ()) &&
					reg.Write (REG_ENTRY_CMD_USAGE_COUNT, m_UsageCount);
	}

	return bResult;
}
//*****************************************************************************************
BOOL CBCGPToolBar::LoadParameters (LPCTSTR lpszProfileName)
{
	CString strProfileName = ::BCGPGetRegPath (strToolbarProfile, lpszProfileName);

	BOOL bResult = FALSE;

	CString strSection;
	strSection.Format (REG_PARAMS_FMT, strProfileName);

	CBCGPRegistrySP regSP;
	CBCGPRegistry& reg = regSP.Create (FALSE, TRUE);

	if (!reg.Open (strSection))
	{
		return FALSE;
	}

	BOOL bLook2000 = FALSE;
	int iAnimType = CBCGPPopupMenu::NO_ANIMATION;

	bResult =	reg.Read (REG_ENTRY_TOOLTIPS, m_bShowTooltips) &&
				reg.Read (REG_ENTRY_KEYS, m_bShowShortcutKeys) &&
				reg.Read (REG_ENTRY_LARGE_ICONS, m_bLargeIcons) &&
				reg.Read (REG_ENTRY_ANIMATION, iAnimType) &&
				reg.Read (REG_ENTRY_RU_MENUS, CBCGPMenuBar::m_bRecentlyUsedMenus) &&
				reg.Read (REG_ENTRY_MENU_SHADOWS, CBCGPMenuBar::m_bMenuShadows) &&
				reg.Read (REG_ENTRY_SHOW_ALL_MENUS_DELAY, CBCGPMenuBar::m_bShowAllMenusDelay) &&
				reg.Read (REG_ENTRY_LOOK2000, bLook2000) &&
				reg.Read (REG_ENTRY_CMD_USAGE_COUNT, m_UsageCount);

	CBCGPPopupMenu::SetAnimationType ((CBCGPPopupMenu::ANIMATION_TYPE) iAnimType);
	SetLargeIcons (m_bLargeIcons);

	CBCGPVisualManager::GetInstance ()->SetLook2000 (bLook2000);
	return bResult;
}
//**********************************************************************************
BOOL CBCGPToolBar::LoadLargeIconsState (LPCTSTR lpszProfileName)
{
	CString strProfileName = ::BCGPGetRegPath (strToolbarProfile, lpszProfileName);

	CString strSection;
	strSection.Format (REG_PARAMS_FMT, strProfileName);

	CBCGPRegistrySP regSP;
	CBCGPRegistry& reg = regSP.Create (FALSE, TRUE);

	if (!reg.Open (strSection))
	{
		return FALSE;
	}

	return reg.Read (REG_ENTRY_LARGE_ICONS, m_bLargeIcons);
}
//**********************************************************************************
void CBCGPToolBar::OnSetFocus(CWnd* pOldWnd) 
{
	CBCGPBaseToolBar::OnSetFocus(pOldWnd);

	if (m_bLeaveFocus && pOldWnd != NULL &&
		::IsWindow (pOldWnd->GetSafeHwnd ()) &&
		DYNAMIC_DOWNCAST (CBCGPToolBar, pOldWnd) == NULL &&
		DYNAMIC_DOWNCAST (CBCGPToolBar, pOldWnd->GetParent ()) == NULL &&
		DYNAMIC_DOWNCAST (CBCGPBaseTabWnd, GetParent ()) == NULL)
	{
		m_hwndLastFocus = pOldWnd->GetSafeHwnd ();
	}
}
//**********************************************************************************
void CBCGPToolBar::RestoreFocus ()
{
	if (::IsWindow (m_hwndLastFocus))
	{
		::SetFocus (m_hwndLastFocus);
	}

	m_hwndLastFocus = NULL;

	if (globalData.m_bUnderlineKeyboardShortcuts && !globalData.m_bSysUnderlineKeyboardShortcuts && !CBCGPToolBar::IsCustomizeMode ())
	{
		globalData.m_bUnderlineKeyboardShortcuts = FALSE;
		RedrawUnderlines ();
	}
}
//*********************************************************************************
void CBCGPToolBar::OnBcgbarresToolbarNewMenu() 
{
	CBCGPToolbarMenuButton* pMenuButton = new CBCGPToolbarMenuButton;
	pMenuButton->m_bText = TRUE;
	pMenuButton->m_bImage = FALSE;

	CBCGPLocalResource locaRes;
	CButtonAppearanceDlg dlg (pMenuButton, m_pUserImages, this, 0, IsPureMenuButton (pMenuButton));
	if (dlg.DoModal () != IDOK)
	{
		delete pMenuButton;
		return;
	}

	m_iSelected = InsertButton (pMenuButton, m_iSelected);

	AdjustLayout ();
	RedrawWindow (NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);

	pMenuButton->OnClick (this, FALSE);
}
//**********************************************************************************
void CBCGPToolBar::SetToolBarBtnText (UINT nBtnIndex,
									LPCTSTR szText,
									BOOL bShowText,
									BOOL bShowImage)
{
	CBCGPToolbarButton* pButton = GetButton (nBtnIndex);
	if (pButton == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	ASSERT(!(pButton->m_nStyle & TBBS_SEPARATOR));

	if (bShowText)
	{
		if (szText == NULL)
		{
			OnSetDefaultButtonText(pButton);
		}
		else
		{
			SetButtonText(nBtnIndex, szText);
		}
	}

	pButton->m_bText = bShowText;
	pButton->m_bImage = bShowImage;
}
//*************************************************************************************
void CBCGPToolBar::SetLargeIcons (BOOL bLargeIcons/* = TRUE*/)
{
	m_bLargeIcons = bLargeIcons;

	if (m_bLargeIcons)
	{
		m_sizeCurButton.cx = (int) (.5 + m_dblLargeImageRatio * m_sizeButton.cx);
		m_sizeCurButton.cy = (int) (.5 + m_dblLargeImageRatio * m_sizeButton.cy);

		m_sizeCurImage.cx = (int) (.5 + m_dblLargeImageRatio * m_sizeImage.cx);
		m_sizeCurImage.cy = (int) (.5 + m_dblLargeImageRatio * m_sizeImage.cy);
	}
	else
	{
		m_sizeCurButton = m_sizeButton;
		m_sizeCurImage = m_sizeImage;
	}

	for (POSITION posTlb = gAllToolbars.GetHeadPosition (); posTlb != NULL;)
	{
		CBCGPToolBar* pToolBar = (CBCGPToolBar*) gAllToolbars.GetNext (posTlb);
		ASSERT (pToolBar != NULL);

		if (CWnd::FromHandlePermanent (pToolBar->m_hWnd) != NULL)
		{
			ASSERT_VALID(pToolBar);

			BOOL	bNeedAdjustLayout	= FALSE;

			if (pToolBar->m_bLocked)
			{
				// Locked toolbars have its individual sizes
				if (m_bLargeIcons)
				{
					if (pToolBar->m_sizeCurButtonLocked.cx != (int) (.5 + m_dblLargeImageRatio * pToolBar->m_sizeButtonLocked.cx) ||
						pToolBar->m_sizeCurButtonLocked.cy != (int) (.5 + m_dblLargeImageRatio * pToolBar->m_sizeButtonLocked.cy) ||
						pToolBar->m_sizeCurImageLocked.cx != (int) (.5 + m_dblLargeImageRatio * pToolBar->m_sizeImageLocked.cx) ||
						pToolBar->m_sizeCurImageLocked.cy != (int) (.5 + m_dblLargeImageRatio * pToolBar->m_sizeImageLocked.cy))
					{
						pToolBar->m_sizeCurButtonLocked.cx = (int) (.5 + m_dblLargeImageRatio * pToolBar->m_sizeButtonLocked.cx);
						pToolBar->m_sizeCurButtonLocked.cy = (int) (.5 + m_dblLargeImageRatio * pToolBar->m_sizeButtonLocked.cy);

						bNeedAdjustLayout = TRUE;

						pToolBar->m_sizeCurImageLocked.cx = (int) (.5 + m_dblLargeImageRatio * pToolBar->m_sizeImageLocked.cx);
						pToolBar->m_sizeCurImageLocked.cy = (int) (.5 + m_dblLargeImageRatio * pToolBar->m_sizeImageLocked.cy);
					}
				}
				else
				{
					if (pToolBar->m_sizeCurButtonLocked != pToolBar->m_sizeButtonLocked ||
						pToolBar->m_sizeCurImageLocked != pToolBar->m_sizeImageLocked)
					{
						bNeedAdjustLayout = TRUE;

						pToolBar->m_sizeCurButtonLocked = pToolBar->m_sizeButtonLocked;
						pToolBar->m_sizeCurImageLocked = pToolBar->m_sizeImageLocked;
					}
				}
			}
			else
			{
				bNeedAdjustLayout = TRUE;
			}

			if (bNeedAdjustLayout)
			{
				pToolBar->AdjustLayout ();
				pToolBar->OnLargeIconsModeChanged();

				if (pToolBar->m_bLocked)
				{
					CBCGPBaseControlBar* pParentBar = 
						DYNAMIC_DOWNCAST (CBCGPBaseControlBar, pToolBar->GetParent ());
					if (pParentBar != NULL)
					{
						pParentBar->AdjustLayout ();
					}
				}
			}
		}
	}
}
//************************************************************************************
BOOL CBCGPToolBar::IsCommandRarelyUsed (UINT uiCmd)
{
	if (IsCustomizeMode () ||
		uiCmd == 0 || uiCmd == (UINT) -1 ||
		IsBCGPStandardCommand (uiCmd) ||
		m_lstBasicCommands.IsEmpty ())
	{
		return FALSE;
	}

	if ((uiCmd == ID_BCGBARRES_TASKPANE_BACK) ||
		(uiCmd == ID_BCGBARRES_TASKPANE_FORWARD) ||
		(uiCmd == ID_BCGBARRES_TASKPANE_OTHER))
	{
		return FALSE;
	}

	return !IsBasicCommand (uiCmd) &&
		!m_UsageCount.IsFreqeuntlyUsedCmd (uiCmd);
}
//************************************************************************************
BOOL CBCGPToolBar::SetCommandUsageOptions (UINT nStartCount, UINT nMinUsagePercentage)
{
	return m_UsageCount.SetOptions (nStartCount, nMinUsagePercentage);
}
//***********************************************************************************
void CBCGPToolBar::EnableLargeIcons (BOOL bEnable)
{
	ASSERT (GetSafeHwnd () == NULL);	// Should not be created yet!
	m_bLargeIconsAreEnbaled = bEnable;
}
//*************************************************************************************
void CBCGPToolBar::EnableCustomizeButton (BOOL bEnable, UINT uiCustomizeCmd, 
										 const CString& strCustomizeText,
										 BOOL bQuickCustomize)
{
	BOOL bWasChanged = FALSE;

	if (bEnable)
	{
		if (m_pCustomizeBtn != NULL)
		{
			ASSERT_VALID (m_pCustomizeBtn);

			m_pCustomizeBtn->m_uiCustomizeCmdId = uiCustomizeCmd;
			m_pCustomizeBtn->m_strText =  strCustomizeText;
		}
		else
		{
			if (InsertButton (CCustomizeButton (uiCustomizeCmd, strCustomizeText)) < 0)
			{
				ASSERT (FALSE);
				return;
			}

			m_pCustomizeBtn = DYNAMIC_DOWNCAST (CCustomizeButton, m_Buttons.GetTail ());
			ASSERT_VALID (m_pCustomizeBtn);

			bWasChanged = TRUE;
		}

		m_bQuickCustomize = bQuickCustomize;
	}
	else if (m_pCustomizeBtn != NULL)
	{
		ASSERT_VALID (m_pCustomizeBtn);
		ASSERT (m_pCustomizeBtn == m_Buttons.GetTail ());	// Should be last

		m_Buttons.RemoveTail ();
		delete m_pCustomizeBtn;
		m_pCustomizeBtn = NULL;

		bWasChanged = TRUE;
	}

	if (bWasChanged)
	{
		AdjustLayout ();
		
		if (IsFloating ())
		{
			CBCGPMiniFrameWnd* pParentMiniFrame = GetParentMiniFrame();
			if (pParentMiniFrame->GetSafeHwnd() != NULL && bWasChanged)
			{
				if (bEnable)
				{
					pParentMiniFrame->SetCaptionButtons(BCGP_CAPTION_BTN_CLOSE|BCGP_CAPTION_BTN_CUSTOMIZE);
				}
				else
				{
					pParentMiniFrame->SetCaptionButtons(BCGP_CAPTION_BTN_CLOSE);
				}
			}

			RecalcLayout ();
		}
		else if (m_pParentDockBar != NULL)
		{
			UpdateVirtualRect();

			if (m_pDockBarRow != NULL)
			{
				m_pDockBarRow->ArrangeBars(this);
			}

			BCGPGetParentFrame(this)->RecalcLayout ();
		}
	}
	
	RedrawWindow (NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
}
//*******************************************************************************************
void CBCGPToolBar::EnableCustomizeButton (BOOL bEnable, UINT uiCustomizeCmd, UINT uiCustomizeTextResId,
										  BOOL bQuickCustomize)
{
	CString strCustomizeText;
	strCustomizeText.LoadString (uiCustomizeTextResId);

	EnableCustomizeButton (bEnable, uiCustomizeCmd, strCustomizeText, bQuickCustomize);
}
//*******************************************************************************************
void CBCGPToolBar::OnWindowPosChanging(WINDOWPOS FAR* lpwndpos) 
{
	CBCGPBaseToolBar::OnWindowPosChanging(lpwndpos);
	
	CBCGPReBar* pBar = DYNAMIC_DOWNCAST (CBCGPReBar, GetParent ());
	if (pBar != NULL)
	{
		AdjustLayout ();
	}
}
//*******************************************************************************************
void CBCGPToolBar::EnableTextLabels (BOOL bEnable/* = TRUE*/)
{
	if (m_bMenuMode)
	{
		ASSERT (FALSE);
		return;
	}

	m_bTextLabels = bEnable;
	AdjustLayout ();
}
//****************************************************************************************
int CBCGPToolBar::CalcMaxButtonHeight ()
{
	ASSERT_VALID (this);

	BOOL bHorz = GetCurrentAlignment () & CBRS_ORIENT_HORZ ? TRUE : FALSE;
	m_bDrawTextLabels = FALSE;

	if (!m_bTextLabels || !bHorz)
	{
		return 0;
	}

	int nMaxBtnHeight = 0;
	CClientDC dc (this);

	CFont* pOldFont = SelectDefaultFont (&dc);
	ASSERT (pOldFont != NULL);

	//-----------------------------------------------------------------------
	// To better look, I'm assuming that all rows shoud be of the same height.
	// Calculate max. button height:
	//-----------------------------------------------------------------------
	for (POSITION pos = m_Buttons.GetHeadPosition (); pos != NULL;)
	{
		CBCGPToolbarButton* pButton = (CBCGPToolbarButton*) m_Buttons.GetNext (pos);
		ASSERT (pButton != NULL);
		
		if (pButton->m_bTextBelow)
		{
			if (pButton->m_strText.IsEmpty ())
			{
				OnSetDefaultButtonText (pButton);
			}

			CSize sizeButton = pButton->OnCalculateSize (&dc, 
				GetButtonSize (), bHorz);

			nMaxBtnHeight = max (nMaxBtnHeight, sizeButton.cy);
		}
	}

	m_bDrawTextLabels = (nMaxBtnHeight > GetButtonSize ().cy);
	dc.SelectObject (pOldFont);
	return nMaxBtnHeight;
}
//***************************************************************************************
void CBCGPToolBar::ResetAllImages()
{
	m_Images.Clear();
	m_ColdImages.Clear();
	m_DisabledImages.Clear();
	m_MenuImages.Clear();
	m_DisabledMenuImages.Clear();
	m_LargeImages.Clear();
	m_LargeColdImages.Clear();
	m_LargeDisabledImages.Clear();

	m_sizeImage	= CSize (16, 15);
}
//**********************************************************************************
void CBCGPToolBar::ResetImages ()
//
// Reset all toolbar images exept user-derfined to the default
//
{
	ASSERT_VALID (this);

	if (m_bLocked)
	{
		return;
	}

	for (POSITION pos = m_Buttons.GetHeadPosition (); pos != NULL;)
	{
		CBCGPToolbarButton* pButton = (CBCGPToolbarButton*) m_Buttons.GetNext (pos);
		ASSERT_VALID (pButton);

		pButton->ResetImageToDefault ();
	}

	if (IsFloating ())
	{
		AdjustLayout ();
	}
}
//**********************************************************************************
BOOL CBCGPToolBar::OnEraseBkgnd(CDC* /*pDC*/) 
{
	return TRUE;
}
//**********************************************************************************
BOOL CBCGPToolBar::OnUserToolTip (CBCGPToolbarButton* pButton, CString& strTTText) const
{
	ASSERT_VALID (pButton);

	CFrameWnd* pTopFrame = BCGPGetParentFrame (this);
	if (pTopFrame == NULL)
	{
		return FALSE;
	}

	CBCGPDropDownFrame* pDropFrame = DYNAMIC_DOWNCAST (CBCGPDropDownFrame, pTopFrame);
	if (pDropFrame != NULL)
	{
		pTopFrame = BCGPGetParentFrame (pDropFrame);
		if (pTopFrame == NULL)
		{
			return FALSE;
		}
	}

	CBCGPMDIFrameWnd* pMainFrame = DYNAMIC_DOWNCAST (CBCGPMDIFrameWnd, pTopFrame);
	if (pMainFrame != NULL)
	{
		return pMainFrame->GetToolbarButtonToolTipText (pButton, strTTText);
	}
	else	// Maybe, SDI frame...
	{
		CBCGPFrameWnd* pFrame = DYNAMIC_DOWNCAST (CBCGPFrameWnd, pTopFrame);
		if (pFrame != NULL)
		{
			return pFrame->GetToolbarButtonToolTipText (pButton, strTTText);
		}
		else // Maybe, MDIChild frame
		{
			 CBCGPMDIChildWnd* pMDIChild = DYNAMIC_DOWNCAST (CBCGPMDIChildWnd, pTopFrame);

			 if(pMDIChild != NULL)
			 {
				 return pMDIChild->GetToolbarButtonToolTipText (pButton, strTTText);
			 }
			 else	// Maybe, OLE frame...
			 {
					CBCGPOleIPFrameWnd* pOleFrame = 
						DYNAMIC_DOWNCAST (CBCGPOleIPFrameWnd, pFrame);
					if (pOleFrame != NULL)
					{
						return pOleFrame->GetToolbarButtonToolTipText (pButton, strTTText);
					}
			 }
		}
	}

	return FALSE;
}
//*****************************************************************************************
void CBCGPToolBar::OnKillFocus(CWnd* pNewWnd) 
{
	CBCGPBaseToolBar::OnKillFocus(pNewWnd);
	
	if (!IsCustomizeMode ())
	{
		CBCGPPopupMenu* pMenu = DYNAMIC_DOWNCAST	(CBCGPPopupMenu, pNewWnd);
		if (pMenu == NULL || pMenu->GetParentToolBar () != this)
		{
			Deactivate ();
		}
	}
}
//********************************************************************************
void CBCGPToolBar::ResetAll ()
{
	BCGPCMD_MGR.ClearAllCmdImages ();

	POSITION pos = NULL;

	//------------------------------------------
	// Fill image hash by the default image ids:
	//------------------------------------------
	for (pos = CBCGPToolBar::m_DefaultImages.GetStartPosition (); pos != NULL;)
	{
		UINT uiCmdId;
		int iImage;

		CBCGPToolBar::m_DefaultImages.GetNextAssoc (pos, uiCmdId, iImage);
		BCGPCMD_MGR.SetCmdImage (uiCmdId, iImage, FALSE);
	}

	for (pos = gAllToolbars.GetHeadPosition (); pos != NULL;)
	{
		CBCGPToolBar* pToolBar = (CBCGPToolBar*) gAllToolbars.GetNext (pos);
		ASSERT (pToolBar != NULL);

		if (pToolBar->CanBeRestored ())
		{
			pToolBar->RestoreOriginalstate ();
		}
	}
}
//****************************************************************************************
BOOL CBCGPToolBar::TranslateChar (UINT nChar)
{
	if (!CBCGPKeyboardManager::IsKeyPrintable (nChar))
	{
		return FALSE;
	}

	UINT nUpperChar = CBCGPKeyboardManager::TranslateCharToUpper (nChar);

	CBCGPToolbarButton* pButton = NULL;
	if (!m_AcellKeys.Lookup (nUpperChar, pButton))
	{
		return FALSE;
	}

	ASSERT_VALID (pButton);

	//-------------------------------------------
	// Save animation type and disable animation:
	//-------------------------------------------
	CBCGPPopupMenu::ANIMATION_TYPE animType = CBCGPPopupMenu::GetAnimationType ();
	CBCGPPopupMenu::SetAnimationType (CBCGPPopupMenu::NO_ANIMATION);

	BOOL bRes = DropDownMenu (pButton);

	//-------------------
	// Restore animation:
	//-------------------
	CBCGPPopupMenu::SetAnimationType (animType);

	if (bRes)
	{
		return TRUE;
	}

	return ProcessCommand (pButton);
}
//**************************************************************************************
const CObList& CBCGPToolBar::GetAllToolbars ()
{
	return gAllToolbars;
}
//**************************************************************************************
void CBCGPToolBar::OnSettingChange(UINT uFlags, LPCTSTR lpszSection) 
{
	CBCGPBaseToolBar::OnSettingChange(uFlags, lpszSection);

	if (uFlags == SPI_SETNONCLIENTMETRICS ||
		uFlags == SPI_SETWORKAREA ||
		uFlags == SPI_SETICONTITLELOGFONT)
	{
		globalData.UpdateFonts();
		AdjustLayout ();
	}
}
//******************************************************************************
BOOL CBCGPToolBar::IsUserDefined () const
{
	ASSERT_VALID (this);

	CFrameWnd* pTopFrame = BCGCBProGetTopLevelFrame (this);
	if (pTopFrame == NULL)
	{
		return FALSE;
	}

	CBCGPMDIFrameWnd* pMainFrame = DYNAMIC_DOWNCAST (CBCGPMDIFrameWnd, pTopFrame);
	if (pMainFrame != NULL)
	{
		return pMainFrame->m_Impl.IsUserDefinedToolbar (this);
	}
	else	// Maybe, SDI frame...
	{
		CBCGPFrameWnd* pFrame = DYNAMIC_DOWNCAST (CBCGPFrameWnd, pTopFrame);
		if (pFrame != NULL)
		{
			return pFrame->m_Impl.IsUserDefinedToolbar (this);
		}
		else	// Maybe, OLE frame...
		{
			CBCGPOleIPFrameWnd* pOleFrame = 
				DYNAMIC_DOWNCAST (CBCGPOleIPFrameWnd, pFrame);
			if (pOleFrame != NULL)
			{
				return pOleFrame->m_Impl.IsUserDefinedToolbar (this);
			}
		}
	}

	return FALSE;
}
//******************************************************************************
void CBCGPToolBar::CleanUpImages ()
{
	m_Images.Clear ();
	m_ColdImages.Clear ();
	m_MenuImages.Clear ();
	m_DisabledImages.Clear ();
	m_DisabledMenuImages.Clear ();
	m_LargeImages.Clear();
	m_LargeColdImages.Clear();
	m_LargeDisabledImages.Clear();

	m_DefaultImages.RemoveAll ();
	m_UsageCount.Reset ();

	CBCGPToolBarImages::CleanUp ();
}
//*********************************************************************
void CBCGPToolBar::CleanUpLockedImages ()
{
	if (!m_bLocked)
	{
		return;
	}

	m_ImagesLocked.Clear ();
	m_ColdImagesLocked.Clear ();
	m_DisabledImagesLocked.Clear ();
	m_LargeImagesLocked.Clear ();
	m_LargeColdImagesLocked.Clear ();
	m_LargeDisabledImagesLocked.Clear ();
	m_MenuImagesLocked.Clear ();
	m_DisabledMenuImagesLocked.Clear ();
}
//*********************************************************************
LRESULT CBCGPToolBar::OnGetButtonCount(WPARAM,LPARAM)
{
	return GetCount();
}
//*********************************************************************
LRESULT CBCGPToolBar::OnGetItemRect(WPARAM wParam, LPARAM lParam)
{
	GetItemRect ((int) wParam, (LPRECT)lParam );
	return TRUE;
}
//*********************************************************************
LRESULT CBCGPToolBar::OnGetButton(WPARAM wParam, LPARAM lParam)
{
	int idx = int(wParam);
	TBBUTTON * pButton = (TBBUTTON *)lParam;
	UINT style = GetButtonStyle( idx );
	pButton->fsStyle = LOBYTE( LOWORD( style ) );
	pButton->fsState = LOBYTE( HIWORD( style ) );
    pButton->idCommand = GetItemID( idx );
	pButton->iBitmap = 0;
	pButton->dwData = 0;
	pButton->iString = 0;
	return TRUE;
}
//*********************************************************************
LRESULT CBCGPToolBar::OnGetButtonText(WPARAM wParam, LPARAM lParam)
{
	int idx = CommandToIndex ((UINT) wParam);
	CString strBuffer = GetButtonText (idx);

	if (lParam != 0)
	{
		lstrcpy ((LPTSTR) lParam, (LPCTSTR) strBuffer);
	}

	return strBuffer.GetLength ();
}
//*********************************************************************
void CBCGPToolBar::SetLook2000 (BOOL bLook2000)
{
	CBCGPVisualManager::GetInstance ()->SetLook2000 (bLook2000);
}
//*********************************************************************
BOOL CBCGPToolBar::IsLook2000 ()
{
	return CBCGPVisualManager::GetInstance ()->IsLook2000 ();
}
//*********************************************************************
BOOL CBCGPToolBar::SmartUpdate (const CObList& lstPrevButtons)
{
	POSITION posPrev = NULL;

	m_bResourceWasChanged = FALSE;

	//-----------------------------
	// Looking for deleted buttons:
	//-----------------------------
	for (posPrev = lstPrevButtons.GetHeadPosition (); posPrev != NULL;)
	{
		CBCGPToolbarButton* pButtonPrev = 
			DYNAMIC_DOWNCAST (CBCGPToolbarButton, lstPrevButtons.GetNext (posPrev));
		ASSERT_VALID (pButtonPrev);

		//----------------------------
		// Find item in the curr.data:
		//----------------------------
		BOOL bFound = FALSE;

		for (POSITION posCurr = m_OrigButtons.GetHeadPosition (); posCurr != NULL;)
		{
			CBCGPToolbarButton* pButtonCurr = 
				DYNAMIC_DOWNCAST (CBCGPToolbarButton, m_OrigButtons.GetNext (posCurr));
			ASSERT_VALID (pButtonCurr);

			if (pButtonCurr->CompareWith (*pButtonPrev))
			{
				bFound = TRUE;
				break;
			}
		}

		if (!bFound)	// Not found, item was deleted
		{
			m_bResourceWasChanged = TRUE;

			int iIndex = CommandToIndex (pButtonPrev->m_nID);
			if (iIndex >= 0)
			{
				RemoveButton (iIndex);
				
				//Update ResetState
				if (IsAddRemoveQuickCustomize ())
				{
					RemoveResetStateButton (pButtonPrev->m_nID);
				}
			}
		}
	}

	//-----------------------------
	// Looking for the new buttons:
	//-----------------------------
	int i = 0;
	POSITION posCurr = NULL;

	for (posCurr = m_OrigButtons.GetHeadPosition (); posCurr != NULL; i++)
	{
		CBCGPToolbarButton* pButtonCurr = 
			DYNAMIC_DOWNCAST (CBCGPToolbarButton, m_OrigButtons.GetNext (posCurr));
		ASSERT_VALID (pButtonCurr);

		//----------------------------
		// Find item in the prev.data:
		//----------------------------
		BOOL bFound = FALSE;

		for (posPrev = lstPrevButtons.GetHeadPosition (); posPrev != NULL;)
		{
			CBCGPToolbarButton* pButtonPrev = 
				DYNAMIC_DOWNCAST (CBCGPToolbarButton, lstPrevButtons.GetNext (posPrev));
			ASSERT_VALID (pButtonPrev);

			if (pButtonCurr->CompareWith (*pButtonPrev))
			{
				bFound = TRUE;
				break;
			}
		}

		if (!bFound)	// Not found, new item!
		{
			m_bResourceWasChanged = TRUE;

			UINT uiCmd = pButtonCurr->m_nID;
			int iIndex = min ((int) m_Buttons.GetCount (), i);

			if (uiCmd == 0)	// Separator
			{
				InsertSeparator (iIndex);
			}
			else
			{
				int iImage = -1;
				m_DefaultImages.Lookup (uiCmd, iImage);

				InsertButton (
					CBCGPToolbarButton (uiCmd, iImage, NULL, FALSE, m_bLocked), iIndex);

				//Update ResetState
				if (IsAddRemoveQuickCustomize ())
				{
					InsertResetStateButton (
						CBCGPToolbarButton (uiCmd, iImage, NULL, FALSE, m_bLocked), iIndex);
				}
			}
		}
	}

	//--------------------------------
	// Compare current and prev. data:
	//--------------------------------
	if (lstPrevButtons.GetCount () != m_OrigButtons.GetCount ())
	{
		m_bResourceWasChanged = TRUE;
	}
	else
	{
		for (posCurr = m_OrigButtons.GetHeadPosition (),
			posPrev = lstPrevButtons.GetHeadPosition (); posCurr != NULL;)
		{
			ASSERT (posPrev != NULL);

			CBCGPToolbarButton* pButtonCurr = 
				DYNAMIC_DOWNCAST (CBCGPToolbarButton, m_OrigButtons.GetNext (posCurr));
			ASSERT_VALID (pButtonCurr);

			CBCGPToolbarButton* pButtonPrev = 
				DYNAMIC_DOWNCAST (CBCGPToolbarButton, lstPrevButtons.GetNext (posPrev));
			ASSERT_VALID (pButtonPrev);

			if (!pButtonCurr->CompareWith (*pButtonPrev))
			{
				m_bResourceWasChanged = TRUE;
				break;
			}
		}
	}

	return m_bResourceWasChanged;
}
//***********************************************************************************
void CBCGPToolBar::UpdateTooltips ()
{
	if (m_pToolTip->GetSafeHwnd () == NULL)
	{
		return;
	}

	while (m_nTooltipsCount-- >= 0)
	{
		m_pToolTip->DelTool (this, m_nTooltipsCount);
	}

	m_nTooltipsCount = 0;
	for (int i = 0; i < m_Buttons.GetCount (); i++)
	{
		CBCGPToolbarButton* pButton = GetButton (i);
		ASSERT_VALID (pButton);

		if (pButton->m_nStyle != TBBS_SEPARATOR)
		{
			TCHAR szFullText [256];
			CString strTipText;

			AfxLoadString (pButton->m_nID, szFullText);
			AfxExtractSubString (strTipText, szFullText, 1, '\n');

			if (!pButton->OnUpdateToolTip (this, i, *m_pToolTip, strTipText))
			{
				m_pToolTip->AddTool (this, LPSTR_TEXTCALLBACK, pButton->Rect (), i + 1);
			}

			m_nTooltipsCount ++;
		}
	}
}
//************************************************************************************
BOOL CBCGPToolBar::OnNeedTipText(UINT /*id*/, NMHDR* pNMH, LRESULT* /*pResult*/)
{
	static CString strTipText;

	if (m_pToolTip->GetSafeHwnd () == NULL || 
		pNMH->hwndFrom != m_pToolTip->GetSafeHwnd ())
	{
		return FALSE;
	}

	CPoint point;
	::GetCursorPos (&point);
	ScreenToClient (&point);

	TOOLINFO ti; memset(&ti, 0, sizeof(TOOLINFO));
	ti.cbSize = sizeof(AFX_OLDTOOLINFO);
	INT_PTR nHit = (INT_PTR) OnToolHitTest(point, &ti);

	if (nHit < 0 || ti.lpszText == NULL || ti.lpszText == LPSTR_TEXTCALLBACK)
	{
		return FALSE;
	}

	LPNMTTDISPINFO	pTTDispInfo	= (LPNMTTDISPINFO) pNMH;
	ASSERT((pTTDispInfo->uFlags & TTF_IDISHWND) == 0);

	strTipText = ti.lpszText;
	free (ti.lpszText);

	pTTDispInfo->lpszText = const_cast<LPTSTR> ((LPCTSTR) strTipText);

	m_pToolTip->SetFont (&globalData.fontRegular, FALSE);
	return TRUE;
}
//************************************************************************************
void CBCGPToolBar::OnAfterFloat ()
{
	CBCGPBaseToolBar::OnAfterFloat ();

	StretchControlBar (m_nMRUWidth, FALSE);

	CBCGPMiniFrameWnd* pParentMiniFrame = GetParentMiniFrame ();
	if (pParentMiniFrame != NULL)
	{
		pParentMiniFrame->SizeToContent ();
	}
	m_bFloating = TRUE;
}
//************************************************************************************
void CBCGPToolBar::OnAfterDock (CBCGPBaseControlBar* pBar, LPCRECT lpRect, BCGP_DOCK_METHOD dockMethod) 
{
	m_bFloating = FALSE;
	CBCGPBaseToolBar::OnAfterDock  (pBar, lpRect, dockMethod);

	if (GetParent() != NULL)
	{
		CRect rect;
		GetWindowRect (rect);
		GetParent()->ScreenToClient (rect);

		GetParent()->RedrawWindow (rect);
	}
}
//************************************************************************************
void CBCGPToolBar::OnBeforeChangeParent (CWnd* pWndNewParent, BOOL bDelay)
{
	CBCGPBaseToolBar::OnBeforeChangeParent (pWndNewParent, bDelay);
	m_bFloating = pWndNewParent != NULL && 
					pWndNewParent->IsKindOf (RUNTIME_CLASS (CBCGPMiniFrameWnd));
}
//************************************************************************************
void CBCGPToolBar::EnableDocking (DWORD dwAlignment)
{
	m_dwBCGStyle = CBRS_BCGP_FLOAT;
	CBCGPBaseToolBar::EnableDocking (dwAlignment);
}
//************************************************************************************
void CBCGPToolBar::SaveOriginalState (CBCGPRegistry& reg)
{
	if (!m_OrigButtons.IsEmpty ())
	{
		reg.Write (REG_ENTRY_ORIG_ITEMS, m_OrigButtons);
	}
}
//*************************************************************************************
BOOL CBCGPToolBar::LoadLastOriginalState (CBCGPRegistry& reg)
{
	BOOL bIsUpdated = FALSE;

	CObList lstOrigButtons;	// Original (resource) data in the last session
	if (reg.Read (REG_ENTRY_ORIG_ITEMS, lstOrigButtons))
	{
		bIsUpdated = SmartUpdate (lstOrigButtons);
	}

	while (!lstOrigButtons.IsEmpty ())
	{
		delete lstOrigButtons.RemoveHead ();
	}

	return bIsUpdated;
}
//*************************************************************************************
void CBCGPToolBar::AddCommandUsage (UINT uiCommand)
{
	m_UsageCount.AddCmd (uiCommand);
}
//***************************************************************************************
CBCGPToolBarImages* CBCGPToolBar::GetImageList (CBCGPToolBarImages& images, CBCGPToolBarImages& imagesLocked, 
							 CBCGPToolBarImages& largeImages, CBCGPToolBarImages& largeImagesLocked) const
{
	if (m_bLocked)
	{
		return (!m_bMenuMode && m_bLargeIcons && largeImagesLocked.GetCount () > 0) ?
				&largeImagesLocked : &imagesLocked;
	}
	else
	{
		return (!m_bMenuMode && m_bLargeIcons && largeImages.GetCount () > 0) ?
				&largeImages : &images;
	}
}
//***************************************************************************************
void CBCGPToolBar::AdjustSize ()
{
	CFrameWnd* pParent = BCGPGetParentFrame (this);
	if (pParent != NULL && pParent->GetSafeHwnd () != NULL)
	{
		BOOL bMode = (m_pParentDockBar == NULL);
		CSize sizeCurr = CalcFixedLayout (bMode, IsHorizontal ());

		if (sizeCurr.cx == 32767 || sizeCurr.cy == 32767)
		{
			CRect rectParent;
			GetParent ()->GetClientRect (&rectParent);

			if (sizeCurr.cx == 32767)
			{
				sizeCurr.cx = rectParent.Width ();

				if (m_nMaxLen != 0)
				{
					sizeCurr.cx = min (sizeCurr.cx, m_nMaxLen);
				}
			}
			else
			{
				sizeCurr.cy = rectParent.Height ();

				if (m_nMaxLen != 0)
				{
					sizeCurr.cy = min (sizeCurr.cy, m_nMaxLen);
				}
			}
		}

		CRect rect;
		GetWindowRect (rect);

		CBCGPTabWnd* pTab = DYNAMIC_DOWNCAST (CBCGPTabWnd, GetParent ());
		if (pTab != NULL)
		{
			CRect rectWndArea;
			pTab->GetWndArea (rectWndArea);
			SetWindowPos (NULL,
					-1, -1,
					rectWndArea.Width (), rectWndArea.Height (),
					SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE);
		}
		else
		{
			if (IsCustomizeMode ())
			{
				if (rect.Height () != sizeCurr.cy && sizeCurr.cy != 32767 && 
					IsHorizontal () ||
					rect.Width () != sizeCurr.cx && sizeCurr.cx != 32767 && 
					!IsHorizontal ())
				{
					SetWindowPos (NULL, 0, 0, sizeCurr.cx, sizeCurr.cy, SWP_NOMOVE  | SWP_NOACTIVATE | SWP_NOZORDER);
					UpdateVirtualRect ();
				} 
				
			}
			else
			{
				if (rect.Height () != sizeCurr.cy && sizeCurr.cy != 32767 && 
					IsHorizontal () ||
					rect.Width () != sizeCurr.cx && sizeCurr.cx != 32767 && 
					!IsHorizontal ())
				{
					CSize sizeMin;
					GetMinSize (sizeMin);

					int nNewWidth = max (sizeMin.cx, sizeCurr.cx);
					int nNewHeight = max (sizeMin.cy, sizeCurr.cy);

					SetWindowPos (NULL, 0, 0, nNewWidth, nNewHeight, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
					UpdateVirtualRect ();
				}
				
			}
		}

		if (!IsFloating() && IsVisible() && m_pParentDockBar != NULL && m_pDockBarRow != NULL)
		{
			CRect rectWnd;
			BOOL bIsHorz = IsHorizontal ();
			BOOL bResize = FALSE;

			m_pDockBarRow->GetClientRect (rectWnd);

			if (rectWnd.Height () != sizeCurr.cy && bIsHorz)
			{
				rectWnd.bottom = rectWnd.top + sizeCurr.cy;
				bResize = TRUE;
			}
			else if (rectWnd.Width () != sizeCurr.cx && !bIsHorz)
			{
				rectWnd.right = rectWnd.left + sizeCurr.cx;
				bResize = TRUE;
			}

			if (bResize)
			{
				m_pParentDockBar->ResizeRow (m_pDockBarRow, 
					 bIsHorz ? sizeCurr.cy : sizeCurr.cx);
			}

			if (IsCustomizeMode ())
			{
				UpdateVirtualRect (sizeCurr);
				m_pDockBarRow->ArrangeBars(this);
			}
			
			pParent->RecalcLayout ();
		}
		else
		{
			CBCGPMiniFrameWnd* pParentMiniFrame = GetParentMiniFrame ();
			if (pParentMiniFrame != NULL && GetParent () == pParentMiniFrame)
			{
				pParentMiniFrame->SizeToContent ();
				pParentMiniFrame->RedrawWindow ();
			}
			else if (!IsKindOf(RUNTIME_CLASS(CBCGPDropDownToolBar)))
			{
				pParent->RecalcLayout ();
			}
		}
	}
}
//***************************************************************************************
void CBCGPToolBar::OnCalcSeparatorRect (CBCGPToolbarButton* pButton, 
										CRect& rectSeparator, 
										BOOL bHorz)
{
	CRect rectClient;
	GetClientRect (rectClient);
	
	rectSeparator = pButton->Rect ();

	if (pButton->m_bWrap && bHorz)
	{
		rectSeparator.left = rectClient.left;
		rectSeparator.right = rectClient.right;

		rectSeparator.top = pButton->Rect ().bottom;
		rectSeparator.bottom = rectSeparator.top + LINE_OFFSET;
	}	
}
//**********************************************************************************
void CBCGPToolBar::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CBCGPBaseToolBar::OnShowWindow(bShow, nStatus);
	
	if (!IsCustomizeMode () || g_pWndCustomize == NULL || m_bLocked)
	{
		return;
	}

	ASSERT_VALID (g_pWndCustomize);

	if (!bShow)
	{
		g_pWndCustomize->ShowToolBar (this, FALSE);

		if (m_pSelToolbar == this)
		{
			m_pSelToolbar = NULL;
			m_iSelected = -1;
		}
	}
	else
	{
		g_pWndCustomize->ShowToolBar (this, TRUE);
	}
}
//*************************************************************************************
LRESULT CBCGPToolBar::OnPromptReset(WPARAM, LPARAM)
{
	//Get Toolbar caption
	CString strCaption;
	GetWindowText(strCaption);
	strCaption.TrimLeft();	strCaption.TrimRight();

	CString strPrompt;

	{
		CBCGPLocalResource locaRes;

		if(!strCaption.GetLength())
		{
			strCaption.LoadString(IDS_BCGBARRES_UNTITLED_TOOLBAR);
		}

		strPrompt.Format (IDS_BCGBARRES_RESET_TOOLBAR_FMT, strCaption);
	}

	//Ask for reset
	if (AfxMessageBox (strPrompt, MB_OKCANCEL|MB_ICONWARNING) == IDOK)
	{
		RestoreOriginalstate();
	}

	return 0;
}
//*************************************************************************************
void CBCGPToolBar::SaveResetOriginalState (CBCGPRegistry& reg)
{
	if (!m_OrigResetButtons.IsEmpty ())
	{
		reg.Write (REG_ENTRY_RESET_ITEMS, m_OrigResetButtons);
	}
}
//*************************************************************************************
BOOL CBCGPToolBar::LoadResetOriginalState (CBCGPRegistry& reg)
{
	CObList lstOrigButtons;
	if (reg.Read (REG_ENTRY_RESET_ITEMS, lstOrigButtons))
	{
		if (lstOrigButtons.GetCount() > 0)
		{
			while (!m_OrigResetButtons.IsEmpty ())
			{
				delete m_OrigResetButtons.RemoveHead ();
			}

			int i = 0;
			for (POSITION pos = lstOrigButtons.GetHeadPosition (); pos != NULL; i++)
			{
				CBCGPToolbarButton* pButton = (CBCGPToolbarButton*) lstOrigButtons.GetNext (pos);

				if(pButton != NULL && pButton->IsKindOf(RUNTIME_CLASS(CBCGPToolbarButton)))
				m_OrigResetButtons.AddTail(pButton);
			}
		}
	}

	return TRUE;
}
//*************************************************************************************
void CBCGPToolBar::SetBrotherToolbar(CBCGPToolBar* pBrotherToolbar)
{
	m_bHasBrother = TRUE;
	m_pBrotherToolBar = pBrotherToolbar;
	pBrotherToolbar->m_bHasBrother = TRUE;
	pBrotherToolbar->m_pBrotherToolBar = this;
	m_bElderBrother = TRUE;
	pBrotherToolbar->m_bElderBrother = FALSE;

}
//*************************************************************************************
BOOL CBCGPToolBar::IsOneRowWithBrother()
{
	CBCGPDockBarRow* pRowThis = GetDockRow ();
	CBCGPDockBarRow* pRowBrother = m_pBrotherToolBar->GetDockRow ();
	if (pRowThis == pRowBrother)
	{
		return TRUE;
	}

	return FALSE;
}
//*************************************************************************************
void CBCGPToolBar::SetOneRowWithBrother()
{
	if (!m_bHasBrother || m_pBrotherToolBar == NULL)
	{
		return;
	}

	CBCGPDockBarRow* pRowThis = GetDockRow ();
	CBCGPDockBarRow* pRowBrother = m_pBrotherToolBar->GetDockRow ();
	if(pRowThis != pRowBrother)
	{
		if(m_bElderBrother)
		{
			pRowBrother->RemoveControlBar (m_pBrotherToolBar);
			pRowThis->AddControlBar (m_pBrotherToolBar, BCGP_DM_STANDARD);
		}
		else
		{
			pRowThis->RemoveControlBar (this);
			pRowBrother->AddControlBar (this, BCGP_DM_STANDARD);
		}
	}
}
//*************************************************************************************
void CBCGPToolBar::SetTwoRowsWithBrother()
{
	if (!m_bHasBrother || m_pBrotherToolBar == NULL)
	{
		return;
	}

	CBCGPDockBarRow* pRowThis = GetDockRow ();
	CBCGPDockBarRow* pRowBrother = m_pBrotherToolBar->GetDockRow ();
	if(pRowThis == pRowBrother)
	{

		if(m_bElderBrother)
		{
			pRowBrother->RemoveControlBar (this);
			CBCGPDockBar* pDockBar = m_pBrotherToolBar->GetParentDockBar ();
			const CObList& list = pDockBar->GetDockBarRowsList ();
			CSize szBarSize = CalcFixedLayout (FALSE, TRUE);

			POSITION pos = list.Find (pRowBrother);
			CBCGPDockBarRow* pNewRow = pDockBar->AddRow (pos, szBarSize.cy);
			pNewRow->AddControlBar (this, BCGP_DM_STANDARD);

			HDWP hdwp = BeginDeferWindowPos (10);
			pNewRow->MoveControlBar (m_pBrotherToolBar, 0, hdwp);
			EndDeferWindowPos (hdwp);
		}
		else
		{
			pRowThis->RemoveControlBar(m_pBrotherToolBar);
			CBCGPDockBar* pDockBar = this->GetParentDockBar();
			const CObList& list = pDockBar->GetDockBarRowsList();
			CSize szBarSize = m_pBrotherToolBar->CalcFixedLayout (FALSE, TRUE);

			POSITION pos = list.Find(pRowThis);
			CBCGPDockBarRow* pNewRow = pDockBar->AddRow (pos, szBarSize.cy);
			pNewRow->AddControlBar(m_pBrotherToolBar, BCGP_DM_STANDARD);

			HDWP hdwp = BeginDeferWindowPos (10);
			pNewRow->MoveControlBar (this, 0, hdwp);
			EndDeferWindowPos (hdwp);
		}
	}
}
//*************************************************************************************
BOOL CBCGPToolBar::CanHandleBrothers()
{
	if (!m_bHasBrother || m_pBrotherToolBar == NULL)
	{
		return FALSE;
	}

	CBCGPDockBar* pDockBarCurrent = GetParentDockBar ();
	CBCGPDockBar* pDockBarBrother = m_pBrotherToolBar->GetParentDockBar ();

	if (pDockBarBrother != NULL && pDockBarCurrent == pDockBarBrother)
	{
		return TRUE;
	}

	return FALSE;
}
//********************************************************************************
void CBCGPToolBar::OnGlobalFontsChanged ()
{
	ASSERT_VALID (this);

	for (POSITION pos = m_Buttons.GetHeadPosition (); pos != NULL;)
	{
		CBCGPToolbarButton* pButton = (CBCGPToolbarButton*) m_Buttons.GetNext (pos);
		ASSERT (pButton != NULL);
	
		pButton->OnGlobalFontsChanged ();
	}
}
//********************************************************************************
void CBCGPToolBar::AutoGrayInactiveImages (BOOL bEnable/* = TRUE*/,
										   int nGrayPercentage/* = 0 */,
										   BOOL bRedrawAllToolbars/* = TRUE*/)
{
	m_bAutoGrayInactiveImages = bEnable;
	m_nGrayImagePercentage = nGrayPercentage;

	if (m_bAutoGrayInactiveImages)
	{
		m_Images.CopyTo (m_ColdImages);
		m_ColdImages.GrayImages (m_nGrayImagePercentage);
	}
	else
	{
		m_ColdImages.Clear ();
	}

	for (POSITION posTlb = gAllToolbars.GetHeadPosition (); posTlb != NULL;)
	{
		CBCGPToolBar* pToolBar = (CBCGPToolBar*) gAllToolbars.GetNext (posTlb);
		ASSERT (pToolBar != NULL);

		if (CWnd::FromHandlePermanent (pToolBar->m_hWnd) != NULL)
		{
			ASSERT_VALID(pToolBar);

			if (pToolBar->IsLocked())
			{
				ASSERT_VALID (pToolBar);

				if (m_bAutoGrayInactiveImages)
				{
					pToolBar->m_ImagesLocked.CopyTo (pToolBar->m_ColdImagesLocked);
					pToolBar->m_ColdImagesLocked.GrayImages (m_nGrayImagePercentage);
				}
				else
				{
					pToolBar->m_ColdImagesLocked.Clear ();
				}
			}

			if (bRedrawAllToolbars)
			{
				pToolBar->RedrawWindow (NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_FRAME | RDW_ERASE | RDW_ALLCHILDREN);
			}
		}
	}
}
//********************************************************************************
BOOL CBCGPToolBar::RemoveResetStateButton (UINT uiCmdId)
{
	int i = 0;
	int nIndex = -1;
	for (POSITION pos = m_OrigResetButtons.GetHeadPosition (); pos != NULL; i ++)
	{
		CBCGPToolbarButton* pButton = (CBCGPToolbarButton*) m_OrigResetButtons.GetNext (pos);
		ASSERT (pButton != NULL);

		if (i >= 0 && pButton->m_nID == uiCmdId)
		{
		   nIndex = i;
		   break;
		}
	}

	if (nIndex < 0 || nIndex >= m_OrigResetButtons.GetCount ())
	{
		return FALSE;
	}

	POSITION posButton = m_OrigResetButtons.FindIndex (nIndex);
	if (posButton == NULL)
	{
		return FALSE;
	}

	m_OrigResetButtons.RemoveAt (posButton);
	return TRUE;
}
//********************************************************************************
int CBCGPToolBar::InsertResetStateButton (const CBCGPToolbarButton& button, int iInsertAt)
{
	if (iInsertAt != -1 &&
		(iInsertAt < 0 || iInsertAt > m_OrigResetButtons.GetCount ()))
	{
		return -1;
	}

	CRuntimeClass* pClass = button.GetRuntimeClass ();
	ASSERT (pClass != NULL);

	CBCGPToolbarButton* pButton = (CBCGPToolbarButton*) pClass->CreateObject ();
	ASSERT_VALID(pButton);
	pButton->CopyFrom (button);

	if (iInsertAt == -1 || iInsertAt == m_OrigResetButtons.GetCount ())
	{
		//-------------------------
		// Add to the tail:
		//-------------------------
		m_OrigResetButtons.AddTail (pButton);
		return (int) m_OrigResetButtons.GetCount () - 1;
	}

	POSITION pos = m_OrigResetButtons.FindIndex (iInsertAt);
	ASSERT (pos != NULL);

	m_OrigResetButtons.InsertBefore (pos, pButton);

	return (int) iInsertAt;
}
//********************************************************************************
void CBCGPToolBar::SetOrigButtons (const CObList& lstOrigButtons)
{
	while (!m_OrigButtons.IsEmpty ())
	{
		delete m_OrigButtons.RemoveHead ();
	}

	m_OrigButtons.AddTail ((CObList*) &lstOrigButtons);
}
//******************************************************************************* 
void CBCGPToolBar::OnRButtonDown(UINT nFlags, CPoint point) 
{
	CBCGPBaseToolBar::OnRButtonDown(nFlags, point);

	if (!IsCustomizeMode () &&
		DYNAMIC_DOWNCAST (CBCGPControlBar, GetParent ()) != NULL)
	{
		GetParent ()->SetFocus ();
	}
}
//****************************************************************************
void CBCGPToolBar::OnChangeVisualManager ()
{
	m_bRoundShape = 
		CBCGPVisualManager::GetInstance ()->IsToolbarRoundShape (this);

	if (m_bRoundShape)
	{
		SetRoundedRgn ();
	}
	else
	{
		SetWindowRgn (NULL, FALSE);
	}

	if (!IsLocked () && !IsTabbed ())
	{
		AdjustSizeImmediate ();
	}

	UpdateImagesColor ();

	m_penDrag.DeleteObject();
	m_penDrag.CreatePen (PS_SOLID, 1, globalData.clrBarText);
}
//****************************************************************************
void CBCGPToolBar::SetRoundedRgn ()
{
	if (GetSafeHwnd () == NULL)
	{
		return;
	}

	if (!m_bRoundShape || GetParentDockBar () == NULL)
	{
		SetWindowRgn (NULL, FALSE);
		return;
	}

	CRect rectWindow;
	GetWindowRect (rectWindow);

	CRgn rgn;
	rgn.CreateRoundRectRgn (0, 0, rectWindow.Width () + 1, rectWindow.Height () + 1, 
		4, 4);

	SetWindowRgn (rgn, FALSE);
}
//****************************************************************************
void CBCGPToolBar::RedrawCustomizeButton ()
{
	if (GetSafeHwnd () == NULL || m_pCustomizeBtn == NULL)
	{
		return;
	}

	ASSERT_VALID (m_pCustomizeBtn);

	CRect rect = m_pCustomizeBtn->GetInvalidateRect ();
	rect.InflateRect (m_pCustomizeBtn->GetExtraSize ());

	rect.right += 10;
	rect.bottom += 10;

	RedrawWindow (rect, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_FRAME);
}
//**************************************************************************
LRESULT CBCGPToolBar::OnBCGUpdateToolTips (WPARAM wp, LPARAM)
{
	UINT nTypes = (UINT) wp;

	if (nTypes & BCGP_TOOLTIP_TYPE_TOOLBAR)
	{
		CBCGPTooltipManager::CreateToolTip (m_pToolTip, this,
			BCGP_TOOLTIP_TYPE_TOOLBAR);
		UpdateTooltips ();
	}

	return 0;
}
//****************************************************************************
BOOL CBCGPToolBar::OnSetAccData (long lVal)
{
	ASSERT_VALID (this);

	m_AccData.Clear ();

	CBCGPToolbarButton* pButton = AccGetButtonByChildId (lVal);
	if (pButton != NULL)
	{
		pButton->SetACCData (this, m_AccData);
		return TRUE;	
	}
	
	return FALSE;
}
//****************************************************************************
HRESULT CBCGPToolBar::accHitTest(long xLeft, long yTop, VARIANT *pvarChild)
{
	if( !pvarChild )
    {
        return E_INVALIDARG;
    }

	pvarChild->vt = VT_I4;
	pvarChild->lVal = CHILDID_SELF;

	CPoint pt (xLeft, yTop);
	ScreenToClient (&pt);

	int count = 1;

	for (POSITION pos = m_Buttons.GetHeadPosition (); pos != NULL;)
	{
		CBCGPToolbarButton* pButton = (CBCGPToolbarButton*) m_Buttons.GetNext (pos);
		ASSERT_VALID (pButton);

		if (pButton->GetAccCount() > 0 || pButton->IsDroppedDown ())
		{
			if (pButton->Rect().PtInRect(pt))
			{
				pvarChild->lVal = count;
				pButton->SetACCData(this, m_AccData);
				break;
			}
			
			count++;
		}
	}

	return S_OK;
}
//****************************************************************************
HRESULT CBCGPToolBar::get_accChildCount(long *pcountChildren)
{
	if( !pcountChildren )
    {
        return E_INVALIDARG;
    }

	*pcountChildren = AccGetButtonsCount ();
	return S_OK;
}
//****************************************************************************
HRESULT CBCGPToolBar::get_accChild(VARIANT varChild, IDispatch **ppdispChild)
{
#if _MSC_VER < 1300
	UNREFERENCED_PARAMETER(varChild);
#endif

	if( !ppdispChild )
    {
        return E_INVALIDARG;
    }

	*ppdispChild = NULL;

#if _MSC_VER >= 1300
	if (varChild.vt == VT_I4 && varChild.lVal != CHILDID_SELF)
	{
		CBCGPToolbarMenuButton* pMenuButton = DYNAMIC_DOWNCAST(CBCGPToolbarMenuButton, AccGetButtonByChildId(varChild.lVal));
		if (pMenuButton != NULL && pMenuButton->GetPopupMenu ()->GetSafeHwnd () != NULL)
		{
			return AccessibleObjectFromWindow(pMenuButton->GetPopupMenu()->GetSafeHwnd(), (DWORD)OBJID_CLIENT,
               IID_IAccessible, (void**) ppdispChild);
		}
	}
#endif

	return S_FALSE;
}
//**************************************************************************
HRESULT CBCGPToolBar::get_accRole(VARIANT varChild, VARIANT *pvarRole)
{
	if (pvarRole == NULL)
    {
        return E_INVALIDARG;
    }

	if (varChild.vt == VT_I4 && varChild.lVal == CHILDID_SELF)
	{
		pvarRole->vt = VT_I4;
		pvarRole->lVal = ROLE_SYSTEM_CLIENT;
		return S_OK;
	}

	return CBCGPBaseToolBar::get_accRole(varChild, pvarRole);
}
//**************************************************************************
HRESULT CBCGPToolBar::get_accState(VARIANT varChild, VARIANT *pvarState)
{
	if (pvarState == NULL)
    {
        return E_INVALIDARG;
    }

	if (varChild.vt == VT_I4 && varChild.lVal == CHILDID_SELF)
	{
		pvarState->vt = VT_I4;
		pvarState->lVal = STATE_SYSTEM_NORMAL;
		return S_OK;
	}

	return CBCGPBaseToolBar::get_accState(varChild, pvarState);
}
//****************************************************************************
HRESULT CBCGPToolBar::accNavigate(long navDir, VARIANT varStart, VARIANT* pvarEndUpAt)
{
	if (pvarEndUpAt == NULL)
	{
        return E_INVALIDARG;
	}

    pvarEndUpAt->vt = VT_EMPTY;

    if (varStart.vt != VT_I4)
    {
        return E_INVALIDARG;
    }

	int count = AccGetButtonsCount ();

	switch (navDir)
    {
	case NAVDIR_FIRSTCHILD:
		if (varStart.lVal == CHILDID_SELF)
		{
			pvarEndUpAt->vt = VT_I4;
			pvarEndUpAt->lVal = 1;
			return S_OK;	
		}
		break;

	case NAVDIR_LASTCHILD:
		if (varStart.lVal == CHILDID_SELF)
		{
			pvarEndUpAt->vt = VT_I4;
			pvarEndUpAt->lVal = count;
			return S_OK;
		}
		break;

	case NAVDIR_NEXT:   
	case NAVDIR_RIGHT:
		if (varStart.lVal != CHILDID_SELF)
		{
			pvarEndUpAt->vt = VT_I4;
			pvarEndUpAt->lVal = varStart.lVal + 1;
			if (pvarEndUpAt->lVal > count)
			{
				pvarEndUpAt->vt = VT_EMPTY;
				return S_FALSE;
			}
			return S_OK;
		}
		break;

	case NAVDIR_PREVIOUS: 
	case NAVDIR_LEFT:
		if (varStart.lVal != CHILDID_SELF)
		{
			pvarEndUpAt->vt = VT_I4;
			pvarEndUpAt->lVal = varStart.lVal - 1;
			if (pvarEndUpAt->lVal <= 0)
			{
				pvarEndUpAt->vt = VT_EMPTY;
				return S_FALSE;
			}
			return S_OK;
		}
		break;
	}

	return S_FALSE;
}
//******************************************************************************
HRESULT CBCGPToolBar::accDoDefaultAction(VARIANT varChild)
{
    if (varChild.vt != VT_I4)
    {
        return E_INVALIDARG;
    }

	CBCGPToolbarButton* pButton = AccGetButtonByChildId (varChild.lVal);

	if (varChild.lVal != CHILDID_SELF && pButton != NULL)
    {
		if (pButton->m_nID != 0 && pButton->m_nID != (UINT) -1)
		{
			if (!pButton->OnClickUp ())
			{
				GetOwner()->SendMessage (WM_COMMAND, pButton->m_nID); 
			}
		}
		else
		{
			pButton->OnClick (this, FALSE);
		}
    }

    return S_OK;
}
//******************************************************************************
void CBCGPToolBar::AccNotifyObjectFocusEvent (int iButton)
{
	if (!globalData.IsAccessibilitySupport ())
	{
		return;
	}

	CBCGPToolbarButton * pButton = GetButton (iButton);
	if (pButton != NULL)
	{
		ASSERT_VALID (pButton);
		pButton->SetACCData (this, m_AccData);

		int nChildID = AccGetChildIdByButtonIndex(iButton);
		if (nChildID > 0)
		{
			globalData.NotifyWinEvent(EVENT_OBJECT_FOCUS, GetSafeHwnd(), OBJID_CLIENT, nChildID);
		}
	}
}
//**************************************************************************
LRESULT CBCGPToolBar::OnBCGSetControlAero (WPARAM wp, LPARAM lp)
{
	CBCGPBaseToolBar::OnBCGSetControlAero (wp, lp);

	for (POSITION pos = m_Buttons.GetHeadPosition (); pos != NULL;)
	{
		CBCGPToolbarButton* pButton = (CBCGPToolbarButton*) m_Buttons.GetNext (pos);
		ASSERT_VALID (pButton);

		pButton->SetOnGlass (IsOnGlass ());
	}

	return 0;
}
//**************************************************************************
void CBCGPToolBar::RedrawUnderlines ()
{
	for (POSITION posTlb = gAllToolbars.GetHeadPosition (); posTlb != NULL;)
	{
		CBCGPToolBar* pToolBar = DYNAMIC_DOWNCAST (CBCGPToolBar, gAllToolbars.GetNext (posTlb));

		if (pToolBar != NULL && CWnd::FromHandlePermanent (pToolBar->m_hWnd) != NULL)
		{
			ASSERT_VALID(pToolBar);

			BOOL bRedrawButtons = FALSE;
				
			for (POSITION pos = pToolBar->m_Buttons.GetHeadPosition (); pos != NULL;)
			{
				CBCGPToolbarButton* pButton = (CBCGPToolbarButton*) pToolBar->m_Buttons.GetNext (pos);
				if (pButton == NULL)
				{
					break;
				}

				ASSERT_VALID (pButton);

				if ((pButton->m_nStyle & TBBS_SEPARATOR) || !pButton->m_bText)
				{
					continue;
				}

				if (pButton->m_strText.Find (_T('&')) >= 0)
				{
					pToolBar->InvalidateRect (pButton->Rect ());
					bRedrawButtons = TRUE;
				}
			}

			if (bRedrawButtons)
			{
				pToolBar->UpdateWindow ();
			}
		}
	}
}
//**************************************************************************
int CBCGPToolBar::AccGetButtonsCount()
{
	int i = 0; int count = 0;
	for (POSITION pos = m_Buttons.GetHeadPosition (); pos != NULL; i ++)
	{
		CBCGPToolbarButton* pButton = (CBCGPToolbarButton*) m_Buttons.GetNext (pos);
		ASSERT (pButton != NULL);

		count += pButton->GetAccCount();
	}

	return count;
}
//**************************************************************************
CBCGPToolbarButton* CBCGPToolBar::AccGetButtonByChildId (long lVal)
{
	int count = 1;

	for (POSITION pos = m_Buttons.GetHeadPosition (); pos != NULL;)
	{
		CBCGPToolbarButton* pButton = (CBCGPToolbarButton*) m_Buttons.GetNext(pos);
		ASSERT_VALID (pButton);

		if (pButton->GetAccCount() > 0)
		{
			if (count == lVal)
			{
				return pButton;
			}
			
			count++;
		}
	}

	return NULL;
}
//**************************************************************************
int CBCGPToolBar::AccGetChildIdByButtonIndex(int nButtonIndex)
{
	if (nButtonIndex < 0 || nButtonIndex >= m_Buttons.GetCount())
	{
		return 0;
	}

	int count = 1;
	int i = 0;

	for (POSITION pos = m_Buttons.GetHeadPosition (); pos != NULL; i++)
	{
		CBCGPToolbarButton* pButton = (CBCGPToolbarButton*) m_Buttons.GetNext(pos);
		ASSERT_VALID (pButton);

		if (pButton->GetAccCount() > 0)
		{
			if (i == nButtonIndex)
			{
				return count;
			}
			
			count++;
		}
	}

	return 0;
}

