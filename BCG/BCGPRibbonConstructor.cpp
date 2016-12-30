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
// BCGPRibbonConstructor.cpp: implementation of the CBCGPRibbonConstrucor class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BCGPRibbonConstructor.h"
#include "BCGPRibbonStatusBar.h"
#include "BCGPRibbonCategory.h"
#include "BCGPRibbonComboBox.h"
#include "BCGPRibbonPaletteButton.h"
#include "BCGPRibbonLabel.h"
#include "BCGPRibbonUndoButton.h"
#include "BCGPRibbonColorButton.h"
#include "BCGPRibbonHyperlink.h"
#include "BCGPRibbonCheckBox.h"
#include "BCGPRibbonStatusBarPane.h"
#include "BCGPRibbonSlider.h"
#include "BCGPRibbonProgressBar.h"
#include "BCGPRibbonMainPanel.h"
#include "BCGPRibbonBackstageViewPanel.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#ifndef BCGP_EXCLUDE_RIBBON

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGPRibbonConstructor::CBCGPRibbonConstructor(const CBCGPRibbonInfo& info)
	: m_Info (info)
{
}

CBCGPRibbonConstructor::~CBCGPRibbonConstructor()
{
}

CBCGPRibbonPanel* CBCGPRibbonConstructor::CreatePanel (CBCGPRibbonCategory& category, const CBCGPRibbonInfo::XPanel& info) const
{
	CBCGPRibbonBar* pBar = category.GetParentRibbonBar ();
	if (pBar != NULL)
	{
		return category.AddPanel (info.m_strName, pBar->m_PanelIcons, info.m_nImageIndex);
	}

	return category.AddPanel (info.m_strName, (HICON)NULL);
}

CBCGPRibbonCategory* CBCGPRibbonConstructor::CreateCategory (CBCGPRibbonBar& bar, const CBCGPRibbonInfo::XCategory& info) const
{
	return bar.AddCategory (info.m_strName, 0, 0, 
							GetInfo ().GetImageSize (CBCGPRibbonInfo::e_ImagesSmall), 
							GetInfo ().GetImageSize (CBCGPRibbonInfo::e_ImagesLarge));
}

CBCGPRibbonCategory* CBCGPRibbonConstructor::CreateCategoryContext (CBCGPRibbonBar& bar, const CBCGPRibbonInfo::XContext& infoContext, const CBCGPRibbonInfo::XCategory& info) const
{
	return bar.AddContextCategory (info.m_strName, infoContext.m_strText, 
							infoContext.m_ID.m_Value, infoContext.m_Color, 0, 0, 
							GetInfo ().GetImageSize (CBCGPRibbonInfo::e_ImagesSmall), 
							GetInfo ().GetImageSize (CBCGPRibbonInfo::e_ImagesLarge));
}

CBCGPRibbonMainPanel* CBCGPRibbonConstructor::CreateCategoryMain (CBCGPRibbonBar& bar, const CBCGPRibbonInfo::XCategoryMain& info) const
{
	return bar.AddMainCategory (info.m_strName, 0, 0, 
							GetInfo ().GetImageSize (CBCGPRibbonInfo::e_ImagesSmall), 
							GetInfo ().GetImageSize (CBCGPRibbonInfo::e_ImagesLarge));
}

CBCGPRibbonBackstageViewPanel* CBCGPRibbonConstructor::CreateCategoryBackstage (CBCGPRibbonBar& bar, const CBCGPRibbonInfo::XCategoryBackstage& info) const
{
	return bar.AddBackstageCategory (info.m_strName, 0, 
							GetInfo ().GetImageSize (CBCGPRibbonInfo::e_ImagesSmall));
}

CBCGPRibbonMainButton* CBCGPRibbonConstructor::CreateMainButton (CBCGPRibbonBar& bar) const
{
	bar.m_bAutoDestroyMainButton = TRUE;
	bar.SetMainButton (new CBCGPRibbonMainButton, CSize (45, 45));

	return bar.GetMainButton ();
}

void CBCGPRibbonConstructor::ConstructRibbonBar (CBCGPRibbonBar& bar) const
{
	const CBCGPRibbonInfo::XRibbonBar& infoBar = GetInfo ().GetRibbonBar ();

	CBCGPRibbonPanel::m_nNextPanelID = (UINT)-10;

	bar.m_VersionStamp = GetInfo ().GetVersionStamp ();
	bar.EnableToolTips     (infoBar.m_bToolTip, infoBar.m_bToolTipDescr);
	bar.EnableKeyTips      (infoBar.m_bKeyTips);
	bar.EnablePrintPreview (infoBar.m_bPrintPreview);
	bar.SetBackstageMode   (infoBar.m_bBackstageMode);

	CBCGPRibbonFontComboBox::m_bDrawUsingFont = infoBar.m_bDrawUsingFont;

	const_cast<CBCGPToolBarImages&>(infoBar.m_Images.m_Image).CopyTo (bar.m_PanelIcons);

	if (infoBar.m_btnMain != NULL)
	{
		CBCGPRibbonMainButton* btnMain = bar.GetMainButton ();
		if (btnMain == NULL)
		{
			btnMain = CreateMainButton (bar);
		}

		if (btnMain != NULL)
		{
			ConstructElement (*btnMain, *infoBar.m_btnMain);
		}
	}

	if (infoBar.m_MainCategory != NULL)
	{
		ConstructCategoryMain (bar, *infoBar.m_MainCategory);
	}

	if (infoBar.m_BackstageCategory != NULL)
	{
		ConstructCategoryBackstage (bar, *infoBar.m_BackstageCategory);
	}

	ConstructTabElements (bar, infoBar);

	int i = 0;
	for (i = 0; i < infoBar.m_arCategories.GetSize (); i++)
	{
		const CBCGPRibbonInfo::XCategory& infoCategory = 
			*(const CBCGPRibbonInfo::XCategory*)infoBar.m_arCategories[i];

		CBCGPRibbonCategory* pCategory = CreateCategory (bar, infoCategory);
		if (pCategory != NULL)
		{
			ASSERT_VALID (pCategory);
			ConstructCategory (*pCategory, infoCategory);
		}
	}

	for (i = 0; i < infoBar.m_arContexts.GetSize (); i++)
	{
		const CBCGPRibbonInfo::XContext* context = infoBar.m_arContexts[i];
		for (int j = 0; j < context->m_arCategories.GetSize (); j++)
		{
			const CBCGPRibbonInfo::XCategory& infoCategory = 
				*(const CBCGPRibbonInfo::XCategory*)context->m_arCategories[j];

			CBCGPRibbonCategory* pCategory = CreateCategoryContext (bar, *context, infoCategory);
			if (pCategory != NULL)
			{
				ASSERT_VALID (pCategory);
				ConstructCategory (*pCategory, infoCategory);
			}
		}
	}

	ConstructQATElements (bar, infoBar);
}

void CBCGPRibbonConstructor::ConstructStatusBar (CBCGPRibbonStatusBar& bar) const
{
	const CBCGPRibbonInfo::XStatusBar& infoBar = GetInfo ().GetStatusBar ();
	CBCGPToolBarImages& images = const_cast<CBCGPToolBarImages&>(infoBar.m_Images.m_Image);

	int i = 0;
	int count = 0;

	const CBCGPRibbonInfo::XStatusBar::XStatusElements& infoElements = infoBar.m_Elements;

	count = (int)infoElements.m_arElements.GetSize ();
	for (i = 0; i < count; i++)
	{
		CBCGPRibbonInfo::XElement& info = 
			(CBCGPRibbonInfo::XElement&)*infoElements.m_arElements[i];
		CBCGPBaseRibbonElement* pElement = CreateElement (info);
		if (pElement != NULL)
		{
			ASSERT_VALID (pElement);

			CBCGPRibbonSeparator* pSeparator = DYNAMIC_DOWNCAST (CBCGPRibbonSeparator, pElement);
			if (pSeparator)
			{
				bar.AddSeparator ();
				delete pSeparator;
			}
			else
			{
				CBCGPRibbonButton* pButton = DYNAMIC_DOWNCAST (CBCGPRibbonButton, pElement);
				if (pButton != NULL && pButton->GetImageIndex (FALSE) != -1)
				{
					SetIcon (*pButton, CBCGPBaseRibbonElement::RibbonImageLarge, images, FALSE);
				}

				bar.AddElement (pElement, infoElements.m_arLabels[i], infoElements.m_arVisible[i]);
			}
		}
	}

	const CBCGPRibbonInfo::XStatusBar::XStatusElements& infoExElements = infoBar.m_ExElements;

	count = (int)infoExElements.m_arElements.GetSize ();
	for (i = 0; i < count; i++)
	{
		CBCGPRibbonInfo::XElement& info = 
			(CBCGPRibbonInfo::XElement&)*infoExElements.m_arElements[i];
		CBCGPBaseRibbonElement* pElement = CreateElement (info);
		if (pElement != NULL)
		{
			ASSERT_VALID (pElement);

			CBCGPRibbonButton* pButton = (CBCGPRibbonButton*) DYNAMIC_DOWNCAST (CBCGPRibbonButton, pElement);
			if (pButton != NULL && pButton->GetImageIndex (FALSE) != -1)
			{
				SetIcon (*pButton, CBCGPBaseRibbonElement::RibbonImageLarge, images, FALSE);
			}

			bar.AddExtendedElement(pElement, infoExElements.m_arLabels[i], infoExElements.m_arVisible[i]);
		}
	}
}

void CBCGPRibbonConstructor::ConstructCategoryMain (CBCGPRibbonBar& bar, const CBCGPRibbonInfo::XCategoryMain& info) const
{
	CBCGPRibbonMainPanel* pPanel = CreateCategoryMain (bar, info);
	ASSERT_VALID (pPanel);

	CBCGPRibbonCategory* pCategory = bar.GetMainCategory ();
	ASSERT_VALID (pCategory);

	const_cast<CBCGPToolBarImages&>(info.m_SmallImages.m_Image).CopyTo (pCategory->GetSmallImages ());
	const_cast<CBCGPToolBarImages&>(info.m_LargeImages.m_Image).CopyTo (pCategory->GetLargeImages ());

	if (info.m_bSearchEnable)
	{
		pPanel->EnableCommandSearch (info.m_bSearchEnable, info.m_strSearchLabel, info.m_strSearchKeys, info.m_nSearchWidth);
	}

	int i = 0;
	for (i = 0; i < info.m_arElements.GetSize (); i++)
	{
		CBCGPBaseRibbonElement* pElement = 
			CreateElement (*(const CBCGPRibbonInfo::XElement*)info.m_arElements[i]);

		if (pElement != NULL)
		{
			ASSERT_VALID (pElement);

			if (info.m_arElements[i]->GetElementName ().Compare (CBCGPRibbonInfo::s_szButton_MainPanel) == 0)
			{
				pPanel->AddToBottom ((CBCGPRibbonMainPanelButton*)pElement);
			}
			else
			{
				pPanel->Add (pElement);
			}
		}
	}

	if (info.m_bRecentListEnable)
	{
		pPanel->AddRecentFilesList (info.m_strRecentListLabel, info.m_nRecentListWidth, info.m_bRecentListShowPins);
	}
}

void CBCGPRibbonConstructor::ConstructCategoryBackstage (CBCGPRibbonBar& bar, const CBCGPRibbonInfo::XCategoryBackstage& info) const
{
	CBCGPRibbonBackstageViewPanel* pPanel = CreateCategoryBackstage (bar, info);
	ASSERT_VALID (pPanel);

	CBCGPRibbonCategory* pCategory = bar.GetBackstageCategory ();
	ASSERT_VALID (pCategory);

	const_cast<CBCGPToolBarImages&>(info.m_SmallImages.m_Image).CopyTo (pCategory->GetSmallImages ());

	int i = 0;
	for (i = 0; i < info.m_arElements.GetSize (); i++)
	{
		if (info.m_arElements[i]->GetElementName ().Compare (CBCGPRibbonInfo::s_szButton_Command) == 0)
		{
			CBCGPBaseRibbonElement* pElement = 
				CreateElement (*(const CBCGPRibbonInfo::XElement*)info.m_arElements[i]);

			if (pElement != NULL)
			{
				ASSERT_VALID (pElement);

				pElement->SetBackstageViewMode ();
				pPanel->CBCGPRibbonMainPanel::Add (pElement);
			}
		}
	}
}

void CBCGPRibbonConstructor::ConstructCategory (CBCGPRibbonCategory& category, const CBCGPRibbonInfo::XCategory& info) const
{
	const_cast<CBCGPToolBarImages&>(info.m_SmallImages.m_Image).CopyTo (category.GetSmallImages ());
	const_cast<CBCGPToolBarImages&>(info.m_LargeImages.m_Image).CopyTo (category.GetLargeImages ());

	category.SetKeys (info.m_strKeys);

	int i = 0;
	for (i = 0; i < info.m_arPanels.GetSize (); i++)
	{
		const CBCGPRibbonInfo::XPanel& infoPanel = 
			*(const CBCGPRibbonInfo::XPanel*)info.m_arPanels[i];

		CBCGPRibbonPanel* pPanel = CreatePanel (category, infoPanel);
		if (pPanel != NULL)
		{
			ASSERT_VALID (pPanel);
			ConstructPanel (*pPanel, infoPanel);
		}
	}

	for (i = 0; i < info.m_arElements.GetSize (); i++)
	{
		CBCGPBaseRibbonElement* pElement = 
			CreateElement (*(const CBCGPRibbonInfo::XElement*)info.m_arElements[i]);

		if (pElement != NULL)
		{
			ASSERT_VALID (pElement);
			category.AddHidden (pElement);
		}
	}

	category.SetCollapseOrder (info.m_arCollapseOrder);
}

void CBCGPRibbonConstructor::ConstructPanel (CBCGPRibbonPanel& panel, const CBCGPRibbonInfo::XPanel& info) const
{
	panel.SetKeys (info.m_strKeys);
	panel.SetJustifyColumns (info.m_bJustifyColumns);
	panel.SetCenterColumnVert (info.m_bCenterColumnVert);

	ConstructElement (panel.GetLaunchButton (), info.m_btnLaunch);

	int i = 0;
	for (i = 0; i < info.m_arElements.GetSize (); i++)
	{
		CBCGPBaseRibbonElement* pElement = 
			CreateElement (*(const CBCGPRibbonInfo::XElement*)info.m_arElements[i]);

		if (pElement != NULL)
		{
			ASSERT_VALID (pElement);

			CBCGPRibbonSeparator* pSeparator = DYNAMIC_DOWNCAST (CBCGPRibbonSeparator, pElement);
			if (pSeparator)
			{
				panel.AddSeparator ();
				delete pSeparator;
			}
			else
			{
				panel.Add (pElement);
			}
		}
	}
}

void CBCGPRibbonConstructor::ConstructQATElements (CBCGPRibbonBar& bar, const CBCGPRibbonInfo::XRibbonBar& info) const
{
	const CBCGPRibbonInfo::XQAT::XArrayQATItem& items = info.m_QAT.m_arItems;

	int count = (int)items.GetSize ();
	if (count == 0)
	{
		return;
	}

	CBCGPRibbonQATDefaultState qatState;

	for (int i = 0; i < count; i++)
	{
		qatState.AddCommand (items[i].m_ID.m_Value, items[i].m_bVisible);
	}

	bar.SetQuickAccessDefaultState (qatState);
	bar.SetQuickAccessToolbarOnTop (info.m_QAT.m_bOnTop);
}

void CBCGPRibbonConstructor::ConstructTabElements (CBCGPRibbonBar& bar, const CBCGPRibbonInfo::XRibbonBar& info) const
{
	int i = 0;
	for (i = 0; i < info.m_TabElements.m_arButtons.GetSize (); i++)
	{
		CBCGPBaseRibbonElement* pElement = 
			CreateElement (*(const CBCGPRibbonInfo::XElement*)info.m_TabElements.m_arButtons[i]);
		if (pElement != NULL)
		{
			CBCGPRibbonButton* pButton = DYNAMIC_DOWNCAST (CBCGPRibbonButton, pElement);
			if (pButton != NULL && pButton->GetImageIndex (FALSE) != -1)
			{
				SetIcon (*pButton, CBCGPBaseRibbonElement::RibbonImageLarge, 
					GetInfo().GetRibbonBar ().m_Images.m_Image, FALSE);
			}

			ASSERT_VALID (pElement);
			bar.AddToTabs (pElement);
		}
	}
}

void CBCGPRibbonConstructor::ConstructElement (CBCGPBaseRibbonElement& element, const CBCGPRibbonInfo::XElement& info) const
{
	if (info.GetElementName ().Compare (CBCGPRibbonInfo::s_szButton_Main) == 0 &&
		element.IsKindOf (RUNTIME_CLASS (CBCGPRibbonMainButton)))
	{
		ConstructBaseElement (element, info);

		const CBCGPRibbonInfo::XElementButtonMain& infoElement = 
			(const CBCGPRibbonInfo::XElementButtonMain&)info;

		CBCGPRibbonMainButton* pElement = (CBCGPRibbonMainButton*)&element;
		ASSERT_VALID (pElement);

		const_cast<CBCGPToolBarImages&>(infoElement.m_Image.m_Image).CopyTo (pElement->m_Image);
		const_cast<CBCGPToolBarImages&>(infoElement.m_ImageScenic.m_Image).CopyTo (pElement->m_ImageScenic);
	}
	else if (info.GetElementName ().Compare (CBCGPRibbonInfo::s_szButton_Launch) == 0 &&
			 element.IsKindOf (RUNTIME_CLASS (CBCGPRibbonLaunchButton)))
	{
		ConstructBaseElement (element, info);
	}
	else if (info.GetElementName ().Compare (CBCGPRibbonInfo::s_szGroup) == 0 &&
			 element.IsKindOf (RUNTIME_CLASS (CBCGPRibbonButtonsGroup)))
	{
		const CBCGPRibbonInfo::XElementGroup& infoElement = 
			(const CBCGPRibbonInfo::XElementGroup&)info;

		CBCGPRibbonButtonsGroup* pElement = (CBCGPRibbonButtonsGroup*)&element;
		ASSERT_VALID (pElement);

		const_cast<CBCGPToolBarImages&>(infoElement.m_Images.m_Image).CopyTo (pElement->m_Images);

		for (int i = 0; i < infoElement.m_arButtons.GetSize (); i++)
		{
			CBCGPBaseRibbonElement* pButton = CreateElement 
				(*(const CBCGPRibbonInfo::XElement*)infoElement.m_arButtons[i]);

			if (pButton != NULL)
			{
				ASSERT_VALID (pButton);
				pElement->AddButton (pButton);
			}
		}
	}
	else
	{
		ASSERT (FALSE);
	}
}

void CBCGPRibbonConstructor::SetID (CBCGPBaseRibbonElement& element, const CBCGPRibbonInfo::XID& info) const
{
	element.SetID (info.m_Value);
}

void CBCGPRibbonConstructor::SetIcon (CBCGPRibbonButton& element, 
									  CBCGPBaseRibbonElement::RibbonImageType type,
									  const CBCGPToolBarImages& images, 
									  BOOL bLargeIcon/* = FALSE*/) const
{
	CBCGPRibbonButton* pButton = (CBCGPRibbonButton*)&element;

	HICON* pIcon = &pButton->m_hIconSmall;
	if (type == CBCGPBaseRibbonElement::RibbonImageLarge)
	{
		pIcon = &pButton->m_hIcon;
	}

	if (*pIcon != NULL && pButton->m_bAutoDestroyIcon)
	{
		::DestroyIcon (*pIcon);
		*pIcon = NULL;
	}

	*pIcon = const_cast<CBCGPToolBarImages&>(images).ExtractIcon (pButton->GetImageIndex (bLargeIcon));
	pButton->m_bAutoDestroyIcon = TRUE;
	pButton->m_bAlphaBlendIcon  = TRUE;

	pButton->SetImageIndex (-1, bLargeIcon);
}

void CBCGPRibbonConstructor::ConstructBaseElement (CBCGPBaseRibbonElement& element, 
												  const CBCGPRibbonInfo::XElement& info,
												  BOOL bSubItems) const
{
	element.SetText (info.m_strText);
	element.SetToolTipText (info.m_strToolTip);
	element.SetDescription (info.m_strDescription);
	element.SetKeys (info.m_strKeys, info.m_strMenuKeys);

	SetID (element, info.m_ID);

	CBCGPRibbonButton* pButton = DYNAMIC_DOWNCAST (CBCGPRibbonButton, &element);
	if (pButton != NULL)
	{
		const CBCGPRibbonInfo::XElementButton& infoElement = 
			(const CBCGPRibbonInfo::XElementButton&)info;

		if (pButton->GetIcon (FALSE) == NULL && pButton->GetIcon (TRUE) == NULL)
		{
			pButton->SetImageIndex (infoElement.m_nSmallImageIndex, FALSE);
			pButton->SetImageIndex (infoElement.m_nLargeImageIndex, TRUE);
		}
		pButton->SetAlwaysLargeImage (info.m_bIsAlwaysLarge);
		pButton->SetDefaultCommand (infoElement.m_bIsDefaultCommand);
		pButton->SetQATType (infoElement.m_QATType);

		if (bSubItems)
		{
			CBCGPRibbonPaletteButton* pButtonPalette = 
				DYNAMIC_DOWNCAST (CBCGPRibbonPaletteButton, pButton);

			for (int i = 0; i < infoElement.m_arSubItems.GetSize (); i++)
			{
				CBCGPBaseRibbonElement* pSubItem = 
					CreateElement (*(const CBCGPRibbonInfo::XElement*)infoElement.m_arSubItems[i]);
				if (pSubItem == NULL)
				{
					continue;
				}

				if (pButtonPalette != NULL)
				{
					pButtonPalette->AddSubItem (pSubItem, -1, infoElement.m_bIsOnPaletteTop);
				}
				else
				{
					pButton->AddSubItem (pSubItem);

					if (pSubItem->GetID() >= AFX_IDM_WINDOW_FIRST && pSubItem->GetID() <= AFX_IDM_WINDOW_LAST)
					{
						pButton->m_bIsWindowsMenu = TRUE;
					}
				}
			}
		}
	}
}

CBCGPBaseRibbonElement* CBCGPRibbonConstructor::CreateElement (const CBCGPRibbonInfo::XElement& info) const
{
	CBCGPBaseRibbonElement* pElement = NULL;

	int i = 0;

	if (info.GetElementName ().Compare (CBCGPRibbonInfo::s_szButton_Main) == 0)
	{
		const CBCGPRibbonInfo::XElementButtonMain& infoElement = 
			(const CBCGPRibbonInfo::XElementButtonMain&)info;

		CBCGPRibbonMainButton* pNewElement = new CBCGPRibbonMainButton;

		if (infoElement.m_Image.m_Image.GetImageWell () != NULL)
		{
			pNewElement->SetImage (infoElement.m_Image.m_Image.GetImageWell ());
		}
		if (infoElement.m_ImageScenic.m_Image.GetImageWell () != NULL)
		{
			pNewElement->SetScenicImage (infoElement.m_ImageScenic.m_Image.GetImageWell ());
		}

		pElement = pNewElement;

		ConstructElement (*pElement, info);
	}
	else if (info.GetElementName ().Compare (CBCGPRibbonInfo::s_szButton_Launch) == 0)
	{
		CBCGPRibbonLaunchButton* pNewElement = new CBCGPRibbonLaunchButton;
		pElement = pNewElement;

		ConstructElement (*pElement, info);
	}
	else if (info.GetElementName ().Compare (CBCGPRibbonInfo::s_szGroup) == 0)
	{
		CBCGPRibbonButtonsGroup* pNewElement = new CBCGPRibbonButtonsGroup;
		pElement = pNewElement;

		ConstructElement (*pElement, info);
	}
	else if (info.GetElementName ().Compare (CBCGPRibbonInfo::s_szLabel) == 0)
	{
		const CBCGPRibbonInfo::XElementLabel& infoElement = 
			(const CBCGPRibbonInfo::XElementLabel&)info;

		CBCGPRibbonLabel* pNewElement = 
			new CBCGPRibbonLabel (infoElement.m_strText, infoElement.m_bIsAlwaysLarge);
		pElement = pNewElement;
	}
	else if (info.GetElementName ().Compare (CBCGPRibbonInfo::s_szComboBox_Font) == 0)
	{
		const CBCGPRibbonInfo::XElementFontComboBox& infoElement = 
			(const CBCGPRibbonInfo::XElementFontComboBox&)info;

		CBCGPRibbonFontComboBox* pNewElement = 
			new CBCGPRibbonFontComboBox (infoElement.m_ID.m_Value, 
										infoElement.m_nFontType, 
										infoElement.m_nCharSet, 
										infoElement.m_nPitchAndFamily,
										infoElement.m_nWidth);
		pElement = pNewElement;

		ConstructBaseElement (*pElement, info);

		if (infoElement.m_nWidthFloaty > 0)
		{
			pNewElement->SetWidth (infoElement.m_nWidthFloaty, TRUE);
		}

		pNewElement->SetTextAlign (infoElement.m_nTextAlign);
		pNewElement->SetPrompt(infoElement.m_strSearchPrompt);

		((CBCGPRibbonFontComboBox*)pNewElement)->m_bHasEditBox = infoElement.m_bHasEditBox;
		pNewElement->EnableDropDownListResize (infoElement.m_bResizeDropDownList);

		if (infoElement.m_bHasEditBox)
		{
			pNewElement->EnableAutoComplete (infoElement.m_bAutoComplete);
		}
	}
	else if (info.GetElementName ().Compare (CBCGPRibbonInfo::s_szComboBox) == 0)
	{
		const CBCGPRibbonInfo::XElementComboBox& infoElement = 
			(const CBCGPRibbonInfo::XElementComboBox&)info;

		CBCGPRibbonComboBox* pNewElement = 
			new CBCGPRibbonComboBox (infoElement.m_ID.m_Value, 
									infoElement.m_bCalculatorMode || infoElement.m_bHasEditBox,
									infoElement.m_nWidth,
									infoElement.m_strText,
									infoElement.m_nSmallImageIndex,
									infoElement.m_sortOrder);
		pElement = pNewElement;

		ConstructBaseElement (*pElement, info);

		if (infoElement.m_nWidthFloaty > 0)
		{
			pNewElement->SetWidth (infoElement.m_nWidthFloaty, TRUE);
		}

		pNewElement->SetTextAlign (infoElement.m_nTextAlign);
		pNewElement->SetPrompt(infoElement.m_strSearchPrompt);

		if (infoElement.m_bCalculatorMode)
		{
			pNewElement->EnableCalculator (TRUE, NULL, &infoElement.m_lstCalculatorExt);
		}
		else
		{
			pNewElement->EnableDropDownListResize (infoElement.m_bResizeDropDownList);
			if (infoElement.m_bHasEditBox)
			{
				pNewElement->EnableAutoComplete (infoElement.m_bAutoComplete);
			}

			if (infoElement.m_bSearchMode)
			{
				pNewElement->EnableSearchMode (TRUE, infoElement.m_strSearchPrompt);
			}

			for (i = 0; i < infoElement.m_arItems.GetSize (); i++)
			{
				pNewElement->AddItem (infoElement.m_arItems[i]);
			}
		}
	}
	else if (info.GetElementName ().Compare (CBCGPRibbonInfo::s_szEdit) == 0)
	{
		const CBCGPRibbonInfo::XElementEdit& infoElement = 
			(const CBCGPRibbonInfo::XElementEdit&)info;

		CBCGPRibbonEdit* pNewElement = 
			new CBCGPRibbonEdit (infoElement.m_ID.m_Value, 
								infoElement.m_nWidth,
								infoElement.m_strText,
								infoElement.m_nSmallImageIndex);
		pElement = pNewElement;

		ConstructBaseElement (*pElement, info);

		if (infoElement.m_nWidthFloaty > 0)
		{
			pNewElement->SetWidth (infoElement.m_nWidthFloaty, TRUE);
		}

		pNewElement->SetTextAlign (infoElement.m_nTextAlign);
		pNewElement->SetPrompt(infoElement.m_strSearchPrompt);

		CString strValue (infoElement.m_strValue);
		if (infoElement.m_bSearchMode)
		{
			pNewElement->EnableSearchMode (TRUE, infoElement.m_strSearchPrompt);
		}
		else if (infoElement.m_bHasSpinButtons)
		{
			pNewElement->EnableSpinButtons (infoElement.m_nMin, infoElement.m_nMax);
			if (strValue.IsEmpty ())
			{
				strValue.Format (_T("%d"), infoElement.m_nMin);
			}
		}

		BOOL bDontNotify = pNewElement->m_bDontNotify;
		pNewElement->m_bDontNotify = TRUE;
		pNewElement->SetEditText (strValue);
		pNewElement->m_bDontNotify = bDontNotify;
	}
	else if (info.GetElementName ().Compare (CBCGPRibbonInfo::s_szButton_Undo) == 0)
	{
		const CBCGPRibbonInfo::XElementButtonUndo& infoElement = 
			(const CBCGPRibbonInfo::XElementButtonUndo&)info;

		CBCGPRibbonUndoButton* pNewElement = 
			new CBCGPRibbonUndoButton (infoElement.m_ID.m_Value,
										infoElement.m_strText,
										infoElement.m_nSmallImageIndex,
										infoElement.m_nLargeImageIndex);
		pElement = pNewElement;

		ConstructBaseElement (*pElement, info, FALSE);

		pNewElement->SetButtonMode (infoElement.m_bIsButtonMode);
		pNewElement->EnableMenuResize (infoElement.m_bEnableMenuResize, infoElement.m_bMenuResizeVertical);
		pNewElement->SetDrawDisabledItems(infoElement.m_bDrawDisabledItems);
	}
	else if (info.GetElementName ().Compare (CBCGPRibbonInfo::s_szButton_Color) == 0)
	{
		const CBCGPRibbonInfo::XElementButtonColor& infoElement = 
			(const CBCGPRibbonInfo::XElementButtonColor&)info;

		CBCGPRibbonColorButton* pNewElement = 
			new CBCGPRibbonColorButton (infoElement.m_ID.m_Value,
										infoElement.m_strText,
										infoElement.m_bSimpleButtonLook,
										infoElement.m_nSmallImageIndex,
										infoElement.m_nLargeImageIndex,
										infoElement.m_clrColor);
		pElement = pNewElement;

		ConstructBaseElement (*pElement, info);

		pNewElement->EnableAutomaticButton (infoElement.m_strAutomaticBtnLabel.IsEmpty () ? NULL : (LPCTSTR)infoElement.m_strAutomaticBtnLabel, 
			infoElement.m_clrAutomaticBtnColor, !infoElement.m_strAutomaticBtnLabel.IsEmpty (), infoElement.m_strAutomaticBtnToolTip, 
			infoElement.m_bAutomaticBtnOnTop, infoElement.m_bAutomaticBtnBorder);

		pNewElement->EnableOtherButton (infoElement.m_strOtherBtnLabel.IsEmpty () ? NULL : (LPCTSTR)infoElement.m_strOtherBtnLabel, 
										infoElement.m_strOtherBtnToolTip);

		pNewElement->SetColorBoxSize (infoElement.m_sizeIcon);
		pNewElement->SetButtonMode (infoElement.m_bIsButtonMode);
		pNewElement->EnableMenuResize (infoElement.m_bEnableMenuResize, infoElement.m_bMenuResizeVertical);
		pNewElement->SetDrawDisabledItems(infoElement.m_bDrawDisabledItems);
		pNewElement->SetIconsInRow (infoElement.m_nIconsInRow);

		if (infoElement.m_arGroups.GetSize () == 0)
		{
			if (!infoElement.m_bIsButtonMode)
			{
				pNewElement->AddGroup (_T(""), (int)pNewElement->m_Colors.GetSize ());
				pNewElement->m_bHasGroups = TRUE;
			}
		}
	}
	else if (info.GetElementName ().Compare (CBCGPRibbonInfo::s_szButton_Palette) == 0)
	{
		const CBCGPRibbonInfo::XElementButtonPalette& infoElement = 
			(const CBCGPRibbonInfo::XElementButtonPalette&)info;

		CBCGPRibbonPaletteButton* pNewElement = 
			new CBCGPRibbonPaletteButton (infoElement.m_ID.m_Value,
										infoElement.m_strText,
										infoElement.m_nSmallImageIndex,
										infoElement.m_nLargeImageIndex);
		pElement = pNewElement;

		ConstructBaseElement (*pElement, info);

		pNewElement->SetButtonMode (infoElement.m_bIsButtonMode);
		pNewElement->EnableMenuResize (infoElement.m_bEnableMenuResize, infoElement.m_bMenuResizeVertical);
		pNewElement->SetDrawDisabledItems(infoElement.m_bDrawDisabledItems);
		pNewElement->SetIconsInRow (infoElement.m_nIconsInRow);

		pNewElement->Clear ();
		int nCount = (int)infoElement.m_arGroups.GetSize ();
		if (nCount > 0)
		{
			BOOL bIsOwnerDraw = pNewElement->m_bIsOwnerDraw;
			pNewElement->m_bIsOwnerDraw = TRUE;
			for (int i = 0; i < nCount; i++)
			{
				pNewElement->AddGroup (infoElement.m_arGroups[i]->m_strName, infoElement.m_arGroups[i]->m_nItems);
			}
			pNewElement->m_bIsOwnerDraw = bIsOwnerDraw;
		}

		const_cast<CBCGPToolBarImages&>(infoElement.m_Images.m_Image).CopyTo (pNewElement->m_imagesPalette);
		pNewElement->m_nIcons = pNewElement->m_imagesPalette.GetCount ();
		pNewElement->CreateIcons ();
	}
	else if (info.GetElementName ().Compare (CBCGPRibbonInfo::s_szButton_Hyperlink) == 0)
	{
		const CBCGPRibbonInfo::XElementButtonHyperlink& infoElement = 
			(const CBCGPRibbonInfo::XElementButtonHyperlink&)info;

		CBCGPRibbonHyperlink* pNewElement = 
			new CBCGPRibbonHyperlink (infoElement.m_ID.m_Value, infoElement.m_strText, infoElement.m_strLink);
		pElement = pNewElement;

		ConstructBaseElement (*pElement, info);
	}
	else if (info.GetElementName ().Compare (CBCGPRibbonInfo::s_szButton_Radio) == 0)
	{
		const CBCGPRibbonInfo::XElementButtonRadio& infoElement = 
			(const CBCGPRibbonInfo::XElementButtonRadio&)info;

		CBCGPRibbonRadioButton* pNewElement = 
			new CBCGPRibbonRadioButton (infoElement.m_ID.m_Value, infoElement.m_strText);
		pElement = pNewElement;

		ConstructBaseElement (*pElement, info);
	}
	else if (info.GetElementName ().Compare (CBCGPRibbonInfo::s_szButton_Check) == 0)
	{
		const CBCGPRibbonInfo::XElementButtonCheck& infoElement = 
			(const CBCGPRibbonInfo::XElementButtonCheck&)info;

		CBCGPRibbonCheckBox* pNewElement = 
			new CBCGPRibbonCheckBox (infoElement.m_ID.m_Value, infoElement.m_strText);
		pElement = pNewElement;

		ConstructBaseElement (*pElement, info);
	}
	else if (info.GetElementName ().Compare (CBCGPRibbonInfo::s_szStatusPane) == 0)
	{
		const CBCGPRibbonInfo::XElementStatusPane& infoElement = 
			(const CBCGPRibbonInfo::XElementStatusPane&)info;

		CBCGPRibbonStatusBarPane* pNewElement = 
			new CBCGPRibbonStatusBarPane (infoElement.m_ID.m_Value,
											infoElement.m_strText,
											infoElement.m_bIsStatic);
		pElement = pNewElement;

		ConstructBaseElement (*pElement, info);

		if (!infoElement.m_strAlmostLargeText.IsEmpty())
		{
			pNewElement->SetAlmostLargeText (infoElement.m_strAlmostLargeText);
		}
		pNewElement->SetTextAlign (infoElement.m_nTextAlign);
	}
	else if (info.GetElementName ().Compare (CBCGPRibbonInfo::s_szButton_MainPanel) == 0)
	{
		const CBCGPRibbonInfo::XElementButtonMainPanel& infoElement = 
			(const CBCGPRibbonInfo::XElementButtonMainPanel&)info;

		CBCGPRibbonMainPanelButton* pNewElement = 
			new CBCGPRibbonMainPanelButton (infoElement.m_ID.m_Value,
									infoElement.m_strText,
									infoElement.m_nSmallImageIndex);

		pElement = pNewElement;

		ConstructBaseElement (*pElement, info);
	}
	else if (info.GetElementName ().Compare (CBCGPRibbonInfo::s_szButton_Command) == 0)
	{
		const CBCGPRibbonInfo::XElementButtonCommand& infoElement = 
			(const CBCGPRibbonInfo::XElementButtonCommand&)info;

		CBCGPRibbonButton* pNewElement = 
			new CBCGPRibbonButton (infoElement.m_ID.m_Value,
									infoElement.m_strText,
									infoElement.m_bIsMenu ? -1 : infoElement.m_nSmallImageIndex);
		pElement = pNewElement;

		ConstructBaseElement (*pElement, info, FALSE);
	}
	else if (info.GetElementName ().Compare (CBCGPRibbonInfo::s_szButton) == 0)
	{
		const CBCGPRibbonInfo::XElementButton& infoElement = 
			(const CBCGPRibbonInfo::XElementButton&)info;

		CBCGPRibbonButton* pNewElement = 
			new CBCGPRibbonButton (infoElement.m_ID.m_Value,
									infoElement.m_strText,
									infoElement.m_nSmallImageIndex,
									infoElement.m_nLargeImageIndex,
									infoElement.m_bIsAlwaysShowDescription);
		pElement = pNewElement;

		ConstructBaseElement (*pElement, info);
	}
	else if (info.GetElementName ().Compare (CBCGPRibbonInfo::s_szSlider) == 0)
	{
		const CBCGPRibbonInfo::XElementSlider& infoElement = 
			(const CBCGPRibbonInfo::XElementSlider&)info;

		CBCGPRibbonSlider* pNewElement = 
			new CBCGPRibbonSlider (infoElement.m_ID.m_Value,
									infoElement.m_nWidth,
									infoElement.m_dwStyle);
		pElement = pNewElement;

		ConstructBaseElement (*pElement, info);

		pNewElement->SetZoomButtons (infoElement.m_bZoomButtons);
		pNewElement->SetRange (infoElement.m_nMin, infoElement.m_nMax);
		pNewElement->SetPos (infoElement.m_nPos, FALSE);
	}
	else if (info.GetElementName ().Compare (CBCGPRibbonInfo::s_szProgress) == 0)
	{
		const CBCGPRibbonInfo::XElementProgressBar& infoElement = 
			(const CBCGPRibbonInfo::XElementProgressBar&)info;

		CBCGPRibbonProgressBar* pNewElement = 
			new CBCGPRibbonProgressBar (infoElement.m_ID.m_Value,
										infoElement.m_nWidth,
										infoElement.m_nHeight,
										infoElement.m_bVertical);
		pElement = pNewElement;

		ConstructBaseElement (*pElement, info);

		pNewElement->SetRange (infoElement.m_nMin, infoElement.m_nMax);
		pNewElement->SetPos (infoElement.m_nPos, FALSE);
		pNewElement->SetInfiniteMode (infoElement.m_bInfinite);
	}
	else if (info.GetElementName ().Compare (CBCGPRibbonInfo::s_szSeparator) == 0)
	{
		const CBCGPRibbonInfo::XElementSeparator& infoElement = 
			(const CBCGPRibbonInfo::XElementSeparator&)info;

		CBCGPRibbonSeparator* pSeparator = 
			new CBCGPRibbonSeparator (infoElement.m_bIsHoriz);

		pElement = pSeparator;
	}

	return pElement;
}


CBCGPRibbonCustomizationConstructor::CBCGPRibbonCustomizationConstructor(const CBCGPRibbonCustomizationInfo& info)
	: m_Info (info)
{
}

CBCGPRibbonCustomizationConstructor::~CBCGPRibbonCustomizationConstructor()
{
}

void CBCGPRibbonCustomizationConstructor::Construct (CBCGPRibbonCustomizationData& data) const
{
	data.m_VersionStamp = GetInfo ().GetVersionStamp ();

	ConstructDataCategory (data, GetInfo ().GetCategoryData ());
	ConstructDataPanel (data, GetInfo ().GetPanelData ());
}

void CBCGPRibbonCustomizationConstructor::ConstructDataCategory (CBCGPRibbonCustomizationData& data, const CBCGPRibbonCustomizationInfo::XDataCategory& info) const
{
	data.m_arHiddenTabs.Copy (info.m_arHidden);

	POSITION pos = 0;

	pos = info.m_mapIndexes.GetStartPosition ();
	while (pos != NULL)
	{
		int key = 0;
		int value = 0;

		info.m_mapIndexes.GetNextAssoc (pos, key, value);
		data.m_mapTabIndexes[key] = value;
	}

	pos = info.m_mapNames.GetStartPosition ();
	while (pos != NULL)
	{
		int key = 0;
		CString value;

		info.m_mapNames.GetNextAssoc (pos, key, value);
		data.m_mapTabNames[key] = value;
	}

	for (int i = 0; i < (int)info.m_arCustom.GetSize (); i++)
	{
		if (info.m_arCustom[i] != NULL)
		{
			CBCGPRibbonCustomCategory* pCategory = new CBCGPRibbonCustomCategory;
			ConstructCustomCategory (*pCategory, *info.m_arCustom[i]);

			data.m_arCustomTabs.Add (pCategory);
		}
	}
}

void CBCGPRibbonCustomizationConstructor::ConstructDataPanel (CBCGPRibbonCustomizationData& data, const CBCGPRibbonCustomizationInfo::XDataPanel& info) const
{
	data.m_arHiddenPanels.Copy (info.m_arHidden);

	POSITION pos = 0;

	pos = info.m_mapIndexes.GetStartPosition ();
	while (pos != NULL)
	{
		DWORD key = 0;
		int value = 0;

		info.m_mapIndexes.GetNextAssoc (pos, key, value);
		data.m_mapPanelIndexes[key] = value;
	}

	pos = info.m_mapNames.GetStartPosition ();
	while (pos != NULL)
	{
		DWORD key = 0;
		CString value;

		info.m_mapNames.GetNextAssoc (pos, key, value);
		data.m_mapPanelNames[key] = value;
	}

	pos = info.m_mapCustom.GetStartPosition ();
	while (pos != NULL)
	{
		DWORD key = 0;
		CBCGPRibbonCustomizationInfo::XCustomPanel* value = NULL;

		info.m_mapCustom.GetNextAssoc (pos, key, value);
		if (value != NULL)
		{
			CBCGPRibbonCustomPanel* pPanel = new CBCGPRibbonCustomPanel;
			ConstructCustomPanel (*pPanel, *value);

			data.m_mapCustomPanels[key] = pPanel;
		}
	}
}


void CBCGPRibbonCustomizationConstructor::ConstructCustomCategory (CBCGPRibbonCustomCategory& category, const CBCGPRibbonCustomizationInfo::XCustomCategory& info) const
{
	category.m_nKey = info.m_nKey;
	category.m_strName = info.m_strName;
	category.m_uiContextID = info.m_uiContextID;

	for (int i = 0; i < (int)info.m_arPanels.GetSize (); i++)
	{
		if (info.m_arPanels[i] != NULL)
		{
			CBCGPRibbonCustomPanel* pPanel = new CBCGPRibbonCustomPanel;
			ConstructCustomPanel (*pPanel, *info.m_arPanels[i]);

			category.m_arPanels.Add (pPanel);
		}
	}
}

void CBCGPRibbonCustomizationConstructor::ConstructCustomPanel (CBCGPRibbonCustomPanel& panel, const CBCGPRibbonCustomizationInfo::XCustomPanel& info) const
{
	panel.m_nID = info.m_nID;
	panel.m_nKey = info.m_nKey;
	panel.m_strName = info.m_strName;
	panel.m_nIndex = info.m_nIndex;
	panel.m_dwOriginal = info.m_dwOriginal;

	if ((int)panel.m_dwOriginal <= 0)
	{
		for (int i = 0; i < (int)info.m_arElements.GetSize (); i++)
		{
			const CBCGPRibbonCustomizationInfo::XCustomElement* infoElement = info.m_arElements[i];
			if (infoElement != NULL)
			{
				CBCGPRibbonCustomElement* pElement = new CBCGPRibbonCustomElement;
				pElement->m_nID = infoElement->m_nID;
				pElement->m_strName = infoElement->m_strName;
				pElement->m_nCustomIcon = infoElement->m_nCustomIcon;

				panel.m_arElements.Add (pElement);
			}
		}
	}
}

#endif // BCGP_EXCLUDE_RIBBON
