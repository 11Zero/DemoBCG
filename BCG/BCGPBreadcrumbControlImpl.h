#if !defined(__BCGPBREADCRUMBCONTROLIMPL_H)
#define __BCGPBREADCRUMBCONTROLIMPL_H

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

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"

#define BREADCRUMBCONTROL_CLASSNAME     _T("BCGPBreadcrumbControl")

// Specifies whether root item is shown in the breadcrumb control.
// Without this style root item is shown only when selected.
// Either way it is accessible through leftmost dropdown list.
#define BCCS_SHOWROOTALWAYS     0x0800L

// Specifies that a highlighted item will be selected when the user presses the ENTER key.
// Without this style, pressing the ENTER key has the same effect as pressing the dialog box’s default pushbutton.
#define BCCS_WANTRETURN         0x1000L

// Specifies that the top and bottom border for each items will not be painted.
// Use this style flag if the item border color
// coincides with the background color of parent dialog or if WS_BORDER style used.
#define BCCS_EXTERNALBORDER     0x2000L

// Enables in-place editing of current path.
// The editor appears when the user presses F2 key or clicks on an empty space inside the control.
#define BCCS_INPLACEEDIT        0x4000L

//////////////////////////////////////////////////////////////////////////
//                      Breadcrumb Control Structures
//////////////////////////////////////////////////////////////////////////

struct _BREADCRUMBITEM;
typedef struct _BREADCRUMBITEM *HBREADCRUMBITEM;

typedef struct tagBREADCRUMBITEMINFO {
	UINT            mask;           // See BCCIF_xxxx constants
	HBREADCRUMBITEM hParentItem;    // ignored for custom list items
	LPTSTR          pszText;
	int             cchTextMax;
	LPTSTR          pszTooltipText;
	int             cchTooltipTextMax;
	int             iImage;
	BOOL            bDynamic;
	LPARAM          lParam;
	DWORD_PTR       dwReserved;
} BREADCRUMBITEMINFO, *PBREADCRUMBITEMINFO;

#define BCCIF_PARENT        0x0001
#define BCCIF_TEXT          0x0002
#define BCCIF_TOOLTIP       0x0004
#define BCCIF_IMAGE         0x0008
#define BCCIF_DYNAMIC       0x0010
#define BCCIF_PARAM         0x0020

//////////////////////////////////////////////////////////////////////////
//                      Breadcrumb Control Messages
//////////////////////////////////////////////////////////////////////////

// BCCM_SETITEM
//      wParam = HBREADCRUMBITEM handle to the item.
//      lParam = Pointer to a BREADCRUMBITEMINFO structure that contains the new item attributes.
//      Returns TRUE if successful, or FALSE otherwise.
#define BCCM_SETITEM                0x2400L

// BCCM_GETITEM
//      wParam = HBREADCRUMBITEM handle to the item.
//      lParam = Pointer to a BREADCRUMBITEMINFO structure that specifies the information to retrieve and receives information about the item.
//      Returns TRUE if successful, or FALSE otherwise.
#define BCCM_GETITEM                0x2401L

// BCCM_INSERTITEM
//      wParam = 0
//      lParam = Pointer to a BREADCRUMBITEMINFO structure that specifies the attributes of the new item. 
//      Returns the HBREADCRUMBITEM handle to the new item if successful, or NULL otherwise.
#define BCCM_INSERTITEM             0x2402L

// BCCM_DELETEITEM
//      wParam = 0
//      lParam = HBREADCRUMBITEM handle to the item to delete. If set to NULL, all items are deleted.
//      Returns TRUE if successful, or FALSE otherwise.
#define BCCM_DELETEITEM             0x2403L

// BCCM_GETITEMRECT
//      wParam = HBREADCRUMBITEM handle to the item.
//      lParam = Pointer to a RECT structure that receives the bounding rectangle. The coordinates are relative to the upper-left corner of the breadcrumb control.
//      Returns TRUE if the item is visible and the bounding rectangle was successfully retrieved, the return value is TRUE. Otherwise the message returns FALSE and does not retrieve the bounding rectangle.
#define BCCM_GETITEMRECT            0x2404L

// BCCM_GETITEMARROWRECT
//      wParam = HBREADCRUMBITEM handle to the item.
//      lParam = Pointer to a RECT structure that receives the bounding rectangle. The coordinates are relative to the upper-left corner of the breadcrumb control.
//      Returns TRUE if the item's arrow is visible and the bounding rectangle was successfully retrieved, the return value is TRUE. Otherwise, the message returns FALSE and does not retrieve the bounding rectangle.
#define BCCM_GETITEMARROWRECT       0x2405L

// BCCM_GETSUBITEMCOUNT
//      wParam = 0
//      lParam = HBREADCRUMBITEM handle to the item.
//      Returns count of item subitems.
#define BCCM_GETSUBITEMCOUNT        0x2406L

// BCCM_GETSUBITEM
//      wParam = Index of subitem to retrieve.
//      lParam = HBREADCRUMBITEM handle to the item.
//      Returns the HBREADCRUMBITEM handle to subitem or NULL.
#define BCCM_GETSUBITEM             0x2407L

// BCCM_GETROOTITEM
//      wParam = 0
//      lParam = 0
//      Returns the HBREADCRUMBITEM handle to the root item
#define BCCM_GETROOTITEM            0x2408L

// BCCM_RESETCONTENT
//      wParam = 0
//      lParam = 0
//      Returns TRUE if successful.
#define BCCM_RESETCONTENT           0x2409L

// BCCM_SELECTITEM
//      wParam = 0
//      lParam = HBREADCRUMBITEM handle to the item to set active
//      Returns the handle to the previous active item
#define BCCM_SELECTITEM             0x240AL

// BCCM_GETSELECTEDITEM
//      wParam = 0
//      lParam = 0
//      Returns the HBREADCRUMBITEM handle to the current active item
#define BCCM_GETSELECTEDITEM        0x240BL


// BCCM_SETTEXTCOLOR
//      wParam = 0
//      lParam = COLORREF value that contains the new text color. If this argument is -1, the control will revert to using the system color for the text color.
//      Returns a COLORREF value that represents the previous text color. If this value is -1, the control was using the system color for the text color.
#define BCCM_SETTEXTCOLOR           0x240CL

// BCCM_GETTEXTCOLOR
//      wParam = 0
//      lParam = 0
//      Returns a COLORREF value that represents the current text color. If this value is -1, the control is using the system color for the text color.
#define BCCM_GETTEXTCOLOR           0x240DL

// BCCM_SETBKCOLOR
//      wParam = 0
//      lParam = COLORREF value that contains the new background color. If this argument is -1, the control will revert to using the system color for the background color.
//      Returns a COLORREF value that represents the previous background color. If this value is -1, the control was using the system color for the background color.
#define BCCM_SETBKCOLOR             0x240EL

// BCCM_GETBKCOLOR
//      wParam = 0
//      lParam = 0
//      Returns a COLORREF value that represents the current background color. If this value is -1, the control is using the system color for the background color.
#define BCCM_GETBKCOLOR             0x240FL

// BCCM_SETIMAGELIST
//      wParam = 0
//      lParam = Handle to the image list. If himl is NULL, the message removes the specified image list from the breadcrumb control
//      Returns the handle to the previous image list, if any, or NULL otherwise.
// The breadcrumb control will not destroy the image list specified with this message. Your application must destroy the image list when it is no longer needed.
#define BCCM_SETIMAGELIST           0x2410L

// BCCM_GETIMAGELIST
//      wParam = 0
//      lParam = 0
//      Returns an HIMAGELIST handle to the specified image list.
#define BCCM_GETIMAGELIST           0x2411L

// BCCM_SETTOOLTIPS
//      wParam = Handle to a ToolTip control.
//      lParam = 0.
//      Returns handle to ToolTip control previously set for the tree-view control,
//          or NULL if ToolTips were not previously used.
#define  BCCM_SETTOOLTIPS           0x2412L

// BCCM_GETTOOLTIPS
//      wParam = 0
//      lParam = 0
//      Returns the handle to the child ToolTip control, or NULL if the control is not using ToolTips.
#define  BCCM_GETTOOLTIPS           0x2413L

// BCCM_SETRIGHTMARGIN
//      wParam = (UINT)width of control's empty part in pixels
//      lParam = 0
//      Returns previous value of right margin size.
#define BCCM_SETRIGHTMARGIN         0x2414L

// BCCM_GETRIGHTMARGIN
//      wParam = TRUE to get actual value according to current control size and layout, FALSE to get value set by user.
//      lParam = 0
//      Returns width of control`s empty part in pixels.
#define BCCM_GETRIGHTMARGIN         0x2415L

// BCCM_SETHILITECOLOR
//      wParam = 0
//      lParam = COLORREF value that contains the item highlight color. If this argument is -1, the control will revert to using the default color.
//      Returns a COLORREF value that represents the previous background color. If this value is -1, the control was using the default color.
#define BCCM_SETHILITECOLOR         0x2416L

// BCCM_GETHILITECOLOR
//      wParam = 0
//      lParam = 0
//      Returns a COLORREF value that represents the current item highlight color. If this value is -1, the control is using the default color.
#define BCCM_GETHILITECOLOR         0x2417L


// BCCM_SETHILITETEXTCOLOR
//      wParam = 0
//      lParam = COLORREF value that contains the new highlighted text color. If this argument is -1, the control will revert to using the default text color.
//      Returns a COLORREF value that represents the previous text color. If this value is -1, the control was using the default color.
#define BCCM_SETHILITETEXTCOLOR     0x2418L

// BCCM_GETHILITETEXTCOLOR
//      wParam = 0
//      lParam = 0
//      Returns a COLORREF value that represents the current highlighted text color. If this value is -1, the default color.
#define BCCM_GETHILITETEXTCOLOR     0x2419L


// BCCM_HITTEST
//      wParam = 0
//      lParam = Pointer to a NMCLICK structure that receives the hit test info. The pt member of this structure specifies the position to hit test, in control client coordinates.
//      Returns TRUE if successful.
// If successful, dwHitInfo member receives the hit-test constant (see BCCHT_xxxxx constants) and dwItemSpec member receives the handle of an item if any or NULL.
// and dwItemSpec receives a handle to breadcrumb item at specified point or NULL.
#define BCCM_HITTEST                0x241AL


// BCCM_GETINPLACEEDITRECT
//      wParam = 0
//      lParam = Pointer to a RECT structure that receives the coordinates of rectangle in control client coordinates.
//      Returns TRUE if successful.
#define BCCM_GETINPLACEEDITRECT     0x241BL



#define BCCHT_OUT           0       // the point is out of control client area
#define BCCHT_EMPTY         1       // the hit-test point is in an empty area
#define BCCHT_ICON          2       // icon area
#define BCCHT_MENUBUTTON    3       //  << button area
#define BCCHT_ITEM          4       // item text part area (dwItemSpec receives HBREADCRUMBITEM handle)
#define BCCHT_ITEMBUTTON    5       // item dropdown button part area (dwItemSpec receives HBREADCRUMBITEM handle)
#define BCCHT_EDIT          6       // clicking this point may cause inplace editor appear

//////////////////////////////////////////////////////////////////////////
//                  Breadcrumb Control Notifications
//////////////////////////////////////////////////////////////////////////

typedef struct tagBREADCRUMB_MENU 
{
	NMHDR               nmhdr;
	HMENU               hMenu;
	HBREADCRUMBITEM     hItem;
	POINT               ptOrigin;
} BREADCRUMB_MENU;

typedef struct tagBREADCRUMB_ITEM_NOTIFICATION 
{
	NMHDR               nmhdr;
	HBREADCRUMBITEM     hItem;
} BREADCRUMB_ITEM_NOTIFICATION;

#define BCCN_FIRST              NM_FIRST

// BCCN_SELECTIONCHANGED
// Notifies a control's parent window that the active item has changed.
// This notification message is sent in the form of a WM_NOTIFY message.
//      (NMHDR*)lParam
//      Return value ignored
#define BCCN_SELECTIONCHANGED       (BCCN_FIRST + 0)

// BCCN_INITROOT
// Notifies a control's parent window that it can set up the root item.
// This notification message is sent in the form of a WM_NOTIFY message.
//      (NMHDR*)lParam
//      Return value ignored
#define BCCN_INITROOT               (BCCN_FIRST + 1)

// BCCN_MENUSHOW
// Notifies a parent window that the control menu was dropped down.
// This notification message is sent in the form of a WM_NOTIFY message.
//      (BREADCRUMB_MENU*)lParam
//      Return nonzero to prevent the control from showing menu.
#define BCCN_MENUSHOW               (BCCN_FIRST + 2)

// BCCN_MENUCLOSE
// Notifies a parent window that the control menu is about to close.
// This notification message is sent in the form of a WM_NOTIFY message.
//      (BREADCRUMB_MENU*)lParam
//      Return value ignored
#define BCCN_MENUCLOSE              (BCCN_FIRST + 3)

// BCCN_DYNAMIC_GETSUBITEMS
// Notifies a parent window that it should dynamically add the child items for the specified item.
// Use BCCM_INSERTITEM to add child items.
// This notification message is sent in the form of a WM_NOTIFY message.
//      (BREADCRUMB_ITEM_NOTIFICATION*)lParam - specifies the parent item info.
//      Return value ignored
#define BCCN_DYNAMIC_GETSUBITEMS    (BCCN_FIRST + 4)

// BCCN_BEGIN_INPLACE_EDITING
// Notifies a parent window that it must show the inplace path editor.
// This notification message is sent in the form of a WM_NOTIFY message.
//      Return value ignored
#define BCCN_BEGIN_INPLACE_EDITING  (BCCN_FIRST + 5)

#define BCCN_LAST                   BCCN_BEGIN_INPLACE_EDITING


class CBCGPBreadcrumbImpl;

CBCGPBreadcrumbImpl* AttachBreadcrumbImplementation(CWnd* pAttachTo);
LRESULT BreadcrumbWindowProc(CBCGPBreadcrumbImpl* pImplementation, UINT message, WPARAM wParam, LPARAM lParam, BOOL* bHandled);


#endif // __BCGPBREADCRUMBCONTROLIMPL_H
