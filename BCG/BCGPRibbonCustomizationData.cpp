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
//
// BCGPRibbonCustomizationData.cpp: implementation of the CBCGPRibbonCustomizationData class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BCGPRibbonCustomizationData.h"
#include "BCGPRibbonPanel.h"
#include "BCGPRibbonBar.h"
#include "BCGPRibbonCategory.h"
#include "BCGPRibbonInfoLoader.h"
#include "BCGPRibbonConstructor.h"
#include "BCGPRibbonCollector.h"
#include "BCGPRibbonInfoWriter.h"
#include "BCGPTagManager.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#ifndef BCGP_EXCLUDE_RIBBON

#define HIDDEN_INDEX 9999

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGPRibbonCustomizationData::CBCGPRibbonCustomizationData()
{
	m_VersionStamp = 0;
}
//*************************************************************************************************************
CBCGPRibbonCustomizationData::~CBCGPRibbonCustomizationData()
{
	ResetAll(NULL);
}
//*************************************************************************************************************
BOOL CBCGPRibbonCustomizationData::LoadFromBuffer(LPCTSTR lpszXMLBuffer)
{
	ASSERT(lpszXMLBuffer != NULL);

	ResetAll(NULL);

	CBCGPRibbonCustomizationInfo info;

	if (!info.FromTag (lpszXMLBuffer))
	{
		TRACE0("Cannot load ribbon customization from buffer\n");
		return FALSE;
	}

	CBCGPRibbonCustomizationConstructor constr (info);
	constr.Construct (*this);	
	
	return TRUE;
}
//*************************************************************************************************************
BOOL CBCGPRibbonCustomizationData::SaveToBuffer(CString& strXMLBuffer)
{
	strXMLBuffer.Empty();

	CBCGPRibbonCustomizationInfo info;
	CBCGPRibbonCustomizationCollector collector (info);

	collector.Collect (*this);
	
	BOOL bFormatTags = CBCGPTagManager::s_bFormatTags;
	CBCGPTagManager::s_bFormatTags = FALSE;
	info.ToTag (strXMLBuffer);
	CBCGPTagManager::s_bFormatTags = bFormatTags;

	return !strXMLBuffer.IsEmpty ();
}
//*************************************************************************************************************
void CBCGPRibbonCustomizationData::CopyFrom(const CBCGPRibbonCustomizationData& src)
{
	if (&src == this)
	{
		return;
	}

	POSITION pos = NULL;

	ResetAll(NULL);

	m_arHiddenTabs.Append(src.m_arHiddenTabs);

	for (pos = src.m_mapTabIndexes.GetStartPosition (); pos != NULL;)
	{
		int nCatKey = 0;
		int nIndex = 0;

		src.m_mapTabIndexes.GetNextAssoc(pos, nCatKey, nIndex);
		m_mapTabIndexes.SetAt(nCatKey, nIndex);
	}

	for (pos = src.m_mapTabNames.GetStartPosition (); pos != NULL;)
	{
		int nCatKey = 0;
		CString strName;

		src.m_mapTabNames.GetNextAssoc(pos, nCatKey, strName);
		m_mapTabNames.SetAt(nCatKey, strName);
	}

	for (int i = 0; i < src.m_arCustomTabs.GetSize(); i++)
	{
		m_arCustomTabs.Add(new CBCGPRibbonCustomCategory(*src.m_arCustomTabs[i]));
	}

	m_arHiddenPanels.Append(src.m_arHiddenPanels);

	for (pos = src.m_mapPanelIndexes.GetStartPosition (); pos != NULL;)
	{
		DWORD dwLocation = 0;
		int nIndex = 0;

		src.m_mapPanelIndexes.GetNextAssoc(pos, dwLocation, nIndex);
		m_mapPanelIndexes.SetAt(dwLocation, nIndex);
	}

	for (pos = src.m_mapPanelNames.GetStartPosition (); pos != NULL;)
	{
		DWORD dwLocation = 0;
		CString strName;

		src.m_mapPanelNames.GetNextAssoc(pos, dwLocation, strName);
		m_mapPanelNames.SetAt(dwLocation, strName);
	}

	for (pos = src.m_mapCustomPanels.GetStartPosition (); pos != NULL;)
	{
		DWORD dwLocation = 0;
		CBCGPRibbonCustomPanel* pSrcPanel = NULL;

		src.m_mapCustomPanels.GetNextAssoc(pos, dwLocation, pSrcPanel);

		m_mapCustomPanels.SetAt(dwLocation, new CBCGPRibbonCustomPanel(*pSrcPanel));
	}
}
//*************************************************************************************************************
void CBCGPRibbonCustomizationData::ResetNonCustomData()
{
	m_arHiddenTabs.RemoveAll();
	m_mapTabIndexes.RemoveAll();
	m_mapTabNames.RemoveAll();

	m_mapPanelIndexes.RemoveAll();
	m_arHiddenPanels.RemoveAll();
	m_mapPanelNames.RemoveAll();
}
//*************************************************************************************************************
void CBCGPRibbonCustomizationData::ResetAll(CBCGPRibbonBar* pRibbonBar)
{
	ResetNonCustomData();

	for (int i = 0; i < m_arCustomTabs.GetSize(); i++)
	{
		delete m_arCustomTabs[i];
	}

	for (POSITION pos = m_mapCustomPanels.GetStartPosition (); pos != NULL;)
	{
		DWORD dwLocation = 0;
		CBCGPRibbonCustomPanel* pCustomPanel = NULL;

		m_mapCustomPanels.GetNextAssoc(pos, dwLocation, pCustomPanel);
		if (pCustomPanel != NULL)
		{
			delete pCustomPanel;
		}

		if (pRibbonBar != NULL)
		{
			CBCGPRibbonPanel* pPanel = pRibbonBar->FindPanel(LOWORD(dwLocation), HIWORD(dwLocation));
			if (pPanel != NULL)
			{
				pPanel->m_bToBeDeleted = TRUE;
			}
		}
	}

	m_arCustomTabs.RemoveAll();
	m_mapCustomPanels.RemoveAll();
}
//*************************************************************************************************************
DWORD CBCGPRibbonCustomizationData::GetPanelLocation(const CBCGPRibbonPanel &panel)
{
	int nKey = panel.GetKey();
	if (nKey <= 0)
	{
		return (DWORD)-1;
	}

	CBCGPRibbonCategory* pCategory = panel.GetParentCategory();
	if (pCategory == NULL)
	{
		return (DWORD)-1;
	}

	int nCatKey = pCategory->GetKey();
	if (nCatKey < 0)
	{
		return (DWORD)-1;
	}

	return MAKELPARAM(nCatKey, nKey);
}
//*************************************************************************************************************
void CBCGPRibbonCustomizationData::ResetTab(CBCGPRibbonBar* pRibbonBar, int nKey)
{
	ASSERT_VALID(pRibbonBar);

	int i = 0;
	POSITION pos = NULL;

	for (pos = m_mapPanelIndexes.GetStartPosition (); pos != NULL;)
	{
		DWORD dwLocation = 0;
		int nIndex = 0;

		m_mapPanelIndexes.GetNextAssoc(pos, dwLocation, nIndex);

		int nCatKey = LOWORD(dwLocation);
		if (nCatKey == nKey)
		{
			m_mapPanelIndexes.RemoveKey(dwLocation);
		}
	}

	for (i = 0; i < (int)m_arHiddenPanels.GetSize();)
	{
		int nCatKey = LOWORD(m_arHiddenPanels[i]);
		if (nCatKey == nKey)
		{
			m_mapPanelIndexes.RemoveKey(m_arHiddenPanels[i]);
			m_arHiddenPanels.RemoveAt(i);
		}
		else
		{
			i++;
		}
	}

	for (pos = m_mapPanelNames.GetStartPosition (); pos != NULL;)
	{
		DWORD dwLocation = 0;
		CString strName;

		m_mapPanelNames.GetNextAssoc(pos, dwLocation, strName);

		int nCatKey = LOWORD(dwLocation);
		if (nCatKey == nKey)
		{
			m_mapPanelNames.RemoveKey(dwLocation);
		}
	}

	for (pos = m_mapCustomPanels.GetStartPosition (); pos != NULL;)
	{
		DWORD dwLocation = 0;
		CBCGPRibbonCustomPanel* pCustomPanel = NULL;

		m_mapCustomPanels.GetNextAssoc(pos, dwLocation, pCustomPanel);

		int nCatKey = LOWORD(dwLocation);
		if (nCatKey == nKey)
		{
			if (pCustomPanel != NULL)
			{
				delete pCustomPanel;
			}
			
			CBCGPRibbonPanel* pPanel = pRibbonBar->FindPanel(LOWORD(dwLocation), HIWORD(dwLocation));
			if (pPanel != NULL)
			{
				pPanel->m_bToBeDeleted = TRUE;
			}
		}
	}
}
//*************************************************************************************************************
BOOL CBCGPRibbonCustomizationData::Apply(CBCGPRibbonBar* pRibbonBar)
{
	ASSERT_VALID(pRibbonBar);

	pRibbonBar->ApplyCustomizationData(*this);
	m_VersionStamp = pRibbonBar->m_VersionStamp;

	return TRUE;
}
//*************************************************************************************************************
BOOL CBCGPRibbonCustomizationData::IsTabHidden(const CBCGPRibbonCategory* pTab) const
{
	ASSERT_VALID(pTab);

	int nKey = pTab->GetKey();
	if (nKey <= 0)
	{
		return FALSE;
	}

	for (int i = 0; i < (int)m_arHiddenTabs.GetSize(); i++)
	{
		if (m_arHiddenTabs[i] == nKey)
		{
			return TRUE;
		}
	}

	return FALSE;
}
//*************************************************************************************************************
void CBCGPRibbonCustomizationData::ShowTab(CBCGPRibbonCategory* pTab, BOOL bShow)
{
	ASSERT_VALID(pTab);

	int nKey = pTab->GetKey();
	if (nKey <= 0)
	{
		ASSERT(FALSE);
		return;
	}

	for (int i = 0; i < (int)m_arHiddenTabs.GetSize(); i++)
	{
		if (m_arHiddenTabs[i] == nKey)
		{
			if (bShow)
			{
				m_arHiddenTabs.RemoveAt(i);
			}

			return;
		}
	}

	if (!bShow)
	{
		m_arHiddenTabs.Add(nKey);
	}
}
//*************************************************************************************************************
BOOL CBCGPRibbonCustomizationData::IsPanelHidden(const CBCGPRibbonPanel* pPanel) const
{
	ASSERT_VALID(pPanel);

	DWORD dw = GetPanelLocation(*pPanel);
	if (dw == (DWORD)-1)
	{
		return FALSE;
	}

	for (int i = 0; i < (int)m_arHiddenPanels.GetSize(); i++)
	{
		if (m_arHiddenPanels[i] == dw)
		{
			return TRUE;
		}
	}

	return FALSE;
}
//*************************************************************************************************************
void CBCGPRibbonCustomizationData::ShowPanel(CBCGPRibbonPanel* pPanel, BOOL bShow)
{
	ASSERT_VALID(pPanel);

	DWORD dw = GetPanelLocation(*pPanel);
	if (dw == (DWORD)-1)
	{
		ASSERT(FALSE);
		return;
	}

	int i = 0;

	for (i = 0; i < (int)m_arHiddenPanels.GetSize(); i++)
	{
		if (m_arHiddenPanels[i] == dw)
		{
			if (bShow)
			{
				m_arHiddenPanels.RemoveAt(i);
				m_mapPanelIndexes.RemoveKey(dw);
			}

			return;
		}
	}

	if (bShow)
	{
		return;
	}

	RemovePanelIndex(pPanel);

	m_arHiddenPanels.Add(dw);
	m_mapPanelIndexes.SetAt(dw, HIDDEN_INDEX);
}
//*************************************************************************************************************
void CBCGPRibbonCustomizationData::RenameTab(CBCGPRibbonCategory* pTab, LPCTSTR lpszName)
{
	ASSERT_VALID(pTab);
	ASSERT(lpszName != NULL);

	int nKey = pTab->GetKey();
	if (nKey <= 0)
	{
		ASSERT(FALSE);
		return;
	}

	CBCGPRibbonCustomCategory* pCustomTab = FindCustomTab(nKey);
	if (pCustomTab != NULL)
	{
		ASSERT_VALID(pCustomTab);
		pCustomTab->m_strName = lpszName;
	}
	else
	{
		m_mapTabNames.SetAt(nKey, lpszName);
	}
}
//*************************************************************************************************************
BOOL CBCGPRibbonCustomizationData::GetTabName(const CBCGPRibbonCategory* pTab, CString& strName) const
{
	ASSERT_VALID(pTab);

	int nKey = pTab->GetKey();
	if (nKey <= 0)
	{
		return FALSE;
	}

	CBCGPRibbonCustomCategory* pCustomTab = FindCustomTab(nKey);
	if (pCustomTab != NULL)
	{
		strName = pCustomTab->m_strName;
		return TRUE;
	}
	else
	{
		return m_mapTabNames.Lookup(nKey, strName);
	}
}
//*************************************************************************************************************
void CBCGPRibbonCustomizationData::RenamePanel(CBCGPRibbonPanel* pPanel, LPCTSTR lpszName)
{
	ASSERT_VALID(pPanel);
	ASSERT(lpszName != NULL);

	CBCGPRibbonCustomPanel* pCustomPanel = pPanel->IsCustom() ? FindCustomPanel(pPanel) : NULL;
	if (pCustomPanel != NULL)
	{
		pCustomPanel->m_strName = lpszName;
	}
	else
	{
		DWORD dw = GetPanelLocation(*pPanel);
		if (dw == (DWORD)-1)
		{
			ASSERT(FALSE);
			return;
		}

		m_mapPanelNames.SetAt(dw, lpszName);
	}
}
//*************************************************************************************************************
CBCGPRibbonCustomElement* CBCGPRibbonCustomizationData::FindCustomElement(CBCGPBaseRibbonElement* pElem)
{
	ASSERT_VALID(pElem);

	CBCGPRibbonPanel* pPanel = pElem->GetParentPanel();
	if (pPanel == NULL || !pPanel->IsCustom())
	{
		return NULL;
	}

	CBCGPRibbonCustomPanel* pCustomPanel = FindCustomPanel(pPanel);
	if (pCustomPanel == NULL)
	{
		return NULL;
	}

	return pCustomPanel->FindByID(pElem->GetID());
}
//*************************************************************************************************************
void CBCGPRibbonCustomizationData::RenameElement(CBCGPBaseRibbonElement* pElem, LPCTSTR lpszName)
{
	ASSERT_VALID(pElem);
	ASSERT(lpszName != NULL);

	CBCGPRibbonCustomElement* pCustomElem = FindCustomElement(pElem);
	if (pCustomElem == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	pCustomElem->m_strName = lpszName;
}
//*************************************************************************************************************
int CBCGPRibbonCustomizationData::GetElelementIndex(CBCGPBaseRibbonElement* pElem, int& nTotal)
{
	ASSERT_VALID(pElem);

	nTotal = 0;

	CBCGPRibbonPanel* pPanel = pElem->GetParentPanel();
	if (pPanel == NULL)
	{
		ASSERT(FALSE);
		return -1;
	}

	if (!pPanel->IsCustom())
	{
		return NULL;
	}

	CBCGPRibbonCustomPanel* pCustomPanel = FindCustomPanel(pPanel);
	if (pCustomPanel == NULL)
	{
		ASSERT(FALSE);
		return -1;
	}

	nTotal = (int)pCustomPanel->m_arElements.GetSize();

	for (int i = 0; i < nTotal; i++)
	{
		CBCGPRibbonCustomElement* pCustomElement = pCustomPanel->m_arElements[i];
		ASSERT_VALID(pCustomElement);

		if (pCustomElement->m_nID == pElem->GetID())
		{
			return i;
		}
	}

	return -1;
}
//*************************************************************************************************************
BOOL CBCGPRibbonCustomizationData::MoveElement(CBCGPBaseRibbonElement* pElem, int nIndex)
{
	if (nIndex == 0)
	{
		return TRUE;
	}

	ASSERT_VALID(pElem);

	CBCGPRibbonPanel* pPanel = pElem->GetParentPanel();
	if (pPanel == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	CBCGPRibbonCustomPanel* pCustomPanel = FindCustomPanel(pPanel);
	if (pCustomPanel == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	if (!pPanel->IsCustom())
	{
		return FALSE;
	}

	int nTotal = (int)pCustomPanel->m_arElements.GetSize();
	for (int i = 0; i < nTotal; i++)
	{
		CBCGPRibbonCustomElement* pCustomElement = pCustomPanel->m_arElements[i];
		ASSERT_VALID(pCustomElement);

		if (pCustomElement->m_nID == pElem->GetID())
		{
			int nNewIndex = min(nTotal - 1, max(0, i + nIndex));
			pCustomPanel->m_arElements.RemoveAt(i);
			pCustomPanel->m_arElements.InsertAt(nNewIndex, pCustomElement);
			return TRUE;
		}
	}

	ASSERT(FALSE);
	return FALSE;
}
//*************************************************************************************************************
BOOL CBCGPRibbonCustomizationData::GetPanelName(const CBCGPRibbonPanel* pPanel, CString& strName) const
{
	ASSERT_VALID(pPanel);

	CBCGPRibbonCustomPanel* pCustomPanel = FindCustomPanel(pPanel);
	if (pCustomPanel != NULL)
	{
		ASSERT_VALID(pCustomPanel);
		strName = pCustomPanel->m_strName;
		return TRUE;
	}

	DWORD dw = GetPanelLocation(*pPanel);
	if (dw == (DWORD)-1)
	{
		return FALSE;
	}

	if (m_mapPanelNames.Lookup(dw, strName))
	{
		return TRUE;
	}

	return FALSE;
}
//*************************************************************************************************************
void CBCGPRibbonCustomizationData::SetTabIndex(const CBCGPRibbonCategory* pTab, int nIndex)
{
	ASSERT_VALID(pTab);

	int nKey = pTab->GetKey();
	if (nKey <= 0)
	{
		ASSERT(FALSE);
		return;
	}

	int nPrevIndex = GetTabIndex(pTab);
	if (nPrevIndex < 0)
	{
		ASSERT(FALSE);
		return;
	}

	if (nIndex == nPrevIndex)
	{
		return;
	}

	m_mapTabIndexes.SetAt(nKey, nIndex);

	CBCGPRibbonBar* pParent = pTab->GetParentRibbonBar();
	ASSERT_VALID(pParent);

	for (int i = 0; i < pParent->GetCategoryCount(); i++)
	{
		CBCGPRibbonCategory* pCurTab = pParent->GetCategory(i);
		ASSERT_VALID(pCurTab);

		if (pCurTab == pTab || pCurTab->IsPrintPreview())
		{
			continue;
		}

		int nCurIndex = GetTabIndex(pCurTab);
		if (nCurIndex < 0)
		{
			continue;
		}

		int nKeyCurr = pCurTab->GetKey();
		if (nKeyCurr <= 0)
		{
			ASSERT(FALSE);
		}

		if (nIndex > nPrevIndex)
		{
			if (nCurIndex > nPrevIndex && nCurIndex <= nIndex)
			{
				m_mapTabIndexes.SetAt(nKeyCurr, nCurIndex - 1);
			}
		}
		else
		{
			if (nCurIndex >= nIndex && nCurIndex < nPrevIndex)
			{
				m_mapTabIndexes.SetAt(nKeyCurr, nCurIndex + 1);
			}
		}
	}
}
//*************************************************************************************************************
int CBCGPRibbonCustomizationData::GetTabIndex(const CBCGPRibbonCategory* pTab) const
{
	ASSERT_VALID(pTab);

	int nKey = pTab->GetKey();
	if (nKey <= 0)
	{
		return -1;
	}

	int nCurrIndex = -1;
	if (m_mapTabIndexes.Lookup(nKey, nCurrIndex))
	{
		return nCurrIndex;
	}

	return -1;
}
//*************************************************************************************************************
void CBCGPRibbonCustomizationData::SetTabContextID(const CBCGPRibbonCategory* pTab, UINT nContextID)
{
	CBCGPRibbonCustomCategory* pCustomTab = FindCustomTab(pTab->m_nKey);
	if (pCustomTab != NULL)
	{
		ASSERT_VALID(pCustomTab);
		pCustomTab->m_uiContextID = nContextID;
	}
}
//*************************************************************************************************************
UINT CBCGPRibbonCustomizationData::GetTabContextID(const CBCGPRibbonCategory* pTab) const
{
	CBCGPRibbonCustomCategory* pCustomTab = FindCustomTab(pTab->m_nKey);
	if (pCustomTab != NULL)
	{
		ASSERT_VALID(pCustomTab);
		return pCustomTab->m_uiContextID;
	}

	return pTab->m_uiContextID;
}
//*************************************************************************************************************
void CBCGPRibbonCustomizationData::PrepareTabsArray(const CArray<CBCGPRibbonCategory*,CBCGPRibbonCategory*>& arTabsIn, CArray<CBCGPRibbonCategory*,CBCGPRibbonCategory*>& arTabsDest)
{
	arTabsDest.RemoveAll();

	if (arTabsIn.GetSize() == 0)
	{
		return;
	}

	CArray<CBCGPRibbonCategory*,CBCGPRibbonCategory*> arTabsSrc;
	arTabsSrc.Append(arTabsIn);

	int i = 0;

	// Check for indexes:
	BOOL bRebuildIndexes = FALSE;

	for (i = 0; i < arTabsSrc.GetSize(); i++)
	{
		CBCGPRibbonCategory* pTabSrc = arTabsSrc[i];
		ASSERT_VALID(pTabSrc);

		if (pTabSrc->IsPrintPreview())
		{
			arTabsSrc.RemoveAt(i);
			i--;
			continue;
		}

		int nTabIndex = GetTabIndex(pTabSrc);
		if (nTabIndex < 0)
		{
			bRebuildIndexes = TRUE;
			break;
		}
	}

	if (bRebuildIndexes)
	{
		int nIndex = 0;

		for (i = 0; i < arTabsSrc.GetSize(); i++)
		{
			CBCGPRibbonCategory* pTab = arTabsSrc[i];
			ASSERT_VALID(pTab);

			int nKey = pTab->GetKey();
			if (nKey <= 0)
			{
				ASSERT(FALSE);
				continue;
			}
			
			m_mapTabIndexes.SetAt(nKey, nIndex++);
		}
	}

	CArray<CBCGPRibbonCategory*,CBCGPRibbonCategory*> arTabs;

	for (i = 0; i < arTabsSrc.GetSize(); i++)
	{
		CBCGPRibbonCategory* pTab = arTabsSrc[i];
		ASSERT_VALID(pTab);

		int nTabIndexSrc = GetTabIndex(pTab);

		BOOL bAdded = FALSE;

		for (int j = 0; j < arTabs.GetSize(); j++)
		{
			int nTabIndexDest = GetTabIndex(arTabs[j]);
			if (nTabIndexSrc < nTabIndexDest)
			{
				arTabs.InsertAt(j, pTab);
				bAdded = TRUE;
				break;
			}
		}

		if (!bAdded)
		{
			arTabs.Add(pTab);
		}
	}

	if (arTabs.GetSize() == 0)
	{
		return;
	}

	// Sort by context ID:
	for (i = 0; i < arTabs.GetSize(); i++)
	{
		BOOL bAdded = FALSE;

		for (int j = 0; !bAdded && j < arTabsDest.GetSize(); j++)
		{
			if (arTabsDest[j]->GetContextID() > arTabs[i]->GetContextID())
			{
				arTabsDest.InsertAt(j, arTabs[i]);
				bAdded = TRUE;
			}
		}

		if (!bAdded)
		{
			arTabsDest.Add(arTabs[i]);
		}
	}
}
//*************************************************************************************************************
void CBCGPRibbonCustomizationData::SetPanelIndex(CBCGPRibbonPanel* pPanel, int nIndex)
{
	ASSERT_VALID(pPanel);

	if (nIndex == HIDDEN_INDEX)
	{
		ASSERT(FALSE);	// Don't move hidden panels
		return;
	}

	int nPrevIndex = GetPanelIndex(pPanel);
	if (nPrevIndex < 0)
	{
		ASSERT(FALSE);
		return;
	}

	if (nPrevIndex == HIDDEN_INDEX)
	{
		ASSERT(FALSE);	// Don't move hidden panels
		return;
	}

	if (nIndex == nPrevIndex)
	{
		return;
	}

	DWORD dw = GetPanelLocation(*pPanel);
	if (dw == (DWORD)-1)
	{
		ASSERT(FALSE);
		return;
	}
	
	m_mapPanelIndexes.SetAt(dw, nIndex);

	CBCGPRibbonCategory* pParent = pPanel->GetParentCategory();
	ASSERT_VALID(pParent);

	for (int i = 0; i < pParent->GetPanelCount(); i++)
	{
		CBCGPRibbonPanel* pCurPanel = pParent->GetPanel(i);
		ASSERT_VALID(pCurPanel);

		if (pCurPanel == pPanel)
		{
			continue;
		}

		int nCurIndex = GetPanelIndex(pCurPanel);
		if (nCurIndex < 0)
		{
			ASSERT(FALSE);
			continue;
		}

		if (nCurIndex == HIDDEN_INDEX)
		{
			continue;
		}

		DWORD dwCurr = GetPanelLocation(*pCurPanel);
		if (dwCurr == (DWORD)-1)
		{
			ASSERT(FALSE);
		}

		if (nIndex > nPrevIndex)
		{
			if (nCurIndex > nPrevIndex && nCurIndex <= nIndex)
			{
				m_mapPanelIndexes.SetAt(dwCurr, nCurIndex - 1);
			}
		}
		else
		{
			if (nCurIndex >= nIndex && nCurIndex < nPrevIndex)
			{
				m_mapPanelIndexes.SetAt(dwCurr, nCurIndex + 1);
			}
		}
	}
}
//*************************************************************************************************************
int CBCGPRibbonCustomizationData::GetPanelIndex(const CBCGPRibbonPanel* pPanel) const
{
	ASSERT_VALID(pPanel);

	DWORD dw = GetPanelLocation(*pPanel);
	if (dw == (DWORD)-1)
	{
		return -1;
	}

	int nCurrIndex = -1;
	m_mapPanelIndexes.Lookup(dw, nCurrIndex);

	return nCurrIndex;
}
//*************************************************************************************************************
void CBCGPRibbonCustomizationData::PreparePanelsArray(const CArray<CBCGPRibbonPanel*,CBCGPRibbonPanel*>& arPanelsSrc, CArray<CBCGPRibbonPanel*,CBCGPRibbonPanel*>& arPanelsDest)
{
	arPanelsDest.RemoveAll();

	if (arPanelsSrc.GetSize() == 0)
	{
		return;
	}

	int i = 0;

	// Check for indexes:
	BOOL bRebuildIndexes = FALSE;

	for (i = 0; i < arPanelsSrc.GetSize(); i++)
	{
		CBCGPRibbonPanel* pPanelSrc = arPanelsSrc[i];
		ASSERT_VALID(pPanelSrc);

		int nPanelIndex = GetPanelIndex(pPanelSrc);
		if (nPanelIndex < 0)
		{
			bRebuildIndexes = TRUE;
			break;
		}
	}

	if (bRebuildIndexes)
	{
		int nIndex = 0;

		for (i = 0; i < arPanelsSrc.GetSize(); i++)
		{
			CBCGPRibbonPanel* pPanel = arPanelsSrc[i];
			ASSERT_VALID(pPanel);

			DWORD dw = GetPanelLocation(*pPanel);
			if (dw == (DWORD)-1)
			{
				ASSERT(FALSE);
				continue;
			}
			
			if (IsPanelHidden(pPanel))
			{
				m_mapPanelIndexes.SetAt(dw, HIDDEN_INDEX);
			}
			else
			{
				m_mapPanelIndexes.SetAt(dw, nIndex++);
			}
		}
	}

	for (i = 0; i < arPanelsSrc.GetSize(); i++)
	{
		CBCGPRibbonPanel* pPanel = arPanelsSrc[i];
		ASSERT_VALID(pPanel);

		int nPanelIndexSrc = GetPanelIndex(pPanel);

		BOOL bAdded = FALSE;

		for (int j = 0; j < arPanelsDest.GetSize(); j++)
		{
			int nPanelIndexDest = GetPanelIndex(arPanelsDest[j]);
			if (nPanelIndexSrc < nPanelIndexDest)
			{
				arPanelsDest.InsertAt(j, pPanel);
				bAdded = TRUE;
				break;
			}
		}

		if (!bAdded)
		{
			arPanelsDest.Add(pPanel);
		}
	}

/*	for (i = 0; i < arPanelsDest.GetSize(); i++)
	{
		CBCGPRibbonPanel* pPanel = arPanelsDest[i];
		ASSERT_VALID(pPanel);

		TRACE("%s %d\n", pPanel->GetName(), GetPanelIndex(pPanel));
	}

	TRACE("-----------------------------------------\n");*/
}
//*************************************************************************************************************
void CBCGPRibbonCustomizationData::AddCustomTab(const CBCGPRibbonCategory& tab, int nIndex, UINT nContextID)
{
	CBCGPRibbonCustomCategory* pNewTab = new CBCGPRibbonCustomCategory(tab);
	pNewTab->m_uiContextID = nContextID;

	int nKey = tab.GetKey();
	if (nKey <= 0)
	{
		ASSERT(FALSE);
		return;
	}

	m_arCustomTabs.Add(pNewTab);
	m_mapTabIndexes.SetAt(nKey, nIndex);

	CBCGPRibbonBar* pParent = tab.GetParentRibbonBar();
	ASSERT_VALID(pParent);
	
	for (int i = 0; i < pParent->GetCategoryCount(); i++)
	{
		CBCGPRibbonCategory* pCurTab = pParent->GetCategory(i);
		ASSERT_VALID(pCurTab);
		
		if (pCurTab == &tab || pCurTab->IsPrintPreview())
		{
			continue;
		}
		
		int nCurIndex = GetTabIndex(pCurTab);
		if (nCurIndex < 0)
		{
			continue;
		}
		
		int nKeyCurr = pCurTab->GetKey();
		if (nKeyCurr <= 0)
		{
			ASSERT(FALSE);
			continue;
		}
		
		if (nIndex <= nCurIndex)
		{
			m_mapTabIndexes.SetAt(nKeyCurr, nCurIndex + 1);
		}
	}
}
//*************************************************************************************************************
void CBCGPRibbonCustomizationData::AddCustomPanel(const CBCGPRibbonPanel& panel, int nIndex)
{
	DWORD dwLocation = GetPanelLocation(panel);
	if (dwLocation == (DWORD)-1)
	{
		ASSERT(FALSE);
		return;
	}

	m_mapCustomPanels.SetAt(dwLocation, new CBCGPRibbonCustomPanel(panel, nIndex));
	m_mapPanelIndexes.SetAt(dwLocation, nIndex);

	CBCGPRibbonCategory* pParent = panel.GetParentCategory();
	ASSERT_VALID(pParent);
	
	for (int i = 0; i < pParent->GetPanelCount(); i++)
	{
		CBCGPRibbonPanel* pCurPanel = pParent->GetPanel(i);
		ASSERT_VALID(pCurPanel);
		
		if (pCurPanel == &panel || pCurPanel->m_bToBeDeleted)
		{
			continue;
		}
		
		int nCurIndex = GetPanelIndex(pCurPanel);
		if (nCurIndex < 0)
		{
			ASSERT(FALSE);
			continue;
		}
		
		DWORD dwCurr = GetPanelLocation(*pCurPanel);
		if (dwCurr == (DWORD)-1)
		{
			ASSERT(FALSE);
			continue;
		}
		
		if (nIndex <= nCurIndex && nCurIndex != HIDDEN_INDEX)
		{
			m_mapPanelIndexes.SetAt(dwCurr, nCurIndex + 1);
		}
	}
}
//*************************************************************************************************************
CBCGPRibbonCustomCategory* CBCGPRibbonCustomizationData::FindCustomTab(int nKey) const
{
	for (int i = 0; i < m_arCustomTabs.GetSize(); i++)
	{
		CBCGPRibbonCustomCategory* pTab = m_arCustomTabs[i];
		ASSERT_VALID(pTab);

		if (pTab->m_nKey == nKey)
		{
			return pTab;
		}
	}

	return NULL;
}
//*************************************************************************************************************
CBCGPRibbonCustomPanel* CBCGPRibbonCustomizationData::FindCustomPanel(const CBCGPRibbonPanel* pPanel) const
{
	ASSERT_VALID(pPanel);

	if (!pPanel->IsCustom())
	{
		return NULL;
	}

	CBCGPRibbonCategory* pTab = pPanel->GetParentCategory();
	if (pTab != NULL)
	{
		ASSERT_VALID(pTab);
		
		if (pTab->IsCustom())
		{
			CBCGPRibbonCustomPanel* pCustomPanel = FindCustomPanel(pTab->GetKey(), pPanel->GetKey());
			if (pCustomPanel != NULL)
			{
				return pCustomPanel;
			}
		}
	}

	return FindCustomPanel(pPanel->GetKey());
}
//*************************************************************************************************************
CBCGPRibbonCustomPanel* CBCGPRibbonCustomizationData::FindCustomPanel(int nTabKey, int nPanelKey) const
{
	for (int i = 0; i < m_arCustomTabs.GetSize(); i++)
	{
		CBCGPRibbonCustomCategory* pTab = m_arCustomTabs[i];
		ASSERT_VALID(pTab);

		if (pTab->m_nKey == nTabKey)
		{
			CBCGPRibbonCustomPanel* pCustomPanel = pTab->FindCustomPanel(nPanelKey);
			if (pCustomPanel != NULL)
			{
				return pCustomPanel;
			}
		}
	}

	return NULL;
}
//*************************************************************************************************************
CBCGPRibbonCustomPanel* CBCGPRibbonCustomizationData::FindCustomPanel(int nPanelKey) const
{
	if (nPanelKey <= 0)
	{
		ASSERT(FALSE);
		return NULL;
	}

	for (POSITION pos = m_mapCustomPanels.GetStartPosition (); pos != NULL;)
	{
		DWORD dwLocation = 0;
		CBCGPRibbonCustomPanel* pCustomPanel = NULL;

		m_mapCustomPanels.GetNextAssoc(pos, dwLocation, pCustomPanel);
		ASSERT_VALID(pCustomPanel);

		if (pCustomPanel->m_nKey == nPanelKey)
		{
			return pCustomPanel;
		}
	}

	for (int i = 0; i < m_arCustomTabs.GetSize(); i++)
	{
		CBCGPRibbonCustomCategory* pTab = m_arCustomTabs[i];
		ASSERT_VALID(pTab);

		CBCGPRibbonCustomPanel* pCustomPanel = pTab->FindCustomPanel(nPanelKey);
		if (pCustomPanel != NULL)
		{
			return pCustomPanel;
		}
	}

	return NULL;
}
//*************************************************************************************************************
void CBCGPRibbonCustomizationData::AddCustomElement(const CBCGPRibbonPanel& panel, const CBCGPBaseRibbonElement& elem)
{
	if (!panel.IsCustom())
	{
		ASSERT(FALSE);
		return;
	}

	CBCGPRibbonCustomPanel* pCustomPanel = FindCustomPanel(&panel);
	if (pCustomPanel == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	if (pCustomPanel->FindByID(elem.GetID()) != NULL)
	{
		ASSERT(FALSE);
		return;
	}

	pCustomPanel->m_arElements.Add(new CBCGPRibbonCustomElement(elem));
}
//*************************************************************************************************************
void CBCGPRibbonCustomizationData::RemoveCustomTab(const CBCGPRibbonCategory& tab)
{
	int nKey = tab.GetKey();

	if (nKey <= 0)
	{
		ASSERT(FALSE);
		return;
	}

	RemoveTabIndex(&tab);

	for (int i = 0; i < m_arCustomTabs.GetSize(); i++)
	{
		CBCGPRibbonCustomCategory* pCustomTab = m_arCustomTabs[i];
		ASSERT_VALID(pCustomTab);

		if (pCustomTab->m_nKey == nKey)
		{
			delete pCustomTab;
			m_arCustomTabs.RemoveAt(i);
			
			return;
		}
	}
}
//*************************************************************************************************************
void CBCGPRibbonCustomizationData::RemoveCustomPanel(const CBCGPRibbonPanel& panel)
{
	int nKey = panel.GetKey();

	if (nKey <= 0)
	{
		ASSERT(FALSE);
		return;
	}

	RemovePanelIndex(&panel);

	for (POSITION pos = m_mapCustomPanels.GetStartPosition (); pos != NULL;)
	{
		DWORD dwLocation = 0;
		CBCGPRibbonCustomPanel* pCustomPanel = NULL;

		m_mapCustomPanels.GetNextAssoc(pos, dwLocation, pCustomPanel);
		ASSERT_VALID(pCustomPanel);

		if (pCustomPanel->m_nKey == nKey)
		{
			delete pCustomPanel;
			m_mapCustomPanels.RemoveKey(dwLocation);
			return;
		}
	}

	for (int i = 0; i < m_arCustomTabs.GetSize(); i++)
	{
		CBCGPRibbonCustomCategory* pTab = m_arCustomTabs[i];
		ASSERT_VALID(pTab);

		for (int j = 0; j < pTab->m_arPanels.GetSize(); j++)
		{
			CBCGPRibbonCustomPanel* pCustomPanel = pTab->m_arPanels[j];
			ASSERT_VALID(pCustomPanel);

			if (pCustomPanel->m_nKey == nKey)
			{
				delete pCustomPanel;
				pTab->m_arPanels.RemoveAt(j);
				return;
			}
		}
	}
}
//*************************************************************************************************************
void CBCGPRibbonCustomizationData::RemoveCustomElement(const CBCGPBaseRibbonElement& elem)
{
	if (elem.GetParentPanel() == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	int nPanelKey = elem.GetParentPanel()->GetKey();
	if (nPanelKey <= 0)
	{
		ASSERT(FALSE);
		return;
	}

	for (POSITION pos = m_mapCustomPanels.GetStartPosition (); pos != NULL;)
	{
		DWORD dwLocation = 0;
		CBCGPRibbonCustomPanel* pCustomPanel = NULL;

		m_mapCustomPanels.GetNextAssoc(pos, dwLocation, pCustomPanel);
		ASSERT_VALID(pCustomPanel);

		if (pCustomPanel->m_nKey == nPanelKey && pCustomPanel->RemoveElementByID(elem.GetID()))
		{
			return;
		}
	}

	for (int i = 0; i < m_arCustomTabs.GetSize(); i++)
	{
		CBCGPRibbonCustomCategory* pTab = m_arCustomTabs[i];
		ASSERT_VALID(pTab);

		for (int j = 0; j < pTab->m_arPanels.GetSize(); j++)
		{
			CBCGPRibbonCustomPanel* pCustomPanel = pTab->m_arPanels[j];
			ASSERT_VALID(pCustomPanel);

			if (pCustomPanel->m_nKey == nPanelKey && pCustomPanel->RemoveElementByID(elem.GetID()))
			{
				return;
			}
		}
	}

	ASSERT(FALSE);
}
//*************************************************************************************************************
LPCTSTR CBCGPRibbonCustomizationData::GetElementName(CBCGPBaseRibbonElement* pElem)
{
	ASSERT_VALID(pElem);

	CBCGPRibbonCustomElement* pCustomElem = FindCustomElement(pElem);
	if (pCustomElem == NULL)
	{
		return NULL;
	}

	return pCustomElem->m_strName;
}
//*************************************************************************************************************
int CBCGPRibbonCustomizationData::GetElementImage(CBCGPBaseRibbonElement* pElem)
{
	ASSERT_VALID(pElem);

	CBCGPRibbonCustomElement* pCustomElem = FindCustomElement(pElem);
	if (pCustomElem == NULL)
	{
		return -1;
	}

	return pCustomElem->m_nCustomIcon;
}
//*************************************************************************************************************
void CBCGPRibbonCustomizationData::SetElementImage(CBCGPBaseRibbonElement* pElem, int nImageIndex)
{
	ASSERT_VALID(pElem);

	CBCGPRibbonCustomElement* pCustomElem = FindCustomElement(pElem);
	if (pCustomElem == NULL)
	{
		return;
	}

	pCustomElem->m_nCustomIcon = nImageIndex;
}
//*************************************************************************************************************
void CBCGPRibbonCustomizationData::RemoveTabIndex(const CBCGPRibbonCategory* pTab)
{
	ASSERT_VALID(pTab);

	int nIndex = GetTabIndex(pTab);
	ASSERT(nIndex >= 0);
	
	CBCGPRibbonBar* pParent = pTab->GetParentRibbonBar();
	ASSERT_VALID(pParent);
	
	for (int i = 0; i < pParent->GetCategoryCount(); i++)
	{
		CBCGPRibbonCategory* pCurTab = pParent->GetCategory(i);
		ASSERT_VALID(pCurTab);
		
		int nCurIndex = GetTabIndex(pCurTab);
		if (nCurIndex < 0)
		{
			continue;
		}
		
		if (nCurIndex > nIndex)
		{
			m_mapTabIndexes.SetAt(pCurTab->GetKey(), nCurIndex - 1);
		}
	}
}
//*************************************************************************************************************
void CBCGPRibbonCustomizationData::RemovePanelIndex(const CBCGPRibbonPanel* pPanel)
{
	ASSERT_VALID(pPanel);

	int nIndex = GetPanelIndex(pPanel);
	ASSERT(nIndex >= 0);
	
	CBCGPRibbonCategory* pParent = pPanel->GetParentCategory();
	ASSERT_VALID(pParent);
	
	for (int i = 0; i < pParent->GetPanelCount(); i++)
	{
		CBCGPRibbonPanel* pCurPanel = pParent->GetPanel(i);
		ASSERT_VALID(pCurPanel);
		
		int nCurIndex = GetPanelIndex(pCurPanel);
		if (nCurIndex < 0)
		{
			ASSERT(FALSE);
			continue;
		}
		
		if (nCurIndex == HIDDEN_INDEX)
		{
			continue;
		}
		
		DWORD dwCurr = GetPanelLocation(*pCurPanel);
		if (dwCurr == (DWORD)-1)
		{
			ASSERT(FALSE);
		}
		
		if (nCurIndex > nIndex)
		{
			m_mapPanelIndexes.SetAt(dwCurr, nCurIndex - 1);
		}
	}
}
//*************************************************************************************************************
void CBCGPRibbonCustomizationData::PrepareElementsArray(const CBCGPRibbonPanel* pPanel, CArray<CBCGPBaseRibbonElement*, CBCGPBaseRibbonElement*>& arElements)
{
	ASSERT_VALID(pPanel);

	CBCGPRibbonCustomPanel* pCustomPanel = FindCustomPanel(pPanel);
	if (pCustomPanel == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	for (int i = 0; i < pCustomPanel->m_arElements.GetSize(); i++)
	{
		CBCGPRibbonCustomElement* pCustomElement = pCustomPanel->m_arElements[i];
		ASSERT_VALID(pCustomElement);

		CBCGPBaseRibbonElement* pElem = pPanel->FindByID(pCustomElement->m_nID, FALSE);
		if (pElem != NULL)
		{
			arElements.Add(pElem);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////
// CBCGPRibbonCustomCategory

CBCGPRibbonCustomCategory::CBCGPRibbonCustomCategory()
{
	m_nKey = 0;
	m_uiContextID = 0;
}
//*************************************************************************************************************
CBCGPRibbonCustomCategory::CBCGPRibbonCustomCategory(const CBCGPRibbonCustomCategory& src)
{
	m_strName = src.m_strName;
	m_nKey = src.m_nKey;
	m_uiContextID = src.m_uiContextID;

	for (int i = 0; i < src.m_arPanels.GetSize(); i++)
	{
		m_arPanels.Add(new CBCGPRibbonCustomPanel(*src.m_arPanels[i]));
	}
}
//*************************************************************************************************************
CBCGPRibbonCustomCategory::CBCGPRibbonCustomCategory(const CBCGPRibbonCategory& src)
{
	m_strName = src.GetName();
	m_nKey = src.GetKey();
	m_uiContextID = src.GetContextID();

	for (int i = 0; i < src.GetPanelCount(); i++)
	{
		CBCGPRibbonPanel* pPanel = ((CBCGPRibbonCategory&)src).GetPanel(i);
		m_arPanels.Add(new CBCGPRibbonCustomPanel(*pPanel, i));
	}
}
//*************************************************************************************************************
CBCGPRibbonCustomCategory::~CBCGPRibbonCustomCategory()
{
	for (int i = 0; i < m_arPanels.GetSize(); i++)
	{
		delete m_arPanels[i];
	}
}
//*************************************************************************************************************
CBCGPRibbonCategory* CBCGPRibbonCustomCategory::CreateRibbonCategory(CBCGPRibbonBar* pRibbonBar)
{
	ASSERT_VALID(pRibbonBar);

	CBCGPRibbonCategory* pCategory = new CBCGPRibbonCategory(pRibbonBar, m_strName, 0, 0);
	pCategory->m_nKey = m_nKey;
	pCategory->m_uiContextID = m_uiContextID;
	pCategory->m_bIsVisible = m_uiContextID == 0;
	pCategory->m_bIsCustom = TRUE;

	if (m_uiContextID != 0)
	{
		pCategory->SetTabColor(pRibbonBar->GetContextColor(m_uiContextID));
	}
	
	for (int i = 0; i < m_arPanels.GetSize(); i++)
	{
		CBCGPRibbonPanel* pPanel = m_arPanels[i]->CreateRibbonPanel(pRibbonBar, pCategory);
		if (pPanel != NULL)
		{
			ASSERT_VALID(pPanel);
			pCategory->m_arPanels.Add(pPanel);
		}
	}

	return pCategory;
}
//*************************************************************************************************************
CBCGPRibbonCustomPanel* CBCGPRibbonCustomCategory::FindCustomPanel(int nKey) const
{
	for (int i = 0; i < m_arPanels.GetSize(); i++)
	{
		CBCGPRibbonCustomPanel* pCustomPanel = m_arPanels[i];
		ASSERT_VALID(pCustomPanel);

		if (pCustomPanel->m_nKey == nKey)
		{
			return pCustomPanel;
		}
	}

	return NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// CBCGPRibbonCustomPanel

CBCGPRibbonCustomPanel::CBCGPRibbonCustomPanel(const CBCGPRibbonCustomPanel& src)
{
	m_nIndex = src.m_nIndex;
	m_strName = src.m_strName;
	m_nKey = src.m_nKey;
	m_dwOriginal = src.m_dwOriginal;
	m_nID = src.m_nID;

	for (int i = 0; i < src.m_arElements.GetSize(); i++)
	{
		m_arElements.Add(new CBCGPRibbonCustomElement(*src.m_arElements[i]));
	}
}
//*************************************************************************************************************
CBCGPRibbonCustomPanel::CBCGPRibbonCustomPanel()
{
	m_nIndex = -1;
	m_nKey = 0;
	m_dwOriginal = (DWORD)-1;
	m_nID = 0;
}
//*************************************************************************************************************
CBCGPRibbonCustomPanel::CBCGPRibbonCustomPanel(const CBCGPRibbonPanel& src, int nIndex)
{
	m_nIndex = nIndex;
	m_strName = src.GetName();
	m_nKey = src.GetKey();
	m_dwOriginal = (DWORD)-1;
	m_nID = src.m_btnDefault.GetID();

	for (int i = 0; i < src.GetCount(); i++)
	{
		m_arElements.Add(new CBCGPRibbonCustomElement(*src.GetElement(i)));
	}

	if (src.m_pOriginal != NULL)
	{
		ASSERT_VALID(src.m_pOriginal);
		
		m_dwOriginal = CBCGPRibbonCustomizationData::GetPanelLocation(*src.m_pOriginal);
		ASSERT(m_dwOriginal != (DWORD)-1);
	}
}
//*************************************************************************************************************
CBCGPRibbonCustomPanel::~CBCGPRibbonCustomPanel()
{
	for (int i = 0; i < m_arElements.GetSize(); i++)
	{
		CBCGPRibbonCustomElement* pCustomElement = m_arElements[i];
		ASSERT_VALID(pCustomElement);

		delete pCustomElement;
	}
}
//*************************************************************************************************************
CBCGPRibbonPanel* CBCGPRibbonCustomPanel::CreateRibbonPanel(CBCGPRibbonBar* pRibbonBar, CBCGPRibbonCategory* pCategory)
{
	ASSERT_VALID(pRibbonBar);

	if ((int)m_dwOriginal > 0)
	{
		int nOriginCatKey = LOWORD(m_dwOriginal);
		int nOriginPanelKey = HIWORD(m_dwOriginal);

		CBCGPRibbonPanel* pOriginalPanel = pRibbonBar->FindPanel(nOriginCatKey, nOriginPanelKey);

		if (pOriginalPanel == NULL)
		{
			ASSERT(FALSE);
			return NULL;
		}

		ASSERT_VALID(pOriginalPanel);
		return pOriginalPanel->CreateCustomCopy(pCategory);
	}

	CBCGPRibbonPanel* pPanel = new CBCGPRibbonPanel(m_strName);

	pPanel->m_pParent = pCategory;
	pPanel->m_nKey = m_nKey;
	pPanel->m_btnDefault.SetID(m_nID);

	for (int i = 0; i < m_arElements.GetSize(); i++)
	{
		CBCGPRibbonCustomElement* pCustomElement = m_arElements[i];
		ASSERT_VALID(pCustomElement);

		CBCGPBaseRibbonElement* pElem = pCustomElement->CreateRibbonElement(pRibbonBar);
		if (pElem != NULL)
		{
			ASSERT_VALID(pElem);

			if (pCategory != NULL)
			{
				pElem->SetParentCategory(pCategory);
			}

			pPanel->Add(pElem);
		}
	}

	pPanel->SetCustom();

	pRibbonBar->m_nNextPanelKey = max(pRibbonBar->m_nNextPanelKey, m_nKey + 1);
	return pPanel;
}
//*************************************************************************************************************
CBCGPRibbonCustomElement* CBCGPRibbonCustomPanel::FindByID(UINT nID)
{
	for (int i = 0; i < m_arElements.GetSize(); i++)
	{
		CBCGPRibbonCustomElement* pCustomElement = m_arElements[i];
		ASSERT_VALID(pCustomElement);

		if (pCustomElement->m_nID == nID)
		{
			return pCustomElement;
		}
	}

	return NULL;
}
//*************************************************************************************************************
BOOL CBCGPRibbonCustomPanel::RemoveElementByID(UINT nID)
{
	for (int i = 0; i < m_arElements.GetSize(); i++)
	{
		CBCGPRibbonCustomElement* pCustomElement = m_arElements[i];
		ASSERT_VALID(pCustomElement);

		if (pCustomElement->m_nID == nID)
		{
			delete pCustomElement;
			m_arElements.RemoveAt(i);

			return TRUE;
		}
	}

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// CBCGPRibbonCustomElement

CBCGPRibbonCustomElement::CBCGPRibbonCustomElement(const CBCGPRibbonCustomElement& src)
{
	m_nID = src.m_nID;
	m_strName = src.m_strName;
	m_strName.Remove(_T('&'));

	m_nCustomIcon = src.m_nCustomIcon;
}
//*************************************************************************************************************
CBCGPRibbonCustomElement::CBCGPRibbonCustomElement()
{
	m_nID = 0;
	m_nCustomIcon = -1;
}
//*************************************************************************************************************
CBCGPRibbonCustomElement::CBCGPRibbonCustomElement(const CBCGPBaseRibbonElement& src)
{
	m_strName = src.GetText();
	m_strName.Remove(_T('&'));

	m_nID = src.GetID();
	m_nCustomIcon = -1;
}
//*************************************************************************************************************
CBCGPRibbonCustomElement::~CBCGPRibbonCustomElement()
{
}
//*************************************************************************************************************
CBCGPBaseRibbonElement* CBCGPRibbonCustomElement::CreateRibbonElement(CBCGPRibbonBar* pRibbonBar)
{
	ASSERT_VALID(pRibbonBar);

	CBCGPBaseRibbonElement* pSrcElem = pRibbonBar->FindByID(m_nID, FALSE, TRUE);
	if (pSrcElem == NULL)
	{
		return NULL;
	}

	ASSERT_VALID (pSrcElem);

	CBCGPBaseRibbonElement* pElem = pSrcElem->CreateCustomCopy();
	ASSERT_VALID(pElem);

	pElem->ApplyCustomizationData(*this);

	return pElem;
}

#endif // BCGP_EXCLUDE_RIBBON
