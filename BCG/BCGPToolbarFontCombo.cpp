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
// BCGToolbarFontCombo.cpp: implementation of the CBCGPToolbarFontCombo class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BCGCBPro.h"
#include "BCGPToolBar.h"
#include "bcgprores.h"
#include "BCGPLocalResource.h"
#include "BCGPToolbarFontCombo.h"
#include "BCGPFontComboBox.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// CBCGPToolbarFontCombo

IMPLEMENT_SERIAL(CBCGPToolbarFontCombo, CBCGPToolbarComboBoxButton, 1)

CObList CBCGPToolbarFontCombo::m_lstFonts;
int CBCGPToolbarFontCombo::m_nCount = 0;
int CBCGPToolbarFontCombo::m_nFontHeight = 0;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGPToolbarFontCombo::CBCGPToolbarFontCombo() :
	m_nCharSet (DEFAULT_CHARSET),
	m_nFontType (DEVICE_FONTTYPE | RASTER_FONTTYPE | TRUETYPE_FONTTYPE),
	m_nPitchAndFamily (DEFAULT_PITCH)
{
	m_pLstFontsExternal = NULL;
	m_nCount++;
}
//****************************************************************************************
CBCGPToolbarFontCombo::CBCGPToolbarFontCombo (UINT uiID, int iImage,
											int nFontType,
											BYTE nCharSet,
											DWORD dwStyle, int iWidth,
											BYTE nPitchAndFamily) :
	CBCGPToolbarComboBoxButton (uiID, iImage, dwStyle, iWidth),
	m_nFontType (nFontType),
	m_nCharSet (nCharSet),
	m_nPitchAndFamily (nPitchAndFamily)
{
	m_pLstFontsExternal = NULL;

	if (m_nCount++ == 0)
	{
		RebuildFonts ();
	}

	SetContext ();
}
//****************************************************************************************
CBCGPToolbarFontCombo::CBCGPToolbarFontCombo (CObList* pLstFontsExternal,
											int nFontType,
											BYTE nCharSet,
											BYTE nPitchAndFamily) :
	m_nFontType (nFontType),
	m_nCharSet (nCharSet),
	m_nPitchAndFamily (nPitchAndFamily),
	m_pLstFontsExternal (pLstFontsExternal)
{
	ASSERT_VALID (m_pLstFontsExternal);
	RebuildFonts ();
}
//****************************************************************************************
CBCGPToolbarFontCombo::~CBCGPToolbarFontCombo()
{
	if (m_pLstFontsExternal == NULL)
	{
		if (--m_nCount == 0)
		{
			ClearFonts ();
		}
	}
}
//****************************************************************************************
void CBCGPToolbarFontCombo::RebuildFonts ()
{
	//------------------------------
	// First, take the screen fonts:
	//------------------------------
	CWindowDC dc (NULL);

	LOGFONT lf;
	memset(&lf, 0, sizeof(LOGFONT));
	lf.lfCharSet = m_nCharSet;

	::EnumFontFamiliesEx (dc.GetSafeHdc (), &lf,
		(FONTENUMPROC) EnumFamScreenCallBackEx, (LPARAM) this, NULL);

	//-----------------------------
	// Now, take the printer fonts:
	//-----------------------------
	CPrintDialog dlgPrint (FALSE);

	if (AfxGetApp ()->GetPrinterDeviceDefaults (&dlgPrint.m_pd))
	{
		HDC hDCPrint = dlgPrint.CreatePrinterDC ();
		if (hDCPrint != NULL)
		{
			::EnumFontFamiliesEx (hDCPrint, &lf,
				(FONTENUMPROC) EnumFamPrinterCallBackEx, (LPARAM) this, NULL);

			::DeleteObject (hDCPrint);
		}
	}
}
//**************************************************************************************
void CBCGPToolbarFontCombo::ClearFonts ()
{
	while (!m_lstFonts.IsEmpty ())
	{
		delete (CBCGPFontDesc*) m_lstFonts.RemoveHead ();
	}
}
//****************************************************************************************
void CBCGPToolbarFontCombo::SetContext ()
{
	for (POSITION pos = m_lstFonts.GetHeadPosition (); pos != NULL;)
	{
		CBCGPFontDesc* pDesc = (CBCGPFontDesc*) m_lstFonts.GetNext (pos);
		ASSERT_VALID (pDesc);

		if ((m_nFontType & pDesc->m_nType) != 0)
		{
			BOOL bIsUnique = GetFontsCount (pDesc->m_strName) <= 1;
			AddItem (bIsUnique ? pDesc->m_strName : pDesc->GetFullName (), (DWORD_PTR) pDesc);
		}
	}
}
//****************************************************************************************
BOOL CALLBACK AFX_EXPORT CBCGPToolbarFontCombo::EnumFamScreenCallBackEx(ENUMLOGFONTEX* pelf,
	NEWTEXTMETRICEX* lpntm, int FontType, LPVOID pThis)
{
	CBCGPToolbarFontCombo* pCombo = (CBCGPToolbarFontCombo*) pThis;
	ASSERT_VALID (pCombo);

	if ((FontType & TRUETYPE_FONTTYPE) == TRUETYPE_FONTTYPE ||
		(lpntm->ntmTm.ntmFlags & (NTM_PS_OPENTYPE | NTM_TT_OPENTYPE | NTM_TYPE1)) != 0)
	{
		FontType = TRUETYPE_FONTTYPE;

		if ((lpntm->ntmTm.ntmFlags & NTM_PS_OPENTYPE) == NTM_PS_OPENTYPE)
		{
			FontType |= PS_OPENTYPE_FONTTYPE;
		}
		if ((lpntm->ntmTm.ntmFlags & NTM_TT_OPENTYPE) == NTM_TT_OPENTYPE)
		{
			FontType |= TT_OPENTYPE_FONTTYPE;
		}
		if ((lpntm->ntmTm.ntmFlags & NTM_TYPE1) == NTM_TYPE1)
		{
			FontType |= TYPE1_FONTTYPE;
		}
	}

	pCombo->AddFont((ENUMLOGFONT*)pelf, FontType, CString(pelf->elfScript));
	return 1;
}
//****************************************************************************************
BOOL CALLBACK AFX_EXPORT CBCGPToolbarFontCombo::EnumFamPrinterCallBackEx(ENUMLOGFONTEX* pelf,
	NEWTEXTMETRICEX* /*lpntm*/, int FontType, LPVOID pThis)
{
	CBCGPToolbarFontCombo* pCombo = (CBCGPToolbarFontCombo*) pThis;
	ASSERT_VALID (pCombo);

	CString strName = pelf->elfLogFont.lfFaceName;

	pCombo->AddFont ((ENUMLOGFONT*)pelf, FontType, CString(pelf->elfScript));
	return 1;
}
//****************************************************************************************
BOOL CBCGPToolbarFontCombo::AddFont (ENUMLOGFONT* pelf, int nType, LPCTSTR lpszScript)
{
	LOGFONT& lf = pelf->elfLogFont;

	CObList& lstFonts = m_pLstFontsExternal != NULL ? 
		*m_pLstFontsExternal : m_lstFonts;

	//-----------------------------------------------
	// Don't put in MAC fonts, commdlg doesn't either
	//-----------------------------------------------
	if (lf.lfCharSet == MAC_CHARSET) 
	{
		return FALSE;
	}

	if (m_nPitchAndFamily != DEFAULT_PITCH &&
		(lf.lfPitchAndFamily & m_nPitchAndFamily) == 0)
	{
		return FALSE;
	}

	POSITION pos = NULL;
	for (pos = lstFonts.GetTailPosition (); pos != NULL;)
	{
		CBCGPFontDesc* pDesc = (CBCGPFontDesc*) lstFonts.GetPrev (pos);
		ASSERT_VALID (pDesc);

		if (pDesc->m_strName == lf.lfFaceName)
		{
			// Already in list
			return FALSE;
		}
	}

	//---------------------------------------------
	// Don't display vertical font for FE platform:
	//-----------------------------------------------
	if ((GetSystemMetrics (SM_DBCSENABLED)) && (lf.lfFaceName[0] == '@'))
	{
		return FALSE;
	}

	CBCGPFontDesc* pDesc = new CBCGPFontDesc (lf.lfFaceName, lpszScript,
		lf.lfCharSet, lf.lfPitchAndFamily, 
		nType & (RASTER_FONTTYPE | DEVICE_FONTTYPE | TRUETYPE_FONTTYPE), 
		nType & (PS_OPENTYPE_FONTTYPE | TT_OPENTYPE_FONTTYPE | TYPE1_FONTTYPE));
	ASSERT_VALID (pDesc);

	//------------------------------
	// Fonts list is sorted by name:
	//------------------------------
	BOOL bInserted = FALSE;
	for (pos = lstFonts.GetTailPosition (); !bInserted && pos != NULL;)
	{
		POSITION posSave = pos;

		CBCGPFontDesc* pDescList = (CBCGPFontDesc*) lstFonts.GetPrev (pos);
		ASSERT_VALID (pDescList);

		if (pDescList->m_strName < pDesc->m_strName || 
			(pDescList->m_strName == pDesc->m_strName && pDescList->m_strScript < pDesc->m_strScript))
		{
			lstFonts.InsertAfter (posSave, pDesc);
			bInserted = TRUE;
		}
	}

	if (!bInserted)
	{
		lstFonts.AddHead (pDesc);
	}

	return TRUE;
}
//**************************************************************************************
void CBCGPToolbarFontCombo::Serialize (CArchive& ar)
{
	// Override to disable item's data serialization!

	CBCGPToolbarButton::Serialize (ar);

	if (ar.IsLoading ())
	{
		ar >> m_iWidth;
		m_rect.right = m_rect.left + m_iWidth;
		ar >> m_dwStyle;
		ar >> m_iSelIndex;
		ar >> m_strEdit;
		ar >> m_nDropDownHeight;
		ar >> m_nFontType;
		ar >> m_nCharSet;

		if (m_lstFonts.IsEmpty ())
		{
			RebuildFonts ();
		}

		SetContext ();
		SelectItem (m_iSelIndex);
	}
	else
	{
		ar << m_iWidth;
		ar << m_dwStyle;
		ar << m_iSelIndex;
		ar << m_strEdit;
		ar << m_nDropDownHeight;
		ar << m_nFontType;
		ar << m_nCharSet;
	}
}
//***************************************************************************************
int CBCGPToolbarFontCombo::GetFontsCount (LPCTSTR lpszName)
{
	ASSERT (!m_lstFonts.IsEmpty ());

	int nCount = 0;

	for (POSITION pos = m_lstFonts.GetHeadPosition (); pos != NULL;)
	{
		CBCGPFontDesc* pDesc = (CBCGPFontDesc*) m_lstFonts.GetNext (pos);
		ASSERT_VALID (pDesc);

		if (pDesc->m_strName == lpszName)
		{
			nCount++;
		}
	}

	return nCount;
}
//********************************************************************************************
CComboBox* CBCGPToolbarFontCombo::CreateCombo (CWnd* pWndParent, const CRect& rect)
{
	CBCGPFontComboBox* pWndCombo = new CBCGPFontComboBox;
	if (!pWndCombo->Create (m_dwStyle | CBS_HASSTRINGS | CBS_OWNERDRAWFIXED, rect, pWndParent, m_nID))
	{
		delete pWndCombo;
		return NULL;
	}

	return pWndCombo;
}
//****************************************************************************************
BOOL CBCGPToolbarFontCombo::SetFont (LPCTSTR lpszName, BYTE nCharSet, BOOL bExact)
{
	ASSERT (lpszName != NULL);
	CString strNameFind = lpszName;
	strNameFind.MakeLower ();

	for (POSITION pos = m_lstItemData.GetHeadPosition (); pos != NULL;)
	{
		BOOL bFound = FALSE;

		CBCGPFontDesc* pDesc = (CBCGPFontDesc*) m_lstItemData.GetNext (pos);
		ASSERT_VALID (pDesc);

		CString strName = pDesc->GetFullName ();
		strName.MakeLower ();

		if (bExact)
		{
			if (strName == strNameFind ||
				(pDesc->m_strName.CompareNoCase(lpszName) == 0 && 
				(nCharSet == pDesc->m_nCharSet || nCharSet == DEFAULT_CHARSET)))
			{
				bFound = TRUE;
			}
		}
		else if (strName.Find (strNameFind) == 0 && 
			(nCharSet == DEFAULT_CHARSET || pDesc->m_nCharSet == nCharSet))
		{
			bFound = TRUE;
		}

		if (bFound)
		{
			SelectItem ((DWORD_PTR) pDesc);
			return TRUE;
		}
	}

	return FALSE;
}


//////////////////////////////////////////////////////////////////////
// CBCGPToolbarFontSizeCombo

static int nFontSizes[] =
	{8, 9, 10, 11, 12, 14, 16, 18, 20, 22, 24, 26, 28, 36, 48, 72};

IMPLEMENT_SERIAL(CBCGPToolbarFontSizeCombo, CBCGPToolbarComboBoxButton, 1)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGPToolbarFontSizeCombo::CBCGPToolbarFontSizeCombo()
{
	m_nTwipsLast = 0;
	m_nLogVert = 0;
}
//****************************************************************************************
CBCGPToolbarFontSizeCombo::CBCGPToolbarFontSizeCombo (UINT uiID, int iImage,
											DWORD dwStyle, int iWidth) :
	CBCGPToolbarComboBoxButton (uiID, iImage, dwStyle, iWidth)
{
	m_nTwipsLast = 0;
	m_nLogVert = 0;
}
//****************************************************************************************
CBCGPToolbarFontSizeCombo::~CBCGPToolbarFontSizeCombo()
{
}
//****************************************************************************************
void CBCGPToolbarFontSizeCombo::RebuildFontSizes (const CString& strFontName)
{
	if (strFontName.IsEmpty ())
	{
		return;
	}

	CString strText = m_strEdit;

	if (m_pWndCombo != NULL)
	{
		m_pWndCombo->SetRedraw (FALSE);
	}

	CWindowDC dc (NULL);

	RemoveAllItems ();

	m_nLogVert = dc.GetDeviceCaps (LOGPIXELSY);
	::EnumFontFamilies (dc.GetSafeHdc (), strFontName,
		(FONTENUMPROC) EnumSizeCallBack, (LPARAM) this);

	if (!SelectItem (strText))
	{
		m_strEdit = strText;
		if (m_pWndCombo != NULL)
		{
			m_pWndCombo->SetWindowText (m_strEdit);
		}
	}

	// Synchronize context with other comboboxes with the same ID:
	CObList listButtons;
	if (CBCGPToolBar::GetCommandButtons (m_nID, listButtons) > 0)
	{
		for (POSITION posCombo = listButtons.GetHeadPosition (); posCombo != NULL;)
		{
			CBCGPToolbarComboBoxButton* pCombo = 
				DYNAMIC_DOWNCAST (CBCGPToolbarComboBoxButton, listButtons.GetNext (posCombo));

			if (pCombo != NULL && pCombo != this)
			{
				if (pCombo->GetComboBox () != NULL)
				{
					pCombo->GetComboBox ()->SetRedraw (FALSE);
				}

				pCombo->RemoveAllItems ();

				POSITION pos;
				POSITION posData;

				for (pos = m_lstItems.GetHeadPosition (),
					posData = m_lstItemData.GetHeadPosition (); 
					pos != NULL && posData != NULL;)
				{
					pCombo->AddItem (m_lstItems.GetNext (pos),
									m_lstItemData.GetNext (posData));
				}

				if (pCombo->GetComboBox () != NULL)
				{
					pCombo->GetComboBox ()->SetRedraw ();
				}

			}
		}
	}

	if (m_pWndCombo != NULL)
	{
		m_pWndCombo->SetRedraw ();
	}
}
//****************************************************************************************
BOOL FAR PASCAL CBCGPToolbarFontSizeCombo::EnumSizeCallBack(LOGFONT FAR* /*lplf*/,
							LPNEWTEXTMETRIC lpntm,int FontType, LPVOID lpv)
{
	CBCGPToolbarFontSizeCombo* pThis = (CBCGPToolbarFontSizeCombo*) lpv;
	ASSERT_VALID (pThis);

	if ((FontType & TRUETYPE_FONTTYPE) ||
		!((FontType & TRUETYPE_FONTTYPE) || (FontType & RASTER_FONTTYPE)))
		// if truetype or vector font
	{
		// this occurs when there is a truetype and nontruetype version of a font
		for (int i = 0; i < 16; i++)
		{
			CString strSize;
			strSize.Format (_T("%d"), nFontSizes[i]);

			pThis->AddItem (strSize);
		}

		return FALSE; // don't call me again
	}

	// calc character height in pixels
	pThis->InsertSize(MulDiv(lpntm->tmHeight-lpntm->tmInternalLeading,
		1440, pThis->m_nLogVert));

	return TRUE; // call me again
}
//****************************************************************************************
CString CBCGPToolbarFontSizeCombo::TwipsToPointString (int nTwips)
{
	CString str;
	if (nTwips >= 0)
	{
		// round to nearest half point
		nTwips = (nTwips + 5) / 10;

		if ((nTwips % 2) == 0)
		{
			str.Format (_T("%ld"), nTwips/2);
		}
		else
		{
			str.Format (_T("%.1f"), (float) nTwips / 2.F);
		}
	}

	return str;
}
//*****************************************************************************************
void CBCGPToolbarFontSizeCombo::SetTwipSize(int nTwips)
{
	SetText (TwipsToPointString (nTwips));
}
//*****************************************************************************************
int CBCGPToolbarFontSizeCombo::GetTwipSize() const
{
	// return values
	// -2 -- error
	// -1 -- edit box empty
	// >=0 -- font size in twips

	CString str = GetItem () == NULL ? m_strEdit : GetItem ();
	if (m_lstItems.Find (m_strEdit) == NULL)
	{
		str = m_strEdit;
	}

	LPCTSTR lpszText = str;

	while (*lpszText == ' ' || *lpszText == '\t')
		lpszText++;

	if (lpszText[0] == NULL)
		return -1; // no text in control

	double d = _tcstod(lpszText, (LPTSTR*)&lpszText);
	while (*lpszText == ' ' || *lpszText == '\t')
		lpszText++;

	if (*lpszText != NULL)
		return -2;   // not terminated properly

	return (d<0.) ? 0 : (int)(d*20.);
}
//****************************************************************************************
void CBCGPToolbarFontSizeCombo::InsertSize (int nSize)
{
	ASSERT(nSize > 0);
	AddItem (TwipsToPointString (nSize), (DWORD) nSize);
}
//********************************************************************************************
CComboBox* CBCGPToolbarFontSizeCombo::CreateCombo (CWnd* pWndParent, const CRect& rect)
{
	CBCGPFontComboBox* pWndCombo = new CBCGPFontComboBox;
	if (!pWndCombo->Create (m_dwStyle, rect, pWndParent, m_nID))
	{
		delete pWndCombo;
		return NULL;
	}

	return pWndCombo;
}
//****************************************************************************************
void CBCGPToolbarFontCombo::CopyFrom (const CBCGPToolbarButton& s)
{
	CBCGPToolbarComboBoxButton::CopyFrom(s);

	const CBCGPToolbarFontCombo& src = (const CBCGPToolbarFontCombo&) s;

	m_nCharSet = src.m_nCharSet;
	m_nFontType = src.m_nFontType;
	m_nPitchAndFamily = src.m_nPitchAndFamily;
}

