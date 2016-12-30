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

// BCGPMouseManager.cpp: implementation of the CBCGPMouseManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BCGPMouseManager.h"
#include "BCGPRegistry.h"
#include "RegPath.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

static const CString strRegEntryName = _T("Mouse");
static const CString strMouseProfile = _T("BCGMouseManager");

CBCGPMouseManager* g_pMouseManager = NULL;

IMPLEMENT_SERIAL(CBCGPMouseManager, CObject, 1)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGPMouseManager::CBCGPMouseManager()
{
	ASSERT (g_pMouseManager == NULL);
	g_pMouseManager = this;
}
//************************************************************************************************
CBCGPMouseManager::~CBCGPMouseManager()
{
	g_pMouseManager = NULL;
}
//************************************************************************************************
BOOL CBCGPMouseManager::AddView (int iViewId, UINT uiViewNameResId, UINT uiIconId)
{
	CString strViewName;
	strViewName.LoadString (uiViewNameResId);

	return AddView (iViewId, strViewName, uiIconId);
}
//************************************************************************************************
BOOL CBCGPMouseManager::AddView (int iViewId, LPCTSTR lpszViewName, UINT uiIconId)
{
	ASSERT (lpszViewName != NULL);

	int iId;
	if (m_ViewsNames.Lookup (lpszViewName, iId))	// Already exist
	{
		return FALSE;
	}

	m_ViewsNames.SetAt (lpszViewName, iViewId);
	
	if (uiIconId != 0)
	{
		m_ViewsToIcons.SetAt (iViewId, uiIconId);
	}

	return TRUE;
}
//************************************************************************************************
UINT CBCGPMouseManager::GetViewDblClickCommand (int iId) const
{
	UINT uiCmd;

	if (!m_ViewsToCommands.Lookup (iId, uiCmd))
	{
		return 0;
	}

	return uiCmd;
}
//************************************************************************************************
void CBCGPMouseManager::GetViewNames (CStringList& listOfNames) const
{
	listOfNames.RemoveAll ();

	for (POSITION pos = m_ViewsNames.GetStartPosition (); pos != NULL;)
	{
		CString strName;
		int iId;

		m_ViewsNames.GetNextAssoc (pos, strName, iId);
		listOfNames.AddTail (strName);
	}
}
//************************************************************************************************
int CBCGPMouseManager::GetViewIdByName (LPCTSTR lpszName) const
{
	ASSERT (lpszName != NULL);

	int iId;

	if (!m_ViewsNames.Lookup (lpszName, iId))
	{
		return -1;
	}

	return iId;
}
//************************************************************************************************
BOOL CBCGPMouseManager::LoadState (LPCTSTR lpszProfileName)
{
	CString strProfileName = ::BCGPGetRegPath (strMouseProfile, lpszProfileName);

	BOOL bResult = FALSE;

	LPBYTE	lpbData = NULL;
	UINT	uiDataSize;

	CBCGPRegistrySP regSP;
	CBCGPRegistry& reg = regSP.Create (FALSE, TRUE);
	
	if (!reg.Open (strProfileName))
	{
		TRACE(_T("CBCGPMouseManager::LoadState: Can't open registry %s!\n"), strProfileName);
		return FALSE;
	}

	if (!reg.Read (strRegEntryName, &lpbData, &uiDataSize))
	{
		TRACE(_T("CBCGPMouseManager::LoadState: Can't load registry data %s!\n"), strProfileName);
		return FALSE;
	}

	try
	{
		CMemFile file (lpbData, uiDataSize);
		CArchive ar (&file, CArchive::load);

		Serialize (ar);
		bResult = TRUE;
	}
	catch (CMemoryException* pEx)
	{
		pEx->Delete ();
		TRACE(_T("Memory exception in CBCGPMouseManager::LoadState ()!\n"));
	}
	catch (CArchiveException* pEx)
	{
		pEx->Delete ();
		TRACE(_T("CArchiveException exception in CBCGPMouseManager::LoadState ()!\n"));
	}

	if (lpbData != NULL)
	{
		delete [] lpbData;
	}

	return bResult;
}
//************************************************************************************************
BOOL CBCGPMouseManager::SaveState (LPCTSTR lpszProfileName)
{
	CString strProfileName = ::BCGPGetRegPath (strMouseProfile, lpszProfileName);

	BOOL bResult = FALSE;

	try
	{
		CMemFile file;

		{
			CArchive ar (&file, CArchive::store);

			Serialize (ar);
			ar.Flush ();
		}

		UINT uiDataSize = (UINT) file.GetLength ();
		LPBYTE lpbData = file.Detach ();

		if (lpbData != NULL)
		{
			CBCGPRegistrySP regSP;
			CBCGPRegistry& reg = regSP.Create (FALSE, FALSE);

			if (reg.CreateKey (strProfileName))
			{
				bResult = reg.Write (strRegEntryName, lpbData, uiDataSize);
			}

			free (lpbData);
		}
	}
	catch (CMemoryException* pEx)
	{
		pEx->Delete ();
		TRACE(_T("Memory exception in CBCGPMouseManager::SaveState ()!\n"));
	}
	catch (CArchiveException* pEx)
	{
		pEx->Delete ();
		TRACE(_T("CArchiveException exception in CBCGPMouseManager::SaveState ()!\n"));
	}

	return bResult;
}
//************************************************************************************************
void CBCGPMouseManager::Serialize (CArchive& ar)
{
	CObject::Serialize (ar);

	if (ar.IsLoading ())
	{
		m_ViewsToCommands.RemoveAll ();

		int iCount;
		ar >> iCount;

		for (int i = 0; i < iCount; i ++)
		{
			int iViewId;
			ar >> iViewId;

			UINT uiCmd;
			ar >> uiCmd;

			m_ViewsToCommands.SetAt (iViewId, uiCmd);
		}
	}
	else
	{
		int iCount = (int) m_ViewsToCommands.GetCount ();
		ar << iCount;

		for (POSITION pos = m_ViewsToCommands.GetStartPosition (); pos != NULL;)
		{
			int iViewId;
			UINT uiCmd;

			m_ViewsToCommands.GetNextAssoc (pos, iViewId, uiCmd);
			
			ar << iViewId;
			ar << uiCmd;
		}
	}
}
//************************************************************************************************
void CBCGPMouseManager::SetCommandForDblClick (int iViewId, UINT uiCmd)
{
	if (uiCmd > 0)
	{
		m_ViewsToCommands.SetAt (iViewId, uiCmd);
	}
	else
	{
		m_ViewsToCommands.RemoveKey (iViewId);
	}
}
//************************************************************************************************
UINT CBCGPMouseManager::GetViewIconId (int iViewId) const
{
	UINT uiIconId;
	if (!m_ViewsToIcons.Lookup (iViewId, uiIconId))
	{
		return 0;
	}

	return uiIconId;
}
