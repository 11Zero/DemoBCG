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

// MenuHash.cpp: implementation of the CBCGPMenuHash class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BCGPToolbar.h"
#include "MenuHash.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CBCGPMenuHash g_menuHash;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGPMenuHash::CBCGPMenuHash()
{
	m_bIsActive = FALSE;
}
//****************************************************************************************
CBCGPMenuHash::~CBCGPMenuHash()
{
}
//****************************************************************************************
BOOL CBCGPMenuHash::SaveMenuBar (HMENU hMenu, CBCGPToolBar* pBar)
{
	ASSERT_VALID (pBar);

	if (pBar->GetCount () == 0)
	{
		return FALSE;
	}

	HANDLE hFileOld = NULL;
	if (m_StoredMenues.Lookup (hMenu, hFileOld))
	{
		//--------------------
		// Free unused handle:
		//--------------------
		::CloseHandle (hFileOld);
	}

	//---------------------
	// Get the temp path...
	//---------------------
	CString strTempPath;
	GetTempPath (MAX_PATH, strTempPath.GetBuffer (MAX_PATH));
	strTempPath.ReleaseBuffer();

	//-------------------------------------------
	// Create a temporary file for the output....
	//-------------------------------------------
	CString strTempName;
	GetTempFileName (strTempPath, _T("BCG"), 0, strTempName.GetBuffer (MAX_PATH));
	strTempName.ReleaseBuffer ();

	HANDLE hFile = ::CreateFile (strTempName, GENERIC_READ | GENERIC_WRITE,
		0, NULL, CREATE_ALWAYS, 
		FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		TRACE(_T("Can't create temporary file!\n"));
		return FALSE;
	}

	try
	{
		//---------------------------------
		// Write a menubar context to file:
		//---------------------------------
		#if _MSC_VER >= 1300
			CFile file (hFile);
		#else
			CFile file ((HFILE) hFile);
		#endif

		CArchive ar (&file, CArchive::store);

		m_bIsActive = TRUE;

		pBar->Serialize (ar);
		ar.Flush ();

		m_bIsActive = FALSE;
    }
	catch (CArchiveException* pEx)
	{
		TRACE(_T("Archive exception in CBCGPMenuHash::SaveMenuBar ()!\n"));
		pEx->Delete ();
        ::CloseHandle(hFile);

		m_bIsActive = FALSE;
		return FALSE;
	}
	catch (CMemoryException* pEx)
	{
		TRACE(_T("Memory exception in CBCGPMenuHash::SaveMenuBar ()!\n"));
		pEx->Delete ();
        ::CloseHandle(hFile);

		m_bIsActive = FALSE;
		return FALSE;
	}
	catch (CFileException* pEx)
	{
		TRACE(_T("File exception in CBCGPMenuHash::SaveMenuBar ()!\n"));
		pEx->Delete ();
        ::CloseHandle(hFile);

		m_bIsActive = FALSE;
		return FALSE;
	}

	m_StoredMenues.SetAt (hMenu, hFile);
	return TRUE;
}
//****************************************************************************************
BOOL CBCGPMenuHash::LoadMenuBar (HMENU hMenu, CBCGPToolBar* pBar)
{
	ASSERT_VALID (pBar);
	ASSERT (hMenu != NULL);

	//----------------------------------------------
	// Find a file handler associated with the menu:
	//----------------------------------------------
	HANDLE hFile;
	if (!m_StoredMenues.Lookup (hMenu, hFile))
	{
		return FALSE;
	}

	//-----------------
	// Rewind the file:
	//-----------------
	if (::SetFilePointer (hFile, 0, NULL, FILE_BEGIN) == 0xFFFFFFFF)
	{
		TRACE(_T("CBCGPMenuHash::LoadMenuBar (). Invalid file handle\n"));
		return FALSE;
	}

	try
	{
		#if _MSC_VER >= 1300
			CFile file (hFile);
		#else
			CFile file ((HFILE) hFile);
		#endif

		CArchive ar (&file, CArchive::load);

		m_bIsActive = TRUE;

		pBar->Serialize (ar);

		m_bIsActive = FALSE;
	}
	catch (CArchiveException* pEx)
	{
		TRACE(_T("Archive exception in CBCGPMenuHash::LoadMenuBar ()!\n"));
		pEx->Delete ();

		m_bIsActive = FALSE;
		return FALSE;
	}
	catch (CMemoryException* pEx)
	{
		TRACE(_T("Memory exception in CBCGPMenuHash::LoadMenuBar ()!\n"));
		pEx->Delete ();

		m_bIsActive = FALSE;
		return FALSE;
	}
	catch (CFileException* pEx)
	{
		TRACE(_T("File exception in CBCGPMenuHash::LoadMenuBar ()!\n"));
		pEx->Delete ();

		m_bIsActive = FALSE;
		return FALSE;
	}

	return TRUE;
}
//*************************************************************************************
BOOL CBCGPMenuHash::RemoveMenu (HMENU hMenu)
{
	HANDLE hFile = NULL;
	if (m_StoredMenues.Lookup (hMenu, hFile))
	{
		//--------------------
		// Free unused handle:
		//--------------------
		::CloseHandle (hFile);
		m_StoredMenues.RemoveKey (hMenu);
		return TRUE;
	}

	return FALSE;
}
//*************************************************************************************
void CBCGPMenuHash::CleanUp ()
{
	for (POSITION pos = m_StoredMenues.GetStartPosition (); pos != NULL;)
	{
		HMENU hMenu;
		HANDLE hFile;

		m_StoredMenues.GetNextAssoc (pos, hMenu, hFile);
		::CloseHandle (hFile);
	}

	m_StoredMenues.RemoveAll ();
}
