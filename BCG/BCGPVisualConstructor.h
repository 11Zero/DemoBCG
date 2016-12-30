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
// BCGPVisualConstructor.h: interface for the CBCGPRibbonConstrucor class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPVISUALCONSTRUCTOR_H__856CE5A1_AC4D_4029_8A23_77ECDC8EEB77__INCLUDED_)
#define AFX_BCGPVISUALCONSTRUCTOR_H__856CE5A1_AC4D_4029_8A23_77ECDC8EEB77__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGPVisualInfo.h"

class CBCGPVisual;
class CBCGPGaugeImpl;

class CBCGPVisualConstructor  
{
public:
	CBCGPVisualConstructor(const CBCGPVisualInfo& info);
	virtual ~CBCGPVisualConstructor();

public:
	virtual void Construct (CBCGPVisualContainer& container) const;

	virtual CBCGPBaseVisualObject* CreateElement (const CBCGPVisualInfo::XElement& info, const CBCGPVisualContainer* container) const;
	virtual void ConstructElement (CBCGPBaseVisualObject& element, const CBCGPVisualInfo::XElement& info) const;

protected:
	virtual CBCGPGaugeImpl* CreateGaugeElement (const CBCGPVisualInfo::XGaugeElement& info, const CBCGPVisualContainer* container) const;
	virtual void ConstructGaugeElement (CBCGPGaugeImpl& element, const CBCGPVisualInfo::XGaugeElement& info) const;
	
	virtual CBCGPDiagramVisualObject* CreateDiagramElement(const CBCGPVisualInfo::XDiagramElement& info, const CBCGPVisualContainer* container) const;
	virtual void ConstructDiagramElement (CBCGPDiagramVisualObject& element, const CBCGPVisualInfo::XDiagramElement& info) const;

	virtual void ConstructBaseElement (CBCGPBaseVisualObject& element, const CBCGPVisualInfo::XElement& info) const;
	virtual void ConstructBaseWinUIElement (CBCGPWinUIBaseObject& element, const CBCGPVisualInfo::XWinUIBaseElement& info) const;	
	virtual void ConstructImage (CBCGPImage& image, const CBCGPVisualInfo::XImage& info) const;

	const CBCGPVisualInfo& GetInfo () const
	{
		return m_Info;
	}

private:
	const CBCGPVisualInfo&	m_Info;
};

#endif // !defined(AFX_BCGPVISUALCONSTRUCTOR_H__856CE5A1_AC4D_4029_8A23_77ECDC8EEB77__INCLUDED_)
