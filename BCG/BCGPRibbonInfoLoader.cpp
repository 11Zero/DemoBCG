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
// BCGPRibbonInfoLoader.cpp: implementation of the CBCGPRibbonInfoLoader class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BCGPRibbonInfoLoader.h"
#include "BCGPTagManager.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#ifndef BCGP_EXCLUDE_RIBBON

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGPRibbonInfoLoader::CBCGPRibbonInfoLoader (CBCGPRibbonInfo& info,
											  DWORD dwFlags)
	: CBCGPBaseInfoLoader(info, _T("BCGP_RIBBON_XML"), dwFlags)
{
}

CBCGPRibbonInfoLoader::~CBCGPRibbonInfoLoader()
{
}

BOOL CBCGPRibbonInfoLoader::LoadFromBuffer (LPCTSTR lpszBuffer)
{
	if (lpszBuffer == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	BOOL bRes = TRUE;

	CBCGPRibbonInfo& info = GetRibbonInfo ();

	if (info.FromTag (lpszBuffer))
	{
		CBCGPRibbonInfo::XArrayImages images;
		info.GetArrayImages (images);

        CBCGPRibbonInfo::XImage* pMainImage = NULL;
        CBCGPRibbonInfo::XImage* pMainImageScenic = NULL;
        if (info.GetRibbonBar().m_btnMain != NULL)
        {
            pMainImage = &info.GetRibbonBar().m_btnMain->m_Image;
			pMainImageScenic = &info.GetRibbonBar().m_btnMain->m_ImageScenic;
        }

		int i = 0;
		for (i = 0; i < images.GetSize (); i++)
		{
			if (!LoadImage (*images[i], (images[i] == pMainImage || images[i] == pMainImageScenic)))
			{
				ASSERT(FALSE);
			}
		}
	}

	return bRes;
}

BOOL CBCGPRibbonInfoLoader::LoadImage (const CBCGPRibbonInfo::XID& id, CBCGPToolBarImages& image, BOOL bSingle)
{
    if (id.IsEmpty ())
    {
        return TRUE;
    }

    if (id.m_Value > 0)
    {
        image.Load (id.m_Value, GetInstance ());
    }

    if (!image.IsValid () && !id.m_Name.IsEmpty ())
    {
        image.LoadStr (id.m_Name, GetInstance ());
    }

    if (image.IsValid ())
    {
        if (bSingle)
        {
            image.SetSingleImage ();
        }

        return TRUE;
    }

    return FALSE;
}

BOOL CBCGPRibbonInfoLoader::LoadImage (CBCGPRibbonInfo::XImage& image, BOOL bSingle)
{
    image.m_Image.Clear();

    if (image.m_ID.IsEmpty())
    {
        return TRUE;
    }

    double dblScale = globalData.GetRibbonImageScale();

	LoadImage(image.m_ID, image.m_Image, bSingle);

	if (!image.m_bDontScale && image.m_Image.IsValid () && dblScale > 1.0)
	{
		if (image.m_Image.GetBitsPerPixel () < 32)
		{
			image.m_Image.ConvertTo32Bits (globalData.clrBtnFace);
		}

		image.m_Image.SetTransparentColor (globalData.clrBtnFace);
		image.m_Image.SmoothResize (dblScale);
	}

	return image.m_Image.IsValid ();
}


CBCGPRibbonCustomizationInfoLoader::CBCGPRibbonCustomizationInfoLoader (CBCGPRibbonCustomizationInfo& info,
											  DWORD dwFlags)
	: CBCGPBaseInfoLoader(info, _T("BCGP_RIBBON_CUSTOMIZATION_XML"), dwFlags)
{
}

CBCGPRibbonCustomizationInfoLoader::~CBCGPRibbonCustomizationInfoLoader()
{
}

BOOL CBCGPRibbonCustomizationInfoLoader::LoadFromBuffer (LPCTSTR lpszBuffer)
{
	if (lpszBuffer == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	CBCGPRibbonCustomizationInfo& info = GetRibbonCustomizationInfo ();

	return info.FromTag (lpszBuffer);
}

#endif // BCGP_EXCLUDE_RIBBON
