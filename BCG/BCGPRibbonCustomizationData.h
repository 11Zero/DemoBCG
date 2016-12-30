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
// BCGPRibbonCustomizationData.h: interface for the CBCGPRibbonCustomizationData class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPRIBBONCUSTOMIZATIONDATA_H__7BA32132_EE88_43DE_BD5D_1356522091C2__INCLUDED_)
#define AFX_BCGPRIBBONCUSTOMIZATIONDATA_H__7BA32132_EE88_43DE_BD5D_1356522091C2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"

#ifndef BCGP_EXCLUDE_RIBBON

class CBCGPRibbonCategory;
class CBCGPRibbonBar;
class CBCGPRibbonPanel;
class CBCGPRibbonCustomCategory;
class CBCGPRibbonCustomPanel;
class CBCGPRibbonCustomElement;
class CBCGPBaseRibbonElement;

class BCGCBPRODLLEXPORT CBCGPRibbonCustomizationData : public CObject  
{
	friend class CBCGPRibbonCustomizeRibbonPage;
	friend class CBCGPRibbonCustomizationCollector;
	friend class CBCGPRibbonCustomizationConstructor;
	friend class CBCGPRibbonCustomPanel;

// Construction
public:
	CBCGPRibbonCustomizationData();
	virtual ~CBCGPRibbonCustomizationData();

// Attributes:
public:
	DWORD GetVersionStamp() const { return m_VersionStamp; }

	BOOL IsTabHidden(const CBCGPRibbonCategory* pTab) const;
	BOOL IsPanelHidden(const CBCGPRibbonPanel* pPanel) const;

	BOOL GetTabName(const CBCGPRibbonCategory* pTab, CString& strName) const;
	BOOL GetPanelName(const CBCGPRibbonPanel* pPanel, CString& strName) const;

	int GetTabIndex(const CBCGPRibbonCategory* pTab) const;
	UINT GetTabContextID(const CBCGPRibbonCategory* pTab) const;

	int GetPanelIndex(const CBCGPRibbonPanel* pPanel) const;

	const CArray<CBCGPRibbonCustomCategory*,CBCGPRibbonCustomCategory*>& GetCustomTabs() const { return m_arCustomTabs; }
	const CMap<DWORD, DWORD, CBCGPRibbonCustomPanel*, CBCGPRibbonCustomPanel*>& GetCustomPanels() const { return m_mapCustomPanels; }

// Operations
public:
	void CopyFrom(const CBCGPRibbonCustomizationData& src);

	BOOL Apply(CBCGPRibbonBar* pRibbonBar);

	void ResetAll(CBCGPRibbonBar* pRibbonBar);
	void ResetTab(CBCGPRibbonBar* pRibbonBar, int nIndex);
	void ResetNonCustomData();

	BOOL LoadFromBuffer(LPCTSTR lpszXMLBuffer);
	BOOL SaveToBuffer(CString& strXMLBuffer);

	void ShowTab(CBCGPRibbonCategory* pTab, BOOL bShow);
	void ShowPanel(CBCGPRibbonPanel* pPanel, BOOL bShow);

	void RenameTab(CBCGPRibbonCategory* pTab, LPCTSTR lpszName);
	void RenamePanel(CBCGPRibbonPanel* pPanel, LPCTSTR lpszName);
	void RenameElement(CBCGPBaseRibbonElement* pElem, LPCTSTR lpszName);
	void SetElementImage(CBCGPBaseRibbonElement* pElem, int nImageIndex);

	int GetElelementIndex(CBCGPBaseRibbonElement* pElem, int& nTotal);

	BOOL MoveElement(CBCGPBaseRibbonElement* pElem, int nOffset);

	void SetTabIndex(const CBCGPRibbonCategory* pTab, int nIndex);
	void SetTabContextID(const CBCGPRibbonCategory* pTab, UINT nContextID);

	void SetPanelIndex(CBCGPRibbonPanel* pPanel, int nIndex);

	void PrepareTabsArray(const CArray<CBCGPRibbonCategory*,CBCGPRibbonCategory*>& arTabsSrc, CArray<CBCGPRibbonCategory*,CBCGPRibbonCategory*>& arTabsDest);
	void PreparePanelsArray(const CArray<CBCGPRibbonPanel*,CBCGPRibbonPanel*>& arPanelsSrc, CArray<CBCGPRibbonPanel*,CBCGPRibbonPanel*>& arPanelsDest);
	void PrepareElementsArray(const CBCGPRibbonPanel* pPanel, CArray<CBCGPBaseRibbonElement*, CBCGPBaseRibbonElement*>& arElements);

	void AddCustomTab(const CBCGPRibbonCategory& tab, int nIndex, UINT nContextID);
	void AddCustomPanel(const CBCGPRibbonPanel& panel, int nIndex);
	void AddCustomElement(const CBCGPRibbonPanel& panel, const CBCGPBaseRibbonElement& elem);

	void RemoveCustomTab(const CBCGPRibbonCategory& tab);
	void RemoveCustomPanel(const CBCGPRibbonPanel& panel);
	void RemoveCustomElement(const CBCGPBaseRibbonElement& elem);

	CBCGPRibbonCustomCategory* FindCustomTab(int nKey) const;
	CBCGPRibbonCustomPanel* FindCustomPanel(int nTabKey, int nPanelKey) const;
	CBCGPRibbonCustomPanel* FindCustomPanel(int nKey) const;
	CBCGPRibbonCustomPanel* FindCustomPanel(const CBCGPRibbonPanel* pPanel) const;

	CBCGPRibbonCustomElement* FindCustomElement(CBCGPBaseRibbonElement* pElem);

	int GetElementImage(CBCGPBaseRibbonElement* pElem);
	LPCTSTR GetElementName(CBCGPBaseRibbonElement* pElem);

protected:
	static DWORD GetPanelLocation(const CBCGPRibbonPanel &panel);
	void RemoveTabIndex(const CBCGPRibbonCategory* pTab);
	void RemovePanelIndex(const CBCGPRibbonPanel* pPanel);

// Attributes:
protected:
	DWORD																	m_VersionStamp;

	// Tab (category) data:
	CArray<int, int>														m_arHiddenTabs;
	CMap<int, int, int, int>												m_mapTabIndexes;
	CMap<int, int, CString, const CString&>									m_mapTabNames;
	CArray<CBCGPRibbonCustomCategory*,CBCGPRibbonCustomCategory*>			m_arCustomTabs;

	// Panel data:
	CArray<DWORD, DWORD>													m_arHiddenPanels;
	CMap<DWORD, DWORD, int, int>											m_mapPanelIndexes;
	CMap<DWORD, DWORD, CString, const CString&>								m_mapPanelNames;
	CMap<DWORD, DWORD, CBCGPRibbonCustomPanel*, CBCGPRibbonCustomPanel*>	m_mapCustomPanels;
};

//////////////////////////////////////////////////////////////////////

class CBCGPRibbonCustomCategory : public CObject
{
	friend class CBCGPRibbonCustomizationData;
	friend class CBCGPRibbonBar;
	friend class CBCGPRibbonCategory;
	friend class CBCGPRibbonCustomizationCollector;
	friend class CBCGPRibbonCustomizationConstructor;

// Construction
public:
	CBCGPRibbonCustomCategory();
	CBCGPRibbonCustomCategory(const CBCGPRibbonCustomCategory& src);
	CBCGPRibbonCustomCategory(const CBCGPRibbonCategory& src);

	virtual ~CBCGPRibbonCustomCategory();

// Operations:
public:
	CBCGPRibbonCategory* CreateRibbonCategory(CBCGPRibbonBar* pRibbonBar);
	CBCGPRibbonCustomPanel* FindCustomPanel(int nKey) const;

// Attributes:
protected:
	int															m_nKey;
	CString														m_strName;
	UINT														m_uiContextID;
	CArray<CBCGPRibbonCustomPanel*, CBCGPRibbonCustomPanel*>	m_arPanels;
};

//////////////////////////////////////////////////////////////////////

class CBCGPRibbonCustomPanel : public CObject
{
	friend class CBCGPRibbonBar;
	friend class CBCGPRibbonCustomizationData;
	friend class CBCGPRibbonCustomCategory;
	friend class CBCGPRibbonCustomizationCollector;
	friend class CBCGPRibbonCustomizationConstructor;
	friend class CBCGPRibbonPanel;

// Construction
public:
	CBCGPRibbonCustomPanel();
	CBCGPRibbonCustomPanel(const CBCGPRibbonCustomPanel& src);
	CBCGPRibbonCustomPanel(const CBCGPRibbonPanel& src, int nIndex);

	virtual ~CBCGPRibbonCustomPanel();

// Operations:
public:
	CBCGPRibbonPanel* CreateRibbonPanel(CBCGPRibbonBar* pRibbonBar, CBCGPRibbonCategory* pParent);
	CBCGPRibbonCustomElement* FindByID(UINT nID);
	BOOL RemoveElementByID(UINT nID);

// Attributes:
protected:
	UINT															m_nID;
	int																m_nKey;
	DWORD															m_dwOriginal;
	CString															m_strName;
	int																m_nIndex;
	CArray<CBCGPRibbonCustomElement*, CBCGPRibbonCustomElement*>	m_arElements;
};

//////////////////////////////////////////////////////////////////////

class CBCGPRibbonCustomElement : public CObject
{
	friend class CBCGPRibbonCustomizationData;
	friend class CBCGPRibbonCustomPanel;
	friend class CBCGPRibbonCustomizationCollector;
	friend class CBCGPRibbonCustomizationConstructor;
	friend class CBCGPRibbonPanel;
	friend class CBCGPBaseRibbonElement;

// Construction
public:
	CBCGPRibbonCustomElement();
	CBCGPRibbonCustomElement(const CBCGPRibbonCustomElement& src);
	CBCGPRibbonCustomElement(const CBCGPBaseRibbonElement& src);

	virtual ~CBCGPRibbonCustomElement();

// Operations:
public:
	CBCGPBaseRibbonElement* CreateRibbonElement(CBCGPRibbonBar* pRibbonBar);

// Attributes:
protected:
	UINT		m_nID;
	CString		m_strName;
	int			m_nCustomIcon;
};

#endif // BCGP_EXCLUDE_RIBBON

#endif // !defined(AFX_BCGPRIBBONCUSTOMIZATIONDATA_H__7BA32132_EE88_43DE_BD5D_1356522091C2__INCLUDED_)
