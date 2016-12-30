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
// BCGPVisualCollector.h: interface for the CBCGPRibbonCollector class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPCONTAINERCOLLECTOR_H__304B8633_B530_459B_B1ED_BA17DF48C6B2__INCLUDED_)
#define AFX_BCGPCONTAINERCOLLECTOR_H__304B8633_B530_459B_B1ED_BA17DF48C6B2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGPVisualInfo.h"

class CBCGPVisualContainer;
class CBCGPGaugeImpl;

class CBCGPVisualCollector
{
public:
	enum XFlags
	{
		e_CollectTagCloudData	= 0x01,
		e_CollectTreeMapData	= 0x02,
		e_CollectWinUITilesData	= 0x04,
		e_CollectImageData		= 0x08,
		e_CollectAll			= 0x0F
	};

public:
	CBCGPVisualCollector(CBCGPVisualInfo& info, DWORD dwFlags = e_CollectAll);
	virtual ~CBCGPVisualCollector();

public:
	virtual void Collect (const CBCGPVisualContainer& container);

	virtual CBCGPVisualInfo::XElement* CollectElement(const CBCGPBaseVisualObject& element);

protected:
	virtual void CollectBaseElement (const CBCGPBaseVisualObject& element, CBCGPVisualInfo::XElement& info);
	virtual void CollectBaseWinUIElement (const CBCGPWinUIBaseObject& element, CBCGPVisualInfo::XWinUIBaseElement& info);

	virtual CBCGPVisualInfo::XGaugeElement* CollectGaugeElement(const CBCGPGaugeImpl& element);
	virtual void CollectBaseGaugeElement (const CBCGPGaugeImpl& element, CBCGPVisualInfo::XGaugeElement& info);

	virtual CBCGPVisualInfo::XDiagramElement* CollectDiagramElement (const CBCGPDiagramVisualObject& element);
	virtual void CollectBaseDiagramElement (const CBCGPDiagramVisualObject& element, CBCGPVisualInfo::XDiagramElement& info);

	virtual void CollectContainer (const CBCGPVisualContainer& container, CBCGPVisualInfo::XContainer& info);
	virtual void CollectImage (const CBCGPImage& image, CBCGPVisualInfo::XImage& info);

	CBCGPVisualInfo& GetInfo ()
	{
		return m_Info;
	}

	DWORD GetFlags () const
	{
		return m_dwFlags;
	}

private:
	CBCGPVisualInfo&	m_Info;
	DWORD				m_dwFlags;
};

#endif // !defined(AFX_BCGPCONTAINERCOLLECTOR_H__304B8633_B530_459B_B1ED_BA17DF48C6B2__INCLUDED_)
