//*******************************************************************************
// COPYRIGHT NOTES
// This is a part of the BCGControlBar Library
// Copyright (C) 1998-2014 BCGSoft Ltd.
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.
 //*******************************************************************************

#include "stdafx.h"
#include "BCGPCommandManager.h"
#include "BCGPRegistry.h"
#include "RegPath.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define REG_PARAMS_FMT						_T("%sBCGCommandManager")
#define REG_ENTRY_COMMANDS_WITHOUT_IMAGES	_T("CommandsWithoutImages")
#define REG_ENTRY_MENU_USER_IMAGES			_T("MenuUserImages")

static const CString strToolbarProfile	= _T("BCGToolBars");

//////////////////////////////////////////////////////////////////////
// One global static CBCGPCommandManager Object
//////////////////////////////////////////////////////////////////////
class _STATIC_CREATOR_
{
public:
	CBCGPCommandManager s_TheCmdMgr;
};

static _STATIC_CREATOR_ STATIC_CREATOR;

BCGCBPRODLLEXPORT CBCGPCommandManager* BCGPGetCmdMgr()
{
	return &STATIC_CREATOR.s_TheCmdMgr;
}
//////////////////////////////////////////////////////////////////////


#ifndef _NO_BCG_LEGACY_
UINT CImageHash::GetImageOfCommand(UINT nID, BOOL bUser /*= false*/)
{
	return BCGPCMD_MGR.GetCmdImage(nID, bUser);
}
#endif

//////////////////////////////////////////////////////////////////////
// Constructor/Destructor
//////////////////////////////////////////////////////////////////////

CBCGPCommandManager::CBCGPCommandManager()
{
}

CBCGPCommandManager::~CBCGPCommandManager()
{
}


//////////////////////////////////////////////////////////////////////
// ImageHash functions
//////////////////////////////////////////////////////////////////////

//****************************************************************************************
void CBCGPCommandManager::SetCmdImage (UINT uiCmd, int iImage, BOOL bUserImage)
{
	if (uiCmd == 0 || uiCmd == (UINT) -1)
	{
		return;
	}

	if (bUserImage)
	{
		// If command is already associated to the "standard" image list,
		// don't assign to to the "user" images
		if (GetCmdImage (uiCmd, FALSE) < 0)
		{
			m_CommandIndexUser.SetAt (uiCmd, iImage);
		}
	}
	else
	{
		if (GetCmdImage (uiCmd, TRUE) < 0)
		{
			m_CommandIndex.SetAt (uiCmd, iImage);
		}
	}
}
//****************************************************************************************
int CBCGPCommandManager::GetCmdImage (UINT uiCmd, BOOL bUserImage) const
{
	int iImage = -1;

	if (bUserImage)
	{
		if (!m_CommandIndexUser.Lookup (uiCmd, iImage))
		{
			return -1;
		}
	}
	else
	{
		if (!m_CommandIndex.Lookup (uiCmd, iImage))
		{
			return -1;
		}
	}
	
	return iImage;
}

//***************************************************************************************
void CBCGPCommandManager::ClearCmdImage (UINT uiCmd)
{
	m_CommandIndexUser.RemoveKey (uiCmd);
}
//****************************************************************************************
void CBCGPCommandManager::ClearUserCmdImages ()
{
	m_CommandIndexUser.RemoveAll ();
}
//****************************************************************************************
void CBCGPCommandManager::ClearAllCmdImages ()
{
	m_CommandIndex.RemoveAll ();
	m_CommandIndexUser.RemoveAll ();
	m_lstCommandsWithoutImages.RemoveAll ();
	m_mapMenuUserImages.RemoveAll ();
}
//****************************************************************************************
void CBCGPCommandManager::CleanUp ()
{
	ClearAllCmdImages ();
}
//**************************************************************************************
void CBCGPCommandManager::EnableMenuItemImage (UINT uiCmd, BOOL bEnable, int iUserImage)
{
	POSITION pos = m_lstCommandsWithoutImages.Find (uiCmd);
	
	if (bEnable)
	{
		if (pos != NULL)
		{
			m_lstCommandsWithoutImages.RemoveAt (pos);
		}

		if (iUserImage >= 0)
		{
			m_mapMenuUserImages.SetAt (uiCmd, iUserImage);
		}
		else
		{
			m_mapMenuUserImages.RemoveKey (uiCmd);
		}
	}
	else
	{
		m_mapMenuUserImages.RemoveKey (uiCmd);

		if (pos == NULL)
		{
			m_lstCommandsWithoutImages.AddTail (uiCmd);
		}
	}
}
//*************************************************************************************
BOOL CBCGPCommandManager::LoadState (LPCTSTR lpszProfileName)
{
	CString strProfileName = ::BCGPGetRegPath (strToolbarProfile, lpszProfileName);

	CString strSection;
	strSection.Format (REG_PARAMS_FMT, strProfileName);

	CBCGPRegistrySP regSP;
	CBCGPRegistry& reg = regSP.Create (FALSE, TRUE);

	if (!reg.Open (strSection))
	{
		return FALSE;
	}

	m_lstCommandsWithoutImages.RemoveAll ();

	return	reg.Read (REG_ENTRY_COMMANDS_WITHOUT_IMAGES, m_lstCommandsWithoutImages) &&
			reg.Read (REG_ENTRY_MENU_USER_IMAGES, m_mapMenuUserImages);
}
//*************************************************************************************
BOOL CBCGPCommandManager::SaveState (LPCTSTR lpszProfileName)
{
	CString strProfileName = ::BCGPGetRegPath (strToolbarProfile, lpszProfileName);

	CString strSection;
	strSection.Format (REG_PARAMS_FMT, strProfileName);

	CBCGPRegistrySP regSP;
	CBCGPRegistry& reg = regSP.Create (FALSE, FALSE);

	if (reg.CreateKey (strSection))
	{
		return	reg.Write (REG_ENTRY_COMMANDS_WITHOUT_IMAGES, m_lstCommandsWithoutImages) &&
				reg.Write (REG_ENTRY_MENU_USER_IMAGES, m_mapMenuUserImages);
	}

	return FALSE;
}
