// XTPOffice2007Theme.h : interface for the CXTPOffice2007Theme class.
//
// This file is a part of the XTREME COMMANDBARS MFC class library.
// (c)1998-2011 Codejock Software, All Rights Reserved.
//
// THIS SOURCE FILE IS THE PROPERTY OF CODEJOCK SOFTWARE AND IS NOT TO BE
// RE-DISTRIBUTED BY ANY MEANS WHATSOEVER WITHOUT THE EXPRESSED WRITTEN
// CONSENT OF CODEJOCK SOFTWARE.
//
// THIS SOURCE CODE CAN ONLY BE USED UNDER THE TERMS AND CONDITIONS OUTLINED
// IN THE XTREME TOOLKIT PRO LICENSE AGREEMENT. CODEJOCK SOFTWARE GRANTS TO
// YOU (ONE SOFTWARE DEVELOPER) THE LIMITED RIGHT TO USE THIS SOFTWARE ON A
// SINGLE COMPUTER.
//
// CONTACT INFORMATION:
// support@codejock.com
// http://www.codejock.com
//
/////////////////////////////////////////////////////////////////////////////

//{{AFX_CODEJOCK_PRIVATE
#if !defined(__XTPOFFICE2007THEME_H__)
#define __XTPOFFICE2007THEME_H__
//}}AFX_CODEJOCK_PRIVATE

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CXTPResourceImage;
class CXTPResourceImages;
class CXTPOffice2007FrameHook;
class CXTPMarkupContext;

#include "XTPOffice2003Theme.h"
#include "XTPControlGallery.h"

//===========================================================================
// Summary:
//     The CXTPOffice2007Theme class is used to enable an Office 2007 style theme for Command Bars
// See Also: CXTPPaintManager::SetTheme
//===========================================================================
class _XTP_EXT_CLASS CXTPOffice2007Theme : public CXTPOffice2003Theme
{
	DECLARE_DYNAMIC(CXTPOffice2007Theme)
public:

	//-----------------------------------------------------------------------
	// Summary:
	//     Constructs a CXTPOffice2007Theme object.
	//-----------------------------------------------------------------------
	CXTPOffice2007Theme();

	//-----------------------------------------------------------------------
	// Summary:
	//     Destroys a CXTPOffice2007Theme object, handles cleanup and deallocation
	//-----------------------------------------------------------------------
	~CXTPOffice2007Theme();

public:

	//-----------------------------------------------------------------------
	// Summary:
	//     Loads a bitmap image from the specified path.
	// Parameters:
	//     lpszFileName - The name of the file that contains the bitmaps
	//                    to load.
	// Remarks:
	//     The images are the bitmaps that represent all the visual components
	//     of the Ribbon Bar.  For example tab buttons, group buttons, menu buttons,
	//     toolbar buttons, option button, toolbar dropdown, etc.
	//     The images are loaded using LoadImage and are stored in the m_pImages
	//     image collection.
	//     Images for the Office 2007 theme can be found in the \Source\Ribbon\Res
	//     folder.
	// Returns: New CXTPResourceImage containing the loaded bitmap.
	// See Also: SetImageHandle, m_pImages
	//-----------------------------------------------------------------------
	CXTPResourceImage* LoadImage(LPCTSTR lpszFileName);

	//-----------------------------------------------------------------------
	// Summary:
	//     Specifies the resource that contains the bitmaps to be loaded for the
	//     theme.  This allows images to be loaded from something other than bitmap files.
	// Parameters:
	//     hResource - Handle to a visual style dll file handle. Or
	//                 some other resource that contains the images
	//                 to load for the theme.
	//     lpszIniFileName - String identifier of ini file with color specifications
	// Remarks:
	//     The images are the bitmaps that represent all the visual components
	//     of the Ribbon Bar.  For example tab buttons, group buttons, menu buttons,
	//     toolbar buttons, option button, toolbar dropdown, etc.
	//     The images are loaded using LoadImage and are stored in the m_pImages
	//     image collection.
	//     Images for the Office 2007 theme can be found in the \Source\Ribbon\Res
	//     folder.
	// Example:
	//     <code>((CXTPOffice2007Theme*)XTPPaintManager())->SetImageHandle(XTPSkinManager()->GetResourceFile()->GetModuleHandle());</code>
	// See Also: LoadImage, m_pImages
	//-----------------------------------------------------------------------
	void SetImageHandle(HMODULE hResource, LPCTSTR lpszIniFileName);


protected:
	//-----------------------------------------------------------------------
	// Summary:
	//     This method is called to refresh the visual metrics of the manager.
	//-----------------------------------------------------------------------
	virtual void RefreshMetrics();

	//-----------------------------------------------------------------------
	// Summary:
	//     Retrieves the base theme of the manager.
	// Returns:
	//     Returns the base theme.
	//-----------------------------------------------------------------------
	XTPPaintTheme BaseTheme() { return xtpThemeResource; }

	//-----------------------------------------------------------------------
	// Summary:
	//     This method is called to fill a status bar.
	// Parameters:
	//     pDC  - Pointer to a valid device context
	//     pBar - Points to a CXTPStatusBar object
	//-----------------------------------------------------------------------
	virtual void FillStatusBar(CDC* pDC, CXTPStatusBar* pBar);

	//-----------------------------------------------------------------------
	// Summary: Draws the message bar.
	// Input:   pDC - Pointer to a valid device context.
	//          pBar - Pointer to the message bar to draw.
	//-----------------------------------------------------------------------
	virtual void FillMessageBar(CDC* pDC, CXTPMessageBar* pBar);

	//-----------------------------------------------------------------------
	// Summary: Call this member to draw a button in the message bar.
	// Input:   pDC - Pointer to a valid device context.
	//          pButton - Button to draw.
	//-----------------------------------------------------------------------
	virtual void DrawMessageBarButton(CDC* pDC, CXTPMessageBarButton* pButton);

	//-----------------------------------------------------------------------
	// Summary:
	//     This method is called to draw the status bar's gripper.
	// Parameters:
	//     pDC      - Pointer to a valid device context.
	//     rcClient - Client rectangle of the status bar.
	//-----------------------------------------------------------------------
	virtual void DrawStatusBarGripper(CDC* pDC, CRect rcClient);

	//-----------------------------------------------------------------------
	// Summary:
	//     This method is called to draw frame of single cell of status bar.
	// Parameters:
	//     pDC - Points to a valid device context.
	//     rc - CRect object specifying size of area.
	//     pPane - The status bar pane need to draw
	//     bGripperPane - TRUE if pane is last cell of status bar
	//-----------------------------------------------------------------------
	virtual void DrawStatusBarPaneBorder(CDC* pDC, CRect rc, CXTPStatusBarPane* pPane, BOOL bGripperPane);

public:

	//-----------------------------------------------------------------------
	// Summary:
	//     This method is called to get CXTPResourceImages pointer
	// Returns:
	//     Pointer to image collection of the class.
	//-----------------------------------------------------------------------
	CXTPResourceImages* GetImages() const;

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this method to set images for theme
	// Parameters:
	//     pImages - new CXTPResourceImages to bet set
	// See Also: GetImages
	//-----------------------------------------------------------------------
	void SetImages(CXTPResourceImages* pImages);

	//-----------------------------------------------------------------------
	// Summary:
	//     This method retrieves buttons text color.
	// Parameters:
	//     bSelected   - TRUE if the control is selected.
	//     bPressed    - TRUE if the control is pushed.
	//     bEnabled    - TRUE if the control is enabled.
	//     bChecked    - TRUE if the control is checked.
	//     bPopuped    - TRUE if the control is popuped.
	//     barType     - Parent's bar type
	//     barPosition - Parent's bar position.
	// Returns:
	//     Buttons text color.
	//-----------------------------------------------------------------------
	virtual COLORREF GetRectangleTextColor(BOOL bSelected, BOOL bPressed, BOOL bEnabled, BOOL bChecked, BOOL bPopuped, XTPBarType barType, XTPBarPosition barPosition);

	//-----------------------------------------------------------------------
	// Summary:
	//     Draws workspace space
	// Parameters:
	//     pDC - Pointer to device context
	//     rc - Bounding rectangle
	//     rcExclude - Excluded rectangle
	//-----------------------------------------------------------------------
	void FillWorkspace(CDC* pDC, CRect rc, CRect rcExclude);

	//-----------------------------------------------------------------------
	// Summary:
	//     Returns paint manager used to draw frames.
	// Returns:
	//     Returns paint manager used to draw frames.
	//-----------------------------------------------------------------------
	CXTPFramePaintManager* GetFramePaintManager();

	//-----------------------------------------------------------------------
	// Summary:
	//     Retrieves ribbon paint manager.
	// Returns:
	//     Pointer to CXTPRibbonPaintManager object.
	//-----------------------------------------------------------------------
	CXTPRibbonPaintManager* GetRibbonPaintManager();

protected:

	//-----------------------------------------------------------------------
	// Summary:
	//     Determines if images is available
	// Returns:
	//     TRUE if images exist in application
	//-----------------------------------------------------------------------
	BOOL IsImagesAvailable();


protected:
	int m_nRibbonCaptionHeight;         // Ribbon caption height


protected:
//{{AFX_CODEJOCK_PRIVATE
	CXTPResourceImages* m_pImages;
	COLORREF m_clrMenuBarText;
	COLORREF m_clrStatusBarShadow;
	CXTPPaintManagerColorGradient m_clrStatusBarTop;
	CXTPPaintManagerColorGradient m_clrStatusBarBottom;
	COLORREF m_clrWorkspaceClientTop;
	COLORREF m_clrWorkspaceClientMiddle;
	COLORREF m_clrWorkspaceClientBottom;
	CXTPPaintManagerColorGradient m_clrMessageBar;
	COLORREF m_clrMessageBarFrame;
	COLORREF m_clrMessageBarFace;
//}}AFX_CODEJOCK_PRIVATE
};



//===========================================================================
// Summary: NEEDS Paint manager for the Office 2007 style window frame.
//===========================================================================
class _XTP_EXT_CLASS CXTPFramePaintManager
{
public:

	//-----------------------------------------------------------------------
	// Summary: Constructs a CXTPFramePaintManager object.
	// Input:   pPaintManager - Reference to a CXTPPaintManager object.
	//-----------------------------------------------------------------------
	CXTPFramePaintManager(CXTPPaintManager* pPaintManager);

	//-----------------------------------------------------------------------
	// Summary:
	//     Destroys a CXTPFramePaintManager object, handles cleanup and
	//     deallocation
	//-----------------------------------------------------------------------
	virtual ~CXTPFramePaintManager();

public:

	//-----------------------------------------------------------------------
	// Summary:
	//     This method is called to create window region
	// Parameters:
	//      pFrameHook - CXTPOffice2007FrameHook hook object
	//      sz - Window size
	// Returns: Region of window to be set
	//-----------------------------------------------------------------------
	virtual HRGN CalcFrameRegion(CXTPOffice2007FrameHook* pFrameHook, CSize sz);

	//-----------------------------------------------------------------------
	// Summary:
	//     This method is called to draw frame if Ribbon was found
	// Parameters:
	//      pDC - Pointer to device context
	//      pFrameHook - CXTPOffice2007FrameHook hook object
	//-----------------------------------------------------------------------
	virtual void DrawFrame(CDC* pDC, CXTPOffice2007FrameHook* pFrameHook);

	//-----------------------------------------------------------------------
	// Summary:
	//     This method is called to draw caption button
	// Parameters:
	//      pDC - Pointer to device context
	//      rc - Bounding rectangle of the button
	//      nId - Identifier of the button
	//      bSelected - TURE if button is highlighted
	//      bPressed -  TURE if button is pressed
	//      bActive - TURE if frame is active
	//-----------------------------------------------------------------------
	virtual void DrawFrameCaptionButton(CDC* pDC, CRect rc, int nId, BOOL bSelected, BOOL bPressed, BOOL bActive);

	//-----------------------------------------------------------------------
	// Summary:
	//     Retrieves height of caption.
	// Returns:
	//     Frame Caption's height.
	//-----------------------------------------------------------------------
	int GetFrameCaptionHeight() const;

	//-----------------------------------------------------------------------
	// Summary:
	//     This method is called to refresh the visual metrics of the manager.
	//-----------------------------------------------------------------------
	virtual void RefreshMetrics();

	//-----------------------------------------------------------------------
	// Summary:
	//     This method is called to get CXTPResourceImages pointer.
	// Returns:
	//     Pointer to image collection of the class.
	//-----------------------------------------------------------------------
	CXTPResourceImages* GetImages() const;

	//-----------------------------------------------------------------------
	// Summary:
	//     This method is called to draw frame caption text
	// Parameters:
	//     pDC - Pointer to device context
	//     rcCaptionText - Bounding rectangle of window text
	//     pSite - Frame to draw
	//     bActive - TRUE if frame is active
	//-----------------------------------------------------------------------
	virtual void DrawCaptionText(CDC* pDC, CRect rcCaptionText, CWnd* pSite, BOOL bActive);

	//-----------------------------------------------------------------------
	// Summary:
	//     Loads a bitmap image from the specified path.
	// Parameters:
	//     lpszFileName - The name of the file that contains the bitmaps
	//                    to load.
	// Remarks:
	//     The images are the bitmaps that represent all the visual components
	//     of the Ribbon Bar.  For example tab buttons, group buttons, menu buttons,
	//     toolbar buttons, option button, toolbar dropdown, etc.
	//     The images are loaded using LoadImage and are stored in the m_pImages
	//     image collection.
	//     Images for the Office 2007 theme can be found in the \Source\Ribbon\Res
	//     folder.
	// Returns: New CXTPResourceImage containing the loaded bitmap.
	// See Also: SetImageHandle, m_pImages
	//-----------------------------------------------------------------------
	CXTPResourceImage* LoadImage(LPCTSTR lpszFileName);

	//-----------------------------------------------------------------------
	// Summary:
	//     Retrieves frame small icon.
	// Parameters:
	//     pFrame - Window to retrieve icon.
	// Returns:
	//     Frame small icon.
	// See Also: GetFrameLargeIcon
	//-----------------------------------------------------------------------
	HICON GetFrameSmallIcon(CWnd* pFrame);

	//-----------------------------------------------------------------------
	// Summary:
	//     Retrieves frame large icon.
	// Parameters:
	//     pFrame - Window to retrieve icon.
	// Returns:
	//     Frame large icon.
	// See Also: GetFrameLargeIcon
	//-----------------------------------------------------------------------
	HICON GetFrameLargeIcon(CWnd* pFrame);


public:
	BOOL m_bFrameStatusBar;                     // TRUE if status bar used to draw.
	BOOL m_bRoundedCornersAlways;

	CXTPPaintManagerFont m_fontFrameCaption;    // Font used to draw text in the frame caption

public:
	int m_nFrameCaptionHeight;                  // Frame caption

	COLORREF m_clrFrameBorderActive0;           // Border color
	COLORREF m_clrFrameBorderActive1;           // Border color
	COLORREF m_clrFrameBorderActive2;           // Border color
	COLORREF m_clrFrameBorderActive3;           // Border color
	COLORREF m_clrFrameBorderInactive0;         // Border color
	COLORREF m_clrFrameBorderInactive1;         // Border color
	COLORREF m_clrFrameBorderInactive2;         // Border color
	COLORREF m_clrFrameBorderInactive3;         // Border color
	COLORREF m_clrFrameCaptionTextActive;       // Caption text color
	COLORREF m_clrFrameCaptionTextInActive;     // Caption text color
	COLORREF m_clrFrameCaptionTextActiveTitle;  // Caption text color
	CXTPPaintManager* m_pPaintManager;          // Parent Paint Manager.

	BOOL m_bFlatFrame;                          // TRUE to use a flat frame, FALSE otherwise.
};


//===========================================================================
// Summary:
//     The CXTPRibbonSystemFrameTheme class is used to enable a system style theme for Command Bars.
// See Also: CXTPPaintManager::SetTheme
//===========================================================================
class _XTP_EXT_CLASS CXTPRibbonSystemFrameTheme : public CXTPFramePaintManager
{
public:

	//-----------------------------------------------------------------------
	// Summary: Constructs a CXTPRibbonSystemFrameTheme object.
	// Input:   pPaintManager - Reference to a CXTPPaintManager object.
	//-----------------------------------------------------------------------
	CXTPRibbonSystemFrameTheme(CXTPPaintManager* pPaintManager);

	//-----------------------------------------------------------------------
	// Summary:
	//     Destroys a CXTPRibbonSystemFrameTheme object, handles cleanup and
	//     deallocation
	//-----------------------------------------------------------------------
	~CXTPRibbonSystemFrameTheme();

public:
	//-----------------------------------------------------------------------
	// Summary:
	//     This method is called to draw frame if Ribbon was found
	// Parameters:
	//      pDC - Pointer to device context
	//      pFrameHook - CXTPOffice2007FrameHook hook object
	//-----------------------------------------------------------------------
	void DrawFrame(CDC* pDC, CXTPOffice2007FrameHook* pFrameHook);

	//-----------------------------------------------------------------------
	// Summary:
	//     This method is called to draw caption button
	// Parameters:
	//      pDC - Pointer to device context
	//      rc - Bounding rectangle of the button
	//      nId - Identifier of the button
	//      bSelected - TURE if button is highlighted
	//      bPressed -  TURE if button is pressed
	//      bActive - TURE if frame is active
	//-----------------------------------------------------------------------
	void DrawFrameCaptionButton(CDC* pDC, CRect rc, int nId, BOOL bSelected, BOOL bPressed, BOOL bActive);

	//-----------------------------------------------------------------------
	// Summary:
	//     This method is called to refresh the visual metrics of the manager.
	//-----------------------------------------------------------------------
	void RefreshMetrics();

	//-----------------------------------------------------------------------
	// Summary:
	//     Renders and markup strings in the ribbon.
	// Parameters:
	//     pDC - Pointer to a valid device context.
	//     rc - Bounding rectangle for the markup.
	//     lpszText - Markup string to render.
	//-----------------------------------------------------------------------
	void RenderMarkup(CDC* pDC, CRect rc, LPCTSTR lpszText);


protected:
	CXTPMarkupContext* m_pMarkupContext;  // Store pointer to CXTPMarkupContext object.
};


//===========================================================================
// Summary:
//     The CXTPControlGalleryOffice2007Theme class is used to enable a Gallery Office 2007 style theme for Command Bars.
// See Also: CXTPPaintManager::SetTheme
//===========================================================================
class _XTP_EXT_CLASS CXTPControlGalleryOffice2007Theme : public CXTPControlGalleryPaintManager
{
public:
	//-----------------------------------------------------------------------
	// Summary: Constructs a CXTPControlGalleryOffice2007Theme object.
	// Input:   pPaintManager - Reference to a CXTPPaintManager object.
	//-----------------------------------------------------------------------
	CXTPControlGalleryOffice2007Theme(CXTPPaintManager* pPaintManager);

public:
	//-----------------------------------------------------------------------
	// Summary:
	//     This method is called to draw background of the gallery
	// Parameters:
	//      pDC - Pointer to device context
	//      pGallery - CXTPControlGallery object to draw
	//      rc - Bounding rectangle of gallery object
	//-----------------------------------------------------------------------
	virtual void FillControl(CDC* pDC, CXTPControlGallery* pGallery, CRect rc);

	//-----------------------------------------------------------------------
	// Summary:
	//     This method called to draw scrollbar of inplace gallery
	// Parameters:
	//     pDC - Pointer to device context
	//     pGallery - CXTPControlGallery object to draw
	// See Also: DrawScrollBar
	//-----------------------------------------------------------------------
	virtual void DrawPopupScrollBar(CDC* pDC, CXTPControlGallery* pGallery);

	//-----------------------------------------------------------------------
	// Summary:
	//     This method is called to draw scrollbar of the gallery.
	// Parameters:
	//     pDC - Pointer to device context.
	//     pGallery - ScrollBar to draw.
	//-----------------------------------------------------------------------
	virtual void DrawScrollBar(CDC* pDC, CXTPScrollBase* pGallery);

	//-----------------------------------------------------------------------
	// Summary:
	//     This method is called to draw label control
	// Parameters:
	//     pDC - Pointer to device context
	//     pLabel - Label item
	//     rc - Bounding rectangle of the label
	//-----------------------------------------------------------------------
	virtual void DrawLabel(CDC* pDC, CXTPControlGalleryItem* pLabel, CRect rc);

	//-----------------------------------------------------------------------
	// Summary:
	//     This method is called to refresh the visual metrics of the manager.
	//-----------------------------------------------------------------------
	virtual void RefreshMetrics();

protected:
	COLORREF m_clrControlGallerySelected;   // Selected item color.
	COLORREF m_clrControlGalleryNormal;     // Normal item color.
	COLORREF m_clrControlGalleryBorder;     // Border color.

};


AFX_INLINE int CXTPFramePaintManager::GetFrameCaptionHeight() const {
	return m_nFrameCaptionHeight;
}



#endif // #if !defined(__XTPOFFICE2007THEME_H__)
