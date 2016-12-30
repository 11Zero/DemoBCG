//*******************************************************************************
// COPYRIGHT NOTES
// ---------------
// This is a part of the BCGProControlBar Library
// Copyright (C) 1998-2014 BCGSoft Ltd.
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.
//
// BCGPSkinManager.cpp: implementation of the CBCGPSkinManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#if defined _AFXDLL && !defined _BCGCBPRO_STATIC_	// Skins manager can not be used in the static version

#include <io.h>
#include <direct.h>

#include "BCGPSkinManager.h"
#include "BCGPWorkspace.h"
#include "BCGPVisualManager.h"
#include "BCGPRegistry.h"
#include "RegPath.h"
#include "bcgprores.h"
#include "BCGPLocalResource.h"
#include "BCGPSelectSkinDlg.h"
#include "BCGCBProVer.h"

#define REG_ENTRY_LIBRARY_NAME				_T("SkinsLibrary")
#define REG_ENTRY_SKIN_NAME					_T("SkinName")
#define REG_ENTRY_SKIN_INDEX				_T("SkinIndex")
#define REG_ENTRY_CURRENT_VERSION			_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion")
#define REG_ENTRY_COMMON_FILES				_T("CommonFilesDir")
#define DLL_FILE_MASK						_T("\\*.dll")
#define TMP_FILE_MASK						_T("\\*.skn")

static const CString strRegEntryName = _T("Skin");
static const CString strSkinProfile  = _T("BCGPSkinManager");

extern CBCGPWorkspace* g_pWorkspace;

CBCGPSkinManager* g_pSkinManager = NULL;

typedef BOOL (__stdcall * BCGCBDOWNLOADSKINS3)(LPCTSTR, LPCTSTR, LPCTSTR, LPCTSTR, int, int, BOOL, int);
typedef BOOL (__stdcall * BCGCBDOWNLOADSKINS2)(LPCTSTR, LPCTSTR, LPCTSTR, LPCTSTR, int, int, BOOL);
typedef BOOL (__stdcall * BCGCBDOWNLOADSKINS)(LPCTSTR, LPCTSTR, LPCTSTR, LPCTSTR, int, int);

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGPSkinManager::CBCGPSkinManager (LPCTSTR lpszSkinsDirectory/* = BCG_DEFAULT_SKINS_DIR*/)
{
	ASSERT (g_pSkinManager == NULL);
	g_pSkinManager = this;

	m_iActiveSkin = BCG_DEFUALT_SKIN;

	if (lpszSkinsDirectory == BCG_DEFAULT_SKINS_DIR)
	{
		CBCGPRegistrySP regSP;
		CBCGPRegistry& reg = regSP.Create (TRUE, TRUE);

		CString strCommonFiles;

		if (reg.Open (REG_ENTRY_CURRENT_VERSION) &&
			reg.Read (REG_ENTRY_COMMON_FILES, strCommonFiles))
		{ 
			m_strSkinsDirectory = strCommonFiles;
			
			if (!m_strSkinsDirectory.IsEmpty ())
			{
				if (m_strSkinsDirectory [m_strSkinsDirectory.GetLength () - 1] != _T('\\'))
				{
					m_strSkinsDirectory += _T('\\');
				}

				CString strSkinsDirectory = m_strSkinsDirectory;

				m_strSkinsDirectory += _T("Bcgsoft\\Skins");

				//-------------------------------------------------
				// Check for directory existance and if this folder
				// doesn't exist, create it now:
				//-------------------------------------------------
				if (_taccess (m_strSkinsDirectory, 0) != 0)
				{
					//-------------------------
					// Directory doesn't exist.
					//-------------------------
					strSkinsDirectory += _T("Bcgsoft\\");
					_tmkdir (strSkinsDirectory);

					strSkinsDirectory += _T("Skins");
					_tmkdir (strSkinsDirectory);
				}
			}
		}
	}
	else if (lpszSkinsDirectory != NULL)
	{
		m_strSkinsDirectory = lpszSkinsDirectory;
	}

	ScanSkinsLocation ();
}
//**********************************************************************************
CBCGPSkinManager::~CBCGPSkinManager ()
{
	CBCGPVisualManager::DestroyInstance ();
	RemoveAllSkins ();
	g_pSkinManager = NULL;
}
//**********************************************************************************
BOOL CBCGPSkinManager::AddSkinLibrary (const CString& strLibraryPath , BOOL bLoadLibrary)
{
	for (int i = 0; i < m_Skins.GetSize (); i++)
	{
		if (m_Skins[i].m_strLibraryPath == strLibraryPath)
		{
			TRACE(_T("Skins library '%s' is already exist\n"), strLibraryPath);
			return TRUE;
		}
	}
	
	HINSTANCE hInstance = ::LoadLibrary (strLibraryPath);

	CBCGPSkinLibrary Library;
	if (!Library.Init (hInstance))
	{
		if (hInstance != NULL)
		{
			::FreeLibrary (hInstance);
		}
		return FALSE;
	}

	// Version validation:
	int iVersionMajor, iVersionMinor;
	if (!Library.GetSkinVersion (iVersionMajor, iVersionMinor))
	{
		::FreeLibrary (hInstance);
		return FALSE;
	}

	BOOL bIsUNICODESkin = Library.IsUNICODE ();

	if (iVersionMajor != _BCGCBPRO_VERSION_MAJOR ||
		iVersionMinor != _BCGCBPRO_VERSION_MINOR)
	{
		::FreeLibrary (hInstance);
		return FALSE;
	}

#ifdef _UNICODE
	if (!bIsUNICODESkin)
	{
		::FreeLibrary (hInstance);
		return FALSE;
	}
#else
	if (bIsUNICODESkin)
	{
		::FreeLibrary (hInstance);
		return FALSE;
	}
#endif

	int iSkinCount = Library.GetSkinsCount ();
	if (iSkinCount < 1)
	{
		::FreeLibrary (hInstance);
		return FALSE;
	}

	int iIndex = BCG_DEFUALT_SKIN;
	BOOL bSkinFound = FALSE;

	for (int iCount = 0; iCount<iSkinCount; iCount++)
	{
		CString strSkinName;
		if (!Library.GetSkinName (iCount,strSkinName))
		{
			continue;
		}

		if (!bSkinFound)
		{
			iIndex = (int) m_SkinLibraresInstances.GetSize ();
			m_SkinLibraresInstances.SetAtGrow (iIndex,Library);
		}

		CString strSkiAuthor;
		Library.GetSkinAuthor (iCount, strSkiAuthor);

		CString strSkiAuthorMail;
		Library.GetSkinAuthorMail (iCount, strSkiAuthorMail);

		CString strSkiAuthorURL;
		Library.GetSkinAuthorURL (iCount, strSkiAuthorURL);

		CBCGPSkinEntry skin (strLibraryPath, strSkinName, 
			strSkiAuthor, strSkiAuthorMail, strSkiAuthorURL, iIndex, iCount);

		int iArIndex = (int) m_Skins.GetSize ();
		m_Skins.SetAtGrow (iArIndex, skin);

		bSkinFound = TRUE;
	}

	if (!bSkinFound)
	{
		::FreeLibrary (hInstance);
		return FALSE;
	}

	if (!bLoadLibrary)
	{
		::FreeLibrary(m_SkinLibraresInstances[iIndex].Detach ());
	}

	return TRUE;
}
//**********************************************************************************
BOOL CBCGPSkinManager::LoadState (LPCTSTR lpszProfileName)
{
	CString strProfileName = ::BCGPGetRegPath (strSkinProfile, lpszProfileName);

	CBCGPRegistrySP regSP;
	CBCGPRegistry& reg = regSP.Create (FALSE, TRUE);

	if (!reg.Open (strProfileName + strRegEntryName))
	{
		return FALSE;
	}

	CString strLibraryPath;
	CString strSkinName;
	int iSkin = BCG_DEFUALT_SKIN;

	if (!reg.Read (REG_ENTRY_LIBRARY_NAME, strLibraryPath) ||
		!reg.Read (REG_ENTRY_SKIN_NAME, strSkinName) ||
		!reg.Read (REG_ENTRY_SKIN_INDEX, iSkin))
	{
		return FALSE;
	}

	if (iSkin == BCG_DEFUALT_SKIN)
	{
		// Default skin, do nothing
		return TRUE;
	}

	if (!AddSkinLibrary (strLibraryPath))
	{
		TRACE(_T("Can't load skin library '%s'"), strLibraryPath);
		return FALSE;
	}

	for (int iIndex = 0; iIndex < m_Skins.GetSize (); iIndex++)
	{
		if (m_Skins[iIndex].m_strLibraryPath == strLibraryPath &&
		   m_Skins[iIndex].m_strSkinName == strSkinName &&
		   m_Skins[iIndex].m_iSkinIndexInLibrary == iSkin)
		{
			return SetActiveSkin (iIndex);
		}
	}

	// Saved skin not found
	return FALSE;
}
//**********************************************************************************
BOOL CBCGPSkinManager::SaveState (LPCTSTR lpszProfileName)
{
	CString strProfileName = ::BCGPGetRegPath (strSkinProfile, lpszProfileName);

 	BOOL bResult = FALSE;

	CString strLibraryPath;
	CString strSkinName;
	int iSkin = BCG_DEFUALT_SKIN;

	if (m_iActiveSkin != BCG_DEFUALT_SKIN)
	{
		ASSERT (m_iActiveSkin >= 0);
		ASSERT (m_iActiveSkin < m_Skins.GetSize ());

		strLibraryPath = m_Skins[m_iActiveSkin].m_strLibraryPath;
		strSkinName = m_Skins[m_iActiveSkin].m_strSkinName;
		iSkin = m_Skins[m_iActiveSkin].m_iSkinIndexInLibrary;
	}

	CBCGPRegistrySP regSP;
	CBCGPRegistry& reg = regSP.Create (FALSE, FALSE);

	if (reg.CreateKey (strProfileName + strRegEntryName))
	{
		bResult =	reg.Write (REG_ENTRY_LIBRARY_NAME, strLibraryPath) &&
					reg.Write (REG_ENTRY_SKIN_NAME, strSkinName) &&
					reg.Write (REG_ENTRY_SKIN_INDEX, iSkin);
	}

	return bResult;
}
//**********************************************************************************
LPCTSTR CBCGPSkinManager::GetSkinName (int iSkin) const
{
	if (iSkin == BCG_DEFUALT_SKIN)
	{
		return _T("Default");
	}

	if (iSkin < 0 || iSkin >= m_Skins.GetSize ())
	{
		return NULL;
	}

	return m_Skins [iSkin].m_strSkinName;
}
//**********************************************************************************
LPCTSTR CBCGPSkinManager::GetSkinAuthor (int iSkin) const
{
	if (iSkin == BCG_DEFUALT_SKIN)
	{
		return _T("BCGSoft");
	}

	if (iSkin < 0 || iSkin >= m_Skins.GetSize ())
	{
		return NULL;
	}

	return m_Skins [iSkin].m_strSkinAuthor;
}
//**********************************************************************************
LPCTSTR CBCGPSkinManager::GetSkinAuthorMail (int iSkin) const
{
	if (iSkin == BCG_DEFUALT_SKIN)
	{
		return _T("info@bcgsoft.com");
	}

	if (iSkin < 0 || iSkin >= m_Skins.GetSize ())
	{
		return NULL;
	}

	return m_Skins [iSkin].m_strSkinAuthorMail;
}
//**********************************************************************************
LPCTSTR CBCGPSkinManager::GetSkinAuthorURL (int iSkin) const
{
	if (iSkin == BCG_DEFUALT_SKIN)
	{
		return _T("http://www.bcgsoft.com");
	}

	if (iSkin < 0 || iSkin >= m_Skins.GetSize ())
	{
		return NULL;
	}

	return m_Skins [iSkin].m_strSkinAuthorURL;
}
//**********************************************************************************
BOOL CBCGPSkinManager::PreviewSkin (CDC* pDC, int iSkinIndex, CRect rect)
{
	ASSERT_VALID (pDC);

	if (iSkinIndex < 0 || iSkinIndex >= m_Skins.GetSize ())
	{
		return FALSE;
	}

	CBCGPSkinEntry& skinEntry = m_Skins [iSkinIndex];
	int iLibIndex = skinEntry.m_iLibraryIndex;
	int iSkin = skinEntry.m_iSkinIndexInLibrary;

	if (m_SkinLibraresInstances [iLibIndex].GetInstance () == NULL)
	{
		return FALSE;
	}

	return m_SkinLibraresInstances [iLibIndex].PreviewSkin (pDC, iSkin, rect);
}
//**********************************************************************************
BOOL CBCGPSkinManager::SetActiveSkin (int iSkin)
{
	if (m_iActiveSkin == iSkin)
	{
		return TRUE;
	}

	if (iSkin == BCG_DEFUALT_SKIN)
	{
		CBCGPVisualManager::DestroyInstance ();
		CBCGPVisualManager::GetInstance ();

		if (m_iActiveSkin != BCG_DEFUALT_SKIN) 
		{
			// In case of not default previous skin we must unload it
			long lPreviousLibIndex = m_Skins [m_iActiveSkin].m_iLibraryIndex;
			::FreeLibrary (m_SkinLibraresInstances [lPreviousLibIndex].Detach ());
		}

		m_iActiveSkin = iSkin;

		if (g_pWorkspace != NULL)
		{
			g_pWorkspace->OnSelectSkin ();
		}

		return TRUE;
	}

	if (iSkin < 0 || iSkin >= m_Skins.GetSize ())
	{
		ASSERT(FALSE);
		return FALSE;
	}

	CBCGPSkinEntry& skinEntry = m_Skins [iSkin];
	int iLibIndex = skinEntry.m_iLibraryIndex;
	int iSkinIndex = skinEntry.m_iSkinIndexInLibrary;

	CBCGPSkinLibrary& skinLibrary = m_SkinLibraresInstances [iLibIndex];

	if (skinLibrary.GetInstance () == NULL)
	{
		HINSTANCE hInstance = ::LoadLibrary (skinEntry.m_strLibraryPath);

		if (!skinLibrary.Init (hInstance))
		{
			ASSERT(FALSE);
			return FALSE;
		}
	}

	CRuntimeClass* pSkin = skinLibrary.GetSkin (iSkinIndex);
	if (pSkin == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	CBCGPVisualManager::DestroyInstance ();

	if (m_iActiveSkin != BCG_DEFUALT_SKIN) // In case of not default previous skin we must unload it
	{
		if (skinEntry.m_strLibraryPath != m_Skins [m_iActiveSkin].m_strLibraryPath)
		{
			long lPreviousLibIndex = m_Skins [m_iActiveSkin].m_iLibraryIndex;

			::FreeLibrary(m_SkinLibraresInstances[lPreviousLibIndex].Detach ());
		}
	}

	m_iActiveSkin = BCG_DEFUALT_SKIN;

	if (CBCGPVisualManager::CreateVisualManager (pSkin) == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	m_iActiveSkin = iSkin;

	if (g_pWorkspace != NULL)
	{
		g_pWorkspace->OnSelectSkin ();
	}
	return TRUE;
}
//**********************************************************************************
BOOL CBCGPSkinManager::ShowSelectSkinDlg ()
{
	CBCGPLocalResource localRes;

	CBCGPSelectSkinDlg dlg;
	return dlg.DoModal () == IDOK;
}
//**********************************************************************************
BOOL CBCGPSkinLibrary::Init (HINSTANCE hInstance)
{
	if (hInstance == NULL)
	{
		return FALSE;
	}

	if ((m_pfGetSkinVersion = 
		(GETBCGSKINVERSION)::GetProcAddress(hInstance, "BCGCBProGetSkinVersion")) == NULL)
	{
		TRACE0("CBCGPSkinLibrary::Init: Can't find BCGCBProGetSkinVersion proc\n");
		return FALSE;
	}

	if ((m_pfIsUNICODE = 
		(ISUNICODE)::GetProcAddress(hInstance, "BCGCBProIsUNICODE")) == NULL)
	{
		TRACE0("CBCGPSkinLibrary::Init: Can't find BCGCBProIsUNICODE proc\n");
	}

	if ((m_pfGetSkinCount = 
		(GETBCGSKINCOUNT)::GetProcAddress(hInstance, "BCGCBProGetSkinCount")) == NULL)
	{
		TRACE0("CBCGPSkinLibrary::Init: Can't find BCGCBProGetSkinCount proc\n");
		return FALSE;
	}

	if ((m_pfGetSkinName = 
		(GETBCGSKINNAME)::GetProcAddress(hInstance, "BCGCBProGetSkinName")) == NULL )
	{
		TRACE0("CBCGPSkinLibrary::Init: Can't find BCGCBProGetSkinName proc\n");
		return FALSE;
	}

	if ((m_pfGetSkinAuthor = 
		(GETBCGSKINNAME)::GetProcAddress(hInstance, "BCGCBProGetSkinAuthor")) == NULL )
	{
		TRACE0("CBCGPSkinLibrary::Init: Can't find BCGCBProGetSkinAuthor proc\n");
		return FALSE;
	}

	if ((m_pfGetSkinAuthorURL = 
		(GETBCGSKINNAME)::GetProcAddress(hInstance, "BCGCBProGetSkinAuthorURL")) == NULL )
	{
		TRACE0("CBCGPSkinLibrary::Init: Can't find BCGCBProGetSkinAuthorURL proc\n");
		return FALSE;
	}

	if ((m_pfGetSkinAuthorMail = 
		(GETBCGSKINNAME)::GetProcAddress(hInstance, "BCGCBProGetSkinAuthorMail")) == NULL )
	{
		TRACE0("CBCGPSkinLibrary::Init: Can't find BCGCBProGetSkinAuthorMail proc\n");
		return FALSE;
	}

	if ((m_pfGetSkin = 
		(GETBCGSKIN)::GetProcAddress(hInstance, "BCGCBProGetSkinClass")) == NULL)
	{
		TRACE0("CBCGPSkinLibrary::Init: Can't find BCGCBProGetSkinClass proc\n");
		return FALSE;
	}

	if ((m_pfSkinPreview = 
		(BCGPREVIEWSKIN)::GetProcAddress(hInstance, "BCGCBProPreviewSkin")) == NULL)
	{
		TRACE0("CBCGPSkinLibrary::Init: Can't find BCGCBProPreviewSkin proc\n");
		return FALSE;
	}

	m_hInstance = hInstance;
	return TRUE;
}
//**********************************************************************************
void CBCGPSkinManager::ScanSkinsLocation ()
{
	if (m_strSkinsDirectory.IsEmpty ())
	{
		return;
	}

	// Save infor about active skin:
	CString strCurrentLib;
	CString strCurrentSkin;

	BOOL bIsDefaultSkin = m_iActiveSkin == BCG_DEFUALT_SKIN;
	if (!bIsDefaultSkin)
	{
		strCurrentLib = m_Skins [m_iActiveSkin].m_strLibraryPath;
		strCurrentSkin = m_Skins [m_iActiveSkin].m_strSkinName;

		SetActiveSkin (BCG_DEFUALT_SKIN);
	}

	RemoveAllSkins ();
	RenameTempLibs ();

	CString strFindCriteria = m_strSkinsDirectory + DLL_FILE_MASK;

	CFileFind find;
	BOOL bResult = find.FindFile (strFindCriteria);

	while (bResult)
	{
		bResult = find.FindNextFile ();
		CString strFileName = find.GetFilePath ();
		AddSkinLibrary (strFileName, FALSE);
	}

	// Restore active skin:
	if (!bIsDefaultSkin)
	{
		for (int iIndex = 0; iIndex < m_Skins.GetSize (); iIndex++)
		{
			if (m_Skins [iIndex].m_strLibraryPath == strCurrentLib &&
			   m_Skins [iIndex].m_strSkinName == strCurrentSkin)
			{
				SetActiveSkin (iIndex);
				break;
			}
		}
	}
}
//**********************************************************************************
void CBCGPSkinManager::LoadAllSkins ()
{
	RenameTempLibs ();

	for (int i = 0; i < m_Skins.GetSize (); i++)
	{
		CBCGPSkinEntry& skinEntry = m_Skins [i];

		int iLibIndex = skinEntry.m_iLibraryIndex;
		CString strLibPath = skinEntry.m_strLibraryPath;

		CBCGPSkinLibrary& skinLibrary = m_SkinLibraresInstances [iLibIndex];

		if (skinLibrary.GetInstance () != NULL)
		{
			continue;
		}

		HINSTANCE hInstance = ::LoadLibrary (skinEntry.m_strLibraryPath);

		if (!skinLibrary.Init (hInstance))
		{
			ASSERT(FALSE);
			continue;
		}
	}
}
//**********************************************************************************
void CBCGPSkinManager::UnLoadAllSkins ()
{
	CString strCurrentLib;

	if (m_iActiveSkin != BCG_DEFUALT_SKIN)
	{
		strCurrentLib = m_Skins [m_iActiveSkin].m_strLibraryPath;
	}

	for (int i = 0; i < m_Skins.GetSize (); i++)
	{
		CBCGPSkinEntry& skinEntry = m_Skins [i];

		int iLibIndex = skinEntry.m_iLibraryIndex;
		CString strLibPath = skinEntry.m_strLibraryPath;

		if (strCurrentLib == strLibPath)
		{
			continue;
		}

		CBCGPSkinLibrary& skinLibrary = m_SkinLibraresInstances [iLibIndex];

		if (skinLibrary.GetInstance () == NULL)
		{
			continue;
		}

		::FreeLibrary(skinLibrary.Detach ());
	}
}
//**********************************************************************************
void CBCGPSkinManager::RemoveAllSkins ()
{
	m_Skins.RemoveAll ();

	for (int i = 0; i < m_SkinLibraresInstances.GetSize (); i++)
	{
		::FreeLibrary(m_SkinLibraresInstances[i].GetInstance ());
	}

	m_SkinLibraresInstances.RemoveAll ();
}
//***********************************************************************************
void CBCGPSkinManager::EnableSkinsDownload (	LPCTSTR lpszURL,
											LPCTSTR lpszUserName,
											LPCTSTR lpszPassword,
											LPCTSTR lpszDownloadDLLName)
{
	m_strSkinsURL = (lpszURL == NULL) ? _T("") : lpszURL;

#ifdef _UNICODE
	m_strDownloadDllName = (lpszDownloadDLLName == NULL) ?
				_T("BCGSkinDownloaderU.dll") : lpszDownloadDLLName;
#else
	m_strDownloadDllName = (lpszDownloadDLLName == NULL) ?
				_T("BCGSkinDownloader.dll") : lpszDownloadDLLName;
#endif

	m_strUserName = (lpszUserName == NULL) ? _T("") : lpszUserName;
	m_strUserPassword = (lpszPassword == NULL) ? _T("") : lpszPassword;
}
//***********************************************************************************
BOOL CBCGPSkinManager::DownloadSkins ()
{
#ifdef _BCGCBPRO_EVAL_
	AfxMessageBox (_T("This feature is available in the retail version only."));
	return FALSE;
#endif

	CWaitCursor wait;

	HINSTANCE hInstDLL = ::LoadLibrary (m_strDownloadDllName);
	if (hInstDLL == NULL)
	{
		CBCGPLocalResource localRes;

		CString strError;
		strError.Format (IDS_BCGBARRES_CANT_LOAD_DLL_FMT, m_strDownloadDllName);

		AfxMessageBox (strError);
		return FALSE;
	}

	BOOL bRes = FALSE;
#ifdef _UNICODE
	BOOL bIsUnicode = TRUE;
#else
	BOOL bIsUnicode = FALSE;
#endif

	LPCSTR lpszLoadSkinsFunc3 = "BCGCBDownloadSkins3";
	LPCSTR lpszLoadSkinsFunc2 = "BCGCBDownloadSkins2";
	LPCSTR lpszLoadSkinsFunc = "BCGCBDownloadSkins";

	BCGCBDOWNLOADSKINS3 pfDownloadSkins3 = 
		(BCGCBDOWNLOADSKINS3)::GetProcAddress (hInstDLL, lpszLoadSkinsFunc3);
	if (pfDownloadSkins3 != NULL)
	{
		bRes = pfDownloadSkins3 (m_strSkinsURL, m_strUserName, m_strUserPassword,
			m_strSkinsDirectory, _BCGCBPRO_VERSION_MAJOR, _BCGCBPRO_VERSION_MINOR, bIsUnicode,
			1 /* PE */);
	}
	else
	{
		BCGCBDOWNLOADSKINS2 pfDownloadSkins2 = 
			(BCGCBDOWNLOADSKINS2)::GetProcAddress (hInstDLL, lpszLoadSkinsFunc2);
		if (pfDownloadSkins2 != NULL)
		{
			bRes = pfDownloadSkins2 (m_strSkinsURL, m_strUserName, m_strUserPassword,
				m_strSkinsDirectory, _BCGCBPRO_VERSION_MAJOR, _BCGCBPRO_VERSION_MINOR, bIsUnicode);

		}
		else
		{
			BCGCBDOWNLOADSKINS pfDownloadSkins = 
				(BCGCBDOWNLOADSKINS)::GetProcAddress (hInstDLL, lpszLoadSkinsFunc);

			if (pfDownloadSkins == NULL)
			{
				CBCGPLocalResource localRes;

				CString strError;
				strError.Format (IDS_BCGBARRES_CANNT_FIND_ENTRY, 
					m_strDownloadDllName, _T("BCGCBDownloadSkins"));

				AfxMessageBox (strError);
				::FreeLibrary(hInstDLL);

				return FALSE;
			}

			bRes = pfDownloadSkins (m_strSkinsURL, m_strUserName, m_strUserPassword,
				m_strSkinsDirectory, _BCGCBPRO_VERSION_MAJOR, _BCGCBPRO_VERSION_MINOR);
		}
	}

	wait.Restore ();

	if (GetWorkspace () != NULL)
	{
		GetWorkspace()->OnAfterDownloadSkins (m_strSkinsDirectory);
	}

	::FreeLibrary(hInstDLL);
	return bRes;
}
//**********************************************************************************
BOOL CBCGPSkinManager::RenameTempLibs ()
{
	//---------------------------------------------
	// We need to rename all temporary (downloaded)
	// skin DLL to the actual names:
	//---------------------------------------------
	if (m_strSkinsDirectory.IsEmpty ())
	{
		return TRUE;
	}

	CString strFindCriteria = m_strSkinsDirectory + TMP_FILE_MASK;

	CFileFind find;
	BOOL bResult = find.FindFile (strFindCriteria);

	while (bResult)
	{
		bResult = find.FindNextFile ();

		CString strNameSkin = find.GetFilePath ();
		
		TCHAR szNameDLL [_MAX_PATH];   
		TCHAR drive [_MAX_DRIVE];   
		TCHAR dir [_MAX_DIR];
		TCHAR fname [_MAX_FNAME];   
		TCHAR ext [_MAX_EXT];

#if _MSC_VER < 1400
		_tsplitpath (strNameSkin, drive, dir, fname, ext);
		_tmakepath (szNameDLL, drive, dir, fname, _T("dll"));
#else
		_tsplitpath_s (strNameSkin, drive, dir, fname, ext);
		_tmakepath_s (szNameDLL, drive, dir, fname, _T("dll"));
#endif

		if (_taccess (szNameDLL, 0) != -1)
		{
			// The previous version is already exist, delete it
			_tremove (szNameDLL);
		}

		_trename (strNameSkin, szNameDLL);
	}

	return TRUE;
}

#endif
