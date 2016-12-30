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
// BCGPTooltipManager.cpp: implementation of the CBCGPTooltipManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BCGPTooltipManager.h"
#include "bcgglobals.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#ifndef TTM_SETTITLE
#define TTM_SETTITLE (WM_USER + 32)
#endif

UINT BCGM_UPDATETOOLTIPS = ::RegisterWindowMessage (_T("BCGM_UPDATETOOLTIPS"));
BCGCBPRODLLEXPORT CBCGPTooltipManager*	g_pTooltipManager = NULL;

BOOL CBCGPTooltipManager::CreateToolTip (
		CToolTipCtrl*& pToolTip, CWnd* pWndParent, UINT nType)
{
	UINT nCurrType = BCGP_TOOLTIP_TYPE_DEFAULT;
	int nIndex = -1;

	for (int i = 0; i < BCGP_TOOLTIP_TYPES; i++)
	{
		if (nCurrType == nType)
		{
			nIndex = i;
			break;
		}

		nCurrType <<= 1;
	}

	if (nIndex == -1)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	if (pToolTip != NULL)
	{
		ASSERT_VALID (pToolTip);

		if (pToolTip->GetSafeHwnd () != NULL)
		{
			pToolTip->DestroyWindow ();
		}

		delete pToolTip;
		pToolTip = NULL;
	}

	if (g_pTooltipManager != NULL)
	{
		if (!g_pTooltipManager->CreateToolTipObject (pToolTip, nIndex))
		{
			return FALSE;
		}
	}
	else
	{
		pToolTip = new CToolTipCtrl;
		ASSERT_VALID (pToolTip);
	}

	if (!pToolTip->Create (pWndParent, TTS_ALWAYSTIP | TTS_NOPREFIX))
	{
		ASSERT (FALSE);
		return FALSE;
	}

	if (pWndParent->GetSafeHwnd () != NULL && (pWndParent->GetExStyle() & WS_EX_LAYOUTRTL) != NULL)
	{
		pToolTip->ModifyStyleEx(0, WS_EX_LAYOUTRTL);
	}

	pToolTip->Activate (TRUE);

	if (globalData.m_nMaxToolTipWidth != -1)
	{
		pToolTip->SetMaxTipWidth (globalData.m_nMaxToolTipWidth);
	}

	if (pWndParent->GetSafeHwnd () != NULL &&
		g_pTooltipManager != NULL &&
		g_pTooltipManager->m_lstOwners.Find (pWndParent->GetSafeHwnd ()) == NULL)
	{
		g_pTooltipManager->m_lstOwners.AddTail (pWndParent->GetSafeHwnd ());

	}

	return TRUE;
}
//***********************************************************************
void CBCGPTooltipManager::DeleteToolTip (CToolTipCtrl*& pToolTip)
{
	if (pToolTip != NULL)
	{
		ASSERT_VALID (pToolTip);

		if (pToolTip->GetSafeHwnd () != NULL)
		{
			HWND hwndParent = pToolTip->GetParent ()->GetSafeHwnd ();

			if (g_pTooltipManager != NULL && hwndParent != NULL)
			{
				POSITION pos = g_pTooltipManager->m_lstOwners.Find (hwndParent);
				if (pos != NULL)
				{
					g_pTooltipManager->m_lstOwners.RemoveAt (pos);
				}
			}

			pToolTip->DestroyWindow ();
		}

		delete pToolTip;
		pToolTip = NULL;
	}
}
//***********************************************************************
void CBCGPTooltipManager::SetTooltipText (TOOLINFO* pTI,
				CToolTipCtrl* pToolTip, UINT nType, const CString strText,
				LPCTSTR lpszDescr)
{
	ASSERT_VALID (pToolTip);
	ASSERT (pTI != NULL);

	int nIndex = -1;
	UINT nCurrType = BCGP_TOOLTIP_TYPE_DEFAULT;

	for (int i = 0; i < BCGP_TOOLTIP_TYPES; i++)
	{
		if (nCurrType == nType)
		{
			nIndex = i;
			break;
		}

		nCurrType <<= 1;
	}

	if (nIndex == -1)
	{
		ASSERT (FALSE);
		return;
	}

	CString strTipText = strText;
	CString strDescr = lpszDescr != NULL ? lpszDescr : _T("");

	if (g_pTooltipManager != NULL &&
		g_pTooltipManager->m_Params [nIndex].m_bBallonTooltip)
	{
		if (strDescr.IsEmpty ())
		{
			pToolTip->SendMessage (TTM_SETTITLE, 1, (LPARAM)(LPCTSTR) strDescr);
		}
		else
		{
			pToolTip->SendMessage (TTM_SETTITLE, 1, (LPARAM)(LPCTSTR) strText);
			strTipText = strDescr;
		}
	}

	pTI->lpszText = (LPTSTR) ::calloc ((strTipText.GetLength () + 1), sizeof (TCHAR));
	lstrcpy (pTI->lpszText, strTipText);

	CBCGPToolTipCtrl* pBCGPToolTip = DYNAMIC_DOWNCAST (
		CBCGPToolTipCtrl, pToolTip);

	if (pBCGPToolTip != NULL)
	{
		pBCGPToolTip->SetDescription (strDescr);
	}
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGPTooltipManager::CBCGPTooltipManager()
{
	ASSERT (g_pTooltipManager == NULL);
	g_pTooltipManager = this;

	for (int i = 0; i < BCGP_TOOLTIP_TYPES; i++)
	{
		m_pRTC [i] = NULL;
	}
}
//**********************************************************************
CBCGPTooltipManager::~CBCGPTooltipManager()
{
	g_pTooltipManager = NULL;
}
//**********************************************************************
BOOL CBCGPTooltipManager::CreateToolTipObject (
		CToolTipCtrl*& pToolTip, UINT nType)
{
	if (nType < 0 || nType >= BCGP_TOOLTIP_TYPES)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	CBCGPToolTipParams& params = m_Params [nType];
	CRuntimeClass* pRTC = m_pRTC [nType];

	if (pRTC == NULL)
	{
		pToolTip = new CToolTipCtrl;
	}
	else
	{
		pToolTip = DYNAMIC_DOWNCAST (
			CToolTipCtrl, pRTC->CreateObject ());
	}

	if (pToolTip == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	ASSERT_VALID (pToolTip);

	CBCGPToolTipCtrl* pBCGPToolTip = DYNAMIC_DOWNCAST (
		CBCGPToolTipCtrl, pToolTip);

	if (pBCGPToolTip != NULL)
	{
		pBCGPToolTip->SetParams (&params);
	}

	return TRUE;
}
//**********************************************************************
void CBCGPTooltipManager::SetTooltipParams (
		UINT nTypes,
		CRuntimeClass* pRTC,
		CBCGPToolTipParams* pParams)
{
	if (pRTC == NULL || !pRTC->IsDerivedFrom (RUNTIME_CLASS (CBCGPToolTipCtrl)))
	{
		if (pParams != NULL)
		{
			// Parameters can be used with CBCGPToolTipCtrl class only!
			ASSERT (FALSE);
			pParams = NULL;
		}
	}

	CBCGPToolTipParams defaultParams;

	UINT nType = BCGP_TOOLTIP_TYPE_DEFAULT;

	for (int i = 0; i < BCGP_TOOLTIP_TYPES; i++)
	{
		if ((nType & nTypes) != 0)
		{
			if (pParams == NULL)
			{
				m_Params [i] = defaultParams;
			}
			else
			{
				m_Params [i] = *pParams;
			}

			m_pRTC [i] = pRTC;
		}

		nType <<= 1;
	}

	for (POSITION pos = m_lstOwners.GetHeadPosition (); pos != NULL;)
	{
		HWND hwndOwner = m_lstOwners.GetNext (pos);

		if (::IsWindow (hwndOwner))
		{
			::SendMessage (hwndOwner, BCGM_UPDATETOOLTIPS, (WPARAM) nTypes, 0);
		}
	}
}
//*******************************************************************************
void CBCGPTooltipManager::UpdateTooltips ()
{
	for (POSITION pos = m_lstOwners.GetHeadPosition (); pos != NULL;)
	{
		HWND hwndOwner = m_lstOwners.GetNext (pos);

		if (::IsWindow (hwndOwner))
		{
			::SendMessage (hwndOwner, BCGM_UPDATETOOLTIPS, (WPARAM) BCGP_TOOLTIP_TYPE_ALL, 0);
		}
	}
}
