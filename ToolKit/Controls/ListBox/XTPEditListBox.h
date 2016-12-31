// XTPEditListBox.h interface for the CXTPEditListBox class.
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

//{{AFX_CODEJOCK_PRIVATE
#if !defined(__XTEDITLISTBOX_H__)
#define __XTEDITLISTBOX_H__
//}}AFX_CODEJOCK_PRIVATE

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

//////////////////////////////////////////////////////////////////////
// ---------------------------------------------------------------------
// Summary:
//     CXTPEditListBoxToolBar is a CStatic derived class. It used by the
//     CXTPEditListBox class to create a toolbar above the edit list box
//     to display icons for editing.
// Remarks:
//     CXTPEditListBoxToolBar can be used for other classes by
//     setting the notify window in Initialize. This window will receive
//     notification messages whenever the new, delete, up, and down
//     buttons are pressed. You can handle these messages by adding an
//     ON_BN_CLICKED handler for each of the buttons XTP_IDC_BTN_NEW,
//     XTP_IDC_BTN_DELETE, XTP_IDC_BTN_UP and XTP_IDC_BTN_DOWN.
// ---------------------------------------------------------------------
class _XTP_EXT_CLASS CXTPEditListBoxToolBar : public CStatic
{
	DECLARE_DYNAMIC(CXTPEditListBoxToolBar)

public:
	//-----------------------------------------------------------------------
	// Summary:
	//     Constructs a CXTPEditListBoxToolBar object
	//-----------------------------------------------------------------------
	CXTPEditListBoxToolBar();

	//-----------------------------------------------------------------------
	// Summary:
	//     Destroys a CXTPEditListBoxToolBar object, handles cleanup and deallocation
	//-----------------------------------------------------------------------
	virtual ~CXTPEditListBoxToolBar();


public:

	// ------------------------------------------------------------------------
	// Summary:
	//     Initializes the CXTPEditListBoxToolBar control.
	// Parameters:
	//     bAutoFont -  True to enable automatic font initialization.
	// Remarks:
	//     Call this member function to initialize the edit group control. This
	//     method should be called directly after creating or sub-classing the
	//     control.
	// ------------------------------------------------------------------------
	virtual void Initialize(bool bAutoFont = true);

	// ---------------------------------------------------------------------
	// Summary:
	//     This member function returns a reference to the new button of the
	//     edit group.
	// Returns:
	//     A reference to a CXTPButton object.
	// ---------------------------------------------------------------------
	CXTPButton& GetNewButton();

	// --------------------------------------------------------------------
	// Summary:
	//     This member function returns a reference to the delete button of
	//     the edit group.
	// Returns:
	//     A reference to a CXTPButton object.
	// --------------------------------------------------------------------
	CXTPButton& GetDeleteButton();

	// --------------------------------------------------------------------
	// Summary:
	//     This member function returns a reference to the up button of the
	//     edit group.
	// Returns:
	//     A reference to a CXTPButton object.
	// --------------------------------------------------------------------
	CXTPButton& GetUpButton();

	// ----------------------------------------------------------------------
	// Summary:
	//     This member function returns a reference to the down button of the
	//     edit group.
	// Returns:
	//     A reference to a CXTPButton object.
	// ----------------------------------------------------------------------
	CXTPButton& GetDownButton();

	//-----------------------------------------------------------------------
	// Summary:
	//     This member function will enable or disable editing.
	// Parameters:
	//     bEnable - True to enable editing.
	//-----------------------------------------------------------------------
	void EnableEdit(bool bEnable);

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this member to switch the visual theme of the control.
	// Parameters:
	//     eTheme - New visual theme. Can be any of the values listed in the Remarks section.
	// Remarks:
	//     nStyle can be one of the following:
	//     * <b>xtpControlThemeDefault</b> Use default theme.
	//     * <b>xtpControlThemeFlat</b> Flat appearance style.
	//     * <b>xtpControlThemeUltraFlat</b> Ultra flat appearance style.
	//     * <b>xtpControlThemeOffice2000</b> Office 2000 appearance style.
	//     * <b>xtpControlThemeOfficeXP</b> Office XP appearance style.
	//     * <b>xtpControlThemeOffice2003</b> Office 2003 appearance style.
	//     * <b>xtpControlThemeResource</b> Office 2007 appearance style.
	// Returns:
	//     The version that accepts a XTPControlTheme style returns a pointer to the
	//     newly set theme, otherwise has no return value.
	//-----------------------------------------------------------------------
	void SetButtonTheme(XTPControlTheme eTheme);

	//{{AFX_CODEJOCK_PRIVATE
	//{{AFX_VIRTUAL(CXTPEditListBoxToolBar)
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL
	//}}AFX_CODEJOCK_PRIVATE

	//-----------------------------------------------------------------------
	// Summary:
	//     This member function is called by the CXTPEditListBoxToolBar object to
	//     render text display for the control.
	// Parameters:
	//     pDC      - Pointer to a valid device context.
	//     rcClient - Area to draw text on.
	//-----------------------------------------------------------------------
	virtual void DrawText(CDC* pDC, CRect& rcClient);

	// ------------------------------------------------------------------
	// Summary:
	//     Recalculates the button layout within the CXTPEditListBoxToolBar window.
	// Remarks:
	//     This member function is called by the CXTPEditListBoxToolBar object to
	//     position the group bar buttons when the window is sized.
	// ------------------------------------------------------------------
	virtual void MoveButtons();

	// ---------------------------------------------------------------------
	// Summary:
	//     Sends notification to the owner window.
	// Parameters:
	//     nCmdID -  Command ID to send.
	// Remarks:
	//     This member function sends the command specified by <i>nCmdID</i>
	//     to the owner of the CXTPEditListBoxToolBar object. The command is
	//     sent whenever a button is pressed on the group bar.
	// ---------------------------------------------------------------------
	virtual void SendCommand(UINT nCmdID);

	// --------------------------------------------------------------------
	// Summary:
	//     This member function returns a reference to the tooltip control
	// Returns:
	//     A reference to a CToolTipCtrl object.
	// --------------------------------------------------------------------
	CToolTipCtrl& GetTooltipControl();

protected:
//{{AFX_CODEJOCK_PRIVATE
	DECLARE_MESSAGE_MAP()

	//{{AFX_MSG(CXTPEditListBoxToolBar)
	afx_msg void OnButtonNew();
	afx_msg void OnButtonDelete();
	afx_msg void OnButtonUp();
	afx_msg void OnButtonDown();
	afx_msg void OnWindowPosChanged(WINDOWPOS FAR* lpwndpos);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnNcPaint();
	afx_msg void OnEnable(BOOL bEnable);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	//}}AFX_MSG
//}}AFX_CODEJOCK_PRIVATE

protected:
	bool m_bShowUpDownButtons;     // Controls whether of not the up.down buttons are shown.
	bool m_bShowNewDeleteButtons;     // Controls whether of not the up.down buttons are shown.
	bool           m_bEnableEdit;   // True if editing is enabled.
	CRect          m_arClipRect[4]; // Array of toolbar button sizes.
	CXTPButton     m_arButton[4];   // Array of toolbar buttons.
	CXTPIconHandle m_arIcon[4];     // Array of toolbar button icons.
	CToolTipCtrl   m_tooltip;       // Tooltip control for edit buttons.

	friend class CXTPEditListBox;
};

//////////////////////////////////////////////////////////////////////

AFX_INLINE CXTPButton& CXTPEditListBoxToolBar::GetNewButton() {
	return m_arButton[0];
}
AFX_INLINE CXTPButton& CXTPEditListBoxToolBar::GetDeleteButton() {
	return m_arButton[1];
}
AFX_INLINE CXTPButton& CXTPEditListBoxToolBar::GetUpButton() {
	return m_arButton[2];
}
AFX_INLINE CXTPButton& CXTPEditListBoxToolBar::GetDownButton() {
	return m_arButton[3];
}
AFX_INLINE void CXTPEditListBoxToolBar::EnableEdit(bool bEnable) {
	m_bEnableEdit = bEnable;
}
AFX_INLINE CToolTipCtrl& CXTPEditListBoxToolBar::GetTooltipControl() {
		return m_tooltip;
}


// forwards

class CXTPItemEdit;

const DWORD LBS_XTP_DEFAULT         = 0x0000;  //<ALIAS CXTPEditListBox::SetListEditStyle@UINT@DWORD>
const DWORD LBS_XTP_CHOOSEDIR       = 0x0001;  //<ALIAS CXTPEditListBox::SetListEditStyle@UINT@DWORD>
const DWORD LBS_XTP_CHOOSEFILE      = 0x0002;  //<ALIAS CXTPEditListBox::SetListEditStyle@UINT@DWORD>
const DWORD LBS_XTP_NOTOOLBAR       = 0x0008;  //<ALIAS CXTPEditListBox::SetListEditStyle@UINT@DWORD>
const DWORD LBS_XTP_BROWSE          = 0x0010; // Browse button
const DWORD LBS_XTP_HIDE_UP_DOWN    = 0x0020; // Hide Up/Down buttons
const DWORD LBS_XTP_ONLY_UP_DOWN    = 0x0040; // Only Up/Down buttons
const DWORD LBS_XTP_BROWSE_ONLY     = 0x0080; // Browse button

//===========================================================================
// Summary:
//     CXTPEditListBox is a CXTPListBox derived class. It is used to create an
//     editable list box. This list box can be configured to display a toolbar
//     for editing. You can define browse styles to search for files or folders.
//     Each entry is made editable with a double mouse click.
//===========================================================================
class _XTP_EXT_CLASS CXTPEditListBox : public CXTPListBox
{
	DECLARE_DYNAMIC(CXTPEditListBox)

public:
	//-----------------------------------------------------------------------
	// Summary:
	//     Constructs a CXTPEditListBox object
	//-----------------------------------------------------------------------
	CXTPEditListBox();

	//-----------------------------------------------------------------------
	// Summary:
	//     Destroys a CXTPEditListBox object, handles cleanup and deallocation
	//-----------------------------------------------------------------------
	virtual ~CXTPEditListBox();

public:

	// -----------------------------------------------------------------------------
	// Summary:
	//     Sets the edit style for the edit list box.
	// Parameters:
	//     lpszTitle -  NULL terminated string that represents the caption title.
	//     nTitle -     Resource ID of the string to load for the caption title.
	//     dwLStyle -   Style for the list edit control. Pass in LBS_XTP_NOTOOLBAR
	//                  if you do not wish the caption edit navigation control bar
	//                  to be displayed.
	// Remarks:
	//     Call this member function to set the style and title for the edit
	//     list box. The style of the edit list box can be set to one or more
	//     of the following values:<p/>
	//
	//     * <b>LBS_XTP_DEFAULT</b> Standard edit field.
	//     * <b>LBS_XTP_CHOOSEDIR</b> Choose directory browse edit field.
	//     * <b>LBS_XTP_CHOOSEFILE</b> Choose file browse edit field.
	//     * <b>LBS_XTP_NOTOOLBAR</b> Do not display edit toolbar.
	// -----------------------------------------------------------------------------
	void SetListEditStyle(UINT nTitle, DWORD dwLStyle = LBS_XTP_DEFAULT);
	void SetListEditStyle(LPCTSTR lpszTitle, DWORD dwLStyle = LBS_XTP_DEFAULT); //<combine CXTPEditListBox::SetListEditStyle@UINT@DWORD>

	// --------------------------------------------------------------------
	// Summary:
	//     Retrieves the current item index.
	// Returns:
	//     An integer value that represents the edit control index.
	// Remarks:
	//     Call this member function to get the current index for the edit
	//     control. Similar to GetCurSel; however, the current index is the
	//     index of the last item to be modified or added to the edit list
	//     box and not necessarily the selected item.
	// --------------------------------------------------------------------
	int GetCurrentIndex();

	//-----------------------------------------------------------------------
	// Summary:
	//     This member function will enable editing for the list box item.
	// Parameters:
	//     iItem - Index of the item to edit.
	//-----------------------------------------------------------------------
	void EditItem(int iItem);

	// --------------------------------------------------------------------
	// Summary:
	//     Retrieves the edited item's text label.
	// Remarks:
	//     This member function is called to retrieve the text for the item
	//     that is being edited in the list box and save the value to
	//     m_strItemText.
	// --------------------------------------------------------------------
	virtual void GetEditItemText();

	// --------------------------------------------------------------------
	// Summary:
	//     This method is called to set inplace edit text
	// Parameters:
	//     pcszText - next text to set
	// --------------------------------------------------------------------
	void SetEditText(LPCTSTR pcszText);

	// --------------------------------------------------------------------
	// Summary:
	//     Returns a pointer to the CXTPEditListBoxToolBar toolbar.
	// Returns:
	//     A reference to a CXTPEditListBoxToolBar object.
	// Remarks:
	//     Call this member function to return a reference to the
	//     CXTPEditListBoxToolBar control that is associated with the edit list box.
	// --------------------------------------------------------------------
	CXTPEditListBoxToolBar& GetEditGroup();

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this member function to set the default filter for the
	//     file dialog.
	// Parameters:
	//     lpszFilter - Points to a NULL terminated string that represents
	//                 the file filter used by the file open dialog.
	//-----------------------------------------------------------------------
	void SetDlgFilter(LPCTSTR lpszFilter = NULL);

	// --------------------------------------------------------------------------
	// Summary:
	//     This member function sets the initial directory for the file dialog.
	// Parameters:
	//     lpszInitialDir -  [in] Points to a NULL terminated string the represents the
	//                  initial directory of the file open dialog..
	// --------------------------------------------------------------------------
	void SetDlgInitialDir(LPCTSTR lpszInitialDir);

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this member function to determine if the edit list has a toolbar.
	// Returns:
	//     true if the toolbar is turned on, otherwise returns false.
	//-----------------------------------------------------------------------
	bool HasToolbar();

	//-----------------------------------------------------------------------
	// Summary:
	//     This member function will enable or disable editing.
	// Parameters:
	//     bEnable - True to enable editing.
	//-----------------------------------------------------------------------
	void EnableEdit(bool bEnable);

	// ---------------------------------------------------------------------
	// Summary:
	//     Initializes the CXTPEditListBox control.
	// Parameters:
	//     bAutoFont -  True to enable automatic font initialization.
	// Remarks:
	//     Call this member function to initialize the list box. This method
	//     should be called directly after creating or sub-classing the
	//     control.
	// ---------------------------------------------------------------------
	virtual void Initialize(bool bAutoFont = true);

	// -------------------------------------------------------------------
	// Summary:
	//     Recalculates the toolbar layout for the CXTPEditListBox.
	// Remarks:
	//     Call this member function to correctly reposition the edit list
	//     box toolbar. This will readjust the layout to correctly and
	//     position the toolbar in relation to the list.
	// -------------------------------------------------------------------
	virtual void RecalcLayout();

	// -------------------------------------------------------------
	// Summary:
	//     Sets the default text for new items.
	// Parameters:
	//     lpszItemDefaultText -  NULL terminated string.
	// Remarks:
	//     Call this member function to set the default text that is
	//     displayed when a new item is added to the edit list box.
	// -------------------------------------------------------------
	void SetNewItemDefaultText(LPCTSTR lpszItemDefaultText);

	// ----------------------------------------------------------------------
	// Summary:
	//     Moves item up
	// Parameters:
	//     nIndex - Item index to move
	// ----------------------------------------------------------------------
	virtual void MoveItemUp(int nIndex);

	// ----------------------------------------------------------------------
	// Summary:
	//     Moves item down
	// Parameters:
	//     nIndex - Item index to move
	// ----------------------------------------------------------------------
	virtual void MoveItemDown(int nIndex);

protected:

	//-----------------------------------------------------------------------
	// Summary:
	//     This member function will create the edit group control.
	// Parameters:
	//     bAutoFont - True to enable automatic font initialization.
	// Returns:
	//     TRUE if successful, otherwise returns FALSE.
	//-----------------------------------------------------------------------
	virtual BOOL CreateEditGroup(bool bAutoFont = true);

	// ----------------------------------------------------------------------
	// Summary:
	//     Enables editing for the currently selected item.
	// Parameters:
	//     bNewItem -  TRUE to add a new item.
	// Remarks:
	//     This member function will enable editing for the currently
	//     selected list box item. If 'bNewItem' is TRUE, a new item is added
	//     to the end of the list box.
	// ----------------------------------------------------------------------
	virtual void EditListItem(BOOL bNewItem);

	// ----------------------------------------------------------------------
	// Summary:
	//     Deletes currently selected item
	// ----------------------------------------------------------------------
	virtual void DeleteItem();

	// ----------------------------------------------------------------------
	// Summary:
	//     This method is called to create in-place Edit control
	// Parameters:
	//     rcItem - Bounding rectangle of edit control.
	// ----------------------------------------------------------------------
	virtual CXTPItemEdit* CreateEditControl(CRect rcItem);

protected:
//{{AFX_CODEJOCK_PRIVATE
	DECLARE_MESSAGE_MAP()

	//{{AFX_VIRTUAL(CXTPEditListBox)
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual COLORREF GetBackColor();
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CXTPEditListBox)
	afx_msg void OnEndLabelEdit();
	afx_msg void OnItemBrowse();
	afx_msg void OnNewItem();
	afx_msg void OnDeleteItem();
	afx_msg void OnMoveItemUp();
	afx_msg void OnMoveItemDown();
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnNcLButtonDown(UINT nHitTest, CPoint point);
	afx_msg void OnNcMButtonDown(UINT nHitTest, CPoint point);
	afx_msg void OnNcRButtonDown(UINT nHitTest, CPoint point);
	afx_msg void OnWindowPosChanged(WINDOWPOS FAR* lpwndpos);
	afx_msg void OnEnable(BOOL bEnable);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
//}}AFX_CODEJOCK_PRIVATE

protected:
	CWnd* m_pParentWnd;             // Pointer to the parent window.
	CString m_strItemDefaultText;   // Default text used when new items are created.
	int             m_nIndex;       // Current index when edit functions are performed.
	BOOL            m_bNewItem;     // TRUE if a new item is being entered into the list box.
	bool            m_bEnableEdit;  // True if editing is enabled.
	DWORD           m_dwLStyle;     // List edit styles.
	CString         m_strTitle;     // Caption area title.
	CString         m_strFilter;    // Default file filter.
	CString         m_strInitialDir;   // Initial Dir.
	CString         m_strItemText;  // Current text of a selected item during edit.
	CXTPItemEdit*    m_pItemEdit;    // Points to the in-place edit item.
	CXTPEditListBoxToolBar    m_editGroup;    // The edit group (toolbar) that appears above the list box.

};

//////////////////////////////////////////////////////////////////////

AFX_INLINE int CXTPEditListBox::GetCurrentIndex() {
	return m_nIndex;
}
AFX_INLINE CXTPEditListBoxToolBar& CXTPEditListBox::GetEditGroup() {
	return m_editGroup;
}
AFX_INLINE void CXTPEditListBox::SetDlgFilter(LPCTSTR lpszFilter/*=NULL*/) {
	m_strFilter = lpszFilter;
}
AFX_INLINE void CXTPEditListBox::SetDlgInitialDir(LPCTSTR lpszInitialDir/*=NULL*/) {
	m_strInitialDir = lpszInitialDir;
}
AFX_INLINE bool CXTPEditListBox::HasToolbar() {
	return ((m_dwLStyle & LBS_XTP_NOTOOLBAR) == 0);
}
AFX_INLINE void CXTPEditListBox::EnableEdit(bool bEnable) {
	m_bEnableEdit = bEnable; m_editGroup.EnableEdit(bEnable);
}
AFX_INLINE void CXTPEditListBox::SetNewItemDefaultText(LPCTSTR lpszItemDefaultText) {
	m_strItemDefaultText = lpszItemDefaultText;
}

#endif // #if !defined(__XTEDITLISTBOX_H__)
