// XTIncludes.h : header file
//
// This file is a part of the XTREME CONTROLS MFC class library.
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

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// common
#include "Controls/Resource.h"
#include "Controls/Defines.h"

// utility / helpers
#include "Controls/Util/XTPGlobal.h"
#include "Controls/Util/XTPUtil.h"
#include "Controls/Util/XTPFunctions.h"
#include "Controls/Util/XTPAnimationDC.h"
#include "Controls/Util/XTPWindowPos.h"
#include "Controls/Util/XTPMemFile.h"
#include "Controls/Util/XTPRegistryManager.h"
#include "Controls/Util/XTPHelpers.h"
#include "Controls/Util/XTPControlTheme.h"

// button
#include "Controls/Button/XTPButton.h"
#include "Controls/Button/XTPButtonTheme.h"
#include "Controls/Button/XTPColorPicker.h"

// edit
#include "Controls/Edit/XTPEdit.h"
#include "Controls/Edit/XTPBrowseEdit.h"
#include "Controls/Edit/XTPHexEdit.h"
#include "Controls/Edit/XTPMaskEdit.h"

// listbox
#include "Controls/ListBox/XTPListBox.h"
#include "Controls/ListBox/XTPEditListBox.h"
#include "Controls/ListBox/XTPShortcutListBox.h"
#include "Controls/ListBox/XTPCheckListBox.h"

// combobox
#include "Controls/Combo/XTPComboBox.h"
#include "Controls/Combo/XTPFontComboBox.h"

// tab
#include "Controls/Tab/XTPTabBase.h"
#include "Controls/Tab/XTPTabCtrl.h"
#include "Controls/Tab/XTPTabCtrlButtons.h"
#include "Controls/Tab/XTPExcelTabCtrl.h"
#include "Controls/Tab/XTPTabView.h"
#include "Controls/Tab/XTPMDIWndTab.h"

// resize
#include "Controls/Resize/XTPResizeRect.h"
#include "Controls/Resize/XTPResizePoint.h"
#include "Controls/Resize/XTPResize.h"
#include "Controls/Resize/XTPResizeDialog.h"
#include "Controls/Resize/XTPResizeFormView.h"
#include "Controls/Resize/XTPResizePropertyPage.h"
#include "Controls/Resize/XTPResizePropertySheet.h"
#include "Controls/Resize/XTPResizeGroupBox.h"

// task dialog
#include "Controls/Dialog/XTPTaskDialogAPI.h"
#include "Controls/Dialog/XTPTaskDialogControls.h"
#include "Controls/Dialog/XTPTaskDialogClient.h"
#include "Controls/Dialog/XTPTaskDialogFrame.h"
#include "Controls/Dialog/XTPTaskDialog.h"

// dialog
#include "Controls/Dialog/XTPTipOfTheDay.h"
#include "Controls/Dialog/XTPBrowseDialog.h"
#include "Controls/Dialog/XTPColorPageCustom.h"
#include "Controls/Dialog/XTPColorPageStandard.h"
#include "Controls/Dialog/XTPWindowList.h"
#include "Controls/Dialog/XTPPropertyPage.h"
#include "Controls/Dialog/XTPPropertySheet.h"
#include "Controls/Dialog/XTPPropertyPageNavigator.h"
#include "Controls/Dialog/XTPColorDialog.h"

// popup window
#include "Controls/Popup/XTPColorSelectorCtrl.h"
#include "Controls/Popup/XTPColorPopup.h"
#include "Controls/Popup/XTPTipWindow.h"
#include "Controls/Popup/XTPPopupControl.h"
#include "Controls/Popup/XTPPopupItem.h"
#include "Controls/Popup/XTPPopupPaintManager.h"

// tree
#include "Controls/Tree/XTPTreeBase.h"
#include "Controls/Tree/XTPTreeCtrlView.h"

// header control
#include "Controls/Header/XTPHeaderCtrlTheme.h"
#include "Controls/Header/XTPHeaderCtrl.h"

// list control
#include "Controls/List/XTPSortClass.h"
#include "Controls/List/XTPListBase.h"
#include "Controls/List/XTPListCtrlView.h"

// shell
#include "Controls/Shell/XTPDropSource.h"
#include "Controls/Shell/XTPShellPidl.h"
#include "Controls/Shell/XTPShellSettings.h"
#include "Controls/Shell/XTPShellTreeBase.h"
#include "Controls/Shell/XTPShellTreeCtrlView.h"
#include "Controls/Shell/XTPDirWatcher.h"
#include "Controls/Shell/XTPTrayIcon.h"
#include "Controls/Shell/XTPShellListBase.h"
#include "Controls/Shell/XTPShellListCtrlView.h"

// scroll
#include "Controls/Scroll/XTPPageScroller.h"
#include "Controls/Scroll/XTPScrollBar.h"

// static
#include "Controls/Static/XTPHyperLink.h"
#include "Controls/Static/XTPSearchOptionsCtrl.h"
#include "Controls/Static/XTPCaptionTheme.h"
#include "Controls/Static/XTPCaption.h"
#include "Controls/Static/XTPCaptionPopupWnd.h"

// splitter
#include "Controls/Splitter/XTPSplitterWnd.h"
#include "Controls/Splitter/XTPSplitterWndTheme.h"

// progress
#include "Controls/Progress/XTPMarqueeCtrl.h"
#include "Controls/Progress/XTPProgressCtrl.h"

// spin
#include "Controls/Spin/XTPSpinButtonCtrl.h"

// deprecated
#include "Controls/Deprecated/Deprecated.h"
