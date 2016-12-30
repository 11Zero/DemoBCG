// BCGCalendarMenuButton.cpp: implementation of the CBCGPCalendarMenuButton class.
// This is a part of the BCGControlBar Library
// Copyright (C) 1998-2014 BCGSoft Ltd.
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "BCGCBPro.h"

#ifndef _BCGSUITE_
#include "BCGPTearOffManager.h"
#include "BCGPPopupMenuBar.h"
#include "BCGPRegistry.h"
#include "BCGPVisualManager.h"
	#define visualManager			CBCGPVisualManager
	#define visualManagerInstance	CBCGPVisualManager::GetInstance ()
#else
	#define visualManager			CMFCVisualManager
	#define visualManagerInstance	CMFCVisualManager::GetInstance ()
#endif

#include "MenuImages.h"
#include "CalendarPopup.h"
#include "BCGGlobals.h"
#include "BCGPCalendarMenuButton.h"
#include "BCGPCalendarBar.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

static const int SEPARATOR_SIZE = 2;

CMap<UINT,UINT,COleDateTime, COleDateTime> CBCGPCalendarMenuButton::m_CalendarsByID;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_SERIAL(CBCGPCalendarMenuButton, CBCGPToolbarMenuButton, VERSIONABLE_SCHEMA | 1)

CBCGPCalendarMenuButton::CBCGPCalendarMenuButton()
{
	Initialize ();
}
//*****************************************************************************************
CBCGPCalendarMenuButton::CBCGPCalendarMenuButton (UINT uiCmdID, LPCTSTR lpszText) :
	CBCGPToolbarMenuButton (uiCmdID, NULL,
#ifndef _BCGSUITE_
		CImageHash::GetImageOfCommand (uiCmdID, FALSE), 
#else
		GetCmdMgr ()->GetCmdImage (uiCmdID, FALSE),
#endif
		lpszText)
{
	Initialize ();

	m_Calendar = GetCalendarByCmdID (uiCmdID);
}
//*****************************************************************************************
void CBCGPCalendarMenuButton::Initialize ()
{
    m_Calendar = COleDateTime::GetCurrentTime();	
	m_nColumns = -1;
	m_nVertDockColumns = -1;
	m_nHorzDockRows = -1;
	m_bStdCalendarDlg = FALSE;
}
//*****************************************************************************************
CBCGPCalendarMenuButton::~CBCGPCalendarMenuButton()
{
}

//*****************************************************************************************
void CBCGPCalendarMenuButton::EnableTearOff (UINT uiID,
										 int nVertDockColumns,
										 int nHorzDockRows)
{
#ifndef _BCGSUITE_
	if (g_pBCGPTearOffMenuManager != NULL &&
		g_pBCGPTearOffMenuManager->IsDynamicID (uiID))
	{
		ASSERT (FALSE);	// SHould be static ID!
		uiID = 0;
	}

	m_uiTearOffBarID = uiID;

	m_nVertDockColumns = nVertDockColumns;
	m_nHorzDockRows = nHorzDockRows;
#else
	UNREFERENCED_PARAMETER(uiID);
	UNREFERENCED_PARAMETER(nVertDockColumns);
	UNREFERENCED_PARAMETER(nHorzDockRows);
#endif
}
//*****************************************************************************************
void CBCGPCalendarMenuButton::OnDraw (CDC* pDC, const CRect& rect, CBCGPToolBarImages* pImages,
			BOOL bHorz, BOOL bCustomizeMode, BOOL bHighlight,
			BOOL bDrawBorder, BOOL bGrayDisabledButtons)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);



	if (m_bMenuMode)
	{
		DrawMenuItem (pDC, rect, pImages, bCustomizeMode, bHighlight, bGrayDisabledButtons);
		return;
	}

	//----------------------
	// Fill button interior:
	//----------------------
	FillInterior (pDC, rect, bHighlight || IsDroppedDown ());

	CSize sizeImage = CBCGPMenuImages::Size ();
	if (CBCGPToolBar::IsLargeIcons ())
	{
		sizeImage.cx *= 2;
		sizeImage.cy *= 2;
	}

	CRect rectInternal = rect;
	CSize sizeExtra = m_bExtraSize ? 
		visualManagerInstance->GetButtonExtraBorder () : CSize (0, 0);

	if (sizeExtra != CSize (0, 0))
	{
		rectInternal.DeflateRect (sizeExtra.cx / 2 + 1, sizeExtra.cy / 2 + 1);
	}

	CRect rectParent = rect;
	CRect rectArrow = rectInternal;

	const int nMargin = visualManagerInstance->GetMenuImageMargin ();
	const int xMargin = bHorz ? nMargin : 0;
	const int yMargin = bHorz ? 0 : nMargin;

	rectInternal.DeflateRect (xMargin, yMargin);
	rectParent.DeflateRect (xMargin, yMargin);


	if (m_bDrawDownArrow)
	{
		if (bHorz)
		{
			rectParent.right -= sizeImage.cx + SEPARATOR_SIZE;
			rectArrow.left = rectParent.right - 1;
			rectArrow.OffsetRect (-sizeExtra.cx / 2, -sizeExtra.cy / 2);
		}
		else
		{
			rectParent.bottom -= sizeImage.cy + SEPARATOR_SIZE;
			rectArrow.top = rectParent.bottom - 1;
		}
	}

	UINT uiStyle = m_nStyle;

	if (visualManagerInstance->IsMenuFlatLook ())
	{
		m_nStyle &= ~(TBBS_PRESSED | TBBS_CHECKED);
	}
	else
	{
		if (m_bClickedOnMenu && m_nID != 0 && m_nID != (UINT) -1 && !m_bMenuOnly) 
		{
			m_nStyle &= ~TBBS_PRESSED;
		}
		else if (m_pPopupMenu != NULL)
		{
			m_nStyle |= TBBS_PRESSED;
		}
	}

	BOOL bDisableFill = m_bDisableFill;
	m_bDisableFill = TRUE;

	CBCGPToolbarButton::OnDraw (pDC, rectParent, pImages, bHorz, 
			bCustomizeMode, bHighlight, bDrawBorder, bGrayDisabledButtons);

	m_bDisableFill = bDisableFill;

	if (m_bDrawDownArrow)
	{
		if ((m_nStyle & (TBBS_PRESSED | TBBS_CHECKED)) &&
			!visualManagerInstance->IsMenuFlatLook ())
		{
			rectArrow.OffsetRect (1, 1);
		}

		BOOL bDisabled = (bCustomizeMode && !IsEditable ()) ||
			(!bCustomizeMode && (m_nStyle & TBBS_DISABLED));

		int iImage;
		if (bHorz && !m_bMenuOnly)
		{
			iImage = CBCGPMenuImages::IdArowDown;
		}
		else
		{
			iImage = CBCGPMenuImages::IdArowRight;
		}

		if (m_pPopupMenu != NULL &&
			(m_nStyle & (TBBS_PRESSED | TBBS_CHECKED)) == 0 &&
			!visualManagerInstance->IsMenuFlatLook ())
		{
			rectArrow.OffsetRect (1, 1);
		}

		CBCGPMenuImages::Draw (pDC, (CBCGPMenuImages::IMAGES_IDS) iImage, rectArrow,
							bDisabled ? CBCGPMenuImages::ImageGray : CBCGPMenuImages::ImageBlack,
							sizeImage);
	}

	m_nStyle = uiStyle;

	if (!bCustomizeMode)
	{
		if ((m_nStyle & (TBBS_PRESSED | TBBS_CHECKED)) ||
			m_pPopupMenu != NULL)
		{
			//-----------------------
			// Pressed in or checked:
			//-----------------------
			if (!visualManagerInstance->IsMenuFlatLook () &&
				m_bClickedOnMenu && m_nID != 0 && m_nID != (UINT) -1 && !m_bMenuOnly)
			{
				visualManagerInstance->OnDrawButtonBorder (pDC,
					this, rectParent, visualManager::ButtonsIsHighlighted);

				rectArrow.right --;
				rectArrow.bottom --;

				visualManagerInstance->OnDrawButtonBorder (pDC,
					this, rectArrow, visualManager::ButtonsIsPressed);
			}
			else
			{
				visualManagerInstance->OnDrawButtonBorder (pDC,
					this, rect, visualManager::ButtonsIsPressed);
			}
		}
		else if (bHighlight && !(m_nStyle & TBBS_DISABLED) &&
			!(m_nStyle & (TBBS_CHECKED | TBBS_INDETERMINATE)))
		{
			visualManagerInstance->OnDrawButtonBorder (pDC,
				this, rect, visualManager::ButtonsIsHighlighted);
		}
	}

}
//**********************************************************************************************
int CBCGPCalendarMenuButton::OnDrawOnCustomizeList (CDC* pDC, const CRect& rect, BOOL bSelected)
{
	int nID = m_nID;
	m_nID = 0;	// Force draw right arrow

	CRect rectCalendar = rect;
	rectCalendar.DeflateRect (1, 0);

	int iRes = CBCGPToolbarMenuButton::OnDrawOnCustomizeList (pDC, rect, bSelected);

	m_nID = nID;
	
	return iRes;
}
//**********************************************************************************************
void CBCGPCalendarMenuButton::SetDate (const COleDateTime& date, BOOL bNotify)
{
	m_Calendar = date;
	m_CalendarsByID.SetAt (m_nID, m_Calendar);

	if (m_pWndParent->GetSafeHwnd () != NULL)
	{
		m_pWndParent->InvalidateRect (m_rect);
	}

	if (bNotify)
	{
		CObList listButtons;
		if (CBCGPToolBar::GetCommandButtons (m_nID, listButtons) > 0)
		{
			for (POSITION pos = listButtons.GetHeadPosition (); pos != NULL;)
			{
				CBCGPCalendarMenuButton* pOther = 
					DYNAMIC_DOWNCAST (CBCGPCalendarMenuButton, listButtons.GetNext (pos));

				if (pOther != NULL && pOther != this)
				{
					pOther->SetDate (date, FALSE);
				}
			}
		}

		const CObList& lstToolBars = CBCGPToolBar::GetAllToolbars ();
		for (POSITION pos = lstToolBars.GetHeadPosition (); pos != NULL;)
		{
			CBCGPCalendarBar* pCalendarBar = DYNAMIC_DOWNCAST (CBCGPCalendarBar, lstToolBars.GetNext (pos));
			if (pCalendarBar != NULL && pCalendarBar->m_nCommandID == m_nID)
			{
				pCalendarBar->SetDate (date);
			}
		}
	}
}
//****************************************************************************************
void CBCGPCalendarMenuButton::OnChangeParentWnd (CWnd* pWndParent)
{
	CBCGPToolbarButton::OnChangeParentWnd (pWndParent);

	if (pWndParent != NULL)
	{
		if (pWndParent->IsKindOf (RUNTIME_CLASS (CBCGPMenuBar)))
		{
			m_bText = TRUE;
		}

		if (pWndParent->IsKindOf (RUNTIME_CLASS (CBCGPPopupMenuBar)))
		{
			m_bMenuMode = TRUE;
			m_bText = TRUE;
		}
		else
		{
			m_bMenuMode = FALSE;
		}
	}

	m_bDrawDownArrow = TRUE;
	m_pWndParent = pWndParent;
}
//************************************************************************************
void CBCGPCalendarMenuButton::Serialize (CArchive& ar)
{
	CBCGPToolbarMenuButton::Serialize (ar);

	if (ar.IsLoading ())
	{
		ar >> m_nColumns;
		ar >> m_nVertDockColumns;
		ar >> m_nHorzDockRows;

		ar >> m_bStdCalendarDlg;

		CObList listButtons;
		if (CBCGPToolBar::GetCommandButtons (m_nID, listButtons) > 0)
		{
			for (POSITION pos = listButtons.GetHeadPosition (); pos != NULL;)
			{
				CBCGPCalendarMenuButton* pOther = 
					DYNAMIC_DOWNCAST (CBCGPCalendarMenuButton, listButtons.GetNext (pos));
				if (pOther != NULL && pOther != this)
				{
					m_Calendar = pOther->m_Calendar;
				}
			}
		}
	}
	else
	{
		ar << m_nColumns;

		ar << m_nVertDockColumns;
		ar << m_nHorzDockRows;

		ar << m_bStdCalendarDlg;
	}
}
//************************************************************************************
void CBCGPCalendarMenuButton::CopyFrom (const CBCGPToolbarButton& s)
{
	CBCGPToolbarMenuButton::CopyFrom (s);

	const CBCGPCalendarMenuButton& src = (const CBCGPCalendarMenuButton&) s;

	m_Calendar = src.m_Calendar;
	m_CalendarsByID.SetAt (m_nID, m_Calendar);	// Just to be happy :-)
	m_nColumns =				src.m_nColumns;
	m_nVertDockColumns =		src.m_nVertDockColumns;
	m_nHorzDockRows =			src.m_nHorzDockRows;

	m_bStdCalendarDlg =			src.m_bStdCalendarDlg;
}


//*****************************************************************************************
BOOL CBCGPCalendarMenuButton::OnClick (CWnd* pWnd, BOOL bDelay)
{
	ASSERT_VALID (pWnd);
	
	m_bClickedOnMenu = FALSE;

	CBCGPMenuBar* pMenuBar = DYNAMIC_DOWNCAST (CBCGPMenuBar, m_pWndParent);

	if (m_pPopupMenu != NULL)
	{
		//-----------------------------------------------------
		// Second click to the popup menu item closes the menu:
		//-----------------------------------------------------		
		ASSERT_VALID(m_pPopupMenu);

		m_pPopupMenu->m_bAutoDestroyParent = FALSE;
		m_pPopupMenu->DestroyWindow ();
		m_pPopupMenu = NULL;

		if (pMenuBar != NULL)
		{
			pMenuBar->SetHot (NULL);
		}
	}
	else
	{
		CBCGPPopupMenuBar* pParentMenu =
			DYNAMIC_DOWNCAST (CBCGPPopupMenuBar, m_pWndParent);

		if (bDelay && pParentMenu != NULL && !CBCGPToolBar::IsCustomizeMode ())
		{
			pParentMenu->StartPopupMenuTimer (this);
		}
		else
		{
			if (pMenuBar != NULL)
			{
				CBCGPToolbarMenuButton* pCurrPopupMenuButton = 
					pMenuBar->GetDroppedDownMenu();
				if (pCurrPopupMenuButton != NULL)
				{
					pCurrPopupMenuButton->OnCancelMode ();
				}
			}
			
			if (!OpenPopupMenu (pWnd))
			{
				return FALSE;
			}
		}

		if (pMenuBar != NULL)
		{
			pMenuBar->SetHot (this);
		}
	}

	if (m_pWndParent != NULL)
	{
		CRect rect = m_rect;
		rect.InflateRect (4, 4);
		m_pWndParent->InvalidateRect (rect);
	}

	return TRUE;
}

//**************************************************************************************
CBCGPPopupMenu* CBCGPCalendarMenuButton::CreatePopupMenu ()
{
	return new CCalendarPopup (m_Calendar, m_nID);
}
//*************************************************************************************
const COleDateTime CBCGPCalendarMenuButton::GetCalendarByCmdID (UINT uiCmdID)
{
	COleDateTime date;
    m_CalendarsByID.Lookup (uiCmdID, date);

	return date ;
}
