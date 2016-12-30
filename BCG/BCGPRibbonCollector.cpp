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
//
// BCGPRibbonCollector.cpp: implementation of the CBCGPRibbonCollector class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BCGPRibbonCollector.h"

#include "BCGPRibbonLabel.h"
#include "BCGPRibbonCheckBox.h"
#include "BCGPRibbonButtonsGroup.h"
#include "BCGPRibbonComboBox.h"
#include "BCGPRibbonUndoButton.h"
#include "BCGPRibbonColorButton.h"
#include "BCGPRibbonHyperlink.h"
#include "BCGPRibbonProgressBar.h"
#include "BCGPRibbonSlider.h"
#include "BCGPRibbonStatusBarPane.h"
#include "BCGPDrawManager.h"
#include "BCGPRibbonBackstageViewPanel.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#ifndef BCGP_EXCLUDE_RIBBON

namespace BCGPRibbonCollector
{

class CMemoryDC
{
public:
	CMemoryDC()
		: m_pOldBitmap (NULL)
		, m_Size       (0, 0)
	{
	}

	void CreateDC ()
	{
		if (m_DC.GetSafeHdc () != NULL)
		{
			return;
		}

		HDC hDC = ::GetDC (NULL);

		HDC hNewDC = ::CreateCompatibleDC (hDC);
		if (hNewDC != NULL)
		{
			m_DC.Attach (hNewDC);
		}

		::ReleaseDC (NULL, hDC);
	}
	void SetSize (const CSize& size)
	{
		if (m_DC.GetSafeHdc () == NULL)
		{
			CreateDC ();
		}

		if (m_Bitmap.GetSafeHandle () != NULL)
		{
			if (m_Size.cx != size.cx || m_Size.cy != size.cy)
			{
				if (m_pOldBitmap != NULL)
				{
					m_DC.SelectObject (m_pOldBitmap);
				}

				m_Bitmap.DeleteObject ();
			}
		}

		m_Size = size;

		if (m_Bitmap.GetSafeHandle () == NULL)
		{
			HBITMAP hbmp = CBCGPDrawManager::CreateBitmap_32 (size, NULL);
			if (hbmp != NULL)
			{
				m_Bitmap.Attach (hbmp);
				m_pOldBitmap = (CBitmap*) m_DC.SelectObject (&m_Bitmap);
			}
		}
	}

	const CSize& GetSize () const
	{
		return m_Size;
	}

	CDC& GetDC ()
	{
		return m_DC;
	}
	const CDC& GetDC () const
	{
		return m_DC;
	}

	CBitmap& GetBitmap ()
	{
		return m_Bitmap;
	}
	const CBitmap& GetBitmap () const
	{
		return m_Bitmap;
	}

protected:
	CDC			m_DC;
	CBitmap		m_Bitmap;
	CBitmap*	m_pOldBitmap;
	
	CSize		m_Size;
};

static BOOL AddIcon (CBCGPToolBarImages& images, HICON hIcon)
{
	if (hIcon == NULL)
	{
		return FALSE;
	}

	ICONINFO ii;
	::GetIconInfo (hIcon, &ii);

	CSize size;
	{
		BITMAP bmp;
		if (::GetObject (ii.hbmColor, sizeof (BITMAP), &bmp) == 0)
		{
			ASSERT (FALSE);
			return NULL;
		}

		size.cx = bmp.bmWidth;
		size.cy = bmp.bmHeight;
	}

	::DeleteObject (ii.hbmColor);
	::DeleteObject (ii.hbmMask);

	CMemoryDC dcColor;
	dcColor.SetSize (size);
	::DrawIconEx (dcColor.GetDC ().GetSafeHdc (), 
		0, 0, 
		hIcon, 
		size.cx, size.cy, 0, NULL,
		DI_NORMAL);

	BITMAP bmpColor;
	dcColor.GetBitmap ().GetBitmap (&bmpColor);
	RGBQUAD* pColor = (RGBQUAD*) bmpColor.bmBits;

	BOOL bConvert = TRUE;
	for (int i = 0; i < size.cx * size.cy; i++)
	{
		if (pColor[i].rgbReserved != 0)
		{
			bConvert = FALSE;
			break;
		}
	}

	if (bConvert)
	{
		CMemoryDC dcMask;
		dcMask.SetSize (size);
		::DrawIconEx (dcMask.GetDC ().GetSafeHdc (), 
			0, 0, 
			hIcon, 
			size.cx, size.cy, 0, NULL,
			DI_MASK);


		BITMAP bmpMask;
		dcMask.GetBitmap ().GetBitmap (&bmpMask);
		RGBQUAD* pMask  = (RGBQUAD*) bmpMask.bmBits;

		if (pColor == NULL || pMask == NULL)
		{
			ASSERT (FALSE);
			return NULL;
		}

		// add alpha channel
		for (int i = 0; i < size.cx * size.cy; i++)
		{
			pColor->rgbReserved = (BYTE) (255 - pMask->rgbRed);
			pColor++;
			pMask++;
		}
	}

	BOOL bRes = FALSE;

	HBITMAP bitmap = CBCGPDrawManager::CreateBitmap_32 (dcColor.GetBitmap ());
	if (bitmap != NULL)
	{
		bRes = images.AddImage (bitmap, FALSE) != -1;
		::DeleteObject (bitmap);
	}

	return bRes;
}

};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGPRibbonCollector::CBCGPRibbonCollector(CBCGPRibbonInfo& info, DWORD dwFlags)
	: m_Info    (info)
	, m_dwFlags (dwFlags)
{
}

CBCGPRibbonCollector::~CBCGPRibbonCollector()
{
}

void CBCGPRibbonCollector::CollectRibbonBar(const CBCGPRibbonBar& bar)
{
	GetInfo ().SetVersionStamp (bar.m_VersionStamp);

	CollectRibbonBar (bar, GetInfo ().GetRibbonBar ());
}

void CBCGPRibbonCollector::CollectRibbonBar(const CBCGPRibbonBar& bar, CBCGPRibbonInfo::XRibbonBar& info)
{
	info.m_bToolTip = bar.IsToolTipEnabled ();
	info.m_bToolTipDescr = bar.IsToolTipDescrEnabled ();
	info.m_bKeyTips = bar.IsKeyTipEnabled ();
	info.m_bPrintPreview = bar.IsPrintPreviewEnabled ();
	info.m_bBackstageMode = bar.IsBackstageMode ();
	info.m_bDrawUsingFont = CBCGPRibbonFontComboBox::m_bDrawUsingFont;

	// main button
	CBCGPRibbonMainButton* pBtnMain = bar.GetMainButton ();
	if (pBtnMain != NULL)
	{
		info.m_btnMain = new CBCGPRibbonInfo::XElementButtonMain;
		CollectElement (*pBtnMain, *info.m_btnMain);
	}

	info.m_MainCategory = new CBCGPRibbonInfo::XCategoryMain;
	CollectCategoryMain (bar, *info.m_MainCategory);

	info.m_BackstageCategory = new CBCGPRibbonInfo::XCategoryBackstage;
	CollectCategoryBackstage (bar, *info.m_BackstageCategory);

	int i = 0;

	// QAT elements
	CollectQATElements (bar, info);

	// tab elements
	CollectElement (bar.m_TabElements, info.m_TabElements);

	int index = info.m_bPrintPreview ? 1 : 0;
	int count = bar.GetCategoryCount ();

	for (i = index; i < count; i++)
	{
		CBCGPRibbonCategory* pCategory = bar.GetCategory (i);
		ASSERT_VALID (pCategory);

		if (pCategory->GetContextID () == 0)
		{
			CBCGPRibbonInfo::XCategory* pInfo = new CBCGPRibbonInfo::XCategory;
			CollectCategory (*pCategory, *pInfo);
			info.m_arCategories.Add (pInfo);
		}
	}

	UINT uiContextID = 0;
	CBCGPRibbonInfo::XContext* pInfoContext = NULL;
	for (i = index; i < count; i++)
	{
		CBCGPRibbonCategory* pCategory = bar.GetCategory (i);
		ASSERT_VALID (pCategory);

		UINT uiID = pCategory->GetContextID ();
		if (uiID != 0)
		{
			if (uiContextID != uiID)
			{
				uiContextID = uiID;
				pInfoContext = new CBCGPRibbonInfo::XContext;

				CBCGPRibbonContextCaption* pCaption = bar.FindContextCaption (uiContextID);
				ASSERT_VALID (pCaption);

				pInfoContext->m_strText = pCaption->GetText ();
				GetID (*pCaption, pInfoContext->m_ID);
				pInfoContext->m_Color = pCaption->GetColor ();

				info.m_arContexts.Add (pInfoContext);
			}

			if (pInfoContext != NULL)
			{
				CBCGPRibbonInfo::XCategory* pInfo = new CBCGPRibbonInfo::XCategory;
				CollectCategory (*pCategory, *pInfo);
				pInfoContext->m_arCategories.Add (pInfo);
			}
		}
	}

	// panel images
	GetRibbonBarImages (bar, info);
}

void CBCGPRibbonCollector::CollectStatusBar(const CBCGPRibbonStatusBar& bar)
{
	CollectStatusBar (bar, GetInfo ().GetStatusBar ());
}

void CBCGPRibbonCollector::CollectStatusBar(const CBCGPRibbonStatusBar& bar, CBCGPRibbonInfo::XStatusBar& info)
{
	DWORD dwFlags = m_dwFlags;
	m_dwFlags &= ~e_CollectGroupImages;

	// standard SB elements count
	int i = 0;
	int count = bar.GetCount ();

	CBCGPRibbonInfo::XStatusBar::XStatusElements& infoElements = info.m_Elements;
	for (i = 0; i < count; i++)
	{
		const CBCGPBaseRibbonElement* pElement = 
			const_cast<CBCGPRibbonStatusBar&>(bar).GetElement (i);
		ASSERT_VALID (pElement);

		CBCGPRibbonInfo::XElement* pElementInfo = CollectElement (*pElement);
		if (pElementInfo != NULL)
		{
			infoElements.m_arElements.Add (pElementInfo);
			infoElements.m_arLabels.Add (bar.GetLabel (pElement));
			infoElements.m_arVisible.Add (pElement->IsVisible ());
		}
	}

	// extended SB elements count
	count = bar.GetExCount ();

	CBCGPRibbonInfo::XStatusBar::XStatusElements& infoExElements = info.m_ExElements;
	for (i = 0; i < count; i++)
	{
		const CBCGPBaseRibbonElement* pElement = 
			const_cast<CBCGPRibbonStatusBar&>(bar).GetExElement (i);
		ASSERT_VALID (pElement);

		CBCGPRibbonInfo::XElement* pElementInfo = CollectElement (*pElement);
		if (pElementInfo != NULL)
		{
			infoExElements.m_arElements.Add (pElementInfo);
			infoExElements.m_arLabels.Add (bar.GetLabel (pElement));
			infoExElements.m_arVisible.Add (pElement->IsVisible ());
		}
	}

	// SB images
	GetStatusBarImages (bar, info);

	m_dwFlags = dwFlags;
}

void CBCGPRibbonCollector::CollectQATElements (const CBCGPRibbonBar& bar, CBCGPRibbonInfo::XRibbonBar& info)
{
    const CBCGPRibbonQATDefaultState& state = bar.m_QAToolbar.m_DefaultState;

	for (int i = 0; i < state.m_arCommands.GetSize(); i++)
	{
        UINT nID = state.m_arCommands[i];
        if (nID != 0)
        {
            CBCGPRibbonInfo::XQAT::XQATItem item;
            item.m_ID.m_Value = nID;
            item.m_bVisible   = state.m_arVisibleState[i];
            info.m_QAT.m_arItems.Add(item);
        }
	}

	info.m_QAT.m_bOnTop = bar.m_bQuickAccessToolbarOnTop;
}

void CBCGPRibbonCollector::CollectCategoryMain(const CBCGPRibbonBar& bar, CBCGPRibbonInfo::XCategoryMain& info)
{
	CBCGPRibbonCategory* pCategory = bar.GetMainCategory ();
	if (pCategory == NULL)
	{
		return;
	}

	info.m_strName = pCategory->GetName ();

	if (pCategory->GetPanelCount () > 0)
	{
		CBCGPRibbonMainPanel* pPanel = DYNAMIC_DOWNCAST (CBCGPRibbonMainPanel, pCategory->GetPanel (0));
		if (pPanel == NULL)
		{
			return;
		}

		int i = 0;
		int count = pPanel->GetCount ();

		CBCGPRibbonEdit* pSearchBox = (CBCGPRibbonEdit*)pPanel->m_pSearchBox;

		for (i = 0; i < count; i++)
		{
			CBCGPBaseRibbonElement* pElement = pPanel->GetElement (i);
			ASSERT_VALID (pElement);

			if (pElement != pPanel->m_pElemOnRight && pElement != pSearchBox)
			{
				CBCGPRibbonInfo::XElement* pElementInfo = CollectElement (*pElement);
				if (pElementInfo != NULL)
				{
					info.m_arElements.Add (pElementInfo);
				}
			}
		}

		if (pPanel->m_pElemOnRight != NULL)
		{
			CBCGPRibbonRecentFilesList* pRecentList = DYNAMIC_DOWNCAST(CBCGPRibbonRecentFilesList, pPanel->m_pElemOnRight);
			if (pRecentList != NULL)
			{
				info.m_bRecentListEnable   = pRecentList != NULL;
				info.m_bRecentListShowPins = pRecentList->m_bShowPins;
			}

			info.m_strRecentListLabel = pPanel->m_pElemOnRight->GetText ();
			info.m_nRecentListWidth   = pPanel->m_nRightPaneWidth;
		}

		if (pSearchBox != NULL)
		{
			info.m_bSearchEnable = TRUE;
			info.m_strSearchLabel = pSearchBox->GetText ();
			info.m_strSearchKeys = pSearchBox->GetKeys ();
			info.m_nSearchWidth = pPanel->m_nSearchBoxWidth;
		}
	}

	GetCategoryImages (*pCategory, info.m_SmallImages, info.m_LargeImages);
}

void CBCGPRibbonCollector::CollectCategoryBackstage(const CBCGPRibbonBar& bar, CBCGPRibbonInfo::XCategoryBackstage& info)
{
	CBCGPRibbonCategory* pCategory = bar.GetBackstageCategory ();
	if (pCategory == NULL)
	{
		return;
	}

	info.m_strName = pCategory->GetName ();

	CBCGPRibbonInfo::XImage dummyImages;
	GetCategoryImages (*pCategory, info.m_SmallImages, dummyImages);

	if (pCategory->GetPanelCount () > 0)
	{
		CBCGPRibbonBackstageViewPanel* pPanel = DYNAMIC_DOWNCAST (CBCGPRibbonBackstageViewPanel, pCategory->GetPanel (0));
		if (pPanel == NULL)
		{
			return;
		}

		int i = 0;
		int count = pPanel->GetCount ();

		for (i = 0; i < count; i++)
		{
			CBCGPBaseRibbonElement* pElement = pPanel->GetElement (i);
			ASSERT_VALID (pElement);

			CBCGPRibbonInfo::XElement* pElementInfo = CollectElement (*pElement);
			if (pElementInfo != NULL)
			{
				info.m_arElements.Add (pElementInfo);
			}
		}
	}
}

void CollectHiddenImage (CBCGPRibbonInfo::XArrayElement& arElements, const CBCGPRibbonInfo::XArrayElement& arHidden, BOOL bSubItems = FALSE)
{
	if (arElements.GetSize () == 0)
	{
		return;
	}

	for (int i = 0; i < arElements.GetSize (); i++)
	{
		CBCGPRibbonInfo::XElement* pElement = arElements[i];
		if (pElement->GetElementName ().Compare (CBCGPRibbonInfo::s_szGroup) == 0)
		{
			CollectHiddenImage (((CBCGPRibbonInfo::XElementGroup*)pElement)->m_arButtons, arHidden, FALSE);
		}
		else if (pElement->GetElementName ().Compare (CBCGPRibbonInfo::s_szButton) == 0 ||
			pElement->GetElementName ().Compare (CBCGPRibbonInfo::s_szButton_Palette) == 0 ||
			pElement->GetElementName ().Compare (CBCGPRibbonInfo::s_szButton_Color) == 0)
		{
			CBCGPRibbonInfo::XElementButton* pButton = (CBCGPRibbonInfo::XElementButton*)pElement;
			if (bSubItems && !pButton->m_ID.IsEmpty () && pButton->m_nSmallImageIndex == -1)
			{
				for (int j = 0; j < arHidden.GetSize (); j++)
				{
					CBCGPRibbonInfo::XElement* pHidden = arHidden[j];
					if (pButton->m_ID == pHidden->m_ID && 
						(pHidden->GetElementName ().Compare (CBCGPRibbonInfo::s_szButton) == 0 ||
						pHidden->GetElementName ().Compare (CBCGPRibbonInfo::s_szButton_Palette) == 0 ||
						pHidden->GetElementName ().Compare (CBCGPRibbonInfo::s_szButton_Color) == 0))
					{
						pButton->m_nSmallImageIndex = ((CBCGPRibbonInfo::XElementButton*)pHidden)->m_nSmallImageIndex;
						break;
					}
				}
			}

			CollectHiddenImage (pButton->m_arSubItems, arHidden, TRUE);
		}
	}
}

void CBCGPRibbonCollector::CollectCategory(const CBCGPRibbonCategory& category, CBCGPRibbonInfo::XCategory& info)
{
	info.m_strName = category.GetName ();
	info.m_strKeys = category.m_Tab.GetKeys ();

	int i = 0;
	int count = category.GetPanelCount ();

	for (i = 0; i < count; i++)
	{
		const CBCGPRibbonPanel* pPanel = 
			const_cast<CBCGPRibbonCategory&>(category).GetPanel (i);
		ASSERT_VALID (pPanel);

		CBCGPRibbonInfo::XPanel* pInfo = new CBCGPRibbonInfo::XPanel;
		CollectPanel (*pPanel, *pInfo);
		info.m_arPanels.Add (pInfo);
	}

	CBCGPRibbonInfo::XArrayElement arHidden;
	const CArray <CBCGPBaseRibbonElement*, CBCGPBaseRibbonElement*>& arElements = 
		category.m_arElements;
	for (i = 0; i < (int)arElements.GetSize (); i++)
	{
		CBCGPBaseRibbonElement* pElement = arElements[i];
		if (DYNAMIC_DOWNCAST (CBCGPRibbonDefaultPanelButton, pElement) == NULL)
		{
			ASSERT_VALID (pElement);

			CBCGPRibbonInfo::XElement* pElementInfo = CollectElement (*pElement);
			if (pElementInfo != NULL)
			{
				arHidden.Add (pElementInfo);
			}
		}
	}

	// hidden element count
	if (arHidden.GetSize () > 0)
	{
		if ((GetFlags() & e_CollectHiddenElements) == e_CollectHiddenElements)
		{
			info.m_arElements.Copy (arHidden);
			arHidden.RemoveAll ();
		}
		else
		{
			for (i = 0; i < info.m_arPanels.GetSize (); i++)
			{
				CollectHiddenImage (info.m_arPanels[i]->m_arElements, arHidden, FALSE);
			}
		}

		for (i = 0; i < arHidden.GetSize (); i++)
		{
			delete arHidden[i];
		}
	}

	info.m_arCollapseOrder.Copy (category.m_arCollapseOrder);

	GetCategoryImages (category, info);
}

void CBCGPRibbonCollector::CollectPanel(const CBCGPRibbonPanel& panel, CBCGPRibbonInfo::XPanel& info)
{
 	info.m_strName = panel.GetName ();
	info.m_strKeys = const_cast<CBCGPRibbonPanel&>(panel).GetDefaultButton ().GetKeys ();
	info.m_nImageIndex = const_cast<CBCGPRibbonPanel&>(panel).GetDefaultButton().GetImageIndex(FALSE);
	info.m_bJustifyColumns = panel.IsJustifyColumns ();
	info.m_bCenterColumnVert = panel.IsCenterColumnVert ();

	CollectElement (const_cast<CBCGPRibbonPanel&>(panel).GetLaunchButton (), info.m_btnLaunch);

	int i = 0;
	int count = panel.GetCount ();

	for (i = 0; i < count; i++)
	{
		CBCGPBaseRibbonElement* pElement = panel.GetElement (i);
		ASSERT_VALID (pElement);

		CBCGPRibbonInfo::XElement* pElementInfo = CollectElement (*pElement);
		if (pElementInfo != NULL)
		{
			info.m_arElements.Add (pElementInfo);
		}
	}
}

CBCGPRibbonInfo::XElement* CBCGPRibbonCollector::CollectElement(const CBCGPBaseRibbonElement& element)
{
	CBCGPRibbonInfo::XElement* info = NULL;
/*
	if (element.IsKindOf (RUNTIME_CLASS (CBCGPRibbonMainButton)))
	{
		ar << CBCGPRibbonInfo::s_szButton_Main;

		CBCGPRibbonMainButton* pElement = DYNAMIC_DOWNCAST (CBCGPRibbonMainButton, &element);
		ASSERT_VALID (pElement);

		SerializeBase (*pElement, ar, dwVersion);

		HBITMAP bitmap = ((CBCGPRibbonMainButtonF*)pElement)->m_Image.GetImageWell ();
		SerializeBitmap (ar, bitmap);
	}
	else if (element.IsKindOf (RUNTIME_CLASS (CBCGPRibbonLaunchButton)))
	{
		ar << CBCGPRibbonInfo::s_szButton_Launch;

		CBCGPRibbonLaunchButton* pElement = (CBCGPRibbonLaunchButton*)&element;
		ASSERT_VALID (pElement);

		SerializeBase (*pElement, ar, dwVersion);
	}
	else */
	if (element.IsKindOf (RUNTIME_CLASS (CBCGPRibbonButtonsGroup)))
	{
		CBCGPRibbonInfo::XElementGroup* pNewInfo = new CBCGPRibbonInfo::XElementGroup;
		info = pNewInfo;
		
		CollectElement (element, *info);
	}
	else if (element.IsKindOf (RUNTIME_CLASS (CBCGPRibbonLabel)))
	{
		CBCGPRibbonInfo::XElementLabel* pNewInfo = new CBCGPRibbonInfo::XElementLabel;
		info = pNewInfo;

		CollectBaseElement (element, *info);
	}
	else if (element.IsKindOf (RUNTIME_CLASS (CBCGPRibbonFontComboBox)))
	{
		CBCGPRibbonInfo::XElementFontComboBox* pNewInfo = new CBCGPRibbonInfo::XElementFontComboBox;
		info = pNewInfo;

		CollectBaseElement (element, *info);

		const CBCGPRibbonFontComboBox* pElement = (const CBCGPRibbonFontComboBox*)&element;
		ASSERT_VALID (pElement);

		pNewInfo->m_nWidth = pElement->GetWidth (FALSE);
		pNewInfo->m_nWidthFloaty = pElement->GetWidth (TRUE);
		pNewInfo->m_nTextAlign = pElement->GetTextAlign ();
		pNewInfo->m_bHasEditBox = pElement->HasEditBox ();
		pNewInfo->m_sortOrder = pElement->GetSortOrder ();
		pNewInfo->m_bResizeDropDownList = pElement->IsResizeDropDownList ();
		pNewInfo->m_strSearchPrompt = pElement->GetPrompt();

		if (pNewInfo->m_bHasEditBox)
		{
			pNewInfo->m_bAutoComplete = pElement->IsAutoCompleteEnabled ();
		}
		
		pNewInfo->m_nFontType = pElement->GetFontType ();
		pNewInfo->m_nCharSet = pElement->GetCharSet ();
		pNewInfo->m_nPitchAndFamily = pElement->GetPitchAndFamily ();
	}
	else if (element.IsKindOf (RUNTIME_CLASS (CBCGPRibbonComboBox)))
	{
		CBCGPRibbonInfo::XElementComboBox* pNewInfo = new CBCGPRibbonInfo::XElementComboBox;
		info = pNewInfo;

		CollectBaseElement (element, *info);

		const CBCGPRibbonComboBox* pElement = (const CBCGPRibbonComboBox*)&element;
		ASSERT_VALID (pElement);

		pNewInfo->m_nWidth = pElement->GetWidth (FALSE);
		pNewInfo->m_nWidthFloaty = pElement->GetWidth (TRUE);
		pNewInfo->m_nTextAlign = pElement->GetTextAlign ();
		pNewInfo->m_bHasEditBox = pElement->HasEditBox ();
		pNewInfo->m_sortOrder = pElement->GetSortOrder ();
		pNewInfo->m_strSearchPrompt = pElement->GetPrompt();

		pNewInfo->m_bCalculatorMode = pElement->IsCalculatorEnabled ();
		if (pNewInfo->m_bCalculatorMode)
		{
			pNewInfo->m_lstCalculatorExt.AddTail ((CList<UINT, UINT>*)&(pElement->m_lstCalcExtCommands));
		}
		else
		{
			pNewInfo->m_bResizeDropDownList = pElement->IsResizeDropDownList ();
			if (pNewInfo->m_bHasEditBox)
			{
				pNewInfo->m_bAutoComplete = pElement->IsAutoCompleteEnabled ();
			}

			pNewInfo->m_bSearchMode = pElement->IsSearchMode ();
			if (pNewInfo->m_bSearchMode)
			{
				pNewInfo->m_strSearchPrompt = pElement->GetSearchPrompt ();
			}

			int i = 0;
			int count = (int)pElement->GetCount ();

			for (i = 0; i < count; i++)
			{
				pNewInfo->m_arItems.Add (pElement->GetItem (i));
			}
		}
	}
	else if (element.IsKindOf (RUNTIME_CLASS (CBCGPRibbonEdit)))
	{
		CBCGPRibbonInfo::XElementEdit* pNewInfo = new CBCGPRibbonInfo::XElementEdit;
		info = pNewInfo;

		CollectBaseElement (element, *info);

		const CBCGPRibbonEdit* pElement = (const CBCGPRibbonEdit*)&element;
		ASSERT_VALID (pElement);

		pNewInfo->m_nWidth = pElement->GetWidth (FALSE);
		pNewInfo->m_nWidthFloaty = pElement->GetWidth (TRUE);
		pNewInfo->m_nTextAlign = pElement->GetTextAlign ();
		pNewInfo->m_strSearchPrompt = pElement->GetPrompt();

		pNewInfo->m_bSearchMode = pElement->IsSearchMode ();
		if (pNewInfo->m_bSearchMode)
		{
			pNewInfo->m_strSearchPrompt = pElement->GetSearchPrompt ();
		}
		else
		{
			pNewInfo->m_bHasSpinButtons = pElement->HasSpinButtons ();
			if (pNewInfo->m_bHasSpinButtons)
			{
				pNewInfo->m_nMin = pElement->GetRangeMin ();
				pNewInfo->m_nMax = pElement->GetRangeMax ();
			}
		}
		pNewInfo->m_strValue = pElement->GetEditText ();
	}
	else if (element.IsKindOf (RUNTIME_CLASS (CBCGPRibbonUndoButton)))
	{
		CBCGPRibbonInfo::XElementButtonUndo* pNewInfo = new CBCGPRibbonInfo::XElementButtonUndo;
		info = pNewInfo;

		CollectBaseElement (element, *info, FALSE);

		const CBCGPRibbonUndoButton* pElement = (const CBCGPRibbonUndoButton*)&element;
		ASSERT_VALID (pElement);

		pNewInfo->m_bIsButtonMode = pElement->IsButtonMode ();
		pNewInfo->m_bEnableMenuResize = pElement->IsMenuResizeEnabled ();
		pNewInfo->m_bMenuResizeVertical = pElement->IsMenuResizeVertical ();
		pNewInfo->m_bDrawDisabledItems = pElement->IsDrawDisabledItems();
	}
	else if (element.IsKindOf (RUNTIME_CLASS (CBCGPRibbonColorButton)))
	{
		CBCGPRibbonInfo::XElementButtonColor* pNewInfo = new CBCGPRibbonInfo::XElementButtonColor;
		info = pNewInfo;

		CollectBaseElement (element, *info);

		const CBCGPRibbonColorButton* pElement = (const CBCGPRibbonColorButton*)&element;
		ASSERT_VALID (pElement);

		pNewInfo->m_bIsButtonMode = pElement->IsButtonMode ();
		pNewInfo->m_bEnableMenuResize = pElement->IsMenuResizeEnabled ();
		pNewInfo->m_bMenuResizeVertical = pElement->IsMenuResizeVertical ();
		pNewInfo->m_bDrawDisabledItems = pElement->IsDrawDisabledItems();
		pNewInfo->m_nIconsInRow = pElement->GetIconsInRow ();

		pNewInfo->m_clrColor = pElement->GetColor ();

		pNewInfo->m_strAutomaticBtnLabel = pElement->m_strAutomaticButtonLabel;
		pNewInfo->m_strAutomaticBtnToolTip = pElement->m_strAutomaticButtonToolTip;
		pNewInfo->m_clrAutomaticBtnColor = pElement->m_ColorAutomatic;
		pNewInfo->m_bAutomaticBtnOnTop = pElement->m_bIsAutomaticButtonOnTop;
		pNewInfo->m_bAutomaticBtnBorder = pElement->m_bIsAutomaticButtonBorder;

		pNewInfo->m_strOtherBtnLabel = pElement->m_strOtherButtonLabel;
		pNewInfo->m_strOtherBtnToolTip = pElement->m_strOtherButtonToolTip;

		pNewInfo->m_sizeIcon = pElement->GetColorBoxSize ();
		pNewInfo->m_bSimpleButtonLook = pElement->IsSimpleButtonLook ();
	}
	else if (element.IsKindOf (RUNTIME_CLASS (CBCGPRibbonPaletteButton)))
	{
		CBCGPRibbonInfo::XElementButtonPalette* pNewInfo = new CBCGPRibbonInfo::XElementButtonPalette;
		info = pNewInfo;

		CollectBaseElement (element, *info);

		const CBCGPRibbonPaletteButton* pElement = (const CBCGPRibbonPaletteButton*)&element;
		ASSERT_VALID (pElement);

		pNewInfo->m_bIsButtonMode = pElement->IsButtonMode ();
		pNewInfo->m_bEnableMenuResize = pElement->IsMenuResizeEnabled ();
		pNewInfo->m_bMenuResizeVertical = pElement->IsMenuResizeVertical ();
		pNewInfo->m_bDrawDisabledItems = pElement->IsDrawDisabledItems();
		pNewInfo->m_nIconsInRow = pElement->GetIconsInRow ();

		GetElementImages (*pElement, pNewInfo->m_Images);

		if (!pElement->IsOwnerDraw ())
		{
			if (pNewInfo->m_Images.m_Image.GetCount () > 0)
			{
				CSize sizeIcon(pNewInfo->m_Images.m_Image.GetImageSize ());
				if (pNewInfo->m_Images.m_Image.IsScaled ())
				{
					sizeIcon = pNewInfo->m_Images.m_Image.m_sizeImageOriginal;
				}
				pNewInfo->m_sizeIcon = sizeIcon;
			}

			int nCount = (int)pElement->m_arGroupLen.GetSize ();
			if (nCount > 0)
			{
				for (int i = 0; i < nCount - 1; i++)
				{
					CBCGPRibbonInfo::XElementButtonPalette::XPaletteGroup* pGroup = 
						new CBCGPRibbonInfo::XElementButtonPalette::XPaletteGroup;
					pGroup->m_strName = pElement->m_arGroupNames[i];
					pGroup->m_nItems  = pElement->m_arGroupLen[i + 1] - pElement->m_arGroupLen[i];

					pNewInfo->m_arGroups.Add (pGroup);
				}

				CBCGPRibbonInfo::XElementButtonPalette::XPaletteGroup* pGroup = 
					new CBCGPRibbonInfo::XElementButtonPalette::XPaletteGroup;
				pGroup->m_strName = pElement->m_arGroupNames[nCount - 1];
				pGroup->m_nItems  = pElement->m_nIcons - pElement->m_arGroupLen[nCount - 1];

				pNewInfo->m_arGroups.Add (pGroup);
			}
		}
	}
	else if (element.IsKindOf (RUNTIME_CLASS (CBCGPRibbonHyperlink)))
	{
		CBCGPRibbonInfo::XElementButtonHyperlink* pNewInfo = new CBCGPRibbonInfo::XElementButtonHyperlink;
		info = pNewInfo;

		CollectBaseElement (element, *info);

		const CBCGPRibbonHyperlink* pElement = (const CBCGPRibbonHyperlink*)&element;
		ASSERT_VALID (pElement);

		pNewInfo->m_strLink = pElement->GetLink ();
	}
	else if (element.IsKindOf (RUNTIME_CLASS (CBCGPRibbonRadioButton)))
	{
		CBCGPRibbonInfo::XElementButtonRadio* pNewInfo = new CBCGPRibbonInfo::XElementButtonRadio;
		info = pNewInfo;

		CollectBaseElement (element, *info);
	}
	else if (element.IsKindOf (RUNTIME_CLASS (CBCGPRibbonCheckBox)))
	{
		CBCGPRibbonInfo::XElementButtonCheck* pNewInfo = new CBCGPRibbonInfo::XElementButtonCheck;
		info = pNewInfo;

		CollectBaseElement (element, *info);
	}
	else if (element.IsKindOf (RUNTIME_CLASS (CBCGPRibbonStatusBarPane)))
	{
		CBCGPRibbonInfo::XElementStatusPane* pNewInfo = new CBCGPRibbonInfo::XElementStatusPane;
		info = pNewInfo;

		CollectBaseElement (element, *info);

		const CBCGPRibbonStatusBarPane* pElement = (const CBCGPRibbonStatusBarPane*)&element;
		ASSERT_VALID (pElement);

		pNewInfo->m_strAlmostLargeText = pElement->GetAlmostLargeText ();
		pNewInfo->m_bIsStatic = pElement->IsStatic ();
		pNewInfo->m_nTextAlign = pElement->GetTextAlign ();
	}
	else if (element.IsKindOf (RUNTIME_CLASS (CBCGPRibbonMainPanelButton)))
	{
		CBCGPRibbonInfo::XElementButtonMainPanel* pNewInfo = new CBCGPRibbonInfo::XElementButtonMainPanel;
		info = pNewInfo;

		CollectBaseElement (element, *info);
	}	
	else if (element.IsKindOf (RUNTIME_CLASS (CBCGPRibbonButton)))
	{
		const CBCGPRibbonButton* pElement = (const CBCGPRibbonButton*)&element;
		ASSERT_VALID (pElement);

		if (pElement->IsBackstageViewMode ())
		{
			CBCGPRibbonInfo::XElementButtonCommand* pNewInfo = new CBCGPRibbonInfo::XElementButtonCommand;
			info = pNewInfo;

			CollectBaseElement (element, *info, FALSE);

			pNewInfo->m_bIsMenu = pElement->GetSubItems ().GetSize () > 0;
		}
		else
		{
			CBCGPRibbonInfo::XElementButton* pNewInfo = new CBCGPRibbonInfo::XElementButton;
			info = pNewInfo;

			CollectBaseElement (element, *info);

			pNewInfo->m_bIsAlwaysShowDescription = pElement->IsAlwaysShowDescription ();
		}
	}
	else if (element.IsKindOf (RUNTIME_CLASS (CBCGPRibbonSlider)))
	{
		CBCGPRibbonInfo::XElementSlider* pNewInfo = new CBCGPRibbonInfo::XElementSlider;
		info = pNewInfo;

		CollectBaseElement (element, *info);

		const CBCGPRibbonSlider* pElement = (const CBCGPRibbonSlider*)&element;
		ASSERT_VALID (pElement);

		pNewInfo->m_dwStyle = pElement->GetStyle ();
		pNewInfo->m_nWidth = pElement->GetWidth ();
		pNewInfo->m_bZoomButtons = pElement->HasZoomButtons ();
		pNewInfo->m_nMin = pElement->GetRangeMin ();
		pNewInfo->m_nMax = pElement->GetRangeMax ();
		pNewInfo->m_nPos = pElement->GetPos ();
	}
	else if (element.IsKindOf (RUNTIME_CLASS (CBCGPRibbonProgressBar)))
	{
		CBCGPRibbonInfo::XElementProgressBar* pNewInfo = new CBCGPRibbonInfo::XElementProgressBar;
		info = pNewInfo;

		CollectBaseElement (element, *info);

		const CBCGPRibbonProgressBar* pElement = (const CBCGPRibbonProgressBar*)&element;
		ASSERT_VALID (pElement);

		pNewInfo->m_nWidth = pElement->GetWidth ();
		pNewInfo->m_nHeight = pElement->GetHeight ();
		pNewInfo->m_nMin = pElement->GetRangeMin ();
		pNewInfo->m_nMax = pElement->GetRangeMax ();
		pNewInfo->m_nPos = pElement->GetPos ();
		pNewInfo->m_bInfinite = pElement->IsInfiniteMode ();
		pNewInfo->m_bVertical = pElement->IsVertical();
	}
	else if (element.IsKindOf (RUNTIME_CLASS (CBCGPRibbonSeparator)))
	{
		CBCGPRibbonInfo::XElementSeparator* pNewInfo = new CBCGPRibbonInfo::XElementSeparator;
		info = pNewInfo;
		
		const CBCGPRibbonSeparator* pElement = (const CBCGPRibbonSeparator*)&element;
		ASSERT_VALID (pElement);

		pNewInfo->m_bIsHoriz = pElement->IsHorizontal ();
	}

	return info;
}

void CBCGPRibbonCollector::CollectElement(const CBCGPBaseRibbonElement& element, CBCGPRibbonInfo::XElement& info)
{
	if (info.GetElementName ().Compare (CBCGPRibbonInfo::s_szButton_Main) == 0 &&
		element.IsKindOf (RUNTIME_CLASS (CBCGPRibbonMainButton)))
	{
		CollectBaseElement (element, info);

		GetMainButtonImages((CBCGPRibbonMainButton&)element, (CBCGPRibbonInfo::XElementButtonMain&)info);
	}
	else if (info.GetElementName ().Compare (CBCGPRibbonInfo::s_szButton_Launch) == 0 &&
			 element.IsKindOf (RUNTIME_CLASS (CBCGPRibbonLaunchButton)))
	{
		CollectBaseElement (element, info);
	}
	else if (info.GetElementName ().Compare (CBCGPRibbonInfo::s_szGroup) == 0 &&
			 element.IsKindOf (RUNTIME_CLASS (CBCGPRibbonButtonsGroup)))
	{
		const CBCGPRibbonButtonsGroup* pElement = (const CBCGPRibbonButtonsGroup*)&element;
		CBCGPRibbonInfo::XElementGroup& infoElement = 
			(CBCGPRibbonInfo::XElementGroup&)info;

		int count = pElement->GetCount ();
		if (count > 0)
		{
			int i = 0;

			if ((GetFlags () & e_CollectGroupImages) == 0)
			{
				GetElementImages (*pElement, infoElement.m_Images);
			}

			for (i = 0; i < count; i++)
			{
				if (pElement->IsKindOf (RUNTIME_CLASS (CBCGPRibbonMinimizeButton)))
				{
					continue;
				}

				CBCGPRibbonInfo::XElement* pElementInfo = CollectElement (*pElement->GetButton (i));
				if (pElementInfo != NULL)
				{
					infoElement.m_arButtons.Add (pElementInfo);
				}
			}
		}
	}
	else
	{
		ASSERT (FALSE);
	}
}

void CBCGPRibbonCollector::CollectBaseElement (const CBCGPBaseRibbonElement& element, 
											   CBCGPRibbonInfo::XElement& info, 
											   BOOL bSubItems)
{
	info.m_strText = element.GetText ();

	GetID (element, info.m_ID);

	if ((GetFlags () & e_CollectUpdatedToolInfo) == e_CollectUpdatedToolInfo)
	{
		((CBCGPBaseRibbonElement&)element).UpdateTooltipInfo ();
	}
	info.m_strToolTip = element.GetToolTip ();
	info.m_strDescription = element.GetDescription ();
	info.m_strKeys = element.GetKeys ();
	info.m_strMenuKeys = element.GetMenuKeys ();

	const CBCGPRibbonButton* pButton = DYNAMIC_DOWNCAST (CBCGPRibbonButton, &element);
	if (pButton != NULL)
	{
		CBCGPRibbonInfo::XElementButton& infoButton = 
			(CBCGPRibbonInfo::XElementButton&)info;

		infoButton.m_nSmallImageIndex = pButton->GetImageIndex (FALSE);
		infoButton.m_nLargeImageIndex = pButton->GetImageIndex (TRUE);
		infoButton.m_bIsAlwaysLarge = pButton->CBCGPBaseRibbonElement::IsAlwaysLargeImage ();
		infoButton.m_bIsDefaultCommand = pButton->IsDefaultCommand ();
		infoButton.m_QATType = pButton->GetQATType ();

		if (bSubItems)
		{
			const CArray<CBCGPBaseRibbonElement*, CBCGPBaseRibbonElement*>& subAr = 
				pButton->GetSubItems ();

			int i = 0;
			int count = (int)subAr.GetSize ();

			for (i = 0; i < count; i++)
			{
				CBCGPBaseRibbonElement* pSubItem = (CBCGPBaseRibbonElement*)subAr[i];
				ASSERT_VALID (pSubItem);

				CBCGPRibbonInfo::XElement* pElementInfo = CollectElement (*pSubItem);
				if (pElementInfo != NULL)
				{
					pElementInfo->m_bIsOnPaletteTop = pSubItem->IsOnPaletteTop ();
					infoButton.m_arSubItems.Add (pElementInfo);
				}
			}
		}
	}
}

void CBCGPRibbonCollector::GetID (const CBCGPBaseRibbonElement& element, CBCGPRibbonInfo::XID& info)
{
	info.m_Value = element.GetID ();

	if (info.m_Value == -1 || info.m_Value == 0)
	{
		const CBCGPRibbonContextCaption* pCaption = DYNAMIC_DOWNCAST(CBCGPRibbonContextCaption, &element);
		if (pCaption != NULL)
		{
			info.m_Value = pCaption->GetContextID ();
		}
	}

	if (info.m_Value == -1)
	{
		info.m_Value = 0;
	}
}

void CBCGPRibbonCollector::GetRibbonBarImages(const CBCGPRibbonBar& bar, CBCGPRibbonInfo::XRibbonBar& info)
{
	CBCGPToolBarImages& images = info.m_Images.m_Image;

	if (bar.m_PanelIcons.IsValid() && bar.m_PanelIcons.GetCount() > 0)
	{
		const_cast<CBCGPToolBarImages&>(bar.m_PanelIcons).CopyTo(images);
	}

	if ((GetFlags () & e_CollectRibbonBarIcons) == 0)
	{
		return;
	}

	if (!images.IsValid())
	{
		images.SetImageSize (GetInfo ().GetImageSize (CBCGPRibbonInfo::e_ImagesSmall));
		images.SetTransparentColor ((COLORREF)-1);
		images.SetPreMultiplyAutoCheck ();
	}

	int nImageIndex = images.GetCount();

	CBCGPRibbonButtonsGroup& group = (CBCGPRibbonButtonsGroup&)(bar.m_TabElements);

	int count = group.GetCount ();
	int i = 0;

	for (i = 0; i < count; i++)
	{
		CBCGPRibbonButton* pButton = DYNAMIC_DOWNCAST(CBCGPRibbonButton, group.GetButton (i));
		if (pButton != NULL)
		{
			HICON hIcon = pButton->GetIcon ();
			if (hIcon != NULL)
			{
				if (BCGPRibbonCollector::AddIcon (images, hIcon))
				{
					((CBCGPRibbonInfo::XElementButton*)info.m_TabElements.m_arButtons[i])->m_nSmallImageIndex = nImageIndex;
					nImageIndex++;
				}
			}
		}
	}

	int index = info.m_bPrintPreview ? 1 : 0;
	int nCategory = 0;
	count = bar.GetCategoryCount ();

	for (i = index; i < count; i++)
	{
		CBCGPRibbonCategory* pCategory = bar.GetCategory (i);
		ASSERT_VALID(pCategory);

		if (pCategory->GetContextID () != 0)
		{
			continue;
		}

		for (int j = 0; j < pCategory->GetPanelCount (); j++)
		{
			CBCGPRibbonPanel* pPanel = pCategory->GetPanel (j);
			ASSERT_VALID(pPanel);

			HICON hIcon = pPanel->GetDefaultButton ().GetIcon (TRUE);
			if (hIcon != NULL)
			{
				if (BCGPRibbonCollector::AddIcon (images, hIcon))
				{
					info.m_arCategories[nCategory]->m_arPanels[j]->m_nImageIndex = nImageIndex;
					nImageIndex++;
				}
			}
		}

		nCategory++;
	}

	int nContext = -1;
	UINT nContextID = 0;
	nCategory = 0;
	for (i = index; i < count; i++)
	{
		CBCGPRibbonCategory* pCategory = bar.GetCategory (i);

		UINT nID = pCategory->GetContextID ();
		if (nID == 0)
		{
			continue;
		}

		if (nID != nContextID)
		{
			nContextID = nID;
			nContext++;
			nCategory = 0;
		}

		for (int j = 0; j < pCategory->GetPanelCount (); j++)
		{
			CBCGPRibbonPanel* pPanel = pCategory->GetPanel (j);
			ASSERT_VALID(pPanel);

			HICON hIcon = pPanel->GetDefaultButton ().GetIcon(FALSE);
			if (hIcon != NULL)
			{
				if (BCGPRibbonCollector::AddIcon (images, hIcon))
				{
					info.m_arContexts[nContext]->m_arCategories[nCategory]->m_arPanels[j]->m_nImageIndex = nImageIndex;
					nImageIndex++;
				}
			}
		}

		nCategory++;
	}
}

void CBCGPRibbonCollector::GetStatusBarImages(const CBCGPRibbonStatusBar& bar, CBCGPRibbonInfo::XStatusBar& info)
{
	if ((GetFlags () & e_CollectStatusBarIcons) == 0)
	{
		return;
	}

	CBCGPToolBarImages images;
	images.SetImageSize (GetInfo ().GetImageSize (CBCGPRibbonInfo::e_ImagesSmall));
	images.SetTransparentColor ((COLORREF)-1);
	images.SetPreMultiplyAutoCheck ();

	int nImageIndex = 0;
	int count = bar.GetCount ();

	for (int i = 0; i < count; i++)
	{
		CBCGPRibbonStatusBarPane* pPane = 
			DYNAMIC_DOWNCAST(CBCGPRibbonStatusBarPane, ((CBCGPRibbonStatusBar&)bar).GetElement (i));
		if (pPane == NULL)
		{
			continue;
		}

		HICON hIcon = pPane->GetIcon (TRUE);
		if (hIcon != NULL)
		{
			if (BCGPRibbonCollector::AddIcon (images, hIcon))
			{
				((CBCGPRibbonInfo::XElementStatusPane*)info.m_Elements.m_arElements[i])->m_nSmallImageIndex = nImageIndex;
				nImageIndex++;
			}
		}
	}

	if (images.IsValid () && images.GetCount () > 0)
	{
		images.CopyTo (info.m_Images.m_Image);
	}
}

void CBCGPRibbonCollector::GetCategoryImages(const CBCGPRibbonCategory& category, CBCGPRibbonInfo::XCategory& info)
{
	GetCategoryImages (category, info.m_SmallImages, info.m_LargeImages);

	if ((GetFlags () & e_CollectGroupImages) == 0)
	{
		return;
	}

	CBCGPRibbonCategory* pCategory = (CBCGPRibbonCategory*)&category;
	CBCGPToolBarImages& infoSmall = info.m_SmallImages.m_Image;

	if (!infoSmall.IsValid () || infoSmall.GetCount () == 0)
	{
		infoSmall.SetImageSize (GetInfo ().GetImageSize (CBCGPRibbonInfo::e_ImagesSmall));
		infoSmall.SetTransparentColor ((COLORREF)-1);
		infoSmall.SetPreMultiplyAutoCheck ();
	}

	int nImageIndex = infoSmall.GetCount ();
	for (int i = 0; i < pCategory->GetPanelCount (); i++)
	{
		CBCGPRibbonPanel* pPanel = pCategory->GetPanel (i);

		for (int j = 0; j < pPanel->GetCount (); j++)
		{
			CBCGPRibbonButtonsGroup* pGroup = DYNAMIC_DOWNCAST(CBCGPRibbonButtonsGroup, pPanel->GetElement (j));
			if (pGroup == NULL)
			{
				continue;
			}

			CBCGPToolBarImages& images = pGroup->GetImages ();
			if (!images.IsValid () || images.GetCount () == 0)
			{
				continue;
			}

			CBCGPRibbonInfo::XElementGroup* pElementGroup = 
				(CBCGPRibbonInfo::XElementGroup*)(info.m_arPanels[i]->m_arElements[j]);

			for (int k = 0; k < pGroup->GetCount (); k++)
			{
				CBCGPRibbonButton* pButton = DYNAMIC_DOWNCAST(CBCGPRibbonButton, pGroup->GetButton (k));
				if (pButton == NULL)
				{
					continue;
				}

				int nImage = pButton->GetImageIndex (FALSE);
				if (nImage == -1)
				{
					continue;
				}

				((CBCGPRibbonInfo::XElementButton*)pElementGroup->m_arButtons[k])->m_nSmallImageIndex = nImageIndex;

				infoSmall.AddImage (images, nImage);
				nImageIndex++;
			}
/*
			int nImageIndex = infoSmall.m_Image.GetCount ();
			for (int k = 0; k < images.GetCount (); k++)
			{
				infoSmall.m_Image.AddImage (images, k);
			}
*/
		}
	}

	if (infoSmall.GetCount () == 0)
	{
		infoSmall.Clear ();
	}
}

void CBCGPRibbonCollector::GetCategoryImages(const CBCGPRibbonCategory& category, CBCGPRibbonInfo::XImage& infoSmall, CBCGPRibbonInfo::XImage& infoLarge)
{
	CBCGPRibbonCategory* pCategory = (CBCGPRibbonCategory*)&category;

	pCategory->GetLargeImages ().CopyTo (infoLarge.m_Image);
	pCategory->GetSmallImages ().CopyTo (infoSmall.m_Image);
}

void CBCGPRibbonCollector::GetMainButtonImages(const CBCGPRibbonMainButton& element, CBCGPRibbonInfo::XElementButtonMain& info)
{
	CBCGPRibbonMainButton* pElement = (CBCGPRibbonMainButton*)&element;

	pElement->GetImage ().CopyTo (info.m_Image.m_Image);
	pElement->GetScenicImage ().CopyTo (info.m_ImageScenic.m_Image);
}

void CBCGPRibbonCollector::GetElementImages(const CBCGPBaseRibbonElement& element, CBCGPRibbonInfo::XImage& info)
{
	if (element.IsKindOf (RUNTIME_CLASS (CBCGPRibbonButtonsGroup)))
	{
		CBCGPRibbonButtonsGroup* pElement = (CBCGPRibbonButtonsGroup*)&element;

		pElement->GetImages ().CopyTo (info.m_Image);
	}
	else if (element.IsKindOf (RUNTIME_CLASS (CBCGPRibbonPaletteButton)))
	{
		CBCGPRibbonPaletteButton* pElement = (CBCGPRibbonPaletteButton*)&element;

		pElement->m_imagesPalette.CopyTo (info.m_Image);
	}
}


CBCGPRibbonCustomizationCollector::CBCGPRibbonCustomizationCollector(CBCGPRibbonCustomizationInfo& info, DWORD dwFlags)
	: m_Info    (info)
	, m_dwFlags (dwFlags)
{
}

CBCGPRibbonCustomizationCollector::~CBCGPRibbonCustomizationCollector()
{
}

void CBCGPRibbonCustomizationCollector::Collect(const CBCGPRibbonCustomizationData& data)
{
	GetInfo ().SetVersionStamp (data.m_VersionStamp);

	CollectDataCategory (data, GetInfo ().GetCategoryData ());
	CollectDataPanel (data, GetInfo ().GetPanelData ());
}

void CBCGPRibbonCustomizationCollector::CollectDataCategory (const CBCGPRibbonCustomizationData& data, CBCGPRibbonCustomizationInfo::XDataCategory& info)
{
	info.m_arHidden.Copy (data.m_arHiddenTabs);

	POSITION pos = 0;

	pos = data.m_mapTabIndexes.GetStartPosition ();
	while (pos != NULL)
	{
		int key = 0;
		int value = 0;

		data.m_mapTabIndexes.GetNextAssoc (pos, key, value);
		info.m_mapIndexes[key] = value;
	}

	pos = data.m_mapTabNames.GetStartPosition ();
	while (pos != NULL)
	{
		int key = 0;
		CString value;

		data.m_mapTabNames.GetNextAssoc (pos, key, value);
		info.m_mapNames[key] = value;
	}
	
	for (int i = 0; i < (int)data.m_arCustomTabs.GetSize (); i++)
	{
		if (data.m_arCustomTabs[i] != NULL)
		{
			CBCGPRibbonCustomizationInfo::XCustomCategory* pCategory = new CBCGPRibbonCustomizationInfo::XCustomCategory;
			CollectCustomCategory (*data.m_arCustomTabs[i], *pCategory);

			info.m_arCustom.Add (pCategory);
		}
	}
}

void CBCGPRibbonCustomizationCollector::CollectDataPanel (const CBCGPRibbonCustomizationData& data, CBCGPRibbonCustomizationInfo::XDataPanel& info)
{
	info.m_arHidden.Copy (data.m_arHiddenPanels);

	POSITION pos = 0;

	pos = data.m_mapPanelIndexes.GetStartPosition ();
	while (pos != NULL)
	{
		DWORD key = 0;
		int value = 0;

		data.m_mapPanelIndexes.GetNextAssoc (pos, key, value);
		info.m_mapIndexes[key] = value;
	}

	pos = data.m_mapPanelNames.GetStartPosition ();
	while (pos != NULL)
	{
		DWORD key = 0;
		CString value;

		data.m_mapPanelNames.GetNextAssoc (pos, key, value);
		info.m_mapNames[key] = value;
	}

	pos = data.m_mapCustomPanels.GetStartPosition ();
	while (pos != NULL)
	{
		DWORD key = 0;
		CBCGPRibbonCustomPanel* value = NULL;

		data.m_mapCustomPanels.GetNextAssoc (pos, key, value);
		if (value != NULL)
		{
			CBCGPRibbonCustomizationInfo::XCustomPanel* pPanel = new CBCGPRibbonCustomizationInfo::XCustomPanel;
			CollectCustomPanel (*value, *pPanel);

			info.m_mapCustom[key] = pPanel;
		}
	}
}

void CBCGPRibbonCustomizationCollector::CollectCustomCategory (const CBCGPRibbonCustomCategory& category, CBCGPRibbonCustomizationInfo::XCustomCategory& info)
{
	info.m_nKey = category.m_nKey;
	info.m_strName = category.m_strName;
	info.m_uiContextID = category.m_uiContextID;

	for (int i = 0; i < (int)category.m_arPanels.GetSize (); i++)
	{
		if (category.m_arPanels[i] != NULL)
		{
			CBCGPRibbonCustomizationInfo::XCustomPanel* pPanel = new CBCGPRibbonCustomizationInfo::XCustomPanel;
			CollectCustomPanel (*category.m_arPanels[i], *pPanel);

			info.m_arPanels.Add (pPanel);
		}
	}
}

void CBCGPRibbonCustomizationCollector::CollectCustomPanel (const CBCGPRibbonCustomPanel& panel, CBCGPRibbonCustomizationInfo::XCustomPanel& info)
{
	info.m_nID = panel.m_nID;
	info.m_nKey = panel.m_nKey;
	info.m_strName = panel.m_strName;
	info.m_nIndex = panel.m_nIndex;
	info.m_dwOriginal = panel.m_dwOriginal;

	if ((int)info.m_dwOriginal <= 0)
	{
		for (int i = 0; i < (int)panel.m_arElements.GetSize (); i++)
		{
			const CBCGPRibbonCustomElement* element = panel.m_arElements[i];
			if (element != NULL)
			{
				CBCGPRibbonCustomizationInfo::XCustomElement* pElement = new CBCGPRibbonCustomizationInfo::XCustomElement;
				pElement->m_nID = element->m_nID;
				pElement->m_strName = element->m_strName;
				pElement->m_nCustomIcon = element->m_nCustomIcon;

				info.m_arElements.Add (pElement);
			}
		}
	}
}

#endif // BCGP_EXCLUDE_RIBBON
