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

#ifndef __GLOBAL_DATA_H
#define __GLOBAL_DATA_H

#include "BCGCBPro.h"

#if (!defined _BCGPCALENDAR_STANDALONE) && !(defined _BCGPGRID_STANDALONE) && !(defined _BCGPEDIT_STANDALONE) && !(defined _BCGPCHART_STANDALONE)
#include "bcgpaccessibility.h"
#include <oleacc.h>

/////////////////////////////////////////////////////////////////////////////
// Auxiliary System/Screen metrics

typedef enum BCGP_DOCK_TYPE
{
	BCGP_DT_UNDEFINED = 0,		// inherit from application
	BCGP_DT_IMMEDIATE = 1,		// control bar torn off immediately and follows the mouse
	BCGP_DT_STANDARD  = 2,		// user drags a frame
    BCGP_DT_SMART = 0x80		// smart docking style
};

// autohide sliding modes
static const UINT BCGP_AHSM_MOVE		= 1;
static const UINT BCGP_AHSM_STRETCH		= 2;

#define BCGP_AUTOHIDE_LEFT		0x0001
#define BCGP_AUTOHIDE_RIGHT		0x0002
#define BCGP_AUTOHIDE_TOP		0x0004
#define BCGP_AUTOHIDE_BOTTOM	0x0008

typedef BOOL (__stdcall * NOTIFYWINEVENT)(DWORD event, HWND hwnd, LONG idObject, LONG idChild);
#endif

#ifndef _UXTHEME_H_

// From uxtheme.h:
typedef HANDLE HTHEME;          // handle to a section of theme data for class
#endif // THEMEAPI

typedef HANDLE BCGPHPAINTBUFFER;  // handle to a buffered paint context

typedef BOOL (__stdcall * SETLAYEATTRIB)(HWND hwnd, COLORREF crKey, BYTE bAlpha, DWORD dwFlags);
typedef BOOL (__stdcall * UPDATELAYEREDWINDOW)(HWND hwnd, HDC hdcDst, POINT *pptDst, SIZE *psize, 
											   HDC hdcSrc, POINT *pptSrc, COLORREF crKey, 
											   BLENDFUNCTION *pblend, DWORD dwFlags);
typedef HRESULT (__stdcall * BCGP_DRAWTHEMEPARENTBACKGROUND)(HWND hWnd, HDC hdc,const RECT *pRec);
typedef HRESULT (__stdcall * BCGP_SETWINDOWTHEME)(HWND hWnd, LPCWSTR pszSubAppName, LPCWSTR pszSubIdList);

typedef HRESULT (__stdcall * BCGP_GETTHEMEBITMAP)(HTHEME hTheme, int iPartId, int iStateId, int iPropId, ULONG dwFlags, HBITMAP* phBitmap);

// BCGP_BP_BUFFERFORMAT
typedef enum _BCGP_BP_BUFFERFORMAT
{
    BCGP_BPBF_COMPATIBLEBITMAP,    // Compatible bitmap
    BCGP_BPBF_DIB,                 // Device-independent bitmap
    BCGP_BPBF_TOPDOWNDIB,          // Top-down device-independent bitmap
    BCGP_BPBF_TOPDOWNMONODIB       // Top-down monochrome device-independent bitmap
} BCGP_BP_BUFFERFORMAT;

#define BCGP_BP_BUFFERFORMATBPBF_COMPOSITED BCGP_BP_BUFFERFORMATBPBF_DIB

// BCGP_BP_PAINTPARAMS
typedef struct _BCGP_BP_PAINTPARAMS
{
    DWORD                       cbSize;
    DWORD                       dwFlags; // BPPF_ flags
    const RECT *                prcExclude;
    const BLENDFUNCTION *       pBlendFunction;
} BCGP_BP_PAINTPARAMS;

typedef BCGPHPAINTBUFFER (__stdcall * BCGP_BEGINBUFFEREDPAINT)(	HDC hdcTarget, const RECT* rcTarget, 
															BCGP_BP_BUFFERFORMAT dwFormat, 
															BCGP_BP_PAINTPARAMS *pPaintParams,
															HDC *phdc);

typedef HRESULT (__stdcall * BCGP_BUFFEREDPAINTSETALPHA)(BCGPHPAINTBUFFER hBufferedPaint, const RECT *prc, BYTE alpha);


typedef HRESULT (__stdcall * BCGP_ENDBUFFEREDPAINT)(BCGPHPAINTBUFFER hBufferedPaint, BOOL fUpdateTarget);

typedef struct _BCGPMARGINS {
    int cxLeftWidth;
    int cxRightWidth;
    int cyTopHeight;
    int cyBottomHeight;
} BCGPMARGINS;

typedef HRESULT (__stdcall * BCGP_DWMEXTENDFRAMEINTOCLIENTAREA)(HWND hWnd, const BCGPMARGINS* pMargins);
typedef HRESULT (__stdcall * BCGP_DWMDEFWINDOWPROC)(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, LRESULT *plResult);
typedef HRESULT (__stdcall * BCGP_DWMISCOMPOSITIONENABLED)(BOOL* pfEnabled);
typedef HRESULT (__stdcall * BCGP_DWMSETWINDOWATTRIBUTE)(HWND, DWORD, LPCVOID, DWORD);
typedef HRESULT (__stdcall * BCGP_DWMSETICONICTHUMBNAIL)(HWND, HBITMAP, DWORD);
typedef HRESULT (__stdcall * BCGP_DWMSETICONICLIVEPRBMP)(HWND, HBITMAP, POINT *, DWORD);
typedef HRESULT (__stdcall * BCGP_DWMINVALIDATEICONICBITMAPS)(HWND);

typedef int (WINAPI *BCGPDTT_CALLBACK_PROC)
(
    HDC hdc,
    LPWSTR pszText,
    int cchText,
    LPRECT prc,
    UINT dwFlags,
    LPARAM lParam);

typedef struct _BCGPDTTOPTS {
    DWORD dwSize;
    DWORD dwFlags;
    COLORREF crText;
    COLORREF crBorder;
    COLORREF crShadow;
    int iTextShadowType;
    POINT ptShadowOffset;
    int iBorderSize;
    int iFontPropId;
    int iColorPropId;
    int iStateId;
    BOOL fApplyOverlay;
    int iGlowSize;
    BCGPDTT_CALLBACK_PROC pfnDrawTextCallback;
    LPARAM lParam;
} BCGPDTTOPTS;

typedef HRESULT (__stdcall * BCGP_DRAWTHEMETEXTEX)(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCWSTR pszText, int iCharCount, DWORD dwFlags, LPRECT pRect, const BCGPDTTOPTS *pOptions);
typedef HRESULT (__stdcall * BCGP_DRAWTHEMEICON)(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, const RECT *pRect, HIMAGELIST himl, int iImageIndex);

class CBCGPToolBarImages;

struct IBcgpTaskbarList3;

typedef enum BCG_TBPFLAG
{
    BCGP_TBPF_NOPROGRESS= 0,
    BCGP_TBPF_INDETERMINATE	= 0x1,
    BCGP_TBPF_NORMAL	= 0x2,
    BCGP_TBPF_ERROR		= 0x4,
    BCGP_TBPF_PAUSED	= 0x8
} 	
BCGP_TBPFLAG;

typedef BOOL (__stdcall * BCGP_CHANGEWINDOWMESSAGEFILTER)(UINT, DWORD);

struct BCGCBPRODLLEXPORT BCGPGLOBAL_DATA
{
	friend class CBCGPMemDC;
	friend class CBCGPGraphicsManagerD2D;

	BOOL	m_bUseSystemFont;	// Use system font for menu/toolbar/ribbons

	// solid brushes with convenient gray colors and system colors
	HBRUSH hbrBtnHilite, hbrBtnShadow;

	HBRUSH hbrWindow;

	// color values of system colors used for CToolBar
	COLORREF clrBtnFace, clrBtnShadow, clrBtnHilite;
	COLORREF clrBtnText, clrWindowFrame;
	COLORREF clrBtnDkShadow, clrBtnLight;
	COLORREF clrGrayedText;
	COLORREF clrPrompt;
	COLORREF clrHilite;
	COLORREF clrTextHilite;
	COLORREF clrHotText;
	COLORREF clrHotLinkText;

	COLORREF clrBarWindow;
	COLORREF clrBarFace;
	COLORREF clrBarShadow, clrBarHilite;
	COLORREF clrBarDkShadow, clrBarLight;
	COLORREF clrBarText;

	COLORREF clrWindow;
	COLORREF clrWindowText;

	COLORREF clrCaptionText;

	COLORREF clrMenuText;

	COLORREF clrActiveCaption;
	COLORREF clrInactiveCaption;

	COLORREF clrActiveCaptionGradient;
	COLORREF clrInactiveCaptionGradient;

	COLORREF clrInactiveCaptionText;

	COLORREF clrActiveBorder;
	COLORREF clrInactiveBorder;

	CBrush	brBtnFace;
	CBrush	brHilite;
	CBrush	brLight;
	CBrush	brBlack;
	CBrush	brActiveCaption;
	CBrush	brInactiveCaption;
	CBrush	brWindow;

	CBrush brBarFace;

	CPen	penHilite;
	CPen	penBarFace;
	CPen	penBarShadow;

	// Library cursors:
	HCURSOR	m_hcurStretch;
	HCURSOR	m_hcurStretchVert;
	HCURSOR	m_hcurHand;
	HCURSOR	m_hcurSizeAll;
	HCURSOR	m_hcurMoveTab;
	HCURSOR	m_hcurNoMoveTab;
	HCURSOR	m_hcurSelectRow;
	HCURSOR m_hcurMagnify;

	HCURSOR	GetHandCursor ();

	HICON	m_hiconTool;
	HICON	m_hiconLink;
	HICON	m_hiconColors;
	HICON	m_hiconApp;

	// Shell icon sizes:
	CSize	m_sizeSmallIcon;

	// Toolbar and menu fonts:
	CFont				fontRegular;
	CFont				fontCaption;
	CFont				fontTooltip;
	CFont				fontBold;
	CFont				fontDefaultGUIBold;
	CFont				fontUnderline;
	CFont				fontDefaultGUIUnderline;
	CFont				fontVert;
	CFont				fontVertCaption;
	CFont				fontSmall;
	CFont				fontGroup;

	CFont				fontMarlett;	// Standard Windows menu symbols
						
	BOOL				bIsWindowsNT4;
	BOOL				bIsWindows9x;
	BOOL				bIsWindows2000;
	BOOL				bIsWindowsXP;
	BOOL				bIsWindowsVista;
	BOOL				bIsWindows7;
	BOOL				bIsWindows8;
	BOOL				bDisableAero;
	int					m_nBitsPerPixel;

	BOOL				bIsRemoteSession;
	BOOL				bIsOSAlphaBlendingSupport;

	int					m_nDragFrameThiknessFloat;
	int					m_nDragFrameThiknessDock;
						
	int					m_nAutoHideToolBarSpacing;
	int					m_nAutoHideToolBarMargin;
						
	int					m_nCoveredMainWndClientAreaPercent;

	int					m_nMaxToolTipWidth;

	BOOL				m_bIsBlackHighContrast;
	BOOL				m_bIsWhiteHighContrast;

	BOOL				m_bUseBuiltIn32BitIcons;
	BOOL				m_bUseVisualManagerInBuiltInDialogs;

	CRect				m_rectVirtual;

	BOOL				m_bMenuAnimation;
	BOOL				m_bMenuFadeEffect;

	BOOL				m_bIsRTL;

	BOOL				m_bEnableAccessibility;
	BOOL				m_bInSettingsChange;

	BOOL				m_bUnderlineKeyboardShortcuts;
	BOOL				m_bSysUnderlineKeyboardShortcuts;

	BOOL				m_bShowTooltipsOnRibbonFloaty;

	BOOL				m_bShowFrameLayeredShadows;

// Implementation
	BCGPGLOBAL_DATA();
	~BCGPGLOBAL_DATA();

	void UpdateSysColors();
	void UpdateFonts();
	void OnSettingChange ();

	void UpdateShellAutohideBars ();
	int GetShellAutohideBars ()
	{
		if (!m_bShellAutohideBarsInitialized)
		{
			UpdateShellAutohideBars ();
			m_bShellAutohideBarsInitialized = TRUE;
		}

		return m_nShellAutohideBars;
	}

	BOOL SetMenuFont (LPLOGFONT lpLogFont, BOOL bHorz);

	int GetTextHeight (BOOL bHorz = TRUE)
	{
		return bHorz ? m_nTextHeightHorz : m_nTextHeightVert;
	}

	int GetTextWidth (BOOL bHorz = TRUE)
	{
		return bHorz ? m_nTextWidthHorz : m_nTextWidthVert;
	}
	
	int GetCaptionTextWidth()
	{
		return m_nCaptionTextWidth;
	}

	int GetCaptionTextHeight()
	{
		return m_nCaptionTextHeight;
	}
	
	int GetTextMargins (BOOL bHorz = TRUE)
	{
		return bHorz ? m_nTextMarginsHorz : m_nTextMarginsVert;
	}

	double GetRibbonImageScale ()
	{
		return m_bIsRibbonImageScale ? m_dblRibbonImageScale : 1.;
	}

	void EnableRibbonImageScale (BOOL bEnable = TRUE)
	{
		m_bIsRibbonImageScale = bEnable;
	}

	BOOL IsRibbonImageScaleEnabled ()
	{
		return m_bIsRibbonImageScale;
	}

	BOOL IsWinXPDrawParentBackground () const
	{
		return m_pfDrawThemeBackground != NULL;
	}

	BOOL DrawParentBackground (CWnd* pWnd, CDC* pDC, LPRECT lpRect = NULL);
	BOOL SetWindowTheme(CWnd* pWnd, LPCWSTR pszSubAppName, LPCWSTR pszSubIdList);

	HBITMAP GetThemeBitmap(HTHEME hTheme, int iPartId, int iStateId, int iPropId, ULONG dwFlags = 0x00000001 /*GBF_DIRECT*/);

	void CleanUp ();

	COLORREF GetColor (int nColor);

	BOOL SetLayeredAttrib (HWND hwnd, COLORREF crKey, BYTE bAlpha, DWORD dwFlags);
	BOOL UpdateLayeredWindow (HWND hwnd, HDC hdcDst, POINT *pptDst, SIZE *psize, HDC hdcSrc,
							POINT *pptSrc, COLORREF crKey, BLENDFUNCTION *pblend, DWORD dwFlags);
	BOOL IsWindowsLayerSupportAvailable () const
	{
		return m_pfSetLayeredWindowAttributes != NULL; 
	}

	BOOL Is32BitIcons () const
	{
		return m_bUseBuiltIn32BitIcons && m_nBitsPerPixel >= 16 && !m_bIsBlackHighContrast && !m_bIsWhiteHighContrast;
	}

	BOOL IsHighContastMode () const
	{
		return m_bIsWhiteHighContrast || m_bIsBlackHighContrast;
	}

#if (!defined _BCGPCALENDAR_STANDALONE) && !(defined _BCGPGRID_STANDALONE) && !(defined _BCGPEDIT_STANDALONE) && !(defined _BCGPCHART_STANDALONE)
	BOOL IsAccessibilitySupport () const
	{
		return m_bEnableAccessibility;
	}

	void EnableAccessibilitySupport (BOOL bEnable = TRUE);
	BOOL NotifyWinEvent (DWORD event, HWND hwnd, LONG idObject, LONG idChild);
#endif

	CString RegisterWindowClass (LPCTSTR lpszClassNamePrefix);
	BOOL ExcludeTag (CString& strBuffer, LPCTSTR lpszTag, CString& strTag, BOOL bIsCharsList = FALSE);

	// DWM wrappers:
	BOOL DwmExtendFrameIntoClientArea (HWND hWnd, BCGPMARGINS* pMargins);
	LRESULT DwmDefWindowProc (HWND hWnd, UINT message, WPARAM wp, LPARAM lp);
	BOOL DwmIsCompositionEnabled ();
	BOOL DwmSetWindowAttribute(HWND hwnd, DWORD dwAttribute, LPCVOID pvAttribute, DWORD cbAttribute);
	BOOL DwmSetIconicThumbnail(HWND hwnd, HBITMAP hbmp, DWORD dwSITFlags);
	BOOL DwmSetIconicLivePreviewBitmap(HWND hwnd, HBITMAP hbmp, POINT *pptClient, DWORD dwSITFlags);
	BOOL DwmInvalidateIconicBitmaps(HWND hwnd);

	BOOL DrawTextOnGlass (	HTHEME hTheme, CDC* pDC, int iPartId, int iStateId, 
							CString strText, CRect rect, DWORD dwFlags,
							int nGlowSize = 0, COLORREF clrText = (COLORREF)-1);
	BOOL DrawIconOnGlass (HTHEME hTheme, CDC* pDC, HICON hIcon, CRect rect);

	void EnableWindowAero(HWND hwnd, BOOL bEnable);

	BOOL Resume ();
	
	BOOL GetNonClientMetrics (NONCLIENTMETRICS& ncm);
	UINT GetRebarBandInfoSize ();
	DWORD GetComCtlVersion ();

	BOOL SetDPIAware ();

	// TaskBar wrappers:
	BOOL TaskBar_RegisterTab(HWND hwndTab, HWND hwndMDI);
	BOOL TaskBar_UnregisterTab(HWND hwndTab);
	BOOL TaskBar_SetTabOrder(HWND hwndTab, HWND hwndInsertBefore);
	BOOL TaskBar_SetThumbnailClip(HWND hwnd, RECT *prcClip);
	BOOL TaskBar_SetTabActive(HWND hwndTab, HWND hwndMDI, UINT uiFlags);
	BOOL TaskBar_SetTabProperties(HWND hwndTab, UINT uiFlags);
	BOOL TaskBar_SetProgressState(HWND hwnd, BCGP_TBPFLAG tbpFlags);
	BOOL TaskBar_SetProgressValue(HWND hwnd, int nCompleted, int nTotal);
	BOOL TaskBar_SetOverlayIcon(HWND hwnd, UINT nIDResource, LPCTSTR lpcszDescr);
	BOOL TaskBar_SetOverlayIcon(HWND hwnd, HICON hIcon, LPCTSTR lpcszDescr);

	BOOL ChangeWindowMessageFilter(UINT message, DWORD dwFlag);

	// Windows NLS wrappers:
	const CStringList& GetLocaleList() const;
	CString GetLocaleName(const CString& strLocale);
	CString GetLocaleByName(const CString& strName);

	void AddLocale(LPCTSTR lpszLocale, LPCTSTR lpszName);

protected:

	void UpdateTextMetrics ();
	HBITMAP CreateDitherBitmap (HDC hDC);

	void UpdateUxThemeWrappers();
	void UpdateUser32Wrappers();
	void UpdateDWMApiWrappers();
	
	int	m_nTextHeightHorz;
	int	m_nTextHeightVert;

	int	m_nCaptionTextHeight;
	int	m_nCaptionTextWidth;

	int	m_nTextMarginsHorz;
	int	m_nTextMarginsVert;

	int	m_nTextWidthHorz;
	int	m_nTextWidthVert;

	double	m_dblRibbonImageScale;
	BOOL	m_bIsRibbonImageScale;

	HINSTANCE m_hinstUXThemeDLL;
	HINSTANCE m_hinstUser32;
	HINSTANCE m_hinstDwmapiDLL;

	SETLAYEATTRIB m_pfSetLayeredWindowAttributes;
	UPDATELAYEREDWINDOW m_pfUpdateLayeredWindow;

#if (!defined _BCGPCALENDAR_STANDALONE) && !(defined _BCGPGRID_STANDALONE) && !(defined _BCGPEDIT_STANDALONE) && !(defined _BCGPCHART_STANDALONE)
	NOTIFYWINEVENT			m_pfNotifyWinEvent;
#endif

	BCGP_DRAWTHEMEPARENTBACKGROUND		m_pfDrawThemeBackground;
	BCGP_DRAWTHEMETEXTEX				m_pfDrawThemeTextEx;
	BCGP_DRAWTHEMEICON					m_pfDrawThemeIcon;
	BCGP_BEGINBUFFEREDPAINT				m_pfBeginBufferedPaint;
	BCGP_BUFFEREDPAINTSETALPHA			m_pfBufferedPaintSetAlpha;
	BCGP_ENDBUFFEREDPAINT				m_pfEndBufferedPaint;
	BCGP_DWMEXTENDFRAMEINTOCLIENTAREA	m_pfDwmExtendFrameIntoClientArea;
	BCGP_DWMDEFWINDOWPROC				m_pfDwmDefWindowProc;
	BCGP_DWMISCOMPOSITIONENABLED		m_pfDwmIsCompositionEnabled;
	BCGP_DWMSETWINDOWATTRIBUTE			m_pfDwmSetWindowAttribute;
	BCGP_DWMSETICONICTHUMBNAIL			m_pfDwmSetIconicThumbnail;
	BCGP_DWMSETICONICLIVEPRBMP			m_pfDwmSetIconicLivePreviewBitmap;
	BCGP_DWMINVALIDATEICONICBITMAPS		m_pfDwmInvalidateIconicBitmaps;
	BCGP_CHANGEWINDOWMESSAGEFILTER		m_pfChangeWindowMessageFilter;
	BCGP_SETWINDOWTHEME					m_pfSetWindowTheme;
	BCGP_GETTHEMEBITMAP					m_pfGetThemeBitmap;

	DWORD								m_dwComCtlVersion;
	int									m_nShellAutohideBars;
	BOOL								m_bShellAutohideBarsInitialized;

	IBcgpTaskbarList3*					GetTaskbarList3();
	void								ReleaseTaskbarList3();

	IBcgpTaskbarList3*					m_pTaskbarList3;

	BOOL								m_bComInitialized;

	CStringList							m_lstLocales;
	CMapStringToString					m_mapLocaleNames;
};

extern BCGCBPRODLLEXPORT BCGPGLOBAL_DATA globalData;

#define IMAGE_MARGIN	4

// MFC Control bar compatibility 
#define CX_BORDER   1
#define CY_BORDER   1

#define CX_GRIPPER  3
#define CY_GRIPPER  3
#define CX_BORDER_GRIPPER 2
#define CY_BORDER_GRIPPER 2

/////////////////////////////////////////////////////////////////////////////

#endif // __GLOBAL_DATA_H
