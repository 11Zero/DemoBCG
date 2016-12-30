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

#if !defined(AFX_BCGPCOMMANDMANAGER_H__C089C790_8038_11D3_A9E7_005056800000__INCLUDED_)
#define AFX_BCGPCOMMANDMANAGER_H__C089C790_8038_11D3_A9E7_005056800000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXTEMPL_H__
	#include "afxtempl.h"
#endif

/**
 * This class is responsible for:
 * - knowledge about all commands
 * - images for all commands
 * - command categories
 * - default toolbars
 */

#include "BCGCBPro.h"
#include "BCGPToolBarImages.h"
#include "BCGPToolbarButton.h"
#include "CmdUsageCount.h"

class CBCGPCommandManager;

BCGCBPRODLLEXPORT CBCGPCommandManager* BCGPGetCmdMgr();
#define BCGPCMD_MGR (*BCGPGetCmdMgr())

#ifndef _NO_BCG_LEGACY_
class BCGCBPRODLLEXPORT CImageHash
{
public:
	static UINT	GetImageOfCommand(UINT nID, BOOL bUser = false);
};
#endif

class BCGCBPRODLLEXPORT CBCGPCommandManager
{
friend class _STATIC_CREATOR_;
protected:
	CBCGPCommandManager();

public:
	virtual ~CBCGPCommandManager();

	//----------------------------------------------------------------
	// Set toolbar buttons image resources.
	// You should use SINGLE CBCGPToolBarImages for ALL your toolbars!
	//----------------------------------------------------------------
	BOOL SetUserImages (CBCGPToolBarImages* pUserImages);

	//------------------------------
	// Clear all images except user:
	//------------------------------
	void ResetAllImages();
	
	void CleanUp ();	// Final cleaning

	// Image functions
	void SetCmdImage (UINT uiCmd, int iImage, BOOL bUserImage);
	int  GetCmdImage (UINT uiCmd, BOOL bUserImage) const;
	void ClearCmdImage (UINT uiCmd);
	void ClearUserCmdImages ();
	void ClearAllCmdImages ();

	void EnableMenuItemImage (UINT uiCmd, BOOL bEnable = TRUE, int nUserImage = -1);
	
	BOOL IsMenuItemWithoutImage (UINT uiCmd) const
	{
		return m_lstCommandsWithoutImages.Find (uiCmd) != NULL;
	}

	int GetMenuUserImage (UINT uiMenuCmd) const
	{
		int iImage = -1;
		m_mapMenuUserImages.Lookup (uiMenuCmd, iImage);

		return iImage;
	}

	BOOL LoadState (LPCTSTR lpszProfileName);
	BOOL SaveState (LPCTSTR lpszProfileName);

protected:
	CMap<UINT, UINT, int, int>	m_CommandIndex;
	CMap<UINT, UINT, int, int>	m_CommandIndexUser;
	CMap<UINT, UINT, int, int>	m_DefaultImages;

	// List of menu items where image appearance is disabled:
	CList<UINT,UINT>			m_lstCommandsWithoutImages;

	// List of menu items with user images:
	CMap<UINT,UINT,int,int>		m_mapMenuUserImages;
};

#endif // !defined(AFX_BCGPCOMMANDMANAGER_H__C089C790_8038_11D3_A9E7_005056800000__INCLUDED_)
