//*******************************************************************************
// COPYRIGHT NOTES
// ---------------
// This is a part of BCGControlBar Library Professional Edition
// Copyright (C) 1998-2014 BCGSoft Ltd.
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.
//*******************************************************************************
//
// BCGPUserToolsManager.h: interface for the CBCGPUserToolsManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPUSERTOOLSMANAGER_H__00F1EBC4_61BF_11D5_A304_F156F535EA11__INCLUDED_)
#define AFX_BCGPUSERTOOLSMANAGER_H__00F1EBC4_61BF_11D5_A304_F156F535EA11__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"
#include "BCGPUserTool.h"

class BCGCBPRODLLEXPORT CBCGPUserToolsManager : public CObject  
{
public:
	//--------------
	// Construction:
	//--------------
	CBCGPUserToolsManager ();	
	CBCGPUserToolsManager (	const UINT uiCmdToolsDummy,
							const UINT uiCmdFirst, const UINT uiCmdLast,
							CRuntimeClass* pToolRTC = RUNTIME_CLASS (CBCGPUserTool),
							UINT uArgMenuID = 0, UINT uInitDirMenuID = 0);
	CBCGPUserToolsManager(const CBCGPUserToolsManager& src);
	virtual ~CBCGPUserToolsManager();

	//------------
	// Operations:
	//------------
	BOOL LoadState (LPCTSTR lpszProfileName = NULL);
	BOOL SaveState (LPCTSTR lpszProfileName = NULL);

	CBCGPUserTool* CreateNewTool ();
	BOOL RemoveTool (CBCGPUserTool* pTool);
	
	BOOL MoveToolUp (CBCGPUserTool* pTool);
	BOOL MoveToolDown (CBCGPUserTool* pTool);

	BOOL IsUserToolCmd (UINT uiCmdId) const
	{
		return uiCmdId >= m_uiCmdFirst && uiCmdId <= m_uiCmdLast;
	}

	UINT GetToolsEntryCmd () const
	{
		return m_uiCmdToolsDummy;
	}

	BOOL InvokeTool (UINT uiCmdId);
	CBCGPUserTool* FindTool (UINT uiCmdId) const;

	void ApplyChanges(const CBCGPUserToolsManager& src);

	//------------
	// Attributes:
	//------------
public:
	const CObList& GetUserTools () const
	{
		return m_lstUserTools;
	}

	int GetMaxTools () const
	{
		return m_uiCmdLast - m_uiCmdFirst + 1; 
	}

	void SetFilter (const CString& strFilter)
	{
		m_strFilter = strFilter;
	}

	const CString& GetFilter () const
	{
		return m_strFilter;
	}

	void SetDefExt (const CString& strDefExt)
	{
		m_strDefExt = strDefExt;
	}

	const CString& GetDefExt () const
	{
		return m_strDefExt;
	}

	UINT GetArgumentsMenuID () const
	{
		return m_uiArgumentsMenuID;
	}

	UINT GetInitialDirMenuID () const
	{
		return m_uiInitialDirMenuID;
	}

protected:
	CObList				m_lstUserTools;			// List of CBCGPUserTool-derived objects
	const UINT			m_uiCmdToolsDummy;		// Will be replaced by the tools list
	const UINT			m_uiCmdFirst;			// First user tool command
	const UINT			m_uiCmdLast;			// Last user tool command
	CRuntimeClass*		m_pToolRTC;				// User tool run-time class
	CString				m_strFilter;			// "Add tool" file dilaog filter
	CString				m_strDefExt;			// Default tool file extension
	const UINT          m_uiArgumentsMenuID;	// ID for arguments popup option menu
	const UINT			m_uiInitialDirMenuID;	// ID for initial dir popup option menu
	const BOOL			m_bIsCopy;
};

extern CBCGPUserToolsManager*	g_pUserToolsManager;

#endif // !defined(AFX_BCGPUSERTOOLSMANAGER_H__00F1EBC4_61BF_11D5_A304_F156F535EA11__INCLUDED_)
