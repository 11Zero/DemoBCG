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
// BCGPDashboardCollector.h: interface for the CBCGPRibbonCollector class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPDASHBOARDCOLLECTOR_H__304B8633_B530_459B_B1ED_BA17DF48C6B2__INCLUDED_)
#define AFX_BCGPDASHBOARDCOLLECTOR_H__304B8633_B530_459B_B1ED_BA17DF48C6B2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGPGaugeInfo.h"

class CBCGPDashboard;
class CBCGPGaugeImpl;

class CBCGPDashboardCollector  
{
public:
	CBCGPDashboardCollector(CBCGPGaugeInfo& info);
	virtual ~CBCGPDashboardCollector();

public:
	virtual void Collect (const CBCGPDashboard& dashboard);

protected:
	virtual CBCGPGaugeInfo::XElement* CollectElement(const CBCGPGaugeImpl& element);

	virtual void CollectBaseElement (const CBCGPGaugeImpl& element, CBCGPGaugeInfo::XElement& info);
	virtual void CollectDashboard (const CBCGPDashboard& dashboard, CBCGPGaugeInfo::XDashboard& info);

	CBCGPGaugeInfo& GetInfo ()
	{
		return m_Info;
	}

private:
	CBCGPGaugeInfo&	m_Info;
};

#endif // !defined(AFX_BCGPDASHBOARDCOLLECTOR_H__304B8633_B530_459B_B1ED_BA17DF48C6B2__INCLUDED_)
