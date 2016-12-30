// BCGColorMenuButton.cpp: implementation of the CBCGPColorMenuButton class.
// This is a part of the BCGControlBar Library
// Copyright (C) 1998-2014 BCGSoft Ltd.
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "BCGPTearOffManager.h"
#include "BCGCBPro.h"
#include "MenuImages.h"
#include "BCGPPopupMenuBar.h"
#include "BCGPColorMenuButton.h"
#include "BCGPColorDialog.h"
#include "BCGPColorBar.h"
#include "BCGPWorkspace.h"
#include "BCGPRegistry.h"
#include "ColorPopup.h"
#include "BCGGlobals.h"
#include "BCGPVisualManager.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

extern CBCGPWorkspace* g_pWorkspace;

static const int SEPARATOR_SIZE = 2;

CMap<UINT,UINT,COLORREF, COLORREF> CBCGPColorMenuButton::m_ColorsByID;

UINT BCGM_GETDOCUMENTCOLORS	= ::RegisterWindowMessage (_T("BCGTOOLBAR__GETDOCUMENTCOLORS"));

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_SERIAL(CBCGPColorMenuButton, CBCGPToolbarMenuButton, VERSIONABLE_SCHEMA | 1)

CBCGPColorMenuButton::CBCGPColorMenuButton()
{
	Initialize ();
}
//*****************************************************************************************
CBCGPColorMenuButton::CBCGPColorMenuButton (UINT uiCmdID, LPCTSTR lpszText, CPalette* pPalette) :
	CBCGPToolbarMenuButton (uiCmdID, NULL,
		CImageHash::GetImageOfCommand (uiCmdID, FALSE), lpszText)
{
	Initialize ();

	CBCGPColorBar::InitColors (pPalette, m_Colors);
	m_Color = GetColorByCmdID (uiCmdID);
}
//*****************************************************************************************
void CBCGPColorMenuButton::Initialize ()
{
	m_Color = (COLORREF) -1;	// Default (automatic) color
	m_colorAutomatic = 0;
	m_nColumns = -1;
	m_nVertDockColumns = -1;
	m_nHorzDockRows = -1;
	m_bIsAutomaticButton = FALSE;
	m_bIsOtherButton = FALSE;
	m_bIsDocumentColors = FALSE;
	m_bStdColorDlg = FALSE;
}
//*****************************************************************************************
CBCGPColorMenuButton::~CBCGPColorMenuButton()
{
}
//*****************************************************************************************
void CBCGPColorMenuButton::SetColors(const CArray<COLORREF, COLORREF>& colors)
{
	if (m_Colors.GetSize () != 0)
	{
		m_Colors.SetSize (0);
		m_Colors.FreeExtra ();
	}

	m_Colors.Append(colors);
}
//*****************************************************************************************
void CBCGPColorMenuButton::EnableAutomaticButton (LPCTSTR lpszLabel, COLORREF colorAutomatic, BOOL bEnable)
{
	m_bIsAutomaticButton = bEnable;
	if (bEnable)
	{
		ASSERT (lpszLabel != NULL);
		m_strAutomaticButtonLabel = lpszLabel;

		m_colorAutomatic = colorAutomatic;
	}
}
//*****************************************************************************************
void CBCGPColorMenuButton::EnableOtherButton (LPCTSTR lpszLabel, BOOL bAltColorDlg, BOOL bEnable)
{
	m_bIsOtherButton = bEnable;

	if (bEnable)
	{
		ASSERT (lpszLabel != NULL);
		m_strOtherButtonLabel = lpszLabel;

		m_bStdColorDlg = !bAltColorDlg;
	}
}
//*****************************************************************************************
void CBCGPColorMenuButton::EnableDocumentColors (LPCTSTR lpszLabel, BOOL bEnable)
{
	m_bIsDocumentColors = bEnable;
	if (bEnable)
	{
		ASSERT (lpszLabel != NULL);
		m_strDocumentColorsLabel = lpszLabel;
	}
}
//*****************************************************************************************
void CBCGPColorMenuButton::EnableTearOff (UINT uiID,
										 int nVertDockColumns,
										 int nHorzDockRows)
{
	if (g_pBCGPTearOffMenuManager != NULL &&
		g_pBCGPTearOffMenuManager->IsDynamicID (uiID))
	{
		ASSERT (FALSE);	// SHould be static ID!
		uiID = 0;
	}

	m_uiTearOffBarID = uiID;

	m_nVertDockColumns = nVertDockColumns;
	m_nHorzDockRows = nHorzDockRows;
}
//*****************************************************************************************
void CBCGPColorMenuButton::OnDraw (CDC* pDC, const CRect& rect, CBCGPToolBarImages* pImages,
			BOOL bHorz, BOOL bCustomizeMode, BOOL bHighlight,
			BOOL bDrawBorder, BOOL bGrayDisabledButtons)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);

	CBCGPToolbarMenuButton::OnDraw (pDC, rect, pImages, bHorz, bCustomizeMode,
		bHighlight, bDrawBorder, bGrayDisabledButtons);

	if (!IsDrawImage () || pImages == NULL)
	{
		return;
	}

    CPalette* pOldPalette = NULL;
	if (globalData.m_nBitsPerPixel == 8) // 256 colors
	{
		if (m_Palette.GetSafeHandle () == NULL)
		{
			//----------------------------------------
			// Palette not created yet; create it now
			//----------------------------------------
			CBCGPColorBar::CreatePalette (m_Colors, m_Palette);
		}

		ASSERT (m_Palette.GetSafeHandle () != NULL);

		pOldPalette = pDC->SelectPalette (&m_Palette, FALSE);
		pDC->RealizePalette ();
	}
	else if (m_Palette.GetSafeHandle () != NULL)
	{
		::DeleteObject (m_Palette.Detach ());
		ASSERT (m_Palette.GetSafeHandle () == NULL);
	}

	ASSERT (pImages != NULL);
	CRect rectColor = pImages->GetLastImageRect ();
	const int nColorBoxSize = CBCGPToolBar::IsLargeIcons () && !m_bMenuMode ? 10 : 5;

	rectColor.top = rectColor.bottom - nColorBoxSize;
	rectColor.OffsetRect (0, 1);

	//----------------
	// Draw color bar:
	//----------------
	BOOL bDrawImageShadow = 
		bHighlight && !bCustomizeMode &&
		CBCGPVisualManager::GetInstance ()->IsShadowHighlightedImage () &&
		!globalData.IsHighContastMode () &&
		((m_nStyle & TBBS_PRESSED) == 0) &&
		((m_nStyle & TBBS_CHECKED) == 0) &&
		((m_nStyle & TBBS_DISABLED) == 0);

	if (bDrawImageShadow)
	{
		CBrush brShadow (globalData.clrBarShadow);
		pDC->FillRect (rectColor, &brShadow);
		rectColor.OffsetRect (-1, -1);
	}

	COLORREF color = (m_nStyle & TBBS_DISABLED) ?
		globalData.clrBarShadow :
			(m_Color == (COLORREF)-1 ? m_colorAutomatic : m_Color);

	CBrush br (PALETTERGB(	GetRValue (color),
							GetGValue (color), 
							GetBValue (color)));

	CBrush* pOldBrush = pDC->SelectObject (&br);
	CPen* pOldPen = (CPen*) pDC->SelectStockObject (NULL_PEN);
	
	pDC->Rectangle (&rectColor);

	pDC->SelectObject (pOldPen);
	pDC->SelectObject (pOldBrush);

	if (CBCGPVisualManager::GetInstance ()->IsMenuFlatLook ())
	{
		if (color == globalData.clrBarFace)
		{
			pDC->Draw3dRect (rectColor, globalData.clrBarDkShadow, globalData.clrBarDkShadow);
		}
	}
	else
	{
		pDC->Draw3dRect (rectColor, globalData.clrBarShadow, globalData.clrBarLight);
	}
	
    if (pOldPalette != NULL)
	{
        pDC->SelectPalette (pOldPalette, FALSE);
	}
}
//**********************************************************************************************
int CBCGPColorMenuButton::OnDrawOnCustomizeList (CDC* pDC, const CRect& rect, BOOL bSelected)
{
	int nID = m_nID;
	m_nID = 0;	// Force draw right arrow

	CRect rectColor = rect;
	rectColor.DeflateRect (1, 0);

	int iRes = CBCGPToolbarMenuButton::OnDrawOnCustomizeList (pDC, rect, bSelected);

	m_nID = nID;
	
	return iRes;
}
//**********************************************************************************************
void CBCGPColorMenuButton::SetColor (COLORREF clr, BOOL bNotify)
{
	m_Color = clr;
	m_ColorsByID.SetAt (m_nID, m_Color);

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
				CBCGPColorMenuButton* pOther = 
					DYNAMIC_DOWNCAST (CBCGPColorMenuButton, listButtons.GetNext (pos));

				if (pOther != NULL && pOther != this)
				{
					pOther->SetColor (clr, FALSE);
				}
			}
		}

		const CObList& lstToolBars = CBCGPToolBar::GetAllToolbars ();
		for (POSITION pos = lstToolBars.GetHeadPosition (); pos != NULL;)
		{
			CBCGPColorBar* pColorBar = DYNAMIC_DOWNCAST (CBCGPColorBar, lstToolBars.GetNext (pos));
			if (pColorBar != NULL && pColorBar->m_nCommandID == m_nID)
			{
				pColorBar->SetColor (clr);
			}
		}
	}
}
//****************************************************************************************
void CBCGPColorMenuButton::OnChangeParentWnd (CWnd* pWndParent)
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
void CBCGPColorMenuButton::Serialize (CArchive& ar)
{
	CBCGPToolbarMenuButton::Serialize (ar);

	if (ar.IsLoading ())
	{
		int nColorsCount;
		ar >> nColorsCount;

		m_Colors.SetSize (nColorsCount);

		for (int i = 0; i < nColorsCount; i++)
		{
			COLORREF color;
			ar >> color;

			m_Colors [i] = color;
		}

		ar >> m_nColumns;
		ar >> m_nVertDockColumns;
		ar >> m_nHorzDockRows;

		ar >> m_bIsAutomaticButton;    
		ar >> m_bIsOtherButton;        
		ar >> m_bIsDocumentColors;     
							
		ar >> m_strAutomaticButtonLabel;
		ar >> m_strOtherButtonLabel;   
		ar >> m_strDocumentColorsLabel;

		ar >> m_colorAutomatic;
		ar >> m_bStdColorDlg;

		// Synchromize color with another buttons with the same ID:
		CObList listButtons;
		if (CBCGPToolBar::GetCommandButtons (m_nID, listButtons) > 0)
		{
			for (POSITION pos = listButtons.GetHeadPosition (); pos != NULL;)
			{
				CBCGPColorMenuButton* pOther = 
					DYNAMIC_DOWNCAST (CBCGPColorMenuButton, listButtons.GetNext (pos));
				if (pOther != NULL && pOther != this &&
					pOther->m_Color != (COLORREF) -1)
				{
					m_Color = pOther->m_Color;
				}
			}
		}
	}
	else
	{
		ar << (int) m_Colors.GetSize ();
		for (int i = 0; i < m_Colors.GetSize (); i++)
		{
			ar << m_Colors [i];
		}

		ar << m_nColumns;

		ar << m_nVertDockColumns;
		ar << m_nHorzDockRows;

		ar << m_bIsAutomaticButton;    
		ar << m_bIsOtherButton;        
		ar << m_bIsDocumentColors;     
							
		ar << m_strAutomaticButtonLabel;
		ar << m_strOtherButtonLabel;   
		ar << m_strDocumentColorsLabel;

		ar << m_colorAutomatic;
		ar << m_bStdColorDlg;
	}
}
//************************************************************************************
void CBCGPColorMenuButton::CopyFrom (const CBCGPToolbarButton& s)
{
	CBCGPToolbarMenuButton::CopyFrom (s);

	const CBCGPColorMenuButton& src = (const CBCGPColorMenuButton&) s;

	m_Color = src.m_Color;
	m_ColorsByID.SetAt (m_nID, m_Color);	// Just to be happy :-)

	m_Colors.SetSize (src.m_Colors.GetSize ());

	for (int i = 0; i < m_Colors.GetSize (); i++)
	{
		m_Colors [i] = src.m_Colors [i];
	}

	m_bIsAutomaticButton =	    src.m_bIsAutomaticButton;    
	m_colorAutomatic =			src.m_colorAutomatic;
	m_bIsOtherButton =		    src.m_bIsOtherButton;        
	m_bIsDocumentColors =	    src.m_bIsDocumentColors;     
							                            
	m_strAutomaticButtonLabel = src.m_strAutomaticButtonLabel;
	m_strOtherButtonLabel =	    src.m_strOtherButtonLabel;   
	m_strDocumentColorsLabel =  src.m_strDocumentColorsLabel;

	m_nColumns =				src.m_nColumns;
	m_nVertDockColumns =		src.m_nVertDockColumns;
	m_nHorzDockRows =			src.m_nHorzDockRows;

	m_bStdColorDlg =			src.m_bStdColorDlg;
}
//*****************************************************************************
BOOL CBCGPColorMenuButton::OpenColorDialog (const COLORREF colorDefault, COLORREF& colorRes)
{
	BOOL bResult = FALSE;

	if (m_bStdColorDlg)
	{
		CColorDialog dlg (colorDefault, CC_FULLOPEN | CC_ANYCOLOR);
		if (dlg.DoModal () == IDOK)
		{
			colorRes = dlg.GetColor ();
			bResult = TRUE;
		}
	}
	else
	{
		CBCGPColorDialog dlg (colorDefault);
		if (dlg.DoModal () == IDOK)
		{
			colorRes = dlg.GetColor ();
			bResult = TRUE;
		}
	}

	return bResult;
}
//**************************************************************************************
CBCGPPopupMenu* CBCGPColorMenuButton::CreatePopupMenu ()
{
	CList<COLORREF,COLORREF> lstDocColors;
	if (m_bIsDocumentColors && m_pWndParent != NULL)
	{
		CFrameWnd* pOwner = BCGCBProGetTopLevelFrame (m_pWndParent);
		ASSERT_VALID (pOwner);

		//---------------------------
		// Fill document colors list:
		//---------------------------
		pOwner->SendMessage (BCGM_GETDOCUMENTCOLORS, (WPARAM) m_nID, 
			(LPARAM) &lstDocColors);
	}

	return new CColorPopup (m_Colors, m_Color, 
		(m_bIsAutomaticButton ? (LPCTSTR) m_strAutomaticButtonLabel : NULL),
		(m_bIsOtherButton ? (LPCTSTR) m_strOtherButtonLabel : NULL),
		(m_bIsDocumentColors ? (LPCTSTR) m_strDocumentColorsLabel : NULL),
		lstDocColors,
		m_nColumns, m_nHorzDockRows, m_nVertDockColumns, m_colorAutomatic, m_nID,
		m_bStdColorDlg);
}
//*************************************************************************************
void CBCGPColorMenuButton::SetColorName (COLORREF color, const CString& strName)
{
	CBCGPColorBar::m_ColorNames.SetAt (color, strName);
}
//*************************************************************************************
COLORREF CBCGPColorMenuButton::GetColorByCmdID (UINT uiCmdID)
{
	COLORREF color = (COLORREF)-1;
	m_ColorsByID.Lookup (uiCmdID, color);

	return color;
}

