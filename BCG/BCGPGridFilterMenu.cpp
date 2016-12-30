//*******************************************************************************
// COPYRIGHT NOTES
// ---------------
// This is a sample for BCGControlBar Library Professional Edition
// Copyright (C) 1998-2014 BCGSoft Ltd.
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.
//*******************************************************************************
//
// BCGPGridFilterMenu.cpp: implementation of the CBCGPGridFilterMenuButton class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BCGPGridFilterMenu.h"
#include "BCGPGridCtrl.h"
#include "BCGPLocalResource.h"
#include "BCGProRes.h"

#define ID_DEFAULTFILTER_APPLY	102

/////////////////////////////////////////////////////////////////////////////
// CBCGPGridFilterListDlg dialog

CBCGPGridFilterListDlg::CBCGPGridFilterListDlg(UINT uiFilterCmd, 
											   const CStringList& lstValues,
											   CBCGPBaseFilterPopupMenu* pParent /*=NULL*/)
	: CBCGPDialog(CBCGPGridFilterListDlg::IDD, pParent),
	m_uiFilterCmd (uiFilterCmd)
{
	//{{AFX_DATA_INIT(CBCGPGridFilterListDlg)
	m_strSearch = _T("");
	//}}AFX_DATA_INIT

	EnableVisualManagerStyle();
	EnableLayout();
	SetWhiteBackground();

	m_lstValues.AddHead((CStringList*)&lstValues);

	m_bIsLocal = TRUE;
	m_bIsEmptyMenu = FALSE;
}
//*********************************************************************************************************
void CBCGPGridFilterListDlg::DoDataExchange(CDataExchange* pDX)
{
	CBCGPDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBCGPGridFilterListDlg)
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_BCGBARRES_FILTER_SEARCH, m_wndEdit);
	DDX_Control(pDX, IDC_BCGBARRES_FILTER_LIST, m_wndFilterList);
	DDX_Text(pDX, IDC_BCGBARRES_FILTER_SEARCH, m_strSearch);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CBCGPGridFilterListDlg, CBCGPDialog)
	//{{AFX_MSG_MAP(CBCGPGridFilterListDlg)
	ON_EN_CHANGE(IDC_BCGBARRES_FILTER_SEARCH, OnChangeFilterSearch)
	//}}AFX_MSG_MAP
	ON_CLBN_CHKCHANGE(IDC_BCGBARRES_FILTER_LIST, OnCheckchangeFilterList)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGPGridFilterListDlg message handlers

BOOL CBCGPGridFilterListDlg::OnInitDialog() 
{
	CBCGPDialog::OnInitDialog();

	CBCGPStaticLayout* pLayout = (CBCGPStaticLayout*)GetLayout();
	if (pLayout != NULL)
	{
		pLayout->AddAnchor(IDC_BCGBARRES_FILTER_SEARCH, CBCGPStaticLayout::e_MoveTypeNone, CBCGPStaticLayout::e_SizeTypeHorz);
		pLayout->AddAnchor(IDC_BCGBARRES_FILTER_LIST, CBCGPStaticLayout::e_MoveTypeNone, CBCGPStaticLayout::e_SizeTypeBoth);
		pLayout->AddAnchor(IDOK, CBCGPStaticLayout::e_MoveTypeBoth, CBCGPStaticLayout::e_SizeTypeNone);
		pLayout->AddAnchor(IDCANCEL, CBCGPStaticLayout::e_MoveTypeBoth, CBCGPStaticLayout::e_SizeTypeNone);
	}

	CString strPrompt;
	
	{
		CBCGPLocalResource locaRes;

		strPrompt.LoadString(IDS_BCGBARRES_SEARCH_PROMPT);
		m_strSelectAll.LoadString(IDS_BCGBARRES_SELECT_ALL);
	}

	m_wndEdit.EnableSearchMode(TRUE, strPrompt);

	FillList();

	m_wndEdit.SetFocus();
	return FALSE; // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
//*********************************************************************************************************
void CBCGPGridFilterListDlg::FillList()
{
	ASSERT_VALID (this);

	m_wndFilterList.ResetContent();

	CBCGPBaseFilterPopupMenu* pMenu = DYNAMIC_DOWNCAST(CBCGPBaseFilterPopupMenu, GetParent());
	if (pMenu == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	CString strAll = _T("(") + m_strSelectAll;
	if (!m_strSearch.IsEmpty())
	{
		strAll += _T(": \'");
		strAll += m_strSearch;
		strAll += _T("\'");
	}
	strAll += _T(")");

	BOOL bCheckAll = pMenu->IsAll() || !m_strSearch.IsEmpty();

	m_wndFilterList.AddString(strAll);
	m_wndFilterList.SetCheck(0, bCheckAll ? 1 : 0);

	BOOL bSomeChecked = FALSE;

	CString strSearchU = m_strSearch;
	strSearchU.TrimLeft();
	strSearchU.TrimRight();
	strSearchU.MakeUpper();

	for (POSITION pos = m_lstValues.GetHeadPosition(); pos != NULL; )
	{
		const CString& str = m_lstValues.GetNext(pos);

		BOOL bSkip = FALSE;
		
		if (!strSearchU.IsEmpty())
		{
			CString strU = str;
			strU.MakeUpper();
			
			if (strU.Find(strSearchU) < 0)
			{
				bSkip = TRUE;
			}
		}

		if (!bSkip)
		{
			int nIndex = m_wndFilterList.AddString(str);
			
			if (bCheckAll)
			{
				m_wndFilterList.SetCheck(nIndex, 1);
			}
			else if (pMenu->IsChecked(str))
			{
				m_wndFilterList.SetCheck(nIndex, 1);
				bSomeChecked = TRUE;
			}
		}
	}

	if (bSomeChecked)
	{
		m_wndFilterList.SetCheck(0, 2);
	}

	if (m_wndFilterList.GetCount() == 1)
	{
		m_wndFilterList.Enable(0, FALSE);
	}

	m_btnOK.EnableWindow(m_wndFilterList.GetCheckCount() > 1);
}
//*********************************************************************************************************
void CBCGPGridFilterListDlg::OnCheckchangeFilterList()
{
	int nCurSel = m_wndFilterList.GetCurSel();
	if (nCurSel == LB_ERR)
	{
		return;
	}

	UpdateData ();

	// "(Select All)" item has been changed
	if (nCurSel == 0)
	{
		int nNewState = m_wndFilterList.GetCheck(0);

		//-------------------
		// Apply to all items
		//-------------------
		int nCount = m_wndFilterList.GetCount ();
		for (int i = 0; i < nCount; i++)
		{
			m_wndFilterList.SetCheck(i, nNewState);
		}

		if (!m_strSearch.IsEmpty() && nNewState == 1)
		{
			m_wndFilterList.SetCheck(0, 2); // some items are filtered
		}
	}
	else
	{
		int nNewState = m_wndFilterList.GetCheck(nCurSel);
		if (nNewState != m_wndFilterList.GetCheck(0))
		{
			//---------------------------
			// Update "(Select All)" item
			//---------------------------
			m_wndFilterList.SetCheck(0, 2);
		}
	}

	m_btnOK.EnableWindow(m_wndFilterList.GetCheckCount() > 1);
}
//*********************************************************************************************************
void CBCGPGridFilterListDlg::OnCancel()
{
	GetParent()->PostMessage(WM_CLOSE);
}
//*********************************************************************************************************
void CBCGPGridFilterListDlg::OnOK() 
{
	UpdateData ();

	CBCGPBaseFilterPopupMenu* pMenu = DYNAMIC_DOWNCAST(CBCGPBaseFilterPopupMenu, GetParent());
	if (pMenu == NULL)
	{
		ASSERT(FALSE);
	}
	else
	{
		//--------------
		// Update params
		//--------------
		pMenu->SetAll(m_wndFilterList.GetCheck (0) == 1 && m_strSearch.IsEmpty());

		if (!pMenu->IsAll())
		{
			// Add only checked items
			int nCount = m_wndFilterList.GetCount ();
			BOOL bAllChecked = TRUE;

			for (int i = 1; i < nCount; i++)
			{
				CString strNewText;
				m_wndFilterList.GetText (i, strNewText);

				if (m_wndFilterList.GetCheck (i) == 1)
				{
					pMenu->AddChecked(strNewText);
				}
				else
				{
					bAllChecked = FALSE;
				}
			}

			if (bAllChecked && m_strSearch.IsEmpty())
			{
				// Clear filter:
				pMenu->SetAll(TRUE);
			}
		}
	}

	GetParent()->PostMessage(WM_CLOSE);

	//----------------------------
	// Send "Apply Filter" command
	//----------------------------
	if (pMenu != NULL)
	{
		pMenu->SendFilterCommand (GetOwner (), m_uiFilterCmd);
	}
}
//*********************************************************************************************************
void CBCGPGridFilterListDlg::OnChangeFilterSearch() 
{
	UpdateData();
	FillList();
}
//*********************************************************************************************************
BOOL CBCGPGridFilterListDlg::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message == WM_MOUSEWHEEL)
	{
		CPoint ptCursor;
		GetCursorPos(&ptCursor);

		CRect rectList;
		m_wndFilterList.GetWindowRect(rectList);

		if (rectList.PtInRect(ptCursor))
		{
			m_wndFilterList.SendMessage(WM_MOUSEWHEEL, pMsg->wParam, pMsg->lParam);
			return TRUE;
		}
	}

	if (pMsg->message == WM_KEYDOWN && !m_bIsEmptyMenu)
	{
		BOOL bIsShiftPressed = (0x8000 & GetKeyState(VK_SHIFT)) != 0;

		if ((pMsg->wParam == VK_UP || (pMsg->wParam == VK_TAB && bIsShiftPressed)) && GetFocus()->GetSafeHwnd() == m_wndEdit.GetSafeHwnd())
		{
			GetParent()->SetFocus();
			GetParent()->PostMessage(WM_KEYDOWN, VK_END);
			return TRUE;
		}
		else if ((pMsg->wParam == VK_DOWN || pMsg->wParam == VK_TAB) && GetFocus()->GetSafeHwnd() == m_btnCancel.GetSafeHwnd())
		{
			GetParent()->SetFocus();
			GetParent()->PostMessage(WM_KEYDOWN, VK_HOME);
			return TRUE;
		}
	}
	
	return CBCGPDialog::PreTranslateMessage(pMsg);
}

//////////////////////////////////////////////////////////////////////////
// CBCGPBaseFilterPopupMenu

IMPLEMENT_DYNAMIC(CBCGPBaseFilterPopupMenu, CBCGPPopupMenu)

CBCGPBaseFilterPopupMenu::CBCGPBaseFilterPopupMenu(UINT uiFilterCmd,
												   const CStringList& lstCheckItems)
	: m_wndList(uiFilterCmd, lstCheckItems)
{
#ifdef _BCGSUITE_
	EnableMenuLogo(0, MENU_LOGO_BOTTOM);
#else
	EnableMenuLogo(0, MENU_LOGO_BOTTOM, TRUE);
#endif
}
//*********************************************************************************************************
CBCGPBaseFilterPopupMenu::~CBCGPBaseFilterPopupMenu()
{
}

BEGIN_MESSAGE_MAP(CBCGPBaseFilterPopupMenu, CBCGPPopupMenu)
	//{{AFX_MSG_MAP(CBCGPBaseFilterPopupMenu)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_WINDOWPOSCHANGING()
	ON_WM_KEYDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

int CBCGPBaseFilterPopupMenu::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	const int iResizeBarBarHeight = 9;

	if (CBCGPPopupMenu::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!m_wndList.Create(CBCGPGridFilterListDlg::IDD, this))
	{
		ASSERT (FALSE);
		return -1;
	}

	m_wndList.ShowWindow(SW_SHOWNOACTIVATE);

	m_wndList.SetFont(&globalData.fontRegular);
	m_wndList.SetOwner(GetTopLevelFrame ());

	CRect rectList;
	m_wndList.GetWindowRect(rectList);

#ifndef _BCGSUITE_
	const int nGutterWidth = m_wndMenuBar.GetGutterWidth();
#else
	const int nGutterWidth = 0;
#endif

	m_wndMenuBar.m_nListWidth = rectList.Width() + nGutterWidth;

	m_iLogoWidth = rectList.Height();

	CSize sizeMenu = m_FinalSize;
	
	sizeMenu.cx = max(rectList.Width() + nGutterWidth, sizeMenu.cx);
	sizeMenu.cy += m_iLogoWidth - iResizeBarBarHeight;

	EnableResize(sizeMenu);
	return 0;
}
//*********************************************************************************************************
void CBCGPBaseFilterPopupMenu::RecalcLayout(BOOL bNotify /* = TRUE */)
{
	CBCGPPopupMenu::RecalcLayout(bNotify);

	if (m_wndMenuBar.GetSafeHwnd() != NULL)
	{
		m_wndMenuBar.m_arColumns.RemoveAll();
		m_wndMenuBar.AdjustLayout();
	}

	if (m_wndList.GetSafeHwnd() == NULL)
	{
		return;
	}

	const int nShadowSize = CBCGPToolBar::IsCustomizeMode () ? 0 : m_iShadowSize;
	const int nBorderSize = GetBorderSize();

	CRect rectClient;
	GetClientRect(rectClient);

	rectClient.DeflateRect (nBorderSize, nBorderSize);

	if (GetExStyle() & WS_EX_LAYOUTRTL)
	{
		rectClient.left += nShadowSize;
	}
	else
	{
		rectClient.right -= nShadowSize;
	}
	
	rectClient.top += m_nMenuBarHeight;
	rectClient.bottom -= nShadowSize;

#ifndef _BCGSUITE_
	rectClient.left += m_wndMenuBar.GetGutterWidth();
#endif

	if (!m_rectResize.IsRectEmpty())
	{
		if (m_bIsResizeBarOnTop)
		{
			rectClient.top += m_rectResize.Height();
		}
		else
		{
			rectClient.bottom -= m_rectResize.Height();
		}
	}

	m_wndList.SetWindowPos(NULL, rectClient.left, rectClient.top, rectClient.Width(), rectClient.Height(), SWP_NOZORDER | SWP_NOACTIVATE);
	m_wndList.m_bIsEmptyMenu = GetMenuItemCount() == 0;
}
//*****************************************************************************************
void CBCGPBaseFilterPopupMenu::OnDestroy()
{
	if (m_wndList.GetSafeHwnd () != NULL)
	{
		m_wndList.DestroyWindow();
	}

	CBCGPPopupMenu::OnDestroy();
}
//*****************************************************************************************
void CBCGPBaseFilterPopupMenu::OnWindowPosChanging(WINDOWPOS FAR* lpwndpos)
{
	if (m_bResizeTracking)
	{
		CRect rectWnd;
		GetWindowRect(&rectWnd);

		m_iLogoWidth += lpwndpos->cy - rectWnd.Height();
		m_wndMenuBar.m_nListWidth += lpwndpos->cx - rectWnd.Width();
	}

	CBCGPPopupMenu::OnWindowPosChanging(lpwndpos);
}
//*****************************************************************************************
void CBCGPBaseFilterPopupMenu::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if (nChar == VK_TAB)
	{
		BOOL bIsShiftPressed = (0x8000 & GetKeyState(VK_SHIFT)) != 0;

		if (!bIsShiftPressed)
		{
			m_wndList.SetFocus();
			
			m_wndMenuBar.m_iHighlighted = -1;
			m_wndMenuBar.m_iSelected = -1;

			m_wndMenuBar.RedrawWindow();

			return;
		}
	}
	
	CBCGPPopupMenu::OnKeyDown(nChar, nRepCnt, nFlags);
}
//*****************************************************************************************
void CBCGPBaseFilterPopupMenu::SendFilterCommand (CWnd* pDestWnd, UINT uiFilterCmd)
{
	if (pDestWnd->GetSafeHwnd() != NULL)
	{
		pDestWnd->PostMessage (WM_COMMAND, uiFilterCmd);
	}
}

//////////////////////////////////////////////////////////////////////////
// CBCGPGridFilterPopupMenu

IMPLEMENT_DYNAMIC(CBCGPGridFilterPopupMenu, CBCGPBaseFilterPopupMenu)

CBCGPGridFilterPopupMenu::CBCGPGridFilterPopupMenu(UINT uiFilterCmd,
												   const CStringList& lstCheckItems,
												   BCGP_FILTER_COLUMN_INFO& filterColumnInfo,
												   CBCGPGridCtrl* pOwnerGrid)
	: CBCGPBaseFilterPopupMenu(uiFilterCmd, lstCheckItems),
	m_filterColumnInfo(filterColumnInfo), m_pOwnerGrid (pOwnerGrid)
{
}
//*********************************************************************************************************
CBCGPGridFilterPopupMenu::~CBCGPGridFilterPopupMenu()
{
}

BEGIN_MESSAGE_MAP(CBCGPGridFilterPopupMenu, CBCGPBaseFilterPopupMenu)
	//{{AFX_MSG_MAP(CBCGPGridFilterPopupMenu)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CBCGPGridFilterPopupMenu::IsAll() const
{
	return m_filterColumnInfo.bAll;	
}
//*****************************************************************************************
BOOL CBCGPGridFilterPopupMenu::IsChecked(LPCTSTR lpszItem) const
{
	return m_filterColumnInfo.lstValues.Find(lpszItem) != NULL;	
}
//*****************************************************************************************
void CBCGPGridFilterPopupMenu::SetAll(BOOL bSet)
{
	m_filterColumnInfo.bAll = bSet;
	m_filterColumnInfo.lstValues.RemoveAll ();
}
//*****************************************************************************************
void CBCGPGridFilterPopupMenu::AddChecked(LPCTSTR lpszItem)
{
	m_filterColumnInfo.lstValues.AddTail(lpszItem);
}
//*****************************************************************************************
void CBCGPGridFilterPopupMenu::SendFilterCommand (CWnd* pDestWnd, UINT uiFilterCmd)
{
	if (uiFilterCmd == ID_DEFAULTFILTER_APPLY)
	{
		if (m_pOwnerGrid->GetSafeHwnd() != NULL)
		{
			m_pOwnerGrid->PostMessage (WM_COMMAND, uiFilterCmd);
			return;
		}
	}

	CBCGPBaseFilterPopupMenu::SendFilterCommand (pDestWnd, uiFilterCmd);
}
