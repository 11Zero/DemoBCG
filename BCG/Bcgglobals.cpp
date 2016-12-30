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

#include "stdafx.h"
#include "BCGCBPro.h"

#if (_MSC_VER >= 1800) && (!defined _USING_V110_SDK71_)
	#include "versionhelpers.h"
#endif

#pragma warning (disable : 4706)

#ifdef _BCGPCHART_STANDALONE
#define COMPILE_MULTIMON_STUBS
#endif // _AFXDLL

#include "multimon.h"

#pragma warning (default : 4706)

#include "comdef.h"
#include "bcgglobals.h"

#if (!defined _BCGPCALENDAR_STANDALONE) && !(defined _BCGPGRID_STANDALONE) && !(defined _BCGPEDIT_STANDALONE) && !(defined _BCGPCHART_STANDALONE)
	#include "BCGPGestureManager.h"
	#include "BCGPGraphicsManagerD2D.h"
	#include "BCGPDrawManager.h"
	#include "BCGPVisualManager.h"
	#include "BCGPKeyboardManager.h"
	#include "MenuHash.h"
	#include "BCGPToolBar.h"
	#include "MenuImages.h"
	#include "BCGPMiniframeWnd.h"
	#include "BCGPDockManager.h"
	#include "BCGPVisualManager2007.h"
	#include "BCGPVisualManager2010.h"
	#include "BCGPLocalResource.h"
	#include "BCGPDialogBar.h"
	#include "bcgprores.h"
#endif

#ifndef SPI_GETMENUANIMATION
#define SPI_GETMENUANIMATION	0x1002
#endif

#ifndef SPI_GETMENUFADE
#define SPI_GETMENUFADE			0x1012
#endif

#ifndef SPI_GETMENUUNDERLINES
#define SPI_GETKEYBOARDCUES		0x100A
#define SPI_GETMENUUNDERLINES	SPI_GETKEYBOARDCUES
#endif

extern CObList	gAllToolbars;

BOOL CBCGPMemDC::m_bUseMemoryDC = TRUE;

static const CString strOfficeFontName		= _T("Tahoma");
static const CString strOffice2007FontName	= _T("Segoe UI");
static const CString strDefaultFontName		= _T("MS Sans Serif");
static const CString strVertFontName		= _T("Arial");
static const CString strMarlettFontName		= _T("Marlett");

/////////////////////////////////////////////////////////////////////////////
// CBCGPMemDC

CBCGPMemDC::CBCGPMemDC (CDC& dc, CWnd* pWnd, BYTE alpha, double dblScale) :
	m_dc			(dc),
	m_bMemDC		(FALSE),
	m_hBufferedPaint(NULL),
	m_pOldBmp		(NULL),
	m_alpha			(alpha),
	m_dblScale		(dblScale)
{
	ASSERT_VALID(pWnd);

	if (m_dblScale <= 0)
	{
		m_dblScale = 1.0;
	}

	pWnd->GetClientRect (m_rect);

	m_rect.right += pWnd->GetScrollPos (SB_HORZ);
	m_rect.bottom += pWnd->GetScrollPos (SB_VERT);

	if (m_dblScale == 1.0 && globalData.m_pfBeginBufferedPaint != NULL && globalData.m_pfEndBufferedPaint != NULL)
	{
		HDC hdcPaint = NULL;

		m_hBufferedPaint = (*globalData.m_pfBeginBufferedPaint) 
			(dc.GetSafeHdc (), m_rect, BCGP_BPBF_TOPDOWNDIB, NULL, &hdcPaint);

		if (m_hBufferedPaint != NULL && hdcPaint != NULL)
		{
			m_bMemDC = TRUE;
			m_dcMem.Attach (hdcPaint);
		}
	}
	else
	{
		if (m_bUseMemoryDC &&
			m_dcMem.CreateCompatibleDC (&m_dc) &&
			m_bmp.CreateCompatibleBitmap (&m_dc, m_rect.Width (), m_rect.Height ()))
		{
			//-------------------------------------------------------------
			// Off-screen DC successfully created. Better paint to it then!
			//-------------------------------------------------------------
			m_bMemDC = TRUE;
			m_pOldBmp = m_dcMem.SelectObject (&m_bmp);
		}
	}
}

CBCGPMemDC::CBCGPMemDC(CDC& dc, const CRect& rect, BYTE alpha, double dblScale) :
	m_dc			(dc),
	m_bMemDC		(FALSE),
	m_hBufferedPaint(NULL),
	m_pOldBmp		(NULL),
	m_rect			(rect),
	m_alpha			(alpha),
	m_dblScale		(dblScale)
{
	ASSERT(!m_rect.IsRectEmpty());

	if (m_dblScale <= 0)
	{
		m_dblScale = 1.0;
	}

	if (m_dblScale == 1.0 && globalData.m_pfBeginBufferedPaint != NULL && globalData.m_pfEndBufferedPaint != NULL)
	{
		HDC hdcPaint = NULL;

		m_hBufferedPaint = (*globalData.m_pfBeginBufferedPaint) 
			(dc.GetSafeHdc (), m_rect, BCGP_BPBF_TOPDOWNDIB, NULL, &hdcPaint);

		if (m_hBufferedPaint != NULL && hdcPaint != NULL)
		{
			m_bMemDC = TRUE;
			m_dcMem.Attach (hdcPaint);
		}
	}
	else
	{
		if (m_bUseMemoryDC &&
			m_dcMem.CreateCompatibleDC (&m_dc) &&
			m_bmp.CreateCompatibleBitmap (&m_dc, m_rect.Width (), m_rect.Height ()))
		{
			//-------------------------------------------------------------
			// Off-screen DC successfully created. Better paint to it then!
			//-------------------------------------------------------------
			m_bMemDC = TRUE;
			m_pOldBmp = m_dcMem.SelectObject (&m_bmp);
		}
	}
}

CBCGPMemDC::~CBCGPMemDC()
{
	if (m_hBufferedPaint != NULL)
	{
		m_dcMem.Detach ();

		if (m_alpha != 0 && globalData.m_pfBufferedPaintSetAlpha != NULL)
		{
			(*globalData.m_pfBufferedPaintSetAlpha) (m_hBufferedPaint, NULL, m_alpha);
		}

		(*globalData.m_pfEndBufferedPaint) (m_hBufferedPaint, TRUE);
	}
	else if (m_bMemDC)
	{
		//--------------------------------------
		// Copy the results to the on-screen DC:
		//-------------------------------------- 
		CRect rectClip;
		int nClipType = m_dc.GetClipBox (rectClip);

		if (nClipType != NULLREGION)
		{
			if (nClipType != SIMPLEREGION)
			{
				rectClip = m_rect;
			}

			if (m_dblScale != 1.0)
			{
				CRect rectDst = m_rect;
				rectDst.right = rectDst.left + (int)(0.5 + m_dblScale * m_rect.Width());
				rectDst.bottom = rectDst.top + (int)(0.5 + m_dblScale * m_rect.Height());

				m_dc.SetStretchBltMode(HALFTONE);

				if (m_dblScale < 1.0)
				{
					CRect rectRight = m_rect;
					rectRight.left = rectDst.right;

					m_dc.FillRect(rectRight, &globalData.brWindow);

					CRect rectBottom = m_rect;
					rectBottom.top = rectDst.bottom;

					m_dc.FillRect(rectBottom, &globalData.brWindow);
				}
				
				m_dc.StretchBlt(rectDst.left, rectDst.top,
					rectDst.Width(), rectDst.Height(), 
					&m_dcMem, m_rect.left, m_rect.top,
					m_rect.Width(), m_rect.Height(), SRCCOPY);
			}
			else
			{
				m_dc.BitBlt (rectClip.left, rectClip.top, rectClip.Width(), rectClip.Height(),
							   &m_dcMem, rectClip.left, rectClip.top, SRCCOPY);
			}
		}

		m_dcMem.SelectObject (m_pOldBmp);
	}
}

static int CALLBACK FontFamalyProcFonts (const LOGFONT FAR* lplf,
									const TEXTMETRIC FAR* /*lptm*/,
									ULONG /*ulFontType*/,
									LPARAM lParam)
{
	ASSERT (lplf != NULL);
	ASSERT (lParam != NULL);

	CString strFont = lplf->lfFaceName;
	return strFont.CollateNoCase ((LPCTSTR) lParam) == 0 ? 0 : 1;
}

typedef int (WINAPI * BCGP_LCIDTOLOCALENAME)(LCID Locale, LPWSTR  lpName, int cchName, DWORD dwFlags);

static BCGP_LCIDTOLOCALENAME g_pfLCIDToLocaleName = NULL;

static BOOL CALLBACK EnumLocalesProc(LPTSTR lpLocaleString)
{
	if (lpLocaleString == NULL || g_pfLCIDToLocaleName == NULL)
	{
		return TRUE;
	}

	LCID lcid = (LCID)_tcstol(lpLocaleString, NULL, 16);

	CString strLocale;
	int requiredLength = ::GetLocaleInfo (lcid, LOCALE_SLANGUAGE, NULL, 0);
	if (requiredLength == 0)
	{
		return 0;
	}

	if (::GetLocaleInfo (lcid, LOCALE_SLANGUAGE, strLocale.GetBuffer (requiredLength + 1), _MAX_PATH) != requiredLength)
	{
 		return TRUE;
	}

	strLocale.ReleaseBuffer ();

 	WCHAR lpNameW[LOCALE_NAME_MAX_LENGTH];
	if ((*g_pfLCIDToLocaleName)(lcid, lpNameW, LOCALE_NAME_MAX_LENGTH, 0) == 0)
 	{
  		return TRUE;
 	}

	LPTSTR lpName = NULL;

#ifdef _UNICODE
	lpName = lpNameW;
#else
	const size_t length = wcslen(lpNameW) + 1;
	lpName = (LPTSTR) new TCHAR[length];
	lpName = AfxW2AHelper (lpName, lpNameW, (int)length);
#endif

	if (lpName == NULL)
	{
		return TRUE;
	}

	globalData.AddLocale(strLocale, lpName);

	if ((LPVOID)lpName != (LPVOID)lpNameW)
	{
		delete[] lpName;
	}

	return TRUE;
}
/////////////////////////////////////////////////////////////////////////////
// Cached system metrics, etc

BCGCBPRODLLEXPORT BCGPGLOBAL_DATA globalData;

// Initialization code
BCGPGLOBAL_DATA::BCGPGLOBAL_DATA()
{
#if _MSC_VER >= 1400
#ifdef BCGP_CLEAN_DLL_PATH
	::SetDllDirectory(_T("")); 
#endif
#endif

	//-----------------------
	// Detect the kind of OS:
	//-----------------------
#if (_MSC_VER >= 1800) && (!defined _USING_V110_SDK71_)
	bIsOSAlphaBlendingSupport = TRUE;

	bIsWindowsNT4 = FALSE;	// Not supported by VS
	bIsWindows9x = FALSE;	// Not supported by VS
	bIsWindows2000 = TRUE;

	bIsWindowsXP = IsWindowsXPOrGreater();
	bIsWindowsVista = IsWindowsVistaOrGreater();
	bIsWindows7 = IsWindows7OrGreater();
	bIsWindows8 = IsWindows8OrGreater();
#else
	OSVERSIONINFO osvi;
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	::GetVersionEx (&osvi);
	bIsWindowsNT4 = ((osvi.dwPlatformId == VER_PLATFORM_WIN32_NT) &&
					(osvi.dwMajorVersion < 5));

	bIsWindows9x = (osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS);

	bIsOSAlphaBlendingSupport = (osvi.dwMajorVersion > 4) ||
		((osvi.dwMajorVersion == 4) && (osvi.dwMinorVersion > 0));

	if (bIsWindowsNT4)
	{
		bIsOSAlphaBlendingSupport = FALSE;
	}

	bIsWindows2000 = (osvi.dwMajorVersion >= 5 && osvi.dwPlatformId == VER_PLATFORM_WIN32_NT);
	bIsWindowsXP = (osvi.dwMajorVersion == 5) && (osvi.dwMinorVersion >= 1) || (osvi.dwMajorVersion > 5);
	bIsWindowsVista = (osvi.dwMajorVersion >= 6);
	bIsWindows7 = (osvi.dwMajorVersion == 6) && (osvi.dwMinorVersion >= 1) || (osvi.dwMajorVersion > 6);
	bIsWindows8 = (osvi.dwMajorVersion == 6) && (osvi.dwMinorVersion >= 2) || (osvi.dwMajorVersion > 6);
#endif

	bIsRemoteSession = FALSE;

	if (bIsWindows2000)
	{
		bIsRemoteSession = GetSystemMetrics (0x1000 /* SM_REMOTESESSION */);
	}

	bDisableAero = FALSE;

	m_bShellAutohideBarsInitialized = FALSE;
	m_nShellAutohideBars = 0;

	m_bIsRibbonImageScale = TRUE;

	//---------------------------------------------------------
	// Cached system values (updated in CWnd::OnSysColorChange)
	//---------------------------------------------------------
	hbrBtnShadow = NULL;
	hbrBtnHilite = NULL;
	hbrWindow = NULL;

	UpdateSysColors();
	UpdateUxThemeWrappers();
	UpdateUser32Wrappers();
	UpdateDWMApiWrappers();

	m_hcurStretch = NULL;
	m_hcurStretchVert = NULL;
	m_hcurHand = NULL;
	m_hcurSizeAll = NULL;
	m_hiconTool = NULL;
	m_hiconLink = NULL;
	m_hiconColors = NULL;
	m_hcurMoveTab = NULL;
	m_hcurNoMoveTab = NULL;
	m_hcurSelectRow = NULL;

	m_bUseSystemFont = FALSE;

	if (bIsWindowsVista)
	{
		LCID lcID = ::GetSystemDefaultLCID();
		switch(lcID)
		{
		// CP - 936
		case 0x0804: // Chinese (PRC)
		case 0x1004: // Chinese (Singapore)
		// CP - 950
		case 0x0c04: // Chinese (Hong Kong SAR, PRC)
		case 0x0404: // Chinese (Taiwan)
		case 0x1404: // Chinese (Macao SAR)
			m_bUseSystemFont = TRUE;
			break;
		}
	}

	m_bInSettingsChange = FALSE;

	UpdateFonts();
	OnSettingChange ();

	m_bIsRTL = FALSE;

	//------------------
	// Small icon sizes:
	//------------------
	m_nDragFrameThiknessFloat = 4;  // pixels
	m_nDragFrameThiknessDock = 3;   // pixels

	m_nAutoHideToolBarSpacing = 14; // pixels
	m_nAutoHideToolBarMargin  = 4;  // pixels

	m_nCoveredMainWndClientAreaPercent = 50; // percents

	m_nMaxToolTipWidth = -1;
	m_bIsBlackHighContrast = FALSE;
	m_bIsWhiteHighContrast = FALSE;

	m_bUseBuiltIn32BitIcons = TRUE;
	m_bUseVisualManagerInBuiltInDialogs = FALSE;

#if (!defined _BCGPCALENDAR_STANDALONE) && !(defined _BCGPGRID_STANDALONE) && !(defined _BCGPEDIT_STANDALONE) && !(defined _BCGPCHART_STANDALONE)
	m_bEnableAccessibility = FALSE;
	m_pfNotifyWinEvent = NULL;
	EnableAccessibilitySupport ();
#endif

	m_dwComCtlVersion = (DWORD)-1;

	m_pTaskbarList3 = NULL;
	m_bComInitialized = FALSE;

	m_bShowTooltipsOnRibbonFloaty = TRUE;
	m_bShowFrameLayeredShadows = !bIsWindows9x && !bIsWindowsNT4;
}
//*******************************************************************************************
BCGPGLOBAL_DATA::~BCGPGLOBAL_DATA()
{
	CleanUp ();
}
//************************************************************************************
void BCGPGLOBAL_DATA::UpdateUxThemeWrappers()
{
	m_hinstUXThemeDLL = LoadLibrary (_T("UxTheme.dll"));

	if (m_hinstUXThemeDLL != NULL)
	{
		m_pfDrawThemeBackground = (BCGP_DRAWTHEMEPARENTBACKGROUND)::GetProcAddress (m_hinstUXThemeDLL, "DrawThemeParentBackground");
		m_pfDrawThemeTextEx = (BCGP_DRAWTHEMETEXTEX)::GetProcAddress (m_hinstUXThemeDLL, "DrawThemeTextEx");
		m_pfDrawThemeIcon = (BCGP_DRAWTHEMEICON)::GetProcAddress (m_hinstUXThemeDLL, "DrawThemeIcon");
		m_pfBeginBufferedPaint = (BCGP_BEGINBUFFEREDPAINT)::GetProcAddress (m_hinstUXThemeDLL, "BeginBufferedPaint");
		m_pfBufferedPaintSetAlpha = (BCGP_BUFFEREDPAINTSETALPHA)::GetProcAddress (m_hinstUXThemeDLL, "BufferedPaintSetAlpha");
		m_pfEndBufferedPaint = (BCGP_ENDBUFFEREDPAINT)::GetProcAddress (m_hinstUXThemeDLL, "EndBufferedPaint");
		m_pfSetWindowTheme = (BCGP_SETWINDOWTHEME)::GetProcAddress (m_hinstUXThemeDLL, "SetWindowTheme");
		m_pfGetThemeBitmap = (BCGP_GETTHEMEBITMAP)::GetProcAddress (m_hinstUXThemeDLL, "GetThemeBitmap");
	}
	else
	{
		m_pfDrawThemeBackground = NULL;
		m_pfDrawThemeTextEx = NULL;
		m_pfDrawThemeIcon = NULL;
		m_pfBeginBufferedPaint = NULL;
		m_pfBufferedPaintSetAlpha = NULL;
		m_pfEndBufferedPaint = NULL;
		m_pfSetWindowTheme = NULL;
		m_pfGetThemeBitmap = NULL;
	}
}
//************************************************************************************
void BCGPGLOBAL_DATA::UpdateUser32Wrappers()
{
	m_pfSetLayeredWindowAttributes = NULL;
	m_pfUpdateLayeredWindow = NULL;

	m_hinstUser32 = LoadLibrary (_T("USER32.DLL"));
	
	if (m_hinstUser32 != NULL)
	{
		m_pfChangeWindowMessageFilter = (BCGP_CHANGEWINDOWMESSAGEFILTER)GetProcAddress(m_hinstUser32, "ChangeWindowMessageFilter");

		if (bIsOSAlphaBlendingSupport)
		{
			m_pfSetLayeredWindowAttributes = (SETLAYEATTRIB)::GetProcAddress(m_hinstUser32, "SetLayeredWindowAttributes");
			m_pfUpdateLayeredWindow = (UPDATELAYEREDWINDOW)::GetProcAddress(m_hinstUser32, "UpdateLayeredWindow");
		}
	}
	else
	{
		m_pfChangeWindowMessageFilter = NULL;
	}
}
//************************************************************************************
void BCGPGLOBAL_DATA::UpdateDWMApiWrappers()
{
	m_hinstDwmapiDLL = LoadLibrary (_T("dwmapi.dll"));

	if (m_hinstDwmapiDLL != NULL)
	{
		m_pfDwmExtendFrameIntoClientArea = (BCGP_DWMEXTENDFRAMEINTOCLIENTAREA)::GetProcAddress (m_hinstDwmapiDLL, "DwmExtendFrameIntoClientArea");
		m_pfDwmDefWindowProc = (BCGP_DWMDEFWINDOWPROC) ::GetProcAddress (m_hinstDwmapiDLL, "DwmDefWindowProc");
		m_pfDwmIsCompositionEnabled = (BCGP_DWMISCOMPOSITIONENABLED)::GetProcAddress (m_hinstDwmapiDLL, "DwmIsCompositionEnabled");
		m_pfDwmSetIconicThumbnail = (BCGP_DWMSETICONICTHUMBNAIL) ::GetProcAddress(m_hinstDwmapiDLL, "DwmSetIconicThumbnail");
		m_pfDwmSetWindowAttribute = (BCGP_DWMSETWINDOWATTRIBUTE) ::GetProcAddress(m_hinstDwmapiDLL, "DwmSetWindowAttribute");
		m_pfDwmSetIconicLivePreviewBitmap = (BCGP_DWMSETICONICLIVEPRBMP) ::GetProcAddress(m_hinstDwmapiDLL, "DwmSetIconicLivePreviewBitmap");
		m_pfDwmInvalidateIconicBitmaps = (BCGP_DWMINVALIDATEICONICBITMAPS) ::GetProcAddress(m_hinstDwmapiDLL, "DwmInvalidateIconicBitmaps");
	}
	else
	{
		m_pfDwmExtendFrameIntoClientArea = NULL;
		m_pfDwmDefWindowProc = NULL;
		m_pfDwmIsCompositionEnabled = NULL;
		m_pfDwmSetIconicThumbnail = NULL;		
		m_pfDwmSetWindowAttribute = NULL;
		m_pfDwmSetIconicLivePreviewBitmap = NULL;
		m_pfDwmInvalidateIconicBitmaps = NULL;
	}
}
//************************************************************************************
void BCGPGLOBAL_DATA::UpdateFonts()
{
	CWindowDC dc (NULL);
	m_dblRibbonImageScale = dc.GetDeviceCaps(LOGPIXELSX) / 96.0f;

	if (m_dblRibbonImageScale > 1. && m_dblRibbonImageScale < 1.1)
	{
		m_dblRibbonImageScale = 1.;
	}

	if (fontRegular.GetSafeHandle () != NULL)
	{
		::DeleteObject (fontRegular.Detach ());
	}

	if (fontTooltip.GetSafeHandle () != NULL)
	{
		::DeleteObject (fontTooltip.Detach ());
	}

	if (fontBold.GetSafeHandle () != NULL)
	{
		::DeleteObject (fontBold.Detach ());
	}

	if (fontDefaultGUIBold.GetSafeHandle () != NULL)
	{
		::DeleteObject (fontDefaultGUIBold.Detach ());
	}

	if (fontUnderline.GetSafeHandle () != NULL)
	{
		::DeleteObject (fontUnderline.Detach ());
	}

	if (fontDefaultGUIUnderline.GetSafeHandle () != NULL)
	{
		::DeleteObject (fontDefaultGUIUnderline.Detach ());
	}

	if (fontVert.GetSafeHandle () != NULL)
	{
		::DeleteObject (fontVert.Detach ());
	}

	if (fontVertCaption.GetSafeHandle () != NULL)
	{
		::DeleteObject (fontVertCaption.Detach ());
	}

	if (fontMarlett.GetSafeHandle () != NULL)
	{
		::DeleteObject (fontMarlett.Detach ());
	}

	if (fontSmall.GetSafeHandle () != NULL)
	{
		::DeleteObject (fontSmall.Detach ());
	}

	if (fontCaption.GetSafeHandle() != NULL)
	{
		::DeleteObject(fontCaption.Detach());
	}

	if (fontGroup.GetSafeHandle() != NULL)
	{
		::DeleteObject(fontGroup.Detach());
	}

	//------------------
	// Initialize fonts:
	//------------------
	NONCLIENTMETRICS info;
	GetNonClientMetrics (info);

	LOGFONT lf;
	memset (&lf, 0, sizeof (LOGFONT));

	lf.lfCharSet = (BYTE) GetTextCharsetInfo (dc.GetSafeHdc (), NULL, 0);

	lf.lfHeight = info.lfMenuFont.lfHeight;
	lf.lfWeight = info.lfMenuFont.lfWeight;
	lf.lfItalic = info.lfMenuFont.lfItalic;

	//------------------
	// Adjust font size:
	//------------------
	int nFontHeight = lf.lfHeight < 0 ? -lf.lfHeight : lf.lfHeight;
	if (nFontHeight <= 12)
	{
		nFontHeight = 11;
	}
	else
	{
		nFontHeight--;
	}

	lf.lfHeight = (lf.lfHeight < 0) ? -nFontHeight : nFontHeight;

	lstrcpy (lf.lfFaceName, info.lfMenuFont.lfFaceName);

	//-------------------------------------
	// 	Check if we should use system font:
	//-------------------------------------
	BOOL fUseSystemFont = m_bUseSystemFont || (info.lfMenuFont.lfCharSet > SYMBOL_CHARSET);
	if (!fUseSystemFont)
	{
		//----------------------------------
		// Check for Office font existance:
		//----------------------------------
		if (::EnumFontFamilies (dc.GetSafeHdc (), NULL, FontFamalyProcFonts, 
			(LPARAM)(LPCTSTR) strOffice2007FontName) == 0)
		{
			//--------------------------------
			// Found! Use MS Office 2007 font!
			//--------------------------------
			lstrcpy (lf.lfFaceName, strOffice2007FontName);
			lf.lfQuality = 5 /*CLEARTYPE_QUALITY*/;
		}
		else if (::EnumFontFamilies (dc.GetSafeHdc (), NULL, FontFamalyProcFonts, 
			(LPARAM)(LPCTSTR) strOfficeFontName) == 0)
		{
			//---------------------------
			// Found! Use MS Office font!
			//---------------------------
			lstrcpy (lf.lfFaceName, strOfficeFontName);
		}
		else
		{
			//-----------------------------
			// Not found. Use default font:
			//-----------------------------
			lstrcpy (lf.lfFaceName, strDefaultFontName);
		}
	}

	fontRegular.CreateFontIndirect (&lf);

	//-------------------
	// Create small font:
	//-------------------
	LONG lfHeightSaved = lf.lfHeight;

	lf.lfHeight = (long) ((1. + abs (lf.lfHeight)) * 2 / 3);
	if (lfHeightSaved < 0)
	{
		lf.lfHeight = -lf.lfHeight;
	}

	fontSmall.CreateFontIndirect (&lf);

	lf.lfHeight = lfHeightSaved;

	//---------------------
	// Create caption font:
	//---------------------
	lf.lfHeight = (long) ((2. + abs (lf.lfHeight)) * 3 / 2);
	if (lfHeightSaved < 0)
	{
		lf.lfHeight = -lf.lfHeight;
	}

	fontCaption.CreateFontIndirect (&lf);

	lf.lfHeight = lfHeightSaved;

	//-------------------
	// Create group font:
	//-------------------
	lf.lfHeight = (long) ((abs (lf.lfHeight)) * 5 / 4);
	if (lfHeightSaved < 0)
	{
		lf.lfHeight = -lf.lfHeight;
	}
	
	fontGroup.CreateFontIndirect (&lf);

	lf.lfHeight = lfHeightSaved;

	//---------------------
	// Create tooltip font:
	//---------------------
	NONCLIENTMETRICS ncm;
	GetNonClientMetrics (ncm);

	lf.lfItalic = ncm.lfStatusFont.lfItalic;
	lf.lfWeight = ncm.lfStatusFont.lfWeight;
	fontTooltip.CreateFontIndirect (&lf);

	lf.lfItalic = info.lfMenuFont.lfItalic;
	lf.lfWeight = info.lfMenuFont.lfWeight;

	//-------------------------
	// Create "underline" font:
	//-------------------------
	lf.lfUnderline = TRUE;
	fontUnderline.CreateFontIndirect (&lf);
	lf.lfUnderline = FALSE;

	//------------------
	// Create bold font:
	//------------------
	lf.lfWeight = FW_BOLD;
	fontBold.CreateFontIndirect (&lf);

	//---------------------
	// Create Marlett font:
	//---------------------
	BYTE bCharSet = lf.lfCharSet;
	lf.lfWeight = info.lfMenuFont.lfWeight;
	lf.lfCharSet = SYMBOL_CHARSET;
	lf.lfWeight = 0;
	lf.lfHeight = ::GetSystemMetrics (SM_CYMENUCHECK) - 1;
	lstrcpy (lf.lfFaceName, strMarlettFontName);

	fontMarlett.CreateFontIndirect (&lf);

	lf.lfCharSet = bCharSet;	// Restore charset

	//----------------------
	// Create vertical font:
	//----------------------
	CFont font;
	if (font.CreateStockObject (DEFAULT_GUI_FONT))
	{
		if (font.GetLogFont (&lf) != 0)
		{
			lf.lfOrientation = 900;
			lf.lfEscapement = 2700;

			lf.lfHeight = info.lfMenuFont.lfHeight;
			lf.lfWeight = info.lfMenuFont.lfWeight;
			lf.lfItalic = info.lfMenuFont.lfItalic;
			
			lstrcpy (lf.lfFaceName, strVertFontName);

			fontVert.CreateFontIndirect (&lf);

			lf.lfEscapement = 900;
			fontVertCaption.CreateFontIndirect (&lf);
		}
	}

	//----------------------------------------
	// Create dialog underline and bold fonts:
	//----------------------------------------
	CFont* pDefaultGUIFont = CFont::FromHandle ((HFONT) GetStockObject (DEFAULT_GUI_FONT));
	ASSERT_VALID (pDefaultGUIFont);
	pDefaultGUIFont->GetLogFont (&lf);

	lf.lfUnderline = TRUE;
	fontDefaultGUIUnderline.CreateFontIndirect (&lf);
	lf.lfUnderline = FALSE;

	lf.lfWeight = FW_BOLD;
	fontDefaultGUIBold.CreateFontIndirect (&lf);

	UpdateTextMetrics();

#if (!defined _BCGPCALENDAR_STANDALONE) && !(defined _BCGPGRID_STANDALONE) && !(defined _BCGPEDIT_STANDALONE) && !(defined _BCGPCHART_STANDALONE)
	//-------------------------------------
	// Notify toolbars about font changing:
	//-------------------------------------
	for (POSITION posTlb = gAllToolbars.GetHeadPosition (); posTlb != NULL;)
	{
		CBCGPToolBar* pToolBar = (CBCGPToolBar*) gAllToolbars.GetNext (posTlb);
		ASSERT (pToolBar != NULL);

		if (CWnd::FromHandlePermanent (pToolBar->m_hWnd) != NULL)
		{
			ASSERT_VALID(pToolBar);
			pToolBar->OnGlobalFontsChanged ();
		}
	}
#endif
}
//*******************************************************************************************
const CStringList& BCGPGLOBAL_DATA::GetLocaleList() const
{
	if (m_lstLocales.IsEmpty() && bIsWindowsVista && g_pfLCIDToLocaleName == NULL)
	{
		HINSTANCE hinstKernel32 = ::LoadLibrary(_T("KERNEL32.DLL"));
		if (hinstKernel32 != NULL)
		{
			g_pfLCIDToLocaleName = (BCGP_LCIDTOLOCALENAME)::GetProcAddress (hinstKernel32, "LCIDToLocaleName");
			if (g_pfLCIDToLocaleName != NULL)
			{
				EnumSystemLocales(&EnumLocalesProc, LCID_INSTALLED);
			}

			g_pfLCIDToLocaleName = NULL;

			::FreeLibrary (hinstKernel32);
		}
	}

	return m_lstLocales;
}
//*******************************************************************************************
CString BCGPGLOBAL_DATA::GetLocaleName(const CString& strLocale)
{
	CString strName;
	m_mapLocaleNames.Lookup(strLocale, strName);

	return strName;
}
//*******************************************************************************************
CString BCGPGLOBAL_DATA::GetLocaleByName(const CString& strNameIn)
{
	for (POSITION pos = m_mapLocaleNames.GetStartPosition(); pos != NULL;)
	{
		CString strLocale;
		CString strName;

		m_mapLocaleNames.GetNextAssoc(pos, strLocale, strName);

		if (strName.CompareNoCase(strNameIn) == 0)
		{
			return strLocale;
		}
	}

	return _T("");
}
//*******************************************************************************************
void BCGPGLOBAL_DATA::AddLocale(LPCTSTR lpszLocale, LPCTSTR lpszName)
{
	BOOL bInserted = FALSE;

	for (POSITION pos = m_lstLocales.GetHeadPosition (); pos != NULL;)
	{
		POSITION posStored = pos;
		if (m_lstLocales.GetNext(pos).Compare(lpszLocale) > 0)
		{
			m_lstLocales.InsertBefore (posStored, lpszLocale);
			bInserted = TRUE;
			break;
		}
	}

	if (!bInserted)
	{
		m_lstLocales.AddTail(lpszLocale);
	}

	m_mapLocaleNames.SetAt(lpszLocale, lpszName);
}
//*******************************************************************************************
static BOOL CALLBACK InfoEnumProc (	HMONITOR hMonitor, HDC /*hdcMonitor*/,
										LPRECT /*lprcMonitor*/, LPARAM dwData)
{
	CRect* pRect = (CRect*) dwData;

	MONITORINFO mi;
	mi.cbSize = sizeof (MONITORINFO);

	if (GetMonitorInfo (hMonitor, &mi))
	{
		CRect rectMon = mi.rcWork;

		pRect->left = min (pRect->left, rectMon.left);
		pRect->right = max (pRect->right, rectMon.right);
		pRect->top = min (pRect->top, rectMon.top);
		pRect->bottom = max (pRect->bottom, rectMon.bottom);
	}

	return TRUE;
}
//*******************************************************************************************
void BCGPGLOBAL_DATA::OnSettingChange ()
{
	m_bInSettingsChange = TRUE;
	m_bShellAutohideBarsInitialized = FALSE;

	m_sizeSmallIcon.cx	= ::GetSystemMetrics (SM_CXSMICON);
	m_sizeSmallIcon.cy	= ::GetSystemMetrics (SM_CYSMICON);

	m_rectVirtual.SetRectEmpty ();

	if (!EnumDisplayMonitors (NULL, NULL, InfoEnumProc, (LPARAM) &m_rectVirtual))
	{
		::SystemParametersInfo (SPI_GETWORKAREA, 0, &m_rectVirtual, 0);
	}

	m_hiconApp = LoadIcon(NULL, IDI_APPLICATION);

	//--------------------------------
	// Get system menu animation type:
	//--------------------------------
	m_bMenuAnimation = FALSE;
	m_bMenuFadeEffect = FALSE;

	if (!bIsRemoteSession && bIsWindows2000)
	{
		::SystemParametersInfo (SPI_GETMENUANIMATION, 
								0, &m_bMenuAnimation, 0);

		if (m_bMenuAnimation)
		{
			::SystemParametersInfo (SPI_GETMENUFADE,
									0, &m_bMenuFadeEffect, 0);
		}
	}

	if (!bIsWindows9x)
	{
		::SystemParametersInfo (SPI_GETMENUUNDERLINES, 0, &m_bSysUnderlineKeyboardShortcuts, 0);
	}
	else
	{
		m_bSysUnderlineKeyboardShortcuts = TRUE;
	}

	m_bUnderlineKeyboardShortcuts = m_bSysUnderlineKeyboardShortcuts;

	m_bInSettingsChange = FALSE;
}
//*******************************************************************************************
void BCGPGLOBAL_DATA::UpdateSysColors()
{
	m_bIsBlackHighContrast = 
		::GetSysColor (COLOR_3DLIGHT) == RGB (255, 255, 255) &&
		::GetSysColor (COLOR_3DFACE) == RGB (0, 0, 0);

	m_bIsWhiteHighContrast = 
		::GetSysColor (COLOR_3DDKSHADOW) == RGB (0, 0, 0) &&
		::GetSysColor (COLOR_3DFACE) == RGB (255, 255, 255);

	CWindowDC dc (NULL);
	m_nBitsPerPixel = dc.GetDeviceCaps (BITSPIXEL);

	clrBarFace = clrBtnFace = ::GetSysColor(COLOR_BTNFACE);
	clrBarShadow = clrBtnShadow = ::GetSysColor(COLOR_BTNSHADOW);
	clrBarDkShadow = clrBtnDkShadow = ::GetSysColor(COLOR_3DDKSHADOW);
	clrBarLight = clrBtnLight = ::GetSysColor(COLOR_3DLIGHT);
	clrBarHilite = clrBtnHilite = ::GetSysColor(COLOR_BTNHIGHLIGHT);
	clrBarText = clrBtnText = ::GetSysColor(COLOR_BTNTEXT);
	clrGrayedText = ::GetSysColor (COLOR_GRAYTEXT);
	clrWindowFrame = ::GetSysColor(COLOR_WINDOWFRAME);

	clrHilite = ::GetSysColor(COLOR_HIGHLIGHT);
	clrTextHilite = ::GetSysColor(COLOR_HIGHLIGHTTEXT);

	clrBarWindow = clrWindow = ::GetSysColor (COLOR_WINDOW);
	clrWindowText = ::GetSysColor (COLOR_WINDOWTEXT);

#if (!defined _BCGPCALENDAR_STANDALONE) && !(defined _BCGPGRID_STANDALONE) && !(defined _BCGPEDIT_STANDALONE) && !(defined _BCGPCHART_STANDALONE)
	if (m_nBitsPerPixel > 8)
	{
		clrPrompt = CBCGPDrawManager::ColorMakeLighter(clrWindowText, .45);
	}
	else
#endif
	{
		clrPrompt = clrGrayedText;
	}

	clrCaptionText = ::GetSysColor (COLOR_CAPTIONTEXT);
	clrMenuText = ::GetSysColor (COLOR_MENUTEXT);

	clrActiveCaption = ::GetSysColor (COLOR_ACTIVECAPTION);
	clrInactiveCaption = ::GetSysColor (COLOR_INACTIVECAPTION);

	if (bIsWindowsNT4 || bIsWindows9x)
	{
		clrActiveCaptionGradient = clrActiveCaption;
		clrInactiveCaptionGradient = clrInactiveCaption;
	}
	else
	{
		clrActiveCaptionGradient = ::GetSysColor (27 /*COLOR_GRADIENTACTIVECAPTION*/);
		clrInactiveCaptionGradient = ::GetSysColor (28 /*COLOR_GRADIENTINACTIVECAPTION*/);
	}

	clrActiveBorder = ::GetSysColor (COLOR_ACTIVEBORDER);
	clrInactiveBorder = ::GetSysColor (COLOR_INACTIVEBORDER);

	clrInactiveCaptionText = ::GetSysColor (COLOR_INACTIVECAPTIONTEXT);

	if (m_bIsBlackHighContrast)
	{
		clrHotText = clrWindowText;
		clrHotLinkText = clrWindowText;
	}
	else
	{
		if (bIsWindows2000)
		{
			clrHotText = ::GetSysColor (26 /*COLOR_HOTLIGHT*/);
			clrHotLinkText = RGB (0, 0, 255);	// Light blue
		}
		else
		{
			clrHotText = RGB (0, 0, 255);		// Light blue
			clrHotLinkText = RGB (255, 0, 255);	// Violet
		}
	}

	hbrBtnShadow = ::GetSysColorBrush (COLOR_BTNSHADOW);
	ASSERT(hbrBtnShadow != NULL);

	hbrBtnHilite = ::GetSysColorBrush (COLOR_BTNHIGHLIGHT);
	ASSERT(hbrBtnHilite != NULL);

	hbrWindow = ::GetSysColorBrush (COLOR_WINDOW);
	ASSERT(hbrWindow != NULL);

	brBtnFace.DeleteObject ();
	brBtnFace.CreateSolidBrush (clrBtnFace);

	brBarFace.DeleteObject ();
	brBarFace.CreateSolidBrush (clrBarFace);

	brActiveCaption.DeleteObject ();
	brActiveCaption.CreateSolidBrush (clrActiveCaption);

	brInactiveCaption.DeleteObject ();
	brInactiveCaption.CreateSolidBrush (clrInactiveCaption);

	brHilite.DeleteObject ();
	brHilite.CreateSolidBrush (clrHilite);

	brBlack.DeleteObject ();
	brBlack.CreateSolidBrush (clrBtnDkShadow);

	brWindow.DeleteObject ();
	brWindow.CreateSolidBrush (clrWindow);

	penHilite.DeleteObject ();
	penHilite.CreatePen (PS_SOLID, 1, globalData.clrHilite);

	penBarFace.DeleteObject ();
	penBarFace.CreatePen (PS_SOLID, 1, globalData.clrBarFace);

	penBarShadow.DeleteObject ();
	penBarShadow.CreatePen (PS_SOLID, 1, globalData.clrBarShadow);

	if (brLight.GetSafeHandle ())
	{
		brLight.DeleteObject ();
	}
	
	if (m_nBitsPerPixel > 8)
	{
		COLORREF clrLight = RGB (
			GetRValue(clrBtnFace) + ((GetRValue(clrBtnHilite) -
				GetRValue(clrBtnFace)) / 2 ),
			GetGValue(clrBtnFace) + ((GetGValue(clrBtnHilite) -
				GetGValue(clrBtnFace)) / 2),
			GetBValue(clrBtnFace) + ((GetBValue(clrBtnHilite) -
				GetBValue(clrBtnFace)) / 2)
			);

		brLight.CreateSolidBrush (clrLight);
	}
	else
	{
		HBITMAP hbmGray = CreateDitherBitmap (dc.GetSafeHdc ());
		ASSERT (hbmGray != NULL);

		CBitmap bmp;
		bmp.Attach (hbmGray);

		brLight.CreatePatternBrush (&bmp);
	}

#if (!defined _BCGPCALENDAR_STANDALONE) && !(defined _BCGPGRID_STANDALONE) && !(defined _BCGPEDIT_STANDALONE) && !(defined _BCGPCHART_STANDALONE)
	CBCGPMenuImages::CleanUp ();
	CBCGPDockManager::m_bSDParamsModified = TRUE;
#endif
}
//************************************************************************************
BOOL BCGPGLOBAL_DATA::SetMenuFont (LPLOGFONT lpLogFont, BOOL bHorz)
{
	ASSERT (lpLogFont != NULL);

	if (bHorz)
	{
		//---------------------
		// Create regular font:
		//---------------------
		fontRegular.DeleteObject ();
		if (!fontRegular.CreateFontIndirect (lpLogFont))
		{
			ASSERT (FALSE);
			return FALSE;
		}

		//-----------------------
		// Create underline font:
		//-----------------------
		lpLogFont->lfUnderline = TRUE;
		fontUnderline.DeleteObject ();
		fontUnderline.CreateFontIndirect (lpLogFont);
		lpLogFont->lfUnderline = FALSE;

		//---------------------------------------------------
		// Create bold font (used in the default menu items):
		//---------------------------------------------------
		long lSavedWeight = lpLogFont->lfWeight;
		lpLogFont->lfWeight = 700;

		fontBold.DeleteObject ();
		BOOL bResult = fontBold.CreateFontIndirect (lpLogFont);

		lpLogFont->lfWeight = lSavedWeight;	// Restore weight

		if (!bResult)
		{
			ASSERT (FALSE);
			return FALSE;
		}
	}
	else	// Vertical font
	{
		fontVert.DeleteObject ();
		if (!fontVert.CreateFontIndirect (lpLogFont))
		{
			ASSERT (FALSE);
			return FALSE;
		}
	}

	UpdateTextMetrics();
	return TRUE;
}
//************************************************************************************
void BCGPGLOBAL_DATA::UpdateTextMetrics ()
{
	CWindowDC dc (NULL);

	CFont* pOldFont = dc.SelectObject (&fontRegular);
	ASSERT (pOldFont != NULL);

	TEXTMETRIC tm;
	dc.GetTextMetrics (&tm);

	m_nTextMarginsHorz = tm.tmHeight < 15 ? 2 : 5;
	m_nTextHeightHorz = tm.tmHeight + m_nTextMarginsHorz;
	m_nTextWidthHorz = tm.tmMaxCharWidth + m_nTextMarginsHorz;

	dc.SelectObject (&fontVert);
	dc.GetTextMetrics (&tm);

	m_nTextMarginsVert = tm.tmHeight < 15 ? 2 : 5;

	m_nTextHeightVert = tm.tmHeight + m_nTextMarginsVert;
	m_nTextWidthVert = tm.tmMaxCharWidth + m_nTextMarginsVert;

	dc.SelectObject (&fontCaption);
	dc.GetTextMetrics (&tm);
	
	m_nCaptionTextHeight = tm.tmHeight + m_nTextMarginsHorz;
	m_nCaptionTextWidth = tm.tmMaxCharWidth + m_nTextMarginsHorz;

	dc.SelectObject (pOldFont);
}
//*******************************************************************************
HBITMAP BCGPGLOBAL_DATA::CreateDitherBitmap (HDC hDC)
{
	struct  // BITMAPINFO with 16 colors
	{
		BITMAPINFOHEADER bmiHeader;
		RGBQUAD      bmiColors[16];
	} 
	bmi;
	memset(&bmi, 0, sizeof(bmi));

	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = 8;
	bmi.bmiHeader.biHeight = 8;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 1;
	bmi.bmiHeader.biCompression = BI_RGB;

	COLORREF clr = globalData.clrBtnFace;

	bmi.bmiColors[0].rgbBlue = GetBValue(clr);
	bmi.bmiColors[0].rgbGreen = GetGValue(clr);
	bmi.bmiColors[0].rgbRed = GetRValue(clr);

	clr = ::GetSysColor(COLOR_BTNHIGHLIGHT);
	bmi.bmiColors[1].rgbBlue = GetBValue(clr);
	bmi.bmiColors[1].rgbGreen = GetGValue(clr);
	bmi.bmiColors[1].rgbRed = GetRValue(clr);

	// initialize the brushes
	long patGray[8];
	for (int i = 0; i < 8; i++)
	   patGray[i] = (i & 1) ? 0xAAAA5555L : 0x5555AAAAL;

	HBITMAP hbm = CreateDIBitmap(hDC, &bmi.bmiHeader, CBM_INIT,
		(LPBYTE)patGray, (LPBITMAPINFO)&bmi, DIB_RGB_COLORS);
	return hbm;
}
//*************************************************************************************
void BCGPGLOBAL_DATA::CleanUp ()
{
	if (brLight.GetSafeHandle ())
	{
		brLight.DeleteObject ();
	}
	
	// cleanup fonts:
	fontRegular.DeleteObject ();
	fontBold.DeleteObject ();
	fontUnderline.DeleteObject ();
	fontVert.DeleteObject ();
	fontVertCaption.DeleteObject ();
	fontTooltip.DeleteObject ();

	if (m_hinstUXThemeDLL != NULL)
	{
		::FreeLibrary (m_hinstUXThemeDLL);
		m_hinstUXThemeDLL = NULL;
	}

	if (m_hinstUser32 != NULL)
	{
		::FreeLibrary (m_hinstUser32);
		m_hinstUser32 = NULL;
	}

	if (m_hinstDwmapiDLL != NULL)
	{
		::FreeLibrary (m_hinstDwmapiDLL);
		m_hinstDwmapiDLL = NULL;
	}

	m_lstLocales.RemoveAll();
	m_mapLocaleNames.RemoveAll();
	
#if (!defined _BCGPCALENDAR_STANDALONE) && !(defined _BCGPGRID_STANDALONE) && !(defined _BCGPEDIT_STANDALONE) && !(defined _BCGPCHART_STANDALONE)
	m_pfSetLayeredWindowAttributes = NULL;
	m_pfUpdateLayeredWindow = NULL;
	m_pfNotifyWinEvent = NULL;
#endif

	ReleaseTaskbarList3();

#if (!defined _BCGPCALENDAR_STANDALONE) && !(defined _BCGPGRID_STANDALONE) && !(defined _BCGPEDIT_STANDALONE)
	CBCGPGraphicsManagerD2D::ReleaseD2DRefs();
#endif

	if (m_bComInitialized)
	{
		CoUninitialize();
		m_bComInitialized = FALSE;
	}
}
//*************************************************************************************
BCGCBPRODLLEXPORT void BCGCBProCleanUp ()
{
	globalData.CleanUp ();

#if (!defined _BCGPCALENDAR_STANDALONE) && !(defined _BCGPGRID_STANDALONE) && !(defined _BCGPEDIT_STANDALONE) && !(defined _BCGPCHART_STANDALONE)
	g_menuHash.CleanUp ();

	CBCGPToolBar::CleanUpImages ();
	CBCGPMenuImages::CleanUp ();

	if (BCGPGetCmdMgr () != NULL)
	{
		BCGPGetCmdMgr ()->CleanUp ();
	}

	CBCGPKeyboardManager::CleanUp ();

	// Destroy visualization manager:
	CBCGPVisualManager::DestroyInstance (TRUE /* bAutoDestroyOnly */);
	CBCGPVisualManager2007::CleanStyle ();
#endif
}
//****************************************************************************************
BOOL BCGPGLOBAL_DATA::DrawParentBackground (CWnd* pWnd, CDC* pDC, LPRECT rectClip)
{
	if (pWnd->GetSafeHwnd() == NULL)
	{
		return FALSE;
	}

	ASSERT_VALID (pDC);
	ASSERT_VALID (pWnd);

	BOOL bRes = FALSE;

	CRgn rgn;
	if (rectClip != NULL)
	{
		rgn.CreateRectRgnIndirect (rectClip);
		pDC->SelectClipRgn (&rgn);
	}

	CWnd* pParent = pWnd->GetParent ();
	ASSERT_VALID (pParent);

	// In Windows XP, we need to call DrawThemeParentBackground function to implement
	// transparent controls
	if (m_pfDrawThemeBackground != NULL)
	{
		bRes = (*m_pfDrawThemeBackground) (pWnd->GetSafeHwnd (), 
			pDC->GetSafeHdc (), rectClip) == S_OK;
	}

	if (!bRes)
	{
		CPoint pt (0, 0);
		pWnd->MapWindowPoints (pParent, &pt, 1);
		pt = pDC->OffsetWindowOrg (pt.x, pt.y);

		bRes = (BOOL) pParent->SendMessage (WM_ERASEBKGND, (WPARAM)pDC->m_hDC);

		pDC->SetWindowOrg(pt.x, pt.y);
	}

	pDC->SelectClipRgn (NULL);

	return bRes;
}
//****************************************************************************************
BOOL BCGPGLOBAL_DATA::SetWindowTheme(CWnd* pWnd, LPCWSTR pszSubAppName, LPCWSTR pszSubIdList)
{
	if (pWnd->GetSafeHwnd() == NULL || m_pfSetWindowTheme == NULL)
	{
		return FALSE;
	}

	HRESULT hr = (*m_pfSetWindowTheme)(pWnd->GetSafeHwnd(), pszSubAppName, pszSubIdList);

	return SUCCEEDED(hr);
}
//****************************************************************************************
HBITMAP BCGPGLOBAL_DATA::GetThemeBitmap(HTHEME hTheme, int iPartId, int iStateId, int iPropId, ULONG dwFlags)
{
	if (hTheme == NULL || m_pfGetThemeBitmap == NULL)
	{
		return NULL;
	}

	HBITMAP hBitmap = NULL;
	HRESULT hr = (*m_pfGetThemeBitmap)(hTheme, iPartId, iStateId, iPropId, dwFlags, &hBitmap);

	if (SUCCEEDED(hr))
	{
		return hBitmap;
	}

	return NULL;
}

#if (!defined _BCGPCALENDAR_STANDALONE) && !(defined _BCGPGRID_STANDALONE) && !(defined _BCGPEDIT_STANDALONE) && !(defined _BCGPCHART_STANDALONE)

BCGCBPRODLLEXPORT CFrameWnd* BCGPGetParentFrame (const CWnd* pWnd)
{
	if (pWnd->GetSafeHwnd () == NULL)
	{
		return NULL;
	}
	ASSERT_VALID (pWnd);

	const CWnd* pParentWnd = pWnd;

	while (pParentWnd != NULL)
	{
		if (pParentWnd->IsKindOf (RUNTIME_CLASS (CBCGPMiniFrameWnd)))
		{
			CBCGPMiniFrameWnd* pMiniFrame = DYNAMIC_DOWNCAST (CBCGPMiniFrameWnd, pParentWnd);
			pParentWnd = pMiniFrame->GetParent ();
		}
		else
		{
			pParentWnd = pParentWnd->GetParent ();
		}

		if (pParentWnd == NULL)
		{
			return NULL;
		}
		if (pParentWnd->IsFrameWnd ())
		{
			return (CFrameWnd*)pParentWnd;
		}
	}

	return NULL;
}

#endif

COLORREF BCGPGLOBAL_DATA::GetColor (int nColor)
{
	switch (nColor)
	{
	case COLOR_BTNFACE:             return clrBtnFace;
	case COLOR_BTNSHADOW:           return clrBtnShadow;			
	case COLOR_3DDKSHADOW:          return clrBtnDkShadow;			
	case COLOR_3DLIGHT:             return clrBtnLight;				
	case COLOR_BTNHIGHLIGHT:        return clrBtnHilite;			
	case COLOR_BTNTEXT:             return clrBtnText;				
	case COLOR_GRAYTEXT:            return clrGrayedText;			
	case COLOR_WINDOWFRAME:         return clrWindowFrame;			
	                          
	case COLOR_HIGHLIGHT:           return clrHilite;				
	case COLOR_HIGHLIGHTTEXT:       return clrTextHilite;			
	                          
	case COLOR_WINDOW:				return clrWindow;				
	case COLOR_WINDOWTEXT:			return clrWindowText;			
	                          
	case COLOR_CAPTIONTEXT:			return clrCaptionText;			
	case COLOR_MENUTEXT:			return clrMenuText;				
	                          
	case COLOR_ACTIVECAPTION:		return clrActiveCaption;		
	case COLOR_INACTIVECAPTION:		return clrInactiveCaption;		
	                          
	case COLOR_ACTIVEBORDER:		return clrActiveBorder;			
	case COLOR_INACTIVEBORDER:		return clrInactiveBorder;
	                          
	case COLOR_INACTIVECAPTIONTEXT:	return clrInactiveCaptionText;
	}

	return ::GetSysColor (nColor);
}
//******************************************************************************
BOOL BCGPGLOBAL_DATA::SetLayeredAttrib (HWND hwnd, COLORREF crKey, BYTE bAlpha, DWORD dwFlags)
{
	if (m_pfSetLayeredWindowAttributes == NULL)
	{
		return FALSE;
	}

	return (*m_pfSetLayeredWindowAttributes) (hwnd, crKey, bAlpha, dwFlags);
}
//******************************************************************************
BOOL BCGPGLOBAL_DATA::UpdateLayeredWindow (HWND hwnd, HDC hdcDst, POINT *pptDst, SIZE *psize, HDC hdcSrc,
							POINT *pptSrc, COLORREF crKey, BLENDFUNCTION *pblend, DWORD dwFlags)
{
	if (m_pfUpdateLayeredWindow == NULL)
	{
		return FALSE;
	}

	return (*m_pfUpdateLayeredWindow) (hwnd, hdcDst, pptDst, psize, hdcSrc, pptSrc, crKey, pblend, dwFlags);
}

#if (!defined _BCGPCALENDAR_STANDALONE) && !(defined _BCGPGRID_STANDALONE) && !(defined _BCGPEDIT_STANDALONE)  && !(defined _BCGPCHART_STANDALONE)

void BCGPGLOBAL_DATA::EnableAccessibilitySupport (BOOL bEnable/* = TRUE*/)
{
	if (!bIsWindows2000)
	{
		return;
	}

	if (bEnable)
	{
		if (m_hinstUser32 == NULL)
		{
			m_pfNotifyWinEvent = NULL;
			ASSERT (FALSE);
			return;
		}

		m_pfNotifyWinEvent = (NOTIFYWINEVENT)GetProcAddress (m_hinstUser32, "NotifyWinEvent");
		ASSERT (m_pfNotifyWinEvent != NULL);
	}

	m_bEnableAccessibility = bEnable;
}
//**********************************************************************************
BOOL BCGPGLOBAL_DATA::NotifyWinEvent (DWORD event, HWND hwnd, LONG idObject, LONG idChild)
{
	if (m_pfNotifyWinEvent == NULL)
	{
		return FALSE;
	}

	return (*m_pfNotifyWinEvent) (event, hwnd, idObject, idChild);
}

#endif

CString BCGPGLOBAL_DATA::RegisterWindowClass (LPCTSTR lpszClassNamePrefix)
{
	ASSERT (lpszClassNamePrefix != NULL);

	//-----------------------------
	// Register a new window class:
	//-----------------------------
	HINSTANCE hInst = AfxGetInstanceHandle();
	UINT uiClassStyle = CS_DBLCLKS;
	HCURSOR hCursor = ::LoadCursor (NULL, IDC_ARROW);
	HBRUSH hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);

	CString strClassName;
	strClassName.Format (_T("%s:%x:%x:%x:%x"), 
		lpszClassNamePrefix,
		(UINT_PTR)hInst, uiClassStyle, (UINT_PTR)hCursor, (UINT_PTR)hbrBackground);

	//---------------------------------
	// See if the class already exists:
	//---------------------------------
	WNDCLASS wndcls;
	if (::GetClassInfo (hInst, strClassName, &wndcls))
	{
		//-----------------------------------------------
		// Already registered, assert everything is good:
		//-----------------------------------------------
		ASSERT (wndcls.style == uiClassStyle);
	}
	else
	{
		//-------------------------------------------
		// Otherwise we need to register a new class:
		//-------------------------------------------
		wndcls.style = uiClassStyle;
		wndcls.lpfnWndProc = ::DefWindowProc;
		wndcls.cbClsExtra = wndcls.cbWndExtra = 0;
		wndcls.hInstance = hInst;
		wndcls.hIcon = NULL;
		wndcls.hCursor = hCursor;
		wndcls.hbrBackground = hbrBackground;
		wndcls.lpszMenuName = NULL;
		wndcls.lpszClassName = strClassName;
		
		if (!AfxRegisterClass (&wndcls))
		{
			AfxThrowResourceException();
		}
	}

	return strClassName;
}
//*************************************************************************************
BOOL BCGPGLOBAL_DATA::ExcludeTag (CString& strBuffer, 
						LPCTSTR lpszTag,
						CString& strTag, BOOL bIsCharsList /* = FALSE*/)
{
	const int iBufLen = strBuffer.GetLength ();

	CString strTagStart = _T("<");
	strTagStart += lpszTag;
	strTagStart += _T(">");

	const int iTagStartLen = strTagStart.GetLength ();

	int iStart = -1;

	int iIndexStart = strBuffer.Find (strTagStart);
	if (iIndexStart < 0)
	{
		return FALSE;
	}

	iStart = iIndexStart + iTagStartLen;

	CString strTagEnd = _T("</");
	strTagEnd += lpszTag;
	strTagEnd += _T(">");

	const int iTagEndLen = strTagEnd.GetLength ();

	int iIndexEnd =  -1;
	int nBalanse = 1;
	for (int i = iStart; i < iBufLen - iTagEndLen + 1; i ++)
	{
		if (strBuffer [i] != (TCHAR)'<')
		{
			continue;
		}

		if (i < iBufLen - iTagStartLen &&
			_tcsncmp (strBuffer.Mid (i), strTagStart, iTagStartLen) == 0)
		{
			i += iTagStartLen - 1;
			nBalanse ++;
			continue;
		}

		if (_tcsncmp (strBuffer.Mid (i), strTagEnd, iTagEndLen) == 0)
		{
			nBalanse --;
			if (nBalanse == 0)
			{
				iIndexEnd = i;
				break;
			}

			i += iTagEndLen - 1;
		}
	}

	if (iIndexEnd == -1 || iStart > iIndexEnd)
	{
		return FALSE;
	}

	strTag = strBuffer.Mid (iStart, iIndexEnd - iStart);
	strTag.TrimLeft ();
	strTag.TrimRight ();

	strBuffer.Delete (iIndexStart, iIndexEnd + iTagEndLen - iIndexStart);

	if (bIsCharsList)
	{
		if (strTag.GetLength () > 1 && strTag [0] == _T('\"') && strTag [strTag.GetLength () - 1] == _T('\"'))
		{
			strTag = strTag.Mid (1, strTag.GetLength () - 2);
		}

		strTag.Replace (_T("\\t"), _T("\t"));
		strTag.Replace (_T("\\n"), _T("\n"));
		strTag.Replace (_T("\\r"), _T("\r"));
		strTag.Replace (_T("\\b"), _T("\b"));
		strTag.Replace (_T("LT"), _T("<"));
		strTag.Replace (_T("GT"), _T(">"));
		strTag.Replace (_T("AMP"), _T("&"));
	}

	return TRUE;
}
//***********************************************************************************************
BOOL BCGPGLOBAL_DATA::DwmExtendFrameIntoClientArea (HWND hWnd, BCGPMARGINS* pMargins)
{
	if (m_pfDwmExtendFrameIntoClientArea == NULL)
	{
		return FALSE;
	}

	HRESULT hres = (*m_pfDwmExtendFrameIntoClientArea) (hWnd, pMargins);
	return hres == S_OK;
}
//***********************************************************************************************
LRESULT BCGPGLOBAL_DATA::DwmDefWindowProc (HWND hWnd, UINT message, WPARAM wp, LPARAM lp)
{
	if (m_pfDwmDefWindowProc == NULL)
	{
		return (LRESULT)-1;
	}

	LRESULT lres = 0;
	(*m_pfDwmDefWindowProc) (hWnd, message, wp, lp, &lres);

	return lres;
}
//***********************************************************************************************
BOOL BCGPGLOBAL_DATA::DwmIsCompositionEnabled ()
{
	if (m_pfDwmIsCompositionEnabled == NULL || bDisableAero)
	{
		return FALSE;
	}

	BOOL bEnabled = FALSE;

	(*m_pfDwmIsCompositionEnabled) (&bEnabled);
	return bEnabled;
}
//***********************************************************************************************
BOOL BCGPGLOBAL_DATA::DwmSetWindowAttribute(HWND hwnd, DWORD dwAttribute, LPCVOID pvAttribute, DWORD cbAttribute)
{
	if (m_pfDwmSetWindowAttribute == NULL)
	{
		return FALSE;
	}

	HRESULT hres = (*m_pfDwmSetWindowAttribute)(hwnd, dwAttribute, pvAttribute, cbAttribute);
	return hres == S_OK;
}
//***********************************************************************************************
BOOL BCGPGLOBAL_DATA::DwmSetIconicThumbnail(HWND hwnd, HBITMAP hbmp, DWORD dwSITFlags)
{
	if (m_pfDwmSetIconicThumbnail == NULL)
	{
		return FALSE;
	}

	HRESULT hres = (*m_pfDwmSetIconicThumbnail)(hwnd, hbmp, dwSITFlags);
	return hres == S_OK;
}
//***********************************************************************************************
BOOL BCGPGLOBAL_DATA::DwmSetIconicLivePreviewBitmap(HWND hwnd, HBITMAP hbmp, POINT *pptClient, DWORD dwSITFlags)
{
	if (m_pfDwmSetIconicLivePreviewBitmap == NULL)
	{
		return FALSE;
	}

	HRESULT hres = (*m_pfDwmSetIconicLivePreviewBitmap)(hwnd, hbmp, pptClient, dwSITFlags);
	return hres == S_OK;
}
//***********************************************************************************************
BOOL BCGPGLOBAL_DATA::DwmInvalidateIconicBitmaps(HWND hwnd)
{
	if (m_pfDwmInvalidateIconicBitmaps == NULL)
	{
		return FALSE;
	}

	HRESULT hres = (*m_pfDwmInvalidateIconicBitmaps)(hwnd);
	return hres == S_OK;
}
//***********************************************************************************************
BOOL BCGPGLOBAL_DATA::DrawTextOnGlass (HTHEME hTheme, CDC* pDC, int iPartId, int iStateId, 
								   CString strText, CRect rect, DWORD dwFlags,
								   int nGlowSize, COLORREF clrText)
{

//---- bits used in dwFlags of DTTOPTS ----
#define BCGP_DTT_TEXTCOLOR       (1UL << 0)      // crText has been specified
#define BCGP_DTT_BORDERCOLOR     (1UL << 1)      // crBorder has been specified
#define BCGP_DTT_SHADOWCOLOR     (1UL << 2)      // crShadow has been specified
#define BCGP_DTT_SHADOWTYPE      (1UL << 3)      // iTextShadowType has been specified
#define BCGP_DTT_SHADOWOFFSET    (1UL << 4)      // ptShadowOffset has been specified
#define BCGP_DTT_BORDERSIZE      (1UL << 5)      // iBorderSize has been specified
#define BCGP_DTT_FONTPROP        (1UL << 6)      // iFontPropId has been specified
#define BCGP_DTT_COLORPROP       (1UL << 7)      // iColorPropId has been specified
#define BCGP_DTT_STATEID         (1UL << 8)      // IStateId has been specified
#define BCGP_DTT_CALCRECT        (1UL << 9)      // Use pRect as and in/out parameter
#define BCGP_DTT_APPLYOVERLAY    (1UL << 10)     // fApplyOverlay has been specified
#define BCGP_DTT_GLOWSIZE        (1UL << 11)     // iGlowSize has been specified
#define BCGP_DTT_CALLBACK        (1UL << 12)     // pfnDrawTextCallback has been specified
#define BCGP_DTT_COMPOSITED      (1UL << 13)     // Draws text with antialiased alpha (needs a DIB section)

	if (hTheme == NULL ||
		m_pfDrawThemeTextEx == NULL || !DwmIsCompositionEnabled ())
	{
		pDC->DrawText (strText, rect, dwFlags);
		return FALSE;
	}

	_bstr_t bstmp = (LPCTSTR) strText; 

	wchar_t* wbuf = new wchar_t[bstmp.length() + 1];
#if _MSC_VER < 1400
	wcscpy (wbuf, bstmp);
#else
    wcscpy_s (wbuf, bstmp.length() + 1, bstmp);
#endif

	BCGPDTTOPTS dto;
	memset (&dto, 0, sizeof (BCGPDTTOPTS));
	dto.dwSize = sizeof (BCGPDTTOPTS);
	dto.dwFlags = BCGP_DTT_COMPOSITED;

	if (nGlowSize > 0)
	{
		dto.dwFlags |= BCGP_DTT_GLOWSIZE;
		dto.iGlowSize = nGlowSize;
	}

	if (clrText != (COLORREF)-1)
	{
		dto.dwFlags |= BCGP_DTT_TEXTCOLOR;
		dto.crText = clrText;
	}

	(*m_pfDrawThemeTextEx) (hTheme, pDC->GetSafeHdc (), iPartId, iStateId, wbuf, -1, 
		dwFlags, rect, &dto);

	delete [] wbuf;

	return TRUE;
}
//******************************************************************************************************************
BOOL BCGPGLOBAL_DATA::DrawIconOnGlass (HTHEME hTheme, CDC* pDC, HICON hIcon, CRect rect)
{
	ASSERT_VALID (pDC);

	if (hTheme == NULL || m_pfDrawThemeIcon == NULL || !DwmIsCompositionEnabled ())
	{
		pDC->DrawState (rect.TopLeft (), rect.Size (), hIcon, DSS_NORMAL, (CBrush*)NULL);
		return FALSE;
	}

	CImageList images;
	images.Create (rect.Width (), rect.Height (), ILC_COLOR32 | ILC_MASK, 0, 0);
	images.Add (hIcon);

	(*m_pfDrawThemeIcon)(hTheme, pDC->GetSafeHdc (), 0, 0, rect, images.GetSafeHandle (), 0);

	return TRUE;
}
//***********************************************************************************************
void BCGPGLOBAL_DATA::EnableWindowAero(HWND hwnd, BOOL bEnable)
{
	DWORD value = bEnable ? 2 /* DWMNCRP_ENABLED */ : 1 /* DWMNCRP_DISABLED */;
	globalData.DwmSetWindowAttribute(hwnd, 2 /* DWMWA_NCRENDERING_POLICY */, &value, sizeof(value));
}
//***********************************************************************************************
HCURSOR	BCGPGLOBAL_DATA::GetHandCursor ()
{
	if (m_hcurHand == NULL)
	{
#if (!defined _BCGPCALENDAR_STANDALONE) && !(defined _BCGPGRID_STANDALONE) && !(defined _BCGPEDIT_STANDALONE) && !(defined _BCGPCHART_STANDALONE)
		if (bIsWindowsNT4 || bIsWindows9x)
		{
			CBCGPLocalResource locaRes;
			m_hcurHand = AfxGetApp ()->LoadCursor (IDC_BCGBARRES_HAND);
		}
		else
#endif
		{
			m_hcurHand = ::LoadCursor (NULL, MAKEINTRESOURCE(32649)/*IDC_HAND*/);
		}
	}

	return m_hcurHand;
}
//***********************************************************************************************
BOOL BCGPGLOBAL_DATA::Resume ()
{
	m_hiconApp = LoadIcon(NULL, IDI_APPLICATION);

	UpdateUxThemeWrappers();
	UpdateUser32Wrappers();
	UpdateDWMApiWrappers();

#if (!defined _BCGPCALENDAR_STANDALONE) && !(defined _BCGPGRID_STANDALONE) && !(defined _BCGPEDIT_STANDALONE)  && !(defined _BCGPCHART_STANDALONE)

	if (m_bEnableAccessibility)
	{
		m_pfNotifyWinEvent = NULL;
		EnableAccessibilitySupport ();
	}

	if (CBCGPToolBarImages::m_pfAlphaBlend != NULL)
	{
		if ((CBCGPToolBarImages::m_hinstMSIMGDLL = LoadLibrary (_T("msimg32.dll"))) != NULL)
		{
			CBCGPToolBarImages::m_pfAlphaBlend = 
				(ALPHABLEND)::GetProcAddress (CBCGPToolBarImages::m_hinstMSIMGDLL, "AlphaBlend");

			if (!bIsWindows9x)
			{
				CBCGPToolBarImages::m_pfTransparentBlt = 
					(TRANSPARENTBLT)::GetProcAddress (CBCGPToolBarImages::m_hinstMSIMGDLL, "TransparentBlt");
			}
		}
	}

	CBCGPVisualManager2007::Style style = CBCGPVisualManager2007::GetStyle();
	CBCGPVisualManager2010::Style style2010 = CBCGPVisualManager2010::GetStyle();

	CBCGPVisualManager2007::CleanStyle ();

	if (CBCGPVisualManager::m_pRTIDefault != NULL)
	{
		CBCGPVisualManager2007::SetStyle (style);
		CBCGPVisualManager2010::SetStyle (style2010);

		CBCGPVisualManager::SetDefaultManager (CBCGPVisualManager::m_pRTIDefault);
	}

	bcgpGestureManager.Resume();
#endif

	return TRUE;
}

struct BCGP_DLLVERSIONINFO
{
		DWORD cbSize;
		DWORD dwMajorVersion;                   // Major version
		DWORD dwMinorVersion;                   // Minor version
		DWORD dwBuildNumber;                    // Build number
		DWORD dwPlatformID;                     // DLLVER_PLATFORM_*
};

typedef HRESULT (CALLBACK* BCGP_DLLGETVERSIONPROC)(BCGP_DLLVERSIONINFO *);

DWORD BCGPGLOBAL_DATA::GetComCtlVersion ()
{
	if (m_dwComCtlVersion != -1)
	{
		return m_dwComCtlVersion;
	}

	HINSTANCE hInst = ::GetModuleHandle(_T("COMCTL32"));
	ASSERT(hInst != NULL);

	BCGP_DLLGETVERSIONPROC pfn;

	pfn = (BCGP_DLLGETVERSIONPROC)GetProcAddress(hInst, "DllGetVersion");
	DWORD dwVersion = VERSION_WIN4;

	if (pfn != NULL)
	{
		BCGP_DLLVERSIONINFO dvi;
		memset(&dvi, 0, sizeof(dvi));
		dvi.cbSize = sizeof(dvi);
		HRESULT hr = (*pfn)(&dvi);
		if (SUCCEEDED(hr))
		{
			ASSERT(dvi.dwMajorVersion <= 0xFFFF);
			ASSERT(dvi.dwMinorVersion <= 0xFFFF);
			dwVersion = MAKELONG(dvi.dwMinorVersion, dvi.dwMajorVersion);
		}
	}

	m_dwComCtlVersion = dwVersion;
	return m_dwComCtlVersion;
}

BOOL BCGPGLOBAL_DATA::GetNonClientMetrics (NONCLIENTMETRICS& ncm)
{
	struct BCGP_OLDNONCLIENTMETRICS
	{
		UINT    cbSize;
		int     iBorderWidth;
		int     iScrollWidth;
		int     iScrollHeight;
		int     iCaptionWidth;
		int     iCaptionHeight;
		LOGFONT lfCaptionFont;
		int     iSmCaptionWidth;
		int     iSmCaptionHeight;
		LOGFONT lfSmCaptionFont;
		int     iMenuWidth;
		int     iMenuHeight;
		LOGFONT lfMenuFont;
		LOGFONT lfStatusFont;
		LOGFONT lfMessageFont;
	};

	const UINT cbProperSize = (GetComCtlVersion () < MAKELONG(1, 6))
		? sizeof(BCGP_OLDNONCLIENTMETRICS) : sizeof(NONCLIENTMETRICS);

	ZeroMemory (&ncm, sizeof (NONCLIENTMETRICS));
	ncm.cbSize = cbProperSize;

	return ::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, cbProperSize, &ncm, 0);
}

UINT BCGPGLOBAL_DATA::GetRebarBandInfoSize ()
{
	struct BCGP_OLDREBARBANDINFO
	{
		UINT cbSize;
		UINT fMask;
		UINT fStyle;
		COLORREF clrFore;
		COLORREF clrBack;
		LPTSTR lpText;
		UINT cch;
		int iImage;
		HWND hwndChild;
		UINT cxMinChild;
		UINT cyMinChild;
		UINT cx;
		HBITMAP hbmBack;
		UINT wID;
	#if (_WIN32_IE >= 0x0400)
		UINT cyChild;  
		UINT cyMaxChild;
		UINT cyIntegral;
		UINT cxIdeal;
		LPARAM lParam;
		UINT cxHeader;
	#endif
	};

	return (GetComCtlVersion () < MAKELONG(1, 6))
		? sizeof(BCGP_OLDREBARBANDINFO) : sizeof(REBARBANDINFO);
}

BOOL BCGPGLOBAL_DATA::SetDPIAware ()
{
	if (m_hinstUser32 == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

typedef BOOL (__stdcall * BCGPSETPROCESSDPIAWARE)();
	
	BCGPSETPROCESSDPIAWARE pSetDPIAware =
		(BCGPSETPROCESSDPIAWARE)::GetProcAddress(m_hinstUser32, "SetProcessDPIAware");

	if (pSetDPIAware == NULL)
	{
		return FALSE;
	}

	BOOL bRes = (*pSetDPIAware) ();

	UpdateSysColors();
	UpdateFonts();
	OnSettingChange ();

   return bRes;
}

void BCGPGLOBAL_DATA::UpdateShellAutohideBars ()
{
	m_nShellAutohideBars = 0;

#if (!defined _BCGPCALENDAR_STANDALONE) && !(defined _BCGPGRID_STANDALONE) && !(defined _BCGPEDIT_STANDALONE)  && !(defined _BCGPCHART_STANDALONE)
	if (!bIsWindows9x && !bIsWindowsNT4)
	{
		APPBARDATA abd;
		ZeroMemory (&abd, sizeof (APPBARDATA));
		abd.cbSize = sizeof (APPBARDATA);

		abd.uEdge = ABE_BOTTOM;
		if (SHAppBarMessage (ABM_GETAUTOHIDEBAR, &abd))
		{
			m_nShellAutohideBars |= BCGP_AUTOHIDE_BOTTOM;
		}

		abd.uEdge = ABE_TOP;
		if (SHAppBarMessage (ABM_GETAUTOHIDEBAR, &abd))
		{
			m_nShellAutohideBars |= BCGP_AUTOHIDE_TOP;
		}

		abd.uEdge = ABE_LEFT;
		if (SHAppBarMessage (ABM_GETAUTOHIDEBAR, &abd))
		{
			m_nShellAutohideBars |= BCGP_AUTOHIDE_LEFT;
		}

		abd.uEdge = ABE_RIGHT;
		if (SHAppBarMessage (ABM_GETAUTOHIDEBAR, &abd))
		{
			m_nShellAutohideBars |= BCGP_AUTOHIDE_RIGHT;
		}
	}
#endif
}

typedef struct tagBCGPTHUMBBUTTON
{
    DWORD dwMask;
    UINT iId;
    UINT iBitmap;
    HICON hIcon;
    WCHAR szTip[ 260 ];
    DWORD dwFlags;
}
BCGPTHUMBBUTTON;

typedef struct tagBCGPTHUMBBUTTON *LPBCGPTHUMBBUTTON;

MIDL_INTERFACE("56FDF342-FD6D-11d0-958A-006097C9A090")
IBcgpTaskbarList : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE HrInit( void) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE AddTab( 
        /* [in] */ HWND hwnd) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE DeleteTab( 
        /* [in] */ HWND hwnd) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE ActivateTab( 
        /* [in] */ HWND hwnd) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE SetActiveAlt( 
        /* [in] */ HWND hwnd) = 0;
    
};

MIDL_INTERFACE("602D4995-B13A-429b-A66E-1935E44F4317")
IBcgpTaskbarList2 : public IBcgpTaskbarList
{
public:
    virtual HRESULT STDMETHODCALLTYPE MarkFullscreenWindow( 
        /* [in] */ HWND hwnd,
        /* [in] */ BOOL fFullscreen) = 0;
    
};

typedef enum BCGP_TBATFLAG
{	
	BCGP_TBATF_USEMDITHUMBNAIL	= 0x1,
	BCGP_TBATF_USEMDILIVEPREVIEW	= 0x2
} 	
BCGP_TBATFLAG;

typedef enum BCGP_STPFLAG
{	
	BCGP_STPF_NONE	= 0,
	BCGP_STPF_USEAPPTHUMBNAILALWAYS	= 0x1,
	BCGP_STPF_USEAPPTHUMBNAILWHENACTIVE	= 0x2,
	BCGP_STPF_USEAPPPEEKALWAYS	= 0x4,
	BCGP_STPF_USEAPPPEEKWHENACTIVE	= 0x8
}
BCGP_STPFLAG;

MIDL_INTERFACE("ea1afb91-9e28-4b86-90e9-9e9f8a5eefaf")
IBcgpTaskbarList3 : public IBcgpTaskbarList2
{
public:
    virtual HRESULT STDMETHODCALLTYPE SetProgressValue( 
        /* [in] */ HWND hwnd,
        /* [in] */ ULONGLONG ullCompleted,
        /* [in] */ ULONGLONG ullTotal) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE SetProgressState( 
        /* [in] */ HWND hwnd,
        /* [in] */ BCGP_TBPFLAG tbpFlags) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE RegisterTab( 
        /* [in] */ HWND hwndTab,
        /* [in] */ HWND hwndMDI) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE UnregisterTab( 
        /* [in] */ HWND hwndTab) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE SetTabOrder( 
        /* [in] */ HWND hwndTab,
        /* [in] */ HWND hwndInsertBefore) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE SetTabActive( 
        /* [in] */ HWND hwndTab,
        /* [in] */ HWND hwndMDI,
        /* [in] */ BCGP_TBATFLAG tbatFlags) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE ThumbBarAddButtons( 
        /* [in] */ HWND hwnd,
        /* [in] */ UINT cButtons,
        /* [size_is][in] */ LPBCGPTHUMBBUTTON pButton) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE ThumbBarUpdateButtons( 
        /* [in] */ HWND hwnd,
        /* [in] */ UINT cButtons,
        /* [size_is][in] */ LPBCGPTHUMBBUTTON pButton) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE ThumbBarSetImageList( 
        /* [in] */ HWND hwnd,
        /* [in] */ HIMAGELIST himl) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE SetOverlayIcon( 
        /* [in] */ HWND hwnd,
        /* [in] */ HICON hIcon,
        /* [string][in] */ LPCWSTR pszDescription) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE SetThumbnailTooltip( 
        /* [in] */ HWND hwnd,
        /* [string][in] */ LPCWSTR pszTip) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE SetThumbnailClip( 
        /* [in] */ HWND hwnd,
        /* [in] */ RECT *prcClip) = 0;
    
};

MIDL_INTERFACE("c43dc798-95d1-4bea-9030-bb99e2983a1a")
IBcgpTaskbarList4 : public IBcgpTaskbarList3
{
public:
    virtual HRESULT STDMETHODCALLTYPE SetTabProperties( 
        /* [in] */ HWND hwndTab,
        /* [in] */ BCGP_STPFLAG stpFlags) = 0;
    
};

IBcgpTaskbarList3* BCGPGLOBAL_DATA::GetTaskbarList3()
{
	HRESULT hr = S_OK;

	if (!bIsWindows7)
	{
		return NULL;
	}

	if (m_pTaskbarList3 != NULL)
	{
		return m_pTaskbarList3;
	}

	CWinThread* pThread = AfxGetThread();
	if (pThread == NULL || pThread->m_pMessageFilter == NULL)
	{
		return NULL;
	}

	if (!m_bComInitialized)
	{
		hr = CoInitialize(NULL);
		if (SUCCEEDED(hr))
		{
			m_bComInitialized = TRUE;
		}
	}

	if (SUCCEEDED(hr))
	{
		hr = ::CoCreateInstance(CLSID_TaskbarList, NULL, CLSCTX_INPROC_SERVER, __uuidof(IBcgpTaskbarList3), (void **)&m_pTaskbarList3);
	}

	ASSERT(SUCCEEDED(hr));
	return m_pTaskbarList3;
}

void BCGPGLOBAL_DATA::ReleaseTaskbarList3()
{
	if (m_pTaskbarList3 != NULL)
	{
		m_pTaskbarList3->Release();
		m_pTaskbarList3 = NULL;
	}
}

BOOL BCGPGLOBAL_DATA::TaskBar_RegisterTab(HWND hwndTab, HWND hwndMDI)
{
	IBcgpTaskbarList3* pTBList = GetTaskbarList3();
	if (pTBList == NULL)
	{
		return FALSE;
	}

	return (SUCCEEDED(pTBList->RegisterTab(hwndTab, hwndMDI)));
}

BOOL BCGPGLOBAL_DATA::TaskBar_UnregisterTab(HWND hwndTab)
{
	IBcgpTaskbarList3* pTBList = GetTaskbarList3();
	if (pTBList == NULL)
	{
		return FALSE;
	}

	return (SUCCEEDED(pTBList->UnregisterTab(hwndTab)));
}

BOOL BCGPGLOBAL_DATA::TaskBar_SetTabOrder(HWND hwndTab, HWND hwndInsertBefore)
{
	IBcgpTaskbarList3* pTBList = GetTaskbarList3();
	if (pTBList == NULL)
	{
		return FALSE;
	}

	return (SUCCEEDED(pTBList->SetTabOrder(hwndTab, hwndInsertBefore)));
}

BOOL BCGPGLOBAL_DATA::TaskBar_SetThumbnailClip(HWND hwnd, RECT *prcClip)
{
	IBcgpTaskbarList3* pTBList = GetTaskbarList3();
	if (pTBList == NULL)
	{
		return FALSE;
	}

	return (SUCCEEDED(pTBList->SetThumbnailClip(hwnd, prcClip)));
}

BOOL BCGPGLOBAL_DATA::TaskBar_SetTabActive(HWND hwndTab, HWND hwndMDI, UINT uiFlags)
{
	IBcgpTaskbarList3* pTBList = GetTaskbarList3();
	if (pTBList == NULL)
	{
		return FALSE;
	}

	return (SUCCEEDED(pTBList->SetTabActive(hwndTab, hwndMDI, (BCGP_TBATFLAG)uiFlags)));
}

BOOL BCGPGLOBAL_DATA::TaskBar_SetTabProperties(HWND hwndTab, UINT uiFlags)
{
	IBcgpTaskbarList3* pTBList = GetTaskbarList3();
	if (pTBList == NULL)
	{
		return FALSE;
	}

	BOOL bRes = FALSE;

	IBcgpTaskbarList4* pTBList4 = NULL;
	HRESULT hr = pTBList->QueryInterface(__uuidof(IBcgpTaskbarList4), (LPVOID*)&pTBList4);
	
	if (SUCCEEDED (hr) && pTBList4 != NULL)
	{
		bRes = (SUCCEEDED(pTBList4->SetTabProperties(hwndTab, (BCGP_STPFLAG)uiFlags)));

		pTBList4->Release();
	}

	return bRes;
}

BOOL BCGPGLOBAL_DATA::TaskBar_SetProgressState(HWND hwnd, BCGP_TBPFLAG tbpFlags)
{
	IBcgpTaskbarList3* pTBList = GetTaskbarList3();
	if (pTBList == NULL)
	{
		return FALSE;
	}

	BOOL bRes = FALSE;

	IBcgpTaskbarList4* pTBList4 = NULL;
	HRESULT hr = pTBList->QueryInterface(__uuidof(IBcgpTaskbarList4), (LPVOID*)&pTBList4);
	
	if (SUCCEEDED (hr) && pTBList4 != NULL)
	{
		bRes = (SUCCEEDED(pTBList4->SetProgressState(hwnd, tbpFlags)));

		pTBList4->Release();
	}

	return bRes;
}

BOOL BCGPGLOBAL_DATA::TaskBar_SetProgressValue(HWND hwnd, int nCompleted, int nTotal)
{
	IBcgpTaskbarList3* pTBList = GetTaskbarList3();
	if (pTBList == NULL)
	{
		return FALSE;
	}

	BOOL bRes = FALSE;

	IBcgpTaskbarList4* pTBList4 = NULL;
	HRESULT hr = pTBList->QueryInterface(__uuidof(IBcgpTaskbarList4), (LPVOID*)&pTBList4);
	
	if (SUCCEEDED (hr) && pTBList4 != NULL)
	{
		bRes = (SUCCEEDED(pTBList4->SetProgressValue(hwnd, nCompleted, nTotal)));

		pTBList4->Release();
	}

	return bRes;
}

BOOL BCGPGLOBAL_DATA::TaskBar_SetOverlayIcon(HWND hwnd, UINT nIDResource, LPCTSTR lpcszDescr)
{
	BOOL bResult = FALSE;

	if (nIDResource == 0)
	{
		bResult = TaskBar_SetOverlayIcon(hwnd, (HICON)NULL, lpcszDescr);
	}
	else
	{
		HICON hIcon = (HICON) LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(nIDResource), 
			IMAGE_ICON, 16, 16, LR_LOADMAP3DCOLORS);

		if (hIcon == NULL)
		{
			TRACE1("Can't load image from the resource with ID %d.", nIDResource);
			return FALSE;
		}

		bResult = TaskBar_SetOverlayIcon(hwnd, hIcon, lpcszDescr);
		DestroyIcon(hIcon);
	}

	return bResult;
}

BOOL BCGPGLOBAL_DATA::TaskBar_SetOverlayIcon(HWND hwnd, HICON hIcon, LPCTSTR lpcszDescr)
{
	IBcgpTaskbarList3* pTBList = GetTaskbarList3();
	if (pTBList == NULL)
	{
		return FALSE;
	}

	BOOL bRes = FALSE;

	IBcgpTaskbarList4* pTBList4 = NULL;
	HRESULT hr = pTBList->QueryInterface(__uuidof(IBcgpTaskbarList4), (LPVOID*)&pTBList4);
	
	if (SUCCEEDED (hr) && pTBList4 != NULL)
	{
#ifdef UNICODE
		bRes = (SUCCEEDED(pTBList4->SetOverlayIcon(hwnd, hIcon, lpcszDescr)));
#else
		USES_CONVERSION;
		LPWSTR lpwszDescr = A2W(lpcszDescr);
		bRes = (SUCCEEDED(pTBList4->SetOverlayIcon(hwnd, hIcon, lpwszDescr)));
#endif

		pTBList4->Release();
	}

	return bRes;
}

BOOL BCGPGLOBAL_DATA::ChangeWindowMessageFilter(UINT message, DWORD dwFlag)
{
	if (m_pfChangeWindowMessageFilter == NULL)
	{
		return FALSE;
	}

	return (*m_pfChangeWindowMessageFilter)(message, dwFlag);
}

