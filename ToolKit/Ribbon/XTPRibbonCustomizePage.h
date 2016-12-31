// XTPRibbonCustomizePage.h: interface for the CXTPRibbonCustomizePage class.
//
// This file is a part of the XTREME RIBBON MFC class library.
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
#if !defined(__XTPRIBBONCUSTOMIZEPAGE_H__)
#define __XTPRIBBONCUSTOMIZEPAGE_H__
//}}AFX_CODEJOCK_PRIVATE

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CXTPCustomizeSheet;
class CXTPCommandBars;
class CXTPRibbonBar;

#include "CommandBars/XTPCustomizeCommandsPage.h"

//===========================================================================
// Summary:
//     CXTPRibbonCustomizePage is a CPropertyPage derived class.
//     It represents the Quick Access page of the Customize dialog.
//===========================================================================
class _XTP_EXT_CLASS CXTPRibbonCustomizePage : public CXTPPropertyPage
{
// Construction
public:
	//-----------------------------------------------------------------------
	// Summary:
	//     Constructs a CXTPRibbonCustomizePage object
	// Parameters:
	//     pSheet - Points to a CXTPCustomizeSheet object
	//-----------------------------------------------------------------------
	CXTPRibbonCustomizePage(CXTPCustomizeSheet* pSheet);

	//-----------------------------------------------------------------------
	// Summary:
	//     Destroys a CXTPCustomizeCommandsPage object, handles cleanup
	//     and deallocation.
	//-----------------------------------------------------------------------
	~CXTPRibbonCustomizePage();

public:

	//-----------------------------------------------------------------------
	// Summary:
	//     Retrieves Ribbon bar to customize
	// Returns:
	//     Pointer to commandbars' ribbon bar
	//-----------------------------------------------------------------------
	CXTPRibbonBar* GetRibbonBar();


public:
	//-----------------------------------------------------------------------
	// Summary:
	//     Adds categories branch from menu resource.
	// Parameters:
	//     nIDResource   - Menu resource from where categories will be built.
	//     bListSubItems - TRUE to add sub menus to categories.
	// Returns:
	//     TRUE if successful; otherwise returns FALSE
	//-----------------------------------------------------------------------
	BOOL AddCategories(UINT nIDResource,  BOOL bListSubItems = FALSE);

	//-----------------------------------------------------------------------
	// Summary:
	//     Adds categories branch from Controls.
	// Parameters:
	//     pControls - Points to a CXTPControls object
	// Returns:
	//     TRUE if successful; otherwise returns FALSE
	//-----------------------------------------------------------------------
	BOOL AddCategories(CXTPControls* pControls);

	//-----------------------------------------------------------------------
	// Summary:
	//     Adds a new category from a CMenu object.
	// Parameters:
	//     strCategory   - Category to be added.
	//     pMenu         - Points to a CMenu object
	//     bListSubItems - TRUE to add sub menus to the category.
	// Returns:
	//     TRUE if successful; otherwise returns FALSE
	//-----------------------------------------------------------------------
	BOOL AddCategory(LPCTSTR strCategory, CMenu* pMenu, BOOL bListSubItems = FALSE);

	//-----------------------------------------------------------------------
	// Summary:
	//     This method adds a new empty category in the given index.
	// Parameters:
	//     strCategory - Category to be added.
	//     nIndex      - Index to insert.
	// Returns:
	//     A pointer to a CXTPControls object
	//-----------------------------------------------------------------------
	CXTPControls* InsertCategory(LPCTSTR strCategory, int nIndex = -1);

	//-----------------------------------------------------------------------
	// Summary:
	//     Retrieves the control list of the given category.
	// Parameters:
	//     strCategory - Category to retrieve.
	// Returns:
	//     A pointer to a CXTPControls object.
	//-----------------------------------------------------------------------
	CXTPControls* GetControls(LPCTSTR strCategory);

protected:

	//-----------------------------------------------------------------------
	// Summary:
	//     Refills list of quick access controls
	//-----------------------------------------------------------------------
	void RefreshRibbonList();

// Implementation
protected:
	DECLARE_MESSAGE_MAP()

	//{{AFX_VIRTUAL(CXTPRibbonCustomizePage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CXTPRibbonCustomizePage)
	virtual BOOL OnInitDialog();
	afx_msg void OnCategoriesSelectionChanged();
	afx_msg void OnDblclkListCommands();
	afx_msg void OnButtonAdd();
	afx_msg void OnButtonRemove();
	afx_msg void OnButtonReset();
	afx_msg void OnCommandsSelectionChanged();
	afx_msg void OnRibbonSelectionChanged();
	afx_msg void OnRibbonTreeClick(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnRibbonTreeKeydown(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg LRESULT OnRibbonTreeCheckChanged(WPARAM, LPARAM lParam);
	afx_msg void OnRibbonSelChanged(NMHDR *pNMHDR, LRESULT *pResult);
	//}}AFX_MSG

private:
	XTP_COMMANDBARS_CATEGORYINFO* FindCategory(LPCTSTR strCategory) const;
	XTP_COMMANDBARS_CATEGORYINFO* GetCategoryInfo(int nIndex);

public:
	CComboBox   m_lstCategories;    // Categories list
	CTreeCtrl   m_treeRibbon;        // Ribbon list
	CXTPCustomizeCommandsListBox    m_lstCommands;      // Commands list

protected:
	CXTPCustomizeSheet* m_pSheet;           // Parent Sheet window
	CXTPCommandBars* m_pCommandBars;        // Parent CommandBars object
	CXTPCommandBarsCategoryArray m_arrCategories;   // Array of categories.

};

#endif // !defined(__XTPRIBBONCUSTOMIZEPAGE_H__)
