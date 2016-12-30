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
// BCGPVisualConstructor.cpp: implementation of the CBCGPRibbonConstrucor class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BCGPVisualConstructor.h"
#include "BCGPVisualContainer.h"
#include "BCGPTextGaugeImpl.h"
#include "BCGPAnalogClock.h"
#include "BCGPImageGaugeImpl.h"
#include "BCGPDiagramVisualObject.h"
#include "BCGPDiagramVisualContainer.h"
#include "BCGPWinUITiles.h"
#include "BCGPGlobalUtils.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

class XBCGPTreeMapGroup: public CBCGPTreeMapGroup
{
	friend CBCGPBaseTreeMapNode* CreateTreeMapData(const CBCGPVisualInfo::XTreeMapData& info);
};

inline CBCGPBaseTreeMapNode* CreateTreeMapData(const CBCGPVisualInfo::XTreeMapData& info)
{
	CBCGPBaseTreeMapNode* pData = NULL;

	if (info.GetName ().Compare (CBCGPVisualInfo::s_szTreeMapGroup) == 0)
	{
		const CBCGPVisualInfo::XTreeMapGroup& infoGroup = 
			(const CBCGPVisualInfo::XTreeMapGroup&)info;

		XBCGPTreeMapGroup* pGroup = (XBCGPTreeMapGroup*)new CBCGPTreeMapGroup(infoGroup.m_brFill, infoGroup.m_strLabel);
		pData = pGroup;

		pGroup->SetMargin (infoGroup.m_szMargin);
		pGroup->m_brText = infoGroup.m_brText;
		pGroup->m_tf     = infoGroup.m_fmtText;

		for (int i = 0; i < (int)infoGroup.m_arNodes.GetSize (); i++)
		{
			CBCGPBaseTreeMapNode* pNode = CreateTreeMapData (*infoGroup.m_arNodes[i]);
			if (pNode != NULL)
			{
				pGroup->AddSubNode (pNode);
			}
		}
	}
	else
	{
		const CBCGPVisualInfo::XTreeMapNode& infoNode = 
			(const CBCGPVisualInfo::XTreeMapNode&)info;

		CBCGPTreeMapNode* pNode = new CBCGPTreeMapNode(infoNode.m_dblValue, infoNode.m_strLabel);
		pData = pNode;
	}

	return pData;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGPVisualConstructor::CBCGPVisualConstructor(const CBCGPVisualInfo& info)
	: m_Info(info)
{
}

CBCGPVisualConstructor::~CBCGPVisualConstructor()
{
}

void CBCGPVisualConstructor::Construct (CBCGPVisualContainer& container) const
{
	const CBCGPVisualInfo::XContainer& infoContainer = GetInfo ().GetContainer ();

	if (container.IsKindOf (RUNTIME_CLASS(CBCGPDiagramVisualContainer)))
	{
		CArray<CBCGPBaseVisualObject*, CBCGPBaseVisualObject*> arConnectors;
		CArray<CBCGPBaseVisualObject*, CBCGPBaseVisualObject*> arItems;

		int i = 0;
		for (i = 0; i < (int)infoContainer.m_arElements.GetSize (); i++)
		{
			CBCGPBaseVisualObject* pElement = CreateElement ((const CBCGPVisualInfo::XElement&)*infoContainer.m_arElements[i], &container);
			if (pElement != NULL)
			{
				ASSERT_VALID(pElement);

				if (pElement->IsKindOf (RUNTIME_CLASS(CBCGPDiagramConnector)))
				{
					arConnectors.Add (pElement);
				}
				else
				{
					arItems.Add (pElement);
				}
			}
		}

		CBCGPDiagramVisualContainer& diagram = (CBCGPDiagramVisualContainer&)container;

		for (i = 0; i < (int)arItems.GetSize (); i++)
		{
			diagram.AddItem(arItems[i], arItems[i]->IsAutoDestroy ());
		}

		for (i = 0; i < (int)arConnectors.GetSize (); i++)
		{
			CBCGPDiagramConnector* pConnector = (CBCGPDiagramConnector*)arConnectors[i];

			diagram.AddConnector(pConnector, arConnectors[i]->IsAutoDestroy ());

			if (pConnector->IsKindOf (RUNTIME_CLASS (CBCGPDiagramShelfConnector)))
			{
				((CBCGPDiagramShelfConnector*)pConnector)->RecalcPoints ();
			}
			else if (pConnector->IsKindOf (RUNTIME_CLASS (CBCGPDiagramElbowConnector)))
			{
				((CBCGPDiagramElbowConnector*)pConnector)->RecalcPoints ();
			}
		}
	}
	else
	{
		for (int i = 0; i < (int)infoContainer.m_arElements.GetSize (); i++)
		{
			CBCGPBaseVisualObject* pElement = CreateElement ((const CBCGPVisualInfo::XElement&)*infoContainer.m_arElements[i], &container);
			if (pElement != NULL)
			{
				ASSERT_VALID (pElement);

				container.Add(pElement);
			}
		}
	}

	container.SetDrawDynamicObjectsOnTop (infoContainer.m_bDrawDynamicObjectsOnTop);

	if (!infoContainer.m_Rect.IsRectEmpty ())
	{
		container.SetRect (infoContainer.m_Rect, TRUE, FALSE);
	}

	container.SetFillBrush (infoContainer.m_brFill);
	container.SetOutlineBrush (infoContainer.m_brOutline);
}

CBCGPBaseVisualObject* CBCGPVisualConstructor::CreateElement (const CBCGPVisualInfo::XElement& info, const CBCGPVisualContainer* container) const
{
	CBCGPBaseVisualObject* pElement = NULL;

	if (info.GetElementName ().Compare (CBCGPVisualInfo::s_szTagCloud) == 0)
	{
		pElement = new CBCGPTagCloud;
		ConstructElement(*pElement, info);
	}
	else if (info.GetElementName ().Compare (CBCGPVisualInfo::s_szTreeMap) == 0)
	{
		pElement = new CBCGPTreeMap;
		ConstructElement(*pElement, info);
	}
	else if (info.GetElementName ().Compare (CBCGPVisualInfo::s_szWinUITiles) == 0)
	{
		pElement = new CBCGPWinUITiles;
		ConstructElement(*pElement, info);
	}
	else if (info.GetElementName ().Compare (CBCGPVisualInfo::s_szCircularGauge) == 0 ||
		info.GetElementName ().Compare (CBCGPVisualInfo::s_szKnob) == 0 ||
		info.GetElementName ().Compare (CBCGPVisualInfo::s_szAnalogClock) == 0 ||
		info.GetElementName ().Compare (CBCGPVisualInfo::s_szLinearGauge) == 0 ||
		info.GetElementName ().Compare (CBCGPVisualInfo::s_szNumericInd) == 0 ||
		info.GetElementName ().Compare (CBCGPVisualInfo::s_szColorInd) == 0 ||
		info.GetElementName ().Compare (CBCGPVisualInfo::s_szTextInd) == 0 ||
		info.GetElementName ().Compare (CBCGPVisualInfo::s_szImage) == 0 ||
		info.GetElementName ().Compare (CBCGPVisualInfo::s_szSwitch) == 0 ||
		info.GetElementName ().Compare (CBCGPVisualInfo::s_szFrame) == 0)
	{
		pElement = CreateGaugeElement((const CBCGPVisualInfo::XGaugeElement&)info, container);
	}
	else if (info.GetElementName ().Compare (CBCGPVisualInfo::s_szDiagramConnector) == 0 ||
		info.GetElementName ().Compare (CBCGPVisualInfo::s_szDiagramConnectorShelf) == 0 ||
		info.GetElementName ().Compare (CBCGPVisualInfo::s_szDiagramConnectorElbow) == 0 ||
		info.GetElementName ().Compare (CBCGPVisualInfo::s_szDiagramShape) == 0 ||
		info.GetElementName ().Compare (CBCGPVisualInfo::s_szDiagramTable) == 0 ||
		info.GetElementName ().Compare (CBCGPVisualInfo::s_szDiagramImage) == 0 ||
		info.GetElementName ().Compare (CBCGPVisualInfo::s_szDiagramCustom) == 0)
	{
		pElement = CreateDiagramElement((const CBCGPVisualInfo::XDiagramElement&)info, container);
	}

	return pElement;
}

CBCGPGaugeImpl* CBCGPVisualConstructor::CreateGaugeElement (const CBCGPVisualInfo::XGaugeElement& info, const CBCGPVisualContainer* /*container*/) const
{
	CBCGPGaugeImpl* pElement = NULL;

	if (info.GetElementName ().Compare (CBCGPVisualInfo::s_szCircularGauge) == 0 ||
		info.GetElementName ().Compare (CBCGPVisualInfo::s_szKnob) == 0 ||
		info.GetElementName ().Compare (CBCGPVisualInfo::s_szAnalogClock) == 0)
	{
		if (info.GetElementName ().Compare (CBCGPVisualInfo::s_szKnob) == 0)
		{
			pElement = new CBCGPKnob;
		}
		else if (info.GetElementName ().Compare (CBCGPVisualInfo::s_szAnalogClock) == 0)
		{
			pElement = new CBCGPAnalogClock;
		}
		else
		{
			pElement = new CBCGPCircularGaugeImpl;
		}
	}
	else if (info.GetElementName ().Compare (CBCGPVisualInfo::s_szLinearGauge) == 0)
	{
		pElement = new CBCGPLinearGaugeImpl;
	}
	else if (info.GetElementName ().Compare (CBCGPVisualInfo::s_szNumericInd) == 0)
	{
		pElement = new CBCGPNumericIndicatorImpl;
	}
	else if (info.GetElementName ().Compare (CBCGPVisualInfo::s_szColorInd) == 0)
	{
		pElement = new CBCGPColorIndicatorImpl;
	}
	else if (info.GetElementName ().Compare (CBCGPVisualInfo::s_szTextInd) == 0)
	{
		pElement = new CBCGPTextGaugeImpl;
	}
	else if (info.GetElementName ().Compare (CBCGPVisualInfo::s_szImage) == 0)
	{
		pElement = new CBCGPImageGaugeImpl;
	}
	else if (info.GetElementName ().Compare (CBCGPVisualInfo::s_szSwitch) == 0)
	{
		pElement = new CBCGPSwitchImpl;
	}
	else if (info.GetElementName ().Compare (CBCGPVisualInfo::s_szFrame) == 0)
	{
		pElement = new CBCGPStaticFrameImpl;
	}

	if (pElement != NULL)
	{
		ConstructGaugeElement(*pElement, info);
	}

	return pElement;
}

CBCGPDiagramVisualObject* CBCGPVisualConstructor::CreateDiagramElement(const CBCGPVisualInfo::XDiagramElement& info, const CBCGPVisualContainer* container) const
{
	CBCGPDiagramVisualObject* pElement = NULL;

	if (info.GetElementName ().Compare (CBCGPVisualInfo::s_szDiagramCustom) == 0 ||
		!info.m_strCustomName.IsEmpty ())
	{
		const CBCGPDiagramVisualContainer* pContainer = DYNAMIC_DOWNCAST(CBCGPDiagramVisualContainer, container);
		if (pContainer != NULL)
		{
			pElement = pContainer->CreateCustomObject(info.m_strCustomName);
		}
	}
	else
	{
		if (info.GetElementName ().Compare (CBCGPVisualInfo::s_szDiagramConnector) == 0)
		{
			pElement = new CBCGPDiagramConnector;
		}
		else if (info.GetElementName ().Compare (CBCGPVisualInfo::s_szDiagramConnectorShelf) == 0)
		{
			pElement = new CBCGPDiagramShelfConnector;
		}
		else if (info.GetElementName ().Compare (CBCGPVisualInfo::s_szDiagramConnectorElbow) == 0)
		{
			pElement = new CBCGPDiagramElbowConnector;
		}
		else if (info.GetElementName ().Compare (CBCGPVisualInfo::s_szDiagramShape) == 0)
		{
			pElement = new CBCGPDiagramShape;
		}
		else if (info.GetElementName ().Compare (CBCGPVisualInfo::s_szDiagramTable) == 0)
		{
			pElement = new CBCGPDiagramTableShape;
		}
		else if (info.GetElementName ().Compare (CBCGPVisualInfo::s_szDiagramImage) == 0)
		{
			pElement = new CBCGPDiagramImageObject;
		}
	}

	if (pElement != NULL)
	{
		ConstructDiagramElement(*pElement, info);
	}

	return pElement;
}

void CBCGPVisualConstructor::ConstructElement (CBCGPBaseVisualObject& element, const CBCGPVisualInfo::XElement& info) const
{
	if (info.GetElementName ().Compare (CBCGPVisualInfo::s_szTagCloud) == 0)
	{
		ConstructBaseElement (element, info);

		const CBCGPVisualInfo::XElementTagCloud& infoElement = 
			(const CBCGPVisualInfo::XElementTagCloud&)info;

		CBCGPTagCloud* pNewElement = (CBCGPTagCloud*)&element;

		pNewElement->SetSortOrder (infoElement.m_SortOrder);
		pNewElement->SetSortDescending (infoElement.m_bDescending);
		pNewElement->SetMaxWeight (infoElement.m_nMaxWeight);
		pNewElement->SetFontSizeStep (infoElement.m_dblFontSizeStep);
		pNewElement->SetHorzMargin (infoElement.m_szMargin.cx);
		pNewElement->SetVertMargin (infoElement.m_szMargin.cy);
		pNewElement->SetTextFormat (infoElement.m_fmtBase);
		pNewElement->SetFillBrush (infoElement.m_brFill);
		pNewElement->SetTextColor (infoElement.m_clrText);
		pNewElement->SetHighlightedTextColor (infoElement.m_clrTextHighlighted);
		
		for (int i = 0; i < (int)infoElement.m_arDataObjects.GetSize (); i++)
		{
			const CBCGPVisualInfo::XTagCloudData* pDataInfo = infoElement.m_arDataObjects[i];

			CBCGPTagCloudElement* pTag = new CBCGPTagCloudElement(pDataInfo->m_strLabel, 
				pDataInfo->m_dblValue, pDataInfo->m_clrText, pDataInfo->m_brFill, pDataInfo->m_clrBorder);
			pTag->SetHighlightedColor (pDataInfo->m_clrTextHighlighted);

			pNewElement->Add (pTag);
		}
	}
	else if (info.GetElementName ().Compare (CBCGPVisualInfo::s_szTreeMap) == 0)
	{
		ConstructBaseElement (element, info);

		const CBCGPVisualInfo::XElementTreeMap& infoElement = 
			(const CBCGPVisualInfo::XElementTreeMap&)info;

		CBCGPTreeMap* pNewElement = (CBCGPTreeMap*)&element;

		pNewElement->SetLayoutType (infoElement.m_LayoutType);
		pNewElement->SetGroupMargin (infoElement.m_Root.m_szMargin);
		pNewElement->SetFillBrush (infoElement.m_Root.m_brFill);

		for (int i = 0; i < (int)infoElement.m_Root.m_arNodes.GetSize (); i++)
		{
			CBCGPBaseTreeMapNode* pNode = CreateTreeMapData (*infoElement.m_Root.m_arNodes[i]);

			CBCGPTreeMapGroup* pGroup = DYNAMIC_DOWNCAST(CBCGPTreeMapGroup, pNode);
			if (pGroup == NULL)
			{
				if (pNode != NULL)
				{
					delete pNode;
					continue;
				}
			}

			pNewElement->AddGroup(pGroup);
		}
	}
	else if (info.GetElementName ().Compare (CBCGPVisualInfo::s_szWinUITiles) == 0)
	{
		ConstructBaseElement (element, info);

		const CBCGPVisualInfo::XElementWinUITiles& infoElement = 
			(const CBCGPVisualInfo::XElementWinUITiles&)info;

		CBCGPWinUITiles* pNewElement = (CBCGPWinUITiles*)&element;

		pNewElement->SetHorizontalLayout (infoElement.m_bIsHorizontalLayout);
		pNewElement->SetRoundedShapes (infoElement.m_bRoundedShapes);
		pNewElement->SetHorzMargin (infoElement.m_szMargin.cx);
		pNewElement->SetVertMargin (infoElement.m_szMargin.cy);
		pNewElement->SetSquareSize (infoElement.m_szSquare);

		pNewElement->SetFillBrush (infoElement.m_brFill, FALSE);
		pNewElement->SetCaptionForegroundColor (infoElement.m_colorCaptionForeground, FALSE);
		pNewElement->SetCaption(infoElement.m_strCaption, infoElement.m_dblCaptionExtraHeight);

		CBCGPImage CustomBadgeGlyphs;
		ConstructImage(CustomBadgeGlyphs, infoElement.m_CustomBadgeGlyphs);
		pNewElement->SetCustomBadgeGlyphs(CustomBadgeGlyphs);

		pNewElement->EnableTilesDragAndDrop(infoElement.m_bTilesDragAndDrop);

		pNewElement->EnableNavigationBackButton(infoElement.m_bHasNavigationBackButton, infoElement.m_pRTINavigationBackButton);

		int i = 0;
		for (i = 0; i < (int)infoElement.m_arGroupCaptions.GetSize (); i++)
		{
			const CBCGPVisualInfo::XWinUIGroupCaption* pDataInfo = infoElement.m_arGroupCaptions[i];

			CBCGPWinUITilesGroupCaption* pGroupCaption = pNewElement->SetGroupCaption(
				pDataInfo->m_nID, pDataInfo->m_strName, pDataInfo->m_colorText, 
				pDataInfo->m_bIsClickable, pDataInfo->m_brFillGroup, pDataInfo->m_brOutlineGroup);
			if (pGroupCaption == NULL)
			{
				continue;
			}

			ConstructBaseWinUIElement(*pGroupCaption, *pDataInfo);

			if (!pDataInfo->m_strCustomProps.IsEmpty ())
			{
				pGroupCaption->SetCustomProps (pDataInfo->m_strCustomProps);
			}

			pGroupCaption->OnAfterLoad();
		}

		for (i = 0; i < (int)infoElement.m_arCaptionButtons.GetSize (); i++)
		{
			const CBCGPVisualInfo::XWinUICaptionButton* pDataInfo = infoElement.m_arCaptionButtons[i];

			CRuntimeClass* pRTI = NULL;
			if (!pDataInfo->m_strRTI.IsEmpty())
			{
				pRTI = CBCGPGlobalUtils::RuntimeClassFromName(pDataInfo->m_strRTI);
			}

			CBCGPWinUITilesCaptionButton* pCaptionButton = NULL;
			if (pRTI != NULL)
			{
				pCaptionButton = (CBCGPWinUITilesCaptionButton*)pRTI->CreateObject();
			}
			else
			{
				pCaptionButton = new CBCGPWinUITilesCaptionButton;
			}

			ConstructBaseWinUIElement(*pCaptionButton, *pDataInfo);

			pCaptionButton->m_nCommandID = pDataInfo->m_nCommandID;
			pCaptionButton->SetDescription(pDataInfo->m_strDescription);

			CBCGPImage image;
			ConstructImage(image, pDataInfo->m_Image);
			pCaptionButton->SetImage(image);

			pNewElement->AddCaptionButton(pCaptionButton);

			if (!pDataInfo->m_strCustomProps.IsEmpty ())
			{
				pCaptionButton->SetCustomProps (pDataInfo->m_strCustomProps);
			}

			pCaptionButton->OnAfterLoad();
		}

		for (i = 0; i < (int)infoElement.m_arTiles.GetSize (); i++)
		{
			const CBCGPVisualInfo::XWinUITile* pDataInfo = infoElement.m_arTiles[i];

			CRuntimeClass* pRTI = NULL;
			if (!pDataInfo->m_strRTI.IsEmpty())
			{
				pRTI = CBCGPGlobalUtils::RuntimeClassFromName(pDataInfo->m_strRTI);
			}

			CBCGPWinUITile* pTile = NULL;
			if (pRTI != NULL)
			{
				pTile = (CBCGPWinUITile*)pRTI->CreateObject();

				pTile->SetName(pDataInfo->m_strName);
				pTile->SetTextColor(pDataInfo->m_colorText);

				pTile->m_Type = pDataInfo->m_Type;
				pTile->SetBorderColor(pDataInfo->m_colorBorder);
				pTile->SetBackgroundBrush(pDataInfo->m_brushBackground);
			}
			else
			{
				pTile = new CBCGPWinUITile(pDataInfo->m_strName, pDataInfo->m_Type, 
					pDataInfo->m_colorText, pDataInfo->m_brushBackground, pDataInfo->m_colorBorder);
			}

			ConstructBaseWinUIElement(*pTile, *pDataInfo);

			pTile->SetBorderWidth (pDataInfo->m_dblBorderWidth);
			pTile->SetBadgeGlyph (pDataInfo->m_BadgeGlyph);
			pTile->SetBadgeNumber (pDataInfo->m_nBadgeNumber);
			pTile->SetCustomBadgeIndex(pDataInfo->m_nCustomBadgeIndex);
			pTile->SetHeader (pDataInfo->m_strHeader);
			pTile->SetText (pDataInfo->m_strText);
			pTile->m_nImportance = pDataInfo->m_nImportance;

			CBCGPImage image;
			ConstructImage(image, pDataInfo->m_Image);
			pTile->SetImage (image, CBCGPWinUITile::BCGP_ANIMATION_NONE, 1000, pDataInfo->m_bStretchImage);

			pNewElement->Add (pTile, pDataInfo->m_nGroup);

			if (!pDataInfo->m_strCustomProps.IsEmpty ())
			{
				pTile->SetCustomProps (pDataInfo->m_strCustomProps);
			}

			pTile->OnAfterLoad();
		}
	}
	else if (DYNAMIC_DOWNCAST(CBCGPGaugeImpl, &element) != NULL)
	{
		ConstructGaugeElement((CBCGPGaugeImpl&)element, (const CBCGPVisualInfo::XGaugeElement&)info);
	}
	else if (DYNAMIC_DOWNCAST(CBCGPDiagramVisualObject, &element) != NULL)
	{
		ConstructDiagramElement((CBCGPDiagramVisualObject&)element, (const CBCGPVisualInfo::XDiagramElement&)info);
	}
}

void CBCGPVisualConstructor::ConstructGaugeElement (CBCGPGaugeImpl& element, const CBCGPVisualInfo::XGaugeElement& info) const
{
	ConstructBaseElement(element, info);

	element.SetInteractiveMode (info.m_bIsInteractiveMode);
	element.SetToolTip (info.m_strToolTip, info.m_strDescription);
	element.SetFrameSize (info.m_nFrameSize);

	if (DYNAMIC_DOWNCAST(CBCGPStaticGaugeImpl, &element) != NULL)
	{
		const CBCGPVisualInfo::XStaticGaugeElement& infoStatic = 
			(const CBCGPVisualInfo::XStaticGaugeElement&)info;

		CBCGPStaticGaugeImpl* pStaticElement = (CBCGPStaticGaugeImpl*)&element;

		pStaticElement->SetFillBrush(infoStatic.m_brFill);
		pStaticElement->SetOutlineBrush(infoStatic.m_brOutline);
		pStaticElement->SetOpacity(infoStatic.m_dblOpacity, FALSE);
		pStaticElement->SetDefaultDrawFlags(infoStatic.m_DefaultDrawFlags, FALSE);

		if (info.GetElementName ().Compare (CBCGPVisualInfo::s_szColorInd) == 0)
		{
			const CBCGPVisualInfo::XElementColor& infoElement = 
				(const CBCGPVisualInfo::XElementColor&)info;

			CBCGPColorIndicatorImpl* pNewElement = (CBCGPColorIndicatorImpl*)pStaticElement;

			pNewElement->SetColors (infoElement.m_Colors);
			pNewElement->SetStyle (infoElement.m_Style);
			pNewElement->SetStretched (infoElement.m_bStretched);
		}
		else if (info.GetElementName ().Compare (CBCGPVisualInfo::s_szTextInd) == 0)
		{
			const CBCGPVisualInfo::XElementText& infoElement = 
				(const CBCGPVisualInfo::XElementText&)info;

			CBCGPTextGaugeImpl* pNewElement = (CBCGPTextGaugeImpl*)pStaticElement;

			pNewElement->SetTextBrush (infoElement.m_brText);
			if (!infoElement.m_fmtText.IsEmpty())
			{
				pNewElement->SetTextFormat (infoElement.m_fmtText);
			}
			pNewElement->SetText (infoElement.m_strText, infoElement.m_brText);
		}
		else if (info.GetElementName ().Compare (CBCGPVisualInfo::s_szImage) == 0)
		{
			const CBCGPVisualInfo::XElementImage& infoElement = 
				(const CBCGPVisualInfo::XElementImage&)info;

			CBCGPImageGaugeImpl* pNewElement = (CBCGPImageGaugeImpl*)pStaticElement;

			pNewElement->SetImageAlign (infoElement.m_AlignHorz, infoElement.m_AlignVert, infoElement.m_bLockAspectRatio);

			ConstructImage(pNewElement->m_Image, infoElement.m_Image);
		}
		else if (info.GetElementName ().Compare (CBCGPVisualInfo::s_szSwitch) == 0)
		{
			const CBCGPVisualInfo::XElementSwitch& infoElement = 
				(const CBCGPVisualInfo::XElementSwitch&)info;

			CBCGPSwitchImpl* pNewElement = (CBCGPSwitchImpl*)pStaticElement;

			pNewElement->SetColors (infoElement.m_Colors);
			pNewElement->SetStyle (infoElement.m_Style);
			pNewElement->SetLabelTextFormat (infoElement.m_fmtText);
			pNewElement->SetLabel (infoElement.m_strLabelOff, FALSE);
			pNewElement->SetLabel (infoElement.m_strLabelOn, TRUE);
			pNewElement->EnableOnOffLabels (infoElement.m_bDrawTextLabels);
			pNewElement->SetOn (infoElement.m_bValue);
		}
		else if (info.GetElementName ().Compare (CBCGPVisualInfo::s_szFrame) == 0)
		{
			const CBCGPVisualInfo::XElementFrame& infoElement = 
				(const CBCGPVisualInfo::XElementFrame&)info;

			CBCGPStaticFrameImpl* pNewElement = (CBCGPStaticFrameImpl*)pStaticElement;

			pNewElement->SetStrokeStyle (infoElement.m_strokeStyle, FALSE);
			pNewElement->SetFrameSize (infoElement.m_dblFrameSize, FALSE);
			pNewElement->SetCornerRadius (infoElement.m_dblCornerRadius, FALSE);
		}

		return;
	}

	if (info.GetElementName ().Compare (CBCGPVisualInfo::s_szCircularGauge) == 0 ||
		info.GetElementName ().Compare (CBCGPVisualInfo::s_szKnob) == 0 ||
		info.GetElementName ().Compare (CBCGPVisualInfo::s_szAnalogClock) == 0)
	{
		BOOL bKnob = info.GetElementName ().Compare (CBCGPVisualInfo::s_szKnob) == 0;
		BOOL bAnalogClock = info.GetElementName ().Compare (CBCGPVisualInfo::s_szAnalogClock) == 0;
		BOOL bAnalogClockChecked = FALSE;

		const CBCGPVisualInfo::XElementCircular& infoElement = 
			(const CBCGPVisualInfo::XElementCircular&)info;

		CBCGPCircularGaugeImpl* pNewElement = (CBCGPCircularGaugeImpl*)&element;

		pNewElement->SetColors (infoElement.m_Colors);

		if (!infoElement.m_fmtText.IsEmpty())
		{
			pNewElement->SetTextFormat (infoElement.m_fmtText);
		}

		pNewElement->SetCapSize (infoElement.m_dblCapSize);

		if (!bKnob && !bAnalogClock)
		{
			pNewElement->EnableShapeByTicksArea (infoElement.m_bShapeByTicksArea);
		}

		int i = 0;
		for (i = 0; i < (int)infoElement.m_arScales.GetSize (); i++)
		{
			const CBCGPVisualInfo::XCircularScale* pScaleInfo = 
				(const CBCGPVisualInfo::XCircularScale*)infoElement.m_arScales[i];

			int index = 0;
			if (i != 0)
			{
				index = pNewElement->AddScale ();
			}

			if (pScaleInfo->m_bIsClosed)
			{
				pNewElement->SetClosedRange (pScaleInfo->m_dblStart, pScaleInfo->m_dblFinish, pScaleInfo->m_dblStartAngle, 
					pScaleInfo->m_bDrawLastTickMark, pScaleInfo->m_bAnimationThroughStart, 
					index);
			}
			else
			{
				pNewElement->SetRange (pScaleInfo->m_dblStart, pScaleInfo->m_dblFinish, index);
				pNewElement->SetTicksAreaAngles (pScaleInfo->m_dblStartAngle, pScaleInfo->m_dblFinishAngle, index);
			}
			pNewElement->SetStep (pScaleInfo->m_dblStep, index);
			pNewElement->SetTextLabelFormat (pScaleInfo->m_strLabelFormat, index);
			pNewElement->SetTickMarkStyle (pScaleInfo->m_MinorTickMarkStyle, FALSE, pScaleInfo->m_dblMinorTickMarkSize, index);
			pNewElement->SetTickMarkStyle (pScaleInfo->m_MajorTickMarkStyle, TRUE, pScaleInfo->m_dblMajorTickMarkSize, index);
			pNewElement->SetMinorTickMarkPosition (pScaleInfo->m_MinorTickMarkPosition, index);
			pNewElement->SetMajorTickMarkStep (pScaleInfo->m_dblMajorTickMarkStep, index);
			pNewElement->SetScaleFillBrush (pScaleInfo->m_brFill, index);
			pNewElement->SetScaleOutlineBrush (pScaleInfo->m_brOutline, index);
			pNewElement->SetScaleTextBrush (pScaleInfo->m_brText, index);
			pNewElement->SetScaleTickMarkBrush (pScaleInfo->m_brTickMarkMinor, FALSE, index);
			pNewElement->SetScaleTickMarkOutlineBrush (pScaleInfo->m_brTickMarkMinorOutline, FALSE, index);
			pNewElement->SetScaleTickMarkBrush (pScaleInfo->m_brTickMarkMajor, TRUE, index);
			pNewElement->SetScaleTickMarkOutlineBrush (pScaleInfo->m_brTickMarkMajorOutline, TRUE, index);
			pNewElement->SetScaleOffsetFromFrame (pScaleInfo->m_dblOffsetFromFrame, index);
			pNewElement->EnableLabelsRotation (pScaleInfo->m_bRotateLabels, index);
		}

		if (bKnob)
		{
			if ((int)infoElement.m_arPointers.GetSize () > 0)
			{
				const CBCGPVisualInfo::XKnobPointer* pPointerInfo = 
					(const CBCGPVisualInfo::XKnobPointer*)infoElement.m_arPointers[0];

				CBCGPKnobPointer pointer(pPointerInfo->m_brFill, 
						pPointerInfo->m_brOutline, pPointerInfo->m_Style, pPointerInfo->m_dblOffsetFromCenter, 
						pPointerInfo->m_dblWidth);

				((CBCGPKnob*)pNewElement)->SetPointer (pointer, FALSE);
				pNewElement->SetValue (pPointerInfo->m_dblValue, 0, 0, FALSE);
			}
		}
		else
		{
			int nDefaultPointers = 1;
			if (bAnalogClock)
			{
				nDefaultPointers = (int)infoElement.m_arPointers.GetSize ();
				if (nDefaultPointers == 3)
				{
					((CBCGPAnalogClock*)pNewElement)->EnableSecondHand ();
				}
			}

			for (i = 0; i < (int)infoElement.m_arPointers.GetSize (); i++)
			{
				const CBCGPVisualInfo::XCircularPointer* pPointerInfo = 
					(const CBCGPVisualInfo::XCircularPointer*)infoElement.m_arPointers[i];

				CBCGPCircularGaugePointer pointer(pPointerInfo->m_brFill, 
						pPointerInfo->m_brOutline, pPointerInfo->m_Style, pPointerInfo->m_dblSize, 
						pPointerInfo->m_dblWidth, pPointerInfo->m_bExtraLen);

				if (i >= nDefaultPointers)
				{
					pNewElement->AddPointer (pointer, pPointerInfo->m_nScale, FALSE);
				}
				else
				{
					pNewElement->ModifyPointer (i, pointer, FALSE);
				}

				if (!bAnalogClock)
				{
					pNewElement->SetValue (pPointerInfo->m_dblValue, i, 0, FALSE);
				}
			}
		}

		for (i = 0; i < (int)infoElement.m_arRanges.GetSize (); i++)
		{
			const CBCGPVisualInfo::XCircularColoredRange* pRangeInfo = 
				(const CBCGPVisualInfo::XCircularColoredRange*)infoElement.m_arRanges[i];

			pNewElement->AddColoredRange (pRangeInfo->m_dblStartValue, pRangeInfo->m_dblFinishValue, pRangeInfo->m_brFill,
				pRangeInfo->m_brOutline, pRangeInfo->m_nScale, pRangeInfo->m_dblStartWidth, pRangeInfo->m_dblFinishWidth, 
				pRangeInfo->m_dblOffsetFromFrame, pRangeInfo->m_brTextLabel, pRangeInfo->m_brTickMarkOutline, pRangeInfo->m_brTickMarkFill, FALSE);
		}

		for (i = 0; i < (int)infoElement.m_arSubGauges.GetSize (); i++)
		{
			CBCGPVisualInfo::XGaugeElement* pSubGaugeInfo = infoElement.m_arSubGauges[i];
			CBCGPGaugeImpl* pSubGauge = CreateGaugeElement (*pSubGaugeInfo, NULL);
			if (pSubGauge != NULL)
			{
				pNewElement->AddSubGauge (pSubGauge, pSubGaugeInfo->m_Pos, pSubGaugeInfo->m_Rect.Size (), pSubGaugeInfo->m_ptOffset);
				
				if (bAnalogClock && !bAnalogClockChecked)
				{
					if (((CBCGPVisualInfo::XElementAnalogClock&)info).m_nDateIndex == i &&
						pSubGauge->IsKindOf (RUNTIME_CLASS(CBCGPNumericIndicatorImpl)))
					{
						((CBCGPAnalogClock*)pNewElement)->m_pDate = (CBCGPNumericIndicatorImpl*)pSubGauge;
						bAnalogClockChecked = TRUE;
					}
				}
			}
		}
	}
	else if (info.GetElementName ().Compare (CBCGPVisualInfo::s_szLinearGauge) == 0)
	{
		const CBCGPVisualInfo::XElementLinear& infoElement = 
			(const CBCGPVisualInfo::XElementLinear&)info;

		CBCGPLinearGaugeImpl* pNewElement = (CBCGPLinearGaugeImpl*)&element;

		pNewElement->SetColors (infoElement.m_Colors);
		if (!infoElement.m_fmtText.IsEmpty())
		{
			pNewElement->SetTextFormat (infoElement.m_fmtText);
		}
		pNewElement->SetVerticalOrientation (infoElement.m_bIsVertical);

		int i = 0;
		for (i = 0; i < (int)infoElement.m_arScales.GetSize (); i++)
		{
			const CBCGPVisualInfo::XLinearScale* pScaleInfo = 
				(const CBCGPVisualInfo::XLinearScale*)infoElement.m_arScales[i];

			int index = 0;
			if (i != 0)
			{
				index = pNewElement->AddScale ();
			}

			pNewElement->SetRange (pScaleInfo->m_dblStart, pScaleInfo->m_dblFinish, index);
			pNewElement->SetStep (pScaleInfo->m_dblStep, index);
			pNewElement->SetTextLabelFormat (pScaleInfo->m_strLabelFormat, index);
			pNewElement->SetTickMarkStyle (pScaleInfo->m_MinorTickMarkStyle, FALSE, pScaleInfo->m_dblMinorTickMarkSize, index);
			pNewElement->SetTickMarkStyle (pScaleInfo->m_MajorTickMarkStyle, TRUE, pScaleInfo->m_dblMajorTickMarkSize, index);
			pNewElement->SetMinorTickMarkPosition (pScaleInfo->m_MinorTickMarkPosition, index);
			pNewElement->SetMajorTickMarkStep (pScaleInfo->m_dblMajorTickMarkStep, index);
			pNewElement->SetScaleFillBrush (pScaleInfo->m_brFill, index);
			pNewElement->SetScaleOutlineBrush (pScaleInfo->m_brOutline, index);
			pNewElement->SetScaleTextBrush (pScaleInfo->m_brText, index);
			pNewElement->SetScaleTickMarkBrush (pScaleInfo->m_brTickMarkMinor, FALSE, index);
			pNewElement->SetScaleTickMarkOutlineBrush (pScaleInfo->m_brTickMarkMinorOutline, FALSE, index);
			pNewElement->SetScaleTickMarkBrush (pScaleInfo->m_brTickMarkMajor, TRUE, index);
			pNewElement->SetScaleTickMarkOutlineBrush (pScaleInfo->m_brTickMarkMajorOutline, TRUE, index);
			pNewElement->SetScaleOffsetFromFrame (pScaleInfo->m_dblOffsetFromFrame, index);
		}

		for (i = 0; i < (int)infoElement.m_arPointers.GetSize (); i++)
		{
			const CBCGPVisualInfo::XLinearPointer* pPointerInfo = 
				(const CBCGPVisualInfo::XLinearPointer*)infoElement.m_arPointers[i];

			CBCGPLinearGaugePointer pointer(pPointerInfo->m_brFill, 
					pPointerInfo->m_brOutline, pPointerInfo->m_Style, pPointerInfo->m_dblSize, 
					pPointerInfo->m_dblWidth, pPointerInfo->m_Position);

			int index = pNewElement->AddPointer (pointer, pPointerInfo->m_nScale, FALSE);
			pNewElement->SetValue (pPointerInfo->m_dblValue, index, 0, FALSE);
		}

		for (i = 0; i < (int)infoElement.m_arRanges.GetSize (); i++)
		{
			const CBCGPVisualInfo::XLinearColoredRange* pRangeInfo = 
				(const CBCGPVisualInfo::XLinearColoredRange*)infoElement.m_arRanges[i];

			pNewElement->AddColoredRange (pRangeInfo->m_dblStartValue, pRangeInfo->m_dblFinishValue, pRangeInfo->m_brFill,
				pRangeInfo->m_brOutline, pRangeInfo->m_nScale, pRangeInfo->m_dblStartWidth, pRangeInfo->m_dblFinishWidth, 
				pRangeInfo->m_dblOffsetFromFrame, pRangeInfo->m_brTextLabel, pRangeInfo->m_brTickMarkOutline, pRangeInfo->m_brTickMarkFill, FALSE);
		}
	}
	else if (info.GetElementName ().Compare (CBCGPVisualInfo::s_szNumericInd) == 0)
	{
		const CBCGPVisualInfo::XElementNumeric& infoElement = 
			(const CBCGPVisualInfo::XElementNumeric&)info;

		CBCGPNumericIndicatorImpl* pNewElement = (CBCGPNumericIndicatorImpl*)&element;

		pNewElement->SetStyle (infoElement.m_Style);
		pNewElement->SetColors (infoElement.m_Colors);
		if (!infoElement.m_fmtText.IsEmpty())
		{
			pNewElement->SetTextFormat (infoElement.m_fmtText);
		}
		pNewElement->SetCells (infoElement.m_nCells);
		pNewElement->SetDecimals (infoElement.m_nDecimals);
		pNewElement->SetSeparatorWidth (infoElement.m_nSeparatorWidth);
		pNewElement->SetDrawSign (infoElement.m_bDrawSign);
		pNewElement->SetDrawDecimalPoint (infoElement.m_bDrawDecimalPoint);
		pNewElement->SetDrawLeadingZeros (infoElement.m_bDrawLeadingZeros);
		pNewElement->SetValue (infoElement.m_dblValue);
	}
}

void CBCGPVisualConstructor::ConstructDiagramElement(CBCGPDiagramVisualObject& element, const CBCGPVisualInfo::XDiagramElement& info) const
{
	ConstructBaseElement(element, info);

	element.SetItemID (CBCGPDiagramItemID(info.m_idItem.m_nID, info.m_idItem.m_bConnector));
	element.m_brFill      = info.m_brFill;
	element.m_brOutline   = info.m_brOutline;
	element.m_brShadow    = info.m_brShadow;
	element.m_Thickness   = info.m_Thickness;
	element.m_StrokeStyle = info.m_StrokeStyle;

	for (int i = 0; i < (int)info.m_arDataObjects.GetSize (); i++)
	{
		CBCGPVisualDataObject* pData = NULL;

		const CBCGPVisualInfo::XData* pDataObject = info.m_arDataObjects[i];
		if (pDataObject->GetName ().Compare (CBCGPVisualInfo::s_szDataDiagramText) == 0)
		{
			const CBCGPVisualInfo::XDiagramTextData* pDataInfo = 
				(const CBCGPVisualInfo::XDiagramTextData*)pDataObject;

			CBCGPDiagramTextDataObject* pDataText = new CBCGPDiagramTextDataObject(pDataInfo->m_strText, pDataInfo->m_brText);
			pDataText->SetTextFormat (pDataInfo->m_fmtText);

			pData = pDataText;
		}

		if (pData != NULL)
		{
			element.AddData (pData);
		}
	}

	if (element.IsKindOf (RUNTIME_CLASS (CBCGPDiagramConnector)))
	{
		const CBCGPVisualInfo::XElementDiagramConnector& infoElement = 
			(const CBCGPVisualInfo::XElementDiagramConnector&)info;

		CBCGPDiagramConnector* pNewElement = (CBCGPDiagramConnector*)&element;

		pNewElement->SetCurveType (infoElement.m_curveType);
		pNewElement->SetBeginArrow (infoElement.m_arrowBegin.m_nShape, infoElement.m_arrowBegin.m_dLength, infoElement.m_arrowBegin.m_dWidth);
		pNewElement->SetBeginArrowFillBrush (infoElement.m_arrowBegin.m_brFill);
		pNewElement->SetBeginArrowOutlineBrush (infoElement.m_arrowBegin.m_brOutline);
		pNewElement->SetEndArrow (infoElement.m_arrowEnd.m_nShape, infoElement.m_arrowEnd.m_dLength, infoElement.m_arrowEnd.m_dWidth);
		pNewElement->SetEndArrowFillBrush (infoElement.m_arrowEnd.m_brFill);
		pNewElement->SetEndArrowOutlineBrush (infoElement.m_arrowEnd.m_brOutline);

		for(int i = 0; i < (int)infoElement.m_arPoints.GetSize (); i++)
		{
			const CBCGPVisualInfo::XDiagramAnchorPoint* pPointInfo = infoElement.m_arPoints[i];

			CBCGPDiagramAnchorPoint point;
			point.m_idObject.m_nId        = pPointInfo->m_idObject.m_nID;
			point.m_idObject.m_bConnector = pPointInfo->m_idObject.m_bConnector;
			point.m_nConnectionPort       = pPointInfo->m_nConnectionPort;
			point.m_ptNullAnchor          = pPointInfo->m_ptNullAnchor;

			pNewElement->m_arPoints.Add (point);
		}

		if (element.IsKindOf (RUNTIME_CLASS (CBCGPDiagramShelfConnector)))
		{
			CBCGPDiagramShelfConnector* pConnector = (CBCGPDiagramShelfConnector*)&element;

			pConnector->SetShelfSize(((const CBCGPVisualInfo::XElementDiagramConnectorShelf&)infoElement).m_dShelfOffset);
		}
		else if (element.IsKindOf (RUNTIME_CLASS (CBCGPDiagramElbowConnector)))
		{
			CBCGPDiagramElbowConnector* pConnector = (CBCGPDiagramElbowConnector*)&element;

			pConnector->SetOrientation(((const CBCGPVisualInfo::XElementDiagramConnectorElbow&)infoElement).m_Orientation);
			pConnector->SetResizeHandlePoint(((const CBCGPVisualInfo::XElementDiagramConnectorElbow&)infoElement).m_ptResizeHandle);
		}
	}
	else if (element.IsKindOf (RUNTIME_CLASS (CBCGPDiagramTableShape)))
	{
		const CBCGPVisualInfo::XElementDiagramTable& infoElement = 
			(const CBCGPVisualInfo::XElementDiagramTable&)info;

		CBCGPDiagramTableShape* pNewElement = (CBCGPDiagramTableShape*)&element;

		pNewElement->m_shape         = infoElement.m_shape;
		pNewElement->m_bCaption      = infoElement.m_bCaption;
		pNewElement->m_brCaptionFill = infoElement.m_brCaptionFill;
		
		pNewElement->m_CaptionData.SetText (infoElement.m_CaptionData.m_strText, infoElement.m_CaptionData.m_brText);
		pNewElement->m_CaptionData.SetTextFormat (infoElement.m_CaptionData.m_fmtText);
	}
	else if (element.IsKindOf (RUNTIME_CLASS (CBCGPDiagramShape)))
	{
		const CBCGPVisualInfo::XElementDiagramShape& infoElement = 
			(const CBCGPVisualInfo::XElementDiagramShape&)info;

		CBCGPDiagramShape* pNewElement = (CBCGPDiagramShape*)&element;

		pNewElement->m_shape = infoElement.m_shape;
	}
	else if (element.IsKindOf (RUNTIME_CLASS (CBCGPDiagramImageObject)))
	{
		const CBCGPVisualInfo::XElementDiagramImage& infoElement = 
			(const CBCGPVisualInfo::XElementDiagramImage&)info;

		CBCGPDiagramImageObject* pNewElement = (CBCGPDiagramImageObject*)&element;

		pNewElement->SetImageAlign (infoElement.m_AlignHorz, infoElement.m_AlignVert, infoElement.m_bLockAspectRatio);

		ConstructImage(pNewElement->m_Image, infoElement.m_Image);
	}

	if (!info.m_strCustomProps.IsEmpty ())
	{
		element.FromTag (info.m_strCustomProps);
	}
}

void CBCGPVisualConstructor::ConstructBaseElement (CBCGPBaseVisualObject& element, const CBCGPVisualInfo::XElement& info) const
{
	element.SetName (info.m_ID.m_Name);
	element.SetID (info.m_ID.m_Value);
	element.SetRect (info.m_Rect);
	element.SetVisible (info.m_bIsVisible);
	element.SetAutoDestroy(info.m_bIsAutoDestroy);
}

void CBCGPVisualConstructor::ConstructBaseWinUIElement (CBCGPWinUIBaseObject& element, const CBCGPVisualInfo::XWinUIBaseElement& info) const
{
	element.SetName(info.m_strName);
	element.SetToolTipText(info.m_strToolTipText);
	element.SetToolTipDescription(info.m_strToolTipDescription);
	element.SetTextColor(info.m_colorText);
	element.SetVisible(info.m_bIsVisible);

	CRuntimeClass* pRTI = NULL;
	if (!info.m_strRTIView.IsEmpty())
	{
		pRTI = CBCGPGlobalUtils::RuntimeClassFromName(info.m_strRTIView);
	}

	if (pRTI != NULL)
	{
		element.SetView(pRTI, info.m_nViewResID, info.m_strViewTitle);
	}
}

void CBCGPVisualConstructor::ConstructImage (CBCGPImage& image, const CBCGPVisualInfo::XImage& info) const
{
	image.SetMap3DColorsInGDI(info.m_bMap3dColorsInGDI);

	if (info.m_dblLightRatio > 0.0)
	{
		if (info.m_dblLightRatio <= 1.0)
		{
			image.MakeDarker(info.m_dblLightRatio);
		}
		else if (info.m_dblLightRatio <= 2.0)
		{
			image.MakeLighter(info.m_dblLightRatio - 1.0);
		}
		else
		{
			image.MakePale(info.m_dblLightRatio - 2.0);
		}
	}

	if (!info.m_clrTheme.IsNull())
	{
		image.Colorize(info.m_clrTheme);
	}

	if (!info.m_sizeDest.IsNull())
	{
		image.Resize(info.m_sizeDest);
	}

	if (info.m_ID.m_Value != 0)
	{
		image.Load (info.m_ID.m_Value, info.m_strType.IsEmpty () ? NULL : (LPCTSTR)info.m_strType);
	}
	else if (!info.m_strPath.IsEmpty ())
	{
		image.Load (info.m_strPath);
	}
}
