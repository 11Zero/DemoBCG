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

// BCGPToolbarDropTarget.cpp : implementation file
//

#include "stdafx.h"

#include "BCGPToolbarButton.h"
#include "BCGPToolbarDropTarget.h"
#include "BCGPToolBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBCGPToolbarDropTarget

CBCGPToolbarDropTarget::CBCGPToolbarDropTarget()
{
	m_pOwner = NULL;
}

CBCGPToolbarDropTarget::~CBCGPToolbarDropTarget()
{
}


BEGIN_MESSAGE_MAP(CBCGPToolbarDropTarget, COleDropTarget)
	//{{AFX_MSG_MAP(CBCGPToolbarDropTarget)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CBCGPToolbarDropTarget::Register (CBCGPToolBar* pOwner)
{
	m_pOwner = pOwner;
	return COleDropTarget::Register (pOwner);
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPToolbarDropTarget message handlers

DROPEFFECT CBCGPToolbarDropTarget::OnDragEnter(CWnd* /*pWnd*/, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point) 
{
	ASSERT (m_pOwner != NULL);

	if (!m_pOwner->IsCustomizeMode () ||
		!pDataObject->IsDataAvailable (CBCGPToolbarButton::m_cFormat))
	{
		return DROPEFFECT_NONE;
	}

	return m_pOwner -> OnDragEnter(pDataObject, dwKeyState, point);
}

void CBCGPToolbarDropTarget::OnDragLeave(CWnd* /*pWnd*/) 
{
	ASSERT (m_pOwner != NULL);
	m_pOwner->OnDragLeave ();
}

DROPEFFECT CBCGPToolbarDropTarget::OnDragOver(CWnd* /*pWnd*/, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point) 
{
	ASSERT (m_pOwner != NULL);

	if (!m_pOwner->IsCustomizeMode () ||
		!pDataObject->IsDataAvailable (CBCGPToolbarButton::m_cFormat))
	{
		return DROPEFFECT_NONE;
	}

	return m_pOwner -> OnDragOver(pDataObject, dwKeyState, point);
}

DROPEFFECT CBCGPToolbarDropTarget::OnDropEx(CWnd* /*pWnd*/, 
							COleDataObject* pDataObject, 
							DROPEFFECT dropEffect, 
							DROPEFFECT /*dropList*/, CPoint point) 
{
	ASSERT (m_pOwner != NULL);

	if (!m_pOwner->IsCustomizeMode () ||
		!pDataObject->IsDataAvailable (CBCGPToolbarButton::m_cFormat))
	{
		return DROPEFFECT_NONE;
	}

	return m_pOwner -> OnDrop(pDataObject, dropEffect, point) ?
			dropEffect : DROPEFFECT_NONE;
}
