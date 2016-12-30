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

// BCGOutlookButton.h: interface for the CBCGPOutlookButton class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPOUTLOOKBUTTON_H__EF4340F6_720D_11D3_A746_0090274409AC__INCLUDED_)
#define AFX_BCGPOUTLOOKBUTTON_H__EF4340F6_720D_11D3_A746_0090274409AC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"
#include "BCGPToolbarButton.h"

class CBCGPOutlookBarPane;

class BCGCBPRODLLEXPORT CBCGPOutlookButton : public CBCGPToolbarButton
{
	friend class CBCGPOutlookBarPane;

	DECLARE_SERIAL(CBCGPOutlookButton)

public:
	CBCGPOutlookButton();
	virtual ~CBCGPOutlookButton();

	virtual void OnDraw (CDC* pDC, const CRect& rect, CBCGPToolBarImages* pImages,
						BOOL bHorz = TRUE, BOOL bCustomizeMode = FALSE,
						BOOL bHighlight = FALSE,
						BOOL bDrawBorder = TRUE,
						BOOL bGrayDisabledButtons = TRUE);
	virtual SIZE OnCalculateSize (CDC* pDC, const CSize& sizeDefault, BOOL bHorz);
	virtual void OnChangeParentWnd (CWnd* pWndParent);

	virtual BOOL CanBeDropped (CBCGPToolBar* pToolbar);
	virtual void SetImage (int iImage);

protected:
	CBCGPOutlookBarPane*	m_pWndParentBar;
	CSize					m_sizeImage;
	BOOL					m_bIsWholeText;
};

#endif // !defined(AFX_BCGPOUTLOOKBUTTON_H__EF4340F6_720D_11D3_A746_0090274409AC__INCLUDED_)
