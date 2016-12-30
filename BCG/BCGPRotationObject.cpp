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
// BCGPRotationObject.cpp: implementation of the CBCGPRotationObject class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BCGPRotationObject.h"
#include "BCGPLocalResource.h"
#include "bcgprores.h"
#include "BCGPMath.h"
#include "BCGPVisualManager.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CBCGPRotationCtrl, CBCGPRadialMenu)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGPRotationObject::CBCGPRotationObject()
{
	{
		CBCGPLocalResource locaRes;

		m_Icons.Load(IDB_BCGBARRES_ROTATE_BUTTONS);

		int i = 0;

		for (i = 0; i <= 8; i++)
		{
			AddCommand(i, i);
		}

		CString strText;
		strText.LoadString(IDS_BCGBARRES_ROTATE_TOOLTIPS);

		for (i = 0; i < (int)m_arItems.GetSize(); i++)
		{
			int nIndex = strText.Find(_T('\n'));

			m_arItems[i]->m_strToolTip = nIndex >= 0 ? strText.Left(nIndex) : strText;

			if (nIndex < 0)
			{
				break;
			}

			strText = strText.Mid(nIndex + 1);
		}
	}

	m_bHasCenterButton = TRUE;
	m_nNotifyCmdID = 0;
	m_bIsCloseOnInvoke = FALSE;
}
//***************************************************************************************
CBCGPRotationObject::~CBCGPRotationObject()
{
}
//***************************************************************************************
BOOL CBCGPRotationObject::NotifyCommand()
{
	if (m_nHighlighted == m_nPressed && m_nPressed >= 0 && m_pCtrl->GetSafeHwnd() != NULL)
	{
		CWnd* pOwner = m_pCtrl->GetOwner();
		if (pOwner != NULL)
		{
			m_nLastClicked = m_nPressed;
			pOwner->SendMessage (	WM_COMMAND,
				MAKEWPARAM (m_nNotifyCmdID == 0 ? m_pCtrl->GetDlgCtrlID () : m_nNotifyCmdID, BN_CLICKED),
									(LPARAM) m_pCtrl->GetSafeHwnd());
		}
	}

	return FALSE;
}
//***************************************************************************************
void CBCGPRotationObject::EnablePart(RotationElement id, BOOL bEnable)
{
	int i = (int)id;

	if (i < 0 || i >= (int)m_arItems.GetSize())
	{
		ASSERT(FALSE);
		return;
	}

	if (id == m_nLastClicked)
	{
		m_nLastClicked = -1;
	}

	m_arItems[i]->m_bIsDisabled = !bEnable;
}
//***************************************************************************************
BOOL CBCGPRotationObject::IsPartEnabled(RotationElement id) const
{
	int i = (int)id;

	if (i < 0 || i >= (int)m_arItems.GetSize())
	{
		ASSERT(FALSE);
		return FALSE;
	}

	return !m_arItems[i]->m_bIsDisabled;
}
