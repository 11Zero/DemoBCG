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

// BCGPUserToolsManager.cpp: implementation of the CBCGPUserToolsManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BCGCBPro.h"
#include "BCGPUserToolsManager.h"
#include "BCGPLocalResource.h"
#include "BCGPToolBar.h"
#include "RegPath.h"
#include "BCGPRegistry.h"

#include "bcgprores.h"

static const CString strUserToolsProfile = _T("BCGUserToolsManager");
static const CString strUserToolsEntry = _T("Tools");

CBCGPUserToolsManager*	g_pUserToolsManager = NULL;
extern CObList	gAllToolbars;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CBCGPUserToolsManager::CBCGPUserToolsManager () :
	m_uiCmdToolsDummy (0),
	m_uiCmdFirst (0),
	m_uiCmdLast (0),
	m_pToolRTC (NULL),
	m_uiArgumentsMenuID (0),
	m_uiInitialDirMenuID (0),
	m_bIsCopy(FALSE)
{
	ASSERT (g_pUserToolsManager == NULL);
	g_pUserToolsManager = this;
}
//****************************************************************************************
CBCGPUserToolsManager::CBCGPUserToolsManager(const UINT uiCmdToolsDummy,
										   const UINT uiCmdFirst, const UINT uiCmdLast,
										   CRuntimeClass* pToolRTC,
										   UINT uArgMenuID, UINT uInitDirMenuID) :
	m_uiCmdToolsDummy (uiCmdToolsDummy),
	m_uiCmdFirst (uiCmdFirst),
	m_uiCmdLast (uiCmdLast),
	m_pToolRTC (pToolRTC),
	m_uiArgumentsMenuID (uArgMenuID),
	m_uiInitialDirMenuID (uInitDirMenuID),
	m_bIsCopy(FALSE)
{
	ASSERT (g_pUserToolsManager == NULL);
	g_pUserToolsManager = this;

	VERIFY (m_pToolRTC != NULL);
	VERIFY (m_pToolRTC->IsDerivedFrom (RUNTIME_CLASS (CBCGPUserTool)));

	ASSERT (m_uiCmdFirst <= m_uiCmdLast);

	//---------------------
	// Load default filter:
	//---------------------
	CBCGPLocalResource locaRes;
	m_strFilter.LoadString (IDS_BCGBARRES_CMD_FILTER);

	m_strDefExt = _T("*.exe");
}
//****************************************************************************************
CBCGPUserToolsManager::CBCGPUserToolsManager(const CBCGPUserToolsManager& src) :
	m_uiCmdToolsDummy (src.m_uiCmdToolsDummy),
	m_uiCmdFirst (src.m_uiCmdFirst),
	m_uiCmdLast (src.m_uiCmdLast),
	m_pToolRTC (src.m_pToolRTC),
	m_uiArgumentsMenuID (src.m_uiArgumentsMenuID),
	m_uiInitialDirMenuID (src.m_uiInitialDirMenuID),
	m_strFilter(src.m_strFilter),
	m_strDefExt(src.m_strDefExt),
	m_bIsCopy(TRUE)
{
	for (POSITION pos = src.m_lstUserTools.GetHeadPosition(); pos != NULL;)
	{
		CBCGPUserTool* pListTool = (CBCGPUserTool*) src.m_lstUserTools.GetNext (pos);
		ASSERT_VALID (pListTool);

		CBCGPUserTool* pTool = m_pToolRTC == NULL ? (new CBCGPUserTool()) : (CBCGPUserTool*) m_pToolRTC->CreateObject ();
		ASSERT_VALID (pTool);

		pTool->CopyFrom(*pListTool);

		m_lstUserTools.AddTail(pTool);
	}
}
//****************************************************************************************
CBCGPUserToolsManager::~CBCGPUserToolsManager()
{
	while (!m_lstUserTools.IsEmpty ())
	{
		delete m_lstUserTools.RemoveHead ();
	}

	if (!m_bIsCopy)
	{
		g_pUserToolsManager = NULL;
	}
}
//***********************************************************************************************
BOOL CBCGPUserToolsManager::LoadState (LPCTSTR lpszProfileName)
{
	CString strProfileName = ::BCGPGetRegPath (strUserToolsProfile, lpszProfileName);

	while (!m_lstUserTools.IsEmpty ())
	{
		delete m_lstUserTools.RemoveHead ();
	}

	CBCGPRegistrySP regSP;
	CBCGPRegistry& reg = regSP.Create (FALSE, FALSE);

	if (!reg.Open (strProfileName))
	{
		return FALSE;
	}

	if (!reg.Read (strUserToolsEntry, m_lstUserTools))
	{
		//---------------------------------------------------------
		// Tools objects may be corrupted, so, I don't delete them.
		// Memory leak is possible!
		//---------------------------------------------------------
		m_lstUserTools.RemoveAll ();
		return FALSE;
	}

	return TRUE;
}
//***********************************************************************************************
BOOL CBCGPUserToolsManager::SaveState (LPCTSTR lpszProfileName)
{
	CString strProfileName = ::BCGPGetRegPath (strUserToolsProfile, lpszProfileName);

	CBCGPRegistrySP regSP;
	CBCGPRegistry& reg = regSP.Create (FALSE, FALSE);

	if (!reg.CreateKey (strProfileName))
	{
		return FALSE;
	}

	return reg.Write (strUserToolsEntry, m_lstUserTools);
}
//*****************************************************************************************
CBCGPUserTool* CBCGPUserToolsManager::CreateNewTool ()
{
	ASSERT (m_pToolRTC != NULL);

	if (m_lstUserTools.GetCount () >= GetMaxTools ())
	{
		TRACE(_T("Too many user-defined tools. The max. number is %d"), GetMaxTools ());
		return FALSE;
	}

	//-----------------------------------
	// Find a first available command id:
	//-----------------------------------
	UINT uiCmdId = 0;
	for (uiCmdId = m_uiCmdFirst; uiCmdId <= m_uiCmdLast; uiCmdId ++)
	{
		BOOL bIsCmdAvailable = TRUE;

		for (POSITION pos = m_lstUserTools.GetHeadPosition (); pos != NULL;)
		{
			CBCGPUserTool* pListTool = (CBCGPUserTool*) m_lstUserTools.GetNext (pos);
			ASSERT_VALID (pListTool);

			if (pListTool->GetCommandId () == uiCmdId)
			{
				bIsCmdAvailable = FALSE;
				break;
			}
		}

		if (bIsCmdAvailable)
		{
			break;
		}
	}

	if (uiCmdId > m_uiCmdLast)
	{
		return NULL;
	}

	CBCGPUserTool* pTool = (CBCGPUserTool*) m_pToolRTC->CreateObject ();
	ASSERT_VALID (pTool);

	pTool->m_uiCmdId = uiCmdId;

	m_lstUserTools.AddTail (pTool);
	return pTool;
}
//*****************************************************************************************
BOOL CBCGPUserToolsManager::RemoveTool (CBCGPUserTool* pTool)
{
	ASSERT_VALID (pTool);
	POSITION pos = m_lstUserTools.Find (pTool);

	if (pos == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	m_lstUserTools.RemoveAt (pos);

	UINT uiCmdId = pTool->GetCommandId ();
	delete pTool;

	if (!m_bIsCopy)
	{
		//------------------------------------
		// Remove user tool from all toolbars:
		//------------------------------------
		for (POSITION posTlb = gAllToolbars.GetHeadPosition (); posTlb != NULL;)
		{
			CBCGPToolBar* pToolBar = (CBCGPToolBar*) gAllToolbars.GetNext (posTlb);
			ASSERT (pToolBar != NULL);

			BOOL bToolIsFound = FALSE;

			int iIndex = -1;
			while ((iIndex = pToolBar->CommandToIndex (uiCmdId)) >= 0)
			{
				pToolBar->RemoveButton (iIndex);
				bToolIsFound = TRUE;
			}

			if (bToolIsFound)
			{
				pToolBar->AdjustLayout ();
			}
		}
	}

	return TRUE;
}
//*****************************************************************************************
BOOL CBCGPUserToolsManager::MoveToolUp (CBCGPUserTool* pTool)
{
	ASSERT_VALID (pTool);

	POSITION pos = m_lstUserTools.Find (pTool);
	if (pos == NULL)
	{
		return FALSE;
	}

	POSITION posPrev = pos;
	m_lstUserTools.GetPrev (posPrev);
	if (posPrev == NULL)
	{
		return FALSE;
	}

	m_lstUserTools.RemoveAt (pos);
	m_lstUserTools.InsertBefore (posPrev, pTool);

	return TRUE;
}
//*****************************************************************************************
BOOL CBCGPUserToolsManager::MoveToolDown (CBCGPUserTool* pTool)
{
	ASSERT_VALID (pTool);

	POSITION pos = m_lstUserTools.Find (pTool);
	if (pos == NULL)
	{
		return FALSE;
	}

	POSITION posNext = pos;
	m_lstUserTools.GetNext (posNext);
	if (posNext == NULL)
	{
		return FALSE;
	}

	m_lstUserTools.RemoveAt (pos);
	m_lstUserTools.InsertAfter (posNext, pTool);

	return TRUE;
}
//*****************************************************************************************
BOOL CBCGPUserToolsManager::InvokeTool (UINT uiCmdId)
{
	CBCGPUserTool* pTool = FindTool (uiCmdId);
	if (pTool == NULL)
	{
		return FALSE;
	}

	return pTool->Invoke ();
}
//******************************************************************************
CBCGPUserTool* CBCGPUserToolsManager::FindTool (UINT uiCmdId) const
{
	if (uiCmdId < m_uiCmdFirst || uiCmdId > m_uiCmdLast)
	{
		return NULL;
	}

	for (POSITION pos = m_lstUserTools.GetHeadPosition (); pos != NULL;)
	{
		CBCGPUserTool* pListTool = (CBCGPUserTool*) m_lstUserTools.GetNext (pos);
		ASSERT_VALID (pListTool);

		if (pListTool->GetCommandId () == uiCmdId)
		{
			return pListTool;
		}
	}

	return NULL;
}
//******************************************************************************
void CBCGPUserToolsManager::ApplyChanges(const CBCGPUserToolsManager& src)
{
	while (!m_lstUserTools.IsEmpty ())
	{
		delete m_lstUserTools.RemoveHead ();
	}

	for (POSITION pos = src.m_lstUserTools.GetHeadPosition(); pos != NULL;)
	{
		CBCGPUserTool* pListTool = (CBCGPUserTool*) src.m_lstUserTools.GetNext (pos);
		ASSERT_VALID (pListTool);
		
		CBCGPUserTool* pTool = m_pToolRTC == NULL ? (new CBCGPUserTool()) : (CBCGPUserTool*) m_pToolRTC->CreateObject ();
		ASSERT_VALID (pTool);
		
		pTool->CopyFrom(*pListTool);

		m_lstUserTools.AddTail(pTool);
	}
}
