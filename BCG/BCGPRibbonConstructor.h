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
// BCGPRibbonConstructor.h: interface for the CBCGPRibbonConstrucor class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPRIBBONCONSTRUCTOR_H__337FFC0F_4C9B_4115_9215_DCAACE3C1494__INCLUDED_)
#define AFX_BCGPRIBBONCONSTRUCTOR_H__337FFC0F_4C9B_4115_9215_DCAACE3C1494__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGPRibbonInfo.h"

#ifndef BCGP_EXCLUDE_RIBBON

class CBCGPRibbonBar;
class CBCGPRibbonStatusBar;

class CBCGPRibbonConstructor  
{
public:
	CBCGPRibbonConstructor(const CBCGPRibbonInfo& info);
	virtual ~CBCGPRibbonConstructor();

public:
	virtual void ConstructRibbonBar (CBCGPRibbonBar& bar) const;
	virtual void ConstructStatusBar (CBCGPRibbonStatusBar& bar) const;

protected:
	virtual CBCGPBaseRibbonElement* CreateElement (const CBCGPRibbonInfo::XElement& info) const;
	virtual CBCGPRibbonPanel* CreatePanel (CBCGPRibbonCategory& category, const CBCGPRibbonInfo::XPanel& info) const;
	virtual CBCGPRibbonCategory* CreateCategory (CBCGPRibbonBar& bar, const CBCGPRibbonInfo::XCategory& info) const;
	virtual CBCGPRibbonCategory* CreateCategoryContext (CBCGPRibbonBar& bar, const CBCGPRibbonInfo::XContext& infoContext, const CBCGPRibbonInfo::XCategory& info) const;
	virtual CBCGPRibbonMainPanel* CreateCategoryMain (CBCGPRibbonBar& bar, const CBCGPRibbonInfo::XCategoryMain& info) const;
	virtual CBCGPRibbonBackstageViewPanel* CreateCategoryBackstage (CBCGPRibbonBar& bar, const CBCGPRibbonInfo::XCategoryBackstage& info) const;
	virtual CBCGPRibbonMainButton* CreateMainButton (CBCGPRibbonBar& bar) const;
	
	virtual void SetID (CBCGPBaseRibbonElement& element, const CBCGPRibbonInfo::XID& info) const;
	virtual void SetIcon (CBCGPRibbonButton& element, CBCGPBaseRibbonElement::RibbonImageType type, const CBCGPToolBarImages& images, BOOL bLargeIcon = FALSE) const;
	virtual void ConstructBaseElement (CBCGPBaseRibbonElement& element, const CBCGPRibbonInfo::XElement& info, BOOL bSubItems = TRUE) const;
	virtual void ConstructElement (CBCGPBaseRibbonElement& element, const CBCGPRibbonInfo::XElement& info) const;
	virtual void ConstructQATElements (CBCGPRibbonBar& bar, const CBCGPRibbonInfo::XRibbonBar& info) const;
	virtual void ConstructTabElements (CBCGPRibbonBar& bar, const CBCGPRibbonInfo::XRibbonBar& info) const;
	virtual void ConstructPanel (CBCGPRibbonPanel& panel, const CBCGPRibbonInfo::XPanel& info) const;
	virtual void ConstructCategory (CBCGPRibbonCategory& category, const CBCGPRibbonInfo::XCategory& info) const;
	virtual void ConstructCategoryMain (CBCGPRibbonBar& bar, const CBCGPRibbonInfo::XCategoryMain& info) const;
	virtual void ConstructCategoryBackstage (CBCGPRibbonBar& bar, const CBCGPRibbonInfo::XCategoryBackstage& info) const;

	const CBCGPRibbonInfo& GetInfo () const
	{
		return m_Info;
	}

private:
	const CBCGPRibbonInfo&	m_Info;
};


class CBCGPRibbonCustomizationConstructor  
{
public:
	CBCGPRibbonCustomizationConstructor(const CBCGPRibbonCustomizationInfo& info);
	virtual ~CBCGPRibbonCustomizationConstructor();

public:
	virtual void Construct (CBCGPRibbonCustomizationData& data) const;

protected:
	virtual void ConstructDataCategory (CBCGPRibbonCustomizationData& data, const CBCGPRibbonCustomizationInfo::XDataCategory& info) const;
	virtual void ConstructDataPanel (CBCGPRibbonCustomizationData& data, const CBCGPRibbonCustomizationInfo::XDataPanel& info) const;

	virtual void ConstructCustomCategory (CBCGPRibbonCustomCategory& category, const CBCGPRibbonCustomizationInfo::XCustomCategory& info) const;
	virtual void ConstructCustomPanel (CBCGPRibbonCustomPanel& panel, const CBCGPRibbonCustomizationInfo::XCustomPanel& info) const;	

	const CBCGPRibbonCustomizationInfo& GetInfo () const
	{
		return m_Info;
	}

private:
	const CBCGPRibbonCustomizationInfo&	m_Info;
};

#endif // BCGP_EXCLUDE_RIBBON

#endif // !defined(AFX_BCGPRIBBONCONSTRUCTOR_H__337FFC0F_4C9B_4115_9215_DCAACE3C1494__INCLUDED_)
