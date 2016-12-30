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

// BCGPToolbarMenuButtonsButton.cpp: implementation of the CBCGPToolbarMenuButtonsButton class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BCGPToolbarMenuButtonsButton.h"
#include "BCGPVisualManager.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CBCGPToolbarMenuButtonsButton, CBCGPToolbarButton)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGPToolbarMenuButtonsButton::CBCGPToolbarMenuButtonsButton()
{
	m_uiSystemCommand = 0;
}
//****************************************************************************************
CBCGPToolbarMenuButtonsButton::CBCGPToolbarMenuButtonsButton(UINT uiCmdId)
{
	if (uiCmdId != SC_CLOSE &&
		uiCmdId != SC_MINIMIZE &&
		uiCmdId != SC_RESTORE)
	{
		ASSERT (FALSE);
	}

	m_uiSystemCommand = uiCmdId;
}
//****************************************************************************************
CBCGPToolbarMenuButtonsButton::~CBCGPToolbarMenuButtonsButton()
{
}
//****************************************************************************************
void CBCGPToolbarMenuButtonsButton::OnDraw (CDC* pDC, const CRect& rect, 
					CBCGPToolBarImages* /*pImages*/,
					BOOL /*bHorz*/, BOOL /*bCustomizeMode*/,
					BOOL bHighlight,
					BOOL /*bDrawBorder*/, BOOL /*bGrayDisabledButtons*/)
{
	CBCGPVisualManager::GetInstance ()->OnDrawMenuSystemButton (
		pDC, rect, m_uiSystemCommand, m_nStyle, bHighlight);
}
//****************************************************************************************
SIZE CBCGPToolbarMenuButtonsButton::OnCalculateSize (CDC* /*pDC*/, const CSize& /*sizeDefault*/,
													BOOL /*bHorz*/)
{
	return CSize (	::GetSystemMetrics (SM_CXMENUSIZE),
					::GetSystemMetrics (SM_CYMENUSIZE));	// Fixed by JX Chen
}
//****************************************************************************************
void CBCGPToolbarMenuButtonsButton::CopyFrom (const CBCGPToolbarButton& s)
{
	CBCGPToolbarButton::CopyFrom (s);

	const CBCGPToolbarMenuButtonsButton& src = (const CBCGPToolbarMenuButtonsButton&) s;
	m_uiSystemCommand = src.m_uiSystemCommand;
}
