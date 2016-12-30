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
// BCGPLocalResource.h: interface for the CBCGPLocalResource class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPLOCALRESOURCE_H__3DD0607A_02AA_4FF1_86B2_C434318EB4D3__INCLUDED_)
#define AFX_BCGPLOCALRESOURCE_H__3DD0607A_02AA_4FF1_86B2_C434318EB4D3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CBCGPLocalResource  
{
public:
	CBCGPLocalResource();
	virtual ~CBCGPLocalResource();

protected:
	HINSTANCE m_hInstOld;
};

#endif // !defined(AFX_BCGPLOCALRESOURCE_H__3DD0607A_02AA_4FF1_86B2_C434318EB4D3__INCLUDED_)
