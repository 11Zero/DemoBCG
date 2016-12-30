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
// BCGPRibbonInfoLoader.h: interface for the CBCGPRibbonInfoLoader class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPRIBBONINFOLOADER_H__721B7C0B_B5A3_4619_BEF9_272BEBF1C81E__INCLUDED_)
#define AFX_BCGPRIBBONINFOLOADER_H__721B7C0B_B5A3_4619_BEF9_272BEBF1C81E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGPRibbonInfo.h"

#ifndef BCGP_EXCLUDE_RIBBON

class CBCGPRibbonInfoLoader: public CBCGPBaseInfoLoader
{
public:
	CBCGPRibbonInfoLoader (CBCGPRibbonInfo& info, 
		DWORD dwFlags = (CBCGPRibbonInfo::e_UseRibbon | CBCGPRibbonInfo::e_UseStatus));
	virtual ~CBCGPRibbonInfoLoader();

	virtual BOOL LoadFromBuffer (LPCTSTR lpszBuffer);

	virtual BOOL LoadImage (CBCGPRibbonInfo::XImage& image, BOOL bSingle = FALSE);

protected:
	virtual BOOL LoadImage (const CBCGPRibbonInfo::XID& id, CBCGPToolBarImages& image, BOOL bSingle);

	CBCGPRibbonInfo& GetRibbonInfo () const
	{
		return (CBCGPRibbonInfo&)GetInfo ();
	}
};

class CBCGPRibbonCustomizationInfoLoader: public CBCGPBaseInfoLoader
{
public:
	CBCGPRibbonCustomizationInfoLoader (CBCGPRibbonCustomizationInfo& info, 
		DWORD dwFlags = 0);
	virtual ~CBCGPRibbonCustomizationInfoLoader();

	virtual BOOL LoadFromBuffer (LPCTSTR lpszBuffer);

protected:
	CBCGPRibbonCustomizationInfo& GetRibbonCustomizationInfo () const
	{
		return (CBCGPRibbonCustomizationInfo&)GetInfo ();
	}
};

#endif // BCGP_EXCLUDE_RIBBON

#endif // !defined(AFX_BCGPRIBBONINFOLOADER_H__721B7C0B_B5A3_4619_BEF9_272BEBF1C81E__INCLUDED_)
