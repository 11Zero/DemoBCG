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
// BCGSkinManager.h: interface for the CBCGPSkinManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPSKINMANAGER_H__5FD2EEA5_7D66_4687_A64F_31A271C233FD__INCLUDED_)
#define AFX_BCGPSKINMANAGER_H__5FD2EEA5_7D66_4687_A64F_31A271C233FD__INCLUDED_

#ifndef __AFXTEMPL_H__
	#include "afxtempl.h"
#endif

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#if defined _AFXDLL && !defined _BCGCBPRO_STATIC_	// Skins manager can not be used in the static version

#include "BCGCBPro.h"
#include "BCGPSkinEntry.h"
#include "BCGPWorkspace.h"

#define BCG_DEFUALT_SKIN	(-1)

class BCGCBPRODLLEXPORT CBCGPSkinManager : public CObject
{
	friend class CBCGPSelectSkinDlg;

public:
	CBCGPSkinManager (LPCTSTR lpszSkinsDirectory = BCG_DEFAULT_SKINS_DIR);
	virtual ~CBCGPSkinManager ();

// Opreations:
public:
	BOOL AddSkinLibrary (const CString& strLibraryPath, BOOL bLoadLibrary = TRUE);

	int GetSkinsCount () const
	{
		return (int) m_Skins.GetSize ();
	}

	LPCTSTR GetSkinName (int iSkinIndex) const;
	LPCTSTR GetSkinAuthor (int iSkinIndex) const;
	LPCTSTR GetSkinAuthorMail (int iSkinIndex) const;
	LPCTSTR GetSkinAuthorURL (int iSkinIndex) const;

	BOOL PreviewSkin (CDC* pDC, int iSkinIndex, CRect rect);
	
	BOOL SetActiveSkin (int iSkinIndex);
	int GetActiveSkin () const
	{
		return m_iActiveSkin;
	}

	BOOL LoadState (LPCTSTR lpszProfileName = NULL);
	BOOL SaveState (LPCTSTR lpszProfileName = NULL);

	BOOL ShowSelectSkinDlg ();

	void EnableSkinsDownload (LPCTSTR lpszURL,
							LPCTSTR lpszUserName = NULL,
							LPCTSTR lpszPassword = NULL,
							LPCTSTR lpszDownloadDLLName = NULL);
	BOOL IsDownloadAvailable () const
	{
		return !m_strSkinsURL.IsEmpty ();
	}

	virtual BOOL DownloadSkins ();

protected:
	virtual void ScanSkinsLocation ();
	virtual void RemoveAllSkins ();
	virtual void LoadAllSkins ();
	virtual void UnLoadAllSkins ();

	virtual BOOL RenameTempLibs ();

// Attributes:
protected:
	int											m_iActiveSkin;
	CArray<CBCGPSkinEntry, CBCGPSkinEntry&>		m_Skins;
	CArray<CBCGPSkinLibrary, CBCGPSkinLibrary&>	m_SkinLibraresInstances;

	CString	m_strSkinsDirectory;
	CString	m_strSkinsURL;
	CString	m_strDownloadDllName;
	CString	m_strUserName;
	CString	m_strUserPassword;
};

extern CBCGPSkinManager*	g_pSkinManager;

#endif // _AFXDLL

#endif // !defined(AFX_BCGPSKINMANAGER_H__5FD2EEA5_7D66_4687_A64F_31A271C233FD__INCLUDED_)
