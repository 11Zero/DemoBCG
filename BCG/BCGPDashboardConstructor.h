//*******************************************************************************
// COPYRIGHT NOTES
// ---------------
// This is a part of the BCGControlBar Library
// Copyright (C) 1998-2010 BCGSoft Ltd.
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.
//*******************************************************************************
//
// BCGPDashboardConstructor.h: interface for the CBCGPRibbonConstrucor class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPDASHBOARDCONSTRUCTOR_H__856CE5A1_AC4D_4029_8A23_77ECDC8EEB77__INCLUDED_)
#define AFX_BCGPDASHBOARDCONSTRUCTOR_H__856CE5A1_AC4D_4029_8A23_77ECDC8EEB77__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGPGaugeInfo.h"

class CBCGPDashboard;
class CBCGPGaugeImpl;

class CBCGPDashboardConstructor  
{
public:
	CBCGPDashboardConstructor(const CBCGPGaugeInfo& info);
	virtual ~CBCGPDashboardConstructor();

public:
	virtual void Construct (CBCGPDashboard& dashboard) const;

protected:
	virtual CBCGPGaugeImpl* CreateElement (const CBCGPGaugeInfo::XElement& info) const;

	virtual void ConstructBaseElement (CBCGPGaugeImpl& element, const CBCGPGaugeInfo::XElement& info) const;

	const CBCGPGaugeInfo& GetInfo () const
	{
		return m_Info;
	}

private:
	const CBCGPGaugeInfo&	m_Info;
};

#endif // !defined(AFX_BCGPDASHBOARDCONSTRUCTOR_H__856CE5A1_AC4D_4029_8A23_77ECDC8EEB77__INCLUDED_)
