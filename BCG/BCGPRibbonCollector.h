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
// BCGPRibbonCollector.h: interface for the CBCGPRibbonCollector class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPRIBBONCOLLECTOR_H__CE1B238C_B6B9_41A7_8E28_4C00184CD8C7__INCLUDED_)
#define AFX_BCGPRIBBONCOLLECTOR_H__CE1B238C_B6B9_41A7_8E28_4C00184CD8C7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGPRibbonInfo.h"
#include "BCGPRibbonBar.h"
#include "BCGPRibbonStatusBar.h"
#include "BCGPBaseRibbonElement.h"
#include "BCGPRibbonPanel.h"
#include "BCGPRibbonCategory.h"
#include "BCGPRibbonMainPanel.h"

#ifndef BCGP_EXCLUDE_RIBBON

class CBCGPRibbonCollector  
{
public:
	enum XFlags
	{
		e_CollectUpdatedToolInfo = 0x01, // update tool tip info
		e_CollectRibbonBarIcons  = 0x02, // add icons of the panels or tab elements to the ribbon image
		e_CollectStatusBarIcons  = 0x04, // add icons of the panes to the status bar image
		e_CollectGroupImages     = 0x08, // add image of each group to the small image of category
		e_CollectHiddenElements  = 0x10, // collect hidden elements of the category
		e_CollectAll             = 0x1F
	};

public:
	CBCGPRibbonCollector(CBCGPRibbonInfo& info, DWORD dwFlags = e_CollectAll);
	virtual ~CBCGPRibbonCollector();

public:
	virtual void CollectRibbonBar (const CBCGPRibbonBar& bar);
	virtual void CollectStatusBar (const CBCGPRibbonStatusBar& bar);

protected:
	virtual CBCGPRibbonInfo::XElement* CollectElement(const CBCGPBaseRibbonElement& element);

	virtual void GetID (const CBCGPBaseRibbonElement& element, CBCGPRibbonInfo::XID& info);
	virtual void GetRibbonBarImages(const CBCGPRibbonBar& bar, CBCGPRibbonInfo::XRibbonBar& info);
	virtual void GetStatusBarImages(const CBCGPRibbonStatusBar& bar, CBCGPRibbonInfo::XStatusBar& info);
	virtual void GetCategoryImages(const CBCGPRibbonCategory& category, CBCGPRibbonInfo::XCategory& info);
	virtual void GetCategoryImages(const CBCGPRibbonCategory& category, CBCGPRibbonInfo::XImage& infoSmall, CBCGPRibbonInfo::XImage& infoLarge);
	virtual void GetMainButtonImages(const CBCGPRibbonMainButton& element, CBCGPRibbonInfo::XElementButtonMain& info);
	virtual void GetElementImages(const CBCGPBaseRibbonElement& element, CBCGPRibbonInfo::XImage& info);

	virtual void CollectBaseElement (const CBCGPBaseRibbonElement& element, CBCGPRibbonInfo::XElement& info, BOOL bSubItems = TRUE);
	virtual void CollectElement(const CBCGPBaseRibbonElement& element, CBCGPRibbonInfo::XElement& info);
	virtual void CollectQATElements (const CBCGPRibbonBar& bar, CBCGPRibbonInfo::XRibbonBar& info);
	virtual void CollectPanel(const CBCGPRibbonPanel& panel, CBCGPRibbonInfo::XPanel& info);
	virtual void CollectCategory(const CBCGPRibbonCategory& category, CBCGPRibbonInfo::XCategory& info);
	virtual void CollectCategoryMain(const CBCGPRibbonBar& bar, CBCGPRibbonInfo::XCategoryMain& info);
	virtual void CollectCategoryBackstage(const CBCGPRibbonBar& bar, CBCGPRibbonInfo::XCategoryBackstage& info);
	virtual void CollectRibbonBar (const CBCGPRibbonBar& bar, CBCGPRibbonInfo::XRibbonBar& info);
	virtual void CollectStatusBar (const CBCGPRibbonStatusBar& bar, CBCGPRibbonInfo::XStatusBar& info);

	CBCGPRibbonInfo& GetInfo ()
	{
		return m_Info;
	}

	DWORD GetFlags () const
	{
		return m_dwFlags;
	}

private:
	CBCGPRibbonInfo&	m_Info;
	DWORD				m_dwFlags;
};

class CBCGPRibbonCustomizationCollector  
{
public:
	CBCGPRibbonCustomizationCollector(CBCGPRibbonCustomizationInfo& info, DWORD dwFlags = 0);
	virtual ~CBCGPRibbonCustomizationCollector();

public:
	virtual void Collect (const CBCGPRibbonCustomizationData& data);

protected:
	virtual void CollectDataCategory (const CBCGPRibbonCustomizationData& data, CBCGPRibbonCustomizationInfo::XDataCategory& info);
	virtual void CollectDataPanel (const CBCGPRibbonCustomizationData& data, CBCGPRibbonCustomizationInfo::XDataPanel& info);
	
	virtual void CollectCustomCategory (const CBCGPRibbonCustomCategory& category, CBCGPRibbonCustomizationInfo::XCustomCategory& info);
	virtual void CollectCustomPanel (const CBCGPRibbonCustomPanel& panel, CBCGPRibbonCustomizationInfo::XCustomPanel& info);

	CBCGPRibbonCustomizationInfo& GetInfo ()
	{
		return m_Info;
	}

	DWORD GetFlags () const
	{
		return m_dwFlags;
	}

private:
	CBCGPRibbonCustomizationInfo&	m_Info;
	DWORD							m_dwFlags;
};

#endif // BCGP_EXCLUDE_RIBBON

#endif // !defined(AFX_BCGPRIBBONCOLLECTOR_H__CE1B238C_B6B9_41A7_8E28_4C00184CD8C7__INCLUDED_)
