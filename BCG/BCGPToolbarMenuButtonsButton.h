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

// BCGToolbarMenuButtonsButton.h: interface for the CBCGPToolbarMenuButtonsButton class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPTOOLBARMENUBUTTONSBUTTON_H__A6E1AD74_D511_11D1_A64D_00A0C93A70EC__INCLUDED_)
#define AFX_BCGPTOOLBARMENUBUTTONSBUTTON_H__A6E1AD74_D511_11D1_A64D_00A0C93A70EC__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "BCGPToolbarButton.h"

class CBCGPToolbarMenuButtonsButton : public CBCGPToolbarButton  
{
	DECLARE_DYNCREATE(CBCGPToolbarMenuButtonsButton)

public:
	CBCGPToolbarMenuButtonsButton();
	CBCGPToolbarMenuButtonsButton(UINT uiCmdId);
	virtual ~CBCGPToolbarMenuButtonsButton();

// Overrides:
	virtual void Serialize (CArchive& /*ar*/)
	{
		ASSERT (FALSE);
	}

	virtual void OnDraw (CDC* pDC, const CRect& rect, CBCGPToolBarImages* pImages,
						BOOL bHorz = TRUE, BOOL bCustomizeMode = FALSE,
						BOOL bHighlight = FALSE,
						BOOL bDrawBorder = TRUE,
						BOOL bGrayDisabledButtons = TRUE);

	virtual SIZE OnCalculateSize (CDC* pDC, const CSize& sizeDefault, BOOL bHorz);
	virtual BOOL CanBeStored () const			{	return FALSE;	}
	virtual BOOL HaveHotBorder () const			{	return FALSE;	}

	virtual void CopyFrom (const CBCGPToolbarButton& src);

// Attributes:
public:
	UINT	m_uiSystemCommand;
};

#endif // !defined(AFX_BCGPTOOLBARMENUBUTTONSBUTTON_H__A6E1AD74_D511_11D1_A64D_00A0C93A70EC__INCLUDED_)
